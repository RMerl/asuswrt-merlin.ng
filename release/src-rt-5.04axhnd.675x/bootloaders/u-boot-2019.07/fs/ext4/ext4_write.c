// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * ext4ls and ext4load : Based on ext2 ls and load support in Uboot.
 *		       Ext4 read optimization taken from Open-Moko
 *		       Qi bootloader
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


#include <common.h>
#include <memalign.h>
#include <linux/stat.h>
#include <div64.h>
#include "ext4_common.h"

static inline void ext4fs_sb_free_inodes_inc(struct ext2_sblock *sb)
{
	sb->free_inodes = cpu_to_le32(le32_to_cpu(sb->free_inodes) + 1);
}

static inline void ext4fs_sb_free_blocks_inc(struct ext2_sblock *sb)
{
	sb->free_blocks = cpu_to_le32(le32_to_cpu(sb->free_blocks) + 1);
}

static inline void ext4fs_bg_free_inodes_inc
	(struct ext2_block_group *bg, const struct ext_filesystem *fs)
{
	uint32_t free_inodes = le16_to_cpu(bg->free_inodes);
	if (fs->gdsize == 64)
		free_inodes += le16_to_cpu(bg->free_inodes_high) << 16;
	free_inodes++;

	bg->free_inodes = cpu_to_le16(free_inodes & 0xffff);
	if (fs->gdsize == 64)
		bg->free_inodes_high = cpu_to_le16(free_inodes >> 16);
}

static inline void ext4fs_bg_free_blocks_inc
	(struct ext2_block_group *bg, const struct ext_filesystem *fs)
{
	uint32_t free_blocks = le16_to_cpu(bg->free_blocks);
	if (fs->gdsize == 64)
		free_blocks += le16_to_cpu(bg->free_blocks_high) << 16;
	free_blocks++;

	bg->free_blocks = cpu_to_le16(free_blocks & 0xffff);
	if (fs->gdsize == 64)
		bg->free_blocks_high = cpu_to_le16(free_blocks >> 16);
}

static void ext4fs_update(void)
{
	short i;
	ext4fs_update_journal();
	struct ext_filesystem *fs = get_fs();
	struct ext2_block_group *bgd = NULL;

	/* update  super block */
	put_ext4((uint64_t)(SUPERBLOCK_SIZE),
		 (struct ext2_sblock *)fs->sb, (uint32_t)SUPERBLOCK_SIZE);

	/* update block bitmaps */
	for (i = 0; i < fs->no_blkgrp; i++) {
		bgd = ext4fs_get_group_descriptor(fs, i);
		bgd->bg_checksum = cpu_to_le16(ext4fs_checksum_update(i));
		uint64_t b_bitmap_blk = ext4fs_bg_get_block_id(bgd, fs);
		put_ext4(b_bitmap_blk * fs->blksz,
			 fs->blk_bmaps[i], fs->blksz);
	}

	/* update inode bitmaps */
	for (i = 0; i < fs->no_blkgrp; i++) {
		bgd = ext4fs_get_group_descriptor(fs, i);
		uint64_t i_bitmap_blk = ext4fs_bg_get_inode_id(bgd, fs);
		put_ext4(i_bitmap_blk * fs->blksz,
			 fs->inode_bmaps[i], fs->blksz);
	}

	/* update the block group descriptor table */
	put_ext4((uint64_t)((uint64_t)fs->gdtable_blkno * (uint64_t)fs->blksz),
		 (struct ext2_block_group *)fs->gdtable,
		 (fs->blksz * fs->no_blk_pergdt));

	ext4fs_dump_metadata();

	gindex = 0;
	gd_index = 0;
}

int ext4fs_get_bgdtable(void)
{
	int status;
	struct ext_filesystem *fs = get_fs();
	int gdsize_total = ROUND(fs->no_blkgrp * fs->gdsize, fs->blksz);
	fs->no_blk_pergdt = gdsize_total / fs->blksz;

	/* allocate memory for gdtable */
	fs->gdtable = zalloc(gdsize_total);
	if (!fs->gdtable)
		return -ENOMEM;
	/* read the group descriptor table */
	status = ext4fs_devread((lbaint_t)fs->gdtable_blkno * fs->sect_perblk,
				0, fs->blksz * fs->no_blk_pergdt, fs->gdtable);
	if (status == 0)
		goto fail;

	if (ext4fs_log_gdt(fs->gdtable)) {
		printf("Error in ext4fs_log_gdt\n");
		return -1;
	}

	return 0;
fail:
	free(fs->gdtable);
	fs->gdtable = NULL;

	return -1;
}

