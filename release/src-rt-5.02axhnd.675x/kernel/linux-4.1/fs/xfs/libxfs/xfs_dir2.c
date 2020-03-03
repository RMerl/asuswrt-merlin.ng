/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_da_format.h"
#include "xfs_da_btree.h"
#include "xfs_inode.h"
#include "xfs_trans.h"
#include "xfs_inode_item.h"
#include "xfs_bmap.h"
#include "xfs_dir2.h"
#include "xfs_dir2_priv.h"
#include "xfs_error.h"
#include "xfs_trace.h"

struct xfs_name xfs_name_dotdot = { (unsigned char *)"..", 2, XFS_DIR3_FT_DIR };

/*
 * @mode, if set, indicates that the type field needs to be set up.
 * This uses the transformation from file mode to DT_* as defined in linux/fs.h
 * for file type specification. This will be propagated into the directory
 * structure if appropriate for the given operation and filesystem config.
 */
const unsigned char xfs_mode_to_ftype[S_IFMT >> S_SHIFT] = {
	[0]			= XFS_DIR3_FT_UNKNOWN,
	[S_IFREG >> S_SHIFT]    = XFS_DIR3_FT_REG_FILE,
	[S_IFDIR >> S_SHIFT]    = XFS_DIR3_FT_DIR,
	[S_IFCHR >> S_SHIFT]    = XFS_DIR3_FT_CHRDEV,
	[S_IFBLK >> S_SHIFT]    = XFS_DIR3_FT_BLKDEV,
	[S_IFIFO >> S_SHIFT]    = XFS_DIR3_FT_FIFO,
	[S_IFSOCK >> S_SHIFT]   = XFS_DIR3_FT_SOCK,
	[S_IFLNK >> S_SHIFT]    = XFS_DIR3_FT_SYMLINK,
};

/*
 * ASCII case-insensitive (ie. A-Z) support for directories that was
 * used in IRIX.
 */
STATIC xfs_dahash_t
xfs_ascii_ci_hashname(
	struct xfs_name	*name)
{
	xfs_dahash_t	hash;
	int		i;

	for (i = 0, hash = 0; i < name->len; i++)
		hash = tolower(name->name[i]) ^ rol32(hash, 7);

	return hash;
}

STATIC enum xfs_dacmp
xfs_ascii_ci_compname(
	struct xfs_da_args *args,
	const unsigned char *name,
	int		len)
{
	enum xfs_dacmp	result;
	int		i;

	if (args->namelen != len)
		return XFS_CMP_DIFFERENT;

	result = XFS_CMP_EXACT;
	for (i = 0; i < len; i++) {
		if (args->name[i] == name[i])
			continue;
		if (tolower(args->name[i]) != tolower(name[i]))
			return XFS_CMP_DIFFERENT;
		result = XFS_CMP_CASE;
	}

	return result;
}

static struct xfs_nameops xfs_ascii_ci_nameops = {
	.hashname	= xfs_ascii_ci_hashname,
	.compname	= xfs_ascii_ci_compname,
};

int
xfs_da_mount(
	struct xfs_mount	*mp)
{
	struct xfs_da_geometry	*dageo;
	int			nodehdr_size;


	ASSERT(mp->m_sb.sb_versionnum & XFS_SB_VERSION_DIRV2BIT);
	ASSERT((1 << (mp->m_sb.sb_blocklog + mp->m_sb.sb_dirblklog)) <=
	       XFS_MAX_BLOCKSIZE);

	mp->m_dir_inode_ops = xfs_dir_get_ops(mp, NULL);
	mp->m_nondir_inode_ops = xfs_nondir_get_ops(mp, NULL);

	nodehdr_size = mp->m_dir_inode_ops->node_hdr_size;
	mp->m_dir_geo = kmem_zalloc(sizeof(struct xfs_da_geometry),
				    KM_SLEEP | KM_MAYFAIL);
	mp->m_attr_geo = kmem_zalloc(sizeof(struct xfs_da_geometry),
				     KM_SLEEP | KM_MAYFAIL);
	if (!mp->m_dir_geo || !mp->m_attr_geo) {
		kmem_free(mp->m_dir_geo);
		kmem_free(mp->m_attr_geo);
		return -ENOMEM;
	}

