/*
 * Broadcom Dongle Host Driver (DHD) - Low Bit Rate Aggregation
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
 * $Id$
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <epivers.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <ethernet.h>
#include <bcmevent.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhd_aggr.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void dhd_aggr_release_timer(struct timer_list *data);
#else
static void dhd_aggr_release_timer(unsigned long data);
#endif
static void dhd_aggr_free_aggregator(dhd_pub_t *, dhd_aggr_info_t *, dhd_aggregator_t *);
static void dhd_aggr_release(dhd_pub_t *dhdp, dhd_aggregator_t *aggr);

#define DHD_AGGR_PKTQ_GETNEXT(p)     PKTLINK(p)
#define DHD_AGGR_PKTQ_SETNEXT(p, x)  PKTSETLINK((p), (x))

/**
 * Init aggregation specific data structure
 *    - for keeping track of active and free aggregators
 */

int
dhd_aggr_init(dhd_pub_t *dhdp)
{
	dhd_aggr_info_t *aggr_info = NULL;
	dhd_aggrlist_t *aggr_table = NULL;
	dhd_aggregator_t *inst = NULL;
	int idx;

	aggr_info =  (dhd_aggr_info_t *) MALLOCZ(dhdp->osh, sizeof(dhd_aggr_info_t));
	if (aggr_info == NULL)
		goto fail;
	aggr_info->table_sz = DHD_AGGR_DEFAULT_TABLE_SIZE;

	aggr_table = (dhd_aggrlist_t *) MALLOCZ(dhdp->osh,
			sizeof(dhd_aggrlist_t) * aggr_info->table_sz);
	aggr_info->aggr_table = aggr_table;
	if (aggr_table == NULL) {
		goto fail;
	}
	for (idx = 0; idx < aggr_info->table_sz; idx++) {
		dll_init(&aggr_table[idx].list);
	}

	/* allocate a pool of aggregators pointed by free_pool */
	dll_init(&aggr_info->free_pool.list);
	for (idx = 0; idx < DHD_AGGR_DEFAULT_POOL_SIZE; idx++) {
		inst = (dhd_aggregator_t *) MALLOCZ(dhdp->osh, sizeof(dhd_aggregator_t));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		timer_setup(&inst->release_timer, dhd_aggr_release_timer, 0);
#else
		init_timer(&inst->release_timer);
#endif
		inst->lock = dhd_os_spin_lock_init(dhdp->osh);
		dll_append(&aggr_info->free_pool.list, &inst->list);
	}

	aggr_info->lock =  dhd_os_spin_lock_init(dhdp->osh);
	dhdp->aggr_info = aggr_info;
	return BCME_OK;
fail:
	if (!aggr_table)
		MFREE(dhdp->osh, aggr_table,
			sizeof(dhd_aggrlist_t) * aggr_info->table_sz);
	if (!aggr_info) {
		MFREE(dhdp->osh, aggr_info, sizeof(dhd_aggr_info_t));
	}
	DHD_ERROR(("%s: dhd aggregation alloc failure\n", __FUNCTION__));
	return BCME_NOMEM;
}

/**
 * Deinit/free aggregation specific data structure
 *   can be called only once per dhd instance
 */

void
dhd_aggr_deinit(dhd_pub_t *dhdp)
{
	dhd_aggr_info_t *aggr_info = (dhd_aggr_info_t *)dhdp->aggr_info;
	dhd_aggregator_t *inst;
	int idx;
	dll_t *item, *next, *dlist;

	if (!aggr_info)
		return;

	for (idx = 0; idx < aggr_info->table_sz; idx++) {
		dlist = &aggr_info->aggr_table[idx].list;
		/*
		 * aggregators should be freed by callers - enforce policy in debug mode
		 */
		for (item = dll_head_p(dlist); !dll_end(dlist, item); item = next) {
			inst = container_of(item, dhd_aggregator_t, list);
			ASSERT(!"Aggregators should get deleted by caller before data deinit\n");
			next = dll_next_p(item);
			dhd_aggr_free_aggregator(dhdp, aggr_info, inst);
		}
	}

	dhd_os_spin_lock_deinit(dhdp->osh, aggr_info->lock);
	/* free aggregator memory by walking over free pool list */
	dlist = &aggr_info->free_pool.list;
	for (item = dll_head_p(dlist); !dll_end(dlist, item); item = next) {
		next = dll_next_p(item);
		inst = container_of(item, dhd_aggregator_t, list);
		/* timer is to be stopped in dhd_aggr_del_aggregator */
		del_timer(&inst->release_timer);
		dhd_os_spin_lock_deinit(dhdp->osh, inst->lock);
		MFREE(dhdp->osh, inst, sizeof(dhd_aggregator_t));
	}

	ASSERT(aggr_info->aggr_table != NULL);
	MFREE(dhdp->osh, aggr_info->aggr_table,
		sizeof(dhd_aggrlist_t) * aggr_info->table_sz);
	MFREE(dhdp->osh, aggr_info, sizeof(dhd_aggr_info_t));

	dhdp->aggr_info = 0;
}

