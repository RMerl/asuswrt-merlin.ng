/*
 * readahead.c -- Prefetch filesystem metadata to speed up fsck.
 *
 * Copyright (C) 2014 Oracle.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#include "config.h"
#include <string.h>

#include "e2fsck.h"

#undef DEBUG

#ifdef DEBUG
# define dbg_printf(f, a...)  do {printf(f, ## a); fflush(stdout); } while (0)
#else
# define dbg_printf(f, a...)
#endif

struct read_dblist {
	errcode_t err;
	blk64_t run_start;
	blk64_t run_len;
	int flags;
};

static int readahead_dir_block(ext2_filsys fs, struct ext2_db_entry2 *db,
			       void *priv_data)
{
	struct read_dblist *pr = priv_data;
	e2_blkcnt_t count = (pr->flags & E2FSCK_RA_DBLIST_IGNORE_BLOCKCNT ?
			     1 : db->blockcnt);

	if (!pr->run_len || db->blk != pr->run_start + pr->run_len) {
		if (pr->run_len) {
			pr->err = io_channel_cache_readahead(fs->io,
							     pr->run_start,
							     pr->run_len);
			dbg_printf("readahead start=%llu len=%llu err=%d\n",
				   pr->run_start, pr->run_len,
				   (int)pr->err);
		}
		pr->run_start = db->blk;
		pr->run_len = 0;
	}
	pr->run_len += count;

	return pr->err ? DBLIST_ABORT : 0;
}

errcode_t e2fsck_readahead_dblist(ext2_filsys fs, int flags,
				  ext2_dblist dblist,
				  unsigned long long start,
				  unsigned long long count)
{
	errcode_t err;
	struct read_dblist pr;

	dbg_printf("%s: flags=0x%x\n", __func__, flags);
	if (flags & ~E2FSCK_RA_DBLIST_ALL_FLAGS)
		return EXT2_ET_INVALID_ARGUMENT;

	memset(&pr, 0, sizeof(pr));
	pr.flags = flags;
	err = ext2fs_dblist_iterate3(dblist, readahead_dir_block, start,
				     count, &pr);
	if (pr.err)
		return pr.err;
	if (err)
		return err;

	if (pr.run_len)
		err = io_channel_cache_readahead(fs->io, pr.run_start,
						 pr.run_len);

	return err;
}

static errcode_t e2fsck_readahead_bitmap(ext2_filsys fs,
					 ext2fs_block_bitmap ra_map)
{
	blk64_t start, end, out;
	errcode_t err;

	start = 1;
	end = ext2fs_blocks_count(fs->super) - 1;

	err = ext2fs_find_first_set_block_bitmap2(ra_map, start, end, &out);
	while (err == 0) {
		start = out;
		err = ext2fs_find_first_zero_block_bitmap2(ra_map, start, end,
							   &out);
		if (err == ENOENT) {
			out = end;
			err = 0;
			if (out == start)
				break;
		} else if (err)
			break;

		err = io_channel_cache_readahead(fs->io, start, out - start);
		if (err)
			break;
		start = out;
		err = ext2fs_find_first_set_block_bitmap2(ra_map, start, end,
							  &out);
	}

	if (err == ENOENT)
		err = 0;

	return err;
}

/* Try not to spew bitmap range errors for readahead */
static errcode_t mark_bmap_range(ext2fs_block_bitmap map,
				 blk64_t blk, unsigned int num)
{
	if (blk >= ext2fs_get_generic_bmap_start(map) &&
	    blk + num <= ext2fs_get_generic_bmap_end(map))
		ext2fs_mark_block_bitmap_range2(map, blk, num);
	else
		return EXT2_ET_INVALID_ARGUMENT;
	return 0;
}

static errcode_t mark_bmap(ext2fs_block_bitmap map, blk64_t blk)
{
	if (blk >= ext2fs_get_generic_bmap_start(map) &&
	    blk <= ext2fs_get_generic_bmap_end(map))
		ext2fs_mark_block_bitmap2(map, blk);
	else
		return EXT2_ET_INVALID_ARGUMENT;
	return 0;
}

