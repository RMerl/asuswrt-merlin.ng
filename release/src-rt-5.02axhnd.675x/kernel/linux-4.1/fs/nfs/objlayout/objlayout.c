/*
 *  pNFS Objects layout driver high level definitions
 *
 *  Copyright (C) 2007 Panasas Inc. [year of first publication]
 *  All rights reserved.
 *
 *  Benny Halevy <bhalevy@panasas.com>
 *  Boaz Harrosh <ooo@electrozaur.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  See the file COPYING included with this distribution for more details.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the Panasas company nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kmod.h>
#include <linux/moduleparam.h>
#include <linux/ratelimit.h>
#include <scsi/osd_initiator.h>
#include "objlayout.h"

#define NFSDBG_FACILITY         NFSDBG_PNFS_LD
/*
 * Create a objlayout layout structure for the given inode and return it.
 */
struct pnfs_layout_hdr *
objlayout_alloc_layout_hdr(struct inode *inode, gfp_t gfp_flags)
{
	struct objlayout *objlay;

	objlay = kzalloc(sizeof(struct objlayout), gfp_flags);
	if (!objlay)
		return NULL;
	spin_lock_init(&objlay->lock);
	INIT_LIST_HEAD(&objlay->err_list);
	dprintk("%s: Return %p\n", __func__, objlay);
	return &objlay->pnfs_layout;
}

/*
 * Free an objlayout layout structure
 */
void
objlayout_free_layout_hdr(struct pnfs_layout_hdr *lo)
{
	struct objlayout *objlay = OBJLAYOUT(lo);

	dprintk("%s: objlay %p\n", __func__, objlay);

	WARN_ON(!list_empty(&objlay->err_list));
	kfree(objlay);
}

/*
 * Unmarshall layout and store it in pnfslay.
 */
struct pnfs_layout_segment *
objlayout_alloc_lseg(struct pnfs_layout_hdr *pnfslay,
		     struct nfs4_layoutget_res *lgr,
		     gfp_t gfp_flags)
{
	int status = -ENOMEM;
	struct xdr_stream stream;
	struct xdr_buf buf = {
		.pages =  lgr->layoutp->pages,
		.page_len =  lgr->layoutp->len,
		.buflen =  lgr->layoutp->len,
		.len = lgr->layoutp->len,
	};
	struct page *scratch;
	struct pnfs_layout_segment *lseg;

	dprintk("%s: Begin pnfslay %p\n", __func__, pnfslay);

	scratch = alloc_page(gfp_flags);
	if (!scratch)
		goto err_nofree;

	xdr_init_decode(&stream, &buf, NULL);
	xdr_set_scratch_buffer(&stream, page_address(scratch), PAGE_SIZE);

	status = objio_alloc_lseg(&lseg, pnfslay, &lgr->range, &stream, gfp_flags);
	if (unlikely(status)) {
		dprintk("%s: objio_alloc_lseg Return err %d\n", __func__,
			status);
		goto err;
	}

	__free_page(scratch);

	dprintk("%s: Return %p\n", __func__, lseg);
	return lseg;

err:
	__free_page(scratch);
err_nofree:
	dprintk("%s: Err Return=>%d\n", __func__, status);
	return ERR_PTR(status);
}

/*
 * Free a layout segement
 */
void
objlayout_free_lseg(struct pnfs_layout_segment *lseg)
{
	dprintk("%s: freeing layout segment %p\n", __func__, lseg);

	if (unlikely(!lseg))
		return;

	objio_free_lseg(lseg);
}

/*
 * I/O Operations
 */
static inline u64
end_offset(u64 start, u64 len)
{
	u64 end;

	end = start + len;
	return end >= start ? end : NFS4_MAX_UINT64;
}

