// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments K3 SoC PLL clock driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Tero Kristo <t-kristo@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <div64.h>
#include <errno.h>
#include <clk-uclass.h>
#include <linux/clk-provider.h>
#include "k3-clk.h"
#include <linux/rational.h>

/* 16FFT register offsets */
#define PLL_16FFT_CFG			0x08
#define PLL_KICK0			0x10
#define PLL_KICK1			0x14
#define PLL_16FFT_CTRL			0x20
#define PLL_16FFT_STAT			0x24
#define PLL_16FFT_FREQ_CTRL0		0x30
#define PLL_16FFT_FREQ_CTRL1		0x34
#define PLL_16FFT_DIV_CTRL		0x38
#define PLL_16FFT_CAL_CTRL		0x60
#define PLL_16FFT_CAL_STAT		0x64

/* CAL STAT register bits */
#define PLL_16FFT_CAL_STAT_CAL_LOCK	BIT(31)

/* CFG register bits */
#define PLL_16FFT_CFG_PLL_TYPE_SHIFT	(0)
#define PLL_16FFT_CFG_PLL_TYPE_MASK	(0x3 << 0)
#define PLL_16FFT_CFG_PLL_TYPE_FRACF	1

/* CAL CTRL register bits */
#define PLL_16FFT_CAL_CTRL_CAL_EN               BIT(31)
#define PLL_16FFT_CAL_CTRL_FAST_CAL             BIT(20)
#define PLL_16FFT_CAL_CTRL_CAL_BYP              BIT(15)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT        16
#define PLL_16FFT_CAL_CTRL_CAL_CNT_MASK         (0x7 << 16)

/* CTRL register bits */
#define PLL_16FFT_CTRL_BYPASS_EN	BIT(31)
#define PLL_16FFT_CTRL_PLL_EN		BIT(15)
#define PLL_16FFT_CTRL_DSM_EN		BIT(1)

/* STAT register bits */
#define PLL_16FFT_STAT_LOCK		BIT(0)

/* FREQ_CTRL0 bits */
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK	0xfff

/* DIV CTRL register bits */
#define PLL_16FFT_DIV_CTRL_REF_DIV_MASK		0x3f

/* HSDIV register bits*/
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN          BIT(15)

/* FREQ_CTRL1 bits */
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS	24
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK	0xffffff
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT	0

/* KICK register magic values */
#define PLL_KICK0_VALUE				0x68ef3490
#define PLL_KICK1_VALUE				0xd172bc5a

/**
 * struct ti_pll_clk - TI PLL clock data info structure
 * @clk: core clock structure
 * @reg: memory address of the PLL controller
 */
struct ti_pll_clk {
	struct clk	clk;
	void __iomem	*reg;
};

#define to_clk_pll(_clk) container_of(_clk, struct ti_pll_clk, clk)

static int ti_pll_wait_for_lock(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 stat;
	u32 cfg;
	u32 cal;
	u32 freq_ctrl1;
	int i;
	u32 pllfm;
	u32 pll_type;
	int success;

	for (i = 0; i < 100000; i++) {
		stat = readl(pll->reg + PLL_16FFT_STAT);
		if (stat & PLL_16FFT_STAT_LOCK) {
			success = 1;
			break;
		}
	}

	/* Enable calibration if not in fractional mode of the FRACF PLL */
	freq_ctrl1 = readl(pll->reg + PLL_16FFT_FREQ_CTRL1);
	pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
	pllfm >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;
	cfg = readl(pll->reg + PLL_16FFT_CFG);
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	if (success && pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF && pllfm == 0) {
		cal = readl(pll->reg + PLL_16FFT_CAL_CTRL);

		/* Enable calibration for FRACF */
		cal |= PLL_16FFT_CAL_CTRL_CAL_EN;

		/* Enable fast cal mode */
		cal |= PLL_16FFT_CAL_CTRL_FAST_CAL;

		/* Disable calibration bypass */
		cal &= ~PLL_16FFT_CAL_CTRL_CAL_BYP;

		/* Set CALCNT to 2 */
		cal &= ~PLL_16FFT_CAL_CTRL_CAL_CNT_MASK;
		cal |= 2 << PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT;

		/* Note this register does not readback the written value. */
		writel(cal, pll->reg + PLL_16FFT_CAL_CTRL);

		success = 0;
		for (i = 0; i < 100000; i++) {
			stat = readl(pll->reg + PLL_16FFT_CAL_STAT);
			if (stat & PLL_16FFT_CAL_STAT_CAL_LOCK) {
				success = 1;
				break;
			}
		}
	}

	if (success == 0) {
		printf("%s: pll (%s) failed to lock\n", __func__,
		       clk->dev->name);
		return -EBUSY;
	} else {
		return 0;
	}
}

static ulong ti_pll_clk_get_rate(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u64 current_freq;
	u64 parent_freq = clk_get_parent_rate(clk);
	u32 pllm;
	u32 plld;
	u32 pllfm;
	u32 ctrl;

	/* Check if we are in bypass */
	ctrl = readl(pll->reg + PLL_16FFT_CTRL);
	if (ctrl & PLL_16FFT_CTRL_BYPASS_EN)
		return parent_freq;

	pllm = readl(pll->reg + PLL_16FFT_FREQ_CTRL0);
	pllfm = readl(pll->reg + PLL_16FFT_FREQ_CTRL1);

	plld = readl(pll->reg + PLL_16FFT_DIV_CTRL) &
		PLL_16FFT_DIV_CTRL_REF_DIV_MASK;

	current_freq = parent_freq * pllm / plld;

	if (pllfm) {
		u64 tmp;

		tmp = parent_freq * pllfm;
		do_div(tmp, plld);
		tmp >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		current_freq += tmp;
	}

	return current_freq;
}

