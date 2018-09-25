/*
 * KeyboardDescriptors.c
 *
 *  Created on: Feb 17, 2014
 *      Author: John
 */

// This file contains the USB enumeration descriptors for a Keyboard and a routine that sets them
// The keyboard supports Boot protocol = 8 byte Interrupt In packets and 1 byte Set Feature Out packets
// Note that there are alignment requirements for descriptors
// This uses my VID = 0x4242 - use is permitted for development but NOT as a product
//

#include "Application.h"

extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

/* Standard device descriptor for USB 3.0 */
const uint8_t CyFxUSB30DeviceDscr[] __attribute__ ((aligned (32))) =
{
    0x12,                           /* Descriptor size */
    CY_U3P_USB_DEVICE_DESCR,        /* Device descriptor type */
    0x00,0x03,                      /* USB 3.0 */
    0x00,                           /* Device class */
    0x00,                           /* Device sub-class */
    0x00,                           /* Device protocol */
    0x09,                           /* Maxpacket size for EP0 : 2^9 */
    0x42,0x42,                      /* Vendor ID */
    0x02,0xEC,                      /* Product ID */
    0x00,0x00,                      /* Device release number */
    0x01,                           /* Manufacturer string index */
    0x02,                           /* Product string index */
    0x00,                           /* Serial number string index */
    0x01                            /* Number of configurations */
};

/* Standard device descriptor for USB 2.0 */
const uint8_t CyFxUSB20DeviceDscr[] __attribute__ ((aligned (32))) =
{
    0x12,                           /* Descriptor size */
    CY_U3P_USB_DEVICE_DESCR,        /* Device descriptor type */
    0x10,0x02,                      /* USB 2.10 */
    0x00,                           /* Device class */
    0x00,                           /* Device sub-class */
    0x00,                           /* Device protocol */
    0x40,                           /* Maxpacket size for EP0 : 64 bytes */
    0x42,0x42,                      /* Vendor ID */
    0x01,0xEC,                      /* Product ID */
    0x00,0x00,                      /* Device release number */
    0x01,                           /* Manufacturer string index */
    0x02,                           /* Product string index */
    0x00,                           /* Serial number string index */
    0x01                            /* Number of configurations */
};

/* Binary device object store descriptor */
const uint8_t CyFxUSBBOSDscr[] __attribute__ ((aligned (32))) =
{
    0x05,                           /* Descriptor size */
    CY_U3P_BOS_DESCR,               /* Device descriptor type */
    0x16,0x00,                      /* Length of this descriptor and all sub descriptors */
    0x02,                           /* Number of device capability descriptors */

    /* USB 2.0 extension */
    0x07,                           /* Descriptor size */
    CY_U3P_DEVICE_CAPB_DESCR,       /* Device capability type descriptor */
    CY_U3P_USB2_EXTN_CAPB_TYPE,     /* USB 2.0 extension capability type */
    0x02,0x00,0x00,0x00,            /* Supported device level features: LPM support  */

    /* SuperSpeed device capability */
    0x0A,                           /* Descriptor size */
    CY_U3P_DEVICE_CAPB_DESCR,       /* Device capability type descriptor */
    CY_U3P_SS_USB_CAPB_TYPE,        /* SuperSpeed device capability type */
    0x00,                           /* Supported device level features  */
    0x0E,0x00,                      /* Speeds supported by the device : SS, HS and FS */
    0x03,                           /* Functionality support */
    0x00,                           /* U1 Device Exit latency */
    0x00,0x00                       /* U2 Device Exit latency */
};

/* Standard device qualifier descriptor */
const uint8_t CyFxUSBDeviceQualDscr[] __attribute__ ((aligned (32))) =
{
    0x0A,                           /* Descriptor size */
    CY_U3P_USB_DEVQUAL_DESCR,       /* Device qualifier descriptor type */
    0x00,0x02,                      /* USB 2.0 */
    0x00,                           /* Device class */
    0x00,                           /* Device sub-class */
    0x00,                           /* Device protocol */
    0x40,                           /* Maxpacket size for EP0 : 64 bytes */
    0x01,                           /* Number of configurations */
    0x00                            /* Reserved */
};

