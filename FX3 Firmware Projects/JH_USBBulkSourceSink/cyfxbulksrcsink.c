// Test Program for GPIF throughput
//
// john@usb-by-example.com
// 2-14-14
//
// Derived from Cypress example:
//
/*
 ## Cypress USB 3.0 Platform source file (cyfxbulksrcsink.c)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2011,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

// I converted this example to GPIF filling Bulk In buffers for throughput measurements


#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyfxbulksrcsink.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3gpio.h"
#include "cyu3utils.h"

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyBool_t glIsApplicationActive;			// Set true once device is enumerated
CyU3PDmaChannel glUARTtoCPU_Handle;		// Handle needed by Uart Callback routine
CyU3PDmaChannel glUSBtoCPU_Handle;		// Handle needed for Interrupt Endpoint
char glConsoleInBuffer[32];				// Buffer for user Console Input
uint32_t glConsoleInIndex;				// Index into ConsoleIn buffer

CyBool_t glIsApplnActive = CyFalse;      /* Whether the source sink application is active or not. */
uint32_t glDMARxCount = 0;               /* Counter to track the number of buffers received. */
uint32_t glDMATxCount = 0;               /* Counter to track the number of buffers transmitted. */
CyBool_t glDataTransStarted = CyFalse;   /* Whether DMA transfer has been started after enumeration. */
CyBool_t StandbyModeEnable  = CyFalse;   /* Whether standby mode entry is enabled. */
CyBool_t TriggerStandbyMode = CyFalse;   /* Request to initiate standby entry. */
CyBool_t glForceLinkU2      = CyFalse;   /* Whether the device should try to initiate U2 mode. */

volatile uint32_t glEp0StatCount = 0;           /* Number of EP0 status events received. */
uint8_t glEp0Buffer[32] __attribute__ ((aligned (32))); /* Local buffer used for vendor command handling. */

/* Control request related variables. */
CyU3PEvent glBulkLpEvent;       /* Event group used to signal the thread that there is a pending request. */
uint32_t   gl_setupdat0;        /* Variable that holds the setupdat0 value (bmRequestType, bRequest and wValue). */
uint32_t   gl_setupdat1;        /* Variable that holds the setupdat1 value (wIndex and wLength). */
#define CYFX_USB_CTRL_TASK      (1 << 0)        /* Event that indicates that there is a pending USB control request. */
#define CYFX_USB_HOSTWAKE_TASK  (1 << 1)        /* Event that indicates the a Remote Wake should be attempted. */

/* Buffer used for USB event logs. */
uint8_t *gl_UsbLogBuffer = NULL;
#define CYFX_USBLOG_SIZE        (0x1000)

/* GPIO used for testing IO state retention when switching from boot firmware to full firmware. */
#define FX3_GPIO_TEST_OUT               (50)
#define FX3_GPIO_TO_LOFLAG(gpio)        (1 << (gpio))
#define FX3_GPIO_TO_HIFLAG(gpio)        (1 << ((gpio) - 32))


/* Application Error Handler */
void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t apiRetStatus    /* API return status */
        )
{
    /* Application failed with the error code apiRetStatus */

    /* Add custom debug or recovery actions here */

    /* Loop Indefinitely */
    for (;;)
    {
        CyU3PThreadSleep (1000);
    }
}

void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status)
// In this initial debugging stage I stall on an un-successful system call, else I display progress
// Note that this assumes that there were no errors bringing up the Debug Console
{
	if (glDebugTxEnabled)				// Need to wait until ConsoleOut is enabled
	{
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PDebugPrint(4, "\r\n%s Successful", StringPtr);
			return;
		}
		// else hang here
		CyU3PDebugPrint(4, "\r\n%s failed, Status = %d\r\n", StringPtr, Status);
		while (1)
		{
			CyU3PDebugPrint(4, "?");
			CyU3PThreadSleep (1000);
		}
	}
}

void UartCallback(CyU3PUartEvt_t Event, CyU3PUartError_t Error)
// Handle characters typed in by the developer
// Later we will respond to commands terminated with a <CR>
{
	CyU3PDmaBuffer_t ConsoleInDmaBuffer;
	char InputChar;
	if (Event == CY_U3P_UART_EVENT_RX_DONE)
    {
		CyU3PDmaChannelSetWrapUp(&glUARTtoCPU_Handle);
		CyU3PDmaChannelGetBuffer(&glUARTtoCPU_Handle, &ConsoleInDmaBuffer, CYU3P_NO_WAIT);
		InputChar = (char)*ConsoleInDmaBuffer.buffer;
		CyU3PDebugPrint(4, "%c", InputChar);			// Echo the character
		if (InputChar == 0x0d)
		{
			CyU3PDebugPrint(4, "\r\nInput: '%s'", &glConsoleInBuffer[0]);
			glConsoleInIndex = 0;
		}
		else
		{
			glConsoleInBuffer[glConsoleInIndex] = InputChar;
			if (glConsoleInIndex++ < sizeof(glConsoleInBuffer)) glConsoleInBuffer[glConsoleInIndex] = 0;
			else glConsoleInIndex--;
		}
		CyU3PDmaChannelDiscardBuffer(&glUARTtoCPU_Handle);
		CyU3PUartRxSetBlockXfer(1);
    }
}

