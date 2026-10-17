// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <timer.h>
#include <dm/device_compat.h>

#include <asm/io.h>

#define GPT_CR_EN			BIT(0)
#define GPT_CR_FRR			BIT(9)
#define GPT_CR_EN_24M			BIT(10)
#define GPT_CR_SWR			BIT(15)

#define GPT_PR_PRESCALER24M_MASK	0x0000F000
#define GPT_PR_PRESCALER24M_SHIFT	12
#define GPT_PR_PRESCALER24M_MAX	(GPT_PR_PRESCALER24M_MASK >> GPT_PR_PRESCALER24M_SHIFT)
#define GPT_PR_PRESCALER_MASK		0x00000FFF
#define GPT_PR_PRESCALER_SHIFT		0
#define GPT_PR_PRESCALER_MAX		(GPT_PR_PRESCALER_MASK >> GPT_PR_PRESCALER_SHIFT)

#define GPT_CLKSRC_IPG_CLK		(1 << 6)
#define GPT_CLKSRC_IPG_CLK_24M		(5 << 6)

/* If CFG_SYS_HZ_CLOCK not specified et's default to 3Mhz */
#ifndef CFG_SYS_HZ_CLOCK
#define CFG_SYS_HZ_CLOCK		3000000
#endif

struct imx_gpt_timer_regs {
	u32 cr;
	u32 pr;
	u32 sr;
	u32 ir;
	u32 ocr1;
	u32 ocr2;
	u32 ocr3;
	u32 icr1;
	u32 icr2;
	u32 cnt;
};

struct imx_gpt_timer_priv {
	struct imx_gpt_timer_regs *base;
};

static u64 imx_gpt_timer_get_count(struct udevice *dev)
{
	struct imx_gpt_timer_priv *priv = dev_get_priv(dev);
	struct imx_gpt_timer_regs *regs = priv->base;

	return timer_conv_64(readl(&regs->cnt));
}

static int imx_gpt_setup(struct imx_gpt_timer_regs *regs, u32 rate)
{
	u32 prescaler = (rate / CFG_SYS_HZ_CLOCK) - 1;

	/* Reset the timer */
	setbits_le32(&regs->cr, GPT_CR_SWR);

	/* Wait for timer to finish reset */
	while (readl(&regs->cr) & GPT_CR_SWR)
		;

	if (rate == 24000000UL) {
		/* Set timer frequency if using 24M clock source */
		if (prescaler > GPT_PR_PRESCALER24M_MAX)
			return -EINVAL;

		/* Set 24M prescaler */
		writel((prescaler << GPT_PR_PRESCALER24M_SHIFT), &regs->pr);
		/* Set Oscillator as clock source, enable 24M input and set gpt
		 * in free-running mode
		 */
		writel(GPT_CLKSRC_IPG_CLK_24M | GPT_CR_EN_24M | GPT_CR_FRR, &regs->cr);
	} else {
		if (prescaler > GPT_PR_PRESCALER_MAX)
			return -EINVAL;

		/* Set prescaler */
		writel((prescaler << GPT_PR_PRESCALER_SHIFT), &regs->pr);
		/* Set Peripheral as clock source and set gpt in free-running
		 * mode
		 */
		writel(GPT_CLKSRC_IPG_CLK | GPT_CR_FRR, &regs->cr);
	}

	/* Start timer */
	setbits_le32(&regs->cr, GPT_CR_EN);

	return 0;
}

static int imx_gpt_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct imx_gpt_timer_priv *priv = dev_get_priv(dev);
	struct imx_gpt_timer_regs *regs;
	struct clk clk;
	fdt_addr_t addr;
	u32 clk_rate;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct imx_gpt_timer_regs *)addr;
	regs = priv->base;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "Failed to enable clock\n");
		return ret;
	}

	/* Get timer clock rate */
	clk_rate = clk_get_rate(&clk);
	if (clk_rate <= 0) {
		dev_err(dev, "Could not get clock rate...\n");
		return -EINVAL;
	}

	ret = imx_gpt_setup(regs, clk_rate);
	if (ret) {
		dev_err(dev, "Could not setup timer\n");
		return ret;
	}

	uc_priv->clock_rate = CFG_SYS_HZ_CLOCK;

	return 0;
}

static const struct timer_ops imx_gpt_timer_ops = {
	.get_count = imx_gpt_timer_get_count,
};

static const struct udevice_id imx_gpt_timer_ids[] = {
	{ .compatible = "fsl,imxrt-gpt" },
	{}
};

U_BOOT_DRIVER(imx_gpt_timer) = {
	.name = "imx_gpt_timer",
	.id = UCLASS_TIMER,
	.of_match = imx_gpt_timer_ids,
	.priv_auto = sizeof(struct imx_gpt_timer_priv),
	.probe = imx_gpt_timer_probe,
	.ops = &imx_gpt_timer_ops,
};
