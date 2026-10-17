// SPDX-License-Identifier: GPL-2.0+
/*
 * phy_uniphier_pcie.c - Socionext UniPhier PCIe PHY driver
 * Copyright 2019-2021 Socionext, Inc.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <regmap.h>
#include <syscon.h>

/* SG */
#define SG_USBPCIESEL		0x590
#define SG_USBPCIESEL_PCIE	BIT(0)

struct uniphier_pciephy_priv {
	int dummy;
};

static int uniphier_pciephy_init(struct phy *phy)
{
	return 0;
}

static int uniphier_pciephy_probe(struct udevice *dev)
{
	struct regmap *regmap;

	regmap = syscon_regmap_lookup_by_phandle(dev,
						 "socionext,syscon");
	if (!IS_ERR(regmap))
		regmap_update_bits(regmap, SG_USBPCIESEL,
				   SG_USBPCIESEL_PCIE, SG_USBPCIESEL_PCIE);

	return 0;
}

static struct phy_ops uniphier_pciephy_ops = {
	.init = uniphier_pciephy_init,
};

static const struct udevice_id uniphier_pciephy_ids[] = {
	{ .compatible = "socionext,uniphier-pro5-pcie-phy" },
	{ .compatible = "socionext,uniphier-ld20-pcie-phy" },
	{ .compatible = "socionext,uniphier-pxs3-pcie-phy" },
	{ }
};

U_BOOT_DRIVER(uniphier_pcie_phy) = {
	.name		= "uniphier-pcie-phy",
	.id		= UCLASS_PHY,
	.of_match	= uniphier_pciephy_ids,
	.ops		= &uniphier_pciephy_ops,
	.probe		= uniphier_pciephy_probe,
	.priv_auto      = sizeof(struct uniphier_pciephy_priv),
};
