/*
 * Copyright (c) 2003-2006, Cluster File Systems, Inc, info@clusterfs.com
 * Written by Alex Tomas <alex@clusterfs.com>
 *
 * Architecture independence:
 *   Copyright (c) 2005, Bull S.A.
 *   Written by Pierre Peiffer <pierre.peiffer@bull.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public Licens
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-
 */

/*
 * Extents support for EXT4
 *
 * TODO:
 *   - ext4*_error() should be used in some situations
 *   - analyze all BUG()/BUG_ON(), use -EIO where appropriate
 *   - smart tree reduction
 */

#include <linux/fs.h>
#include <linux/time.h>
#include <linux/jbd2.h>
#include <linux/highuid.h>
#include <linux/pagemap.h>
#include <linux/quotaops.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/fiemap.h>
#include "ext4_jbd2.h"
#include "ext4_extents.h"
#include "xattr.h"

#include <trace/events/ext4.h>

/*
 * used by extent splitting.
 */
#define EXT4_EXT_MAY_ZEROOUT	0x1  /* safe to zeroout if split fails \
					due to ENOSPC */
#define EXT4_EXT_MARK_UNWRIT1	0x2  /* mark first half unwritten */
#define EXT4_EXT_MARK_UNWRIT2	0x4  /* mark second half unwritten */

#define EXT4_EXT_DATA_VALID1	0x8  /* first half contains valid data */
#define EXT4_EXT_DATA_VALID2	0x10 /* second half contains valid data */

static __le32 ext4_extent_block_csum(struct inode *inode,
				     struct ext4_extent_header *eh)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	__u32 csum;

	csum = ext4_chksum(sbi, ei->i_csum_seed, (__u8 *)eh,
			   EXT4_EXTENT_TAIL_OFFSET(eh));
	return cpu_to_le32(csum);
}

static int ext4_extent_block_csum_verify(struct inode *inode,
					 struct ext4_extent_header *eh)
{
	struct ext4_extent_tail *et;

	if (!ext4_has_metadata_csum(inode->i_sb))
		return 1;

	et = find_ext4_extent_tail(eh);
	if (et->et_checksum != ext4_extent_block_csum(inode, eh))
		return 0;
	return 1;
}

static void ext4_extent_block_csum_set(struct inode *inode,
				       struct ext4_extent_header *eh)
{
	struct ext4_extent_tail *et;

	if (!ext4_has_metadata_csum(inode->i_sb))
		return;

	et = find_ext4_extent_tail(eh);
	et->et_checksum = ext4_extent_block_csum(inode, eh);
}

static int ext4_split_extent(handle_t *handle,
				struct inode *inode,
				struct ext4_ext_path **ppath,
				struct ext4_map_blocks *map,
				int split_flag,
				int flags);

static int ext4_split_extent_at(handle_t *handle,
			     struct inode *inode,
			     struct ext4_ext_path **ppath,
			     ext4_lblk_t split,
			     int split_flag,
			     int flags);

static int ext4_find_delayed_extent(struct inode *inode,
				    struct extent_status *newes);

static int ext4_ext_truncate_extend_restart(handle_t *handle,
					    struct inode *inode,
					    int needed)
{
	int err;

	if (!ext4_handle_valid(handle))
		return 0;
	if (handle->h_buffer_credits > needed)
		return 0;
	err = ext4_journal_extend(handle, needed);
	if (err <= 0)
		return err;
	err = ext4_truncate_restart_trans(handle, inode, needed);
	if (err == 0)
		err = -EAGAIN;

	return err;
}

/*
 * could return:
 *  - EROFS
 *  - ENOMEM
 */
static int ext4_ext_get_access(handle_t *handle, struct inode *inode,
				struct ext4_ext_path *path)
{
	if (path->p_bh) {
		/* path points to block */
		BUFFER_TRACE(path->p_bh, "get_write_access");
		return ext4_journal_get_write_access(handle, path->p_bh);
	}
	/* path points to leaf/index in inode body */
	/* we use in-core data, no need to protect them */
	return 0;
}

/*
 * could return:
 *  - EROFS
 *  - ENOMEM
 *  - EIO
 */
int __ext4_ext_dirty(const char *where, unsigned int line, handle_t *handle,
		     struct inode *inode, struct ext4_ext_path *path)
{
	int err;

	WARN_ON(!rwsem_is_locked(&EXT4_I(inode)->i_data_sem));
	if (path->p_bh) {
		ext4_extent_block_csum_set(inode, ext_block_hdr(path->p_bh));
		/* path points to block */
		err = __ext4_handle_dirty_metadata(where, line, handle,
						   inode, path->p_bh);
	} else {
		/* path points to leaf/index in inode body */
		err = ext4_mark_inode_dirty(handle, inode);
	}
	return err;
}

static ext4_fsblk_t ext4_ext_find_goal(struct inode *inode,
			      struct ext4_ext_path *path,
			      ext4_lblk_t block)
{
	if (path) {
		int depth = path->p_depth;
		struct ext4_extent *ex;

		/*
		 * Try to predict block placement assuming that we are
		 * filling in a file which will eventually be
		 * non-sparse --- i.e., in the case of libbfd writing
		 * an ELF object sections out-of-order but in a way
		 * the eventually results in a contiguous object or
		 * executable file, or some database extending a table
		 * space file.  However, this is actually somewhat
		 * non-ideal if we are writing a sparse file such as
		 * qemu or KVM writing a raw image file that is going
		 * to stay fairly sparse, since it will end up
		 * fragmenting the file system's free space.  Maybe we
		 * should have some hueristics or some way to allow
		 * userspace to pass a hint to file system,
		 * especially if the latter case turns out to be
		 * common.
		 */
		ex = path[depth].p_ext;
		if (ex) {
			ext4_fsblk_t ext_pblk = ext4_ext_pblock(ex);
			ext4_lblk_t ext_block = le32_to_cpu(ex->ee_block);

			if (block > ext_block)
				return ext_pblk + (block - ext_block);
			else
				return ext_pblk - (ext_block - block);
		}

		/* it looks like index is empty;
		 * try to find starting block from index itself */
		if (path[depth].p_bh)
			return path[depth].p_bh->b_blocknr;
	}

	/* OK. use inode's group */
	return ext4_inode_to_goal_block(inode);
}

/*
 * Allocation for a meta data block
 */
static ext4_fsblk_t
ext4_ext_new_meta_block(handle_t *handle, struct inode *inode,
			struct ext4_ext_path *path,
			struct ext4_extent *ex, int *err, unsigned int flags)
{
	ext4_fsblk_t goal, newblock;

	goal = ext4_ext_find_goal(inode, path, le32_to_cpu(ex->ee_block));
	newblock = ext4_new_meta_blocks(handle, inode, goal, flags,
					NULL, err);
	return newblock;
}

static inline int ext4_ext_space_block(struct inode *inode, int check)
{
	int size;

	size = (inode->i_sb->s_blocksize - sizeof(struct ext4_extent_header))
			/ sizeof(struct ext4_extent);
#ifdef AGGRESSIVE_TEST
	if (!check && size > 6)
		size = 6;
#endif
	return size;
}

static inline int ext4_ext_space_block_idx(struct inode *inode, int check)
{
	int size;

	size = (inode->i_sb->s_blocksize - sizeof(struct ext4_extent_header))
			/ sizeof(struct ext4_extent_idx);
#ifdef AGGRESSIVE_TEST
	if (!check && size > 5)
		size = 5;
#endif
	return size;
}

static inline int ext4_ext_space_root(struct inode *inode, int check)
{
	int size;

	size = sizeof(EXT4_I(inode)->i_data);
	size -= sizeof(struct ext4_extent_header);
	size /= sizeof(struct ext4_extent);
#ifdef AGGRESSIVE_TEST
	if (!check && size > 3)
		size = 3;
#endif
	return size;
}

static inline int ext4_ext_space_root_idx(struct inode *inode, int check)
{
	int size;

	size = sizeof(EXT4_I(inode)->i_data);
	size -= sizeof(struct ext4_extent_header);
	size /= sizeof(struct ext4_extent_idx);
#ifdef AGGRESSIVE_TEST
	if (!check && size > 4)
		size = 4;
#endif
	return size;
}

static inline int
ext4_force_split_extent_at(handle_t *handle, struct inode *inode,
			   struct ext4_ext_path **ppath, ext4_lblk_t lblk,
			   int nofail)
{
	struct ext4_ext_path *path = *ppath;
	int unwritten = ext4_ext_is_unwritten(path[path->p_depth].p_ext);

	return ext4_split_extent_at(handle, inode, ppath, lblk, unwritten ?
			EXT4_EXT_MARK_UNWRIT1|EXT4_EXT_MARK_UNWRIT2 : 0,
			EXT4_EX_NOCACHE | EXT4_GET_BLOCKS_PRE_IO |
			(nofail ? EXT4_GET_BLOCKS_METADATA_NOFAIL:0));
}

/*
 * Calculate the number of metadata blocks needed
 * to allocate @blocks
 * Worse case is one block per extent
 */
int ext4_ext_calc_metadata_amount(struct inode *inode, ext4_lblk_t lblock)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	int idxs;

	idxs = ((inode->i_sb->s_blocksize - sizeof(struct ext4_extent_header))
		/ sizeof(struct ext4_extent_idx));

	/*
	 * If the new delayed allocation block is contiguous with the
	 * previous da block, it can share index blocks with the
	 * previous block, so we only need to allocate a new index
	 * block every idxs leaf blocks.  At ldxs**2 blocks, we need
	 * an additional index block, and at ldxs**3 blocks, yet
	 * another index blocks.
	 */
	if (ei->i_da_metadata_calc_len &&
	    ei->i_da_metadata_calc_last_lblock+1 == lblock) {
		int num = 0;

		if ((ei->i_da_metadata_calc_len % idxs) == 0)
			num++;
		if ((ei->i_da_metadata_calc_len % (idxs*idxs)) == 0)
			num++;
		if ((ei->i_da_metadata_calc_len % (idxs*idxs*idxs)) == 0) {
			num++;
			ei->i_da_metadata_calc_len = 0;
		} else
			ei->i_da_metadata_calc_len++;
		ei->i_da_metadata_calc_last_lblock++;
		return num;
	}

	/*
	 * In the worst case we need a new set of index blocks at
	 * every level of the inode's extent tree.
	 */
	ei->i_da_metadata_calc_len = 1;
	ei->i_da_metadata_calc_last_lblock = lblock;
	return ext_depth(inode) + 1;
}

static int
ext4_ext_max_entries(struct inode *inode, int depth)
{
	int max;

	if (depth == ext_depth(inode)) {
		if (depth == 0)
			max = ext4_ext_space_root(inode, 1);
		else
			max = ext4_ext_space_root_idx(inode, 1);
	} else {
		if (depth == 0)
			max = ext4_ext_space_block(inode, 1);
		else
			max = ext4_ext_space_block_idx(inode, 1);
	}

	return max;
}

static int ext4_valid_extent(struct inode *inode, struct ext4_extent *ext)
{
	ext4_fsblk_t block = ext4_ext_pblock(ext);
	int len = ext4_ext_get_actual_len(ext);
	ext4_lblk_t lblock = le32_to_cpu(ext->ee_block);

	/*
	 * We allow neither:
	 *  - zero length
	 *  - overflow/wrap-around
	 */
	if (lblock + len <= lblock)
		return 0;
	return ext4_data_block_valid(EXT4_SB(inode->i_sb), block, len);
}

static int ext4_valid_extent_idx(struct inode *inode,
				struct ext4_extent_idx *ext_idx)
{
	ext4_fsblk_t block = ext4_idx_pblock(ext_idx);

	return ext4_data_block_valid(EXT4_SB(inode->i_sb), block, 1);
}

static int ext4_valid_extent_entries(struct inode *inode,
				struct ext4_extent_header *eh,
				int depth)
{
	unsigned short entries;
	if (eh->eh_entries == 0)
		return 1;

	entries = le16_to_cpu(eh->eh_entries);

	if (depth == 0) {
		/* leaf entries */
		struct ext4_extent *ext = EXT_FIRST_EXTENT(eh);
		struct ext4_super_block *es = EXT4_SB(inode->i_sb)->s_es;
		ext4_fsblk_t pblock = 0;
		ext4_lblk_t lblock = 0;
		ext4_lblk_t prev = 0;
		int len = 0;
		while (entries) {
			if (!ext4_valid_extent(inode, ext))
				return 0;

			/* Check for overlapping extents */
			lblock = le32_to_cpu(ext->ee_block);
			len = ext4_ext_get_actual_len(ext);
			if ((lblock <= prev) && prev) {
				pblock = ext4_ext_pblock(ext);
				es->s_last_error_block = cpu_to_le64(pblock);
				return 0;
			}
			ext++;
			entries--;
			prev = lblock + len - 1;
		}
	} else {
		struct ext4_extent_idx *ext_idx = EXT_FIRST_INDEX(eh);
		while (entries) {
			if (!ext4_valid_extent_idx(inode, ext_idx))
				return 0;
			ext_idx++;
			entries--;
		}
	}
	return 1;
}

static int __ext4_ext_check(const char *function, unsigned int line,
			    struct inode *inode, struct ext4_extent_header *eh,
			    int depth, ext4_fsblk_t pblk)
{
	const char *error_msg;
	int max = 0;

	if (unlikely(eh->eh_magic != EXT4_EXT_MAGIC)) {
		error_msg = "invalid magic";
		goto corrupted;
	}
	if (unlikely(le16_to_cpu(eh->eh_depth) != depth)) {
		error_msg = "unexpected eh_depth";
		goto corrupted;
	}
	if (unlikely(eh->eh_max == 0)) {
		error_msg = "invalid eh_max";
		goto corrupted;
	}
	max = ext4_ext_max_entries(inode, depth);
	if (unlikely(le16_to_cpu(eh->eh_max) > max)) {
		error_msg = "too large eh_max";
		goto corrupted;
	}
	if (unlikely(le16_to_cpu(eh->eh_entries) > le16_to_cpu(eh->eh_max))) {
		error_msg = "invalid eh_entries";
		goto corrupted;
	}
	if (!ext4_valid_extent_entries(inode, eh, depth)) {
		error_msg = "invalid extent entries";
		goto corrupted;
	}
	if (unlikely(depth > 32)) {
		error_msg = "too large eh_depth";
		goto corrupted;
	}
	/* Verify checksum on non-root extent tree nodes */
	if (ext_depth(inode) != depth &&
	    !ext4_extent_block_csum_verify(inode, eh)) {
		error_msg = "extent tree corrupted";
		goto corrupted;
	}
	return 0;

corrupted:
	ext4_error_inode(inode, function, line, 0,
			 "pblk %llu bad header/extent: %s - magic %x, "
			 "entries %u, max %u(%u), depth %u(%u)",
			 (unsigned long long) pblk, error_msg,
			 le16_to_cpu(eh->eh_magic),
			 le16_to_cpu(eh->eh_entries), le16_to_cpu(eh->eh_max),
			 max, le16_to_cpu(eh->eh_depth), depth);
	return -EIO;
}

#define ext4_ext_check(inode, eh, depth, pblk)			\
	__ext4_ext_check(__func__, __LINE__, (inode), (eh), (depth), (pblk))

int ext4_ext_check_inode(struct inode *inode)
{
	return ext4_ext_check(inode, ext_inode_hdr(inode), ext_depth(inode), 0);
}

static struct buffer_head *
__read_extent_tree_block(const char *function, unsigned int line,
			 struct inode *inode, ext4_fsblk_t pblk, int depth,
			 int flags)
{
	struct buffer_head		*bh;
	int				err;

	bh = sb_getblk_gfp(inode->i_sb, pblk, __GFP_MOVABLE | GFP_NOFS);
	if (unlikely(!bh))
		return ERR_PTR(-ENOMEM);

	if (!bh_uptodate_or_lock(bh)) {
		trace_ext4_ext_load_extent(inode, pblk, _RET_IP_);
		err = bh_submit_read(bh);
		if (err < 0)
			goto errout;
	}
	if (buffer_verified(bh) && !(flags & EXT4_EX_FORCE_CACHE))
		return bh;
	err = __ext4_ext_check(function, line, inode,
			       ext_block_hdr(bh), depth, pblk);
	if (err)
		goto errout;
	set_buffer_verified(bh);
	/*
	 * If this is a leaf block, cache all of its entries
	 */
	if (!(flags & EXT4_EX_NOCACHE) && depth == 0) {
		struct ext4_extent_header *eh = ext_block_hdr(bh);
		struct ext4_extent *ex = EXT_FIRST_EXTENT(eh);
		ext4_lblk_t prev = 0;
		int i;

		for (i = le16_to_cpu(eh->eh_entries); i > 0; i--, ex++) {
			unsigned int status = EXTENT_STATUS_WRITTEN;
			ext4_lblk_t lblk = le32_to_cpu(ex->ee_block);
			int len = ext4_ext_get_actual_len(ex);

			if (prev && (prev != lblk))
				ext4_es_cache_extent(inode, prev,
						     lblk - prev, ~0,
						     EXTENT_STATUS_HOLE);

			if (ext4_ext_is_unwritten(ex))
				status = EXTENT_STATUS_UNWRITTEN;
			ext4_es_cache_extent(inode, lblk, len,
					     ext4_ext_pblock(ex), status);
			prev = lblk + len;
		}
	}
	return bh;
errout:
	put_bh(bh);
	return ERR_PTR(err);

}

#define read_extent_tree_block(inode, pblk, depth, flags)		\
	__read_extent_tree_block(__func__, __LINE__, (inode), (pblk),   \
				 (depth), (flags))

/*
 * This function is called to cache a file's extent information in the
 * extent status tree
 */
int ext4_ext_precache(struct inode *inode)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	struct ext4_ext_path *path = NULL;
	struct buffer_head *bh;
	int i = 0, depth, ret = 0;

	if (!ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS))
		return 0;	/* not an extent-mapped inode */

	down_read(&ei->i_data_sem);
	depth = ext_depth(inode);

	path = kzalloc(sizeof(struct ext4_ext_path) * (depth + 1),
		       GFP_NOFS);
	if (path == NULL) {
		up_read(&ei->i_data_sem);
		return -ENOMEM;
	}

	/* Don't cache anything if there are no external extent blocks */
	if (depth == 0)
		goto out;
	path[0].p_hdr = ext_inode_hdr(inode);
	ret = ext4_ext_check(inode, path[0].p_hdr, depth, 0);
	if (ret)
		goto out;
	path[0].p_idx = EXT_FIRST_INDEX(path[0].p_hdr);
	while (i >= 0) {
		/*
		 * If this is a leaf block or we've reached the end of
		 * the index block, go up
		 */
		if ((i == depth) ||
		    path[i].p_idx > EXT_LAST_INDEX(path[i].p_hdr)) {
			brelse(path[i].p_bh);
			path[i].p_bh = NULL;
			i--;
			continue;
		}
		bh = read_extent_tree_block(inode,
					    ext4_idx_pblock(path[i].p_idx++),
					    depth - i - 1,
					    EXT4_EX_FORCE_CACHE);
		if (IS_ERR(bh)) {
			ret = PTR_ERR(bh);
			break;
		}
		i++;
		path[i].p_bh = bh;
		path[i].p_hdr = ext_block_hdr(bh);
		path[i].p_idx = EXT_FIRST_INDEX(path[i].p_hdr);
	}
	ext4_set_inode_state(inode, EXT4_STATE_EXT_PRECACHED);
out:
	up_read(&ei->i_data_sem);
	ext4_ext_drop_refs(path);
	kfree(path);
	return ret;
}

#ifdef EXT_DEBUG
static void ext4_ext_show_path(struct inode *inode, struct ext4_ext_path *path)
{
	int k, l = path->p_depth;

	ext_debug("path:");
	for (k = 0; k <= l; k++, path++) {
		if (path->p_idx) {
		  ext_debug("  %d->%llu", le32_to_cpu(path->p_idx->ei_block),
			    ext4_idx_pblock(path->p_idx));
		} else if (path->p_ext) {
			ext_debug("  %d:[%d]%d:%llu ",
				  le32_to_cpu(path->p_ext->ee_block),
				  ext4_ext_is_unwritten(path->p_ext),
				  ext4_ext_get_actual_len(path->p_ext),
				  ext4_ext_pblock(path->p_ext));
		} else
			ext_debug("  []");
	}
	ext_debug("\n");
}

static void ext4_ext_show_leaf(struct inode *inode, struct ext4_ext_path *path)
{
	int depth = ext_depth(inode);
	struct ext4_extent_header *eh;
	struct ext4_extent *ex;
	int i;

	if (!path)
		return;

	eh = path[depth].p_hdr;
	ex = EXT_FIRST_EXTENT(eh);

	ext_debug("Displaying leaf extents for inode %lu\n", inode->i_ino);

	for (i = 0; i < le16_to_cpu(eh->eh_entries); i++, ex++) {
		ext_debug("%d:[%d]%d:%llu ", le32_to_cpu(ex->ee_block),
			  ext4_ext_is_unwritten(ex),
			  ext4_ext_get_actual_len(ex), ext4_ext_pblock(ex));
	}
	ext_debug("\n");
}

static void ext4_ext_show_move(struct inode *inode, struct ext4_ext_path *path,
			ext4_fsblk_t newblock, int level)
{
	int depth = ext_depth(inode);
	struct ext4_extent *ex;

	if (depth != level) {
		struct ext4_extent_idx *idx;
		idx = path[level].p_idx;
		while (idx <= EXT_MAX_INDEX(path[level].p_hdr)) {
			ext_debug("%d: move %d:%llu in new index %llu\n", level,
					le32_to_cpu(idx->ei_block),
					ext4_idx_pblock(idx),
					newblock);
			idx++;
		}

		return;
	}

	ex = path[depth].p_ext;
	while (ex <= EXT_MAX_EXTENT(path[depth].p_hdr)) {
		ext_debug("move %d:%llu:[%d]%d in new leaf %llu\n",
				le32_to_cpu(ex->ee_block),
				ext4_ext_pblock(ex),
				ext4_ext_is_unwritten(ex),
				ext4_ext_get_actual_len(ex),
				newblock);
		ex++;
	}
}

#else
#define ext4_ext_show_path(inode, path)
#define ext4_ext_show_leaf(inode, path)
#define ext4_ext_show_move(inode, path, newblock, level)
#endif

void ext4_ext_drop_refs(struct ext4_ext_path *path)
{
	int depth, i;

	if (!path)
		return;
	depth = path->p_depth;
	for (i = 0; i <= depth; i++, path++)
		if (path->p_bh) {
			brelse(path->p_bh);
			path->p_bh = NULL;
		}
}

/*
 * ext4_ext_binsearch_idx:
 * binary search for the closest index of the given block
 * the header must be checked before calling this
 */
static void
ext4_ext_binsearch_idx(struct inode *inode,
			struct ext4_ext_path *path, ext4_lblk_t block)
{
	struct ext4_extent_header *eh = path->p_hdr;
	struct ext4_extent_idx *r, *l, *m;


	ext_debug("binsearch for %u(idx):  ", block);

	l = EXT_FIRST_INDEX(eh) + 1;
	r = EXT_LAST_INDEX(eh);
	while (l <= r) {
		m = l + (r - l) / 2;
		if (block < le32_to_cpu(m->ei_block))
			r = m - 1;
		else
			l = m + 1;
		ext_debug("%p(%u):%p(%u):%p(%u) ", l, le32_to_cpu(l->ei_block),
				m, le32_to_cpu(m->ei_block),
				r, le32_to_cpu(r->ei_block));
	}

	path->p_idx = l - 1;
	ext_debug("  -> %u->%lld ", le32_to_cpu(path->p_idx->ei_block),
		  ext4_idx_pblock(path->p_idx));

#ifdef CHECK_BINSEARCH
	{
		struct ext4_extent_idx *chix, *ix;
		int k;

		chix = ix = EXT_FIRST_INDEX(eh);
		for (k = 0; k < le16_to_cpu(eh->eh_entries); k++, ix++) {
		  if (k != 0 &&
		      le32_to_cpu(ix->ei_block) <= le32_to_cpu(ix[-1].ei_block)) {
				printk(KERN_DEBUG "k=%d, ix=0x%p, "
				       "first=0x%p\n", k,
				       ix, EXT_FIRST_INDEX(eh));
				printk(KERN_DEBUG "%u <= %u\n",
				       le32_to_cpu(ix->ei_block),
				       le32_to_cpu(ix[-1].ei_block));
			}
			BUG_ON(k && le32_to_cpu(ix->ei_block)
					   <= le32_to_cpu(ix[-1].ei_block));
			if (block < le32_to_cpu(ix->ei_block))
				break;
			chix = ix;
		}
		BUG_ON(chix != path->p_idx);
	}
#endif

}

