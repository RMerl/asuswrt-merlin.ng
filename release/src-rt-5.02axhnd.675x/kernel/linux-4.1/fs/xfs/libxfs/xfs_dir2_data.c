/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * Copyright (c) 2013 Red Hat, Inc.
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
#include "xfs_dir2.h"
#include "xfs_dir2_priv.h"
#include "xfs_error.h"
#include "xfs_trans.h"
#include "xfs_buf_item.h"
#include "xfs_cksum.h"

/*
 * Check the consistency of the data block.
 * The input can also be a block-format directory.
 * Return 0 is the buffer is good, otherwise an error.
 */
int
__xfs_dir3_data_check(
	struct xfs_inode	*dp,		/* incore inode pointer */
	struct xfs_buf		*bp)		/* data block's buffer */
{
	xfs_dir2_dataptr_t	addr;		/* addr for leaf lookup */
	xfs_dir2_data_free_t	*bf;		/* bestfree table */
	xfs_dir2_block_tail_t	*btp=NULL;	/* block tail */
	int			count;		/* count of entries found */
	xfs_dir2_data_hdr_t	*hdr;		/* data block header */
	xfs_dir2_data_entry_t	*dep;		/* data entry */
	xfs_dir2_data_free_t	*dfp;		/* bestfree entry */
	xfs_dir2_data_unused_t	*dup;		/* unused entry */
	char			*endp;		/* end of useful data */
	int			freeseen;	/* mask of bestfrees seen */
	xfs_dahash_t		hash;		/* hash of current name */
	int			i;		/* leaf index */
	int			lastfree;	/* last entry was unused */
	xfs_dir2_leaf_entry_t	*lep=NULL;	/* block leaf entries */
	xfs_mount_t		*mp;		/* filesystem mount point */
	char			*p;		/* current data position */
	int			stale;		/* count of stale leaves */
	struct xfs_name		name;
	const struct xfs_dir_ops *ops;
	struct xfs_da_geometry	*geo;

	mp = bp->b_target->bt_mount;
	geo = mp->m_dir_geo;

	/*
	 * We can be passed a null dp here from a verifier, so we need to go the
	 * hard way to get them.
	 */
	ops = xfs_dir_get_ops(mp, dp);

	hdr = bp->b_addr;
	p = (char *)ops->data_entry_p(hdr);

	switch (hdr->magic) {
	case cpu_to_be32(XFS_DIR3_BLOCK_MAGIC):
	case cpu_to_be32(XFS_DIR2_BLOCK_MAGIC):
		btp = xfs_dir2_block_tail_p(geo, hdr);
		lep = xfs_dir2_block_leaf_p(btp);
		endp = (char *)lep;

		/*
		 * The number of leaf entries is limited by the size of the
		 * block and the amount of space used by the data entries.
		 * We don't know how much space is used by the data entries yet,
		 * so just ensure that the count falls somewhere inside the
		 * block right now.
		 */
		XFS_WANT_CORRUPTED_RETURN(mp, be32_to_cpu(btp->count) <
			((char *)btp - p) / sizeof(struct xfs_dir2_leaf_entry));
		break;
	case cpu_to_be32(XFS_DIR3_DATA_MAGIC):
	case cpu_to_be32(XFS_DIR2_DATA_MAGIC):
		endp = (char *)hdr + geo->blksize;
		break;
	default:
		XFS_ERROR_REPORT("Bad Magic", XFS_ERRLEVEL_LOW, mp);
		return -EFSCORRUPTED;
	}

	/*
	 * Account for zero bestfree entries.
	 */
	bf = ops->data_bestfree_p(hdr);
	count = lastfree = freeseen = 0;
	if (!bf[0].length) {
		XFS_WANT_CORRUPTED_RETURN(mp, !bf[0].offset);
		freeseen |= 1 << 0;
	}
	if (!bf[1].length) {
		XFS_WANT_CORRUPTED_RETURN(mp, !bf[1].offset);
		freeseen |= 1 << 1;
	}
	if (!bf[2].length) {
		XFS_WANT_CORRUPTED_RETURN(mp, !bf[2].offset);
		freeseen |= 1 << 2;
	}

