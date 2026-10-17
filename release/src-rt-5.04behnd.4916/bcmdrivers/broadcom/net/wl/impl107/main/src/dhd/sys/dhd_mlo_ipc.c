/*
 * DHD_MLO_IPC: Inter Processor communication between MLO AP's
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: dhd_mlo_ipc.c 852834 2025-02-28 21:16:00Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <pcicfg.h>
#include <pcie_core.h>
#include <mlo_ipc.h>
#include <wl_dbg.h>
#include <dhd.h>
#include <dhd_proto.h>
#include <dhd_pcie.h>
#include <dhd_dbg.h>
#include <dhd_mlo_ipc.h>
#include <dhd_macdbg.h>
#include <dhd_linux.h>

#if 0 && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
#define DEVTREE_CONFIGURED_PCIE_MLC_BRIDGE
#endif

#if defined(MLO_IPC)

#define MLO_IPC_PREFIX	IN_GREEN"[MLO::IPC] "
#define MLO_IPC_POSTFIX	IN_NORMAL"\n"

#define DHD_MLO_IPC_EVT_QUEUE_BUDGET	64

#ifndef AP_UNIT_INV
#define AP_UNIT_INV	PCIE_IPC_AP_UNIT_INV
#endif

#define MLO_GR_MAXWAIT	3

// MAP object
typedef struct dhd_mlo_ipc_map {
	mlo_ipc_msg_t	msg;	// Copy of State Transition request in progress
	uint8	mlc_state;	// Current FSA State
	uint8	next_state;	// FSA pending state
	uint8	gr_req_ap_unit;	// Graceful Recovery requesting AP unit
	uint8   next_mlo_down_ap_bm; //next mlo_down update for all peers
} dhd_mlo_ipc_map_t;

typedef struct dhd_mlo_ipc {
	pcie_ipc_mlo_ap_t	* pcie_ipc_mlo_ap;	// MLO attributes of AP
	struct task_struct      * kthread;
	wait_queue_head_t	wq_head;		// Linux wait_event queue

	dhd_mlo_ipc_map_t	* map;

	uint8	wq_event_msg_queue;		// wait_event conditional for msg_queue
	uint8	wq_event_state_change;		// wait_event conditional for state change
	uint8	wq_event_evt_queue;		// wait_event conditional for mlo event
	uint8	req_mlc_state;			// State Transition to be processed
	uint8	ap_unit;
	uint8	mlc_state;			// Current FSA state
	bool	suspended;			// MLO in SUSPEND state
	bool	ioc_pending;			// IOVAR requested when MLO_IPC_DOWN in progress
	bool	wl_down_pending;		// WL down iovar waiting for MLO_DOWN to complete
	uint	ioc_complete;			/* Wake up event on MLO_IPC (DOWN/BH) complete */
	uint8	iov_defer;			/* defer iovar processing on interface */
#if !defined(DHD_USE_PERIM)
	struct	semaphore	lock;
#endif /* !DHD_USE_PERIM */
} dhd_mlo_ipc_t;

#define DHD_MLO_IPC(dhd_pub)		((dhd_mlo_ipc_t *)((dhd_pub)->dhd_mlo_ipc))
#define DHD_IS_MLO_AP(dhd_pub)		(DHD_MLO_IPC(dhd_pub) != NULL)
#define DHD_IS_MLO_MAP(dhd_pub)		(DHD_MLO_IPC(dhd_pub)->map != NULL)

#if !defined(DHD_USE_PERIM)
#define DHD_MLO_IPC_LOCK_INIT(mlo_ipc)	sema_init(&mlo_ipc->lock, 1)
#define DHD_MLO_IPC_LOCK(mlo_ipc)	if (CAN_SLEEP())	down(&mlo_ipc->lock)
#define DHD_MLO_IPC_UNLOCK(mlo_ipc)	if (CAN_SLEEP())	up(&mlo_ipc->lock)
#else
#define DHD_MLO_IPC_LOCK_INIT(mlo_ipc)
#define DHD_MLO_IPC_LOCK(mlo_ipc)
#define DHD_MLO_IPC_UNLOCK(mlo_ipc)
#endif /* !DHD_USE_PERIM */

// Forward Declarations
static int	dhd_mlo_ipc_kthread_fn(void * data);

static void	dhd_mlo_ipc_map_msg_enqueue(int ap_unit, uint8 msg_id, uint8 msg_prio,
                                            uint8 msg_unit, void * data);
static void	dhd_mlo_ipc_map_msg_queue_process(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc);
static void	dhd_mlo_ipc_aap_state_change_cb(int ap_unit, uint8 mlc_state, void * data);
static int	dhd_mlo_ipc_process_state_change(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc,
                                                 uint8 mlc_state);
static void	dhd_mlo_ipc_evt_queue_process(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc,
                                              bool bound);
static void	dhd_mlo_ipc_evt_wake_up_cb(void *data);
static int	dhd_mlo_ipc_wlioctl(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t *dhd_mlo_ipc,
                                    uint8 mlc_state);
static int	dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc_t * dhd_mlo_ipc,
                                          mlo_ipc_mlc_state_t mlc_state);
static int	dhd_mlo_ipc_enqueue_evt_iov_msg(dhd_pub_t *dhd_pub, uint8 dest_ap_unit,
                                                mlo_ipc_iov_msg_t *iov_msg);
static int	dhd_mlo_ipc_bcast_update_iov_defer(dhd_pub_t *dhd_pub, int8 mld_unit,
                                                   uint msg_subtype);
static void	dhd_mlo_handle_role_reconfig(dhd_pub_t *dhd_pub, uint8 new_mlo_unit);
static int	dhd_mlo_ipc_restart(void *data);
static int	dhd_mlo_ipc_generic_cb(mlo_ipc_cb_t cb, void * data, int ap_unit,
                                       uint32 param1, uint32 param2, uint32 param3);
static void	dhd_mlo_ipc_schedule_state_change(dhd_pub_t *dhd_pub);
/*
 * ------------------------------------------------------------------------------------------------
 *  Initialize dhd_mlo_ipc objects
 *  - Configure per AP PCIE IPC MLO attributes
 *  - Create a wait_event and kthread for dhd_mlo_ipc
 * ------------------------------------------------------------------------------------------------
 */

static int // Alloc and Initialize MAP object
dhd_mlo_ipc_map_init(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc)
{
	dhd_mlo_ipc_map_t * map;

	map = MALLOCZ(dhd_pub->osh, sizeof(dhd_mlo_ipc_map_t));
	if (map == NULL) {
		DHD_ERROR(("%s: malloc failed; bytes %u\n",
			__FUNCTION__, (uint)sizeof(dhd_mlo_ipc_map_t)));
		return BCME_NORESOURCE;
	}

	map->mlc_state = mlo_ipc_mlc_attach_e;
	map->msg.id = ~0;
	map->msg.prio = ~0;
	map->msg.req_ap_bm = 0;
	map->gr_req_ap_unit = (uint8)AP_UNIT_INV;

	dhd_mlo_ipc->map = map;

	return BCME_OK;
} // dhd_mlo_ipc_map_init()

int
dhd_mlo_ipc_init(dhd_pub_t * dhd_pub)
{
	mlo_ipc_sys_t   * mlo_ipc_sys;
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	int	ap_unit;
	int	mlo_unit;
	int	ret;
	char	kthread_name[32] = {0};
	int	len;

	ret = BCME_OK;
	ap_unit = dhd_pub->unit;
	mlo_ipc_sys = mlo_ipc_sys_gp;
	mlo_unit = PCIE_IPC_MLO_UNIT_INV;

	if ((mlo_ipc_sys != NULL) && (mlo_ipc_sys->pcie_ipc_mlo != NULL)) {
		mlo_unit = mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit];
	}

	DHD_TRACE(("%s: ENTER ap_unit[%d] mlo_unit[%d]\n", __FUNCTION__, ap_unit, mlo_unit));

	if (mlo_unit == PCIE_IPC_MLO_UNIT_INV) {
		// No further actions for Non-MLO AP
		return BCME_OK;
	}

	ASSERT(DHD_MLO_IPC(dhd_pub) == NULL);

	len = sizeof(dhd_mlo_ipc_t);

	dhd_mlo_ipc = MALLOCZ(dhd_pub->osh, len);
	if (dhd_mlo_ipc == NULL) {
		DHD_ERROR(("%s: dhd%d: Malloc failed; bytes %u\n",
			__FUNCTION__, ap_unit, (uint)len));
		return BCME_NOMEM;
	}

	dhd_pub->dhd_mlo_ipc = (void *)dhd_mlo_ipc;

	{ // Configure the RO pcie_ipc_mlo::ap[ap_unit]
		pcie_ipc_mlo_ap_t * pcie_ipc_mlo_ap;
		int pcie_gen, pcie_lane;
		uint16 pcie_unit;

		pcie_ipc_mlo_ap = &mlo_ipc_sys->pcie_ipc_mlo->ap[ap_unit];

		pcie_ipc_mlo_ap->mlo_unit       = (int8)mlo_unit;
		pcie_ipc_mlo_ap->ap_unit        = (uint8)ap_unit;
		pcie_ipc_mlo_ap->cpu_unit       = (uint8)ap_unit;

		pcie_unit = ap_unit;
		// Get PCIe Gen and Lanes
		dhdpcie_bus_get_pcie_gen_lane(dhd_pub, &pcie_gen, &pcie_lane);
		/* PCIe unit(b15..b8), gen(b7..b4), lane(b3..b0) */
		pcie_ipc_mlo_ap->bus_info	= (uint16)(((pcie_lane & 0xF) << 0) |
				((pcie_gen & 0xF) << 4) | ((pcie_unit & 0xFF) << 8));
		pcie_ipc_mlo_ap->chip_id        = (uint32) dhd_bus_chip_id(dhd_pub);

		/* Get Bar-2 mapped address and size */
		pcie_ipc_mlo_ap->bar2_haddr64 = dhdpcie_bus_cfg_set_mlo_win(dhd_pub);
		if (HADDR64_IS_ZERO(pcie_ipc_mlo_ap->bar2_haddr64)) {
			DHD_ERROR(("%s: dhd%d: BAR2 unsupport\n", __FUNCTION__, ap_unit));
			ret = BCME_UNSUPPORTED;
			goto dhd_mlo_ipc_init_fail;
		}

		dhd_mlo_ipc->pcie_ipc_mlo_ap    = pcie_ipc_mlo_ap;
	}

	// Create a wait_event queue
	dhd_mlo_ipc->wq_event_msg_queue = 0;
	dhd_mlo_ipc->wq_event_state_change = 0;
	dhd_mlo_ipc->wq_event_evt_queue = 0;
	init_waitqueue_head(&dhd_mlo_ipc->wq_head);
	DHD_MLO_IPC_LOCK_INIT(dhd_mlo_ipc);

	// Create kthread for dhd_mlo_ipc
	sprintf(kthread_name, "dhd%d_mlo_kthrd", ap_unit);
	dhd_mlo_ipc->kthread = kthread_create(dhd_mlo_ipc_kthread_fn, dhd_pub, kthread_name);
	if (IS_ERR(dhd_mlo_ipc->kthread)) {
		DHD_ERROR(("%s: %s create failed\n", __FUNCTION__, kthread_name));
		ret = BCME_ERROR;
		goto dhd_mlo_ipc_init_fail;
	}

	// kthread_bind(dhd_mlo_ipc->kthread, cpu_unit);
	wake_up_process(dhd_mlo_ipc->kthread);

	dhd_mlo_ipc->ap_unit = ap_unit;
	dhd_mlo_ipc->map = NULL;
	dhd_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
	dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlo_ipc_mlc_reset_e);
	dhd_mlo_ipc->suspended = FALSE;

	// Register MLO AP context with System wide MLO_IPC
	if (mlo_unit == PCIE_IPC_MLO_UNIT_MAP) {	// Is MAP?
		ret = dhd_mlo_ipc_map_init(dhd_pub, dhd_mlo_ipc);
		if (ret != BCME_OK) {
			goto dhd_mlo_ipc_init_fail;
		}
		mlo_ipc_map_register(ap_unit, dhd_mlo_ipc_generic_cb,
			(void *)dhd_pub, PCIE_IPC_MLO_OPMODE_FD);
	} else {
		mlo_ipc_aap_register(ap_unit, dhd_mlo_ipc_generic_cb,
			(void *)dhd_pub, PCIE_IPC_MLO_OPMODE_FD);
	}

	DHD_ERROR(("%s: ap_unit[%d] SUCCESS; mlo_unit[%d] is_mlo[%d] is_map[%d]\n",
		__FUNCTION__, ap_unit, mlo_unit, DHD_IS_MLO_AP(dhd_pub), DHD_IS_MLO_MAP(dhd_pub)));

	return BCME_OK;

