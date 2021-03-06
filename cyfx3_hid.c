/*
 ## Cypress FX3 Example source file (cyfx3_hid.c)
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

/* This file implements HID (Mouse) Firmware Application source code. Application on getting
 * interrupt from Switch Press, will send multiple reports to the HID host for generating a
 * circular pattern, which can be verified by tracking the movement of cursor on monitor. */

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3dma.h>
#include <cyu3error.h>
#include <cyu3usb.h>
#include <cyu3uart.h>
#include <cyu3utils.h>
#include <cyu3gpio.h>

#include "cyfx3_hid.h"

CyU3PThread UsbHidAppThread; /* HID application thread structure */
static CyU3PDmaChannel glChHandleIntrCPU2U; /* DMA Channel handle */
static CyU3PDmaChannel glChHandleIntrU2CPU; /* DMA Channel handle */
static uint32_t glButtonPress = 0; /* Button Press Counter */
static CyBool_t glIsApplnActive = CyFalse; /* HID application Active Flag */

uint8_t ReportDataDefault[] = { 0x30, 0x31, 0x91, 0x00, 0x80, 0x00, 0x48, 0xB8,
		0x86, 0x06, 0x28, 0x77, 0x02, 0xC8, 0x01, 0x1E, 0x05, 0x46, 0x0F, 0xEB,
		0xFF, 0x15, 0x00, 0x1A, 0x00, 0xCC, 0x01, 0x21, 0x05, 0x3C, 0x0F, 0xE9,
		0xFF, 0x19, 0x00, 0x21, 0x00, 0xCB, 0x01, 0x24, 0x05, 0x30, 0x0F, 0xE9,
		0xFF, 0x19, 0x00, 0x21 };
uint8_t committime = 0;

