/*
 * +--------------------------------------------------------------------------+
 * WL_MLO_IPC: Inter Processor communication between MLO AP's
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
 * $Id: wl_mlo_ipc.c 833865 2023-12-06 11:05:47Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <pcicfg.h>
#include <pcie_core.h>
#include <mlo_ipc.h>
#include <mlc_lib.h>
#include <mlc_export.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wl_linux.h>
#include <wl_core.h>
#include <wl_mlo_ipc.h>
#include <d11_cfg.h>
#include <wldev_common.h>
#include <wlc.h>

#if defined(MLO_IPC)

#define WL_MLO_IPC_EVT_QUEUE_BUDGET	64
/* Currently, MLO_IPC wait event is used only for WLAN IOCTL/IOVAR forwarding in MLO mode
 * so using default IOCTL reponse timeout for MLO_IPC wait timeout
 */
#define WL_MLO_IPC_WAIT_TIMEOUT		IOCTL_RESP_TIMEOUT

// MAP object
typedef struct wl_mlo_ipc_map {
	mlo_ipc_msg_t	msg;	// Copy of State Transition request in progress
	uint8	mlc_state;	// Current FSA State
	uint8	next_state;	// FSA pending state
	uint8	gr_req_ap_unit;	// Graceful Recovery requesting AP unit
} wl_mlo_ipc_map_t;

typedef struct wl_mlo_ipc {
	pcie_ipc_mlo_ap_t	* pcie_ipc_mlo_ap;	// MLO attributes of AP
	struct task_struct      * kthread;
	wait_queue_head_t	wq_head;		// Linux wait_event queue
	wait_queue_head_t	mlo_ipc_wait;	// Wait queue for DOWN/BH events

	wl_mlo_ipc_map_t	* map;

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
} wl_mlo_ipc_t;

#define WL_MLO_IPC(wl_info)		((wl_mlo_ipc_t *)((wl_info)->wl_mlo_ipc))
#define WL_IS_MLO_AP(wl_info)		(WL_MLO_IPC(wl_info) != NULL)
#define WL_IS_MLO_MAP(wl_info)		(WL_MLO_IPC(wl_info)->map != NULL)

// Forward Declarations
static int	wl_mlo_ipc_kthread_fn(void * data);

/* MLO IPC MSG */
static void	wl_mlo_ipc_map_msg_enqueue(int ap_unit, uint8 msg_id, uint8 msg_prio,
                                            uint8 msg_unit, void * data);
static void	wl_mlo_ipc_map_msg_queue_process(wl_info_t * wl, wl_mlo_ipc_t * wl_mlo_ipc);
static void	wl_mlo_ipc_aap_state_change_cb(int ap_unit, uint8 mlc_state, void * data);
static void	wl_mlo_ipc_process_state_change(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc,
                                                 uint8 mlc_state);

/* MLO IPC EVT */
static void	wl_mlo_ipc_evt_wake_up_cb(void *data);
static void	wl_mlo_ipc_evt_queue_process(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc,
	bool bound);

/* MLO IOCTL intercept wait */
static int	wl_mlo_ipc_wait(wl_info_t * wl, wl_mlo_ipc_t *wl_mlo_ipc, uint *condition);
static int	wl_mlo_ipc_wake(wl_info_t * wl);

/*
 * ------------------------------------------------------------------------------------------------
 *  Initialize wl_mlo_ipc objects
 *  - Configure per AP PCIE IPC MLO attributes
 *  - Create a wait_event and kthread for wl_mlo_ipc
 * ------------------------------------------------------------------------------------------------
 */

static int // Alloc and Initialize MAP object
wl_mlo_ipc_map_init(wl_info_t * wl, wl_mlo_ipc_t * wl_mlo_ipc)
{
	wl_mlo_ipc_map_t * map;

	map = MALLOCZ(wl->osh, sizeof(wl_mlo_ipc_map_t));
	if (map == NULL) {
		WL_ERROR(("%s: malloc failed; bytes %u\n",
			__FUNCTION__, (uint)sizeof(wl_mlo_ipc_map_t)));
		return BCME_NORESOURCE;
	}

	map->mlc_state = mlo_ipc_mlc_attach_e;
	map->msg.id = ~0;
	map->msg.prio = ~0;
	map->msg.req_ap_bm = 0;
	map->gr_req_ap_unit = (uint8)AP_UNIT_INV;

	wl_mlo_ipc->map = map;

	return BCME_OK;
} // wl_mlo_ipc_map_init()

/* Get PCIE Gen and Lanes from configuration Registers. */
static void
wl_pcie_bus_get_pcie_gen_lane(wlc_pub_t *wlc_pub, int *pcie_gen, int *pcie_lane)
{
	uint32 v32;

	if (BUSTYPE(wlc_pub->sih->bustype) == PCI_BUS) {
		if (wlc_pub->sih->buscoretype == PCIE2_CORE_ID) {
			*pcie_gen = 2;

			v32 = OSL_PCI_READ_CONFIG(wlc_pub->osh, PCIECFGREG_LINKCAP, sizeof(uint32));
			*pcie_lane = BCM_GBF(v32, PCIECFGREG_LINKCAP_MW);

		} else if (wlc_pub->sih->buscoretype == PCIE_CORE_ID) {
			*pcie_gen = 1;

			v32 = OSL_PCI_READ_CONFIG(wlc_pub->osh, PCI_CFG_LINKCAP, sizeof(uint32));
			*pcie_lane = BCM_GBF(v32, PCI_CFG_LINKCAP_MW);

		} else {
			WL_ERROR(("wl%d: %s: Unsupported PCIE gen; buscoretype[%d]\n",
				wlc_pub->unit, __FUNCTION__, wlc_pub->sih->buscoretype));
			ASSERT(0);
		}
	} else {
		*pcie_gen = 0; /* signal non-PCIE using '0' version */
		*pcie_lane = 0;
		WL_ERROR(("wl%d: %s: not in PCIE mode, return fake nrs (GEN %d, lane %d)\n",
			wlc_pub->unit, __FUNCTION__, *pcie_gen, *pcie_lane));
	}

	WL_INFORM(("%s: wl%d: pcie_gen %d pcie_lane %d\n", __FUNCTION__, wlc_pub->unit,
		*pcie_gen, *pcie_lane));
} /* wl_pcie_bus_get_pcie_gen_lane() */

