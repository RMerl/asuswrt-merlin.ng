// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 */

#include <linux/bitops.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"

#define SDCTRL_EMMC_HW_RESET	0x59810280

void uniphier_pxs3_clk_init(void)
{
	u32 tmp;

	tmp = readl(sc_base + SC_RSTCTRL6);
	tmp |= BIT(8);			/* Mali */
	writel(tmp, sc_base + SC_RSTCTRL6);

	tmp = readl(sc_base + SC_CLKCTRL6);
	tmp |= BIT(8);			/* Mali */
	writel(tmp, sc_base + SC_CLKCTRL6);

	/* TODO: use "mmc-pwrseq-emmc" */
	writel(1, SDCTRL_EMMC_HW_RESET);
}
