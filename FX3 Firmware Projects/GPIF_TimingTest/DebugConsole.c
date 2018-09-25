/*
 * DebugConsole.c
 *
 *      Author: john@usb-by-example.com
 */

#include "Application.h"

extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t StartGPIF(void);

CyU3PEvent CallbackEvent;				// Used for events detected in CallBack routines
CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyU3PDmaChannel glUARTtoCPU_Handle;		// Handle needed by Uart Callback routine
char ConsoleInBuffer[32];				// Buffer for user Console Input
uint32_t ConsoleInIndex;				// Index into ConsoleIn buffer
uint32_t glCounter[20];					// Counters used to monitor GPIF
uint32_t ClockValue;					// Used to Set/Display GPIF clock
uint8_t Toggle;

// For Debug and education display the name of the Event
const char* EventName[] = {
	    "CONNECT", "DISCONNECT", "SUSPEND", "RESUME", "RESET", "SET_CONFIGURATION", "SPEED",
	    "SET_INTERFACE", "SET_EXIT_LATENCY", "SOF_ITP", "USER_EP0_XFER_COMPLETE", "VBUS_VALID",
	    "VBUS_REMOVED", "HOSTMODE_CONNECT", "HOSTMODE_DISCONNECT", "OTG_CHANGE", "OTG_VBUS_CHG",
	    "OTG_SRP", "EP_UNDERRUN", "LINK_RECOVERY", "USB3_LINKFAIL", "SS_COMP_ENTRY", "SS_COMP_EXIT"
};

void DebugPrintEvent(uint32_t ActualEvents)
{
	uint32_t Index = 0;
	while (ActualEvents)
	{
		if (ActualEvents & 1) DebugPrint(4, "\r\nEvent received = %s", EventName[Index]);
		Index++;
		ActualEvents >>= 1;
	}
}

void ParseCommand(void)
{
	// User has entered a command, process it
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    if (strncmp("pclk", ConsoleInBuffer, 4) == 0)
	{
	    CyU3PPibClock_t pibClock;
		if (ConsoleInBuffer[4] == '-') ClockValue++;
		if ((ConsoleInBuffer[4] == '+') && (ClockValue > 0)) ClockValue--;
		CyU3PGpifDisable(CyFalse);			// Stop GPIF engine
	    Status = CyU3PPibDeInit();			// Turn off GPIF so that I can start it again
	    pibClock.clkDiv = (ClockValue & 0xFFFE) + 4;
	    pibClock.clkSrc = CY_U3P_SYS_CLK;
	    pibClock.isHalfDiv = (ClockValue & 1);
	    pibClock.isDllEnable = CyFalse;		// Disable Dll since this application is synchronous
	    Status = CyU3PPibInit(CyTrue, &pibClock);
		CheckStatus("\r\nChange GPIF Clock", Status);
		Status = StartGPIF();
		CheckStatus("GpifRestart", Status);
		DebugPrint(4, "\r\nGPIF Clock = %d MHz = %d MBps\r\n", 400/(ClockValue + 4), 1600/(ClockValue + 4));
	}
	else if (!strcmp("threads", ConsoleInBuffer))
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
	else if (!strcmp("reset", ConsoleInBuffer))
	{
		DebugPrint(4, "\r\nRESETTING CPU\r\n");
		CyU3PThreadSleep(100);
		CyU3PDeviceReset(CyFalse);
	}
	else if (!strcmp("cpld", ConsoleInBuffer))
	{
		CyBool_t State;
		CyU3PGpioSimpleGetValue(CPLD_RESET, &State);
		State = !State;
		CyU3PGpioSetValue(CPLD_RESET, State);
		DebugPrint(4, "\r\nToggle CPLD RESET, now = %c\r\n", State ? '1' : '0');
	}
	else if (!strcmp("gpif", ConsoleInBuffer))
	{
		uint8_t State = 0xFF;
		Status = CyU3PGpifGetSMState(&State);
		DebugPrint(4, "\r\nGPIF State = %d\r\n", State);
	}
	else DebugPrint(4, "Input: '%s'\r\n", &ConsoleInBuffer[0]);
	ConsoleInIndex = 0;
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
		DebugPrint(4, "%c", InputChar);			// Echo the character
		// On CR, signal Main loop to process the command entered by the user.  Should NOT do this in a CallBack routine
		if (InputChar == 0x0d) CyU3PEventSet(&CallbackEvent, USER_COMMAND_AVAILABLE, CYU3P_EVENT_OR);
		else
		{
			ConsoleInBuffer[ConsoleInIndex] = InputChar;
			if (ConsoleInIndex++ < sizeof(ConsoleInBuffer)) ConsoleInBuffer[ConsoleInIndex] = 0;
			else ConsoleInIndex--;
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