// Report Descriptor, needed by all interfaces
const uint8_t ReportDescriptor[] __attribute__ ((aligned (32))) =
{
	5, 1,			// Usage Page(Generic Desktop)
	9, 6,			// Usage (Keyboard)
	0xA1, 1,		// Collection(Application)
	// First declare the 8 byte input report for Keycodes
	5, 7,			//	Usage Page(Keycodes)
	0x19, 224,		//	Usage Minimum
	0x29, 231,		//	Usage Maximum
	0x15, 0,		//	Logical Minimum
	0x25, 1,		//	Logical Maximum
	0x75, 1,		//	Report Size = 1 bit
	0x95, 8,		//	Report Count
	0x81, 2,		//	Input(DataAbsolute) = Modifier byte
	0x81, 1,		//	Input(Constant) = Reserved byte
	0x75, 8,		//	Report Size = 1 byte
	0x95, 6,		//	Report Count
// I only support printable ASCII keys (and CR)
	0x19, 0,		//	Usage Minimum
	0x29, 0x38,		//	Usage Maximum
	0x15, 0,		//	Logical Minimum
	0x25, 0x38,		//	Logical Maximum
	0x81, 0,		//	Input(DataArray) = 6 Keycodes
	// Now declare the 5 bit input report for LEDs
	5, 8,			//	Usage Page(LEDs)
	0x19, 1,		//	Usage Minimum
	0x29, 5,		//	Usage Maximum
	0x15, 0,		//	Logical Minimum
	0x25, 1,		//	Logical Maximum
	0x75, 1,		//	Report Size = 1 bit
	0x95, 5,		//	Report Count
	0x91, 2,		//	Output(DataAbsolute) = LEDs
	0x95, 3,		//	Report Count
	0x91, 1,		//	Output(Constant) = LED padding to next byte boundary
	0xC0			// End Collection
};

/* Standard super speed configuration descriptor */
const uint8_t CyFxUSBSSConfigDscr[] __attribute__ ((aligned (32))) =
{
    /* Configuration descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_CONFIG_DESCR,        /* Configuration descriptor type */
    9+9+9+7+6, 0,               	/* Length of this descriptor and all sub descriptors */
    1,                           	/* Number of interfaces */
    1,                           	/* Configuration number */
    0,                           	/* Configuration string index */
    0x80,                           /* Config characteristics - Bus powered */
    12,                           	/* Max power consumption of device (in 8mA unit) : 96mA */

    /* HID Interface descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_INTRFC_DESCR,        /* Interface Descriptor type */
    0,                           	/* Interface number */
    0,                        	   	/* Alternate setting number */
    1,                       	    /* Number of end points */
    3,                     	      	/* HID class */
    1,                           	/* Boot Interface sub class */
    1,                           	/* Keyboard protocol code */
    0,                           	/* Interface descriptor string index */

    /* HID descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_HID_DESCR,        	/* HID Class */
    0x00, 0x01,                     /* HID Specification Level */
    0,                           	/* HID Target Country */
    1,                           	/* Number of HID Class descriptors */
    CY_U3P_USB_REPORT_DESCR,        /* HID class type */
    sizeof(ReportDescriptor), 0,    /* Total length of Report Descriptor */

    /* Endpoint descriptor for consumer EP */
    7,                           	/* Descriptor size */
    CY_U3P_USB_ENDPNT_DESCR,        /* Endpoint descriptor type */
    CY_FX_EP_CONSUMER,              /* Endpoint address and description */
    CY_U3P_USB_EP_INTR,             /* Interrupt endpoint type */
    8, 0,                      		/* Max packet size = 8 bytes */
    8,                           	/* Servicing interval for data transfers */

    /* Super speed endpoint companion descriptor for consumer EP */
    6,                           	/* Descriptor size */
    CY_U3P_SS_EP_COMPN_DESCR,       /* SS endpoint companion descriptor type */
    1,    							/* Max no. of packets in a burst(0-15) - 0: burst 1 packet at a time */
    0,                           	/* Max streams for interrupt endpoint = 0 (No streams) */
    8, 0,                      		/* Service interval for the endpoint */

};

