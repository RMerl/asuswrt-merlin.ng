/*
 * Device operations for the pnfs nfs4 file layout driver.
 *
 * Copyright (c) 2014, Primary Data, Inc. All rights reserved.
 *
 * Tao Peng <bergwolf@primarydata.com>
 */

#include <linux/nfs_fs.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/sunrpc/addr.h>

#include "../internal.h"
#include "../nfs4session.h"
#include "flexfilelayout.h"

#define NFSDBG_FACILITY		NFSDBG_PNFS_LD

static unsigned int dataserver_timeo = NFS4_DEF_DS_TIMEO;
static unsigned int dataserver_retrans = NFS4_DEF_DS_RETRANS;

void nfs4_ff_layout_put_deviceid(struct nfs4_ff_layout_ds *mirror_ds)
{
	if (mirror_ds)
		nfs4_put_deviceid_node(&mirror_ds->id_node);
}

void nfs4_ff_layout_free_deviceid(struct nfs4_ff_layout_ds *mirror_ds)
{
	nfs4_print_deviceid(&mirror_ds->id_node.deviceid);
	nfs4_pnfs_ds_put(mirror_ds->ds);
	kfree(mirror_ds->ds_versions);
	kfree_rcu(mirror_ds, id_node.rcu);
}

/* Decode opaque device data and construct new_ds using it */
struct nfs4_ff_layout_ds *
nfs4_ff_alloc_deviceid_node(struct nfs_server *server, struct pnfs_device *pdev,
			    gfp_t gfp_flags)
{
	struct xdr_stream stream;
	struct xdr_buf buf;
	struct page *scratch;
	struct list_head dsaddrs;
	struct nfs4_pnfs_ds_addr *da;
	struct nfs4_ff_layout_ds *new_ds = NULL;
	struct nfs4_ff_ds_version *ds_versions = NULL;
	u32 mp_count;
	u32 version_count;
	__be32 *p;
	int i, ret = -ENOMEM;

	/* set up xdr stream */
	scratch = alloc_page(gfp_flags);
	if (!scratch)
		goto out_err;

	new_ds = kzalloc(sizeof(struct nfs4_ff_layout_ds), gfp_flags);
	if (!new_ds)
		goto out_scratch;

	nfs4_init_deviceid_node(&new_ds->id_node,
				server,
				&pdev->dev_id);
	INIT_LIST_HEAD(&dsaddrs);

	xdr_init_decode_pages(&stream, &buf, pdev->pages, pdev->pglen);
	xdr_set_scratch_buffer(&stream, page_address(scratch), PAGE_SIZE);

	/* multipath count */
	p = xdr_inline_decode(&stream, 4);
	if (unlikely(!p))
		goto out_err_drain_dsaddrs;
	mp_count = be32_to_cpup(p);
	dprintk("%s: multipath ds count %d\n", __func__, mp_count);

	for (i = 0; i < mp_count; i++) {
		/* multipath ds */
		da = nfs4_decode_mp_ds_addr(server->nfs_client->cl_net,
					    &stream, gfp_flags);
		if (da)
			list_add_tail(&da->da_node, &dsaddrs);
	}
	if (list_empty(&dsaddrs)) {
		dprintk("%s: no suitable DS addresses found\n",
			__func__);
		ret = -ENOMEDIUM;
		goto out_err_drain_dsaddrs;
	}

	/* version count */
	p = xdr_inline_decode(&stream, 4);
	if (unlikely(!p))
		goto out_err_drain_dsaddrs;
	version_count = be32_to_cpup(p);
	dprintk("%s: version count %d\n", __func__, version_count);

	ds_versions = kzalloc(version_count * sizeof(struct nfs4_ff_ds_version),
			      gfp_flags);
	if (!ds_versions)
		goto out_scratch;

	for (i = 0; i < version_count; i++) {
		/* 20 = version(4) + minor_version(4) + rsize(4) + wsize(4) +
		 * tightly_coupled(4) */
		p = xdr_inline_decode(&stream, 20);
		if (unlikely(!p))
			goto out_err_drain_dsaddrs;
		ds_versions[i].version = be32_to_cpup(p++);
		ds_versions[i].minor_version = be32_to_cpup(p++);
		ds_versions[i].rsize = nfs_block_size(be32_to_cpup(p++), NULL);
		ds_versions[i].wsize = nfs_block_size(be32_to_cpup(p++), NULL);
		ds_versions[i].tightly_coupled = be32_to_cpup(p);

		if (ds_versions[i].rsize > NFS_MAX_FILE_IO_SIZE)
			ds_versions[i].rsize = NFS_MAX_FILE_IO_SIZE;
		if (ds_versions[i].wsize > NFS_MAX_FILE_IO_SIZE)
			ds_versions[i].wsize = NFS_MAX_FILE_IO_SIZE;

		if (ds_versions[i].version != 3 || ds_versions[i].minor_version != 0) {
			dprintk("%s: [%d] unsupported ds version %d-%d\n", __func__,
				i, ds_versions[i].version,
				ds_versions[i].minor_version);
			ret = -EPROTONOSUPPORT;
			goto out_err_drain_dsaddrs;
		}

		dprintk("%s: [%d] vers %u minor_ver %u rsize %u wsize %u coupled %d\n",
			__func__, i, ds_versions[i].version,
			ds_versions[i].minor_version,
			ds_versions[i].rsize,
			ds_versions[i].wsize,
			ds_versions[i].tightly_coupled);
	}

	new_ds->ds_versions = ds_versions;
	new_ds->ds_versions_cnt = version_count;

	new_ds->ds = nfs4_pnfs_ds_add(&dsaddrs, gfp_flags);
	if (!new_ds->ds)
		goto out_err_drain_dsaddrs;

	/* If DS was already in cache, free ds addrs */
	while (!list_empty(&dsaddrs)) {
		da = list_first_entry(&dsaddrs,
				      struct nfs4_pnfs_ds_addr,
				      da_node);
		list_del_init(&da->da_node);
		kfree(da->da_remotestr);
		kfree(da);
	}

	__free_page(scratch);
	return new_ds;

out_err_drain_dsaddrs:
	while (!list_empty(&dsaddrs)) {
		da = list_first_entry(&dsaddrs, struct nfs4_pnfs_ds_addr,
				      da_node);
		list_del_init(&da->da_node);
		kfree(da->da_remotestr);
		kfree(da);
	}

	kfree(ds_versions);
out_scratch:
	__free_page(scratch);
out_err:
	kfree(new_ds);

	dprintk("%s ERROR: returning %d\n", __func__, ret);
	return NULL;
}

