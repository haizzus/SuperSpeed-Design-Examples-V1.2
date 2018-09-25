// StartUp.c - set up the CPU environment then start the RTOS
//
// john@usb-by-example.com
//

#include "Application.h"

void IndicateError(uint16_t ErrorCode)
{
	// Setup a PWM to blink the SuperSpeed Explorer's only user LED at an "error rate"
    CyU3PGpioComplexConfig_t gpioConfig;
	// LED is on UART_CTS which has been assigned to the UART driver, claim it back
    CyU3PDeviceGpioOverride(UART_CTS, CyFalse);
    // ConFigure UART_CTS as PWM output
    CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = CyTrue;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    gpioConfig.pinMode = (ErrorCode == 0) ? CY_U3P_GPIO_MODE_STATIC : CY_U3P_GPIO_MODE_PWM;
    gpioConfig.timerMode = CY_U3P_GPIO_TIMER_HIGH_FREQ;
    gpioConfig.period = PWM_PERIOD << ErrorCode;
    gpioConfig.threshold = PWM_THRESHOLD << ErrorCode;
    CyU3PGpioSetComplexConfig(UART_CTS, &gpioConfig);
    // Last ditch effort to tell the user, it may not get through
    if (ErrorCode) DebugPrint(1, "\r\nFatal Error = %d", ErrorCode);
}

// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PGpioClock_t GpioClock;
    CyU3PIoMatrixConfig_t io_Config;
    CyU3PReturnStatus_t Status;

    // The default clock runs at 384MHz
    Status = CyU3PDeviceInit(0);
    if (Status == CY_U3P_SUCCESS)
    {
        // Startup the GPIO module clocks, needed for ErrorIndicator
        GpioClock.fastClkDiv = 2;
        GpioClock.slowClkDiv = 0;
        GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
        GpioClock.clkSrc = CY_U3P_SYS_CLK;
        GpioClock.halfDiv = 0;
        Status = CyU3PGpioInit(&GpioClock, 0);
        if (Status == CY_U3P_SUCCESS)
        {
			Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
			if (Status == CY_U3P_SUCCESS)
			{
				CyU3PMemSet((uint8_t *)&io_Config, 0, sizeof(io_Config));
				io_Config.isDQ32Bit = CyTrue;
				io_Config.useUart   = CyTrue;
				io_Config.lppMode   = CY_U3P_IO_MATRIX_LPP_DEFAULT;
				Status = CyU3PDeviceConfigureIOMatrix(&io_Config);
				IndicateError(1);		// Turn on so we know if RTOS Start fails
				if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();// This does not return
			}
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;				// Won't get here but compiler wants this!
}

