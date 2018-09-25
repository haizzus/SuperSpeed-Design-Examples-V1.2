// Keyboard.c - demonstrate USB by enumerating as a keyboard
//
// john@usb-by-example.com
//
// This is an enhancement of the Keyboard in the book (which is now called Keyboard0)
// The program got into several hangs due to my use of DebugPrint in a Callback routines
// Yes, the book tells you not to do this.  So this example follow my advice!
// I now set flags in a DisplayEvent EventGroup and use BackgoundPrint to print these in main context
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void BackgroundPrint(uint32_t TimeToWait);

extern CyU3PDmaChannel glCPUtoUSB_Handle;
extern CyU3PEvent DisplayEvent;			// Used to display events

CyU3PThread ApplicationThread;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;			// Set true once device is enumerated

const uint8_t Ascii2Usage[] = {
// Create a lookup table that uses the ASCII character as an index and produces a Modifier/Usage Code pair
	0, 0x2C, 2, 0x1E, 2, 0x34, 2, 0x20, 2, 0x21, 2, 0x22, 2, 0x24, 0, 0x34,		// 20..27    !"#$%&'
	2, 0x26, 2, 0x27, 2, 0x23, 2, 0x2E, 0, 0x36, 0, 0x2D, 0, 0x37, 0, 0x38,		// 28..2F   ()*+,-./
	0, 0x27, 0, 0x1E, 0, 0x1F, 0, 0x20, 0, 0x21, 0, 0x22, 0, 0x23, 0, 0x24,		// 28..2F   01234567
	0, 0x25, 0, 0x26, 2, 0x33, 0, 0x33, 2, 0x36, 0, 0x2E, 2, 0x37, 2, 0x38,		// 28..2F   89:;<=>?
	2, 0x1F, 2, 0x04, 2, 0x05, 2, 0x06, 2, 0x07, 2, 0x08, 2, 0x09, 0, 0x0A,		// 00..07 ^ @ABCDEFG
	2, 0x0B, 2, 0x0C, 2, 0x0D, 2, 0x0E, 2, 0x0F, 2, 0x10, 2, 0x11, 2, 0x12,		// 08..1F ^ HIJKLMNO
	2, 0x13, 2, 0x14, 2, 0x15, 2, 0x16, 2, 0x17, 2, 0x18, 2, 0x19, 2, 0x1A,		// 10..17 ^ PQRSTUVW
	2, 0x1B, 2, 0x1C, 2, 0x1D, 0, 0x2F, 0, 0x31, 0, 0x30, 2, 0x23, 2, 0x2D,		// 18..1F ^ XYZ[\]^_
	0, 0x2C, 0, 0x04, 0, 0x05, 0, 0x06, 0, 0x07, 0, 0x08, 0, 0x09, 0, 0x0A,		// 00..07 ^ @abcdefg
	0, 0x0B, 0, 0x0C, 0, 0x0D, 0, 0x0E, 0, 0x0F, 0, 0x10, 0, 0x11, 0, 0x12,		// 08..1F ^ hijklmno
	0, 0x13, 0, 0x14, 0, 0x15, 0, 0x16, 0, 0x17, 0, 0x18, 0, 0x19, 0, 0x1A,		// 10..17 ^ pqrstuvw
	0, 0x1B, 0, 0x1C, 0, 0x1D, 2, 0x2F, 2, 0x31, 2, 0x30, 2, 0x35, 0, 0x28		// 18..1F ^ xyz{|}~
};


void SendKeystroke(char InputChar)
// In this example characters typed on the debug console are sent as key strokes
// The format of a keystroke is defined in the report descriptor; it is 8 bytes long = Modifier, Reserved, UsageCode[6]
// A keyboard will send two reports, one for key press and one for key release
// A 'standard' keyboard can encode up to 6 key usages in one report, this example only does 1
// CheckStatus calls commented out following debug, reinsert them if needed
{
	uint16_t Index;
	CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
	CyU3PDmaBuffer_t ReportBuffer;
	// The only Control character I handle is CR
	if (InputChar == 0x0D) InputChar = 0x7F;
	if (InputChar > 0x1F)
	{
		Index = (((uint8_t)InputChar & 0x7F)-0x20)<<1;		// Each entry is two uint8_t
		// First need a buffer to build the report
		Status = CyU3PDmaChannelGetBuffer(&glCPUtoUSB_Handle, &ReportBuffer, CYU3P_WAIT_FOREVER);
//		CheckStatus("GetReportBuffer4KeyPress", Status);
		// Most of this report will be 0's
		ReportBuffer.count = REPORT_SIZE;
		CyU3PMemSet(ReportBuffer.buffer, 0, REPORT_SIZE);
		// Convert InputChar to a Modifier and a Usage
		ReportBuffer.buffer[0] = Ascii2Usage[Index++];
		ReportBuffer.buffer[2] = Ascii2Usage[Index];
		// Send the Key Press to the host
		Status = CyU3PDmaChannelCommitBuffer(&glCPUtoUSB_Handle, REPORT_SIZE, 0);
		CheckStatus("Send KeyPress ", Status);
		// Wait 50msec then send a Key Release
		CyU3PThreadSleep(50);
		Status = CyU3PDmaChannelGetBuffer(&glCPUtoUSB_Handle, &ReportBuffer, CYU3P_WAIT_FOREVER);
		CheckStatus("GetReportBuffer4KeyRelease", Status);
		CyU3PMemSet(ReportBuffer.buffer, 0, REPORT_SIZE);
		ReportBuffer.count = REPORT_SIZE;
		Status = CyU3PDmaChannelCommitBuffer(&glCPUtoUSB_Handle, REPORT_SIZE, 0);
//		CheckStatus("Send KeyRelease", Status);
	}
}


void ApplicationThread_Entry (uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

    Status = InitializeDebugConsole();
    CheckStatus("Debug Console Initialized", Status);

    // Create an Event Group that Callbacks can use to signal events to a Background DebugPrint
    Status = CyU3PEventCreate(&DisplayEvent);
    CheckStatus("Create Event", Status);

    // Give me time to start up my Ellisys Explorer
    DebugPrint(4, "\r\n\r\n\r\nStart Ellisys NOW\r\n");
    CyU3PThreadSleep (10000);

    Status = InitializeUSB();
    CheckStatus("USB Initialized", Status);

    // Wait for the USB connection to be up
    while (!glIsApplicationActive) BackgroundPrint(10);

    if (Status == CY_U3P_SUCCESS)
    {
    	DebugPrint(4, "\r\nApplication started with %d\r\n", Value);
    	// Now run forever
    	while (1)
    	{
    		DebugPrint(4, "%d, ", Seconds++);
			BackgroundPrint(1000);
		}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    while (1);		// Hang here
}


