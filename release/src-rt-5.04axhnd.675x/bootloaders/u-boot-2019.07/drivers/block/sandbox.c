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
#include <sandboxblockdev.h>
#include <linux/errno.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_BLK
static struct host_block_dev host_devices[CONFIG_HOST_MAX_DEVICES];

static struct host_block_dev *find_host_device(int dev)
{
	if (dev >= 0 && dev < CONFIG_HOST_MAX_DEVICES)
		return &host_devices[dev];

	return NULL;
}
#endif

#ifdef CONFIG_BLK
static unsigned long host_block_read(struct udevice *dev,
				     unsigned long start, lbaint_t blkcnt,
				     void *buffer)
{
	struct host_block_dev *host_dev = dev_get_platdata(dev);
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);

#else
static unsigned long host_block_read(struct blk_desc *block_dev,
				     unsigned long start, lbaint_t blkcnt,
				     void *buffer)
{
	int dev = block_dev->devnum;
	struct host_block_dev *host_dev = find_host_device(dev);

	if (!host_dev)
		return -1;
#endif

	if (os_lseek(host_dev->fd, start * block_dev->blksz, OS_SEEK_SET) ==
			-1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_read(host_dev->fd, buffer, blkcnt * block_dev->blksz);
	if (len >= 0)
		return len / block_dev->blksz;
	return -1;
}

#ifdef CONFIG_BLK
static unsigned long host_block_write(struct udevice *dev,
				      unsigned long start, lbaint_t blkcnt,
				      const void *buffer)
{
	struct host_block_dev *host_dev = dev_get_platdata(dev);
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#else
static unsigned long host_block_write(struct blk_desc *block_dev,
				      unsigned long start, lbaint_t blkcnt,
				      const void *buffer)
{
	int dev = block_dev->devnum;
	struct host_block_dev *host_dev = find_host_device(dev);
#endif

	if (os_lseek(host_dev->fd, start * block_dev->blksz, OS_SEEK_SET) ==
			-1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_write(host_dev->fd, buffer, blkcnt * block_dev->blksz);
	if (len >= 0)
		return len / block_dev->blksz;
	return -1;
}

#ifdef CONFIG_BLK
int host_dev_bind(int devnum, char *filename)
{
	struct host_block_dev *host_dev;
	struct udevice *dev;
	char dev_name[20], *str, *fname;
	int ret, fd;

	/* Remove and unbind the old device, if any */
	ret = blk_get_device(IF_TYPE_HOST, devnum, &dev);
	if (ret == 0) {
		ret = device_remove(dev, DM_REMOVE_NORMAL);
		if (ret)
			return ret;
		ret = device_unbind(dev);
		if (ret)
			return ret;
	} else if (ret != -ENODEV) {
		return ret;
	}

	if (!filename)
		return 0;

	snprintf(dev_name, sizeof(dev_name), "host%d", devnum);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;
	fname = strdup(filename);
	if (!fname) {
		free(str);
		return -ENOMEM;
	}

	fd = os_open(filename, OS_O_RDWR);
	if (fd == -1) {
		printf("Failed to access host backing file '%s'\n", filename);
		ret = -ENOENT;
		goto err;
	}
	ret = blk_create_device(gd->dm_root, "sandbox_host_blk", str,
				IF_TYPE_HOST, devnum, 512,
				os_lseek(fd, 0, OS_SEEK_END) / 512, &dev);
	if (ret)
		goto err_file;

	host_dev = dev_get_platdata(dev);
	host_dev->fd = fd;
	host_dev->filename = fname;

	ret = device_probe(dev);
	if (ret) {
		device_unbind(dev);
		goto err_file;
	}

	return 0;
err_file:
	os_close(fd);
err:
	free(fname);
	free(str);
	return ret;
}
#else
int host_dev_bind(int dev, char *filename)
{
	struct host_block_dev *host_dev = find_host_device(dev);

	if (!host_dev)
		return -1;
	if (host_dev->blk_dev.priv) {
		os_close(host_dev->fd);
		host_dev->blk_dev.priv = NULL;
	}
	if (host_dev->filename)
		free(host_dev->filename);
	if (filename && *filename) {
		host_dev->filename = strdup(filename);
	} else {
		host_dev->filename = NULL;
		return 0;
	}

	host_dev->fd = os_open(host_dev->filename, OS_O_RDWR);
	if (host_dev->fd == -1) {
		printf("Failed to access host backing file '%s'\n",
		       host_dev->filename);
		return 1;
	}

	struct blk_desc *blk_dev = &host_dev->blk_dev;
	blk_dev->if_type = IF_TYPE_HOST;
	blk_dev->priv = host_dev;
	blk_dev->blksz = 512;
	blk_dev->lba = os_lseek(host_dev->fd, 0, OS_SEEK_END) / blk_dev->blksz;
	blk_dev->block_read = host_block_read;
	blk_dev->block_write = host_block_write;
	blk_dev->devnum = dev;
	blk_dev->part_type = PART_TYPE_UNKNOWN;
	part_init(blk_dev);

	return 0;
}
#endif

int host_get_dev_err(int devnum, struct blk_desc **blk_devp)
{
#ifdef CONFIG_BLK
	struct udevice *dev;
	int ret;

	ret = blk_get_device(IF_TYPE_HOST, devnum, &dev);
	if (ret)
		return ret;
	*blk_devp = dev_get_uclass_platdata(dev);
#else
	struct host_block_dev *host_dev = find_host_device(devnum);

	if (!host_dev)
		return -ENODEV;

	if (!host_dev->blk_dev.priv)
		return -ENOENT;

	*blk_devp = &host_dev->blk_dev;
#endif

	return 0;
}

#ifdef CONFIG_BLK
static const struct blk_ops sandbox_host_blk_ops = {
	.read	= host_block_read,
	.write	= host_block_write,
};

U_BOOT_DRIVER(sandbox_host_blk) = {
	.name		= "sandbox_host_blk",
	.id		= UCLASS_BLK,
	.ops		= &sandbox_host_blk_ops,
	.platdata_auto_alloc_size = sizeof(struct host_block_dev),
};
#else
U_BOOT_LEGACY_BLK(sandbox_host) = {
	.if_typename	= "host",
	.if_type	= IF_TYPE_HOST,
	.max_devs	= CONFIG_HOST_MAX_DEVICES,
	.get_dev	= host_get_dev_err,
};
#endif
