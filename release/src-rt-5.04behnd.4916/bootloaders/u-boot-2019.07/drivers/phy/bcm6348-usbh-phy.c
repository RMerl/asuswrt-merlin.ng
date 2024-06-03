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
#include <reset.h>
#include <asm/io.h>
#include <dm/device.h>

#define USBH_SETUP_PORT1_EN	BIT(0)

struct bcm6348_usbh_priv {
	void __iomem *regs;
};

static int bcm6348_usbh_init(struct phy *phy)
{
	struct bcm6348_usbh_priv *priv = dev_get_priv(phy->dev);

	writel_be(USBH_SETUP_PORT1_EN, priv->regs);

	return 0;
}

static struct phy_ops bcm6348_usbh_ops = {
	.init = bcm6348_usbh_init,
};

static const struct udevice_id bcm6348_usbh_ids[] = {
	{ .compatible = "brcm,bcm6348-usbh" },
	{ /* sentinel */ }
};

static int bcm6348_usbh_probe(struct udevice *dev)
{
	struct bcm6348_usbh_priv *priv = dev_get_priv(dev);
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

U_BOOT_DRIVER(bcm6348_usbh) = {
	.name = "bcm6348-usbh",
	.id = UCLASS_PHY,
	.of_match = bcm6348_usbh_ids,
	.ops = &bcm6348_usbh_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6348_usbh_priv),
	.probe = bcm6348_usbh_probe,
};
