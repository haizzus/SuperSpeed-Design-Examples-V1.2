// Demonstrate USB by enumerating as a composite device - CDC + BulkLoop Interfaces
//
// john@usb-by-example.com
//
// On power on the CDC interface is looped-back such that characters are just echoed
// and the UART is connected as the Debug Console
// Following a "switch" command the Debug Console is connected to the CDC interface
// Another "switch" will switch it back to the UART
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void BackgroundPrint(uint32_t TimeToWait);
extern SwitchConsoles(void);

extern CyU3PEvent DisplayEvent;			// Used to display events

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;			// Set true once device is enumerated

void ApplicationThread_Entry (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;

    Status = InitializeDebugConsole();
    CheckStatus("Debug Console Initialized", Status);

    // Create an Event Group that Callbacks can use to signal events to a Background DebugPrint
    Status = CyU3PEventCreate(&DisplayEvent);
    CheckStatus("Create Event", Status);

    // Give me time to start up my Ellisys Explorer
    DebugPrint(4, "\n\n\nStart Ellisys NOW\n");
    CyU3PThreadSleep(10000);

    Status = InitializeUSB();
    CheckStatus("USB Initialized", Status);

    // Wait for the USB connection to be up
    while (!glIsApplicationActive) BackgroundPrint(10);

    if (Status == CY_U3P_SUCCESS)
    {
    	DebugPrint(4, "\nApplication started with %d\n", Value);
    	SwitchConsoles();
    	// Now run forever
    	while (1)
    	{
        	{
        		DebugPrint(4, "%d, ", Seconds++);
    			BackgroundPrint(1000);				// Do something useful while waiting
    		}
		}
    }
    DebugPrint(4, "\nApplication failed to initialize. Error code: %d.\n", Status);
    while (1);		// Hang here
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate (&ApplicationThread, // Handle to my Application Thread
            "11:CDC_BulkLoop",                		// Thread ID and name
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