	XFS_WANT_CORRUPTED_RETURN(mp, be16_to_cpu(bf[0].length) >=
						be16_to_cpu(bf[1].length));
	XFS_WANT_CORRUPTED_RETURN(mp, be16_to_cpu(bf[1].length) >=
						be16_to_cpu(bf[2].length));
	/*
	 * Loop over the data/unused entries.
	 */
	while (p < endp) {
		dup = (xfs_dir2_data_unused_t *)p;
		/*
		 * If it's unused, look for the space in the bestfree table.
		 * If we find it, account for that, else make sure it
		 * doesn't need to be there.
		 */
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			XFS_WANT_CORRUPTED_RETURN(mp, lastfree == 0);
			XFS_WANT_CORRUPTED_RETURN(mp,
				be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)) ==
					       (char *)dup - (char *)hdr);
			dfp = xfs_dir2_data_freefind(hdr, bf, dup);
			if (dfp) {
				i = (int)(dfp - bf);
				XFS_WANT_CORRUPTED_RETURN(mp,
					(freeseen & (1 << i)) == 0);
				freeseen |= 1 << i;
			} else {
				XFS_WANT_CORRUPTED_RETURN(mp,
					be16_to_cpu(dup->length) <=
						be16_to_cpu(bf[2].length));
			}
			p += be16_to_cpu(dup->length);
			lastfree = 1;
			continue;
		}
		/*
		 * It's a real entry.  Validate the fields.
		 * If this is a block directory then make sure it's
		 * in the leaf section of the block.
		 * The linear search is crude but this is DEBUG code.
		 */
		dep = (xfs_dir2_data_entry_t *)p;
		XFS_WANT_CORRUPTED_RETURN(mp, dep->namelen != 0);
		XFS_WANT_CORRUPTED_RETURN(mp,
			!xfs_dir_ino_validate(mp, be64_to_cpu(dep->inumber)));
		XFS_WANT_CORRUPTED_RETURN(mp,
			be16_to_cpu(*ops->data_entry_tag_p(dep)) ==
					       (char *)dep - (char *)hdr);
		XFS_WANT_CORRUPTED_RETURN(mp,
				ops->data_get_ftype(dep) < XFS_DIR3_FT_MAX);
		count++;
		lastfree = 0;
		if (hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
		    hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC)) {
			addr = xfs_dir2_db_off_to_dataptr(geo, geo->datablk,
						(xfs_dir2_data_aoff_t)
						((char *)dep - (char *)hdr));
			name.name = dep->name;
			name.len = dep->namelen;
			hash = mp->m_dirnameops->hashname(&name);
			for (i = 0; i < be32_to_cpu(btp->count); i++) {
				if (be32_to_cpu(lep[i].address) == addr &&
				    be32_to_cpu(lep[i].hashval) == hash)
					break;
			}
			XFS_WANT_CORRUPTED_RETURN(mp,
						  i < be32_to_cpu(btp->count));
		}
		p += ops->data_entsize(dep->namelen);
	}
	/*
	 * Need to have seen all the entries and all the bestfree slots.
	 */
	XFS_WANT_CORRUPTED_RETURN(mp, freeseen == 7);
	if (hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	    hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC)) {
		for (i = stale = 0; i < be32_to_cpu(btp->count); i++) {
			if (lep[i].address ==
			    cpu_to_be32(XFS_DIR2_NULL_DATAPTR))
				stale++;
			if (i > 0)
				XFS_WANT_CORRUPTED_RETURN(mp,
					be32_to_cpu(lep[i].hashval) >=
						be32_to_cpu(lep[i - 1].hashval));
		}
		XFS_WANT_CORRUPTED_RETURN(mp, count ==
			be32_to_cpu(btp->count) - be32_to_cpu(btp->stale));
		XFS_WANT_CORRUPTED_RETURN(mp, stale == be32_to_cpu(btp->stale));
	}
	return 0;
}

static bool
xfs_dir3_data_verify(
	struct xfs_buf		*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;
	struct xfs_dir3_blk_hdr	*hdr3 = bp->b_addr;

	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		if (hdr3->magic != cpu_to_be32(XFS_DIR3_DATA_MAGIC))
			return false;
		if (!uuid_equal(&hdr3->uuid, &mp->m_sb.sb_uuid))
			return false;
		if (be64_to_cpu(hdr3->blkno) != bp->b_bn)
			return false;
	} else {
		if (hdr3->magic != cpu_to_be32(XFS_DIR2_DATA_MAGIC))
			return false;
	}
	if (__xfs_dir3_data_check(NULL, bp))
		return false;
	return true;
}

/*
 * Readahead of the first block of the directory when it is opened is completely
 * oblivious to the format of the directory. Hence we can either get a block
 * format buffer or a data format buffer on readahead.
 */
static void
xfs_dir3_data_reada_verify(
	struct xfs_buf		*bp)
{
	struct xfs_dir2_data_hdr *hdr = bp->b_addr;

	switch (hdr->magic) {
	case cpu_to_be32(XFS_DIR2_BLOCK_MAGIC):
	case cpu_to_be32(XFS_DIR3_BLOCK_MAGIC):
		bp->b_ops = &xfs_dir3_block_buf_ops;
		bp->b_ops->verify_read(bp);
		return;
	case cpu_to_be32(XFS_DIR2_DATA_MAGIC):
	case cpu_to_be32(XFS_DIR3_DATA_MAGIC):
		bp->b_ops = &xfs_dir3_data_buf_ops;
		bp->b_ops->verify_read(bp);
		return;
	default:
		xfs_buf_ioerror(bp, -EFSCORRUPTED);
		xfs_verifier_error(bp);
		break;
	}
}

