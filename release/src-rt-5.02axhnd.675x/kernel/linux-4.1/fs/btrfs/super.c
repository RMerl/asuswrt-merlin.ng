/*
 * Copyright (C) 2007 Oracle.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/backing-dev.h>
#include <linux/mount.h>
#include <linux/mpage.h>
#include <linux/swap.h>
#include <linux/writeback.h>
#include <linux/statfs.h>
#include <linux/compat.h>
#include <linux/parser.h>
#include <linux/ctype.h>
#include <linux/namei.h>
#include <linux/miscdevice.h>
#include <linux/magic.h>
#include <linux/slab.h>
#include <linux/cleancache.h>
#include <linux/ratelimit.h>
#include <linux/btrfs.h>
#include "delayed-inode.h"
#include "ctree.h"
#include "disk-io.h"
#include "transaction.h"
#include "btrfs_inode.h"
#include "print-tree.h"
#include "hash.h"
#include "props.h"
#include "xattr.h"
#include "volumes.h"
#include "export.h"
#include "compression.h"
#include "rcu-string.h"
#include "dev-replace.h"
#include "free-space-cache.h"
#include "backref.h"
#include "tests/btrfs-tests.h"

#include "qgroup.h"
#define CREATE_TRACE_POINTS
#include <trace/events/btrfs.h>

static const struct super_operations btrfs_super_ops;
static struct file_system_type btrfs_fs_type;

static int btrfs_remount(struct super_block *sb, int *flags, char *data);

static const char *btrfs_decode_error(int errno)
{
	char *errstr = "unknown";

	switch (errno) {
	case -EIO:
		errstr = "IO failure";
		break;
	case -ENOMEM:
		errstr = "Out of memory";
		break;
	case -EROFS:
		errstr = "Readonly filesystem";
		break;
	case -EEXIST:
		errstr = "Object already exists";
		break;
	case -ENOSPC:
		errstr = "No space left";
		break;
	case -ENOENT:
		errstr = "No such entry";
		break;
	}

	return errstr;
}

static void save_error_info(struct btrfs_fs_info *fs_info)
{
	/*
	 * today we only save the error info into ram.  Long term we'll
	 * also send it down to the disk
	 */
	set_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state);
}

/* btrfs handle error by forcing the filesystem readonly */
static void btrfs_handle_error(struct btrfs_fs_info *fs_info)
{
	struct super_block *sb = fs_info->sb;

	if (sb->s_flags & MS_RDONLY)
		return;

	if (test_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state)) {
		sb->s_flags |= MS_RDONLY;
		btrfs_info(fs_info, "forced readonly");
		/*
		 * Note that a running device replace operation is not
		 * canceled here although there is no way to update
		 * the progress. It would add the risk of a deadlock,
		 * therefore the canceling is ommited. The only penalty
		 * is that some I/O remains active until the procedure
		 * completes. The next time when the filesystem is
		 * mounted writeable again, the device replace
		 * operation continues.
		 */
	}
}

#ifdef CONFIG_PRINTK
/*
 * __btrfs_std_error decodes expected errors from the caller and
 * invokes the approciate error response.
 */
void __btrfs_std_error(struct btrfs_fs_info *fs_info, const char *function,
		       unsigned int line, int errno, const char *fmt, ...)
{
	struct super_block *sb = fs_info->sb;
	const char *errstr;

	/*
	 * Special case: if the error is EROFS, and we're already
	 * under MS_RDONLY, then it is safe here.
	 */
	if (errno == -EROFS && (sb->s_flags & MS_RDONLY))
  		return;

	errstr = btrfs_decode_error(errno);
	if (fmt) {
		struct va_format vaf;
		va_list args;

		va_start(args, fmt);
		vaf.fmt = fmt;
		vaf.va = &args;

		printk(KERN_CRIT
			"BTRFS: error (device %s) in %s:%d: errno=%d %s (%pV)\n",
			sb->s_id, function, line, errno, errstr, &vaf);
		va_end(args);
	} else {
		printk(KERN_CRIT "BTRFS: error (device %s) in %s:%d: errno=%d %s\n",
			sb->s_id, function, line, errno, errstr);
	}

	/* Don't go through full error handling during mount */
	save_error_info(fs_info);
	if (sb->s_flags & MS_BORN)
		btrfs_handle_error(fs_info);
}

static const char * const logtypes[] = {
	"emergency",
	"alert",
	"critical",
	"error",
	"warning",
	"notice",
	"info",
	"debug",
};

void btrfs_printk(const struct btrfs_fs_info *fs_info, const char *fmt, ...)
{
	struct super_block *sb = fs_info->sb;
	char lvl[4];
	struct va_format vaf;
	va_list args;
	const char *type = logtypes[4];
	int kern_level;

	va_start(args, fmt);

	kern_level = printk_get_level(fmt);
	if (kern_level) {
		size_t size = printk_skip_level(fmt) - fmt;
		memcpy(lvl, fmt,  size);
		lvl[size] = '\0';
		fmt += size;
		type = logtypes[kern_level - '0'];
	} else
		*lvl = '\0';

	vaf.fmt = fmt;
	vaf.va = &args;

	printk("%sBTRFS %s (device %s): %pV\n", lvl, type, sb->s_id, &vaf);

	va_end(args);
}

#else

void __btrfs_std_error(struct btrfs_fs_info *fs_info, const char *function,
		       unsigned int line, int errno, const char *fmt, ...)
{
	struct super_block *sb = fs_info->sb;

	/*
	 * Special case: if the error is EROFS, and we're already
	 * under MS_RDONLY, then it is safe here.
	 */
	if (errno == -EROFS && (sb->s_flags & MS_RDONLY))
		return;

	/* Don't go through full error handling during mount */
	if (sb->s_flags & MS_BORN) {
		save_error_info(fs_info);
		btrfs_handle_error(fs_info);
	}
}
#endif

/*
 * We only mark the transaction aborted and then set the file system read-only.
 * This will prevent new transactions from starting or trying to join this
 * one.
 *
 * This means that error recovery at the call site is limited to freeing
 * any local memory allocations and passing the error code up without
 * further cleanup. The transaction should complete as it normally would
 * in the call path but will return -EIO.
 *
 * We'll complete the cleanup in btrfs_end_transaction and
 * btrfs_commit_transaction.
 */
void __btrfs_abort_transaction(struct btrfs_trans_handle *trans,
			       struct btrfs_root *root, const char *function,
			       unsigned int line, int errno)
{
	/*
	 * Report first abort since mount
	 */
	if (!test_and_set_bit(BTRFS_FS_STATE_TRANS_ABORTED,
				&root->fs_info->fs_state)) {
		WARN(1, KERN_DEBUG "BTRFS: Transaction aborted (error %d)\n",
				errno);
	}
	trans->aborted = errno;
	/* Nothing used. The other threads that have joined this
	 * transaction may be able to continue. */
	if (!trans->dirty && list_empty(&trans->new_bgs)) {
		const char *errstr;

		errstr = btrfs_decode_error(errno);
		btrfs_warn(root->fs_info,
		           "%s:%d: Aborting unused transaction(%s).",
		           function, line, errstr);
		return;
	}
	ACCESS_ONCE(trans->transaction->aborted) = errno;
	/* Wake up anybody who may be waiting on this transaction */
	wake_up(&root->fs_info->transaction_wait);
	wake_up(&root->fs_info->transaction_blocked_wait);
	__btrfs_std_error(root->fs_info, function, line, errno, NULL);
}
/*
 * __btrfs_panic decodes unexpected, fatal errors from the caller,
 * issues an alert, and either panics or BUGs, depending on mount options.
 */
void __btrfs_panic(struct btrfs_fs_info *fs_info, const char *function,
		   unsigned int line, int errno, const char *fmt, ...)
{
	char *s_id = "<unknown>";
	const char *errstr;
	struct va_format vaf = { .fmt = fmt };
	va_list args;

	if (fs_info)
		s_id = fs_info->sb->s_id;

	va_start(args, fmt);
	vaf.va = &args;

	errstr = btrfs_decode_error(errno);
	if (fs_info && (fs_info->mount_opt & BTRFS_MOUNT_PANIC_ON_FATAL_ERROR))
		panic(KERN_CRIT "BTRFS panic (device %s) in %s:%d: %pV (errno=%d %s)\n",
			s_id, function, line, &vaf, errno, errstr);

	btrfs_crit(fs_info, "panic in %s:%d: %pV (errno=%d %s)",
		   function, line, &vaf, errno, errstr);
	va_end(args);
	/* Caller calls BUG() */
}

static void btrfs_put_super(struct super_block *sb)
{
	close_ctree(btrfs_sb(sb)->tree_root);
}

