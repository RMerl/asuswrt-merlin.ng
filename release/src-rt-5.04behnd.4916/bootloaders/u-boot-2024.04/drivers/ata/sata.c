// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 */

#include <common.h>
#include <ahci.h>
#include <blk.h>
#include <dm.h>
#include <part.h>
#include <sata.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

int sata_reset(struct udevice *dev)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->reset)
		return -ENOSYS;

	return ops->reset(dev);
}

int sata_dm_port_status(struct udevice *dev, int port)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->port_status)
		return -ENOSYS;

	return ops->port_status(dev, port);
}

int sata_scan(struct udevice *dev)
{
	struct ahci_ops *ops = ahci_get_ops(dev);

	if (!ops->scan)
		return -ENOSYS;

	return ops->scan(dev);
}

int sata_rescan(bool verbose)
{
	int ret;
	struct udevice *dev;

	if (verbose)
		printf("Removing devices on SATA bus...\n");

	blk_unbind_all(UCLASS_AHCI);

	ret = uclass_find_first_device(UCLASS_AHCI, &dev);
	if (ret || !dev) {
		printf("Cannot find SATA device (err=%d)\n", ret);
		return -ENOENT;
	}

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret) {
		printf("Cannot remove SATA device '%s' (err=%d)\n", dev->name, ret);
		return -ENOSYS;
	}

	if (verbose)
		printf("Rescanning SATA bus for devices...\n");

	ret = uclass_probe_all(UCLASS_AHCI);

	if (ret == -ENODEV) {
		if (verbose)
			printf("No SATA block device found\n");
		return 0;
	}

	return ret;
}

static unsigned long sata_bread(struct udevice *dev, lbaint_t start,
				lbaint_t blkcnt, void *dst)
{
	return -ENOSYS;
}

static unsigned long sata_bwrite(struct udevice *dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	return -ENOSYS;
}

static const struct blk_ops sata_blk_ops = {
	.read	= sata_bread,
	.write	= sata_bwrite,
};

U_BOOT_DRIVER(sata_blk) = {
	.name		= "sata_blk",
	.id		= UCLASS_BLK,
	.ops		= &sata_blk_ops,
};
