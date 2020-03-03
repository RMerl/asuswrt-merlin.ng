/*
 * Xen SCSI backend driver
 *
 * Copyright (c) 2008, FUJITSU Limited
 *
 * Based on the blkback driver code.
 * Adaption to kernel taget core infrastructure taken from vhost/scsi.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation; or, when distributed
 * separately from the Linux kernel or incorporated into other
 * software packages, subject to the following license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this source file (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define pr_fmt(fmt) "xen-pvscsi: " fmt

#include <stdarg.h>

#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/configfs.h>

#include <generated/utsrelease.h>

#include <scsi/scsi.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_tcq.h>

#include <target/target_core_base.h>
#include <target/target_core_fabric.h>
#include <target/target_core_configfs.h>
#include <target/target_core_fabric_configfs.h>

#include <asm/hypervisor.h>

#include <xen/xen.h>
#include <xen/balloon.h>
#include <xen/events.h>
#include <xen/xenbus.h>
#include <xen/grant_table.h>
#include <xen/page.h>

#include <xen/interface/grant_table.h>
#include <xen/interface/io/vscsiif.h>

#define VSCSI_VERSION	"v0.1"
#define VSCSI_NAMELEN	32

struct ids_tuple {
	unsigned int hst;		/* host    */
	unsigned int chn;		/* channel */
	unsigned int tgt;		/* target  */
	unsigned int lun;		/* LUN     */
};

struct v2p_entry {
	struct ids_tuple v;		/* translate from */
	struct scsiback_tpg *tpg;	/* translate to   */
	unsigned int lun;
	struct kref kref;
	struct list_head l;
};

struct vscsibk_info {
	struct xenbus_device *dev;

	domid_t domid;
	unsigned int irq;

	struct vscsiif_back_ring ring;
	int ring_error;

	spinlock_t ring_lock;
	atomic_t nr_unreplied_reqs;

	spinlock_t v2p_lock;
	struct list_head v2p_entry_lists;

	wait_queue_head_t waiting_to_free;
};

/* theoretical maximum of grants for one request */
#define VSCSI_MAX_GRANTS	(SG_ALL + VSCSIIF_SG_TABLESIZE)

/*
 * VSCSI_GRANT_BATCH is the maximum number of grants to be processed in one
 * call to map/unmap grants. Don't choose it too large, as there are arrays
 * with VSCSI_GRANT_BATCH elements allocated on the stack.
 */
#define VSCSI_GRANT_BATCH	16

struct vscsibk_pend {
	uint16_t rqid;

	uint8_t cmnd[VSCSIIF_MAX_COMMAND_SIZE];
	uint8_t cmd_len;

	uint8_t sc_data_direction;
	uint16_t n_sg;		/* real length of SG list */
	uint16_t n_grants;	/* SG pages and potentially SG list */
	uint32_t data_len;
	uint32_t result;

	struct vscsibk_info *info;
	struct v2p_entry *v2p;
	struct scatterlist *sgl;

	uint8_t sense_buffer[VSCSIIF_SENSE_BUFFERSIZE];

	grant_handle_t grant_handles[VSCSI_MAX_GRANTS];
	struct page *pages[VSCSI_MAX_GRANTS];

	struct se_cmd se_cmd;
};

struct scsiback_tmr {
	atomic_t tmr_complete;
	wait_queue_head_t tmr_wait;
};

struct scsiback_nexus {
	/* Pointer to TCM session for I_T Nexus */
	struct se_session *tvn_se_sess;
};

struct scsiback_tport {
	/* SCSI protocol the tport is providing */
	u8 tport_proto_id;
	/* Binary World Wide unique Port Name for pvscsi Target port */
	u64 tport_wwpn;
	/* ASCII formatted WWPN for pvscsi Target port */
	char tport_name[VSCSI_NAMELEN];
	/* Returned by scsiback_make_tport() */
	struct se_wwn tport_wwn;
};

struct scsiback_tpg {
	/* scsiback port target portal group tag for TCM */
	u16 tport_tpgt;
	/* track number of TPG Port/Lun Links wrt explicit I_T Nexus shutdown */
	int tv_tpg_port_count;
	/* xen-pvscsi references to tpg_nexus, protected by tv_tpg_mutex */
	int tv_tpg_fe_count;
	/* list for scsiback_list */
	struct list_head tv_tpg_list;
	/* Used to protect access for tpg_nexus */
	struct mutex tv_tpg_mutex;
	/* Pointer to the TCM pvscsi I_T Nexus for this TPG endpoint */
	struct scsiback_nexus *tpg_nexus;
	/* Pointer back to scsiback_tport */
	struct scsiback_tport *tport;
	/* Returned by scsiback_make_tpg() */
	struct se_portal_group se_tpg;
	/* alias used in xenstore */
	char param_alias[VSCSI_NAMELEN];
	/* list of info structures related to this target portal group */
	struct list_head info_list;
};

#define SCSIBACK_INVALID_HANDLE (~0)

static bool log_print_stat;
module_param(log_print_stat, bool, 0644);

static int scsiback_max_buffer_pages = 1024;
module_param_named(max_buffer_pages, scsiback_max_buffer_pages, int, 0644);
MODULE_PARM_DESC(max_buffer_pages,
"Maximum number of free pages to keep in backend buffer");

static struct kmem_cache *scsiback_cachep;
static DEFINE_SPINLOCK(free_pages_lock);
static int free_pages_num;
static LIST_HEAD(scsiback_free_pages);

/* Global spinlock to protect scsiback TPG list */
static DEFINE_MUTEX(scsiback_mutex);
static LIST_HEAD(scsiback_list);

static const struct target_core_fabric_ops scsiback_ops;

static void scsiback_get(struct vscsibk_info *info)
{
	atomic_inc(&info->nr_unreplied_reqs);
}

static void scsiback_put(struct vscsibk_info *info)
{
	if (atomic_dec_and_test(&info->nr_unreplied_reqs))
		wake_up(&info->waiting_to_free);
}

static void put_free_pages(struct page **page, int num)
{
	unsigned long flags;
	int i = free_pages_num + num, n = num;

	if (num == 0)
		return;
	if (i > scsiback_max_buffer_pages) {
		n = min(num, i - scsiback_max_buffer_pages);
		gnttab_free_pages(n, page + num - n);
		n = num - n;
	}
	spin_lock_irqsave(&free_pages_lock, flags);
	for (i = 0; i < n; i++)
		list_add(&page[i]->lru, &scsiback_free_pages);
	free_pages_num += n;
	spin_unlock_irqrestore(&free_pages_lock, flags);
}

static int get_free_page(struct page **page)
{
	unsigned long flags;

	spin_lock_irqsave(&free_pages_lock, flags);
	if (list_empty(&scsiback_free_pages)) {
		spin_unlock_irqrestore(&free_pages_lock, flags);
		return gnttab_alloc_pages(1, page);
	}
	page[0] = list_first_entry(&scsiback_free_pages, struct page, lru);
	list_del(&page[0]->lru);
	free_pages_num--;
	spin_unlock_irqrestore(&free_pages_lock, flags);
	return 0;
}

static unsigned long vaddr_page(struct page *page)
{
	unsigned long pfn = page_to_pfn(page);

	return (unsigned long)pfn_to_kaddr(pfn);
}

static unsigned long vaddr(struct vscsibk_pend *req, int seg)
{
	return vaddr_page(req->pages[seg]);
}

static void scsiback_print_status(char *sense_buffer, int errors,
					struct vscsibk_pend *pending_req)
{
	struct scsiback_tpg *tpg = pending_req->v2p->tpg;

	pr_err("[%s:%d] cmnd[0]=%02x -> st=%02x msg=%02x host=%02x drv=%02x\n",
	       tpg->tport->tport_name, pending_req->v2p->lun,
	       pending_req->cmnd[0], status_byte(errors), msg_byte(errors),
	       host_byte(errors), driver_byte(errors));
}

