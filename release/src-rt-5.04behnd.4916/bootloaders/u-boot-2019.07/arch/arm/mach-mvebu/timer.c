// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>

#define TIMER_LOAD_VAL			0xffffffff

static int init_done __attribute__((section(".data"))) = 0;

/*
 * Timer initialization
 */
int timer_init(void)
{
	/* Only init the timer once */
	if (init_done)
		return 0;
	init_done = 1;

	/* load value into timer */
	writel(TIMER_LOAD_VAL, MVEBU_TIMER_BASE + 0x10);
	writel(TIMER_LOAD_VAL, MVEBU_TIMER_BASE + 0x14);

#if defined(CONFIG_ARCH_MVEBU)
	/* On Armada XP / 38x ..., the 25MHz clock source needs to be enabled */
	setbits_le32(MVEBU_TIMER_BASE + 0x00, BIT(11));
#endif
	/* enable timer in auto reload mode */
	setbits_le32(MVEBU_TIMER_BASE + 0x00, 0x3);

	return 0;
}
