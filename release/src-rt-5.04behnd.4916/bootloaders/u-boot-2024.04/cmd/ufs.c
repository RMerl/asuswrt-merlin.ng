// SPDX-License-Identifier: GPL-2.0+
/**
 * ufs.c - UFS specific U-Boot commands
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 *
 */
#include <common.h>
#include <command.h>
#include <ufs.h>

static int do_ufs(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int dev, ret;

	if (argc >= 2) {
		if (!strcmp(argv[1], "init")) {
			if (argc == 3) {
				dev = dectoul(argv[2], NULL);
				ret = ufs_probe_dev(dev);
				if (ret)
					return CMD_RET_FAILURE;
			} else {
				ufs_probe();
			}

			return CMD_RET_SUCCESS;
		}
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(ufs, 3, 1, do_ufs,
	   "UFS sub-system",
	   "init [dev] - init UFS subsystem\n"
);
