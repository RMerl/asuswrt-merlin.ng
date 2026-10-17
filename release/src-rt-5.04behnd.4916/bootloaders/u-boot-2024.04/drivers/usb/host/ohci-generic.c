// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/ofnode.h>
#include <generic-phy.h>
#include <reset.h>
#include "ohci.h"

struct generic_ohci {
	ohci_t ohci;
	struct clk_bulk clocks;	/* clock list */
	struct reset_ctl_bulk resets; /* reset list */
	struct phy phy;
};

static int ohci_usb_probe(struct udevice *dev)
{
	struct ohci_regs *regs = dev_read_addr_ptr(dev);
	struct generic_ohci *priv = dev_get_priv(dev);
	int err, ret;

	ret = clk_get_bulk(dev, &priv->clocks);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "Failed to get clocks (ret=%d)\n", ret);
		return ret;
	}

	err = clk_enable_bulk(&priv->clocks);
	if (err) {
		dev_err(dev, "Failed to enable clocks (err=%d)\n", err);
		goto clk_err;
	}

	err = reset_get_bulk(dev, &priv->resets);
	if (err && err != -ENOENT) {
		dev_err(dev, "failed to get resets (err=%d)\n", err);
		goto clk_err;
	}

	err = reset_deassert_bulk(&priv->resets);
	if (err) {
		dev_err(dev, "failed to deassert resets (err=%d)\n", err);
		goto reset_err;
	}

	err = generic_setup_phy(dev, &priv->phy, 0);
	if (err)
		goto reset_err;

	err = ohci_register(dev, regs);
	if (err)
		goto phy_err;

	return 0;

phy_err:
	ret = generic_shutdown_phy(&priv->phy);
	if (ret)
		dev_err(dev, "failed to shutdown usb phy\n");

reset_err:
	ret = reset_release_bulk(&priv->resets);
	if (ret)
		dev_err(dev, "failed to release resets (ret=%d)\n", ret);
clk_err:
	ret = clk_release_bulk(&priv->clocks);
	if (ret)
		dev_err(dev, "failed to release clocks (ret=%d)\n", ret);

	return err;
}

static int ohci_usb_remove(struct udevice *dev)
{
	struct generic_ohci *priv = dev_get_priv(dev);
	int ret;

	ret = ohci_deregister(dev);
	if (ret)
		return ret;

	ret = generic_shutdown_phy(&priv->phy);
	if (ret)
		return ret;

	ret = reset_release_bulk(&priv->resets);
	if (ret)
		return ret;

	return clk_release_bulk(&priv->clocks);
}

static const struct udevice_id ohci_usb_ids[] = {
	{ .compatible = "generic-ohci" },
	{ }
};

U_BOOT_DRIVER(ohci_generic) = {
	.name	= "ohci_generic",
	.id	= UCLASS_USB,
	.of_match = ohci_usb_ids,
	.probe = ohci_usb_probe,
	.remove = ohci_usb_remove,
	.ops	= &ohci_usb_ops,
	.priv_auto	= sizeof(struct generic_ohci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
