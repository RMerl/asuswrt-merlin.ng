// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <pci.h>
#include "nvme.h"

static int nvme_bind(struct udevice *udev)
{
	static int ndev_num;
	char name[20];

	sprintf(name, "nvme#%d", ndev_num++);

	return device_set_name(udev, name);
}

static int nvme_probe(struct udevice *udev)
{
	struct nvme_dev *ndev = dev_get_priv(udev);
	struct pci_child_plat *pplat;

	pplat = dev_get_parent_plat(udev);
	sprintf(ndev->vendor, "0x%.4x", pplat->vendor);

	ndev->instance = trailing_strtol(udev->name);
	ndev->bar = dm_pci_map_bar(udev, PCI_BASE_ADDRESS_0, 0, 0,
				   PCI_REGION_TYPE, PCI_REGION_MEM);

	/* Turn on bus-mastering */
	dm_pci_clrset_config16(udev, PCI_COMMAND, 0, PCI_COMMAND_MASTER);

	return nvme_init(udev);
}

U_BOOT_DRIVER(nvme) = {
	.name	= "nvme",
	.id	= UCLASS_NVME,
	.bind	= nvme_bind,
	.probe	= nvme_probe,
	.priv_auto	= sizeof(struct nvme_dev),
};

struct pci_device_id nvme_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_STORAGE_EXPRESS, ~0) },
	{}
};

U_BOOT_PCI_DEVICE(nvme, nvme_supported);