static ulong ti_pll_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u64 current_freq;
	u64 parent_freq = clk_get_parent_rate(clk);
	int ret;
	u32 ctrl;
	unsigned long pllm;
	u32 pllfm = 0;
	unsigned long plld;
	u32 div_ctrl;
	u32 rem;
	int shift;

	debug("%s(clk=%p, rate=%u)\n", __func__, clk, (u32)rate);

	if (ti_pll_clk_get_rate(clk) == rate)
		return rate;

	if (rate != parent_freq)
		/*
		 * Attempt with higher max multiplier value first to give
		 * some space for fractional divider to kick in.
		 */
		for (shift = 8; shift >= 0; shift -= 8) {
			rational_best_approximation(rate, parent_freq,
				((PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK + 1) << shift) - 1,
				PLL_16FFT_DIV_CTRL_REF_DIV_MASK, &pllm, &plld);
			if (pllm / plld <= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK)
				break;
		}

	/* Put PLL to bypass mode */
	ctrl = readl(pll->reg + PLL_16FFT_CTRL);
	ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	writel(ctrl, pll->reg + PLL_16FFT_CTRL);

	if (rate == parent_freq) {
		debug("%s: put %s to bypass\n", __func__, clk->dev->name);
		return rate;
	}

	debug("%s: pre-frac-calc: rate=%u, parent_freq=%u, plld=%u, pllm=%u\n",
	      __func__, (u32)rate, (u32)parent_freq, (u32)plld, (u32)pllm);

	/* Check if we need fractional config */
	if (plld > 1) {
		pllfm = pllm % plld;
		pllfm <<= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		rem = pllfm % plld;
		pllfm /= plld;
		if (rem)
			pllfm++;
		pllm /= plld;
		plld = 1;
	}

	if (pllfm)
		ctrl |= PLL_16FFT_CTRL_DSM_EN;
	else
		ctrl &= ~PLL_16FFT_CTRL_DSM_EN;

	writel(pllm, pll->reg + PLL_16FFT_FREQ_CTRL0);
	writel(pllfm, pll->reg + PLL_16FFT_FREQ_CTRL1);

	/*
	 * div_ctrl register contains other divider values, so rmw
	 * only plld and leave existing values alone
	 */
	div_ctrl = readl(pll->reg + PLL_16FFT_DIV_CTRL);
	div_ctrl &= ~PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	div_ctrl |= plld;
	writel(div_ctrl, pll->reg + PLL_16FFT_DIV_CTRL);

	ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;
	ctrl |= PLL_16FFT_CTRL_PLL_EN;
	writel(ctrl, pll->reg + PLL_16FFT_CTRL);

	ret = ti_pll_wait_for_lock(clk);
	if (ret)
		return ret;

	debug("%s: pllm=%u, plld=%u, pllfm=%u, parent_freq=%u\n",
	      __func__, (u32)pllm, (u32)plld, (u32)pllfm, (u32)parent_freq);

	current_freq = parent_freq * pllm / plld;

	if (pllfm) {
		u64 tmp;

		tmp = parent_freq * pllfm;
		do_div(tmp, plld);
		tmp >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		current_freq += tmp;
	}

	return current_freq;
}

static int ti_pll_clk_enable(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 ctrl;

	ctrl = readl(pll->reg + PLL_16FFT_CTRL);
	ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;
	ctrl |= PLL_16FFT_CTRL_PLL_EN;
	writel(ctrl, pll->reg + PLL_16FFT_CTRL);

	return ti_pll_wait_for_lock(clk);
}

static int ti_pll_clk_disable(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 ctrl;

	ctrl = readl(pll->reg + PLL_16FFT_CTRL);
	ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	writel(ctrl, pll->reg + PLL_16FFT_CTRL);

	return 0;
}

static const struct clk_ops ti_pll_clk_ops = {
	.get_rate = ti_pll_clk_get_rate,
	.set_rate = ti_pll_clk_set_rate,
	.enable = ti_pll_clk_enable,
	.disable = ti_pll_clk_disable,
};

struct clk *clk_register_ti_pll(const char *name, const char *parent_name,
				void __iomem *reg)
{
	struct ti_pll_clk *pll;
	int ret;
	int i;
	u32 cfg, ctrl, hsdiv_presence_bit, hsdiv_ctrl_offs;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->reg = reg;

	ret = clk_register(&pll->clk, "ti-pll-clk", name, parent_name);
	if (ret) {
		printf("%s: failed to register: %d\n", __func__, ret);
		kfree(pll);
		return ERR_PTR(ret);
	}

	/* Unlock the PLL registers */
	writel(PLL_KICK0_VALUE, pll->reg + PLL_KICK0);
	writel(PLL_KICK1_VALUE, pll->reg + PLL_KICK1);

	/* Enable all HSDIV outputs */
	cfg = readl(pll->reg + PLL_16FFT_CFG);
	for (i = 0; i < 16; i++) {
		hsdiv_presence_bit = BIT(16 + i);
		hsdiv_ctrl_offs = 0x80 + (i * 4);
		/* Enable HSDIV output if present */
		if ((hsdiv_presence_bit & cfg) != 0UL) {
			ctrl = readl(pll->reg + hsdiv_ctrl_offs);
			ctrl |= PLL_16FFT_HSDIV_CTRL_CLKOUT_EN;
			writel(ctrl, pll->reg + hsdiv_ctrl_offs);
		}
	}

	return &pll->clk;
}

U_BOOT_DRIVER(ti_pll_clk) = {
	.name = "ti-pll-clk",
	.id = UCLASS_CLK,
	.ops = &ti_pll_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