// Spin up the DEBUG Console, Out and In
CyU3PReturnStatus_t InitializeDebugConsole(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    Status = CyU3PUartInit();										// Start the UART driver
    CheckStatus("CyU3PUartInit", Status);

    CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
//r	uartConfig.parity   = CY_U3P_UART_NO_PARITY;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyTrue;
//r	uartConfig.flowCtrl = CyFalse;
	uartConfig.isDma    = CyTrue;
	Status = CyU3PUartSetConfig(&uartConfig, UartCallback);			// Configure the UART hardware
    CheckStatus("CyU3PUartSetConfig", Status);

    Status = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);					// Send as much data as I need to
    CheckStatus("CyU3PUartTxSetBlockXfer", Status);

	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 8);		// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) glDebugTxEnabled = CyTrue;
    CheckStatus("ConsoleOutEnabled", Status);
	CyU3PDebugPreamble(CyFalse);									// Skip preamble, debug info is targeted for a person

	// Now setup a DMA channel to receive characters from the Uart Rx
    Status = CyU3PUartRxSetBlockXfer(1);
    CheckStatus("CyU3PUartRxSetBlockXfer", Status);
	CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
	dmaConfig.size  		= 16;									// Minimum size allowed, I only need 1 byte
	dmaConfig.count 		= 1;									// I can't type faster than the Uart Callback routine!
	dmaConfig.prodSckId		= CY_U3P_LPP_SOCKET_UART_PROD;
	dmaConfig.consSckId 	= CY_U3P_CPU_SOCKET_CONS;
	dmaConfig.dmaMode 		= CY_U3P_DMA_MODE_BYTE;
	dmaConfig.notification	= CY_U3P_DMA_CB_PROD_EVENT;
	Status = CyU3PDmaChannelCreate(&glUARTtoCPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    CheckStatus("CreateDebugRxDmaChannel", Status);
    if (Status != CY_U3P_SUCCESS) CyU3PDmaChannelDestroy(&glUARTtoCPU_Handle);
    else
    {
		Status = CyU3PDmaChannelSetXfer(&glUARTtoCPU_Handle, 0);
		CheckStatus("ConsoleInEnabled", Status);
    }
    return Status;
}

/* This function starts the application. This is called
 * when a SET_CONF event is received from the USB host. The endpoints
 * are configured and the DMA pipe is setup in this function. */
void CyFxBulkSrcSinkApplnStart(void)
{
	uint32_t Seconds = 0;
    uint16_t size = 0;
    CyU3PEpConfig_t epCfg;
//    CyU3PDmaChannelConfig_t dmaCfg;
    CyU3PDmaMultiChannelConfig_t dmaMultiConfig;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();

    /* First identify the usb speed. Once that is identified,
     * create a DMA channel and start the transfer on this. */

    /* Based on the Bus Speed configure the endpoint packet size */
    switch (usbSpeed)
    {
    case CY_U3P_FULL_SPEED:
        size = 64;
        CyU3PDebugPrint(4, "\r\nRunning at Full Speed");
        break;

    case CY_U3P_HIGH_SPEED:
        size = 512;
        CyU3PDebugPrint(4, "\r\nRunning at High Speed");
        break;

    case  CY_U3P_SUPER_SPEED:
        size = 1024;
        CyU3PDebugPrint(4, "\r\nRunning at SuperSpeed");
        break;

    default:
        CyU3PDebugPrint(4, "Error! Invalid USB speed.\r\n");
        CyFxAppErrorHandler (CY_U3P_ERROR_FAILURE);
        break;
    }

    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyTrue;
    epCfg.epType = CY_U3P_USB_EP_BULK;
    epCfg.burstLen = (usbSpeed == CY_U3P_SUPER_SPEED) ? (CY_FX_EP_BURST_LENGTH) : 1;
    epCfg.streams = 0;
    epCfg.pcktSize = size;

    /* Producer endpoint configuration */
 //   apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epCfg);
 //   CheckStatus("CyU3PSetEpConfig(PRODUCER)", apiRetStatus);

    /* Consumer endpoint configuration */
    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epCfg);
    CheckStatus("CyU3PSetEpConfig(CONSUMER)", apiRetStatus);

    /* Flush the endpoint memory */
//    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);


    // I need an AUTO channel from GPIF, and since I have two producers I need to use dmaMultiConfig
    CyU3PMemSet ((uint8_t *)&dmaMultiConfig, 0, sizeof (dmaMultiConfig));
    dmaMultiConfig.size           = (size * CY_FX_EP_BURST_LENGTH);
    dmaMultiConfig.count          = CY_FX_BULKSRCSINK_DMA_BUF_COUNT;
    dmaMultiConfig.validSckCount  = 2;
    dmaMultiConfig.prodSckId [0]  = (CyU3PDmaSocketId_t)CY_U3P_PIB_SOCKET_0;	// This is PING
    dmaMultiConfig.prodSckId [1]  = (CyU3PDmaSocketId_t)CY_U3P_PIB_SOCKET_1;	// This is PONG
    dmaMultiConfig.consSckId [0]  = (CyU3PDmaSocketId_t)CY_FX_EP_CONSUMER_SOCKET;
