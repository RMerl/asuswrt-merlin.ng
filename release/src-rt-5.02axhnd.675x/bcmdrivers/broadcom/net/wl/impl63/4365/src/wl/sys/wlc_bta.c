/*
 * BTA (BlueTooth Alternate Mac and Phy module aka BT-AMP)
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_bta.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

/* compile this file only if BTAMP is enabled */
#ifdef WLBTAMP

#include <wlc_cfg.h>

#include <epivers.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi_channels.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_frmutil.h>
#ifdef BCMSUP_PSK
#include <proto/eapol.h>
#include <wlc_sup.h>
#endif // endif
#ifdef BCMAUTH_PSK
#include <wlc_auth.h>
#endif // endif
#include <wlc_bta.h>
#include <proto/bt_amp_hci.h>
#include <proto/802.11_bta.h>
#include <wl_export.h>
#include <wlc_ap.h>
#ifdef APCS
#include <wlc_apcs.h>
#include <wlc_scan.h>
#endif // endif
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_pcb.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#ifdef WDS
#include <wlc_wds.h>
#endif // endif

#ifdef WLBTWUSB
/* XXX There is a cookie of 'void *' bytes inserted before the 'buffer' pointer
 * hence make sure to reserve the storage.
 * Events are allocated using wlc_bta_alloc_hcievent() to ensure this.
 * Data use RX buffers and headroom + extra can ensure this?
 */
extern void Bta_queue_event(void *handle, uint8 *buffer, uint len);
extern VOID BtKernForwardData(void *Context, PVOID buffer, ULONG len);
#endif // endif

/* iovar table */
enum {
	IOV_BT_AMP,		/* enable/disable BT-AMP */
	IOV_BT_AMP_FLAGS,	/* test parameters */
	IOV_BT_AMP_CHAN,	/* specify channel */
	IOV_HCI_CMD,		/* HCI command */
	IOV_HCI_ACL_DATA,	/* HCI data packet */
	IOV_BT_AMP_11N,		/* enable/disable 11n support in BT-AMP */
	IOV_BT_AMP_FB,		/* enable FB when BTAMP is actived */
	IOV_BT_AMP_STATE_LOG,	/* get/clear BTAMP state log */
	IOV_BT_AMP_MSGLEVEL,
	IOV_BT_AMP_EVT_MSK,
#if defined(BCMDBG) || defined(WLMSG_BTA)
	IOV_BT_AMP_ACT_REPORT	/* force generate activity report */
#endif // endif
};

static const bcm_iovar_t bta_iovars[] = {
	{"btamp", IOV_BT_AMP, IOVF_SET_DOWN, IOVT_BOOL, 0},
	{"btamp_flags", IOV_BT_AMP_FLAGS, 0, IOVT_UINT8, 0},
	{"btamp_chan", IOV_BT_AMP_CHAN, 0, IOVT_UINT8, 0},
#ifndef DONGLEBUILD
	{"HCI_cmd", IOV_HCI_CMD, 0, IOVT_BUFFER, 0},
	{"HCI_ACL_data", IOV_HCI_ACL_DATA, 0, IOVT_BUFFER, 0},
#endif // endif
	{"btamp_11n_support", IOV_BT_AMP_11N, 0, IOVT_UINT8, 0},
	{"btamp_fb", IOV_BT_AMP_FB, 0, IOVT_BOOL, 0},
	{"btamp_statelog", IOV_BT_AMP_STATE_LOG, 0, IOVT_BUFFER, 0},
#if defined(BCMDBG) || defined(WLMSG_BTA)
	{"btamp_msglevel", IOV_BT_AMP_MSGLEVEL, 0, IOVT_UINT32, 0},
	{"btamp_evt_mask", IOV_BT_AMP_EVT_MSK, 0, IOVT_UINT32, 0},
	{"btamp_act_report", IOV_BT_AMP_ACT_REPORT, 0, IOVT_UINT32, 0},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

#define HCI_VERSION		5
#define HCI_REVISION		((EPI_VERSION_NUM >> 16) & 0xffff)
#define PAL_VERSION		1
#define MFG_NAME		0x000f
#define PAL_SUBVERSION		(EPI_VERSION_NUM & 0xffff)

#define WLC_MAXBTA		4
#define BTA_MAXLOGLINKS		(WLC_MAXBTA * 4)

/* maximum PDU allowed (1492 octets) */
#define BTA_MTU	(ETHER_MAX_DATA - DOT11_LLC_SNAP_HDR_LEN)

/* maximum number of data blocks allowed (no validation yet) */
#ifndef BTA_MAXDATABLKS
#define BTA_MAXDATABLKS		12
#endif // endif

/* bsscfg flags */
#define BTA_BSSCFG_FLAGS	(WLC_BSSCFG_NOBCMC | \
				 WLC_BSSCFG_NOIF | \
				 WLC_BSSCFG_11H_DISABLE)

/* invalid flush timeout value */
#define BTA_INVFTO		0xffffffff

/* maximum MAC retries */
#ifndef WLBTAMP_D11RETRY
#define WLBTAMP_D11RETRY	15
#endif // endif

/* BT-AMP logical link */
typedef struct bta_ll {
	uint8 plh;			/* associated physical link handle */
	uint8 plidx;			/* physical link states struct index in bta_info_t */
	uint8 prio;
	uint8 tx_fs_ID;			/* tx flow spec ID */
	uint16 reqbw;
	uint32 fto;			/* flush timeout */
	uint8 datablks_complete;	/* number of data blocks completed */
	uint16 failed_contact_cnt;	/* failed contact counter */
} bta_ll_t;

/* BT-AMP physical link */
typedef struct bta_pl {
	uint8 *local;			/* local AMP_ASSOC */
	uint8 *remote;			/* remote AMP_ASSOC */
	uint16 llen;			/* local AMP_ASSOC length */
	uint16 rlen;			/* remote AMP_ASSOC length */
	uint ca_ts;			/* connection attempt timestamp */
	struct scb *scb;		/* scb used by this physical link */
	uint8 link_key[32];
	uint8 lk_type_len;		/* link_key_type | link_key_length */
	uint8 flags;			/* see BTA_PL_XXXX */
	uint16 ls_to;			/* link supervision timeout */
	uint16 allocbw;			/* guaranteed bandwidth allocated for this physical link */
	uint8 short_range;		/* short range mode */
	uint used;
} bta_pl_t;

/*
 * each order represent the byte-offset and bit-poistion
 * Reset				( 5, 7)
 * Read_Connection_Accept_Timeout	( 7, 2)
 * Write_Connection_Accept_Timeout	( 7, 3)
 * Read_Link_Supervision_Timeout	(11, 0)
 * Write_Link_Supervision_Timeout	(11, 1)
 * Read_Local_Version_Info		(14, 3)
 * Read_Buffer_Size			(14, 7)
 * Read_Failed_Contact_Counter		(15, 2)
 * Reset_Failed_Contact_Counter		(15, 3)
 * Read_Link_Quality			(15, 4)
 * Enhance_Flush			(19, 6)
 * Create_Physical_Link			(21, 0)
 * Accept_Physical_Link			(21, 1)
 * Disconnect_Physical_Link		(21, 2)
 * Create_Logical_Link			(21, 3)
 * Accept_Logical_Link			(21, 4)
 * Disconnect_Logic_Link		(21, 5)
 * Logical_Link_Cancel			(21, 6)
 * Flow_Spec_Modify			(21, 7)
 * Read_Logic_Link_Accept_timeout	(22, 0)
 * Write_Logic_Link_Accept_timeout	(22, 1)
 * Set_Event_Mask_Page_2		(22, 2)
 * Read_Location_Data_Command		(22, 3)
 * Write_Location_Data_Command		(22, 4)
 * Read_Local_AMP_Info			(22, 5)
 * Read_Local_AMP_ASSOC			(22, 6)
 * Write_Remote_AMP_ASSOC		(22, 7)
 * Read_Data_Block_Size			(23, 2)
 * Read_Best_Effort_Flash_Timeout	(24, 2)
 * Write_Best_Effort_Flash_Timeout	(24, 3)
 * Short_Range_Mode			(24, 4)
 */
static const uint8 hci_cmd_vec[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0c,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x88, 0x1c,
	0x00, 0x00, 0x00, 0x40, 0x00, 0xff, 0xff, 0x04,
	0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* link_key type and length */
#define	BTA_PLK_LENGTH_MASK	0x3F
#define	BTA_PLK_TYPE_MASK	0xC0
#define BTA_PLK_TYPE_SHIFT	6

/* physical link flags */
#define	BTA_PL_CREATOR		0x01	/* creator */
#define BTA_PL_COMPLETE		0x02	/* link completed */
#define BTA_PL_USE_RTS		0x04	/* RTS/CTS protection */
#define BTA_PL_CS_PEND		0x08	/* Channel Select process pending */
#define BTA_PL_CSE_PEND		0x10	/* HCI_Channel_Select event pending */
#define BTA_PL_CONN		0x20	/* connecting */

/* Channel Select info */
typedef struct bta_cs {
	/* the index of the physical link that initiated the process */
	int plidx;
} bta_cs_t;

/* BTA module info */
struct bta_info {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	int scb_handle;			/* scb cubby handle to retrieve data from scb */
	bta_ll_t ll[BTA_MAXLOGLINKS];	/* array of logical links (indexed by handle) */
	bta_pl_t pl[WLC_MAXBTA];	/* array of physical links */
	uint8 plh[WLC_MAXBTA];		/* physical link handles */
	uint8 *local;			/* default local AMP_ASSOC */
	uint16 llen;			/* length of default local AMP_ASSOC */
	uint16 lla_to;			/* logical link accept timeout */
	uint16 ca_to;			/* connection accept timeout */
	uint8 flags;			/* see BTA_FLAGS_XXXX */
	uint8 numpls;			/* number of active physical links */
	wlc_bsscfg_t *bsscfg;		/* creators' container/acceptors' beaconing bsscfg */
	struct wl_timer *cse_timer;	/* deferred Channel Select event timer */
	bta_cs_t *cs;			/* Channel Select control block */
	chanspec_t chanspec_sel;	/* auto selected channel */
	chanspec_t chanspec_user;	/* user specified channel */
	uint8 datablks_pending;		/* number of date blocks being xmitted */
	uint8 datablks_complete;	/* number of data blocks completed */
	uint16 evt_mask_2;		/* non-reserved portion of event mask page 2 */
	uint8 ld_aware;			/* regulatory domain unknown or known */
	uint8 ld[2];			/* 2-byte country code */
	uint8 ld_opts;			/* 1-byte country code qualifier */
	uint8 l_opts;			/* location options */
	uint8 numpls_allocated;		/* number of used bta_pl_t structs */
	uint8 amp_state;		/* track of amp state */

	/* global states to save and restore */
	bool _apsta;
	bool _ap;
	bool _fb;
	uint8 infra;
#if WLBTAMP_D11RETRY > 7
	uint8 SRL;
	uint8 LRL;
#endif // endif
	bool support_11n;
	uint32 msglevel;
	uint16 evt_prn_mask;
	uint32 acl_pkts;
	uint32 acl_bytes;
	uint8 state[BTA_STATE_LOG_SZ];
	uint8 state_idx;
	wlc_bsscfg_t *bsscfg_creator;   /* creators' container bsscfg */
};

#define BTA_HCI_DATA_MSG	0x1
#define BTA_HCI_EVENTS_MSG	0x2
#define BTA_HCI_DATALOST_MSG	0x4
#define BTA_HCI_PKTCNT_MSG	0x8

#define WL_BTA_OFF(arg)

/* BTA module specific state */
#define BTA_FLAGS_NO_ASSOC		0x01	/* acceptor doesn't have to associate w/ creator */
#define BTA_FLAGS_NO_SECURITY		0x02	/* no 4-way handshake */
#define BTA_FLAGS_NO_QOS		0x04	/* no QoS processing */
#define BTA_FLAGS_ET_RX			0x08	/* preserve Ethernet header */

#define BTA_FLAGS_QTP_AR		0x10	/* send activity report spec'd in Qual Test Plan */

#define BTA_FLAGS_NO_11N		0x40	/* no .11n (for pre 11n) */
#define BTA_FLAGS_NO_CATO		0x80	/* no link accept timeout (for debugging) */

/* scb cubby structure */
typedef struct scb_bta {
	uint8 plh;
	uint8 plidx;
} scb_bta_t;

#define SCB_BTA(bta, scb)	(scb_bta_t *)SCB_CUBBY((scb), (bta)->scb_handle)

#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define VENDOR_SPECIFIC_EVENT
#endif // endif

#ifndef VENDOR_SPECIFIC_EVENT
#define VENDOR_SPECIFIC_EVENT
#endif // endif

#ifdef VENDOR_SPECIFIC_EVENT
/* code to differentiate HCI_ERR_UNSPECIFIED/HCI_ERR_MEMORY_FULL in different context */
#define WLC_BTA_ERR_JOIN_START		1	/* join start */
#define WLC_BTA_ERR_JOIN_COMPLETE	2	/* join complete */
#define WLC_BTA_ERR_BSSCFG		3	/* no bsscfg */
#define WLC_BTA_ERR_ALLOC_BSSCFG	4	/* alloc bsscfg */
#define WLC_BTA_ERR_INIT_BSSCFG		5	/* init bsscfg */
#define WLC_BTA_ERR_ENABLE_BSSCFG	6	/* enable bsscfg */
#define WLC_BTA_ERR_DIFF_CHANSPEC	7	/* different chanspec */
#define WLC_BTA_ERR_FIND_SCB		8	/* find scb */
#define WLC_BTA_ERR_AUTH		9	/* no authenticator */
#define WLC_BTA_ERR_SUP			10	/* no supplicant */
#define WLC_BTA_ERR_CSE_TIMER		11	/* no cse timer */
#define WLC_BTA_ERR_PREF_CHAN_LIST	12	/* preferred channel list */
#define WLC_BTA_ERR_CS_CHAN_LIST1	13	/* alloc chan list for channel select */
#define WLC_BTA_ERR_CS_CHAN_LIST2	14	/* alloc chan list for channel select */
#define WLC_BTA_ERR_CHAN		15	/* no channel in preferred channel list */
#define WLC_BTA_ERR_CHANSPEC_RANGE	16	/* chanspec range */
#define WLC_BTA_ERR_CS_START		17	/* channel select start */
#define WLC_BTA_ERR_CS_COMPLETE		18	/* channel select complete */
#define WLC_BTA_ERR_CS_CHAN_LIST3	19	/* alloc chan list for channel select */
#define WLC_BTA_ERR_CS_CB		20	/* alloc callback for channel select */
#define WLC_BTA_ERR_CS_RC_LIST		21	/* alloc reg. class list for channel select */
#define WLC_BTA_ERR_FIND_RC		22	/* find regulatory class from chanspec */
#define WLC_BTA_ERR_FIND_RC_LIST	23	/* find regulatory class list */
#define WLC_BTA_ERR_TOO_MANY_RC		24	/* too many regulatory classes */
#define WLC_BTA_ERR_RC			25	/* no regulatory class */
#define WLC_BTA_ERR_LOCAL_AMP_ASSOC	26	/* NULL 'local' AMP ASSOC */
#define WLC_BTA_ERR_REMOTE_AMP_ASSOC	27	/* alloc 'remote' AMP ASSOC */
#define WLC_BTA_ERR_TOO_MUCH_DATA	28	/* too much HCI command payload */
#define WLC_BTA_ERR_PHY_LINK_EXIST	29	/* phy link exists */
#define WLC_BTA_ERR_MAC_LIST		30	/* alloc MAC address list */
#define WLC_BTA_ERR_FLOW_SPEC_ID	31	/* flow spec ID */
#define WLC_BTA_ERR_FLOW_SPEC_TYPE	32	/* flow spec service type */
#define WLC_BTA_ERR_SCB_EXIST		33	/* scb exist for other physical link */
#define WLC_BTA_ERR_AP_EXIST		34	/* softAP exist */
#endif /* VENDOR_SPECIFIC_EVENT */

/* local prototypes */
static int wlc_bsscfg_bta_init(bta_info_t *bta, wlc_bsscfg_t *bsscfg);
static int scb_bta_init(void *context, struct scb *scb);
static void scb_bta_deinit(void *context, struct scb *scb);

static int wlc_bta_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen, int vsize,
	struct wlc_if *wlcif);
static void wlc_bta_doevent(bta_info_t *bta, amp_hci_event_t *event);

static void wlc_bta_disconnect_physical_link(bta_info_t *bta, int plidx, uint8 reason,
	bool lle);

static int wlc_bta_dodisconnect(bta_info_t *bta, int plidx);
static int wlc_bta_phy_link_dodisconnect(bta_info_t *bta, int plidx, uint8 reason,
	bool ple, bool lle, bool dis);

static uint8 wlc_bta_doconnect(bta_info_t *bta, int plidx, chanspec_t chanspec);
static void wlc_bta_doconnect_error(bta_info_t *bta, int plidx, uint8 status);
static void wlc_bta_phy_link_doconnect(bta_info_t *bta, int plidx);
static void wlc_bta_phy_link_complete(bta_info_t *bta, struct scb *scb);
static void wlc_bta_phy_link_complete_event(bta_info_t *bta, uint8 *buf, uint buflen,
	bool disconnect);

static void wlc_bta_tx_hcidata_complete_event(bta_info_t *bta, uint16 llh, bool flush);

static void wlc_bta_disconnect_logical_link(bta_info_t *bta, uint16 llh, uint8 reason,
	bool lle);

static uint8 wlc_bta_valid_logical_link(bta_info_t *bta, uint16 llh);

static void wlc_bta_log_link_event(bta_info_t *bta, uint8 ecode, uint8 *buf, uint buflen);

static void wlc_bta_flush_hcidata(bta_info_t *bta, uint16 llh);
static void wlc_bta_flush_hcidata_complete_event(bta_info_t *bta, uint16 llh);
static void wlc_bta_flush_hcidata_occurred_event(bta_info_t *bta, uint16 llh);

static amp_hci_event_t *wlc_bta_alloc_hcievent(bta_info_t *bta, uint8 ecode, uint8 plen);
static void wlc_bta_free_hcievent(bta_info_t *bta, amp_hci_event_t *evt);

static void wlc_bta_send_link_supervision_pkt(bta_info_t *bta, bta_pl_t *pl, bool request);
static void wlc_bta_send_activity_report_pkt(bta_info_t *bta, bta_pl_t *phy_link);
#if defined(BCMDBG) || defined(WLMSG_BTA)
static void wlc_bta_send_activity_report_flag(bta_info_t *bta, bta_pl_t *phy_link, int flag);
#endif /* BCMDBG || WLMSG_BTA */
static void wlc_bta_send_hcidata_pkt(bta_info_t *bta, uint16 llh, void *p);

static void wlc_bta_evtfwd_upd(bta_info_t *bta);
static void wlc_bta_flags_upd(bta_info_t *bta);

static void wlc_bta_watchdog(void *context);
static int wlc_bta_down(void *context);

static uint8 wlc_bta_build_AMP_ASSOC(bta_info_t *bta, chanspec_t chanspec,
	uint8 **AMP_ASSOC, uint16 *AMP_ASSOC_len);
static uint8 wlc_bta_cs(bta_info_t *bta, int plidx, uint8 *pref_chan, uint pref_chan_len,
	chanspec_t *chanspec_sel);

static uint8 *wlc_bta_parse_tlvs(void *buf, int buflen, uint key);
static void wlc_bta_add_hdrs(bta_pl_t *pl, void *p, uint16 type);

static uint8 wlc_bta_join(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uint8 *SSID, int SSID_len, uint8 *bssid, chanspec_t chanspec);

static void wlc_bta_default(bta_info_t *bta);

static void wlc_bta_tx_hcidata_complete(wlc_info_t *wlc, void *pkt, uint txs);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_bta_dump(bta_info_t *bta, struct bcmstrbuf *b);
#endif // endif

/* IE mgmt */
#ifdef AP
static uint wlc_bta_calc_edca_param_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_bta_write_edca_param_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef STA
static int wlc_bta_parse_edca_param_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_bta_scan_parse_edca_param_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
#ifdef STA
static uint wlc_bta_calc_qos_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_bta_write_qos_cap_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef AP
static int wlc_bta_parse_qos_cap_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif

/* default local AMP_ASSOC */
static const uint8 default_local[] = {
	0x04, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x05, 0x05, 0x00, 0x01, 0x0f, 0x00, 0x10, 0x09,
	/* BTA_TYPE_MAC_ADDRESS and BTA_TYPE_ID_PREFERRED_CHANNELS are here... */
};

/* AMP SSID format */
static const char *amp_ssid_format = "AMP-%02x-%02x-%02x-%02x-%02x-%02x";

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static INLINE void
wlc_bta_state_log(bta_info_t *bta, uint8 state)
{
	bta->state[bta->state_idx++] = state;
	bta->state_idx %= BTA_STATE_LOG_SZ;
}

bta_info_t *
BCMATTACHFN(wlc_bta_attach)(wlc_info_t *wlc)
{
	bta_info_t *bta;
#ifdef AP
	uint16 edca_build_fstbmp =
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_PROBE_RESP) |
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_RESP) |
	        0;
#endif // endif
#ifdef STA
	uint16 edca_parse_fstbmp =
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_RESP) |
	        0;
	uint16 edca_scan_fstbmp =
	        FT2BMP(WLC_IEM_FC_SCAN_BCN) |
	        FT2BMP(WLC_IEM_FC_SCAN_PRBRSP) |
	        0;
