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
#include <dm.h>
#include <sata.h>

#ifndef CONFIG_AHCI
struct blk_desc sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];
#endif

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

#ifndef CONFIG_AHCI
#ifdef CONFIG_PARTITIONS
struct blk_desc *sata_get_dev(int dev)
{
	return (dev < CONFIG_SYS_SATA_MAX_DEVICE) ? &sata_dev_desc[dev] : NULL;
}
#endif
#endif

#ifdef CONFIG_BLK
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
#else
static unsigned long sata_bread(struct blk_desc *block_dev, lbaint_t start,
				lbaint_t blkcnt, void *dst)
{
	return sata_read(block_dev->devnum, start, blkcnt, dst);
}

static unsigned long sata_bwrite(struct blk_desc *block_dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	return sata_write(block_dev->devnum, start, blkcnt, buffer);
}
#endif

#ifndef CONFIG_AHCI
int __sata_initialize(void)
{
	int rc, ret = -1;
	int i;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
		memset(&sata_dev_desc[i], 0, sizeof(struct blk_desc));
		sata_dev_desc[i].if_type = IF_TYPE_SATA;
		sata_dev_desc[i].devnum = i;
		sata_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		sata_dev_desc[i].type = DEV_TYPE_HARDDISK;
		sata_dev_desc[i].lba = 0;
		sata_dev_desc[i].blksz = 512;
		sata_dev_desc[i].log2blksz = LOG2(sata_dev_desc[i].blksz);
#ifndef CONFIG_BLK
		sata_dev_desc[i].block_read = sata_bread;
		sata_dev_desc[i].block_write = sata_bwrite;
#endif
		rc = init_sata(i);
		if (!rc) {
			rc = scan_sata(i);
			if (!rc && sata_dev_desc[i].lba > 0 &&
			    sata_dev_desc[i].blksz > 0) {
				part_init(&sata_dev_desc[i]);
				ret = i;
			}
		}
	}

	return ret;
}
int sata_initialize(void) __attribute__((weak, alias("__sata_initialize")));

__weak int __sata_stop(void)
{
	int i, err = 0;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++)
		err |= reset_sata(i);

	if (err)
		printf("Could not reset some SATA devices\n");

	return err;
}
int sata_stop(void) __attribute__((weak, alias("__sata_stop")));
#endif

#ifdef CONFIG_BLK
static const struct blk_ops sata_blk_ops = {
	.read	= sata_bread,
	.write	= sata_bwrite,
};

U_BOOT_DRIVER(sata_blk) = {
	.name		= "sata_blk",
	.id		= UCLASS_BLK,
	.ops		= &sata_blk_ops,
};
#else
U_BOOT_LEGACY_BLK(sata) = {
	.if_typename	= "sata",
	.if_type	= IF_TYPE_SATA,
	.max_devs	= CONFIG_SYS_SATA_MAX_DEVICE,
	.desc		= sata_dev_desc,
};
#endif
