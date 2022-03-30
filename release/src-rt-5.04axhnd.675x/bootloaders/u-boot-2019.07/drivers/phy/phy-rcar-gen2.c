// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas RCar Gen2 USB PHY driver
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

#define USBHS_LPSTS			0x02
#define USBHS_UGCTRL			0x80
#define USBHS_UGCTRL2			0x84
#define USBHS_UGSTS			0x88	/* From technical update */

/* Low Power Status register (LPSTS) */
#define USBHS_LPSTS_SUSPM		0x4000

/* USB General control register (UGCTRL) */
#define USBHS_UGCTRL_CONNECT		BIT(2)
#define USBHS_UGCTRL_PLLRESET		BIT(0)

/* USB General control register 2 (UGCTRL2) */
#define USBHS_UGCTRL2_USB2SEL		0x80000000
#define USBHS_UGCTRL2_USB2SEL_PCI	0x00000000
#define USBHS_UGCTRL2_USB2SEL_USB30	0x80000000
#define USBHS_UGCTRL2_USB0SEL		0x00000030
#define USBHS_UGCTRL2_USB0SEL_PCI	0x00000010
#define USBHS_UGCTRL2_USB0SEL_HS_USB	0x00000030

/* USB General status register (UGSTS) */
#define USBHS_UGSTS_LOCK		0x00000100 /* From technical update */

#define PHYS_PER_CHANNEL	2

struct rcar_gen2_phy {
	fdt_addr_t	regs;
	struct clk	clk;
};

static int rcar_gen2_phy_phy_init(struct phy *phy)
{
	struct rcar_gen2_phy *priv = dev_get_priv(phy->dev);
	u16 chan = phy->id & 0xffff;
	u16 mode = (phy->id >> 16) & 0xffff;
	u32 clrmask, setmask;

	if (chan == 0) {
		clrmask = USBHS_UGCTRL2_USB0SEL;
		setmask = mode ? USBHS_UGCTRL2_USB0SEL_HS_USB :
				 USBHS_UGCTRL2_USB0SEL_PCI;
	} else {
		clrmask = USBHS_UGCTRL2_USB2SEL;
		setmask = mode ? USBHS_UGCTRL2_USB2SEL_USB30 :
				 USBHS_UGCTRL2_USB2SEL_PCI;
	}
	clrsetbits_le32(priv->regs + USBHS_UGCTRL2, clrmask, setmask);

	return 0;
}

static int rcar_gen2_phy_phy_power_on(struct phy *phy)
{
	struct rcar_gen2_phy *priv = dev_get_priv(phy->dev);
	int i;
	u32 value;

	/* Power on USBHS PHY */
	clrbits_le32(priv->regs + USBHS_UGCTRL, USBHS_UGCTRL_PLLRESET);

	setbits_le16(priv->regs + USBHS_LPSTS, USBHS_LPSTS_SUSPM);

	for (i = 0; i < 20; i++) {
		value = readl(priv->regs + USBHS_UGSTS);
		if ((value & USBHS_UGSTS_LOCK) == USBHS_UGSTS_LOCK) {
			setbits_le32(priv->regs + USBHS_UGCTRL,
				     USBHS_UGCTRL_CONNECT);
			return 0;
		}
		udelay(1);
	}

	return -ETIMEDOUT;
}

static int rcar_gen2_phy_phy_power_off(struct phy *phy)
{
	struct rcar_gen2_phy *priv = dev_get_priv(phy->dev);

	/* Power off USBHS PHY */
	clrbits_le32(priv->regs + USBHS_UGCTRL, USBHS_UGCTRL_CONNECT);

	clrbits_le16(priv->regs + USBHS_LPSTS, USBHS_LPSTS_SUSPM);

	setbits_le32(priv->regs + USBHS_UGCTRL, USBHS_UGCTRL_PLLRESET);

	return 0;
}

static int rcar_gen2_phy_of_xlate(struct phy *phy,
				  struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		dev_err(phy->dev, "Invalid DT PHY argument count: %d\n",
			args->args_count);
		return -EINVAL;
	}

	if (args->args[0] != 0 && args->args[0] != 2) {
		dev_err(phy->dev, "Invalid DT PHY channel: %d\n",
			args->args[0]);
		return -EINVAL;
	}

	if (args->args[1] != 0 && args->args[1] != 1) {
		dev_err(phy->dev, "Invalid DT PHY mode: %d\n",
			args->args[1]);
		return -EINVAL;
	}

	if (args->args_count)
		phy->id = args->args[0] | (args->args[1] << 16);
	else
		phy->id = 0;

	return 0;
}

static const struct phy_ops rcar_gen2_phy_phy_ops = {
	.init		= rcar_gen2_phy_phy_init,
	.power_on	= rcar_gen2_phy_phy_power_on,
	.power_off	= rcar_gen2_phy_phy_power_off,
	.of_xlate	= rcar_gen2_phy_of_xlate,
};

static int rcar_gen2_phy_probe(struct udevice *dev)
{
	struct rcar_gen2_phy *priv = dev_get_priv(dev);
	int ret;

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Enable clock */
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	return 0;
}

static int rcar_gen2_phy_remove(struct udevice *dev)
{
	struct rcar_gen2_phy *priv = dev_get_priv(dev);

	clk_disable(&priv->clk);
	clk_free(&priv->clk);

	return 0;
}

static const struct udevice_id rcar_gen2_phy_of_match[] = {
	{ .compatible = "renesas,rcar-gen2-usb-phy", },
	{ },
};

U_BOOT_DRIVER(rcar_gen2_phy) = {
	.name		= "rcar-gen2-phy",
	.id		= UCLASS_PHY,
	.of_match	= rcar_gen2_phy_of_match,
	.ops		= &rcar_gen2_phy_phy_ops,
	.probe		= rcar_gen2_phy_probe,
	.remove		= rcar_gen2_phy_remove,
	.priv_auto_alloc_size = sizeof(struct rcar_gen2_phy),
};
