// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner sun50i(H6) USB 3.0 phy driver
 *
 * Copyright (C) 2020 Samuel Holland <samuel@sholland.org>
 *
 * Based on the Linux driver, which is:
 *
 * Copyright (C) 2017 Icenowy Zheng <icenowy@aosc.io>
 *
 * Based on phy-sun9i-usb.c, which is:
 *
 * Copyright (C) 2014-2015 Chen-Yu Tsai <wens@csie.org>
 *
 * Based on code from Allwinner BSP, which is:
 *
 * Copyright (c) 2010-2015 Allwinner Technology Co., Ltd.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <reset.h>

/* Interface Status and Control Registers */
#define SUNXI_ISCR			0x00
#define SUNXI_PIPE_CLOCK_CONTROL	0x14
#define SUNXI_PHY_TUNE_LOW		0x18
#define SUNXI_PHY_TUNE_HIGH		0x1c
#define SUNXI_PHY_EXTERNAL_CONTROL	0x20

/* USB2.0 Interface Status and Control Register */
#define SUNXI_ISCR_FORCE_VBUS		(3 << 12)

/* PIPE Clock Control Register */
#define SUNXI_PCC_PIPE_CLK_OPEN		(1 << 6)

/* PHY External Control Register */
#define SUNXI_PEC_EXTERN_VBUS		(3 << 1)
#define SUNXI_PEC_SSC_EN		(1 << 24)
#define SUNXI_PEC_REF_SSP_EN		(1 << 26)

/* PHY Tune High Register */
#define SUNXI_TX_DEEMPH_3P5DB(n)	((n) << 19)
#define SUNXI_TX_DEEMPH_3P5DB_MASK	GENMASK(24, 19)
#define SUNXI_TX_DEEMPH_6DB(n)		((n) << 13)
#define SUNXI_TX_DEEMPH_6GB_MASK	GENMASK(18, 13)
#define SUNXI_TX_SWING_FULL(n)		((n) << 6)
#define SUNXI_TX_SWING_FULL_MASK	GENMASK(12, 6)
#define SUNXI_LOS_BIAS(n)		((n) << 3)
#define SUNXI_LOS_BIAS_MASK		GENMASK(5, 3)
#define SUNXI_TXVBOOSTLVL(n)		((n) << 0)
#define SUNXI_TXVBOOSTLVL_MASK		GENMASK(2, 0)

struct sun50i_usb3_phy_priv {
	void __iomem *regs;
	struct reset_ctl reset;
	struct clk clk;
};

static void sun50i_usb3_phy_open(struct sun50i_usb3_phy_priv *phy)
{
	u32 val;

	val = readl(phy->regs + SUNXI_PHY_EXTERNAL_CONTROL);
	val |= SUNXI_PEC_EXTERN_VBUS;
	val |= SUNXI_PEC_SSC_EN | SUNXI_PEC_REF_SSP_EN;
	writel(val, phy->regs + SUNXI_PHY_EXTERNAL_CONTROL);

	val = readl(phy->regs + SUNXI_PIPE_CLOCK_CONTROL);
	val |= SUNXI_PCC_PIPE_CLK_OPEN;
	writel(val, phy->regs + SUNXI_PIPE_CLOCK_CONTROL);

	val = readl(phy->regs + SUNXI_ISCR);
	val |= SUNXI_ISCR_FORCE_VBUS;
	writel(val, phy->regs + SUNXI_ISCR);

	/*
	 * All the magic numbers written to the PHY_TUNE_{LOW_HIGH}
	 * registers are directly taken from the BSP USB3 driver from
	 * Allwiner.
	 */
	writel(0x0047fc87, phy->regs + SUNXI_PHY_TUNE_LOW);

	val = readl(phy->regs + SUNXI_PHY_TUNE_HIGH);
	val &= ~(SUNXI_TXVBOOSTLVL_MASK | SUNXI_LOS_BIAS_MASK |
		 SUNXI_TX_SWING_FULL_MASK | SUNXI_TX_DEEMPH_6GB_MASK |
		 SUNXI_TX_DEEMPH_3P5DB_MASK);
	val |= SUNXI_TXVBOOSTLVL(0x7);
	val |= SUNXI_LOS_BIAS(0x7);
	val |= SUNXI_TX_SWING_FULL(0x55);
	val |= SUNXI_TX_DEEMPH_6DB(0x20);
	val |= SUNXI_TX_DEEMPH_3P5DB(0x15);
	writel(val, phy->regs + SUNXI_PHY_TUNE_HIGH);
}

static int sun50i_usb3_phy_init(struct phy *phy)
{
	struct sun50i_usb3_phy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_prepare_enable(&priv->clk);
	if (ret)
		return ret;

	ret = reset_deassert(&priv->reset);
	if (ret) {
		clk_disable_unprepare(&priv->clk);
		return ret;
	}

	sun50i_usb3_phy_open(priv);

	return 0;
}

static int sun50i_usb3_phy_exit(struct phy *phy)
{
	struct sun50i_usb3_phy_priv *priv = dev_get_priv(phy->dev);

	reset_assert(&priv->reset);
	clk_disable_unprepare(&priv->clk);

	return 0;
}

static const struct phy_ops sun50i_usb3_phy_ops = {
	.init		= sun50i_usb3_phy_init,
	.exit		= sun50i_usb3_phy_exit,
};

static int sun50i_usb3_phy_probe(struct udevice *dev)
{
	struct sun50i_usb3_phy_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		dev_err(dev, "failed to get phy clock\n");
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &priv->reset);
	if (ret) {
		dev_err(dev, "failed to get reset control\n");
		return ret;
	}

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static const struct udevice_id sun50i_usb3_phy_ids[] = {
	{ .compatible = "allwinner,sun50i-h6-usb3-phy" },
	{ },
};

U_BOOT_DRIVER(sun50i_usb3_phy) = {
	.name		= "sun50i-usb3-phy",
	.id		= UCLASS_PHY,
	.of_match	= sun50i_usb3_phy_ids,
	.ops		= &sun50i_usb3_phy_ops,
	.probe		= sun50i_usb3_phy_probe,
	.priv_auto	= sizeof(struct sun50i_usb3_phy_priv),
};