static void
xfs_dir3_data_read_verify(
	struct xfs_buf	*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;

	if (xfs_sb_version_hascrc(&mp->m_sb) &&
	     !xfs_buf_verify_cksum(bp, XFS_DIR3_DATA_CRC_OFF))
		 xfs_buf_ioerror(bp, -EFSBADCRC);
	else if (!xfs_dir3_data_verify(bp))
		xfs_buf_ioerror(bp, -EFSCORRUPTED);

	if (bp->b_error)
		xfs_verifier_error(bp);
}

static void
xfs_dir3_data_write_verify(
	struct xfs_buf	*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;
	struct xfs_buf_log_item	*bip = bp->b_fspriv;
	struct xfs_dir3_blk_hdr	*hdr3 = bp->b_addr;

	if (!xfs_dir3_data_verify(bp)) {
		xfs_buf_ioerror(bp, -EFSCORRUPTED);
		xfs_verifier_error(bp);
		return;
	}

	if (!xfs_sb_version_hascrc(&mp->m_sb))
		return;

	if (bip)
		hdr3->lsn = cpu_to_be64(bip->bli_item.li_lsn);

	xfs_buf_update_cksum(bp, XFS_DIR3_DATA_CRC_OFF);
}

const struct xfs_buf_ops xfs_dir3_data_buf_ops = {
	.name = "xfs_dir3_data",
	.verify_read = xfs_dir3_data_read_verify,
	.verify_write = xfs_dir3_data_write_verify,
};

static const struct xfs_buf_ops xfs_dir3_data_reada_buf_ops = {
	.name = "xfs_dir3_data_reada",
	.verify_read = xfs_dir3_data_reada_verify,
	.verify_write = xfs_dir3_data_write_verify,
};


int
xfs_dir3_data_read(
	struct xfs_trans	*tp,
	struct xfs_inode	*dp,
	xfs_dablk_t		bno,
	xfs_daddr_t		mapped_bno,
	struct xfs_buf		**bpp)
{
	int			err;

	err = xfs_da_read_buf(tp, dp, bno, mapped_bno, bpp,
				XFS_DATA_FORK, &xfs_dir3_data_buf_ops);
	if (!err && tp)
		xfs_trans_buf_set_type(tp, *bpp, XFS_BLFT_DIR_DATA_BUF);
	return err;
}

int
xfs_dir3_data_readahead(
	struct xfs_inode	*dp,
	xfs_dablk_t		bno,
	xfs_daddr_t		mapped_bno)
{
	return xfs_da_reada_buf(dp, bno, mapped_bno,
				XFS_DATA_FORK, &xfs_dir3_data_reada_buf_ops);
}

/*
 * Given a data block and an unused entry from that block,
 * return the bestfree entry if any that corresponds to it.
 */
xfs_dir2_data_free_t *
xfs_dir2_data_freefind(
	struct xfs_dir2_data_hdr *hdr,		/* data block header */
	struct xfs_dir2_data_free *bf,		/* bestfree table pointer */
	struct xfs_dir2_data_unused *dup)	/* unused space */
{
	xfs_dir2_data_free_t	*dfp;		/* bestfree entry */
	xfs_dir2_data_aoff_t	off;		/* offset value needed */
#ifdef DEBUG
	int			matched;	/* matched the value */
	int			seenzero;	/* saw a 0 bestfree entry */
#endif

	off = (xfs_dir2_data_aoff_t)((char *)dup - (char *)hdr);

#ifdef DEBUG
	/*
	 * Validate some consistency in the bestfree table.
	 * Check order, non-overlapping entries, and if we find the
	 * one we're looking for it has to be exact.
	 */
	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));
	for (dfp = &bf[0], seenzero = matched = 0;
	     dfp < &bf[XFS_DIR2_DATA_FD_COUNT];
	     dfp++) {
		if (!dfp->offset) {
			ASSERT(!dfp->length);
			seenzero = 1;
			continue;
		}
		ASSERT(seenzero == 0);
		if (be16_to_cpu(dfp->offset) == off) {
			matched = 1;
			ASSERT(dfp->length == dup->length);
		} else if (off < be16_to_cpu(dfp->offset))
			ASSERT(off + be16_to_cpu(dup->length) <= be16_to_cpu(dfp->offset));
		else
			ASSERT(be16_to_cpu(dfp->offset) + be16_to_cpu(dfp->length) <= off);
		ASSERT(matched || be16_to_cpu(dfp->length) >= be16_to_cpu(dup->length));
		if (dfp > &bf[0])
			ASSERT(be16_to_cpu(dfp[-1].length) >= be16_to_cpu(dfp[0].length));
	}
