/*
 * Common CCX function header
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
 * $Id: wlc_ccx.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_wlc_ccx_h_
#define	_wlc_ccx_h_

#ifdef CCX_SDK
/* header files from Cisco SDK */
#include "ccxCommon.h"
#include "ccxCliRep.h"
#include "ccxRM.h"
#include "ccxKM.h"
#include <ccxNL.h>
#include "ccxDiag.h"
#include "ccx_common.h"

#include <wlc_rm.h>

/* service capability */
#define CAP_VOICE	0x01
#define CAP_UNIDIR_VIDEO	0x02
#define CAP_BIDIR_VIDEO		0x04
#define CAP_GPS		0x08

/* CCX_ANTENNA_TYPE */
#define CCX_ANTENNA_UNKNOWN	7

/* CCX_NIC_SPECIFIC_EXTENSION and related structure header lengths */
#define CCX_TLV_HDR_LEN	OFFSETOF(IHV_CCX_TLV, value[0])
#define CCX_TLV_DATA_HDR_LEN	OFFSETOF(IHV_CCX_TLV_DATA, tlv[0])
#define CCX_NIC_SPEC_EXT_HDR_LEN	OFFSETOF(CCX_NIC_SPECIFIC_EXTENSION, tlvData)
#define CCX_NIC_SPEC_EXT_TOTAL_HDR_LEN	(CCX_NIC_SPEC_EXT_HDR_LEN + \
	CCX_TLV_DATA_HDR_LEN + CCX_TLV_HDR_LEN)

/* OID set, query and method request header lengths */
#define	CCX_SET_INFO_HDR_LEN	OFFSETOF(IHV_SET_INFO, InformationBuffer[0])
#define	CCX_QUERY_INFO_HDR_LEN	OFFSETOF(IHV_QUERY_INFO, InformationBuffer[0])
#define	CCX_METHOD_INFO_HDR_LEN	OFFSETOF(IHV_METHOD_INFO, InformationBuffer[0])

/* CCX_NEIGHBOR_LIST header length */
#define CCX_NEIGHBOR_LIST_HDR_LEN	OFFSETOF(CCX_NEIGHBOR_LIST, ccxElement[0])

#define CCX_PACKET_HDR_LEN	OFFSETOF(IHV_CCX_PACKET, packet[0])

/* forward declaration */
typedef struct wl_rxsts wl_rxsts_t;
#endif /* CCX_SDK */

#define WLC_NRMFRAMEHASH	16	/* # rm frame measurement hash buckets */
/* radio measurement frame report book-keeping data structure */
typedef struct wlc_ccx_rm_frm_elt {
	struct wlc_ccx_rm_frm_elt *next;
	struct ether_addr ta;		/* transmitter address */
	struct ether_addr bssid;	/* bssid transmitter belongs to */
	uint8	frames;			/* total number of frames */
	int	rssi_sum;		/* sum of rssi */
} wlc_ccx_rm_frm_elt_t;
#define CCX_RMPATHLOSS_CHANNELMAX	32
/* S60 related  pathloss measurement */
#ifdef BCMCCX
typedef struct ccx_rm_pl_data {
	/* Data from AP's Pathloss Measurment request */
	uint16	nbursts;	/* nbursts */
	uint16	burst_len;	/* burst len */
	uint16	burst_interval;	/* Burst Interval */
	uint16	duration;	/* Burst Duration */
	int8	req_txpower;	/* txpower */
	struct	ether_addr da;	/* Destination Multicast address */
	uint8	nchannels;	/* number of channels */
	uint8	channels[CCX_RMPATHLOSS_CHANNELMAX];	/* channel list */

	/* local data */
	uint16	cur_burst;	/* actvie burst */
	uint16	cur_burstlen;	/* active burst len */
	uint16	cur_chanidx;	/* current channel idx */
	uint16	seq_number;	/* seq number */

	/* cache the internal rm request */
	wlc_rm_req_t	req;
} ccx_rm_pl_data_t;

/* radio measurement book-keeping data structure */
struct wlc_ccx_rm {
	/* Beacon measurements */
	bool	scan_active;		/* true if measurement in progress */
	int	scan_dur;		/* TU, specified duration */
	wlc_bss_list_t	scan_results;
	/* Frame measurements */
	bool	frame_active;		/* true if frame measurement in progress */
	int	frame_dur;		/* TU, specified frame measurement duration */
	bool	promisc_org;		/* saved promisc. mode */
	bool	promisc_off;		/* promisc mode off req. pending during test */
	uint32	frm_elts;		/* total frame elements */
	wlc_ccx_rm_frm_elt_t	*frmhash[WLC_NRMFRAMEHASH];

