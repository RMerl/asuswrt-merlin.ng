/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2011, 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * Implementation of cl_io for LOV layer.
 *
 *   Author: Nikita Danilov <nikita.danilov@sun.com>
 *   Author: Jinshan Xiong <jinshan.xiong@whamcloud.com>
 */

#define DEBUG_SUBSYSTEM S_LOV

#include "lov_cl_internal.h"

/** \addtogroup lov
 *  @{
 */

static inline void lov_sub_enter(struct lov_io_sub *sub)
{
	sub->sub_reenter++;
}
static inline void lov_sub_exit(struct lov_io_sub *sub)
{
	sub->sub_reenter--;
}

static void lov_io_sub_fini(const struct lu_env *env, struct lov_io *lio,
			    struct lov_io_sub *sub)
{
	if (sub->sub_io != NULL) {
		if (sub->sub_io_initialized) {
			lov_sub_enter(sub);
			cl_io_fini(sub->sub_env, sub->sub_io);
			lov_sub_exit(sub);
			sub->sub_io_initialized = 0;
			lio->lis_active_subios--;
		}
		if (sub->sub_stripe == lio->lis_single_subio_index)
			lio->lis_single_subio_index = -1;
		else if (!sub->sub_borrowed)
			OBD_FREE_PTR(sub->sub_io);
		sub->sub_io = NULL;
	}
	if (sub->sub_env != NULL && !IS_ERR(sub->sub_env)) {
		if (!sub->sub_borrowed)
			cl_env_put(sub->sub_env, &sub->sub_refcheck);
		sub->sub_env = NULL;
	}
}

static void lov_io_sub_inherit(struct cl_io *io, struct lov_io *lio,
			       int stripe, loff_t start, loff_t end)
{
	struct lov_stripe_md *lsm    = lio->lis_object->lo_lsm;
	struct cl_io	 *parent = lio->lis_cl.cis_io;

	switch (io->ci_type) {
	case CIT_SETATTR: {
		io->u.ci_setattr.sa_attr = parent->u.ci_setattr.sa_attr;
		io->u.ci_setattr.sa_valid = parent->u.ci_setattr.sa_valid;
		io->u.ci_setattr.sa_capa = parent->u.ci_setattr.sa_capa;
		if (cl_io_is_trunc(io)) {
			loff_t new_size = parent->u.ci_setattr.sa_attr.lvb_size;

			new_size = lov_size_to_stripe(lsm, new_size, stripe);
			io->u.ci_setattr.sa_attr.lvb_size = new_size;
		}
		break;
	}
	case CIT_FAULT: {
		struct cl_object *obj = parent->ci_obj;
		loff_t off = cl_offset(obj, parent->u.ci_fault.ft_index);

		io->u.ci_fault = parent->u.ci_fault;
		off = lov_size_to_stripe(lsm, off, stripe);
		io->u.ci_fault.ft_index = cl_index(obj, off);
		break;
	}
	case CIT_FSYNC: {
		io->u.ci_fsync.fi_start = start;
		io->u.ci_fsync.fi_end = end;
		io->u.ci_fsync.fi_capa = parent->u.ci_fsync.fi_capa;
		io->u.ci_fsync.fi_fid = parent->u.ci_fsync.fi_fid;
		io->u.ci_fsync.fi_mode = parent->u.ci_fsync.fi_mode;
		break;
	}
	case CIT_READ:
	case CIT_WRITE: {
		io->u.ci_wr.wr_sync = cl_io_is_sync_write(parent);
		if (cl_io_is_append(parent)) {
			io->u.ci_wr.wr_append = 1;
		} else {
			io->u.ci_rw.crw_pos = start;
			io->u.ci_rw.crw_count = end - start;
		}
		break;
	}
	default:
		break;
	}
}

static int lov_io_sub_init(const struct lu_env *env, struct lov_io *lio,
			   struct lov_io_sub *sub)
{
	struct lov_object *lov = lio->lis_object;
	struct lov_device *ld  = lu2lov_dev(lov2cl(lov)->co_lu.lo_dev);
	struct cl_io      *sub_io;
	struct cl_object  *sub_obj;
	struct cl_io      *io  = lio->lis_cl.cis_io;

	int stripe = sub->sub_stripe;
	int result;

