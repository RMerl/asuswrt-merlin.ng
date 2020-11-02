/*
 * Reliable multicast implementation for Broadcom 802.11 Networking Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_relmcast.c 370989 2012-11-26 19:33:35Z $
 */
#include "wlc_relmcast.h"
#include <wlc_bmac.h>
#include <wlc.h>
#include <wlc_frmutil.h>
#include <wlc_pcb.h>

#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#define WL_RMC_SIGNATURE 0x02345689

#ifdef IBSS_RMC
#define WL_RMC_NOACK_TIMEOUT	1000 /* 1 second */
#else
#define WL_RMC_NOACK_TIMEOUT	5000 /* 5 second */
#endif // endif

/* periodic check timeout */
#define WL_RMC_DEFAULT_TIMEOUT	2 /* in seconds;  set 0 to disable */
#define WL_RMC_MAX_TIMEOUT	5 /* in seconds */
#define WL_RMC_RSSI_THRESHOLD	-80 /* Lowest allowable RSSI of an active rcver */
#define WL_RMC_DEFAULT_ARTMO    3000 /* Default of 3000 ms for AR timeout */

/* cal the exact af frame size we need for MALLOC */
#define ERMC_AF_SIZE (sizeof(wl_action_frame_t) - ACTION_FRAME_SIZE + sizeof(rmc_notify_af_t))

#define ERMC_AF_TX_ATTEMPTS	1	/* No. of attempts to transmit an action frame */

#define WL_ERMC_AFTX_PERIOD	300	/* default mcast notification act frame period ms */
/*  act frames use 6mbps */
#define WL_ERMC_DEFAULT_AFTX_RATE DOT11_RATE_6M
/* legacy: #define WL_ERMC_DEFAULT_DFTX_RATE DOT11_RATE_MAX */
#define ERMC_DFLT_TXRATE_VHT	VHT_RSPEC(8, 1)	/* MCS8,1x1 = 78mbit 256QAM FEC 3/4 */
#define WL_ERMC_DEFAULT_DFTX_RATE ERMC_DFLT_TXRATE_VHT

#define ERMC_NUM_AFTX_PERIOD_ALLOWED 5
#define ERMC_NUM_AFTX_AGED_OUT       70
#define ERMC_SHMEM_ACK_BIT	     0x0800
#define ERMC_INVALID_AMT_IDX	     127

#if defined(ERMC_DEBUG)
#define ERMC_TRACE	/* printf("TS:%d, %s \n", TmS, __FUNCTION__) */
#define ERMC_DBG(x) printf x
#else
#define ERMC_TRACE
#define ERMC_DBG(x)
#endif // endif

#define TmS OSL_SYSUPTIME()
#define ERMC_ERROR(x) printf x

/* bit mask for notif act frame data act_mcbmp */
#define ERMC_ACK_MCAST_ALL			0x01
#define ERMC_ACK_MCAST0				0x02
#define ERMC_ACK_MCAST1				0x04
#define ERMC_ACK_MCAST2				0x08
#define ERMC_ACK_MCAST3				0x10
#define ERMC_ACK_MCAST_MASK			0x1F
#define ERMC_ACK_MCAST_MASK_GROUPS		0x1E

#define ERMC_ACK_MCAST_VALID_MASK	(ERMC_ACK_MCAST_ALL | \
					ERMC_ACK_MCAST0 | \
					ERMC_ACK_MCAST1 | \
					ERMC_ACK_MCAST2 | \
					ERMC_ACK_MCAST3)

/* send 3 more act notif frames before rmc txer shuts off */
#define ERMC_AF_TX_BEFORE_TXOFF			2

#define ERMC_ACKER_RSSI_DELTA			-6  /* rssi delta for switching the ACKER */

#define ERMC_MAX_TRS_PER_MC_GRP			1   /* max no. of transmitters per mcast group */

#define ERMC_NUM_AFTX_PERIOD_ALLOWED		5
#define ERMC_HASH_TBL_SIZE			16

#define HASH_MAC(ea)				(((uint8 *)(ea))[5] & 0x0F)

#ifdef IBSS_RMC
enum rmcActionType {
	ERMC_ACT_NONE			= 0,	/* No action */
	ERMC_ACT_LEAD_SWITCH		= 1,	/* AR switch */
	ERMC_ACT_STOP			= 2,	/* ACK-all stop */
	ERMC_ACT_DELETE			= 3,	/* Delete AR */
	ERMC_ACT_CLEAR_ENTRIES		= 4,	/* ACK-all start */
};
#endif /* IBSS_RMC */

/* ermc commands passed in the subtype */
/* only ERMC_SUBTYPE_LEAD_SELECTED and ERMC_SUBTYPE_LEAD_CANCELED are used as subtype         */
/* The rest of the subtypes after ERMC_SUBTYPE_LEAD_CANCELED are not used in transmitted      */
/* AF - there are remapped from bcm_action type to the reset of the subtypes                  */
enum rmc_subtype {
	/* following are reserved for customer; do not change the order */
	ERMC_SUBTYPE_ENABLE_RMC         = 0,    /* enabled RMC feature */
	ERMC_SUBTYPE_DISABLE_RMC        = 1,    /* disabled RMC feature */
	ERMC_SUBTYPE_LEAD_SELECTED      = 2,    /* active receiver is selected */
	ERMC_SUBTYPE_LEAD_CANCELED      = 3,    /* inform act recvr that is not a leader */
	/* end reserved subtypes */
	/* The following are not used in SS action frame - they are mapped from action field */
	ERMC_SUBTYPE_RCVR_SET_ACKALL    = 4,    /* command to set ack all bit */
	ERMC_SUBTYPE_RCVR_CLEAR_ENTRIES = 5,    /* command to clear all entries in AMT */
	ERMC_SUBTYPE_RCVR_NO_ACT        = 6,    /* transmitter cannot select any active rcv */
};

/* ermc error definations */
enum rmc_returncode {
	ERMC_MC_NO_AMT_SLOT		= 1,	/* AMT table for mcast grps is fully occupied */
	ERMC_MC_NO_GLB_SLOT		= 2,	/* Global table is fully occupied */
	ERMC_MC_NOT_MIRRORED	        = 3,	/* Unable to mirror in AMT */
	ERMC_MC_EXISTING_TR		= 4,	/* Entry in AMT belongs to existing transmitter */
	ERMC_MC_EXIST_IN_AMT	        = 5,	/* Entry is in AMT */
	ERMC_MC_NOT_EXIST_IN_GBL        = 6,	/* Entry is not in global table */
	ERMC_MC_NOT_EXIST_IN_AMT        = 7,	/* Entry is not in AMT cache copy */
	ERMC_MC_UTILIZED		= 8,	/* Entry is already taken and being utilized */
	ERMC_MC_MIRRORED		= 9,	/* Entry is successfully mirrored */
	ERMC_MC_PROCESSED		= 10,	/* Operation on mcast entry processed fine */
	ERMC_MC_PROGRAMMED		= 11,	/* Entry is programmed into AMT table */
	ERMC_MC_INVALID_MASK		= 12,	/* Invalid mask */
	ERMC_MC_INVALID_OPCODE		= 13,	/* Invalid opcode */
	ERMC_MC_NOT_PROCESSED		= 14,	/* Not processed */
	ERMC_MC_TAKEN_BY_OTHER_TR	= 15,	/* Taken by other transmitter */
	ERMC_MC_EXCEED_MAX_TR_CNT	= 16,	/* Exceed maximun # of transmitter */
};

/* IOVar table */
enum rmc_iovar_tbl {

	/* IOV: IOV_RELMCAST_ACKREQ
		This IOVAR enables the tx side to require ACK for the specified multicast packets
	 */
	IOV_RELMCAST_ACKREQ	= 0,
	/* IOV: IOV_RELMCAST_TXACK
		This IOVAR enables the rx side to tx ACK when rx specified multicast packets
	*/
	IOV_RELMCAST_TXACK	= 1,
	/* IOV: IOV_RELMCAST_TXRATE
		This IOVAR get/set the transmit rate of  the specified multicast packets
	 */
	IOV_RELMCAST_TXRATE	= 2,
	/* IOV: IOV_RELMCAST_ACKMAC
		This IOVAR display/configures the multicast mac addresses needing ACKed
	 */
	IOV_RELMCAST_ACKMAC	= 3,
	/* IOV: IOV_RELMCAST_ACKTMO
		This IOVAR display/configures the timeout value of unACKed client as gone.
	 */
	IOV_RELMCAST_ACKTMO	= 4,
	/* IOV: IOV_RELMCAST_STATUS
		This IOVAR displays reliable multicast clients' status.
	 */
	IOV_RELMCAST_STATUS	= 5,
	/* IOV: IOV_RELMCAST_CHKTMO
		This IOVAR display/sets timeout value of periodic associated clients' check.
	 */
	IOV_RELMCAST_CHKTMO	= 6,
	/* IOV: IOV_RELMCAST_ACTF_TIME
		This IOVAR display/sets time to transmit action frames periodically.
	 */
	IOV_RELMCAST_ACTF_TIME	= 7,
	/* IOV: IOV_RELMCAST_STATS
		This IOVAR display/clear the statistical counters.
	 */
	IOV_RELMCAST_STATS	= 8,
	/* IOV: IOV_RELMCAST_RSSI_THRESH
		This IOVAR display/sets the lowest rssi of the receiver that can be an active
		receiver. Any recevier whose rssi value less than this won't be chosen as an
		active receiver.
	 */
	IOV_RELMCAST_RSSI_THRESH = 9,
	/* IOV: IOV_RELMCAST_RSSI_DELTA
	 * This IOVAR display/sets rssi delta.
	 */
	 IOV_RELMCAST_RSSI_DELTA = 10,
	/* IOV: IOV_RELMCAST_RSSI_DELTA
	 * This IOVAR display/sets rssi delta.
	 */
	IOV_RELMCAST_VSIE = 11,
	/* IOV: IOV_RELMCAST_ARTMO
	 * This IOVAR display/sets age out time for the Active Receiver
	 */
	IOV_RELMCAST_ARTMO = 12,
	/* IOV: IOV_RELMCAST_AR
	 * This IOVAR display/sets mac address of the active receiver
	 */
	IOV_RELMCAST_AR		= 13
};

static const bcm_iovar_t wlc_rmc_iovars[] = {
	{"rmc_ackreq", IOV_RELMCAST_ACKREQ,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(uint8)
	},
	{"rmc_txack", IOV_RELMCAST_TXACK,
	(0), IOVT_BOOL, 0
	},
	{"rmc_ackmac", IOV_RELMCAST_ACKMAC,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER,
	(sizeof(wl_rmc_trans_in_network_t))
	},
	{"rmc_txrate", IOV_RELMCAST_TXRATE,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"rmc_acktmo", IOV_RELMCAST_ACKTMO,
	(0), IOVT_UINT32, 0
	},
	{"rmc_chktmo", IOV_RELMCAST_CHKTMO,
	(0), IOVT_UINT32, 0
	},
	{"rmc_status", IOV_RELMCAST_STATUS,
	(0), IOVT_BUFFER, sizeof(wl_relmcast_status_t)
	},
	{"rmc_actf_time", IOV_RELMCAST_ACTF_TIME,
	(0), IOVT_UINT16, 0
	},
	{"rmc_rssi_thresh", IOV_RELMCAST_RSSI_THRESH,
	(0), IOVT_INT8, 0
	},
	{"rmc_stats", IOV_RELMCAST_STATS,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(wl_rmc_cnts_t)
	},
	{"rmc_rssi_delta", IOV_RELMCAST_RSSI_DELTA,
	(0), IOVT_UINT8, 0
	},
	{"rmc_vsie", IOV_RELMCAST_VSIE,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER,
	(sizeof(wl_rmc_vsie_t))
	},
	{"rmc_ar_timeout", IOV_RELMCAST_ARTMO,
	(0), IOVT_UINT16, 0
	},
	{"rmc_ar", IOV_RELMCAST_AR,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER,
	(sizeof(wl_rmc_entry_t))
	},
	{NULL, 0, 0, 0, 0 }
};

/* Transmitter Info which is maintained by every receiver */
typedef struct wlc_rmc_trans_info {
	struct ether_addr addr;         /* transmitter mac */
	uint32 time_val;                /* time val when last AF from transmitter was received */
	uint16 seq;                     /* last seq number of packet received from transmitter */
	uint16 artmo;                   /* AR timeout */
	uint8 amt_idx;                  /* amt table entry */
	uint16 flag;                    /* entry will be acked, not acked */
	struct ether_addr ar_mac;       /* active receiver mac */
	struct wlc_rmc_trans_info* next; /* pointer to the next one in the list */
} wlc_rmc_trans_info_t;

/* maintain a static array of pointers to wlc_rmc_trans_info_t */
static wlc_rmc_trans_info_t *ptr_tr_array[ERMC_HASH_TBL_SIZE];

/* Check reliable multicast acknowledgement needed or not for selective multicast packets. */
typedef int (*wlc_rmc_process_fn_t)(wlc_rmc_info_t * const rmcp,
	uint16 type, void *p, struct dot11_header *h, uint16 *mclp, uint32 *rspecp);

static int wl_ermc_xmitter_start(wlc_rmc_info_t* ermc);
static int wl_ermc_xmitter_stop(wlc_rmc_info_t* ermc);
static uint8 wlc_rmc_ackmac(wlc_rmc_info_t * const rmcp,
	const wl_rmc_entry_table_t * table);
static uint8 wlc_rmc_send_unicast_action(wlc_rmc_info_t *rmcp,
	uint8 subtype);
static uint8 wlc_rmc_send_ctrlframe(wlc_rmc_info_t *rmcp, uint8 count);

/* Tranmsit action frame */
static int wlc_rmc_transmitaf(wlc_rmc_info_t* rmc, wl_action_frame_t* af,
	ratespec_t rate_override, pkcb_fn_t fn);

/* Process XMTR_TO_RCVRS_STOP type message from transmitter */
static uint8 wlc_rmc_handle_stop_msg(wlc_rmc_info_t *ermc, rmc_notify_af_t *afdata);

/* Process XMTR_TO_RCVRS_START type message from transmitter */
static uint8 wlc_rmc_handle_start_msg(wlc_rmc_info_t *ermc, rmc_notify_af_t *afdata);
/* If any changes in scb state, call this function to find possible new acker */
static void wlc_rmc_check(wlc_rmc_info_t * rmcp, struct scb *scb);

/* Rmc check call back function */
static void wlc_rmc_check_cb(wlc_rmc_info_t * rmcp, scb_state_upd_data_t *data);

/* Rmc function to make sure that ack to multi-cast packet are processed properly */
static void wlc_rmc_dotxstatus(wlc_rmc_info_t * rmcp, bool acked);

/* packet tx completion callback */
static void wlc_rmc_txcomplete(wlc_info_t *wlc, void *pkt, uint txs);

