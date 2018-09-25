// SPI Example - this example uses the CPLD board and the SPI connection
//
// The CPLD should have a SPI image in it (I2C_Slave_And_SPI.xsvf is the best choice)
//
// Since UART is not available when SPI is used this example uses an I2C Debug Console
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

CyU3PThread ThreadHandle[APP_THREADS];		// Handles to my Application Threads
void *StackPtr[APP_THREADS];				// Each thread has its own stack
CyU3PMutex SPI_CS_Mutex;					// Used to control access to a shared resource

void SelectSPI_Device(uint32_t DeviceID)
{
    CyU3PReturnStatus_t Status;
	// I use CTRL[9:8] as Address lines for SPI Devices, these have been assigned as GPIOs
    // The address is pre-selected prior to any SPI commands, CPLD routes SSN
	// I use a Mutex to ensure that only one thread can control these lines at any one time
    // Call with DeviceID = -1 to Deselect the device ASAP
    if (DeviceID == -1)
    {
    	// Deselect the SPI device
		Status = CyU3PMutexPut(&SPI_CS_Mutex);
		CheckStatus("Return SPI_CS Mutex", Status);
		CyU3PGpioSimpleSetValue(SPI_Address0, 1);
		CyU3PGpioSimpleSetValue(SPI_Address1, 1);
    }
    else
    {
		Status = CyU3PMutexGet(&SPI_CS_Mutex, CYU3P_WAIT_FOREVER);
		CheckStatus("Get SPI_CS Mutex", Status);
		CyU3PGpioSimpleSetValue(SPI_Address0, DeviceID & 1);
		CyU3PGpioSimpleSetValue(SPI_Address1, (DeviceID>>1) & 1);
	}
}

CyU3PReturnStatus_t ConfigureSPI(uint8_t Mode)
{
    CyU3PReturnStatus_t Status;
    CyU3PSpiConfig_t spiConfig;
	CyU3PMemSet ((uint8_t *)&spiConfig, 0, sizeof(spiConfig));
	spiConfig.clock      = 1000000;						// Run at 33MHz
	spiConfig.wordLen    = 8;								// Byte transfers
//	spiConfig.isLsbFirst = CyFalse;
	spiConfig.cpol       = CyTrue;
//	spiConfig.ssnPol     = CyFalse;
//	spiConfig.cpha       = CyFalse;
	spiConfig.leadTime   = CY_U3P_SPI_SSN_LAG_LEAD_ONE_CLK;
	spiConfig.lagTime    = CY_U3P_SPI_SSN_LAG_LEAD_ONE_CLK;
    if (Mode == RegisterMode) spiConfig.ssnCtrl = CY_U3P_SPI_SSN_CTRL_HW_EACH_WORD;	// Single byte transfers
    if (Mode == DMA_Mode) spiConfig.ssnCtrl = CY_U3P_SPI_SSN_CTRL_HW_END_OF_XFER;	// Block transfers
	Status = CyU3PSpiSetConfig(&spiConfig, NULL);
	CheckStatus("Configure SPI Channel", Status);
	return Status;
}

CyU3PReturnStatus_t InitializeSPI(void)
{
    CyU3PReturnStatus_t Status;
    CyU3PGpioSimpleConfig_t gpioConfig;

    // I need a Mutex to allow control over the SPI CS Lines
    Status = CyU3PMutexCreate(&SPI_CS_Mutex, CYU3P_INHERIT);
    CheckStatus("Create Mutex", Status);
    // Reallocate CTRL[9:8] = GPIO[26:25] to be SPI address select lines
    Status = CyU3PDeviceGpioOverride(SPI_Address0, CyTrue);
    CheckStatus("Override SPI_Address0", Status);
    Status = CyU3PDeviceGpioOverride(SPI_Address1, CyTrue);
    CheckStatus("Override SPI_Address1", Status);
    CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    Status = CyU3PGpioSetSimpleConfig(SPI_Address0, &gpioConfig);
    CheckStatus("Configure SPI_Address0", Status);
    Status = CyU3PGpioSetSimpleConfig(SPI_Address1, &gpioConfig);
    CheckStatus("Configure SPI_Address1", Status);

    // Start the SPI device driver then configure the master SPI port
    Status = CyU3PSpiInit();
    CheckStatus("Start SPI Driver", Status);

    if (Status == CY_U3P_SUCCESS) Status = ConfigureSPI(0);
	return Status;
}

