// Demonstrate GPIF as a Slave FIFO sending and receiving data from the CPLD
//
// GPIF_Example6 uses SlaveFifoReadWrite.h and CPLD requires CPLDasFifoMaster.xsvf
// Preset Button[6] = 0 to enable READ from CPLD and = 1 to WRITE to CPLD
//
// There are some changes from the implementation described in the first edition of the book:
//  1	My development board worked successfully but some production boards did not operate correctly with the PushButton
//		The problem was tracked down to mechanical button bounce
//		A preferred fix was to put a button debouncer in the CPLD but there were not enough resources
//		So now the FX3 debounces the button on the CPLD's behalf and passes a CPLD_PUSH_BUTTON signal to it
//	2	On a WRITE the FX3 no longer produces a LastRDData signal - this allows the example to be used repeatedly with no resets
//		On a READ the CPLD produces a LastWRData signal - this is detected by the GPIF state machine which COMMITs the last short packet
//  3	I was incorrectly using DebugPrint in several CallBack routines - I now set an Event and use DebugPrint in Main context
//	4	If a High Speed USB connection is made then PCLK is reduced to 10MHz and this allows debugging with simpler tools
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void DebugPrintEvent(uint32_t ActualEvents);
extern void ParseCommand(void);

uint32_t ClockValue;				// Used to select GPIF speed
CyU3PThread ThreadHandle;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;		// Set true once device is enumerated
uint32_t Counter[12];				// Some DEBUG counters
TX_TIMER DebounceTimer;				// Timer used to debounce PushButton
CyU3PEvent CallbackEvent;			// Used by Callback to signal Main()


void DebounceTimerExpired(ULONG NotUsed)
{
	// PushButton has finished bouncing, copy its current value to the CPLD
	CyBool_t Value;
	CyU3PGpioSimpleGetValue(PUSH_BUTTON, &Value);
	CyU3PGpioSetValue(CPLD_PUSH_BUTTON, Value);
}

void GPIO_CallBack(uint8_t gpioId)
{
	// At each detected edge of the PushButton restart the Debounce Timer
	UINT Active;
	uint32_t RemainingTicks;
	if (gpioId == PUSH_BUTTON)
	{
// Resync the Debounce Timer to the change on the PushButton
		tx_timer_info_get(&DebounceTimer, 0, &Active, &RemainingTicks, 0, 0);
		tx_timer_deactivate(&DebounceTimer);
		tx_timer_change(&DebounceTimer, RemainingTicks+DEBOUNCE_TIME, DEBOUNCE_TIME);
		tx_timer_activate(&DebounceTimer);
	}
}

void InitializeCPLD(void)
// CPLD needs to be RESET for correct operation
{
	CyU3PReturnStatus_t Status;
	CyU3PGpioClock_t GpioClock;
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyBool_t Value;

    // Startup the GPIO module clocks
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    CheckStatus("Start GPIO Clocks", Status);

    // Need to claim CTRL[9] and CTRL[10] from the GPIF Interface
	Status = CyU3PDeviceGpioOverride(CPLD_PUSH_BUTTON, CyTrue);
	CheckStatus("CPLD_RUN_STOP Override", Status);
	Status = CyU3PDeviceGpioOverride(CPLD_RESET, CyTrue);
	CheckStatus("CPLD_RESET Override", Status);

	// Reset by driving CPLD_RESET High
	CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = 1;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    Status = CyU3PGpioSetSimpleConfig(CPLD_RESET, &gpioConfig);
    CheckStatus("Reset CPLD", Status);
    // Initial values for CPLD_PUSH_BUTTON = 0
    gpioConfig.outValue = 0;
    Status = CyU3PGpioSetSimpleConfig(CPLD_PUSH_BUTTON, &gpioConfig);
    CheckStatus("Set CPLD_PUSH_BUTTON", Status);
    // Setup PushButton as an input that can generate an interrupt
    gpioConfig.outValue = 1;
    gpioConfig.inputEn = CyTrue;
    gpioConfig.driveLowEn = CyFalse;
    gpioConfig.driveHighEn = CyFalse;
    gpioConfig.intrMode = CY_U3P_GPIO_INTR_BOTH_EDGE;
    Status = CyU3PGpioSetSimpleConfig(PUSH_BUTTON, &gpioConfig);
    CheckStatus("Setup PUSH_BUTTON", Status);
    // CPLD can also drive the PushButton, ensure that it isn't (ie check Value = 1)
	CyU3PGpioSimpleGetValue(PUSH_BUTTON, &Value);
	DebugPrint(4, ", Initial Value = %d,", Value);
    // Register a CallBack to deal with PushButton
    CyU3PRegisterGpioCallBack(GPIO_CallBack);
}

void ApplicationThread(uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;
    uint32_t ActualEvents, Count;

// Insert a delay here if using a USB Bus Spy to give time to start capture after the FX3 firmware has been loaded and started
//    DebugPrint(4, "\r\nReady:");
//    CyU3PThreadSleep(10000);

    Status = InitializeUSB();
    CheckStatus("Initialize USB", Status);

    if (Status == CY_U3P_SUCCESS)
    {
		// Wait for the device to be enumerated and initialized
		while (!glIsApplicationActive)
		{
			// Wait up to 100msec for USB CallBack Events
			Status = CyU3PEventGet(&CallbackEvent, USB_EVENTS, CYU3P_EVENT_OR_CLEAR, &ActualEvents, 100);
			if (Status == TX_SUCCESS) DebugPrintEvent(ActualEvents);
		}

		DebugPrint(4, "\r\nApplication started with %d", Value);
		CyU3PGpioSetValue(CPLD_RESET, 0);		// Release CPLD_RESET
    	// Now run forever
    	while (1)
    	{
    		for (Count = 0; Count<10; Count++)
    		{
				// Check for User Commands (and other CallBack Events) every 100msec
				CyU3PThreadSleep(100);
				Status = CyU3PEventGet(&CallbackEvent, ANY_EVENT, CYU3P_EVENT_OR_CLEAR, &ActualEvents, TX_NO_WAIT);
				if (Status == TX_SUCCESS)
				{
					if (ActualEvents & USER_COMMAND_AVAILABLE) ParseCommand();
					else DebugPrintEvent(ActualEvents);
				}
    		}
			DebugPrint(4, "%d, ", Seconds++);
		}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    // Returning here will stop the Application Thread running - it failed anyway so this is OK
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status;

    // Now create any needed resources then the Application thread
    Status = InitializeDebugConsole();
    CheckStatus("Initialize Debug Console", Status);

    // GPIO module already started, need to Initialize CPLD
    InitializeCPLD();

    // Need a system timer to debounce the pushbutton
    Status = tx_timer_create(&DebounceTimer, "DebounceTimer", DebounceTimerExpired, 0, DEBOUNCE_TIME, DEBOUNCE_TIME, TX_AUTO_ACTIVATE);
    CheckStatus("Create DebounceTimer", Status);

    // Create an Event so that alerts from CallBack routines can be monitored
    Status = CyU3PEventCreate(&CallbackEvent);
    CheckStatus("Create CallbackEvent", Status);

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle, 	// Handle to my Application Thread
            "11:GPIF_Example6",               	// Thread ID and name
            ApplicationThread,     				// Thread entry function
            42,                             	// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,           // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,        // Thread priority
            APPLICATION_THREAD_PRIORITY,        // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice not supported
            CYU3P_AUTO_START                	// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS)
    {
        // Thread creation failed with the Status = error code
        while(1)
        {
        	// Application cannot continue. Loop indefinitely
        }
    }
}

