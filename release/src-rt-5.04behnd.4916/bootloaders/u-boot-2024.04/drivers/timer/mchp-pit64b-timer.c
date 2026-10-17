// SPDX-License-Identifier: GPL-2.0+
/*
 * 64-bit Periodic Interval Timer driver
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>

#define MCHP_PIT64B_CR			0x00	/* Control Register */
#define		MCHP_PIT64B_CR_START	BIT(0)
#define		MCHP_PIT64B_CR_SWRST	BIT(8)
#define MCHP_PIT64B_MR			0x04	/* Mode Register */
#define		MCHP_PIT64B_MR_CONT	BIT(0)
#define MCHP_PIT64B_LSB_PR		0x08	/* LSB Period Register */
#define MCHP_PIT64B_MSB_PR		0x0C	/* MSB Period Register */
#define MCHP_PIT64B_TLSBR		0x20	/* Timer LSB Register */
#define MCHP_PIT64B_TMSBR		0x24	/* Timer MSB Register */

struct mchp_pit64b_priv {
	void __iomem *base;
};

static u64 mchp_pit64b_get_count(struct udevice *dev)
{
	struct mchp_pit64b_priv *priv = dev_get_priv(dev);

	u32 lsb = readl(priv->base + MCHP_PIT64B_TLSBR);
	u32 msb = readl(priv->base + MCHP_PIT64B_TMSBR);

	return ((u64)msb << 32) | lsb;
}

static int mchp_pit64b_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mchp_pit64b_priv *priv = dev_get_priv(dev);
	struct clk clk;
	ulong rate;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	rate = clk_get_rate(&clk);
	if (!rate) {
		clk_disable(&clk);
		return -ENOTSUPP;
	}

	/* Reset the timer in case it was used by previous bootloaders. */
	writel(MCHP_PIT64B_CR_SWRST, priv->base + MCHP_PIT64B_CR);

	/*
	 * Use highest prescaller (for a peripheral clock running at 200MHz
	 * this will lead to the timer running at 12.5MHz) and continuous mode.
	 */
	writel((15 << 8) | MCHP_PIT64B_MR_CONT, priv->base + MCHP_PIT64B_MR);
	uc_priv->clock_rate = rate / 16;

	/*
	 * Simulate free running counter by setting max values to period
	 * registers.
	 */
	writel(~0UL, priv->base + MCHP_PIT64B_MSB_PR);
	writel(~0UL, priv->base + MCHP_PIT64B_LSB_PR);

	/* Start the timer. */
	writel(MCHP_PIT64B_CR_START, priv->base + MCHP_PIT64B_CR);

	return 0;
}

static const struct timer_ops mchp_pit64b_ops = {
	.get_count = mchp_pit64b_get_count,
};

static const struct udevice_id mchp_pit64b_ids[] = {
	{ .compatible = "microchip,sam9x60-pit64b", },
	{ .compatible = "microchip,sama7g5-pit64b", },
	{ }
};

U_BOOT_DRIVER(mchp_pit64b) = {
	.name	= "mchp-pit64b",
	.id	= UCLASS_TIMER,
	.of_match = mchp_pit64b_ids,
	.priv_auto	= sizeof(struct mchp_pit64b_priv),
	.probe	= mchp_pit64b_probe,
	.ops	= &mchp_pit64b_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