	/* set up directory geometry */
	dageo = mp->m_dir_geo;
	dageo->blklog = mp->m_sb.sb_blocklog + mp->m_sb.sb_dirblklog;
	dageo->fsblog = mp->m_sb.sb_blocklog;
	dageo->blksize = 1 << dageo->blklog;
	dageo->fsbcount = 1 << mp->m_sb.sb_dirblklog;

	/*
	 * Now we've set up the block conversion variables, we can calculate the
	 * segment block constants using the geometry structure.
	 */
	dageo->datablk = xfs_dir2_byte_to_da(dageo, XFS_DIR2_DATA_OFFSET);
	dageo->leafblk = xfs_dir2_byte_to_da(dageo, XFS_DIR2_LEAF_OFFSET);
	dageo->freeblk = xfs_dir2_byte_to_da(dageo, XFS_DIR2_FREE_OFFSET);
	dageo->node_ents = (dageo->blksize - nodehdr_size) /
				(uint)sizeof(xfs_da_node_entry_t);
	dageo->magicpct = (dageo->blksize * 37) / 100;

	/* set up attribute geometry - single fsb only */
	dageo = mp->m_attr_geo;
	dageo->blklog = mp->m_sb.sb_blocklog;
	dageo->fsblog = mp->m_sb.sb_blocklog;
	dageo->blksize = 1 << dageo->blklog;
	dageo->fsbcount = 1;
	dageo->node_ents = (dageo->blksize - nodehdr_size) /
				(uint)sizeof(xfs_da_node_entry_t);
	dageo->magicpct = (dageo->blksize * 37) / 100;

	if (xfs_sb_version_hasasciici(&mp->m_sb))
		mp->m_dirnameops = &xfs_ascii_ci_nameops;
	else
		mp->m_dirnameops = &xfs_default_nameops;

	return 0;
}

void
xfs_da_unmount(
	struct xfs_mount	*mp)
{
	kmem_free(mp->m_dir_geo);
	kmem_free(mp->m_attr_geo);
}

/*
 * Return 1 if directory contains only "." and "..".
 */
int
xfs_dir_isempty(
	xfs_inode_t	*dp)
{
	xfs_dir2_sf_hdr_t	*sfp;

	ASSERT(S_ISDIR(dp->i_d.di_mode));
	if (dp->i_d.di_size == 0)	/* might happen during shutdown. */
		return 1;
	if (dp->i_d.di_size > XFS_IFORK_DSIZE(dp))
		return 0;
	sfp = (xfs_dir2_sf_hdr_t *)dp->i_df.if_u1.if_data;
	return !sfp->count;
}

/*
 * Validate a given inode number.
 */
int
xfs_dir_ino_validate(
	xfs_mount_t	*mp,
	xfs_ino_t	ino)
{
	xfs_agblock_t	agblkno;
	xfs_agino_t	agino;
	xfs_agnumber_t	agno;
	int		ino_ok;
	int		ioff;

	agno = XFS_INO_TO_AGNO(mp, ino);
	agblkno = XFS_INO_TO_AGBNO(mp, ino);
	ioff = XFS_INO_TO_OFFSET(mp, ino);
	agino = XFS_OFFBNO_TO_AGINO(mp, agblkno, ioff);
	ino_ok =
		agno < mp->m_sb.sb_agcount &&
		agblkno < mp->m_sb.sb_agblocks &&
		agblkno != 0 &&
		ioff < (1 << mp->m_sb.sb_inopblog) &&
		XFS_AGINO_TO_INO(mp, agno, agino) == ino;
	if (unlikely(XFS_TEST_ERROR(!ino_ok, mp, XFS_ERRTAG_DIR_INO_VALIDATE,
			XFS_RANDOM_DIR_INO_VALIDATE))) {
		xfs_warn(mp, "Invalid inode number 0x%Lx",
				(unsigned long long) ino);
		XFS_ERROR_REPORT("xfs_dir_ino_validate", XFS_ERRLEVEL_LOW, mp);
		return -EFSCORRUPTED;
	}
	return 0;
}

