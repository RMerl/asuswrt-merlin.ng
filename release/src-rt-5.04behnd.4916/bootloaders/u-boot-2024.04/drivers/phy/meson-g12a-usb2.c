// SPDX-License-Identifier: GPL-2.0+
/*
 * Meson G12A USB2 PHY driver
 *
 * Copyright (C) 2017 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstron@baylibre.com>
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
#include <linux/delay.h>
#include <linux/printk.h>
#include <power/regulator.h>
#include <power-domain.h>
#include <reset.h>
#include <clk.h>

#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/bitfield.h>

#define PHY_CTRL_R0						0x0
#define PHY_CTRL_R1						0x4
#define PHY_CTRL_R2						0x8

#define PHY_CTRL_R3						0xc
	#define PHY_CTRL_R3_SQUELCH_REF				GENMASK(1, 0)
	#define PHY_CTRL_R3_HSDIC_REF				GENMASK(3, 2)
	#define PHY_CTRL_R3_DISC_THRESH				GENMASK(7, 4)

#define PHY_CTRL_R4						0x10
	#define PHY_CTRL_R4_CALIB_CODE_7_0			GENMASK(7, 0)
	#define PHY_CTRL_R4_CALIB_CODE_15_8			GENMASK(15, 8)
	#define PHY_CTRL_R4_CALIB_CODE_23_16			GENMASK(23, 16)
	#define PHY_CTRL_R4_I_C2L_CAL_EN			BIT(24)
	#define PHY_CTRL_R4_I_C2L_CAL_RESET_N			BIT(25)
	#define PHY_CTRL_R4_I_C2L_CAL_DONE			BIT(26)
	#define PHY_CTRL_R4_TEST_BYPASS_MODE_EN			BIT(27)
	#define PHY_CTRL_R4_I_C2L_BIAS_TRIM_1_0			GENMASK(29, 28)
	#define PHY_CTRL_R4_I_C2L_BIAS_TRIM_3_2			GENMASK(31, 30)

#define PHY_CTRL_R5						0x14
#define PHY_CTRL_R6						0x18
#define PHY_CTRL_R7						0x1c
#define PHY_CTRL_R8						0x20
#define PHY_CTRL_R9						0x24
#define PHY_CTRL_R10						0x28
#define PHY_CTRL_R11						0x2c
#define PHY_CTRL_R12						0x30

#define PHY_CTRL_R13						0x34
	#define PHY_CTRL_R13_CUSTOM_PATTERN_19			GENMASK(7, 0)
	#define PHY_CTRL_R13_LOAD_STAT				BIT(14)
	#define PHY_CTRL_R13_UPDATE_PMA_SIGNALS			BIT(15)
	#define PHY_CTRL_R13_MIN_COUNT_FOR_SYNC_DET		GENMASK(20, 16)
	#define PHY_CTRL_R13_CLEAR_HOLD_HS_DISCONNECT		BIT(21)
	#define PHY_CTRL_R13_BYPASS_HOST_DISCONNECT_VAL		BIT(22)
	#define PHY_CTRL_R13_BYPASS_HOST_DISCONNECT_EN		BIT(23)
	#define PHY_CTRL_R13_I_C2L_HS_EN			BIT(24)
	#define PHY_CTRL_R13_I_C2L_FS_EN			BIT(25)
	#define PHY_CTRL_R13_I_C2L_LS_EN			BIT(26)
	#define PHY_CTRL_R13_I_C2L_HS_OE			BIT(27)
	#define PHY_CTRL_R13_I_C2L_FS_OE			BIT(28)
	#define PHY_CTRL_R13_I_C2L_HS_RX_EN			BIT(29)
	#define PHY_CTRL_R13_I_C2L_FSLS_RX_EN			BIT(30)

#define PHY_CTRL_R14						0x38
#define PHY_CTRL_R15						0x3c

#define PHY_CTRL_R16						0x40
	#define PHY_CTRL_R16_MPLL_M				GENMASK(8, 0)
	#define PHY_CTRL_R16_MPLL_N				GENMASK(14, 10)
	#define PHY_CTRL_R16_MPLL_TDC_MODE			BIT(20)
	#define PHY_CTRL_R16_MPLL_SDM_EN			BIT(21)
	#define PHY_CTRL_R16_MPLL_LOAD				BIT(22)
	#define PHY_CTRL_R16_MPLL_DCO_SDM_EN			BIT(23)
	#define PHY_CTRL_R16_MPLL_LOCK_LONG			GENMASK(25, 24)
	#define PHY_CTRL_R16_MPLL_LOCK_F			BIT(26)
	#define PHY_CTRL_R16_MPLL_FAST_LOCK			BIT(27)
	#define PHY_CTRL_R16_MPLL_EN				BIT(28)
	#define PHY_CTRL_R16_MPLL_RESET				BIT(29)
	#define PHY_CTRL_R16_MPLL_LOCK				BIT(30)
	#define PHY_CTRL_R16_MPLL_LOCK_DIG			BIT(31)

#define PHY_CTRL_R17						0x44
	#define PHY_CTRL_R17_MPLL_FRAC_IN			GENMASK(13, 0)
	#define PHY_CTRL_R17_MPLL_FIX_EN			BIT(16)
	#define PHY_CTRL_R17_MPLL_LAMBDA1			GENMASK(19, 17)
	#define PHY_CTRL_R17_MPLL_LAMBDA0			GENMASK(22, 20)
	#define PHY_CTRL_R17_MPLL_FILTER_MODE			BIT(23)
	#define PHY_CTRL_R17_MPLL_FILTER_PVT2			GENMASK(27, 24)
	#define PHY_CTRL_R17_MPLL_FILTER_PVT1			GENMASK(31, 28)

#define PHY_CTRL_R18						0x48
	#define PHY_CTRL_R18_MPLL_LKW_SEL			GENMASK(1, 0)
	#define PHY_CTRL_R18_MPLL_LK_W				GENMASK(5, 2)
	#define PHY_CTRL_R18_MPLL_LK_S				GENMASK(11, 6)
	#define PHY_CTRL_R18_MPLL_DCO_M_EN			BIT(12)
	#define PHY_CTRL_R18_MPLL_DCO_CLK_SEL			BIT(13)
	#define PHY_CTRL_R18_MPLL_PFD_GAIN			GENMASK(15, 14)
	#define PHY_CTRL_R18_MPLL_ROU				GENMASK(18, 16)
	#define PHY_CTRL_R18_MPLL_DATA_SEL			GENMASK(21, 19)
	#define PHY_CTRL_R18_MPLL_BIAS_ADJ			GENMASK(23, 22)
	#define PHY_CTRL_R18_MPLL_BB_MODE			GENMASK(25, 24)
	#define PHY_CTRL_R18_MPLL_ALPHA				GENMASK(28, 26)
	#define PHY_CTRL_R18_MPLL_ADJ_LDO			GENMASK(30, 29)
	#define PHY_CTRL_R18_MPLL_ACG_RANGE			BIT(31)

#define PHY_CTRL_R19						0x4c

#define PHY_CTRL_R20						0x50
	#define PHY_CTRL_R20_USB2_IDDET_EN			BIT(0)
	#define PHY_CTRL_R20_USB2_OTG_VBUS_TRIM_2_0		GENMASK(3, 1)
	#define PHY_CTRL_R20_USB2_OTG_VBUSDET_EN		BIT(4)
	#define PHY_CTRL_R20_USB2_AMON_EN			BIT(5)
	#define PHY_CTRL_R20_USB2_CAL_CODE_R5			BIT(6)
	#define PHY_CTRL_R20_BYPASS_OTG_DET			BIT(7)
	#define PHY_CTRL_R20_USB2_DMON_EN			BIT(8)
	#define PHY_CTRL_R20_USB2_DMON_SEL_3_0			GENMASK(12, 9)
	#define PHY_CTRL_R20_USB2_EDGE_DRV_EN			BIT(13)
	#define PHY_CTRL_R20_USB2_EDGE_DRV_TRIM_1_0		GENMASK(15, 14)
	#define PHY_CTRL_R20_USB2_BGR_ADJ_4_0			GENMASK(20, 16)
	#define PHY_CTRL_R20_USB2_BGR_START			BIT(21)
	#define PHY_CTRL_R20_USB2_BGR_VREF_4_0			GENMASK(28, 24)
	#define PHY_CTRL_R20_USB2_BGR_DBG_1_0			GENMASK(30, 29)
	#define PHY_CTRL_R20_BYPASS_CAL_DONE_R5			BIT(31)

#define PHY_CTRL_R21						0x54
	#define PHY_CTRL_R21_USB2_BGR_FORCE			BIT(0)
	#define PHY_CTRL_R21_USB2_CAL_ACK_EN			BIT(1)
	#define PHY_CTRL_R21_USB2_OTG_ACA_EN			BIT(2)
	#define PHY_CTRL_R21_USB2_TX_STRG_PD			BIT(3)
	#define PHY_CTRL_R21_USB2_OTG_ACA_TRIM_1_0		GENMASK(5, 4)
	#define PHY_CTRL_R21_BYPASS_UTMI_CNTR			GENMASK(15, 6)
	#define PHY_CTRL_R21_BYPASS_UTMI_REG			GENMASK(25, 20)

#define PHY_CTRL_R22						0x58
#define PHY_CTRL_R23						0x5c

#define RESET_COMPLETE_TIME					1000
#define PLL_RESET_COMPLETE_TIME					100

enum meson_soc_id {
	MESON_SOC_A1,
	MESON_SOC_G12A,
};

struct phy_meson_g12a_usb2_priv {
	struct regmap		*regmap;
#if CONFIG_IS_ENABLED(CLK)
	struct clk		clk;
#endif
	struct reset_ctl	reset;
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
	struct power_domain pwrdm;
#endif
	int soc_id;
};

static int phy_meson_g12a_usb2_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	u32 value;
	int ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
		pr_err("failed to enable PHY clock\n");
		return ret;
	}
#endif

	ret = reset_assert(&priv->reset);
	udelay(1);
	ret |= reset_deassert(&priv->reset);
	if (ret)
		return ret;

	udelay(RESET_COMPLETE_TIME);

	/* usb2_otg_aca_en == 0 */
	regmap_update_bits(priv->regmap, PHY_CTRL_R21, BIT(2), 0);

	/* PLL Setup : 24MHz * 20 / 1 = 480MHz */
	regmap_write(priv->regmap, PHY_CTRL_R16,
		FIELD_PREP(PHY_CTRL_R16_MPLL_M, 20) |
		FIELD_PREP(PHY_CTRL_R16_MPLL_N, 1) |
		PHY_CTRL_R16_MPLL_LOAD |
		FIELD_PREP(PHY_CTRL_R16_MPLL_LOCK_LONG, 1) |
		PHY_CTRL_R16_MPLL_FAST_LOCK |
		PHY_CTRL_R16_MPLL_EN |
		PHY_CTRL_R16_MPLL_RESET);

	regmap_write(priv->regmap, PHY_CTRL_R17,
		FIELD_PREP(PHY_CTRL_R17_MPLL_FRAC_IN, 0) |
		FIELD_PREP(PHY_CTRL_R17_MPLL_LAMBDA1, 7) |
		FIELD_PREP(PHY_CTRL_R17_MPLL_LAMBDA0, 7) |
		FIELD_PREP(PHY_CTRL_R17_MPLL_FILTER_PVT2, 2) |
		FIELD_PREP(PHY_CTRL_R17_MPLL_FILTER_PVT1, 9));

	value = FIELD_PREP(PHY_CTRL_R18_MPLL_LKW_SEL, 1) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_LK_W, 9) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_LK_S, 0x27) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_PFD_GAIN, 1) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_ROU, 7) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_DATA_SEL, 3) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_BIAS_ADJ, 1) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_BB_MODE, 0) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_ALPHA, 3) |
		FIELD_PREP(PHY_CTRL_R18_MPLL_ADJ_LDO, 1) |
		PHY_CTRL_R18_MPLL_ACG_RANGE;

	if (priv->soc_id == MESON_SOC_A1)
		value |= PHY_CTRL_R18_MPLL_DCO_CLK_SEL;

	regmap_write(priv->regmap, PHY_CTRL_R18, value);

	udelay(PLL_RESET_COMPLETE_TIME);

	/* UnReset PLL */
	regmap_write(priv->regmap, PHY_CTRL_R16,
		FIELD_PREP(PHY_CTRL_R16_MPLL_M, 20) |
		FIELD_PREP(PHY_CTRL_R16_MPLL_N, 1) |
		PHY_CTRL_R16_MPLL_LOAD |
		FIELD_PREP(PHY_CTRL_R16_MPLL_LOCK_LONG, 1) |
		PHY_CTRL_R16_MPLL_FAST_LOCK |
		PHY_CTRL_R16_MPLL_EN);

	/* PHY Tuning */
	regmap_write(priv->regmap, PHY_CTRL_R20,
		FIELD_PREP(PHY_CTRL_R20_USB2_OTG_VBUS_TRIM_2_0, 4) |
		PHY_CTRL_R20_USB2_OTG_VBUSDET_EN |
		FIELD_PREP(PHY_CTRL_R20_USB2_DMON_SEL_3_0, 15) |
		PHY_CTRL_R20_USB2_EDGE_DRV_EN |
		FIELD_PREP(PHY_CTRL_R20_USB2_EDGE_DRV_TRIM_1_0, 3) |
		FIELD_PREP(PHY_CTRL_R20_USB2_BGR_ADJ_4_0, 0) |
		FIELD_PREP(PHY_CTRL_R20_USB2_BGR_VREF_4_0, 0) |
		FIELD_PREP(PHY_CTRL_R20_USB2_BGR_DBG_1_0, 0));

	if (priv->soc_id == MESON_SOC_G12A)
		regmap_write(priv->regmap, PHY_CTRL_R4,
			FIELD_PREP(PHY_CTRL_R4_CALIB_CODE_7_0, 0xf) |
			FIELD_PREP(PHY_CTRL_R4_CALIB_CODE_15_8, 0xf) |
			FIELD_PREP(PHY_CTRL_R4_CALIB_CODE_23_16, 0xf) |
			PHY_CTRL_R4_TEST_BYPASS_MODE_EN |
			FIELD_PREP(PHY_CTRL_R4_I_C2L_BIAS_TRIM_1_0, 0) |
			FIELD_PREP(PHY_CTRL_R4_I_C2L_BIAS_TRIM_3_2, 0));
	else if (priv->soc_id == MESON_SOC_A1)
		regmap_write(priv->regmap, PHY_CTRL_R21,
			PHY_CTRL_R21_USB2_CAL_ACK_EN |
			PHY_CTRL_R21_USB2_TX_STRG_PD |
			FIELD_PREP(PHY_CTRL_R21_USB2_OTG_ACA_TRIM_1_0, 2));

	/* Tuning Disconnect Threshold */
	regmap_write(priv->regmap, PHY_CTRL_R3,
		FIELD_PREP(PHY_CTRL_R3_SQUELCH_REF, 0) |
		FIELD_PREP(PHY_CTRL_R3_HSDIC_REF, 1) |
		FIELD_PREP(PHY_CTRL_R3_DISC_THRESH, 3));

	/* Analog Settings */
	if (priv->soc_id == MESON_SOC_G12A) {
		regmap_write(priv->regmap, PHY_CTRL_R14, 0);
		regmap_write(priv->regmap, PHY_CTRL_R13,
			PHY_CTRL_R13_UPDATE_PMA_SIGNALS |
			FIELD_PREP(PHY_CTRL_R13_MIN_COUNT_FOR_SYNC_DET, 7));
	} else if (priv->soc_id == MESON_SOC_A1) {
		regmap_write(priv->regmap, PHY_CTRL_R13,
			FIELD_PREP(PHY_CTRL_R13_MIN_COUNT_FOR_SYNC_DET, 7));
	}

	return 0;
}

