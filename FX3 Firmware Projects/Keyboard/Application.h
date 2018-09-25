
/* This file contains the externals used by the Keyboard application. */

#ifndef _INCLUDED_KEYBOARD_H_
#define _INCLUDED_KEYBOARD_H_

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define DebugPrint CyU3PDebugPrint
#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

#define CY_FX_EP_PRODUCER			(0x02)	// EP2 Out
#define CY_FX_EP_CONSUMER			(0x82)	// EP2 In
#define CY_FX_EP_PRODUCER_SOCKET	(CY_U3P_UIB_SOCKET_PROD_2)
#define CY_FX_EP_CONSUMER_SOCKET	(CY_U3P_UIB_SOCKET_CONS_2)
#define CY_FX_CPU_PRODUCER_SOCKET	(CY_U3P_CPU_SOCKET_PROD)
#define CY_FX_CPU_CONSUMER_SOCKET	(CY_U3P_CPU_SOCKET_CONS)

#define CLASS_REQUEST		(1)
#define HID_SET_IDLE		(10)
#define HID_SET_REPORT		(9)
#define HID_GET_REPORT		(1)
#define REPORT_SIZE			(8)

#include "cyu3externcend.h"

#endif /* _INCLUDED_KEYBOARD_H_ */