/*
 * Initialize a directory with its "." and ".." entries.
 */
int
xfs_dir_init(
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	xfs_inode_t	*pdp)
{
	struct xfs_da_args *args;
	int		error;

	ASSERT(S_ISDIR(dp->i_d.di_mode));
	error = xfs_dir_ino_validate(tp->t_mountp, pdp->i_ino);
	if (error)
		return error;

	args = kmem_zalloc(sizeof(*args), KM_SLEEP | KM_NOFS);
	if (!args)
		return -ENOMEM;

	args->geo = dp->i_mount->m_dir_geo;
	args->dp = dp;
	args->trans = tp;
	error = xfs_dir2_sf_create(args, pdp->i_ino);
	kmem_free(args);
	return error;
}

/*
 * Enter a name in a directory, or check for available space.
 * If inum is 0, only the available space test is performed.
 */
int
xfs_dir_createname(
	xfs_trans_t		*tp,
	xfs_inode_t		*dp,
	struct xfs_name		*name,
	xfs_ino_t		inum,		/* new entry inode number */
	xfs_fsblock_t		*first,		/* bmap's firstblock */
	xfs_bmap_free_t		*flist,		/* bmap's freeblock list */
	xfs_extlen_t		total)		/* bmap's total block count */
{
	struct xfs_da_args	*args;
	int			rval;
	int			v;		/* type-checking value */

	ASSERT(S_ISDIR(dp->i_d.di_mode));
	if (inum) {
		rval = xfs_dir_ino_validate(tp->t_mountp, inum);
		if (rval)
			return rval;
		XFS_STATS_INC(xs_dir_create);
	}

	args = kmem_zalloc(sizeof(*args), KM_SLEEP | KM_NOFS);
	if (!args)
		return -ENOMEM;

	args->geo = dp->i_mount->m_dir_geo;
	args->name = name->name;
	args->namelen = name->len;
	args->filetype = name->type;
	args->hashval = dp->i_mount->m_dirnameops->hashname(name);
	args->inumber = inum;
	args->dp = dp;
	args->firstblock = first;
	args->flist = flist;
	args->total = total;
	args->whichfork = XFS_DATA_FORK;
	args->trans = tp;
	args->op_flags = XFS_DA_OP_ADDNAME | XFS_DA_OP_OKNOENT;
	if (!inum)
		args->op_flags |= XFS_DA_OP_JUSTCHECK;

	if (dp->i_d.di_format == XFS_DINODE_FMT_LOCAL) {
		rval = xfs_dir2_sf_addname(args);
		goto out_free;
	}

	rval = xfs_dir2_isblock(args, &v);
	if (rval)
		goto out_free;
	if (v) {
		rval = xfs_dir2_block_addname(args);
		goto out_free;
	}

	rval = xfs_dir2_isleaf(args, &v);
	if (rval)
		goto out_free;
	if (v)
		rval = xfs_dir2_leaf_addname(args);
	else
		rval = xfs_dir2_node_addname(args);

out_free:
	kmem_free(args);
	return rval;
}

/*
 * If doing a CI lookup and case-insensitive match, dup actual name into
 * args.value. Return EEXIST for success (ie. name found) or an error.
 */
int
xfs_dir_cilookup_result(
	struct xfs_da_args *args,
	const unsigned char *name,
	int		len)
{
	if (args->cmpresult == XFS_CMP_DIFFERENT)
		return -ENOENT;
	if (args->cmpresult != XFS_CMP_CASE ||
					!(args->op_flags & XFS_DA_OP_CILOOKUP))
		return -EEXIST;

	args->value = kmem_alloc(len, KM_NOFS | KM_MAYFAIL);
	if (!args->value)
		return -ENOMEM;

	memcpy(args->value, name, len);
	args->valuelen = len;
	return -EEXIST;
}

