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
#include "ohci.h"

struct npcm_ohci_priv {
	ohci_t ohci;
	struct phy phy;
};

static int npcm_ohci_setup_phy(struct udevice *dev, struct phy *phy)
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

static int npcm_ohci_init(struct udevice *dev)
{
	struct npcm_ohci_priv *priv = dev_get_priv(dev);
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
	ret = npcm_ohci_setup_phy(dev, &priv->phy);
	if (ret)
		return ret;

	/* release controller from reset */
	if (reset_valid(&reset))
		reset_deassert(&reset);

	return 0;
}

static int npcm_ohci_probe(struct udevice *dev)
{
	struct ohci_regs *regs = dev_read_addr_ptr(dev);
	int ret;

	ret = npcm_ohci_init(dev);
	if (ret)
		return ret;

	return ohci_register(dev, regs);
}

static int npcm_ohci_remove(struct udevice *dev)
{
	struct npcm_ohci_priv *priv = dev_get_priv(dev);

	generic_phy_exit(&priv->phy);

	return ohci_deregister(dev);
}

static const struct udevice_id npcm_ohci_ids[] = {
	{ .compatible = "nuvoton,npcm845-ohci" },
	{ .compatible = "nuvoton,npcm750-ohci" },
	{ }
};

U_BOOT_DRIVER(ohci_npcm) = {
	.name	= "ohci_npcm",
	.id	= UCLASS_USB,
	.of_match = npcm_ohci_ids,
	.probe = npcm_ohci_probe,
	.remove = npcm_ohci_remove,
	.ops	= &ohci_usb_ops,
	.priv_auto = sizeof(struct npcm_ohci_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
