// Chapter4Example5 - demonstrate the operation of an RTOS Semaphore
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyU3PThread ThreadHandle[APP_THREADS];		// Handles to my Application Threads
void *StackPtr[APP_THREADS];				// Stack allocated to each thread
CyU3PSemaphore DataToProcess, DataToOutput;	// Used for thread communications
CyU3PTimer InputDataTimer;					// Used to create periodic input data
uint32_t DataOverrun, TotalData;			// Used to monitor for missed input data
uint32_t InputDataBuffer[100];				// InputData thread puts data here
uint32_t ProcessedDataBuffer[10];			// ProcessData thread puts data here
uint32_t TempCounter;						// Used to generate 'data'
uint32_t SampleTime = 3500;					// Time between data collections in Input Thread


// Declare some helper routines so that I can simply add/remove progress messages
void DoWork(uint32_t Time, char* Name)
{
	DebugPrint(4, "\r\n%s is busy working", Name);
	CyU3PThreadSleep(Time);
}

// Declare main application code
void InputDataThread(uint32_t Value)
{
    char* ThreadName;
    CyU3PThread *ThisThread;
    uint32_t i, CurrentValue;

	ThisThread = CyU3PThreadIdentify();
	CyU3PThreadInfoGet(ThisThread, &ThreadName, 0, 0, 0);
	DebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Gather some input data
   		for (i = 0; i<Elements(InputDataBuffer); i++) InputDataBuffer[i] = TempCounter++;
   		DoWork(SampleTime, ThreadName);		// Pad the actual work for demonstration
   		TotalData++;
		// Check that the previous data has been processed
		tx_semaphore_info_get(&DataToProcess, 0, &CurrentValue, 0, 0, 0);
		if (CurrentValue == 1) DataOverrun++;
		// Set an Semaphore to indicate at input data has been created/collected/found
		else CyU3PSemaphorePut(&DataToProcess);
   	}
}

void ProcessDataThread(uint32_t Value)
{
    char* ThreadName;
    CyU3PThread *ThisThread;
    uint32_t i, j;
    uint16_t TX_Status;

	ThisThread = CyU3PThreadIdentify();
	CyU3PThreadInfoGet(ThisThread, &ThreadName, 0, 0, 0);
	DebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some input data to process
   		TX_Status = tx_semaphore_get(&DataToProcess, A_LONG_TIME);
   		if (TX_Status) DebugPrint(4, "\r\nGet on 'DataToProcess' error = %x", TX_Status);
   		else
   		{
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
}

void OutputDataThread(uint32_t Value)
{
    char* ThreadName;
    CyU3PThread *ThisThread;
    uint16_t TX_Status;

	ThisThread = CyU3PThreadIdentify();
	CyU3PThreadInfoGet(ThisThread, &ThreadName, 0, 0, 0);
	DebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some processed data to output
   		TX_Status = tx_semaphore_get(&DataToOutput, A_LONG_TIME);
   		if (TX_Status) DebugPrint(4, "\r\nGet on 'DataToOutput' error = %x", TX_Status);
   		DoWork(1000, ThreadName);		// Pad the actual work for demonstration
   		// Go back and find more work
    }
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    uint32_t Status;

    // First, get a debug console running so that we can display some progress
    // This DebugConsole will run in its own thread
    Status = InitializeDebugConsole();
    CheckStatus("Enable DebugConsole", Status);

    // Create two semaphores that the threads will use to communicate
    Status = CyU3PSemaphoreCreate(&DataToProcess, 0);
    CheckStatus("Create ToProcess Semaphore", Status);
    Status = CyU3PSemaphoreCreate(&DataToOutput, 0);
    CheckStatus("Create ToOutput Semaphore", Status);

    // Create three threads, InputData, ProcessData and OutputData.  Each will need a stack
    StackPtr[0] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[0],// Handle for this Thread
            "10:Input_Thread",                	// Thread ID and name
            InputDataThread,     				// Thread function
            0,                             		// Parameter passed to Thread
            StackPtr[0],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            INPUT_DATA_THREAD_PRIORITY,			// Thread priority
            INPUT_DATA_THREAD_PRIORITY,			// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus("Start InputData", Status);
    StackPtr[1] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[1],// Handle for this Thread
            "11:Process_Thread",                // Thread ID and name
            ProcessDataThread,     				// Thread function
            1,                             		// Parameter passed to Thread
            StackPtr[1],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            PROCESS_DATA_THREAD_PRIORITY,		// Thread priority
            PROCESS_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus("Start ProcessData", Status);
    StackPtr[2] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[2],// Handle for this Thread
    		"12:Output_Thread",                	// Thread ID and name
    		OutputDataThread,     				// Thread function
    		2,                             		// Parameter passed to Thread
    		StackPtr[2],                       	// Pointer to the allocated thread stack
    		APPLICATION_THREAD_STACK,			// Allocated thread stack size
    		OUTPUT_DATA_THREAD_PRIORITY,        // Thread priority
    		OUTPUT_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
    		CYU3P_NO_TIME_SLICE,            	// Time slice no supported
    		CYU3P_AUTO_START);					// Start the thread immediately
    CheckStatus("Start OutputData", Status);

    // This thread now becomes a monitoring function that displays statistics of the other two threads
    // Check for missed data every 10 seconds
    while(1)
    {
    	CyU3PThreadSleep(10000);
		DebugPrint(4, "\r\nAt %d seconds, Missed Data = %d/%d", CyU3PGetTime()/1000, DataOverrun, TotalData);
    }
}

