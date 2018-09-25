/*
` * StartStopApplication.c
 *
 *      Author: john@USB-By-Example.com
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

// Declare external data
extern CyBool_t glIsApplicationActive;	// Set true once device is enumerated


const char* BusSpeed[] = { "Not Connected", "Full ", "High ", "Super" };
const uint16_t EpSize[] = { 0, 64, 512, 1024 };

CyU3PDmaChannel glBulkLoop_Handle;		// Handle needed for BulkLoop Transfers
CyU3PDmaChannel glCDCtoCPU_Handle;		// Handle needed for CDC Out Endpoint
CyU3PDmaChannel glCPUtoCDC_Handle;		// Handle needed for CDC In Endpoint
CyU3PDmaChannel glCDCtoCDC_Handle;		// Handle needed for CDC Loopback

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
    uint16_t Size = EpSize[CyU3PUsbGetSpeed()];

    // Display the enumerated device bus speed
    DebugPrint(4, "\nRunning at %sSpeed", BusSpeed[CyU3PUsbGetSpeed()]);

    // Configure and enable the BulkLoop Endpoints
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
    epConfig.enable = CyTrue;
    epConfig.epType = CY_U3P_USB_EP_BULK;
    epConfig.burstLen = 1;
    epConfig.pcktSize = Size;
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER_BULKLOOP, &epConfig);
    CheckStatus("Setup BulkLoop Consumer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER_BULKLOOP, &epConfig);
    CheckStatus("Setup BulkLoop Producer Endpoint", Status);
    // Configure and enable the CDC Endpoints
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER_CDC, &epConfig);
    CheckStatus("Setup CDC Consumer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER_CDC, &epConfig);
    CheckStatus("Setup CDC Producer Endpoint", Status);
    epConfig.epType = CY_U3P_USB_EP_INTR;
    epConfig.pcktSize = 64;
    epConfig.isoPkts = 1;
    Status = CyU3PSetEpConfig(CY_FX_EP_INTERRUPT_CDC, &epConfig);
    CheckStatus("Setup CDC Interrupt Endpoint", Status);

    // Create a DMA Auto Channel between two USB sockets for BulkLoop interface
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = Size;
    dmaConfig.count = CY_FX_BULKLOOP_DMA_BUFFER_COUNT;
    dmaConfig.prodSckId = CY_FX_EP_PRODUCER_BULKLOOP_SOCKET;
    dmaConfig.consSckId = CY_FX_EP_CONSUMER_BULKLOOP_SOCKET;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    Status = CyU3PDmaChannelCreate(&glBulkLoop_Handle, CY_U3P_DMA_TYPE_AUTO, &dmaConfig);
    CheckStatus("CreateBulkLoopDmaChannel", Status);
    /* Flush the Endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER_BULKLOOP);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER_BULKLOOP);
    // Set DMA Channel transfer size to Infinite
    Status = CyU3PDmaChannelSetXfer(&glBulkLoop_Handle, 0);
    CheckStatus("SetXferBulkLoopDmaChannel", Status);

    // At power on create an AUTO channel between CDC producer and consumer = loopback
    dmaConfig.size = 16;		// I assume a person is typing
    dmaConfig.count = 2;
    dmaConfig.prodSckId = CY_FX_EP_PRODUCER_CDC_SOCKET;
    dmaConfig.consSckId = CY_FX_EP_CONSUMER_CDC_SOCKET;;
    Status = CyU3PDmaChannelCreate(&glCDCtoCDC_Handle, CY_U3P_DMA_TYPE_AUTO, &dmaConfig);
    CheckStatus("CreateCDCLoopbackDmaChannel", Status);
    /* Flush the Endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER_CDC);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER_CDC);
    CyU3PUsbFlushEp(CY_FX_EP_INTERRUPT_CDC);
    // Set DMA Channel transfer size = infinite
    Status = CyU3PDmaChannelSetXfer(&glCDCtoCDC_Handle, 0);
    CheckStatus("CDCLoopbackDmaChannelSetXfer", Status);

    glIsApplicationActive = CyTrue;			// Now ready to run!
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PReturnStatus_t Status;

    glIsApplicationActive = CyFalse;

    // Close down and disable the endpoints then close the DMA channels
// CDC Interface
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER_CDC);
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER_CDC);
    CyU3PUsbFlushEp(CY_FX_EP_INTERRUPT_CDC);
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof (epConfig));
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER_CDC, &epConfig);
    CheckStatus("Disable UART Consumer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER_CDC, &epConfig);
    CheckStatus("Disable UART Producer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_INTERRUPT_CDC, &epConfig);
    CheckStatus("Disable Interrupt Endpoint", Status);
    Status = CyU3PDmaChannelDestroy(&glCDCtoCDC_Handle);
    CheckStatus("Close CDCLoopback DMA Channel", Status);
//    Status = CyU3PDmaChannelDestroy(&glCPUtoCDC_Handle);
//    CheckStatus("Close CPUtoUSB DMA Channel", Status);
//    CyU3PMemFree(UserBuffer.buffer);
// BulkLoop Interface
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER_BULKLOOP);
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER_BULKLOOP);
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER_BULKLOOP, &epConfig);
    CheckStatus("Disable BulkLoop Consumer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER_BULKLOOP, &epConfig);
    CheckStatus("Disable BulkLoop Producer Endpoint", Status);
    Status = CyU3PDmaChannelDestroy(&glBulkLoop_Handle);
    CheckStatus("Close BulkLoop DMA Channel", Status);
}