/* Calculates rmc ie length */
static uint wlc_rmc_calc_rmc_ie_len(void *ctx,
                                   wlc_iem_calc_data_t *calc);
/* Write rmc ie call back */
static int wlc_rmc_write_rmc_ie(void *ctx, wlc_iem_build_data_t *build);

/* Parse rmc ie call back */
static int wlc_rmc_parse_rmc_ie(void *ctx, wlc_iem_parse_data_t *parse);

/* Beacon process routine */
static void wlc_relmcast_beacon_process(wlc_rmc_info_t * rmcp, wlc_bsscfg_t *cfg,
	uint8 *tag_params, int tag_params_len);

static void wlc_rmc_chktimer_update(wlc_rmc_info_t * const rmcp, const int tmo);
static uint8 wlc_rmc_stats(wlc_rmc_info_t * const rmcp, const wl_rmc_cnts_t *cnts);
static void wlc_rmc_status(wlc_rmc_info_t * const rmcp, wl_relmcast_status_t *statusp);

#ifdef AP
/* Process association request */
static void wlc_relmcast_assocreq_process(wlc_rmc_info_t * rmcp, struct scb *scb,
	uint8 *tag_params, int tag_params_len);
#endif // endif

/* Delete TA entry from AMT and update the AMT cache copy */
static uint8 wlc_rmc_del_ta_entry(wlc_rmc_info_t *rmcp, struct ether_addr *tr);

/* Free all the allocated wlc_rmc_trans_info_t before exiting the module */
static void wlc_rmc_free_trs(wlc_rmc_info_t * const rmcp);

/* Copy all the wlc_rmc_elem_t structures to the location pointed by tr_net */
static void wlc_rmc_get_trs(wlc_rmc_info_t * const rmcp, wl_rmc_trans_in_network_t* tr_net);

/* If ar_flag, add TA entry into AMT and update AMT cache copy. */
/* AMT is programmed to ack on all multi-cast packet with this SA address */
/* If ar_flag is not set, add TA into AMT cache copy and make sure AMT is */
/* not progammed to ACK for this TA                                       */
static uint8 wlc_rmc_add_ta(wlc_rmc_info_t *rmcp, struct ether_addr *tr_mac,
	rmc_notify_af_t *afdata, uint8 ar_flag);

/* Delete AMT index if not empty, just return otherwise, del and set attr to indicate */
/* that this TA entry no longer exist */
static uint8 wlc_rmc_amt_del_ta(wlc_info_t *wlc, wlc_rmc_trans_info_t *del_tr);

/* Get the index for this address if it is present. Otherwise, return free entry index */
static int8 wlc_rmc_get_amt_index(wlc_info_t *wlc, const struct ether_addr *ea,
	uint8* amt_idx, uint16* attr);

/* insert wlc_rmc_trans_info_t allocated structure in the front of the link list */
static inline uint8 wlc_rmc_insert_tr(wlc_rmc_trans_info_t **head_tr,
	wlc_rmc_trans_info_t* tr_info);

/* timer handler to check whether any AMT entry needs to be delected */
static void wlc_rmc_age_timer(void *arg);

/* get mac address of active receiver */
static void wlc_rmc_get_ar(wlc_rmc_info_t * const rmcp, wl_rmc_entry_t* ar_net);
/* manual selection of active receiver */
static int8 wlc_rmc_set_ar(wlc_rmc_info_t * const rmcp, wl_rmc_entry_t* ar_net);

static void wlc_rmc_newacker(wlc_rmc_info_t * const rmcp, struct scb *scb);
static void wlc_rmc_find_acker(wlc_rmc_info_t * const rmcp);

struct wlc_rmc_info {
	uint32			signature;		/* rmc signature */
	wlc_info_t		*wlc;			/* wlc info structure to back refer */
	struct scb		*mc_ackerscb;		/* active receiver's scb */
	struct ether_addr	mc_acker;		/* Lead rcvr (acker) mac address */
	bool			mc_txack;		/* TX ack or not */
	bool			mc_noack_timer_started;	/* Noack timer started */
	bool			mc_check_timer_started;	/* Check timer started */
	uint8			mc_periodtmo;		/* Period check timeout in sec */
	uint32			mc_noack_txnum;		/* number of pkts txed since NO ACK */
	uint32			mc_txrate;		/* transmit rate */
	struct ether_addr	mc_ackmac;		/* reliable mcast ACK required MAC */
	wlc_rmc_process_fn_t	mc_process_fn;		/* process funtion ptr and enable flag */
	struct wl_timer		*mc_noack_timer;	/* Acker timeout timer */
	struct wl_timer		*mc_check_timer;	/* Periodic check timer to find new acker */
	uint			mc_noacktmo;		/* Ack timeout value */
	uint			mc_lasttx;		/* timestamp of last tx null data for STA */

	/* NEW ERMC vars SHOULD go only BELOW this line, otherwise it may BREAK autoaban */

	struct wl_timer		*mc_aftx_timer; /* Periodic timer to transmit action frames  */
	uint32			af_txcnt;	/* act fram tx sequence */
	uint32			af_rxcnt;	/* act fram tx sequence */
	uint32			tx_bytes;	/* stats: total counter of txed bytes */
	uint16			af_cnt2off;	/* send act frame before transmitter change state */
	bool			aftx_timer_started;	/* keep track of action frame timer  */
	uint8			ermc_mode;	/* may be rcvr, initiator or initr+txmiter(dual) */
	int8			rssi_delta;	/* when switching a leader rcvr */
	uint16			err;
	wl_action_frame_t	*ctl_af;	/* control channel action frame struct ptr */
	rmc_notify_af_t		*af_data;	/* helper ptr to af_data within ctl_af */

	/* transmiter uses it to tx act frame, rcvrs just copy rcvd act frame in here as is */
	ratespec_t		af_txrate;
	struct ether_addr	mc_ctlmac;	/* mcast mac for the notification af ctl channel */
	struct ether_addr	self_mac;	/* self mac */
	struct ether_addr	af_sender_mac;
	/* gtbl not used - kept for rom compatiblity */
	char			*gtbl;		/* table containing global multi-cast entry */
	uint8			tr_mcbmp;	/* bitmask of multi-cast groups for transmit */
	uint16			mc_actf_period;	/* action frame tx period in ms */
	int8			rmc_lowrssi;	/* RMC RSSI threshold */
	wl_rmc_cnts_t		*ermc_cnts;	/* pointer to structure containing RMC counters */
#ifdef IBSS_RMC
	uint8	vendor_oui[3];	/* vendor oui */
	uint16	vendor_ie_data;		/* data payload to ie; passed through vsie iovar */
#endif // endif
	struct wl_timer		*mc_age_timer; /* Periodic check timer to age out entries */
	uint16			mc_artmo;      /* active receiver timeout in ms */
	wlc_rmc_trans_info_t	**trs;		/* arrays of pointers to wlc_rmc_tr_info */
	uint8			tr_cnt;		/* current number of transmitter in the network */
	bool			auto_ar_select; /* active receiver auto selection */
};

#ifdef IBSS_RMC

#include<packed_section_start.h>

#define RELMCAST_NON_BRCM_PROP_IE_TYPE 64

#define RMC_PROP_OUI		"\x00\x16\x32"	/* Customer's OUI */
#define RMC_MAGIC_STRING	"OXYGEN"        /* Customed RMC magic code */

/* specific action frame with variable length */
BWL_PRE_PACKED_STRUCT struct rmc_dot11_action_vs_frmhdr {
	uint8   category;
	uint8   oui[DOT11_OUI_LEN];
	uint8   magic[RMC_MAGIC_CODE_LEN];
	uint8   version;
	uint8   subtype;
	uint32  dialog_counter;
	uint8   data[1];
} BWL_POST_PACKED_STRUCT;
typedef struct rmc_dot11_action_vs_frmhdr rmc_dot11_action_vs_frmhdr_t;

#define RMC_DOT11_ACTION_VS_HDR_LEN  16

#include<packed_section_end.h>

#endif /* IBSS_RMC */

/* default values for the notif act frame */
static const rmc_notify_af_t         ctlaf_s = {
			0x7f,			/* category (vendor specific) */
#ifdef IBSS_RMC
			{0x00, 0x16, 0x32},	/* OUI */
			{'O', 'X', 'Y', 'G', 'E', 'N'}, /* magic word */
			0x01,			/* version */
			0x0,			/* subtype */
			0,			/* dialog_counter */
#else
			{0x00, 0x90, 0x4c},	/* OUI */
			BRCM_RELMACST_AF_TYPE,	/* type */
			0x05,			/* subtype */
			0x01,			/* version */
#endif // endif
			/* unicast mac of the leader_rcvr */
			{{0x00, 0x90, 0x4c, 0xb5, 0x43, 0x39}},   /* leader_mac */
#ifdef IBSS_RMC
			0x7f,		        /* bcm_category, VS (vendor specific) */
			{0x00, 0x90, 0x4c},     /* bcm_oui */
			ERMC_ACT_NONE,		/* bcm_action */
#endif // endif
			/* cmd bits for the Lead rcvr what to ACK */
			/* bit[7]=1:Lead_RCVR ack all; bit[0]:ACK mctable[0]; */
			/* ...bit[3]:ACK mctable[3]; bits[4,5,6]:RFU */
			0x00,		        /* act_mcbmp */
			/* mctable */
			{
			/*  default ACK MAC entries  */
			{{0x01, 0x00, 0x5e, 0x00, 0x00, 0x37}},
			{{0x01, 0x00, 0x5e, 0x00, 0x00, 0x38}},
			{{0x01, 0x00, 0x5e, 0x00, 0x00, 0x39}},
			{{0x01, 0x00, 0x5e, 0x00, 0x00, 0x3A}},
			},
			3000	/* artmo */
};

/*  default ERMC notification mcast mac */
#ifdef IBSS_RMC
static const struct ether_addr df_notify_af_mcastmac = {
	{0x01, 0x00, 0x5E, 0x00, 0x02, 0x0A}
};
#else
static const struct ether_addr df_notify_af_mcastmac = {
	{0x01, 0x00, 0x5E, 0x01, 0x00, 0x40}
};
#endif // endif

static inline void wl_rmc_assign_signature(wlc_rmc_info_t *rmcp, uint32 value)
{
	rmcp->signature = (uint32)value;
}

static inline void wl_rmc_check_signature(wlc_rmc_info_t *rmcp, uint32 value)
{
	ASSERT(rmcp->signature == (uint32)value);
}
static inline bool wl_rmc_is_p2p_go(wlc_info_t *wlc)
{
	int idx;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, idx, bsscfg) {
		if (P2P_GO(wlc, bsscfg)) {
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				if (SCB_P2P(scb) && (scb -> flags3 & SCB3_RELMCAST)) {
					ERMC_DBG(("p2p go is enabled\n"));
					return TRUE;
				} else {
					continue;
				}
			}
		}
	}
	return FALSE;
}

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/*
 * wlc_rmc_aftx_timer_cbfn send notification mcast act frames (aka ERMC ctrl channel)
 * from the transmitter to all receivers
 * This function is called when the rmc_aftx_timer  expired
 */
static void wlc_rmc_aftx_timer_cbfn(void *arg)
{
	int err;
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)arg;

	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_hwaddr(rmcp->wlc, &rmcp->self_mac);

	ASSERT(bsscfg != NULL);

	if (!rmcp->wlc->pub->up ||
	    !BSSCFG_IBSS(rmcp->wlc->cfg)) {
		ERMC_DBG(("aftx timer triggered in invalid mode/state\n"));
		return;
	}

	/* update leader mac in the act frame body */
	bcopy(&rmcp->mc_acker, &rmcp->af_data->leader_mac, ETHER_ADDR_LEN);

	/* update artmo value in the act frame body */
	rmcp->af_data->artmo = rmcp->mc_artmo;

	rmcp->ctl_af->packetId = rmcp->af_txcnt++;

	bcopy(&rmcp->mc_ctlmac, &rmcp->ctl_af->da,  ETHER_ADDR_LEN);

	err = wlc_send_action_frame(rmcp->wlc, bsscfg, NULL, rmcp->ctl_af);
	if (err != BCME_OK)
		rmcp->ermc_cnts->af_tx_err++;

	/* When af_cnt20ff is > 0, device is disabling its RMC transmitter */
	/* Device does not stop right away - it must ensure that remote receivers */
	/* have a good chance of receiving the ERMC_SUBTYPE_LEAD_CANCELED subtype */
	/* by transmitting this message af_cnt2off times.                      */
	/*  ERMC_XMTR_TO_RVRS_STOP msgs to receivers inform receivers to update */
	/* AMT and global tables i.e delete groups set up by this transmitter  */

	if  (rmcp->af_cnt2off) {
		/* the cnt down to txer shut-off has started */
		if (--rmcp->af_cnt2off == 0) {
		/* when cnt2off goes to zero, delete and clear the AMT of */
		/* all the entries that are set up by local transmitter/device  */
			rmcp->mc_process_fn = NULL;
			rmcp->wlc->pub->_rmc = FALSE;
			rmcp->mc_ackerscb = NULL;

			wlc_update_beacon(rmcp->wlc);

			/* clear acker mac */
			bzero(&rmcp->mc_acker, ETHER_ADDR_LEN);

			/* Delete timer to stop sending regular multi-cast */
			/* control msgs to receivers */
			wl_del_timer(rmcp->wlc->wl, rmcp->mc_aftx_timer);

			rmcp->aftx_timer_started = FALSE;

		}
	}

	if (err) {
		/* ignore for now, we'll get err for mcast act frames */
		ERMC_ERROR(("ERMC act frame tx failed\n"));
	}

	return;

}

/*
 * wlc_rmc_send_ctrlframe sends various multi-cast control messages to devices
 * in the network
 */
static uint8 wlc_rmc_send_ctrlframe(wlc_rmc_info_t *rmcp, uint8 count)
{
	int err = BCME_ERROR;

	/* arbitrary upper bound count 64 */
	if (count <= 0 || count > 64) {
		ERMC_ERROR(("RMC ERR: count is out of bound\n"));
		return BCME_ERROR;
	}

	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_hwaddr(rmcp->wlc, &rmcp->self_mac);

	ASSERT(bsscfg != NULL);

	if (!rmcp->wlc->pub->up) {
		ERMC_ERROR(("ERROR: wl is not up\n"));
		return BCME_ERROR;
	}

	ERMC_DBG(("Sending ctrl frame subtype %d \n",
		rmcp->af_data->subtype));

	/* update leader mac in the act frame body */
	bcopy(&rmcp->mc_acker, &rmcp->af_data->leader_mac, ETHER_ADDR_LEN);

	rmcp->ctl_af->packetId = rmcp->af_txcnt++;
#ifdef IBSS_RMC
	if (rmcp->af_data->subtype == ERMC_SUBTYPE_LEAD_CANCELED) {
		if (ETHER_ISNULLADDR(&rmcp->mc_acker)) {
			/* not to transmit a cancelation frame when no acker */
			count = 0;
		} else {
			bcopy(&rmcp->mc_acker, &rmcp->ctl_af->da, ETHER_ADDR_LEN);
		}
	}
#else
	bcopy(&rmcp->mc_ctlmac, &rmcp->ctl_af->da,  ETHER_ADDR_LEN);
#endif /* IBSS_RMC */

	for (; count > 0; count--) {

		err = wlc_send_action_frame(rmcp->wlc, bsscfg, NULL, rmcp->ctl_af);

		if (err != BCME_OK) {
			ERMC_ERROR(("ERMC act frame tx failed: %s\n", __FUNCTION__));
			rmcp->ermc_cnts->af_unicast_tx_err++;
		} else {
			err = BCME_OK;
		}
	}

	return err;
}