/*
 * ext4_ext_binsearch:
 * binary search for closest extent of the given block
 * the header must be checked before calling this
 */
static void
ext4_ext_binsearch(struct inode *inode,
		struct ext4_ext_path *path, ext4_lblk_t block)
{
	struct ext4_extent_header *eh = path->p_hdr;
	struct ext4_extent *r, *l, *m;

	if (eh->eh_entries == 0) {
		/*
		 * this leaf is empty:
		 * we get such a leaf in split/add case
		 */
		return;
	}

	ext_debug("binsearch for %u:  ", block);

	l = EXT_FIRST_EXTENT(eh) + 1;
	r = EXT_LAST_EXTENT(eh);

	while (l <= r) {
		m = l + (r - l) / 2;
		if (block < le32_to_cpu(m->ee_block))
			r = m - 1;
		else
			l = m + 1;
		ext_debug("%p(%u):%p(%u):%p(%u) ", l, le32_to_cpu(l->ee_block),
				m, le32_to_cpu(m->ee_block),
				r, le32_to_cpu(r->ee_block));
	}

	path->p_ext = l - 1;
	ext_debug("  -> %d:%llu:[%d]%d ",
			le32_to_cpu(path->p_ext->ee_block),
			ext4_ext_pblock(path->p_ext),
			ext4_ext_is_unwritten(path->p_ext),
			ext4_ext_get_actual_len(path->p_ext));

#ifdef CHECK_BINSEARCH
	{
		struct ext4_extent *chex, *ex;
		int k;

		chex = ex = EXT_FIRST_EXTENT(eh);
		for (k = 0; k < le16_to_cpu(eh->eh_entries); k++, ex++) {
			BUG_ON(k && le32_to_cpu(ex->ee_block)
					  <= le32_to_cpu(ex[-1].ee_block));
			if (block < le32_to_cpu(ex->ee_block))
				break;
			chex = ex;
		}
		BUG_ON(chex != path->p_ext);
	}
#endif

}

int ext4_ext_tree_init(handle_t *handle, struct inode *inode)
{
	struct ext4_extent_header *eh;

	eh = ext_inode_hdr(inode);
	eh->eh_depth = 0;
	eh->eh_entries = 0;
	eh->eh_magic = EXT4_EXT_MAGIC;
	eh->eh_max = cpu_to_le16(ext4_ext_space_root(inode, 0));
	ext4_mark_inode_dirty(handle, inode);
	return 0;
}

struct ext4_ext_path *
ext4_find_extent(struct inode *inode, ext4_lblk_t block,
		 struct ext4_ext_path **orig_path, int flags)
{
	struct ext4_extent_header *eh;
	struct buffer_head *bh;
	struct ext4_ext_path *path = orig_path ? *orig_path : NULL;
	short int depth, i, ppos = 0;
	int ret;

	eh = ext_inode_hdr(inode);
	depth = ext_depth(inode);

	if (path) {
		ext4_ext_drop_refs(path);
		if (depth > path[0].p_maxdepth) {
			kfree(path);
			*orig_path = path = NULL;
		}
	}
	if (!path) {
		/* account possible depth increase */
		path = kzalloc(sizeof(struct ext4_ext_path) * (depth + 2),
				GFP_NOFS);
		if (unlikely(!path))
			return ERR_PTR(-ENOMEM);
		path[0].p_maxdepth = depth + 1;
	}
	path[0].p_hdr = eh;
	path[0].p_bh = NULL;

	i = depth;
	/* walk through the tree */
	while (i) {
		ext_debug("depth %d: num %d, max %d\n",
			  ppos, le16_to_cpu(eh->eh_entries), le16_to_cpu(eh->eh_max));

		ext4_ext_binsearch_idx(inode, path + ppos, block);
		path[ppos].p_block = ext4_idx_pblock(path[ppos].p_idx);
		path[ppos].p_depth = i;
		path[ppos].p_ext = NULL;

		bh = read_extent_tree_block(inode, path[ppos].p_block, --i,
					    flags);
		if (unlikely(IS_ERR(bh))) {
			ret = PTR_ERR(bh);
			goto err;
		}

		eh = ext_block_hdr(bh);
		ppos++;
		if (unlikely(ppos > depth)) {
			put_bh(bh);
			EXT4_ERROR_INODE(inode,
					 "ppos %d > depth %d", ppos, depth);
			ret = -EIO;
			goto err;
		}
		path[ppos].p_bh = bh;
		path[ppos].p_hdr = eh;
	}

	path[ppos].p_depth = i;
	path[ppos].p_ext = NULL;
	path[ppos].p_idx = NULL;

	/* find extent */
	ext4_ext_binsearch(inode, path + ppos, block);
	/* if not an empty leaf */
	if (path[ppos].p_ext)
		path[ppos].p_block = ext4_ext_pblock(path[ppos].p_ext);

	ext4_ext_show_path(inode, path);

	return path;

err:
	ext4_ext_drop_refs(path);
	kfree(path);
	if (orig_path)
		*orig_path = NULL;
	return ERR_PTR(ret);
}

/*
 * ext4_ext_insert_index:
 * insert new index [@logical;@ptr] into the block at @curp;
 * check where to insert: before @curp or after @curp
 */
static int ext4_ext_insert_index(handle_t *handle, struct inode *inode,
				 struct ext4_ext_path *curp,
				 int logical, ext4_fsblk_t ptr)
{
	struct ext4_extent_idx *ix;
	int len, err;

	err = ext4_ext_get_access(handle, inode, curp);
	if (err)
		return err;

	if (unlikely(logical == le32_to_cpu(curp->p_idx->ei_block))) {
		EXT4_ERROR_INODE(inode,
				 "logical %d == ei_block %d!",
				 logical, le32_to_cpu(curp->p_idx->ei_block));
		return -EIO;
	}

	if (unlikely(le16_to_cpu(curp->p_hdr->eh_entries)
			     >= le16_to_cpu(curp->p_hdr->eh_max))) {
		EXT4_ERROR_INODE(inode,
				 "eh_entries %d >= eh_max %d!",
				 le16_to_cpu(curp->p_hdr->eh_entries),
				 le16_to_cpu(curp->p_hdr->eh_max));
		return -EIO;
	}

	if (logical > le32_to_cpu(curp->p_idx->ei_block)) {
		/* insert after */
		ext_debug("insert new index %d after: %llu\n", logical, ptr);
		ix = curp->p_idx + 1;
	} else {
		/* insert before */
		ext_debug("insert new index %d before: %llu\n", logical, ptr);
		ix = curp->p_idx;
	}

	len = EXT_LAST_INDEX(curp->p_hdr) - ix + 1;
	BUG_ON(len < 0);
	if (len > 0) {
		ext_debug("insert new index %d: "
				"move %d indices from 0x%p to 0x%p\n",
				logical, len, ix, ix + 1);
		memmove(ix + 1, ix, len * sizeof(struct ext4_extent_idx));
	}

	if (unlikely(ix > EXT_MAX_INDEX(curp->p_hdr))) {
		EXT4_ERROR_INODE(inode, "ix > EXT_MAX_INDEX!");
		return -EIO;
	}

	ix->ei_block = cpu_to_le32(logical);
	ext4_idx_store_pblock(ix, ptr);
	le16_add_cpu(&curp->p_hdr->eh_entries, 1);

	if (unlikely(ix > EXT_LAST_INDEX(curp->p_hdr))) {
		EXT4_ERROR_INODE(inode, "ix > EXT_LAST_INDEX!");
		return -EIO;
	}

	err = ext4_ext_dirty(handle, inode, curp);
	ext4_std_error(inode->i_sb, err);

	return err;
}

/*
 * ext4_ext_split:
 * inserts new subtree into the path, using free index entry
 * at depth @at:
 * - allocates all needed blocks (new leaf and all intermediate index blocks)
 * - makes decision where to split
 * - moves remaining extents and index entries (right to the split point)
 *   into the newly allocated blocks
 * - initializes subtree
 */
static int ext4_ext_split(handle_t *handle, struct inode *inode,
			  unsigned int flags,
			  struct ext4_ext_path *path,
			  struct ext4_extent *newext, int at)
{
	struct buffer_head *bh = NULL;
	int depth = ext_depth(inode);
	struct ext4_extent_header *neh;
	struct ext4_extent_idx *fidx;
	int i = at, k, m, a;
	ext4_fsblk_t newblock, oldblock;
	__le32 border;
	ext4_fsblk_t *ablocks = NULL; /* array of allocated blocks */
	int err = 0;

	/* make decision: where to split? */
	/* FIXME: now decision is simplest: at current extent */

	/* if current leaf will be split, then we should use
	 * border from split point */
	if (unlikely(path[depth].p_ext > EXT_MAX_EXTENT(path[depth].p_hdr))) {
		EXT4_ERROR_INODE(inode, "p_ext > EXT_MAX_EXTENT!");
		return -EIO;
	}
	if (path[depth].p_ext != EXT_MAX_EXTENT(path[depth].p_hdr)) {
		border = path[depth].p_ext[1].ee_block;
		ext_debug("leaf will be split."
				" next leaf starts at %d\n",
				  le32_to_cpu(border));
	} else {
		border = newext->ee_block;
		ext_debug("leaf will be added."
				" next leaf starts at %d\n",
				le32_to_cpu(border));
	}

	/*
	 * If error occurs, then we break processing
	 * and mark filesystem read-only. index won't
	 * be inserted and tree will be in consistent
	 * state. Next mount will repair buffers too.
	 */

	/*
	 * Get array to track all allocated blocks.
	 * We need this to handle errors and free blocks
	 * upon them.
	 */
	ablocks = kzalloc(sizeof(ext4_fsblk_t) * depth, GFP_NOFS);
	if (!ablocks)
		return -ENOMEM;

	/* allocate all needed blocks */
	ext_debug("allocate %d blocks for indexes/leaf\n", depth - at);
	for (a = 0; a < depth - at; a++) {
		newblock = ext4_ext_new_meta_block(handle, inode, path,
						   newext, &err, flags);
		if (newblock == 0)
			goto cleanup;
		ablocks[a] = newblock;
	}

	/* initialize new leaf */
	newblock = ablocks[--a];
	if (unlikely(newblock == 0)) {
		EXT4_ERROR_INODE(inode, "newblock == 0!");
		err = -EIO;
		goto cleanup;
	}
	bh = sb_getblk_gfp(inode->i_sb, newblock, __GFP_MOVABLE | GFP_NOFS);
	if (unlikely(!bh)) {
		err = -ENOMEM;
		goto cleanup;
	}
	lock_buffer(bh);

	err = ext4_journal_get_create_access(handle, bh);
	if (err)
		goto cleanup;

	neh = ext_block_hdr(bh);
	neh->eh_entries = 0;
	neh->eh_max = cpu_to_le16(ext4_ext_space_block(inode, 0));
	neh->eh_magic = EXT4_EXT_MAGIC;
	neh->eh_depth = 0;

	/* move remainder of path[depth] to the new leaf */
	if (unlikely(path[depth].p_hdr->eh_entries !=
		     path[depth].p_hdr->eh_max)) {
		EXT4_ERROR_INODE(inode, "eh_entries %d != eh_max %d!",
				 path[depth].p_hdr->eh_entries,
				 path[depth].p_hdr->eh_max);
		err = -EIO;
		goto cleanup;
	}
	/* start copy from next extent */
	m = EXT_MAX_EXTENT(path[depth].p_hdr) - path[depth].p_ext++;
	ext4_ext_show_move(inode, path, newblock, depth);
	if (m) {
		struct ext4_extent *ex;
		ex = EXT_FIRST_EXTENT(neh);
		memmove(ex, path[depth].p_ext, sizeof(struct ext4_extent) * m);
		le16_add_cpu(&neh->eh_entries, m);
	}

	ext4_extent_block_csum_set(inode, neh);
	set_buffer_uptodate(bh);
	unlock_buffer(bh);

	err = ext4_handle_dirty_metadata(handle, inode, bh);
	if (err)
		goto cleanup;
	brelse(bh);
	bh = NULL;

	/* correct old leaf */
	if (m) {
		err = ext4_ext_get_access(handle, inode, path + depth);
		if (err)
			goto cleanup;
		le16_add_cpu(&path[depth].p_hdr->eh_entries, -m);
		err = ext4_ext_dirty(handle, inode, path + depth);
		if (err)
			goto cleanup;

	}

	/* create intermediate indexes */
	k = depth - at - 1;
	if (unlikely(k < 0)) {
		EXT4_ERROR_INODE(inode, "k %d < 0!", k);
		err = -EIO;
		goto cleanup;
	}
	if (k)
		ext_debug("create %d intermediate indices\n", k);
	/* insert new index into current index block */
	/* current depth stored in i var */
	i = depth - 1;
	while (k--) {
		oldblock = newblock;
		newblock = ablocks[--a];
		bh = sb_getblk(inode->i_sb, newblock);
		if (unlikely(!bh)) {
			err = -ENOMEM;
			goto cleanup;
		}
		lock_buffer(bh);

		err = ext4_journal_get_create_access(handle, bh);
		if (err)
			goto cleanup;

		neh = ext_block_hdr(bh);
		neh->eh_entries = cpu_to_le16(1);
		neh->eh_magic = EXT4_EXT_MAGIC;
		neh->eh_max = cpu_to_le16(ext4_ext_space_block_idx(inode, 0));
		neh->eh_depth = cpu_to_le16(depth - i);
		fidx = EXT_FIRST_INDEX(neh);
		fidx->ei_block = border;
		ext4_idx_store_pblock(fidx, oldblock);

		ext_debug("int.index at %d (block %llu): %u -> %llu\n",
				i, newblock, le32_to_cpu(border), oldblock);

		/* move remainder of path[i] to the new index block */
		if (unlikely(EXT_MAX_INDEX(path[i].p_hdr) !=
					EXT_LAST_INDEX(path[i].p_hdr))) {
			EXT4_ERROR_INODE(inode,
					 "EXT_MAX_INDEX != EXT_LAST_INDEX ee_block %d!",
					 le32_to_cpu(path[i].p_ext->ee_block));
			err = -EIO;
			goto cleanup;
		}
		/* start copy indexes */
		m = EXT_MAX_INDEX(path[i].p_hdr) - path[i].p_idx++;
		ext_debug("cur 0x%p, last 0x%p\n", path[i].p_idx,
				EXT_MAX_INDEX(path[i].p_hdr));
		ext4_ext_show_move(inode, path, newblock, i);
		if (m) {
			memmove(++fidx, path[i].p_idx,
				sizeof(struct ext4_extent_idx) * m);
			le16_add_cpu(&neh->eh_entries, m);
		}
		ext4_extent_block_csum_set(inode, neh);
		set_buffer_uptodate(bh);
		unlock_buffer(bh);

		err = ext4_handle_dirty_metadata(handle, inode, bh);
		if (err)
			goto cleanup;
		brelse(bh);
		bh = NULL;

		/* correct old index */
		if (m) {
			err = ext4_ext_get_access(handle, inode, path + i);
			if (err)
				goto cleanup;
			le16_add_cpu(&path[i].p_hdr->eh_entries, -m);
			err = ext4_ext_dirty(handle, inode, path + i);
			if (err)
				goto cleanup;
		}

		i--;
	}

	/* insert new index */
	err = ext4_ext_insert_index(handle, inode, path + at,
				    le32_to_cpu(border), newblock);

cleanup:
	if (bh) {
		if (buffer_locked(bh))
			unlock_buffer(bh);
		brelse(bh);
	}

	if (err) {
		/* free all allocated blocks in error case */
		for (i = 0; i < depth; i++) {
			if (!ablocks[i])
				continue;
			ext4_free_blocks(handle, inode, NULL, ablocks[i], 1,
					 EXT4_FREE_BLOCKS_METADATA);
		}
	}
	kfree(ablocks);

	return err;
}

/*
 * ext4_ext_grow_indepth:
 * implements tree growing procedure:
 * - allocates new block
 * - moves top-level data (index block or leaf) into the new block
 * - initializes new top-level, creating index that points to the
 *   just created block
 */
static int ext4_ext_grow_indepth(handle_t *handle, struct inode *inode,
				 unsigned int flags)
{
	struct ext4_extent_header *neh;
	struct buffer_head *bh;
	ext4_fsblk_t newblock, goal = 0;
	struct ext4_super_block *es = EXT4_SB(inode->i_sb)->s_es;
	int err = 0;

	/* Try to prepend new index to old one */
	if (ext_depth(inode))
		goal = ext4_idx_pblock(EXT_FIRST_INDEX(ext_inode_hdr(inode)));
	if (goal > le32_to_cpu(es->s_first_data_block)) {
		flags |= EXT4_MB_HINT_TRY_GOAL;
		goal--;
	} else
		goal = ext4_inode_to_goal_block(inode);
	newblock = ext4_new_meta_blocks(handle, inode, goal, flags,
					NULL, &err);
	if (newblock == 0)
		return err;

	bh = sb_getblk_gfp(inode->i_sb, newblock, __GFP_MOVABLE | GFP_NOFS);
	if (unlikely(!bh))
		return -ENOMEM;
	lock_buffer(bh);

	err = ext4_journal_get_create_access(handle, bh);
	if (err) {
		unlock_buffer(bh);
		goto out;
	}

	/* move top-level index/leaf into new block */
	memmove(bh->b_data, EXT4_I(inode)->i_data,
		sizeof(EXT4_I(inode)->i_data));

	/* set size of new block */
	neh = ext_block_hdr(bh);
	/* old root could have indexes or leaves
	 * so calculate e_max right way */
	if (ext_depth(inode))
		neh->eh_max = cpu_to_le16(ext4_ext_space_block_idx(inode, 0));
	else
		neh->eh_max = cpu_to_le16(ext4_ext_space_block(inode, 0));
	neh->eh_magic = EXT4_EXT_MAGIC;
	ext4_extent_block_csum_set(inode, neh);
	set_buffer_uptodate(bh);
	unlock_buffer(bh);

	err = ext4_handle_dirty_metadata(handle, inode, bh);
	if (err)
		goto out;

	/* Update top-level index: num,max,pointer */
	neh = ext_inode_hdr(inode);
	neh->eh_entries = cpu_to_le16(1);
	ext4_idx_store_pblock(EXT_FIRST_INDEX(neh), newblock);
	if (neh->eh_depth == 0) {
		/* Root extent block becomes index block */
		neh->eh_max = cpu_to_le16(ext4_ext_space_root_idx(inode, 0));
		EXT_FIRST_INDEX(neh)->ei_block =
			EXT_FIRST_EXTENT(neh)->ee_block;
	}
	ext_debug("new root: num %d(%d), lblock %d, ptr %llu\n",
		  le16_to_cpu(neh->eh_entries), le16_to_cpu(neh->eh_max),
		  le32_to_cpu(EXT_FIRST_INDEX(neh)->ei_block),
		  ext4_idx_pblock(EXT_FIRST_INDEX(neh)));

	le16_add_cpu(&neh->eh_depth, 1);
	ext4_mark_inode_dirty(handle, inode);
out:
	brelse(bh);

	return err;
}

/*
 * ext4_ext_create_new_leaf:
 * finds empty index and adds new leaf.
 * if no free index is found, then it requests in-depth growing.
 */
static int ext4_ext_create_new_leaf(handle_t *handle, struct inode *inode,
				    unsigned int mb_flags,
				    unsigned int gb_flags,
				    struct ext4_ext_path **ppath,
				    struct ext4_extent *newext)
{
	struct ext4_ext_path *path = *ppath;
	struct ext4_ext_path *curp;
	int depth, i, err = 0;

repeat:
	i = depth = ext_depth(inode);

	/* walk up to the tree and look for free index entry */
	curp = path + depth;
	while (i > 0 && !EXT_HAS_FREE_INDEX(curp)) {
		i--;
		curp--;
	}

	/* we use already allocated block for index block,
	 * so subsequent data blocks should be contiguous */
	if (EXT_HAS_FREE_INDEX(curp)) {
		/* if we found index with free entry, then use that
		 * entry: create all needed subtree and add new leaf */
		err = ext4_ext_split(handle, inode, mb_flags, path, newext, i);
		if (err)
			goto out;

		/* refill path */
		path = ext4_find_extent(inode,
				    (ext4_lblk_t)le32_to_cpu(newext->ee_block),
				    ppath, gb_flags);
		if (IS_ERR(path))
			err = PTR_ERR(path);
	} else {
		/* tree is full, time to grow in depth */
		err = ext4_ext_grow_indepth(handle, inode, mb_flags);
		if (err)
			goto out;

		/* refill path */
		path = ext4_find_extent(inode,
				   (ext4_lblk_t)le32_to_cpu(newext->ee_block),
				    ppath, gb_flags);
		if (IS_ERR(path)) {
			err = PTR_ERR(path);
			goto out;
		}

		/*
		 * only first (depth 0 -> 1) produces free space;
		 * in all other cases we have to split the grown tree
		 */
		depth = ext_depth(inode);
		if (path[depth].p_hdr->eh_entries == path[depth].p_hdr->eh_max) {
			/* now we need to split */
			goto repeat;
		}
	}

out:
	return err;
}

/*
 * search the closest allocated block to the left for *logical
 * and returns it at @logical + it's physical address at @phys
 * if *logical is the smallest allocated block, the function
 * returns 0 at @phys
 * return value contains 0 (success) or error code
 */
static int ext4_ext_search_left(struct inode *inode,
				struct ext4_ext_path *path,
				ext4_lblk_t *logical, ext4_fsblk_t *phys)
{
	struct ext4_extent_idx *ix;
	struct ext4_extent *ex;
	int depth, ee_len;

	if (unlikely(path == NULL)) {
		EXT4_ERROR_INODE(inode, "path == NULL *logical %d!", *logical);
		return -EIO;
	}
	depth = path->p_depth;
	*phys = 0;

	if (depth == 0 && path->p_ext == NULL)
		return 0;

	/* usually extent in the path covers blocks smaller
	 * then *logical, but it can be that extent is the
	 * first one in the file */

	ex = path[depth].p_ext;
	ee_len = ext4_ext_get_actual_len(ex);
	if (*logical < le32_to_cpu(ex->ee_block)) {
		if (unlikely(EXT_FIRST_EXTENT(path[depth].p_hdr) != ex)) {
			EXT4_ERROR_INODE(inode,
					 "EXT_FIRST_EXTENT != ex *logical %d ee_block %d!",
					 *logical, le32_to_cpu(ex->ee_block));
			return -EIO;
		}
		while (--depth >= 0) {
			ix = path[depth].p_idx;
			if (unlikely(ix != EXT_FIRST_INDEX(path[depth].p_hdr))) {
				EXT4_ERROR_INODE(inode,
				  "ix (%d) != EXT_FIRST_INDEX (%d) (depth %d)!",
				  ix != NULL ? le32_to_cpu(ix->ei_block) : 0,
				  EXT_FIRST_INDEX(path[depth].p_hdr) != NULL ?
		le32_to_cpu(EXT_FIRST_INDEX(path[depth].p_hdr)->ei_block) : 0,
				  depth);
				return -EIO;
			}
		}
		return 0;
	}

	if (unlikely(*logical < (le32_to_cpu(ex->ee_block) + ee_len))) {
		EXT4_ERROR_INODE(inode,
				 "logical %d < ee_block %d + ee_len %d!",
				 *logical, le32_to_cpu(ex->ee_block), ee_len);
		return -EIO;
	}

	*logical = le32_to_cpu(ex->ee_block) + ee_len - 1;
	*phys = ext4_ext_pblock(ex) + ee_len - 1;
	return 0;
}

/*
 * search the closest allocated block to the right for *logical
 * and returns it at @logical + it's physical address at @phys
 * if *logical is the largest allocated block, the function
 * returns 0 at @phys
 * return value contains 0 (success) or error code
 */
static int ext4_ext_search_right(struct inode *inode,
				 struct ext4_ext_path *path,
				 ext4_lblk_t *logical, ext4_fsblk_t *phys,
				 struct ext4_extent **ret_ex)
{
	struct buffer_head *bh = NULL;
	struct ext4_extent_header *eh;
	struct ext4_extent_idx *ix;
	struct ext4_extent *ex;
	ext4_fsblk_t block;
	int depth;	/* Note, NOT eh_depth; depth from top of tree */
	int ee_len;

	if (unlikely(path == NULL)) {
		EXT4_ERROR_INODE(inode, "path == NULL *logical %d!", *logical);
		return -EIO;
	}
	depth = path->p_depth;
	*phys = 0;

	if (depth == 0 && path->p_ext == NULL)
		return 0;

