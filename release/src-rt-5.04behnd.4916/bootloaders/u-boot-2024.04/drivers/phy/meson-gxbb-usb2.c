// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson8, Meson8b and GXBB USB2 PHY driver
 *
 * Copyright (C) 2016 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2018 BayLibre, SAS
 *
 * Author: Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <reset.h>
#include <linux/bitops.h>
#include <linux/printk.h>

#define REG_CONFIG					0x00
	#define REG_CONFIG_CLK_EN			BIT(0)
	#define REG_CONFIG_CLK_SEL_MASK			GENMASK(3, 1)
	#define REG_CONFIG_CLK_DIV_MASK			GENMASK(10, 4)
	#define REG_CONFIG_CLK_32k_ALTSEL		BIT(15)
	#define REG_CONFIG_TEST_TRIG			BIT(31)

#define REG_CTRL					0x04
	#define REG_CTRL_SOFT_PRST			BIT(0)
	#define REG_CTRL_SOFT_HRESET			BIT(1)
	#define REG_CTRL_SS_SCALEDOWN_MODE_MASK		GENMASK(3, 2)
	#define REG_CTRL_CLK_DET_RST			BIT(4)
	#define REG_CTRL_INTR_SEL			BIT(5)
	#define REG_CTRL_CLK_DETECTED			BIT(8)
	#define REG_CTRL_SOF_SENT_RCVD_TGL		BIT(9)
	#define REG_CTRL_SOF_TOGGLE_OUT			BIT(10)
	#define REG_CTRL_POWER_ON_RESET			BIT(15)
	#define REG_CTRL_SLEEPM				BIT(16)
	#define REG_CTRL_TX_BITSTUFF_ENN_H		BIT(17)
	#define REG_CTRL_TX_BITSTUFF_ENN		BIT(18)
	#define REG_CTRL_COMMON_ON			BIT(19)
	#define REG_CTRL_REF_CLK_SEL_MASK		GENMASK(21, 20)
	#define REG_CTRL_REF_CLK_SEL_SHIFT		20
	#define REG_CTRL_FSEL_MASK			GENMASK(24, 22)
	#define REG_CTRL_FSEL_SHIFT			22
	#define REG_CTRL_PORT_RESET			BIT(25)
	#define REG_CTRL_THREAD_ID_MASK			GENMASK(31, 26)

/* bits [31:26], [24:21] and [15:3] seem to be read-only */
#define REG_ADP_BC					0x0c
	#define REG_ADP_BC_VBUS_VLD_EXT_SEL		BIT(0)
	#define REG_ADP_BC_VBUS_VLD_EXT			BIT(1)
	#define REG_ADP_BC_OTG_DISABLE			BIT(2)
	#define REG_ADP_BC_ID_PULLUP			BIT(3)
	#define REG_ADP_BC_DRV_VBUS			BIT(4)
	#define REG_ADP_BC_ADP_PRB_EN			BIT(5)
	#define REG_ADP_BC_ADP_DISCHARGE		BIT(6)
	#define REG_ADP_BC_ADP_CHARGE			BIT(7)
	#define REG_ADP_BC_SESS_END			BIT(8)
	#define REG_ADP_BC_DEVICE_SESS_VLD		BIT(9)
	#define REG_ADP_BC_B_VALID			BIT(10)
	#define REG_ADP_BC_A_VALID			BIT(11)
	#define REG_ADP_BC_ID_DIG			BIT(12)
	#define REG_ADP_BC_VBUS_VALID			BIT(13)
	#define REG_ADP_BC_ADP_PROBE			BIT(14)
	#define REG_ADP_BC_ADP_SENSE			BIT(15)
	#define REG_ADP_BC_ACA_ENABLE			BIT(16)
	#define REG_ADP_BC_DCD_ENABLE			BIT(17)
	#define REG_ADP_BC_VDAT_DET_EN_B		BIT(18)
	#define REG_ADP_BC_VDAT_SRC_EN_B		BIT(19)
	#define REG_ADP_BC_CHARGE_SEL			BIT(20)
	#define REG_ADP_BC_CHARGE_DETECT		BIT(21)
	#define REG_ADP_BC_ACA_PIN_RANGE_C		BIT(22)
	#define REG_ADP_BC_ACA_PIN_RANGE_B		BIT(23)
	#define REG_ADP_BC_ACA_PIN_RANGE_A		BIT(24)
	#define REG_ADP_BC_ACA_PIN_GND			BIT(25)
	#define REG_ADP_BC_ACA_PIN_FLOAT		BIT(26)

