/**
 * @file Broadcom Dongle Host Driver (DHD), Flow ring specific code at top level
 *
 * Flow rings are transmit traffic (=propagating towards antenna) related entities
 *
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_flowrings.c jaganlv $
 */

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <ethernet.h>
#include <bcmevent.h>
#include <dngl_stats.h>

#include <dhd.h>

#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <802.1d.h>
#include <pcie_core.h>
#include <bcmmsgbuf.h>
#include <dhd_pcie.h>
#include <bcmpcie.h>
#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */
#ifdef DHD_WMF
#include <dhd_wmf_linux.h>
#endif /* DHD_WMF */

#include <dhd_linux.h>

#if defined(BCM_ROUTER_DHD) || defined(STB)
extern bool dhd_sta_associated(dhd_pub_t *dhdp, uint32 bssidx, uint8 *mac);
#endif

static INLINE void *dhd_flowid_map_init(dhd_pub_t *dhdp, int num_flow_rings);

static INLINE void *dhd_flowid_map_fini(dhd_pub_t *dhdp, void *flowid_map);

static INLINE uint16 dhd_flowid_map_alloc(dhd_pub_t *dhdp, void *flowid_map,
	uint8 ifindex, uint8 prio, char *sa, char *da);

static INLINE void dhd_flowid_map_free(dhd_pub_t *dhdp, void *flowid_map,
	uint16 flowid);

static INLINE int dhd_flow_queue_throttle(flow_queue_t *queue);

static INLINE uint16 dhd_flowid_find(dhd_pub_t *dhdp, uint8 ifindex,
                                     uint8 prio, char *sa, char *da);

static INLINE uint16 dhd_flowid_alloc(dhd_pub_t *dhdp, uint8 ifindex,
                                      uint8 prio, char *sa, char *da);

int BCMFASTPATH dhd_flow_queue_overflow(flow_queue_t *queue, void *pkt);

#define FLOW_QUEUE_PKT_NEXT(p)          PKTLINK(p)
#define FLOW_QUEUE_PKT_SETNEXT(p, x)    PKTSETLINK((p), (x))

#if defined(CONFIG_BCM_DPI_WLAN_QOS)
const uint8 prio2ac[8] = { 1, 0, 0, 0, 0, 0, 2, 3 };
#else
const uint8 prio2ac[8] = { 0, 1, 1, 0, 2, 2, 3, 3 };
#endif
const uint8 prio2tid[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

/** Queue overflow throttle. Return value: TRUE if throttle needs to be applied */
static INLINE int
dhd_flow_queue_throttle(flow_queue_t *queue)
{
#if defined(BCM_ROUTER_DHD)
	/* Two tests
	 * 1) Test whether overall level 2 (grandparent) cummulative threshold crossed.
	 * 2) Or test whether queue's budget and overall cummulative threshold crossed.
	 */
	void *gp_clen_ptr = DHD_FLOW_QUEUE_L2CLEN_PTR(queue);
	void *parent_clen_ptr = DHD_FLOW_QUEUE_CLEN_PTR(queue);
	int gp_cumm_threshold = DHD_FLOW_QUEUE_L2THRESHOLD(queue);
	int cumm_threshold = DHD_FLOW_QUEUE_THRESHOLD(queue);

	int ret = ((DHD_CUMM_CTR_READ(gp_clen_ptr) > gp_cumm_threshold) ||
		((DHD_FLOW_QUEUE_OVFL(queue, DHD_FLOW_QUEUE_MAX(queue))) &&
		(DHD_CUMM_CTR_READ(parent_clen_ptr) > cumm_threshold)));
	return ret;
#else
	return DHD_FLOW_QUEUE_FULL(queue);
#endif /* ! BCM_ROUTER_DHD */
}

#ifdef BCM_PKTFWD
static INLINE int
dhd_pktfwd_queue_throttle(flow_queue_t *queue, int pkt_count)
{
	/* Two tests
	 * 1) Test whether overall level 2 (grandparent) cummulative threshold crossed.
	 * 2) Or test whether queue's budget and overall cummulative threshold crossed.
	 */
	void *gp_clen_ptr = DHD_FLOW_QUEUE_L2CLEN_PTR(queue);
	void *parent_clen_ptr = DHD_FLOW_QUEUE_CLEN_PTR(queue);
	int gp_cumm_threshold = DHD_FLOW_QUEUE_L2THRESHOLD(queue);
	int cumm_threshold = DHD_FLOW_QUEUE_THRESHOLD(queue);
	int ret = (((DHD_CUMM_CTR_READ(gp_clen_ptr) + pkt_count) > gp_cumm_threshold) ||
		(((queue->len + pkt_count) > DHD_FLOW_QUEUE_MAX(queue)) &&
		((DHD_CUMM_CTR_READ(parent_clen_ptr) + pkt_count) > cumm_threshold)));

	return ret;
} /* dhd_pktfwd_queue_throttle */

/**
 * Enqueue a list of 802.3 packet at the back of a flow ring's queue.
 * From there, it will travel later on to the flow ring itself.
 */
int BCMFASTPATH
dhd_pktfwd_pktlist_enqueue(dhd_pub_t *dhdp, uint16 flowid,
	void *pktlist_head, void *pktlist_tail, uint16 pktlist_count)
{
	int ret = BCME_OK;
	flow_ring_node_t *flow_ring_node;
	flow_queue_t *queue;

	ASSERT(flowid < dhdp->num_flow_rings);

	flow_ring_node = DHD_FLOW_RING(dhdp, flowid);

	DHD_TRACE(("%s: pkt flowid %d, status %d active %d\n",
		__FUNCTION__, flowid, flow_ring_node->status,
		flow_ring_node->active));

	queue = &flow_ring_node->queue; /* queue associated with flow ring */

	ASSERT(queue != NULL);

	if (dhd_pktfwd_queue_throttle(queue, pktlist_count)) {
		queue->failures++;
		ret = (*queue->cb)(queue, pktlist_head);
		goto done;
	}

	if (queue->head) {
		FLOW_QUEUE_PKT_SETNEXT(queue->tail, pktlist_head);
	} else {
		queue->head = pktlist_head;
	}

	queue->tail = pktlist_tail;
	FLOW_QUEUE_PKT_SETNEXT(queue->tail, NULL);
	queue->len += pktlist_count;
	/* increment parent's cummulative length */
	DHD_CUMM_CTR(DHD_FLOW_QUEUE_CLEN_PTR(queue)) += pktlist_count;
	/* increment grandparent's cummulative length */
	DHD_CUMM_CTR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue)) += pktlist_count;

	/* move the flowring queue (from txqueue_xxx) to txqueue_pend, ignore ret */
	dll_delete(&queue->list);
	dll_prepend(&dhdp->bus->txqueue_pend, &queue->list);

done:
	return ret;
} /* dhd_pktfwd_pktlist_enqueue */
#endif /* BCM_PKTFWD */

int BCMFASTPATH
dhd_flow_queue_overflow(flow_queue_t *queue, void *pkt)
{
	return BCME_NORESOURCE;
}

