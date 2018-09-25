//	Demonstrate GPIF sending CPLD data to USB
//	Use CollectData to save the CPLD generated data (it removes CPLD Reset, then reasserts it)
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void DebugPrintEvent(uint32_t ActualEvents);
extern void ParseCommand(void);


CyU3PEvent CallbackEvent;			// Used by Callback to signal the user
CyU3PThread ThreadHandle;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;		// Set true once device is enumerated
uint32_t Counter[12];

void InitializeCPLD(void)
// CPLD needs to be RESET for correct operation
{
	CyU3PReturnStatus_t Status;
	CyU3PGpioClock_t GpioClock;
	CyU3PGpioSimpleConfig_t gpioConfig;

    // Startup the GPIO module clocks
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    CheckStatus("Start GPIO Clocks", Status);

    // Need to claim CTRL[10] from the GPIF Interface
	Status = CyU3PDeviceGpioOverride(CPLD_RESET, CyTrue);
	CheckStatus("CPLD_RESET Override", Status);

	// Reset by driving CPLD_RESET High
	CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = 1;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    Status = CyU3PGpioSetSimpleConfig(CPLD_RESET, &gpioConfig);
    CheckStatus("Reset CPLD", Status);
}

void ApplicationThread(uint32_t Value)
{
	// Value is passed to this routine from CyU3PThreadCreate, useful if the same code is used for multiple threads
	// Value is not used in this example
	int32_t Seconds = 0;
	uint32_t ActualEvents, Count;
    CyU3PReturnStatus_t Status;

    // Spin up the USB Connection
    Status = InitializeUSB();
    CheckStatus("Initialize USB", Status);

    if (Status == CY_U3P_SUCCESS)
    {
    	DebugPrint(4, "\r\nApplication started with %d", Value);
		// Wait for the device to be enumerated
		while (!glIsApplicationActive)
		{
			// Check for USB CallBack Events every 100msec
			Status = CyU3PEventGet(&CallbackEvent, USB_EVENTS, CYU3P_EVENT_OR_CLEAR, &ActualEvents, 100);
			if (Status == TX_SUCCESS) DebugPrintEvent(ActualEvents);
		}

    	// Now run forever
		DebugPrint(4, "\r\nMAIN now running forever: ");
    	while (1)
    	{
    		for (Count = 0; Count<10; Count++)
    		{
				// Check for User Commands (and other CallBack Events) every 100msec
				CyU3PThreadSleep(100);
				Status = CyU3PEventGet(&CallbackEvent, ANY_EVENT, CYU3P_EVENT_OR_CLEAR, &ActualEvents, TX_NO_WAIT);
				if (Status == TX_SUCCESS)
				{
					if (ActualEvents & USER_COMMAND_AVAILABLE) ParseCommand();
					else DebugPrintEvent(ActualEvents);
				}
    		}
			DebugPrint(4, "%d, ", Seconds++);
		}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    // Returning here will stop the Application Thread running - it failed anyway so this is OK
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status;

    // Create any needed resources then the Application thread
    Status = InitializeDebugConsole();
    CheckStatus("Initialize Debug Console", Status);

    // Create an Event so that alerts from CallBack routines can be monitored
    Status = CyU3PEventCreate(&CallbackEvent);
    CheckStatus("Create CallbackEvent", Status);

    // Need to Initialize CPLD
    InitializeCPLD();

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle, 	// Handle to my Application Thread
            "11:GPIF_Example2",               	// Thread ID and name
            ApplicationThread,     				// Thread entry function
            42,                             	// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,           // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,        // Thread priority
            APPLICATION_THREAD_PRIORITY,        // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice not supported
            CYU3P_AUTO_START                	// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS)
    {
        // Thread creation failed with the Status = error code
        while(1)
        {
        	// Application cannot continue. Loop indefinitely
        }
    }
}