#endif
	/*
	 * If this is smaller than the smallest bestfree entry,
	 * it can't be there since they're sorted.
	 */
	if (be16_to_cpu(dup->length) <
	    be16_to_cpu(bf[XFS_DIR2_DATA_FD_COUNT - 1].length))
		return NULL;
	/*
	 * Look at the three bestfree entries for our guy.
	 */
	for (dfp = &bf[0]; dfp < &bf[XFS_DIR2_DATA_FD_COUNT]; dfp++) {
		if (!dfp->offset)
			return NULL;
		if (be16_to_cpu(dfp->offset) == off)
			return dfp;
	}
	/*
	 * Didn't find it.  This only happens if there are duplicate lengths.
	 */
	return NULL;
}

/*
 * Insert an unused-space entry into the bestfree table.
 */
xfs_dir2_data_free_t *				/* entry inserted */
xfs_dir2_data_freeinsert(
	struct xfs_dir2_data_hdr *hdr,		/* data block pointer */
	struct xfs_dir2_data_free *dfp,		/* bestfree table pointer */
	struct xfs_dir2_data_unused *dup,	/* unused space */
	int			*loghead)	/* log the data header (out) */
{
	xfs_dir2_data_free_t	new;		/* new bestfree entry */

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));

	new.length = dup->length;
	new.offset = cpu_to_be16((char *)dup - (char *)hdr);

	/*
	 * Insert at position 0, 1, or 2; or not at all.
	 */
	if (be16_to_cpu(new.length) > be16_to_cpu(dfp[0].length)) {
		dfp[2] = dfp[1];
		dfp[1] = dfp[0];
		dfp[0] = new;
		*loghead = 1;
		return &dfp[0];
	}
	if (be16_to_cpu(new.length) > be16_to_cpu(dfp[1].length)) {
		dfp[2] = dfp[1];
		dfp[1] = new;
		*loghead = 1;
		return &dfp[1];
	}
	if (be16_to_cpu(new.length) > be16_to_cpu(dfp[2].length)) {
		dfp[2] = new;
		*loghead = 1;
		return &dfp[2];
	}
	return NULL;
}

/*
 * Remove a bestfree entry from the table.
 */
STATIC void
xfs_dir2_data_freeremove(
	struct xfs_dir2_data_hdr *hdr,		/* data block header */
	struct xfs_dir2_data_free *bf,		/* bestfree table pointer */
	struct xfs_dir2_data_free *dfp,		/* bestfree entry pointer */
	int			*loghead)	/* out: log data header */
{

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));

	/*
	 * It's the first entry, slide the next 2 up.
	 */
	if (dfp == &bf[0]) {
		bf[0] = bf[1];
		bf[1] = bf[2];
	}
	/*
	 * It's the second entry, slide the 3rd entry up.
	 */
	else if (dfp == &bf[1])
		bf[1] = bf[2];
	/*
	 * Must be the last entry.
	 */
	else
		ASSERT(dfp == &bf[2]);
	/*
	 * Clear the 3rd entry, must be zero now.
	 */
	bf[2].length = 0;
	bf[2].offset = 0;
	*loghead = 1;
}

/*
 * Given a data block, reconstruct its bestfree map.
 */
void
xfs_dir2_data_freescan(
	struct xfs_inode	*dp,
	struct xfs_dir2_data_hdr *hdr,
	int			*loghead)
{
	xfs_dir2_block_tail_t	*btp;		/* block tail */
	xfs_dir2_data_entry_t	*dep;		/* active data entry */
	xfs_dir2_data_unused_t	*dup;		/* unused data entry */
	struct xfs_dir2_data_free *bf;
	char			*endp;		/* end of block's data */
	char			*p;		/* current entry pointer */
	struct xfs_da_geometry	*geo = dp->i_mount->m_dir_geo;

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));

	/*
	 * Start by clearing the table.
	 */
	bf = dp->d_ops->data_bestfree_p(hdr);
	memset(bf, 0, sizeof(*bf) * XFS_DIR2_DATA_FD_COUNT);
	*loghead = 1;
	/*
	 * Set up pointers.
	 */
	p = (char *)dp->d_ops->data_entry_p(hdr);
	if (hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	    hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC)) {
		btp = xfs_dir2_block_tail_p(geo, hdr);
		endp = (char *)xfs_dir2_block_leaf_p(btp);
	} else
		endp = (char *)hdr + geo->blksize;
	/*
	 * Loop over the block's entries.
	 */
	while (p < endp) {
		dup = (xfs_dir2_data_unused_t *)p;
		/*
		 * If it's a free entry, insert it.
		 */
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			ASSERT((char *)dup - (char *)hdr ==
			       be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)));
			xfs_dir2_data_freeinsert(hdr, bf, dup, loghead);
			p += be16_to_cpu(dup->length);
		}
		/*
		 * For active entries, check their tags and skip them.
		 */
		else {
			dep = (xfs_dir2_data_entry_t *)p;
			ASSERT((char *)dep - (char *)hdr ==
			       be16_to_cpu(*dp->d_ops->data_entry_tag_p(dep)));
			p += dp->d_ops->data_entsize(dep->namelen);
		}
	}
}