int
wl_mlo_ipc_init(wl_info_t *wl)
{
	mlo_ipc_sys_t   * mlo_ipc_sys;
	wl_mlo_ipc_t	* wl_mlo_ipc;
	wlc_pub_t       * wlc_pub;
	int	ap_unit;
	int	mlo_unit;
	int	ret;
	char	kthread_name[32] = {0};

	ret = BCME_OK;
	wlc_pub = wl->pub;
	ap_unit = wl->unit;
	mlo_ipc_sys = mlo_ipc_sys_gp;
	mlo_unit = PCIE_IPC_MLO_UNIT_INV;

	if ((mlo_ipc_sys != NULL) && (mlo_ipc_sys->pcie_ipc_mlo != NULL)) {
		mlo_unit = mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit];
	}

	WL_TRACE(("%s: ENTER ap_unit[%d] mlo_unit[%d]\n", __FUNCTION__, ap_unit, mlo_unit));

	if (mlo_unit == PCIE_IPC_MLO_UNIT_INV) {
		// No further actions for Non-MLO AP
		return BCME_OK;
	}

	ASSERT(WL_MLO_IPC(wl) == NULL);

	wl_mlo_ipc = MALLOCZ(wl->osh, sizeof(wl_mlo_ipc_t));

	if (wl_mlo_ipc == NULL) {
		WL_ERROR(("wl%d: %s: Malloc failed; bytes %u\n",
			ap_unit, __FUNCTION__, (uint)sizeof(wl_mlo_ipc_t)));
		return BCME_NOMEM;
	}

	wl->wl_mlo_ipc = (void *)wl_mlo_ipc;

	{ // Configure the RO pcie_ipc_mlo::ap[ap_unit]
		pcie_ipc_mlo_ap_t * pcie_ipc_mlo_ap;
		int pcie_gen = -1, pcie_lane = -1;
		uint16 pcie_unit;
		uint64 bar2_paddr;

		pcie_ipc_mlo_ap = &mlo_ipc_sys->pcie_ipc_mlo->ap[ap_unit];

		pcie_ipc_mlo_ap->mlo_unit       = (int8)mlo_unit;
		pcie_ipc_mlo_ap->ap_unit        = (uint8)ap_unit;
		pcie_ipc_mlo_ap->cpu_unit       = (uint8)ap_unit;

		pcie_unit = ap_unit;
		// Get PCIe Gen and Lanes
		wl_pcie_bus_get_pcie_gen_lane(wlc_pub, &pcie_gen, &pcie_lane);
		/* PCIe unit(b15..b8), gen(b7..b4), lane(b3..b0) */
		pcie_ipc_mlo_ap->bus_info	= (uint16)(((pcie_lane & 0xF) << 0) |
				((pcie_gen & 0xF) << 4) | ((pcie_unit & 0xFF) << 8));
		pcie_ipc_mlo_ap->chip_id        = (uint32) wlc_pub->sih->chip;

		/* TODO: Operating mode - NIC/FD */

		if (BUSTYPE(wlc_pub->sih->bustype) == SI_BUS) {
#if defined(BCMQT) && !defined(BCMQT_COMBO)
			bar2_paddr = DUAL_11BEDEV_OTHERSLICE_PHYSADDR;
#else
			bar2_paddr = SI_ENUM_BASE_PA(wlc_pub->sih);
#endif /* BCMQT && !BCMQT_COMBO */
		} else {

			bar2_paddr = wl_pcie_bar2_paddr(wl);

			OSL_PCI_WRITE_CONFIG(wl->osh, PCI_BAR2_WIN, sizeof(uint32),
				SI_ENUM_BASE_PA(wlc_pub->sih));
		}

		HADDR64_FROM_U64(pcie_ipc_mlo_ap->bar2_haddr64, bar2_paddr);

		if (HADDR64_IS_ZERO(pcie_ipc_mlo_ap->bar2_haddr64)) {
			WL_ERROR(("wl%d: %s: BAR2 unsupport\n", ap_unit, __FUNCTION__));
			ret = BCME_UNSUPPORTED;
			goto wl_mlo_ipc_init_fail;
		}

		wl_mlo_ipc->pcie_ipc_mlo_ap    = pcie_ipc_mlo_ap;
	}

	// Create a wait_event queue
	wl_mlo_ipc->wq_event_msg_queue = 0;
	wl_mlo_ipc->wq_event_state_change = 0;
	init_waitqueue_head(&wl_mlo_ipc->wq_head);
	init_waitqueue_head(&wl_mlo_ipc->mlo_ipc_wait);

	/* TODO: WL runs on a seperate thread on router platfom.
	 * Same thread can be used here
	 */
	// Create kthread for wl_mlo_ipc
	sprintf(kthread_name, "wl%d_mlo_kthrd", ap_unit);
	wl_mlo_ipc->kthread = kthread_create(wl_mlo_ipc_kthread_fn, wl, kthread_name);
	if (IS_ERR(wl_mlo_ipc->kthread)) {
		WL_ERROR(("wl%d: %s: %s create failed\n", ap_unit, __FUNCTION__, kthread_name));
		ret = BCME_ERROR;
		goto wl_mlo_ipc_init_fail;
	}

	// kthread_bind(wl_mlo_ipc->kthread, cpu_unit);
	wake_up_process(wl_mlo_ipc->kthread);

	wl_mlo_ipc->ap_unit = ap_unit;
	wl_mlo_ipc->map = NULL;
	wl_mlo_ipc->req_mlc_state = ~0;
	wl_mlo_ipc->mlc_state = mlo_ipc_mlc_reset_e;

	// Register MLO AP context with System wide MLO_IPC
	if (mlo_unit == PCIE_IPC_MLO_UNIT_MAP) {	// Is MAP?
		ret = wl_mlo_ipc_map_init(wl, wl_mlo_ipc);
		if (ret != BCME_OK) {
			goto wl_mlo_ipc_init_fail;
		}
		mlo_ipc_map_register(ap_unit, wl_mlo_ipc_map_msg_enqueue,
			wl_mlo_ipc_evt_wake_up_cb, (void *)wl, PCIE_IPC_MLO_OPMODE_NIC);
	} else {
		mlo_ipc_aap_register(ap_unit, wl_mlo_ipc_aap_state_change_cb,
			wl_mlo_ipc_evt_wake_up_cb, (void *)wl, PCIE_IPC_MLO_OPMODE_NIC);
	}

	WL_ERROR(("wl%d: %s: SUCCESS; mlo_unit[%d] is_mlo[%d] is_map[%d]\n",
		ap_unit, __FUNCTION__, mlo_unit, WL_IS_MLO_AP(wl), WL_IS_MLO_MAP(wl)));

	return BCME_OK;

wl_mlo_ipc_init_fail:

	WL_ERROR(("wl%d: %s: Failure\n", ap_unit,  __FUNCTION__));
	wl_mlo_ipc_deinit(wl);
	return ret;
} // wl_mlo_ipc_init()