static void scsiback_fast_flush_area(struct vscsibk_pend *req)
{
	struct gnttab_unmap_grant_ref unmap[VSCSI_GRANT_BATCH];
	struct page *pages[VSCSI_GRANT_BATCH];
	unsigned int i, invcount = 0;
	grant_handle_t handle;
	int err;

	kfree(req->sgl);
	req->sgl = NULL;
	req->n_sg = 0;

	if (!req->n_grants)
		return;

	for (i = 0; i < req->n_grants; i++) {
		handle = req->grant_handles[i];
		if (handle == SCSIBACK_INVALID_HANDLE)
			continue;
		gnttab_set_unmap_op(&unmap[invcount], vaddr(req, i),
				    GNTMAP_host_map, handle);
		req->grant_handles[i] = SCSIBACK_INVALID_HANDLE;
		pages[invcount] = req->pages[i];
		put_page(pages[invcount]);
		invcount++;
		if (invcount < VSCSI_GRANT_BATCH)
			continue;
		err = gnttab_unmap_refs(unmap, NULL, pages, invcount);
		BUG_ON(err);
		invcount = 0;
	}

	if (invcount) {
		err = gnttab_unmap_refs(unmap, NULL, pages, invcount);
		BUG_ON(err);
	}

	put_free_pages(req->pages, req->n_grants);
	req->n_grants = 0;
}

static void scsiback_free_translation_entry(struct kref *kref)
{
	struct v2p_entry *entry = container_of(kref, struct v2p_entry, kref);
	struct scsiback_tpg *tpg = entry->tpg;

	mutex_lock(&tpg->tv_tpg_mutex);
	tpg->tv_tpg_fe_count--;
	mutex_unlock(&tpg->tv_tpg_mutex);

	kfree(entry);
}

static void scsiback_do_resp_with_sense(char *sense_buffer, int32_t result,
			uint32_t resid, struct vscsibk_pend *pending_req)
{
	struct vscsiif_response *ring_res;
	struct vscsibk_info *info = pending_req->info;
	int notify;
	struct scsi_sense_hdr sshdr;
	unsigned long flags;
	unsigned len;

	spin_lock_irqsave(&info->ring_lock, flags);

	ring_res = RING_GET_RESPONSE(&info->ring, info->ring.rsp_prod_pvt);
	info->ring.rsp_prod_pvt++;

	ring_res->rslt   = result;
	ring_res->rqid   = pending_req->rqid;

	if (sense_buffer != NULL &&
	    scsi_normalize_sense(sense_buffer, VSCSIIF_SENSE_BUFFERSIZE,
				 &sshdr)) {
		len = min_t(unsigned, 8 + sense_buffer[7],
			    VSCSIIF_SENSE_BUFFERSIZE);
		memcpy(ring_res->sense_buffer, sense_buffer, len);
		ring_res->sense_len = len;
	} else {
		ring_res->sense_len = 0;
	}

	ring_res->residual_len = resid;

	RING_PUSH_RESPONSES_AND_CHECK_NOTIFY(&info->ring, notify);
	spin_unlock_irqrestore(&info->ring_lock, flags);

	if (notify)
		notify_remote_via_irq(info->irq);

	if (pending_req->v2p)
		kref_put(&pending_req->v2p->kref,
			 scsiback_free_translation_entry);
}

static void scsiback_cmd_done(struct vscsibk_pend *pending_req)
{
	struct vscsibk_info *info = pending_req->info;
	unsigned char *sense_buffer;
	unsigned int resid;
	int errors;

	sense_buffer = pending_req->sense_buffer;
	resid        = pending_req->se_cmd.residual_count;
	errors       = pending_req->result;

	if (errors && log_print_stat)
		scsiback_print_status(sense_buffer, errors, pending_req);

	scsiback_fast_flush_area(pending_req);
	scsiback_do_resp_with_sense(sense_buffer, errors, resid, pending_req);
	scsiback_put(info);
}

static void scsiback_cmd_exec(struct vscsibk_pend *pending_req)
{
	struct se_cmd *se_cmd = &pending_req->se_cmd;
	struct se_session *sess = pending_req->v2p->tpg->tpg_nexus->tvn_se_sess;
	int rc;

	memset(pending_req->sense_buffer, 0, VSCSIIF_SENSE_BUFFERSIZE);

	memset(se_cmd, 0, sizeof(*se_cmd));

	scsiback_get(pending_req->info);
	rc = target_submit_cmd_map_sgls(se_cmd, sess, pending_req->cmnd,
			pending_req->sense_buffer, pending_req->v2p->lun,
			pending_req->data_len, 0,
			pending_req->sc_data_direction, 0,
			pending_req->sgl, pending_req->n_sg,
			NULL, 0, NULL, 0);
	if (rc < 0) {
		transport_send_check_condition_and_sense(se_cmd,
				TCM_LOGICAL_UNIT_COMMUNICATION_FAILURE, 0);
		transport_generic_free_cmd(se_cmd, 0);
	}
}

static int scsiback_gnttab_data_map_batch(struct gnttab_map_grant_ref *map,
	struct page **pg, grant_handle_t *grant, int cnt)
{
	int err, i;

	if (!cnt)
		return 0;

	err = gnttab_map_refs(map, NULL, pg, cnt);
	BUG_ON(err);
	for (i = 0; i < cnt; i++) {
		if (unlikely(map[i].status != GNTST_okay)) {
			pr_err("invalid buffer -- could not remap it\n");
			map[i].handle = SCSIBACK_INVALID_HANDLE;
			err = -ENOMEM;
		} else {
			get_page(pg[i]);
		}
		grant[i] = map[i].handle;
	}
	return err;
}

static int scsiback_gnttab_data_map_list(struct vscsibk_pend *pending_req,
			struct scsiif_request_segment *seg, struct page **pg,
			grant_handle_t *grant, int cnt, u32 flags)
{
	int mapcount = 0, i, err = 0;
	struct gnttab_map_grant_ref map[VSCSI_GRANT_BATCH];
	struct vscsibk_info *info = pending_req->info;

	for (i = 0; i < cnt; i++) {
		if (get_free_page(pg + mapcount)) {
			put_free_pages(pg, mapcount);
			pr_err("no grant page\n");
			return -ENOMEM;
		}
		gnttab_set_map_op(&map[mapcount], vaddr_page(pg[mapcount]),
				  flags, seg[i].gref, info->domid);
		mapcount++;
		if (mapcount < VSCSI_GRANT_BATCH)
			continue;
		err = scsiback_gnttab_data_map_batch(map, pg, grant, mapcount);
		pg += mapcount;
		grant += mapcount;
		pending_req->n_grants += mapcount;
		if (err)
			return err;
		mapcount = 0;
	}
	err = scsiback_gnttab_data_map_batch(map, pg, grant, mapcount);
	pending_req->n_grants += mapcount;
	return err;
}

static int scsiback_gnttab_data_map(struct vscsiif_request *ring_req,
					struct vscsibk_pend *pending_req)
{
	u32 flags;
	int i, err, n_segs, i_seg = 0;
	struct page **pg;
	struct scsiif_request_segment *seg;
	unsigned long end_seg = 0;
	unsigned int nr_segments = (unsigned int)ring_req->nr_segments;
	unsigned int nr_sgl = 0;
	struct scatterlist *sg;
	grant_handle_t *grant;

	pending_req->n_sg = 0;
	pending_req->n_grants = 0;
	pending_req->data_len = 0;

	nr_segments &= ~VSCSIIF_SG_GRANT;
	if (!nr_segments)
		return 0;

	if (nr_segments > VSCSIIF_SG_TABLESIZE) {
		pr_debug("invalid parameter nr_seg = %d\n",
			ring_req->nr_segments);
		return -EINVAL;
	}

	if (ring_req->nr_segments & VSCSIIF_SG_GRANT) {
		err = scsiback_gnttab_data_map_list(pending_req, ring_req->seg,
			pending_req->pages, pending_req->grant_handles,
			nr_segments, GNTMAP_host_map | GNTMAP_readonly);
		if (err)
			return err;
		nr_sgl = nr_segments;
		nr_segments = 0;
		for (i = 0; i < nr_sgl; i++) {
			n_segs = ring_req->seg[i].length /
				 sizeof(struct scsiif_request_segment);
			if ((unsigned)ring_req->seg[i].offset +
			    (unsigned)ring_req->seg[i].length > PAGE_SIZE ||
			    n_segs * sizeof(struct scsiif_request_segment) !=
			    ring_req->seg[i].length)
				return -EINVAL;
			nr_segments += n_segs;
		}
		if (nr_segments > SG_ALL) {
			pr_debug("invalid nr_seg = %d\n", nr_segments);
			return -EINVAL;
		}
	}

	/* free of (sgl) in fast_flush_area() */
	pending_req->sgl = kmalloc_array(nr_segments,
					sizeof(struct scatterlist), GFP_KERNEL);
	if (!pending_req->sgl)
		return -ENOMEM;