dhd_mlo_ipc_init_fail:

	DHD_ERROR(("%s: ap_unit[%d] Failure\n", __FUNCTION__, ap_unit));
	dhd_mlo_ipc_deinit(dhd_pub);
	return ret;
} // dhd_mlo_ipc_init()

void // Destruct DHD_MLO_IPC objects
dhd_mlo_ipc_deinit(dhd_pub_t * dhd_pub)
{
	dhd_mlo_ipc_t * dhd_mlo_ipc;
	int len = sizeof(dhd_mlo_ipc_t);

	DHD_TRACE(("%s: ENTER ap_unit[%d]\n", __FUNCTION__, dhd_pub->unit));

	if (DHD_IS_MLO_AP(dhd_pub) == FALSE) {
		return;
	}

	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	if (dhd_mlo_ipc->kthread) {
		kthread_stop(dhd_mlo_ipc->kthread);
	}

	DHD_LOCK(dhd_pub);

	if (dhd_mlo_ipc->wq_event_msg_queue) {
		ASSERT(DHD_IS_MLO_MAP(dhd_pub));
		dhd_mlo_ipc_map_msg_queue_process(dhd_pub, dhd_mlo_ipc);
	}

	if (dhd_mlo_ipc->wq_event_state_change) {
		ASSERT(DHD_IS_MLO_MAP(dhd_pub) == FALSE);
		dhd_mlo_ipc_process_state_change(dhd_pub, dhd_mlo_ipc, dhd_mlo_ipc->req_mlc_state);
		// Reset Wait Event
		dhd_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
		dhd_mlo_ipc->wq_event_state_change = 0;
	}

	/* Process all events from event queue */
	if (dhd_mlo_ipc->wq_event_evt_queue) {
		dhd_mlo_ipc_evt_queue_process(dhd_pub, dhd_mlo_ipc, FALSE);
	}

#if defined(BCM_ROUTER) && !defined(DEVTREE_CONFIGURED_PCIE_MLC_BRIDGE)
	dhdpcie_bus_bridge_deinit(dhd_pub);
#endif /* BCM_ROUTER && !DEVTREE_CONFIGURED_PCIE_MLC_BRIDGE */

	if (DHD_IS_MLO_MAP(dhd_pub)) {
		mlo_ipc_map_deregister(dhd_pub->unit);

		MFREE(dhd_pub->osh, dhd_mlo_ipc->map, sizeof(dhd_mlo_ipc_map_t));
		dhd_mlo_ipc->map = NULL;
	} else {
		mlo_ipc_aap_deregister(dhd_pub->unit);
	}

	MFREE(dhd_pub->osh, dhd_mlo_ipc, len);
	dhd_pub->dhd_mlo_ipc = NULL;

	DHD_UNLOCK(dhd_pub);

} // dhd_mlo_ipc_deinit()

