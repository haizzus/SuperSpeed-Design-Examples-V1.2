// DebugConsole.c	include the Cypress UART-based Debug Console into the project

#include "Application.h"

extern uint32_t SampleTime;						// Application variable that we need to change
extern CyU3PThread ThreadHandle[APP_THREADS];	// Handles to my Application Threads
extern void *StackPtr[APP_THREADS];				// Stack allocated to each thread
//extern CyU3PSemaphore SemaphoreHandle[2];		// Used for thread communications
extern CyU3PSemaphore DataToProcess, DataToOutput;	// Used for thread communications

extern void IndicateError(uint16_t ErrorCode);

static CyBool_t DebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
static CyU3PDmaChannel UARTtoCPU_Handle;	// Handle needed by Uart Callback routine
static char ConsoleInBuffer[32];			// Buffer for user Console Input
static uint32_t ConsoleInIndex;				// Index into ConsoleIn buffer

struct { uint32_t Type; uint32_t ID; uint32_t GPIO; } RTOS_Trace[16] = {
	{ TRACE_THREAD, 0, 33, }, { TRACE_THREAD, 10, 34, }, { TRACE_THREAD, 11, 35, }, { TRACE_THREAD, 12, 36, },
	{ TRACE_SEMAPHORE, 0, 37, }, { TRACE_SEMAPHORE, 1, 38, }, { 0, 0, 39, }, { 0, 0, 40, },
	{ 0, 0, 41, }, { 0, 0, 42, }, { 0, 0, 43, }, { 0, 0, 44, },
	{ 0, 0, 46, }, { 0, 0, 47, }, { 0, 0, 48, }, { 0, 0, 49, } };


void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status)
// In this initial debugging stage I stall on an un-successful system call, else I display progress
// Note that this assumes that there were no errors bringing up the Debug Console
{
	if (DebugTxEnabled)				// Need to wait until ConsoleOut is enabled
	{
		if (Status == CY_U3P_SUCCESS)
		{
			DebugPrint(4, "\r\n%s Successful", StringPtr);
			return;
		}
		// else hang here
		DebugPrint(4, "\r\n%s failed, Status = %d\r\n", StringPtr, Status);
		while (1)
		{
			DebugPrint(4, "?");
			CyU3PThreadSleep (1000);
		}
	}
}

CyU3PThread* FindThread(uint32_t ID)
{
	// Extract the thread number from the thread name
	CyU3PThread *ThisThread, *NextThread, *StartingThread;
	char* ThreadName;
	uint32_t ThreadNumber;
	// First find out who I am
	StartingThread = ThisThread = CyU3PThreadIdentify();
	tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	// Now, using the Thread linked list, look for other threads until I find myself again
	while (NextThread != StartingThread)
	{
		// Process ThisThread
		ThreadNumber = (*ThreadName++ - '0') * 10;
		ThreadNumber += (*ThreadName-- - '0');
		DebugPrint(8, "\r\nFound: '%s', %X, %d", ThreadName, ThisThread, ThreadNumber);
		if (ThreadNumber == ID) return ThisThread;
		// Now get NextThread
		ThisThread = NextThread;
		tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	}
	return 0;
}

CyU3PReturnStatus_t SetupTrace(uint32_t Index)
{
	CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
	CyU3PThread* Thread;
	switch (RTOS_Trace[Index].Type)
	{
		case 0: break;				// Do nothing
		case TRACE_THREAD:
			Thread = FindThread(RTOS_Trace[Index].ID);
			if (Thread)
			{
				DebugPrint(8, "\r\nThread = %X is using GPIO = %d", Thread, RTOS_Trace[Index].GPIO);
				Status = CyU3PThreadSetActivityGpio(Thread, RTOS_Trace[Index].GPIO);
				CheckStatus("Register thread monitoring GPIO", Status);
			}
			break;
		case TRACE_SEMAPHORE:
//		    Status = CyU3PSemaphoreSetActivityGpio(&SemaphoreHandle[RTOS_Trace[Index].ID], RTOS_Trace[Index].GPIO);
		    if (RTOS_Trace[Index].ID == 0) Status = CyU3PSemaphoreSetActivityGpio(&DataToProcess, RTOS_Trace[Index].GPIO);
		    if (RTOS_Trace[Index].ID == 1) Status = CyU3PSemaphoreSetActivityGpio(&DataToOutput, RTOS_Trace[Index].GPIO);
		    CheckStatus("Register semaphore monitoring GPIO", Status);
			break;
		default: DebugPrint(4, "\r\nInvalid Type in SetupTrace = %d", RTOS_Trace[Index].Type);
			break;
	}
	return Status;
}

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

void DisplayThreads(void)
{
	CyU3PThread *ThisThread, *NextThread;
	char* ThreadName;
	// First find out who I am
	ThisThread = CyU3PThreadIdentify();
	tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	// Now, using the Thread linked list, look for other threads until I find myself again
	while (NextThread != ThisThread)
	{
		tx_thread_info_get(NextThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
		DebugPrint(4, "\r\nFound: '%s'", ThreadName);
	}
}

void DisplayUsage(void)
{
	CyU3PThread *ThisThread, *NextThread, *StartingThread;
	char* ThreadName;
	uint32_t Usage;
	// First find out who I am
	StartingThread = ThisThread = CyU3PThreadIdentify();
	tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	// Now, using the Thread linked list, look for other threads until I find myself again
	while (NextThread != StartingThread)
	{
		Usage = CyU3PDeviceGetThreadLoad(ThisThread);
		DebugPrint(4, "\r\nThread: '%s' = %d%%", ThreadName, Usage);
		ThisThread = NextThread;
		tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	}
	DebugPrint(4, "\r\nTotal Drivers: = %d%%", CyU3PDeviceGetDriverLoad());
	DebugPrint(4, "\r\nTotal CPU: = %d%%", CyU3PDeviceGetCpuLoad());
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


void ParseCommand(void)
{
	DebugPrint(4, "\r\n");
	if (strncmp("set", ConsoleInBuffer, 3) == 0)
		{
			SampleTime = GetValue(&ConsoleInBuffer[3]);
			DebugPrint(4, "\r\nSet SampleTime = %d", SampleTime);
		}
	else if (strcmp("reset", ConsoleInBuffer) == 0)
	{
		DebugPrint(4, "\r\nRESETTING CPU\r\n");
		CyU3PThreadSleep(100);
		CyU3PDeviceReset(CyFalse);
	}
	else if (strcmp("threads", ConsoleInBuffer) == 0) DisplayThreads();
	else if (strcmp("stacks", ConsoleInBuffer) == 0) DisplayStacks();
	else if (strcmp("usage", ConsoleInBuffer) == 0) DisplayUsage();
	else if (strncmp("error", ConsoleInBuffer, 5) == 0) IndicateError(GetValue(&ConsoleInBuffer[5]));
	else DebugPrint(4, "\r\nUnknown Command: '%s'\r\nAvailable commands:\r\n"
			"Reset Set Stacks Threads Usage\r\n", ConsoleInBuffer);
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

