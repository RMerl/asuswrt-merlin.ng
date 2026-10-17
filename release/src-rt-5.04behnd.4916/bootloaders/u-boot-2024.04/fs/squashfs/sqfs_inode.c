// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#include <asm/unaligned.h>
#include <compiler.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqfs_decompressor.h"
#include "sqfs_filesystem.h"
#include "sqfs_utils.h"

int sqfs_inode_size(struct squashfs_base_inode *inode, u32 blk_size)
{
	switch (get_unaligned_le16(&inode->inode_type)) {
	case SQFS_DIR_TYPE:
		return sizeof(struct squashfs_dir_inode);

	case SQFS_REG_TYPE: {
		struct squashfs_reg_inode *reg =
			(struct squashfs_reg_inode *)inode;
		u32 fragment = get_unaligned_le32(&reg->fragment);
		u32 file_size = get_unaligned_le32(&reg->file_size);
		unsigned int blk_list_size;

		if (SQFS_IS_FRAGMENTED(fragment))
			blk_list_size = file_size / blk_size;
		else
			blk_list_size = DIV_ROUND_UP(file_size, blk_size);

		return sizeof(*reg) + blk_list_size * sizeof(u32);
	}

	case SQFS_LDIR_TYPE: {
		struct squashfs_ldir_inode *ldir =
			(struct squashfs_ldir_inode *)inode;
		u16 i_count = get_unaligned_le16(&ldir->i_count);
		unsigned int index_list_size = 0, l = 0;
		struct squashfs_directory_index *di;
		u32 sz;

		if (i_count == 0)
			return sizeof(*ldir);

		di = ldir->index;
		while (l < i_count) {
			sz = get_unaligned_le32(&di->size) + 1;
			index_list_size += sz;
			di = (void *)di + sizeof(*di) + sz;
			l++;
		}

		return sizeof(*ldir) + index_list_size +
			i_count * SQFS_DIR_INDEX_BASE_LENGTH;
	}

	case SQFS_LREG_TYPE: {
		struct squashfs_lreg_inode *lreg =
			(struct squashfs_lreg_inode *)inode;
		u32 fragment = get_unaligned_le32(&lreg->fragment);
		u64 file_size = get_unaligned_le64(&lreg->file_size);
		unsigned int blk_list_size;

		if (fragment == 0xFFFFFFFF)
			blk_list_size = DIV_ROUND_UP(file_size, blk_size);
		else
			blk_list_size = file_size / blk_size;

		return sizeof(*lreg) + blk_list_size * sizeof(u32);
	}

	case SQFS_SYMLINK_TYPE:
	case SQFS_LSYMLINK_TYPE: {
		struct squashfs_symlink_inode *symlink =
			(struct squashfs_symlink_inode *)inode;

		return sizeof(*symlink) +
			get_unaligned_le32(&symlink->symlink_size);
	}

	case SQFS_BLKDEV_TYPE:
	case SQFS_CHRDEV_TYPE:
		return sizeof(struct squashfs_dev_inode);
	case SQFS_LBLKDEV_TYPE:
	case SQFS_LCHRDEV_TYPE:
		return sizeof(struct squashfs_ldev_inode);
	case SQFS_FIFO_TYPE:
	case SQFS_SOCKET_TYPE:
		return sizeof(struct squashfs_ipc_inode);
	case SQFS_LFIFO_TYPE:
	case SQFS_LSOCKET_TYPE:
		return sizeof(struct squashfs_lipc_inode);
	default:
		printf("Error while searching inode: unknown type.\n");
		return -EINVAL;
	}
}

/*
 * Given the uncompressed inode table, the inode to be found and the number of
 * inodes in the table, return inode position in case of success.
 */
void *sqfs_find_inode(void *inode_table, int inode_number, __le32 inode_count,
		      __le32 block_size)
{
	struct squashfs_base_inode *base;
	unsigned int offset = 0, k;
	int sz;

	if (!inode_table) {
		printf("%s: Invalid pointer to inode table.\n", __func__);
		return NULL;
	}

	for (k = 0; k < le32_to_cpu(inode_count); k++) {
		base = inode_table + offset;
		if (get_unaligned_le32(&base->inode_number) == inode_number)
			return inode_table + offset;

		sz = sqfs_inode_size(base, le32_to_cpu(block_size));
		if (sz < 0)
			return NULL;

		offset += sz;
	}

	printf("Inode not found.\n");

	return NULL;
}

int sqfs_read_metablock(unsigned char *file_mapping, int offset,
			bool *compressed, u32 *data_size)
{
	const unsigned char *data;
	u16 header;

	if (!file_mapping)
		return -EFAULT;
	data = file_mapping + offset;

	header = get_unaligned((u16 *)data);
	if (!header)
		return -EINVAL;

	*compressed = SQFS_COMPRESSED_METADATA(header);
	*data_size = SQFS_METADATA_SIZE(header);

	if (*data_size > SQFS_METADATA_BLOCK_SIZE) {
		printf("Invalid metatada block size: %d bytes.\n", *data_size);
		return -EINVAL;
	}

	return 0;
}