	LASSERT(sub->sub_io == NULL);
	LASSERT(sub->sub_env == NULL);
	LASSERT(sub->sub_stripe < lio->lis_stripe_count);

	if (unlikely(lov_r0(lov)->lo_sub[stripe] == NULL))
		return -EIO;

	result = 0;
	sub->sub_io_initialized = 0;
	sub->sub_borrowed = 0;

	if (lio->lis_mem_frozen) {
		LASSERT(mutex_is_locked(&ld->ld_mutex));
		sub->sub_io  = &ld->ld_emrg[stripe]->emrg_subio;
		sub->sub_env = ld->ld_emrg[stripe]->emrg_env;
		sub->sub_borrowed = 1;
	} else {
		void *cookie;

		/* obtain new environment */
		cookie = cl_env_reenter();
		sub->sub_env = cl_env_get(&sub->sub_refcheck);
		cl_env_reexit(cookie);
		if (IS_ERR(sub->sub_env))
			result = PTR_ERR(sub->sub_env);

		if (result == 0) {
			/*
			 * First sub-io. Use ->lis_single_subio to
			 * avoid dynamic allocation.
			 */
			if (lio->lis_active_subios == 0) {
				sub->sub_io = &lio->lis_single_subio;
				lio->lis_single_subio_index = stripe;
			} else {
				OBD_ALLOC_PTR(sub->sub_io);
				if (sub->sub_io == NULL)
					result = -ENOMEM;
			}
		}
	}

	if (result == 0) {
		sub_obj = lovsub2cl(lov_r0(lov)->lo_sub[stripe]);
		sub_io  = sub->sub_io;

		sub_io->ci_obj    = sub_obj;
		sub_io->ci_result = 0;

		sub_io->ci_parent  = io;
		sub_io->ci_lockreq = io->ci_lockreq;
		sub_io->ci_type    = io->ci_type;
		sub_io->ci_no_srvlock = io->ci_no_srvlock;
		sub_io->ci_noatime = io->ci_noatime;

		lov_sub_enter(sub);
		result = cl_io_sub_init(sub->sub_env, sub_io,
					io->ci_type, sub_obj);
		lov_sub_exit(sub);
		if (result >= 0) {
			lio->lis_active_subios++;
			sub->sub_io_initialized = 1;
			result = 0;
		}
	}
	if (result != 0)
		lov_io_sub_fini(env, lio, sub);
	return result;
}

struct lov_io_sub *lov_sub_get(const struct lu_env *env,
			       struct lov_io *lio, int stripe)
{
	int rc;
	struct lov_io_sub *sub = &lio->lis_subs[stripe];

	LASSERT(stripe < lio->lis_stripe_count);

	if (!sub->sub_io_initialized) {
		sub->sub_stripe = stripe;
		rc = lov_io_sub_init(env, lio, sub);
	} else
		rc = 0;
	if (rc == 0)
		lov_sub_enter(sub);
	else
		sub = ERR_PTR(rc);
	return sub;
}

void lov_sub_put(struct lov_io_sub *sub)
{
	lov_sub_exit(sub);
}

/*****************************************************************************
 *
 * Lov io operations.
 *
 */

static int lov_page_stripe(const struct cl_page *page)
{
	struct lovsub_object *subobj;

	subobj = lu2lovsub(
		lu_object_locate(page->cp_child->cp_obj->co_lu.lo_header,
				 &lovsub_device_type));
	LASSERT(subobj != NULL);
	return subobj->lso_index;
}

struct lov_io_sub *lov_page_subio(const struct lu_env *env, struct lov_io *lio,
				  const struct cl_page_slice *slice)
{
	struct lov_stripe_md *lsm  = lio->lis_object->lo_lsm;
	struct cl_page       *page = slice->cpl_page;
	int stripe;

	LASSERT(lio->lis_cl.cis_io != NULL);
	LASSERT(cl2lov(slice->cpl_obj) == lio->lis_object);
	LASSERT(lsm != NULL);
	LASSERT(lio->lis_nr_subios > 0);

	stripe = lov_page_stripe(page);
	return lov_sub_get(env, lio, stripe);
}


