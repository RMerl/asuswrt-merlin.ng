/*
 * +--------------------------------------------------------------------------+
 * DHD_MLO_IPC: Inter Processor communication between MLO AP's
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: dhd_mlo_ipc.c 833865 2023-12-06 11:05:47Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <pcicfg.h>
#include <pcie_core.h>
#include <mlo_ipc.h>
#include <dhd.h>
#include <dhd_proto.h>
#include <dhd_pcie.h>
#include <dhd_dbg.h>
#include <dhd_mlo_ipc.h>
#include <dhd_macdbg.h>
#include <dhd_linux.h>

#if defined(MLO_IPC)

#define MLO_IPC_PREFIX	"\e[0;32m[MLO::IPC] "
#define MLO_IPC_POSTFIX	"\e[0m\n"

#define DHD_MLO_IPC_EVT_QUEUE_BUDGET	64

#ifndef AP_UNIT_INV
#define AP_UNIT_INV	PCIE_IPC_AP_UNIT_INV
#endif

// MAP object
typedef struct dhd_mlo_ipc_map {
	mlo_ipc_msg_t	msg;	// Copy of State Transition request in progress
	uint8	mlc_state;	// Current FSA State
	uint8	next_state;	// FSA pending state
	uint8	gr_req_ap_unit;	// Graceful Recovery requesting AP unit
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
static void	dhd_mlo_ipc_process_state_change(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc,
                                                 uint8 mlc_state);

static void	dhd_mlo_ipc_evt_queue_process(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc,
	bool bound);
static void	dhd_mlo_ipc_evt_wake_up_cb(void *data);

static int	dhd_mlo_ipc_wlioctl(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t *dhd_mlo_ipc,
	uint8 mlc_state);
static int	dhd_mlo_ipc_set_mlc_state(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc,
	uint8 mlc_state);

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
	dhd_mlo_ipc->req_mlc_state = ~0;
	dhd_mlo_ipc->mlc_state = mlo_ipc_mlc_reset_e;
	dhd_mlo_ipc->suspended = FALSE;

	// Register MLO AP context with System wide MLO_IPC
	if (mlo_unit == PCIE_IPC_MLO_UNIT_MAP) {	// Is MAP?
		ret = dhd_mlo_ipc_map_init(dhd_pub, dhd_mlo_ipc);
		if (ret != BCME_OK) {
			goto dhd_mlo_ipc_init_fail;
		}
		mlo_ipc_map_register(ap_unit, dhd_mlo_ipc_map_msg_enqueue,
			dhd_mlo_ipc_evt_wake_up_cb, (void *)dhd_pub, PCIE_IPC_MLO_OPMODE_FD);
	} else {
		mlo_ipc_aap_register(ap_unit, dhd_mlo_ipc_aap_state_change_cb,
			dhd_mlo_ipc_evt_wake_up_cb, (void *)dhd_pub, PCIE_IPC_MLO_OPMODE_FD);
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

	if (dhd_mlo_ipc->kthread) {
		kthread_stop(dhd_mlo_ipc->kthread);
	}

#ifdef BCM_ROUTER
	dhdpcie_bus_bridge_deinit(dhd_pub);
#endif /* BCM_ROUTER */

	if (DHD_IS_MLO_MAP(dhd_pub)) {
		mlo_ipc_map_deregister(dhd_pub->unit);

		MFREE(dhd_pub->osh, dhd_mlo_ipc->map, sizeof(dhd_mlo_ipc_map_t));
		dhd_mlo_ipc->map = NULL;
	} else {
		mlo_ipc_aap_deregister(dhd_pub->unit);
	}

	MFREE(dhd_pub->osh, dhd_mlo_ipc, len);
	dhd_pub->dhd_mlo_ipc = NULL;

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

			ASSERT(DHD_IS_MLO_MAP(dhd_pub) == FALSE);

			DHD_INFO(("%s: ap_unit[%d] received STATE_CHANGE event\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit));

			// Reset Wait Event
			dhd_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
			dhd_mlo_ipc->wq_event_state_change = 0;

			dhd_mlo_ipc_process_state_change(dhd_pub, dhd_mlo_ipc,
				req_mlc_state);
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
	msg->req_ap_bm |= (1 << msg_unit);

	// Add msg to the active list
	if (msg_prio == MLO_IPC_MSG_PRIO_HIGH) {
		// Enqueue high prio messages to the head
		dll_prepend(&msg_queue->active_list, &msg->node);
	} else {
		dll_append(&msg_queue->active_list, &msg->node);
	}

	msg_queue->enqueue_cnt++;

	MLO_IPC_MSG_QUEUE_UNLK(); // --------------------------------------------------------------

	// Inform MAP of new message
	DHD_INFO(("%s: ap_unit[%d] informs MAP of new message[%d]\n",
		__FUNCTION__, ap_unit, msg_id));
	dhd_mlo_ipc->wq_event_msg_queue = ~0;
	wake_up_interruptible(&dhd_mlo_ipc->wq_head);

} // dhd_mlo_ipc_map_msg_enqueue()

/** Broadcast new state to all AAP's */
static INLINE void
dhd_mlo_ipc_map_state_broadcast(dhd_pub_t * dhd_pub, uint8 mlc_state)
{
	int	iter;
	uint8	mlo_ap_bm;
	uint8	ap_unit_map = mlo_ipc_get_ap_unit_map();

	// Broadcast new state to all AAP's
	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		mlo_ipc_aap_state_change_cb_t	state_change_cb;
		void *aap_data;

		mlo_ap_bm = mlo_ipc_get_ap_bm();

		DHD_INFO(("%s: iter[%d]: ap_unit_map[%d] mlo_ap_bm[%x]\n", __FUNCTION__,
			iter, ap_unit_map, mlo_ap_bm));
		if ((iter == ap_unit_map) || ((mlo_ap_bm & (1 << iter)) == 0)) {
			// Don't send state transition request to MAP and non MLO AP
			continue;
		}

		state_change_cb = mlo_ipc_get_state_change_cb(iter);
		aap_data = mlo_ipc_get_aap_data(iter);
		DHD_UNLOCK(dhd_pub);
		state_change_cb(iter, mlc_state, aap_data);
		DHD_LOCK(dhd_pub);

	}
}