#endif // endif
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);

	WL_TRACE(("wl%d: wlc_bta_attach: enter\n", wlc->pub->unit));

	/* sanity checks */
	ASSERT(WLC_MAXBTA > 0);

	if ((bta = (bta_info_t *)MALLOCZ(wlc->osh, sizeof(bta_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bta->wlc = wlc;

	/* reserve cubby in the scb container for per-scb private data */
	bta->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(scb_bta_t),
		scb_bta_init, scb_bta_deinit, NULL, (void *)bta);

	if (bta->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, bta_iovars, "bta", bta, wlc_bta_doiovar,
	                        wlc_bta_watchdog, NULL, wlc_bta_down)) {
		WL_ERROR(("wl%d: bta wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_BTA_HCI_ACL,
	                   wlc_bta_tx_hcidata_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt callback */
	/* calc/build */
#ifdef AP
	/* bcn/prbrsp/assocresp/reassocresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, edca_build_fstbmp, DOT11_MNG_EDCA_PARAM_ID,
	      wlc_bta_calc_edca_param_ie_len, wlc_bta_write_edca_param_ie, bta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, edca param in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
#ifdef STA
	/* calc/build */
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_QOS_CAP_ID,
	      wlc_bta_calc_qos_cap_ie_len, wlc_bta_write_qos_cap_ie, bta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, qos cap in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
	/* parse */
#ifdef STA
	/* assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, edca_parse_fstbmp, DOT11_MNG_EDCA_PARAM_ID,
	                             wlc_bta_parse_edca_param_ie, bta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, edca param in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* bcn/prbresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, edca_scan_fstbmp, DOT11_MNG_EDCA_PARAM_ID,
	                             wlc_bta_scan_parse_edca_param_ie, bta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, edca param in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
#ifdef AP
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_QOS_CAP_ID,
	                             wlc_bta_parse_qos_cap_ie, bta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, qos cap in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	wlc_bta_default(bta);

	/* temporary inits */
	wlc->pub->_bta = TRUE;

	/* initial flags */
#ifdef DONGLEBUILD
	bta->flags |= BTA_FLAGS_ET_RX;
#endif // endif
	/* bta->flags |= BTA_FLAGS_NO_SECURITY; */
	if (!bta->support_11n)
		bta->flags |= BTA_FLAGS_NO_11N;

	/* update event forwarding mask */
	wlc_bta_evtfwd_upd(bta);
	/* update stuff depending on flags */
	wlc_bta_flags_upd(bta);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "bta", (dump_fn_t)wlc_bta_dump, (void *)bta);
#endif // endif

	/* initialize radio to disabled */
	bta->amp_state = WLC_BTA_RADIO_DISABLE;

#if defined(BCMDBG) || defined(WLMSG_BTA)
	bta->msglevel = (BTA_HCI_DATA_MSG | BTA_HCI_EVENTS_MSG | BTA_HCI_DATALOST_MSG);
	bta->evt_prn_mask = HCI_All_Event_Mask;
#endif // endif
	wlc_bsscfg_type_register(wlc, BSSCFG_TYPE_BTA,
		(bsscfg_type_init_t)wlc_bsscfg_bta_init, bta);
	return bta;

fail:
	MFREE(wlc->osh, bta, sizeof(bta_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_bta_detach)(bta_info_t *bta)
{
	wlc_info_t *wlc = bta->wlc;
	int i;

	wlc_module_unregister(wlc->pub, "bta", bta);
	for (i = 0; i < WLC_MAXBTA; i++) {
		bta_pl_t *phy_link = &bta->pl[i];

		if (phy_link->local)
			MFREE(wlc->osh, phy_link->local, phy_link->llen);
		if (phy_link->remote)
			MFREE(wlc->osh, phy_link->remote, phy_link->rlen);
	}
	if (bta->local)
		MFREE(wlc->osh, bta->local, bta->llen);
	if (bta->cse_timer != NULL)
		wl_free_timer(wlc->wl, bta->cse_timer);
	if (bta->cs != NULL)
		MFREE(wlc->osh, bta->cs, sizeof(bta_cs_t));
	MFREE(wlc->osh, bta, sizeof(bta_info_t));
}

static int
wlc_bsscfg_bta_init(bta_info_t *bta, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bta->wlc;

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_bta_init: bsscfg %p\n",
	          wlc->pub->unit, bsscfg));

	/* N.B.: QoS settings configured implicitly */

	return wlc_bsscfg_init(wlc, bsscfg);
}

static int
scb_bta_init(void *context, struct scb *scb)
{
	return 0;
}

static void
scb_bta_deinit(void *context, struct scb *scb)
{
	bta_info_t *bta = (bta_info_t *)context;
	scb_bta_t *scb_bta;

	scb_bta = SCB_BTA(bta, scb);
	bzero(scb_bta, sizeof(scb_bta_t));
}

static void
wlc_bta_default(bta_info_t *bta)
{
	/* set default connection accept timeout */
	bta->ca_to = 0x1f40;

	/* set default logical link accept timeout */
	bta->lla_to = 0x1f40;

	/* reset event mask page 2 to disable AMP events */
	bta->evt_mask_2 = 0;
}

static uint8
wlc_bta_phy_link_rssi(bta_info_t *bta, int plidx)
{
	bta_pl_t *pl = &bta->pl[plidx];
	int rssi = WLC_RSSI_MINVAL;
	uint8 rval = 0;

	if (pl->scb != NULL)
		rssi = wlc_scb_rssi(pl->scb);

	if (rssi != WLC_RSSI_MINVAL) {
		if (rssi < -90)
			rval = 0;
		else if (rssi < -70)
			rval = (rssi + 90) * 2;
		else if (rssi < -68)
			rval = ((rssi + 70) * 10) + 40;
		else if (rssi < -58)
			rval = ((rssi + 68) * 2) + 60;
		else if (rssi < -30)
			rval = (((rssi + 58) * 5) / 7) + 80;
		else
			rval = 100;
		rval += 150;
	}
	return rval;
}

static void
wlc_bta_watchdog(void *context)
{
	bta_info_t *bta = (bta_info_t *)context;
	wlc_info_t *wlc = bta->wlc;
	uint now;
	int plidx;

	now = wlc->pub->now;

	/* For each physical link... */
	for (plidx = 0; plidx < WLC_MAXBTA; plidx++) {
		bta_pl_t *phy_link;
		uint8 plh;

		if (bta->plh[plidx] == 0)
			continue;

		plh = bta->plh[plidx];

		phy_link = &bta->pl[plidx];

		if (phy_link->flags & BTA_PL_COMPLETE) {
			uint time = (now - phy_link->used) * 1600;

			/* if completed, check link supervision timeout */
			if (time > (uint)phy_link->ls_to) {
				WL_BTA(("wl%d: link supervision timeout for plh 0x%x\n",
					wlc->pub->unit, plh));
				BCM_REFERENCE(plh);
				/* disconnect the physical link */
				wlc_bta_phy_link_dodisconnect(bta, plidx,
				                              HCI_ERR_CONNECTION_TIMEOUT,
				                              TRUE, TRUE, FALSE);
			}
			/* if completed and close to link supervision timeout, probe */
			else if (time > ((uint)phy_link->ls_to >> 1)) {
				/* send link supervision request */
				wlc_bta_send_link_supervision_pkt(bta, phy_link, TRUE);
			}
		} else {

			/* if pending, check connection accept timeout */
			if (!(bta->flags & BTA_FLAGS_NO_CATO) &&
			    ((now - phy_link->ca_ts) * 1600) > bta->ca_to) {
				WL_BTA(("wl%d: connection accept timeout for plh 0x%x delta %d\n",
					wlc->pub->unit, plh, (now - phy_link->ca_ts)));

				/* kill the physical link */
				wlc_bta_phy_link_dodisconnect(bta, plidx,
				                              HCI_ERR_CONNECTION_ACCEPT_TIMEOUT,
				                              TRUE, FALSE, FALSE);
			}

			/* if connection accept not timeout, process pending Channel Select */
			if (phy_link->flags & BTA_PL_CS_PEND) {
				WL_BTA(("wl%d: restart Channel Select for physical link %d\n",
				        wlc->pub->unit, plh));

				phy_link->flags &= ~BTA_PL_CS_PEND;
				wlc_bta_phy_link_doconnect(bta, plidx);
			}
		}
	}
}

static int
wlc_bta_down(void *context)
{
	bta_info_t *bta = (bta_info_t *)context;
	int plidx;
	int callbacks = 0;

	for (plidx = 0; plidx < WLC_MAXBTA; plidx ++) {
		if (bta->plh[plidx] == 0)
			continue;
		WL_BTA(("wl%d: wlc_bta_down: disconnect physical link %d\n",
		        bta->wlc->pub->unit, bta->plh[plidx]));
		callbacks += wlc_bta_phy_link_dodisconnect(bta, plidx, 0, FALSE, FALSE, FALSE);
	}

	return callbacks;
}

/* AMP HCI ACL data packet processing */
#if defined(BCMDBG) || defined(WLMSG_BTA)
void
wlc_bta_dump_stats(bta_info_t *bta)
{
	if ((bta->msglevel & BTA_HCI_PKTCNT_MSG) && bta->acl_pkts) {
		WL_ERROR(("DataFromBTW total pkts %d in %d bytes\n",
			bta->acl_pkts, bta->acl_bytes));
		bta->acl_bytes = 0;
		bta->acl_pkts = 0;
	}
}

static void
wlc_bta_hcidump_ACL_data(bta_info_t *bta, amp_hci_ACL_data_t *ACL_data, bool tx)
{
	wlc_info_t *wlc = bta->wlc;
	uint16 handle = ltoh16(ACL_data->handle);
	uint16 dlen = ltoh16(ACL_data->dlen);

	if (bta->msglevel & BTA_HCI_DATA_MSG) {
		WL_BTA(("wl%d: %s ACL data: handle 0x%04x flags 0x%02x dlen %d\n",
			wlc->pub->unit,	tx ? "<" : ">", HCI_ACL_DATA_HANDLE(handle),
			HCI_ACL_DATA_FLAGS(handle), dlen));
		prhex(NULL, ACL_data->data, dlen);
		WL_BTA(("\n"));
	}
}
#endif /* BCMDBG || WLMSG_BTA */

static void
wlc_bta_send_link_supervision_pkt(bta_info_t *bta, bta_pl_t *phy_link, bool request)
{
	wlc_info_t *wlc = bta->wlc;
	void *pkt;

	/* get an OSL packet w/ appropriate headroom */
	if (!(pkt = PKTGET(wlc->osh, TXOFF + RFC1042_HDR_LEN, TRUE))) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
		          wlc->pub->unit, __FUNCTION__, 0));
		return;
	}

	/* headers only and no payload */
	PKTPULL(wlc->osh, pkt, TXOFF + RFC1042_HDR_LEN);

	/* add Ethernet header with 802.11 PAL LLC/SNAP header */
	wlc_bta_add_hdrs(phy_link, pkt, request ?
	                 BTA_PROT_LINK_SUPERVISION_REQUEST :
	                 BTA_PROT_LINK_SUPERVISION_REPLY);

	/* RTS/CTS protection */
	if (phy_link->flags & BTA_PL_USE_RTS)
		WLPKTTAG(pkt)->flags |= WLF_USERTS;

	/* submit to wl tx path */
	wlc_sendpkt(wlc, pkt, SCB_WDS(phy_link->scb));
}

/* Return TRUE if the packet must be freed by the caller otherwise allow the
 * packet to go up to the stack
 */
bool
wlc_bta_recv_proc(bta_info_t *bta, struct wlc_frminfo *f, struct scb *scb)
{
	wlc_info_t *wlc = bta->wlc;
	uint16 type = ntoh16(((struct dot11_llc_snap_header *)&f->eh[1])->type);
	uint16 len = ntoh16(f->eh->ether_type);
	amp_hci_ACL_data_t *HCI_hdr = NULL;
	bool toss = FALSE;
	bta_pl_t *phy_link;
	scb_bta_t *scb_bta;
	bool update = TRUE;

	ASSERT(scb != NULL);

	scb_bta = SCB_BTA(bta, scb);
	phy_link = &bta->pl[scb_bta->plidx];

	switch (type) {
	case BTA_PROT_L2CAP: {
		uint8 *src = (uint8 *)f->eh;
		uint16 handle, dlen;
		uint8 i;
		uint8 *dst = src - HCI_ACL_DATA_PREAMBLE_SIZE;

		/* insert HCI ACL data header between RFC1042 header and HCI ACL data */
		/* copy Ethernet/LLC/SNAP header back to preserve HCI ACL data header */
		for (i = 0; i < RFC1042_HDR_LEN; i++)
			dst[i] = src[i];

		/* update frame info fields */
		PKTPUSH(wlc->osh, f->p, HCI_ACL_DATA_PREAMBLE_SIZE);
#if defined(EXT_STA) && defined(NOT_YET)
		f->h = (struct dot11_header *)
			((uintptr)f->h - HCI_ACL_DATA_PREAMBLE_SIZE);
#else
		f->eh = (struct ether_header *)
			((uintptr)f->eh - HCI_ACL_DATA_PREAMBLE_SIZE);
#endif // endif
		f->pbody -= HCI_ACL_DATA_PREAMBLE_SIZE;

		/* update packet headers (with insertion of HCI ACL data header) */
		f->eh->ether_type = hton16(len + HCI_ACL_DATA_PREAMBLE_SIZE);
		handle = HCI_ACL_DATA_BC_FLAGS | HCI_ACL_DATA_PB_FLAGS | scb_bta->plh;
		dlen = PKTLEN(wlc->osh, f->p) - RFC1042_HDR_LEN - HCI_ACL_DATA_PREAMBLE_SIZE;
		HCI_hdr = (amp_hci_ACL_data_t *)((uintptr)f->eh + RFC1042_HDR_LEN);
		HCI_hdr->handle = htol16(handle);
		HCI_hdr->dlen = htol16(dlen);

		/* Forward HCI ACL data only */
		if (!(bta->flags & BTA_FLAGS_ET_RX)) {
#ifdef WLBTWUSB
			/* Forward data to BTKRNL */
			WL_BTA_OFF(("wl%d: forwarding rx HCI data frame to BTKRNL\n",
				wlc->pub->unit));
			/*
			 * XXX - This should go through sendup-like path through per-port code
			 *       which makes drops lock and makes this call...wl_btsendup()
			 */
			BtKernForwardData(wlc->wl, HCI_hdr, dlen + HCI_ACL_DATA_PREAMBLE_SIZE);

#endif // endif
			toss = TRUE;
		}
		/* Forward HCI ACL data along with the Ethernet/LLC/header */
		/* else {} */

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON())
			wlc_bta_hcidump_ACL_data(bta, HCI_hdr, FALSE);
#endif // endif

		break;
	}

	case BTA_PROT_ACTIVITY_REPORT: {

		amp_hci_activity_report_t *activity_report_body;

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: received Activity Report on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, f->p), PKTLEN(wlc->osh, f->p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* strip Ethernet/LLC/SNAP header */
		PKTPULL(wlc->osh, f->p, RFC1042_HDR_LEN);

		/*
		 * process activity report:
		 * if ScheduleKnown is set to 0, enable RTS
		 * else if NumReports is NOT 0, enable RTS or schedule
		 * else disable RTS
		 */
		activity_report_body = (amp_hci_activity_report_t *)PKTDATA(wlc->osh, f->p);
		if (!(activity_report_body->ScheduleKnown & HCI_AR_SCHEDULE_KNOWN)) {
			/* enable RTS */
			phy_link->flags |= BTA_PL_USE_RTS;
		} else if (activity_report_body->NumReports != 0) {
#ifdef NOT_YET
#else
			/* ...or enable RTS */
			phy_link->flags |= BTA_PL_USE_RTS;
#endif // endif
		} else {
			/* disable RTS */
			phy_link->flags &= ~BTA_PL_USE_RTS;
		}

		WL_BTA(("wl%d: turn %s RTS/CTS on physical link %d\n", wlc->pub->unit,
		        (phy_link->flags & BTA_PL_USE_RTS) ? "on" : "off",
		        bta->plh[scb_bta->plidx]));

		toss = TRUE;
		break;
	}

	case BTA_PROT_SECURITY: {

#if defined(BCMAUTH_PSK) || defined(BCMSUP_PSK)
		wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
		bool sendup = TRUE;
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: received BT-SIG 802.1x frame on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, f->p), PKTLEN(wlc->osh, f->p));
			WL_BTA(("\n"));
		}
#endif // endif

		if (bta->flags & BTA_FLAGS_NO_SECURITY) {
			toss = TRUE;
			break;
		}

#if defined(BCMAUTH_PSK) || defined(BCMSUP_PSK)
		/* call in-driver supplicant or authenticator */
		if (phy_link->flags & BTA_PL_CREATOR) {
#ifdef BCMAUTH_PSK
			if (BSS_SUP_INFO(wlc->idsup, bsscfg) && BCMAUTH_PSK_ENAB(wlc->pub) &&
				(bsscfg->authenticator != NULL))
				sendup = FALSE;
#endif // endif
		}
		else {
#ifdef BCMSUP_PSK
			if (BSS_SUP_TYPE(wlc->idsup, bsscfg) != SUP_UNUSED)
				sendup = FALSE;
#endif // endif
		}
		/* forward to in-driver supplicant or authenticator */
		if (!sendup) {
			/* XXX use the dispatching mechanism in wlc_recvdata_ordered() to
			 * forward the packet to the in-driver supplicant or authenticator,
			 * which can only work with the Ethernet II frame format.
			 */
			PKTPULL(wlc->osh, f->p, DOT11_LLC_SNAP_HDR_LEN);
			f->eh = (struct ether_header *)PKTDATA(wlc->osh, f->p);
			f->eh->ether_type = HTON16(ETHER_TYPE_802_1X);
			bcopy(f->sa, f->eh->ether_shost, ETHER_ADDR_LEN);
			bcopy(f->da, f->eh->ether_dhost, ETHER_ADDR_LEN);
		}
#endif /* BCMAUTH_PSK || BCMSUP_PSK */

		/* mark packet as 802.1x (since ether_type will be hard to detect) */
		WLPKTTAG(f->p)->flags |= WLF_8021X;

		/* don't update the tick count as the link is not complete yet */
		update = FALSE;

		/* continue the rx processing... */
		break;
	}

	case BTA_PROT_LINK_SUPERVISION_REQUEST:

		/* received link supervision request... */

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: received Link Supervision Request on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, f->p), PKTLEN(wlc->osh, f->p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* send link supervision reply */
		wlc_bta_send_link_supervision_pkt(bta, phy_link, FALSE);

		toss = TRUE;
		break;

	case BTA_PROT_LINK_SUPERVISION_REPLY:

		/* received link supervision reply... */

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: received Link Supervision Reply on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, f->p), PKTLEN(wlc->osh, f->p));
			WL_BTA(("\n"));
		}
#endif // endif

		toss = TRUE;
		break;

	default:

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: wlc_bta_recv_proc: unrecognized protocol: 0x%04x\n",
			        wlc->pub->unit, type));
			prhex(NULL, PKTDATA(wlc->osh, f->p), PKTLEN(wlc->osh, f->p));
			WL_BTA(("\n"));
		}
#endif // endif

		update = FALSE;
		toss = TRUE;
		break;
	}

	/* update phy_link's last activity time */
	if (update)
		phy_link->used = wlc->pub->now;

	return toss;
}

/* generate vendor-specific event */
static void
wlc_bta_vendor_specific_event(bta_info_t *bta, uint8 *data, uint8 len)
{
	amp_hci_event_t *evt;
	vendor_specific_evt_parms_t *parms;

	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Vendor_Specific,
	                sizeof(vendor_specific_evt_parms_t) - 1 + DOT11_OUI_LEN + len)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create "
		          "vendor-specific event\n", bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	parms = (vendor_specific_evt_parms_t *)evt->parms;
	parms->len = DOT11_OUI_LEN + len;
	bcopy(BRCM_PROP_OUI, &parms->parms[0], DOT11_OUI_LEN);
	bcopy(data, &parms->parms[DOT11_OUI_LEN], len);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

#ifdef VENDOR_SPECIFIC_EVENT
static void
wlc_bta_tx_hcidata_lost_event(bta_info_t *bta, uint16 llh)
{
	uint8 data[3];

	if (!(bta->msglevel & BTA_HCI_DATALOST_MSG))
		return;

	data[0] = 1; /* possibly lost packet */
	data[1] = (uint8)llh;
	data[2] = (uint8)(llh >> 8);

	wlc_bta_vendor_specific_event(bta, data, sizeof(data));
}

static void
wlc_bta_unspecified_error_event(bta_info_t *bta, uint8 err)
{
	uint8 data[3];

	data[0] = 2; /* HCI status */
	data[1] = HCI_ERR_UNSPECIFIED;
	data[2] = err;

	wlc_bta_vendor_specific_event(bta, data, sizeof(data));
}

static void
wlc_bta_no_memory_event(bta_info_t *bta, uint8 err)
{
	uint8 data[3];

	data[0] = 2; /* HCI status */
	data[1] = HCI_ERR_MEMORY_FULL;
	data[2] = err;

	wlc_bta_vendor_specific_event(bta, data, sizeof(data));
}

static void
wlc_bta_no_channel_event(bta_info_t *bta, uint8 err)
{
	uint8 data[3];

	data[0] = 2; /* HCI status */
	data[1] = HCI_ERR_NO_SUITABLE_CHANNEL;
	data[2] = err;

	wlc_bta_vendor_specific_event(bta, data, sizeof(data));
}
#else
#define wlc_bta_tx_hcidata_lost_event(bta, llh) do {} while (FALSE)
#define wlc_bta_unspecified_error_event(bta, err) do {} while (FALSE)
#define wlc_bta_no_memory_event(bta, err) do {} while (FALSE)
#define wlc_bta_no_channel_event(bta, err) do {} while (FALSE)
#endif /* VENDOR_SPECIFIC_EVENT */

static void
wlc_bta_chan_event(bta_info_t *bta, chanspec_t chanspec)
{
	uint8 data[3];

	data[0] = 3; /* chanspec */
	data[1] = (uint8)chanspec;
	data[2] = (uint8)(chanspec >> 8);

	wlc_bta_vendor_specific_event(bta, data, sizeof(data));
}

/* tx complete (success or failure) callback */
static void
wlc_bta_tx_hcidata_complete(wlc_info_t *wlc, void *p, uint txstatus)
{
	bta_info_t *bta = wlc->bta;
	uint16 llh;

	/* generate Number of Completed Data Blocks event is the main goal,
	 * generate other events as they fit.
	 */

	ASSERT(WLPKTFLAG_BTA_HCI_ACL(WLPKTTAG(p)));

	llh = WLPKTTAG(p)->shared.bta.llh;

	/* frame is xmit'd (was tx'd and ack'd */
	if (txstatus & TX_STATUS_ACK_RCV)
		goto doevent;

	/* frame is 'lost' (wasn't ack'd and suppressed) */
	if ((txstatus & TX_STATUS_SUPR_MASK) == 0) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		uint tx_frame_count = (txstatus & TX_STATUS_FRM_RTX_MASK) >>
		        TX_STATUS_FRM_RTX_SHIFT;

		if (WLPKTTAG(p)->flags & WLF_USERTS) {
			uint tx_rts_count = (txstatus & TX_STATUS_RTS_RTX_MASK) >>
			        TX_STATUS_RTS_RTX_SHIFT;

			if (txstatus)
				WL_ERROR(("wl%d: %s: "
					"RTS retries %u frame retries %u, "
					"frame possibly lost on logical link %u\n",
					bta->wlc->pub->unit, __FUNCTION__,
					tx_rts_count, tx_frame_count, llh));
		}
		else {
			if (txstatus)
				WL_ERROR(("wl%d: %s: "
						  "frame retries %u, "
						  "frame possibly lost on logical link %u\n",
						  bta->wlc->pub->unit, __FUNCTION__, tx_frame_count,
						  llh));
		}
#endif /* BCMDBG || BCMDBG_ERR */

		wlc_bta_tx_hcidata_lost_event(bta, llh);
		goto doevent;
	}

	/* frame is suppressed */
	switch ((txstatus & TX_STATUS_SUPR_MASK) >> TX_STATUS_SUPR_SHIFT) {
	case TX_STATUS_SUPR_PMQ:
		/* frame is suppressed due to PM mode transition to ON, do nothing */
		WL_BTA(("wl%d: frame is suppressed due to PM transition, do nothing\n",
		        bta->wlc->pub->unit));
		return;

	case TX_STATUS_SUPR_EXPTIME:
		/* frame is suppressed due to lifetime expiration */
		wlc_bta_flush_hcidata_occurred_event(bta, llh);
		break;

	default:
		break;
	}

doevent:
	wlc_bta_tx_hcidata_complete_event(bta, llh, FALSE);
}

/* XXX reserve 'void *' worth of bytes in front of the event for BtKern cookie,
 * which is inserted in BtKernForwardEvent().
 */
#define BTKERN_COOKIE_SIZE	sizeof(void *)

static amp_hci_event_t *
wlc_bta_alloc_hcievent(bta_info_t *bta, uint8 ecode, uint8 plen)
{
	uint8 *buf;
	uint buf_size = BTKERN_COOKIE_SIZE + HCI_EVT_PREAMBLE_SIZE + plen;
	amp_hci_event_t *evt;

	if ((buf = MALLOCZ(bta->wlc->osh, buf_size)) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          bta->wlc->pub->unit, __FUNCTION__, MALLOCED(bta->wlc->osh)));
		return NULL;
	}

	evt = (amp_hci_event_t *)(buf + BTKERN_COOKIE_SIZE);

	evt->ecode = ecode;
	evt->plen = plen;

	return evt;
}

/* XXX evt pointer is 'void *' bytes past the allocated memory.
 * How can we be sure we are safe to just adjust the pointer?
 */
static void
wlc_bta_free_hcievent(bta_info_t *bta, amp_hci_event_t *evt)
{
	MFREE(bta->wlc->osh, (void *)((uint8 *)evt - BTKERN_COOKIE_SIZE),
	      BTKERN_COOKIE_SIZE + HCI_EVT_PREAMBLE_SIZE + evt->plen);
}

/* defer or generate Number of Completed Data Blocks events */
#define BTA_MAXDATABLKS_DLY	(BTA_MAXDATABLKS / 2)

static void
wlc_bta_tx_hcidata_complete_event(bta_info_t *bta, uint16 llh, bool flush)
{
	/* XXX DHD locally generates the completion event to in order to save
	 * bus transaction time.
	 */
#ifndef DONGLEBUILD
	bool generate = FALSE;
	bool invllh = FALSE;

	if (flush) {
		if (bta->ll[llh].datablks_complete > 0) {
			generate = TRUE;
			goto gen_evt;
		}
		return;
	}

	/* llh may be invalid at this point if it was wrong when passed in or
	 * if the logical link has been disconnected.
	 */

	ASSERT(bta->datablks_pending >= 1);
	bta->datablks_pending --;

	bta->datablks_complete ++;

	if (wlc_bta_valid_logical_link(bta, llh) == HCI_SUCCESS)
		bta->ll[llh].datablks_complete ++;
	else {
		generate = TRUE;
		invllh = TRUE;
	}

	/* defer the event as much as possible to save some resources */
	if (!generate &&
	    (bta->datablks_pending == 0 ||
	     bta->datablks_complete >= BTA_MAXDATABLKS_DLY))
		generate = TRUE;

gen_evt:
	/* generate Number of Completed Data Blocks event */
	if (generate || flush) {
		num_completed_data_blocks_evt_parms_t *parms;
		amp_hci_event_t *evt;
		uint16 llidx;
		uint8 lls;

		/* collapse all individual blocks completes into a single event... */

		for (llidx = 0, lls = 0; llidx < BTA_MAXLOGLINKS; llidx ++) {
			if (bta->ll[llidx].plh != 0 &&
			    bta->ll[llidx].datablks_complete > 0)
				lls ++;
		}
		if (invllh)
			lls ++;

		if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Number_of_Completed_Data_Blocks,
		                sizeof(num_completed_data_blocks_evt_parms_t) +
		                sizeof(data_blocks_t) * (lls - 1))) == NULL) {
			WL_ERROR(("wl%d: %s: failed to create "
			          "event\n", bta->wlc->pub->unit, __FUNCTION__));
			return;
		}

		parms = (num_completed_data_blocks_evt_parms_t *)evt->parms;
		htol16_ua_store(BTA_MAXDATABLKS, (uint8 *)&parms->num_blocks);
		parms->num_handles = lls;
		for (llidx = 0, lls = 0; llidx < BTA_MAXLOGLINKS; llidx ++) {
			if (bta->ll[llidx].plh != 0 &&
			    bta->ll[llidx].datablks_complete > 0) {
				htol16_ua_store(llidx, (uint8 *)&parms->completed[lls].handle);
				htol16_ua_store(1, (uint8 *)&parms->completed[lls].pkts);
				htol16_ua_store(bta->ll[llidx].datablks_complete,
				                (uint8 *)&parms->completed[lls].blocks);
				lls ++;
			}
		}
		if (invllh) {
			htol16_ua_store(llh, (uint8 *)&parms->completed[lls].handle);
			htol16_ua_store(1, (uint8 *)&parms->completed[lls].pkts);
			htol16_ua_store(1, (uint8 *)&parms->completed[lls].blocks);
		}

		wlc_bta_doevent(bta, evt);

		wlc_bta_free_hcievent(bta, evt);

		/* reset counters */
		for (llidx = 0; llidx < BTA_MAXLOGLINKS; llidx ++) {
			if (bta->ll[llidx].plh != 0 &&
			    bta->ll[llidx].datablks_complete > 0) {
				ASSERT(bta->datablks_complete >= bta->ll[llidx].datablks_complete);
				bta->datablks_complete -= bta->ll[llidx].datablks_complete;
				bta->ll[llidx].datablks_complete = 0;
			}
		}
		if (invllh) {
			ASSERT(bta->datablks_complete >= 1);
			bta->datablks_complete -= 1;
		}
		if (bta->datablks_complete != 0) {
			/* under a unknown condition, we do not cover
			 * all commpleted blocks due to event/packet
			 * got lost.  This need debug futher
			 */
			WL_BTA(("wl%d: %s: bta->datablks_complete %d flush %d\n",
				bta->wlc->pub->unit, __FUNCTION__, bta->datablks_complete, flush));
			bta->datablks_complete = 0;
		}
		ASSERT(bta->datablks_complete == 0);
	}
