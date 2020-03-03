/*
 * Copyright (C) Sistina Software, Inc.  1997-2003 All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc.  All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License version 2.
 */

#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/buffer_head.h>
#include <linux/gfs2_ondisk.h>
#include <linux/namei.h>
#include <linux/crc32.h>

#include "gfs2.h"
#include "incore.h"
#include "dir.h"
#include "glock.h"
#include "super.h"
#include "util.h"
#include "inode.h"

/**
 * gfs2_drevalidate - Check directory lookup consistency
 * @dentry: the mapping to check
 * @flags: lookup flags
 *
 * Check to make sure the lookup necessary to arrive at this inode from its
 * parent is still good.
 *
 * Returns: 1 if the dentry is ok, 0 if it isn't
 */

static int gfs2_drevalidate(struct dentry *dentry, unsigned int flags)
{
	struct dentry *parent;
	struct gfs2_sbd *sdp;
	struct gfs2_inode *dip;
	struct inode *inode;
	struct gfs2_holder d_gh;
	struct gfs2_inode *ip = NULL;
	int error;
	int had_lock = 0;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	parent = dget_parent(dentry);
	sdp = GFS2_SB(d_inode(parent));
	dip = GFS2_I(d_inode(parent));
	inode = d_inode(dentry);

	if (inode) {
		if (is_bad_inode(inode))
			goto invalid;
		ip = GFS2_I(inode);
	}

	if (sdp->sd_lockstruct.ls_ops->lm_mount == NULL)
		goto valid;

	had_lock = (gfs2_glock_is_locked_by_me(dip->i_gl) != NULL);
	if (!had_lock) {
		error = gfs2_glock_nq_init(dip->i_gl, LM_ST_SHARED, 0, &d_gh);
		if (error)
			goto fail;
	} 

	error = gfs2_dir_check(d_inode(parent), &dentry->d_name, ip);
	switch (error) {
	case 0:
		if (!inode)
			goto invalid_gunlock;
		break;
	case -ENOENT:
		if (!inode)
			goto valid_gunlock;
		goto invalid_gunlock;
	default:
		goto fail_gunlock;
	}

valid_gunlock:
	if (!had_lock)
		gfs2_glock_dq_uninit(&d_gh);
valid:
	dput(parent);
	return 1;

invalid_gunlock:
	if (!had_lock)
		gfs2_glock_dq_uninit(&d_gh);
invalid:
	dput(parent);
	return 0;

fail_gunlock:
	gfs2_glock_dq_uninit(&d_gh);
fail:
	dput(parent);
	return 0;
}

static int gfs2_dhash(const struct dentry *dentry, struct qstr *str)
{
	str->hash = gfs2_disk_hash(str->name, str->len);
	return 0;
}

static int gfs2_dentry_delete(const struct dentry *dentry)
{
	struct gfs2_inode *ginode;

	if (d_really_is_negative(dentry))
		return 0;

	ginode = GFS2_I(d_inode(dentry));
	if (!ginode->i_iopen_gh.gh_gl)
		return 0;

	if (test_bit(GLF_DEMOTE, &ginode->i_iopen_gh.gh_gl->gl_flags))
		return 1;

	return 0;
}

const struct dentry_operations gfs2_dops = {
	.d_revalidate = gfs2_drevalidate,
	.d_hash = gfs2_dhash,
	.d_delete = gfs2_dentry_delete,
};