	/* usually extent in the path covers blocks smaller
	 * then *logical, but it can be that extent is the
	 * first one in the file */

	ex = path[depth].p_ext;
	ee_len = ext4_ext_get_actual_len(ex);
	if (*logical < le32_to_cpu(ex->ee_block)) {
		if (unlikely(EXT_FIRST_EXTENT(path[depth].p_hdr) != ex)) {
			EXT4_ERROR_INODE(inode,
					 "first_extent(path[%d].p_hdr) != ex",
					 depth);
			return -EIO;
		}
		while (--depth >= 0) {
			ix = path[depth].p_idx;
			if (unlikely(ix != EXT_FIRST_INDEX(path[depth].p_hdr))) {
				EXT4_ERROR_INODE(inode,
						 "ix != EXT_FIRST_INDEX *logical %d!",
						 *logical);
				return -EIO;
			}
		}
		goto found_extent;
	}

	if (unlikely(*logical < (le32_to_cpu(ex->ee_block) + ee_len))) {
		EXT4_ERROR_INODE(inode,
				 "logical %d < ee_block %d + ee_len %d!",
				 *logical, le32_to_cpu(ex->ee_block), ee_len);
		return -EIO;
	}

	if (ex != EXT_LAST_EXTENT(path[depth].p_hdr)) {
		/* next allocated block in this leaf */
		ex++;
		goto found_extent;
	}

	/* go up and search for index to the right */
	while (--depth >= 0) {
		ix = path[depth].p_idx;
		if (ix != EXT_LAST_INDEX(path[depth].p_hdr))
			goto got_index;
	}

	/* we've gone up to the root and found no index to the right */
	return 0;

got_index:
	/* we've found index to the right, let's
	 * follow it and find the closest allocated
	 * block to the right */
	ix++;
	block = ext4_idx_pblock(ix);
	while (++depth < path->p_depth) {
		/* subtract from p_depth to get proper eh_depth */
		bh = read_extent_tree_block(inode, block,
					    path->p_depth - depth, 0);
		if (IS_ERR(bh))
			return PTR_ERR(bh);
		eh = ext_block_hdr(bh);
		ix = EXT_FIRST_INDEX(eh);
		block = ext4_idx_pblock(ix);
		put_bh(bh);
	}

	bh = read_extent_tree_block(inode, block, path->p_depth - depth, 0);
	if (IS_ERR(bh))
		return PTR_ERR(bh);
	eh = ext_block_hdr(bh);
	ex = EXT_FIRST_EXTENT(eh);
found_extent:
	*logical = le32_to_cpu(ex->ee_block);
	*phys = ext4_ext_pblock(ex);
	*ret_ex = ex;
	if (bh)
		put_bh(bh);
	return 0;
}

/*
 * ext4_ext_next_allocated_block:
 * returns allocated block in subsequent extent or EXT_MAX_BLOCKS.
 * NOTE: it considers block number from index entry as
 * allocated block. Thus, index entries have to be consistent
 * with leaves.
 */
ext4_lblk_t
ext4_ext_next_allocated_block(struct ext4_ext_path *path)
{
	int depth;

	BUG_ON(path == NULL);
	depth = path->p_depth;

	if (depth == 0 && path->p_ext == NULL)
		return EXT_MAX_BLOCKS;

	while (depth >= 0) {
		if (depth == path->p_depth) {
			/* leaf */
			if (path[depth].p_ext &&
				path[depth].p_ext !=
					EXT_LAST_EXTENT(path[depth].p_hdr))
			  return le32_to_cpu(path[depth].p_ext[1].ee_block);
		} else {
			/* index */
			if (path[depth].p_idx !=
					EXT_LAST_INDEX(path[depth].p_hdr))
			  return le32_to_cpu(path[depth].p_idx[1].ei_block);
		}
		depth--;
	}

	return EXT_MAX_BLOCKS;
}

/*
 * ext4_ext_next_leaf_block:
 * returns first allocated block from next leaf or EXT_MAX_BLOCKS
 */
static ext4_lblk_t ext4_ext_next_leaf_block(struct ext4_ext_path *path)
{
	int depth;

	BUG_ON(path == NULL);
	depth = path->p_depth;

	/* zero-tree has no leaf blocks at all */
	if (depth == 0)
		return EXT_MAX_BLOCKS;

	/* go to index block */
	depth--;

	while (depth >= 0) {
		if (path[depth].p_idx !=
				EXT_LAST_INDEX(path[depth].p_hdr))
			return (ext4_lblk_t)
				le32_to_cpu(path[depth].p_idx[1].ei_block);
		depth--;
	}

	return EXT_MAX_BLOCKS;
}

/*
 * ext4_ext_correct_indexes:
 * if leaf gets modified and modified extent is first in the leaf,
 * then we have to correct all indexes above.
 * TODO: do we need to correct tree in all cases?
 */
static int ext4_ext_correct_indexes(handle_t *handle, struct inode *inode,
				struct ext4_ext_path *path)
{
	struct ext4_extent_header *eh;
	int depth = ext_depth(inode);
	struct ext4_extent *ex;
	__le32 border;
	int k, err = 0;

	eh = path[depth].p_hdr;
	ex = path[depth].p_ext;

	if (unlikely(ex == NULL || eh == NULL)) {
		EXT4_ERROR_INODE(inode,
				 "ex %p == NULL or eh %p == NULL", ex, eh);
		return -EIO;
	}

	if (depth == 0) {
		/* there is no tree at all */
		return 0;
	}

	if (ex != EXT_FIRST_EXTENT(eh)) {
		/* we correct tree if first leaf got modified only */
		return 0;
	}

	/*
	 * TODO: we need correction if border is smaller than current one
	 */
	k = depth - 1;
	border = path[depth].p_ext->ee_block;
	err = ext4_ext_get_access(handle, inode, path + k);
	if (err)
		return err;
	path[k].p_idx->ei_block = border;
	err = ext4_ext_dirty(handle, inode, path + k);
	if (err)
		return err;

	while (k--) {
		/* change all left-side indexes */
		if (path[k+1].p_idx != EXT_FIRST_INDEX(path[k+1].p_hdr))
			break;
		err = ext4_ext_get_access(handle, inode, path + k);
		if (err)
			break;
		path[k].p_idx->ei_block = border;
		err = ext4_ext_dirty(handle, inode, path + k);
		if (err)
			break;
	}

	return err;
}

int
ext4_can_extents_be_merged(struct inode *inode, struct ext4_extent *ex1,
				struct ext4_extent *ex2)
{
	unsigned short ext1_ee_len, ext2_ee_len;

	if (ext4_ext_is_unwritten(ex1) != ext4_ext_is_unwritten(ex2))
		return 0;

	ext1_ee_len = ext4_ext_get_actual_len(ex1);
	ext2_ee_len = ext4_ext_get_actual_len(ex2);

	if (le32_to_cpu(ex1->ee_block) + ext1_ee_len !=
			le32_to_cpu(ex2->ee_block))
		return 0;

	/*
	 * To allow future support for preallocated extents to be added
	 * as an RO_COMPAT feature, refuse to merge to extents if
	 * this can result in the top bit of ee_len being set.
	 */
	if (ext1_ee_len + ext2_ee_len > EXT_INIT_MAX_LEN)
		return 0;
	if (ext4_ext_is_unwritten(ex1) &&
	    (ext4_test_inode_state(inode, EXT4_STATE_DIO_UNWRITTEN) ||
	     atomic_read(&EXT4_I(inode)->i_unwritten) ||
	     (ext1_ee_len + ext2_ee_len > EXT_UNWRITTEN_MAX_LEN)))
		return 0;
#ifdef AGGRESSIVE_TEST
	if (ext1_ee_len >= 4)
		return 0;
#endif

	if (ext4_ext_pblock(ex1) + ext1_ee_len == ext4_ext_pblock(ex2))
		return 1;
	return 0;
}

/*
 * This function tries to merge the "ex" extent to the next extent in the tree.
 * It always tries to merge towards right. If you want to merge towards
 * left, pass "ex - 1" as argument instead of "ex".
 * Returns 0 if the extents (ex and ex+1) were _not_ merged and returns
 * 1 if they got merged.
 */
static int ext4_ext_try_to_merge_right(struct inode *inode,
				 struct ext4_ext_path *path,
				 struct ext4_extent *ex)
{
	struct ext4_extent_header *eh;
	unsigned int depth, len;
	int merge_done = 0, unwritten;

	depth = ext_depth(inode);
	BUG_ON(path[depth].p_hdr == NULL);
	eh = path[depth].p_hdr;

	while (ex < EXT_LAST_EXTENT(eh)) {
		if (!ext4_can_extents_be_merged(inode, ex, ex + 1))
			break;
		/* merge with next extent! */
		unwritten = ext4_ext_is_unwritten(ex);
		ex->ee_len = cpu_to_le16(ext4_ext_get_actual_len(ex)
				+ ext4_ext_get_actual_len(ex + 1));
		if (unwritten)
			ext4_ext_mark_unwritten(ex);

		if (ex + 1 < EXT_LAST_EXTENT(eh)) {
			len = (EXT_LAST_EXTENT(eh) - ex - 1)
				* sizeof(struct ext4_extent);
			memmove(ex + 1, ex + 2, len);
		}
		le16_add_cpu(&eh->eh_entries, -1);
		merge_done = 1;
		WARN_ON(eh->eh_entries == 0);
		if (!eh->eh_entries)
			EXT4_ERROR_INODE(inode, "eh->eh_entries = 0!");
	}

	return merge_done;
}

/*
 * This function does a very simple check to see if we can collapse
 * an extent tree with a single extent tree leaf block into the inode.
 */
static void ext4_ext_try_to_merge_up(handle_t *handle,
				     struct inode *inode,
				     struct ext4_ext_path *path)
{
	size_t s;
	unsigned max_root = ext4_ext_space_root(inode, 0);
	ext4_fsblk_t blk;

	if ((path[0].p_depth != 1) ||
	    (le16_to_cpu(path[0].p_hdr->eh_entries) != 1) ||
	    (le16_to_cpu(path[1].p_hdr->eh_entries) > max_root))
		return;

	/*
	 * We need to modify the block allocation bitmap and the block
	 * group descriptor to release the extent tree block.  If we
	 * can't get the journal credits, give up.
	 */
	if (ext4_journal_extend(handle, 2))
		return;

	/*
	 * Copy the extent data up to the inode
	 */
	blk = ext4_idx_pblock(path[0].p_idx);
	s = le16_to_cpu(path[1].p_hdr->eh_entries) *
		sizeof(struct ext4_extent_idx);
	s += sizeof(struct ext4_extent_header);

	path[1].p_maxdepth = path[0].p_maxdepth;
	memcpy(path[0].p_hdr, path[1].p_hdr, s);
	path[0].p_depth = 0;
	path[0].p_ext = EXT_FIRST_EXTENT(path[0].p_hdr) +
		(path[1].p_ext - EXT_FIRST_EXTENT(path[1].p_hdr));
	path[0].p_hdr->eh_max = cpu_to_le16(max_root);

	brelse(path[1].p_bh);
	ext4_free_blocks(handle, inode, NULL, blk, 1,
			 EXT4_FREE_BLOCKS_METADATA | EXT4_FREE_BLOCKS_FORGET);
}

/*
 * This function tries to merge the @ex extent to neighbours in the tree.
 * return 1 if merge left else 0.
 */
static void ext4_ext_try_to_merge(handle_t *handle,
				  struct inode *inode,
				  struct ext4_ext_path *path,
				  struct ext4_extent *ex) {
	struct ext4_extent_header *eh;
	unsigned int depth;
	int merge_done = 0;

	depth = ext_depth(inode);
	BUG_ON(path[depth].p_hdr == NULL);
	eh = path[depth].p_hdr;

	if (ex > EXT_FIRST_EXTENT(eh))
		merge_done = ext4_ext_try_to_merge_right(inode, path, ex - 1);

	if (!merge_done)
		(void) ext4_ext_try_to_merge_right(inode, path, ex);

	ext4_ext_try_to_merge_up(handle, inode, path);
}

/*
 * check if a portion of the "newext" extent overlaps with an
 * existing extent.
 *
 * If there is an overlap discovered, it updates the length of the newext
 * such that there will be no overlap, and then returns 1.
 * If there is no overlap found, it returns 0.
 */
static unsigned int ext4_ext_check_overlap(struct ext4_sb_info *sbi,
					   struct inode *inode,
					   struct ext4_extent *newext,
					   struct ext4_ext_path *path)
{
	ext4_lblk_t b1, b2;
	unsigned int depth, len1;
	unsigned int ret = 0;

	b1 = le32_to_cpu(newext->ee_block);
	len1 = ext4_ext_get_actual_len(newext);
	depth = ext_depth(inode);
	if (!path[depth].p_ext)
		goto out;
	b2 = EXT4_LBLK_CMASK(sbi, le32_to_cpu(path[depth].p_ext->ee_block));

	/*
	 * get the next allocated block if the extent in the path
	 * is before the requested block(s)
	 */
	if (b2 < b1) {
		b2 = ext4_ext_next_allocated_block(path);
		if (b2 == EXT_MAX_BLOCKS)
			goto out;
		b2 = EXT4_LBLK_CMASK(sbi, b2);
	}

	/* check for wrap through zero on extent logical start block*/
	if (b1 + len1 < b1) {
		len1 = EXT_MAX_BLOCKS - b1;
		newext->ee_len = cpu_to_le16(len1);
		ret = 1;
	}

	/* check for overlap */
	if (b1 + len1 > b2) {
		newext->ee_len = cpu_to_le16(b2 - b1);
		ret = 1;
	}
out:
	return ret;
}

/*
 * ext4_ext_insert_extent:
 * tries to merge requsted extent into the existing extent or
 * inserts requested extent as new one into the tree,
 * creating new leaf in the no-space case.
 */
int ext4_ext_insert_extent(handle_t *handle, struct inode *inode,
				struct ext4_ext_path **ppath,
				struct ext4_extent *newext, int gb_flags)
{
	struct ext4_ext_path *path = *ppath;
	struct ext4_extent_header *eh;
	struct ext4_extent *ex, *fex;
	struct ext4_extent *nearex; /* nearest extent */
	struct ext4_ext_path *npath = NULL;
	int depth, len, err;
	ext4_lblk_t next;
	int mb_flags = 0, unwritten;

	if (gb_flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE)
		mb_flags |= EXT4_MB_DELALLOC_RESERVED;
	if (unlikely(ext4_ext_get_actual_len(newext) == 0)) {
		EXT4_ERROR_INODE(inode, "ext4_ext_get_actual_len(newext) == 0");
		return -EIO;
	}
	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	eh = path[depth].p_hdr;
	if (unlikely(path[depth].p_hdr == NULL)) {
		EXT4_ERROR_INODE(inode, "path[%d].p_hdr == NULL", depth);
		return -EIO;
	}

	/* try to insert block into found extent and return */
	if (ex && !(gb_flags & EXT4_GET_BLOCKS_PRE_IO)) {

		/*
		 * Try to see whether we should rather test the extent on
		 * right from ex, or from the left of ex. This is because
		 * ext4_find_extent() can return either extent on the
		 * left, or on the right from the searched position. This
		 * will make merging more effective.
		 */
		if (ex < EXT_LAST_EXTENT(eh) &&
		    (le32_to_cpu(ex->ee_block) +
		    ext4_ext_get_actual_len(ex) <
		    le32_to_cpu(newext->ee_block))) {
			ex += 1;
			goto prepend;
		} else if ((ex > EXT_FIRST_EXTENT(eh)) &&
			   (le32_to_cpu(newext->ee_block) +
			   ext4_ext_get_actual_len(newext) <
			   le32_to_cpu(ex->ee_block)))
			ex -= 1;

		/* Try to append newex to the ex */
		if (ext4_can_extents_be_merged(inode, ex, newext)) {
			ext_debug("append [%d]%d block to %u:[%d]%d"
				  "(from %llu)\n",
				  ext4_ext_is_unwritten(newext),
				  ext4_ext_get_actual_len(newext),
				  le32_to_cpu(ex->ee_block),
				  ext4_ext_is_unwritten(ex),
				  ext4_ext_get_actual_len(ex),
				  ext4_ext_pblock(ex));
			err = ext4_ext_get_access(handle, inode,
						  path + depth);
			if (err)
				return err;
			unwritten = ext4_ext_is_unwritten(ex);
			ex->ee_len = cpu_to_le16(ext4_ext_get_actual_len(ex)
					+ ext4_ext_get_actual_len(newext));
			if (unwritten)
				ext4_ext_mark_unwritten(ex);
			eh = path[depth].p_hdr;
			nearex = ex;
			goto merge;
		}

prepend:
		/* Try to prepend newex to the ex */
		if (ext4_can_extents_be_merged(inode, newext, ex)) {
			ext_debug("prepend %u[%d]%d block to %u:[%d]%d"
				  "(from %llu)\n",
				  le32_to_cpu(newext->ee_block),
				  ext4_ext_is_unwritten(newext),
				  ext4_ext_get_actual_len(newext),
				  le32_to_cpu(ex->ee_block),
				  ext4_ext_is_unwritten(ex),
				  ext4_ext_get_actual_len(ex),
				  ext4_ext_pblock(ex));
			err = ext4_ext_get_access(handle, inode,
						  path + depth);
			if (err)
				return err;

			unwritten = ext4_ext_is_unwritten(ex);
			ex->ee_block = newext->ee_block;
			ext4_ext_store_pblock(ex, ext4_ext_pblock(newext));
			ex->ee_len = cpu_to_le16(ext4_ext_get_actual_len(ex)
					+ ext4_ext_get_actual_len(newext));
			if (unwritten)
				ext4_ext_mark_unwritten(ex);
			eh = path[depth].p_hdr;
			nearex = ex;
			goto merge;
		}
	}

	depth = ext_depth(inode);
	eh = path[depth].p_hdr;
	if (le16_to_cpu(eh->eh_entries) < le16_to_cpu(eh->eh_max))
		goto has_space;

	/* probably next leaf has space for us? */
	fex = EXT_LAST_EXTENT(eh);
	next = EXT_MAX_BLOCKS;
	if (le32_to_cpu(newext->ee_block) > le32_to_cpu(fex->ee_block))
		next = ext4_ext_next_leaf_block(path);
	if (next != EXT_MAX_BLOCKS) {
		ext_debug("next leaf block - %u\n", next);
		BUG_ON(npath != NULL);
		npath = ext4_find_extent(inode, next, NULL, 0);
		if (IS_ERR(npath))
			return PTR_ERR(npath);
		BUG_ON(npath->p_depth != path->p_depth);
		eh = npath[depth].p_hdr;
		if (le16_to_cpu(eh->eh_entries) < le16_to_cpu(eh->eh_max)) {
			ext_debug("next leaf isn't full(%d)\n",
				  le16_to_cpu(eh->eh_entries));
			path = npath;
			goto has_space;
		}
		ext_debug("next leaf has no free space(%d,%d)\n",
			  le16_to_cpu(eh->eh_entries), le16_to_cpu(eh->eh_max));
	}

	/*
	 * There is no free space in the found leaf.
	 * We're gonna add a new leaf in the tree.
	 */
	if (gb_flags & EXT4_GET_BLOCKS_METADATA_NOFAIL)
		mb_flags |= EXT4_MB_USE_RESERVED;
	err = ext4_ext_create_new_leaf(handle, inode, mb_flags, gb_flags,
				       ppath, newext);
	if (err)
		goto cleanup;
	depth = ext_depth(inode);
	eh = path[depth].p_hdr;

has_space:
	nearex = path[depth].p_ext;

	err = ext4_ext_get_access(handle, inode, path + depth);
	if (err)
		goto cleanup;

	if (!nearex) {
		/* there is no extent in this leaf, create first one */
		ext_debug("first extent in the leaf: %u:%llu:[%d]%d\n",
				le32_to_cpu(newext->ee_block),
				ext4_ext_pblock(newext),
				ext4_ext_is_unwritten(newext),
				ext4_ext_get_actual_len(newext));
		nearex = EXT_FIRST_EXTENT(eh);
	} else {
		if (le32_to_cpu(newext->ee_block)
			   > le32_to_cpu(nearex->ee_block)) {
			/* Insert after */
			ext_debug("insert %u:%llu:[%d]%d before: "
					"nearest %p\n",
					le32_to_cpu(newext->ee_block),
					ext4_ext_pblock(newext),
					ext4_ext_is_unwritten(newext),
					ext4_ext_get_actual_len(newext),
					nearex);
			nearex++;
		} else {
			/* Insert before */
			BUG_ON(newext->ee_block == nearex->ee_block);
			ext_debug("insert %u:%llu:[%d]%d after: "
					"nearest %p\n",
					le32_to_cpu(newext->ee_block),
					ext4_ext_pblock(newext),
					ext4_ext_is_unwritten(newext),
					ext4_ext_get_actual_len(newext),
					nearex);
		}
		len = EXT_LAST_EXTENT(eh) - nearex + 1;
		if (len > 0) {
			ext_debug("insert %u:%llu:[%d]%d: "
					"move %d extents from 0x%p to 0x%p\n",
					le32_to_cpu(newext->ee_block),
					ext4_ext_pblock(newext),
					ext4_ext_is_unwritten(newext),
					ext4_ext_get_actual_len(newext),
					len, nearex, nearex + 1);
			memmove(nearex + 1, nearex,
				len * sizeof(struct ext4_extent));
		}
	}

	le16_add_cpu(&eh->eh_entries, 1);
	path[depth].p_ext = nearex;
	nearex->ee_block = newext->ee_block;
	ext4_ext_store_pblock(nearex, ext4_ext_pblock(newext));
	nearex->ee_len = newext->ee_len;

merge:
	/* try to merge extents */
	if (!(gb_flags & EXT4_GET_BLOCKS_PRE_IO))
		ext4_ext_try_to_merge(handle, inode, path, nearex);


	/* time to correct all indexes above */
	err = ext4_ext_correct_indexes(handle, inode, path);
	if (err)
		goto cleanup;

	err = ext4_ext_dirty(handle, inode, path + path->p_depth);

cleanup:
	ext4_ext_drop_refs(npath);
	kfree(npath);
	return err;
}

static int ext4_fill_fiemap_extents(struct inode *inode,
				    ext4_lblk_t block, ext4_lblk_t num,
				    struct fiemap_extent_info *fieinfo)
{
	struct ext4_ext_path *path = NULL;
	struct ext4_extent *ex;
	struct extent_status es;
	ext4_lblk_t next, next_del, start = 0, end = 0;
	ext4_lblk_t last = block + num;
	int exists, depth = 0, err = 0;
	unsigned int flags = 0;
	unsigned char blksize_bits = inode->i_sb->s_blocksize_bits;