#endif /* DONGLEBUILD */
}

/* Add Ethernet and 802.11 PAL LLC/SNAP headers */
static void
wlc_bta_add_hdrs(bta_pl_t *pl, void *p, uint16 type)
{
	struct ether_header *eh;
	struct dot11_llc_snap_header *lsh;
	uint16 len;
	wlc_bsscfg_t *bsscfg;
	struct scb *scb;

	scb = pl->scb;
	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	len = (uint16)PKTLEN(bsscfg->wlc->osh, p);

	PKTPUSH(bsscfg->wlc->osh, p, RFC1042_HDR_LEN);

	eh = (struct ether_header *)PKTDATA(bsscfg->wlc->osh, p);
	bcopy(&scb->ea, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy(&bsscfg->cur_etheraddr, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(DOT11_LLC_SNAP_HDR_LEN + len);

	lsh = (struct dot11_llc_snap_header *)&eh[1];
	bcopy(BT_SIG_SNAP_MPROT, lsh, DOT11_LLC_SNAP_HDR_LEN - 2);
	lsh->type = hton16(type);
}

/* Transmit a HCI ACL data packet encapsulated in BT-SIG frame.
 * Input format:
 *	hci-acl-data header
 *	hci-acl-data
 * Output format:
 *	ether_header
 *	bt-sig-llc-snap header
 *	hci-acl-data header
 *	hci-acl-data
 * Note: The packet will go through the send path during which the hci-acl-data header
 *       will be removed by wlc_bta_send_proc() before transmission.
 */
static void
wlc_bta_send_hcidata_pkt(bta_info_t *bta, uint16 llh, void *p)
{
	wlc_info_t *wlc = bta->wlc;
	bta_ll_t *ll;
	bta_pl_t *pl;

	ll = &bta->ll[llh];
	pl = &bta->pl[ll->plidx];

	/* add Ethernet header with 802.11 PAL LLC/SNAP header */
	wlc_bta_add_hdrs(pl, p, BTA_PROT_L2CAP);

	/* submit to wl tx path */
	wlc_sendpkt(wlc, p, SCB_WDS(pl->scb));
}

/* Process the BT-SIG packet being transmitted.
 *   Input format:
 *     ether header
 *     bt-sig-llc-snap header
 *     payload data
 * HCI ACL data payload: Remove HCI ACL data header before going through 802.11 processing.
 *   Input format:
 *     hci-acl-data header
 *     hci-acl-data
 *   Output format:
 *     hci-acl-data
 * Type 0 (undefined, used to encapsulate HCI cmd when DA is locally admin'd address) payload:
 * Route the HCI cmd to ioctl handler.
 *   The packet will not be transmitted.
 * Security data payload:
 *   No processing, transmit as is.
 * Activity report payload:
 *   No processing, transmit as is.
 * Link Supervision Request payload:
 *   No processing, transmit as is.
 * Link Supervision Reply payload:
 *   No processing, transmit as is.
 * Other payloads:
 *   The packet will not be transmitted.
 *
 * Update the wlcif pointer for HCI ACL data and Security packets as well since
 * these two types of frames could come from outside the wlc_bta.c module and the
 * wlcif pointer may not be correct.
 *
 * Make sure the routine is called at the very beginning of the tx processing path.
 *
 * Return TRUE to indicate the frame has been consumed and should not be transmitted
 * hence free the packet upon return from this function.
 */
bool
wlc_bta_send_proc(bta_info_t *bta, void *p, wlc_if_t **wlcif)
{
	wlc_info_t *wlc = bta->wlc;
	struct ether_header *eh;
	struct dot11_llc_snap_header *lsh;
	bta_pl_t *pl;
	bool toss = FALSE;
	uint16 type;
	struct scb *scb;
	scb_bta_t *scb_bta;

	ASSERT(PKTLEN(wlc->osh, p) >= RFC1042_HDR_LEN);

	eh = (struct ether_header *)PKTDATA(wlc->osh, p);
	lsh = (struct dot11_llc_snap_header *)&eh[1];

	type = ntoh16(lsh->type);

	/* HCI cmd */
	/* XXX encapsulated HCI cmd in BT-SIG LLC/SNAP format with value 0 in PID field
	 * and locally administered address in DA field.
	 */
	if (type == 0 && ETHER_IS_LOCALADDR(eh->ether_dhost)) {

		wlc_bta_docmd((void *)bta, (uint8 *)&lsh[1], PKTLEN(wlc->osh, p) - RFC1042_HDR_LEN);

		return TRUE;
	}

	switch (type) {
	case BTA_PROT_L2CAP: {

		amp_hci_ACL_data_t *ACL_data = (amp_hci_ACL_data_t *)&lsh[1];
		uint16 handle = ltoh16(ACL_data->handle);
		uint16 llh = HCI_ACL_DATA_HANDLE(handle);
		bta_ll_t *ll;

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON())
			wlc_bta_hcidump_ACL_data(bta, ACL_data, TRUE);
#endif // endif

		/* register the packet callback early so that no matter what happens the packet
		 * callback is invoked.
		 */
		WLF2_PCB1_REG(p, WLF2_PCB1_BTA_HCI_ACL);
		WLPKTTAG(p)->shared.bta.llh = llh;

		/* optimize the data completion event generation */
		bta->datablks_pending ++;

		/* get logical link */
		if (wlc_bta_valid_logical_link(bta, llh) != HCI_SUCCESS) {
			WL_INFORM(("wl%d: dropping packet sent on invalid logical link %u\n",
			           wlc->pub->unit, llh));
			toss = TRUE;
			break;
		}
		ll = &bta->ll[llh];

		/* drop packet if txflow spec has service type = no_traffic */
		if (ll->prio == PRIO_8021D_NONE) {
			WL_INFORM(("wl%d: dropping packet sent on no traffic logical link %u\n",
			           wlc->pub->unit, llh));
			toss = TRUE;
			break;
		}

		/* remove HCL ACL data header and fix up Ethernet headers */
		PKTPULL(wlc->osh, p, RFC1042_HDR_LEN + HCI_ACL_DATA_PREAMBLE_SIZE);
		pl = &bta->pl[ll->plidx];
		wlc_bta_add_hdrs(pl, p, BTA_PROT_L2CAP);

		/* RTS/CTS protection */
		if (pl->flags & BTA_PL_USE_RTS)
			WLPKTTAG(p)->flags |= WLF_USERTS;

		/* enable short range mode */
		if (pl->short_range != 0)
			WLPKTTAG(p)->flags |= WLF_BTA_SRM;

		/* mark packet priority */
		PKTSETPRIO(p, ll->prio);

		/* set packet lifetime */
		if (ll->fto != BTA_INVFTO)
			wlc_lifetime_set(wlc, p, ll->fto);

		/* route the frame to the wlcif representing the phy link */
		*wlcif = SCB_WDS(pl->scb);

		/* continue wl tx processing... */
		break;
	}

	case BTA_PROT_ACTIVITY_REPORT:

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			scb = wlc_scbfindband(wlc, bta->bsscfg,
				(struct ether_addr *)eh->ether_dhost,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
			scb_bta = SCB_BTA(bta, scb);
			WL_BTA(("wl%d: sending Activity Report on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* continue wl tx processing... */
		break;

	case BTA_PROT_SECURITY:

		/* find the scb from DA */
		scb = wlc_scbfindband(wlc, bta->bsscfg,
			(struct ether_addr *)eh->ether_dhost,
			CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
		if (scb == NULL) {
#if defined(BCMDBG)
			char eabuf[ETHER_ADDR_STR_LEN];
			WL_ERROR(("wl%d: %s: no scb for %s\n", wlc->pub->unit, __FUNCTION__,
			          bcm_ether_ntoa((struct ether_addr *)eh->ether_dhost, eabuf)));
#endif // endif
			toss = TRUE;
			break;
		}

		scb_bta = SCB_BTA(bta, scb);
		pl = &bta->pl[scb_bta->plidx];

		/* RTS/CTS protection */
		if (pl->flags & BTA_PL_USE_RTS)
			WLPKTTAG(p)->flags |= WLF_USERTS;

		/* mark packet as 802.1x (since ether_type will be hard to detect) */
		WLPKTTAG(p)->flags |= WLF_8021X;

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: sending BT-SIG 802.1x frame on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* route the frame to the wlcif representing the phy link */
		*wlcif = SCB_WDS(pl->scb);

		/* continue wl tx processing... */
		break;

	case BTA_PROT_LINK_SUPERVISION_REQUEST:

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			scb = wlc_scbfindband(wlc, bta->bsscfg,
				(struct ether_addr *)eh->ether_dhost,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
			scb_bta = SCB_BTA(bta, scb);
			WL_BTA(("wl%d: sending Link Supervision Request on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* continue wl tx processing... */
		break;

	case BTA_PROT_LINK_SUPERVISION_REPLY:

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			scb = wlc_scbfindband(wlc, bta->bsscfg,
				(struct ether_addr *)eh->ether_dhost,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
			scb_bta = SCB_BTA(bta, scb);
			WL_BTA(("wl%d: sending Link Supervision Reply on physical link 0x%x\n",
			        wlc->pub->unit, bta->plh[scb_bta->plidx]));
			prhex(NULL, PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
			WL_BTA(("\n"));
		}
#endif // endif

		/* continue wl tx processing... */
		break;

	default:

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON()) {
			WL_BTA(("wl%d: wlc_bta_send_proc: unrecognized protocol: 0x%04x\n",
			        wlc->pub->unit, type));
			prhex(NULL, PKTDATA(wlc->osh, p), PKTLEN(wlc->osh, p));
			WL_BTA(("\n"));
		}
#endif // endif

		toss = TRUE;
		break;
	}

	/* toss! */
	return toss;
}

void
wlc_bta_tx_hcidata(void *handle, uint8 *data_buf, uint data_len)
{
	bta_info_t *bta = (bta_info_t *)handle;
	amp_hci_ACL_data_t *ACL_data = (amp_hci_ACL_data_t *)data_buf;
	uint16 dhandle = ltoh16(ACL_data->handle);
	uint16 dlen = ltoh16(ACL_data->dlen);
	wlc_info_t *wlc = bta->wlc;
	uint16 llh;
	void *pkt;
	void *buf;

	if (!BTA_ENAB(wlc->pub)) {
		WL_ERROR(("wl%d: %s: PAL is disabled\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* get logical link */
	llh = HCI_ACL_DATA_HANDLE(dhandle);
	if (wlc_bta_valid_logical_link(bta, llh) != HCI_SUCCESS) {
		WL_BTA(("wl%d: dropping packet sent on invalid logical link %u\n",
		        wlc->pub->unit, llh));
		return;
	}

	/* get an OSL packet w/ appropriate headroom */
	if (!(pkt = PKTGET(wlc->osh,
		TXOFF + RFC1042_HDR_LEN + HCI_ACL_DATA_PREAMBLE_SIZE + dlen, TRUE))) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
		          wlc->pub->unit, __FUNCTION__, dlen));
		return;
	}

#if defined(BCMDBG) || defined(WLMSG_BTA)
	bta->acl_bytes += data_len;
	bta->acl_pkts++;
#endif // endif

	/* copy HCI ACL data header and ACL data at this offset */
	buf = PKTPULL(wlc->osh, pkt, TXOFF + RFC1042_HDR_LEN);

	/* copy in HCI ACL data header and ACL data */
	bcopy((uint8 *)ACL_data, buf, HCI_ACL_DATA_PREAMBLE_SIZE + dlen);

	/* send packet */
	wlc_bta_send_hcidata_pkt(bta, llh, pkt);
}

/* AMP HCI event processing */
static bool
wlc_bta_event_enabled(bta_info_t *bta, uint16 evt_mask, uint8 ecode)
{
	switch (ecode) {
	case HCI_Physical_Link_Complete:
		return ((evt_mask & HCI_Physical_Link_Complete_Event_Mask) != 0);
	case HCI_Channel_Select:
		return ((evt_mask & HCI_Channel_Select_Event_Mask) != 0);
	case HCI_Disconnect_Physical_Link_Complete:
		return ((evt_mask & HCI_Disconnect_Physical_Link_Complete_Event_Mask) != 0);
	case HCI_Logical_Link_Complete:
		return ((evt_mask & HCI_Logical_Link_Complete_Event_Mask) != 0);
	case HCI_Disconnect_Logical_Link_Complete:
		return ((evt_mask & HCI_Disconnect_Logical_Link_Complete_Event_Mask) != 0);
	case HCI_Flow_Spec_Modify_Complete:
		return ((evt_mask & HCI_Flow_Spec_Modify_Complete_Event_Mask) != 0);
	case HCI_Number_of_Completed_Data_Blocks:
		return ((evt_mask & HCI_Number_of_Completed_Data_Blocks_Event_Mask) != 0);
	case HCI_Short_Range_Mode_Change_Complete:
		return ((evt_mask & HCI_Short_Range_Mode_Change_Complete_Event_Mask) != 0);
	case HCI_Status_Change_Event:
		return ((evt_mask & HCI_Status_Change_Event_Mask) != 0);
	case HCI_Command_Complete:
	case HCI_Command_Status:
	case HCI_Flush_Occurred:
	case HCI_Enhanced_Flush_Complete:
	case HCI_Vendor_Specific:
		return TRUE;
	default:
		return FALSE;
	}
}

#if defined(BCMDBG) || defined(WLMSG_BTA)
static const struct {
	uint8 evtval;
	char *evtstr;
} evt_map[] = {
	{ HCI_Command_Complete, "Command Complete" },
	{ HCI_Command_Status, "Command Status" },
	{ HCI_Flush_Occurred, "Flush Occurred" },
	{ HCI_Enhanced_Flush_Complete, "Enhanced Flush Complete" },
	{ HCI_Physical_Link_Complete, "Physical Link Complete" },
	{ HCI_Channel_Select, "Channel Select" },
	{ HCI_Disconnect_Physical_Link_Complete, "Disconnect Physical Link Complete" },
	{ HCI_Logical_Link_Complete, "Logical Link Complete" },
	{ HCI_Disconnect_Logical_Link_Complete, "Disconnect Logical Link Complete" },
	{ HCI_Flow_Spec_Modify_Complete, "Flow Spec Modify Complete" },
	{ HCI_Number_of_Completed_Data_Blocks, "Number of Completed Data Blocks" },
	{ HCI_Short_Range_Mode_Change_Complete, "Short Range Mode Change Complete" },
	{ HCI_Status_Change_Event, "Status Change Event" },
	{ HCI_Vendor_Specific, "Vendor Specific" }
};

static char *
evt2str(uint8 evt, char *buf)
{
	uint i;

	sprintf(buf, "Unknown");
	for (i = 0; i < ARRAYSIZE(evt_map); i++) {
		if (evt == evt_map[i].evtval) {
			sprintf(buf, evt_map[i].evtstr);
		}
	}

	return buf;
}

static void
wlc_bta_hcidump_evt(bta_info_t *bta, amp_hci_event_t *event)
{
	wlc_info_t *wlc = bta->wlc;
	char buf[34];

	if ((bta->msglevel & BTA_HCI_EVENTS_MSG) &&
	    wlc_bta_event_enabled(bta, bta->evt_prn_mask, event->ecode)) {
		WL_BTA(("wl%d: > HCI Event: %s(0x%x) plen %d\n", wlc->pub->unit,
			evt2str(event->ecode, buf), event->ecode, event->plen));
		prhex(NULL, event->parms, event->plen);
		WL_BTA(("\n"));
	}
}
#endif /* BCMDBG || WLMSG_BTA */

static void
wlc_bta_doevent(bta_info_t *bta, amp_hci_event_t *event)
{
	if (wlc_bta_event_enabled(bta, bta->evt_mask_2, event->ecode)) {
		uint event_size = HCI_EVT_PREAMBLE_SIZE + event->plen;

#if defined(BCMDBG) || defined(WLMSG_BTA)
		if (WL_BTA_ON())
			wlc_bta_hcidump_evt(bta, event);
#endif // endif

		if (!(bta->flags & BTA_FLAGS_ET_RX)) {
#ifdef WLBTWUSB
			/* Forward data to BTKRNL */
			WL_BTA_OFF(("wl%d: forwarding HCI event to BTKRNL\n", bta->wlc->pub->unit));
			/*
			 * XXX - Per code above, this should go through event path and per-port
			 * event processing code should make this call...
			 * This also needs to be asynchronous due to re-entrancy.  Victor says
			 * the BT stack can handle events delivered out of order relative to data.
			 */
			Bta_queue_event((void *)bta->wlc->wl, (uint8 *)event, event_size);
#endif // endif
		}
		else {
			/* Forward event to upper stack */
			wlc_mac_event(bta->wlc, WLC_E_BTA_HCI_EVENT, NULL, 0, 0, 0,
			              event, event_size);
		}
	}
}

static void
wlc_bta_cmd_status_event(bta_info_t *bta, uint16 op, uint8 status)
{
	amp_hci_event_t *evt;
	cmd_status_parms_t *parms;

	/* generate Command Status event */
	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Command_Status,
	                sizeof(cmd_status_parms_t))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	parms = (cmd_status_parms_t *)evt->parms;
	parms->status = status;
	parms->cmdpkts = 1;
	htol16_ua_store(op, (uint8 *)&parms->opcode);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_phy_link_complete_event(bta_info_t *bta, uint8 *buf, uint buflen, bool disconnect)
{
	amp_hci_event_t *evt;

	/* generate Disconnect Physical Link Complete/Physical Link Complete event */
	if ((evt = wlc_bta_alloc_hcievent(bta, disconnect ?
	                        HCI_Disconnect_Physical_Link_Complete :
	                        HCI_Physical_Link_Complete,
	                (uint8)buflen)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	bcopy(buf, evt->parms, buflen);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_log_link_event(bta_info_t *bta, uint8 ecode, uint8 *buf, uint buflen)
{
	amp_hci_event_t *evt;

	if ((evt = wlc_bta_alloc_hcievent(bta, ecode, (uint8)buflen)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	bcopy(buf, evt->parms, buflen);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_cmd_complete_event(bta_info_t *bta, uint16 op, uint8 *buf, uint buflen)
{
	amp_hci_event_t *evt;
	cmd_complete_parms_t *parms;

	/* generate Command Complete event */
	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Command_Complete,
	                (uint8)(OFFSETOF(cmd_complete_parms_t, parms) + buflen))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	parms = (cmd_complete_parms_t *)evt->parms;
	parms->cmdpkts = 1;
	htol16_ua_store(op, (uint8 *)&parms->opcode);
	bcopy(buf, parms->parms, buflen);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_status_change_event(bta_info_t *bta, uint8 status, uint8 amp_status)
{
	amp_hci_event_t *evt;
	status_change_evt_parms_t *parms;

	if (!BTA_ENAB(bta->wlc->pub)) {
		WL_ERROR(("wl%d: %s: PAL is disabled\n",
			bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* generate Status Change event */
	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Status_Change_Event,
		sizeof(status_change_evt_parms_t))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
			bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	parms = (status_change_evt_parms_t *)evt->parms;
	parms->status = status;
	parms->amp_status = amp_status;

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_amp_status_upd(bta_info_t *bta, uint8 state)
{
	wlc_bta_state_log(bta, (uint8)HCIAmpStateChange);
	if (state != bta->amp_state) {
		WL_BTA(("wl%d: wlc_bta_amp_status_upd: AMP Radio State %d\n",
			bta->wlc->pub->unit, state));
		bta->amp_state = state;
		wlc_bta_status_change_event(bta, 0, state);
	}
}

void
wlc_bta_radio_status_upd(bta_info_t *bta)
{
	uint8 radio_state = 0;
	mbool status;

	wlc_ioctl(bta->wlc, WLC_GET_RADIO, &status, sizeof(status), NULL);
	radio_state = (status & (WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE)) ?
		WLC_BTA_RADIO_DISABLE : WLC_BTA_RADIO_ENABLE;
	/* amp state is RADIO_DISABLE & new state is RADIO_ENABLE then process it
	 * amp state is NOT RADIO_DISABLE & new state is RADIO_DISABLE then process it
	 *  otherwise don't process it
	 */
	if ((radio_state == WLC_BTA_RADIO_ENABLE && bta->amp_state == WLC_BTA_RADIO_DISABLE) ||
	    (radio_state == WLC_BTA_RADIO_DISABLE && bta->amp_state != WLC_BTA_RADIO_DISABLE))
		wlc_bta_amp_status_upd(bta, radio_state);
}

/* AMP HCI command processing */
#if defined(BCMDBG) || defined(WLMSG_BTA)
static const struct {
	uint16 opval;
	char *opstr;
} op_map[] = {
	{ HCI_Read_Logical_Link_Accept_Timeout, "Read Logical Link Accept Timeout" },
	{ HCI_Write_Logical_Link_Accept_Timeout, "Write Logical Link Accept Timeout" },
	{ HCI_Set_Event_Mask_Page_2, "Set Event Mask Page 2" },
	{ HCI_Read_Location_Data_Command, "Read Location Data Command" },
	{ HCI_Write_Location_Data_Command, "Write Location Data Command" },
	{ HCI_Read_Local_Version_Info, "Read Local Version Info" },
	{ HCI_Read_Local_Supported_Commands, "Read Local Supported Commands" },
	{ HCI_Read_Buffer_Size, "Read Buffer Size" },
	{ HCI_Read_Data_Block_Size, "Read Data Block Size" },
	{ HCI_Reset, "Reset" },
	{ HCI_Enhanced_Flush, "Enhanced Flush" },
	{ HCI_Read_Best_Effort_Flush_Timeout, "Read Best Effort Flush Timeout" },
	{ HCI_Write_Best_Effort_Flush_Timeout, "Write Best Effort Flush Timeout" },
	{ HCI_Read_Connection_Accept_Timeout, "Read Connection Accept Timeout" },
	{ HCI_Write_Connection_Accept_Timeout, "Write Connection Accept Timeout" },
	{ HCI_Read_Link_Supervision_Timeout, "Read Link Supervision Timeout" },
	{ HCI_Write_Link_Supervision_Timeout, "Write Link Supervision Timeout" },
	{ HCI_Read_Failed_Contact_Counter, "Read Failed Contact Counter" },
	{ HCI_Reset_Failed_Contact_Counter, "Reset Failed Contact Counter" },
	{ HCI_Read_Link_Quality, "Read Link Quality" },
	{ HCI_Read_Local_AMP_Info, "Read Local AMP Info" },
	{ HCI_Read_Local_AMP_ASSOC, "Read Local AMP ASSOC" },
	{ HCI_Write_Remote_AMP_ASSOC, "Write Remote AMP ASSOC" },
	{ HCI_Create_Physical_Link, "Create Physical Link" },
	{ HCI_Accept_Physical_Link_Request, "Accept Physical Link Request" },
	{ HCI_Disconnect_Physical_Link, "Disconnect Physical Link" },
	{ HCI_Create_Logical_Link, "Create Logical Link" },
	{ HCI_Accept_Logical_Link, "Accept Logical Link" },
	{ HCI_Disconnect_Logical_Link, "Disconnect Logical Link" },
	{ HCI_Logical_Link_Cancel, "Logical Link Cancel" },
	{ HCI_Flow_Spec_Modify, "Flow Spec Modify" },
	{ HCI_Short_Range_Mode, "Short Range Mode" }
};

static char *
op2str(uint16 op, char *buf)
{
	uint i;

	sprintf(buf, "Unknown");
	for (i = 0; i < ARRAYSIZE(op_map); i++) {
		if (op == op_map[i].opval) {
			sprintf(buf, op_map[i].opstr);
		}
	}

	return buf;
}

static void
wlc_bta_hcidump_cmd(bta_info_t *bta, amp_hci_cmd_t *cmd)
{
	wlc_info_t *wlc = bta->wlc;
	uint16 op = ltoh16_ua((uint8 *)&cmd->opcode);
	char buf[40];

	WL_BTA(("wl%d: < HCI Command: %s(0x%x|0x%x) plen %d\n", wlc->pub->unit,
		op2str(op, buf), HCI_CMD_OGF(op), HCI_CMD_OCF(op),
		cmd->plen));
	prhex(NULL, cmd->parms, cmd->plen);
	WL_BTA(("\n"));
}
#endif /* BCMDBG || WLMSG_BTA */

static uint8
wlc_bta_connect_phy_link(bta_info_t *bta, phy_link_cmd_parms_t *parms, uint8 plen, bool creator)
{
	bta_pl_t *phy_link;
	int plidx;
	wlc_info_t *wlc = bta->wlc;

	/* checking for any SoftAp exist */
	if (AP_ENAB(wlc->pub) && AP_ACTIVE(wlc) && !wlc_bta_active(bta)) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_AP_EXIST);
		WL_ERROR(("wl%d: softAP exist\n", bta->wlc->pub->unit));
		return HCI_ERR_UNSPECIFIED;
	}

	/* sanity check: all parameters present */
	if (plen < (OFFSETOF(phy_link_cmd_parms_t, key) + parms->key_length))
		return HCI_ERR_UNIT_KEY_USED;

	/* sanity check: key not too long */
	if (parms->key_length > 32)
		return HCI_ERR_UNSUPPORTED_VALUE;

	/* sanity check: physical link handle cannot be zero */
	if (parms->plh == 0)
		return HCI_ERR_UNSUPPORTED_VALUE;

	/* sanity check: physical link already exists */
	for (plidx = 0; plidx < WLC_MAXBTA; plidx++) {
		if (bta->plh[plidx] == parms->plh)
			break;
	}
	if (plidx < WLC_MAXBTA)
		return HCI_ERR_CONNECTION_EXISTS;

	/* allocate physical link */
	for (plidx = 0; plidx < WLC_MAXBTA; plidx++) {
		if (bta->plh[plidx] == 0)
			break;
	}
	if (plidx == WLC_MAXBTA)
		return HCI_ERR_MAX_NUM_OF_CONNECTIONS;

	if (!wlc->pub->up) {
		if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
		    mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE)) {
			WL_ERROR(("wl%d: %s: Radio Disabled\n",
				bta->wlc->pub->unit, __FUNCTION__));
			return HCI_ERR_UNSPECIFIED;
		}
	}

	/* update physical link handle */
	bta->plh[plidx] = parms->plh;

	phy_link = &bta->pl[plidx];
	bzero(phy_link, sizeof(bta_pl_t));

	/* use this counter along with numpls to tell if there is
	 * any physical links being setup...
	 */
	bta->numpls_allocated ++;

	/* set role */
	if (creator)
		phy_link->flags |= BTA_PL_CREATOR;

	/* set link key */
	phy_link->lk_type_len |= (((parms->key_type & BTA_PLK_TYPE_MASK) - 3) <<
		BTA_PLK_TYPE_SHIFT);
	phy_link->lk_type_len |= (parms->key_length & BTA_PLK_LENGTH_MASK);
	bcopy(parms->key, phy_link->link_key, parms->key_length);

	/* timestamp connection attempt */
	phy_link->ca_ts = bta->wlc->pub->now;
	WL_BTA(("wl%d: start CATO ticks %d; timeout in %dsec\n",
		wlc->pub->unit, phy_link->ca_ts, bta->ca_to));

	/* set default link supervision timeout */
	phy_link->ls_to = 0x3e80;

	return HCI_SUCCESS;
}

/*
 * Traverse a string of 1-byte tag/2-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
static uint8 *
wlc_bta_parse_tlvs(void *buf, int buflen, uint key)
{
	uint8 *elt;
	int totlen;

	elt = (uint8 *)buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= 3) {
		int len = ltoh16_ua(&elt[1]);

		/* validate remaining totlen */
		if ((elt[0] == key) && (totlen >= (len + 3)))
			return (elt);

		elt = &elt[len + 3];
		totlen -= (len + 3);
	}

	return NULL;
}

static void
wlc_bta_activate(bta_info_t *bta)
{
	wlc_info_t *wlc = bta->wlc;
#if WLBTAMP_D11RETRY > 7
	int SRL, LRL;
#endif // endif
	int infra;

#if WLBTAMP_D11RETRY > 7
	bta->SRL = (uint8)wlc->SRL;
	SRL = WLBTAMP_D11RETRY;
	wlc_ioctl(wlc, WLC_SET_SRL, &SRL, sizeof(SRL), NULL);
	bta->LRL = (uint8)wlc->LRL;
	LRL = WLBTAMP_D11RETRY / 2;
	wlc_ioctl(wlc, WLC_SET_LRL, &LRL, sizeof(LRL), NULL);
#endif // endif

	bta->infra = wlc->default_bss->infra;
	infra = 1;
	wlc_ioctl(wlc, WLC_SET_INFRA, &infra, sizeof(infra), NULL);

	/* save global states */
	bta->_apsta = wlc->pub->_apsta;
	wlc->pub->_apsta = TRUE;
	bta->_ap = wlc->pub->_ap;
	wlc->pub->_ap = TRUE;
}

static void
wlc_bta_deactivate(bta_info_t *bta)
{
	wlc_info_t *wlc = bta->wlc;
#if WLBTAMP_D11RETRY > 7
	int SRL, LRL;
#endif // endif
	int infra;

	/* restore global states */
	wlc->pub->_ap = bta->_ap;
	wlc->pub->_apsta = bta->_apsta;

	infra = bta->infra;
	wlc_ioctl(wlc, WLC_SET_INFRA, &infra, sizeof(infra), NULL);

#if WLBTAMP_D11RETRY > 7
	SRL = bta->SRL;
	wlc_ioctl(wlc, WLC_SET_SRL, &SRL, sizeof(SRL), NULL);
	LRL = bta->LRL;
	wlc_ioctl(wlc, WLC_SET_LRL, &LRL, sizeof(LRL), NULL);
#endif // endif
}

static void
wlc_bta_enable_btcx(bta_info_t *bta, bta_pl_t *pl, bool enable)
{
}

/* disconnect physical link and generate events based on 'ple' parm.
 * generate HCI_Disconnect_Physical_Link event if phy link is established otherwise
 * generate HCI_Physical_Link_Complete event. use 'dis' parm to force it to genearte
 * HCI_Disconnect_Physical_Link event. pass 'reason' as reason to HCI_Disconnect_Physical_Link
 * event and pass 'reason' as status to HCI_Physical_Link_Complete event.
 * disconnect all still-connected logical links and generate events based on 'lle' parm.
 */
static int
wlc_bta_phy_link_dodisconnect(bta_info_t *bta, int plidx, uint8 reason,
	bool ple, bool lle, bool dis)
{
	int callbacks = 0;
	uint8 plh = bta->plh[plidx];
	uint16 llidx;
	uint8 flags;

	/* disconnect any still-connected logical links and generate events if lle is TRUE */
	for (llidx = 0; llidx < BTA_MAXLOGLINKS; llidx ++) {
		if (bta->ll[llidx].plh == plh) {
			wlc_bta_disconnect_logical_link(bta, llidx, reason, lle);
		}
	}

	/* cache bta->pl[plidx].flags, which is clear in wlc_bta_dodisconnect() */
	flags = bta->pl[plidx].flags;

#ifdef APCS
	/* abort ongoing Channel Select process */
	if (bta->cs != NULL) {
		wlc_scan_abort(bta->wlc->scan, WLC_E_STATUS_CS_ABORT);
		/* wlc_bta_cs_complete() will call wlc_bta_dodisconnect() */
	}
	else
#endif // endif
	/* do disconnecting physical link */
	callbacks = wlc_bta_dodisconnect(bta, plidx);

	/* generate Disconnect Physical Link Complete or Physical Link Complete event */
	if (ple) {
		if ((flags & BTA_PL_COMPLETE) || dis) {
			dis_phy_link_evt_parms_t dis_pl_evt_parms;

			dis_pl_evt_parms.status = HCI_SUCCESS;
			dis_pl_evt_parms.plh = plh;
			dis_pl_evt_parms.reason = reason;
			wlc_bta_phy_link_complete_event(bta, (uint8 *)&dis_pl_evt_parms,
			                                sizeof(dis_pl_evt_parms), TRUE);
		}
		else {
			phy_link_evt_parms_t pl_evt_parms;

			pl_evt_parms.status = reason;
			pl_evt_parms.plh = plh;
			wlc_bta_phy_link_complete_event(bta, (uint8 *)&pl_evt_parms,
			                                sizeof(pl_evt_parms), FALSE);
		}
	}

	return callbacks;
}

static bool
wlc_bta_scb_shared(bta_info_t *bta, int plidx, struct scb *scb)
{
	int idx;

	for (idx = 0; idx < WLC_MAXBTA; idx ++) {
		if (idx != plidx &&
		    bta->plh[idx] != 0 &&
		    bta->pl[idx].scb == scb)
			return TRUE;
	}

	return FALSE;
}

static int
wlc_bta_dodisconnect(bta_info_t *bta, int plidx)
{
	wlc_info_t *wlc = bta->wlc;
	bta_pl_t *phy_link;
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	int callbacks = 0;

	if (bta->plh[plidx]) {
		bta->numpls_allocated --;
		/* clear physical link handle */
		bta->plh[plidx] = 0;
	} else
		return callbacks;

	/* tear down the supporting state structs */
	/*
	 * XXX - What do we need to do here?
	 *       1) clear WDS state (but that frees scb)
	 *       2) clear scb state
	 *       3) cleanup bsscfg
	 */
	phy_link = &bta->pl[plidx];

	if ((scb = phy_link->scb) != NULL) {

		wlc_bta_enable_btcx(bta, phy_link, FALSE);

		/* force to free the scb */
		scb->permanent = FALSE;

		/* free the scb for creator, the container bsscfg will be freed only
		 * when there is no other physical links existing.
		 */
		if ((bsscfg = SCB_BSSCFG(scb)) == NULL) {
			ASSERT(bsscfg);
			return callbacks;
		} else if (phy_link->flags & BTA_PL_CREATOR) {
			struct maclist *maclist;
			uint len;
			uint i;

			len = sizeof(*maclist) + (MAXMACLIST - 1) * sizeof(struct ether_addr);
			maclist = MALLOC(wlc->osh, len);
			if (maclist != NULL) {
				wlc_ioctl(wlc, WLC_GET_MACLIST, maclist, len, bsscfg->wlcif);
				for (i = 0; i < maclist->count; i ++) {
					if (bcmp(&scb->ea, &maclist->ea[i], ETHER_ADDR_LEN) != 0)
						continue;
					bcopy(&maclist->ea[i + 1], &maclist->ea[i],
					      (maclist->count - i - 1) * ETHER_ADDR_LEN);
					maclist->count -= 1;
					break;
				}
				wlc_ioctl(wlc, WLC_SET_MACLIST, maclist, len, bsscfg->wlcif);
				MFREE(wlc->osh, maclist, len);
			}
			else {
				WL_ERROR(("wl%d: %s: "
					"failed to allocate maclist\n",
					wlc->pub->unit, __FUNCTION__));
			}
			wlc_scbfree(wlc, scb);
		}
		/* free the bsscfg for acceptor as well */
		else {
			wlc_assoc_abort(bsscfg);
			if (bsscfg->enable)
				wlc_bsscfg_disable(wlc, bsscfg);
			wlc_bsscfg_free(wlc, bsscfg);
			bta->bsscfg = NULL;
		}

		bta->numpls --;
	}

	/* free local and remote AMP_ASSOCs */
	if (phy_link->local)
		MFREE(wlc->osh, phy_link->local, phy_link->llen);
	if (phy_link->remote)
		MFREE(wlc->osh, phy_link->remote, phy_link->rlen);

	/* clear the rest of the physical link */
	bzero(phy_link, sizeof(bta_pl_t));

	/* Other clean ups when there are no physical links in the system */

	if (bta->numpls > 0)
		return 0;

	/* tear down creators' container/beaconing bsscfg */
	if ((bsscfg = bta->bsscfg_creator) != NULL) {
		if (bsscfg->enable)
			wlc_bsscfg_disable(wlc, bsscfg);

		wlc_bsscfg_free(wlc, bsscfg);
		bta->bsscfg_creator = NULL;
	}

	/* delete any Channel Select event delay timer */
	if (bta->cse_timer != NULL) {
		if (!wl_del_timer(wlc->wl, bta->cse_timer))
			callbacks ++;
		else {
			wl_free_timer(wlc->wl, bta->cse_timer);
			bta->cse_timer = NULL;
		}
	}

	/* restore global states */
	wlc_bta_deactivate(bta);

	bta->chanspec_sel = 0;

	wlc_set_wake_ctrl(wlc);

	return callbacks;
}

void
wlc_bta_scb_cleanup(bta_info_t *bta, struct scb *scb)
{
}

static uint8
wlc_bta_pllookup(bta_info_t *bta, uint8 plh, int *plidx)
{
	int index;

	if (plh == 0)
		return HCI_ERR_UNSUPPORTED_VALUE;

	for (index = 0; index < WLC_MAXBTA; index++) {
		if (bta->plh[index] == plh) {
			*plidx = index;
			return HCI_SUCCESS;
		}
	}

	return HCI_ERR_NO_CONNECTION;
}

static bool
wlc_bta_qos(bta_info_t *bta)
{
	return ((bta->flags & BTA_FLAGS_NO_QOS) ? FALSE : TRUE);
}

bool
wlc_bta_active(bta_info_t *bta)
{
	return (bta != NULL ? bta->numpls > 0 : FALSE);
}

bool
wlc_bta_inprog(bta_info_t *bta)
{
	return (bta != NULL ? bta->numpls_allocated > bta->numpls : FALSE);
}

bool
wlc_bta_frameburst_active(bta_info_t *bta, wlc_pkttag_t *pkttag, uint rate)
{
	if (WLPKTFLAG_BTA_HCI_ACL(pkttag) &&
	    bta->_fb && rate > WLC_FRAMEBURST_MIN_RATE)
		return TRUE;
	return FALSE;
}

static void
wlc_bta_phy_link_complete(bta_info_t *bta, struct scb *scb)
{
	phy_link_evt_parms_t evt_parms;
	scb_bta_t *scb_bta;
	bta_pl_t *pl;

	scb_bta = SCB_BTA(bta, scb);
	pl = &bta->pl[scb_bta->plidx];

	/* we are no longer in connecting process */
	pl->flags &= ~BTA_PL_CONN;

	/* generate Physical Link Complete event */
	evt_parms.status = HCI_SUCCESS;
	evt_parms.plh = scb_bta->plh;
	wlc_bta_phy_link_complete_event(bta, (uint8 *)&evt_parms, sizeof(evt_parms), FALSE);

	/* mark physical link as having been completed (regardless 'status') */
	pl->flags |= BTA_PL_COMPLETE;

	/* enable RTS/CTS protection */
	pl->flags |= BTA_PL_USE_RTS;

	/* initialize link supervision timer */
	pl->used = bta->wlc->pub->now;

	/* turn on BT coex protection */
	wlc_bta_enable_btcx(bta, pl, TRUE);

	wlc_bta_send_activity_report_pkt(bta, pl);
}

static uint8
wlc_bta_join(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uint8 *SSID, int SSID_len, uint8 *bssid, chanspec_t chanspec)
{
	wl_assoc_params_t assoc_params;

	memset(&assoc_params, 0, sizeof(wl_assoc_params_t));
	bcopy(bssid, (void *)&assoc_params.bssid, ETHER_ADDR_LEN);
	assoc_params.chanspec_list[0] = chanspec;
	assoc_params.chanspec_num = 1;

	wlc_join(wlc, bsscfg, SSID, SSID_len, NULL, &assoc_params, sizeof(assoc_params));

	return 0;
}

void
wlc_bta_assoc_status_upd(bta_info_t *bta, wlc_bsscfg_t *cfg, uint8 state)
{
	wlc_info_t *wlc = bta->wlc;

	WL_TRACE(("wl%d: wlc_bta_assoc_status_upd\n", wlc->pub->unit));
	/* operate on primary interface */
	if (cfg == wlc->cfg) {
		wlc_bta_amp_status_upd(bta, state);
	}
}

void
wlc_bta_join_complete(bta_info_t *bta, struct scb *scb, uint8 status)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg = NULL;

	wlc = bta->wlc;
	ASSERT(scb != NULL);
	if (scb != NULL) {
		bsscfg = SCB_BSSCFG(scb);
		ASSERT(bsscfg != NULL);
	}

	if (status == 0 && scb != NULL) {
#ifdef WDS
		/* configure BT-AMP peer for WDS */
		wlc_wds_create(wlc, scb, WDS_INFRA_BSS);
#endif /* WDS */
	}

	if (status != 0 || bsscfg == NULL || scb == NULL) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_JOIN_COMPLETE);
		WL_ERROR(("wl%d: %s: failed with error %d\n",
			wlc->pub->unit, __FUNCTION__, status));
	} else if (bta->flags & BTA_FLAGS_NO_SECURITY)
		wlc_bta_phy_link_complete(bta, scb);
	else {
		scb_bta_t *scb_bta;
		int plidx;

		scb_bta = SCB_BTA(bta, scb);
		ASSERT(scb_bta != NULL);

		plidx = scb_bta->plidx;

		if (!(bta->pl[plidx].flags & BTA_PL_CREATOR)) {
#if defined(BCMSUP_PSK)
			uint auth_ies_len;
			bool stat = FALSE;

			auth_ies_len = wlc->pub->bcn_tmpl_len;

			/* mutually derive keys */
			if (SUP_ENAB(wlc->pub)) {
				uint8 *auth_ies;
				uint8 *sup_ies;
				uint sup_ies_len;

				/* prep supplicant */
				wlc_find_sup_auth_ies(wlc->idsup, bsscfg, &sup_ies, &sup_ies_len,
				                      &auth_ies, &auth_ies_len);
				stat = wlc_set_sup(wlc->idsup, bsscfg, SUP_WPAPSK,
				                   sup_ies, sup_ies_len, auth_ies, auth_ies_len);
			}
			if (!stat) {
				WL_ERROR(("wl%d: %s: 4-way handshake "
				          "config problem\n", wlc->pub->unit, __FUNCTION__));
			}
#endif /* BCMSUP_PSK */
		}
	}
}

/* disconnect the physical link and generate events.
 * disconnect all still-connected logical links and generate events based on 'lle' parm.
 */
static void
wlc_bta_disconnect_physical_link(bta_info_t *bta, int plidx, uint8 reason,
	bool lle)
{
	bool completed = (bta->pl[plidx].flags & BTA_PL_COMPLETE) ? TRUE : FALSE;
	uint8 plh = bta->plh[plidx];

	WL_BTA(("wl%d: wlc_bta_disconnect_physical_link: disconnec physical link %d "
	        "reason %d generate HCI_Disconnect_Physical_Link event %d\n",
	        bta->wlc->pub->unit, bta->plh[plidx], reason, lle));

	wlc_bta_phy_link_dodisconnect(bta, plidx, reason, TRUE, lle, TRUE);

	/* generate Physical Link Complete event with error code 0x2 */
	if (!completed) {
		phy_link_evt_parms_t evt_parms;

		evt_parms.status = HCI_ERR_NO_CONNECTION;
		evt_parms.plh = plh;
		wlc_bta_phy_link_complete_event(bta, (uint8 *)&evt_parms, sizeof(evt_parms), FALSE);
	}
}

void
wlc_bta_assoc_complete(bta_info_t *bta, wlc_bsscfg_t *cfg)
{
	int plidx;
	uint8 status;
	wlc_info_t *wlc = bta->wlc;

	if (cfg == bta->wlc->cfg) {
		status = bta->wlc->cfg->associated ?
			WLC_BTA_AP_ASSOC : WLC_BTA_RADIO_ENABLE;
		WL_BTA(("wl%d: wlc_bta_assoc_complete: Change Radio Stat %d\n",
			wlc->pub->unit, status));
		wlc_bta_amp_status_upd(bta, status);
	}

	/* do following processing only when channel has changed */
	if (bta->chanspec_sel == 0 ||
	    bta->chanspec_sel == CH20MHZ_CHSPEC(wf_chspec_ctlchan(bta->wlc->home_chanspec)))
		return;

	/* disconnect the physical link and generate events */
	for (plidx = 0; plidx < WLC_MAXBTA; plidx ++) {
		if (bta->plh[plidx] == 0)
			continue;

		wlc_bta_phy_link_dodisconnect(bta, plidx, HCI_ERR_CHANNEL_MOVE, TRUE, TRUE, FALSE);
	}
	wlc->aps_associated = (uint8)AP_BSS_UP_COUNT(wlc);
	wlc_mac_bcn_promisc(wlc);
	WL_BTA(("wl%d: wlc_bta_assoc_complete: update associated status %d %d %d\n",
	        wlc->pub->unit, wlc->pub->associated, wlc->stas_associated, wlc->aps_associated));
}

static chanspec_t
wlc_bta_chan_select(bta_info_t *bta)
{

	/* use the same channel as existing creators and acceptors */
	if (wlc_bta_active(bta))
		return bta->chanspec_sel;
	/* use the same channel as existing connections */
	else if (bta->wlc->pub->associated)
		return CH20MHZ_CHSPEC(wf_chspec_ctlchan(bta->wlc->home_chanspec));
	/* use user specified channel */
	else if (bta->chanspec_user != 0)
		return bta->chanspec_user;

	/* use any channel in the supported bands */
	return 0;
}

static void
wlc_bta_send_activity_report_pkt(bta_info_t *bta, bta_pl_t *phy_link)
{
	wlc_info_t *wlc = bta->wlc;
	void *pkt;
	uint len;
	amp_hci_activity_report_t *activity_report;

	/* compute size of activity report triples */
	len = OFFSETOF(amp_hci_activity_report_t, data);
	if (bta->flags & BTA_FLAGS_QTP_AR)
		len += sizeof(amp_hci_activity_report_triple_t);

	/* get an OSL packet w/ appropriate headroom */
	if (!(pkt = PKTGET(wlc->osh, TXOFF + RFC1042_HDR_LEN + len, TRUE))) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
		          wlc->pub->unit, __FUNCTION__, len));
		return;
	}

	/* add payload at this location */
	PKTPULL(wlc->osh, pkt, TXOFF + RFC1042_HDR_LEN);

	/* fill in header and activity reports */
	activity_report = (amp_hci_activity_report_t *)PKTDATA(wlc->osh, pkt);
	activity_report->ScheduleKnown = HCI_AR_SCHEDULE_KNOWN;
	if (bta->flags & BTA_FLAGS_QTP_AR) {
		amp_hci_activity_report_triple_t *activity_report_triple;
		uint32 tsf_l, tsf_h;

		activity_report->NumReports = 1;

		activity_report_triple = (amp_hci_activity_report_triple_t *)activity_report->data;
		activity_report_triple->Duration = 20000;
		activity_report_triple->Periodicity = 50000;
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		activity_report_triple->StartTime = tsf_l;
	} else {
		activity_report->NumReports = 0;
	}

	/* add Ethernet header with 802.11 PAL LLC/SNAP header */
	wlc_bta_add_hdrs(phy_link, pkt, BTA_PROT_ACTIVITY_REPORT);

	/* RTS/CTS protection */
	if (phy_link->flags & BTA_PL_USE_RTS)
		WLPKTTAG(pkt)->flags |= WLF_USERTS;

	/* submit to wl tx path */
	wlc_sendpkt(wlc, pkt, SCB_WDS(phy_link->scb));
}

#if defined(BCMDBG) || defined(WLMSG_BTA)
static void
wlc_bta_send_activity_report_flag(bta_info_t *bta, bta_pl_t *phy_link, int flag)
{
	wlc_info_t *wlc = bta->wlc;
	void *pkt;
	uint len;
	amp_hci_activity_report_t *activity_report;

	/* compute size of activity report triples */
	len = OFFSETOF(amp_hci_activity_report_t, data);
	if (flag)
		len += sizeof(amp_hci_activity_report_triple_t);

	/* get an OSL packet w/ appropriate headroom */
	if (!(pkt = PKTGET(wlc->osh, TXOFF + RFC1042_HDR_LEN + len, TRUE))) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
		          wlc->pub->unit, __FUNCTION__, len));
		return;
	}

	/* add payload at this location */
	PKTPULL(wlc->osh, pkt, TXOFF + RFC1042_HDR_LEN);

	/* fill in header and activity reports */
	activity_report = (amp_hci_activity_report_t *)PKTDATA(wlc->osh, pkt);
	if (flag) {
		amp_hci_activity_report_triple_t *activity_report_triple;
		uint32 tsf_l, tsf_h;

		activity_report->ScheduleKnown = HCI_AR_SCHEDULE_KNOWN;
		activity_report->NumReports = 1;

		activity_report_triple = (amp_hci_activity_report_triple_t *)activity_report->data;
		activity_report_triple->Duration = 20000;
		activity_report_triple->Periodicity = 50000;
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		activity_report_triple->StartTime = tsf_l;
	} else {
		activity_report->ScheduleKnown = 0;
		activity_report->NumReports = 0;
	}

	/* add Ethernet header with 802.11 PAL LLC/SNAP header */
	wlc_bta_add_hdrs(phy_link, pkt, BTA_PROT_ACTIVITY_REPORT);

	/* RTS/CTS protection */
	if (phy_link->flags & BTA_PL_USE_RTS)
		WLPKTTAG(pkt)->flags |= WLF_USERTS;

	/* submit to wl tx path */
	wlc_sendpkt(wlc, pkt, SCB_WDS(phy_link->scb));
}
#endif /* BCMDBG || WLMSG_BTA */

void
wlc_bta_AKM_complete(bta_info_t *bta, struct scb *scb)
{
	ASSERT(scb != NULL);

	/* generate Physical Link Complete event */
	wlc_bta_phy_link_complete(bta, scb);
}

/* disconnect the logical link and generate event based on 'dce' parm */
static void
wlc_bta_disconnect_logical_link(bta_info_t *bta, uint16 llh, uint8 reason, bool lle)
{
	bta_ll_t *ll = &bta->ll[llh];

	wlc_bta_flush_hcidata(bta, llh);

	/* generate Number of Completed Data Blocks event if necessary */
	if (lle)
		wlc_bta_tx_hcidata_complete_event(bta, llh, TRUE);

	/* update aggregate requested bandwidth */
	bta->pl[ll->plidx].allocbw -= ll->reqbw;

	/* not much to do here... */
	bzero(ll, sizeof(bta_ll_t));

	/* generate Disconnect Logical Link event */
	if (lle) {
		disc_log_link_evt_parms_t dis_ll_evt_parms;

		dis_ll_evt_parms.status = HCI_SUCCESS;
		htol16_ua_store(llh, (uint8 *)&dis_ll_evt_parms.llh);
		dis_ll_evt_parms.reason = reason;

		wlc_bta_log_link_event(bta, HCI_Disconnect_Logical_Link_Complete,
		                       (uint8 *)&dis_ll_evt_parms, sizeof(dis_ll_evt_parms));
	}
}

static uint8
wlc_bta_valid_logical_link(bta_info_t *bta, uint16 llh)
{
	/* sanity check logical link handle */
	if (llh >= BTA_MAXLOGLINKS)
		return HCI_ERR_NO_CONNECTION;

	/* make sure logical link has been allocated */
	if (bta->ll[llh].plh == 0)
		return HCI_ERR_NO_CONNECTION;

	return HCI_SUCCESS;
}

static void
wlc_bta_cse_timer(void *arg)
{
	bta_info_t *bta = (bta_info_t *)arg;
	int plidx;

	for (plidx = 0; plidx < WLC_MAXBTA; plidx ++) {
		if (bta->plh[plidx] == 0)
			continue;
		if (bta->pl[plidx].scb == NULL)
			continue;
		if (bta->pl[plidx].flags & BTA_PL_CSE_PEND) {
			amp_hci_event_t *evt;

			/* generate channel select event */
			if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Channel_Select, 1)) == NULL) {
				WL_ERROR(("wl%d: %s: failed to create event\n",
				          bta->wlc->pub->unit, __FUNCTION__));
				return;
			}

			bta->pl[plidx].flags &= ~BTA_PL_CSE_PEND;

			evt->parms[0] = bta->plh[plidx];

			wlc_bta_doevent(bta, evt);

			wlc_bta_free_hcievent(bta, evt);
		}
	}

	if (bta->cse_timer != NULL) {
		wl_free_timer(bta->wlc->wl, bta->cse_timer);
		bta->cse_timer = NULL;
	}
}

