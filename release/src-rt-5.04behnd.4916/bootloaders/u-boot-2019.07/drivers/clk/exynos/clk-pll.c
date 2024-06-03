// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos PLL helper functions for clock drivers.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>

#define PLL145X_MDIV_SHIFT	16
#define PLL145X_MDIV_MASK	0x3ff
#define PLL145X_PDIV_SHIFT	8
#define PLL145X_PDIV_MASK	0x3f
#define PLL145X_SDIV_SHIFT	0
#define PLL145X_SDIV_MASK	0x7

unsigned long pll145x_get_rate(unsigned int *con1, unsigned long fin_freq)
{
	unsigned long pll_con1 = readl(con1);
	unsigned long mdiv, sdiv, pdiv;
	uint64_t fvco = fin_freq;

	mdiv = (pll_con1 >> PLL145X_MDIV_SHIFT) & PLL145X_MDIV_MASK;
	pdiv = (pll_con1 >> PLL145X_PDIV_SHIFT) & PLL145X_PDIV_MASK;
	sdiv = (pll_con1 >> PLL145X_SDIV_SHIFT) & PLL145X_SDIV_MASK;

	fvco *= mdiv;
	do_div(fvco, (pdiv << sdiv));
	return (unsigned long)fvco;
}
