// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas RCar Gen3 USB PHY driver
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
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

/* USB2.0 Host registers (original offset is +0x200) */
#define USB2_INT_ENABLE		0x000
#define USB2_USBCTR		0x00c
#define USB2_SPD_RSM_TIMSET	0x10c
#define USB2_OC_TIMSET		0x110
#define USB2_COMMCTRL		0x600
#define USB2_OBINTSTA		0x604
#define USB2_OBINTEN		0x608
#define USB2_VBCTRL		0x60c
#define USB2_LINECTRL1		0x610
#define USB2_ADPCTRL		0x630

/* USBCTR */
#define USB2_USBCTR_PLL_RST	BIT(1)

/* SPD_RSM_TIMSET */
#define USB2_SPD_RSM_TIMSET_INIT	0x014e029b

/* OC_TIMSET */
#define USB2_OC_TIMSET_INIT		0x000209ab

/* COMMCTRL */
#define USB2_COMMCTRL_OTG_PERI		BIT(31)	/* 1 = Peripheral mode */

/* LINECTRL1 */
#define USB2_LINECTRL1_DP_RPD		BIT(18)
#define USB2_LINECTRL1_DM_RPD		BIT(16)

/* ADPCTRL */
#define USB2_ADPCTRL_DRVVBUS		BIT(4)

struct rcar_gen3_phy {
	fdt_addr_t	regs;
	struct clk	clk;
	struct udevice	*vbus_supply;
};

static int rcar_gen3_phy_phy_init(struct phy *phy)
{
	struct rcar_gen3_phy *priv = dev_get_priv(phy->dev);

	/* Initialize USB2 part */
	writel(0, priv->regs + USB2_INT_ENABLE);
	writel(USB2_SPD_RSM_TIMSET_INIT, priv->regs + USB2_SPD_RSM_TIMSET);
	writel(USB2_OC_TIMSET_INIT, priv->regs + USB2_OC_TIMSET);

	setbits_le32(priv->regs + USB2_LINECTRL1,
		     USB2_LINECTRL1_DP_RPD | USB2_LINECTRL1_DM_RPD);

	clrbits_le32(priv->regs + USB2_COMMCTRL, USB2_COMMCTRL_OTG_PERI);

	setbits_le32(priv->regs + USB2_ADPCTRL, USB2_ADPCTRL_DRVVBUS);

	return 0;
}

static int rcar_gen3_phy_phy_power_on(struct phy *phy)
{
	struct rcar_gen3_phy *priv = dev_get_priv(phy->dev);
	int ret;

	if (priv->vbus_supply) {
		ret = regulator_set_enable(priv->vbus_supply, true);
		if (ret)
			return ret;
	}

	setbits_le32(priv->regs + USB2_USBCTR, USB2_USBCTR_PLL_RST);
	clrbits_le32(priv->regs + USB2_USBCTR, USB2_USBCTR_PLL_RST);

	return 0;
}

static int rcar_gen3_phy_phy_power_off(struct phy *phy)
{
	struct rcar_gen3_phy *priv = dev_get_priv(phy->dev);

	if (!priv->vbus_supply)
		return 0;

	return regulator_set_enable(priv->vbus_supply, false);
}

static const struct phy_ops rcar_gen3_phy_phy_ops = {
	.init		= rcar_gen3_phy_phy_init,
	.power_on	= rcar_gen3_phy_phy_power_on,
	.power_off	= rcar_gen3_phy_phy_power_off,
};

static int rcar_gen3_phy_probe(struct udevice *dev)
{
	struct rcar_gen3_phy *priv = dev_get_priv(dev);
	int ret;

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret && ret != -ENOENT) {
		pr_err("Failed to get PHY regulator\n");
		return ret;
	}

	/* Enable clock */
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	return 0;
}

static int rcar_gen3_phy_remove(struct udevice *dev)
{
	struct rcar_gen3_phy *priv = dev_get_priv(dev);

	clk_disable(&priv->clk);
	clk_free(&priv->clk);

	return 0;
}

static const struct udevice_id rcar_gen3_phy_of_match[] = {
	{ .compatible = "renesas,rcar-gen3-usb2-phy", },
	{ },
};

U_BOOT_DRIVER(rcar_gen3_phy) = {
	.name		= "rcar-gen3-phy",
	.id		= UCLASS_PHY,
	.of_match	= rcar_gen3_phy_of_match,
	.ops		= &rcar_gen3_phy_phy_ops,
	.probe		= rcar_gen3_phy_probe,
	.remove		= rcar_gen3_phy_remove,
	.priv_auto_alloc_size = sizeof(struct rcar_gen3_phy),
};
