// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>
#include "clk-core.h"

/* Enable appropriate clocks for a BSC/I2C port */
int clk_bsc_enable(void *base)
{
	int ret;
	char *bscstr, *apbstr;

	switch ((u32) base) {
	case PMU_BSC_BASE_ADDR:
		/* PMU clock is always enabled */
		return 0;
	case BSC1_BASE_ADDR:
		bscstr = "bsc1_clk";
		apbstr = "bsc1_apb_clk";
		break;
	case BSC2_BASE_ADDR:
		bscstr = "bsc2_clk";
		apbstr = "bsc2_apb_clk";
		break;
	case BSC3_BASE_ADDR:
		bscstr = "bsc3_clk";
		apbstr = "bsc3_apb_clk";
		break;
	default:
		printf("%s: base 0x%p not found\n", __func__, base);
		return -EINVAL;
	}

	/* Note that the bus clock must be enabled first */

	ret = clk_get_and_enable(apbstr);
	if (ret)
		return ret;

	ret = clk_get_and_enable(bscstr);
	if (ret)
		return ret;

	return 0;
}
