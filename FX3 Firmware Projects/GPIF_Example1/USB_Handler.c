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

// Declare external data
extern CyU3PEvent CallbackEvent;			// Used for events detected in CallBack routines
extern CyBool_t glIsApplicationActive;		// Set true once device is enumerated
extern uint32_t ClockValue;					// Used to select GPIF speed
extern uint32_t MAXCLOCKVALUE;

// Global data owned by this module
volatile uint8_t  glVendorRequestCount = 0;
volatile uint32_t glUnderrunCount = 0;
uint8_t glEp0Buffer[32] __attribute__ ((aligned (32))); /* Local buffer used for vendor command handling. */
CyBool_t glForceLinkU2;
CyBool_t glChannelSuspended;


// Declare the callbacks needed to support the USB device driver
CyBool_t USBSetup_Callback(uint32_t setupdat0, uint32_t setupdat1)
{
	CyBool_t isHandled = CyFalse;
	union { uint32_t SetupData[2];
			struct { uint8_t Target:5; uint8_t Type:2; uint8_t Direction:1;
				 	 uint8_t Request; uint16_t Value; uint16_t Index; uint16_t Length; };
		  } Setup;
	// Copy the incoming Setup Packet into my Setup union which will "unpack" the variables
	Setup.SetupData[0] = setupdat0;
	Setup.SetupData[1] = setupdat1;

	// USB Driver will send me Class and Vendor requests to handle
    if (Setup.Type == VENDOR_REQUEST)
    {
        if (Setup.Request == 0xB5)
        {
        	DebugPrint(4, "\r\nGot Command = %s Data Collection", Setup.Value ? "Start" : "Stop");
        	// CollectData can remotely Start and Stop data collection, don't implement this in Example1
        	if (Setup.Value == 1) CyU3PGpioSetValue(CPLD_RESET, 0);		// Start = Release CPLD_RESET
        	if (Setup.Value == 0) CyU3PGpioSetValue(CPLD_RESET, 1);		// Stop = Drive CPLD_RESET
        	CyU3PUsbAckSetup();
            isHandled = CyTrue;
        }
        if (Setup.Request == 0x76)
        {
            /* When operating at USB 2.0 speeds, there is a possibility that the bulk transfers create
               errors during control transfers. Ensure that the bulk channel is suspended for the duration
               of the control request to avoid this possibility. We use a timeout of 1 second to ensure
               that the control pipe does not get stuck in the case where the host is not reading the
               bulk pipe. */
            glEp0Buffer[0] = glVendorRequestCount++;
            glEp0Buffer[1] = glUnderrunCount;
            glEp0Buffer[2] = 1;
            glEp0Buffer[3] = 2;
            CyU3PUsbSendEP0Data(Setup.Length, glEp0Buffer);
            isHandled = CyTrue;
        }
    }

    if (Setup.Type == STANDARD_REQUEST)
    {
        /* Handle SET_FEATURE(FUNCTION_SUSPEND) and CLEAR_FEATURE(FUNCTION_SUSPEND)
         * requests here. It should be allowed to pass if the device is in configured
         * state and failed otherwise. */
        if ((Setup.Target == CY_U3P_USB_TARGET_INTF) && ((Setup.Request == CY_U3P_USB_SC_SET_FEATURE)
                    || (Setup.Request == CY_U3P_USB_SC_CLEAR_FEATURE)) && (Setup.Value == 0))
        {
            if (glIsApplicationActive)
            {
                CyU3PUsbAckSetup ();
                /* As we have only one interface, the link can be pushed into U2 state as soon as
                   this interface is suspended.
                 */
                glForceLinkU2 = (Setup.Request == CY_U3P_USB_SC_SET_FEATURE);
             }
            else CyU3PUsbStall(0, CyTrue, CyFalse);

            isHandled = CyTrue;
        }

        /* CLEAR_FEATURE request for endpoint is always passed to the setup callback
         * regardless of the enumeration model used. When a clear feature is received,
         * the previous transfer has to be flushed and cleaned up. This is done at the
         * protocol level. Since this is just a loopback operation, there is no higher
         * level protocol. So flush the EP memory and reset the DMA channel associated
         * with it. If there are more than one EP associated with the channel reset both
         * the EPs. The endpoint stall and toggle / sequence number is also expected to be
         * reset. Return CyFalse to make the library clear the stall and reset the endpoint
         * toggle. Or invoke the CyU3PUsbStall (ep, CyFalse, CyTrue) and return CyTrue.
         * Here we are clearing the stall. */
        if ((Setup.Target == CY_U3P_USB_TARGET_ENDPT) && (Setup.Request == CY_U3P_USB_SC_CLEAR_FEATURE)
                && (Setup.Value == CY_U3P_USBX_FS_EP_HALT))
        {
            if (glIsApplicationActive)
            {
                if (Setup.Index == CONSUMER_ENDPOINT)
                {
                    CyU3PUsbStall(Setup.Index, CyFalse, CyTrue);
                    isHandled = CyTrue;
                    CyU3PUsbAckSetup();
                }
            }
        }
    }
    return isHandled;
}

