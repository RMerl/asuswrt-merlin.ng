// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell PXA2xx/3xx timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/io.h>

int timer_init(void)
{
	writel(0, CONFIG_SYS_TIMER_COUNTER);
	return 0;
}
