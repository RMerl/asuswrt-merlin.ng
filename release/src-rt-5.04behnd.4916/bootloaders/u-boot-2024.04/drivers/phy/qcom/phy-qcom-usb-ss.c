// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sumit Garg <sumit.garg@linaro.org>
 *
 * Based on Linux driver
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <reset.h>
#include <clk.h>
#include <linux/delay.h>

#define PHY_CTRL0			0x6C
#define PHY_CTRL1			0x70
#define PHY_CTRL2			0x74
#define PHY_CTRL4			0x7C

/* PHY_CTRL bits */
#define REF_PHY_EN			BIT(0)
#define LANE0_PWR_ON			BIT(2)
#define SWI_PCS_CLK_SEL			BIT(4)
#define TST_PWR_DOWN			BIT(4)
#define PHY_RESET			BIT(7)

struct ssphy_priv {
	void __iomem *base;
	struct clk_bulk clks;
	struct reset_ctl com_rst;
	struct reset_ctl phy_rst;
};

static inline void ssphy_updatel(void __iomem *addr, u32 mask, u32 val)
{
	writel((readl(addr) & ~mask) | val, addr);
}

static int ssphy_do_reset(struct ssphy_priv *priv)
{
	int ret;

	ret = reset_assert(&priv->com_rst);
	if (ret)
		return ret;

	ret = reset_assert(&priv->phy_rst);
	if (ret)
		return ret;

	udelay(10);

	ret = reset_deassert(&priv->com_rst);
	if (ret)
		return ret;

	ret = reset_deassert(&priv->phy_rst);
	if (ret)
		return ret;

	return 0;
}

static int ssphy_power_on(struct phy *phy)
{
	struct ssphy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = ssphy_do_reset(priv);
	if (ret)
		return ret;

	writeb(SWI_PCS_CLK_SEL, priv->base + PHY_CTRL0);
	ssphy_updatel(priv->base + PHY_CTRL4, LANE0_PWR_ON, LANE0_PWR_ON);
	ssphy_updatel(priv->base + PHY_CTRL2, REF_PHY_EN, REF_PHY_EN);
	ssphy_updatel(priv->base + PHY_CTRL4, TST_PWR_DOWN, 0);

	return 0;
}

static int ssphy_power_off(struct phy *phy)
{
	struct ssphy_priv *priv = dev_get_priv(phy->dev);

	ssphy_updatel(priv->base + PHY_CTRL4, LANE0_PWR_ON, 0);
	ssphy_updatel(priv->base + PHY_CTRL2, REF_PHY_EN, 0);
	ssphy_updatel(priv->base + PHY_CTRL4, TST_PWR_DOWN, TST_PWR_DOWN);

	return 0;
}

static int ssphy_clk_init(struct udevice *dev, struct ssphy_priv *priv)
{
	int ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret == -ENOSYS || ret == -ENOENT)
		return 0;
	if (ret)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		clk_release_bulk(&priv->clks);
		return ret;
	}

	return 0;
}

static int ssphy_probe(struct udevice *dev)
{
	struct ssphy_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = ssphy_clk_init(dev, priv);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "com", &priv->com_rst);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "phy", &priv->phy_rst);
	if (ret)
		return ret;

	return 0;
}

static struct phy_ops ssphy_ops = {
	.power_on = ssphy_power_on,
	.power_off = ssphy_power_off,
};

static const struct udevice_id ssphy_ids[] = {
	{ .compatible = "qcom,usb-ss-28nm-phy" },
	{ }
};

U_BOOT_DRIVER(qcom_usb_ss) = {
	.name		= "qcom-usb-ss",
	.id		= UCLASS_PHY,
	.of_match	= ssphy_ids,
	.ops		= &ssphy_ops,
	.probe		= ssphy_probe,
	.priv_auto	= sizeof(struct ssphy_priv),
};
