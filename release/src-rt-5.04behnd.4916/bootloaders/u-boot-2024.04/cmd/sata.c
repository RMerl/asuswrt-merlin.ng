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
#include <blk.h>
#include <dm.h>
#include <command.h>
#include <part.h>
#include <sata.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

static int sata_curr_device = -1;

int sata_remove(int devnum)
{
#ifdef CONFIG_AHCI
	struct udevice *dev;
	int rc;

	blk_unbind_all(UCLASS_AHCI);

	rc = uclass_find_device(UCLASS_AHCI, devnum, &dev);
	if (!rc && !dev)
		rc = uclass_find_first_device(UCLASS_AHCI, &dev);
	if (rc || !dev) {
		printf("Cannot find SATA device %d (err=%d)\n", devnum, rc);
		return CMD_RET_FAILURE;
	}

	rc = device_remove(dev, DM_REMOVE_NORMAL);
	if (rc) {
		printf("Cannot remove SATA device '%s' (err=%d)\n", dev->name,
		       rc);
		return CMD_RET_FAILURE;
	}

	return 0;
#else
	return sata_stop();
#endif
}

int sata_probe(int devnum)
{
#ifdef CONFIG_AHCI
	struct udevice *dev;
	int rc;

	rc = uclass_get_device(UCLASS_AHCI, devnum, &dev);
	if (rc)
		rc = uclass_find_first_device(UCLASS_AHCI, &dev);
	if (rc) {
		printf("Cannot probe SATA device %d (err=%d)\n", devnum, rc);
		return CMD_RET_FAILURE;
	}
	if (!dev) {
		printf("No SATA device found!\n");
		return CMD_RET_FAILURE;
	}
	rc = sata_scan(dev);
	if (rc) {
		printf("Cannot scan SATA device %d (err=%d)\n", devnum, rc);
		return CMD_RET_FAILURE;
	}

	return 0;
#else
	return sata_initialize() < 0 ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
#endif
}

static int do_sata(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int rc = 0;

	if (argc >= 2) {
		int devnum = 0;

		if (argc == 3)
			devnum = (int)dectoul(argv[2], NULL);
		if (!strcmp(argv[1], "stop"))
			return sata_remove(devnum);

		if (!strcmp(argv[1], "init")) {
			if (sata_curr_device != -1) {
				rc = sata_remove(devnum);
				if (rc)
					return rc;
			}

			return sata_probe(devnum);
		}
	}

	/* If the user has not yet run `sata init`, do it now */
	if (sata_curr_device == -1) {
		rc = sata_probe(0);
		if (rc)
			return rc;
		sata_curr_device = 0;
	}

	return blk_common_cmd(argc, argv, UCLASS_AHCI, &sata_curr_device);
}

U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"init - init SATA sub system\n"
	"sata stop [dev] - disable SATA sub system or device\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);
