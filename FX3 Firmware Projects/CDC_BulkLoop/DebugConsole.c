/*
 * DebugConsole.c
 *
 *      Author: john@USB-By-Example.com
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void IndicateError(uint16_t ErrorCode);
extern CyU3PDmaChannel glCDCtoCDC_Handle;
extern CyBool_t glIsApplicationActive;			// Set true once device is enumerated
extern uint16_t EpSize[];
extern const char* EventName[];

CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyU3PDmaChannel UARTtoCPU_Handle;		// Handle needed by Uart Callback routine
CyU3PDmaChannel CDCtoCPU_Handle;		// Handle needed when CDC is ConsoleIn
CyBool_t NeedToParseCommand;			// Detected in a Callback, process in Main context
CyU3PEvent DisplayEvent;				// Used to display events in the background

// For Debug and education display the name of the Event


static CyBool_t UsingUARTConsole;
static char ConsoleInBuffer[32];		// Buffer for user Console Input
static uint32_t ConsoleInIndex;			// Index into ConsoleIn buffer

CyU3PReturnStatus_t InitializeDebugConsoleIn(CyBool_t UsingUART);

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
		DebugPrint(4, "\nFound: '%s'", ThreadName);
	}
}

#if (ProfilingEnabled)
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
		DebugPrint(4, "\nThread: '%s' = %d%%", ThreadName, Usage);
		ThisThread = NextThread;
		tx_thread_info_get(ThisThread, &ThreadName, NULL, NULL, NULL, NULL, NULL, &NextThread, NULL);
	}
	DebugPrint(4, "\nTotal Drivers: = %d%%", CyU3PDeviceGetDriverLoad());
	DebugPrint(4, "\nTotal CPU: = %d%%", CyU3PDeviceGetCpuLoad());
}
#endif

void SwitchConsoles(void)
{
	CyU3PReturnStatus_t Status;
	// Only proceed if USB connection is up
	if (glIsApplicationActive)
	{
		// Tear down DMA channels that need to be reassigned
		if (UsingUARTConsole)
		{
			CyU3PDmaChannelDestroy(&glCDCtoCDC_Handle);
			CyU3PDmaChannelDestroy(&UARTtoCPU_Handle);
		}
		else CyU3PDmaChannelDestroy(&CDCtoCPU_Handle);
		// Switch console
		UsingUARTConsole = !UsingUARTConsole;
		DebugPrint(4, "Switching console to %s", UsingUARTConsole ? "UART" : "USB");
		CyU3PThreadSleep(100);		// Delay to allow message to get to the user
		// Disconnect the current console
		CyU3PDebugDeInit();
		CyU3PThreadSleep(100);		// Delay to allow Debug thread to complete and all buffers returned
		// Connect up the new Console out - this is simpler than the I2C case since the USB socket is an infinite sink
		Status = CyU3PDebugInit(UsingUARTConsole ? CY_U3P_LPP_SOCKET_UART_CONS : CY_FX_EP_CONSUMER_CDC_SOCKET, 8);
		CheckStatus("DebugInit", Status);
		CyU3PDebugPreamble(CyFalse);							// Skip preamble, debug info is targeted for a person
		// Say hello on the new console
		DebugPrint(4, "Console is now %s", UsingUARTConsole ? "UART" : "USB" );
		// Now connect up Console In
		Status = InitializeDebugConsoleIn(UsingUARTConsole);
		CheckStatus("InitializeDebugConsoleIn", Status);
		// Connect CDC_Loopback if necessary
	}
	else DebugPrint(4, "USB not active, cannot switch consoles\n");
}

void ParseCommand(void)
{
	NeedToParseCommand = CyFalse;
	DebugPrint(4, "\n");
	if (strcmp("reset", ConsoleInBuffer) == 0)
	{
		DebugPrint(4, "\nRESETTING CPU\n");
		CyU3PThreadSleep(100);
		CyU3PDeviceReset(CyFalse);
	}
	else if (strcmp("switch", ConsoleInBuffer) == 0) SwitchConsoles();
	else if (strcmp("threads", ConsoleInBuffer) == 0) DisplayThreads();
#if (ProfilingEnabled)
	else if (strcmp("usage", ConsoleInBuffer) == 0) DisplayUsage();
#endif
	else DebugPrint(4, "\nUnknown Command: '%s'\nAvailable commands:\n"
			"Reset Switch Threads Usage\n", ConsoleInBuffer);
	ConsoleInIndex = 0;
}

void ConsoleIn(char InputChar)
{
	DebugPrint(4, "%c", InputChar);			// Echo the character
	if (InputChar == 0x0d) NeedToParseCommand = CyTrue;
	else
	{
		ConsoleInBuffer[ConsoleInIndex] = InputChar | 0x20;		// Force lower case
		if (ConsoleInIndex++ < sizeof(ConsoleInBuffer)) ConsoleInBuffer[ConsoleInIndex] = 0;
		else ConsoleInIndex--;
	}
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
		ConsoleIn(InputChar);
		CyU3PDmaChannelDiscardBuffer(&UARTtoCPU_Handle);
		CyU3PUartRxSetBlockXfer(1);
    }
}

void CDC_CharsReceived(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaCBInput_t *Input)
{
	uint32_t i;
    if (Type == CY_U3P_DMA_CB_PROD_EVENT)
    {
    	for (i=0; i<Input->buffer_p.count; i++) ConsoleIn(*Input->buffer_p.buffer++);
		CyU3PDmaChannelDiscardBuffer(Handle);
    }
}


CyU3PReturnStatus_t InitializeDebugConsoleIn(CyBool_t UsingUART)
{
	CyU3PReturnStatus_t Status;
    CyU3PDmaChannelConfig_t dmaConfig;
    if (UsingUART)
    {
    	DebugPrint(4, "\nSetting up UART Console In");
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
    		Status = CyU3PDmaChannelSetXfer(&UARTtoCPU_Handle, 0);
    		CheckStatus("ConsoleInEnabled", Status);
        }
    }
    else
    {
    	DebugPrint(4, "\nSetting up USB_CDC Console In");
		CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
		dmaConfig.size  		= EpSize[CyU3PUsbGetSpeed()];
		dmaConfig.count 		= 2;
		dmaConfig.prodSckId		= CY_FX_EP_PRODUCER_CDC_SOCKET;
		dmaConfig.consSckId 	= CY_U3P_CPU_SOCKET_CONS;
		dmaConfig.dmaMode 		= CY_U3P_DMA_MODE_BYTE;
		dmaConfig.notification	= CY_U3P_DMA_CB_PROD_EVENT;
		dmaConfig.cb = CDC_CharsReceived;
		Status = CyU3PDmaChannelCreate(&CDCtoCPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
		CheckStatus("CreateCDC_ConsoleInDmaChannel", Status);
		if (Status != CY_U3P_SUCCESS) CyU3PDmaChannelDestroy(&CDCtoCPU_Handle);
		else
		{
			Status = CyU3PDmaChannelSetXfer(&CDCtoCPU_Handle, 0);
			CheckStatus("ConsoleInEnabled", Status);
		}
    }
	return Status;
}


// Spin up the UART DEBUG Console, Out and In
CyU3PReturnStatus_t InitializeDebugConsole(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t Status;

    Status = CyU3PUartInit();										// Start the UART driver
    CheckStatus("CyU3PUartInit", Status);

    CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyTrue;
	uartConfig.isDma    = CyTrue;
	Status = CyU3PUartSetConfig(&uartConfig, UartCallback);			// Configure the UART hardware
    CheckStatus("CyU3PUartSetConfig", Status);

    Status = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);					// Send as much data as I need to
    CheckStatus("CyU3PUartTxSetBlockXfer", Status);
	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 8);		// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) glDebugTxEnabled = CyTrue;
    CheckStatus("ConsoleOutEnabled", Status);
	CyU3PDebugPreamble(CyFalse);									// Skip preamble, debug info is targeted for a person
	UsingUARTConsole = CyTrue;
	Status = InitializeDebugConsoleIn(CyTrue);
    return Status;
}

void BackgroundPrint(uint32_t TimeToWait)
{
	uint32_t ActualEvents, i, Count, Limit;
	CyU3PReturnStatus_t Status;
	if (TimeToWait > 10)
	{
		// Check more often so that the events are better displayed in the order that they appeared
		Limit = TimeToWait/10;
		TimeToWait = 10;
	}
	else Limit = 1;
	// Check to see if any Events posted and, if they did, display them
	for (Count = 0; Count < Limit; Count++)
		{
		ActualEvents = 0;
		Status = CyU3PEventGet(&DisplayEvent, 0xFFFF, TX_OR_CLEAR, &ActualEvents, TimeToWait);
		if (Status != TX_NO_EVENTS)
		{
			for (i = 0; i<32; i++) if ((ActualEvents & (1<<i)) != 0) DebugPrint(4, "\nEvent(%d) received = '%s' ", i, EventName[i]);
		}
	}
	if (NeedToParseCommand) ParseCommand();
}
