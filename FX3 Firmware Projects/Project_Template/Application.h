// This file contains the constants used by the application

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define USB_CONNECTION_ACTIVE		 1			// An Event Flag

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

#include "cyu3externcend.h"