enum {
	Opt_degraded, Opt_subvol, Opt_subvolid, Opt_device, Opt_nodatasum,
	Opt_nodatacow, Opt_max_inline, Opt_alloc_start, Opt_nobarrier, Opt_ssd,
	Opt_nossd, Opt_ssd_spread, Opt_thread_pool, Opt_noacl, Opt_compress,
	Opt_compress_type, Opt_compress_force, Opt_compress_force_type,
	Opt_notreelog, Opt_ratio, Opt_flushoncommit, Opt_discard,
	Opt_space_cache, Opt_clear_cache, Opt_user_subvol_rm_allowed,
	Opt_enospc_debug, Opt_subvolrootid, Opt_defrag, Opt_inode_cache,
	Opt_no_space_cache, Opt_recovery, Opt_skip_balance,
	Opt_check_integrity, Opt_check_integrity_including_extent_data,
	Opt_check_integrity_print_mask, Opt_fatal_errors, Opt_rescan_uuid_tree,
	Opt_commit_interval, Opt_barrier, Opt_nodefrag, Opt_nodiscard,
	Opt_noenospc_debug, Opt_noflushoncommit, Opt_acl, Opt_datacow,
	Opt_datasum, Opt_treelog, Opt_noinode_cache,
	Opt_err,
};

static match_table_t tokens = {
	{Opt_degraded, "degraded"},
	{Opt_subvol, "subvol=%s"},
	{Opt_subvolid, "subvolid=%s"},
	{Opt_device, "device=%s"},
	{Opt_nodatasum, "nodatasum"},
	{Opt_datasum, "datasum"},
	{Opt_nodatacow, "nodatacow"},
	{Opt_datacow, "datacow"},
	{Opt_nobarrier, "nobarrier"},
	{Opt_barrier, "barrier"},
	{Opt_max_inline, "max_inline=%s"},
	{Opt_alloc_start, "alloc_start=%s"},
	{Opt_thread_pool, "thread_pool=%d"},
	{Opt_compress, "compress"},
	{Opt_compress_type, "compress=%s"},
	{Opt_compress_force, "compress-force"},
	{Opt_compress_force_type, "compress-force=%s"},
	{Opt_ssd, "ssd"},
	{Opt_ssd_spread, "ssd_spread"},
	{Opt_nossd, "nossd"},
	{Opt_acl, "acl"},
	{Opt_noacl, "noacl"},
	{Opt_notreelog, "notreelog"},
	{Opt_treelog, "treelog"},
	{Opt_flushoncommit, "flushoncommit"},
	{Opt_noflushoncommit, "noflushoncommit"},
	{Opt_ratio, "metadata_ratio=%d"},
	{Opt_discard, "discard"},
	{Opt_nodiscard, "nodiscard"},
	{Opt_space_cache, "space_cache"},
	{Opt_clear_cache, "clear_cache"},
	{Opt_user_subvol_rm_allowed, "user_subvol_rm_allowed"},
	{Opt_enospc_debug, "enospc_debug"},
	{Opt_noenospc_debug, "noenospc_debug"},
	{Opt_subvolrootid, "subvolrootid=%d"},
	{Opt_defrag, "autodefrag"},
	{Opt_nodefrag, "noautodefrag"},
	{Opt_inode_cache, "inode_cache"},
	{Opt_noinode_cache, "noinode_cache"},
	{Opt_no_space_cache, "nospace_cache"},
	{Opt_recovery, "recovery"},
	{Opt_skip_balance, "skip_balance"},
	{Opt_check_integrity, "check_int"},
	{Opt_check_integrity_including_extent_data, "check_int_data"},
	{Opt_check_integrity_print_mask, "check_int_print_mask=%d"},
	{Opt_rescan_uuid_tree, "rescan_uuid_tree"},
	{Opt_fatal_errors, "fatal_errors=%s"},
	{Opt_commit_interval, "commit=%d"},
	{Opt_err, NULL},
};

/*
 * Regular mount options parser.  Everything that is needed only when
 * reading in a new superblock is parsed here.
 * XXX JDM: This needs to be cleaned up for remount.
 */