static int // dhd_mlo_ipc kthread handler
dhd_mlo_ipc_kthread_fn(void * data)
{
	dhd_pub_t	* dhd_pub;
	dhd_mlo_ipc_t	* dhd_mlo_ipc;

	dhd_pub = (dhd_pub_t *)data;
	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	while (1) {

		// MLO AP wait event queue
		wait_event_interruptible(dhd_mlo_ipc->wq_head,
				(dhd_mlo_ipc->wq_event_msg_queue ||
				dhd_mlo_ipc->wq_event_state_change ||
				dhd_mlo_ipc->wq_event_evt_queue ||
				kthread_should_stop()));

		if (kthread_should_stop()) {
			DHD_ERROR(("kthread_should_stop detected on ap_unit[%d]\n",
				dhd_mlo_ipc->ap_unit));
			break;
		}

		DHD_INFO(("%s: ap_unit[%d] grabbing lock\n", __FUNCTION__, dhd_mlo_ipc->ap_unit));

		DHD_LOCK(dhd_pub); /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

		// New messages are avialable in msg queue
		if (dhd_mlo_ipc->wq_event_msg_queue) {

			ASSERT(DHD_IS_MLO_MAP(dhd_pub));
			DHD_INFO(("%s: ap_unit[%d] received MSG_QUEUE event\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit));

			dhd_mlo_ipc_map_msg_queue_process(dhd_pub, dhd_mlo_ipc);
		}

		DHD_MLO_IPC_LOCK(dhd_mlo_ipc);
		/* State Transition request from MAP => AAP */
		if (dhd_mlo_ipc->wq_event_state_change) {
			uint req_mlc_state = dhd_mlo_ipc->req_mlc_state;

			ASSERT((DHD_IS_MLO_MAP(dhd_pub) == FALSE) ||
				(req_mlc_state == mlo_ipc_mlc_up_e));

			DHD_INFO(("%s: ap_unit[%d] received STATE_CHANGE event\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit));

			// Reset Wait Event
			dhd_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
			dhd_mlo_ipc->wq_event_state_change = 0;

			if ((dhd_mlo_ipc_process_state_change(dhd_pub, dhd_mlo_ipc,
				req_mlc_state) != BCME_OK) &&
				(req_mlc_state == mlo_ipc_mlc_up_e)) {
				DHD_ERROR(("%s: ap_unit[%d] retry state change[%d] after"
					" ioctl finished\n", __FUNCTION__, dhd_mlo_ipc->ap_unit,
					req_mlc_state));
				dhd_mlo_ipc->req_mlc_state = req_mlc_state;
			}
		}

		if (dhd_mlo_ipc->wq_event_evt_queue) {
			DHD_INFO(("%s: ap_unit[%d] received MSG_QUEUE event\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit));

			dhd_mlo_ipc_evt_queue_process(dhd_pub, dhd_mlo_ipc, TRUE);
		}
		DHD_MLO_IPC_UNLOCK(dhd_mlo_ipc);

		DHD_UNLOCK(dhd_pub); /* -------------------------------------------------------- */
	}

	return 0;
} // dhd_mlo_ipc_kthread_fn()

bool
dhd_is_mlo_ap(dhd_pub_t * dhd_pub)
{
	return DHD_IS_MLO_AP(dhd_pub);
}

bool
dhd_is_mlo_map(dhd_pub_t * dhd_pub)
{
	return DHD_IS_MLO_AP(dhd_pub) && DHD_IS_MLO_MAP(dhd_pub);
}

int
dhd_mlo_state(dhd_pub_t * dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	return dhd_mlo_ipc->mlc_state;
}

/**
 * ------------------------------------------------------------------------------------------------
 * SECTION: MLO_IPC MSG processing
 *
 * All AP's participating in MLO has to synchronize the state (BIND, LINK, SYNC)
 * before moving on to the next state.
 * Using star topology with MAP as the center.
 *
 * Inter AP Finite State Machine:
 * - Each MLO AP informs their dongle instance to perform an operation (MLO BIND, LINK, SYNC).
 * - The MLO AP (AAP & MAP) on receiving an op-complete event from dongle will inform MAP
 *   about the new state.
 * - After receiving the request from all MLO AP's, MAP will broadcast the state
 *   transition to all MLO AP's.
 *
 * MAP will have a msg queue (mlo_ipc_msg_queue_t) attached to it and maintains an array of MSG
 * objects (mlo_ipc_msg_t) with size equal to MAX msg IDs (MLO_IPC_MSG_ID_LAST).
 * MLO AP will set its ap_unit bit in msg::req_ap_bm and move the MSG object from the free list to
 * active list.
 * MAP on receiving a msg will synchronize the state transition and move the MSG object to
 * free list.
 * ------------------------------------------------------------------------------------------------
 */

/* Enqueue the state transition request from MLO AP to MAP and inform MAP of new messages.
 * Invoked from producer context.
 */
static void
dhd_mlo_ipc_map_msg_enqueue(int ap_unit, uint8 msg_id, uint8 msg_prio,
                            uint8 msg_unit, void * data)
{
	dhd_pub_t	* dhd_pub;
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	mlo_ipc_msg_t	* msg;
	mlo_ipc_msg_queue_t * msg_queue;

	DHD_INFO(("%s: ap_unit[%d] id[%d] prio[%d] unit[%d] data[%p]\n", __FUNCTION__,
		ap_unit, msg_id, msg_prio, msg_unit, data));

	dhd_pub = (dhd_pub_t *)data;
	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	msg_queue = MLO_IPC_MSG_QUEUE();

	ASSERT(ap_unit == dhd_mlo_ipc->ap_unit);
	ASSERT(DHD_IS_MLO_MAP(dhd_pub));

	MLO_IPC_MSG_QUEUE_LOCK(); // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	msg = MLO_IPC_MSG(msg_id);

	// Delete from free list
	dll_delete(&msg->node);

	ASSERT(msg->id == msg_id);
	msg->prio = msg_prio;
	msg->req_ap_bm |= NBITVAL(msg_unit);

	// Add msg to the active list
	if (msg_prio == MLO_IPC_MSG_PRIO_HIGH) {
		// Enqueue high prio messages to the head
		dll_prepend(&msg_queue->active_list, &msg->node);
	} else {
		dll_append(&msg_queue->active_list, &msg->node);
	}

	msg_queue->enqueue_cnt++;

	dhd_mlo_ipc->wq_event_msg_queue = ~0;

	MLO_IPC_MSG_QUEUE_UNLK(); // --------------------------------------------------------------

	// Inform MAP of new message
	DHD_INFO(("%s: ap_unit[%d] informs MAP of new message[%d]\n",
		__FUNCTION__, ap_unit, msg_id));

	wake_up_interruptible(&dhd_mlo_ipc->wq_head);
} // dhd_mlo_ipc_map_msg_enqueue()

/** Broadcast new state to all AAP's */
static INLINE void
dhd_mlo_ipc_map_state_broadcast(dhd_pub_t * dhd_pub, uint8 mlc_state, uint8 bcast_ap_bm)
{
	int	iter;
	uint8	ap_unit_map = mlo_ipc_get_ap_unit_map();

	// Broadcast new state to all AAP's
	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		mlo_ipc_generic_cb_t generic_cb;
		void *aap_data;

		DHD_INFO(("%s: iter[%d]: ap_unit_map[%d] bcast_ap_bm[%x]\n", __FUNCTION__,
			iter, ap_unit_map, bcast_ap_bm));
		if ((iter == ap_unit_map) || ((bcast_ap_bm & NBITVAL(iter)) == 0)) {
			// Don't send state transition request to MAP and non MLO AP
			continue;
		}

		if ((generic_cb = mlo_ipc_get_generic_cb(iter)) != NULL) {
			aap_data = mlo_ipc_get_mlo_ap_data(iter);
			DHD_UNLOCK(dhd_pub);
			generic_cb(MLO_IPC_CB_AAP_STATE_CHG, aap_data, iter,
				(uint32)mlc_state, 0, 0);
			DHD_LOCK(dhd_pub);
		} else {
			DHD_ERROR(("%s: generic_cb[%d] is not registered\n",
				__FUNCTION__, iter));
		}
	}
}

static bool
dhd_mlo_ipc_map_mlo_up_ready(dhd_pub_t * dhd_pub, uint8 req_ap_bm)
{
	int	iter;

	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		mlo_ipc_generic_cb_t generic_cb;
		void *aap_data;

		DHD_INFO(("%s: iter[%d]: ap_unit_map[%d] req_ap_bm[%x]\n", __FUNCTION__,
			iter, dhd_pub->unit, req_ap_bm));
		if (iter == dhd_pub->unit || (req_ap_bm & NBITVAL(iter)) == 0) {
			continue;
		}

		if ((generic_cb = mlo_ipc_get_generic_cb(iter)) != NULL) {
			aap_data = mlo_ipc_get_mlo_ap_data(iter);
			DHD_UNLOCK(dhd_pub);
			if (generic_cb(MLO_IPC_CB_WL_ISUP, aap_data, iter, 0, 0, 0) != BCME_OK) {
				DHD_ERROR(("dhd%d: %s: wl%d is not up\n",
					dhd_pub->unit, __FUNCTION__, iter));
				DHD_LOCK(dhd_pub);
				return FALSE;
			}
			DHD_LOCK(dhd_pub);
		} else {
			DHD_ERROR(("dhd%d: %s: generic_cb[%d] is not registered\n",
				dhd_pub->unit, __FUNCTION__, iter));
			ASSERT(0);
		}
	}

	return TRUE;
}

/** Parse the messge from MLO AP and broadcast state transition to all MLO AP's */
static INLINE void
dhd_mlo_ipc_map_msg_process(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, mlo_ipc_msg_t * msg)
{
	dhd_mlo_ipc_map_t * map;
	uint8	new_mlc_state;
	uint8	mlo_ap_bm = mlo_ipc_get_ap_bm();
	uint8   mlo_down_ap_bm = mlo_ipc_get_mlo_down_ap_bm();
	uint8   mlo_up_ap_bm = mlo_ap_bm & ~(mlo_down_ap_bm);
	uint8   bcast_ap_bm = mlo_ap_bm;
	uint8   mlo_disable_ap_bm = mlo_ipc_get_mlo_disable_ap_bm();

	map = dhd_mlo_ipc->map;

	DHD_TRACE(("%s: ap_unit[%d] - Process MAP msg::id[%d] mlc_state %d\n", __FUNCTION__,
		dhd_mlo_ipc->ap_unit, msg->id, map->mlc_state));

	if (map->mlc_state == mlo_ipc_mlc_halt_e) {
		DHD_ERROR(("%s: ap_unit[%d]: already halted, ignoring msg::id[%d]\n",
			__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id));
		return;
	}

	switch (msg->id) {
		case MLO_IPC_MSG_ID_ATTACH:
			new_mlc_state = mlo_ipc_mlc_bind_e;
			break;

		case MLO_IPC_MSG_ID_BIND:
			if ((map->mlc_state != mlo_ipc_mlc_bind_e) &&
				(map->mlc_state != mlo_ipc_mlc_active_e)) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(0);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if (mlo_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_link_e;
			break;

		case MLO_IPC_MSG_ID_LINK:
			if ((map->mlc_state != mlo_ipc_mlc_link_e) &&
				(map->mlc_state != mlo_ipc_mlc_active_e)) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(0);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if (mlo_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_bridge_e;
			break;

		case MLO_IPC_MSG_ID_BRIDGE:
			if ((map->mlc_state != mlo_ipc_mlc_bridge_e) &&
				(map->mlc_state != mlo_ipc_mlc_active_e)) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(0);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if (mlo_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_sync_e;
			break;

		case MLO_IPC_MSG_ID_SYNC:
			if (map->mlc_state != mlo_ipc_mlc_sync_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(0);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if ((mlo_ap_bm & ~mlo_disable_ap_bm) != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_ready_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;

		case MLO_IPC_MSG_ID_TRAP:
			new_mlc_state = mlo_ipc_mlc_halt_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;

		case MLO_IPC_MSG_ID_DOWN_REQ:
			ASSERT(map->mlc_state == mlo_ipc_mlc_active_e);
			/* Perform MLO_SUSPEND on all MLO links */
			new_mlc_state = mlo_ipc_mlc_suspend_e;
			map->next_state = mlo_ipc_mlc_down_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;
#if defined(BCMMLO_GR)
		case MLO_IPC_MSG_ID_BH_REQ:
		case MLO_IPC_MSG_ID_BH_HALT_REQ:
			if (map->mlc_state != mlo_ipc_mlc_active_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Ignoring msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				return;
			}
			if (msg->id == MLO_IPC_MSG_ID_BH_REQ) {
				new_mlc_state = mlo_ipc_mlc_suspend_e;
			} else {
				new_mlc_state = mlo_ipc_mlc_suspend_halt_e;
			}
			map->next_state = mlo_ipc_mlc_bh_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;
#endif /* BCMMLO_GR */

		case MLO_IPC_MSG_ID_DOWN_BH_REQ:
			ASSERT(map->mlc_state == mlo_ipc_mlc_active_e);
			new_mlc_state = mlo_ipc_mlc_suspend_e;
			map->next_state = mlo_ipc_mlc_down_bh_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;

		case MLO_IPC_MSG_ID_SUSPEND:
			if ((map->mlc_state != mlo_ipc_mlc_suspend_e) &&
				(map->mlc_state != mlo_ipc_mlc_suspend_halt_e)) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(map->mlc_state == mlo_ipc_mlc_suspend_e ||
					map->mlc_state == mlo_ipc_mlc_suspend_halt_e);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if (mlo_up_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = map->next_state;
			break;

		case MLO_IPC_MSG_ID_DOWN:
			if (map->mlc_state != mlo_ipc_mlc_down_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(map->mlc_state == mlo_ipc_mlc_down_e);
				return;
			}
			new_mlc_state = mlo_ipc_mlc_down_complete_e;
			map->next_state = mlo_ipc_mlc_ready_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;

#if defined(BCMMLO_GR)
		case MLO_IPC_MSG_ID_BH:
			if (map->mlc_state != mlo_ipc_mlc_bh_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(map->mlc_state == mlo_ipc_mlc_bh_e);
				return;
			}
			new_mlc_state = mlo_ipc_mlc_bh_complete_e;
			map->next_state = mlo_ipc_mlc_ready_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;
#endif /* BCMMLO_GR */

		case MLO_IPC_MSG_ID_DOWN_BH:
			if (map->mlc_state != mlo_ipc_mlc_down_bh_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(map->mlc_state == mlo_ipc_mlc_down_bh_e);
				return;
			}
			new_mlc_state = mlo_ipc_mlc_down_bh_complete_e;
			map->next_state = mlo_ipc_mlc_ready_e;
			/* broadcast to the APs which are MLO up before or after mlo down/up */
			bcast_ap_bm = mlo_up_ap_bm | (mlo_ap_bm & ~map->next_mlo_down_ap_bm);
			mlo_ipc_set_mlo_down_ap_bm(map->next_mlo_down_ap_bm);
			break;

		case MLO_IPC_MSG_ID_UP_REQ:
			if (map->mlc_state < mlo_ipc_mlc_sync_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				return;
			}

			if (map->mlc_state == mlo_ipc_mlc_active_e) {
				new_mlc_state = mlo_ipc_mlc_suspend_e;
				map->next_state = mlo_ipc_mlc_down_bh_e;
				bcast_ap_bm = mlo_up_ap_bm;
				if (!dhd_mlo_ipc_map_mlo_up_ready(dhd_pub, bcast_ap_bm)) {
					DHD_ERROR(("%s: ap_unit[%d]: drop MLO_UP request\n",
						__FUNCTION__, dhd_mlo_ipc->ap_unit));
					return;
				}
			} else {
				new_mlc_state = mlo_ipc_mlc_up_e;
				bcast_ap_bm &= ~(map->next_mlo_down_ap_bm);
				if (!dhd_mlo_ipc_map_mlo_up_ready(dhd_pub, bcast_ap_bm)) {
					DHD_ERROR(("%s: ap_unit[%d]: drop MLO_UP request\n",
						__FUNCTION__, dhd_mlo_ipc->ap_unit));
					return;
				}
				mlo_ipc_set_mlo_down_ap_bm(map->next_mlo_down_ap_bm);
			}
			break;

		case MLO_IPC_MSG_ID_ACTIVE:
			if (map->mlc_state != mlo_ipc_mlc_ready_e &&
				map->mlc_state != mlo_ipc_mlc_down_bh_complete_e) {
				DHD_ERROR(("%s: ap_unit[%d]: Unexpected msg::id[%d] state[%d]\n",
					__FUNCTION__, dhd_mlo_ipc->ap_unit, msg->id,
					map->mlc_state));
				ASSERT(0);
				return;
			}

			// Check if state transition is received from all MLO AP's
			if (mlo_up_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_active_e;
			bcast_ap_bm = mlo_up_ap_bm;
			break;

		default:
			DHD_ERROR(("%s: ap_unit[%d]: Invalid msg::id[%d]\n", __FUNCTION__,
				dhd_mlo_ipc->ap_unit, msg->id));
			ASSERT(0);
			return;
	}

	if (new_mlc_state == mlo_ipc_mlc_down_e ||
#if defined(BCMMLO_GR)
		new_mlc_state == mlo_ipc_mlc_bh_e ||
#endif /* BCMMLO_GR */
		new_mlc_state == mlo_ipc_mlc_down_bh_e ||
		FALSE) {
		goto map_state_transition;
	}

	DHD_INFO(("%s: ap_unit[%d] - Broadcasting state[%d] transition to MLO AP bm 0x%x\n",
		__FUNCTION__, dhd_mlo_ipc->ap_unit, new_mlc_state, bcast_ap_bm));

	dhd_mlo_ipc_map_state_broadcast(dhd_pub, new_mlc_state, bcast_ap_bm);

map_state_transition:
	// Process state change in MAP
	// Don't change MAP's active state when a powering up AAP triggers the reBIND
	if (!((map->mlc_state == mlo_ipc_mlc_active_e) &&
		(new_mlc_state < mlo_ipc_mlc_ready_e)) &&
		(new_mlc_state != mlo_ipc_mlc_up_e)) {
		map->mlc_state = new_mlc_state;
	}

	if ((dhd_mlo_ipc_process_state_change(dhd_pub, dhd_mlo_ipc, new_mlc_state) != BCME_OK) &&
		(new_mlc_state == mlo_ipc_mlc_up_e)) {
		DHD_ERROR(("%s: ap_unit[%d] retry state change[%d] after ioctl finished\n",
			__FUNCTION__, dhd_mlo_ipc->ap_unit, new_mlc_state));
		dhd_mlo_ipc->req_mlc_state = mlo_ipc_mlc_up_e;
	}

	// Reset request in progess
	msg->id = ~0;

	return;
} // dhd_mlo_ipc_map_msg_process()

static uint8
dhd_mlo_ipc_map_mlo_down_process(dhd_mlo_ipc_t *dhd_mlo_ipc, uint8 req_ap_unit, bool mlo_up)
{
	uint8 mlo_ap_bm = mlo_ipc_get_ap_bm();
	uint8 mlo_down_ap_bm = mlo_ipc_get_mlo_down_ap_bm();
	uint8 mlo_disable_ap_bm = mlo_ipc_get_mlo_disable_ap_bm();
	uint8 ap_unit;
	uint8 map_ap_unit = dhd_mlo_ipc->ap_unit;

	if (mlo_up) {
		if ((req_ap_unit != map_ap_unit) &&
			(mlo_down_ap_bm & NBITVAL(map_ap_unit)) == 0) {
			/* enable MLO for the requested AAP if MLO on MAP is enabled */
			mlo_down_ap_bm &= ~NBITVAL(req_ap_unit);
		} else {
			/* enable MLO excluding the force disabled units */
			for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ap_unit++) {
				if (!(mlo_ap_bm & NBITVAL(ap_unit)) ||
					(mlo_disable_ap_bm & NBITVAL(ap_unit))) {
					continue;
				}
				mlo_down_ap_bm &= ~NBITVAL(ap_unit);
			}
		}
	} else {
		if (req_ap_unit != map_ap_unit) {
			/* disable MLO for the requested AAP */
			mlo_down_ap_bm |= NBITVAL(req_ap_unit);
		} else {
			/* disable MLO for all MLO APs */
			mlo_down_ap_bm |= mlo_ap_bm;
		}
	}

	return mlo_down_ap_bm;
}

/** Check and process all messages from MAP msg queue. */
static void
dhd_mlo_ipc_map_msg_queue_process(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc)
{
	mlo_ipc_msg_t * msg;
	mlo_ipc_msg_queue_t * msg_queue;
	dhd_mlo_ipc_map_t * map;
	dll_t * item;

	msg_queue = MLO_IPC_MSG_QUEUE();
	map = dhd_mlo_ipc->map;

msg_queue_process_continue:

	MLO_IPC_MSG_QUEUE_LOCK(); // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (dll_empty(&msg_queue->active_list)) {
		// No more messages to process
		// Reset msg_queue pending work
		dhd_mlo_ipc->wq_event_msg_queue = 0;
		MLO_IPC_MSG_QUEUE_UNLK();
		return;
	}

	item = dll_head_p(&msg_queue->active_list);
	msg = container_of(item, mlo_ipc_msg_t, node);
	// Remove the msg node from active list
	dll_delete(&msg->node);
	msg_queue->dequeue_cnt++;

	DHD_INFO(("%s: map msg.id[%d] bm[%d] msg id[%d] bm[%x]\n",
		__FUNCTION__, map->msg.id, map->msg.req_ap_bm, msg->id, msg->req_ap_bm));

	if ((msg->id == MLO_IPC_MSG_ID_SYNC) &&	dhd_mlo_ipc_is_ready(dhd_pub)) {
		uint8 req_ap_bm = msg->req_ap_bm;
		DHD_ERROR(("%s: id[%d] prio[%d] req_ap_bm[%x] SYNC complete but MAP is ready\n",
			__FUNCTION__, msg->id, msg->prio, msg->req_ap_bm));
		// Reset original msg and move it to the free list
		msg->prio = ~0;
		msg->req_ap_bm = 0;
		dll_append(&msg_queue->free_list, &msg->node);
		MLO_IPC_MSG_QUEUE_UNLK(); // ------------------------------------------------------
		dhd_mlo_ipc_map_state_broadcast(dhd_pub, mlo_ipc_mlc_ready_e, req_ap_bm);
		goto msg_queue_process_continue;

	} else if (map->msg.id == msg->id) {
		// Request in process. Copy the udpated msg values
		map->msg.req_ap_bm |= msg->req_ap_bm;
	} else {
		// New request. Get full message
		map->msg = *msg;
		if (map->msg.id == MLO_IPC_MSG_ID_DOWN_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_BH_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_BH_HALT_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_DOWN_BH_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_UP_REQ) {
			uint8 req_ap_unit = bcm_find_fsb(msg->req_ap_bm);
			ASSERT(req_ap_unit > 0);
			// Keep the requesting AP unit with DOWN and BH requests
			map->gr_req_ap_unit = req_ap_unit - 1;
			if (map->msg.id == MLO_IPC_MSG_ID_UP_REQ) {
				map->next_mlo_down_ap_bm =
					dhd_mlo_ipc_map_mlo_down_process(dhd_mlo_ipc,
					map->gr_req_ap_unit, TRUE);
			} else if (map->msg.id == MLO_IPC_MSG_ID_DOWN_BH_REQ) {
				map->next_mlo_down_ap_bm =
					dhd_mlo_ipc_map_mlo_down_process(dhd_mlo_ipc,
					map->gr_req_ap_unit, FALSE);
			} else {
				map->next_mlo_down_ap_bm = mlo_ipc_get_mlo_down_ap_bm();
			}
		} else if (map->msg.id != MLO_IPC_MSG_ID_SUSPEND &&
			map->msg.id != MLO_IPC_MSG_ID_DOWN &&
			map->msg.id != MLO_IPC_MSG_ID_BH &&
			map->msg.id != MLO_IPC_MSG_ID_DOWN_BH) {
			map->gr_req_ap_unit = (uint8)AP_UNIT_INV;
		}
	}

	DHD_TRACE(("%s: id[%d] prio[%d] req_ap_bm[%x]\n", __FUNCTION__,
		msg->id, msg->prio, msg->req_ap_bm));

	// Reset original msg and move it to the free list
	msg->prio = ~0;
	msg->req_ap_bm = 0;
	dll_append(&msg_queue->free_list, &msg->node);

	// Delete low priority messages in queue on receiving a High priority message
	if (map->msg.prio == MLO_IPC_MSG_PRIO_HIGH) {
		dll_t * next;

		for (item = dll_head_p(&msg_queue->active_list);
			!dll_end(&msg_queue->active_list, item); item = next) {
			next = dll_next_p(item);

			msg = container_of(item, mlo_ipc_msg_t, node);

			if (msg->prio == MLO_IPC_MSG_PRIO_LOW) {
				// Reset and move it to the free list
				dll_delete(&msg->node);
				msg_queue->dequeue_cnt++;

				msg->prio = ~0;
				msg->req_ap_bm = 0;
				dll_append(&msg_queue->free_list, &msg->node);
			}
		}
	}

	MLO_IPC_MSG_QUEUE_UNLK(); // --------------------------------------------------------------

	dhd_mlo_ipc_map_msg_process(dhd_pub, dhd_mlo_ipc, &map->msg);

	goto msg_queue_process_continue;

} // dhd_mlo_ipc_map_msg_queue_process()

/** Callback registered with MAP invoked to inform state transition */
static void
dhd_mlo_ipc_aap_state_change_cb(int ap_unit, uint8 mlc_state, void * data)
{
	dhd_pub_t	* dhd_pub;
	dhd_mlo_ipc_t	* dhd_mlo_ipc;

	dhd_pub = (dhd_pub_t *) data;
	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	DHD_TRACE(("%s: ap_unit[%d] data[%p] mlc_state[%u]\n",
		__FUNCTION__, dhd_mlo_ipc->ap_unit, data, mlc_state));

	DHD_LOCK(dhd_pub); /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	DHD_MLO_IPC_LOCK(dhd_mlo_ipc);

	if ((dhd_mlo_ipc->req_mlc_state != mlo_ipc_mlc_reset_e) &&
		(mlc_state != mlo_ipc_mlc_halt_e)) {
		DHD_ERROR(("%s: ap_unit[%d] req_mlc_state[%u]:%s not being served "
			"but new request[%u]:%s comes\n",
			__FUNCTION__, dhd_mlo_ipc->ap_unit, dhd_mlo_ipc->req_mlc_state,
			__mlo_ipc_mlc_state_str[dhd_mlo_ipc->req_mlc_state],
			mlc_state, __mlo_ipc_mlc_state_str[mlc_state]));
		ASSERT(0);
	}
	dhd_mlo_ipc->req_mlc_state = mlc_state;
	dhd_mlo_ipc->wq_event_state_change = ~0;
	DHD_INFO(("%s: ap_unit[%d] data[%p] mlc_state[%u] wq_event_state_change[%u]\n",
		__FUNCTION__, dhd_mlo_ipc->ap_unit, data, mlc_state,
		dhd_mlo_ipc->wq_event_state_change));

	DHD_MLO_IPC_UNLOCK(dhd_mlo_ipc);
	DHD_UNLOCK(dhd_pub); /* -------------------------------------------------------- */
	wake_up_interruptible(&dhd_mlo_ipc->wq_head);

} // dhd_mlo_ipc_aap_state_change_cb()

/* Process new state transition request for MLO AP */
static int
dhd_mlo_ipc_process_state_change(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, uint8 mlc_state)
{
	int ap_unit = dhd_mlo_ipc->ap_unit;
	int ret = BCME_OK;

	if (mlc_state < mlo_ipc_mlc_max_e) {
		DHD_ERROR((MLO_IPC_PREFIX "ap_unit[%d] current %s enters %s" MLO_IPC_POSTFIX,
			ap_unit, __mlo_ipc_mlc_state_str[dhd_mlo_ipc->mlc_state],
			__mlo_ipc_mlc_state_str[mlc_state]));
	}

	switch (mlc_state) {

		case mlo_ipc_mlc_bind_e:
		case mlo_ipc_mlc_link_e:
		case mlo_ipc_mlc_bridge_e:
		case mlo_ipc_mlc_sync_e:
			if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
				dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);
			}

#if defined(BCM_ROUTER) && !defined(DEVTREE_CONFIGURED_PCIE_MLC_BRIDGE)
			if (mlc_state == mlo_ipc_mlc_bridge_e &&
				dhdpcie_bus_bridge_init(dhd_pub) != BCME_OK) {
				ASSERT(0);
				break;
			}
#endif /* BCM_ROUTER && !DEVTREE_CONFIGURED_PCIE_MLC_BRIDGE */
			/* Send mlc_state event to dongle. */
			ret = dhdpcie_mlo_h2d_notify(dhd_pub->bus, mlc_state);
			break;

		case mlo_ipc_mlc_ready_e:
			if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_sync_e) {
				/* Send mlc_state event to dongle. */
				dhdpcie_mlo_h2d_notify(dhd_pub->bus, mlc_state);
			}
			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);
			break;

		case mlo_ipc_mlc_suspend_e:
		case mlo_ipc_mlc_suspend_halt_e:
			DHD_ERROR(("dhd%d: %s: MLO_SUSPEND request\n", ap_unit, __FUNCTION__));
			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);
			ret = dhd_mlo_ipc_update_suspend_state(dhd_pub);
			break;

		case mlo_ipc_mlc_down_e:
#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_e:
#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_e:
		{
			DHD_ERROR(("dhd%d: %s: %s iovar request to dongle\n",
				ap_unit, __FUNCTION__,
				(mlc_state == mlo_ipc_mlc_down_e) ? "MLO_IPC_DOWN": "MLO_IPC_BH"));
			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);
			/* Use iovar instead of a mailbox event */
			ret = dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlc_state);
			break;
		}

		case mlo_ipc_mlc_down_complete_e:
#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_complete_e:
#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_complete_e:
		{
			bool mlo_up = FALSE;

			dhd_mlo_ipc->suspended = FALSE;

			DHD_ERROR((MLO_IPC_PREFIX "%s: ap_unit[%d] continues with non-MLO %s"
				MLO_IPC_POSTFIX, __FUNCTION__, ap_unit,
				(mlc_state == mlo_ipc_mlc_down_complete_e) ? "down" : "bh"));

			/* bring up MLO, resume the ioctl after MLO_ACTIVE */
			if ((mlc_state == mlo_ipc_mlc_down_bh_complete_e) &&
				(mlo_ipc_get_sys_mlo_down(ap_unit) == 0)) {
				mlo_up = TRUE;
			}

			if (dhd_mlo_ipc_is_ready(dhd_pub)) {
				uint8 new_mlc_state;

				if (mlo_up) {
					/* keep not ready to ignore new mlo ipc request */
					new_mlc_state = mlc_state;
				} else {
					/* MLO down is complete so revert the state to mlc_ready */
					new_mlc_state = mlo_ipc_mlc_ready_e;
				}

				if (DHD_IS_MLO_MAP(dhd_pub)) {
					dhd_mlo_ipc->map->mlc_state = new_mlc_state;
				}
				dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, new_mlc_state);
			}

			if (mlo_up) {
				ret = dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlo_ipc_mlc_up_e);
				break;
			}

			if (dhd_mlo_ipc->wl_down_pending) {
				ASSERT(dhd_mlo_ipc->ioc_pending == FALSE);
				/* Resume and continue iovar processig for non-MLO (wl down) */
				dhd_mlo_ipc->wl_down_pending = FALSE;
				dhd_mlo_ipc->ioc_complete = 1;
				dhd_os_mlo_ipc_wake(dhd_pub);
			} else {
				/* Bring down/re-init all xAPs */
				ret = dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlc_state);

				/* Resume processing of iovar requested when
				 * MLO_IPC (DOWN/BH) in progress
				 */
				if (dhd_mlo_ipc->ioc_pending) {
					dhd_mlo_ipc->ioc_pending = FALSE;
					dhd_mlo_ipc->ioc_complete = 1;
					dhd_os_mlo_ipc_wake(dhd_pub);
				}
			}
			break;
		}

		case mlo_ipc_mlc_up_e:
		{
			DHD_ERROR(("dhd%d: %s: MLO_UP request\n", ap_unit, __FUNCTION__));

			/* Bring MLO up on all xAPs */
			ret = dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlc_state);

			break;
		}

		case mlo_ipc_mlc_halt_e:
			DHD_ERROR((MLO_IPC_PREFIX "%s: ap_unit[%d] MLO TRAP prev mlc_state[%s]"
				MLO_IPC_POSTFIX, __FUNCTION__, ap_unit,
				__mlo_ipc_mlc_state_str[dhd_mlo_ipc->mlc_state]));

			/* Send mlo_ipc_mlc_halt_e state event to dongle except to the AP
			 * in which Firmware Trap detected.
			 */
			if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_trap_e) {
				DHD_ERROR(("%s: ap_unit[%d] MLO TRAP notify\n",
					__FUNCTION__, ap_unit));
				dhdpcie_mlo_h2d_notify(dhd_pub->bus, mlo_ipc_mlc_halt_e);
			}

			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);
			break;

		case mlo_ipc_mlc_active_e:
			DHD_ERROR(("dhd%d: is MLO_ACTIVE\n", dhd_mlo_ipc->ap_unit));
			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlc_state);

			if (dhd_mlo_ipc->wl_down_pending) {
				ASSERT(dhd_mlo_ipc->ioc_pending == FALSE);
				/* Resume and continue iovar processig */
				dhd_mlo_ipc->wl_down_pending = FALSE;
				dhd_mlo_ipc->ioc_complete = 1;
				dhd_os_mlo_ipc_wake(dhd_pub);
			} else {
				/* Resume processing of iovar requested */
				if (dhd_mlo_ipc->ioc_pending) {
					dhd_mlo_ipc->ioc_pending = FALSE;
					dhd_mlo_ipc->ioc_complete = 1;
					dhd_os_mlo_ipc_wake(dhd_pub);
				}
			}
			break;

		default:
			DHD_ERROR(("%s: ap_unit[%d] Invalid state[%d]\n",
				__FUNCTION__, ap_unit, mlc_state));
			ASSERT(0);
			break;
	}

	if (ret != BCME_OK) {
		DHD_ERROR(("%s: ap_unit[%d] state change[%u] failed ret %d\n",
				__FUNCTION__, ap_unit, mlc_state, ret));
	}

	return ret;
} // dhd_mlo_ipc_process_state_change()

