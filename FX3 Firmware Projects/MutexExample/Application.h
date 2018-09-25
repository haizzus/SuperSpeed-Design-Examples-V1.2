
// This file contains the externals used by MutexExample


#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3uart.h"
#include "cyu3types.h"
#include "cyu3externcstart.h"

#define APPLICATION_THREAD_STACK	(0x1000)    /* App thread stack size */
#define SPEEDY_THREAD_PRIORITY		(15)
#define SLOW_THREAD_PRIORITY    	(20)		/* App thread priority */

#include "cyu3externcend.h"
