// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/usb-common.c:
 *	Copyright 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright 2013 Florian Fainelli <florian@openwrt.org>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <power-domain.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device.h>

/* USBH Setup register */
#define USBH_SETUP_REG		0x00
#define USBH_SETUP_IOC		BIT(4)

/* USBH PLL Control register */
#define USBH_PLL_REG		0x04
#define USBH_PLL_SUSP_EN	BIT(27)
#define USBH_PLL_IDDQ_PWRDN	BIT(31)

/* USBH Swap Control register */
#define USBH_SWAP_REG		0x0c
#define USBH_SWAP_OHCI_DATA	BIT(0)
#define USBH_SWAP_OHCI_ENDIAN	BIT(1)
#define USBH_SWAP_EHCI_DATA	BIT(3)
#define USBH_SWAP_EHCI_ENDIAN	BIT(4)

/* USBH Sim Control register */
#define USBH_SIM_REG		0x20
#define USBH_SIM_LADDR		BIT(5)

struct bcm6318_usbh_priv {
	void __iomem *regs;
};

static int bcm6318_usbh_init(struct phy *phy)
{
	struct bcm6318_usbh_priv *priv = dev_get_priv(phy->dev);

	/* enable pll control susp */
	setbits_be32(priv->regs + USBH_PLL_REG, USBH_PLL_SUSP_EN);

	/* configure to work in native cpu endian */
	clrsetbits_be32(priv->regs + USBH_SWAP_REG,
			USBH_SWAP_EHCI_ENDIAN | USBH_SWAP_OHCI_ENDIAN,
			USBH_SWAP_EHCI_DATA | USBH_SWAP_OHCI_DATA);

	/* setup config */
	setbits_be32(priv->regs + USBH_SETUP_REG, USBH_SETUP_IOC);

	/* disable pll control pwrdn */
	clrbits_be32(priv->regs + USBH_PLL_REG, USBH_PLL_IDDQ_PWRDN);

	/* sim control config */
	setbits_be32(priv->regs + USBH_SIM_REG, USBH_SIM_LADDR);

	return 0;
}

static struct phy_ops bcm6318_usbh_ops = {
	.init = bcm6318_usbh_init,
};

static const struct udevice_id bcm6318_usbh_ids[] = {
	{ .compatible = "brcm,bcm6318-usbh" },
	{ /* sentinel */ }
};

static int bcm6318_usbh_probe(struct udevice *dev)
{
	struct bcm6318_usbh_priv *priv = dev_get_priv(dev);
	struct power_domain pwr_dom;
	struct reset_ctl rst_ctl;
	struct clk clk;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	/* enable usbh clock */
	ret = clk_get_by_name(dev, "usbh", &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret < 0)
		return ret;

	ret = clk_free(&clk);
	if (ret < 0)
		return ret;

	/* enable power domain */
	ret = power_domain_get(dev, &pwr_dom);
	if (ret < 0)
		return ret;

	ret = power_domain_on(&pwr_dom);
	if (ret < 0)
		return ret;

	ret = power_domain_free(&pwr_dom);
	if (ret < 0)
		return ret;

	/* perform reset */
	ret = reset_get_by_index(dev, 0, &rst_ctl);
	if (ret < 0)
		return ret;

	ret = reset_deassert(&rst_ctl);
	if (ret < 0)
		return ret;

	ret = reset_free(&rst_ctl);
	if (ret < 0)
		return ret;

	mdelay(100);

	return 0;
}

U_BOOT_DRIVER(bcm6318_usbh) = {
	.name = "bcm6318-usbh",
	.id = UCLASS_PHY,
	.of_match = bcm6318_usbh_ids,
	.ops = &bcm6318_usbh_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6318_usbh_priv),
	.probe = bcm6318_usbh_probe,
};