	sg_init_table(pending_req->sgl, nr_segments);
	pending_req->n_sg = nr_segments;

	flags = GNTMAP_host_map;
	if (pending_req->sc_data_direction == DMA_TO_DEVICE)
		flags |= GNTMAP_readonly;

	pg = pending_req->pages + nr_sgl;
	grant = pending_req->grant_handles + nr_sgl;
	if (!nr_sgl) {
		seg = ring_req->seg;
		err = scsiback_gnttab_data_map_list(pending_req, seg,
			pg, grant, nr_segments, flags);
		if (err)
			return err;
	} else {
		for (i = 0; i < nr_sgl; i++) {
			seg = (struct scsiif_request_segment *)(
			      vaddr(pending_req, i) + ring_req->seg[i].offset);
			n_segs = ring_req->seg[i].length /
				 sizeof(struct scsiif_request_segment);
			err = scsiback_gnttab_data_map_list(pending_req, seg,
				pg, grant, n_segs, flags);
			if (err)
				return err;
			pg += n_segs;
			grant += n_segs;
		}
		end_seg = vaddr(pending_req, 0) + ring_req->seg[0].offset;
		seg = (struct scsiif_request_segment *)end_seg;
		end_seg += ring_req->seg[0].length;
		pg = pending_req->pages + nr_sgl;
	}

	for_each_sg(pending_req->sgl, sg, nr_segments, i) {
		sg_set_page(sg, pg[i], seg->length, seg->offset);
		pending_req->data_len += seg->length;
		seg++;
		if (nr_sgl && (unsigned long)seg >= end_seg) {
			i_seg++;
			end_seg = vaddr(pending_req, i_seg) +
				  ring_req->seg[i_seg].offset;
			seg = (struct scsiif_request_segment *)end_seg;
			end_seg += ring_req->seg[i_seg].length;
		}
		if (sg->offset >= PAGE_SIZE ||
		    sg->length > PAGE_SIZE ||
		    sg->offset + sg->length > PAGE_SIZE)
			return -EINVAL;
	}

	return 0;
}

static void scsiback_disconnect(struct vscsibk_info *info)
{
	wait_event(info->waiting_to_free,
		atomic_read(&info->nr_unreplied_reqs) == 0);

	unbind_from_irqhandler(info->irq, info);
	info->irq = 0;
	xenbus_unmap_ring_vfree(info->dev, info->ring.sring);
}

static void scsiback_device_action(struct vscsibk_pend *pending_req,
	enum tcm_tmreq_table act, int tag)
{
	int rc, err = FAILED;
	struct scsiback_tpg *tpg = pending_req->v2p->tpg;
	struct se_cmd *se_cmd = &pending_req->se_cmd;
	struct scsiback_tmr *tmr;

	tmr = kzalloc(sizeof(struct scsiback_tmr), GFP_KERNEL);
	if (!tmr)
		goto out;

	init_waitqueue_head(&tmr->tmr_wait);

	transport_init_se_cmd(se_cmd, tpg->se_tpg.se_tpg_tfo,
		tpg->tpg_nexus->tvn_se_sess, 0, DMA_NONE, TCM_SIMPLE_TAG,
		&pending_req->sense_buffer[0]);

	rc = core_tmr_alloc_req(se_cmd, tmr, act, GFP_KERNEL);
	if (rc < 0)
		goto out;

	se_cmd->se_tmr_req->ref_task_tag = tag;

	if (transport_lookup_tmr_lun(se_cmd, pending_req->v2p->lun) < 0)
		goto out;

	transport_generic_handle_tmr(se_cmd);
	wait_event(tmr->tmr_wait, atomic_read(&tmr->tmr_complete));

	err = (se_cmd->se_tmr_req->response == TMR_FUNCTION_COMPLETE) ?
		SUCCESS : FAILED;

out:
	if (tmr) {
		transport_generic_free_cmd(&pending_req->se_cmd, 1);
		kfree(tmr);
	}

	scsiback_do_resp_with_sense(NULL, err, 0, pending_req);

	kmem_cache_free(scsiback_cachep, pending_req);
}

/*
  Perform virtual to physical translation
*/
static struct v2p_entry *scsiback_do_translation(struct vscsibk_info *info,
			struct ids_tuple *v)
{
	struct v2p_entry *entry;
	struct list_head *head = &(info->v2p_entry_lists);
	unsigned long flags;

	spin_lock_irqsave(&info->v2p_lock, flags);
	list_for_each_entry(entry, head, l) {
		if ((entry->v.chn == v->chn) &&
		    (entry->v.tgt == v->tgt) &&
		    (entry->v.lun == v->lun)) {
			kref_get(&entry->kref);
			goto out;
		}
	}
	entry = NULL;

out:
	spin_unlock_irqrestore(&info->v2p_lock, flags);
	return entry;
}

static int prepare_pending_reqs(struct vscsibk_info *info,
				struct vscsiif_request *ring_req,
				struct vscsibk_pend *pending_req)
{
	struct v2p_entry *v2p;
	struct ids_tuple vir;

	pending_req->rqid       = ring_req->rqid;
	pending_req->info       = info;

	vir.chn = ring_req->channel;
	vir.tgt = ring_req->id;
	vir.lun = ring_req->lun;

	v2p = scsiback_do_translation(info, &vir);
	if (!v2p) {
		pending_req->v2p = NULL;
		pr_debug("the v2p of (chn:%d, tgt:%d, lun:%d) doesn't exist.\n",
			vir.chn, vir.tgt, vir.lun);
		return -ENODEV;
	}
	pending_req->v2p = v2p;

	/* request range check from frontend */
	pending_req->sc_data_direction = ring_req->sc_data_direction;
	if ((pending_req->sc_data_direction != DMA_BIDIRECTIONAL) &&
		(pending_req->sc_data_direction != DMA_TO_DEVICE) &&
		(pending_req->sc_data_direction != DMA_FROM_DEVICE) &&
		(pending_req->sc_data_direction != DMA_NONE)) {
		pr_debug("invalid parameter data_dir = %d\n",
			pending_req->sc_data_direction);
		return -EINVAL;
	}

	pending_req->cmd_len = ring_req->cmd_len;
	if (pending_req->cmd_len > VSCSIIF_MAX_COMMAND_SIZE) {
		pr_debug("invalid parameter cmd_len = %d\n",
			pending_req->cmd_len);
		return -EINVAL;
	}
	memcpy(pending_req->cmnd, ring_req->cmnd, pending_req->cmd_len);

	return 0;
}

static int scsiback_do_cmd_fn(struct vscsibk_info *info)
{
	struct vscsiif_back_ring *ring = &info->ring;
	struct vscsiif_request ring_req;
	struct vscsibk_pend *pending_req;
	RING_IDX rc, rp;
	int err, more_to_do;
	uint32_t result;

	rc = ring->req_cons;
	rp = ring->sring->req_prod;
	rmb();	/* guest system is accessing ring, too */

	if (RING_REQUEST_PROD_OVERFLOW(ring, rp)) {
		rc = ring->rsp_prod_pvt;
		pr_warn("Dom%d provided bogus ring requests (%#x - %#x = %u). Halting ring processing\n",
			   info->domid, rp, rc, rp - rc);
		info->ring_error = 1;
		return 0;
	}

	while ((rc != rp)) {
		if (RING_REQUEST_CONS_OVERFLOW(ring, rc))
			break;
		pending_req = kmem_cache_alloc(scsiback_cachep, GFP_KERNEL);
		if (!pending_req)
			return 1;

		ring_req = *RING_GET_REQUEST(ring, rc);
		ring->req_cons = ++rc;

		err = prepare_pending_reqs(info, &ring_req, pending_req);
		if (err) {
			switch (err) {
			case -ENODEV:
				result = DID_NO_CONNECT;
				break;
			default:
				result = DRIVER_ERROR;
				break;
			}
			scsiback_do_resp_with_sense(NULL, result << 24, 0,
						    pending_req);
			kmem_cache_free(scsiback_cachep, pending_req);
			return 1;
		}

		switch (ring_req.act) {
		case VSCSIIF_ACT_SCSI_CDB:
			if (scsiback_gnttab_data_map(&ring_req, pending_req)) {
				scsiback_fast_flush_area(pending_req);
				scsiback_do_resp_with_sense(NULL,
					DRIVER_ERROR << 24, 0, pending_req);
				kmem_cache_free(scsiback_cachep, pending_req);
			} else {
				scsiback_cmd_exec(pending_req);
			}
			break;
		case VSCSIIF_ACT_SCSI_ABORT:
			scsiback_device_action(pending_req, TMR_ABORT_TASK,
				ring_req.ref_rqid);
			break;
		case VSCSIIF_ACT_SCSI_RESET:
			scsiback_device_action(pending_req, TMR_LUN_RESET, 0);
			break;
		default:
			pr_err_ratelimited("invalid request\n");
			scsiback_do_resp_with_sense(NULL, DRIVER_ERROR << 24,
						    0, pending_req);
			kmem_cache_free(scsiback_cachep, pending_req);
			break;
		}

		/* Yield point for this unbounded loop. */
		cond_resched();
	}

	RING_FINAL_CHECK_FOR_REQUESTS(&info->ring, more_to_do);
	return more_to_do;
}