static void delete_single_indirect_block(struct ext2_inode *inode)
{
	struct ext2_block_group *bgd = NULL;
	static int prev_bg_bmap_idx = -1;
	uint32_t blknr;
	int remainder;
	int bg_idx;
	int status;
	uint32_t blk_per_grp = le32_to_cpu(ext4fs_root->sblock.blocks_per_group);
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	if (!journal_buffer) {
		printf("No memory\n");
		return;
	}

	/* deleting the single indirect block associated with inode */
	if (inode->b.blocks.indir_block != 0) {
		blknr = le32_to_cpu(inode->b.blocks.indir_block);
		debug("SIPB releasing %u\n", blknr);
		bg_idx = blknr / blk_per_grp;
		if (fs->blksz == 1024) {
			remainder = blknr % blk_per_grp;
			if (!remainder)
				bg_idx--;
		}
		ext4fs_reset_block_bmap(blknr, fs->blk_bmaps[bg_idx], bg_idx);
		/* get  block group descriptor table */
		bgd = ext4fs_get_group_descriptor(fs, bg_idx);
		ext4fs_bg_free_blocks_inc(bgd, fs);
		ext4fs_sb_free_blocks_inc(fs->sb);
		/* journal backup */
		if (prev_bg_bmap_idx != bg_idx) {
			uint64_t b_bitmap_blk = ext4fs_bg_get_block_id(bgd, fs);
			status = ext4fs_devread(
					   b_bitmap_blk * fs->sect_perblk,
					   0, fs->blksz, journal_buffer);
			if (status == 0)
				goto fail;
			if (ext4fs_log_journal(journal_buffer, b_bitmap_blk))
				goto fail;
			prev_bg_bmap_idx = bg_idx;
		}
	}
fail:
	free(journal_buffer);
}

static void delete_double_indirect_block(struct ext2_inode *inode)
{
	int i;
	short status;
	static int prev_bg_bmap_idx = -1;
	uint32_t blknr;
	int remainder;
	int bg_idx;
	uint32_t blk_per_grp = le32_to_cpu(ext4fs_root->sblock.blocks_per_group);
	__le32 *di_buffer = NULL;
	void *dib_start_addr = NULL;
	struct ext2_block_group *bgd = NULL;
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	if (!journal_buffer) {
		printf("No memory\n");
		return;
	}

	if (inode->b.blocks.double_indir_block != 0) {
		di_buffer = zalloc(fs->blksz);
		if (!di_buffer) {
			printf("No memory\n");
			return;
		}
		dib_start_addr = di_buffer;
		blknr = le32_to_cpu(inode->b.blocks.double_indir_block);
		status = ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0,
					fs->blksz, (char *)di_buffer);
		for (i = 0; i < fs->blksz / sizeof(int); i++) {
			if (*di_buffer == 0)
				break;

			debug("DICB releasing %u\n", *di_buffer);
			bg_idx = le32_to_cpu(*di_buffer) / blk_per_grp;
			if (fs->blksz == 1024) {
				remainder = le32_to_cpu(*di_buffer) % blk_per_grp;
				if (!remainder)
					bg_idx--;
			}
			/* get  block group descriptor table */
			bgd = ext4fs_get_group_descriptor(fs, bg_idx);
			ext4fs_reset_block_bmap(le32_to_cpu(*di_buffer),
					fs->blk_bmaps[bg_idx], bg_idx);
			di_buffer++;
			ext4fs_bg_free_blocks_inc(bgd, fs);
			ext4fs_sb_free_blocks_inc(fs->sb);
			/* journal backup */
			if (prev_bg_bmap_idx != bg_idx) {
				uint64_t b_bitmap_blk =
					ext4fs_bg_get_block_id(bgd, fs);
				status = ext4fs_devread(b_bitmap_blk
							* fs->sect_perblk, 0,
							fs->blksz,
							journal_buffer);
				if (status == 0)
					goto fail;

				if (ext4fs_log_journal(journal_buffer,
						       b_bitmap_blk))
					goto fail;
				prev_bg_bmap_idx = bg_idx;
			}
		}

		/* removing the parent double indirect block */
		blknr = le32_to_cpu(inode->b.blocks.double_indir_block);
		bg_idx = blknr / blk_per_grp;
		if (fs->blksz == 1024) {
			remainder = blknr % blk_per_grp;
			if (!remainder)
				bg_idx--;
		}
		/* get  block group descriptor table */
		bgd = ext4fs_get_group_descriptor(fs, bg_idx);
		ext4fs_reset_block_bmap(blknr, fs->blk_bmaps[bg_idx], bg_idx);
		ext4fs_bg_free_blocks_inc(bgd, fs);
		ext4fs_sb_free_blocks_inc(fs->sb);
		/* journal backup */
		if (prev_bg_bmap_idx != bg_idx) {
			uint64_t b_bitmap_blk = ext4fs_bg_get_block_id(bgd, fs);
			status = ext4fs_devread(b_bitmap_blk * fs->sect_perblk,
						0, fs->blksz, journal_buffer);
			if (status == 0)
				goto fail;

			if (ext4fs_log_journal(journal_buffer, b_bitmap_blk))
				goto fail;
			prev_bg_bmap_idx = bg_idx;
		}
		debug("DIPB releasing %d\n", blknr);
	}