struct Buttons
{
	CyBool_t DPAD_DOWN;
	CyBool_t DPAD_RIGHT;
	CyBool_t DPAD_LEFT;
	CyBool_t DPAD_UP;
	CyBool_t SL;
	CyBool_t SR;
	CyBool_t MINUS;
	CyBool_t HOME;
	CyBool_t PLUS;
	CyBool_t CAPTURE;
	CyBool_t STICK;
	CyBool_t SHOULDER_1;
	CyBool_t SHOULDER_2;
	CyBool_t B;
	CyBool_t A;
	CyBool_t Y;
	CyBool_t X;
	CyBool_t STICK2;
	CyBool_t SHOULDER2_1;
	CyBool_t SHOULDER2_2;
}ButtonMap[10]=
{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

int ButtonIndex=0;
static CyU3PTimer ButtonChange;
#define ButtonChangePeriod 1000
//Every 2 seconds change the button map
uint8_t timestamp=0x00;

static void ButtonIndexChange(uint32_t arg){
	ButtonIndex++;
	if(ButtonIndex>9) ButtonIndex=0;
}


/* Application Error Handler */
void CyFxAppErrorHandler(CyU3PReturnStatus_t apiRetStatus /* API return status */
) {
	/* Application failed with the error code apiRetStatus */

	/* Add custom debug or recovery actions here */

	/* Loop Indefinitely */
	for (;;) {
		/* Thread sleep : 100 ms */
		CyU3PThreadSleep(100);
	}
}

/* This function initializes the debug module. The debug prints
 * are routed to the UART and can be seen using a UART console
 * running at 115200 baud rate. */
void CyFxUsbHidApplnDebugInit(void) {
	CyU3PUartConfig_t uartConfig;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	/* Initialize the UART for printing debug messages */
	apiRetStatus = CyU3PUartInit();
	if (apiRetStatus != CY_U3P_SUCCESS) {
		/* Error handling */
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Set UART configuration */
	CyU3PMemSet((uint8_t *) &uartConfig, 0, sizeof(uartConfig));
	uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
	uartConfig.stopBit = CY_U3P_UART_ONE_STOP_BIT;
	uartConfig.parity = CY_U3P_UART_NO_PARITY;
	uartConfig.txEnable = CyTrue;
	uartConfig.rxEnable = CyFalse;
	uartConfig.flowCtrl = CyFalse;
	uartConfig.isDma = CyTrue;

	apiRetStatus = CyU3PUartSetConfig(&uartConfig, NULL);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Set the UART transfer to a really large value. */
	apiRetStatus = CyU3PUartTxSetBlockXfer(0xFFFFFFFF);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Initialize the debug module. */
	apiRetStatus = CyU3PDebugInit(CY_U3P_LPP_SOCKET_UART_CONS, 8);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyFxAppErrorHandler(apiRetStatus);
	}

	CyU3PDebugPreamble(CyFalse);
}

/* This function starts the HID application. This is called
 * when a SET_CONF event is received from the USB host. The endpoints
 * are configured and the DMA pipe is setup in this function. */
void CyFxUsbHidApplnStart(void) {
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
	CyU3PDmaChannelConfig_t dmaCfg;
	uint16_t size = 0x40;
	CyU3PEpConfig_t epCfg;

	CyU3PMemSet((uint8_t *) &epCfg, 0, sizeof(epCfg));
	epCfg.enable = CyTrue;
	epCfg.epType = CY_U3P_USB_EP_INTR;
	epCfg.burstLen = 1;
	epCfg.streams = 0;
	epCfg.pcktSize = size;

	apiRetStatus = CyU3PSetEpConfig(CY_FX_HID_EP_INTR_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PSetEpConfig failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	CyU3PMemSet((uint8_t *) &epCfg, 0, sizeof(epCfg));
	epCfg.enable = CyTrue;
	epCfg.epType = CY_U3P_USB_EP_INTR;
	epCfg.burstLen = 1;
	epCfg.streams = 0;
	epCfg.pcktSize = size;

	apiRetStatus = CyU3PSetEpConfig(CY_FX_HID_EP_INTR_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PSetEpConfig failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	//CyFxHidApplnGpioInit ();

	dmaCfg.size = size;
	dmaCfg.count = 4;
	dmaCfg.prodSckId = CY_U3P_CPU_SOCKET_PROD;
	dmaCfg.consSckId = (CyU3PDmaSocketId_t) CY_U3P_UIB_SOCKET_CONS_1;
	dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;
	dmaCfg.notification = 0;
	dmaCfg.cb = NULL;
	dmaCfg.prodHeader = 0;
	dmaCfg.prodFooter = 0;
	dmaCfg.consHeader = 0;
	dmaCfg.prodAvailCount = 0;

	apiRetStatus = CyU3PDmaChannelCreate(&glChHandleIntrCPU2U,
			CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PDmaChannelCreate failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	dmaCfg.size = size;
	dmaCfg.count = 10;
	dmaCfg.prodSckId = CY_U3P_UIB_SOCKET_PROD_1;
	dmaCfg.consSckId = (CyU3PDmaSocketId_t) CY_U3P_CPU_SOCKET_CONS;
	dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;
	dmaCfg.notification = 0;
	dmaCfg.cb = NULL;
	dmaCfg.prodHeader = 0;
	dmaCfg.prodFooter = 0;
	dmaCfg.consHeader = 0;
	dmaCfg.prodAvailCount = 0;

	apiRetStatus = CyU3PDmaChannelCreate(&glChHandleIntrU2CPU,
			CY_U3P_DMA_TYPE_MANUAL_IN, &dmaCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PDmaChannelCreate failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Set Channel for Infinite Transfer */
	CyU3PDmaChannelSetXfer(&glChHandleIntrCPU2U, 0);
	CyU3PDmaChannelSetXfer(&glChHandleIntrU2CPU, 0);

	/* Flush the Endpoint memory */
	CyU3PUsbFlushEp(CY_FX_HID_EP_INTR_IN);
	CyU3PUsbFlushEp(CY_FX_HID_EP_INTR_OUT);
	/* Update the status flag. */
	glIsApplnActive = CyTrue;

	CyU3PTimerCreate(&ButtonChange,ButtonIndexChange,0x00,ButtonChangePeriod,ButtonChangePeriod,CYU3P_NO_ACTIVATE);

}

/* This function stops the HID application. This shall be called whenever
 * a RESET or DISCONNECT event is received from the USB host. The endpoints are
 * disabled and the DMA pipe is destroyed by this function. */
void CyFxUsbHidApplnStop(void) {
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
	CyU3PEpConfig_t epCfg;

	/* Update the flag. */
	glIsApplnActive = CyFalse;

	/* Flush the endpoint memory */
	CyU3PUsbFlushEp(CY_FX_HID_EP_INTR_IN);
	CyU3PUsbFlushEp(CY_FX_HID_EP_INTR_OUT);
	/* Destroy the channel */
	CyU3PDmaChannelDestroy(&glChHandleIntrCPU2U);
	CyU3PDmaChannelDestroy(&glChHandleIntrU2CPU);

	/* Disable endpoints. */
	CyU3PMemSet((uint8_t *) &epCfg, 0, sizeof(epCfg));
	epCfg.enable = CyFalse;

	/* Consumer Intr endpoint configuration. */
	apiRetStatus = CyU3PSetEpConfig(CY_FX_HID_EP_INTR_IN, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PSetEpConfig failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}
	apiRetStatus = CyU3PSetEpConfig(CY_FX_HID_EP_INTR_OUT, &epCfg);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PSetEpConfig failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}
	/* De-Activate GPIO Block */
	apiRetStatus = CyU3PGpioDeInit();
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "GPIO De-Init failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}
}

/* Send Reports containing pre-defined patterns to Host through interrupt EP */
CyU3PReturnStatus_t CyFxHidSendReport(void) {
	timestamp+=0x02;
    CyU3PDmaBuffer_t DmaBuffer;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    status = CyU3PDmaChannelGetBuffer (&glChHandleIntrU2CPU, &DmaBuffer, 10);
	if (status != CY_U3P_SUCCESS) {
			CyU3PDmaChannelReset(&glChHandleIntrCPU2U);
			CyU3PDmaChannelSetXfer(&glChHandleIntrCPU2U, 0);
	}
	else
    {
    	if(DmaBuffer.buffer[0]==0x80)
    	{
    		committime=0;
    	}
    	if(DmaBuffer.buffer[0]==0x01)
    	{
    		committime=2;
    	}
    	status = CyU3PDmaChannelDiscardBuffer (&glChHandleIntrU2CPU);
    }

	//CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
	CyU3PDmaBuffer_t outBuf;
	//uint16_t i = 0;

	outBuf.buffer = 0;
	outBuf.status = 0;
	outBuf.size = 64;
	outBuf.count = 64;

	CyU3PDebugPrint(4, "Input Report \r\n");

	/* Retrieve Free Buffer */
	status = CyU3PDmaChannelGetBuffer(&glChHandleIntrCPU2U, &outBuf, 500);
	if (status != CY_U3P_SUCCESS) {
			CyU3PDmaChannelReset(&glChHandleIntrCPU2U);
			CyU3PDmaChannelSetXfer(&glChHandleIntrCPU2U, 0);
	} else {
		if (committime == 0) {
			CyU3PMemSet((uint8_t *) outBuf.buffer, 0, outBuf.size);
			outBuf.buffer[0] = 0x81;
			outBuf.buffer[1] = timestamp;
			outBuf.buffer[3] = 0x03;
			outBuf.buffer[4] = 0xEC;
			outBuf.buffer[5] = 0x6B;
			outBuf.buffer[6] = 0x0E;
			outBuf.buffer[7] = 0xCD;
			outBuf.buffer[8] = 0xA0;
			outBuf.buffer[9] = 0xE8;
			committime++;
		} else if (committime == 1) {
			CyU3PMemSet((uint8_t *) outBuf.buffer, 0, outBuf.size);
			outBuf.buffer[0] = 0x81;
			outBuf.buffer[1] = timestamp;
			committime+=2;
		} else if (committime == 2) {

			CyU3PMemSet((uint8_t *) outBuf.buffer, 0, outBuf.size);
			outBuf.buffer[0] = 0x21;
			outBuf.buffer[1] = timestamp;
			outBuf.buffer[2] = 0x91;
			outBuf.buffer[3] = 0x00;
			outBuf.buffer[4] = 0x80;
			outBuf.buffer[5] = 0x00;
			outBuf.buffer[6] = 0x49;
			outBuf.buffer[7] = 0x98;
			outBuf.buffer[8] = 0x86;
			outBuf.buffer[9] = 0x06;
			outBuf.buffer[10] = 0x38;
			outBuf.buffer[11] = 0x77;
			outBuf.buffer[12] = 0x0C;
			outBuf.buffer[13] = 0x80;
			outBuf.buffer[14] = 0x03;
			outBuf.buffer[15] = 0x00;
			CyU3PBusyWait(8000);

			committime++;
		} else {

			CyU3PTimerStart(&ButtonChange);
			for (int i = 0; i < 64; i++) {
				outBuf.buffer[i] = ReportDataDefault[i];
			}

			outBuf.buffer[1] = timestamp;

			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].DPAD_DOWN);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].DPAD_UP<<1);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].DPAD_RIGHT<<2);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].DPAD_LEFT<<3);

			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].SR<<4);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].SL<<5);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].SHOULDER_1<<6);
			outBuf.buffer[5]|=(uint8_t)(ButtonMap[ButtonIndex].SHOULDER_2<<7);

			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].HOME<<4);
			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].CAPTURE<<5);
			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].MINUS);
			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].PLUS<<1);
