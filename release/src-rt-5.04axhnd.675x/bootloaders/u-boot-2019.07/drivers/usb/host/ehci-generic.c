// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 */

#include <common.h>
#include <clk.h>
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
	struct clk *clocks;
	struct reset_ctl *resets;
	struct phy phy;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *vbus_supply;
#endif
	int clock_count;
	int reset_count;
};

#ifdef CONFIG_DM_REGULATOR
static int ehci_enable_vbus_supply(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret && ret != -ENOENT)
		return ret;

	if (priv->vbus_supply) {
		ret = regulator_set_enable(priv->vbus_supply, true);
		if (ret) {
			dev_err(dev, "Error enabling VBUS supply\n");
			return ret;
		}
	} else {
		dev_dbg(dev, "No vbus supply\n");
	}

	return 0;
}

static int ehci_disable_vbus_supply(struct generic_ehci *priv)
{
	if (priv->vbus_supply)
		return regulator_set_enable(priv->vbus_supply, false);
	else
		return 0;
}
#else
static int ehci_enable_vbus_supply(struct udevice *dev)
{
	return 0;
}

static int ehci_disable_vbus_supply(struct generic_ehci *priv)
{
	return 0;
}
#endif

static int ehci_usb_probe(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int i, err, ret, clock_nb, reset_nb;

	err = 0;
	priv->clock_count = 0;
	clock_nb = ofnode_count_phandle_with_args(dev_ofnode(dev), "clocks",
						  "#clock-cells");
	if (clock_nb > 0) {
		priv->clocks = devm_kcalloc(dev, clock_nb, sizeof(struct clk),
					    GFP_KERNEL);
		if (!priv->clocks)
			return -ENOMEM;

		for (i = 0; i < clock_nb; i++) {
			err = clk_get_by_index(dev, i, &priv->clocks[i]);

			if (err < 0)
				break;
			err = clk_enable(&priv->clocks[i]);
			if (err) {
				dev_err(dev, "failed to enable clock %d\n", i);
				clk_free(&priv->clocks[i]);
				goto clk_err;
			}
			priv->clock_count++;
		}
	} else {
		if (clock_nb != -ENOENT) {
			dev_err(dev, "failed to get clock phandle(%d)\n",
				clock_nb);
			return clock_nb;
		}
	}

	priv->reset_count = 0;
	reset_nb = ofnode_count_phandle_with_args(dev_ofnode(dev), "resets",
						  "#reset-cells");
	if (reset_nb > 0) {
		priv->resets = devm_kcalloc(dev, reset_nb,
					    sizeof(struct reset_ctl),
					    GFP_KERNEL);
		if (!priv->resets)
			return -ENOMEM;

		for (i = 0; i < reset_nb; i++) {
			err = reset_get_by_index(dev, i, &priv->resets[i]);
			if (err < 0)
				break;

			if (reset_deassert(&priv->resets[i])) {
				dev_err(dev, "failed to deassert reset %d\n",
					i);
				reset_free(&priv->resets[i]);
				goto reset_err;
			}
			priv->reset_count++;
		}
	} else {
		if (reset_nb != -ENOENT) {
			dev_err(dev, "failed to get reset phandle(%d)\n",
				reset_nb);
			goto clk_err;
		}
	}

	err = ehci_enable_vbus_supply(dev);
	if (err)
		goto reset_err;

	err = ehci_setup_phy(dev, &priv->phy, 0);
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
	ret = ehci_shutdown_phy(dev, &priv->phy);
	if (ret)
		dev_err(dev, "failed to shutdown usb phy\n");

regulator_err:
	ret = ehci_disable_vbus_supply(priv);
	if (ret)
		dev_err(dev, "failed to disable VBUS supply\n");

reset_err:
	ret = reset_release_all(priv->resets, priv->reset_count);
	if (ret)
		dev_err(dev, "failed to assert all resets\n");
clk_err:
	ret = clk_release_all(priv->clocks, priv->clock_count);
	if (ret)
		dev_err(dev, "failed to disable all clocks\n");

	return err;
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct generic_ehci *priv = dev_get_priv(dev);
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	ret = ehci_shutdown_phy(dev, &priv->phy);
	if (ret)
		return ret;

	ret = ehci_disable_vbus_supply(priv);
	if (ret)
		return ret;

	ret =  reset_release_all(priv->resets, priv->reset_count);
	if (ret)
		return ret;

	return clk_release_all(priv->clocks, priv->clock_count);
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
	.priv_auto_alloc_size = sizeof(struct generic_ehci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