/*
 * Initialize a data block at the given block number in the directory.
 * Give back the buffer for the created block.
 */
int						/* error */
xfs_dir3_data_init(
	xfs_da_args_t		*args,		/* directory operation args */
	xfs_dir2_db_t		blkno,		/* logical dir block number */
	struct xfs_buf		**bpp)		/* output block buffer */
{
	struct xfs_buf		*bp;		/* block buffer */
	xfs_dir2_data_hdr_t	*hdr;		/* data block header */
	xfs_inode_t		*dp;		/* incore directory inode */
	xfs_dir2_data_unused_t	*dup;		/* unused entry pointer */
	struct xfs_dir2_data_free *bf;
	int			error;		/* error return value */
	int			i;		/* bestfree index */
	xfs_mount_t		*mp;		/* filesystem mount point */
	xfs_trans_t		*tp;		/* transaction pointer */
	int                     t;              /* temp */

	dp = args->dp;
	mp = dp->i_mount;
	tp = args->trans;
	/*
	 * Get the buffer set up for the block.
	 */
	error = xfs_da_get_buf(tp, dp, xfs_dir2_db_to_da(args->geo, blkno),
			       -1, &bp, XFS_DATA_FORK);
	if (error)
		return error;
	bp->b_ops = &xfs_dir3_data_buf_ops;
	xfs_trans_buf_set_type(tp, bp, XFS_BLFT_DIR_DATA_BUF);

	/*
	 * Initialize the header.
	 */
	hdr = bp->b_addr;
	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		struct xfs_dir3_blk_hdr *hdr3 = bp->b_addr;

		memset(hdr3, 0, sizeof(*hdr3));
		hdr3->magic = cpu_to_be32(XFS_DIR3_DATA_MAGIC);
		hdr3->blkno = cpu_to_be64(bp->b_bn);
		hdr3->owner = cpu_to_be64(dp->i_ino);
		uuid_copy(&hdr3->uuid, &mp->m_sb.sb_uuid);

	} else
		hdr->magic = cpu_to_be32(XFS_DIR2_DATA_MAGIC);

	bf = dp->d_ops->data_bestfree_p(hdr);
	bf[0].offset = cpu_to_be16(dp->d_ops->data_entry_offset);
	for (i = 1; i < XFS_DIR2_DATA_FD_COUNT; i++) {
		bf[i].length = 0;
		bf[i].offset = 0;
	}

	/*
	 * Set up an unused entry for the block's body.
	 */
	dup = dp->d_ops->data_unused_p(hdr);
	dup->freetag = cpu_to_be16(XFS_DIR2_DATA_FREE_TAG);

	t = args->geo->blksize - (uint)dp->d_ops->data_entry_offset;
	bf[0].length = cpu_to_be16(t);
	dup->length = cpu_to_be16(t);
	*xfs_dir2_data_unused_tag_p(dup) = cpu_to_be16((char *)dup - (char *)hdr);
	/*
	 * Log it and return it.
	 */
	xfs_dir2_data_log_header(args, bp);
	xfs_dir2_data_log_unused(args, bp, dup);
	*bpp = bp;
	return 0;
}

/*
 * Log an active data entry from the block.
 */
void
xfs_dir2_data_log_entry(
	struct xfs_da_args	*args,
	struct xfs_buf		*bp,
	xfs_dir2_data_entry_t	*dep)		/* data entry pointer */
{
	struct xfs_dir2_data_hdr *hdr = bp->b_addr;

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));

	xfs_trans_log_buf(args->trans, bp, (uint)((char *)dep - (char *)hdr),
		(uint)((char *)(args->dp->d_ops->data_entry_tag_p(dep) + 1) -
		       (char *)hdr - 1));
}

/*
 * Log a data block header.
 */
void
xfs_dir2_data_log_header(
	struct xfs_da_args	*args,
	struct xfs_buf		*bp)
{
#ifdef DEBUG
	struct xfs_dir2_data_hdr *hdr = bp->b_addr;

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));
#endif

	xfs_trans_log_buf(args->trans, bp, 0,
			  args->dp->d_ops->data_entry_offset - 1);
}

/*
 * Log a data unused entry.
 */
