// Demonstrate USB by enumerating as a CDC Device
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void BackgroundPrint(uint32_t TimeToWait);

extern CyU3PEvent DisplayEvent;			// Used to display events

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;			// Set true once device is enumerated





void ApplicationThread_Entry (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

#if (DirectConnect)
    // Can't have a UART Debug Console with DirectConnect
    // This means that all of the DebugPrints will fail since glDebugTxEnabled = CyFalse
#else
    Status = InitializeDebugConsole();
    CheckStatus("Debug Console Initialized", Status);
#endif

    // Create an Event Group that Callbacks can use to signal events to a Background DebugPrint
    Status = CyU3PEventCreate(&DisplayEvent);
    CheckStatus("Create Event", Status);

    // Give me time to start up my Ellisys Explorer
    DebugPrint(4, "\r\n\r\n\r\nStart Ellisys NOW\r\n");
    CyU3PThreadSleep(10000);

    Status = InitializeUSB();
    CheckStatus("USB Initialized", Status);

    // Wait for the USB connection to be up
    while (!glIsApplicationActive) BackgroundPrint(10);

    if (Status == CY_U3P_SUCCESS)
    {
    	DebugPrint(4, "\r\nApplication started with %d\r\n", Value);
    	// Now run forever
    	while (1)
    	{
        	{
        		DebugPrint(4, "%d, ", Seconds++);
    			BackgroundPrint(1000);				// Do something useful while waiting
    		}
		}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}


