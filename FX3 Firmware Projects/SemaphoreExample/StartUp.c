//	StartUp.c - used by all applications to set up the CPU environment and start the RTOS
//
//	john@usb-by-example.com
//

#include "Application.h"

int main (void)
{
    CyU3PIoMatrixConfig_t ioConfig;
    CyU3PReturnStatus_t Status;

// Start CPU with a default clock (384MHz), caches enabled and just the UART enabled
	Status = CyU3PDeviceInit(0);
	if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&ioConfig, 0, sizeof(ioConfig));
//r			ioConfig.isDQ32Bit	= CyFalse;
//r			ioConfig.s0Mode 	= CY_U3P_SPORT_INACTIVE;
//r			ioConfig.s1Mode		= CY_U3P_SPORT_INACTIVE;
			ioConfig.useUart	= CyTrue;
//r			ioConfig.useI2C		= CyFalse;
//r			ioConfig.useI2S		= CyFalse;
//r			ioConfig.useSpi		= CyFalse;
			ioConfig.lppMode	= CY_U3P_IO_MATRIX_LPP_UART_ONLY;
//r			ioConfig.gpioSimpleEn[0]  = 0;
//r			ioConfig.gpioSimpleEn[1]  = 0;
//r			ioConfig.gpioComplexEn[0] = 0;
//r			ioConfig.gpioComplexEn[1] = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
	// Once the programs become more complex we shall do something more elegant here!
    while (1);
	return 0;				// Won't get here but compiler wants this!
}

