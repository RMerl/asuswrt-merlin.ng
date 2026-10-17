// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#if CONFIG_IS_ENABLED(SANDBOX64)
#include <asm/test.h>
#endif
#include <linux/bitops.h>
#include "nvmxip.h"

/* LBA Macros */

#define DEFAULT_LBA_SHIFT 10 /* 1024 bytes per block */
#define DEFAULT_LBA_COUNT 1024 /* block count */

#define DEFAULT_LBA_SZ BIT(DEFAULT_LBA_SHIFT)

int nvmxip_probe(struct udevice *udev)
{
	int ret;
	struct udevice *bdev = NULL;
	char bdev_name[NVMXIP_BLKDEV_NAME_SZ + 1];
	int devnum;

	devnum = uclass_id_count(UCLASS_NVMXIP);
	snprintf(bdev_name, NVMXIP_BLKDEV_NAME_SZ, "blk#%d", devnum);

	ret = blk_create_devicef(udev, NVMXIP_BLKDRV_NAME, bdev_name, UCLASS_NVMXIP,
				 devnum, DEFAULT_LBA_SZ,
				 DEFAULT_LBA_COUNT, &bdev);
	if (ret) {
		log_err("[%s]: failure during creation of the block device %s, error %d\n",
			udev->name, bdev_name, ret);
		return ret;
	}

	ret = blk_probe_or_unbind(bdev);
	if (ret) {
		log_err("[%s]: failure during probing the block device %s, error %d\n",
			udev->name, bdev_name, ret);
		return ret;
	}

	log_info("[%s]: the block device %s ready for use\n", udev->name, bdev_name);

	return 0;
}

static int nvmxip_post_bind(struct udevice *udev)
{
	dev_or_flags(udev, DM_FLAG_PROBE_AFTER_BIND);
	return 0;
}

UCLASS_DRIVER(nvmxip) = {
	.name	   = "nvmxip",
	.id	   = UCLASS_NVMXIP,
	.post_bind = nvmxip_post_bind,
};
