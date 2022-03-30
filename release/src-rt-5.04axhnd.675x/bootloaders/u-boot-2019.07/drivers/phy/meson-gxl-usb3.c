// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson GXL USB3 PHY driver
 *
 * Copyright (C) 2018 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstron@baylibre.com>
 */

#include <common.h>
#include <asm/io.h>
#include <bitfield.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <clk.h>

#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/bitfield.h>

#define USB_R0							0x00
	#define USB_R0_P30_FSEL_MASK				GENMASK(5, 0)
	#define USB_R0_P30_PHY_RESET				BIT(6)
	#define USB_R0_P30_TEST_POWERDOWN_HSP			BIT(7)
	#define USB_R0_P30_TEST_POWERDOWN_SSP			BIT(8)
	#define USB_R0_P30_ACJT_LEVEL_MASK			GENMASK(13, 9)
	#define USB_R0_P30_TX_BOOST_LEVEL_MASK			GENMASK(16, 14)
	#define USB_R0_P30_LANE0_TX2RX_LOOPBACK			BIT(17)
	#define USB_R0_P30_LANE0_EXT_PCLK_REQ			BIT(18)
	#define USB_R0_P30_PCS_RX_LOS_MASK_VAL_MASK		GENMASK(28, 19)
	#define USB_R0_U2D_SS_SCALEDOWN_MODE_MASK		GENMASK(30, 29)
	#define USB_R0_U2D_ACT					BIT(31)

#define USB_R1							0x04
	#define USB_R1_U3H_BIGENDIAN_GS				BIT(0)
	#define USB_R1_U3H_PME_ENABLE				BIT(1)
	#define USB_R1_U3H_HUB_PORT_OVERCURRENT_MASK		GENMASK(6, 2)
	#define USB_R1_U3H_HUB_PORT_PERM_ATTACH_MASK		GENMASK(11, 7)
	#define USB_R1_U3H_HOST_U2_PORT_DISABLE_MASK		GENMASK(15, 12)
	#define USB_R1_U3H_HOST_U3_PORT_DISABLE			BIT(16)
	#define USB_R1_U3H_HOST_PORT_POWER_CONTROL_PRESENT	BIT(17)
	#define USB_R1_U3H_HOST_MSI_ENABLE			BIT(18)
	#define USB_R1_U3H_FLADJ_30MHZ_REG_MASK			GENMASK(24, 19)
	#define USB_R1_P30_PCS_TX_SWING_FULL_MASK		GENMASK(31, 25)

#define USB_R2							0x08
	#define USB_R2_P30_CR_DATA_IN_MASK			GENMASK(15, 0)
	#define USB_R2_P30_CR_READ				BIT(16)
	#define USB_R2_P30_CR_WRITE				BIT(17)
	#define USB_R2_P30_CR_CAP_ADDR				BIT(18)
	#define USB_R2_P30_CR_CAP_DATA				BIT(19)
	#define USB_R2_P30_PCS_TX_DEEMPH_3P5DB_MASK		GENMASK(25, 20)
	#define USB_R2_P30_PCS_TX_DEEMPH_6DB_MASK		GENMASK(31, 26)

#define USB_R3							0x0c
	#define USB_R3_P30_SSC_ENABLE				BIT(0)
	#define USB_R3_P30_SSC_RANGE_MASK			GENMASK(3, 1)
	#define USB_R3_P30_SSC_REF_CLK_SEL_MASK			GENMASK(12, 4)
	#define USB_R3_P30_REF_SSP_EN				BIT(13)
	#define USB_R3_P30_LOS_BIAS_MASK			GENMASK(18, 16)
	#define USB_R3_P30_LOS_LEVEL_MASK			GENMASK(23, 19)
	#define USB_R3_P30_MPLL_MULTIPLIER_MASK			GENMASK(30, 24)

#define USB_R4							0x10
	#define USB_R4_P21_PORT_RESET_0				BIT(0)
	#define USB_R4_P21_SLEEP_M0				BIT(1)
	#define USB_R4_MEM_PD_MASK				GENMASK(3, 2)
	#define USB_R4_P21_ONLY					BIT(4)

