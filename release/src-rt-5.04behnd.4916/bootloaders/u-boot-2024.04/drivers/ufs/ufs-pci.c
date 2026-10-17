// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 tinylab.org
 * Author: Bin Meng <bmeng@tinylab.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <ufs.h>
#include <dm/device_compat.h>
#include "ufs.h"

static int ufs_pci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static int ufs_pci_probe(struct udevice *dev)
{
	int err;

	err = ufshcd_probe(dev, NULL);
	if (err)
		dev_err(dev, "%s failed (ret=%d)\n", __func__, err);

	return err;
}

U_BOOT_DRIVER(ufs_pci) = {
	.name	= "ufs_pci",
	.id	= UCLASS_UFS,
	.bind	= ufs_pci_bind,
	.probe	= ufs_pci_probe,
};

static struct pci_device_id ufs_supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REDHAT, PCI_DEVICE_ID_REDHAT_UFS) },
	{},
};

U_BOOT_PCI_DEVICE(ufs_pci, ufs_supported);
