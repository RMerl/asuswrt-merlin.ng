// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020
 * Niel Fourie, DENX Software Engineering, lusus@denx.de.
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <dm.h>

static int do_lsblk(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct driver *d = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	struct udevice *udev;
	struct uclass *uc;
	struct blk_desc *desc;
	int ret, i;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret) {
		puts("Could not get BLK uclass.\n");
		return CMD_RET_FAILURE;
	}
	puts("Block Driver          Devices\n");
	puts("-----------------------------\n");
	for (entry = d; entry < d + n_ents; entry++) {
		if (entry->id != UCLASS_BLK)
			continue;
		i = 0;
		printf("%-20.20s", entry->name);
		uclass_foreach_dev(udev, uc) {
			if (udev->driver != entry)
				continue;
			desc = dev_get_uclass_plat(udev);
			printf("%c %s %u", i ? ',' : ':',
			       blk_get_uclass_name(desc->uclass_id),
			       desc->devnum);
			i++;
		}
		if (!i)
			puts(": <none>");
		puts("\n");
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(lsblk, 1, 0, do_lsblk, "list block drivers and devices",
	   "- display list of block device drivers and attached block devices"
);
