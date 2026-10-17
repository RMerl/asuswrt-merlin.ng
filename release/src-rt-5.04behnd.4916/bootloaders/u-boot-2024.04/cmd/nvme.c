// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <dm.h>
#include <nvme.h>

static int nvme_curr_dev;

static int do_nvme(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int ret;

	if (argc == 2) {
		if (strncmp(argv[1], "scan", 4) == 0) {
			ret = nvme_scan_namespace();
			if (ret)
				return CMD_RET_FAILURE;

			return ret;
		}
		if (strncmp(argv[1], "deta", 4) == 0) {
			struct udevice *udev;

			ret = blk_get_device(UCLASS_NVME, nvme_curr_dev,
					     &udev);
			if (ret < 0)
				return CMD_RET_FAILURE;

			nvme_print_info(udev);

			return ret;
		}
	}

	return blk_common_cmd(argc, argv, UCLASS_NVME, &nvme_curr_dev);
}

U_BOOT_CMD(
	nvme, 8, 1, do_nvme,
	"NVM Express sub-system",
	"scan - scan NVMe devices\n"
	"nvme detail - show details of current NVMe device\n"
	"nvme info - show all available NVMe devices\n"
	"nvme device [dev] - show or set current NVMe device\n"
	"nvme part [dev] - print partition table of one or all NVMe devices\n"
	"nvme read addr blk# cnt - read `cnt' blocks starting at block\n"
	"     `blk#' to memory address `addr'\n"
	"nvme write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
);