void // Destruct WL_MLO_IPC objects
wl_mlo_ipc_deinit(wl_info_t * wl)
{
	wl_mlo_ipc_t * wl_mlo_ipc;

	WL_TRACE(("%s: ENTER ap_unit[%d]\n", __FUNCTION__, wl->unit));

	if (WL_IS_MLO_AP(wl) == FALSE) {
		return;
	}

	wl_mlo_ipc = WL_MLO_IPC(wl);

	if (wl_mlo_ipc->wq_event_msg_queue) {
		ASSERT(WL_IS_MLO_MAP(wl));
		wl_mlo_ipc_map_msg_queue_process(wl, wl_mlo_ipc);
	}

	if (wl_mlo_ipc->wq_event_state_change) {
		ASSERT(WL_IS_MLO_MAP(wl) == FALSE);
		wl_mlo_ipc_process_state_change(wl, wl_mlo_ipc, wl_mlo_ipc->req_mlc_state);
		// Reset Wait Event
		wl_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
		wl_mlo_ipc->wq_event_state_change = 0;
	}

	if (wl_mlo_ipc->wq_event_evt_queue) {
		wl_mlo_ipc_evt_queue_process(wl, wl_mlo_ipc, FALSE);
	}

	if (wl_mlo_ipc->kthread) {
		kthread_stop(wl_mlo_ipc->kthread);
	}

#ifdef BCM_ROUTER
	if (BUSTYPE(wl->pub->sih->bustype) == PCI_BUS) {
		wl_pcie_bus_bridge_deinit(wl);
	}
#endif /* BCM_ROUTER */

	if (WL_IS_MLO_MAP(wl)) {
		mlo_ipc_map_deregister(wl->unit);

		MFREE(wl->osh, wl_mlo_ipc->map, sizeof(wl_mlo_ipc_map_t));
		wl_mlo_ipc->map = NULL;
	} else {
		mlo_ipc_aap_deregister(wl->unit);
	}

	MFREE(wl->osh, wl_mlo_ipc, sizeof(wl_mlo_ipc_t));
	wl->wl_mlo_ipc = NULL;

} // wl_mlo_ipc_deinit()

static int // wl_mlo_ipc kthread handler
wl_mlo_ipc_kthread_fn(void * data)
{
	wl_info_t       *wl;
	wl_mlo_ipc_t	* wl_mlo_ipc;

	wl = (wl_info_t *) data;
	wl_mlo_ipc = WL_MLO_IPC(wl);

	while (1) {

		// MLO AP wait event queue
		wait_event_interruptible(wl_mlo_ipc->wq_head,
				(wl_mlo_ipc->wq_event_msg_queue ||
				wl_mlo_ipc->wq_event_state_change ||
				wl_mlo_ipc->wq_event_evt_queue ||
				kthread_should_stop()));

		if (kthread_should_stop()) {
			WL_ERROR(("kthread_should_stop detected on ap_unit[%d]\n",
				wl_mlo_ipc->ap_unit));
			break;
		}

		WL_INFORM(("%s: ap_unit[%d] grabbing lock\n", __FUNCTION__, wl_mlo_ipc->ap_unit));

		WL_LOCK(wl); /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

		// New messages are available in msg queue
		if (wl_mlo_ipc->wq_event_msg_queue) {

			ASSERT(WL_IS_MLO_MAP(wl));
			WL_INFORM(("%s: ap_unit[%d] received MSG_QUEUE event\n",
				__FUNCTION__, wl_mlo_ipc->ap_unit));

			wl_mlo_ipc_map_msg_queue_process(wl, wl_mlo_ipc);
		}

		/* State Transition request from MAP => AAP */
		if (wl_mlo_ipc->wq_event_state_change) {
			uint req_mlc_state = wl_mlo_ipc->req_mlc_state;

			ASSERT(WL_IS_MLO_MAP(wl) == FALSE);

			WL_INFORM(("%s: ap_unit[%d] received STATE_CHANGE event\n",
				__FUNCTION__, wl_mlo_ipc->ap_unit));

			// Reset Wait Event
			wl_mlo_ipc->req_mlc_state = mlo_ipc_mlc_reset_e;
			wl_mlo_ipc->wq_event_state_change = 0;

			wl_mlo_ipc_process_state_change(wl, wl_mlo_ipc,
				req_mlc_state);
		}

		if (wl_mlo_ipc->wq_event_evt_queue) {
			WL_INFORM(("%s: ap_unit[%d] received MSG_QUEUE event\n",
				__FUNCTION__, wl_mlo_ipc->ap_unit));

			wl_mlo_ipc_evt_queue_process(wl, wl_mlo_ipc, TRUE);
		}

		WL_UNLOCK(wl); /* -------------------------------------------------------- */
	}

	return 0;
} // wl_mlo_ipc_kthread_fn()

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
wl_mlo_ipc_map_msg_enqueue(int ap_unit, uint8 msg_id, uint8 msg_prio,
                            uint8 msg_unit, void * data)
{
	wl_info_t       * wl;
	wl_mlo_ipc_t	* wl_mlo_ipc;
	mlo_ipc_msg_t	* msg;
	mlo_ipc_msg_queue_t * msg_queue;

	WL_ERROR(("wl%d: %s: id[%d] prio[%d] unit[%d] data[%p]\n",
		ap_unit, __FUNCTION__, msg_id, msg_prio, msg_unit, data));

	wl = (wl_info_t*)data;
	wl_mlo_ipc = WL_MLO_IPC(wl);
	msg_queue = MLO_IPC_MSG_QUEUE();

	ASSERT(wl_mlo_ipc != NULL);
	ASSERT(msg_queue != NULL);
	ASSERT(ap_unit == wl_mlo_ipc->ap_unit);
	ASSERT(WL_IS_MLO_MAP(wl));

	MLO_IPC_MSG_QUEUE_LOCK(); // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	msg = MLO_IPC_MSG(msg_id);
	ASSERT(msg != NULL);

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
	wl_mlo_ipc->wq_event_msg_queue = ~0;
	wake_up_interruptible(&wl_mlo_ipc->wq_head);

} // wl_mlo_ipc_map_msg_enqueue()

