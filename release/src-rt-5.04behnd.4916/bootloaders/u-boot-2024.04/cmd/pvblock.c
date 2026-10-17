// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 EPAM Systems Inc.
 *
 * XEN para-virtualized block device support
 */

#include <blk.h>
#include <common.h>
#include <command.h>

/* Current I/O Device */
static int pvblock_curr_device;

int do_pvblock(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return blk_common_cmd(argc, argv, UCLASS_PVBLOCK,
			      &pvblock_curr_device);
}

U_BOOT_CMD(pvblock, 5, 1, do_pvblock,
	   "Xen para-virtualized block device",
	   "info  - show available block devices\n"
	   "pvblock device [dev] - show or set current device\n"
	   "pvblock part [dev] - print partition table of one or all devices\n"
	   "pvblock read  addr blk# cnt\n"
	   "pvblock write addr blk# cnt - read/write `cnt'"
	   " blocks starting at block `blk#'\n"
	   "    to/from memory address `addr'");
