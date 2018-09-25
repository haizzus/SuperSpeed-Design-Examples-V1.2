// DebugConsole.c	include the Cypress UART-based Debug Console into the project

#include "Application.h"

extern uint32_t SampleTime;						// Application variable that we need to change
extern CyU3PThread ThreadHandle[APP_THREADS];	// Handles to my Application Threads
extern void *StackPtr[APP_THREADS];				// Stack allocated to each thread
extern void  IndicateError(uint16_t ErrorCode);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyBool_t DebugTxEnabled = CyFalse;			// Set true once I can output messages to the Console
static CyU3PDmaChannel UARTtoCPU_Handle;	// Handle needed by Uart Callback routine
static char ConsoleInBuffer[32];			// Buffer for user Console Input
static uint32_t ConsoleInIndex;				// Index into ConsoleIn buffer

CyBool_t ASCII_Digit(char Char)
{
	return ((Char >= '0') && (Char <= '9'));
}

uint32_t GetValue(char* CharPtr)
{
	uint32_t Value = 0;
	while (ASCII_Digit(*CharPtr)) Value = (10*Value) + (*CharPtr++ - '0');
	return Value;
}

void DisplayStacks(void)
{
	int i, j;
	char* ThreadName;
	for (i = 0; i<APP_THREADS; i++)
	{
		// Note that StackSize is in bytes but RTOS fill pattern is a uint32
		uint32_t* StackStartPtr = StackPtr[i];
		uint32_t* DataPtr = StackStartPtr;
		for (j = 0; j<APPLICATION_THREAD_STACK>>2; j++) if (*DataPtr++ != 0xEFEFEFEF) break;
		CyU3PThreadInfoGet(&ThreadHandle[i], &ThreadName, 0, 0, 0);
		ThreadName += 3;	// Skip numeric ID
		DebugPrint(4, "\r\nStack free in %s is %d/%d", ThreadName, (DataPtr - StackStartPtr)<<2, APPLICATION_THREAD_STACK);
	}
	DebugPrint(4, "\r\n");
}

void DisplayThreads(void)
{
	CyU3PThread *ThisThread, *NextThread;
	char* ThreadName;
	// First find out who I am
	ThisThread = CyU3PThreadIdentify();
	tx_thread_info_get(ThisThread, &ThreadName, 0, 0, 0, 0, 0, &NextThread, 0);
	// Now, using the Thread linked list, look for other threads until I find myself again
	while (NextThread != ThisThread)
	{
		tx_thread_info_get(NextThread, &ThreadName, 0, 0, 0, 0, 0, &NextThread, 0);
		DebugPrint(4, "\r\nFound: '%s'", ThreadName);
	}
}

void ParseCommand(void)
{
	DebugPrint(4, "\r\n");
	if (strncmp("set", ConsoleInBuffer, 3) == 0)
	{
		SampleTime = GetValue(&ConsoleInBuffer[3]);
		DebugPrint(4, "\r\nSet SampleTime = %d", SampleTime);
	}
	else if (strcmp("threads", ConsoleInBuffer) == 0) DisplayThreads();
	else if (strcmp("stacks", ConsoleInBuffer) == 0) DisplayStacks();
	else if (strncmp("error", ConsoleInBuffer, 5) == 0) IndicateError(GetValue(&ConsoleInBuffer[5]));
	else if (strcmp("reset", ConsoleInBuffer) == 0)
	{
		DebugPrint(4, "\r\nRESETTING CPU\r\n");
		CyU3PThreadSleep(100);
		CyU3PDeviceReset(CyFalse);
	}
	else DebugPrint(4, "\r\nUnknown Command: '%s'\r\nAvailable commands:\r\n"
			"Reset Set Stacks Threads\r\n", ConsoleInBuffer);
	ConsoleInIndex = 0;
}

void UartCallback(CyU3PUartEvt_t Event, CyU3PUartError_t Error)
// Handle characters typed in by the developer
// Look for and execute commands on a <CR>
{
	CyU3PDmaBuffer_t ConsoleInDmaBuffer;
	char InputChar;
	if (Event == CY_U3P_UART_EVENT_RX_DONE)
    {
		CyU3PDmaChannelSetWrapUp(&UARTtoCPU_Handle);
		CyU3PDmaChannelGetBuffer(&UARTtoCPU_Handle, &ConsoleInDmaBuffer, CYU3P_NO_WAIT);
		InputChar = (char)*ConsoleInDmaBuffer.buffer;
		DebugPrint(4, "%c", InputChar);			// Echo the character
		if (InputChar == 0x0d) ParseCommand();
		else
		{
			ConsoleInBuffer[ConsoleInIndex] = InputChar | 0x20;		// Force lower case
			if (ConsoleInIndex++ < sizeof(ConsoleInBuffer)) ConsoleInBuffer[ConsoleInIndex] = 0;
			else ConsoleInIndex--;
		}
		CyU3PDmaChannelDiscardBuffer(&UARTtoCPU_Handle);
		CyU3PUartRxSetBlockXfer(1);
    }
}

// Spin up the DEBUG Console, Out and In
CyU3PReturnStatus_t InitializeDebugConsole(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status;

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

	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 6);		// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) DebugTxEnabled = CyTrue;
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
	Status = CyU3PDmaChannelCreate(&UARTtoCPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    CheckStatus("CreateDebugRxDmaChannel", Status);
    if (Status != CY_U3P_SUCCESS) CyU3PDmaChannelDestroy(&UARTtoCPU_Handle);
    else
    {
		Status = CyU3PDmaChannelSetXfer(&UARTtoCPU_Handle, INFINITE_TRANSFER_SIZE);
		CheckStatus("ConsoleInEnabled", Status);
    }
    return Status;
}
