/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#ifndef __XFS_QM_H__
#define __XFS_QM_H__

#include "xfs_dquot_item.h"
#include "xfs_dquot.h"

struct xfs_inode;

extern struct kmem_zone	*xfs_qm_dqtrxzone;

/*
 * Number of bmaps that we ask from bmapi when doing a quotacheck.
 * We make this restriction to keep the memory usage to a minimum.
 */
#define XFS_DQITER_MAP_SIZE	10

#define XFS_IS_DQUOT_UNINITIALIZED(dqp) ( \
	!dqp->q_core.d_blk_hardlimit && \
	!dqp->q_core.d_blk_softlimit && \
	!dqp->q_core.d_rtb_hardlimit && \
	!dqp->q_core.d_rtb_softlimit && \
	!dqp->q_core.d_ino_hardlimit && \
	!dqp->q_core.d_ino_softlimit && \
	!dqp->q_core.d_bcount && \
	!dqp->q_core.d_rtbcount && \
	!dqp->q_core.d_icount)

/*
 * This defines the unit of allocation of dquots.
 * Currently, it is just one file system block, and a 4K blk contains 30
 * (136 * 30 = 4080) dquots. It's probably not worth trying to make
 * this more dynamic.
 * XXXsup However, if this number is changed, we have to make sure that we don't
 * implicitly assume that we do allocations in chunks of a single filesystem
 * block in the dquot/xqm code.
 */
#define XFS_DQUOT_CLUSTER_SIZE_FSB	(xfs_filblks_t)1

/*
 * Various quota information for individual filesystems.
 * The mount structure keeps a pointer to this.
 */
typedef struct xfs_quotainfo {
	struct radix_tree_root qi_uquota_tree;
	struct radix_tree_root qi_gquota_tree;
	struct radix_tree_root qi_pquota_tree;
	struct mutex qi_tree_lock;
	struct xfs_inode	*qi_uquotaip;	/* user quota inode */
	struct xfs_inode	*qi_gquotaip;	/* group quota inode */
	struct xfs_inode	*qi_pquotaip;	/* project quota inode */
	struct list_lru	 qi_lru;
	int		 qi_dquots;
	time_t		 qi_btimelimit;	 /* limit for blks timer */
	time_t		 qi_itimelimit;	 /* limit for inodes timer */
	time_t		 qi_rtbtimelimit;/* limit for rt blks timer */
	xfs_qwarncnt_t	 qi_bwarnlimit;	 /* limit for blks warnings */
	xfs_qwarncnt_t	 qi_iwarnlimit;	 /* limit for inodes warnings */
	xfs_qwarncnt_t	 qi_rtbwarnlimit;/* limit for rt blks warnings */
	struct mutex	 qi_quotaofflock;/* to serialize quotaoff */
	xfs_filblks_t	 qi_dqchunklen;	 /* # BBs in a chunk of dqs */
	uint		 qi_dqperchunk;	 /* # ondisk dqs in above chunk */
	xfs_qcnt_t	 qi_bhardlimit;	 /* default data blk hard limit */
	xfs_qcnt_t	 qi_bsoftlimit;	 /* default data blk soft limit */
	xfs_qcnt_t	 qi_ihardlimit;	 /* default inode count hard limit */
	xfs_qcnt_t	 qi_isoftlimit;	 /* default inode count soft limit */
	xfs_qcnt_t	 qi_rtbhardlimit;/* default realtime blk hard limit */
	xfs_qcnt_t	 qi_rtbsoftlimit;/* default realtime blk soft limit */
	struct shrinker  qi_shrinker;
} xfs_quotainfo_t;

static inline struct radix_tree_root *
xfs_dquot_tree(
	struct xfs_quotainfo	*qi,
	int			type)
{
	switch (type) {
	case XFS_DQ_USER:
		return &qi->qi_uquota_tree;
	case XFS_DQ_GROUP:
		return &qi->qi_gquota_tree;
	case XFS_DQ_PROJ:
		return &qi->qi_pquota_tree;
	default:
		ASSERT(0);
	}
	return NULL;
}

static inline struct xfs_inode *
xfs_dq_to_quota_inode(struct xfs_dquot *dqp)
{
	switch (dqp->dq_flags & XFS_DQ_ALLTYPES) {
	case XFS_DQ_USER:
		return dqp->q_mount->m_quotainfo->qi_uquotaip;
	case XFS_DQ_GROUP:
		return dqp->q_mount->m_quotainfo->qi_gquotaip;
	case XFS_DQ_PROJ:
		return dqp->q_mount->m_quotainfo->qi_pquotaip;
	default:
		ASSERT(0);
	}
	return NULL;
}

extern void	xfs_trans_mod_dquot(struct xfs_trans *,
					struct xfs_dquot *, uint, long);
extern int	xfs_trans_reserve_quota_bydquots(struct xfs_trans *,
			struct xfs_mount *, struct xfs_dquot *,
			struct xfs_dquot *, struct xfs_dquot *,
			long, long, uint);
extern void	xfs_trans_dqjoin(struct xfs_trans *, struct xfs_dquot *);
extern void	xfs_trans_log_dquot(struct xfs_trans *, struct xfs_dquot *);

/*
 * We keep the usr, grp, and prj dquots separately so that locking will be
 * easier to do at commit time. All transactions that we know of at this point
 * affect no more than two dquots of one type. Hence, the TRANS_MAXDQS value.
 */
enum {
	XFS_QM_TRANS_USR = 0,
	XFS_QM_TRANS_GRP,
	XFS_QM_TRANS_PRJ,
	XFS_QM_TRANS_DQTYPES
};
#define XFS_QM_TRANS_MAXDQS		2
struct xfs_dquot_acct {
	struct xfs_dqtrx	dqs[XFS_QM_TRANS_DQTYPES][XFS_QM_TRANS_MAXDQS];
};

/*
 * Users are allowed to have a usage exceeding their softlimit for
 * a period this long.
 */
#define XFS_QM_BTIMELIMIT	(7 * 24*60*60)          /* 1 week */
#define XFS_QM_RTBTIMELIMIT	(7 * 24*60*60)          /* 1 week */
#define XFS_QM_ITIMELIMIT	(7 * 24*60*60)          /* 1 week */

#define XFS_QM_BWARNLIMIT	5
#define XFS_QM_IWARNLIMIT	5
#define XFS_QM_RTBWARNLIMIT	5

extern void		xfs_qm_destroy_quotainfo(struct xfs_mount *);

/* dquot stuff */
extern void		xfs_qm_dqpurge_all(struct xfs_mount *, uint);
extern void		xfs_qm_dqrele_all_inodes(struct xfs_mount *, uint);

/* quota ops */
extern int		xfs_qm_scall_trunc_qfiles(struct xfs_mount *, uint);
extern int		xfs_qm_scall_getquota(struct xfs_mount *, xfs_dqid_t,
					uint, struct qc_dqblk *);
extern int		xfs_qm_scall_setqlim(struct xfs_mount *, xfs_dqid_t, uint,
					struct qc_dqblk *);
extern int		xfs_qm_scall_quotaon(struct xfs_mount *, uint);
extern int		xfs_qm_scall_quotaoff(struct xfs_mount *, uint);

#endif /* __XFS_QM_H__ */