static void _fix_verify_io_params(struct pnfs_layout_segment *lseg,
			   struct page ***p_pages, unsigned *p_pgbase,
			   u64 offset, unsigned long count)
{
	u64 lseg_end_offset;

	BUG_ON(offset < lseg->pls_range.offset);
	lseg_end_offset = end_offset(lseg->pls_range.offset,
				     lseg->pls_range.length);
	BUG_ON(offset >= lseg_end_offset);
	WARN_ON(offset + count > lseg_end_offset);

	if (*p_pgbase > PAGE_SIZE) {
		dprintk("%s: pgbase(0x%x) > PAGE_SIZE\n", __func__, *p_pgbase);
		*p_pages += *p_pgbase >> PAGE_SHIFT;
		*p_pgbase &= ~PAGE_MASK;
	}
}

/*
 * I/O done common code
 */
static void
objlayout_iodone(struct objlayout_io_res *oir)
{
	if (likely(oir->status >= 0)) {
		objio_free_result(oir);
	} else {
		struct objlayout *objlay = oir->objlay;

		spin_lock(&objlay->lock);
		objlay->delta_space_valid = OBJ_DSU_INVALID;
		list_add(&objlay->err_list, &oir->err_list);
		spin_unlock(&objlay->lock);
	}
}

/*
 * objlayout_io_set_result - Set an osd_error code on a specific osd comp.
 *
 * The @index component IO failed (error returned from target). Register
 * the error for later reporting at layout-return.
 */
void
objlayout_io_set_result(struct objlayout_io_res *oir, unsigned index,
			struct pnfs_osd_objid *pooid, int osd_error,
			u64 offset, u64 length, bool is_write)
{
	struct pnfs_osd_ioerr *ioerr = &oir->ioerrs[index];

	BUG_ON(index >= oir->num_comps);
	if (osd_error) {
		ioerr->oer_component = *pooid;
		ioerr->oer_comp_offset = offset;
		ioerr->oer_comp_length = length;
		ioerr->oer_iswrite = is_write;
		ioerr->oer_errno = osd_error;

		dprintk("%s: err[%d]: errno=%d is_write=%d dev(%llx:%llx) "
			"par=0x%llx obj=0x%llx offset=0x%llx length=0x%llx\n",
			__func__, index, ioerr->oer_errno,
			ioerr->oer_iswrite,
			_DEVID_LO(&ioerr->oer_component.oid_device_id),
			_DEVID_HI(&ioerr->oer_component.oid_device_id),
			ioerr->oer_component.oid_partition_id,
			ioerr->oer_component.oid_object_id,
			ioerr->oer_comp_offset,
			ioerr->oer_comp_length);
	} else {
		/* User need not call if no error is reported */
		ioerr->oer_errno = 0;
	}
}

/* Function scheduled on rpc workqueue to call ->nfs_readlist_complete().
 * This is because the osd completion is called with ints-off from
 * the block layer
 */
static void _rpc_read_complete(struct work_struct *work)
{
	struct rpc_task *task;
	struct nfs_pgio_header *hdr;

	dprintk("%s enter\n", __func__);
	task = container_of(work, struct rpc_task, u.tk_work);
	hdr = container_of(task, struct nfs_pgio_header, task);

	pnfs_ld_read_done(hdr);
}

void
objlayout_read_done(struct objlayout_io_res *oir, ssize_t status, bool sync)
{
	struct nfs_pgio_header *hdr = oir->rpcdata;

	oir->status = hdr->task.tk_status = status;
	if (status >= 0)
		hdr->res.count = status;
	else
		hdr->pnfs_error = status;
	objlayout_iodone(oir);
	/* must not use oir after this point */

	dprintk("%s: Return status=%zd eof=%d sync=%d\n", __func__,
		status, hdr->res.eof, sync);

	if (sync)
		pnfs_ld_read_done(hdr);
	else {
		INIT_WORK(&hdr->task.u.tk_work, _rpc_read_complete);
		schedule_work(&hdr->task.u.tk_work);
	}
}

