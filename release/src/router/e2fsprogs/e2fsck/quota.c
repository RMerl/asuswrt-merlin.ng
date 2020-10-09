/*
 * quota.c --- code for handling ext4 quota inodes
 *
 */

#include "config.h"
#ifdef HAVE_SYS_MOUNT_H
#include <sys/param.h>
#include <sys/mount.h>
#define MNT_FL (MS_MGC_VAL | MS_RDONLY)
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "e2fsck.h"
#include "problem.h"

static errcode_t move_quota_inode(ext2_filsys fs, ext2_ino_t from_ino,
				  ext2_ino_t to_ino, enum quota_type qtype)
{
	struct ext2_inode	inode;
	errcode_t		retval;
	char			qf_name[QUOTA_NAME_LEN];

	/* We need the inode bitmap to be loaded */
	retval = ext2fs_read_bitmaps(fs);
	if (retval) {
		com_err("ext2fs_read_bitmaps", retval, "%s",
			_("in move_quota_inode"));
		return retval;
	}

	retval = ext2fs_read_inode(fs, from_ino, &inode);
	if (retval) {
		com_err("ext2fs_read_inode", retval, "%s",
			_("in move_quota_inode"));
		return retval;
	}

	inode.i_links_count = 1;
	inode.i_mode = LINUX_S_IFREG | 0600;
	inode.i_flags = EXT2_IMMUTABLE_FL;
	if (ext2fs_has_feature_extents(fs->super))
		inode.i_flags |= EXT4_EXTENTS_FL;

	retval = ext2fs_write_new_inode(fs, to_ino, &inode);
	if (retval) {
		com_err("ext2fs_write_new_inode", retval, "%s",
			_("in move_quota_inode"));
		return retval;
	}

	/* unlink the old inode */
	quota_get_qf_name(qtype, QFMT_VFS_V1, qf_name);
	retval = ext2fs_unlink(fs, EXT2_ROOT_INO, qf_name, from_ino, 0);
	if (retval) {
		com_err("ext2fs_unlink", retval, "%s",
			_("in move_quota_inode"));
		return retval;
	}
	ext2fs_inode_alloc_stats(fs, from_ino, -1);
	/* Clear out the original inode in the inode-table block. */
	memset(&inode, 0, sizeof(struct ext2_inode));
	ext2fs_write_inode(fs, from_ino, &inode);
	return 0;
}

void e2fsck_hide_quota(e2fsck_t ctx)
{
	struct ext2_super_block *sb = ctx->fs->super;
	struct problem_context	pctx;
	ext2_filsys		fs = ctx->fs;
	enum quota_type qtype;
	ext2_ino_t quota_ino;

	clear_problem_context(&pctx);

	if ((ctx->options & E2F_OPT_READONLY) ||
	    !ext2fs_has_feature_quota(sb))
		return;

	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		pctx.dir = 2;	/* This is a guess, but it's a good one */
		pctx.ino = *quota_sb_inump(sb, qtype);
		pctx.num = qtype;
		quota_ino = quota_type2inum(qtype, fs->super);
		if (pctx.ino && (pctx.ino != quota_ino) &&
		    fix_problem(ctx, PR_0_HIDE_QUOTA, &pctx)) {
			if (move_quota_inode(fs, pctx.ino, quota_ino, qtype))
				continue;
			*quota_sb_inump(sb, qtype) = quota_ino;
			ext2fs_mark_super_dirty(fs);
		}
	}

	return;
}

void e2fsck_validate_quota_inodes(e2fsck_t ctx)
{
	struct ext2_super_block *sb = ctx->fs->super;
	struct problem_context	pctx;
	ext2_filsys		fs = ctx->fs;
	enum quota_type qtype;

	clear_problem_context(&pctx);

	for (qtype = 0; qtype < MAXQUOTAS; qtype++) {
		pctx.ino = *quota_sb_inump(sb, qtype);
		pctx.num = qtype;
		if (pctx.ino &&
		    ((pctx.ino == EXT2_BAD_INO) ||
		     (pctx.ino == EXT2_ROOT_INO) ||
		     (pctx.ino == EXT2_BOOT_LOADER_INO) ||
		     (pctx.ino == EXT2_UNDEL_DIR_INO) ||
		     (pctx.ino == EXT2_RESIZE_INO) ||
		     (pctx.ino == EXT2_JOURNAL_INO) ||
		     (pctx.ino == EXT2_EXCLUDE_INO) ||
		     (pctx.ino == EXT4_REPLICA_INO) ||
		     (pctx.ino > fs->super->s_inodes_count)) &&
		    fix_problem(ctx, PR_0_INVALID_QUOTA_INO, &pctx)) {
			*quota_sb_inump(sb, qtype) = 0;
			ext2fs_mark_super_dirty(fs);
		}
	}
}