errcode_t e2fsck_readahead(ext2_filsys fs, int flags, dgrp_t start,
			   dgrp_t ngroups)
{
	blk64_t		super, old_gdt, new_gdt;
	blk_t		blocks;
	dgrp_t		i;
	ext2fs_block_bitmap		ra_map = NULL;
	dgrp_t		end = start + ngroups;
	errcode_t	err = 0;

	dbg_printf("%s: flags=0x%x start=%d groups=%d\n", __func__, flags,
		   start, ngroups);
	if (flags & ~E2FSCK_READA_ALL_FLAGS)
		return EXT2_ET_INVALID_ARGUMENT;

	if (end > fs->group_desc_count)
		end = fs->group_desc_count;

	if (flags == 0)
		return 0;

	err = ext2fs_allocate_block_bitmap(fs, "readahead bitmap",
					   &ra_map);
	if (err)
		return err;

	for (i = start; i < end; i++) {
		err = ext2fs_super_and_bgd_loc2(fs, i, &super, &old_gdt,
						&new_gdt, &blocks);
		if (err)
			break;

		if (flags & E2FSCK_READA_SUPER) {
			err = mark_bmap(ra_map, super);
			if (err)
				break;
		}

		if (flags & E2FSCK_READA_GDT) {
			err = mark_bmap_range(ra_map,
					      old_gdt ? old_gdt : new_gdt,
					      blocks);
			if (err)
				break;
		}

		if ((flags & E2FSCK_READA_BBITMAP) &&
		    !ext2fs_bg_flags_test(fs, i, EXT2_BG_BLOCK_UNINIT) &&
		    ext2fs_bg_free_blocks_count(fs, i) <
				fs->super->s_blocks_per_group) {
			super = ext2fs_block_bitmap_loc(fs, i);
			err = mark_bmap(ra_map, super);
			if (err)
				break;
		}

		if ((flags & E2FSCK_READA_IBITMAP) &&
		    !ext2fs_bg_flags_test(fs, i, EXT2_BG_INODE_UNINIT) &&
		    ext2fs_bg_free_inodes_count(fs, i) <
				fs->super->s_inodes_per_group) {
			super = ext2fs_inode_bitmap_loc(fs, i);
			err = mark_bmap(ra_map, super);
			if (err)
				break;
		}

		if ((flags & E2FSCK_READA_ITABLE) &&
		    ext2fs_bg_free_inodes_count(fs, i) <
				fs->super->s_inodes_per_group) {
			super = ext2fs_inode_table_loc(fs, i);
			blocks = fs->inode_blocks_per_group -
				 (ext2fs_bg_itable_unused(fs, i) *
				  EXT2_INODE_SIZE(fs->super) / fs->blocksize);
			err = mark_bmap_range(ra_map, super, blocks);
			if (err)
				break;
		}
	}

	if (!err)
		err = e2fsck_readahead_bitmap(fs, ra_map);

	ext2fs_free_block_bitmap(ra_map);
	return err;
}

int e2fsck_can_readahead(ext2_filsys fs)
{
	errcode_t err;

	err = io_channel_cache_readahead(fs->io, 0, 1);
	dbg_printf("%s: supp=%d\n", __func__, err != EXT2_ET_OP_NOT_SUPPORTED);
	return err != EXT2_ET_OP_NOT_SUPPORTED;
}

unsigned long long e2fsck_guess_readahead(ext2_filsys fs)
{
	unsigned long long guess;

	/*
	 * The optimal readahead sizes were experimentally determined by
	 * djwong in August 2014.  Setting the RA size to two block groups'
	 * worth of inode table blocks seems to yield the largest reductions
	 * in e2fsck runtime.
	 */
	guess = 2ULL * fs->blocksize * fs->inode_blocks_per_group;

	/* Disable RA if it'd use more 1/50th of RAM. */
	if (get_memory_size() > (guess * 50))
		return guess / 1024;

	return 0;
}