/** Broadcast new state to all AAP's */
static INLINE void
wl_mlo_ipc_map_state_broadcast(wl_info_t *wl, uint8 mlc_state)
{
	int	iter;
	uint8	mlo_ap_bm = mlo_ipc_get_ap_bm();
	uint8	ap_unit_map = mlo_ipc_get_ap_unit_map();

	// Broadcast new state to all AAP's
	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		mlo_ipc_aap_state_change_cb_t	state_change_cb;
		void *aap_data;

		WL_INFORM(("%s: iter[%d]: ap_unit_map[%d] mlo_ap_bm[%x]\n", __FUNCTION__,
			iter, ap_unit_map, mlo_ap_bm));
		if ((iter == ap_unit_map) || ((mlo_ap_bm & (1 << iter)) == 0)) {
			// Don't send state transition request to MAP and non MLO AP
			continue;
		}

		state_change_cb = mlo_ipc_get_state_change_cb(iter);
		aap_data = mlo_ipc_get_aap_data(iter);
		state_change_cb(iter, mlc_state, aap_data);
	}
}

/** Parse the messge from MLO AP and broadcast state transition to all MLO AP's */
static INLINE void
wl_mlo_ipc_map_msg_process(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc, mlo_ipc_msg_t * msg)
{
	wl_mlo_ipc_map_t * map;
	uint8	new_mlc_state;
	uint8	mlo_ap_bm = mlo_ipc_get_ap_bm();

	map = wl_mlo_ipc->map;

	WL_ERROR(("wl%d: %s: Process MAP msg::id[%d] mlc_state[%d]\n",
		wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id, map->mlc_state));

	switch (msg->id) {
		case MLO_IPC_MSG_ID_ATTACH:
			new_mlc_state = mlo_ipc_mlc_bind_e;
			break;

		case MLO_IPC_MSG_ID_BIND:
			if (map->mlc_state != mlo_ipc_mlc_bind_e) {
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] mlc_state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__,
					msg->id, map->mlc_state));
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] mlc_state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__,
					msg->id, map->mlc_state));
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] mlc_state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__,
					msg->id, map->mlc_state));
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] mlc_state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__,
					msg->id, map->mlc_state));
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
				WL_ERROR(("wl%d: %s: Ignoring msg::id[%d] state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id,
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id,
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id,
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id,
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
				WL_ERROR(("wl%d: %s: Unexpected msg::id[%d] state[%d]\n",
					wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id,
					map->mlc_state));
				ASSERT(map->mlc_state == mlo_ipc_mlc_down_bh_e);
				return;
			}
			new_mlc_state = mlo_ipc_mlc_down_bh_complete_e;
			map->next_state = mlo_ipc_mlc_ready_e;
			break;

		default:
			WL_ERROR(("wl%d: %s: Invalid msg::id[%d]\n",
				wl_mlo_ipc->ap_unit, __FUNCTION__, msg->id));
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

	WL_ERROR(("wl%d: %s: Broadcasting state[%d] transition to all MLO AP's\n",
		wl_mlo_ipc->ap_unit, __FUNCTION__, new_mlc_state));

	wl_mlo_ipc_map_state_broadcast(wl, new_mlc_state);

map_state_transition:
	// Process state change in MAP
	map->mlc_state = new_mlc_state;
	wl_mlo_ipc_process_state_change(wl, wl_mlo_ipc, new_mlc_state);

	// Reset request in progess
	msg->id = ~0;

	return;
} // wl_mlo_ipc_map_msg_process()

/** Check and process all messages from MAP msg queue. */
static void
wl_mlo_ipc_map_msg_queue_process(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc)
{
	mlo_ipc_msg_t * msg;
	mlo_ipc_msg_queue_t * msg_queue;
	wl_mlo_ipc_map_t * map;
	dll_t * item;

	msg_queue = MLO_IPC_MSG_QUEUE();
	map = wl_mlo_ipc->map;

msg_queue_process_continue:

	if (dll_empty(&msg_queue->active_list)) {
		// No more messages to process
		// Reset msg_queue pending work
		wl_mlo_ipc->wq_event_msg_queue = 0;
		return;
	}

	MLO_IPC_MSG_QUEUE_LOCK(); // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	item = dll_head_p(&msg_queue->active_list);
	msg = container_of(item, mlo_ipc_msg_t, node);
	// Remove the msg node from active list
	dll_delete(&msg->node);
	msg_queue->dequeue_cnt++;

	// Copy the request to WL context
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

	WL_ERROR(("wl%d: %s: id[%d] prio[%d] req_ap_bm[%d]\n",
		wl_mlo_ipc->ap_unit, __FUNCTION__,
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

	wl_mlo_ipc_map_msg_process(wl, wl_mlo_ipc, &map->msg);

	goto msg_queue_process_continue;

} // wl_mlo_ipc_map_msg_queue_process()

/** Callback registered with MAP invoked to inform state transition */
static void
wl_mlo_ipc_aap_state_change_cb(int ap_unit, uint8 mlc_state, void * data)
{
	wl_info_t       *wl;
	wl_mlo_ipc_t	* wl_mlo_ipc;

	wl = (wl_info_t *) data;
	wl_mlo_ipc = WL_MLO_IPC(wl);

	WL_ERROR(("wl%d: %s: data[%p] mlc_state[%u]\n",
		wl_mlo_ipc->ap_unit, __FUNCTION__, data, mlc_state));

	WL_LOCK(wl); /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	wl_mlo_ipc->req_mlc_state = mlc_state;

	wl_mlo_ipc->wq_event_state_change = ~0;
	WL_INFORM(("%s: ap_unit[%d] data[%p] mlc_state[%u] wq_event_state_change[%u]\n",
		__FUNCTION__, wl_mlo_ipc->ap_unit, data, mlc_state,
		wl_mlo_ipc->wq_event_state_change));
	WL_UNLOCK(wl); /* -------------------------------------------------------- */
	wake_up_interruptible(&wl_mlo_ipc->wq_head);

} // wl_mlo_ipc_aap_state_change_cb()

static int
wl_mlo_ipc_set_mlc_state(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc, uint8 mlc_state)
{
	int ret = BCME_OK;

	if (WL_IS_MLO_MAP(wl))
		wl_mlo_ipc->map->mlc_state = mlc_state;

	wl_mlo_ipc->mlc_state = mlc_state;

	return ret;
} /* wl_mlo_ipc_set_mlc_state() */

/* Process new state transition request for MLO AP */
static void
wl_mlo_ipc_process_state_change(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc, uint8 mlc_state)
{
	int ap_unit = wl_mlo_ipc->ap_unit;
	wlc_pub_t *wlc_pub = wl->pub;

	BCM_REFERENCE(ap_unit);
	BCM_REFERENCE(__mlo_ipc_mlc_state_str);
	WL_ERROR(("wl%d: %s: %s begin\n",
		ap_unit, __FUNCTION__, __mlo_ipc_mlc_state_str[mlc_state]));

	switch (mlc_state) {
		case mlo_ipc_mlc_bind_e:
			wl_mlo_ipc->mlc_state = mlc_state;
			if (mlc_bind_mlo_ipc(wlc_pub_2_mlc_dev(wl->pub)) != BCME_OK) {
				WL_ERROR(("MLC bind mlo ipc failed\n"));
				ASSERT(0);
			}
			wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_bind_e);
			break;

		case mlo_ipc_mlc_link_e:
			wl_mlo_ipc->mlc_state = mlc_state;
			/* Start MLC_LINK state with the mlc_dev */
			if (mlc_link_mlo_ipc(wlc_pub_2_mlc_dev(wlc_pub)) != BCME_OK) {
				WL_ERROR(("MLC link mlo ipc failed\n"));
				ASSERT(0);
			}
			wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_link_e);
			break;

		case mlo_ipc_mlc_bridge_e:
			wl_mlo_ipc->mlc_state = mlc_state;
			/* Host bridge setup */
#ifdef BCM_ROUTER
			if (BUSTYPE(wlc_pub->sih->bustype) == PCI_BUS) {
				if (wl_pcie_bus_bridge_init(wl) != BCME_OK) {
					WL_ERROR(("PCIE RC Bridge not supported\n"));
					ASSERT(0);
					break;
				}
			}
#endif /* BCM_ROUTER */
			wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_bridge_e);
			break;

		case mlo_ipc_mlc_sync_e:
			wl_mlo_ipc->mlc_state = mlc_state;
#if defined(WL_SMAC) && defined(WL_MLO)
			/* Inform SYNC to MAP if icc link is ready
			 * else defer sync event to wl up
			 */
			if (!ICC_ENAB(wlc_pub) ||
				wlc_icc_is_link_ready(wlc_pub->wlc))
#endif /* WL_SMAC && WL_MLO */
			{
				wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_sync_e);
			}
			mlc_sync_mlo_ipc(wlc_pub_2_mlc_dev(wlc_pub));
			break;

		case mlo_ipc_mlc_ready_e:
			wl_mlo_ipc->mlc_state = mlc_state;
