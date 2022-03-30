/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_WDOG_H
#define _BCM2835_WDOG_H

#ifndef CONFIG_BCM2835
#define BCM2835_WDOG_PHYSADDR			0x3f100000
#else
#define BCM2835_WDOG_PHYSADDR			0x20100000
#endif

struct bcm2835_wdog_regs {
	u32 unknown0[7];
	u32 rstc;
	u32 rsts;
	u32 wdog;
};

#define BCM2835_WDOG_PASSWORD			0x5a000000

#define BCM2835_WDOG_RSTC_WRCFG_MASK		0x00000030
#define BCM2835_WDOG_RSTC_WRCFG_FULL_RESET	0x00000020

#define BCM2835_WDOG_WDOG_TIMEOUT_MASK		0x0000ffff

#endif