	while (block < last && block != EXT_MAX_BLOCKS) {
		num = last - block;
		/* find extent for this block */
		down_read(&EXT4_I(inode)->i_data_sem);

		path = ext4_find_extent(inode, block, &path, 0);
		if (IS_ERR(path)) {
			up_read(&EXT4_I(inode)->i_data_sem);
			err = PTR_ERR(path);
			path = NULL;
			break;
		}

		depth = ext_depth(inode);
		if (unlikely(path[depth].p_hdr == NULL)) {
			up_read(&EXT4_I(inode)->i_data_sem);
			EXT4_ERROR_INODE(inode, "path[%d].p_hdr == NULL", depth);
			err = -EIO;
			break;
		}
		ex = path[depth].p_ext;
		next = ext4_ext_next_allocated_block(path);

		flags = 0;
		exists = 0;
		if (!ex) {
			/* there is no extent yet, so try to allocate
			 * all requested space */
			start = block;
			end = block + num;
		} else if (le32_to_cpu(ex->ee_block) > block) {
			/* need to allocate space before found extent */
			start = block;
			end = le32_to_cpu(ex->ee_block);
			if (block + num < end)
				end = block + num;
		} else if (block >= le32_to_cpu(ex->ee_block)
					+ ext4_ext_get_actual_len(ex)) {
			/* need to allocate space after found extent */
			start = block;
			end = block + num;
			if (end >= next)
				end = next;
		} else if (block >= le32_to_cpu(ex->ee_block)) {
			/*
			 * some part of requested space is covered
			 * by found extent
			 */
			start = block;
			end = le32_to_cpu(ex->ee_block)
				+ ext4_ext_get_actual_len(ex);
			if (block + num < end)
				end = block + num;
			exists = 1;
		} else {
			BUG();
		}
		BUG_ON(end <= start);

		if (!exists) {
			es.es_lblk = start;
			es.es_len = end - start;
			es.es_pblk = 0;
		} else {
			es.es_lblk = le32_to_cpu(ex->ee_block);
			es.es_len = ext4_ext_get_actual_len(ex);
			es.es_pblk = ext4_ext_pblock(ex);
			if (ext4_ext_is_unwritten(ex))
				flags |= FIEMAP_EXTENT_UNWRITTEN;
		}

		/*
		 * Find delayed extent and update es accordingly. We call
		 * it even in !exists case to find out whether es is the
		 * last existing extent or not.
		 */
		next_del = ext4_find_delayed_extent(inode, &es);
		if (!exists && next_del) {
			exists = 1;
			flags |= (FIEMAP_EXTENT_DELALLOC |
				  FIEMAP_EXTENT_UNKNOWN);
		}
		up_read(&EXT4_I(inode)->i_data_sem);

		if (unlikely(es.es_len == 0)) {
			EXT4_ERROR_INODE(inode, "es.es_len == 0");
			err = -EIO;
			break;
		}

		/*
		 * This is possible iff next == next_del == EXT_MAX_BLOCKS.
		 * we need to check next == EXT_MAX_BLOCKS because it is
		 * possible that an extent is with unwritten and delayed
		 * status due to when an extent is delayed allocated and
		 * is allocated by fallocate status tree will track both of
		 * them in a extent.
		 *
		 * So we could return a unwritten and delayed extent, and
		 * its block is equal to 'next'.
		 */
		if (next == next_del && next == EXT_MAX_BLOCKS) {
			flags |= FIEMAP_EXTENT_LAST;
			if (unlikely(next_del != EXT_MAX_BLOCKS ||
				     next != EXT_MAX_BLOCKS)) {
				EXT4_ERROR_INODE(inode,
						 "next extent == %u, next "
						 "delalloc extent = %u",
						 next, next_del);
				err = -EIO;
				break;
			}
		}

		if (exists) {
			err = fiemap_fill_next_extent(fieinfo,
				(__u64)es.es_lblk << blksize_bits,
				(__u64)es.es_pblk << blksize_bits,
				(__u64)es.es_len << blksize_bits,
				flags);
			if (err < 0)
				break;
			if (err == 1) {
				err = 0;
				break;
			}
		}

		block = es.es_lblk + es.es_len;
	}

	ext4_ext_drop_refs(path);
	kfree(path);
	return err;
}

/*
 * ext4_ext_put_gap_in_cache:
 * calculate boundaries of the gap that the requested block fits into
 * and cache this gap
 */
static void
ext4_ext_put_gap_in_cache(struct inode *inode, struct ext4_ext_path *path,
				ext4_lblk_t block)
{
	int depth = ext_depth(inode);
	ext4_lblk_t len;
	ext4_lblk_t lblock;
	struct ext4_extent *ex;
	struct extent_status es;

	ex = path[depth].p_ext;
	if (ex == NULL) {
		/* there is no extent yet, so gap is [0;-] */
		lblock = 0;
		len = EXT_MAX_BLOCKS;
		ext_debug("cache gap(whole file):");
	} else if (block < le32_to_cpu(ex->ee_block)) {
		lblock = block;
		len = le32_to_cpu(ex->ee_block) - block;
		ext_debug("cache gap(before): %u [%u:%u]",
				block,
				le32_to_cpu(ex->ee_block),
				 ext4_ext_get_actual_len(ex));
	} else if (block >= le32_to_cpu(ex->ee_block)
			+ ext4_ext_get_actual_len(ex)) {
		ext4_lblk_t next;
		lblock = le32_to_cpu(ex->ee_block)
			+ ext4_ext_get_actual_len(ex);

		next = ext4_ext_next_allocated_block(path);
		ext_debug("cache gap(after): [%u:%u] %u",
				le32_to_cpu(ex->ee_block),
				ext4_ext_get_actual_len(ex),
				block);
		BUG_ON(next == lblock);
		len = next - lblock;
	} else {
		BUG();
	}

	ext4_es_find_delayed_extent_range(inode, lblock, lblock + len - 1, &es);
	if (es.es_len) {
		/* There's delayed extent containing lblock? */
		if (es.es_lblk <= lblock)
			return;
		len = min(es.es_lblk - lblock, len);
	}
	ext_debug(" -> %u:%u\n", lblock, len);
	ext4_es_insert_extent(inode, lblock, len, ~0, EXTENT_STATUS_HOLE);
}

/*
 * ext4_ext_rm_idx:
 * removes index from the index block.
 */
static int ext4_ext_rm_idx(handle_t *handle, struct inode *inode,
			struct ext4_ext_path *path, int depth)
{
	int err;
	ext4_fsblk_t leaf;

	/* free index block */
	depth--;
	path = path + depth;
	leaf = ext4_idx_pblock(path->p_idx);
	if (unlikely(path->p_hdr->eh_entries == 0)) {
		EXT4_ERROR_INODE(inode, "path->p_hdr->eh_entries == 0");
		return -EIO;
	}
	err = ext4_ext_get_access(handle, inode, path);
	if (err)
		return err;

	if (path->p_idx != EXT_LAST_INDEX(path->p_hdr)) {
		int len = EXT_LAST_INDEX(path->p_hdr) - path->p_idx;
		len *= sizeof(struct ext4_extent_idx);
		memmove(path->p_idx, path->p_idx + 1, len);
	}

	le16_add_cpu(&path->p_hdr->eh_entries, -1);
	err = ext4_ext_dirty(handle, inode, path);
	if (err)
		return err;
	ext_debug("index is empty, remove it, free block %llu\n", leaf);
	trace_ext4_ext_rm_idx(inode, leaf);

	ext4_free_blocks(handle, inode, NULL, leaf, 1,
			 EXT4_FREE_BLOCKS_METADATA | EXT4_FREE_BLOCKS_FORGET);

	while (--depth >= 0) {
		if (path->p_idx != EXT_FIRST_INDEX(path->p_hdr))
			break;
		path--;
		err = ext4_ext_get_access(handle, inode, path);
		if (err)
			break;
		path->p_idx->ei_block = (path+1)->p_idx->ei_block;
		err = ext4_ext_dirty(handle, inode, path);
		if (err)
			break;
	}
	return err;
}

/*
 * ext4_ext_calc_credits_for_single_extent:
 * This routine returns max. credits that needed to insert an extent
 * to the extent tree.
 * When pass the actual path, the caller should calculate credits
 * under i_data_sem.
 */
int ext4_ext_calc_credits_for_single_extent(struct inode *inode, int nrblocks,
						struct ext4_ext_path *path)
{
	if (path) {
		int depth = ext_depth(inode);
		int ret = 0;

		/* probably there is space in leaf? */
		if (le16_to_cpu(path[depth].p_hdr->eh_entries)
				< le16_to_cpu(path[depth].p_hdr->eh_max)) {

			/*
			 *  There are some space in the leaf tree, no
			 *  need to account for leaf block credit
			 *
			 *  bitmaps and block group descriptor blocks
			 *  and other metadata blocks still need to be
			 *  accounted.
			 */
			/* 1 bitmap, 1 block group descriptor */
			ret = 2 + EXT4_META_TRANS_BLOCKS(inode->i_sb);
			return ret;
		}
	}

	return ext4_chunk_trans_blocks(inode, nrblocks);
}

/*
 * How many index/leaf blocks need to change/allocate to add @extents extents?
 *
 * If we add a single extent, then in the worse case, each tree level
 * index/leaf need to be changed in case of the tree split.
 *
 * If more extents are inserted, they could cause the whole tree split more
 * than once, but this is really rare.
 */
int ext4_ext_index_trans_blocks(struct inode *inode, int extents)
{
	int index;
	int depth;

	/* If we are converting the inline data, only one is needed here. */
	if (ext4_has_inline_data(inode))
		return 1;

	depth = ext_depth(inode);

	if (extents <= 1)
		index = depth * 2;
	else
		index = depth * 3;

	return index;
}

static inline int get_default_free_blocks_flags(struct inode *inode)
{
	if (S_ISDIR(inode->i_mode) || S_ISLNK(inode->i_mode))
		return EXT4_FREE_BLOCKS_METADATA | EXT4_FREE_BLOCKS_FORGET;
	else if (ext4_should_journal_data(inode))
		return EXT4_FREE_BLOCKS_FORGET;
	return 0;
}

static int ext4_remove_blocks(handle_t *handle, struct inode *inode,
			      struct ext4_extent *ex,
			      long long *partial_cluster,
			      ext4_lblk_t from, ext4_lblk_t to)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	unsigned short ee_len = ext4_ext_get_actual_len(ex);
	ext4_fsblk_t pblk;
	int flags = get_default_free_blocks_flags(inode);

	/*
	 * For bigalloc file systems, we never free a partial cluster
	 * at the beginning of the extent.  Instead, we make a note
	 * that we tried freeing the cluster, and check to see if we
	 * need to free it on a subsequent call to ext4_remove_blocks,
	 * or at the end of ext4_ext_rm_leaf or ext4_ext_remove_space.
	 */
	flags |= EXT4_FREE_BLOCKS_NOFREE_FIRST_CLUSTER;

	trace_ext4_remove_blocks(inode, ex, from, to, *partial_cluster);
	/*
	 * If we have a partial cluster, and it's different from the
	 * cluster of the last block, we need to explicitly free the
	 * partial cluster here.
	 */
	pblk = ext4_ext_pblock(ex) + ee_len - 1;
	if (*partial_cluster > 0 &&
	    *partial_cluster != (long long) EXT4_B2C(sbi, pblk)) {
		ext4_free_blocks(handle, inode, NULL,
				 EXT4_C2B(sbi, *partial_cluster),
				 sbi->s_cluster_ratio, flags);
		*partial_cluster = 0;
	}

#ifdef EXTENTS_STATS
	{
		struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
		spin_lock(&sbi->s_ext_stats_lock);
		sbi->s_ext_blocks += ee_len;
		sbi->s_ext_extents++;
		if (ee_len < sbi->s_ext_min)
			sbi->s_ext_min = ee_len;
		if (ee_len > sbi->s_ext_max)
			sbi->s_ext_max = ee_len;
		if (ext_depth(inode) > sbi->s_depth_max)
			sbi->s_depth_max = ext_depth(inode);
		spin_unlock(&sbi->s_ext_stats_lock);
	}
#endif
	if (from >= le32_to_cpu(ex->ee_block)
	    && to == le32_to_cpu(ex->ee_block) + ee_len - 1) {
		/* tail removal */
		ext4_lblk_t num;
		long long first_cluster;

		num = le32_to_cpu(ex->ee_block) + ee_len - from;
		pblk = ext4_ext_pblock(ex) + ee_len - num;
		/*
		 * Usually we want to free partial cluster at the end of the
		 * extent, except for the situation when the cluster is still
		 * used by any other extent (partial_cluster is negative).
		 */
		if (*partial_cluster < 0 &&
		    *partial_cluster == -(long long) EXT4_B2C(sbi, pblk+num-1))
			flags |= EXT4_FREE_BLOCKS_NOFREE_LAST_CLUSTER;

		ext_debug("free last %u blocks starting %llu partial %lld\n",
			  num, pblk, *partial_cluster);
		ext4_free_blocks(handle, inode, NULL, pblk, num, flags);
		/*
		 * If the block range to be freed didn't start at the
		 * beginning of a cluster, and we removed the entire
		 * extent and the cluster is not used by any other extent,
		 * save the partial cluster here, since we might need to
		 * delete if we determine that the truncate or punch hole
		 * operation has removed all of the blocks in the cluster.
		 * If that cluster is used by another extent, preserve its
		 * negative value so it isn't freed later on.
		 *
		 * If the whole extent wasn't freed, we've reached the
		 * start of the truncated/punched region and have finished
		 * removing blocks.  If there's a partial cluster here it's
		 * shared with the remainder of the extent and is no longer
		 * a candidate for removal.
		 */
		if (EXT4_PBLK_COFF(sbi, pblk) && ee_len == num) {
			first_cluster = (long long) EXT4_B2C(sbi, pblk);
			if (first_cluster != -*partial_cluster)
				*partial_cluster = first_cluster;
		} else {
			*partial_cluster = 0;
		}
	} else
		ext4_error(sbi->s_sb, "strange request: removal(2) "
			   "%u-%u from %u:%u\n",
			   from, to, le32_to_cpu(ex->ee_block), ee_len);
	return 0;
}


/*
 * ext4_ext_rm_leaf() Removes the extents associated with the
 * blocks appearing between "start" and "end".  Both "start"
 * and "end" must appear in the same extent or EIO is returned.
 *
 * @handle: The journal handle
 * @inode:  The files inode
 * @path:   The path to the leaf
 * @partial_cluster: The cluster which we'll have to free if all extents
 *                   has been released from it.  However, if this value is
 *                   negative, it's a cluster just to the right of the
 *                   punched region and it must not be freed.
 * @start:  The first block to remove
 * @end:   The last block to remove
 */
static int
ext4_ext_rm_leaf(handle_t *handle, struct inode *inode,
		 struct ext4_ext_path *path,
		 long long *partial_cluster,
		 ext4_lblk_t start, ext4_lblk_t end)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	int err = 0, correct_index = 0;
	int depth = ext_depth(inode), credits;
	struct ext4_extent_header *eh;
	ext4_lblk_t a, b;
	unsigned num;
	ext4_lblk_t ex_ee_block;
	unsigned short ex_ee_len;
	unsigned unwritten = 0;
	struct ext4_extent *ex;
	ext4_fsblk_t pblk;

	/* the header must be checked already in ext4_ext_remove_space() */
	ext_debug("truncate since %u in leaf to %u\n", start, end);
	if (!path[depth].p_hdr)
		path[depth].p_hdr = ext_block_hdr(path[depth].p_bh);
	eh = path[depth].p_hdr;
	if (unlikely(path[depth].p_hdr == NULL)) {
		EXT4_ERROR_INODE(inode, "path[%d].p_hdr == NULL", depth);
		return -EIO;
	}
	/* find where to start removing */
	ex = path[depth].p_ext;
	if (!ex)
		ex = EXT_LAST_EXTENT(eh);

	ex_ee_block = le32_to_cpu(ex->ee_block);
	ex_ee_len = ext4_ext_get_actual_len(ex);

	trace_ext4_ext_rm_leaf(inode, start, ex, *partial_cluster);

	while (ex >= EXT_FIRST_EXTENT(eh) &&
			ex_ee_block + ex_ee_len > start) {

		if (ext4_ext_is_unwritten(ex))
			unwritten = 1;
		else
			unwritten = 0;

		ext_debug("remove ext %u:[%d]%d\n", ex_ee_block,
			  unwritten, ex_ee_len);
		path[depth].p_ext = ex;

		a = ex_ee_block > start ? ex_ee_block : start;
		b = ex_ee_block+ex_ee_len - 1 < end ?
			ex_ee_block+ex_ee_len - 1 : end;

		ext_debug("  border %u:%u\n", a, b);

		/* If this extent is beyond the end of the hole, skip it */
		if (end < ex_ee_block) {
			/*
			 * We're going to skip this extent and move to another,
			 * so note that its first cluster is in use to avoid
			 * freeing it when removing blocks.  Eventually, the
			 * right edge of the truncated/punched region will
			 * be just to the left.
			 */
			if (sbi->s_cluster_ratio > 1) {
				pblk = ext4_ext_pblock(ex);
				*partial_cluster =
					-(long long) EXT4_B2C(sbi, pblk);
			}
			ex--;
			ex_ee_block = le32_to_cpu(ex->ee_block);
			ex_ee_len = ext4_ext_get_actual_len(ex);
			continue;
		} else if (b != ex_ee_block + ex_ee_len - 1) {
			EXT4_ERROR_INODE(inode,
					 "can not handle truncate %u:%u "
					 "on extent %u:%u",
					 start, end, ex_ee_block,
					 ex_ee_block + ex_ee_len - 1);
			err = -EIO;
			goto out;
		} else if (a != ex_ee_block) {
			/* remove tail of the extent */
			num = a - ex_ee_block;
		} else {
			/* remove whole extent: excellent! */
			num = 0;
		}
		/*
		 * 3 for leaf, sb, and inode plus 2 (bmap and group
		 * descriptor) for each block group; assume two block
		 * groups plus ex_ee_len/blocks_per_block_group for
		 * the worst case
		 */
		credits = 7 + 2*(ex_ee_len/EXT4_BLOCKS_PER_GROUP(inode->i_sb));
		if (ex == EXT_FIRST_EXTENT(eh)) {
			correct_index = 1;
			credits += (ext_depth(inode)) + 1;
		}
		credits += EXT4_MAXQUOTAS_TRANS_BLOCKS(inode->i_sb);

		err = ext4_ext_truncate_extend_restart(handle, inode, credits);
		if (err)
			goto out;

		err = ext4_ext_get_access(handle, inode, path + depth);
		if (err)
			goto out;

		err = ext4_remove_blocks(handle, inode, ex, partial_cluster,
					 a, b);
		if (err)
			goto out;

		if (num == 0)
			/* this extent is removed; mark slot entirely unused */
			ext4_ext_store_pblock(ex, 0);

		ex->ee_len = cpu_to_le16(num);
		/*
		 * Do not mark unwritten if all the blocks in the
		 * extent have been removed.
		 */
		if (unwritten && num)
			ext4_ext_mark_unwritten(ex);
		/*
		 * If the extent was completely released,
		 * we need to remove it from the leaf
		 */
		if (num == 0) {
			if (end != EXT_MAX_BLOCKS - 1) {
				/*
				 * For hole punching, we need to scoot all the
				 * extents up when an extent is removed so that
				 * we dont have blank extents in the middle
				 */
				memmove(ex, ex+1, (EXT_LAST_EXTENT(eh) - ex) *
					sizeof(struct ext4_extent));

				/* Now get rid of the one at the end */
				memset(EXT_LAST_EXTENT(eh), 0,
					sizeof(struct ext4_extent));
			}
			le16_add_cpu(&eh->eh_entries, -1);
		}

		err = ext4_ext_dirty(handle, inode, path + depth);
		if (err)
			goto out;

		ext_debug("new extent: %u:%u:%llu\n", ex_ee_block, num,
				ext4_ext_pblock(ex));
		ex--;
		ex_ee_block = le32_to_cpu(ex->ee_block);
		ex_ee_len = ext4_ext_get_actual_len(ex);
	}

	if (correct_index && eh->eh_entries)
		err = ext4_ext_correct_indexes(handle, inode, path);

	/*
	 * If there's a partial cluster and at least one extent remains in
	 * the leaf, free the partial cluster if it isn't shared with the
	 * current extent.  If it is shared with the current extent
	 * we zero partial_cluster because we've reached the start of the
	 * truncated/punched region and we're done removing blocks.
	 */
	if (*partial_cluster > 0 && ex >= EXT_FIRST_EXTENT(eh)) {
		pblk = ext4_ext_pblock(ex) + ex_ee_len - 1;
		if (*partial_cluster != (long long) EXT4_B2C(sbi, pblk)) {
			ext4_free_blocks(handle, inode, NULL,
					 EXT4_C2B(sbi, *partial_cluster),
					 sbi->s_cluster_ratio,
					 get_default_free_blocks_flags(inode));
		}
		*partial_cluster = 0;
	}

	/* if this leaf is free, then we should
	 * remove it from index block above */
	if (err == 0 && eh->eh_entries == 0 && path[depth].p_bh != NULL)
		err = ext4_ext_rm_idx(handle, inode, path, depth);

out:
	return err;
}

/*
 * ext4_ext_more_to_rm:
 * returns 1 if current index has to be freed (even partial)
 */
static int
ext4_ext_more_to_rm(struct ext4_ext_path *path)
{
	BUG_ON(path->p_idx == NULL);

	if (path->p_idx < EXT_FIRST_INDEX(path->p_hdr))
		return 0;

	/*
	 * if truncate on deeper level happened, it wasn't partial,
	 * so we have to consider current index for truncation
	 */
	if (le16_to_cpu(path->p_hdr->eh_entries) == path->p_block)
		return 0;
	return 1;
}

int ext4_ext_remove_space(struct inode *inode, ext4_lblk_t start,
			  ext4_lblk_t end)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	int depth = ext_depth(inode);
	struct ext4_ext_path *path = NULL;
	long long partial_cluster = 0;
	handle_t *handle;
	int i = 0, err = 0;

	ext_debug("truncate since %u to %u\n", start, end);

	/* probably first extent we're gonna free will be last in block */
	handle = ext4_journal_start(inode, EXT4_HT_TRUNCATE, depth + 1);
	if (IS_ERR(handle))
		return PTR_ERR(handle);

again:
	trace_ext4_ext_remove_space(inode, start, end, depth);

	/*
	 * Check if we are removing extents inside the extent tree. If that
	 * is the case, we are going to punch a hole inside the extent tree
	 * so we have to check whether we need to split the extent covering
	 * the last block to remove so we can easily remove the part of it
	 * in ext4_ext_rm_leaf().
	 */
	if (end < EXT_MAX_BLOCKS - 1) {
		struct ext4_extent *ex;
		ext4_lblk_t ee_block, ex_end, lblk;
		ext4_fsblk_t pblk;

		/* find extent for or closest extent to this block */
		path = ext4_find_extent(inode, end, NULL, EXT4_EX_NOCACHE);
		if (IS_ERR(path)) {
			ext4_journal_stop(handle);
			return PTR_ERR(path);
		}
		depth = ext_depth(inode);
		/* Leaf not may not exist only if inode has no blocks at all */
		ex = path[depth].p_ext;
		if (!ex) {
			if (depth) {
				EXT4_ERROR_INODE(inode,
						 "path[%d].p_hdr == NULL",
						 depth);
				err = -EIO;
			}
			goto out;
		}

		ee_block = le32_to_cpu(ex->ee_block);
		ex_end = ee_block + ext4_ext_get_actual_len(ex) - 1;

		/*
		 * See if the last block is inside the extent, if so split
		 * the extent at 'end' block so we can easily remove the
		 * tail of the first part of the split extent in
		 * ext4_ext_rm_leaf().
		 */
		if (end >= ee_block && end < ex_end) {

			/*
			 * If we're going to split the extent, note that
			 * the cluster containing the block after 'end' is
			 * in use to avoid freeing it when removing blocks.
			 */
			if (sbi->s_cluster_ratio > 1) {
				pblk = ext4_ext_pblock(ex) + end - ee_block + 2;
				partial_cluster =
					-(long long) EXT4_B2C(sbi, pblk);
			}

			/*
			 * Split the extent in two so that 'end' is the last
			 * block in the first new extent. Also we should not
			 * fail removing space due to ENOSPC so try to use
			 * reserved block if that happens.
			 */
			err = ext4_force_split_extent_at(handle, inode, &path,
							 end + 1, 1);
			if (err < 0)
				goto out;

		} else if (sbi->s_cluster_ratio > 1 && end >= ex_end) {
			/*
			 * If there's an extent to the right its first cluster
			 * contains the immediate right boundary of the
			 * truncated/punched region.  Set partial_cluster to
			 * its negative value so it won't be freed if shared
			 * with the current extent.  The end < ee_block case
			 * is handled in ext4_ext_rm_leaf().
			 */
			lblk = ex_end + 1;
			err = ext4_ext_search_right(inode, path, &lblk, &pblk,
						    &ex);
			if (err)
				goto out;
			if (pblk)
				partial_cluster =
					-(long long) EXT4_B2C(sbi, pblk);
		}
	}
	/*
	 * We start scanning from right side, freeing all the blocks
	 * after i_size and walking into the tree depth-wise.
	 */
	depth = ext_depth(inode);
	if (path) {
		int k = i = depth;
		while (--k > 0)
			path[k].p_block =
				le16_to_cpu(path[k].p_hdr->eh_entries)+1;
	} else {
		path = kzalloc(sizeof(struct ext4_ext_path) * (depth + 1),
			       GFP_NOFS);
		if (path == NULL) {
			ext4_journal_stop(handle);
			return -ENOMEM;
		}
		path[0].p_maxdepth = path[0].p_depth = depth;
		path[0].p_hdr = ext_inode_hdr(inode);
		i = 0;

		if (ext4_ext_check(inode, path[0].p_hdr, depth, 0)) {
			err = -EIO;
			goto out;
		}
	}
	err = 0;

	while (i >= 0 && err == 0) {
		if (i == depth) {
			/* this is leaf block */
			err = ext4_ext_rm_leaf(handle, inode, path,
					       &partial_cluster, start,
					       end);
			/* root level has p_bh == NULL, brelse() eats this */
			brelse(path[i].p_bh);
			path[i].p_bh = NULL;
			i--;
			continue;
		}

		/* this is index block */
		if (!path[i].p_hdr) {
			ext_debug("initialize header\n");
			path[i].p_hdr = ext_block_hdr(path[i].p_bh);
		}

		if (!path[i].p_idx) {
			/* this level hasn't been touched yet */
			path[i].p_idx = EXT_LAST_INDEX(path[i].p_hdr);
			path[i].p_block = le16_to_cpu(path[i].p_hdr->eh_entries)+1;
			ext_debug("init index ptr: hdr 0x%p, num %d\n",
				  path[i].p_hdr,
				  le16_to_cpu(path[i].p_hdr->eh_entries));
		} else {
			/* we were already here, see at next index */
			path[i].p_idx--;
		}

		ext_debug("level %d - index, first 0x%p, cur 0x%p\n",
				i, EXT_FIRST_INDEX(path[i].p_hdr),
				path[i].p_idx);
		if (ext4_ext_more_to_rm(path + i)) {
			struct buffer_head *bh;
			/* go to the next level */
			ext_debug("move to level %d (block %llu)\n",
				  i + 1, ext4_idx_pblock(path[i].p_idx));
			memset(path + i + 1, 0, sizeof(*path));
			bh = read_extent_tree_block(inode,
				ext4_idx_pblock(path[i].p_idx), depth - i - 1,
				EXT4_EX_NOCACHE);
			if (IS_ERR(bh)) {
				/* should we reset i_size? */
				err = PTR_ERR(bh);
				break;
			}
			/* Yield here to deal with large extent trees.
			 * Should be a no-op if we did IO above. */
			cond_resched();
			if (WARN_ON(i + 1 > depth)) {
				err = -EIO;
				break;
			}
			path[i + 1].p_bh = bh;

			/* save actual number of indexes since this
			 * number is changed at the next iteration */
			path[i].p_block = le16_to_cpu(path[i].p_hdr->eh_entries);
			i++;
		} else {
			/* we finished processing this index, go up */
			if (path[i].p_hdr->eh_entries == 0 && i > 0) {
				/* index is empty, remove it;
				 * handle must be already prepared by the
				 * truncatei_leaf() */
				err = ext4_ext_rm_idx(handle, inode, path, i);
			}
			/* root level has p_bh == NULL, brelse() eats this */
			brelse(path[i].p_bh);
			path[i].p_bh = NULL;
			i--;
			ext_debug("return to level %d\n", i);
		}
	}

	trace_ext4_ext_remove_space_done(inode, start, end, depth,
			partial_cluster, path->p_hdr->eh_entries);

	/*
	 * If we still have something in the partial cluster and we have removed
	 * even the first extent, then we should free the blocks in the partial
	 * cluster as well.  (This code will only run when there are no leaves
	 * to the immediate left of the truncated/punched region.)
	 */
	if (partial_cluster > 0 && err == 0) {
		/* don't zero partial_cluster since it's not used afterwards */
		ext4_free_blocks(handle, inode, NULL,
				 EXT4_C2B(sbi, partial_cluster),
				 sbi->s_cluster_ratio,
				 get_default_free_blocks_flags(inode));
	}

	/* TODO: flexible tree reduction should be here */
	if (path->p_hdr->eh_entries == 0) {
		/*
		 * truncate to zero freed all the tree,
		 * so we need to correct eh_depth
		 */
		err = ext4_ext_get_access(handle, inode, path);
		if (err == 0) {
			ext_inode_hdr(inode)->eh_depth = 0;
			ext_inode_hdr(inode)->eh_max =
				cpu_to_le16(ext4_ext_space_root(inode, 0));
			err = ext4_ext_dirty(handle, inode, path);
		}
	}
