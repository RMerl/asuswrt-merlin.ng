/*
 * Copyright (C) Sistina Software, Inc.  1997-2003 All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc.  All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License version 2.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/buffer_head.h>
#include <linux/crc32.h>
#include <linux/gfs2_ondisk.h>
#include <asm/uaccess.h>

#include "gfs2.h"
#include "incore.h"
#include "glock.h"
#include "util.h"

struct kmem_cache *gfs2_glock_cachep __read_mostly;
struct kmem_cache *gfs2_glock_aspace_cachep __read_mostly;
struct kmem_cache *gfs2_inode_cachep __read_mostly;
struct kmem_cache *gfs2_bufdata_cachep __read_mostly;
struct kmem_cache *gfs2_rgrpd_cachep __read_mostly;
struct kmem_cache *gfs2_quotad_cachep __read_mostly;
struct kmem_cache *gfs2_rsrv_cachep __read_mostly;
mempool_t *gfs2_page_pool __read_mostly;

void gfs2_assert_i(struct gfs2_sbd *sdp)
{
	fs_emerg(sdp, "fatal assertion failed\n");
}

int gfs2_lm_withdraw(struct gfs2_sbd *sdp, const char *fmt, ...)
{
	struct lm_lockstruct *ls = &sdp->sd_lockstruct;
	const struct lm_lockops *lm = ls->ls_ops;
	va_list args;
	struct va_format vaf;

	if (sdp->sd_args.ar_errors == GFS2_ERRORS_WITHDRAW &&
	    test_and_set_bit(SDF_SHUTDOWN, &sdp->sd_flags))
		return 0;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	fs_err(sdp, "%pV", &vaf);

	va_end(args);

	if (sdp->sd_args.ar_errors == GFS2_ERRORS_WITHDRAW) {
		fs_err(sdp, "about to withdraw this file system\n");
		BUG_ON(sdp->sd_args.ar_debug);

		kobject_uevent(&sdp->sd_kobj, KOBJ_OFFLINE);

		if (!strcmp(sdp->sd_lockstruct.ls_ops->lm_proto_name, "lock_dlm"))
			wait_for_completion(&sdp->sd_wdack);

		if (lm->lm_unmount) {
			fs_err(sdp, "telling LM to unmount\n");
			lm->lm_unmount(sdp);
		}
		fs_err(sdp, "withdrawn\n");
		dump_stack();
	}

	if (sdp->sd_args.ar_errors == GFS2_ERRORS_PANIC)
		panic("GFS2: fsid=%s: panic requested\n", sdp->sd_fsname);

	return -1;
}

/**
 * gfs2_assert_withdraw_i - Cause the machine to withdraw if @assertion is false
 * Returns: -1 if this call withdrew the machine,
 *          -2 if it was already withdrawn
 */

int gfs2_assert_withdraw_i(struct gfs2_sbd *sdp, char *assertion,
			   const char *function, char *file, unsigned int line)
{
	int me;
	me = gfs2_lm_withdraw(sdp,
			      "fatal: assertion \"%s\" failed\n"
			      "   function = %s, file = %s, line = %u\n",
			      assertion, function, file, line);
	dump_stack();
	return (me) ? -1 : -2;
}

/**
 * gfs2_assert_warn_i - Print a message to the console if @assertion is false
 * Returns: -1 if we printed something
 *          -2 if we didn't
 */

int gfs2_assert_warn_i(struct gfs2_sbd *sdp, char *assertion,
		       const char *function, char *file, unsigned int line)
{
	if (time_before(jiffies,
			sdp->sd_last_warning +
			gfs2_tune_get(sdp, gt_complain_secs) * HZ))
		return -2;

	if (sdp->sd_args.ar_errors == GFS2_ERRORS_WITHDRAW)
		fs_warn(sdp, "warning: assertion \"%s\" failed at function = %s, file = %s, line = %u\n",
			assertion, function, file, line);

	if (sdp->sd_args.ar_debug)
		BUG();
	else
		dump_stack();

	if (sdp->sd_args.ar_errors == GFS2_ERRORS_PANIC)
		panic("GFS2: fsid=%s: warning: assertion \"%s\" failed\n"
		      "GFS2: fsid=%s:   function = %s, file = %s, line = %u\n",
		      sdp->sd_fsname, assertion,
		      sdp->sd_fsname, function, file, line);

	sdp->sd_last_warning = jiffies;

	return -1;
}

