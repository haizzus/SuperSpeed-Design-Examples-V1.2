/*
 * USB Handler.c
 *
 *      Author: john@USB-By-Example.com
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t SetUSBdescriptors(void);
extern void StartApplication(void);
extern void StopApplication(void);
extern uint32_t BitPosition(uint32_t Value);

// Declare external data
extern CyBool_t glIsApplicationActive;			// Set true once device is enumerated
extern CyU3PEvent DisplayEvent;					// Used to display events

CyU3PUartConfig_t glUartConfig;
struct { uint32_t DTE_Rate; uint8_t StopBits; uint8_t Parity; uint8_t DataLength; } LineCoding = { 115200, 1, 0, 8 };

uint8_t glEP0Buffer[32] __attribute__ ((aligned(32)));

// Declare the callbacks needed to support the USB device driver
CyBool_t USBSetup_Callback(uint32_t setupdat0, uint32_t setupdat1)
{
	CyU3PReturnStatus_t Status;
	union { uint32_t SetupData[2];
			uint8_t RawBytes[8];
			struct { uint8_t Target:5; uint8_t Type:2; uint8_t Direction:1;
				 	 uint8_t Request; uint16_t Value; uint16_t Index; uint16_t Length; };
		  } Setup;
	uint16_t ReadCount;

	// Copy the incoming Setup Packet into my Setup union which will "unpack" the variables
	Setup.SetupData[0] = setupdat0;
	Setup.SetupData[1] = setupdat1;
#if (0)
	// Included for DEBUG to display each sub field in the USB Command if needed
	// Note that we are in a Callback so shouldn't really using DebugPrint
	uint32_t i;
//	DebugPrint(4, "\r\nSetup Input %X,%X", setupdat0, setupdat1);
//	DebugPrint(4, "\r\nRaw Bytes: ");
//	for (i=0; i<8; i++) DebugPrint(4, "%x,", Setup.RawBytes[i]);
	DebugPrint(4, "\r\nDirection:%d", Setup.Direction);
	DebugPrint(4, ", Type:%d", Setup.Type);
	DebugPrint(4, ", Target:%d", Setup.Target);
	DebugPrint(4, ", Request:%X", Setup.Request);
	DebugPrint(4, "\r\nValue:%X", Setup.Value);
	DebugPrint(4, ", Index:%d", Setup.Index);
	DebugPrint(4, ", Length:%d", Setup.Length);
#endif
	// USB Driver will send me Class and Vendor requests to handle
	// I only have to handle three class requests for a CDC Device
    if (Setup.Target == CLASS_REQUEST)
    {
    	if (Setup.Request == SET_LINE_CODING)
    	{
    		Status = CyU3PUsbGetEP0Data(sizeof(LineCoding), (uint8_t*)&LineCoding, &ReadCount);
			// 'Set Line Coding' moved to char* EventName[30]
    		Status = CyU3PEventSet(&DisplayEvent, 1<<30, CYU3P_EVENT_OR);
#if (DirectConnect)
    		{
    			glUartConfig.baudRate = LineCoding.DTE_Rate;
    			// Update other parameters only if I can support them
    			if (LineCoding.StopBits == 0) glUartConfig.stopBit = CY_U3P_UART_ONE_STOP_BIT;
    			if (LineCoding.StopBits == 2) glUartConfig.stopBit = CY_U3P_UART_TWO_STOP_BIT;
    			if (LineCoding.Parity == 0) glUartConfig.parity = CY_U3P_UART_NO_PARITY;
    			if (LineCoding.Parity == 1) glUartConfig.parity = CY_U3P_UART_ODD_PARITY;
    			if (LineCoding.Parity == 2) glUartConfig.parity = CY_U3P_UART_EVEN_PARITY;
    			Status = CyU3PUartSetConfig(&glUartConfig, NULL);
    			CheckStatus("Change UART Configuration", Status);

    		}
#endif
    		return CyTrue;
    	}
    	else if (Setup.Request == GET_LINE_CODING)
    	{
			// 'Get Line Coding' moved to char* EventName[29]
    		Status = CyU3PEventSet(&DisplayEvent, 1<<29, CYU3P_EVENT_OR);
#if (DirectConnect)
    		LineCoding.DTE_Rate = glUartConfig.baudRate;
    		if (glUartConfig.stopBit == CY_U3P_UART_ONE_STOP_BIT) LineCoding.StopBits = 0;
    		if (glUartConfig.stopBit == CY_U3P_UART_TWO_STOP_BIT) LineCoding.StopBits = 2;
    		if (glUartConfig.parity == CY_U3P_UART_NO_PARITY) LineCoding.Parity = 0;
    		if (glUartConfig.parity == CY_U3P_UART_ODD_PARITY) LineCoding.Parity = 1;
    		if (glUartConfig.parity == CY_U3P_UART_EVEN_PARITY) LineCoding.Parity = 2;
#endif
    		Status = CyU3PUsbSendEP0Data(sizeof(LineCoding), (uint8_t*)&LineCoding);
			CheckStatus("Report UART Configuration", Status);
			return CyTrue;
    	}
    	else if (Setup.Request == SET_CONTROL_LINE_STATE)
    	{
			// 'Set Control Line State' moved to char* EventName[28]
    		Status = CyU3PEventSet(&DisplayEvent, 1<<28, CYU3P_EVENT_OR);
    		if (glIsApplicationActive) CyU3PUsbAckSetup();
    		else CyU3PUsbStall(0, CyTrue, CyFalse);
    		return CyTrue;
    	}
    }
	return CyFalse;
}

void USBEvent_Callback(CyU3PUsbEventType_t Event, uint16_t EventData )
{
//	Don't user DebugPrint in a Callback, set an event and this will be displayed later
//	DebugPrint(4, "\r\nEvent received = %s", EventName[Event]);
	CyU3PEventSet(&DisplayEvent, BitPosition((uint32_t)Event), CYU3P_EVENT_OR);
    switch (Event)
    {
        case CY_U3P_USB_EVENT_SETCONF:
            /* Stop the application before re-starting. */
            if (glIsApplicationActive) StopApplication();
            StartApplication();
            break;

        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_CONNECT:
        case CY_U3P_USB_EVENT_DISCONNECT:
            if (glIsApplicationActive)
            {
                CyU3PUsbLPMEnable();
                StopApplication();
            }
            break;

        default:
            break;
    }
}

CyBool_t LPMRequest_Callback(CyU3PUsbLinkPowerMode link_mode)
{
	// OK to move into U1/U2 in this application
    return CyTrue;
}

// Spin up USB, let the USB driver handle enumeration
CyU3PReturnStatus_t InitializeUSB(void)
{
	CyU3PReturnStatus_t Status;
	Status = CyU3PUsbStart();
	CheckStatus("Start USB Driver", Status);
	// Setup callbacks to handle the setup requests, USB Events and LPM Requests (for USB 3.0)
    CyU3PUsbRegisterSetupCallback(USBSetup_Callback, CyTrue);
    CyU3PUsbRegisterEventCallback(USBEvent_Callback);
    CyU3PUsbRegisterLPMRequestCallback(LPMRequest_Callback);

    // Driver needs all of the descriptors so it can supply them to the host when requested
    Status = SetUSBdescriptors();
    CheckStatus("Set USB Descriptors", Status);

    /* Connect the USB Pins with SuperSpeed operation enabled. */
    ////// I have a problem with my USB 3.0 Descriptors and this does not enumerate at SuperSpeed
    ////// Get an Ellisys trace and FIX!
    Status = CyU3PConnectState(CyTrue, CyFalse);
    CheckStatus("Connect USB", Status);

    return Status;
}




