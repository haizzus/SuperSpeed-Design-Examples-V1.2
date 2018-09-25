// Introduction to ThreadX threads and Thread Communication using Events
//
// john@usb-by-example.com
//

#include "Application.h"

CyU3PThread ThreadHandle[3];			// Handles to my Application Threads
CyU3PEvent SharedEvent;					// Used for thread communications
uint32_t DataOverrun, TotalData;		// Used to monitor for missed input data
uint32_t InputDataBuffer[100];			// InputData thread puts data here
uint32_t ProcessedDataBuffer[10];		// ProcessData thread puts data here
uint32_t TempCounter;					// Used to generate 'data'
uint8_t DID = 8;						// Set DID = Display Intermediate Data = 4 to see more display
uint32_t SampleTime = 3500;				// Time between data collections in Input Thread

extern void CheckStatus(uint8_t DisplayLevel, char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern void DisplayThreads(void);

// Declare some helper routines so that I can simply add/remove progress messages
void DoWork(uint32_t Time, char* Name)
{
	CyU3PDebugPrint(DID, "\r\n%s is busy working", Name);
	CyU3PThreadSleep(Time);
}

// Declare main application code
void InputDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t ActualEvents, Status, i;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Gather some input data
   		for (i = 0; i<Elements(InputDataBuffer); i++) InputDataBuffer[i] = TempCounter++;
   		DoWork(SampleTime, ThreadName);		// Pad the actual work for demonstration
   		TotalData++;
   		// Check that the previous data has been processed
   		Status = CyU3PEventGet(&SharedEvent, INPUT_DATA_AVAILABLE, CYU3P_EVENT_OR, &ActualEvents, CYU3P_NO_WAIT);
   		if (Status == 0) DataOverrun++;
   		else CyU3PEventSet(&SharedEvent, INPUT_DATA_AVAILABLE, CYU3P_EVENT_OR);
   		// Go back and find more input
    }
}

void ProcessDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t ActualEvents, i, j;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some input data to process
   		CyU3PEventGet(&SharedEvent, INPUT_DATA_AVAILABLE, CYU3P_EVENT_OR_CLEAR, &ActualEvents, CYU3P_WAIT_FOREVER);
   		for (i = 0; i<Elements(ProcessedDataBuffer); i++)
   		{
   			ProcessedDataBuffer[i] = 0;
   			for (j = 0; j<10; j++) ProcessedDataBuffer[i] += InputDataBuffer[(10*i)+j];
   		}
   		DoWork(2000, ThreadName);		// Pad the actual work for demonstration
   		// Hand off the processed data to the Output thread
   		CyU3PEventSet(&SharedEvent, PROCESSED_DATA_AVAILABLE, CYU3P_EVENT_OR);
   		// Do any tidy-up required
   		DoWork(100, ThreadName);
   		// Go back and find more work
    }
}

void OutputDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t i, ActualEvents;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some processed data to output
   		CyU3PEventGet(&SharedEvent, PROCESSED_DATA_AVAILABLE, CYU3P_EVENT_OR_CLEAR, &ActualEvents, CYU3P_WAIT_FOREVER);
   		DoWork(1000, ThreadName);		// Pad the actual work for demonstration
   		CyU3PDebugPrint(4, "\r\nOutput: ");
   		for (i = 0; i<Elements(ProcessedDataBuffer); i++) CyU3PDebugPrint(4, "%d ", ProcessedDataBuffer[i]);
   		// Go back and find more work
    }
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status, Counter;

    // First, get a debug console running so that we can display some progress
    // This DebugConsole will run in its own thread
    Status = InitializeDebugConsole();
    CheckStatus(4, "Enable DebugConsole", Status);

    // Create an Event Group that the threads will use to inter-communicate
    Status = CyU3PEventCreate(&SharedEvent);
    CheckStatus(4, "Create Event", Status);

    // Create three threads, InputData, ProcessData and OutputData.  Each will need a stack
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[0],// Handle for this Thread
            "10:Input_Thread",                	// Thread ID and name
            InputDataThread,     				// Thread function
            0,                             		// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            INPUT_DATA_THREAD_PRIORITY,			// Thread priority
            INPUT_DATA_THREAD_PRIORITY,			// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start ProcessData", Status);
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[1],// Handle for this Thread
            "11:Process_Thread",                // Thread ID and name
            ProcessDataThread,     				// Thread function
            1,                             		// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            PROCESS_DATA_THREAD_PRIORITY,		// Thread priority
            PROCESS_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start ProcessData", Status);
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[2],// Handle for this Thread
    		"12:Output_Thread",                	// Thread ID and name
    		OutputDataThread,     				// Thread function
    		2,                             		// Parameter passed to Thread
    		StackPtr,                       	// Pointer to the allocated thread stack
    		APPLICATION_THREAD_STACK,			// Allocated thread stack size
    		OUTPUT_DATA_THREAD_PRIORITY,        // Thread priority
    		OUTPUT_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
    		CYU3P_NO_TIME_SLICE,            	// Time slice no supported
    		CYU3P_AUTO_START);					// Start the thread immediately
    CheckStatus(4, "Start OutputData", Status);

    DisplayThreads();
    // This thread now becomes a monitoring function that displays statistics of the other two threads
    // Check for missed data every 10 seconds
    Counter = 0;
    while(1)
    {
    	CyU3PThreadSleep(10);
    	if (Counter & 0xE)
    	{
			Status = CyU3PGpioSetValue((Counter & 0xE)>>1, Counter & 1);
			if (Status != 0) CyU3PDebugPrint(4, "x");
    	}
		if (++Counter == 1000)
    	{
			CyU3PDebugPrint(4, "\r\nAt %d seconds, Missed Data = %d/%d", CyU3PGetTime()/1000, DataOverrun, TotalData);
			Counter = 0;
    	}
    }
}