/**
 * Search in hash-table for an active aggregator of specificed type and key
 */

dhd_aggregator_t *
dhd_aggr_find_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t aggr_type, uint16 key)
{
	dhd_aggr_info_t *aggr_info = (dhd_aggr_info_t *)dhdp->aggr_info;
	dhd_aggregator_t *inst  = NULL, *curp = NULL;
	dll_t *item, *next, *dlist;
	uint32 flags;
	int idx;

	if (!aggr_info) return NULL;

	idx = key%aggr_info->table_sz;

	flags = dhd_os_spin_lock(aggr_info->lock);

	dlist = &aggr_info->aggr_table[idx].list;

	if (!dlist)  goto unlock_and_return;

	for (item = dll_head_p(dlist); !dll_end(dlist, item); item = next) {
		next = dll_next_p(item);
		curp = container_of(item, dhd_aggregator_t, list);
		ASSERT(curp != NULL);
		if ((curp->type == aggr_type) && (curp->uid == key)) {
			inst = curp;
			break;
		}
	}
unlock_and_return:
	dhd_os_spin_unlock(aggr_info->lock, flags);
	return inst;
}

/**
 * Deinit specified aggregator and add it back to free pool
 *  - local helper funtion - caller needs to acquire locks to perform operation
 */
static void
dhd_aggr_free_aggregator(dhd_pub_t *dhdp, dhd_aggr_info_t *aggr_info, dhd_aggregator_t *inst)
{
	if (!inst)
		return;

	/* empty packet queue  through release function */
	dhd_aggr_release(dhdp, inst);
	/* remove from aggr_table */
	dll_delete(&inst->list);
	/* add to free_pool */
	dll_append(&aggr_info->free_pool.list, &inst->list);
	return;
}

static int
dhd_aggr_find_and_free_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t aggr_type, uint16 key)
{
	dhd_aggr_info_t *aggr_info = (dhd_aggr_info_t *)dhdp->aggr_info;
	dhd_aggregator_t *curp = NULL;
	dll_t *item, *next, *dlist;
	uint32 flags;
	int idx;
	int ret = BCME_ERROR;

	if (!aggr_info) return ret;

	idx = key%aggr_info->table_sz;

	flags = dhd_os_spin_lock(aggr_info->lock);

	dlist = &aggr_info->aggr_table[idx].list;

	if (!dlist)  goto unlock_and_return;

	for (item = dll_head_p(dlist); !dll_end(dlist, item); item = next) {
		next = dll_next_p(item);
		curp = container_of(item, dhd_aggregator_t, list);
		ASSERT(curp != NULL);
		DHD_ERROR(("%s: find and free aggregator type = %d, id = %d\n",
				__FUNCTION__, curp->type, curp->uid));
		if ((curp->type == aggr_type) && (curp->uid == key)) {
			dhd_aggr_free_aggregator(dhdp, aggr_info, curp);
			ret = BCME_OK;
			break;
		}
	}
unlock_and_return:
	dhd_os_spin_unlock(aggr_info->lock, flags);
	return ret;
}

/**
 * Add an aggregator entry with specified configuration
 */
dhd_aggregator_t *
dhd_aggr_add_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t aggr_type, uint16 key,
	dhd_aggr_cb_t release_cb, int release_timeout, int max_aggr, int cb_data)
{
	dhd_aggr_info_t *aggr_info = (dhd_aggr_info_t *)dhdp->aggr_info;
	dhd_aggrlist_t *aggr_table;
	dhd_aggregator_t *inst  = NULL;
	uint32 flags;
	dll_t *dlist, *item;
	int idx;

	if (!aggr_info) return NULL;

	aggr_table = (dhd_aggrlist_t *)aggr_info->aggr_table;
	if (!aggr_table) return NULL;

	if (!aggr_info->table_sz) return NULL;

	flags = dhd_os_spin_lock(aggr_info->lock);

	dlist = &aggr_info->free_pool.list;
	item = dll_head_p(dlist);
	if (dll_end(dlist, item))  {
		goto unlock_and_return;
	}
	inst = container_of(item, dhd_aggregator_t, list);
	dll_delete(&inst->list);
	ASSERT(inst != NULL);

	inst->dhdp = dhdp;
	inst->type = aggr_type;
	inst->uid = key;
	inst->release_cb = release_cb;
	inst->max_aggr = max_aggr;
	inst->cb_data = cb_data;

	if (release_timeout) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
		inst->release_timer.data = (ulong)inst;
#endif
		inst->release_timer.function = dhd_aggr_release_timer;
		inst->release_timer_active = FALSE;
		inst->release_time_msec = release_timeout;
		inst->timeren = TRUE;
	}
	else {
		inst->timeren = FALSE;
	}

	idx = key%aggr_info->table_sz;
	dlist = &aggr_table[idx].list;
	/* add instance to aggr_table at idx */
	dll_append(dlist, &inst->list);

