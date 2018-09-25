
/* This file contains the externals used by the Hello World application. */

#ifndef _INCLUDED_HELLOWORLD_H_
#define _INCLUDED_HELLOWORLD_H_

#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3externcstart.h"

#define APPLICATION_THREAD_STACK       (0x0800)    /* App thread stack size */
#define APPLICATION_THREAD_PRIORITY    (8)         /* App thread priority */

#define LED		(54)		// This is also UART_CTS
#define Button	(45)


#include "cyu3externcend.h"

#endif /* _INCLUDED_HELLOWORLD_H_ */

