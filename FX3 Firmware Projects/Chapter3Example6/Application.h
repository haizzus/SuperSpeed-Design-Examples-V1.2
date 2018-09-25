// This file contains the externals used by this example

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define Elements(X)	sizeof(X)/sizeof(X[0])

#define APPLICATION_THREAD_STACK	 (0x800)    /* App thread stack size */
#define OUTPUT_DATA_THREAD_PRIORITY		(15)
#define PROCESS_DATA_THREAD_PRIORITY   	(20)	/* App thread priority */
#define INPUT_DATA_THREAD_PRIORITY		(15)

// Define values for event flags
#define INPUT_DATA_AVAILABLE		(1 << 0)
#define PROCESSED_DATA_AVAILABLE	(1 << 1)

// With these two defines you can use 'true' and 'false' as well as 'CyTrue' and 'CyFalse'
#define true	CyTrue
#define false	CyFalse

#include "cyu3externcend.h"
