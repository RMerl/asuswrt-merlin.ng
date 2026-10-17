// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic GXL DWC3 Glue layer
 *
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#define DEBUG
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dwc3-uboot.h>
#include <generic-phy.h>
#include <linux/io.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <malloc.h>
#include <regmap.h>
#include <usb.h>
#include "core.h"
#include "gadget.h"
#include <reset.h>
#include <clk.h>
#include <power/regulator.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <asm/arch/usb-gx.h>

/* USB Glue Control Registers */

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

enum {
	USB2_HOST_PHY0 = 0,
	USB2_OTG_PHY1,
	USB2_HOST_PHY2,
	PHY_COUNT,
};

static const char *phy_names[PHY_COUNT] = {
	"usb2-phy0", "usb2-phy1", "usb2-phy2",
};

struct dwc3_meson_gxl {
	struct udevice		*dev;
	struct regmap           *regmap;
	struct clk		clk;
	struct reset_ctl	reset;
	struct phy		phys[PHY_COUNT];
	enum usb_dr_mode	otg_mode;
	enum usb_dr_mode	otg_phy_mode;
	unsigned int		usb2_ports;
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	struct udevice		*vbus_supply;
#endif
};

#define U2P_REG_SIZE						0x20
#define USB_REG_OFFSET						0x80

#define USB2_OTG_PHY						USB2_OTG_PHY1

static void dwc3_meson_gxl_usb2_set_mode(struct dwc3_meson_gxl *priv, enum usb_dr_mode mode)
{
	switch (mode) {
	case USB_DR_MODE_HOST:
	case USB_DR_MODE_OTG:
	case USB_DR_MODE_UNKNOWN:
		regmap_update_bits(priv->regmap, USB_R1,
				   USB_R1_U3H_HOST_U2_PORT_DISABLE_MASK, 0);
		regmap_update_bits(priv->regmap, USB_R0,
				   USB_R0_U2D_ACT, 0);
		regmap_update_bits(priv->regmap, USB_R4,
				   USB_R4_P21_SLEEP_M0, 0);
		break;

	case USB_DR_MODE_PERIPHERAL:
		regmap_update_bits(priv->regmap, USB_R0,
				   USB_R0_U2D_ACT, USB_R0_U2D_ACT);
		regmap_update_bits(priv->regmap, USB_R0,
				   USB_R0_U2D_SS_SCALEDOWN_MODE_MASK, 0);
		regmap_update_bits(priv->regmap, USB_R4,
				   USB_R4_P21_SLEEP_M0, USB_R4_P21_SLEEP_M0);
		break;
	}
}

static int dwc3_meson_gxl_usb2_init(struct dwc3_meson_gxl *priv)
{
	int i;

	for (i = 0; i < PHY_COUNT; ++i) {
		if (!priv->phys[i].dev)
			continue;

		phy_meson_gxl_usb2_set_mode(&priv->phys[i],
				(i == USB2_OTG_PHY) ? USB_DR_MODE_PERIPHERAL
						    : USB_DR_MODE_HOST);
	}

	return 0;
}

static int dwc3_meson_gxl_usb_init(struct dwc3_meson_gxl *priv)
{
	int ret;

	ret = dwc3_meson_gxl_usb2_init(priv);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap, USB_R1,
			   USB_R1_U3H_FLADJ_30MHZ_REG_MASK,
			   FIELD_PREP(USB_R1_U3H_FLADJ_30MHZ_REG_MASK, 0x20));

	regmap_update_bits(priv->regmap, USB_R5,
			   USB_R5_ID_DIG_EN_0,
			   USB_R5_ID_DIG_EN_0);
	regmap_update_bits(priv->regmap, USB_R5,
			   USB_R5_ID_DIG_EN_1,
			   USB_R5_ID_DIG_EN_1);
	regmap_update_bits(priv->regmap, USB_R5,
			   USB_R5_ID_DIG_TH_MASK,
			   FIELD_PREP(USB_R5_ID_DIG_TH_MASK, 0xff));

	dwc3_meson_gxl_usb2_set_mode(priv, priv->otg_phy_mode);

	return 0;
}

int dwc3_meson_gxl_force_mode(struct udevice *dev, enum usb_dr_mode mode)
{
	struct dwc3_meson_gxl *priv = dev_get_plat(dev);

	if (!priv)
		return -EINVAL;

	if (mode != USB_DR_MODE_HOST && mode != USB_DR_MODE_PERIPHERAL)
		return -EINVAL;

	if (!priv->phys[USB2_OTG_PHY].dev)
		return -EINVAL;

	if (mode == priv->otg_phy_mode)
		return 0;

	if (mode == USB_DR_MODE_HOST)
		debug("%s: switching to Host Mode\n", __func__);
	else
		debug("%s: switching to Device Mode\n", __func__);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply) {
		int ret = regulator_set_enable(priv->vbus_supply,
					(mode == USB_DR_MODE_PERIPHERAL));
		if (ret)
			return ret;
	}
#endif
	priv->otg_phy_mode = mode;

	phy_meson_gxl_usb2_set_mode(&priv->phys[USB2_OTG_PHY], mode);

	dwc3_meson_gxl_usb2_set_mode(priv, mode);

	return 0;
}

