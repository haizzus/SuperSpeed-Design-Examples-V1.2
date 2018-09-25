// CPLD_Programmer - an XSVF file is downloaded from the PC and this program bitbangs IO to program the CPLD
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void IndicateError(uint16_t ErrorCode);
extern void xsvfExecute(void);

extern CyU3PDmaChannel glDmaChannelHandle;

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyU3PEvent glApplicationEvent;			// An Event to allow threads to communicate
uint16_t AvailableBytes;				// Bytes in DMA buffer not read yet
CyU3PDmaBuffer_t DmaBuffer;

void readByte(unsigned char *DataPtr)
{
	// Get a data byte from the input DMA channel
	CyU3PReturnStatus_t Status;
	if (AvailableBytes == 0)
	{
		DebugPrint(4, "\r\nWaiting for a buffer from the host");
		Status = CyU3PDmaChannelGetBuffer(&glDmaChannelHandle, &DmaBuffer, CYU3P_WAIT_FOREVER);
		AvailableBytes = DmaBuffer.count;
		DebugPrint(4, "\r\nGot data buffer Length = %d", AvailableBytes);
	}
	*DataPtr = *DmaBuffer.buffer++;
	if (--AvailableBytes == 0)
	{
        // Can now give the buffer back to the DMA controller so that it can refill it
        Status = CyU3PDmaChannelDiscardBuffer(&glDmaChannelHandle);
        CheckStatus("Return DMA Buffer", Status);
	}
}

void ApplicationThread_Entry (uint32_t Value)
{
    CyU3PReturnStatus_t Status;
    uint32_t ActualEvents;
    char* ThreadName;

    // First step is to get the Debug Console running so that the developer knows what is going on!
    Status = InitializeDebugConsole();
    CheckStatus("Debug Console Initialized", Status);

    // Create an Event which will allow the different threads/modules to synchronize
    Status = CyU3PEventCreate(&glApplicationEvent);
    CheckStatus("Create Event", Status);

    Status = InitializeUSB();
    CheckStatus("USB Initialized", Status);

    if (Status == CY_U3P_SUCCESS)
    {
    	CyU3PThreadInfoGet(&ApplicationThread, &ThreadName, 0, 0, 0);
    	ThreadName += 3;	// Skip numeric ID
    	DebugPrint(4, "\r\n%s started with %d", ThreadName, Value);
   		// Wait if the USB connection is not active
		CyU3PEventGet(&glApplicationEvent, USB_CONNECTION_ACTIVE, CYU3P_EVENT_AND, &ActualEvents, CYU3P_WAIT_FOREVER);
		AvailableBytes = 0;
		DebugPrint(4, "\r\nWaiting for data file", 0);
		// Now run the translation
		xsvfExecute();
		DebugPrint(4, "\r\nAvailable Bytes = %d", AvailableBytes);
		// All done, may as well RESET the board ready to run the next program
		DebugPrint(4, "\r\nProcessing Completed\r\nRESETTING CPU\r\n");
		CyU3PThreadSleep(100);
		CyU3PDeviceReset(CyFalse);
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    IndicateError(6);
    while (1);		// Hang here
}

// ApplicationDefine function is called by RTOS to startup the application thread after it has initialized its own threads
void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate (&ApplicationThread, // Handle to my Application Thread
            "15:CPLD_Programmmer",                	// Thread ID and name
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
    	IndicateError(5);
        /* Application cannot continue. Loop indefinitely */
        while(1);
    }
}


