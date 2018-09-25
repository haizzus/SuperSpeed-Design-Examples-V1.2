// This file contains the externals used by this example

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define APPLICATION_THREAD_STACKSIZE	(0x1000)    /* App thread stack size */
#define SPEEDY_THREAD_PRIORITY			(15)
#define SLOW_THREAD_PRIORITY    		(20)		/* App thread priority */

#define LED		(54)		// This is also UART_CTS
#define Button	(45)

// With these two defines you can use 'true' and 'false' as well as 'CyTrue' and 'CyFalse'
#define true	CyTrue
#define false	CyFalse

#include "cyu3externcend.h"