static void
wlc_bta_flush_hcidata_occurred_event(bta_info_t *bta, uint16 llh)
{
	amp_hci_event_t *evt;
	flush_occurred_evt_parms_t *flush_parms;

	/* generate Flush Occurred event */
	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Flush_Occurred,
	                sizeof(flush_occurred_evt_parms_t))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	flush_parms = (flush_occurred_evt_parms_t *)evt->parms;
	htol16_ua_store(llh, (uint8 *)&flush_parms->handle);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_flush_hcidata_complete_event(bta_info_t *bta, uint16 llh)
{
	amp_hci_event_t *evt;
	eflush_complete_evt_parms_t *eflush_parms;

	/* generate Enhanced Flush Complete event */
	if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Enhanced_Flush_Complete,
	                sizeof(eflush_complete_evt_parms_t))) == NULL) {
		WL_ERROR(("wl%d: %s: failed to create event\n",
		          bta->wlc->pub->unit, __FUNCTION__));
		return;
	}

	eflush_parms = (eflush_complete_evt_parms_t *)evt->parms;
	htol16_ua_store(llh, (uint8 *)&eflush_parms->handle);

	wlc_bta_doevent(bta, evt);

	wlc_bta_free_hcievent(bta, evt);
}

static void
wlc_bta_flush_hcidata(bta_info_t *bta, uint16 llh)
{
	int prec;
	struct pktq *q;
#if defined(BCMDBG) || defined(WLMSG_BTA)
	int count = 0;
#endif // endif

	/* XXX WES: what queue should we use? Is there a way to associate a BTA connection
	 * with a bsscfg or wlcif?
	 */
	q = WLC_GET_TXQ(bta->wlc->active_queue);

	/* Walk through the txq and toss all HCI ACL data packets */
	PKTQ_PREC_ITER(q, prec) {
		void *head_pkt = NULL;

		while (pktq_ppeek(q, prec) != head_pkt) {
			void *pkt = pktq_pdeq(q, prec);
			wlc_pkttag_t *pkttag = WLPKTTAG(pkt);

			if (WLPKTFLAG_BTA_HCI_ACL(pkttag) &&
			    pkttag->shared.bta.llh == llh) {
				PKTFREE(bta->wlc->osh, pkt, TRUE);
				bta->ll[llh].datablks_complete ++;
				bta->datablks_complete ++;
#if defined(BCMDBG) || defined(WLMSG_BTA)
				count ++;
#endif // endif
				continue;
			}

			if (head_pkt == NULL)
				head_pkt = pkt;
			pktq_penq(q, prec, pkt);
		}
	}

#if defined(BCMDBG) || defined(WLMSG_BTA)
	WL_BTA(("wl%d: wlc_bta_flush_hcidata: freed %d packets\n",
	        bta->wlc->pub->unit, count));
#endif // endif
}

