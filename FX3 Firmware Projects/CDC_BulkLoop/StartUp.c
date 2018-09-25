/*
 * StartUp.c
 *
 *      Author: john@USB-By-Example.com
 */

#include "Application.h"


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
			ioConfig.useUart   = CyTrue;
			ioConfig.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();			// Start RTOS, this does not return
		}
	}

    while (1);			// Get here on a failure, can't recover, just hang here
						// Once the programs get more complex we shall do something more elegant here
	return 0;			// Won't get here but compiler wants this!
}