static int lov_io_subio_init(const struct lu_env *env, struct lov_io *lio,
			     struct cl_io *io)
{
	struct lov_stripe_md *lsm = lio->lis_object->lo_lsm;
	int result;

	LASSERT(lio->lis_object != NULL);

	/*
	 * Need to be optimized, we can't afford to allocate a piece of memory
	 * when writing a page. -jay
	 */
	OBD_ALLOC_LARGE(lio->lis_subs,
			lsm->lsm_stripe_count * sizeof(lio->lis_subs[0]));
	if (lio->lis_subs != NULL) {
		lio->lis_nr_subios = lio->lis_stripe_count;
		lio->lis_single_subio_index = -1;
		lio->lis_active_subios = 0;
		result = 0;
	} else
		result = -ENOMEM;
	return result;
}

static void lov_io_slice_init(struct lov_io *lio,
			      struct lov_object *obj, struct cl_io *io)
{
	io->ci_result = 0;
	lio->lis_object = obj;

	LASSERT(obj->lo_lsm != NULL);
	lio->lis_stripe_count = obj->lo_lsm->lsm_stripe_count;

	switch (io->ci_type) {
	case CIT_READ:
	case CIT_WRITE:
		lio->lis_pos = io->u.ci_rw.crw_pos;
		lio->lis_endpos = io->u.ci_rw.crw_pos + io->u.ci_rw.crw_count;
		lio->lis_io_endpos = lio->lis_endpos;
		if (cl_io_is_append(io)) {
			LASSERT(io->ci_type == CIT_WRITE);
			lio->lis_pos = 0;
			lio->lis_endpos = OBD_OBJECT_EOF;
		}
		break;

	case CIT_SETATTR:
		if (cl_io_is_trunc(io))
			lio->lis_pos = io->u.ci_setattr.sa_attr.lvb_size;
		else
			lio->lis_pos = 0;
		lio->lis_endpos = OBD_OBJECT_EOF;
		break;

	case CIT_FAULT: {
		pgoff_t index = io->u.ci_fault.ft_index;
		lio->lis_pos = cl_offset(io->ci_obj, index);
		lio->lis_endpos = cl_offset(io->ci_obj, index + 1);
		break;
	}

	case CIT_FSYNC: {
		lio->lis_pos = io->u.ci_fsync.fi_start;
		lio->lis_endpos = io->u.ci_fsync.fi_end;
		break;
	}

	case CIT_MISC:
		lio->lis_pos = 0;
		lio->lis_endpos = OBD_OBJECT_EOF;
		break;

	default:
		LBUG();
	}
}

static void lov_io_fini(const struct lu_env *env, const struct cl_io_slice *ios)
{
	struct lov_io *lio = cl2lov_io(env, ios);
	struct lov_object *lov = cl2lov(ios->cis_obj);
	int i;

	if (lio->lis_subs != NULL) {
		for (i = 0; i < lio->lis_nr_subios; i++)
			lov_io_sub_fini(env, lio, &lio->lis_subs[i]);
		OBD_FREE_LARGE(lio->lis_subs,
			 lio->lis_nr_subios * sizeof(lio->lis_subs[0]));
		lio->lis_nr_subios = 0;
	}

	LASSERT(atomic_read(&lov->lo_active_ios) > 0);
	if (atomic_dec_and_test(&lov->lo_active_ios))
		wake_up_all(&lov->lo_waitq);
}

static u64 lov_offset_mod(u64 val, int delta)
{
	if (val != OBD_OBJECT_EOF)
		val += delta;
	return val;
}

