// SPDX-License-Identifier: GPL-2.0+
/*
 *
 *	based on code of fs/reiserfs/dev.c by
 *
 *	(C) Copyright 2003 - 2004
 *	Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 */


#include <common.h>
#include <config.h>
#include <fs_internal.h>
#include <zfs_common.h>

static struct blk_desc *zfs_blk_desc;
static disk_partition_t *part_info;

void zfs_set_blk_dev(struct blk_desc *rbdd, disk_partition_t *info)
{
	zfs_blk_desc = rbdd;
	part_info = info;
}

/* err */
int zfs_devread(int sector, int byte_offset, int byte_len, char *buf)
{
	return fs_devread(zfs_blk_desc, part_info, sector, byte_offset,
			  byte_len, buf);
}
