// Keyboard.c - demonstrate USB by enumerating as a keyboard
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyU3PEvent glApplicationEvent;			// An Event to allow threads to communicate


void ApplicationThread_Entry (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;
    uint32_t ActualEvents;
    char* ThreadName;

    // First step is to get the Debug Console running so that the developer knows what is going on!
    Status = InitializeDebugConsole();
    CheckStatus("Debug Console Initialized", Status);

    // Create an Event which will allow the different threads/modules to synchronize
    Status = CyU3PEventCreate(&glApplicationEvent);
    CheckStatus("Create Event", Status);

    // Give me time to start up my Ellisys Explorer
//    CyU3PThreadSleep (10000);

    Status = InitializeUSB();
    CheckStatus("USB Initialized", Status);

    if (Status == CY_U3P_SUCCESS)
    {
    	CyU3PThreadInfoGet(&ApplicationThread, &ThreadName, 0, 0, 0);
    	ThreadName += 3;	// Skip numeric ID
    	CyU3PDebugPrint(4, "\r\n%s started with %d", ThreadName, Value);
    	// Now run forever
    	while (1)
    	{
    		// Wait if the USB connection is not active
    		CyU3PEventGet(&glApplicationEvent, USB_CONNECTION_ACTIVE, CYU3P_EVENT_AND, &ActualEvents, CYU3P_WAIT_FOREVER);
			CyU3PDebugPrint(4, "%d, ", Seconds++);
    		CyU3PThreadSleep(1000);
		}
    }
    CyU3PDebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}

// ApplicationDefine function is called by RTOS to startup the application thread after it has initialized its own threads
void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate (&ApplicationThread, // Handle to my Application Thread
            "11:Template",                			// Thread ID and name
            ApplicationThread_Entry,     			// Thread entry function
            42,                             		// Parameter passed to Thread
            StackPtr,                       		// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,               // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,            // Thread priority
            APPLICATION_THREAD_PRIORITY,            // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            		// Time slice not supported in FX3 implementation
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