	/* Pathloss Measurement */
	bool	pathloss_active; /* true if pathloss measurement active now */
	bool	pathloss_longrun; /* true if pathloss measurement will run in future */
	bool	cur_txpowerctrl;	/* true if the txpower control is done in hw */
	uint	cur_txpower; 		/* current transmit power */
	bool	cur_txpoweroverride;	/* current transmit power override */
	ccx_rm_pl_data_t pathloss_data;	/* pathloss request data */
	struct wl_timer *plm_burst_timer; /* pathloss measurment burst interval timer */
	struct wl_timer *plm_dur_timer;	/* pathloss measurment burst duration timer */
};
#endif /* BCMCCX */

#ifdef CCX_SDK
/* Management frame counters */
typedef struct wlc_ccx_mgmt_cnt {
	uint32	tkipmgmticverr;		/* dot11RSNAMgmtStatsTKIPICVErrors */
	uint32	tkipmgmtnoencrypterr;	/* dot11RSNAMgmtStatsTKIPNoEncryptrrors */
	uint32	tkipmgmtlocalmicfail;	/* dot11RSNAMgmtStatsTKIPLocalMICFailures */
	uint32	tkipmgmtmhdrerr;	/* dot11RSNAMgmtStatsTKIPMHDRErrors */
	uint32	tkipmgmtreplayerr;	/* dot11RSNAMgmtStatsTKIPReplaysErrors */
	uint32	ccmpmgmtdecrypterr;	/* dot11RSNAMgmtStatsCCMPDecryptErrors */
	uint32	ccmpmgmtnoenypterr;	/* dot11RSNAMgmtStatsCCMPNoEncryptErrors */
	uint32	ccmpmgmtreplayerr;	/* dot11RSNAMgmtStatsCCMPReplays */
	uint32	bcastmgmtdisassoc;	/* dot11RSNAStatsBroadcastDisassociateCount */
	uint32	bcastmgmtdeauth;	/* dot11RSNAStatsBroadcastDeauthenticateCount */
	uint32	bcastmgmtaction;	/* dot11RSNAStatsBroadcastActionFrameCount */
} wlc_ccx_mgmt_cnt_t;
#endif /* CCX_SDK */

#define CCX_ROAM_SCAN_CHANNELS	30	/* max roaming scan channels */
/* general ccx data structure */
struct wlc_ccx {
	uint		leap_start;	/* time instance of leap LEAP starting point */
	bool		leap_on;	/* flag for CCX leap */
	bool		fast_roam;	/* flag signalling use of AP channel list */
	uint		orig_reason;	/* original roam reason code(before fast roaming fails) */

	bool		ccx_v4_only;	/* v4 only flag */
	bool		rm;		/* CCX Radio Management enable */

	chanspec_t	ap_chanspec_list[CCX_ROAM_SCAN_CHANNELS];	/* List of channels provided
						 * by AP for fast roaming
						 */
	uint		ap_channel_num;		/* Number of valid channels in list */
	/* roaming stats - S56.4.3, S56.5.2.6 */
	uint32		roaming_start_time;	/* roam start time. low 32 bits of tsf */
	uint32		roaming_delay_tu;	/* time in TU from roam_start_time till
						 * association completion for the last
						 * successful association.
						 */
	uint32		roaming_count;		/* number of successful roams. */

	bool		ccx_network;	/* is ccx network */

#ifdef CCX_SDK
	bool		ccx_ihv;		/* ihv dll presents */
	uint8		*ccx_ihv_ies;	/* CCX IEs from IHV */
	uint		ccx_ihv_ies_len;	/* length of CCX IEs from IHV */
	bool		diag_mode;		/* diag mode */
	bool		frame_log;		/* frame log mode */
	bool		oidreq;		/* CCX oid request */

	/* ihv tx-ed pkt parameters */
	void		*ihv_txpkt;		/* tx-ed pkt */
	bool		ihv_txpkt_sent;	/* ihv pkt is sent */
	bool		ihv_txpkt_max_retries;	/* reached max retries */

	wlc_ccx_mgmt_cnt_t	mgmt_cnt;	/* management frame counters */
#endif /* CCX_SDK */
};

#ifdef CCX_SDK

#define CCX_MFP_TKIP_PRIO	0xff	/* tkip priority in CCX MFP */

/* flag for using IHV interface */
#define IHV_ENAB(ccx) ((ccx)->ccx_ihv && (ccx)->ccx_network)
#endif /* CCX_SDK */