out:
	ext4_ext_drop_refs(path);
	kfree(path);
	path = NULL;
	if (err == -EAGAIN)
		goto again;
	ext4_journal_stop(handle);

	return err;
}

/*
 * called at mount time
 */
void ext4_ext_init(struct super_block *sb)
{
	/*
	 * possible initialization would be here
	 */

	if (EXT4_HAS_INCOMPAT_FEATURE(sb, EXT4_FEATURE_INCOMPAT_EXTENTS)) {
#if defined(AGGRESSIVE_TEST) || defined(CHECK_BINSEARCH) || defined(EXTENTS_STATS)
		printk(KERN_INFO "EXT4-fs: file extents enabled"
#ifdef AGGRESSIVE_TEST
		       ", aggressive tests"
#endif
#ifdef CHECK_BINSEARCH
		       ", check binsearch"
#endif
#ifdef EXTENTS_STATS
		       ", stats"
#endif
		       "\n");
#endif
#ifdef EXTENTS_STATS
		spin_lock_init(&EXT4_SB(sb)->s_ext_stats_lock);
		EXT4_SB(sb)->s_ext_min = 1 << 30;
		EXT4_SB(sb)->s_ext_max = 0;
#endif
	}
}

/*
 * called at umount time
 */
void ext4_ext_release(struct super_block *sb)
{
	if (!EXT4_HAS_INCOMPAT_FEATURE(sb, EXT4_FEATURE_INCOMPAT_EXTENTS))
		return;

#ifdef EXTENTS_STATS
	if (EXT4_SB(sb)->s_ext_blocks && EXT4_SB(sb)->s_ext_extents) {
		struct ext4_sb_info *sbi = EXT4_SB(sb);
		printk(KERN_ERR "EXT4-fs: %lu blocks in %lu extents (%lu ave)\n",
			sbi->s_ext_blocks, sbi->s_ext_extents,
			sbi->s_ext_blocks / sbi->s_ext_extents);
		printk(KERN_ERR "EXT4-fs: extents: %lu min, %lu max, max depth %lu\n",
			sbi->s_ext_min, sbi->s_ext_max, sbi->s_depth_max);
	}
#endif
}

static int ext4_zeroout_es(struct inode *inode, struct ext4_extent *ex)
{
	ext4_lblk_t  ee_block;
	ext4_fsblk_t ee_pblock;
	unsigned int ee_len;

	ee_block  = le32_to_cpu(ex->ee_block);
	ee_len    = ext4_ext_get_actual_len(ex);
	ee_pblock = ext4_ext_pblock(ex);

	if (ee_len == 0)
		return 0;

	return ext4_es_insert_extent(inode, ee_block, ee_len, ee_pblock,
				     EXTENT_STATUS_WRITTEN);
}

/* FIXME!! we need to try to merge to left or right after zero-out  */
static int ext4_ext_zeroout(struct inode *inode, struct ext4_extent *ex)
{
	ext4_fsblk_t ee_pblock;
	unsigned int ee_len;
	int ret;

	ee_len    = ext4_ext_get_actual_len(ex);
	ee_pblock = ext4_ext_pblock(ex);

	if (ext4_encrypted_inode(inode))
		return ext4_encrypted_zeroout(inode, ex);

	ret = sb_issue_zeroout(inode->i_sb, ee_pblock, ee_len, GFP_NOFS);
	if (ret > 0)
		ret = 0;

	return ret;
}

/*
 * ext4_split_extent_at() splits an extent at given block.
 *
 * @handle: the journal handle
 * @inode: the file inode
 * @path: the path to the extent
 * @split: the logical block where the extent is splitted.
 * @split_flags: indicates if the extent could be zeroout if split fails, and
 *		 the states(init or unwritten) of new extents.
 * @flags: flags used to insert new extent to extent tree.
 *
 *
 * Splits extent [a, b] into two extents [a, @split) and [@split, b], states
 * of which are deterimined by split_flag.
 *
 * There are two cases:
 *  a> the extent are splitted into two extent.
 *  b> split is not needed, and just mark the extent.
 *
 * return 0 on success.
 */
static int ext4_split_extent_at(handle_t *handle,
			     struct inode *inode,
			     struct ext4_ext_path **ppath,
			     ext4_lblk_t split,
			     int split_flag,
			     int flags)
{
	struct ext4_ext_path *path = *ppath;
	ext4_fsblk_t newblock;
	ext4_lblk_t ee_block;
	struct ext4_extent *ex, newex, orig_ex, zero_ex;
	struct ext4_extent *ex2 = NULL;
	unsigned int ee_len, depth;
	int err = 0;

	BUG_ON((split_flag & (EXT4_EXT_DATA_VALID1 | EXT4_EXT_DATA_VALID2)) ==
	       (EXT4_EXT_DATA_VALID1 | EXT4_EXT_DATA_VALID2));

	ext_debug("ext4_split_extents_at: inode %lu, logical"
		"block %llu\n", inode->i_ino, (unsigned long long)split);

	ext4_ext_show_leaf(inode, path);

	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);
	newblock = split - ee_block + ext4_ext_pblock(ex);

	BUG_ON(split < ee_block || split >= (ee_block + ee_len));
	BUG_ON(!ext4_ext_is_unwritten(ex) &&
	       split_flag & (EXT4_EXT_MAY_ZEROOUT |
			     EXT4_EXT_MARK_UNWRIT1 |
			     EXT4_EXT_MARK_UNWRIT2));

	err = ext4_ext_get_access(handle, inode, path + depth);
	if (err)
		goto out;

	if (split == ee_block) {
		/*
		 * case b: block @split is the block that the extent begins with
		 * then we just change the state of the extent, and splitting
		 * is not needed.
		 */
		if (split_flag & EXT4_EXT_MARK_UNWRIT2)
			ext4_ext_mark_unwritten(ex);
		else
			ext4_ext_mark_initialized(ex);

		if (!(flags & EXT4_GET_BLOCKS_PRE_IO))
			ext4_ext_try_to_merge(handle, inode, path, ex);

		err = ext4_ext_dirty(handle, inode, path + path->p_depth);
		goto out;
	}

	/* case a */
	memcpy(&orig_ex, ex, sizeof(orig_ex));
	ex->ee_len = cpu_to_le16(split - ee_block);
	if (split_flag & EXT4_EXT_MARK_UNWRIT1)
		ext4_ext_mark_unwritten(ex);

	/*
	 * path may lead to new leaf, not to original leaf any more
	 * after ext4_ext_insert_extent() returns,
	 */
	err = ext4_ext_dirty(handle, inode, path + depth);
	if (err)
		goto fix_extent_len;

	ex2 = &newex;
	ex2->ee_block = cpu_to_le32(split);
	ex2->ee_len   = cpu_to_le16(ee_len - (split - ee_block));
	ext4_ext_store_pblock(ex2, newblock);
	if (split_flag & EXT4_EXT_MARK_UNWRIT2)
		ext4_ext_mark_unwritten(ex2);

	err = ext4_ext_insert_extent(handle, inode, ppath, &newex, flags);
	if (err == -ENOSPC && (EXT4_EXT_MAY_ZEROOUT & split_flag)) {
		if (split_flag & (EXT4_EXT_DATA_VALID1|EXT4_EXT_DATA_VALID2)) {
			if (split_flag & EXT4_EXT_DATA_VALID1) {
				err = ext4_ext_zeroout(inode, ex2);
				zero_ex.ee_block = ex2->ee_block;
				zero_ex.ee_len = cpu_to_le16(
						ext4_ext_get_actual_len(ex2));
				ext4_ext_store_pblock(&zero_ex,
						      ext4_ext_pblock(ex2));
			} else {
				err = ext4_ext_zeroout(inode, ex);
				zero_ex.ee_block = ex->ee_block;
				zero_ex.ee_len = cpu_to_le16(
						ext4_ext_get_actual_len(ex));
				ext4_ext_store_pblock(&zero_ex,
						      ext4_ext_pblock(ex));
			}
		} else {
			err = ext4_ext_zeroout(inode, &orig_ex);
			zero_ex.ee_block = orig_ex.ee_block;
			zero_ex.ee_len = cpu_to_le16(
						ext4_ext_get_actual_len(&orig_ex));
			ext4_ext_store_pblock(&zero_ex,
					      ext4_ext_pblock(&orig_ex));
		}

		if (err)
			goto fix_extent_len;
		/* update the extent length and mark as initialized */
		ex->ee_len = cpu_to_le16(ee_len);
		ext4_ext_try_to_merge(handle, inode, path, ex);
		err = ext4_ext_dirty(handle, inode, path + path->p_depth);
		if (err)
			goto fix_extent_len;

		/* update extent status tree */
		err = ext4_zeroout_es(inode, &zero_ex);

		goto out;
	} else if (err)
		goto fix_extent_len;

out:
	ext4_ext_show_leaf(inode, path);
	return err;

fix_extent_len:
	ex->ee_len = orig_ex.ee_len;
	ext4_ext_dirty(handle, inode, path + path->p_depth);
	return err;
}

/*
 * ext4_split_extents() splits an extent and mark extent which is covered
 * by @map as split_flags indicates
 *
 * It may result in splitting the extent into multiple extents (up to three)
 * There are three possibilities:
 *   a> There is no split required
 *   b> Splits in two extents: Split is happening at either end of the extent
 *   c> Splits in three extents: Somone is splitting in middle of the extent
 *
 */
static int ext4_split_extent(handle_t *handle,
			      struct inode *inode,
			      struct ext4_ext_path **ppath,
			      struct ext4_map_blocks *map,
			      int split_flag,
			      int flags)
{
	struct ext4_ext_path *path = *ppath;
	ext4_lblk_t ee_block;
	struct ext4_extent *ex;
	unsigned int ee_len, depth;
	int err = 0;
	int unwritten;
	int split_flag1, flags1;
	int allocated = map->m_len;

	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);
	unwritten = ext4_ext_is_unwritten(ex);

	if (map->m_lblk + map->m_len < ee_block + ee_len) {
		split_flag1 = split_flag & EXT4_EXT_MAY_ZEROOUT;
		flags1 = flags | EXT4_GET_BLOCKS_PRE_IO;
		if (unwritten)
			split_flag1 |= EXT4_EXT_MARK_UNWRIT1 |
				       EXT4_EXT_MARK_UNWRIT2;
		if (split_flag & EXT4_EXT_DATA_VALID2)
			split_flag1 |= EXT4_EXT_DATA_VALID1;
		err = ext4_split_extent_at(handle, inode, ppath,
				map->m_lblk + map->m_len, split_flag1, flags1);
		if (err)
			goto out;
	} else {
		allocated = ee_len - (map->m_lblk - ee_block);
	}
	/*
	 * Update path is required because previous ext4_split_extent_at() may
	 * result in split of original leaf or extent zeroout.
	 */
	path = ext4_find_extent(inode, map->m_lblk, ppath, 0);
	if (IS_ERR(path))
		return PTR_ERR(path);
	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	if (!ex) {
		EXT4_ERROR_INODE(inode, "unexpected hole at %lu",
				 (unsigned long) map->m_lblk);
		return -EIO;
	}
	unwritten = ext4_ext_is_unwritten(ex);
	split_flag1 = 0;

	if (map->m_lblk >= ee_block) {
		split_flag1 = split_flag & EXT4_EXT_DATA_VALID2;
		if (unwritten) {
			split_flag1 |= EXT4_EXT_MARK_UNWRIT1;
			split_flag1 |= split_flag & (EXT4_EXT_MAY_ZEROOUT |
						     EXT4_EXT_MARK_UNWRIT2);
		}
		err = ext4_split_extent_at(handle, inode, ppath,
				map->m_lblk, split_flag1, flags);
		if (err)
			goto out;
	}

	ext4_ext_show_leaf(inode, path);
out:
	return err ? err : allocated;
}

/*
 * This function is called by ext4_ext_map_blocks() if someone tries to write
 * to an unwritten extent. It may result in splitting the unwritten
 * extent into multiple extents (up to three - one initialized and two
 * unwritten).
 * There are three possibilities:
 *   a> There is no split required: Entire extent should be initialized
 *   b> Splits in two extents: Write is happening at either end of the extent
 *   c> Splits in three extents: Somone is writing in middle of the extent
 *
 * Pre-conditions:
 *  - The extent pointed to by 'path' is unwritten.
 *  - The extent pointed to by 'path' contains a superset
 *    of the logical span [map->m_lblk, map->m_lblk + map->m_len).
 *
 * Post-conditions on success:
 *  - the returned value is the number of blocks beyond map->l_lblk
 *    that are allocated and initialized.
 *    It is guaranteed to be >= map->m_len.
 */
static int ext4_ext_convert_to_initialized(handle_t *handle,
					   struct inode *inode,
					   struct ext4_map_blocks *map,
					   struct ext4_ext_path **ppath,
					   int flags)
{
	struct ext4_ext_path *path = *ppath;
	struct ext4_sb_info *sbi;
	struct ext4_extent_header *eh;
	struct ext4_map_blocks split_map;
	struct ext4_extent zero_ex;
	struct ext4_extent *ex, *abut_ex;
	ext4_lblk_t ee_block, eof_block;
	unsigned int ee_len, depth, map_len = map->m_len;
	int allocated = 0, max_zeroout = 0;
	int err = 0;
	int split_flag = 0;

	ext_debug("ext4_ext_convert_to_initialized: inode %lu, logical"
		"block %llu, max_blocks %u\n", inode->i_ino,
		(unsigned long long)map->m_lblk, map_len);

	sbi = EXT4_SB(inode->i_sb);
	eof_block = (inode->i_size + inode->i_sb->s_blocksize - 1) >>
		inode->i_sb->s_blocksize_bits;
	if (eof_block < map->m_lblk + map_len)
		eof_block = map->m_lblk + map_len;

	depth = ext_depth(inode);
	eh = path[depth].p_hdr;
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);
	zero_ex.ee_len = 0;

	trace_ext4_ext_convert_to_initialized_enter(inode, map, ex);

	/* Pre-conditions */
	BUG_ON(!ext4_ext_is_unwritten(ex));
	BUG_ON(!in_range(map->m_lblk, ee_block, ee_len));

	/*
	 * Attempt to transfer newly initialized blocks from the currently
	 * unwritten extent to its neighbor. This is much cheaper
	 * than an insertion followed by a merge as those involve costly
	 * memmove() calls. Transferring to the left is the common case in
	 * steady state for workloads doing fallocate(FALLOC_FL_KEEP_SIZE)
	 * followed by append writes.
	 *
	 * Limitations of the current logic:
	 *  - L1: we do not deal with writes covering the whole extent.
	 *    This would require removing the extent if the transfer
	 *    is possible.
	 *  - L2: we only attempt to merge with an extent stored in the
	 *    same extent tree node.
	 */
	if ((map->m_lblk == ee_block) &&
		/* See if we can merge left */
		(map_len < ee_len) &&		/*L1*/
		(ex > EXT_FIRST_EXTENT(eh))) {	/*L2*/
		ext4_lblk_t prev_lblk;
		ext4_fsblk_t prev_pblk, ee_pblk;
		unsigned int prev_len;

		abut_ex = ex - 1;
		prev_lblk = le32_to_cpu(abut_ex->ee_block);
		prev_len = ext4_ext_get_actual_len(abut_ex);
		prev_pblk = ext4_ext_pblock(abut_ex);
		ee_pblk = ext4_ext_pblock(ex);

		/*
		 * A transfer of blocks from 'ex' to 'abut_ex' is allowed
		 * upon those conditions:
		 * - C1: abut_ex is initialized,
		 * - C2: abut_ex is logically abutting ex,
		 * - C3: abut_ex is physically abutting ex,
		 * - C4: abut_ex can receive the additional blocks without
		 *   overflowing the (initialized) length limit.
		 */
		if ((!ext4_ext_is_unwritten(abut_ex)) &&		/*C1*/
			((prev_lblk + prev_len) == ee_block) &&		/*C2*/
			((prev_pblk + prev_len) == ee_pblk) &&		/*C3*/
			(prev_len < (EXT_INIT_MAX_LEN - map_len))) {	/*C4*/
			err = ext4_ext_get_access(handle, inode, path + depth);
			if (err)
				goto out;

			trace_ext4_ext_convert_to_initialized_fastpath(inode,
				map, ex, abut_ex);

			/* Shift the start of ex by 'map_len' blocks */
			ex->ee_block = cpu_to_le32(ee_block + map_len);
			ext4_ext_store_pblock(ex, ee_pblk + map_len);
			ex->ee_len = cpu_to_le16(ee_len - map_len);
			ext4_ext_mark_unwritten(ex); /* Restore the flag */

			/* Extend abut_ex by 'map_len' blocks */
			abut_ex->ee_len = cpu_to_le16(prev_len + map_len);

			/* Result: number of initialized blocks past m_lblk */
			allocated = map_len;
		}
	} else if (((map->m_lblk + map_len) == (ee_block + ee_len)) &&
		   (map_len < ee_len) &&	/*L1*/
		   ex < EXT_LAST_EXTENT(eh)) {	/*L2*/
		/* See if we can merge right */
		ext4_lblk_t next_lblk;
		ext4_fsblk_t next_pblk, ee_pblk;
		unsigned int next_len;

		abut_ex = ex + 1;
		next_lblk = le32_to_cpu(abut_ex->ee_block);
		next_len = ext4_ext_get_actual_len(abut_ex);
		next_pblk = ext4_ext_pblock(abut_ex);
		ee_pblk = ext4_ext_pblock(ex);

		/*
		 * A transfer of blocks from 'ex' to 'abut_ex' is allowed
		 * upon those conditions:
		 * - C1: abut_ex is initialized,
		 * - C2: abut_ex is logically abutting ex,
		 * - C3: abut_ex is physically abutting ex,
		 * - C4: abut_ex can receive the additional blocks without
		 *   overflowing the (initialized) length limit.
		 */
		if ((!ext4_ext_is_unwritten(abut_ex)) &&		/*C1*/
		    ((map->m_lblk + map_len) == next_lblk) &&		/*C2*/
		    ((ee_pblk + ee_len) == next_pblk) &&		/*C3*/
		    (next_len < (EXT_INIT_MAX_LEN - map_len))) {	/*C4*/
			err = ext4_ext_get_access(handle, inode, path + depth);
			if (err)
				goto out;

			trace_ext4_ext_convert_to_initialized_fastpath(inode,
				map, ex, abut_ex);

			/* Shift the start of abut_ex by 'map_len' blocks */
			abut_ex->ee_block = cpu_to_le32(next_lblk - map_len);
			ext4_ext_store_pblock(abut_ex, next_pblk - map_len);
			ex->ee_len = cpu_to_le16(ee_len - map_len);
			ext4_ext_mark_unwritten(ex); /* Restore the flag */

			/* Extend abut_ex by 'map_len' blocks */
			abut_ex->ee_len = cpu_to_le16(next_len + map_len);

			/* Result: number of initialized blocks past m_lblk */
			allocated = map_len;
		}
	}
	if (allocated) {
		/* Mark the block containing both extents as dirty */
		ext4_ext_dirty(handle, inode, path + depth);

		/* Update path to point to the right extent */
		path[depth].p_ext = abut_ex;
		goto out;
	} else
		allocated = ee_len - (map->m_lblk - ee_block);

	WARN_ON(map->m_lblk < ee_block);
	/*
	 * It is safe to convert extent to initialized via explicit
	 * zeroout only if extent is fully inside i_size or new_size.
	 */
	split_flag |= ee_block + ee_len <= eof_block ? EXT4_EXT_MAY_ZEROOUT : 0;

	if (EXT4_EXT_MAY_ZEROOUT & split_flag)
		max_zeroout = sbi->s_extent_max_zeroout_kb >>
			(inode->i_sb->s_blocksize_bits - 10);

	/* If extent is less than s_max_zeroout_kb, zeroout directly */
	if (max_zeroout && (ee_len <= max_zeroout)) {
		err = ext4_ext_zeroout(inode, ex);
		if (err)
			goto out;
		zero_ex.ee_block = ex->ee_block;
		zero_ex.ee_len = cpu_to_le16(ext4_ext_get_actual_len(ex));
		ext4_ext_store_pblock(&zero_ex, ext4_ext_pblock(ex));

		err = ext4_ext_get_access(handle, inode, path + depth);
		if (err)
			goto out;
		ext4_ext_mark_initialized(ex);
		ext4_ext_try_to_merge(handle, inode, path, ex);
		err = ext4_ext_dirty(handle, inode, path + path->p_depth);
		goto out;
	}

	/*
	 * four cases:
	 * 1. split the extent into three extents.
	 * 2. split the extent into two extents, zeroout the first half.
	 * 3. split the extent into two extents, zeroout the second half.
	 * 4. split the extent into two extents with out zeroout.
	 */
	split_map.m_lblk = map->m_lblk;
	split_map.m_len = map->m_len;

	if (max_zeroout && (allocated > map->m_len)) {
		if (allocated <= max_zeroout) {
			/* case 3 */
			zero_ex.ee_block =
					 cpu_to_le32(map->m_lblk);
			zero_ex.ee_len = cpu_to_le16(allocated);
			ext4_ext_store_pblock(&zero_ex,
				ext4_ext_pblock(ex) + map->m_lblk - ee_block);
			err = ext4_ext_zeroout(inode, &zero_ex);
			if (err)
				goto out;
			split_map.m_lblk = map->m_lblk;
			split_map.m_len = allocated;
		} else if (map->m_lblk - ee_block + map->m_len < max_zeroout) {
			/* case 2 */
			if (map->m_lblk != ee_block) {
				zero_ex.ee_block = ex->ee_block;
				zero_ex.ee_len = cpu_to_le16(map->m_lblk -
							ee_block);
				ext4_ext_store_pblock(&zero_ex,
						      ext4_ext_pblock(ex));
				err = ext4_ext_zeroout(inode, &zero_ex);
				if (err)
					goto out;
			}

			split_map.m_lblk = ee_block;
			split_map.m_len = map->m_lblk - ee_block + map->m_len;
			allocated = map->m_len;
		}
	}

	err = ext4_split_extent(handle, inode, ppath, &split_map, split_flag,
				flags);
	if (err > 0)
		err = 0;
