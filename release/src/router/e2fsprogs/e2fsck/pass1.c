/*
 * pass1.c -- pass #1 of e2fsck: sequential scan of the inode table
 *
 * Copyright (C) 1993, 1994, 1995, 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 *
 * Pass 1 of e2fsck iterates over all the inodes in the filesystems,
 * and applies the following tests to each inode:
 *
 * 	- The mode field of the inode must be legal.
 * 	- The size and block count fields of the inode are correct.
 * 	- A data block must not be used by another inode
 *
 * Pass 1 also gathers the collects the following information:
 *
 * 	- A bitmap of which inodes are in use.		(inode_used_map)
 * 	- A bitmap of which inodes are directories.	(inode_dir_map)
 * 	- A bitmap of which inodes are regular files.	(inode_reg_map)
 * 	- A bitmap of which inodes have bad fields.	(inode_bad_map)
 * 	- A bitmap of which inodes are in bad blocks.	(inode_bb_map)
 * 	- A bitmap of which inodes are imagic inodes.	(inode_imagic_map)
 * 	- A bitmap of which blocks are in use.		(block_found_map)
 * 	- A bitmap of which blocks are in use by two inodes	(block_dup_map)
 * 	- The data blocks of the directory inodes.	(dir_map)
 * 	- Ref counts for ea_inodes.			(ea_inode_refs)
 *
 * Pass 1 is designed to stash away enough information so that the
 * other passes should not need to read in the inode information
 * during the normal course of a filesystem check.  (Although if an
 * inconsistency is detected, other passes may need to read in an
 * inode to fix it.)
 *
 * Note that pass 1B will be invoked if there are any duplicate blocks
 * found.
 */

#define _GNU_SOURCE 1 /* get strnlen() */
#include "config.h"
#include <string.h>
#include <time.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "e2fsck.h"
#include <ext2fs/ext2_ext_attr.h>
#include <e2p/e2p.h>

#include "problem.h"

#ifdef NO_INLINE_FUNCS
#define _INLINE_
#else
#define _INLINE_ inline
#endif

#undef DEBUG

struct ea_quota {
	blk64_t blocks;
	__u64 inodes;
};

static int process_block(ext2_filsys fs, blk64_t	*blocknr,
			 e2_blkcnt_t blockcnt, blk64_t ref_blk,
			 int ref_offset, void *priv_data);
static int process_bad_block(ext2_filsys fs, blk64_t *block_nr,
			     e2_blkcnt_t blockcnt, blk64_t ref_blk,
			     int ref_offset, void *priv_data);
static void check_blocks(e2fsck_t ctx, struct problem_context *pctx,
			 char *block_buf,
			 const struct ea_quota *ea_ibody_quota);
static void mark_table_blocks(e2fsck_t ctx);
static void alloc_bb_map(e2fsck_t ctx);
static void alloc_imagic_map(e2fsck_t ctx);
static void mark_inode_bad(e2fsck_t ctx, ino_t ino);
static void add_encrypted_dir(e2fsck_t ctx, ino_t ino);
static void handle_fs_bad_blocks(e2fsck_t ctx);
static void process_inodes(e2fsck_t ctx, char *block_buf);
static EXT2_QSORT_TYPE process_inode_cmp(const void *a, const void *b);
static errcode_t scan_callback(ext2_filsys fs, ext2_inode_scan scan,
				  dgrp_t group, void * priv_data);
static void adjust_extattr_refcount(e2fsck_t ctx, ext2_refcount_t refcount,
				    char *block_buf, int adjust_sign);
/* static char *describe_illegal_block(ext2_filsys fs, blk64_t block); */

struct process_block_struct {
	ext2_ino_t	ino;
	unsigned	is_dir:1, is_reg:1, clear:1, suppress:1,
				fragmented:1, compressed:1, bbcheck:1,
				inode_modified:1;
	blk64_t		num_blocks;
	blk64_t		max_blocks;
	blk64_t		last_block;
	e2_blkcnt_t	last_init_lblock;
	e2_blkcnt_t	last_db_block;
	int		num_illegal_blocks;
	blk64_t		previous_block;
	struct ext2_inode *inode;
	struct problem_context *pctx;
	ext2fs_block_bitmap fs_meta_blocks;
	e2fsck_t	ctx;
	blk64_t		next_lblock;
	struct extent_tree_info	eti;
};

struct process_inode_block {
	ext2_ino_t ino;
	struct ea_quota ea_ibody_quota;
	struct ext2_inode_large inode;
};

struct scan_callback_struct {
	e2fsck_t	ctx;
	char		*block_buf;
};

/*
 * For the inodes to process list.
 */
static struct process_inode_block *inodes_to_process;
static int process_inode_count;

static __u64 ext2_max_sizes[EXT2_MAX_BLOCK_LOG_SIZE -
			    EXT2_MIN_BLOCK_LOG_SIZE + 1];

/*
 * Free all memory allocated by pass1 in preparation for restarting
 * things.
 */
static void unwind_pass1(ext2_filsys fs EXT2FS_ATTR((unused)))
{
	ext2fs_free_mem(&inodes_to_process);
	inodes_to_process = 0;
}

/*
 * Check to make sure a device inode is real.  Returns 1 if the device
 * checks out, 0 if not.
 *
 * Note: this routine is now also used to check FIFO's and Sockets,
 * since they have the same requirement; the i_block fields should be
 * zero.
 */
int e2fsck_pass1_check_device_inode(ext2_filsys fs EXT2FS_ATTR((unused)),
				    struct ext2_inode *inode)
{
	int	i;

	/*
	 * If the index or extents flag is set, then this is a bogus
	 * device/fifo/socket
	 */
	if (inode->i_flags & (EXT2_INDEX_FL | EXT4_EXTENTS_FL))
		return 0;

	/*
	 * We should be able to do the test below all the time, but
	 * because the kernel doesn't forcibly clear the device
	 * inode's additional i_block fields, there are some rare
	 * occasions when a legitimate device inode will have non-zero
	 * additional i_block fields.  So for now, we only complain
	 * when the immutable flag is set, which should never happen
	 * for devices.  (And that's when the problem is caused, since
	 * you can't set or clear immutable flags for devices.)  Once
	 * the kernel has been fixed we can change this...
	 */
	if (inode->i_flags & (EXT2_IMMUTABLE_FL | EXT2_APPEND_FL)) {
		for (i=4; i < EXT2_N_BLOCKS; i++)
			if (inode->i_block[i])
				return 0;
	}
	return 1;
}

/*
 * Check to make sure a symlink inode is real.  Returns 1 if the symlink
 * checks out, 0 if not.
 */
int e2fsck_pass1_check_symlink(ext2_filsys fs, ext2_ino_t ino,
			       struct ext2_inode *inode, char *buf)
{
	unsigned int buflen;
	unsigned int len;

	if ((inode->i_size_high || inode->i_size == 0) ||
	    (inode->i_flags & EXT2_INDEX_FL))
		return 0;

	if (inode->i_flags & EXT4_INLINE_DATA_FL) {
		size_t inline_size;

		if (inode->i_flags & EXT4_EXTENTS_FL)
			return 0;
		if (ext2fs_inline_data_size(fs, ino, &inline_size))
			return 0;
		if (inode->i_size != inline_size)
			return 0;

		return 1;
	}

	if (ext2fs_is_fast_symlink(inode)) {
		if (inode->i_flags & EXT4_EXTENTS_FL)
			return 0;
		buf = (char *)inode->i_block;
		buflen = sizeof(inode->i_block);
	} else {
		ext2_extent_handle_t	handle;
		struct ext2_extent_info	info;
		struct ext2fs_extent	extent;
		blk64_t blk;
		int i;

		if (inode->i_flags & EXT4_EXTENTS_FL) {
			if (ext2fs_extent_open2(fs, ino, inode, &handle))
				return 0;
			if (ext2fs_extent_get_info(handle, &info) ||
			    (info.num_entries != 1) ||
			    (info.max_depth != 0)) {
				ext2fs_extent_free(handle);
				return 0;
			}
			if (ext2fs_extent_get(handle, EXT2_EXTENT_ROOT,
					      &extent) ||
			    (extent.e_lblk != 0) ||
			    (extent.e_len != 1)) {
				ext2fs_extent_free(handle);
				return 0;
			}
			blk = extent.e_pblk;
			ext2fs_extent_free(handle);
		} else {
			blk = inode->i_block[0];

			for (i = 1; i < EXT2_N_BLOCKS; i++)
				if (inode->i_block[i])
					return 0;
		}

		if (blk < fs->super->s_first_data_block ||
		    blk >= ext2fs_blocks_count(fs->super))
			return 0;

		if (io_channel_read_blk64(fs->io, blk, 1, buf))
			return 0;

		buflen = fs->blocksize;
	}

	if (inode->i_flags & EXT4_ENCRYPT_FL)
		len = ext2fs_le16_to_cpu(*(__u16 *)buf) + 2;
	else
		len = strnlen(buf, buflen);

	if (len >= buflen)
		return 0;

	if (len != inode->i_size)
		return 0;
	return 1;
}

/*
 * If the extents or inlinedata flags are set on the inode, offer to clear 'em.
 */
#define BAD_SPECIAL_FLAGS (EXT4_EXTENTS_FL | EXT4_INLINE_DATA_FL)
static void check_extents_inlinedata(e2fsck_t ctx,
				     struct problem_context *pctx)
{
	if (!(pctx->inode->i_flags & BAD_SPECIAL_FLAGS))
		return;

	if (!fix_problem(ctx, PR_1_SPECIAL_EXTENTS_IDATA, pctx))
		return;

	pctx->inode->i_flags &= ~BAD_SPECIAL_FLAGS;
	e2fsck_write_inode(ctx, pctx->ino, pctx->inode, "pass1");
}
#undef BAD_SPECIAL_FLAGS

/*
 * If the immutable (or append-only) flag is set on the inode, offer
 * to clear it.
 */
#define BAD_SPECIAL_FLAGS (EXT2_IMMUTABLE_FL | EXT2_APPEND_FL)
static void check_immutable(e2fsck_t ctx, struct problem_context *pctx)
{
	if (!(pctx->inode->i_flags & BAD_SPECIAL_FLAGS))
		return;

	if (!fix_problem(ctx, PR_1_SET_IMMUTABLE, pctx))
		return;

	pctx->inode->i_flags &= ~BAD_SPECIAL_FLAGS;
	e2fsck_write_inode(ctx, pctx->ino, pctx->inode, "pass1");
}

/*
 * If device, fifo or socket, check size is zero -- if not offer to
 * clear it
 */
static void check_size(e2fsck_t ctx, struct problem_context *pctx)
{
	struct ext2_inode *inode = pctx->inode;

	if (EXT2_I_SIZE(inode) == 0)
		return;

	if (!fix_problem(ctx, PR_1_SET_NONZSIZE, pctx))
		return;

	ext2fs_inode_size_set(ctx->fs, inode, 0);
	e2fsck_write_inode(ctx, pctx->ino, pctx->inode, "pass1");
}

/*
 * For a given size, calculate how many blocks would be charged towards quota.
 */
static blk64_t size_to_quota_blocks(ext2_filsys fs, size_t size)
{
	blk64_t clusters;

	clusters = DIV_ROUND_UP(size, fs->blocksize << fs->cluster_ratio_bits);
	return EXT2FS_C2B(fs, clusters);
}

/*
 * Check validity of EA inode. Return 0 if EA inode is valid, otherwise return
 * the problem code.
 */
static problem_t check_large_ea_inode(e2fsck_t ctx,
				      struct ext2_ext_attr_entry *entry,
				      struct problem_context *pctx,
				      blk64_t *quota_blocks)
{
	struct ext2_inode inode;
	__u32 hash;
	errcode_t retval;

	/* Check if inode is within valid range */
	if ((entry->e_value_inum < EXT2_FIRST_INODE(ctx->fs->super)) ||
	    (entry->e_value_inum > ctx->fs->super->s_inodes_count)) {
		pctx->num = entry->e_value_inum;
		return PR_1_ATTR_VALUE_EA_INODE;
	}

	e2fsck_read_inode(ctx, entry->e_value_inum, &inode, "pass1");

	retval = ext2fs_ext_attr_hash_entry2(ctx->fs, entry, NULL, &hash);
	if (retval) {
		com_err("check_large_ea_inode", retval,
			_("while hashing entry with e_value_inum = %u"),
			entry->e_value_inum);
		fatal_error(ctx, 0);
	}

	if (hash == entry->e_hash) {
		*quota_blocks = size_to_quota_blocks(ctx->fs,
						     entry->e_value_size);
	} else {
		/* This might be an old Lustre-style ea_inode reference. */
		if (inode.i_mtime == pctx->ino &&
		    inode.i_generation == pctx->inode->i_generation) {
			*quota_blocks = 0;
		} else {
			/* If target inode is also missing EA_INODE flag,
			 * this is likely to be a bad reference.
			 */
			if (!(inode.i_flags & EXT4_EA_INODE_FL)) {
				pctx->num = entry->e_value_inum;
				return PR_1_ATTR_VALUE_EA_INODE;
			} else {
				pctx->num = entry->e_hash;
				return PR_1_ATTR_HASH;
			}
		}
	}

	if (!(inode.i_flags & EXT4_EA_INODE_FL)) {
		pctx->num = entry->e_value_inum;
		if (fix_problem(ctx, PR_1_ATTR_SET_EA_INODE_FL, pctx)) {
			inode.i_flags |= EXT4_EA_INODE_FL;
			ext2fs_write_inode(ctx->fs, entry->e_value_inum,
					   &inode);
		} else {
			return PR_1_ATTR_NO_EA_INODE_FL;
		}
	}
	return 0;
}

static void inc_ea_inode_refs(e2fsck_t ctx, struct problem_context *pctx,
			      struct ext2_ext_attr_entry *first, void *end)
{
	struct ext2_ext_attr_entry *entry;

	for (entry = first;
	     (void *)entry < end && !EXT2_EXT_IS_LAST_ENTRY(entry);
	     entry = EXT2_EXT_ATTR_NEXT(entry)) {
		if (!entry->e_value_inum)
			continue;
		if (!ctx->ea_inode_refs) {
			pctx->errcode = ea_refcount_create(0,
							   &ctx->ea_inode_refs);
			if (pctx->errcode) {
				pctx->num = 4;
				fix_problem(ctx, PR_1_ALLOCATE_REFCOUNT, pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				return;
			}
		}
		ea_refcount_increment(ctx->ea_inode_refs, entry->e_value_inum,
				      0);
	}
}

static void check_ea_in_inode(e2fsck_t ctx, struct problem_context *pctx,
			      struct ea_quota *ea_ibody_quota)
{
	struct ext2_super_block *sb = ctx->fs->super;
	struct ext2_inode_large *inode;
	struct ext2_ext_attr_entry *entry;
	char *start, *header, *end;
	unsigned int storage_size, remain;
	problem_t problem = 0;
	region_t region = 0;

	ea_ibody_quota->blocks = 0;
	ea_ibody_quota->inodes = 0;

	inode = (struct ext2_inode_large *) pctx->inode;
	storage_size = EXT2_INODE_SIZE(ctx->fs->super) - EXT2_GOOD_OLD_INODE_SIZE -
		inode->i_extra_isize;
	header = ((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
		 inode->i_extra_isize;
	end = header + storage_size;
	start = header + sizeof(__u32);
	entry = (struct ext2_ext_attr_entry *) start;

	/* scan all entry's headers first */

	/* take finish entry 0UL into account */
	remain = storage_size - sizeof(__u32);

	region = region_create(0, storage_size);
	if (!region) {
		fix_problem(ctx, PR_1_EA_ALLOC_REGION_ABORT, pctx);
		problem = 0;
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	if (region_allocate(region, 0, sizeof(__u32))) {
		problem = PR_1_INODE_EA_ALLOC_COLLISION;
		goto fix;
	}

	while (remain >= sizeof(struct ext2_ext_attr_entry) &&
	       !EXT2_EXT_IS_LAST_ENTRY(entry)) {
		__u32 hash;

		if (region_allocate(region, (char *)entry - (char *)header,
				    EXT2_EXT_ATTR_LEN(entry->e_name_len))) {
			problem = PR_1_INODE_EA_ALLOC_COLLISION;
			goto fix;
		}

		/* header eats this space */
		remain -= sizeof(struct ext2_ext_attr_entry);

		/* is attribute name valid? */
		if (EXT2_EXT_ATTR_SIZE(entry->e_name_len) > remain) {
			pctx->num = entry->e_name_len;
			problem = PR_1_ATTR_NAME_LEN;
			goto fix;
		}

		/* attribute len eats this space */
		remain -= EXT2_EXT_ATTR_SIZE(entry->e_name_len);

		if (entry->e_value_inum == 0) {
			/* check value size */
			if (entry->e_value_size > remain) {
				pctx->num = entry->e_value_size;
				problem = PR_1_ATTR_VALUE_SIZE;
				goto fix;
			}

			if (entry->e_value_size &&
			    region_allocate(region,
					    sizeof(__u32) + entry->e_value_offs,
					    EXT2_EXT_ATTR_SIZE(
						entry->e_value_size))) {
				problem = PR_1_INODE_EA_ALLOC_COLLISION;
				goto fix;
			}

			hash = ext2fs_ext_attr_hash_entry(entry,
							  start + entry->e_value_offs);

			/* e_hash may be 0 in older inode's ea */
			if (entry->e_hash != 0 && entry->e_hash != hash) {
				pctx->num = entry->e_hash;
				problem = PR_1_ATTR_HASH;
				goto fix;
			}
		} else {
			blk64_t quota_blocks;

			problem = check_large_ea_inode(ctx, entry, pctx,
						       &quota_blocks);
			if (problem != 0)
				goto fix;

			ea_ibody_quota->blocks += quota_blocks;
			ea_ibody_quota->inodes++;
		}

		/* If EA value is stored in external inode then it does not
		 * consume space here */
		if (entry->e_value_inum == 0)
			remain -= entry->e_value_size;

		entry = EXT2_EXT_ATTR_NEXT(entry);
	}

	if (region_allocate(region, (char *)entry - (char *)header,
			    sizeof(__u32))) {
		problem = PR_1_INODE_EA_ALLOC_COLLISION;
		goto fix;
	}
fix:
	if (region)
		region_free(region);
	/*
	 * it seems like a corruption. it's very unlikely we could repair
	 * EA(s) in automatic fashion -bzzz
	 */
	if (problem == 0 || !fix_problem(ctx, problem, pctx)) {
		inc_ea_inode_refs(ctx, pctx,
				  (struct ext2_ext_attr_entry *)start, end);
		return;
	}

	/* simply remove all possible EA(s) */
	*((__u32 *)header) = 0UL;
	e2fsck_write_inode_full(ctx, pctx->ino, pctx->inode,
				EXT2_INODE_SIZE(sb), "pass1");
	ea_ibody_quota->blocks = 0;
	ea_ibody_quota->inodes = 0;
}

static int check_inode_extra_negative_epoch(__u32 xtime, __u32 extra) {
	return (xtime & (1U << 31)) != 0 &&
		(extra & EXT4_EPOCH_MASK) == EXT4_EPOCH_MASK;
}

#define CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, xtime) \
	check_inode_extra_negative_epoch(inode->i_##xtime, \
					 inode->i_##xtime##_extra)

/* When today's date is earlier than 2242, we assume that atimes,
 * ctimes, crtimes, and mtimes with years in the range 2310..2378 are
 * actually pre-1970 dates mis-encoded.
 */
#define EXT4_EXTRA_NEGATIVE_DATE_CUTOFF 2 * (1LL << 32)

static void check_inode_extra_space(e2fsck_t ctx, struct problem_context *pctx,
				    struct ea_quota *ea_ibody_quota)
{
	struct ext2_super_block *sb = ctx->fs->super;
	struct ext2_inode_large *inode;
	__u32 *eamagic;
	int min, max;

	ea_ibody_quota->blocks = 0;
	ea_ibody_quota->inodes = 0;

	inode = (struct ext2_inode_large *) pctx->inode;
	if (EXT2_INODE_SIZE(sb) == EXT2_GOOD_OLD_INODE_SIZE) {
		/* this isn't large inode. so, nothing to check */
		return;
	}

#if 0
	printf("inode #%u, i_extra_size %d\n", pctx->ino,
			inode->i_extra_isize);
#endif
	/* i_extra_isize must cover i_extra_isize + i_checksum_hi at least */
	min = sizeof(inode->i_extra_isize) + sizeof(inode->i_checksum_hi);
	max = EXT2_INODE_SIZE(sb) - EXT2_GOOD_OLD_INODE_SIZE;
	/*
	 * For now we will allow i_extra_isize to be 0, but really
	 * implementations should never allow i_extra_isize to be 0
	 */
	if (inode->i_extra_isize &&
	    (inode->i_extra_isize < min || inode->i_extra_isize > max ||
	     inode->i_extra_isize & 3)) {
		if (!fix_problem(ctx, PR_1_EXTRA_ISIZE, pctx))
			return;
		if (inode->i_extra_isize < min || inode->i_extra_isize > max)
			inode->i_extra_isize = sb->s_want_extra_isize;
		else
			inode->i_extra_isize = (inode->i_extra_isize + 3) & ~3;
		e2fsck_write_inode_full(ctx, pctx->ino, pctx->inode,
					EXT2_INODE_SIZE(sb), "pass1");
	}

	/* check if there is no place for an EA header */
	if (inode->i_extra_isize >= max - sizeof(__u32))
		return;

	eamagic = (__u32 *) (((char *) inode) + EXT2_GOOD_OLD_INODE_SIZE +
			inode->i_extra_isize);
	if (*eamagic == EXT2_EXT_ATTR_MAGIC) {
		/* it seems inode has an extended attribute(s) in body */
		check_ea_in_inode(ctx, pctx, ea_ibody_quota);
	}

	/*
	 * If the inode's extended atime (ctime, crtime, mtime) is stored in
	 * the old, invalid format, repair it.
	 */
	if (((sizeof(time_t) <= 4) ||
	     (((sizeof(time_t) > 4) &&
	       ctx->now < EXT4_EXTRA_NEGATIVE_DATE_CUTOFF))) &&
	    (CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, atime) ||
	     CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, ctime) ||
	     CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, crtime) ||
	     CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, mtime))) {

		if (!fix_problem(ctx, PR_1_EA_TIME_OUT_OF_RANGE, pctx))
			return;

		if (CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, atime))
			inode->i_atime_extra &= ~EXT4_EPOCH_MASK;
		if (CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, ctime))
			inode->i_ctime_extra &= ~EXT4_EPOCH_MASK;
		if (CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, crtime))
			inode->i_crtime_extra &= ~EXT4_EPOCH_MASK;
		if (CHECK_INODE_EXTRA_NEGATIVE_EPOCH(inode, mtime))
			inode->i_mtime_extra &= ~EXT4_EPOCH_MASK;
		e2fsck_write_inode_full(ctx, pctx->ino, pctx->inode,
					EXT2_INODE_SIZE(sb), "pass1");
	}

}