static u64
end_offset(u64 start, u64 len)
{
	u64 end;

	end = start + len;
	return end >= start ? end : NFS4_MAX_UINT64;
}

static void extend_ds_error(struct nfs4_ff_layout_ds_err *err,
			    u64 offset, u64 length)
{
	u64 end;

	end = max_t(u64, end_offset(err->offset, err->length),
		    end_offset(offset, length));
	err->offset = min_t(u64, err->offset, offset);
	err->length = end - err->offset;
}

static bool ds_error_can_merge(struct nfs4_ff_layout_ds_err *err,  u64 offset,
			       u64 length, int status, enum nfs_opnum4 opnum,
			       nfs4_stateid *stateid,
			       struct nfs4_deviceid *deviceid)
{
	return err->status == status && err->opnum == opnum &&
	       nfs4_stateid_match(&err->stateid, stateid) &&
	       !memcmp(&err->deviceid, deviceid, sizeof(*deviceid)) &&
	       end_offset(err->offset, err->length) >= offset &&
	       err->offset <= end_offset(offset, length);
}

static bool merge_ds_error(struct nfs4_ff_layout_ds_err *old,
			   struct nfs4_ff_layout_ds_err *new)
{
	if (!ds_error_can_merge(old, new->offset, new->length, new->status,
				new->opnum, &new->stateid, &new->deviceid))
		return false;

	extend_ds_error(old, new->offset, new->length);
	return true;
}

static bool
ff_layout_add_ds_error_locked(struct nfs4_flexfile_layout *flo,
			      struct nfs4_ff_layout_ds_err *dserr)
{
	struct nfs4_ff_layout_ds_err *err;

	list_for_each_entry(err, &flo->error_list, list) {
		if (merge_ds_error(err, dserr)) {
			return true;
		}
	}