/** Returns flow ring given a flowid */
flow_ring_node_t *
dhd_flow_ring_node(dhd_pub_t *dhdp, uint16 flowid)
{
	flow_ring_node_t * flow_ring_node;

	ASSERT(dhdp != (dhd_pub_t*)NULL);
	ASSERT(flowid < dhdp->num_flow_rings);

	flow_ring_node = &(((flow_ring_node_t*)(dhdp->flow_ring_table))[flowid]);
	ASSERT(flow_ring_node->flowid == flowid);

	return flow_ring_node;
}

/** Returns 'backup' queue given a flowid */
flow_queue_t *
dhd_flow_queue(dhd_pub_t *dhdp, uint16 flowid)
{
	flow_ring_node_t * flow_ring_node;

	flow_ring_node = dhd_flow_ring_node(dhdp, flowid);

	return &flow_ring_node->queue;
}

/* Flow ring's queue management functions */

/** Reinitialize a flow ring's queue. */
void
dhd_flow_queue_reinit(dhd_pub_t *dhdp, flow_queue_t *queue, int max)
{
	ASSERT((queue != NULL) && (max > 0));

	queue->head = queue->tail = NULL;
	queue->len = 0;

	/* Set queue's threshold and queue's parent cummulative length counter */
	ASSERT(max > 1);
	DHD_FLOW_QUEUE_SET_MAX(queue, max);
	DHD_FLOW_QUEUE_SET_THRESHOLD(queue, max);
	DHD_FLOW_QUEUE_SET_CLEN(queue, &dhdp->cumm_ctr);
	DHD_FLOW_QUEUE_SET_L2CLEN(queue, &dhdp->l2cumm_ctr);

	queue->failures = 0U;
	queue->cb = &dhd_flow_queue_overflow;
}

/** Initialize a flow ring's queue, called on driver initialization. */
void
dhd_flow_queue_init(dhd_pub_t *dhdp, flow_queue_t *queue, int max)
{
	ASSERT((queue != NULL) && (max > 0));

	dll_init(&queue->list);
	dhd_flow_queue_reinit(dhdp, queue, max);
}

/** Register an enqueue overflow callback handler */
void
dhd_flow_queue_register(flow_queue_t *queue, flow_queue_cb_t cb)
{
	ASSERT(queue != NULL);
	queue->cb = cb;
}

/**
 * Enqueue an 802.3 packet at the back of a flow ring's queue. From there, it will travel later on
 * to the flow ring itself.
 */
int BCMFASTPATH
dhd_flow_queue_enqueue(dhd_pub_t *dhdp, flow_queue_t *queue, void *pkt)
{
	int ret = BCME_OK;
	struct ether_header *eh = NULL;

	ASSERT(queue != NULL);

	eh = (struct ether_header *)PKTDATA(dhdp->osh, pkt);

	/* Allow non IP stack generated packets to go through */
	if (((ntoh16(eh->ether_type) == ETHER_TYPE_IP) ||
		((ntoh16(eh->ether_type) == ETHER_TYPE_IPV6))) &&
		dhd_flow_queue_throttle(queue)) {
	    queue->failures++;
	    ret = (*queue->cb)(queue, pkt);
	    goto done;
	}

	if (queue->head) {
		FLOW_QUEUE_PKT_SETNEXT(queue->tail, pkt);
	} else {
		queue->head = pkt;
	}

	FLOW_QUEUE_PKT_SETNEXT(pkt, NULL);

	queue->tail = pkt; /* at tail */

	queue->len++;
	/* increment parent's cummulative length */
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	/* increment grandparent's cummulative length */
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));

done:
	return ret;
}

/** Dequeue an 802.3 packet from a flow ring's queue, from head (FIFO) */
void * BCMFASTPATH
dhd_flow_queue_dequeue(dhd_pub_t *dhdp, flow_queue_t *queue)
{
	void * pkt;

	ASSERT(queue != NULL);

	pkt = queue->head; /* from head */

	if (pkt == NULL) {
		ASSERT((queue->len == 0) && (queue->tail == NULL));
		goto done;
	}

	queue->head = FLOW_QUEUE_PKT_NEXT(pkt);
	if (queue->head == NULL)
		queue->tail = NULL;

	queue->len--;
	/* decrement parent's cummulative length */
	DHD_CUMM_CTR_DECR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	/* decrement grandparent's cummulative length */
	DHD_CUMM_CTR_DECR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));

	FLOW_QUEUE_PKT_SETNEXT(pkt, NULL); /* dettach packet from queue */

done:
	return pkt;
}

/** Reinsert a dequeued 802.3 packet back at the head */
void BCMFASTPATH
dhd_flow_queue_reinsert(dhd_pub_t *dhdp, flow_queue_t *queue, void *pkt)
{
	if (queue->head == NULL) {
		queue->tail = pkt;
	}

	FLOW_QUEUE_PKT_SETNEXT(pkt, queue->head);
	queue->head = pkt;
	queue->len++;
	/* increment parent's cummulative length */
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_CLEN_PTR(queue));
	/* increment grandparent's cummulative length */
	DHD_CUMM_CTR_INCR(DHD_FLOW_QUEUE_L2CLEN_PTR(queue));
}

/** Fetch the backup queue for a flowring, and assign flow control thresholds */
void
dhd_flow_ring_config_thresholds(dhd_pub_t *dhdp, uint16 flowid,
                     int queue_budget, int cumm_threshold, void *cumm_ctr,
                     int l2cumm_threshold, void *l2cumm_ctr)
{
	flow_queue_t * queue;

	ASSERT(dhdp != (dhd_pub_t*)NULL);
	ASSERT(queue_budget > 1);
	ASSERT(cumm_threshold > 1);
	ASSERT(cumm_ctr != (void*)NULL);
	ASSERT(l2cumm_threshold > 1);
	ASSERT(l2cumm_ctr != (void*)NULL);

	queue = dhd_flow_queue(dhdp, flowid);

	DHD_FLOW_QUEUE_SET_MAX(queue, queue_budget); /* Max queue length */

	/* Set the queue's parent threshold and cummulative counter */
	DHD_FLOW_QUEUE_SET_THRESHOLD(queue, cumm_threshold);
	DHD_FLOW_QUEUE_SET_CLEN(queue, cumm_ctr);

	/* Set the queue's grandparent threshold and cummulative counter */
	DHD_FLOW_QUEUE_SET_L2THRESHOLD(queue, l2cumm_threshold);
	DHD_FLOW_QUEUE_SET_L2CLEN(queue, l2cumm_ctr);
}

