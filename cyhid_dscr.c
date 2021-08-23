/*
 ## Cypress FX3 Example Source File (cyhid_dscr.c)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2018,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

/* This file contains the USB descriptors for the HID application example */
#include "cyfx3_hid.h"

/* Standard Device Descriptor for 2.0 */
const uint8_t CyFxUSB20DeviceDscr[] __attribute__ ((aligned (32))) =
{
    0x12,                           /* Descriptor Size */
    CY_U3P_USB_DEVICE_DESCR,        /* Device Descriptor Type */
    0x00,0x02,                      /* USB 2.1 */
    0x00,                           /* Device Class */
    0x00,                           /* Device Sub-class */
    0x00,                           /* Device protocol */
    0x40,                           /* Maxpacket size for EP0 : 64 bytes */
    0x7E,0x05,                      /* Vendor ID */
    0x09,0x20,                      /* Product ID */
    0x10,0x02,                      /* Device release number */
    0x01,                           /* Manufacture string index */
    0x02,                           /* Product string index */
    0x03,                           /* Serial number string index */
    0x01                            /* Number of configurations */
};

/* Standard Device Descriptor for USB 3.0 */
const uint8_t CyFxUSB30DeviceDscr[] __attribute__ ((aligned (32))) =
{
    0x12,                           /* Descriptor Size */
    CY_U3P_USB_DEVICE_DESCR,        /* Device Descriptor Type */
    0x10,0x03,                      /* USB 3.1 */
    0x00,                           /* Device Class */
    0x00,                           /* Device Sub-class */
    0x00,                           /* Device protocol */
    0x09,                           /* Maxpacket size for EP0 : 2^9 */
    0xB4,0x04,                      /* Vendor ID */
    0x25,0x60,                      /* Product ID */
    0x00,0x00,                      /* Device release number */
    0x01,                           /* Manufacture string index */
    0x02,                           /* Product string index */
    0x03,                           /* Serial number string index */
    0x01                            /* Number of configurations */
};

/* Standard Device Qualifier Descriptor */
const uint8_t CyFxUSBDeviceQualDscr[] __attribute__ ((aligned (32))) =
{
    0x0A,                           /* Descriptor Size */
    CY_U3P_USB_DEVQUAL_DESCR,       /* Device Qualifier Descriptor Type */
    0x01,0x02,                      /* USB 2.1 */
    0x00,                           /* Device Class */
    0x00,                           /* Device Sub-class */
    0x00,                           /* Device protocol */
    0x40,                           /* Maxpacket size for EP0 : 64 bytes */
    0x01,                           /* Number of configurations */
    0x00                            /* Reserved */
};

/* Standard Full Speed Configuration Descriptor */
const uint8_t CyFxUSBFSConfigDscr[] __attribute__ ((aligned (32))) =
{
    0x09,
    0x02,
    0x29,
    0x00,
    0x01,
    0x01,
    0x00,
    0xA0,
    0xFA,
    0x09,
    0x04,
    0x00,
    0x00,
    0x02,
    0x03,
    0x00,
    0x00,
    0x00,
    0x09,
    0x21,
    0x11,
    0x01,
    0x00,
    0x01,
    0x22,
    0xCB,
    0x00,
    0x07,
    0x05,
    0x81,
    0x03,
    0x40,
    0x00,
    0x08,
    0x07,
    0x05,
    0x01,
    0x03,
    0x40,
    0x00,
    0x08

};