void
xfs_dir2_data_log_unused(
	struct xfs_da_args	*args,
	struct xfs_buf		*bp,
	xfs_dir2_data_unused_t	*dup)		/* data unused pointer */
{
	xfs_dir2_data_hdr_t	*hdr = bp->b_addr;

	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));

	/*
	 * Log the first part of the unused entry.
	 */
	xfs_trans_log_buf(args->trans, bp, (uint)((char *)dup - (char *)hdr),
		(uint)((char *)&dup->length + sizeof(dup->length) -
		       1 - (char *)hdr));
	/*
	 * Log the end (tag) of the unused entry.
	 */
	xfs_trans_log_buf(args->trans, bp,
		(uint)((char *)xfs_dir2_data_unused_tag_p(dup) - (char *)hdr),
		(uint)((char *)xfs_dir2_data_unused_tag_p(dup) - (char *)hdr +
		       sizeof(xfs_dir2_data_off_t) - 1));
}

/*
 * Make a byte range in the data block unused.
 * Its current contents are unimportant.
 */
void
xfs_dir2_data_make_free(
	struct xfs_da_args	*args,
	struct xfs_buf		*bp,
	xfs_dir2_data_aoff_t	offset,		/* starting byte offset */
	xfs_dir2_data_aoff_t	len,		/* length in bytes */
	int			*needlogp,	/* out: log header */
	int			*needscanp)	/* out: regen bestfree */
{
	xfs_dir2_data_hdr_t	*hdr;		/* data block pointer */
	xfs_dir2_data_free_t	*dfp;		/* bestfree pointer */
	char			*endptr;	/* end of data area */
	int			needscan;	/* need to regen bestfree */
	xfs_dir2_data_unused_t	*newdup;	/* new unused entry */
	xfs_dir2_data_unused_t	*postdup;	/* unused entry after us */
	xfs_dir2_data_unused_t	*prevdup;	/* unused entry before us */
	struct xfs_dir2_data_free *bf;

	hdr = bp->b_addr;

	/*
	 * Figure out where the end of the data area is.
	 */
	if (hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	    hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC))
		endptr = (char *)hdr + args->geo->blksize;
	else {
		xfs_dir2_block_tail_t	*btp;	/* block tail */

		ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
			hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));
		btp = xfs_dir2_block_tail_p(args->geo, hdr);
		endptr = (char *)xfs_dir2_block_leaf_p(btp);
	}
	/*
	 * If this isn't the start of the block, then back up to
	 * the previous entry and see if it's free.
	 */
	if (offset > args->dp->d_ops->data_entry_offset) {
		__be16			*tagp;	/* tag just before us */

		tagp = (__be16 *)((char *)hdr + offset) - 1;
		prevdup = (xfs_dir2_data_unused_t *)((char *)hdr + be16_to_cpu(*tagp));
		if (be16_to_cpu(prevdup->freetag) != XFS_DIR2_DATA_FREE_TAG)
			prevdup = NULL;
	} else
		prevdup = NULL;
	/*
	 * If this isn't the end of the block, see if the entry after
	 * us is free.
	 */
	if ((char *)hdr + offset + len < endptr) {
		postdup =
			(xfs_dir2_data_unused_t *)((char *)hdr + offset + len);
		if (be16_to_cpu(postdup->freetag) != XFS_DIR2_DATA_FREE_TAG)
			postdup = NULL;
	} else
		postdup = NULL;
	ASSERT(*needscanp == 0);
	needscan = 0;
	/*
	 * Previous and following entries are both free,
	 * merge everything into a single free entry.
	 */
	bf = args->dp->d_ops->data_bestfree_p(hdr);
	if (prevdup && postdup) {
		xfs_dir2_data_free_t	*dfp2;	/* another bestfree pointer */

		/*
		 * See if prevdup and/or postdup are in bestfree table.
		 */
		dfp = xfs_dir2_data_freefind(hdr, bf, prevdup);
		dfp2 = xfs_dir2_data_freefind(hdr, bf, postdup);
		/*
		 * We need a rescan unless there are exactly 2 free entries
		 * namely our two.  Then we know what's happening, otherwise
		 * since the third bestfree is there, there might be more
		 * entries.
		 */
		needscan = (bf[2].length != 0);
		/*
		 * Fix up the new big freespace.
		 */
		be16_add_cpu(&prevdup->length, len + be16_to_cpu(postdup->length));
		*xfs_dir2_data_unused_tag_p(prevdup) =
			cpu_to_be16((char *)prevdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, prevdup);
		if (!needscan) {
			/*
			 * Has to be the case that entries 0 and 1 are
			 * dfp and dfp2 (don't know which is which), and
			 * entry 2 is empty.
			 * Remove entry 1 first then entry 0.
			 */
			ASSERT(dfp && dfp2);
			if (dfp == &bf[1]) {
				dfp = &bf[0];
				ASSERT(dfp2 == dfp);
				dfp2 = &bf[1];
			}
			xfs_dir2_data_freeremove(hdr, bf, dfp2, needlogp);
			xfs_dir2_data_freeremove(hdr, bf, dfp, needlogp);
			/*
			 * Now insert the new entry.
			 */
			dfp = xfs_dir2_data_freeinsert(hdr, bf, prevdup,
						       needlogp);
			ASSERT(dfp == &bf[0]);
			ASSERT(dfp->length == prevdup->length);
			ASSERT(!dfp[1].length);
			ASSERT(!dfp[2].length);
		}
	}
	/*
	 * The entry before us is free, merge with it.
	 */
	else if (prevdup) {
		dfp = xfs_dir2_data_freefind(hdr, bf, prevdup);
		be16_add_cpu(&prevdup->length, len);
		*xfs_dir2_data_unused_tag_p(prevdup) =
			cpu_to_be16((char *)prevdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, prevdup);
		/*
		 * If the previous entry was in the table, the new entry
		 * is longer, so it will be in the table too.  Remove
		 * the old one and add the new one.
		 */
		if (dfp) {
			xfs_dir2_data_freeremove(hdr, bf, dfp, needlogp);
			xfs_dir2_data_freeinsert(hdr, bf, prevdup, needlogp);
		}
		/*
		 * Otherwise we need a scan if the new entry is big enough.
		 */
		else {
			needscan = be16_to_cpu(prevdup->length) >
				   be16_to_cpu(bf[2].length);
		}
	}
	/*
	 * The following entry is free, merge with it.
	 */
	else if (postdup) {
		dfp = xfs_dir2_data_freefind(hdr, bf, postdup);
		newdup = (xfs_dir2_data_unused_t *)((char *)hdr + offset);
		newdup->freetag = cpu_to_be16(XFS_DIR2_DATA_FREE_TAG);
		newdup->length = cpu_to_be16(len + be16_to_cpu(postdup->length));
		*xfs_dir2_data_unused_tag_p(newdup) =
			cpu_to_be16((char *)newdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup);
		/*
		 * If the following entry was in the table, the new entry
		 * is longer, so it will be in the table too.  Remove
		 * the old one and add the new one.
		 */
		if (dfp) {
			xfs_dir2_data_freeremove(hdr, bf, dfp, needlogp);
			xfs_dir2_data_freeinsert(hdr, bf, newdup, needlogp);
		}
		/*
		 * Otherwise we need a scan if the new entry is big enough.
		 */
		else {
			needscan = be16_to_cpu(newdup->length) >
				   be16_to_cpu(bf[2].length);
		}
	}
	/*
	 * Neither neighbor is free.  Make a new entry.
	 */
	else {
		newdup = (xfs_dir2_data_unused_t *)((char *)hdr + offset);
		newdup->freetag = cpu_to_be16(XFS_DIR2_DATA_FREE_TAG);
		newdup->length = cpu_to_be16(len);
		*xfs_dir2_data_unused_tag_p(newdup) =
			cpu_to_be16((char *)newdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup);
		xfs_dir2_data_freeinsert(hdr, bf, newdup, needlogp);
	}
	*needscanp = needscan;
}

