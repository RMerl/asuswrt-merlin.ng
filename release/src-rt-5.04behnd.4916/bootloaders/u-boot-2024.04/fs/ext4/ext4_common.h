/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * ext4ls and ext4load :  based on ext2 ls load support in Uboot.
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * based on code from grub2 fs/ext2.c and fs/fshelp.c by
 * GRUB  --  GRand Unified Bootloader
 * Copyright (C) 2003, 2004  Free Software Foundation, Inc.
 *
 * ext4write : Based on generic ext4 protocol.
 */

#ifndef __EXT4_COMMON__
#define __EXT4_COMMON__
#include <ext_common.h>
#include <ext4fs.h>
#include <malloc.h>
#include <asm/cache.h>
#include <linux/errno.h>
#if defined(CONFIG_EXT4_WRITE)
#include "ext4_journal.h"
#include <linux/crc16.h>
#endif

#define YES		1
#define NO		0
#define RECOVER	1
#define SCAN		0

#define S_IFLNK		0120000		/* symbolic link */
#define BLOCK_NO_ONE		1
#define SUPERBLOCK_START	(2 * 512)
#define SUPERBLOCK_SIZE	1024
#define F_FILE			1

static inline void *zalloc(size_t size)
{
	void *p = memalign(ARCH_DMA_MINALIGN, size);
	memset(p, 0, size);
	return p;
}

int ext4fs_read_inode(struct ext2_data *data, int ino,
		      struct ext2_inode *inode);
int ext4fs_read_file(struct ext2fs_node *node, loff_t pos, loff_t len,
		     char *buf, loff_t *actread);
int ext4fs_find_file(const char *path, struct ext2fs_node *rootnode,
			struct ext2fs_node **foundnode, int expecttype);
int ext4fs_iterate_dir(struct ext2fs_node *dir, char *name,
			struct ext2fs_node **fnode, int *ftype);

#if defined(CONFIG_EXT4_WRITE)
uint32_t ext4fs_div_roundup(uint32_t size, uint32_t n);
uint16_t ext4fs_checksum_update(unsigned int i);
int ext4fs_get_parent_inode_num(const char *dirname, char *dname, int flags);
int ext4fs_update_parent_dentry(char *filename, int file_type);
uint32_t ext4fs_get_new_blk_no(void);
int ext4fs_get_new_inode_no(void);
void ext4fs_reset_block_bmap(long int blockno, unsigned char *buffer,
					int index);
int ext4fs_set_block_bmap(long int blockno, unsigned char *buffer, int index);
int ext4fs_set_inode_bmap(int inode_no, unsigned char *buffer, int index);
void ext4fs_reset_inode_bmap(int inode_no, unsigned char *buffer, int index);
int ext4fs_iget(int inode_no, struct ext2_inode *inode);
void ext4fs_allocate_blocks(struct ext2_inode *file_inode,
				unsigned int total_remaining_blocks,
				unsigned int *total_no_of_block);
void put_ext4(uint64_t off, const void *buf, uint32_t size);
struct ext2_block_group *ext4fs_get_group_descriptor
	(const struct ext_filesystem *fs, uint32_t bg_idx);
uint64_t ext4fs_bg_get_block_id(const struct ext2_block_group *bg,
	const struct ext_filesystem *fs);
uint64_t ext4fs_bg_get_inode_id(const struct ext2_block_group *bg,
	const struct ext_filesystem *fs);
uint64_t ext4fs_bg_get_inode_table_id(const struct ext2_block_group *bg,
	const struct ext_filesystem *fs);
uint64_t ext4fs_sb_get_free_blocks(const struct ext2_sblock *sb);
void ext4fs_sb_set_free_blocks(struct ext2_sblock *sb, uint64_t free_blocks);
uint32_t ext4fs_bg_get_free_blocks(const struct ext2_block_group *bg,
	const struct ext_filesystem *fs);
#endif
#endif