#ifdef WL_MLO
			wlc_mlo_ready_all(wlc_pub->wlc);
#endif /* WL_MLO */
			mlc_ready_mlo_ipc(wlc_pub_2_mlc_dev(wlc_pub));
			break;

		case mlo_ipc_mlc_suspend_e:
		case mlo_ipc_mlc_suspend_halt_e:
			WL_ERROR(("wl%d: %s: MLO_SUSPEND request\n", ap_unit, __FUNCTION__));
			wl_mlo_ipc->mlc_state = mlc_state;
			ASSERT(!wl_mlo_ipc->suspended);
			wl_mlo_ipc->suspended = TRUE;
			if (mlc_state == mlo_ipc_mlc_suspend_e) {
				wlc_mlo_ipc_process_state_change(wl->wlc,
					mlc_state_suspend_e);
			} else {
				wlc_mlo_ipc_process_state_change(wl->wlc,
					mlc_state_suspend_halt_e);
			}
			break;

		case mlo_ipc_mlc_down_e:
			WL_ERROR(("wl%d: %s: MLO_IPC_DOWN request\n",
				ap_unit, __FUNCTION__));
			wl_mlo_ipc->mlc_state = mlc_state;
			wlc_mlo_ipc_down((void *)wl->wlc, wl_mlo_ipc->map->gr_req_ap_unit);
			break;

#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_e:
			WL_ERROR(("wl%d: %s: MLO_BH_E request\n",
				ap_unit, __FUNCTION__));
			wl_mlo_ipc->mlc_state = mlc_state;
			wlc_mlo_ipc_bh(wl->wlc, wl_mlo_ipc->map->gr_req_ap_unit);
			break;

#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_e:
			WL_ERROR(("wl%d: %s: MLO_IPC_DOWN_BH request\n",
				ap_unit, __FUNCTION__));
			wl_mlo_ipc->mlc_state = mlc_state;
			wlc_mlo_ipc_down_bh((void *)wl->wlc, wl_mlo_ipc->map->gr_req_ap_unit);
			break;

		case mlo_ipc_mlc_down_complete_e:
#if defined(BCMMLO_GR)
		case mlo_ipc_mlc_bh_complete_e:
#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_complete_e:
			wl_mlo_ipc->mlc_state = mlc_state;
			wl_mlo_ipc->suspended = FALSE;

			WL_ERROR(("wl%d: %s: continues with non-MLO %s\n",
				ap_unit, __FUNCTION__,
				(mlc_state == mlo_ipc_mlc_down_complete_e) ? "down" : "bh"));

			/* MLO down is complete so revert the state to mlc_ready */
			wl_mlo_ipc_set_mlc_state(wl, wl_mlo_ipc, mlo_ipc_mlc_ready_e);

			if (wl_mlo_ipc->wl_down_pending) {
				ASSERT(wl_mlo_ipc->ioc_pending == FALSE);
				/* Resume and continue iovar processig for non-MLO (wl down) */
				wl_mlo_ipc->wl_down_pending = FALSE;
				wl_mlo_ipc->ioc_complete = 1;
				wl_mlo_ipc_wake(wl);
			} else {
#if defined(BCMMLO_GR)
				if (mlc_state == mlo_ipc_mlc_bh_complete_e) {
					wlc_do_init(wl->wlc);
				} else
#endif
				if (mlc_state == mlo_ipc_mlc_down_bh_complete_e) {
					wlc_set_mlo_down(wl->wlc);
					wlc_do_init(wl->wlc);
				} else {
					wlc_do_down(wl->wlc);
				}

				/* Resume processing of iovar requested when
				 * MLO_IPC (DOWN/BH) in progress
				 */
				if (wl_mlo_ipc->ioc_pending) {
					wl_mlo_ipc->ioc_pending = FALSE;
					wl_mlo_ipc->ioc_complete = 1;
					wl_mlo_ipc_wake(wl);
				}
			}

			break;

		case mlo_ipc_mlc_halt_e:

			/* Send mlo_ipc_mlc_halt_e state event to dongle except to the AP
			 * in which Firmware Trap detected.
			 */
			if (wl_mlo_ipc->mlc_state != mlo_ipc_mlc_trap_e) {
				WL_ERROR(("wl%d: %s: MLO TRAP notify\n", ap_unit, __FUNCTION__));
				wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_halt_e);
			}

			wl_mlo_ipc->mlc_state = mlc_state;
			break;

		default:
			WL_ERROR(("wl%d: %s: Invalid state[%d]\n",
				ap_unit, __FUNCTION__, mlc_state));
			ASSERT(0);
			break;
	}

} // wl_mlo_ipc_process_state_change()