int btrfs_parse_options(struct btrfs_root *root, char *options)
{
	struct btrfs_fs_info *info = root->fs_info;
	substring_t args[MAX_OPT_ARGS];
	char *p, *num, *orig = NULL;
	u64 cache_gen;
	int intarg;
	int ret = 0;
	char *compress_type;
	bool compress_force = false;

	cache_gen = btrfs_super_cache_generation(root->fs_info->super_copy);
	if (cache_gen)
		btrfs_set_opt(info->mount_opt, SPACE_CACHE);

	if (!options)
		goto out;

	/*
	 * strsep changes the string, duplicate it because parse_options
	 * gets called twice
	 */
	options = kstrdup(options, GFP_NOFS);
	if (!options)
		return -ENOMEM;

	orig = options;

	while ((p = strsep(&options, ",")) != NULL) {
		int token;
		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
		case Opt_degraded:
			btrfs_info(root->fs_info, "allowing degraded mounts");
			btrfs_set_opt(info->mount_opt, DEGRADED);
			break;
		case Opt_subvol:
		case Opt_subvolid:
		case Opt_subvolrootid:
		case Opt_device:
			/*
			 * These are parsed by btrfs_parse_early_options
			 * and can be happily ignored here.
			 */
			break;
		case Opt_nodatasum:
			btrfs_set_and_info(root, NODATASUM,
					   "setting nodatasum");
			break;
		case Opt_datasum:
			if (btrfs_test_opt(root, NODATASUM)) {
				if (btrfs_test_opt(root, NODATACOW))
					btrfs_info(root->fs_info, "setting datasum, datacow enabled");
				else
					btrfs_info(root->fs_info, "setting datasum");
			}
			btrfs_clear_opt(info->mount_opt, NODATACOW);
			btrfs_clear_opt(info->mount_opt, NODATASUM);
			break;
		case Opt_nodatacow:
			if (!btrfs_test_opt(root, NODATACOW)) {
				if (!btrfs_test_opt(root, COMPRESS) ||
				    !btrfs_test_opt(root, FORCE_COMPRESS)) {
					btrfs_info(root->fs_info,
						   "setting nodatacow, compression disabled");
				} else {
					btrfs_info(root->fs_info, "setting nodatacow");
				}
			}
			btrfs_clear_opt(info->mount_opt, COMPRESS);
			btrfs_clear_opt(info->mount_opt, FORCE_COMPRESS);
			btrfs_set_opt(info->mount_opt, NODATACOW);
			btrfs_set_opt(info->mount_opt, NODATASUM);
			break;
		case Opt_datacow:
			btrfs_clear_and_info(root, NODATACOW,
					     "setting datacow");
			break;
		case Opt_compress_force:
		case Opt_compress_force_type:
			compress_force = true;
			/* Fallthrough */
		case Opt_compress:
		case Opt_compress_type:
			if (token == Opt_compress ||
			    token == Opt_compress_force ||
			    strcmp(args[0].from, "zlib") == 0) {
				compress_type = "zlib";
				info->compress_type = BTRFS_COMPRESS_ZLIB;
				btrfs_set_opt(info->mount_opt, COMPRESS);
				btrfs_clear_opt(info->mount_opt, NODATACOW);
				btrfs_clear_opt(info->mount_opt, NODATASUM);
			} else if (strcmp(args[0].from, "lzo") == 0) {
				compress_type = "lzo";
				info->compress_type = BTRFS_COMPRESS_LZO;
				btrfs_set_opt(info->mount_opt, COMPRESS);
				btrfs_clear_opt(info->mount_opt, NODATACOW);
				btrfs_clear_opt(info->mount_opt, NODATASUM);
				btrfs_set_fs_incompat(info, COMPRESS_LZO);
			} else if (strncmp(args[0].from, "no", 2) == 0) {
				compress_type = "no";
				btrfs_clear_opt(info->mount_opt, COMPRESS);
				btrfs_clear_opt(info->mount_opt, FORCE_COMPRESS);
				compress_force = false;
			} else {
				ret = -EINVAL;
				goto out;
			}

			if (compress_force) {
				btrfs_set_and_info(root, FORCE_COMPRESS,
						   "force %s compression",
						   compress_type);
			} else {
				if (!btrfs_test_opt(root, COMPRESS))
					btrfs_info(root->fs_info,
						   "btrfs: use %s compression",
						   compress_type);
				/*
				 * If we remount from compress-force=xxx to
				 * compress=xxx, we need clear FORCE_COMPRESS
				 * flag, otherwise, there is no way for users
				 * to disable forcible compression separately.
				 */
				btrfs_clear_opt(info->mount_opt, FORCE_COMPRESS);
			}
			break;
		case Opt_ssd:
			btrfs_set_and_info(root, SSD,
					   "use ssd allocation scheme");
			break;
		case Opt_ssd_spread:
			btrfs_set_and_info(root, SSD_SPREAD,
					   "use spread ssd allocation scheme");
			btrfs_set_opt(info->mount_opt, SSD);
			break;
		case Opt_nossd:
			btrfs_set_and_info(root, NOSSD,
					     "not using ssd allocation scheme");
			btrfs_clear_opt(info->mount_opt, SSD);
			break;
		case Opt_barrier:
			btrfs_clear_and_info(root, NOBARRIER,
					     "turning on barriers");
			break;
		case Opt_nobarrier:
			btrfs_set_and_info(root, NOBARRIER,
					   "turning off barriers");
			break;
		case Opt_thread_pool:
			ret = match_int(&args[0], &intarg);
			if (ret) {
				goto out;
			} else if (intarg > 0) {
				info->thread_pool_size = intarg;
			} else {
				ret = -EINVAL;
				goto out;
			}
			break;
		case Opt_max_inline:
			num = match_strdup(&args[0]);
			if (num) {
				info->max_inline = memparse(num, NULL);
				kfree(num);

				if (info->max_inline) {
					info->max_inline = min_t(u64,
						info->max_inline,
						root->sectorsize);
				}
				btrfs_info(root->fs_info, "max_inline at %llu",
					info->max_inline);
			} else {
				ret = -ENOMEM;
				goto out;
			}
			break;
		case Opt_alloc_start:
			num = match_strdup(&args[0]);
			if (num) {
				mutex_lock(&info->chunk_mutex);
				info->alloc_start = memparse(num, NULL);
				mutex_unlock(&info->chunk_mutex);
				kfree(num);
				btrfs_info(root->fs_info, "allocations start at %llu",
					info->alloc_start);
			} else {
				ret = -ENOMEM;
				goto out;
			}
			break;
		case Opt_acl:
#ifdef CONFIG_BTRFS_FS_POSIX_ACL
			root->fs_info->sb->s_flags |= MS_POSIXACL;
			break;
#else
			btrfs_err(root->fs_info,
				"support for ACL not compiled in!");
			ret = -EINVAL;
			goto out;
#endif
		case Opt_noacl:
			root->fs_info->sb->s_flags &= ~MS_POSIXACL;
			break;
		case Opt_notreelog:
			btrfs_set_and_info(root, NOTREELOG,
					   "disabling tree log");
			break;
		case Opt_treelog:
			btrfs_clear_and_info(root, NOTREELOG,
					     "enabling tree log");
			break;
		case Opt_flushoncommit:
			btrfs_set_and_info(root, FLUSHONCOMMIT,
					   "turning on flush-on-commit");
			break;
		case Opt_noflushoncommit:
			btrfs_clear_and_info(root, FLUSHONCOMMIT,
					     "turning off flush-on-commit");
			break;
		case Opt_ratio:
			ret = match_int(&args[0], &intarg);
			if (ret) {
				goto out;
			} else if (intarg >= 0) {
				info->metadata_ratio = intarg;
				btrfs_info(root->fs_info, "metadata ratio %d",
				       info->metadata_ratio);
			} else {
				ret = -EINVAL;
				goto out;
			}
			break;
		case Opt_discard:
			btrfs_set_and_info(root, DISCARD,
					   "turning on discard");
			break;
		case Opt_nodiscard:
			btrfs_clear_and_info(root, DISCARD,
					     "turning off discard");
			break;
		case Opt_space_cache:
			btrfs_set_and_info(root, SPACE_CACHE,
					   "enabling disk space caching");
			break;
		case Opt_rescan_uuid_tree:
			btrfs_set_opt(info->mount_opt, RESCAN_UUID_TREE);
			break;
		case Opt_no_space_cache:
			btrfs_clear_and_info(root, SPACE_CACHE,
					     "disabling disk space caching");
			break;
		case Opt_inode_cache:
			btrfs_set_pending_and_info(info, INODE_MAP_CACHE,
					   "enabling inode map caching");
			break;
		case Opt_noinode_cache:
			btrfs_clear_pending_and_info(info, INODE_MAP_CACHE,
					     "disabling inode map caching");
			break;
		case Opt_clear_cache:
			btrfs_set_and_info(root, CLEAR_CACHE,
					   "force clearing of disk cache");
			break;
		case Opt_user_subvol_rm_allowed:
			btrfs_set_opt(info->mount_opt, USER_SUBVOL_RM_ALLOWED);
			break;
		case Opt_enospc_debug:
			btrfs_set_opt(info->mount_opt, ENOSPC_DEBUG);
			break;
		case Opt_noenospc_debug:
			btrfs_clear_opt(info->mount_opt, ENOSPC_DEBUG);
			break;
		case Opt_defrag:
			btrfs_set_and_info(root, AUTO_DEFRAG,
					   "enabling auto defrag");
			break;
		case Opt_nodefrag:
			btrfs_clear_and_info(root, AUTO_DEFRAG,
					     "disabling auto defrag");
			break;
		case Opt_recovery:
			btrfs_info(root->fs_info, "enabling auto recovery");
			btrfs_set_opt(info->mount_opt, RECOVERY);
			break;
		case Opt_skip_balance:
			btrfs_set_opt(info->mount_opt, SKIP_BALANCE);
			break;
#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
		case Opt_check_integrity_including_extent_data:
			btrfs_info(root->fs_info,
				   "enabling check integrity including extent data");
			btrfs_set_opt(info->mount_opt,
				      CHECK_INTEGRITY_INCLUDING_EXTENT_DATA);
			btrfs_set_opt(info->mount_opt, CHECK_INTEGRITY);
			break;
		case Opt_check_integrity:
			btrfs_info(root->fs_info, "enabling check integrity");
			btrfs_set_opt(info->mount_opt, CHECK_INTEGRITY);
			break;
		case Opt_check_integrity_print_mask:
			ret = match_int(&args[0], &intarg);
			if (ret) {
				goto out;
			} else if (intarg >= 0) {
				info->check_integrity_print_mask = intarg;
				btrfs_info(root->fs_info, "check_integrity_print_mask 0x%x",
				       info->check_integrity_print_mask);
			} else {
				ret = -EINVAL;
				goto out;
			}
			break;
#else
		case Opt_check_integrity_including_extent_data:
		case Opt_check_integrity:
		case Opt_check_integrity_print_mask:
			btrfs_err(root->fs_info,
				"support for check_integrity* not compiled in!");
			ret = -EINVAL;
			goto out;
#endif
		case Opt_fatal_errors:
			if (strcmp(args[0].from, "panic") == 0)
				btrfs_set_opt(info->mount_opt,
					      PANIC_ON_FATAL_ERROR);
			else if (strcmp(args[0].from, "bug") == 0)
				btrfs_clear_opt(info->mount_opt,
					      PANIC_ON_FATAL_ERROR);
			else {
				ret = -EINVAL;
				goto out;
			}
			break;
		case Opt_commit_interval:
			intarg = 0;
			ret = match_int(&args[0], &intarg);
			if (ret < 0) {
				btrfs_err(root->fs_info, "invalid commit interval");
				ret = -EINVAL;
				goto out;
			}
			if (intarg > 0) {
				if (intarg > 300) {
					btrfs_warn(root->fs_info, "excessive commit interval %d",
							intarg);
				}
				info->commit_interval = intarg;
			} else {
				btrfs_info(root->fs_info, "using default commit interval %ds",
				    BTRFS_DEFAULT_COMMIT_INTERVAL);
				info->commit_interval = BTRFS_DEFAULT_COMMIT_INTERVAL;
			}
			break;
		case Opt_err:
			btrfs_info(root->fs_info, "unrecognized mount option '%s'", p);
			ret = -EINVAL;
			goto out;
		default:
			break;
		}
	}
out:
	if (!ret && btrfs_test_opt(root, SPACE_CACHE))
		btrfs_info(root->fs_info, "disk space caching is enabled");
	kfree(orig);
	return ret;
}

/*
 * Parse mount options that are required early in the mount process.
 *
 * All other options will be parsed on much later in the mount process and
 * only when we need to allocate a new super block.
 */
static int btrfs_parse_early_options(const char *options, fmode_t flags,
		void *holder, char **subvol_name, u64 *subvol_objectid,
		struct btrfs_fs_devices **fs_devices)
{
	substring_t args[MAX_OPT_ARGS];
	char *device_name, *opts, *orig, *p;
	char *num = NULL;
	int error = 0;

	if (!options)
		return 0;

	/*
	 * strsep changes the string, duplicate it because parse_options
	 * gets called twice
	 */
	opts = kstrdup(options, GFP_KERNEL);
	if (!opts)
		return -ENOMEM;
	orig = opts;

	while ((p = strsep(&opts, ",")) != NULL) {
		int token;
		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
		case Opt_subvol:
			kfree(*subvol_name);
			*subvol_name = match_strdup(&args[0]);
			if (!*subvol_name) {
				error = -ENOMEM;
				goto out;
			}
			break;
		case Opt_subvolid:
			num = match_strdup(&args[0]);
			if (num) {
				*subvol_objectid = memparse(num, NULL);
				kfree(num);
				/* we want the original fs_tree */
				if (!*subvol_objectid)
					*subvol_objectid =
						BTRFS_FS_TREE_OBJECTID;
			} else {
				error = -EINVAL;
				goto out;
			}
			break;
		case Opt_subvolrootid:
			printk(KERN_WARNING
				"BTRFS: 'subvolrootid' mount option is deprecated and has "
				"no effect\n");
			break;
		case Opt_device:
			device_name = match_strdup(&args[0]);
			if (!device_name) {
				error = -ENOMEM;
				goto out;
			}
			error = btrfs_scan_one_device(device_name,
					flags, holder, fs_devices);
			kfree(device_name);
			if (error)
				goto out;
			break;
		default:
			break;
		}
	}

