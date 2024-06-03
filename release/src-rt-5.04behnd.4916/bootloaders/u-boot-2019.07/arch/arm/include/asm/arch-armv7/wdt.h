/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Matt Waddel, <matt.waddel@linaro.org>
 */
#ifndef _WDT_H_
#define _WDT_H_

/* Watchdog timer (SP805) register base address */
#define WDT_BASE	0x100E5000

#define WDT_EN		0x2
#define WDT_RESET_LOAD	0x0

struct wdt {
	u32 wdogload;		/* 0x000 */
	u32 wdogvalue;
	u32 wdogcontrol;
	u32 wdogintclr;
	u32 wdogris;
	u32 wdogmis;
	u32 res1[0x2F9];
	u32 wdoglock;		/* 0xC00 */
	u32 res2[0xBE];
	u32 wdogitcr;		/* 0xF00 */
	u32 wdogitop;
	u32 res3[0x35];
	u32 wdogperiphid0;	/* 0xFE0 */
	u32 wdogperiphid1;
	u32 wdogperiphid2;
	u32 wdogperiphid3;
	u32 wdogpcellid0;
	u32 wdogpcellid1;
	u32 wdogpcellid2;
	u32 wdogpcellid3;
};

#endif /* _WDT_H_ */
