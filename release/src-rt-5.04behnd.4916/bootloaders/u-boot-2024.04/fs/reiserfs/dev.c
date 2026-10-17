// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 */


#include <common.h>
#include <config.h>
#include <reiserfs.h>
#include <fs_internal.h>
#include "reiserfs_private.h"

static struct blk_desc *reiserfs_blk_desc;
static struct disk_partition *part_info;


void reiserfs_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info)
{
	reiserfs_blk_desc = rbdd;
	part_info = info;
}

int reiserfs_devread(int sector, int byte_offset, int byte_len, char *buf)
{
	return fs_devread(reiserfs_blk_desc, part_info, sector, byte_offset,
			  byte_len, buf);
}