	list_add(&dserr->list, &flo->error_list);
	return false;
}

static bool
ff_layout_update_ds_error(struct nfs4_flexfile_layout *flo, u64 offset,
			  u64 length, int status, enum nfs_opnum4 opnum,
			  nfs4_stateid *stateid, struct nfs4_deviceid *deviceid)
{
	bool found = false;
	struct nfs4_ff_layout_ds_err *err;

	list_for_each_entry(err, &flo->error_list, list) {
		if (ds_error_can_merge(err, offset, length, status, opnum,
				       stateid, deviceid)) {
			found = true;
			extend_ds_error(err, offset, length);
			break;
		}
	}

	return found;
}

int ff_layout_track_ds_error(struct nfs4_flexfile_layout *flo,
			     struct nfs4_ff_layout_mirror *mirror, u64 offset,
			     u64 length, int status, enum nfs_opnum4 opnum,
			     gfp_t gfp_flags)
{
	struct nfs4_ff_layout_ds_err *dserr;
	bool needfree;

	if (status == 0)
		return 0;

	if (mirror->mirror_ds == NULL)
		return -EINVAL;

	spin_lock(&flo->generic_hdr.plh_inode->i_lock);
	if (ff_layout_update_ds_error(flo, offset, length, status, opnum,
				      &mirror->stateid,
				      &mirror->mirror_ds->id_node.deviceid)) {
		spin_unlock(&flo->generic_hdr.plh_inode->i_lock);
		return 0;
	}
	spin_unlock(&flo->generic_hdr.plh_inode->i_lock);
	dserr = kmalloc(sizeof(*dserr), gfp_flags);
	if (!dserr)
		return -ENOMEM;

	INIT_LIST_HEAD(&dserr->list);
	dserr->offset = offset;
	dserr->length = length;
	dserr->status = status;
	dserr->opnum = opnum;
	nfs4_stateid_copy(&dserr->stateid, &mirror->stateid);
	memcpy(&dserr->deviceid, &mirror->mirror_ds->id_node.deviceid,
	       NFS4_DEVICEID4_SIZE);

	spin_lock(&flo->generic_hdr.plh_inode->i_lock);
	needfree = ff_layout_add_ds_error_locked(flo, dserr);
	spin_unlock(&flo->generic_hdr.plh_inode->i_lock);
	if (needfree)
		kfree(dserr);

	return 0;
}

/* currently we only support AUTH_NONE and AUTH_SYS */
static rpc_authflavor_t
nfs4_ff_layout_choose_authflavor(struct nfs4_ff_layout_mirror *mirror)
{
	if (mirror->uid == (u32)-1)
		return RPC_AUTH_NULL;
	return RPC_AUTH_UNIX;
}

/* fetch cred for NFSv3 DS */
static int ff_layout_update_mirror_cred(struct nfs4_ff_layout_mirror *mirror,
				      struct nfs4_pnfs_ds *ds)
{
	if (ds->ds_clp && !mirror->cred &&
	    mirror->mirror_ds->ds_versions[0].version == 3) {
		struct rpc_auth *auth = ds->ds_clp->cl_rpcclient->cl_auth;
		struct rpc_cred *cred;
		struct auth_cred acred = {
			.uid = make_kuid(&init_user_ns, mirror->uid),
			.gid = make_kgid(&init_user_ns, mirror->gid),
		};

		/* AUTH_NULL ignores acred */
		cred = auth->au_ops->lookup_cred(auth, &acred, 0);
		if (IS_ERR(cred)) {
			dprintk("%s: lookup_cred failed with %ld\n",
				__func__, PTR_ERR(cred));
			return PTR_ERR(cred);
		} else {
			if (cmpxchg(&mirror->cred, NULL, cred))
				put_rpccred(cred);
		}
	}
	return 0;
}