fail:
	free(dib_start_addr);
	free(journal_buffer);
}

static void delete_triple_indirect_block(struct ext2_inode *inode)
{
	int i, j;
	short status;
	static int prev_bg_bmap_idx = -1;
	uint32_t blknr;
	int remainder;
	int bg_idx;
	uint32_t blk_per_grp = le32_to_cpu(ext4fs_root->sblock.blocks_per_group);
	__le32 *tigp_buffer = NULL;
	void *tib_start_addr = NULL;
	__le32 *tip_buffer = NULL;
	void *tipb_start_addr = NULL;
	struct ext2_block_group *bgd = NULL;
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	if (!journal_buffer) {
		printf("No memory\n");
		return;
	}

	if (inode->b.blocks.triple_indir_block != 0) {
		tigp_buffer = zalloc(fs->blksz);
		if (!tigp_buffer) {
			printf("No memory\n");
			return;
		}
		tib_start_addr = tigp_buffer;
		blknr = le32_to_cpu(inode->b.blocks.triple_indir_block);
		status = ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0,
					fs->blksz, (char *)tigp_buffer);
		for (i = 0; i < fs->blksz / sizeof(int); i++) {
			if (*tigp_buffer == 0)
				break;
			debug("tigp buffer releasing %u\n", *tigp_buffer);

			tip_buffer = zalloc(fs->blksz);
			if (!tip_buffer)
				goto fail;
			tipb_start_addr = tip_buffer;
			status = ext4fs_devread((lbaint_t)le32_to_cpu(*tigp_buffer) *
						fs->sect_perblk, 0, fs->blksz,
						(char *)tip_buffer);
			for (j = 0; j < fs->blksz / sizeof(int); j++) {
				if (le32_to_cpu(*tip_buffer) == 0)
					break;
				bg_idx = le32_to_cpu(*tip_buffer) / blk_per_grp;
				if (fs->blksz == 1024) {
					remainder = le32_to_cpu(*tip_buffer) % blk_per_grp;
					if (!remainder)
						bg_idx--;
				}

				ext4fs_reset_block_bmap(le32_to_cpu(*tip_buffer),
							fs->blk_bmaps[bg_idx],
							bg_idx);

				tip_buffer++;
				/* get  block group descriptor table */
				bgd = ext4fs_get_group_descriptor(fs, bg_idx);
				ext4fs_bg_free_blocks_inc(bgd, fs);
				ext4fs_sb_free_blocks_inc(fs->sb);
				/* journal backup */
				if (prev_bg_bmap_idx != bg_idx) {
					uint64_t b_bitmap_blk =
						ext4fs_bg_get_block_id(bgd, fs);
					status =
					    ext4fs_devread(
							b_bitmap_blk *
							fs->sect_perblk, 0,
							fs->blksz,
							journal_buffer);
					if (status == 0)
						goto fail;

					if (ext4fs_log_journal(journal_buffer,
							       b_bitmap_blk))
						goto fail;
					prev_bg_bmap_idx = bg_idx;
				}
			}
			free(tipb_start_addr);
			tipb_start_addr = NULL;

			/*
			 * removing the grand parent blocks
			 * which is connected to inode
			 */
			bg_idx = le32_to_cpu(*tigp_buffer) / blk_per_grp;
			if (fs->blksz == 1024) {
				remainder = le32_to_cpu(*tigp_buffer) % blk_per_grp;
				if (!remainder)
					bg_idx--;
			}
			ext4fs_reset_block_bmap(le32_to_cpu(*tigp_buffer),
						fs->blk_bmaps[bg_idx], bg_idx);

			tigp_buffer++;
			/* get  block group descriptor table */
			bgd = ext4fs_get_group_descriptor(fs, bg_idx);
			ext4fs_bg_free_blocks_inc(bgd, fs);
			ext4fs_sb_free_blocks_inc(fs->sb);
			/* journal backup */
			if (prev_bg_bmap_idx != bg_idx) {
				uint64_t b_bitmap_blk =
					ext4fs_bg_get_block_id(bgd, fs);
				memset(journal_buffer, '\0', fs->blksz);
				status = ext4fs_devread(b_bitmap_blk *
							fs->sect_perblk, 0,
							fs->blksz,
							journal_buffer);
				if (status == 0)
					goto fail;

				if (ext4fs_log_journal(journal_buffer,
						       b_bitmap_blk))
					goto fail;
				prev_bg_bmap_idx = bg_idx;
			}
		}

		/* removing the grand parent triple indirect block */
		blknr = le32_to_cpu(inode->b.blocks.triple_indir_block);
		bg_idx = blknr / blk_per_grp;
		if (fs->blksz == 1024) {
			remainder = blknr % blk_per_grp;
			if (!remainder)
				bg_idx--;
		}
		ext4fs_reset_block_bmap(blknr, fs->blk_bmaps[bg_idx], bg_idx);
		/* get  block group descriptor table */
		bgd = ext4fs_get_group_descriptor(fs, bg_idx);
		ext4fs_bg_free_blocks_inc(bgd, fs);
		ext4fs_sb_free_blocks_inc(fs->sb);
		/* journal backup */
		if (prev_bg_bmap_idx != bg_idx) {
			uint64_t b_bitmap_blk = ext4fs_bg_get_block_id(bgd, fs);
			status = ext4fs_devread(b_bitmap_blk * fs->sect_perblk,
						0, fs->blksz, journal_buffer);
			if (status == 0)
				goto fail;

			if (ext4fs_log_journal(journal_buffer, b_bitmap_blk))
				goto fail;
			prev_bg_bmap_idx = bg_idx;
		}
		debug("tigp buffer itself releasing %d\n", blknr);
	}
