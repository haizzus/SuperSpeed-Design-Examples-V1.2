// This file contains the externals used by this example

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3utils.h"
#include "cyu3externcstart.h"

#define Elements(X)	sizeof(X)/sizeof(X[0])

#define DebugPrint CyU3PDebugPrint				// Use Cypress UART routines for now

#define A_LONG_TIME					 (10000)	// 10 seconds is "a long time" for this example
#define APP_THREADS						 (3)
#define APPLICATION_THREAD_STACK	(0x0400)    /* App thread stack size */
#define INPUT_DATA_THREAD_PRIORITY		(15)
#define PROCESS_DATA_THREAD_PRIORITY   	(15)	/* App thread priority */
#define OUTPUT_DATA_THREAD_PRIORITY		(15)

#define INFINITE_TRANSFER_SIZE			 (0)

#define UART_CTS						(54)

// Define constants for blinking Error LED
#define PWM_PERIOD 				(20000000)   // Approximately 10Hz
#define PWM_THRESHOLD			( 5000000)   // On for 25% of the time

// With these two defines you can use 'true' and 'false' as well as 'CyTrue' and 'CyFalse'
#define true	CyTrue
#define false	CyFalse

#include "cyu3externcend.h"
