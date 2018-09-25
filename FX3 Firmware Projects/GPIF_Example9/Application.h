
/* This file contains the externals and constants used by the application. */

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3pib.h"
#include "cyu3gpif.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3lpp.h"
#include "cyu3gpio.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3utils.h"
#include "cyu3externcstart.h"

#define DebugPrint	CyU3PDebugPrint

#define UART_CTS			(54)	// Also LED
#define UART_RTS			(53)	// Won't be used with UART so this gives me an extra GPIO to use
#define PUSH_BUTTON			(45)	// This example checks the PushButton and debounces it for the CPLD
#define CPLD_RESET			(27)	// Hold CPLD in RESET until I've set up and am ready to run
#define CPLD_PUSH_BUTTON	(26)	// Debounced PushButton
#define CPLD_LastRDData		(20)	// CPLD needs to tell me the last data on a READ

#define MAXCLOCKVALUE		(10)	// For CPLD board, with a -7 XC2C128 the maximum frequency with margin is 800/MAXCLOCKVALUE = 80MHz
#define DEBOUNCE_TIME		(20)	// 20 msec

// Define Event Flag values
// First 22 Events are defined in cyu3usb.h at "typedef enum CyU3PUsbEventType_t"
#define ANY_EVENT				(0xFFFFFFFF)
#define USB_EVENTS				(0x7FFFFF)
#define USER_COMMAND_AVAILABLE	(1<<23)
#define CPLD_WRITE_XFER_DONE	(1<<24)

// Define constants for blinking Error LED
#define PWM_PERIOD 			(20000000)	// Approximately 10Hz
#define PWM_THRESHOLD		( 5000000)	// On for 25% of the time

#define STANDARD_REQUEST	(0)			// My values are not shifted
#define CLASS_REQUEST		(1)
#define VENDOR_REQUEST		(2)

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

#define USB_CONSUMER_ENDPOINT			(0x81)    /* EP 1 IN */
#define USB_CONSUMER_ENDPOINT_SOCKET	(CY_U3P_UIB_SOCKET_CONS_1)
#define USB_PRODUCER_ENDPOINT			(0x02)    /* EP 2 OUT */
#define USB_PRODUCER_ENDPOINT_SOCKET	(CY_U3P_UIB_SOCKET_PROD_2)
#define CPU_PRODUCER_SOCKET				(CY_U3P_CPU_SOCKET_PROD)

#define GPIF_PRODUCER_SOCKET			(CY_U3P_PIB_SOCKET_2)
#define GPIF_CONSUMER_SOCKET			(CY_U3P_PIB_SOCKET_3)

/* Burst length in 1 KB packets. Only applicable to USB 3.0. */
#define ENDPOINT_BURST_LENGTH	(16)

/* DMA buffers used by the application. */
#define DMA_BUFFER_SIZE			(16384)
#define DMA_BUFFER_COUNT		(4)

#include "cyu3externcend.h"


