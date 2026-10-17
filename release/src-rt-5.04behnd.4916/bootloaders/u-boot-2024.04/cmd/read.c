/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <mapmem.h>
#include <part.h>

static int
do_rw(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct blk_desc *dev_desc = NULL;
	struct disk_partition part_info;
	ulong offset, limit;
	uint blk, cnt, res;
	void *addr;
	int part;

	if (argc != 6) {
		cmd_usage(cmdtp);
		return 1;
	}

	part = part_get_info_by_dev_and_name_or_num(argv[1], argv[2],
						    &dev_desc, &part_info, 1);
	if (part < 0)
		return 1;

	addr = map_sysmem(hextoul(argv[3], NULL), 0);
	blk = hextoul(argv[4], NULL);
	cnt = hextoul(argv[5], NULL);

	if (part > 0) {
		offset = part_info.start;
		limit = part_info.size;
	} else {
		/* Largest address not available in struct blk_desc. */
		offset = 0;
		limit = ~0;
	}

	if (cnt + blk > limit) {
		printf("%s out of range\n", cmdtp->name);
		return 1;
	}

	if (IS_ENABLED(CONFIG_CMD_WRITE) && !strcmp(cmdtp->name, "write"))
		res = blk_dwrite(dev_desc, offset + blk, cnt, addr);
	else
		res = blk_dread(dev_desc, offset + blk, cnt, addr);

	if (res != cnt) {
		printf("%s error\n", cmdtp->name);
		return 1;
	}

	return 0;
}

#ifdef CONFIG_CMD_READ
U_BOOT_CMD(
	read,	6,	0,	do_rw,
	"Load binary data from a partition",
	"<interface> <dev[:part|#partname]> addr blk# cnt"
);
#endif

#ifdef CONFIG_CMD_WRITE
U_BOOT_CMD(
	write,	6,	0,	do_rw,
	"Store binary data to a partition",
	"<interface> <dev[:part|#partname]> addr blk# cnt"
);
#endif