static irqreturn_t scsiback_irq_fn(int irq, void *dev_id)
{
	struct vscsibk_info *info = dev_id;

	if (info->ring_error)
		return IRQ_HANDLED;

	while (scsiback_do_cmd_fn(info))
		cond_resched();

	return IRQ_HANDLED;
}

static int scsiback_init_sring(struct vscsibk_info *info, grant_ref_t ring_ref,
			evtchn_port_t evtchn)
{
	void *area;
	struct vscsiif_sring *sring;
	int err;

	if (info->irq)
		return -1;

	err = xenbus_map_ring_valloc(info->dev, &ring_ref, 1, &area);
	if (err)
		return err;

	sring = (struct vscsiif_sring *)area;
	BACK_RING_INIT(&info->ring, sring, PAGE_SIZE);

	err = bind_interdomain_evtchn_to_irq(info->domid, evtchn);
	if (err < 0)
		goto unmap_page;

	info->irq = err;

	err = request_threaded_irq(info->irq, NULL, scsiback_irq_fn,
				   IRQF_ONESHOT, "vscsiif-backend", info);
	if (err)
		goto free_irq;

	return 0;

free_irq:
	unbind_from_irqhandler(info->irq, info);
	info->irq = 0;
unmap_page:
	xenbus_unmap_ring_vfree(info->dev, area);

	return err;
}

static int scsiback_map(struct vscsibk_info *info)
{
	struct xenbus_device *dev = info->dev;
	unsigned int ring_ref, evtchn;
	int err;

	err = xenbus_gather(XBT_NIL, dev->otherend,
			"ring-ref", "%u", &ring_ref,
			"event-channel", "%u", &evtchn, NULL);
	if (err) {
		xenbus_dev_fatal(dev, err, "reading %s ring", dev->otherend);
		return err;
	}

	return scsiback_init_sring(info, ring_ref, evtchn);
}

/*
  Add a new translation entry
*/
static int scsiback_add_translation_entry(struct vscsibk_info *info,
					  char *phy, struct ids_tuple *v)
{
	int err = 0;
	struct v2p_entry *entry;
	struct v2p_entry *new;
	struct list_head *head = &(info->v2p_entry_lists);
	unsigned long flags;
	char *lunp;
	unsigned int lun;
	struct scsiback_tpg *tpg_entry, *tpg = NULL;
	char *error = "doesn't exist";

	lunp = strrchr(phy, ':');
	if (!lunp) {
		pr_err("illegal format of physical device %s\n", phy);
		return -EINVAL;
	}
	*lunp = 0;
	lunp++;
	if (kstrtouint(lunp, 10, &lun) || lun >= TRANSPORT_MAX_LUNS_PER_TPG) {
		pr_err("lun number not valid: %s\n", lunp);
		return -EINVAL;
	}

	mutex_lock(&scsiback_mutex);
	list_for_each_entry(tpg_entry, &scsiback_list, tv_tpg_list) {
		if (!strcmp(phy, tpg_entry->tport->tport_name) ||
		    !strcmp(phy, tpg_entry->param_alias)) {
			spin_lock(&tpg_entry->se_tpg.tpg_lun_lock);
			if (tpg_entry->se_tpg.tpg_lun_list[lun]->lun_status ==
			    TRANSPORT_LUN_STATUS_ACTIVE) {
				if (!tpg_entry->tpg_nexus)
					error = "nexus undefined";
				else
					tpg = tpg_entry;
			}
			spin_unlock(&tpg_entry->se_tpg.tpg_lun_lock);
			break;
		}
	}
	if (tpg) {
		mutex_lock(&tpg->tv_tpg_mutex);
		tpg->tv_tpg_fe_count++;
		mutex_unlock(&tpg->tv_tpg_mutex);
	}
	mutex_unlock(&scsiback_mutex);

	if (!tpg) {
		pr_err("%s:%d %s\n", phy, lun, error);
		return -ENODEV;
	}

	new = kmalloc(sizeof(struct v2p_entry), GFP_KERNEL);
	if (new == NULL) {
		err = -ENOMEM;
		goto out_free;
	}

	spin_lock_irqsave(&info->v2p_lock, flags);

	/* Check double assignment to identical virtual ID */
	list_for_each_entry(entry, head, l) {
		if ((entry->v.chn == v->chn) &&
		    (entry->v.tgt == v->tgt) &&
		    (entry->v.lun == v->lun)) {
			pr_warn("Virtual ID is already used. Assignment was not performed.\n");
			err = -EEXIST;
			goto out;
		}

	}

	/* Create a new translation entry and add to the list */
	kref_init(&new->kref);
	new->v = *v;
	new->tpg = tpg;
	new->lun = lun;
	list_add_tail(&new->l, head);

out:
	spin_unlock_irqrestore(&info->v2p_lock, flags);

out_free:
	if (err) {
		mutex_lock(&tpg->tv_tpg_mutex);
		tpg->tv_tpg_fe_count--;
		mutex_unlock(&tpg->tv_tpg_mutex);
		kfree(new);
	}

	return err;
}

static void __scsiback_del_translation_entry(struct v2p_entry *entry)
{
	list_del(&entry->l);
	kref_put(&entry->kref, scsiback_free_translation_entry);
}

/*
  Delete the translation entry specfied
*/
static int scsiback_del_translation_entry(struct vscsibk_info *info,
					  struct ids_tuple *v)
{
	struct v2p_entry *entry;
	struct list_head *head = &(info->v2p_entry_lists);
	unsigned long flags;

	spin_lock_irqsave(&info->v2p_lock, flags);
	/* Find out the translation entry specified */
	list_for_each_entry(entry, head, l) {
		if ((entry->v.chn == v->chn) &&
		    (entry->v.tgt == v->tgt) &&
		    (entry->v.lun == v->lun)) {
			goto found;
		}
	}

	spin_unlock_irqrestore(&info->v2p_lock, flags);
	return 1;

found:
	/* Delete the translation entry specfied */
	__scsiback_del_translation_entry(entry);

	spin_unlock_irqrestore(&info->v2p_lock, flags);
	return 0;
}

static void scsiback_do_add_lun(struct vscsibk_info *info, const char *state,
				char *phy, struct ids_tuple *vir, int try)
{
	if (!scsiback_add_translation_entry(info, phy, vir)) {
		if (xenbus_printf(XBT_NIL, info->dev->nodename, state,
				  "%d", XenbusStateInitialised)) {
			pr_err("xenbus_printf error %s\n", state);
			scsiback_del_translation_entry(info, vir);
		}
	} else if (!try) {
		xenbus_printf(XBT_NIL, info->dev->nodename, state,
			      "%d", XenbusStateClosed);
	}
}

static void scsiback_do_del_lun(struct vscsibk_info *info, const char *state,
				struct ids_tuple *vir)
{
	if (!scsiback_del_translation_entry(info, vir)) {
		if (xenbus_printf(XBT_NIL, info->dev->nodename, state,
				  "%d", XenbusStateClosed))
			pr_err("xenbus_printf error %s\n", state);
	}
}

#define VSCSIBACK_OP_ADD_OR_DEL_LUN	1
#define VSCSIBACK_OP_UPDATEDEV_STATE	2

