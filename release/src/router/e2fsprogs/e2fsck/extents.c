/*
 * extents.c --- rebuild extent tree
 *
 * Copyright (C) 2014 Oracle.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License, version 2.
 * %End-Header%
 */

#include "config.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "e2fsck.h"
#include "problem.h"

#undef DEBUG
#undef DEBUG_SUMMARY
#undef DEBUG_FREE

#define NUM_EXTENTS	341	/* about one ETB' worth of extents */

static errcode_t e2fsck_rebuild_extents(e2fsck_t ctx, ext2_ino_t ino);

/* Schedule an inode to have its extent tree rebuilt during pass 1E. */
errcode_t e2fsck_rebuild_extents_later(e2fsck_t ctx, ext2_ino_t ino)
{
	errcode_t retval = 0;

	if (!ext2fs_has_feature_extents(ctx->fs->super) ||
	    (ctx->options & E2F_OPT_NO) ||
	    (ino != EXT2_ROOT_INO && ino < ctx->fs->super->s_first_ino))
		return 0;

	if (ctx->flags & E2F_FLAG_ALLOC_OK)
		return e2fsck_rebuild_extents(ctx, ino);

	if (!ctx->inodes_to_rebuild)
		retval = e2fsck_allocate_inode_bitmap(ctx->fs,
					     _("extent rebuild inode map"),
					     EXT2FS_BMAP64_RBTREE,
					     "inodes_to_rebuild",
					     &ctx->inodes_to_rebuild);
	if (retval)
		return retval;

	ext2fs_mark_inode_bitmap2(ctx->inodes_to_rebuild, ino);
	return 0;
}

/* Ask if an inode will have its extents rebuilt during pass 1E. */
int e2fsck_ino_will_be_rebuilt(e2fsck_t ctx, ext2_ino_t ino)
{
	if (!ctx->inodes_to_rebuild)
		return 0;
	return ext2fs_test_inode_bitmap2(ctx->inodes_to_rebuild, ino);
}

struct extent_list {
	blk64_t blocks_freed;
	struct ext2fs_extent *extents;
	unsigned int count;
	unsigned int size;
	unsigned int ext_read;
	errcode_t retval;
	ext2_ino_t ino;
};

static errcode_t load_extents(e2fsck_t ctx, struct extent_list *list)
{
	ext2_filsys		fs = ctx->fs;
	ext2_extent_handle_t	handle;
	struct ext2fs_extent	extent;
	errcode_t		retval;

	retval = ext2fs_extent_open(fs, list->ino, &handle);
	if (retval)
		return retval;

	retval = ext2fs_extent_get(handle, EXT2_EXTENT_ROOT, &extent);
	if (retval)
		goto out;

	do {
		if (extent.e_flags & EXT2_EXTENT_FLAGS_SECOND_VISIT)
			goto next;

		/* Internal node; free it and we'll re-allocate it later */
		if (!(extent.e_flags & EXT2_EXTENT_FLAGS_LEAF)) {
#if defined(DEBUG) || defined(DEBUG_FREE)
			printf("ino=%d free=%llu bf=%llu\n", list->ino,
					extent.e_pblk, list->blocks_freed + 1);
#endif
			list->blocks_freed++;
			ext2fs_block_alloc_stats2(fs, extent.e_pblk, -1);
			goto next;
		}

		list->ext_read++;
		/* Can we attach it to the previous extent? */
		if (list->count) {
			struct ext2fs_extent *last = list->extents +
						     list->count - 1;
			blk64_t end = last->e_len + extent.e_len;

			if (last->e_pblk + last->e_len == extent.e_pblk &&
			    last->e_lblk + last->e_len == extent.e_lblk &&
			    (last->e_flags & EXT2_EXTENT_FLAGS_UNINIT) ==
			    (extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT) &&
			    end < (1ULL << 32)) {
				last->e_len += extent.e_len;
#ifdef DEBUG
				printf("R: ino=%d len=%u\n", list->ino,
						last->e_len);
#endif
				goto next;
			}
		}

		/* Do we need to expand? */
		if (list->count == list->size) {
			unsigned int new_size = (list->size + NUM_EXTENTS) *
						sizeof(struct ext2fs_extent);
			retval = ext2fs_resize_mem(0, new_size, &list->extents);
			if (retval)
				goto out;
			list->size += NUM_EXTENTS;
		}

		/* Add a new extent */
		memcpy(list->extents + list->count, &extent, sizeof(extent));
#ifdef DEBUG
		printf("R: ino=%d pblk=%llu lblk=%llu len=%u\n", list->ino,
				extent.e_pblk, extent.e_lblk, extent.e_len);
#endif
		list->count++;
next:
		retval = ext2fs_extent_get(handle, EXT2_EXTENT_NEXT, &extent);
	} while (retval == 0);

out:
	/* Ok if we run off the end */
	if (retval == EXT2_ET_EXTENT_NO_NEXT)
		retval = 0;
	ext2fs_extent_free(handle);
	return retval;
}

