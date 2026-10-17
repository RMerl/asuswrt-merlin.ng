// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/pci.h>

static int _pci_x86_read_config(const struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	return pci_x86_read_config(bdf, offset, valuep, size);
}

static int _pci_x86_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value, enum pci_size_t size)
{
	return pci_x86_write_config(bdf, offset, value, size);
}

static const struct dm_pci_ops pci_x86_ops = {
	.read_config	= _pci_x86_read_config,
	.write_config	= _pci_x86_write_config,
};

static const struct udevice_id pci_x86_ids[] = {
	{ .compatible = "pci-x86" },
	{ }
};

U_BOOT_DRIVER(pci_x86) = {
	.name	= "pci_x86",
	.id	= UCLASS_PCI,
	.of_match = pci_x86_ids,
	.ops	= &pci_x86_ops,
};