static void scsiback_do_1lun_hotplug(struct vscsibk_info *info, int op,
				     char *ent)
{
	int err;
	struct ids_tuple vir;
	char *val;
	int device_state;
	char phy[VSCSI_NAMELEN];
	char str[64];
	char state[64];
	struct xenbus_device *dev = info->dev;

	/* read status */
	snprintf(state, sizeof(state), "vscsi-devs/%s/state", ent);
	err = xenbus_scanf(XBT_NIL, dev->nodename, state, "%u", &device_state);
	if (XENBUS_EXIST_ERR(err))
		return;

	/* physical SCSI device */
	snprintf(str, sizeof(str), "vscsi-devs/%s/p-dev", ent);
	val = xenbus_read(XBT_NIL, dev->nodename, str, NULL);
	if (IS_ERR(val)) {
		xenbus_printf(XBT_NIL, dev->nodename, state,
			      "%d", XenbusStateClosed);
		return;
	}
	strlcpy(phy, val, VSCSI_NAMELEN);
	kfree(val);

	/* virtual SCSI device */
	snprintf(str, sizeof(str), "vscsi-devs/%s/v-dev", ent);
	err = xenbus_scanf(XBT_NIL, dev->nodename, str, "%u:%u:%u:%u",
			   &vir.hst, &vir.chn, &vir.tgt, &vir.lun);
	if (XENBUS_EXIST_ERR(err)) {
		xenbus_printf(XBT_NIL, dev->nodename, state,
			      "%d", XenbusStateClosed);
		return;
	}

	switch (op) {
	case VSCSIBACK_OP_ADD_OR_DEL_LUN:
		switch (device_state) {
		case XenbusStateInitialising:
			scsiback_do_add_lun(info, state, phy, &vir, 0);
			break;
		case XenbusStateConnected:
			scsiback_do_add_lun(info, state, phy, &vir, 1);
			break;
		case XenbusStateClosing:
			scsiback_do_del_lun(info, state, &vir);
			break;
		default:
			break;
		}
		break;

	case VSCSIBACK_OP_UPDATEDEV_STATE:
		if (device_state == XenbusStateInitialised) {
			/* modify vscsi-devs/dev-x/state */
			if (xenbus_printf(XBT_NIL, dev->nodename, state,
					  "%d", XenbusStateConnected)) {
				pr_err("xenbus_printf error %s\n", str);
				scsiback_del_translation_entry(info, &vir);
				xenbus_printf(XBT_NIL, dev->nodename, state,
					      "%d", XenbusStateClosed);
			}
		}
		break;
	/* When it is necessary, processing is added here. */
	default:
		break;
	}
}

static void scsiback_do_lun_hotplug(struct vscsibk_info *info, int op)
{
	int i;
	char **dir;
	unsigned int ndir = 0;

	dir = xenbus_directory(XBT_NIL, info->dev->nodename, "vscsi-devs",
			       &ndir);
	if (IS_ERR(dir))
		return;

	for (i = 0; i < ndir; i++)
		scsiback_do_1lun_hotplug(info, op, dir[i]);

	kfree(dir);
}

static void scsiback_frontend_changed(struct xenbus_device *dev,
					enum xenbus_state frontend_state)
{
	struct vscsibk_info *info = dev_get_drvdata(&dev->dev);

	switch (frontend_state) {
	case XenbusStateInitialising:
		break;

	case XenbusStateInitialised:
		if (scsiback_map(info))
			break;

		scsiback_do_lun_hotplug(info, VSCSIBACK_OP_ADD_OR_DEL_LUN);
		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateConnected:
		scsiback_do_lun_hotplug(info, VSCSIBACK_OP_UPDATEDEV_STATE);

		if (dev->state == XenbusStateConnected)
			break;

		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateClosing:
		if (info->irq)
			scsiback_disconnect(info);

		xenbus_switch_state(dev, XenbusStateClosing);
		break;

	case XenbusStateClosed:
		xenbus_switch_state(dev, XenbusStateClosed);
		if (xenbus_dev_is_online(dev))
			break;
		/* fall through if not online */
	case XenbusStateUnknown:
		device_unregister(&dev->dev);
		break;

	case XenbusStateReconfiguring:
		scsiback_do_lun_hotplug(info, VSCSIBACK_OP_ADD_OR_DEL_LUN);
		xenbus_switch_state(dev, XenbusStateReconfigured);

		break;

	default:
		xenbus_dev_fatal(dev, -EINVAL, "saw state %d at frontend",
					frontend_state);
		break;
	}
}

/*
  Release the translation entry specfied
*/
static void scsiback_release_translation_entry(struct vscsibk_info *info)
{
	struct v2p_entry *entry, *tmp;
	struct list_head *head = &(info->v2p_entry_lists);
	unsigned long flags;

	spin_lock_irqsave(&info->v2p_lock, flags);

	list_for_each_entry_safe(entry, tmp, head, l)
		__scsiback_del_translation_entry(entry);

	spin_unlock_irqrestore(&info->v2p_lock, flags);
}

static int scsiback_remove(struct xenbus_device *dev)
{
	struct vscsibk_info *info = dev_get_drvdata(&dev->dev);

	if (info->irq)
		scsiback_disconnect(info);

	scsiback_release_translation_entry(info);

	dev_set_drvdata(&dev->dev, NULL);

	return 0;
}

static int scsiback_probe(struct xenbus_device *dev,
			   const struct xenbus_device_id *id)
{
	int err;

	struct vscsibk_info *info = kzalloc(sizeof(struct vscsibk_info),
					    GFP_KERNEL);

	pr_debug("%s %p %d\n", __func__, dev, dev->otherend_id);

	if (!info) {
		xenbus_dev_fatal(dev, -ENOMEM, "allocating backend structure");
		return -ENOMEM;
	}
	info->dev = dev;
	dev_set_drvdata(&dev->dev, info);

	info->domid = dev->otherend_id;
	spin_lock_init(&info->ring_lock);
	info->ring_error = 0;
	atomic_set(&info->nr_unreplied_reqs, 0);
	init_waitqueue_head(&info->waiting_to_free);
	info->dev = dev;
	info->irq = 0;
	INIT_LIST_HEAD(&info->v2p_entry_lists);
	spin_lock_init(&info->v2p_lock);

	err = xenbus_printf(XBT_NIL, dev->nodename, "feature-sg-grant", "%u",
			    SG_ALL);
	if (err)
		xenbus_dev_error(dev, err, "writing feature-sg-grant");

	err = xenbus_switch_state(dev, XenbusStateInitWait);
	if (err)
		goto fail;

	return 0;

fail:
	pr_warn("%s failed\n", __func__);
	scsiback_remove(dev);

	return err;
}

static char *scsiback_dump_proto_id(struct scsiback_tport *tport)
{
	switch (tport->tport_proto_id) {
	case SCSI_PROTOCOL_SAS:
		return "SAS";
	case SCSI_PROTOCOL_FCP:
		return "FCP";
	case SCSI_PROTOCOL_ISCSI:
		return "iSCSI";
	default:
		break;
	}

	return "Unknown";
}

static u8 scsiback_get_fabric_proto_ident(struct se_portal_group *se_tpg)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport = tpg->tport;

	switch (tport->tport_proto_id) {
	case SCSI_PROTOCOL_SAS:
		return sas_get_fabric_proto_ident(se_tpg);
	case SCSI_PROTOCOL_FCP:
		return fc_get_fabric_proto_ident(se_tpg);
	case SCSI_PROTOCOL_ISCSI:
		return iscsi_get_fabric_proto_ident(se_tpg);
	default:
		pr_err("Unknown tport_proto_id: 0x%02x, using SAS emulation\n",
			tport->tport_proto_id);
		break;
	}

	return sas_get_fabric_proto_ident(se_tpg);
}

static char *scsiback_get_fabric_wwn(struct se_portal_group *se_tpg)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport = tpg->tport;

	return &tport->tport_name[0];
}

static u16 scsiback_get_tag(struct se_portal_group *se_tpg)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	return tpg->tport_tpgt;
}

static u32 scsiback_get_default_depth(struct se_portal_group *se_tpg)
{
	return 1;
}

static u32
scsiback_get_pr_transport_id(struct se_portal_group *se_tpg,
			      struct se_node_acl *se_nacl,
			      struct t10_pr_registration *pr_reg,
			      int *format_code,
			      unsigned char *buf)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport = tpg->tport;

	switch (tport->tport_proto_id) {
	case SCSI_PROTOCOL_SAS:
		return sas_get_pr_transport_id(se_tpg, se_nacl, pr_reg,
					format_code, buf);
	case SCSI_PROTOCOL_FCP:
		return fc_get_pr_transport_id(se_tpg, se_nacl, pr_reg,
					format_code, buf);
	case SCSI_PROTOCOL_ISCSI:
		return iscsi_get_pr_transport_id(se_tpg, se_nacl, pr_reg,
					format_code, buf);
	default:
		pr_err("Unknown tport_proto_id: 0x%02x, using SAS emulation\n",
			tport->tport_proto_id);
		break;
	}

	return sas_get_pr_transport_id(se_tpg, se_nacl, pr_reg,
			format_code, buf);
}