static int find_blocks(ext2_filsys fs, blk64_t *blocknr, e2_blkcnt_t blockcnt,
		       blk64_t ref_blk EXT2FS_ATTR((unused)),
		       int ref_offset EXT2FS_ATTR((unused)), void *priv_data)
{
	struct extent_list *list = priv_data;

	/* Internal node? */
	if (blockcnt < 0) {
#if defined(DEBUG) || defined(DEBUG_FREE)
		printf("ino=%d free=%llu bf=%llu\n", list->ino, *blocknr,
				list->blocks_freed + 1);
#endif
		list->blocks_freed++;
		ext2fs_block_alloc_stats2(fs, *blocknr, -1);
		return 0;
	}

	/* Can we attach it to the previous extent? */
	if (list->count) {
		struct ext2fs_extent *last = list->extents +
					     list->count - 1;
		blk64_t end = last->e_len + 1;

		if (last->e_lblk + last->e_len == (__u64) blockcnt &&
		    last->e_pblk + last->e_len == *blocknr &&
		    end < (1ULL << 32)) {
			last->e_len++;
#ifdef DEBUG
			printf("R: ino=%d len=%u\n", list->ino, last->e_len);
#endif
			return 0;
		}
	}

	/* Do we need to expand? */
	if (list->count == list->size) {
		unsigned int new_size = (list->size + NUM_EXTENTS) *
					sizeof(struct ext2fs_extent);
		list->retval = ext2fs_resize_mem(0, new_size, &list->extents);
		if (list->retval)
			return BLOCK_ABORT;
		list->size += NUM_EXTENTS;
	}

	/* Add a new extent */
	list->extents[list->count].e_pblk = *blocknr;
	list->extents[list->count].e_lblk = blockcnt;
	list->extents[list->count].e_len = 1;
	list->extents[list->count].e_flags = 0;
#ifdef DEBUG
	printf("R: ino=%d pblk=%llu lblk=%llu len=%u\n", list->ino, *blocknr,
			blockcnt, 1);
#endif
	list->count++;

	return 0;
}

