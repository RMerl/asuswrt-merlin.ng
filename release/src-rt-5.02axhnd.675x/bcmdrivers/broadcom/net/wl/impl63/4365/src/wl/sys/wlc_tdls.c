/*
 * TDLS (Tunnel Direct Link Setup) source file
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
 * $Id: wlc_tdls.c 779795 2019-10-07 23:12:17Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriver80211z]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLTDLS

/* TDLS channel Switch rclist needs WL11N defined. */
#ifndef WL11N
#error "WL11N not defined."
#endif	/* WL11N */
#ifndef WL11H
#error "WL11H not defined."
#endif	/* WL11H */
#ifndef STA
#error "STA not defined."
#endif	/* STA */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_sup.h>
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif // endif
#include <wlc_tdls.h>
#include <wlc_scan.h>
#include <wlc_ampdu.h>
#include <wlc_ampdu_cmn.h>
#include <wlc_frmutil.h>
#include <wl_export.h>
#include <bcmcrypto/sha256.h>
#include <bcmcrypto/aes.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_apps.h>
#include <wlc_ap.h>
#include <wlc_pcb.h>
#include <wlc_utils.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif /* WL11AC */
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_ht.h>
#include <wlc_obss.h>

#include <wlc_rx.h>

/* XXX
 *
 * Note: integration with IE mgmt. module is partially done to a digree
 * to remove direct references to wlc_channel/wlc_cntry/wlc_csa... more
 * to be done...
 */
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif /* PROP_TXSTATUS */

#include <wlc_tx.h>
#include <wlc_tbtt.h>

#ifndef DONGLEBUILD
#define BUFEND_AND_RETURN(buf_pre, buf_now) \
{ \
	if ((uchar*)(buf_pre) == (uchar*)(buf_now)) {				\
		WL_ERROR(("%s, line %d, BUFEND_AND_RETURN: end at buf_pre = %p\n", \
			  __FUNCTION__, __LINE__, (uchar*)buf_pre)); \
		return (FALSE);				\
	}								\
}
#else /* DONGLEBUILD */
/* __LINE__ cannot be used in ROM functions since this will result in ROM abandons due to
 * mismatched ROM and RAM line numbers.
 */
#define BUFEND_AND_RETURN(buf_pre, buf_now)	\
{	\
	if ((uchar*)(buf_pre) == (uchar*)(buf_now)) {					\
		WL_ERROR(("%s, BUFEND_AND_RETURN: end at buf_pre = %p\n",	\
			__FUNCTION__, (uchar*)buf_pre));	\
		return (FALSE);							\
	}											\
}
#endif /* !DONGLEBUILD */

/* iovar table */
enum {
	IOV_TDLS_ENABLE,		/* enable/disable TDLS */
	IOV_TDLS_AUTO_OP,		/* enable TDLS auto operation */
	IOV_TDLS_DISC_WINDOW,	/* TDLS DiscoveryRequestWindow */
	IOV_TDLS_UAPSD_SLEEP,	/* TDLS U-APSD sleep */
	IOV_TDLS_QOSINFO,		/* TDLS QoS info */
	IOV_TDLS_TRIGGER_PKTCNT_HIGH, /* TDLS discvoery trigger pkt count per second */
	IOV_TDLS_TRIGGER_PKTCNT_LOW,  /* TDLS teardown trigger pkt count per second */
	IOV_TDLS_RSSI_HIGH,		/* higher RSSI threshold to establish the TDLS link */
	IOV_TDLS_RSSI_LOW,		/* lower RSSI threshold to teardown the TDLS link */
	IOV_TDLS_ENDPOINT,		/* create/mod/del TDLS manual endpoint  */
	IOV_TDLS_MAX_SESSIONS,	/* max # of TDLS connections */
	IOV_TDLS_LIFETIME,		/* SA lifetime for TDLS link */
	IOV_TDLS_PU_IND_WINDOW,
	IOV_TDLS_SWITCH_TIME,	/* in us */
	IOV_TDLS_SWITCH_TIMEOUT,	/* in us */
	IOV_TDLS_MANUAL_CHSW,	/* TDLS manual channel switch */
	IOV_TDLS_WFD_IE,
#ifdef TDLS_TESTBED
	IOV_TDLS_TEST_PROHIBIT,
	IOV_TDLS_TEST_MAC,
	IOV_TDLS_WRONG_BSSID,
	IOV_TDLS_NO_RESP,
	IOV_TDLS_CHSW_TIMEOUT,
	IOV_TDLS_STATUS,
	IOV_TDLS_QUIET_DOWN,
	IOV_TDLS_TEST_TKIP,
#endif // endif
	IOV_TDLS_CHSW_MODE,
	IOV_TDLS_IDLE_TIME,
	IOV_TDLS_STA_INFO,
	IOV_TDLS_SETUP_RESP_TIMEOUT,	/* in seconds */
	IOV_TDLS_CERT_TEST
	};

static const bcm_iovar_t tdls_iovars[] = {
	{"tdls_enable", IOV_TDLS_ENABLE, 0, IOVT_BOOL, 0},
	{"tdls_auto_op", IOV_TDLS_AUTO_OP, 0, IOVT_BOOL, 0},
	{"tdls_uapsd_sleep", IOV_TDLS_UAPSD_SLEEP, 0, IOVT_BOOL, 0},
	{"tdls_trigger_pktcnt_high", IOV_TDLS_TRIGGER_PKTCNT_HIGH, 0, IOVT_UINT32, 0},
	{"tdls_trigger_pktcnt_low", IOV_TDLS_TRIGGER_PKTCNT_LOW, 0, IOVT_UINT32, 0},
	{"tdls_rssi_high", IOV_TDLS_RSSI_HIGH, 0, IOVT_INT32, 0},
	{"tdls_rssi_low", IOV_TDLS_RSSI_LOW, 0, IOVT_INT32, 0},
	{"tdls_endpoint", IOV_TDLS_ENDPOINT, IOVF_SET_UP, IOVT_BUFFER, sizeof(tdls_iovar_t)},
#ifdef TDLS_TESTBED
	{"tdls_test_prohibit", IOV_TDLS_TEST_PROHIBIT, 0, IOVT_BOOL, 0},
	{"tdls_test_mac", IOV_TDLS_TEST_MAC, 0, IOVT_BOOL, 0},
	{"tdls_wrong_bssid", IOV_TDLS_WRONG_BSSID, 0, IOVT_BOOL, 0},
	{"tdls_no_resp", IOV_TDLS_NO_RESP, 0, IOVT_BOOL, 0},
	{"tdls_chsw_timeout", IOV_TDLS_CHSW_TIMEOUT, 0, IOVT_BOOL, 0},
	{"tdls_status", IOV_TDLS_STATUS, 0, IOVT_UINT32, 0},
	{"tdls_quiet_down", IOV_TDLS_QUIET_DOWN, 0, IOVT_BOOL, 0},
	{"tdls_test_tkip", IOV_TDLS_TEST_TKIP, 0, IOVT_BOOL, 0},
#endif // endif
	{"tdls_chsw_mode", IOV_TDLS_CHSW_MODE, 0, IOVT_UINT32, 0},
	{"tdls_wfd_ie", IOV_TDLS_WFD_IE, 0, IOVT_BUFFER, sizeof(tdls_wfd_ie_iovar_t)},
	{"tdls_idle_time", IOV_TDLS_IDLE_TIME, 0, IOVT_UINT32, 0},
	{"tdls_sta_info", IOV_TDLS_STA_INFO, IOVF_GET_UP, IOVT_BUFFER, WL_OLD_STAINFO_SIZE},
	{"tdls_setup_resp_timeout", IOV_TDLS_SETUP_RESP_TIMEOUT, 0, IOVT_UINT32, 0},
	{"tdls_cert_test", IOV_TDLS_CERT_TEST, 0, IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

static uint16 opermode_bw_chspec_map[] = {
	WL_CHANSPEC_BW_20,
	WL_CHANSPEC_BW_40,
	WL_CHANSPEC_BW_80,
	/* Op mode value for 80+80 and 160 is same */
	WL_CHANSPEC_BW_8080
};
typedef enum tpk_handshake_msg {
	HANDSHAKE_MSG_1,
	HANDSHAKE_MSG_2,
	HANDSHAKE_MSG_3
}	tpk_handshake_msg_t;

/* TDLS default constant */
#define TDLS_PU_IND_WINDOW_DEFAULT	1 /* min interval in beacon intervals between */
						/* successive Peer Traffic Indication(PTI) frames */
#define TDLS_ACK_RETY_LIMIT_DEFAULT	3
#define TDLS_RESP_TIMEOUT_DEFAULT	5	/* in sec */
#define TDLS_PROBE_DELAY_DEFAULT	1000	/* in ms */

#define TDLS_QOSINFO_DEFAULT	0x0f	/* all AC's U-APSD flag set to 1 */

#define TDLS_SWITCH_TIME_DEFAULT	10000	/* us, to enable PHYCAL_CACHING to reduce it */
#define TDLS_SWITCH_TIMEOUT_DEFAULT	30000	/* us */

#define TDLS_SA_LIFETIME_MIN	300	/* in sec */

#define TDLS_SA_LIFETIME_DEFAULT	300000 /* in second */
#define TDLS_CHSW_RECOVERY_TIMEOUT_DEFAULT	1000	/* in milli sec */

#define TDLS_MAX_HEARTBEAT_MISSED	5

#define TDLS_DISCOVERY_REQ_WINDOW	2 	/* minumum number of DTIM between */
		/* successive TDLS Discovery Requests */
#define TDLS_LOOKASIDE_DEFAULT_IDLE_TIME	60000	/* entry idle time  in  ms */
#define TDLS_LOOKASIDE_DECLINED_IDLE_TIME	1800000 /* entry declined idle time ms */

#define TDLS_LOOKASIDE_SIZE	10	/* max TDLS lookaside entries */

#define TDLS_TPK_LEN	32		/* TK_bits(16*8 for AES) + 128 bits */
#define TDLS_TPK_KCK_LEN	16		/* 0 - 127 of TPK */
#define TDLS_TPK_TK_LEN	16		/* 128 - 255 of TPK */

#define TDLS_RSSI_THRESHOLD_HIGH	-70	/* rssi threshold for establishing TDLS link */
#define TDLS_RSSI_THRESHOLD_LOW		-80 /* rssi threshold for tearing down TDLS link */
#define TDLS_TRIGGER_PKTCNT_HIGH 100 /* TDLS discvoery trigger pkt threshold, per sec */
#define TDLS_TRIGGER_PKTCNT_LOW	 10  /* TDLS teardown trigger pkt threshold, per sec */

#define TDLS_WFD_IE_SETUP_SIZE_INT	64 /* smaller than TDLS_WFD_IE_SIZE */
#define TDLS_WFD_IE_PROBE_SIZE_INT	64 /* smaller than TDLS_WFD_IE_SIZE */

/* flags for TDLS scb */
#define TDLS_SCB_ACTIVE		0x0001	/* TDLS link active */
#define TDLS_SCB_MANUAL		0x0002	/* TDLS manual */
#define TDLS_SCB_SENT_SETUP_REQ		0x0004	/* TDLS setup req sent */
#define TDLS_SCB_SENT_SETUP_RESP	0x0008	/* TDLS setup resp sent */
#define TDLS_SCB_SENT_SETUP_CFM		0x0010	/* TDLS setup confirm sent */
#define TDLS_SCB_CONNECTED	0x0020	/* TDLS connection with peer exists */
#define TDLS_SCB_SENT_TEARDOWN 0x040	/* TDLS link teardown */
#define TDLS_SCB_SENT_PTI		0x80	/* TDLS PTI req */
#define TDLS_SCB_SENT_PTI_RESP	0x100	/* TDLS PTI resp */
#define TDLS_SCB_PM_SET			0x200 /* TDLS buffer STA PM set */
#define TDLS_SCB_SCHED_PTI		0x400	/* TDLS schedule a PTI req */
#define TDLS_SCB_RECV_CHSW_REQ	0x800 /* TDLS received a Channel Switch Request */
#define TDLS_SCB_SENT_CHSW_REQ	0x1000	/* TDLS sent a Channel Switch Request */
#define TDLS_SCB_SENT_CHSW_RESP	0x2000	/* TDLS sent a Channel Switch Response */
#define TDLS_SCB_WAIT_SWITCHTIME 	0x4000 /* TDLS channel switch wait switchTime to end */
#define TDLS_SCB_LISTEN_ON_TARGET_CH	0x8000 /* TDLS channel switch wait for data frame on */
						 /* target channel */
#define TDLS_SCB_WAIT_TX_ON_TARGET_CH	0x10000 /* TDLS channel switch initiator wait to tx */
						 /* on target channel */
#define TDLS_SCB_WAIT_NULLDATA_ACK	0x20000 /* TDLS channel switch wait for ACK of txed */
						 /* null data on target channel */
#define TDLS_SCB_WAIT_HEARTBEAT_ACK	0x40000 /* wait for ACK of heart beat pkt to peer */

#define TDLS_SCB_CHSWRSP_CANCEL		0x80000	/* set TDLS Channel Swich Rsp cancleable */
#define TDLS_SCB_RCVD_TEARDOWN 0x100000	/* TDLS link teardown action frame received */

#define TDLS_SCB_PTI_INTERVAL	0x200000 /* the minimum interval between 2 PTIs */

#define TDLS_SCB_PTI_RESP_TIMED_OUT        0x80000000

#define TDLS_SCB_CON_PENDING 0x1c
#define TDLS_SCB_CHSW_MASK	0x3f800

/* ext_cap_flags bits */
#define TDLS_CAP_TDLS			0x1		/* TDLS support */
#define TDLS_CAP_PU_BUFFER_STA	0x2		/* TDLS Peer U-APSD buffer STA support */
#define TDLS_CAP_PEER_PSM		0x4		/* TDLS Peer PSM support */
#define TDLS_CAP_CH_SW			0x8		/* TDLS Channel switch */
#define TDLS_CAP_PROH			0x10	/* TDLS prohibited */
#define TDLS_CAP_CH_SW_PROH		0x20	/* TDLS Channel switch prohibited */
#define TDLS_CAP_TDLS_WIDER_BW	0x40	/* TDLS Wider Band-Width */
#define TDLS_CAP_OP_MODE_NOTIF	0x80	/* TDLS Support receiption Operation Mode Notification */

/* SCB_TDLS ext cap flags */
#define TDLS_SCB_TDLS_WB_CAP	0x1		/* TDLS Wider Band-Width */
#define TDLS_SCB_TDLS_OP_MODE_NOTIF	0x80	/* TDLS Operating Mode Nofitication */

/* SCB WB Capability */
#define	TDLS_SCB_WB_CAP(a)	(((a)->ext_cap_flags & TDLS_SCB_TDLS_WB_CAP))

/* Local WB Capability */
#define TDLS_WB_CAP(a)		(((a)->ext_cap_flags & TDLS_CAP_TDLS_WIDER_BW))

/* SCB TDLS Operating Mode Nofitication Capability */
#define	TDLS_SCB_OP_MODE_NOTIF(a)	(((a)->ext_cap_flags & TDLS_SCB_TDLS_OP_MODE_NOTIF))

#define TDLS_SCBFLAGS_STR \
	"active", \
	"manual", \
	"setup_req_sent", \
	"setup_resp_sent", \
	"setup_confirm_set", \
	"connected", \
	"sent_teardown", \
	"sent_pti", \
	"sent_ptr", \
	"pm_set", \
	"sched_pti", \
	"recv_chsw_req", \
	"sent_chsw_req", \
	"sent_chsw_resp", \
	"wait_switchtime", \
	"listen_on_target_ch", \
	"wait_tx_on_target_ch", \
	"wait_tx_nulldata_ack", \
	"wait_hb_ack", \
	"chswrsp_cancel", \
	"rcvd_teardown", \
	"pti_interval", \
	"resp_timed_out", \

/* flags for lookaside entries */
#define TDLS_LOOKASIDE_INUSE 	0x1	/* entry in use */
#define TDLS_LOOKASIDE_PERM 	0x2	/* persistent entry */
#define TDLS_LOOKASIDE_TEMP		0x4 /* temperary entry */

/* status for lookaside entries */
#define TDLS_LOOKASIDE_IDLE		0	/* entry in IDLE state */
#define TDLS_LOOKASIDE_DISC 	0x1	/* entry in DISCOVERY state */
#define TDLS_LOOKASIDE_ACTIVE 	0x2	/* entry is in a TDLS connection */
#define TDLS_LOOKASIDE_WAIT_TO_DISC	0x4	/* entry waiting to send DISCOVERY */
#define TDLS_LOOKASIDE_DECLINED_IDLE 0x8	/* TDLS setup is declined by peer */

typedef struct extcap_vec {
	uint8   vec[DOT11_11AC_EXTCAP_LEN_TDLS];   /* bitvec of ext capabilities */
} extcap_vec_t;

/* TDLS related stats */
typedef struct wlc_tdls_cnt {
	uint32 txsetupreq;	/* tdls setup req sent */
	uint32 txsetupresp;	/* tdls setup resp sent */
	uint32 txsetupcfm;	/* tdls setup confirm sent */
	uint32 txteardown;	/* tdls teardwon frames sent */
	uint32 txptireq;
	uint32 txptiresp;
	uint32 txchswreq;
	uint32 txchswresp;
	uint32 rxsetupreq;	/* tdls setup req recd */
	uint32 rxdsetupresp;	/* tdls setup resp recd */
	uint32 rxsetupcfm;	/* tdls setup confirm rcvd */
	uint32 rxteardown;	/* tdls teardown frames rcvd */
	uint32 rxptireq;
	uint32 rxptiresp;
	uint32 rxchswreq;
	uint32 rxchswresp;
	uint32 discard;		/* frames discarded due to full buffer */
	uint32 ubuffered;	/* frames buffered by TDLS txmod */
	uint32 buf_reinserted;	/* frames reinserted */
} wlc_tdls_cnt_t;

/* structure to store information about a TDLS peer that has not yet
 * responded
 */
typedef struct tdls_lookaside {
	struct ether_addr ea;	/* mac address of TDLS peer */
	wlc_bsscfg_t *parent;
	uint8 flags;		/* flags for the connection */
	uint8 status;		/* status for the entry */
	int8 rssi;
	uint32 pkt_cnt;		/* packet count */
	uint32 upd_time;	/* entry status update time */
	struct wl_timer *timer;	/* per entry timer */
	uint8 *wfd_ie_probe_rx; /* dynamically allocated memory */
	uint16 wfd_ie_probe_rx_length; /* if null, wfd_ie_probe_rx is n/a */
	bool pmoverride_set;
} tdls_lookaside_t;

#define TDLS_CHSW_ENABLE	0
#define TDLS_CHSW_DISABLE	1
#define TDLS_CHSW_REJREQ	2
#define TDLS_CHSW_UNSOLRESP	3
#define TDLS_CHSW_SEND_REQ	4

/* TDLS module specific state */
struct tdls_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	wlc_pub_t *pub;
	osl_t *osh;
	bool up;
	int scb_handle;		/* scb cubby handle to retrieve data from scb */
	uint32 ext_cap_flags;		/* flags for capabilities */
	bool auto_op;		/* auto discovery/setup/teardown */
	bool uapsd_sleep;	/* where to be a U-APSD sleep STA */
	bool manual_chsw;	/* TDLS manual channel switch */
	uint8 disc_window;	/* minimum number of DTIM intervals */
	uint8 qosinfo;		/* TDLS QoS info in WMM info/param element */
	uint8 pu_ind_window; /* TDLS PU Indication window in beacon intervals */
	uint8 token;		/* id for the TDLS action frames */
	uint16 switch_time;	/* TDLS channel switch time */
	uint16 switch_timeout;	/* time to wait for a data pkt after TDLS channel switch */
	int16 rssi_threshold_high;	/* higher rssi threshold for establish TDLS link */
	int16 rssi_threshold_low;	/* lower rssi threshold for tear down TDLS link */
	uint32 trigger_pktcnt_high;	/* TDLS discovery trigger pkt count per second */
	uint32 trigger_pktcnt_low;	/* TDLS teardwon trigger pkt count per second */
	uint32 sa_lifetime;		/* TDLS SA lifetime */
	uint max_sessions;	/* max number of TDLS sessions */
	uint cur_sessions;	/* cur number of TDLS sessions */
#ifdef WLCNT
	wlc_tdls_cnt_t *cnt;	/* counters/stats */
#endif /* WLCNT */
	tdls_lookaside_t *lookaside;	/* lookaside table[TDLS_LOOKASIDE_SIZE] */
	uint8 wfd_ie_setup_tx[TDLS_WFD_IE_SETUP_SIZE_INT];
	uint8 wfd_ie_setup_tx_length;
	/* peer information is to be found on peer scb (peer_ies) */
	uint8 wfd_ie_probe_tx[TDLS_WFD_IE_PROBE_SIZE_INT];
	uint8 wfd_ie_probe_tx_length;
	/* peer information is to be found in the lookaside table */
	uint16 idle_time;
	uint16 setup_resp_timeout; /* wait for resp after TDLS Set Req/Resp */
	uint8 chsw_mode;
	bool cert_test;
#ifdef TDLS_TESTBED
	bool test_mac;
	bool test_prohibit;
	bool wrong_bssid;
	bool test_no_resp;
	bool test_chsw_timeout;
	struct ether_addr test_BSSID;
	bool test_send;
	uint16 test_status_code;
	bool quiet_down;
	bool test_tkip;
#endif // endif
	bool wl_down_triggered;
};

#define TDLS_UAPSD_BUF_STA		0x1
#define TDLS_PEER_PSM			0x2
#define TDLS_CH_SW				0x4
/* #define TDLS_SUPPORT			0x8 */
#define TDLS_PROHIBIT			0x10
#define TDLS_CH_SW_PROHIBIT		0x20

/* scb cubby structure */
typedef struct scb_tdls {
	uint32 flags;		/* flags for the connection */
	uint8 reason;		/* reason code for link tear down */
	uint8 con_token;	/* connection request token */
	uint8 pti_token;
	uint8 pti_token_rx;	/* received PTI req token */
	uint8 sp_token;
	bool free_me;		/* free scb at next watchdog */
	bool timer_start;
	uint32 action_frame_sent_time;
	uint8 anonce[32];
	uint8 snonce[32];
	uint8 mic[16];
	uint8 tpk[TDLS_TPK_LEN];
	rsn_parms_t rsn;
	bool pmkid_cnt;
	uint32 timeout_interval;
	uint16 peer_ies_len;	/* length of saved peer ies */
	uint8 *peer_ies;	/* peer ies */
	wlc_bsscfg_t *parent;	/* Infra STA bsscfg */
	bool ps_allowed;
	tdls_lookaside_t *peer_addr;
	uint8	tid;
	uint16 	seq;
	uint32	apsd_usp_endtime;	/* the time the current APSD USP ends */
	uint32  pti_sent_time;
	chanspec_t base_chanspec;
	chanspec_t cur_chanspec;
	chanspec_t target_chanspec;
	uint16 switch_time;			/* store the max switch_time of TDLS peers */
	uint16 switch_timeout;		/* store the max switch_timeout of TDLS peers */
	int off_chan_sw_cnt;
	uint8		rclen;			/* regulatory class length */
	uint8		rclist[MAXRCLISTSIZE];	/* regulatory class list */
	uint8 heartbeat_missed;
	uint8 heartbeat_countdown;
	struct pktq ubufq;	/* buffered queue */
	uint32	ext_cap_flags;
	uint32	chswreq_sent_time;
	uint	base_chan_bandunit;
	wlc_rateset_t	base_chan_rateset;	/* rateset for TDLS Peer in Base Channel */
	uint32 max_null_attempt_time;
} scb_tdls_t;

/* actual pointer in the cubby */
typedef struct scb_tdls_cubby {
	scb_tdls_t *cubby;
} scb_tdls_cubby_t;

#define SCB_TDLS_CUBBY(tdls, scb) (scb_tdls_cubby_t *)SCB_CUBBY((scb), (tdls)->scb_handle)
#define SCB_TDLS(tdls, scb) (SCB_TDLS_CUBBY(tdls, scb))->cubby

#define TDLS_HEADER_LEN		(sizeof(struct ether_header) + TDLS_PAYLOAD_TYPE_LEN)
#define TDLS_ACTION_FRAME_DEFAULT_LEN		512
#define TDLS_ACTION_FRAME_DISCOVERY_RESP_BASELEN 5 /* action Category, token and capability */

typedef struct tdls_chsw_req {
	uint8 target_ch;
	uint8 regclass;
	uint8 second_ch_offset;
	uint16 switch_time;
	uint16 switch_timeout;
} tdls_chsw_req_t;

typedef struct tdls_chsw_resp {
	uint16 switch_time;
	uint16 switch_timeout;
} tdls_chsw_resp_t;

typedef struct tdls_timer_arg {
	void *hdl;
	int idx;
} tdls_timer_arg_t;

tdls_timer_arg_t timer_arg[TDLS_LOOKASIDE_SIZE];

/* local prototypes */
static int wlc_bsscfg_tdls_init(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg);
static int scb_tdls_init(void *context, struct scb *scb);
static void scb_tdls_deinit(void *context, struct scb *scb);
static int wlc_tdls_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

static void wlc_tdls_buffer_upper(void *hdl, struct scb *scb, void *p, uint prec);
static uint wlc_tdls_txpktcnt_upper(void *hdl);

static void wlc_tdls_qosinfo_update(tdls_info_t *tdls, struct scb *scb, wlc_bss_info_t *bi);

static uint8 *wlc_tdls_write_rsn_ie(tdls_info_t *tdls, uint8 *cp, int buflen, uint32 WPA_auth,
	uint32 wsec, rsn_parms_t *rsn, bool pmkid_cnt);
static uint8 *wlc_tdls_write_ft_ie(tdls_info_t *tdls, struct scb *scb, uint8 *cp, int buflen);
static uint8 *wlc_tdls_write_link_id_ie(tdls_info_t *tdls, struct scb *scb, uint8 *cp, int buflen);
static uint8 *wlc_tdls_write_wmm_ie(tdls_info_t *tdls, struct scb *scb, uint8 subtype,
	uint8 qosinfo, uint8 *cp, int buflen);
static uint8 *wlc_tdls_write_pti_control_ie(tdls_info_t *tdls, struct scb *scb,
	uint8 *cp, int buflen);

static bool wlc_tdls_write_action_frame(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct scb *scb,
	void *p, uint8 action_field, uint16 status_code, uint8 token, void *req);

static void wlc_tdls_send_discovery_req(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst);

static void wlc_tdls_send_setup_req(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst);
static void wlc_tdls_send_setup_resp(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst, uint16 status, uint8 token, link_id_ie_t *link_id);
static void wlc_tdls_send_setup_cfm(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst, uint16 status, uint8 token);
static void wlc_tdls_send_teardown(tdls_info_t *tdls, struct scb *scb, bool direct);
static void wlc_tdls_send_pti_resp(tdls_info_t *tdls, struct scb *scb);
static int wlc_tdls_send_chsw_req(tdls_info_t *tdls, struct scb *scb, chanspec_t chanspec);
static void wlc_tdls_send_chsw_resp(tdls_info_t *tdls, struct scb *scb,
	uint16 status_code, tdls_chsw_resp_t *chsw_resp, bool cb);
static void wlc_tdls_send_tunneled_probe(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst, uint8 type);
static int wlc_tdls_return_to_base_ch(tdls_info_t *tdls, struct scb *scb);
static int wlc_tdls_process_setup_req(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_setup_resp(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_setup_cfm(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_teardown(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_pti(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_pti_resp(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_chsw_req(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);
static int wlc_tdls_process_chsw_resp(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset);

static tdls_lookaside_t *wlc_tdls_lookaside_find(tdls_info_t *tdls,
	struct ether_addr *ea);
static tdls_lookaside_t *wlc_tdls_lookaside_create(tdls_info_t *tdls,
	struct ether_addr *ea, wlc_bsscfg_t *parent, uint8 status);
static void wlc_tdls_lookaside_delete(tdls_info_t *tdls, tdls_lookaside_t *peer);
static int wlc_tdls_lookaside_inittimers(tdls_info_t *tdls);
static void wlc_tdls_lookaside_freetimers(tdls_info_t *tdls);
static tdls_lookaside_t *wlc_tdls_lookaside_move(tdls_info_t *tdls, tdls_lookaside_t *peer);
static void wlc_tdls_lookaside_status_upd(tdls_info_t *tdls, tdls_lookaside_t *peer, uint8 status);

static struct scb *wlc_tdls_scb_create(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *ea, struct ether_addr *bssid, bool initiator);

static void wlc_tdls_disconnect(tdls_info_t *tdls, struct scb *scb, bool force);

static void wlc_tdls_send_action_frame(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *da, void *p, bool direct);

static int wlc_tdls_join(tdls_info_t *tdls, wlc_bss_info_t *bi, struct scb *scb,
	uint8 *body, int body_len);
static void wlc_tdls_peer_timer(void *arg);

static void wlc_tdls_get_pkt(wlc_info_t *wlc, uint len, void **P);

static bool wlc_tdls_cal_mic_chk(tdls_info_t *tdls, struct scb *scb, uchar *tlvs, int tlvs_len,
	uint8 transId);
static bool wlc_tdls_cal_teardown_mic_chk(tdls_info_t *tdls, struct scb *scb, uchar *tlvs,
	int tlvs_len, uint16 reason, bool chk_mic);

static void wlc_tdls_set_pmstate(tdls_info_t *tdls, struct scb *scb, bool state);

static void wlc_tdls_pm_allowed_update(tdls_info_t *tdls, wlc_bsscfg_t *parent);
static bool wlc_tdls_need_pti(tdls_info_t *tdls, struct scb *scb);

static void wlc_tdls_chsw_complete(tdls_info_t *tdls, struct scb *scb);
static void wlc_tdls_delete_chan_ctx(tdls_info_t *tdls, struct scb *scb, chanspec_t chanspec);
static void wlc_tdls_switch_to_target_ch(tdls_info_t *tdls, struct scb *scb, chanspec_t chanspec);
static chanspec_t wlc_tdls_target_chanspec(uint8 target_ch, uint8 second_ch_offset);
static int wlc_tdls_validate_target_chanspec(tdls_info_t *tdls, struct scb *scb,
	chanspec_t target_chanspec);
static int wlc_tdls_chsw_validate_pmstate(tdls_info_t *tdls, struct scb *scb, wlc_bsscfg_t *parent);
static void wlc_tdls_pm_mode_leave(tdls_info_t *tdls, wlc_bsscfg_t *parent);
static void wlc_tdls_chsw_send_nulldata(tdls_info_t *tdls, struct scb *scb, uint32 now);
static void wlc_tdls_pretbtt_update(wlc_info_t *wlc);
#ifdef WLMCHAN
static int wlc_tdls_mchan_change_bss_chan_context(tdls_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t new_chanspec);
#endif // endif

static int wlc_tdls_up(void *hdl);
int wlc_tdls_down(void *hdl);
static void wlc_tdls_watchdog(void *hdl);
static void wlc_tdls_build_ext_cap_ie(tdls_info_t *tdls, wlc_bsscfg_t *cfg,
	uint8 *extcap, uint8 *len);

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(TDLS_TESTBED)
static int wlc_tdls_dump(tdls_info_t *tdls, struct bcmstrbuf *b);
static void wlc_tdls_scb_dump(void *context, struct scb *scb, struct bcmstrbuf *b);
#else
#define wlc_tdls_scb_dump NULL
#endif // endif

static void wlc_tdls_wake_ctrl(wlc_info_t *wlc, tdls_info_t *tdls, struct scb *scb);
/* IE mgmt callbacks */
static uint wlc_tdls_setup_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_ext_cap_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_rsn_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_wme_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_wme_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_coex_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_coex_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_fte_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_fte_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_ft_ti_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_ft_ti_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_link_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_link_id_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_setup_calc_aid_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_setup_write_aid_id_ie(void *ctx, wlc_iem_build_data_t *build);
static int wlc_tdls_setup_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *parse);
static int wlc_tdls_setup_parse_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *parse);
static int wlc_tdls_setup_parse_ft_ti_ie(void *ctx, wlc_iem_parse_data_t *parse);
static int wlc_tdls_setup_parse_fte_ie(void *ctx, wlc_iem_parse_data_t *parse);
static int wlc_tdls_setup_parse_wme_ie(void *ctx, wlc_iem_parse_data_t *parse);
static uint wlc_tdls_disc_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_disc_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_disc_calc_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_ext_cap_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_disc_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_rsn_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_disc_calc_ft_ti_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_ft_ti_ie(void *ctx, wlc_iem_build_data_t *build);
static uint wlc_tdls_disc_calc_link_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_tdls_disc_write_link_id_ie(void *ctx, wlc_iem_build_data_t *build);

static int wlc_tdls_endpoint_op(tdls_info_t *tdls, tdls_iovar_t *info, wlc_bsscfg_t *parent);
static int wlc_tdls_wfd_ie(tdls_info_t *tdls, tdls_wfd_ie_iovar_t *info_in,
	tdls_wfd_ie_iovar_t *info_out, wlc_bsscfg_t *parent, uint8 type);
struct scb * wlc_tdls_scbfind_all_ex(wlc_info_t *wlc, const struct ether_addr *ea);
static struct scb * wlc_tdls_scbfind_all(wlc_info_t *wlc, const struct ether_addr *ea);
static bool wlc_tdls_link_add_ht_op_ie(tdls_info_t *tdls, scb_t	*scb);
static bool wlc_tdls_link_add_vht_op_ie(tdls_info_t *tdls, scb_t *scb);

/* functions for the upper TDLS txmod; This is used to buffer frames at the top
 * during the setup process. To avoid the re-ordering, TDLS initiator and responder
 * need to cease the transmission. After the setup is confirmed, the transmission will
 * resume again.
 */
static txmod_fns_t BCMATTACHDATA(tdls_txmod_upper_fns) = {
	wlc_tdls_buffer_upper,
	wlc_tdls_txpktcnt_upper,
	NULL,
	NULL
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

tdls_info_t *
BCMATTACHFN(wlc_tdls_attach)(wlc_info_t *wlc)
{
	tdls_info_t *tdls;
	int i;

	/* sanity checks */
	ASSERT(wlc->pub->tunables->maxtdls > 0);
	ASSERT((wlc->pub->tunables->maxtdls + WLC_MAXBSSCFG)
	       <= wlc->pub->tunables->maxscb);
	ASSERT((wlc->pub->tunables->maxtdls + 1) <= WLC_MAXBSSCFG);

	if (!(tdls = (tdls_info_t *)MALLOCZ(wlc->osh, sizeof(tdls_info_t)))) {
		WL_ERROR(("wl%d: wlc_tdls_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	if (!(tdls->lookaside = (tdls_lookaside_t *)MALLOCZ(wlc->osh,
		sizeof(tdls_lookaside_t) * TDLS_LOOKASIDE_SIZE))) {
		WL_ERROR(("wl%d: wlc_tdls_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto fail;
	}

#ifdef WLCNT
	if (!(tdls->cnt = (wlc_tdls_cnt_t *)MALLOCZ(wlc->osh, sizeof(wlc_tdls_cnt_t)))) {
		WL_ERROR(("wl%d: wlc_tdls_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /* WLCNT */

	tdls->wlc = wlc;
	tdls->pub = wlc->pub;
	tdls->osh = wlc->pub->osh;

	tdls->ext_cap_flags = (TDLS_CAP_TDLS | TDLS_CAP_PU_BUFFER_STA | TDLS_CAP_CH_SW);
	tdls->max_sessions = wlc->pub->tunables->maxtdls;
	tdls->sa_lifetime = TDLS_SA_LIFETIME_DEFAULT;
	tdls->uapsd_sleep = FALSE;
	tdls->qosinfo = TDLS_QOSINFO_DEFAULT;
	tdls->switch_time = TDLS_SWITCH_TIME_DEFAULT;
	tdls->switch_timeout = TDLS_SWITCH_TIMEOUT_DEFAULT;
	tdls->setup_resp_timeout = TDLS_RESP_TIMEOUT_DEFAULT;
	tdls->disc_window = TDLS_DISCOVERY_REQ_WINDOW;
	tdls->pu_ind_window = TDLS_PU_IND_WINDOW_DEFAULT;
	tdls->rssi_threshold_high = TDLS_RSSI_THRESHOLD_HIGH;
	tdls->rssi_threshold_low =  TDLS_RSSI_THRESHOLD_LOW;
	tdls->trigger_pktcnt_high = TDLS_TRIGGER_PKTCNT_HIGH;
	tdls->trigger_pktcnt_low = TDLS_TRIGGER_PKTCNT_LOW;
	tdls->idle_time = TDLS_LOOKASIDE_DEFAULT_IDLE_TIME;
	/* reserve cubby in the scb container for per-scb private data */
	tdls->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(scb_tdls_cubby_t),
		scb_tdls_init, scb_tdls_deinit, wlc_tdls_scb_dump, (void *)tdls);

	if (tdls->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, tdls_iovars, "tdls",
		tdls, wlc_tdls_doiovar, wlc_tdls_watchdog, wlc_tdls_up, wlc_tdls_down)) {
		WL_ERROR(("wl%d: TDLS wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	wlc_txmod_fn_register(wlc, TXMOD_TDLS, tdls, tdls_txmod_upper_fns);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)|| defined(TDLS_TESTBED)
	wlc_dump_register(wlc->pub, "tdls", (dump_fn_t)wlc_tdls_dump, (void *)tdls);
#endif // endif

#ifdef TDLS_TESTBED
	memset(&tdls->test_BSSID, 0x32, ETHER_ADDR_LEN);
#endif // endif

	tdls->wfd_ie_setup_tx_length = 0;
	tdls->wfd_ie_probe_tx_length = 0;

	/* try to set TDLS to the default value */
	wlc_tdls_set(tdls, wlc->pub->_tdls);

	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		timer_arg[i].hdl = tdls;
		timer_arg[i].idx = i;
	}

	if (wlc_tdls_lookaside_inittimers(tdls) != BCME_OK) {
		goto fail;
	}

	/* register IE mgmt callbacks */
	/* calc/build setupreq */
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_RATES_ID,
	      wlc_tdls_setup_calc_sup_rates_ie_len, wlc_tdls_setup_write_sup_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, sup rates in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_EXT_RATES_ID,
	      wlc_tdls_setup_calc_ext_rates_ie_len, wlc_tdls_setup_write_ext_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext rates in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_EXT_CAP_ID,
	      wlc_tdls_setup_calc_ext_cap_ie_len, wlc_tdls_setup_write_ext_cap_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext cap in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_RSN_ID,
	      wlc_tdls_setup_calc_rsn_ie_len, wlc_tdls_setup_write_rsn_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, rsn in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_vs_add_build_fn(wlc->ier_tdls_srq, WLC_IEM_VS_IE_PRIO_WME,
	      wlc_tdls_setup_calc_wme_ie_len, wlc_tdls_setup_write_wme_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_vs_add_build_fn failed, wme in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_HT_BSS_COEXINFO_ID,
	      wlc_tdls_setup_calc_coex_ie_len, wlc_tdls_setup_write_coex_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, 20/40 coex in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_FTIE_ID,
	      wlc_tdls_setup_calc_fte_ie_len, wlc_tdls_setup_write_fte_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_FT_TI_ID,
	      wlc_tdls_setup_calc_ft_ti_ie_len, wlc_tdls_setup_write_ft_ti_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT TI in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_LINK_IDENTIFIER_ID,
	      wlc_tdls_setup_calc_link_id_ie_len, wlc_tdls_setup_write_link_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, link id in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_AID_ID,
	      wlc_tdls_setup_calc_aid_id_ie_len, wlc_tdls_setup_write_aid_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, assoc id in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* calc/build setupresp */
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_RATES_ID,
	      wlc_tdls_setup_calc_sup_rates_ie_len, wlc_tdls_setup_write_sup_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, sup rates in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_EXT_RATES_ID,
	      wlc_tdls_setup_calc_ext_rates_ie_len, wlc_tdls_setup_write_ext_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext rates in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_EXT_CAP_ID,
	      wlc_tdls_setup_calc_ext_cap_ie_len, wlc_tdls_setup_write_ext_cap_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext cap in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_RSN_ID,
	      wlc_tdls_setup_calc_rsn_ie_len, wlc_tdls_setup_write_rsn_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, rsn in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_vs_add_build_fn(wlc->ier_tdls_srs, WLC_IEM_VS_IE_PRIO_WME,
	      wlc_tdls_setup_calc_wme_ie_len, wlc_tdls_setup_write_wme_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_vs_add_build_fn failed, wme in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_HT_BSS_COEXINFO_ID,
	      wlc_tdls_setup_calc_coex_ie_len, wlc_tdls_setup_write_coex_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, 20/40 coex in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_FTIE_ID,
	      wlc_tdls_setup_calc_fte_ie_len, wlc_tdls_setup_write_fte_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_FT_TI_ID,
	      wlc_tdls_setup_calc_ft_ti_ie_len, wlc_tdls_setup_write_ft_ti_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT TI in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_LINK_IDENTIFIER_ID,
	      wlc_tdls_setup_calc_link_id_ie_len, wlc_tdls_setup_write_link_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, link id in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_AID_ID,
	      wlc_tdls_setup_calc_aid_id_ie_len, wlc_tdls_setup_write_aid_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, assoc id in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* calc/build setupconfirm */
	if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_RSN_ID,
	      wlc_tdls_setup_calc_rsn_ie_len, wlc_tdls_setup_write_rsn_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, rsn in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_FTIE_ID,
	      wlc_tdls_setup_calc_fte_ie_len, wlc_tdls_setup_write_fte_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_FT_TI_ID,
	      wlc_tdls_setup_calc_ft_ti_ie_len, wlc_tdls_setup_write_ft_ti_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT TI in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_vs_add_build_fn(wlc->ier_tdls_scf, WLC_IEM_VS_IE_PRIO_WME,
	      wlc_tdls_setup_calc_wme_ie_len, wlc_tdls_setup_write_wme_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_vs_add_build_fn failed, wme in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_LINK_IDENTIFIER_ID,
	      wlc_tdls_setup_calc_link_id_ie_len, wlc_tdls_setup_write_link_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, link id in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* calc/build discresp */
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_RATES_ID,
	      wlc_tdls_disc_calc_sup_rates_ie_len, wlc_tdls_disc_write_sup_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, sup rates in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_EXT_RATES_ID,
	      wlc_tdls_disc_calc_ext_rates_ie_len, wlc_tdls_disc_write_ext_rates_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext rates in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_EXT_CAP_ID,
	      wlc_tdls_disc_calc_ext_cap_ie_len, wlc_tdls_disc_write_ext_cap_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, ext cap in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_RSN_ID,
	      wlc_tdls_disc_calc_rsn_ie_len, wlc_tdls_disc_write_rsn_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, rsn in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_FT_TI_ID,
	      wlc_tdls_disc_calc_ft_ti_ie_len, wlc_tdls_disc_write_ft_ti_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, FT TI in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_LINK_IDENTIFIER_ID,
	      wlc_tdls_disc_calc_link_id_ie_len, wlc_tdls_disc_write_link_id_ie,
	      tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, link id in discresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* parse setupreq */
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_RATES_ID,
	                         wlc_tdls_setup_parse_sup_rates_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, sup rates in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_FT_TI_ID,
	                         wlc_tdls_setup_parse_ft_ti_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft ti in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_FTIE_ID,
	                         wlc_tdls_setup_parse_fte_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_EXT_CAP_ID,
	                         wlc_tdls_setup_parse_ext_cap_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ext cap in setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_ier_vs_add_parse_fn(wlc->ier_tdls_srq, WLC_IEM_VS_IE_PRIO_WME,
	                         wlc_tdls_setup_parse_wme_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, wme setupreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* parse setupresp */
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_RATES_ID,
	                         wlc_tdls_setup_parse_sup_rates_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, sup rates in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_FT_TI_ID,
	                         wlc_tdls_setup_parse_ft_ti_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft ti in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_FTIE_ID,
	                         wlc_tdls_setup_parse_fte_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_EXT_CAP_ID,
	                         wlc_tdls_setup_parse_ext_cap_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ext cap in setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_ier_vs_add_parse_fn(wlc->ier_tdls_srs, WLC_IEM_VS_IE_PRIO_WME,
	                         wlc_tdls_setup_parse_wme_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, wme setupresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* parse setupconfirm */
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_RATES_ID,
	                         wlc_tdls_setup_parse_sup_rates_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, sup rates in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_FT_TI_ID,
	                         wlc_tdls_setup_parse_ft_ti_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft ti in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_FTIE_ID,
	                         wlc_tdls_setup_parse_fte_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft in setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_vs_add_parse_fn(wlc->ier_tdls_scf, WLC_IEM_VS_IE_PRIO_WME,
	                         wlc_tdls_setup_parse_wme_ie, tdls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, wme setupconfirm\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc_bsscfg_type_register(wlc, BSSCFG_TYPE_TDLS,
		(bsscfg_type_init_t)wlc_bsscfg_tdls_init, tdls);
	return tdls;

fail:
	if (tdls->lookaside) wlc_tdls_lookaside_freetimers(tdls);
#ifdef WLCNT
	if (tdls->cnt)
		MFREE(tdls->wlc->osh, tdls->cnt, sizeof(wlc_tdls_cnt_t));
#endif /* WLCNT */
	if (tdls->lookaside) MFREE(wlc->osh, tdls->lookaside,
		sizeof(tdls_lookaside_t) * TDLS_LOOKASIDE_SIZE);
	MFREE(wlc->osh, tdls, sizeof(tdls_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_tdls_detach)(tdls_info_t *tdls)
{
	int i;

	if (!tdls)
		return;

	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		if (tdls->lookaside[i].flags & TDLS_LOOKASIDE_INUSE)
			wlc_tdls_lookaside_delete(tdls, &tdls->lookaside[i]);
	}

	/* sanity */
	wlc_module_unregister(tdls->pub, "tdls", tdls);

#ifdef WLCNT
	if (tdls->cnt)
		MFREE(tdls->wlc->osh, tdls->cnt, sizeof(wlc_tdls_cnt_t));
#endif /* WLCNT */
	wlc_tdls_lookaside_freetimers(tdls);
	MFREE(tdls->wlc->osh, tdls->lookaside,
		sizeof(tdls_lookaside_t) * TDLS_LOOKASIDE_SIZE);
	MFREE(tdls->wlc->osh, tdls, sizeof(tdls_info_t));
}

/* Global scb lookup */
static struct scb *
wlc_tdls_scbfindband_all(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit)
{
	int32 idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	FOREACH_BSS(wlc, idx, cfg) {
		scb = wlc_scbfindband(wlc, cfg, ea, bandunit);
		if (scb != NULL)
			break;
	}

	return scb;
}

/* Wrapper to save ROM invalidation(Rb:86978) - if we change static to global */
struct scb *
wlc_tdls_scbfind_all_ex(wlc_info_t *wlc, const struct ether_addr *ea)
{
	return wlc_tdls_scbfind_all(wlc, ea);
}

/* Global find of station control block corresponding to the remote id */
static struct scb *
wlc_tdls_scbfind_all(wlc_info_t *wlc, const struct ether_addr *ea)
{
	int32 idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	FOREACH_BSS(wlc, idx, cfg) {
		scb = wlc_scbfind(wlc, cfg, ea);
		if (scb != NULL)
			break;
	}

	return scb;
}

static void wlc_tdls_wake_ctrl(wlc_info_t *wlc, tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;
	uint32 flags;
	wlc_bsscfg_t *bsscfg;

	ASSERT(scb != NULL);
	scb_tdls = SCB_TDLS(tdls, scb);
	flags = scb_tdls->flags;
	bsscfg = SCB_BSSCFG(scb);

	/* Note: TDLS_SCB_SENT_PTI_RESP need hot be handled below because that
	 * will start a STA USP
	 */
	bsscfg->tdls->tdls_PMAwake = ((flags & TDLS_SCB_SENT_PTI) ||
		(flags & TDLS_SCB_SENT_CHSW_REQ) ||
		(flags & TDLS_SCB_SENT_CHSW_RESP) ||
		(flags & TDLS_SCB_WAIT_SWITCHTIME) ||
		(bsscfg->pm->PMenabled && bsscfg->pm->PMpending));

	wlc_set_wake_ctrl(wlc);
}

static void
wlc_tdls_bsscfg_free(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, wlc_bsscfg_t *parent)
{
	ASSERT(bsscfg);
	ASSERT(parent != NULL);

	/* cleanup the sup/auth */
	/* XXX TDLS manages auth/sup itself and doesn't follow
	 * AP/authenticator and STA/supplicant rule therefore
	 * freeing any auth/sup itself.
	 * When there is a need to support IBSS 4-way handshake
	 * in the driver move this few lines to wlc_bsscfg.c.
	 */
#if defined(BCMAUTH_PSK)
	if (bsscfg->authenticator) {
		wlc_authenticator_detach(bsscfg->authenticator);
		bsscfg->authenticator = NULL;
	}
#endif // endif

	/* XXX: we are freeing bsscfg here. There is a possible race
	 * in future if we ever use bsscfg in dotxstatus
	 */
	wlc_bsscfg_free(tdls->wlc, bsscfg);

	wlc_set_wake_ctrl(tdls->wlc);

	/* wlc_set_ps_ctrl(parent); */

	wlc_tdls_pm_allowed_update(tdls, parent);

	wlc_set_pmstate(parent, parent->pm->PMenabled);

}

/* scb cubby init fn */
static int
scb_tdls_init(void *context, struct scb *scb)
{
	return 0;
}

/* scb cubby deinit fn */
static void
scb_tdls_deinit(void *context, struct scb *scb)
{
	tdls_info_t *tdls = (tdls_info_t *)context;
	wlc_info_t *wlc = tdls->wlc;
	wlc_bsscfg_t *bsscfg = scb->bsscfg;
	scb_tdls_cubby_t *scb_tdls_cubby;
	scb_tdls_t *scb_tdls;
	tdls_lookaside_t *peer;
	wlc_tdls_t *tc;

	if (!bsscfg || !BSSCFG_IS_TDLS(bsscfg))
		return;

	WL_TDLS(("wl%d: scb_tdls_deinit: enter\n", tdls->pub->unit));

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls != NULL);
	if (scb_tdls == NULL)
		return;

	tc = bsscfg->tdls;
	if (tc->tdls_PMEnable) {
		tc->tdls_PMEnable = FALSE;
		wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);
		WL_TDLS(("wl%d:%s(): wake up, "
			"(send NULL data peer).\n",
			tdls->pub->unit, __FUNCTION__));
		wlc_set_pmstate(scb_tdls->parent, FALSE);
	}

#ifdef WLMCHAN
	if ((scb_tdls->target_chanspec != scb_tdls->base_chanspec) &&
		(scb_tdls->cur_chanspec != scb_tdls->target_chanspec)) {
		/* need to fix the bsscfg->chan_ctxt, not to free the primary's chan_ctxt */
		wlc_mchan_create_bss_chan_context(wlc, bsscfg, scb_tdls->target_chanspec);
	}
#endif /* WLMCHAN */

	if (bsscfg->current_bss->bcn_prb) {
		bsscfg->current_bss->bcn_prb = NULL;
		bsscfg->current_bss->bcn_prb_len = 0;
#ifdef WLMCNX
		/* P2P: reset the BSS TSF */
		wlc_mcnx_reset_bss(wlc->mcnx, bsscfg);
#endif // endif
	}

	if (pktq_len(&scb_tdls->ubufq)) {
		WL_TDLS(("wl%d: flushing %d frames from ubufq\n",
		tdls->pub->unit, pktq_len(&scb_tdls->ubufq)));
		pktq_flush(wlc->osh, &scb_tdls->ubufq, TRUE, NULL, 0);
	}

	if (scb_tdls->peer_ies) {
		MFREE(wlc->osh, scb_tdls->peer_ies, scb_tdls->peer_ies_len);
		scb_tdls->peer_ies = NULL;
	}

	tdls->cur_sessions--;

	/* free the cubby structure */
	scb_tdls_cubby = SCB_TDLS_CUBBY(tdls, scb);
	MFREE(wlc->osh, scb_tdls_cubby->cubby, sizeof(scb_tdls_t));
	scb_tdls_cubby->cubby = NULL; /* scb_tdls = NULL, should not be referenced */

	/* change the entry status in TDLS lookaside table */
	peer = wlc_tdls_lookaside_find(tdls, &scb->ea);
	ASSERT(peer);
	if (peer && (peer->status == TDLS_LOOKASIDE_ACTIVE))
		wlc_tdls_lookaside_delete(tdls, peer);

}

/* frees all the buffers and cleanup everything on down */
int
wlc_tdls_down(void *hdl)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	struct scb_iter scbiter;
	int i;

	if (!TDLS_SUPPORT(wlc->pub) && !tdls->up)
		return BCME_OK;

	tdls->up = FALSE;
	tdls->wl_down_triggered = TRUE;

	/* free all TDLS scbs; deinit function frees up the bsscfgs */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (BSSCFG_IS_TDLS(scb->bsscfg))
			wlc_tdls_disconnect(tdls, scb, TRUE);
	}

	/* cleanup the lookaside entries */
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++)
		if (tdls->lookaside[i].flags & TDLS_LOOKASIDE_INUSE)
			wlc_tdls_lookaside_delete(tdls, &tdls->lookaside[i]);

	return 0;
}

/* Setup Request frame IEs' order */
static const uint8 BCMINITDATA(srq_ie_tags)[] = {
	DOT11_MNG_RATES_ID,
	DOT11_MNG_COUNTRY_ID,
	DOT11_MNG_EXT_RATES_ID,
	DOT11_MNG_SUPP_CHANNELS_ID,
	DOT11_MNG_RSN_ID,
	DOT11_MNG_EXT_CAP_ID,
	DOT11_MNG_QOS_CAP_ID,	/* IEEE 802.11 - 2012 */
	DOT11_MNG_FTIE_ID,
	DOT11_MNG_FT_TI_ID,
	DOT11_MNG_REGCLASS_ID,
	DOT11_MNG_HT_CAP,
	DOT11_MNG_HT_BSS_COEXINFO_ID,
	DOT11_MNG_LINK_IDENTIFIER_ID,
	DOT11_MNG_AID_ID,
	DOT11_MNG_VHT_CAP_ID,
	DOT11_MNG_VS_ID	/* WFA */
};

/* Setup Response frame IEs' order */
static const uint8 BCMINITDATA(srs_ie_tags)[] = {
	DOT11_MNG_RATES_ID,
	DOT11_MNG_COUNTRY_ID,
	DOT11_MNG_EXT_RATES_ID,
	DOT11_MNG_SUPP_CHANNELS_ID,
	DOT11_MNG_RSN_ID,
	DOT11_MNG_EXT_CAP_ID,
	DOT11_MNG_QOS_CAP_ID,	/* IEEE 802.11 - 2012 */
	DOT11_MNG_FTIE_ID,
	DOT11_MNG_FT_TI_ID,
	DOT11_MNG_REGCLASS_ID,
	DOT11_MNG_HT_CAP,
	DOT11_MNG_HT_BSS_COEXINFO_ID,
	DOT11_MNG_LINK_IDENTIFIER_ID,
	DOT11_MNG_AID_ID,
	DOT11_MNG_VHT_CAP_ID,
	DOT11_MNG_OPER_MODE_NOTIF_ID,
	DOT11_MNG_VS_ID	/* WFA */
};

/* Setup Confirm frame IEs' order */
static const uint8 BCMINITDATA(scf_ie_tags)[] = {
	DOT11_MNG_RSN_ID,
	DOT11_MNG_EDCA_PARAM_ID,	/* IEEE 802.11 - 2012 */
	DOT11_MNG_FTIE_ID,
	DOT11_MNG_FT_TI_ID,
#ifdef IEEE2012_TDLSSEPC
	DOT11_MNG_HT_ADD,	/* IEEE 802.11 - 2012 */
#else
	DOT11_MNG_HT_CAP,	/* WFA */
#endif // endif
	DOT11_MNG_LINK_IDENTIFIER_ID,
	DOT11_MNG_VHT_OPERATION_ID,
	DOT11_MNG_OPER_MODE_NOTIF_ID,
	DOT11_MNG_VS_ID	/* WFA */
};

/* Discovery Response frame IEs' order */
static const uint8 BCMINITDATA(drs_ie_tags)[] = {
	DOT11_MNG_RATES_ID,
	DOT11_MNG_EXT_RATES_ID,
	DOT11_MNG_SUPP_CHANNELS_ID,
	DOT11_MNG_RSN_ID,
	DOT11_MNG_EXT_CAP_ID,
	DOT11_MNG_FTIE_ID,
	DOT11_MNG_FT_TI_ID,
	DOT11_MNG_REGCLASS_ID,
	DOT11_MNG_HT_CAP,
	DOT11_MNG_HT_BSS_COEXINFO_ID,
	DOT11_MNG_LINK_IDENTIFIER_ID,
	DOT11_MNG_VHT_CAP_ID,
	DOT11_MNG_VS_ID
};

static int
wlc_tdls_up(void *hdl)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	wlc_info_t *wlc = tdls->wlc;

	if (!TDLS_SUPPORT(wlc->pub))
		return BCME_OK;

	if (!tdls->up) {
		/* sort calc_len/build callbacks */
		/* ignore the return code */
		(void)wlc_ier_sort_cbtbl(wlc->ier_tdls_srq, srq_ie_tags, sizeof(srq_ie_tags));
		(void)wlc_ier_sort_cbtbl(wlc->ier_tdls_srs, srs_ie_tags, sizeof(srs_ie_tags));
		(void)wlc_ier_sort_cbtbl(wlc->ier_tdls_scf, scf_ie_tags, sizeof(scf_ie_tags));
		(void)wlc_ier_sort_cbtbl(wlc->ier_tdls_drs, drs_ie_tags, sizeof(drs_ie_tags));
	}
	tdls->wl_down_triggered = FALSE;
	tdls->up = TRUE;

	return 0;
}

void
wlc_tdls_cleanup(tdls_info_t *tdls, wlc_bsscfg_t *parent)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (BSSCFG_IS_TDLS(scb->bsscfg) &&
		    wlc_tdls_get_parent_bsscfg(wlc, scb) == parent) {
			wlc_tdls_disconnect(tdls, scb, TRUE);
		}
	}
}

static void
_wlc_tdls_free_scb(wlc_info_t *wlc, struct scb *scb)
{
#ifdef WLAMPDU
	/* Flush AMPDU RX/TX queues before TDLS cubby gets freed */
	if (SCB_AMPDU(scb)) {
		wlc_scb_ampdu_disable(wlc, scb);
	}
#endif // endif
	wlc_scbfree(wlc, scb);
}

bool wlc_tdls_buffer_sta_enable(tdls_info_t *tdls)
{
	return (tdls->ext_cap_flags & TDLS_CAP_PU_BUFFER_STA);
}

bool wlc_tdls_sleep_sta_enable(tdls_info_t *tdls)
{
	return tdls->uapsd_sleep;
}

void
wlc_tdls_update_tid_seq(tdls_info_t *tdls, struct scb *scb, uint8 tid, uint16 seq)
{
	scb_tdls_t *scb_tdls;

	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);

	scb_tdls->tid = tid;
	scb_tdls->seq = seq;
}

/* TD:S buffering function called through txmod */
static void
wlc_tdls_buffer_upper(void *hdl, struct scb *scb, void *p, uint prec)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

	ASSERT(scb_tdls != NULL);

	if (pktq_full(&scb_tdls->ubufq)) {
		WL_ERROR(("wl%d: wlc_tdls_buffer: ubufq full; Discarding\n",
			tdls->pub->unit));

		PKTFREE(tdls->osh, p, TRUE);
		WLCNTINCR(tdls->pub->_cnt->txnobuf);
		WLCNTINCR(tdls->cnt->discard);
		return;
	}

	pktenq(&scb_tdls->ubufq, p);
	WLCNTINCR(tdls->cnt->ubuffered);
}

/* Return the buffered packets held by TDLS */
static uint
wlc_tdls_txpktcnt_upper(void *hdl)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	struct scb_iter scbiter;
	int pktcnt = 0;
	scb_tdls_t *scb_tdls;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (BSSCFG_IS_TDLS(scb->bsscfg)) {
			scb_tdls = SCB_TDLS(tdls, scb);
			ASSERT(scb_tdls);
			pktcnt += pktq_len(&scb_tdls->ubufq);
		}
	}

	return pktcnt;
}

static void
wlc_tdls_listen_on_target_ch(tdls_info_t *tdls, scb_tdls_t *scb_tdls)
{
	int delta;
	uint32 now = OSL_SYSUPTIME();

	delta = (int)(now - scb_tdls->action_frame_sent_time);
	if (delta < scb_tdls->switch_timeout/1000) {
		delta = scb_tdls->switch_timeout/1000 - delta;
		scb_tdls->flags |= TDLS_SCB_LISTEN_ON_TARGET_CH;
		wl_add_timer(tdls->wlc->wl, scb_tdls->peer_addr->timer, delta, 0);
		scb_tdls->timer_start = TRUE;
	}
}

static int
wlc_tdls_setup_validate_parent(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	wlc_bss_info_t *bi = bsscfg->current_bss;

	if (!bsscfg->BSS) {
		WL_ERROR(("wl%d: BSS is not infra BSS!\n",
		tdls->pub->unit));
		return BCME_ERROR;
	}

	if (!bsscfg->associated) {
		WL_ERROR(("wl%d: BSS is not associted!\n",
		tdls->pub->unit));
		return BCME_NOTASSOCIATED;
	}

	scb = wlc_tdls_scbfindband_all(wlc, &bi->BSSID, CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
	if (!scb)
		return BCME_EPERM;

	/* XXXXX: prohibited TDLS check */
	if (scb->flags2 & SCB2_TDLS_PROHIBIT) {
		WL_ERROR(("wl%d: TDLS is porhibited in BSS!\n",
			tdls->pub->unit));
#ifdef TDLS_TESTBED
		if (!tdls->test_prohibit)
#endif // endif
		return DOT11_SC_DECLINED;
	}

	/* security */
	if (WSEC_ENABLED(bsscfg->wsec)) {
		struct rsn_parms *rsn;

#ifdef TDLS_TESTBED
		if (tdls->test_tkip) return 0;
#endif // endif

		if (!(bsscfg->wsec & AES_ENABLED)) {
			WL_ERROR(("wl%d: AP has weak security!\n", tdls->pub->unit));
			return DOT11_SC_TDLS_SEC_DISABLED;
		}

		if (!(bi->wpa2.flags & RSN_FLAGS_SUPPORTED)) {
			WL_ERROR(("wl%d: AP does not have RSN IE!\n",
			tdls->pub->unit));
			/* return DOT11_SC_INVALID_PARAMS; */
			return 0;
		}

		rsn = &(bi->wpa2);
		if (/*MCAST_TKIP(rsn) || */MCAST_WEP(rsn) ||
			/* UCAST_TKIP(rsn) || */UCAST_WEP(rsn)) {
			WL_ERROR(("wl%d: TKIP/WEP in mcast/ucast cipher!\n",
			tdls->pub->unit));
			return DOT11_SC_DECLINED;
		}
		if (UCAST_NONE(rsn)) {
			WL_ERROR(("wl%d: group cipher used as ucast cipher!\n",
			tdls->pub->unit));
			return DOT11_SC_DECLINED;
		}
	}

	return 0;
}

static int wlc_tdls_endpoint_op(tdls_info_t *tdls, tdls_iovar_t *info, wlc_bsscfg_t *parent)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	struct scb *scb_i;
	scb_tdls_t *scb_tdls;
	struct scb_iter scbiter;
	int err = 0;

	if (ETHER_ISMULTI(&info->ea) && info->mode != TDLS_MANUAL_EP_WFD_TPQ) {
		WL_ERROR(("wl%d:%s(): Peer MAC cannot be a multi-cast address!\n", wlc->pub->unit,
			__FUNCTION__));
		return BCME_BADADDR;
	}
	if (!ether_cmp(&info->ea, &parent->current_bss->BSSID)) {
		WL_ERROR(("wl%d:%s(): Peer MAC cannot be the serving AP!\n", wlc->pub->unit,
			__FUNCTION__));
		return BCME_BADADDR;
	}
	switch (info->mode) {
		case TDLS_MANUAL_EP_CREATE: {
			err = wlc_tdls_setup_validate_parent(tdls, parent);
			if (err) {
				break;
			}

			scb = wlc_tdls_scbfindband_all(wlc, &info->ea,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

			if (scb) {
				scb_tdls = SCB_TDLS(tdls, scb);
				if (!scb_tdls) {
					WL_ERROR(("wl%d:%s(): scb is in use, free it...\n",
						wlc->pub->unit, __FUNCTION__));
					_wlc_tdls_free_scb(tdls->wlc, scb);
				}
				else {
					if (scb_tdls->parent != parent) {
						WL_ERROR(("wl%d:%s(): scb not in the same BSS.\n",
						wlc->pub->unit, __FUNCTION__));
						err = BCME_ERROR;
						break;
					}

					if (scb_tdls->flags & TDLS_SCB_CON_PENDING) {
						WL_ERROR(("wl%d:%s(): pending setup req exists.\n",
							wlc->pub->unit, __FUNCTION__));
						err = BCME_ERROR;
						break;
					}

					/* if the TDLS session already connected, */
					/* tear it down first */
					if (scb_tdls->flags & TDLS_SCB_CONNECTED) {
						WL_TDLS(("wl%d:%s(): Direct link is up,"
							" tear down first.\n",
							wlc->pub->unit, __FUNCTION__));
						wlc_tdls_disconnect(tdls, scb, FALSE);
					}
				}
			}

			/* check to see if there is any other pending TDLS */
			/* initiator in the same BSS allow one each time */
			FOREACHSCB(wlc->scbstate, &scbiter, scb_i) {
				scb_tdls = SCB_TDLS(tdls, scb_i);
				if (scb_tdls && (scb_tdls->parent == parent)) {
					if ((scb_i->bsscfg->tdls->initiator) &&
						(scb_tdls->flags & TDLS_SCB_CON_PENDING)) {
						WL_ERROR(("wl%d:%s(): pending setup "
					          "request exists.\n",
					          wlc->pub->unit, __FUNCTION__));
						err = BCME_ERROR;
						break;
					}
				}
			}
			if (err == BCME_ERROR)
				break;

			/* create a new tdls scb */
			scb = wlc_tdls_scb_create(tdls, parent, &info->ea,
				&parent->BSSID, TRUE);

			if (!scb) {
				WL_ERROR(("wl%d: could not create scb\n",
					wlc->pub->unit));
				err = BCME_BADARG;
				break;
			}
			scb_tdls = SCB_TDLS(tdls, scb);
			ASSERT(scb_tdls != NULL);

			if (!tdls->auto_op)
				scb_tdls->flags |= TDLS_SCB_MANUAL;
			else
				scb_tdls->flags &= ~TDLS_SCB_MANUAL;

			/* send TDLS setup request */
			wlc_tdls_send_setup_req(tdls, scb_tdls->parent, &scb->ea);

			break;

		}

		case  TDLS_MANUAL_EP_DELETE: {
			scb = wlc_tdls_scbfindband_all(wlc, &info->ea,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

			if (!scb || !BSSCFG_IS_TDLS(scb->bsscfg)) {
				WL_ERROR(("wl%d:%s(): could not find scb\n",
					wlc->pub->unit, __FUNCTION__));
				err = BCME_BADARG;
				break;
			}

			scb_tdls = SCB_TDLS(tdls, scb);
			ASSERT(scb_tdls != NULL);

			WL_TDLS(("wl%d:%s(): deleting peer...!\n",
				wlc->pub->unit, __FUNCTION__));

			scb_tdls->flags &= ~TDLS_SCB_MANUAL;
			wlc_tdls_disconnect(tdls, scb, TRUE);

			break;

		}

		case TDLS_MANUAL_EP_PM:
		case TDLS_MANUAL_EP_WAKE: {
			wlc_pm_st_t *pm;
			wlc_tdls_t *tc;

			UNUSED_PARAMETER(pm);
			scb = wlc_tdls_scbfindband_all(wlc, &info->ea,
				CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

			if (!scb || !BSSCFG_IS_TDLS(scb->bsscfg)) {
				WL_ERROR(("wl%d: could not find scb\n",
					wlc->pub->unit));
				err = BCME_BADARG;
				break;
			}

			scb_tdls = SCB_TDLS(tdls, scb);
			ASSERT(scb_tdls != NULL);

			if (!(scb_tdls->flags & TDLS_SCB_CONNECTED)) {
				WL_TDLS(("wl%d:%s(): Direct link is not connected!\n",
					wlc->pub->unit, __FUNCTION__));
				err = BCME_NOTASSOCIATED;
				break;
			}

			if (!scb_tdls->ps_allowed) {
				WL_TDLS(("wl%d:%s(): TDLS sleep STA not supported!\n",
					wlc->pub->unit, __FUNCTION__));
				err = BCME_UNSUPPORTED;
				break;
			}

			pm = scb_tdls->parent->pm;
			tc = scb->bsscfg->tdls;

			if (info->mode == TDLS_MANUAL_EP_PM)
				tc->tdls_PMEnable = TRUE;
			else
				tc->tdls_PMEnable = FALSE;

			wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);
			WL_TDLS(("wl%d:%s(): set PM state = %s of parent BSS"
				"(send NULL data to AP) first!\n", wlc->pub->unit, __FUNCTION__,
				tc->tdls_PMEnable ? "TRUE" : "FALSE"));
			wlc_set_pmstate(scb_tdls->parent, tc->tdls_PMEnable);
			WL_TDLS(("wl%d:%s(): set PM state = %s of TDLS BSS"
				"(send NULL data peer).\n", wlc->pub->unit, __FUNCTION__,
				tc->tdls_PMEnable ? "TRUE" : "FALSE"));
			wlc_tdls_set_pmstate(tdls, scb, tc->tdls_PMEnable);

			break;

		}

		case TDLS_MANUAL_EP_DISCOVERY: {

			wlc_tdls_send_discovery_req(tdls, wlc->cfg, &info->ea);

			break;
		}

		case TDLS_MANUAL_EP_CHSW: {
			struct scb *parent_scb;
			scb_tdls_t *case_local_scb_tdls = NULL;

			WL_TDLS(("wl%d:%s(): TDLS channel switch Req: "
				"target_chanspec = 0x%04x\n",
				wlc->pub->unit, __FUNCTION__, info->chanspec));

			scb = wlc_tdls_scbfind_all(wlc, &info->ea);
			if (!scb || !BSS_TDLS_ENAB(wlc, scb->bsscfg)) {
				if (NBANDS(wlc) > 1)
					scb = wlc_tdls_scbfindband_all(wlc, &info->ea,
					                               OTHERBANDUNIT(wlc));
				if (!scb || !BSS_TDLS_ENAB(wlc, scb->bsscfg)) {
					WL_TDLS(("wl%d:%s(): TDLS LINK not established.\n",
						wlc->pub->unit, __FUNCTION__));
					err = BCME_BADADDR;
					break;
				}
			}

			case_local_scb_tdls = SCB_TDLS(tdls, scb);
			if (!case_local_scb_tdls) {
				WL_TDLS(("wl%d:%s(): SCB not TDLS!\n",
					wlc->pub->unit, __FUNCTION__));
				err = BCME_ERROR;
				break;
			}

			parent_scb = wlc_tdls_scbfindband_all(wlc, &parent->BSSID,
				CHSPEC_WLCBANDUNIT(case_local_scb_tdls->base_chanspec));
			if (!parent_scb) {
				WL_TDLS(("wl%d:%s(): parent SCB not found!\n",
					wlc->pub->unit, __FUNCTION__));
				err = BCME_BADADDR;
				break;
			}

			if ((parent_scb->flags2 & SCB2_TDLS_CHSW_PROHIBIT) &&
#ifdef TDLS_TESTBED
				!(tdls->test_prohibit) &&
#endif // endif
				TRUE) {
				WL_TDLS(("wl%d:%s(): TDLS channel switch is prohibitted "
					" by AP.\n", wlc->pub->unit, __FUNCTION__));
				err = BCME_UNSUPPORTED;
				break;
			}

			if (info->chanspec) {
				/* Channel Switch Request to off channel */
				err = wlc_tdls_validate_target_chanspec(wlc->tdls, scb,
					info->chanspec);
				if (err) {
					WL_TDLS(("wl%d:%s(): channel switch: invalid "
						"target channel.\n",
						wlc->pub->unit, __FUNCTION__));
					break;
				}

				err = wlc_tdls_send_chsw_req(tdls, scb, info->chanspec);
				if (err) {
					WL_TDLS(("wl%d:%s(): channel switch: send channel"
						" switch request failed.\n",
						wlc->pub->unit, __FUNCTION__));
					break;
				}
#ifdef TDLS_TESTBED
				if (tdls->chsw_mode == TDLS_CHSW_SEND_REQ)
					break;
#endif // endif

				SCB_BSSCFG(scb)->tdls->chsw_req_enabled = TRUE;
				if (info->chanspec == case_local_scb_tdls->base_chanspec)
					SCB_BSSCFG(scb)->tdls->chsw_req_enabled = FALSE;
			}
			else {
				if (!SCB_BSSCFG(scb)->tdls->chsw_req_enabled) {
					WL_TDLS(("wl%d:%s(): channel switch not enabled.",
					wlc->pub->unit, __FUNCTION__));
				}
				SCB_BSSCFG(scb)->tdls->chsw_req_enabled = FALSE;
				/* Channel Switch back to base channel */
				err = wlc_tdls_return_to_base_ch(wlc->tdls, scb);
				if (case_local_scb_tdls->target_chanspec !=
					case_local_scb_tdls->base_chanspec) {
					chanspec_t target_ch_s =
						case_local_scb_tdls->target_chanspec;
					if (!tdls->manual_chsw) {
						case_local_scb_tdls->target_chanspec =
							case_local_scb_tdls->base_chanspec;
					}
					WL_TDLS(("wl%d:%s(): Channel switch disabled, "
						"deleting chan_ctx for target_chanspec: 0x%04x"
						"scb_tdls->flags = 0x%08x\n",
						tdls->pub->unit, __FUNCTION__,
						case_local_scb_tdls->target_chanspec,
						case_local_scb_tdls->flags));
					/* chan_ctx will be deleted in the channel switch Resp cb */
					if (case_local_scb_tdls->cur_chanspec ==
						case_local_scb_tdls->base_chanspec)
						wlc_tdls_delete_chan_ctx(tdls, scb, target_ch_s);
					case_local_scb_tdls->flags &= ~TDLS_SCB_CHSW_MASK;
				}
			}

			break;
		}

		case TDLS_MANUAL_EP_WFD_TPQ: {

			wlc_tdls_send_tunneled_probe(tdls, parent, &info->ea, WFA_OUI_TYPE_TPQ);

			break;
		}

		default:
			err = BCME_BADARG;
			break;
	}

	return err;
}

static int wlc_tdls_wfd_ie(tdls_info_t *tdls, tdls_wfd_ie_iovar_t *info_in,
	tdls_wfd_ie_iovar_t *info_out, wlc_bsscfg_t *parent, uint8 type)
{
	wlc_info_t *wlc = tdls->wlc;
	tdls_wfd_ie_iovar_t info;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	tdls_lookaside_t *peer;
	int err;

	memcpy(&info, info_in, sizeof(info));
	memcpy(info_out, &info, sizeof(info));
	info_out->length = 0;

	if (ETHER_ISMULTI(&info.ea)) {
		WL_ERROR(("wl%d:%s(): Peer MAC cannot be a multi-cast address!\n", tdls->pub->unit,
		          __FUNCTION__));
		return BCME_BADADDR;
	}

	if (type == TDLS_WFD_PROBE_IE_RX) {

		peer = wlc_tdls_lookaside_find(tdls, &info.ea);

		if (peer && peer->wfd_ie_probe_rx_length) {
			if (peer->wfd_ie_probe_rx_length > sizeof(info_out->data))
				return BCME_BUFTOOSHORT;

			memcpy(info_out->data, peer->wfd_ie_probe_rx, peer->wfd_ie_probe_rx_length);
			info_out->length = peer->wfd_ie_probe_rx_length;
		}

		return BCME_OK;
	}

	err = wlc_tdls_setup_validate_parent(tdls, parent);
	if (err) return err;

	scb = wlc_tdls_scbfindband_all(wlc, &info.ea,
	                      CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

	/* no error returned if peer has not connected yet */

	if (scb) {
		scb_tdls = SCB_TDLS(tdls, scb);
		if (scb_tdls) {
			if ((scb_tdls->flags & TDLS_SCB_CONNECTED) && (scb_tdls->peer_ies_len)) {
				if (scb_tdls->peer_ies_len > sizeof(info_out->data))
					return BCME_BUFTOOSHORT;

				memcpy(info_out->data, scb_tdls->peer_ies, scb_tdls->peer_ies_len);
				info_out->length = scb_tdls->peer_ies_len;
			}
		}
	}

	return err;
}

/* handle TDLS related iovars */
static int
wlc_tdls_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	wlc_info_t *wlc = tdls->wlc;
	int32 int_val = 0;
	bool bool_val;
	uint32 *ret_uint_ptr;
	int32 *ret_int_ptr;
	int err = 0;
	wlc_bsscfg_t *bsscfg;
	wlc_tdls_t *tc;

	UNUSED_PARAMETER(tc);
	ASSERT(tdls == wlc->tdls);

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	tc = bsscfg->tdls;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_uint_ptr = (uint32 *)a;
	ret_int_ptr = (int32 *)a;

	switch (actionid) {

	case IOV_SVAL(IOV_TDLS_ENDPOINT):
	{
		tdls_iovar_t iov_info;

		if (!TDLS_ENAB(wlc->pub)) {
			err = BCME_ERROR;
			break;
		}

		bcopy(a, &iov_info, sizeof(tdls_iovar_t));
		err = wlc_tdls_endpoint_op(tdls, &iov_info, bsscfg);

		break;
	}

	case IOV_GVAL(IOV_TDLS_ENABLE):
		*ret_uint_ptr = TDLS_ENAB(wlc->pub);
		break;

	case IOV_SVAL(IOV_TDLS_ENABLE):
		err =  wlc_tdls_set(tdls, bool_val);
		break;

	case IOV_GVAL(IOV_TDLS_AUTO_OP):
		*ret_uint_ptr = (uint32)tdls->auto_op;
		break;

	case IOV_SVAL(IOV_TDLS_AUTO_OP):
		tdls->auto_op = (bool)int_val;
		break;

	case IOV_GVAL(IOV_TDLS_UAPSD_SLEEP):
		*ret_uint_ptr = (uint32)tdls->uapsd_sleep;
		break;

	case IOV_SVAL(IOV_TDLS_UAPSD_SLEEP):
		tdls->uapsd_sleep = (bool)int_val;
		break;

	case IOV_GVAL(IOV_TDLS_TRIGGER_PKTCNT_HIGH):
		*ret_uint_ptr = (uint32)tdls->trigger_pktcnt_high;
		break;

	case IOV_SVAL(IOV_TDLS_TRIGGER_PKTCNT_HIGH):
		tdls->trigger_pktcnt_high = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_TDLS_TRIGGER_PKTCNT_LOW):
		*ret_uint_ptr = (uint32)tdls->trigger_pktcnt_low;
		break;

	case IOV_SVAL(IOV_TDLS_TRIGGER_PKTCNT_LOW):
		tdls->trigger_pktcnt_low = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_TDLS_RSSI_HIGH):
		*ret_int_ptr = tdls->rssi_threshold_high;
		break;

	case IOV_SVAL(IOV_TDLS_RSSI_HIGH):
		tdls->rssi_threshold_high = (int16)int_val;
		break;

	case IOV_GVAL(IOV_TDLS_RSSI_LOW):
		*ret_int_ptr = tdls->rssi_threshold_low;
		break;

	case IOV_SVAL(IOV_TDLS_RSSI_LOW):
		tdls->rssi_threshold_low = (int16)int_val;
		break;
	case IOV_GVAL(IOV_TDLS_SETUP_RESP_TIMEOUT):
		*ret_uint_ptr = (uint32)tdls->setup_resp_timeout;
		break;

	case IOV_SVAL(IOV_TDLS_SETUP_RESP_TIMEOUT):
		if ((int_val <= 0) || (int_val > 10)) {
			err = BCME_RANGE;
			break;
		}
		tdls->setup_resp_timeout = (uint16)int_val;
		break;

#ifdef TDLS_TESTBED
	case IOV_SVAL(IOV_TDLS_TEST_PROHIBIT):
		tdls->test_prohibit = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_TEST_PROHIBIT):
		*ret_uint_ptr = (uint32)(tdls->test_prohibit);

		break;

	case IOV_SVAL(IOV_TDLS_TEST_MAC):
		WL_INFORM(("wl%d:%s(): set TDLS test_mac to %s\n",
			tdls->pub->unit, __FUNCTION__, bool_val ? "TRUE" : "FALSE"));
		tdls->test_mac = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_TEST_MAC):
		*ret_uint_ptr = (uint32)(tdls->test_mac);
		WL_INFORM(("wl%d:%s(): get TDLS test_mac = %d\n",
			tdls->pub->unit, __FUNCTION__, tdls->test_mac));

		break;

	case IOV_SVAL(IOV_TDLS_WRONG_BSSID):
		tdls->wrong_bssid = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_WRONG_BSSID):
		*ret_uint_ptr = (uint32)(tdls->wrong_bssid);

		break;

	case IOV_SVAL(IOV_TDLS_NO_RESP):
		tdls->test_no_resp = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_NO_RESP):
		*ret_uint_ptr = (uint32)(tdls->test_no_resp);

		break;

	case IOV_SVAL(IOV_TDLS_CHSW_TIMEOUT):
		tdls->test_chsw_timeout = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_CHSW_TIMEOUT):
		*ret_uint_ptr = (uint32)(tdls->test_chsw_timeout);

		break;

	case IOV_SVAL(IOV_TDLS_STATUS):
		tdls->test_status_code = (uint16)int_val;

		break;

	case IOV_GVAL(IOV_TDLS_STATUS):
		*ret_uint_ptr = (uint32)(tdls->test_status_code);

		break;

	case IOV_SVAL(IOV_TDLS_QUIET_DOWN):
		tdls->quiet_down = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_QUIET_DOWN):
		*ret_uint_ptr = (uint32)(tdls->quiet_down);

		break;

	case IOV_SVAL(IOV_TDLS_TEST_TKIP):
		tdls->test_tkip = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_TEST_TKIP):
		*ret_uint_ptr = (uint32)(tdls->test_tkip);

		break;

#endif /* TDLS_TESTBED */

	case IOV_SVAL(IOV_TDLS_CHSW_MODE):
		if ((int_val < TDLS_CHSW_ENABLE) || (int_val > TDLS_CHSW_SEND_REQ)) {
			err = BCME_RANGE;
			break;
		}

		tdls->chsw_mode = (uint8)int_val;
		if (tdls->chsw_mode != TDLS_CHSW_ENABLE)
			tdls->manual_chsw = TRUE;
		else
			tdls->manual_chsw = FALSE;

		break;

	case IOV_GVAL(IOV_TDLS_CHSW_MODE):
		*ret_uint_ptr = (uint32)(tdls->chsw_mode);

		break;

	case IOV_SVAL(IOV_TDLS_CERT_TEST):
		tdls->cert_test = bool_val;

		break;

	case IOV_GVAL(IOV_TDLS_CERT_TEST):
		*ret_uint_ptr = tdls->cert_test;
		break;

case IOV_SVAL(IOV_TDLS_WFD_IE):
{
	tdls_wfd_ie_iovar_t* iov_info = (tdls_wfd_ie_iovar_t*) a;

	/* the clear operation is implicit, since iov_info->length is 0 */

	if (iov_info->mode == TDLS_WFD_IE_TX) {

		if (iov_info->length > sizeof(tdls->wfd_ie_setup_tx)) {
			err = BCME_BUFTOOLONG;
			break;
		}

		memset(tdls->wfd_ie_setup_tx, 0, sizeof(tdls->wfd_ie_setup_tx));
		memcpy(tdls->wfd_ie_setup_tx, iov_info->data, iov_info->length);
		tdls->wfd_ie_setup_tx_length = (uint8)iov_info->length;
	}

	else if (iov_info->mode == TDLS_WFD_PROBE_IE_TX) {

		if (iov_info->length > sizeof(tdls->wfd_ie_probe_tx)) {
			err = BCME_BUFTOOLONG;
			break;
		}

		memset(tdls->wfd_ie_probe_tx, 0, sizeof(tdls->wfd_ie_probe_tx));
		memcpy(tdls->wfd_ie_probe_tx, iov_info->data, iov_info->length);
		tdls->wfd_ie_probe_tx_length = (uint8)iov_info->length;
	}

	else err = BCME_BADOPTION;

	break;
}
case IOV_GVAL(IOV_TDLS_WFD_IE):
{
	tdls_wfd_ie_iovar_t* iov_info = (tdls_wfd_ie_iovar_t*) a;
	tdls_wfd_ie_iovar_t* iov_info_param = (tdls_wfd_ie_iovar_t*) p;

	if (iov_info_param->mode == TDLS_WFD_IE_TX) {
	memcpy(iov_info->data, tdls->wfd_ie_setup_tx, tdls->wfd_ie_setup_tx_length);
	iov_info->length = tdls->wfd_ie_setup_tx_length;
	}
	else if (iov_info_param->mode == TDLS_WFD_PROBE_IE_TX) {
	memcpy(iov_info->data, tdls->wfd_ie_probe_tx, tdls->wfd_ie_probe_tx_length);
	iov_info->length = tdls->wfd_ie_probe_tx_length;
	}
	else if (iov_info_param->mode == TDLS_WFD_IE_RX) {
		err = wlc_tdls_wfd_ie(tdls, iov_info_param, iov_info, bsscfg, TDLS_WFD_IE_RX);
	}
	else if (iov_info_param->mode == TDLS_WFD_PROBE_IE_RX) {
		err = wlc_tdls_wfd_ie(tdls, iov_info_param, iov_info, bsscfg, TDLS_WFD_PROBE_IE_RX);
	}
	else err = BCME_BADOPTION;

	break;
}

	case IOV_GVAL(IOV_TDLS_STA_INFO): {
		struct ether_addr ea;

		bcopy(p, &ea, sizeof(struct ether_addr));

		if (TDLS_ENAB(wlc->pub)) {
			struct scb * tdls_scb;
			tdls_scb = wlc_tdls_scbfind_all(wlc, &ea);
			if (tdls_scb) {
				err = wlc_sta_info(wlc, tdls_scb->bsscfg, &ea, a, alen);
			}
			else {
				err = BCME_NOTFOUND;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_TDLS_IDLE_TIME):
		*ret_uint_ptr = (uint32)tdls->idle_time;
		break;

	case IOV_SVAL(IOV_TDLS_IDLE_TIME): {
		if ((uint16)int_val > 0)
			tdls->idle_time = (uint16)int_val;
		else
			err = BCME_BADARG;
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

bool wlc_tdls_cert_test_enabled(wlc_info_t *wlc)
{
	tdls_info_t *tdls = wlc->tdls;
	return tdls->cert_test;
}

/* This is recovery funtion to kick in channel switch logic in auto channel switch mode
 * e.g. if errors happens during channel switch handshake or may be other scenarios like
 * bg scan or may be other possible reasons, which could lead to our multi channel timer
 * logic stop kicking in
 * This funtion helps channel switch initiator to reinstate the auto channel switch logic
*/

static void
wlc_tdls_chsw_recover(wlc_info_t *wlc, struct scb *scb)
{
	scb_tdls_t *scb_tdls = SCB_TDLS(wlc->tdls, scb);
	if (wlc->tdls->manual_chsw) {
		WL_TDLS(("wl%d:%s(): Recovery not needed for manual mode\n",
			wlc->tdls->pub->unit, __FUNCTION__));
		return;
	}
	if (scb_tdls->target_chanspec != scb_tdls->base_chanspec)
		wlc_tdls_send_chsw_req(wlc->tdls, scb, scb_tdls->target_chanspec);

	return;
}

static void
wlc_tdls_peer_state_update(tdls_info_t *tdls, tdls_lookaside_t *peer)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	scb_tdls_t *scb_tdls;
	uint32 now = OSL_SYSUPTIME();
	int delta;

	UNUSED_PARAMETER(delta);
	WL_TDLS(("wl%d:%s(): enter...\n", tdls->pub->unit, __FUNCTION__));

	scb = wlc_tdls_scbfind_all(wlc, &peer->ea);
	if ((NBANDS(wlc) > 1) && (scb == NULL))
		scb = wlc_tdls_scbfindband_all(wlc, &peer->ea, OTHERBANDUNIT(wlc));
	if (!scb || !scb->bsscfg || !BSS_TDLS_ENAB(wlc, scb->bsscfg))
		return;

	bsscfg = SCB_BSSCFG(scb);
	if (!BSS_TDLS_ENAB(wlc, bsscfg))
		return;

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	scb_tdls->timer_start = FALSE;

	if (scb_tdls->flags & TDLS_SCB_SENT_TEARDOWN ||
		scb_tdls->flags & TDLS_SCB_RCVD_TEARDOWN) {
		wlc_bsscfg_t *parent = scb_tdls->parent;

		WL_TDLS(("wl%d:%s(): free SCB.\n", tdls->pub->unit, __FUNCTION__));
		_wlc_tdls_free_scb(tdls->wlc, scb);
		wlc_tdls_bsscfg_free(tdls, bsscfg, parent);
		wlc_tdls_pretbtt_update(wlc);
		return;
	}

	if ((scb_tdls->flags & TDLS_SCB_SENT_SETUP_REQ) ||
		(scb_tdls->flags & TDLS_SCB_SENT_SETUP_RESP)) {
		WL_TDLS(("wl%d:%s(): action_frame_sent_time = 0x%08x\n",
			tdls->pub->unit, __FUNCTION__, scb_tdls->action_frame_sent_time));
		WL_TDLS(("wl%d:%s(): start timer for response, now = 0x%08x!\n",
			tdls->pub->unit, __FUNCTION__, now));
			WL_TDLS(("wl%d:%s(): response timeout!\n", tdls->pub->unit,
				__FUNCTION__));

		wlc_tdls_disconnect(tdls, scb, TRUE);
		return;
		}

	if (scb_tdls->flags & TDLS_SCB_PTI_INTERVAL) {
		WL_TDLS(("wl%d:%s(): PTI interval expired...\n",
			tdls->pub->unit, __FUNCTION__));
		scb_tdls->flags &= ~TDLS_SCB_PTI_INTERVAL;
		wlc_apps_apsd_tdls_send(wlc, scb);
		return;
	}

		if (scb_tdls->flags & TDLS_SCB_SCHED_PTI) {
			WL_TDLS(("wl%d:%s(): time to schedule PTI...\n",
				tdls->pub->unit, __FUNCTION__));
			scb_tdls->flags &= ~TDLS_SCB_SCHED_PTI;
		wlc_tdls_send_pti(tdls, scb);
		return;
		}

		if (scb_tdls->flags & TDLS_SCB_WAIT_SWITCHTIME) {
			WL_TDLS(("wl%d:%s(): Channel switchTime done!\n",
				tdls->pub->unit, __FUNCTION__));
			scb_tdls->flags &= ~TDLS_SCB_WAIT_SWITCHTIME;
		wlc_tdls_wake_ctrl(wlc, tdls, scb);
			if (scb_tdls->cur_chanspec == scb_tdls->base_chanspec)
			return;
			wlc_tdls_chsw_complete(tdls, scb);

			return;
		}

		if (scb_tdls->flags & TDLS_SCB_WAIT_TX_ON_TARGET_CH) {
			scb_tdls->flags &= ~TDLS_SCB_WAIT_TX_ON_TARGET_CH;
			wlc_tdls_chsw_send_nulldata(tdls, scb, now);
			return;
		}

		if (scb_tdls->flags & TDLS_SCB_WAIT_NULLDATA_ACK) {
			delta = (int)(now - scb_tdls->action_frame_sent_time);
			scb_tdls->flags &= ~TDLS_SCB_WAIT_NULLDATA_ACK;
			WL_TDLS(("wl%d:%s(): SwitchTimout expiered, delta = %dms"
					"go back to base channel.\n",
					tdls->pub->unit, __FUNCTION__, delta));
#ifdef WLMCHAN
			wlc_tdls_mchan_change_bss_chan_context(tdls, bsscfg,
				scb_tdls->base_chanspec);
#ifdef PROP_TXSTATUS
			wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
				WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
			wlc_wlfc_mchan_interface_state_update(wlc, bsscfg,
				WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif /* PROP_TXSTATUS */
#endif /* WLMCHAN */
			scb_tdls->cur_chanspec = scb_tdls->base_chanspec;
			if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
				wlc_tdls_chsw_recover(wlc, scb);
			return;
		}

		if (scb_tdls->flags & TDLS_SCB_LISTEN_ON_TARGET_CH) {
			chanspec_t cur_chanspec_save = 0;
			WL_TDLS(("wl%d:%s(): SwitchTimout expiered, go back to base channel.\n",
				tdls->pub->unit, __FUNCTION__));
			scb_tdls->flags &= ~(TDLS_SCB_LISTEN_ON_TARGET_CH | TDLS_SCB_RECV_CHSW_REQ);
			cur_chanspec_save = scb_tdls->cur_chanspec;
#ifdef WLMCHAN
			wlc_tdls_mchan_change_bss_chan_context(tdls, bsscfg,
				scb_tdls->base_chanspec);
#ifdef PROP_TXSTATUS
			wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
				WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
			wlc_wlfc_mchan_interface_state_update(wlc, bsscfg,
				WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif /* PROP_TXSTATUS */
#endif /* WLMCHAN */
			scb_tdls->cur_chanspec = scb_tdls->base_chanspec;

			if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled) {
				wlc_tdls_chsw_recover(wlc, scb);
			} else if (cur_chanspec_save) {
				wlc_tdls_delete_chan_ctx(tdls, scb, cur_chanspec_save);
			}

			return;
		}

		if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_REQ) {
			WL_TDLS(("wl%d:%s(): Channel switch response timeout!"
				"now = 0x%08x, action_frame_sent_time = 0x%08x,"
				" switchTimeout = %d\n", tdls->pub->unit, __FUNCTION__,
				now, scb_tdls->action_frame_sent_time,
				tdls->switch_timeout/1000));
			scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_REQ;
#if defined(WLMCHAN) && defined(PROP_TXSTATUS)
			wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
				WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
			wlc_wlfc_mchan_interface_state_update(wlc, bsscfg, WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif // endif
			if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
				wlc_tdls_chsw_recover(wlc, scb);
		}
}

static void
wlc_tdls_pretbtt_update(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	FOREACH_BSS(wlc, idx, cfg) {
		wlc_tbtt_ent_init(wlc->tbtt, cfg);
	}
	wlc_mcnx_tbtt_adj_all(wlc->mcnx, 0, 0);
}

static void
wlc_tdls_peer_timer(void *arg)
{
	tdls_timer_arg_t *local_timer_arg;
	tdls_info_t *tdls = NULL;
	wlc_info_t *wlc = NULL;
	int idx;
	tdls_lookaside_t *peer;
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	UNUSED_PARAMETER(wlc);
	ASSERT(arg);

	local_timer_arg = (tdls_timer_arg_t *)arg;
	tdls = (tdls_info_t *)local_timer_arg->hdl;
	wlc = tdls->wlc;

	idx = local_timer_arg->idx;
	peer = &tdls->lookaside[idx];
	WL_TDLS(("wl%d:%s(): Enter, idx = %d, peer->flags = 0x%x peer->status = 0x%x\n",
		tdls->pub->unit, __FUNCTION__, idx, peer->flags, peer->status));

	if (peer->pmoverride_set) {
		if (!(peer->flags & TDLS_LOOKASIDE_INUSE) ||
			!(peer->status & TDLS_LOOKASIDE_DISC))
			WL_ERROR(("wl%d:%s(): peer(%d)'s pmoverride is set,"
				"but the flags 0x%x and status 0x%x seem wrong.\n",
				tdls->pub->unit, __FUNCTION__, idx, peer->flags, peer->status));
		wlc_set_pmoverride(peer->parent, FALSE);
		peer->pmoverride_set = FALSE;
	}

	if (!(peer->flags & TDLS_LOOKASIDE_INUSE))
		return;

	if (peer->status & TDLS_LOOKASIDE_DISC) {
		/* if no response within dot11TDLSDiscoveryRequestWindow, delete the entry */
		if ((peer->flags & TDLS_LOOKASIDE_TEMP) &&
			!(peer->status & TDLS_LOOKASIDE_ACTIVE)) {
			WL_TDLS(("wl%d:%s(): TDLS lookaside table entry %d: %s =>delete\n",
				tdls->pub->unit, __FUNCTION__,
				idx, bcm_ether_ntoa(&peer->ea, eabuf)));
			wlc_tdls_lookaside_delete(tdls, peer);
			}
		else {
			WL_TDLS(("wl%d:%s(): TDLS lookaside table entry %d: %s =>IDLE\n",
				tdls->pub->unit, __FUNCTION__,
				idx, bcm_ether_ntoa(&peer->ea, eabuf)));
			peer->status &= ~(TDLS_LOOKASIDE_DISC | TDLS_LOOKASIDE_WAIT_TO_DISC);
		}
		return;
	}

	wlc_tdls_peer_state_update(tdls, peer);

}

int
wlc_tdls_set(tdls_info_t *tdls, bool on)
{
	tdls->pub->_tdls = FALSE;

	if (on) {
	/* still need P2P enabled for now, because TDLS is using P2P ucode. */
		tdls->pub->_tdls = on;
	} else
		wlc_tdls_down(tdls);

	return 0;
}

bool
wlc_tdls_cap(tdls_info_t *tdls)
{
	if (N_ENAB(tdls->pub))
		return TRUE;
	else
		return FALSE;
}

struct scb *
wlc_tdls_query(tdls_info_t *tdls, wlc_bsscfg_t *parent, void *p, struct ether_addr *ea)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	scb_tdls_t *scb_tdls = NULL;
	tdls_lookaside_t *dla;

	ASSERT(TDLS_ENAB(tdls->pub));

	/* For phase 1, have to be associated in a BSS */
	if (wlc->ibss_bsscfgs > 0)
		return NULL;

	/* No mcast/bcast traffic on tdls link */
	if (ETHER_ISMULTI(ea))
		return NULL;
	if (tdls->cur_sessions) {
		scb = wlc_tdls_scbfind_all(wlc, ea);
		if ((NBANDS(wlc) > 1) && (scb == NULL || !BSSCFG_IS_TDLS(scb->bsscfg)))
			scb = wlc_tdls_scbfindband_all(wlc, ea, (int)OTHERBANDUNIT(wlc));

		if (scb && BSSCFG_IS_TDLS(scb->bsscfg)) {
			scb_tdls = SCB_TDLS(tdls, scb);
			ASSERT(scb_tdls);
			/* Make sure that both direct and ap path are not set */
			if ((parent == scb_tdls->parent) &&
				(scb_tdls->flags & TDLS_SCB_CONNECTED) &&
				!(WLPKTTAG(p)->flags & WLF_TDLS_APPATH)) {
				return scb;
			}
		}
	}

	/* return if auto discovery not enabled */
	if (!tdls->auto_op)
		return NULL;

	dla = wlc_tdls_lookaside_find(tdls, ea);

	if (!dla) {
		/* create lookaside entry for TDLS discovery */
		dla = wlc_tdls_lookaside_create(tdls, ea, parent, TDLS_LOOKASIDE_WAIT_TO_DISC);
		if (!dla)
			return NULL;
	}

	if (dla->status == TDLS_LOOKASIDE_WAIT_TO_DISC)
		dla->pkt_cnt++;

	return NULL;
}

static uint8 *
wlc_tdls_write_rsn_ie(tdls_info_t *tdls, uint8 *cp, int buflen, uint32 WPA_auth,
	uint32 wsec, rsn_parms_t *rsn, bool pmkid_cnt)
{
	/* Infrastructure WPA info element */
	uint WPA_len = 0;	/* tag length */
	bcm_tlv_t *wpa2ie = (bcm_tlv_t *)cp;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *auth;
	uint16 count;
	uint8 *cap;
	uint8 *orig_cp = cp;
	int totlen;

	WL_WSEC(("wl%d: adding RSN IE, wsec = 0x%x\n", tdls->pub->unit, wsec));

	/* perform length check */
	/* if buffer too small, return untouched buffer */
	totlen = (int)(&wpa2ie->data[WPA2_VERSION_LEN] - &wpa2ie->id) +
		WPA_SUITE_LEN + WPA_IE_SUITE_COUNT_LEN;
	BUFLEN_CHECK_AND_RETURN(totlen, buflen, orig_cp);
	buflen -= totlen;

	/* fixed portion */
	wpa2ie->id = DOT11_MNG_RSN_ID;
	wpa2ie->data[0] = (uint8)WPA2_VERSION;
	wpa2ie->data[1] = (uint8)(WPA2_VERSION>>8);
	WPA_len = WPA2_VERSION_LEN;

	/* multicast suite */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	bcopy(WPA2_OUI, mcast->oui, DOT11_OUI_LEN);
	mcast->type = WPA_CIPHER_TPK;
	WPA_len += WPA_SUITE_LEN;

	/* unicast suite list */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = 0;

	WPA_len += WPA_IE_SUITE_COUNT_LEN;

	if (WSEC_AES_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);

		bcopy(WPA2_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WPA_CIPHER_AES_CCM;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	ASSERT(count);
	ucast->count.low = (uint8)count;
	ucast->count.high = (uint8)(count>>8);

	/* authenticated key management suite list */
	auth = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = 0;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_cp);

	WPA_len += WPA_IE_SUITE_COUNT_LEN;
	buflen -= WPA_IE_SUITE_COUNT_LEN;

	BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);
	bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
	auth->list[count++].type = RSN_AKM_TPK;
	WPA_len += WPA_SUITE_LEN;
	buflen -= WPA_SUITE_LEN;

	ASSERT(count);
	auth->count.low = (uint8)count;
	auth->count.high = (uint8)(count>>8);

	/* WPA capabilities */
	cap = (uint8 *)&auth->list[count];
	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_CAP_LEN, buflen, orig_cp);
	cap[0] = rsn ? rsn->cap[0] : 0;
	cap[1] = WPA_CAP_PEER_KEY_ENABLE;
	WPA_len += WPA_CAP_LEN;
	buflen -= WPA_CAP_LEN;

	if (WPA_len)
	    cp += TLV_HDR_LEN + WPA_len;

	if (pmkid_cnt) {
		*cp++ = 0;
		*cp++ = 0;
		WPA_len += WPA_PMKID_CNT_LEN;
		buflen -= WPA_PMKID_CNT_LEN;
	}

	/* update tag length */
	wpa2ie->len = (uint8)WPA_len;

	return (cp);
}

static uint8 *
wlc_tdls_write_ft_ie(tdls_info_t *tdls, struct scb *scb, uint8 *cp, int buflen)
{
	scb_tdls_t *scb_tdls;
	dot11_ft_ie_t *ftie;

	BUFLEN_CHECK_AND_RETURN(sizeof(dot11_ft_ie_t), buflen, cp);

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);

	ftie = (dot11_ft_ie_t *)cp;
	bzero(ftie, sizeof(dot11_ft_ie_t));
	ftie->id = DOT11_MNG_FTIE_ID;
	ftie->len = sizeof(dot11_ft_ie_t) - 2;

	bcopy(scb_tdls->snonce, ftie->snonce, EAPOL_WPA_KEY_NONCE_LEN);
	bcopy(scb_tdls->anonce, ftie->anonce, EAPOL_WPA_KEY_NONCE_LEN);
	bcopy(scb_tdls->mic, ftie->mic, 16);

	cp += sizeof(dot11_ft_ie_t);

	return cp;
}

static uint8 *
wlc_tdls_write_link_id_ie(tdls_info_t *tdls, struct scb *scb, uint8 *cp, int buflen)
{
	scb_tdls_t *scb_tdls;
	link_id_ie_t *link_id_ie;

	BUFLEN_CHECK_AND_RETURN(sizeof(link_id_ie_t), buflen, cp);

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);

	link_id_ie = (link_id_ie_t *)cp;
	bzero(link_id_ie, sizeof(link_id_ie_t));
	link_id_ie->id = DOT11_MNG_LINK_IDENTIFIER_ID;
	link_id_ie->len = TDLS_LINK_ID_IE_LEN;

	bcopy((const char*)&scb_tdls->parent->BSSID, (char*)&link_id_ie->bssid,
		ETHER_ADDR_LEN);
#ifdef TDLS_TESTBED
	if (tdls->test_send && tdls->wrong_bssid) {
		bcopy((const char*)&tdls->test_BSSID, (char*)&link_id_ie->bssid, ETHER_ADDR_LEN);
		WL_TDLS(("%s(): Test sending wrong BSSID:\n", __FUNCTION__));
		TDLS_PRHEX("wrong_BSSID:", (uchar*)&link_id_ie->bssid, ETHER_ADDR_LEN);
	}
#endif // endif

	if (scb->bsscfg->tdls->initiator) {
		bcopy((const char*)&scb->bsscfg->cur_etheraddr, (char*)&link_id_ie->tdls_init_mac,
			ETHER_ADDR_LEN);
		bcopy((const char*)&scb->ea, (char*)&link_id_ie->tdls_resp_mac, ETHER_ADDR_LEN);
	}
	else {
		bcopy((const char*)&scb->bsscfg->cur_etheraddr, (char*)&link_id_ie->tdls_resp_mac,
			ETHER_ADDR_LEN);
		bcopy((const char*)&scb->ea, (char*)&link_id_ie->tdls_init_mac, ETHER_ADDR_LEN);
	}

	cp += sizeof(link_id_ie_t);

	return cp;
}

static uint8 *
wlc_tdls_write_wmm_ie(tdls_info_t *tdls, struct scb *scb, uint8 subtype, uint8 qosinfo,
	uint8 *cp, int buflen)
{
	scb_tdls_t *scb_tdls;
	wlc_bsscfg_t *bsscfg;
	uint8 *cp_save = cp;

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	bsscfg = scb_tdls->parent;

	/* add ElementID+Length */
	*cp++ = DOT11_MNG_VS_ID;

	switch (subtype) {
		case WME_SUBTYPE_IE: {
			wme_ie_t *wme_ie;

			BUFLEN_CHECK_AND_RETURN((sizeof(wme_ie_t) + 2), buflen, cp);

			*cp++ = WME_IE_LEN;
			wme_ie = (wme_ie_t *)cp;
			bcopy(WMM_OUI, wme_ie->oui, WME_OUI_LEN);
			wme_ie->type = WME_OUI_TYPE;
			wme_ie->subtype = subtype;
			wme_ie->version = WME_VER;
			wme_ie->qosinfo = qosinfo;
			cp += WME_IE_LEN;

			break;
		}

		case WME_SUBTYPE_PARAM_IE: {
			wme_param_ie_t *wme_param;

			BUFLEN_CHECK_AND_RETURN((sizeof(wme_param_ie_t) + 2), buflen, cp);

			*cp++ = WMM_PARAMETER_IE_LEN;

			wme_param = (wme_param_ie_t *)cp;
			bzero(wme_param, sizeof(wme_param_ie_t));
			bcopy(WMM_OUI, wme_param->oui, WMM_OUI_LEN);
			wme_param->type = WME_OUI_TYPE;
			wme_param->subtype = subtype;
			wme_param->version = WME_VER;
			wme_param->qosinfo = qosinfo;

			ASSERT((OFFSETOF(wme_param_ie_t, rsvd) + sizeof(edca_param_ie_t) - 1) ==
				sizeof(wme_param_ie_t));
			bcopy(&bsscfg->wme->wme_param_ie.rsvd, &wme_param->rsvd,
				(sizeof(edca_param_ie_t) - 1));

			cp += WMM_PARAMETER_IE_LEN;

			break;
		}

		default:
			break;
	}

	TDLS_PRHEX("wme ie:", cp_save, (uint)(cp - cp_save));

	return cp;

}

static uint8 *
wlc_tdls_write_pti_control_ie(tdls_info_t *tdls, struct scb *scb, uint8 *cp, int buflen)
{
	scb_tdls_t *scb_tdls;
	pti_control_ie_t *pti_control_ie;

	BUFLEN_CHECK_AND_RETURN(sizeof(pti_control_ie_t), buflen, cp);

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);

	pti_control_ie = (pti_control_ie_t *)cp;
	bzero(pti_control_ie, sizeof(pti_control_ie_t));
	pti_control_ie->id = DOT11_MNG_PTI_CONTROL_ID;
	pti_control_ie->len = TDLS_PTI_CONTROL_IE_LEN;
	pti_control_ie->tid = scb_tdls->tid;
	pti_control_ie->seq_control = scb_tdls->seq;

	cp += sizeof(pti_control_ie_t);

	return cp;
}

/* Generate IEs (use callbacks in order to fit in the IE mgmt architecture) */
/* Supported Rates IE in Setup frames */
static uint
wlc_tdls_setup_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	ASSERT(ftcbparm->tdls.sup != NULL);

	return TLV_HDR_LEN + ftcbparm->tdls.sup->count;
}

static int
wlc_tdls_setup_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	ASSERT(ftcbparm->tdls.sup != NULL);

	bcm_write_tlv_safe(DOT11_MNG_RATES_ID, ftcbparm->tdls.sup->rates,
		ftcbparm->tdls.sup->count, build->buf, build->buf_len);

	return BCME_OK;
}

/* Extended Supported Rates IE in Setup frames */
static uint
wlc_tdls_setup_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	if (ftcbparm->tdls.ext == NULL || ftcbparm->tdls.ext->count == 0)
		return 0;

	return TLV_HDR_LEN + ftcbparm->tdls.ext->count;
}

static int
wlc_tdls_setup_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	if (ftcbparm->tdls.ext == NULL || ftcbparm->tdls.ext->count == 0) {
		return BCME_OK;
	}

	bcm_write_tlv_safe(DOT11_MNG_EXT_RATES_ID, ftcbparm->tdls.ext->rates,
		ftcbparm->tdls.ext->count, build->buf, build->buf_len);

	return BCME_OK;
}

/* Ext Cap IE in Setup frames */
static uint
wlc_tdls_setup_calc_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	return TLV_HDR_LEN + DOT11_11AC_EXTCAP_LEN_TDLS;
}

static int
wlc_tdls_setup_write_ext_cap_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	bcm_write_tlv_safe(DOT11_MNG_EXT_CAP_ID, ftcbparm->tdls.cap,
		DOT11_11AC_EXTCAP_LEN_TDLS, build->buf, build->buf_len);

	return BCME_OK;
}

/* RSN IE in Setup frames */
static uint
wlc_tdls_setup_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	wlc_bsscfg_t *cfg = calc->cfg;
	scb_tdls_t *scb_tdls;
	uint8 buf[257];

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return 0;

	scb_tdls = SCB_TDLS(tdls, ftcbparm->tdls.scb);
	ASSERT(scb_tdls != NULL);

	/* TODO: find another way to calculate the IE length */

	return (uint)(wlc_tdls_write_rsn_ie(tdls, buf, sizeof(buf), WPA2_AUTH_TPK, cfg->wsec,
	                                    &scb_tdls->rsn, scb_tdls->pmkid_cnt) - buf);
}

static int
wlc_tdls_setup_write_rsn_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;
	wlc_bsscfg_t *cfg = build->cfg;
	scb_tdls_t *scb_tdls;

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return 0;

	scb_tdls = SCB_TDLS(tdls, ftcbparm->tdls.scb);
	ASSERT(scb_tdls != NULL);

	wlc_tdls_write_rsn_ie(tdls, build->buf, build->buf_len, WPA2_AUTH_TPK, cfg->wsec,
	                      &scb_tdls->rsn, scb_tdls->pmkid_cnt);

	return BCME_OK;
}

/* WME IE in Setup frames */
/* TODO: move it to wme module */
static uint
wlc_tdls_setup_calc_wme_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	uint8 buf[257];
	uint8 st;

	if (!WME_ENAB(tdls->pub))
		return 0;

	if (ftcbparm->tdls.action == TDLS_SETUP_CONFIRM)
		st = WME_SUBTYPE_PARAM_IE;
	else
		st = WME_SUBTYPE_IE;

	return (uint)(wlc_tdls_write_wmm_ie(tdls, ftcbparm->tdls.scb, st,
	                                    tdls->uapsd_sleep ? tdls->qosinfo : 0,
	                                    buf, sizeof(buf)) - buf);

	return BCME_OK;
}

static int
wlc_tdls_setup_write_wme_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;
	uint8 st;

	if (!WME_ENAB(tdls->pub))
		return BCME_OK;

	if (ftcbparm->tdls.action == TDLS_SETUP_CONFIRM)
		st = WME_SUBTYPE_PARAM_IE;
	else
		st = WME_SUBTYPE_IE;

	wlc_tdls_write_wmm_ie(tdls, ftcbparm->tdls.scb, st,
	                      tdls->uapsd_sleep ? tdls->qosinfo : 0,
	                      build->buf, build->buf_len);

	return BCME_OK;
}

/* 20/40 Coexistance IE in Setup frames */
/* TODO: move it to coex module */
static uint
wlc_tdls_setup_calc_coex_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	return TLV_HDR_LEN + DOT11_OBSS_COEXINFO_LEN;
}

static int
wlc_tdls_setup_write_coex_ie(void *ctx, wlc_iem_build_data_t *build)
{
	dot11_obss_coex_t *coex_ie = (dot11_obss_coex_t *)build->buf;

	coex_ie->id = DOT11_MNG_HT_BSS_COEXINFO_ID;
	coex_ie->len = DOT11_OBSS_COEXINFO_LEN;
	coex_ie->info = DOT11_OBSS_COEX_INFO_REQ;

	return BCME_OK;
}

/* FT IE in Setup frames */
static uint
wlc_tdls_setup_calc_fte_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_bsscfg_t *cfg = calc->cfg;

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return 0;

	return sizeof(dot11_ft_ie_t);
}

static int
wlc_tdls_setup_write_fte_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;
	wlc_bsscfg_t *cfg = build->cfg;

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return BCME_OK;

	wlc_tdls_write_ft_ie(tdls, ftcbparm->tdls.scb, build->buf, build->buf_len);
	ftcbparm->tdls.ft_ie = build->buf;

	return BCME_OK;
}

/* FT TI IE in Setup frames */
static uint
wlc_tdls_setup_calc_ft_ti_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_bsscfg_t *cfg = calc->cfg;

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return 0;

	return TLV_HDR_LEN + sizeof(ti_ie_t);
}

static int
wlc_tdls_setup_write_ft_ti_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;
	wlc_bsscfg_t *cfg = build->cfg;
	scb_tdls_t *scb_tdls;
	ti_ie_t ti_ie;

	if (!WSEC_AES_ENABLED(cfg->wsec))
		return BCME_OK;

	ti_ie.ti_type = TI_TYPE_KEY_LIFETIME;
	switch (ftcbparm->tdls.action) {
	case TDLS_SETUP_REQ:
		ti_ie.ti_val = cfg->tdls->tpk_lifetime;
		break;
	case TDLS_SETUP_RESP:
	case TDLS_SETUP_CONFIRM:
		scb_tdls = SCB_TDLS(tdls, ftcbparm->tdls.scb);
		ASSERT(scb_tdls != NULL);
		ti_ie.ti_val = scb_tdls->timeout_interval;
		break;
	default:
		ASSERT(0);
		break;
	}

	bcm_write_tlv_safe(DOT11_MNG_FT_TI_ID, &ti_ie, sizeof(ti_ie_t),
		build->buf, build->buf_len);

	return BCME_OK;
}

/* LinkID IE in Setup frames */
static uint
wlc_tdls_setup_calc_link_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	return TLV_HDR_LEN + TDLS_LINK_ID_IE_LEN;
}

static int
wlc_tdls_setup_write_link_id_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	wlc_tdls_write_link_id_ie(tdls, ftcbparm->tdls.scb, build->buf, build->buf_len);

	return BCME_OK;
}

/* AID ID in setup frames */
static uint
wlc_tdls_setup_calc_aid_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_bsscfg_t *cfg = calc->cfg;

	if (!calc->cbparm->vht)
		return 0;

	/* Make 2.4 operation check here */
	if (CHSPEC_IS2G(cfg->current_bss->chanspec))
		return 0;

	return TLV_HDR_LEN + AID_IE_LEN;
}

static int
wlc_tdls_setup_write_aid_id_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_bsscfg_t *cfg = build->cfg;

	if (!build->cbparm->vht) {
		return BCME_OK;
	}

	if (CHSPEC_IS2G(cfg->current_bss->chanspec))
		return BCME_OK;

	bcm_write_tlv_safe(DOT11_MNG_AID_ID, &cfg->AID, sizeof(cfg->AID),
		build->buf, build->buf_len);

	return BCME_OK;
}

static bool
wlc_tdls_link_add_ht_op_ie(tdls_info_t *tdls, scb_t	*scb)
{
	wlc_bsscfg_t *cfg = scb->bsscfg;
	wlc_info_t *wlc = tdls->wlc;
	chanspec_t cspec;
	scb_t *parent_scb;

	parent_scb = wlc_tdls_scbfindband_all(wlc, &cfg->BSSID,
		CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));

	if (!parent_scb) {
		WL_TDLS(("wl%d:%s(): parent SCB not found!\n",
			wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* Check if we need to add HT and VHT OP IE's */
	/* 1. STA_HT_ENAB, PEER_HT_CAP and BSS not HT
	 * 2. STA_HT_ENAB and 80Mhz , PEER_HT_ENAB and 80Mhz BW
	 * and BSS < 80Mhz
	 * 3. add HT OP IE when VHT OP IE is required
	 */
	if (BSS_N_ENAB(wlc, cfg) && SCB_HT_CAP(scb)) {
		if (TDLS_WB_CAP(wlc->tdls) && TDLS_SCB_WB_CAP(SCB_TDLS(tdls, scb)) &&
			CHSPEC_IS20(cfg->current_bss->chanspec)) {
			/* Convert the chanspec */
			cspec = wf_chspec_ctlchan(cfg->current_bss->chanspec);
			cspec = wf_channel2chspec(cspec, WL_CHANSPEC_BW_40);

			if (cspec) {
				wlc_bsscfg_set_current_bss_chan(cfg, cspec);
				return TRUE;
			}
		}
		if (!SCB_HT_CAP(parent_scb)) {
			return TRUE;
		}
	}

	return FALSE;
}

static bool
wlc_tdls_link_add_vht_op_ie(tdls_info_t *tdls, scb_t	*scb)
{
	wlc_bsscfg_t *cfg = scb->bsscfg;
	wlc_info_t *wlc = tdls->wlc;
	scb_t *parent_scb;
	chanspec_t cspec;

	parent_scb = wlc_tdls_scbfindband_all(wlc, &cfg->BSSID,
		CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));

	if (!parent_scb) {
		WL_TDLS(("wl%d:%s(): parent SCB not found!\n",
			wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* 1. STA_VHT_ENAB, PEER_VTH_ENAB and BSS not VHT
	 * 2. STA_VHT_ENAB and 80Mhz, PEER_VHT_ENAB and 80Mhz BW
	 * and BSS < 80Mhz
	 */
	if (BSS_VHT_ENAB(wlc, cfg) && SCB_VHT_CAP(scb)) {
		if (TDLS_WB_CAP(wlc->tdls) &&
			TDLS_SCB_WB_CAP(SCB_TDLS(tdls, scb)) &&
			CHSPEC_IS40(cfg->current_bss->chanspec) &&
			DOT11_OPER_MODE_CHANNEL_WIDTH_80MHZ(cfg->oper_mode)) {
			cspec = wf_chspec_ctlchan(cfg->current_bss->chanspec);
			cspec = wf_channel2chspec(cspec, WL_CHANSPEC_BW_80);
			if (cspec) {
				wlc_bsscfg_set_current_bss_chan(cfg, cspec);
				return TRUE;
			}
		}
		if (!SCB_VHT_CAP(parent_scb)) {
			return TRUE;
		}
	}

	return FALSE;
}

static void
wlc_tdls_update_ext_cap_len(uint8 *ext_cap, uint8 *ext_cap_len)
{
	int i;

	for (i = DOT11_EXTCAP_LEN_MAX - 1; i >= 0; i--) {
		if (ext_cap[i] != 0)
			break;
	}

	*ext_cap_len = i + 1;

	if (isset(ext_cap, DOT11_EXT_CAP_SPSMP)) {
		if (*ext_cap_len < DOT11_EXTCAP_LEN_SI)
			*ext_cap_len = DOT11_EXTCAP_LEN_SI;
	}
}

static void
wlc_tdls_build_ext_cap_ie(tdls_info_t *tdls, wlc_bsscfg_t *cfg, uint8 *extcap, uint8 *len)
{
	if (cfg == NULL)
		return;
	/* if cfg is primary cfg */
	if (tdls->wlc->cfg == cfg) {
		if (!len)
			return;
		if (!extcap) {
			*len = 0;
			return;
		}
		if (tdls->ext_cap_flags & TDLS_CAP_TDLS)
			setbit(extcap, DOT11_TDLS_CAP_TDLS);
		if (tdls->ext_cap_flags & TDLS_CAP_PU_BUFFER_STA)
			setbit(extcap, DOT11_TDLS_CAP_PU_BUFFER_STA);
		if (tdls->ext_cap_flags & TDLS_CAP_PEER_PSM)
			setbit(extcap, DOT11_TDLS_CAP_PEER_PSM);
		if (tdls->ext_cap_flags & TDLS_CAP_CH_SW)
			setbit(extcap, DOT11_TDLS_CAP_CH_SW);
		if (tdls->ext_cap_flags & TDLS_CAP_PROH)
			setbit(extcap, DOT11_TDLS_CAP_PROH);
		if (tdls->ext_cap_flags & TDLS_CAP_CH_SW_PROH)
			setbit(extcap, DOT11_TDLS_CAP_CH_SW_PROH);
		wlc_tdls_update_ext_cap_len(extcap, len);
		if (BSS_VHT_ENAB(tdls->wlc, cfg)) {
			if (tdls->ext_cap_flags & TDLS_CAP_TDLS_WIDER_BW)
				setbit(extcap, DOT11_TDLS_CAP_TDLS_WIDER_BW);
			*len = DOT11_11AC_EXTCAP_LEN_TDLS;
			return;
		}
		return;
	}
	memset(&cfg->ext_cap, 0, DOT11_EXTCAP_LEN_MAX);

	if (tdls->ext_cap_flags & TDLS_CAP_TDLS)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_TDLS, TRUE);
	if (tdls->ext_cap_flags & TDLS_CAP_PU_BUFFER_STA)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_PU_BUFFER_STA, TRUE);
	if (tdls->ext_cap_flags & TDLS_CAP_PEER_PSM)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_PEER_PSM, TRUE);
	if (tdls->ext_cap_flags & TDLS_CAP_CH_SW)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_CH_SW, TRUE);
	if (tdls->ext_cap_flags & TDLS_CAP_PROH)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_PROH, TRUE);
	if (tdls->ext_cap_flags & TDLS_CAP_CH_SW_PROH)
		wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_CH_SW_PROH, TRUE);

	if (BSS_VHT_ENAB(tdls->wlc, cfg)) {
		if (tdls->ext_cap_flags & TDLS_CAP_TDLS_WIDER_BW) {
			wlc_bsscfg_set_ext_cap(cfg, DOT11_TDLS_CAP_TDLS_WIDER_BW, TRUE);
		}
	/* OMN is not dependent on wider BW, may be needed for lowering BW(tdls peer->bw_cap) */
	wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_OPER_MODE_NOTIF, TRUE);
	}
}

static uint8
wlc_tdls_create_oper_mode(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *cfg)
{
	uint8 width;
	wlcband_t *band;
	uint8 nss = MIN(wlc->stf->op_rxstreams, VHT_CAP_MCS_MAP_NSS_MAX);

	if (CHSPEC_IS2G(cfg->current_bss->chanspec)) {
		band = wlc->bandstate[BAND_2G_INDEX];
	}
	else {
		band = wlc->bandstate[BAND_5G_INDEX];
	}

	if (BSS_VHT_ENAB(wlc, cfg) && WL_BW_CAP_80MHZ(band->bw_cap) &&
		!(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_80MHZ)) {
		width = DOT11_OPER_MODE_80MHZ;
	} else if (BSS_N_ENAB(wlc, scb->bsscfg) && WL_BW_CAP_40MHZ(band->bw_cap) &&
		!(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ)) {
		width = DOT11_OPER_MODE_40MHZ;
	} else {
		width = DOT11_OPER_MODE_20MHZ;
	}

	return DOT11_D8_OPER_MODE(0, nss, 0, 0, width);
}

static bool
wlc_tdls_write_action_frame(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct scb *scb,
	void *p, uint8 action_field, uint16 status_code, uint8 token, void *req)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls;
	uchar *data_save, *data_pre;
	wlc_rateset_t sup_rates, ext_rates;
	uchar *bufend;
	wlc_bsscfg_t *bsscfg;
	uchar *tlvs;
	uchar *ft_ie = NULL;
	uint16 cap;
	uint8 *data;
	uint data_len;
	uint8 transId;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	wlc_ier_reg_t *ier = NULL;
	wlc_tdls_t *tc;
	bool ht = FALSE;

	ASSERT(parent);
	ASSERT(p);
	if (action_field != TDLS_SETUP_RESP || !status_code) {
		/* if status_code for TDLS_SETUP_RESP failed, scb can be NULL */
		ASSERT(scb);
		if (!scb)
			return FALSE;
	}

	WL_TDLS(("wl%d: %s(): action_filed = %d\n",
		tdls->pub->unit, __FUNCTION__, action_field));

	data = data_save = PKTDATA(tdls->osh, p);
	/* save the end of buffer, used for buffer length checks */
	bufend = data + PKTLEN(tdls->osh, p);

	/* 1: TDLS category */
	*data++ = TDLS_ACTION_CATEGORY_CODE;

	/* 2: TDLS action field */
	*data++ = action_field;

	switch (action_field) {
	case TDLS_SETUP_RESP:
		/* 3: status code */
		WL_TDLS(("status_code:  %d\n", status_code));
		htol16_ua_store(status_code, data);
		data += sizeof(uint16);
		/* if status_code is not 0, skip rest of the IEs */
		if (status_code) {
			*data++ = token;
			WL_TDLS(("token:  %d\n", token));
			break;
		}
		/* FALLTHROUGH */

	case TDLS_SETUP_REQ:
		ASSERT(scb);
		bsscfg = SCB_BSSCFG(scb);
		scb_tdls = SCB_TDLS(tdls, scb);

		/* 4: Dialog Token */
		*data++ = token;
		WL_TDLS(("token:  %d\n", token));

		/* 5: capability */
		cap = wlc_assoc_capability(wlc, parent, parent->current_bss);
		WL_TDLS(("token:  0x%04x\n", cap));
		store16_ua(data, cap);
		data += sizeof(uint16);

		/* supported rates */
		bzero(&sup_rates, sizeof(wlc_rateset_t));
		bzero(&ext_rates, sizeof(wlc_rateset_t));
		switch (action_field) {
		case TDLS_SETUP_REQ:
			wlc_rateset_elts(wlc, bsscfg, &bsscfg->target_bss->rateset,
			                 &sup_rates, &ext_rates);
			break;
		case TDLS_SETUP_RESP:
			wlc_rateset_elts(wlc, bsscfg, &scb->rateset, &sup_rates, &ext_rates);
			break;
		}

		goto build;

	case TDLS_SETUP_CONFIRM:
		ASSERT(scb);
		bsscfg = SCB_BSSCFG(scb);
		scb_tdls = SCB_TDLS(tdls, scb);

		/* 3: status code */
		htol16_ua_store(status_code, data);
		data += sizeof(uint16);
		if (status_code) {
			*data++ = token;
			WL_TDLS(("wl%d: %s(): TDLS_SETUP_CONFIRM failed. status_code = %d,"
			         "token = %d.\n",
			         tdls->pub->unit, __FUNCTION__, status_code, token));
			data = wlc_tdls_write_link_id_ie(tdls, scb, data,
			                                 BUFLEN(data, bufend));
			break;
		}

		/* 4: Dialog Token */
		*data++ = token;

		if (WSEC_AES_ENABLED(bsscfg->wsec))
			memset(scb_tdls->mic, 0, 16);

	build:	/* Build IEs */

		/* prepare the IE mgmt calls */
		bzero(&ftcbparm, sizeof(ftcbparm));
		ftcbparm.tdls.scb = scb;
		ftcbparm.tdls.sup = &sup_rates;
		ftcbparm.tdls.ext = &ext_rates;
		ftcbparm.tdls.cap = bsscfg->ext_cap;
		ftcbparm.tdls.chspec = parent->current_bss->chanspec;
		ftcbparm.tdls.action = action_field;
		ftcbparm.tdls.ht_op_ie = FALSE;
		ftcbparm.tdls.vht_op_ie = FALSE;
		bzero(&cbparm, sizeof(cbparm));
		cbparm.ft = &ftcbparm;
		switch (action_field) {
		case TDLS_SETUP_REQ:
			ht = BSS_N_ENAB(wlc, bsscfg);
			break;
		case TDLS_SETUP_RESP:
		case TDLS_SETUP_CONFIRM:
			ht = BSS_N_ENAB(wlc, bsscfg) && SCB_HT_CAP(scb);
			break;
		}
		cbparm.ht = ht;
		cbparm.vht = VHT_ENAB(wlc->pub);

		switch (action_field) {
		case TDLS_SETUP_REQ:
		case TDLS_SETUP_RESP:
			tc = bsscfg->tdls;
			tc->rclen = wlc_get_regclass_list(wlc->cmi, tc->rclist, MAXRCLISTSIZE,
				parent->current_bss->chanspec, TRUE);
			break;
		}
		/* which registry? */
		switch (action_field) {
		case TDLS_SETUP_REQ:
			ier = wlc->ier_tdls_srq;
			break;
		case TDLS_SETUP_RESP:
			ier = wlc->ier_tdls_srs;
			break;
		case TDLS_SETUP_CONFIRM:
			if (wlc_tdls_link_add_ht_op_ie(tdls, scb)) {
				ftcbparm.tdls.ht_op_ie = TRUE;
			}

			if (wlc_tdls_link_add_vht_op_ie(tdls, scb)) {
				ftcbparm.tdls.ht_op_ie = TRUE;
				ftcbparm.tdls.vht_op_ie = TRUE;
			}

			ier = wlc->ier_tdls_scf;
			break;
		}

		/* calc IEs' length */
		data_len = wlc_ier_calc_len(ier, bsscfg, FC_ACTION, &cbparm);

		/* build IEs */
		if (wlc_ier_build_frame(ier, bsscfg, FC_ACTION, &cbparm,
		                        data, data_len) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
			          wlc->pub->unit, __FUNCTION__));
			return FALSE;
		}

		/* MIC */
		switch (action_field) {
		case TDLS_SETUP_RESP:
			transId = 2;
			goto mic;
		case TDLS_SETUP_CONFIRM:
			transId = 3;
		mic:
			if (WSEC_AES_ENABLED(bsscfg->wsec) == 0)
				break;
			ft_ie = ftcbparm.tdls.ft_ie;
			/* calculate mic and update FTIE */
			WL_TDLS(("wl%d: %s(): MIC:\n", tdls->pub->unit, __FUNCTION__));
			wlc_tdls_cal_mic_chk(tdls, scb, data, data_len, transId);
			WL_TDLS(("wl%d: %s(): update FT IE\n", tdls->pub->unit, __FUNCTION__));
			wlc_tdls_write_ft_ie(tdls, scb, ft_ie, BUFLEN(ft_ie, bufend));
			break;
		}

		data += data_len;

		switch (action_field) {
		case TDLS_SETUP_REQ:
		case TDLS_SETUP_RESP:
			if (tdls->wfd_ie_setup_tx_length == 0)
				break;

			BUFLEN_CHECK_AND_RETURN(tdls->wfd_ie_setup_tx_length,
			                        (uint)(bufend - data), FALSE);

			memcpy(data, tdls->wfd_ie_setup_tx, tdls->wfd_ie_setup_tx_length);
			data += tdls->wfd_ie_setup_tx_length;
			break;
		}

		break;

		case TDLS_TEARDOWN:
			ASSERT(scb);
			scb_tdls = SCB_TDLS(tdls, scb);

			/* 3: reason code */
			htol16_ua_store(scb_tdls->reason, data);
			data += sizeof(uint16);

			/* 4: FTIE */
			tlvs = data;
			if (WSEC_AES_ENABLED(parent->wsec) && !status_code) {
				memset(scb_tdls->mic, 0, 16);
				data = wlc_tdls_write_ft_ie(tdls, scb, data, BUFLEN(data, bufend));
			}

			/* 5: Link Identifier */
			data = wlc_tdls_write_link_id_ie(tdls, scb, data, BUFLEN(data, bufend));

			/* calculate MIC in ft_ie */
			if (WSEC_AES_ENABLED(parent->wsec) && !status_code) {
				wlc_tdls_cal_teardown_mic_chk(tdls, scb, tlvs, (int)(data - tlvs),
					scb_tdls->reason, FALSE);
			}

			break;

		case TDLS_PEER_TRAFFIC_IND: {
			uint8 ac_bitmap;

			ASSERT(scb);
			scb_tdls = SCB_TDLS(tdls, scb);

			/* 3: dialog token */
			*data++ = token;
			WL_TDLS(("token: %d\n", token));

			/* 4: link Identifier */
			data_pre = data;
			data = wlc_tdls_write_link_id_ie(tdls, scb, data,
				BUFLEN(data, bufend));
			BUFEND_AND_RETURN(data_pre, data);

			/* 5: PTI Control */
			data_pre = data;
			data = wlc_tdls_write_pti_control_ie(tdls, scb, data,
				BUFLEN(data, bufend));
			BUFEND_AND_RETURN(data_pre, data);
			TDLS_PRHEX("PTI:", data_pre, (int)(data - data_pre));

			/* 6: PU Buffer Status */
			ac_bitmap = wlc_apps_apsd_ac_buffer_status(wlc, scb);
			data_pre = data;

			data = bcm_write_tlv_safe(DOT11_MNG_PU_BUFFER_STATUS_ID, &ac_bitmap,
				TDLS_PU_BUFFER_STATUS_IE_LEN, data, BUFLEN(data, bufend));

			BUFEND_AND_RETURN(data_pre, data);
			TDLS_PRHEX("PU Buffer Status IE:", data_pre, (int)(data - data_pre));

			break;
		}

		case TDLS_PEER_TRAFFIC_RESP: {
			/* 3: dialog token */
			*data++ = token;
			WL_TDLS(("token: %d\n", token));

			/* 4: link Identifier */
			data = wlc_tdls_write_link_id_ie(tdls, scb, data,
				BUFLEN(data, bufend));

			break;
		}

		case TDLS_CHANNEL_SWITCH_REQ: {
			tdls_chsw_req_t *chsw_req = (tdls_chsw_req_t *)req;
			channel_switch_timing_ie_t *chsw_timing_ie;

			/* 3: target channel */
			*data++ = chsw_req->target_ch;

			/* 4: regulatory class */
			*data++ = chsw_req->regclass;

			/* 5: Link Idenetifier */
			data = wlc_tdls_write_link_id_ie(tdls, scb, data, BUFLEN(data, bufend));

			/* 6: Secondary Offset */
			if (chsw_req->second_ch_offset) {
				dot11_extch_ie_t *extch_ie = (dot11_extch_ie_t *)data;

				extch_ie->id = DOT11_MNG_EXT_CHANNEL_OFFSET;
				extch_ie->len = DOT11_EXTCH_IE_LEN;
				extch_ie->extch = chsw_req->second_ch_offset;

				data += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);
			}

			/* 7: Channel Switch Timing */
			chsw_timing_ie = (channel_switch_timing_ie_t *)data;
			chsw_timing_ie->id = DOT11_MNG_CHANNEL_SWITCH_TIMING_ID;
			chsw_timing_ie->len = TDLS_CHANNEL_SWITCH_TIMING_IE_LEN;
			chsw_timing_ie->switch_time = htol16(chsw_req->switch_time);
			chsw_timing_ie->switch_timeout = htol16(chsw_req->switch_timeout);
			data += (TLV_HDR_LEN + TDLS_CHANNEL_SWITCH_TIMING_IE_LEN);

			TDLS_PRHEX("Channel Switch Req:", data_save, (uint)(data - data_save));

			break;
		}

		case TDLS_CHANNEL_SWITCH_RESP: {
			/* 3: status code */
			htol16_ua_store(status_code, data);
			data += sizeof(uint16);

			/* 4: Link Ideneifier */
			data = wlc_tdls_write_link_id_ie(tdls, scb, data, BUFLEN(data, bufend));

			/* 5: Channel Switch Timing */
			if (!status_code) {
				tdls_chsw_resp_t *chsw_resp = (tdls_chsw_resp_t *)req;
				channel_switch_timing_ie_t *chsw_timing_ie;

				chsw_timing_ie = (channel_switch_timing_ie_t *)data;
				chsw_timing_ie->id = DOT11_MNG_CHANNEL_SWITCH_TIMING_ID;
				chsw_timing_ie->len = TDLS_CHANNEL_SWITCH_TIMING_IE_LEN;
				chsw_timing_ie->switch_time = htol16(chsw_resp->switch_time);
				chsw_timing_ie->switch_timeout = htol16(chsw_resp->switch_timeout);
				data += (TLV_HDR_LEN + TDLS_CHANNEL_SWITCH_TIMING_IE_LEN);
			}

			TDLS_PRHEX("Channel Switch Resp:", data_save, (uint)(data - data_save));

			break;
		}

		default:
			break;
	}

	PKTSETLEN(wlc->osh, p, (uint)(data - data_save));

	return TRUE;
}

static void
wlc_tdls_pti_tx_complete(tdls_info_t *tdls, struct scb *scb, uint32 txstatus)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		if ((scb_tdls->flags & TDLS_SCB_PTI_INTERVAL) && scb_tdls->timer_start) {
			wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
			scb_tdls->timer_start = FALSE;
		}

		if (SCB_PS(scb)) {
			scb_tdls->flags &=  ~(TDLS_SCB_SENT_PTI | TDLS_SCB_PTI_INTERVAL);
			wlc_tdls_wake_ctrl(wlc, tdls, scb);
#ifdef WLTDLS_SEND_PTI_RETRY
			/* Send the PTI again if Previous PTI Request is Not Acked */
			if (!(scb_tdls->flags & TDLS_SCB_PTI_RESP_TIMED_OUT)) {
				scb_tdls->flags |= TDLS_SCB_PTI_RESP_TIMED_OUT;
				wlc_tdls_send_pti(tdls, scb);
			}
#endif // endif
		}
		WL_TDLS(("wl%d:%s(): no ACK!\n", tdls->pub->unit, __FUNCTION__));
		return;
	}

	WL_TDLS(("wl%d:%s(): sent PTI req, start timer for response,"
		"sent_time = 0x%08x!\n",
		tdls->pub->unit, __FUNCTION__, scb_tdls->pti_sent_time));
}

static void
wlc_tdls_action_frame_tx_complete_chswreq(wlc_info_t *wlc, uint32 txstatus, void *arg)
{
	tdls_lookaside_t *dla = (tdls_lookaside_t *)arg;
	struct scb *scb;
	scb_tdls_t *scb_tdls;

	if (!arg)
		return;

	scb = wlc_tdls_scbfind_all(wlc, &dla->ea);
	if (!scb || !scb->bsscfg || !BSS_TDLS_ENAB(wlc, scb->bsscfg))
		return;

	scb_tdls = SCB_TDLS(wlc->tdls, scb);
	if (!scb_tdls)
		return;

	WL_TDLS(("wl%d:%s():flags = 0x%08x!\n", wlc->pub->unit, __FUNCTION__, scb_tdls->flags));

	if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_REQ) {
		if (!(txstatus & TX_STATUS_ACK_RCV)) {
			WL_TDLS(("wl%d:%s(): no ACK!\n", wlc->pub->unit, __FUNCTION__));
			scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_REQ;
			if (wlc->tdls->manual_chsw)
				scb_tdls->target_chanspec = scb_tdls->base_chanspec;
#if defined(WLMCHAN) && defined(PROP_TXSTATUS)
			wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
				WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
			wlc_wlfc_mchan_interface_state_update(wlc, scb->bsscfg,
				WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif // endif
			if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
				wlc_tdls_chsw_recover(wlc, scb);
			return;
		}
		if (!scb_tdls->timer_start) {
			/* scb_tdls->action_frame_sent_time = OSL_SYSUPTIME(); */
			wl_add_timer(wlc->wl, scb_tdls->peer_addr->timer,
				TDLS_RESP_TIMEOUT_DEFAULT * 1000, 0);
			scb_tdls->timer_start = TRUE;
		}
		WL_TDLS(("wl%d:%s():Channel switch req ACKed, start timer for response,"
		         "sent_time = 0x%08x!\n",
		         wlc->pub->unit, __FUNCTION__, scb_tdls->action_frame_sent_time));
		return;
	}

}

static void
wlc_tdls_action_frame_tx_complete_chswresp(wlc_info_t *wlc, uint32 txstatus, void *arg)
{
	tdls_lookaside_t *dla = (tdls_lookaside_t *)arg;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	chanspec_t cur_chanspec_save = 0;

	if (!arg)
		return;

	scb = wlc_tdls_scbfind_all(wlc, &dla->ea);
	if (!scb || !scb->bsscfg || !BSS_TDLS_ENAB(wlc, scb->bsscfg))
		return;

	scb_tdls = SCB_TDLS(wlc->tdls, scb);
	if (!scb_tdls)
		return;

	WL_TDLS(("wl%d:%s():flags = 0x%08x!\n", wlc->pub->unit, __FUNCTION__, scb_tdls->flags));

	if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_RESP) {
		/* no ack */
		if (!(txstatus & TX_STATUS_ACK_RCV)) {
			WL_TDLS(("wl%d:%s(): no ACK!\n", wlc->pub->unit, __FUNCTION__));
			scb_tdls->flags &= ~(TDLS_SCB_SENT_CHSW_RESP | TDLS_SCB_RECV_CHSW_REQ);
			if (scb_tdls->base_chanspec != scb_tdls->cur_chanspec) {
				cur_chanspec_save = scb_tdls->cur_chanspec;
#ifdef WLMCHAN
				wlc_tdls_mchan_change_bss_chan_context(wlc->tdls, scb->bsscfg,
					scb_tdls->base_chanspec);
#endif // endif
				scb_tdls->cur_chanspec = scb_tdls->base_chanspec;
			}
#if defined(WLMCHAN) && defined(PROP_TXSTATUS)
			wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
				WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
			wlc_wlfc_mchan_interface_state_update(wlc, scb->bsscfg,
				WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif // endif
			if (cur_chanspec_save && !SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
				wlc_tdls_delete_chan_ctx(wlc->tdls, scb, cur_chanspec_save);
			return;
		}
		WL_TDLS(("wl%d:%s(): Channel Switch Response ACKed, start switchTime...!\n",
		         wlc->pub->unit, __FUNCTION__));
		scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_RESP;
		if (scb_tdls->flags & TDLS_SCB_RECV_CHSW_REQ)
			wlc_tdls_switch_to_target_ch(wlc->tdls, scb, scb_tdls->target_chanspec);
		else
			wlc_tdls_switch_to_target_ch(wlc->tdls, scb, scb_tdls->base_chanspec);

		return;
	}
}

static void
wlc_tdls_action_frame_tx_complete(wlc_info_t *wlc, uint32 txstatus, void *arg)
{
	tdls_lookaside_t *dla = (tdls_lookaside_t *)arg;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	wlc_txq_info_t *qi;

	if (!arg)
		return;

	scb = wlc_tdls_scbfind_all(wlc, &dla->ea);
	if (!scb || !scb->bsscfg || !BSS_TDLS_ENAB(wlc, scb->bsscfg))
		return;

	scb_tdls = SCB_TDLS(wlc->tdls, scb);
	if (!scb_tdls)
		return;

	WL_TDLS(("wl%d:%s():flags = 0x%08x!\n", wlc->pub->unit, __FUNCTION__, scb_tdls->flags));

	if (scb_tdls->flags & TDLS_SCB_SENT_PTI) {
		wlc_tdls_pti_tx_complete(wlc->tdls, scb, txstatus);
		return;
	}

	/* no ack */
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		WL_TDLS(("wl%d:%s(): no ACK!\n", wlc->pub->unit, __FUNCTION__));
		if (scb_tdls->flags & TDLS_SCB_SENT_TEARDOWN) {
			wlc_tdls_send_teardown(wlc->tdls, scb, FALSE);
			scb_tdls->free_me = FALSE;
			wlc_tdls_free_scb(wlc->tdls, scb);
			return;
		}
		if (scb_tdls->flags & (TDLS_SCB_SENT_SETUP_REQ | TDLS_SCB_SENT_SETUP_RESP)) {
			/* If Setup REQ/RESP is not acked, disconnect immediately (no need to start
			 * response timer and wait for timeout to trigger disconnect)
			 */
			wlc_tdls_disconnect(wlc->tdls, scb, TRUE);
			return;
		}
	}

	if (scb_tdls->flags & TDLS_SCB_SENT_TEARDOWN) {
		WL_TDLS(("wl%d:%s(): TDLS Teardown Acked. Free scb...\n",
			wlc->pub->unit, __FUNCTION__));
		scb_tdls->flags &= ~TDLS_SCB_CONNECTED;
		for (qi = wlc->tx_queues; qi != NULL; qi = qi->next)
			wlc_pktq_scb_free(wlc, WLC_GET_TXQ(qi), scb);
		scb_tdls->free_me = FALSE;
		wlc_tdls_free_scb(wlc->tdls, scb);
		return;
	}

	if (scb_tdls->flags == TDLS_SCB_SENT_SETUP_CFM) {
		wlc_bsscfg_t *cfg = SCB_BSSCFG(scb);

		chanspec_t cur_chanspec;
#ifdef WLMCHAN
		cur_chanspec = wlc_mchan_current_chanspec(wlc->mchan, cfg);
#else	/* WLMCHAN */
		cur_chanspec = wlc->chanspec;
#endif	/* WLMCHAN */

		if (cur_chanspec == INVCHANSPEC) {
			WL_ERROR(("wl%d: %s: invalid chanspec 0x%x, bail out!\n",
			          wlc->pub->unit, __FUNCTION__, cur_chanspec));
			return;
		}

		WL_TDLS(("wl%d:%s(): SETUP confirm ACKed. TDLS connection established!\n",
			wlc->pub->unit, __FUNCTION__));

		/* Check if TDLS link has higher BW than primary interface */
		if (CHSPEC_BW(cfg->current_bss->chanspec) > CHSPEC_BW(cur_chanspec)) {
			wlc_update_bandwidth(wlc, cfg, cfg->current_bss->chanspec);
			/* update the scb_tdls base/cur/target chanspec to the higher BW chanspec */
			scb_tdls->base_chanspec = scb_tdls->cur_chanspec =
			scb_tdls->target_chanspec = cfg->current_bss->chanspec;
		}

		scb_tdls->flags = TDLS_SCB_CONNECTED;
		wlc_tdls_port_open(wlc->tdls, &scb->ea);

		wlc_bss_mac_event(wlc, scb->bsscfg, WLC_E_TDLS_PEER_EVENT, &scb->ea,
			WLC_E_STATUS_SUCCESS, WLC_E_TDLS_PEER_CONNECTED, 0, NULL, 0);
		wlc_tdls_pretbtt_update(wlc);
		return;
	}

	if ((scb_tdls->flags == TDLS_SCB_SENT_SETUP_REQ) ||
		(scb_tdls->flags == TDLS_SCB_SENT_SETUP_RESP)) {
		scb_tdls->action_frame_sent_time = OSL_SYSUPTIME();
		wl_add_timer(wlc->wl, scb_tdls->peer_addr->timer,
			wlc->tdls->setup_resp_timeout * 1000, 0);
		scb_tdls->timer_start = TRUE;

		WL_TDLS(("wl%d:%s(): req/resp ACKed, start timer for response,"
			"sent_time = 0x%08x!\n",
			wlc->pub->unit, __FUNCTION__, scb_tdls->action_frame_sent_time));
	}
}

static void
wlc_tdls_send_discovery_req(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct ether_addr *dst)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	int len = 0;
	uchar *pdata;
	link_id_ie_t *link_id_ie;
	wlc_bss_info_t *bi = parent->current_bss;
	uint32 disc_win;
	tdls_lookaside_t *peer;

	if (!tdls->pub->up)
		return;

	WL_TDLS(("wl%d: wlc_tdls_send_discovery_req: at %d to %s\n",
		wlc->pub->unit, wlc->pub->now, bcm_ether_ntoa(dst, eabuf)));

	peer = wlc_tdls_lookaside_find(tdls, dst);
	if (peer) {

		if (peer->status & TDLS_LOOKASIDE_DISC) {
			WL_TDLS(("wl%d:%s(): is still in Discovery ...\n",
				tdls->pub->unit, __FUNCTION__));
			return;
		}
		else
			peer->status |= TDLS_LOOKASIDE_DISC;
	}
	else {
		peer = wlc_tdls_lookaside_create(tdls, dst, parent, TDLS_LOOKASIDE_DISC);
		if (peer == NULL) {
			WL_TDLS(("wl%d: No memory in TDLS Look-aside table for %s\n",
			tdls->pub->unit, bcm_ether_ntoa(dst, eabuf)));
			return;
		}
	}
	peer->flags |= TDLS_LOOKASIDE_TEMP;

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending SETUP req.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	pdata = PKTDATA(wlc->osh, p);
	/* 1: TDLS category */
	*pdata++ = TDLS_ACTION_CATEGORY_CODE;
	len++;

	/* 2: TDLS action field */
	*pdata++ = TDLS_DISCOVERY_REQ;
	len++;

	/* 3: dialog token */
	tdls->token++;
	*pdata++ = tdls->token;
	len++;
	WL_TDLS(("token: %d\n", tdls->token));

	/* 4: link Identifier */
	link_id_ie = (link_id_ie_t *)pdata;
	bzero(link_id_ie, sizeof(link_id_ie_t));
	link_id_ie->id = DOT11_MNG_LINK_IDENTIFIER_ID;
	link_id_ie->len = TDLS_LINK_ID_IE_LEN;
	bcopy((const char*)&parent->BSSID, (char*)&link_id_ie->bssid,
		ETHER_ADDR_LEN);
#ifdef TDLS_TESTBED
	if (tdls->wrong_bssid) {
		bcopy((const char*)&tdls->test_BSSID, (char*)&link_id_ie->bssid, ETHER_ADDR_LEN);
		WL_TDLS(("%s(): Test sending wrong BSSID:\n", __FUNCTION__));
		TDLS_PRHEX("wrong_BSSID:", (uchar*)&link_id_ie->bssid, ETHER_ADDR_LEN);
	}
#endif // endif

	bcopy((const char*)&parent->cur_etheraddr, (char*)&link_id_ie->tdls_init_mac,
		ETHER_ADDR_LEN);
	bcopy((const char*)dst, (char*)&link_id_ie->tdls_resp_mac, ETHER_ADDR_LEN);
	TDLS_PRHEX("Link ID IE:", pdata, sizeof(link_id_ie_t));

	len += sizeof(link_id_ie_t);
	ASSERT(len <= TDLS_ACTION_FRAME_DEFAULT_LEN);

	PKTSETLEN(wlc->osh, p, len);
	wlc_set_pmoverride(parent, TRUE);
	peer->pmoverride_set = TRUE;

	wlc_tdls_send_action_frame(tdls, parent, dst, p, FALSE);

	disc_win = tdls->disc_window * bi->dtim_period * bi->beacon_period;

	wl_add_timer(tdls->wlc->wl, peer->timer, disc_win, 0);

}

static void
wlc_tdls_send_setup_req(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct ether_addr *dst)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	int err;

	UNUSED_PARAMETER(err);
	if (!tdls->pub->up)
		return;

	WL_TDLS(("wl%d: wlc_tdls_send_setup_req: at %d to %s\n",
		tdls->pub->unit, wlc->pub->now, bcm_ether_ntoa(dst, eabuf)));

	scb = wlc_tdls_scbfindband_all(wlc, dst,
	                      CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

	if (!scb || !BSSCFG_IS_TDLS(scb->bsscfg)) {
		/* create a new TDLS scb */
		WL_ERROR(("wl%d: could not find scb\n", tdls->pub->unit));
		err = BCME_BADARG;
		return;
	}
	scb_tdls = SCB_TDLS(tdls, scb);

	wlc_tdls_build_ext_cap_ie(tdls, scb->bsscfg, NULL, NULL);
	scb->bsscfg->oper_mode = wlc_tdls_create_oper_mode(wlc, scb, scb->bsscfg);
	scb->bsscfg->oper_mode_enabled = TRUE;

	/* get a token */
	tdls->token++;
	scb_tdls->con_token = tdls->token;

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending SETUP req.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

#ifdef TDLS_TESTBED
	if (tdls->wrong_bssid)
		tdls->test_send = TRUE;
#endif // endif

	if (!wlc_tdls_write_action_frame(tdls, parent, scb, p, TDLS_SETUP_REQ, 0,
		tdls->token, NULL)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

#ifdef TDLS_TESTBED
	if (tdls->wrong_bssid)
		tdls->test_send = FALSE;
#endif // endif

	wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete,
		(void *)scb_tdls->peer_addr, p);

	scb_tdls->flags = TDLS_SCB_SENT_SETUP_REQ;
	wlc_tdls_send_action_frame(tdls, parent, dst, p, FALSE);

	/* Add the TDLS to the txpath for this SCB */
	wlc_txmod_config(wlc, scb, TXMOD_TDLS);

	WLCNTINCR(tdls->cnt->txsetupreq);
}

static int
wlc_bsscfg_tdls_init(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg)
{
	int ret;

	wlc_info_t *wlc = tdls->wlc;
	bsscfg->_ap = FALSE;

	bsscfg->BSS = 0;

	if ((ret = wlc_bsscfg_init(wlc, bsscfg)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: cannot init bsscfg\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub))
		bsscfg->wlcif->if_flags |= WLC_IF_PKT_80211;
#endif // endif

	bsscfg->pm->PM = 0;

	bsscfg->up = TRUE;
exit:
	return ret;
}

static wlc_bsscfg_t *
wlc_tdls_bsscfg_create(wlc_info_t *wlc, wlc_bsscfg_t *parent, bool initiator)
{
	int idx;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_tdls_t *tc, *ptc;
	wlcband_t *band;
	bool cck_only;

	if ((idx = wlc_bsscfg_get_free_idx(wlc)) == -1) {
		WL_ERROR(("wl%d: cannot find free index for bsscfg\n", wlc->pub->unit));
		goto error;
	}

	if ((bsscfg = wlc_bsscfg_alloc(wlc, idx, WLC_BSSCFG_NOIF | WLC_BSSCFG_NOBCMC,
	                               &parent->cur_etheraddr, FALSE)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate bsscfg\n", wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	/* init current_bss and target_bss with parent's */
	memcpy(bsscfg->current_bss, parent->current_bss, sizeof(wlc_bss_info_t));
	memcpy(bsscfg->target_bss, parent->current_bss, sizeof(wlc_bss_info_t));

	if (wlc_bsscfg_type_init(wlc, bsscfg, BSSCFG_TYPE_TDLS) != BCME_OK) {
		WL_ERROR(("wl%d: %s: TDLS bsscfg init failed\n", wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	/* init chanspec and rateset */
	bsscfg->current_bss->chanspec = parent->current_bss->chanspec;

	if (BAND_2G(wlc->band->bandtype) && wlc->band->gmode == GMODE_LEGACY_B)
		cck_only = TRUE;
	else
		cck_only = FALSE;

	wlc_bsscfg_rateset_init(wlc, bsscfg, cck_only ? WLC_RATES_CCK : WLC_RATES_CCK_OFDM,
			WL_BW_CAP_40MHZ(wlc->band->bw_cap) ?
			CHSPEC_WLC_BW(parent->current_bss->chanspec) : 0,
			cck_only ? 0 : wlc_get_mcsallow(wlc, NULL));

	tc = bsscfg->tdls;

	tc->initiator = initiator;

	tc->tpk_lifetime = wlc->tdls->sa_lifetime;

	tc->resp_timeout = TDLS_RESP_TIMEOUT_DEFAULT;
	if (wlc->tdls->ext_cap_flags & TDLS_CAP_TDLS)
		tc->tdls_cap |= BSS_TDLS_SUPPORT;
	if (wlc->tdls->ext_cap_flags & TDLS_CAP_PU_BUFFER_STA)
		tc->tdls_cap |= BSS_TDLS_UAPSD_BUF_STA;
	if (wlc->tdls->ext_cap_flags & TDLS_CAP_CH_SW)
		tc->tdls_cap |= BSS_TDLS_CH_SW;

	if (CHSPEC_IS2G(bsscfg->current_bss->chanspec)) {
		band = wlc->bandstate[BAND_2G_INDEX];
	}
	else {
		band = wlc->bandstate[BAND_5G_INDEX];
	}

	/* Set the Wider Bandwidth bit in the extended capabilities
	 * if we can support a wider bandwidth than the current BSS
	 */
	if (((CHSPEC_IS20(bsscfg->current_bss->chanspec) &&
		WL_BW_CAP_40MHZ(band->bw_cap) &&
		!(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ)) ||
		(CHSPEC_IS40(bsscfg->current_bss->chanspec) &&
		WL_BW_CAP_80MHZ(band->bw_cap) &&
		!(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_80MHZ)) ||
		(CHSPEC_IS80(bsscfg->current_bss->chanspec) &&
		WL_BW_CAP_160MHZ(band->bw_cap) &&
		!(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_160MHZ))) &&
		((WL11H_ENAB(wlc) &&
		wlc_radar_chanspec(wlc->cmi, bsscfg->current_bss->chanspec)) == FALSE)) {
		wlc->tdls->ext_cap_flags |= TDLS_CAP_TDLS_WIDER_BW;
	} else {
		/* Clear the Wider Bandwidth bit if we cannot support it
		 * This will come handy if AP changes its channel from non-radar to radar
		 */
		wlc->tdls->ext_cap_flags &= ~TDLS_CAP_TDLS_WIDER_BW;
	}

	ptc = parent->tdls;

	if (WSEC_ENABLED(parent->wsec)) {
		bsscfg->WPA_auth = ptc->WPA_auth;
		bsscfg->wsec = ptc->wsec;
		bsscfg->wsec_restrict = TRUE;
		/* restrict access until authorized */
		if (bsscfg->WPA_auth)
			bsscfg->eap_restrict = TRUE;
	} else {
		bsscfg->wsec = 0;
	}

	return bsscfg;

error:
	if (bsscfg != NULL) {
		wlc_bsscfg_free(wlc, bsscfg);
	}

	return NULL;
}

static void
wlc_tdls_send_setup_resp(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct ether_addr *dst,
	uint16 status_code, uint8 token, link_id_ie_t *link_id)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	struct scb *scb;
	scb_tdls_t *scb_tdls = NULL;

	if (!tdls->pub->up)
		return;

	ASSERT(parent != NULL);

	WL_TDLS(("wl%d: wlc_tdls_send_setup_resp: at %d to %s\n",
		tdls->pub->unit, wlc->pub->now, bcm_ether_ntoa(dst, eabuf)));

	scb = wlc_tdls_scbfindband_all(wlc, dst, CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
	if (scb != NULL) {
		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls != NULL);
		if (!scb_tdls)
			return;
	}

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending SETUP response.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	if (!wlc_tdls_write_action_frame(tdls, parent, scb, p, TDLS_SETUP_RESP, status_code,
		token, link_id)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	if (scb) {
		wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete,
			(void *)scb_tdls->peer_addr, p);
		scb_tdls->flags = TDLS_SCB_SENT_SETUP_RESP;
	}

	wlc_tdls_send_action_frame(tdls, parent, dst, p, FALSE);

	if (!scb)
		return;

	WLCNTINCR(tdls->cnt->txsetupresp);

}

static void
wlc_tdls_send_setup_cfm(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct ether_addr *dst,
	uint16 status_code, uint8 token)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	struct scb *scb;
	scb_tdls_t *scb_tdls = NULL;

	if (!tdls->pub->up)
		return;

	ASSERT(parent != NULL);

	scb = wlc_tdls_scbfindband_all(wlc, dst, CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
	if (scb != NULL) {
		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls != NULL);
	}
	else {
		WL_ERROR(("wl%d:%s(): cannot find peer scb!\n", tdls->pub->unit, __FUNCTION__));
		return;
	}

	WL_TDLS(("wl%d: wlc_tdls_send_setup_cfm: at %d to %s\n",
		tdls->pub->unit, wlc->pub->now, bcm_ether_ntoa(&scb->ea, eabuf)));

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending SETUP confirm.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	if (!wlc_tdls_write_action_frame(tdls, parent, scb, p, TDLS_SETUP_CONFIRM,
		status_code, token, NULL)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	if (!status_code) {
		wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete,
			(void *)scb_tdls->peer_addr, p);
	}

	scb_tdls->flags = TDLS_SCB_SENT_SETUP_CFM;
	wlc_tdls_send_action_frame(tdls, parent, dst, p, FALSE);

	WLCNTINCR(tdls->cnt->txsetupcfm);

}

static void
wlc_tdls_send_teardown(tdls_info_t *tdls, struct scb *scb, bool direct)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	scb_tdls_t *scb_tdls = NULL;

	ASSERT(scb);

	if (!tdls->pub->up || (SCB_PS(scb) && direct))
		return;

	if (scb != NULL) {
		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls != NULL);
	}
	else {
		WL_ERROR(("wl%d:%s(): cannot find peer scb!\n", tdls->pub->unit, __FUNCTION__));
		return;
	}

	WL_TDLS(("wl%d: wlc_tdls_send_teardown: at %d to %s\n",
		tdls->pub->unit, wlc->pub->now, bcm_ether_ntoa(&scb->ea, eabuf)));

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending teardown\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	scb_tdls->reason = direct ? DOT11_RC_TDLS_DOWN_UNSPECIFIED : DOT11_RC_TDLS_PEER_UNREACH;

	if (!wlc_tdls_write_action_frame(tdls, scb_tdls->parent, scb, p,
		TDLS_TEARDOWN, 0, 0, NULL)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	if (direct && tdls->up) {
		wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete,
			(void *)scb_tdls->peer_addr,  p);
	}

	scb_tdls->flags |= TDLS_SCB_SENT_TEARDOWN;

	wlc_tdls_send_action_frame(tdls, scb_tdls->parent, &scb->ea, p, direct);

	/* Add the TDLS to the txpath for this SCB */
	wlc_txmod_config(tdls->wlc, scb, TXMOD_TDLS);

	WLCNTINCR(tdls->cnt->txteardown);

}

void
wlc_tdls_send_pti(tdls_info_t *tdls, struct scb *scb)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	scb_tdls_t *scb_tdls = NULL;

	if (!tdls->pub->up || !tdls->up)
		return;

	ASSERT(scb != NULL);

	if (!scb)
		return;

	if (!wlc_tdls_need_pti(tdls, scb)) {
		return;
	}

	scb_tdls = SCB_TDLS(tdls, scb);

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending PTI.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	/* get a token */
	tdls->token++;
	scb_tdls->pti_token = tdls->token;
	if (!wlc_tdls_write_action_frame(tdls, scb_tdls->parent, scb, p,
		TDLS_PEER_TRAFFIC_IND, 0, tdls->token, NULL)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete,
		(void *)scb_tdls->peer_addr, p);

	scb_tdls->pti_sent_time = OSL_SYSUPTIME();
	scb_tdls->flags |= TDLS_SCB_SENT_PTI;
	wlc_tdls_wake_ctrl(wlc, tdls, scb);
	wlc_tdls_send_action_frame(tdls, scb_tdls->parent, &scb->ea, p, FALSE);

	WL_TDLS(("wl%d: %s: pti_token= %d, at %d to %s\n",	tdls->pub->unit, __FUNCTION__,
		scb_tdls->pti_token, tdls->pub->now, bcm_ether_ntoa(&scb->ea, eabuf)));

	WLCNTINCR(tdls->cnt->txptireq);

}

static void
wlc_tdls_send_pti_resp(tdls_info_t *tdls, struct scb *scb)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	scb_tdls_t *scb_tdls = NULL;

	if (!tdls->pub->up)
		return;

	ASSERT(scb != NULL);

	scb_tdls = SCB_TDLS(tdls, scb);

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending PTI.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	if (!wlc_tdls_write_action_frame(tdls, scb_tdls->parent, scb, p,
		TDLS_PEER_TRAFFIC_RESP, 0, scb_tdls->pti_token_rx, NULL)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	/* don't enq  PTI resp to psq, if peer is in PS mode */
	if (SCB_PS(scb))
		WLPKTTAG(p)->flags |= WLF_PSDONTQ;

	wlc_tdls_send_action_frame(tdls, scb_tdls->parent, &scb->ea, p, TRUE);

	WL_TDLS(("wl%d:%s(): pti_token_rx = %d, at %d to %s\n", tdls->pub->unit, __FUNCTION__,
		scb_tdls->pti_token_rx, tdls->pub->now, bcm_ether_ntoa(&scb->ea, eabuf)));

	WLCNTINCR(tdls->cnt->txptiresp);

}

static int wlc_tdls_send_chsw_req(tdls_info_t *tdls, struct scb *scb, chanspec_t target_chanspec)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls;
	void *p;
	uint8 target_ch;
	uint8 second_ch_offset = 0;
	tdls_chsw_req_t req;

	ASSERT(scb);
	if (!scb)
		return BCME_ERROR;

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls != NULL);

	if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_REQ) {
		WL_TDLS(("wl%d:%s(): out-standing Channel Switch Request exited!\n",
			tdls->pub->unit, __FUNCTION__));
		return BCME_NOTREADY;
	}

	if (!(scb_tdls->flags & TDLS_SCB_CONNECTED)) {
		WL_TDLS(("wl%d:%s(): Direct link is not connected!\n",
			tdls->pub->unit, __FUNCTION__));
		return BCME_NOTASSOCIATED;
	}

	if (wlc_tdls_chsw_validate_pmstate(tdls, scb, scb_tdls->parent))
		return BCME_NOTREADY;

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending Channel Switch Req.\n",
			tdls->pub->unit, __FUNCTION__));
		return BCME_NOMEM;
	}

#ifdef TDLS_TESTBED
	if (tdls->chsw_mode != TDLS_CHSW_SEND_REQ)
		scb_tdls->target_chanspec = target_chanspec;
#else
	scb_tdls->target_chanspec = target_chanspec;
#endif // endif
	target_ch = wf_chspec_ctlchan(target_chanspec);
	/* target_chanspec is for the primary channel, the secondary
	 * channel offset is the opposit of the target_chanspec's SB mask
	 */
	if (CHSPEC_IS40(target_chanspec)) {
		if (CHSPEC_SB_LOWER(target_chanspec))
			second_ch_offset = DOT11_EXT_CH_UPPER;
		else if (CHSPEC_SB_UPPER(target_chanspec))
			second_ch_offset = DOT11_EXT_CH_LOWER;
	}

	req.target_ch = target_ch;
	req.regclass = wlc_get_regclass(tdls->wlc->cmi, target_chanspec);
	req.second_ch_offset = second_ch_offset;
	if (scb_tdls->switch_time > tdls->switch_time)
		req.switch_time = scb_tdls->switch_time;
	else
		req.switch_time = tdls->switch_time;
	req.switch_timeout = tdls->switch_timeout;

	if (!wlc_tdls_write_action_frame(tdls, scb_tdls->parent, scb, p,
		TDLS_CHANNEL_SWITCH_REQ, 0, 0, &req)) {
		PKTFREE(tdls->osh, p, TRUE);
		return BCME_NOMEM;
	}

#ifdef TDLS_TESTBED
	if (tdls->chsw_mode == TDLS_CHSW_SEND_REQ)
		return 0;
#endif // endif

	wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete_chswreq,
		(void *)scb_tdls->peer_addr,  p);
	scb_tdls->chswreq_sent_time = OSL_SYSUPTIME();

	/* stay awake in off-channel for now */
	scb_tdls->flags |= TDLS_SCB_SENT_CHSW_REQ;
	wlc_tdls_wake_ctrl(wlc, tdls, scb);

	wlc_tdls_send_action_frame(tdls, scb_tdls->parent, &scb->ea, p, TRUE);

	WLCNTINCR(tdls->cnt->txchswreq);

	return 0;
}

static void
wlc_tdls_send_chsw_resp(tdls_info_t *tdls, struct scb *scb,
	uint16 status_code, tdls_chsw_resp_t *chsw_resp, bool cb)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	scb_tdls_t *scb_tdls = NULL;

	if (!tdls->pub->up)
		return;

	ASSERT(scb != NULL);

	scb_tdls = SCB_TDLS(tdls, scb);

	WL_TDLS(("wl%d:%s(): at %d to %s\n",
		tdls->pub->unit, __FUNCTION__, tdls->pub->now, bcm_ether_ntoa(&scb->ea, eabuf)));

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending Channel Switch Resp.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	if (!wlc_tdls_write_action_frame(tdls, scb_tdls->parent, scb, p,
		TDLS_CHANNEL_SWITCH_RESP, status_code, 0, chsw_resp)) {
		PKTFREE(tdls->osh, p, TRUE);
		return;
	}

	if (cb) {
		wlc_pcb_fn_register(wlc->pcb, wlc_tdls_action_frame_tx_complete_chswresp,
			(void *)scb_tdls->peer_addr,  p);
		scb_tdls->flags |= TDLS_SCB_SENT_CHSW_RESP;
	}

	if (scb_tdls->flags & TDLS_SCB_CHSWRSP_CANCEL) {
		WLPKTTAG(p)->flags3 |= WLF3_TXQ_SHORT_LIFETIME;
		scb_tdls->flags &= ~TDLS_SCB_CHSWRSP_CANCEL;
		WL_TDLS(("wl%d:%s(): set chswrsp short-lived pkt %p.\n",
		          tdls->pub->unit, __FUNCTION__, p));
	}

	/* stay awake in off-channel for now */
	wlc_tdls_wake_ctrl(wlc, tdls, scb);

	wlc_tdls_send_action_frame(tdls, scb_tdls->parent, &scb->ea, p, TRUE);

	WLCNTINCR(tdls->cnt->txchswresp);
}

static void
wlc_tdls_send_tunneled_probe(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *dst, uint8 type)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *p;
	uchar *data, *data_save;
	int pkt_len;
	uchar *bufend;

	if (!tdls->pub->up)
		return;

	ASSERT(parent != NULL);

	WL_TDLS(("wl%d: wlc_tdls_send_tunneled_probe: at %d to %s\n",
		tdls->pub->unit, wlc->pub->now, bcm_ether_ntoa(dst, eabuf)));

	wlc_tdls_get_pkt(wlc, TDLS_ACTION_FRAME_DEFAULT_LEN, &p);
	if (!p) {
		WL_ERROR(("wl%d:%s(): failed to get pkt for sending tunneled probe.\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	/* create the corresponding action frame */

	data = data_save = PKTDATA(tdls->osh, p);
	/* save the end of buffer, used for buffer length checks */
	bufend = data + PKTLEN(tdls->osh, p);

	/* 1: TDLS category */
	*data++ = TDLS_VENDOR_SPECIFIC;

	/* 2: TDLS action field */
	/* there is no action field: see WFA OUI Type */

	/* 3: WFA OUI */
	memcpy(data, WFA_OUI, WFA_OUI_LEN);
	data += WFA_OUI_LEN;

	/* 4: WFA OUI Type */
	*data++ = type;

	/* 5: the WFD IEs */
	if (tdls->wfd_ie_probe_tx_length) {
		if (bufend - data >= tdls->wfd_ie_probe_tx_length) {
			memcpy(data, tdls->wfd_ie_probe_tx, tdls->wfd_ie_probe_tx_length);
			data += tdls->wfd_ie_probe_tx_length;
		} else {
			WL_ERROR(("wl%d:%s(): WFD IEs too large.\n",
				tdls->pub->unit, __FUNCTION__));
			PKTFREE(tdls->osh, p, TRUE);
			return;
		}
	}

	pkt_len = (uint)(data - data_save);
	PKTSETLEN(wlc->osh, p, pkt_len);
	wlc_tdls_send_action_frame(tdls, parent, dst, p, FALSE);
}

static int
wlc_tdls_return_to_base_ch(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;
	tdls_chsw_resp_t chsw_resp;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls != NULL);

	if (scb_tdls->cur_chanspec == scb_tdls->base_chanspec) {
		WL_TDLS(("wl%d:%s(): already at base chanspec %s\n",
		tdls->pub->unit, __FUNCTION__,
		wf_chspec_ntoa_ex(scb_tdls->base_chanspec, chanbuf)));
		return BCME_OK;
	}

	if (scb_tdls->flags & TDLS_SCB_CHSW_MASK) {
		WL_TDLS(("wl%d:%s(): out-standing Channel Switch Request in progress...\n",
		tdls->pub->unit, __FUNCTION__));
		return BCME_BUSY;
	}

	chsw_resp.switch_time = scb_tdls->switch_time;
	chsw_resp.switch_timeout = scb_tdls->switch_timeout;

	if (tdls->manual_chsw || !SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
		scb_tdls->target_chanspec = scb_tdls->base_chanspec;

	wlc_tdls_send_chsw_resp(tdls, scb, 0, &chsw_resp, TRUE);

	return BCME_OK;

}

wlc_bsscfg_t *
wlc_tdls_get_parent_bsscfg(wlc_info_t *wlc, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	ASSERT(scb != NULL);

	scb_tdls = SCB_TDLS(wlc->tdls, scb);
	ASSERT(scb_tdls != NULL);

	return scb_tdls->parent;
}

static int
wlc_tdls_join(tdls_info_t *tdls, wlc_bss_info_t *bi, struct scb *scb, uint8 *parse,
	int len)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);
	wlc_bss_info_t *target_bss;
	wlc_bss_info_t *cur_bss = scb_tdls->parent->current_bss;
	bool cck_only;
	bool bss_ht, bss_vht;
	uint8 mcsallow = 0;
	uint16 bw, bw_peer, common_bw;
	chanspec_t cspec;

#if defined(BCMDBG) && defined(PROP_TXSTATUS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) && defined(PROP_TXSTATUS) */

	ASSERT(scb_tdls != NULL);
	if (!scb->bsscfg)
		return BCME_ERROR;
	target_bss = SCB_BSSCFG(scb)->target_bss;

	/* save the peer ies */
	if (scb_tdls->peer_ies)
		MFREE(tdls->osh, scb_tdls->peer_ies, scb_tdls->peer_ies_len);
	if ((scb_tdls->peer_ies = MALLOC(tdls->osh, len)) == NULL) {
		WL_ERROR(("wl%d: wlc_tdls_join: out of memory, malloced %d bytes\n",
			tdls->pub->unit, MALLOCED(tdls->osh)));
		scb_tdls->peer_ies_len = 0;
	} else {
		scb_tdls->peer_ies_len = (uint16)len;
		bcopy(parse, scb_tdls->peer_ies, len);
	}

	/* select target bss info */
	bcopy((char*)&bi->rateset, (char*)&target_bss->rateset, sizeof(wlc_rateset_t));
	target_bss->capability = scb->cap;
	target_bss->flags = bi->flags;

	/* Keep only CCK if gmode == GMODE_LEGACY_B */
	if (BAND_2G(wlc->band->bandtype) && wlc->band->gmode == GMODE_LEGACY_B)
		cck_only = TRUE;
	else
		cck_only = FALSE;

	/* apply default rateset to invalid rateset */
	if (!wlc_rate_hwrs_filter_sort_validate(&target_bss->rateset /* [in+out] */,
		&wlc->band->hw_rateset /* [in] */,
		TRUE, wlc->stf->op_txstreams)) {
		WL_RATE(("wl%d: %s: invalid rateset in target_bss. bandunit 0x%x phy_type 0x%x "
			"gmode 0x%x\n", wlc->pub->unit, __FUNCTION__, wlc->band->bandunit,
			wlc->band->phytype, wlc->band->gmode));
#ifdef BCMDBG
		wlc_rateset_show(wlc, &target_bss->rateset, &bi->BSSID);
#endif // endif
		wlc_rateset_default(&target_bss->rateset, &wlc->band->hw_rateset,
			wlc->band->phytype, wlc->band->bandtype, cck_only,
			RATE_MASK_FULL, cck_only ? 0 : wlc_get_mcsallow(wlc, NULL),
			CHSPEC_WLC_BW(bi->chanspec), wlc->stf->op_rxstreams);
	}

#ifdef BCMDBG
	wlc_rateset_show(wlc, &target_bss->rateset, &bi->BSSID);
#endif // endif

	/* Update SCB WB Capability */
	if (bi->ext_cap_flags & TDLS_CAP_TDLS_WIDER_BW) {
		scb_tdls->ext_cap_flags |= TDLS_SCB_TDLS_WB_CAP;
	}

	/* Update SCB Operating Mode Nofitication Capability */
	if (bi->ext_cap_flags & TDLS_CAP_OP_MODE_NOTIF)
		scb_tdls->ext_cap_flags |= TDLS_SCB_TDLS_OP_MODE_NOTIF;

	wlc_rate_lookup_init(wlc, &target_bss->rateset);

	bss_ht = ((bi->flags & WLC_BSS_HT)) && BSS_N_ENAB(wlc, SCB_BSSCFG(scb));
	bss_vht = ((bi->flags2 & WLC_BSS_VHT)) && BSS_VHT_ENAB(wlc, SCB_BSSCFG(scb));

	if (bss_ht) {
		ht_cap_ie_t *cap_ie;
		ht_add_ie_t *add_ie;
		obss_params_t *obss_ie = NULL;

		/* extract ht cap and additional ie */
		cap_ie = wlc_read_ht_cap_ies(wlc, parse, len);
		add_ie = wlc_read_ht_add_ies(wlc, parse, len);
		if (COEX_ENAB(wlc))
			obss_ie = wlc_ht_read_obss_scanparams_ie(wlc, parse, len);

		wlc_ht_update_scbstate(wlc->hti, scb, cap_ie, add_ie, obss_ie);
#ifdef WL11AC
		if (bss_vht) {
			vht_cap_ie_t vht_cap_ie;
			int brcm_prop_tlv_len = 0;
			uint8 *brcm_prop_tlv = NULL;
			uint8 vht_ratemask = 0;
			/*
			 * Locate  the VHT cap IE
			 * Encapsulated VHT Prop IE appears if we are running VHT in 2.4G or
			 * the extended rates are enabled
			 */
			if (BAND_2G(wlc->band->bandtype) || WLC_VHT_FEATURES_RATES(wlc->pub)) {
				brcm_prop_tlv = wlc_read_vht_features_ie(wlc->vhti,
				parse, len, &vht_ratemask, &brcm_prop_tlv_len, target_bss);
			}
			if (wlc_read_vht_cap_ie(wlc->vhti,
				((brcm_prop_tlv != NULL) ? brcm_prop_tlv : parse),
				((brcm_prop_tlv_len != 0)?brcm_prop_tlv_len : len), &vht_cap_ie)) {
				wlc_vht_update_scb_state(wlc->vhti, wlc->band->bandtype, scb,
					cap_ie, &vht_cap_ie, NULL,
					((BAND_5G(wlc->band->bandtype)) ?
					WLC_VHT_FEATURES_RATES_5G(wlc->pub) : vht_ratemask));
			}
		}
#endif /* WL11AC */
	} else if ((scb->flags & SCB_HTCAP) &&
	           ((bi->flags & WLC_BSS_HT) != WLC_BSS_HT))
		wlc_ht_update_scbstate(wlc->hti, scb, NULL, NULL, NULL);

	/* replace any old scb rateset with new target rateset */
	if (bss_ht)
		mcsallow |= WLC_MCS_ALLOW;
	if (bss_vht)
		mcsallow |= WLC_MCS_ALLOW_VHT;

	if (WLPROPRIETARY_11N_RATES_ENAB(wlc->pub)&&
	    wlc->pub->ht_features == WLC_HT_FEATURES_PROPRATES_FORCE)
		mcsallow |= WLC_MCS_ALLOW_PROP_HT;

	wlc_rateset_filter(&target_bss->rateset /* src */, &scb->rateset /* dst */, FALSE,
	                   WLC_RATES_CCK_OFDM, RATE_MASK, mcsallow);
	scb_tdls->base_chan_bandunit = wlc->band->bandunit;
	bcopy(&scb->rateset, &scb_tdls->base_chan_rateset, sizeof(wlc_rateset_t));
	wlc_scb_ratesel_init(wlc, scb);

#ifdef WLMCHAN
	if (MCHAN_ENAB(tdls->pub)) {
		WL_TDLS(("wl%d.%d: %s: Creating chanctx for 0x%x\n",
			tdls->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			__FUNCTION__, cur_bss->chanspec));
		wlc_mchan_create_bss_chan_context(wlc, SCB_BSSCFG(scb), cur_bss->chanspec);
	}
#endif /* WLMCHAN */

	/* If oper mode is rx and updated in tdls response, then update BW accordingly.
	 * This should be done after mchan context, dependency in update_bandwidth()
	 */
	if (bi->oper_mode != 0xFF) {
		/* Get local BW */
		bw = opermode_bw_chspec_map[DOT11_OPER_MODE_CHANNEL_WIDTH(cur_bss->chanspec)];
		/* Get peer BW from recvd oper mode */
		bw_peer = opermode_bw_chspec_map[DOT11_OPER_MODE_CHANNEL_WIDTH(bi->oper_mode)];
		common_bw = MIN(bw, bw_peer);

		cspec = wf_chspec_ctlchan(cur_bss->chanspec);
		cspec = wf_channel2chspec(cspec, common_bw);
		if (cspec && cspec != cur_bss->chanspec) {
			bi->chanspec = cspec;
			/* scb is local,
			 * update local opee_mode to send applied oper_mode in tdls cfm -> OMN IE.
			 */
			scb->bsscfg->oper_mode = MIN(bi->oper_mode, scb->bsscfg->oper_mode);
			wlc_update_bandwidth(wlc, scb->bsscfg, bi->chanspec);
		}
	}

	scb_tdls->base_chanspec = scb_tdls->cur_chanspec =
		scb_tdls->target_chanspec = cur_bss->chanspec;

	/* scb->bsscfg->associated = TRUE; */

	if (!tdls->uapsd_sleep || !(scb->flags2 & SCB2_TDLS_PU_BUFFER_STA))
		scb_tdls->ps_allowed = FALSE;
	else
		scb_tdls->ps_allowed = TRUE;

	wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);

	SCB_BSSCFG(scb)->enable = TRUE;

#ifdef WLMCNX
	wlc_mcnx_ra_set(wlc->mcnx, SCB_BSSCFG(scb));

	if (cur_bss->bcn_prb) {
		bi->bcn_prb = cur_bss->bcn_prb;
		bi->bcn_prb_len = cur_bss->bcn_prb_len;
		/* P2P: configure the BSS TSF */
		wlc_mcnx_adopt_bss(wlc->mcnx, SCB_BSSCFG(scb), bi);
		wlc_mcnx_dtim_upd(wlc->mcnx, SCB_BSSCFG(scb), TRUE);
	}
#endif // endif

	if (bi->ti_val) {
		WL_TDLS(("wl%d:%s(): set SA lifetime to %d s.\n",
			tdls->pub->unit, __FUNCTION__, bi->ti_val));
		scb->bsscfg->tdls->tpk_lifetime = bi->ti_val;
	}

	scb->bsscfg->tdls->up_time = 0;

#ifdef PROP_TXSTATUS
	/* allocate scb mac address handle for wlfc, if required. This
	 * is released when scb is freed at teardown.
	 */
	if (wlc_scb_wlfc_entry_add(wlc, scb) == BCME_OK) {
		WL_TDLS(("wl%d.%d: wlc_scb_wlfc_entry_add() successful for "
			"handle: %d, addr: %s\n",
			WLCWLUNIT(wlc), WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			scb->mac_address_handle, bcm_ether_ntoa(&scb->ea, eabuf)));
	}
#endif /* PROP_TXSTATUS */

	return 0;
}

static struct scb *
wlc_tdls_scb_create(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *ea, struct ether_addr *bssid, bool initiator)
{
	wlc_info_t *wlc = tdls->wlc;
	wlc_bsscfg_t *bsscfg = NULL;
	struct scb *scb = NULL;
	scb_tdls_cubby_t *scb_tdls_cubby;
	tdls_lookaside_t *dla;
	tdls_lookaside_t *dla_pre = NULL;
	scb_tdls_t *scb_tdls = NULL;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	ASSERT(parent != NULL);

	scb = wlc_tdls_scbfindband_all(wlc, ea, CHSPEC_WLCBANDUNIT(wlc->home_chanspec));

	if (!scb) {
		if (tdls->cur_sessions >= tdls->max_sessions) {
			/* check again to see if aging helped */
			if (tdls->cur_sessions >= tdls->max_sessions) {
				WL_TDLS(("wl%d: wlc_tdls_scb_lookup: could NOT age out "
					"immediately\n", tdls->pub->unit));
				return NULL;
			}
		}

		bsscfg = wlc_tdls_bsscfg_create(wlc, parent, initiator);
		if (bsscfg == NULL) {
			WL_ERROR(("wl%d: cannot malloc bsscfg\n", tdls->pub->unit));
			return NULL;
		}

		scb = wlc_scblookupband(wlc, bsscfg, ea,
		                        CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
		if (!scb) {
			WL_ERROR(("wl%d: wlc_tdls_scb_lookup: out of scbs\n",
				tdls->pub->unit));
			goto fail;
		}

		WL_TDLS(("wl%d: Created a new TDLS scb with ea %s\n",
			tdls->pub->unit, bcm_ether_ntoa(ea, eabuf)));
	} else {
		/* cleanup up the existing info on the scb */
		scb_tdls_deinit(tdls, scb);

		WL_TDLS(("wl%d: Reusing TDLS scb with ea %s\n",
			tdls->pub->unit, bcm_ether_ntoa(ea, eabuf)));

		bsscfg = wlc_tdls_bsscfg_create(wlc, parent, initiator);
		if (bsscfg == NULL) {
			WL_ERROR(("wl%d: cannot malloc bsscfg\n", tdls->pub->unit));
			goto fail;
		}
	}

	/* malloc the scb cubby data */
	scb_tdls = (scb_tdls_t *)MALLOCZ(tdls->osh, sizeof(scb_tdls_t));
	if (!scb_tdls) {
		WL_ERROR(("wl%d: cannot malloc scb_tdls\n", tdls->pub->unit));
		goto fail;
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		bsscfg->packet_filter = WLC_ACCEPT_DIRECTED;
	}
#endif // endif

	if (bssid)
		bcopy(bssid, &bsscfg->BSSID, ETHER_ADDR_LEN);
	else {
		/* create a random bssid */
		wlc_getrand(wlc, &bsscfg->BSSID.octet[0], ETHER_ADDR_LEN);

		/* Set MAC addr to unicast and "locally administered" */
		ETHER_SET_LOCALADDR(&bsscfg->BSSID);
		ETHER_SET_UNICAST(&bsscfg->BSSID);
	}

	wlc_scb_set_bsscfg(scb, bsscfg);
	bsscfg->tdls_scb = scb;

	scb_tdls_cubby = SCB_TDLS_CUBBY(tdls, scb);
	scb_tdls_cubby->cubby = scb_tdls;
	scb_tdls->parent = parent;
	scb_tdls->ps_allowed = tdls->uapsd_sleep;

	if (WSEC_ENABLED(parent->wsec))
		wlc_getrand(wlc,
			initiator ? scb_tdls->snonce : scb_tdls->anonce,
			32);

	pktq_init(&scb_tdls->ubufq, 1, PKTQ_LEN_DEFAULT);

	/* cleanup the lookaside entry if it exists */
	dla = wlc_tdls_lookaside_find(tdls, ea);
	if (dla) {
		if (tdls->auto_op) {
			wlc_scb_rssi_init(scb, dla->rssi);
		}
		dla_pre = dla;
		wlc_tdls_lookaside_delete(tdls, dla);
	}
	dla = wlc_tdls_lookaside_create(tdls, ea, parent, TDLS_LOOKASIDE_ACTIVE);
	if (!dla) {
		WL_TDLS(("wl%d:%s(): TDLS lookaside table is full!\n",
			tdls->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (dla == dla_pre)
		dla = wlc_tdls_lookaside_move(tdls, dla);

	scb_tdls->peer_addr = dla;

	tdls->cur_sessions++;

	wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);

	wlc_set_ps_ctrl(parent);
	scb->listen = TDLS_DEFAULT_PEER_LISTEN_INTERVAL;

	return scb;

fail:
	if (scb != NULL)
		_wlc_tdls_free_scb(wlc, scb);
	if (scb_tdls != NULL)
		MFREE(wlc->osh, scb_tdls, sizeof(scb_tdls_t));
	if (bsscfg != NULL)
		wlc_bsscfg_free(wlc, bsscfg);
	return NULL;
}

void
wlc_tdls_port_open(tdls_info_t *tdls, struct ether_addr *ea)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */
	wlc_bsscfg_t *parent;

	scb = wlc_tdls_scbfindband_all(wlc, ea, CHSPEC_WLCBANDUNIT(wlc->home_chanspec));
	if (!scb || !BSSCFG_IS_TDLS(scb->bsscfg) || !scb->bsscfg->wlcif) {
		WL_ERROR(("wl%d: wlc_tdls_port_open: tdls scb not found for ea %s\n",
			tdls->pub->unit, bcm_ether_ntoa(ea, eabuf)));
		return;
	}

	/* mark scb as associated */
	scb->state |= ASSOCIATED;

	/* Authorize scb for data */
	wlc_ioctl(wlc, WLC_SCB_AUTHORIZE, ea, ETHER_ADDR_LEN, scb->bsscfg->wlcif);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls != NULL);

	parent = scb_tdls->parent;
	ASSERT(parent != NULL);
	wlc_set_pmstate(parent, parent->pm->PMenabled);

}

/*
 * Set PS mode and communicate new state to AP by sending null data frame.
 */
static void
wlc_tdls_set_pmstate(tdls_info_t *tdls, struct scb *scb, bool state)
{
	wlc_bsscfg_t *cfg = SCB_BSSCFG(scb);
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	wlc_tdls_t *tc = cfg->tdls;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

	WL_TDLS(("wl%d.%d: PM-MODE: wlc_set_pmstate: set PMenabled to %d\n",
	       tdls->pub->unit, WLC_BSSCFG_IDX(cfg), state));

	if (!scb_tdls->ps_allowed) {
		WL_TDLS(("wl%d.%d: PM-MODE: wlc_set_pmstate: set PMenabled to %d\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), state));
		return;
	}

	pm->PMenabled = state;
	pm->PMpending = TRUE;

	tc->tdls_PMEnable = state;

	/* Update TDLS PM state */
	wlc_set_ps_ctrl(SCB_BSSCFG(scb));
	wlc_tdls_wake_ctrl(wlc, tdls, scb);

	if (SCB_PS(scb) && state) {
		/* peer is u-apsd sleep STA, need to notify peer of us to be a u-apsd sleep STA
		 * as well
		 * 1) send PTI req through AP to peer sleep STA
		 * 2) peer sleep STA sends PS-Poll to AP to get PTI through AP and sends PTI
		 * resp through
		 *     direct link
		 * 3) send null-data with PM=1 and EOSP=1 to peer
		 */

		scb_tdls->flags |= TDLS_SCB_PM_SET;
		wlc_tdls_send_pti(tdls, scb);
	}
	else {
		/* announce PM change */
		/* send NULL data frame to communicate PM state to each associated APs */
		/* don't bother sending a null data frame if we lost our AP connection */
		WL_TDLS(("wl%d.%d:%s(): send null-data to peer buffer STA.\n",
			tdls->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		if (!wlc_sendnulldata(wlc, cfg, &scb->ea, 0, 0, PRIO_8021D_BE, NULL, NULL))
		{
			WL_ERROR(("wl%d.%d: failed to send PM null frame, "
				"fake a PM0->PM1 transition\n",
				tdls->pub->unit, WLC_BSSCFG_IDX(cfg)));
			tc->tdls_PMEnable = !state;

			pm->PMenabled = !state;
			pm->PMpending = FALSE;

			wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);
			wlc_set_ps_ctrl(scb_tdls->parent);

			wlc_set_ps_ctrl(SCB_BSSCFG(scb));
			wlc_tdls_wake_ctrl(wlc, tdls, scb);
		}
	}

}

void
wlc_tdls_update_pm(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, uint txstatus)
{
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	wlc_bsscfg_t *parent;
	wlc_pm_st_t *pm;

	ASSERT(bsscfg != NULL);
	if (!BSS_TDLS_ENAB(wlc, bsscfg))
		return;

	pm = bsscfg->pm;
	scb = bsscfg->tdls_scb;

	pm->PMpending = FALSE;

	if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
		WL_TDLS(("wl%d.%d:%s(): pm->PMpending = %s, txstatus = 0x%x\n",
			tdls->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
			pm->PMpending ? "TRUE" : "FALSE", txstatus));
		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls != NULL);

		parent = scb_tdls->parent;
		if (!parent)
			return;

		if (pm->PMenabled) {
			void *p;

			wlc_txmod_unconfig(tdls->wlc, scb, TXMOD_TDLS);

			/* release all the packets from the buffered queues */
			while ((p = pktq_deq(&scb_tdls->ubufq, NULL))) {
				wlc_tdls_reinsert_pkt(tdls->wlc, scb_tdls->parent, scb, p);
				WLCNTINCR(tdls->cnt->buf_reinserted);
			}
		}
		pm->PMenabled = FALSE;

		wlc_tdls_pm_allowed_update(tdls, parent);
		wlc_set_ps_ctrl(parent);
		wlc_set_ps_ctrl(SCB_BSSCFG(scb));
		goto done;
	}
	if (pm->apsd_sta_usp) {
		WL_PS(("wl%d.%d: APSD sleep\n", tdls->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		wlc_set_apsd_stausp(bsscfg, FALSE);
	}

done:
	wlc_tdls_wake_ctrl(wlc, tdls, scb);

}

bool
wlc_tdls_pm_allowed(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg)
{
	return TRUE;
}

static void
wlc_tdls_pm_allowed_update(tdls_info_t *tdls, wlc_bsscfg_t *parent)
{
	return;
}

/* sends NULL-data frames with PS bit to all TDLS peers */
void
wlc_tdls_notify_pm_state(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, bool state)
{
	struct scb *scb;
	scb_tdls_t *scb_tdls;

	if (!tdls->uapsd_sleep)
		return;

	scb = bsscfg->tdls_scb;
	ASSERT(scb != NULL);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	if (!(scb->flags2 & SCB2_TDLS_PU_BUFFER_STA) ||
		!(scb_tdls->flags & TDLS_SCB_CONNECTED))
		return;

	wlc_tdls_set_pmstate(tdls, scb, state);

	/* Add/delete the TDLS to the txpath for this SCB */
	if (state)
		wlc_txmod_config(tdls->wlc, scb, TXMOD_TDLS);
	else {
		void *p;

		wlc_txmod_unconfig(tdls->wlc, scb, TXMOD_TDLS);

		/* release all the packets from the buffered queues */
		while ((p = pktq_deq(&scb_tdls->ubufq, NULL))) {
			wlc_tdls_reinsert_pkt(tdls->wlc, scb_tdls->parent, scb, p);
			WLCNTINCR(tdls->cnt->buf_reinserted);
		}
	}

	return;
}

/* function to find the lookaside entry with the matching ea */
static tdls_lookaside_t *
wlc_tdls_lookaside_find(tdls_info_t *tdls, struct ether_addr *ea)
{
	int i;
	tdls_lookaside_t *dla;

	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		dla = &tdls->lookaside[i];
		if (!bcmp(&dla->ea, ea, ETHER_ADDR_LEN))
			return dla;
	}

	return NULL;
}

/* function to create the lookaside entry with the matching ea
 * Entry should not exist before calling this function
 */
static tdls_lookaside_t *
wlc_tdls_lookaside_create(tdls_info_t *tdls, struct ether_addr *ea, wlc_bsscfg_t *parent,
	uint8 status)
{
	int i;

	ASSERT(!wlc_tdls_lookaside_find(tdls, ea));

	/* get next free entry */
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		if (!(tdls->lookaside[i].flags & TDLS_LOOKASIDE_INUSE))
			break;
	}

	/* XXX: return for now if no free entry found;
	 * either age or increase size of lookaside
	 */
	if (i == TDLS_LOOKASIDE_SIZE)
		return NULL;

	bcopy(ea, &tdls->lookaside[i].ea, ETHER_ADDR_LEN);
	tdls->lookaside[i].flags = TDLS_LOOKASIDE_INUSE;
	tdls->lookaside[i].parent = parent;
	tdls->lookaside[i].status = status;
	tdls->lookaside[i].upd_time = OSL_SYSUPTIME();
	tdls->lookaside[i].wfd_ie_probe_rx_length = 0;
	tdls->lookaside[i].pmoverride_set = FALSE;

	WL_TDLS(("wl%d:%s(): create lookaside talbe entry idx=%d!\n",
		tdls->pub->unit, __FUNCTION__, i));

	return &tdls->lookaside[i];
}

static void
wlc_tdls_lookaside_delete(tdls_info_t *tdls, tdls_lookaside_t *peer)
{
	struct wl_timer *timer = peer->timer;
	ASSERT(peer);

	if (peer->timer) {
		WL_TDLS(("wl%d:%s(): disarm the peer TDLS timer!\n",
			tdls->pub->unit, __FUNCTION__));
		wl_del_timer(tdls->wlc->wl, peer->timer);
	}
	if (peer->pmoverride_set) {
		WL_TDLS(("wl%d:%s(): Turn pmoverride off!\n",
			tdls->pub->unit, __FUNCTION__));
		wlc_set_pmoverride(peer->parent, FALSE);
		peer->pmoverride_set = FALSE;
	}

	if (peer->wfd_ie_probe_rx_length && peer->wfd_ie_probe_rx)
		MFREE(tdls->osh, peer->wfd_ie_probe_rx, peer->wfd_ie_probe_rx_length);

	bzero(peer, sizeof(tdls_lookaside_t));
	peer->timer = timer;

}
static int
BCMATTACHFN(wlc_tdls_lookaside_inittimers)(tdls_info_t *tdls)
{
	int i;
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		/* create a timer for each entry */
		if ((tdls->lookaside[i].timer =
			wl_init_timer(tdls->wlc->wl, wlc_tdls_peer_timer, (void *)&timer_arg[i],
				"tdls_peer")) == NULL) {
			WL_ERROR(("wl%d: wl_init_timer for TDLS peer timer failed\n",
				tdls->pub->unit));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

static void
BCMATTACHFN(wlc_tdls_lookaside_freetimers)(tdls_info_t *tdls)
{
	int i;
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		/* free the timer for each entry */
		if (tdls->lookaside[i].timer) {
			WL_TDLS(("wl%d:%s(): free the peer lookaside timer!\n",
				tdls->pub->unit, __FUNCTION__));
			wl_free_timer(tdls->wlc->wl, tdls->lookaside[i].timer);
			tdls->lookaside[i].timer = NULL;
		}
	}
}

static tdls_lookaside_t *
wlc_tdls_lookaside_move(tdls_info_t *tdls, tdls_lookaside_t *peer)
{
	int i;
	struct wl_timer *peer_timer, *target_timer;

	ASSERT(peer);

	/* get next free entry */
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		if (!(tdls->lookaside[i].flags & TDLS_LOOKASIDE_INUSE))
			break;
	}
	if (i == TDLS_LOOKASIDE_SIZE)
		return peer;

	peer_timer = peer->timer;
	target_timer = tdls->lookaside[i].timer;
	/* copy the peer to the new entry in the lookaside table */
	bcopy(peer, &tdls->lookaside[i], sizeof(tdls_lookaside_t));
	bzero(peer, sizeof(tdls_lookaside_t));
	peer->timer = peer_timer;
	tdls->lookaside[i].timer = target_timer;

	WL_TDLS(("wl%d:%s(): move lookaside table entry to %d, flags = 0x%x!\n",
		tdls->pub->unit, __FUNCTION__, i, tdls->lookaside[i].flags));

	return &tdls->lookaside[i];
}

static void
wlc_tdls_lookaside_status_upd(tdls_info_t * tdls, tdls_lookaside_t * peer, uint8 status)
{
	ASSERT(peer);

	peer->status = status;
	peer->upd_time = OSL_SYSUPTIME();
}

void
wlc_tdls_free_scb(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;
	void *p;

	ASSERT(tdls != NULL);
	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls != NULL);
	ASSERT(scb_tdls->peer_addr);

	/* mark scb for deletion in 0 timer. Leads to race conditions
	 * if we delete here
	 */
	wlc_txmod_unconfig(tdls->wlc, scb, TXMOD_TDLS);

	/* release all the packets from the buffered queues */
	while ((p = pktq_deq(&scb_tdls->ubufq, NULL))) {
		WLPKTTAG(p)->flags &= ~(WLF_TDLS_TYPE | WLF_TDLS_DIRECT);
		WLPKTTAG(p)->flags |= WLF_TDLS_APPATH;
		wlc_tdls_reinsert_pkt(tdls->wlc, scb_tdls->parent, scb, p);
		WLCNTINCR(tdls->cnt->buf_reinserted);
	}

	if (!scb_tdls->free_me && tdls->up)
		wl_add_timer(tdls->wlc->wl, scb_tdls->peer_addr->timer, 0, 0);
	else {
		wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
		wlc_bsscfg_t *parent = scb_tdls->parent;

		ASSERT(parent != NULL);
		if (scb_tdls->timer_start) {
			WL_TDLS(("wl%d:%s(): del peer TDLS timer!\n",
				tdls->pub->unit, __FUNCTION__));
			wl_del_timer(tdls->wlc->wl, scb_tdls->peer_addr->timer);
		}
		_wlc_tdls_free_scb(tdls->wlc, scb);
		wlc_tdls_bsscfg_free(tdls, bsscfg, parent);
		wlc_tdls_pretbtt_update(tdls->wlc);
	}

}

static void
wlc_tdls_disconnect(tdls_info_t *tdls, struct scb *scb, bool force)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

	ASSERT(scb_tdls != NULL);
	ASSERT(scb_tdls->peer_addr);

	/* Check if any other TDLS BSS has higher bandwitdh */
	if (scb->bsscfg->current_bss->chanspec &&
		scb->bsscfg->current_bss->chanspec !=
		scb_tdls->parent->current_bss->chanspec) {
		wlc_update_bandwidth(wlc, scb->bsscfg, scb_tdls->parent->current_bss->chanspec);
		scb_tdls->base_chanspec = scb_tdls->cur_chanspec =
		scb_tdls->target_chanspec = scb_tdls->parent->current_bss->chanspec;
	}

	if (scb_tdls->timer_start) {
		WL_TDLS(("wl%d:%s(): del peer TDLS timer!\n", tdls->pub->unit, __FUNCTION__));
		wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
		scb_tdls->timer_start = FALSE;
	}

#ifdef TDLS_TESTBED
	if (tdls->quiet_down) {
		/* Don't send TDLS tear down */
		force = FALSE;
	}
#endif // endif

	wlc_bss_mac_event(wlc, scb->bsscfg, WLC_E_TDLS_PEER_EVENT, &scb->ea,
		WLC_E_STATUS_SUCCESS, WLC_E_TDLS_PEER_DISCONNECTED, 0, NULL, 0);

	if (force && (scb_tdls->flags & TDLS_SCB_CONNECTED)) {
		if (!tdls->up ||
			(scb_tdls->flags & TDLS_SCB_SENT_PTI) ||
			SCB_PS(scb) || (scb_tdls->heartbeat_missed > TDLS_MAX_HEARTBEAT_MISSED)) {
			if (tdls->wl_down_triggered && !SCB_PS(scb))
				wlc_tdls_send_teardown(tdls, scb, TRUE);
			else
				wlc_tdls_send_teardown(tdls, scb, FALSE);
			scb_tdls->free_me = TRUE;
			wlc_tdls_free_scb(tdls, scb);
		}
		else
			wlc_tdls_send_teardown(tdls, scb, TRUE);
		/* free the scb in wlc_tdls_action_frame_tx_complete() */
	}
	else {
		WL_TDLS(("wl%d:%s(): scb_tdls->flags = 0x%x, free scb...!\n",
			tdls->pub->unit, __FUNCTION__, scb_tdls->flags));
		if (scb_tdls->flags & TDLS_SCB_RCVD_TEARDOWN)
			scb_tdls->free_me = FALSE;
		else
			scb_tdls->free_me = TRUE;
		wlc_tdls_free_scb(tdls, scb);
	}
	return;
}

static void
wlc_tdls_get_pkt(wlc_info_t *wlc, uint len, void **p)
{
	uint alloc_len;

	alloc_len = TDLS_HEADER_LEN + len;

	/* alloc new frame buffer */
	if ((*p = PKTGET(wlc->osh, (TXOFF + alloc_len), TRUE)) == NULL) {
		WL_ERROR(("wl%d: wlc_tdls_get_pkt: pktget failed\n",
			wlc->pub->unit));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return;
	}

	/* reserve TXOFF bytes of headroom */
	PKTPULL(wlc->osh, *p, TXOFF + TDLS_HEADER_LEN);
	PKTSETLEN(wlc->osh, *p, len);

	return;
}

static void
wlc_tdls_send_action_frame(tdls_info_t *tdls, wlc_bsscfg_t *parent,
	struct ether_addr *da, void *p, bool direct)
{
	wlc_info_t *wlc = tdls->wlc;
	struct ether_addr *sa;
	struct ether_header *eh;
	uchar *payload;

	ASSERT(parent != NULL);

	WL_TDLS(("wl%d:%s(): enter.\n", tdls->pub->unit, __FUNCTION__));
	TDLS_PRHEX("action_frame", PKTDATA(tdls->osh, p), PKTLEN(tdls->osh, p));

	sa = &parent->cur_etheraddr;

	/* push the TDLS header */
	PKTPUSH(wlc->osh, p, TDLS_HEADER_LEN);

	/* init ether_header */
	eh = (struct ether_header *) PKTDATA(tdls->osh, p);
	bcopy(da, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy(sa, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(ETHER_TYPE_89_0D);
	PKTSETPRIO(p, PRIO_8021D_VI);
	/* add payload type field */
	payload = (uchar *) &eh[1];
	*payload = TDLS_PAYLOAD_TYPE;
	TDLS_PRHEX("added_header pkt:", PKTDATA(tdls->osh, p), PKTLEN(tdls->osh, p));

	WLPKTTAG(p)->flags |= ((direct ? 0 : WLF_TDLS_APPATH));
	WLPKTTAG(p)->flags3 |= WLF3_TDLS_BYPASS_AMPDU;
	wlc_sendpkt(wlc, p, parent->wlcif);
}

static uint16
wlc_tdls_validate_linkId_ie(tdls_info_t *tdls, wlc_bsscfg_t *parent, uchar *tlv, int len,
	struct wlc_frminfo *f, bool initiator)
{
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */
	link_id_ie_t *linkId_ie;

	/* starts TLV IEs */
	linkId_ie = (link_id_ie_t*)bcm_parse_tlvs(tlv, len, DOT11_MNG_LINK_IDENTIFIER_ID);
	if (!linkId_ie) {
		WL_ERROR(("wl%d:%s(): no Link Identifier IE in setup Request!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	if (linkId_ie->len != TDLS_LINK_ID_IE_LEN) {
		WL_ERROR(("wl%d:%s(): Link Identifier IE len = %d wrong!\n",
			tdls->pub->unit, __FUNCTION__, linkId_ie->len));
		return DOT11_SC_DECLINED;
	}

#if defined(BCMDBG) || defined(BCMDBG_ERR)
	WL_TDLS(("wl%d:%s():", tdls->pub->unit, __FUNCTION__));
	WL_TDLS(("Link Identifier: BSSID %s, ", bcm_ether_ntoa(&linkId_ie->bssid, eabuf)));
	WL_TDLS(("init_mac %s, ", bcm_ether_ntoa(&linkId_ie->tdls_init_mac, eabuf)));
	WL_TDLS(("resp_mac %s\n", bcm_ether_ntoa(&linkId_ie->tdls_resp_mac, eabuf)));
#endif // endif

	if (memcmp(&linkId_ie->bssid, &parent->BSSID, ETHER_ADDR_LEN)) {
		WL_TDLS(("parent SSID %s, ", bcm_ether_ntoa(&parent->BSSID, eabuf)));
		WL_ERROR(("wl%d:%s(): BSSID mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	if (memcmp(&linkId_ie->tdls_init_mac,
		initiator ? f->da : f->sa,
		ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): Link Identifier Initiator mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_UNEXP_MSG;
	}
	if (memcmp(&linkId_ie->tdls_resp_mac,
		initiator ? f->sa : f->da,
		ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): Link Identifier responder mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_UNEXP_MSG;
	}

	return 0;

}

/*
 * Check if this TLVS entry is a WME IE or params IE entry.
 * If not, move the tlvs pointer/length to indicate the next TLVS entry.
 */
static bool
wlc_tdls_find_wme_qosinfo(uint8 *tlvs, uint tlvs_len, uint8 *qosinfo)
{
	bcm_tlv_t *tlv_ie;

	if ((tlv_ie =
		bcm_find_wmeie(tlvs, tlvs_len, WME_SUBTYPE_IE, WME_IE_LEN)) != NULL) {
		wme_ie_t *wme_ie;

		wme_ie = (wme_ie_t *)(tlv_ie->data);
		*qosinfo = wme_ie->qosinfo;
		WL_TDLS(("%s(): find WME IE, qosinfo = 0x%x\n", __FUNCTION__, *qosinfo));
		TDLS_PRHEX("wme_ie:", (uchar*)wme_ie, WME_IE_LEN);
		return TRUE;
	}

	if ((tlv_ie =
		bcm_find_wmeie(tlvs, tlvs_len, WME_SUBTYPE_PARAM_IE, WME_PARAM_IE_LEN)) != NULL) {
		wme_param_ie_t *wme_param_ie;

		wme_param_ie = (wme_param_ie_t *)(tlv_ie->data);
		*qosinfo = wme_param_ie->qosinfo;
		WL_TDLS(("%s(): find WME PARAM IE, qosinfo = 0x%x\n", __FUNCTION__, *qosinfo));
		TDLS_PRHEX("wme_param_ie:", (uchar*)wme_param_ie, WME_PARAM_IE_LEN);
		return TRUE;
	}

	if ((tlv_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_QOS_CAP_ID)) != NULL) {
		qos_cap_ie_t *qoscap_ie_tlv = (qos_cap_ie_t *)tlv_ie;
		*qosinfo = qoscap_ie_tlv->qosinfo;
		WL_TDLS(("%s(): find QoS CAP IE, qosinfo = 0x%x\n", __FUNCTION__, *qosinfo));
		return TRUE;
	}

	return FALSE;
}

/* Supported Rates IE */
static int
wlc_tdls_setup_parse_sup_rates_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	bcm_tlv_t *sup_rates = (bcm_tlv_t *)parse->ie;

	/* parse rateset and pull out raw rateset */
	if (sup_rates == NULL) {
		WL_INFORM(("Missing or Invalid rate info in beacon\n"));
	}
	/* Check for a legacy 54G bcn/proberesp by looking for more than 8 rates
	 * in the Supported Rates elt
	 */
	else if (sup_rates->len > 8) {
		bi->flags |= WLC_BSS_54G;
	}

	return BCME_OK;
}

/* Ext Cap IE */
static int
wlc_tdls_setup_parse_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	dot11_extcap_ie_t *extcap_ie_tlv = (dot11_extcap_ie_t *)parse->ie;
	dot11_extcap_t *cap;

	/* too short ies are possible: guarded against here */
	if (extcap_ie_tlv == NULL) {
		return BCME_OK;
	} else if (parse->ie_len < DOT11_EXTCAP_LEN_TDLS + TLV_HDR_LEN) {
		WL_INFORM(("%s:malformed ie ignore\n", __FUNCTION__));
		return BCME_OK;
	}

	cap = (dot11_extcap_t*)(extcap_ie_tlv->cap);
	if (extcap_ie_tlv->len >= DOT11_EXTCAP_LEN_TDLS) {
		if (isset(cap->extcap, DOT11_TDLS_CAP_PROH))
				bi->ext_cap_flags |= TDLS_CAP_PROH;
		if (isset(cap->extcap, DOT11_TDLS_CAP_CH_SW_PROH))
				bi->ext_cap_flags |= TDLS_CAP_CH_SW_PROH;
		if (isset(cap->extcap, DOT11_TDLS_CAP_TDLS))
				bi->ext_cap_flags |= TDLS_CAP_TDLS;
		if (isset(cap->extcap, DOT11_TDLS_CAP_PU_BUFFER_STA))
				bi->ext_cap_flags |= TDLS_CAP_PU_BUFFER_STA;
		if (isset(cap->extcap, DOT11_TDLS_CAP_PEER_PSM))
				bi->ext_cap_flags |= TDLS_CAP_PEER_PSM;
		if (isset(cap->extcap, DOT11_TDLS_CAP_CH_SW))
				bi->ext_cap_flags |= TDLS_CAP_CH_SW;
	}

	if (extcap_ie_tlv->len >= DOT11_11AC_EXTCAP_LEN_TDLS) {
		if (isset(cap->extcap, DOT11_TDLS_CAP_TDLS_WIDER_BW))
			bi->ext_cap_flags |= TDLS_CAP_TDLS_WIDER_BW;
		if (isset(cap->extcap, DOT11_EXT_CAP_OPER_MODE_NOTIF)) {
			bi->ext_cap_flags |= TDLS_CAP_OP_MODE_NOTIF;
		}
	}

	return BCME_OK;
}

/* Lifetime Timeout IE */
static int
wlc_tdls_setup_parse_ft_ti_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	ti_ie_t *ti_ie;

	if (parse->ie == NULL)
		return BCME_OK;

	/* too short ies are possible: guarded against here */
	if (parse->ie_len != TLV_HDR_LEN + sizeof(ti_ie_t)) {
		/* may happen if ie is corrupted etc. */
		WL_ERROR(("%s:malformed ie abort\n", __FUNCTION__));
		return BCME_BADLEN;
	}

	ti_ie = (ti_ie_t *)&parse->ie[TLV_BODY_OFF];
	bi->ti_type = ti_ie->ti_type;
	bi->ti_val = ti_ie->ti_val;

	return BCME_OK;
}

/* FTIE */
static int
wlc_tdls_setup_parse_fte_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	dot11_ft_ie_t *ft_ie = (dot11_ft_ie_t *)parse->ie;

	if (ft_ie == NULL)
		return BCME_OK;

	/* too short ies are possible: guarded against here */
	if (parse->ie_len != sizeof(dot11_ft_ie_t)) {
		WL_ERROR(("%s:malformed ie abort\n", __FUNCTION__));
		return BCME_BADLEN;
	}

	bcopy(ft_ie->anonce, bi->anonce, EAPOL_WPA_KEY_NONCE_LEN);
	bcopy(ft_ie->snonce, bi->snonce, EAPOL_WPA_KEY_NONCE_LEN);
	bcopy(ft_ie->mic, bi->mic, 16);

	return BCME_OK;
}

/* WME parameters */
static int
wlc_tdls_setup_parse_wme_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;

	if (parse->ie == NULL)
		return BCME_OK;

	if (WME_ENAB(tdls->pub)) {
		uint8 qosinfo = 0;

		if (wlc_tdls_find_wme_qosinfo(parse->ie, parse->ie_len, &qosinfo)) {
			bi->flags |= WLC_BSS_WME;
			bi->wme_qosinfo = qosinfo;
		}
	}

	return BCME_OK;
}

static void
wlc_tdls_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_tdls_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

static int
wlc_tdls_parse_setup_ies(tdls_info_t *tdls, struct ether_addr *bssid,
	void *tlvs, uint tlvs_len, wlc_bss_info_t *bi, uint8 action)
{
	wlc_info_t *wlc = tdls->wlc;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	wlc_ier_reg_t *ier = NULL;

	bcopy((char *)bssid, (char *)&bi->BSSID, ETHER_ADDR_LEN);

	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_tdls_nhdlr_cb;
	upp.vsie_fn = wlc_tdls_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.tdls.result = bi;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	switch (action) {
	case TDLS_SETUP_REQ:
		ier = wlc->ier_tdls_srq;
		break;
	case TDLS_SETUP_RESP:
		ier = wlc->ier_tdls_srs;
		break;
	case TDLS_SETUP_CONFIRM:
		ier = wlc->ier_tdls_scf;
		break;
	default:
		return BCME_BADARG;
	}

	return wlc_ier_parse_frame(ier, NULL, WLC_IEM_FC_UNK, &upp, &pparm, tlvs, tlvs_len);
}

static void
wlc_tdls_qosinfo_update(tdls_info_t *tdls, struct scb *scb, wlc_bss_info_t *bi)
{
	scb_tdls_t *scb_tdls;

	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	WL_TDLS(("wl%d:%s(): bi->flags=0x%0x, bi->wme_qosinfo=0x%x\n",
		tdls->pub->unit, __FUNCTION__, bi->flags, bi->wme_qosinfo));

	/* Handle WME association */
	scb->flags &= ~(SCB_WMECAP | SCB_APSDCAP);
	scb->flags2 &= ~SCB2_TDLS_PU_SLEEP_STA;
	if (BSS_WME_ENAB(tdls->wlc, scb_tdls->parent)) {
		/* wlc_pm_st_t *pm = scb->bsscfg->pm; */

		wlc_qosinfo_update(scb, 0, TRUE);     /* Clear Qos Info by default */
		if (bi->flags & WLC_BSS_WME) {
			scb->flags |= SCB_WMECAP;
			if (bi->wme_qosinfo) {
				wlc_qosinfo_update(scb, bi->wme_qosinfo, TRUE);
				if (scb->apsd.ac_trig & AC_BITMAP_ALL)
					scb->flags |= SCB_APSDCAP;
			}
			scb->flags2 |= SCB2_TDLS_PU_SLEEP_STA;
		}
	}

}

static uint8
wlc_tdls_validate_wsec(tdls_info_t *tdls, wlc_bsscfg_t *parent, struct scb *scb,
	wlc_bss_info_t *bi, tpk_handshake_msg_t tpk_msg)
{
	int i;
	uchar zero_mem[32];

	ASSERT(tdls);

	if (WSEC_AES_ENABLED(parent->wsec) == 0) {
		if (bi->flags & WLC_BSS_WPA2) {
			WL_TDLS(("wl%d():%s(): AP link security not enabled. "
				"return 5(DOT11_SC_TDLS_SEC_DISABLED)\n",
				tdls->pub->unit, __FUNCTION__));
			return DOT11_SC_TDLS_SEC_DISABLED;
		}
		else
			return 0;
	}

	if (!(bi->flags & WLC_BSS_WPA2)) {
		WL_TDLS(("wl%d():%s(): No RSNIE, return 38 (DOT11_SC_INVALID_PARAMS)\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_INVALID_PARAMS;
	}

	if ((bi->wpa2.acount != 1) && (bi->wpa2.auth[0] != RSN_AKM_TPK)) {
		WL_TDLS(("wl%d():%s(): RSNIE: AKM is not TPK, return 43 (DOT11_SC_INVALID_AKMP)\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_INVALID_AKMP;
	}

	if (bi->wpa2.ucount == 0) {
		WL_TDLS(("wl%d():%s(): RSNIE: No uni-cast cipher. "
			"return 42 (DOT11_SC_INVALID_PAIRWISE_CIPHER)\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_INVALID_PAIRWISE_CIPHER;
	}

	for (i = 0; i < bi->wpa2.ucount; i++) {
		if (bi->wpa2.unicast[i] != WPA_CIPHER_AES_CCM) {
			WL_TDLS(("wl%d():%s(): RSNIE: uni-cast cipher = %d not acceptable"
				"return 42 (DOT11_SC_INVALID_PAIRWISE_CIPHER)\n",
				tdls->pub->unit, __FUNCTION__, bi->wpa2.unicast[i]));
			return DOT11_SC_INVALID_PAIRWISE_CIPHER;
		}
	}

	if (!(bi->wpa2.flags & RSN_FLAGS_PEER_KEY_ENAB)) {
		WL_TDLS(("wl%d():%s(): RSNIE: PeerKeyEnable not set in RSN Capabilities. "
			"return 45 (DOT11_SC_INVALID_RSNIE_CAP)\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_INVALID_RSNIE_CAP;
	}

	if ((bi->ti_type != TI_TYPE_KEY_LIFETIME) ||
		(bi->ti_val < TDLS_SA_LIFETIME_MIN)) {
		WL_TDLS(("wl%d():%s(): Time Interval IE: Lifetime = %d is not acceptable. "
			"return 6 (DOT11_SC_LIFETIME_REJ)\n",
			tdls->pub->unit, __FUNCTION__, bi->ti_val));
		return DOT11_SC_LIFETIME_REJ;
	}

	bzero(zero_mem, 32);
	switch (tpk_msg) {
		case HANDSHAKE_MSG_1:
			if (!memcmp(bi->snonce, zero_mem, EAPOL_WPA_KEY_NONCE_LEN) ||
				memcmp(bi->anonce, zero_mem, EAPOL_WPA_KEY_NONCE_LEN) ||
				memcmp(bi->mic, zero_mem, 16)) {
				WL_TDLS(("wl%d():%s(): FTIE: invalid. return 55 "
					"(DOT11_SC_INVALID_FTIE)\n",
					tdls->pub->unit, __FUNCTION__));
				return DOT11_SC_INVALID_FTIE;
			}
			break;

		case HANDSHAKE_MSG_2:
		case HANDSHAKE_MSG_3: {
			scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

			if (memcmp(bi->snonce, scb_tdls->snonce, EAPOL_WPA_KEY_NONCE_LEN)) {
				WL_TDLS(("wl%d():%s(): FTIE: invalid SNonce. return 71 "
					"(DOT11_SC_INVALID_SNONCE)\n",
					tdls->pub->unit, __FUNCTION__));
				return DOT11_SC_INVALID_SNONCE;
			}

			if (tpk_msg == HANDSHAKE_MSG_3) {
				if (memcmp(bi->anonce, scb_tdls->anonce, EAPOL_WPA_KEY_NONCE_LEN)) {
					WL_TDLS(("wl%d():%s(): FTIE: invalid ANonce. return 72 "
						"(DOT11_SC_INVALID_RSNIE)\n",
						tdls->pub->unit, __FUNCTION__));
					return DOT11_SC_INVALID_RSNIE;
				}
			}

			if (bi->wpa2.ucount > 1) {
				WL_TDLS(("wl%d():%s(): Invalid RSN pairwise cipher, return 42 "
					"(DOT11_SC_INVALID_PAIRWISE_CIPHER)\n",
					tdls->pub->unit, __FUNCTION__));
				return DOT11_SC_INVALID_PAIRWISE_CIPHER;
			}

			if (scb_tdls->timeout_interval != bi->ti_val) {
				WL_TDLS(("wl%d():%s(): Timeout Interval IE not the same. %d vs %d"
					"return 6 (DOT11_SC_LIFETIME_REJ)\n",
					tdls->pub->unit, __FUNCTION__, scb_tdls->timeout_interval,
					bi->ti_val));
				return DOT11_SC_LIFETIME_REJ;
			}

			break;
		}

		default:
			break;
	}

	return 0;
}

/* plumb the pairwise key */
static void
wlc_tdls_wpa_plumb_tk(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, uint8 *tk, uint32 tk_len,
	uint32 cipher, struct ether_addr *ea)
{
	wlc_info_t *wlc = tdls->wlc;
	wl_wsec_key_t *key;
	int err;

	if (!(key = MALLOCZ(tdls->osh, sizeof(wl_wsec_key_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			tdls->pub->unit, __FUNCTION__,  MALLOCED(tdls->osh)));
		return;
	}

	WL_TDLS(("wl%d:wlc_tdls_wpa_plumb_tk()\n", tdls->pub->unit));

	key->len = tk_len;
	bcopy(tk, (char*)key->data, key->len);
	bcopy((char*)ea, (char*)&key->ea, ETHER_ADDR_LEN);
	/* NB: wlc_insert_key() will re-infer key.algo from key_len */
	key->algo = cipher;
	key->flags = WL_PRIMARY_KEY;

	err = wlc_iovar_op(wlc, "wsec_key", NULL, 0, key, sizeof(wl_wsec_key_t),
	                   IOV_SET, bsscfg->wlcif);
	if (err) {
		WL_ERROR(("wl%d: ERROR %d calling wlc_iovar_op with iovar \"wsec_key\"\n",
		          tdls->pub->unit, err));
	}

	MFREE(tdls->osh, key, sizeof(wl_wsec_key_t));
	return;
}

static void
wlc_tdls_cal_tpk(tdls_info_t *tdls, struct scb *scb, wlc_bss_info_t *bi)
{
	wlc_bsscfg_t *parent;
	wlc_bsscfg_t *bsscfg;
	scb_tdls_t *scb_tdls;
	wlc_tdls_t *tc;

	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);
	parent = scb_tdls->parent;
	bsscfg = SCB_BSSCFG(scb);
	tc = bsscfg->tdls;

	wpa_calc_tpk((tc->initiator ? &bsscfg->cur_etheraddr : &scb->ea),
		(tc->initiator ? &scb->ea : &bsscfg->cur_etheraddr),
		&parent->BSSID,
		(tc->initiator ? scb_tdls->snonce : bi->snonce),
		(tc->initiator ? bi->anonce : scb_tdls->anonce),
		scb_tdls->tpk,
		TDLS_TPK_LEN);

	return;
}

static int
tdls_aes_cmac_calc(tdls_info_t *tdls,
	const bcm_const_xlvp_t *lvp, int nlvps, const uint8 *key, const int key_len,
	uint8 *mic, int mic_len)
{
	uint8 *data = NULL;
	int data_len = 0;
	int i;
	int err = BCME_OK;

	for (i = 0; i < nlvps; ++i) {
		data_len += lvp[i].len;
	}

	/* not strictly necessary, but keep compatibility w/ old allocation */
	if (data_len > 256) {
		err = BCME_RANGE;
		goto done;
	}

	data = MALLOCZ(tdls->osh, data_len);
	if (!data) {
		err = BCME_NOMEM;
		goto done;
	}

	for (i = 0, data_len = 0; i < nlvps; ++i) {
		memcpy(&data[data_len], lvp[i].data, lvp[i].len);
		data_len += lvp[i].len;
	}

	aes_cmac_calc(data, (const size_t)data_len, key, key_len, mic, mic_len);
done:
	if (data) {
		MFREE(tdls->osh, data, data_len);
	}

	return err;
}

/* number of lvps for mic check calculations
 *	 initiator addr, responder addr, tran seq, link id ie, rsn ie,
 *   timeout interval ie, ft ie
 */
#define TDLS_CAL_MIC_CHK_NUM_LVPS 7

static bool
wlc_tdls_cal_mic_chk(tdls_info_t *tdls, struct scb *scb, uchar *tlvs, int tlvs_len, uint8 transId)
{
	bool check_mic = FALSE;
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);
	link_id_ie_t *link_id_ie;
	bcm_tlv_t *ie;
	int err = BCME_IE_NOTFOUND;
	uint8 mic_save[EAPOL_WPA_KEY_MIC_LEN];
	bcm_const_xlvp_t lvps[TDLS_CAL_MIC_CHK_NUM_LVPS];
	int nlvps = 0;

	link_id_ie = (link_id_ie_t*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_LINK_IDENTIFIER_ID);
	if (!link_id_ie) {
		goto done;
	}

	if (link_id_ie->len != TDLS_LINK_ID_IE_LEN) {
		err = BCME_BADLEN;
		goto done;
	}
	/* TDLS initiator STA MAC addr */
	lvps[nlvps].len = ETHER_ADDR_LEN;
	lvps[nlvps++].data = (const uint8 *)&link_id_ie->tdls_init_mac;

	/* TDLS responder STA MAC addr */
	lvps[nlvps].len = ETHER_ADDR_LEN;
	lvps[nlvps++].data = (const uint8 *)&link_id_ie->tdls_resp_mac;

	/* Transaction Sequence number */
	lvps[nlvps].len = 1;
	lvps[nlvps++].data = (const uint8 *)&transId;

	/* Link Identifier IE */
	lvps[nlvps].len = TDLS_LINK_ID_IE_LEN + TLV_HDR_LEN;
	lvps[nlvps++].data = (const uint8 *)link_id_ie;

	/* RSN IE */
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RSN_ID);
	if (!ie) {
		goto done;
	}

	lvps[nlvps].len = BCM_TLV_SIZE(ie);
	lvps[nlvps++].data = (const uint8 *)ie;

	/* Timeout Interval IE */
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_FT_TI_ID);
	if (!ie) {
		goto done;
	}
	if (ie->len != sizeof(ti_ie_t)) {
		err = BCME_BADLEN;
		goto done;
	}

	lvps[nlvps].len = BCM_TLV_SIZE(ie);
	lvps[nlvps++].data = (const uint8 *)ie;

	/* FTIE */
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_FTIE_ID);
	if (!ie) {
		goto done;
	} else {
		dot11_ft_ie_t *ft_ie = (dot11_ft_ie_t *)ie;

		if (ie->len != sizeof(dot11_ft_ie_t) - TLV_HDR_LEN) {
			err = BCME_BADLEN;
			goto done;
		}

		memset(mic_save, 0, EAPOL_WPA_KEY_MIC_LEN);
		if (memcmp(ft_ie->mic, mic_save, EAPOL_WPA_KEY_MIC_LEN)) {
			memcpy(mic_save, ft_ie->mic, sizeof(mic_save));
			check_mic = TRUE;
		}
		memset(ft_ie->mic, 0, EAPOL_WPA_KEY_MIC_LEN);
		lvps[nlvps].len = BCM_TLV_SIZE(ie);
		lvps[nlvps++].data = (const uint8 *)ie;
	}

	err = tdls_aes_cmac_calc(tdls, lvps, nlvps, scb_tdls->tpk, WPA_MIC_KEY_LEN,
		scb_tdls->mic, EAPOL_WPA_KEY_MIC_LEN);
	if (err != BCME_OK)
		goto done;

	if (check_mic && memcmp(mic_save, scb_tdls->mic, EAPOL_WPA_KEY_MIC_LEN)) {
		WL_TDLS(("wl%d:%s(): MIC error for trans seq NO. %d\n", tdls->pub->unit,
			__FUNCTION__, transId));
		TDLS_PRHEX("FTIE MIC:", mic_save, EAPOL_WPA_KEY_MIC_LEN);
		TDLS_PRHEX("Cal MIC:", scb_tdls->mic, EAPOL_WPA_KEY_MIC_LEN);
		err = BCME_MICERR;
		goto done;
	}

done:
	if (err != BCME_OK) {
		WL_ERROR(("wl%d:%s(): status %d!\n", tdls->pub->unit, __FUNCTION__, err));
	}

	return (err == BCME_OK);
}

/* number of lvps for teardown mic check calculations
 *	 link id ie, reason code, token, trans seq = 4, ft ie
 */
#define TDLS_CAL_TEARDOWN_MIC_CHK_NUM_LVPS 5
static bool
wlc_tdls_cal_teardown_mic_chk(tdls_info_t *tdls, struct scb *scb, uchar *tlvs, int tlvs_len,
	uint16 reason, bool check_mic)
{
	scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);
	link_id_ie_t *link_id_ie;
	bcm_tlv_t *ie;

	int err = BCME_IE_NOTFOUND;
	uint8 mic_save[EAPOL_WPA_KEY_MIC_LEN];
	bcm_const_xlvp_t lvps[TDLS_CAL_TEARDOWN_MIC_CHK_NUM_LVPS];
	int nlvps = 0;
	uint8 four = 4;

	/* cancatenation of Link ID IE, reason code, dialog toaken, transaction seq 4, FTIE */
	/* Link Identifier IE */

	link_id_ie = (link_id_ie_t*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_LINK_IDENTIFIER_ID);
	if (!link_id_ie) {
		goto done;
	}

	if (link_id_ie->len != TDLS_LINK_ID_IE_LEN) {
		err = BCME_BADLEN;
		goto done;
	}
	/* Link Identifier IE */
	lvps[nlvps].len = TDLS_LINK_ID_IE_LEN + TLV_HDR_LEN;
	lvps[nlvps++].data = (const uint8 *)link_id_ie;

	/* reason code */
	reason = htol16(reason); /* LE */
	lvps[nlvps].len = sizeof(reason);
	lvps[nlvps++].data = (const uint8 *)&reason;

	/* token */
	lvps[nlvps].len = sizeof(scb_tdls->con_token);
	lvps[nlvps++].data = (const uint8 *)&scb_tdls->con_token;

	/* transaction seq: 4 */
	lvps[nlvps].len = sizeof(four);
	lvps[nlvps++].data = (const uint8 *)&four;

	/* FTIE */
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_FTIE_ID);
	if (!ie) {
		goto done;
	}

	if (ie->len != sizeof(dot11_ft_ie_t) - TLV_HDR_LEN) {
		err = BCME_BADLEN;
		goto done;
	}

	if (check_mic) {
		memcpy(mic_save, ((dot11_ft_ie_t*)ie)->mic, EAPOL_WPA_KEY_MIC_LEN);
		memset(((dot11_ft_ie_t*)ie)->mic, 0, EAPOL_WPA_KEY_MIC_LEN);
	}

	lvps[nlvps].len = BCM_TLV_SIZE(ie);
	lvps[nlvps++].data = (const uint8 *)ie;

	err = tdls_aes_cmac_calc(tdls, lvps, nlvps, scb_tdls->tpk, WPA_MIC_KEY_LEN,
		((dot11_ft_ie_t*)ie)->mic, EAPOL_WPA_KEY_MIC_LEN);
	if (err != BCME_OK)
		goto done;

	if (check_mic &&
		memcmp(((dot11_ft_ie_t*)ie)->mic, mic_save, EAPOL_WPA_KEY_MIC_LEN)) {
		WL_TDLS(("wl%d:%s(): MIC error!\n", tdls->pub->unit, __FUNCTION__));
		TDLS_PRHEX("FTIE MIC:", mic_save, EAPOL_WPA_KEY_MIC_LEN);
		TDLS_PRHEX("Cal MIC:", ((dot11_ft_ie_t*)ie)->mic, EAPOL_WPA_KEY_MIC_LEN);
		err = BCME_MICERR;
		goto done;
	}

done:
	if (err != BCME_OK) {
		WL_ERROR(("wl%d:%s(): status %d!\n", tdls->pub->unit, __FUNCTION__, err));
	}

	return (err == BCME_OK);
}

static void
wlc_tdls_parse_rclist(tdls_info_t *tdls, struct scb *scb, uint8 *tlvs, int tlv_len)
{
	ASSERT(scb);

	if (BSS_TDLS_ENAB(tdls->wlc, scb->bsscfg) && (scb->flags2 & SCB2_TDLS_CHSW_SUPPORT)) {
		bcm_tlv_t *ie;
		scb_tdls_t *scb_tdls = SCB_TDLS(tdls, scb);

		ASSERT(scb_tdls);

		/* find regulatory class IE, if found copy the regclass
		 * list into the requested regclass
		 */
		ie = bcm_parse_tlvs(tlvs, tlv_len, DOT11_MNG_REGCLASS_ID);
		if (ie) {
			bzero(scb_tdls->rclist, MAXRCLISTSIZE);
			if (ie->len > 0 && ie->len < MAXRCLISTSIZE) {
				scb_tdls->rclen = ie->len;
				bcopy(ie->data, scb_tdls->rclist, ie->len);
			} else
				WL_TDLS(("wl%d: improper regulatory class list size %d\n",
					tdls->pub->unit, ie->len));
		}
	}
}

static int
wlc_tdls_process_setup_req(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	int err;
	uint8 token;
	uint16 cap;
	uchar *pdata;
	int plen;
	struct scb_iter scbiter;
	struct scb *scb_i;
	scb_tdls_t *scb_tdls;
	uint16 status_code = 0;
	link_id_ie_t *linkId_ie = NULL;
	wlc_bss_info_t *bi = NULL;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);
	wlc_bsscfg_t *bsscfg = NULL;

#ifdef TDLS_TESTBED
	if (tdls->test_no_resp)
		return DOT11_SC_DECLINED;
#endif // endif

	WLCNTINCR(tdls->cnt->rxsetupreq);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at token field */
	token = *pdata++;
	plen--;

	/* capability */
	cap = ntoh16(*(uint16*)pdata);
	pdata += 2;
	plen -= 2;

	linkId_ie = (link_id_ie_t*)bcm_parse_tlvs(pdata, plen, DOT11_MNG_LINK_IDENTIFIER_ID);

	status_code = wlc_tdls_validate_linkId_ie(tdls, parent, pdata, plen, f, FALSE);
	if (status_code)
		goto send;

	err = wlc_tdls_setup_validate_parent(tdls, parent);
	if (err < 0)
		return err;
	else if (err > 0) {
		status_code = (uint8)err;
		goto send;
	}

	FOREACHSCB(wlc->scbstate, &scbiter, scb_i) {
		if (!BSS_TDLS_ENAB(wlc, scb_i->bsscfg))
			continue;
		scb_tdls = SCB_TDLS(tdls, scb_i);
		if (scb_tdls) {
			if (scb_tdls->parent == parent) {
				if ((scb_i->bsscfg->tdls->initiator) &&
					(scb_tdls->flags & TDLS_SCB_SENT_SETUP_REQ)) {
					WL_TDLS(("wl%d:%s(): outstanding setup request exists.\n",
						tdls->pub->unit, __FUNCTION__));
					if (memcmp(f->sa, &scb_i->bsscfg->cur_etheraddr,
						ETHER_ADDR_LEN) > 0) {
						WL_TDLS(("wl%d:%s(): setup REQ MAC is higher, "
							"no resp.\n",
							tdls->pub->unit, __FUNCTION__));
						return BCME_ERROR;
					}
					if (memcmp(f->sa, &scb_i->bsscfg->cur_etheraddr,
						ETHER_ADDR_LEN) < 0) {
						WL_TDLS(("wl%d:%s():setup REQ MAC is lower, "
							"terminate current setup req\n",
							tdls->pub->unit, __FUNCTION__));
						wlc_tdls_disconnect(tdls, scb_i, FALSE);
						break;
					}
				}
				if (!memcmp(f->sa, &scb_i->ea, ETHER_ADDR_LEN)) {
					if (scb_tdls->flags & TDLS_SCB_CONNECTED) {
						WL_TDLS(("wl%d:%s(): TDLS exists, terminate "
							"current TDLS session.\n",
							tdls->pub->unit, __FUNCTION__));
						wlc_tdls_disconnect(tdls, scb_i, FALSE);
						break;
					}
				}
			}
		}
	}

	if ((bi = MALLOCZ(tdls->osh, sizeof(wlc_bss_info_t))) == NULL)
		return BCME_ERROR;
	/* Fill in bss with new info */
	/* init bi->chanspec */
	bi->oper_mode = 0xFF;
	bi->chanspec = parent->current_bss->chanspec;
	if (wlc_tdls_parse_setup_ies(tdls, &parent->BSSID, pdata, plen, bi, TDLS_SETUP_REQ) !=
		BCME_OK) {
		WL_TDLS(("wl%d:%s():TDLS_SETUP_REQ parse error.\n", tdls->pub->unit,
			__FUNCTION__));
		status_code = DOT11_SC_INVALID_PARAMS;
		goto send;
	}
	if (!(bi->ext_cap_flags & TDLS_CAP_TDLS)) {
		WL_TDLS(("wl%d:%s():TDLS support is not set in extcap IE.\n", tdls->pub->unit,
			__FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto send;
	}

	if (bi->ext_cap_flags & TDLS_CAP_CH_SW) {
		if (!bcm_parse_tlvs(pdata, plen, DOT11_MNG_REGCLASS_ID) ||
			!bcm_parse_tlvs(pdata, plen, DOT11_MNG_SUPP_CHANNELS_ID)) {
			WL_TDLS(("%s():wl%d: Missing Supported Regulatory Classes or "
				"Supported Channels IE.\n",
				__FUNCTION__, tdls->pub->unit));
			status_code = DOT11_SC_DECLINED;
			goto send;
		}
	}

	status_code = wlc_tdls_validate_wsec(tdls, parent, NULL, bi, HANDSHAKE_MSG_1);
	if (status_code)
		goto send;

	scb_i = wlc_tdls_scb_create(tdls, parent, f->sa, &parent->BSSID, FALSE);
	if (scb_i == NULL) {
		WL_ERROR(("wl%d:%s(): cannot create scb for the peer STA!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto send;
	}
	bsscfg = SCB_BSSCFG(scb_i);

	wlc_process_extcap_ie(tdls->wlc, pdata, plen, scb_i);
	wlc_tdls_parse_rclist(tdls, scb_i, pdata, plen);

	scb_tdls = SCB_TDLS(tdls, scb_i);
	scb_tdls->con_token = token;

	scb_i->cap = cap;

	wlc_tdls_build_ext_cap_ie(tdls, bsscfg, NULL, NULL);
	scb_i->bsscfg->oper_mode = wlc_tdls_create_oper_mode(wlc, scb, scb->bsscfg);
	scb_i->bsscfg->oper_mode_enabled = TRUE;

	WL_TDLS(("wl%d:%s(): op-mode-recv=%x\n", tdls->pub->unit, __FUNCTION__, bi->oper_mode));
	if (bi->oper_mode != 0xFF)
		wlc_vht_update_scb_oper_mode(wlc->vhti, scb_i, bi->oper_mode);

	if (WSEC_AES_ENABLED(bsscfg->wsec)) {
		/* copy the SNonce from initiator */
		bcopy(bi->snonce, scb_tdls->snonce, EAPOL_WPA_KEY_NONCE_LEN);
		wlc_tdls_cal_tpk(tdls, scb_i, bi);
		WL_TDLS(("wl%d:%s():calculate TPK.\n", tdls->pub->unit,
			__FUNCTION__));
		TDLS_PRHEX("TPK", scb_tdls->tpk, TDLS_TPK_LEN);
		memcpy(&scb_tdls->rsn, &bi->wpa2, sizeof(rsn_parms_t));
		scb_tdls->timeout_interval = bi->ti_val;
		if (bi->wpa2.flags & RSN_FLAGS_PMKID_COUNT_PRESENT) {
			WL_TDLS(("wl%d:%s():set scb_tdls->pmkid_cnt.\n", tdls->pub->unit,
			__FUNCTION__));
			scb_tdls->pmkid_cnt = TRUE;
		}
		scb_tdls->rsn.cap[0] = bi->wpa2.cap[0];
		WL_TDLS(("wl%d:%s():wpa2 PAD[0] = 0x%02x\n", tdls->pub->unit,
			__FUNCTION__, bi->wpa2.cap[0]));
	}

	if (wlc_tdls_join(tdls, bi, scb_i, pdata, plen) != BCME_OK) {
		status_code = DOT11_SC_DECLINED;
		goto send;
	}

	wlc_txmod_config(wlc, scb_i, TXMOD_TDLS);

send:
	wlc_tdls_send_setup_resp(tdls, parent, f->sa, status_code, token, linkId_ie);

	if (bi)
		MFREE(tdls->osh, bi, sizeof(wlc_bss_info_t));
	return status_code;

}

static int
wlc_tdls_process_setup_resp(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint8 token;
	uint16 cap;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	uint16 status_code = 0;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);
	wlc_bsscfg_t *bsscfg;
	void *p;
	wlc_bss_info_t *bi = NULL;

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	WLCNTINCR(tdls->cnt->rxdsetupresp);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	if (!(scb_tdls->flags & TDLS_SCB_SENT_SETUP_REQ)) {
		/* This resp is for previous sent SETUP Req, if we already cleaned up our state
		 * we don't need to send confirm with status DECLINED, otherwise it may mess
		 * up state for current ongoing handshake
		 */
		return DOT11_SC_DECLINED;
	}

	/* offset starts at status code field */
	status_code = ltoh16(*(uint16*)(pdata));
	if (status_code) {
		WL_TDLS(("wl%d:%s(): SETUP resp status=%d, terminate the setup process!\n",
			tdls->pub->unit, __FUNCTION__, status_code));
		wlc_tdls_disconnect(tdls, scb_peer, TRUE);
		return status_code;
	}

	pdata += 2;
	plen -= 2;

	token = *pdata++;
	plen--;
	if (token != scb_tdls->con_token) {
		WL_ERROR(("wl%d:%s(): Mismatched token: sent req token=%d, "
			"recv response token=%d!\n",
			tdls->pub->unit, __FUNCTION__, scb_tdls->con_token, token));
		if (token < scb_tdls->con_token) {
			/* This could be response for previous sent Setup Req for which
			 * cleanup is done already, just discard it, Sending a confirm with
			 * status DECLINED will mess up current ongoing handshake
			 * Fixme: Ideal way is to check the token for previous setup req,
			 * but there is no way we can have it or store it
			 */
			return DOT11_SC_DECLINED;
		}
		status_code = DOT11_SC_DECLINED;
		goto send;
	}

#ifdef TDLS_TESTBED
	if (tdls->test_status_code) {
		status_code = ltoh16(tdls->test_status_code);
		goto send;
	}
#endif // endif

	/* capability */
	cap = ntoh16(*(uint16*)pdata);
	pdata += 2;
	plen -= 2;

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, parent, pdata, plen, f, TRUE);
	if (status_code)
		goto send;

	bsscfg = SCB_BSSCFG(scb_peer);
	if (!bsscfg || !BSSCFG_IS_TDLS(bsscfg)) {
		WL_ERROR(("wl%d:%s(): No bsscfg for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto send;
	}

	if (scb_tdls->parent != parent) {
		WL_ERROR(("wl%d:%s(): TDLS responder not in the same BSS!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code =  DOT11_SC_DECLINED;
		goto send;
	}
	wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
	scb_tdls->timer_start = FALSE;

	if ((bi = MALLOCZ(tdls->osh, sizeof(wlc_bss_info_t))) == NULL) {
		return DOT11_SC_DECLINED;
	}
	bi->oper_mode = 0xFF;
	/* Fill in bss with new info */
	/* init bi->chanspec */
	bi->chanspec = scb_peer->bsscfg->current_bss->chanspec;
	if (wlc_tdls_parse_setup_ies(tdls, &parent->BSSID, pdata, plen, bi, TDLS_SETUP_RESP) !=
		BCME_OK) {
		WL_TDLS(("wl%d:%s():TDLS_SETUP_RESP parse error.\n", tdls->pub->unit,
			__FUNCTION__));
		status_code = DOT11_SC_INVALID_PARAMS;
		goto send;
	}
	scb_peer->cap = cap;

	wlc_process_extcap_ie(tdls->wlc, pdata, plen, scb_peer);
	wlc_tdls_parse_rclist(tdls, scb_peer, pdata, plen);

	wlc_tdls_qosinfo_update(tdls, scb_peer, bi);

	WL_TDLS(("wl%d:%s(): op-mode-recv=%x\n", tdls->pub->unit, __FUNCTION__, bi->oper_mode));
	if (VHT_ENAB(wlc->pub) || N_ENAB(wlc->pub)) {
		if (bi->oper_mode) {
			wlc_vht_set_scb_opermode_enab(wlc->vhti, scb, TRUE);
			wlc_vht_update_scb_oper_mode(wlc->vhti, scb, bi->oper_mode);
		}
	}

	if (WSEC_AES_ENABLED(bsscfg->wsec)) {
		/* copy the ANonce from responder */
		bcopy(bi->anonce, scb_tdls->anonce, EAPOL_WPA_KEY_NONCE_LEN);

		scb_tdls->timeout_interval = bi->ti_val;
		status_code = wlc_tdls_validate_wsec(tdls, parent, scb_peer, bi, HANDSHAKE_MSG_2);
		if (status_code)
			goto send;
		wlc_tdls_cal_tpk(tdls, scb_peer, bi);
		WL_TDLS(("wl%d:%s():calculate TPK.\n", tdls->pub->unit,
			__FUNCTION__));
		TDLS_PRHEX("TPK", scb_tdls->tpk, TDLS_TPK_LEN);
		if (!wlc_tdls_cal_mic_chk(tdls, scb_peer, pdata, plen, 2)) {
			status_code = DOT11_SC_DECLINED;
			goto send;
		}
		memcpy(&scb_tdls->rsn, &bi->wpa2, sizeof(rsn_parms_t));
		if (((bi->wpa2.flags & RSN_FLAGS_PMKID_COUNT_PRESENT) && !scb_tdls->pmkid_cnt) ||
		(!(bi->wpa2.flags & RSN_FLAGS_PMKID_COUNT_PRESENT) && scb_tdls->pmkid_cnt)) {
			WL_TDLS(("wl%d:%s():RSN IE pmkid_cnt length not match.\n", tdls->pub->unit,
				__FUNCTION__));
			status_code = DOT11_SC_DECLINED;
			goto send;
		}
		scb_tdls->rsn.cap[0] = bi->wpa2.cap[0];
		WL_TDLS(("wl%d:%s():wpa2 PAD[0] = 0x%02x\n", tdls->pub->unit,
			__FUNCTION__, bi->wpa2.cap[0]));
	}

	if (wlc_tdls_join(tdls, bi, scb_peer, pdata, plen) != BCME_OK) {
		status_code = DOT11_SC_DECLINED;
		goto send;
	}

	/* install keys */
	wlc_tdls_wpa_plumb_tk(tdls, bsscfg, &scb_tdls->tpk[TDLS_TPK_KCK_LEN], TDLS_TPK_TK_LEN,
		CRYPTO_ALGO_AES_CCM, &scb_peer->ea);

send:

	WL_TDLS(("%d%s(): send SETUP confirm: status=%d, token=%d\n",
		tdls->pub->unit, __FUNCTION__, status_code, token));

	wlc_tdls_send_setup_cfm(tdls, parent, f->sa, status_code, token);

	wlc_txmod_unconfig(wlc, scb_peer, TXMOD_TDLS);

	/* release all the packets from the buffered queues */
	while ((p = pktq_deq(&scb_tdls->ubufq, NULL))) {
		wlc_tdls_reinsert_pkt(wlc, parent, scb_peer, p);
		WLCNTINCR(tdls->cnt->buf_reinserted);
	}

	if (status_code) {
		wlc_tdls_lookaside_status_upd(tdls, scb_tdls->peer_addr,
			TDLS_LOOKASIDE_DECLINED_IDLE);
		wlc_tdls_disconnect(tdls, scb_peer, FALSE);
	}

	if (bi)
		MFREE(tdls->osh, bi, sizeof(wlc_bss_info_t));
	return status_code;

}

static int
wlc_tdls_process_setup_cfm(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint8 token;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	uint16 status_code = 0;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);
	wlc_bsscfg_t *bsscfg;
	void *p;
	wlc_bss_info_t *bi = NULL;
	uint16 bw, bw_peer, common_bw;
	chanspec_t cspec;
	uint32 idx = 0;

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS Initiator!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	WLCNTINCR(tdls->cnt->rxsetupcfm);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at status code field */
	status_code = ltoh16(*(uint16*)(pdata));
	if (status_code) {
		WL_TDLS(("wl%d:%s(): SETUP confrim status=%d, terminate the setup process!\n",
			tdls->pub->unit, __FUNCTION__, status_code));
		wlc_tdls_disconnect(tdls, scb_peer, FALSE);
		return status_code;
	}
	pdata += 2;
	plen -= 2;

	token = *pdata++;
	plen--;
	if (token != scb_tdls->con_token) {
		WL_ERROR(("wl%d:%s(): Mismatched token: sent response token=%d, "
			"recv confirm token=%d!\n",
			tdls->pub->unit, __FUNCTION__, scb_tdls->con_token, token));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, parent, pdata, plen, f, FALSE);
	if (status_code)
		goto exit;

	bsscfg = SCB_BSSCFG(scb_peer);
	if (!BSSCFG_IS_TDLS(bsscfg)) {
		WL_ERROR(("wl%d:%s(): No bsscfg for TDLS Initiator!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	if (scb_tdls->parent != parent) {
		WL_ERROR(("wl%d:%s(): TDLS Initiator not in the same BSS!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code =  DOT11_SC_DECLINED;
		goto exit;
	}

	if (scb_tdls->flags != TDLS_SCB_SENT_SETUP_RESP) {
		WL_TDLS(("wl%d:%s(): TDLS Initiator is in wrong state!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
	scb_tdls->timer_start = FALSE;

	if ((bi = MALLOCZ(tdls->osh, sizeof(wlc_bss_info_t))) == NULL) {
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}
	bi->oper_mode = 0xFF;
	/* Fill in bss with new info */
	/* init bi->chanspec */
	bi->chanspec = scb_peer->bsscfg->current_bss->chanspec;
	if (wlc_tdls_parse_setup_ies(tdls, &parent->BSSID, pdata, plen, bi, TDLS_SETUP_CONFIRM) !=
		BCME_OK) {
		WL_TDLS(("wl%d:%s():TDLS_SETUP_CONFIRM parse error.\n", tdls->pub->unit,
			__FUNCTION__));
		status_code = DOT11_SC_INVALID_PARAMS;
		goto exit;
	}
	wlc_tdls_qosinfo_update(tdls, scb_peer, bi);

	WL_TDLS(("wl%d:%s(): op-mode-recv=%x\n", tdls->pub->unit, __FUNCTION__, bi->oper_mode));
	if (VHT_ENAB(wlc->pub) || N_ENAB(wlc->pub)) {
		if (bi->oper_mode) {
			wlc_vht_set_scb_opermode_enab(wlc->vhti, scb, TRUE);
			wlc_vht_update_scb_oper_mode(wlc->vhti, scb, bi->oper_mode);
		}
	}

	if ((bi->chanspec && bi->chanspec != INVCHANSPEC &&
		bi->chanspec != scb_peer->bsscfg->current_bss->chanspec) ||
		(bi->oper_mode != 0xFF)) {

		/* Received invalid chanspec tear down TDLS link */
		if (wf_chspec_ctlchan(bi->chanspec) !=
			wf_chspec_ctlchan(scb_peer->bsscfg->current_bss->chanspec)) {
			wlc_tdls_disconnect(tdls, scb, TRUE);
			if (bi) {
				MFREE(tdls->osh, bi, sizeof(wlc_bss_info_t));
			}
			return DOT11_SC_FAILURE;
		}

		if (bi->oper_mode != 0xFF) {
			/* Get local BW */
			idx = DOT11_OPER_MODE_CHANNEL_WIDTH(scb_peer->bsscfg->oper_mode);
			bw = opermode_bw_chspec_map[idx];
			/* Get tdls peer BW */
			idx = DOT11_OPER_MODE_CHANNEL_WIDTH(bi->oper_mode);
			bw_peer = opermode_bw_chspec_map[idx];
			common_bw = MIN(bw, bw_peer);
			cspec = wf_chspec_ctlchan(parent->current_bss->chanspec);
			cspec = wf_channel2chspec(cspec, common_bw);
			bi->chanspec = cspec;
		}
		if (bi->chanspec != scb_peer->bsscfg->current_bss->chanspec) {
			wlc_update_bandwidth(wlc, scb_peer->bsscfg, bi->chanspec);
			scb_tdls->base_chanspec = scb_tdls->cur_chanspec =
				scb_tdls->target_chanspec = bi->chanspec;
		}
	}

	if (WSEC_AES_ENABLED(bsscfg->wsec)) {
		status_code = wlc_tdls_validate_wsec(tdls, parent, scb_peer, bi, HANDSHAKE_MSG_3);
		if (status_code)
			goto exit;

		if (!wlc_tdls_cal_mic_chk(tdls, scb_peer, pdata, plen, 3)) {
			WL_TDLS(("wl%d:%s(): MIC check error!\n",
				tdls->pub->unit, __FUNCTION__));
			status_code = DOT11_SC_DECLINED;
			goto exit;
		}
		if (((bi->wpa2.flags & RSN_FLAGS_PMKID_COUNT_PRESENT) && !scb_tdls->pmkid_cnt) ||
		(!(bi->wpa2.flags & RSN_FLAGS_PMKID_COUNT_PRESENT) && scb_tdls->pmkid_cnt)) {
			WL_TDLS(("wl%d:%s():RSN IE pmkid_cnt length not match.\n", tdls->pub->unit,
				__FUNCTION__));
			goto exit;
		}
		scb_tdls->rsn.cap[0] = bi->wpa2.cap[0];
		WL_TDLS(("wl%d:%s():wpa2 PAD[0] = 0x%02x\n", tdls->pub->unit,
			__FUNCTION__, bi->wpa2.cap[0]));

		/* Install keys */
		wlc_tdls_wpa_plumb_tk(tdls, bsscfg, &scb_tdls->tpk[TDLS_TPK_KCK_LEN],
			TDLS_TPK_TK_LEN, CRYPTO_ALGO_AES_CCM, &scb_peer->ea);
	}
exit:
	if (status_code) {
		wlc_tdls_lookaside_status_upd(tdls, scb_tdls->peer_addr,
			TDLS_LOOKASIDE_DECLINED_IDLE);
		wlc_tdls_disconnect(tdls, scb_peer, TRUE);
	}
	else {
		wlc_txmod_unconfig(wlc, scb_peer, TXMOD_TDLS);

		/* release all the packets from the buffered queues */
		while ((p = pktq_deq(&scb_tdls->ubufq, NULL))) {
			wlc_tdls_reinsert_pkt(wlc, parent, scb_peer, p);
			WLCNTINCR(tdls->cnt->buf_reinserted);
		}
		scb_tdls->flags = TDLS_SCB_CONNECTED;

		wlc_tdls_port_open(tdls, &scb_peer->ea);

		wlc_bss_mac_event(wlc, scb_peer->bsscfg, WLC_E_TDLS_PEER_EVENT, &scb_peer->ea,
			WLC_E_STATUS_SUCCESS, WLC_E_TDLS_PEER_CONNECTED, 0, NULL, 0);
		wlc_tdls_pretbtt_update(wlc);
	}

	if (bi)
		MFREE(tdls->osh, bi, sizeof(wlc_bss_info_t));
	return status_code;

}

static int
wlc_tdls_process_teardown(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uint16 reason;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	wlc_bsscfg_t *bsscfg;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);
	wlc_txq_info_t *qi;

	UNUSED_PARAMETER(reason);
	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	WLCNTINCR(tdls->cnt->rxteardown);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at reason code field */
	reason = ltoh16(*(uint16*)pdata);
	pdata += 2;
	plen -= 2;
	WL_TDLS(("wl%d:%s(): reason = %d from %s.\n", tdls->pub->unit, __FUNCTION__,
		reason, (scb == scb_peer) ? "peer" : "AP"));

	bsscfg = SCB_BSSCFG(scb_peer);
	if (!BSSCFG_IS_TDLS(bsscfg)) {
		WL_ERROR(("wl%d:%s(): No bsscfg for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	if ((scb != scb_peer) && (scb_tdls->parent != parent)) {
		WL_ERROR(("wl%d:%s(): TDLS responder not in the same BSS!\n",
			tdls->pub->unit, __FUNCTION__));
	}

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, parent, pdata, plen, f,
		scb_peer->bsscfg->tdls->initiator);
	if (status_code)
		goto exit;

	if (WSEC_AES_ENABLED(parent->wsec) &&
		!wlc_tdls_cal_teardown_mic_chk(tdls, scb_peer, pdata, plen, reason, TRUE))
		status_code = DOT11_SC_DECLINED;

exit:
	if (status_code)
		return status_code;

	scb_tdls->flags &= ~TDLS_SCB_CONNECTED;
	scb_tdls->flags |= TDLS_SCB_RCVD_TEARDOWN;
	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next)
		wlc_pktq_scb_free(wlc, WLC_GET_TXQ(qi), scb_peer);
	wlc_tdls_disconnect(tdls, scb_peer, TRUE);

	return 0;

}

static int
wlc_tdls_process_pti(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	wlc_bsscfg_t *bsscfg;
	uint8 token;
	bcm_tlv_t *ie;

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	WLCNTINCR(tdls->cnt->rxptireq);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at token field */
	token = *pdata++;
	plen--;

	bsscfg = SCB_BSSCFG(scb_peer);
	if (!BSSCFG_IS_TDLS(bsscfg)) {
		WL_ERROR(("wl%d:%s(): No bsscfg for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	if (scb_tdls->pti_token_rx && (scb_tdls->pti_token_rx == token)) {
		WL_ERROR(("wl%d:%s(): same token, already process the PTI req!\n",
			tdls->pub->unit, __FUNCTION__));
	}
	scb_tdls->pti_token_rx = token;
	WL_TDLS(("wl%d:%s(): token = %d\n", tdls->pub->unit, __FUNCTION__, token));

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, scb_tdls->parent, pdata, plen, f,
		scb_peer->bsscfg->tdls->initiator);
	if (status_code)
		goto exit;

	if ((ie = bcm_parse_tlvs(pdata, plen, DOT11_MNG_PTI_CONTROL_ID)) != NULL) {
		WL_TDLS(("wl%d:%s(): PTI control IE: tid=%d, seq=0x%x\n",
			tdls->pub->unit, __FUNCTION__,
			((pti_control_ie_t *)ie)->tid, ((pti_control_ie_t *)ie)->seq_control));
		if (ie->len != TDLS_PTI_CONTROL_IE_LEN) {
			WL_ERROR(("wl%d:%s(): PTI control IE len = %d wrong!\n",
				tdls->pub->unit, __FUNCTION__, ie->len));
			goto exit;
		}
	}
	if ((ie = bcm_parse_tlvs(pdata, plen, DOT11_MNG_PU_BUFFER_STATUS_ID)) != NULL) {
		if (ie->len != TDLS_PU_BUFFER_STATUS_IE_LEN) {
			WL_ERROR(("wl%d:%s(): PU buffer status IE len = %d wrong!\n",
				tdls->pub->unit, __FUNCTION__, ie->len));
			goto exit;
		}

		WL_TDLS(("wl%d:%s(): PU buffer status: 0x%x\n",
			tdls->pub->unit, __FUNCTION__, ((pu_buffer_status_ie_t *)ie)->status));

		wlc_tdls_send_pti_resp(tdls, scb_peer);

	}

exit:
	if (status_code)
		return status_code;

	return 0;

}

static int
wlc_tdls_process_pti_resp(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	wlc_bsscfg_t *bsscfg;
	uint8 token;

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	WLCNTINCR(tdls->cnt->rxptiresp);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at token field */
	token = *pdata++;
	plen--;
	WL_TDLS(("wl%d:%s(): token = %d\n", tdls->pub->unit, __FUNCTION__, token));

	bsscfg = SCB_BSSCFG(scb_peer);
	if (!BSSCFG_IS_TDLS(bsscfg)) {
		WL_ERROR(("wl%d:%s(): No bsscfg for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	if (scb_tdls->pti_token != token) {
		WL_ERROR(("wl%d:%s(): not the same token, ignore PTI resp!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}

#ifdef WLTDLS_SEND_PTI_RETRY
	scb_tdls->flags &= ~TDLS_SCB_PTI_RESP_TIMED_OUT;
#endif /* WLTDLS_SEND_PTI_RETRY */

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, scb_tdls->parent, pdata, plen, f,
		scb_peer->bsscfg->tdls->initiator);
	if (status_code)
		return status_code;

	scb_tdls->flags &= ~ TDLS_SCB_SENT_PTI;
	wlc_tdls_wake_ctrl(wlc, tdls, scb);

		if (!(scb_tdls->flags & TDLS_SCB_PM_SET)) {
			wlc_apps_apsd_trigger(wlc, scb_peer,
				wlc_apps_apsd_ac_available(wlc, scb_peer));
			return 0;
		}

		scb_tdls->flags &= ~TDLS_SCB_PM_SET;

		/* send NULL-data pkt with PM bit set */
		WL_TDLS(("wl%d.%d:%s(): send null-data to peer buffer STA.\n",
			tdls->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		if (!wlc_sendnulldata(wlc, bsscfg, f->sa, 0, 0, PRIO_8021D_BE, NULL, NULL))
		{
			WL_ERROR(("wl%d.%d: failed to send PM null frame, "
				"fake a PM0->PM1 transition\n",
				tdls->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			bsscfg->tdls->tdls_PMEnable = !bsscfg->tdls->tdls_PMEnable;

			bsscfg->pm->PMenabled = !bsscfg->pm->PMenabled;
			bsscfg->pm->PMpending = FALSE;

			wlc_tdls_pm_allowed_update(tdls, scb_tdls->parent);
			wlc_set_ps_ctrl(scb_tdls->parent);
			wlc_tdls_wake_ctrl(wlc, tdls, scb);
		}

	return 0;
}

/* Supported Rates IE in Discovery frames */
static uint
wlc_tdls_disc_calc_sup_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	ASSERT(ftcbparm->disc.sup != NULL);

	return TLV_HDR_LEN + ftcbparm->disc.sup->count;
}

static int
wlc_tdls_disc_write_sup_rates_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	ASSERT(ftcbparm->disc.sup != NULL);

	bcm_write_tlv_safe(DOT11_MNG_RATES_ID, ftcbparm->disc.sup->rates,
		ftcbparm->disc.sup->count, build->buf, build->buf_len);

	return BCME_OK;
}

/* Ext Supported Rates IE in Discovery frames */
static uint
wlc_tdls_disc_calc_ext_rates_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	if (ftcbparm->disc.ext == NULL || ftcbparm->disc.ext->count == 0)
		return 0;

	return TLV_HDR_LEN + ftcbparm->disc.ext->count;
}

static int
wlc_tdls_disc_write_ext_rates_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	if (ftcbparm->disc.ext == NULL || ftcbparm->disc.ext->count == 0) {
		return BCME_OK;
	}

	bcm_write_tlv_safe(DOT11_MNG_EXT_RATES_ID, ftcbparm->disc.ext->rates,
		ftcbparm->disc.ext->count, build->buf, build->buf_len);

	return BCME_OK;
}

/* Ext Cap IE in Discovery frames */
static uint
wlc_tdls_disc_calc_ext_cap_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	return TLV_HDR_LEN + ftcbparm->disc.ext_cap_len;
}

static int
wlc_tdls_disc_write_ext_cap_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	bcm_write_tlv_safe(DOT11_MNG_EXT_CAP_ID, ftcbparm->disc.cap,
		ftcbparm->disc.ext_cap_len, build->buf, build->buf_len);

	return BCME_OK;
}

/* RSN IE in Discovery frames */
static uint
wlc_tdls_disc_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_bsscfg_t *cfg = calc->cfg;
	uint8 buf[257];

	if (WSEC_AES_ENABLED(cfg->wsec) == 0)
		return 0;

	/* TODO: find another way to calculate the IE length */

	return (uint)(wlc_tdls_write_rsn_ie(tdls, buf, sizeof(buf), WPA2_AUTH_TPK, AES_ENABLED,
	                                    NULL, FALSE) - buf);
}

static int
wlc_tdls_disc_write_rsn_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_bsscfg_t *cfg = build->cfg;

	if (WSEC_AES_ENABLED(cfg->wsec) == 0)
		return 0;

	wlc_tdls_write_rsn_ie(tdls, build->buf, build->buf_len, WPA2_AUTH_TPK, AES_ENABLED,
	                      NULL, FALSE);

	return BCME_OK;
}

/* FT TI IE in Discovery frames */
static uint
wlc_tdls_disc_calc_ft_ti_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_bsscfg_t *cfg = calc->cfg;

	if (WSEC_AES_ENABLED(cfg->wsec) == 0)
		return 0;

	return TLV_HDR_LEN + sizeof(ti_ie_t);
}

static int
wlc_tdls_disc_write_ft_ti_ie(void *ctx, wlc_iem_build_data_t *build)
{
	tdls_info_t *tdls = (tdls_info_t *)ctx;
	wlc_bsscfg_t *cfg = build->cfg;
	ti_ie_t ti_ie;

	if (WSEC_AES_ENABLED(cfg->wsec) == 0)
		return BCME_OK;

	ti_ie.ti_type = TI_TYPE_KEY_LIFETIME;
	ti_ie.ti_val = tdls->sa_lifetime;

	bcm_write_tlv_safe(DOT11_MNG_FT_TI_ID, &ti_ie,
		sizeof(ti_ie_t), build->buf, build->buf_len);

	return BCME_OK;
}

/* LinkID IE in Discovery frames */
static uint
wlc_tdls_disc_calc_link_id_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	return TLV_HDR_LEN + TDLS_LINK_ID_IE_LEN;
}

static int
wlc_tdls_disc_write_link_id_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	bcopy(ftcbparm->disc.linkid_ie, build->buf, sizeof(link_id_ie_t));

	return BCME_OK;
}

static uint16
wlc_tdls_process_discovery_req(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uint8 token;
	uint8 *pdata;
	int pdata_len;
	uint8 *data;
	uint data_len;
	link_id_ie_t *linkId_ie;
	void *p;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);
	wlc_rateset_t sup_rates, ext_rates;
	uint16 cap;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	uint8	ext_cap[DOT11_EXTCAP_LEN_MAX];

	pdata_len = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at reason code field */
	token = *pdata++;
	pdata_len--;

	/* starts TLV IEs */
	linkId_ie = (link_id_ie_t*)bcm_parse_tlvs(pdata, pdata_len, DOT11_MNG_LINK_IDENTIFIER_ID);
	if (!linkId_ie) {
		WL_ERROR(("wl%d:%s(): no Link Identifier IE in Discovery Request!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	if (linkId_ie->len != TDLS_LINK_ID_IE_LEN) {
		WL_ERROR(("wl%d:%s(): Link Identifier IE len = %d wrong!\n",
			tdls->pub->unit, __FUNCTION__, linkId_ie->len));
		return DOT11_SC_DECLINED;
	}

	if (memcmp(&linkId_ie->bssid, &parent->BSSID, ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): BSSID mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		return status_code;
	}
	if (!tdls->up)
		return DOT11_SC_DECLINED;

	/* Findout the packet length to be allocated using wlc_ier_calc_len. */
	/* supported rates */
	bzero(&sup_rates, sizeof(wlc_rateset_t));
	bzero(&ext_rates, sizeof(wlc_rateset_t));
	wlc_rateset_elts(wlc, parent, &parent->target_bss->rateset, &sup_rates, &ext_rates);

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.disc.sup = &sup_rates;
	ftcbparm.disc.ext = &ext_rates;
	ftcbparm.disc.cap = ext_cap;
	ftcbparm.disc.linkid_ie = (uint8 *)linkId_ie;
	wlc_tdls_build_ext_cap_ie(tdls, parent, ftcbparm.disc.cap, &ftcbparm.disc.ext_cap_len);
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;
	cbparm.ht = BSS_N_ENAB(wlc, parent);
	cbparm.vht = VHT_ENAB(wlc->pub);

	/* calc IEs' length */
	data_len = wlc_ier_calc_len(wlc->ier_tdls_drs, parent, FC_ACTION, &cbparm);

	if ((p = wlc_frame_get_action(wlc, FC_ACTION, f->sa, &parent->cur_etheraddr, &parent->BSSID,
		data_len + TDLS_ACTION_FRAME_DISCOVERY_RESP_BASELEN, &data,
		DOT11_ACTION_CAT_PUBLIC)) == NULL) {
		return BCME_NOMEM;
	}

	WL_TDLS(("wl%d:%s(): Compose Discovery Response...\n", tdls->pub->unit, __FUNCTION__));

	/* 1: TDLS category */
	*data++ = DOT11_ACTION_CAT_PUBLIC;

	/* 2: TDLS action field */
	*data++ = TDLS_DISCOVERY_RESP;

	/* 3: token */
	*data++ = token;
	WL_TDLS(("token: %d\n", token));

	/* capability */
	cap = wlc_assoc_capability(wlc, parent, parent->current_bss);
	WL_TDLS(("Capability: 0x%04x\n", cap));
	*(uint16 *)data = cap;
	data += sizeof(uint16);

	/* build IEs */
	if (wlc_ier_build_frame(wlc->ier_tdls_drs, parent, FC_ACTION, &cbparm,
	                        data, data_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, p, TRUE);
		return DOT11_SC_FAILURE;
	}

	PKTSETLEN(tdls->osh, p, data_len + (uint)(data - PKTDATA(tdls->osh, p)));

	TDLS_PRHEX("discovery_resp:", data, data_len);
	TDLS_PRHEX("data:", PKTDATA(tdls->osh, p), PKTLEN(tdls->osh, p));

	wlc_sendmgmt(wlc, p, parent->wlcif->qi, scb);

	return 0;

}

static int wlc_tdls_process_chsw_req(tdls_info_t *tdls, struct scb *scb,
	struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uchar *pdata;
	int plen;
	struct scb *parent_scb = NULL;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	int ret_code = 0;
	bcm_tlv_t *ie;
	channel_switch_timing_ie_t *chsw_timing_ie;
	tdls_chsw_req_t chsw_req;
	tdls_chsw_resp_t chsw_resp;
	chanspec_t target_chanspec;

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if ((NBANDS(wlc) > 1) && (scb_peer == NULL))
		scb_peer = wlc_tdls_scbfindband_all(wlc, f->sa, (int)OTHERBANDUNIT(wlc));
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	memset(&chsw_req, 0, sizeof(tdls_chsw_req_t));

#ifdef TDLS_TESTBED
	if (tdls->chsw_mode == TDLS_CHSW_SEND_REQ)
		return DOT11_SC_DECLINED;
	if (tdls->chsw_mode == TDLS_CHSW_REJREQ)
		status_code = DOT11_SC_DECLINED;
	else if (tdls->test_status_code)
		status_code = tdls->test_status_code;
	if (status_code)
		goto exit;
#endif // endif

	WLCNTINCR(tdls->cnt->rxchswreq);

	parent_scb = wlc_tdls_scbfindband_all(wlc, &scb_peer->bsscfg->BSSID,
		CHSPEC_WLCBANDUNIT(scb_tdls->base_chanspec));
	if (!parent_scb) {
		WL_TDLS(("wl%d:%s(): parent SCB not found!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	if (parent_scb->flags2 & SCB2_TDLS_CHSW_PROHIBIT) {
		WL_TDLS(("wl%d:%s(): TDLS Channel Switch prohibited at AP!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	if (scb_tdls->flags & TDLS_SCB_RECV_CHSW_REQ) {
		WL_TDLS(("wl%d:%s(): outstanding channel switch request existed!\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	chsw_req.target_ch = *pdata++;
	plen--;
	chsw_req.regclass = *pdata++;
	plen--;

	/* starts TLV IEs */
	status_code = wlc_tdls_validate_linkId_ie(tdls, scb_tdls->parent, pdata, plen, f,
		scb_peer->bsscfg->tdls->initiator);
	if (status_code) {
		WL_ERROR(("wl%d:%s(): wlc_tdls_validate_linkId_ie) failed. status_code = %d\n",
			tdls->pub->unit, __FUNCTION__, status_code));
		goto exit;
	}

	/* Secondary Channel Offset */
	if ((ie = bcm_parse_tlvs(pdata, plen, DOT11_MNG_EXT_CHANNEL_OFFSET)) != NULL) {
		if (ie->len != DOT11_EXTCH_IE_LEN) {
			WL_ERROR(("wl%d:%s(): wrong ext channel offset IE len = %d\n",
				tdls->pub->unit, __FUNCTION__, ie->len));

			status_code = DOT11_SC_DECLINED;
			goto exit;
		}

		chsw_req.second_ch_offset = ie->data[0];
	}
	else
		chsw_req.second_ch_offset = 0;

	target_chanspec = wlc_tdls_target_chanspec(chsw_req.target_ch,
		chsw_req.second_ch_offset);
	if ((ret_code = wlc_tdls_validate_target_chanspec(tdls, scb_peer, target_chanspec)) != 0) {
		WL_ERROR(("wl%d:%s(): Target channel validation failed, ret_code = %d\n",
			tdls->pub->unit, __FUNCTION__, ret_code));

		status_code = DOT11_SC_DECLINED;
		goto exit;
	}
	WL_TDLS(("wl%d:%s(): target_ch=%d, second_ch_offset=%d, target_chanspec=0x%04x\n",
		tdls->pub->unit, __FUNCTION__, chsw_req.target_ch, chsw_req.second_ch_offset,
		target_chanspec));

	/* Channel Switch Timing */
	ie = bcm_parse_tlvs(pdata, plen, DOT11_MNG_CHANNEL_SWITCH_TIMING_ID);
	if (!ie) {
		WL_ERROR(("wl%d:%s(): Missing Channel Switch Timing IE.\n",
			tdls->pub->unit, __FUNCTION__));
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}
	if (ie->len != TDLS_CHANNEL_SWITCH_TIMING_IE_LEN) {
		WL_ERROR(("wl%d:%s(): wrong channel switch timing IE len = %d\n",
			tdls->pub->unit, __FUNCTION__, ie->len));

		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	chsw_timing_ie = (channel_switch_timing_ie_t *)ie;
	chsw_req.switch_time = ltoh16(chsw_timing_ie->switch_time);
	chsw_req.switch_timeout = ltoh16(chsw_timing_ie->switch_timeout);

	/* put in PS mode with AP, if not already be so */
	if (wlc_tdls_chsw_validate_pmstate(tdls, scb_peer, scb_tdls->parent)) {
		status_code = DOT11_SC_DECLINED;
		goto exit;
	}

	scb_tdls->flags |= TDLS_SCB_RECV_CHSW_REQ;
	scb_tdls->target_chanspec = target_chanspec;

exit:
	chsw_resp.switch_time = (tdls->switch_time >= chsw_req.switch_time) ?
		tdls->switch_time : chsw_req.switch_time;
	chsw_resp.switch_timeout = (tdls->switch_timeout >= chsw_req.switch_timeout) ?
		tdls->switch_timeout : chsw_req.switch_timeout;

	/* save the max switch_time/timout from the Channel Switch Resp */
	if (!status_code) {
		scb_tdls->switch_time = chsw_resp.switch_time;
		scb_tdls->switch_timeout = chsw_resp.switch_timeout;
	}

	if (scb_tdls->switch_time != tdls->switch_time) {
		wlc_tdls_pretbtt_update(wlc);
	}

	wlc_tdls_send_chsw_resp(tdls, scb_peer, status_code, &chsw_resp, TRUE);

	return 0;
}

static int wlc_tdls_process_chsw_resp(tdls_info_t *tdls, struct scb *scb,
	struct wlc_frminfo *f, uint offset)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 status_code = 0;
	uchar *pdata;
	int plen;
	struct scb *scb_peer = NULL;
	scb_tdls_t *scb_tdls;
	bcm_tlv_t *ie;
	channel_switch_timing_ie_t *chsw_timing_ie;
	bool off_ch_req = TRUE;
	chanspec_t target_chanspec;

#ifdef TDLS_TESTBED
	if (tdls->chsw_mode == TDLS_CHSW_SEND_REQ)
		return DOT11_SC_DECLINED;
#endif // endif

	scb_peer = wlc_tdls_scbfind_all(wlc, f->sa);
	if ((NBANDS(wlc) > 1) && (scb_peer == NULL))
		scb_peer = wlc_tdls_scbfindband_all(wlc, f->sa, (int)OTHERBANDUNIT(wlc));
	if (scb_peer == NULL) {
		WL_ERROR(("wl%d:%s(): No scb for TDLS responder!\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	scb_tdls = SCB_TDLS(tdls, scb_peer);
	ASSERT(scb_tdls);

	WLCNTINCR(tdls->cnt->rxchswresp);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	status_code = ltoh16(*(uint16*)pdata);
	plen += 2;

	/* starts TLV IEs */
	if (wlc_tdls_validate_linkId_ie(tdls, scb_tdls->parent, pdata, plen, f,
		scb_peer->bsscfg->tdls->initiator))
		return DOT11_SC_DECLINED;

	if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_REQ) {
		WL_TDLS(("wl%d:%s(): recv channel switch resp.\n",
			tdls->pub->unit, __FUNCTION__));
		scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_REQ;
		wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
		scb_tdls->timer_start = FALSE;
	}
	else {
		/* could be a return to base channel */
		WL_TDLS(("wl%d:%s(): Return to base channel request.\n",
			tdls->pub->unit, __FUNCTION__));
		if (scb_tdls->cur_chanspec == scb_tdls->base_chanspec) {
			WL_TDLS(("wl%d:%s(): already on base channel.\n",
			tdls->pub->unit, __FUNCTION__));
			return DOT11_SC_DECLINED;
		}
		off_ch_req = FALSE;
		if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_RESP) {
			WL_TDLS(("wl%d:%s(): Cancel waiting for Channel Switch Response ACK."
				" flags = 0x%08x\n",
				tdls->pub->unit, __FUNCTION__, scb_tdls->flags));
			scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_RESP;
			/* Need to dequeue the packet from txq */
		}
		if (SCB_BSSCFG(scb_peer)->tdls->chsw_req_enabled) {
			wlc_mchan_start_tdls_timer(wlc->mchan, scb_tdls->parent, scb_peer, FALSE);
		}
	}

	if (status_code) {
		WL_ERROR(("wl%d:%s(): Channel Switch Request declined, status_code = %d!\n",
			tdls->pub->unit, __FUNCTION__, status_code));
		/* prevent any further CHSW Request until explicitly requested */
		SCB_BSSCFG(scb_peer)->tdls->chsw_req_enabled = FALSE;
#ifdef WLMCHAN
		wlc_mchan_stop_tdls_timer(wlc->mchan);
#endif // endif
		if (scb_tdls->target_chanspec != scb_tdls->base_chanspec) {
			wlc_tdls_delete_chan_ctx(tdls, scb_peer, scb_tdls->target_chanspec);
			scb_tdls->target_chanspec = scb_tdls->base_chanspec;
		}
		if (scb_tdls->flags & TDLS_SCB_SENT_CHSW_RESP) {
			WL_TDLS(("wl%d:%s(): Cancel waiting for Channel Switch Response ACK."
				" flags = 0x%08x\n",
				tdls->pub->unit, __FUNCTION__, scb_tdls->flags));
			scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_RESP;
			/* Need to dequeue the packet from txq */
		}
		scb_tdls->flags &= ~TDLS_SCB_CHSW_MASK;
		wlc_tdls_pretbtt_update(wlc);
		return status_code;
	}

	/* Channel Switch Timing */
	ie = bcm_parse_tlvs(pdata, plen, DOT11_MNG_CHANNEL_SWITCH_TIMING_ID);
	if (!ie) {
		WL_ERROR(("wl%d:%s(): Missing Channel Switch Timing IE.\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_DECLINED;
	}
	if (ie->len != TDLS_CHANNEL_SWITCH_TIMING_IE_LEN) {
		WL_ERROR(("wl%d:%s(): wrong channel switch timing IE len = %d\n",
			tdls->pub->unit, __FUNCTION__, ie->len));

		return DOT11_SC_DECLINED;
	}

	/* save the switch_time/timout from the Channel Switch Response */
	chsw_timing_ie = (channel_switch_timing_ie_t *)ie;
	if (off_ch_req) {
		scb_tdls->switch_time = ltoh16(chsw_timing_ie->switch_time);
		scb_tdls->switch_timeout = ltoh16(chsw_timing_ie->switch_timeout);
		target_chanspec = scb_tdls->target_chanspec;
		if (scb_tdls->switch_time != tdls->switch_time) {
			wlc_tdls_pretbtt_update(wlc);
		}
	}
	else {
		if ((scb_tdls->switch_time != ltoh16(chsw_timing_ie->switch_time)) ||
			(scb_tdls->switch_timeout != ltoh16(chsw_timing_ie->switch_timeout))) {
			WL_TDLS(("wl%d:%s(): Mis-match SwitchTime(%d) and SwitchTimeout(%d) with"
				"previous Channel Switch Response SwitchTIme(%d) and "
				"SwitchTimeout(%d)\n", tdls->pub->unit, __FUNCTION__,
				ltoh16(chsw_timing_ie->switch_time),
				ltoh16(chsw_timing_ie->switch_timeout),
				scb_tdls->switch_time, scb_tdls->switch_timeout));
			/* return DOT11_SC_DECLINED; */
		}

		/* change target_chanspec to base chanspec */
		if (tdls->manual_chsw || !SCB_BSSCFG(scb_peer)->tdls->chsw_req_enabled)
			scb_tdls->target_chanspec = scb_tdls->base_chanspec;

		target_chanspec = scb_tdls->base_chanspec;
	}

	WL_TDLS(("wl%d:%s(): Channel Switch Request to 0x%04x accepted, switch_time = %d, "
		"switch_timeout = %d\n", tdls->pub->unit, __FUNCTION__,
		target_chanspec, scb_tdls->switch_time, scb_tdls->switch_timeout));

	wlc_tdls_switch_to_target_ch(tdls, scb_peer, target_chanspec);

	return 0;
}

static uint16
wlc_tdls_process_vendor_specific(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint offset)
{
	uchar *pdata;
	uint plen;
	tdls_lookaside_t *peer;
	wlc_bsscfg_t *parent = SCB_BSSCFG(scb);

	plen = PKTLEN(tdls->osh, f->p) - offset;

	pdata = PKTDATA(tdls->osh, f->p);
	pdata += offset;

	/* offset starts at vendor OUI field */
	/* only WFD Tunneled Probe Request/Response are considered */
	if ((plen < WFA_OUI_LEN + 1) || memcmp(pdata, WFA_OUI, WFA_OUI_LEN) ||
		(pdata[WFA_OUI_LEN] < WFA_OUI_TYPE_TPQ) ||
		(pdata[WFA_OUI_LEN] > WFA_OUI_TYPE_TPS)) {
		WL_ERROR(("wl%d:%s(): unsupported Vendor Specific Request/Response\n",
			tdls->pub->unit, __FUNCTION__));
		return DOT11_SC_INVALID_PARAMS;
	}

	pdata += WFA_OUI_LEN;
	plen -= WFA_OUI_LEN + 1;

	/* get all IEs */

	if (plen > 0) {

		peer = wlc_tdls_lookaside_find(tdls, f->sa);
		if (!peer) peer = wlc_tdls_lookaside_create(tdls, f->sa, parent, 0);

		if (!peer)
			WL_TDLS(("wl%d:%s(): TDLS lookaside table is full!\n",
				tdls->pub->unit, __FUNCTION__));
		else {

			if (peer->wfd_ie_probe_rx_length) {
				MFREE(tdls->osh, peer->wfd_ie_probe_rx,
					peer->wfd_ie_probe_rx_length);
				peer->wfd_ie_probe_rx = NULL;
				peer->wfd_ie_probe_rx_length = 0;
			}

			peer->wfd_ie_probe_rx = MALLOC(tdls->osh, plen);

			if (peer->wfd_ie_probe_rx) {
				peer->wfd_ie_probe_rx_length = (uint16) plen;
				memcpy(peer->wfd_ie_probe_rx, pdata + 1, plen);
			}
			else
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					tdls->pub->unit, __FUNCTION__,  MALLOCED(tdls->osh)));
		}
	}

	if (*pdata == WFA_OUI_TYPE_TPS) {

		/* we received a Tunneled Probe Response */

		/* nothing more to do */

	} else {

		/* we received a Tunneled Probe Request */

		wlc_tdls_send_tunneled_probe(tdls, parent, f->sa, WFA_OUI_TYPE_TPS);
	}

	return 0;
}

static void
wlc_tdls_tx_null_data_complete(wlc_info_t *wlc, uint32 txstatus, void *arg)
{
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */
	tdls_lookaside_t *dla = (tdls_lookaside_t *)arg;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	WL_NONE(("wl%d:%s(): Enter, arg = %p\n", wlc->pub->unit, __FUNCTION__, arg));

	if (!arg)
		return;

	scb = wlc_tdls_scbfind_all(wlc, &dla->ea);
	if (!scb || !scb->bsscfg || !BSS_TDLS_ENAB(wlc, scb->bsscfg))
		return;

	scb_tdls = SCB_TDLS(wlc->tdls, scb);
	ASSERT(scb_tdls);

	if (scb_tdls->flags & TDLS_SCB_WAIT_NULLDATA_ACK) {
		if (scb_tdls->timer_start)
			wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
		scb_tdls->timer_start = FALSE;
		scb_tdls->flags &= ~TDLS_SCB_WAIT_NULLDATA_ACK;
		if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
			WL_TDLS(("wl%d:%s(): NO ACK from %s for null-data on target channel %d, "
				"start listening for peer data transfer.\n",
				wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf),
				CHSPEC_CHANNEL(scb_tdls->target_chanspec)));
			if (OSL_SYSUPTIME() < scb_tdls->max_null_attempt_time) {
				/* still possible to retry frame */
				scb_tdls->flags |= TDLS_SCB_WAIT_TX_ON_TARGET_CH;
				/* send NULL-data anyways, in case peer can receive it */
				wl_del_timer(wlc->wl, scb_tdls->peer_addr->timer);
				wl_add_timer(wlc->wl, scb_tdls->peer_addr->timer, 0, 0);
				scb_tdls->timer_start = TRUE;
			} else {
				/* no ack and we expired allowed time, go back to base channel */
				chanspec_t cur_chanspec_save = 0;
				cur_chanspec_save = scb_tdls->cur_chanspec;
#ifdef WLMCHAN
				wlc_tdls_mchan_change_bss_chan_context(wlc->tdls, SCB_BSSCFG(scb),
					scb_tdls->base_chanspec);
#ifdef PROP_TXSTATUS
				wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
					WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
				wlc_wlfc_mchan_interface_state_update(wlc, SCB_BSSCFG(scb),
					WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif /* PROP_TXSTATUS */
#endif /* WLMCHAN */
				if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled) {
					wlc_tdls_chsw_recover(wlc, scb);
				} else if (cur_chanspec_save) {
					wlc_tdls_delete_chan_ctx(wlc->tdls, scb, cur_chanspec_save);
				}
			}
			return;
		}
		else {
			WL_TDLS(("wl%d.%d:%s(): flags = 0x%08x, rcv ACK from %s, successfully "
				"switch to target chanspec %s\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__,
				scb_tdls->flags, bcm_ether_ntoa(&scb->ea, eabuf),
				wf_chspec_ntoa_ex(scb_tdls->target_chanspec, chanbuf)));
#ifdef TDLS_TESTBED
			if (wlc->tdls->chsw_mode == TDLS_CHSW_UNSOLRESP)
				wlc_tdls_return_to_base_ch(wlc->tdls, scb);
#endif // endif
		}
	}

	if (scb_tdls->flags & TDLS_SCB_WAIT_HEARTBEAT_ACK) {
		scb_tdls->flags &= ~TDLS_SCB_WAIT_HEARTBEAT_ACK;
		if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK) {
			scb_tdls->heartbeat_missed++;
			WL_TDLS(("wl%d:%s(): heartbeat_missed (%d) to peer %s.\n",
				wlc->pub->unit, __FUNCTION__,
				scb_tdls->heartbeat_missed,
				bcm_ether_ntoa(&scb->ea, eabuf)));
		}
		else if (txstatus & TX_STATUS_ACK_RCV) {
			scb_tdls->heartbeat_missed = 0;
		}
		scb_tdls->heartbeat_countdown =
			(6 - scb_tdls->heartbeat_missed) > 0 ?
			(6 - scb_tdls->heartbeat_missed) : 0;
	}
}

void
wlc_tdls_return_to_base_ch_on_eosp(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	/* return to base channel if off channel */
	if (scb_tdls->cur_chanspec != scb_tdls->base_chanspec) {
		WL_TDLS(("wl%d:%s(): recv EOSP, return to base channel 0x%04x",
			tdls->pub->unit, __FUNCTION__, scb_tdls->base_chanspec));
		wlc_tdls_switch_to_target_ch(tdls, scb, scb_tdls->base_chanspec);
	}
}

static bool wlc_tdls_rcv_on_target_ch(tdls_info_t *tdls, d11rxhdr_t *rxhdr,
	chanspec_t target_chanspec)
{
	wlc_info_t *wlc = tdls->wlc;
	uint16 chan_bw, chan_band, chan_num;

	chan_bw = CHSPEC_BW(rxhdr->RxChan);
	if (CHSPEC_BW(target_chanspec) != chan_bw)
		return FALSE;

	chan_band = CHSPEC_BAND(rxhdr->RxChan);
	if (CHSPEC_BAND(target_chanspec) != chan_band)
		return FALSE;

	chan_num = WLC_RX_CHANNEL(rxhdr);
	if (chan_bw == WL_CHANSPEC_BW_40) {
		chanspec_t wlc_chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
		if (chan_num == CHSPEC_CHANNEL(wlc_chanspec)) {
			return (target_chanspec == wlc_chanspec);
		} else {
			/* best guess, the control channel could be different */
			return (chan_num == CHSPEC_CHANNEL(target_chanspec));
		}
	} else {
		/* XXX 4360: the is20 check looks redundant since target_chanspec
		 * bw was checked above
		 */
		return ((chan_num == CHSPEC_CHANNEL(target_chanspec)) &&
			(CHSPEC_IS20(target_chanspec)));
	}
}

void
wlc_tdls_rcv_data_frame(tdls_info_t *tdls, struct scb *scb, d11rxhdr_t *rxhdr)
{
	scb_tdls_t *scb_tdls;
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char chanbuf1[CHANSPEC_STR_LEN];
	char chanbuf2[CHANSPEC_STR_LEN];
#endif // endif

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	if (!scb_tdls->cur_chanspec || !WLC_RX_CHANNEL(rxhdr)) {
		WL_TDLS(("wl%d:%s(): TDLS SCB chanspec not initialized 0x%x rxhdr chspec 0x%x\n",
		         tdls->pub->unit, __FUNCTION__, scb_tdls->cur_chanspec,
		         WLC_RX_CHANNEL(rxhdr)));
		return;
	}

	if (wf_chspec_ctlchan(scb_tdls->cur_chanspec) != wf_chspec_ctlchan(rxhdr->RxChan)) {
		WL_ERROR(("wl%d:%s():rcv pkt on chanspec %s, not in the current control"
			" chanspec %s\n", tdls->pub->unit, __FUNCTION__,
			wf_chspec_ntoa_ex(WLC_RX_CHANNEL(rxhdr), chanbuf1),
			wf_chspec_ntoa_ex(scb_tdls->cur_chanspec, chanbuf2)));
		return;
	}

	if (!(scb_tdls->flags & TDLS_SCB_LISTEN_ON_TARGET_CH) &&
		(scb_tdls->flags & TDLS_SCB_WAIT_TX_ON_TARGET_CH) &&
		((scb_tdls->flags & (TDLS_SCB_RECV_CHSW_REQ | TDLS_SCB_WAIT_SWITCHTIME)) !=
		(TDLS_SCB_RECV_CHSW_REQ | TDLS_SCB_WAIT_SWITCHTIME))) {
		WL_TDLS(("wl%d:%s():TDLS link not in the right state flags = 0x%08x\n",
			tdls->pub->unit, __FUNCTION__, scb_tdls->flags));
		return;
	}

	if (wlc_tdls_rcv_on_target_ch(tdls, rxhdr, scb_tdls->target_chanspec)) {

		WL_TDLS(("wl%d:%s(): rcv data frame from %s, successfully switch to "
			"target chanspec %s\n",
			tdls->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf),
			wf_chspec_ntoa_ex(scb_tdls->target_chanspec, chanbuf1)));
		scb_tdls->flags &= ~(TDLS_SCB_LISTEN_ON_TARGET_CH);
		wl_del_timer(tdls->wlc->wl, scb_tdls->peer_addr->timer);
		scb_tdls->timer_start = FALSE;
		scb_tdls->flags &= ~(TDLS_SCB_RECV_CHSW_REQ | TDLS_SCB_WAIT_SWITCHTIME);
		wlc_tdls_wake_ctrl(tdls->wlc, tdls, scb);
	}
	else {
		WL_TDLS(("wl%d:%s(): rcv data frame from %s not on target chanspec %s.\n",
			tdls->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf),
			wf_chspec_ntoa_ex(scb_tdls->target_chanspec, chanbuf1)));
	}

	return;
}

/* Returns true if packet consumed */
void
wlc_tdls_rcv_action_frame(tdls_info_t *tdls, struct scb *scb, struct wlc_frminfo *f,
	uint pdata_offset)
{
	osl_t *osh = tdls->osh;
	uint plen;
	uint8 action_field;
	uchar *pdata;
	uint	offset;
	int status_code = 0;
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */

	 UNUSED_PARAMETER(plen);
	WL_TDLS(("wl%d:%s(): rcv TDLS action frame from %s\n", tdls->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(f->sa, eabuf)));

	if (scb->flags2 & SCB2_TDLS_PROHIBIT) {
		WL_ERROR(("wl%d:%s(): TDLS is prohibited in ext cap.\n", tdls->pub->unit,
			__FUNCTION__));
		goto end;
	}

	pdata = PKTDATA(osh, f->p);
	plen = PKTLEN(osh, f->p);

	pdata += pdata_offset;

	/* validate payload type */
	if (*pdata != TDLS_PAYLOAD_TYPE) {
		WL_ERROR(("wl%d:%s(): wrong payload type for 89-0d eth frame %d\n",
			tdls->pub->unit, __FUNCTION__, *pdata));
		goto end;
	}
	pdata += TDLS_PAYLOAD_TYPE_LEN;

	/* validate TDLS action category */
	if ((*pdata != TDLS_ACTION_CATEGORY_CODE) && (*pdata != TDLS_VENDOR_SPECIFIC)) {
		WL_ERROR(("wl%d:%s(): wrong TDLS Category %d\n", tdls->pub->unit,
			__FUNCTION__, *pdata));
		goto end;
	}

	if (*pdata == TDLS_VENDOR_SPECIFIC) {
		/* there is no action field with the vendor specific category */
		action_field = TDLS_VENDOR_SPECIFIC;
	} else {
		pdata++;
		action_field = *pdata;
	}

	pdata++;
	WL_TDLS(("wl%d:%s(): TDLS action field is %d\n", tdls->pub->unit,
		__FUNCTION__, action_field));

	/* offset starts after action field */
	offset = (uint)(pdata - PKTDATA(osh, f->p));
	TDLS_PRHEX("recv_action_frame", pdata, (PKTLEN(osh, f->p) - offset));

	switch (action_field) {
		case TDLS_SETUP_REQ:
#ifdef TDLS_TESTBED
			if (tdls->test_mac) {
				tdls_iovar_t info;

				tdls->test_mac = FALSE;
				info.mode = TDLS_MANUAL_EP_CREATE;
				bcopy(f->sa, &info.ea, 6);

				wlc_tdls_endpoint_op(tdls, &info, SCB_BSSCFG(scb));
			}
#endif /* TDLS_TESTBED */

			status_code = wlc_tdls_process_setup_req(tdls, scb, f, offset);

			break;

		case TDLS_SETUP_RESP:
			status_code = wlc_tdls_process_setup_resp(tdls, scb, f, offset);

			break;

		case TDLS_SETUP_CONFIRM:
			status_code = wlc_tdls_process_setup_cfm(tdls, scb, f, offset);

			break;

		case TDLS_TEARDOWN:
			status_code = wlc_tdls_process_teardown(tdls, scb, f, offset);

			break;

		case TDLS_PEER_TRAFFIC_IND:
			status_code = wlc_tdls_process_pti(tdls, scb, f, offset);

			break;

		case TDLS_CHANNEL_SWITCH_REQ:
			status_code = wlc_tdls_process_chsw_req(tdls, scb, f, offset);

			break;

		case TDLS_CHANNEL_SWITCH_RESP:
			status_code = wlc_tdls_process_chsw_resp(tdls, scb, f, offset);

			break;

		case TDLS_PEER_PSM_REQ:

			break;

		case TDLS_PEER_PSM_RESP:

			break;

		case TDLS_PEER_TRAFFIC_RESP:
			status_code = wlc_tdls_process_pti_resp(tdls, scb, f, offset);

			break;

		case TDLS_DISCOVERY_REQ:
			status_code = wlc_tdls_process_discovery_req(tdls, scb, f, offset);

			break;

		case TDLS_VENDOR_SPECIFIC:
			status_code = wlc_tdls_process_vendor_specific(tdls, scb, f, offset);

			break;

		default:
			WL_TDLS(("wl%d:%s(): un-defined TDLS action field.\n",
				tdls->pub->unit, __FUNCTION__));
			break;
	}

end:
	if (status_code < 0) {
		WL_TDLS(("wl%d:%s(): rcv TDLS action frame processing failed!\n",
		tdls->pub->unit, __FUNCTION__));
	}

	PKTFREE(osh, f->p, FALSE);
	return;
}

bool
wlc_tdls_recvfilter(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	ASSERT(scb);

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	if (!(scb_tdls->flags & TDLS_SCB_CONNECTED))
		return TRUE;

	return FALSE;
}

void
wlc_tdls_process_discovery_resp(tdls_info_t *tdls, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi)
{
	tdls_pub_act_frame_t *tdls_disc_resp = (tdls_pub_act_frame_t *)body;
#if defined(BCMDBG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */
	link_id_ie_t *linkId_ie;
	tdls_lookaside_t *peer = NULL;

	WL_TDLS(("wl%d:%s(): rcv TDLS discovery action frame from %s\n",
		tdls->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));

	/* starts TLV IEs */
	linkId_ie = (link_id_ie_t*)bcm_parse_tlvs(&tdls_disc_resp->elts[0], body_len,
		DOT11_MNG_LINK_IDENTIFIER_ID);
	if (!linkId_ie) {
		WL_ERROR(("wl%d:%s(): no Link Identifier IE in TDLS Discovery Request!\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}
	if (linkId_ie->len != TDLS_LINK_ID_IE_LEN) {
		WL_ERROR(("wl%d:%s(): Link Identifier IE len = %d wrong!\n",
			tdls->pub->unit, __FUNCTION__, linkId_ie->len));
		return;
	}

	if (memcmp(&linkId_ie->bssid, &hdr->bssid, ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): BSSID mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	if (memcmp(&linkId_ie->tdls_init_mac, &hdr->da,	ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): Link Identifier Initiator mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}
	if (memcmp(&linkId_ie->tdls_resp_mac, &hdr->sa, ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d:%s(): Link Identifier responder mismatch!\n",
			tdls->pub->unit, __FUNCTION__));
		return;
	}

	peer = wlc_tdls_lookaside_find(tdls, &hdr->sa);
	if (!peer) {
		struct scb *parent_scb = wlc_tdls_scbfindband_all(tdls->wlc, &hdr->bssid,
			CHSPEC_WLCBANDUNIT(tdls->wlc->home_chanspec));
		ASSERT(parent_scb);

		peer = wlc_tdls_lookaside_create(tdls, &hdr->sa, SCB_BSSCFG(parent_scb), 0);
	}
	else
		peer->flags &= ~TDLS_LOOKASIDE_TEMP;
	peer->rssi = rssi;
	if (peer->pmoverride_set) {
		wlc_set_pmoverride(peer->parent, FALSE);
		peer->pmoverride_set = FALSE;
	}
	wlc_mac_event(tdls->wlc, WLC_E_TDLS_PEER_EVENT, &hdr->sa,
		WLC_E_STATUS_SUCCESS, WLC_E_TDLS_PEER_DISCOVERED, 0, NULL, 0);
}

bool
wlc_tdls_quiet_down(tdls_info_t *tdls)
{
#ifdef TDLS_TESTBED
	return tdls->quiet_down;
#else
	return FALSE;
#endif // endif
}

static bool
wlc_tdls_need_pti(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	if (!scb)
		return FALSE;

	scb_tdls = SCB_TDLS(tdls, scb);
	if (!scb_tdls)
		return FALSE;

	if ((scb_tdls->flags & TDLS_SCB_SENT_PTI) ||
		(scb_tdls->flags & TDLS_SCB_SCHED_PTI)) {
		WL_TDLS(("wl%d: %s(): exiting PTI sent/scheduled.\n",
			tdls->wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	return TRUE;
}

bool
wlc_tdls_in_pti_interval(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	if (!scb)
		return FALSE;

	scb_tdls = SCB_TDLS(tdls, scb);
	if (!scb_tdls)
		return FALSE;

	if (scb_tdls->flags & TDLS_SCB_PTI_INTERVAL) {
		WL_TDLS(("%s(): TRUE.\n", __FUNCTION__));
		return TRUE;
	}

	WL_TDLS(("%s(): FALSE.\n", __FUNCTION__));
	return FALSE;
}

void
wlc_tdls_apsd_usp_end(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;

	if (!scb)
		return;

	scb_tdls = SCB_TDLS(tdls, scb);
	if (!scb_tdls)
		return;

	/* scb_tdls->apsd_usp_endtime = OSL_SYSUPTIME(); */

	/* return to base channel if off-channel */
	wlc_tdls_return_to_base_ch_on_eosp(tdls, scb);
}

uint
wlc_tdls_apsd_usp_interval(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;
	uint now = OSL_SYSUPTIME();

	if (!scb)
		return 0;

	scb_tdls = SCB_TDLS(tdls, scb);
	if (!scb_tdls)
		return 0;

	if (now >= scb_tdls->apsd_usp_endtime)
		return now - scb_tdls->apsd_usp_endtime;
	else
		return scb_tdls->apsd_usp_endtime - now;
}

bool
wlc_tdls_stay_awake(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	return (!cfg->pm->PMenabled || cfg->tdls->tdls_PMAwake);
}

bool wlc_tdls_is_chsw_enabled(tdls_info_t *tdls)
{
	if (tdls->manual_chsw)
		return FALSE;
	return TRUE;
}

void
wlc_tdls_do_chsw(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg, bool off_channel)
{
	scb_tdls_t *scb_tdls;
	struct scb *scb;

	ASSERT(bsscfg);
	if (!BSS_TDLS_ENAB(tdls->wlc, bsscfg))
		return;

	scb = bsscfg->tdls_scb;
	if (!scb)
		return;

	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	if (scb_tdls->flags & TDLS_SCB_CHSW_MASK) {
		if ((scb_tdls->flags & TDLS_SCB_SENT_CHSW_RESP) && !off_channel) {
			WL_TDLS(("wl%d.%d:%s(): clear waiting for Channel Switch Resp.\n",
			         tdls->pub->unit, bsscfg->_idx, __FUNCTION__));
			scb_tdls->flags &= ~TDLS_SCB_SENT_CHSW_RESP;
			wlc_tdls_switch_to_target_ch(tdls, scb, scb_tdls->base_chanspec);
		}
		else {
			WL_TDLS(("wl%d.%d:%s(): TDLS channel switch in progress(0x%08x)...\n",
				tdls->pub->unit, bsscfg->_idx, __FUNCTION__, scb_tdls->flags));
		}
		return;
	}
	if (scb_tdls->target_chanspec == scb_tdls->base_chanspec) {
		WL_TDLS(("wl%d.%d:%s(): auto channel switch stopped.\n",
			tdls->pub->unit, bsscfg->_idx, __FUNCTION__));
		return;
	}
	if (off_channel) {
		if (tdls->manual_chsw)
			return;
		if ((scb_tdls->base_chanspec == scb_tdls->target_chanspec) ||
			(scb_tdls->cur_chanspec == scb_tdls->target_chanspec))
			return;
		if (SCB_PS(scb)) {
			WL_TDLS(("wl%d.%d:%s(): Peer sleep, auto channel switch stopped.\n",
			tdls->pub->unit, bsscfg->_idx, __FUNCTION__));
			return;
		}
		if (wlc_tdls_send_chsw_req(tdls, scb, scb_tdls->target_chanspec))
			wlc_tdls_chsw_recover(tdls->wlc, scb);
	}
	else {
		scb_tdls->flags |= TDLS_SCB_CHSWRSP_CANCEL;
		wlc_tdls_return_to_base_ch(tdls, scb);
	}

	return;
}

bool
wlc_tdls_chk_switch_allowed(tdls_info_t *tdls, wlc_bsscfg_t *bsscfg)
{
	scb_tdls_t *scb_tdls;
	struct scb *scb;
	if (!BSS_TDLS_ENAB(tdls->wlc, bsscfg))
		return FALSE;
	scb = bsscfg->tdls_scb;
	if	(!scb) {
	return FALSE;
	}
	scb_tdls = SCB_TDLS(tdls, scb);
	return !((scb_tdls->flags & TDLS_SCB_LISTEN_ON_TARGET_CH) ||
			(scb_tdls->flags & TDLS_SCB_WAIT_TX_ON_TARGET_CH));
}

uint16
wlc_tdls_get_pretbtt_time(tdls_info_t *tdls)
{
	/* need the max switch time here for any TDLS scb */
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	uint16 switch_time = 0;
	uint16 max_switch_time = 0;
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_BSS(wlc, idx, cfg) {
		if (BSS_TDLS_ENAB(wlc, cfg)) {
			scb = cfg->tdls_scb;
			scb_tdls = SCB_TDLS(tdls, scb);
			switch_time = (tdls->switch_time > scb_tdls->switch_time) ?
				tdls->switch_time : scb_tdls->switch_time;
			if (switch_time > max_switch_time)
				max_switch_time = switch_time;
		} else
			continue;
	}
	return max_switch_time;
}

bool
wlc_tdls_is_active(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;
	bool ret = FALSE;

	FOREACH_BSS(wlc, idx, cfg) {
		if (BSS_TDLS_ENAB(wlc, cfg)) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

#ifdef WLMCHAN
static void
wlc_tdls_do_chanswitch(tdls_info_t *tdls, wlc_bsscfg_t *cfg, chanspec_t newchspec)
{
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	wlc_info_t *wlc = cfg->wlc;
	bool band_chg = FALSE;
	bool bw_chg = FALSE;
	int bandunit;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	if (!BSS_TDLS_ENAB(wlc, cfg))
		return;

	scb = cfg->tdls_scb;
	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	WL_INFORM(("wl%d: %s: new chanspec %s\n", wlc->pub->unit, __FUNCTION__,
		wf_chspec_ntoa_ex(newchspec, chanbuf)));

	if (!wlc_valid_chanspec_db(wlc->cmi, newchspec)) {
		WL_REGULATORY(("%s: Received invalid channel - ignoring\n", __FUNCTION__));
		/* something needs to be done here ?? */
		return;
	}

	/* evaludate band or bandwidth change here before wlc->home_chanspec gets changed */
	if (CHSPEC_WLCBANDUNIT(scb_tdls->cur_chanspec) != CHSPEC_WLCBANDUNIT(newchspec))
		band_chg = TRUE;
	if (CHSPEC_BW(scb_tdls->cur_chanspec) != CHSPEC_BW(newchspec))
		bw_chg = TRUE;

	wlc_suspend_mac_and_wait(wlc);
	wlc_set_chanspec(wlc, newchspec);
#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		wlc_mchan_set_priq(wlc->mchan, cfg);
	}
#endif // endif
	wlc_enable_mac(wlc);

	WL_REGULATORY(("%s: switched to chanspec %s\n", __FUNCTION__,
		wf_chspec_ntoa_ex(newchspec, chanbuf)));

	bandunit = CHSPEC_WLCBANDUNIT(newchspec);
	if (band_chg) {
		wlc_scb_switch_band(wlc, scb, bandunit, scb->bsscfg);
		/* in case of band switch, restart ap */
		if (AP_ACTIVE(wlc))
			wlc_restart_ap(wlc->ap);
		if (scb_tdls->base_chan_bandunit == bandunit) {
			bcopy(&scb_tdls->base_chan_rateset, &scb->rateset, sizeof(wlc_rateset_t));
			wlc_scb_ratesel_init(wlc, scb);
			return;
		}
	}

	/* force a rate selection init. */
	if (bw_chg || band_chg)
		wlc_scb_reinit(wlc);

}

static int
wlc_tdls_mchan_change_bss_chan_context(tdls_info_t *tdls, wlc_bsscfg_t *cfg,
	chanspec_t new_chanspec)
{
	wlc_info_t *wlc = tdls->wlc;
	int ret;

	WL_TDLS(("%s():wl%d.%d: BEFORE: cfg->wlcif->qi = %p, "
		"wlc->active_q = %p\n", __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		cfg->wlcif->qi, wlc->active_queue));

	ret = wlc_mchan_create_bss_chan_context(wlc, cfg, new_chanspec);
	if (ret)
		return ret;

	wlc_tdls_do_chanswitch(tdls, cfg, new_chanspec);

	WL_TDLS(("%s():wl%d.%d: AFTER: cfg->wlcif->qi = %p, "
		"wlc->active_q = %p\n", __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		cfg->wlcif->qi, wlc->active_queue));

	return 0;
}
#endif /* WLMCHAN */

static void
wlc_tdls_chsw_complete(tdls_info_t *tdls, struct scb *scb)
{
	scb_tdls_t *scb_tdls;
	uint32 delta;
	uint32 now = OSL_SYSUPTIME();

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	/* how to perform CCA ??? */
	if (!(scb_tdls->flags & TDLS_SCB_RECV_CHSW_REQ)) {
		delta = now - scb_tdls->action_frame_sent_time;
		/* the initiator shall send a NULL-data frame to peer */
#ifdef TDLS_TESTBED
		if (tdls->test_chsw_timeout)
			delta += scb_tdls->switch_timeout/1000;
#endif // endif

		if (delta < (uint)scb_tdls->switch_time/1000) {
#ifdef TDLS_TESTBED
			if (!tdls->test_chsw_timeout)
#endif // endif
			delta = scb_tdls->switch_time/1000 - delta;
			WL_TDLS(("wl%d:%s(): Initiator schedule sending NULL-data to Peer, "
				" delta = %d\n",
				tdls->pub->unit, __FUNCTION__, delta));
			scb_tdls->flags |= TDLS_SCB_WAIT_TX_ON_TARGET_CH;
			/* send NULL-data anyways, in case peer can receive it */
			wl_add_timer(tdls->wlc->wl, scb_tdls->peer_addr->timer, delta, 0);
			scb_tdls->timer_start = TRUE;
		}
		else
			wlc_tdls_chsw_send_nulldata(tdls, scb, now);
	}
	else {
		/* responder waiting for data transmition within switchTimeout */
		WL_TDLS(("wl%d:%s(): Responder waiting for Peer to transmit data "
			"frame.\n", tdls->pub->unit, __FUNCTION__));
		wlc_tdls_listen_on_target_ch(tdls, scb_tdls);
	}

	return;
}

static void
wlc_tdls_delete_chan_ctx(tdls_info_t *tdls, struct scb *scb, chanspec_t chanspec)
{
	scb_tdls_t *scb_tdls;

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

#ifdef WLMCHAN
	wlc_mchan_create_bss_chan_context(tdls->wlc, SCB_BSSCFG(scb), chanspec);
	wlc_mchan_delete_bss_chan_context(tdls->wlc, SCB_BSSCFG(scb));
	wlc_mchan_create_bss_chan_context(tdls->wlc, SCB_BSSCFG(scb), scb_tdls->base_chanspec);
#endif // endif

	return;
}

static void
wlc_tdls_switch_to_target_ch(tdls_info_t *tdls, struct scb *scb, chanspec_t target_chanspec)
{
	scb_tdls_t *scb_tdls;
	chanspec_t cur_chanspec_save;

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	WL_TDLS(("wl%d:%s(): switch from cur_chanspec 0x%04x to target_chanspec 0x%04x\n",
		tdls->pub->unit, __FUNCTION__, scb_tdls->cur_chanspec, target_chanspec));

	if (scb_tdls->cur_chanspec == target_chanspec)
		return;

	cur_chanspec_save = scb_tdls->cur_chanspec;

	scb_tdls->action_frame_sent_time = OSL_SYSUPTIME();

#ifdef WLMCHAN
	wlc_tdls_mchan_change_bss_chan_context(tdls, SCB_BSSCFG(scb), target_chanspec);
#endif // endif
	scb_tdls->cur_chanspec = target_chanspec;

#if defined(WLMCHAN) && defined(PROP_TXSTATUS)
	if (scb_tdls->cur_chanspec == scb_tdls->base_chanspec) {
		wlc_wlfc_mchan_interface_state_update(tdls->wlc, scb_tdls->parent,
			WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
		wlc_wlfc_mchan_interface_state_update(tdls->wlc, SCB_BSSCFG(scb),
			WLFC_CTL_TYPE_MAC_OPEN, FALSE);
	}
#endif // endif
	/* for off-channel switch, start/wait for data transfer on target channel */
	if (scb_tdls->cur_chanspec != scb_tdls->base_chanspec) {
		wlc_tdls_chsw_complete(tdls, scb);
	}
	else {
		wlc_tdls_pm_mode_leave(tdls, scb_tdls->parent);
		if (scb_tdls->target_chanspec == scb_tdls->base_chanspec) {
			WL_TDLS(("wl%d:%s(): Disable channel switch...\n",
				tdls->pub->unit, __FUNCTION__));

			wlc_tdls_delete_chan_ctx(tdls, scb, cur_chanspec_save);
		}
	}
}

static chanspec_t wlc_tdls_target_chanspec(uint8 target_ch, uint8 second_ch_offset)
{
	uint channel, band, bw, ctl_sb = 0;

	if (target_ch > MAXCHANNEL)
		return 0;

	channel = (uint)target_ch;
	band = ((channel <= CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G);

	switch (second_ch_offset) {
		case DOT11_EXT_CH_NONE:
			bw = WL_CHANSPEC_BW_20;

			break;

		case DOT11_EXT_CH_UPPER:
			bw = WL_CHANSPEC_BW_40;
			/* control SB is the opposit of Secondary channel offset */
			ctl_sb = WL_CHANSPEC_CTL_SB_LOWER;
			/* adjust channel to center of 40MHz band */
			if (target_ch <= (MAXCHANNEL - CH_20MHZ_APART))
				channel += CH_10MHZ_APART;
			else
				return 0;

			break;

		case DOT11_EXT_CH_LOWER:
			bw = WL_CHANSPEC_BW_40;
			/* control SB is the opposit of Secondary channel offset */
			ctl_sb = WL_CHANSPEC_CTL_SB_UPPER;
			/* adjust channel to center of 40MHz band */
			if (target_ch > CH_20MHZ_APART)
				channel -= CH_10MHZ_APART;
			else
				return 0;

			break;

		default:
			WL_ERROR(("%s(): Invalid Secondary Channel Offset %d.\n",
				__FUNCTION__, second_ch_offset));
			return 0;
	}

	return (channel | band | bw | ctl_sb);
}

static int wlc_tdls_validate_target_chanspec(tdls_info_t *tdls, struct scb *scb,
	chanspec_t target_chanspec)
{
	scb_tdls_t *scb_tdls;
	uint8 target_ch;
	uint8 target_regclass;
	wlc_bsscfg_t *bsscfg;
	wlc_tdls_t *tc;
	int i;

	target_ch = wf_chspec_ctlchan(target_chanspec);
	if (target_ch > MAXCHANNEL) {
		WL_TDLS(("%s(): wl%d: invalid target channel %d!\n", __FUNCTION__,
		         tdls->pub->unit, target_ch));
		return BCME_BADCHAN;
	}

	if (wlc_radar_chanspec(tdls->wlc->cmi, target_chanspec)) {
		WL_TDLS(("%s(): wl%d: target channel is a radar channel %d!\n", __FUNCTION__,
		         tdls->pub->unit, target_ch));
		return BCME_BADCHAN;
	}

	ASSERT(scb);
	bsscfg = SCB_BSSCFG(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	if (!scb_tdls)
		return BCME_ERROR;
	tc = bsscfg->tdls;

	if (target_chanspec == scb_tdls->cur_chanspec) {
		WL_TDLS(("wl%d:%s(): Same chanspec 0x%x as cur channel, no switch.\n",
			tdls->pub->unit, __FUNCTION__, target_chanspec));
		return BCME_BADCHAN;
	}

	target_regclass = wlc_get_regclass(tdls->wlc->cmi, target_chanspec);
	if (!target_regclass) {
		WL_TDLS(("%s(): wl%d: Cannot find target channel %d in the Regulatory Classes!\n",
			__FUNCTION__, tdls->pub->unit, target_ch));
		return BCME_BADCHAN;
	}

	WL_TDLS(("%s(): wl%d: found target channel %d in Regulatory Class %d\n",
		__FUNCTION__, tdls->pub->unit, target_ch, target_regclass));

	/* should be selected from the set of regulatory classes supported by both TDLS peer STAs */
	for (i = 0; i < scb_tdls->rclen; i++)
		if (target_regclass == scb_tdls->rclist[i])
			break;

	if (i == scb_tdls->rclen) {
		WL_TDLS(("%s(): wl%d: target channel %d not in the Peer's Regulatory Classes!\n",
			__FUNCTION__, tdls->pub->unit, target_ch));
		return BCME_BADCHAN;
	}

	for (i = 0; i < tc->rclen; i++)
		if (target_regclass == tc->rclist[i])
			break;

	if (i == tc->rclen) {
		WL_TDLS(("%s(): wl%d: target channel %d not in its Regulatory Classes!\n",
			__FUNCTION__, tdls->pub->unit, target_ch));
		return BCME_BADCHAN;
	}

	if (CHSPEC_IS40(target_chanspec)) {
		int target_bandunit = CHSPEC_WLCBANDUNIT(target_chanspec);
		wlcband_t *target_band = tdls->wlc->bandstate[target_bandunit];
		/* TDLS peer STAs indicated 40MHz support in the Supported Channel Width Set
		  * filed of the HT Capabilities element
		*/
		if ((!(tc->ht_capinfo & HT_CAP_40MHZ) ||
			!(wlc_ht_is_scb_40MHZ_cap(tdls->wlc->hti, scb))) &&
			!WL_BW_CAP_40MHZ(target_band->bw_cap)) {
			WL_TDLS(("wl%d:%s(): wl%d: no 40MHz support on STA or peer!\n",
				tdls->pub->unit, __FUNCTION__, target_ch));
			return BCME_BADCHAN;
		}
	}

	return 0;
}

static int wlc_tdls_chsw_validate_pmstate(tdls_info_t *tdls, struct scb *scb, wlc_bsscfg_t *parent)
{
	if ((!parent->pm->PMenabled || parent->pm->PMpending)) {
		wlc_tdls_pm_allowed_update(tdls, parent);
		/* TDLS peer STA shall be in PS mode with the AP */
		WL_TDLS(("wl%d:%s(): set PM state of parent BSS"
			"(send NULL data to AP) first!\n",
			tdls->pub->unit, __FUNCTION__));
		wlc_set_pmstate(parent, TRUE);
	}
	else {
		/* TDLS peer STA shall not be involved in an active SP with AP */
		if (scb->bsscfg->pm->apsd_sta_usp) {
			WL_TDLS(("wl%d:%s(): STA is in active SP with AP.\n",
				tdls->pub->unit, __FUNCTION__));
			return BCME_NOTREADY;
		}
	}

	return 0;
}

static void
wlc_tdls_pm_mode_leave(tdls_info_t *tdls, wlc_bsscfg_t *parent)
{
	wlc_info_t *wlc = tdls->wlc;
	if ((parent->pm->PM == PM_OFF || parent->pm->PM == PM_FAST ||
		parent->pm->WME_PM_blocked) && parent->pm->PMenabled) {
		if (parent->pm->PM == PM_FAST) {
			/* inform AP that STA's is ready for receiving
			* traffic without waiting for beacon and start the
			* pm2 timer using remaining idle time accounting
			* from absolute to on-channel time.
			*/
			wlc_bcn_tim_ie_pm2_action(wlc, parent);
		}
		else {
			wlc_set_pmstate(parent, FALSE);
		}
	}
}

static int
wlc_tdls_sendnulldata_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *p, void *data)
{
	scb_tdls_t *scb_tdls = (scb_tdls_t *)data;
	int err;
	ASSERT(wlc != NULL);

	err = wlc_pcb_fn_register(wlc->pcb, wlc_tdls_tx_null_data_complete,
		(void *)scb_tdls->peer_addr, p);
	if (err) {
		WL_ERROR(("%s: wlc_pcb_fn_register returned %d\n", __FUNCTION__, err));
		return err;
	}

	return BCME_OK;
}

static void
wlc_tdls_chsw_send_nulldata(tdls_info_t *tdls, struct scb *scb, uint32 now)
{
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls;
	uint32 delta;

	ASSERT(scb);
	scb_tdls = SCB_TDLS(tdls, scb);
	ASSERT(scb_tdls);

	delta = now - scb_tdls->action_frame_sent_time;
	WL_TDLS(("wl%d:%s(): Initiator sending NULL-data to Peer, delta = %dms\n",
		tdls->pub->unit, __FUNCTION__, delta));
	if (delta < (uint)scb_tdls->switch_timeout/1000) {
		delta = scb_tdls->switch_timeout/1000 - delta;
		scb_tdls->max_null_attempt_time = now + delta +
			(scb_tdls->switch_timeout - scb_tdls->switch_time);
		WL_TDLS(("wl%d:%s(): Initiator sending NULL-data to Peer now,"
			"wait %dms for NULL data ACK.\n",
			tdls->pub->unit, __FUNCTION__, delta));
		scb_tdls->flags |= TDLS_SCB_WAIT_NULLDATA_ACK;
		wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, 0, 0, PRIO_8021D_VO,
			wlc_tdls_sendnulldata_cb, (void *)scb_tdls);
		wl_add_timer(wlc->wl, scb_tdls->peer_addr->timer, delta, 0);
			scb_tdls->timer_start = TRUE;
	}
	else {
		WL_TDLS(("wl%d:%s(): SwitchTimout expiered, "
			"go back to base channel.\n",
			tdls->pub->unit, __FUNCTION__));
#ifdef WLMCHAN
		wlc_tdls_mchan_change_bss_chan_context(tdls, scb->bsscfg,
			scb_tdls->base_chanspec);
#ifdef PROP_TXSTATUS
		wlc_wlfc_mchan_interface_state_update(wlc, scb_tdls->parent,
			WLFC_CTL_TYPE_INTERFACE_OPEN, FALSE);
		wlc_wlfc_mchan_interface_state_update(wlc, scb->bsscfg,
			WLFC_CTL_TYPE_MAC_OPEN, FALSE);
#endif /* PROP_TXSTATUS */
#endif /* WLMCHAN */
		scb_tdls->cur_chanspec = scb_tdls->base_chanspec;
		if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled)
			wlc_tdls_chsw_recover(wlc, scb);
	}

	return;

}

static void
wlc_tdls_watchdog(void *hdl)
{
	tdls_info_t *tdls = (tdls_info_t *)hdl;
	wlc_info_t *wlc = tdls->wlc;
	struct scb *scb;
	scb_tdls_t *scb_tdls;
	struct scb_iter scbiter;
	wlc_tdls_t *tc;
	int i;
	tdls_lookaside_t *dla;
	uint32 now = OSL_SYSUPTIME();

	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		dla = &tdls->lookaside[i];
		if (dla->status == TDLS_LOOKASIDE_ACTIVE)
			continue;

		if (dla->status == TDLS_LOOKASIDE_WAIT_TO_DISC) {
			if (dla->pkt_cnt > tdls->trigger_pktcnt_high) {
				/* send TDLS discovery Req */
				wlc_tdls_send_discovery_req(tdls, dla->parent, &dla->ea);
			}
			else
				dla->pkt_cnt = 0;

			if (now - dla->upd_time > tdls->idle_time)
				wlc_tdls_lookaside_delete(tdls, dla);

			continue;
		}

		if ((tdls->auto_op) && (dla->flags & TDLS_LOOKASIDE_INUSE)) {
			if (dla->status == TDLS_LOOKASIDE_IDLE) {
				/* set up TDLS if
				 * -RSSI is over threshold
				 */
				if (dla->rssi > tdls->rssi_threshold_high) {
					tdls_iovar_t info;
					bzero(&info, sizeof(tdls_iovar_t));
					bcopy(&dla->ea, &info.ea, ETHER_ADDR_LEN);
					info.mode = TDLS_MANUAL_EP_CREATE;
					wlc_tdls_endpoint_op(tdls, &info, dla->parent);
				}
			}

			if (dla->status == TDLS_LOOKASIDE_DECLINED_IDLE) {
				if (now - dla->upd_time > TDLS_LOOKASIDE_DECLINED_IDLE_TIME)
					wlc_tdls_lookaside_delete(tdls, dla);
			}
		}
	}

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!BSSCFG_IS_TDLS(scb->bsscfg))
			continue;

		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls != NULL);

		if (!scb_tdls || !(scb_tdls->flags & TDLS_SCB_CONNECTED))
			continue;

		tc = scb->bsscfg->tdls;
		tc->up_time++;
		WL_TDLS(("wl%d.%d:%s(): up_time = %d!\n", tdls->pub->unit,
			WLC_BSSCFG_IDX(scb->bsscfg), __FUNCTION__, tc->up_time));
		if ((tc->up_time > tc->tpk_lifetime) ||
			(scb_tdls->heartbeat_missed > TDLS_MAX_HEARTBEAT_MISSED)) {
			WL_TDLS(("wl%d.%d:%s(): SA life time expired/max heartbeat missed!\n",
				tdls->pub->unit, WLC_BSSCFG_IDX(scb->bsscfg), __FUNCTION__));
			wlc_tdls_disconnect(tdls, scb, TRUE);
			continue;
		}

		if ((scb_tdls->flags & TDLS_SCB_SENT_PTI) &&
			((now - scb_tdls->pti_sent_time) > TDLS_RESP_TIMEOUT_DEFAULT * 1000)) {
			WL_TDLS(("wl%d:%s(): PTI response timeout!\n", tdls->pub->unit,
			         __FUNCTION__));

#ifndef WLTDLS_SEND_PTI_RETRY
				wlc_tdls_disconnect(tdls, scb, TRUE);
				continue;
#else
			/* retry on first PTI response timeout */
			if (scb_tdls->flags & TDLS_SCB_PTI_RESP_TIMED_OUT) {
				wlc_tdls_disconnect(tdls, scb, TRUE);
				continue;
			}
			scb_tdls->flags |= TDLS_SCB_PTI_RESP_TIMED_OUT;
			scb_tdls->flags &= ~TDLS_SCB_SENT_PTI;
			wlc_tdls_wake_ctrl(wlc, tdls, scb);
			wlc_tdls_send_pti(tdls, scb);
#endif // endif
		}

		if (tdls->auto_op) {
			int rssi = wlc_scb_rssi(scb);
#ifdef WLCNTSCB
			uint32 link_pkt_cnt;
#endif // endif

			if (rssi && (rssi < tdls->rssi_threshold_low)) {
				WL_TDLS(("wl%d.%d:%s(): RSSI < rssi_threshold_low %d,"
					"TDLS tear down...\n",
					tdls->pub->unit, WLC_BSSCFG_IDX(scb->bsscfg), __FUNCTION__,
					tdls->rssi_threshold_low));
				wlc_tdls_disconnect(tdls, scb, TRUE);
				continue;
			}

			if (now - scb_tdls->peer_addr->upd_time >
				tdls->idle_time) {
				WL_TDLS(("wl%d.%d:%s(): No activity for %d s, TDLS tear down...\n",
					tdls->pub->unit, WLC_BSSCFG_IDX(scb->bsscfg), __FUNCTION__,
					tdls->idle_time));
				wlc_tdls_disconnect(tdls, scb, TRUE);
				continue;
			}
#ifdef WLCNTSCB
			link_pkt_cnt = (scb->scb_stats.rx_ucast_pkts + scb->scb_stats.tx_pkts);
			if (scb_tdls->peer_addr->pkt_cnt + tdls->trigger_pktcnt_low <
				link_pkt_cnt) {
				wlc_tdls_lookaside_status_upd(tdls, scb_tdls->peer_addr,
					TDLS_LOOKASIDE_ACTIVE);
			}
				scb_tdls->peer_addr->pkt_cnt = link_pkt_cnt;
#endif /* WLCNTSCB */
		}

		if (!SCB_PS(scb)) {
			if (!scb_tdls->heartbeat_countdown) {
				if (scb_tdls->flags & TDLS_SCB_WAIT_HEARTBEAT_ACK)
					continue;

				if (wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea,
					0, 0, PRIO_8021D_VO, wlc_tdls_sendnulldata_cb,
					(void *)scb_tdls))
					scb_tdls->flags |= TDLS_SCB_WAIT_HEARTBEAT_ACK;
			}
			else
				scb_tdls->heartbeat_countdown--;
		}
		if (SCB_BSSCFG(scb)->tdls->chsw_req_enabled &&
			((now - scb_tdls->chswreq_sent_time) >=
			TDLS_CHSW_RECOVERY_TIMEOUT_DEFAULT)) {
			wlc_tdls_chsw_recover(wlc, scb);
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)|| defined(TDLS_TESTBED)
const char *tdls_scbflags_str[] = { TDLS_SCBFLAGS_STR };
const int tdls_scbflags_cnt = sizeof(tdls_scbflags_str)/sizeof(tdls_scbflags_str[0]);

static int
wlc_tdls_dump(tdls_info_t *tdls, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = tdls->wlc;
#ifdef WLCNT
	wlc_tdls_cnt_t *cnt = tdls->cnt;
#endif // endif
	struct scb *scb;
	struct scb_iter scbiter;
	char eabuf[ETHER_ADDR_STR_LEN];
	int i;

	bcm_bprintf(b, "tdls auto_op = %s\n", tdls->auto_op ? "TRUE" : "FALSE");
	bcm_bprintf(b, "tdls %d now %d cur_sessions %d max_sessions %d\n",
		wlc->pub->_tdls, tdls->pub->now, tdls->cur_sessions, tdls->max_sessions);
	bcm_bprintf(b, "discard %d ubuffered %d buf_reins %d\n",
		cnt->discard, cnt->ubuffered, cnt->buf_reinserted);
#ifdef WLCNT
	bcm_bprintf(b, "txsetupreq=%d, txsetupresp=%d, txsetupcfm=%d\n",
		cnt->txsetupreq, cnt->txsetupresp, cnt->txsetupcfm);
	bcm_bprintf(b, "rxsetupreq=%d, rxdsetupresp=%d, rxsetupcfm=%d\n",
		cnt->rxsetupreq, cnt->rxdsetupresp, cnt->rxsetupcfm);
	bcm_bprintf(b, "txteardown=%d, rxteardown=%d\n",
		cnt->txteardown, cnt->rxteardown);
	bcm_bprintf(b, "txptireq=%d, txptiresp=%d\n",
		cnt->txptireq, cnt->txptiresp);
	bcm_bprintf(b, "rxptireq=%d, rxptiresp=%d\n",
		cnt->rxptireq, cnt->rxptiresp);
	bcm_bprintf(b, "txchswreq=%d, txchswresp=%d\n",
		cnt->txchswreq, cnt->txchswresp);
	bcm_bprintf(b, "rxchswreq=%d, rxchswresp=%d\n",
		cnt->rxchswreq, cnt->rxchswresp);
	bcm_bprintf(b, "discard=%d, ubuffered=%d, buf_reinserted=%d\n",
		cnt->discard, cnt->ubuffered, cnt->buf_reinserted);
#endif /* WLCNT */

	bcm_bprintf(b, "TDLS lookaside list is\n");
	for (i = 0; i < TDLS_LOOKASIDE_SIZE; i++) {
		if (tdls->lookaside[i].flags & TDLS_LOOKASIDE_INUSE) {
			bcm_bprintf(b, "%s: parent = %p, flags = 0x%02x, status = 0x%02x\n",
				bcm_ether_ntoa(&tdls->lookaside[i].ea, eabuf),
				tdls->lookaside[i].parent,
				tdls->lookaside[i].flags, tdls->lookaside[i].status);
			bcm_bprintf(b, "\tpckt_cnt = %d, rssi = %d, upd_time = %d\n",
				tdls->lookaside[i].pkt_cnt, tdls->lookaside[i].rssi,
				tdls->lookaside[i].upd_time);
		}
	}

	bcm_bprintf(b, "TDLS stations are\n");
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		wlc_tdls_scb_dump(tdls, scb, b);
	}

	return 0;
}

static void
wlc_tdls_scb_dump(void *context, struct scb *scb, struct bcmstrbuf *b)
{
	tdls_info_t *tdls = (tdls_info_t *)context;
	wlc_info_t *wlc = tdls->wlc;
	scb_tdls_t *scb_tdls;
	sta_info_t sta;
	int i, j = 0;
	char eabuf[ETHER_ADDR_STR_LEN];
	wlc_bsscfg_t *cfg = SCB_BSSCFG(scb);

	if (BSSCFG_IS_TDLS(cfg)) {
		wlc_tdls_t *tc = cfg->tdls;

		scb_tdls = SCB_TDLS(tdls, scb);
		ASSERT(scb_tdls);

		wlc_sta_info(wlc, cfg, &scb->ea, &sta, sizeof(sta_info_t));
		bcm_bprintf(b, "peer %d: %s\n", j, bcm_ether_ntoa(&scb->ea, eabuf));
		bcm_bprintf(b, "rx %d tx %d rssi %d flags 0x%x:",
		            sta.rx_ucast_pkts, sta.tx_pkts,
		            wlc_scb_rssi(scb), scb_tdls->flags);
		bcm_bprintf(b, "\n tdls_PMEnable=%s, tdls_PMAwake=%s, ps_pending = %s\n",
			tc->tdls_PMEnable ? "TRUE" : "FALSE",
			tc->tdls_PMAwake ? "TRUE" : "FALSE",
			scb_tdls->parent->tdls->ps_pending ? "TRUE" : "FALSE");
		bcm_bprintf(b, "base chanspec = 0x%x, cur_chanspec = 0x%x, "
			"target_chanspec = 0x%x\n", scb_tdls->base_chanspec,
			scb_tdls->cur_chanspec, scb_tdls->target_chanspec);
		bcm_bprintf(b, "switch_time = %d, switch_timeout = %d\n",
			scb_tdls->switch_time, scb_tdls->switch_timeout);
		bcm_bprintf(b, "up time = %d, heartbeat_missed = %d, heartbeat_countdown = %d\n",
			tc->up_time, scb_tdls->heartbeat_missed, scb_tdls->heartbeat_countdown);
		for (i = 0; i < tdls_scbflags_cnt; i++) {
			if (scb_tdls->flags & (1 << i))
				bcm_bprintf(b, " %s", tdls_scbflags_str[i]);
		}
		bcm_bprintf(b, "\n");

		/* should be selected from the set of regulatory classes supported by both peers */
		bcm_bprintf(b, " peer's regulatory class number: %d\n", scb_tdls->rclen);
		for (i = 0; i < scb_tdls->rclen; i++)
			bcm_bprintf(b, "%d ", scb_tdls->rclist[i]);

		bcm_bprintf(b, "\n");

		bcm_bprintf(b, "    parent bsscfg idx: %d\n", WLC_BSSCFG_IDX(scb_tdls->parent));

		j++;
	}
}
#endif /* BCMDBG */

#endif /* WLTDLS */
