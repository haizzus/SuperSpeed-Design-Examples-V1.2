/*
 * DebugConsole.c
 *
 *  Created on: Jun 14, 2014
 *      Author: John
 */

#include "Application.h"

extern uint32_t SampleTime;				// Time between data collections in Input Thread

#define THREADLISTMAX (17)				// Allow for 16 threads
CyU3PThread *ThreadList[THREADLISTMAX];	// Used to attach an Entry/Exit callback to toggle IOs
CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyU3PDmaChannel glUARTtoCPU_Handle;		// Handle needed by Uart Callback routine
char glConsoleInBuffer[32];				// Buffer for user Console Input
uint32_t glConsoleInIndex;				// Index into ConsoleIn buffer

void CheckStatus(uint8_t DisplayLevel, char* StringPtr, CyU3PReturnStatus_t Status)
// In this initial debugging stage I stall on an un-successful system call, else I display progress
// Note that this assumes that there were no errors bringing up the Debug Console
{
	if (glDebugTxEnabled)				// Need to wait until ConsoleOut is enabled
	{
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PDebugPrint(DisplayLevel, "\r\n%s Successful", StringPtr);
			return;
		}
		// else hang here
		CyU3PDebugPrint(4, "\r\n%s failed, Status = %d\r\n", StringPtr, Status);
		while (1)
		{
			CyU3PDebugPrint(4, "?");
			CyU3PThreadSleep(1000);
		}
	}
}

CyU3PReturnStatus_t ReassignGPIFpins(void)
{
    CyU3PGpioSimpleConfig_t gpioConfig;
    CyU3PGpioClock_t GpioClock;
    CyU3PReturnStatus_t Status, OverallStatus;
    uint8_t i;

    // Startup the GPIO module in case it is not already running
	GpioClock.fastClkDiv = 2;
	GpioClock.slowClkDiv = 0;
	GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
	GpioClock.clkSrc = CY_U3P_SYS_CLK;
	GpioClock.halfDiv = 0;
	Status = CyU3PGpioInit(&GpioClock, 0);
	if (Status == CY_U3P_ERROR_ALREADY_STARTED) Status = CY_U3P_SUCCESS;
	CheckStatus(4, "Start GPIO clocks", Status);
    OverallStatus = Status;
    // Now reassign DQ[0..15] to be SimpleIO pins that I can toggle
    for (i=0; i<THREADLISTMAX-1; i++)
    {
    	OverallStatus |= Status = CyU3PDeviceGpioOverride(i, CyTrue);			// DQ[0..15] = GPIO[0..15]
		CheckStatus(8, "Disconnect DQ pin from GPIF interface", Status);
		gpioConfig.outValue = CyFalse;
		gpioConfig.inputEn = CyFalse;
		gpioConfig.driveLowEn = CyTrue;
		gpioConfig.driveHighEn = CyTrue;
		gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
		OverallStatus |= Status = CyU3PGpioSetSimpleConfig(i, &gpioConfig);
		CheckStatus(8, "Setup DQpin as Simple Output", Status);
    }
    return OverallStatus;
}

void ToggleIOpin(CyU3PThread *Thread, UINT Condition)
{
	uint8_t i;
	// If GPIF has been turned on and DQ[0..15] are no longer simple IO then return
	CyU3PGpioSetValue(0, 0);
	if (CyU3PIsGpioSimpleIOConfigured(0) == CyFalse) return;
	// Match the thread from the table created then toggle the matching IO pin
	for (i=0; i<Elements(ThreadList); i++)
	{
		if (ThreadList[i] == 0) return;		// The thread was not in the list!
		if (Thread == ThreadList[i])
		{
			CyU3PGpioSetValue(i, (Condition == TX_THREAD_ENTRY));
			return;
		}
	}
	CyU3PGpioSetValue(0, 1);
}

