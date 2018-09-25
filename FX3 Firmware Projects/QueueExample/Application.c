// Introduction to ThreadX threads and Thread Communication using Queues
//
// john@usb-by-example.com
//


#include "Application.h"

#define THREAD_COUNT	(4)
CyU3PThread ThreadHandle[THREAD_COUNT];	// Handles to my Application Threads
void *StackPtr[THREAD_COUNT];			// Save these to track stack usage
CyU3PQueue InputDataAvailable;			// Used for thread communications
uint32_t InputAvailableQueue[3];		// Queue for up to 3 uint32_t pointers
CyU3PQueue InputDataDone;				// Used for thread communications
uint32_t InputDoneQueue[3];				// Queue for up to 3 uint32_t pointers
CyU3PQueue ProcessedDataAvailable;		// Used for thread communications
uint32_t ProcessedAvailableQueue[2];	// Queue for up to 2 uint32_t pointers
CyU3PQueue ProcessedDataDone;			// Used for thread communications
uint32_t ProcessedDoneQueue[2];			// Queue for up to 2 uint32_t pointers
uint32_t DataOverrun, TotalData;		// Used to monitor for missed input data
uint32_t InputDataBuffer[3][100];		// InputData thread puts data here - I now have 3 buffers
uint32_t ProcessedDataBuffer[2][10];	// ProcessData thread puts data here - I now have 2 buffers
uint32_t TempCounter;					// Used to generate 'data'
uint8_t DID = 8;						// Set DID = Display Intermediate Data = 4 to see more information

extern void CheckStatus(uint8_t DisplayLevel, char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t InitializeDebugConsole(void);

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
    uint32_t Status, i;
    uint32_t *BufferPtr, *Message;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Gather some input data, need a buffer to put it in
   		Status = CyU3PQueueReceive(&InputDataDone, &Message, CYU3P_NO_WAIT);
   		if (Status != 0)
   		{
   			DataOverrun++;
   			CyU3PQueueReceive(&InputDataDone, &Message, CYU3P_WAIT_FOREVER);
   		}
   		BufferPtr = Message;
   		CyU3PDebugPrint(DID, "\r\n%s got blank %X", ThreadName, Message);
   		for (i = 0; i<Elements(InputDataBuffer[0]); i++) *BufferPtr++ = TempCounter++;
   		DoWork(1500, ThreadName);		// Pad the actual work for demonstration
   		TotalData++;
   		// Send off the Input Data (well, a pointer to it)
   		CyU3PDebugPrint(DID, "\r\n%s Sending %X", ThreadName, Message);
   		CyU3PQueueSend(&InputDataAvailable, &Message, CYU3P_WAIT_FOREVER);
   		// Now look for more input
    }
}

void ProcessDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t i, j;
    uint32_t *InBufferPtr, *OutBufferPtr, *InMessage, *OutMessage;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
  		// Wait for some input data to process
   		CyU3PQueueReceive(&InputDataAvailable, &InMessage, CYU3P_WAIT_FOREVER);
   		InBufferPtr = InMessage;
   		CyU3PDebugPrint(DID, "\r\n%s got %X", ThreadName, InMessage);
   		// Get a buffer to write processed data into
   		CyU3PQueueReceive(&ProcessedDataDone, &OutMessage, CYU3P_WAIT_FOREVER);
   		OutBufferPtr = OutMessage;
   		CyU3PDebugPrint(DID, "\r\n%s got blank %X", ThreadName, OutMessage);
   		for (i = 0; i<Elements(ProcessedDataBuffer[0]); i++)
   		{
   			*OutBufferPtr = 0;
   			for (j = 0; j<10; j++) *OutBufferPtr += *InBufferPtr++;
   			OutBufferPtr++;
   		}
   		DoWork(2000, ThreadName);		// Pad the actual work for demonstration
   		// Return the used buffer
   		CyU3PDebugPrint(DID, "\r\n%s Returning %X", ThreadName, InMessage);
   		CyU3PQueueSend(&InputDataDone, &InMessage, CYU3P_WAIT_FOREVER);
   		// Hand off the processed data to the Output thread
   		CyU3PDebugPrint(DID, "\r\n%s Sending %X", ThreadName, OutMessage);
   		CyU3PQueueSend(&ProcessedDataAvailable, &OutMessage, CYU3P_WAIT_FOREVER);
   		// Do any tidy-up required
   		DoWork(100, ThreadName);
   		// Go back and find more work
    }
}