void ReadSwitchesWriteLED_Thread(void)
{
	CyU3PReturnStatus_t Status;
	uint32_t* SPI_DataIn_Register = (uint32_t*)0x0E0000C14;
	uint8_t Value = 0;
	uint32_t InValue;
	// Use register mode for single byte Read and Writes
	while (1)
	{
		SelectSPI_Device(SwitchesLEDs);
		Status = ConfigureSPI(RegisterMode);
		// The Cypress API is "Transmit" or "Receive", my CPLD does both at the same time
		Status = CyU3PSpiTransmitWords(&Value, 1);
		// Retrieve the input byte from the SPI DataIn Register
		InValue = *SPI_DataIn_Register & 0x0FF;
		// There was a problem with InValue so, for now, show a counter on the LEDs
//		DebugPrint(4,"%d=", InValue);
//		Status = CyU3PSpiReceiveWords((uint8_t*)&InValue, 1);
//		DebugPrint(4,"%d.", InValue);
		Value--;
		SelectSPI_Device(-1);
		CyU3PThreadSleep(100);
	}
}

void ApplicationThread (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;

    // Start up the UART Console - I couldn't get the application to start without this
	Status = InitializeDebugConsole(6);
    CheckStatus("Debug Console Initialized", Status);

    if (Status == CY_U3P_SUCCESS)
    {

    	// Start up the I2C Console
    	Status = I2C_DebugInit(8);
        CheckStatus("I2C_DebugInit", Status);
        // DebugPrint is now directed at the I2C Debug Console
        DebugPrint(4, "\r\nSPI Example\r\nApplication started with %d\r\n", Value);

	   // Now turn off the UART console - this shouldn't be on!
        Status = CyU3PDebugDeInit();
        CheckStatus("Disable UART", Status);
        CyU3PThreadSleep(1000);
        Status = InitializeSPI();
        CheckStatus("Initialize_SPI", Status);

        // Start a thread to transfer SPI switched to SPI LEDs
        StackPtr[1] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
        CyU3PThreadCreate(&ThreadHandle[1],			// Handle for this Thread
                "20:ReadSwitchesWriteLEDs",			// Thread ID and name
                ReadSwitchesWriteLED_Thread,  		// Thread function
                42,                             	// Parameter passed to Thread
                StackPtr[1],                       	// Pointer to the allocated thread stack
                APPLICATION_THREAD_STACK,			// Allocated thread stack size
                APPLICATION_THREAD_PRIORITY,		// Thread priority
                APPLICATION_THREAD_PRIORITY,		// = Thread priority so no preemption
                CYU3P_NO_TIME_SLICE,            	// Time slice no supported
                CYU3P_AUTO_START);                	// Start the thread immediately

        // Now run forever
    	while (1)
    	{
    		CyU3PThreadSleep(1000);
			DebugPrint(4, "%d, ", Seconds);
			Seconds++;
			CheckForCommand();		// Check for commands in Main context and execute them in Main context
    	}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    CyU3PReturnStatus_t Status;
    CyU3PGpioSimpleConfig_t gpioConfig;
    // If I get here then RTOS has started correctly, turn off ErrorIndicator
    IndicateError(0);

    // This examples uses the CPLD which must be RESET
    // I have reallocated CTRL[10] = GPIO[27] for this
    Status = CyU3PDeviceGpioOverride(CPLD_RESET, CyTrue);
    CheckStatus("Override CPLD_RESET", Status);
    CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = CyTrue;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    Status = CyU3PGpioSetSimpleConfig(CPLD_RESET, &gpioConfig);
    CyU3PThreadSleep(10);
	CyU3PGpioSimpleSetValue(CPLD_RESET, 0);

    // Create an Application thread
    StackPtr[0] = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    CyU3PThreadCreate(&ThreadHandle[0],			// Handle for this Thread
            "15:SPI_Example",                	// Thread ID and name
            ApplicationThread,  				// Thread function
            42,                             	// Parameter passed to Thread
            StackPtr[0],                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,			// Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,		// Thread priority
            APPLICATION_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice no supported
            CYU3P_AUTO_START);                	// Start the thread immediately
}

