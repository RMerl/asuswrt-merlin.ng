// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 * Based on Linux driver
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/delay.h>

struct ipq4019_usb_phy {
	phys_addr_t			base;
	struct reset_ctl	por_rst;
	struct reset_ctl	srif_rst;
};

static int ipq4019_ss_phy_power_off(struct phy *_phy)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(_phy->dev);

	reset_assert(&phy->por_rst);
	mdelay(10);

	return 0;
}

static int ipq4019_ss_phy_power_on(struct phy *_phy)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(_phy->dev);

	ipq4019_ss_phy_power_off(_phy);

	reset_deassert(&phy->por_rst);

	return 0;
}

static struct phy_ops ipq4019_usb_ss_phy_ops = {
	.power_on = ipq4019_ss_phy_power_on,
	.power_off = ipq4019_ss_phy_power_off,
};

static int ipq4019_usb_ss_phy_probe(struct udevice *dev)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(dev);
	int ret;

	phy->base = dev_read_addr(dev);
	if (phy->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = reset_get_by_name(dev, "por_rst", &phy->por_rst);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id ipq4019_usb_ss_phy_ids[] = {
	{ .compatible = "qcom,usb-ss-ipq4019-phy" },
	{ }
};

U_BOOT_DRIVER(ipq4019_usb_ss_phy) = {
	.name		= "ipq4019-usb-ss-phy",
	.id		= UCLASS_PHY,
	.of_match	= ipq4019_usb_ss_phy_ids,
	.ops		= &ipq4019_usb_ss_phy_ops,
	.probe		= ipq4019_usb_ss_phy_probe,
	.priv_auto	= sizeof(struct ipq4019_usb_phy),
};

static int ipq4019_hs_phy_power_off(struct phy *_phy)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(_phy->dev);

	reset_assert(&phy->por_rst);
	mdelay(10);

	reset_assert(&phy->srif_rst);
	mdelay(10);

	return 0;
}

static int ipq4019_hs_phy_power_on(struct phy *_phy)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(_phy->dev);

	ipq4019_hs_phy_power_off(_phy);

	reset_deassert(&phy->srif_rst);
	mdelay(10);

	reset_deassert(&phy->por_rst);

	return 0;
}

static struct phy_ops ipq4019_usb_hs_phy_ops = {
	.power_on = ipq4019_hs_phy_power_on,
	.power_off = ipq4019_hs_phy_power_off,
};

static int ipq4019_usb_hs_phy_probe(struct udevice *dev)
{
	struct ipq4019_usb_phy *phy = dev_get_priv(dev);
	int ret;

	phy->base = dev_read_addr(dev);
	if (phy->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = reset_get_by_name(dev, "por_rst", &phy->por_rst);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "srif_rst", &phy->srif_rst);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id ipq4019_usb_hs_phy_ids[] = {
	{ .compatible = "qcom,usb-hs-ipq4019-phy" },
	{ }
};

U_BOOT_DRIVER(ipq4019_usb_hs_phy) = {
	.name		= "ipq4019-usb-hs-phy",
	.id		= UCLASS_PHY,
	.of_match	= ipq4019_usb_hs_phy_ids,
	.ops		= &ipq4019_usb_hs_phy_ops,
	.probe		= ipq4019_usb_hs_phy_probe,
	.priv_auto	= sizeof(struct ipq4019_usb_phy),
};