void DisplayThreads(void)
{
	char* ThreadName;
	UINT Priority[THREADLISTMAX];	// Compiler insisted on UINT here!
	UINT Status;
	uint8_t Count;
	uint8_t Swapped = 1;
	uint8_t i = 1;
	// First find out who I am
	ThreadList[0] = CyU3PThreadIdentify();
	tx_thread_info_get(ThreadList[0], &ThreadName, NULL, NULL, (UINT*)&Priority[0], NULL, NULL, &ThreadList[1], NULL);
	// Now, using the Thread linked list, look for other threads until I find myself again
	// Put the threads in a table since I will sort them
	while (ThreadList[i] != ThreadList[0])
	{
		tx_thread_info_get(ThreadList[i], &ThreadName, NULL, NULL, (UINT*)&Priority[i], NULL, NULL, &ThreadList[i+1], NULL);
		if (i++ > Elements(ThreadList)) break;
	}
	// Since I want minimum impact on the system, order the list according to thread priorities. Use a bubble sort
	for (i=0; i<THREADLISTMAX-1; i++) if (ThreadList[i] == 0) break;
	Count = i - 2;
	while (Swapped)
	{
		Swapped = 0;
		for (i=0; i<Count; i++)
		{
			if (Priority[i+1] < Priority[i])
			{
				Swapped = 1;	// Need to swap two entries, use last entry as a temporary value
				Priority[THREADLISTMAX-1] = Priority[i]; Priority[i] = Priority[i+1]; Priority[i+1] = Priority[THREADLISTMAX-1];
				ThreadList[THREADLISTMAX-1] = ThreadList[i]; ThreadList[i] = ThreadList[i+1]; ThreadList[i+1] = ThreadList[THREADLISTMAX-1];
			}
		}
	}
	// Now register a callback that will toggle IO pins as threads are running
	Status = ReassignGPIFpins();
	if (Status != CY_U3P_SUCCESS) CyU3PDebugPrint(4, "\r\nSorry, could not assign IO pins");
	else
	{
		CyU3PDebugPrint(4, "\r\nIO pins allocated to threads as follows:");
		for (i=0; i<(Count+1); i++)
		{
			tx_thread_info_get(ThreadList[i], &ThreadName, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			Status = tx_thread_entry_exit_notify(ThreadList[i], ToggleIOpin);
			if (Status == 0) CyU3PDebugPrint(4, "\r\nDQ[%d] = %s", i, ThreadName);
		}
	}
}

CyBool_t ASCII_Digit(char Char)
{
	return ((Char >= '0') && (Char <= '9'));
}

uint32_t GetValue(char* CharPtr)
{
	uint32_t Value = 0;
	while (ASCII_Digit(*CharPtr)) Value = (10*Value) + (*CharPtr++ - '0');
//	CyU3PDebugPrint(4, "\r\nValue set to %d", Value);
	return Value;
}

void UartCallback(CyU3PUartEvt_t Event, CyU3PUartError_t Error)
// Handle characters typed in by the developer
// Look for and execute commands on a <CR>
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
			if (strncmp("set", glConsoleInBuffer, 3) == 0) SampleTime = GetValue(&glConsoleInBuffer[3]);
			else if (!strcmp("reset", glConsoleInBuffer))
			{
				CyU3PDebugPrint(4, "\r\nRESETTING CPU\r\n");
				CyU3PThreadSleep(100);
				CyU3PDeviceReset(CyFalse);
			}
			else if (!strcmp("threads", glConsoleInBuffer)) DisplayThreads();
			else CyU3PDebugPrint(4, "\r\nUnknown Command: '%s'", &glConsoleInBuffer[0]);
			glConsoleInIndex = 0;
		}
		else
		{
			glConsoleInBuffer[glConsoleInIndex] = InputChar | 0x20;		// Force lower case
			if (glConsoleInIndex++ < sizeof(glConsoleInBuffer)) glConsoleInBuffer[glConsoleInIndex] = 0;
			else glConsoleInIndex--;
		}
		CyU3PDmaChannelDiscardBuffer(&glUARTtoCPU_Handle);
		CyU3PUartRxSetBlockXfer(1);
    }
}

// Spin up the DEBUG ConsoleOut on UART
CyU3PReturnStatus_t InitializeDebugConsole(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status;

    Status = CyU3PUartInit();										// Start the UART driver
    CheckStatus(4, "CyU3PUartInit", Status);						// This won't display since ConsoleOut is not up yet!

    CyU3PMemSet((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
//r	uartConfig.parity   = CY_U3P_UART_NO_PARITY;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyTrue;
//r	uartConfig.flowCtrl = CyFalse;
	uartConfig.isDma    = CyTrue;
	Status = CyU3PUartSetConfig(&uartConfig, UartCallback);			// Configure the UART hardware
    CheckStatus(4, "CyU3PUartSetConfig", Status);

    Status = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);					// Send as much data as I need to
    CheckStatus(4, "CyU3PUartTxSetBlockXfer", Status);

	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 7);		// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) glDebugTxEnabled = CyTrue;		// ConsoleOut is now operational :-)
	CyU3PDebugPreamble(CyFalse);									// Skip preamble, debug info is targeted for a person
	// Now setup a DMA channel to receive characters from the Uart Rx
    Status = CyU3PUartRxSetBlockXfer(1);
    CheckStatus(4, "CyU3PUartRxSetBlockXfer", Status);
	CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
	dmaConfig.size  		= 16;									// Minimum size allowed, I only need 1 byte
	dmaConfig.count 		= 1;									// I can't type faster than the Uart Callback routine!
	dmaConfig.prodSckId		= CY_U3P_LPP_SOCKET_UART_PROD;
	dmaConfig.consSckId 	= CY_U3P_CPU_SOCKET_CONS;
	dmaConfig.dmaMode 		= CY_U3P_DMA_MODE_BYTE;
//	dmaConfig.notification	= CY_U3P_DMA_CB_PROD_EVENT;
//	dmaConfig.cb = DmaCallback;
	Status = CyU3PDmaChannelCreate(&glUARTtoCPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    CheckStatus(4, "CreateDebugRxDmaChannel", Status);
    if (Status != CY_U3P_SUCCESS) CyU3PDmaChannelDestroy(&glUARTtoCPU_Handle);
    else
    {
		Status = CyU3PDmaChannelSetXfer(&glUARTtoCPU_Handle, 0);
		CheckStatus(4, "ConsoleInEnabled", Status);
    }

    return Status;
}