/*
 * Check to see if the inode might really be a directory, despite i_mode
 *
 * This is a lot of complexity for something for which I'm not really
 * convinced happens frequently in the wild.  If for any reason this
 * causes any problems, take this code out.
 * [tytso:20070331.0827EDT]
 */
static void check_is_really_dir(e2fsck_t ctx, struct problem_context *pctx,
				char *buf)
{
	struct ext2_inode *inode = pctx->inode;
	struct ext2_dir_entry 	*dirent;
	errcode_t		retval;
	blk64_t			blk;
	unsigned int		i, rec_len, not_device = 0;
	int			extent_fs;
	int			inlinedata_fs;

	/*
	 * If the mode looks OK, we believe it.  If the first block in
	 * the i_block array is 0, this cannot be a directory. If the
	 * inode is extent-mapped, it is still the case that the latter
	 * cannot be 0 - the magic number in the extent header would make
	 * it nonzero.
	 */
	if (LINUX_S_ISDIR(inode->i_mode) || LINUX_S_ISREG(inode->i_mode) ||
	    LINUX_S_ISLNK(inode->i_mode) || inode->i_block[0] == 0)
		return;

	/* 
	 * Check the block numbers in the i_block array for validity:
	 * zero blocks are skipped (but the first one cannot be zero -
	 * see above), other blocks are checked against the first and
	 * max data blocks (from the the superblock) and against the
	 * block bitmap. Any invalid block found means this cannot be
	 * a directory.
	 * 
	 * If there are non-zero blocks past the fourth entry, then
	 * this cannot be a device file: we remember that for the next
	 * check.
	 *
	 * For extent mapped files, we don't do any sanity checking:
	 * just try to get the phys block of logical block 0 and run
	 * with it.
	 *
	 * For inline data files, we just try to get the size of inline
	 * data.  If it's true, we will treat it as a directory.
	 */

	extent_fs = ext2fs_has_feature_extents(ctx->fs->super);
	inlinedata_fs = ext2fs_has_feature_inline_data(ctx->fs->super);
	if (inlinedata_fs && (inode->i_flags & EXT4_INLINE_DATA_FL)) {
		size_t size;
		__u32 dotdot;
		unsigned int rec_len2;
		struct ext2_dir_entry de;

		if (ext2fs_inline_data_size(ctx->fs, pctx->ino, &size))
			return;
		/*
		 * If the size isn't a multiple of 4, it's probably not a
		 * directory??
		 */
		if (size & 3)
			return;
		/*
		 * If the first 10 bytes don't look like a directory entry,
		 * it's probably not a directory.
		 */
		memcpy(&dotdot, inode->i_block, sizeof(dotdot));
		memcpy(&de, ((char *)inode->i_block) + EXT4_INLINE_DATA_DOTDOT_SIZE,
		       EXT2_DIR_REC_LEN(0));
		dotdot = ext2fs_le32_to_cpu(dotdot);
		de.inode = ext2fs_le32_to_cpu(de.inode);
		de.rec_len = ext2fs_le16_to_cpu(de.rec_len);
		ext2fs_get_rec_len(ctx->fs, &de, &rec_len2);
		if (dotdot >= ctx->fs->super->s_inodes_count ||
		    (dotdot < EXT2_FIRST_INO(ctx->fs->super) &&
		     dotdot != EXT2_ROOT_INO) ||
		    de.inode >= ctx->fs->super->s_inodes_count ||
		    (de.inode < EXT2_FIRST_INO(ctx->fs->super) &&
		     de.inode != 0) ||
		    rec_len2 > EXT4_MIN_INLINE_DATA_SIZE -
			      EXT4_INLINE_DATA_DOTDOT_SIZE)
			return;
		/* device files never have a "system.data" entry */
		goto isdir;
	} else if (extent_fs && (inode->i_flags & EXT4_EXTENTS_FL)) {
		/* extent mapped */
		if  (ext2fs_bmap2(ctx->fs, pctx->ino, inode, 0, 0, 0, 0,
				 &blk))
			return;
		/* device files are never extent mapped */
		not_device++;
	} else {
		for (i=0; i < EXT2_N_BLOCKS; i++) {
			blk = inode->i_block[i];
			if (!blk)
				continue;
			if (i >= 4)
				not_device++;

			if (blk < ctx->fs->super->s_first_data_block ||
			    blk >= ext2fs_blocks_count(ctx->fs->super) ||
			    ext2fs_fast_test_block_bitmap2(ctx->block_found_map,
							   blk))
				return;	/* Invalid block, can't be dir */
		}
		blk = inode->i_block[0];
	}

	/*
	 * If the mode says this is a device file and the i_links_count field
	 * is sane and we have not ruled it out as a device file previously,
	 * we declare it a device file, not a directory.
	 */
	if ((LINUX_S_ISCHR(inode->i_mode) || LINUX_S_ISBLK(inode->i_mode)) &&
	    (inode->i_links_count == 1) && !not_device)
		return;

	/* read the first block */
	ehandler_operation(_("reading directory block"));
	retval = ext2fs_read_dir_block4(ctx->fs, blk, buf, 0, pctx->ino);
	ehandler_operation(0);
	if (retval)
		return;

	dirent = (struct ext2_dir_entry *) buf;
	retval = ext2fs_get_rec_len(ctx->fs, dirent, &rec_len);
	if (retval)
		return;
	if ((ext2fs_dirent_name_len(dirent) != 1) ||
	    (dirent->name[0] != '.') ||
	    (dirent->inode != pctx->ino) ||
	    (rec_len < 12) ||
	    (rec_len % 4) ||
	    (rec_len >= ctx->fs->blocksize - 12))
		return;

	dirent = (struct ext2_dir_entry *) (buf + rec_len);
	retval = ext2fs_get_rec_len(ctx->fs, dirent, &rec_len);
	if (retval)
		return;
	if ((ext2fs_dirent_name_len(dirent) != 2) ||
	    (dirent->name[0] != '.') ||
	    (dirent->name[1] != '.') ||
	    (rec_len < 12) ||
	    (rec_len % 4))
		return;

isdir:
	if (fix_problem(ctx, PR_1_TREAT_AS_DIRECTORY, pctx)) {
		inode->i_mode = (inode->i_mode & 07777) | LINUX_S_IFDIR;
		e2fsck_write_inode_full(ctx, pctx->ino, inode,
					EXT2_INODE_SIZE(ctx->fs->super),
					"check_is_really_dir");
	}
}

extern errcode_t e2fsck_setup_icount(e2fsck_t ctx, const char *icount_name,
				     int flags, ext2_icount_t hint,
				     ext2_icount_t *ret)
{
	unsigned int		threshold;
	unsigned int		save_type;
	ext2_ino_t		num_dirs;
	errcode_t		retval;
	char			*tdb_dir;
	int			enable;

	*ret = 0;

	profile_get_string(ctx->profile, "scratch_files", "directory", 0, 0,
			   &tdb_dir);
	profile_get_uint(ctx->profile, "scratch_files",
			 "numdirs_threshold", 0, 0, &threshold);
	profile_get_boolean(ctx->profile, "scratch_files",
			    "icount", 0, 1, &enable);

	retval = ext2fs_get_num_dirs(ctx->fs, &num_dirs);
	if (retval)
		num_dirs = 1024;	/* Guess */

	if (enable && tdb_dir && !access(tdb_dir, W_OK) &&
	    (!threshold || num_dirs > threshold)) {
		retval = ext2fs_create_icount_tdb(ctx->fs, tdb_dir,
						  flags, ret);
		if (retval == 0)
			return 0;
	}
	e2fsck_set_bitmap_type(ctx->fs, EXT2FS_BMAP64_RBTREE, icount_name,
			       &save_type);
	if (ctx->options & E2F_OPT_ICOUNT_FULLMAP)
		flags |= EXT2_ICOUNT_OPT_FULLMAP;
	retval = ext2fs_create_icount2(ctx->fs, flags, 0, hint, ret);
	ctx->fs->default_bitmap_type = save_type;
	return retval;
}

static errcode_t recheck_bad_inode_checksum(ext2_filsys fs, ext2_ino_t ino,
					    e2fsck_t ctx,
					    struct problem_context *pctx)
{
	errcode_t retval;
	struct ext2_inode_large inode;

	/*
	 * Reread inode.  If we don't see checksum error, then this inode
	 * has been fixed elsewhere.
	 */
	ctx->stashed_ino = 0;
	retval = ext2fs_read_inode_full(fs, ino, (struct ext2_inode *)&inode,
					sizeof(inode));
	if (retval && retval != EXT2_ET_INODE_CSUM_INVALID)
		return retval;
	if (!retval)
		return 0;

	/*
	 * Checksum still doesn't match.  That implies that the inode passes
	 * all the sanity checks, so maybe the checksum is simply corrupt.
	 * See if the user will go for fixing that.
	 */
	if (!fix_problem(ctx, PR_1_INODE_ONLY_CSUM_INVALID, pctx))
		return 0;

	retval = ext2fs_write_inode_full(fs, ino, (struct ext2_inode *)&inode,
					 sizeof(inode));
	return retval;
}

static void reserve_block_for_root_repair(e2fsck_t ctx)
{
	blk64_t		blk = 0;
	errcode_t	err;
	ext2_filsys	fs = ctx->fs;

	ctx->root_repair_block = 0;
	if (ext2fs_test_inode_bitmap2(ctx->inode_used_map, EXT2_ROOT_INO))
		return;

	err = ext2fs_new_block2(fs, 0, ctx->block_found_map, &blk);
	if (err)
		return;
	ext2fs_mark_block_bitmap2(ctx->block_found_map, blk);
	ctx->root_repair_block = blk;
}

static void reserve_block_for_lnf_repair(e2fsck_t ctx)
{
	blk64_t		blk = 0;
	errcode_t	err;
	ext2_filsys	fs = ctx->fs;
	static const char name[] = "lost+found";
	ext2_ino_t	ino;

	ctx->lnf_repair_block = 0;
	if (!ext2fs_lookup(fs, EXT2_ROOT_INO, name, sizeof(name)-1, 0, &ino))
		return;

	err = ext2fs_new_block2(fs, 0, ctx->block_found_map, &blk);
	if (err)
		return;
	ext2fs_mark_block_bitmap2(ctx->block_found_map, blk);
	ctx->lnf_repair_block = blk;
}

static errcode_t get_inline_data_ea_size(ext2_filsys fs, ext2_ino_t ino,
					 size_t *sz)
{
	void *p;
	struct ext2_xattr_handle *handle;
	errcode_t retval;

	retval = ext2fs_xattrs_open(fs, ino, &handle);
	if (retval)
		return retval;

	retval = ext2fs_xattrs_read(handle);
	if (retval)
		goto err;

	retval = ext2fs_xattr_get(handle, "system.data", &p, sz);
	if (retval)
		goto err;
	ext2fs_free_mem(&p);
err:
	(void) ext2fs_xattrs_close(&handle);
	return retval;
}

static void finish_processing_inode(e2fsck_t ctx, ext2_ino_t ino,
				    struct problem_context *pctx,
				    int failed_csum)
{
	if (!failed_csum)
		return;

	/*
	 * If the inode failed the checksum and the user didn't
	 * clear the inode, test the checksum again -- if it still
	 * fails, ask the user if the checksum should be corrected.
	 */
	pctx->errcode = recheck_bad_inode_checksum(ctx->fs, ino, ctx, pctx);
	if (pctx->errcode)
		ctx->flags |= E2F_FLAG_ABORT;
}
#define FINISH_INODE_LOOP(ctx, ino, pctx, failed_csum) \
	do { \
		finish_processing_inode((ctx), (ino), (pctx), (failed_csum)); \
		if ((ctx)->flags & E2F_FLAG_ABORT) \
			return; \
	} while (0)

static int could_be_block_map(ext2_filsys fs, struct ext2_inode *inode)
{
	__u32 x;
	int i;

	for (i = 0; i < EXT2_N_BLOCKS; i++) {
		x = inode->i_block[i];
#ifdef WORDS_BIGENDIAN
		x = ext2fs_swab32(x);
#endif
		if (x >= ext2fs_blocks_count(fs->super))
			return 0;
	}

	return 1;
}

/*
 * Figure out what to do with an inode that has both extents and inline data
 * inode flags set.  Returns -1 if we decide to erase the inode, 0 otherwise.
 */
static int fix_inline_data_extents_file(e2fsck_t ctx,
					ext2_ino_t ino,
					struct ext2_inode *inode,
					int inode_size,
					struct problem_context *pctx)
{
	size_t max_inline_ea_size;
	ext2_filsys fs = ctx->fs;
	int dirty = 0;

	/* Both feature flags not set?  Just run the regular checks */
	if (!ext2fs_has_feature_extents(fs->super) &&
	    !ext2fs_has_feature_inline_data(fs->super))
		return 0;

	/* Clear both flags if it's a special file */
	if (LINUX_S_ISCHR(inode->i_mode) ||
	    LINUX_S_ISBLK(inode->i_mode) ||
	    LINUX_S_ISFIFO(inode->i_mode) ||
	    LINUX_S_ISSOCK(inode->i_mode)) {
		check_extents_inlinedata(ctx, pctx);
		return 0;
	}

	/* If it looks like an extent tree, try to clear inlinedata */
	if (ext2fs_extent_header_verify(inode->i_block,
				 sizeof(inode->i_block)) == 0 &&
	    fix_problem(ctx, PR_1_CLEAR_INLINE_DATA_FOR_EXTENT, pctx)) {
		inode->i_flags &= ~EXT4_INLINE_DATA_FL;
		dirty = 1;
		goto out;
	}

	/* If it looks short enough to be inline data, try to clear extents */
	if (inode_size > EXT2_GOOD_OLD_INODE_SIZE)
		max_inline_ea_size = inode_size -
				     (EXT2_GOOD_OLD_INODE_SIZE +
				      ((struct ext2_inode_large *)inode)->i_extra_isize);
	else
		max_inline_ea_size = 0;
	if (EXT2_I_SIZE(inode) <
	    EXT4_MIN_INLINE_DATA_SIZE + max_inline_ea_size &&
	    fix_problem(ctx, PR_1_CLEAR_EXTENT_FOR_INLINE_DATA, pctx)) {
		inode->i_flags &= ~EXT4_EXTENTS_FL;
		dirty = 1;
		goto out;
	}

	/*
	 * Too big for inline data, but no evidence of extent tree -
	 * maybe it's a block map file?  If the mappings all look valid?
	 */
	if (could_be_block_map(fs, inode) &&
	    fix_problem(ctx, PR_1_CLEAR_EXTENT_INLINE_DATA_FLAGS, pctx)) {
#ifdef WORDS_BIGENDIAN
		int i;

		for (i = 0; i < EXT2_N_BLOCKS; i++)
			inode->i_block[i] = ext2fs_swab32(inode->i_block[i]);
#endif

		inode->i_flags &= ~(EXT4_EXTENTS_FL | EXT4_INLINE_DATA_FL);
		dirty = 1;
		goto out;
	}

	/* Oh well, just clear the busted inode. */
	if (fix_problem(ctx, PR_1_CLEAR_EXTENT_INLINE_DATA_INODE, pctx)) {
		e2fsck_clear_inode(ctx, ino, inode, 0, "pass1");
		return -1;
	}

out:
	if (dirty)
		e2fsck_write_inode(ctx, ino, inode, "pass1");

	return 0;
}

static void pass1_readahead(e2fsck_t ctx, dgrp_t *group, ext2_ino_t *next_ino)
{
	ext2_ino_t inodes_in_group = 0, inodes_per_block, inodes_per_buffer;
	dgrp_t start = *group, grp;
	blk64_t blocks_to_read = 0;
	errcode_t err = EXT2_ET_INVALID_ARGUMENT;

	if (ctx->readahead_kb == 0)
		goto out;

	/* Keep iterating groups until we have enough to readahead */
	inodes_per_block = EXT2_INODES_PER_BLOCK(ctx->fs->super);
	for (grp = start; grp < ctx->fs->group_desc_count; grp++) {
		if (ext2fs_bg_flags_test(ctx->fs, grp, EXT2_BG_INODE_UNINIT))
			continue;
		inodes_in_group = ctx->fs->super->s_inodes_per_group -
					ext2fs_bg_itable_unused(ctx->fs, grp);
		blocks_to_read += (inodes_in_group + inodes_per_block - 1) /
					inodes_per_block;
		if (blocks_to_read * ctx->fs->blocksize >
		    ctx->readahead_kb * 1024)
			break;
	}

	err = e2fsck_readahead(ctx->fs, E2FSCK_READA_ITABLE, start,
			       grp - start + 1);
	if (err == EAGAIN) {
		ctx->readahead_kb /= 2;
		err = 0;
	}

out:
	if (err) {
		/* Error; disable itable readahead */
		*group = ctx->fs->group_desc_count;
		*next_ino = ctx->fs->super->s_inodes_count;
	} else {
		/*
		 * Don't do more readahead until we've reached the first inode
		 * of the last inode scan buffer block for the last group.
		 */
		*group = grp + 1;
		inodes_per_buffer = (ctx->inode_buffer_blocks ?
				     ctx->inode_buffer_blocks :
				     EXT2_INODE_SCAN_DEFAULT_BUFFER_BLOCKS) *
				    ctx->fs->blocksize /
				    EXT2_INODE_SIZE(ctx->fs->super);
		inodes_in_group--;
		*next_ino = inodes_in_group -
			    (inodes_in_group % inodes_per_buffer) + 1 +
			    (grp * ctx->fs->super->s_inodes_per_group);
	}
}

/*
 * Check if the passed ino is one of the used superblock quota inodes.
 *
 * Before the quota inodes were journaled, older superblock quota inodes
 * were just regular files in the filesystem and not reserved inodes.  This
 * checks if the passed ino is one of the s_*_quota_inum superblock fields,
 * which may not always be the same as the EXT4_*_QUOTA_INO fields.
 */