unlock_and_return:
	dhd_os_spin_unlock(aggr_info->lock, flags);

	return inst;
}

/**
 * Del a previously configured aggregator
 */
void dhd_aggr_del_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t aggr_type, uint16 uid)
{
	dhd_aggr_info_t *aggr_info = (dhd_aggr_info_t *)dhdp->aggr_info;

	if (!aggr_info) return;

	if (dhd_aggr_find_and_free_aggregator(dhdp, aggr_type, uid) != BCME_OK) {
		DHD_INFO(("%s: cannot find aggregator type = %d, id = %d\n",
			__FUNCTION__, aggr_type, uid));
		return;
	}
}

/**
 * Handle release event (timeout or aggregation limit)
 */
static void BCMFASTPATH
dhd_aggr_release(dhd_pub_t *dhdp, dhd_aggregator_t *inst)
{
	void *pkt;

	if (!inst)
		return;

	ASSERT(inst->pktqlen >= 0);

	if (inst->pktqlen == 0)
		return;

	/* send all previously accumulated packets */
	while (inst->pktqlen > 0) {
		/* get packet from queue */
		pkt = inst->pktqhead;
		inst->pktqhead = DHD_AGGR_PKTQ_GETNEXT(pkt);
		DHD_AGGR_PKTQ_SETNEXT(pkt, NULL);
		inst->release_cb(dhdp, inst->cb_data, pkt);
		inst->pktqlen--;
	}

	ASSERT(inst->pktqlen == 0);

	inst->pktqhead = NULL;
	inst->pktqtail = NULL;
}

/**
 * Handle request to accumulate a packet
 */
void BCMFASTPATH
dhd_aggr_accumulate(dhd_pub_t *dhdp, dhd_aggregator_t *inst, void *pkt)
{
	uint32 flags = 0;

	ASSERT(inst != NULL);

	if (inst->timeren) {
		flags = dhd_os_spin_lock(inst->lock);
	}
	if (!inst->pktqhead) {
		inst->pktqhead = pkt;
	} else {
		DHD_AGGR_PKTQ_SETNEXT(inst->pktqtail, pkt);
	}
	/* set this packet as the last packet */
	DHD_AGGR_PKTQ_SETNEXT(pkt, NULL);
	inst->pktqtail = pkt;

	inst->pktqlen++;

	if (inst->pktqlen >= inst->max_aggr) {
		dhd_aggr_release(dhdp, inst);
	}

	if (inst->timeren) {
		dhd_os_spin_unlock(inst->lock, flags);
	}
}

/**
 * dhd_aggr_intercept
 *  returns FALSE if packet cannot be intercepted.
 */

bool BCMFASTPATH
dhd_aggr_intercept(dhd_pub_t *dhdp, dhd_aggregator_t *inst, void *pktbuf)
{
	uint32 flags;

	if (inst == NULL)
		return FALSE;

	/* intercept packets from datapath */
	dhd_aggr_accumulate(dhdp, inst, pktbuf);

	/* rearm the timer if timed release is configured */
	if (inst->timeren && (inst->release_timer_active == FALSE)) {
		flags = dhd_os_spin_lock(inst->lock);
		mod_timer(&inst->release_timer,
				jiffies + msecs_to_jiffies(inst->release_time_msec));
		inst->release_timer_active = TRUE;
		dhd_os_spin_unlock(inst->lock, flags);
	}

	return TRUE;
}

/**
 * Handle release timeout event
 */
static
void
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
dhd_aggr_release_timer(struct timer_list *data)
#else
dhd_aggr_release_timer(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	dhd_aggregator_t *inst = from_timer(inst, data, release_timer);
#else
	dhd_aggregator_t *inst = (dhd_aggregator_t *)data;
#endif
	dhd_pub_t *dhdp = inst->dhdp;
	uint32 flags;

	flags = dhd_os_spin_lock(inst->lock);
	dhd_aggr_release(dhdp, inst);

	/* request rearming of timer */
	inst->release_timer_active = FALSE;

	dhd_os_spin_unlock(inst->lock, flags);
}