/*
 * wlc_rmc_sendnull send uni-cast null message to transmitter
 */
static void wlc_rmc_sendnull(wlc_rmc_info_t * const rmcp, wlc_bsscfg_t *bsscfg)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	struct ether_addr *da = NULL;

	/* wlc->pub->now is in sec. but we need to transmit null frame approx. every 500ms. */
	/* factor (normalize) to avoid floating operation */
	int normalize = rmcp->mc_periodtmo * 2;

	/* for infra, transmit null frame only on timer expiry */
	if (!BSSCFG_IBSS(rmcp->wlc->cfg)) {
		/* transmit null frame every arbitrary no. of beacons which is 5 */
		if (((rmcp->wlc->pub->now * normalize) - rmcp->mc_lasttx) <
		    (rmcp->mc_periodtmo)) {
			return;
		}
	}

	/*  note for IBSS: da can't be &bsscfg->BSSID  */
	if (BSSCFG_IBSS(rmcp->wlc->cfg))
		da = &rmcp->af_sender_mac;
	else
		da = &bsscfg->BSSID;

	if (wlc_sendnulldata(rmcp->wlc, bsscfg, da, 0, 0, -1, NULL, NULL)) {
		rmcp->mc_lasttx = rmcp->wlc->pub->now * normalize;
	} else {
		rmcp->ermc_cnts->null_tx_err++;
		ERMC_ERROR(("Unable to send null frame to %s\n",
		             bcm_ether_ntoa(&rmcp->af_sender_mac, eabuf)));
	}

}

/* This is the main function that processes rmc multi-cast action frame
 * both in receiver/transmitter
 */
int wlc_rmc_recv_action_frames(wlc_info_t *wlc, struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh)
{
	wlc_rmc_info_t* ermc = wlc->rmc;
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, &ermc->self_mac);
	rmc_notify_af_t afdata;
	int ret = BCME_OK;
	bool flag = TRUE;
#ifdef IBSS_RMC
	bool is_brcm_action;
#endif // endif

	ASSERT(wlc != NULL);
	ASSERT(body != NULL);

	if (ermc->ermc_mode == WL_RMC_MODE_INITIATOR) {
		/* remote initiator doesn't need action frame from xmiter */
		return ret;
	}

	/* copy received notif af data to our local var   */
	bcopy((rmc_notify_af_t *)body, &afdata, sizeof(rmc_notify_af_t));

	/* In infra, initiator's unicast action frame will have null AR */
	if (BSSCFG_IBSS(wlc->cfg) &&
	    !bcmp(&afdata.leader_mac, &ether_null, ETHER_ADDR_LEN))  {
		afdata.subtype = ERMC_SUBTYPE_RCVR_NO_ACT;
	}

#ifdef IBSS_RMC
	/* translate the action type to subtype */
	is_brcm_action = !bcmp(afdata.bcm_oui, BRCM_PROP_OUI, sizeof(afdata.bcm_oui));
	if (is_brcm_action) {
	/* translate the action type to subtype */
		switch (afdata.bcm_action) {
		case ERMC_ACT_STOP:
			ERMC_DBG(("REMAP: ERMC_ACT_STOP -> ERMC_SUBTYPE_LEAD_CANCELED\n"));
			afdata.subtype = ERMC_SUBTYPE_LEAD_CANCELED;
			break;
		case ERMC_ACT_DELETE:
			ERMC_DBG(("REMAP: ERMC_ACT_DELETE -> ERMC_SUBTYPE_LEAD_CANCELED\n"));
			afdata.subtype = ERMC_SUBTYPE_LEAD_CANCELED;
			break;
		case ERMC_ACT_CLEAR_ENTRIES:
			ERMC_DBG(("REMAP: ERMC_ACT_CLEAR_ENTRIES \n"));
			afdata.subtype = ERMC_SUBTYPE_RCVR_CLEAR_ENTRIES;
			break;
		}
	}
#endif /* IBSS_RMC */
	if ((BSSCFG_AP(wlc->cfg) ||
	     wl_rmc_is_p2p_go(wlc)) &&
	     ((afdata.subtype != ERMC_SUBTYPE_LEAD_CANCELED) ||
	     (afdata.subtype != ERMC_SUBTYPE_LEAD_SELECTED))) {
#ifdef IBSS_RMC
		if (afdata.subtype != ERMC_SUBTYPE_DISABLE_RMC ||
			afdata.subtype != ERMC_SUBTYPE_ENABLE_RMC)
#endif // endif
		{
			ERMC_ERROR(("%s: Received subtype %d in Infra mode\n",
			           __FUNCTION__, afdata.subtype));
			return BCME_ERROR;
		}
	}
	switch (afdata.subtype) {

	case ERMC_SUBTYPE_DISABLE_RMC:
		/* we are the xmtr  need to stop ermc */
		ERMC_DBG(("ERMC_AF_SUBTYPE_RMC_DISABLING_REQUEST received\n"));
		if (!BSSCFG_IBSS(bsscfg)) {
			wl_ermc_xmitter_stop(ermc);
		}

		/* update mc_ackmac from initiator */
		if (BSSCFG_AP(wlc->cfg) ||
		    wl_rmc_is_p2p_go(wlc)) {

			bcopy(&afdata.mctable[0], &ermc->mc_ackmac,
			      sizeof(ermc->mc_ackmac));
			/* update rmcp's mctable */
			bcopy(&afdata.mctable[0],
			      &ermc->af_data->mctable[0],
			      sizeof(ermc->mc_ackmac));
			/* update beacon with new info */
			wlc_update_beacon(ermc->wlc);
		}
		flag = FALSE;
		break;

	case ERMC_SUBTYPE_ENABLE_RMC:
		/* we are the xmtr  need to start ermc */
		if (ermc->wlc->pub->_rmc != TRUE)
			wl_ermc_xmitter_start(ermc);

		/* update mc_ackmac from initiator */
		if (BSSCFG_AP(wlc->cfg) ||
		    wl_rmc_is_p2p_go(wlc)) {

			bcopy(&afdata.mctable[0], &ermc->mc_ackmac,
			      sizeof(ermc->mc_ackmac));
			/* update rmcp's mctable */
			bcopy(&afdata.mctable[0], &ermc->af_data->mctable[0],
			      sizeof(ermc->mc_ackmac));
			/* update beacon with new info */
			wlc_update_beacon(ermc->wlc);
			flag = FALSE;
		}
		break;

	case ERMC_SUBTYPE_LEAD_SELECTED:
		ermc->ermc_cnts->mc_ar_role_selected++;
		/* rcvr got periodical notif af from trasmitter */
		/* make a note of the sender's mac (it can be initr or xmtr)  */
		bcopy(&hdr->sa, &ermc->af_sender_mac, ETHER_ADDR_LEN);

		/* processed ERMC_XMTR_TO_RCVRS_START message */
		ret = wlc_rmc_handle_start_msg(ermc, &afdata);

		break;

	case ERMC_SUBTYPE_LEAD_CANCELED:
		/* rcvr got periodical notif af from trasmitter */
		/* make a note of the sender's mac (it can be initr or xmtr)  */
		bcopy(&hdr->sa, &ermc->af_sender_mac, ETHER_ADDR_LEN);

		ret = wlc_rmc_handle_stop_msg(ermc, &afdata);

		break;

	case ERMC_SUBTYPE_RCVR_NO_ACT:
		ERMC_DBG((" No Active Receiver\n"));
		ermc->ermc_cnts->mc_null_ar_cnt++;
		break;

	default:
		ERMC_ERROR(("unknown actf type\n"));
		flag = FALSE;
		break;
	}

	if (flag == TRUE) {
		ASSERT(bsscfg != NULL);
#ifdef IBSS_RMC
	/* allowing null data response only for the periodic action frame */
	if ((afdata.subtype == ERMC_SUBTYPE_LEAD_SELECTED) &&
		(afdata.bcm_action == ERMC_ACT_NONE))
#endif // endif
		if (BSSCFG_IBSS(wlc->cfg))
			wlc_rmc_sendnull(ermc, bsscfg);
	}
	ermc->af_rxcnt ++;
	return ret;
}

/* Process the ERMC_SUBTYPE_LEAD_SELECTED control messages from transmitter */
static uint8 wlc_rmc_handle_start_msg(wlc_rmc_info_t *ermc, rmc_notify_af_t *afdata)
{
	uint8 ret = ERMC_MC_PROCESSED;
#ifdef ERMC_DEBUG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (bcmp(&afdata->leader_mac, &ermc->self_mac, ETHER_ADDR_LEN) == 0) {
		/* rcvr is the leader */
		/* Take care of multiple transmitter scenario */
		/* Do not overwrite any AMT entries that are active */
		ret =  wlc_rmc_add_ta(ermc, &ermc->af_sender_mac, afdata, TRUE);
		if (ret != ERMC_MC_PROCESSED) {
			ret = ERMC_MC_NOT_PROCESSED;
		}
		ermc->mc_txack = TRUE;
		ERMC_DBG(("Active Receiver for %s \n",
			bcm_ether_ntoa(&ermc->af_sender_mac, eabuf)));
	} else {
		ERMC_DBG(("Non-active Receiver for %s \n",
			bcm_ether_ntoa(&ermc->af_sender_mac, eabuf)));
		ret = wlc_rmc_add_ta(ermc, &ermc->af_sender_mac, afdata, FALSE);
	}
	return ret;
}

/* Process the ERMC_SUBTYPE_LEAD_CANCELED control message from transmitter */
static uint8 wlc_rmc_handle_stop_msg(wlc_rmc_info_t *ermc, rmc_notify_af_t *afdata)
{
	uint8 ret = BCME_OK;
	ASSERT(ermc != NULL);
	ASSERT(afdata != NULL);

	if (bcmp(&afdata->leader_mac, &ermc->self_mac, ETHER_ADDR_LEN) == 0) {
		ermc->ermc_cnts->mc_ar_role_deleted++;
		/* rcvr is the leader */
		/* Do not overwrite any AMT entries that is active */
		ret = wlc_rmc_del_ta_entry(ermc, &ermc->af_sender_mac);
	} else {
		/*  delete tr in the list */
		ret = wlc_rmc_del_ta_entry(ermc, &ermc->af_sender_mac);
	}
	return ret;
}

/*
 * Function, wlc_rmc_ackmac is invoked as a result of the wl "rmc_ackmac" command
 * This function adds, deletes multi-cast entries
 */
/*   "rmc_ackmac" iovar SET handler  */
static uint8 wlc_rmc_ackmac(wlc_rmc_info_t * const rmcp,
	const wl_rmc_entry_table_t *table)
{
	uint8 opcode, flag = TRUE;
	uint8 err = BCME_OK;
	uint8 idx = table->index;

	ASSERT(rmcp != NULL);
	ASSERT(table != NULL);
	ERMC_DBG(("%s: mac index:%d, op_code:%d\n",
		__FUNCTION__, idx, table->opcode));

	opcode = table->entry[idx].flag;

	/* for now, just handle insertion of one wl_rmc_entry_t */
	switch (opcode) {

		case RELMCAST_ENTRY_OP_ENABLE:
			if (idx == 8) {
			/* special case: change notif act frame mcast mac */
			/* Need iovar to change it */
				bcopy(&table->entry[0].addr, &rmcp->mc_ctlmac, ETHER_ADDR_LEN);
				break;
			} else {
				ERMC_DBG(("Invalid IOVAR with this idx %d\n", idx));
			}
			break;

		default:
			ERMC_DBG(("%s: ERROR: don't know OPCODE:%d\n",
				__FUNCTION__, opcode));
			err = ERMC_MC_INVALID_OPCODE;
			flag = FALSE;
			break;
	}
	/* update only if it is AP */
	if (flag && (BSSCFG_AP(rmcp->wlc->cfg) ||
	             wl_rmc_is_p2p_go(rmcp->wlc))) {
		wlc_update_beacon(rmcp->wlc);
	}

	return err;

}

/*
 * Function, wlc_rmc_set_ar is invoked as a result of the wl "rmc_ar" command
 * This function set active receiver manually
 */
/*   "rmc_ar" iovar SET handler  */
static int8 wlc_rmc_set_ar(wlc_rmc_info_t * const rmcp, wl_rmc_entry_t* ar_net)
{
	uint8 err = BCME_OK;
	struct scb_iter scbiter;
	struct scb *scb = NULL, *scb2 = NULL;

	if (!bcmp(&ether_null, &ar_net->addr, ETHER_ADDR_LEN)) {
		wlc_eventq_set_ind(rmcp->wlc->eventq, WLC_E_RMC_EVENT, 0);
		rmcp->auto_ar_select = TRUE;
		wlc_rmc_find_acker(rmcp);
		ERMC_ERROR(("ERMC mode changed to auto : %s\n", __FUNCTION__));
		return err;
	}

	FOREACHSCB(rmcp->wlc->scbstate, &scbiter, scb) {
		if ((((SCB_ASSOCIATED(scb) &&
			(BSSCFG_AP(scb->bsscfg) || SCB_P2P(scb)))) ||
			BSSCFG_IBSS(scb->bsscfg)) &&
			(scb->flags3 & SCB3_RELMCAST) &&
			!(scb->flags3 & SCB3_RELMCAST_NOACK)) {
			if (!bcmp(&scb->ea, &ar_net->addr, ETHER_ADDR_LEN)) {
				scb2 = scb;
				break;
			}
		}
	}

	if (scb2) {
		ERMC_ERROR(("ERMC mac address found : %s\n", __FUNCTION__));
		rmcp->auto_ar_select = FALSE;
		wlc_eventq_set_ind(rmcp->wlc->eventq, WLC_E_RMC_EVENT, 1);
		wlc_rmc_newacker(rmcp, scb2);
	} else {
		ERMC_ERROR(("ERMC mac address not found, mode changed to auto : %s\n",
			__FUNCTION__));
		wlc_eventq_set_ind(rmcp->wlc->eventq, WLC_E_RMC_EVENT, 0);
		rmcp->auto_ar_select = TRUE;
		wlc_rmc_find_acker(rmcp);
		err = BCME_ERROR;
	}

	return err;
}

/*
 * "rmc_ar" iovar SET handler
 * copy activer receiver mac address into memory location arg
 */