/* Standard high speed configuration descriptor */
// AND
/* Standard full speed configuration descriptor */
const uint8_t CyFxUSBHSFSConfigDscr[] __attribute__ ((aligned (32))) =
{
    /* Configuration descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_CONFIG_DESCR,        /* Configuration descriptor type */
    9+9+9+7, 0,                     /* Length of this descriptor and all sub descriptors */
    1,                           	/* Number of interfaces */
    1,                           	/* Configuration number */
    0,                           	/* Configuration string index */
    0x80,                           /* Config characteristics - bus powered */
    50,                           	/* Max power consumption of device (in 2mA unit) : 100mA */

    /* HID Interface descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_INTRFC_DESCR,        /* Interface Descriptor type */
    0,                           	/* Interface number */
    0,                        	   	/* Alternate setting number */
    1,                       	    /* Number of end points */
    3,                     	      	/* HID class */
    1,                           	/* Boot Interface sub class */
    1,                           	/* Keyboard protocol code */
    0,                           	/* Interface descriptor string index */

    /* HID descriptor */
    9,                           	/* Descriptor size */
    CY_U3P_USB_HID_DESCR,        	/* HID Class */
    0x00, 0x01,                     /* HID Specification Level */
    0,                           	/* HID Target Country */
    1,                           	/* Number of HID Class descriptors */
    CY_U3P_USB_REPORT_DESCR,        /* HID class type */
    sizeof(ReportDescriptor), 0,    /* Total length of Report Descriptor */

    /* Endpoint descriptor for consumer EP */
    7,                           	/* Descriptor size */
    CY_U3P_USB_ENDPNT_DESCR,        /* Endpoint descriptor type */
    CY_FX_EP_CONSUMER,              /* Endpoint address and description */
    CY_U3P_USB_EP_INTR,             /* Interrupt endpoint type */
    8, 0,                      		/* Max packet size = 8 bytes */
    8,                           	/* Servicing interval for data transfers */
 };


/* Standard language ID string descriptor */
const uint8_t CyFxUSBStringLangIDDscr[] __attribute__ ((aligned (32))) =
{
    0x04,                           /* Descriptor size */
    CY_U3P_USB_STRING_DESCR,        /* Device descriptor type */
    0x09,0x04                       /* Language ID supported */
};

/* Standard manufacturer string descriptor */
const uint8_t CyFxUSBManufactureDscr[] __attribute__ ((aligned (32))) =
{
    44,                             /* Descriptor size */
    CY_U3P_USB_STRING_DESCR,        /* Device descriptor type */
    'U',0,'S',0,'B',0,' ',0,'D',0,'e',0,'s',0,'i',0,'g',0,'n',0,' ',0,
    'B',0,'y',0,' ',0,'E',0,'x',0,'a',0,'m',0,'p',0,'l',0,'e',0
};

/* Standard product string descriptor */
const uint8_t CyFxUSBProductDscr[] __attribute__ ((aligned (32))) =
{
    18,                           	/* Descriptor size */
    CY_U3P_USB_STRING_DESCR,        /* Device descriptor type */
    'K',0,'e',0,'y',0,'b',0,'o',0,'a',0,'r',0,'d',0
};

CyU3PReturnStatus_t SetUSBdescriptors(void)
{
	CyU3PReturnStatus_t OverallStatus, Status;
	OverallStatus = Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_DEVICE_DESCR, NULL, (uint8_t *)CyFxUSB30DeviceDscr);
    CheckStatus("SET_SS_DEVICE_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_DEVICE_DESCR, NULL, (uint8_t *)CyFxUSB20DeviceDscr);
    CheckStatus("SET_HS_DEVICE_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_BOS_DESCR, NULL, (uint8_t *)CyFxUSBBOSDscr);
    CheckStatus("SET_SS_BOS_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_DEVQUAL_DESCR, NULL, (uint8_t *)CyFxUSBDeviceQualDscr);
    CheckStatus("SET_DEVQUAL_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBSSConfigDscr);
    CheckStatus("SET_SS_CONFIG_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBHSFSConfigDscr);
    CheckStatus("SET_HS_CONFIG_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_FS_CONFIG_DESCR, NULL, (uint8_t *)CyFxUSBHSFSConfigDscr);
    CheckStatus("SET_FS_CONFIG_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 0, (uint8_t *)CyFxUSBStringLangIDDscr);
    CheckStatus("SET_STRING0_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 1, (uint8_t *)CyFxUSBManufactureDscr);
    CheckStatus("SET_STRING1_DESCR", Status);
    OverallStatus |= Status = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 2, (uint8_t *)CyFxUSBProductDscr);
    CheckStatus("SET_STRING2_DESCR", Status);

    return OverallStatus;
}