/*
 * Lookup a name in a directory, give back the inode number.
 * If ci_name is not NULL, returns the actual name in ci_name if it differs
 * to name, or ci_name->name is set to NULL for an exact match.
 */

int
xfs_dir_lookup(
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	struct xfs_name	*name,
	xfs_ino_t	*inum,		/* out: inode number */
	struct xfs_name *ci_name)	/* out: actual name if CI match */
{
	struct xfs_da_args *args;
	int		rval;
	int		v;		/* type-checking value */

	ASSERT(S_ISDIR(dp->i_d.di_mode));
	XFS_STATS_INC(xs_dir_lookup);

	/*
	 * We need to use KM_NOFS here so that lockdep will not throw false
	 * positive deadlock warnings on a non-transactional lookup path. It is
	 * safe to recurse into inode recalim in that case, but lockdep can't
	 * easily be taught about it. Hence KM_NOFS avoids having to add more
	 * lockdep Doing this avoids having to add a bunch of lockdep class
	 * annotations into the reclaim path for the ilock.
	 */
	args = kmem_zalloc(sizeof(*args), KM_SLEEP | KM_NOFS);
	args->geo = dp->i_mount->m_dir_geo;
	args->name = name->name;
	args->namelen = name->len;
	args->filetype = name->type;
	args->hashval = dp->i_mount->m_dirnameops->hashname(name);
	args->dp = dp;
	args->whichfork = XFS_DATA_FORK;
	args->trans = tp;
	args->op_flags = XFS_DA_OP_OKNOENT;
	if (ci_name)
		args->op_flags |= XFS_DA_OP_CILOOKUP;

	if (dp->i_d.di_format == XFS_DINODE_FMT_LOCAL) {
		rval = xfs_dir2_sf_lookup(args);
		goto out_check_rval;
	}

	rval = xfs_dir2_isblock(args, &v);
	if (rval)
		goto out_free;
	if (v) {
		rval = xfs_dir2_block_lookup(args);
		goto out_check_rval;
	}

	rval = xfs_dir2_isleaf(args, &v);
	if (rval)
		goto out_free;
	if (v)
		rval = xfs_dir2_leaf_lookup(args);
	else
		rval = xfs_dir2_node_lookup(args);

out_check_rval:
	if (rval == -EEXIST)
		rval = 0;
	if (!rval) {
		*inum = args->inumber;
		if (ci_name) {
			ci_name->name = args->value;
			ci_name->len = args->valuelen;
		}
	}
out_free:
	kmem_free(args);
	return rval;
}

/*
 * Remove an entry from a directory.
 */
int
xfs_dir_removename(
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	struct xfs_name	*name,
	xfs_ino_t	ino,
	xfs_fsblock_t	*first,		/* bmap's firstblock */
	xfs_bmap_free_t	*flist,		/* bmap's freeblock list */
	xfs_extlen_t	total)		/* bmap's total block count */
{
	struct xfs_da_args *args;
	int		rval;
	int		v;		/* type-checking value */

	ASSERT(S_ISDIR(dp->i_d.di_mode));
	XFS_STATS_INC(xs_dir_remove);

	args = kmem_zalloc(sizeof(*args), KM_SLEEP | KM_NOFS);
	if (!args)
		return -ENOMEM;

	args->geo = dp->i_mount->m_dir_geo;
	args->name = name->name;
	args->namelen = name->len;
	args->filetype = name->type;
	args->hashval = dp->i_mount->m_dirnameops->hashname(name);
	args->inumber = ino;
	args->dp = dp;
	args->firstblock = first;
	args->flist = flist;
	args->total = total;
	args->whichfork = XFS_DATA_FORK;
	args->trans = tp;

	if (dp->i_d.di_format == XFS_DINODE_FMT_LOCAL) {
		rval = xfs_dir2_sf_removename(args);
		goto out_free;
	}

	rval = xfs_dir2_isblock(args, &v);
	if (rval)
		goto out_free;
	if (v) {
		rval = xfs_dir2_block_removename(args);
		goto out_free;
	}

	rval = xfs_dir2_isleaf(args, &v);
	if (rval)
		goto out_free;
	if (v)
		rval = xfs_dir2_leaf_removename(args);
	else
		rval = xfs_dir2_node_removename(args);
out_free:
	kmem_free(args);
	return rval;
}