static void wlc_rmc_get_ar(wlc_rmc_info_t * const rmcp, wl_rmc_entry_t* ar_net)
{
	ar_net->flag = 0x00;
	bcopy(&rmcp->mc_ackerscb->ea, &ar_net->addr, ETHER_ADDR_LEN);
}

/*
 * Function, wlc_rmc_stats is invoked as a result of the wl "rmc_stats" command
 * This function display/clear the statistical counters
 */
/*   "rmc_stats" iovar SET handler  */
static uint8 wlc_rmc_stats(wlc_rmc_info_t * const rmcp,
	const wl_rmc_cnts_t *cnts)
{
	uint8 err = BCME_OK;

	/* get all counters values */
	if (cnts == NULL)
		return BCME_ERROR;

	bcopy((char*)rmcp->ermc_cnts, (char*)cnts, sizeof(*rmcp->ermc_cnts));
	return err;
}

/* Function, wlc_rmc_newacker is called by transmitter to decide whether to select
 * another sta as a active receiver/acker
 */

#ifdef AP
static void wlc_rmc_newacker(wlc_rmc_info_t * const rmcp, struct scb *scb)
{
	uint subtype;
#ifdef ERMC_DEBUG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (rmcp->mc_process_fn) {
		ERMC_DBG(("%s %p %p\n", __FUNCTION__, scb, rmcp->mc_ackerscb));

		/* have to send a ctl message to delete old acker groups */
		/* if before we switch over */

		if (rmcp->mc_ackerscb && scb != rmcp->mc_ackerscb) {
			subtype = rmcp->af_data->subtype;
			rmcp->af_data->subtype =  ERMC_SUBTYPE_LEAD_SELECTED;
#ifdef IBSS_RMC
			/* This is to inform receivers in IBSS_RMC scheme */
			rmcp->af_data->bcm_action =  ERMC_ACT_DELETE;
#endif // endif
			if (wlc_rmc_send_ctrlframe(rmcp, ERMC_AF_TX_ATTEMPTS) != BCME_OK) {
				ERMC_ERROR(("ERMC act frame tx failed: %s\n", __FUNCTION__));
			}

			rmcp->af_data->subtype = subtype;
#ifdef IBSS_RMC
			rmcp->af_data->bcm_action = ERMC_ACT_NONE;
#endif // endif
		}

		if (scb) {
			bcopy(&scb->ea, &rmcp->mc_acker, ETHER_ADDR_LEN);
		} else {
			bzero(&rmcp->mc_acker, ETHER_ADDR_LEN);
		}

		rmcp->mc_ackerscb = scb;

		ERMC_DBG(("\n%s: %s ", __FUNCTION__, bcm_ether_ntoa(&rmcp->mc_acker, eabuf)));

		if (rmcp->mc_check_timer_started) {
			rmcp->mc_check_timer_started = FALSE;
			wl_del_timer(rmcp->wlc->wl, rmcp->mc_check_timer);
		}

		if (scb) {
			if (rmcp->mc_periodtmo) {
				rmcp->mc_check_timer_started = TRUE;
				wl_add_timer(rmcp->wlc->wl, rmcp->mc_check_timer,
					rmcp->mc_periodtmo*1000, FALSE);
			}
		}
		wlc_update_beacon(rmcp->wlc);
		rmcp->af_data->subtype = ERMC_SUBTYPE_LEAD_SELECTED;
#ifdef IBSS_RMC
		rmcp->af_data->bcm_action = ERMC_ACT_LEAD_SWITCH;
#endif // endif

		if (!(BSSCFG_AP(rmcp->wlc->cfg) ||
		     wl_rmc_is_p2p_go(rmcp->wlc))) {

			wlc_rmc_aftx_timer_cbfn(rmcp);
		}
#ifdef IBSS_RMC
		rmcp->af_data->bcm_action = ERMC_ACT_NONE;
#endif // endif
	}
}

/* The function, wlc_rmc_find_acker is called to find a new active receiver
 * based on the RSSI values. It monitors to make sure the RSSI values are
 * valid
 */
static void wlc_rmc_find_acker(wlc_rmc_info_t * const rmcp)
{
	int rssi = 10000, rssi1;
	struct scb_iter scbiter;
	struct scb *scb = NULL, *scb2 = NULL;

	if (rmcp->mc_process_fn) {
		if (rmcp->mc_ackerscb &&
			!(rmcp->mc_ackerscb->flags3 & SCB3_RELMCAST_NOACK)) {
			if ((1000*(rmcp->wlc->pub->now - rmcp->mc_ackerscb->used)) >
				rmcp->mc_artmo) {
				ERMC_DBG(("AR scb->used:%d\n", rmcp->mc_ackerscb->used));
				rmcp->mc_ackerscb->flags3 |= SCB3_RELMCAST_NOACK;
				wlc_scb_rssi_init(rmcp->mc_ackerscb, WLC_RSSI_INVALID);
			}
			else {
				/* Current acker is still acking, else use rssi 10000 */
				rssi = wlc_scb_rssi(rmcp->mc_ackerscb);
			}
		}

		FOREACHSCB(rmcp->wlc->scbstate, &scbiter, scb) {
			if ((((SCB_ASSOCIATED(scb) &&
			    (BSSCFG_AP(scb->bsscfg) || SCB_P2P(scb)))) ||
				BSSCFG_IBSS(scb->bsscfg)) &&
				(scb->flags3 & SCB3_RELMCAST) &&
				scb != rmcp->mc_ackerscb) {
				if ((scb->flags3 & SCB3_RELMCAST_NOACK)) {
					if ((1000*(rmcp->wlc->pub->now - scb->used)) <=
						(rmcp->mc_actf_period*
						ERMC_NUM_AFTX_PERIOD_ALLOWED))
						/* if scb is used before aftx period expiry,
						 *  remove sta from black list
						 */
						scb->flags3 &= ~SCB3_RELMCAST_NOACK;

				}
				else
				{
					if ((1000*(rmcp->wlc->pub->now - scb->used)) >
						(rmcp->mc_actf_period*
						ERMC_NUM_AFTX_PERIOD_ALLOWED)) {

						ERMC_DBG(("%s: scb->used:%d\n", __FUNCTION__,
							scb->used));
						/* if scb is not used within aftx period,
						 *  keep it in black list.
						 *  when a sta is in black list, it cannot be an AR
						 */
						scb->flags3 |= SCB3_RELMCAST_NOACK;
						/* rssi in not updated in aftx period, it is stale;
						 * mark it invalid
						 */
						wlc_scb_rssi_init(scb, WLC_RSSI_INVALID);
					}
				}
				if (!(scb->flags3 & SCB3_RELMCAST_NOACK) &&
					rmcp->auto_ar_select == TRUE) {
					rssi1 = wlc_scb_rssi(scb);
					if  (rssi1 < (rssi+rmcp->rssi_delta) &&
						rssi1 >= rmcp->rmc_lowrssi) {
						rssi1 = wlc_scb_rssi(scb);
						scb2 = scb;
						rssi = rssi1;
					}
				}
			}
		}

		if ((scb2 && scb2 != rmcp->mc_ackerscb && rmcp->auto_ar_select == TRUE)||
			(!scb2 && rmcp->mc_ackerscb &&
			(rmcp->mc_ackerscb->flags3 & SCB3_RELMCAST_NOACK))) {
			if (rmcp->auto_ar_select == FALSE) {
				rmcp->auto_ar_select = TRUE;
				wlc_mac_event(rmcp->wlc, WLC_E_RMC_EVENT, NULL,
				WLC_E_STATUS_ABORT, WLC_E_REASON_RMC_AR_LOST, 0, 0, 0);
			}
			wlc_rmc_newacker(rmcp, scb2);
		}
		else if (rmcp->mc_check_timer_started && rmcp->mc_periodtmo) {

			wl_add_timer(rmcp->wlc->wl,
			             rmcp->mc_check_timer,
			             rmcp->mc_periodtmo*1000, FALSE);
		}
	}
}

/* handler for no ack timer */
static void wlc_rmc_noack_timer(void *arg)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)arg;
	rmcp->ermc_cnts->mc_noacktimer_expired++;

	if (rmcp->mc_ackerscb && rmcp->mc_noack_timer_started &&
		rmcp->mc_noack_txnum > 0) {

		/* Acker timeout without ACK */
		rmcp->mc_ackerscb->flags3 |= SCB3_RELMCAST_NOACK;
		rmcp->mc_noack_timer_started = FALSE;
		if (rmcp->auto_ar_select == FALSE) {
			rmcp->auto_ar_select = TRUE;
			wlc_mac_event(rmcp->wlc, WLC_E_RMC_EVENT,
				NULL, WLC_E_STATUS_ABORT, WLC_E_REASON_RMC_AR_NO_ACK, 0, 0, 0);
		}
		wlc_rmc_find_acker(rmcp);
	}
}
/* on timer to check on the rcvr devices if there is a new acker */
static void wlc_rmc_check_timer(void *arg)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)arg;

	if (rmcp->mc_ackerscb && rmcp->mc_process_fn &&
		rmcp->mc_check_timer_started) {

		/* on the transmitter: Periodic Check */
		wlc_rmc_find_acker(rmcp);
	}
}

/* Sets the txrate and ack bit for RMC */
static int wlc_rmc_process_fn(wlc_rmc_info_t * const rmcp,
	uint16 type, void *pkt, struct dot11_header *h, uint16 *mclp,
	ratespec_t *rspecp)
{
	bool ack_set = FALSE;

	/* if ackreq is set to 1 on this device, then ackreq is needed */
	if (rmcp->ermc_mode == WL_RMC_MODE_TRANSMITTER) {
		ack_set = TRUE;
	}

	/* Reliable multicast frame needs ACK and set tx rate */
	if (type == FC_TYPE_DATA && ETHER_ISMULTI(&h->a1)) {

		if (ack_set == TRUE) {
			if (rmcp->mc_ackerscb) {
				*mclp |= D11AC_TXC_IACK;
			}

			if (rmcp->mc_txrate) {
				*rspecp = rmcp->mc_txrate;
			}

			if (rmcp->mc_noack_timer_started) {
				rmcp->mc_noack_txnum++;
			}
			return TRUE;
		}
	}
	return FALSE;
}

static void wlc_rmc_chktimer_update(wlc_rmc_info_t * const rmcp, const int tmo)
{
	rmcp->mc_periodtmo = tmo;

	ERMC_TRACE;

	if (!rmcp->mc_process_fn)
		return;

	wlc_update_beacon(rmcp->wlc);

	if (!tmo && rmcp->mc_check_timer_started) {

		wl_del_timer(rmcp->wlc->wl, rmcp->mc_check_timer);
		rmcp->mc_check_timer_started = FALSE;

	} else if (tmo)	{

		if (rmcp->mc_check_timer_started) {
			wl_del_timer(rmcp->wlc->wl, rmcp->mc_check_timer);
		}

		wl_add_timer(rmcp->wlc->wl,
		             rmcp->mc_check_timer,
		             rmcp->mc_periodtmo*1000, FALSE);

		rmcp->mc_check_timer_started = TRUE;
	}
	return;
}

/* start the ermc transmitter */
static int wl_ermc_xmitter_start(wlc_rmc_info_t* rmcp)
{
	rmcp->mc_process_fn = wlc_rmc_process_fn;
	rmcp->wlc->pub->_rmc = TRUE;

	rmcp->af_data->subtype = ERMC_SUBTYPE_LEAD_SELECTED;

	if (!rmcp->aftx_timer_started) {

		/* start ermcc act frame timer if it hasn't been yet */
		wl_add_timer(rmcp->wlc->wl,
		             rmcp->mc_aftx_timer,
		             rmcp->mc_actf_period, TRUE);

		rmcp->aftx_timer_started = TRUE;

		rmcp->af_cnt2off = 0;
	}

	wlc_rmc_chktimer_update(rmcp, rmcp->mc_periodtmo);
	wlc_rmc_find_acker(rmcp);
	wlc_update_beacon(rmcp->wlc);  /* leagcy rmc, need RMC IE  */

	return FALSE;
}

/* stop the ermc transmitter */
static int wl_ermc_xmitter_stop(wlc_rmc_info_t* rmcp)
{
	if (rmcp->aftx_timer_started) {
		rmcp->af_cnt2off = ERMC_AF_TX_BEFORE_TXOFF;
	}

	/* indicate to rcvrs all streasm acks must be swtchd off */
	/* rmcp->af_data->act_mcbmp = 0x00; - only when it completely stop  */
	/* Need a new subtype to remove AMT entries from transmitter  */
	/* Reason: we now use action mask to operated on AMT entries  */
	/* Not all AMT entries has to be removed now that we support multi-groups */
#ifdef IBSS_RMC
	rmcp->af_data->subtype = ERMC_SUBTYPE_LEAD_SELECTED;
	rmcp->af_data->bcm_action = ERMC_ACT_STOP;
#else
	rmcp->af_data->subtype = ERMC_SUBTYPE_LEAD_CANCELED;
#endif // endif

	return FALSE;
}
#endif /* AP */

static void wlc_rmc_status(wlc_rmc_info_t * const rmcp,
	wl_relmcast_status_t *statusp)
{
	struct scb_iter scbiter;
	struct scb *scb = NULL;
	wl_relmcast_client_t *clientp = NULL;

	ERMC_TRACE;

	bzero(statusp, sizeof(*statusp));
	statusp->ver = WL_RMC_VER;

	if (rmcp->mc_process_fn) {
		FOREACHSCB(rmcp->wlc->scbstate, &scbiter, scb) {
			if ((SCB_ASSOCIATED(scb) && BSSCFG_AP(scb->bsscfg)) ||
				BSSCFG_IBSS(scb->bsscfg)) {
				clientp = &statusp->clients[statusp->num];
				clientp->rssi = wlc_scb_rssi(scb);
				bcopy(&scb->ea, &clientp->addr, ETHER_ADDR_LEN);

				if (scb == rmcp->mc_ackerscb)
					clientp->flag |= WL_RMC_FLAG_ACTIVEACKER;

				if (scb->flags3 & SCB3_RELMCAST)
					clientp->flag |= WL_RMC_FLAG_RELMCAST;

				if (scb->flags3 & SCB3_RELMCAST_NOACK)
					clientp->flag |= WL_RMC_FLAG_INBLACKLIST;

				statusp->num++;
			}
		} /* FOREACHSCB */
	}

	statusp->err = rmcp->err;
	statusp->actf_time = rmcp->mc_actf_period;
}

/*  Send control action frames to transmitter */

static void wlc_rmc_ackreq_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		wlc->rmc->err = BCME_RXFAIL;
		wlc->rmc->ermc_cnts->ackreq_err++;
		WL_ERROR(("ackreq was lost\n"));
	} else {
		wlc->rmc->err = BCME_OK;
	}
}

