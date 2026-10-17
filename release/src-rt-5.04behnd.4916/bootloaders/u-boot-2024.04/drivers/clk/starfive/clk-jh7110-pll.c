// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-23 StarFive Technology Co., Ltd.
 *
 * Author:	Yanhong Wang <yanhong.wang@starfivetech.com>
 *		Xingyu Wu <xingyu.wu@starfivetech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dt-bindings/clock/starfive,jh7110-crg.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "clk.h"

#define UBOOT_DM_CLK_JH7110_PLLX "jh7110_clk_pllx"

#define PLL_PD_OFF		1
#define PLL_PD_ON		0

#define CLK_DDR_BUS_MASK	GENMASK(29, 24)
#define CLK_DDR_BUS_OFFSET	0xAC
#define CLK_DDR_BUS_OSC_DIV2	0
#define CLK_DDR_BUS_PLL1_DIV2	1
#define CLK_DDR_BUS_PLL1_DIV4	2
#define CLK_DDR_BUS_PLL1_DIV8	3

#define JH7110_PLL_ID_TRANS(id)	((id) + JH7110_EXTCLK_END)

enum starfive_pll_type {
	PLL0 = 0,
	PLL1,
	PLL2,
	PLL_MAX = PLL2
};

struct starfive_pllx_rate {
	u64 rate;
	u32 prediv;
	u32 fbdiv;
	u32 frac;
};

struct starfive_pllx_offset {
	u32 pd;
	u32 prediv;
	u32 fbdiv;
	u32 frac;
	u32 postdiv1;
	u32 dacpd;
	u32 dsmpd;
	u32 pd_mask;
	u32 prediv_mask;
	u32 fbdiv_mask;
	u32 frac_mask;
	u32 postdiv1_mask;
	u32 dacpd_mask;
	u32 dsmpd_mask;
};

struct starfive_pllx_clk {
	enum starfive_pll_type type;
	const struct starfive_pllx_offset *offset;
	const struct starfive_pllx_rate *rate_table;
	int rate_count;
	int flags;
};

struct clk_jh7110_pllx {
	struct clk		clk;
	void __iomem	*base;
	void __iomem	*sysreg;
	enum starfive_pll_type	type;
	const struct starfive_pllx_offset *offset;
	const struct starfive_pllx_rate *rate_table;
	int rate_count;
};

#define getbits_le32(addr, mask) ((in_le32(addr) & (mask)) >> __ffs((mask)))

#define PLLX_SET(offset, mask, val) do {\
		reg = readl((ulong *)((ulong)pll->base + (offset))); \
		reg &= ~(mask); \
		reg |= (mask) & ((val) << __ffs(mask)); \
		writel(reg, (ulong *)((ulong)pll->base + (offset))); \
	} while (0)

#define PLLX_RATE(_rate, _pd, _fd)	\
	{						\
		.rate		= (_rate),	\
		.prediv		= (_pd),	\
		.fbdiv		= (_fd),	\
	}

#define to_clk_pllx(_clk) container_of(_clk, struct clk_jh7110_pllx, clk)

static const struct starfive_pllx_rate jh7110_pll0_tbl[] = {
	PLLX_RATE(375000000UL, 8, 125),
	PLLX_RATE(500000000UL, 6, 125),
	PLLX_RATE(625000000UL, 24, 625),
	PLLX_RATE(750000000UL, 4, 125),
	PLLX_RATE(875000000UL, 24, 875),
	PLLX_RATE(1000000000UL, 3, 125),
	PLLX_RATE(1250000000UL, 12, 625),
	PLLX_RATE(1375000000UL, 24, 1375),
	PLLX_RATE(1500000000UL, 2, 125),
	PLLX_RATE(1625000000UL, 24, 1625),
	PLLX_RATE(1750000000UL, 12, 875),
	PLLX_RATE(1800000000UL, 3, 225),
};

static const struct starfive_pllx_rate jh7110_pll1_tbl[] = {
	PLLX_RATE(1066000000UL, 12, 533),
	PLLX_RATE(1200000000UL, 1, 50),
	PLLX_RATE(1400000000UL, 6, 350),
	PLLX_RATE(1600000000UL, 3, 200),
};