struct nfs_fh *
nfs4_ff_layout_select_ds_fh(struct pnfs_layout_segment *lseg, u32 mirror_idx)
{
	struct nfs4_ff_layout_mirror *mirror = FF_LAYOUT_COMP(lseg, mirror_idx);
	struct nfs_fh *fh = NULL;
	struct nfs4_deviceid_node *devid;

	if (mirror == NULL || mirror->mirror_ds == NULL ||
	    mirror->mirror_ds->ds == NULL) {
		printk(KERN_ERR "NFS: %s: No data server for mirror offset index %d\n",
			__func__, mirror_idx);
		if (mirror && mirror->mirror_ds) {
			devid = &mirror->mirror_ds->id_node;
			pnfs_generic_mark_devid_invalid(devid);
		}
		goto out;
	}

	/* FIXME: For now assume there is only 1 version available for the DS */
	fh = &mirror->fh_versions[0];
out:
	return fh;
}

/* Upon return, either ds is connected, or ds is NULL */
struct nfs4_pnfs_ds *
nfs4_ff_layout_prepare_ds(struct pnfs_layout_segment *lseg, u32 ds_idx,
			  bool fail_return)
{
	struct nfs4_ff_layout_mirror *mirror = FF_LAYOUT_COMP(lseg, ds_idx);
	struct nfs4_pnfs_ds *ds = NULL;
	struct nfs4_deviceid_node *devid;
	struct inode *ino = lseg->pls_layout->plh_inode;
	struct nfs_server *s = NFS_SERVER(ino);
	unsigned int max_payload;
	rpc_authflavor_t flavor;

	if (mirror == NULL || mirror->mirror_ds == NULL ||
	    mirror->mirror_ds->ds == NULL) {
		printk(KERN_ERR "NFS: %s: No data server for offset index %d\n",
			__func__, ds_idx);
		if (mirror && mirror->mirror_ds) {
			devid = &mirror->mirror_ds->id_node;
			pnfs_generic_mark_devid_invalid(devid);
		}
		goto out;
	}

	devid = &mirror->mirror_ds->id_node;
	if (ff_layout_test_devid_unavailable(devid))
		goto out;

	ds = mirror->mirror_ds->ds;
	/* matching smp_wmb() in _nfs4_pnfs_v3/4_ds_connect */
	smp_rmb();
	if (ds->ds_clp)
		goto out_update_creds;

	flavor = nfs4_ff_layout_choose_authflavor(mirror);

	/* FIXME: For now we assume the server sent only one version of NFS
	 * to use for the DS.
	 */
	nfs4_pnfs_ds_connect(s, ds, devid, dataserver_timeo,
			     dataserver_retrans,
			     mirror->mirror_ds->ds_versions[0].version,
			     mirror->mirror_ds->ds_versions[0].minor_version,
			     flavor);

	/* connect success, check rsize/wsize limit */
	if (ds->ds_clp) {
		max_payload =
			nfs_block_size(rpc_max_payload(ds->ds_clp->cl_rpcclient),
				       NULL);
		if (mirror->mirror_ds->ds_versions[0].rsize > max_payload)
			mirror->mirror_ds->ds_versions[0].rsize = max_payload;
		if (mirror->mirror_ds->ds_versions[0].wsize > max_payload)
			mirror->mirror_ds->ds_versions[0].wsize = max_payload;
	} else {
		ff_layout_track_ds_error(FF_LAYOUT_FROM_HDR(lseg->pls_layout),
					 mirror, lseg->pls_range.offset,
					 lseg->pls_range.length, NFS4ERR_NXIO,
					 OP_ILLEGAL, GFP_NOIO);
		if (fail_return) {
			pnfs_error_mark_layout_for_return(ino, lseg);
			if (ff_layout_has_available_ds(lseg))
				pnfs_set_retry_layoutget(lseg->pls_layout);
			else
				pnfs_clear_retry_layoutget(lseg->pls_layout);

		} else {
			if (ff_layout_has_available_ds(lseg))
				set_bit(NFS_LAYOUT_RETURN_BEFORE_CLOSE,
					&lseg->pls_layout->plh_flags);
			else {
				pnfs_error_mark_layout_for_return(ino, lseg);
				pnfs_clear_retry_layoutget(lseg->pls_layout);
			}
		}
	}
out_update_creds:
	if (ff_layout_update_mirror_cred(mirror, ds))
		ds = NULL;
out:
	return ds;
}

struct rpc_cred *
ff_layout_get_ds_cred(struct pnfs_layout_segment *lseg, u32 ds_idx,
		      struct rpc_cred *mdscred)
{
	struct nfs4_ff_layout_mirror *mirror = FF_LAYOUT_COMP(lseg, ds_idx);
	struct rpc_cred *cred = ERR_PTR(-EINVAL);

