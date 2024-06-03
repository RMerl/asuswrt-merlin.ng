/*
 * +--------------------------------------------------------------------------+
 * MLO_IPC: Inter Processor communication between MLO AP's
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
 * $Id: mlo_ipc.h 832812 2023-11-14 01:54:32Z $
 * +--------------------------------------------------------------------------+
 */

#ifndef _MLO_IPC_H_
#define _MLO_IPC_H_

#include <bcmpcie.h>

// Forward declarations
typedef struct mlo_ipc_sys		mlo_ipc_sys_t;
typedef struct mlo_ipc_msg		mlo_ipc_msg_t;
typedef struct mlo_ipc_peer_msg		mlo_ipc_peer_msg_t;
typedef struct mlo_ipc_msg_queue	mlo_ipc_msg_queue_t;
typedef struct mlo_ipc_evt_queue	mlo_ipc_evt_queue_t;

// MLO IPC HOST driver mlc_state
typedef enum mlo_ipc_mlc_state
{
	mlo_ipc_mlc_reset_e            = 0,    // MLO AP is not yet attached
	mlo_ipc_mlc_attach_e           = 1,    // MLO IPC allocated
	mlo_ipc_mlc_bind_e             = 2,    // MLO IPC MLC_BIND started
	mlo_ipc_mlc_link_e             = 3,    // MLO IPC MLC_LINK started

	mlo_ipc_mlc_bridge_e           = 4,    // MLO AP RC bridge setup
	mlo_ipc_mlc_sync_e             = 5,    // MLO AP UMAC sync with all peer UMAC started
	mlo_ipc_mlc_ready_e            = 6,    // AP is MLC READY, sent READY to dongle

	mlo_ipc_mlc_trap_e             = 7,    // MLO AP FW trap "PSM Wdog" detected
	mlo_ipc_mlc_halt_e             = 8,    // MLC HALT detected

	/* MLO_IPC_DOWN states */
	mlo_ipc_mlc_suspend_e          = 9,    // MLO IPC for mlc suspend
	mlo_ipc_mlc_suspend_halt_e     = 10,   // MLO IPC for mlc suspend with PSMWD
	mlo_ipc_mlc_down_req_e         = 11,   // Notify MAP to start mlo down
	mlo_ipc_mlc_down_e             = 12,   // MLO IPC for mlc down
	mlo_ipc_mlc_down_complete_e    = 13,   // Proceed to non-mlo down

	/* BCMMLO_GR states */
	mlo_ipc_mlc_bh_req_e           = 14,   // Notify MAP to start mlo bh
	mlo_ipc_mlc_bh_halt_req_e      = 15,   // Notify MAP to start mlo bh with PSMWD
	mlo_ipc_mlc_bh_e               = 16,   // MLO IPC for mlc bh
	mlo_ipc_mlc_bh_complete_e      = 17,   // Proceed to non-mlo bh

	mlo_ipc_mlc_down_bh_req_e      = 18,   // Notify MAP to start mlo down
	mlo_ipc_mlc_down_bh_e          = 19,   // MLO IPC for mlc down
	mlo_ipc_mlc_down_bh_complete_e = 20,   // Proceed to non-mlo bh

	mlo_ipc_mlc_active_e           = 21,   // MLO IPC for MLO_ACTIVE

	mlo_ipc_mlc_max_e

} mlo_ipc_mlc_state_t;

extern const char * __mlo_ipc_mlc_state_str[mlo_ipc_mlc_max_e];

/**
 * ------------------------------------------------------------------------------------------------
 * SECTION: MLO_IPC MSG
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
 * active list and inform MAP about new message.
 * MAP on receiving a msg will synchronize the state transition and move the MSG object to
 * free list.
 * ------------------------------------------------------------------------------------------------
 */

/** MLO_IPC MSG enqueue callback registered by MAP; AAP will enqueue the msg using this CB. */
typedef void (* mlo_ipc_map_msg_enqueue_fn_t)(int ap_unit, uint8 msg_id,
                                              uint8 msg_prio, uint8 msg_unit, void * data);

/* MLO_IPC State transition callback registered by AAP;
 * MAP will inform AAP about new state change using this callback.
 */
typedef void (* mlo_ipc_aap_state_change_cb_t)(int ap_unit, uint8 state, void * data);

#define MLO_IPC_MSG_PRIO_MAX		(2)

#define MLO_IPC_MSG_PRIO_LOW		(0)
#define MLO_IPC_MSG_PRIO_HIGH		(1)

