// Chapter3Example3 - demonstrate the operation of an RTOS Mutex
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyU3PThread ThreadHandle[2];			// Handle to my Application Thread
CyU3PMutex SharedMutex;					// Used to control access to a shared resource
CyU3PTimer StatsTimer;					// Used to display Thread statistics
uint32_t LoopCounter[2];				// Count passes through each Thread loop
uint32_t TotalTime[2];					// Measure time through each loop of Thread

const uint32_t ActivityTime[2][5] = {
		{ 200,  500, 400,  300,   0 },	// Times for Speedy
		{   0, 1200, 800, 1100, 900 }	// Times for Slow (= 2x Speedy)
};


// Declare three helper routines so that I can simply add/remove progress messages
void DoWork(uint32_t Time, char* Name)
{
//	CyU3PDebugPrint(4, "\r\n%s waiting %d", Name, Time);
	CyU3PThreadSleep(Time);
}

void GetMutex(char* Name)
{
	CyU3PReturnStatus_t Status;
	Status = CyU3PMutexGet(&SharedMutex, CYU3P_WAIT_FOREVER);
//	CheckStatus("Get", Status);
	CyU3PDebugPrint(4, "\r\n%s has Mutex", Name);
}

void PutMutex(char* Name)
{
	CyU3PReturnStatus_t Status;
//	CyU3PDebugPrint(4, "\r\n%s returning Mutex", Name);
	Status = CyU3PMutexPut(&SharedMutex);
//	CheckStatus("Put", Status);
}

// Declare main application code
// Note that both threads use the SAME CODE; Value passed in determines the thread identity
void ApplicationThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t StartTime;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		StartTime = CyU3PGetTime();
   		DoWork(ActivityTime[Value][0], ThreadName);
   		GetMutex(ThreadName);
   		DoWork(ActivityTime[Value][1], ThreadName);		// Work done with Mutex owned
   		PutMutex(ThreadName);
   		DoWork(ActivityTime[Value][2], ThreadName);
   		GetMutex(ThreadName);
   		DoWork(ActivityTime[Value][3], ThreadName);		// Work done with Mutex owned
   		PutMutex(ThreadName);
   		DoWork(ActivityTime[Value][4], ThreadName);
   		LoopCounter[Value]++;							// Keep loop statistics
   		TotalTime[Value] += CyU3PGetTime() - StartTime;	// Keep loop statistics
    }
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status;

    // First, get a debug console running so that we can display some progress
    // This DebugConsole will run in its own thread
    Status = InitializeDebugConsole();
    CheckStatus("Enable DebugConsole", Status);

    // Create a Mutex that will protect a shared resource
    Status = CyU3PMutexCreate(&SharedMutex, CYU3P_INHERIT);
    CheckStatus("Create Mutex", Status);

    // Create two threads, Speedy and Slow.  Each will need a stack
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACKSIZE);
    Status = CyU3PThreadCreate(&ThreadHandle[0],// Handle for this Thread
            "10:Speedy",                		// Thread ID and name
            ApplicationThread,     				// Thread function
            0,                             		// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACKSIZE,		// Allocated thread stack size
            SPEEDY_THREAD_PRIORITY,				// Thread priority
            SPEEDY_THREAD_PRIORITY,				// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus("Start Speedy", Status);
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACKSIZE);
    Status = CyU3PThreadCreate(&ThreadHandle[1],// Handle for this Thread
    		"11:Slow",                			// Thread ID and name
    		ApplicationThread,     				// Thread function
    		1,                             		// Parameter passed to Thread
    		StackPtr,                       	// Pointer to the allocated thread stack
    		APPLICATION_THREAD_STACKSIZE,		// Allocated thread stack size
    		SLOW_THREAD_PRIORITY,           	// Thread priority
    		SLOW_THREAD_PRIORITY,				// = Thread priority so no preemption
    		CYU3P_NO_TIME_SLICE,            	// Time slice no supported
    		CYU3P_AUTO_START);					// Start the thread immediately
    CheckStatus("Start Slow", Status);

    // This thread now becomes a monitoring function that displays statistics of the other two threads
    // Display the average time each thread is taking to run every 10 seconds
    CyU3PSetTime(0);							// Reset System Timer
    while(1)
    {
    	CyU3PThreadSleep(10000);
		CyU3PDebugPrint(4, "\r\nAt %d seconds, Speedy Average Time = %d, Slow Average Time = %d",
			CyU3PGetTime()/1000, TotalTime[0]/LoopCounter[0], TotalTime[1]/LoopCounter[1]);
    }
}

// Main sets up the CPU environment the starts the RTOS
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
			ioConfig.useUart   = true;
			ioConfig.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}

    while (1);			// Get here on a failure, can't recover, just hang here
						// Once the programs get more complex we shall do something more elegant here
	return 0;			// Won't get here but compiler wants this!
}


