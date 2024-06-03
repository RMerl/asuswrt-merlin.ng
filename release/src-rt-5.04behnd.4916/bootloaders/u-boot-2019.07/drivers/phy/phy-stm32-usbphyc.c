// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <fdtdec.h>
#include <generic-phy.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <power/regulator.h>

/* USBPHYC registers */
#define STM32_USBPHYC_PLL	0x0
#define STM32_USBPHYC_MISC	0x8

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

#define MAX_PHYS		2

/* max 100 us for PLL lock and 100 us for PHY init */
#define PLL_INIT_TIME_US	200
#define PLL_PWR_DOWN_TIME_US	5
#define PLL_FVCO		2880	 /* in MHz */
#define PLL_INFF_MIN_RATE	19200000 /* in Hz */
#define PLL_INFF_MAX_RATE	38400000 /* in Hz */

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
		bool init;
		bool powered;
	} phys[MAX_PHYS];
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
		pr_debug("%s: input clk freq (%dHz) out of range\n",
			 __func__, clk_rate);
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

	pr_debug("%s: input clk freq=%dHz, ndiv=%d, frac=%d\n", __func__,
		 clk_rate, pll_params.ndiv, pll_params.frac);

	return 0;
}

static bool stm32_usbphyc_is_init(struct stm32_usbphyc *usbphyc)
{
	int i;

	for (i = 0; i < MAX_PHYS; i++) {
		if (usbphyc->phys[i].init)
			return true;
	}

	return false;
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

static int stm32_usbphyc_phy_init(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	bool pllen = readl(usbphyc->base + STM32_USBPHYC_PLL) & PLLEN ?
		     true : false;
	int ret;

	pr_debug("%s phy ID = %lu\n", __func__, phy->id);
	/* Check if one phy port has already configured the pll */
	if (pllen && stm32_usbphyc_is_init(usbphyc))
		goto initialized;

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

initialized:
	usbphyc_phy->init = true;

	return 0;
}

static int stm32_usbphyc_phy_exit(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	pr_debug("%s phy ID = %lu\n", __func__, phy->id);
	usbphyc_phy->init = false;

	/* Check if other phy port requires pllen */
	if (stm32_usbphyc_is_init(usbphyc))
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

static int stm32_usbphyc_phy_power_on(struct phy *phy)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(phy->dev);
	struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + phy->id;
	int ret;

	pr_debug("%s phy ID = %lu\n", __func__, phy->id);
	if (usbphyc_phy->vdd) {
		ret = regulator_set_enable(usbphyc_phy->vdd, true);
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

	pr_debug("%s phy ID = %lu\n", __func__, phy->id);
	usbphyc_phy->powered = false;

	if (stm32_usbphyc_is_powered(usbphyc))
		return 0;

	if (usbphyc_phy->vdd) {
		ret = regulator_set_enable(usbphyc_phy->vdd, false);
		if (ret)
			return ret;
	}

	return 0;
}

static int stm32_usbphyc_get_regulator(struct udevice *dev, ofnode node,
				       char *supply_name,
				       struct udevice **regulator)
{
	struct ofnode_phandle_args regulator_phandle;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, supply_name,
					     NULL, 0, 0,
					     &regulator_phandle);
	if (ret) {
		dev_err(dev, "Can't find %s property (%d)\n", supply_name, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_REGULATOR,
					  regulator_phandle.node,
					  regulator);

	if (ret) {
		dev_err(dev, "Can't get %s regulator (%d)\n", supply_name, ret);
		return ret;
	}

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
		dev_err(dev, "invalid number of cells for phy port%ld\n",
			phy->id);
		return -EINVAL;
	}

	return 0;
}

static const struct phy_ops stm32_usbphyc_phy_ops = {
	.init = stm32_usbphyc_phy_init,
	.exit = stm32_usbphyc_phy_exit,
	.power_on = stm32_usbphyc_phy_power_on,
	.power_off = stm32_usbphyc_phy_power_off,
	.of_xlate = stm32_usbphyc_of_xlate,
};

static int stm32_usbphyc_probe(struct udevice *dev)
{
	struct stm32_usbphyc *usbphyc = dev_get_priv(dev);
	struct reset_ctl reset;
	ofnode node;
	int i, ret;

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

	/*
	 * parse all PHY subnodes in order to populate regulator associated
	 * to each PHY port
	 */
	node = dev_read_first_subnode(dev);
	for (i = 0; i < MAX_PHYS; i++) {
		struct stm32_usbphyc_phy *usbphyc_phy = usbphyc->phys + i;

		usbphyc_phy->init = false;
		usbphyc_phy->powered = false;
		ret = stm32_usbphyc_get_regulator(dev, node, "phy-supply",
						  &usbphyc_phy->vdd);
		if (ret)
			return ret;

		node = dev_read_next_subnode(node);
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
	.probe = stm32_usbphyc_probe,
	.priv_auto_alloc_size = sizeof(struct stm32_usbphyc),
};
