// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson GXL and GXM USB2 PHY driver
 *
 * Copyright (C) 2017 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
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
#include <power/regulator.h>
#include <clk.h>

#include <linux/bitops.h>
#include <linux/compat.h>

/* bits [31:27] are read-only */
#define U2P_R0							0x0
	#define U2P_R0_BYPASS_SEL				BIT(0)
	#define U2P_R0_BYPASS_DM_EN				BIT(1)
	#define U2P_R0_BYPASS_DP_EN				BIT(2)
	#define U2P_R0_TXBITSTUFF_ENH				BIT(3)
	#define U2P_R0_TXBITSTUFF_EN				BIT(4)
	#define U2P_R0_DM_PULLDOWN				BIT(5)
	#define U2P_R0_DP_PULLDOWN				BIT(6)
	#define U2P_R0_DP_VBUS_VLD_EXT_SEL			BIT(7)
	#define U2P_R0_DP_VBUS_VLD_EXT				BIT(8)
	#define U2P_R0_ADP_PRB_EN				BIT(9)
	#define U2P_R0_ADP_DISCHARGE				BIT(10)
	#define U2P_R0_ADP_CHARGE				BIT(11)
	#define U2P_R0_DRV_VBUS					BIT(12)
	#define U2P_R0_ID_PULLUP				BIT(13)
	#define U2P_R0_LOOPBACK_EN_B				BIT(14)
	#define U2P_R0_OTG_DISABLE				BIT(15)
	#define U2P_R0_COMMON_ONN				BIT(16)
	#define U2P_R0_FSEL_MASK				GENMASK(19, 17)
	#define U2P_R0_REF_CLK_SEL_MASK				GENMASK(21, 20)
	#define U2P_R0_POWER_ON_RESET				BIT(22)
	#define U2P_R0_V_ATE_TEST_EN_B_MASK			GENMASK(24, 23)
	#define U2P_R0_ID_SET_ID_DQ				BIT(25)
	#define U2P_R0_ATE_RESET				BIT(26)
	#define U2P_R0_FSV_MINUS				BIT(27)
	#define U2P_R0_FSV_PLUS					BIT(28)
	#define U2P_R0_BYPASS_DM_DATA				BIT(29)
	#define U2P_R0_BYPASS_DP_DATA				BIT(30)

#define U2P_R1							0x4
	#define U2P_R1_BURN_IN_TEST				BIT(0)
	#define U2P_R1_ACA_ENABLE				BIT(1)
	#define U2P_R1_DCD_ENABLE				BIT(2)
	#define U2P_R1_VDAT_SRC_EN_B				BIT(3)
	#define U2P_R1_VDAT_DET_EN_B				BIT(4)
	#define U2P_R1_CHARGES_SEL				BIT(5)
	#define U2P_R1_TX_PREEMP_PULSE_TUNE			BIT(6)
	#define U2P_R1_TX_PREEMP_AMP_TUNE_MASK			GENMASK(8, 7)
	#define U2P_R1_TX_RES_TUNE_MASK				GENMASK(10, 9)
	#define U2P_R1_TX_RISE_TUNE_MASK			GENMASK(12, 11)
	#define U2P_R1_TX_VREF_TUNE_MASK			GENMASK(16, 13)
	#define U2P_R1_TX_FSLS_TUNE_MASK			GENMASK(20, 17)
	#define U2P_R1_TX_HSXV_TUNE_MASK			GENMASK(22, 21)
	#define U2P_R1_OTG_TUNE_MASK				GENMASK(25, 23)
	#define U2P_R1_SQRX_TUNE_MASK				GENMASK(28, 26)
	#define U2P_R1_COMP_DIS_TUNE_MASK			GENMASK(31, 29)

