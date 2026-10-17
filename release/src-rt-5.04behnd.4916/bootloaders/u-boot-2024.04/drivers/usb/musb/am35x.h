/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am35x.h - TI's AM35x platform specific usb wrapper definitions.
 *
 * Author: Ajay Kumar Gupta <ajay.gupta@ti.com>
 *
 * Based on drivers/usb/musb/da8xx.h
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 */

#ifndef __AM35X_USB_H__
#define __AM35X_USB_H__

#include <asm/arch/am35x_def.h>
#include "musb_core.h"

/* Base address of musb wrapper */
#define AM35X_USB_OTG_BASE	0x5C040000

/* Base address of musb core */
#define AM35X_USB_OTG_CORE_BASE	(AM35X_USB_OTG_BASE + 0x400)

/* Timeout for AM35x usb module */
#define AM35X_USB_OTG_TIMEOUT	0x3FFFFFF

/*
 * AM35x platform USB wrapper register overlay.
 */
struct am35x_usb_regs {
	u32	revision;
	u32	control;
	u32	status;
	u32	emulation;
	u32	reserved0[1];
	u32	autoreq;
	u32	srpfixtime;
	u32	ep_intsrc;
	u32	ep_intsrcset;
	u32	ep_intsrcclr;
	u32	ep_intmsk;
	u32	ep_intmskset;
	u32	ep_intmskclr;
	u32	ep_intsrcmsked;
	u32	reserved1[1];
	u32	core_intsrc;
	u32	core_intsrcset;
	u32	core_intsrcclr;
	u32	core_intmsk;
	u32	core_intmskset;
	u32	core_intmskclr;
	u32	core_intsrcmsked;
	u32	reserved2[1];
	u32	eoi;
	u32	mop_sop_en;
	u32	reserved3[2];
	u32	txmode;
	u32	rxmode;
	u32	epcount_mode;
};

#define am35x_usb_regs ((struct am35x_usb_regs *)AM35X_USB_OTG_BASE)

/* USB 2.0 PHY Control */
#define DEVCONF2_PHY_GPIOMODE	(1 << 23)
#define DEVCONF2_OTGMODE	(3 << 14)
#define DEVCONF2_SESENDEN	(1 << 13)       /* Vsess_end comparator */
#define DEVCONF2_VBDTCTEN	(1 << 12)       /* Vbus comparator */
#define DEVCONF2_REFFREQ_24MHZ	(2 << 8)
#define DEVCONF2_REFFREQ_26MHZ	(7 << 8)
#define DEVCONF2_REFFREQ_13MHZ	(6 << 8)
#define DEVCONF2_REFFREQ	(0xf << 8)
#define DEVCONF2_PHYCKGD	(1 << 7)
#define DEVCONF2_VBUSSENSE	(1 << 6)
#define DEVCONF2_PHY_PLLON	(1 << 5)        /* override PLL suspend */
#define DEVCONF2_RESET		(1 << 4)
#define DEVCONF2_PHYPWRDN	(1 << 3)
#define DEVCONF2_OTGPWRDN	(1 << 2)
#define DEVCONF2_DATPOL		(1 << 1)

#endif	/* __AM35X_USB_H__ */
