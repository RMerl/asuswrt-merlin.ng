// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#include <linux/delay.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"
#include "../sg-regs.h"
#include "pll.h"

static void vpll_init(void)
{
	u32 tmp, clk_mode_axosel;

	/* Set VPLL27A &  VPLL27B */
	tmp = readl(SG_PINMON0);
	clk_mode_axosel = tmp & SG_PINMON0_CLK_MODE_AXOSEL_MASK;

	/* 25MHz or 6.25MHz is default for Pro4R, no need to set VPLLA/B */
	if (clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ ||
	    clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_6250KHZ)
		return;

	/* Disable write protect of VPLL27ACTRL[2-7]*, VPLL27BCTRL[2-8] */
	tmp = readl(SC_VPLL27ACTRL);
	tmp |= 0x00000001;
	writel(tmp, SC_VPLL27ACTRL);
	tmp = readl(SC_VPLL27BCTRL);
	tmp |= 0x00000001;
	writel(tmp, SC_VPLL27BCTRL);

	/* Unset VPLA_K_LD and VPLB_K_LD bit */
	tmp = readl(SC_VPLL27ACTRL3);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27ACTRL3);
	tmp = readl(SC_VPLL27BCTRL3);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27BCTRL3);

	/* Set VPLA_M and VPLB_M to 0x20 */
	tmp = readl(SC_VPLL27ACTRL2);
	tmp &= ~0x0000007f;
	tmp |= 0x00000020;
	writel(tmp, SC_VPLL27ACTRL2);
	tmp = readl(SC_VPLL27BCTRL2);
	tmp &= ~0x0000007f;
	tmp |= 0x00000020;
	writel(tmp, SC_VPLL27BCTRL2);

	if (clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ ||
	    clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_6250KHZ) {
		/* Set VPLA_K and VPLB_K for AXO: 25MHz */
		tmp = readl(SC_VPLL27ACTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x00066666;
		writel(tmp, SC_VPLL27ACTRL3);
		tmp = readl(SC_VPLL27BCTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x00066666;
		writel(tmp, SC_VPLL27BCTRL3);
	} else {
		/* Set VPLA_K and VPLB_K for AXO: 24.576 MHz */
		tmp = readl(SC_VPLL27ACTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x000f5800;
		writel(tmp, SC_VPLL27ACTRL3);
		tmp = readl(SC_VPLL27BCTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x000f5800;
		writel(tmp, SC_VPLL27BCTRL3);
	}

	/* wait 1 usec */
	udelay(1);

	/* Set VPLA_K_LD and VPLB_K_LD to load K parameters */
	tmp = readl(SC_VPLL27ACTRL3);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27ACTRL3);
	tmp = readl(SC_VPLL27BCTRL3);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27BCTRL3);

	/* Unset VPLA_SNRST and VPLB_SNRST bit */
	tmp = readl(SC_VPLL27ACTRL2);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27ACTRL2);
	tmp = readl(SC_VPLL27BCTRL2);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27BCTRL2);

	/* Enable write protect of VPLL27ACTRL[2-7]*, VPLL27BCTRL[2-8] */
	tmp = readl(SC_VPLL27ACTRL);
	tmp &= ~0x00000001;
	writel(tmp, SC_VPLL27ACTRL);
	tmp = readl(SC_VPLL27BCTRL);
	tmp &= ~0x00000001;
	writel(tmp, SC_VPLL27BCTRL);
}

void uniphier_pro4_pll_init(void)
{
	vpll_init();
	uniphier_ld4_dpll_ssc_en();
}