// Don't need these two sticks
//			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].STICK<<6);
//			outBuf.buffer[4]|=(uint8_t)(ButtonMap[ButtonIndex].STICK2<<7);

			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].B);
			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].A<<2);
			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].X<<1);
			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].Y<<3);
			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].SHOULDER2_1<<6);
			outBuf.buffer[3]|=(uint8_t)(ButtonMap[ButtonIndex].SHOULDER2_2<<7);

			CyU3PDebugPrint(4, "test %x %x %x \r\n", outBuf.buffer[3],outBuf.buffer[4],outBuf.buffer[5]);

		}

		/* Copy Report Data into the output buffer */
		CyU3PBusyWait(8000);
		status = CyU3PDmaChannelCommitBuffer(&glChHandleIntrCPU2U, 64, 0);
		if (status != CY_U3P_SUCCESS) {
			CyU3PDmaChannelReset(&glChHandleIntrCPU2U);
			CyU3PDmaChannelSetXfer(&glChHandleIntrCPU2U, 0);
		}
	}
	/* Wait for 2 msec after sending each packet */
	//CyU3PDebugPrint(4, "Packet %d Status %d\r\n", i, status);


	return status;
}

/* Callback to handle the USB setup requests. */
CyBool_t CyFxUsbHidApplnUSBSetupCB(uint32_t setupdat0, /* SETUP Data 0 */
uint32_t setupdat1 /* SETUP Data 1 */
) {
	/* Fast enumeration is used. Only requests addressed to the interface, class,
	 * vendor and unknown control requests are received by this function.
	 * This application does not support any class or vendor requests. */

	uint8_t bRequest, bReqType;
	uint8_t bType, bTarget;
	uint16_t wValue, wIndex;
	CyBool_t isHandled = CyFalse;
	CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

	/* Decode the fields from the setup request. */
	bReqType = (setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK);
	bType = (bReqType & CY_U3P_USB_TYPE_MASK);
	bTarget = (bReqType & CY_U3P_USB_TARGET_MASK);
	bRequest =
			((setupdat0 & CY_U3P_USB_REQUEST_MASK) >> CY_U3P_USB_REQUEST_POS);
	wValue = ((setupdat0 & CY_U3P_USB_VALUE_MASK) >> CY_U3P_USB_VALUE_POS);
	wIndex = ((setupdat1 & CY_U3P_USB_INDEX_MASK) >> CY_U3P_USB_INDEX_POS);

	if (bType == CY_U3P_USB_STANDARD_RQT) {
		/* Handle SET_FEATURE(FUNCTION_SUSPEND) and CLEAR_FEATURE(FUNCTION_SUSPEND)
		 * requests here. It should be allowed to pass if the device is in configured
		 * state and failed otherwise. */
		if ((bTarget == CY_U3P_USB_TARGET_INTF)
				&& ((bRequest == CY_U3P_USB_SC_SET_FEATURE)
						|| (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE))
				&& (wValue == 0)) {
			if (glIsApplnActive)
				CyU3PUsbAckSetup();
			else
				CyU3PUsbStall(0, CyTrue, CyFalse);

			isHandled = CyTrue;
		}

		if ((bTarget == CY_U3P_USB_TARGET_ENDPT)
				&& (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)
				&& (wValue == CY_U3P_USBX_FS_EP_HALT)) {
			if (wIndex == CY_FX_HID_EP_INTR_IN) {
				if (glIsApplnActive) {
					CyU3PUsbSetEpNak(CY_FX_HID_EP_INTR_IN, CyTrue);
					CyU3PBusyWait(125);

					CyU3PDmaChannelReset(&glChHandleIntrCPU2U);
					CyU3PUsbFlushEp(CY_FX_HID_EP_INTR_IN);
					CyU3PUsbResetEp(CY_FX_HID_EP_INTR_IN);
					CyU3PUsbStall(wIndex, CyFalse, CyTrue);

					CyU3PUsbSetEpNak(CY_FX_HID_EP_INTR_IN, CyFalse);

					CyU3PUsbAckSetup();
					isHandled = CyTrue;
				}
			}
		}

		/* Class specific descriptors such as HID Report descriptor need to handled by the callback. */
		bReqType = ((setupdat0 & CY_U3P_USB_VALUE_MASK) >> 24);
		if ((bRequest == CY_U3P_USB_SC_GET_DESCRIPTOR)
				&& (bReqType == CY_FX_GET_REPORT_DESC)) {
			isHandled = CyTrue;

			status = CyU3PUsbSendEP0Data(0xCB, (uint8_t *) CyFxUSBReportDscr);
			if (status != CY_U3P_SUCCESS) {
				/* There was some error. We should try stalling EP0. */
				CyU3PUsbStall(0, CyTrue, CyFalse);
			}
		}
	} else if (bType == CY_U3P_USB_CLASS_RQT) {
		/* Class Specific Request Handler */
		if (bRequest == CY_FX_HID_SET_IDLE) {
			CyU3PUsbAckSetup();
			glButtonPress = 0;
			isHandled = CyTrue;
		}
	}

	return isHandled;
}

