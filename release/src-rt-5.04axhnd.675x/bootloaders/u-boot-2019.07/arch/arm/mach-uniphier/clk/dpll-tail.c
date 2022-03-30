// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/io.h>

#include "../sc-regs.h"
#include "pll.h"

void uniphier_ld4_dpll_ssc_en(void)
{
	u32 tmp;

	tmp = readl(SC_DPLLCTRL);
	tmp |= SC_DPLLCTRL_SSC_EN;
	writel(tmp, SC_DPLLCTRL);
}