static u32
scsiback_get_pr_transport_id_len(struct se_portal_group *se_tpg,
				  struct se_node_acl *se_nacl,
				  struct t10_pr_registration *pr_reg,
				  int *format_code)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport = tpg->tport;

	switch (tport->tport_proto_id) {
	case SCSI_PROTOCOL_SAS:
		return sas_get_pr_transport_id_len(se_tpg, se_nacl, pr_reg,
					format_code);
	case SCSI_PROTOCOL_FCP:
		return fc_get_pr_transport_id_len(se_tpg, se_nacl, pr_reg,
					format_code);
	case SCSI_PROTOCOL_ISCSI:
		return iscsi_get_pr_transport_id_len(se_tpg, se_nacl, pr_reg,
					format_code);
	default:
		pr_err("Unknown tport_proto_id: 0x%02x, using SAS emulation\n",
			tport->tport_proto_id);
		break;
	}

	return sas_get_pr_transport_id_len(se_tpg, se_nacl, pr_reg,
			format_code);
}

static char *
scsiback_parse_pr_out_transport_id(struct se_portal_group *se_tpg,
				    const char *buf,
				    u32 *out_tid_len,
				    char **port_nexus_ptr)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport = tpg->tport;

	switch (tport->tport_proto_id) {
	case SCSI_PROTOCOL_SAS:
		return sas_parse_pr_out_transport_id(se_tpg, buf, out_tid_len,
					port_nexus_ptr);
	case SCSI_PROTOCOL_FCP:
		return fc_parse_pr_out_transport_id(se_tpg, buf, out_tid_len,
					port_nexus_ptr);
	case SCSI_PROTOCOL_ISCSI:
		return iscsi_parse_pr_out_transport_id(se_tpg, buf, out_tid_len,
					port_nexus_ptr);
	default:
		pr_err("Unknown tport_proto_id: 0x%02x, using SAS emulation\n",
			tport->tport_proto_id);
		break;
	}

	return sas_parse_pr_out_transport_id(se_tpg, buf, out_tid_len,
			port_nexus_ptr);
}

static struct se_wwn *
scsiback_make_tport(struct target_fabric_configfs *tf,
		     struct config_group *group,
		     const char *name)
{
	struct scsiback_tport *tport;
	char *ptr;
	u64 wwpn = 0;
	int off = 0;

	tport = kzalloc(sizeof(struct scsiback_tport), GFP_KERNEL);
	if (!tport)
		return ERR_PTR(-ENOMEM);

	tport->tport_wwpn = wwpn;
	/*
	 * Determine the emulated Protocol Identifier and Target Port Name
	 * based on the incoming configfs directory name.
	 */
	ptr = strstr(name, "naa.");
	if (ptr) {
		tport->tport_proto_id = SCSI_PROTOCOL_SAS;
		goto check_len;
	}
	ptr = strstr(name, "fc.");
	if (ptr) {
		tport->tport_proto_id = SCSI_PROTOCOL_FCP;
		off = 3; /* Skip over "fc." */
		goto check_len;
	}
	ptr = strstr(name, "iqn.");
	if (ptr) {
		tport->tport_proto_id = SCSI_PROTOCOL_ISCSI;
		goto check_len;
	}

	pr_err("Unable to locate prefix for emulated Target Port: %s\n", name);
	kfree(tport);
	return ERR_PTR(-EINVAL);

check_len:
	if (strlen(name) >= VSCSI_NAMELEN) {
		pr_err("Emulated %s Address: %s, exceeds max: %d\n", name,
			scsiback_dump_proto_id(tport), VSCSI_NAMELEN);
		kfree(tport);
		return ERR_PTR(-EINVAL);
	}
	snprintf(&tport->tport_name[0], VSCSI_NAMELEN, "%s", &name[off]);

	pr_debug("Allocated emulated Target %s Address: %s\n",
		 scsiback_dump_proto_id(tport), name);

	return &tport->tport_wwn;
}

static void scsiback_drop_tport(struct se_wwn *wwn)
{
	struct scsiback_tport *tport = container_of(wwn,
				struct scsiback_tport, tport_wwn);

	pr_debug("Deallocating emulated Target %s Address: %s\n",
		 scsiback_dump_proto_id(tport), tport->tport_name);

	kfree(tport);
}

static struct se_node_acl *
scsiback_alloc_fabric_acl(struct se_portal_group *se_tpg)
{
	return kzalloc(sizeof(struct se_node_acl), GFP_KERNEL);
}

static void
scsiback_release_fabric_acl(struct se_portal_group *se_tpg,
			     struct se_node_acl *se_nacl)
{
	kfree(se_nacl);
}

static u32 scsiback_tpg_get_inst_index(struct se_portal_group *se_tpg)
{
	return 1;
}

static int scsiback_check_stop_free(struct se_cmd *se_cmd)
{
	/*
	 * Do not release struct se_cmd's containing a valid TMR pointer.
	 * These will be released directly in scsiback_device_action()
	 * with transport_generic_free_cmd().
	 */
	if (se_cmd->se_cmd_flags & SCF_SCSI_TMR_CDB)
		return 0;

	transport_generic_free_cmd(se_cmd, 0);
	return 1;
}

static void scsiback_release_cmd(struct se_cmd *se_cmd)
{
	struct vscsibk_pend *pending_req = container_of(se_cmd,
				struct vscsibk_pend, se_cmd);

	kmem_cache_free(scsiback_cachep, pending_req);
}

static int scsiback_shutdown_session(struct se_session *se_sess)
{
	return 0;
}

static void scsiback_close_session(struct se_session *se_sess)
{
}

static u32 scsiback_sess_get_index(struct se_session *se_sess)
{
	return 0;
}

static int scsiback_write_pending(struct se_cmd *se_cmd)
{
	/* Go ahead and process the write immediately */
	target_execute_cmd(se_cmd);

	return 0;
}

static int scsiback_write_pending_status(struct se_cmd *se_cmd)
{
	return 0;
}

static void scsiback_set_default_node_attrs(struct se_node_acl *nacl)
{
}

static u32 scsiback_get_task_tag(struct se_cmd *se_cmd)
{
	struct vscsibk_pend *pending_req = container_of(se_cmd,
				struct vscsibk_pend, se_cmd);

	return pending_req->rqid;
}

static int scsiback_get_cmd_state(struct se_cmd *se_cmd)
{
	return 0;
}

static int scsiback_queue_data_in(struct se_cmd *se_cmd)
{
	struct vscsibk_pend *pending_req = container_of(se_cmd,
				struct vscsibk_pend, se_cmd);

	pending_req->result = SAM_STAT_GOOD;
	scsiback_cmd_done(pending_req);
	return 0;
}

static int scsiback_queue_status(struct se_cmd *se_cmd)
{
	struct vscsibk_pend *pending_req = container_of(se_cmd,
				struct vscsibk_pend, se_cmd);

	if (se_cmd->sense_buffer &&
	    ((se_cmd->se_cmd_flags & SCF_TRANSPORT_TASK_SENSE) ||
	     (se_cmd->se_cmd_flags & SCF_EMULATED_TASK_SENSE)))
		pending_req->result = (DRIVER_SENSE << 24) |
				      SAM_STAT_CHECK_CONDITION;
	else
		pending_req->result = se_cmd->scsi_status;

	scsiback_cmd_done(pending_req);
	return 0;
}

static void scsiback_queue_tm_rsp(struct se_cmd *se_cmd)
{
	struct se_tmr_req *se_tmr = se_cmd->se_tmr_req;
	struct scsiback_tmr *tmr = se_tmr->fabric_tmr_ptr;

	atomic_set(&tmr->tmr_complete, 1);
	wake_up(&tmr->tmr_wait);
}

static void scsiback_aborted_task(struct se_cmd *se_cmd)
{
}

static ssize_t scsiback_tpg_param_show_alias(struct se_portal_group *se_tpg,
					     char *page)
{
	struct scsiback_tpg *tpg = container_of(se_tpg, struct scsiback_tpg,
						se_tpg);
	ssize_t rb;

	mutex_lock(&tpg->tv_tpg_mutex);
	rb = snprintf(page, PAGE_SIZE, "%s\n", tpg->param_alias);
	mutex_unlock(&tpg->tv_tpg_mutex);

	return rb;
}