//    dmaMultiConfig.prodAvailCount = 0;
//    dmaMultiConfig.prodHeader     = 0;
//    dmaMultiConfig.prodFooter     = 0;
//    dmaMultiConfig.consHeader     = 0;
    dmaMultiConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
//    dmaMultiConfig.notification   = 0;		// Mmmmm, I'd like these for tracking
//    dmaMultiConfig.cb             = 0;		// No notifications means no CallBacks
    apiRetStatus = CyU3PDmaMultiChannelCreate(&glChHandleBulkSrc, CY_U3P_DMA_TYPE_MANUAL_MANY_TO_ONE, &dmaMultiConfig);
    CheckStatus("CyU3PDmaMultiChannelCreate", apiRetStatus);

    /* Set DMA Channel transfer size */
    apiRetStatus = CyU3PDmaChannelSetXfer(&glChHandleBulkSrc, CY_FX_BULKSRCSINK_DMA_TX_SIZE);
    CheckStatus("CyU3PDmaChannelSetXfer", apiRetStatus);

///    CyFxBulkSrcSinkFillInBuffers ();

    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyTrue;
}

/* This function stops the application. This shall be called whenever a RESET
 * or DISCONNECT event is received from the USB host. The endpoints are
 * disabled and the DMA pipe is destroyed by this function. */
void CyFxBulkSrcSinkApplnStop(void)
{
    CyU3PEpConfig_t epCfg;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyFalse;

    /* Destroy the channels */
//    CyU3PDmaChannelDestroy (&glChHandleBulkSink);
    CyU3PDmaChannelDestroy (&glChHandleBulkSrc);

    /* Flush the endpoint memory */
//    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);

    /* Disable endpoints. */
    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyFalse;

    /* Producer endpoint configuration. */
//    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epCfg);
//    CheckStatus("CyU3PSetEpConfig(PRODUCER)", apiRetStatus);

    /* Consumer endpoint configuration. */
    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epCfg);
    CheckStatus("CyU3PSetEpConfig(CONSUMER)", apiRetStatus);

}