static int lov_io_iter_init(const struct lu_env *env,
			    const struct cl_io_slice *ios)
{
	struct lov_io	*lio = cl2lov_io(env, ios);
	struct lov_stripe_md *lsm = lio->lis_object->lo_lsm;
	struct lov_io_sub    *sub;
	u64 endpos;
	u64 start;
	u64 end;
	int stripe;
	int rc = 0;

	endpos = lov_offset_mod(lio->lis_endpos, -1);
	for (stripe = 0; stripe < lio->lis_stripe_count; stripe++) {
		if (!lov_stripe_intersects(lsm, stripe, lio->lis_pos,
					   endpos, &start, &end))
			continue;

		if (unlikely(lov_r0(lio->lis_object)->lo_sub[stripe] == NULL)) {
			if (ios->cis_io->ci_type == CIT_READ ||
			    ios->cis_io->ci_type == CIT_WRITE ||
			    ios->cis_io->ci_type == CIT_FAULT)
				return -EIO;

			continue;
		}

		end = lov_offset_mod(end, 1);
		sub = lov_sub_get(env, lio, stripe);
		if (!IS_ERR(sub)) {
			lov_io_sub_inherit(sub->sub_io, lio, stripe,
					   start, end);
			rc = cl_io_iter_init(sub->sub_env, sub->sub_io);
			lov_sub_put(sub);
			CDEBUG(D_VFSTRACE, "shrink: %d [%llu, %llu)\n",
			       stripe, start, end);
		} else
			rc = PTR_ERR(sub);

		if (!rc)
			list_add_tail(&sub->sub_linkage, &lio->lis_active);
		else
			break;
	}
	return rc;
}

static int lov_io_rw_iter_init(const struct lu_env *env,
			       const struct cl_io_slice *ios)
{
	struct lov_io	*lio = cl2lov_io(env, ios);
	struct cl_io	 *io  = ios->cis_io;
	struct lov_stripe_md *lsm = lio->lis_object->lo_lsm;
	__u64 start = io->u.ci_rw.crw_pos;
	loff_t next;
	unsigned long ssize = lsm->lsm_stripe_size;

	LASSERT(io->ci_type == CIT_READ || io->ci_type == CIT_WRITE);

	/* fast path for common case. */
	if (lio->lis_nr_subios != 1 && !cl_io_is_append(io)) {

		lov_do_div64(start, ssize);
		next = (start + 1) * ssize;
		if (next <= start * ssize)
			next = ~0ull;

		io->ci_continue = next < lio->lis_io_endpos;
		io->u.ci_rw.crw_count = min_t(loff_t, lio->lis_io_endpos,
					      next) - io->u.ci_rw.crw_pos;
		lio->lis_pos    = io->u.ci_rw.crw_pos;
		lio->lis_endpos = io->u.ci_rw.crw_pos + io->u.ci_rw.crw_count;
		CDEBUG(D_VFSTRACE, "stripe: %llu chunk: [%llu, %llu) %llu\n",
		       (__u64)start, lio->lis_pos, lio->lis_endpos,
		       (__u64)lio->lis_io_endpos);
	}
	/*
	 * XXX The following call should be optimized: we know, that
	 * [lio->lis_pos, lio->lis_endpos) intersects with exactly one stripe.
	 */
	return lov_io_iter_init(env, ios);
}

static int lov_io_call(const struct lu_env *env, struct lov_io *lio,
		       int (*iofunc)(const struct lu_env *, struct cl_io *))
{
	struct cl_io *parent = lio->lis_cl.cis_io;
	struct lov_io_sub *sub;
	int rc = 0;

	list_for_each_entry(sub, &lio->lis_active, sub_linkage) {
		lov_sub_enter(sub);
		rc = iofunc(sub->sub_env, sub->sub_io);
		lov_sub_exit(sub);
		if (rc)
			break;

		if (parent->ci_result == 0)
			parent->ci_result = sub->sub_io->ci_result;
	}
	return rc;
}

static int lov_io_lock(const struct lu_env *env, const struct cl_io_slice *ios)
{
	return lov_io_call(env, cl2lov_io(env, ios), cl_io_lock);
}

static int lov_io_start(const struct lu_env *env, const struct cl_io_slice *ios)
{
	return lov_io_call(env, cl2lov_io(env, ios), cl_io_start);
}

static int lov_io_end_wrapper(const struct lu_env *env, struct cl_io *io)
{
	/*
	 * It's possible that lov_io_start() wasn't called against this
	 * sub-io, either because previous sub-io failed, or upper layer
	 * completed IO.
	 */
	if (io->ci_state == CIS_IO_GOING)
		cl_io_end(env, io);
	else
		io->ci_state = CIS_IO_FINISHED;
	return 0;
}

static int lov_io_iter_fini_wrapper(const struct lu_env *env, struct cl_io *io)
{
	cl_io_iter_fini(env, io);
	return 0;
}

static int lov_io_unlock_wrapper(const struct lu_env *env, struct cl_io *io)
{
	cl_io_unlock(env, io);
	return 0;
}

