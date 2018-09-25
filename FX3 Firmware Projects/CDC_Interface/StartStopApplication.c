/*
 * StartStopApplication.c
 *
 *      Author: john@USB-By-Example.com
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

// Declare external data
extern CyBool_t glIsApplicationActive;	// Set true once device is enumerated
extern CyU3PEvent DisplayEvent;			// Used to display events

const char* BusSpeed[] = { "Not Connected", "Full ", "High ", "Super" };
const uint16_t EpSize[] = { 0, 64, 512, 1024 };

#if (DirectConnect)
CyU3PDmaChannel glUSBtoUART_Handle;		// Handle needed for Bulk Out Endpoint
CyU3PDmaChannel glUARTtoUSB_Handle;		// Handle needed for Bulk In Endpoint

void UartCharsOutDmaCallback(CyU3PDmaChannel *chHandle, CyU3PDmaCbType_t type, CyU3PDmaCBInput_t *input)
{
    if (type == CY_U3P_DMA_CB_PROD_EVENT)
    {
    	CyU3PDmaChannelCommitBuffer(chHandle, input->buffer_p.count, 0);
    	CyU3PEventSet(&DisplayEvent, 1<<27, CYU3P_EVENT_OR);
    }
}

#else
CyU3PDmaChannel glUSBtoCPU_Handle;		// Handle needed for Bulk Out Endpoint
CyU3PDmaChannel glCPUtoUSB_Handle;		// Handle needed for Bulk In Endpoint
CyU3PDmaBuffer_t UserBuffer;			// Used for sending to EP Consumer

void GotCharactersFromHost(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaCBInput_t *Input)
{
    if (Type == CY_U3P_DMA_CB_PROD_EVENT)
    {
		uint8_t* BytePtr = Input->buffer_p.buffer;
		uint8_t* EndPtr = BytePtr + Input->buffer_p.count;
		*EndPtr = 0;
		// Shouldn't call DebugPrint in a callback but this is a special case
		DebugPrint(4, "\r\nReceived: %s", BytePtr);
		CyU3PDmaChannelDiscardBuffer(Handle);
    }
}

void SentCharacterToHost(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaCBInput_t *Input)
{
    if (Type == CY_U3P_DMA_CB_CONS_EVENT) CyU3PDmaChannelDiscardBuffer(Handle);
}

void SendCharacter(char InputChar)
{
	*UserBuffer.buffer = InputChar;
	UserBuffer.count = 1;
	UserBuffer.status = 0;
	CyU3PDmaChannelSetupSendBuffer(&glCPUtoUSB_Handle, &UserBuffer);
}
#endif

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
    uint16_t Size = EpSize[CyU3PUsbGetSpeed()];

    // Display the enumerated device bus speed
    DebugPrint(4, "\r\nRunning at %sSpeed", BusSpeed[CyU3PUsbGetSpeed()]);

    // Configure and enable the Consumer Endpoint
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
    epConfig.enable = CyTrue;
    epConfig.epType = CY_U3P_USB_EP_BULK;
    epConfig.burstLen = 1;
    epConfig.pcktSize = Size;
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
    CheckStatus("Setup Consumer Endpoint", Status);
    // Configure and enable the Producer Endpoint
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epConfig);
    CheckStatus("Setup Producer Endpoint", Status);
    // Configure and enable the Interrupt Endpoint
    epConfig.epType = CY_U3P_USB_EP_INTR;
    epConfig.pcktSize = 64;
    epConfig.isoPkts = 1;
    Status = CyU3PSetEpConfig(CY_FX_EP_INTERRUPT, &epConfig);
    CheckStatus("Setup Interrupt Endpoint", Status);
#if (DirectConnect)
    // Create an auto DMA channel between USB producer socket and UART Consumer
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = 16;
    dmaConfig.count = 4;
    dmaConfig.prodSckId = CY_FX_EP_PRODUCER_SOCKET;
    dmaConfig.consSckId = CY_U3P_LPP_SOCKET_UART_CONS;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    Status = CyU3PDmaChannelCreate(&glUSBtoUART_Handle, CY_U3P_DMA_TYPE_AUTO, &dmaConfig);
    CheckStatus("CreateUSBtoCPUdmaChannel", Status);
    // Set DMA Channel transfer size = infinite
    Status = CyU3PDmaChannelSetXfer(&glUSBtoUART_Handle, 0);
    CheckStatus("USBtoCPUdmaChannelSetXfer", Status);

    // Create a manual DMA channel between UART producer socket and USB Consumer
    // Use a smaller buffer so that gets filled in a shorter amount of time
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = 16;
    dmaConfig.count = 2;
    dmaConfig.prodSckId = CY_U3P_LPP_SOCKET_UART_PROD;
    dmaConfig.consSckId = CY_FX_EP_CONSUMER_SOCKET;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;

    dmaConfig.notification = CY_U3P_DMA_CB_PROD_EVENT;
    dmaConfig.cb = UartCharsOutDmaCallback;
    Status = CyU3PDmaChannelCreate(&glUARTtoUSB_Handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig);
    CheckStatus("CreateCPUtoUSBdmaChannel", Status);
    // Set DMA Channel transfer size = infinite
    Status = CyU3PDmaChannelSetXfer(&glUSBtoUART_Handle, 0);
    CheckStatus("USBtoCPUdmaChannelSetXfer", Status);
#else
    // Create a manual DMA channel between USB producer socket and CPU Consumer
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = 32;		// I assume a person is typing
    dmaConfig.count = 2;
    dmaConfig.prodSckId = CY_FX_EP_PRODUCER_SOCKET;
    dmaConfig.consSckId = CY_U3P_CPU_SOCKET_CONS;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    dmaConfig.notification = CY_U3P_DMA_CB_PROD_EVENT;
    dmaConfig.cb = GotCharactersFromHost;
    Status = CyU3PDmaChannelCreate(&glUSBtoCPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    CheckStatus("CreateUSBtoCPUdmaChannel", Status);
    // Set DMA Channel transfer size = infinite
    Status = CyU3PDmaChannelSetXfer(&glUSBtoCPU_Handle, 0);
    CheckStatus("USBtoCPUdmaChannelSetXfer", Status);

    // Create a manual DMA channel between CPU producer socket and USB Consumer
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size = 32;		// I assume a person is typing
    dmaConfig.count = 0;		// Don't assign any buffers here, will do manually
    dmaConfig.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaConfig.consSckId = CY_FX_EP_CONSUMER_SOCKET;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    dmaConfig.notification = CY_U3P_DMA_CB_CONS_EVENT;
    dmaConfig.cb = SentCharacterToHost;
    Status = CyU3PDmaChannelCreate(&glCPUtoUSB_Handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig);
    CheckStatus("CreateCPUtoUSBdmaChannel", Status);

    UserBuffer.buffer = CyU3PMemAlloc(32);
    if (UserBuffer.buffer == NULL) Status = CY_U3P_ERROR_MEMORY_ERROR;
    CheckStatus("Get UserBuffer", Status);
    UserBuffer.size = 32;
#endif
    glIsApplicationActive = CyTrue;			// Now ready to run!
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    glIsApplicationActive = CyFalse;

    // Close down and disable the endpoints then close the DMA channels
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
    CyU3PUsbFlushEp(CY_FX_EP_INTERRUPT);
    CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof (epConfig));
    Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
    CheckStatus("Disable Consumer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epConfig);
    CheckStatus("Disable Producer Endpoint", Status);
    Status = CyU3PSetEpConfig(CY_FX_EP_INTERRUPT, &epConfig);
    CheckStatus("Disable Interrupt Endpoint", Status);
#if (DirectConnect)
    Status = CyU3PDmaChannelDestroy(&glUSBtoUART_Handle);
    CheckStatus("Close USBtoUART DMA Channel", Status);
    Status = CyU3PDmaChannelDestroy(&glUARTtoUSB_Handle);
    CheckStatus("Close UARTtoUSB DMA Channel", Status);
#else
    Status = CyU3PDmaChannelDestroy(&glUSBtoCPU_Handle);
    CheckStatus("Close USBtoCPU DMA Channel", Status);
    Status = CyU3PDmaChannelDestroy(&glCPUtoUSB_Handle);
    CheckStatus("Close CPUtoUSB DMA Channel", Status);
    CyU3PMemFree(UserBuffer.buffer);
#endif
}