out:
	kfree(orig);
	return error;
}

static struct dentry *get_default_root(struct super_block *sb,
				       u64 subvol_objectid)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	struct btrfs_root *root = fs_info->tree_root;
	struct btrfs_root *new_root;
	struct btrfs_dir_item *di;
	struct btrfs_path *path;
	struct btrfs_key location;
	struct inode *inode;
	u64 dir_id;
	int new = 0;

	/*
	 * We have a specific subvol we want to mount, just setup location and
	 * go look up the root.
	 */
	if (subvol_objectid) {
		location.objectid = subvol_objectid;
		location.type = BTRFS_ROOT_ITEM_KEY;
		location.offset = (u64)-1;
		goto find_root;
	}

	path = btrfs_alloc_path();
	if (!path)
		return ERR_PTR(-ENOMEM);
	path->leave_spinning = 1;

	/*
	 * Find the "default" dir item which points to the root item that we
	 * will mount by default if we haven't been given a specific subvolume
	 * to mount.
	 */
	dir_id = btrfs_super_root_dir(fs_info->super_copy);
	di = btrfs_lookup_dir_item(NULL, root, path, dir_id, "default", 7, 0);
	if (IS_ERR(di)) {
		btrfs_free_path(path);
		return ERR_CAST(di);
	}
	if (!di) {
		/*
		 * Ok the default dir item isn't there.  This is weird since
		 * it's always been there, but don't freak out, just try and
		 * mount to root most subvolume.
		 */
		btrfs_free_path(path);
		dir_id = BTRFS_FIRST_FREE_OBJECTID;
		new_root = fs_info->fs_root;
		goto setup_root;
	}

	btrfs_dir_item_key_to_cpu(path->nodes[0], di, &location);
	btrfs_free_path(path);

find_root:
	new_root = btrfs_read_fs_root_no_name(fs_info, &location);
	if (IS_ERR(new_root))
		return ERR_CAST(new_root);

	if (!(sb->s_flags & MS_RDONLY)) {
		int ret;
		down_read(&fs_info->cleanup_work_sem);
		ret = btrfs_orphan_cleanup(new_root);
		up_read(&fs_info->cleanup_work_sem);
		if (ret)
			return ERR_PTR(ret);
	}

	dir_id = btrfs_root_dirid(&new_root->root_item);
setup_root:
	location.objectid = dir_id;
	location.type = BTRFS_INODE_ITEM_KEY;
	location.offset = 0;

	inode = btrfs_iget(sb, &location, new_root, &new);
	if (IS_ERR(inode))
		return ERR_CAST(inode);

	/*
	 * If we're just mounting the root most subvol put the inode and return
	 * a reference to the dentry.  We will have already gotten a reference
	 * to the inode in btrfs_fill_super so we're good to go.
	 */
	if (!new && d_inode(sb->s_root) == inode) {
		iput(inode);
		return dget(sb->s_root);
	}

	return d_obtain_root(inode);
}

static int btrfs_fill_super(struct super_block *sb,
			    struct btrfs_fs_devices *fs_devices,
			    void *data, int silent)
{
	struct inode *inode;
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	struct btrfs_key key;
	int err;

	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_magic = BTRFS_SUPER_MAGIC;
	sb->s_op = &btrfs_super_ops;
	sb->s_d_op = &btrfs_dentry_operations;
	sb->s_export_op = &btrfs_export_ops;
	sb->s_xattr = btrfs_xattr_handlers;
	sb->s_time_gran = 1;
#ifdef CONFIG_BTRFS_FS_POSIX_ACL
	sb->s_flags |= MS_POSIXACL;
#endif
	sb->s_flags |= MS_I_VERSION;
	err = open_ctree(sb, fs_devices, (char *)data);
	if (err) {
		printk(KERN_ERR "BTRFS: open_ctree failed\n");
		return err;
	}

	key.objectid = BTRFS_FIRST_FREE_OBJECTID;
	key.type = BTRFS_INODE_ITEM_KEY;
	key.offset = 0;
	inode = btrfs_iget(sb, &key, fs_info->fs_root, NULL);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto fail_close;
	}

	sb->s_root = d_make_root(inode);
	if (!sb->s_root) {
		err = -ENOMEM;
		goto fail_close;
	}

	save_mount_options(sb, data);
	cleancache_init_fs(sb);
	sb->s_flags |= MS_ACTIVE;
	return 0;

fail_close:
	close_ctree(fs_info->tree_root);
	return err;
}

int btrfs_sync_fs(struct super_block *sb, int wait)
{
	struct btrfs_trans_handle *trans;
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	struct btrfs_root *root = fs_info->tree_root;

	trace_btrfs_sync_fs(wait);

	if (!wait) {
		filemap_flush(fs_info->btree_inode->i_mapping);
		return 0;
	}

	btrfs_wait_ordered_roots(fs_info, -1);

	trans = btrfs_attach_transaction_barrier(root);
	if (IS_ERR(trans)) {
		/* no transaction, don't bother */
		if (PTR_ERR(trans) == -ENOENT) {
			/*
			 * Exit unless we have some pending changes
			 * that need to go through commit
			 */
			if (fs_info->pending_changes == 0)
				return 0;
			/*
			 * A non-blocking test if the fs is frozen. We must not
			 * start a new transaction here otherwise a deadlock
			 * happens. The pending operations are delayed to the
			 * next commit after thawing.
			 */
			if (__sb_start_write(sb, SB_FREEZE_WRITE, false))
				__sb_end_write(sb, SB_FREEZE_WRITE);
			else
				return 0;
			trans = btrfs_start_transaction(root, 0);
		}
		if (IS_ERR(trans))
			return PTR_ERR(trans);
	}
	return btrfs_commit_transaction(trans, root);
}

static int btrfs_show_options(struct seq_file *seq, struct dentry *dentry)
{
	struct btrfs_fs_info *info = btrfs_sb(dentry->d_sb);
	struct btrfs_root *root = info->tree_root;
	char *compress_type;

	if (btrfs_test_opt(root, DEGRADED))
		seq_puts(seq, ",degraded");
	if (btrfs_test_opt(root, NODATASUM))
		seq_puts(seq, ",nodatasum");
	if (btrfs_test_opt(root, NODATACOW))
		seq_puts(seq, ",nodatacow");
	if (btrfs_test_opt(root, NOBARRIER))
		seq_puts(seq, ",nobarrier");
	if (info->max_inline != BTRFS_DEFAULT_MAX_INLINE)
		seq_printf(seq, ",max_inline=%llu", info->max_inline);
	if (info->alloc_start != 0)
		seq_printf(seq, ",alloc_start=%llu", info->alloc_start);
	if (info->thread_pool_size !=  min_t(unsigned long,
					     num_online_cpus() + 2, 8))
		seq_printf(seq, ",thread_pool=%d", info->thread_pool_size);
	if (btrfs_test_opt(root, COMPRESS)) {
		if (info->compress_type == BTRFS_COMPRESS_ZLIB)
			compress_type = "zlib";
		else
			compress_type = "lzo";
		if (btrfs_test_opt(root, FORCE_COMPRESS))
			seq_printf(seq, ",compress-force=%s", compress_type);
		else
			seq_printf(seq, ",compress=%s", compress_type);
	}
	if (btrfs_test_opt(root, NOSSD))
		seq_puts(seq, ",nossd");
	if (btrfs_test_opt(root, SSD_SPREAD))
		seq_puts(seq, ",ssd_spread");
	else if (btrfs_test_opt(root, SSD))
		seq_puts(seq, ",ssd");
	if (btrfs_test_opt(root, NOTREELOG))
		seq_puts(seq, ",notreelog");
	if (btrfs_test_opt(root, FLUSHONCOMMIT))
		seq_puts(seq, ",flushoncommit");
	if (btrfs_test_opt(root, DISCARD))
		seq_puts(seq, ",discard");
	if (!(root->fs_info->sb->s_flags & MS_POSIXACL))
		seq_puts(seq, ",noacl");
	if (btrfs_test_opt(root, SPACE_CACHE))
		seq_puts(seq, ",space_cache");
	else
		seq_puts(seq, ",nospace_cache");
	if (btrfs_test_opt(root, RESCAN_UUID_TREE))
		seq_puts(seq, ",rescan_uuid_tree");
	if (btrfs_test_opt(root, CLEAR_CACHE))
		seq_puts(seq, ",clear_cache");
	if (btrfs_test_opt(root, USER_SUBVOL_RM_ALLOWED))
		seq_puts(seq, ",user_subvol_rm_allowed");
	if (btrfs_test_opt(root, ENOSPC_DEBUG))
		seq_puts(seq, ",enospc_debug");
	if (btrfs_test_opt(root, AUTO_DEFRAG))
		seq_puts(seq, ",autodefrag");
	if (btrfs_test_opt(root, INODE_MAP_CACHE))
		seq_puts(seq, ",inode_cache");
	if (btrfs_test_opt(root, SKIP_BALANCE))
		seq_puts(seq, ",skip_balance");
	if (btrfs_test_opt(root, RECOVERY))
		seq_puts(seq, ",recovery");
#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
	if (btrfs_test_opt(root, CHECK_INTEGRITY_INCLUDING_EXTENT_DATA))
		seq_puts(seq, ",check_int_data");
	else if (btrfs_test_opt(root, CHECK_INTEGRITY))
		seq_puts(seq, ",check_int");
	if (info->check_integrity_print_mask)
		seq_printf(seq, ",check_int_print_mask=%d",
				info->check_integrity_print_mask);
#endif
	if (info->metadata_ratio)
		seq_printf(seq, ",metadata_ratio=%d",
				info->metadata_ratio);
	if (btrfs_test_opt(root, PANIC_ON_FATAL_ERROR))
		seq_puts(seq, ",fatal_errors=panic");
	if (info->commit_interval != BTRFS_DEFAULT_COMMIT_INTERVAL)
		seq_printf(seq, ",commit=%d", info->commit_interval);
	return 0;
}

