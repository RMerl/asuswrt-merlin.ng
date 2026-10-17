// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017-2019 NXP.
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <clk.h>
#include <div64.h>
#include <linux/printk.h>

#include "clk.h"

#define UBOOT_DM_CLK_IMX_PLL1443X "imx_clk_pll1443x"
#define UBOOT_DM_CLK_IMX_PLL1416X "imx_clk_pll1416x"

#define GNRL_CTL	0x0
#define DIV_CTL		0x4
#define LOCK_STATUS	BIT(31)
#define LOCK_SEL_MASK	BIT(29)
#define CLKE_MASK	BIT(11)
#define RST_MASK	BIT(9)
#define BYPASS_MASK	BIT(4)
#define MDIV_SHIFT	12
#define MDIV_MASK	GENMASK(21, 12)
#define PDIV_SHIFT	4
#define PDIV_MASK	GENMASK(9, 4)
#define SDIV_SHIFT	0
#define SDIV_MASK	GENMASK(2, 0)
#define KDIV_SHIFT	0
#define KDIV_MASK	GENMASK(15, 0)

#define LOCK_TIMEOUT_US		10000

struct clk_pll14xx {
	struct clk			clk;
	void __iomem			*base;
	enum imx_pll14xx_type		type;
	const struct imx_pll14xx_rate_table *rate_table;
	int rate_count;
};

#define to_clk_pll14xx(_clk) container_of(_clk, struct clk_pll14xx, clk)

#define PLL_1416X_RATE(_rate, _m, _p, _s)		\
	{						\
		.rate	=	(_rate),		\
		.mdiv	=	(_m),			\
		.pdiv	=	(_p),			\
		.sdiv	=	(_s),			\
	}

#define PLL_1443X_RATE(_rate, _m, _p, _s, _k)		\
	{						\
		.rate	=	(_rate),		\
		.mdiv	=	(_m),			\
		.pdiv	=	(_p),			\
		.sdiv	=	(_s),			\
		.kdiv	=	(_k),			\
	}

static const struct imx_pll14xx_rate_table imx_pll1416x_tbl[] = {
	PLL_1416X_RATE(1800000000U, 225, 3, 0),
	PLL_1416X_RATE(1600000000U, 200, 3, 0),
	PLL_1416X_RATE(1500000000U, 375, 3, 1),
	PLL_1416X_RATE(1400000000U, 350, 3, 1),
	PLL_1416X_RATE(1200000000U, 300, 3, 1),
	PLL_1416X_RATE(1000000000U, 250, 3, 1),
	PLL_1416X_RATE(800000000U,  200, 3, 1),
	PLL_1416X_RATE(750000000U,  250, 2, 2),
	PLL_1416X_RATE(700000000U,  350, 3, 2),
	PLL_1416X_RATE(600000000U,  300, 3, 2),
};

const struct imx_pll14xx_rate_table imx_pll1443x_tbl[] = {
	PLL_1443X_RATE(1039500000U, 173, 2, 1, 16384),
	PLL_1443X_RATE(650000000U, 325, 3, 2, 0),
	PLL_1443X_RATE(594000000U, 198, 2, 2, 0),
	PLL_1443X_RATE(519750000U, 173, 2, 2, 16384),
	PLL_1443X_RATE(393216000U, 262, 2, 3, 9437),
	PLL_1443X_RATE(361267200U, 361, 3, 3, 17511),
};

struct imx_pll14xx_clk imx_1443x_pll __initdata = {
	.type = PLL_1443X,
	.rate_table = imx_pll1443x_tbl,
	.rate_count = ARRAY_SIZE(imx_pll1443x_tbl),
};
EXPORT_SYMBOL_GPL(imx_1443x_pll);

struct imx_pll14xx_clk imx_1443x_dram_pll __initdata = {
	.type = PLL_1443X,
	.rate_table = imx_pll1443x_tbl,
	.rate_count = ARRAY_SIZE(imx_pll1443x_tbl),
	.flags = CLK_GET_RATE_NOCACHE,
};
EXPORT_SYMBOL_GPL(imx_1443x_dram_pll);

struct imx_pll14xx_clk imx_1416x_pll __initdata = {
	.type = PLL_1416X,
	.rate_table = imx_pll1416x_tbl,
	.rate_count = ARRAY_SIZE(imx_pll1416x_tbl),
};
EXPORT_SYMBOL_GPL(imx_1416x_pll);

static const struct imx_pll14xx_rate_table *imx_get_pll_settings(
		struct clk_pll14xx *pll, unsigned long rate)
{
	const struct imx_pll14xx_rate_table *rate_table = pll->rate_table;
	int i;

	for (i = 0; i < pll->rate_count; i++)
		if (rate == rate_table[i].rate)
			return &rate_table[i];

	return NULL;
}

static unsigned long clk_pll1416x_recalc_rate(struct clk *clk)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	u64 fvco = clk_get_parent_rate(clk);
	u32 mdiv, pdiv, sdiv, pll_div;

	pll_div = readl(pll->base + 4);
	mdiv = (pll_div & MDIV_MASK) >> MDIV_SHIFT;
	pdiv = (pll_div & PDIV_MASK) >> PDIV_SHIFT;
	sdiv = (pll_div & SDIV_MASK) >> SDIV_SHIFT;

	fvco *= mdiv;
	do_div(fvco, pdiv << sdiv);

	return fvco;
}

