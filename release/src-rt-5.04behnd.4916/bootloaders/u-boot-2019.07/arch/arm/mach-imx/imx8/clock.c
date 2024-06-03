// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/arch/clock.h>

DECLARE_GLOBAL_DATA_PTR;

u32 mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	default:
		printf("Unsupported mxc_clock %d\n", clk);
		break;
	}

	return 0;
}