// MSG_ID definitions based on MLO phases
typedef enum mlo_ipc_msg_id {
	MLO_IPC_MSG_ID_ATTACH		= 0,	// MLC_STATE attach complete
	MLO_IPC_MSG_ID_BIND		= 1,	// MLC_STATE bind complete
	MLO_IPC_MSG_ID_LINK		= 2,	// MLC_STATE link complete
	MLO_IPC_MSG_ID_BRIDGE		= 3,	// MLC_STATE link complete
	MLO_IPC_MSG_ID_SYNC		= 4,	// MLC_STATE sync complete
	MLO_IPC_MSG_ID_TRAP		= 5,	// dongle TRAP detected

	MLO_IPC_MSG_ID_SUSPEND		= 6,	// Report for suspend complete
	MLO_IPC_MSG_ID_DOWN_REQ		= 7,	// Request MAP for wl down
	MLO_IPC_MSG_ID_DOWN		= 8,	// Report for down complete

	/* BCMMLO_GR */
	MLO_IPC_MSG_ID_BH_REQ		= 9,	// Request MAP for bh
	MLO_IPC_MSG_ID_BH_HALT_REQ	= 10,	// Request MAP for bh with PSMWD
	MLO_IPC_MSG_ID_BH		= 11,	// Report for bh complete

	MLO_IPC_MSG_ID_DOWN_BH_REQ	= 12,	// Request MAP for mlo down
	MLO_IPC_MSG_ID_DOWN_BH		= 13,	// Report for mlo down complete

	MLO_IPC_MSG_ID_LAST
} mlo_ipc_msg_id_t;

// Inter MLO AP IPC message object
typedef struct mlo_ipc_msg {
	dll_t	node;
	uint8	id;		// Message ID mlo_ipc_msg_id
	uint8	prio;		// Priority of msg LOW/HIGH
	uint8	req_ap_bm;	// bitmap of AP unit's requesting for transition
} mlo_ipc_msg_t;

// Msg queue attached to MAP
typedef struct mlo_ipc_msg_queue {
	spinlock_t      lock;		// System wide LOCK
	mlo_ipc_msg_t	msg[MLO_IPC_MSG_ID_LAST]; // Array of messages indexed by mlo_ipc_msg_id
	dll_t		active_list;	// List of new messages recived from MLO AP's
	dll_t		free_list;

	// Stats
	uint32		enqueue_cnt;
	uint32		dequeue_cnt;
} mlo_ipc_msg_queue_t;

extern mlo_ipc_map_msg_enqueue_fn_t mlo_ipc_get_map_msg_enqueue_fn(void);
extern mlo_ipc_aap_state_change_cb_t mlo_ipc_get_state_change_cb(int ap_unit);

/**
 * -----------------------------------------------------------------------------------------------
 *  Section: MLO_IPC_EVENT
 *
 *  All MLO participating links need to share iovars/events/packets with each other.
 *
 *  Every link has an event queue attached to the MLO IPC. The producer link prepares the
 *  event message (mlo_ipc_evt_msg_t) and enqueues the message to the consumer link queue.
 *  After enqueueing the message, the Producer link wakes up the consumer link using a
 *  registered Wakeup callback (mlo_ipc_evt_wake_up_cb_t).
 *
 *  evt_id and evt_flags defines the type of message.
 *
 *  TODO: Define event flags for IOVAR, Data Packet etc.
 * ------------------------------------------------------------------------------------------------
 */

typedef void (* mlo_ipc_evt_wake_up_cb_t)(void * ctx);

typedef enum mlo_ipc_evt_id {

	MLO_IPC_EVT_ID_BCMC_XMIT = 1,

	MLO_IPC_EVT_ID_MAX
} mlo_ipc_evt_id_t;

typedef struct mlo_ipc_bcmc_xmit_msg {
	uint64  pkt;			// pkt pointer type casted to uint64
	uint16	map_seq;		// seqno generated by MAP
	int8	mld_unit;		// mld unit
} mlo_ipc_bcmc_xmit_msg_t;

// Inter AP MLO IPC Event message object
typedef struct mlo_ipc_evt_msg {
	dll_t	node;
	uint16	ap_unit;		// Source AP unit
	uint16	evt_id;			// MLO IPC Event ID
	uint16	evt_flags;		// For future use
	uint16	bytes;			// Length of the Event msg-(and event buffer)
	uint8	data[0];		// Event Buffer/Data
} mlo_ipc_evt_msg_t;

// Event Msg queue attached to the link
typedef struct mlo_ipc_evt_queue {
	spinlock_t      lock;		// System wide LOCK

	dll_t		active_list;	// List of new messages recived from MLO AP's
	mlo_ipc_evt_wake_up_cb_t wake_up_cb;	// Registered cosumer Wake up callback

	// Stats
	uint32		active_cnt;
	uint32		enqueue_cnt;
	uint32		dequeue_cnt;
	unsigned long	last_enq_time;
} mlo_ipc_evt_queue_t;

