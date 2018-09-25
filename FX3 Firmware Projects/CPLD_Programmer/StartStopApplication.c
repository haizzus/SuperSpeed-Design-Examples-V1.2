/*
 * StartStopApplication.c - this is where resources such as Endpoints and DMA channels, required by the Application
 * 							are created and deleted in response to events from USB
 *
 *  Created on: Feb 18, 2014
 *      Author: John
 */

#include "Application.h"

// Declare external data
extern CyU3PEvent glApplicationEvent;		// An Event to allow threads to communicate

CyU3PDmaCBInput_t* glDmaBufferDescriptor;

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

const char* BusSpeed[] = { "Not Connected ", "Full ", "High ", "Super" };
const uint16_t EpSize[] = { 0, 64, 512, 1024 };

CyU3PDmaChannel glDmaChannelHandle;
//uint8_t* glDmaBufferPtr;
//uint16_t glDmaBufferLength;

CyU3PReturnStatus_t InitializeGPIO(void)
{
	// Setup GPIO pins to bit-bang JTAG interface of CPLD
	CyU3PReturnStatus_t Status;
    CyU3PGpioClock_t GpioClock;
    CyU3PGpioSimpleConfig_t GpioConfig;

    // Startup the GPIO module
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    CheckStatus("\r\nStart GPIO Module", Status);

    // Configure pins as outputs or inputs with no interrupts
    CyU3PMemSet((uint8_t *)&GpioConfig, 0, sizeof(GpioConfig));
    GpioConfig.outValue = 1;
//r	GpioConfig.inputEn = CyFalse;
    GpioConfig.driveLowEn = CyTrue;
    GpioConfig.driveHighEn = CyTrue;
//r	GpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
    Status = CyU3PGpioSetSimpleConfig(JTAG_TCK, &GpioConfig);
    Status |= CyU3PGpioSetSimpleConfig(JTAG_TMS, &GpioConfig);
    Status |= CyU3PGpioSetSimpleConfig(JTAG_TDI, &GpioConfig);
	GpioConfig.inputEn = CyTrue;
	GpioConfig.driveLowEn = CyFalse;
	GpioConfig.driveHighEn = CyFalse;
    Status |= CyU3PGpioSetSimpleConfig(JTAG_TDO, &GpioConfig);
	return Status;
}

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
	CyU3PReturnStatus_t Status;
    CyU3PEpConfig_t EpConfig;
    CyU3PDmaChannelConfig_t DmaConfig;
    uint16_t USBSpeed;

	// Display the enumerated device bus speed
    USBSpeed = CyU3PUsbGetSpeed();
    DebugPrint(4, "\r\nRunning at %sSpeed", BusSpeed[USBSpeed]);

    Status = InitializeGPIO();
    CheckStatus("\r\nInitialize GPIO", Status);

    // Declare two endpoints so that the device looks like a BulkLoopBack device
    CyU3PMemSet((uint8_t *)&EpConfig, 0, sizeof(EpConfig));
    EpConfig.enable = CyTrue;
    EpConfig.epType = CY_U3P_USB_EP_BULK;
    EpConfig.burstLen = (USBSpeed == CY_U3P_SUPER_SPEED) ? (ENDPOINT_BURST_LENGTH) : 1;
    EpConfig.pcktSize = EpSize[USBSpeed];
    Status = CyU3PSetEpConfig(EP_PRODUCER, &EpConfig);
    CheckStatus("Configure Producer Endpoint", Status);
    Status = CyU3PUsbFlushEp(EP_PRODUCER);
    CheckStatus("Flush Producer Endpoint", Status);
    Status = CyU3PSetEpConfig(EP_CONSUMER, &EpConfig);
    CheckStatus("Configure Consumer Endpoint", Status);
    Status = CyU3PUsbFlushEp(EP_CONSUMER);
    CheckStatus("Flush Consumer Endpoint", Status);

    // Data will move from Producer EP to CPU then optionally to Consumer EP
    CyU3PMemSet((uint8_t *)&DmaConfig, 0, sizeof(DmaConfig));
    DmaConfig.size = EpConfig.pcktSize * EpConfig.burstLen;
    DmaConfig.count = DMA_BUFFER_COUNT;
    DmaConfig.prodSckId = EP_PRODUCER_SOCKET;
    DmaConfig.consSckId = EP_CONSUMER_SOCKET;
    DmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    Status = CyU3PDmaChannelCreate(&glDmaChannelHandle, CY_U3P_DMA_TYPE_MANUAL, &DmaConfig);
    CheckStatus("Create DMA Channel", Status);
    Status = CyU3PDmaChannelSetXfer(&glDmaChannelHandle, INFINITE_TRANSFER_SIZE);
    CheckStatus("Start DMA Channel", Status);

    // Now signal the Application so that it can run
    CyU3PEventSet(&glApplicationEvent, USB_CONNECTION_ACTIVE, CYU3P_EVENT_OR);
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
	CyU3PReturnStatus_t Status;
    CyU3PEpConfig_t EpConfig;

    DebugPrint(4, "\r\nStopping application");

    // Signal the Application so that it can stop
    CyU3PEventSet(&glApplicationEvent, ~USB_CONNECTION_ACTIVE, CYU3P_EVENT_AND);

    // Disable the IOs used
	Status = CyU3PGpioDisable(JTAG_TCK);
	Status |= CyU3PGpioDisable(JTAG_TMS);
	Status |= CyU3PGpioDisable(JTAG_TDO);
	Status |= CyU3PGpioDisable(JTAG_TDI);
	CheckStatus("Disable IOs", Status);

    // Close down and disable the endpoints then close the DMA channel
    CyU3PMemSet((uint8_t *)&EpConfig, 0, sizeof(EpConfig));
    CyU3PUsbFlushEp(EP_PRODUCER);
    Status = CyU3PSetEpConfig(EP_PRODUCER, &EpConfig);
    CheckStatus("Close Producer Endpoint", Status);
    CyU3PUsbFlushEp(EP_CONSUMER);
    Status = CyU3PSetEpConfig(EP_CONSUMER, &EpConfig);
    CheckStatus("Close Consumer Endpoint", Status);

    CyU3PDmaChannelDestroy(&glDmaChannelHandle);
}

