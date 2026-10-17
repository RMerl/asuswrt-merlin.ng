// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * made from existing ext2/dev.c file of Uboot
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * based on code of fs/reiserfs/dev.c by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 */

/*
 * Changelog:
 *	0.1 - Newly created file for ext4fs support. Taken from
 *		fs/ext2/dev.c file in uboot.
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <fs_internal.h>
#include <ext4fs.h>
#include <ext_common.h>
#include "ext4_common.h"
#include <log.h>

lbaint_t part_offset;

static struct blk_desc *ext4fs_blk_desc;
static struct disk_partition *part_info;

void ext4fs_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info)
{
	assert(rbdd->blksz == (1 << rbdd->log2blksz));
	ext4fs_blk_desc = rbdd;
	get_fs()->dev_desc = rbdd;
	part_info = info;
	part_offset = info->start;
	get_fs()->total_sect = ((uint64_t)info->size * info->blksz) >>
		get_fs()->dev_desc->log2blksz;
}

int ext4fs_devread(lbaint_t sector, int byte_offset, int byte_len,
		   char *buffer)
{
	return fs_devread(get_fs()->dev_desc, part_info, sector, byte_offset,
			  byte_len, buffer);
}

int ext4_read_superblock(char *buffer)
{
	struct ext_filesystem *fs = get_fs();
	int sect = SUPERBLOCK_START >> fs->dev_desc->log2blksz;
	int off = SUPERBLOCK_START % fs->dev_desc->blksz;

	return ext4fs_devread(sect, off, SUPERBLOCK_SIZE,
				buffer);
}
