// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

#include <common.h>
#include <android_ab.h>
#include <command.h>
#include <env.h>
#include <part.h>

static int do_ab_select(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	char slot[2];
	bool dec_tries = true;

	if (argc < 4)
		return CMD_RET_USAGE;

	for (int i = 4; i < argc; i++) {
		if (strcmp(argv[i], "--no-dec") == 0) {
			dec_tries = false;
		} else {
			return CMD_RET_USAGE;
		}
	}

	/* Lookup the "misc" partition from argv[2] and argv[3] */
	if (part_get_info_by_dev_and_name_or_num(argv[2], argv[3],
						 &dev_desc, &part_info,
						 false) < 0) {
		return CMD_RET_FAILURE;
	}


	ret = ab_select_slot(dev_desc, &part_info, dec_tries);
	if (ret < 0) {
		printf("Android boot failed, error %d.\n", ret);
		return CMD_RET_FAILURE;
	}

	/* Android standard slot names are 'a', 'b', ... */
	slot[0] = BOOT_SLOT_NAME(ret);
	slot[1] = '\0';
	env_set(argv[1], slot);
	printf("ANDROID: Booting slot: %s\n", slot);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ab_select, 5, 0, do_ab_select,
	   "Select the slot used to boot from and register the boot attempt.",
	   "<slot_var_name> <interface> <dev[:part|#part_name]> [--no-dec]\n"
	   "    - Load the slot metadata from the partition 'part' on\n"
	   "      device type 'interface' instance 'dev' and store the active\n"
	   "      slot in the 'slot_var_name' variable. This also updates the\n"
	   "      Android slot metadata with a boot attempt, which can cause\n"
	   "      successive calls to this function to return a different result\n"
	   "      if the returned slot runs out of boot attempts.\n"
	   "    - If 'part_name' is passed, preceded with a # instead of :, the\n"
	   "      partition name whose label is 'part_name' will be looked up in\n"
	   "      the partition table. This is commonly the \"misc\" partition.\n"
           "    - If '--no-dec' is set, the number of tries remaining will not\n"
           "      decremented for the selected boot slot\n"
);
