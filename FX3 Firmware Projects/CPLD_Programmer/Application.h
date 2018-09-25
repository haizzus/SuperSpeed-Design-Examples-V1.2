// This file contains the constants used by the application

#include "cyu3types.h"
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3gpio.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usbconst.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3externcstart.h"

#define VERBOSE		1	// Set to 1 to get more descriptions

#define DebugPrint CyU3PDebugPrint

// Define event flags to synchronize the modules
#define USB_CONNECTION_ACTIVE		1
#define DATA_BUFFER_AVAILABLE		2
#define DATA_BUFFER_PROCESSED		3

#define UART_CTS	(54)

// Define GPIOs used to bit bang JTAG interface of CPLD
#define JTAG_TCK	(50)
#define JTAG_TMS	(57)
#define JTAG_TDO	(51)
#define JTAG_TDI	(52)

#define APPLICATION_THREAD_STACK	(0x1000)
#define APPLICATION_THREAD_PRIORITY	(8)

// Define endpoints that the Control Panel can use
#define EP_PRODUCER				(0x01)    /* EP 1 OUT */
#define EP_CONSUMER				(0x81)    /* EP 1 IN */
#define EP_PRODUCER_SOCKET 		(CY_U3P_UIB_SOCKET_PROD_1)
#define EP_CONSUMER_SOCKET		(CY_U3P_UIB_SOCKET_CONS_1)
#define CPU_CONSUMER_SOCKET		(CY_U3P_CPU_SOCKET_CONS)
#define DMA_BUFFER_COUNT		(8)
// Burst length in 1 KB packets. Only applicable to USB 3.0.
#define ENDPOINT_BURST_LENGTH	(16)
#define INFINITE_TRANSFER_SIZE	(0)

// Define constants for blinking Error LED
#define PWM_PERIOD 				(20000000)   // Approximately 10Hz
#define PWM_THRESHOLD			( 5000000)   // On for 25% of the time

#include "cyu3externcend.h"
