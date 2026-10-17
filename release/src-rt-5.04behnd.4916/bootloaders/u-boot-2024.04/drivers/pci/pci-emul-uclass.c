// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <linux/libfdt.h>
#include <pci.h>
#include <dm/lists.h>

struct sandbox_pci_emul_priv {
	int dev_count;
};

int sandbox_pci_get_emul(const struct udevice *bus, pci_dev_t find_devfn,
			 struct udevice **containerp, struct udevice **emulp)
{
	struct pci_emul_uc_priv *upriv;
	struct udevice *dev;
	int ret;

	*containerp = NULL;
	ret = pci_bus_find_devfn(bus, PCI_MASK_BUS(find_devfn), &dev);
	if (ret) {
		debug("%s: Could not find emulator for dev %x\n", __func__,
		      find_devfn);
		return ret;
	}
	*containerp = dev;

	ret = uclass_get_device_by_phandle(UCLASS_PCI_EMUL, dev, "sandbox,emul",
					   emulp);
	if (!ret) {
		upriv = dev_get_uclass_priv(*emulp);

		upriv->client = dev;
	} else if (device_get_uclass_id(dev) != UCLASS_PCI_GENERIC) {
		/*
		 * See commit 4345998ae9df,
		 * "pci: sandbox: Support dynamically binding device driver"
		 */
		*emulp = dev;
	}

	return 0;
}

int sandbox_pci_get_client(struct udevice *emul, struct udevice **devp)
{
	struct pci_emul_uc_priv *upriv = dev_get_uclass_priv(emul);

	if (!upriv->client)
		return -ENOENT;
	*devp = upriv->client;

	return 0;
}

uint sandbox_pci_read_bar(u32 barval, int type, uint size)
{
	u32 result;

	result = barval;
	if (result == 0xffffffff) {
		if (type == PCI_BASE_ADDRESS_SPACE_IO) {
			result = (~(size - 1) &
				PCI_BASE_ADDRESS_IO_MASK) |
				PCI_BASE_ADDRESS_SPACE_IO;
		} else {
			result = (~(size - 1) &
				PCI_BASE_ADDRESS_MEM_MASK) |
				PCI_BASE_ADDRESS_MEM_TYPE_32;
		}
	}

	return result;
}

static int sandbox_pci_emul_post_probe(struct udevice *dev)
{
	struct sandbox_pci_emul_priv *priv = uclass_get_priv(dev->uclass);

	priv->dev_count++;
	sandbox_set_enable_pci_map(true);

	return 0;
}

static int sandbox_pci_emul_pre_remove(struct udevice *dev)
{
	struct sandbox_pci_emul_priv *priv = uclass_get_priv(dev->uclass);

	priv->dev_count--;
	sandbox_set_enable_pci_map(priv->dev_count > 0);

	return 0;
}

UCLASS_DRIVER(pci_emul) = {
	.id		= UCLASS_PCI_EMUL,
	.name		= "pci_emul",
	.post_probe	= sandbox_pci_emul_post_probe,
	.pre_remove	= sandbox_pci_emul_pre_remove,
	.priv_auto	= sizeof(struct sandbox_pci_emul_priv),
	.per_device_auto	= sizeof(struct pci_emul_uc_priv),
};

/*
 * This uclass is a child of the pci bus. Its plat is not defined here so
 * is defined by its parent, UCLASS_PCI, which uses struct pci_child_plat.
 * See per_child_plat_auto	in UCLASS_DRIVER(pci).
 */
UCLASS_DRIVER(pci_emul_parent) = {
	.id		= UCLASS_PCI_EMUL_PARENT,
	.name		= "pci_emul_parent",
	.post_bind	= dm_scan_fdt_dev,
};

static const struct udevice_id pci_emul_parent_ids[] = {
	{ .compatible = "sandbox,pci-emul-parent" },
	{ }
};

U_BOOT_DRIVER(pci_emul_parent_drv) = {
	.name		= "pci_emul_parent_drv",
	.id		= UCLASS_PCI_EMUL_PARENT,
	.of_match	= pci_emul_parent_ids,
};