void USBEvent_Callback(CyU3PUsbEventType_t Event, uint16_t EventData )
{
	CyU3PEventSet(&CallbackEvent, 1<<Event, CYU3P_EVENT_OR);
	switch (Event)
    {
     case CY_U3P_USB_EVENT_CONNECT:
       break;

    case CY_U3P_USB_EVENT_SETCONF:
        /* If the application is already active stop it before re-enabling. */
        if (glIsApplicationActive) StopApplication();

        MAXCLOCKVALUE = (CyU3PUsbGetSpeed() == CY_U3P_SUPER_SPEED) ? 10 : 80;	//	DEBUG at 10MHz with USB 2.0
        ClockValue = MAXCLOCKVALUE;
        StartApplication();
        break;

    case CY_U3P_USB_EVENT_DISCONNECT:
    case CY_U3P_USB_EVENT_RESET:
        glForceLinkU2 = CyFalse;
        if (glIsApplicationActive) StopApplication();
        break;

    case CY_U3P_USB_EVENT_EP_UNDERRUN:
    	glUnderrunCount++;
        DebugPrint (7, "\r\nEP Underrun on %d", EventData);
        break;

    case CY_U3P_USB_EVENT_EP0_STAT_CPLT:
        /* Make sure the bulk pipe is resumed once the control transfer is done. */
        break;

    default:
        break;
    }
}

CyBool_t LPMRequest_Callback(CyU3PUsbLinkPowerMode link_mode)
{
    return CyTrue;
}

// Spin up USB, let the USB driver handle enumeration
CyU3PReturnStatus_t InitializeUSB(void)
{
	CyU3PReturnStatus_t Status;
	CyBool_t NeedToRenumerate = CyTrue;
	Status = CyU3PUsbStart();
    if (Status == CY_U3P_ERROR_NO_REENUM_REQUIRED)
    {
    	NeedToRenumerate = CyFalse;
    	Status = CY_U3P_SUCCESS;
    }
	CheckStatus("Start USB Driver", Status);
	// Setup callbacks to handle the setup requests, USB Events and LPM Requests (for USB 3.0)
    CyU3PUsbRegisterSetupCallback(USBSetup_Callback, CyTrue);
    CyU3PUsbRegisterEventCallback(USBEvent_Callback);
    CyU3PUsbRegisterLPMRequestCallback(LPMRequest_Callback);

    // Driver needs all of the descriptors so it can supply them to the host when requested
    Status = SetUSBdescriptors();
    CheckStatus("Set USB Descriptors", Status);

    // Connect the USB Pins with SuperSpeed operation enabled
    if (NeedToRenumerate)
    {
    	Status = CyU3PConnectState(CyTrue, CyTrue);
    	CheckStatus("Connect USB", Status);
    }
    else	// USB connection already exists, restart the Application
    {
        if (glIsApplicationActive) StopApplication();
        StartApplication();
    }

    return Status;
}