/* This is the callback function to handle the USB events. */
void CyFxUsbHidApplnUSBEventCB(CyU3PUsbEventType_t evtype, /* Event type */
uint16_t evdata /* Event data */
) {
	switch (evtype) {
	case CY_U3P_USB_EVENT_SETCONF:
		/* Disable the low power entry to optimize USB throughput */
		CyU3PUsbLPMDisable();

		/* Stop application before re-starting. */
		if (glIsApplnActive) {
			CyFxUsbHidApplnStop();
		}
		/* Start application. */
		CyFxUsbHidApplnStart();
		break;

	case CY_U3P_USB_EVENT_RESET:
	case CY_U3P_USB_EVENT_DISCONNECT:
		/* Stop application. */
		if (glIsApplnActive) {
			CyFxUsbHidApplnStop();
		}
		break;

	default:
		break;
	}
}

/* Callback function to handle LPM requests from the USB 3.0 host. This function is invoked by the API
 whenever a state change from U0 -> U1 or U0 -> U2 happens. If we return CyTrue from this function, the
 FX3 device is retained in the low power state. If we return CyFalse, the FX3 device immediately tries
 to trigger an exit back to U0.

 This application does not have any state in which we should not allow U1/U2 transitions; and therefore
 the function always return CyTrue.
 */