static int dwc3_meson_gxl_get_phys(struct dwc3_meson_gxl *priv)
{
	int i, ret;

	for (i = 0 ; i < PHY_COUNT ; ++i) {
		ret = generic_phy_get_by_name(priv->dev, phy_names[i],
					      &priv->phys[i]);
		if (ret == -ENOENT || ret == -ENODATA) {
			priv->phys[i].dev = NULL;
			continue;
		}

		if (ret)
			return ret;

		priv->usb2_ports++;
	}

	debug("%s: usb2 ports: %d\n", __func__, priv->usb2_ports);

	return 0;
}

static int dwc3_meson_gxl_reset_init(struct dwc3_meson_gxl *priv)
{
	int ret;

	ret = reset_get_by_index(priv->dev, 0, &priv->reset);
	if (ret)
		return ret;

	ret = reset_assert(&priv->reset);
	udelay(1);
	ret |= reset_deassert(&priv->reset);
	if (ret) {
		reset_free(&priv->reset);
		return ret;
	}

	return 0;
}

static int dwc3_meson_gxl_clk_init(struct dwc3_meson_gxl *priv)
{
	int ret;

	ret = clk_get_by_index(priv->dev, 0, &priv->clk);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;
#endif

	return 0;
}

static int dwc3_meson_gxl_probe(struct udevice *dev)
{
	struct dwc3_meson_gxl *priv = dev_get_plat(dev);
	int ret, i;

	priv->dev = dev;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	ret = dwc3_meson_gxl_clk_init(priv);
	if (ret)
		return ret;

	ret = dwc3_meson_gxl_reset_init(priv);
	if (ret)
		return ret;

	ret = dwc3_meson_gxl_get_phys(priv);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret && ret != -ENOENT) {
		pr_err("Failed to get PHY regulator\n");
		return ret;
	}

	if (priv->vbus_supply) {
		ret = regulator_set_enable(priv->vbus_supply, true);
		if (ret)
			return ret;
	}
#endif

	/* On GXL PHY must be started in device mode for DWC2 init */
	priv->otg_mode = USB_DR_MODE_PERIPHERAL;

	ret = dwc3_meson_gxl_usb_init(priv);
	if (ret)
		return ret;

	priv->otg_mode = usb_get_dr_mode(dev_ofnode(dev));

	if (priv->otg_mode == USB_DR_MODE_PERIPHERAL)
		priv->otg_phy_mode = USB_DR_MODE_PERIPHERAL;
	else
		priv->otg_phy_mode = USB_DR_MODE_HOST;

	for (i = 0 ; i < PHY_COUNT ; ++i) {
		if (!priv->phys[i].dev)
			continue;

		ret = generic_phy_init(&priv->phys[i]);
		if (ret)
			goto err_phy_init;
	}

	for (i = 0; i < PHY_COUNT; ++i) {
		if (!priv->phys[i].dev)
			continue;

		ret = generic_phy_power_on(&priv->phys[i]);
		if (ret)
			goto err_phy_init;
	}

	if (priv->phys[USB2_OTG_PHY].dev)
		phy_meson_gxl_usb2_set_mode(&priv->phys[USB2_OTG_PHY],
					    priv->otg_phy_mode);

	dwc3_meson_gxl_usb2_set_mode(priv, priv->otg_phy_mode);

	return 0;

err_phy_init:
	for (i = 0 ; i < PHY_COUNT ; ++i) {
		if (!priv->phys[i].dev)
			continue;

		 generic_phy_exit(&priv->phys[i]);
	}

	return ret;
}

static int dwc3_meson_gxl_remove(struct udevice *dev)
{
	struct dwc3_meson_gxl *priv = dev_get_plat(dev);
	int i;

	reset_release_all(&priv->reset, 1);

	clk_release_all(&priv->clk, 1);

	for (i = 0; i < PHY_COUNT; ++i) {
		if (!priv->phys[i].dev)
			continue;

		 generic_phy_power_off(&priv->phys[i]);
	}

	for (i = 0 ; i < PHY_COUNT ; ++i) {
		if (!priv->phys[i].dev)
			continue;

		 generic_phy_exit(&priv->phys[i]);
	}

	return dm_scan_fdt_dev(dev);
}

static int dwc3_meson_gxl_child_pre_probe(struct udevice *dev)
{
	if (ofnode_device_is_compatible(dev_ofnode(dev), "amlogic,meson-g12a-usb"))
		return dwc3_meson_gxl_force_mode(dev->parent, USB_DR_MODE_PERIPHERAL);

	return 0;
}

static int dwc3_meson_gxl_child_post_remove(struct udevice *dev)
{
	if (ofnode_device_is_compatible(dev_ofnode(dev), "amlogic,meson-g12a-usb"))
		return dwc3_meson_gxl_force_mode(dev->parent, USB_DR_MODE_HOST);

	return 0;
}

static const struct udevice_id dwc3_meson_gxl_ids[] = {
	{ .compatible = "amlogic,meson-axg-usb-ctrl" },
	{ .compatible = "amlogic,meson-gxl-usb-ctrl" },
	{ .compatible = "amlogic,meson-gxm-usb-ctrl" },
	{ }
};

U_BOOT_DRIVER(dwc3_generic_wrapper) = {
	.name	= "dwc3-meson-gxl",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = dwc3_meson_gxl_ids,
	.probe = dwc3_meson_gxl_probe,
	.remove = dwc3_meson_gxl_remove,
	.child_pre_probe = dwc3_meson_gxl_child_pre_probe,
	.child_post_remove = dwc3_meson_gxl_child_post_remove,
	.plat_auto	= sizeof(struct dwc3_meson_gxl),

};
