// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_PHY

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <fdtdec.h>
#include <generic-phy.h>
#include <log.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <power/regulator.h>

/* USBPHYC registers */
#define STM32_USBPHYC_PLL	0x0
#define STM32_USBPHYC_MISC	0x8
#define STM32_USBPHYC_TUNE(X)	(0x10C + ((X) * 0x100))

/* STM32_USBPHYC_PLL bit fields */
#define PLLNDIV			GENMASK(6, 0)
#define PLLNDIV_SHIFT		0
#define PLLFRACIN		GENMASK(25, 10)
#define PLLFRACIN_SHIFT		10
#define PLLEN			BIT(26)
#define PLLSTRB			BIT(27)
#define PLLSTRBYP		BIT(28)
#define PLLFRACCTL		BIT(29)
#define PLLDITHEN0		BIT(30)
#define PLLDITHEN1		BIT(31)

/* STM32_USBPHYC_MISC bit fields */
#define SWITHOST		BIT(0)

/* STM32_USBPHYC_TUNE bit fields */
#define INCURREN		BIT(0)
#define INCURRINT		BIT(1)
#define LFSCAPEN		BIT(2)
#define HSDRVSLEW		BIT(3)
#define HSDRVDCCUR		BIT(4)
#define HSDRVDCLEV		BIT(5)
#define HSDRVCURINCR		BIT(6)
#define FSDRVRFADJ		BIT(7)
#define HSDRVRFRED		BIT(8)
#define HSDRVCHKITRM		GENMASK(12, 9)
#define HSDRVCHKZTRM		GENMASK(14, 13)
#define OTPCOMP			GENMASK(19, 15)
#define SQLCHCTL		GENMASK(21, 20)
#define HDRXGNEQEN		BIT(22)
#define HSRXOFF			GENMASK(24, 23)
#define HSFALLPREEM		BIT(25)
#define SHTCCTCTLPROT		BIT(26)
#define STAGSEL			BIT(27)

#define MAX_PHYS		2

/* max 100 us for PLL lock and 100 us for PHY init */
#define PLL_INIT_TIME_US	200
#define PLL_PWR_DOWN_TIME_US	5
#define PLL_FVCO		2880	 /* in MHz */
#define PLL_INFF_MIN_RATE	19200000 /* in Hz */
#define PLL_INFF_MAX_RATE	38400000 /* in Hz */

/* USBPHYC_CLK48 */
#define USBPHYC_CLK48_FREQ	48000000 /* in Hz */

enum boosting_vals {
	BOOST_1000_UA = 1000,
	BOOST_2000_UA = 2000,
};

enum dc_level_vals {
	DC_MINUS_5_TO_7_MV,
	DC_PLUS_5_TO_7_MV,
	DC_PLUS_10_TO_14_MV,
	DC_MAX,
};

enum current_trim {
	CUR_NOMINAL,
	CUR_PLUS_1_56_PCT,
	CUR_PLUS_3_12_PCT,
	CUR_PLUS_4_68_PCT,
	CUR_PLUS_6_24_PCT,
	CUR_PLUS_7_8_PCT,
	CUR_PLUS_9_36_PCT,
	CUR_PLUS_10_92_PCT,
	CUR_PLUS_12_48_PCT,
	CUR_PLUS_14_04_PCT,
	CUR_PLUS_15_6_PCT,
	CUR_PLUS_17_16_PCT,
	CUR_PLUS_19_01_PCT,
	CUR_PLUS_20_58_PCT,
	CUR_PLUS_22_16_PCT,
	CUR_PLUS_23_73_PCT,
	CUR_MAX,
};

enum impedance_trim {
	IMP_NOMINAL,
	IMP_MINUS_2_OHMS,
	IMP_MINUS_4_OMHS,
	IMP_MINUS_6_OHMS,
	IMP_MAX,
};

enum squelch_level {
	SQLCH_NOMINAL,
	SQLCH_PLUS_7_MV,
	SQLCH_MINUS_5_MV,
	SQLCH_PLUS_14_MV,
	SQLCH_MAX,
};

enum rx_offset {
	NO_RX_OFFSET,
	RX_OFFSET_PLUS_5_MV,
	RX_OFFSET_PLUS_10_MV,
	RX_OFFSET_MINUS_5_MV,
	RX_OFFSET_MAX,
};

struct pll_params {
	u8 ndiv;
	u16 frac;
};

