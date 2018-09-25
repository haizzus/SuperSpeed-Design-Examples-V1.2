/*
 * StartUp.c	Setup the CPU environment and start RTOS
 *
 */

#include "Application.h"

void IndicateError(uint16_t ErrorCode)
{
	// Setup a PWM to blink the DVK's only user LED at an "error rate"
    CyU3PGpioComplexConfig_t gpioConfig;
	// LED is on UART_CTS which is currently been assigned to the UART driver, claim it back
    CyU3PDeviceGpioOverride(UART_CTS, CyFalse);
    // Configure UART_CTS as PWM output
    CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
	gpioConfig.outValue = CyFalse;
//r	gpioConfig.inputEn = CyFalse;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    gpioConfig.pinMode = (ErrorCode == 0) ? CY_U3P_GPIO_MODE_STATIC : CY_U3P_GPIO_MODE_PWM;
//r	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
    gpioConfig.timerMode = CY_U3P_GPIO_TIMER_HIGH_FREQ;
//r	gpioConfig.timer = 0;
    gpioConfig.period = PWM_PERIOD << ErrorCode;
    gpioConfig.threshold = PWM_THRESHOLD << ErrorCode;
    CyU3PGpioSetComplexConfig(UART_CTS, &gpioConfig);
}


// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t Status;

    Status = CyU3PDeviceInit(NULL);									// Setup with default clock parameters
    if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);	// Setup with both caches enabled
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&io_cfg, 0, sizeof(io_cfg));		// This clears the io_cfg structure
//			io_cfg.isDQ32Bit = CyTrue;
//r			io_cfg.s0Mode 	 = CY_U3P_SPORT_INACTIVE;
//	//r means that this line is commented out since it is not required, but I left it in so that it is clear what is going on
//r			io_cfg.s1Mode	 = CY_U3P_SPORT_INACTIVE;
			io_cfg.useUart   = CyTrue;
//r			io_cfg.useI2C    = CyFalse;
//r			io_cfg.useI2S    = CyFalse;
//r			io_cfg.useSpi    = CyFalse;
			io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;		// This will be our debug console
//r			io_cfg.gpioSimpleEn[0]  = 0;							// Use I2S pins as JTAG output pins in this example
			io_cfg.gpioSimpleEn[1]  = 0x021c0000;					// Allocate GPIO's 57,52,51 and 50
//r			io_cfg.gpioComplexEn[0] = 0;
//r			io_cfg.gpioComplexEn[1] = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&io_cfg);			// Setup IO to use UART only
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();		// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    IndicateError(1);
    while (1);
	return 0;				// Won't get here but compiler wants this!
}


