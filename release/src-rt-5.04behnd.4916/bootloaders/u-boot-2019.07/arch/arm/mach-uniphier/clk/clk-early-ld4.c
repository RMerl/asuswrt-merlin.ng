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

void uniphier_ld4_early_clk_init(void)
{
	u32 tmp;

	/* deassert reset */
	if (spl_boot_device() != BOOT_DEVICE_NAND) {
		tmp = readl(SC_RSTCTRL);
		tmp &= ~SC_RSTCTRL_NRST_NAND;
		writel(tmp, SC_RSTCTRL);
	};

	/* provide clocks */
	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_SBC | SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