/* Callback to handle the USB setup requests. */
CyBool_t CyFxBulkSrcSinkApplnUSBSetupCB (
        uint32_t setupdat0, /* SETUP Data 0 */
        uint32_t setupdat1  /* SETUP Data 1 */
    )
{
    /* Fast enumeration is used. Only requests addressed to the interface, class,
     * vendor and unknown control requests are received by this function.
     * This application does not support any class or vendor requests. */

// This looks awful, a union structure would be better

	uint8_t  bRequest, bReqType;
    uint8_t  bType, bTarget;
    uint16_t wValue, wIndex, wLength;
    CyBool_t isHandled = CyFalse;

    /* Decode the fields from the setup request. */
    bReqType = (setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK);
    bType    = (bReqType & CY_U3P_USB_TYPE_MASK);
    bTarget  = (bReqType & CY_U3P_USB_TARGET_MASK);
    bRequest = ((setupdat0 & CY_U3P_USB_REQUEST_MASK) >> CY_U3P_USB_REQUEST_POS);
    wValue   = ((setupdat0 & CY_U3P_USB_VALUE_MASK)   >> CY_U3P_USB_VALUE_POS);
    wIndex   = ((setupdat1 & CY_U3P_USB_INDEX_MASK)   >> CY_U3P_USB_INDEX_POS);
    wLength  = ((setupdat1 & CY_U3P_USB_LENGTH_MASK)  >> CY_U3P_USB_LENGTH_POS);

    if (bType == CY_U3P_USB_STANDARD_RQT)
    {
        /* Handle SET_FEATURE(FUNCTION_SUSPEND) and CLEAR_FEATURE(FUNCTION_SUSPEND)
         * requests here. It should be allowed to pass if the device is in configured
         * state and failed otherwise. */
        if ((bTarget == CY_U3P_USB_TARGET_INTF) && ((bRequest == CY_U3P_USB_SC_SET_FEATURE)
                    || (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)) && (wValue == 0))
        {
            if (glIsApplnActive)
            {
                CyU3PUsbAckSetup ();

                /* As we have only one interface, the link can be pushed into U2 state as soon as
                   this interface is suspended.
                 */
                if (bRequest == CY_U3P_USB_SC_SET_FEATURE)
                {
                    glDataTransStarted = CyFalse;
                    glForceLinkU2      = CyTrue;
                }
                else
                {
                    glForceLinkU2 = CyFalse;
                }
            }
            else
                CyU3PUsbStall (0, CyTrue, CyFalse);

            isHandled = CyTrue;
        }

        /* CLEAR_FEATURE request for endpoint is always passed to the setup callback
         * regardless of the enumeration model used. When a clear feature is received,
         * the previous transfer has to be flushed and cleaned up. This is done at the
         * protocol level. Since this is just a loopback operation, there is no higher
         * level protocol. So flush the EP memory and reset the DMA channel associated
         * with it. If there are more than one EP associated with the channel reset both
         * the EPs. The endpoint stall and toggle / sequence number is also expected to be
         * reset. Return CyFalse to make the library clear the stall and reset the endpoint
         * toggle. Or invoke the CyU3PUsbStall (ep, CyFalse, CyTrue) and return CyTrue.
         * Here we are clearing the stall. */
        if ((bTarget == CY_U3P_USB_TARGET_ENDPT) && (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)
                && (wValue == CY_U3P_USBX_FS_EP_HALT))
        {
            if (glIsApplnActive)
            {
#if (0)
                if (wIndex == CY_FX_EP_PRODUCER)
                {
                    CyU3PUsbSetEpNak (CY_FX_EP_PRODUCER, CyTrue);
                    CyU3PBusyWait (125);

                    CyU3PDmaChannelReset (&glChHandleBulkSink);
                    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
                    CyU3PUsbResetEp (CY_FX_EP_PRODUCER);
                    CyU3PUsbSetEpNak (CY_FX_EP_PRODUCER, CyFalse);

                    CyU3PDmaChannelSetXfer (&glChHandleBulkSink, CY_FX_BULKSRCSINK_DMA_TX_SIZE);
                    CyU3PUsbStall (wIndex, CyFalse, CyTrue);
                    isHandled = CyTrue;
                    CyU3PUsbAckSetup ();
                }
#endif
                if (wIndex == CY_FX_EP_CONSUMER)
                {
                    CyU3PUsbSetEpNak (CY_FX_EP_CONSUMER, CyTrue);
                    CyU3PBusyWait (125);

                    CyU3PDmaChannelReset (&glChHandleBulkSrc);
                    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);
                    CyU3PUsbResetEp (CY_FX_EP_CONSUMER);
                    CyU3PUsbSetEpNak (CY_FX_EP_CONSUMER, CyFalse);

                    CyU3PDmaChannelSetXfer (&glChHandleBulkSrc, CY_FX_BULKSRCSINK_DMA_TX_SIZE);
                    CyU3PUsbStall (wIndex, CyFalse, CyTrue);
                    isHandled = CyTrue;
                    CyU3PUsbAckSetup ();

                }
            }
        }
    }

    if ((bType == CY_U3P_USB_VENDOR_RQT) && (bTarget == CY_U3P_USB_TARGET_DEVICE))
    {
        /* We set an event here and let the application thread below handle these requests.
         * isHandled needs to be set to True, so that the driver does not stall EP0. */
        isHandled = CyTrue;
        gl_setupdat0 = setupdat0;
        gl_setupdat1 = setupdat1;
        CyU3PEventSet (&glBulkLpEvent, CYFX_USB_CTRL_TASK, CYU3P_EVENT_OR);
    }

    return isHandled;
}

/* This is the callback function to handle the USB events. */
void CyFxBulkSrcSinkApplnUSBEventCB(CyU3PUsbEventType_t evtype, uint16_t evdata)
{
    static uint32_t num_connect    = 0;
    static uint32_t num_disconnect = 0;

    switch (evtype)
    {
    case CY_U3P_USB_EVENT_CONNECT:
      CyU3PDebugPrint (8, "CY_U3P_USB_EVENT_CONNECT detected (%d)\r\n", ++num_connect);
      break;

    case CY_U3P_USB_EVENT_SETCONF:
        /* If the application is already active
         * stop it before re-enabling. */
        if (glIsApplnActive)
        {
            CyFxBulkSrcSinkApplnStop ();
        }

        /* Start the source sink function. */
        CyFxBulkSrcSinkApplnStart ();
        break;

    case CY_U3P_USB_EVENT_RESET:
    case CY_U3P_USB_EVENT_DISCONNECT:
        glForceLinkU2 = CyFalse;

        /* Stop the source sink function. */
        if (glIsApplnActive)
        {
            CyFxBulkSrcSinkApplnStop ();
        }
        glDataTransStarted = CyFalse;

        if (evtype == CY_U3P_USB_EVENT_DISCONNECT) {
            CyU3PDebugPrint (8, "CY_U3P_USB_EVENT_DISCONNECT detected (%d)\r\n", ++num_disconnect);
        }
        break;

    case CY_U3P_USB_EVENT_EP0_STAT_CPLT:
        glEp0StatCount++;
        break;

    case CY_U3P_USB_EVENT_VBUS_REMOVED:
        if (StandbyModeEnable)
        {
        	TriggerStandbyMode = CyTrue;
            StandbyModeEnable  = CyFalse;
        }
        break;

    default:
        break;
    }
}

/* Callback function to handle LPM requests from the USB 3.0 host. This function is invoked by the API
   whenever a state change from U0 -> U1 or U0 -> U2 happens. If we return CyTrue from this function, the
   FX3 device is retained in the low power state. If we return CyFalse, the FX3 device immediately tries
   to trigger an exit back to U0.

   This application does not have any state in which we should not allow U1/U2 transitions; and therefore
   the function always return CyTrue.
 */