/* Function to transmit uni-cast action frames */
static int wlc_rmc_transmitaf(wlc_rmc_info_t* rmc, wl_action_frame_t *af,
	ratespec_t rate_override, pkcb_fn_t fn)
{
	wlc_info_t *wlc = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	struct ether_addr *bssid = NULL;
	uint8* pbody = NULL;
	wlc_pkttag_t *pkttag = NULL;
	void *pkt;

	ASSERT(rmc != NULL);
	ASSERT(af != NULL);

	wlc = rmc->wlc;
	bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, &rmc->self_mac);

	ASSERT(bsscfg != NULL);

	bssid = &bsscfg->BSSID;

	/* get allocation of action frame */
	if ((pkt = wlc_frame_get_action(wlc, FC_ACTION, &af->da,
	                                &bsscfg->cur_etheraddr,
	                                bssid, af->len, &pbody,
	                                DOT11_ACTION_CAT_VS)) == NULL) {
		return BCME_NOMEM;
	}

	pkttag = WLPKTTAG(pkt);
	pkttag->shared.packetid = af->packetId;
	WLPKTTAGBSSCFGSET(pkt, bsscfg->_idx);

	((rmc_notify_af_t*)af->data)->artmo = rmc->mc_artmo;
	/* copy action frame payload */
	bcopy(af->data, pbody, af->len);

	/* Need to set a proper scb in action frame transmission so that lower layer
	   functions can have a correct reference to scb and bsscfg. If scb is not
	   provided on wlc_queue_80211_frag(), it internally uses the default scb
	   which points to a wrong bsscfg.
	*/

	if (fn) {
		wlc_pcb_fn_register(wlc->pcb, fn, (void *) (uintptr)bsscfg->ID, pkt);
	}

	rmc->err = 0;

	/* put into queue and then transmit */
	if (!wlc_queue_80211_frag(wlc, pkt, wlc->active_queue,
	                          NULL, bsscfg, FALSE, NULL, rate_override)) {
		return BCME_ERROR;
	}

	/* WLF2_PCB1_AF callback is not needed because the action frame was not
	 * initiated from Host. More importantly, queueing up WLC_E_ACTION_FRAME_COMPLETE event
	 * which would be done in the callback would keep the device from going into sleep.
	 */

	return BCME_OK;
}

/* This function is used by the initiator to send entries to the transmitter */
/* when it is in infrastructure mode */

static uint8 wlc_rmc_send_unicast_action(wlc_rmc_info_t *rmcp, uint8 subtype)
{
	wlc_bsscfg_t *bsscfg = NULL;
	struct ether_addr *bssid = NULL;
	wlc_info_t *wlc = NULL;
	wl_action_frame_t *af = NULL;
	pkcb_fn_t fn = NULL;
	uint32 ret = BCME_ERROR;

	wlc = rmcp->wlc;
	if ((af = (wl_action_frame_t *)MALLOC(wlc->osh, ERMC_AF_SIZE)) == NULL) {
		ERMC_ERROR(("%s: malloc failed!\n", __FUNCTION__));
		return BCME_ERROR;
	}

	bzero(af, ERMC_AF_SIZE);

	bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, &rmcp->self_mac);

	ASSERT(bsscfg != NULL);

	bssid = &bsscfg->BSSID;

	if (bssid)
		bcopy(bssid, &af->da, ETHER_ADDR_LEN);

	af->packetId = rmcp->af_txcnt;

	af->len = sizeof(rmc_notify_af_t);

	if (!BSSCFG_IBSS(bsscfg)) {
		/* copy the token */
		switch (subtype)
		{
			case ERMC_SUBTYPE_DISABLE_RMC:
			case ERMC_SUBTYPE_ENABLE_RMC:
				rmcp->err = BCME_NOTREADY;
				bcopy(rmcp->af_data, &af->data[0], sizeof(rmc_notify_af_t));

				((rmc_notify_af_t*)af->data)->subtype = subtype;

				fn = wlc_rmc_ackreq_complete;

				break;

			default:
				break;
		}
	}

	if ((ret = wlc_rmc_transmitaf(rmcp, af,
	                             rmcp->af_txrate,
	                             fn)) == BCME_OK) {
		rmcp->af_txcnt++;
	} else {
		ERMC_DBG(("Inappropriate subtype \n"));
		rmcp->ermc_cnts->af_unicast_tx_err++;
	}

	MFREE(wlc->osh, af, ERMC_AF_SIZE);
	return ret;
}

/* wlc_rmc_doiovar is the  iovar processing method for rmc */
static int wlc_rmc_doiovar(void *ctx, const bcm_iovar_t *vi,
	uint32 actionid, const char *name, void *params, uint p_len, void *arg,
	int len, int val_size, struct wlc_if *wlcif)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)ctx;
	uint8 *ackreq_mode = NULL;
	wlc_info_t *wlc = NULL;
	wlc_bsscfg_t *cfg = NULL;
	bool bool_val;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *)arg;

	ERMC_TRACE;
	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	wlc = rmcp->wlc;

	/* update bsscfg w/provided interface context */
	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);

	if (cfg == NULL) {
		return BCME_ERROR;
	}

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* Do the actual parameter implementation */
	switch (actionid) {
		case IOV_GVAL(IOV_RELMCAST_ACKREQ):
			ackreq_mode =  (uint8*)arg;
			*ackreq_mode = rmcp->ermc_mode;
			break;

		case IOV_SVAL(IOV_RELMCAST_ACKREQ):
			{
				ackreq_mode = (uint8 *)arg;
				switch (*ackreq_mode) {

				case WL_RMC_MODE_RECEIVER:

					if (!BSSCFG_IBSS(cfg)) {
						if (rmcp->ermc_mode == WL_RMC_MODE_INITIATOR) {
							/*  tell transmiter to stop  */
							err = wlc_rmc_send_unicast_action(rmcp,
								ERMC_SUBTYPE_DISABLE_RMC);
						}
					} else {
						/* we are the transmitter, do local cleanup  */
						err = wl_ermc_xmitter_stop(rmcp);
					}
					rmcp->ermc_mode = WL_RMC_MODE_RECEIVER;
					break;

				case WL_RMC_MODE_INITIATOR:
					/* XXX inititator role is abandoned now.,
					 * enable when needed
					 */
					ERMC_DBG(("enable initiator STA -> remote xmiter\n"));
#ifdef RMC_INFRA_NOT_YET
						err = wlc_rmc_send_unicast_action(rmcp,
							ERMC_SUBTYPE_LEAD_SELECTED);
						rmcp->ermc_mode = WL_RMC_MODE_INITIATOR;
#endif // endif
					err = BCME_UNSUPPORTED;
					break;

				case WL_RMC_MODE_TRANSMITTER:

					if (rmcp->af_cnt2off) {
					/* rms is being turned off */
					/* need to wait before it can be turned on again */
						return BCME_BUSY;
					}

					wl_ermc_xmitter_start(rmcp);
					rmcp->ermc_mode = WL_RMC_MODE_TRANSMITTER;

					break;

				default:
					err = BCME_UNSUPPORTED;
					break;
				}
			}
			break;

		case IOV_GVAL(IOV_RELMCAST_TXACK):
			*ret_int_ptr = (int32)rmcp->mc_txack;
			break;

		case IOV_SVAL(IOV_RELMCAST_TXACK):
			rmcp->mc_txack = bool_val;
			break;

		case IOV_GVAL(IOV_RELMCAST_ACKMAC):
			wlc_rmc_get_trs(rmcp, arg);
			break;

		case IOV_SVAL(IOV_RELMCAST_ACKMAC):
			/* Consider setting a single entry first */
			ERMC_DBG(("RMC ackmac set \n"));
			wlc_rmc_ackmac(rmcp, arg);
			break;
		case IOV_GVAL(IOV_RELMCAST_AR):
			wlc_rmc_get_ar(rmcp, arg);
			break;

		case IOV_SVAL(IOV_RELMCAST_AR):
			ERMC_DBG(("RMC ar(active receiver) set \n"));
			err = wlc_rmc_set_ar(rmcp, arg);
			break;
		case IOV_GVAL(IOV_RELMCAST_TXRATE):
			*ret_int_ptr = RSPEC2RATE(rmcp->mc_txrate);
			break;

		case IOV_SVAL(IOV_RELMCAST_TXRATE):
			rmcp->mc_txrate = int_val;
			break;

		case IOV_GVAL(IOV_RELMCAST_ACKTMO):
			*ret_int_ptr = (int32)rmcp->mc_noacktmo;
			break;

		case IOV_SVAL(IOV_RELMCAST_ACKTMO):
			rmcp->mc_noacktmo = int_val;
			break;

		case IOV_GVAL(IOV_RELMCAST_CHKTMO):
			*ret_int_ptr = (int32)rmcp->mc_periodtmo;
			break;

		case IOV_SVAL(IOV_RELMCAST_CHKTMO):
			if (rmcp->mc_periodtmo != int_val) {
				if (int_val >= 0 && int_val <= WL_RMC_MAX_TIMEOUT) {
					wlc_rmc_chktimer_update(rmcp, int_val);
				} else {
					err = BCME_RANGE;
				}
			}
			break;

		case IOV_GVAL(IOV_RELMCAST_STATUS):
			if (len < sizeof(wl_relmcast_status_t))
				err = BCME_BUFTOOSHORT;
			else
				wlc_rmc_status(rmcp, (wl_relmcast_status_t *)arg);
			break;

		case IOV_GVAL(IOV_RELMCAST_ACTF_TIME):
			*ret_int_ptr = rmcp->mc_actf_period;
			break;

		case IOV_SVAL(IOV_RELMCAST_ACTF_TIME):
			/* don't need to transmit action frames; don't set period */
			if (BSSCFG_AP(wlc->cfg) ||
			    wl_rmc_is_p2p_go(wlc)) {
				err = BCME_UNSUPPORTED;
			}

			if (*(uint16*)arg >= WL_RMC_ACTF_TIME_MIN &&
				*(uint16*)arg <= WL_RMC_ACTF_TIME_MAX)
				rmcp->mc_actf_period = *(uint16*)arg;
			else
				err = BCME_RANGE;

			break;

		case IOV_GVAL(IOV_RELMCAST_RSSI_THRESH):
			*ret_int_ptr = rmcp->rmc_lowrssi;
			break;

		case IOV_SVAL(IOV_RELMCAST_RSSI_THRESH):
			if (int_val < 0) {
				rmcp->rmc_lowrssi = int_val;
			}
			else
				err = BCME_RANGE;
			break;

		case IOV_GVAL(IOV_RELMCAST_STATS):
			err = wlc_rmc_stats(rmcp, (wl_rmc_cnts_t *)arg);
			break;

		case IOV_SVAL(IOV_RELMCAST_STATS):
			/* Clear statistical counters */
			ERMC_DBG(("%s %d \n", __FUNCTION__, sizeof(*rmcp->ermc_cnts)));
			bzero(rmcp->ermc_cnts, sizeof(*rmcp->ermc_cnts));
			WLCNTSET(rmcp->ermc_cnts->version, WL_RMC_CNT_VERSION);
			WLCNTSET(rmcp->ermc_cnts->length, sizeof(wl_rmc_cnts_t));

			break;

		case IOV_GVAL(IOV_RELMCAST_RSSI_DELTA):
			/* for user, rssi delta is always a +ve value */
			*ret_int_ptr = -rmcp->rssi_delta;
			break;

		case IOV_SVAL(IOV_RELMCAST_RSSI_DELTA):
			if (int_val >= 0) {
				rmcp->rssi_delta = -int_val;
			}
			else
				err = BCME_RANGE;
			break;

		case IOV_GVAL(IOV_RELMCAST_VSIE):
			bcopy(rmcp->vendor_oui, ((wl_rmc_vsie_t*)arg)->oui, DOT11_OUI_LEN);
			((wl_rmc_vsie_t*)arg)->payload = rmcp->vendor_ie_data;
			break;

		case IOV_SVAL(IOV_RELMCAST_VSIE):
			bcopy(((wl_rmc_vsie_t*)arg)->oui, rmcp->vendor_oui, DOT11_OUI_LEN);
			rmcp->vendor_ie_data = ((wl_rmc_vsie_t*)arg)->payload;

			/* Now update beacon and probe resp */
			if (rmcp->wlc->cfg->up &&
			    (BSSCFG_AP(rmcp->wlc->cfg) || BSSCFG_IBSS(rmcp->wlc->cfg))) {
				/* update AP or IBSS beacons */
				wlc_bss_update_beacon(wlc, rmcp->wlc->cfg);
				/* update AP or IBSS probe responses */
				wlc_bss_update_probe_resp(wlc, rmcp->wlc->cfg, FALSE);
			}
			break;

		case IOV_GVAL(IOV_RELMCAST_ARTMO):
			*ret_int_ptr = (uint16)rmcp->mc_artmo;
			break;

		case IOV_SVAL(IOV_RELMCAST_ARTMO):
			if (*(uint16*)arg >= WL_RMC_ARTMO_MIN &&
				*(uint16*)arg <= WL_RMC_ARTMO_MAX) {
				rmcp->mc_artmo = *(uint16*)arg;
			} else {
				err = BCME_RANGE;
			}
			break;

		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

/* RMC module attached function  */
wlc_rmc_info_t * BCMATTACHFN(wlc_rmc_attach)(wlc_info_t *wlc)
{
	wlc_rmc_info_t * rmcp = NULL;
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint8 arr_idx;

	ERMC_TRACE;

	if ((rmcp = MALLOC(wlc->osh, sizeof(wlc_rmc_info_t))) == NULL) {
		ERMC_ERROR(("%s: malloc failed!\n", __FUNCTION__));
		return NULL;
	}

	bzero(rmcp, sizeof(*rmcp));
	rmcp->wlc = wlc;
	wl_rmc_assign_signature(rmcp, (uint32)WL_RMC_SIGNATURE);
	rmcp->mc_noacktmo = WL_RMC_NOACK_TIMEOUT;
	rmcp->mc_periodtmo = WL_RMC_DEFAULT_TIMEOUT;

	if (!(rmcp->mc_noack_timer = wl_init_timer(wlc->wl,
	                                           wlc_rmc_noack_timer,
	                                           rmcp, "rmc"))) {
		WL_ERROR(("wl%d: rmc timer failed\n", wlc->pub->unit));
		goto fail;
	}

	if (!(rmcp->mc_check_timer = wl_init_timer(wlc->wl,
		wlc_rmc_check_timer, rmcp, "relmchk"))) {
		WL_ERROR(("wl%d: rmc check timer failed\n", wlc->pub->unit));
		goto fail;
	}

	/* periodical timer multicast service channel */
	rmcp->aftx_timer_started = FALSE;
	if (!(rmcp->mc_aftx_timer = wl_init_timer(wlc->wl,
	                                          wlc_rmc_aftx_timer_cbfn,
	                                          rmcp, "relmchk"))) {
		WL_ERROR(("wl%d: rmc act-frame-tx timer failed\n",
			wlc->pub->unit));
		goto fail;
	}

	/* periodical timer to age out entries */
	if (!(rmcp->mc_age_timer = wl_init_timer(wlc->wl,
		wlc_rmc_age_timer, rmcp, "relmchk"))) {
		WL_ERROR(("wl%d: rmc age timer failed\n", wlc->pub->unit));
		goto fail;
	}

	if (wlc_module_register(wlc->pub,
	                        wlc_rmc_iovars,
	                        "rmc",
	                         (void *)rmcp,
	                         wlc_rmc_doiovar,
	                         NULL, NULL, NULL) != BCME_OK) {

		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n", wlc->pub->unit,
			__FUNCTION__));

		goto fail;

	}

	/* register IE mgmt callback */
	/* calc/build */
