// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <fdtdec.h>
#include <part.h>
#include <os.h>
#include <malloc.h>
#include <sandbox_host.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

static unsigned long host_block_read(struct udevice *dev,
				     unsigned long start, lbaint_t blkcnt,
				     void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	struct udevice *host_dev = dev_get_parent(dev);
	struct host_sb_plat *plat = dev_get_plat(host_dev);

	if (os_lseek(plat->fd, start * desc->blksz, OS_SEEK_SET) == -1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_read(plat->fd, buffer, blkcnt * desc->blksz);
	if (len >= 0)
		return len / desc->blksz;

	return -EIO;
}

static unsigned long host_block_write(struct udevice *dev,
				      unsigned long start, lbaint_t blkcnt,
				      const void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	struct udevice *host_dev = dev_get_parent(dev);
	struct host_sb_plat *plat = dev_get_plat(host_dev);

	if (os_lseek(plat->fd, start * desc->blksz, OS_SEEK_SET) == -1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_write(plat->fd, buffer, blkcnt * desc->blksz);
	if (len >= 0)
		return len / desc->blksz;

	return -EIO;
}

static const struct blk_ops sandbox_host_blk_ops = {
	.read	= host_block_read,
	.write	= host_block_write,
};

U_BOOT_DRIVER(sandbox_host_blk) = {
	.name		= "sandbox_host_blk",
	.id		= UCLASS_BLK,
	.ops		= &sandbox_host_blk_ops,
};