static errcode_t rebuild_extent_tree(e2fsck_t ctx, struct extent_list *list,
				     ext2_ino_t ino)
{
	struct ext2_inode_large	inode;
	errcode_t		retval;
	ext2_extent_handle_t	handle;
	unsigned int		i, ext_written;
	struct ext2fs_extent	*ex, extent;
	blk64_t			start_val, delta;

	list->count = 0;
	list->blocks_freed = 0;
	list->ino = ino;
	list->ext_read = 0;
	e2fsck_read_inode_full(ctx, ino, EXT2_INODE(&inode), sizeof(inode),
			       "rebuild_extents");

	/* Skip deleted inodes and inline data files */
	if (inode.i_links_count == 0 ||
	    inode.i_flags & EXT4_INLINE_DATA_FL)
		return 0;

	/* Collect lblk->pblk mappings */
	if (inode.i_flags & EXT4_EXTENTS_FL) {
		retval = load_extents(ctx, list);
		if (retval)
			goto err;
		goto extents_loaded;
	}

	retval = ext2fs_block_iterate3(ctx->fs, ino, BLOCK_FLAG_READ_ONLY, 0,
				       find_blocks, list);
	if (retval)
		goto err;
	if (list->retval) {
		retval = list->retval;
		goto err;
	}

extents_loaded:
	/* Reset extent tree */
	inode.i_flags &= ~EXT4_EXTENTS_FL;
	memset(inode.i_block, 0, sizeof(inode.i_block));

	/* Make a note of freed blocks */
	quota_data_sub(ctx->qctx, &inode, ino,
		       list->blocks_freed * ctx->fs->blocksize);
	retval = ext2fs_iblk_sub_blocks(ctx->fs, EXT2_INODE(&inode),
					list->blocks_freed);
	if (retval)
		goto err;

	/* Now stuff extents into the file */
	retval = ext2fs_extent_open2(ctx->fs, ino, EXT2_INODE(&inode), &handle);
	if (retval)
		goto err;

	ext_written = 0;
	start_val = ext2fs_get_stat_i_blocks(ctx->fs, EXT2_INODE(&inode));
	for (i = 0, ex = list->extents; i < list->count; i++, ex++) {
		memcpy(&extent, ex, sizeof(struct ext2fs_extent));
		extent.e_flags &= EXT2_EXTENT_FLAGS_UNINIT;
		if (extent.e_flags & EXT2_EXTENT_FLAGS_UNINIT) {
			if (extent.e_len > EXT_UNINIT_MAX_LEN) {
				extent.e_len = EXT_UNINIT_MAX_LEN;
				ex->e_pblk += EXT_UNINIT_MAX_LEN;
				ex->e_lblk += EXT_UNINIT_MAX_LEN;
				ex->e_len -= EXT_UNINIT_MAX_LEN;
				ex--;
				i--;
			}
		} else {
			if (extent.e_len > EXT_INIT_MAX_LEN) {
				extent.e_len = EXT_INIT_MAX_LEN;
				ex->e_pblk += EXT_INIT_MAX_LEN;
				ex->e_lblk += EXT_INIT_MAX_LEN;
				ex->e_len -= EXT_INIT_MAX_LEN;
				ex--;
				i--;
			}
		}

#ifdef DEBUG
		printf("W: ino=%d pblk=%llu lblk=%llu len=%u\n", ino,
				extent.e_pblk, extent.e_lblk, extent.e_len);
#endif
		retval = ext2fs_extent_insert(handle, EXT2_EXTENT_INSERT_AFTER,
					      &extent);
		if (retval)
			goto err2;
		retval = ext2fs_extent_fix_parents(handle);
		if (retval)
			goto err2;
		ext_written++;
	}

	delta = ext2fs_get_stat_i_blocks(ctx->fs, EXT2_INODE(&inode)) -
		start_val;
	if (delta)
		quota_data_add(ctx->qctx, &inode, ino, delta << 9);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
	printf("rebuild: ino=%d extents=%d->%d\n", ino, list->ext_read,
	       ext_written);
#endif
	e2fsck_write_inode(ctx, ino, EXT2_INODE(&inode), "rebuild_extents");

err2:
	ext2fs_extent_free(handle);
err:
	return retval;
}

/* Rebuild the extents immediately */
static errcode_t e2fsck_rebuild_extents(e2fsck_t ctx, ext2_ino_t ino)
{
	struct extent_list list = { 0 };
	errcode_t err;

	if (!ext2fs_has_feature_extents(ctx->fs->super) ||
	    (ctx->options & E2F_OPT_NO) ||
	    (ino != EXT2_ROOT_INO && ino < ctx->fs->super->s_first_ino))
		return 0;

	e2fsck_read_bitmaps(ctx);
	err = ext2fs_get_array(NUM_EXTENTS, sizeof(struct ext2fs_extent),
			       &list.extents);
	if (err)
		return err;
	list.size = NUM_EXTENTS;
	err = rebuild_extent_tree(ctx, &list, ino);
	ext2fs_free_mem(&list.extents);

	return err;
}

