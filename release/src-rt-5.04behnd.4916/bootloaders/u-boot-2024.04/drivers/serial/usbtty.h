/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * (C) Copyright 2006
 * Bryan O'Donoghue, bodonoghue@codehermit.ie, CodeHermit
 */

#ifndef __USB_TTY_H__
#define __USB_TTY_H__

#include <usbdevice.h>
#if defined(CONFIG_PPC)
#include <usb/mpc8xx_udc.h>
#elif defined(CONFIG_CI_UDC)
#include <usb/ci_udc.h>
#endif

#include <usb/udc.h>
#include <version.h>

#ifndef CFG_USBD_CONFIGURATION_STR
#define CFG_USBD_CONFIGURATION_STR	"TTY via USB"
#endif

#define CFG_USBD_SERIAL_OUT_ENDPOINT UDC_OUT_ENDPOINT
#define CFG_USBD_SERIAL_OUT_PKTSIZE	UDC_OUT_PACKET_SIZE
#define CFG_USBD_SERIAL_IN_ENDPOINT	UDC_IN_ENDPOINT
#define CFG_USBD_SERIAL_IN_PKTSIZE	UDC_IN_PACKET_SIZE
#define CFG_USBD_SERIAL_INT_ENDPOINT UDC_INT_ENDPOINT
#define CFG_USBD_SERIAL_INT_PKTSIZE	UDC_INT_PACKET_SIZE
#define CFG_USBD_SERIAL_BULK_PKTSIZE	UDC_BULK_PACKET_SIZE

#define USBTTY_DEVICE_CLASS	COMMUNICATIONS_DEVICE_CLASS

#define USBTTY_BCD_DEVICE	0x00
#define USBTTY_MAXPOWER		0x00

#define STR_LANG		0x00
#define STR_MANUFACTURER	0x01
#define STR_PRODUCT		0x02
#define STR_SERIAL		0x03
#define STR_CONFIG		0x04
#define STR_DATA_INTERFACE	0x05
#define STR_CTRL_INTERFACE	0x06
#define STR_COUNT		0x07

#endif