fail:
	free(tib_start_addr);
	free(tipb_start_addr);
	free(journal_buffer);
}

static int ext4fs_delete_file(int inodeno)
{
	struct ext2_inode inode;
	short status;
	int i;
	int remainder;
	long int blknr;
	int bg_idx;
	int ibmap_idx;
	char *read_buffer = NULL;
	char *start_block_address = NULL;
	uint32_t no_blocks;

	static int prev_bg_bmap_idx = -1;
	unsigned int inodes_per_block;
	uint32_t blkno;
	unsigned int blkoff;
	uint32_t blk_per_grp = le32_to_cpu(ext4fs_root->sblock.blocks_per_group);
	uint32_t inode_per_grp = le32_to_cpu(ext4fs_root->sblock.inodes_per_group);
	struct ext2_inode *inode_buffer = NULL;
	struct ext2_block_group *bgd = NULL;
	struct ext_filesystem *fs = get_fs();
	char *journal_buffer = zalloc(fs->blksz);
	if (!journal_buffer)
		return -ENOMEM;
	status = ext4fs_read_inode(ext4fs_root, inodeno, &inode);
	if (status == 0)
		goto fail;

	/* read the block no allocated to a file */
	no_blocks = le32_to_cpu(inode.size) / fs->blksz;
	if (le32_to_cpu(inode.size) % fs->blksz)
		no_blocks++;

	/*
	 * special case for symlinks whose target are small enough that
	 *it fits in struct ext2_inode.b.symlink: no block had been allocated
	 */
	if ((le16_to_cpu(inode.mode) & S_IFLNK) &&
	    le32_to_cpu(inode.size) <= sizeof(inode.b.symlink)) {
		no_blocks = 0;
	}

	if (le32_to_cpu(inode.flags) & EXT4_EXTENTS_FL) {
		/* FIXME delete extent index blocks, i.e. eh_depth >= 1 */
		struct ext4_extent_header *eh =
			(struct ext4_extent_header *)
				inode.b.blocks.dir_blocks;
		debug("del: dep=%d entries=%d\n", eh->eh_depth, eh->eh_entries);
	} else {
		delete_single_indirect_block(&inode);
		delete_double_indirect_block(&inode);
		delete_triple_indirect_block(&inode);
	}

	/* release data blocks */
	for (i = 0; i < no_blocks; i++) {
		blknr = read_allocated_block(&inode, i, NULL);
		if (blknr == 0)
			continue;
		if (blknr < 0)
			goto fail;
		bg_idx = blknr / blk_per_grp;
		if (fs->blksz == 1024) {
			remainder = blknr % blk_per_grp;
			if (!remainder)
				bg_idx--;
		}
		ext4fs_reset_block_bmap(blknr, fs->blk_bmaps[bg_idx],
					bg_idx);
		debug("EXT4 Block releasing %ld: %d\n", blknr, bg_idx);

		/* get  block group descriptor table */
		bgd = ext4fs_get_group_descriptor(fs, bg_idx);
		ext4fs_bg_free_blocks_inc(bgd, fs);
		ext4fs_sb_free_blocks_inc(fs->sb);
		/* journal backup */
		if (prev_bg_bmap_idx != bg_idx) {
			uint64_t b_bitmap_blk = ext4fs_bg_get_block_id(bgd, fs);
			status = ext4fs_devread(b_bitmap_blk * fs->sect_perblk,
						0, fs->blksz,
						journal_buffer);
			if (status == 0)
				goto fail;
			if (ext4fs_log_journal(journal_buffer, b_bitmap_blk))
				goto fail;
			prev_bg_bmap_idx = bg_idx;
		}
	}

	/* release inode */
	/* from the inode no to blockno */
	inodes_per_block = fs->blksz / fs->inodesz;
	ibmap_idx = inodeno / inode_per_grp;

	/* get the block no */
	inodeno--;
	/* get  block group descriptor table */
	bgd = ext4fs_get_group_descriptor(fs, ibmap_idx);
	blkno = ext4fs_bg_get_inode_table_id(bgd, fs) +
		(inodeno % inode_per_grp) / inodes_per_block;

	/* get the offset of the inode */
	blkoff = ((inodeno) % inodes_per_block) * fs->inodesz;

	/* read the block no containing the inode */
	read_buffer = zalloc(fs->blksz);
	if (!read_buffer)
		goto fail;
	start_block_address = read_buffer;
	status = ext4fs_devread((lbaint_t)blkno * fs->sect_perblk,
				0, fs->blksz, read_buffer);
	if (status == 0)
		goto fail;

	if (ext4fs_log_journal(read_buffer, blkno))
		goto fail;

	read_buffer = read_buffer + blkoff;
	inode_buffer = (struct ext2_inode *)read_buffer;
	memset(inode_buffer, '\0', fs->inodesz);

	/* write the inode to original position in inode table */
	if (ext4fs_put_metadata(start_block_address, blkno))
		goto fail;

	/* update the respective inode bitmaps */
	inodeno++;
	ext4fs_reset_inode_bmap(inodeno, fs->inode_bmaps[ibmap_idx], ibmap_idx);
	ext4fs_bg_free_inodes_inc(bgd, fs);
	ext4fs_sb_free_inodes_inc(fs->sb);
	/* journal backup */
	memset(journal_buffer, '\0', fs->blksz);
	status = ext4fs_devread(ext4fs_bg_get_inode_id(bgd, fs) *
				fs->sect_perblk, 0, fs->blksz, journal_buffer);
	if (status == 0)
		goto fail;
	if (ext4fs_log_journal(journal_buffer, ext4fs_bg_get_inode_id(bgd, fs)))
		goto fail;

	ext4fs_update();
	ext4fs_deinit();
	ext4fs_reinit_global();

	if (ext4fs_init() != 0) {
		printf("error in File System init\n");
		goto fail;
	}

	free(start_block_address);
	free(journal_buffer);

	return 0;
fail:
	free(start_block_address);
	free(journal_buffer);

	return -1;
}