/*
 * Perform sync or async reads.
 */
enum pnfs_try_status
objlayout_read_pagelist(struct nfs_pgio_header *hdr)
{
	struct inode *inode = hdr->inode;
	loff_t offset = hdr->args.offset;
	size_t count = hdr->args.count;
	int err;
	loff_t eof;

	eof = i_size_read(inode);
	if (unlikely(offset + count > eof)) {
		if (offset >= eof) {
			err = 0;
			hdr->res.count = 0;
			hdr->res.eof = 1;
			/*FIXME: do we need to call pnfs_ld_read_done() */
			goto out;
		}
		count = eof - offset;
	}

	hdr->res.eof = (offset + count) >= eof;
	_fix_verify_io_params(hdr->lseg, &hdr->args.pages,
			      &hdr->args.pgbase,
			      hdr->args.offset, hdr->args.count);

	dprintk("%s: inode(%lx) offset 0x%llx count 0x%Zx eof=%d\n",
		__func__, inode->i_ino, offset, count, hdr->res.eof);

	err = objio_read_pagelist(hdr);
 out:
	if (unlikely(err)) {
		hdr->pnfs_error = err;
		dprintk("%s: Returned Error %d\n", __func__, err);
		return PNFS_NOT_ATTEMPTED;
	}
	return PNFS_ATTEMPTED;
}

/* Function scheduled on rpc workqueue to call ->nfs_writelist_complete().
 * This is because the osd completion is called with ints-off from
 * the block layer
 */
static void _rpc_write_complete(struct work_struct *work)
{
	struct rpc_task *task;
	struct nfs_pgio_header *hdr;

	dprintk("%s enter\n", __func__);
	task = container_of(work, struct rpc_task, u.tk_work);
	hdr = container_of(task, struct nfs_pgio_header, task);

	pnfs_ld_write_done(hdr);
}

void
objlayout_write_done(struct objlayout_io_res *oir, ssize_t status, bool sync)
{
	struct nfs_pgio_header *hdr = oir->rpcdata;

	oir->status = hdr->task.tk_status = status;
	if (status >= 0) {
		hdr->res.count = status;
		hdr->verf.committed = oir->committed;
	} else {
		hdr->pnfs_error = status;
	}
	objlayout_iodone(oir);
	/* must not use oir after this point */

	dprintk("%s: Return status %zd committed %d sync=%d\n", __func__,
		status, hdr->verf.committed, sync);

	if (sync)
		pnfs_ld_write_done(hdr);
	else {
		INIT_WORK(&hdr->task.u.tk_work, _rpc_write_complete);
		schedule_work(&hdr->task.u.tk_work);
	}
}

/*
 * Perform sync or async writes.
 */
enum pnfs_try_status
objlayout_write_pagelist(struct nfs_pgio_header *hdr, int how)
{
	int err;

	_fix_verify_io_params(hdr->lseg, &hdr->args.pages,
			      &hdr->args.pgbase,
			      hdr->args.offset, hdr->args.count);

	err = objio_write_pagelist(hdr, how);
	if (unlikely(err)) {
		hdr->pnfs_error = err;
		dprintk("%s: Returned Error %d\n", __func__, err);
		return PNFS_NOT_ATTEMPTED;
	}
	return PNFS_ATTEMPTED;
}

void
objlayout_encode_layoutcommit(struct pnfs_layout_hdr *pnfslay,
			      struct xdr_stream *xdr,
			      const struct nfs4_layoutcommit_args *args)
{
	struct objlayout *objlay = OBJLAYOUT(pnfslay);
	struct pnfs_osd_layoutupdate lou;
	__be32 *start;

	dprintk("%s: Begin\n", __func__);

	spin_lock(&objlay->lock);
	lou.dsu_valid = (objlay->delta_space_valid == OBJ_DSU_VALID);
	lou.dsu_delta = objlay->delta_space_used;
	objlay->delta_space_used = 0;
	objlay->delta_space_valid = OBJ_DSU_INIT;
	lou.olu_ioerr_flag = !list_empty(&objlay->err_list);
	spin_unlock(&objlay->lock);

	start = xdr_reserve_space(xdr, 4);

	BUG_ON(pnfs_osd_xdr_encode_layoutupdate(xdr, &lou));

	*start = cpu_to_be32((xdr->p - start - 1) * 4);

	dprintk("%s: Return delta_space_used %lld err %d\n", __func__,
		lou.dsu_delta, lou.olu_ioerr_flag);
}