#ifdef STA
	/* assocreq/reassocreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_BRCM_RMC,
	                                wlc_rmc_calc_rmc_ie_len, wlc_rmc_write_rmc_ie,
	                                rmcp) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ie_mgmt_vs_add_build_fn failed, rmc in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */
#ifdef AP
	/* bcn */
	if (wlc_iem_vs_add_build_fn(wlc->iemi, FC_BEACON, WLC_IEM_VS_IE_PRIO_BRCM_RMC,
	                            wlc_rmc_calc_rmc_ie_len, wlc_rmc_write_rmc_ie,
	                            rmcp) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ie_mgmt_vs_add_build_fn failed, rmc in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
	/* parse */
#ifdef AP
	/* assocreq/reassocreq */
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_BRCM_RMC,
	                                wlc_rmc_parse_rmc_ie, rmcp) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ie_mgmt_vs_add_parse_fn failed, rmc in assocreq\n",
		           wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
#ifdef STA
	/* bcn */
	if (wlc_iem_vs_add_parse_fn(wlc->iemi, FC_BEACON, WLC_IEM_VS_IE_PRIO_BRCM_RMC,
	                            wlc_rmc_parse_rmc_ie, rmcp) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ie_mgmt_vs_add_parse_fn failed, rmc in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */

	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_RMC, wlc_rmc_txcomplete) != BCME_OK)
	{
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto fail;
	}

	/* Add scb association state callback to the notification list */
	if (wlc_scb_state_upd_register(wlc, (bcm_notif_client_callback)wlc_rmc_check_cb,
		(bcm_notif_client_data)rmcp) != BCME_OK) {
		WL_ERROR(("wl%d: %s: unable to register rmc state callback\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->_rmc_support = TRUE;

	/* alloc mcast_ctl act frame */
	rmcp->ctl_af = MALLOC(wlc->osh, ERMC_AF_SIZE);

	if (!rmcp->ctl_af) {
		WL_ERROR(("Failed to alloc ctl_act frame structure \n"));
		goto fail;
	}

	/* alloc memory for counters */
	rmcp->ermc_cnts = MALLOC(wlc->osh, sizeof(wl_rmc_cnts_t));

	if (!rmcp->ermc_cnts) {
		WL_ERROR(("Failed to alloc memory for counters\n"));
		goto fail;
	}

	/* point to the array of pointers that point to wlc_rmc_trans_info_t */
	rmcp->trs = ptr_tr_array;

	for (arr_idx = 0; arr_idx < ERMC_HASH_TBL_SIZE; arr_idx++) {
		rmcp->trs[arr_idx] = NULL;
	}

	bzero(rmcp->ermc_cnts, sizeof(*rmcp->ermc_cnts));
	rmcp->ermc_cnts->version = WL_RMC_CNT_VERSION;
	rmcp->ermc_cnts->length = sizeof(wl_rmc_cnts_t);

	/* direct ptr to notif af payload */
	rmcp->af_data = (rmc_notify_af_t *)rmcp->ctl_af->data;
	rmcp->ctl_af->len = sizeof(rmc_notify_af_t);

	rmcp->af_txrate = WL_ERMC_DEFAULT_AFTX_RATE;
	rmcp->mc_txrate = WL_ERMC_DEFAULT_DFTX_RATE;
	rmcp->af_txcnt = 0;
	rmcp->af_rxcnt = 0;
	rmcp->af_cnt2off = 0;
	rmcp->ermc_mode = WL_RMC_MODE_RECEIVER;
	rmcp->rssi_delta = ERMC_ACKER_RSSI_DELTA;
	rmcp->err = 0;
	rmcp->tr_mcbmp = 0;
	rmcp->mc_actf_period = WL_ERMC_AFTX_PERIOD;
	rmcp->rmc_lowrssi = WL_RMC_RSSI_THRESHOLD;
	rmcp->mc_artmo = WL_RMC_DEFAULT_ARTMO;
#ifdef IBSS_RMC
	bcopy(BRCM_PROP_OUI, rmcp->vendor_oui, DOT11_OUI_LEN);
	rmcp->vendor_ie_data = 0;
#endif // endif
	rmcp->auto_ar_select = TRUE;

	/* intitalize mcast act frame data   */
	bcopy(&ctlaf_s, rmcp->af_data, sizeof(rmc_notify_af_t));
	/* init default nitif af mcast mac  */
	bcopy(&df_notify_af_mcastmac, &rmcp->mc_ctlmac, ETHER_ADDR_LEN);

	/* clear hardcoded debug mac addresseses */
	bzero(&rmcp->af_data->mctable[0], ETHER_ADDR_LEN * ERMC_NUM_OF_MC_STREAMS);

	/* read our own mac  */
	bcopy(&wlc->pub->cur_etheraddr, &rmcp->self_mac, ETHER_ADDR_LEN);

	bcopy(&rmcp->af_data->mctable[0], &rmcp->mc_ackmac, sizeof(rmcp->mc_ackmac));

	wl_add_timer(rmcp->wlc->wl, rmcp->mc_age_timer,
		WL_ERMC_AFTX_PERIOD * 2, TRUE);

	return rmcp;

fail:
	if (rmcp->ctl_af) {
		MFREE(wlc->osh, rmcp->ctl_af, ERMC_AF_SIZE);
		rmcp->ctl_af = NULL;
	}

	if (rmcp->ermc_cnts) {
		MFREE(wlc->osh, rmcp->ermc_cnts, sizeof(*rmcp->ermc_cnts));
		rmcp->ermc_cnts = NULL;
	}

	if (rmcp->gtbl) {
		MFREE(wlc->osh, rmcp->gtbl, sizeof(*rmcp->gtbl));
		rmcp->gtbl = NULL;
	}

	if (rmcp->mc_noack_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_noack_timer);
		rmcp->mc_noack_timer = NULL;
	}

	if (rmcp->mc_check_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_check_timer);
		rmcp->mc_check_timer = NULL;
	}

	if (rmcp->mc_age_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_age_timer);
		rmcp->mc_age_timer = NULL;
	}

	MFREE(wlc->osh, rmcp, sizeof(wlc_rmc_info_t));
	rmcp = NULL;

	return NULL;
}

/* RMC module detach function */
void BCMATTACHFN(wlc_rmc_detach)(wlc_rmc_info_t * rmcp)
{
	wlc_info_t *wlc = NULL;

	ERMC_TRACE;

	if (rmcp == NULL)
		return;

	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);
	wl_rmc_assign_signature(rmcp, 0);

	wlc = (wlc_info_t*) rmcp->wlc;
	if (rmcp->mc_noack_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_noack_timer);
		rmcp->mc_noack_timer = NULL;
	}

	if (rmcp->mc_check_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_check_timer);
		rmcp->mc_check_timer = NULL;
	}

	if (rmcp->mc_aftx_timer) {
		wl_free_timer(wlc->wl, rmcp->mc_aftx_timer);
		rmcp->mc_aftx_timer = NULL;
	}

	wlc_module_unregister(wlc->pub, "rmc", rmcp);

	MFREE(wlc->osh, rmcp->ctl_af, ERMC_AF_SIZE);
	rmcp->ctl_af = NULL;
	MFREE(wlc->osh, rmcp->ermc_cnts, sizeof(*rmcp->ermc_cnts));
	rmcp->ermc_cnts = NULL;
	/* Need to free trs list */
	wlc_rmc_free_trs(rmcp);
	MFREE(wlc->osh, rmcp, sizeof(*rmcp));
	rmcp = NULL;
}

int wlc_rmc_process(wlc_rmc_info_t * const rmcp, uint16 type,
	void *pkt, struct dot11_header *const h, uint16 * const mclp,
	ratespec_t *rspecp)
{
	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	/* Reliable multicast frame needs ACK and set tx rate */
	if (rmcp->mc_process_fn) {
		return (*rmcp->mc_process_fn)(rmcp, type, pkt, h, mclp, rspecp);
	}

	return FALSE;
}

/*   RCVR gets the beacon   */
static void wlc_relmcast_beacon_process(wlc_rmc_info_t * rmcp, wlc_bsscfg_t *cfg,
	uint8 *tag_params, int tag_params_len)
{
	ASSERT(rmcp != NULL);
	ASSERT(cfg != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	/* Beacon is processed elsewhere in IBSS RMC. */
	if (BSSCFG_AP(cfg) || BSSCFG_IBSS(cfg) ||
		wl_rmc_is_p2p_go(rmcp->wlc)) {
		ERMC_ERROR(("rcvd beacon in ap mode\n"));
		return;
	}

}

/* Invoked when a beacon is received in IBSS network to see whether RMC
 * is supported and to trigger RSSI update and check for rmc acker
 */
#ifdef AP
static void wlc_relmcast_assocreq_process(wlc_rmc_info_t * rmcp, struct scb *scb,
	uint8 *tag_params, int tag_params_len)
{
	relmcast_brcm_prop_ie_t *rmc_brcm_prop_ie = NULL;
	uint8 type;
	int type_len;
	char *voui;

	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	if (rmcp->mc_process_fn) {
#ifdef IBSS_RMC
		voui = RMC_PROP_OUI;
		type = 0;
		type_len = 0;
#else
		voui = BRCM_PROP_OUI;
		type = RELMCAST_BRCM_PROP_IE_TYPE;
		type_len = sizeof(type);
#endif // endif
		rmc_brcm_prop_ie = (relmcast_brcm_prop_ie_t *)bcm_find_vendor_ie(tag_params,
		                                                            tag_params_len,
		                                                            voui,
		                                                            &type, type_len);

		if (rmc_brcm_prop_ie) {
			/* reliable multicast enabled in the client */
			scb->flags3 &= ~SCB3_RELMCAST_NOACK;
			scb->flags3 |= SCB3_RELMCAST;

			wlc_scb_rssi_update_enable(scb, TRUE, RSSI_UPDATE_FOR_WLC);

			if (BSSCFG_IBSS(scb->bsscfg))
				wlc_rmc_check(rmcp, scb);

		} else {
			ERMC_DBG(("client doesn't have BRCM rmc IE type:%x\n", type));
			scb->flags3 &= ~(SCB3_RELMCAST|SCB3_RELMCAST_NOACK);
		}
	}
}

/* This function is called periodically to check for new acker */
static void wlc_rmc_check(wlc_rmc_info_t * rmcp, struct scb *scb)
{
	int arrssi = 1000;
	ASSERT(rmcp != NULL);

	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	if (rmcp && rmcp->mc_process_fn && scb) {
		if (SCB_ASSOCIATED(scb) || BSSCFG_IBSS(scb->bsscfg)) {
			if (scb != rmcp->mc_ackerscb && (scb->flags3 & SCB3_RELMCAST) &&
				!(scb->flags3 & SCB3_RELMCAST_NOACK) &&
				wlc_scb_rssi(scb) >= rmcp->rmc_lowrssi) {
				/* New station authenticated */
				if (rmcp->mc_ackerscb)
					arrssi = wlc_scb_rssi(rmcp->mc_ackerscb);

				/* find new acker when - current AR's rssi is < rssi threshold OR
				 *  rssi of a STA is more than rssi delta OR when AR is not yet
				 *  selected/unavailable
				 */
				if (!rmcp->mc_ackerscb ||
					(wlc_scb_rssi(scb) < arrssi + rmcp->rssi_delta) ||
					(arrssi < rmcp->rmc_lowrssi) ||
					ETHER_ISNULLADDR(&rmcp->mc_acker))  {
					if (rmcp->auto_ar_select == TRUE) {
						wlc_rmc_newacker(rmcp, scb);
					}
				}
			}
		} else {
			if (scb == rmcp->mc_ackerscb) {
				wlc_rmc_find_acker(rmcp);
			}
		}
	}

	return;
}

/* rmc check call back function */
static void wlc_rmc_check_cb(wlc_rmc_info_t * rmcp, scb_state_upd_data_t *data)
{
	ASSERT(data != NULL);
	ASSERT(rmcp != NULL);
	struct scb *scb = data->scb;
	ASSERT(scb != NULL);

	wlc_rmc_check(rmcp, scb);
}

/* Update the RSSI values by processing the RMC control frames */
void wlc_rmc_mgmtctl_rssi_update(wlc_rmc_info_t * rmcp,
	wlc_d11rxhdr_t *wrxh, struct scb *scb, bool datapkt)
{
	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);
	ERMC_TRACE;

	if (rmcp && rmcp->mc_process_fn && scb &&
		(scb->flags3 & SCB3_RELMCAST)) {
		wlc_scb_rssi_update_enable(scb, TRUE, RSSI_UPDATE_FOR_WLC);

		/* for data frames, rssi is already updated in wlc_recvdata,
		 * do not update it again
		 */
		if (datapkt) {
			scb->used = rmcp->wlc->pub->now;
		} else if (scb->rssi_window) {
			scb->rssi_window[scb->rssi_index] = wlc_lq_rssi_pktrxh_cal(rmcp->wlc,
			                                                           wrxh);
			scb->rssi_index = MODINC_POW2(scb->rssi_index, MA_WINDOW_SZ);
		}
		if (scb->flags3 & SCB3_RELMCAST_NOACK) {
			/* remove a station from black list, when a data
			 * or null frame is received.
			 */
			scb->flags3 &= ~SCB3_RELMCAST_NOACK;
			wlc_rmc_check(rmcp, scb);
		}
	}
}

/* Rmc function to make sure that ack to multi-cast packet are processed properly */
static void wlc_rmc_dotxstatus(wlc_rmc_info_t* rmcp, bool acked)
{
	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);
	ERMC_TRACE;

	if (!acked) {

		if (!rmcp->mc_noack_timer_started && rmcp->mc_process_fn &&
			rmcp->mc_ackerscb) {
			wl_add_timer(rmcp->wlc->wl,
			             rmcp->mc_noack_timer,
			             rmcp->mc_noacktmo, FALSE);

			rmcp->mc_noack_timer_started = TRUE;
			rmcp->mc_noack_txnum = 0;
		}
	} else {

		if (rmcp->mc_noack_timer_started) {
			wl_del_timer(rmcp->wlc->wl, rmcp->mc_noack_timer);
			rmcp->mc_noack_timer_started = FALSE;
		}
	}
}