/*
 * Replace the inode number of a directory entry.
 */
int
xfs_dir_replace(
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	struct xfs_name	*name,		/* name of entry to replace */
	xfs_ino_t	inum,		/* new inode number */
	xfs_fsblock_t	*first,		/* bmap's firstblock */
	xfs_bmap_free_t	*flist,		/* bmap's freeblock list */
	xfs_extlen_t	total)		/* bmap's total block count */
{
	struct xfs_da_args *args;
	int		rval;
	int		v;		/* type-checking value */

	ASSERT(S_ISDIR(dp->i_d.di_mode));

	rval = xfs_dir_ino_validate(tp->t_mountp, inum);
	if (rval)
		return rval;

	args = kmem_zalloc(sizeof(*args), KM_SLEEP | KM_NOFS);
	if (!args)
		return -ENOMEM;

	args->geo = dp->i_mount->m_dir_geo;
	args->name = name->name;
	args->namelen = name->len;
	args->filetype = name->type;
	args->hashval = dp->i_mount->m_dirnameops->hashname(name);
	args->inumber = inum;
	args->dp = dp;
	args->firstblock = first;
	args->flist = flist;
	args->total = total;
	args->whichfork = XFS_DATA_FORK;
	args->trans = tp;

	if (dp->i_d.di_format == XFS_DINODE_FMT_LOCAL) {
		rval = xfs_dir2_sf_replace(args);
		goto out_free;
	}

	rval = xfs_dir2_isblock(args, &v);
	if (rval)
		goto out_free;
	if (v) {
		rval = xfs_dir2_block_replace(args);
		goto out_free;
	}

	rval = xfs_dir2_isleaf(args, &v);
	if (rval)
		goto out_free;
	if (v)
		rval = xfs_dir2_leaf_replace(args);
	else
		rval = xfs_dir2_node_replace(args);
out_free:
	kmem_free(args);
	return rval;
}

/*
 * See if this entry can be added to the directory without allocating space.
 */
int
xfs_dir_canenter(
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	struct xfs_name	*name)		/* name of entry to add */
{
	return xfs_dir_createname(tp, dp, name, 0, NULL, NULL, 0);
}

/*
 * Utility routines.
 */

/*
 * Add a block to the directory.
 *
 * This routine is for data and free blocks, not leaf/node blocks which are
 * handled by xfs_da_grow_inode.
 */
int
xfs_dir2_grow_inode(
	struct xfs_da_args	*args,
	int			space,	/* v2 dir's space XFS_DIR2_xxx_SPACE */
	xfs_dir2_db_t		*dbp)	/* out: block number added */
{
	struct xfs_inode	*dp = args->dp;
	struct xfs_mount	*mp = dp->i_mount;
	xfs_fileoff_t		bno;	/* directory offset of new block */
	int			count;	/* count of filesystem blocks */
	int			error;

	trace_xfs_dir2_grow_inode(args, space);

	/*
	 * Set lowest possible block in the space requested.
	 */
	bno = XFS_B_TO_FSBT(mp, space * XFS_DIR2_SPACE_SIZE);
	count = args->geo->fsbcount;

	error = xfs_da_grow_inode_int(args, &bno, count);
	if (error)
		return error;

	*dbp = xfs_dir2_da_to_db(args->geo, (xfs_dablk_t)bno);

	/*
	 * Update file's size if this is the data space and it grew.
	 */
	if (space == XFS_DIR2_DATA_SPACE) {
		xfs_fsize_t	size;		/* directory file (data) size */

		size = XFS_FSB_TO_B(mp, bno + count);
		if (size > dp->i_d.di_size) {
			dp->i_d.di_size = size;
			xfs_trans_log_inode(args->trans, dp, XFS_ILOG_CORE);
		}
	}
	return 0;
}