/* Mapping WLAN MLC_STATE to HND MLO_IPC state */
static uint8
wl_mlo_ipc_mlc_state_to_ipc_state(uint8 mlc_state)
{
	uint8 mlo_ipc_state = mlo_ipc_mlc_reset_e;

	switch (mlc_state) {
	case mlc_state_attach_e:
		mlo_ipc_state = mlo_ipc_mlc_attach_e;
		break;
	case mlc_state_bind_e:
		mlo_ipc_state = mlo_ipc_mlc_bind_e;
		break;
	case mlc_state_link_e:
		mlo_ipc_state = mlo_ipc_mlc_link_e;
		break;
	case mlc_state_bridge_e:
		mlo_ipc_state = mlo_ipc_mlc_bridge_e;
		break;
	case mlc_state_sync_e:
		mlo_ipc_state = mlo_ipc_mlc_sync_e;
		break;
	case mlc_state_ready_e:
		mlo_ipc_state = mlo_ipc_mlc_ready_e;
		break;
	case mlc_state_halt_e:
		mlo_ipc_state = mlo_ipc_mlc_halt_e;
		break;
	case mlc_state_suspend_e:
		mlo_ipc_state = mlo_ipc_mlc_suspend_e;
		break;
	case mlc_state_suspend_halt_e:
		mlo_ipc_state = mlo_ipc_mlc_suspend_halt_e;
		break;
	case mlc_state_down_e:
		mlo_ipc_state = mlo_ipc_mlc_down_e;
		break;
	case mlc_state_active_e:
		mlo_ipc_state = mlo_ipc_mlc_active_e;
		break;
#if defined(BCMMLO_GR)
	case mlc_state_bh_req_e:
		mlo_ipc_state = mlo_ipc_mlc_bh_req_e;
		break;
	case mlc_state_bh_halt_req_e:
		mlo_ipc_state = mlo_ipc_mlc_bh_halt_req_e;
		break;
	case mlc_state_bh_e:
		mlo_ipc_state = mlo_ipc_mlc_bh_e;
		break;
#endif /* BCMMLO_GR */
	case mlc_state_down_bh_e:
		mlo_ipc_state = mlo_ipc_mlc_down_bh_e;
		break;

	default:
		WL_ERROR(("Invalid mlc_state %d\n", mlc_state));
		break;
	}

	return mlo_ipc_state;
}

/**
 * Process the mlo_ipc event received from wl layer, which results in a message being sent to the
 * 'other' MLO APSTA.
 */
int
wl_mlo_ipc_process_event(wl_info_t *wl, uint8 mlo_ipc_state)
{
	wl_mlo_ipc_t	* wl_mlo_ipc;
	uint8	msg_id;
	uint8	msg_prio;
	mlo_ipc_map_msg_enqueue_fn_t msg_enqueue_fn;
	void	*map_data;
	uint8	ap_unit_map;

	if (!WL_IS_MLO_AP(wl)) {
		return BCME_ERROR;
	}

	wl_mlo_ipc = WL_MLO_IPC(wl);

	WL_ERROR(("wl%d: %s: mlo_ipc_state[%d]\n",
		wl_mlo_ipc->ap_unit, __FUNCTION__, mlo_ipc_state));

	switch (mlo_ipc_state) {
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
			WL_ERROR(("wl%d reporting a dongle TRAP\n", wl_mlo_ipc->ap_unit));
			/* If a trap event happened before, don't bother to halt other
			 * interfaces again.
			 */
			if (wl_mlo_ipc->mlc_state == mlo_ipc_mlc_halt_e) {
				return BCME_OK;
			}
			msg_id = MLO_IPC_MSG_ID_TRAP;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			wl_mlo_ipc->mlc_state = mlo_ipc_mlc_trap_e;
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
			/* Hijacking the dongle event process routine for BH with PSMWD */
			msg_id = MLO_IPC_MSG_ID_BH_HALT_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_bh_e:
			msg_id = MLO_IPC_MSG_ID_BH;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;
#endif /* BCMMLO_GR */
		case mlo_ipc_mlc_down_bh_req_e:
			msg_id = MLO_IPC_MSG_ID_DOWN_BH_REQ;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_down_bh_e:
			msg_id = MLO_IPC_MSG_ID_DOWN_BH;
			msg_prio = MLO_IPC_MSG_PRIO_HIGH;
			break;

		case mlo_ipc_mlc_active_e:
			wl_mlo_ipc_set_mlc_state(wl, wl_mlo_ipc, mlo_ipc_mlc_active_e);
			WL_ERROR(("wl%d: is MLO_ACTIVE\n", wl_mlo_ipc->ap_unit));
			/* Nothing to do. All links will inform to the host (WL/DHD) */
			return BCME_OK;

		default:
			WL_ERROR(("wl%d %s: Unexpected mlo_ipc_state[%d] from wlc\n",
				wl_mlo_ipc->ap_unit, __FUNCTION__, mlo_ipc_state));
			return BCME_ERROR;
	}

	msg_enqueue_fn = mlo_ipc_get_map_msg_enqueue_fn();
	map_data = mlo_ipc_get_map_data();
	ap_unit_map = mlo_ipc_get_ap_unit_map();

	msg_enqueue_fn(ap_unit_map, msg_id,
		msg_prio, wl_mlo_ipc->ap_unit, map_data);

	return BCME_OK;
} // wl_mlo_ipc_process_event()

void
wl_mlo_ipc_mlc_state_complete(wl_info_t *wl,  uint8 mlc_state)
{
	uint8 mlo_ipc_state;

	mlo_ipc_state = wl_mlo_ipc_mlc_state_to_ipc_state(mlc_state);

	/* For MLO BH, schedule a wlioctl request */
	if (mlo_ipc_state == mlo_ipc_mlc_bh_req_e) {
		wl_mlo_sched_bh(wl, FALSE);
	} else if (mlo_ipc_state == mlo_ipc_mlc_bh_halt_req_e) {
		wl_mlo_sched_bh(wl, TRUE);
	} else {
		wl_mlo_ipc_process_event(wl, mlo_ipc_state);
	}
}

