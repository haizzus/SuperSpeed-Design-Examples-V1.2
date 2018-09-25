// Dual Console Example - this connects two debug consoles to the FX3
//
// One is the standard UART and the other is an I2C console
// You can type on either console and DebugPrint output is sent to both consoles
//
// You can only type quite slowly in the I2C Input Console
// This is a BUG an I am investigating the issue
//
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(uint8_t TraceLevel);
extern CyU3PReturnStatus_t I2C_DebugInit(uint8_t TraceLevel);
extern CyU3PReturnStatus_t I2C_DebugPrint(uint8_t Priority, char* Message, ...);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void IndicateError(uint16_t ErrorCode);
extern void CheckForCommand(void);

CyU3PThread ThreadHandle;		// Handles to my Application Threads

void ApplicationThread (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;

    // Start up the UART Console
    Status = InitializeDebugConsole(6);
    CheckStatus("Debug Console Initialized", Status);

    if (Status == CY_U3P_SUCCESS)
    {
    	CyU3PDebugPrint(4, "\r\nApplication started with %d\r\n", Value);

    	// Start up the I2C Console
    	Status = I2C_DebugInit(6);
        CheckStatus("I2C_DebugInit", Status);
        DebugPrint(4, "\r\nI2C Debug Console Example\r\n");		// This will display on BOTH consoles

        // Now run forever
    	while (1)
    	{
    		CyU3PThreadSleep(1000);
			DebugPrint(4, "%d, ", Seconds);
			Seconds++;
			CheckForCommand();		// Check for commands in Main context and execute them in Main context
    	}
    }
    CyU3PDebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    // If I get here then RTOS has started correctly, turn off ErrorIndicator
    IndicateError(0);

    // Create three threads, InputData, ProcessData and OutputData.  Each will need a stack
    void *StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    CyU3PThreadCreate(&ThreadHandle,			// Handle for this Thread
            "10:Dual_Console",                	// Thread ID and name
            ApplicationThread,     				// Thread function
            42,                             	// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,		// Thread priority
            APPLICATION_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
}

