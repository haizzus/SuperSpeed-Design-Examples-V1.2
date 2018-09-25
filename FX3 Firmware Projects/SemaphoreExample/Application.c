// Introduction to ThreadX threads and Thread Communications using Semaphores
//
// john@usb-by-example.com
//

#include "Application.h"

CyU3PThread ThreadHandle[2];				// Handles to my Application Threads
CyU3PSemaphore DataToProcess, DataToOutput;	// Used for thread communications
CyU3PTimer InputDataTimer;					// Used to create periodic input data
uint32_t DataOverrun, TotalData;			// Used to monitor for missed input data
uint32_t InputDataBuffer[100];				// InputData thread puts data here
uint32_t ProcessedDataBuffer[10];			// ProcessData thread puts data here
uint32_t TempCounter;						// Used to generate 'data'

extern void CheckStatus(uint8_t DisplayLevel, char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t InitializeDebugConsole(void);

// Declare some helper routines so that I can simply add/remove progress messages
void DoWork(uint32_t Time, char* Name)
{
	CyU3PDebugPrint(4, "\r\n%s is busy working", Name);
	CyU3PThreadSleep(Time);
}

// Declare main application code
// Input data is created on a periodic basis using a System Timer
void CreateInputData(uint32_t InitialValue)
{
	// NOTE: a System Timer routine runs in ISR context so it CANNOT use any blocking calls
	// CyU3PDebugPrint() is a blocking call :-(
	uint32_t i, CurrentValue;
	for (i = 0; i<Elements(InputDataBuffer); i++) InputDataBuffer[i] = TempCounter++;
	TotalData++;
	// Check that the previous data has been processed
	tx_semaphore_info_get(&DataToProcess, 0, &CurrentValue, 0, 0, 0);
	if (CurrentValue == 1) DataOverrun++;
	// Set an Semaphore to indicate at input data has been created/collected/found
	else CyU3PSemaphorePut(&DataToProcess);
}

void ProcessDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t i, j;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some input data to process
   		CyU3PSemaphoreGet(&DataToProcess, CYU3P_WAIT_FOREVER);
   		for (i = 0; i<Elements(ProcessedDataBuffer); i++)
   		{
   			ProcessedDataBuffer[i] = 0;
   			for (j = 0; j<10; j++) ProcessedDataBuffer[i] += InputDataBuffer[(10*i)+j];
   		}
   		DoWork(2000, ThreadName);		// Pad the actual work for demonstration
   		// Hand off the processed data to the Output thread
   		CyU3PSemaphorePut(&DataToOutput);
   		// Do any tidy-up required
   		DoWork(100, ThreadName);
   		// Go back and find more work
    }
}

void OutputDataThread(uint32_t Value)
{
    char* ThreadName;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some processed data to output
   		CyU3PSemaphoreGet(&DataToOutput, CYU3P_WAIT_FOREVER);
   		DoWork(1000, ThreadName);		// Pad the actual work for demonstration
   		// Go back and find more work
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
    CheckStatus(4, "Enable DebugConsole", Status);

    // Create two semaphores that the threads will use to communicate
    Status = CyU3PSemaphoreCreate(&DataToProcess, 0);
    CheckStatus(4, "Create ToProcess Semaphore", Status);
    Status = CyU3PSemaphoreCreate(&DataToOutput, 0);
    CheckStatus(4, "Create ToOutput Semaphore", Status);

    // Use a System Timer to generate Input Data on a regular basis
    Status = CyU3PTimerCreate(&InputDataTimer, CreateInputData, 0, 500, 3500, CYU3P_AUTO_ACTIVATE);

    // Create two threads, ProcessData and OutputData.  Each will need a stack
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[0],// Handle for this Thread
            "10:ProcessData",                	// Thread ID and name
            ProcessDataThread,     				// Thread function
            0,                             		// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            PROCESS_DATA_THREAD_PRIORITY,		// Thread priority
            PROCESS_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start ProcessData", Status);
    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[1],// Handle for this Thread
    		"11:OutputData",                	// Thread ID and name
    		OutputDataThread,     				// Thread function
    		1,                             		// Parameter passed to Thread
    		StackPtr,                       	// Pointer to the allocated thread stack
    		APPLICATION_THREAD_STACK,			// Allocated thread stack size
    		OUTPUT_DATA_THREAD_PRIORITY,        // Thread priority
    		OUTPUT_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
    		CYU3P_NO_TIME_SLICE,            	// Time slice no supported
    		CYU3P_AUTO_START);					// Start the thread immediately
    CheckStatus(4, "Start OutputData", Status);

    // This thread now becomes a monitoring function that displays statistics of the other two threads
    // Check for missed data every 10 seconds
    while(1)
    {
    	CyU3PThreadSleep(10000);
		CyU3PDebugPrint(4, "\r\nAt %d seconds, Missed Data = %d/%d", CyU3PGetTime()/1000, DataOverrun, TotalData);
    }
}