/*
 * Take a byte range out of an existing unused space and make it un-free.
 */
void
xfs_dir2_data_use_free(
	struct xfs_da_args	*args,
	struct xfs_buf		*bp,
	xfs_dir2_data_unused_t	*dup,		/* unused entry */
	xfs_dir2_data_aoff_t	offset,		/* starting offset to use */
	xfs_dir2_data_aoff_t	len,		/* length to use */
	int			*needlogp,	/* out: need to log header */
	int			*needscanp)	/* out: need regen bestfree */
{
	xfs_dir2_data_hdr_t	*hdr;		/* data block header */
	xfs_dir2_data_free_t	*dfp;		/* bestfree pointer */
	int			matchback;	/* matches end of freespace */
	int			matchfront;	/* matches start of freespace */
	int			needscan;	/* need to regen bestfree */
	xfs_dir2_data_unused_t	*newdup;	/* new unused entry */
	xfs_dir2_data_unused_t	*newdup2;	/* another new unused entry */
	int			oldlen;		/* old unused entry's length */
	struct xfs_dir2_data_free *bf;

	hdr = bp->b_addr;
	ASSERT(hdr->magic == cpu_to_be32(XFS_DIR2_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_DATA_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR2_BLOCK_MAGIC) ||
	       hdr->magic == cpu_to_be32(XFS_DIR3_BLOCK_MAGIC));
	ASSERT(be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG);
	ASSERT(offset >= (char *)dup - (char *)hdr);
	ASSERT(offset + len <= (char *)dup + be16_to_cpu(dup->length) - (char *)hdr);
	ASSERT((char *)dup - (char *)hdr == be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)));
	/*
	 * Look up the entry in the bestfree table.
	 */
	oldlen = be16_to_cpu(dup->length);
	bf = args->dp->d_ops->data_bestfree_p(hdr);
	dfp = xfs_dir2_data_freefind(hdr, bf, dup);
	ASSERT(dfp || oldlen <= be16_to_cpu(bf[2].length));
	/*
	 * Check for alignment with front and back of the entry.
	 */
	matchfront = (char *)dup - (char *)hdr == offset;
	matchback = (char *)dup + oldlen - (char *)hdr == offset + len;
	ASSERT(*needscanp == 0);
	needscan = 0;
	/*
	 * If we matched it exactly we just need to get rid of it from
	 * the bestfree table.
	 */
	if (matchfront && matchback) {
		if (dfp) {
			needscan = (bf[2].offset != 0);
			if (!needscan)
				xfs_dir2_data_freeremove(hdr, bf, dfp,
							 needlogp);
		}
	}
	/*
	 * We match the first part of the entry.
	 * Make a new entry with the remaining freespace.
	 */
	else if (matchfront) {
		newdup = (xfs_dir2_data_unused_t *)((char *)hdr + offset + len);
		newdup->freetag = cpu_to_be16(XFS_DIR2_DATA_FREE_TAG);
		newdup->length = cpu_to_be16(oldlen - len);
		*xfs_dir2_data_unused_tag_p(newdup) =
			cpu_to_be16((char *)newdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup);
		/*
		 * If it was in the table, remove it and add the new one.
		 */
		if (dfp) {
			xfs_dir2_data_freeremove(hdr, bf, dfp, needlogp);
			dfp = xfs_dir2_data_freeinsert(hdr, bf, newdup,
						       needlogp);
			ASSERT(dfp != NULL);
			ASSERT(dfp->length == newdup->length);
			ASSERT(be16_to_cpu(dfp->offset) == (char *)newdup - (char *)hdr);
			/*
			 * If we got inserted at the last slot,
			 * that means we don't know if there was a better
			 * choice for the last slot, or not.  Rescan.
			 */
			needscan = dfp == &bf[2];
		}
	}
	/*
	 * We match the last part of the entry.
	 * Trim the allocated space off the tail of the entry.
	 */
	else if (matchback) {
		newdup = dup;
		newdup->length = cpu_to_be16(((char *)hdr + offset) - (char *)newdup);
		*xfs_dir2_data_unused_tag_p(newdup) =
			cpu_to_be16((char *)newdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup);
		/*
		 * If it was in the table, remove it and add the new one.
		 */
		if (dfp) {
			xfs_dir2_data_freeremove(hdr, bf, dfp, needlogp);
			dfp = xfs_dir2_data_freeinsert(hdr, bf, newdup,
						       needlogp);
			ASSERT(dfp != NULL);
			ASSERT(dfp->length == newdup->length);
			ASSERT(be16_to_cpu(dfp->offset) == (char *)newdup - (char *)hdr);
			/*
			 * If we got inserted at the last slot,
			 * that means we don't know if there was a better
			 * choice for the last slot, or not.  Rescan.
			 */
			needscan = dfp == &bf[2];
		}
	}
	/*
	 * Poking out the middle of an entry.
	 * Make two new entries.
	 */
	else {
		newdup = dup;
		newdup->length = cpu_to_be16(((char *)hdr + offset) - (char *)newdup);
		*xfs_dir2_data_unused_tag_p(newdup) =
			cpu_to_be16((char *)newdup - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup);
		newdup2 = (xfs_dir2_data_unused_t *)((char *)hdr + offset + len);
		newdup2->freetag = cpu_to_be16(XFS_DIR2_DATA_FREE_TAG);
		newdup2->length = cpu_to_be16(oldlen - len - be16_to_cpu(newdup->length));
		*xfs_dir2_data_unused_tag_p(newdup2) =
			cpu_to_be16((char *)newdup2 - (char *)hdr);
		xfs_dir2_data_log_unused(args, bp, newdup2);
		/*
		 * If the old entry was in the table, we need to scan
		 * if the 3rd entry was valid, since these entries
		 * are smaller than the old one.
		 * If we don't need to scan that means there were 1 or 2
		 * entries in the table, and removing the old and adding
		 * the 2 new will work.
		 */
		if (dfp) {
			needscan = (bf[2].length != 0);
			if (!needscan) {
				xfs_dir2_data_freeremove(hdr, bf, dfp,
							 needlogp);
				xfs_dir2_data_freeinsert(hdr, bf, newdup,
							 needlogp);
				xfs_dir2_data_freeinsert(hdr, bf, newdup2,
							 needlogp);
			}
		}
	}
	*needscanp = needscan;
}