CyBool_t CyFxBulkSrcSinkApplnLPMRqtCB(CyU3PUsbLinkPowerMode link_mode)
{
    return CyTrue;
}

/* This function initializes the USB Module, sets the enumeration descriptors.
 * This function does not start the bulk streaming and this is done only when
 * SET_CONF event is received. */
void CyFxBulkSrcSinkApplnInit (void)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyBool_t no_renum = CyFalse;

    /* Start the USB functionality. */
    apiRetStatus = CyU3PUsbStart();
    if (apiRetStatus == CY_U3P_ERROR_NO_REENUM_REQUIRED) no_renum = CyTrue;
    else CheckStatus("CyU3PUsbStart", apiRetStatus);

    /* The fast enumeration is the easiest way to setup a USB connection,
     * where all enumeration phase is handled by the library. Only the
     * class / vendor requests need to be handled by the application. */
    CyU3PUsbRegisterSetupCallback(CyFxBulkSrcSinkApplnUSBSetupCB, CyTrue);

    /* Setup the callback to handle the USB events. */
    CyU3PUsbRegisterEventCallback(CyFxBulkSrcSinkApplnUSBEventCB);

    /* Register a callback to handle LPM requests from the USB 3.0 host. */
    CyU3PUsbRegisterLPMRequestCallback(CyFxBulkSrcSinkApplnLPMRqtCB);

    /* Set the USB Enumeration descriptors */

    /* Super speed device descriptor. */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_DEVICE_DESCR, NULL, (uint8_t *)CyFxUSB30DeviceDscr);
    CheckStatus("CyU3PUsbSetDes(SS_DEVICE)", apiRetStatus);

    /* High speed device descriptor. */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_DEVICE_DESCR, NULL, (uint8_t *)CyFxUSB20DeviceDscr);
    CheckStatus("CyU3PUsbSetDes(HS_DEVICE)", apiRetStatus);

    /* BOS descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_BOS_DESCR, NULL, (uint8_t *)CyFxUSBBOSDscr);
    CheckStatus("CyU3PUsbSetDes(SS_BOS)", apiRetStatus);

    /* Device qualifier descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_DEVQUAL_DESCR, NULL, (uint8_t *)CyFxUSBDeviceQualDscr);
    CheckStatus("CyU3PUsbSetDes(DEVQUAL)", apiRetStatus);

    /* Super speed configuration descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBSSConfigDscr);
    CheckStatus("CyU3PUsbSetDes(SS_CONFIG)", apiRetStatus);

    /* High speed configuration descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBHSConfigDscr);
    CheckStatus("CyU3PUsbSetDes(HS_CONFIG)", apiRetStatus);

    /* Full speed configuration descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_FS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBFSConfigDscr);
    CheckStatus("CyU3PUsbSetDes(FS_CONFIG)", apiRetStatus);

    /* String descriptor 0 */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 0, (uint8_t *)CyFxUSBStringLangIDDscr);
    CheckStatus("CyU3PUsbSetDes(STRING_0)", apiRetStatus);

    /* String descriptor 1 */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 1, (uint8_t *)CyFxUSBManufactureDscr);
    CheckStatus("CyU3PUsbSetDes(STRING_1)", apiRetStatus);

    /* String descriptor 2 */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 2, (uint8_t *)CyFxUSBProductDscr);
    CheckStatus("CyU3PUsbSetDes(STRING_2)", apiRetStatus);

    /* Register a buffer into which the USB driver can log relevant events. */
    gl_UsbLogBuffer = (uint8_t *)CyU3PDmaBufferAlloc (CYFX_USBLOG_SIZE);
    if (gl_UsbLogBuffer)
        CyU3PUsbInitEventLog (gl_UsbLogBuffer, CYFX_USBLOG_SIZE);

    CyU3PDebugPrint (4, "About to connect to USB host\r\r\n");

    /* Connect the USB Pins with super speed operation enabled. */
    if (!no_renum)
    {
        apiRetStatus = CyU3PConnectState(CyTrue, CyTrue);
        CheckStatus("CyU3PConnectState", apiRetStatus);
    }
    else
    {
        /* USB connection is already active. Configure the endpoints and DMA channels. */
        CyFxBulkSrcSinkApplnStart ();
    }

    CyU3PDebugPrint (8, "CyFxBulkSrcSinkApplnInit complete\r\r\n");
}