static int
err_prio(u32 oer_errno)
{
	switch (oer_errno) {
	case 0:
		return 0;

	case PNFS_OSD_ERR_RESOURCE:
		return OSD_ERR_PRI_RESOURCE;
	case PNFS_OSD_ERR_BAD_CRED:
		return OSD_ERR_PRI_BAD_CRED;
	case PNFS_OSD_ERR_NO_ACCESS:
		return OSD_ERR_PRI_NO_ACCESS;
	case PNFS_OSD_ERR_UNREACHABLE:
		return OSD_ERR_PRI_UNREACHABLE;
	case PNFS_OSD_ERR_NOT_FOUND:
		return OSD_ERR_PRI_NOT_FOUND;
	case PNFS_OSD_ERR_NO_SPACE:
		return OSD_ERR_PRI_NO_SPACE;
	default:
		WARN_ON(1);
		/* fallthrough */
	case PNFS_OSD_ERR_EIO:
		return OSD_ERR_PRI_EIO;
	}
}

static void
merge_ioerr(struct pnfs_osd_ioerr *dest_err,
	    const struct pnfs_osd_ioerr *src_err)
{
	u64 dest_end, src_end;

	if (!dest_err->oer_errno) {
		*dest_err = *src_err;
		/* accumulated device must be blank */
		memset(&dest_err->oer_component.oid_device_id, 0,
			sizeof(dest_err->oer_component.oid_device_id));

		return;
	}

	if (dest_err->oer_component.oid_partition_id !=
				src_err->oer_component.oid_partition_id)
		dest_err->oer_component.oid_partition_id = 0;

	if (dest_err->oer_component.oid_object_id !=
				src_err->oer_component.oid_object_id)
		dest_err->oer_component.oid_object_id = 0;

	if (dest_err->oer_comp_offset > src_err->oer_comp_offset)
		dest_err->oer_comp_offset = src_err->oer_comp_offset;

	dest_end = end_offset(dest_err->oer_comp_offset,
			      dest_err->oer_comp_length);
	src_end =  end_offset(src_err->oer_comp_offset,
			      src_err->oer_comp_length);
	if (dest_end < src_end)
		dest_end = src_end;

	dest_err->oer_comp_length = dest_end - dest_err->oer_comp_offset;

	if ((src_err->oer_iswrite == dest_err->oer_iswrite) &&
	    (err_prio(src_err->oer_errno) > err_prio(dest_err->oer_errno))) {
			dest_err->oer_errno = src_err->oer_errno;
	} else if (src_err->oer_iswrite) {
		dest_err->oer_iswrite = true;
		dest_err->oer_errno = src_err->oer_errno;
	}
}