	if (!nfs4_ff_layout_prepare_ds(lseg, ds_idx, true))
		goto out;

	if (mirror && mirror->cred)
		cred = mirror->cred;
	else
		cred = mdscred;
out:
	return cred;
}

/**
* Find or create a DS rpc client with th MDS server rpc client auth flavor
* in the nfs_client cl_ds_clients list.
*/
struct rpc_clnt *
nfs4_ff_find_or_create_ds_client(struct pnfs_layout_segment *lseg, u32 ds_idx,
				 struct nfs_client *ds_clp, struct inode *inode)
{
	struct nfs4_ff_layout_mirror *mirror = FF_LAYOUT_COMP(lseg, ds_idx);

	switch (mirror->mirror_ds->ds_versions[0].version) {
	case 3:
		/* For NFSv3 DS, flavor is set when creating DS connections */
		return ds_clp->cl_rpcclient;
	case 4:
		return nfs4_find_or_create_ds_client(ds_clp, inode);
	default:
		BUG();
	}
}

static bool is_range_intersecting(u64 offset1, u64 length1,
				  u64 offset2, u64 length2)
{
	u64 end1 = end_offset(offset1, length1);
	u64 end2 = end_offset(offset2, length2);

	return (end1 == NFS4_MAX_UINT64 || end1 > offset2) &&
	       (end2 == NFS4_MAX_UINT64 || end2 > offset1);
}

/* called with inode i_lock held */
int ff_layout_encode_ds_ioerr(struct nfs4_flexfile_layout *flo,
			      struct xdr_stream *xdr, int *count,
			      const struct pnfs_layout_range *range)
{
	struct nfs4_ff_layout_ds_err *err, *n;
	__be32 *p;

	list_for_each_entry_safe(err, n, &flo->error_list, list) {
		if (!is_range_intersecting(err->offset, err->length,
					   range->offset, range->length))
			continue;
		/* offset(8) + length(8) + stateid(NFS4_STATEID_SIZE)
		 * + array length + deviceid(NFS4_DEVICEID4_SIZE)
		 * + status(4) + opnum(4)
		 */
		p = xdr_reserve_space(xdr,
				28 + NFS4_STATEID_SIZE + NFS4_DEVICEID4_SIZE);
		if (unlikely(!p))
			return -ENOBUFS;
		p = xdr_encode_hyper(p, err->offset);
		p = xdr_encode_hyper(p, err->length);
		p = xdr_encode_opaque_fixed(p, &err->stateid,
					    NFS4_STATEID_SIZE);
		/* Encode 1 error */
		*p++ = cpu_to_be32(1);
		p = xdr_encode_opaque_fixed(p, &err->deviceid,
					    NFS4_DEVICEID4_SIZE);
		*p++ = cpu_to_be32(err->status);
		*p++ = cpu_to_be32(err->opnum);
		*count += 1;
		list_del(&err->list);
		dprintk("%s: offset %llu length %llu status %d op %d count %d\n",
			__func__, err->offset, err->length, err->status,
			err->opnum, *count);
		kfree(err);
	}

	return 0;
}

bool ff_layout_has_available_ds(struct pnfs_layout_segment *lseg)
{
	struct nfs4_ff_layout_mirror *mirror;
	struct nfs4_deviceid_node *devid;
	int idx;

	for (idx = 0; idx < FF_LAYOUT_MIRROR_COUNT(lseg); idx++) {
		mirror = FF_LAYOUT_COMP(lseg, idx);
		if (mirror && mirror->mirror_ds) {
			devid = &mirror->mirror_ds->id_node;
			if (!ff_layout_test_devid_unavailable(devid))
				return true;
		}
	}

	return false;
}

module_param(dataserver_retrans, uint, 0644);
MODULE_PARM_DESC(dataserver_retrans, "The  number of times the NFSv4.1 client "
			"retries a request before it attempts further "
			" recovery  action.");
module_param(dataserver_timeo, uint, 0644);
MODULE_PARM_DESC(dataserver_timeo, "The time (in tenths of a second) the "
			"NFSv4.1  client  waits for a response from a "
			" data server before it retries an NFS request.");
