/**
 * @file Broadcom Dongle Host Driver (DHD), Idle Flowring Eviction specific code
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_ife.c sg944736 $
 */

#include <typedefs.h>

#include <dhd_ife.h>
#include <dhd_dbg.h>
#include <pcie_core.h>
#include <dhd_pcie.h>
#include <dhd_linux.h>
#if defined(BCM_DHD_RUNNER)

#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */
#ifdef BCM_BLOG
#include <dhd_blog.h>
#endif /* BCM_BLOG */
#ifdef BCM_PKTFWD
#include <dhd_pktfwd.h>
#endif /* BCM_PKTFWD */

#define DHD_IFE_DEFAULT_TIMEOUT		25000	// 25 sec
#define DHD_IFE_DEFAULT_FLRING_BUDGET	10	// Max no of flowrings to iterate
						// on each timer invocation
typedef struct dhd_ife_info {
	struct timer_list   ife_timer;	/* Timer struct */
	uint32		    ife_intvl;	/* Timer interval */
	uint32		    budget;	/* No of flowrings to iterate */
	dhd_pub_t           *dhdp;	/* dhd pub */
	dll_t		    *iter_start; /* Pointer for iteration start */
} dhd_ife_info_t;

extern uint16
dhd_msgbuf_get_curr_idx(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node,
        uint16 *curr_rd_idx, uint16 *curr_wr_idx);
static void
_dhd_ife_evict_timeout(dhd_pub_t *dhdp);

static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
dhd_ife_evict_timeout(ulong data);
#else
dhd_ife_evict_timeout(struct timer_list *t);
#endif /* LINUX_VERSION_CODE */

/*
 * Initialize the ife specific data structures
 *      Allocate ife private structure
 *      Initialize IFE timer
 */
int
dhd_ife_init(dhd_pub_t *dhdp)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info =  (dhd_ife_info_t*) MALLOCZ(dhdp->osh, sizeof(dhd_ife_info_t));

	if (ife_info == NULL) {
		DHD_ERROR(("Failed to Allocate IFE structures\n"));
		return BCME_ERROR;
	}

	ife_info->ife_intvl = DHD_IFE_DEFAULT_TIMEOUT;
	ife_info->budget = DHD_IFE_DEFAULT_FLRING_BUDGET;
	ife_info->iter_start = NULL;
	/* Set up the ife timer */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&ife_info->ife_timer);
	ife_info->ife_timer.data = (ulong)dhdp;
	ife_info->ife_timer.function = dhd_ife_evict_timeout;
#else
	timer_setup(&ife_info->ife_timer, dhd_ife_evict_timeout, 0);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */

	ife_info->dhdp = dhdp;
	dhdp->ife_info = ife_info;
	return BCME_OK;
}

/* Deinit IFE - Delete IFE timer and free up data structure */
int
dhd_ife_deinit(dhd_pub_t *dhdp)
{
	dhd_ife_info_t *ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	del_timer_sync(&ife_info->ife_timer);
	MFREE(dhdp->osh, ife_info, sizeof(dhd_ife_info_t));
	dhdp->ife_info = NULL;

	return BCME_OK;
}

/* Module specific book keeping during flowring creation */
void
dhd_ife_flowring_create(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node)
{
	dhd_ife_info_t *ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	flow_ring_node->evict_inprogress = FALSE;

	if (!ife_info->iter_start)
		return;

	ife_info->iter_start = dll_prev_p(ife_info->iter_start);
}

/* Module specific book keeping during flowring deletion */
void
dhd_ife_flowring_delete(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node)
{
	dhd_ife_info_t *ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	flow_ring_node->evict_inprogress = FALSE;

	if (ife_info->iter_start == &flow_ring_node->list) {
		ife_info->iter_start = dll_next_p(ife_info->iter_start);
	}
}

/* Timer callback function */
static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
dhd_ife_evict_timeout(ulong data)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)data;
	dhd_ife_info_t *ife_info = dhdp->ife_info;
