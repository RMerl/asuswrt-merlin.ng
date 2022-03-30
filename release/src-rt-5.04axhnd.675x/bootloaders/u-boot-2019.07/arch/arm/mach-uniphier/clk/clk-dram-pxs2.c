// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

void uniphier_pxs2_dram_clk_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL4);
	tmp |= SC_RSTCTRL4_NRST_UMCSB | SC_RSTCTRL4_NRST_UMCA2 |
	       SC_RSTCTRL4_NRST_UMCA1 | SC_RSTCTRL4_NRST_UMCA0 |
	       SC_RSTCTRL4_NRST_UMC32 | SC_RSTCTRL4_NRST_UMC31 |
	       SC_RSTCTRL4_NRST_UMC30;
	writel(tmp, SC_RSTCTRL4);
	readl(SC_RSTCTRL4); /* dummy read */

	/* provide clocks */
	tmp = readl(SC_CLKCTRL4);
	tmp |= SC_CLKCTRL4_CEN_UMCSB | SC_CLKCTRL4_CEN_UMC2 |
	       SC_CLKCTRL4_CEN_UMC1 | SC_CLKCTRL4_CEN_UMC0;
	writel(tmp, SC_CLKCTRL4);
	readl(SC_CLKCTRL4); /* dummy read */
}
