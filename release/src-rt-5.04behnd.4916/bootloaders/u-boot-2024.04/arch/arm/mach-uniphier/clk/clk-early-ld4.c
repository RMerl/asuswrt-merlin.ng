// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 */

#include <spl.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

void uniphier_ld4_early_clk_init(void)
{
	u32 tmp;

	/* provide clocks */
	tmp = readl(sc_base + SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_SBC | SC_CLKCTRL_CEN_PERI;
	writel(tmp, sc_base + SC_CLKCTRL);
	readl(sc_base + SC_CLKCTRL); /* dummy read */
}