int
wl_mlo_ipc_start(wl_info_t *wl)
{
	wl_mlo_ipc_t	* wl_mlo_ipc;
	uint8	mlo_ap_bm; /* registered APs */
	uint8	sys_ap_bm; /* nvram configured APs */

	if (!WL_IS_MLO_AP(wl)) {
		/* Non-MLO AP */
		return BCME_OK;
	}

	wl_mlo_ipc = WL_MLO_IPC(wl);
	wl_mlo_ipc->mlc_state = mlo_ipc_mlc_attach_e;

	sys_ap_bm = mlo_ipc_get_sys_ap_bm();
	ASSERT(sys_ap_bm);

	mlo_ap_bm = mlo_ipc_set_ap_bm(wl_mlo_ipc->ap_unit);

	WL_TRACE(("%s: mlo_ap_bm %x expecting %x\n", __FUNCTION__,
		mlo_ap_bm, sys_ap_bm));

	if (sys_ap_bm == mlo_ap_bm) {
		WL_ERROR(("%s: All MLO APs registered.\n", __FUNCTION__));
		return wl_mlo_ipc_process_event(wl, wl_mlo_ipc->mlc_state);
	}

	return BCME_OK;
} // wl_mlo_ipc_start()

/**
 * ------------------------------------------------------------------------------------------------
 * SECTION: MLO_IPC Event processing
 * ------------------------------------------------------------------------------------------------
 */

/* Wakeup callback registered with MLO IPC Event invoked by producer link after
 * queuing a new event message.
 */
static void
wl_mlo_ipc_evt_wake_up_cb(void *data)
{
	wl_info_t       *wl = (wl_info_t*)data;
	wl_mlo_ipc_t	*wl_mlo_ipc = WL_MLO_IPC(wl);

	ASSERT(wl_mlo_ipc != NULL);

	wl_mlo_ipc->wq_event_evt_queue = ~0;
	wake_up_interruptible(&wl_mlo_ipc->wq_head);

	return;
} // wl_mlo_ipc_evt_wake_up_cb()

/* Process all events messages from event queue */
static void
wl_mlo_ipc_evt_queue_process(wl_info_t *wl, wl_mlo_ipc_t * wl_mlo_ipc, bool bound)
{
	mlo_ipc_evt_msg_t * evt_msg;
	int	evt_msgs_processed = 0;

	WL_TRACE(("%s: wl%d: ENTER\n", __FUNCTION__, wl->unit));

evt_queue_process_continue:

	/* Get the Event message from the queue */
	evt_msg = mlo_ipc_evt_queue_dequeue(wl->unit);
	if (evt_msg == NULL) {
		// No more messages to process
		wl_mlo_ipc->wq_event_evt_queue = 0;
		return;
	}

	WL_INFORM(("%s: wl%d: MLO IPC EVT [%d] from ap [%d]\n",
		__FUNCTION__, wl->unit, evt_msg->evt_id, evt_msg->ap_unit));

	switch (evt_msg->evt_id) {
		case MLO_IPC_EVT_ID_BCMC_XMIT:
		{
			mlo_ipc_bcmc_xmit_msg_t *bcmc_xmit_msg;
			bcmc_xmit_msg = (mlo_ipc_bcmc_xmit_msg_t *)evt_msg->data;

			wl_mlo_aap_handle_bcmc(wl, (void *)bcmc_xmit_msg);
			break;
		}
		default:
			WL_ERROR(("[wl%d] MLO IPC Event[%d] not yet implemented\n",
				wl->pub->unit, evt_msg->evt_id));
			ASSERT(0);
			break;
	}

	/* Release event message */
	mlo_ipc_evt_msg_free(evt_msg);

	evt_msgs_processed++;
	if (bound && (evt_msgs_processed > WL_MLO_IPC_EVT_QUEUE_BUDGET)) {
		return;
	}

	goto evt_queue_process_continue;

} // wl_mlo_ipc_evt_queue_process()

/* Intercept WLAN IOCTL/IOVARs like WLC_DOWN/WLC_INIT and forward them to MLO_IPC */
int
wl_mlo_ipc_wl_ioctl_intercept(wl_info_t *wl, wl_if_t *wlif, wl_ioctl_t *ioc, void * buf)
{
	int ret = BCME_OK;
	wl_mlo_ipc_t *wl_mlo_ipc = WL_MLO_IPC(wl);

	WL_TRACE(("wl%d: %s: cmd=\"%s\" ioc cmd<%u> len<%u>\n", wl->pub->unit,
		__FUNCTION__, (char *)buf, ioc->cmd, ioc->len));

	if (wl_mlo_ipc->suspended) {
		WL_ERROR(("wl%d: %s: MLO_SUSPEND in progress cmd=\"%s\" ioc cmd<%u> len<%u>\n",
			wl->pub->unit, __FUNCTION__, (char *)buf, ioc->cmd, ioc->len));

		if (wl_mlo_ipc->ioc_complete || wl_mlo_ipc->wl_down_pending) {
			WL_ERROR(("wl%d: %s: ioc_complete[%d] == 1 || wl_down_pending[%d] == 1\n",
				wl->pub->unit, __FUNCTION__,
				wl_mlo_ipc->ioc_complete, wl_mlo_ipc->wl_down_pending));

			// ASSERT(0);
			return BCME_BUSY;
		}

		wl_mlo_ipc->ioc_pending = TRUE;

		/* The peer AP has started the GR Down/BH procedure */
		if (wl_mlo_ipc_wait(wl, wl_mlo_ipc, &wl_mlo_ipc->ioc_complete) > 0) {
			wl_mlo_ipc->ioc_complete = 0;
			WL_ERROR(("wl%d: %s: Resume the ioctl\n", wl->pub->unit, __FUNCTION__));
			return BCME_OK;
		} else {
			/* Timeout */
			WL_ERROR(("wl%d: %s: cmd %d MLO IPC timeout\n",
				wl->pub->unit, __FUNCTION__, ioc->cmd));
			return BCME_NOTREADY;
		}
	}

	if ((ioc->cmd != WLC_DOWN) && (ioc->cmd != WLC_REBOOT) &&
		(ioc->cmd != WLC_INIT) && (ioc->cmd != WLC_INIT_HALT) &&
		(ioc->cmd != WLC_MLO_DOWN)) {
		return ret;
	}

	/* Forward WLC_DOWN/WLC_REBOOT request to MAP to bringdown MLO */
	if ((ioc->cmd == WLC_DOWN) || (ioc->cmd == WLC_REBOOT)) {
		if (wl_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			WL_ERROR(("wl%d: %s: mlc state is not active. mlc_state %d\n",
				wl->pub->unit, __FUNCTION__, wl_mlo_ipc->mlc_state));
			goto done;
		}

		if (wl->wlc->psm_watchdog_debug) {
			WL_ERROR(("wl%d: %s: skip mlc down when psm watchdog fires!!\n",
				wl->pub->unit, __FUNCTION__));
			goto done;
		}

		WL_ERROR(("wl%d: %s:  mlc_state %d\n", wl->pub->unit,  __FUNCTION__,
			wl_mlo_ipc->mlc_state));
		wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_down_req_e);
#if defined(BCMMLO_GR)
	} else if (ioc->cmd == WLC_INIT || ioc->cmd == WLC_INIT_HALT) {
		if (wl_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			WL_ERROR(("wl%d: %s: mlc state is not active. mlc_state %d\n",
				wl->pub->unit, __FUNCTION__, wl_mlo_ipc->mlc_state));
			goto done;
		}

		WL_ERROR(("wl%d: %s: mlc_state %d\n", wl->pub->unit,
			__FUNCTION__, wl_mlo_ipc->mlc_state));
		if (ioc->cmd == WLC_INIT) {
			wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_bh_req_e);
		} else {
			/* WLC_INIT_HALT */
			wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_bh_halt_req_e);
		}
