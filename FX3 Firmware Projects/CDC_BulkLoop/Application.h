
/* This file contains the externals used by the CDC Interface application. */

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3usb.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3utils.h"
#include "cyu3externcstart.h"

#define DebugPrint CyU3PDebugPrint

#define UART_CTS	(54)

// Define constants for blinking Error LED
#define PWM_PERIOD 				(20000000)   // Approximately 10Hz
#define PWM_THRESHOLD			( 5000000)   // On for 25% of the time

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

#define CLASS_REQUEST			(1)
#define SET_LINE_CODING			(0x20)
#define GET_LINE_CODING			(0x21)
#define	SET_CONTROL_LINE_STATE	(0x22)

// Endpoint and socket definitions for the virtual COM port
#define CY_FX_EP_PRODUCER_CDC           	(0x02)	// EP 2 OUT
#define CY_FX_EP_CONSUMER_CDC           	(0x82)	// EP 2 IN
#define CY_FX_EP_INTERRUPT_CDC          	(0x83)	// EP 3 INTR
#define CY_FX_EP_PRODUCER_CDC_SOCKET    	(CY_U3P_UIB_SOCKET_PROD_2)
#define CY_FX_EP_CONSUMER_CDC_SOCKET    	(CY_U3P_UIB_SOCKET_CONS_2)
#define CY_FX_EP_INTERRUPT_CDC_SOCKET   	(CY_U3P_UIB_SOCKET_CONS_3)

// Endpoint and socket definitions for BulkLoop Interface
#define CY_FX_EP_PRODUCER_BULKLOOP			(0x01)	// EP1 Out
#define CY_FX_EP_CONSUMER_BULKLOOP			(0x81)	// EP1 In
#define CY_FX_EP_PRODUCER_BULKLOOP_SOCKET	(CY_U3P_UIB_SOCKET_PROD_1)
#define CY_FX_EP_CONSUMER_BULKLOOP_SOCKET	(CY_U3P_UIB_SOCKET_CONS_1)
#define CY_FX_BULKLOOP_DMA_BUFFER_COUNT		(8)

/* Descriptor Types */
#define CY_FX_BOS_DSCR_TYPE             15
#define CY_FX_DEVICE_CAPB_DSCR_TYPE     16
#define CY_FX_SS_EP_COMPN_DSCR_TYPE     48


#include "cyu3externcend.h"


