
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3error.h"
#include "cyu3types.h"
#include "cyu3gpio.h"
#include "cyu3uart.h"
#include "cyu3i2c.h"
#include "cyu3spi.h"
#include "cyu3utils.h"
#include "cyu3externcstart.h"

#define Elements(X)		(sizeof(X)/sizeof(X[0]))

#define DebugPrint I2C_DebugPrint

#define APP_THREADS						 (3)
#define APPLICATION_THREAD_STACK	(0x0400)
#define APPLICATION_THREAD_PRIORITY	    (15)

#define SPI_Address0				(25)
#define SPI_Address1				(26)
#define CPLD_RESET					(27)
#define SPIFlash_PageSize			(256)		// (1024)
#define CY_FX_USB_SPI_TIMEOUT       (5000)

enum SPI_DeviceID {
	SPI_Flash,
	SwitchesLEDs,
	UserPort,
	I2C_Control
};
enum SPI_Mode {
	RegisterMode,
	DMA_Mode
};

#define CY_FX_USBI2C_I2C_BITRATE	(400000)
#define I2C_BufferSize				(64)
#define CY7C65215_DeviceAddress		(0x42)
#define REGISTER_MODE				(0)
#define CY_FX_CPU_PRODUCER_SOCKET	(CY_U3P_CPU_SOCKET_PROD)
#define CY_FX_CPU_CONSUMER_SOCKET	(CY_U3P_CPU_SOCKET_CONS)

#define DEBUG_THREAD_STACK_SIZE	    (0x800)
#define DEBUG_THREAD_PRIORITY    	(6)
#define I2C_CONSOLEIN_BUFFER_SIZE	(16)		// Only need 1 but 16 is minimum allocation

#define UART_CTS	(54)

// Define constants for blinking Error LED
#define PWM_PERIOD 				(20000000)   // Approximately 10Hz
#define PWM_THRESHOLD			( 5000000)   // On for 25% of the time

// With these two defines you can use 'true' and 'false' as well as 'CyTrue' and 'CyFalse'
#define true	CyTrue
#define false	CyFalse

#define I2C_RETRY_COUNT         (20)
#define I2C_READ_SIZE           (64)


#include "cyu3externcend.h"
