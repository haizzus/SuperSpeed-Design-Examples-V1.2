
/* This file contains the externals used by the CDC Interface application. */

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define	DirectConnect				(0)

#define DebugPrint CyU3PDebugPrint

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

#define CLASS_REQUEST			(1)
#define SET_LINE_CODING			(0x20)
#define GET_LINE_CODING			(0x21)
#define	SET_CONTROL_LINE_STATE	(0x22)

#define CY_FX_EP_PRODUCER				(0x02)	// EP2 Out
#define CY_FX_EP_CONSUMER				(0x82)	// EP2 In
#define CY_FX_EP_INTERRUPT				(0x81)	// EP1 In
#define CY_FX_EP_PRODUCER_SOCKET		(CY_U3P_UIB_SOCKET_PROD_2)
#define CY_FX_EP_CONSUMER_SOCKET		(CY_U3P_UIB_SOCKET_CONS_2)
#define CY_FX_EP_INTR_CONSUMER_SOCKET	(CY_U3P_UIB_SOCKET_CONS_1)

/* Descriptor Types */
#define CY_FX_BOS_DSCR_TYPE             15
#define CY_FX_DEVICE_CAPB_DSCR_TYPE     16
#define CY_FX_SS_EP_COMPN_DSCR_TYPE     48


#include "cyu3externcend.h"