/* HID Report Descriptor */
const uint8_t CyFxUSBReportDscr[] __attribute__ ((aligned (32))) =
{
		0x05,0x01,0x15,0x00 ,0x09,0x04,0xA1,0x01 ,0x85,0x30,0x05,0x01 ,0x05,0x09,0x19,0x01,
		0x29,0x0A,0x15,0x00 ,0x25,0x01,0x75,0x01 ,0x95,0x0A,0x55,0x00 ,0x65,0x00,0x81,0x02,
		0x05,0x09,0x19,0x0B ,0x29,0x0E,0x15,0x00 ,0x25,0x01,0x75,0x01 ,0x95,0x04,0x81,0x02,
		0x75,0x01,0x95,0x02 ,0x81,0x03,0x0B,0x01 ,0x00,0x01,0x00,0xA1 ,0x00,0x0B,0x30,0x00,

		0x01,0x00,0x0B,0x31, 0x00,0x01,0x00,0x0B, 0x32,0x00,0x01,0x00, 0x0B,0x35,0x00,0x01,
		0x00,0x15,0x00,0x27, 0xFF,0xFF,0x00,0x00, 0x75,0x10,0x95,0x04, 0x81,0x02,0xC0,0x0B,
		0x39,0x00,0x01,0x00, 0x15,0x00,0x25,0x07, 0x35,0x00,0x46,0x3B, 0x01,0x65,0x14,0x75,
		0x04,0x95,0x01,0x81, 0x02,0x05,0x09,0x19, 0x0F,0x29,0x12,0x15, 0x00,0x25,0x01,0x75,

		0x01,0x95,0x04,0x81, 0x02,0x75,0x08,0x95, 0x34,0x81,0x03,0x06, 0x00,0xFF,0x85,0x21,
		0x09,0x01,0x75,0x08, 0x95,0x3F,0x81,0x03, 0x85,0x81,0x09,0x02, 0x75,0x08,0x95,0x3F,
		0x81,0x03,0x85,0x01, 0x09,0x03,0x75,0x08, 0x95,0x3F,0x91,0x83, 0x85,0x10,0x09,0x04,
		0x75,0x08,0x95,0x3F, 0x91,0x83,0x85,0x80, 0x09,0x05,0x75,0x08, 0x95,0x3F,0x91,0x83,

		0x85,0x82,0x09,0x06, 0x75,0x08,0x95,0x3F, 0x91,0x83,0xC0

};

/* Standard High Speed Configuration Descriptor */
const uint8_t CyFxUSBHSConfigDscr[] __attribute__ ((aligned (32))) =
{
	    0x09,
	    0x02,
	    0x29,
	    0x00,
	    0x01,
	    0x01,
	    0x00,
	    0xA0,
	    0xFA,
	    0x09,
	    0x04,
	    0x00,
	    0x00,
	    0x02,
	    0x03,
	    0x00,
	    0x00,
	    0x00,
	    0x09,
	    0x21,
	    0x11,
	    0x01,
	    0x00,
	    0x01,
	    0x22,
	    0xCB,
	    0x00,
	    0x07,
	    0x05,
	    0x81,
	    0x03,
	    0x40,
	    0x00,
	    0x08,
	    0x07,
	    0x05,
	    0x01,
	    0x03,
	    0x40,
	    0x00,
	    0x08
};

/* Binary Device Object Store Descriptor */
const uint8_t CyFxUSBBOSDscr[] __attribute__ ((aligned (32))) =
{
    0x05,                           /* Descriptor Size */
    CY_FX_BOS_DSCR_TYPE,            /* Device Descriptor Type */
    0x16,0x00,                      /* Length of this descriptor and all sub descriptors */
    0x02,                           /* Number of device capability descriptors */

    /* USB 2.0 Extension */
    0x07,                           /* Descriptor Size */
    CY_FX_DEVICE_CAPB_DSCR_TYPE,    /* Device Capability Type descriptor */
    CY_U3P_USB2_EXTN_CAPB_TYPE,     /* USB 2.0 Extension Capability Type */
    0x1E,0x64,0x00,0x00,            /* Supported device level features: LPM support, BESL supported,
                                       Baseline BESL=400 us, Deep BESL=1000 us. */

    /* SuperSpeed Device Capability */
    0x0A,                           /* Descriptor Size */
    CY_FX_DEVICE_CAPB_DSCR_TYPE,    /* Device Capability Type descriptor */
    CY_U3P_SS_USB_CAPB_TYPE,        /* SuperSpeed Device Capability Type */
    0x00,                           /* Supported device level features  */
    0x0E,0x00,                      /* Speeds Supported by the device : SS, HS and FS */
    0x03,                           /* Functionality support */
    0x00,                           /* U1 Device Exit Latency */
    0x00,0x00                       /* U2 Device Exit Latency */
};