/* Entry function for the BulkSrcSinkAppThread. */
void ApplicationThread_Entry(uint32_t input)
{
	uint32_t Seconds = 0;
    CyU3PReturnStatus_t Status;
    uint32_t eventMask = CYFX_USB_CTRL_TASK | CYFX_USB_HOSTWAKE_TASK;   /* Mask representing events that we are interested in. */
    uint32_t eventStat;                                                 /* Variable to hold current status of the events. */

    uint16_t prevUsbLogIndex = 0, tmp1, tmp2;
    CyU3PUsbLinkPowerMode curState;

    /* Initialize the debug module */
    Status = InitializeDebugConsole();
    CheckStatus("InitializeDebugConsole", Status);

    while (1)
    {
    	CyU3PDebugPrint(4, "%d ", Seconds++);
    	CyU3PThreadSleep(1000);
    }

    /* Initialize the application */
    CyFxBulkSrcSinkApplnInit();

    while (1)
    {
        /* The following call will block until at least one of the events enabled in eventMask is received.
           The eventStat variable will hold the events that were active at the time of returning from this API.
           The CLEAR flag means that all events will be atomically cleared before this function returns.
          
           We cause this event wait to time out every 10 milli-seconds, so that we can periodically get the FX3
           device out of low power modes.
         */
        stat = CyU3PEventGet (&glBulkLpEvent, eventMask, CYU3P_EVENT_OR_CLEAR, &eventStat, 10);
        if (stat == CY_U3P_SUCCESS)
        {
            /* If the HOSTWAKE task is set, send a DEV_NOTIFICATION (FUNCTION_WAKE) or remote wakeup signalling
               based on the USB connection speed. */
            if (eventStat & CYFX_USB_HOSTWAKE_TASK)
            {
                CyU3PThreadSleep (1000);
                if (CyU3PUsbGetSpeed () == CY_U3P_SUPER_SPEED)
                    stat = CyU3PUsbSendDevNotification (1, 0, 0);
                else
                    stat = CyU3PUsbDoRemoteWakeup ();

                CheckStatus("Remote wake attempt)", stat);

             }

            /* If there is a pending control request, handle it here. */
            if (eventStat & CYFX_USB_CTRL_TASK)
            {
                uint8_t  bRequest, bReqType;
                uint16_t wLength, temp;
                uint16_t wValue, wIndex;

                /* Decode the fields from the setup request. */
                bReqType = (gl_setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK);
                bRequest = ((gl_setupdat0 & CY_U3P_USB_REQUEST_MASK) >> CY_U3P_USB_REQUEST_POS);
                wLength  = ((gl_setupdat1 & CY_U3P_USB_LENGTH_MASK)  >> CY_U3P_USB_LENGTH_POS);
                wValue   = ((gl_setupdat0 & CY_U3P_USB_VALUE_MASK) >> CY_U3P_USB_VALUE_POS);
                wIndex   = ((gl_setupdat1 & CY_U3P_USB_INDEX_MASK) >> CY_U3P_USB_INDEX_POS);

                if ((bReqType & CY_U3P_USB_TYPE_MASK) == CY_U3P_USB_VENDOR_RQT)
                {
                    switch (bRequest)
                    {
                    case 0x77:      /* Trigger remote wakeup. */
                        CyU3PUsbAckSetup ();
                        CyU3PEventSet (&glBulkLpEvent, CYFX_USB_HOSTWAKE_TASK, CYU3P_EVENT_OR);
                        break;

                    case 0x78:      /* Get count of EP0 status events received. */
                        CyU3PMemCopy ((uint8_t *)glEp0Buffer, ((uint8_t *)&glEp0StatCount), 4);
                        CyU3PUsbSendEP0Data (4, glEp0Buffer);
                        break;

                    case 0x79:      /* Request with no data phase. Insert a delay and then ACK the request. */
                        CyU3PThreadSleep (5);
                        CyU3PUsbAckSetup ();
                        break;

                    case 0x80:      /* Request with OUT data phase. Just get the data and ignore it for now. */
                        CyU3PUsbGetEP0Data (sizeof (glEp0Buffer), (uint8_t *)glEp0Buffer, &wLength);
                        break;

                    case 0x81:
                        /* Get the current event log index and send it to the host. */
                        if (wLength == 2)
                        {
                            temp = CyU3PUsbGetEventLogIndex ();
                            CyU3PMemCopy ((uint8_t *)glEp0Buffer, (uint8_t *)&temp, 2);
                            CyU3PUsbSendEP0Data (2, glEp0Buffer);
                        }
                        else
                            CyU3PUsbStall (0, CyTrue, CyFalse);
                        break;

                    case 0x82:
                        /* Send the USB event log buffer content to the host. */
                        if (wLength != 0)
                        {
                            if (wLength < CYFX_USBLOG_SIZE)
                                CyU3PUsbSendEP0Data (wLength, gl_UsbLogBuffer);
                            else
                                CyU3PUsbSendEP0Data (CYFX_USBLOG_SIZE, gl_UsbLogBuffer);
                        }
                        else
                            CyU3PUsbAckSetup ();
                        break;

                    case 0x83:
                        {
                            uint32_t addr = ((uint32_t)wValue << 16) | (uint32_t)wIndex;
                            CyU3PReadDeviceRegisters ((uvint32_t *)addr, 1, (uint32_t *)glEp0Buffer);
                            CyU3PUsbSendEP0Data (4, glEp0Buffer);
                        }
                        break;

                    case 0x84:
                        {
                            uint8_t major, minor, patch;

                            if (CyU3PUsbGetBooterVersion (&major, &minor, &patch) == CY_U3P_SUCCESS)
                            {
                                glEp0Buffer[0] = major;
                                glEp0Buffer[1] = minor;
                                glEp0Buffer[2] = patch;
                                CyU3PUsbSendEP0Data (3, glEp0Buffer);
                            }
                            else
                                CyU3PUsbStall (0, CyTrue, CyFalse);
                        }
                        break;

                    case 0x90:
                        /* Request to switch control back to the boot firmware. */

                        /* Complete the control request. */
                        CyU3PUsbAckSetup ();
                        CyU3PThreadSleep (10);

                        /* Get rid of the DMA channels and EP configuration. */
                        CyFxBulkSrcSinkApplnStop ();

                        /* De-initialize the Debug and UART modules. */
                        CyU3PDebugDeInit ();
                        CyU3PUartDeInit ();

                        /* Now jump back to the boot firmware image. */
                        CyU3PUsbSetBooterSwitch (CyTrue);
                        CyU3PUsbJumpBackToBooter (0x40078000);
                        while (1)
                            CyU3PThreadSleep (100);
                        break;

                    case 0xB1:
                        /* Switch to a USB 2.0 Connection. */
                        CyU3PUsbAckSetup ();
                        CyU3PThreadSleep (1000);
                        CyFxBulkSrcSinkApplnStop ();
                        CyU3PConnectState (CyFalse, CyTrue);
                        CyU3PThreadSleep (100);
                        CyU3PConnectState (CyTrue, CyFalse);
                        break;

                    case 0xB2:
                        /* Switch to a USB 3.0 connection. */
                        CyU3PUsbAckSetup ();
                        CyU3PThreadSleep (100);
                        CyFxBulkSrcSinkApplnStop ();
                        CyU3PConnectState (CyFalse, CyTrue);
                        CyU3PThreadSleep (10);
                        CyU3PConnectState (CyTrue, CyTrue);
                        break;

                    case 0xE0:
                        /* Request to reset the FX3 device. */
                        CyU3PUsbAckSetup ();
                        CyU3PThreadSleep (2000);
                        CyU3PConnectState (CyFalse, CyTrue);
                        CyU3PThreadSleep (1000);
                        CyU3PDeviceReset (CyFalse);
                        CyU3PThreadSleep (1000);
                        break;

                    case 0xE1:
                        /* Request to place FX3 in standby when VBus is next disconnected. */
                        StandbyModeEnable = CyTrue;
                        CyU3PUsbAckSetup ();
                        break;

                    default:        /* Unknown request. Stall EP0. */
                        CyU3PUsbStall (0, CyTrue, CyFalse);
                        break;
                    }
                }
                else
                {
                    /* Only vendor requests are to be handled here. */
                    CyU3PUsbStall (0, CyTrue, CyFalse);
                }
            }
        }

        /* Try to get the USB 3.0 link back to U0. */
        if (glForceLinkU2)
        {
            stat = CyU3PUsbGetLinkPowerState (&curState);
            while ((glForceLinkU2) && (stat == CY_U3P_SUCCESS) && (curState == CyU3PUsbLPM_U0))
            {
                /* Repeatedly try to go into U2 state.*/
                CyU3PUsbSetLinkPowerState (CyU3PUsbLPM_U2);
                CyU3PThreadSleep (5);
                stat = CyU3PUsbGetLinkPowerState (&curState);
            }
        }
        else
        {

            /* Once data transfer has started, we keep trying to get the USB link to stay in U0. If this is done
               before data transfers have started, there is a likelihood of failing the TD 9.24 U1/U2 test. */
            if ((CyU3PUsbGetSpeed () == CY_U3P_SUPER_SPEED) && (glDataTransStarted))
            {
                /* If the link is in U1/U2 states, try to get back to U0. */
                stat = CyU3PUsbGetLinkPowerState (&curState);
                while ((stat == CY_U3P_SUCCESS) && (curState >= CyU3PUsbLPM_U1) && (curState <= CyU3PUsbLPM_U2) &&
                        (glDataTransStarted))
                {
                    CyU3PUsbSetLinkPowerState (CyU3PUsbLPM_U0);
                    CyU3PThreadSleep (1);
                    stat = CyU3PUsbGetLinkPowerState (&curState);
                }
            }
        }

        if (TriggerStandbyMode)
        {
            TriggerStandbyMode = CyFalse;

            CyU3PConnectState (CyFalse, CyTrue);
            CyU3PUsbStop ();
            CyU3PDebugDeInit ();
            CyU3PUartDeInit ();

            /* VBus has been turned off. Go into standby mode and wait for VBus to be turned on again.
               The I-TCM content and GPIO register state will be backed up in the memory area starting
               at address 0x40060000. */
            stat = CyU3PSysEnterStandbyMode (CY_U3P_SYS_USB_VBUS_WAKEUP_SRC, CY_U3P_SYS_USB_VBUS_WAKEUP_SRC,
                    (uint8_t *)0x40060000);
            if (stat != CY_U3P_SUCCESS)
            {
            	ApplicationDebugInit ();
                CyU3PDebugPrint (4, "Enter standby returned %d\r\r\n", stat);
                CyFxAppErrorHandler (stat);
            }

            /* If the entry into standby succeeds, the CyU3PSysEnterStandbyMode function never returns. The
               firmware application starts running again from the main entry point. Therefore, this code
               will never be executed. */
            CyFxAppErrorHandler (1);
        }
        else
        {
            /* Compare the current USB driver log index against the previous value. */
            tmp1 = CyU3PUsbGetEventLogIndex ();
            if (tmp1 != prevUsbLogIndex)
            {
                tmp2 = prevUsbLogIndex;
                while (tmp2 != tmp1)
                {
                    CyU3PDebugPrint (4, "USB LOG: %x\r\r\n", gl_UsbLogBuffer[tmp2]);
                    tmp2++;
                    if (tmp2 == CYFX_USBLOG_SIZE)
                        tmp2 = 0;
                }
            }

            /* Store the current log index. */
            prevUsbLogIndex = tmp1;
        }
    }
}

