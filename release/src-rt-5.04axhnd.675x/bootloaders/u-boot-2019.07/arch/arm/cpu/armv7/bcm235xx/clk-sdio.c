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

/* Enable appropriate clocks for an SDIO port */
int clk_sdio_enable(void *base, u32 rate, u32 *actual_ratep)
{
	int ret;
	struct clk *c;

	char *clkstr;
	char *slpstr;
	char *ahbstr;

	switch ((u32) base) {
	case CONFIG_SYS_SDIO_BASE0:
		clkstr = CONFIG_SYS_SDIO0 "_clk";
		ahbstr = CONFIG_SYS_SDIO0 "_ahb_clk";
		slpstr = CONFIG_SYS_SDIO0 "_sleep_clk";
		break;
	case CONFIG_SYS_SDIO_BASE1:
		clkstr = CONFIG_SYS_SDIO1 "_clk";
		ahbstr = CONFIG_SYS_SDIO1 "_ahb_clk";
		slpstr = CONFIG_SYS_SDIO1 "_sleep_clk";
		break;
	case CONFIG_SYS_SDIO_BASE2:
		clkstr = CONFIG_SYS_SDIO2 "_clk";
		ahbstr = CONFIG_SYS_SDIO2 "_ahb_clk";
		slpstr = CONFIG_SYS_SDIO2 "_sleep_clk";
		break;
	case CONFIG_SYS_SDIO_BASE3:
		clkstr = CONFIG_SYS_SDIO3 "_clk";
		ahbstr = CONFIG_SYS_SDIO3 "_ahb_clk";
		slpstr = CONFIG_SYS_SDIO3 "_sleep_clk";
		break;
	default:
		printf("%s: base 0x%p not found\n", __func__, base);
		return -EINVAL;
	}

	ret = clk_get_and_enable(ahbstr);
	if (ret)
		return ret;

	ret = clk_get_and_enable(slpstr);
	if (ret)
		return ret;

	c = clk_get(clkstr);
	if (c) {
		ret = clk_set_rate(c, rate);
		if (ret)
			return ret;

		ret = clk_enable(c);
		if (ret)
			return ret;
	} else {
		printf("%s: Couldn't find %s\n", __func__, clkstr);
		return -EINVAL;
	}
	*actual_ratep = rate;
	return 0;
}