#else
dhd_ife_evict_timeout(struct timer_list *t)
{
	dhd_ife_info_t *ife_info = from_timer(ife_info, t, ife_timer);
	dhd_pub_t *dhdp = ife_info->dhdp;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */

	ASSERT(ife_info);

	if (dhdp->dongle_reset || dhd_is_device_removed(dhdp)) {
		return;
	}

	_dhd_ife_evict_timeout(dhdp);

	/* Reschedule the timer again */
	mod_timer(&ife_info->ife_timer, jiffies +
		msecs_to_jiffies(ife_info->ife_intvl));
}

/* Start IFE timer - should be called after first flow ring is created */
void
dhd_ife_evict_timer_start(dhd_pub_t *dhdp)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	mod_timer(&ife_info->ife_timer, jiffies +
		msecs_to_jiffies(ife_info->ife_intvl));
}

/* IOVAR to tune eviction timer */
uint32
dhd_ife_get_timer_interval(dhd_pub_t *dhdp)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	return ife_info->ife_intvl;
}

void
dhd_ife_set_timer_interval(dhd_pub_t *dhdp, uint32 intvl)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	/* Don't allow to go below default threshold */
	if (intvl < DHD_IFE_DEFAULT_TIMEOUT)
		ife_info->ife_intvl = DHD_IFE_DEFAULT_TIMEOUT;
	else
		ife_info->ife_intvl = intvl;
}

/* IOVAR to tune eviction budget */
uint32
dhd_ife_get_budget(dhd_pub_t *dhdp)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	return ife_info->budget;
}

void
dhd_ife_set_budget(dhd_pub_t *dhdp, uint32 budget)
{
	dhd_ife_info_t *ife_info = NULL;

	ife_info = dhdp->ife_info;

	ASSERT(ife_info);

	ife_info->budget = budget;
}

/*
 * Evict a particular flow ring node and notify Runner/Blog layer
 */
static int
dhd_ife_flowring_evict(dhd_pub_t *dhdp, flow_ring_node_t *flow_ring_node)
{
	int ret = BCME_OK;

#if defined(BCM_BLOG)
	/* Sync wr index managed by runner */
	dhd_blog_flush_flowring(dhdp, flow_ring_node->flowid);
#endif /* BCM_BLOG */
#if defined(BCM_DHD_RUNNER)
	if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
		uint16 curr_wr_idx, curr_rd_idx;
		uint16 evict_wr_idx = flow_ring_node->evict_wr_idx;

		/*
		 * Flow ring is already enabled in the runner
		 * Disable it and flush any outstanding packets on that ring
		 */
		dhd_runner_notify(dhdp->runner_hlp,
			H2R_FLRING_DISAB_NOTIF, flow_ring_node->flowid, 0);

		dhd_msgbuf_get_curr_idx(dhdp, flow_ring_node, &curr_rd_idx, &curr_wr_idx);

		if (unlikely(curr_wr_idx != evict_wr_idx)) {
			/* Enable back the runner flow and abort eviction */
			dhd_runner_notify(dhdp->runner_hlp,
				H2R_FLRING_ENAB_NOTIF, flow_ring_node->flowid, 0);
			return BCME_OK;
		}
	}
#endif /* BCM_DHD_RUNNER */
	/* Send Delete request to Dongle - grabs flowing lock */
	ret = dhd_bus_flow_ring_delete_request(dhdp->bus, flow_ring_node);

	if (ret == BCME_OK) {
#ifdef BCM_PKTFWD
		if (PKTFWD_ENABLED(dhdp)) {
			uint32 ifidx = flow_ring_node->flow_info.ifindex;
			uint32 staidx =
				dhd_if_get_staidx(dhdp, ifidx, flow_ring_node->flow_info.da);

			/* Reset pktfwd mapping
			 * This is also being done in dhd_del_flowid(). But we
			 * may skip calling dhd_del_flowid if we see pkts in
			 * the queue during delete response.
			 */
			dhd_pktfwd_reset_keymap(dhdp->unit,
				DHD_STAIDX2LUTID(staidx), flow_ring_node->flowid,
				flow_ring_node->flow_info.tid);
		}
#endif /* BCM_PKTFWD */
		flow_ring_node->evict_inprogress = TRUE;
	}

	return ret;
}

