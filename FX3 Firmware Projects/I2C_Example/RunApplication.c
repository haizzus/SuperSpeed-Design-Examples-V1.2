// Keyboard.c - demonstrate USB by enumerating as a keyboard
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(uint8_t TraceLevel);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyU3PThread ApplicationThreadHandle;	// Handle to my Application Thread
CyBool_t glIsApplicationActive;			// Set true once device is enumerated

CyU3PReturnStatus_t SetupGPIO(void)
{
	CyU3PReturnStatus_t Status;
	CyU3PGpioClock_t GpioClock;
    CyU3PGpioSimpleConfig_t GpioConfig;

    // Startup the GPIO module
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    CheckStatus("\r\nStart GPIO Module", Status);

    // I have assigned GPIF_CTRL[10] as the CPLD RESET pin
    Status = CyU3PDeviceGpioOverride(CPLD_RESET, CyTrue);
    CheckStatus("\r\nClaim CPLD_RESET", Status);
    CyU3PMemSet((uint8_t *)&GpioConfig, 0, sizeof(GpioConfig));
    GpioConfig.outValue = 1;
//r	GpioConfig.inputEn = CyFalse;
    GpioConfig.driveLowEn = CyTrue;
    GpioConfig.driveHighEn = CyTrue;
//r	GpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
    Status = CyU3PGpioSetSimpleConfig(CPLD_RESET, &GpioConfig);

	return Status;
}

CyU3PReturnStatus_t I2C_Init(void)
{
	CyU3PI2cConfig_t i2cConfig;
	CyU3PReturnStatus_t Status;

    Status = CyU3PI2cInit();										// Start the I2C driver
    CheckStatus("CyU3PI2cInit", Status);

    i2cConfig.bitRate    = CY_FX_USBI2C_I2C_BITRATE;
    i2cConfig.busTimeout = 0xFFFFFFFF;
    i2cConfig.dmaTimeout = 0xFFFF;
    i2cConfig.isDma      = CyFalse;
    Status = CyU3PI2cSetConfig(&i2cConfig, NULL);
    CheckStatus("CyU3PI2cSetConfig", Status);

    return Status;
}

uint8_t ReadButtons(void)
{
	CyU3PReturnStatus_t Status;
    CyU3PI2cPreamble_t preamble;
    uint8_t Value;
    preamble.length    = 1;
    preamble.buffer[0] = (DeviceAddress<<1) | 1;
    preamble.ctrlMask  = 0x0000;

    Status = CyU3PI2cReceiveBytes(&preamble, &Value, 1, 0);
    CheckStatus("I2C_Read", Status);
    CyU3PDebugPrint(4, "\r\nButtons = %x", Value);
	return Value;
}

void WriteLEDs(uint8_t Value)
{
//	CyU3PDebugPrint(4, "\r\nLEDs = %d, ", Value);
	CyU3PReturnStatus_t Status;
    CyU3PI2cPreamble_t preamble;
	preamble.length    = 1;
    preamble.buffer[0] = DeviceAddress<<1;
    preamble.ctrlMask  = 0x0000;

    Status = CyU3PI2cTransmitBytes(&preamble, &Value, 1, 0);
    CheckStatus("I2C_Write", Status);

    /* Wait for the write to complete. */
    preamble.length = 1;
    Status = CyU3PI2cWaitForAck(&preamble, 10);
    CheckStatus("I2C_WaitForAck", Status);
}

void ApplicationThread(uint32_t Value)
{
	int32_t Seconds = 0;
	uint8_t Buttons = 0xAA;
    CyU3PReturnStatus_t Status;

    Status = SetupGPIO();		// Needed for CPLD_Reset = GPIF_CTRL[10]
    CheckStatus("GPIO Initialized", Status);

    Status = InitializeDebugConsole(9);
    CheckStatus("Debug Console Initialized", Status);

    // Remove Reset from the CPLD
    CyU3PThreadSleep(10);
    CyU3PGpioSetValue(CPLD_RESET, 0);

    // Give me time to start up my Ellisys Explorer
//	CyU3PThreadSleep(10000);

    if (Status == CY_U3P_SUCCESS)
    {
    	Status = I2C_Init();
        CheckStatus("I2C_Init", Status);

        // Now run forever
    	while (1)
    	{
    		CyU3PThreadSleep(100);
			WriteLEDs(Buttons);
//			CyU3PDebugPrint(4, "%d, %d: ", Seconds, Buttons);
    		CyU3PThreadSleep(100);
			Buttons = ReadButtons();
    		Seconds++;
		}
    }
    CyU3PDebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}