static ssize_t scsiback_tpg_param_store_alias(struct se_portal_group *se_tpg,
					      const char *page, size_t count)
{
	struct scsiback_tpg *tpg = container_of(se_tpg, struct scsiback_tpg,
						se_tpg);
	int len;

	if (strlen(page) >= VSCSI_NAMELEN) {
		pr_err("param alias: %s, exceeds max: %d\n", page,
			VSCSI_NAMELEN);
		return -EINVAL;
	}

	mutex_lock(&tpg->tv_tpg_mutex);
	len = snprintf(tpg->param_alias, VSCSI_NAMELEN, "%s", page);
	if (tpg->param_alias[len - 1] == '\n')
		tpg->param_alias[len - 1] = '\0';
	mutex_unlock(&tpg->tv_tpg_mutex);

	return count;
}

TF_TPG_PARAM_ATTR(scsiback, alias, S_IRUGO | S_IWUSR);

static struct configfs_attribute *scsiback_param_attrs[] = {
	&scsiback_tpg_param_alias.attr,
	NULL,
};

static int scsiback_make_nexus(struct scsiback_tpg *tpg,
				const char *name)
{
	struct se_portal_group *se_tpg;
	struct se_session *se_sess;
	struct scsiback_nexus *tv_nexus;

	mutex_lock(&tpg->tv_tpg_mutex);
	if (tpg->tpg_nexus) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		pr_debug("tpg->tpg_nexus already exists\n");
		return -EEXIST;
	}
	se_tpg = &tpg->se_tpg;

	tv_nexus = kzalloc(sizeof(struct scsiback_nexus), GFP_KERNEL);
	if (!tv_nexus) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		return -ENOMEM;
	}
	/*
	 * Initialize the struct se_session pointer
	 */
	tv_nexus->tvn_se_sess = transport_init_session(TARGET_PROT_NORMAL);
	if (IS_ERR(tv_nexus->tvn_se_sess)) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		kfree(tv_nexus);
		return -ENOMEM;
	}
	se_sess = tv_nexus->tvn_se_sess;
	/*
	 * Since we are running in 'demo mode' this call with generate a
	 * struct se_node_acl for the scsiback struct se_portal_group with
	 * the SCSI Initiator port name of the passed configfs group 'name'.
	 */
	tv_nexus->tvn_se_sess->se_node_acl = core_tpg_check_initiator_node_acl(
				se_tpg, (unsigned char *)name);
	if (!tv_nexus->tvn_se_sess->se_node_acl) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		pr_debug("core_tpg_check_initiator_node_acl() failed for %s\n",
			 name);
		goto out;
	}
	/* Now register the TCM pvscsi virtual I_T Nexus as active. */
	transport_register_session(se_tpg, tv_nexus->tvn_se_sess->se_node_acl,
			tv_nexus->tvn_se_sess, tv_nexus);
	tpg->tpg_nexus = tv_nexus;

	mutex_unlock(&tpg->tv_tpg_mutex);
	return 0;

out:
	transport_free_session(se_sess);
	kfree(tv_nexus);
	return -ENOMEM;
}

static int scsiback_drop_nexus(struct scsiback_tpg *tpg)
{
	struct se_session *se_sess;
	struct scsiback_nexus *tv_nexus;

	mutex_lock(&tpg->tv_tpg_mutex);
	tv_nexus = tpg->tpg_nexus;
	if (!tv_nexus) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		return -ENODEV;
	}

	se_sess = tv_nexus->tvn_se_sess;
	if (!se_sess) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		return -ENODEV;
	}

	if (tpg->tv_tpg_port_count != 0) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		pr_err("Unable to remove xen-pvscsi I_T Nexus with active TPG port count: %d\n",
			tpg->tv_tpg_port_count);
		return -EBUSY;
	}

	if (tpg->tv_tpg_fe_count != 0) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		pr_err("Unable to remove xen-pvscsi I_T Nexus with active TPG frontend count: %d\n",
			tpg->tv_tpg_fe_count);
		return -EBUSY;
	}

	pr_debug("Removing I_T Nexus to emulated %s Initiator Port: %s\n",
		scsiback_dump_proto_id(tpg->tport),
		tv_nexus->tvn_se_sess->se_node_acl->initiatorname);

	/*
	 * Release the SCSI I_T Nexus to the emulated xen-pvscsi Target Port
	 */
	transport_deregister_session(tv_nexus->tvn_se_sess);
	tpg->tpg_nexus = NULL;
	mutex_unlock(&tpg->tv_tpg_mutex);

	kfree(tv_nexus);
	return 0;
}

static ssize_t scsiback_tpg_show_nexus(struct se_portal_group *se_tpg,
					char *page)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_nexus *tv_nexus;
	ssize_t ret;

	mutex_lock(&tpg->tv_tpg_mutex);
	tv_nexus = tpg->tpg_nexus;
	if (!tv_nexus) {
		mutex_unlock(&tpg->tv_tpg_mutex);
		return -ENODEV;
	}
	ret = snprintf(page, PAGE_SIZE, "%s\n",
			tv_nexus->tvn_se_sess->se_node_acl->initiatorname);
	mutex_unlock(&tpg->tv_tpg_mutex);

	return ret;
}

static ssize_t scsiback_tpg_store_nexus(struct se_portal_group *se_tpg,
					 const char *page,
					 size_t count)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);
	struct scsiback_tport *tport_wwn = tpg->tport;
	unsigned char i_port[VSCSI_NAMELEN], *ptr, *port_ptr;
	int ret;
	/*
	 * Shutdown the active I_T nexus if 'NULL' is passed.
	 */
	if (!strncmp(page, "NULL", 4)) {
		ret = scsiback_drop_nexus(tpg);
		return (!ret) ? count : ret;
	}
	/*
	 * Otherwise make sure the passed virtual Initiator port WWN matches
	 * the fabric protocol_id set in scsiback_make_tport(), and call
	 * scsiback_make_nexus().
	 */
	if (strlen(page) >= VSCSI_NAMELEN) {
		pr_err("Emulated NAA Sas Address: %s, exceeds max: %d\n",
			page, VSCSI_NAMELEN);
		return -EINVAL;
	}
	snprintf(&i_port[0], VSCSI_NAMELEN, "%s", page);

	ptr = strstr(i_port, "naa.");
	if (ptr) {
		if (tport_wwn->tport_proto_id != SCSI_PROTOCOL_SAS) {
			pr_err("Passed SAS Initiator Port %s does not match target port protoid: %s\n",
				i_port, scsiback_dump_proto_id(tport_wwn));
			return -EINVAL;
		}
		port_ptr = &i_port[0];
		goto check_newline;
	}
	ptr = strstr(i_port, "fc.");
	if (ptr) {
		if (tport_wwn->tport_proto_id != SCSI_PROTOCOL_FCP) {
			pr_err("Passed FCP Initiator Port %s does not match target port protoid: %s\n",
				i_port, scsiback_dump_proto_id(tport_wwn));
			return -EINVAL;
		}
		port_ptr = &i_port[3]; /* Skip over "fc." */
		goto check_newline;
	}
	ptr = strstr(i_port, "iqn.");
	if (ptr) {
		if (tport_wwn->tport_proto_id != SCSI_PROTOCOL_ISCSI) {
			pr_err("Passed iSCSI Initiator Port %s does not match target port protoid: %s\n",
				i_port, scsiback_dump_proto_id(tport_wwn));
			return -EINVAL;
		}
		port_ptr = &i_port[0];
		goto check_newline;
	}
	pr_err("Unable to locate prefix for emulated Initiator Port: %s\n",
		i_port);
	return -EINVAL;
	/*
	 * Clear any trailing newline for the NAA WWN
	 */
check_newline:
	if (i_port[strlen(i_port) - 1] == '\n')
		i_port[strlen(i_port) - 1] = '\0';

	ret = scsiback_make_nexus(tpg, port_ptr);
	if (ret < 0)
		return ret;

	return count;
}

TF_TPG_BASE_ATTR(scsiback, nexus, S_IRUGO | S_IWUSR);

static struct configfs_attribute *scsiback_tpg_attrs[] = {
	&scsiback_tpg_nexus.attr,
	NULL,
};

static ssize_t
scsiback_wwn_show_attr_version(struct target_fabric_configfs *tf,
				char *page)
{
	return sprintf(page, "xen-pvscsi fabric module %s on %s/%s on "
		UTS_RELEASE"\n",
		VSCSI_VERSION, utsname()->sysname, utsname()->machine);
}

