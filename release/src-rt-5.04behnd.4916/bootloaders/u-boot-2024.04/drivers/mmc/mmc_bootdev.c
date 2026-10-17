// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for MMC
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <mmc.h>

static int mmc_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_2_INTERNAL_FAST;

	return 0;
}

struct bootdev_ops mmc_bootdev_ops = {
};

static const struct udevice_id mmc_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-mmc" },
	{ }
};

U_BOOT_DRIVER(mmc_bootdev) = {
	.name		= "mmc_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &mmc_bootdev_ops,
	.bind		= mmc_bootdev_bind,
	.of_match	= mmc_bootdev_ids,
};

BOOTDEV_HUNTER(mmc_bootdev_hunter) = {
	.prio		= BOOTDEVP_2_INTERNAL_FAST,
	.uclass		= UCLASS_MMC,
	.drv		= DM_DRIVER_REF(mmc_bootdev),
};
