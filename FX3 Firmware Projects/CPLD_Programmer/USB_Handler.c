/*
 * USB Handler.c
 *
 */

#include "Application.h"

// Declare external data
CyU3PDmaChannel glDmaChannelHandle;

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void StartApplication(void);
extern void StopApplication(void);
extern CyBool_t ApplicationIsActive(void);
CyU3PReturnStatus_t SetUSBdescriptors(void);

// For Debug and education display the name of the Event
const char* EventName[] = {
#if (VERBOSE)
	    "CONNECT", "DISCONNECT", "SUSPEND", "RESUME", "RESET", "SET_CONFIGURATION", "SPEED",
	    "SET_INTERFACE", "SET_EXIT_LATENCY", "SOF_ITP", "USER_EP0_XFER_COMPLETE", "VBUS_VALID",
	    "VBUS_REMOVED", "HOSTMODE_CONNECT", "HOSTMODE_DISCONNECT", "OTG_CHANGE", "OTG_VBUS_CHG",
	    "OTG_SRP", "EP_UNDERRUN", "LINK_RECOVERY", "USB3_LINKFAIL", "SS_COMP_ENTRY", "SS_COMP_EXIT"
#endif
};

/* Callback to handle the USB setup requests. */
CyBool_t USBSetupCallback(uint32_t setupdat0, uint32_t setupdat1)
{
	CyBool_t isHandled = CyFalse;
	union { uint32_t SetupData[2];
//			uint8_t RawBytes[8];
			struct { uint8_t Target:5; uint8_t Type:2; uint8_t Direction:1;
				 	 uint8_t Request; uint16_t Value; uint16_t Index; uint16_t Length; };
		  } Setup;
	// Copy the incoming Setup Packet into my Setup union which will "unpack" the variables
	Setup.SetupData[0] = setupdat0;
	Setup.SetupData[1] = setupdat1;

    if (Setup.Type == CY_U3P_USB_STANDARD_RQT)
    {
        /* Handle SET_FEATURE(FUNCTION_SUSPEND) and CLEAR_FEATURE(FUNCTION_SUSPEND)
         * requests here. It should be allowed to pass if the device is in configured
         * state and failed otherwise. */
        if ((Setup.Target == CY_U3P_USB_TARGET_INTF) && ((Setup.Request == CY_U3P_USB_SC_SET_FEATURE)
                    || (Setup.Request == CY_U3P_USB_SC_CLEAR_FEATURE)) && (Setup.Value == 0))
        {
            if (ApplicationIsActive()) CyU3PUsbAckSetup();
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
            if ((Setup.Index == EP_PRODUCER) || (Setup.Index == EP_CONSUMER))
            {
                if (ApplicationIsActive())
                {
                    CyU3PDmaChannelReset(&glDmaChannelHandle);
                    CyU3PUsbFlushEp(EP_PRODUCER);
                    CyU3PUsbResetEp(EP_PRODUCER);
                    CyU3PUsbFlushEp(EP_CONSUMER);
                    CyU3PUsbResetEp(EP_CONSUMER);
                    CyU3PDmaChannelSetXfer(&glDmaChannelHandle, 0);
                    CyU3PUsbStall(Setup.Index, CyFalse, CyTrue);
                    CyU3PUsbAckSetup();
                    isHandled = CyTrue;
                }
            }
        }
    }

    return isHandled;
}

/* This is the callback function to handle the USB events. */
void USBEventCallback(CyU3PUsbEventType_t EventType, uint16_t EventData)
{
	DebugPrint(4, "\r\nEvent received = %s", EventName[EventType]);
    switch (EventType)
    {
        case CY_U3P_USB_EVENT_SETCONF:
            /* Stop the application before re-starting. */
            if (ApplicationIsActive()) StopApplication();
            StartApplication();
            break;

        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_DISCONNECT:
            if (ApplicationIsActive()) StopApplication();
            break;

        default:
            break;
    }
}

// This application does not have any state in which we should not allow U1/U2 transitions, therefore always return CyTrue.
CyBool_t LPMRequestCallback(CyU3PUsbLinkPowerMode link_mode)
{
    return CyTrue;
}

// Spin up USB, let the USB driver handle enumeration
CyU3PReturnStatus_t InitializeUSB(void)
{
    CyU3PReturnStatus_t Status;

    // Spin up the USB Driver
    Status = CyU3PUsbStart();
    CheckStatus("Start USB driver", Status);
    // Let the driver handle most of the enumeration
    // I need to handle unknown class and vendor requests
	// Setup callbacks to handle the setup requests, USB Events and LPM Requests (for USB 3.0)
    CyU3PUsbRegisterSetupCallback(USBSetupCallback, CyTrue);
    CyU3PUsbRegisterLPMRequestCallback(LPMRequestCallback);
    CyU3PUsbRegisterEventCallback(USBEventCallback);

    // Driver needs all of the descriptors so it can supply them to the host when requested
    Status = SetUSBdescriptors();
    CheckStatus("Set USB Descriptors", Status);

    /* Connect the USB Pins with super speed operation enabled. */
    Status = CyU3PConnectState(CyTrue, CyTrue);

	return Status;
}




