/*
 * pass4.c -- pass #4 of e2fsck: Check reference counts
 *
 * Copyright (C) 1993, 1994, 1995, 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 *
 * Pass 4 frees the following data structures:
 * 	- A bitmap of which inodes are in bad blocks.	(inode_bb_map)
 * 	- A bitmap of which inodes are imagic inodes.	(inode_imagic_map)
 *	- Ref counts for ea_inodes.			(ea_inode_refs)
 */

#include "config.h"
#include "e2fsck.h"
#include "problem.h"
#include <ext2fs/ext2_ext_attr.h>

/*
 * This routine is called when an inode is not connected to the
 * directory tree.
 *
 * This subroutine returns 1 then the caller shouldn't bother with the
 * rest of the pass 4 tests.
 */
static int disconnect_inode(e2fsck_t ctx, ext2_ino_t i,
			    struct ext2_inode_large *inode)
{
	ext2_filsys fs = ctx->fs;
	struct problem_context	pctx;
	__u32 eamagic = 0;
	int extra_size = 0;

	e2fsck_read_inode_full(ctx, i, EXT2_INODE(inode),
			       EXT2_INODE_SIZE(fs->super),
			       "pass4: disconnect_inode");
	if (EXT2_INODE_SIZE(fs->super) > EXT2_GOOD_OLD_INODE_SIZE)
		extra_size = inode->i_extra_isize;

	clear_problem_context(&pctx);
	pctx.ino = i;
	pctx.inode = EXT2_INODE(inode);

	if (EXT2_INODE_SIZE(fs->super) -EXT2_GOOD_OLD_INODE_SIZE -extra_size >0)
		eamagic = *(__u32 *)(((char *)inode) +EXT2_GOOD_OLD_INODE_SIZE +
				     extra_size);
	/*
	 * Offer to delete any zero-length files that does not have
	 * blocks.  If there is an EA block, it might have useful
	 * information, so we won't prompt to delete it, but let it be
	 * reconnected to lost+found.
	 */
	if (!inode->i_blocks && eamagic != EXT2_EXT_ATTR_MAGIC &&
	    (LINUX_S_ISREG(inode->i_mode) || LINUX_S_ISDIR(inode->i_mode))) {
		if (fix_problem(ctx, PR_4_ZERO_LEN_INODE, &pctx)) {
			e2fsck_clear_inode(ctx, i, EXT2_INODE(inode), 0,
					   "disconnect_inode");
			/*
			 * Fix up the bitmaps...
			 */
			e2fsck_read_bitmaps(ctx);
			ext2fs_inode_alloc_stats2(fs, i, -1,
						  LINUX_S_ISDIR(inode->i_mode));
			quota_data_inodes(ctx->qctx, inode, i, -1);
			return 0;
		}
	}

	/*
	 * Prompt to reconnect.
	 */
	if (fix_problem(ctx, PR_4_UNATTACHED_INODE, &pctx)) {
		if (e2fsck_reconnect_file(ctx, i))
			ext2fs_unmark_valid(fs);
	} else {
		/*
		 * If we don't attach the inode, then skip the
		 * i_links_test since there's no point in trying to
		 * force i_links_count to zero.
		 */
		ext2fs_unmark_valid(fs);
		return 1;
	}
	return 0;
}

static void check_ea_inode(e2fsck_t ctx, ext2_ino_t i,
			   struct ext2_inode_large *inode, __u16 *link_counted)
{
	__u64 actual_refs = 0;
	__u64 ref_count;

	/*
	 * This function is called when link_counted is zero. So this may not
	 * be an xattr inode at all. Return immediately if EA_INODE flag is not
	 * set.
	 */
	e2fsck_read_inode_full(ctx, i, EXT2_INODE(inode),
			       EXT2_INODE_SIZE(ctx->fs->super),
			       "pass4: check_ea_inode");
	if (!(inode->i_flags & EXT4_EA_INODE_FL))
		return;

	if (ctx->ea_inode_refs)
		ea_refcount_fetch(ctx->ea_inode_refs, i, &actual_refs);
	if (!actual_refs)
		return;

	/*
	 * There are some attribute references, link_counted is now considered
	 * to be 1.
	 */
	*link_counted = 1;

	ref_count = ext2fs_get_ea_inode_ref(EXT2_INODE(inode));

	/* Old Lustre-style xattr inodes do not have a stored refcount.
	 * However, their i_ctime and i_atime should be the same.
	 */
	if (ref_count != actual_refs && inode->i_ctime != inode->i_atime) {
		struct problem_context pctx;

		clear_problem_context(&pctx);
		pctx.ino = i;
		pctx.num = ref_count;
		pctx.num2 = actual_refs;
		if (fix_problem(ctx, PR_4_EA_INODE_REF_COUNT, &pctx)) {
			ext2fs_set_ea_inode_ref(EXT2_INODE(inode), actual_refs);
			e2fsck_write_inode(ctx, i, EXT2_INODE(inode), "pass4");
		}
	}
}

