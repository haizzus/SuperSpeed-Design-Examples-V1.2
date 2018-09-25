/*
 * I2C_DebugConsole.c
 *
 *  This module implements the DebugPrint portion of cyu3debug.c for an I2C-based console
 *    The LOG function is not implemented which makes this code simpler
 */

#include "Application.h"
#include <stdarg.h>        // For argument processing
#include <string.h>

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void GotConsoleInput(uint8_t Source, char InputChar);
extern CyU3PReturnStatus_t MyDebugSNPrint(uint8_t *debugMsg, uint16_t *length, char *message, va_list argp);

// Variables static to this module
static CyU3PThread      I2C_DebugThread;
static CyU3PMutex       I2C_DebugLock;
static CyU3PQueue       I2C_DebugQueue;
static CyU3PDmaChannel  I2C_DebugTXHandle;
static CyU3PDmaChannel  I2C_DebugRXHandle;
CyBool_t         I2C_DebugEnabled;       // Debug Init has been called or not
static CyU3PDmaBuffer_t Queue[CY_U3P_DEBUG_DMA_BUFFER_COUNT];
static uint8_t          I2C_DebugTraceLevel;

CyU3PReturnStatus_t I2C_DebugPrint(uint8_t Priority, char* Message, ...)
{
    // This takes the same parameters as CyU3PDebugPrint and my code is modeled on CyU3PDebugPrint
    // I format Message, including any parameters, into a DMA Buffer then Queue this buffer for I2C
    // I check for Console Input after every Console Output
    // A Queue timeout is used to ensure that Console Input is called at least once a second
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
    va_list argp;
    CyU3PDmaBuffer_t CurrentDMABuffer;

    // First do some error checking
    if (!I2C_DebugEnabled) return CY_U3P_ERROR_NOT_STARTED;
    if (Priority > glDebugTraceLevel) return CY_U3P_SUCCESS;
    if (CyU3PThreadIdentify() == NULL) return CY_U3P_ERROR_INVALID_CALLER;    // This function can only be called from a thread

    // OK to proceed, get a buffer then use a Cypress routine to do the Message interpretation
    CyU3PMutexGet(&I2C_DebugLock, CYU3P_WAIT_FOREVER);

    // Allocate the buffer for formatting the string.
    CurrentDMABuffer.buffer = CyU3PDmaBufferAlloc(CY_U3P_DEBUG_DMA_BUFFER_SIZE);
    if (CurrentDMABuffer.buffer == NULL) CheckStatus("CyU3PDmaBufferAlloc", CY_U3P_ERROR_MEMORY_ERROR);

    if (Status == CY_U3P_SUCCESS)   
    {
        CurrentDMABuffer.count = CurrentDMABuffer.size = CY_U3P_DEBUG_DMA_BUFFER_SIZE;
        CurrentDMABuffer.status = 0;
        va_start(argp, Message);
        // MyDebugSNPrint updates CurrentDMABuffer.count
        Status = MyDebugSNPrint(CurrentDMABuffer.buffer, &CurrentDMABuffer.count, Message, argp);
        va_end(argp);
        // Increment the count to include the NULL character also.
        CurrentDMABuffer.count++;
    }
    if (Status == CY_U3P_SUCCESS)   
    {
    	// Copy the output to the UART Console also for this dual console example
    	CyU3PDebugPrint(4, "%s", CurrentDMABuffer.buffer);
        // Now queue this message to be displayed on the I2C console
        Status = CyU3PQueueSend(&I2C_DebugQueue, &CurrentDMABuffer, CYU3P_WAIT_FOREVER);
        if (Status != CY_U3P_SUCCESS) CheckStatus("QueueSend", Status);
    }
    if ((Status != CY_U3P_SUCCESS) && (CurrentDMABuffer.buffer != NULL))
    {
        CyU3PDmaBufferFree(CurrentDMABuffer.buffer);
    }
    CyU3PMutexPut(&I2C_DebugLock);
    return Status;
}

