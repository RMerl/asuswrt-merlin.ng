/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 */

struct watchdog_regs {
	u16	wcr;	/* Control */
	u16	wsr;	/* Service */
	u16	wrsr;	/* Reset Status */
};

#define WCR_WDZST	0x01
#define WCR_WDBG	0x02
#define WCR_WDE		0x04
#define WCR_WDT		0x08
#define WCR_SRS		0x10
#define WCR_WDA 	0x20
#define SET_WCR_WT(x)	(x << 8)
#define WCR_WT_MSK	SET_WCR_WT(0xFF)