int
dhd_mlo_ipc_process_dongle_event(dhd_pub_t *dhd_pub, uint8 mlc_state)
{
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	uint8	msg_id;
	uint8	msg_prio;
	mlo_ipc_generic_cb_t generic_cb;
	void	*map_data;
	uint8	ap_unit_map;

	if (!DHD_IS_MLO_AP(dhd_pub)) {
		return BCME_ERROR;
	}

	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	if (!DHD_IS_MLO_MAP(dhd_pub)) {
		uint8 mlo_down_ap_bm = mlo_ipc_get_mlo_down_ap_bm();

		/* ignore NIC MAP role check while MLO is down */
		if ((mlo_down_ap_bm & NBITVAL(dhd_mlo_ipc->ap_unit)) == 0) {
			ap_unit_map = mlo_ipc_get_ap_unit_map();

			/*
			* NIC shouldn't be set to MAP in NIC/FD mlo config.
			* When exchanging initial messages through IPC after each AP is configured,
			* it will detect config error during mlo initialization.
			*/
			if (mlo_ipc_get_ap_op_mode(ap_unit_map) == PCIE_IPC_MLO_OPMODE_NIC) {
				DHD_ERROR(("%s: FD should be MAP in bilingual mlo modes\n",
					__FUNCTION__));
				ASSERT(0);
				return BCME_UNSUPPORTED;
			}
		}
	}

	DHD_TRACE(("%s: ap_unit[%d] mlc_state[%d]\n", __FUNCTION__,
		dhd_mlo_ipc->ap_unit, mlc_state));

	switch (mlc_state) {
		case mlo_ipc_mlc_attach_e:
			msg_id = MLO_IPC_MSG_ID_ATTACH;
			msg_prio = MLO_IPC_MSG_PRIO_LOW;
			break;

		case mlo_ipc_mlc_bind_e:
			msg_id = MLO_IPC_MSG_ID_BIND;
			msg_prio = MLO_IPC_MSG_PRIO_LOW;
			break;

		case mlo_ipc_mlc_link_e:
			msg_id = MLO_IPC_MSG_ID_LINK;
			msg_prio = MLO_IPC_MSG_PRIO_LOW;
			break;

		case mlo_ipc_mlc_bridge_e:
			msg_id = MLO_IPC_MSG_ID_BRIDGE;
			msg_prio = MLO_IPC_MSG_PRIO_LOW;
			break;

		case mlo_ipc_mlc_sync_e:
			msg_id = MLO_IPC_MSG_ID_SYNC;
			msg_prio = MLO_IPC_MSG_PRIO_LOW;
			break;

		case mlo_ipc_mlc_trap_e:
			DHD_ERROR(("dhd%d: reporting a dongle TRAP\n", dhd_mlo_ipc->ap_unit));

			/* If a trap event happened while MLO is down, don't inform other
			 * interfaces.
			 */
			if (mlo_ipc_get_sys_mlo_down(dhd_mlo_ipc->ap_unit)) {
				DHD_ERROR(("dhd%d: MLO is already down\n", dhd_mlo_ipc->ap_unit));
				return BCME_ERROR;
			}

			/* If a trap event happened before, don't bother to halt other
			 * interfaces again.
			 */
			if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_halt_e) {
				return BCME_OK;
			}
			msg_id = MLO_IPC_MSG_ID_TRAP;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlo_ipc_mlc_trap_e);
			break;

		case mlo_ipc_mlc_suspend_e:
			msg_id = MLO_IPC_MSG_ID_SUSPEND;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_down_req_e:
			/* Hijacking the dongle event process routine for DOWN events */
			msg_id = MLO_IPC_MSG_ID_DOWN_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_down_e:
			msg_id = MLO_IPC_MSG_ID_DOWN;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;