/*
 * See if the directory is a single-block form directory.
 */
int
xfs_dir2_isblock(
	struct xfs_da_args	*args,
	int			*vp)	/* out: 1 is block, 0 is not block */
{
	xfs_fileoff_t		last;	/* last file offset */
	int			rval;

	if ((rval = xfs_bmap_last_offset(args->dp, &last, XFS_DATA_FORK)))
		return rval;
	rval = XFS_FSB_TO_B(args->dp->i_mount, last) == args->geo->blksize;
	ASSERT(rval == 0 || args->dp->i_d.di_size == args->geo->blksize);
	*vp = rval;
	return 0;
}

/*
 * See if the directory is a single-leaf form directory.
 */
int
xfs_dir2_isleaf(
	struct xfs_da_args	*args,
	int			*vp)	/* out: 1 is block, 0 is not block */
{
	xfs_fileoff_t		last;	/* last file offset */
	int			rval;

	if ((rval = xfs_bmap_last_offset(args->dp, &last, XFS_DATA_FORK)))
		return rval;
	*vp = last == args->geo->leafblk + args->geo->fsbcount;
	return 0;
}

/*
 * Remove the given block from the directory.
 * This routine is used for data and free blocks, leaf/node are done
 * by xfs_da_shrink_inode.
 */
int
xfs_dir2_shrink_inode(
	xfs_da_args_t	*args,
	xfs_dir2_db_t	db,
	struct xfs_buf	*bp)
{
	xfs_fileoff_t	bno;		/* directory file offset */
	xfs_dablk_t	da;		/* directory file offset */
	int		done;		/* bunmap is finished */
	xfs_inode_t	*dp;
	int		error;
	xfs_mount_t	*mp;
	xfs_trans_t	*tp;

	trace_xfs_dir2_shrink_inode(args, db);

	dp = args->dp;
	mp = dp->i_mount;
	tp = args->trans;
	da = xfs_dir2_db_to_da(args->geo, db);
	/*
	 * Unmap the fsblock(s).
	 */
	if ((error = xfs_bunmapi(tp, dp, da, args->geo->fsbcount,
			XFS_BMAPI_METADATA, 0, args->firstblock, args->flist,
			&done))) {
		/*
		 * ENOSPC actually can happen if we're in a removename with
		 * no space reservation, and the resulting block removal
		 * would cause a bmap btree split or conversion from extents
		 * to btree.  This can only happen for un-fragmented
		 * directory blocks, since you need to be punching out
		 * the middle of an extent.
		 * In this case we need to leave the block in the file,
		 * and not binval it.
		 * So the block has to be in a consistent empty state
		 * and appropriately logged.
		 * We don't free up the buffer, the caller can tell it
		 * hasn't happened since it got an error back.
		 */
		return error;
	}
	ASSERT(done);
	/*
	 * Invalidate the buffer from the transaction.
	 */
	xfs_trans_binval(tp, bp);
	/*
	 * If it's not a data block, we're done.
	 */
	if (db >= xfs_dir2_byte_to_db(args->geo, XFS_DIR2_LEAF_OFFSET))
		return 0;
	/*
	 * If the block isn't the last one in the directory, we're done.
	 */
	if (dp->i_d.di_size > xfs_dir2_db_off_to_byte(args->geo, db + 1, 0))
		return 0;
	bno = da;
	if ((error = xfs_bmap_last_before(tp, dp, &bno, XFS_DATA_FORK))) {
		/*
		 * This can't really happen unless there's kernel corruption.
		 */
		return error;
	}
	if (db == args->geo->datablk)
		ASSERT(bno == 0);
	else
		ASSERT(bno > 0);
	/*
	 * Set the size to the new last block.
	 */
	dp->i_d.di_size = XFS_FSB_TO_B(mp, bno);
	xfs_trans_log_inode(tp, dp, XFS_ILOG_CORE);
	return 0;
}