/** Initializes data structures of multiple flow rings */
int
dhd_flow_rings_init(dhd_pub_t *dhdp, uint32 num_flow_rings)
{
	uint32 idx;
	uint32 flow_ring_table_sz;
	uint32 if_flow_lkup_sz = 0;
	void * flowid_allocator;
	flow_ring_table_t *flow_ring_table = NULL;
	if_flow_lkup_t *if_flow_lkup = NULL;
	void *lock = NULL;
	unsigned long flags;

	DHD_INFO(("%s\n", __FUNCTION__));

	/* called without deinit */
	if (dhdp->flow_rings_inited == TRUE) {
		DHD_ERROR(("%s: called while already inited\n", __FUNCTION__));
		return BCME_BUSY;
	}

	/* Construct a 16bit flowid allocator */
	flowid_allocator = dhd_flowid_map_init(dhdp, num_flow_rings);
	if (flowid_allocator == NULL) {
		DHD_ERROR(("%s: flowid allocator init failure\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	/* Allocate a flow ring table, comprising of requested number of rings */
	flow_ring_table_sz = (num_flow_rings * sizeof(flow_ring_node_t));
	flow_ring_table = (flow_ring_table_t *)MALLOCZ(dhdp->osh, flow_ring_table_sz);
	if (flow_ring_table == NULL) {
		DHD_ERROR(("%s: flow ring table alloc failure\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize flow ring table state */
	DHD_CUMM_CTR_INIT(&dhdp->cumm_ctr);
	DHD_CUMM_CTR_INIT(&dhdp->l2cumm_ctr);
	bzero((uchar *)flow_ring_table, flow_ring_table_sz);
	for (idx = 0; idx < num_flow_rings; idx++) {
		flow_ring_table[idx].status = FLOW_RING_STATUS_CLOSED;
		flow_ring_table[idx].flowid = (uint16)idx;
		flow_ring_table[idx].lock = dhd_os_spin_lock_init(dhdp->osh);
		if (flow_ring_table[idx].lock == NULL) {
			DHD_ERROR(("%s: Failed to init spinlock for queue!\n", __FUNCTION__));
			goto fail;
		}

		dll_init(&flow_ring_table[idx].list);

		/* Initialize the per flow ring backup queue */
		dhd_flow_queue_init(dhdp, &flow_ring_table[idx].queue,
		                    FLOW_RING_QUEUE_THRESHOLD);
	}

	/* Allocate per interface hash table (for fast lookup from interface to flow ring) */
	if_flow_lkup_sz = sizeof(if_flow_lkup_t) * DHD_MAX_IFS;
	if_flow_lkup = (if_flow_lkup_t *)DHD_OS_PREALLOC(dhdp,
		DHD_PREALLOC_IF_FLOW_LKUP, if_flow_lkup_sz);
	if (if_flow_lkup == NULL) {
		DHD_ERROR(("%s: if flow lkup alloc failure\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize per interface hash table */
	for (idx = 0; idx < DHD_MAX_IFS; idx++) {
		int hash_ix;
		if_flow_lkup[idx].status = 0;
		if_flow_lkup[idx].role = 0;
		for (hash_ix = 0; hash_ix < DHD_FLOWRING_HASH_SIZE; hash_ix++)
			if_flow_lkup[idx].fl_hash[hash_ix] = NULL;
	}

	lock = dhd_os_spin_lock_init(dhdp->osh);
	if (lock == NULL)
		goto fail;

	if (dhdp->bus->pcie_ipc.hcap1 & PCIE_IPC_HCAP1_FLOWRING_TID) {
		dhdp->flow_prio_map_type = DHD_FLOW_PRIO_TID_MAP;
		bcopy(prio2tid, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);
	} else {
		dhdp->flow_prio_map_type = DHD_FLOW_PRIO_AC_MAP;
		bcopy(prio2ac, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);
	}

	/* Now populate into dhd pub */
	DHD_FLOWID_LOCK(lock, flags);
	dhdp->num_flow_rings = num_flow_rings;
	dhdp->flowid_allocator = (void *)flowid_allocator;
	dhdp->flow_ring_table = (void *)flow_ring_table;
	dhdp->if_flow_lkup = (void *)if_flow_lkup;
	dhdp->flowid_lock = lock;
	dhdp->flow_rings_inited = TRUE;
	DHD_FLOWID_UNLOCK(lock, flags);

	DHD_INFO(("%s done\n", __FUNCTION__));

	return BCME_OK;

fail:
	/* Destruct the per interface flow lkup table */
	if (if_flow_lkup != NULL) {
		DHD_OS_PREFREE(dhdp, if_flow_lkup, if_flow_lkup_sz);
	}

	if (flow_ring_table != NULL) {
		for (idx = 0; idx < num_flow_rings; idx++) {
			if (flow_ring_table[idx].lock != NULL)
				dhd_os_spin_lock_deinit(dhdp->osh, flow_ring_table[idx].lock);
		}
		MFREE(dhdp->osh, flow_ring_table, flow_ring_table_sz);
	}

	dhd_flowid_map_fini(dhdp, flowid_allocator);

	return BCME_NOMEM;
}

/** Deinit Flow Ring specific data structures */
void
dhd_flow_rings_deinit(dhd_pub_t *dhdp)
{
	uint16 idx;
	uint32 flow_ring_table_sz;
	uint32 if_flow_lkup_sz;
	flow_ring_table_t *flow_ring_table;
	unsigned long flags;
	void *lock;

	DHD_INFO(("dhd_flow_rings_deinit\n"));

	if (!(dhdp->flow_rings_inited)) {
		DHD_ERROR(("dhd_flow_rings not initialized!\n"));
		return;
	}

	if (dhdp->flow_ring_table != NULL) {

		ASSERT(dhdp->num_flow_rings > 0);

		DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
		flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
		dhdp->flow_ring_table = NULL;
		DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
		for (idx = 0; idx < dhdp->num_flow_rings; idx++) {
			if (flow_ring_table[idx].active) {
				dhd_bus_clean_flow_ring(dhdp->bus, &flow_ring_table[idx]);
			}
			ASSERT(DHD_FLOW_QUEUE_EMPTY(&flow_ring_table[idx].queue));

			/* Deinit flow ring queue locks before destroying flow ring table */
			dhd_os_spin_lock_deinit(dhdp->osh, flow_ring_table[idx].lock);
			flow_ring_table[idx].lock = NULL;

		}

		/* Destruct the flow ring table */
		flow_ring_table_sz = dhdp->num_flow_rings * sizeof(flow_ring_table_t);
		MFREE(dhdp->osh, flow_ring_table, flow_ring_table_sz);
	}

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);

	/* Destruct the per interface flow lkup table */
	if (dhdp->if_flow_lkup != NULL) {
		if_flow_lkup_sz = sizeof(if_flow_lkup_t) * DHD_MAX_IFS;
		bzero((uchar *)dhdp->if_flow_lkup, if_flow_lkup_sz);
		DHD_OS_PREFREE(dhdp, dhdp->if_flow_lkup, if_flow_lkup_sz);
		dhdp->if_flow_lkup = NULL;
	}

	/* Destruct the flowid allocator */
	if (dhdp->flowid_allocator != NULL)
		dhdp->flowid_allocator = dhd_flowid_map_fini(dhdp, dhdp->flowid_allocator);

	dhdp->num_flow_rings = 0U;
	bzero(dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);

	lock = dhdp->flowid_lock;
	dhdp->flowid_lock = NULL;

	DHD_FLOWID_UNLOCK(lock, flags);
	dhd_os_spin_lock_deinit(dhdp->osh, lock);

	ASSERT(dhdp->if_flow_lkup == NULL);
	ASSERT(dhdp->flowid_allocator == NULL);
	ASSERT(dhdp->flow_ring_table == NULL);
	dhdp->flow_rings_inited = FALSE;
}

/** Uses hash table to quickly map from ifindex to a flow ring 'role' (STA/AP) */
uint8
dhd_flow_rings_ifindex2role(dhd_pub_t *dhdp, uint8 ifindex)
{
	if_flow_lkup_t *if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	ASSERT(if_flow_lkup);

	return if_flow_lkup[ifindex].role;
}

#ifdef WLTDLS
bool is_tdls_destination(dhd_pub_t *dhdp, uint8 *da)
{
	tdls_peer_node_t *cur = dhdp->peer_tbl.node;

	while (cur != NULL) {
		if (!memcmp(da, cur->addr, ETHER_ADDR_LEN)) {
			return TRUE;
		}
		cur = cur->next;
	}

	return FALSE;
}
#endif /* WLTDLS */

/** Uses hash table to quickly map from ifindex+prio+da to a flow ring id */
static INLINE uint16
dhd_flowid_find(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, char *sa, char *da)
{
	int hash;
	bool ismcast = FALSE;
	flow_hash_info_t *cur;
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	ASSERT(if_flow_lkup);

	if ((if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) ||
			(if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_WDS)) {
#ifdef WLTDLS
		if (dhdp->peer_tbl.tdls_peer_count && !(ETHER_ISMULTI(da)) &&
			is_tdls_destination(dhdp, da)) {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
			cur = if_flow_lkup[ifindex].fl_hash[hash];
			while (cur != NULL) {
				if (!memcmp(cur->flow_info.da, da, ETHER_ADDR_LEN)) {
					DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
					return cur->flowid;
				}
				cur = cur->next;
			}
			DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
			return FLOWID_INVALID;
		}
#endif /* WLTDLS */
		/* For STA non TDLS dest and WDS dest flow ring id is mapped based on prio only */
		cur = if_flow_lkup[ifindex].fl_hash[prio];
		if (cur) {
			DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
			return cur->flowid;
		}
	} else {

		if (ETHER_ISMULTI(da)) {
			ismcast = TRUE;
			hash = 0;
		} else {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
		}

		cur = if_flow_lkup[ifindex].fl_hash[hash];

		while (cur) {
			if ((ismcast && ETHER_ISMULTI(cur->flow_info.da)) ||
				(!memcmp(cur->flow_info.da, da, ETHER_ADDR_LEN) &&
				(cur->flow_info.tid == prio))) {
				DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
				return cur->flowid;
			}
			cur = cur->next;
		}
	}

	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
	DHD_INFO(("%s: cannot find flowid\n", __FUNCTION__));

	return FLOWID_INVALID;
} /* dhd_flowid_find */

/** Create unique Flow ID, called when a flow ring is created. */
static INLINE uint16
dhd_flowid_alloc(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, char *sa, char *da)
{
	flow_hash_info_t *fl_hash_node, *cur;
	if_flow_lkup_t *if_flow_lkup;
	int hash;
	uint16 flowid;
	unsigned long flags;

	fl_hash_node = (flow_hash_info_t *) MALLOC(dhdp->osh, sizeof(flow_hash_info_t));
	memcpy(fl_hash_node->flow_info.da, da, sizeof(fl_hash_node->flow_info.da));

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	ASSERT(dhdp->flowid_allocator != NULL);
	flowid = dhd_flowid_map_alloc(dhdp, dhdp->flowid_allocator,
		ifindex, prio, sa, da);
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	if (flowid == FLOWID_INVALID) {
		MFREE(dhdp->osh, fl_hash_node,  sizeof(flow_hash_info_t));
		DHD_ERROR(("%s: %s: cannot get free flowid \n",
			dhd_ifname(dhdp, ifindex), __FUNCTION__));
		return FLOWID_INVALID;
	}

	fl_hash_node->flowid = flowid;
	fl_hash_node->flow_info.tid = prio;
	fl_hash_node->flow_info.ifindex = ifindex;
	fl_hash_node->next = NULL;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if ((if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) ||
			(if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_WDS)) {
		/* For STA non TDLS dest and WDS dest we allocate entry based on prio only */
#ifdef WLTDLS
		if (dhdp->peer_tbl.tdls_peer_count &&
			(is_tdls_destination(dhdp, da))) {
			hash = DHD_FLOWRING_HASHINDEX(da, prio);
			cur = if_flow_lkup[ifindex].fl_hash[hash];
			if (cur) {
				while (cur->next) {
					cur = cur->next;
				}
				cur->next = fl_hash_node;
			} else {
				if_flow_lkup[ifindex].fl_hash[hash] = fl_hash_node;
			}
		} else
#endif /* WLTDLS */
			if_flow_lkup[ifindex].fl_hash[prio] = fl_hash_node;
	} else {

		/* For bcast/mcast assign first slot in in interface */
		hash = ETHER_ISMULTI(da) ? 0 : DHD_FLOWRING_HASHINDEX(da, prio);
		cur = if_flow_lkup[ifindex].fl_hash[hash];
		if (cur) {
			while (cur->next) {
				cur = cur->next;
			}
			cur->next = fl_hash_node;
		} else
			if_flow_lkup[ifindex].fl_hash[hash] = fl_hash_node;
	}

	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
	DHD_INFO(("%s: %s: allocated flowid %d\n", dhd_ifname(dhdp, ifindex),
		__FUNCTION__, fl_hash_node->flowid));

	return fl_hash_node->flowid;
} /* dhd_flowid_alloc */

/** Get flow ring ID, if not present try to create one */
int
dhd_flowid_lookup(dhd_pub_t *dhdp, uint8 ifindex,
                  uint8 prio, char *sa, char *da, uint16 *flowid)
{
	uint16 id;
	flow_ring_node_t *flow_ring_node;
	flow_ring_table_t *flow_ring_table;
	unsigned long flags;

	DHD_INFO(("%s\n", __FUNCTION__));

	if (!dhdp->flow_ring_table) {
		return BCME_ERROR;
	}

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;

	id = dhd_flowid_find(dhdp, ifindex, prio, sa, da);

	if (id == FLOWID_INVALID) {

		if_flow_lkup_t *if_flow_lkup;
		if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

		if (!if_flow_lkup[ifindex].status)
			return BCME_ERROR;

#if defined(BCM_ROUTER_DHD) || defined(STB)
		if ((if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_AP) ||
				(if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_P2P_GO)) {
			if (ETHER_ISMULTI(da)) {
				/* For multicast packets, set prio to BE */
				prio = PRIO_8021D_BE;
			} else if (!dhd_sta_associated(dhdp, ifindex, da)) {
				return BCME_NOTASSOCIATED;
			}
		}
#endif

		id = dhd_flowid_alloc(dhdp, ifindex, prio, sa, da);
		if (id == FLOWID_INVALID) {
			DHD_ERROR(("%s: %s: alloc flowid status %u\n",
				dhd_ifname(dhdp, ifindex), __FUNCTION__,
				if_flow_lkup[ifindex].status));
			return BCME_ERROR;
		}

		/* register this flowid in dhd_pub */
		dhd_add_flowid(dhdp, ifindex, prio, da, id);
	}

	ASSERT(id < dhdp->num_flow_rings);

	flow_ring_node = (flow_ring_node_t *) &flow_ring_table[id];
	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
	if (flow_ring_node->active) {
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		*flowid = id;
		return BCME_OK;
	}
	/* Init Flow info */
	memcpy(flow_ring_node->flow_info.sa, sa, sizeof(flow_ring_node->flow_info.sa));
	memcpy(flow_ring_node->flow_info.da, da, sizeof(flow_ring_node->flow_info.da));
	flow_ring_node->flow_info.tid = prio;
	flow_ring_node->flow_info.ifindex = ifindex;
	flow_ring_node->active = TRUE;
	flow_ring_node->status = FLOW_RING_STATUS_PENDING;
#ifdef BCMDBG
	bzero(&flow_ring_node->flow_info.tx_status[0],
		sizeof(uint32) * DHD_FLOWRING_MAXSTATUS_MSGS);
#endif
	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Create and inform device about the new flow */
	if (dhd_bus_flow_ring_create_request(dhdp->bus, (void *)flow_ring_node)
	        != BCME_OK) {
		DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
		flow_ring_node->status = FLOW_RING_STATUS_CLOSED;
		flow_ring_node->active = FALSE;
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		DHD_ERROR(("%s: create error %d\n", __FUNCTION__, id));
		return BCME_ERROR;
	}

	*flowid = id;

	return BCME_OK;
} /* dhd_flowid_lookup */

/**
 * Assign existing or newly created flowid to an 802.3 packet. This flowid is later on used to
 * select the flowring to send the packet to the dongle.
 */
int BCMFASTPATH
dhd_flowid_update(dhd_pub_t *dhdp, uint8 ifindex, uint8 prio, void *pktbuf)
{
	uint8 *pktdata = (uint8 *)PKTDATA(dhdp->osh, pktbuf);
	struct ether_header *eh = (struct ether_header *)pktdata;
	uint16 flowid;
	int result;

	ASSERT(ifindex < DHD_MAX_IFS);

	if (ifindex >= DHD_MAX_IFS) {
		return BCME_BADARG;
	}

	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		return BCME_ERROR;
	}

#ifdef BCM_NBUFF_WLMCAST
	/* if it is coming from EMF, the mac address is in TAG */
	if ((DHD_PKT_GET_WMF_FKB_UCAST(pktbuf)))
		pktdata = DHD_PKT_GET_MAC(pktbuf);
	else if (FKB_HAS_DHDHDR(pktbuf))
		pktdata -= DOT11_LLC_SNAP_HDR_LEN;
	else
		pktdata = eh->ether_dhost;
	result = dhd_flowid_lookup(dhdp, ifindex, prio, eh->ether_shost, pktdata, &flowid);
#else

	result = dhd_flowid_lookup(dhdp, ifindex, prio, eh->ether_shost, eh->ether_dhost,
		&flowid);
#endif /* BCM_NBUFF_WLMCAST */
	if (result != BCME_OK) {
		return result;
	}

	DHD_INFO(("%s: %s: prio %d flowid %d\n",
		dhd_ifname(dhdp, ifindex), __FUNCTION__, prio, flowid));
	/* Tag the packet with flowid */
	DHD_PKT_SET_FLOWID(pktbuf, flowid);

	return BCME_OK;
}

#ifdef BCM_NBUFF_WLMCAST

static int
dhd_client_flowid_update(dhd_pub_t *dhdp, dhd_sta_t *sta, uint8 prio)
{
	uint16 flowid;
	uint8 ifindex = sta->ifidx;

	ASSERT(ifindex < DHD_MAX_IFS);

	if (ifindex >= DHD_MAX_IFS) {
		return BCME_BADARG;
	}

	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (dhd_flowid_lookup(dhdp, ifindex, dhdp->flow_prio_map[prio],
		sta->ea.octet, sta->ea.octet, &flowid) != BCME_OK) {
		return BCME_ERROR;
	}

	DHD_INFO((" dhd client flowip updated and get flowid:%d \r\n", flowid));

	return BCME_OK;
}

int
dhd_flowid_get_type(dhd_pub_t *dhdp, void *sta_p, int priority)
{
	dhd_sta_t *sta = (dhd_sta_t *)sta_p;
	uint16 flowid = sta->flowid[priority];
	flow_ring_node_t *flowring_node;

	/* in case the flowid has not even been assigned , it will be allocated  */
	if (flowid == FLOWID_INVALID) {
		if (dhd_client_flowid_update(dhdp, sta, priority) != BCME_OK) {
			DHD_INFO((" COULD NOT GET FLOWID ASSIGNED, return CPU type \r\n"));
			return WLAN_CLIENT_TYPE_CPU;
		}
		flowid = sta->flowid[priority];
	}

	DHD_INFO((" sta's prioroty:%d and get flowid:%d \r\n", priority, sta->flowid[priority]));
	flowring_node = DHD_FLOW_RING(dhdp, flowid);
#if defined(BCM_DHD_RUNNER)
	if (flowring_node && DHD_FLOWRING_RNR_OFFL(flowring_node)) {
		DHD_INFO((" RUNNER TYPE returned \r\n"));
		return WLAN_CLIENT_TYPE_RUNNER;
	} else
#endif /* BCM_DHD_RUNNER */
	{
		DHD_INFO((" WFD TYPE returned \r\n"));
		return WLAN_CLIENT_TYPE_WFD;
	}
}

#endif /* BCM_NBUFF_WLMCAST */

void
dhd_flowid_free(dhd_pub_t *dhdp, uint8 ifindex, uint16 flowid)
{
	int hashix;
	bool found = FALSE;
	flow_hash_info_t *cur, *prev;
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	for (hashix = 0; hashix < DHD_FLOWRING_HASH_SIZE; hashix++) {

		cur = if_flow_lkup[ifindex].fl_hash[hashix];

		if (cur) {
			if (cur->flowid == flowid) {
				found = TRUE;
			}

			prev = NULL;
			while (!found && cur) {
				if (cur->flowid == flowid) {
					found = TRUE;
					break;
				}
				prev = cur;
				cur = cur->next;
			}
			if (found) {
				if (!prev) {
					if_flow_lkup[ifindex].fl_hash[hashix] = cur->next;
				} else {
					prev->next = cur->next;
				}

				/* deregister flowid from dhd_pub. */
				dhd_del_flowid(dhdp, ifindex, flowid);

				dhd_flowid_map_free(dhdp,
				    dhdp->flowid_allocator,
				    flowid);
				DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
				MFREE(dhdp->osh, cur, sizeof(flow_hash_info_t));

				return;
			}
		}
	}

	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
	DHD_ERROR(("%s: %s: could not free flow ring hash entry flowid %d\n",
	           dhd_ifname(dhdp, ifindex), __FUNCTION__, flowid));
} /* dhd_flowid_free */

/**
 * Delete all Flow rings associated with the given interface. Is called when e.g. the dongle
 * indicates that a wireless link has gone down.
 */
void
dhd_flow_rings_delete(dhd_pub_t *dhdp, uint8 ifindex)
{
	uint32 id;
	flow_ring_table_t *flow_ring_table;

	DHD_INFO(("%s: %s: ifindex %u\n", dhd_ifname(dhdp, ifindex), __FUNCTION__, ifindex));

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	if (!dhdp->flow_ring_table)
		return;

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
	for (id = 0; id < dhdp->num_flow_rings; id++) {
		if (flow_ring_table[id].active &&
		    (flow_ring_table[id].flow_info.ifindex == ifindex) &&
		    (flow_ring_table[id].status != FLOW_RING_STATUS_DELETE_PENDING)) {
			dhd_bus_flow_ring_delete_request(dhdp->bus,
			                                 (void *) &flow_ring_table[id]);
		}
#ifdef DHD_IFE
		/*
		 * We skipped the delete request because its already initiated
		 * from IFE but not completed yet.
		 * Set inprogress to false so that we clean the Q after
		 * getting the response back from dongle
		 */
		if ((flow_ring_table[id].status == FLOW_RING_STATUS_DELETE_PENDING) &&
			(flow_ring_table[id].flow_info.ifindex == ifindex) &&
			flow_ring_table[id].evict_inprogress) {
			flow_ring_table[id].evict_inprogress = FALSE;
		}
#endif /* DHD_IFE */
	}
}

/** Delete flow ring(s) for given peer address. Related to AP/TDLS functionality. */
void
dhd_flow_rings_delete_for_peer(dhd_pub_t *dhdp, uint8 ifindex, char *addr)
{
	uint32 id;
	flow_ring_table_t *flow_ring_table;

	DHD_ERROR(("%s: %s: ifindex %u\n", dhd_ifname(dhdp, ifindex), __FUNCTION__, ifindex));

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	if (!dhdp->flow_ring_table)
		return;

	flow_ring_table = (flow_ring_table_t *)dhdp->flow_ring_table;
	for (id = 0; id < dhdp->num_flow_rings; id++) {
		if (flow_ring_table[id].active &&
			(flow_ring_table[id].flow_info.ifindex == ifindex) &&
			(!memcmp(flow_ring_table[id].flow_info.da, addr, ETHER_ADDR_LEN)) &&
			(flow_ring_table[id].status != FLOW_RING_STATUS_DELETE_PENDING)) {
			DHD_INFO(("%s: %s: deleting flowid %d\n", dhd_ifname(dhdp, ifindex),
				__FUNCTION__, flow_ring_table[id].flowid));
			dhd_bus_flow_ring_delete_request(dhdp->bus,
				(void *) &flow_ring_table[id]);
		}

#ifdef DHD_IFE
		/*
		 * We skipped the delete request because its already initiated
		 * from IFE but not completed yet.
		 * Set inprogress to false so that we clean the Q after
		 * getting the response back from dongle
		 */
		if ((flow_ring_table[id].status == FLOW_RING_STATUS_DELETE_PENDING) &&
			(flow_ring_table[id].flow_info.ifindex == ifindex) &&
			flow_ring_table[id].evict_inprogress) {
			flow_ring_table[id].evict_inprogress = FALSE;
		}
#endif /* DHD_IFE */
	}
}

/** Handles interface ADD, CHANGE, DEL indications from the dongle */
void
dhd_update_interface_flow_info(dhd_pub_t *dhdp, uint8 ifindex,
                               uint8 op, uint8 role)
{
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return;

	DHD_INFO(("%s: %s: ifindex %u op %u role is %u \n", dhd_ifname(dhdp, ifindex),
	          __FUNCTION__, ifindex, op, role));
	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		return;
	}

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if (op == WLC_E_IF_ADD || op == WLC_E_IF_CHANGE) {

		if_flow_lkup[ifindex].role = role;

		if (role == WLC_E_IF_ROLE_WDS) {
			/**
			 * WDS role does not send WLC_E_LINK event after interface is up.
			 * So to create flowrings for WDS, make status as TRUE in WLC_E_IF itself.
			 * same is true while making the status as FALSE.
			 * TODO: Fix FW to send WLC_E_LINK for WDS role as well. So that all the
			 * interfaces are handled uniformly.
			 */
			if_flow_lkup[ifindex].status = TRUE;
			DHD_INFO(("%s: %s: Flow ring for ifindex %d role is %d \n",
			          dhd_ifname(dhdp, ifindex), __FUNCTION__, ifindex, role));
		}
#if defined(BCM_PKTFWD)
		if (ifindex == 0) {
			if (role == WLC_E_IF_ROLE_STA)
				dhd_wlan_set_dwds_client(dhdp, ifindex, TRUE);
			else
				dhd_wlan_set_dwds_client(dhdp, ifindex, FALSE);
		}
#endif /* BCM_PKTFWD */

#ifdef DHD_WMF
		/* dhd already has created a primary interface by default,
		 * Configure wmf for primary interface.
		 * For virtual interfaces confiure wmf in dhd_allocate_if.
		 */
		if (ifindex == 0) {
			dhd_schedule_wmf_bss_enable(dhdp, ifindex);
		}
#endif /* DHD_WMF */

	} else	if ((op == WLC_E_IF_DEL) && (role == WLC_E_IF_ROLE_WDS)) {
		if_flow_lkup[ifindex].status = FALSE;
		DHD_INFO(("%s: %s: cleanup all Flow rings for ifindex %d role is %d \n",
		          dhd_ifname(dhdp, ifindex), __FUNCTION__, ifindex, role));
	}
	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);
}

/** Handles a STA 'link' indication from the dongle */
int
dhd_update_interface_link_status(dhd_pub_t *dhdp, uint8 ifindex, uint8 status)
{
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;

	ASSERT(ifindex < DHD_MAX_IFS);
	if (ifindex >= DHD_MAX_IFS)
		return BCME_BADARG;

	DHD_INFO(("%s: %s: ifindex %d status %d\n",
		dhd_ifname(dhdp, ifindex), __FUNCTION__, ifindex, status));

	DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
	if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	if_flow_lkup[ifindex].status = status ? TRUE : FALSE;

#ifdef BCM_ROUTER_DHD
	if (if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_AP ||
		if_flow_lkup[ifindex].role == WLC_E_IF_ROLE_STA) {
		dhd_update_bsscfg_state(dhdp, ifindex, status);
	}
#endif

	DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

	return BCME_OK;
}

/** Update flow priority mapping, called on IOVAR */
int
dhd_update_flow_prio_map(dhd_pub_t *dhdp, uint8 map)
{
	uint16 flowid;
	flow_ring_node_t *flow_ring_node;

	if (map > DHD_FLOW_PRIO_TID_MAP)
		return BCME_BADOPTION;

	/* Check if we need to change prio map */
	if (map == dhdp->flow_prio_map_type)
		return BCME_OK;

	/* If any ring is active we cannot change priority mapping for flow rings */
	for (flowid = 0; flowid < dhdp->num_flow_rings; flowid++) {
		flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		if (flow_ring_node->active)
			return BCME_EPERM;
	}

	/* Inform firmware about new mapping type */
	if (BCME_OK != dhd_flow_prio_map(dhdp, &map, TRUE))
		return BCME_ERROR;

	/* update internal structures */
	dhdp->flow_prio_map_type = map;
	if (dhdp->flow_prio_map_type == DHD_FLOW_PRIO_TID_MAP)
		bcopy(prio2tid, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);
	else
		bcopy(prio2ac, dhdp->flow_prio_map, sizeof(uint8) * NUMPRIO);

	return BCME_OK;
}

/** Inform firmware on updated flow priority mapping, called on IOVAR */
int
dhd_flow_prio_map(dhd_pub_t *dhd, uint8 *map, bool set)
{
	uint8 iovbuf[24];
	if (!set) {
		bcm_mkiovar("bus:fl_prio_map", NULL, 0, (char*)iovbuf, sizeof(iovbuf));
		if (dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, iovbuf, sizeof(iovbuf), FALSE, 0) < 0) {
			DHD_ERROR(("%s: failed to get fl_prio_map\n", __FUNCTION__));
			return BCME_ERROR;
		}
		*map = iovbuf[0];
		return BCME_OK;
	}

	bcm_mkiovar("bus:fl_prio_map", (char *)map, 4, (char*)iovbuf, sizeof(iovbuf));
	if (dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0) < 0) {
		DHD_ERROR(("%s: failed to set fl_prio_map \n",
			__FUNCTION__));
		return BCME_ERROR;
	}

	return BCME_OK;
}

/*
 * +----------------------------------------------------------------------------
 *           Section: iDMA Flowring Split Manager
 * +----------------------------------------------------------------------------
 */
typedef struct dhd_idma_map
{
	void *flow_id16_map; /* id number pool for each iDMA set */
	uint16 max;
	uint16 avail;
} dhd_idma_map_t;

/* flow ring manager in iDMA */
typedef struct dhd_idma_flowmgr
{
	void *flow_ids_map; /* flow id number pool */
	uint16 flows_per_set;    /* idma flows per set */
	dhd_idma_map_t maps[BCMPCIE_IDMA_H2D_MAX_DESC]; /* idma map per Set */
	dhd_idma_map_t *map_max; /* highest idma map */
} dhd_idma_flowmgr_t;

static int
dhd_idma_flowring_alloc(dhd_idma_flowmgr_t *flowmgr, dhd_idma_map_t *map)
{
	uint16 flow_id;

	flow_id = id16_map_alloc(flowmgr->flow_ids_map);
	if (flow_id == FLOWID_INVALID) {
		DHD_ERROR(("%s: Invalid flowid\n", __FUNCTION__));
		return BCME_ERROR;
	}

	/* Free this flow id to other map */
	id16_map_free(map->flow_id16_map, flow_id);
	map->avail++;
	DHD_INFO(("%s(): map @%p add flow id %d avail %d\n",
		__FUNCTION__, map, flow_id, map->avail));

	return BCME_OK;
}

void *
dhd_idma_flowmgr_init(dhd_pub_t *dhdp, int max_h2d_rings)
{
	int max_flows;
	uint16 total_ids;
	uint16 index = 0;
	dhd_idma_map_t *map;
	dhd_idma_flowmgr_t *flowmgr;

	ASSERT(dhdp->flowid_allocator == NULL);
	flowmgr = (dhd_idma_flowmgr_t *) MALLOCZ(dhdp->osh, sizeof(dhd_idma_flowmgr_t));
	if (flowmgr == NULL) {
		DHD_ERROR(("%s: flowmgr alloc failure\n", __FUNCTION__));
		return NULL;
	}

	/* Populate the flowids that will be used from 2..max */
	total_ids = max_h2d_rings - FLOW_RING_COMMON;
	flowmgr->flow_ids_map = id16_map_init(dhdp->osh, total_ids, FLOWID_RESERVED);
	if (flowmgr->flow_ids_map == NULL) {
		DHD_ERROR(("%s: flow_ids_map init failure\n", __FUNCTION__));
		MFREE(dhdp->osh, flowmgr, sizeof(dhd_idma_flowmgr_t));
		return NULL;
	}

	/*
	 * We can use 15 iDMA set for H2D WR, each set fully packed with
	 * 64B (32 2B indices or 16 4B indices)
	 */
	map = flowmgr->maps;

	/* First id16 map allocator only contains
	 * BCMPCIE_IDMA_FLOWS_PER_SET - FLOWID_RESERVED flows.
	 */
	flowmgr->flows_per_set = IDMA_FLOWS_PER_SET(dhdp);
	ASSERT(flowmgr->flows_per_set > FLOWID_RESERVED);
	max_flows = flowmgr->flows_per_set - FLOWID_RESERVED;
	map->max = max_flows;
	map->flow_id16_map = id16_map_init(dhdp->osh, max_flows, ID16_UNDEFINED);
	if (map->flow_id16_map == NULL) {
		DHD_ERROR(("%s: idma maps[%d] init failure\n",
			__FUNCTION__, index));
		goto error_rtn;
	}
	flowmgr->map_max = map;
	total_ids -= max_flows;
	map++;
	index++;

	/* Rest id16 map allocators except the last one */
	max_flows = flowmgr->flows_per_set;
	while (total_ids && index < (BCMPCIE_IDMA_H2D_MAX_DESC-1)) {
		if (total_ids < max_flows)
			max_flows = total_ids;

		map->max = max_flows;
		map->flow_id16_map = id16_map_init(dhdp->osh, max_flows, ID16_UNDEFINED);
		if (map->flow_id16_map == NULL) {
			DHD_ERROR(("%s: idma maps[%d] init failure\n",
				__FUNCTION__, index));
			goto error_rtn;
		}
		flowmgr->map_max = map;
		total_ids -= max_flows;
		map++;
		index++;
	}

	/* Last one */
	if (total_ids && index == (BCMPCIE_IDMA_H2D_MAX_DESC-1)) {
		max_flows = total_ids;
		map->max = max_flows;
		map->flow_id16_map = id16_map_init(dhdp->osh, max_flows, ID16_UNDEFINED);
		if (map->flow_id16_map == NULL) {
			DHD_ERROR(("%s: idma maps[%d] init failure\n",
				__FUNCTION__, index));
			goto error_rtn;
		}
		flowmgr->map_max = map;
		total_ids -= max_flows;
	}

	/* Because the id16 is stack system, and I want BCMC id stay at low.
	 * Allocate in reverse order.
	 */
	map = flowmgr->map_max;
	while (TRUE) {
		max_flows = map->max;
		while (max_flows) {
			if (dhd_idma_flowring_alloc(flowmgr, map) == BCME_ERROR) {
				DHD_ERROR(("Failed to allocate %d rings\n", max_flows));
				goto error_rtn;
			}
			max_flows--;
		}

		if (map == flowmgr->maps)
			break;
		map--;
	}

	/* Dump each map available count */
	map = flowmgr->maps;
	while (TRUE) {
		DHD_INFO(("%s: map @ %p avail %d\n", __FUNCTION__, map, map->avail));
		if (map == flowmgr->map_max)
			break;
		map++;
	}

	/* By now the flow_ids_map should be empty */
	flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);
	dhdp->flowid_allocator = flowmgr;

	return flowmgr;

error_rtn:

	dhd_idma_flowmgr_fini(dhdp, flowmgr);
	return NULL;
}

void *
dhd_idma_flowmgr_fini(dhd_pub_t *dhdp, void *mgr)
{
	dhd_idma_flowmgr_t *flowmgr;
	dhd_idma_map_t *map;

	flowmgr = (dhd_idma_flowmgr_t*)mgr;
	if (!flowmgr)
	    goto done;

	flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);

	/* Free iDMA maps */
	map = flowmgr->maps;
	while (TRUE) {
		map->flow_id16_map = id16_map_fini(dhdp->osh, map->flow_id16_map);
		if (map == flowmgr->map_max)
			break;
		map++;
	}

	/* Free iDMA flow ring mgr */
	MFREE(dhdp->osh, mgr, sizeof(dhd_idma_flowmgr_t));
done:
	return NULL;
}

uint16
dhd_idma_flowmgr_alloc(void *mgr, uint8 *mac)
{
	uint16 flow_id = ID16_INVALID;
	dhd_idma_map_t *map;
	dhd_idma_flowmgr_t *flowmgr = (dhd_idma_flowmgr_t *)mgr;

	ASSERT(mgr != NULL);

	if (ETHER_ISMULTI(mac)) {
		/* Get from the first map */
		map = flowmgr->maps;
		while (TRUE) {
			if (map->avail) {
				flow_id = id16_map_alloc(map->flow_id16_map);
				ASSERT(flow_id != ID16_INVALID);
				map->avail--;
				DHD_INFO(("%s: BCMC : Get flow id %d from map @ %p avail %d\n",
					__FUNCTION__, flow_id, map, map->avail));
				break;
			}

			if (map == flowmgr->map_max)
				break;

			map++;
		}
	} else {
		int active = -1;
		dhd_idma_map_t *sel_map = NULL;

		/* Search from the second map */
		map = (!flowmgr->maps[1].flow_id16_map) ?
			&flowmgr->maps[0] : &flowmgr->maps[1];
		while (TRUE) {
			if (map->avail && (map->max - map->avail) > active) {
				active = map->max - map->avail;
				sel_map = map;
			}

			if (map == flowmgr->map_max)
				break;

			map++;
		}

		/* Try first map if all of map id are running out. */
		if (!sel_map && flowmgr->maps[0].avail)
			sel_map = &flowmgr->maps[0];

		if (sel_map) {
			flow_id = id16_map_alloc(sel_map->flow_id16_map);
			ASSERT(flow_id != ID16_INVALID);
			sel_map->avail--;
			DHD_INFO(("%s: UCAST : Get flow id %d from map @ %p avail %d\n",
				__FUNCTION__, flow_id, sel_map, sel_map->avail));
		}
	}

	return flow_id;
}

void
dhd_idma_flowmgr_free(void *mgr, uint16 flow_id)
{
	uint16 index;
	dhd_idma_map_t *map;
	dhd_idma_flowmgr_t *flowmgr = (dhd_idma_flowmgr_t *)mgr;

	ASSERT(mgr != NULL);

	index = flow_id / flowmgr->flows_per_set;
	if (index >= BCMPCIE_IDMA_H2D_MAX_DESC)
		index = BCMPCIE_IDMA_H2D_MAX_DESC - 1;

	map = &flowmgr->maps[index];
	ASSERT(map->flow_id16_map != NULL);
	if (map->flow_id16_map == NULL)
		return;

	id16_map_free(map->flow_id16_map, flow_id);
	map->avail++;
	DHD_INFO(("Free flowid %d to idma map @ %p\n", flow_id, map));
}

/* See bcmpcie.h */
uint16
dhd_idma_flowmgr_ringid_2_index(dhd_pub_t *dhdp, uint16 ringid, uint16 flow_id)
{
	uint16 uindex;

	/* D2H:: CtrlCpl:2 TxCpl:3 RxCpl:4 */
	if (ringid >= FLOW_RING_COMMON &&
		ringid < BCMPCIE_COMMON_MSGRINGS) {
		return BCMPCIE_IDMA_D2H_COMMON_INDEX;
	}

	/* H2D::CtrlPost:0, RxPost:1, TxFlowRing2..N */
	/* Partition per 32 flow ring indices for rest flow rings */
	ASSERT(IDMA_FLOWS_PER_SET(dhdp));
	uindex = flow_id / IDMA_FLOWS_PER_SET(dhdp);
	uindex += BCMPCIE_IDMA_H2D_COMMON_INDEX;

	if (uindex >= BCMPCIE_IDMA_MAX_DESC)
		uindex = BCMPCIE_IDMA_MAX_DESC-1;

	return uindex;
}

#if defined(BCM_DHD_RUNNER)

static INLINE void *
dhd_flowid_map_init(dhd_pub_t *dhdp, int num_flow_rings)
{
	int max_bss;

#if defined(BCM_DHD_DNGL_MAXIFS)
	/* Use bss advertized by the dongle/default from bus layer */
	max_bss = dhd_bus_max_interfaces(dhdp->bus);
#else /* !BCM_DHD_DNGL_MAXIFS */
	max_bss = 8;

	/* Dongle should advertize max_bss */
	/* Temporary fix until dongle advertizes supported max_bss */
	if (BCM43602_CHIP(dhdp->bus->sih->chip))
		max_bss = 4;
	else if (BCM43684_CHIP(dhdp->bus->sih->chip))
		max_bss = 16;
#endif /* !BCM_DHD_DNGL_MAXIFS */

	return dhd_runner_flowmgr_init(dhdp->runner_hlp, num_flow_rings, max_bss);
}

static INLINE void *
dhd_flowid_map_fini(dhd_pub_t *dhdp, void *flowid_map)
{
	return dhd_runner_flowmgr_fini(dhdp->runner_hlp, flowid_map);
}

static INLINE uint16
dhd_flowid_map_alloc(dhd_pub_t *dhdp, void *flowid_map,
	uint8 ifindex, uint8 prio, char *sa, char *da)
{
	bool is_hw_ring = FALSE;
	bool is_11ac = TRUE;    /* dongle should give this information */
	uint16 flowid;

	/* Runner manages ucast pools by access category */
	if (dhdp->flow_prio_map_type == DHD_FLOW_PRIO_TID_MAP)
		prio = prio2ac[prio];

	flowid = dhd_runner_flowmgr_alloc(flowid_map,
		ifindex, prio, da, is_11ac, &is_hw_ring);

	if (flowid != FLOWID_INVALID) {
		flow_ring_node_t *flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		DHD_FLOWRING_SET_RNR_OFFL(flow_ring_node, is_hw_ring);
	}

	return flowid;
}

static INLINE void
dhd_flowid_map_free(dhd_pub_t *dhdp, void *flowid_map, uint16 flowid)
{
	dhd_runner_flowmgr_free(flowid_map, flowid);
}

#else /* !BCM_DHD_RUNNER */

static INLINE void *
dhd_flowid_map_init(dhd_pub_t *dhdp, int num_flow_rings)
{
	if (IDMA_ACTIVE(dhdp))
		return dhd_idma_flowmgr_init(dhdp, num_flow_rings);
	else
		return id16_map_init(dhdp->osh,
			num_flow_rings - FLOW_RING_COMMON, FLOWID_RESERVED);
}

static INLINE void *
dhd_flowid_map_fini(dhd_pub_t *dhdp, void *flowid_map)
{
	if (IDMA_ACTIVE(dhdp))
		return dhd_idma_flowmgr_fini(dhdp, flowid_map);
	else
		return id16_map_fini(dhdp->osh, flowid_map);
}

static INLINE uint16
dhd_flowid_map_alloc(dhd_pub_t *dhdp, void *flowid_map,
	uint8 ifindex, uint8 prio, char *sa, char *da)
{
	if (IDMA_ACTIVE(dhdp))
		return dhd_idma_flowmgr_alloc(flowid_map, da);
	else
		return id16_map_alloc(flowid_map);
}

static INLINE void
dhd_flowid_map_free(dhd_pub_t *dhdp, void *flowid_map,
	uint16 flowid)
{
	if (IDMA_ACTIVE(dhdp))
		dhd_idma_flowmgr_free(flowid_map, flowid);
	else
		id16_map_free(flowid_map, flowid);
}

#endif /* !BCM_DHD_RUNNER */