#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_req_e:
			/* Hijacking the dongle event process routine for BH events */
			msg_id = MLO_IPC_MSG_ID_BH_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_bh_halt_req_e:
			/* Hijacking the dongle event process routine for BH events with PSMWD */
			msg_id = MLO_IPC_MSG_ID_BH_HALT_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_bh_e:
			msg_id = MLO_IPC_MSG_ID_BH;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;
#endif /* BCMMLO_GR */

		case mlo_ipc_mlc_down_bh_req_e:
			/* Hijacking the dongle event process routine for DOWN events */
			msg_id = MLO_IPC_MSG_ID_DOWN_BH_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_down_bh_e:
			msg_id = MLO_IPC_MSG_ID_DOWN_BH;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_up_req_e:
			msg_id = MLO_IPC_MSG_ID_UP_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_active_e:
			msg_id = MLO_IPC_MSG_ID_ACTIVE;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		default:
			DHD_ERROR(("%s: ap_unit[%d]: Unexpected mlc_state[%d] from dongle\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit, mlc_state));
			return BCME_ERROR;
	}

	ap_unit_map = mlo_ipc_get_ap_unit_map();
	if ((generic_cb = mlo_ipc_get_generic_cb(ap_unit_map)) != NULL) {
		map_data = mlo_ipc_get_map_data();
		generic_cb(MLO_IPC_CB_MAP_MSG_ENQ, map_data, ap_unit_map, msg_id,
			msg_prio, dhd_mlo_ipc->ap_unit);
		return BCME_OK;
	} else {
		DHD_ERROR(("%s: ap_unit[%d]: mlc_state[%d] generic_cb[%d] is not registered\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit, mlc_state, ap_unit_map));
		return BCME_ERROR;
	}
} // dhd_mlo_ipc_process_dongle_event()

int
dhd_mlo_ipc_start(dhd_pub_t *dhd_pub, uint8 *mlo_ipc_started)
{
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	uint8	mlo_ap_bm; /* registered APs */
	uint8	mlo_active_bm, mlo_unit_bm = 0, ap_unit;
	int ret = BCME_OK;
	bool started = FALSE;

	if (!DHD_IS_MLO_AP(dhd_pub)) {
		/* Non-MLO AP */
		goto out;
	}

	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc, mlo_ipc_mlc_attach_e);

	mlo_ap_bm = mlo_ipc_set_ap_bm(dhd_mlo_ipc->ap_unit);

	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ap_unit++) {
		if (mlo_ap_bm & NBITVAL(ap_unit)) {
			int mlo_unit = mlo_ipc_get_mlo_unit(ap_unit);
			ASSERT(mlo_unit != PCIE_IPC_MLO_UNIT_INV);
			mlo_unit_bm |= NBITVAL(mlo_unit);
		}
	}

	mlo_active_bm = mlo_ipc_get_sys_mlo_active_bm();
	ASSERT(mlo_active_bm);

	DHD_PRINT(("%s: mlo_unit_bm %x expecting %x\n", __FUNCTION__,
		mlo_unit_bm, mlo_active_bm));

	if (mlo_active_bm != mlo_unit_bm) {
		goto out;
	}

	DHD_PRINT(("%s: All active MLO APs registered.\n", __FUNCTION__));
	if ((ret = dhd_mlo_ipc_process_dongle_event(dhd_pub, dhd_mlo_ipc->mlc_state)) == BCME_OK) {
		started = TRUE;
	}

out:
	if (mlo_ipc_started != NULL) {
		*mlo_ipc_started = ((started == TRUE) ? mlo_unit_bm : 0);
	}

	return ret;
}

