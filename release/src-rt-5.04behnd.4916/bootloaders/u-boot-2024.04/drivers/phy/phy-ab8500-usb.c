// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2019 Stephan Gerhold */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <power/pmic.h>
#include <power/ab8500.h>

#define AB8500_USB_PHY_CTRL_REG			AB8500_USB(0x8A)
#define AB8500_BIT_PHY_CTRL_HOST_EN		BIT(0)
#define AB8500_BIT_PHY_CTRL_DEVICE_EN		BIT(1)
#define AB8500_USB_PHY_CTRL_MASK		(AB8500_BIT_PHY_CTRL_HOST_EN |\
						 AB8500_BIT_PHY_CTRL_DEVICE_EN)

static int ab8500_usb_phy_power_on(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	uint set = AB8500_BIT_PHY_CTRL_DEVICE_EN;

	if (IS_ENABLED(CONFIG_USB_MUSB_HOST))
		set = AB8500_BIT_PHY_CTRL_HOST_EN;

	return pmic_clrsetbits(dev->parent, AB8500_USB_PHY_CTRL_REG,
			       AB8500_USB_PHY_CTRL_MASK, set);
}

static int ab8500_usb_phy_power_off(struct phy *phy)
{
	struct udevice *dev = phy->dev;

	return pmic_clrsetbits(dev->parent, AB8500_USB_PHY_CTRL_REG,
			       AB8500_USB_PHY_CTRL_MASK, 0);
}

struct phy_ops ab8500_usb_phy_ops = {
	.power_on = ab8500_usb_phy_power_on,
	.power_off = ab8500_usb_phy_power_off,
};

static const struct udevice_id ab8500_usb_phy_ids[] = {
	{ .compatible = "stericsson,ab8500-usb" },
	{ }
};

U_BOOT_DRIVER(ab8500_usb_phy) = {
	.name = "ab8500_usb_phy",
	.id = UCLASS_PHY,
	.of_match = ab8500_usb_phy_ids,
	.ops = &ab8500_usb_phy_ops,
};
