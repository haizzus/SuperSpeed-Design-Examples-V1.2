/*
 * StartUp.c
 *
 *  Created on: Feb 18, 2014
 *      Author: John
 */

#include "Application.h"

extern void ApplicationThread_Entry (uint32_t Value);
extern CyU3PThread ApplicationThread;

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status = CY_U3P_SUCCESS;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate (&ApplicationThread, // Handle to my Application Thread
            "11:HelloWorld",                		// Thread ID and name
            ApplicationThread_Entry,     			// Thread entry function
            42,                             		// Parameter passed to Thread
            StackPtr,                       		// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,               // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,            // Thread priority
            APPLICATION_THREAD_PRIORITY,            // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            		// Time slice no supported
            CYU3P_AUTO_START                		// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS)
    {
        /* Thread creation failed with the Status = error code */

        /* Add custom recovery or debug actions here */

        /* Application cannot continue. Loop indefinitely */
        while(1);
    }
}


// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    Status = CyU3PDeviceInit(NULL);		// Setup with default clock parameters
    if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&io_cfg, 0, sizeof(io_cfg));
//r			io_cfg.isDQ32Bit = CyFalse;
//r			io_cfg.s0Mode 	 = CY_U3P_SPORT_INACTIVE;
//r			io_cfg.s1Mode	 = CY_U3P_SPORT_INACTIVE;
			io_cfg.useUart   = CyTrue;
//r			io_cfg.useI2C    = CyFalse;
//r			io_cfg.useI2S    = CyFalse;
//r			io_cfg.useSpi    = CyFalse;
			io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
//r			io_cfg.gpioSimpleEn[0]  = 0;
//r			io_cfg.gpioSimpleEn[1]  = 0;
//r			io_cfg.gpioComplexEn[0] = 0;
//r			io_cfg.gpioComplexEn[1] = 0;
			Status = CyU3PDeviceConfigureIOMatrix(&io_cfg);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}
    // Get here on a failure, can't recover, just hang here
    while (1);
	return 0;				// Won't get here but compiler wants this!
}