int ext4fs_init(void)
{
	short status;
	int i;
	uint32_t real_free_blocks = 0;
	struct ext_filesystem *fs = get_fs();

	/* populate fs */
	fs->blksz = EXT2_BLOCK_SIZE(ext4fs_root);
	fs->sect_perblk = fs->blksz >> fs->dev_desc->log2blksz;

	/* get the superblock */
	fs->sb = zalloc(SUPERBLOCK_SIZE);
	if (!fs->sb)
		return -ENOMEM;
	if (!ext4_read_superblock((char *)fs->sb))
		goto fail;

	/* init journal */
	if (ext4fs_init_journal())
		goto fail;

	/* get total no of blockgroups */
	fs->no_blkgrp = (uint32_t)ext4fs_div_roundup(
			le32_to_cpu(ext4fs_root->sblock.total_blocks)
			- le32_to_cpu(ext4fs_root->sblock.first_data_block),
			le32_to_cpu(ext4fs_root->sblock.blocks_per_group));

	/* get the block group descriptor table */
	fs->gdtable_blkno = ((EXT2_MIN_BLOCK_SIZE == fs->blksz) + 1);
	if (ext4fs_get_bgdtable() == -1) {
		printf("Error in getting the block group descriptor table\n");
		goto fail;
	}

	/* load all the available bitmap block of the partition */
	fs->blk_bmaps = zalloc(fs->no_blkgrp * sizeof(char *));
	if (!fs->blk_bmaps)
		goto fail;
	for (i = 0; i < fs->no_blkgrp; i++) {
		fs->blk_bmaps[i] = zalloc(fs->blksz);
		if (!fs->blk_bmaps[i])
			goto fail;
	}

	for (i = 0; i < fs->no_blkgrp; i++) {
		struct ext2_block_group *bgd =
			ext4fs_get_group_descriptor(fs, i);
		status = ext4fs_devread(ext4fs_bg_get_block_id(bgd, fs) *
				   fs->sect_perblk, 0,
				   fs->blksz, (char *)fs->blk_bmaps[i]);
		if (status == 0)
			goto fail;
	}

	/* load all the available inode bitmap of the partition */
	fs->inode_bmaps = zalloc(fs->no_blkgrp * sizeof(unsigned char *));
	if (!fs->inode_bmaps)
		goto fail;
	for (i = 0; i < fs->no_blkgrp; i++) {
		fs->inode_bmaps[i] = zalloc(fs->blksz);
		if (!fs->inode_bmaps[i])
			goto fail;
	}

	for (i = 0; i < fs->no_blkgrp; i++) {
		struct ext2_block_group *bgd =
			ext4fs_get_group_descriptor(fs, i);
		status = ext4fs_devread(ext4fs_bg_get_inode_id(bgd, fs) *
					fs->sect_perblk,
					0, fs->blksz,
					(char *)fs->inode_bmaps[i]);
		if (status == 0)
			goto fail;
	}

	/*
	 * check filesystem consistency with free blocks of file system
	 * some time we observed that superblock freeblocks does not match
	 * with the  blockgroups freeblocks when improper
	 * reboot of a linux kernel
	 */
	for (i = 0; i < fs->no_blkgrp; i++) {
		struct ext2_block_group *bgd =
			ext4fs_get_group_descriptor(fs, i);
		real_free_blocks = real_free_blocks +
			ext4fs_bg_get_free_blocks(bgd, fs);
	}
	if (real_free_blocks != ext4fs_sb_get_free_blocks(fs->sb))
		ext4fs_sb_set_free_blocks(fs->sb, real_free_blocks);

	return 0;
fail:
	ext4fs_deinit();

	return -1;
}