static unsigned long clk_pll1443x_recalc_rate(struct clk *clk)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	u64 fvco = clk_get_parent_rate(clk);
	u32 mdiv, pdiv, sdiv, pll_div_ctl0, pll_div_ctl1;
	short int kdiv;

	pll_div_ctl0 = readl(pll->base + 4);
	pll_div_ctl1 = readl(pll->base + 8);
	mdiv = (pll_div_ctl0 & MDIV_MASK) >> MDIV_SHIFT;
	pdiv = (pll_div_ctl0 & PDIV_MASK) >> PDIV_SHIFT;
	sdiv = (pll_div_ctl0 & SDIV_MASK) >> SDIV_SHIFT;
	kdiv = pll_div_ctl1 & KDIV_MASK;

	/* fvco = (m * 65536 + k) * Fin / (p * 65536) */
	fvco *= (mdiv * 65536 + kdiv);
	pdiv *= 65536;

	do_div(fvco, pdiv << sdiv);

	return fvco;
}

static inline bool clk_pll1416x_mp_change(const struct imx_pll14xx_rate_table *rate,
					  u32 pll_div)
{
	u32 old_mdiv, old_pdiv;

	old_mdiv = (pll_div & MDIV_MASK) >> MDIV_SHIFT;
	old_pdiv = (pll_div & PDIV_MASK) >> PDIV_SHIFT;

	return rate->mdiv != old_mdiv || rate->pdiv != old_pdiv;
}

static inline bool clk_pll1443x_mpk_change(const struct imx_pll14xx_rate_table *rate,
					   u32 pll_div_ctl0, u32 pll_div_ctl1)
{
	u32 old_mdiv, old_pdiv, old_kdiv;

	old_mdiv = (pll_div_ctl0 & MDIV_MASK) >> MDIV_SHIFT;
	old_pdiv = (pll_div_ctl0 & PDIV_MASK) >> PDIV_SHIFT;
	old_kdiv = (pll_div_ctl1 & KDIV_MASK) >> KDIV_SHIFT;

	return rate->mdiv != old_mdiv || rate->pdiv != old_pdiv ||
		rate->kdiv != old_kdiv;
}

static inline bool clk_pll1443x_mp_change(const struct imx_pll14xx_rate_table *rate,
					  u32 pll_div_ctl0, u32 pll_div_ctl1)
{
	u32 old_mdiv, old_pdiv, old_kdiv;

	old_mdiv = (pll_div_ctl0 & MDIV_MASK) >> MDIV_SHIFT;
	old_pdiv = (pll_div_ctl0 & PDIV_MASK) >> PDIV_SHIFT;
	old_kdiv = (pll_div_ctl1 & KDIV_MASK) >> KDIV_SHIFT;

	return rate->mdiv != old_mdiv || rate->pdiv != old_pdiv ||
		rate->kdiv != old_kdiv;
}

static int clk_pll14xx_wait_lock(struct clk_pll14xx *pll)
{
	u32 val;

	return readl_poll_timeout(pll->base, val, val & LOCK_TIMEOUT_US,
			LOCK_TIMEOUT_US);
}

static ulong clk_pll1416x_set_rate(struct clk *clk, unsigned long drate)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	const struct imx_pll14xx_rate_table *rate;
	u32 tmp, div_val;
	int ret;

	rate = imx_get_pll_settings(pll, drate);
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for pll clk %s\n", __func__,
		       drate, "xxxx");
		return -EINVAL;
	}

	tmp = readl(pll->base + 4);

	if (!clk_pll1416x_mp_change(rate, tmp)) {
		tmp &= ~(SDIV_MASK) << SDIV_SHIFT;
		tmp |= rate->sdiv << SDIV_SHIFT;
		writel(tmp, pll->base + 4);

		return clk_pll1416x_recalc_rate(clk);
	}

	/* Bypass clock and set lock to pll output lock */
	tmp = readl(pll->base);
	tmp |= LOCK_SEL_MASK;
	writel(tmp, pll->base);

	/* Enable RST */
	tmp &= ~RST_MASK;
	writel(tmp, pll->base);

	/* Enable BYPASS */
	tmp |= BYPASS_MASK;
	writel(tmp, pll->base);


	div_val = (rate->mdiv << MDIV_SHIFT) | (rate->pdiv << PDIV_SHIFT) |
		(rate->sdiv << SDIV_SHIFT);
	writel(div_val, pll->base + 0x4);

	/*
	 * According to SPEC, t3 - t2 need to be greater than
	 * 1us and 1/FREF, respectively.
	 * FREF is FIN / Prediv, the prediv is [1, 63], so choose
	 * 3us.
	 */
	udelay(3);

	/* Disable RST */
	tmp |= RST_MASK;
	writel(tmp, pll->base);

	/* Wait Lock */
	ret = clk_pll14xx_wait_lock(pll);
	if (ret)
		return ret;

	/* Bypass */
	tmp &= ~BYPASS_MASK;
	writel(tmp, pll->base);

	return clk_pll1416x_recalc_rate(clk);
}