struct stm32_usbphyc {
	fdt_addr_t base;
	struct clk clk;
	struct udevice *vdda1v1;
	struct udevice *vdda1v8;
	struct stm32_usbphyc_phy {
		struct udevice *vdd;
		struct udevice *vbus;
		bool init;
		bool powered;
	} phys[MAX_PHYS];
	int n_pll_cons;
};

static void stm32_usbphyc_get_pll_params(u32 clk_rate,
					 struct pll_params *pll_params)
{
	unsigned long long fvco, ndiv, frac;

	/*
	 *    | FVCO = INFF*2*(NDIV + FRACT/2^16 ) when DITHER_DISABLE[1] = 1
	 *    | FVCO = 2880MHz
	 *    | NDIV = integer part of input bits to set the LDF
	 *    | FRACT = fractional part of input bits to set the LDF
	 *  =>	PLLNDIV = integer part of (FVCO / (INFF*2))
	 *  =>	PLLFRACIN = fractional part of(FVCO / INFF*2) * 2^16
	 * <=>  PLLFRACIN = ((FVCO / (INFF*2)) - PLLNDIV) * 2^16
	 */
	fvco = (unsigned long long)PLL_FVCO * 1000000; /* In Hz */

	ndiv = fvco;
	do_div(ndiv, (clk_rate * 2));
	pll_params->ndiv = (u8)ndiv;

	frac = fvco * (1 << 16);
	do_div(frac, (clk_rate * 2));
	frac = frac - (ndiv * (1 << 16));
	pll_params->frac = (u16)frac;
}

static int stm32_usbphyc_pll_init(struct stm32_usbphyc *usbphyc)
{
	struct pll_params pll_params;
	u32 clk_rate = clk_get_rate(&usbphyc->clk);
	u32 usbphyc_pll;

	if ((clk_rate < PLL_INFF_MIN_RATE) || (clk_rate > PLL_INFF_MAX_RATE)) {
		log_debug("input clk freq (%dHz) out of range\n",
			  clk_rate);
		return -EINVAL;
	}

	stm32_usbphyc_get_pll_params(clk_rate, &pll_params);

	usbphyc_pll = PLLDITHEN1 | PLLDITHEN0 | PLLSTRBYP;
	usbphyc_pll |= ((pll_params.ndiv << PLLNDIV_SHIFT) & PLLNDIV);

	if (pll_params.frac) {
		usbphyc_pll |= PLLFRACCTL;
		usbphyc_pll |= ((pll_params.frac << PLLFRACIN_SHIFT)
				 & PLLFRACIN);
	}

	writel(usbphyc_pll, usbphyc->base + STM32_USBPHYC_PLL);

	log_debug("input clk freq=%dHz, ndiv=%d, frac=%d\n",
		  clk_rate, pll_params.ndiv, pll_params.frac);

	return 0;
}

static bool stm32_usbphyc_is_powered(struct stm32_usbphyc *usbphyc)
{
	int i;

	for (i = 0; i < MAX_PHYS; i++) {
		if (usbphyc->phys[i].powered)
			return true;
	}

	return false;
}

static int stm32_usbphyc_pll_enable(struct stm32_usbphyc *usbphyc)
{
	bool pllen = readl(usbphyc->base + STM32_USBPHYC_PLL) & PLLEN ?
		     true : false;
	int ret;

	/* Check if one consumer has already configured the pll */
	if (pllen && usbphyc->n_pll_cons) {
		usbphyc->n_pll_cons++;
		return 0;
	}

	if (usbphyc->vdda1v1) {
		ret = regulator_set_enable(usbphyc->vdda1v1, true);
		if (ret)
			return ret;
	}

	if (usbphyc->vdda1v8) {
		ret = regulator_set_enable(usbphyc->vdda1v8, true);
		if (ret)
			return ret;
	}

	if (pllen) {
		clrbits_le32(usbphyc->base + STM32_USBPHYC_PLL, PLLEN);
		udelay(PLL_PWR_DOWN_TIME_US);
	}

	ret = stm32_usbphyc_pll_init(usbphyc);
	if (ret)
		return ret;

	setbits_le32(usbphyc->base + STM32_USBPHYC_PLL, PLLEN);

	/* We must wait PLL_INIT_TIME_US before using PHY */
	udelay(PLL_INIT_TIME_US);

	if (!(readl(usbphyc->base + STM32_USBPHYC_PLL) & PLLEN))
		return -EIO;

	usbphyc->n_pll_cons++;

	return 0;
}

