/*
 * USB Handler.c
 *
 *      Author: john@usb-by-example.com
 */

#include "Application.h"

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern CyU3PReturnStatus_t SetUSBdescriptors(void);
extern void StartApplication(void);
extern void StopApplication(void);
extern uint32_t BitPosition(uint32_t Value);

// Declare external data
extern CyBool_t glIsApplicationActive;			// Set true once device is enumerated
extern const uint8_t ReportDescriptor[];
extern CyU3PEvent DisplayEvent;					// Used to display events

uint8_t glEP0Buffer[32] __attribute__ ((aligned(32)));

// Declare the callbacks needed to support the USB device driver
CyBool_t USBSetup_Callback(uint32_t setupdat0, uint32_t setupdat1)
{
	CyBool_t isHandled = CyFalse;
	CyU3PReturnStatus_t Status;
	uint16_t Count;
	union { uint32_t SetupData[2];
			uint8_t RawBytes[8];
			struct { uint8_t Target:5; uint8_t Type:2; uint8_t Direction:1;
				 	 uint8_t Request; uint16_t Value; uint16_t Index; uint16_t Length; };
		  } Setup;
	// Copy the incoming Setup Packet into my Setup union which will "unpack" the variables
	Setup.SetupData[0] = setupdat0;
	Setup.SetupData[1] = setupdat1;
#if (0)
	// Included for DEBUG to display each sub field in the USB Command if needed
	// Note that we are in a Callback so shouldn't really using DebugPrint
	uint32_t i;
//	DebugPrint(4, "\r\nSetup Input %X,%X", setupdat0, setupdat1);
//	DebugPrint(4, "\r\nRaw Bytes: ");
//	for (i=0; i<8; i++) DebugPrint(4, "%x,", Setup.RawBytes[i]);
	DebugPrint(4, "\r\nDirection:%d", Setup.Direction);
	DebugPrint(4, ", Type:%d", Setup.Type);
	DebugPrint(4, ", Target:%d", Setup.Target);
	DebugPrint(4, ", Request:%X", Setup.Request);
	DebugPrint(4, "\r\nValue:%X", Setup.Value);
	DebugPrint(4, ", Index:%d", Setup.Index);
	DebugPrint(4, ", Length:%d", Setup.Length);
#endif
	// USB Driver will send me Class and Vendor requests to handle
	// I only have to handle three class requests for a Keyboard
    if (Setup.Target == CLASS_REQUEST)
    {
    	if (Setup.Direction == 0)		// Host-to-Device
    	{
			if (Setup.Request == HID_SET_REPORT)
			{
				CyU3PUsbGetEP0Data(sizeof(glEP0Buffer), glEP0Buffer, &Count);
				isHandled = CyTrue;
//				DebugPrint(4, "\r\nSet LEDs = 0x%x\r\n", glEP0Buffer[0]);
				// 'Set LEDs' moved to char* EventName[30]
				CyU3PEventSet(&DisplayEvent, 1<<30, CYU3P_EVENT_OR);
			}
			if (Setup.Request == HID_SET_IDLE)
			{
				CyU3PUsbAckSetup();
				isHandled = CyTrue;
//				DebugPrint(4, "\r\nSent Ack to Set Idle");
				// 'Ack to Set Idle' moved to char* EventName[29]
				CyU3PEventSet(&DisplayEvent, 1<<29, CYU3P_EVENT_OR);
			}
    	}
    	else							// Device-to-Host
    	{
			if ((Setup.Request == CY_U3P_USB_SC_GET_DESCRIPTOR) && ((Setup.Value >> 8) == CY_U3P_USB_REPORT_DESCR))
			{
				Status = CyU3PUsbSendEP0Data(59, (uint8_t*)ReportDescriptor);
//				CheckStatus("Send Report Descriptor", Status);
				// 'Send Report Descriptor' moved to char* EventName[28]
				CyU3PEventSet(&DisplayEvent, 1<<28, CYU3P_EVENT_OR);
				isHandled = CyTrue;
			}
    	}
    }
	return isHandled;
}

// For Debug and education display the name of the Event
// This structure moved to Support.c
//const char* EventName[] = {
//	    "CONNECT", "DISCONNECT", "SUSPEND", "RESUME", "RESET", "SET_CONFIGURATION", "SPEED",
//	    "SET_INTERFACE", "SET_EXIT_LATENCY", "SOF_ITP", "USER_EP0_XFER_COMPLETE", "VBUS_VALID",
//	    "VBUS_REMOVED", "HOSTMODE_CONNECT", "HOSTMODE_DISCONNECT", "OTG_CHANGE", "OTG_VBUS_CHG",
//	    "OTG_SRP", "EP_UNDERRUN", "LINK_RECOVERY", "USB3_LINKFAIL", "SS_COMP_ENTRY", "SS_COMP_EXIT"
//};


void USBEvent_Callback(CyU3PUsbEventType_t Event, uint16_t EventData )
{
//	Don't use DebugPrint in a Callback, set an event and this will be displayed later
//	DebugPrint(4, "\r\nEvent received = %s", EventName[Event]);
	CyU3PEventSet(&DisplayEvent, BitPosition((uint32_t)Event), CYU3P_EVENT_OR);
    switch (Event)
    {
        case CY_U3P_USB_EVENT_SETCONF:
            /* Stop the application before re-starting. */
            if (glIsApplicationActive) StopApplication();
            StartApplication();
            break;

        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_CONNECT:
        case CY_U3P_USB_EVENT_DISCONNECT:
            if (glIsApplicationActive)
            {
                CyU3PUsbLPMEnable();
                StopApplication();
            }
            break;

        default:
            break;
    }
}

CyBool_t LPMRequest_Callback(CyU3PUsbLinkPowerMode link_mode)
{
	// This is a keyboard, I can Suspend between keystrokes
    return CyTrue;
}

// Spin up USB, let the USB driver handle enumeration
CyU3PReturnStatus_t InitializeUSB(void)
{
	CyU3PReturnStatus_t Status;
	Status = CyU3PUsbStart();
	CheckStatus("Start USB Driver", Status);
	// Setup callbacks to handle the setup requests, USB Events and LPM Requests (for USB 3.0)
    CyU3PUsbRegisterSetupCallback(USBSetup_Callback, CyTrue);
    CyU3PUsbRegisterEventCallback(USBEvent_Callback);
    CyU3PUsbRegisterLPMRequestCallback(LPMRequest_Callback);

    // Driver needs all of the descriptors so it can supply them to the host when requested
    Status = SetUSBdescriptors();
    CheckStatus("Set USB Descriptors", Status);

    // Connect the USB Pins with super speed operation enabled
    ////// I have a problem with my USB 3.0 Descriptors and this does not enumerate at SuperSpeed
    ////// Get an Ellisys trace and FIX!
    Status = CyU3PConnectState(CyTrue, CyTrue);
    CheckStatus("Connect USB", Status);

    return Status;
}