static void lov_io_end(const struct lu_env *env, const struct cl_io_slice *ios)
{
	int rc;

	rc = lov_io_call(env, cl2lov_io(env, ios), lov_io_end_wrapper);
	LASSERT(rc == 0);
}

static void lov_io_iter_fini(const struct lu_env *env,
			     const struct cl_io_slice *ios)
{
	struct lov_io *lio = cl2lov_io(env, ios);
	int rc;

	rc = lov_io_call(env, lio, lov_io_iter_fini_wrapper);
	LASSERT(rc == 0);
	while (!list_empty(&lio->lis_active))
		list_del_init(lio->lis_active.next);
}

static void lov_io_unlock(const struct lu_env *env,
			  const struct cl_io_slice *ios)
{
	int rc;

	rc = lov_io_call(env, cl2lov_io(env, ios), lov_io_unlock_wrapper);
	LASSERT(rc == 0);
}


static struct cl_page_list *lov_io_submit_qin(struct lov_device *ld,
					      struct cl_page_list *qin,
					      int idx, int alloc)
{
	return alloc ? &qin[idx] : &ld->ld_emrg[idx]->emrg_page_list;
}

/**
 * lov implementation of cl_operations::cio_submit() method. It takes a list
 * of pages in \a queue, splits it into per-stripe sub-lists, invokes
 * cl_io_submit() on underlying devices to submit sub-lists, and then splices
 * everything back.
 *
 * Major complication of this function is a need to handle memory cleansing:
 * cl_io_submit() is called to write out pages as a part of VM memory
 * reclamation, and hence it may not fail due to memory shortages (system
 * dead-locks otherwise). To deal with this, some resources (sub-lists,
 * sub-environment, etc.) are allocated per-device on "startup" (i.e., in a
 * not-memory cleansing context), and in case of memory shortage, these
 * pre-allocated resources are used by lov_io_submit() under
 * lov_device::ld_mutex mutex.
 */
static int lov_io_submit(const struct lu_env *env,
			 const struct cl_io_slice *ios,
			 enum cl_req_type crt, struct cl_2queue *queue)
{
	struct lov_io	  *lio = cl2lov_io(env, ios);
	struct lov_object      *obj = lio->lis_object;
	struct lov_device       *ld = lu2lov_dev(lov2cl(obj)->co_lu.lo_dev);
	struct cl_page_list    *qin = &queue->c2_qin;
	struct cl_2queue      *cl2q = &lov_env_info(env)->lti_cl2q;
	struct cl_page_list *stripes_qin = NULL;
	struct cl_page *page;
	struct cl_page *tmp;
	int stripe;

#define QIN(stripe) lov_io_submit_qin(ld, stripes_qin, stripe, alloc)

	int rc = 0;
	int alloc =
		!(current->flags & PF_MEMALLOC);

	if (lio->lis_active_subios == 1) {
		int idx = lio->lis_single_subio_index;
		struct lov_io_sub *sub;

		LASSERT(idx < lio->lis_nr_subios);
		sub = lov_sub_get(env, lio, idx);
		LASSERT(!IS_ERR(sub));
		LASSERT(sub->sub_io == &lio->lis_single_subio);
		rc = cl_io_submit_rw(sub->sub_env, sub->sub_io,
				     crt, queue);
		lov_sub_put(sub);
		return rc;
	}

	LASSERT(lio->lis_subs != NULL);
	if (alloc) {
		OBD_ALLOC_LARGE(stripes_qin,
				sizeof(*stripes_qin) * lio->lis_nr_subios);
		if (stripes_qin == NULL)
			return -ENOMEM;

		for (stripe = 0; stripe < lio->lis_nr_subios; stripe++)
			cl_page_list_init(&stripes_qin[stripe]);
	} else {
		/*
		 * If we get here, it means pageout & swap doesn't help.
		 * In order to not make things worse, even don't try to
		 * allocate the memory with __GFP_NOWARN. -jay
		 */
		mutex_lock(&ld->ld_mutex);
		lio->lis_mem_frozen = 1;
	}

	cl_2queue_init(cl2q);
	cl_page_list_for_each_safe(page, tmp, qin) {
		stripe = lov_page_stripe(page);
		cl_page_list_move(QIN(stripe), qin, page);
	}