static int stm32_usbphyc_pll_disable(struct stm32_usbphyc *usbphyc)
{
	int ret;

	usbphyc->n_pll_cons--;

	/* Check if other consumer requires pllen */
	if (usbphyc->n_pll_cons)
		return 0;

	clrbits_le32(usbphyc->base + STM32_USBPHYC_PLL, PLLEN);

	/*
	 * We must wait PLL_PWR_DOWN_TIME_US before checking that PLLEN
	 * bit is still clear
	 */
	udelay(PLL_PWR_DOWN_TIME_US);

	if (readl(usbphyc->base + STM32_USBPHYC_PLL) & PLLEN)
		return -EIO;

	if (usbphyc->vdda1v1) {
		ret = regulator_set_enable(usbphyc->vdda1v1, false);
		if (ret)
			return ret;
	}

	if (usbphyc->vdda1v8) {
		ret = regulator_set_enable(usbphyc->vdda1v8, false);
		if (ret)
			return ret;
	}

	return 0;
}

static int stm32_usbphyc_phy_init(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	dev_dbg(phy->dev, "phy ID = %lu\n", phy->id);
	if (usbphyc_phy->init)
		return 0;

	ret = stm32_usbphyc_pll_enable(usbphyc);
	if (ret)
		return log_ret(ret);

	usbphyc_phy->init = true;

	return 0;
}

static int stm32_usbphyc_phy_exit(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	dev_dbg(phy->dev, "phy ID = %lu\n", phy->id);
	if (!usbphyc_phy->init)
		return 0;

	ret = stm32_usbphyc_pll_disable(usbphyc);

	usbphyc_phy->init = false;

	return log_ret(ret);
}

static int stm32_usbphyc_phy_power_on(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	dev_dbg(phy->dev, "phy ID = %lu\n", phy->id);
	if (usbphyc_phy->vdd) {
		ret = regulator_set_enable(usbphyc_phy->vdd, true);
		if (ret)
			return ret;
	}
	if (usbphyc_phy->vbus) {
		ret = regulator_set_enable(usbphyc_phy->vbus, true);
		if (ret)
			return ret;
	}

	usbphyc_phy->powered = true;

	return 0;
}

static int stm32_usbphyc_phy_power_off(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	dev_dbg(phy->dev, "phy ID = %lu\n", phy->id);
	usbphyc_phy->powered = false;

	if (stm32_usbphyc_is_powered(usbphyc))
		return 0;

	if (usbphyc_phy->vbus) {
		ret = regulator_set_enable_if_allowed(usbphyc_phy->vbus, false);
		if (ret)
			return ret;
	}
	if (usbphyc_phy->vdd) {
		ret = regulator_set_enable_if_allowed(usbphyc_phy->vdd, false);
		if (ret)
			return ret;
	}

	return 0;
}

static int stm32_usbphyc_get_regulator(ofnode node,
				       char *supply_name,
				       struct udevice **regulator)
{
	struct ofnode_phandle_args regulator_phandle;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, supply_name,
					     NULL, 0, 0,
					     &regulator_phandle);
	if (ret)
		return ret;

	ret = uclass_get_device_by_ofnode(UCLASS_REGULATOR,
					  regulator_phandle.node,
					  regulator);
	if (ret)
		return ret;

	return 0;
}

static int stm32_usbphyc_of_xlate(struct phy *phy,
				  struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -ENODEV;

	if (args->args[0] >= MAX_PHYS)
		return -ENODEV;

	phy->id = args->args[0];

	if ((phy->id == 0 && args->args_count != 1) ||
	    (phy->id == 1 && args->args_count != 2)) {
		dev_err(phy->dev, "invalid number of cells for phy port%ld\n",
			phy->id);
		return -EINVAL;
	}

	return 0;
}