/**
 * ------------------------------------------------------------------------------------------------
 * SECTION: MLO_IPC Event processing
 * ------------------------------------------------------------------------------------------------
 */

/* Wakeup callback registered with MLO IPC Event invoked by producer link after
 * queuing a new event message.
 */
static void
dhd_mlo_ipc_evt_wake_up_cb(void *data)
{
	dhd_pub_t	*dhd_pub = (dhd_pub_t*)data;
	dhd_mlo_ipc_t	*dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	ASSERT(DHD_IS_MLO_AP(dhd_pub));

	DHD_MLO_IPC_LOCK(dhd_mlo_ipc);
	dhd_mlo_ipc->wq_event_evt_queue = ~0;
	DHD_MLO_IPC_UNLOCK(dhd_mlo_ipc);
	wake_up_interruptible(&dhd_mlo_ipc->wq_head);

	return;
} // dhd_mlo_ipc_evt_wake_up_cb()

/* Process all (bounded) events messages from event queue */
static void
dhd_mlo_ipc_evt_queue_process(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, bool bound)
{
	mlo_ipc_evt_msg_t * evt_msg;
	int	evt_msgs_processed = 0;

	DHD_TRACE(("%s: dhd%d: ENTER\n", __FUNCTION__, dhd_pub->unit));

evt_queue_process_continue:

	/* Get the Event message from the queue */
	evt_msg = mlo_ipc_evt_queue_dequeue(dhd_pub->osh, dhd_pub->unit);
	if (evt_msg == NULL) {
		// No more messages to process
		dhd_mlo_ipc->wq_event_evt_queue = 0;
		return;
	}

	DHD_INFO(("%s: dhd%d: MLO IPC EVT [%d] from ap [%d]\n", __FUNCTION__, dhd_pub->unit,
		evt_msg->evt_id, evt_msg->ap_unit));

	switch (evt_msg->evt_id) {
#ifdef MLO_BCMC
		case MLO_IPC_EVT_ID_BCMC_XMIT:
		{
			mlo_ipc_bcmc_xmit_msg_t *bcmc_xmit_msg;
			bcmc_xmit_msg = (mlo_ipc_bcmc_xmit_msg_t *)evt_msg->data;

			dhd_mlo_aap_handle_bcmc(dhd_pub, bcmc_xmit_msg);
			break;
		}
#endif /* MLO_BCMC */
		case MLO_IPC_EVT_ID_IOV_MSG:
		{
			mlo_ipc_iov_msg_t *iov_msg;
			iov_msg = (mlo_ipc_iov_msg_t*)evt_msg->data;

			if (iov_msg->msg_subtype == MLO_IPC_EVT_IOV_SUBTYPE_ALLOW_IOV) {
				/* wake up */
				if (dhd_mlo_ipc->ioc_pending) {
					dhd_mlo_ipc->ioc_complete = 1;
					dhd_os_mlo_ipc_wake(dhd_pub);
					dhd_mlo_ipc->ioc_pending = 0;
				}
				dhd_mlo_ipc->iov_defer = 0;
			} else if (iov_msg->msg_subtype == MLO_IPC_EVT_IOV_SUBTYPE_DEFER_IOV) {
				DHD_INFO(("dhd%d check for mld bss for mld unit[%d]\n",
					dhd_pub->unit, iov_msg->mld_unit));
				if (dhd_is_mld_bss_configured(dhd_pub, iov_msg->mld_unit)) {
					DHD_INFO(("dhd%d %s: set IOV_DEFER 1\n", dhd_pub->unit,
						__FUNCTION__));
					dhd_mlo_ipc->iov_defer = 1;
				} else {
					DHD_INFO(("dhd%d NO valid mld bss for mld unit[%d]\n",
						dhd_pub->unit, iov_msg->mld_unit));
				}
			}
			break;
		}
		case MLO_IPC_EVT_ID_ROLE_RECONFIG:
		{
			mlo_ipc_role_reconfig_msg_t *role_reconfig_msg;
			void *mlo_ap_data;

			role_reconfig_msg = (mlo_ipc_role_reconfig_msg_t *)evt_msg->data;
			dhd_mlo_handle_role_reconfig(dhd_pub, role_reconfig_msg->new_mlo_unit);
			mlo_ap_data = mlo_ipc_get_mlo_ap_data(evt_msg->ap_unit);
			role_reconfig_msg->mlo_role_reconfig_cb(mlo_ap_data, dhd_mlo_ipc->ap_unit);
			break;
		}
		default:
		    DHD_ERROR(("MLO IPC Event[%d] not yet implemented\n", evt_msg->evt_id));
		    ASSERT(0);
		    break;
	}
	/* Release event message */
	mlo_ipc_evt_msg_free(evt_msg);

	evt_msgs_processed++;
	if (bound && (evt_msgs_processed > DHD_MLO_IPC_EVT_QUEUE_BUDGET)) {
		return;
	}

	goto evt_queue_process_continue;
} // dhd_mlo_ipc_evt_queue_process()

int
dhd_mlo_ipc_wlioctl_intercept(dhd_pub_t *dhd_pub, int ifidx, wl_ioctl_t * ioc, void * buf)
{
	int ret = BCME_OK;
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	int ioctl_timeout_wait = 0;
	bool cmd_is_radio_off = FALSE;
	int slen = 0;
	struct bss_setbuf {
		int cfg;
		int val;
	} *params;
	uint8 mld_unit;

	DHD_TRACE(("%s: %s: cmd=\"%s\" ioc cmd<%u> len<%u>\n", dhd_ifname(dhd_pub, ifidx),
		__FUNCTION__, (char *)buf, ioc->cmd, ioc->len));

	if (dhd_mlo_ipc->suspended || dhd_mlo_ipc->iov_defer) {
		DHD_ERROR(("%s: %s: %s in progress cmd=\"%s\" ioc cmd<%u> len<%u>\n",
			dhd_ifname(dhd_pub, ifidx),
			__FUNCTION__,
			(dhd_mlo_ipc->suspended ? "MLO_SUSPEND" : "MLO_BSS_IOV_DEFER"),
			(char *)buf, ioc->cmd, ioc->len));

		if (dhd_mlo_ipc->suspended && (dhd_mlo_ipc->ioc_complete ||
				dhd_mlo_ipc->wl_down_pending)) {
			DHD_ERROR(("%s: %s: ioc_complete[%d] == 1 || wl_down_pending[%d] == 1\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				dhd_mlo_ipc->ioc_complete, dhd_mlo_ipc->wl_down_pending));

			// ASSERT(0);
			return BCME_BUSY;
		}

		if (dhd_mlo_ipc->iov_defer && dhd_mlo_ipc->ioc_pending) {
			return BCME_BUSY;
		}
		dhd_mlo_ipc->ioc_pending = TRUE;

		/* The peer AP has started the GR Down/BH procedure */
wait_retry:
		if (dhd_os_mlo_ipc_wait(dhd_pub, &dhd_mlo_ipc->ioc_complete) > 0) {
			dhd_mlo_ipc->ioc_complete = 0;
			dhd_mlo_ipc->iov_defer = 0;
			DHD_ERROR(("%s: %s: Resume the ioctl\n",
					dhd_ifname(dhd_pub, ifidx),
				__FUNCTION__));
			return BCME_OK;
		} else {
			/* Timeout */
			ioctl_timeout_wait++;
			DHD_ERROR(("%s: %s: cmd %d MLO IPC timeout %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__, ioc->cmd,
				ioctl_timeout_wait));
			if (ioctl_timeout_wait < MLO_GR_MAXWAIT)
				goto wait_retry;
			return -ETIMEDOUT;
		}
	}

	if ((ioc->cmd == WLC_SET_RADIO) && buf) {
		int val, mask;
		uint32 validbits = WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;
		bcopy(buf, (void*)&val, sizeof(val));
		mask = (val & 0xffff0000) >> 16;
		val = val & 0x0000ffff;
		cmd_is_radio_off = ((val & mask) & validbits) ? TRUE : FALSE;
	}

	if ((ioc->cmd != WLC_DOWN) && (ioc->cmd != WLC_REBOOT) && (ioc->cmd != WLC_INIT) &&
		(ioc->cmd != WLC_INIT_HALT) && (ioc->cmd != WLC_MLO_DOWN) && !cmd_is_radio_off &&
		(ioc->cmd != WLC_MLO_UP) && ((ioc->cmd != WLC_SET_VAR) || (buf == NULL))) {
		return ret;
	}

	/* Forward WLC_DOWN/WLC_REBOOT request to MAP to bringdown MLO */
	if ((ioc->cmd == WLC_DOWN) || (ioc->cmd == WLC_REBOOT) || cmd_is_radio_off) {
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: cmd<%u> mlc state is not active. mlc_state %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				ioc->cmd, dhd_mlo_ipc->mlc_state));
			goto done;
		}
		DHD_ERROR(("%s: dhd%d: cmd<%u> mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			ioc->cmd, dhd_mlo_ipc->mlc_state));
		dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_down_req_e);
#if defined(BCMMLO_GR)
	} else if (ioc->cmd == WLC_INIT || ioc->cmd == WLC_INIT_HALT) {
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: cmd<%u> mlc state is not active. mlc_state %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				ioc->cmd, dhd_mlo_ipc->mlc_state));
			goto done;
		}
		DHD_ERROR(("%s: dhd%d: cmd<%u> mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			ioc->cmd, dhd_mlo_ipc->mlc_state));
		if (ioc->cmd == WLC_INIT) {
			dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_bh_req_e);
		} else {
			/* WLC_INIT_HALT */
			dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_bh_halt_req_e);
		}