static void rebuild_extents(e2fsck_t ctx, const char *pass_name, int pr_header)
{
	struct problem_context	pctx;
#ifdef RESOURCE_TRACK
	struct resource_track	rtrack;
#endif
	struct extent_list	list = { 0 };
	int			first = 1;
	ext2_ino_t		ino = 0;
	errcode_t		retval;

	if (!ext2fs_has_feature_extents(ctx->fs->super) ||
	    !ext2fs_test_valid(ctx->fs) ||
	    ctx->invalid_bitmaps) {
		if (ctx->inodes_to_rebuild)
			ext2fs_free_inode_bitmap(ctx->inodes_to_rebuild);
		ctx->inodes_to_rebuild = NULL;
	}

	if (ctx->inodes_to_rebuild == NULL)
		return;

	init_resource_track(&rtrack, ctx->fs->io);
	clear_problem_context(&pctx);
	e2fsck_read_bitmaps(ctx);

	list.size = NUM_EXTENTS;
	retval = ext2fs_get_array(sizeof(struct ext2fs_extent),
				  list.size, &list.extents);
	if (retval)
		return;
	while (1) {
		retval = ext2fs_find_first_set_inode_bitmap2(
				ctx->inodes_to_rebuild, ino + 1,
				ctx->fs->super->s_inodes_count, &ino);
		if (retval)
			break;
		pctx.ino = ino;
		if (first) {
			fix_problem(ctx, pr_header, &pctx);
			first = 0;
		}
		pctx.errcode = rebuild_extent_tree(ctx, &list, ino);
		if (pctx.errcode) {
			end_problem_latch(ctx, PR_LATCH_OPTIMIZE_EXT);
			fix_problem(ctx, PR_1E_OPTIMIZE_EXT_ERR, &pctx);
		}
		if (ctx->progress && !ctx->progress_fd)
			e2fsck_simple_progress(ctx, "Rebuilding extents",
					100.0 * (float) ino /
					(float) ctx->fs->super->s_inodes_count,
					ino);
	}
	end_problem_latch(ctx, PR_LATCH_OPTIMIZE_EXT);

	ext2fs_free_inode_bitmap(ctx->inodes_to_rebuild);
	ctx->inodes_to_rebuild = NULL;
	ext2fs_free_mem(&list.extents);

	print_resource_track(ctx, pass_name, &rtrack, ctx->fs->io);
}

/* Scan a file to see if we should rebuild its extent tree */
errcode_t e2fsck_check_rebuild_extents(e2fsck_t ctx, ext2_ino_t ino,
				  struct ext2_inode *inode,
				  struct problem_context *pctx)
{
	struct extent_tree_info	eti;
	struct ext2_extent_info	info, top_info;
	struct ext2fs_extent	extent;
	ext2_extent_handle_t	ehandle;
	ext2_filsys		fs = ctx->fs;
	errcode_t		retval;

	/* block map file and we want extent conversion */
	if (!(inode->i_flags & EXT4_EXTENTS_FL) &&
	    !(inode->i_flags & EXT4_INLINE_DATA_FL) &&
	    (ctx->options & E2F_OPT_CONVERT_BMAP)) {
		return e2fsck_rebuild_extents_later(ctx, ino);
	}

	if (!(inode->i_flags & EXT4_EXTENTS_FL))
		return 0;
	memset(&eti, 0, sizeof(eti));
	eti.ino = ino;

	/* Otherwise, go scan the extent tree... */
	retval = ext2fs_extent_open2(fs, ino, inode, &ehandle);
	if (retval)
		return 0;

	retval = ext2fs_extent_get_info(ehandle, &top_info);
	if (retval)
		goto out;

	/* Check maximum extent depth */
	pctx->ino = ino;
	pctx->blk = top_info.max_depth;
	pctx->blk2 = ext2fs_max_extent_depth(ehandle);
	if (pctx->blk2 < pctx->blk &&
	    fix_problem(ctx, PR_1_EXTENT_BAD_MAX_DEPTH, pctx))
		eti.force_rebuild = 1;

