// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#include <errno.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sqfs_filesystem.h"
#include "sqfs_utils.h"

bool sqfs_is_dir(u16 type)
{
	return type == SQFS_DIR_TYPE || type == SQFS_LDIR_TYPE;
}

/*
 * Receives a pointer (void *) to a position in the inode table containing the
 * directory's inode. Returns directory inode offset into the directory table.
 * m_list contains each metadata block's position, and m_count is the number of
 * elements of m_list. Those metadata blocks come from the compressed directory
 * table.
 */
int sqfs_dir_offset(void *dir_i, u32 *m_list, int m_count)
{
	struct squashfs_base_inode *base = dir_i;
	struct squashfs_ldir_inode *ldir;
	struct squashfs_dir_inode *dir;
	u32 start_block;
	int j, offset;

	switch (get_unaligned_le16(&base->inode_type)) {
	case SQFS_DIR_TYPE:
		dir = (struct squashfs_dir_inode *)base;
		start_block = get_unaligned_le32(&dir->start_block);
		offset = get_unaligned_le16(&dir->offset);
		break;
	case SQFS_LDIR_TYPE:
		ldir = (struct squashfs_ldir_inode *)base;
		start_block = get_unaligned_le32(&ldir->start_block);
		offset = get_unaligned_le16(&ldir->offset);
		break;
	default:
		printf("Error: this is not a directory.\n");
		return -EINVAL;
	}

	if (offset < 0)
		return -EINVAL;

	for (j = 0; j < m_count; j++) {
		if (m_list[j] == start_block)
			return (++j * SQFS_METADATA_BLOCK_SIZE) + offset;
	}

	if (start_block == 0)
		return offset;

	printf("Error: invalid inode reference to directory table.\n");

	return -EINVAL;
}

bool sqfs_is_empty_dir(void *dir_i)
{
	struct squashfs_base_inode *base = dir_i;
	struct squashfs_ldir_inode *ldir;
	struct squashfs_dir_inode *dir;
	u32 file_size;

	switch (get_unaligned_le16(&base->inode_type)) {
	case SQFS_DIR_TYPE:
		dir = (struct squashfs_dir_inode *)base;
		file_size = get_unaligned_le16(&dir->file_size);
		break;
	case SQFS_LDIR_TYPE:
		ldir = (struct squashfs_ldir_inode *)base;
		file_size = get_unaligned_le16(&ldir->file_size);
		break;
	default:
		printf("Error: this is not a directory.\n");
		return false;
	}

	return file_size == SQFS_EMPTY_FILE_SIZE;
}
