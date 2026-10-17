// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for sata
 *
 * Copyright 2023 Tony Dinh <mibodhi@gmail.com>
 */

#include <common.h>
#include <ahci.h>
#include <bootdev.h>
#include <dm.h>
#include <init.h>
#include <sata.h>

static int sata_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST;

	return 0;
}

static int sata_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	if (IS_ENABLED(CONFIG_PCI)) {
		ret = pci_init();
		if (ret)
			return ret;
	}

	ret = sata_rescan(true);
	if (ret)
		return ret;

	return 0;
}

struct bootdev_ops sata_bootdev_ops = {
};

static const struct udevice_id sata_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-sata" },
	{ }
};

U_BOOT_DRIVER(sata_bootdev) = {
	.name		= "sata_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &sata_bootdev_ops,
	.bind		= sata_bootdev_bind,
	.of_match	= sata_bootdev_ids,
};

BOOTDEV_HUNTER(sata_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_AHCI,
	.hunt		= sata_bootdev_hunt,
	.drv		= DM_DRIVER_REF(sata_bootdev),
};