	/* Can we collect extent tree level stats? */
	pctx->blk = MAX_EXTENT_DEPTH_COUNT;
	if (pctx->blk2 > pctx->blk)
		fix_problem(ctx, PR_1E_MAX_EXTENT_TREE_DEPTH, pctx);

	/* We need to fix tree depth problems, but the scan isn't a fix. */
	if (ctx->options & E2F_OPT_FIXES_ONLY)
		goto out;

	retval = ext2fs_extent_get(ehandle, EXT2_EXTENT_ROOT, &extent);
	if (retval)
		goto out;

	do {
		retval = ext2fs_extent_get_info(ehandle, &info);
		if (retval)
			break;

		/*
		 * If this is the first extent in an extent block that we
		 * haven't visited, collect stats on the block.
		 */
		if (info.curr_entry == 1 &&
		    !(extent.e_flags & EXT2_EXTENT_FLAGS_SECOND_VISIT) &&
		    !eti.force_rebuild) {
			struct extent_tree_level *etl;

			etl = eti.ext_info + info.curr_level;
			etl->num_extents += info.num_entries;
			etl->max_extents += info.max_entries;
			/*
			 * Implementation wart: Splitting extent blocks when
			 * appending will leave the old block with one free
			 * entry.  Therefore unless the node is totally full,
			 * pretend that a non-root extent block can hold one
			 * fewer entry than it actually does, so that we don't
			 * repeatedly rebuild the extent tree.
			 */
			if (info.curr_level &&
			    info.num_entries < info.max_entries)
				etl->max_extents--;
		}

		/* Skip to the end of a block of leaf nodes */
		if (extent.e_flags & EXT2_EXTENT_FLAGS_LEAF) {
			retval = ext2fs_extent_get(ehandle,
						    EXT2_EXTENT_LAST_SIB,
						    &extent);
			if (retval)
				break;
		}

		retval = ext2fs_extent_get(ehandle, EXT2_EXTENT_NEXT, &extent);
	} while (retval == 0);
out:
	ext2fs_extent_free(ehandle);
	return e2fsck_should_rebuild_extents(ctx, pctx, &eti, &top_info);
}

/* Having scanned a file's extent tree, decide if we should rebuild it */
errcode_t e2fsck_should_rebuild_extents(e2fsck_t ctx,
				   struct problem_context *pctx,
				   struct extent_tree_info *eti,
				   struct ext2_extent_info *info)
{
	struct extent_tree_level *ei;
	int i, j, op;
	unsigned int extents_per_block;

	if (eti->force_rebuild)
		goto rebuild;

	if (ctx->options & E2F_OPT_NOOPT_EXTENTS)
		return 0;

	extents_per_block = (ctx->fs->blocksize -
			     sizeof(struct ext3_extent_header)) /
			    sizeof(struct ext3_extent);
	/*
	 * If we can consolidate a level or shorten the tree, schedule the
	 * extent tree to be rebuilt.
	 */
	for (i = 0, ei = eti->ext_info; i < info->max_depth + 1; i++, ei++) {
		if (ei->max_extents - ei->num_extents > extents_per_block) {
			pctx->blk = i;
			op = PR_1E_CAN_NARROW_EXTENT_TREE;
			goto rebuild;
		}
		for (j = 0; j < i; j++) {
			if (ei->num_extents < eti->ext_info[j].max_extents) {
				pctx->blk = i;
				op = PR_1E_CAN_COLLAPSE_EXTENT_TREE;
				goto rebuild;
			}
		}
	}
	return 0;

rebuild:
	if (eti->force_rebuild || fix_problem(ctx, op, pctx))
		return e2fsck_rebuild_extents_later(ctx, eti->ino);
	return 0;
}

void e2fsck_pass1e(e2fsck_t ctx)
{
	rebuild_extents(ctx, "Pass 1E", PR_1E_PASS_HEADER);
}