static void stm32_usbphyc_tuning(struct udevice *dev, ofnode node, u32 index)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(dev);
	u32 reg = STM32_USBPHYC_TUNE(index);
	u32 otpcomp, val, tune = 0;
	int ret;

	/* Backup OTP compensation code */
	otpcomp = FIELD_GET(OTPCOMP, readl(usbphyc->base + reg));

	ret = ofnode_read_u32(node, "st,current-boost-microamp", &val);
	if (!ret && (val == BOOST_1000_UA || val == BOOST_2000_UA)) {
		val = (val == BOOST_2000_UA) ? 1 : 0;
		tune |= INCURREN | FIELD_PREP(INCURRINT, val);
	} else if (ret != -EINVAL) {
		dev_warn(dev, "phy%d: invalid st,current-boost-microamp value\n", index);
	}

	if (!ofnode_read_bool(node, "st,no-lsfs-fb-cap"))
		tune |= LFSCAPEN;

	if (ofnode_read_bool(node, "st,decrease-hs-slew-rate"))
		tune |= HSDRVSLEW;

	ret = ofnode_read_u32(node, "st,tune-hs-dc-level", &val);
	if (!ret && val < DC_MAX) {
		if (val == DC_MINUS_5_TO_7_MV) {
			tune |= HSDRVDCCUR;
		} else {
			val = (val == DC_PLUS_10_TO_14_MV) ? 1 : 0;
			tune |= HSDRVCURINCR | FIELD_PREP(HSDRVDCLEV, val);
		}
	} else if (ret != -EINVAL) {
		dev_warn(dev, "phy%d: invalid st,tune-hs-dc-level value\n", index);
	}

	if (ofnode_read_bool(node, "st,enable-fs-rftime-tuning"))
		tune |= FSDRVRFADJ;

	if (ofnode_read_bool(node, "st,enable-hs-rftime-reduction"))
		tune |= HSDRVRFRED;

	ret = ofnode_read_u32(node, "st,trim-hs-current", &val);
	if (!ret && val < CUR_MAX)
		tune |= FIELD_PREP(HSDRVCHKITRM, val);
	else if (ret != -EINVAL)
		dev_warn(dev, "phy%d: invalid st,trim-hs-current value\n", index);

	ret = ofnode_read_u32(node, "st,trim-hs-impedance", &val);
	if (!ret && val < IMP_MAX)
		tune |= FIELD_PREP(HSDRVCHKZTRM, val);
	else if (ret != -EINVAL)
		dev_warn(dev, "phy%d: invalid trim-hs-impedance value\n", index);

	ret = ofnode_read_u32(node, "st,tune-squelch-level", &val);
	if (!ret && val < SQLCH_MAX)
		tune |= FIELD_PREP(SQLCHCTL, val);
	else if (ret != -EINVAL)
		dev_warn(dev, "phy%d: invalid st,tune-squelch-level value\n", index);

	if (ofnode_read_bool(node, "st,enable-hs-rx-gain-eq"))
		tune |= HDRXGNEQEN;

	ret = ofnode_read_u32(node, "st,tune-hs-rx-offset", &val);
	if (!ret && val < RX_OFFSET_MAX)
		tune |= FIELD_PREP(HSRXOFF, val);
	else if (ret != -EINVAL)
		dev_warn(dev, "phy%d: invalid st,tune-hs-rx-offset value\n", index);

	if (ofnode_read_bool(node, "st,no-hs-ftime-ctrl"))
		tune |= HSFALLPREEM;

	if (!ofnode_read_bool(node, "st,no-lsfs-sc"))
		tune |= SHTCCTCTLPROT;

	if (ofnode_read_bool(node, "st,enable-hs-tx-staggering"))
		tune |= STAGSEL;

	/* Restore OTP compensation code */
	tune |= FIELD_PREP(OTPCOMP, otpcomp);

	writel(tune, usbphyc->base + reg);
}

static const struct phy_ops stm32_usbphyc_phy_ops = {
	.init = stm32_usbphyc_phy_init,
	.exit = stm32_usbphyc_phy_exit,
	.power_on = stm32_usbphyc_phy_power_on,
	.power_off = stm32_usbphyc_phy_power_off,
	.of_xlate = stm32_usbphyc_of_xlate,
};

static int stm32_usbphyc_bind(struct udevice *dev)
{
	int ret;

	ret = device_bind_driver_to_node(dev, "stm32-usbphyc-clk", "ck_usbo_48m",
					 dev_ofnode(dev), NULL);

	return log_ret(ret);
}

