/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Mentor USB OTG Core host controller driver.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#ifndef __MUSB_HCD_H__
#define __MUSB_HCD_H__

#include "musb_core.h"
#ifdef CONFIG_USB_KEYBOARD
#include <stdio_dev.h>
extern unsigned char new[];
#endif

#ifndef CONFIG_USB_MUSB_TIMEOUT
# define CONFIG_USB_MUSB_TIMEOUT 100000
#endif

/* This defines the endpoint number used for control transfers */
#define MUSB_CONTROL_EP 0

/* This defines the endpoint number used for bulk transfer */
#ifndef MUSB_BULK_EP
# define MUSB_BULK_EP 1
#endif

/* This defines the endpoint number used for interrupt transfer */
#define MUSB_INTR_EP 2

/* Determine the operating speed of MUSB core */
#define musb_ishighspeed() \
	((readb(&musbr->power) & MUSB_POWER_HSMODE) \
		>> MUSB_POWER_HSMODE_SHIFT)

/* USB HUB CONSTANTS (not OHCI-specific; see hub.h) */

/* destination of request */
#define RH_INTERFACE		   0x01
#define RH_ENDPOINT		   0x02
#define RH_OTHER		   0x03

#define RH_CLASS		   0x20
#define RH_VENDOR		   0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS		0x0080
#define RH_CLEAR_FEATURE	0x0100
#define RH_SET_FEATURE		0x0300
#define RH_SET_ADDRESS		0x0500
#define RH_GET_DESCRIPTOR	0x0680
#define RH_SET_DESCRIPTOR	0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE		0x0280
#define RH_GET_INTERFACE	0x0A80
#define RH_SET_INTERFACE	0x0B00
#define RH_SYNC_FRAME		0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP		0x2000

/* Hub port features */
#define RH_PORT_CONNECTION	   0x00
#define RH_PORT_ENABLE		   0x01
#define RH_PORT_SUSPEND		   0x02
#define RH_PORT_OVER_CURRENT	   0x03
#define RH_PORT_RESET		   0x04
#define RH_PORT_POWER		   0x08
#define RH_PORT_LOW_SPEED	   0x09

#define RH_C_PORT_CONNECTION	   0x10
#define RH_C_PORT_ENABLE	   0x11
#define RH_C_PORT_SUSPEND	   0x12
#define RH_C_PORT_OVER_CURRENT	   0x13
#define RH_C_PORT_RESET		   0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER	   0x00
#define RH_C_HUB_OVER_CURRENT	   0x01

#define RH_DEVICE_REMOTE_WAKEUP	   0x00
#define RH_ENDPOINT_STALL	   0x01

#define RH_ACK			   0x01
#define RH_REQ_ERR		   -1
#define RH_NACK			   0x00

/* extern functions */
extern int musb_platform_init(void);
extern void musb_platform_deinit(void);

#endif	/* __MUSB_HCD_H__ */