static uint8
wlc_bta_doconnect(bta_info_t *bta, int plidx, chanspec_t chanspec)
{
	struct scb *scb = NULL;
	scb_bta_t *scb_bta;
	int idx, macmode;
	wlc_bsscfg_t *bsscfg = NULL;
	struct ether_addr *ea;
	uint8 *mac;
	wsec_pmk_t pmk;
	bool defer_cse = FALSE;
	bta_pl_t *phy_link;
	wlc_info_t *wlc = bta->wlc;
	uint8 plh;
	uint8 chan;
	uint8 band;
	uint32 flags;
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_BTA)
	char chanbuf[CHANSPEC_STR_LEN];
#endif /* BCMDBG */

	phy_link = &bta->pl[plidx];
	plh = bta->plh[plidx];

	/* if creator, update local AMP_ASSOC */
	if (phy_link->flags & BTA_PL_CREATOR) {
		uint8 status;

		if (phy_link->local != NULL) {
			MFREE(wlc->osh, phy_link->local, phy_link->llen);
			phy_link->local = NULL;
		}

		status = wlc_bta_build_AMP_ASSOC(bta, chanspec, &phy_link->local, &phy_link->llen);
		if (status != HCI_SUCCESS) {
			WL_ERROR(("wl%d: %s: fail to build local AMP_ASSOC for "
			          "physical link %d\n", wlc->pub->unit, __FUNCTION__, plh));
			return status;
		}
	}

	chan = wf_chspec_ctlchan(chanspec);
	band = CHANNEL_BANDUNIT(wlc, chan);

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_CHAN);
		WL_ERROR(("wl%d: %s: invalid chanspec %s to build local AMP_ASSOC"
			" for physical link %d\n", wlc->pub->unit, __FUNCTION__,
			wf_chspec_ntoa_ex(chanspec, chanbuf), plh));
		return HCI_ERR_NO_SUITABLE_CHANNEL;
	}

	/* disable 11n for now until it becomes standard */
	flags = BTA_BSSCFG_FLAGS;
	if (bta->flags & BTA_FLAGS_NO_11N)
		flags |= WLC_BSSCFG_11N_DISABLE;

	/* prepare a "global" AP bsscfg for:
	 * - beaconing (per device, for creator as well as acceptor)
	 * - creators scbs' container
	 */
	if (bta->bsscfg_creator == NULL) {
		/* allocate bsscfg */
		idx = wlc_bsscfg_get_free_idx(wlc);
		/* BTAMP always secondary interface, idx 0 use by primary interface */
		if (idx == -1) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_BSSCFG);
			WL_ERROR(("wl%d: no free index for bsscfg\n", wlc->pub->unit));
			return HCI_ERR_UNSPECIFIED;
		}

		bsscfg = wlc_bsscfg_alloc(wlc, idx, flags, NULL, TRUE);
		if (bsscfg == NULL) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_ALLOC_BSSCFG);
			WL_ERROR(("wl%d: Cannot create bsscfg\n", wlc->pub->unit));
			return HCI_ERR_UNSPECIFIED;
		}
		else if (wlc_bsscfg_type_init(wlc, bsscfg, BSSCFG_TYPE_BTA)) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_INIT_BSSCFG);
			WL_ERROR(("wl%d: Cannot init bsscfg\n", wlc->pub->unit));
			wlc_bsscfg_free(wlc, bsscfg);
			return HCI_ERR_UNSPECIFIED;
		}

		bta->bsscfg_creator = bsscfg;

		/* set SSID */
		ea = &bsscfg->cur_etheraddr;

		bsscfg->SSID_len = (uint8)sprintf((char *)bsscfg->SSID, amp_ssid_format,
		                                  ea->octet[0], ea->octet[1], ea->octet[2],
		                                  ea->octet[3], ea->octet[4], ea->octet[5]);

		/* send up the channel to use for AMP connection to BTW */
		WL_BTA(("wl%d: set VSE chanspec to use 0x%02x\n", wlc->pub->unit, chanspec));
		wlc_bta_chan_event(bta, chanspec);

		/* set the channel (set again if already set) */
		wlc_set(wlc, WLC_SET_CHANNEL, chan);

		/* restrict associations */
		macmode = WLC_MACMODE_ALLOW;
		wlc_ioctl(wlc, WLC_SET_MACMODE, &macmode, sizeof(macmode), bsscfg->wlcif);

		/* reset glabal states. */
		wlc_bta_activate(bta);

		/* remember the channel, when it changes later
		 * for whatever reason for example non-AMP STA roam,
		 * compare it to determine if all physical links
		 * should be torn down...
		 */
		bta->chanspec_sel = chanspec;

		/* configure security settings... */
		if (!(bta->flags & BTA_FLAGS_NO_SECURITY)) {
			/* ...for association */
			bsscfg->WPA_auth = WPA2_AUTH_PSK;
			bsscfg->wsec = AES_ENABLED;
			bsscfg->wsec_restrict = TRUE;
			bsscfg->eap_restrict = TRUE;
		}

		/* ready, steady, go */
		if (wlc_bsscfg_enable(wlc, bsscfg)) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_ENABLE_BSSCFG);
			WL_ERROR(("wl%d: Cannot enable bsscfg\n", wlc->pub->unit));
			return HCI_ERR_UNSPECIFIED;
		}

		wlc_set_wake_ctrl(wlc);

		/* defer Channel Select event until we are capable of
		 * responding to probe request.
		 */
		/* XXX only need to defer the event when bringing up the first bsscfg
		 * when the MAC/PHY could be still in reset (or low power mode) and
		 * it takes time to get the MAC/PHY working normally.
		 */
		if (phy_link->flags & BTA_PL_CREATOR)
			defer_cse = TRUE;
	}
	else {
		if (bta->chanspec_sel != chanspec) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_DIFF_CHANSPEC);
			WL_ERROR(("wl%d: a different chanspec 0x%04x is selected "
			          "(we are currently on 0x%04x)\n", wlc->pub->unit,
			          chanspec, bta->chanspec_sel));
			return HCI_ERR_UNSPECIFIED;
		}
	}

	mac = wlc_bta_parse_tlvs(phy_link->remote, phy_link->rlen, BTA_TYPE_ID_MAC_ADDRESS);
	ASSERT(mac != NULL);

	/* use the "global" AP bsscfg for all creators */
	if (phy_link->flags & BTA_PL_CREATOR) {
		struct maclist *maclist;
		uint16 len;

		bsscfg = bta->bsscfg_creator;

		/* restrict associations */
		/* configure BT-AMP peer for association */
		len = sizeof(*maclist) + (MAXMACLIST - 1) * sizeof(struct ether_addr);
		maclist = MALLOC(wlc->osh, len);
		if (maclist == NULL) {
			wlc_bta_no_memory_event(bta, WLC_BTA_ERR_MAC_LIST);
			WL_ERROR(("wl%d: %s: failed to allocate "
			          "maclist\n", wlc->pub->unit, __FUNCTION__));
			return HCI_ERR_MEMORY_FULL;
		}
		wlc_ioctl(wlc, WLC_GET_MACLIST, maclist, len, bsscfg->wlcif);
		bcopy(&mac[3], &maclist->ea[maclist->count], ETHER_ADDR_LEN);
		maclist->count += 1;
		wlc_ioctl(wlc, WLC_SET_MACLIST, maclist, len, bsscfg->wlcif);
		MFREE(wlc->osh, maclist, len);
	}
	/* use a separate STA bsscfg for acceptor */
	else {
		/* allocate bsscfg */
		idx = wlc_bsscfg_get_free_idx(wlc);
		/* BTAMP always secondary interface, idx 0 use by primary interface */
		if (idx == -1) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_BSSCFG);
			WL_ERROR(("wl%d: no free index for bsscfg\n", wlc->pub->unit));
			return HCI_ERR_UNSPECIFIED;
		}

		bsscfg = wlc_bsscfg_alloc(wlc, idx, flags, NULL, FALSE);
		if (bsscfg == NULL) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_ALLOC_BSSCFG);
			WL_ERROR(("wl%d: Cannot create bsscfg\n", wlc->pub->unit));
			return HCI_ERR_UNSPECIFIED;
		}
		else if (wlc_bsscfg_type_init(wlc, bsscfg, BSSCFG_TYPE_BTA)) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_INIT_BSSCFG);
			WL_ERROR(("wl%d: Cannot init bsscfg\n", wlc->pub->unit));
			wlc_bsscfg_free(wlc, bsscfg);
			return HCI_ERR_UNSPECIFIED;
		}

		/* turn roam off */
		bsscfg->roam->off = TRUE;

		bsscfg->assoc->retry_max = 16;

		/* configure security settings... */
		if (!(bta->flags & BTA_FLAGS_NO_SECURITY)) {
			int set_sup = 1;
			(void)wlc_iovar_op(wlc, "sup_wpa", NULL, 0, &set_sup, sizeof(int),
				IOV_SET, bsscfg->wlcif);
			bsscfg->WPA_auth = WPA2_AUTH_PSK;
			bsscfg->wsec = AES_ENABLED;
			bsscfg->wsec_restrict = TRUE;
			bsscfg->eap_restrict = TRUE;
		}
	}

	/* find or create BT-AMP peer scb */
	if (!(scb = wlc_scblookupband(wlc, bsscfg, (struct ether_addr *)&mac[3], band))) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_FIND_SCB);
		WL_ERROR(("wl%d: no scb for BT-AMP peer\n", wlc->pub->unit));
		return HCI_ERR_UNSPECIFIED;
	}

	/* NO multiple physical links between the same AMP pair */
	if (wlc_bta_scb_shared(bta, plidx, scb)) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_SCB_EXIST);
		WL_ERROR(("wl%d: scb existsfor other BT-AMP physical link\n",
		          wlc->pub->unit));
		return HCI_ERR_UNSPECIFIED;
	}

	wlc_scb_set_bsscfg(scb, bsscfg);

	/* Update bsscfg to bta */
	bta->bsscfg = bsscfg;

	/* fixup scb cubby */
	scb_bta = SCB_BTA(bta, scb);
	scb_bta->plidx = (uint8)plidx;
	scb_bta->plh = plh;

	/* fix association state */
	if (bta->flags & BTA_FLAGS_NO_ASSOC) {
		wlc_scb_setstatebit(scb, AUTHENTICATED);
		wlc_scb_setstatebit(scb, ASSOCIATED);
	}

	/* cacbe scb in physical link struct */
	phy_link->scb = scb;
	bta->numpls ++;

	/* we are done! */
	if (bta->flags & BTA_FLAGS_NO_ASSOC) {
		wlc_bta_join_complete(bta, scb, 0);
	}

	/* configure security settings... */
	if (!(bta->flags & BTA_FLAGS_NO_SECURITY)) {

		/* ...for authenticated key management */
		bzero(&pmk, sizeof(wsec_pmk_t));
		pmk.key_len = phy_link->lk_type_len & BTA_PLK_LENGTH_MASK;
		bcopy(phy_link->link_key, pmk.key, pmk.key_len);

		if (phy_link->flags & BTA_PL_CREATOR) {
#ifdef BCMAUTH_PSK
			WL_BTA(("wl%d: setting up as authenticator\n", wlc->pub->unit));

			if (BCMAUTH_PSK_ENAB(wlc->pub) && (bsscfg->authenticator == NULL)) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_AUTH);
				WL_ERROR(("wl%d: authenticator not attached\n", wlc->pub->unit));
				return HCI_ERR_UNSPECIFIED;
			}
			wlc_auth_set_pmk(bsscfg->authenticator, &pmk);
#endif /* BCMAUTH_PSK */
		} else {
#ifdef BCMSUP_PSK
			WL_BTA(("wl%d: setting up as supplicant\n", wlc->pub->unit));

			if (!(SUP_ENAB(wlc->pub) && (BSS_SUP_INFO(wlc->idsup, bsscfg) != NULL))) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_SUP);
				WL_ERROR(("wl%d: supplicant not attached\n", wlc->pub->unit));
				return HCI_ERR_UNSPECIFIED;
			}

			if (SUP_ENAB(wlc->pub)) {
				wlc_sup_set_ea(wlc->idsup, bsscfg, (struct ether_addr *)&mac[3]);
				wlc_sup_set_pmk(wlc->idsup, bsscfg, &pmk, FALSE);
			}
#endif /* BCMSUP_PSK */
		}
	}

	/* announce creator up running */
	if (phy_link->flags & BTA_PL_CREATOR) {
		/* generate Channel Select event */

		/* mark the Channel Select event pending on this physical link */
		phy_link->flags |= BTA_PL_CSE_PEND;

		/* we are told to defer */
		if (defer_cse) {
			ASSERT(bta->cse_timer == NULL);
			if ((bta->cse_timer =
			     wl_init_timer(wlc->wl, wlc_bta_cse_timer, bta, "bta_cse")) == NULL) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_CSE_TIMER);
				WL_ERROR(("wl%d: %s: fail to create cse timer\n",
				          wlc->pub->unit, __FUNCTION__));
				return HCI_ERR_UNSPECIFIED;
			}
			wl_add_timer(wlc->wl, bta->cse_timer, 40, FALSE);
		}
		/* we are not told to defer and there is no pending event */
		else if (bta->cse_timer == NULL)
			wlc_bta_cse_timer(bta);
		/* we are not told to defer but there are pending event(s) */
		/* else {} */
	}
	/* connect acceptor to creator */
	else if (!(bta->flags & BTA_FLAGS_NO_ASSOC)) {
		wlc_ssid_t ssid;

		ea = (struct ether_addr *)&mac[3];

		ssid.SSID_len = sprintf((char *)ssid.SSID, amp_ssid_format,
		                        ea->octet[0], ea->octet[1], ea->octet[2],
		                        ea->octet[3], ea->octet[4], ea->octet[5]);

		if (wlc_bta_join(wlc, bsscfg, ssid.SSID, ssid.SSID_len, ea->octet, chanspec)) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_JOIN_START);
			WL_ERROR(("wl%d: %s: fail to join %s\n",
			          wlc->pub->unit, __FUNCTION__, ssid.SSID));
			return HCI_ERR_UNSPECIFIED;
		}
	}

	return HCI_SUCCESS;
}

static void
wlc_bta_doconnect_error(bta_info_t *bta, int plidx, uint8 status)
{
	phy_link_evt_parms_t evt_parms;

	WL_ERROR(("wl%d: %s: physical link %d error %d\n",
	          bta->wlc->pub->unit, __FUNCTION__, bta->plh[plidx], status));

	/* generate Physical Link Complete event */
	evt_parms.status = status;
	evt_parms.plh = bta->plh[plidx];
	wlc_bta_phy_link_complete_event(bta, (uint8 *)&evt_parms, sizeof(evt_parms), FALSE);

	/* perform the rest of disconnecting physical link */
	wlc_bta_dodisconnect(bta, plidx);
}

static void
wlc_bta_phy_link_doconnect(bta_info_t *bta, int plidx)
{
	chanspec_t chanspec = 0;
	uint8 *pref_chan;
	uint8 status;

	pref_chan = wlc_bta_parse_tlvs(bta->pl[plidx].remote, bta->pl[plidx].rlen,
	                               BTA_TYPE_ID_PREFERRED_CHANNELS);
	ASSERT(pref_chan != NULL);

	/* select a channel or start a channel selection process */
	status = wlc_bta_cs(bta, plidx, &pref_chan[3], ltoh16_ua(&pref_chan[1]), &chanspec);

	/* non 0 chanspec indicates the channel is selected so go start it */
	if (status == HCI_SUCCESS && chanspec != 0)
		status = wlc_bta_doconnect(bta, plidx, chanspec);
	/* 0 chanspec indicates the channel selection process in ongoing
	 * and we will start the connection once the process is finished.
	 */
	if (status == HCI_SUCCESS)
		return;

	/* channel select or start connetion failed */
	wlc_bta_doconnect_error(bta, plidx, status);
}

/* Channel Selection */

#ifdef APCS
/* Channel Select complete callback */
static void
wlc_bta_cs_complete(void *arg, int st)
{
	bta_info_t *bta = (bta_info_t *)arg;
	wlc_info_t *wlc = bta->wlc;
	int plidx = bta->cs->plidx;
	uint8 status;

	WL_BTA(("wl%d: wlc_bta_cs_complete: finished Channel Select "
	        "for physical link %d status %d\n",
	        wlc->pub->unit, bta->plh[plidx], st));

	MFREE(wlc->osh, bta->cs, sizeof(bta_cs_t));
	bta->cs = NULL;

	if (st == WLC_E_STATUS_SUCCESS) {
		status = wlc_bta_doconnect(bta, plidx, wlc->ap->chanspec_selected);
		if (status == HCI_SUCCESS)
			return;
		WL_ERROR(("wl%d: %s: wlc_bta_doconnect() failed %d\n",
		          wlc->pub->unit, __FUNCTION__, status));
	}
	else if (st == WLC_E_STATUS_CS_ABORT) {
		wlc_bta_dodisconnect(bta, plidx);
		return;
	}
	else if (st == WLC_E_STATUS_NEWASSOC) {
		WL_BTA(("wl%d: wlc_bta_cs_complete: defer Channel Select for "
		        "physical link %d due to scan abort for association\n",
		        wlc->pub->unit, bta->plh[plidx]));
		bta->pl[plidx].flags |= BTA_PL_CS_PEND;
		return;
	}
	else {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_CS_COMPLETE);
		WL_ERROR(("wl%d: %s: Channel Select failed %d\n",
		          wlc->pub->unit, __FUNCTION__, st));
		status = HCI_ERR_UNSPECIFIED;
	}

	wlc_bta_doconnect_error(bta, plidx, status);
}
#endif /* APCS */

/* build a channel list based on the country string and the regulatory class, and filter
 * the result by driver supported channels.
 */
/* XXX The result list 'list' must be large enough to hold all possible channels available
 * recommend to allocate MAXCHANNEL worth of elements
 */
static int
wlc_bta_get_chanlist(bta_info_t *bta, char country[WLC_CNTRY_BUF_SZ], uint8 rclass, uint8 chan,
	wl_uint32_list_t *local, wl_uint32_list_t *list)
{
	wlc_info_t *wlc = bta->wlc;
	uint i, j, k;
	bool non_country;
	wl_uint32_list_t *req = NULL;
	uint req_len;
	int status = BCME_OK;

	non_country = country[0] == 'X';

	/* get the regulatory supported channels */
	req_len = OFFSETOF(wl_uint32_list_t, element) + sizeof(req->element[0]) * MAXCHANNEL;
	if ((req = MALLOC(wlc->osh, req_len)) == NULL) {
		wlc_bta_no_memory_event(bta, WLC_BTA_ERR_CS_CHAN_LIST3);
		WL_ERROR(("wl%d: failed to allocate regulatory class channel list\n",
		          wlc->pub->unit));
		status = BCME_ERROR;
		goto exit;
	}
	req->count = 0;
	/* XXX remove country string 'XXX' and regulatory class 254 hack once it is
	 * supported in the wlc_channel.c
	 */
	if (non_country && rclass == 254) {
		for (i = 0; i < 11; i ++)
			req->element[i] = i + 1;
		req->count = i;
	}
	else
		wlc_rclass_get_channel_list(wlc->cmi, country, rclass, TRUE, req);

	/* make sure the specific 'chan' is one of the supported channels of the 'rclass' */
	if (chan != 0) {
		for (i = 0; i < req->count; i ++) {
			if (chan == req->element[i])
				break;
		}
		if (i >= req->count) {
			WL_ERROR(("wl%d: channel %d is not in regulatory class %d in country %s\n",
			          wlc->pub->unit, chan, rclass, country));
			status = BCME_ERROR;
			goto exit;
		}
		req->element[0] = chan;
		req->count = 1;
	}

	/* add both driver and regulatory supported channels to the list */
	for (i = 0; i < req->count; i ++) {
		uint32 c = req->element[i];
		/* XXX remove country string 'XXX' and regulatory class 254 hack once it is
		 * supported in the wlc_channel.c
		 */
		if (non_country && c > 11)
			continue;
		for (j = 0; j < local->count; j ++) {
			/* limit channels to non-quiet channels if it doesn't
			 * specify a particular channel.
			 */
			if (chan == 0 &&
			    wlc_quiet_chanspec(wlc->cmi, (chanspec_t)local->element[j]))
				continue;
			if (c != wf_chspec_ctlchan((chanspec_t)local->element[j]))
				continue;
			for (k = 0; k < list->count; k ++) {
				if (list->element[k] == c)
					break;
			}
			if (k == list->count) {
				list->element[list->count ++] = c;
				break;
			}
		}
	}

exit:
	if (req != NULL)
		MFREE(wlc->osh, req, req_len);

	return status;
}