TF_WWN_ATTR_RO(scsiback, version);

static struct configfs_attribute *scsiback_wwn_attrs[] = {
	&scsiback_wwn_version.attr,
	NULL,
};

static char *scsiback_get_fabric_name(void)
{
	return "xen-pvscsi";
}

static int scsiback_port_link(struct se_portal_group *se_tpg,
			       struct se_lun *lun)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);

	mutex_lock(&tpg->tv_tpg_mutex);
	tpg->tv_tpg_port_count++;
	mutex_unlock(&tpg->tv_tpg_mutex);

	return 0;
}

static void scsiback_port_unlink(struct se_portal_group *se_tpg,
				  struct se_lun *lun)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);

	mutex_lock(&tpg->tv_tpg_mutex);
	tpg->tv_tpg_port_count--;
	mutex_unlock(&tpg->tv_tpg_mutex);
}

static struct se_portal_group *
scsiback_make_tpg(struct se_wwn *wwn,
		   struct config_group *group,
		   const char *name)
{
	struct scsiback_tport *tport = container_of(wwn,
			struct scsiback_tport, tport_wwn);

	struct scsiback_tpg *tpg;
	u16 tpgt;
	int ret;

	if (strstr(name, "tpgt_") != name)
		return ERR_PTR(-EINVAL);
	ret = kstrtou16(name + 5, 10, &tpgt);
	if (ret)
		return ERR_PTR(ret);

	tpg = kzalloc(sizeof(struct scsiback_tpg), GFP_KERNEL);
	if (!tpg)
		return ERR_PTR(-ENOMEM);

	mutex_init(&tpg->tv_tpg_mutex);
	INIT_LIST_HEAD(&tpg->tv_tpg_list);
	INIT_LIST_HEAD(&tpg->info_list);
	tpg->tport = tport;
	tpg->tport_tpgt = tpgt;

	ret = core_tpg_register(&scsiback_ops, wwn,
				&tpg->se_tpg, tpg, TRANSPORT_TPG_TYPE_NORMAL);
	if (ret < 0) {
		kfree(tpg);
		return NULL;
	}
	mutex_lock(&scsiback_mutex);
	list_add_tail(&tpg->tv_tpg_list, &scsiback_list);
	mutex_unlock(&scsiback_mutex);

	return &tpg->se_tpg;
}

static void scsiback_drop_tpg(struct se_portal_group *se_tpg)
{
	struct scsiback_tpg *tpg = container_of(se_tpg,
				struct scsiback_tpg, se_tpg);

	mutex_lock(&scsiback_mutex);
	list_del(&tpg->tv_tpg_list);
	mutex_unlock(&scsiback_mutex);
	/*
	 * Release the virtual I_T Nexus for this xen-pvscsi TPG
	 */
	scsiback_drop_nexus(tpg);
	/*
	 * Deregister the se_tpg from TCM.
	 */
	core_tpg_deregister(se_tpg);
	kfree(tpg);
}

static int scsiback_check_true(struct se_portal_group *se_tpg)
{
	return 1;
}

static int scsiback_check_false(struct se_portal_group *se_tpg)
{
	return 0;
}

static const struct target_core_fabric_ops scsiback_ops = {
	.module				= THIS_MODULE,
	.name				= "xen-pvscsi",
	.get_fabric_name		= scsiback_get_fabric_name,
	.get_fabric_proto_ident		= scsiback_get_fabric_proto_ident,
	.tpg_get_wwn			= scsiback_get_fabric_wwn,
	.tpg_get_tag			= scsiback_get_tag,
	.tpg_get_default_depth		= scsiback_get_default_depth,
	.tpg_get_pr_transport_id	= scsiback_get_pr_transport_id,
	.tpg_get_pr_transport_id_len	= scsiback_get_pr_transport_id_len,
	.tpg_parse_pr_out_transport_id	= scsiback_parse_pr_out_transport_id,
	.tpg_check_demo_mode		= scsiback_check_true,
	.tpg_check_demo_mode_cache	= scsiback_check_true,
	.tpg_check_demo_mode_write_protect = scsiback_check_false,
	.tpg_check_prod_mode_write_protect = scsiback_check_false,
	.tpg_alloc_fabric_acl		= scsiback_alloc_fabric_acl,
	.tpg_release_fabric_acl		= scsiback_release_fabric_acl,
	.tpg_get_inst_index		= scsiback_tpg_get_inst_index,
	.check_stop_free		= scsiback_check_stop_free,
	.release_cmd			= scsiback_release_cmd,
	.put_session			= NULL,
	.shutdown_session		= scsiback_shutdown_session,
	.close_session			= scsiback_close_session,
	.sess_get_index			= scsiback_sess_get_index,
	.sess_get_initiator_sid		= NULL,
	.write_pending			= scsiback_write_pending,
	.write_pending_status		= scsiback_write_pending_status,
	.set_default_node_attributes	= scsiback_set_default_node_attrs,
	.get_task_tag			= scsiback_get_task_tag,
	.get_cmd_state			= scsiback_get_cmd_state,
	.queue_data_in			= scsiback_queue_data_in,
	.queue_status			= scsiback_queue_status,
	.queue_tm_rsp			= scsiback_queue_tm_rsp,
	.aborted_task			= scsiback_aborted_task,
	/*
	 * Setup callers for generic logic in target_core_fabric_configfs.c
	 */
	.fabric_make_wwn		= scsiback_make_tport,
	.fabric_drop_wwn		= scsiback_drop_tport,
	.fabric_make_tpg		= scsiback_make_tpg,
	.fabric_drop_tpg		= scsiback_drop_tpg,
	.fabric_post_link		= scsiback_port_link,
	.fabric_pre_unlink		= scsiback_port_unlink,
	.fabric_make_np			= NULL,
	.fabric_drop_np			= NULL,
#if 0
	.fabric_make_nodeacl		= scsiback_make_nodeacl,
	.fabric_drop_nodeacl		= scsiback_drop_nodeacl,
#endif

	.tfc_wwn_attrs			= scsiback_wwn_attrs,
	.tfc_tpg_base_attrs		= scsiback_tpg_attrs,
	.tfc_tpg_param_attrs		= scsiback_param_attrs,
};

static const struct xenbus_device_id scsiback_ids[] = {
	{ "vscsi" },
	{ "" }
};

static struct xenbus_driver scsiback_driver = {
	.ids			= scsiback_ids,
	.probe			= scsiback_probe,
	.remove			= scsiback_remove,
	.otherend_changed	= scsiback_frontend_changed
};

static void scsiback_init_pend(void *p)
{
	struct vscsibk_pend *pend = p;
	int i;

	memset(pend, 0, sizeof(*pend));
	for (i = 0; i < VSCSI_MAX_GRANTS; i++)
		pend->grant_handles[i] = SCSIBACK_INVALID_HANDLE;
}

static int __init scsiback_init(void)
{
	int ret;

	if (!xen_domain())
		return -ENODEV;

	pr_debug("xen-pvscsi: fabric module %s on %s/%s on "UTS_RELEASE"\n",
		 VSCSI_VERSION, utsname()->sysname, utsname()->machine);

	scsiback_cachep = kmem_cache_create("vscsiif_cache",
		sizeof(struct vscsibk_pend), 0, 0, scsiback_init_pend);
	if (!scsiback_cachep)
		return -ENOMEM;

	ret = xenbus_register_backend(&scsiback_driver);
	if (ret)
		goto out_cache_destroy;

	ret = target_register_template(&scsiback_ops);
	if (ret)
		goto out_unregister_xenbus;

	return 0;

out_unregister_xenbus:
	xenbus_unregister_driver(&scsiback_driver);
out_cache_destroy:
	kmem_cache_destroy(scsiback_cachep);
	pr_err("%s: error %d\n", __func__, ret);
	return ret;
}

static void __exit scsiback_exit(void)
{
	struct page *page;

	while (free_pages_num) {
		if (get_free_page(&page))
			BUG();
		gnttab_free_pages(1, &page);
	}
	target_unregister_template(&scsiback_ops);
	xenbus_unregister_driver(&scsiback_driver);
	kmem_cache_destroy(scsiback_cachep);
}

module_init(scsiback_init);
module_exit(scsiback_exit);

MODULE_DESCRIPTION("Xen SCSI backend driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("xen-backend:vscsi");
MODULE_AUTHOR("Juergen Gross <jgross@suse.com>");