#define RESET_COMPLETE_TIME				500
#define ACA_ENABLE_COMPLETE_TIME			50

struct phy_meson_gxbb_usb2_priv {
	struct regmap *regmap;
	struct reset_ctl_bulk resets;
};

static int phy_meson_gxbb_usb2_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxbb_usb2_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_update_bits(priv->regmap, REG_CONFIG,
			   REG_CONFIG_CLK_32k_ALTSEL,
			   REG_CONFIG_CLK_32k_ALTSEL);
	regmap_update_bits(priv->regmap, REG_CTRL,
			   REG_CTRL_REF_CLK_SEL_MASK,
			   0x2 << REG_CTRL_REF_CLK_SEL_SHIFT);
	regmap_update_bits(priv->regmap, REG_CTRL,
			   REG_CTRL_FSEL_MASK,
			   0x5 << REG_CTRL_FSEL_SHIFT);

	/* reset the PHY */
	regmap_update_bits(priv->regmap, REG_CTRL,
			   REG_CTRL_POWER_ON_RESET,
			   REG_CTRL_POWER_ON_RESET);
	udelay(RESET_COMPLETE_TIME);
	regmap_update_bits(priv->regmap, REG_CTRL,
			   REG_CTRL_POWER_ON_RESET,
			   0);
	udelay(RESET_COMPLETE_TIME);

	regmap_update_bits(priv->regmap, REG_CTRL,
			   REG_CTRL_SOF_TOGGLE_OUT,
			   REG_CTRL_SOF_TOGGLE_OUT);

	/* Set host mode */
	regmap_update_bits(priv->regmap, REG_ADP_BC,
			   REG_ADP_BC_ACA_ENABLE,
			   REG_ADP_BC_ACA_ENABLE);
	udelay(ACA_ENABLE_COMPLETE_TIME);

	regmap_read(priv->regmap, REG_ADP_BC, &val);
	if (val & REG_ADP_BC_ACA_PIN_FLOAT) {
		pr_err("Error powering on GXBB USB PHY\n");
		return -EINVAL;
	}

	return 0;
}

static struct phy_ops meson_gxbb_usb2_phy_ops = {
	.power_on = phy_meson_gxbb_usb2_power_on,
};

static int meson_gxbb_usb2_phy_probe(struct udevice *dev)
{
	struct phy_meson_gxbb_usb2_priv *priv = dev_get_priv(dev);
	struct clk clk_usb_general, clk_usb;
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	ret = clk_get_by_name(dev, "usb_general", &clk_usb_general);
	if (ret)
		return ret;

	ret = clk_enable(&clk_usb_general);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
		pr_err("Failed to enable PHY general clock\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "usb", &clk_usb);
	if (ret)
		return ret;

	ret = clk_enable(&clk_usb);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
		pr_err("Failed to enable PHY clock\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &priv->resets);
	if (!ret) {
		ret = reset_deassert_bulk(&priv->resets);
		if (ret) {
			pr_err("Failed to deassert reset\n");
			return ret;
		}
	}

	return 0;
}

static int meson_gxbb_usb2_phy_remove(struct udevice *dev)
{
	struct phy_meson_gxbb_usb2_priv *priv = dev_get_priv(dev);

	return reset_release_bulk(&priv->resets);
}

static const struct udevice_id meson_gxbb_usb2_phy_ids[] = {
	{ .compatible = "amlogic,meson8-usb2-phy" },
	{ .compatible = "amlogic,meson8b-usb2-phy" },
	{ .compatible = "amlogic,meson-gxbb-usb2-phy" },
	{ }
};

U_BOOT_DRIVER(meson_gxbb_usb2_phy) = {
	.name = "meson_gxbb_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = meson_gxbb_usb2_phy_ids,
	.probe = meson_gxbb_usb2_phy_probe,
	.remove = meson_gxbb_usb2_phy_remove,
	.ops = &meson_gxbb_usb2_phy_ops,
	.priv_auto	= sizeof(struct phy_meson_gxbb_usb2_priv),
};