/* select a channel or kick off channel selection process */
static uint8
wlc_bta_cs(bta_info_t *bta, int plidx, uint8 *pref_chan, uint pref_chan_len,
	chanspec_t *chanspec_sel)
{
	wlc_info_t *wlc = bta->wlc;
	uint8 *triplet;
	char country[WLC_CNTRY_BUF_SZ];
	uint8 pref_rc[MAXRCLISTSIZE];
	uint8 pref_rcs;
	uint8 i;
	chanspec_t chanspec;
	int band;
	int8 cur_pref_rc_idx = -1;
	wl_uint32_list_t *cur = NULL;
	uint cur_len;
	wl_uint32_list_t *req = NULL;
	uint req_len = 0;
	uint8 status;
	bool local_non_country;
	const char *abbrev;
	bool apcs = (bta->pl[plidx].flags & BTA_PL_CREATOR) != 0;
	bool explicit_chanlist_exist = FALSE;

	abbrev = wlc_channel_country_abbrev(wlc->cmi);

	local_non_country = abbrev[0] == 'X';

	/* the pref_chan must include country string and 1 channel specification */
	if (pref_chan_len < 6 || pref_chan_len % 3 != 0) {
		wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_PREF_CHAN_LIST);
		WL_ERROR(("wl%d: %s: malformed Preferred Channel List\n",
		          wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_UNSPECIFIED;
		goto exit;
	}

	/* get the country string, assuming the first 2 letters are same as
	 * our country abbrevation
	 */
	/* XXX the only difference between the country string and the country abbrevation
	 * seems to be 'X''X''X' vs. 'X''X'
	 */
	country[0] = pref_chan[0];
	country[1] = pref_chan[1];
	country[2] = 0;

	/* 1. Get channels supported by both Preferred Channel List and the driver */

	/* get the driver supported chanspecs */
	cur_len = OFFSETOF(wl_uint32_list_t, element) + sizeof(cur->element[0]) * MAXCHANNEL;
	if ((cur = MALLOC(wlc->osh, cur_len)) == NULL) {
		wlc_bta_no_memory_event(bta, WLC_BTA_ERR_CS_CHAN_LIST1);
		WL_ERROR(("wl%d: %s: failed to allocate driver supported channel list\n",
		          wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_UNSPECIFIED;
		goto exit;
	}
	cur->count = 0;
	wlc_ioctl(wlc, WLC_GET_BAND, &band, sizeof(band), NULL);
	if (band == WLC_BAND_AUTO || band == WLC_BAND_2G)
		wlc_get_valid_chanspecs(wlc->cmi, cur, WL_CHANSPEC_BW_20, TRUE, abbrev);
	/* XXX remove country string 'XXX' and regulatory class 254 hack once it is
	 * supported in the wlc_channel.c
	 */
	if (local_non_country) {
		/* restrict to channel 1 to 11 */
		for (i = (uint8)cur->count; i >= 1; i --)
			if (CHSPEC_CHANNEL((chanspec_t)cur->element[i - 1]) > 11)
				cur->count --;
	}
#ifdef BAND5G
	else if (band == WLC_BAND_AUTO || band == WLC_BAND_5G)
		wlc_get_valid_chanspecs(wlc->cmi, cur, WL_CHANSPEC_BW_20, FALSE, abbrev);
#endif // endif

	/* get the Preferred Channel List requested channels */
	req_len = OFFSETOF(wl_uint32_list_t, element) + sizeof(req->element[0]) * MAXCHANNEL;
	if ((req = MALLOC(wlc->osh, req_len)) == NULL) {
		wlc_bta_no_memory_event(bta, WLC_BTA_ERR_CS_CHAN_LIST2);
		WL_ERROR(("wl%d: %s: unable to allocate Preferred Channel List "
		          "channel list\n", wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_MEMORY_FULL;
		goto exit;
	}
	req->count = 0;
	bzero(pref_rc, sizeof(pref_rc));
	pref_rcs = 0;
	/* XXX the spec. says there won't be any overlap in the Preferred Channel List
	 * and the channel number increases so no need to perform any check/sort
	 * before copy...
	 */
	for (triplet = pref_chan + 3; triplet < pref_chan + pref_chan_len; triplet += 3) {
		/* regulatory extension identifier */
		if (triplet[0] == 201) {
			if (pref_rcs >= sizeof(pref_rc)) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_TOO_MANY_RC);
				WL_ERROR(("wl%d: %s: Preferred Channel List error, "
					"two many regulatory classes\n",
					wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				goto exit;
			}
			/* copy channels of the last regulatory class if it hasn't
			 * been invalidated by any explicit channels.
			 */
			if (cur_pref_rc_idx >= 0 && !explicit_chanlist_exist)
				wlc_bta_get_chanlist(bta, country, pref_rc[cur_pref_rc_idx],
					0, cur, req);

			/* save the current regulatory class */
			pref_rc[pref_rcs] = triplet[1];
			cur_pref_rc_idx = (int8)pref_rcs;
			pref_rcs ++;
		}
		/* explicit channel number */
		else if (triplet[0] < MAXCHANNEL) {
			if (cur_pref_rc_idx < 0) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_RC);
				WL_ERROR(("wl%d: %s: Preferred Channel List error, "
				          "channel %u present without a regulatory class\n",
				          wlc->pub->unit, __FUNCTION__, triplet[0]));
				status = HCI_ERR_UNSPECIFIED;
				goto exit;
			}
			/* copy specific channel(s) */
			for (i = 0; i < triplet[1]; i ++) {
				wlc_bta_get_chanlist(bta, country, pref_rc[cur_pref_rc_idx],
					triplet[0] + i, cur, req);
			}
			/* explicit channel number exist, invalidate
			 * the current regulatory class chanlist
			 */
			explicit_chanlist_exist = TRUE;
			/* XXX Don't use APCS if any specific channel is specified.
			 * It certainly works for the list with all specific channels
			 * as the real Preferred Channel List but it won't work for
			 * the list with mixed specific channels and regulatory classes
			 * but that should be a rare case anyway.
			 */
			apcs = FALSE;
		}
	}
	/* copy channels of the last regulatory class if it hasn't
	 * been invalidated by any explicit channels.
	 */
	if (cur_pref_rc_idx >= 0 && !explicit_chanlist_exist)
		wlc_bta_get_chanlist(bta, country, pref_rc[cur_pref_rc_idx], 0, cur, req);

	/* there must be at least one channel in the channel list,
	 */
	if (req->count == 0) {
		wlc_bta_no_channel_event(bta, WLC_BTA_ERR_CHAN);
		WL_ERROR(("wl%d: %s: driver doesn't support channel(s) "
			"specified in Preferred Channel List\n",
			wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_NO_SUITABLE_CHANNEL;
		goto exit;
	}

	/* 2. Get the 'current/home' channel */

	chanspec = wlc_bta_chan_select(bta);

	/* we are connected to someone so honor the channel we are on when
	 * it is also in the acceptor's Preferred Chennel
	 */
	if (chanspec != 0) {
		for (i = 0; i < req->count; i ++) {
			if (chanspec == CH20MHZ_CHSPEC(req->element[i])) {
				*chanspec_sel = chanspec;
				status = HCI_SUCCESS;
				goto exit;
			}
		}
		wlc_bta_no_channel_event(bta, WLC_BTA_ERR_CHANSPEC_RANGE);
		WL_ERROR(("wl%d: %s: chanspec 0x%04x is not in Preferred Channel List\n",
		          wlc->pub->unit, __FUNCTION__, chanspec));
		status = HCI_ERR_NO_SUITABLE_CHANNEL;
		goto exit;
	}

	/* we are not connected so go by the acceptor's Preferred Channel List */
	band = CHANNEL_BAND(wlc, req->element[0]);
	for (i = 1; i < req->count; i ++) {
		if (band != CHANNEL_BAND(wlc, req->element[i]))
			break;
	}
	req->count = i;

	/* select it if there is only one channel in the channel list */
	if (req->count == 1 || !apcs) {
		*chanspec_sel = CH20MHZ_CHSPEC(req->element[0]);
		status = HCI_SUCCESS;
		goto exit;
	}

	/* 3. Perform Channel Select (to select a channel in which there is
	 * the least number of BSS(es) running if we are a creator; otherwise
	 * pick the first channel.
	 */

	if (bta->pl[plidx].flags & BTA_PL_CREATOR) {
		if (APCS_ENAB(wlc->pub)) {
			int ret;
			/* we can only do one channel selection at a time... defer others'. */
			if (bta->cs != NULL) {
				WL_BTA(("wl%d: wlc_bta_cs: defer Channel Select for "
				        "physical link %d due to Channel Select in progress\n",
				        wlc->pub->unit, bta->plh[plidx]));
				bta->pl[plidx].flags |= BTA_PL_CS_PEND;
				*chanspec_sel = 0;
				status = HCI_SUCCESS;
				goto exit;
			}

			/* allocate memory to hold necessary parameters for the channel select
			 * callback.
			 */
			if ((bta->cs = MALLOC(wlc->osh, sizeof(bta_cs_t))) == NULL) {
				wlc_bta_no_memory_event(bta, WLC_BTA_ERR_CS_CB);
				WL_ERROR(("wl%d: %s: unable to allocate Channel Select "
				          "info block\n", wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_MEMORY_FULL;
				goto exit;
			}
			bta->cs->plidx = plidx;

			/* kick off Channel Select process */

			/* wlc_cs_scan_start() requires chanspec */
			for (i = 0; i < req->count; i ++)
				req->element[i] = CH20MHZ_CHSPEC(req->element[i]);
			/* use active scan to speed up the scan process */
			/* FIXMEL use the appropriate bsscfg */
			ret = wlc_cs_scan_start(wlc->cfg, req, FALSE, TRUE, FALSE, band, APCS_BTA,
			                        wlc_bta_cs_complete, bta);
			if (ret != BCME_OK) {
				MFREE(wlc->osh, bta->cs, sizeof(bta_cs_t));
				bta->cs = NULL;
				/* defer Channel Select if needed */
				if (ret == BCME_BUSY ||
				    ret == BCME_NOTREADY) {
					WL_BTA(("wl%d: wlc_bta_cs: defer Channel Select for "
					        "physical link %d due to scan/assoc in progress\n",
					        wlc->pub->unit, bta->plh[plidx]));
					bta->pl[plidx].flags |= BTA_PL_CS_PEND;
					*chanspec_sel = 0;
					status = HCI_SUCCESS;
				}
				else {
					wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_CS_START);
					WL_ERROR(("wl%d: %s: unable to start Channel Select\n",
					          wlc->pub->unit, __FUNCTION__));
					status = HCI_ERR_UNSPECIFIED;
				}
				goto exit;
			}
		} else {
			*chanspec_sel = CH20MHZ_CHSPEC(req->element[0]);
		}
	}
	status = HCI_SUCCESS;

exit:
	if (cur != NULL)
		MFREE(wlc->osh, cur, cur_len);
	if (req != NULL)
		MFREE(wlc->osh, req, req_len);
	return status;
}

/* build regulatory class list of the currently configured country in the driver and
 * a given band.
 */
static uint8
wlc_bta_get_rclist(bta_info_t *bta, uint8 rclist[], uint *count, uint8 band)
{
	wlc_info_t *wlc = bta->wlc;
	wl_uint32_list_t *chspec_list;
	uint chspec_list_len;
	uint idx, i, j;
	uint8 rc;
	const char *abbrev;

	abbrev = wlc_channel_country_abbrev(wlc->cmi);

	/* query the driver supported chanspecs */
	chspec_list_len = OFFSETOF(wl_uint32_list_t, element) +
	        sizeof(chspec_list->element[0]) * MAXCHANNEL;
	if ((chspec_list = MALLOC(wlc->osh, chspec_list_len)) == NULL) {
		wlc_bta_no_memory_event(bta, WLC_BTA_ERR_CS_RC_LIST);
		WL_ERROR(("wl%d: failed to allocate Local AMP ASSOC\n", wlc->pub->unit));
		return HCI_ERR_MEMORY_FULL;
	}
	chspec_list->count = 0;
	wlc_get_valid_chanspecs(wlc->cmi, chspec_list,
		WL_CHANSPEC_BW_20, band == WLC_BAND_2G, abbrev);

	/* figure out the regulatory classes of these chanspecs */
	for (i = 0, idx = 0; i < chspec_list->count; i ++) {
		/* skip all duplicates */
		if (chspec_list->element[i] == 0)
			continue;

		rc = wlc_get_regclass(wlc->cmi, (chanspec_t)chspec_list->element[i]);
		if (rc == 0) {
			WL_ERROR(("wl%d: failed to find regulatory class for chanspec 0x%04x\n",
			          wlc->pub->unit, chspec_list->element[i]));
			continue;
		}
		rclist[idx++] = rc;

		/* remove all duplicates */
		for (j = i + 1; j < chspec_list->count; j ++) {
			if (wlc_get_regclass(wlc->cmi, (chanspec_t)chspec_list->element[j]) == rc)
				chspec_list->element[j] = 0;
		}
	}

	*count = idx;

	MFREE(wlc->osh, chspec_list, chspec_list_len);

	return HCI_SUCCESS;
}

/* build local AMP_ASSOC
 * 'chanspec' value 0 indicates a specific channel has been chosen for the physical link
 * otherwise any channel in the band(s) (5GHz band is preferred) in the country context
 * is okay.
 */
/* make sure AMP_ASSOC/AMP_ASSOC_len must not be NULL */
static uint8
wlc_bta_build_AMP_ASSOC(bta_info_t *bta, chanspec_t chanspec,
	uint8 **AMP_ASSOC, uint16 *AMP_ASSOC_len)
{
	wlc_info_t *wlc = bta->wlc;
	uint8 *local;
	uint16 local_len;
	uint8 *mac;
	uint8 rclist[MAXRCLISTSIZE];
	uint8 rcs;
	uint8 *pref_chan;
	uint16 pref_chan_len;
	uint8 i;
	bool local_non_country;
	const char *abbrev;

	ASSERT(AMP_ASSOC != NULL);
	ASSERT(AMP_ASSOC_len != NULL);

	abbrev = wlc_channel_country_abbrev(wlc->cmi);

	local_non_country = abbrev[0] == 'X';

	/* local AMP_ASSOC length is default_local + MAC Address + Preferred Channel List */
	if (chanspec != 0) {
		if (local_non_country)
			rclist[0] = 254;
		else if ((rclist[0] = wlc_get_regclass(wlc->cmi, chanspec)) == 0) {
			wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_FIND_RC);
			WL_ERROR(("wl%d: failed to find regulatory class for chanspec 0x%04x\n",
			          wlc->pub->unit, chanspec));
			return HCI_ERR_UNSPECIFIED;
		}
		rcs = 1;
		/* Country String + Reg Class + Chan Num */
		pref_chan_len = 3 + 3 + 3;
	}
	else {
		if (local_non_country) {
			rclist[0] = 254;
			rcs = 1;
		}
		else {
			uint8 status;
			uint lsize = 0;
			int band;

			rcs = 0;
			wlc_ioctl(wlc, WLC_GET_BAND, &band, sizeof(band), NULL);
#ifdef BAND5G
			if (band == WLC_BAND_AUTO || band == WLC_BAND_5G) {
				if ((status = wlc_bta_get_rclist(bta, &rclist[rcs], &lsize,
				                WLC_BAND_5G)) != HCI_SUCCESS)
					return status;
				rcs += (uint8)lsize;
			}
#endif // endif
			if (band == WLC_BAND_AUTO || band == WLC_BAND_2G) {
				if ((status = wlc_bta_get_rclist(bta, &rclist[rcs], &lsize,
				                WLC_BAND_2G)) != HCI_SUCCESS)
					return status;
				rcs += (uint8)lsize;
			}
			if (rcs == 0) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_FIND_RC_LIST);
				WL_ERROR(("wl%d: failed to find regulatory class(es)\n",
				          wlc->pub->unit));
				return HCI_ERR_UNSPECIFIED;
			}
		}
		/* Country String + Reg Class(es)... */
		pref_chan_len = 3 + rcs * 3;
	}
	ASSERT(rcs < MAXRCLISTSIZE);
	local_len = sizeof(default_local) + 3 + ETHER_ADDR_LEN + 3 + pref_chan_len;

	if ((local = MALLOC(wlc->osh, local_len)) == NULL) {
		wlc_bta_no_memory_event(bta, WLC_BTA_ERR_LOCAL_AMP_ASSOC);
		WL_ERROR(("wl%d: failed to allocate Local AMP ASSOC\n", wlc->pub->unit));
		return HCI_ERR_MEMORY_FULL;
	}

	/* initialize local AMP_ASSOC */
	/* default_local:
	 * 0x04 0x04 0x00 0x01 0x00 0x00 0x00
	 * 0x05 0x05 0x00 0x00 0x0f 0x00 0x10 0x09
	 */
	bcopy(default_local, local, sizeof(default_local));
	/* mac address:
	 * 0x01 0x06 0x00 <MAC Address>
	 */
	mac = local + sizeof(default_local);
	mac[0] = BTA_TYPE_ID_MAC_ADDRESS;
	htol16_ua_store(ETHER_ADDR_LEN, &mac[1]);
	bcopy(&wlc->pub->cur_etheraddr, &mac[3], ETHER_ADDR_LEN);
	/* preferred channel list:
	 * 0x02 (tag)
	 * 0xXX 0xXX (length)
	 * 0xXX 0xXX 0xXX (country string)
	 * ... (regulatory class(es))
	 */
	pref_chan = local + sizeof(default_local) + 3 + ETHER_ADDR_LEN;
	pref_chan[0] = BTA_TYPE_ID_PREFERRED_CHANNELS;
	htol16_ua_store(pref_chan_len, &pref_chan[1]);
	pref_chan[3] = abbrev[0];
	pref_chan[4] = local_non_country ? 'X' : abbrev[1];
	/* our country abbrev is only 2 character long, extend it to 3
	 * by adding 'X' (for 'X''X') or ' ' (for others in all environments).
	 */
	pref_chan[5] = local_non_country ? 'X' : ' ';
	if (chanspec != 0) {
		pref_chan[6] = 201;
		pref_chan[7] = rclist[0];
		pref_chan[8] = 0;
		pref_chan[9] = CHSPEC_CHANNEL(chanspec);
		pref_chan[10] = 1;
		pref_chan[11] = 20;
	}
	else {
		for (i = 0; i < rcs; i ++) {
			pref_chan[6 + i * 3] = 201;
			pref_chan[7 + i * 3] = rclist[i];
			pref_chan[8 + i * 3] = 0;
		}
	}

	*AMP_ASSOC = local;
	*AMP_ASSOC_len = local_len;

	return HCI_SUCCESS;
}

#if defined(BCMDBG) || defined(WLMSG_BTA)
/* #define VAL_HCI_CMD_LEN */
#endif // endif

#ifdef VAL_HCI_CMD_LEN
/* cmd parms minimum length (for those we care) */
static const struct {
	uint16 op_code;
	uint8 min_plen;
} bta_cmd_min_plen[] = {
	{HCI_Read_Local_AMP_ASSOC, OFFSETOF(read_local_cmd_parms_t, max_remote)},
	{HCI_Write_Remote_AMP_ASSOC, OFFSETOF(write_remote_cmd_parms_t, frag)},
	{HCI_Read_Link_Supervision_Timeout, OFFSETOF(ls_to_cmd_parms_t, timeout)},
	{HCI_Write_Link_Supervision_Timeout, sizeof(ls_to_cmd_parms_t)},
	{HCI_Write_Connection_Accept_Timeout, 2},
	{HCI_Read_Failed_Contact_Counter, 2},
	{HCI_Reset_Failed_Contact_Counter, 2},
	{HCI_Read_Link_Quality, 2},
	{HCI_Read_Best_Effort_Flush_Timeout, OFFSETOF(befto_cmd_parms_t, befto)},
	{HCI_Write_Best_Effort_Flush_Timeout, sizeof(befto_cmd_parms_t)},
	{HCI_Short_Range_Mode, sizeof(srm_cmd_parms_t)},
	{HCI_Create_Physical_Link, sizeof(phy_link_cmd_parms_t)},
	{HCI_Accept_Physical_Link_Request, sizeof(phy_link_cmd_parms_t)},
	{HCI_Disconnect_Physical_Link, sizeof(dis_phy_link_cmd_parms_t)},
	{HCI_Accept_Logical_Link, sizeof(log_link_cmd_parms_t)},
	{HCI_Create_Logical_Link, sizeof(log_link_cmd_parms_t)},
	{HCI_Disconnect_Logical_Link, 2},
	{HCI_Logical_Link_Cancel, sizeof(log_link_cancel_cmd_parms_t)},
	{HCI_Flow_Spec_Modify, sizeof(flow_spec_mod_cmd_parms_t)},
	{HCI_Enhanced_Flush, 2},
	{HCI_Write_Logical_Link_Accept_Timeout, 2},
	{HCI_Set_Event_Mask_Page_2, 2},
	{HCI_Write_Location_Data_Command, sizeof(ld_cmd_parms_t)}
};
#endif /* VAL_HCI_CMD_LEN */

static bool
wlc_bta_valid_flow_spec(bta_info_t *bta, ext_flow_spec_t *flowspec)
{
	bool valid = FALSE;

	switch (flowspec->service_type) {
	case EFS_SVCTYPE_BEST_EFFORT: {
		uint32 fto, acc_lat;
		uint16 max_sdu;
		uint32 sdu_ia_time;

		/* flush timeout shall be set to 0xffffffff */
		fto = ltoh32_ua(flowspec->flush_timeout);
		if (fto != BTA_INVFTO) {
			WL_BTA(("wl%d: wlc_bta_valid_flow_sped(): illegal flush timeout: "
				"0x%x\n", fto, bta->wlc->pub->unit));
			break;
		}

		/* access latency shall be set to 0xffffffff */
		acc_lat = ltoh32_ua(flowspec->access_latency);
		if (acc_lat != 0xffffffff) {
			WL_BTA(("wl%d: wlc_bta_valid_flow_spec(): illegal access latency: "
				"0x%x\n", acc_lat, bta->wlc->pub->unit));
			break;
		}

		/*
		 * if max_sdu is 0xffff, sdu_ia_time shall be 0xffffffff;
		 * if sdu_ia_time is 0xffffffff, max_sdu shall be 0xffff
		 */
		max_sdu = ltoh16_ua(flowspec->max_sdu);
		sdu_ia_time = ltoh32_ua(flowspec->sdu_ia_time);
		if (((max_sdu == 0xffff) && (sdu_ia_time != 0xffffffff)) ||
		    ((sdu_ia_time == 0xffffffff) && (max_sdu != 0xffff))) {
			WL_BTA(("wl%d: wlc_bta_valid_flow_spec(): illegal max_sdu and sdu_ia_time "
				"values: 0x%x 0x%x\n", max_sdu, sdu_ia_time, bta->wlc->pub->unit));
			break;
		}

		valid = TRUE;
		break;
	}

	case EFS_SVCTYPE_GUARANTEED: {
		uint32 sdu_ia_time;

		/* sdu_ia_time should not be zero */
		sdu_ia_time = ltoh32_ua(flowspec->sdu_ia_time);
		if (sdu_ia_time == 0) {
			WL_BTA(("wl%d: SDU inter-arrival time of 0\n", bta->wlc->pub->unit));
			break;
		}

		valid = TRUE;
		break;
	}

	case EFS_SVCTYPE_NO_TRAFFIC:
		/* ignore parameters */
		valid = TRUE;
		break;

	default:
		WL_BTA(("wl%d: wlc_bta_valid_flow_spec: invalid service type\n",
		        bta->wlc->pub->unit));
		break;
	}

	return valid;
}

static uint8
wlc_bta_valid_log_link_parms(bta_info_t *bta, bta_pl_t *pl,
	ext_flow_spec_t *tx, ext_flow_spec_t *rx, bool *gl, uint16 *bw)
{
	uint32 sdu_ia_time;

	/* validate flow spec parameters */
	if (!wlc_bta_valid_flow_spec(bta, tx)) {
		WL_BTA(("wl%d: illegal tx flow spec\n", bta->wlc->pub->unit));
		return HCI_ERR_UNSUPPORTED_VALUE;
	}

	if (!wlc_bta_valid_flow_spec(bta, rx)) {
		WL_BTA(("wl%d: illegal rx flow spec\n", bta->wlc->pub->unit));
		return HCI_ERR_UNSUPPORTED_VALUE;
	}

	/* validate bandwidth request */
	if (tx->service_type != EFS_SVCTYPE_GUARANTEED)
		return HCI_SUCCESS;

	sdu_ia_time = ltoh32_ua(tx->sdu_ia_time);
	if (sdu_ia_time != 0xffffffff) {
		uint16 max_sdu;

		*gl = FALSE;

		/*
		 * SDU inter-arrival time is in us, so bw is Mbps
		 * Multiply by 1000 to get to Kbps
		 */
		max_sdu = ltoh16_ua(tx->max_sdu);
		ASSERT(sdu_ia_time != 0);

		*bw = ((max_sdu * 8) * 1000)/sdu_ia_time;

		if ((pl->allocbw + *bw) <= 30000)
			return HCI_SUCCESS;

		WL_BTA(("wl%d: Max bandwidth exceeded: rejecting guaranteed logical link \n",
		        bta->wlc->pub->unit));
		return HCI_ERR_QOS_REJECTED;
	}

	*gl = TRUE;
	*bw = 0;

	return HCI_SUCCESS;
}

void
wlc_bta_docmd(void *handle, uint8 *cmd_buf, uint cmd_len)
{
	bta_info_t *bta = (bta_info_t *)handle;
	amp_hci_cmd_t *cmd = (amp_hci_cmd_t *)cmd_buf;
	wlc_info_t *wlc = bta->wlc;
	uint16 op = 0; /* HCI_NOP */
	uint8 status;
	bool radio_disabled = FALSE;
#ifdef VAL_HCI_CMD_LEN
	uint8 plen = 0;
	uint8 i;
#endif // endif

	if (!BTA_ENAB(wlc->pub)) {
		WL_ERROR(("wl%d: %s: PAL is disabled\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

#ifdef VAL_HCI_CMD_LEN
	/* validate cmd buf length */
	if (cmd_len < HCI_CMD_PREAMBLE_SIZE) {
		WL_ERROR(("wl%d: %s: command buffer is too short\n",
		          wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
		goto fmt_err;
	}
#endif // endif

	op = ltoh16_ua((uint8 *)&cmd->opcode);

#ifdef VAL_HCI_CMD_LEN
	plen = cmd->plen;

	/* validate cmd buf parm length */
	if (cmd_len < HCI_CMD_PREAMBLE_SIZE + plen) {
		WL_ERROR(("wl%d: %s: command buffer length and parms length mismatch\n",
		          wlc->pub->unit, __FUNCTION__));
		status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
		goto fmt_err;
	}
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_BTA)
	if (WL_BTA_ON())
		wlc_bta_hcidump_cmd(bta, cmd);
#endif // endif

#ifdef VAL_HCI_CMD_LEN
	/* validate minimum cmd parm length */
	for (i = 0; i < ARRAYSIZE(bta_cmd_min_plen); i ++) {
		if (bta_cmd_min_plen[i].op_code == op &&
		    bta_cmd_min_plen[i].min_plen > plen) {
			WL_ERROR(("wl%d: %s: command parms is too short\n",
			          wlc->pub->unit, __FUNCTION__));
			status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
			goto fmt_err;
		}
	}

	status = HCI_SUCCESS;

fmt_err:
	/* generate Command Status event */
	if (status != HCI_SUCCESS) {
		wlc_bta_cmd_status_event(bta, op, status);
		return;
	}
#endif /* VAL_HCI_CMD_LEN */

	if (!wlc->pub->up) {
		if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
		    mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE)) {
			radio_disabled = TRUE;
		}
	}

	switch (op) {
	case HCI_Read_Local_AMP_Info: {
		read_local_info_evt_parms_t evt_parms;

		wlc_bta_state_log(bta, (uint8)HCIReadLocalAMPInfo);
		/* generate the "global" default AMP_Info */
		evt_parms.status = HCI_SUCCESS;
		evt_parms.AMP_status = bta->amp_state;
		htol32_ua_store(30000, (uint8 *)&evt_parms.bandwidth);
		htol32_ua_store(30000, (uint8 *)&evt_parms.gbandwidth);
		htol32_ua_store(28, (uint8 *)&evt_parms.latency);
		htol32_ua_store(BTA_MTU, (uint8 *)&evt_parms.PDU_size);
		evt_parms.ctrl_type = 0x1;
		htol16_ua_store(wlc_bta_qos(bta), (uint8 *)&evt_parms.PAL_cap);
		htol16_ua_store(672, (uint8 *)&evt_parms.AMP_ASSOC_len);
		htol32_ua_store(BTA_INVFTO, (uint8 *)&evt_parms.max_flush_timeout);
		htol32_ua_store(BTA_INVFTO, (uint8 *)&evt_parms.be_flush_timeout);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Read_Local_AMP_ASSOC: {
		read_local_cmd_parms_t *parms = (read_local_cmd_parms_t *)cmd->parms;
		read_local_evt_parms_t *evt_ptr = NULL;
		read_local_evt_parms_t evt_parms;
		uint evt_size = 0;
		uint16 offset;
		uint8 *local = NULL;
		uint16 len = 0;
		uint16 flen = 0;	/* len of frag field */

		wlc_bta_state_log(bta, (uint8)HCIReadLocalAMPASSOC);
		offset = ltoh16_ua(parms->offset);

		/* read global AMP_ASSOC (for AMP_Get_Info_Response) */
		if (parms->plh == 0) {
			if (offset == 0) {
				if (bta->local != NULL) {
					MFREE(wlc->osh, bta->local, bta->llen);
					bta->local = NULL;
				}

				status = wlc_bta_build_AMP_ASSOC(bta, wlc_bta_chan_select(bta),
				                                 &bta->local, &bta->llen);
			}
			else if (bta->local == NULL) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_LOCAL_AMP_ASSOC);
				WL_ERROR(("wl%d: %s: local AMP_ASSOC on "
				          "physical link %d hasn't been initialized\n",
				          wlc->pub->unit, __FUNCTION__, parms->plh));
				status = HCI_ERR_UNSPECIFIED;
			}
			else
				status = HCI_SUCCESS;
			if (status == HCI_SUCCESS) {
				if (offset >= bta->llen)
					status = HCI_ERR_PARAM_OUT_OF_RANGE;
				else {
					local = bta->local;
					len = bta->llen;
				}
			}
		}
		/* read specific physical links AMP_ASSOC */
		else {
			int plidx = 0;

			status = wlc_bta_pllookup(bta, parms->plh, &plidx);
			if (status == HCI_SUCCESS) {
				if (offset >= bta->pl[plidx].llen)
					status = HCI_ERR_PARAM_OUT_OF_RANGE;
				else if (bta->pl[plidx].local == NULL) {
					wlc_bta_unspecified_error_event(bta,
						WLC_BTA_ERR_LOCAL_AMP_ASSOC);
					WL_ERROR(("wl%d: %s: local AMP_ASSOC on "
					          "physical link %d hasn't been initialized\n",
					          wlc->pub->unit, __FUNCTION__, parms->plh));
					status = HCI_ERR_UNSPECIFIED;
				}
				else {
					local = bta->pl[plidx].local;
					len = bta->pl[plidx].llen;
				}
			}
		}

		/* generate Command Complete event with local AMP_ASSOC */
		if (status == HCI_SUCCESS) {

			flen = len - offset;

			/* limit the frag field to 246 octets */
			if (flen > 246)
				flen = 246;

			/* allocate memory for local AMP_ASSOC data */
			evt_size = OFFSETOF(read_local_evt_parms_t, frag) + flen;
			if ((evt_ptr = MALLOC(wlc->osh, evt_size)) == NULL) {
				wlc_bta_no_memory_event(bta, WLC_BTA_ERR_LOCAL_AMP_ASSOC);
				WL_ERROR(("wl%d: %s: failed to create "
					"Read Local AMP ASSOC event\n",
					wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_MEMORY_FULL;
			}
		}
		/* use minimum event on the stack to communicate error status */
		if (status != HCI_SUCCESS) {
			flen = 0;
			evt_size = OFFSETOF(read_local_evt_parms_t, frag);
			evt_ptr = &evt_parms;
		}
		evt_ptr->status = status;
		evt_ptr->plh = parms->plh;
		htol16_ua_store(flen, (uint8 *)&evt_ptr->len);
		/* start reading at offset Length So Far parameter */
		if (status == HCI_SUCCESS)
			bcopy(&local[offset], evt_ptr->frag, flen);
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)evt_ptr, evt_size);

		if (status == HCI_SUCCESS)
			MFREE(wlc->osh, evt_ptr, evt_size);

		break;
	}

	case HCI_Write_Remote_AMP_ASSOC: {
		write_remote_cmd_parms_t *parms = (write_remote_cmd_parms_t *)cmd->parms;
		write_remote_evt_parms_t evt_parms;
		bta_pl_t *phy_link = NULL;
		uint16 offset;
		uint16 fraglen = 0;
		int plidx = 0;
		uint8 *remote, *mac, *pref_chan = NULL, *ver_ie;
		uint16 len = 0;

		wlc_bta_state_log(bta, (uint8)HCIWriteRemoteAMPASSOC);
		offset = ltoh16_ua(parms->offset);

		/* find target physical link */
		status = wlc_bta_pllookup(bta, parms->plh, &plidx);

		/* accumulate remote AMP_ASSOC */
		do {
			if (status != HCI_SUCCESS) {
				WL_ERROR(("wl%d: %s: wlc_bta_pllookup failed\n",
				          wlc->pub->unit, __FUNCTION__));
				break;
			}

			phy_link = &bta->pl[plidx];

			/* XXX it should never happen but it did so leave this here
			 * just in case.
			 */
			if (phy_link->flags & BTA_PL_CONN) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_PHY_LINK_EXIST);
				WL_ERROR(("wl%d: physical link %d is already in "
				          "connecting/connected state\n",
				          wlc->pub->unit, parms->plh));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}
			phy_link->flags |= BTA_PL_CONN;

			/* allocate remote AMP_ASSOC if necessary */
			if (offset == 0) {
				if (phy_link->remote != NULL) {
					MFREE(wlc->osh, phy_link->remote, phy_link->rlen);
					phy_link->remote = NULL;
				}
				len = phy_link->rlen = ltoh16_ua(parms->len);
				remote = phy_link->remote = MALLOC(wlc->osh, len);
				if (remote == NULL) {
					wlc_bta_no_memory_event(bta, WLC_BTA_ERR_REMOTE_AMP_ASSOC);
					WL_ERROR(("wl%d: %s: failed to allocate Remote AMP ASSOC\n",
						wlc->pub->unit, __FUNCTION__));
					status = HCI_ERR_MEMORY_FULL;
					break;
				}
			}
			else {
				remote = phy_link->remote;
				len = phy_link->rlen;
			}

			/* write frag starting at offset */
			fraglen = cmd->plen - OFFSETOF(write_remote_cmd_parms_t, frag);
			if (len < offset + fraglen) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_TOO_MUCH_DATA);
				WL_ERROR(("wl%d: %s: too much data\n",
				          wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}
			bcopy(parms->frag, &remote[offset], fraglen);

			/* remote AMP_ASSOC is partial... */
			if (len > offset + fraglen) {
				WL_BTA(("wl%d: wlc_bta_docmd: more data to come...\n",
				        wlc->pub->unit));
				break;
			}

			/* MAC address */
			mac = wlc_bta_parse_tlvs(remote, len, BTA_TYPE_ID_MAC_ADDRESS);
			if (!mac || (mac[1] != ETHER_ADDR_LEN) || (mac[2] != 0)) {
				WL_ERROR(("wl%d: missing MAC address or invalid MAC address "
				          "length in remote AMP_ASSOC\n", wlc->pub->unit));
				status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
				break;
			}

			/* Preferred Channel List */
			pref_chan = wlc_bta_parse_tlvs(remote, len, BTA_TYPE_ID_PREFERRED_CHANNELS);
			if (pref_chan == NULL) {
				WL_ERROR(("wl%d: no preferred channel list in "
					  "remote AMP_ASSOC\n", wlc->pub->unit));
				status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
				break;
			}

			/* PAL version */
			ver_ie = wlc_bta_parse_tlvs(remote, len, BTA_TYPE_ID_VERSION);
			if (!ver_ie || (ver_ie[1] != 5) || (ver_ie[2] != 0)) {
				WL_ERROR(("wl%d: missing PAL version or invalid PAL version length "
					  "in remote AMP_ASSOC\n", wlc->pub->unit));
				status = HCI_ERR_ILLEGAL_PARAMETER_FMT;
				break;
			}

			/* radio disabled check */
			if (radio_disabled) {
				WL_ERROR(("wl%d: %s: HCI_Write_Remote_AMP_ASSOC,"
					  " Radio Disabled\n", bta->wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}
		} while (FALSE);

		/* generate Command Complete event */
		evt_parms.status = status;
		evt_parms.plh = parms->plh;
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));

		/* we now have the complete remote AMP_ASSOC */
		if ((status == HCI_SUCCESS) && (len == offset + fraglen))
			wlc_bta_phy_link_doconnect(bta, plidx);

		break;
	}

	case HCI_Read_Link_Supervision_Timeout: {
		ls_to_cmd_parms_t *cmdparms = (ls_to_cmd_parms_t *)cmd->parms;
		read_ls_to_evt_parms_t evt_parms;
		uint8 cmdplh;
		int plidx = 0;

		do {
			/* find target physical link */
			cmdplh = cmdparms->handle.amp.plh;
			status = wlc_bta_pllookup(bta, cmdplh, &plidx);
			if (status != HCI_SUCCESS)
				break;

			evt_parms.handle.amp.plh = cmdplh;
			evt_parms.handle.amp.pad = 0;
			htol16_ua_store(bta->pl[plidx].ls_to, (uint8 *)&evt_parms.timeout);

		} while (FALSE);

		/* generate Command Complete event */
		evt_parms.status = status;
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Write_Link_Supervision_Timeout: {
		ls_to_cmd_parms_t *cmdparms = (ls_to_cmd_parms_t *)cmd->parms;
		int plidx = 0;

		do {
			/* find target physical link */
			status = wlc_bta_pllookup(bta, cmdparms->handle.amp.plh, &plidx);
			if (status != HCI_SUCCESS)
				break;

			/* set timeout */
			bta->pl[plidx].ls_to = ltoh16_ua(cmdparms->timeout);

		} while (FALSE);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));
		break;
	}

	case HCI_Read_Connection_Accept_Timeout: {
		read_lla_ca_to_evt_parms_t evt_parms;

		evt_parms.status = HCI_SUCCESS;
		htol16_ua_store(bta->ca_to, (uint8 *)&evt_parms.timeout);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Write_Connection_Accept_Timeout: {
		uint16 timeout;

		/* check range */
		timeout = ltoh16_ua(cmd->parms);
		if (timeout >= 1 && timeout <= 0xb540) {
			bta->ca_to = timeout;
			status = HCI_SUCCESS;
		} else {
			status = HCI_ERR_PARAM_OUT_OF_RANGE;
		}

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));
		break;
	}

	case HCI_Read_Failed_Contact_Counter: {
		contact_counter_evt_parms_t evt_parms;
		uint16 count = 0;
		uint16 llidx;

		llidx = ltoh16(cmd->parms[0]);
		status = wlc_bta_valid_logical_link(bta, llidx);
		if (status == HCI_SUCCESS)
			count = bta->ll[llidx].failed_contact_cnt;

		evt_parms.status = status;
		htol16_ua_store(llidx, (uint8 *)&evt_parms.llh);
		htol16_ua_store(count, (uint8 *)&evt_parms.counter);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Reset_Failed_Contact_Counter: {
		contact_counter_reset_evt_parms_t evt_parms;
		uint16 llidx;

		llidx = ltoh16(cmd->parms[0]);

		/* find target physical link */
		status = wlc_bta_valid_logical_link(bta, llidx);
		if (status == HCI_SUCCESS)
			bta->ll[llidx].failed_contact_cnt = 0;

		evt_parms.status = status;
		htol16_ua_store(llidx, (uint8 *)&evt_parms.llh);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Read_Link_Quality: {
		read_linkq_evt_parms_t evt_parms;
		uint8 plh;
		int plidx = 0;

		do {
			/* find target physical link */
			plh = cmd->parms[0];
			status = wlc_bta_pllookup(bta, plh, &plidx);
			if (status != HCI_SUCCESS)
				break;

			evt_parms.handle.amp.plh = plh;
			evt_parms.handle.amp.pad = 0;
			evt_parms.link_quality = wlc_bta_phy_link_rssi(bta, plidx);

		} while (FALSE);

		/* generate Command Complete event */
		evt_parms.status = status;
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Read_Best_Effort_Flush_Timeout: {
		uint16 llh = ltoh16_ua(cmd->parms);
		befto_evt_parms_t evt_parms;

		do {
			status = wlc_bta_valid_logical_link(bta, llh);
			if (status != HCI_SUCCESS)
				break;

			htol32_ua_store(bta->ll[llh].fto, (uint8 *)&evt_parms.befto);

		} while (FALSE);

		/* generate Command Complete event */
		evt_parms.status = status;
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));

		break;
	}

	case HCI_Write_Best_Effort_Flush_Timeout: {
		befto_cmd_parms_t *cmd_parms = (befto_cmd_parms_t *)cmd->parms;
		uint16 llh = ltoh16_ua(cmd_parms->llh);

		do {
			status = wlc_bta_valid_logical_link(bta, llh);
			if (status != HCI_SUCCESS)
				break;

			bta->ll[llh].fto = ltoh32_ua(cmd_parms->befto);
		} while (FALSE);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));

		break;
	}

	case HCI_Short_Range_Mode: {
		srm_cmd_parms_t *cmd_parms = (srm_cmd_parms_t *)cmd->parms;
		srm_evt_parms_t *evt_parms;
		amp_hci_event_t *evt;
		int plidx = 0;

		do {
			/* find target physical link */
			status = wlc_bta_pllookup(bta, cmd_parms->plh, &plidx);
			if (status != HCI_SUCCESS)
				break;

			/* sanity check short range mode setting */
			if (cmd_parms->srm > 1) {
				status = HCI_ERR_UNSUPPORTED_VALUE;
				break;
			}

			/* apply short range mode setting */
			bta->pl[plidx].short_range = cmd_parms->srm;

		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS)
			break;

		/* generate Short Range Mode Change Complete event */
		if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Short_Range_Mode_Change_Complete,
		                sizeof(srm_evt_parms_t))) == NULL) {
			WL_ERROR(("wl%d: %s: failed to create Short Range Mode event\n",
			          wlc->pub->unit, __FUNCTION__));
			break;
		}

		evt_parms = (srm_evt_parms_t *)evt->parms;
		evt_parms->status = status;
		evt_parms->plh = cmd_parms->plh;
		evt_parms->srm = cmd_parms->srm;

		wlc_bta_doevent(bta, evt);

		wlc_bta_free_hcievent(bta, evt);
		break;
	}

	case HCI_Create_Physical_Link:
		wlc_bta_state_log(bta, (uint8)HCICreatePhysicalLink);
		status = wlc_bta_connect_phy_link(bta,
		        (phy_link_cmd_parms_t *)cmd->parms, cmd->plen, TRUE);
		wlc_bta_cmd_status_event(bta, op, status);
		break;

	case HCI_Accept_Physical_Link_Request:
		wlc_bta_state_log(bta, (uint8)HCIAcceptPhysicalLinkRequest);
		status = wlc_bta_connect_phy_link(bta,
		        (phy_link_cmd_parms_t *)cmd->parms, cmd->plen, FALSE);
		wlc_bta_cmd_status_event(bta, op, status);
		break;

	case HCI_Disconnect_Physical_Link: {
		dis_phy_link_cmd_parms_t *parms = (dis_phy_link_cmd_parms_t *)cmd->parms;
		int plidx = 0;

		wlc_bta_state_log(bta, (uint8)HCIDisconnectPhysicalLink);
		do {
			/* find target physical link */
			if ((status = wlc_bta_pllookup(bta, parms->plh, &plidx)) != HCI_SUCCESS) {
				WL_ERROR(("wl%d: %s: wlc_bta_pllookup failed\n",
					wlc->pub->unit, __FUNCTION__));
				break;
			}

			if (radio_disabled) {
				WL_ERROR(("wl%d: %s: HCI_Disconnect_Physical_Link,"
					  " Radio Disabled\n", bta->wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}
		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS)
			break;

		/* disconnect the physical link and generate events */
		wlc_bta_disconnect_physical_link(bta, plidx, parms->reason, FALSE);
		break;
	}

	case HCI_Accept_Logical_Link:
	case HCI_Create_Logical_Link: {
		log_link_cmd_parms_t *cmdparms = (log_link_cmd_parms_t *)cmd->parms;
		log_link_evt_parms_t evtparms;
		ext_flow_spec_t *txflow = NULL;
		bool guaranteed_latency = 0;
		bta_pl_t *phy_link = NULL;
		uint16 reqbw = 0;
		uint16 llidx = 0;
		int plidx = 0;
		uint8 plh = cmdparms->plh;

		wlc_bta_state_log(bta, ((op == HCI_Accept_Logical_Link) ?
			(uint8)HCIAcceptLogicalLink: (uint8)HCICreateLogicalLink));
		do {
			ext_flow_spec_t *rxflow;

			/* validate physical link */
			status = wlc_bta_pllookup(bta, plh, &plidx);
			if (status != HCI_SUCCESS)
				break;
			phy_link = &bta->pl[plidx];

			if (!(phy_link->flags & BTA_PL_COMPLETE)) {
				status = HCI_ERR_CONNECTION_DISALLOWED;
				break;
			}

			/* allocate logical link */
			for (llidx = 0; llidx < BTA_MAXLOGLINKS; llidx++) {
				if (bta->ll[llidx].plh == 0) {
					status = HCI_SUCCESS;
					break;
				}
			}
			if (llidx == BTA_MAXLOGLINKS) {
				status = HCI_ERR_MAX_NUM_OF_CONNECTIONS;
				break;
			}

			bzero(&bta->ll[llidx], sizeof(bta_ll_t));

			/* establish channel */
			txflow = (ext_flow_spec_t *)cmdparms->txflow;
			rxflow = (ext_flow_spec_t *)cmdparms->rxflow;

			/* validate service types */
			if (((txflow->service_type == EFS_SVCTYPE_BEST_EFFORT) &&
			     (rxflow->service_type == EFS_SVCTYPE_GUARANTEED)) ||
			    ((txflow->service_type == EFS_SVCTYPE_GUARANTEED) &&
			     (rxflow->service_type == EFS_SVCTYPE_BEST_EFFORT))) {
				WL_BTA(("wl%d: Create/Accept Logical Link: "
					"illegal logical link service types\n", wlc->pub->unit));
				status = HCI_ERR_UNSUPPORTED_VALUE;
				break;
			}

			if (txflow->service_type == EFS_SVCTYPE_GUARANTEED) {
				if (bta->flags & BTA_FLAGS_NO_QOS) {
					WL_BTA(("wl%d: Create/Accept Logical Link: "
						"No QoS: rejecting guaranteed logical link\n",
						wlc->pub->unit));
					status = HCI_ERR_QOS_REJECTED;
					break;
				}
			}

			/* validate the rest of the flow spec */
			status = wlc_bta_valid_log_link_parms(bta, phy_link, txflow, rxflow,
			                                      &guaranteed_latency, &reqbw);
			if (status != HCI_SUCCESS)
				break;

			if (radio_disabled) {
				WL_ERROR(("wl%d: %s:"
					" HCI_Accept_Logical_Link/HCI_Create_Logical_Link,"
					" Radio Disabled\n", bta->wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}

		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS) {
			if (status != HCI_ERR_MAX_NUM_OF_CONNECTIONS)
				bta->ll[llidx].failed_contact_cnt++;
			break;
		}

		/* derive logical link properties */
		if (txflow->service_type == EFS_SVCTYPE_NO_TRAFFIC) {
			bta->ll[llidx].prio = PRIO_8021D_NONE;
			bta->ll[llidx].fto = BTA_INVFTO;
			bta->ll[llidx].reqbw = 0;
		} else if (txflow->service_type == EFS_SVCTYPE_BEST_EFFORT) {
			bta->ll[llidx].prio = PRIO_8021D_BE;
			bta->ll[llidx].fto = BTA_INVFTO;
			bta->ll[llidx].reqbw = 0;
		} else {
			ASSERT(txflow->service_type == EFS_SVCTYPE_GUARANTEED);

			if (guaranteed_latency) {
				/* Guaranteed latency with no specified bandwidth */
				/* map to a non-Best Effort UP (Voice for latency) */
				bta->ll[llidx].prio = PRIO_8021D_VO;
			} else {
				/* Guaranteed bandwidth */
				/* map to a non-Best Effort UP (Video for bandwidth) */
				bta->ll[llidx].prio = PRIO_8021D_VI;
			}
			bta->ll[llidx].fto = ltoh32_ua(txflow->flush_timeout);
			bta->ll[llidx].reqbw = reqbw;
		}

		/* update aggregate requested bandwidth */
		phy_link->allocbw += bta->ll[llidx].reqbw;

		/* cache physical link handle, tx flow spec identifier, and requested bw */
		bta->ll[llidx].plh = plh;
		bta->ll[llidx].plidx = (uint8)plidx;
		bta->ll[llidx].tx_fs_ID = txflow->id;

		WL_BTA(("wl%d: logical link allocated: llh %d prio %d fto 0x%x reqbw 0x%x\n",
			wlc->pub->unit, llidx, bta->ll[llidx].prio, bta->ll[llidx].fto,
			bta->ll[llidx].reqbw));

		/* generate Logical Link Complete event */
		evtparms.status = status;
		htol16_ua_store(llidx, (uint8 *)&evtparms.llh);
		evtparms.plh = plh;
		evtparms.tx_fs_ID = bta->ll[llidx].tx_fs_ID;

		wlc_bta_log_link_event(bta, HCI_Logical_Link_Complete,
			(uint8 *)&evtparms, sizeof(evtparms));
		break;
	}

	case HCI_Disconnect_Logical_Link: {
		uint16 llh = ltoh16_ua(cmd->parms);

		wlc_bta_state_log(bta, (uint8)HCIDisconnectLogicalLink);
		do {
			if ((status = wlc_bta_valid_logical_link(bta, llh)) != HCI_SUCCESS) {
				WL_ERROR(("wl%d: %s: wlc_bta_pllookup failed\n",
					wlc->pub->unit, __FUNCTION__));
				break;
			}

			if (radio_disabled) {
				WL_ERROR(("wl%d: %s: HCI_Disconnect_Logical_Link,"
					" Radio Disabled\n",
					bta->wlc->pub->unit, __FUNCTION__));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}

		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS)
			break;

		/* disconnect the logical link and generate events */
		wlc_bta_disconnect_logical_link(bta, llh, HCI_ERR_CONN_TERM_BY_LOCAL_HOST, TRUE);
		break;
	}

	case HCI_Logical_Link_Cancel: {
		log_link_cancel_cmd_parms_t *parms = (log_link_cancel_cmd_parms_t *)cmd->parms;
		log_link_cancel_evt_parms_t evtparms;
		uint8 plh = parms->plh;
		uint16 llidx;

		wlc_bta_state_log(bta, (uint8)HCILogicalLinkCancel);
		do {
			/* sanity check: physical link handle cannot be zero */
			if (plh == 0) {
				status = HCI_ERR_UNSUPPORTED_VALUE;
				break;
			}

			status = HCI_ERR_NO_CONNECTION;

			/* find logical link with desired tx flow spec ID */
			for (llidx = 0; llidx < BTA_MAXLOGLINKS; llidx++) {
				bta_ll_t *ll = &bta->ll[llidx];

				if ((ll->plh == plh) &&
				    (ll->tx_fs_ID == parms->tx_fs_ID)) {
					status = HCI_ERR_CONNECTION_EXISTS;
					break;
				}
			}

		} while (FALSE);

		/* N.B.: nothing to do here since creation is atomic */

		/* generate Command Complete event */
		evtparms.status = status;
		evtparms.plh = plh;
		evtparms.tx_fs_ID = parms->tx_fs_ID;

		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evtparms, sizeof(evtparms));
		break;
	}

	case HCI_Flow_Spec_Modify: {
		flow_spec_mod_cmd_parms_t *cmdparms = (flow_spec_mod_cmd_parms_t *)cmd->parms;
		amp_hci_event_t *evt;
		flow_spec_mod_evt_parms_t *evt_parms;
		uint16 llh = ltoh16_ua(cmdparms->llh);
		bta_pl_t *phy_link = NULL;
		ext_flow_spec_t *txflow = NULL;
		bool guaranteed_latency = FALSE;
		uint16 reqbw = 0;

		do {
			int plidx = 0;
			ext_flow_spec_t *rxflow;

			status = wlc_bta_valid_logical_link(bta, llh);
			if (status != HCI_SUCCESS)
				break;

			/* lookup physical link */
			status = wlc_bta_pllookup(bta, bta->ll[llh].plh, &plidx);
			if (status != HCI_SUCCESS)
				break;
			phy_link = &bta->pl[plidx];

			/* validate flow spec parameters */
			txflow = (ext_flow_spec_t *)cmdparms->txflow;
			rxflow = (ext_flow_spec_t *)cmdparms->rxflow;

			/* can't change identifier */
			if (txflow->id != bta->ll[llh].tx_fs_ID) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_FLOW_SPEC_ID);
				WL_BTA(("wl%d: Flow Spec Modify: can't change identifier\n",
					wlc->pub->unit));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}

			/* can't change service type */
			if ((txflow->service_type == EFS_SVCTYPE_GUARANTEED &&
			     bta->ll[llh].prio != PRIO_8021D_VO &&
			     bta->ll[llh].prio != PRIO_8021D_VI) ||
			    (txflow->service_type == EFS_SVCTYPE_BEST_EFFORT &&
			     bta->ll[llh].prio != PRIO_8021D_BE) ||
			    (txflow->service_type == EFS_SVCTYPE_NO_TRAFFIC &&
			     bta->ll[llh].prio != PRIO_8021D_NONE)) {
				wlc_bta_unspecified_error_event(bta, WLC_BTA_ERR_FLOW_SPEC_TYPE);
				WL_BTA(("wl%d: Flow Spec Modify: can't change service type\n",
					wlc->pub->unit));
				status = HCI_ERR_UNSPECIFIED;
				break;
			}

			status = wlc_bta_valid_log_link_parms(bta, phy_link, txflow, rxflow,
			                                      &guaranteed_latency, &reqbw);
			if (status != HCI_SUCCESS)
				break;

		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS)
			break;

		if (txflow->service_type == EFS_SVCTYPE_GUARANTEED) {
			if (guaranteed_latency) {
				/* Guaranteed latency with no specified bandwidth */
				/* map to a non-Best Effort UP (Voice for latency) */
				bta->ll[llh].prio = PRIO_8021D_VO;
			} else {
				/* Guaranteed bandwidth */
				/* map to a non-Best Effort UP (Video for bandwidth) */
				bta->ll[llh].prio = PRIO_8021D_VI;
			}

			bta->ll[llh].fto = ltoh32_ua(txflow->flush_timeout);

			/* update aggregate requested bandwidth (before modifying ll's) */
			phy_link->allocbw -= bta->ll[llh].reqbw;
			phy_link->allocbw += reqbw;

			/* modify ll's requested bandwidth */
			bta->ll[llh].reqbw = reqbw;
		}

		WL_BTA(("wl%d: flow spec for logical link modified: llh = %d prio = %d "
			"fto = 0x%x reqbw = 0x%x\n", wlc->pub->unit, llh, bta->ll[llh].prio,
			bta->ll[llh].fto, bta->ll[llh].reqbw));

		/* generate Flow_Spec_Modify_Complete event */
		if ((evt = wlc_bta_alloc_hcievent(bta, HCI_Flow_Spec_Modify_Complete,
		                sizeof(flow_spec_mod_evt_parms_t))) == NULL) {
			WL_ERROR(("wl%d: %s: failed to create Flow Spec Modify event\n",
			          wlc->pub->unit, __FUNCTION__));
			break;
		}

		evt_parms = (flow_spec_mod_evt_parms_t *)evt->parms;
		evt_parms->status = status;
		htol16_ua_store(llh, (uint8 *)&evt_parms->llh);

		wlc_bta_doevent(bta, evt);

		wlc_bta_free_hcievent(bta, evt);
		break;
	}

	case HCI_Reset: {
		int plidx;

		status = HCI_SUCCESS;
		wlc_bta_state_log(bta, (uint8)HCIReset);

		/* disconnect any physical links */
		for (plidx = 0; plidx < WLC_MAXBTA; plidx++) {
			if (bta->plh[plidx] == 0)
				continue;

			wlc_bta_phy_link_dodisconnect(bta, plidx, 0, FALSE, FALSE, FALSE);
		}

		/* restore parameters to their default values */
		wlc_bta_default(bta);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));
		break;
	}

	case HCI_Enhanced_Flush: {
		eflush_cmd_parms_t *cmdparms = (eflush_cmd_parms_t *)cmd->parms;
		uint16 llh = ltoh16_ua(cmdparms->llh);

		do {
			status = wlc_bta_valid_logical_link(bta, llh);

		} while (FALSE);

		/* generate Command Status event */
		wlc_bta_cmd_status_event(bta, op, status);
		if (status != HCI_SUCCESS)
			break;

		wlc_bta_flush_hcidata(bta, llh);

		/* generate Enhanced Flush Complete event */
		wlc_bta_flush_hcidata_complete_event(bta, llh);
		break;
	}

	case HCI_Read_Logical_Link_Accept_Timeout: {
		read_lla_ca_to_evt_parms_t evt_parms;

		evt_parms.status = HCI_SUCCESS;
		htol16_ua_store(bta->lla_to, (uint8 *)&evt_parms.timeout);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Write_Logical_Link_Accept_Timeout: {
		uint16 timeout = ltoh16_ua(cmd->parms);

		wlc_bta_state_log(bta, (uint8)HCIWriteLogicalLinkAcceptTimeout);
		/* check range */
		if (timeout >= 1 && timeout <= 0xb540) {
			bta->lla_to = timeout;
			status = HCI_SUCCESS;
		} else {
			status = HCI_ERR_PARAM_OUT_OF_RANGE;
		}

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));
		break;
	}

	case HCI_Set_Event_Mask_Page_2:
		do {
			status = HCI_SUCCESS;

			/* store the non-reserved portion of the mask */
			bta->evt_mask_2 = ltoh16_ua(cmd->parms);

		} while (FALSE);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));
		break;

	case HCI_Read_Location_Data_Command: {
		ld_evt_parms_t evt_parms;

		evt_parms.status = HCI_SUCCESS;
		evt_parms.ld_aware = bta->ld_aware;
		bcopy(bta->ld, evt_parms.ld, 2);
		evt_parms.ld_opts = bta->ld_opts;
		evt_parms.l_opts = bta->l_opts;

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	case HCI_Write_Location_Data_Command: {
		ld_cmd_parms_t *ld_parms = (ld_cmd_parms_t *)cmd->parms;

		do {
			/* country code must be "XX" if regulatory domain unknown */
			if (ld_parms->ld_aware == 0x00 &&
			    (ld_parms->ld[0] != 'X' || ld_parms->ld[1] != 'X')) {
				status = HCI_ERR_UNSUPPORTED_VALUE;
				break;
			}

			status = HCI_SUCCESS;

			/* store the location data */
			bta->ld_aware = ld_parms->ld_aware;
			bcopy(ld_parms->ld, bta->ld, 2);
			bta->ld_opts = ld_parms->ld_opts;
			bta->l_opts = ld_parms->l_opts;

		} while (FALSE);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));

		break;
	}

	case HCI_Read_Local_Version_Info: {
		local_version_info_evt_parms_t evt_parms;

		evt_parms.status = HCI_SUCCESS;
		evt_parms.hci_version = HCI_VERSION;
		htol16_ua_store(HCI_REVISION, (uint8 *)&evt_parms.hci_revision);
		evt_parms.pal_version = PAL_VERSION;
		htol16_ua_store(MFG_NAME, (uint8 *)&evt_parms.mfg_name);
		htol16_ua_store(PAL_SUBVERSION, (uint8 *)&evt_parms.pal_subversion);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));

		break;
	}

	case HCI_Read_Local_Supported_Commands: {
		local_supported_cmd_evt_parms_t	evt_parms;

		evt_parms.status = HCI_SUCCESS;
		bcopy(hci_cmd_vec, evt_parms.cmd, MAX_SUPPORTED_CMD_BYTE);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));

		break;
	}

	case HCI_Read_Buffer_Size: {
		uint8 rbs_pars[8];

		rbs_pars[0] = HCI_SUCCESS; /* status */
		htol16_ua_store(BTA_MTU, &rbs_pars[1]); /* HC ACL Data Packet Length */
		rbs_pars[3] = 0;  /* HC Synchronous Data Packet Length */
		htol16_ua_store(8, &rbs_pars[4]); /* HC Total Num ACL Data Packets */
		htol16_ua_store(0, &rbs_pars[6]); /* HC Total Num Synchronous Data Packets */

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, rbs_pars, sizeof(rbs_pars));
		break;
	}

	case HCI_Read_Data_Block_Size: {
		read_data_block_size_evt_parms_t evt_parms;

		evt_parms.status = HCI_SUCCESS;
		htol16_ua_store(BTA_MTU, (uint8 *)&evt_parms.ACL_pkt_len);
		htol16_ua_store(BTA_MTU, (uint8 *)&evt_parms.data_block_len);
		htol16_ua_store(BTA_MAXDATABLKS, (uint8 *)&evt_parms.data_block_num);

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&evt_parms, sizeof(evt_parms));
		break;
	}

	default:
		WL_ERROR(("wl%d: < HCI Command: Unsupported(0x%x|0x%x)\n",
		          wlc->pub->unit, HCI_CMD_OGF(op), HCI_CMD_OCF(op)));

		status = HCI_ERR_ILLEGAL_COMMAND;

		/* generate Command Complete event */
		wlc_bta_cmd_complete_event(bta, op, (uint8 *)&status, sizeof(status));

		break;
	}
}