static int btrfs_test_super(struct super_block *s, void *data)
{
	struct btrfs_fs_info *p = data;
	struct btrfs_fs_info *fs_info = btrfs_sb(s);

	return fs_info->fs_devices == p->fs_devices;
}

static int btrfs_set_super(struct super_block *s, void *data)
{
	int err = set_anon_super(s, data);
	if (!err)
		s->s_fs_info = data;
	return err;
}

/*
 * subvolumes are identified by ino 256
 */
static inline int is_subvolume_inode(struct inode *inode)
{
	if (inode && inode->i_ino == BTRFS_FIRST_FREE_OBJECTID)
		return 1;
	return 0;
}

/*
 * This will strip out the subvol=%s argument for an argument string and add
 * subvolid=0 to make sure we get the actual tree root for path walking to the
 * subvol we want.
 */
static char *setup_root_args(char *args)
{
	unsigned len = strlen(args) + 2 + 1;
	char *src, *dst, *buf;

	/*
	 * We need the same args as before, but with this substitution:
	 * s!subvol=[^,]+!subvolid=0!
	 *
	 * Since the replacement string is up to 2 bytes longer than the
	 * original, allocate strlen(args) + 2 + 1 bytes.
	 */

	src = strstr(args, "subvol=");
	/* This shouldn't happen, but just in case.. */
	if (!src)
		return NULL;

	buf = dst = kmalloc(len, GFP_NOFS);
	if (!buf)
		return NULL;

	/*
	 * If the subvol= arg is not at the start of the string,
	 * copy whatever precedes it into buf.
	 */
	if (src != args) {
		*src++ = '\0';
		strcpy(buf, args);
		dst += strlen(args);
	}

	strcpy(dst, "subvolid=0");
	dst += strlen("subvolid=0");

	/*
	 * If there is a "," after the original subvol=... string,
	 * copy that suffix into our buffer.  Otherwise, we're done.
	 */
	src = strchr(src, ',');
	if (src)
		strcpy(dst, src);

	return buf;
}

static struct dentry *mount_subvol(const char *subvol_name, int flags,
				   const char *device_name, char *data)
{
	struct dentry *root;
	struct vfsmount *mnt;
	char *newargs;

	newargs = setup_root_args(data);
	if (!newargs)
		return ERR_PTR(-ENOMEM);
	mnt = vfs_kern_mount(&btrfs_fs_type, flags, device_name,
			     newargs);

	if (PTR_RET(mnt) == -EBUSY) {
		if (flags & MS_RDONLY) {
			mnt = vfs_kern_mount(&btrfs_fs_type, flags & ~MS_RDONLY, device_name,
					     newargs);
		} else {
			int r;
			mnt = vfs_kern_mount(&btrfs_fs_type, flags | MS_RDONLY, device_name,
					     newargs);
			if (IS_ERR(mnt)) {
				kfree(newargs);
				return ERR_CAST(mnt);
			}

			r = btrfs_remount(mnt->mnt_sb, &flags, NULL);
			if (r < 0) {
				/* FIXME: release vfsmount mnt ??*/
				kfree(newargs);
				return ERR_PTR(r);
			}
		}
	}

	kfree(newargs);

	if (IS_ERR(mnt))
		return ERR_CAST(mnt);

	root = mount_subtree(mnt, subvol_name);

	if (!IS_ERR(root) && !is_subvolume_inode(d_inode(root))) {
		struct super_block *s = root->d_sb;
		dput(root);
		root = ERR_PTR(-EINVAL);
		deactivate_locked_super(s);
		printk(KERN_ERR "BTRFS: '%s' is not a valid subvolume\n",
				subvol_name);
	}

	return root;
}

static int parse_security_options(char *orig_opts,
				  struct security_mnt_opts *sec_opts)
{
	char *secdata = NULL;
	int ret = 0;

	secdata = alloc_secdata();
	if (!secdata)
		return -ENOMEM;
	ret = security_sb_copy_data(orig_opts, secdata);
	if (ret) {
		free_secdata(secdata);
		return ret;
	}
	ret = security_sb_parse_opts_str(secdata, sec_opts);
	free_secdata(secdata);
	return ret;
}

static int setup_security_options(struct btrfs_fs_info *fs_info,
				  struct super_block *sb,
				  struct security_mnt_opts *sec_opts)
{
	int ret = 0;

	/*
	 * Call security_sb_set_mnt_opts() to check whether new sec_opts
	 * is valid.
	 */
	ret = security_sb_set_mnt_opts(sb, sec_opts, 0, NULL);
	if (ret)
		return ret;

#ifdef CONFIG_SECURITY
	if (!fs_info->security_opts.num_mnt_opts) {
		/* first time security setup, copy sec_opts to fs_info */
		memcpy(&fs_info->security_opts, sec_opts, sizeof(*sec_opts));
	} else {
		/*
		 * Since SELinux(the only one supports security_mnt_opts) does
		 * NOT support changing context during remount/mount same sb,
		 * This must be the same or part of the same security options,
		 * just free it.
		 */
		security_free_mnt_opts(sec_opts);
	}
#endif
	return ret;
}

/*
 * Find a superblock for the given device / mount point.
 *
 * Note:  This is based on get_sb_bdev from fs/super.c with a few additions
 *	  for multiple device setup.  Make sure to keep it in sync.
 */
static struct dentry *btrfs_mount(struct file_system_type *fs_type, int flags,
		const char *device_name, void *data)
{
	struct block_device *bdev = NULL;
	struct super_block *s;
	struct dentry *root;
	struct btrfs_fs_devices *fs_devices = NULL;
	struct btrfs_fs_info *fs_info = NULL;
	struct security_mnt_opts new_sec_opts;
	fmode_t mode = FMODE_READ;
	char *subvol_name = NULL;
	u64 subvol_objectid = 0;
	int error = 0;

	if (!(flags & MS_RDONLY))
		mode |= FMODE_WRITE;

	error = btrfs_parse_early_options(data, mode, fs_type,
					  &subvol_name, &subvol_objectid,
					  &fs_devices);
	if (error) {
		kfree(subvol_name);
		return ERR_PTR(error);
	}

	if (subvol_name) {
		root = mount_subvol(subvol_name, flags, device_name, data);
		kfree(subvol_name);
		return root;
	}

	security_init_mnt_opts(&new_sec_opts);
	if (data) {
		error = parse_security_options(data, &new_sec_opts);
		if (error)
			return ERR_PTR(error);
	}

	error = btrfs_scan_one_device(device_name, mode, fs_type, &fs_devices);
	if (error)
		goto error_sec_opts;

	/*
	 * Setup a dummy root and fs_info for test/set super.  This is because
	 * we don't actually fill this stuff out until open_ctree, but we need
	 * it for searching for existing supers, so this lets us do that and
	 * then open_ctree will properly initialize everything later.
	 */
	fs_info = kzalloc(sizeof(struct btrfs_fs_info), GFP_NOFS);
	if (!fs_info) {
		error = -ENOMEM;
		goto error_sec_opts;
	}

	fs_info->fs_devices = fs_devices;

	fs_info->super_copy = kzalloc(BTRFS_SUPER_INFO_SIZE, GFP_NOFS);
	fs_info->super_for_commit = kzalloc(BTRFS_SUPER_INFO_SIZE, GFP_NOFS);
	security_init_mnt_opts(&fs_info->security_opts);
	if (!fs_info->super_copy || !fs_info->super_for_commit) {
		error = -ENOMEM;
		goto error_fs_info;
	}

	error = btrfs_open_devices(fs_devices, mode, fs_type);
	if (error)
		goto error_fs_info;

	if (!(flags & MS_RDONLY) && fs_devices->rw_devices == 0) {
		error = -EACCES;
		goto error_close_devices;
	}

	bdev = fs_devices->latest_bdev;
	s = sget(fs_type, btrfs_test_super, btrfs_set_super, flags | MS_NOSEC,
		 fs_info);
	if (IS_ERR(s)) {
		error = PTR_ERR(s);
		goto error_close_devices;
	}

	if (s->s_root) {
		btrfs_close_devices(fs_devices);
		free_fs_info(fs_info);
		if ((flags ^ s->s_flags) & MS_RDONLY)
			error = -EBUSY;
	} else {
		char b[BDEVNAME_SIZE];

		strlcpy(s->s_id, bdevname(bdev, b), sizeof(s->s_id));
		btrfs_sb(s)->bdev_holder = fs_type;
		error = btrfs_fill_super(s, fs_devices, data,
					 flags & MS_SILENT ? 1 : 0);
	}

	root = !error ? get_default_root(s, subvol_objectid) : ERR_PTR(error);
	if (IS_ERR(root)) {
		deactivate_locked_super(s);
		error = PTR_ERR(root);
		goto error_sec_opts;
	}

	fs_info = btrfs_sb(s);
	error = setup_security_options(fs_info, s, &new_sec_opts);
	if (error) {
		dput(root);
		deactivate_locked_super(s);
		goto error_sec_opts;
	}

	return root;

error_close_devices:
	btrfs_close_devices(fs_devices);
error_fs_info:
	free_fs_info(fs_info);
error_sec_opts:
	security_free_mnt_opts(&new_sec_opts);
	return ERR_PTR(error);
}

