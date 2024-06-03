// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/timer.h>
#include <asm/io.h>
#include <linux/types.h>

struct rk_timer * const timer_ptr = (void *)CONFIG_SYS_TIMER_BASE;

static uint64_t rockchip_get_ticks(void)
{
	uint64_t timebase_h, timebase_l;

	timebase_l = readl(&timer_ptr->timer_curr_value0);
	timebase_h = readl(&timer_ptr->timer_curr_value1);

	return timebase_h << 32 | timebase_l;
}

void rockchip_udelay(unsigned int usec)
{
	uint64_t tmp;

	/* get timestamp */
	tmp = rockchip_get_ticks() + usec_to_tick(usec);

	/* loop till event */
	while (rockchip_get_ticks() < tmp+1)
		;
}

void rockchip_timer_init(void)
{
	writel(0xffffffff, &timer_ptr->timer_load_count0);
	writel(0xffffffff, &timer_ptr->timer_load_count1);
	writel(1, &timer_ptr->timer_ctrl_reg);
}