static ulong clk_pll1443x_set_rate(struct clk *clk, unsigned long drate)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	const struct imx_pll14xx_rate_table *rate;
	u32 tmp, div_val;
	int ret;

	rate = imx_get_pll_settings(pll, drate);
	if (!rate) {
		pr_err("%s: Invalid rate : %lu for pll clk %s\n", __func__,
		       drate, "===");
		return -EINVAL;
	}

	tmp = readl(pll->base + 4);
	div_val = readl(pll->base + 8);

	if (!clk_pll1443x_mpk_change(rate, tmp, div_val)) {
		tmp &= ~(SDIV_MASK) << SDIV_SHIFT;
		tmp |= rate->sdiv << SDIV_SHIFT;
		writel(tmp, pll->base + 4);

		return clk_pll1443x_recalc_rate(clk);
	}

	tmp = readl(pll->base);

	/* Enable RST */
	tmp &= ~RST_MASK;
	writel(tmp, pll->base);

	/* Enable BYPASS */
	tmp |= BYPASS_MASK;
	writel(tmp, pll->base);

	div_val = (rate->mdiv << MDIV_SHIFT) | (rate->pdiv << PDIV_SHIFT) |
		(rate->sdiv << SDIV_SHIFT);
	writel(div_val, pll->base + 0x4);
	writel(rate->kdiv << KDIV_SHIFT, pll->base + 0x8);

	/*
	 * According to SPEC, t3 - t2 need to be greater than
	 * 1us and 1/FREF, respectively.
	 * FREF is FIN / Prediv, the prediv is [1, 63], so choose
	 * 3us.
	 */
	udelay(3);

	/* Disable RST */
	tmp |= RST_MASK;
	writel(tmp, pll->base);

	/* Wait Lock*/
	ret = clk_pll14xx_wait_lock(pll);
	if (ret)
		return ret;

	/* Bypass */
	tmp &= ~BYPASS_MASK;
	writel(tmp, pll->base);

	return clk_pll1443x_recalc_rate(clk);
}

static int clk_pll14xx_prepare(struct clk *clk)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	u32 val;

	/*
	 * RESETB = 1 from 0, PLL starts its normal
	 * operation after lock time
	 */
	val = readl(pll->base + GNRL_CTL);
	val |= RST_MASK;
	writel(val, pll->base + GNRL_CTL);

	return clk_pll14xx_wait_lock(pll);
}

static int clk_pll14xx_unprepare(struct clk *clk)
{
	struct clk_pll14xx *pll = to_clk_pll14xx(dev_get_clk_ptr(clk->dev));
	u32 val;

	/*
	 * Set RST to 0, power down mode is enabled and
	 * every digital block is reset
	 */
	val = readl(pll->base + GNRL_CTL);
	val &= ~RST_MASK;
	writel(val, pll->base + GNRL_CTL);

	return 0;
}

static const struct clk_ops clk_pll1416x_ops = {
	.enable		= clk_pll14xx_prepare,
	.disable	= clk_pll14xx_unprepare,
	.set_rate	= clk_pll1416x_set_rate,
	.get_rate	= clk_pll1416x_recalc_rate,
};

static const struct clk_ops clk_pll1443x_ops = {
	.enable		= clk_pll14xx_prepare,
	.disable	= clk_pll14xx_unprepare,
	.set_rate	= clk_pll1443x_set_rate,
	.get_rate	= clk_pll1443x_recalc_rate,
};

struct clk *imx_clk_pll14xx(const char *name, const char *parent_name,
			    void __iomem *base,
			    const struct imx_pll14xx_clk *pll_clk)
{
	struct clk_pll14xx *pll;
	struct clk *clk;
	char *type_name;
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	switch (pll_clk->type) {
	case PLL_1416X:
		type_name = UBOOT_DM_CLK_IMX_PLL1416X;
		break;
	case PLL_1443X:
		type_name = UBOOT_DM_CLK_IMX_PLL1443X;
		break;
	default:
		pr_err("%s: Unknown pll type for pll clk %s\n",
		       __func__, name);
		return ERR_PTR(-EINVAL);
	};

	pll->base = base;
	pll->type = pll_clk->type;
	pll->rate_table = pll_clk->rate_table;
	pll->rate_count = pll_clk->rate_count;

	clk = &pll->clk;

	ret = clk_register(clk, type_name, name, parent_name);
	if (ret) {
		pr_err("%s: failed to register pll %s %d\n",
		       __func__, name, ret);
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_pll1443x) = {
	.name	= UBOOT_DM_CLK_IMX_PLL1443X,
	.id	= UCLASS_CLK,
	.ops	= &clk_pll1443x_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pll1416x) = {
	.name	= UBOOT_DM_CLK_IMX_PLL1416X,
	.id	= UCLASS_CLK,
	.ops	= &clk_pll1416x_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