void ext4fs_deinit(void)
{
	int i;
	struct ext2_inode inode_journal;
	struct journal_superblock_t *jsb;
	uint32_t blknr;
	struct ext_filesystem *fs = get_fs();
	uint32_t new_feature_incompat;

	/* free journal */
	char *temp_buff = zalloc(fs->blksz);
	if (temp_buff) {
		ext4fs_read_inode(ext4fs_root, EXT2_JOURNAL_INO,
				  &inode_journal);
		blknr = read_allocated_block(&inode_journal,
					EXT2_JOURNAL_SUPERBLOCK, NULL);
		ext4fs_devread((lbaint_t)blknr * fs->sect_perblk, 0, fs->blksz,
			       temp_buff);
		jsb = (struct journal_superblock_t *)temp_buff;
		jsb->s_start = 0;
		put_ext4((uint64_t) ((uint64_t)blknr * (uint64_t)fs->blksz),
			 (struct journal_superblock_t *)temp_buff, fs->blksz);
		free(temp_buff);
	}
	ext4fs_free_journal();

	/* get the superblock */
	ext4_read_superblock((char *)fs->sb);
	new_feature_incompat = le32_to_cpu(fs->sb->feature_incompat);
	new_feature_incompat &= ~EXT3_FEATURE_INCOMPAT_RECOVER;
	fs->sb->feature_incompat = cpu_to_le32(new_feature_incompat);
	put_ext4((uint64_t)(SUPERBLOCK_SIZE),
		 (struct ext2_sblock *)fs->sb, (uint32_t)SUPERBLOCK_SIZE);
	free(fs->sb);
	fs->sb = NULL;

	if (fs->blk_bmaps) {
		for (i = 0; i < fs->no_blkgrp; i++) {
			free(fs->blk_bmaps[i]);
			fs->blk_bmaps[i] = NULL;
		}
		free(fs->blk_bmaps);
		fs->blk_bmaps = NULL;
	}

	if (fs->inode_bmaps) {
		for (i = 0; i < fs->no_blkgrp; i++) {
			free(fs->inode_bmaps[i]);
			fs->inode_bmaps[i] = NULL;
		}
		free(fs->inode_bmaps);
		fs->inode_bmaps = NULL;
	}


	free(fs->gdtable);
	fs->gdtable = NULL;
	/*
	 * reinitiliazed the global inode and
	 * block bitmap first execution check variables
	 */
	fs->first_pass_ibmap = 0;
	fs->first_pass_bbmap = 0;
	fs->curr_inode_no = 0;
	fs->curr_blkno = 0;
}

/*
 * Write data to filesystem blocks. Uses same optimization for
 * contigous sectors as ext4fs_read_file
 */
static int ext4fs_write_file(struct ext2_inode *file_inode,
			     int pos, unsigned int len, const char *buf)
{
	int i;
	int blockcnt;
	uint32_t filesize = le32_to_cpu(file_inode->size);
	struct ext_filesystem *fs = get_fs();
	int log2blksz = fs->dev_desc->log2blksz;
	int log2_fs_blocksize = LOG2_BLOCK_SIZE(ext4fs_root) - log2blksz;
	int previous_block_number = -1;
	int delayed_start = 0;
	int delayed_extent = 0;
	int delayed_next = 0;
	const char *delayed_buf = NULL;

	/* Adjust len so it we can't read past the end of the file. */
	if (len > filesize)
		len = filesize;

	blockcnt = ((len + pos) + fs->blksz - 1) / fs->blksz;

	for (i = pos / fs->blksz; i < blockcnt; i++) {
		long int blknr;
		int blockend = fs->blksz;
		int skipfirst = 0;
		blknr = read_allocated_block(file_inode, i, NULL);
		if (blknr <= 0)
			return -1;

		blknr = blknr << log2_fs_blocksize;

		if (blknr) {
			if (previous_block_number != -1) {
				if (delayed_next == blknr) {
					delayed_extent += blockend;
					delayed_next += blockend >> log2blksz;
				} else {	/* spill */
					put_ext4((uint64_t)
						 ((uint64_t)delayed_start << log2blksz),
						 delayed_buf,
						 (uint32_t) delayed_extent);
					previous_block_number = blknr;
					delayed_start = blknr;
					delayed_extent = blockend;
					delayed_buf = buf;
					delayed_next = blknr +
					    (blockend >> log2blksz);
				}
			} else {
				previous_block_number = blknr;
				delayed_start = blknr;
				delayed_extent = blockend;
				delayed_buf = buf;
				delayed_next = blknr +
				    (blockend >> log2blksz);
			}
		} else {
			if (previous_block_number != -1) {
				/* spill */
				put_ext4((uint64_t) ((uint64_t)delayed_start <<
						     log2blksz),
					 delayed_buf,
					 (uint32_t) delayed_extent);
				previous_block_number = -1;
			}
		}
		buf += fs->blksz - skipfirst;
	}
	if (previous_block_number != -1) {
		/* spill */
		put_ext4((uint64_t) ((uint64_t)delayed_start << log2blksz),
			 delayed_buf, (uint32_t) delayed_extent);
		previous_block_number = -1;
	}

	return len;
}