static int stm32_usbphyc_probe(struct udevice *dev)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(dev);
	struct reset_ctl reset;
	ofnode node, connector;
	int ret;

	usbphyc->base = dev_read_addr(dev);
	if (usbphyc->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Enable clock */
	ret = clk_get_by_index(dev, 0, &usbphyc->clk);
	if (ret)
		return ret;

	ret = clk_enable(&usbphyc->clk);
	if (ret)
		return ret;

	/* Reset */
	ret = reset_get_by_index(dev, 0, &reset);
	if (!ret) {
		reset_assert(&reset);
		udelay(2);
		reset_deassert(&reset);
	}

	/* get usbphyc regulator */
	ret = device_get_supply_regulator(dev, "vdda1v1-supply",
					  &usbphyc->vdda1v1);
	if (ret) {
		dev_err(dev, "Can't get vdda1v1-supply regulator\n");
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vdda1v8-supply",
					  &usbphyc->vdda1v8);
	if (ret) {
		dev_err(dev, "Can't get vdda1v8-supply regulator\n");
		return ret;
	}

	/* parse all PHY subnodes to populate regulator associated to each PHY port */
	dev_for_each_subnode(node, dev) {
		fdt_addr_t phy_id;
		struct stm32_usbphyc_phy *usbphyc_phy;

		phy_id = ofnode_read_u32_default(node, "reg", FDT_ADDR_T_NONE);
		if (phy_id >= MAX_PHYS) {
			dev_err(dev, "invalid reg value %llx for %s\n",
				(fdt64_t)phy_id, ofnode_get_name(node));
			return -ENOENT;
		}

		/* Configure phy tuning */
		stm32_usbphyc_tuning(dev, node, phy_id);

		usbphyc_phy = usbphyc->phys + phy_id;
		usbphyc_phy->init = false;
		usbphyc_phy->powered = false;
		ret = stm32_usbphyc_get_regulator(node, "phy-supply",
						  &usbphyc_phy->vdd);
		if (ret) {
			dev_err(dev, "Can't get phy-supply regulator\n");
			return ret;
		}

		usbphyc_phy->vbus = NULL;
		connector = ofnode_find_subnode(node, "connector");
		if (ofnode_valid(connector)) {
			ret = stm32_usbphyc_get_regulator(connector, "vbus-supply",
							  &usbphyc_phy->vbus);
		}
	}

	/* Check if second port has to be used for host controller */
	if (dev_read_bool(dev, "st,port2-switch-to-host"))
		setbits_le32(usbphyc->base + STM32_USBPHYC_MISC, SWITHOST);

	return 0;
}

static const struct udevice_id stm32_usbphyc_of_match[] = {
	{ .compatible = "st,stm32mp1-usbphyc", },
	{ },
};

U_BOOT_DRIVER(stm32_usb_phyc) = {
	.name = "stm32-usbphyc",
	.id = UCLASS_PHY,
	.of_match = stm32_usbphyc_of_match,
	.ops = &stm32_usbphyc_phy_ops,
	.bind = stm32_usbphyc_bind,
	.probe = stm32_usbphyc_probe,
	.priv_auto	= sizeof(struct stm32_usbphyc),
};

struct stm32_usbphyc_clk {
	bool enable;
};

static ulong stm32_usbphyc_clk48_get_rate(struct clk *clk)
{
	return USBPHYC_CLK48_FREQ;
}

static int stm32_usbphyc_clk48_enable(struct clk *clk)
{
	struct stm32_usbphyc_clk *usbphyc_clk = dev_get_priv(clk->dev);
	struct stm32_usbphyc *usbphyc;
	int ret;

	if (usbphyc_clk->enable)
		return 0;

	usbphyc = dev_get_priv(clk->dev->parent);

	/* ck_usbo_48m is generated by usbphyc PLL */
	ret = stm32_usbphyc_pll_enable(usbphyc);
	if (ret)
		return ret;

	usbphyc_clk->enable = true;

	return 0;
}

static int stm32_usbphyc_clk48_disable(struct clk *clk)
{
	struct stm32_usbphyc_clk *usbphyc_clk = dev_get_priv(clk->dev);
	struct stm32_usbphyc *usbphyc;
	int ret;

	if (!usbphyc_clk->enable)
		return 0;

	usbphyc = dev_get_priv(clk->dev->parent);

	ret = stm32_usbphyc_pll_disable(usbphyc);
	if (ret)
		return ret;

	usbphyc_clk->enable = false;

	return 0;
}

const struct clk_ops usbphyc_clk48_ops = {
	.get_rate = stm32_usbphyc_clk48_get_rate,
	.enable = stm32_usbphyc_clk48_enable,
	.disable = stm32_usbphyc_clk48_disable,
};

U_BOOT_DRIVER(stm32_usb_phyc_clk) = {
	.name = "stm32-usbphyc-clk",
	.id = UCLASS_CLK,
	.ops = &usbphyc_clk48_ops,
	.priv_auto = sizeof(struct stm32_usbphyc_clk),
};