out:
	/* If we have gotten a failure, don't zero out status tree */
	if (!err)
		err = ext4_zeroout_es(inode, &zero_ex);
	return err ? err : allocated;
}

/*
 * This function is called by ext4_ext_map_blocks() from
 * ext4_get_blocks_dio_write() when DIO to write
 * to an unwritten extent.
 *
 * Writing to an unwritten extent may result in splitting the unwritten
 * extent into multiple initialized/unwritten extents (up to three)
 * There are three possibilities:
 *   a> There is no split required: Entire extent should be unwritten
 *   b> Splits in two extents: Write is happening at either end of the extent
 *   c> Splits in three extents: Somone is writing in middle of the extent
 *
 * This works the same way in the case of initialized -> unwritten conversion.
 *
 * One of more index blocks maybe needed if the extent tree grow after
 * the unwritten extent split. To prevent ENOSPC occur at the IO
 * complete, we need to split the unwritten extent before DIO submit
 * the IO. The unwritten extent called at this time will be split
 * into three unwritten extent(at most). After IO complete, the part
 * being filled will be convert to initialized by the end_io callback function
 * via ext4_convert_unwritten_extents().
 *
 * Returns the size of unwritten extent to be written on success.
 */
static int ext4_split_convert_extents(handle_t *handle,
					struct inode *inode,
					struct ext4_map_blocks *map,
					struct ext4_ext_path **ppath,
					int flags)
{
	struct ext4_ext_path *path = *ppath;
	ext4_lblk_t eof_block;
	ext4_lblk_t ee_block;
	struct ext4_extent *ex;
	unsigned int ee_len;
	int split_flag = 0, depth;

	ext_debug("%s: inode %lu, logical block %llu, max_blocks %u\n",
		  __func__, inode->i_ino,
		  (unsigned long long)map->m_lblk, map->m_len);

	eof_block = (inode->i_size + inode->i_sb->s_blocksize - 1) >>
		inode->i_sb->s_blocksize_bits;
	if (eof_block < map->m_lblk + map->m_len)
		eof_block = map->m_lblk + map->m_len;
	/*
	 * It is safe to convert extent to initialized via explicit
	 * zeroout only if extent is fully insde i_size or new_size.
	 */
	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);

	/* Convert to unwritten */
	if (flags & EXT4_GET_BLOCKS_CONVERT_UNWRITTEN) {
		split_flag |= EXT4_EXT_DATA_VALID1;
	/* Convert to initialized */
	} else if (flags & EXT4_GET_BLOCKS_CONVERT) {
		split_flag |= ee_block + ee_len <= eof_block ?
			      EXT4_EXT_MAY_ZEROOUT : 0;
		split_flag |= (EXT4_EXT_MARK_UNWRIT2 | EXT4_EXT_DATA_VALID2);
	}
	flags |= EXT4_GET_BLOCKS_PRE_IO;
	return ext4_split_extent(handle, inode, ppath, map, split_flag, flags);
}

static int ext4_convert_unwritten_extents_endio(handle_t *handle,
						struct inode *inode,
						struct ext4_map_blocks *map,
						struct ext4_ext_path **ppath)
{
	struct ext4_ext_path *path = *ppath;
	struct ext4_extent *ex;
	ext4_lblk_t ee_block;
	unsigned int ee_len;
	int depth;
	int err = 0;

	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);

	ext_debug("ext4_convert_unwritten_extents_endio: inode %lu, logical"
		"block %llu, max_blocks %u\n", inode->i_ino,
		  (unsigned long long)ee_block, ee_len);

	/* If extent is larger than requested it is a clear sign that we still
	 * have some extent state machine issues left. So extent_split is still
	 * required.
	 * TODO: Once all related issues will be fixed this situation should be
	 * illegal.
	 */
	if (ee_block != map->m_lblk || ee_len > map->m_len) {
#ifdef EXT4_DEBUG
		ext4_warning("Inode (%ld) finished: extent logical block %llu,"
			     " len %u; IO logical block %llu, len %u\n",
			     inode->i_ino, (unsigned long long)ee_block, ee_len,
			     (unsigned long long)map->m_lblk, map->m_len);
#endif
		err = ext4_split_convert_extents(handle, inode, map, ppath,
						 EXT4_GET_BLOCKS_CONVERT);
		if (err < 0)
			return err;
		path = ext4_find_extent(inode, map->m_lblk, ppath, 0);
		if (IS_ERR(path))
			return PTR_ERR(path);
		depth = ext_depth(inode);
		ex = path[depth].p_ext;
	}

	err = ext4_ext_get_access(handle, inode, path + depth);
	if (err)
		goto out;
	/* first mark the extent as initialized */
	ext4_ext_mark_initialized(ex);

	/* note: ext4_ext_correct_indexes() isn't needed here because
	 * borders are not changed
	 */
	ext4_ext_try_to_merge(handle, inode, path, ex);

	/* Mark modified extent as dirty */
	err = ext4_ext_dirty(handle, inode, path + path->p_depth);
out:
	ext4_ext_show_leaf(inode, path);
	return err;
}

static void unmap_underlying_metadata_blocks(struct block_device *bdev,
			sector_t block, int count)
{
	int i;
	for (i = 0; i < count; i++)
                unmap_underlying_metadata(bdev, block + i);
}

/*
 * Handle EOFBLOCKS_FL flag, clearing it if necessary
 */
static int check_eofblocks_fl(handle_t *handle, struct inode *inode,
			      ext4_lblk_t lblk,
			      struct ext4_ext_path *path,
			      unsigned int len)
{
	int i, depth;
	struct ext4_extent_header *eh;
	struct ext4_extent *last_ex;

	if (!ext4_test_inode_flag(inode, EXT4_INODE_EOFBLOCKS))
		return 0;

	depth = ext_depth(inode);
	eh = path[depth].p_hdr;

	/*
	 * We're going to remove EOFBLOCKS_FL entirely in future so we
	 * do not care for this case anymore. Simply remove the flag
	 * if there are no extents.
	 */
	if (unlikely(!eh->eh_entries))
		goto out;
	last_ex = EXT_LAST_EXTENT(eh);
	/*
	 * We should clear the EOFBLOCKS_FL flag if we are writing the
	 * last block in the last extent in the file.  We test this by
	 * first checking to see if the caller to
	 * ext4_ext_get_blocks() was interested in the last block (or
	 * a block beyond the last block) in the current extent.  If
	 * this turns out to be false, we can bail out from this
	 * function immediately.
	 */
	if (lblk + len < le32_to_cpu(last_ex->ee_block) +
	    ext4_ext_get_actual_len(last_ex))
		return 0;
	/*
	 * If the caller does appear to be planning to write at or
	 * beyond the end of the current extent, we then test to see
	 * if the current extent is the last extent in the file, by
	 * checking to make sure it was reached via the rightmost node
	 * at each level of the tree.
	 */
	for (i = depth-1; i >= 0; i--)
		if (path[i].p_idx != EXT_LAST_INDEX(path[i].p_hdr))
			return 0;
out:
	ext4_clear_inode_flag(inode, EXT4_INODE_EOFBLOCKS);
	return ext4_mark_inode_dirty(handle, inode);
}

/**
 * ext4_find_delalloc_range: find delayed allocated block in the given range.
 *
 * Return 1 if there is a delalloc block in the range, otherwise 0.
 */
int ext4_find_delalloc_range(struct inode *inode,
			     ext4_lblk_t lblk_start,
			     ext4_lblk_t lblk_end)
{
	struct extent_status es;

	ext4_es_find_delayed_extent_range(inode, lblk_start, lblk_end, &es);
	if (es.es_len == 0)
		return 0; /* there is no delay extent in this tree */
	else if (es.es_lblk <= lblk_start &&
		 lblk_start < es.es_lblk + es.es_len)
		return 1;
	else if (lblk_start <= es.es_lblk && es.es_lblk <= lblk_end)
		return 1;
	else
		return 0;
}

int ext4_find_delalloc_cluster(struct inode *inode, ext4_lblk_t lblk)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	ext4_lblk_t lblk_start, lblk_end;
	lblk_start = EXT4_LBLK_CMASK(sbi, lblk);
	lblk_end = lblk_start + sbi->s_cluster_ratio - 1;

	return ext4_find_delalloc_range(inode, lblk_start, lblk_end);
}

/**
 * Determines how many complete clusters (out of those specified by the 'map')
 * are under delalloc and were reserved quota for.
 * This function is called when we are writing out the blocks that were
 * originally written with their allocation delayed, but then the space was
 * allocated using fallocate() before the delayed allocation could be resolved.
 * The cases to look for are:
 * ('=' indicated delayed allocated blocks
 *  '-' indicates non-delayed allocated blocks)
 * (a) partial clusters towards beginning and/or end outside of allocated range
 *     are not delalloc'ed.
 *	Ex:
 *	|----c---=|====c====|====c====|===-c----|
 *	         |++++++ allocated ++++++|
 *	==> 4 complete clusters in above example
 *
 * (b) partial cluster (outside of allocated range) towards either end is
 *     marked for delayed allocation. In this case, we will exclude that
 *     cluster.
 *	Ex:
 *	|----====c========|========c========|
 *	     |++++++ allocated ++++++|
 *	==> 1 complete clusters in above example
 *
 *	Ex:
 *	|================c================|
 *            |++++++ allocated ++++++|
 *	==> 0 complete clusters in above example
 *
 * The ext4_da_update_reserve_space will be called only if we
 * determine here that there were some "entire" clusters that span
 * this 'allocated' range.
 * In the non-bigalloc case, this function will just end up returning num_blks
 * without ever calling ext4_find_delalloc_range.
 */
static unsigned int
get_reserved_cluster_alloc(struct inode *inode, ext4_lblk_t lblk_start,
			   unsigned int num_blks)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	ext4_lblk_t alloc_cluster_start, alloc_cluster_end;
	ext4_lblk_t lblk_from, lblk_to, c_offset;
	unsigned int allocated_clusters = 0;

	alloc_cluster_start = EXT4_B2C(sbi, lblk_start);
	alloc_cluster_end = EXT4_B2C(sbi, lblk_start + num_blks - 1);

	/* max possible clusters for this allocation */
	allocated_clusters = alloc_cluster_end - alloc_cluster_start + 1;

	trace_ext4_get_reserved_cluster_alloc(inode, lblk_start, num_blks);

	/* Check towards left side */
	c_offset = EXT4_LBLK_COFF(sbi, lblk_start);
	if (c_offset) {
		lblk_from = EXT4_LBLK_CMASK(sbi, lblk_start);
		lblk_to = lblk_from + c_offset - 1;

		if (ext4_find_delalloc_range(inode, lblk_from, lblk_to))
			allocated_clusters--;
	}

	/* Now check towards right. */
	c_offset = EXT4_LBLK_COFF(sbi, lblk_start + num_blks);
	if (allocated_clusters && c_offset) {
		lblk_from = lblk_start + num_blks;
		lblk_to = lblk_from + (sbi->s_cluster_ratio - c_offset) - 1;

		if (ext4_find_delalloc_range(inode, lblk_from, lblk_to))
			allocated_clusters--;
	}

	return allocated_clusters;
}

static int
convert_initialized_extent(handle_t *handle, struct inode *inode,
			   struct ext4_map_blocks *map,
			   struct ext4_ext_path **ppath, int flags,
			   unsigned int allocated, ext4_fsblk_t newblock)
{
	struct ext4_ext_path *path = *ppath;
	struct ext4_extent *ex;
	ext4_lblk_t ee_block;
	unsigned int ee_len;
	int depth;
	int err = 0;

	/*
	 * Make sure that the extent is no bigger than we support with
	 * unwritten extent
	 */
	if (map->m_len > EXT_UNWRITTEN_MAX_LEN)
		map->m_len = EXT_UNWRITTEN_MAX_LEN / 2;

	depth = ext_depth(inode);
	ex = path[depth].p_ext;
	ee_block = le32_to_cpu(ex->ee_block);
	ee_len = ext4_ext_get_actual_len(ex);

	ext_debug("%s: inode %lu, logical"
		"block %llu, max_blocks %u\n", __func__, inode->i_ino,
		  (unsigned long long)ee_block, ee_len);

	if (ee_block != map->m_lblk || ee_len > map->m_len) {
		err = ext4_split_convert_extents(handle, inode, map, ppath,
				EXT4_GET_BLOCKS_CONVERT_UNWRITTEN);
		if (err < 0)
			return err;
		path = ext4_find_extent(inode, map->m_lblk, ppath, 0);
		if (IS_ERR(path))
			return PTR_ERR(path);
		depth = ext_depth(inode);
		ex = path[depth].p_ext;
		if (!ex) {
			EXT4_ERROR_INODE(inode, "unexpected hole at %lu",
					 (unsigned long) map->m_lblk);
			return -EIO;
		}
	}

	err = ext4_ext_get_access(handle, inode, path + depth);
	if (err)
		return err;
	/* first mark the extent as unwritten */
	ext4_ext_mark_unwritten(ex);

	/* note: ext4_ext_correct_indexes() isn't needed here because
	 * borders are not changed
	 */
	ext4_ext_try_to_merge(handle, inode, path, ex);

	/* Mark modified extent as dirty */
	err = ext4_ext_dirty(handle, inode, path + path->p_depth);
	if (err)
		return err;
	ext4_ext_show_leaf(inode, path);

	ext4_update_inode_fsync_trans(handle, inode, 1);
	err = check_eofblocks_fl(handle, inode, map->m_lblk, path, map->m_len);
	if (err)
		return err;
	map->m_flags |= EXT4_MAP_UNWRITTEN;
	if (allocated > map->m_len)
		allocated = map->m_len;
	map->m_len = allocated;
	return allocated;
}

static int
ext4_ext_handle_unwritten_extents(handle_t *handle, struct inode *inode,
			struct ext4_map_blocks *map,
			struct ext4_ext_path **ppath, int flags,
			unsigned int allocated, ext4_fsblk_t newblock)
{
	struct ext4_ext_path *path = *ppath;
	int ret = 0;
	int err = 0;
	ext4_io_end_t *io = ext4_inode_aio(inode);

	ext_debug("ext4_ext_handle_unwritten_extents: inode %lu, logical "
		  "block %llu, max_blocks %u, flags %x, allocated %u\n",
		  inode->i_ino, (unsigned long long)map->m_lblk, map->m_len,
		  flags, allocated);
	ext4_ext_show_leaf(inode, path);

	/*
	 * When writing into unwritten space, we should not fail to
	 * allocate metadata blocks for the new extent block if needed.
	 */
	flags |= EXT4_GET_BLOCKS_METADATA_NOFAIL;

	trace_ext4_ext_handle_unwritten_extents(inode, map, flags,
						    allocated, newblock);

	/* get_block() before submit the IO, split the extent */
	if (flags & EXT4_GET_BLOCKS_PRE_IO) {
		ret = ext4_split_convert_extents(handle, inode, map, ppath,
					 flags | EXT4_GET_BLOCKS_CONVERT);
		if (ret <= 0)
			goto out;
		/*
		 * Flag the inode(non aio case) or end_io struct (aio case)
		 * that this IO needs to conversion to written when IO is
		 * completed
		 */
		if (io)
			ext4_set_io_unwritten_flag(inode, io);
		else
			ext4_set_inode_state(inode, EXT4_STATE_DIO_UNWRITTEN);
		map->m_flags |= EXT4_MAP_UNWRITTEN;
		goto out;
	}
	/* IO end_io complete, convert the filled extent to written */
	if (flags & EXT4_GET_BLOCKS_CONVERT) {
		ret = ext4_convert_unwritten_extents_endio(handle, inode, map,
							   ppath);
		if (ret >= 0) {
			ext4_update_inode_fsync_trans(handle, inode, 1);
			err = check_eofblocks_fl(handle, inode, map->m_lblk,
						 path, map->m_len);
		} else
			err = ret;
		map->m_flags |= EXT4_MAP_MAPPED;
		map->m_pblk = newblock;
		if (allocated > map->m_len)
			allocated = map->m_len;
		map->m_len = allocated;
		goto out2;
	}
	/* buffered IO case */
	/*
	 * repeat fallocate creation request
	 * we already have an unwritten extent
	 */
	if (flags & EXT4_GET_BLOCKS_UNWRIT_EXT) {
		map->m_flags |= EXT4_MAP_UNWRITTEN;
		goto map_out;
	}

	/* buffered READ or buffered write_begin() lookup */
	if ((flags & EXT4_GET_BLOCKS_CREATE) == 0) {
		/*
		 * We have blocks reserved already.  We
		 * return allocated blocks so that delalloc
		 * won't do block reservation for us.  But
		 * the buffer head will be unmapped so that
		 * a read from the block returns 0s.
		 */
		map->m_flags |= EXT4_MAP_UNWRITTEN;
		goto out1;
	}

	/* buffered write, writepage time, convert*/
	ret = ext4_ext_convert_to_initialized(handle, inode, map, ppath, flags);
	if (ret >= 0)
		ext4_update_inode_fsync_trans(handle, inode, 1);
out:
	if (ret <= 0) {
		err = ret;
		goto out2;
	} else
		allocated = ret;
	map->m_flags |= EXT4_MAP_NEW;
	/*
	 * if we allocated more blocks than requested
	 * we need to make sure we unmap the extra block
	 * allocated. The actual needed block will get
	 * unmapped later when we find the buffer_head marked
	 * new.
	 */
	if (allocated > map->m_len) {
		unmap_underlying_metadata_blocks(inode->i_sb->s_bdev,
					newblock + map->m_len,
					allocated - map->m_len);
		allocated = map->m_len;
	}
	map->m_len = allocated;

	/*
	 * If we have done fallocate with the offset that is already
	 * delayed allocated, we would have block reservation
	 * and quota reservation done in the delayed write path.
	 * But fallocate would have already updated quota and block
	 * count for this offset. So cancel these reservation
	 */
	if (flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE) {
		unsigned int reserved_clusters;
		reserved_clusters = get_reserved_cluster_alloc(inode,
				map->m_lblk, map->m_len);
		if (reserved_clusters)
			ext4_da_update_reserve_space(inode,
						     reserved_clusters,
						     0);
	}

map_out:
	map->m_flags |= EXT4_MAP_MAPPED;
	if ((flags & EXT4_GET_BLOCKS_KEEP_SIZE) == 0) {
		err = check_eofblocks_fl(handle, inode, map->m_lblk, path,
					 map->m_len);
		if (err < 0)
			goto out2;
	}
out1:
	if (allocated > map->m_len)
		allocated = map->m_len;
	ext4_ext_show_leaf(inode, path);
	map->m_pblk = newblock;
	map->m_len = allocated;
out2:
	return err ? err : allocated;
}

/*
 * get_implied_cluster_alloc - check to see if the requested
 * allocation (in the map structure) overlaps with a cluster already
 * allocated in an extent.
 *	@sb	The filesystem superblock structure
 *	@map	The requested lblk->pblk mapping
 *	@ex	The extent structure which might contain an implied
 *			cluster allocation
 *
 * This function is called by ext4_ext_map_blocks() after we failed to
 * find blocks that were already in the inode's extent tree.  Hence,
 * we know that the beginning of the requested region cannot overlap
 * the extent from the inode's extent tree.  There are three cases we
 * want to catch.  The first is this case:
 *
 *		 |--- cluster # N--|
 *    |--- extent ---|	|---- requested region ---|
 *			|==========|
 *
 * The second case that we need to test for is this one:
 *
 *   |--------- cluster # N ----------------|
 *	   |--- requested region --|   |------- extent ----|
 *	   |=======================|
 *
 * The third case is when the requested region lies between two extents
 * within the same cluster:
 *          |------------- cluster # N-------------|
 * |----- ex -----|                  |---- ex_right ----|
 *                  |------ requested region ------|
 *                  |================|
 *
 * In each of the above cases, we need to set the map->m_pblk and
 * map->m_len so it corresponds to the return the extent labelled as
 * "|====|" from cluster #N, since it is already in use for data in
 * cluster EXT4_B2C(sbi, map->m_lblk).	We will then return 1 to
 * signal to ext4_ext_map_blocks() that map->m_pblk should be treated
 * as a new "allocated" block region.  Otherwise, we will return 0 and
 * ext4_ext_map_blocks() will then allocate one or more new clusters
 * by calling ext4_mb_new_blocks().
 */
static int get_implied_cluster_alloc(struct super_block *sb,
				     struct ext4_map_blocks *map,
				     struct ext4_extent *ex,
				     struct ext4_ext_path *path)
{
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	ext4_lblk_t c_offset = EXT4_LBLK_COFF(sbi, map->m_lblk);
	ext4_lblk_t ex_cluster_start, ex_cluster_end;
	ext4_lblk_t rr_cluster_start;
	ext4_lblk_t ee_block = le32_to_cpu(ex->ee_block);
	ext4_fsblk_t ee_start = ext4_ext_pblock(ex);
	unsigned short ee_len = ext4_ext_get_actual_len(ex);

	/* The extent passed in that we are trying to match */
	ex_cluster_start = EXT4_B2C(sbi, ee_block);
	ex_cluster_end = EXT4_B2C(sbi, ee_block + ee_len - 1);

	/* The requested region passed into ext4_map_blocks() */
	rr_cluster_start = EXT4_B2C(sbi, map->m_lblk);

	if ((rr_cluster_start == ex_cluster_end) ||
	    (rr_cluster_start == ex_cluster_start)) {
		if (rr_cluster_start == ex_cluster_end)
			ee_start += ee_len - 1;
		map->m_pblk = EXT4_PBLK_CMASK(sbi, ee_start) + c_offset;
		map->m_len = min(map->m_len,
				 (unsigned) sbi->s_cluster_ratio - c_offset);
		/*
		 * Check for and handle this case:
		 *
		 *   |--------- cluster # N-------------|
		 *		       |------- extent ----|
		 *	   |--- requested region ---|
		 *	   |===========|
		 */

		if (map->m_lblk < ee_block)
			map->m_len = min(map->m_len, ee_block - map->m_lblk);

		/*
		 * Check for the case where there is already another allocated
		 * block to the right of 'ex' but before the end of the cluster.
		 *
		 *          |------------- cluster # N-------------|
		 * |----- ex -----|                  |---- ex_right ----|
		 *                  |------ requested region ------|
		 *                  |================|
		 */
		if (map->m_lblk > ee_block) {
			ext4_lblk_t next = ext4_ext_next_allocated_block(path);
			map->m_len = min(map->m_len, next - map->m_lblk);
		}

		trace_ext4_get_implied_cluster_alloc_exit(sb, map, 1);
		return 1;
	}

	trace_ext4_get_implied_cluster_alloc_exit(sb, map, 0);
	return 0;
}


/*
 * Block allocation/map/preallocation routine for extents based files
 *
 *
 * Need to be called with
 * down_read(&EXT4_I(inode)->i_data_sem) if not allocating file system block
 * (ie, create is zero). Otherwise down_write(&EXT4_I(inode)->i_data_sem)
 *
 * return > 0, number of of blocks already mapped/allocated
 *          if create == 0 and these are pre-allocated blocks
 *          	buffer head is unmapped
 *          otherwise blocks are mapped
 *
 * return = 0, if plain look up failed (blocks have not been allocated)
 *          buffer head is unmapped
 *
 * return < 0, error case.
 */
int ext4_ext_map_blocks(handle_t *handle, struct inode *inode,
			struct ext4_map_blocks *map, int flags)
{
	struct ext4_ext_path *path = NULL;
	struct ext4_extent newex, *ex, *ex2;
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	ext4_fsblk_t newblock = 0;
	int free_on_err = 0, err = 0, depth, ret;
	unsigned int allocated = 0, offset = 0;
	unsigned int allocated_clusters = 0;
	struct ext4_allocation_request ar;
	ext4_io_end_t *io = ext4_inode_aio(inode);
	ext4_lblk_t cluster_offset;
	int set_unwritten = 0;
	bool map_from_cluster = false;