	for (stripe = 0; stripe < lio->lis_nr_subios; stripe++) {
		struct lov_io_sub   *sub;
		struct cl_page_list *sub_qin = QIN(stripe);

		if (list_empty(&sub_qin->pl_pages))
			continue;

		cl_page_list_splice(sub_qin, &cl2q->c2_qin);
		sub = lov_sub_get(env, lio, stripe);
		if (!IS_ERR(sub)) {
			rc = cl_io_submit_rw(sub->sub_env, sub->sub_io,
					     crt, cl2q);
			lov_sub_put(sub);
		} else
			rc = PTR_ERR(sub);
		cl_page_list_splice(&cl2q->c2_qin,  &queue->c2_qin);
		cl_page_list_splice(&cl2q->c2_qout, &queue->c2_qout);
		if (rc != 0)
			break;
	}

	for (stripe = 0; stripe < lio->lis_nr_subios; stripe++) {
		struct cl_page_list *sub_qin = QIN(stripe);

		if (list_empty(&sub_qin->pl_pages))
			continue;

		cl_page_list_splice(sub_qin, qin);
	}

	if (alloc) {
		OBD_FREE_LARGE(stripes_qin,
			 sizeof(*stripes_qin) * lio->lis_nr_subios);
	} else {
		int i;

		for (i = 0; i < lio->lis_nr_subios; i++) {
			struct cl_io *cio = lio->lis_subs[i].sub_io;

			if (cio && cio == &ld->ld_emrg[i]->emrg_subio)
				lov_io_sub_fini(env, lio, &lio->lis_subs[i]);
		}
		lio->lis_mem_frozen = 0;
		mutex_unlock(&ld->ld_mutex);
	}

	return rc;
#undef QIN
}

static int lov_io_prepare_write(const struct lu_env *env,
				const struct cl_io_slice *ios,
				const struct cl_page_slice *slice,
				unsigned from, unsigned to)
{
	struct lov_io     *lio      = cl2lov_io(env, ios);
	struct cl_page    *sub_page = lov_sub_page(slice);
	struct lov_io_sub *sub;
	int result;

	sub = lov_page_subio(env, lio, slice);
	if (!IS_ERR(sub)) {
		result = cl_io_prepare_write(sub->sub_env, sub->sub_io,
					     sub_page, from, to);
		lov_sub_put(sub);
	} else
		result = PTR_ERR(sub);
	return result;
}

static int lov_io_commit_write(const struct lu_env *env,
			       const struct cl_io_slice *ios,
			       const struct cl_page_slice *slice,
			       unsigned from, unsigned to)
{
	struct lov_io     *lio      = cl2lov_io(env, ios);
	struct cl_page    *sub_page = lov_sub_page(slice);
	struct lov_io_sub *sub;
	int result;

	sub = lov_page_subio(env, lio, slice);
	if (!IS_ERR(sub)) {
		result = cl_io_commit_write(sub->sub_env, sub->sub_io,
					    sub_page, from, to);
		lov_sub_put(sub);
	} else
		result = PTR_ERR(sub);
	return result;
}

static int lov_io_fault_start(const struct lu_env *env,
			      const struct cl_io_slice *ios)
{
	struct cl_fault_io *fio;
	struct lov_io      *lio;
	struct lov_io_sub  *sub;

	fio = &ios->cis_io->u.ci_fault;
	lio = cl2lov_io(env, ios);
	sub = lov_sub_get(env, lio, lov_page_stripe(fio->ft_page));
	sub->sub_io->u.ci_fault.ft_nob = fio->ft_nob;
	lov_sub_put(sub);
	return lov_io_start(env, ios);
}

static void lov_io_fsync_end(const struct lu_env *env,
			     const struct cl_io_slice *ios)
{
	struct lov_io *lio = cl2lov_io(env, ios);
	struct lov_io_sub *sub;
	unsigned int *written = &ios->cis_io->u.ci_fsync.fi_nr_written;

	*written = 0;
	list_for_each_entry(sub, &lio->lis_active, sub_linkage) {
		struct cl_io *subio = sub->sub_io;

		lov_sub_enter(sub);
		lov_io_end_wrapper(sub->sub_env, subio);
		lov_sub_exit(sub);

		if (subio->ci_result == 0)
			*written += subio->u.ci_fsync.fi_nr_written;
	}
}