/* packet tx completion callback */
static void wlc_rmc_txcomplete(wlc_info_t *wlc, void *pkt, uint txs)
{
	ASSERT(wlc != NULL);
	wlc_rmc_info_t *rmcp = wlc->rmc;
	ASSERT(rmcp != NULL);
	wlc_rmc_dotxstatus(rmcp, (txs & TX_STATUS_ACK_RCV));
}
#endif /* AP */

/* wlc_rmc_verify_dup_pkt filters out duplicate multi-cast packets */
uint8 wlc_rmc_verify_dup_pkt(wlc_info_t * wlc, wlc_frminfo_t  *f)
{
#if defined(ERMC_DEBUG) || defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	/* The found_flag indicates as follows: */
	/* For selective ta  support: if the found_flag is set, it indicates that the source mac */
	/* of the packet matches with an AMT A2 entry that is programmed on a active receiver in */
	/* the IBSS network. Hence, we know we have a valid RMC transmitter packet. We can       */
	/* proceed to processed to see whether duplicate filtering is needed.                    */

	bool found_flag = FALSE;
	uint16 prev_seq = 0;
	wlc_rmc_trans_info_t *tr = NULL;

	wlc_rmc_info_t *rmcp = wlc->rmc;

	ASSERT(wlc != NULL);
	ASSERT(f != NULL);
	rmcp->ermc_cnts->rmc_rx_frames_mac++;

	for (tr = rmcp->trs[HASH_MAC(&f->h->a2)]; ((tr != NULL) && (!found_flag)); tr = tr->next) {
		/* check to find out whether the sa of packet matches with one of the  */
		/* transmitter mac address                                             */
		if (!bcmp((const void *)&f->h->a2, &tr->addr, sizeof(tr->addr))) {

			/* The packet is from one of the RMC transmitter */
			prev_seq = tr->seq;
			tr->seq = f->seq;
			found_flag = TRUE;
		}
	}

#ifdef ERMC_DEBUG
	if (found_flag == FALSE) {
		ERMC_DBG(("\nError af_sender_mac %s   packet src %s ",
			bcm_ether_ntoa(&rmcp->af_sender_mac, eabuf),
			bcm_ether_ntoa(&f->h->a2, eabuf)));
		ERMC_DBG((" dest  %s ",
			bcm_ether_ntoa(&f->h->a1, eabuf)));
	}
#endif /* ERMC_DEBUG */

	if ((found_flag == TRUE) && (f->fc & FC_RETRY) &&
		(f->seq == prev_seq)) {

		rmcp->ermc_cnts->dupcnt++;

		WL_TRACE(("wl%d: %s: discarding duplicate MPDU %04x "
		          "received from %s dupcnt %d \n",
		           wlc->pub->unit, __FUNCTION__, f->seq,
		           bcm_ether_ntoa(&(f->h->a2), eabuf), rmcp->ermc_cnts->dupcnt));

		WLCNTINCR(wlc->pub->_cnt->rxfilter);
		/* Discard duplicates - return TRUE */
		return TRUE;
	}

	/* Do not discard - return FALSE */
	return FALSE;
}

/*
 * if there is already an entry in amt, return true
 * and the *amt_idx is the index to the AMT entry
 * if the entry with ea is not found, return BCME_NOTFOUND
 * if the entry is not found, and there is no free slot,
 * return BCME_ERROR
 * and the first free AMT slot is in *amt_idx
 */
static int8 wlc_rmc_get_amt_index(wlc_info_t *wlc, const struct ether_addr *ea,
	uint8* amt_idx, uint16* attr)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	uint8 idx, free_idx = 0;
	struct ether_addr tmp;
	bool free_idx_found = FALSE;

	BCM_REFERENCE(eabuf);
	ERMC_DBG(("%s  ea: %s\n", __FUNCTION__, bcm_ether_ntoa(ea, eabuf)));

	ASSERT(ea != NULL);
	ASSERT(amt_idx != NULL);
	ASSERT(attr != NULL);

	if (wlc == NULL || ea == NULL || attr == NULL || amt_idx == NULL) {
		return BCME_ERROR;
	}

	*attr = 0;
	*amt_idx = ERMC_INVALID_AMT_IDX;

	if (!wlc->clk)
		return BCME_NOCLK;

	/* check if there is already an entry  */
	for (idx = 0; idx < (uint8)wlc->pub->max_addrma_idx; idx++) {
		wlc_bmac_read_amt(wlc->hw, idx, &tmp, attr);
		if (ETHER_ISNULLADDR(&tmp) || (*attr == 0)) {
			if (!free_idx_found) {
				free_idx_found = TRUE;
				free_idx = idx;
			}
			continue;
		}
		if ((memcmp((void *)&tmp, (const void *)ea, sizeof(struct ether_addr)) == 0)) {
			ERMC_DBG(("%s,Existng AMT entry at %d\n", __FUNCTION__, idx));
			*amt_idx = idx;
			return BCME_OK;
		}
	}
	if (free_idx_found == TRUE) {
		*amt_idx = free_idx;
		return BCME_NOTFOUND;
	} else {
		return BCME_ERROR;
	}
}

/*
 * Set the attr to valid and ta and set the device to ack if the tr flag is RMC_ACTIVE_RECEIVER
 * if the packet is from the ta mac
 */
static int
wlc_rmc_amt_set(wlc_info_t *wlc, wlc_rmc_trans_info_t *tr, uint16 attr)
{
	char eabuf[ETHER_ADDR_STR_LEN];

	ASSERT(tr != NULL);
	ASSERT(wlc != NULL);

	if (tr == NULL || wlc == NULL) {
		return BCME_ERROR;
	}

	BCM_REFERENCE(eabuf);
	ERMC_DBG(("%s  ea: %s\n", __FUNCTION__, bcm_ether_ntoa(&tr->addr, eabuf)));

	if ((attr & ((AMT_ATTR_VALID) | (AMT_ATTR_A2)))
		== ((AMT_ATTR_VALID) | (AMT_ATTR_A2))) {
		ERMC_DBG(("%s, Existing amt entry %d\n", __FUNCTION__, tr->amt_idx));
	} else if (!(attr & AMT_ATTR_VALID)) {
		attr = (AMT_ATTR_VALID | AMT_ATTR_A2);
		wlc_bmac_write_amt(wlc->hw, tr->amt_idx, &tr->addr, attr);
	} else if (!(attr & AMT_ATTR_A2)) {
		attr |= AMT_ATTR_A2;
		wlc_bmac_write_amt(wlc->hw, tr->amt_idx, &tr->addr, attr);
	}

	/* Set bit 11 to indicate TA should ACK */
	tr->flag = wlc_read_amtinfo_by_idx(wlc, tr->amt_idx);

	ERMC_DBG(("Reading existing val %04x  i %d \n", tr->flag, tr->amt_idx));
	if (!(tr->flag & ERMC_SHMEM_ACK_BIT)) {
		tr->flag |= ERMC_SHMEM_ACK_BIT;
		wlc_write_amtinfo_by_idx(wlc, tr->amt_idx, tr->flag);
	}

	return BCME_OK;
}

/*
 * create or write a new entry
 */
static int
wlc_rmc_amt_insert_entry(wlc_info_t *wlc, wlc_rmc_trans_info_t *tr_info)
{
	uint16 attr;
#ifdef ERMC_DEBUG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	ASSERT(tr_info != NULL);
	ASSERT(wlc != NULL);
	ASSERT(&tr_info->addr != NULL);

	attr = (AMT_ATTR_VALID | AMT_ATTR_A2);
	wlc_bmac_write_amt(wlc->hw, tr_info->amt_idx, &tr_info->addr, attr);
	ERMC_DBG(("\n%s, New amt entry %d\n", __FUNCTION__, tr_info->amt_idx));
#ifdef ERMC_DEBUG
	ERMC_ERROR(("New AMT A2 %s  attr %04x\n",
		bcm_ether_ntoa(&tr_info->addr, eabuf), attr));
#endif // endif

	/* Set bit 11 to indicate TA should ACK */
	tr_info->flag = wlc_read_amtinfo_by_idx(wlc, tr_info->amt_idx);
	tr_info->flag |= ERMC_SHMEM_ACK_BIT;
	wlc_write_amtinfo_by_idx(wlc, tr_info->amt_idx, tr_info->flag);

	return BCME_OK;
}

/*
 * insert tr in front of the rmc maintained list
 */
static inline uint8 wlc_rmc_insert_tr(wlc_rmc_trans_info_t **head_tr, wlc_rmc_trans_info_t* tr_info)
{
	ASSERT(tr_info != NULL);
	ASSERT(head_tr != NULL);
	tr_info->next = head_tr[HASH_MAC(&tr_info->addr)];
	head_tr[HASH_MAC(&tr_info->addr)] = tr_info;
	return TRUE;
}

/*
 * free all the trs structure that is MALLOC before exiting the module
 */
static void wlc_rmc_free_trs(wlc_rmc_info_t * const rmcp)
{
	ASSERT(rmcp != NULL);
	uint8 tr_idx = 0;
	wlc_rmc_trans_info_t* del_tr = NULL;
	for (tr_idx = 0; tr_idx < ERMC_HASH_TBL_SIZE; tr_idx++) {
		del_tr = rmcp->trs[tr_idx];
		while (del_tr) {
			wlc_rmc_trans_info_t* next_tr = del_tr->next;
			MFREE(rmcp->wlc->osh, del_tr, sizeof(*del_tr));
			del_tr = next_tr;
		}
	}
}

/*
 * copy all valid trs structure into memory location arg
 */
static void wlc_rmc_get_trs(wlc_rmc_info_t * const rmcp, wl_rmc_trans_in_network_t* tr_net)
{
	uint8 tr_idx = 0;
	uint8 tr_cnt = 0;
	wlc_rmc_trans_info_t* tr_info = NULL;
	ASSERT(rmcp != NULL);
	ASSERT(tr_net != NULL);

	tr_net->ver = WL_RMC_VER;
	for (tr_idx = 0; tr_idx < ERMC_HASH_TBL_SIZE; tr_idx++) {
		tr_info = rmcp->trs[tr_idx];
		while (tr_info) {
			wlc_rmc_trans_info_t* next_tr = tr_info->next;
			tr_net->trs[tr_cnt].tr_mac = tr_info->addr;
			tr_net->trs[tr_cnt].ar_mac = tr_info->ar_mac;
			tr_net->trs[tr_cnt].artmo = tr_info->artmo;
			tr_net->trs[tr_cnt].amt_idx = tr_info->amt_idx;
			tr_net->trs[tr_cnt].flag = tr_info->flag;
			tr_cnt++;
			tr_info = next_tr;
		}
	}
	tr_net->num_tr = tr_cnt;
}

/*
 * if there is already an entry in amt, just return the index
 * otherwise write a new AMT entry
 */
static uint8 wlc_rmc_amt_del_ta(wlc_info_t *wlc, wlc_rmc_trans_info_t *del_tr)
{
	uint16 attr;
	char eabuf[ETHER_ADDR_STR_LEN];
	struct ether_addr tmp;

	ASSERT(wlc != NULL);
	ASSERT(del_tr != NULL);
	BCM_REFERENCE(eabuf);
	ERMC_DBG(("%s  addr: %s\n", __FUNCTION__, bcm_ether_ntoa(&del_tr->addr, eabuf)));

	/* check if the entry match */
	wlc_bmac_read_amt(wlc->hw, del_tr->amt_idx, &tmp, &attr);
	if ((memcmp((void *)&tmp, (const void *)&del_tr->addr, sizeof(struct ether_addr)) == 0)) {

		ERMC_DBG(("\nwlc_rmc_amt_del_ta: Deleting idx %d  attr %04x\n",
		         del_tr->amt_idx, attr));
		if ((attr & ((AMT_ATTR_VALID) | (AMT_ATTR_A2))) ==
		    (attr | ((AMT_ATTR_VALID) | (AMT_ATTR_A2)))) {

			wlc_bmac_write_amt(wlc->hw, del_tr->amt_idx, &ether_null, 0);
			ERMC_DBG(("Deleted amt_idx %d \n", del_tr->amt_idx));
			del_tr->amt_idx = ERMC_INVALID_AMT_IDX;
			wlc->rmc->ermc_cnts->mc_ar_role_deleted++;
		} else {
		/* As long as it is a a2 entry with the transmitter ta, it should be deleted */
			attr = attr & ~AMT_ATTR_A2;
			wlc_bmac_write_amt(wlc->hw, del_tr->amt_idx, &del_tr->addr, attr);

			/* Clear bit 11 to indicate TA should not ACK */
			del_tr->flag = wlc_read_amtinfo_by_idx(wlc, del_tr->amt_idx);
			del_tr->flag &= ~ERMC_SHMEM_ACK_BIT;
			wlc_write_amtinfo_by_idx(wlc, del_tr->amt_idx, del_tr->flag);

			ERMC_DBG(("\nModifying No Ack/Not A2 amt_idx %d flag %04x \n",
				del_tr->amt_idx, del_tr->flag));
			wlc->rmc->ermc_cnts->mc_ar_role_deleted++;
		}
		del_tr->flag &= ~ERMC_SHMEM_ACK_BIT;  /* Make sure ack bit is not set */
	} else {
		/* entry does not match */
		ERMC_DBG(("\n%s ERROR  amt_idx %d mis-match ", __FUNCTION__, del_tr->amt_idx));
		del_tr->amt_idx = ERMC_INVALID_AMT_IDX;
		del_tr->flag &= ~ERMC_SHMEM_ACK_BIT;
		wlc->rmc->ermc_cnts->mc_not_exist_in_amt++;
		return ERMC_MC_NOT_EXIST_IN_AMT;
	}
	return ERMC_MC_PROGRAMMED;
}

/* wlc_rmc_del_ta_entry delete a ta a2 entry and update the amt cache */
static uint8 wlc_rmc_del_ta_entry(wlc_rmc_info_t *rmcp, struct ether_addr *tr_mac)
{
	wlc_rmc_trans_info_t *tr_info;
	wlc_rmc_trans_info_t *prev_tr = NULL;
	uint8 ret = ERMC_MC_PROGRAMMED;

	ASSERT(rmcp != NULL);
	ASSERT(tr_mac != NULL);
	/* First check to see whether it exist in the rmcp maintained tr list */
	for (tr_info = rmcp->trs[HASH_MAC(tr_mac)];  tr_info != NULL;
		tr_info = tr_info->next) {

		ERMC_DBG(("tr_info %08x  next %08x \n", (int)tr_info, (int)(tr_info->next)));
		/* First check rmcp maintained trs list to see whether it is already present */
		if (!(bcmp(tr_mac, &tr_info->addr, sizeof(*tr_mac))))  {
			/* found - delete or modify and update AMT it */
			if (tr_info->amt_idx != ERMC_INVALID_AMT_IDX) {
				ERMC_DBG(("amt del idx %d \n", tr_info->amt_idx));
				ret = wlc_rmc_amt_del_ta(rmcp->wlc, tr_info);
			}

			/* delete trinfo in the rmc maintained list if */
			/* the subtype is ERMC_SUBTYPE_LEAD_CANCELED if it gets here */
			rmcp->tr_cnt--;
			ERMC_DBG(("Free tr from list %08x  \n", (int)tr_info));
			if (tr_info == rmcp->trs[HASH_MAC(tr_mac)]) {
#ifdef ERMC_DEBUG
				int mac_idx;
				mac_idx = HASH_MAC(tr_mac);
				ERMC_DBG(("mac_idx %d amt_idx %d \n", mac_idx, tr_info->amt_idx));
				ERMC_DBG(("tr_info %08x \n", (int)tr_info));
#endif // endif
				/* special case head of list */
				rmcp->trs[HASH_MAC(tr_mac)] = tr_info->next;
				MFREE(rmcp->wlc->osh, tr_info, sizeof(*tr_info));
			} else {
				prev_tr->next = tr_info->next;
				MFREE(rmcp->wlc->osh, tr_info, sizeof(*tr_info));
			}
			break;
		}
		prev_tr = tr_info;
	}
	return ret;
}

