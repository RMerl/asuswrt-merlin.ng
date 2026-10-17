// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <ahci.h>
#include <scsi.h>
#include <dm.h>
#include <pci.h>

static int ahci_pci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ahci_bind_scsi(dev, &scsi_dev);
}

static int ahci_pci_probe(struct udevice *dev)
{
	return ahci_probe_scsi_pci(dev);
}

static const struct udevice_id ahci_pci_ids[] = {
	{ .compatible = "ahci-pci" },
	{ }
};

U_BOOT_DRIVER(ahci_pci) = {
	.name	= "ahci_pci",
	.id	= UCLASS_AHCI,
	.ops	= &scsi_ops,
	.of_match = ahci_pci_ids,
	.bind	= ahci_pci_bind,
	.probe = ahci_pci_probe,
};

static struct pci_device_id ahci_pci_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_STORAGE_SATA_AHCI, ~0) },
	{ PCI_DEVICE(PCI_VENDOR_ID_ASMEDIA, 0x0611) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL, 0x6121) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL, 0x6145) },
	{},
};

U_BOOT_PCI_DEVICE(ahci_pci, ahci_pci_supported);