/** Parse the messge from MLO AP and broadcast state transition to all MLO AP's */
static INLINE void
dhd_mlo_ipc_map_msg_process(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, mlo_ipc_msg_t * msg)
{
	dhd_mlo_ipc_map_t * map;
	uint8	new_mlc_state;
	uint8	mlo_ap_bm = mlo_ipc_get_ap_bm();

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
			if (map->mlc_state != mlo_ipc_mlc_bind_e) {
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
			if (map->mlc_state != mlo_ipc_mlc_link_e) {
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
			if (map->mlc_state != mlo_ipc_mlc_bridge_e) {
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
			if (mlo_ap_bm != msg->req_ap_bm) {
				return; // Wait for event from all MLO AP's
			}

			new_mlc_state = mlo_ipc_mlc_ready_e;
			break;

		case MLO_IPC_MSG_ID_TRAP:
			new_mlc_state = mlo_ipc_mlc_halt_e;
			break;

		case MLO_IPC_MSG_ID_DOWN_REQ:
			ASSERT(map->mlc_state == mlo_ipc_mlc_active_e);
			/* Perform MLO_SUSPEND on all MLO links */
			new_mlc_state = mlo_ipc_mlc_suspend_e;
			map->next_state = mlo_ipc_mlc_down_e;
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
			break;
#endif /* BCMMLO_GR */

		case MLO_IPC_MSG_ID_DOWN_BH_REQ:
			ASSERT(map->mlc_state == mlo_ipc_mlc_active_e);
			new_mlc_state = mlo_ipc_mlc_suspend_e;
			map->next_state = mlo_ipc_mlc_down_bh_e;
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
			if (mlo_ap_bm != msg->req_ap_bm) {
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

	DHD_INFO(("%s: ap_unit[%d] - Broadcasting state[%d] transition to all MLO AP's\n",
		__FUNCTION__, dhd_mlo_ipc->ap_unit, new_mlc_state));
	dhd_mlo_ipc_map_state_broadcast(dhd_pub, new_mlc_state);

map_state_transition:
	// Process state change in MAP
	map->mlc_state = new_mlc_state;
	dhd_mlo_ipc_process_state_change(dhd_pub, dhd_mlo_ipc, new_mlc_state);

	// Reset request in progess
	msg->id = ~0;

	return;
} // dhd_mlo_ipc_map_msg_process()

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

	if (dll_empty(&msg_queue->active_list)) {
		// No more messages to process
		// Reset msg_queue pending work
		dhd_mlo_ipc->wq_event_msg_queue = 0;
		return;
	}

	MLO_IPC_MSG_QUEUE_LOCK(); // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	item = dll_head_p(&msg_queue->active_list);
	msg = container_of(item, mlo_ipc_msg_t, node);
	// Remove the msg node from active list
	dll_delete(&msg->node);
	msg_queue->dequeue_cnt++;

	DHD_INFO(("%s: map msg.id[%d] bm[%d] msg id[%d] bm[%d]\n",
		__FUNCTION__, map->msg.id, map->msg.req_ap_bm, msg->id, msg->req_ap_bm));
	// Copy the request to DHD context
	if (map->msg.id == msg->id) {
		// Request in process. Copy the udpated msg values
		map->msg.req_ap_bm |= msg->req_ap_bm;
	} else {
		// New request. Get full message
		map->msg = *msg;
		if (map->msg.id == MLO_IPC_MSG_ID_DOWN_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_BH_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_BH_HALT_REQ ||
			map->msg.id == MLO_IPC_MSG_ID_DOWN_BH_REQ) {
			uint8 req_ap_unit = bcm_find_fsb(msg->req_ap_bm);
			ASSERT(req_ap_unit > 0);
			// Keep the requesting AP unit with DOWN and BH requests
			map->gr_req_ap_unit = req_ap_unit - 1;
		} else if (map->msg.id != MLO_IPC_MSG_ID_SUSPEND &&
			map->msg.id != MLO_IPC_MSG_ID_DOWN &&
			map->msg.id != MLO_IPC_MSG_ID_BH &&
			map->msg.id != MLO_IPC_MSG_ID_DOWN_BH) {
			map->gr_req_ap_unit = (uint8)AP_UNIT_INV;
		}
	}

	DHD_TRACE(("%s: id[%d] prio[%d] req_ap_bm[%d]\n", __FUNCTION__,
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
static void
dhd_mlo_ipc_process_state_change(dhd_pub_t * dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, uint8 mlc_state)
{
	int ap_unit = dhd_mlo_ipc->ap_unit;

	if (mlc_state < mlo_ipc_mlc_max_e) {
		DHD_ERROR((MLO_IPC_PREFIX "ap_unit[%d] enters %s" MLO_IPC_POSTFIX,
			ap_unit, __mlo_ipc_mlc_state_str[mlc_state]));
	}

	switch (mlc_state) {

		case mlo_ipc_mlc_bind_e:
		case mlo_ipc_mlc_link_e:
		case mlo_ipc_mlc_bridge_e:
		case mlo_ipc_mlc_sync_e:
			dhd_mlo_ipc->mlc_state = mlc_state;
#ifdef BCM_ROUTER
			if (mlc_state == mlo_ipc_mlc_bridge_e &&
				dhdpcie_bus_bridge_init(dhd_pub) != BCME_OK) {
				ASSERT(0);
				break;
			}
#endif /* BCM_ROUTER */
			/* Send mlc_state event to dongle. */
			dhdpcie_mlo_h2d_notify(dhd_pub->bus, mlc_state);
			break;

		case mlo_ipc_mlc_ready_e:
			if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_sync_e) {
				/* Send mlc_state event to dongle. */
				dhdpcie_mlo_h2d_notify(dhd_pub->bus, mlc_state);
			}
			dhd_mlo_ipc->mlc_state = mlc_state;
			break;

		case mlo_ipc_mlc_suspend_e:
		case mlo_ipc_mlc_suspend_halt_e:
			DHD_ERROR(("dhd%d: %s: MLO_SUSPEND request\n", ap_unit, __FUNCTION__));
			dhd_mlo_ipc->mlc_state = mlc_state;
			dhd_mlo_ipc_update_suspend_state(dhd_pub);
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
			dhd_mlo_ipc->mlc_state = mlc_state;
			/* Use iovar instead of a mailbox event */
			dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlc_state);
			break;
		}

		case mlo_ipc_mlc_down_complete_e:
#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_complete_e:
#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_complete_e:
		{
			dhd_mlo_ipc->mlc_state = mlc_state;
			dhd_mlo_ipc->suspended = FALSE;

			DHD_ERROR((MLO_IPC_PREFIX "%s: ap_unit[%d] continues with non-MLO %s"
				MLO_IPC_POSTFIX, __FUNCTION__, ap_unit,
				(mlc_state == mlo_ipc_mlc_down_complete_e) ? "down" : "bh"));

			/* MLO down is complete so revert the state to mlc_ready */
			dhd_mlo_ipc_set_mlc_state(dhd_pub, dhd_mlo_ipc, mlo_ipc_mlc_ready_e);

			if (dhd_mlo_ipc->wl_down_pending) {
				ASSERT(dhd_mlo_ipc->ioc_pending == FALSE);
				/* Resume and continue iovar processig for non-MLO (wl down) */
				dhd_mlo_ipc->wl_down_pending = FALSE;
				dhd_mlo_ipc->ioc_complete = 1;
				dhd_os_mlo_ipc_wake(dhd_pub);
			} else {
				/* Bring down/re-init all xAPs */
				dhd_mlo_ipc_wlioctl(dhd_pub, dhd_mlo_ipc, mlc_state);

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

			dhd_mlo_ipc->mlc_state = mlc_state;
			break;

		default:
			DHD_ERROR(("%s: ap_unit[%d] Invalid state[%d]\n",
				__FUNCTION__, ap_unit, mlc_state));
			ASSERT(0);
			break;
	}

} // dhd_mlo_ipc_process_state_change()

int
dhd_mlo_ipc_process_dongle_event(dhd_pub_t *dhd_pub, uint8 mlc_state)
{
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	uint8	msg_id;
	uint8	msg_prio;
	mlo_ipc_map_msg_enqueue_fn_t msg_enqueue_fn;
	void	*map_data;
	uint8	ap_unit_map;

	if (!DHD_IS_MLO_AP(dhd_pub)) {
		return BCME_ERROR;
	}

	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);

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
			DHD_ERROR(("dhd%d reporting a dongle TRAP\n", dhd_mlo_ipc->ap_unit));
			/* If a trap event happened before, don't bother to halt other
			 * interfaces again.
			 */
			if (dhd_mlo_ipc->mlc_state == mlo_ipc_mlc_halt_e) {
				return BCME_OK;
			}
			msg_id = MLO_IPC_MSG_ID_TRAP;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			dhd_mlo_ipc->mlc_state = mlo_ipc_mlc_trap_e;
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

		case mlo_ipc_mlc_active_e:
			dhd_mlo_ipc_set_mlc_state(dhd_pub, dhd_mlo_ipc, mlo_ipc_mlc_active_e);
			DHD_ERROR(("dhd%d: is MLO_ACTIVE\n", dhd_mlo_ipc->ap_unit));
			/* Nothing to do. All dongle links will inform the DHD */
			return BCME_OK;

		default:
			DHD_ERROR(("%s: ap_unit[%d]: Unexpected mlc_state[%d] from dongle\n",
				__FUNCTION__, dhd_mlo_ipc->ap_unit, mlc_state));
			return BCME_ERROR;
	}

	msg_enqueue_fn = mlo_ipc_get_map_msg_enqueue_fn();
	map_data = mlo_ipc_get_map_data();
	ap_unit_map = mlo_ipc_get_ap_unit_map();

	msg_enqueue_fn(ap_unit_map, msg_id,
		msg_prio, dhd_mlo_ipc->ap_unit, map_data);

	return BCME_OK;
} // dhd_mlo_ipc_process_dongle_event()

int
dhd_mlo_ipc_start(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t	* dhd_mlo_ipc;
	uint8	mlo_ap_bm; /* registered APs */
	uint8	sys_ap_bm; /* nvram configured APs */

	if (!DHD_IS_MLO_AP(dhd_pub)) {
		/* Non-MLO AP */
		return BCME_OK;
	}

	dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	dhd_mlo_ipc->mlc_state = mlo_ipc_mlc_attach_e;

	sys_ap_bm = mlo_ipc_get_sys_ap_bm();
	ASSERT(sys_ap_bm);

	mlo_ap_bm = mlo_ipc_set_ap_bm(dhd_mlo_ipc->ap_unit);

	DHD_TRACE(("%s: mlo_ap_bm %x expecting %x\n", __FUNCTION__,
		mlo_ap_bm, sys_ap_bm));

	if (sys_ap_bm == mlo_ap_bm) {
		DHD_ERROR(("%s: All MLO APs registered.\n", __FUNCTION__));
		return dhd_mlo_ipc_process_dongle_event(dhd_pub, dhd_mlo_ipc->mlc_state);
	}

	return BCME_OK;
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
	evt_msg = mlo_ipc_evt_queue_dequeue(dhd_pub->unit);
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

	DHD_TRACE(("%s: %s: cmd=\"%s\" ioc cmd<%u> len<%u>\n", dhd_ifname(dhd_pub, ifidx),
		__FUNCTION__, (char *)buf, ioc->cmd, ioc->len));

	if (dhd_mlo_ipc->suspended) {
		DHD_ERROR(("%s: %s: MLO_SUSPEND in progress cmd=\"%s\" ioc cmd<%u> len<%u>\n",
			dhd_ifname(dhd_pub, ifidx),
			__FUNCTION__, (char *)buf, ioc->cmd, ioc->len));

		if (dhd_mlo_ipc->ioc_complete || dhd_mlo_ipc->wl_down_pending) {
			DHD_ERROR(("%s: %s: ioc_complete[%d] == 1 || wl_down_pending[%d] == 1\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				dhd_mlo_ipc->ioc_complete, dhd_mlo_ipc->wl_down_pending));

			// ASSERT(0);
			return BCME_BUSY;
		}

		dhd_mlo_ipc->ioc_pending = TRUE;

		/* The peer AP has started the GR Down/BH procedure */
		if (dhd_os_mlo_ipc_wait(dhd_pub, &dhd_mlo_ipc->ioc_complete) > 0) {
			dhd_mlo_ipc->ioc_complete = 0;
			DHD_ERROR(("%s: %s: Resume the ioctl\n",
					dhd_ifname(dhd_pub, ifidx),
				__FUNCTION__));
			return BCME_OK;
		} else {
			/* Timeout */
			DHD_ERROR(("%s: %s: cmd %d MLO IPC timeout\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__, ioc->cmd));
			return -ETIMEDOUT;
		}
	}

	if ((ioc->cmd != WLC_DOWN) && (ioc->cmd != WLC_REBOOT) && (ioc->cmd != WLC_INIT) &&
		(ioc->cmd != WLC_INIT_HALT) && (ioc->cmd != WLC_MLO_DOWN) &&
		((ioc->cmd != WLC_SET_VAR) || (buf == NULL))) {
		return ret;
	}

	/* Forward WLC_DOWN/WLC_REBOOT request to MAP to bringdown MLO */
	if ((ioc->cmd == WLC_DOWN) || (ioc->cmd == WLC_REBOOT)) {
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: mlc state is not active. mlc_state %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__, dhd_mlo_ipc->mlc_state));
			goto done;
		}

		DHD_ERROR(("%s: dhd%d: mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			dhd_mlo_ipc->mlc_state));
		dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_down_req_e);

#if defined(BCMMLO_GR)
	} else if (ioc->cmd == WLC_INIT || ioc->cmd == WLC_INIT_HALT) {
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: mlc state is not active. mlc_state %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__, dhd_mlo_ipc->mlc_state));
			goto done;
		}

		DHD_ERROR(("%s: dhd%d: mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			dhd_mlo_ipc->mlc_state));
		if (ioc->cmd == WLC_INIT) {
			dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_bh_req_e);
		} else {
			/* WLC_INIT_HALT */
			dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_bh_halt_req_e);
		}
#endif /* BCMMLO_GR */
	} else if (ioc->cmd == WLC_MLO_DOWN) {
		if (dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			DHD_ERROR(("%s: %s: mlc state is not active. mlc_state %d\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__, dhd_mlo_ipc->mlc_state));
			goto done;
		}

		DHD_ERROR(("%s: dhd%d: mlc_state %d\n",  __FUNCTION__, dhd_pub->unit,
			dhd_mlo_ipc->mlc_state));
		dhd_mlo_ipc_process_dongle_event(dhd_pub, mlo_ipc_mlc_down_bh_req_e);
	} else {
		goto done;
	}

	DHD_ERROR(("%s: WAIT for MLO_IPC_DOWN to complete\n", dhd_ifname(dhd_pub, ifidx)));
	/* Wait for MLO_IPC work (DOWN/BH) to be complete */
	ASSERT(dhd_mlo_ipc->ioc_pending == FALSE);
	dhd_mlo_ipc->wl_down_pending = TRUE;
	dhd_mlo_ipc->ioc_complete = 0;
	if (dhd_os_mlo_ipc_wait(dhd_pub, &dhd_mlo_ipc->ioc_complete) > 0) {
		/* MLO_IPC is done. Now continue with non-MLO work */
		dhd_mlo_ipc->ioc_complete = 0;
		return BCME_OK;
	} else {
		/* Timeout */
		DHD_ERROR(("%s: %s: cmd %d MLO IPC timeout\n",
			dhd_ifname(dhd_pub, ifidx), __FUNCTION__, ioc->cmd));
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

	default:
		DHD_ERROR(("%s: dhd%d: unknown mlc_state %u\n",
			__FUNCTION__, dhd_pub->unit, mlc_state));
		return BCME_ERROR;
	}

	return ret;
}

/* Check and process MLO Suspend state */
int
dhd_mlo_ipc_update_suspend_state(dhd_pub_t *dhd_pub)
{
	dhd_mlo_ipc_t *dhd_mlo_ipc = DHD_MLO_IPC(dhd_pub);
	int ret = BCME_OK;

	if (((dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_suspend_e) &&
		(dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_suspend_halt_e)) ||
		dhd_mlo_ipc->suspended) {
		return BCME_OK;
	}

	DHD_ERROR(("dhd%d: %s: ENTER suspended[%d] ioctl_in_progress[%s]\n",
		dhd_pub->unit, __FUNCTION__, dhd_mlo_ipc->suspended,
		dhd_prot_ioctl_in_progress(dhd_pub) ? "TRUE":"FALSE"));

	/* Wait for IOCTL in progress to complete */
	if ((dhd_mlo_ipc->mlc_state != mlo_ipc_mlc_suspend_halt_e) &&
		dhd_prot_ioctl_in_progress(dhd_pub)) {
		return BCME_OK;
	}

	dhd_mlo_ipc->suspended = TRUE;

	/* Send mlc_state event to dongle. */
	dhdpcie_mlo_h2d_notify(dhd_pub->bus, dhd_mlo_ipc->mlc_state);

	return ret;
} /* dhd_mlo_ipc_update_suspend_state() */

static int
dhd_mlo_ipc_set_mlc_state(dhd_pub_t *dhd_pub, dhd_mlo_ipc_t * dhd_mlo_ipc, uint8 mlc_state)
{
	int ret = BCME_OK;

	if (DHD_IS_MLO_MAP(dhd_pub))
		dhd_mlo_ipc->map->mlc_state = mlc_state;

	dhd_mlo_ipc->mlc_state = mlc_state;

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

	DHD_INFO(("%s: dhd%d: Sending BCMC XMIT req event len %d\n",
		__FUNCTION__, dhd_pub->unit, evt_msg_bytes));

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

	ret = mlo_ipc_evt_queue_enqueue(dest_ap_unit, evt_msg);

	if (ret	!= BCME_OK) {
		DHD_ERROR(("%s: BCMC XMIT EVT enqueue to ap[%d]failed ret %d\n",
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
#endif /* MLO_IPC */