static const struct starfive_pllx_rate jh7110_pll2_tbl[] = {
	PLLX_RATE(1228800000UL, 15, 768),
	PLLX_RATE(1188000000UL, 2, 99),
};

static const struct starfive_pllx_offset jh7110_pll0_offset = {
	.pd = 0x20,
	.prediv = 0x24,
	.fbdiv = 0x1c,
	.frac = 0x20,
	.postdiv1 = 0x20,
	.dacpd = 0x18,
	.dsmpd = 0x18,
	.pd_mask = BIT(27),
	.prediv_mask = GENMASK(5, 0),
	.fbdiv_mask = GENMASK(11, 0),
	.frac_mask = GENMASK(23, 0),
	.postdiv1_mask = GENMASK(29, 28),
	.dacpd_mask = BIT(24),
	.dsmpd_mask = BIT(25)
};

static const struct starfive_pllx_offset jh7110_pll1_offset = {
	.pd = 0x28,
	.prediv = 0x2c,
	.fbdiv = 0x24,
	.frac = 0x28,
	.postdiv1 = 0x28,
	.dacpd = 0x24,
	.dsmpd = 0x24,
	.pd_mask = BIT(27),
	.prediv_mask = GENMASK(5, 0),
	.fbdiv_mask = GENMASK(28, 17),
	.frac_mask = GENMASK(23, 0),
	.postdiv1_mask = GENMASK(29, 28),
	.dacpd_mask = BIT(15),
	.dsmpd_mask = BIT(16)
};

static const struct starfive_pllx_offset jh7110_pll2_offset = {
	.pd = 0x30,
	.prediv = 0x34,
	.fbdiv = 0x2c,
	.frac = 0x30,
	.postdiv1 = 0x30,
	.dacpd = 0x2c,
	.dsmpd = 0x2c,
	.pd_mask = BIT(27),
	.prediv_mask = GENMASK(5, 0),
	.fbdiv_mask = GENMASK(28, 17),
	.frac_mask = GENMASK(23, 0),
	.postdiv1_mask = GENMASK(29, 28),
	.dacpd_mask = BIT(15),
	.dsmpd_mask = BIT(16)
};

struct starfive_pllx_clk starfive_jh7110_pll0 __initdata = {
	.type = PLL0,
	.offset = &jh7110_pll0_offset,
	.rate_table = jh7110_pll0_tbl,
	.rate_count = ARRAY_SIZE(jh7110_pll0_tbl),
};

struct starfive_pllx_clk starfive_jh7110_pll1 __initdata = {
	.type = PLL1,
	.offset = &jh7110_pll1_offset,
	.rate_table = jh7110_pll1_tbl,
	.rate_count = ARRAY_SIZE(jh7110_pll1_tbl),
};

struct starfive_pllx_clk starfive_jh7110_pll2 __initdata = {
	.type = PLL2,
	.offset = &jh7110_pll2_offset,
	.rate_table = jh7110_pll2_tbl,
	.rate_count = ARRAY_SIZE(jh7110_pll2_tbl),
};

static const struct starfive_pllx_rate *
jh7110_get_pll_settings(struct clk_jh7110_pllx *pll, unsigned long rate)
{
	for (int i = 0; i < pll->rate_count; i++)
		if (rate == pll->rate_table[i].rate)
			return &pll->rate_table[i];

	return NULL;
}