#endif /* BCMMLO_GR */
	} else if (ioc->cmd == WLC_MLO_DOWN) {
		if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_ready_e &&
			mlo_ipc_get_sys_mlo_down(dhd_mlo_ipc->ap_unit) != 0) {
			goto done;
		} else if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: mlc state is %d. Ignore cmd<%u>.\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				dhd_mlo_ipc->mlc_state, ioc->cmd));
			ret = BCME_ERROR;
			goto done;
		}

		DHD_ERROR(("%s: dhd%d: cmd<%u> mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			ioc->cmd, dhd_mlo_ipc->mlc_state));
		dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_down_bh_req_e);
	} else if (ioc->cmd == WLC_MLO_UP) {
		uint8 mlo_disable_ap_bm = mlo_ipc_get_mlo_disable_ap_bm();

		if (mlo_disable_ap_bm & NBITVAL(dhd_pub->unit)) {
			DHD_ERROR(("%s: %s: MLO is diabled. Ignore cmd<%u>.\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				ioc->cmd));
			ret = BCME_ERROR;
			goto done;
		} else if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_active_e ||
			mlo_ipc_get_sys_mlo_down(dhd_mlo_ipc->ap_unit) == 0) {
			goto done;
		} else if ((dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_sync_e) &&
			(dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_ready_e)) {
			DHD_ERROR(("%s: %s: mlc state is %d. Ignore cmd<%u>.\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				dhd_mlo_ipc->mlc_state, ioc->cmd));
			ret = BCME_NOTREADY;
			goto done;
		}
#ifdef BCM_ROUTER
		else if (!dhd_wl_isup(dhd_pub)) {
			DHD_ERROR(("%s: %s: Not up. Ignore cmd<%u>.\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				ioc->cmd));
			ret = BCME_NOTUP;
			goto done;
		}
#endif /* BCM_ROUTER */

		DHD_ERROR(("%s: dhd%d: cmd<%u> mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			ioc->cmd, dhd_mlo_ipc->mlc_state));
		ret = dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_up_req_e);
	} else if (buf && !strncmp(buf, "bss", 3) && FALSE /* intentional false */) {
		/* handle bss down for mld peers */
		slen = strlen("bss") + 1;
		params = (struct bss_setbuf*)((char *)buf + slen);
		mld_unit = dhd_get_mld_unit(dhd_pub, ifidx);

		if (params->val != WLC_AP_IOV_OP_DISABLE) {
			goto done;
		}
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			goto done;
		}
		if (!dhd_if_bss_up(dhd_pub, ifidx)) {
			goto done;
		}

		dhd_mlo_ipc_bcast_update_iov_defer(dhd_pub, mld_unit,
			MLO_IPC_EVT_IOV_SUBTYPE_DEFER_IOV);
		dhd_mlo_ipc->iov_defer = 1;

		DHD_INFO(("dhd%d: %s: set iov_defer[%d]\n", dhd_pub->unit, __FUNCTION__,
			dhd_mlo_ipc->iov_defer));

		dhd_msgbuf_set_ioctl(dhd_pub, ifidx, ioc->cmd, buf, ioc->len, ioc->set);
	} else {
		goto done;
	}

	if (ret != BCME_OK) {
		DHD_ERROR(("%s: cmd<%u> MLO_IPC error %d\n",
			dhd_ifname(dhd_pub, ifidx), ioc->cmd, ret));
		goto done;
	}

	DHD_INFO(("%s: WAIT for MLO_IPC to complete\n", dhd_ifname(dhd_pub, ifidx)));
	/* Wait for MLO_IPC work (DOWN/BH) to be complete */
	ASSERT(dhd_mlo_ipc->ioc_pending == FALSE);
	if (!dhd_mlo_ipc->iov_defer) {
		dhd_mlo_ipc->wl_down_pending = TRUE;
	}
	dhd_mlo_ipc->ioc_complete = 0;
	if (dhd_os_mlo_ipc_wait(dhd_pub, &dhd_mlo_ipc->ioc_complete) > 0) {
		/* MLO_IPC is done. Now continue with non-MLO work */
		dhd_mlo_ipc->ioc_complete = 0;
		return BCME_OK;
	} else {
		/* Timeout */
		DHD_ERROR(("%s: %s: cmd %d MLO IPC timeout\n",
			dhd_ifname(dhd_pub, ifidx), __FUNCTION__, ioc->cmd));
		dhd_mlo_ipc->iov_defer = 0;
		return -ETIMEDOUT;
	}

done:
	return ret;
}

/* Forward MLO_IPC state (DOWN/BH) to dongle through an IOVAR */
static int
dhd_mlo_ipc_wlioctl(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t *dhd_mlo_ipc, uint8 mlc_state)
{
	char	iovbuf[WLC_IOCTL_SMLEN] = { 0 };
	char	*cmd = NULL;
	wl_ioctl_t ioc;
	int	ret = BCME_OK;
	uint32	req_ap_unit;

	switch (mlc_state) {
	case mlo_ipc_mlc_down_e:
	{
		req_ap_unit = dhd_mlo_ipc->map->gr_req_ap_unit;
		cmd = "mlo_ipc_down";
		bcm_mkiovar(cmd, (char *)&req_ap_unit, sizeof(uint32), iovbuf, sizeof(iovbuf));

		ioc.cmd = WLC_SET_VAR;
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, ioc.cmd, iovbuf, sizeof(iovbuf), TRUE);
		break;
	}
	case mlo_ipc_mlc_down_complete_e:
		/* For Phase-1: Bring down all MLO APs */
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, WLC_DOWN, NULL, 0, TRUE);
		break;

#if defined(BCMMLO_GR)
	case mlo_ipc_mlc_bh_e:
	{
		req_ap_unit = dhd_mlo_ipc->map->gr_req_ap_unit;
		cmd = "mlo_ipc_bh";
		bcm_mkiovar(cmd, (char *)&req_ap_unit, sizeof(uint32), iovbuf, sizeof(iovbuf));

		ioc.cmd = WLC_SET_VAR;
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, ioc.cmd, iovbuf, sizeof(iovbuf), TRUE);
		break;
	}

	case mlo_ipc_mlc_bh_complete_e:
		/* Big hammer with all MLO APs */
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, WLC_INIT, NULL, 0, TRUE);
		break;
#endif /* BCMMLO_GR */

	case mlo_ipc_mlc_down_bh_e:
	{
		req_ap_unit = dhd_mlo_ipc->map->gr_req_ap_unit;
		cmd = "mlo_ipc_down_bh";
		bcm_mkiovar(cmd, (char *)&req_ap_unit, sizeof(uint32), iovbuf, sizeof(iovbuf));

		ioc.cmd = WLC_SET_VAR;
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, ioc.cmd, iovbuf, sizeof(iovbuf), TRUE);
		break;
	}

	case mlo_ipc_mlc_down_bh_complete_e:
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, WLC_MLO_DOWN, NULL, 0, TRUE);
		break;

	case mlo_ipc_mlc_up_e:
		ret = dhd_msgbuf_set_ioctl(dhd_pub, 0, WLC_MLO_UP, NULL, 0, TRUE);
		break;

	default:
		DHD_ERROR(("%s: dhd%d: unknown mlc_state %u\n",
			__FUNCTION__, dhd_pub->unit, mlc_state));
		return BCME_ERROR;
	}

	if (ret != BCME_OK) {
		DHD_ERROR(("%s: dhd%d: ioctl for mlc_state %u failed, ret %d\n",
			__FUNCTION__, dhd_pub->unit, mlc_state, ret));
	}

	return ret;
}

/* Check and process MLO Suspend state */
int
dhd_mlo_ipc_update_suspend_state(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	int ret = BCME_OK;

	if (dhd_mlo_ipc->suspended) {
		return BCME_OK;
	}

	if ((dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_suspend_e) &&
		(dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_suspend_halt_e)) {
		dhd_mlo_ipc_schedule_state_change(dhd_pub);
		return BCME_OK;
	}

	DHD_ERROR(("dhd%d: %s: ENTER suspended[%d] ioctl_in_progress[%s]\n",
		dhd_pub->unit, __FUNCTION__, dhd_mlo_ipc->suspended,
		dhd_prot_ioctl_in_progress(dhd_pub) ? "TRUE":"FALSE"));

	/* Wait for IOCTL in progress to complete */
	if (dhd_prot_ioctl_in_progress(dhd_pub)) {
		return BCME_OK;
	}

	dhd_mlo_ipc->suspended = TRUE;

	/* Send mlc_state event to dongle. */
	dhdpcie_mlo_h2d_notify(dhd_pub->bus, dhd_mlo_ipc->mlc_state);

	return ret;
} /* dhd_mlo_ipc_update_suspend_state() */

static int
dhd_mlo_ipc_set_mlc_state(dhd_mlo_ipc_t * dhd_mlo_ipc, mlo_ipc_mlc_state_t mlc_state)
{
	int ret = BCME_OK;

	dhd_mlo_ipc->mlc_state = mlc_state;

	mlo_ipc_set_mlc_state(dhd_mlo_ipc->ap_unit, mlc_state);

	return ret;
} /* dhd_mlo_ipc_set_mlc_state() */

#ifdef MLO_BCMC
/* Queue BCMC XMIT msg to one of the participating aux link */
int
dhd_mlo_ipc_evt_bcmc_xmit_req(dhd_pub_t *dhd_pub, uint16 dest_ap_unit,
	void *pkt, uint16 seq, int8 mld_unit)
{
	mlo_ipc_evt_msg_t * evt_msg;
	mlo_ipc_bcmc_xmit_msg_t *xmit_msg;
	uint16	evt_msg_bytes;
	int	ret;

	evt_msg_bytes = sizeof(mlo_ipc_evt_msg_t) + sizeof(mlo_ipc_bcmc_xmit_msg_t);

	DHD_INFO(("%s: dhd%d: Sending BCMC XMIT req event len %d to ap_unit %d\n",
		__FUNCTION__, dhd_pub->unit, evt_msg_bytes, dest_ap_unit));

	evt_msg = mlo_ipc_evt_msg_alloc(evt_msg_bytes);

	if (evt_msg == NULL) {
		DHD_ERROR(("dhd%d: %s: MLO IPC EVENT MSG alloc failed\n",
			dhd_pub->unit, __FUNCTION__));
		ASSERT(evt_msg);
		return BCME_NOMEM;
	}

	evt_msg->ap_unit	= dhd_pub->unit;
	evt_msg->evt_id		= MLO_IPC_EVT_ID_BCMC_XMIT;
	evt_msg->evt_flags	= 0;
	evt_msg->bytes		= evt_msg_bytes;
	xmit_msg		= (mlo_ipc_bcmc_xmit_msg_t*)(evt_msg->data);
	xmit_msg->map_seq	= seq;
	xmit_msg->mld_unit	= mld_unit;
	xmit_msg->pkt		= (uint64)(uintptr)pkt;

	ret = mlo_ipc_evt_queue_enqueue(dhd_pub->osh, dest_ap_unit, evt_msg);

	if (ret	!= BCME_OK) {
		DHD_INFO(("%s: BCMC XMIT EVT enqueue to ap[%d] failed ret %d\n",
			__FUNCTION__, dest_ap_unit, ret));
		/* Release event message */
		mlo_ipc_evt_msg_free(evt_msg);
	}

	return ret;
} // dhd_mlo_ipc_evt_bcmc_xmit_req
#endif /* MLO_BCMC */

bool
dhd_mlo_ipc_is_active(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	return (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_active_e);
}

bool
dhd_mlo_ipc_is_ready(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	return (dhd_mlo_ipc->mlc_state >= mlo_ipc_mlc_ready_e) ? TRUE : FALSE;
}

uint16
dhd_mlo_get_dump_signature(void)
{
	return mlo_ipc_sys_gp->dump_signature;
}

void
dhd_mlo_ipc_dump(dhd_pub_t *dhd_pub, struct bcmstrbuf *b)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	if (dhd_mlo_ipc == NULL)
		return;

	bcm_bprintf(b, "mlc_state[%u]:%s req_mlc_state[%u]:%s\n",
		dhd_mlo_ipc->mlc_state, __mlo_ipc_mlc_state_str[dhd_mlo_ipc->mlc_state],
		dhd_mlo_ipc->req_mlc_state, __mlo_ipc_mlc_state_str[dhd_mlo_ipc->req_mlc_state]);

	mlo_ipc_dump(dhd_pub->unit, b);
}