/**
 * gfs2_consist_i - Flag a filesystem consistency error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          0 if it was already withdrawn
 */

int gfs2_consist_i(struct gfs2_sbd *sdp, int cluster_wide, const char *function,
		   char *file, unsigned int line)
{
	int rv;
	rv = gfs2_lm_withdraw(sdp,
			      "fatal: filesystem consistency error - function = %s, file = %s, line = %u\n",
			      function, file, line);
	return rv;
}

/**
 * gfs2_consist_inode_i - Flag an inode consistency error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          0 if it was already withdrawn
 */

int gfs2_consist_inode_i(struct gfs2_inode *ip, int cluster_wide,
			 const char *function, char *file, unsigned int line)
{
	struct gfs2_sbd *sdp = GFS2_SB(&ip->i_inode);
	int rv;
	rv = gfs2_lm_withdraw(sdp,
			      "fatal: filesystem consistency error\n"
			      "  inode = %llu %llu\n"
			      "  function = %s, file = %s, line = %u\n",
			      (unsigned long long)ip->i_no_formal_ino,
			      (unsigned long long)ip->i_no_addr,
			      function, file, line);
	return rv;
}

/**
 * gfs2_consist_rgrpd_i - Flag a RG consistency error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          0 if it was already withdrawn
 */

int gfs2_consist_rgrpd_i(struct gfs2_rgrpd *rgd, int cluster_wide,
			 const char *function, char *file, unsigned int line)
{
	struct gfs2_sbd *sdp = rgd->rd_sbd;
	int rv;
	rv = gfs2_lm_withdraw(sdp,
			      "fatal: filesystem consistency error\n"
			      "  RG = %llu\n"
			      "  function = %s, file = %s, line = %u\n",
			      (unsigned long long)rgd->rd_addr,
			      function, file, line);
	return rv;
}

/**
 * gfs2_meta_check_ii - Flag a magic number consistency error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          -2 if it was already withdrawn
 */

int gfs2_meta_check_ii(struct gfs2_sbd *sdp, struct buffer_head *bh,
		       const char *type, const char *function, char *file,
		       unsigned int line)
{
	int me;
	me = gfs2_lm_withdraw(sdp,
			      "fatal: invalid metadata block\n"
			      "  bh = %llu (%s)\n"
			      "  function = %s, file = %s, line = %u\n",
			      (unsigned long long)bh->b_blocknr, type,
			      function, file, line);
	return (me) ? -1 : -2;
}

/**
 * gfs2_metatype_check_ii - Flag a metadata type consistency error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          -2 if it was already withdrawn
 */

int gfs2_metatype_check_ii(struct gfs2_sbd *sdp, struct buffer_head *bh,
			   u16 type, u16 t, const char *function,
			   char *file, unsigned int line)
{
	int me;
	me = gfs2_lm_withdraw(sdp,
			      "fatal: invalid metadata block\n"
			      "  bh = %llu (type: exp=%u, found=%u)\n"
			      "  function = %s, file = %s, line = %u\n",
			      (unsigned long long)bh->b_blocknr, type, t,
			      function, file, line);
	return (me) ? -1 : -2;
}

/**
 * gfs2_io_error_i - Flag an I/O error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          0 if it was already withdrawn
 */

int gfs2_io_error_i(struct gfs2_sbd *sdp, const char *function, char *file,
		    unsigned int line)
{
	int rv;
	rv = gfs2_lm_withdraw(sdp,
			      "fatal: I/O error\n"
			      "  function = %s, file = %s, line = %u\n",
			      function, file, line);
	return rv;
}

/**
 * gfs2_io_error_bh_i - Flag a buffer I/O error and withdraw
 * Returns: -1 if this call withdrew the machine,
 *          0 if it was already withdrawn
 */

int gfs2_io_error_bh_i(struct gfs2_sbd *sdp, struct buffer_head *bh,
		       const char *function, char *file, unsigned int line)
{
	int rv;
	rv = gfs2_lm_withdraw(sdp,
			      "fatal: I/O error\n"
			      "  block = %llu\n"
			      "  function = %s, file = %s, line = %u\n",
			      (unsigned long long)bh->b_blocknr,
			      function, file, line);
	return rv;
}

