/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * omap_wdt.h
 *
 * OMAP Watchdog header file
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __OMAP_WDT_H__
#define __OMAP_WDT_H__

/*
 * Watchdog:
 * Using the prescaler, the OMAP watchdog could go for many
 * months before firing.  These limits work without scaling,
 * with the 60 second default assumed by most tools and docs.
 */
#define TIMER_MARGIN_MAX	(24 * 60 * 60)	/* 1 day */
#define TIMER_MARGIN_DEFAULT	60	/* 60 secs */
#define TIMER_MARGIN_MIN	1

#define PTV			0	/* prescale */
#define GET_WLDR_VAL(secs)	(0xffffffff - ((secs) * (32768/(1<<PTV))) + 1)
#define WDT_WWPS_PEND_WCLR	BIT(0)
#define WDT_WWPS_PEND_WLDR	BIT(2)
#define WDT_WWPS_PEND_WTGR	BIT(3)
#define WDT_WWPS_PEND_WSPR	BIT(4)

#define WDT_WCLR_PRE		BIT(5)
#define WDT_WCLR_PTV_OFF	2

/* Watchdog timer registers */
struct wd_timer {
	unsigned int resv1[4];
	unsigned int wdtwdsc;	/* offset 0x010 */
	unsigned int wdtwdst;	/* offset 0x014 */
	unsigned int wdtwisr;	/* offset 0x018 */
	unsigned int wdtwier;	/* offset 0x01C */
	unsigned int wdtwwer;	/* offset 0x020 */
	unsigned int wdtwclr;	/* offset 0x024 */
	unsigned int wdtwcrr;	/* offset 0x028 */
	unsigned int wdtwldr;	/* offset 0x02C */
	unsigned int wdtwtgr;	/* offset 0x030 */
	unsigned int wdtwwps;	/* offset 0x034 */
	unsigned int resv2[3];
	unsigned int wdtwdly;	/* offset 0x044 */
	unsigned int wdtwspr;	/* offset 0x048 */
	unsigned int resv3[1];
	unsigned int wdtwqeoi;	/* offset 0x050 */
	unsigned int wdtwqstar;	/* offset 0x054 */
	unsigned int wdtwqsta;	/* offset 0x058 */
	unsigned int wdtwqens;	/* offset 0x05C */
	unsigned int wdtwqenc;	/* offset 0x060 */
	unsigned int resv4[39];
	unsigned int wdt_unfr;	/* offset 0x100 */
};

#endif /* __OMAP_WDT_H__ */