static const struct cl_io_operations lov_io_ops = {
	.op = {
		[CIT_READ] = {
			.cio_fini      = lov_io_fini,
			.cio_iter_init = lov_io_rw_iter_init,
			.cio_iter_fini = lov_io_iter_fini,
			.cio_lock      = lov_io_lock,
			.cio_unlock    = lov_io_unlock,
			.cio_start     = lov_io_start,
			.cio_end       = lov_io_end
		},
		[CIT_WRITE] = {
			.cio_fini      = lov_io_fini,
			.cio_iter_init = lov_io_rw_iter_init,
			.cio_iter_fini = lov_io_iter_fini,
			.cio_lock      = lov_io_lock,
			.cio_unlock    = lov_io_unlock,
			.cio_start     = lov_io_start,
			.cio_end       = lov_io_end
		},
		[CIT_SETATTR] = {
			.cio_fini      = lov_io_fini,
			.cio_iter_init = lov_io_iter_init,
			.cio_iter_fini = lov_io_iter_fini,
			.cio_lock      = lov_io_lock,
			.cio_unlock    = lov_io_unlock,
			.cio_start     = lov_io_start,
			.cio_end       = lov_io_end
		},
		[CIT_FAULT] = {
			.cio_fini      = lov_io_fini,
			.cio_iter_init = lov_io_iter_init,
			.cio_iter_fini = lov_io_iter_fini,
			.cio_lock      = lov_io_lock,
			.cio_unlock    = lov_io_unlock,
			.cio_start     = lov_io_fault_start,
			.cio_end       = lov_io_end
		},
		[CIT_FSYNC] = {
			.cio_fini      = lov_io_fini,
			.cio_iter_init = lov_io_iter_init,
			.cio_iter_fini = lov_io_iter_fini,
			.cio_lock      = lov_io_lock,
			.cio_unlock    = lov_io_unlock,
			.cio_start     = lov_io_start,
			.cio_end       = lov_io_fsync_end
		},
		[CIT_MISC] = {
			.cio_fini   = lov_io_fini
		}
	},
	.req_op = {
		 [CRT_READ] = {
			 .cio_submit    = lov_io_submit
		 },
		 [CRT_WRITE] = {
			 .cio_submit    = lov_io_submit
		 }
	 },
	.cio_prepare_write = lov_io_prepare_write,
	.cio_commit_write  = lov_io_commit_write
};

/*****************************************************************************
 *
 * Empty lov io operations.
 *
 */

static void lov_empty_io_fini(const struct lu_env *env,
			      const struct cl_io_slice *ios)
{
	struct lov_object *lov = cl2lov(ios->cis_obj);

	if (atomic_dec_and_test(&lov->lo_active_ios))
		wake_up_all(&lov->lo_waitq);
}

static void lov_empty_impossible(const struct lu_env *env,
				 struct cl_io_slice *ios)
{
	LBUG();
}

#define LOV_EMPTY_IMPOSSIBLE ((void *)lov_empty_impossible)

/**
 * An io operation vector for files without stripes.
 */
