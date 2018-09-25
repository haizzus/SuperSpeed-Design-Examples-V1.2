/*
 * DebugConsole.c
 *
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void IndicateError(uint16_t ErrorCode);
extern CyU3PReturnStatus_t I2C_DebugPrint(uint8_t Priority, char* Message, ...);
extern void SelectSPI_Device(uint32_t DeviceID);
extern CyU3PReturnStatus_t ConfigureSPI(uint8_t Mode);
extern uint32_t Swap4Bytes(uint32_t Value);

CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console
CyU3PDmaChannel glUARTtoCPU_Handle;		// Handle needed by Uart Callback routine
char ConsoleInBuffer[32];				// Buffer for user Console Input
uint32_t ConsoleInIndex;				// Index into ConsoleIn buffer
CyBool_t CommandEntered;				// Check for commands and run them in Main context
CyU3PDmaChannel SpiReadHandle, SpiWriteHandle;
CyBool_t DmaChannelsSetup;
CyU3PDmaBuffer_t SPI_Buffer_p;

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

void SetupSPI_DMA_Channels(void)
{
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status;
    // Allocate buffer at run time
    CyU3PMemSet ((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size      = SPIFlash_PageSize;
    dmaConfig.count     = 0;
    dmaConfig.dmaMode   = CY_U3P_DMA_MODE_BYTE;
    dmaConfig.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaConfig.consSckId = CY_U3P_LPP_SOCKET_SPI_CONS;
    Status = CyU3PDmaChannelCreate (&SpiWriteHandle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig);
    CheckStatus("SPI Write Channel", Status);
    dmaConfig.prodSckId = CY_U3P_LPP_SOCKET_SPI_PROD;
    dmaConfig.consSckId = CY_U3P_CPU_SOCKET_CONS;
    Status = CyU3PDmaChannelCreate (&SpiReadHandle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    SPI_Buffer_p.buffer = CyU3PMemAlloc(SPIFlash_PageSize);
    SPI_Buffer_p.size = SPIFlash_PageSize;
}

void DisplaySPIBlock(uint32_t Block)
{
	CyU3PReturnStatus_t Status;
	uint32_t i;
	DebugPrint(4, "\r\nDisplaying Block[%d]", Block);
	// Need 4 byte 'header' for a read = 0x03 + Addr[23:16] + Addr[15:8] + Addr[7:0]
	uint32_t SPI_Command_Address = ((Swap4Bytes(Block))>>8) | 0x03;
	if (SPI_Buffer_p.buffer == 0) SetupSPI_DMA_Channels();
	// Send this header using Register mode
	SelectSPI_Device(SPI_Flash);
	ConfigureSPI(RegisterMode);
	Status = CyU3PSpiTransmitWords((uint8_t*)&SPI_Command_Address, 4);
	// Read the block using DMA mode
	ConfigureSPI(DMA_Mode);
	Status = CyU3PSpiSetBlockXfer(0, SPIFlash_PageSize);
	CheckStatus("SpiSetBlockXfer", Status);
	Status = CyU3PDmaChannelSetupRecvBuffer(&SpiReadHandle, &SPI_Buffer_p);
	CheckStatus("SetupRecvBuffer", Status);
	Status = CyU3PDmaChannelWaitForCompletion(&SpiReadHandle, CY_FX_USB_SPI_TIMEOUT);
	CheckStatus("WaitForCompletion", Status);
	SelectSPI_Device(-1);
	Status = CyU3PSpiDisableBlockXfer (CyFalse, CyTrue);	// Needed since next command will be register mode
	CheckStatus("SpiDisableBlockXfer", Status);
	DebugPrint(4, "\r\nRead %d bytes", SPI_Buffer_p.count);
	for (i = 0; i<SPIFlash_PageSize; i++)
	{
		if ((i & 15) == 0) DebugPrint(4, "\r\n%x: ", i);
		DebugPrint(4, "%x ", *SPI_Buffer_p.buffer++);
	}
}

void ParseCommand(void)
{
	DebugPrint(4, "\r\n");
	if (ConsoleInBuffer[0] == 'd') DisplaySPIBlock(GetValue(&ConsoleInBuffer[1]));
	else if (strcmp("reset", ConsoleInBuffer) == 0)
	{
		DebugPrint(4, "\r\nRESETTING CPU\r\n");
		CyU3PThreadSleep(500);
		CyU3PDeviceReset(CyFalse);
	}
	else if (strcmp("threads", ConsoleInBuffer) == 0) DisplayThreads();
	else if (strncmp("error", ConsoleInBuffer, 5) == 0) IndicateError(GetValue(&ConsoleInBuffer[5]));
	else DebugPrint(4, "\r\nUnknown Command: '%s'\r\nAvailable commands:\r\n"
			"Reset Threads\r\n", ConsoleInBuffer);
	ConsoleInIndex = 0;
	CommandEntered = CyFalse;
}

void CheckForCommand(void)
{
	// Since ParseCommand often does DebugPrint ensure that it is called from Main context
	if (CommandEntered) ParseCommand();
}

void GotConsoleInput(uint8_t Source, char InputChar)
{
	DebugPrint(4, "%c", InputChar);							// This will echo to both consoles
															// 'Source' was for DEBUG
	if (InputChar == 0x0d) CommandEntered = CyTrue;			// Post a command to be processed
	else
	{
		ConsoleInBuffer[ConsoleInIndex] = InputChar | 0x20;		// Force lower case
		if (ConsoleInIndex++ < sizeof(ConsoleInBuffer)) ConsoleInBuffer[ConsoleInIndex] = 0;
		else ConsoleInIndex--;
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
		GotConsoleInput(0, InputChar);
		CyU3PDmaChannelDiscardBuffer(&glUARTtoCPU_Handle);
		CyU3PUartRxSetBlockXfer(1);
    }
}


CyU3PReturnStatus_t InitializeDebugConsole(uint8_t TraceLevel)
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
	Status = CyU3PUartSetConfig(&uartConfig, UartCallback);				// Configure the UART hardware
    CheckStatus("CyU3PUartSetConfig", Status);

    Status = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);						// Send as much data as I need to
    CheckStatus("CyU3PUartTxSetBlockXfer", Status);

	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, TraceLevel);	// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) glDebugTxEnabled = CyTrue;
    CheckStatus("ConsoleOutEnabled", Status);
	CyU3PDebugPreamble(CyFalse);										// Skip preamble, debug info is targeted for a person

	return Status;

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
