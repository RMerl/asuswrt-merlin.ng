// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

/*
 * MX7ULP WDOG Register Map
 */
struct wdog_regs {
	u8 cs1;
	u8 cs2;
	u16 reserve0;
	u32 cnt;
	u32 toval;
	u32 win;
};

#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 0x1500
#endif

#define REFRESH_WORD0 0xA602 /* 1st refresh word */
#define REFRESH_WORD1 0xB480 /* 2nd refresh word */

#define UNLOCK_WORD0 0xC520 /* 1st unlock word */
#define UNLOCK_WORD1 0xD928 /* 2nd unlock word */

#define WDGCS1_WDGE                      (1<<7)
#define WDGCS1_WDGUPDATE                 (1<<5)

#define WDGCS2_FLG                       (1<<6)

#define WDG_BUS_CLK                      (0x0)
#define WDG_LPO_CLK                      (0x1)
#define WDG_32KHZ_CLK                    (0x2)
#define WDG_EXT_CLK                      (0x3)

void hw_watchdog_set_timeout(u16 val)
{
	/* setting timeout value */
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	writel(val, &wdog->toval);
}

void hw_watchdog_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	writel(REFRESH_WORD0, &wdog->cnt);
	writel(REFRESH_WORD1, &wdog->cnt);
}

void hw_watchdog_init(void)
{
	u8 val;
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	writel(UNLOCK_WORD0, &wdog->cnt);
	writel(UNLOCK_WORD1, &wdog->cnt);

	val = readb(&wdog->cs2);
	val |= WDGCS2_FLG;
	writeb(val, &wdog->cs2);

	hw_watchdog_set_timeout(CONFIG_WATCHDOG_TIMEOUT_MSECS);
	writel(0, &wdog->win);

	writeb(WDG_LPO_CLK, &wdog->cs2);/* setting 1-kHz clock source */
	writeb((WDGCS1_WDGE | WDGCS1_WDGUPDATE), &wdog->cs1);/* enable counter running */

	hw_watchdog_reset();
}

void reset_cpu(ulong addr)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG_BASE_ADDR;

	writel(UNLOCK_WORD0, &wdog->cnt);
	writel(UNLOCK_WORD1, &wdog->cnt);

	hw_watchdog_set_timeout(5); /* 5ms timeout */
	writel(0, &wdog->win);

	writeb(WDG_LPO_CLK, &wdog->cs2);/* setting 1-kHz clock source */
	writeb(WDGCS1_WDGE, &wdog->cs1);/* enable counter running */

	hw_watchdog_reset();

	while (1);
}