/* bits [31:14] are read-only */
#define U2P_R2							0x8
	#define U2P_R2_TESTDATA_IN_MASK				GENMASK(7, 0)
	#define U2P_R2_TESTADDR_MASK				GENMASK(11, 8)
	#define U2P_R2_TESTDATA_OUT_SEL				BIT(12)
	#define U2P_R2_TESTCLK					BIT(13)
	#define U2P_R2_TESTDATA_OUT_MASK			GENMASK(17, 14)
	#define U2P_R2_ACA_PIN_RANGE_C				BIT(18)
	#define U2P_R2_ACA_PIN_RANGE_B				BIT(19)
	#define U2P_R2_ACA_PIN_RANGE_A				BIT(20)
	#define U2P_R2_ACA_PIN_GND				BIT(21)
	#define U2P_R2_ACA_PIN_FLOAT				BIT(22)
	#define U2P_R2_CHARGE_DETECT				BIT(23)
	#define U2P_R2_DEVICE_SESSION_VALID			BIT(24)
	#define U2P_R2_ADP_PROBE				BIT(25)
	#define U2P_R2_ADP_SENSE				BIT(26)
	#define U2P_R2_SESSION_END				BIT(27)
	#define U2P_R2_VBUS_VALID				BIT(28)
	#define U2P_R2_B_VALID					BIT(29)
	#define U2P_R2_A_VALID					BIT(30)
	#define U2P_R2_ID_DIG					BIT(31)

#define U2P_R3							0xc

#define RESET_COMPLETE_TIME				500

struct phy_meson_gxl_usb2_priv {
	struct regmap		*regmap;
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	struct udevice		*phy_supply;
#endif
#if CONFIG_IS_ENABLED(CLK)
	struct clk		clk;
#endif
};

static void phy_meson_gxl_usb2_reset(struct phy_meson_gxl_usb2_priv *priv)
{
	uint val;

	regmap_read(priv->regmap, U2P_R0, &val);

	/* reset the PHY and wait until settings are stabilized */
	val |= U2P_R0_POWER_ON_RESET;
	regmap_write(priv->regmap, U2P_R0, val);
	udelay(RESET_COMPLETE_TIME);

	val &= ~U2P_R0_POWER_ON_RESET;
	regmap_write(priv->regmap, U2P_R0, val);
	udelay(RESET_COMPLETE_TIME);
}

static void
phy_meson_gxl_usb2_set_host_mode(struct phy_meson_gxl_usb2_priv *priv)
{
	uint val;

	regmap_read(priv->regmap, U2P_R0, &val);
	val |= U2P_R0_DM_PULLDOWN;
	val |= U2P_R0_DP_PULLDOWN;
	val &= ~U2P_R0_ID_PULLUP;
	regmap_write(priv->regmap, U2P_R0, val);

	phy_meson_gxl_usb2_reset(priv);
}

static int phy_meson_gxl_usb2_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxl_usb2_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_read(priv->regmap, U2P_R0, &val);
	/* power on the PHY by taking it out of reset mode */
	val &= ~U2P_R0_POWER_ON_RESET;
	regmap_write(priv->regmap, U2P_R0, val);

	phy_meson_gxl_usb2_set_host_mode(priv);

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->phy_supply) {
		int ret = regulator_set_enable(priv->phy_supply, true);
		if (ret)
			return ret;
	}
#endif

	return 0;
}

static int phy_meson_gxl_usb2_power_off(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_gxl_usb2_priv *priv = dev_get_priv(dev);
	uint val;

	regmap_read(priv->regmap, U2P_R0, &val);
	/* power off the PHY by putting it into reset mode */
	val |= U2P_R0_POWER_ON_RESET;
	regmap_write(priv->regmap, U2P_R0, val);

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

struct phy_ops meson_gxl_usb2_phy_ops = {
	.power_on = phy_meson_gxl_usb2_power_on,
	.power_off = phy_meson_gxl_usb2_power_off,
};

int meson_gxl_usb2_phy_probe(struct udevice *dev)
{
	struct phy_meson_gxl_usb2_priv *priv = dev_get_priv(dev);
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

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "phy-supply", &priv->phy_supply);
	if (ret && ret != -ENOENT) {
		pr_err("Failed to get PHY regulator\n");
		return ret;
	}
#endif

	return 0;
}

static const struct udevice_id meson_gxl_usb2_phy_ids[] = {
	{ .compatible = "amlogic,meson-gxl-usb2-phy" },
	{ }
};

U_BOOT_DRIVER(meson_gxl_usb2_phy) = {
	.name = "meson_gxl_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = meson_gxl_usb2_phy_ids,
	.probe = meson_gxl_usb2_phy_probe,
	.ops = &meson_gxl_usb2_phy_ops,
	.priv_auto_alloc_size = sizeof(struct phy_meson_gxl_usb2_priv),
};