	ext_debug("blocks %u/%u requested for inode %lu\n",
		  map->m_lblk, map->m_len, inode->i_ino);
	trace_ext4_ext_map_blocks_enter(inode, map->m_lblk, map->m_len, flags);

	/* find extent for this block */
	path = ext4_find_extent(inode, map->m_lblk, NULL, 0);
	if (IS_ERR(path)) {
		err = PTR_ERR(path);
		path = NULL;
		goto out2;
	}

	depth = ext_depth(inode);

	/*
	 * consistent leaf must not be empty;
	 * this situation is possible, though, _during_ tree modification;
	 * this is why assert can't be put in ext4_find_extent()
	 */
	if (unlikely(path[depth].p_ext == NULL && depth != 0)) {
		EXT4_ERROR_INODE(inode, "bad extent address "
				 "lblock: %lu, depth: %d pblock %lld",
				 (unsigned long) map->m_lblk, depth,
				 path[depth].p_block);
		err = -EIO;
		goto out2;
	}

	ex = path[depth].p_ext;
	if (ex) {
		ext4_lblk_t ee_block = le32_to_cpu(ex->ee_block);
		ext4_fsblk_t ee_start = ext4_ext_pblock(ex);
		unsigned short ee_len;


		/*
		 * unwritten extents are treated as holes, except that
		 * we split out initialized portions during a write.
		 */
		ee_len = ext4_ext_get_actual_len(ex);

		trace_ext4_ext_show_extent(inode, ee_block, ee_start, ee_len);

		/* if found extent covers block, simply return it */
		if (in_range(map->m_lblk, ee_block, ee_len)) {
			newblock = map->m_lblk - ee_block + ee_start;
			/* number of remaining blocks in the extent */
			allocated = ee_len - (map->m_lblk - ee_block);
			ext_debug("%u fit into %u:%d -> %llu\n", map->m_lblk,
				  ee_block, ee_len, newblock);

			/*
			 * If the extent is initialized check whether the
			 * caller wants to convert it to unwritten.
			 */
			if ((!ext4_ext_is_unwritten(ex)) &&
			    (flags & EXT4_GET_BLOCKS_CONVERT_UNWRITTEN)) {
				allocated = convert_initialized_extent(
						handle, inode, map, &path,
						flags, allocated, newblock);
				goto out2;
			} else if (!ext4_ext_is_unwritten(ex))
				goto out;

			ret = ext4_ext_handle_unwritten_extents(
				handle, inode, map, &path, flags,
				allocated, newblock);
			if (ret < 0)
				err = ret;
			else
				allocated = ret;
			goto out2;
		}
	}

	/*
	 * requested block isn't allocated yet;
	 * we couldn't try to create block if create flag is zero
	 */
	if ((flags & EXT4_GET_BLOCKS_CREATE) == 0) {
		/*
		 * put just found gap into cache to speed up
		 * subsequent requests
		 */
		ext4_ext_put_gap_in_cache(inode, path, map->m_lblk);
		goto out2;
	}

	/*
	 * Okay, we need to do block allocation.
	 */
	newex.ee_block = cpu_to_le32(map->m_lblk);
	cluster_offset = EXT4_LBLK_COFF(sbi, map->m_lblk);

	/*
	 * If we are doing bigalloc, check to see if the extent returned
	 * by ext4_find_extent() implies a cluster we can use.
	 */
	if (cluster_offset && ex &&
	    get_implied_cluster_alloc(inode->i_sb, map, ex, path)) {
		ar.len = allocated = map->m_len;
		newblock = map->m_pblk;
		map_from_cluster = true;
		goto got_allocated_blocks;
	}

	/* find neighbour allocated blocks */
	ar.lleft = map->m_lblk;
	err = ext4_ext_search_left(inode, path, &ar.lleft, &ar.pleft);
	if (err)
		goto out2;
	ar.lright = map->m_lblk;
	ex2 = NULL;
	err = ext4_ext_search_right(inode, path, &ar.lright, &ar.pright, &ex2);
	if (err)
		goto out2;

	/* Check if the extent after searching to the right implies a
	 * cluster we can use. */
	if ((sbi->s_cluster_ratio > 1) && ex2 &&
	    get_implied_cluster_alloc(inode->i_sb, map, ex2, path)) {
		ar.len = allocated = map->m_len;
		newblock = map->m_pblk;
		map_from_cluster = true;
		goto got_allocated_blocks;
	}

	/*
	 * See if request is beyond maximum number of blocks we can have in
	 * a single extent. For an initialized extent this limit is
	 * EXT_INIT_MAX_LEN and for an unwritten extent this limit is
	 * EXT_UNWRITTEN_MAX_LEN.
	 */
	if (map->m_len > EXT_INIT_MAX_LEN &&
	    !(flags & EXT4_GET_BLOCKS_UNWRIT_EXT))
		map->m_len = EXT_INIT_MAX_LEN;
	else if (map->m_len > EXT_UNWRITTEN_MAX_LEN &&
		 (flags & EXT4_GET_BLOCKS_UNWRIT_EXT))
		map->m_len = EXT_UNWRITTEN_MAX_LEN;

	/* Check if we can really insert (m_lblk)::(m_lblk + m_len) extent */
	newex.ee_len = cpu_to_le16(map->m_len);
	err = ext4_ext_check_overlap(sbi, inode, &newex, path);
	if (err)
		allocated = ext4_ext_get_actual_len(&newex);
	else
		allocated = map->m_len;

	/* allocate new block */
	ar.inode = inode;
	ar.goal = ext4_ext_find_goal(inode, path, map->m_lblk);
	ar.logical = map->m_lblk;
	/*
	 * We calculate the offset from the beginning of the cluster
	 * for the logical block number, since when we allocate a
	 * physical cluster, the physical block should start at the
	 * same offset from the beginning of the cluster.  This is
	 * needed so that future calls to get_implied_cluster_alloc()
	 * work correctly.
	 */
	offset = EXT4_LBLK_COFF(sbi, map->m_lblk);
	ar.len = EXT4_NUM_B2C(sbi, offset+allocated);
	ar.goal -= offset;
	ar.logical -= offset;
	if (S_ISREG(inode->i_mode))
		ar.flags = EXT4_MB_HINT_DATA;
	else
		/* disable in-core preallocation for non-regular files */
		ar.flags = 0;
	if (flags & EXT4_GET_BLOCKS_NO_NORMALIZE)
		ar.flags |= EXT4_MB_HINT_NOPREALLOC;
	if (flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE)
		ar.flags |= EXT4_MB_DELALLOC_RESERVED;
	newblock = ext4_mb_new_blocks(handle, &ar, &err);
	if (!newblock)
		goto out2;
	ext_debug("allocate new block: goal %llu, found %llu/%u\n",
		  ar.goal, newblock, allocated);
	free_on_err = 1;
	allocated_clusters = ar.len;
	ar.len = EXT4_C2B(sbi, ar.len) - offset;
	if (ar.len > allocated)
		ar.len = allocated;

got_allocated_blocks:
	/* try to insert new extent into found leaf and return */
	ext4_ext_store_pblock(&newex, newblock + offset);
	newex.ee_len = cpu_to_le16(ar.len);
	/* Mark unwritten */
	if (flags & EXT4_GET_BLOCKS_UNWRIT_EXT){
		ext4_ext_mark_unwritten(&newex);
		map->m_flags |= EXT4_MAP_UNWRITTEN;
		/*
		 * io_end structure was created for every IO write to an
		 * unwritten extent. To avoid unnecessary conversion,
		 * here we flag the IO that really needs the conversion.
		 * For non asycn direct IO case, flag the inode state
		 * that we need to perform conversion when IO is done.
		 */
		if (flags & EXT4_GET_BLOCKS_PRE_IO)
			set_unwritten = 1;
	}

	err = 0;
	if ((flags & EXT4_GET_BLOCKS_KEEP_SIZE) == 0)
		err = check_eofblocks_fl(handle, inode, map->m_lblk,
					 path, ar.len);
	if (!err)
		err = ext4_ext_insert_extent(handle, inode, &path,
					     &newex, flags);

	if (!err && set_unwritten) {
		if (io)
			ext4_set_io_unwritten_flag(inode, io);
		else
			ext4_set_inode_state(inode,
					     EXT4_STATE_DIO_UNWRITTEN);
	}

	if (err && free_on_err) {
		int fb_flags = flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE ?
			EXT4_FREE_BLOCKS_NO_QUOT_UPDATE : 0;
		/* free data blocks we just allocated */
		/* not a good idea to call discard here directly,
		 * but otherwise we'd need to call it every free() */
		ext4_discard_preallocations(inode);
		ext4_free_blocks(handle, inode, NULL, newblock,
				 EXT4_C2B(sbi, allocated_clusters), fb_flags);
		goto out2;
	}

	/* previous routine could use block we allocated */
	newblock = ext4_ext_pblock(&newex);
	allocated = ext4_ext_get_actual_len(&newex);
	if (allocated > map->m_len)
		allocated = map->m_len;
	map->m_flags |= EXT4_MAP_NEW;

	/*
	 * Update reserved blocks/metadata blocks after successful
	 * block allocation which had been deferred till now.
	 */
	if (flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE) {
		unsigned int reserved_clusters;
		/*
		 * Check how many clusters we had reserved this allocated range
		 */
		reserved_clusters = get_reserved_cluster_alloc(inode,
						map->m_lblk, allocated);
		if (!map_from_cluster) {
			BUG_ON(allocated_clusters < reserved_clusters);
			if (reserved_clusters < allocated_clusters) {
				struct ext4_inode_info *ei = EXT4_I(inode);
				int reservation = allocated_clusters -
						  reserved_clusters;
				/*
				 * It seems we claimed few clusters outside of
				 * the range of this allocation. We should give
				 * it back to the reservation pool. This can
				 * happen in the following case:
				 *
				 * * Suppose s_cluster_ratio is 4 (i.e., each
				 *   cluster has 4 blocks. Thus, the clusters
				 *   are [0-3],[4-7],[8-11]...
				 * * First comes delayed allocation write for
				 *   logical blocks 10 & 11. Since there were no
				 *   previous delayed allocated blocks in the
				 *   range [8-11], we would reserve 1 cluster
				 *   for this write.
				 * * Next comes write for logical blocks 3 to 8.
				 *   In this case, we will reserve 2 clusters
				 *   (for [0-3] and [4-7]; and not for [8-11] as
				 *   that range has a delayed allocated blocks.
				 *   Thus total reserved clusters now becomes 3.
				 * * Now, during the delayed allocation writeout
				 *   time, we will first write blocks [3-8] and
				 *   allocate 3 clusters for writing these
				 *   blocks. Also, we would claim all these
				 *   three clusters above.
				 * * Now when we come here to writeout the
				 *   blocks [10-11], we would expect to claim
				 *   the reservation of 1 cluster we had made
				 *   (and we would claim it since there are no
				 *   more delayed allocated blocks in the range
				 *   [8-11]. But our reserved cluster count had
				 *   already gone to 0.
				 *
				 *   Thus, at the step 4 above when we determine
				 *   that there are still some unwritten delayed
				 *   allocated blocks outside of our current
				 *   block range, we should increment the
				 *   reserved clusters count so that when the
				 *   remaining blocks finally gets written, we
				 *   could claim them.
				 */
				dquot_reserve_block(inode,
						EXT4_C2B(sbi, reservation));
				spin_lock(&ei->i_block_reservation_lock);
				ei->i_reserved_data_blocks += reservation;
				spin_unlock(&ei->i_block_reservation_lock);
			}
			/*
			 * We will claim quota for all newly allocated blocks.
			 * We're updating the reserved space *after* the
			 * correction above so we do not accidentally free
			 * all the metadata reservation because we might
			 * actually need it later on.
			 */
			ext4_da_update_reserve_space(inode, allocated_clusters,
							1);
		}
	}

	/*
	 * Cache the extent and update transaction to commit on fdatasync only
	 * when it is _not_ an unwritten extent.
	 */
	if ((flags & EXT4_GET_BLOCKS_UNWRIT_EXT) == 0)
		ext4_update_inode_fsync_trans(handle, inode, 1);
	else
		ext4_update_inode_fsync_trans(handle, inode, 0);
out:
	if (allocated > map->m_len)
		allocated = map->m_len;
	ext4_ext_show_leaf(inode, path);
	map->m_flags |= EXT4_MAP_MAPPED;
	map->m_pblk = newblock;
	map->m_len = allocated;
out2:
	ext4_ext_drop_refs(path);
	kfree(path);

	trace_ext4_ext_map_blocks_exit(inode, flags, map,
				       err ? err : allocated);
	return err ? err : allocated;
}

void ext4_ext_truncate(handle_t *handle, struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	ext4_lblk_t last_block;
	int err = 0;

	/*
	 * TODO: optimization is possible here.
	 * Probably we need not scan at all,
	 * because page truncation is enough.
	 */

	/* we have to know where to truncate from in crash case */
	EXT4_I(inode)->i_disksize = inode->i_size;
	ext4_mark_inode_dirty(handle, inode);

	last_block = (inode->i_size + sb->s_blocksize - 1)
			>> EXT4_BLOCK_SIZE_BITS(sb);
retry:
	err = ext4_es_remove_extent(inode, last_block,
				    EXT_MAX_BLOCKS - last_block);
	if (err == -ENOMEM) {
		cond_resched();
		congestion_wait(BLK_RW_ASYNC, HZ/50);
		goto retry;
	}
	if (err) {
		ext4_std_error(inode->i_sb, err);
		return;
	}
	err = ext4_ext_remove_space(inode, last_block, EXT_MAX_BLOCKS - 1);
	ext4_std_error(inode->i_sb, err);
}

static int ext4_alloc_file_blocks(struct file *file, ext4_lblk_t offset,
				  ext4_lblk_t len, loff_t new_size,
				  int flags, int mode)
{
	struct inode *inode = file_inode(file);
	handle_t *handle;
	int ret = 0;
	int ret2 = 0;
	int retries = 0;
	struct ext4_map_blocks map;
	unsigned int credits;
	loff_t epos;

	map.m_lblk = offset;
	map.m_len = len;
	/*
	 * Don't normalize the request if it can fit in one extent so
	 * that it doesn't get unnecessarily split into multiple
	 * extents.
	 */
	if (len <= EXT_UNWRITTEN_MAX_LEN)
		flags |= EXT4_GET_BLOCKS_NO_NORMALIZE;

	/*
	 * credits to insert 1 extent into extent tree
	 */
	credits = ext4_chunk_trans_blocks(inode, len);

retry:
	while (ret >= 0 && len) {
		handle = ext4_journal_start(inode, EXT4_HT_MAP_BLOCKS,
					    credits);
		if (IS_ERR(handle)) {
			ret = PTR_ERR(handle);
			break;
		}
		ret = ext4_map_blocks(handle, inode, &map, flags);
		if (ret <= 0) {
			ext4_debug("inode #%lu: block %u: len %u: "
				   "ext4_ext_map_blocks returned %d",
				   inode->i_ino, map.m_lblk,
				   map.m_len, ret);
			ext4_mark_inode_dirty(handle, inode);
			ret2 = ext4_journal_stop(handle);
			break;
		}
		map.m_lblk += ret;
		map.m_len = len = len - ret;
		epos = (loff_t)map.m_lblk << inode->i_blkbits;
		inode->i_ctime = ext4_current_time(inode);
		if (new_size) {
			if (epos > new_size)
				epos = new_size;
			if (ext4_update_inode_size(inode, epos) & 0x1)
				inode->i_mtime = inode->i_ctime;
		} else {
			if (epos > inode->i_size)
				ext4_set_inode_flag(inode,
						    EXT4_INODE_EOFBLOCKS);
		}
		ext4_mark_inode_dirty(handle, inode);
		ext4_update_inode_fsync_trans(handle, inode, 1);
		ret2 = ext4_journal_stop(handle);
		if (ret2)
			break;
	}
	if (ret == -ENOSPC &&
			ext4_should_retry_alloc(inode->i_sb, &retries)) {
		ret = 0;
		goto retry;
	}

	return ret > 0 ? ret2 : ret;
}

static long ext4_zero_range(struct file *file, loff_t offset,
			    loff_t len, int mode)
{
	struct inode *inode = file_inode(file);
	handle_t *handle = NULL;
	unsigned int max_blocks;
	loff_t new_size = 0;
	int ret = 0;
	int flags;
	int credits;
	int partial_begin, partial_end;
	loff_t start, end;
	ext4_lblk_t lblk;
	unsigned int blkbits = inode->i_blkbits;

	trace_ext4_zero_range(inode, offset, len, mode);

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	/* Call ext4_force_commit to flush all data in case of data=journal. */
	if (ext4_should_journal_data(inode)) {
		ret = ext4_force_commit(inode->i_sb);
		if (ret)
			return ret;
	}

	/*
	 * Round up offset. This is not fallocate, we neet to zero out
	 * blocks, so convert interior block aligned part of the range to
	 * unwritten and possibly manually zero out unaligned parts of the
	 * range.
	 */
	start = round_up(offset, 1 << blkbits);
	end = round_down((offset + len), 1 << blkbits);

	if (start < offset || end > offset + len)
		return -EINVAL;
	partial_begin = offset & ((1 << blkbits) - 1);
	partial_end = (offset + len) & ((1 << blkbits) - 1);

	lblk = start >> blkbits;
	max_blocks = (end >> blkbits);
	if (max_blocks < lblk)
		max_blocks = 0;
	else
		max_blocks -= lblk;

	mutex_lock(&inode->i_mutex);

	/*
	 * Indirect files do not support unwritten extnets
	 */
	if (!(ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS))) {
		ret = -EOPNOTSUPP;
		goto out_mutex;
	}

	if (!(mode & FALLOC_FL_KEEP_SIZE) &&
	    (offset + len > i_size_read(inode) ||
	     offset + len > EXT4_I(inode)->i_disksize)) {
		new_size = offset + len;
		ret = inode_newsize_ok(inode, new_size);
		if (ret)
			goto out_mutex;
	}

	flags = EXT4_GET_BLOCKS_CREATE_UNWRIT_EXT;
	if (mode & FALLOC_FL_KEEP_SIZE)
		flags |= EXT4_GET_BLOCKS_KEEP_SIZE;

	/* Wait all existing dio workers, newcomers will block on i_mutex */
	ext4_inode_block_unlocked_dio(inode);
	inode_dio_wait(inode);

	/* Preallocate the range including the unaligned edges */
	if (partial_begin || partial_end) {
		ret = ext4_alloc_file_blocks(file,
				round_down(offset, 1 << blkbits) >> blkbits,
				(round_up((offset + len), 1 << blkbits) -
				 round_down(offset, 1 << blkbits)) >> blkbits,
				new_size, flags, mode);
		if (ret)
			goto out_dio;

	}

	/* Zero range excluding the unaligned edges */
	if (max_blocks > 0) {
		flags |= (EXT4_GET_BLOCKS_CONVERT_UNWRITTEN |
			  EXT4_EX_NOCACHE);

		/*
		 * Prevent page faults from reinstantiating pages we have
		 * released from page cache.
		 */
		down_write(&EXT4_I(inode)->i_mmap_sem);
		ret = ext4_update_disksize_before_punch(inode, offset, len);
		if (ret) {
			up_write(&EXT4_I(inode)->i_mmap_sem);
			goto out_dio;
		}
		/* Now release the pages and zero block aligned part of pages */
		truncate_pagecache_range(inode, start, end - 1);
		inode->i_mtime = inode->i_ctime = ext4_current_time(inode);

		ret = ext4_alloc_file_blocks(file, lblk, max_blocks, new_size,
					     flags, mode);
		up_write(&EXT4_I(inode)->i_mmap_sem);
		if (ret)
			goto out_dio;
	}
	if (!partial_begin && !partial_end)
		goto out_dio;

	/*
	 * In worst case we have to writeout two nonadjacent unwritten
	 * blocks and update the inode
	 */
	credits = (2 * ext4_ext_index_trans_blocks(inode, 2)) + 1;
	if (ext4_should_journal_data(inode))
		credits += 2;
	handle = ext4_journal_start(inode, EXT4_HT_MISC, credits);
	if (IS_ERR(handle)) {
		ret = PTR_ERR(handle);
		ext4_std_error(inode->i_sb, ret);
		goto out_dio;
	}

	inode->i_mtime = inode->i_ctime = ext4_current_time(inode);
	if (new_size) {
		ext4_update_inode_size(inode, new_size);
	} else {
		/*
		* Mark that we allocate beyond EOF so the subsequent truncate
		* can proceed even if the new size is the same as i_size.
		*/
		if ((offset + len) > i_size_read(inode))
			ext4_set_inode_flag(inode, EXT4_INODE_EOFBLOCKS);
	}
	ext4_mark_inode_dirty(handle, inode);

	/* Zero out partial block at the edges of the range */
	ret = ext4_zero_partial_blocks(handle, inode, offset, len);

	if (file->f_flags & O_SYNC)
		ext4_handle_sync(handle);

	ext4_journal_stop(handle);
out_dio:
	ext4_inode_resume_unlocked_dio(inode);
out_mutex:
	mutex_unlock(&inode->i_mutex);
	return ret;
}

/*
 * preallocate space for a file. This implements ext4's fallocate file
 * operation, which gets called from sys_fallocate system call.
 * For block-mapped files, posix_fallocate should fall back to the method
 * of writing zeroes to the required new blocks (the same behavior which is
 * expected for file systems which do not support fallocate() system call).
 */
long ext4_fallocate(struct file *file, int mode, loff_t offset, loff_t len)
{
	struct inode *inode = file_inode(file);
	loff_t new_size = 0;
	unsigned int max_blocks;
	int ret = 0;
	int flags;
	ext4_lblk_t lblk;
	unsigned int blkbits = inode->i_blkbits;

	/*
	 * Encrypted inodes can't handle collapse range or insert
	 * range since we would need to re-encrypt blocks with a
	 * different IV or XTS tweak (which are based on the logical
	 * block number).
	 *
	 * XXX It's not clear why zero range isn't working, but we'll
	 * leave it disabled for encrypted inodes for now.  This is a
	 * bug we should fix....
	 */
	if (ext4_encrypted_inode(inode) &&
	    (mode & (FALLOC_FL_COLLAPSE_RANGE | FALLOC_FL_ZERO_RANGE)))
		return -EOPNOTSUPP;

	/* Return error if mode is not supported */
	if (mode & ~(FALLOC_FL_KEEP_SIZE | FALLOC_FL_PUNCH_HOLE |
		     FALLOC_FL_COLLAPSE_RANGE | FALLOC_FL_ZERO_RANGE))
		return -EOPNOTSUPP;

	if (mode & FALLOC_FL_PUNCH_HOLE)
		return ext4_punch_hole(inode, offset, len);

	ret = ext4_convert_inline_data(inode);
	if (ret)
		return ret;

	if (mode & FALLOC_FL_COLLAPSE_RANGE)
		return ext4_collapse_range(inode, offset, len);

	if (mode & FALLOC_FL_ZERO_RANGE)
		return ext4_zero_range(file, offset, len, mode);

	trace_ext4_fallocate_enter(inode, offset, len, mode);
	lblk = offset >> blkbits;
	/*
	 * We can't just convert len to max_blocks because
	 * If blocksize = 4096 offset = 3072 and len = 2048
	 */
	max_blocks = (EXT4_BLOCK_ALIGN(len + offset, blkbits) >> blkbits)
		- lblk;

	flags = EXT4_GET_BLOCKS_CREATE_UNWRIT_EXT;
	if (mode & FALLOC_FL_KEEP_SIZE)
		flags |= EXT4_GET_BLOCKS_KEEP_SIZE;

	mutex_lock(&inode->i_mutex);

	/*
	 * We only support preallocation for extent-based files only
	 */
	if (!(ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS))) {
		ret = -EOPNOTSUPP;
		goto out;
	}

	if (!(mode & FALLOC_FL_KEEP_SIZE) &&
	    (offset + len > i_size_read(inode) ||
	     offset + len > EXT4_I(inode)->i_disksize)) {
		new_size = offset + len;
		ret = inode_newsize_ok(inode, new_size);
		if (ret)
			goto out;
	}

	/* Wait all existing dio workers, newcomers will block on i_mutex */
	ext4_inode_block_unlocked_dio(inode);
	inode_dio_wait(inode);

	ret = ext4_alloc_file_blocks(file, lblk, max_blocks, new_size,
				     flags, mode);
	ext4_inode_resume_unlocked_dio(inode);
	if (ret)
		goto out;

	if (file->f_flags & O_SYNC && EXT4_SB(inode->i_sb)->s_journal) {
		ret = jbd2_complete_transaction(EXT4_SB(inode->i_sb)->s_journal,
						EXT4_I(inode)->i_sync_tid);
	}
