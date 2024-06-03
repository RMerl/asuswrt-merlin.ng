// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>

#define TIMER_LOAD_VAL		0xFFFFFFFF

static const struct socfpga_timer *timer_base = (void *)CONFIG_SYS_TIMERBASE;

/*
 * Timer initialization
 */
int timer_init(void)
{
	writel(TIMER_LOAD_VAL, &timer_base->load_val);
	writel(TIMER_LOAD_VAL, &timer_base->curr_val);
	writel(readl(&timer_base->ctrl) | 0x3, &timer_base->ctrl);
	return 0;
}