/* wlc_rmc_add_ta - if ar_flag is set, add a TA entry and update the AMT cache copy */
/* make sure that the AMT is programmed with this TA entry */
/* if ar_flag is not set, add TA into the AMT cache copy only, and make sure     */
/* that the AMT is not programmed with this entry.                               */
static uint8 wlc_rmc_add_ta(wlc_rmc_info_t *rmcp, struct ether_addr *tr_mac,
	rmc_notify_af_t* afdata, uint8 ar_flag)
{
	wlc_rmc_trans_info_t *new_tr = NULL;
	uint8 tr_exist = FALSE;
	uint16 attr = 0;
	uint8 amt_idx = 0;
	int8 ret;
	wlc_rmc_trans_info_t *tr_info;

	ASSERT(tr_mac != NULL);
	ASSERT(afdata != NULL);
	ASSERT(rmcp != NULL);
	tr_info	= (wlc_rmc_trans_info_t *)rmcp->trs[HASH_MAC(tr_mac)];
#ifdef ERMC_DEBUG
	int mac_idx;
	mac_idx = HASH_MAC(tr_mac);
	ERMC_DBG(("idx %d amt_idx %d \n", mac_idx, tr_info->amt_idx));
	ERMC_DBG(("tr_info %08x \n", (int)tr_info));
#endif // endif
	/* First check to see whether it exist in the AMT */
	for (tr_info = rmcp->trs[HASH_MAC(tr_mac)]; ((tr_info != NULL) && (new_tr == NULL));
		tr_info = tr_info->next) {

		/* First check rmcp maintained trs list to see whether it is already present */
		if (!(bcmp(tr_mac, &tr_info->addr, sizeof(*tr_mac))))  {

			tr_info->time_val = OSL_SYSUPTIME();
			tr_info->artmo = afdata->artmo;

			/* it is already in cache copy of AMT, and enabled , do  not insert */
			/* Next check whether it is acking */
			if (ar_flag && (tr_info->flag & ERMC_SHMEM_ACK_BIT))  {
				/* don't need to do anything, already in the list and programmed */
				rmcp->ermc_cnts->mc_exist_in_amt++;
				ERMC_DBG(("\nIn rmcp list - ack 1"));
				return ERMC_MC_EXIST_IN_AMT;
			} else if (!ar_flag && !(tr_info->flag & ERMC_SHMEM_ACK_BIT)) {
				/* Not acking - no need to do anything since it should not */
				/* have an AMT entry either - it is only in rmcp tr list */
				ERMC_DBG(("\nIn rmcp list - ACK 0 "));
				return ERMC_MC_EXISTING_TR;
			} else {
			/* Program AMT if ar_flag is set */
			/* If ar_flag is not set, check to make sure TA  is not in AMT  */
				new_tr = tr_info;
				tr_exist = TRUE;
				ERMC_DBG(("\nIn rmcp list ar_flag %d ", ar_flag));
			}
		}
	}

	if ((rmcp->tr_cnt >= WL_RMC_MAX_NUM_TRS) && (tr_exist == FALSE)) {
		/* Only support up to WL_RMC_MAX_NUM_TRS  */
		ERMC_DBG(("%s: Exceed max num transmitters:%d\n", __FUNCTION__, rmcp->tr_cnt));
		rmcp->ermc_cnts->mc_no_amt_slot++;
		return ERMC_MC_NO_AMT_SLOT;
	}

	/* No AMT entry with the tr mac is found in list maintained trs list */
	/* Create this new tr */

	if (tr_exist == FALSE) {
		if ((new_tr = (wlc_rmc_trans_info_t *)MALLOC(rmcp->wlc->osh,
			sizeof(wlc_rmc_trans_info_t))) == NULL) {
			ERMC_ERROR(("%s: malloc failed!\n", __FUNCTION__));
			return BCME_ERROR;
		} else {
			bzero(new_tr, sizeof(*new_tr));
			bcopy(tr_mac, &new_tr->addr, sizeof(*tr_mac));
			bcopy(&afdata->leader_mac, &new_tr->ar_mac, sizeof(afdata->leader_mac));
			new_tr->time_val = OSL_SYSUPTIME();
			new_tr->artmo = afdata->artmo;
			if (!ar_flag) {
				new_tr->amt_idx = ERMC_INVALID_AMT_IDX;
			}
			ERMC_DBG(("Create new tr_info"));
		}
		rmcp->tr_cnt++;
		wlc_rmc_insert_tr(rmcp->trs, new_tr);
		rmcp->ermc_cnts->mc_existing_tr++;
		if (!ar_flag) {
			return ERMC_MC_EXISTING_TR;
		}
	}

	if (!ar_flag) {
		/* It is an existing tr entry - need to make sure there is no AMT entry */
		if (new_tr->amt_idx != ERMC_INVALID_AMT_IDX) {
			if (wlc_rmc_amt_del_ta(rmcp->wlc, new_tr) == BCME_OK) {
				return ERMC_MC_PROGRAMMED;
			}
		}
		return BCME_ERROR;
	} else {

		/* Before programmng entry in AMT, get free amt_idx or find ta */
		ret = wlc_rmc_get_amt_index(rmcp->wlc, tr_mac, &amt_idx, &attr);

		if (ret == BCME_OK) {
			ERMC_DBG(("Existing AMT idx %d", amt_idx));
			/* Found AMT entry - verify and make sure it has right attr and val */
			/* If they are not of the right attr or val, change it              */
			new_tr->amt_idx = amt_idx;
			wlc_rmc_amt_set(rmcp->wlc, new_tr, attr);
		} else if (ret == BCME_NOTFOUND) {
			/* Did not find AMT entry, insert the entry */
			/* This TA is the one we should ack */
			ERMC_DBG(("Not existing idx, insert new idx %d", amt_idx));
			new_tr->amt_idx = amt_idx;
			wlc_rmc_amt_insert_entry(rmcp->wlc, new_tr);
		} else if (BCME_NOCLK) {
			ERMC_DBG(("No wl clk \n"));
			/* add err counter */
			rmcp->ermc_cnts->mc_no_wl_clk++;
			return ERMC_MC_NOT_PROCESSED;
		} else {
			ERMC_DBG(("No free slot \n"));
			rmcp->ermc_cnts->mc_no_amt_slot++;
			return ERMC_MC_NO_AMT_SLOT;
		}
		return ERMC_MC_PROGRAMMED;
	}
}

/* To validate whether the received frame is rmc action frame or not */
bool wlc_rmc_check_actframe(wlc_info_t *wlc, uint8 *body, int body_len)
{
	bool flag = FALSE;

	/* XXX action frames are not allowed in infra mode to maintain,
	  * compatibility with legacy rmc implementation. Don't process,
	  * them.
	  */
	if (!(RMC_SUPPORT(wlc->pub) &&
	    (body_len >= sizeof(rmc_notify_af_t))) ||
	    (BSSCFG_AP(wlc->cfg) ||
		wl_rmc_is_p2p_go(wlc))) {
		return FALSE;
	}

#ifdef IBSS_RMC
	flag = (!bcmp(((rmc_dot11_action_vs_frmhdr_t *)body)->oui,
	                 RMC_PROP_OUI, DOT11_OUI_LEN) &&
	        !bcmp(((rmc_dot11_action_vs_frmhdr_t *)body)->magic,
	                 RMC_MAGIC_STRING, strlen(RMC_MAGIC_STRING)));

	ASSERT(((rmc_dot11_action_vs_frmhdr_t *)body)->category == DOT11_ACTION_CAT_VS);
#else
	flag = (!bcmp(((dot11_action_vs_frmhdr_t *)body)->OUI,
	                 BRCM_PROP_OUI, DOT11_OUI_LEN) &&
	        (((dot11_action_vs_frmhdr_t *)body)->type == BRCM_RELMACST_AF_TYPE));

	ASSERT(((dot11_action_vs_frmhdr_t *)body)->category == DOT11_ACTION_CAT_VS);
#endif /* IBSS_RMC */

	return flag;
}

/* increment counter when a multicast frames is transmitted */
void wlc_rmc_tx_frame_inc(wlc_info_t *wlc)
{
	wlc_rmc_info_t* ermc = wlc->rmc;
	ermc->ermc_cnts->rmc_tx_frames_mac++;
}

static void wlc_rmc_build_ie(wlc_rmc_info_t * const rmcp,
        relmcast_brcm_prop_ie_t *relmcast_brcm_prop_ie)
{
	relmcast_brcm_prop_ie->id = DOT11_MNG_PROPR_ID;
	relmcast_brcm_prop_ie->len = RELMCAST_BRCM_PROP_IE_LEN;
#ifdef IBSS_RMC
	bcopy(RMC_PROP_OUI, &relmcast_brcm_prop_ie->oui[0], DOT11_OUI_LEN);
	relmcast_brcm_prop_ie->type = RELMCAST_NON_BRCM_PROP_IE_TYPE;
#else
	bcopy(BRCM_PROP_OUI, &relmcast_brcm_prop_ie->oui[0], DOT11_OUI_LEN);
	relmcast_brcm_prop_ie->type = RELMCAST_BRCM_PROP_IE_TYPE;
#endif // endif
	bcopy(&rmcp->mc_ackmac, &relmcast_brcm_prop_ie->mcast_ea,
		sizeof(struct ether_addr));
	relmcast_brcm_prop_ie->updtmo = rmcp->mc_periodtmo;
	bcopy(&rmcp->mc_acker, &relmcast_brcm_prop_ie->ea, sizeof(struct ether_addr));
}

static int wlc_rmc_write_ie(wlc_rmc_info_t * const rmcp, wlc_bsscfg_t *cfg,
	uint type, uint8 * const buf, int maxbuflen)
{
	relmcast_brcm_prop_ie_t relmcast_brcm_prop_ie;

	ASSERT(rmcp != NULL);
	wl_rmc_check_signature(rmcp, WL_RMC_SIGNATURE);

	if (type != FC_BEACON && type != FC_ASSOC_REQ && type != FC_REASSOC_REQ)
		return 0;

	if (maxbuflen < sizeof(relmcast_brcm_prop_ie_t))
		return 0;

	if (!(cfg->target_bss->flags & WLC_BSS_BRCM))
		return 0;
	wlc_rmc_build_ie(rmcp, &relmcast_brcm_prop_ie);
	bcopy(&relmcast_brcm_prop_ie, buf, sizeof(relmcast_brcm_prop_ie_t));

	return sizeof(relmcast_brcm_prop_ie_t);
}

static uint
wlc_rmc_calc_rmc_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)ctx;
	uint8 buf[257];

	/* TODO: need a better way to calculate the legnth */
	buf[TLV_LEN_OFF] = 0;
	wlc_rmc_write_ie(rmcp, calc->cfg, calc->ft, buf, sizeof(buf));

	if (buf[TLV_LEN_OFF] > 0)
		return TLV_HDR_LEN + buf[TLV_LEN_OFF];

	return 0;
}

static int
wlc_rmc_write_rmc_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)ctx;
	wlc_rmc_write_ie(rmcp, build->cfg, build->ft, build->buf, build->buf_len);
	return BCME_OK;
}

static int
wlc_rmc_parse_rmc_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)ctx;

	if (parse->ie == NULL)
		return BCME_OK;

	if (!rmcp) {
		return BCME_ERROR;
	}

	switch (parse->ft) {
#ifdef STA
	case FC_BEACON:
		if (RMC_SUPPORT(rmcp->wlc->pub)) {
			if (BSSCFG_IBSS(parse->cfg)) {
				if (parse->pparm->ft->bcn.scb != NULL) {
					wlc_relmcast_assocreq_process(rmcp,
					                       parse->pparm->ft->bcn.scb,
					                       parse->ie, parse->ie_len);
				}
			} else {
					wlc_relmcast_beacon_process(rmcp, parse->cfg,
					                            parse->ie,
					                            parse->ie_len);
			}
		}

		break;
#endif /* STA */
#ifdef AP
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		wlc_relmcast_assocreq_process(rmcp, parse->pparm->ft->assocreq.scb,
		                              parse->ie, parse->ie_len);
		break;
#endif // endif
	default:
		break;
	}

	return BCME_OK;
}

/* timer to age out transmitter entries on receivers */
static void wlc_rmc_age_timer(void *arg)
{
	wlc_rmc_info_t *rmcp = (wlc_rmc_info_t *)arg;
	ASSERT(arg != NULL);
	uint32 sysUpTime = OSL_SYSUPTIME();
	wlc_rmc_trans_info_t *prev_tr = NULL;
	uint8 tr_idx = 0;
	wlc_rmc_trans_info_t* tr_info = NULL;
	for (tr_idx = 0, tr_info = rmcp->trs[0]; tr_idx < ERMC_HASH_TBL_SIZE;
		tr_idx++, tr_info = rmcp->trs[tr_idx]) {
		while (tr_info && rmcp->tr_cnt) {
			if ((sysUpTime - tr_info->time_val) > tr_info->artmo) {
				/* Exceed aging time, need to remove the tr_info */
				ERMC_DBG(("\nExceed aging sysUpTime %d time_val %d ",
					sysUpTime, tr_info->time_val));
				/* Remove from the list and AMT */
				if (tr_info->amt_idx != ERMC_INVALID_AMT_IDX) {
					wlc_rmc_amt_del_ta(rmcp->wlc, tr_info);
				}

				if (tr_info == rmcp->trs[tr_idx]) {
					/* special case head of list */
					rmcp->trs[tr_idx] = tr_info->next;
				} else {
					prev_tr->next = tr_info->next;
				}
				MFREE(rmcp->wlc->osh, tr_info, sizeof(*tr_info));
				tr_info = NULL;
				rmcp->tr_cnt--;
			}
			prev_tr = tr_info;
			if (tr_info != NULL)
				tr_info = tr_info->next;
		}
	}
}