/* Standard Super Speed Configuration Descriptor */
const uint8_t CyFxUSBSSConfigDscr[] __attribute__ ((aligned (32))) =
{
    /* Configuration Descriptor Type */
    0x09,                           /* Descriptor Size */
    CY_U3P_USB_CONFIG_DESCR,        /* Configuration Descriptor Type */
    0x28,0x00,                      /* Length of this descriptor and all sub descriptors */
    0x01,                           /* Number of interfaces */
    0x01,                           /* Configuration number */
    0x00,                           /* Configuration string index */
    0xC0,                           /* Self powered device. */
    0x0C,                           /* Max power consumption of device (in 8mA unit) : 96 mA */

    /* Interface Descriptor */
    0x09,                           /* Descriptor size */
    CY_U3P_USB_INTRFC_DESCR,        /* Interface Descriptor type */
    CY_FX_USB_HID_INTF,             /* Interface number */
    0x00,                           /* Alternate setting number */
    0x01,                           /* Number of end points */
    0x03,                           /* Interface class */
    0x00,                           /* Interface sub class : None */
    0x02,                           /* Interface protocol code */
    0x00,                           /* Interface descriptor string index */

    /* HID Descriptor (Mouse) */
    0x09,                           /* Descriptor size */
    CY_FX_USB_HID_DESC_TYPE,        /* Descriptor Type */
    0x10,0x11,                      /* HID Class Spec 11.1 */
    0x00,                           /* Target Country */
    0x01,                           /* Total HID Class Descriptors */
    0x22,                           /* Report Descriptor Type */
    0x1C,0x00,                      /* Total Length of Report Descriptor */      

    /* Endpoint Descriptor (Mouse) */
    0x07,                           /* Descriptor size */
    CY_U3P_USB_ENDPNT_DESCR,        /* Endpoint Descriptor Type */
    CY_FX_HID_EP_INTR_IN,           /* Endpoint address and description */
    CY_U3P_USB_EP_INTR,             /* Bulk End point Type */
    0x02,0x00,                      /* Max packet size = 2 bytes */
    0x05,                           /* Servicing interval is 2 ** (5 - 1) = 16 Intervals = 2 ms. */

    /* Super Speed Endpoint Companion Descriptor (Mouse) */
    0x06,                           /* Descriptor size */
    CY_FX_SS_EP_COMPN_DSCR_TYPE,    /* SS Endpoint Companion Descriptor Type */
    0x00,                           /* Max no. of packets in a Burst. */
    0x00,                           /* No streaming for Interrupt Endpoints. */
    0x02,0x00                       /* Number of bytes per interval = 2. */
};


/* Standard Language ID String Descriptor */
const uint8_t CyFxUSBStringLangIDDscr[] __attribute__ ((aligned (32))) =
{
    0x04,                           /* Descriptor Size */
    CY_U3P_USB_STRING_DESCR,        /* String Descriptor Type */
    0x09,0x04                       /* Language ID supported: US English */
};

/* Standard Manufacturer String Descriptor */
const uint8_t CyFxUSBManufactureDscr[] __attribute__ ((aligned (32))) =
{
		0x26,0x03,0x4E,0x00,0x69,0x00,0x6E,0x00,0x74,0x00,0x65,0x00,0x6E,0x00,0x64,0x00,
		0x6F,0x00,0x20,0x00,0x43,0x00,0x6F,0x00,0x2E,0x00,0x2C,0x00,0x20,0x00,0x4C,0x00,
		0x74,0x00,0x64,0x00,0x2E,0x00
};


/* Standard Product String Descriptor */
const uint8_t CyFxUSBProductDscr[] __attribute__ ((aligned (32))) =
{
		0x1E,0x03,0x50,0x00,0x72,0x00,0x6F,0x00,0x20,0x00,0x43,0x00,0x6F,0x00,0x6E,0x00,
		0x74,0x00,0x72,0x00,0x6F,0x00,0x6C,0x00,0x6C,0x00,0x65,0x00,0x72,0x00
};

/* Product Serial Number String Descriptor */
const uint8_t CyFxUSBSerialNumberDscr[] __attribute__ ((aligned (32))) =
{
		0x1A,0x03,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,
		0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x31,0x00
};

/*[]*/