out:
	mutex_unlock(&inode->i_mutex);
	trace_ext4_fallocate_exit(inode, offset, max_blocks, ret);
	return ret;
}

/*
 * This function convert a range of blocks to written extents
 * The caller of this function will pass the start offset and the size.
 * all unwritten extents within this range will be converted to
 * written extents.
 *
 * This function is called from the direct IO end io call back
 * function, to convert the fallocated extents after IO is completed.
 * Returns 0 on success.
 */
int ext4_convert_unwritten_extents(handle_t *handle, struct inode *inode,
				   loff_t offset, ssize_t len)
{
	unsigned int max_blocks;
	int ret = 0;
	int ret2 = 0;
	struct ext4_map_blocks map;
	unsigned int credits, blkbits = inode->i_blkbits;

	map.m_lblk = offset >> blkbits;
	/*
	 * We can't just convert len to max_blocks because
	 * If blocksize = 4096 offset = 3072 and len = 2048
	 */
	max_blocks = ((EXT4_BLOCK_ALIGN(len + offset, blkbits) >> blkbits) -
		      map.m_lblk);
	/*
	 * This is somewhat ugly but the idea is clear: When transaction is
	 * reserved, everything goes into it. Otherwise we rather start several
	 * smaller transactions for conversion of each extent separately.
	 */
	if (handle) {
		handle = ext4_journal_start_reserved(handle,
						     EXT4_HT_EXT_CONVERT);
		if (IS_ERR(handle))
			return PTR_ERR(handle);
		credits = 0;
	} else {
		/*
		 * credits to insert 1 extent into extent tree
		 */
		credits = ext4_chunk_trans_blocks(inode, max_blocks);
	}
	while (ret >= 0 && ret < max_blocks) {
		map.m_lblk += ret;
		map.m_len = (max_blocks -= ret);
		if (credits) {
			handle = ext4_journal_start(inode, EXT4_HT_MAP_BLOCKS,
						    credits);
			if (IS_ERR(handle)) {
				ret = PTR_ERR(handle);
				break;
			}
		}
		ret = ext4_map_blocks(handle, inode, &map,
				      EXT4_GET_BLOCKS_IO_CONVERT_EXT);
		if (ret <= 0)
			ext4_warning(inode->i_sb,
				     "inode #%lu: block %u: len %u: "
				     "ext4_ext_map_blocks returned %d",
				     inode->i_ino, map.m_lblk,
				     map.m_len, ret);
		ext4_mark_inode_dirty(handle, inode);
		if (credits)
			ret2 = ext4_journal_stop(handle);
		if (ret <= 0 || ret2)
			break;
	}
	if (!credits)
		ret2 = ext4_journal_stop(handle);
	return ret > 0 ? ret2 : ret;
}

/*
 * If newes is not existing extent (newes->ec_pblk equals zero) find
 * delayed extent at start of newes and update newes accordingly and
 * return start of the next delayed extent.
 *
 * If newes is existing extent (newes->ec_pblk is not equal zero)
 * return start of next delayed extent or EXT_MAX_BLOCKS if no delayed
 * extent found. Leave newes unmodified.
 */
static int ext4_find_delayed_extent(struct inode *inode,
				    struct extent_status *newes)
{
	struct extent_status es;
	ext4_lblk_t block, next_del;

	if (newes->es_pblk == 0) {
		ext4_es_find_delayed_extent_range(inode, newes->es_lblk,
				newes->es_lblk + newes->es_len - 1, &es);

		/*
		 * No extent in extent-tree contains block @newes->es_pblk,
		 * then the block may stay in 1)a hole or 2)delayed-extent.
		 */
		if (es.es_len == 0)
			/* A hole found. */
			return 0;

		if (es.es_lblk > newes->es_lblk) {
			/* A hole found. */
			newes->es_len = min(es.es_lblk - newes->es_lblk,
					    newes->es_len);
			return 0;
		}

		newes->es_len = es.es_lblk + es.es_len - newes->es_lblk;
	}

	block = newes->es_lblk + newes->es_len;
	ext4_es_find_delayed_extent_range(inode, block, EXT_MAX_BLOCKS, &es);
	if (es.es_len == 0)
		next_del = EXT_MAX_BLOCKS;
	else
		next_del = es.es_lblk;

	return next_del;
}
/* fiemap flags we can handle specified here */
#define EXT4_FIEMAP_FLAGS	(FIEMAP_FLAG_SYNC|FIEMAP_FLAG_XATTR)

static int ext4_xattr_fiemap(struct inode *inode,
				struct fiemap_extent_info *fieinfo)
{
	__u64 physical = 0;
	__u64 length;
	__u32 flags = FIEMAP_EXTENT_LAST;
	int blockbits = inode->i_sb->s_blocksize_bits;
	int error = 0;

	/* in-inode? */
	if (ext4_test_inode_state(inode, EXT4_STATE_XATTR)) {
		struct ext4_iloc iloc;
		int offset;	/* offset of xattr in inode */

		error = ext4_get_inode_loc(inode, &iloc);
		if (error)
			return error;
		physical = (__u64)iloc.bh->b_blocknr << blockbits;
		offset = EXT4_GOOD_OLD_INODE_SIZE +
				EXT4_I(inode)->i_extra_isize;
		physical += offset;
		length = EXT4_SB(inode->i_sb)->s_inode_size - offset;
		flags |= FIEMAP_EXTENT_DATA_INLINE;
		brelse(iloc.bh);
	} else { /* external block */
		physical = (__u64)EXT4_I(inode)->i_file_acl << blockbits;
		length = inode->i_sb->s_blocksize;
	}

	if (physical)
		error = fiemap_fill_next_extent(fieinfo, 0, physical,
						length, flags);
	return (error < 0 ? error : 0);
}

int ext4_fiemap(struct inode *inode, struct fiemap_extent_info *fieinfo,
		__u64 start, __u64 len)
{
	ext4_lblk_t start_blk;
	int error = 0;

	if (ext4_has_inline_data(inode)) {
		int has_inline = 1;

		error = ext4_inline_data_fiemap(inode, fieinfo, &has_inline,
						start, len);

		if (has_inline)
			return error;
	}

	if (fieinfo->fi_flags & FIEMAP_FLAG_CACHE) {
		error = ext4_ext_precache(inode);
		if (error)
			return error;
	}

	/* fallback to generic here if not in extents fmt */
	if (!(ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS)))
		return generic_block_fiemap(inode, fieinfo, start, len,
			ext4_get_block);

	if (fiemap_check_flags(fieinfo, EXT4_FIEMAP_FLAGS))
		return -EBADR;

	if (fieinfo->fi_flags & FIEMAP_FLAG_XATTR) {
		error = ext4_xattr_fiemap(inode, fieinfo);
	} else {
		ext4_lblk_t len_blks;
		__u64 last_blk;

		start_blk = start >> inode->i_sb->s_blocksize_bits;
		last_blk = (start + len - 1) >> inode->i_sb->s_blocksize_bits;
		if (last_blk >= EXT_MAX_BLOCKS)
			last_blk = EXT_MAX_BLOCKS-1;
		len_blks = ((ext4_lblk_t) last_blk) - start_blk + 1;

		/*
		 * Walk the extent tree gathering extent information
		 * and pushing extents back to the user.
		 */
		error = ext4_fill_fiemap_extents(inode, start_blk,
						 len_blks, fieinfo);
	}
	return error;
}

/*
 * ext4_access_path:
 * Function to access the path buffer for marking it dirty.
 * It also checks if there are sufficient credits left in the journal handle
 * to update path.
 */
static int
ext4_access_path(handle_t *handle, struct inode *inode,
		struct ext4_ext_path *path)
{
	int credits, err;

	if (!ext4_handle_valid(handle))
		return 0;

	/*
	 * Check if need to extend journal credits
	 * 3 for leaf, sb, and inode plus 2 (bmap and group
	 * descriptor) for each block group; assume two block
	 * groups
	 */
	if (handle->h_buffer_credits < 7) {
		credits = ext4_writepage_trans_blocks(inode);
		err = ext4_ext_truncate_extend_restart(handle, inode, credits);
		/* EAGAIN is success */
		if (err && err != -EAGAIN)
			return err;
	}

	err = ext4_ext_get_access(handle, inode, path);
	return err;
}

/*
 * ext4_ext_shift_path_extents:
 * Shift the extents of a path structure lying between path[depth].p_ext
 * and EXT_LAST_EXTENT(path[depth].p_hdr) downwards, by subtracting shift
 * from starting block for each extent.
 */
static int
ext4_ext_shift_path_extents(struct ext4_ext_path *path, ext4_lblk_t shift,
			    struct inode *inode, handle_t *handle,
			    ext4_lblk_t *start)
{
	int depth, err = 0;
	struct ext4_extent *ex_start, *ex_last;
	bool update = 0;
	depth = path->p_depth;

	while (depth >= 0) {
		if (depth == path->p_depth) {
			ex_start = path[depth].p_ext;
			if (!ex_start)
				return -EIO;

			ex_last = EXT_LAST_EXTENT(path[depth].p_hdr);

			err = ext4_access_path(handle, inode, path + depth);
			if (err)
				goto out;

			if (ex_start == EXT_FIRST_EXTENT(path[depth].p_hdr))
				update = 1;

			*start = le32_to_cpu(ex_last->ee_block) +
				ext4_ext_get_actual_len(ex_last);

			while (ex_start <= ex_last) {
				le32_add_cpu(&ex_start->ee_block, -shift);
				/* Try to merge to the left. */
				if ((ex_start >
				     EXT_FIRST_EXTENT(path[depth].p_hdr)) &&
				    ext4_ext_try_to_merge_right(inode,
							path, ex_start - 1))
					ex_last--;
				else
					ex_start++;
			}
			err = ext4_ext_dirty(handle, inode, path + depth);
			if (err)
				goto out;

			if (--depth < 0 || !update)
				break;
		}

		/* Update index too */
		err = ext4_access_path(handle, inode, path + depth);
		if (err)
			goto out;

		le32_add_cpu(&path[depth].p_idx->ei_block, -shift);
		err = ext4_ext_dirty(handle, inode, path + depth);
		if (err)
			goto out;

		/* we are done if current index is not a starting index */
		if (path[depth].p_idx != EXT_FIRST_INDEX(path[depth].p_hdr))
			break;

		depth--;
	}

out:
	return err;
}

/*
 * ext4_ext_shift_extents:
 * All the extents which lies in the range from start to the last allocated
 * block for the file are shifted downwards by shift blocks.
 * On success, 0 is returned, error otherwise.
 */
static int
ext4_ext_shift_extents(struct inode *inode, handle_t *handle,
		       ext4_lblk_t start, ext4_lblk_t shift)
{
	struct ext4_ext_path *path;
	int ret = 0, depth;
	struct ext4_extent *extent;
	ext4_lblk_t stop_block;
	ext4_lblk_t ex_start, ex_end;

	/* Let path point to the last extent */
	path = ext4_find_extent(inode, EXT_MAX_BLOCKS - 1, NULL, 0);
	if (IS_ERR(path))
		return PTR_ERR(path);

	depth = path->p_depth;
	extent = path[depth].p_ext;
	if (!extent)
		goto out;

	stop_block = le32_to_cpu(extent->ee_block) +
			ext4_ext_get_actual_len(extent);

	/* Nothing to shift, if hole is at the end of file */
	if (start >= stop_block)
		goto out;

	/*
	 * Don't start shifting extents until we make sure the hole is big
	 * enough to accomodate the shift.
	 */
	path = ext4_find_extent(inode, start - 1, &path, 0);
	if (IS_ERR(path))
		return PTR_ERR(path);
	depth = path->p_depth;
	extent =  path[depth].p_ext;
	if (extent) {
		ex_start = le32_to_cpu(extent->ee_block);
		ex_end = le32_to_cpu(extent->ee_block) +
			ext4_ext_get_actual_len(extent);
	} else {
		ex_start = 0;
		ex_end = 0;
	}

	if ((start == ex_start && shift > ex_start) ||
	    (shift > start - ex_end))
		return -EINVAL;

	/* Its safe to start updating extents */
	while (start < stop_block) {
		path = ext4_find_extent(inode, start, &path, 0);
		if (IS_ERR(path))
			return PTR_ERR(path);
		depth = path->p_depth;
		extent = path[depth].p_ext;
		if (!extent) {
			EXT4_ERROR_INODE(inode, "unexpected hole at %lu",
					 (unsigned long) start);
			return -EIO;
		}
		if (start > le32_to_cpu(extent->ee_block)) {
			/* Hole, move to the next extent */
			if (extent < EXT_LAST_EXTENT(path[depth].p_hdr)) {
				path[depth].p_ext++;
			} else {
				start = ext4_ext_next_allocated_block(path);
				continue;
			}
		}
		ret = ext4_ext_shift_path_extents(path, shift, inode,
				handle, &start);
		if (ret)
			break;
	}
out:
	ext4_ext_drop_refs(path);
	kfree(path);
	return ret;
}

/*
 * ext4_collapse_range:
 * This implements the fallocate's collapse range functionality for ext4
 * Returns: 0 and non-zero on error.
 */
int ext4_collapse_range(struct inode *inode, loff_t offset, loff_t len)
{
	struct super_block *sb = inode->i_sb;
	ext4_lblk_t punch_start, punch_stop;
	handle_t *handle;
	unsigned int credits;
	loff_t new_size, ioffset;
	int ret;

	/*
	 * We need to test this early because xfstests assumes that a
	 * collapse range of (0, 1) will return EOPNOTSUPP if the file
	 * system does not support collapse range.
	 */
	if (!ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS))
		return -EOPNOTSUPP;

	/* Collapse range works only on fs block size aligned offsets. */
	if (offset & (EXT4_CLUSTER_SIZE(sb) - 1) ||
	    len & (EXT4_CLUSTER_SIZE(sb) - 1))
		return -EINVAL;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	trace_ext4_collapse_range(inode, offset, len);

	punch_start = offset >> EXT4_BLOCK_SIZE_BITS(sb);
	punch_stop = (offset + len) >> EXT4_BLOCK_SIZE_BITS(sb);

	/* Call ext4_force_commit to flush all data in case of data=journal. */
	if (ext4_should_journal_data(inode)) {
		ret = ext4_force_commit(inode->i_sb);
		if (ret)
			return ret;
	}

	mutex_lock(&inode->i_mutex);
	/*
	 * There is no need to overlap collapse range with EOF, in which case
	 * it is effectively a truncate operation
	 */
	if (offset + len >= i_size_read(inode)) {
		ret = -EINVAL;
		goto out_mutex;
	}

	/* Currently just for extent based files */
	if (!ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS)) {
		ret = -EOPNOTSUPP;
		goto out_mutex;
	}

	/* Wait for existing dio to complete */
	ext4_inode_block_unlocked_dio(inode);
	inode_dio_wait(inode);

	/*
	 * Prevent page faults from reinstantiating pages we have released from
	 * page cache.
	 */
	down_write(&EXT4_I(inode)->i_mmap_sem);
	/*
	 * Need to round down offset to be aligned with page size boundary
	 * for page size > block size.
	 */
	ioffset = round_down(offset, PAGE_SIZE);
	/*
	 * Write tail of the last page before removed range since it will get
	 * removed from the page cache below.
	 */
	ret = filemap_write_and_wait_range(inode->i_mapping, ioffset, offset);
	if (ret)
		goto out_mmap;
	/*
	 * Write data that will be shifted to preserve them when discarding
	 * page cache below. We are also protected from pages becoming dirty
	 * by i_mmap_sem.
	 */
	ret = filemap_write_and_wait_range(inode->i_mapping, offset + len,
					   LLONG_MAX);
	if (ret)
		goto out_mmap;
	truncate_pagecache(inode, ioffset);

	credits = ext4_writepage_trans_blocks(inode);
	handle = ext4_journal_start(inode, EXT4_HT_TRUNCATE, credits);
	if (IS_ERR(handle)) {
		ret = PTR_ERR(handle);
		goto out_mmap;
	}

	down_write(&EXT4_I(inode)->i_data_sem);
	ext4_discard_preallocations(inode);

	ret = ext4_es_remove_extent(inode, punch_start,
				    EXT_MAX_BLOCKS - punch_start);
	if (ret) {
		up_write(&EXT4_I(inode)->i_data_sem);
		goto out_stop;
	}

	ret = ext4_ext_remove_space(inode, punch_start, punch_stop - 1);
	if (ret) {
		up_write(&EXT4_I(inode)->i_data_sem);
		goto out_stop;
	}
	ext4_discard_preallocations(inode);

	ret = ext4_ext_shift_extents(inode, handle, punch_stop,
				     punch_stop - punch_start);
	if (ret) {
		up_write(&EXT4_I(inode)->i_data_sem);
		goto out_stop;
	}

	new_size = i_size_read(inode) - len;
	i_size_write(inode, new_size);
	EXT4_I(inode)->i_disksize = new_size;

	up_write(&EXT4_I(inode)->i_data_sem);
	if (IS_SYNC(inode))
		ext4_handle_sync(handle);
	inode->i_mtime = inode->i_ctime = ext4_current_time(inode);
	ext4_mark_inode_dirty(handle, inode);

out_stop:
	ext4_journal_stop(handle);
out_mmap:
	up_write(&EXT4_I(inode)->i_mmap_sem);
	ext4_inode_resume_unlocked_dio(inode);
out_mutex:
	mutex_unlock(&inode->i_mutex);
	return ret;
}

/**
 * ext4_swap_extents - Swap extents between two inodes
 *
 * @inode1:	First inode
 * @inode2:	Second inode
 * @lblk1:	Start block for first inode
 * @lblk2:	Start block for second inode
 * @count:	Number of blocks to swap
 * @mark_unwritten: Mark second inode's extents as unwritten after swap
 * @erp:	Pointer to save error value
 *
 * This helper routine does exactly what is promise "swap extents". All other
 * stuff such as page-cache locking consistency, bh mapping consistency or
 * extent's data copying must be performed by caller.
 * Locking:
 * 		i_mutex is held for both inodes
 * 		i_data_sem is locked for write for both inodes
 * Assumptions:
 *		All pages from requested range are locked for both inodes
 */
int
ext4_swap_extents(handle_t *handle, struct inode *inode1,
		     struct inode *inode2, ext4_lblk_t lblk1, ext4_lblk_t lblk2,
		  ext4_lblk_t count, int unwritten, int *erp)
{
	struct ext4_ext_path *path1 = NULL;
	struct ext4_ext_path *path2 = NULL;
	int replaced_count = 0;

	BUG_ON(!rwsem_is_locked(&EXT4_I(inode1)->i_data_sem));
	BUG_ON(!rwsem_is_locked(&EXT4_I(inode2)->i_data_sem));
	BUG_ON(!mutex_is_locked(&inode1->i_mutex));
	BUG_ON(!mutex_is_locked(&inode1->i_mutex));

	*erp = ext4_es_remove_extent(inode1, lblk1, count);
	if (unlikely(*erp))
		return 0;
	*erp = ext4_es_remove_extent(inode2, lblk2, count);
	if (unlikely(*erp))
		return 0;

	while (count) {
		struct ext4_extent *ex1, *ex2, tmp_ex;
		ext4_lblk_t e1_blk, e2_blk;
		int e1_len, e2_len, len;
		int split = 0;

		path1 = ext4_find_extent(inode1, lblk1, NULL, EXT4_EX_NOCACHE);
		if (unlikely(IS_ERR(path1))) {
			*erp = PTR_ERR(path1);
			path1 = NULL;
		finish:
			count = 0;
			goto repeat;
		}
		path2 = ext4_find_extent(inode2, lblk2, NULL, EXT4_EX_NOCACHE);
		if (unlikely(IS_ERR(path2))) {
			*erp = PTR_ERR(path2);
			path2 = NULL;
			goto finish;
		}
		ex1 = path1[path1->p_depth].p_ext;
		ex2 = path2[path2->p_depth].p_ext;
		/* Do we have somthing to swap ? */
		if (unlikely(!ex2 || !ex1))
			goto finish;

		e1_blk = le32_to_cpu(ex1->ee_block);
		e2_blk = le32_to_cpu(ex2->ee_block);
		e1_len = ext4_ext_get_actual_len(ex1);
		e2_len = ext4_ext_get_actual_len(ex2);

		/* Hole handling */
		if (!in_range(lblk1, e1_blk, e1_len) ||
		    !in_range(lblk2, e2_blk, e2_len)) {
			ext4_lblk_t next1, next2;

			/* if hole after extent, then go to next extent */
			next1 = ext4_ext_next_allocated_block(path1);
			next2 = ext4_ext_next_allocated_block(path2);
			/* If hole before extent, then shift to that extent */
			if (e1_blk > lblk1)
				next1 = e1_blk;
			if (e2_blk > lblk2)
				next2 = e1_blk;
			/* Do we have something to swap */
			if (next1 == EXT_MAX_BLOCKS || next2 == EXT_MAX_BLOCKS)
				goto finish;
			/* Move to the rightest boundary */
			len = next1 - lblk1;
			if (len < next2 - lblk2)
				len = next2 - lblk2;
			if (len > count)
				len = count;
			lblk1 += len;
			lblk2 += len;
			count -= len;
			goto repeat;
		}

		/* Prepare left boundary */
		if (e1_blk < lblk1) {
			split = 1;
			*erp = ext4_force_split_extent_at(handle, inode1,
						&path1, lblk1, 0);
			if (unlikely(*erp))
				goto finish;
		}
		if (e2_blk < lblk2) {
			split = 1;
			*erp = ext4_force_split_extent_at(handle, inode2,
						&path2,  lblk2, 0);
			if (unlikely(*erp))
				goto finish;
		}
		/* ext4_split_extent_at() may result in leaf extent split,
		 * path must to be revalidated. */
		if (split)
			goto repeat;

		/* Prepare right boundary */
		len = count;
		if (len > e1_blk + e1_len - lblk1)
			len = e1_blk + e1_len - lblk1;
		if (len > e2_blk + e2_len - lblk2)
			len = e2_blk + e2_len - lblk2;

		if (len != e1_len) {
			split = 1;
			*erp = ext4_force_split_extent_at(handle, inode1,
						&path1, lblk1 + len, 0);
			if (unlikely(*erp))
				goto finish;
		}
		if (len != e2_len) {
			split = 1;
			*erp = ext4_force_split_extent_at(handle, inode2,
						&path2, lblk2 + len, 0);
			if (*erp)
				goto finish;
		}
		/* ext4_split_extent_at() may result in leaf extent split,
		 * path must to be revalidated. */
		if (split)
			goto repeat;

		BUG_ON(e2_len != e1_len);
		*erp = ext4_ext_get_access(handle, inode1, path1 + path1->p_depth);
		if (unlikely(*erp))
			goto finish;
		*erp = ext4_ext_get_access(handle, inode2, path2 + path2->p_depth);
		if (unlikely(*erp))
			goto finish;

		/* Both extents are fully inside boundaries. Swap it now */
		tmp_ex = *ex1;
		ext4_ext_store_pblock(ex1, ext4_ext_pblock(ex2));
		ext4_ext_store_pblock(ex2, ext4_ext_pblock(&tmp_ex));
		ex1->ee_len = cpu_to_le16(e2_len);
		ex2->ee_len = cpu_to_le16(e1_len);
		if (unwritten)
			ext4_ext_mark_unwritten(ex2);
		if (ext4_ext_is_unwritten(&tmp_ex))
			ext4_ext_mark_unwritten(ex1);

		ext4_ext_try_to_merge(handle, inode2, path2, ex2);
		ext4_ext_try_to_merge(handle, inode1, path1, ex1);
		*erp = ext4_ext_dirty(handle, inode2, path2 +
				      path2->p_depth);
		if (unlikely(*erp))
			goto finish;
		*erp = ext4_ext_dirty(handle, inode1, path1 +
				      path1->p_depth);
		/*
		 * Looks scarry ah..? second inode already points to new blocks,
		 * and it was successfully dirtied. But luckily error may happen
		 * only due to journal error, so full transaction will be
		 * aborted anyway.
		 */
		if (unlikely(*erp))
			goto finish;
		lblk1 += len;
		lblk2 += len;
		replaced_count += len;
		count -= len;

	repeat:
		ext4_ext_drop_refs(path1);
		kfree(path1);
		ext4_ext_drop_refs(path2);
		kfree(path2);
		path1 = path2 = NULL;
	}
	return replaced_count;
}