static void
encode_accumulated_error(struct objlayout *objlay, __be32 *p)
{
	struct objlayout_io_res *oir, *tmp;
	struct pnfs_osd_ioerr accumulated_err = {.oer_errno = 0};

	list_for_each_entry_safe(oir, tmp, &objlay->err_list, err_list) {
		unsigned i;

		for (i = 0; i < oir->num_comps; i++) {
			struct pnfs_osd_ioerr *ioerr = &oir->ioerrs[i];

			if (!ioerr->oer_errno)
				continue;

			printk(KERN_ERR "NFS: %s: err[%d]: errno=%d "
				"is_write=%d dev(%llx:%llx) par=0x%llx "
				"obj=0x%llx offset=0x%llx length=0x%llx\n",
				__func__, i, ioerr->oer_errno,
				ioerr->oer_iswrite,
				_DEVID_LO(&ioerr->oer_component.oid_device_id),
				_DEVID_HI(&ioerr->oer_component.oid_device_id),
				ioerr->oer_component.oid_partition_id,
				ioerr->oer_component.oid_object_id,
				ioerr->oer_comp_offset,
				ioerr->oer_comp_length);

			merge_ioerr(&accumulated_err, ioerr);
		}
		list_del(&oir->err_list);
		objio_free_result(oir);
	}

	pnfs_osd_xdr_encode_ioerr(p, &accumulated_err);
}

void
objlayout_encode_layoutreturn(struct pnfs_layout_hdr *pnfslay,
			      struct xdr_stream *xdr,
			      const struct nfs4_layoutreturn_args *args)
{
	struct objlayout *objlay = OBJLAYOUT(pnfslay);
	struct objlayout_io_res *oir, *tmp;
	__be32 *start;

	dprintk("%s: Begin\n", __func__);
	start = xdr_reserve_space(xdr, 4);
	BUG_ON(!start);

	spin_lock(&objlay->lock);

	list_for_each_entry_safe(oir, tmp, &objlay->err_list, err_list) {
		__be32 *last_xdr = NULL, *p;
		unsigned i;
		int res = 0;

		for (i = 0; i < oir->num_comps; i++) {
			struct pnfs_osd_ioerr *ioerr = &oir->ioerrs[i];

			if (!ioerr->oer_errno)
				continue;

			dprintk("%s: err[%d]: errno=%d is_write=%d "
				"dev(%llx:%llx) par=0x%llx obj=0x%llx "
				"offset=0x%llx length=0x%llx\n",
				__func__, i, ioerr->oer_errno,
				ioerr->oer_iswrite,
				_DEVID_LO(&ioerr->oer_component.oid_device_id),
				_DEVID_HI(&ioerr->oer_component.oid_device_id),
				ioerr->oer_component.oid_partition_id,
				ioerr->oer_component.oid_object_id,
				ioerr->oer_comp_offset,
				ioerr->oer_comp_length);

			p = pnfs_osd_xdr_ioerr_reserve_space(xdr);
			if (unlikely(!p)) {
				res = -E2BIG;
				break; /* accumulated_error */
			}

			last_xdr = p;
			pnfs_osd_xdr_encode_ioerr(p, &oir->ioerrs[i]);
		}

		/* TODO: use xdr_write_pages */
		if (unlikely(res)) {
			/* no space for even one error descriptor */
			BUG_ON(!last_xdr);

			/* we've encountered a situation with lots and lots of
			 * errors and no space to encode them all. Use the last
			 * available slot to report the union of all the
			 * remaining errors.
			 */
			encode_accumulated_error(objlay, last_xdr);
			goto loop_done;
		}
		list_del(&oir->err_list);
		objio_free_result(oir);
	}
loop_done:
	spin_unlock(&objlay->lock);

	*start = cpu_to_be32((xdr->p - start - 1) * 4);
	dprintk("%s: Return\n", __func__);
}

enum {
	OBJLAYOUT_MAX_URI_LEN = 256, OBJLAYOUT_MAX_OSDNAME_LEN = 64,
	OBJLAYOUT_MAX_SYSID_HEX_LEN = OSD_SYSTEMID_LEN * 2 + 1,
	OSD_LOGIN_UPCALL_PATHLEN  = 256
};

static char osd_login_prog[OSD_LOGIN_UPCALL_PATHLEN] = "/sbin/osd_login";

module_param_string(osd_login_prog, osd_login_prog, sizeof(osd_login_prog),
		    0600);
MODULE_PARM_DESC(osd_login_prog, "Path to the osd_login upcall program");