CyU3PReturnStatus_t Restart_I2C()
{
    CyU3PReturnStatus_t Status;
    CyU3PI2cConfig_t i2cConfig;

    CyU3PI2cDeInit();           // Turn it off if it is on
    Status = CyU3PI2cInit();    // Start the I2C driver
    CheckStatus("I2cInit", Status);

    i2cConfig.bitRate    = CY_FX_USBI2C_I2C_BITRATE;
    i2cConfig.busTimeout = -1;
    i2cConfig.dmaTimeout = -1;
    i2cConfig.isDma      = CyTrue;
    Status = CyU3PI2cSetConfig(&i2cConfig, NULL);
    CheckStatus("I2cSetConfig", Status);

    CyU3PDmaChannelReset(&I2C_DebugRXHandle);
    CyU3PDmaChannelReset(&I2C_DebugTXHandle);

    return Status;
}

void I2C_ConsoleThread(uint32_t Value)
{
    // Value passed to this thread is a Semaphore that thread should signal once it is ready process buffers
    CyU3PReturnStatus_t Status, Q_Status;
    CyU3PDmaBuffer_t FilledBuffer, ConsoleIn;
    CyU3PI2cPreamble_t Preamble;
    int32_t retryCount = I2C_RETRY_COUNT;

    // Get an aligned buffer to collect I2C Console Input
    ConsoleIn.buffer = CyU3PDmaBufferAlloc(I2C_READ_SIZE);
    ConsoleIn.size = I2C_READ_SIZE;
    // Preset fixed data
    Preamble.buffer[0] = CY7C65215_DeviceAddress<<1;
    Preamble.length = 1;
    Preamble.ctrlMask = 0;

    // Tell InitDebug that the thread is ready for work
    Status = CyU3PSemaphorePut((CyU3PSemaphore*)Value);
    CheckStatus("Signal Thread Ready", Status);

    // Now wait for filled buffers to be send to the Queue and forward them to the I2C Block
    while (1)
    {
        Q_Status = CyU3PQueueReceive(&I2C_DebugQueue, &FilledBuffer, 100);

        // It is recommended to read from the I2C device before transmitting anything.
        if ((Q_Status == CY_U3P_ERROR_QUEUE_EMPTY) || (Q_Status == CY_U3P_SUCCESS))
        {
            // Poll I2C for console in
            Preamble.buffer[0] |= 1;		// For a Read
            Status = CyU3PI2cSendCommand(&Preamble, I2C_READ_SIZE, CyTrue);
            CheckStatus("CyU3PI2cSendCommand", Status);
            if (Status == CY_U3P_SUCCESS)
            {
                CyU3PMemSet (ConsoleIn.buffer, 0xFF, I2C_READ_SIZE);
                ConsoleIn.count = ConsoleIn.status = 0;
                Status = CyU3PDmaChannelSetupRecvBuffer(&I2C_DebugRXHandle, &ConsoleIn);
                CheckStatus("CyU3PDmaChannelSetupRecvBuffer", Status);
            }
            if (Status == CY_U3P_SUCCESS)
            {
                Status = CyU3PDmaChannelWaitForCompletion(&I2C_DebugRXHandle, 100);
//                CheckStatus("CyU3PDmaChannelWaitForCompletion(RX)", Status);
            }
            if (Status == CY_U3P_SUCCESS)
            {
                uint32_t i;
                for (i = 0; i < I2C_READ_SIZE; i++)
                {
                    if (ConsoleIn.buffer[i] != 0xFF) GotConsoleInput(1, ConsoleIn.buffer[i]);
                    else break;
                }
            }
            else Restart_I2C ();		// Read failed, recover the I2C Channel
            CyU3PThreadSleep(50); 		// Short sleep after an I2C operation is recommended.
        }

        if (Q_Status == CY_U3P_SUCCESS)
        {
            // There was a buffer waiting, send it to the I2C Block
            Status = CyU3PDmaChannelSetupSendBuffer (&I2C_DebugTXHandle, &FilledBuffer);
            CheckStatus("CyU3PDmaChannelSetupSendBuffer", Status);
            // Now tell the I2C Block what to do with this buffer of data
            Preamble.buffer[0] &= 0xFE;	// Clear LSb = Write
            if (Status == CY_U3P_SUCCESS) CyU3PI2cSendCommand(&Preamble, FilledBuffer.count, CyFalse);
            // Wait for the I2C transfer to be done
            if (Status == CY_U3P_SUCCESS) Status = CyU3PI2cWaitForBlockXfer(CyFalse);
            if (Status != CY_U3P_SUCCESS)
            {
            	CheckStatus("I2C_Write", Status);
                Restart_I2C();
                CyU3PThreadSleep (50);
                if (retryCount > 0)
                {
                    retryCount--;
                    // Put this buffer at the start of the Queue
                    Status = CyU3PQueuePrioritySend(&I2C_DebugQueue, &FilledBuffer, CYU3P_NO_WAIT);
                    if (Status != CY_U3P_SUCCESS) CyU3PDebugPrint(4, "Unable to re-queue data buffer");
                }
            }
            else	// Successfully displayed the message, wrap up
            {
            	retryCount = I2C_RETRY_COUNT;				// Reset Retry Count
            	CyU3PDmaBufferFree(FilledBuffer.buffer);	// Give back the original buffer
            }
        }
    }
}