static int quota_inum_is_super(struct ext2_super_block *sb, ext2_ino_t ino)
{
	enum quota_type qtype;

	for (qtype = 0; qtype < MAXQUOTAS; qtype++)
		if (*quota_sb_inump(sb, qtype) == ino)
			return 1;

	return 0;
}

/*
 * Check if the passed ino is one of the reserved quota inodes.
 * This checks if the inode number is one of the reserved EXT4_*_QUOTA_INO
 * inodes.  These inodes may or may not be in use by the quota feature.
 */
static int quota_inum_is_reserved(ext2_filsys fs, ext2_ino_t ino)
{
	enum quota_type qtype;

	for (qtype = 0; qtype < MAXQUOTAS; qtype++)
		if (quota_type2inum(qtype, fs->super) == ino)
			return 1;

	return 0;
}

void e2fsck_pass1(e2fsck_t ctx)
{
	int	i;
	__u64	max_sizes;
	ext2_filsys fs = ctx->fs;
	ext2_ino_t	ino = 0;
	struct ext2_inode *inode = NULL;
	ext2_inode_scan	scan = NULL;
	char		*block_buf = NULL;
#ifdef RESOURCE_TRACK
	struct resource_track	rtrack;
#endif
	unsigned char	frag, fsize;
	struct		problem_context pctx;
	struct		scan_callback_struct scan_struct;
	struct ext2_super_block *sb = ctx->fs->super;
	const char	*old_op;
	const char	*eop_next_inode = _("getting next inode from scan");
	int		imagic_fs, extent_fs, inlinedata_fs, casefold_fs;
	int		low_dtime_check = 1;
	unsigned int	inode_size = EXT2_INODE_SIZE(fs->super);
	unsigned int	bufsize;
	int		failed_csum = 0;
	ext2_ino_t	ino_threshold = 0;
	dgrp_t		ra_group = 0;
	struct ea_quota	ea_ibody_quota;

	init_resource_track(&rtrack, ctx->fs->io);
	clear_problem_context(&pctx);

	/* If we can do readahead, figure out how many groups to pull in. */
	if (!e2fsck_can_readahead(ctx->fs))
		ctx->readahead_kb = 0;
	else if (ctx->readahead_kb == ~0ULL)
		ctx->readahead_kb = e2fsck_guess_readahead(ctx->fs);
	pass1_readahead(ctx, &ra_group, &ino_threshold);

	if (!(ctx->options & E2F_OPT_PREEN))
		fix_problem(ctx, PR_1_PASS_HEADER, &pctx);

	if (ext2fs_has_feature_dir_index(fs->super) &&
	    !(ctx->options & E2F_OPT_NO)) {
		if (ext2fs_u32_list_create(&ctx->dirs_to_hash, 50))
			ctx->dirs_to_hash = 0;
	}

#ifdef MTRACE
	mtrace_print("Pass 1");
#endif

#define EXT2_BPP(bits) (1ULL << ((bits) - 2))

	for (i = EXT2_MIN_BLOCK_LOG_SIZE; i <= EXT2_MAX_BLOCK_LOG_SIZE; i++) {
		max_sizes = EXT2_NDIR_BLOCKS + EXT2_BPP(i);
		max_sizes = max_sizes + EXT2_BPP(i) * EXT2_BPP(i);
		max_sizes = max_sizes + EXT2_BPP(i) * EXT2_BPP(i) * EXT2_BPP(i);
		max_sizes = (max_sizes * (1UL << i));
		ext2_max_sizes[i - EXT2_MIN_BLOCK_LOG_SIZE] = max_sizes;
	}
#undef EXT2_BPP

	imagic_fs = ext2fs_has_feature_imagic_inodes(sb);
	extent_fs = ext2fs_has_feature_extents(sb);
	inlinedata_fs = ext2fs_has_feature_inline_data(sb);
	casefold_fs = ext2fs_has_feature_casefold(sb);

	/*
	 * Allocate bitmaps structures
	 */
	pctx.errcode = e2fsck_allocate_inode_bitmap(fs, _("in-use inode map"),
						    EXT2FS_BMAP64_RBTREE,
						    "inode_used_map",
						    &ctx->inode_used_map);
	if (pctx.errcode) {
		pctx.num = 1;
		fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = e2fsck_allocate_inode_bitmap(fs,
			_("directory inode map"),
			EXT2FS_BMAP64_AUTODIR,
			"inode_dir_map", &ctx->inode_dir_map);
	if (pctx.errcode) {
		pctx.num = 2;
		fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = e2fsck_allocate_inode_bitmap(fs,
			_("regular file inode map"), EXT2FS_BMAP64_RBTREE,
			"inode_reg_map", &ctx->inode_reg_map);
	if (pctx.errcode) {
		pctx.num = 6;
		fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = e2fsck_allocate_subcluster_bitmap(fs,
			_("in-use block map"), EXT2FS_BMAP64_RBTREE,
			"block_found_map", &ctx->block_found_map);
	if (pctx.errcode) {
		pctx.num = 1;
		fix_problem(ctx, PR_1_ALLOCATE_BBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = e2fsck_allocate_block_bitmap(fs,
			_("metadata block map"), EXT2FS_BMAP64_RBTREE,
			"block_metadata_map", &ctx->block_metadata_map);
	if (pctx.errcode) {
		pctx.num = 1;
		fix_problem(ctx, PR_1_ALLOCATE_BBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = e2fsck_setup_icount(ctx, "inode_link_info", 0, NULL,
					   &ctx->inode_link_info);
	if (pctx.errcode) {
		fix_problem(ctx, PR_1_ALLOCATE_ICOUNT, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	bufsize = inode_size;
	if (bufsize < sizeof(struct ext2_inode_large))
		bufsize = sizeof(struct ext2_inode_large);
	inode = (struct ext2_inode *)
		e2fsck_allocate_memory(ctx, bufsize, "scratch inode");

	inodes_to_process = (struct process_inode_block *)
		e2fsck_allocate_memory(ctx,
				       (ctx->process_inode_size *
					sizeof(struct process_inode_block)),
				       "array of inodes to process");
	process_inode_count = 0;

	pctx.errcode = ext2fs_init_dblist(fs, 0);
	if (pctx.errcode) {
		fix_problem(ctx, PR_1_ALLOCATE_DBCOUNT, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		goto endit;
	}

	/*
	 * If the last orphan field is set, clear it, since the pass1
	 * processing will automatically find and clear the orphans.
	 * In the future, we may want to try using the last_orphan
	 * linked list ourselves, but for now, we clear it so that the
	 * ext3 mount code won't get confused.
	 */
	if (!(ctx->options & E2F_OPT_READONLY)) {
		if (fs->super->s_last_orphan) {
			fs->super->s_last_orphan = 0;
			ext2fs_mark_super_dirty(fs);
		}
	}

	mark_table_blocks(ctx);
	pctx.errcode = ext2fs_convert_subcluster_bitmap(fs,
						&ctx->block_found_map);
	if (pctx.errcode) {
		fix_problem(ctx, PR_1_CONVERT_SUBCLUSTER, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		goto endit;
	}
	block_buf = (char *) e2fsck_allocate_memory(ctx, fs->blocksize * 3,
						    "block interate buffer");
	if (EXT2_INODE_SIZE(fs->super) == EXT2_GOOD_OLD_INODE_SIZE)
		e2fsck_use_inode_shortcuts(ctx, 1);
	e2fsck_intercept_block_allocations(ctx);
	old_op = ehandler_operation(_("opening inode scan"));
	pctx.errcode = ext2fs_open_inode_scan(fs, ctx->inode_buffer_blocks,
					      &scan);
	ehandler_operation(old_op);
	if (pctx.errcode) {
		fix_problem(ctx, PR_1_ISCAN_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		goto endit;
	}
	ext2fs_inode_scan_flags(scan, EXT2_SF_SKIP_MISSING_ITABLE |
				      EXT2_SF_WARN_GARBAGE_INODES, 0);
	ctx->stashed_inode = inode;
	scan_struct.ctx = ctx;
	scan_struct.block_buf = block_buf;
	ext2fs_set_inode_callback(scan, scan_callback, &scan_struct);
	if (ctx->progress && ((ctx->progress)(ctx, 1, 0,
					      ctx->fs->group_desc_count)))
		goto endit;
	if ((fs->super->s_wtime &&
	     fs->super->s_wtime < fs->super->s_inodes_count) ||
	    (fs->super->s_mtime &&
	     fs->super->s_mtime < fs->super->s_inodes_count) ||
	    (fs->super->s_mkfs_time &&
	     fs->super->s_mkfs_time < fs->super->s_inodes_count))
		low_dtime_check = 0;

	if (ext2fs_has_feature_mmp(fs->super) &&
	    fs->super->s_mmp_block > fs->super->s_first_data_block &&
	    fs->super->s_mmp_block < ext2fs_blocks_count(fs->super))
		ext2fs_mark_block_bitmap2(ctx->block_found_map,
					  fs->super->s_mmp_block);

	/* Set up ctx->lost_and_found if possible */
	(void) e2fsck_get_lost_and_found(ctx, 0);

	while (1) {
		if (ino % (fs->super->s_inodes_per_group * 4) == 1) {
			if (e2fsck_mmp_update(fs))
				fatal_error(ctx, 0);
		}
		old_op = ehandler_operation(eop_next_inode);
		pctx.errcode = ext2fs_get_next_inode_full(scan, &ino,
							  inode, inode_size);
		if (ino > ino_threshold)
			pass1_readahead(ctx, &ra_group, &ino_threshold);
		ehandler_operation(old_op);
		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			goto endit;
		if (pctx.errcode == EXT2_ET_BAD_BLOCK_IN_INODE_TABLE) {
			/*
			 * If badblocks says badblocks is bad, offer to clear
			 * the list, update the in-core bb list, and restart
			 * the inode scan.
			 */
			if (ino == EXT2_BAD_INO &&
			    fix_problem(ctx, PR_1_BADBLOCKS_IN_BADBLOCKS,
					&pctx)) {
				errcode_t err;

				e2fsck_clear_inode(ctx, ino, inode, 0, "pass1");
				ext2fs_badblocks_list_free(ctx->fs->badblocks);
				ctx->fs->badblocks = NULL;
				err = ext2fs_read_bb_inode(ctx->fs,
							&ctx->fs->badblocks);
				if (err) {
					fix_problem(ctx, PR_1_ISCAN_ERROR,
						    &pctx);
					ctx->flags |= E2F_FLAG_ABORT;
					goto endit;
				}
				err = ext2fs_inode_scan_goto_blockgroup(scan,
									0);
				if (err) {
					fix_problem(ctx, PR_1_ISCAN_ERROR,
						    &pctx);
					ctx->flags |= E2F_FLAG_ABORT;
					goto endit;
				}
				continue;
			}
			if (!ctx->inode_bb_map)
				alloc_bb_map(ctx);
			ext2fs_mark_inode_bitmap2(ctx->inode_bb_map, ino);
			ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
			continue;
		}
		if (pctx.errcode &&
		    pctx.errcode != EXT2_ET_INODE_CSUM_INVALID &&
		    pctx.errcode != EXT2_ET_INODE_IS_GARBAGE) {
			fix_problem(ctx, PR_1_ISCAN_ERROR, &pctx);
			ctx->flags |= E2F_FLAG_ABORT;
			goto endit;
		}
		if (!ino)
			break;
		pctx.ino = ino;
		pctx.inode = inode;
		ctx->stashed_ino = ino;

		/* Clear trashed inode? */
		if (pctx.errcode == EXT2_ET_INODE_IS_GARBAGE &&
		    inode->i_links_count > 0 &&
		    fix_problem(ctx, PR_1_INODE_IS_GARBAGE, &pctx)) {
			pctx.errcode = 0;
			e2fsck_clear_inode(ctx, ino, inode, 0, "pass1");
		}
		failed_csum = pctx.errcode != 0;

		/*
		 * Check for inodes who might have been part of the
		 * orphaned list linked list.  They should have gotten
		 * dealt with by now, unless the list had somehow been
		 * corrupted.
		 *
		 * FIXME: In the future, inodes which are still in use
		 * (and which are therefore) pending truncation should
		 * be handled specially.  Right now we just clear the
		 * dtime field, and the normal e2fsck handling of
		 * inodes where i_size and the inode blocks are
		 * inconsistent is to fix i_size, instead of releasing
		 * the extra blocks.  This won't catch the inodes that
		 * was at the end of the orphan list, but it's better
		 * than nothing.  The right answer is that there
		 * shouldn't be any bugs in the orphan list handling.  :-)
		 */
		if (inode->i_dtime && low_dtime_check &&
		    inode->i_dtime < ctx->fs->super->s_inodes_count) {
			if (fix_problem(ctx, PR_1_LOW_DTIME, &pctx)) {
				inode->i_dtime = inode->i_links_count ?
					0 : ctx->now;
				e2fsck_write_inode(ctx, ino, inode,
						   "pass1");
				failed_csum = 0;
			}
		}

		if (inode->i_links_count) {
			pctx.errcode = ext2fs_icount_store(ctx->inode_link_info,
					   ino, inode->i_links_count);
			if (pctx.errcode) {
				pctx.num = inode->i_links_count;
				fix_problem(ctx, PR_1_ICOUNT_STORE, &pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
		} else if ((ino >= EXT2_FIRST_INODE(fs->super)) &&
			   !quota_inum_is_reserved(fs, ino)) {
			if (!inode->i_dtime && inode->i_mode) {
				if (fix_problem(ctx,
					    PR_1_ZERO_DTIME, &pctx)) {
					inode->i_dtime = ctx->now;
					e2fsck_write_inode(ctx, ino, inode,
							   "pass1");
					failed_csum = 0;
				}
			}
			FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
			continue;
		}

		if ((inode->i_flags & EXT4_CASEFOLD_FL) &&
		    ((!LINUX_S_ISDIR(inode->i_mode) &&
		      fix_problem(ctx, PR_1_CASEFOLD_NONDIR, &pctx)) ||
		     (!casefold_fs &&
		      fix_problem(ctx, PR_1_CASEFOLD_FEATURE, &pctx)))) {
			inode->i_flags &= ~EXT4_CASEFOLD_FL;
			e2fsck_write_inode(ctx, ino, inode, "pass1");
		}

		/* Conflicting inlinedata/extents inode flags? */
		if ((inode->i_flags & EXT4_INLINE_DATA_FL) &&
		    (inode->i_flags & EXT4_EXTENTS_FL)) {
			int res = fix_inline_data_extents_file(ctx, ino, inode,
							       inode_size,
							       &pctx);
			if (res < 0) {
				/* skip FINISH_INODE_LOOP */
				continue;
			}
		}

		/* Test for incorrect inline_data flags settings. */
		if ((inode->i_flags & EXT4_INLINE_DATA_FL) && !inlinedata_fs &&
		    (ino >= EXT2_FIRST_INODE(fs->super))) {
			size_t size = 0;

			pctx.errcode = get_inline_data_ea_size(fs, ino, &size);
			if (!pctx.errcode &&
			    fix_problem(ctx, PR_1_INLINE_DATA_FEATURE, &pctx)) {
				ext2fs_set_feature_inline_data(sb);
				ext2fs_mark_super_dirty(fs);
				inlinedata_fs = 1;
			} else if (fix_problem(ctx, PR_1_INLINE_DATA_SET, &pctx)) {
				e2fsck_clear_inode(ctx, ino, inode, 0, "pass1");
				/* skip FINISH_INODE_LOOP */
				continue;
			}
		}

		/* Test for inline data flag but no attr */
		if ((inode->i_flags & EXT4_INLINE_DATA_FL) && inlinedata_fs &&
		    (ino >= EXT2_FIRST_INODE(fs->super))) {
			size_t size = 0;
			errcode_t err;
			int flags;

			flags = fs->flags;
			if (failed_csum)
				fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
			err = get_inline_data_ea_size(fs, ino, &size);
			fs->flags = (flags & EXT2_FLAG_IGNORE_CSUM_ERRORS) |
				    (fs->flags & ~EXT2_FLAG_IGNORE_CSUM_ERRORS);

			switch (err) {
			case 0:
				/* Everything is awesome... */
				break;
			case EXT2_ET_BAD_EA_BLOCK_NUM:
			case EXT2_ET_BAD_EA_HASH:
			case EXT2_ET_BAD_EA_HEADER:
			case EXT2_ET_EA_BAD_NAME_LEN:
			case EXT2_ET_EA_BAD_VALUE_SIZE:
			case EXT2_ET_EA_KEY_NOT_FOUND:
			case EXT2_ET_EA_NO_SPACE:
			case EXT2_ET_MISSING_EA_FEATURE:
			case EXT2_ET_INLINE_DATA_CANT_ITERATE:
			case EXT2_ET_INLINE_DATA_NO_BLOCK:
			case EXT2_ET_INLINE_DATA_NO_SPACE:
			case EXT2_ET_NO_INLINE_DATA:
			case EXT2_ET_EXT_ATTR_CSUM_INVALID:
			case EXT2_ET_EA_BAD_VALUE_OFFSET:
			case EXT2_ET_EA_INODE_CORRUPTED:
				/* broken EA or no system.data EA; truncate */
				if (fix_problem(ctx, PR_1_INLINE_DATA_NO_ATTR,
						&pctx)) {
					err = ext2fs_inode_size_set(fs, inode, 0);
					if (err) {
						pctx.errcode = err;
						ctx->flags |= E2F_FLAG_ABORT;
						goto endit;
					}
					inode->i_flags &= ~EXT4_INLINE_DATA_FL;
					memset(&inode->i_block, 0,
					       sizeof(inode->i_block));
					e2fsck_write_inode(ctx, ino, inode,
							   "pass1");
					failed_csum = 0;
				}
				break;
			default:
				/* Some other kind of non-xattr error? */
				pctx.errcode = err;
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
		}

		/*
		 * Test for incorrect extent flag settings.
		 *
		 * On big-endian machines we must be careful:
		 * When the inode is read, the i_block array is not swapped
		 * if the extent flag is set.  Therefore if we are testing
		 * for or fixing a wrongly-set flag, we must potentially
		 * (un)swap before testing, or after fixing.
		 */

		/*
		 * In this case the extents flag was set when read, so
		 * extent_header_verify is ok.  If the inode is cleared,
		 * no need to swap... so no extra swapping here.
		 */
		if ((inode->i_flags & EXT4_EXTENTS_FL) && !extent_fs &&
		    (inode->i_links_count || (ino == EXT2_BAD_INO) ||
		     (ino == EXT2_ROOT_INO) || (ino == EXT2_JOURNAL_INO))) {
			if ((ext2fs_extent_header_verify(inode->i_block,
						 sizeof(inode->i_block)) == 0) &&
			    fix_problem(ctx, PR_1_EXTENT_FEATURE, &pctx)) {
				ext2fs_set_feature_extents(sb);
				ext2fs_mark_super_dirty(fs);
				extent_fs = 1;
			} else if (fix_problem(ctx, PR_1_EXTENTS_SET, &pctx)) {
			clear_inode:
				e2fsck_clear_inode(ctx, ino, inode, 0, "pass1");
				if (ino == EXT2_BAD_INO)
					ext2fs_mark_inode_bitmap2(ctx->inode_used_map,
								 ino);
				/* skip FINISH_INODE_LOOP */
				continue;
			}
		}

		/*
		 * For big-endian machines:
		 * If the inode didn't have the extents flag set when it
		 * was read, then the i_blocks array was swapped.  To test
		 * as an extents header, we must swap it back first.
		 * IF we then set the extents flag, the entire i_block
		 * array must be un/re-swapped to make it proper extents data.
		 */
		if (extent_fs && !(inode->i_flags & EXT4_EXTENTS_FL) &&
		    (inode->i_links_count || (ino == EXT2_BAD_INO) ||
		     (ino == EXT2_ROOT_INO) || (ino == EXT2_JOURNAL_INO)) &&
		    (LINUX_S_ISREG(inode->i_mode) ||
		     LINUX_S_ISDIR(inode->i_mode))) {
			void *ehp;
#ifdef WORDS_BIGENDIAN
			__u32 tmp_block[EXT2_N_BLOCKS];

			for (i = 0; i < EXT2_N_BLOCKS; i++)
				tmp_block[i] = ext2fs_swab32(inode->i_block[i]);
			ehp = tmp_block;
#else
			ehp = inode->i_block;
#endif
			if ((ext2fs_extent_header_verify(ehp,
					 sizeof(inode->i_block)) == 0) &&
			    (fix_problem(ctx, PR_1_UNSET_EXTENT_FL, &pctx))) {
				inode->i_flags |= EXT4_EXTENTS_FL;
#ifdef WORDS_BIGENDIAN
				memcpy(inode->i_block, tmp_block,
				       sizeof(inode->i_block));
#endif
				e2fsck_write_inode(ctx, ino, inode, "pass1");
				failed_csum = 0;
			}
		}

		if (ino == EXT2_BAD_INO) {
			struct process_block_struct pb;

			if ((failed_csum || inode->i_mode || inode->i_uid ||
			     inode->i_gid || inode->i_links_count ||
			     (inode->i_flags & EXT4_INLINE_DATA_FL) ||
			     inode->i_file_acl) &&
			    fix_problem(ctx, PR_1_INVALID_BAD_INODE, &pctx)) {
				memset(inode, 0, sizeof(struct ext2_inode));
				e2fsck_write_inode(ctx, ino, inode,
						   "clear bad inode");
				failed_csum = 0;
			}

			pctx.errcode = ext2fs_copy_bitmap(ctx->block_found_map,
							  &pb.fs_meta_blocks);
			if (pctx.errcode) {
				pctx.num = 4;
				fix_problem(ctx, PR_1_ALLOCATE_BBITMAP_ERROR, &pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
			pb.ino = EXT2_BAD_INO;
			pb.num_blocks = pb.last_block = 0;
			pb.last_db_block = -1;
			pb.num_illegal_blocks = 0;
			pb.suppress = 0; pb.clear = 0; pb.is_dir = 0;
			pb.is_reg = 0; pb.fragmented = 0; pb.bbcheck = 0;
			pb.inode = inode;
			pb.pctx = &pctx;
			pb.ctx = ctx;
			pctx.errcode = ext2fs_block_iterate3(fs, ino, 0,
				     block_buf, process_bad_block, &pb);
			ext2fs_free_block_bitmap(pb.fs_meta_blocks);
			if (pctx.errcode) {
				fix_problem(ctx, PR_1_BLOCK_ITERATE, &pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
			if (pb.bbcheck)
				if (!fix_problem(ctx, PR_1_BBINODE_BAD_METABLOCK_PROMPT, &pctx)) {
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
			ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
			clear_problem_context(&pctx);
			FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
			continue;
		} else if (ino == EXT2_ROOT_INO) {
			/*
			 * Make sure the root inode is a directory; if
			 * not, offer to clear it.  It will be
			 * regenerated in pass #3.
			 */
			if (!LINUX_S_ISDIR(inode->i_mode)) {
				if (fix_problem(ctx, PR_1_ROOT_NO_DIR, &pctx))
					goto clear_inode;
			}
			/*
			 * If dtime is set, offer to clear it.  mke2fs
			 * version 0.2b created filesystems with the
			 * dtime field set for the root and lost+found
			 * directories.  We won't worry about
			 * /lost+found, since that can be regenerated
			 * easily.  But we will fix the root directory
			 * as a special case.
			 */
			if (inode->i_dtime && inode->i_links_count) {
				if (fix_problem(ctx, PR_1_ROOT_DTIME, &pctx)) {
					inode->i_dtime = 0;
					e2fsck_write_inode(ctx, ino, inode,
							   "pass1");
					failed_csum = 0;
				}
			}
		} else if (ino == EXT2_JOURNAL_INO) {
			ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
			if (fs->super->s_journal_inum == EXT2_JOURNAL_INO) {
				if (!LINUX_S_ISREG(inode->i_mode) &&
				    fix_problem(ctx, PR_1_JOURNAL_BAD_MODE,
						&pctx)) {
					inode->i_mode = LINUX_S_IFREG;
					e2fsck_write_inode(ctx, ino, inode,
							   "pass1");
					failed_csum = 0;
				}
				check_blocks(ctx, &pctx, block_buf, NULL);
				FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
				continue;
			}
			if ((inode->i_links_count ||
			     inode->i_blocks || inode->i_block[0]) &&
			    fix_problem(ctx, PR_1_JOURNAL_INODE_NOT_CLEAR,
					&pctx)) {
				memset(inode, 0, inode_size);
				ext2fs_icount_store(ctx->inode_link_info,
						    ino, 0);
				e2fsck_write_inode_full(ctx, ino, inode,
							inode_size, "pass1");
				failed_csum = 0;
			}
		} else if (quota_inum_is_reserved(fs, ino)) {
			ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
			if (ext2fs_has_feature_quota(fs->super) &&
			    quota_inum_is_super(fs->super, ino)) {
				if (!LINUX_S_ISREG(inode->i_mode) &&
				    fix_problem(ctx, PR_1_QUOTA_BAD_MODE,
							&pctx)) {
					inode->i_mode = LINUX_S_IFREG;
					e2fsck_write_inode(ctx, ino, inode,
							"pass1");
					failed_csum = 0;
				}
				check_blocks(ctx, &pctx, block_buf, NULL);
				FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
				continue;
			}
			if ((inode->i_links_count ||
			     inode->i_blocks || inode->i_block[0]) &&
			    fix_problem(ctx, PR_1_QUOTA_INODE_NOT_CLEAR,
					&pctx)) {
				memset(inode, 0, inode_size);
				ext2fs_icount_store(ctx->inode_link_info,
						    ino, 0);
				e2fsck_write_inode_full(ctx, ino, inode,
							inode_size, "pass1");
				failed_csum = 0;
			}
		} else if (ino < EXT2_FIRST_INODE(fs->super)) {
			problem_t problem = 0;

			ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
			if (ino == EXT2_BOOT_LOADER_INO) {
				if (LINUX_S_ISDIR(inode->i_mode))
					problem = PR_1_RESERVED_BAD_MODE;
			} else if (ino == EXT2_RESIZE_INO) {
				if (inode->i_mode &&
				    !LINUX_S_ISREG(inode->i_mode))
					problem = PR_1_RESERVED_BAD_MODE;
			} else {
				if (inode->i_mode != 0)
					problem = PR_1_RESERVED_BAD_MODE;
			}
			if (problem) {
				if (fix_problem(ctx, problem, &pctx)) {
					inode->i_mode = 0;
					e2fsck_write_inode(ctx, ino, inode,
							   "pass1");
					failed_csum = 0;
				}
			}
			check_blocks(ctx, &pctx, block_buf, NULL);
			FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
			continue;
		}

		if (!inode->i_links_count) {
			FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
			continue;
		}
		/*
		 * n.b.  0.3c ext2fs code didn't clear i_links_count for
		 * deleted files.  Oops.
		 *
		 * Since all new ext2 implementations get this right,
		 * we now assume that the case of non-zero
		 * i_links_count and non-zero dtime means that we
		 * should keep the file, not delete it.
		 *
		 */
		if (inode->i_dtime) {
			if (fix_problem(ctx, PR_1_SET_DTIME, &pctx)) {
				inode->i_dtime = 0;
				e2fsck_write_inode(ctx, ino, inode, "pass1");
				failed_csum = 0;
			}
		}

		ext2fs_mark_inode_bitmap2(ctx->inode_used_map, ino);
		switch (fs->super->s_creator_os) {
		    case EXT2_OS_HURD:
			frag = inode->osd2.hurd2.h_i_frag;
			fsize = inode->osd2.hurd2.h_i_fsize;
			break;
		    default:
			frag = fsize = 0;
		}

		if (inode->i_faddr || frag || fsize ||
		    (!ext2fs_has_feature_largedir(fs->super) &&
		    (LINUX_S_ISDIR(inode->i_mode) && inode->i_size_high)))
			mark_inode_bad(ctx, ino);
		if ((fs->super->s_creator_os != EXT2_OS_HURD) &&
		    !ext2fs_has_feature_64bit(fs->super) &&
		    inode->osd2.linux2.l_i_file_acl_high != 0)
			mark_inode_bad(ctx, ino);
		if ((fs->super->s_creator_os != EXT2_OS_HURD) &&
		    !ext2fs_has_feature_huge_file(fs->super) &&
		    (inode->osd2.linux2.l_i_blocks_hi != 0))
			mark_inode_bad(ctx, ino);
		if (inode->i_flags & EXT2_IMAGIC_FL) {
			if (imagic_fs) {
				if (!ctx->inode_imagic_map)
					alloc_imagic_map(ctx);
				ext2fs_mark_inode_bitmap2(ctx->inode_imagic_map,
							 ino);
			} else {
				if (fix_problem(ctx, PR_1_SET_IMAGIC, &pctx)) {
					inode->i_flags &= ~EXT2_IMAGIC_FL;
					e2fsck_write_inode(ctx, ino,
							   inode, "pass1");
					failed_csum = 0;
				}
			}
		}

		check_inode_extra_space(ctx, &pctx, &ea_ibody_quota);
		check_is_really_dir(ctx, &pctx, block_buf);

		/*
		 * ext2fs_inode_has_valid_blocks2 does not actually look
		 * at i_block[] values, so not endian-sensitive here.
		 */
		if (extent_fs && (inode->i_flags & EXT4_EXTENTS_FL) &&
		    LINUX_S_ISLNK(inode->i_mode) &&
		    !ext2fs_inode_has_valid_blocks2(fs, inode) &&
		    fix_problem(ctx, PR_1_FAST_SYMLINK_EXTENT_FL, &pctx)) {
			inode->i_flags &= ~EXT4_EXTENTS_FL;
			e2fsck_write_inode(ctx, ino, inode, "pass1");
			failed_csum = 0;
		}

		if (LINUX_S_ISDIR(inode->i_mode)) {
			ext2fs_mark_inode_bitmap2(ctx->inode_dir_map, ino);
			e2fsck_add_dir_info(ctx, ino, 0);
			ctx->fs_directory_count++;
			if (inode->i_flags & EXT4_ENCRYPT_FL)
				add_encrypted_dir(ctx, ino);
		} else if (LINUX_S_ISREG (inode->i_mode)) {
			ext2fs_mark_inode_bitmap2(ctx->inode_reg_map, ino);
			ctx->fs_regular_count++;
		} else if (LINUX_S_ISCHR (inode->i_mode) &&
			   e2fsck_pass1_check_device_inode(fs, inode)) {
			check_extents_inlinedata(ctx, &pctx);
			check_immutable(ctx, &pctx);
			check_size(ctx, &pctx);
			ctx->fs_chardev_count++;
		} else if (LINUX_S_ISBLK (inode->i_mode) &&
			   e2fsck_pass1_check_device_inode(fs, inode)) {
			check_extents_inlinedata(ctx, &pctx);
			check_immutable(ctx, &pctx);
			check_size(ctx, &pctx);
			ctx->fs_blockdev_count++;
		} else if (LINUX_S_ISLNK (inode->i_mode) &&
			   e2fsck_pass1_check_symlink(fs, ino, inode,
						      block_buf)) {
			check_immutable(ctx, &pctx);
			ctx->fs_symlinks_count++;
			if (inode->i_flags & EXT4_INLINE_DATA_FL) {
				FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
				continue;
			} else if (ext2fs_is_fast_symlink(inode)) {
				ctx->fs_fast_symlinks_count++;
				check_blocks(ctx, &pctx, block_buf,
					     &ea_ibody_quota);
				FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);
				continue;
			}
		}
		else if (LINUX_S_ISFIFO (inode->i_mode) &&
			 e2fsck_pass1_check_device_inode(fs, inode)) {
			check_extents_inlinedata(ctx, &pctx);
			check_immutable(ctx, &pctx);
			check_size(ctx, &pctx);
			ctx->fs_fifo_count++;
		} else if ((LINUX_S_ISSOCK (inode->i_mode)) &&
			   e2fsck_pass1_check_device_inode(fs, inode)) {
			check_extents_inlinedata(ctx, &pctx);
			check_immutable(ctx, &pctx);
			check_size(ctx, &pctx);
			ctx->fs_sockets_count++;
		} else
			mark_inode_bad(ctx, ino);
		if (!(inode->i_flags & EXT4_EXTENTS_FL) &&
		    !(inode->i_flags & EXT4_INLINE_DATA_FL)) {
			if (inode->i_block[EXT2_IND_BLOCK])
				ctx->fs_ind_count++;
			if (inode->i_block[EXT2_DIND_BLOCK])
				ctx->fs_dind_count++;
			if (inode->i_block[EXT2_TIND_BLOCK])
				ctx->fs_tind_count++;
		}
		if (!(inode->i_flags & EXT4_EXTENTS_FL) &&
		    !(inode->i_flags & EXT4_INLINE_DATA_FL) &&
		    (inode->i_block[EXT2_IND_BLOCK] ||
		     inode->i_block[EXT2_DIND_BLOCK] ||
		     inode->i_block[EXT2_TIND_BLOCK] ||
		     ext2fs_file_acl_block(fs, inode))) {
			struct process_inode_block *itp;

			itp = &inodes_to_process[process_inode_count];
			itp->ino = ino;
			itp->ea_ibody_quota = ea_ibody_quota;
			if (inode_size < sizeof(struct ext2_inode_large))
				memcpy(&itp->inode, inode, inode_size);
			else
				memcpy(&itp->inode, inode, sizeof(itp->inode));
			process_inode_count++;
		} else
			check_blocks(ctx, &pctx, block_buf, &ea_ibody_quota);

		FINISH_INODE_LOOP(ctx, ino, &pctx, failed_csum);

		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			goto endit;

		if (process_inode_count >= ctx->process_inode_size) {
			process_inodes(ctx, block_buf);

			if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
				goto endit;
		}
	}
	process_inodes(ctx, block_buf);
	ext2fs_close_inode_scan(scan);
	scan = NULL;

	reserve_block_for_root_repair(ctx);
	reserve_block_for_lnf_repair(ctx);

	/*
	 * If any extended attribute blocks' reference counts need to
	 * be adjusted, either up (ctx->refcount_extra), or down
	 * (ctx->refcount), then fix them.
	 */
	if (ctx->refcount) {
		adjust_extattr_refcount(ctx, ctx->refcount, block_buf, -1);
		ea_refcount_free(ctx->refcount);
		ctx->refcount = 0;
	}
	if (ctx->refcount_extra) {
		adjust_extattr_refcount(ctx, ctx->refcount_extra,
					block_buf, +1);
		ea_refcount_free(ctx->refcount_extra);
		ctx->refcount_extra = 0;
	}

	if (ctx->ea_block_quota_blocks) {
		ea_refcount_free(ctx->ea_block_quota_blocks);
		ctx->ea_block_quota_blocks = 0;
	}

	if (ctx->ea_block_quota_inodes) {
		ea_refcount_free(ctx->ea_block_quota_inodes);
		ctx->ea_block_quota_inodes = 0;
	}

	if (ctx->invalid_bitmaps)
		handle_fs_bad_blocks(ctx);

	/* We don't need the block_ea_map any more */
	if (ctx->block_ea_map) {
		ext2fs_free_block_bitmap(ctx->block_ea_map);
		ctx->block_ea_map = 0;
	}

	if (ctx->flags & E2F_FLAG_RESIZE_INODE) {
		clear_problem_context(&pctx);
		pctx.errcode = ext2fs_create_resize_inode(fs);
		if (pctx.errcode) {
			if (!fix_problem(ctx, PR_1_RESIZE_INODE_CREATE,
					 &pctx)) {
				ctx->flags |= E2F_FLAG_ABORT;
				goto endit;
			}
			pctx.errcode = 0;
		}
		if (!pctx.errcode) {
			e2fsck_read_inode(ctx, EXT2_RESIZE_INO, inode,
					  "recreate inode");
			inode->i_mtime = ctx->now;
			e2fsck_write_inode(ctx, EXT2_RESIZE_INO, inode,
					   "recreate inode");
		}
		ctx->flags &= ~E2F_FLAG_RESIZE_INODE;
	}

	if (ctx->flags & E2F_FLAG_RESTART) {
		/*
		 * Only the master copy of the superblock and block
		 * group descriptors are going to be written during a
		 * restart, so set the superblock to be used to be the
		 * master superblock.
		 */
		ctx->use_superblock = 0;
		unwind_pass1(fs);
		goto endit;
	}

	if (ctx->block_dup_map) {
		if (ctx->options & E2F_OPT_PREEN) {
			clear_problem_context(&pctx);
			fix_problem(ctx, PR_1_DUP_BLOCKS_PREENSTOP, &pctx);
		}
		e2fsck_pass1_dupblocks(ctx, block_buf);
	}
	ctx->flags |= E2F_FLAG_ALLOC_OK;
	ext2fs_free_mem(&inodes_to_process);
endit:
	e2fsck_use_inode_shortcuts(ctx, 0);

	if (scan)
		ext2fs_close_inode_scan(scan);
	if (block_buf)
		ext2fs_free_mem(&block_buf);
	if (inode)
		ext2fs_free_mem(&inode);

	/*
	 * The l+f inode may have been cleared, so zap it now and
	 * later passes will recalculate it if necessary
	 */
	ctx->lost_and_found = 0;

	if ((ctx->flags & E2F_FLAG_SIGNAL_MASK) == 0)
		print_resource_track(ctx, _("Pass 1"), &rtrack, ctx->fs->io);
	else
		ctx->invalid_bitmaps++;
}
#undef FINISH_INODE_LOOP

/*
 * When the inode_scan routines call this callback at the end of the
 * glock group, call process_inodes.
 */
static errcode_t scan_callback(ext2_filsys fs,
			       ext2_inode_scan scan EXT2FS_ATTR((unused)),
			       dgrp_t group, void * priv_data)
{
	struct scan_callback_struct *scan_struct;
	e2fsck_t ctx;

	scan_struct = (struct scan_callback_struct *) priv_data;
	ctx = scan_struct->ctx;

	process_inodes((e2fsck_t) fs->priv_data, scan_struct->block_buf);

	if (ctx->progress)
		if ((ctx->progress)(ctx, 1, group+1,
				    ctx->fs->group_desc_count))
			return EXT2_ET_CANCEL_REQUESTED;

	return 0;
}

/*
 * Process the inodes in the "inodes to process" list.
 */
static void process_inodes(e2fsck_t ctx, char *block_buf)
{
	int			i;
	struct ext2_inode	*old_stashed_inode;
	ext2_ino_t		old_stashed_ino;
	const char		*old_operation;
	char			buf[80];
	struct problem_context	pctx;

#if 0
	printf("begin process_inodes: ");
#endif
	if (process_inode_count == 0)
		return;
	old_operation = ehandler_operation(0);
	old_stashed_inode = ctx->stashed_inode;
	old_stashed_ino = ctx->stashed_ino;
	qsort(inodes_to_process, process_inode_count,
		      sizeof(struct process_inode_block), process_inode_cmp);
	clear_problem_context(&pctx);
	for (i=0; i < process_inode_count; i++) {
		pctx.inode = ctx->stashed_inode =
			(struct ext2_inode *) &inodes_to_process[i].inode;
		pctx.ino = ctx->stashed_ino = inodes_to_process[i].ino;

#if 0
		printf("%u ", pctx.ino);
#endif
		sprintf(buf, _("reading indirect blocks of inode %u"),
			pctx.ino);
		ehandler_operation(buf);
		check_blocks(ctx, &pctx, block_buf,
			     &inodes_to_process[i].ea_ibody_quota);
		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			break;
	}
	ctx->stashed_inode = old_stashed_inode;
	ctx->stashed_ino = old_stashed_ino;
	process_inode_count = 0;
#if 0
	printf("end process inodes\n");
#endif
	ehandler_operation(old_operation);
}

static EXT2_QSORT_TYPE process_inode_cmp(const void *a, const void *b)
{
	const struct process_inode_block *ib_a =
		(const struct process_inode_block *) a;
	const struct process_inode_block *ib_b =
		(const struct process_inode_block *) b;
	int	ret;

	ret = (ib_a->inode.i_block[EXT2_IND_BLOCK] -
	       ib_b->inode.i_block[EXT2_IND_BLOCK]);
	if (ret == 0)
		/*
		 * We only call process_inodes() for non-extent
		 * inodes, so it's OK to pass NULL to
		 * ext2fs_file_acl_block() here.
		 */
		ret = ext2fs_file_acl_block(0, ext2fs_const_inode(&ib_a->inode)) -
			ext2fs_file_acl_block(0, ext2fs_const_inode(&ib_b->inode));
	if (ret == 0)
		ret = ib_a->ino - ib_b->ino;
	return ret;
}

/*
 * Mark an inode as being bad in some what
 */
static void mark_inode_bad(e2fsck_t ctx, ino_t ino)
{
	struct		problem_context pctx;

	if (!ctx->inode_bad_map) {
		clear_problem_context(&pctx);

		pctx.errcode = e2fsck_allocate_inode_bitmap(ctx->fs,
				_("bad inode map"), EXT2FS_BMAP64_RBTREE,
				"inode_bad_map", &ctx->inode_bad_map);
		if (pctx.errcode) {
			pctx.num = 3;
			fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
			/* Should never get here */
			ctx->flags |= E2F_FLAG_ABORT;
			return;
		}
	}
	ext2fs_mark_inode_bitmap2(ctx->inode_bad_map, ino);
}

static void add_encrypted_dir(e2fsck_t ctx, ino_t ino)
{
	struct		problem_context pctx;

	if (!ctx->encrypted_dirs) {
		pctx.errcode = ext2fs_u32_list_create(&ctx->encrypted_dirs, 0);
		if (pctx.errcode)
			goto error;
	}
	pctx.errcode = ext2fs_u32_list_add(ctx->encrypted_dirs, ino);
	if (pctx.errcode == 0)
		return;
error:
	fix_problem(ctx, PR_1_ALLOCATE_ENCRYPTED_DIRLIST, &pctx);
	/* Should never get here */
	ctx->flags |= E2F_FLAG_ABORT;
}

/*
 * This procedure will allocate the inode "bb" (badblock) map table
 */
static void alloc_bb_map(e2fsck_t ctx)
{
	struct		problem_context pctx;

	clear_problem_context(&pctx);
	pctx.errcode = e2fsck_allocate_inode_bitmap(ctx->fs,
			_("inode in bad block map"), EXT2FS_BMAP64_RBTREE,
			"inode_bb_map", &ctx->inode_bb_map);
	if (pctx.errcode) {
		pctx.num = 4;
		fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
		/* Should never get here */
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
}

/*
 * This procedure will allocate the inode imagic table
 */
static void alloc_imagic_map(e2fsck_t ctx)
{
	struct		problem_context pctx;

	clear_problem_context(&pctx);
	pctx.errcode = e2fsck_allocate_inode_bitmap(ctx->fs,
			_("imagic inode map"), EXT2FS_BMAP64_RBTREE,
			"inode_imagic_map", &ctx->inode_imagic_map);
	if (pctx.errcode) {
		pctx.num = 5;
		fix_problem(ctx, PR_1_ALLOCATE_IBITMAP_ERROR, &pctx);
		/* Should never get here */
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
}

/*
 * Marks a block as in use, setting the dup_map if it's been set
 * already.  Called by process_block and process_bad_block.
 *
 * WARNING: Assumes checks have already been done to make sure block
 * is valid.  This is true in both process_block and process_bad_block.
 */
static _INLINE_ void mark_block_used(e2fsck_t ctx, blk64_t block)
{
	struct		problem_context pctx;

	clear_problem_context(&pctx);

	if (ext2fs_fast_test_block_bitmap2(ctx->block_found_map, block)) {
		if (ext2fs_has_feature_shared_blocks(ctx->fs->super) &&
		    !(ctx->options & E2F_OPT_UNSHARE_BLOCKS)) {
			return;
		}
		if (!ctx->block_dup_map) {
			pctx.errcode = e2fsck_allocate_block_bitmap(ctx->fs,
					_("multiply claimed block map"),
					EXT2FS_BMAP64_RBTREE, "block_dup_map",
					&ctx->block_dup_map);
			if (pctx.errcode) {
				pctx.num = 3;
				fix_problem(ctx, PR_1_ALLOCATE_BBITMAP_ERROR,
					    &pctx);
				/* Should never get here */
				ctx->flags |= E2F_FLAG_ABORT;
				return;
			}
		}
		ext2fs_fast_mark_block_bitmap2(ctx->block_dup_map, block);
	} else {
		ext2fs_fast_mark_block_bitmap2(ctx->block_found_map, block);
	}
}

/*
 * When cluster size is greater than one block, it is caller's responsibility
 * to make sure block parameter starts at a cluster boundary.
 */
static _INLINE_ void mark_blocks_used(e2fsck_t ctx, blk64_t block,
				      unsigned int num)
{
	if (ext2fs_test_block_bitmap_range2(ctx->block_found_map, block, num))
		ext2fs_mark_block_bitmap_range2(ctx->block_found_map, block, num);
	else {
		unsigned int i;

		for (i = 0; i < num; i += EXT2FS_CLUSTER_RATIO(ctx->fs))
			mark_block_used(ctx, block + i);
	}
}

/*
 * Adjust the extended attribute block's reference counts at the end
 * of pass 1, either by subtracting out references for EA blocks that
 * are still referenced in ctx->refcount, or by adding references for
 * EA blocks that had extra references as accounted for in
 * ctx->refcount_extra.
 */
static void adjust_extattr_refcount(e2fsck_t ctx, ext2_refcount_t refcount,
				    char *block_buf, int adjust_sign)
{
	struct ext2_ext_attr_header 	*header;
	struct problem_context		pctx;
	ext2_filsys			fs = ctx->fs;
	blk64_t				blk;
	__u32				should_be;
	ea_value_t			count;

	clear_problem_context(&pctx);

	ea_refcount_intr_begin(refcount);
	while (1) {
		if ((blk = ea_refcount_intr_next(refcount, &count)) == 0)
			break;
		pctx.blk = blk;
		pctx.errcode = ext2fs_read_ext_attr3(fs, blk, block_buf,
						     pctx.ino);
		if (pctx.errcode) {
			fix_problem(ctx, PR_1_EXTATTR_READ_ABORT, &pctx);
			return;
		}
		header = (struct ext2_ext_attr_header *) block_buf;
		pctx.blkcount = header->h_refcount;
		should_be = header->h_refcount + adjust_sign * (int)count;
		pctx.num = should_be;
		if (fix_problem(ctx, PR_1_EXTATTR_REFCOUNT, &pctx)) {
			header->h_refcount = should_be;
			pctx.errcode = ext2fs_write_ext_attr3(fs, blk,
							     block_buf,
							     pctx.ino);
			if (pctx.errcode) {
				fix_problem(ctx, PR_1_EXTATTR_WRITE_ABORT,
					    &pctx);
				continue;
			}
		}
	}
}

/*
 * Handle processing the extended attribute blocks
 */
static int check_ext_attr(e2fsck_t ctx, struct problem_context *pctx,
			   char *block_buf, struct ea_quota *ea_block_quota)
{
	ext2_filsys fs = ctx->fs;
	ext2_ino_t	ino = pctx->ino;
	struct ext2_inode *inode = pctx->inode;
	blk64_t		blk;
	char *		end;
	struct ext2_ext_attr_header *header;
	struct ext2_ext_attr_entry *first, *entry;
	blk64_t		quota_blocks = EXT2FS_C2B(fs, 1);
	__u64		quota_inodes = 0;
	region_t	region = 0;
	int		failed_csum = 0;

	ea_block_quota->blocks = 0;
	ea_block_quota->inodes = 0;

	blk = ext2fs_file_acl_block(fs, inode);
	if (blk == 0)
		return 0;

	/*
	 * If the Extended attribute flag isn't set, then a non-zero
	 * file acl means that the inode is corrupted.
	 *
	 * Or if the extended attribute block is an invalid block,
	 * then the inode is also corrupted.
	 */
	if (!ext2fs_has_feature_xattr(fs->super) ||
	    (blk < fs->super->s_first_data_block) ||
	    (blk >= ext2fs_blocks_count(fs->super))) {
		mark_inode_bad(ctx, ino);
		return 0;
	}

	/* If ea bitmap hasn't been allocated, create it */
	if (!ctx->block_ea_map) {
		pctx->errcode = e2fsck_allocate_block_bitmap(fs,
					_("ext attr block map"),
					EXT2FS_BMAP64_RBTREE, "block_ea_map",
					&ctx->block_ea_map);
		if (pctx->errcode) {
			pctx->num = 2;
			fix_problem(ctx, PR_1_ALLOCATE_BBITMAP_ERROR, pctx);
			ctx->flags |= E2F_FLAG_ABORT;
			return 0;
		}
	}

	/* Create the EA refcount structure if necessary */
	if (!ctx->refcount) {
		pctx->errcode = ea_refcount_create(0, &ctx->refcount);
		if (pctx->errcode) {
			pctx->num = 1;
			fix_problem(ctx, PR_1_ALLOCATE_REFCOUNT, pctx);
			ctx->flags |= E2F_FLAG_ABORT;
			return 0;
		}
	}

#if 0
	/* Debugging text */
	printf("Inode %u has EA block %u\n", ino, blk);
#endif

	/* Have we seen this EA block before? */
	if (ext2fs_fast_test_block_bitmap2(ctx->block_ea_map, blk)) {
		ea_block_quota->blocks = EXT2FS_C2B(fs, 1);
		ea_block_quota->inodes = 0;

		if (ctx->ea_block_quota_blocks) {
			ea_refcount_fetch(ctx->ea_block_quota_blocks, blk,
					  &quota_blocks);
			if (quota_blocks)
				ea_block_quota->blocks = quota_blocks;
		}

		if (ctx->ea_block_quota_inodes)
			ea_refcount_fetch(ctx->ea_block_quota_inodes, blk,
					  &ea_block_quota->inodes);

		if (ea_refcount_decrement(ctx->refcount, blk, 0) == 0)
			return 1;
		/* Ooops, this EA was referenced more than it stated */
		if (!ctx->refcount_extra) {
			pctx->errcode = ea_refcount_create(0,
					   &ctx->refcount_extra);
			if (pctx->errcode) {
				pctx->num = 2;
				fix_problem(ctx, PR_1_ALLOCATE_REFCOUNT, pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				return 0;
			}
		}
		ea_refcount_increment(ctx->refcount_extra, blk, 0);
		return 1;
	}

	/*
	 * OK, we haven't seen this EA block yet.  So we need to
	 * validate it
	 */
	pctx->blk = blk;
	pctx->errcode = ext2fs_read_ext_attr3(fs, blk, block_buf, pctx->ino);
	if (pctx->errcode == EXT2_ET_EXT_ATTR_CSUM_INVALID) {
		pctx->errcode = 0;
		failed_csum = 1;
	} else if (pctx->errcode == EXT2_ET_BAD_EA_HEADER)
		pctx->errcode = 0;

	if (pctx->errcode &&
	    fix_problem(ctx, PR_1_READ_EA_BLOCK, pctx)) {
		pctx->errcode = 0;
		goto clear_extattr;
	}
	header = (struct ext2_ext_attr_header *) block_buf;
	pctx->blk = ext2fs_file_acl_block(fs, inode);
	if (((ctx->ext_attr_ver == 1) &&
	     (header->h_magic != EXT2_EXT_ATTR_MAGIC_v1)) ||
	    ((ctx->ext_attr_ver == 2) &&
	     (header->h_magic != EXT2_EXT_ATTR_MAGIC))) {
		if (fix_problem(ctx, PR_1_BAD_EA_BLOCK, pctx))
			goto clear_extattr;
	}

	if (header->h_blocks != 1) {
		if (fix_problem(ctx, PR_1_EA_MULTI_BLOCK, pctx))
			goto clear_extattr;
	}

	if (pctx->errcode && fix_problem(ctx, PR_1_READ_EA_BLOCK, pctx))
		goto clear_extattr;

	region = region_create(0, fs->blocksize);
	if (!region) {
		fix_problem(ctx, PR_1_EA_ALLOC_REGION_ABORT, pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return 0;
	}
	if (region_allocate(region, 0, sizeof(struct ext2_ext_attr_header))) {
		if (fix_problem(ctx, PR_1_EA_ALLOC_COLLISION, pctx))
			goto clear_extattr;
	}

	first = (struct ext2_ext_attr_entry *)(header+1);
	end = block_buf + fs->blocksize;
	entry = first;
	while ((char *)entry < end && *(__u32 *)entry) {
		__u32 hash;

		if (region_allocate(region, (char *)entry - (char *)header,
			           EXT2_EXT_ATTR_LEN(entry->e_name_len))) {
			if (fix_problem(ctx, PR_1_EA_ALLOC_COLLISION, pctx))
				goto clear_extattr;
			break;
		}
		if ((ctx->ext_attr_ver == 1 &&
		     (entry->e_name_len == 0 || entry->e_name_index != 0)) ||
		    (ctx->ext_attr_ver == 2 &&
		     entry->e_name_index == 0)) {
			if (fix_problem(ctx, PR_1_EA_BAD_NAME, pctx))
				goto clear_extattr;
			break;
		}
		if (entry->e_value_inum == 0) {
			if (entry->e_value_offs + entry->e_value_size >
			    fs->blocksize) {
				if (fix_problem(ctx, PR_1_EA_BAD_VALUE, pctx))
					goto clear_extattr;
				break;
			}
			if (entry->e_value_size &&
			    region_allocate(region, entry->e_value_offs,
					    EXT2_EXT_ATTR_SIZE(entry->e_value_size))) {
				if (fix_problem(ctx, PR_1_EA_ALLOC_COLLISION,
						pctx))
					goto clear_extattr;
			}

			hash = ext2fs_ext_attr_hash_entry(entry, block_buf +
							  entry->e_value_offs);

			if (entry->e_hash != hash) {
				pctx->num = entry->e_hash;
				if (fix_problem(ctx, PR_1_ATTR_HASH, pctx))
					goto clear_extattr;
				entry->e_hash = hash;
			}
		} else {
			problem_t problem;
			blk64_t entry_quota_blocks;

			problem = check_large_ea_inode(ctx, entry, pctx,
						       &entry_quota_blocks);
			if (problem && fix_problem(ctx, problem, pctx))
				goto clear_extattr;

			quota_blocks += entry_quota_blocks;
			quota_inodes++;
		}

		entry = EXT2_EXT_ATTR_NEXT(entry);
	}
	if (region_allocate(region, (char *)entry - (char *)header, 4)) {
		if (fix_problem(ctx, PR_1_EA_ALLOC_COLLISION, pctx))
			goto clear_extattr;
	}
	region_free(region);

	/*
	 * We only get here if there was no other errors that were fixed.
	 * If there was a checksum fail, ask to correct it.
	 */
	if (failed_csum &&
	    fix_problem(ctx, PR_1_EA_BLOCK_ONLY_CSUM_INVALID, pctx)) {
		pctx->errcode = ext2fs_write_ext_attr3(fs, blk, block_buf,
						       pctx->ino);
		if (pctx->errcode)
			return 0;
	}

	if (quota_blocks != EXT2FS_C2B(fs, 1U)) {
		if (!ctx->ea_block_quota_blocks) {
			pctx->errcode = ea_refcount_create(0,
						&ctx->ea_block_quota_blocks);
			if (pctx->errcode) {
				pctx->num = 3;
				goto refcount_fail;
			}
		}
		ea_refcount_store(ctx->ea_block_quota_blocks, blk,
				  quota_blocks);
	}

	if (quota_inodes) {
		if (!ctx->ea_block_quota_inodes) {
			pctx->errcode = ea_refcount_create(0,
						&ctx->ea_block_quota_inodes);
			if (pctx->errcode) {
				pctx->num = 4;
refcount_fail:
				fix_problem(ctx, PR_1_ALLOCATE_REFCOUNT, pctx);
				ctx->flags |= E2F_FLAG_ABORT;
				return 0;
			}
		}

		ea_refcount_store(ctx->ea_block_quota_inodes, blk,
				  quota_inodes);
	}
	ea_block_quota->blocks = quota_blocks;
	ea_block_quota->inodes = quota_inodes;

	inc_ea_inode_refs(ctx, pctx, first, end);
	ea_refcount_store(ctx->refcount, blk, header->h_refcount - 1);
	mark_block_used(ctx, blk);
	ext2fs_fast_mark_block_bitmap2(ctx->block_ea_map, blk);
	return 1;

clear_extattr:
	if (region)
		region_free(region);
	ext2fs_file_acl_block_set(fs, inode, 0);
	e2fsck_write_inode(ctx, ino, inode, "check_ext_attr");
	return 0;
}

/* Returns 1 if bad htree, 0 if OK */
static int handle_htree(e2fsck_t ctx, struct problem_context *pctx,
			ext2_ino_t ino, struct ext2_inode *inode,
			char *block_buf)
{
	struct ext2_dx_root_info	*root;
	ext2_filsys			fs = ctx->fs;
	errcode_t			retval;
	blk64_t				blk;

	if ((!LINUX_S_ISDIR(inode->i_mode) &&
	     fix_problem(ctx, PR_1_HTREE_NODIR, pctx)) ||
	    (!ext2fs_has_feature_dir_index(fs->super) &&
	     fix_problem(ctx, PR_1_HTREE_SET, pctx)))
		return 1;

	pctx->errcode = ext2fs_bmap2(fs, ino, inode, 0, 0, 0, 0, &blk);

	if ((pctx->errcode) ||
	    (blk == 0) ||
	    (blk < fs->super->s_first_data_block) ||
	    (blk >= ext2fs_blocks_count(fs->super))) {
		if (fix_problem(ctx, PR_1_HTREE_BADROOT, pctx))
			return 1;
		else
			return 0;
	}

	retval = io_channel_read_blk64(fs->io, blk, 1, block_buf);
	if (retval && fix_problem(ctx, PR_1_HTREE_BADROOT, pctx))
		return 1;

	/* XXX should check that beginning matches a directory */
	root = (struct ext2_dx_root_info *) (block_buf + 24);

	if ((root->reserved_zero || root->info_length < 8) &&
	    fix_problem(ctx, PR_1_HTREE_BADROOT, pctx))
		return 1;

	pctx->num = root->hash_version;
	if ((root->hash_version != EXT2_HASH_LEGACY) &&
	    (root->hash_version != EXT2_HASH_HALF_MD4) &&
	    (root->hash_version != EXT2_HASH_TEA) &&
	    fix_problem(ctx, PR_1_HTREE_HASHV, pctx))
		return 1;

	if ((root->unused_flags & EXT2_HASH_FLAG_INCOMPAT) &&
	    fix_problem(ctx, PR_1_HTREE_INCOMPAT, pctx))
		return 1;

	pctx->num = root->indirect_levels;
	if ((root->indirect_levels > ext2_dir_htree_level(fs)) &&
	    fix_problem(ctx, PR_1_HTREE_DEPTH, pctx))
		return 1;

	return 0;
}

void e2fsck_clear_inode(e2fsck_t ctx, ext2_ino_t ino,
			struct ext2_inode *inode, int restart_flag,
			const char *source)
{
	inode->i_flags = 0;
	inode->i_links_count = 0;
	ext2fs_icount_store(ctx->inode_link_info, ino, 0);
	inode->i_dtime = ctx->now;

	/*
	 * If a special inode has such rotten block mappings that we
	 * want to clear the whole inode, be sure to actually zap
	 * the block maps because i_links_count isn't checked for
	 * special inodes, and we'll end up right back here the next
	 * time we run fsck.
	 */
	if (ino < EXT2_FIRST_INODE(ctx->fs->super))
		memset(inode->i_block, 0, sizeof(inode->i_block));

	ext2fs_unmark_inode_bitmap2(ctx->inode_dir_map, ino);
	ext2fs_unmark_inode_bitmap2(ctx->inode_used_map, ino);
	if (ctx->inode_reg_map)
		ext2fs_unmark_inode_bitmap2(ctx->inode_reg_map, ino);
	if (ctx->inode_bad_map)
		ext2fs_unmark_inode_bitmap2(ctx->inode_bad_map, ino);

	/*
	 * If the inode was partially accounted for before processing
	 * was aborted, we need to restart the pass 1 scan.
	 */
	ctx->flags |= restart_flag;

	if (ino == EXT2_BAD_INO)
		memset(inode, 0, sizeof(struct ext2_inode));

	e2fsck_write_inode(ctx, ino, inode, source);
}

/*
 * Use the multiple-blocks reclamation code to fix alignment problems in
 * a bigalloc filesystem.  We want a logical cluster to map to *only* one
 * physical cluster, and we want the block offsets within that cluster to
 * line up.
 */
static int has_unaligned_cluster_map(e2fsck_t ctx,
				     blk64_t last_pblk, blk64_t last_lblk,
				     blk64_t pblk, blk64_t lblk)
{
	blk64_t cluster_mask;

	if (!ctx->fs->cluster_ratio_bits)
		return 0;
	cluster_mask = EXT2FS_CLUSTER_MASK(ctx->fs);

	/*
	 * If the block in the logical cluster doesn't align with the block in
	 * the physical cluster...
	 */
	if ((lblk & cluster_mask) != (pblk & cluster_mask))
		return 1;

	/*
	 * If we cross a physical cluster boundary within a logical cluster...
	 */
	if (last_pblk && (lblk & cluster_mask) != 0 &&
	    EXT2FS_B2C(ctx->fs, lblk) == EXT2FS_B2C(ctx->fs, last_lblk) &&
	    EXT2FS_B2C(ctx->fs, pblk) != EXT2FS_B2C(ctx->fs, last_pblk))
		return 1;

	return 0;
}

static void scan_extent_node(e2fsck_t ctx, struct problem_context *pctx,
			     struct process_block_struct *pb,
			     blk64_t start_block, blk64_t end_block,
			     blk64_t eof_block,
			     ext2_extent_handle_t ehandle,
			     int try_repairs)
{
	struct ext2fs_extent	extent;
	blk64_t			blk, last_lblk;
	unsigned int		i, n;
	int			is_dir, is_leaf;
	problem_t		problem;
	struct ext2_extent_info	info;
	int			failed_csum = 0;

	if (pctx->errcode == EXT2_ET_EXTENT_CSUM_INVALID)
		failed_csum = 1;

	pctx->errcode = ext2fs_extent_get_info(ehandle, &info);
	if (pctx->errcode)
		return;
	if (!(ctx->options & E2F_OPT_FIXES_ONLY) &&
	    !pb->eti.force_rebuild) {
		struct extent_tree_level *etl;

		etl = pb->eti.ext_info + info.curr_level;
		etl->num_extents += info.num_entries;
		etl->max_extents += info.max_entries;
		/*
		 * Implementation wart: Splitting extent blocks when appending
		 * will leave the old block with one free entry.  Therefore
		 * unless the node is totally full, pretend that a non-root
		 * extent block can hold one fewer entry than it actually does,
		 * so that we don't repeatedly rebuild the extent tree.
		 */
		if (info.curr_level && info.num_entries < info.max_entries)
			etl->max_extents--;
	}

	pctx->errcode = ext2fs_extent_get(ehandle, EXT2_EXTENT_FIRST_SIB,
					  &extent);
	while ((pctx->errcode == 0 ||
		pctx->errcode == EXT2_ET_EXTENT_CSUM_INVALID) &&
	       info.num_entries-- > 0) {
		is_leaf = extent.e_flags & EXT2_EXTENT_FLAGS_LEAF;
		is_dir = LINUX_S_ISDIR(pctx->inode->i_mode);
		last_lblk = extent.e_lblk + extent.e_len - 1;

		problem = 0;
		pctx->blk = extent.e_pblk;
		pctx->blk2 = extent.e_lblk;
		pctx->num = extent.e_len;
		pctx->blkcount = extent.e_lblk + extent.e_len;

		if (extent.e_pblk == 0 ||
		    extent.e_pblk < ctx->fs->super->s_first_data_block ||
		    extent.e_pblk >= ext2fs_blocks_count(ctx->fs->super))
			problem = PR_1_EXTENT_BAD_START_BLK;
		else if (extent.e_lblk < start_block)
			problem = PR_1_OUT_OF_ORDER_EXTENTS;
		else if ((end_block && last_lblk > end_block) &&
			 !(last_lblk > eof_block &&
			   ((extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT) ||
			    (pctx->inode->i_flags & EXT4_VERITY_FL))))
			problem = PR_1_EXTENT_END_OUT_OF_BOUNDS;
		else if (is_leaf && extent.e_len == 0)
			problem = PR_1_EXTENT_LENGTH_ZERO;
		else if (is_leaf &&
			 (extent.e_pblk + extent.e_len) >
			 ext2fs_blocks_count(ctx->fs->super))
			problem = PR_1_EXTENT_ENDS_BEYOND;
		else if (is_leaf && is_dir &&
			 ((extent.e_lblk + extent.e_len) >
			  (1U << (21 - ctx->fs->super->s_log_block_size))))
			problem = PR_1_TOOBIG_DIR;

		if (is_leaf && problem == 0 && extent.e_len > 0) {
#if 0
			printf("extent_region(ino=%u, expect=%llu, "
			       "lblk=%llu, len=%u)\n",
			       pb->ino, pb->next_lblock,
			       extent.e_lblk, extent.e_len);
#endif
			if (extent.e_lblk < pb->next_lblock)
				problem = PR_1_EXTENT_COLLISION;
			else if (extent.e_lblk + extent.e_len > pb->next_lblock)
				pb->next_lblock = extent.e_lblk + extent.e_len;
		}

		/*
		 * Uninitialized blocks in a directory?  Clear the flag and
		 * we'll interpret the blocks later.
		 */
		if (try_repairs && is_dir && problem == 0 &&
		    (extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT) &&
		    fix_problem(ctx, PR_1_UNINIT_DBLOCK, pctx)) {
			extent.e_flags &= ~EXT2_EXTENT_FLAGS_UNINIT;
			pb->inode_modified = 1;
			pctx->errcode = ext2fs_extent_replace(ehandle, 0,
							      &extent);
			if (pctx->errcode)
				return;
			failed_csum = 0;
		}

		if (try_repairs && problem) {
report_problem:
			if (fix_problem(ctx, problem, pctx)) {
				if (ctx->invalid_bitmaps) {
					/*
					 * If fsck knows the bitmaps are bad,
					 * skip to the next extent and
					 * try to clear this extent again
					 * after fixing the bitmaps, by
					 * restarting fsck.
					 */
					pctx->errcode = ext2fs_extent_get(
							  ehandle,
							  EXT2_EXTENT_NEXT_SIB,
							  &extent);
					ctx->flags |= E2F_FLAG_RESTART_LATER;
					if (pctx->errcode ==
						    EXT2_ET_NO_CURRENT_NODE) {
						pctx->errcode = 0;
						break;
					}
					continue;
				}
				e2fsck_read_bitmaps(ctx);
				pb->inode_modified = 1;
				pctx->errcode =
					ext2fs_extent_delete(ehandle, 0);
				if (pctx->errcode) {
					pctx->str = "ext2fs_extent_delete";
					return;
				}
				pctx->errcode = ext2fs_extent_fix_parents(ehandle);
				if (pctx->errcode &&
				    pctx->errcode != EXT2_ET_NO_CURRENT_NODE) {
					pctx->str = "ext2fs_extent_fix_parents";
					return;
				}
				pctx->errcode = ext2fs_extent_get(ehandle,
								  EXT2_EXTENT_CURRENT,
								  &extent);
				if (pctx->errcode == EXT2_ET_NO_CURRENT_NODE) {
					pctx->errcode = 0;
					break;
				}
				failed_csum = 0;
				continue;
			}
			goto next;
		}

		if (!is_leaf) {
			blk64_t lblk = extent.e_lblk;
			int next_try_repairs = 1;

			blk = extent.e_pblk;

			/*
			 * If this lower extent block collides with critical
			 * metadata, don't try to repair the damage.  Pass 1b
			 * will reallocate the block; then we can try again.
			 */
			if (pb->ino != EXT2_RESIZE_INO &&
			    extent.e_pblk < ctx->fs->super->s_blocks_count &&
			    ext2fs_test_block_bitmap2(ctx->block_metadata_map,
						      extent.e_pblk)) {
				next_try_repairs = 0;
				pctx->blk = blk;
				fix_problem(ctx,
					    PR_1_CRITICAL_METADATA_COLLISION,
					    pctx);
				if ((ctx->options & E2F_OPT_NO) == 0)
					ctx->flags |= E2F_FLAG_RESTART_LATER;
			}
			pctx->errcode = ext2fs_extent_get(ehandle,
						  EXT2_EXTENT_DOWN, &extent);
			if (pctx->errcode &&
			    pctx->errcode != EXT2_ET_EXTENT_CSUM_INVALID) {
				pctx->str = "EXT2_EXTENT_DOWN";
				problem = PR_1_EXTENT_HEADER_INVALID;
				if (!next_try_repairs)
					return;
				if (pctx->errcode == EXT2_ET_EXTENT_HEADER_BAD)
					goto report_problem;
				return;
			}
			/* The next extent should match this index's logical start */
			if (extent.e_lblk != lblk) {
				struct ext2_extent_info e_info;

				ext2fs_extent_get_info(ehandle, &e_info);
				pctx->blk = lblk;
				pctx->blk2 = extent.e_lblk;
				pctx->num = e_info.curr_level - 1;
				problem = PR_1_EXTENT_INDEX_START_INVALID;
				if (fix_problem(ctx, problem, pctx)) {
					pb->inode_modified = 1;
					pctx->errcode =
						ext2fs_extent_fix_parents(ehandle);
					if (pctx->errcode) {
						pctx->str = "ext2fs_extent_fix_parents";
						return;
					}
				}
			}
			scan_extent_node(ctx, pctx, pb, extent.e_lblk,
					 last_lblk, eof_block, ehandle,
					 next_try_repairs);
			if (pctx->errcode)
				return;
			pctx->errcode = ext2fs_extent_get(ehandle,
						  EXT2_EXTENT_UP, &extent);
			if (pctx->errcode) {
				pctx->str = "EXT2_EXTENT_UP";
				return;
			}
			mark_block_used(ctx, blk);
			pb->num_blocks++;
			goto next;
		}

		if ((pb->previous_block != 0) &&
		    (pb->previous_block+1 != extent.e_pblk)) {
			if (ctx->options & E2F_OPT_FRAGCHECK) {
				char type = '?';

				if (pb->is_dir)
					type = 'd';
				else if (pb->is_reg)
					type = 'f';

				printf(("%6lu(%c): expecting %6lu "
					"actual extent "
					"phys %6lu log %lu len %lu\n"),
				       (unsigned long) pctx->ino, type,
				       (unsigned long) pb->previous_block+1,
				       (unsigned long) extent.e_pblk,
				       (unsigned long) extent.e_lblk,
				       (unsigned long) extent.e_len);
			}
			pb->fragmented = 1;
		}
		/*
		 * If we notice a gap in the logical block mappings of an
		 * extent-mapped directory, offer to close the hole by
		 * moving the logical block down, otherwise we'll go mad in
		 * pass 3 allocating empty directory blocks to fill the hole.
		 */
		if (try_repairs && is_dir &&
		    pb->last_block + 1 < extent.e_lblk) {
			blk64_t new_lblk;

			new_lblk = pb->last_block + 1;
			if (EXT2FS_CLUSTER_RATIO(ctx->fs) > 1)
				new_lblk = ((new_lblk +
					     EXT2FS_CLUSTER_RATIO(ctx->fs) - 1) &
					    ~EXT2FS_CLUSTER_MASK(ctx->fs)) |
					   (extent.e_pblk &
					    EXT2FS_CLUSTER_MASK(ctx->fs));
			pctx->blk = extent.e_lblk;
			pctx->blk2 = new_lblk;
			if (fix_problem(ctx, PR_1_COLLAPSE_DBLOCK, pctx)) {
				extent.e_lblk = new_lblk;
				pb->inode_modified = 1;
				pctx->errcode = ext2fs_extent_replace(ehandle,
								0, &extent);
				if (pctx->errcode) {
					pctx->errcode = 0;
					goto alloc_later;
				}
				pctx->errcode = ext2fs_extent_fix_parents(ehandle);
				if (pctx->errcode)
					goto failed_add_dir_block;
				pctx->errcode = ext2fs_extent_goto(ehandle,
								extent.e_lblk);
				if (pctx->errcode)
					goto failed_add_dir_block;
				last_lblk = extent.e_lblk + extent.e_len - 1;
				failed_csum = 0;
			}
		}
alloc_later:
		if (is_dir) {
			while (++pb->last_db_block <
			       (e2_blkcnt_t) extent.e_lblk) {
				pctx->errcode = ext2fs_add_dir_block2(
							ctx->fs->dblist,
							pb->ino, 0,
							pb->last_db_block);
				if (pctx->errcode) {
					pctx->blk = 0;
					pctx->num = pb->last_db_block;
					goto failed_add_dir_block;
				}
			}

			for (i = 0; i < extent.e_len; i++) {
				pctx->errcode = ext2fs_add_dir_block2(
							ctx->fs->dblist,
							pctx->ino,
							extent.e_pblk + i,
							extent.e_lblk + i);
				if (pctx->errcode) {
					pctx->blk = extent.e_pblk + i;
					pctx->num = extent.e_lblk + i;
				failed_add_dir_block:
					fix_problem(ctx, PR_1_ADD_DBLOCK, pctx);
					/* Should never get here */
					ctx->flags |= E2F_FLAG_ABORT;
					return;
				}
			}
			if (extent.e_len > 0)
				pb->last_db_block = extent.e_lblk + extent.e_len - 1;
		}
		if (has_unaligned_cluster_map(ctx, pb->previous_block,
					      pb->last_block,
					      extent.e_pblk,
					      extent.e_lblk)) {
			for (i = 0; i < extent.e_len; i++) {
				pctx->blk = extent.e_lblk + i;
				pctx->blk2 = extent.e_pblk + i;
				fix_problem(ctx, PR_1_MISALIGNED_CLUSTER, pctx);
				mark_block_used(ctx, extent.e_pblk + i);
				mark_block_used(ctx, extent.e_pblk + i);
			}
		}

		/*
		 * Check whether first cluster got marked in previous iteration.
		 */
		if (ctx->fs->cluster_ratio_bits &&
		    pb->previous_block &&
		    (EXT2FS_B2C(ctx->fs, extent.e_pblk) ==
		     EXT2FS_B2C(ctx->fs, pb->previous_block)))
			/* Set blk to the beginning of next cluster. */
			blk = EXT2FS_C2B(
				ctx->fs,
				EXT2FS_B2C(ctx->fs, extent.e_pblk) + 1);
		else
			/* Set blk to the beginning of current cluster. */
			blk = EXT2FS_C2B(ctx->fs,
					 EXT2FS_B2C(ctx->fs, extent.e_pblk));

		if (blk < extent.e_pblk + extent.e_len) {
			mark_blocks_used(ctx, blk,
					 extent.e_pblk + extent.e_len - blk);
			n = DIV_ROUND_UP(extent.e_pblk + extent.e_len - blk,
					 EXT2FS_CLUSTER_RATIO(ctx->fs));
			pb->num_blocks += n;
		}
		pb->last_block = extent.e_lblk + extent.e_len - 1;
		pb->previous_block = extent.e_pblk + extent.e_len - 1;
		start_block = pb->last_block = last_lblk;
		if (is_leaf && !is_dir &&
		    !(extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT))
			pb->last_init_lblock = last_lblk;
	next:
		pctx->errcode = ext2fs_extent_get(ehandle,
						  EXT2_EXTENT_NEXT_SIB,
						  &extent);
	}

	/* Failed csum but passes checks?  Ask to fix checksum. */
	if (failed_csum &&
	    fix_problem(ctx, PR_1_EXTENT_ONLY_CSUM_INVALID, pctx)) {
		pb->inode_modified = 1;
		pctx->errcode = ext2fs_extent_replace(ehandle, 0, &extent);
		if (pctx->errcode)
			return;
	}

	if (pctx->errcode == EXT2_ET_EXTENT_NO_NEXT)
		pctx->errcode = 0;
}

static void check_blocks_extents(e2fsck_t ctx, struct problem_context *pctx,
				 struct process_block_struct *pb)
{
	struct ext2_extent_info info;
	struct ext2_inode	*inode = pctx->inode;
	ext2_extent_handle_t	ehandle;
	ext2_filsys		fs = ctx->fs;
	ext2_ino_t		ino = pctx->ino;
	errcode_t		retval;
	blk64_t                 eof_lblk;
	struct ext3_extent_header	*eh;

	/* Check for a proper extent header... */
	eh = (struct ext3_extent_header *) &inode->i_block[0];
	retval = ext2fs_extent_header_verify(eh, sizeof(inode->i_block));
	if (retval) {
		if (fix_problem(ctx, PR_1_MISSING_EXTENT_HEADER, pctx))
			e2fsck_clear_inode(ctx, ino, inode, 0,
					   "check_blocks_extents");
		pctx->errcode = 0;
		return;
	}

	/* ...since this function doesn't fail if i_block is zeroed. */
	pctx->errcode = ext2fs_extent_open2(fs, ino, inode, &ehandle);
	if (pctx->errcode) {
		if (fix_problem(ctx, PR_1_READ_EXTENT, pctx))
			e2fsck_clear_inode(ctx, ino, inode, 0,
					   "check_blocks_extents");
		pctx->errcode = 0;
		return;
	}

	retval = ext2fs_extent_get_info(ehandle, &info);
	if (retval == 0) {
		int max_depth = info.max_depth;

		if (max_depth >= MAX_EXTENT_DEPTH_COUNT)
			max_depth = MAX_EXTENT_DEPTH_COUNT-1;
		ctx->extent_depth_count[max_depth]++;
	}

	/* Check maximum extent depth */
	pctx->blk = info.max_depth;
	pctx->blk2 = ext2fs_max_extent_depth(ehandle);
	if (pctx->blk2 < pctx->blk &&
	    fix_problem(ctx, PR_1_EXTENT_BAD_MAX_DEPTH, pctx))
		pb->eti.force_rebuild = 1;

	/* Can we collect extent tree level stats? */
	pctx->blk = MAX_EXTENT_DEPTH_COUNT;
	if (pctx->blk2 > pctx->blk)
		fix_problem(ctx, PR_1E_MAX_EXTENT_TREE_DEPTH, pctx);
	memset(pb->eti.ext_info, 0, sizeof(pb->eti.ext_info));
	pb->eti.ino = pb->ino;

	pb->next_lblock = 0;

	eof_lblk = ((EXT2_I_SIZE(inode) + fs->blocksize - 1) >>
		EXT2_BLOCK_SIZE_BITS(fs->super)) - 1;
	scan_extent_node(ctx, pctx, pb, 0, 0, eof_lblk, ehandle, 1);
	if (pctx->errcode &&
	    fix_problem(ctx, PR_1_EXTENT_ITERATE_FAILURE, pctx)) {
		pb->num_blocks = 0;
		inode->i_blocks = 0;
		e2fsck_clear_inode(ctx, ino, inode, E2F_FLAG_RESTART,
				   "check_blocks_extents");
		pctx->errcode = 0;
	}
	ext2fs_extent_free(ehandle);

	/* Rebuild unless it's a dir and we're rehashing it */
	if (LINUX_S_ISDIR(inode->i_mode) &&
	    e2fsck_dir_will_be_rehashed(ctx, ino))
		return;

	if (ctx->options & E2F_OPT_CONVERT_BMAP)
		e2fsck_rebuild_extents_later(ctx, ino);
	else
		e2fsck_should_rebuild_extents(ctx, pctx, &pb->eti, &info);
}

/*
 * In fact we don't need to check blocks for an inode with inline data
 * because this inode doesn't have any blocks.  In this function all
 * we need to do is add this inode into dblist when it is a directory.
 */
static void check_blocks_inline_data(e2fsck_t ctx, struct problem_context *pctx,
				     struct process_block_struct *pb)
{
	int	flags;
	size_t	inline_data_size = 0;

	if (!pb->is_dir) {
		pctx->errcode = 0;
		return;
	}

	/* Process the dirents in i_block[] as the "first" block. */
	pctx->errcode = ext2fs_add_dir_block2(ctx->fs->dblist, pb->ino, 0, 0);
	if (pctx->errcode)
		goto err;

	/* Process the dirents in the EA as a "second" block. */
	flags = ctx->fs->flags;
	ctx->fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
	pctx->errcode = ext2fs_inline_data_size(ctx->fs, pb->ino,
						&inline_data_size);
	ctx->fs->flags = (flags & EXT2_FLAG_IGNORE_CSUM_ERRORS) |
			 (ctx->fs->flags & ~EXT2_FLAG_IGNORE_CSUM_ERRORS);
	if (pctx->errcode) {
		pctx->errcode = 0;
		return;
	}

	if (inline_data_size <= EXT4_MIN_INLINE_DATA_SIZE)
		return;

	pctx->errcode = ext2fs_add_dir_block2(ctx->fs->dblist, pb->ino, 0, 1);
	if (pctx->errcode)
		goto err;

	return;
err:
	pctx->blk = 0;
	pctx->num = 0;
	fix_problem(ctx, PR_1_ADD_DBLOCK, pctx);
	ctx->flags |= E2F_FLAG_ABORT;
}

/*
 * This subroutine is called on each inode to account for all of the
 * blocks used by that inode.
 */
static void check_blocks(e2fsck_t ctx, struct problem_context *pctx,
			 char *block_buf, const struct ea_quota *ea_ibody_quota)
{
	ext2_filsys fs = ctx->fs;
	struct process_block_struct pb;
	ext2_ino_t	ino = pctx->ino;
	struct ext2_inode *inode = pctx->inode;
	unsigned	bad_size = 0;
	int		dirty_inode = 0;
	int		extent_fs;
	int		inlinedata_fs;
	__u64		size;
	struct ea_quota	ea_block_quota;

	pb.ino = ino;
	pb.num_blocks = EXT2FS_B2C(ctx->fs,
				   ea_ibody_quota ? ea_ibody_quota->blocks : 0);
	pb.last_block = ~0;
	pb.last_init_lblock = -1;
	pb.last_db_block = -1;
	pb.num_illegal_blocks = 0;
	pb.suppress = 0; pb.clear = 0;
	pb.fragmented = 0;
	pb.compressed = 0;
	pb.previous_block = 0;
	pb.is_dir = LINUX_S_ISDIR(inode->i_mode);
	pb.is_reg = LINUX_S_ISREG(inode->i_mode);
	pb.max_blocks = 1U << (31 - fs->super->s_log_block_size);
	pb.inode = inode;
	pb.pctx = pctx;
	pb.ctx = ctx;
	pb.inode_modified = 0;
	pb.eti.force_rebuild = 0;
	pctx->ino = ino;
	pctx->errcode = 0;

	extent_fs = ext2fs_has_feature_extents(ctx->fs->super);
	inlinedata_fs = ext2fs_has_feature_inline_data(ctx->fs->super);

	if (check_ext_attr(ctx, pctx, block_buf, &ea_block_quota)) {
		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			goto out;
		pb.num_blocks += EXT2FS_B2C(ctx->fs, ea_block_quota.blocks);
	}

	if (inlinedata_fs && (inode->i_flags & EXT4_INLINE_DATA_FL))
		check_blocks_inline_data(ctx, pctx, &pb);
	else if (ext2fs_inode_has_valid_blocks2(fs, inode)) {
		if (extent_fs && (inode->i_flags & EXT4_EXTENTS_FL))
			check_blocks_extents(ctx, pctx, &pb);
		else {
			int flags;
			/*
			 * If we've modified the inode, write it out before
			 * iterate() tries to use it.
			 */
			if (dirty_inode) {
				e2fsck_write_inode(ctx, ino, inode,
						   "check_blocks");
				dirty_inode = 0;
			}
			flags = fs->flags;
			fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
			pctx->errcode = ext2fs_block_iterate3(fs, ino,
						pb.is_dir ? BLOCK_FLAG_HOLE : 0,
						block_buf, process_block, &pb);
			/*
			 * We do not have uninitialized extents in non extent
			 * files.
			 */
			pb.last_init_lblock = pb.last_block;
			/*
			 * If iterate() changed a block mapping, we have to
			 * re-read the inode.  If we decide to clear the
			 * inode after clearing some stuff, we'll re-write the
			 * bad mappings into the inode!
			 */
			if (pb.inode_modified)
				e2fsck_read_inode(ctx, ino, inode,
						  "check_blocks");
			fs->flags = (flags & EXT2_FLAG_IGNORE_CSUM_ERRORS) |
				    (fs->flags & ~EXT2_FLAG_IGNORE_CSUM_ERRORS);

			if (ctx->options & E2F_OPT_CONVERT_BMAP) {
#ifdef DEBUG
				printf("bmap rebuild ino=%d\n", ino);
#endif
				if (!LINUX_S_ISDIR(inode->i_mode) ||
				    !e2fsck_dir_will_be_rehashed(ctx, ino))
					e2fsck_rebuild_extents_later(ctx, ino);
			}
		}
	}
	end_problem_latch(ctx, PR_LATCH_BLOCK);
	end_problem_latch(ctx, PR_LATCH_TOOBIG);
	if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
		goto out;
	if (pctx->errcode)
		fix_problem(ctx, PR_1_BLOCK_ITERATE, pctx);

	if (pb.fragmented && pb.num_blocks < fs->super->s_blocks_per_group) {
		if (LINUX_S_ISDIR(inode->i_mode))
			ctx->fs_fragmented_dir++;
		else
			ctx->fs_fragmented++;
	}

	if (pb.clear) {
		e2fsck_clear_inode(ctx, ino, inode, E2F_FLAG_RESTART,
				   "check_blocks");
		return;
	}

	if (inode->i_flags & EXT2_INDEX_FL) {
		if (handle_htree(ctx, pctx, ino, inode, block_buf)) {
			inode->i_flags &= ~EXT2_INDEX_FL;
			dirty_inode++;
		} else {
			e2fsck_add_dx_dir(ctx, ino, inode, pb.last_block+1);
		}
	}

	if (!pb.num_blocks && pb.is_dir &&
	    !(inode->i_flags & EXT4_INLINE_DATA_FL)) {
		if (fix_problem(ctx, PR_1_ZERO_LENGTH_DIR, pctx)) {
			e2fsck_clear_inode(ctx, ino, inode, 0, "check_blocks");
			ctx->fs_directory_count--;
			return;
		}
	}

	if (ino != quota_type2inum(PRJQUOTA, fs->super) &&
	    (ino == EXT2_ROOT_INO || ino >= EXT2_FIRST_INODE(ctx->fs->super)) &&
	    !(inode->i_flags & EXT4_EA_INODE_FL)) {
		quota_data_add(ctx->qctx, (struct ext2_inode_large *) inode,
			       ino,
			       pb.num_blocks * EXT2_CLUSTER_SIZE(fs->super));
		quota_data_inodes(ctx->qctx, (struct ext2_inode_large *) inode,
				  ino, (ea_ibody_quota ?
					ea_ibody_quota->inodes : 0) +
						ea_block_quota.inodes + 1);
	}

	if (!ext2fs_has_feature_huge_file(fs->super) ||
	    !(inode->i_flags & EXT4_HUGE_FILE_FL))
		pb.num_blocks *= (fs->blocksize / 512);
	pb.num_blocks *= EXT2FS_CLUSTER_RATIO(fs);
#if 0
	printf("inode %u, i_size = %u, last_block = %llu, i_blocks=%llu, num_blocks = %llu\n",
	       ino, inode->i_size, pb.last_block, ext2fs_inode_i_blocks(fs, inode),
	       pb.num_blocks);
#endif
	if (pb.is_dir) {
		unsigned nblock = inode->i_size >> EXT2_BLOCK_SIZE_BITS(fs->super);
		if (inode->i_flags & EXT4_INLINE_DATA_FL) {
			int flags;
			size_t sz = 0;
			errcode_t err;

			flags = ctx->fs->flags;
			ctx->fs->flags |= EXT2_FLAG_IGNORE_CSUM_ERRORS;
			err = ext2fs_inline_data_size(ctx->fs, pctx->ino,
						      &sz);
			ctx->fs->flags = (flags &
					  EXT2_FLAG_IGNORE_CSUM_ERRORS) |
					 (ctx->fs->flags &
					  ~EXT2_FLAG_IGNORE_CSUM_ERRORS);
			if (err || sz != inode->i_size) {
				bad_size = 7;
				pctx->num = sz;
			}
		} else if (inode->i_size & (fs->blocksize - 1))
			bad_size = 5;
		else if (nblock > (pb.last_block + 1))
			bad_size = 1;
		else if (nblock < (pb.last_block + 1)) {
			if (((pb.last_block + 1) - nblock) >
			    fs->super->s_prealloc_dir_blocks)
				bad_size = 2;
		}
	} else {
		size = EXT2_I_SIZE(inode);
		if ((pb.last_init_lblock >= 0) &&
		    /* Do not allow initialized allocated blocks past i_size*/
		    (size < (__u64)pb.last_init_lblock * fs->blocksize) &&
		    !(inode->i_flags & EXT4_VERITY_FL))
			bad_size = 3;
		else if (!(extent_fs && (inode->i_flags & EXT4_EXTENTS_FL)) &&
			 size > ext2_max_sizes[fs->super->s_log_block_size])
			/* too big for a direct/indirect-mapped file */
			bad_size = 4;
		else if ((extent_fs && (inode->i_flags & EXT4_EXTENTS_FL)) &&
			 size >
			 ((1ULL << (32 + EXT2_BLOCK_SIZE_BITS(fs->super))) - 1))
			/* too big for an extent-based file - 32bit ee_block */
			bad_size = 6;
	}
	/* i_size for symlinks is checked elsewhere */
	if (bad_size && !LINUX_S_ISLNK(inode->i_mode)) {
		/* Did inline_data set pctx->num earlier? */
		if (bad_size != 7)
			pctx->num = (pb.last_block + 1) * fs->blocksize;
		pctx->group = bad_size;
		if (fix_problem(ctx, PR_1_BAD_I_SIZE, pctx)) {
			if (LINUX_S_ISDIR(inode->i_mode))
				pctx->num &= 0xFFFFFFFFULL;
			ext2fs_inode_size_set(fs, inode, pctx->num);
			if (EXT2_I_SIZE(inode) == 0 &&
			    (inode->i_flags & EXT4_INLINE_DATA_FL)) {
				memset(inode->i_block, 0,
				       sizeof(inode->i_block));
				inode->i_flags &= ~EXT4_INLINE_DATA_FL;
			}
			dirty_inode++;
		}
		pctx->num = 0;
	}
	if (LINUX_S_ISREG(inode->i_mode) &&
	    ext2fs_needs_large_file_feature(EXT2_I_SIZE(inode)))
		ctx->large_files++;
	if ((fs->super->s_creator_os != EXT2_OS_HURD) &&
	    ((pb.num_blocks != ext2fs_inode_i_blocks(fs, inode)) ||
	     (ext2fs_has_feature_huge_file(fs->super) &&
	      (inode->i_flags & EXT4_HUGE_FILE_FL) &&
	      (inode->osd2.linux2.l_i_blocks_hi != 0)))) {
		pctx->num = pb.num_blocks;
		if (fix_problem(ctx, PR_1_BAD_I_BLOCKS, pctx)) {
			inode->i_blocks = pb.num_blocks;
			inode->osd2.linux2.l_i_blocks_hi = pb.num_blocks >> 32;
			dirty_inode++;
		}
		pctx->num = 0;
	}

	/*
	 * The kernel gets mad if we ask it to allocate bigalloc clusters to
	 * a block mapped file, so rebuild it as an extent file.  We can skip
	 * symlinks because they're never rewritten.
	 */
	if (ext2fs_has_feature_bigalloc(fs->super) &&
	    (LINUX_S_ISREG(inode->i_mode) || LINUX_S_ISDIR(inode->i_mode)) &&
	    ext2fs_inode_data_blocks2(fs, inode) > 0 &&
	    (ino == EXT2_ROOT_INO || ino >= EXT2_FIRST_INO(fs->super)) &&
	    !(inode->i_flags & (EXT4_EXTENTS_FL | EXT4_INLINE_DATA_FL)) &&
	    fix_problem(ctx, PR_1_NO_BIGALLOC_BLOCKMAP_FILES, pctx)) {
		pctx->errcode = e2fsck_rebuild_extents_later(ctx, ino);
		if (pctx->errcode)
			goto out;
	}

	if (ctx->dirs_to_hash && pb.is_dir &&
	    !(ctx->lost_and_found && ctx->lost_and_found == ino) &&
	    !(inode->i_flags & EXT2_INDEX_FL) &&
	    ((inode->i_size / fs->blocksize) >= 3))
		e2fsck_rehash_dir_later(ctx, ino);

out:
	if (dirty_inode)
		e2fsck_write_inode(ctx, ino, inode, "check_blocks");
}

#if 0
/*
 * Helper function called by process block when an illegal block is
 * found.  It returns a description about why the block is illegal
 */
static char *describe_illegal_block(ext2_filsys fs, blk64_t block)
{
	blk64_t	super;
	int	i;
	static char	problem[80];

	super = fs->super->s_first_data_block;
	strcpy(problem, "PROGRAMMING ERROR: Unknown reason for illegal block");
	if (block < super) {
		sprintf(problem, "< FIRSTBLOCK (%u)", super);
		return(problem);
	} else if (block >= ext2fs_blocks_count(fs->super)) {
		sprintf(problem, "> BLOCKS (%u)", ext2fs_blocks_count(fs->super));
		return(problem);
	}
	for (i = 0; i < fs->group_desc_count; i++) {
		if (block == super) {
			sprintf(problem, "is the superblock in group %d", i);
			break;
		}
		if (block > super &&
		    block <= (super + fs->desc_blocks)) {
			sprintf(problem, "is in the group descriptors "
				"of group %d", i);
			break;
		}
		if (block == ext2fs_block_bitmap_loc(fs, i)) {
			sprintf(problem, "is the block bitmap of group %d", i);
			break;
		}
		if (block == ext2fs_inode_bitmap_loc(fs, i)) {
			sprintf(problem, "is the inode bitmap of group %d", i);
			break;
		}
		if (block >= ext2fs_inode_table_loc(fs, i) &&
		    (block < ext2fs_inode_table_loc(fs, i)
		     + fs->inode_blocks_per_group)) {
			sprintf(problem, "is in the inode table of group %d",
				i);
			break;
		}
		super += fs->super->s_blocks_per_group;
	}
	return(problem);
}
#endif

/*
 * This is a helper function for check_blocks().
 */
static int process_block(ext2_filsys fs,
		  blk64_t	*block_nr,
		  e2_blkcnt_t blockcnt,
		  blk64_t ref_block EXT2FS_ATTR((unused)),
		  int ref_offset EXT2FS_ATTR((unused)),
		  void *priv_data)
{
	struct process_block_struct *p;
	struct problem_context *pctx;
	blk64_t	blk = *block_nr;
	int	ret_code = 0;
	problem_t	problem = 0;
	e2fsck_t	ctx;

	p = (struct process_block_struct *) priv_data;
	pctx = p->pctx;
	ctx = p->ctx;

	/*
	 * For a directory, add logical block zero for processing even if it's
	 * not mapped or we'll be perennially stuck with broken "." and ".."
	 * entries.
	 */
	if (p->is_dir && blockcnt == 0 && blk == 0) {
		pctx->errcode = ext2fs_add_dir_block2(fs->dblist, p->ino, 0, 0);
		if (pctx->errcode) {
			pctx->blk = blk;
			pctx->num = blockcnt;
			goto failed_add_dir_block;
		}
		p->last_db_block++;
	}

	if (blk == 0)
		return 0;

#if 0
	printf("Process_block, inode %lu, block %u, #%d\n", p->ino, blk,
	       blockcnt);
#endif

	/*
	 * Simplistic fragmentation check.  We merely require that the
	 * file be contiguous.  (Which can never be true for really
	 * big files that are greater than a block group.)
	 */
	if (p->previous_block && p->ino != EXT2_RESIZE_INO) {
		if (p->previous_block+1 != blk) {
			if (ctx->options & E2F_OPT_FRAGCHECK) {
				char type = '?';

				if (p->is_dir)
					type = 'd';
				else if (p->is_reg)
					type = 'f';

				printf(_("%6lu(%c): expecting %6lu "
					 "got phys %6lu (blkcnt %lld)\n"),
				       (unsigned long) pctx->ino, type,
				       (unsigned long) p->previous_block+1,
				       (unsigned long) blk,
				       blockcnt);
			}
			p->fragmented = 1;
		}
	}

	if (p->is_dir && !ext2fs_has_feature_largedir(fs->super) &&
	    blockcnt > (1 << (21 - fs->super->s_log_block_size)))
		problem = PR_1_TOOBIG_DIR;
	if (p->is_dir && p->num_blocks + 1 >= p->max_blocks)
		problem = PR_1_TOOBIG_DIR;
	if (p->is_reg && p->num_blocks + 1 >= p->max_blocks)
		problem = PR_1_TOOBIG_REG;
	if (!p->is_dir && !p->is_reg && blockcnt > 0)
		problem = PR_1_TOOBIG_SYMLINK;

	if (blk < fs->super->s_first_data_block ||
	    blk >= ext2fs_blocks_count(fs->super))
		problem = PR_1_ILLEGAL_BLOCK_NUM;

	/*
	 * If this IND/DIND/TIND block is squatting atop some critical metadata
	 * (group descriptors, superblock, bitmap, inode table), any write to
	 * "fix" mapping problems will destroy the metadata.  We'll let pass 1b
	 * fix that and restart fsck.
	 */
	if (blockcnt < 0 &&
	    p->ino != EXT2_RESIZE_INO &&
	    blk < ctx->fs->super->s_blocks_count &&
	    ext2fs_test_block_bitmap2(ctx->block_metadata_map, blk)) {
		pctx->blk = blk;
		fix_problem(ctx, PR_1_CRITICAL_METADATA_COLLISION, pctx);
		if ((ctx->options & E2F_OPT_NO) == 0)
			ctx->flags |= E2F_FLAG_RESTART_LATER;
	}

	if (problem) {
		p->num_illegal_blocks++;
		/*
		 * A bit of subterfuge here -- we're trying to fix a block
		 * mapping, but the IND/DIND/TIND block could have collided
		 * with some critical metadata.  So, fix the in-core mapping so
		 * iterate won't go insane, but return 0 instead of
		 * BLOCK_CHANGED so that it won't write the remapping out to
		 * our multiply linked block.
		 *
		 * Even if we previously determined that an *IND block
		 * conflicts with critical metadata, we must still try to
		 * iterate the *IND block as if it is an *IND block to find and
		 * mark the blocks it points to.  Better to be overly cautious
		 * with the used_blocks map so that we don't move the *IND
		 * block to a block that's really in use!
		 */
		if (p->ino != EXT2_RESIZE_INO &&
		    ref_block != 0 &&
		    ext2fs_test_block_bitmap2(ctx->block_metadata_map,
					      ref_block)) {
			*block_nr = 0;
			return 0;
		}
		if (!p->suppress && (p->num_illegal_blocks % 12) == 0) {
			if (fix_problem(ctx, PR_1_TOO_MANY_BAD_BLOCKS, pctx)) {
				p->clear = 1;
				return BLOCK_ABORT;
			}
			if (fix_problem(ctx, PR_1_SUPPRESS_MESSAGES, pctx)) {
				p->suppress = 1;
				set_latch_flags(PR_LATCH_BLOCK,
						PRL_SUPPRESS, 0);
			}
		}
		pctx->blk = blk;
		pctx->blkcount = blockcnt;
		if (fix_problem(ctx, problem, pctx)) {
			blk = *block_nr = 0;
			ret_code = BLOCK_CHANGED;
			p->inode_modified = 1;
			/*
			 * If the directory block is too big and is beyond the
			 * end of the FS, don't bother trying to add it for
			 * processing -- the kernel would never have created a
			 * directory this large, and we risk an ENOMEM abort.
			 * In any case, the toobig handler for extent-based
			 * directories also doesn't feed toobig blocks to
			 * pass 2.
			 */
			if (problem == PR_1_TOOBIG_DIR)
				return ret_code;
			goto mark_dir;
		} else
			return 0;
	}

	if (p->ino == EXT2_RESIZE_INO) {
		/*
		 * The resize inode has already be sanity checked
		 * during pass #0 (the superblock checks).  All we
		 * have to do is mark the double indirect block as
		 * being in use; all of the other blocks are handled
		 * by mark_table_blocks()).
		 */
		if (blockcnt == BLOCK_COUNT_DIND)
			mark_block_used(ctx, blk);
		p->num_blocks++;
	} else if (!(ctx->fs->cluster_ratio_bits &&
		     p->previous_block &&
		     (EXT2FS_B2C(ctx->fs, blk) ==
		      EXT2FS_B2C(ctx->fs, p->previous_block)) &&
		     (blk & EXT2FS_CLUSTER_MASK(ctx->fs)) ==
		     ((unsigned) blockcnt & EXT2FS_CLUSTER_MASK(ctx->fs)))) {
		mark_block_used(ctx, blk);
		p->num_blocks++;
	} else if (has_unaligned_cluster_map(ctx, p->previous_block,
					     p->last_block, blk, blockcnt)) {
		pctx->blk = blockcnt;
		pctx->blk2 = blk;
		fix_problem(ctx, PR_1_MISALIGNED_CLUSTER, pctx);
		mark_block_used(ctx, blk);
		mark_block_used(ctx, blk);
	}
	if (blockcnt >= 0)
		p->last_block = blockcnt;
	p->previous_block = blk;
mark_dir:
	if (p->is_dir && (blockcnt >= 0)) {
		while (++p->last_db_block < blockcnt) {
			pctx->errcode = ext2fs_add_dir_block2(fs->dblist,
							      p->ino, 0,
							      p->last_db_block);
			if (pctx->errcode) {
				pctx->blk = 0;
				pctx->num = p->last_db_block;
				goto failed_add_dir_block;
			}
		}
		pctx->errcode = ext2fs_add_dir_block2(fs->dblist, p->ino,
						      blk, blockcnt);
		if (pctx->errcode) {
			pctx->blk = blk;
			pctx->num = blockcnt;
		failed_add_dir_block:
			fix_problem(ctx, PR_1_ADD_DBLOCK, pctx);
			/* Should never get here */
			ctx->flags |= E2F_FLAG_ABORT;
			return BLOCK_ABORT;
		}
	}
	return ret_code;
}

static int process_bad_block(ext2_filsys fs,
		      blk64_t *block_nr,
		      e2_blkcnt_t blockcnt,
		      blk64_t ref_block EXT2FS_ATTR((unused)),
		      int ref_offset EXT2FS_ATTR((unused)),
		      void *priv_data)
{
	struct process_block_struct *p;
	blk64_t		blk = *block_nr;
	blk64_t		first_block;
	dgrp_t		i;
	struct problem_context *pctx;
	e2fsck_t	ctx;

	if (!blk)
		return 0;

	p = (struct process_block_struct *) priv_data;
	ctx = p->ctx;
	pctx = p->pctx;

	pctx->ino = EXT2_BAD_INO;
	pctx->blk = blk;
	pctx->blkcount = blockcnt;

	if ((blk < fs->super->s_first_data_block) ||
	    (blk >= ext2fs_blocks_count(fs->super))) {
		if (fix_problem(ctx, PR_1_BB_ILLEGAL_BLOCK_NUM, pctx)) {
			*block_nr = 0;
			return BLOCK_CHANGED;
		} else
			return 0;
	}

	if (blockcnt < 0) {
		if (ext2fs_test_block_bitmap2(p->fs_meta_blocks, blk)) {
			p->bbcheck = 1;
			if (fix_problem(ctx, PR_1_BB_FS_BLOCK, pctx)) {
				*block_nr = 0;
				return BLOCK_CHANGED;
			}
		} else if (ext2fs_test_block_bitmap2(ctx->block_found_map,
						    blk)) {
			p->bbcheck = 1;
			if (fix_problem(ctx, PR_1_BBINODE_BAD_METABLOCK,
					pctx)) {
				*block_nr = 0;
				return BLOCK_CHANGED;
			}
			if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
				return BLOCK_ABORT;
		} else
			mark_block_used(ctx, blk);
		return 0;
	}
#if 0
	printf ("DEBUG: Marking %u as bad.\n", blk);
#endif
	ctx->fs_badblocks_count++;
	/*
	 * If the block is not used, then mark it as used and return.
	 * If it is already marked as found, this must mean that
	 * there's an overlap between the filesystem table blocks
	 * (bitmaps and inode table) and the bad block list.
	 */
	if (!ext2fs_test_block_bitmap2(ctx->block_found_map, blk)) {
		ext2fs_mark_block_bitmap2(ctx->block_found_map, blk);
		return 0;
	}
	/*
	 * Try to find the where the filesystem block was used...
	 */
	first_block = fs->super->s_first_data_block;

	for (i = 0; i < fs->group_desc_count; i++ ) {
		pctx->group = i;
		pctx->blk = blk;
		if (!ext2fs_bg_has_super(fs, i))
			goto skip_super;
		if (blk == first_block) {
			if (i == 0) {
				if (fix_problem(ctx,
						PR_1_BAD_PRIMARY_SUPERBLOCK,
						pctx)) {
					*block_nr = 0;
					return BLOCK_CHANGED;
				}
				return 0;
			}
			fix_problem(ctx, PR_1_BAD_SUPERBLOCK, pctx);
			return 0;
		}
		if ((blk > first_block) &&
		    (blk <= first_block + fs->desc_blocks)) {
			if (i == 0) {
				pctx->blk = *block_nr;
				if (fix_problem(ctx,
			PR_1_BAD_PRIMARY_GROUP_DESCRIPTOR, pctx)) {
					*block_nr = 0;
					return BLOCK_CHANGED;
				}
				return 0;
			}
			fix_problem(ctx, PR_1_BAD_GROUP_DESCRIPTORS, pctx);
			return 0;
		}
	skip_super:
		if (blk == ext2fs_block_bitmap_loc(fs, i)) {
			if (fix_problem(ctx, PR_1_BB_BAD_BLOCK, pctx)) {
				ctx->invalid_block_bitmap_flag[i]++;
				ctx->invalid_bitmaps++;
			}
			return 0;
		}
		if (blk == ext2fs_inode_bitmap_loc(fs, i)) {
			if (fix_problem(ctx, PR_1_IB_BAD_BLOCK, pctx)) {
				ctx->invalid_inode_bitmap_flag[i]++;
				ctx->invalid_bitmaps++;
			}
			return 0;
		}
		if ((blk >= ext2fs_inode_table_loc(fs, i)) &&
		    (blk < (ext2fs_inode_table_loc(fs, i) +
			    fs->inode_blocks_per_group))) {
			/*
			 * If there are bad blocks in the inode table,
			 * the inode scan code will try to do
			 * something reasonable automatically.
			 */
			return 0;
		}
		first_block += fs->super->s_blocks_per_group;
	}
	/*
	 * If we've gotten to this point, then the only
	 * possibility is that the bad block inode meta data
	 * is using a bad block.
	 */
	if ((blk == p->inode->i_block[EXT2_IND_BLOCK]) ||
	    (blk == p->inode->i_block[EXT2_DIND_BLOCK]) ||
	    (blk == p->inode->i_block[EXT2_TIND_BLOCK])) {
		p->bbcheck = 1;
		if (fix_problem(ctx, PR_1_BBINODE_BAD_METABLOCK, pctx)) {
			*block_nr = 0;
			return BLOCK_CHANGED;
		}
		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			return BLOCK_ABORT;
		return 0;
	}

	pctx->group = -1;

	/* Warn user that the block wasn't claimed */
	fix_problem(ctx, PR_1_PROGERR_CLAIMED_BLOCK, pctx);

	return 0;
}

static void new_table_block(e2fsck_t ctx, blk64_t first_block, dgrp_t group,
			    const char *name, int num, blk64_t *new_block)
{
	ext2_filsys fs = ctx->fs;
	dgrp_t		last_grp;
	blk64_t		old_block = *new_block;
	blk64_t		last_block;
	dgrp_t		flexbg;
	unsigned	flexbg_size;
	int		i, is_flexbg;
	char		*buf;
	struct problem_context	pctx;

	clear_problem_context(&pctx);

	pctx.group = group;
	pctx.blk = old_block;
	pctx.str = name;

	/*
	 * For flex_bg filesystems, first try to allocate the metadata
	 * within the flex_bg, and if that fails then try finding the
	 * space anywhere in the filesystem.
	 */
	is_flexbg = ext2fs_has_feature_flex_bg(fs->super);
	if (is_flexbg) {
		flexbg_size = 1 << fs->super->s_log_groups_per_flex;
		flexbg = group / flexbg_size;
		first_block = ext2fs_group_first_block2(fs,
							flexbg_size * flexbg);
		last_grp = group | (flexbg_size - 1);
		if (last_grp >= fs->group_desc_count)
			last_grp = fs->group_desc_count - 1;
		last_block = ext2fs_group_last_block2(fs, last_grp);
	} else
		last_block = ext2fs_group_last_block2(fs, group);
	pctx.errcode = ext2fs_get_free_blocks2(fs, first_block, last_block,
					       num, ctx->block_found_map,
					       new_block);
	if (is_flexbg && (pctx.errcode == EXT2_ET_BLOCK_ALLOC_FAIL))
		pctx.errcode = ext2fs_get_free_blocks2(fs,
				fs->super->s_first_data_block,
				ext2fs_blocks_count(fs->super),
				num, ctx->block_found_map, new_block);
	if (pctx.errcode) {
		pctx.num = num;
		fix_problem(ctx, PR_1_RELOC_BLOCK_ALLOCATE, &pctx);
		ext2fs_unmark_valid(fs);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	pctx.errcode = ext2fs_get_mem(fs->blocksize, &buf);
	if (pctx.errcode) {
		fix_problem(ctx, PR_1_RELOC_MEMORY_ALLOCATE, &pctx);
		ext2fs_unmark_valid(fs);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	ext2fs_mark_super_dirty(fs);
	fs->flags &= ~EXT2_FLAG_MASTER_SB_ONLY;
	pctx.blk2 = *new_block;
	fix_problem(ctx, (old_block ? PR_1_RELOC_FROM_TO :
			  PR_1_RELOC_TO), &pctx);
	pctx.blk2 = 0;
	for (i = 0; i < num; i++) {
		pctx.blk = i;
		ext2fs_mark_block_bitmap2(ctx->block_found_map, (*new_block)+i);
		if (old_block) {
			pctx.errcode = io_channel_read_blk64(fs->io,
				   old_block + i, 1, buf);
			if (pctx.errcode)
				fix_problem(ctx, PR_1_RELOC_READ_ERR, &pctx);
			pctx.blk = (*new_block) + i;
			pctx.errcode = io_channel_write_blk64(fs->io, pctx.blk,
							      1, buf);
		} else {
			pctx.blk = (*new_block) + i;
			pctx.errcode = ext2fs_zero_blocks2(fs, pctx.blk, 1,
							   NULL, NULL);
		}

		if (pctx.errcode)
			fix_problem(ctx, PR_1_RELOC_WRITE_ERR, &pctx);
	}
	ext2fs_free_mem(&buf);
}

/*
 * This routine gets called at the end of pass 1 if bad blocks are
 * detected in the superblock, group descriptors, inode_bitmaps, or
 * block bitmaps.  At this point, all of the blocks have been mapped
 * out, so we can try to allocate new block(s) to replace the bad
 * blocks.
 */
static void handle_fs_bad_blocks(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	dgrp_t		i;
	blk64_t		first_block;
	blk64_t		new_blk;

	for (i = 0; i < fs->group_desc_count; i++) {
		first_block = ext2fs_group_first_block2(fs, i);

		if (ctx->invalid_block_bitmap_flag[i]) {
			new_blk = ext2fs_block_bitmap_loc(fs, i);
			new_table_block(ctx, first_block, i, _("block bitmap"),
					1, &new_blk);
			ext2fs_block_bitmap_loc_set(fs, i, new_blk);
		}
		if (ctx->invalid_inode_bitmap_flag[i]) {
			new_blk = ext2fs_inode_bitmap_loc(fs, i);
			new_table_block(ctx, first_block, i, _("inode bitmap"),
					1, &new_blk);
			ext2fs_inode_bitmap_loc_set(fs, i, new_blk);
		}
		if (ctx->invalid_inode_table_flag[i]) {
			new_blk = ext2fs_inode_table_loc(fs, i);
			new_table_block(ctx, first_block, i, _("inode table"),
					fs->inode_blocks_per_group,
					&new_blk);
			ext2fs_inode_table_loc_set(fs, i, new_blk);
			ctx->flags |= E2F_FLAG_RESTART;
		}
	}
	ctx->invalid_bitmaps = 0;
}

/*
 * This routine marks all blocks which are used by the superblock,
 * group descriptors, inode bitmaps, and block bitmaps.
 */
static void mark_table_blocks(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	blk64_t	b;
	dgrp_t	i;
	unsigned int	j;
	struct problem_context pctx;

	clear_problem_context(&pctx);

	for (i = 0; i < fs->group_desc_count; i++) {
		pctx.group = i;

		ext2fs_reserve_super_and_bgd(fs, i, ctx->block_found_map);
		ext2fs_reserve_super_and_bgd(fs, i, ctx->block_metadata_map);

		/*
		 * Mark the blocks used for the inode table
		 */
		if (ext2fs_inode_table_loc(fs, i)) {
			for (j = 0, b = ext2fs_inode_table_loc(fs, i);
			     j < fs->inode_blocks_per_group;
			     j++, b++) {
				if (ext2fs_test_block_bitmap2(ctx->block_found_map,
							     b)) {
					pctx.blk = b;
					if (!ctx->invalid_inode_table_flag[i] &&
					    fix_problem(ctx,
						PR_1_ITABLE_CONFLICT, &pctx)) {
						ctx->invalid_inode_table_flag[i]++;
						ctx->invalid_bitmaps++;
					}
				} else {
				    ext2fs_mark_block_bitmap2(
						ctx->block_found_map, b);
				    ext2fs_mark_block_bitmap2(
						ctx->block_metadata_map, b);
			    	}
			}
		}

		/*
		 * Mark block used for the block bitmap
		 */
		if (ext2fs_block_bitmap_loc(fs, i)) {
			if (ext2fs_test_block_bitmap2(ctx->block_found_map,
				     ext2fs_block_bitmap_loc(fs, i))) {
				pctx.blk = ext2fs_block_bitmap_loc(fs, i);
				if (fix_problem(ctx, PR_1_BB_CONFLICT, &pctx)) {
					ctx->invalid_block_bitmap_flag[i]++;
					ctx->invalid_bitmaps++;
				}
			} else {
			    ext2fs_mark_block_bitmap2(ctx->block_found_map,
				     ext2fs_block_bitmap_loc(fs, i));
			    ext2fs_mark_block_bitmap2(ctx->block_metadata_map,
				     ext2fs_block_bitmap_loc(fs, i));
			}
		}
		/*
		 * Mark block used for the inode bitmap
		 */
		if (ext2fs_inode_bitmap_loc(fs, i)) {
			if (ext2fs_test_block_bitmap2(ctx->block_found_map,
				     ext2fs_inode_bitmap_loc(fs, i))) {
				pctx.blk = ext2fs_inode_bitmap_loc(fs, i);
				if (fix_problem(ctx, PR_1_IB_CONFLICT, &pctx)) {
					ctx->invalid_inode_bitmap_flag[i]++;
					ctx->invalid_bitmaps++;
				}
			} else {
			    ext2fs_mark_block_bitmap2(ctx->block_metadata_map,
				     ext2fs_inode_bitmap_loc(fs, i));
			    ext2fs_mark_block_bitmap2(ctx->block_found_map,
				     ext2fs_inode_bitmap_loc(fs, i));
			}
		}
	}
}

/*
 * These subroutines short circuits ext2fs_get_blocks and
 * ext2fs_check_directory; we use them since we already have the inode
 * structure, so there's no point in letting the ext2fs library read
 * the inode again.
 */
static errcode_t pass1_get_blocks(ext2_filsys fs, ext2_ino_t ino,
				  blk_t *blocks)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;
	int	i;

	if ((ino != ctx->stashed_ino) || !ctx->stashed_inode)
		return EXT2_ET_CALLBACK_NOTHANDLED;

	for (i=0; i < EXT2_N_BLOCKS; i++)
		blocks[i] = ctx->stashed_inode->i_block[i];
	return 0;
}

static errcode_t pass1_read_inode(ext2_filsys fs, ext2_ino_t ino,
				  struct ext2_inode *inode)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;

	if ((ino != ctx->stashed_ino) || !ctx->stashed_inode)
		return EXT2_ET_CALLBACK_NOTHANDLED;
	*inode = *ctx->stashed_inode;
	return 0;
}

static errcode_t pass1_write_inode(ext2_filsys fs, ext2_ino_t ino,
			    struct ext2_inode *inode)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;

	if ((ino == ctx->stashed_ino) && ctx->stashed_inode &&
		(inode != ctx->stashed_inode))
		*ctx->stashed_inode = *inode;
	return EXT2_ET_CALLBACK_NOTHANDLED;
}

static errcode_t pass1_check_directory(ext2_filsys fs, ext2_ino_t ino)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;

	if ((ino != ctx->stashed_ino) || !ctx->stashed_inode)
		return EXT2_ET_CALLBACK_NOTHANDLED;

	if (!LINUX_S_ISDIR(ctx->stashed_inode->i_mode))
		return EXT2_ET_NO_DIRECTORY;
	return 0;
}

static errcode_t e2fsck_get_alloc_block(ext2_filsys fs, blk64_t goal,
					blk64_t *ret)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;
	errcode_t	retval;
	blk64_t		new_block;

	if (ctx->block_found_map) {
		retval = ext2fs_new_block2(fs, goal, ctx->block_found_map,
					   &new_block);
		if (retval)
			return retval;
		if (fs->block_map) {
			ext2fs_mark_block_bitmap2(fs->block_map, new_block);
			ext2fs_mark_bb_dirty(fs);
		}
	} else {
		if (!fs->block_map) {
			retval = ext2fs_read_block_bitmap(fs);
			if (retval)
				return retval;
		}

		retval = ext2fs_new_block2(fs, goal, fs->block_map, &new_block);
		if (retval)
			return retval;
	}

	*ret = new_block;
	return (0);
}

static errcode_t e2fsck_new_range(ext2_filsys fs, int flags, blk64_t goal,
				  blk64_t len, blk64_t *pblk, blk64_t *plen)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;
	errcode_t	retval;

	if (ctx->block_found_map)
		return ext2fs_new_range(fs, flags, goal, len,
					ctx->block_found_map, pblk, plen);

	if (!fs->block_map) {
		retval = ext2fs_read_block_bitmap(fs);
		if (retval)
			return retval;
	}

	return ext2fs_new_range(fs, flags, goal, len, fs->block_map,
				pblk, plen);
}

static void e2fsck_block_alloc_stats(ext2_filsys fs, blk64_t blk, int inuse)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;

	/* Never free a critical metadata block */
	if (ctx->block_found_map &&
	    ctx->block_metadata_map &&
	    inuse < 0 &&
	    ext2fs_test_block_bitmap2(ctx->block_metadata_map, blk))
		return;

	if (ctx->block_found_map) {
		if (inuse > 0)
			ext2fs_mark_block_bitmap2(ctx->block_found_map, blk);
		else
			ext2fs_unmark_block_bitmap2(ctx->block_found_map, blk);
	}
}

static void e2fsck_block_alloc_stats_range(ext2_filsys fs, blk64_t blk,
					   blk_t num, int inuse)
{
	e2fsck_t ctx = (e2fsck_t) fs->priv_data;

	/* Never free a critical metadata block */
	if (ctx->block_found_map &&
	    ctx->block_metadata_map &&
	    inuse < 0 &&
	    ext2fs_test_block_bitmap_range2(ctx->block_metadata_map, blk, num))
		return;

	if (ctx->block_found_map) {
		if (inuse > 0)
			ext2fs_mark_block_bitmap_range2(ctx->block_found_map,
							blk, num);
		else
			ext2fs_unmark_block_bitmap_range2(ctx->block_found_map,
							blk, num);
	}
}

void e2fsck_use_inode_shortcuts(e2fsck_t ctx, int use_shortcuts)
{
	ext2_filsys fs = ctx->fs;

	if (use_shortcuts) {
		fs->get_blocks = pass1_get_blocks;
		fs->check_directory = pass1_check_directory;
		fs->read_inode = pass1_read_inode;
		fs->write_inode = pass1_write_inode;
		ctx->stashed_ino = 0;
	} else {
		fs->get_blocks = 0;
		fs->check_directory = 0;
		fs->read_inode = 0;
		fs->write_inode = 0;
	}
}

void e2fsck_intercept_block_allocations(e2fsck_t ctx)
{
	ext2fs_set_alloc_block_callback(ctx->fs, e2fsck_get_alloc_block, 0);
	ext2fs_set_block_alloc_stats_callback(ctx->fs,
						e2fsck_block_alloc_stats, 0);
	ext2fs_set_new_range_callback(ctx->fs, e2fsck_new_range, NULL);
	ext2fs_set_block_alloc_stats_range_callback(ctx->fs,
					e2fsck_block_alloc_stats_range, NULL);
}
