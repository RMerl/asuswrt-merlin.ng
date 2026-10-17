// SPDX-License-Identifier: GPL-2.0+
/*
 * phy_uniphier_usb3.c - Socionext UniPhier Usb3 PHY driver
 * Copyright 2019-2023 Socionext, Inc.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>

#include <clk.h>
#include <reset.h>

struct uniphier_usb3phy_priv {
	struct clk *clk_link, *clk_phy, *clk_parent, *clk_phyext;
	struct reset_ctl *rst_link, *rst_phy, *rst_parent;
};

static int uniphier_usb3phy_init(struct phy *phy)
{
	struct uniphier_usb3phy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable(priv->clk_phy);
	if (ret)
		return ret;

	ret = reset_deassert(priv->rst_phy);
	if (ret)
		goto out_clk;

	if (priv->clk_phyext) {
		ret = clk_enable(priv->clk_phyext);
		if (ret)
			goto out_rst;
	}

	return 0;

out_rst:
	reset_assert(priv->rst_phy);
out_clk:
	clk_disable(priv->clk_phy);

	return ret;
}

static int uniphier_usb3phy_exit(struct phy *phy)
{
	struct uniphier_usb3phy_priv *priv = dev_get_priv(phy->dev);

	if (priv->clk_phyext)
		clk_disable(priv->clk_phyext);

	reset_assert(priv->rst_phy);
	clk_disable(priv->clk_phy);

	return 0;
}

static int uniphier_usb3phy_probe(struct udevice *dev)
{
	struct uniphier_usb3phy_priv *priv = dev_get_priv(dev);
	int ret;

	priv->clk_link = devm_clk_get(dev, "link");
	if (IS_ERR(priv->clk_link)) {
		printf("Failed to get link clock\n");
		return PTR_ERR(priv->clk_link);
	}

	priv->clk_phy = devm_clk_get(dev, "phy");
	if (IS_ERR(priv->clk_link)) {
		printf("Failed to get phy clock\n");
		return PTR_ERR(priv->clk_link);
	}

	priv->clk_parent = devm_clk_get_optional(dev, "gio");
	if (IS_ERR(priv->clk_parent)) {
		printf("Failed to get parent clock\n");
		return PTR_ERR(priv->clk_parent);
	}

	priv->clk_phyext = devm_clk_get_optional(dev, "phy-ext");
	if (IS_ERR(priv->clk_phyext)) {
		printf("Failed to get external phy clock\n");
		return PTR_ERR(priv->clk_phyext);
	}

	priv->rst_link = devm_reset_control_get(dev, "link");
	if (IS_ERR(priv->rst_link)) {
		printf("Failed to get link reset\n");
		return PTR_ERR(priv->rst_link);
	}

	priv->rst_phy = devm_reset_control_get(dev, "phy");
	if (IS_ERR(priv->rst_phy)) {
		printf("Failed to get phy reset\n");
		return PTR_ERR(priv->rst_phy);
	}

	priv->rst_parent = devm_reset_control_get_optional(dev, "gio");
	if (IS_ERR(priv->rst_parent)) {
		printf("Failed to get parent reset\n");
		return PTR_ERR(priv->rst_parent);
	}

	if (priv->clk_parent) {
		ret = clk_enable(priv->clk_parent);
		if (ret)
			return ret;
	}
	if (priv->rst_parent) {
		ret = reset_deassert(priv->rst_parent);
		if (ret)
			goto out_clk_parent;
	}

	ret = clk_enable(priv->clk_link);
	if (ret)
		goto out_rst_parent;

	ret = reset_deassert(priv->rst_link);
	if (ret)
		goto out_clk;

	return 0;

out_clk:
	clk_disable(priv->clk_link);
out_rst_parent:
	if (priv->rst_parent)
		reset_assert(priv->rst_parent);
out_clk_parent:
	if (priv->clk_parent)
		clk_disable(priv->clk_parent);

	return ret;
}

static struct phy_ops uniphier_usb3phy_ops = {
	.init = uniphier_usb3phy_init,
	.exit = uniphier_usb3phy_exit,
};

static const struct udevice_id uniphier_usb3phy_ids[] = {
	{ .compatible = "socionext,uniphier-pro4-usb3-ssphy" },
	{ .compatible = "socionext,uniphier-pro5-usb3-hsphy" },
	{ .compatible = "socionext,uniphier-pro5-usb3-ssphy" },
	{ .compatible = "socionext,uniphier-pxs2-usb3-hsphy" },
	{ .compatible = "socionext,uniphier-pxs2-usb3-ssphy" },
	{ .compatible = "socionext,uniphier-ld20-usb3-hsphy" },
	{ .compatible = "socionext,uniphier-ld20-usb3-ssphy" },
	{ .compatible = "socionext,uniphier-pxs3-usb3-hsphy" },
	{ .compatible = "socionext,uniphier-pxs3-usb3-ssphy" },
	{ .compatible = "socionext,uniphier-nx1-usb3-hsphy" },
	{ .compatible = "socionext,uniphier-nx1-usb3-ssphy" },
	{ }
};

U_BOOT_DRIVER(uniphier_usb3_phy) = {
	.name		= "uniphier-usb3-phy",
	.id		= UCLASS_PHY,
	.of_match	= uniphier_usb3phy_ids,
	.ops		= &uniphier_usb3phy_ops,
	.probe		= uniphier_usb3phy_probe,
	.priv_auto      = sizeof(struct uniphier_usb3phy_priv),
};