CyU3PReturnStatus_t I2C_DebugInit(uint8_t TraceLevel)
{
    CyU3PI2cConfig_t i2cConfig;
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PReturnStatus_t Status;
    CyU3PSemaphore ThreadSignal;
    void* StackPtr;

    if (I2C_DebugEnabled) return CY_U3P_ERROR_ALREADY_STARTED;

    Status = CyU3PI2cInit();    // Start the I2C driver
    CheckStatus("CyU3PI2cInit", Status);

    i2cConfig.bitRate    = CY_FX_USBI2C_I2C_BITRATE;
    i2cConfig.busTimeout = -1;
    i2cConfig.dmaTimeout = -1;
    i2cConfig.isDma      = CyTrue;
    Status = CyU3PI2cSetConfig(&i2cConfig, NULL);
    CheckStatus("Set I2C Config", Status);

    // Create MANUAL DMA channels to send and receive data from the I2C IO block
    CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    // Get a set of buffers to output debug messages
    dmaConfig.size = CY_U3P_DEBUG_DMA_BUFFER_SIZE;
    dmaConfig.count = 0;
    dmaConfig.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaConfig.consSckId = CY_U3P_LPP_SOCKET_I2C_CONS;
    dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
    Status = CyU3PDmaChannelCreate(&I2C_DebugTXHandle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig);
    CheckStatus("CyU3PDmaChannelCreate(I2C_TX)", Status);
    // Console In Buffer will be assigned manually
    dmaConfig.size = I2C_CONSOLEIN_BUFFER_SIZE; // 0 should work here, but CyU3PDmaChannelCreate says it is an error
    dmaConfig.count = 0;
    dmaConfig.prodSckId = CY_U3P_LPP_SOCKET_I2C_PROD;
    dmaConfig.consSckId = CY_U3P_CPU_SOCKET_CONS;
    Status = CyU3PDmaChannelCreate(&I2C_DebugRXHandle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
    CheckStatus("CyU3PDmaChannelCreate(I2C_RX)", Status);

    // Create a Mutex and a Queue for the I2C_Console to use
    Status = CyU3PMutexCreate(&I2C_DebugLock, CYU3P_NO_INHERIT);
    CheckStatus("I2C_Debug Mutex", Status);
    Status = CyU3PQueueCreate(&I2C_DebugQueue, sizeof (CyU3PDmaBuffer_t), Queue, sizeof(Queue));
    CheckStatus("I2C_Debug Queue", Status);

    // I need to create a thread that will manage the Queue
    // I also need a signal to let me know that this thread is running
    Status = CyU3PSemaphoreCreate(&ThreadSignal, 0);
    CheckStatus("ThreadSignal SemaphoreCreate", Status);
    StackPtr = CyU3PMemAlloc(DEBUG_THREAD_STACK_SIZE);
    Status = CyU3PThreadCreate(&I2C_DebugThread,        // Handle to my Application Thread
            "30:I2C_Debug_Thread",                      // Thread ID and name
            I2C_ConsoleThread,                          // Thread entry function
            (uint32_t)&ThreadSignal,                    // Parameter passed to Thread
            StackPtr,                                   // Pointer to the allocated thread stack
            DEBUG_THREAD_STACK_SIZE,                    // Allocated thread stack size
            DEBUG_THREAD_PRIORITY,                      // Thread priority
            DEBUG_THREAD_PRIORITY,                      // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,                        // Time slice no supported
            CYU3P_AUTO_START                            // Start the thread immediately
            );
    CheckStatus("Create I2C_Debug_Thread", Status);

    // Wait for the thread to be set up
    Status = CyU3PSemaphoreGet(&ThreadSignal, CYU3P_WAIT_FOREVER);

    I2C_DebugTraceLevel = TraceLevel;

    I2C_DebugEnabled = CyTrue;

    return Status;
}

