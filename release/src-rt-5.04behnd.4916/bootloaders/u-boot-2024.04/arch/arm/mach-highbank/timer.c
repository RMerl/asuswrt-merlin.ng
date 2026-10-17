// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * Based on arm926ejs/mx27/timer.c
 */

#include <common.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch-armv7/systimer.h>

#undef SYSTIMER_BASE
#define SYSTIMER_BASE		0xFFF34000	/* Timer 0 and 1 base	*/

static struct systimer *systimer_base = (struct systimer *)SYSTIMER_BASE;

/*
 * Start the timer
 */
int timer_init(void)
{
	/*
	 * Setup timer0
	 */
	writel(0, &systimer_base->timer0control);
	writel(SYSTIMER_RELOAD, &systimer_base->timer0load);
	writel(SYSTIMER_RELOAD, &systimer_base->timer0value);
	writel(SYSTIMER_EN | SYSTIMER_32BIT | SYSTIMER_PRESC_256,
		&systimer_base->timer0control);

	return 0;

}
