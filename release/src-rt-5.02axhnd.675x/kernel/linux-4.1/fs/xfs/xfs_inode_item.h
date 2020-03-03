/*
 * Copyright (c) 2000,2005 Silicon Graphics, Inc.
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
#ifndef	__XFS_INODE_ITEM_H__
#define	__XFS_INODE_ITEM_H__

/* kernel only definitions */

struct xfs_buf;
struct xfs_bmbt_rec;
struct xfs_inode;
struct xfs_mount;

typedef struct xfs_inode_log_item {
	xfs_log_item_t		ili_item;	   /* common portion */
	struct xfs_inode	*ili_inode;	   /* inode ptr */
	xfs_lsn_t		ili_flush_lsn;	   /* lsn at last flush */
	xfs_lsn_t		ili_last_lsn;	   /* lsn at last transaction */
	unsigned short		ili_lock_flags;	   /* lock flags */
	unsigned short		ili_logged;	   /* flushed logged data */
	unsigned int		ili_last_fields;   /* fields when flushed */
	unsigned int		ili_fields;	   /* fields to be logged */
} xfs_inode_log_item_t;

static inline int xfs_inode_clean(xfs_inode_t *ip)
{
	return !ip->i_itemp || !(ip->i_itemp->ili_fields & XFS_ILOG_ALL);
}

extern void xfs_inode_item_init(struct xfs_inode *, struct xfs_mount *);
extern void xfs_inode_item_destroy(struct xfs_inode *);
extern void xfs_iflush_done(struct xfs_buf *, struct xfs_log_item *);
extern void xfs_istale_done(struct xfs_buf *, struct xfs_log_item *);
extern void xfs_iflush_abort(struct xfs_inode *, bool);
extern int xfs_inode_item_format_convert(xfs_log_iovec_t *,
					 xfs_inode_log_format_t *);

extern struct kmem_zone	*xfs_ili_zone;

#endif	/* __XFS_INODE_ITEM_H__ */