static void jh7110_pll_set_rate(struct clk_jh7110_pllx *pll,
				const struct starfive_pllx_rate *rate)
{
	u32 reg;
	bool set = (pll->type == PLL1) ? true : false;

	if (set) {
		reg = readl((ulong *)((ulong)pll->sysreg + CLK_DDR_BUS_OFFSET));
		reg &= ~CLK_DDR_BUS_MASK;
		reg |= CLK_DDR_BUS_OSC_DIV2 << __ffs(CLK_DDR_BUS_MASK);
		writel(reg, (ulong *)((ulong)pll->sysreg + CLK_DDR_BUS_OFFSET));
	}

	PLLX_SET(pll->offset->pd, pll->offset->pd_mask, PLL_PD_OFF);
	PLLX_SET(pll->offset->dacpd, pll->offset->dacpd_mask, 1);
	PLLX_SET(pll->offset->dsmpd, pll->offset->dsmpd_mask, 1);
	PLLX_SET(pll->offset->prediv, pll->offset->prediv_mask, rate->prediv);
	PLLX_SET(pll->offset->fbdiv, pll->offset->fbdiv_mask, rate->fbdiv);
	PLLX_SET(pll->offset->postdiv1, pll->offset->postdiv1_mask, 0);
	PLLX_SET(pll->offset->pd, pll->offset->pd_mask, PLL_PD_ON);

	if (set) {
		udelay(100);
		reg = readl((ulong *)((ulong)pll->sysreg + CLK_DDR_BUS_OFFSET));
		reg &= ~CLK_DDR_BUS_MASK;
		reg |= CLK_DDR_BUS_PLL1_DIV2 << __ffs(CLK_DDR_BUS_MASK);
		writel(reg, (ulong *)((ulong)pll->sysreg + CLK_DDR_BUS_OFFSET));
	}
}

static ulong jh7110_pllx_recalc_rate(struct clk *clk)
{
	struct clk_jh7110_pllx *pll = to_clk_pllx(dev_get_clk_ptr(clk->dev));
	u64 refclk = clk_get_parent_rate(clk);
	u32 dacpd, dsmpd;
	u32 prediv, fbdiv, postdiv1;
	u64 frac;

	dacpd = getbits_le32((ulong)pll->base + pll->offset->dacpd,
			     pll->offset->dacpd_mask);
	dsmpd = getbits_le32((ulong)pll->base + pll->offset->dsmpd,
			     pll->offset->dsmpd_mask);
	prediv = getbits_le32((ulong)pll->base + pll->offset->prediv,
			      pll->offset->prediv_mask);
	fbdiv = getbits_le32((ulong)pll->base + pll->offset->fbdiv,
			     pll->offset->fbdiv_mask);
	postdiv1 = 1 << getbits_le32((ulong)pll->base + pll->offset->postdiv1,
			pll->offset->postdiv1_mask);
	frac = (u64)getbits_le32((ulong)pll->base + pll->offset->frac,
			pll->offset->frac_mask);

	/* Integer Multiple Mode
	 * Both dacpd and dsmpd should be set as 1 while integer multiple mode.
	 *
	 * The frequency of outputs can be figured out as below.
	 *
     *       Fvco = Fref*Nl/M
	 * NI is integer frequency dividing ratio of feedback divider, set by fbdiv1[11:0] ,
	 *    NI = 8, 9, 10, 12.13....4095
	 * M is frequency dividing ratio of pre-divider, set by prediv[5:0],M = 1,2...63
	 *
     *      Fclko1 = Fvco/Q1
	 * Q1 is frequency dividing ratio of post divider, set by postdiv1[1:0],Q1= 1,2,4,8
	 *
	 * Fraction Multiple Mode
	 *
	 *  Both dacpd and dsmpd should be set as 0 while integer multiple mode.
	 *
     *      Fvco = Fref*(NI+NF)/M
	 * NI is integer frequency dividing ratio of feedback divider, set by fbdiv[11:0] ,
	 *     NI = 8, 9, 10, 12.13....4095
	 * NF is fractional frequency dividing ratio, set by frac[23:0],  NF =frac[23:0]/2^24= 0~0.99999994
	 * M is frequency dividing ratio of pre-divider, set by prediv[5:0],M = 1,2...63
	 *
     *     Fclko1 = Fvco/Q1
	 * Q1 is frequency dividing ratio of post divider, set by postdivl[1:0],Q1= 1,2,4,8
	 */
	if (dacpd == 1 && dsmpd == 1)
		frac = 0;
	else if (dacpd == 0 && dsmpd == 0)
		do_div(frac, 1 << 24);
	else
		return -EINVAL;

	refclk *= (fbdiv + frac);
	do_div(refclk, prediv * postdiv1);

	return refclk;
}