#endif /* BCMMLO_GR */
	} else if (ioc->cmd == WLC_MLO_DOWN) {
		if (wl_mlo_ipc->mlc_state != mlo_ipc_mlc_active_e) {
			WL_ERROR(("wl%d: %s: mlc state is not active. mlc_state %d\n",
				wl->pub->unit, __FUNCTION__, wl_mlo_ipc->mlc_state));
			goto done;
		}

		WL_ERROR(("wl%d: %s: mlc_state %d\n", wl->pub->unit,
			__FUNCTION__, wl_mlo_ipc->mlc_state));
		wl_mlo_ipc_process_event(wl, mlo_ipc_mlc_down_bh_req_e);
	} else {
		goto done;
	}

	WL_ERROR(("wl%d: WAIT for MLO_IPC_DOWN to complete\n", wl->pub->unit));
	/* Wait for MLO_IPC work (DOWN/BH) to be complete */
	ASSERT(wl_mlo_ipc->ioc_pending == FALSE);
	wl_mlo_ipc->wl_down_pending = TRUE;
	wl_mlo_ipc->ioc_complete = 0;
	if (wl_mlo_ipc_wait(wl, wl_mlo_ipc, &wl_mlo_ipc->ioc_complete) > 0) {
		/* MLO_IPC is done. Now continue with non-MLO work */
		wl_mlo_ipc->ioc_complete = 0;
		return BCME_OK;
	} else {
		/* Timeout */
		WL_ERROR(("wl%d: %s: cmd %d MLO IPC timeout\n", wl->pub->unit,
			__FUNCTION__, ioc->cmd));
		return BCME_NOTREADY;
	}

done:
	return ret;
} // wl_mlo_ipc_wl_ioctl_intercept()

/* Wait forn MLO_IPC event to complete */
static int
wl_mlo_ipc_wait(wl_info_t *wl, wl_mlo_ipc_t *wl_mlo_ipc, uint *condition)
{
	int timeout;

	/* Convert timeout in millsecond to jiffies */
	timeout = msecs_to_jiffies(WL_MLO_IPC_WAIT_TIMEOUT);

	WL_UNLOCK(wl);

	timeout = wait_event_timeout(wl_mlo_ipc->mlo_ipc_wait, (*condition), timeout);

	WL_LOCK(wl);

	return timeout;
}

static int
wl_mlo_ipc_wake(wl_info_t * wl)
{
	wl_mlo_ipc_t *wl_mlo_ipc = WL_MLO_IPC(wl);

	wake_up(&wl_mlo_ipc->mlo_ipc_wait);

	return 0;
}

bool
wl_mlo_ipc_is_mlo_ap(wl_info_t *wl)
{
	return WL_IS_MLO_AP(wl);
}

uint16
wl_mlo_get_dump_signature(void)
{
	return mlo_ipc_sys_gp->dump_signature;
}

void
wl_mlo_reset_dump_signature(void)
{
	mlo_ipc_sys_gp->dump_signature = (uint16)OSL_RAND();
}

bool
wl_mlo_ipc_is_active(wl_info_t * wl)
{
	wl_mlo_ipc_t *wl_mlo_ipc = WL_MLO_IPC(wl);

	return (wl_mlo_ipc->mlc_state == mlo_ipc_mlc_active_e);
}

bool
wl_mlo_ipc_is_ready(wl_info_t * wl)
{
	wl_mlo_ipc_t *wl_mlo_ipc = WL_MLO_IPC(wl);

	return (wl_mlo_ipc->mlc_state >= mlo_ipc_mlc_ready_e) ? TRUE : FALSE;
}

/* Queue BCMC XMIT msg to one of the participating aux link */
int
wl_mlo_ipc_evt_bcmc_xmit_req(wl_info_t * wl, uint16 dest_ap_unit,
	void *pkt, uint16 seq, int8 mld_unit)
{
	mlo_ipc_evt_msg_t * evt_msg;
	mlo_ipc_bcmc_xmit_msg_t *xmit_msg;
	uint16	evt_msg_bytes;
	int	ret;

	evt_msg_bytes = sizeof(mlo_ipc_evt_msg_t) + sizeof(mlo_ipc_bcmc_xmit_msg_t);

	WL_INFORM(("%s: wl%d: Sending BCMC XMIT req event len %d\n",
		__FUNCTION__, wl->pub->unit, evt_msg_bytes));

	evt_msg = mlo_ipc_evt_msg_alloc(evt_msg_bytes);

	if (evt_msg == NULL) {
		WL_ERROR(("dhd%d: %s: MLO IPC EVENT MSG alloc failed\n",
			wl->pub->unit, __FUNCTION__));
		ASSERT(evt_msg);
		return BCME_NOMEM;
	}

	evt_msg->ap_unit	= wl->pub->unit;
	evt_msg->evt_id		= MLO_IPC_EVT_ID_BCMC_XMIT;
	evt_msg->evt_flags	= 0;
	evt_msg->bytes		= evt_msg_bytes;
	xmit_msg		= (mlo_ipc_bcmc_xmit_msg_t*)(evt_msg->data);
	xmit_msg->map_seq	= seq;
	xmit_msg->mld_unit	= mld_unit;
	xmit_msg->pkt		= (uint64)(uintptr)pkt;

	ret = mlo_ipc_evt_queue_enqueue(dest_ap_unit, evt_msg);

	if (ret	!= BCME_OK) {
		WL_ERROR(("%s: BCMC XMIT EVT enqueue to ap[%d]failed ret %d\n",
			__FUNCTION__, dest_ap_unit, ret));
		/* Release event message */
		mlo_ipc_evt_msg_free(evt_msg);
	}

	return ret;
} // wl_mlo_ipc_evt_bcmc_xmit_req

#endif /* MLO_IPC */