static const struct cl_io_operations lov_empty_io_ops = {
	.op = {
		[CIT_READ] = {
			.cio_fini       = lov_empty_io_fini,
#if 0
			.cio_iter_init  = LOV_EMPTY_IMPOSSIBLE,
			.cio_lock       = LOV_EMPTY_IMPOSSIBLE,
			.cio_start      = LOV_EMPTY_IMPOSSIBLE,
			.cio_end	= LOV_EMPTY_IMPOSSIBLE
#endif
		},
		[CIT_WRITE] = {
			.cio_fini      = lov_empty_io_fini,
			.cio_iter_init = LOV_EMPTY_IMPOSSIBLE,
			.cio_lock      = LOV_EMPTY_IMPOSSIBLE,
			.cio_start     = LOV_EMPTY_IMPOSSIBLE,
			.cio_end       = LOV_EMPTY_IMPOSSIBLE
		},
		[CIT_SETATTR] = {
			.cio_fini      = lov_empty_io_fini,
			.cio_iter_init = LOV_EMPTY_IMPOSSIBLE,
			.cio_lock      = LOV_EMPTY_IMPOSSIBLE,
			.cio_start     = LOV_EMPTY_IMPOSSIBLE,
			.cio_end       = LOV_EMPTY_IMPOSSIBLE
		},
		[CIT_FAULT] = {
			.cio_fini      = lov_empty_io_fini,
			.cio_iter_init = LOV_EMPTY_IMPOSSIBLE,
			.cio_lock      = LOV_EMPTY_IMPOSSIBLE,
			.cio_start     = LOV_EMPTY_IMPOSSIBLE,
			.cio_end       = LOV_EMPTY_IMPOSSIBLE
		},
		[CIT_FSYNC] = {
			.cio_fini   = lov_empty_io_fini
		},
		[CIT_MISC] = {
			.cio_fini   = lov_empty_io_fini
		}
	},
	.req_op = {
		 [CRT_READ] = {
			 .cio_submit    = LOV_EMPTY_IMPOSSIBLE
		 },
		 [CRT_WRITE] = {
			 .cio_submit    = LOV_EMPTY_IMPOSSIBLE
		 }
	 },
	.cio_commit_write = LOV_EMPTY_IMPOSSIBLE
};

int lov_io_init_raid0(const struct lu_env *env, struct cl_object *obj,
		      struct cl_io *io)
{
	struct lov_io       *lio = lov_env_io(env);
	struct lov_object   *lov = cl2lov(obj);

	INIT_LIST_HEAD(&lio->lis_active);
	lov_io_slice_init(lio, lov, io);
	if (io->ci_result == 0) {
		io->ci_result = lov_io_subio_init(env, lio, io);
		if (io->ci_result == 0) {
			cl_io_slice_add(io, &lio->lis_cl, obj, &lov_io_ops);
			atomic_inc(&lov->lo_active_ios);
		}
	}
	return io->ci_result;
}

int lov_io_init_empty(const struct lu_env *env, struct cl_object *obj,
		      struct cl_io *io)
{
	struct lov_object *lov = cl2lov(obj);
	struct lov_io *lio = lov_env_io(env);
	int result;

	lio->lis_object = lov;
	switch (io->ci_type) {
	default:
		LBUG();
	case CIT_MISC:
	case CIT_READ:
		result = 0;
		break;
	case CIT_FSYNC:
	case CIT_SETATTR:
		result = 1;
		break;
	case CIT_WRITE:
		result = -EBADF;
		break;
	case CIT_FAULT:
		result = -EFAULT;
		CERROR("Page fault on a file without stripes: "DFID"\n",
		       PFID(lu_object_fid(&obj->co_lu)));
		break;
	}
	if (result == 0) {
		cl_io_slice_add(io, &lio->lis_cl, obj, &lov_empty_io_ops);
		atomic_inc(&lov->lo_active_ios);
	}

	io->ci_result = result < 0 ? result : 0;
	return result != 0;
}

int lov_io_init_released(const struct lu_env *env, struct cl_object *obj,
			struct cl_io *io)
{
	struct lov_object *lov = cl2lov(obj);
	struct lov_io *lio = lov_env_io(env);
	int result;

	LASSERT(lov->lo_lsm != NULL);
	lio->lis_object = lov;

	switch (io->ci_type) {
	default:
		LASSERTF(0, "invalid type %d\n", io->ci_type);
	case CIT_MISC:
	case CIT_FSYNC:
		result = 1;
		break;
	case CIT_SETATTR:
		/* the truncate to 0 is managed by MDT:
		 * - in open, for open O_TRUNC
		 * - in setattr, for truncate
		 */
		/* the truncate is for size > 0 so triggers a restore */
		if (cl_io_is_trunc(io))
			io->ci_restore_needed = 1;
		result = -ENODATA;
		break;
	case CIT_READ:
	case CIT_WRITE:
	case CIT_FAULT:
		io->ci_restore_needed = 1;
		result = -ENODATA;
		break;
	}
	if (result == 0) {
		cl_io_slice_add(io, &lio->lis_cl, obj, &lov_empty_io_ops);
		atomic_inc(&lov->lo_active_ios);
	}

	io->ci_result = result < 0 ? result : 0;
	return result != 0;
}
/** @} lov */