static void btrfs_resize_thread_pool(struct btrfs_fs_info *fs_info,
				     int new_pool_size, int old_pool_size)
{
	if (new_pool_size == old_pool_size)
		return;

	fs_info->thread_pool_size = new_pool_size;

	btrfs_info(fs_info, "resize thread pool %d -> %d",
	       old_pool_size, new_pool_size);

	btrfs_workqueue_set_max(fs_info->workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->delalloc_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->submit_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->caching_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->endio_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->endio_meta_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->endio_meta_write_workers,
				new_pool_size);
	btrfs_workqueue_set_max(fs_info->endio_write_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->endio_freespace_worker, new_pool_size);
	btrfs_workqueue_set_max(fs_info->delayed_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->readahead_workers, new_pool_size);
	btrfs_workqueue_set_max(fs_info->scrub_wr_completion_workers,
				new_pool_size);
}

static inline void btrfs_remount_prepare(struct btrfs_fs_info *fs_info)
{
	set_bit(BTRFS_FS_STATE_REMOUNTING, &fs_info->fs_state);
}

static inline void btrfs_remount_begin(struct btrfs_fs_info *fs_info,
				       unsigned long old_opts, int flags)
{
	if (btrfs_raw_test_opt(old_opts, AUTO_DEFRAG) &&
	    (!btrfs_raw_test_opt(fs_info->mount_opt, AUTO_DEFRAG) ||
	     (flags & MS_RDONLY))) {
		/* wait for any defraggers to finish */
		wait_event(fs_info->transaction_wait,
			   (atomic_read(&fs_info->defrag_running) == 0));
		if (flags & MS_RDONLY)
			sync_filesystem(fs_info->sb);
	}
}

static inline void btrfs_remount_cleanup(struct btrfs_fs_info *fs_info,
					 unsigned long old_opts)
{
	/*
	 * We need cleanup all defragable inodes if the autodefragment is
	 * close or the fs is R/O.
	 */
	if (btrfs_raw_test_opt(old_opts, AUTO_DEFRAG) &&
	    (!btrfs_raw_test_opt(fs_info->mount_opt, AUTO_DEFRAG) ||
	     (fs_info->sb->s_flags & MS_RDONLY))) {
		btrfs_cleanup_defrag_inodes(fs_info);
	}

	clear_bit(BTRFS_FS_STATE_REMOUNTING, &fs_info->fs_state);
}

static int btrfs_remount(struct super_block *sb, int *flags, char *data)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	struct btrfs_root *root = fs_info->tree_root;
	unsigned old_flags = sb->s_flags;
	unsigned long old_opts = fs_info->mount_opt;
	unsigned long old_compress_type = fs_info->compress_type;
	u64 old_max_inline = fs_info->max_inline;
	u64 old_alloc_start = fs_info->alloc_start;
	int old_thread_pool_size = fs_info->thread_pool_size;
	unsigned int old_metadata_ratio = fs_info->metadata_ratio;
	int ret;

	sync_filesystem(sb);
	btrfs_remount_prepare(fs_info);

	if (data) {
		struct security_mnt_opts new_sec_opts;

		security_init_mnt_opts(&new_sec_opts);
		ret = parse_security_options(data, &new_sec_opts);
		if (ret)
			goto restore;
		ret = setup_security_options(fs_info, sb,
					     &new_sec_opts);
		if (ret) {
			security_free_mnt_opts(&new_sec_opts);
			goto restore;
		}
	}

	ret = btrfs_parse_options(root, data);
	if (ret) {
		ret = -EINVAL;
		goto restore;
	}

	btrfs_remount_begin(fs_info, old_opts, *flags);
	btrfs_resize_thread_pool(fs_info,
		fs_info->thread_pool_size, old_thread_pool_size);

	if ((*flags & MS_RDONLY) == (sb->s_flags & MS_RDONLY))
		goto out;

	if (*flags & MS_RDONLY) {
		/*
		 * this also happens on 'umount -rf' or on shutdown, when
		 * the filesystem is busy.
		 */
		cancel_work_sync(&fs_info->async_reclaim_work);

		/* wait for the uuid_scan task to finish */
		down(&fs_info->uuid_tree_rescan_sem);
		/* avoid complains from lockdep et al. */
		up(&fs_info->uuid_tree_rescan_sem);

		sb->s_flags |= MS_RDONLY;

		btrfs_dev_replace_suspend_for_unmount(fs_info);
		btrfs_scrub_cancel(fs_info);
		btrfs_pause_balance(fs_info);

		ret = btrfs_commit_super(root);
		if (ret)
			goto restore;
	} else {
		if (test_bit(BTRFS_FS_STATE_ERROR, &root->fs_info->fs_state)) {
			btrfs_err(fs_info,
				"Remounting read-write after error is not allowed");
			ret = -EINVAL;
			goto restore;
		}
		if (fs_info->fs_devices->rw_devices == 0) {
			ret = -EACCES;
			goto restore;
		}

		if (fs_info->fs_devices->missing_devices >
		     fs_info->num_tolerated_disk_barrier_failures &&
		    !(*flags & MS_RDONLY)) {
			btrfs_warn(fs_info,
				"too many missing devices, writeable remount is not allowed");
			ret = -EACCES;
			goto restore;
		}

		if (btrfs_super_log_root(fs_info->super_copy) != 0) {
			ret = -EINVAL;
			goto restore;
		}

		ret = btrfs_cleanup_fs_roots(fs_info);
		if (ret)
			goto restore;

		/* recover relocation */
		mutex_lock(&fs_info->cleaner_mutex);
		ret = btrfs_recover_relocation(root);
		mutex_unlock(&fs_info->cleaner_mutex);
		if (ret)
			goto restore;

		ret = btrfs_resume_balance_async(fs_info);
		if (ret)
			goto restore;

		ret = btrfs_resume_dev_replace_async(fs_info);
		if (ret) {
			btrfs_warn(fs_info, "failed to resume dev_replace");
			goto restore;
		}

		btrfs_qgroup_rescan_resume(fs_info);

		if (!fs_info->uuid_root) {
			btrfs_info(fs_info, "creating UUID tree");
			ret = btrfs_create_uuid_tree(fs_info);
			if (ret) {
				btrfs_warn(fs_info, "failed to create the UUID tree %d", ret);
				goto restore;
			}
		}
		sb->s_flags &= ~MS_RDONLY;
	}
out:
	wake_up_process(fs_info->transaction_kthread);
	btrfs_remount_cleanup(fs_info, old_opts);
	return 0;

restore:
	/* We've hit an error - don't reset MS_RDONLY */
	if (sb->s_flags & MS_RDONLY)
		old_flags |= MS_RDONLY;
	sb->s_flags = old_flags;
	fs_info->mount_opt = old_opts;
	fs_info->compress_type = old_compress_type;
	fs_info->max_inline = old_max_inline;
	mutex_lock(&fs_info->chunk_mutex);
	fs_info->alloc_start = old_alloc_start;
	mutex_unlock(&fs_info->chunk_mutex);
	btrfs_resize_thread_pool(fs_info,
		old_thread_pool_size, fs_info->thread_pool_size);
	fs_info->metadata_ratio = old_metadata_ratio;
	btrfs_remount_cleanup(fs_info, old_opts);
	return ret;
}

/* Used to sort the devices by max_avail(descending sort) */
static int btrfs_cmp_device_free_bytes(const void *dev_info1,
				       const void *dev_info2)
{
	if (((struct btrfs_device_info *)dev_info1)->max_avail >
	    ((struct btrfs_device_info *)dev_info2)->max_avail)
		return -1;
	else if (((struct btrfs_device_info *)dev_info1)->max_avail <
		 ((struct btrfs_device_info *)dev_info2)->max_avail)
		return 1;
	else
	return 0;
}

