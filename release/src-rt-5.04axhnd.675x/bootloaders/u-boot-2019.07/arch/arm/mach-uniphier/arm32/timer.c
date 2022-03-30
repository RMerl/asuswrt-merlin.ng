// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/io.h>

#include "arm-mpcore.h"

#define PERIPHCLK (50 * 1000 * 1000) /* 50 MHz */
#define PRESCALER ((PERIPHCLK) / (CONFIG_SYS_TIMER_RATE) - 1)

static void *get_global_timer_base(void)
{
	void *val;

	asm("mrc p15, 4, %0, c15, c0, 0" : "=r" (val) : : "memory");

	return val + GLOBAL_TIMER_OFFSET;
}

unsigned long timer_read_counter(void)
{
	/*
	 * ARM 64bit Global Timer is too much for our purpose.
	 * We use only lower 32 bit of the timer counter.
	 */
	return readl(get_global_timer_base() + GTIMER_CNT_L);
}

int timer_init(void)
{
	/* enable timer */
	writel(PRESCALER << 8 | 1, get_global_timer_base() + GTIMER_CTRL);

	return 0;
}
