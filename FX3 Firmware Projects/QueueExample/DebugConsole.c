/*
 * DebugConsole.c
 *
 *  Created on: Jun 14, 2014
 *      Author: John
 */

#include "Application.h"

CyBool_t glDebugTxEnabled = CyFalse;	// Set true once I can output messages to the Console

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
			CyU3PThreadSleep (1000);
		}
	}
}

// Spin up the DEBUG ConsoleOut on UART
CyU3PReturnStatus_t InitializeDebugConsole(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t Status;

    Status = CyU3PUartInit();										// Start the UART driver
    CheckStatus(4, "CyU3PUartInit", Status);							// This won't display since ConsoleOut is not up yet!

    CyU3PMemSet((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
//r	uartConfig.parity   = CY_U3P_UART_NO_PARITY;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyTrue;
//r	uartConfig.flowCtrl = CyFalse;
	uartConfig.isDma    = CyTrue;
	Status = CyU3PUartSetConfig(&uartConfig, 0);					// Configure the UART hardware
    CheckStatus(4, "CyU3PUartSetConfig", Status);

    Status = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);					// Send as much data as I need to
    CheckStatus(4, "CyU3PUartTxSetBlockXfer", Status);

	Status = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 7);		// Attach the Debug driver above the UART driver
	if (Status == CY_U3P_SUCCESS) glDebugTxEnabled = CyTrue;		// ConsoleOut is now operational :-)
	CyU3PDebugPreamble(CyFalse);									// Skip preamble, debug info is targeted for a person

    return Status;
}