int ext4fs_write(const char *fname, const char *buffer,
		 unsigned long sizebytes, int type)
{
	int ret = 0;
	struct ext2_inode *file_inode = NULL;
	unsigned char *inode_buffer = NULL;
	int parent_inodeno;
	int inodeno;
	time_t timestamp = 0;

	uint64_t bytes_reqd_for_file;
	unsigned int blks_reqd_for_file;
	unsigned int blocks_remaining;
	int existing_file_inodeno;
	char *temp_ptr = NULL;
	long int itable_blkno;
	long int parent_itable_blkno;
	long int blkoff;
	struct ext2_sblock *sblock = &(ext4fs_root->sblock);
	unsigned int inodes_per_block;
	unsigned int ibmap_idx;
	struct ext2_block_group *bgd = NULL;
	struct ext_filesystem *fs = get_fs();
	ALLOC_CACHE_ALIGN_BUFFER(char, filename, 256);
	bool store_link_in_inode = false;
	memset(filename, 0x00, 256);

	if (type != FILETYPE_REG && type != FILETYPE_SYMLINK)
		return -1;

	g_parent_inode = zalloc(fs->inodesz);
	if (!g_parent_inode)
		goto fail;

	if (ext4fs_init() != 0) {
		printf("error in File System init\n");
		return -1;
	}

	if (le32_to_cpu(fs->sb->feature_ro_compat) & EXT4_FEATURE_RO_COMPAT_METADATA_CSUM) {
		printf("Unsupported feature metadata_csum found, not writing.\n");
		return -1;
	}

	inodes_per_block = fs->blksz / fs->inodesz;
	parent_inodeno = ext4fs_get_parent_inode_num(fname, filename, F_FILE);
	if (parent_inodeno == -1)
		goto fail;
	if (ext4fs_iget(parent_inodeno, g_parent_inode))
		goto fail;
	/* do not mess up a directory using hash trees */
	if (le32_to_cpu(g_parent_inode->flags) & EXT4_INDEX_FL) {
		printf("hash tree directory\n");
		goto fail;
	}
	/* check if the filename is already present in root */
	existing_file_inodeno = ext4fs_filename_unlink(filename);
	if (existing_file_inodeno != -1) {
		ret = ext4fs_delete_file(existing_file_inodeno);
		fs->first_pass_bbmap = 0;
		fs->curr_blkno = 0;

		fs->first_pass_ibmap = 0;
		fs->curr_inode_no = 0;
		if (ret)
			goto fail;
	}

	/* calculate how many blocks required */
	if (type == FILETYPE_SYMLINK &&
	    sizebytes <= sizeof(file_inode->b.symlink)) {
		store_link_in_inode = true;
		bytes_reqd_for_file = 0;
	} else {
		bytes_reqd_for_file = sizebytes;
	}

	blks_reqd_for_file = lldiv(bytes_reqd_for_file, fs->blksz);
	if (do_div(bytes_reqd_for_file, fs->blksz) != 0) {
		blks_reqd_for_file++;
		debug("total bytes for a file %u\n", blks_reqd_for_file);
	}
	blocks_remaining = blks_reqd_for_file;
	/* test for available space in partition */
	if (le32_to_cpu(fs->sb->free_blocks) < blks_reqd_for_file) {
		printf("Not enough space on partition !!!\n");
		goto fail;
	}

	inodeno = ext4fs_update_parent_dentry(filename, type);
	if (inodeno == -1)
		goto fail;
	/* prepare file inode */
	inode_buffer = zalloc(fs->inodesz);
	if (!inode_buffer)
		goto fail;
	file_inode = (struct ext2_inode *)inode_buffer;
	file_inode->size = cpu_to_le32(sizebytes);
	if (type == FILETYPE_SYMLINK) {
		file_inode->mode = cpu_to_le16(S_IFLNK | S_IRWXU | S_IRWXG |
					       S_IRWXO);
		if (store_link_in_inode) {
			strncpy(file_inode->b.symlink, buffer, sizebytes);
			sizebytes = 0;
		}
	} else {
		file_inode->mode = cpu_to_le16(S_IFREG | S_IRWXU | S_IRGRP |
					       S_IROTH | S_IXGRP | S_IXOTH);
	}
	/* ToDo: Update correct time */
	file_inode->mtime = cpu_to_le32(timestamp);
	file_inode->atime = cpu_to_le32(timestamp);
	file_inode->ctime = cpu_to_le32(timestamp);
	file_inode->nlinks = cpu_to_le16(1);

	/* Allocate data blocks */
	ext4fs_allocate_blocks(file_inode, blocks_remaining,
			       &blks_reqd_for_file);
	file_inode->blockcnt = cpu_to_le32((blks_reqd_for_file * fs->blksz) >>
		fs->dev_desc->log2blksz);

	temp_ptr = zalloc(fs->blksz);
	if (!temp_ptr)
		goto fail;
	ibmap_idx = inodeno / le32_to_cpu(ext4fs_root->sblock.inodes_per_group);
	inodeno--;
	bgd = ext4fs_get_group_descriptor(fs, ibmap_idx);
	itable_blkno = ext4fs_bg_get_inode_table_id(bgd, fs) +
			(inodeno % le32_to_cpu(sblock->inodes_per_group)) /
			inodes_per_block;
	blkoff = (inodeno % inodes_per_block) * fs->inodesz;
	ext4fs_devread((lbaint_t)itable_blkno * fs->sect_perblk, 0, fs->blksz,
		       temp_ptr);
	if (ext4fs_log_journal(temp_ptr, itable_blkno))
		goto fail;

	memcpy(temp_ptr + blkoff, inode_buffer, fs->inodesz);
	if (ext4fs_put_metadata(temp_ptr, itable_blkno))
		goto fail;
	/* copy the file content into data blocks */
	if (ext4fs_write_file(file_inode, 0, sizebytes, buffer) == -1) {
		printf("Error in copying content\n");
		/* FIXME: Deallocate data blocks */
		goto fail;
	}
	ibmap_idx = parent_inodeno / le32_to_cpu(ext4fs_root->sblock.inodes_per_group);
	parent_inodeno--;
	bgd = ext4fs_get_group_descriptor(fs, ibmap_idx);
	parent_itable_blkno = ext4fs_bg_get_inode_table_id(bgd, fs) +
	    (parent_inodeno %
	     le32_to_cpu(sblock->inodes_per_group)) / inodes_per_block;
	blkoff = (parent_inodeno % inodes_per_block) * fs->inodesz;
	if (parent_itable_blkno != itable_blkno) {
		memset(temp_ptr, '\0', fs->blksz);
		ext4fs_devread((lbaint_t)parent_itable_blkno * fs->sect_perblk,
			       0, fs->blksz, temp_ptr);
		if (ext4fs_log_journal(temp_ptr, parent_itable_blkno))
			goto fail;

		memcpy(temp_ptr + blkoff, g_parent_inode, fs->inodesz);
		if (ext4fs_put_metadata(temp_ptr, parent_itable_blkno))
			goto fail;
	} else {
		/*
		 * If parent and child fall in same inode table block
		 * both should be kept in 1 buffer
		 */
		memcpy(temp_ptr + blkoff, g_parent_inode, fs->inodesz);
		gd_index--;
		if (ext4fs_put_metadata(temp_ptr, itable_blkno))
			goto fail;
	}
	ext4fs_update();
	ext4fs_deinit();

	fs->first_pass_bbmap = 0;
	fs->curr_blkno = 0;
	fs->first_pass_ibmap = 0;
	fs->curr_inode_no = 0;
	free(inode_buffer);
	free(g_parent_inode);
	free(temp_ptr);
	g_parent_inode = NULL;

	return 0;
fail:
	ext4fs_deinit();
	free(inode_buffer);
	free(g_parent_inode);
	free(temp_ptr);
	g_parent_inode = NULL;

	return -1;
}

int ext4_write_file(const char *filename, void *buf, loff_t offset,
		    loff_t len, loff_t *actwrite)
{
	int ret;

	if (offset != 0) {
		printf("** Cannot support non-zero offset **\n");
		return -1;
	}

	ret = ext4fs_write(filename, buf, len, FILETYPE_REG);
	if (ret) {
		printf("** Error ext4fs_write() **\n");
		goto fail;
	}

	*actwrite = len;

	return 0;

fail:
	*actwrite = 0;

	return -1;
}

int ext4fs_create_link(const char *target, const char *fname)
{
	return ext4fs_write(fname, target, strlen(target), FILETYPE_SYMLINK);
}