#define USB_R5							0x14
	#define USB_R5_ID_DIG_SYNC				BIT(0)
	#define USB_R5_ID_DIG_REG				BIT(1)
	#define USB_R5_ID_DIG_CFG_MASK				GENMASK(3, 2)
	#define USB_R5_ID_DIG_EN_0				BIT(4)
	#define USB_R5_ID_DIG_EN_1				BIT(5)
	#define USB_R5_ID_DIG_CURR				BIT(6)
	#define USB_R5_ID_DIG_IRQ				BIT(7)
	#define USB_R5_ID_DIG_TH_MASK				GENMASK(15, 8)
	#define USB_R5_ID_DIG_CNT_MASK				GENMASK(23, 16)

/* read-only register */
#define USB_R6							0x18
	#define USB_R6_P30_CR_DATA_OUT_MASK			GENMASK(15, 0)
	#define USB_R6_P30_CR_ACK				BIT(16)

struct phy_meson_gxl_usb3_priv {
	struct regmap		*regmap;
#if CONFIG_IS_ENABLED(CLK)
	struct clk		clk;
#endif
};

static int
phy_meson_gxl_usb3_set_host_mode(struct phy_meson_gxl_usb3_priv *priv)
{
	uint val;

	regmap_read(priv->regmap, USB_R0, &val);
	val &= ~USB_R0_U2D_ACT;
	regmap_write(priv->regmap, USB_R0, val);

	regmap_read(priv->regmap, USB_R4, &val);
	val &= ~USB_R4_P21_SLEEP_M0;
	regmap_write(priv->regmap, USB_R4, val);

	return 0;
}

static int phy_meson_gxl_usb3_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxl_usb3_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_read(priv->regmap, USB_R5, &val);
	val |= USB_R5_ID_DIG_EN_0;
	val |= USB_R5_ID_DIG_EN_1;
	val &= ~USB_R5_ID_DIG_TH_MASK;
	val |= FIELD_PREP(USB_R5_ID_DIG_TH_MASK, 0xff);
	regmap_write(priv->regmap, USB_R5, val);

	return phy_meson_gxl_usb3_set_host_mode(priv);
}

static int phy_meson_gxl_usb3_power_off(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxl_usb3_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_read(priv->regmap, USB_R5, &val);
	val &= ~USB_R5_ID_DIG_EN_0;
	val &= ~USB_R5_ID_DIG_EN_1;
	regmap_write(priv->regmap, USB_R5, val);

	return 0;
}

static int phy_meson_gxl_usb3_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxl_usb3_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_read(priv->regmap, USB_R1, &val);
	val &= ~USB_R1_U3H_FLADJ_30MHZ_REG_MASK;
	val |= FIELD_PREP(USB_R1_U3H_FLADJ_30MHZ_REG_MASK, 0x20);
	regmap_write(priv->regmap, USB_R1, val);

	return 0;
}

struct phy_ops meson_gxl_usb3_phy_ops = {
	.init = phy_meson_gxl_usb3_init,
	.power_on = phy_meson_gxl_usb3_power_on,
	.power_off = phy_meson_gxl_usb3_power_off,
};

int meson_gxl_usb3_phy_probe(struct udevice *dev)
{
	struct phy_meson_gxl_usb3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;
	
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

	return 0;
}

static const struct udevice_id meson_gxl_usb3_phy_ids[] = {
	{ .compatible = "amlogic,meson-gxl-usb3-phy" },
	{ }
};

U_BOOT_DRIVER(meson_gxl_usb3_phy) = {
	.name = "meson_gxl_usb3_phy",
	.id = UCLASS_PHY,
	.of_match = meson_gxl_usb3_phy_ids,
	.probe = meson_gxl_usb3_phy_probe,
	.ops = &meson_gxl_usb3_phy_ops,
	.priv_auto_alloc_size = sizeof(struct phy_meson_gxl_usb3_priv),
};
