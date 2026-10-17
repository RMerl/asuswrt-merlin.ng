// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "../sc64-regs.h"
#include "pll.h"

/* PLL type: SSC */
#define SC_PLLCTRL_SSC_DK_MASK		GENMASK(14, 0)
#define SC_PLLCTRL_SSC_EN		BIT(31)
#define SC_PLLCTRL2_NRSTDS		BIT(28)
#define SC_PLLCTRL2_SSC_JK_MASK		GENMASK(26, 0)
#define SC_PLLCTRL3_REGI_MASK		GENMASK(19, 16)

/* PLL type: VPLL27 */
#define SC_VPLL27CTRL_WP		BIT(0)
#define SC_VPLL27CTRL3_K_LD		BIT(28)

/* PLL type: DSPLL */
#define SC_DSPLLCTRL2_K_LD		BIT(28)

int uniphier_ld20_sscpll_init(unsigned long reg_base, unsigned int freq,
			      unsigned int ssc_rate, unsigned int divn)
{
	void __iomem *base = sc_base + reg_base;
	u32 tmp;

	if (freq != UNIPHIER_PLL_FREQ_DEFAULT) {
		tmp = readl(base);	/* SSCPLLCTRL */
		tmp &= ~SC_PLLCTRL_SSC_DK_MASK;
		tmp |= FIELD_PREP(SC_PLLCTRL_SSC_DK_MASK,
				  DIV_ROUND_CLOSEST(487UL * freq * ssc_rate,
						    divn * 512));
		writel(tmp, base);

		tmp = readl(base + 4);
		tmp &= ~SC_PLLCTRL2_SSC_JK_MASK;
		tmp |= FIELD_PREP(SC_PLLCTRL2_SSC_JK_MASK,
				  DIV_ROUND_CLOSEST(21431887UL * freq,
						    divn * 512));
		writel(tmp, base + 4);

		udelay(50);
	}

	tmp = readl(base + 4);		/* SSCPLLCTRL2 */
	tmp |= SC_PLLCTRL2_NRSTDS;
	writel(tmp, base + 4);

	return 0;
}

int uniphier_ld20_sscpll_ssc_en(unsigned long reg_base)
{
	void __iomem *base = sc_base + reg_base;
	u32 tmp;

	tmp = readl(base);	/* SSCPLLCTRL */
	tmp |= SC_PLLCTRL_SSC_EN;
	writel(tmp, base);

	return 0;
}

int uniphier_ld20_sscpll_set_regi(unsigned long reg_base, unsigned regi)
{
	void __iomem *base = sc_base + reg_base;
	u32 tmp;

	tmp = readl(base + 8);	/* SSCPLLCTRL3 */
	tmp &= ~SC_PLLCTRL3_REGI_MASK;
	tmp |= FIELD_PREP(SC_PLLCTRL3_REGI_MASK, regi);
	writel(tmp, base + 8);

	return 0;
}

int uniphier_ld20_vpll27_init(unsigned long reg_base)
{
	void __iomem *base = sc_base + reg_base;
	u32 tmp;

	tmp = readl(base);		/* VPLL27CTRL */
	tmp |= SC_VPLL27CTRL_WP;	/* write protect off */
	writel(tmp, base);

	tmp = readl(base + 8);		/* VPLL27CTRL3 */
	tmp |= SC_VPLL27CTRL3_K_LD;
	writel(tmp, base + 8);

	tmp = readl(base);		/* VPLL27CTRL */
	tmp &= ~SC_VPLL27CTRL_WP;	/* write protect on */
	writel(tmp, base);

	return 0;
}

int uniphier_ld20_dspll_init(unsigned long reg_base)
{
	void __iomem *base = sc_base + reg_base;
	u32 tmp;

	tmp = readl(base + 4);		/* DSPLLCTRL2 */
	tmp |= SC_DSPLLCTRL2_K_LD;
	writel(tmp, base + 4);

	return 0;
}