void e2fsck_pass4(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	ext2_ino_t	i;
	struct ext2_inode_large	*inode;
	int inode_size = EXT2_INODE_SIZE(fs->super);
#ifdef RESOURCE_TRACK
	struct resource_track	rtrack;
#endif
	struct problem_context	pctx;
	__u16	link_count, link_counted;
	int dir_nlink_fs;
	char	*buf = 0;
	dgrp_t	group, maxgroup;

	init_resource_track(&rtrack, ctx->fs->io);

#ifdef MTRACE
	mtrace_print("Pass 4");
#endif
	/*
	 * Since pass4 is mostly CPU bound, start readahead of bitmaps
	 * ahead of pass 5 if we haven't already loaded them.
	 */
	if (ctx->readahead_kb &&
	    (fs->block_map == NULL || fs->inode_map == NULL))
		e2fsck_readahead(fs, E2FSCK_READA_BBITMAP |
				     E2FSCK_READA_IBITMAP,
				 0, fs->group_desc_count);

	clear_problem_context(&pctx);

	if (!(ctx->options & E2F_OPT_PREEN))
		fix_problem(ctx, PR_4_PASS_HEADER, &pctx);

	dir_nlink_fs = ext2fs_has_feature_dir_nlink(fs->super);

	group = 0;
	maxgroup = fs->group_desc_count;
	if (ctx->progress)
		if ((ctx->progress)(ctx, 4, 0, maxgroup))
			return;

	inode = e2fsck_allocate_memory(ctx, inode_size, "scratch inode");

	/* Protect loop from wrap-around if s_inodes_count maxed */
	for (i=1; i <= fs->super->s_inodes_count && i > 0; i++) {
		int isdir;

		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			goto errout;
		if ((i % fs->super->s_inodes_per_group) == 0) {
			group++;
			if (ctx->progress)
				if ((ctx->progress)(ctx, 4, group, maxgroup))
					goto errout;
		}
		if (i == quota_type2inum(PRJQUOTA, ctx->fs->super) ||
		    i == EXT2_BAD_INO ||
		    (i > EXT2_ROOT_INO && i < EXT2_FIRST_INODE(fs->super)))
			continue;
		if (!(ext2fs_test_inode_bitmap2(ctx->inode_used_map, i)) ||
		    (ctx->inode_imagic_map &&
		     ext2fs_test_inode_bitmap2(ctx->inode_imagic_map, i)) ||
		    (ctx->inode_bb_map &&
		     ext2fs_test_inode_bitmap2(ctx->inode_bb_map, i)))
			continue;
		ext2fs_icount_fetch(ctx->inode_link_info, i, &link_count);
		ext2fs_icount_fetch(ctx->inode_count, i, &link_counted);

		if (link_counted == 0) {
			/*
			 * link_counted is expected to be 0 for an ea_inode.
			 * check_ea_inode() will update link_counted if
			 * necessary.
			 */
			check_ea_inode(ctx, i, inode, &link_counted);
		}

		if (link_counted == 0) {
			if (!buf)
				buf = e2fsck_allocate_memory(ctx,
				     fs->blocksize, "bad_inode buffer");
			if (e2fsck_process_bad_inode(ctx, 0, i, buf))
				continue;
			if (disconnect_inode(ctx, i, inode))
				continue;
			ext2fs_icount_fetch(ctx->inode_link_info, i,
					    &link_count);
			ext2fs_icount_fetch(ctx->inode_count, i,
					    &link_counted);
		}
		isdir = ext2fs_test_inode_bitmap2(ctx->inode_dir_map, i);
		if (isdir && (link_counted > EXT2_LINK_MAX)) {
			if (!dir_nlink_fs &&
			    fix_problem(ctx, PR_4_DIR_NLINK_FEATURE, &pctx)) {
				ext2fs_set_feature_dir_nlink(fs->super);
				ext2fs_mark_super_dirty(fs);
				dir_nlink_fs = 1;
			}
			link_counted = 1;
		}
		if (link_counted != link_count) {
			int fix_nlink = 0;

			e2fsck_read_inode_full(ctx, i, EXT2_INODE(inode),
					       inode_size, "pass4");
			pctx.ino = i;
			pctx.inode = EXT2_INODE(inode);
			if ((link_count != inode->i_links_count) && !isdir &&
			    (inode->i_links_count <= EXT2_LINK_MAX)) {
				pctx.num = link_count;
				fix_problem(ctx,
					    PR_4_INCONSISTENT_COUNT, &pctx);
			}
			pctx.num = link_counted;
			/* i_link_count was previously exceeded, but no longer
			 * is, fix this but don't consider it an error */
			if (isdir && link_counted > 1 &&
			    (inode->i_flags & EXT2_INDEX_FL) &&
			    link_count == 1) {
				if ((ctx->options & E2F_OPT_READONLY) == 0) {
					fix_nlink =
						fix_problem(ctx,
							PR_4_DIR_OVERFLOW_REF_COUNT,
							&pctx);
				}
			} else {
				fix_nlink = fix_problem(ctx, PR_4_BAD_REF_COUNT,
						&pctx);
			}
			if (fix_nlink) {
				inode->i_links_count = link_counted;
				e2fsck_write_inode_full(ctx, i,
							EXT2_INODE(inode),
							inode_size, "pass4");
			}
		}
	}
	ext2fs_free_icount(ctx->inode_link_info); ctx->inode_link_info = 0;
	ext2fs_free_icount(ctx->inode_count); ctx->inode_count = 0;
	ext2fs_free_inode_bitmap(ctx->inode_bb_map);
	ctx->inode_bb_map = 0;
	ea_refcount_free(ctx->ea_inode_refs);
	ctx->ea_inode_refs = 0;
	ext2fs_free_inode_bitmap(ctx->inode_imagic_map);
	ctx->inode_imagic_map = 0;
errout:
	if (buf)
		ext2fs_free_mem(&buf);

	ext2fs_free_mem(&inode);
	print_resource_track(ctx, _("Pass 4"), &rtrack, ctx->fs->io);
}

