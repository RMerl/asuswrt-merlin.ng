/*
 * NFSv4 flexfile layout driver data structures.
 *
 * Copyright (c) 2014, Primary Data, Inc. All rights reserved.
 *
 * Tao Peng <bergwolf@primarydata.com>
 */

#ifndef FS_NFS_NFS4FLEXFILELAYOUT_H
#define FS_NFS_NFS4FLEXFILELAYOUT_H

#include "../pnfs.h"

/* XXX: Let's filter out insanely large mirror count for now to avoid oom
 * due to network error etc. */
#define NFS4_FLEXFILE_LAYOUT_MAX_MIRROR_CNT 4096

struct nfs4_ff_ds_version {
	u32				version;
	u32				minor_version;
	u32				rsize;
	u32				wsize;
	bool				tightly_coupled;
};

/* chained in global deviceid hlist */
struct nfs4_ff_layout_ds {
	struct nfs4_deviceid_node	id_node;
	u32				ds_versions_cnt;
	struct nfs4_ff_ds_version	*ds_versions;
	struct nfs4_pnfs_ds		*ds;
};

struct nfs4_ff_layout_ds_err {
	struct list_head		list; /* linked in mirror error_list */
	u64				offset;
	u64				length;
	int				status;
	enum nfs_opnum4			opnum;
	nfs4_stateid			stateid;
	struct nfs4_deviceid		deviceid;
};

struct nfs4_ff_layout_mirror {
	u32				ds_count;
	u32				efficiency;
	struct nfs4_ff_layout_ds	*mirror_ds;
	u32				fh_versions_cnt;
	struct nfs_fh			*fh_versions;
	nfs4_stateid			stateid;
	struct nfs4_string		user_name;
	struct nfs4_string		group_name;
	u32				uid;
	u32				gid;
	struct rpc_cred			*cred;
	spinlock_t			lock;
};

struct nfs4_ff_layout_segment {
	struct pnfs_layout_segment	generic_hdr;
	u64				stripe_unit;
	u32				mirror_array_cnt;
	struct nfs4_ff_layout_mirror	**mirror_array;
};

struct nfs4_flexfile_layout {
	struct pnfs_layout_hdr generic_hdr;
	struct pnfs_ds_commit_info commit_info;
	struct list_head	error_list; /* nfs4_ff_layout_ds_err */
};

static inline struct nfs4_flexfile_layout *
FF_LAYOUT_FROM_HDR(struct pnfs_layout_hdr *lo)
{
	return container_of(lo, struct nfs4_flexfile_layout, generic_hdr);
}

static inline struct nfs4_ff_layout_segment *
FF_LAYOUT_LSEG(struct pnfs_layout_segment *lseg)
{
	return container_of(lseg,
			    struct nfs4_ff_layout_segment,
			    generic_hdr);
}

static inline struct nfs4_deviceid_node *
FF_LAYOUT_DEVID_NODE(struct pnfs_layout_segment *lseg, u32 idx)
{
	if (idx >= FF_LAYOUT_LSEG(lseg)->mirror_array_cnt ||
	    FF_LAYOUT_LSEG(lseg)->mirror_array[idx] == NULL ||
	    FF_LAYOUT_LSEG(lseg)->mirror_array[idx]->mirror_ds == NULL)
		return NULL;
	return &FF_LAYOUT_LSEG(lseg)->mirror_array[idx]->mirror_ds->id_node;
}

static inline struct nfs4_ff_layout_ds *
FF_LAYOUT_MIRROR_DS(struct nfs4_deviceid_node *node)
{
	return container_of(node, struct nfs4_ff_layout_ds, id_node);
}

static inline struct nfs4_ff_layout_mirror *
FF_LAYOUT_COMP(struct pnfs_layout_segment *lseg, u32 idx)
{
	if (idx >= FF_LAYOUT_LSEG(lseg)->mirror_array_cnt)
		return NULL;
	return FF_LAYOUT_LSEG(lseg)->mirror_array[idx];
}

static inline u32
FF_LAYOUT_MIRROR_COUNT(struct pnfs_layout_segment *lseg)
{
	return FF_LAYOUT_LSEG(lseg)->mirror_array_cnt;
}

static inline bool
ff_layout_test_devid_unavailable(struct nfs4_deviceid_node *node)
{
	return nfs4_test_deviceid_unavailable(node);
}

static inline int
nfs4_ff_layout_ds_version(struct pnfs_layout_segment *lseg, u32 ds_idx)
{
	return FF_LAYOUT_COMP(lseg, ds_idx)->mirror_ds->ds_versions[0].version;
}

struct nfs4_ff_layout_ds *
nfs4_ff_alloc_deviceid_node(struct nfs_server *server, struct pnfs_device *pdev,
			    gfp_t gfp_flags);
void nfs4_ff_layout_put_deviceid(struct nfs4_ff_layout_ds *mirror_ds);
void nfs4_ff_layout_free_deviceid(struct nfs4_ff_layout_ds *mirror_ds);
int ff_layout_track_ds_error(struct nfs4_flexfile_layout *flo,
			     struct nfs4_ff_layout_mirror *mirror, u64 offset,
			     u64 length, int status, enum nfs_opnum4 opnum,
			     gfp_t gfp_flags);
int ff_layout_encode_ds_ioerr(struct nfs4_flexfile_layout *flo,
			      struct xdr_stream *xdr, int *count,
			      const struct pnfs_layout_range *range);
struct nfs_fh *
nfs4_ff_layout_select_ds_fh(struct pnfs_layout_segment *lseg, u32 mirror_idx);

struct nfs4_pnfs_ds *
nfs4_ff_layout_prepare_ds(struct pnfs_layout_segment *lseg, u32 ds_idx,
			  bool fail_return);

struct rpc_clnt *
nfs4_ff_find_or_create_ds_client(struct pnfs_layout_segment *lseg,
				 u32 ds_idx,
				 struct nfs_client *ds_clp,
				 struct inode *inode);
struct rpc_cred *ff_layout_get_ds_cred(struct pnfs_layout_segment *lseg,
				       u32 ds_idx, struct rpc_cred *mdscred);
bool ff_layout_has_available_ds(struct pnfs_layout_segment *lseg);
#endif /* FS_NFS_NFS4FLEXFILELAYOUT_H */
