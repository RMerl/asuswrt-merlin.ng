// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>

#define NPCM_TIMER_CLOCK_RATE	1000000UL		/* 1MHz timer */
#define NPCM_TIMER_INPUT_RATE	25000000UL		/* Rate of input clock */
#define NPCM_TIMER_TDR_MASK	GENMASK(23, 0)
#define NPCM_TIMER_MAX_VAL	NPCM_TIMER_TDR_MASK	/* max counter value */

/* Register offsets */
#define TCR0	0x0	/* Timer Control and Status Register */
#define TICR0	0x8	/* Timer Initial Count Register */
#define TDR0	0x10	/* Timer Data Register */

/* TCR fields */
#define TCR_MODE_PERIODIC	BIT(27)
#define TCR_EN			BIT(30)
#define TCR_PRESCALE		(NPCM_TIMER_INPUT_RATE / NPCM_TIMER_CLOCK_RATE - 1)

enum input_clock_type {
	INPUT_CLOCK_FIXED,	/* input clock rate is fixed */
	INPUT_CLOCK_NON_FIXED
};

/**
 * struct npcm_timer_priv - private data for npcm timer driver
 * npcm timer is a 24-bits down-counting timer.
 *
 * @last_count: last hw counter value
 * @counter: the value to be returned for get_count ops
 */
struct npcm_timer_priv {
	void __iomem *base;
	u32 last_count;
	u64 counter;
};

static u64 npcm_timer_get_count(struct udevice *dev)
{
	struct npcm_timer_priv *priv = dev_get_priv(dev);
	u32 val;

	/* The timer is counting down */
	val = readl(priv->base + TDR0) & NPCM_TIMER_TDR_MASK;
	if (val <= priv->last_count)
		priv->counter += priv->last_count - val;
	else
		priv->counter += priv->last_count + (NPCM_TIMER_MAX_VAL + 1 - val);
	priv->last_count = val;

	return priv->counter;
}

static int npcm_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct npcm_timer_priv *priv = dev_get_priv(dev);
	enum input_clock_type type = dev_get_driver_data(dev);
	struct clk clk;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;
	uc_priv->clock_rate = NPCM_TIMER_CLOCK_RATE;

	if (type == INPUT_CLOCK_NON_FIXED) {
		ret = clk_get_by_index(dev, 0, &clk);
		if (ret < 0)
			return ret;

		ret = clk_set_rate(&clk, NPCM_TIMER_INPUT_RATE);
		if (ret < 0)
			return ret;
	}

	/*
	 * Configure timer and start
	 * periodic mode
	 * timer clock rate = input clock / prescale
	 */
	writel(0, priv->base + TCR0);
	writel(NPCM_TIMER_MAX_VAL, priv->base + TICR0);
	writel(TCR_EN | TCR_MODE_PERIODIC | TCR_PRESCALE,
	       priv->base + TCR0);

	return 0;
}

static const struct timer_ops npcm_timer_ops = {
	.get_count = npcm_timer_get_count,
};

static const struct udevice_id npcm_timer_ids[] = {
	{ .compatible = "nuvoton,npcm845-timer", .data = INPUT_CLOCK_FIXED},
	{ .compatible = "nuvoton,npcm750-timer", .data = INPUT_CLOCK_NON_FIXED},
	{}
};

U_BOOT_DRIVER(npcm_timer) = {
	.name	= "npcm_timer",
	.id	= UCLASS_TIMER,
	.of_match = npcm_timer_ids,
	.priv_auto = sizeof(struct npcm_timer_priv),
	.probe = npcm_timer_probe,
	.ops	= &npcm_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