/* Application define function which creates the threads. */
void CyFxApplicationDefine(void)
{
    void *ptr = NULL;
    uint32_t ret = CY_U3P_SUCCESS;

    /* Create an event flag group that will be used for signalling the application thread. */
//    ret = CyU3PEventCreate(&glBulkLpEvent);
    if (ret == 0)
    {
		/* Allocate the memory for the threads */
		ptr = CyU3PMemAlloc(CY_FX_BULKSRCSINK_THREAD_STACK);

		/* Create the thread for the application */
		ret = CyU3PThreadCreate(&ApplicationThread,                /* App thread structure */
                          "21:Bulk_src_sink",                      /* Thread ID and thread name */
                          ApplicationThread_Entry,                 /* App thread entry function */
                          0,                                       /* No input parameter to thread */
                          ptr,                                     /* Pointer to the allocated thread stack */
                          CY_FX_BULKSRCSINK_THREAD_STACK,          /* App thread stack size */
                          CY_FX_BULKSRCSINK_THREAD_PRIORITY,       /* App thread priority */
                          CY_FX_BULKSRCSINK_THREAD_PRIORITY,       /* App thread priority */
                          CYU3P_NO_TIME_SLICE,                     /* No time slice for the application thread */
                          CYU3P_AUTO_START                         /* Start the thread immediately */
                          );

		/* Check the return code */
		if (ret == 0) return;
    }
    /* Thread Creation failed with the error code retThrdCreate */

    /* Add custom recovery or debug actions here */

    /* Application cannot continue */

    /* Loop indefinitely */
    while(1);
 }