/* Traffic Stream Metrics (TSM) parameters per AC use by CAC in CCX TSM
 * protocol and TSM reporting
 */
struct ccx_ts_metrics {
	uint16 msdu_lifetime;		/* msdu lifetime (TU) */
	bool ts_metrics;		/* TRUE = enable */
	uint16 ts_metrics_interval;	/* interval between IAPP message (TU) */
	uint16 ts_metrics_count;	/* count number of watchdog timeout period */
	uint16 cnt_delay10;		/* # of packet < 10 ms delay */
	uint16 cnt_delay20;		/* # of packet > 10 ms < 20 ms delay */
	uint16 cnt_delay40;		/* # of packet > 20 ms < 40 ms delay */
	uint16 cnt_delay;		/* # of packet > 40 ms delay */
	uint32 total_media_delay;	/* total media delay in usec */
	uint32 total_pkt_delay;		/* total packet delay usec */
	uint32 pkt_tx;			/* packet tx count */
	uint32 pkt_tx_ok;		/* packet tx successful count */
	uint32 last_tsf_timestamp;	/* buffer last packet tsf timestamp usec */
	uint8 tid;				/* traffic stream id */
	uint16 total_used_time;	/* total used time in measurement interval */
};

/* Traffic Stream Metrics constant defined by CCX spec */
#define CCX_CAC_VOICE_TID		0	/* Default for Voice TID */
#define CCX_CAC_VOICE_USER_PRIO		6	/* Default for Voice User Priority */
#define CCX_CAC_SIGNAL_TID		1	/* Default for Signal TID */
#define CCX_CAC_SIGNAL_USER_PRIO	4	/* Default for Signal User Priority */

/* defines used for quick compare */
#define CCX_CAC_TS_METRICS_IE		(CISCO_AIRONET_OUI"\x07")
#define CCX_CAC_TS_RATESET_IE		(CISCO_AIRONET_OUI"\x08")
#define CCX_CAC_MSDU_LIFETIME_IE	(CISCO_AIRONET_OUI"\x09")
#define CCX_CAC_OUI_TYPE_LEN		(DOT11_OUI_LEN + 1)

/* Traffic Stream Metrics interval */
#define CCX_TSMETRIC_INTERVAL_MIN	977	/* minimum interval TU (1 sec) */
#define CCX_TSMETRIC_INTERVAL_MAX	9770	/* maximum interval TU (10sec) */
#define CCX_TSMETRIC_INTERVAL_DEFAULT	4883	/* default interval TU (5 sec) */

/* Traffic Stream Metrics delay constants */
#define CCX_TSM_10MS_DELAY	10000	/* 10 ms delay */
#define CCX_TSM_20MS_DELAY	20000	/* 20 ms delay */
#define CCX_TSM_40MS_DELAY	40000	/* 40 ms delay */

/* exported externs */
#ifdef BCMCCX
extern ccx_t *wlc_ccx_attach(wlc_info_t *wlc);
extern void wlc_ccx_detach(ccx_t *ccx);
extern void wlc_ccx_on_join_adopt_bss(ccx_t *ccx);
extern void wlc_ccx_on_leave_bss(ccx_t *ccx);
extern void wlc_ccx_on_roam_start(ccx_t *ccx, uint reason);
extern int wlc_ccx_set_auth(ccx_t *ccx, int auth);
extern void wlc_ccx_iapp_roam_rpt(ccx_t *ccx);
extern void wlc_ccx_tx_pwr_ctl(ccx_t *ccx, uint8 *tlvs, int tlvs_len);
extern int wlc_ccx_chk_iapp_frm(ccx_t *ccxh, struct wlc_frminfo *f);
extern void wlc_ccx_update_BSS(ccx_t *ccx, uint32 rx_tsf_l, uint rx_phy,
	wlc_bss_info_t *BSS);
extern uint wlc_ccx_prune(ccx_t *ccx, wlc_bss_info_t *bi);
extern uint wlc_ccx_roam(ccx_t *ccx, uint roam_reason_code);
extern uint8 *wlc_ccx_append_ies(ccx_t *ccxh, uint8 *pbody, wlc_bss_info_t *bi, bool reassoc,
	uint32 wpa_auth, struct ether_addr *curea, wpa_ie_fixed_t* wpa_ie);
#ifdef CCX_SDK
extern int wlc_ccx_oid(ccx_t *ccx, void *wl_ctx, void *info_buf, uint inbuf_len,
	uint outbuf_Len, uint *bytes_written, uint *bytes_read, uint *bytes_needed,
	void *oid_request);