/*
 * sort the devices by max_avail, in which max free extent size of each device
 * is stored.(Descending Sort)
 */
static inline void btrfs_descending_sort_devices(
					struct btrfs_device_info *devices,
					size_t nr_devices)
{
	sort(devices, nr_devices, sizeof(struct btrfs_device_info),
	     btrfs_cmp_device_free_bytes, NULL);
}

/*
 * The helper to calc the free space on the devices that can be used to store
 * file data.
 */
static int btrfs_calc_avail_data_space(struct btrfs_root *root, u64 *free_bytes)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_device_info *devices_info;
	struct btrfs_fs_devices *fs_devices = fs_info->fs_devices;
	struct btrfs_device *device;
	u64 skip_space;
	u64 type;
	u64 avail_space;
	u64 used_space;
	u64 min_stripe_size;
	int min_stripes = 1, num_stripes = 1;
	int i = 0, nr_devices;
	int ret;

	/*
	 * We aren't under the device list lock, so this is racey-ish, but good
	 * enough for our purposes.
	 */
	nr_devices = fs_info->fs_devices->open_devices;
	if (!nr_devices) {
		smp_mb();
		nr_devices = fs_info->fs_devices->open_devices;
		ASSERT(nr_devices);
		if (!nr_devices) {
			*free_bytes = 0;
			return 0;
		}
	}

	devices_info = kmalloc_array(nr_devices, sizeof(*devices_info),
			       GFP_NOFS);
	if (!devices_info)
		return -ENOMEM;

	/* calc min stripe number for data space alloction */
	type = btrfs_get_alloc_profile(root, 1);
	if (type & BTRFS_BLOCK_GROUP_RAID0) {
		min_stripes = 2;
		num_stripes = nr_devices;
	} else if (type & BTRFS_BLOCK_GROUP_RAID1) {
		min_stripes = 2;
		num_stripes = 2;
	} else if (type & BTRFS_BLOCK_GROUP_RAID10) {
		min_stripes = 4;
		num_stripes = 4;
	}

	if (type & BTRFS_BLOCK_GROUP_DUP)
		min_stripe_size = 2 * BTRFS_STRIPE_LEN;
	else
		min_stripe_size = BTRFS_STRIPE_LEN;

	if (fs_info->alloc_start)
		mutex_lock(&fs_devices->device_list_mutex);
	rcu_read_lock();
	list_for_each_entry_rcu(device, &fs_devices->devices, dev_list) {
		if (!device->in_fs_metadata || !device->bdev ||
		    device->is_tgtdev_for_dev_replace)
			continue;

		if (i >= nr_devices)
			break;

		avail_space = device->total_bytes - device->bytes_used;

		/* align with stripe_len */
		avail_space = div_u64(avail_space, BTRFS_STRIPE_LEN);
		avail_space *= BTRFS_STRIPE_LEN;

		/*
		 * In order to avoid overwritting the superblock on the drive,
		 * btrfs starts at an offset of at least 1MB when doing chunk
		 * allocation.
		 */
		skip_space = 1024 * 1024;

		/* user can set the offset in fs_info->alloc_start. */
		if (fs_info->alloc_start &&
		    fs_info->alloc_start + BTRFS_STRIPE_LEN <=
		    device->total_bytes) {
			rcu_read_unlock();
			skip_space = max(fs_info->alloc_start, skip_space);

			/*
			 * btrfs can not use the free space in
			 * [0, skip_space - 1], we must subtract it from the
			 * total. In order to implement it, we account the used
			 * space in this range first.
			 */
			ret = btrfs_account_dev_extents_size(device, 0,
							     skip_space - 1,
							     &used_space);
			if (ret) {
				kfree(devices_info);
				mutex_unlock(&fs_devices->device_list_mutex);
				return ret;
			}

			rcu_read_lock();

			/* calc the free space in [0, skip_space - 1] */
			skip_space -= used_space;
		}

		/*
		 * we can use the free space in [0, skip_space - 1], subtract
		 * it from the total.
		 */
		if (avail_space && avail_space >= skip_space)
			avail_space -= skip_space;
		else
			avail_space = 0;

		if (avail_space < min_stripe_size)
			continue;

		devices_info[i].dev = device;
		devices_info[i].max_avail = avail_space;

		i++;
	}
	rcu_read_unlock();
	if (fs_info->alloc_start)
		mutex_unlock(&fs_devices->device_list_mutex);

	nr_devices = i;

	btrfs_descending_sort_devices(devices_info, nr_devices);

	i = nr_devices - 1;
	avail_space = 0;
	while (nr_devices >= min_stripes) {
		if (num_stripes > nr_devices)
			num_stripes = nr_devices;

		if (devices_info[i].max_avail >= min_stripe_size) {
			int j;
			u64 alloc_size;

			avail_space += devices_info[i].max_avail * num_stripes;
			alloc_size = devices_info[i].max_avail;
			for (j = i + 1 - num_stripes; j <= i; j++)
				devices_info[j].max_avail -= alloc_size;
		}
		i--;
		nr_devices--;
	}

	kfree(devices_info);
	*free_bytes = avail_space;
	return 0;
}

/*
 * Calculate numbers for 'df', pessimistic in case of mixed raid profiles.
 *
 * If there's a redundant raid level at DATA block groups, use the respective
 * multiplier to scale the sizes.
 *
 * Unused device space usage is based on simulating the chunk allocator
 * algorithm that respects the device sizes, order of allocations and the
 * 'alloc_start' value, this is a close approximation of the actual use but
 * there are other factors that may change the result (like a new metadata
 * chunk).
 *
 * If metadata is exhausted, f_bavail will be 0.
 *
 * FIXME: not accurate for mixed block groups, total and free/used are ok,
 * available appears slightly larger.
 */
static int btrfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(dentry->d_sb);
	struct btrfs_super_block *disk_super = fs_info->super_copy;
	struct list_head *head = &fs_info->space_info;
	struct btrfs_space_info *found;
	u64 total_used = 0;
	u64 total_free_data = 0;
	u64 total_free_meta = 0;
	int bits = dentry->d_sb->s_blocksize_bits;
	__be32 *fsid = (__be32 *)fs_info->fsid;
	unsigned factor = 1;
	struct btrfs_block_rsv *block_rsv = &fs_info->global_block_rsv;
	int ret;
	u64 thresh = 0;

	/*
	 * holding chunk_muext to avoid allocating new chunks, holding
	 * device_list_mutex to avoid the device being removed
	 */
	rcu_read_lock();
	list_for_each_entry_rcu(found, head, list) {
		if (found->flags & BTRFS_BLOCK_GROUP_DATA) {
			int i;

			total_free_data += found->disk_total - found->disk_used;
			total_free_data -=
				btrfs_account_ro_block_groups_free_space(found);

			for (i = 0; i < BTRFS_NR_RAID_TYPES; i++) {
				if (!list_empty(&found->block_groups[i])) {
					switch (i) {
					case BTRFS_RAID_DUP:
					case BTRFS_RAID_RAID1:
					case BTRFS_RAID_RAID10:
						factor = 2;
					}
				}
			}
		}
		if (found->flags & BTRFS_BLOCK_GROUP_METADATA)
			total_free_meta += found->disk_total - found->disk_used;

		total_used += found->disk_used;
	}

	rcu_read_unlock();

	buf->f_blocks = div_u64(btrfs_super_total_bytes(disk_super), factor);
	buf->f_blocks >>= bits;
	buf->f_bfree = buf->f_blocks - (div_u64(total_used, factor) >> bits);

	/* Account global block reserve as used, it's in logical size already */
	spin_lock(&block_rsv->lock);
	buf->f_bfree -= block_rsv->size >> bits;
	spin_unlock(&block_rsv->lock);

	buf->f_bavail = div_u64(total_free_data, factor);
	ret = btrfs_calc_avail_data_space(fs_info->tree_root, &total_free_data);
	if (ret)
		return ret;
	buf->f_bavail += div_u64(total_free_data, factor);
	buf->f_bavail = buf->f_bavail >> bits;

	/*
	 * We calculate the remaining metadata space minus global reserve. If
	 * this is (supposedly) smaller than zero, there's no space. But this
	 * does not hold in practice, the exhausted state happens where's still
	 * some positive delta. So we apply some guesswork and compare the
	 * delta to a 4M threshold.  (Practically observed delta was ~2M.)
	 *
	 * We probably cannot calculate the exact threshold value because this
	 * depends on the internal reservations requested by various
	 * operations, so some operations that consume a few metadata will
	 * succeed even if the Avail is zero. But this is better than the other
	 * way around.
	 */
	thresh = 4 * 1024 * 1024;

	if (total_free_meta - thresh < block_rsv->size)
		buf->f_bavail = 0;

	buf->f_type = BTRFS_SUPER_MAGIC;
	buf->f_bsize = dentry->d_sb->s_blocksize;
	buf->f_namelen = BTRFS_NAME_LEN;

	/* We treat it as constant endianness (it doesn't matter _which_)
	   because we want the fsid to come out the same whether mounted
	   on a big-endian or little-endian host */
	buf->f_fsid.val[0] = be32_to_cpu(fsid[0]) ^ be32_to_cpu(fsid[2]);
	buf->f_fsid.val[1] = be32_to_cpu(fsid[1]) ^ be32_to_cpu(fsid[3]);
	/* Mask in the root object ID too, to disambiguate subvols */
	buf->f_fsid.val[0] ^= BTRFS_I(d_inode(dentry))->root->objectid >> 32;
	buf->f_fsid.val[1] ^= BTRFS_I(d_inode(dentry))->root->objectid;

	return 0;
}