CyBool_t CyFxUsbHidApplnLPMRqtCB(CyU3PUsbLinkPowerMode link_mode) {
	return CyTrue;
}

/* This function initializes the USB Module, sets the enumeration descriptors.
 * This function does not start the bulk streaming and this is done only when
 * SET_CONF event is received. */
void CyFxUsbHidApplnInit(void) {
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	/* Start the USB functionality. */
	apiRetStatus = CyU3PUsbStart();
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "CyU3PUsbStart failed to Start, Error code = %d\r\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Register callbacks for handling USB events and requests. */
	CyU3PUsbRegisterSetupCallback(CyFxUsbHidApplnUSBSetupCB, CyTrue);

	CyU3PUsbRegisterLPMRequestCallback(CyFxUsbHidApplnLPMRqtCB);

	CyU3PUsbRegisterEventCallback(CyFxUsbHidApplnUSBEventCB);

	/* Register the USB descriptors with the driver. */

	/* Super speed device descriptor. */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_DEVICE_DESCR, 0,
			(uint8_t *) CyFxUSB30DeviceDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set device descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* High speed device descriptor. */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_DEVICE_DESCR, 0,
			(uint8_t *) CyFxUSB20DeviceDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set device descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* BOS descriptor */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_BOS_DESCR, 0,
			(uint8_t *) CyFxUSBBOSDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set configuration descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Device qualifier descriptor */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_DEVQUAL_DESCR, 0,
			(uint8_t *) CyFxUSBDeviceQualDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set device qualifier descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Super speed configuration descriptor */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_CONFIG_DESCR, 0,
			(uint8_t *) CyFxUSBSSConfigDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set configuration descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* High speed configuration descriptor */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_CONFIG_DESCR, 0,
			(uint8_t *) CyFxUSBHSConfigDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB Set Other Speed Descriptor failed, Error Code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Full speed configuration descriptor */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_FS_CONFIG_DESCR, 0,
			(uint8_t *) CyFxUSBFSConfigDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB Set Configuration Descriptor failed, Error Code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* String descriptor 0 */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 0,
			(uint8_t *) CyFxUSBStringLangIDDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set string descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* String descriptor 1 */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 1,
			(uint8_t *) CyFxUSBManufactureDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set string descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* String descriptor 2 */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 2,
			(uint8_t *) CyFxUSBProductDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set string descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* String descriptor 3 */
	apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 3,
			(uint8_t *) CyFxUSBSerialNumberDscr);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4,
				"USB set string descriptor failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	/* Connect the USB Pins with super speed operation enabled. */
	apiRetStatus = CyU3PConnectState(CyTrue, CyFalse);
	if (apiRetStatus != CY_U3P_SUCCESS) {
		CyU3PDebugPrint(4, "USB Connect failed, Error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}
}

/* Entry function for the UsbHidAppThread. */
void Fx3HidAppThread_Entry(uint32_t input) {
	//CyU3PReturnStatus_t status;

	/* Initialize the debug module */
	CyFxUsbHidApplnDebugInit();

	/* Initialize the HID application */
	CyFxUsbHidApplnInit();

	CyU3PDebugPrint(4, "App Init\r\n");

	for (;;) {

		CyFxHidSendReport();

	}
}

/* Application define function which creates the threads. */
void CyFxApplicationDefine(void) {
	void *ptr = NULL;
	uint32_t retThrdCreate = CY_U3P_SUCCESS;

	/* Allocate the memory for the threads */
	ptr = CyU3PMemAlloc(CY_FX_HID_THREAD_STACK);

	/* Create the thread for the application */
	retThrdCreate = CyU3PThreadCreate(&UsbHidAppThread, /* App Thread structure */
	"21:FX3_HID_MOUSE", /* Thread ID and Thread name */
	Fx3HidAppThread_Entry, /* App Thread Entry function */
	0, /* No input parameter to thread */
	ptr, /* Pointer to the allocated thread stack */
	CY_FX_HID_THREAD_STACK, /* App Thread stack size */
	CY_FX_HID_THREAD_PRIORITY, /* App Thread priority */
	CY_FX_HID_THREAD_PRIORITY, /* App Thread priority */
	CYU3P_NO_TIME_SLICE, /* No time slice for the application thread */
	CYU3P_AUTO_START /* Start the Thread immediately */
	);

	/* Check the return code */
	if (retThrdCreate != 0) {
		/* Thread Creation failed with the error code retThrdCreate */

		/* Add custom recovery or debug actions here */

		/* Application cannot continue */
		/* Loop indefinitely */
		while (1)
			;
	}
}

/*
 * Main function
 */
int main(void) {
	CyU3PIoMatrixConfig_t io_cfg;
	CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

	/* Initialize the device */
	status = CyU3PDeviceInit(0);
	if (status != CY_U3P_SUCCESS) {
		goto handle_fatal_error;
	}

	/* Initialize the caches. Enable both Instruction and Data Caches. */
	status = CyU3PDeviceCacheControl(CyTrue, CyFalse, CyFalse);
	if (status != CY_U3P_SUCCESS) {
		goto handle_fatal_error;
	}

	/* Configure the IO matrix for the device. On the FX3 DVK board, the COM port
	 * is connected to the IO(53:56). This means that either DQ32 mode should be
	 * selected or lppMode should be set to UART_ONLY. Here we are choosing
	 * UART_ONLY configuration. */
	io_cfg.isDQ32Bit = CyFalse;
	io_cfg.s0Mode = CY_U3P_SPORT_INACTIVE;
	io_cfg.s1Mode = CY_U3P_SPORT_INACTIVE;
	io_cfg.useUart = CyTrue;
	io_cfg.useI2C = CyFalse;
	io_cfg.useI2S = CyFalse;
	io_cfg.useSpi = CyFalse;
	io_cfg.lppMode = CY_U3P_IO_MATRIX_LPP_UART_ONLY;

	/* No GPIOs are enabled. */
	io_cfg.gpioSimpleEn[0] = 0;
	io_cfg.gpioSimpleEn[1] = 0;
	io_cfg.gpioComplexEn[0] = 0;
	io_cfg.gpioComplexEn[1] = 0;

	status = CyU3PDeviceConfigureIOMatrix(&io_cfg);
	if (status != CY_U3P_SUCCESS) {
		goto handle_fatal_error;
	}

	/* This is a non returnable call for initializing the RTOS kernel */
	CyU3PKernelEntry();

	/* Dummy return to make the compiler happy */
	return 0;

	handle_fatal_error:

	/* Cannot recover from this error. */
	while (1)
		;
}

/* [] */

