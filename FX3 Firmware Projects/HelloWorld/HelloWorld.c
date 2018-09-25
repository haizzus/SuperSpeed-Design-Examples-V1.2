// HelloWorld.c - starter project to get used to the tools
//
// john@usb-by-example.com
//


#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3uart.h"
#include "HelloWorld.h"

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyU3PDmaChannel glUARTtoCPU_Handle;		// Handle needed by Uart Callback routine
char glConsoleInBuffer[32];				// Buffer for user Console Input
uint32_t glConsoleInIndex;				// Index into ConsoleIn buffer

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

void ApplicationThread_Entry (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    Status = InitializeDebugConsole();
    if (Status == CY_U3P_SUCCESS)
    {
    	CyU3PDebugPrint(4, "\r\nApplication started with %d\r\n", Value);
    	// Now run forever
    	while (1)
    	{
    		CyU3PThreadSleep (1000);
			CyU3PDebugPrint(4, "%d, ", Seconds++);
		}
    }
    CyU3PDebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status = CY_U3P_SUCCESS;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate (&ApplicationThread, // Handle to my Application Thread
            "11:HelloWorld",                		// Thread ID and name
            ApplicationThread_Entry,     			// Thread entry function
            42,                             		// Parameter passed to Thread
            StackPtr,                       		// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,               // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,            // Thread priority
            APPLICATION_THREAD_PRIORITY,            // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            		// Time slice no supported
            CYU3P_AUTO_START                		// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS)
    {
        /* Thread creation failed with the Status = error code */

        /* Add custom recovery or debug actions here */

        /* Application cannot continue. Loop indefinitely */
        while(1);
    }
}


// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PIoMatrixConfig_t ioConfig;
    CyU3PSysClockConfig_t clockConfig;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

 // The default clock parameters run the CPU at 384 MHz, change the setup to 403.2MHz so that I can use a 100MHz GPIF (later)
    clockConfig.setSysClk400 = CyTrue;
    clockConfig.cpuClkDiv = 2;
	clockConfig.dmaClkDiv = 2;
	clockConfig.mmioClkDiv = 2;
	clockConfig.useStandbyClk = CyFalse;
	clockConfig.clkSrc = CY_U3P_SYS_CLK;
	Status = CyU3PDeviceInit (&clockConfig);
	if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&ioConfig, 0, sizeof(ioConfig));
//r			ioConfig.isDQ32Bit = CyFalse;
//r			ioConfig.s0Mode 	 = CY_U3P_SPORT_INACTIVE;
//r			ioConfig.s1Mode	 = CY_U3P_SPORT_INACTIVE;
			ioConfig.useUart   = CyTrue;
//r			ioConfig.useI2C    = CyFalse;
//r			ioConfig.useI2S    = CyFalse;
//r			ioConfig.useSpi    = CyFalse;
			ioConfig.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
//r			ioConfig.gpioSimpleEn[0]  = 0;
//r			ioConfig.gpioSimpleEn[1]  = 0;
//r			ioConfig.gpioComplexEn[0] = 0;
//r			ioConfig.gpioComplexEn[1] = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;				// Won't get here but compiler wants this!
}


