// This file contains the externals used by this example

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3externcstart.h"

#define APPLICATION_THREAD_STACKSIZE   (0x0800)    /* App thread stack size */
#define APPLICATION_THREAD_PRIORITY    (8)         /* App thread priority */

#define LED		(54)		// This is also UART_CTS
#define Button	(45)

#include "cyu3externcend.h"
