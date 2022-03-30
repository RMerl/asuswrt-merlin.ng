// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/watchdog.h>

#define PRESCALER_VAL 255

void wdt_stop(void)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wtcon = readl(&wdt->wtcon);
	wtcon &= ~(WTCON_EN | WTCON_INT | WTCON_RESET);

	writel(wtcon, &wdt->wtcon);
}

void wdt_start(unsigned int timeout)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wdt_stop();

	wtcon = readl(&wdt->wtcon);
	wtcon |= (WTCON_EN | WTCON_CLK(WTCON_CLK_128));
	wtcon &= ~WTCON_INT;
	wtcon |= WTCON_RESET;
	wtcon |= WTCON_PRESCALER(PRESCALER_VAL);

	writel(timeout, &wdt->wtdat);
	writel(timeout, &wdt->wtcnt);
	writel(wtcon, &wdt->wtcon);
}