static void
wlc_bta_evtfwd_upd(bta_info_t *bta)
{
	/* turn on/off event forwarding */
	if (bta->wlc->eventq != NULL)
		wlc_eventq_set_ind(bta->wlc->eventq, WLC_E_BTA_HCI_EVENT,
		                   (bta->flags & BTA_FLAGS_ET_RX) != 0);
}

static void
wlc_bta_flags_upd(bta_info_t *bta)
{
}

/* handle BTA related iovars */
static int
wlc_bta_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	bta_info_t *bta = (bta_info_t *)context;
	wlc_info_t *wlc = bta->wlc;
	int32 int_val = 0;
	bool bool_val;
	int err = 0;

	ASSERT(bta == wlc->bta);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
#ifndef DONGLEBUILD
	case IOV_SVAL(IOV_HCI_CMD): {
		amp_hci_cmd_t *cmd = (amp_hci_cmd_t *)a;

		/* sanity check: command preamble present */
		if (alen < (int)HCI_CMD_PREAMBLE_SIZE)
			return BCME_BUFTOOSHORT;

		/* sanity check: command parameters are present */
		if (alen < (int)(HCI_CMD_PREAMBLE_SIZE + cmd->plen))
			return BCME_BUFTOOSHORT;

		wlc_bta_docmd((void *)bta, (uint8 *)cmd, alen);
		break;
	}

	case IOV_SVAL(IOV_HCI_ACL_DATA): {
		amp_hci_ACL_data_t *ACL_data = (amp_hci_ACL_data_t *)a;
		uint16 dlen;

		/* sanity check: HCI header present */
		if (alen < (int)HCI_ACL_DATA_PREAMBLE_SIZE)
			return BCME_BUFTOOSHORT;

		/* sanity check: ACL data is present */
		dlen = ltoh16(ACL_data->dlen);
		if (alen < (int)(HCI_ACL_DATA_PREAMBLE_SIZE + dlen))
			return BCME_BUFTOOSHORT;

		wlc_bta_tx_hcidata((void *)bta, (uint8 *)ACL_data, alen);
		break;
	}