static int phy_meson_g12a_usb2_exit(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	int ret;

#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
#endif

	ret = reset_assert(&priv->reset);
	if (ret)
		return ret;

	return 0;
}

struct phy_ops meson_g12a_usb2_phy_ops = {
	.init = phy_meson_g12a_usb2_init,
	.exit = phy_meson_g12a_usb2_exit,
};

int meson_g12a_usb2_phy_probe(struct udevice *dev)
{
	struct phy_meson_g12a_usb2_priv *priv = dev_get_priv(dev);
	int ret;

	priv->soc_id = (enum meson_soc_id)dev_get_driver_data(dev);

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

#if CONFIG_IS_ENABLED(POWER_DOMAIN)
	ret = power_domain_get(dev, &priv->pwrdm);
	if (ret < 0 && ret != -ENODEV && ret != -ENOENT) {
		pr_err("failed to get power domain : %d\n", ret);
		return ret;
	}

	if (ret != -ENODEV && ret != -ENOENT) {
		ret = power_domain_on(&priv->pwrdm);
		if (ret < 0) {
			pr_err("failed to enable power domain\n");
			return ret;
		}
	}
#endif

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;
#endif

	return 0;
}

static const struct udevice_id meson_g12a_usb2_phy_ids[] = {
	{
		.compatible = "amlogic,g12a-usb2-phy",
		.data = (ulong)MESON_SOC_G12A,
	},
	{
		.compatible = "amlogic,a1-usb2-phy",
		.data = (ulong)MESON_SOC_A1,
	},
	{ }
};

U_BOOT_DRIVER(meson_g12a_usb2_phy) = {
	.name = "meson_g12a_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = meson_g12a_usb2_phy_ids,
	.probe = meson_g12a_usb2_phy_probe,
	.ops = &meson_g12a_usb2_phy_ops,
	.priv_auto	= sizeof(struct phy_meson_g12a_usb2_priv),
};
