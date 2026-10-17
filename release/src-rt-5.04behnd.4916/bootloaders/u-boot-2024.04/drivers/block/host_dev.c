// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for sandbox host interface, used to access files on the host which
 * contain partitions and filesystem
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_HOST

#include <common.h>
#include <blk.h>
#include <bootdev.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <os.h>
#include <sandbox_host.h>
#include <dm/device-internal.h>

static int host_sb_attach_file(struct udevice *dev, const char *filename)
{
	struct host_sb_plat *plat = dev_get_plat(dev);
	struct blk_desc *desc;
	struct udevice *blk;
	int ret, fd;
	off_t size;
	char *fname;

	if (!filename)
		return -EINVAL;

	if (plat->fd)
		return log_msg_ret("fd", -EEXIST);

	/* Sanity check that host_sb_bind() has been used */
	ret = blk_find_from_parent(dev, &blk);
	if (ret)
		return ret;

	fd = os_open(filename, OS_O_RDWR);
	if (fd == -1) {
		printf("Failed to access host backing file '%s', trying read-only\n",
		       filename);
		fd = os_open(filename, OS_O_RDONLY);
		if (fd == -1) {
			printf("- still failed\n");
			return log_msg_ret("open", -ENOENT);
		}
	}

	fname = strdup(filename);
	if (!fname) {
		ret = -ENOMEM;
		goto err_fname;
	}

	size = os_filesize(fd);
	desc = dev_get_uclass_plat(blk);
	if (size % desc->blksz) {
		printf("The size of host backing file '%s' is not multiple of "
		       "the device block size\n", filename);
		ret = -EINVAL;
		goto err_fname;
	}
	desc->lba = size / desc->blksz;

	/* write this in last, when nothing can go wrong */
	plat = dev_get_plat(dev);
	plat->fd = fd;
	plat->filename = fname;

	return 0;

err_fname:
	os_close(fd);

	return ret;
}

static int host_sb_detach_file(struct udevice *dev)
{
	struct host_sb_plat *plat = dev_get_plat(dev);
	int ret;

	if (!plat->fd)
		return log_msg_ret("fd", -ENOENT);

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret)
		return log_msg_ret("rem", ret);

	/* Unbind all children */
	ret = device_chld_unbind(dev, NULL);
	if (ret)
		return log_msg_ret("unb", ret);

	os_close(plat->fd);
	plat->fd = 0;
	free(plat->filename);
	free(plat->label);

	return 0;
}

static int host_sb_bind(struct udevice *dev)
{
	struct udevice *blk, *bdev;
	struct blk_desc *desc;
	int ret;

	ret = blk_create_devicef(dev, "sandbox_host_blk", "blk", UCLASS_HOST,
				 dev_seq(dev), DEFAULT_BLKSZ, 0, &blk);
	if (ret)
		return log_msg_ret("blk", ret);

	desc = dev_get_uclass_plat(blk);
	snprintf(desc->vendor, BLK_VEN_SIZE, "U-Boot");
	snprintf(desc->product, BLK_PRD_SIZE, "hostfile");
	snprintf(desc->revision, BLK_REV_SIZE, "1.0");

	if (CONFIG_IS_ENABLED(BOOTSTD)) {
		ret = bootdev_bind(dev, "host_bootdev", "bootdev", &bdev);
		if (ret)
			return log_msg_ret("bd", ret);
	}

	return 0;
}

static struct host_ops host_sb_ops = {
	.attach_file	= host_sb_attach_file,
	.detach_file	= host_sb_detach_file,
};

static const struct udevice_id host_ids[] = {
	{ .compatible = "sandbox,host" },
	{ }
};

U_BOOT_DRIVER(host_sb_drv) = {
	.name		= "host_sb_drv",
	.id		= UCLASS_HOST,
	.of_match	= host_ids,
	.ops		= &host_sb_ops,
	.bind		= host_sb_bind,
	.plat_auto	= sizeof(struct host_sb_plat),
};
