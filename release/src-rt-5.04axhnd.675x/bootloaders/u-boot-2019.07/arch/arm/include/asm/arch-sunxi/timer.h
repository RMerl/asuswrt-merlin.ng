/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner A10-evb board.
 */

#ifndef _SUNXI_TIMER_H_
#define _SUNXI_TIMER_H_

#ifndef __ASSEMBLY__

#include <linux/types.h>
#include <asm/arch/watchdog.h>

/* General purpose timer */
struct sunxi_timer {
	u32 ctl;
	u32 inter;
	u32 val;
	u8 res[4];
};

/* Audio video sync*/
struct sunxi_avs {
	u32 ctl;		/* 0x80 */
	u32 cnt0;		/* 0x84 */
	u32 cnt1;		/* 0x88 */
	u32 div;		/* 0x8c */
};

/* 64 bit counter */
struct sunxi_64cnt {
	u32 ctl;		/* 0xa0 */
	u32 lo;			/* 0xa4 */
	u32 hi;			/* 0xa8 */
};

/* Rtc */
struct sunxi_rtc {
	u32 ctl;		/* 0x100 */
	u32 yymmdd;		/* 0x104 */
	u32 hhmmss;		/* 0x108 */
};

/* Alarm */
struct sunxi_alarm {
	u32 ddhhmmss;		/* 0x10c */
	u32 hhmmss;		/* 0x110 */
	u32 en;			/* 0x114 */
	u32 irqen;		/* 0x118 */
	u32 irqsta;		/* 0x11c */
};

/* Timer general purpose register */
struct sunxi_tgp {
	u32 tgpd;
};

struct sunxi_timer_reg {
	u32 tirqen;		/* 0x00 */
	u32 tirqsta;		/* 0x04 */
	u8 res1[8];
	struct sunxi_timer timer[6];	/* We have 6 timers */
	u8 res2[16];
	struct sunxi_avs avs;
#if defined(CONFIG_SUNXI_GEN_SUN4I) || defined(CONFIG_MACH_SUN8I_R40)
	struct sunxi_wdog wdog;	/* 0x90 */
	/* XXX the following is not accurate for sun5i/sun7i */
	struct sunxi_64cnt cnt64;	/* 0xa0 */
	u8 res4[0x58];
	struct sunxi_rtc rtc;
	struct sunxi_alarm alarm;
	struct sunxi_tgp tgp[4];
	u8 res5[8];
	u32 cpu_cfg;
#elif defined(CONFIG_SUNXI_GEN_SUN6I) || defined(CONFIG_MACH_SUN50I_H6)
	u8 res3[16];
	struct sunxi_wdog wdog[5];	/* We have 5 watchdogs */
#endif
};

#endif /* __ASSEMBLY__ */

#endif
