/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Chen-Yu Tsai <wens@csie.org>
 *
 * Watchdog register definitions
 */

#ifndef _SUNXI_WATCHDOG_H_
#define _SUNXI_WATCHDOG_H_

#define WDT_CTRL_RESTART	(0x1 << 0)
#define WDT_CTRL_KEY		(0x0a57 << 1)

#if defined(CONFIG_MACH_SUN4I) || \
    defined(CONFIG_MACH_SUN5I) || \
    defined(CONFIG_MACH_SUN7I) || \
    defined(CONFIG_MACH_SUN8I_R40)

#define WDT_MODE_EN		(0x1 << 0)
#define WDT_MODE_RESET_EN	(0x1 << 1)

struct sunxi_wdog {
	u32 ctl;		/* 0x00 */
	u32 mode;		/* 0x04 */
	u32 res[2];
};

#else

#define WDT_CFG_RESET		(0x1)
#define WDT_MODE_EN		(0x1)

struct sunxi_wdog {
	u32 irq_en;		/* 0x00 */
	u32 irq_sta;		/* 0x04 */
	u32 res1[2];
	u32 ctl;		/* 0x10 */
	u32 cfg;		/* 0x14 */
	u32 mode;		/* 0x18 */
	u32 res2;
};

#endif

#endif /* _SUNXI_WATCHDOG_H_ */
