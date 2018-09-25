/*
 * StartStopApplication.c
 *
 * john@usb-by-example.com
 *
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
//extern void PacketReceived_Callback(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaBuffer_t *DMAbuffer);

// Declare external data
extern CyBool_t glIsApplicationActive;	// Set true once device is enumerated

CyU3PDmaChannel glUSBtoCPU_Handle;		// Handle needed for Interrupt Out Endpoint
CyU3PDmaChannel glCPUtoUSB_Handle;		// Handle needed for Interrupt In Endpoint

const char* BusSpeed[] = { "Not Connected", "Full ", "High ", "Super" };

void PacketReceived_Callback(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaBuffer_t *DMAbuffer)
{
	// I only get producer events at this callback
	CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
	uint8_t Packet = *DMAbuffer->buffer;
	CyU3PDebugPrint(4, "\r\nPacket received = %x", Packet);
	Status = CyU3PDmaChannelSetXfer(Handle, 0);
	CheckStatus("Post another Producer buffer", Status);
}

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    // Display the enumerated device bus speed
    DebugPrint(4, "\r\nRunning at %sSpeed", BusSpeed[CyU3PUsbGetSpeed()]);

    // Configure and enable the Interrupt Endpoint
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
    epConfig.enable = CyTrue;
    epConfig.epType = CY_U3P_USB_EP_INTR;
    epConfig.burstLen = 1;
//r	epConfig.streams = 0;
    epConfig.pcktSize = REPORT_SIZE;
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
    CheckStatus("Setup Interrupt In Endpoint", Status);

    // Create a manual DMA channel between CPU producer socket and USB
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = 16;		// Minimum size, I only need REPORT_SIZE
    dmaConfig.count = 2;		// KeyDown and KeyUp
    dmaConfig.prodSckId = CY_FX_CPU_PRODUCER_SOCKET;
    dmaConfig.consSckId = CY_FX_EP_CONSUMER_SOCKET;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
//r dmaConfig.notification = 0;
//r	dmaConfig.cb = NULL;
//r	dmaConfig.prodHeader = 0;
//r	dmaConfig.prodFooter = 0;
//r	dmaConfig.consHeader = 0;
//r	dmaConfig.prodAvailCount = 0;
    Status = CyU3PDmaChannelCreate(&glCPUtoUSB_Handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig);
    CheckStatus("CreateCPUtoUSBdmaChannel", Status);

    // Set DMA Channel transfer size = infinite
    Status = CyU3PDmaChannelSetXfer(&glCPUtoUSB_Handle, 0);
    CheckStatus("CPUtoUSBdmaChannelSetXfer", Status);

    glIsApplicationActive = CyTrue;			// Now ready to run!
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    glIsApplicationActive = CyFalse;

    // Close down and disable the endpoint then close the DMA channel
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof (epConfig));
//r	epConfig.enable = CyFalse;
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
    CheckStatus("Disable Producer Endpoint", Status);
    Status = CyU3PDmaChannelDestroy(&glCPUtoUSB_Handle);
    CheckStatus("Close USBtoCPU DMA Channel", Status);
}

