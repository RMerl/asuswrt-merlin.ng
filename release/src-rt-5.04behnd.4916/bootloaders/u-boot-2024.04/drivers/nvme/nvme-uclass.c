// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY UCLASS_NVME

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <nvme.h>

static int nvme_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST;

	return 0;
}

static int nvme_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	/* init PCI first since this is often used to provide NVMe */
	if (IS_ENABLED(CONFIG_PCI)) {
		ret = pci_init();
		if (ret)
			log_warning("Failed to init PCI (%dE)\n", ret);
	}

	ret = nvme_scan_namespace();
	if (ret)
		return log_msg_ret("scan", ret);

	return 0;
}

UCLASS_DRIVER(nvme) = {
	.name	= "nvme",
	.id	= UCLASS_NVME,
};

struct bootdev_ops nvme_bootdev_ops = {
};

static const struct udevice_id nvme_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-nvme" },
	{ }
};

U_BOOT_DRIVER(nvme_bootdev) = {
	.name		= "nvme_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &nvme_bootdev_ops,
	.bind		= nvme_bootdev_bind,
	.of_match	= nvme_bootdev_ids,
};

BOOTDEV_HUNTER(nvme_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_NVME,
	.hunt		= nvme_bootdev_hunt,
	.drv		= DM_DRIVER_REF(nvme_bootdev),
};
