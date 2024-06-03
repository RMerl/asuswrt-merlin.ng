// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/usb-common.c:
 *	Copyright 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright 2013 Florian Fainelli <florian@openwrt.org>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device.h>

/* USBH Swap Control register */
#define USBH_SWAP_REG		0x00
#define USBH_SWAP_OHCI_DATA	BIT(0)
#define USBH_SWAP_OHCI_ENDIAN	BIT(1)
#define USBH_SWAP_EHCI_DATA	BIT(3)
#define USBH_SWAP_EHCI_ENDIAN	BIT(4)

/* USBH Test register */
#define USBH_TEST_REG		0x24
#define USBH_TEST_PORT_CTL	0x1c0020

struct bcm6358_usbh_priv {
	void __iomem *regs;
};

static int bcm6358_usbh_init(struct phy *phy)
{
	struct bcm6358_usbh_priv *priv = dev_get_priv(phy->dev);

	/* configure to work in native cpu endian */
	clrsetbits_be32(priv->regs + USBH_SWAP_REG,
			USBH_SWAP_EHCI_ENDIAN | USBH_SWAP_OHCI_ENDIAN,
			USBH_SWAP_EHCI_DATA | USBH_SWAP_OHCI_DATA);

	/* test port control */
	writel_be(USBH_TEST_PORT_CTL, priv->regs + USBH_TEST_REG);

	return 0;
}

static struct phy_ops bcm6358_usbh_ops = {
	.init = bcm6358_usbh_init,
};

static const struct udevice_id bcm6358_usbh_ids[] = {
	{ .compatible = "brcm,bcm6358-usbh" },
	{ /* sentinel */ }
};

static int bcm6358_usbh_probe(struct udevice *dev)
{
	struct bcm6358_usbh_priv *priv = dev_get_priv(dev);
	struct reset_ctl rst_ctl;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

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

	return 0;
}

U_BOOT_DRIVER(bcm6358_usbh) = {
	.name = "bcm6358-usbh",
	.id = UCLASS_PHY,
	.of_match = bcm6358_usbh_ids,
	.ops = &bcm6358_usbh_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6358_usbh_priv),
	.probe = bcm6358_usbh_probe,
};
