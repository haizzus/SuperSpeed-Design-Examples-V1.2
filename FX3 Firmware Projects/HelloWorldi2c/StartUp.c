// Startup.c - set up the CPU environment then start the RTOS
//
// john@usb-by-example.com
//

#include "Application.h"

void IndicateError(uint16_t ErrorCode)
{
	// Setup a PWM to blink the DVK's only user LED at an "error rate"
    CyU3PGpioComplexConfig_t gpioConfig;
    // Try the console, it may not get through
    if (ErrorCode) CyU3PDebugPrint(1, "\r\nFatal Error = %d", ErrorCode);
	// LED is on UART_CTS which is currently been assigned to the UART driver, claim it back
    CyU3PDeviceGpioOverride(UART_CTS, CyFalse);
    // Configure UART_CTS as PWM output
    CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = CyTrue;
	gpioConfig.outValue = CyTrue;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    gpioConfig.pinMode = (ErrorCode == 0) ? CY_U3P_GPIO_MODE_STATIC : CY_U3P_GPIO_MODE_PWM;
    gpioConfig.timerMode = CY_U3P_GPIO_TIMER_HIGH_FREQ;
    gpioConfig.period = PWM_PERIOD << ErrorCode;
    gpioConfig.threshold = PWM_THRESHOLD << ErrorCode;
    CyU3PGpioSetComplexConfig(UART_CTS, &gpioConfig);
}

CyU3PReturnStatus_t InitGpioClocks(void)
{
	CyU3PReturnStatus_t Status;
    CyU3PGpioClock_t GpioClock;

    // Startup the GPIO module
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    return Status;
}

// Main sets up the CPU environment then starts the RTOS
int main (void)
{
    CyU3PIoMatrixConfig_t ioConfig;
    CyU3PReturnStatus_t Status;

 // Start with the default clock at 384 MHz
	Status = CyU3PDeviceInit(0);
	if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&ioConfig, 0, sizeof(ioConfig));
			ioConfig.isDQ32Bit = CyTrue;
			ioConfig.useUart   = true;
			ioConfig.useI2C    = CyTrue;
			ioConfig.lppMode   = CY_U3P_IO_MATRIX_LPP_DEFAULT;
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS)
			{
				Status = InitGpioClocks();	// Need GPIO clocks working for Error Indicator
				IndicateError(1);			// Turn on Error Indicator
				// One of the first things RTOS should do (in ApplicationDefine) is turn off ErrorIndicator
				CyU3PKernelEntry();			// Start RTOS, this does not return
			}
		}
	}

    while (1);			// Get here on a failure, can't recover, just hang here
						// Once the programs get more complex we shall do something more elegant here
	return 0;			// Won't get here but compiler wants this!
}
