// SPDX-License-Identifier: GPL-2.0+
/*
 * Amlogic AXG MIPI + PCIE analog PHY driver
 *
 * Copyright (C) 2019 Remi Pommarel <repk@triplefau.lt>
 * Copyright (C) 2020 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <bitfield.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <reset.h>
#include <clk.h>
#include <phy-mipi-dphy.h>

#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/bitfield.h>

#define HHI_MIPI_CNTL0 0x00
#define		HHI_MIPI_CNTL0_COMMON_BLOCK	GENMASK(31, 28)
#define		HHI_MIPI_CNTL0_ENABLE		BIT(29)
#define		HHI_MIPI_CNTL0_BANDGAP		BIT(26)
#define		HHI_MIPI_CNTL0_DIF_REF_CTL1	GENMASK(25, 16)
#define		HHI_MIPI_CNTL0_DIF_REF_CTL0	GENMASK(15, 0)

#define HHI_MIPI_CNTL1 0x04
#define		HHI_MIPI_CNTL1_CH0_CML_PDR_EN	BIT(12)
#define		HHI_MIPI_CNTL1_LP_ABILITY	GENMASK(5, 4)
#define		HHI_MIPI_CNTL1_LP_RESISTER	BIT(3)
#define		HHI_MIPI_CNTL1_INPUT_SETTING	BIT(2)
#define		HHI_MIPI_CNTL1_INPUT_SEL	BIT(1)
#define		HHI_MIPI_CNTL1_PRBS7_EN		BIT(0)

#define HHI_MIPI_CNTL2 0x08
#define		HHI_MIPI_CNTL2_CH_PU		GENMASK(31, 25)
#define		HHI_MIPI_CNTL2_CH_CTL		GENMASK(24, 19)
#define		HHI_MIPI_CNTL2_CH0_DIGDR_EN	BIT(18)
#define		HHI_MIPI_CNTL2_CH_DIGDR_EN	BIT(17)
#define		HHI_MIPI_CNTL2_LPULPS_EN	BIT(16)
#define		HHI_MIPI_CNTL2_CH_EN		GENMASK(15, 11)
#define		HHI_MIPI_CNTL2_CH0_LP_CTL	GENMASK(10, 1)

#define DSI_LANE_0              (1 << 4)
#define DSI_LANE_1              (1 << 3)
#define DSI_LANE_CLK            (1 << 2)
#define DSI_LANE_2              (1 << 1)
#define DSI_LANE_3              (1 << 0)
#define DSI_LANE_MASK		(0x1F)

struct phy_meson_axg_mipi_pcie_analog_priv {
	struct regmap *regmap;
	struct phy_configure_opts_mipi_dphy config;
	bool dsi_configured;
	bool dsi_enabled;
	bool powered;
};

static void phy_bandgap_enable(struct phy_meson_axg_mipi_pcie_analog_priv *priv)
{
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_BANDGAP, HHI_MIPI_CNTL0_BANDGAP);

	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_ENABLE, HHI_MIPI_CNTL0_ENABLE);
}

static void phy_bandgap_disable(struct phy_meson_axg_mipi_pcie_analog_priv *priv)
{
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_BANDGAP, 0);
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_ENABLE, 0);
}

static void phy_dsi_analog_enable(struct phy_meson_axg_mipi_pcie_analog_priv *priv)
{
	u32 reg;

	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			   HHI_MIPI_CNTL0_DIF_REF_CTL1,
			   FIELD_PREP(HHI_MIPI_CNTL0_DIF_REF_CTL1, 0x1b8));
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			   BIT(31), BIT(31));
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			   HHI_MIPI_CNTL0_DIF_REF_CTL0,
			   FIELD_PREP(HHI_MIPI_CNTL0_DIF_REF_CTL0, 0x8));

	regmap_write(priv->regmap, HHI_MIPI_CNTL1, 0x001e);

	regmap_write(priv->regmap, HHI_MIPI_CNTL2,
		     (0x26e0 << 16) | (0x459 << 0));

	reg = DSI_LANE_CLK;
	switch (priv->config.lanes) {
	case 4:
		reg |= DSI_LANE_3;
		fallthrough;
	case 3:
		reg |= DSI_LANE_2;
		fallthrough;
	case 2:
		reg |= DSI_LANE_1;
		fallthrough;
	case 1:
		reg |= DSI_LANE_0;
		break;
	default:
		reg = 0;
	}

	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL2,
			   HHI_MIPI_CNTL2_CH_EN,
			   FIELD_PREP(HHI_MIPI_CNTL2_CH_EN, reg));

	priv->dsi_enabled = true;
}

static void phy_dsi_analog_disable(struct phy_meson_axg_mipi_pcie_analog_priv *priv)
{
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_DIF_REF_CTL1,
			FIELD_PREP(HHI_MIPI_CNTL0_DIF_REF_CTL1, 0));
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0, BIT(31), 0);
	regmap_update_bits(priv->regmap, HHI_MIPI_CNTL0,
			HHI_MIPI_CNTL0_DIF_REF_CTL1, 0);

	regmap_write(priv->regmap, HHI_MIPI_CNTL1, 0x6);

	regmap_write(priv->regmap, HHI_MIPI_CNTL2, 0x00200000);

	priv->dsi_enabled = false;
}

static int phy_meson_axg_mipi_pcie_analog_configure(struct phy *phy, void *params)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_axg_mipi_pcie_analog_priv *priv = dev_get_priv(dev);
	struct phy_configure_opts_mipi_dphy *config = params;
	int ret;

	ret = phy_mipi_dphy_config_validate(config);
	if (ret)
		return ret;

	memcpy(&priv->config, config, sizeof(priv->config));

	priv->dsi_configured = true;

	/* If PHY was already powered on, setup the DSI analog part */
	if (priv->powered) {
		/* If reconfiguring, disable & reconfigure */
		if (priv->dsi_enabled)
			phy_dsi_analog_disable(priv);

		udelay(100);

		phy_dsi_analog_enable(priv);
	}

	return 0;
}

