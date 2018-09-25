/*
 * StartUp.c - this is a 'housekeeping' file that changes infrequently with each project
 *
 *
 */

#include "Application.h"


// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PSysClockConfig_t ClockConfig;
    CyU3PIoMatrixConfig_t io_Config;
    CyU3PReturnStatus_t Status;

    // The default clock runs at 384MHz, adjust it up to 403MHz so that GPIF can be "100MHz"
    ClockConfig.setSysClk400  = CyTrue;
    ClockConfig.cpuClkDiv     = 2;
    ClockConfig.dmaClkDiv     = 2;
    ClockConfig.mmioClkDiv    = 2;
    ClockConfig.useStandbyClk = CyFalse;
    ClockConfig.clkSrc        = CY_U3P_SYS_CLK;
    Status = CyU3PDeviceInit(&ClockConfig);
    if (Status == CY_U3P_SUCCESS)
    {
        {
			Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);	// Icache, Dcache, DMAcache
			if (Status == CY_U3P_SUCCESS)
			{
				CyU3PMemSet((uint8_t *)&io_Config, 0, sizeof(io_Config));
// Now spin up 32bit GPIF and the UART
				io_Config.isDQ32Bit = CyTrue;
				io_Config.useUart   = CyTrue;
				io_Config.lppMode   = CY_U3P_IO_MATRIX_LPP_DEFAULT;
				Status = CyU3PDeviceConfigureIOMatrix(&io_Config);
				if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();		// Start RTOS, this does not return
			}
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;				// Won't get here but compiler wants this!
}


