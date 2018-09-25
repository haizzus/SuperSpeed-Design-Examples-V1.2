/*
 * StartUp.c	Setup the CPU environment and start RTOS
 *
 */

#include "Application.h"

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
//r			io_cfg.gpioSimpleEn[0]  = 0;							// No GPIO pins used in this example
//r			io_cfg.gpioSimpleEn[1]  = 0;
//r			io_cfg.gpioComplexEn[0] = 0;
//r			io_cfg.gpioComplexEn[1] = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&io_cfg);			// Setup IO to use UART only
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();		// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;				// Won't get here but compiler wants this!
}