void OutputDataThread(uint32_t Value)
{
    char* ThreadName;
    uint32_t i;
    uint32_t *Message, *BufferPtr;

	CyU3PThreadInfoGet(&ThreadHandle[Value], &ThreadName, 0, 0, 0);
	ThreadName += 3;	// Skip numeric ID
	CyU3PDebugPrint(4, "\r\n%s started", ThreadName);
    // Now run forever
   	while (1)
   	{
   		// Wait for some processed data to output
   		CyU3PQueueReceive(&ProcessedDataAvailable, &Message, CYU3P_WAIT_FOREVER);
   		BufferPtr = Message;
   		CyU3PDebugPrint(DID, "\r\n%s Got %X", ThreadName, Message);
   		DoWork(1000, ThreadName);		// Pad the actual work for demonstration
   		CyU3PDebugPrint(4, "\r\nOutput: ");
   		for (i = 0; i<Elements(ProcessedDataBuffer[0]); i++) CyU3PDebugPrint(4, "%d ", *BufferPtr++);
   		CyU3PDebugPrint(DID, "\r\n%s Returning %X", ThreadName, Message);
   		CyU3PQueueSend(&ProcessedDataDone, &Message, CYU3P_WAIT_FOREVER);
   		// Go back and find more work
    }
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
//    void *StackPtr;
    uint32_t Status, i, j;
    uint32_t* Message;

    // First, get a debug console running so that we can display some progress
    // This DebugConsole will run in its own thread
    Status = InitializeDebugConsole();
    CheckStatus(4, "Enable DebugConsole", Status);

    // Create four Queues that the threads will use to pass data (well, data pointers)
    // Each queue element is 1 uint32_t long and I have room for up to 3 of these in each queue
    Status = CyU3PQueueCreate(&InputDataAvailable, 1, &InputAvailableQueue, sizeof(InputAvailableQueue));
    CheckStatus(4, "Create InputAvailableQueue", Status);
    Status = CyU3PQueueCreate(&InputDataDone, 1, &InputDoneQueue, sizeof(InputDoneQueue));
    CheckStatus(4, "Create InputDoneQueue", Status);
    // Now post three buffers onto the InputDoneQueue for the InputDataThread to use
    for (i = 0; i<3; i++)
    {
		Message = InputDataBuffer[i];
		Status = CyU3PQueueSend(&InputDataDone, &Message, CYU3P_WAIT_FOREVER);
		CheckStatus(4, "Post buffer", Status);
    }
    Status = CyU3PQueueCreate(&ProcessedDataAvailable, 1, &ProcessedAvailableQueue, sizeof(ProcessedAvailableQueue));
    CheckStatus(4, "Create ProcessedAvailableQueue", Status);
    Status = CyU3PQueueCreate(&ProcessedDataDone, 1, &ProcessedDoneQueue, sizeof(ProcessedDoneQueue));
    CheckStatus(4, "Create InputDoneQueue", Status);
    // Now post two buffers onto the ProcessedDoneQueue for the ProcessDataThread to use
    for (i = 0; i<2; i++)
    {
    	Message = ProcessedDataBuffer[i];
    	Status = CyU3PQueueSend(&ProcessedDataDone, &Message, CYU3P_WAIT_FOREVER);
    	CheckStatus(4, "Post buffer", Status);
    }
    // Create four threads, InputData, 2 * ProcessData and OutputData.  Each will need a stack
    StackPtr[0] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[0],// Handle for this Thread
            "10:InputData",                		// Thread ID and name
            InputDataThread,     				// Thread function
            0,                             		// Parameter passed to Thread
            StackPtr[0],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            INPUT_DATA_THREAD_PRIORITY,			// Thread priority
            INPUT_DATA_THREAD_PRIORITY,			// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start InputData", Status);
    StackPtr[1] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[1],// Handle for this Thread
            "11:Processor_0",                	// Thread ID and name
            ProcessDataThread,     				// Thread function
            1,                             		// Parameter passed to Thread
            StackPtr[1],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            PROCESS_DATA_THREAD_PRIORITY,		// Thread priority
            PROCESS_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start Processor_0", Status);
    StackPtr[2] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[2],// Handle for this Thread
            "13:Processor_1",                	// Thread ID and name
            ProcessDataThread,     				// Thread function
            2,                             		// Parameter passed to Thread
            StackPtr[2],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            PROCESS_DATA_THREAD_PRIORITY,		// Thread priority
            PROCESS_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
    CheckStatus(4, "Start Processor_1", Status);
    StackPtr[3] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle[3],// Handle for this Thread
    		"12:OutputData",                	// Thread ID and name
    		OutputDataThread,     				// Thread function
    		3,                             		// Parameter passed to Thread
    		StackPtr[3],                       	// Pointer to the allocated thread stack
    		APPLICATION_THREAD_STACK,			// Allocated thread stack size
    		OUTPUT_DATA_THREAD_PRIORITY,        // Thread priority
    		OUTPUT_DATA_THREAD_PRIORITY,		// = Thread priority so no preemption
    		CYU3P_NO_TIME_SLICE,            	// Time slice no supported
    		CYU3P_AUTO_START);					// Start the thread immediately
    CheckStatus(4, "Start OutputData", Status);

    // This thread now becomes a monitoring function that displays statistics of the other two threads
    // Check for missed data and stack usage every 10 seconds
    while(1)
    {
    	CyU3PThreadSleep(10000);
		CyU3PDebugPrint(4, "\r\nAt %d seconds, Missed Data = %d/%d", CyU3PGetTime()/1000, DataOverrun, TotalData);
		for (i = 0; i<THREAD_COUNT; i++)
		{
			// Note that StackSize is in bytes but RTOS fill pattern is a uint32
			uint32_t* StackStartPtr = StackPtr[i];
			uint32_t* DataPtr = StackStartPtr;
			char* ThreadName;
			for (j = 0; j<APPLICATION_THREAD_STACK>>2; j++) if (*DataPtr++ != 0xEFEFEFEF) break;
			CyU3PThreadInfoGet(&ThreadHandle[i], &ThreadName, 0, 0, 0);
			ThreadName += 3;	// Skip numeric ID
			CyU3PDebugPrint(4, "\r\nStack free in %s is %d/%d", ThreadName, (DataPtr - StackStartPtr)<<2, APPLICATION_THREAD_STACK);
		}
    }
}
