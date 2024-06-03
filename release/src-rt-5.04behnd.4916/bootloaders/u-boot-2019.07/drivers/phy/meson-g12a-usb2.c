// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson G12A USB2 PHY driver
 *
 * Copyright (C) 2017 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstron@baylibre.com>
 */

#include <common.h>
#include <asm/io.h>
#include <bitfield.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <power/regulator.h>
#include <reset.h>
#include <clk.h>

#include <linux/bitops.h>
#include <linux/compat.h>

#define PHY_CTRL_R0						0x0
#define PHY_CTRL_R1						0x4
#define PHY_CTRL_R2						0x8
#define PHY_CTRL_R3						0xc
#define PHY_CTRL_R4						0x10
#define PHY_CTRL_R5						0x14
#define PHY_CTRL_R6						0x18
#define PHY_CTRL_R7						0x1c
#define PHY_CTRL_R8						0x20
#define PHY_CTRL_R9						0x24
#define PHY_CTRL_R10						0x28
#define PHY_CTRL_R11						0x2c
#define PHY_CTRL_R12						0x30
#define PHY_CTRL_R13						0x34
#define PHY_CTRL_R14						0x38
#define PHY_CTRL_R15						0x3c
#define PHY_CTRL_R16						0x40
#define PHY_CTRL_R17						0x44
#define PHY_CTRL_R18						0x48
#define PHY_CTRL_R19						0x4c
#define PHY_CTRL_R20						0x50
#define PHY_CTRL_R21						0x54
#define PHY_CTRL_R22						0x58
#define PHY_CTRL_R23						0x5c

#define RESET_COMPLETE_TIME					1000
#define PLL_RESET_COMPLETE_TIME					100

struct phy_meson_g12a_usb2_priv {
	struct regmap		*regmap;
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	struct udevice		*phy_supply;
#endif
#if CONFIG_IS_ENABLED(CLK)
	struct clk		clk;
#endif
	struct reset_ctl	reset;
};


static int phy_meson_g12a_usb2_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->phy_supply) {
		int ret = regulator_set_enable(priv->phy_supply, true);
		if (ret)
			return ret;
	}
#endif

	return 0;
}

static int phy_meson_g12a_usb2_power_off(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->phy_supply) {
		int ret = regulator_set_enable(priv->phy_supply, false);
		if (ret) {
			pr_err("Error disabling PHY supply\n");
			return ret;
		}
	}
#endif

	return 0;
}

static int phy_meson_g12a_usb2_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = reset_assert(&priv->reset);
	udelay(1);
	ret |= reset_deassert(&priv->reset);
	if (ret)
		return ret;

	udelay(RESET_COMPLETE_TIME);

	/* usb2_otg_aca_en == 0 */
	regmap_update_bits(priv->regmap, PHY_CTRL_R21, BIT(2), 0);

	/* PLL Setup : 24MHz * 20 / 1 = 480MHz */
	regmap_write(priv->regmap, PHY_CTRL_R16, 0x39400414);
	regmap_write(priv->regmap, PHY_CTRL_R17, 0x927e0000);
	regmap_write(priv->regmap, PHY_CTRL_R18, 0xac5f49e5);

	udelay(PLL_RESET_COMPLETE_TIME);

	/* UnReset PLL */
	regmap_write(priv->regmap, PHY_CTRL_R16, 0x19400414);

	/* PHY Tuning */
	regmap_write(priv->regmap, PHY_CTRL_R20, 0xfe18);
	regmap_write(priv->regmap, PHY_CTRL_R4, 0x8000fff);

	/* Tuning Disconnect Threshold */
	regmap_write(priv->regmap, PHY_CTRL_R3, 0x34);

	/* Analog Settings */
	regmap_write(priv->regmap, PHY_CTRL_R14, 0);
	regmap_write(priv->regmap, PHY_CTRL_R13, 0x78000);

	return 0;
}

static int phy_meson_g12a_usb2_exit(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = reset_assert(&priv->reset);
	if (ret)
		return ret;

	return 0;
}

struct phy_ops meson_g12a_usb2_phy_ops = {
	.init = phy_meson_g12a_usb2_init,
	.exit = phy_meson_g12a_usb2_exit,
	.power_on = phy_meson_g12a_usb2_power_on,
	.power_off = phy_meson_g12a_usb2_power_off,
};

int meson_g12a_usb2_phy_probe(struct udevice *dev)
{
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	ret = reset_get_by_index(dev, 0, &priv->reset);
	if (ret == -ENOTSUPP)
		return 0;
	else if (ret)
		return ret;

	ret = reset_deassert(&priv->reset);
	if (ret) {
		reset_release_all(&priv->reset, 1);
		return ret;
	}

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
		pr_err("failed to enable PHY clock\n");
		clk_free(&priv->clk);
		return ret;
	}
#endif

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "phy-supply", &priv->phy_supply);
	if (ret && ret != -ENOENT) {
		pr_err("Failed to get PHY regulator\n");
		return ret;
	}
#endif

	return 0;
}

static const struct udevice_id meson_g12a_usb2_phy_ids[] = {
	{ .compatible = "amlogic,g12a-usb2-phy" },
	{ }
};

U_BOOT_DRIVER(meson_g12a_usb2_phy) = {
	.name = "meson_g12a_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = meson_g12a_usb2_phy_ids,
	.probe = meson_g12a_usb2_phy_probe,
	.ops = &meson_g12a_usb2_phy_ops,
	.priv_auto_alloc_size = sizeof(struct phy_meson_g12a_usb2_priv),
};