struct __auto_login {
	char uri[OBJLAYOUT_MAX_URI_LEN];
	char osdname[OBJLAYOUT_MAX_OSDNAME_LEN];
	char systemid_hex[OBJLAYOUT_MAX_SYSID_HEX_LEN];
};

static int __objlayout_upcall(struct __auto_login *login)
{
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/sbin:/bin:/usr/bin",
		NULL
	};
	char *argv[8];
	int ret;

	if (unlikely(!osd_login_prog[0])) {
		dprintk("%s: osd_login_prog is disabled\n", __func__);
		return -EACCES;
	}

	dprintk("%s uri: %s\n", __func__, login->uri);
	dprintk("%s osdname %s\n", __func__, login->osdname);
	dprintk("%s systemid_hex %s\n", __func__, login->systemid_hex);

	argv[0] = (char *)osd_login_prog;
	argv[1] = "-u";
	argv[2] = login->uri;
	argv[3] = "-o";
	argv[4] = login->osdname;
	argv[5] = "-s";
	argv[6] = login->systemid_hex;
	argv[7] = NULL;

	ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	/*
	 * Disable the upcall mechanism if we're getting an ENOENT or
	 * EACCES error. The admin can re-enable it on the fly by using
	 * sysfs to set the objlayoutdriver.osd_login_prog module parameter once
	 * the problem has been fixed.
	 */
	if (ret == -ENOENT || ret == -EACCES) {
		printk(KERN_ERR "PNFS-OBJ: %s was not found please set "
			"objlayoutdriver.osd_login_prog kernel parameter!\n",
			osd_login_prog);
		osd_login_prog[0] = '\0';
	}
	dprintk("%s %s return value: %d\n", __func__, osd_login_prog, ret);

	return ret;
}

/* Assume dest is all zeros */
static void __copy_nfsS_and_zero_terminate(struct nfs4_string s,
					   char *dest, int max_len,
					   const char *var_name)
{
	if (!s.len)
		return;

	if (s.len >= max_len) {
		pr_warn_ratelimited(
			"objlayout_autologin: %s: s.len(%d) >= max_len(%d)",
			var_name, s.len, max_len);
		s.len = max_len - 1; /* space for null terminator */
	}

	memcpy(dest, s.data, s.len);
}

/* Assume sysid is all zeros */
static void _sysid_2_hex(struct nfs4_string s,
		  char sysid[OBJLAYOUT_MAX_SYSID_HEX_LEN])
{
	int i;
	char *cur;

	if (!s.len)
		return;

	if (s.len != OSD_SYSTEMID_LEN) {
		pr_warn_ratelimited(
		    "objlayout_autologin: systemid_len(%d) != OSD_SYSTEMID_LEN",
		    s.len);
		if (s.len > OSD_SYSTEMID_LEN)
			s.len = OSD_SYSTEMID_LEN;
	}

	cur = sysid;
	for (i = 0; i < s.len; i++)
		cur = hex_byte_pack(cur, s.data[i]);
}

int objlayout_autologin(struct pnfs_osd_deviceaddr *deviceaddr)
{
	int rc;
	struct __auto_login login;

	if (!deviceaddr->oda_targetaddr.ota_netaddr.r_addr.len)
		return -ENODEV;

	memset(&login, 0, sizeof(login));
	__copy_nfsS_and_zero_terminate(
		deviceaddr->oda_targetaddr.ota_netaddr.r_addr,
		login.uri, sizeof(login.uri), "URI");

	__copy_nfsS_and_zero_terminate(
		deviceaddr->oda_osdname,
		login.osdname, sizeof(login.osdname), "OSDNAME");

	_sysid_2_hex(deviceaddr->oda_systemid, login.systemid_hex);

	rc = __objlayout_upcall(&login);
	if (rc > 0) /* script returns positive values */
		rc = -ENODEV;

	return rc;
}
