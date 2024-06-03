// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/device.h>
#include "nvme.h"

static int nvme_uclass_post_probe(struct udevice *udev)
{
	char name[20];
	struct udevice *ns_udev;
	int i, ret;
	struct nvme_dev *ndev = dev_get_priv(udev);

	/* Create a blk device for each namespace */
	for (i = 0; i < ndev->nn; i++) {
		/*
		 * Encode the namespace id to the device name so that
		 * we can extract it when doing the probe.
		 */
		sprintf(name, "blk#%d", i);

		/* The real blksz and size will be set by nvme_blk_probe() */
		ret = blk_create_devicef(udev, "nvme-blk", name, IF_TYPE_NVME,
					 -1, 512, 0, &ns_udev);
		if (ret)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(nvme) = {
	.name	= "nvme",
	.id	= UCLASS_NVME,
	.post_probe = nvme_uclass_post_probe,
};