#endif /* !DONGLEBUILD */

	case IOV_GVAL(IOV_BT_AMP):
		*((uint32 *)a) = BTA_ENAB(wlc->pub);
		break;

	case IOV_SVAL(IOV_BT_AMP):
		wlc->pub->_bta = bool_val;
		break;

	case IOV_GVAL(IOV_BT_AMP_FLAGS):
		*((uint32 *)a) = (uint32)bta->flags;
		break;

	case IOV_SVAL(IOV_BT_AMP_FLAGS): {
		uint8 flags = (uint8)int_val;

		/* use of security and QoS require association */
		if ((flags & BTA_FLAGS_NO_ASSOC) &&
		    (!(flags & BTA_FLAGS_NO_SECURITY) || !(flags & BTA_FLAGS_NO_QOS))) {
			WL_ERROR(("wl%d: use of security and QoS require association\n",
			          wlc->pub->unit));
			err = BCME_EPERM;
		} else {
			bta->flags = flags;
#ifdef DONGLEBUILD
			bta->flags |= BTA_FLAGS_ET_RX;
#endif // endif
			if (!bta->support_11n)
				bta->flags |= BTA_FLAGS_NO_11N;
		}

		/* update event forwarding */
		wlc_bta_evtfwd_upd(bta);
		/* update stuff depending on flags */
		wlc_bta_flags_upd(bta);
		break;
	}

	case IOV_GVAL(IOV_BT_AMP_CHAN):
		*((uint32 *)a) = wf_chspec_ctlchan(bta->chanspec_user);
		break;

	case IOV_SVAL(IOV_BT_AMP_CHAN):
		if (int_val)
			bta->chanspec_user = CH20MHZ_CHSPEC((uint8)int_val);
		else
			bta->chanspec_user = 0;
		break;

	case IOV_GVAL(IOV_BT_AMP_11N):
		*((uint32 *)a) = bta->support_11n;
		break;

	case IOV_SVAL(IOV_BT_AMP_11N):
		bta->support_11n = bool_val;
		bta->flags &= ~BTA_FLAGS_NO_11N;
		if (!bta->support_11n)
			bta->flags |= BTA_FLAGS_NO_11N;
		break;
	case IOV_GVAL(IOV_BT_AMP_FB):
		*((uint32 *)a) = bta->_fb;
		break;

	case IOV_SVAL(IOV_BT_AMP_FB):
		bta->_fb = bool_val;
		break;

	case IOV_GVAL(IOV_BT_AMP_STATE_LOG):
		/* total buffer space required = 64 bytes + 1 byte for idx */
		if (alen >= BTA_STATE_LOG_SZ) {
			uint8 *b = (uint8 *)a;
			/* first byte is index */
			b[0] = bta->state_idx;
			bcopy(bta->state, b, BTA_STATE_LOG_SZ);
		} else
			err = BCME_BUFTOOSHORT;
		break;

	case IOV_SVAL(IOV_BT_AMP_STATE_LOG):
		bzero(bta->state, BTA_STATE_LOG_SZ);
		bta->state_idx = 0;
		break;

#if defined(BCMDBG) || defined(WLMSG_BTA)
	case IOV_GVAL(IOV_BT_AMP_MSGLEVEL):
		*((uint32 *)a) = bta->msglevel;
		break;

	case IOV_SVAL(IOV_BT_AMP_MSGLEVEL):
		bta->msglevel = (uint32)int_val;
		if (bta->msglevel & BTA_HCI_DATA_MSG)
		        bta->evt_prn_mask |= HCI_Number_of_Completed_Data_Blocks_Event_Mask;
		else
			bta->evt_prn_mask &= ~HCI_Number_of_Completed_Data_Blocks_Event_Mask;
		break;
	case IOV_GVAL(IOV_BT_AMP_EVT_MSK):
		*((uint32 *)a) = bta->evt_prn_mask;
		break;

	case IOV_SVAL(IOV_BT_AMP_EVT_MSK):
		bta->evt_prn_mask = (uint16)int_val;
		break;

	case IOV_SVAL(IOV_BT_AMP_ACT_REPORT):
	{
		int plidx;
		uint8 flag = (uint8)int_val;

		/* For each physical link... */
		for (plidx = 0; plidx < WLC_MAXBTA; plidx++) {
			bta_pl_t *pl;

			if (bta->plh[plidx] == 0)
				continue;

			pl = &bta->pl[plidx];

			wlc_bta_send_activity_report_flag(bta, pl, flag);
		}
		break;
	}
#endif /* BCMDBG || WLMSG_BTA */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_bta_dump(bta_info_t *bta, struct bcmstrbuf *b)
{
	int pl, ll;

	bcm_bprintf(b, "numpls_allocated: %u numpls: %u lla_to: %d ca_to: %d "
	            "chanspec_sel: 0x%04x\n",
	            bta->numpls_allocated, bta->numpls, bta->lla_to, bta->ca_to,
	            bta->chanspec_sel);
	bcm_bprintf(b, "pending: %u complete: %u\n",
	            bta->datablks_pending, bta->datablks_complete);
	for (pl = 0; pl < WLC_MAXBTA; pl ++) {
		if (bta->plh[pl] == 0)
			continue;
		bcm_bprintf(b, "plh: %u plidx: %d\n",
		            bta->plh[pl], pl);
		bcm_bprintf(b, "scb: %p flags: 0x%02x ca_ts: %u short_range: %d\n",
		            bta->pl[pl].scb, bta->pl[pl].flags, bta->pl[pl].ca_ts,
		            bta->pl[pl].short_range);
		bcm_bprintf(b, "ls_to: %d used: %d\n",
		            bta->pl[pl].ls_to, bta->pl[pl].used);
	}
	for (ll = 0; ll < BTA_MAXLOGLINKS; ll ++) {
		if (bta->ll[ll].plh == 0)
			continue;
		bcm_bprintf(b, "llh: %u plh: %d plidx: %d\n",
		            ll, bta->ll[ll].plh, bta->ll[ll].plidx);
		bcm_bprintf(b, "prio: %u complete: %u\n",
		            bta->ll[ll].prio, bta->ll[ll].datablks_complete);
	}
	bcm_bprintf(b, "\nbsscfg %p amp_state: %d support_11n %d FB %d\n",
	            bta->bsscfg, bta->amp_state, bta->support_11n, bta->_fb);
	bcm_bprintf(b, "evt_mask_2: 0x%x evt_prn_mask: 0x%x msglevel: 0x%x\n",
	            bta->evt_mask_2, bta->evt_prn_mask, bta->msglevel);
	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef AP
/* EDCA Params */
/* 802.11e EDCA Parameter Set Element */
static uint
wlc_bta_calc_edca_param_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	bool is11e = FALSE;

	if (data->ft == FC_ASSOC_RESP || data->ft == FC_REASSOC_RESP) {
		wlc_iem_ft_cbparm_t *cbparm = data->cbparm->ft;
		struct scb *scb = cbparm->assocresp.scb;
		is11e = SCB_11E(scb);
	}
	else if (BSS_BTA_ENAB(wlc, cfg) && wlc_bta_qos(bta)) {
		is11e = TRUE;
	}

	if (is11e)
		return TLV_HDR_LEN + sizeof(edca_param_ie_t);

	return 0;
}

static int
wlc_bta_write_edca_param_ie(void *ctx, wlc_iem_build_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	bool is11e = FALSE;

	if (data->ft == FC_ASSOC_RESP || data->ft == FC_REASSOC_RESP) {
		wlc_iem_ft_cbparm_t *cbparm = data->cbparm->ft;
		struct scb *scb = cbparm->assocresp.scb;
		is11e = SCB_11E(scb);
	}
	else if (BSS_BTA_ENAB(wlc, cfg) && wlc_bta_qos(bta)) {
		is11e = TRUE;
	}

	if (is11e) {
		WL_BTA(("wl%d: %s: adding EDCA Parameter Set IE\n",
		        wlc->pub->unit, __FUNCTION__));

		bcm_write_tlv(DOT11_MNG_EDCA_PARAM_ID, &cfg->wme->wme_param_ie_ad->qosinfo,
			sizeof(edca_param_ie_t), data->buf);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_bta_parse_edca_param_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->assocresp.scb;

	/* If BT-AMP enabled and QoS in use, check if response indicates 802.11e assoc */
	/*
	 * XXX - Eventually, this should be BSS_BTA_ENAB(wlc, bsscfg), but that won't work
	 * right now.  How can I tell this is a BT-AMP association attempt?  Punt for now,
	 * realizing simultaneous BT-AMP and legacy 802.11 (using 802.11e?) is broken...
	 */
	if (!BSS_BTA_ENAB(wlc, cfg) || !wlc_bta_qos(wlc->bta))
		return BCME_OK;

	if (ie != NULL)
		return BCME_OK;

	WL_BTA(("wl%d: EDCA Parameter Set IE in assocresp\n",
	        WLCWLUNIT(wlc)));

	wlc_qosinfo_update(scb, 0, TRUE);	/* Clear Qos Info by default */
	scb->flags |= SCB_11ECAP;
#ifdef NOT_YET
	bcopy(edcaie->acparam, XXX, XXX);
	/* XXX - Apply the AC params sent by physical link creator,
	 * will be done in wlc_join_adopt_bss()
	 */
	wlc_edcf_acp_apply(wlc, cfg, TRUE);
#endif /* NOT_YET */

	return BCME_OK;
}

static int
wlc_bta_scan_parse_edca_param_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (!BTA_ENAB(wlc->pub))
		return BCME_OK;

	/* 802.11e parameters */
	if (ie != NULL) {
		edca_param_ie_t *edcaie = (edca_param_ie_t *)&ie->data;

		bi->flags |= WLC_BSS_11E;
		bi->wme_qosinfo = edcaie->qosinfo;
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef STA
/* QoS Cap */
/* Include an 802.11e QoS Capability info element if the peer supports 802.11e */
/*
 * XXX - Eventually, this should be BSS_BTA_ENAB(wlc, bsscfg), but that won't work
 * right now.  How can I tell this is a BT-AMP association attempt?  Punt for now,
 * realizing simultaneous BT-AMP and legacy 802.11 (using 802.11e?) is broken...
 */
static uint
wlc_bta_calc_qos_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_BTA_ENAB(wlc, cfg) && wlc_bta_qos(bta) &&
	    (bi->flags & WLC_BSS_11E))
		return sizeof(qos_cap_ie_t);

	return 0;
}

static int
wlc_bta_write_qos_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_BTA_ENAB(wlc, cfg) && wlc_bta_qos(bta) &&
	    (bi->flags & WLC_BSS_11E)) {
		qos_cap_ie_t qos_cap;

		qos_cap.qosinfo = 0;

		bcm_write_tlv(DOT11_MNG_QOS_CAP_ID, &qos_cap, sizeof(qos_cap), data->buf);
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef AP
static int
wlc_bta_parse_qos_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	bta_info_t *bta = (bta_info_t *)ctx;
	wlc_info_t *wlc = bta->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->assocreq.scb;

	if (!BSS_BTA_ENAB(wlc, cfg) || !wlc_bta_qos(wlc->bta))
		return BCME_OK;

	if (ie != NULL)
		return BCME_OK;

	/* Handle 802.11e BT-AMP association */
	WL_BTA(("wl%d: QoS Capability IE present in assocreq\n", wlc->pub->unit));

	wlc_qosinfo_update(scb, 0, TRUE);     /* Clear Qos Info by default */
	scb->flags |= SCB_11ECAP;

	return BCME_OK;
}
#endif /* AP */

#endif /* #ifdef WLBTAMP */