extern int	mlo_ipc_evt_queue_enqueue(uint16 dest_ap_unit, mlo_ipc_evt_msg_t * evt_msg);
extern mlo_ipc_evt_msg_t * mlo_ipc_evt_queue_dequeue(uint16 ap_unit);
extern mlo_ipc_evt_msg_t * mlo_ipc_evt_msg_alloc(int bytes);
extern void	mlo_ipc_evt_msg_free(mlo_ipc_evt_msg_t * evt_msg);

// System wide Run-time MLO_IPC state
struct mlo_ipc_sys {
	osl_t			* osh;
	pcie_ipc_mlo_t		* pcie_ipc_mlo;
	dmaaddr_t               pcie_ipc_mlo_pa;
	uint32			pcie_ipc_mlo_alloced;
	mlo_ipc_msg_queue_t	msg_queue;
	mlo_ipc_evt_queue_t	evt_queue[PCIE_IPC_AP_UNIT_MAX];

	void			* map_data;
	mlo_ipc_map_msg_enqueue_fn_t	msg_enqueue_fn;
	void			* aap_data[PCIE_IPC_AP_UNIT_MAX];
	mlo_ipc_aap_state_change_cb_t	state_change_cb[PCIE_IPC_AP_UNIT_MAX];

	uint8	mlo_ap_bm;
	uint8	ap_unit_map;
	uint16	dump_signature;
};

// Global instance of MLO IPC context
extern mlo_ipc_sys_t	* mlo_ipc_sys_gp;

/* MSG_QUEUE lock macros mutual exclusive access */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define MLO_IPC_MSG_QUEUE_LOCK()	spin_lock_bh(&(mlo_ipc_sys_gp->msg_queue.lock))
#define MLO_IPC_MSG_QUEUE_UNLK()	spin_unlock_bh(&(mlo_ipc_sys_gp->msg_queue.lock))
#define MLO_IPC_EVT_QUEUE_LOCK(unit)	spin_lock_bh(&(mlo_ipc_sys_gp->evt_queue[unit].lock))
#define MLO_IPC_EVT_QUEUE_UNLK(unit)	spin_unlock_bh(&(mlo_ipc_sys_gp->evt_queue[unit].lock))
#else   /* ! (CONFIG_SMP || CONFIG_PREEMPT) */
#define MLO_IPC_MSG_QUEUE_LOCK()	local_irq_disable()
#define MLO_IPC_MSG_QUEUE_UNLK()	local_irq_enable()
#define MLO_IPC_EVT_QUEUE_LOCK(unit)	local_irq_disable()
#define MLO_IPC_EVT_QUEUE_UNLK(unit)	local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

#define MLO_IPC_MSG(_id)		(&(mlo_ipc_sys_gp->msg_queue.msg[_id]))
#define MLO_IPC_MSG_QUEUE()		(&((mlo_ipc_sys_gp)->msg_queue))
#define MLO_IPC_EVT_QUEUE(unit)		(&((mlo_ipc_sys_gp)->evt_queue[unit]))

extern int	mlo_ipc_init(void);
extern void	mlo_ipc_exit(void);

// MLO AP context register/deregister handlers
extern int mlo_ipc_get_mlo_unit(int ap_unit);
extern int mlo_ipc_get_mlink_count(void);
extern void mlo_ipc_map_register(int ap_unit, mlo_ipc_map_msg_enqueue_fn_t msg_enqueue_fn,
	mlo_ipc_evt_wake_up_cb_t wake_up_cb, void * data, pcie_ipc_mlo_opmode_t op_mode);
extern void mlo_ipc_map_deregister(int ap_unit);

extern void mlo_ipc_aap_register(int ap_unit, mlo_ipc_aap_state_change_cb_t state_change_cb,
	mlo_ipc_evt_wake_up_cb_t wake_up_cb, void * data, pcie_ipc_mlo_opmode_t op_mode);
extern void mlo_ipc_aap_deregister(int ap_unit);

extern uint8 mlo_ipc_get_ap_bm(void);
extern uint8 mlo_ipc_set_ap_bm(int ap_unit);
extern uint8 mlo_ipc_clear_ap_bm(int ap_unit);
extern uint8 mlo_ipc_get_ap_unit_map(void);
extern void * mlo_ipc_get_map_data(void);
extern void * mlo_ipc_get_aap_data(int ap_unit);
extern uint8 mlo_ipc_get_sys_ap_bm(void);

extern struct device *mlo_ipc_get_dev(void);
#endif /* _MLO_IPC_H_ */