static ulong jh7110_pllx_set_rate(struct clk *clk, ulong drate)
{
	struct clk_jh7110_pllx *pll = to_clk_pllx(dev_get_clk_ptr(clk->dev));
	const struct starfive_pllx_rate *rate;

	rate = jh7110_get_pll_settings(pll, drate);
	if (!rate)
		return -EINVAL;

	jh7110_pll_set_rate(pll, rate);

	return jh7110_pllx_recalc_rate(clk);
}

static const struct clk_ops jh7110_clk_pllx_ops = {
	.set_rate	= jh7110_pllx_set_rate,
	.get_rate	= jh7110_pllx_recalc_rate,
};

struct clk *starfive_jh7110_pll(const char *name, const char *parent_name,
				void __iomem *base, void __iomem *sysreg,
				const struct starfive_pllx_clk *pll_clk)
{
	struct clk_jh7110_pllx *pll;
	struct clk *clk;
	int ret;

	if (!pll_clk || !base || !sysreg)
		return ERR_PTR(-EINVAL);

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->base = base;
	pll->sysreg = sysreg;
	pll->type = pll_clk->type;
	pll->offset = pll_clk->offset;
	pll->rate_table = pll_clk->rate_table;
	pll->rate_count = pll_clk->rate_count;

	clk = &pll->clk;
	ret = clk_register(clk, UBOOT_DM_CLK_JH7110_PLLX, name, parent_name);
	if (ret) {
		kfree(pll);
		return ERR_PTR(ret);
	}

	if (IS_ENABLED(CONFIG_SPL_BUILD) && pll->type == PLL0)
		jh7110_pllx_set_rate(clk, 1000000000);

	if (IS_ENABLED(CONFIG_SPL_BUILD) && pll->type == PLL2)
		jh7110_pllx_set_rate(clk, 1188000000);

	return clk;
}

/* PLLx clock implementation */
U_BOOT_DRIVER(jh7110_clk_pllx) = {
	.name	= UBOOT_DM_CLK_JH7110_PLLX,
	.id	= UCLASS_CLK,
	.ops	= &jh7110_clk_pllx_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

static int jh7110_pll_clk_probe(struct udevice *dev)
{
	void __iomem *reg =  (void __iomem *)dev_read_addr_ptr(dev->parent);
	fdt_addr_t sysreg = ofnode_get_addr(ofnode_by_compatible(ofnode_null(),
					    "starfive,jh7110-syscrg"));

	if (sysreg == FDT_ADDR_T_NONE)
		return -EINVAL;

	clk_dm(JH7110_PLL_ID_TRANS(JH7110_SYSCLK_PLL0_OUT),
	       starfive_jh7110_pll("pll0_out", "oscillator", reg,
				   (void __iomem *)sysreg, &starfive_jh7110_pll0));
	clk_dm(JH7110_PLL_ID_TRANS(JH7110_SYSCLK_PLL1_OUT),
	       starfive_jh7110_pll("pll1_out", "oscillator", reg,
				   (void __iomem *)sysreg, &starfive_jh7110_pll1));
	clk_dm(JH7110_PLL_ID_TRANS(JH7110_SYSCLK_PLL2_OUT),
	       starfive_jh7110_pll("pll2_out", "oscillator", reg,
				   (void __iomem *)sysreg, &starfive_jh7110_pll2));

	return 0;
}

static int jh7110_pll_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = JH7110_PLL_ID_TRANS(args->args[0]);
	else
		clk->id = 0;

	return 0;
}

static const struct udevice_id jh7110_pll_clk_of_match[] = {
	{ .compatible = "starfive,jh7110-pll", },
	{ }
};

JH7110_CLK_OPS(pll);

/* PLL clk device */
U_BOOT_DRIVER(jh7110_pll_clk) = {
	.name	= "jh7110_pll_clk",
	.id	= UCLASS_CLK,
	.of_match	= jh7110_pll_clk_of_match,
	.probe	= jh7110_pll_clk_probe,
	.ops	= &jh7110_pll_clk_ops,
};