/*
 * Main function
 */
int main (void)
{
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize the device */
    CyU3PSysClockConfig_t clockConfig;
    clockConfig.setSysClk400  = CyTrue;
    clockConfig.cpuClkDiv     = 2;
    clockConfig.dmaClkDiv     = 2;
    clockConfig.mmioClkDiv    = 2;
    clockConfig.useStandbyClk = CyFalse;
    clockConfig.clkSrc         = CY_U3P_SYS_CLK;
    status = CyU3PDeviceInit (&clockConfig);
    if (status == CY_U3P_SUCCESS)
    {
		/* Initialize the caches. Enable both Instruction and Data caches. */
		status = CyU3PDeviceCacheControl (CyTrue, CyTrue, CyTrue);
		if (status == CY_U3P_SUCCESS)
		{
			/* Configure the IO matrix for the device. On the FX3 DVK board, the COM port
			 * is connected to the IO(53:56). This means that either DQ32 mode should be
			 * selected or lppMode should be set to UART_ONLY. Here we are choosing
			 * UART_ONLY configuration. */
			io_cfg.isDQ32Bit = CyFalse;
			io_cfg.s0Mode 	 = CY_U3P_SPORT_INACTIVE;
			io_cfg.s1Mode 	 = CY_U3P_SPORT_INACTIVE;
			io_cfg.useUart   = CyTrue;
			io_cfg.useI2C    = CyFalse;
			io_cfg.useI2S    = CyFalse;
			io_cfg.useSpi    = CyFalse;
			io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;

			/* No GPIOs are enabled. */
			io_cfg.gpioSimpleEn[0]  = 0;
			io_cfg.gpioSimpleEn[1]  = FX3_GPIO_TO_HIFLAG(FX3_GPIO_TEST_OUT);
			io_cfg.gpioComplexEn[0] = 0;
			io_cfg.gpioComplexEn[1] = 0;
			status = CyU3PDeviceConfigureIOMatrix(&io_cfg);
			if (status == CY_U3P_SUCCESS)
			{
				/* This is a non returnable call for initializing the RTOS kernel */
				CyU3PKernelEntry ();
			}
		}
    }

// Arrive here on a fatal error
    /* Cannot recover from this error. */
    while (1);

    /* Dummy return to keep the compiler happy */
    return 0;
}

/* [ ] */