static void btrfs_kill_super(struct super_block *sb)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	kill_anon_super(sb);
	free_fs_info(fs_info);
}

static struct file_system_type btrfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "btrfs",
	.mount		= btrfs_mount,
	.kill_sb	= btrfs_kill_super,
	.fs_flags	= FS_REQUIRES_DEV | FS_BINARY_MOUNTDATA,
};
MODULE_ALIAS_FS("btrfs");

static int btrfs_control_open(struct inode *inode, struct file *file)
{
	/*
	 * The control file's private_data is used to hold the
	 * transaction when it is started and is used to keep
	 * track of whether a transaction is already in progress.
	 */
	file->private_data = NULL;
	return 0;
}

/*
 * used by btrfsctl to scan devices when no FS is mounted
 */
static long btrfs_control_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	struct btrfs_ioctl_vol_args *vol;
	struct btrfs_fs_devices *fs_devices;
	int ret = -ENOTTY;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	vol = memdup_user((void __user *)arg, sizeof(*vol));
	if (IS_ERR(vol))
		return PTR_ERR(vol);

	switch (cmd) {
	case BTRFS_IOC_SCAN_DEV:
		ret = btrfs_scan_one_device(vol->name, FMODE_READ,
					    &btrfs_fs_type, &fs_devices);
		break;
	case BTRFS_IOC_DEVICES_READY:
		ret = btrfs_scan_one_device(vol->name, FMODE_READ,
					    &btrfs_fs_type, &fs_devices);
		if (ret)
			break;
		ret = !(fs_devices->num_devices == fs_devices->total_devices);
		break;
	}

	kfree(vol);
	return ret;
}

static int btrfs_freeze(struct super_block *sb)
{
	struct btrfs_trans_handle *trans;
	struct btrfs_root *root = btrfs_sb(sb)->tree_root;

	trans = btrfs_attach_transaction_barrier(root);
	if (IS_ERR(trans)) {
		/* no transaction, don't bother */
		if (PTR_ERR(trans) == -ENOENT)
			return 0;
		return PTR_ERR(trans);
	}
	return btrfs_commit_transaction(trans, root);
}

static int btrfs_show_devname(struct seq_file *m, struct dentry *root)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(root->d_sb);
	struct btrfs_fs_devices *cur_devices;
	struct btrfs_device *dev, *first_dev = NULL;
	struct list_head *head;
	struct rcu_string *name;

	mutex_lock(&fs_info->fs_devices->device_list_mutex);
	cur_devices = fs_info->fs_devices;
	while (cur_devices) {
		head = &cur_devices->devices;
		list_for_each_entry(dev, head, dev_list) {
			if (dev->missing)
				continue;
			if (!dev->name)
				continue;
			if (!first_dev || dev->devid < first_dev->devid)
				first_dev = dev;
		}
		cur_devices = cur_devices->seed;
	}

	if (first_dev) {
		rcu_read_lock();
		name = rcu_dereference(first_dev->name);
		seq_escape(m, name->str, " \t\n\\");
		rcu_read_unlock();
	} else {
		WARN_ON(1);
	}
	mutex_unlock(&fs_info->fs_devices->device_list_mutex);
	return 0;
}

static const struct super_operations btrfs_super_ops = {
	.drop_inode	= btrfs_drop_inode,
	.evict_inode	= btrfs_evict_inode,
	.put_super	= btrfs_put_super,
	.sync_fs	= btrfs_sync_fs,
	.show_options	= btrfs_show_options,
	.show_devname	= btrfs_show_devname,
	.write_inode	= btrfs_write_inode,
	.alloc_inode	= btrfs_alloc_inode,
	.destroy_inode	= btrfs_destroy_inode,
	.statfs		= btrfs_statfs,
	.remount_fs	= btrfs_remount,
	.freeze_fs	= btrfs_freeze,
};

static const struct file_operations btrfs_ctl_fops = {
	.open = btrfs_control_open,
	.unlocked_ioctl	 = btrfs_control_ioctl,
	.compat_ioctl = btrfs_control_ioctl,
	.owner	 = THIS_MODULE,
	.llseek = noop_llseek,
};

static struct miscdevice btrfs_misc = {
	.minor		= BTRFS_MINOR,
	.name		= "btrfs-control",
	.fops		= &btrfs_ctl_fops
};

MODULE_ALIAS_MISCDEV(BTRFS_MINOR);
MODULE_ALIAS("devname:btrfs-control");

static int btrfs_interface_init(void)
{
	return misc_register(&btrfs_misc);
}

static void btrfs_interface_exit(void)
{
	if (misc_deregister(&btrfs_misc) < 0)
		printk(KERN_INFO "BTRFS: misc_deregister failed for control device\n");
}

static void btrfs_print_info(void)
{
	printk(KERN_INFO "Btrfs loaded"
#ifdef CONFIG_BTRFS_DEBUG
			", debug=on"
#endif
#ifdef CONFIG_BTRFS_ASSERT
			", assert=on"
#endif
#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
			", integrity-checker=on"
#endif
			"\n");
}

static int btrfs_run_sanity_tests(void)
{
	int ret;

	ret = btrfs_init_test_fs();
	if (ret)
		return ret;

	ret = btrfs_test_free_space_cache();
	if (ret)
		goto out;
	ret = btrfs_test_extent_buffer_operations();
	if (ret)
		goto out;
	ret = btrfs_test_extent_io();
	if (ret)
		goto out;
	ret = btrfs_test_inodes();
	if (ret)
		goto out;
	ret = btrfs_test_qgroups();
out:
	btrfs_destroy_test_fs();
	return ret;
}

static int __init init_btrfs_fs(void)
{
	int err;

	err = btrfs_hash_init();
	if (err)
		return err;

	btrfs_props_init();

	err = btrfs_init_sysfs();
	if (err)
		goto free_hash;

	btrfs_init_compress();

	err = btrfs_init_cachep();
	if (err)
		goto free_compress;

	err = extent_io_init();
	if (err)
		goto free_cachep;

	err = extent_map_init();
	if (err)
		goto free_extent_io;

	err = ordered_data_init();
	if (err)
		goto free_extent_map;

	err = btrfs_delayed_inode_init();
	if (err)
		goto free_ordered_data;

	err = btrfs_auto_defrag_init();
	if (err)
		goto free_delayed_inode;

	err = btrfs_delayed_ref_init();
	if (err)
		goto free_auto_defrag;

	err = btrfs_prelim_ref_init();
	if (err)
		goto free_delayed_ref;

	err = btrfs_end_io_wq_init();
	if (err)
		goto free_prelim_ref;

	err = btrfs_interface_init();
	if (err)
		goto free_end_io_wq;

	btrfs_init_lockdep();

	btrfs_print_info();

	err = btrfs_run_sanity_tests();
	if (err)
		goto unregister_ioctl;

	err = register_filesystem(&btrfs_fs_type);
	if (err)
		goto unregister_ioctl;

	return 0;

unregister_ioctl:
	btrfs_interface_exit();
free_end_io_wq:
	btrfs_end_io_wq_exit();
free_prelim_ref:
	btrfs_prelim_ref_exit();
free_delayed_ref:
	btrfs_delayed_ref_exit();
free_auto_defrag:
	btrfs_auto_defrag_exit();
free_delayed_inode:
	btrfs_delayed_inode_exit();
free_ordered_data:
	ordered_data_exit();
free_extent_map:
	extent_map_exit();
free_extent_io:
	extent_io_exit();
free_cachep:
	btrfs_destroy_cachep();
free_compress:
	btrfs_exit_compress();
	btrfs_exit_sysfs();
free_hash:
	btrfs_hash_exit();
	return err;
}

static void __exit exit_btrfs_fs(void)
{
	btrfs_destroy_cachep();
	btrfs_delayed_ref_exit();
	btrfs_auto_defrag_exit();
	btrfs_delayed_inode_exit();
	btrfs_prelim_ref_exit();
	ordered_data_exit();
	extent_map_exit();
	extent_io_exit();
	btrfs_interface_exit();
	btrfs_end_io_wq_exit();
	unregister_filesystem(&btrfs_fs_type);
	btrfs_exit_sysfs();
	btrfs_cleanup_fs_uuids();
	btrfs_exit_compress();
	btrfs_hash_exit();
}

late_initcall(init_btrfs_fs);
module_exit(exit_btrfs_fs)

MODULE_LICENSE("GPL");
