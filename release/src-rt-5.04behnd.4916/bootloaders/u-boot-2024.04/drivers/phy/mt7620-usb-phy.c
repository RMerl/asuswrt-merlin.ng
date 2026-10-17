// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <misc.h>
#include <reset.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <mach/mt7620-sysc.h>

struct mt7620_usb_phy {
	struct udevice *sysc;
	struct clk_bulk clocks;
	struct reset_ctl_bulk resets;
};

static int mt7620_usb_phy_power_on(struct phy *_phy)
{
	struct mt7620_usb_phy *phy = dev_get_priv(_phy->dev);
	u32 mode = MT7620_SYSC_USB_HOST_MODE;
	int ret;

	reset_deassert_bulk(&phy->resets);

	clk_enable_bulk(&phy->clocks);

	mdelay(10);

	ret = misc_ioctl(phy->sysc, MT7620_SYSC_IOCTL_SET_USB_MODE, &mode);
	if (ret) {
		dev_err(_phy->dev,
			"mt7620_usbphy: failed to set USB host mode\n");
		return ret;
	}

	mdelay(10);

	return 0;
}

static int mt7620_usb_phy_power_off(struct phy *_phy)
{
	struct mt7620_usb_phy *phy = dev_get_priv(_phy->dev);

	clk_disable_bulk(&phy->clocks);

	reset_assert_bulk(&phy->resets);

	return 0;
}

static int mt7620_usb_phy_probe(struct udevice *dev)
{
	struct mt7620_usb_phy *phy = dev_get_priv(dev);
	struct ofnode_phandle_args sysc_args;
	int ret;

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "mediatek,sysc", NULL,
					     0, 0, &sysc_args);
	if (ret) {
		dev_err(dev, "mt7620_usbphy: sysc property not found\n");
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, sysc_args.node,
					  &phy->sysc);
	if (ret) {
		dev_err(dev, "mt7620_usbphy: failed to sysc device\n");
		return ret;
	}

	ret = clk_get_bulk(dev, &phy->clocks);
	if (ret) {
		dev_err(dev, "mt7620_usbphy: failed to get clocks\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &phy->resets);
	if (ret) {
		dev_err(dev, "mt7620_usbphy: failed to get reset control\n");
		return ret;
	}

	return 0;
}

static struct phy_ops mt7620_usb_phy_ops = {
	.power_on = mt7620_usb_phy_power_on,
	.power_off = mt7620_usb_phy_power_off,
};

static const struct udevice_id mt7620_usb_phy_ids[] = {
	{ .compatible = "mediatek,mt7620-usbphy" },
	{ }
};

U_BOOT_DRIVER(mt7620_usb_phy) = {
	.name		= "mt7620_usb_phy",
	.id		= UCLASS_PHY,
	.of_match	= mt7620_usb_phy_ids,
	.ops		= &mt7620_usb_phy_ops,
	.probe		= mt7620_usb_phy_probe,
	.priv_auto	= sizeof(struct mt7620_usb_phy),
};