extern void wlc_ccx_ind_ndis_status(ccx_t *ccx, NDIS_STATUS status, void *status_buf, uint buf_len);
extern uint wlc_ccx_log_rx_frame(ccx_t *ccxh, uint8 *d11_pkt, uint d11_pkt_len, wl_rxsts_t *rxsts);
extern uint wlc_ccx_log_tx_frame(ccx_t *ccxh, uint8 *d11_pkt, uint d11_pkt_len, uint txstatus,
	bool max_retries, bool sent);
extern void wlc_ccx_mfp_set_key(ccx_t *ccx, wl_wsec_key_t *key);
extern int wlc_ccx_mfp_rx(ccx_t *ccx, wlc_d11rxhdr_t *wrxh,
	struct dot11_management_header *hdr, void *p, struct scb *scb);
extern int wlc_ccx_mfp_tx(ccx_t *ccx, void *p, struct scb *scb);
extern void wlc_ccx_mfp_tx_mod_len(ccx_t *ccx, struct scb *scb, uint *iv_len, uint *tail_len);
extern void wlc_ccx_mgmt_status_hdlr(ccx_t *ccx, struct ether_addr* bssid,
	int code, bool is_status, void *arg, int arg_len);
extern bool wlc_ccx_in_addts_delay(ccx_t *ccx, struct ether_addr *bssid);
extern void wlc_ccx_diag_assoc_overrides(ccx_t *ccx, struct dot11_assoc_req *assoc);
extern bool wlc_ccx_cckm_reassoc_key(ccx_t *ccxh, wl_wsec_key_t *key, bool mfp_key);
void* wlc_ccx_frame_get_mgmt(ccx_t *ccxh, uint16 fc, uint8 cat,
    const struct ether_addr *da, const struct ether_addr *sa,
    const struct ether_addr *bssid, uint body_len, uint8 **pbody);
#endif /* CCX_SDK */

/* radio measurement functions */
extern int wlc_ccx_rm_validate(ccx_t *ccx, chanspec_t cur_chanspec, wlc_rm_req_t *req);
extern int wlc_ccx_rm_begin(ccx_t *ccx, wlc_rm_req_t *req);
extern void wlc_ccx_rm_frm(ccx_t *ccx, wlc_d11rxhdr_t *wrxh, struct dot11_header *h);
extern void wlc_ccx_rm_frm_complete(ccx_t *ccx);
extern void wlc_ccx_rm_scan_complete(ccx_t *ccx);
extern void wlc_ccx_rm_pathloss_complete(ccx_t *ccx, bool force_done);

extern void wlc_ccx_rm_report(ccx_t *ccx, wlc_rm_req_t *req_block, int count);
extern void wlc_ccx_rm_end(ccx_t *ccx);
extern void wlc_ccx_rm_free(ccx_t *ccx);

extern bool wlc_ccx_is_droam(ccx_t *ccxh);

extern void wlc_cckm_calc_reassocreq_MIC(cckm_reassoc_req_ie_t *cckmie,
	struct ether_addr *bssid, wpa_ie_fixed_t *rsnie, struct ether_addr *cur_ea,
	uint32 rn, uint8 *key_refresh_key, uint32 WPA_auth);

/* exported data */
extern const uint8 ccx_rm_capability_ie[];
extern const uint8 ccx_version_ie[];
#endif /* BCMCCX */

/* CCX radio measurement */
#define WLC_RM_TYPE_BEACON_TABLE	0x10 /* Beacon table RM */
#define WLC_RM_TYPE_ACTIVE_SCAN		0x20 /* Active scan RM */
#define WLC_RM_TYPE_PASSIVE_SCAN	0x30 /* Passic scan RM */
#define WLC_RM_TYPE_FRAME		0x40 /* Frame measurement RM */
#define WLC_RM_TYPE_PATHLOSS		0x50 /* Pathloss measurement RM */

#define CCX_LEAP_ROGUE_DURATION	30	/* seconds */

/* CCXv4 S51 - L2 Roaming */
#define WLC_QBSS_LOAD_AAC_LEGACY_AP 	0 	/* AAC for AP without QBSS Load IE */
#define WLC_QBSS_LOAD_CHAN_FREE_LEGACY_AP 0	/* channel free scroe for AP withotu QBSS Load Ie */
#define WLC_CHANNEL_FREE_SORTING_RANGE	7 	/* pref score range for Channel Free Sorting */
#endif	/* _wlc_ccx_h_ */
