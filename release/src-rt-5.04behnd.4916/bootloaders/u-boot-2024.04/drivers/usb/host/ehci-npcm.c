// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include "ehci.h"

struct npcm_ehci_priv {
	struct ehci_ctrl ctrl;
	struct ehci_hccr *hcd;
	struct phy phy;
};

static int npcm_ehci_setup_phy(struct udevice *dev, struct phy *phy)
{
	int ret;

	if (!phy)
		return 0;

	ret = generic_phy_get_by_index(dev, 0, phy);
	if (ret) {
		if (ret != -ENOENT) {
			dev_err(dev, "failed to get usb phy\n");
			return ret;
		}
	} else {
		ret = generic_phy_init(phy);
		if (ret) {
			dev_err(dev, "failed to init usb phy\n");
			return ret;
		}
	}

	return 0;
}

static int npcm_ehci_init(struct udevice *dev)
{
	struct npcm_ehci_priv *priv = dev_get_priv(dev);
	struct reset_ctl reset;
	int ret;

	ret = reset_get_by_index(dev, 0, &reset);
	if (ret && ret != -ENOENT && ret != -ENOTSUPP) {
		dev_err(dev, "failed to get reset\n");
		return ret;
	}

	/* reset controller */
	if (reset_valid(&reset))
		reset_assert(&reset);

	/* setup phy */
	ret = npcm_ehci_setup_phy(dev, &priv->phy);
	if (ret)
		return ret;

	/* release controller from reset */
	if (reset_valid(&reset))
		reset_deassert(&reset);

	return 0;
}

static int npcm_ehci_probe(struct udevice *dev)
{
	struct npcm_ehci_priv *priv = dev_get_priv(dev);
	struct ehci_hcor *hcor;
	enum usb_init_type type = dev_get_driver_data(dev);
	int ret;

	ret = npcm_ehci_init(dev);
	if (ret)
		return ret;

	priv->hcd = (struct ehci_hccr *)dev_read_addr_ptr(dev);
	debug("USB HCD @0x%p\n", priv->hcd);
	hcor = (struct ehci_hcor *)((uintptr_t)priv->hcd +
			HC_LENGTH(ehci_readl(&priv->hcd->cr_capbase)));

	return ehci_register(dev, priv->hcd, hcor, NULL, 0, type);
}

static int npcm_ehci_remove(struct udevice *dev)
{
	struct npcm_ehci_priv *priv = dev_get_priv(dev);

	generic_phy_exit(&priv->phy);

	return ehci_deregister(dev);
}

static const struct udevice_id npcm_ehci_ids[] = {
	{ .compatible = "nuvoton,npcm845-ehci", .data = USB_INIT_HOST },
	{ .compatible = "nuvoton,npcm845-udc", .data = USB_INIT_DEVICE },
	{ .compatible = "nuvoton,npcm750-ehci", .data = USB_INIT_HOST },
	{ .compatible = "nuvoton,npcm750-udc", .data = USB_INIT_DEVICE },
	{ }
};

U_BOOT_DRIVER(ehci_npcm) = {
	.name	= "ehci_npcm",
	.id	= UCLASS_USB,
	.of_match = npcm_ehci_ids,
	.probe = npcm_ehci_probe,
	.remove = npcm_ehci_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto = sizeof(struct npcm_ehci_priv),
	.plat_auto = sizeof(struct usb_plat),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
