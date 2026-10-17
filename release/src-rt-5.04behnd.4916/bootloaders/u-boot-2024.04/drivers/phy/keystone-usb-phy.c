// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - https://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/device.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <asm/arch/psc_defs.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>

/* USB PHY control register offsets */
#define USB_PHY_CTL_UTMI		0x0000
#define USB_PHY_CTL_PIPE		0x0004
#define USB_PHY_CTL_PARAM_1		0x0008
#define USB_PHY_CTL_PARAM_2		0x000c
#define USB_PHY_CTL_CLOCK		0x0010
#define USB_PHY_CTL_PLL			0x0014

#define PHY_OTG_VBUSVLDECTSEL		BIT(16)
#define PHY_REF_SSP_EN			BIT(29)

struct keystone_usb_phy {
	u32 psc_domain;
	void __iomem *reg;
};

static int keystone_usb_init(struct phy *phy)
{
	u32 val;
	int rc;
	struct udevice *dev = phy->dev;
	struct keystone_usb_phy *keystone = dev_get_priv(dev);

	/* Release USB from reset */
	rc = psc_enable_module(keystone->psc_domain);
	if (rc) {
		debug("Cannot enable USB module");
		return -rc;
	}
	mdelay(10);

	/*
	 * VBUSVLDEXTSEL has a default value of 1 in BootCfg but shouldn't.
	 * It should always be cleared because our USB PHY has an onchip VBUS
	 * analog comparator.
	 */
	val = readl(keystone->reg + USB_PHY_CTL_CLOCK);
	/* quit selecting the vbusvldextsel by default! */
	val &= ~PHY_OTG_VBUSVLDECTSEL;
	writel(val, keystone->reg + USB_PHY_CTL_CLOCK);

	return 0;
}

static int keystone_usb_power_on(struct phy *phy)
{
	u32 val;
	struct udevice *dev = phy->dev;
	struct keystone_usb_phy *keystone = dev_get_priv(dev);

	val = readl(keystone->reg + USB_PHY_CTL_CLOCK);
	val |= PHY_REF_SSP_EN;
	writel(val, keystone->reg + USB_PHY_CTL_CLOCK);

	return 0;
}

static int keystone_usb_power_off(struct phy *phy)
{
	u32 val;
	struct udevice *dev = phy->dev;
	struct keystone_usb_phy *keystone = dev_get_priv(dev);

	val = readl(keystone->reg + USB_PHY_CTL_CLOCK);
	val &= ~PHY_REF_SSP_EN;
	writel(val, keystone->reg + USB_PHY_CTL_CLOCK);

	return 0;
}

static int keystone_usb_exit(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct keystone_usb_phy *keystone = dev_get_priv(dev);

	if (psc_disable_module(keystone->psc_domain))
		debug("failed to disable USB module!\n");

	return 0;
}

static int keystone_usb_phy_probe(struct udevice *dev)
{
	int rc;
	struct keystone_usb_phy *keystone = dev_get_priv(dev);

	rc = dev_read_u32(dev, "psc-domain", &keystone->psc_domain);
	if (rc)
		return rc;

	keystone->reg = dev_remap_addr_index(dev, 0);
	if (!keystone->reg) {
		pr_err("unable to remap usb phy\n");
		return -EINVAL;
	}
	return 0;
}

static const struct udevice_id keystone_usb_phy_ids[] = {
	{ .compatible = "ti,keystone-usbphy" },
	{ }
};

static struct phy_ops keystone_usb_phy_ops = {
	.init = keystone_usb_init,
	.power_on = keystone_usb_power_on,
	.power_off = keystone_usb_power_off,
	.exit = keystone_usb_exit,
};

U_BOOT_DRIVER(keystone_usb_phy) = {
	.name	= "keystone_usb_phy",
	.id	= UCLASS_PHY,
	.of_match = keystone_usb_phy_ids,
	.ops = &keystone_usb_phy_ops,
	.probe = keystone_usb_phy_probe,
	.priv_auto	= sizeof(struct keystone_usb_phy),
};