static int phy_meson_axg_mipi_pcie_analog_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_axg_mipi_pcie_analog_priv *priv = dev_get_priv(dev);

	phy_bandgap_enable(priv);

	if (priv->dsi_configured)
		phy_dsi_analog_enable(priv);

	priv->powered = true;

	return 0;
}

static int phy_meson_axg_mipi_pcie_analog_power_off(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_axg_mipi_pcie_analog_priv *priv = dev_get_priv(dev);

	phy_bandgap_disable(priv);

	if (priv->dsi_enabled)
		phy_dsi_analog_disable(priv);

	priv->powered = false;

	return 0;
}

struct phy_ops meson_axg_mipi_pcie_analog_ops = {
	.power_on = phy_meson_axg_mipi_pcie_analog_power_on,
	.power_off = phy_meson_axg_mipi_pcie_analog_power_off,
	.configure = phy_meson_axg_mipi_pcie_analog_configure,
};

int meson_axg_mipi_pcie_analog_probe(struct udevice *dev)
{
	struct phy_meson_axg_mipi_pcie_analog_priv *priv = dev_get_priv(dev);

	priv->regmap = syscon_node_to_regmap(dev_ofnode(dev_get_parent(dev)));
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	return 0;
}

static const struct udevice_id meson_axg_mipi_pcie_analog_ids[] = {
	{ .compatible = "amlogic,axg-mipi-pcie-analog-phy" },
	{ }
};

U_BOOT_DRIVER(meson_axg_mipi_pcie_analog) = {
	.name = "meson_axg_mipi_pcie_analog",
	.id = UCLASS_PHY,
	.of_match = meson_axg_mipi_pcie_analog_ids,
	.probe = meson_axg_mipi_pcie_analog_probe,
	.ops = &meson_axg_mipi_pcie_analog_ops,
	.priv_auto = sizeof(struct phy_meson_axg_mipi_pcie_analog_priv),
};