static void
_dhd_ife_evict_timeout(dhd_pub_t *dhdp)
{
	dll_t *item = NULL, *next = NULL;
	struct dhd_bus *bus = dhdp->bus;
	dhd_ife_info_t *ife_info = dhdp->ife_info;
	uint16 curr_wr_idx, curr_rd_idx, evict_wr_idx;
	flow_ring_node_t *flow_ring_node;
	uint16 ifidx;
	int budget = 0;

	ASSERT(ife_info);

	DHD_LOCK(dhdp);

	/* Update start pointer with the first flowring node */
	if (ife_info->iter_start == NULL && !dll_empty(&bus->const_flowring))
		ife_info->iter_start = dll_head_p(&bus->const_flowring);

	if (!ife_info->iter_start) {
		/* Flow not created yet */
		DHD_UNLOCK(dhdp);
		return;
	}

	/* Iterate over budget no of flowrings
	 * const_flowring is managed in dhd_pcie.c
	 */
	for (item = ife_info->iter_start;
		(dll_prev_p(ife_info->iter_start) != item) && (budget < ife_info->budget);
		item = next) {

		next = dll_next_p(item);
		if (dll_end(&bus->const_flowring, item)) {
			continue;
		}

		flow_ring_node = dhd_constlist_to_flowring(item);
		ifidx = flow_ring_node->flow_info.ifindex;

		/*
		 * Eviction applicable only for
		 *   AP interface
		 *   Unicast flowrings
		 *   Deletion not in progress
		 */
		if ((dhd_flow_rings_ifindex2role(dhdp, ifidx) != WLC_E_IF_ROLE_AP) ||
			ETHER_ISMULTI(flow_ring_node->flow_info.da)||
			(flow_ring_node->status == FLOW_RING_STATUS_DELETE_PENDING)) {
			continue;
		}

		budget++;
		evict_wr_idx = flow_ring_node->evict_wr_idx;

		dhd_msgbuf_get_curr_idx(dhdp, flow_ring_node, &curr_rd_idx, &curr_wr_idx);

		/* Conditions for Eviction -
		 * Nothing written since last time it fired
		 * RD = WR implies dongle has not stopped fetching
		 * BKP Q is empty
		 */
		if ((curr_wr_idx == evict_wr_idx) && (curr_wr_idx == curr_rd_idx) &&
			DHD_FLOW_QUEUE_EMPTY(&flow_ring_node->queue)) {
			char *da;
			da = flow_ring_node->flow_info.da;
			/* Candidate for eviction */
			DHD_INFO(("[IFE::] Evict flring %d {%02x:%02x:%02x:%02x:%02x:%02x tid %d} "
				    "wridx [curr %d last %d]\n",
				    flow_ring_node->flowid,
				    da[0], da[1], da[2], da[3], da[4], da[5],
				    flow_ring_node->flow_info.tid,
				    curr_wr_idx, evict_wr_idx));
			if (BCME_OK != dhd_ife_flowring_evict(dhdp, flow_ring_node)) {
				DHD_ERROR(("Failed to evict flowid %d. Will be retried",
					flow_ring_node->flowid));
				break;
			}
		} else {
			flow_ring_node->evict_wr_idx = curr_wr_idx;
		}
	}

	ife_info->iter_start = dll_end(&bus->const_flowring, next) ?
		dll_head_p(&bus->const_flowring) : next;

	DHD_UNLOCK(dhdp);
}