/* Queue msg to one of the participating aux link */
static int
dhd_mlo_ipc_enqueue_evt_iov_msg(dhd_pub_t *dhd_pub, uint8 dest_ap_unit, mlo_ipc_iov_msg_t *iov_msg)
{
	mlo_ipc_evt_msg_t *evt_msg;
	uint16	evt_msg_bytes;
	int	ret;

	evt_msg_bytes = sizeof(mlo_ipc_evt_msg_t) + sizeof(mlo_ipc_iov_msg_t);

	DHD_INFO(("%s: dhd%d: Sending IOV_MSG req event len %d\n",
		__FUNCTION__, dhd_pub->unit, evt_msg_bytes));

	evt_msg = mlo_ipc_evt_msg_alloc(evt_msg_bytes);

	if (evt_msg == NULL) {
		DHD_ERROR(("dhd%d: %s: MLO IPC EVENT MSG alloc failed\n",
			dhd_pub->unit, __FUNCTION__));
		ASSERT(evt_msg);
		return BCME_NOMEM;
	}

	evt_msg->ap_unit	= dhd_pub->unit;
	evt_msg->evt_id		= MLO_IPC_EVT_ID_IOV_MSG;
	evt_msg->evt_flags	= 0;
	evt_msg->bytes		= evt_msg_bytes;

	memcpy((uint8*)evt_msg->data, (uint8*)iov_msg, sizeof(mlo_ipc_iov_msg_t));

	ret = mlo_ipc_evt_queue_enqueue(dhd_pub->osh, dest_ap_unit, evt_msg);

	if (ret	!= BCME_OK) {
		DHD_INFO(("%s: IOV_MSG EVT enqueue to ap[%d]failed ret %d\n",
			__FUNCTION__, dest_ap_unit, ret));
		/* Release event message */
		mlo_ipc_evt_msg_free(evt_msg);
	}

	return ret;
} // dhd_mlo_ipc_enqueue_evt_iov_msg

static int
dhd_mlo_ipc_bcast_update_iov_defer(dhd_pub_t *dhd_pub, int8 mld_unit, uint msg_subtype)
{
	mlo_ipc_iov_msg_t	iov_msg;
	uint8	iter;
	uint8	mld_links_bmp;

	memset(&iov_msg, 0, sizeof(iov_msg));
	iov_msg.msg_subtype = msg_subtype;
	iov_msg.mld_unit = mld_unit;
	mld_links_bmp = mlo_ipc_get_ap_bm();

	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		if (iter == dhd_pub->unit) {
			continue;
		}
		if (mld_links_bmp & NBITVAL(iter)) {
			DHD_INFO(("%s: dhd%d send msg subtype[%s] to ap unit[%d]\n",
				__FUNCTION__, dhd_pub->unit,
				(msg_subtype == 1 ? "DEFER_IOV_HNDLING" : "ALLOW_IOV_HNDLING"),
				iter));
			dhd_mlo_ipc_enqueue_evt_iov_msg(dhd_pub, iter, &iov_msg);
		}
	}
	return BCME_OK;
}

void
dhd_mlo_ipc_process_mld_down_event(dhd_pub_t *dhd_pub, void *event_data)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	mld_link_info_t *mld_info;

	if (dhd_mlo_ipc == NULL)
		return;

	if (!dhd_mlo_ipc->iov_defer) {
		DHD_INFO(("%s: dhd%d no iovar defer set, ignore mld down event\n",
			__FUNCTION__, dhd_pub->unit));
		return;
	}

	DHD_INFO(("%s: dhd%d IOV CMPLT RCVD from all aux, including self"
		" ALLOW IOV HNDLING at self, and auxes, ioc_pending[%d]\n",
		__FUNCTION__, dhd_pub->unit, dhd_mlo_ipc->ioc_pending));

	mld_info = (mld_link_info_t*)event_data;
	/* broadcast to other mld peers, mld_unit not required */
	dhd_mlo_ipc_bcast_update_iov_defer(dhd_pub, mld_info->mld_unit,
		MLO_IPC_EVT_IOV_SUBTYPE_ALLOW_IOV);

	if (dhd_mlo_ipc->ioc_pending) {
		/* wake up */
		dhd_mlo_ipc->ioc_complete = 1;
		dhd_os_mlo_ipc_wake(dhd_pub);
		dhd_mlo_ipc->ioc_pending = 0;
	}
	dhd_mlo_ipc->iov_defer = 0;
	return;
}

bool
dhd_mlo_ipc_is_ap_trapped(uint16 ap_unit)
{
	return mlo_ipc_get_ap_trap_complete(ap_unit);
}

void
dhd_mlo_ipc_set_ap_trapped(uint16 ap_unit)
{
	mlo_ipc_set_ap_trap_complete(ap_unit);
}

static void
dhd_mlo_handle_role_reconfig(dhd_pub_t *dhd_pub, uint8 new_mlo_unit)
{
	dhd_mlo_ipc_t	*dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	DHD_ERROR(("dhd%d: %s: new_mlo_unit %d\n",
		dhd_pub->unit, __FUNCTION__, new_mlo_unit));

	ASSERT(dhd_mlo_ipc->wq_event_msg_queue == 0);
	ASSERT(dhd_mlo_ipc->wq_event_state_change == 0);

	if (DHD_IS_MLO_MAP(dhd_pub)) {
		MFREE(dhd_pub->osh, dhd_mlo_ipc->map, sizeof(dhd_mlo_ipc_map_t));
		dhd_mlo_ipc->map = NULL;
	}

	if ((new_mlo_unit == PCIE_IPC_MLO_UNIT_MAP) &&
		(dhd_mlo_ipc_map_init(dhd_pub, dhd_mlo_ipc) != BCME_OK)) {
		DHD_ERROR(("dhd%d: %s: map init failure\n",
			dhd_pub->unit, __FUNCTION__));
		ASSERT(0);
	}
	dhd_mlo_ipc->pcie_ipc_mlo_ap->mlo_unit = new_mlo_unit;

	return;
}

static int
dhd_mlo_ipc_restart(void *data)
{
	dhd_pub_t *dhd_pub = (dhd_pub_t*)data;
	uint8 mlo_ipc_started = 0;
	int ret;

	DHD_LOCK(dhd_pub);

	DHD_ERROR(("dhd%d: %s: MLO IPC restart for role reassignment\n",
		dhd_pub->unit, __FUNCTION__));

	mlo_ipc_mlo_unit_reconfig();
	ret = dhd_mlo_ipc_start(dhd_pub, &mlo_ipc_started);

	DHD_UNLOCK(dhd_pub);

	if (ret == BCME_OK) {
		if (mlo_ipc_started != 0) {
			return BCME_OK;
		} else {
			return BCME_NOTREADY;
		}
	}

	return BCME_ERROR;
}

void *
dhd_mlo_ipc_get_bhm_map(dhd_pub_t *dhd_pub, uint32 size, uint32 align_bits,
	uint32 *alloced, dmaaddr_t *pap)
{
	void *va;

	va = mlo_ipc_get_bhm_map(size, align_bits, pap);

	if (va) {
		*alloced = size;
	}

	DHD_ERROR(("dhd%d: %s: MLO IPC get BHM MAP va %px size %u\n",
		dhd_pub->unit, __FUNCTION__, va, size));

	return va;
}

bool
dhd_is_mlo_default_map(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	return DHD_IS_MLO_AP(dhd_pub) &&
		(mlo_ipc_get_ap_unit_default_map() == dhd_mlo_ipc->ap_unit);
}

static int
dhd_mlo_ipc_isup(void *data)
{
	dhd_pub_t *dhd_pub = (dhd_pub_t*)data;
	bool isup = TRUE;

#ifdef BCM_ROUTER
	DHD_LOCK(dhd_pub);
	isup = dhd_wl_isup(dhd_pub);
	DHD_UNLOCK(dhd_pub);
#endif /* BCM_ROUTER */

	if (isup) {
		return BCME_OK;
	} else {
		return BCME_NOTREADY;
	}
}

static int
dhd_mlo_ipc_generic_cb(mlo_ipc_cb_t cb, void *data, int ap_unit,
	uint32 param1, uint32 param2, uint32 param3)
{
	dhd_pub_t *dhd_pub = (dhd_pub_t*)data;
	int ret = BCME_OK;
#if !defined(OSL_CACHE_COHERENT)
	uintptr ul_addr;
	dmaaddr_t addr;
#endif /* OSL_CACHE_COHERENT */

	BCM_REFERENCE(dhd_pub);

	ASSERT(dhd_pub->unit == (uint)ap_unit);

	switch (cb) {
	case MLO_IPC_CB_MAP_MSG_ENQ:
		dhd_mlo_ipc_map_msg_enqueue(ap_unit, (uint8)param1, (uint8)param2,
			(uint8)param3, data);
		break;
	case MLO_IPC_CB_AAP_STATE_CHG:
		dhd_mlo_ipc_aap_state_change_cb(ap_unit, (uint8)param1, data);
		break;
	case MLO_IPC_CB_EVT_WAKE_UP:
		dhd_mlo_ipc_evt_wake_up_cb(data);
		break;
	case MLO_IPC_CB_RESTART:
		ret = dhd_mlo_ipc_restart(data);
		break;
	case MLO_IPC_CB_WL_ISUP:
		ret = dhd_mlo_ipc_isup(data);
		break;
	case MLO_IPC_CB_COMMIT_SYS_STATE:
#if !defined(OSL_CACHE_COHERENT)
		addr.lo = param1;
		addr.hi = param2;
		PHYSADDRTOULONG(addr, ul_addr);
		/* push updates to devices, flush also invalidates the line */
		OSL_CACHE_FLUSH(dhd_pub->osh, (void *)ul_addr, param3);
#endif /* OSL_CACHE_COHERENT */
		break;
	case MLO_IPC_CB_RESYNC_SYS_STATE:
#if !defined(OSL_CACHE_COHERENT)
		addr.lo = param1;
		addr.hi = param2;
		PHYSADDRTOULONG(addr, ul_addr);
		/* invalidate to sync any device update */
		OSL_CACHE_INV(dhd_pub->osh, (void *)ul_addr, param3);
#endif /* OSL_CACHE_COHERENT */
		break;
	default:
		DHD_ERROR(("dhd%d: %s: MLO IPC CALLBACK[%d] not yet implemented\n",
			dhd_pub->unit, __FUNCTION__, cb));
		ASSERT(0);
		break;
	}

	return ret;
}

/* called with DHD_LOCK held */
static void
dhd_mlo_ipc_schedule_state_change(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

	if ((dhd_mlo_ipc->req_mlc_state == mlo_ipc_mlc_reset_e) ||
		(dhd_mlo_ipc->wq_event_state_change != 0)) {
		return;
	}

	DHD_ERROR(("dhd%d: %s: req_mlc_state %u\n",
		dhd_pub->unit, __FUNCTION__, dhd_mlo_ipc->req_mlc_state));

	ASSERT((DHD_IS_MLO_MAP(dhd_pub) == FALSE) ||
		(dhd_mlo_ipc->req_mlc_state == mlo_ipc_mlc_up_e));

	dhd_mlo_ipc->wq_event_state_change = ~0;
	wake_up_interruptible(&dhd_mlo_ipc->wq_head);
}
#endif /* MLO_IPC */
