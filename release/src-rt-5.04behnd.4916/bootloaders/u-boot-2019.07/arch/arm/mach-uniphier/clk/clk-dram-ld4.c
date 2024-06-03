// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

void uniphier_ld4_dram_clk_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL);
	tmp |= SC_RSTCTRL_NRST_UMC1 | SC_RSTCTRL_NRST_UMC0;
	writel(tmp, SC_RSTCTRL);
	readl(SC_RSTCTRL); /* dummy read */

	/* provide clocks */
	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_UMC;
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
