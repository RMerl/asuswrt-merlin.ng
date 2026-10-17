// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 */

#include <common.h>
#include <clk.h>
#include <log.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/ofnode.h>
#include <generic-phy.h>
#include <reset.h>
#include <asm/io.h>
#include <dm.h>
#include "ehci.h"
#include <power/regulator.h>

/*
 * Even though here we don't explicitly use "struct ehci_ctrl"
 * ehci_register() expects it to be the first thing that resides in
 * device's private data.
 */
struct generic_ehci {
	struct ehci_ctrl ctrl;
	struct clk_bulk clocks;
	struct reset_ctl_bulk resets;
	struct phy phy;
	struct udevice *vbus_supply;
};

static int ehci_enable_vbus_supply(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret && ret != -ENOENT)
		return ret;

	ret = regulator_set_enable_if_allowed(priv->vbus_supply, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "Error enabling VBUS supply (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int ehci_disable_vbus_supply(struct generic_ehci *priv)
{
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vbus_supply, false);
	if (ret && ret != -ENOSYS)
		return ret;

	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int err, ret;

	err = 0;
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
		dev_err(dev, "Failed to get resets (err=%d)\n", err);
		goto clk_err;
	}

	err = reset_deassert_bulk(&priv->resets);
	if (err) {
		dev_err(dev, "Failed to get deassert resets (err=%d)\n", err);
		goto reset_err;
	}

	err = ehci_enable_vbus_supply(dev);
	if (err)
		goto reset_err;

	err = generic_setup_phy(dev, &priv->phy, 0);
	if (err)
		goto regulator_err;

	hccr = map_physmem(dev_read_addr(dev), 0x100, MAP_NOCACHE);
	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	err = ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
	if (err)
		goto phy_err;

	return 0;

phy_err:
	ret = generic_shutdown_phy(&priv->phy);
	if (ret)
		dev_err(dev, "failed to shutdown usb phy (ret=%d)\n", ret);

regulator_err:
	ret = ehci_disable_vbus_supply(priv);
	if (ret)
		dev_err(dev, "failed to disable VBUS supply (ret=%d)\n", ret);

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

static int ehci_usb_remove(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	ret = generic_shutdown_phy(&priv->phy);
	if (ret)
		return ret;

	ret = ehci_disable_vbus_supply(priv);
	if (ret)
		return ret;

	ret = reset_release_bulk(&priv->resets);
	if (ret)
		return ret;

	return clk_release_bulk(&priv->clocks);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "generic-ehci" },
	{ }
};

U_BOOT_DRIVER(ehci_generic) = {
	.name	= "ehci_generic",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto	= sizeof(struct generic_ehci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
