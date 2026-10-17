// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for SCSI
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <init.h>
#include <scsi.h>

static int scsi_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST;

	return 0;
}

static int scsi_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	if (IS_ENABLED(CONFIG_PCI)) {
		ret = pci_init();
		if (ret)
			return log_msg_ret("pci", ret);
	}

	ret = scsi_scan(true);
	if (ret)
		return log_msg_ret("scs", ret);

	return 0;
}

struct bootdev_ops scsi_bootdev_ops = {
};

static const struct udevice_id scsi_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-scsi" },
	{ }
};

U_BOOT_DRIVER(scsi_bootdev) = {
	.name		= "scsi_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &scsi_bootdev_ops,
	.bind		= scsi_bootdev_bind,
	.of_match	= scsi_bootdev_ids,
};

BOOTDEV_HUNTER(scsi_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_SCSI,
	.hunt		= scsi_bootdev_hunt,
	.drv		= DM_DRIVER_REF(scsi_bootdev),
};
