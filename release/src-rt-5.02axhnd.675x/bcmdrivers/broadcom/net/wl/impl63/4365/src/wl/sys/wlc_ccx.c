#ifdef BCMCCX

/**
 * @file
 * @brief
 * Common CCX functions
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
 * $Id: wlc_ccx.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [DongleCCX]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11_ccx.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>

#include <wlc_scb.h>
#include <bcmwpa.h>

#include <bcmcrypto/passhash.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/sha1.h>

#include <wlc_wpa.h>
#include <wlc_sup.h>
#include <bcmcrypto/ccx.h>
#include <bcmcrypto/bcmccx.h>
#include <wlc_rm.h>
#include <wlc_ccx.h>
#include <wlc_cac.h>
#include <wl_export.h>
#include <wlc_frmutil.h>

#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif /* WLOFFLD */

#ifdef CCX_SDK
/* XXX - consider creating wl_ndccx.c file and move CCX OID handlings,
 * CCX indications and other NDIS related interface into it
 */
#include <oidencap.h>
#if defined(NDIS)
#if (NDISVER >= 0x0630)
#include <wl_ndp2p.h>
#endif // endif
#if (NDISVER >= 0x0600)
#include <wl_oid.h>
#include <wl_ndis.h>
#else
#include <wl_oid5.h>
#include <wl_ndis5.h>
#endif /* (NDISVER >= 0x0600) */
#endif /* NDIS */
#endif /* CCX_SDK */

#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif // endif

#include <wlc_utils.h>
#include <wlc_pcb.h>

/* CCX radio measurement report */
typedef struct wlc_ccx_rm_rept {
	struct wlc_ccx_rm_rept *next;
	uint	len;
	uchar	data[CCX_RM_REP_DATA_MAX_LEN];
} wlc_ccx_rm_rept_t;

/* CCX directed roaming AP paramenters */
typedef struct wlc_ccx_droam {
	struct ether_addr bssid;	/* AP bssid */
	bool	rf_ie;			/* set if RF subelement presents */
	int8	min_rssi;		/* min. recv pwr in dBm required to associate with the AP */
	int8	ap_tx_pwr;		/* AP's tx power */
	uint8	roam_time;		/* transition time(in 0.1s) permmited in roam. not use */
	int8	roam_delta;		/* roam delta */
	int8	roam_trigger;	/* roam trigger */
} wlc_ccx_droam_t;

#define CCX_ROAM_AP_ELEMENTS	16	/* max roam AP elements */

/* CCXv5 S71 Interpretation of Status and Result Codes */
typedef struct wlc_ccx_status_row {
	uint8	status_row;		/* status row number from 1 to 5 defined in spec. */
	struct ether_addr	bssid;	/* bssid where status/reason code comes from */
	uint	wait_time;	/* delay time required in second */
	struct wlc_ccx_status_row *next;
} wlc_ccx_status_row_t;

#define CCX_MIN_RECON_WAIT	5	/* minumum wait time(in second) before reconnect
					 * to AP which returned error status in status row 2 to 5
					 */

#ifdef CCX_SDK
/* max number of ihv tx packets allowed in the driver
 * which is consistant with WLF_TX_PKT_IDX_MASK value
 */
#define IHV_TX_Q_SIZE	4
#endif /* CCX_SDK */

/* Use for setting auth to zero */
#define AUTH_MODE_MAGIC		0x10000
typedef struct wlc_ccx_info {
	struct wlc_ccx  pubci;	/* ccx public state(must be first field of ccx_info) */
	wlc_info_t	*wlc;
	wlc_pub_t	*pub;
	wlc_rm_req_state_t	*rm_state;

	wlc_ccx_rm_rept_t	*rm_reports;	/* linked list of ccx reports */

	uint		rm_limit;		/* do not perform non-home channel
						 * measurements with a duration long than
						 * this many TUs
						 */

	uint		disassoc_time;		/* CCX roam report: time since assoc */
	uint8		prev_channel;		/* CCX roam report: previous channel */
	struct ether_addr	prev_ap_mac;	/* CCX roam report: previous BSSID */

	uint		auth_mode;		/* auth mode to be set in msg */

	/* link test */
	bool		linktest_pending;	/* link test report packet pending */
	uint		linktest_txretry;	/* link test report packet retry times */
	uint16		linktest_frm_num;	/* link test frame number */

	/* directed roaming */
	wlc_ccx_droam_t droam_ap_list[CCX_ROAM_AP_ELEMENTS];	/* directed roam neighbor AP list */
	uint		droam_ap_num;		/* directed roam elements */
	bool		droam_rf_roam;		/* set if RF subelement roam parameters applied */

	/* TPC - transmit power control */
	uint		cur_qtxpwr;		/* tx power in qdbm before apply TPC */
	uint		tpc_qtxpwr;		/* TPC in qdbm */
	bool		tpc;			/* TPC flag */

	/* support for machine name in Aironet IE */
	char		staname[AIRONET_IE_MAX_NAME_LEN];
	char		apname[AIRONET_IE_MAX_NAME_LEN];

	uint32		wlan_assoc_reason;	/* iovar interface for association reason config */

#ifdef CCX_SDK
	uint8		capability;		/* capability for CCXv5 and above */
	wlc_ccx_status_row_t	*blocked_ap;	/* blocked ap list due to connection error status */
	void		*ihv_pkt_arg[IHV_TX_Q_SIZE];	/* ihv reserved argument(ReservedIHV) */
	uint		ihv_pkt_arg_idx;	/* index into ihv_pkt_arg[] */
	int			org_monitor;	/* orginal monitor mode */

	/* cached CCKM reassociation pairwise key */
	wl_wsec_key_t	cckm_p_key;
	bool		cckm_mfp_key;
	bool		cckm_p_key_cached;

	/* diag mode */
	uint8		org_listen_interval;	/* original listen interval value */
#endif /* CCX_SDK */
	uint32		rn;				/* seq number for external supplicant */
	uint8 key_refresh_key[CCKM_KRK_LEN]; /* krk for external supplicant */
} wlc_ccx_info_t;

#define	RMFRMHASHINDEX(id)		(((id)[3] ^ (id)[4] ^ (id)[5]) & (WLC_NRMFRAMEHASH - 1))

/* Extend max limitation for CCX S36: 8.7.2 */
#define WLC_CCX_DEF_RM_TM_LIMIT		100 /* default off-channel measurement limit in ms */

/* null packet sending interval in seconds for keep-alive */
#define	KEEP_ALIVE_INTERVAL		10	/* default 10 seconds as Cisco suggested */

#ifdef CCX_SDK
#define DIAG_ASSOC_LISTEN	1	/* diag listen interval in assoc req */
#endif /* CCX_SDK */

#define WLC_CCX_IS_CURRENT_BSSID(cfg, bssid) \
	(!bcmp((char*)(bssid), (char*)&((cfg)->BSSID), ETHER_ADDR_LEN))

/* local functions */
static int wlc_ccx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_ccx_down(void *hdl);
static void wlc_ccx_watchdog(void *hdl);
static void wlc_ccxv2_process_roam_iapp(wlc_ccx_info_t *ci, ccx_roam_ap_ie_t *ap_ie,
	uint len);
static void wlc_ccx_process_roam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie,
	uint len, bool append);
static void wlc_ccx_process_droam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie,
	uint len);

static bool wlc_ccx_rf_ie_good(wlc_ccx_info_t *ci, wlc_ccx_droam_t *droam_ap, wlc_bss_info_t *bi);
static void wlc_ccx_process_droam_rf_ie(wlc_ccx_info_t *ci, ccx_radio_param_subie_t *rf_ie,
	int idx, bool curr_ap);
static int16 wlc_ccx_get_min_rssi(wlc_ccx_info_t *ci, int8 ap_txpwr, int8 ap_rssi,
	uint8 ap_channel);
static void wlc_ccx_send_roam_rpt(wlc_ccx_info_t *ci, uint8 *ssid, uint8 ssid_len, uint8 channel,
	struct ether_addr *bssid, uint disassoc_time);
static uint8 wlc_ccx_get_roam_reason(wlc_ccx_info_t *ci);
static void wlc_ccx_send_linktest_rpt(wlc_ccx_info_t *ci, struct ether_addr* da,
	struct ether_addr* sa, ccx_link_test_t* req_pkt, int req_len, int rssi);
#ifdef CCX_SDK
static void wlc_ccx_feature_info(wlc_ccx_info_t *ci, uint8 *ie_body, uint ie_len);
static void wlc_ccx_update_blocked_ap_list(wlc_ccx_info_t *ci);
static void wlc_ccx_ind_iapp_pkt(wlc_ccx_info_t *ci, struct dot11_header *d11_pkt,
	uint d11_pkt_len, wlc_pkttag_t *pkttag);
#ifdef BCMDBG
static const char *wlc_ccx_oid2str(uint oidval);
static const char *wlc_ccx_tlvcode2str(uint tlv_code);
#endif /* BCMDBG */
#endif /* CCX_SDK */

/* local radio measurement functions */
static void wlc_ccx_rm_recv(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	uint16 rx_time, ccx_rm_req_t* req_pkt, int req_len);
static void wlc_ccx_rm_parse_requests(wlc_ccx_info_t *ci, ccx_rm_req_ie_t* ie,
	int len, wlc_rm_req_t* req, int count);
static int wlc_ccx_rm_state_init(wlc_ccx_info_t *ci, bool bcast, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len);
static void wlc_ccx_rm_scan_begin(wlc_ccx_info_t *ci, int type, uint32 dur);
static void wlc_ccx_rm_frm_begin(wlc_ccx_info_t *ci, uint32 dur);
static int wlc_ccx_rm_ie_count(ccx_rm_req_ie_t* ie, int len);
static ccx_rm_req_ie_t* wlc_ccx_rm_next_ie(ccx_rm_req_ie_t* ie, int* len);
static wlc_ccx_rm_rept_t* wlc_ccx_rm_new_report(wlc_ccx_info_t *ci, uchar** buf, int* avail_len);
static void wlc_ccx_rm_append_report_header(uint8 type, uint16 token, uint8 flags, int data_len,
	uchar* buf);
static int wlc_ccx_rm_append_load_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar** bufptr,
	int *buflen);
static int wlc_ccx_rm_append_noise_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar** bufptr,
	int *buflen);
static int wlc_ccx_rm_append_frame_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr,
	int *buflen);
static int wlc_ccx_rm_beacon_table(wlc_bss_info_t **results, uint num, wlc_rm_req_t *req,
	uchar **bufptr, int *buflen);
static int wlc_ccx_rm_beacon_table_entry(wlc_bss_info_t *bi, uint8 channel, uint16 dur, uchar *buf,
	int buflen);
static void wlc_ccx_rm_frm_list_free(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_send_measure_report(wlc_ccx_info_t *ci, struct ether_addr *da,
	uint16 token, uint8 *report, uint report_len);
#ifdef BCMDBG
static void wlc_ccx_rm_req_dump(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	ccx_rm_req_t* req, int req_len);
static void wlc_ccx_rm_req_ie_dump(wlc_ccx_info_t *ci, ccx_rm_req_ie_t *ie, int buflen);
#endif /* BCMDBG */

/* pathloss measurement related routines */
static void wlc_ccx_rm_pathloss_begin(wlc_ccx_info_t *ci, wlc_rm_req_t* req);
static void wlc_ccx_rm_pathloss(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_send_pathlossmeasure_frame(wlc_ccx_info_t *ci);
static void wlc_ccx_rm_pathloss_pktcallback(wlc_info_t *wlc, uint txstatus, void *arg);
static void wlc_ccx_pathloss_burst_timer(void *arg);
static void wlc_ccx_pathloss_dur_timer(void *arg);

/* IE mgmt */
static uint wlc_ccx_calc_ihv_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_ihv_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_ver_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_ver_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_rm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_rm_cap_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_cckm_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_cckm_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ccx_calc_aironet_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ccx_write_aironet_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_ccx_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_ver_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_scan_parse_qbss_load_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_bcn_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ccx_bcn_parse_qos_ie(void *ctx, wlc_iem_parse_data_t *data);

const uint8 ccx_rm_capability_ie[] = {
	DOT11_MNG_PROPR_ID, 6,
	0x00, 0x40, 0x96, 0x01,
	0x01, 0x00
};
const uint8 ccx_version_ie[] = {
	DOT11_MNG_PROPR_ID, 5,
	0x00, 0x40, 0x96, 0x03,
	0x04 /* version */
};

static const uint8 CCKM_info_element[] = {
	DOT11_MNG_CCKM_REASSOC_ID, 0x18, 0x00, 0x40, 0x96, 00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* iovar table */
enum {
	IOV_CCX_RM,
	IOV_CCX_RM_LIMIT,
	IOV_CCX_AUTH_MODE,
	IOV_STA_NAME,
	IOV_AP_NAME,
	IOV_CCX_VERSION,
	IOV_WLAN_ASSOC_REASON,
	IOV_TESTS60,
	IOV_CCX_IHV,
	IOV_CCX_ENABLE,
	IOV_CCX_V4_ONLY,
	IOV_CCKM_KRK,
	IOV_CCKM_RN

};

static const bcm_iovar_t ccx_iovars[] = {
	{"ccx_rm", IOV_CCX_RM, (0), IOVT_BOOL, 0},
	{"ccx_rm_limit", IOV_CCX_RM_LIMIT, (IOVF_WHL), IOVT_UINT32, 0},
	{"ccx_auth_mode", IOV_CCX_AUTH_MODE, (IOVF_WHL), IOVT_INT32, 0},
	{"staname", IOV_STA_NAME, (0), IOVT_BUFFER, 0},
	{"apname", IOV_AP_NAME, (0), IOVT_BUFFER, 0},
	{"ccx_version", IOV_CCX_VERSION, (0), IOVT_UINT32, 0},
	{"wlan_assoc_reason", IOV_WLAN_ASSOC_REASON, (0), IOVT_UINT32, 0},
#ifdef BCMDBG
	{"test_s60", IOV_TESTS60, (0), IOVT_BUFFER, 0},
#endif // endif
#ifdef CCX_SDK
	{"ccx_ihv", IOV_CCX_IHV, (0), IOVT_BOOL, 0},
#endif /* CCX_SDK */
	{"ccx_enable", IOV_CCX_ENABLE, (0), IOVT_BOOL, 0},
	{"ccx_v4_only", IOV_CCX_V4_ONLY, (0), IOVT_BOOL, 0},
	{"cckm_krk", IOV_CCKM_KRK, (IOVF_SET_UP), IOVT_BUFFER, 0 },
	{"cckm_rn", IOV_CCKM_RN, (IOVF_SET_UP), IOVT_UINT32, 0 },

	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

ccx_t*
BCMATTACHFN(wlc_ccx_attach)(wlc_info_t *wlc)
{
	wlc_ccx_info_t *ci;
	wlc_rm_req_state_t *rm;
	wlc_pub_t *pub = wlc->pub;
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 rmcapfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_PROBE_REQ);
	uint16 arsfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

	WL_TRACE(("wl: %s\n", __FUNCTION__));

	if ((ci = (wlc_ccx_info_t *)MALLOCZ(pub->osh, sizeof(wlc_ccx_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			pub->unit, __FUNCTION__, MALLOCED(pub->osh)));
		return NULL;
	}

	ci->wlc = wlc;
	ci->pub = pub;
	ci->rm_state = wlc->rm_info->rm_state;
	rm = ci->rm_state;

	/* allocate memory for CCX radio measurement variables */
	if ((rm->ccx = (ccx_rm_t *)MALLOCZ(wlc->osh, sizeof(ccx_rm_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto error;
	}

	if (!(rm->ccx->plm_burst_timer = wl_init_timer(wlc->wl,
		wlc_ccx_pathloss_burst_timer, wlc, "ccx_pathloss"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for pathloss burst failed\n",
			pub->unit, __FUNCTION__));
		goto error;
	}
	if (!(rm->ccx->plm_dur_timer = wl_init_timer(wlc->wl, wlc_ccx_pathloss_dur_timer, wlc,
	                                             "ccx_plm_dur"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for pathloss duartion failed\n",
			pub->unit, __FUNCTION__));
		goto error;
	}

	/* By default CCX is enabled. */
	pub->_ccx = TRUE;

#ifndef CCX_AP_KEEP_ALIVE_DISABLED
	wlc->pub->_ccx_aka = TRUE;
#else
	wlc->pub->_ccx_aka = FALSE;
#endif // endif

#ifndef CCX_SDK
	ci->pubci.ccx_v4_only = TRUE;
#endif // endif

	/* default off-channel measurement limit */
	ci->rm_limit = WLC_CCX_DEF_RM_TM_LIMIT;

	/* register module */
	if (wlc_module_register(pub, ccx_iovars, "ccx", ci, wlc_ccx_doiovar,
	                        wlc_ccx_watchdog, NULL, wlc_ccx_down)) {
		WL_ERROR(("wl%d: ccx wlc_module_register() failed\n", wlc->pub->unit));
		goto error;
	}

	/* register IE mgmt callbacks */
	/* calc/build */
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_AIRONET_ID,
	      wlc_ccx_calc_aironet_ie_len, wlc_ccx_write_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, aironet in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_CCX_IHV,
	      wlc_ccx_calc_ihv_ie_len, wlc_ccx_write_ihv_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, ihv in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_CCX_VER,
	      wlc_ccx_calc_ver_ie_len, wlc_ccx_write_ver_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, ver in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* assocreq/reassocreq/prbreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, rmcapfstbmp, WLC_IEM_VS_IE_PRIO_CCX_RM_CAP,
	      wlc_ccx_calc_rm_cap_ie_len, wlc_ccx_write_rm_cap_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn failed, rm cap ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* reassocreq */
	if (wlc_iem_add_build_fn(wlc->iemi, FC_REASSOC_REQ, DOT11_MNG_CCKM_REASSOC_ID,
	      wlc_ccx_calc_cckm_ie_len, wlc_ccx_write_cckm_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, cckm in reassocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* parse */
	/* assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arsfstbmp, DOT11_MNG_AIRONET_ID,
	                             wlc_ccx_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_AIRONET_ID,
	                             wlc_ccx_scan_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_QBSS_LOAD_ID,
	                             wlc_ccx_scan_parse_qbss_load_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, qbss load in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_CCX_VER,
	                                wlc_ccx_scan_parse_ver_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed, ver in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_AIRONET_ID,
	                         wlc_ccx_bcn_parse_aironet_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, aironet ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}
	if (wlc_iem_vs_add_parse_fn(wlc->iemi, FC_BEACON, WLC_IEM_VS_IE_PRIO_CCX_QOS,
	                            wlc_ccx_bcn_parse_qos_ie, ci) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed, qos ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto error;
	}

	ci->wlan_assoc_reason = (uint32)WL_WLAN_ASSOC_REASON_NORMAL_NETWORK;

	return (ccx_t *)ci;
error:
#ifndef BCMNODOWN
	wlc_ccx_detach((ccx_t *)ci);
#endif /* BCMNODOWN */
	return NULL;
}

void
BCMATTACHFN(wlc_ccx_detach)(ccx_t *ccxh)
{
	WL_TRACE(("wl: %s: ci = %p\n", __FUNCTION__, ccxh));

	if (ccxh) {
		wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

		wlc_module_unregister(ci->pub, "ccx", ccxh);
		if (ci->rm_state->ccx) {
			if (ci->rm_state->ccx->plm_burst_timer) {
				wl_free_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer);
				ci->rm_state->ccx->plm_burst_timer = NULL;
			}
			if (ci->rm_state->ccx->plm_dur_timer) {
				wl_free_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer);
				ci->rm_state->ccx->plm_dur_timer = NULL;
			}
			MFREE(ci->pub->osh, ci->rm_state->ccx, sizeof(ccx_rm_t));
			ci->rm_state->ccx = NULL;
		}
#ifdef CCX_SDK
		if (ci->pubci.ccx_ihv_ies)
			MFREE(ci->pub->osh, ci->pubci.ccx_ihv_ies, ci->pubci.ccx_ihv_ies_len);
		if (ci->blocked_ap) {
			wlc_ccx_status_row_t *p1, *p2;
			for (p1 = ci->blocked_ap; p1; p1 = p2) {
				p2 = p1->next;
				MFREE(ci->pub->osh, p1, sizeof(wlc_ccx_status_row_t));
			}
		}
#endif /* CCX_SDK */
		MFREE(ci->pub->osh, ci, sizeof(wlc_ccx_info_t));
	}
}

static int
wlc_ccx_down(void *hdl)
{
	wlc_ccx_info_t *ci = hdl;
	int callbacks = 0;

	if (ci->rm_state->ccx) {
		if (ci->rm_state->ccx->plm_burst_timer &&
			!wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer))
			callbacks++;
		if (ci->rm_state->ccx->plm_dur_timer &&
			!wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer))
			callbacks++;
	}

	return callbacks;
}

/* handling CCX related iovars */
static int
wlc_ccx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_ccx_info_t *ci = hdl;
	int32 int_val = 0;
	bool bool_val;
	int err = 0;
	int32 *ret_int_ptr = (int32 *)a;
	wlc_bsscfg_t *cfg;

	cfg = wlc_bsscfg_find_by_wlcif(ci->wlc, wlcif);
	ASSERT(cfg != NULL);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));
	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_CCX_RM):
		*ret_int_ptr = (int32)ci->pubci.rm;
		break;

	case IOV_SVAL(IOV_CCX_RM):
		ci->pubci.rm = bool_val;
		break;

	case IOV_GVAL(IOV_CCX_RM_LIMIT):
		*ret_int_ptr = (int32)ci->rm_limit;
		break;

	case IOV_SVAL(IOV_CCX_RM_LIMIT):
		ci->rm_limit = (uint)int_val;
		break;

	case IOV_GVAL(IOV_CCX_AUTH_MODE):
		*ret_int_ptr = ci->auth_mode;
		break;

	case IOV_SVAL(IOV_CCX_AUTH_MODE):
		ci->auth_mode = int_val;
		break;

	case IOV_GVAL(IOV_STA_NAME):
		/* This ioctl return the machine name string */
		if (alen < (int)(strlen(ci->staname)+1))
			err = BCME_BUFTOOSHORT;
		else
			strcpy((char *)a, ci->staname);
		break;

	case IOV_SVAL(IOV_STA_NAME):
		/*
		 * This ioctl sets the station name to given input string.
		 * If the input string length is longer than AIRONET_IE_MAX_NAME_LEN
		 * then the input string will be truncated.
		 */
		strncpy(ci->staname, (char*)a, AIRONET_IE_MAX_NAME_LEN);
		ci->staname[AIRONET_IE_MAX_NAME_LEN-1] = '\0';
		break;

	case IOV_GVAL(IOV_AP_NAME):
		/* This ioctl return the AP name string */
		if (alen < (int)(strlen(ci->apname)+1))
			err = BCME_BUFTOOSHORT;
		else
			strcpy((char*)a, ci->apname);
		break;

	case IOV_GVAL(IOV_CCX_VERSION):
		*ret_int_ptr = (int32)cfg->current_bss->ccx_version;
		break;

	case IOV_SVAL(IOV_WLAN_ASSOC_REASON):
		if (int_val < 0 || int_val > WL_WLAN_ASSOC_REASON_MAX) {
			err = BCME_RANGE;
			break;
		}

		ci->wlan_assoc_reason = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_WLAN_ASSOC_REASON):
		*ret_int_ptr = (int32)ci->wlan_assoc_reason;
		break;
#ifdef BCMDBG
	case IOV_SVAL(IOV_TESTS60):
		{
		wlc_ccx_rm_recv(ci, &ci->wlc->pub->cur_etheraddr,
			&cfg->BSSID, 100, (ccx_rm_req_t *)a, 28);
		}
		break;
#endif // endif

#ifdef CCX_SDK
	case IOV_GVAL(IOV_CCX_IHV):
		int_val = (int32)ci->pubci.ccx_ihv;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_CCX_IHV):
		ci->pubci.ccx_ihv = bool_val;
		break;
#endif /* CCX_SDK */

	case IOV_GVAL(IOV_CCX_ENABLE):
		int_val = (int32)ci->pub->_ccx;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_CCX_ENABLE):
		ci->pub->_ccx = bool_val;
		break;

	case IOV_SVAL(IOV_CCX_V4_ONLY):
		ci->pubci.ccx_v4_only = bool_val;
		break;

	case IOV_SVAL(IOV_CCKM_KRK):
		if (p == NULL || plen != CCKM_KRK_LEN) {
			err = BCME_BADARG;
			break;
		}
		bcopy(p, ci->key_refresh_key, CCKM_KRK_LEN);
		break;

#ifdef BCMDBG
	case IOV_GVAL(IOV_CCKM_KRK):

		if (a == NULL || alen < CCKM_KRK_LEN) {
			err = BCME_BADARG;
			break;
		}
		bcopy(ci->key_refresh_key, a, CCKM_KRK_LEN);
		break;
#endif /* BCMDBG */
	case IOV_GVAL(IOV_CCKM_RN):
		if (a == NULL || alen < sizeof(ci->rn)) {
			err = BCME_BADARG;
			break;
		}
		bcopy(&ci->rn, a, sizeof(ci->rn));
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* watchdog */
static void wlc_ccx_watchdog(void *hdl)
{
	wlc_ccx_info_t *ci = hdl;
#if defined(BCMINTSUP)
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
#endif /* BCMINTSUP */

	(void)ci;

	if (!CCX_ENAB(ci->pub))
		return;

#if defined(BCMINTSUP)
	if (SUP_ENAB(wlc->pub) && BSSCFG_STA(cfg) && wlc_sup_getleapauthpend(wlc->ccxsup, cfg) &&
	    ((wlc->pub->now - wlc->ccx->leap_start) > CCX_LEAP_ROGUE_DURATION)) {
		wlc_ccx_rogue_timer(wlc->ccxsup, cfg, &cfg->prev_BSSID);
	}
#endif /* BCMINTSUP */

#ifdef CCX_SDK
		/* update block ap list */
		if (ci->blocked_ap)
			wlc_ccx_update_blocked_ap_list(ci);
#endif /* CCX_SDK */
}

bool
wlc_ccx_is_droam(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

	return ci->droam_rf_roam;
}

void
wlc_ccx_on_join_adopt_bss(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	struct wlc_ccx *pubci = &ci->pubci;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	bool rf_info_avail = FALSE;
	uint i = 0;

	if (pubci->ccx_network) {
		/* search if rf info available for this BSS */
		for (; i < ci->droam_ap_num; i++) {
			if (!bcmp((char*)&ci->droam_ap_list[i].bssid,
				(char*)&cfg->target_bss->BSSID, ETHER_ADDR_LEN) &&
				ci->droam_ap_list[i].rf_ie) {
				rf_info_avail = TRUE;
				break;
			}
		}
	}

	if (rf_info_avail) {
		wlcband_t *band = wlc->band;
		/* apply roam parameters in RF subelement */
		band->roam_trigger = ci->droam_ap_list[i].roam_trigger;
		/* roam delta is no less than 10 db */
		band->roam_delta = MAX(ci->droam_ap_list[i].roam_delta, 10);
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[OTHERBANDUNIT(wlc)];
			band->roam_trigger = ci->droam_ap_list[i].roam_trigger;
			band->roam_delta = ci->droam_ap_list[i].roam_delta;
		}
		ci->droam_rf_roam = TRUE;
	} else {
		if (!pubci->ccx_network) {
			/* clear existing AP channel list */
			pubci->ap_channel_num = 0;
			/* clear existing directed roaming AP list */
			ci->droam_ap_num = 0;
		}
		/* clear AP RF information flag */
		ci->droam_rf_roam = FALSE;
	}

	/* clear CCX fast roam indication */
	pubci->fast_roam = FALSE;

	/* CCXv4 S61 */
	/* default keep alive value */
	if (pubci->ccx_network)
		wlc_ap_keep_alive_count_update(wlc, KEEP_ALIVE_INTERVAL);

	/* reset link test frame number */
	ci->linktest_frm_num = 0;

#ifdef CCX_SDK
	if (IHV_ENAB(pubci)) {
		/* initialize counters */
		WLCNTSET(ci->pubci.mgmt_cnt.bcastmgmtdisassoc, 0);
		WLCNTSET(ci->pubci.mgmt_cnt.bcastmgmtdeauth, 0);
		WLCNTSET(ci->pubci.mgmt_cnt.bcastmgmtaction, 0);

		/* set key if cached and is for current bssid */
		if (ci->cckm_p_key_cached &&
		    !bcmp(&cfg->target_bss->BSSID, &ci->cckm_p_key.ea, ETHER_ADDR_LEN)) {
			if (wlc_ioctl(ci->wlc, WLC_SET_KEY, &ci->cckm_p_key,
				sizeof(ci->cckm_p_key), NULL))
				WL_ERROR(("wl%d: wlc_ccx_on_join_adopt_bss: error set"
				" pairwise key\n", ci->pub->unit));
			else if (ci->cckm_mfp_key)
				wlc_ccx_mfp_set_key((ccx_t *)ci, &ci->cckm_p_key);
		}
		ci->cckm_p_key_cached = FALSE;
		ci->cckm_mfp_key = FALSE;
	}
#endif /* CCX_SDK */

	/* Save the information regarding the previous BSS to send to the new AP */
	WL_WSEC(("wl%d: wlc_ccx_on_join_adopt_bss(): building roam IAPP report\n",
		ci->pub->unit));
	bcopy(cfg->prev_BSSID.octet, ci->prev_ap_mac.octet, ETHER_ADDR_LEN);
	ci->prev_channel = CHSPEC_CHANNEL(cfg->current_bss->chanspec);
	ci->disassoc_time = cfg->roam->time_since_bcn;
}

void
wlc_ccx_on_leave_bss(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	struct wlc_ccx *pubci = &ci->pubci;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (WSEC_CKIP_MIC_ENABLED(cfg->wsec) || WSEC_CKIP_KP_ENABLED(cfg->wsec)) {
		uint32 val = cfg->wsec;

		val &= ~(CKIP_MIC_ENABLED | CKIP_KP_ENABLED | WSEC_SWFLAG);
		if (wlc_wsec(wlc, cfg, val)) {
			WL_ERROR(("wl%d: %s: error clearing CKIP settings\n",
				ci->pub->unit, __FUNCTION__));
		}
	}

	/* If RF subelement roam parameters applied, restore roam parameters
	 * before leave BBS
	 */
	if (ci->droam_rf_roam) {
		wlcband_t *band = wlc->band;
		band->roam_trigger = band->roam_trigger_def;
		band->roam_delta = band->roam_delta_def;
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[OTHERBANDUNIT(wlc)];
			band->roam_trigger = band->roam_trigger_def;
			band->roam_delta = band->roam_delta_def;
		}
	}

	/* reset roaming variables */
	pubci->fast_roam = FALSE;

	/* if tpc applied */
	if (ci->tpc) {
		/* reset tx power if needed */
#if defined(WLTXPWR1_SIGNED)
		int8 cur_qtxpwr;
#else
		uint cur_qtxpwr;
#endif // endif
		ppr_t *txpwr;
		wlc_phy_t *pi = WLC_PI(ci->wlc);

		/* XXX MUST REVIEW MUST REVIEW. Doesn't this code set the power only
		 * if it is already that value? johnvb
		 */
		wlc_phy_txpower_get(pi, &cur_qtxpwr, NULL);
		if (cur_qtxpwr == ci->tpc_qtxpwr) {
			if ((txpwr = ppr_create(wlc->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
				return;
			}

			wlc_channel_reg_limits(wlc->cmi, wlc->chanspec, txpwr);
			ppr_apply_max(txpwr, WLC_TXPWR_MAX);
			/* restore tx power */
			wlc_phy_txpower_set(pi, ci->cur_qtxpwr, FALSE, txpwr);
			ppr_delete(wlc->osh, txpwr);
		}
		ci->tpc = FALSE;
	}
	/* restore the original value of keep alive timeout */
	wlc_ap_keep_alive_count_default(ci->wlc);
}

int
wlc_ccx_chk_iapp_frm(ccx_t *ccxh, struct wlc_frminfo *f)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_iapp_hdr_t *iapp;
	uint iapp_len;
	uint16 iapp_id;
	uint data_len;

#ifdef CCX_SDK
	iapp = (ccx_iapp_hdr_t *)((char *)f->pbody + DOT11_LLC_SNAP_HDR_LEN);
	iapp_len = f->body_len - DOT11_LLC_SNAP_HDR_LEN;
#else
	iapp = (ccx_iapp_hdr_t *)((char *)PKTDATA(ci->pub->osh, f->p) + ETHER_HDR_LEN +
		DOT11_LLC_SNAP_HDR_LEN);
	iapp_len = PKTLEN(ci->pub->osh, f->p) - (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);
#endif /* CCX_SDK */

	iapp_id = (ntoh16(iapp->id_len) & CCX_IAPP_ID_MASK) >> CCX_IAPP_ID_SHIFT;
	data_len = (ntoh16(iapp->id_len) & CCX_IAPP_LEN_MASK) - CCX_IAPP_HDR_LEN;

	/* validate id message type and length */
	if (iapp_id != CCX_IAPP_ID_CONTROL || iapp_len < (CCX_IAPP_HDR_LEN + data_len))
		return 0;

	switch (iapp->type) {
	case CCX_IAPP_TYPE_ROAM:
#ifdef CCX_SDK
		if (IHV_ENAB(&ci->pubci))
			goto ccx_ihv;
#endif /* CCX_SDK */
		if (iapp->subtype == CCX_IAPP_SUBTYPE_ROAM_REP) {
			WL_ASSOC(("ROAM: CCX FAST, received adjacent AP list IAPP frame\n"));
			wlc_ccx_process_roam_iapp(ci, (ccx_neighbor_rept_ie_t*)iapp->data,
				data_len, FALSE);
			return -1;
		} else if (iapp->subtype == CCX_IAPP_SUBTYPE_ROAM_REQ) {
			/* must be unicast packet for CCX directed roam request */
			if (bcmp(f->da, (char*)&ci->pub->cur_etheraddr, ETHER_ADDR_LEN))
				break;
			WL_ASSOC(("ROAM: CCX FAST, received directed roam IAPP frame\n"));
			wlc_ccx_process_droam_iapp(ci, (ccx_neighbor_rept_ie_t*)iapp->data,
				data_len);
			return -1;
		}
		break;
	case CCXv2_IAPP_TYPE_ROAM:
#ifdef CCX_SDK
		if (IHV_ENAB(&ci->pubci))
			goto ccx_ihv;
#endif /* CCX_SDK */
		/* must be unicast packet for CCX roam request */
		if (bcmp(f->da, (char*)&ci->pub->cur_etheraddr, ETHER_ADDR_LEN))
			break;
		if (iapp->subtype == CCXv2_IAPP_SUBTYPE_ROAM_REQ) {
			WL_ASSOC(("ROAM: CCX FAST, received adjacent AP list IAPP frame(v2)\n"));
			wlc_ccxv2_process_roam_iapp(ci, (ccx_roam_ap_ie_t*)iapp->data, data_len);
			return -1;
		}
		break;
	case CCX_IAPP_TYPE_RM:
		if (ci->pubci.rm && (iapp->subtype == CCX_IAPP_SUBTYPE_REQ)) {
#ifdef CCX_SDK
			if (data_len >= (CCX_RM_REQ_LEN + CCX_RM_REQ_IE_FIXED_LEN)) {
				ccx_rm_req_ie_t *rm_ie =
					(ccx_rm_req_ie_t*)((ccx_rm_req_t*)iapp->data)->data;
				if (IHV_ENAB(&ci->pubci) && rm_ie->type == CCX_RM_TYPE_STATISTICS)
					goto ccx_ihv;
			}
#endif /* CCX_SDK */
			/* looks like a valid radio mgmt request packet */
			wlc_ccx_rm_recv(ci, (struct ether_addr*)f->da,
				(struct ether_addr*)f->sa,
				f->rxh->RxTSFTime,
				(ccx_rm_req_t*)iapp->data,
				data_len);
			return -1;
		}
		break;
	case CCX_IAPP_TYPE_LINK_TEST:
		if (iapp->subtype == CCX_IAPP_SUBTYPE_REQ) {
			/* looks like a valid link test request packet */
			wlc_ccx_send_linktest_rpt(ci,
				(struct ether_addr*)f->sa, /* it is dst address */
				(struct ether_addr*)f->da, /* it is src address */
				(ccx_link_test_t*)iapp->data,
				data_len,
				wlc_lq_rssi_pktrxh_cal(ci->wlc, f->wrxh));
			return -1;
		}
		break;
	default:
#ifdef CCX_SDK
		if (IHV_ENAB(&ci->pubci))
			/* indicate the IAPP frame to ihv */
			wlc_ccx_ind_iapp_pkt(ci, f->h, PKTLEN(ci->pub->osh, f->p),
				PKTTAG(f->p));
#endif /* CCX_SDK */
		break;
	}

	return 0;

#ifdef CCX_SDK
ccx_ihv:
	/* indicate the IAPP frame to ihv */
	wlc_ccx_ind_iapp_pkt(ci, f->h, PKTLEN(ci->pub->osh, f->p), PKTTAG(f->p));
	return -1;
#endif /* CCX_SDK */
}

void
wlc_ccx_on_roam_start(ccx_t *ccxh, uint reason)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	/* clear previous CKIP settings (facilitating shared key!) */
	if (WSEC_CKIP_MIC_ENABLED(cfg->wsec) || WSEC_CKIP_KP_ENABLED(cfg->wsec)) {
		uint32 val = cfg->wsec;
#ifdef BCMDBG_ERR
		const char *msg_name = (reason == WLC_E_DEAUTH_IND)? "deauth" : "disassoc";
#endif /* BCMDBG_ERR */
		val &= ~(CKIP_MIC_ENABLED | CKIP_KP_ENABLED | WSEC_SWFLAG);

		if (wlc_wsec(wlc, cfg, val)) {
			WL_ERROR(("wl%d: %s: error clearing CKIP settings on "
				"receipt of %s\n", wlc->pub->unit, __FUNCTION__, msg_name));
		}
	}

	/* issue a CCX fast roam scan request if roam not disabled */
	if (!cfg->roam->off && ci->pubci.ap_channel_num) {
		ci->pubci.fast_roam = TRUE;
		WL_ASSOC(("wl%d: %s: ROAM: fast_roam begins\n", wlc->pub->unit, __FUNCTION__));
	}
}

static void
wlc_ccx_linktest_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)arg;
	uint tx_frame_count = (txstatus & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT;
	uint txretry = (uint)((tx_frame_count > 1) ? (tx_frame_count - 1) : 0);

	if (!ci->linktest_pending) {
		return;
	}

	ci->linktest_pending = FALSE;
	/* save retries when send link test frame */
	ci->linktest_txretry = txretry;
}

int
wlc_ccx_set_auth(ccx_t *ccxh, int auth)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;

	/* LEAP needs its trick auth mode in the association
	 * request, but NDIS could be discomfitted by seeing
	 * that value.  Use it here, but hide it otherwise.
	 */
	if (ci->pubci.leap_on)
		auth = DOT11_LEAP_AUTH;
	/* auth mode will be set for PEAP/GTC */
	if (ci->auth_mode) {
		if (ci->auth_mode == AUTH_MODE_MAGIC || !ci->pubci.ccx_network)
			auth = 0;
		else
			auth = ci->auth_mode;
	}

	return auth;
}

void
wlc_ccx_iapp_roam_rpt(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_bsscfg_t *cfg = ci->wlc->cfg;

	wlc_ccx_send_roam_rpt(ci, cfg->current_bss->SSID,
		cfg->current_bss->SSID_len, ci->prev_channel,
		&ci->prev_ap_mac, ci->disassoc_time);
	ci->prev_channel = 0;
}

static void
wlc_ccx_send_roam_rpt(wlc_ccx_info_t *ci, uint8 *ssid, uint8 ssid_len, uint8 channel_prev_ap,
	struct ether_addr *bssid, uint disassoc_time)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	uint alloc_len;
	ccx_roam_iapp_pkt_t *iapp_pkt;
	ccx_roam_reason_ie_t *reason_ie;
	osl_t *osh, *p;
	uint16 iapp_len;
	uint16 ether_type;
	wlc_bsscfg_t *cfg = wlc->cfg;

	WL_WSEC(("wl%d: wlc_ccx_send_roam_rpt: report previous AP\n", ci->pub->unit));
	WL_ASSOC(("wl%d: %s: ROAM: CCX version %d AP\n",
		wlc->pub->unit, __FUNCTION__, cfg->current_bss->ccx_version));

	/*
	 * In pre-CCXv4, this reporting was not required if it is an initial association.
	 * CCXv4 requires reason code generation in all association. Since initial association
	 * does not have previous AP by definition, faked information is supplied in previous AP
	 * infon field.
	 */

	if ((!channel_prev_ap) && (cfg->current_bss->ccx_version < 4)) {
		WL_ASSOC(("wl%d: %s: ROAM: skip roam report for initial assoc pre-V4 AP\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}
	else {
		WL_ASSOC(("wl%d: %s: ROAM: sending roam report\n", wlc->pub->unit, __FUNCTION__));
	}

	osh = ci->pub->osh;

	alloc_len = sizeof(ccx_roam_iapp_pkt_t);
	/* iapp length field excludes snap */
	iapp_len = CCX_ROAM_IAPP_MSG_SIZE - sizeof(struct dot11_llc_snap_header);
	ether_type = CCX_ROAM_IAPP_MSG_SIZE;

	/* add space for reason ie for CCX version larger or equal to 4 */
	if (cfg->current_bss->ccx_version >= 4) {
		alloc_len += sizeof(ccx_roam_reason_ie_t);
		iapp_len += sizeof(ccx_roam_reason_ie_t);
		ether_type += sizeof(ccx_roam_reason_ie_t);
	}

	if ((p = PKTGET(osh, alloc_len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n", ci->pub->unit,
			__FUNCTION__, alloc_len));
		WLCNTINCR(ci->pub->_cnt->txnobuf);
		return;
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, alloc_len);

	iapp_pkt = (ccx_roam_iapp_pkt_t *)PKTDATA(osh, p);

	/* clear prev. AP info */
	bzero((char *)&iapp_pkt->ap_ie, sizeof(ccx_roam_ap_ie_t));

	bcopy((char *)&cfg->BSSID, (char *)&iapp_pkt->eth.ether_dhost, ETHER_ADDR_LEN);
	bcopy((char *)&ci->pub->cur_etheraddr, (char *)&iapp_pkt->eth.ether_shost,
	      ETHER_ADDR_LEN);
	iapp_pkt->eth.ether_type = hton16(ether_type);

	bcopy(CISCO_AIRONET_SNAP, (char *)&iapp_pkt->snap, DOT11_LLC_SNAP_HDR_LEN);

	iapp_pkt->msg_len = hton16(iapp_len);
	iapp_pkt->msg_type = CCXv2_IAPP_TYPE_ROAM;
	iapp_pkt->fcn_code = 0;
	bcopy((char *)&cfg->BSSID, (char *)&iapp_pkt->dest_mac, ETHER_ADDR_LEN);
	bcopy((char *)&ci->pub->cur_etheraddr, (char *)&iapp_pkt->src_mac, ETHER_ADDR_LEN);

	/* fill Adjacent AP Report portion for previous AP */
	iapp_pkt->ap_ie.tag = hton16(CCX_ROAM_ADJ_AP_TAG);
	iapp_pkt->ap_ie.len = hton16(48);
	bcopy(CISCO_AIRONET_OUI, iapp_pkt->ap_ie.oui, DOT11_OUI_LEN);
	/* iapp_pkt->ap_ie.ver = 0; it is not necessary due to bzero above */

	/* Fill previous AP info for roaming */
	if (channel_prev_ap) {
		bcopy((char *)bssid, (char *)&iapp_pkt->ap_ie.mac, ETHER_ADDR_LEN);
		iapp_pkt->ap_ie.channel = hton16(channel_prev_ap);
		iapp_pkt->ap_ie.ssid_len = hton16(ssid_len);
		bcopy((char *)ssid, (char *)iapp_pkt->ap_ie.ssid, ssid_len);
		iapp_pkt->ap_ie.disassoc_time = hton16((uint16)disassoc_time);
	}

	/* add reason ie for CCX version 4 or larger */
	if (cfg->current_bss->ccx_version >= 4) {
		reason_ie = (ccx_roam_reason_ie_t *)(iapp_pkt + 1);
		reason_ie->tag = hton16(CCX_ROAM_REASON_TAG);
		reason_ie->len = hton16(sizeof(ccx_roam_reason_ie_t) - 4);
		bcopy(CISCO_AIRONET_OUI, reason_ie->oui, DOT11_OUI_LEN);
		reason_ie->ver = 0;
		reason_ie->reason = wlc_ccx_get_roam_reason(ci);
	}

	wlc_sendpkt(ci->wlc, p, cfg->wlcif);
}

/* convert driver roaming reason to CCX roaming reason */
static uint8
wlc_ccx_get_roam_reason(wlc_ccx_info_t *ci)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	uint8 reason, roam_reason;

	roam_reason = (cfg->roam->reason == WLC_E_REASON_FAST_ROAM_FAILED) ?
		ci->pubci.orig_reason : cfg->roam->reason;

	switch (roam_reason) {
	case 0:				/* no roam reason - initial association */
		if (ci->wlan_assoc_reason != (uint32)WL_WLAN_ASSOC_REASON_NORMAL_NETWORK) {
			reason = CCX_ROAM_IN_NET; /* test plan 4.4.5 */
		}
		else {
			reason = CCX_ROAM_FIRST_ASSOC; /* test plan 4.4.4 */
		}
		break;
	case WLC_E_REASON_LOW_RSSI:	/* roamed due to low RSSI */
		reason = CCX_ROAM_NORMAL;
		break;
	case WLC_E_REASON_DEAUTH:	/* roamed due to DEAUTH indication */
	case WLC_E_REASON_DISASSOC:	/* roamed due to DISASSOC indication */
	case WLC_E_REASON_BCNS_LOST:	/* roamed due to lost beacons */
		reason = CCX_ROAM_LINK_DOWN;
		break;
	case WLC_E_REASON_DIRECTED_ROAM:	/* roamed due to request by AP */
		reason = CCX_ROAM_DIRECTED_ROAM;
		break;
	case WLC_E_REASON_TSPEC_REJECTED:	/* roamed due to TSPEC rejection */
		reason = CCX_ROAM_AP_INCAPACITY;
		break;
	case WLC_E_REASON_BETTER_AP:	/* roamed due to finding better AP */
		reason = CCX_ROAM_BETTER_AP;
		break;
	default:
		reason = CCX_ROAM_UNSPECIFIED;
		break;
	}

	WL_ASSOC(("wl%d: %s: ROAM: roam_reason 0x%x association reason 0x%x\n",
		wlc->pub->unit, __FUNCTION__, cfg->roam->reason, reason));
	return reason;
}

/* set as high priority as possible for CCX tx pkt */
static int
wlc_ccx_get_txpkt_prio(wlc_ccx_info_t *ci, struct ether_addr* da)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	struct scb *scb;
	int prio;

	if (!CAC_ENAB(ci->pub))
		return MAXPRIO;

	if ((scb = wlc_scbfindband(wlc, wlc_bsscfg_primary(wlc), da,
	                           CHSPEC_WLCBANDUNIT(wlc->home_chanspec)))) {
		for (prio = MAXPRIO; prio > PRIO_8021D_NONE; prio--) {
			if (WLC_CAC_NO_ADM_CTRL ==
				wlc_cac_is_traffic_admitted(wlc->cac, WME_PRIO2AC(prio), scb))
				return prio;
		}
	}

	return PRIO_8021D_BE;
}

/* CCXv4 S62 */
static void
wlc_ccx_send_linktest_rpt(wlc_ccx_info_t *ci, struct ether_addr* da,
	struct ether_addr* sa, ccx_link_test_t* req_pkt, int req_len, int rssi)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_llc_snap_header *snap;
	ccx_iapp_hdr_t* ccx_iapp;
	ccx_link_test_t *link_resp;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	int prio;
	wlc_bsscfg_t *cfg = wlc->cfg;

	WL_INFORM(("wl%d: wlc_ccx_send_linktest_rpt: link test response\n", ci->pub->unit));

	/* make sure this is a valid CCX link test request packet */
	if (req_len < CCX_LINK_TEST_REQ_LEN) {
		WL_ERROR(("wl%d: %s: link test request packet too "
			"short, len = %d expected at least %d", ci->pub->unit, __FUNCTION__,
			req_len, CCX_LINK_TEST_REQ_LEN));
		return;
	}

	body_len = DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN + req_len;

	p = wlc_dataget(wlc, da,
		(uint16)body_len, /* 803.2 length_type-like field */
		sizeof(struct ether_header) + body_len); /* buffer includes ether_header */
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: wlc_dataget returns null\n",
			ci->pub->unit, __FUNCTION__));
		return;
	}

	pbody = (uint8*)PKTDATA(ci->pub->osh, p);

	snap = (struct dot11_llc_snap_header *)(pbody + ETHER_HDR_LEN);
	bcopy(CISCO_AIRONET_SNAP, snap, DOT11_LLC_SNAP_HDR_LEN);

	ccx_iapp = (ccx_iapp_hdr_t *)((int8*)snap + DOT11_LLC_SNAP_HDR_LEN);
	ccx_iapp->id_len = hton16(CCX_IAPP_ID_CONTROL | (body_len - DOT11_LLC_SNAP_HDR_LEN));
	ccx_iapp->type = CCX_IAPP_TYPE_LINK_TEST;
	ccx_iapp->subtype = CCX_IAPP_SUBTYPE_ROAM_REP;
	bcopy((char *)&cfg->BSSID, ccx_iapp->da.octet, ETHER_ADDR_LEN);
	bcopy(ci->pub->cur_etheraddr.octet, ccx_iapp->sa.octet, ETHER_ADDR_LEN);

	link_resp = (ccx_link_test_t *)((int8*)ccx_iapp + CCX_IAPP_HDR_LEN);

	/* If the rssi is negative, we have to make it positive. */
	if (rssi < 0)
		rssi = 0 - rssi;
	/* clamp link_margin value if we overflow an int8 */
	link_resp->rssi = MIN(rssi, 127);

	/* increment frame number */
	ci->linktest_frm_num++;
	/* report current frame number */
	link_resp->frm_num = hton16(ci->linktest_frm_num);
	/* last link test packet tx retries */
	link_resp->txretried = (uint8)ci->linktest_txretry;
	/* unchanged fields from request frame */
	link_resp->time = req_pkt->time;
	link_resp->rsq = req_pkt->rsq;
	link_resp->rss = req_pkt->rss;
	link_resp->sqp = req_pkt->sqp;
	link_resp->ssp = req_pkt->ssp;

	bcopy(req_pkt->data, link_resp->data, req_len - CCX_LINK_TEST_REQ_LEN);

	/* Set high priority so that the packet will get enqueued at the end of
	 * higher precedence queue in order
	 */
	prio = wlc_ccx_get_txpkt_prio(ci, da);
	PKTSETPRIO(p, prio);

	/* if new request come in before current response sent out,
	 * AP may think there is problem and timeout the previous request
	 */
	if (ci->linktest_pending) {
		WL_ERROR(("wl%d: %s: get a new link test request before "
			"current link test response sent out\n",
			ci->pub->unit, __FUNCTION__));
	}

	/* save packet pointer. */
	ci->linktest_pending = TRUE;
	/* clean retry count */
	ci->linktest_txretry = 0;

	wlc_pcb_fn_register(wlc->pcb, wlc_ccx_linktest_tx_complete, ci, p);
	wlc_sendpkt(ci->wlc, p, NULL);
}

/* CCXv2 Transmit Power Control, sec S31 */
void
wlc_ccx_tx_pwr_ctl(ccx_t *ccxh, uint8 *tlvs, int tlvs_len)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_phy_t *pi = WLC_PI(ci->wlc);
	ccx_cell_pwr_t *pwr_ie;

	/* search for the CCXv2 Cell Power Limit IE */
	pwr_ie = (ccx_cell_pwr_t*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_CELL_PWR_ID);

	/* 11h Power Constraint takes precedence, so search for the 11h Power Constraint IE
	 */
	if (pwr_ie && !bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_PWR_CONSTRAINT_ID)) {
		/* validate the pwr_ie */
		if (pwr_ie->len >= 5 && !bcmp(pwr_ie->oui, CISCO_AIRONET_OUI, 3) &&
			pwr_ie->ver == 0) {
#if defined(WLTXPWR1_SIGNED)
			int8 cur_qtxpwr;
#else
			uint cur_qtxpwr;
#endif // endif
			/* get current tx power */
			wlc_phy_txpower_get(pi, &cur_qtxpwr, NULL);
			if (ci->tpc && ci->tpc_qtxpwr == cur_qtxpwr)
				/* roaming case.  always use host-set tx power level
				 * in comparison
				 */
				cur_qtxpwr = ci->cur_qtxpwr;
			/* if host-set tx power level is higher than the limit */
			if ((uint32)(cur_qtxpwr) >
				((uint32)pwr_ie->power * WLC_TXPWR_DB_FACTOR)) {
				/* adjust tx power */
				ppr_t *txpwr;

				if ((txpwr = ppr_create(ci->wlc->osh,
					PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
					return;
				}

				wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
				ppr_apply_max(txpwr, WLC_TXPWR_MAX);
				/* restore tx power */
				wlc_phy_txpower_set(pi,
					pwr_ie->power * WLC_TXPWR_DB_FACTOR, FALSE, txpwr);
				ppr_delete(ci->wlc->osh, txpwr);
				ci->cur_qtxpwr = cur_qtxpwr;
				ci->tpc_qtxpwr = pwr_ie->power * WLC_TXPWR_DB_FACTOR;
				ci->tpc = TRUE;

			}
		}
	}
}

#ifdef CCX_SDK
static void
wlc_ccx_init_ckip(wlc_ccx_info_t *ci, bcm_tlv_t *AIE, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	uint32 val = cfg->wsec;

	/* clear previous CKIP settings */
	if (WSEC_CKIP_MIC_ENABLED(val) || WSEC_CKIP_KP_ENABLED(val)) {
		val &= ~(CKIP_MIC_ENABLED | CKIP_KP_ENABLED | WSEC_SWFLAG);
		if (wlc_wsec(wlc, cfg, val)) {
			WL_ERROR(("wl%d: %s: error clearing previous CKIP"
				" setting\n", ci->pub->unit, __FUNCTION__));
		}
	}

	/* CKIP mode */
	if (cfg->target_bss->ckip_support) {
		if (AIE && AIE->len > AIRONET_IE_CKIP) {
			uint8 ckip_neg;

			ckip_neg = AIE->data[AIRONET_IE_CKIP];

			WL_WSEC(("wl%d: %s: CCX: CKIP mode = 0x%x\n",
				ci->pub->unit, __FUNCTION__,
				ckip_neg & (CKIP_MIC | CKIP_KP)));
			val = cfg->wsec;
			if (ckip_neg & CKIP_MIC) {
				/* enable CKIP MIC */
				val |= CKIP_MIC_ENABLED | WSEC_SWFLAG;
			}
			if (ckip_neg & CKIP_KP) {
				/* enable CKIP KP */
				val |= CKIP_KP_ENABLED | WSEC_SWFLAG;
			}
			if (val & (CKIP_MIC_ENABLED | CKIP_KP_ENABLED)) {
				int i;

				/* apply new CKIP settings */
				if (wlc_wsec(wlc, cfg, val)) {
					WL_ERROR(("wl%d: %s: error applying new"
						" CKIP settings\n", ci->pub->unit, __FUNCTION__));
				}

				/* reinit CKIP SEQ for any static keys */
				for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
					wlc_key_t *key;
					key = wlc_keymgmt_get_bss_key(wlc->keymgmt, cfg,
						(wlc_key_id_t)i, NULL);
					(void)wlc_key_set_seq(key, NULL, 0,
						WLC_KEY_SEQ_ID_ALL, FALSE);
				}
			}
		}
#ifdef BCMDBG
		else {
			WL_WSEC(("wl%d: %s: CCX: invalid Aironet IE...no CKIP mode\n",
				ci->pub->unit, __FUNCTION__));
		}
#endif /* BCMDBG */
	}
}
#endif /* CCX_SDK */

static uint8
wlc_ccx_get_rx_phy_type(uint rx_phy, wlc_bss_info_t *BSS)
{
	uint8 rx_phy_type;

	if (rx_phy == PHY_TYPE_A) {
		rx_phy_type = CCX_RM_PHY_OFDM;
	} else {
		/* base type on supported rates, default DSS */
		rx_phy_type = CCX_RM_PHY_DSS;
		if (wlc_rateset_isofdm(BSS->rateset.count, BSS->rateset.rates))
			rx_phy_type = CCX_RM_PHY_ERP;
		else {
			uint i;
			for (i = 0; i < BSS->rateset.count; i++) {
				uint8 rate = BSS->rateset.rates[i] & RATE_MASK;
				if (rate == WLC_RATE_5M5 || rate == WLC_RATE_11M) {
					rx_phy_type = CCX_RM_PHY_HRDSS;
					break;
				}
			}
		}
	}

	return rx_phy_type;
}

void
wlc_ccx_update_BSS(ccx_t *ccxh, uint32 rx_tsf_l, uint rx_phy, wlc_bss_info_t *BSS)
{
	BSS->rx_tsf_l = rx_tsf_l;

	/* save the CCX RM PHY Type */
	BSS->ccx_phy_type = wlc_ccx_get_rx_phy_type(rx_phy, BSS);
}

static void
wlc_ccxv2_process_roam_iapp(wlc_ccx_info_t *ci, ccx_roam_ap_ie_t *ap_ie, uint len)
{
	uint idx = 0;
	uint off;

	if (len < CCX_ROAM_AP_IE_LEN)
		return;

	/* do not overwrite v4 neighbor AP list */
	if (ci->droam_ap_num)
		return;

	while (len && (idx < CCX_ROAM_SCAN_CHANNELS)) {
		if (ap_ie->tag == ntoh16(CCX_ROAM_ADJ_AP_TAG)) {
			uint16 channel = ntoh16(ap_ie->channel);
			uint i;

			if (channel) {
				/* add unique channels to the list */
				for (i = 0; i < idx; i++)
					if (ci->pubci.ap_chanspec_list[i] ==
						CH20MHZ_CHSPEC(channel))
						break;
				if (i == idx)
					ci->pubci.ap_chanspec_list[idx++] = CH20MHZ_CHSPEC(channel);
			}
		}

		off = ntoh16(ap_ie->len) + 4;
		ap_ie = (ccx_roam_ap_ie_t *)((char *)ap_ie + off);
		len = (len > off) ? (len - off) : 0;
	}

	ci->pubci.ap_channel_num = idx;
}

static void
wlc_ccx_process_roam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie, uint len,
	bool append)
{
	bcm_tlv_t *tlv;
	ccx_neighbor_rept_ie_t *ie;
	uint channel_idx, list_idx;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	uint i, off;
	ccx_radio_param_subie_t *subie;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_bsscfg_t *cfg = wlc->cfg;

	tlv = (bcm_tlv_t *)rept_ie;
	if (append) {
		channel_idx = ci->pubci.ap_channel_num;
		list_idx = ci->droam_ap_num;
	} else {
		channel_idx = 0;
		list_idx = 0;
		bzero(ci->droam_ap_list, sizeof(ci->droam_ap_list));
	}

	while (len) {
		ie = (ccx_neighbor_rept_ie_t *)bcm_parse_tlvs((void *)tlv, len,
			CCX_ROAM_NEIGHBOR_REPT_ID);
		if (!ie) {
			break;
		}

		if (ie->channel) {
			/* add unique channels to the list */
			for (i = 0; i < channel_idx; i++) {
				if (CHSPEC_CHANNEL(ci->pubci.ap_chanspec_list[i]) == ie->channel) {
					break;
				}
			}
			if (i == channel_idx) {
				ci->pubci.ap_chanspec_list[channel_idx++] =
					CH20MHZ_CHSPEC(ie->channel);
			}
			WL_ROAM(("IAPP: Neighbor %s, CH%03d\n", bcm_ether_ntoa(&ie->mac, eabuf),
				ie->channel));
			WL_ASSOC(("wl%d: %s: ROAM: neighbor report ie: BSS %s, channel %d,"
				" cur_ap: %s\n",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&ie->mac, eabuf), ie->channel,
				bcm_ether_ntoa(&cfg->BSSID, eabuf)));

			/* save bssid */
			bcopy((char *)&ie->mac, (char *)&ci->droam_ap_list[list_idx].bssid,
				ETHER_ADDR_LEN);

			ci->droam_ap_list[list_idx].rf_ie = FALSE; /* mark as rf_ie not present */

			/* process RF subelement if present */
			subie = (ccx_radio_param_subie_t *)NULL;
			if (ie->len > CCX_NEIGHBOR_REPT_IE_LEN_W_H) {
				subie = (ccx_radio_param_subie_t *)bcm_parse_tlvs(
					(void *)((char *)ie + CCX_NEIGHBOR_REPT_IE_LEN),
					ie->len - CCX_NEIGHBOR_REPT_IE_LEN_W_H,
					CCX_ROAM_SUB_RF_PARAMS);
			}

			if (subie) {
				wlc_ccx_process_droam_rf_ie(ci, subie, list_idx,
					!bcmp((char *)&ie->mac, &cfg->BSSID, ETHER_ADDR_LEN));
			}

			list_idx++;
		}

		off = (uint)((char *)ie - (char *)tlv) + ie->len + TLV_HDR_LEN;
		tlv = (bcm_tlv_t *)((char *)tlv + off);
		len = (len > off) ? (len - off) : 0;

		if (channel_idx == CCX_ROAM_SCAN_CHANNELS) {
			WL_ERROR(("wl%d: %s: too many channels",
				ci->pub->unit, __FUNCTION__));
			break;
		}
		if (list_idx == CCX_ROAM_AP_ELEMENTS) {
			WL_ERROR(("wl%d: %s: too many AP elements",
				ci->pub->unit, __FUNCTION__));
			break;
		}
	}

	ci->pubci.ap_channel_num = channel_idx;
	ci->droam_ap_num = list_idx;
	WL_ROAM(("IAPP neighbor list, %d channels, %d APs\n", channel_idx, list_idx));
}

static void
wlc_ccx_process_droam_rf_ie(wlc_ccx_info_t *ci, ccx_radio_param_subie_t *rf_ie,
	int idx, bool curr_ap)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;

	/* check parameters */
	if (rf_ie->roam_delta < 0)
		return;

	/* save minimum rssi */
	ci->droam_ap_list[idx].min_rssi = rf_ie->min_rssi;
	/* save AP's tx power */
	ci->droam_ap_list[idx].ap_tx_pwr = rf_ie->ap_tx_pwr;
	ci->droam_ap_list[idx].roam_time = rf_ie->roam_time;
	/* save roaming parameters */
	ci->droam_ap_list[idx].roam_trigger = rf_ie->roam_trigger;
	ci->droam_ap_list[idx].roam_delta = rf_ie->roam_delta;

	WL_ASSOC(("wl%d: %s: cur_ap = %s, ROAM: rf_ie: roam_trigger %d roam_delta %d min_rssi %d"
		" ap_tx_pwr %d roam_time %d\n",
		wlc->pub->unit, __FUNCTION__,
		curr_ap ? "TRUE" : "FALSE",
		rf_ie->roam_trigger, rf_ie->roam_delta,
		rf_ie->min_rssi, rf_ie->ap_tx_pwr, rf_ie->roam_time));

	/* if this is for current AP, set roaming parameters with RF subelement */
	if (curr_ap) {
		wlcband_t *band = wlc->band;
		/* apply roam parameters in RF subelement */
		band->roam_trigger = rf_ie->roam_trigger;
		/* roam delta is no less than 10 db */
		band->roam_delta = MAX(rf_ie->roam_delta, 10);
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[OTHERBANDUNIT(wlc)];
			band->roam_trigger = rf_ie->roam_trigger;
			band->roam_delta = rf_ie->roam_delta;
		}
		/* RF roaming parameters applied */
		ci->droam_rf_roam = TRUE;
		WL_ASSOC(("wl%d: ROAM: CCX FAST, set roam trigger: %d set roam delta: %d\n",
			wlc->pub->unit, rf_ie->roam_trigger, rf_ie->roam_delta));
	}

	/* RF subelement exists for this AP */
	ci->droam_ap_list[idx].rf_ie = TRUE;
}

/* CCXv4 S51 directed roam */
static void
wlc_ccx_process_droam_iapp(wlc_ccx_info_t *ci, ccx_neighbor_rept_ie_t *rept_ie, uint len)
{
	/* process directed roam iapp frame */
	wlc_ccx_process_roam_iapp(ci, rept_ie, len, FALSE);
	wlc_ccx_roam(&ci->pubci, WLC_E_REASON_DIRECTED_ROAM);
}

/* ccx roam starting point
 * return : 0 for starting roam process successfully, non-zero otherwise.
 */
uint
wlc_ccx_roam(ccx_t *ccxh, uint roam_reason_code)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_bsscfg_t *cfg = ci->wlc->cfg;

	if (cfg->roam->off) {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam disabled\n", roam_reason_code));
		return 1;
	}

	if (ci->pubci.ap_channel_num) {
		ci->pubci.fast_roam = TRUE;
	}

	if (!wlc_roam_scan(cfg, roam_reason_code, NULL, 0)) {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam started\n", roam_reason_code));
		return 0;
	}
	else {
		WL_ASSOC(("wlc_ccx_roam(reason 0x%x): roam scan failed\n", roam_reason_code));
		return 1;
	}
}

static bool
wlc_ccx_rf_ie_good(wlc_ccx_info_t *ci, wlc_ccx_droam_t *droam_ap, wlc_bss_info_t *bi)
{
	int16 min_rssi;

	if (droam_ap->rf_ie == TRUE) {
		/* if rssi meets minimum value, report as good */
		min_rssi = wlc_ccx_get_min_rssi(ci, droam_ap->ap_tx_pwr,
			droam_ap->min_rssi, CHSPEC_CHANNEL(bi->chanspec));
		WL_ASSOC(("wl%d: %s: rssi %d min_rssi %d\n",
			ci->pub->unit, __FUNCTION__, bi->RSSI, min_rssi));

		return (bi->RSSI >= min_rssi);
	} else
		return TRUE; /* report pre-CCXv4 AP as good */
}

uint
wlc_ccx_prune(ccx_t *ccxh, wlc_bss_info_t *bi)
{
	uint i;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	uint32 medium_time_needed = 0;
	struct scb *scb;
#ifdef CCX_SDK
	wlc_ccx_status_row_t *p, **prev;
#endif /* CCX_SDK */
	uint32 qbss_load_aac = 0;

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (WLC_CCX_IS_CURRENT_BSSID(cfg, &bi->BSSID)) {
		if (cfg->roam->reason == WLC_E_REASON_TSPEC_REJECTED) {
			WL_ASSOC(("wl%d: %s: ROAM: prune home AP %s for roam_reason 0x%x",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&bi->BSSID, eabuf), cfg->roam->reason));
			return WLC_E_PRUNE_HOME_AP;
		}
	}

	scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);

	if (!scb)
		return 0;

	medium_time_needed = wlc_cac_medium_time_total(wlc->cac, scb);

	/* qbss_load_aac is in 32us unit */
	qbss_load_aac = bi->qbss_load_aac << 5;
	if (medium_time_needed &&
		(bi->ccx_version >= 4) &&
		(qbss_load_aac < medium_time_needed)) {
			WL_ASSOC(("wl%d: %s: CCX: prune BSSID %s for insufficient AAC\n",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&bi->BSSID, eabuf)));
			return WLC_E_PRUNE_QBSS_LOAD;
	}

#ifdef CCX_SDK
	/* if diagnostic mode enabled, only associate to the AP with
	 * diagnostic capability
	 */
	if (ci->pubci.diag_mode) {
		void *ies = (void*)&bi->bcn_prb[1];
		uint16 ies_len = bi->bcn_prb_len - DOT11_BCN_PRB_LEN;
		ccx_sfa_ie_t *ccx_sfa_ie;
		uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;
		ccx_sfa_ie = (ccx_sfa_ie_t *)bcm_find_vendor_ie(ies, ies_len,
			CISCO_AIRONET_OUI, &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype));
		if (!ccx_sfa_ie || ccx_sfa_ie->length != (sizeof(ccx_sfa_ie_t) - TLV_HDR_LEN) ||
			!(ccx_sfa_ie->capability & CAP_DIAG_CHANL)) {
			WL_ASSOC(("wl%d: %s: CCX: prune BSSID %s for unavailable diag function\n",
				wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&bi->BSSID, eabuf)));
			/* prune the AP */
			return WLC_E_PRUNE_NO_DIAG_SUPPORT;
		}
	}

	/* CCXv5 S71 Interpretation of Status and Result Codes */
	for (prev = &ci->blocked_ap; *prev != NULL; prev = &(*prev)->next) {
		if (!bcmp((char*)&(*prev)->bssid, (char*)&bi->BSSID, ETHER_ADDR_LEN)) {
			/* no prune decision based on addts delay */
			if ((*prev)->status_row == 5)
				break;
			/* unblock connection to the AP if
			 *  - it is in association process initiated by set ssid, and
			 *  - passed minimum wait time since blocked
			 */
			if (cfg->assoc->type == AS_ASSOCIATION && !(*prev)->wait_time) {
				/* remove the blocked AP entry and continue */
				p = *prev;
				*prev = p->next;
				MFREE(ci->pub->osh, p, sizeof(wlc_ccx_status_row_t));
				break;
			} else {
				WL_ASSOC(("wl%d: %s: CCX: prune BSSID %s due to AP blocked\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa(&bi->BSSID, eabuf)));
				return WLC_E_PRUNE_AP_BLOCKED;
			}
		}
	}
#endif /* CCX_SDK */

	if (wlc->ccx->fast_roam == FALSE) {
		return 0;
	}

	/* it is ccx fast_roam */
	/* prune previous AP */
	if (cfg->roam->reason != WLC_E_REASON_DEAUTH &&
	    cfg->roam->reason != WLC_E_REASON_DISASSOC &&
	    !bcmp((char *)&cfg->prev_BSSID, (char *)&bi->BSSID, ETHER_ADDR_LEN)) {
		WL_ASSOC(("wl%d: %s: CCX: prune previous AP BSSID %s\n",
			wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(&bi->BSSID, eabuf)));
		return WLC_E_PRUNE_CCXFAST_PREVAP;
	}

	/* do not prune if there is no roam AP list */
	if (!ci->droam_ap_num)
		return 0;

	/* search for matched AP in roam list */
	for (i = 0; i < ci->droam_ap_num; i++) {
		if (!bcmp((char*)&ci->droam_ap_list[i].bssid, (char*)&bi->BSSID,
			ETHER_ADDR_LEN)) {
			/* make sure rssi is good and aac is non-zero */
			if (wlc_ccx_rf_ie_good(ci, &ci->droam_ap_list[i], bi) == TRUE) {
				return 0;
			}
		}
	}

	WL_ASSOC(("wl%d: %s: CCX: FAST ROAM: prune BSSID %s in AP-assisted roaming\n",
		wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&bi->BSSID, eabuf)));

	return WLC_E_PRUNE_CCXFAST_DROAM;
}

static int16
wlc_ccx_get_min_rssi(wlc_ccx_info_t *ci, int8 ap_txpwr, int8 ap_rssi, uint8 ap_channel)
{
	uint8 sta_txpwr_phy;
	int8 sta_txpwr;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	int16 min_rssi;

	/* XXX REVISIT johnvb  Need to re-implement a query of tx power for a channel.
	 * The old phy API didn't actually use the limits for the given channel.  Also
	 * there are questions on what effect should local constraints, user target and
	 * power percentage should have on the result.  Lastly should these values
	 * based on targets vs. limits (presumbably targets).  File a PR for more
	 * careful review.  Temporarily just assume a maximum tx power which will
	 * cause us to use the AP's RSSI.
	 */
	sta_txpwr_phy = WLC_TXPWR_MAX;
	sta_txpwr_phy /= WLC_TXPWR_DB_FACTOR;
	sta_txpwr = MIN(sta_txpwr_phy, 0x7f); /* saturate it to max. positive */

	if (ap_txpwr > sta_txpwr) {
		min_rssi = (int16)(ap_rssi + (ap_txpwr - sta_txpwr));
	}
	else {
		min_rssi = (int16)(ap_rssi);
	}

	WL_ASSOC(("wl%d: %s: ROAM: min_rssi 0x%x\n",
		wlc->pub->unit, __FUNCTION__, min_rssi));
	BCM_REFERENCE(wlc);

	return min_rssi;
}

int
wlc_ccx_rm_begin(ccx_t *ccxh, wlc_rm_req_t* req)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	int type = req->type;
	int dur = req->dur;

	switch (type) {
	case WLC_RM_TYPE_BEACON_TABLE:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: Beacon Table request noted\n",
		           ci->pub->unit));
		break;
	case WLC_RM_TYPE_PASSIVE_SCAN:
	case WLC_RM_TYPE_ACTIVE_SCAN:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting Beacon measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_scan_begin(ci, type, dur);
		break;
	case WLC_RM_TYPE_FRAME:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting Frame measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_frm_begin(ci, dur);
		break;
	case WLC_RM_TYPE_PATHLOSS:
		WL_INFORM(("wl%d: wlc_ccx_rm_begin: starting PathLoss measurement\n",
		           ci->pub->unit));
		wlc_ccx_rm_pathloss_begin(ci, req);
		break;
	default:
		return -1;
	}

	return 0;
}

static void
wlc_ccx_rm_scan_begin(wlc_ccx_info_t *ci, int type, uint32 dur)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	ccx_rm_t *rm = ci->rm_state->ccx;

	rm->scan_active = TRUE;
	rm->scan_dur = dur;
	wlc_bss_list_free(wlc, &rm->scan_results);
	bzero(rm->scan_results.ptrs, wlc->pub->tunables->maxbss * sizeof(wlc_bss_info_t*));

	/* hijack scan state so wlc_scan_parse() will capture the right info */
	wlc->scan->wlc_scan_cmn->bss_type = DOT11_BSSTYPE_ANY;
	bcopy(ether_bcast.octet, wlc->scan->bssid.octet, ETHER_ADDR_LEN);
	wlc->scan->wlc_scan_cmn->usage = SCAN_ENGINE_USAGE_RM;
	wlc->scan->state |= SCAN_STATE_SAVE_PRB;

	wlc_bss_list_free(wlc, wlc->scan_results);

	/* enable promisc reception of beacons and probe responses for scan */
	wlc->bcnmisc_scan = TRUE;
	wlc_mac_bcn_promisc(wlc);
#ifdef WLOFFLD
	/* disable bcn offloads (set our block on offloads) */
	if (WLOFFLD_CAP(wlc)) {
		wlc_ol_rx_deferral(wlc->ol, OL_RM_SCAN_MASK, OL_RM_SCAN_MASK);
	}
#endif /* WLOFFLD */
	if (type == WLC_RM_TYPE_ACTIVE_SCAN)
		wlc_sendprobe(wlc, wlc->cfg, (const uint8 *)"", 0,
		              &ether_bcast, &ether_bcast, 0, NULL, 0);
}

void
wlc_ccx_rm_scan_complete(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 diff_h, diff_l;

	/* calc the actual duration of the scan */
	wlc_read_tsf(wlc, &diff_l, &diff_h);
	wlc_uint64_sub(&diff_h, &diff_l, rm_state->actual_start_h, rm_state->actual_start_l);
	rm_state->ccx->scan_dur = diff_l / DOT11_TU_TO_US;

	/* Restore promisc behavior for beacons and probes */
	wlc->bcnmisc_scan = FALSE;
	wlc_mac_bcn_promisc(wlc);

	/* copy scan results to rm_state for reporting */
	wlc_bss_list_xfer(wlc->scan_results, &rm_state->ccx->scan_results);

	/* clear the save probe flag in case there are late probe responses */
	wlc->scan->wlc_scan_cmn->usage = SCAN_ENGINE_USAGE_NORM;
	wlc->scan->state &= ~SCAN_STATE_SAVE_PRB;
#ifdef WLOFFLD
	/* re-enable bcn offloads (really just remove our block) */
	if (WLOFFLD_CAP(wlc)) {
		wlc_ol_rx_deferral(wlc->ol, OL_RM_SCAN_MASK, 0);
	}
#endif /* WLOFFLD */

	rm_state->ccx->scan_active = FALSE;

	if (rm_state->ccx->scan_results.count > 0) {
		WL_INFORM(("wl%d: radio measure scan complete, %d results\n",
			ci->pub->unit, rm_state->ccx->scan_results.count));
	} else {
		WL_INFORM(("wl%d: radio measure scan complete, no networks found\n",
			ci->pub->unit));
	}

	/* see if we are done with all measures in the current set */
	wlc_rm_meas_end(wlc);

	return;
}

static void
wlc_ccx_rm_frm_begin(wlc_ccx_info_t *ci, uint32 dur)
{
	int i;
	ccx_rm_t *rm = ci->rm_state->ccx;

	rm->frame_active = TRUE;
	rm->frame_dur = dur;
	rm->frm_elts = 0;
	rm->promisc_org = ci->pub->promisc;
	rm->promisc_off = FALSE;
	for (i = 0; i < WLC_NRMFRAMEHASH; i++)
		ASSERT(!rm->frmhash[i]);

	/* enable promisc reception */
	ci->pub->promisc = TRUE;
	wlc_mac_promisc(ci->wlc);
}

void
wlc_ccx_rm_frm_complete(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 diff_h, diff_l;

	rm_state->ccx->frame_active = FALSE;

	/* calc the actual duration of the scan */
	wlc_read_tsf(ci->wlc, &diff_l, &diff_h);
	wlc_uint64_sub(&diff_h, &diff_l, rm_state->actual_start_h, rm_state->actual_start_l);
	rm_state->ccx->frame_dur = diff_l / DOT11_TU_TO_US;

	/* restore promisc. mode if no mode change req. during test */
	if (!rm_state->ccx->promisc_off)
		ci->pub->promisc = rm_state->ccx->promisc_org;
	wlc_mac_promisc(ci->wlc);

	/* see if we are done with all measures in the current set */
	wlc_rm_meas_end(ci->wlc);
}

static void
wlc_ccx_pathloss_dur_timer(void *arg)
{
	wlc_info_t *wlc =  (wlc_info_t *)arg;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)wlc->ccx;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		return;
	}
	if (!wlc->pub->up)
		return;

	WL_ERROR(("wl%d: %s: Pathloss Duration timer expired, so ending the measurement\n",
		ci->wlc->pub->unit, __FUNCTION__));

	wlc_ccx_rm_pathloss_complete((ccx_t *)ci, TRUE);
}

static void
wlc_ccx_pathloss_burst_timer(void *arg)
{
	wlc_info_t *wlc =  (wlc_info_t *)arg;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)wlc->ccx;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	ccx_rm_t *rm = rm_state->ccx;
	wlc_rm_req_t *rm_req;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		return;
	}
	if (!wlc->pub->up)
		return;

	ASSERT(rm->pathloss_longrun);

	WL_INFORM(("wl%d: %s: Pathloss Burst timer expired so schduling the next burst \n",
		ci->wlc->pub->unit, __FUNCTION__));
	rm_req = (wlc_rm_req_t*)MALLOC(wlc->osh, sizeof(wlc_rm_req_t));
	if (rm_req == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->wlc->pub->unit, __FUNCTION__, MALLOCED(ci->wlc->osh)));
		return;
	}

	bcopy(&rm->pathloss_data.req, rm_req, sizeof(wlc_rm_req_t));

	rm_state->broadcast = FALSE;
	rm_state->token = rm->pathloss_data.req.token;
	rm_state->req_count = 1;
	rm_state->req = rm_req;

	wlc_rm_start(wlc);
}

static void
wlc_ccx_rm_pathloss_begin(wlc_ccx_info_t *ci, wlc_rm_req_t* req)
{
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	wlc_phy_t *pi = WLC_PI(ci->wlc);
	ppr_t *txpwr;
#if defined(WLTXPWR1_SIGNED)
	int8 cur_qtxpwr;
#endif // endif

	WL_INFORM(("starting the pathloss measurement\n"));

	if (req->flags & WLC_RM_FLAG_REFUSED) {
		WL_INFORM(("Refused flag is set..so coming out\n"));
		return;
	}

	/* save tx power control setting and turn off tx power control */
	ccx_rm->cur_txpowerctrl = wlc_phy_txpower_hw_ctrl_get(pi);
	wlc_phy_txpower_hw_ctrl_set(pi, FALSE);

	/* save tx power and set the desired tx power */
#if defined(WLTXPWR1_SIGNED)
	wlc_phy_txpower_get(pi, &cur_qtxpwr, &ccx_rm->cur_txpoweroverride);
	ccx_rm->cur_txpower = cur_qtxpwr;
#else
	wlc_phy_txpower_get(pi, &ccx_rm->cur_txpower, &ccx_rm->cur_txpoweroverride);
#endif // endif
	if ((txpwr = ppr_create(ci->wlc->osh, PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
		return;
	}

	wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
	ppr_apply_max(txpwr, WLC_TXPWR_MAX);
	/* restore tx power */
	wlc_phy_txpower_set(pi, pathloss_data->req_txpower * 4, FALSE, txpwr);
	ppr_delete(ci->wlc->osh, txpwr);

	WL_INFORM(("Pathloss measurement: staring the burst duration timer\n"));
	wl_add_timer(ci->wlc->wl, ccx_rm->plm_dur_timer, pathloss_data->duration, FALSE);

	/* begin the pathloss measurement */
	if (ccx_rm->pathloss_longrun == TRUE) {

	} else if ((pathloss_data->nbursts == 0) || (pathloss_data->nbursts > 1)) {
		WL_INFORM(("Pathloss measurement for more than one burst %d\n",
			pathloss_data->burst_interval));
		ccx_rm->pathloss_longrun = TRUE;
		wl_add_timer(ci->wlc->wl, ccx_rm->plm_burst_timer,
			pathloss_data->burst_interval, TRUE);
	}

	ccx_rm->pathloss_active = TRUE;

	wlc_ccx_rm_pathloss(ci);
}

static void
wlc_ccx_rm_pathloss(wlc_ccx_info_t *ci)
{
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;

	/* send the pathloss measurement frame */
	wlc_ccx_rm_send_pathlossmeasure_frame(ci);

	/* update the local stats */
	pathloss_data->cur_chanidx++;
	if (pathloss_data->cur_chanidx == pathloss_data->nchannels) {
		pathloss_data->cur_chanidx = 0;
		pathloss_data->seq_number++;
		pathloss_data->cur_burstlen++;
		if (pathloss_data->cur_burstlen == pathloss_data->burst_len) {
			pathloss_data->cur_burst++;
			pathloss_data->cur_burstlen = 0;
		}
	}
}

void
wlc_ccx_rm_pathloss_complete(ccx_t *ccxh, bool force_done)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	wlc_phy_t *pi  = WLC_PI(ci->wlc);
	ppr_t *txpwr;

	WL_INFORM(("stopping the Pathloss Measurment burst duration timer\n"));
	wl_del_timer(ci->wlc->wl, ccx_rm->plm_dur_timer);

	/* restore tx power control setting */
	wlc_phy_txpower_hw_ctrl_set(pi, ccx_rm->cur_txpowerctrl);

	/* restore tx power and override */
	if ((txpwr = ppr_create(ci->wlc->osh, PPR_CHSPEC_BW(ci->wlc->chanspec))) == NULL) {
		return;
	}

	wlc_channel_reg_limits(ci->wlc->cmi, ci->wlc->chanspec, txpwr);
	ppr_apply_max(txpwr, WLC_TXPWR_MAX);
	/* restore tx power */
#if defined(WLTXPWR1_SIGNED)
	wlc_phy_txpower_set(pi, (int8)ccx_rm->cur_txpower,
#else
	wlc_phy_txpower_set(pi, ccx_rm->cur_txpower,
#endif // endif
		ccx_rm->cur_txpoweroverride, txpwr);
	ppr_delete(ci->wlc->osh, txpwr);

	/* check if we are already done */
	if (ccx_rm->pathloss_active == FALSE)
		return;

	ccx_rm->pathloss_active = FALSE;

	/* done with all the bursts */
	if ((pathloss_data->nbursts && (pathloss_data->cur_burst == pathloss_data->nbursts)) ||
		force_done) {
		WL_INFORM(("*****Pathloss Measurments complete****\n"));
		ccx_rm->pathloss_longrun = FALSE;
		wl_del_timer(ci->wlc->wl, ccx_rm->plm_burst_timer);
	}
	/* current burst done */
	else if ((!pathloss_data->nbursts) && pathloss_data->cur_burst) {
		WL_INFORM(("Pathloss Measurments complete for current burst\n"));
		pathloss_data->cur_burst = 0;
	}

	/* measurement end */
	wlc_rm_meas_end(ci->wlc);
}

static void
wlc_ccx_rm_pathloss_pktcallback(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)arg;
	ccx_rm_t *ccx_rm = ci->rm_state->ccx;
	ccx_rm_pl_data_t *pathloss_data = &ccx_rm->pathloss_data;
	bool force_done = FALSE;

	WL_INFORM(("pathloss callback: pktstatus is 0x%x\n", txstatus));
	if (ccx_rm->pathloss_active == FALSE)
		return;

	if (txstatus != (TX_STATUS_SUPR_FLUSH << TX_STATUS_SUPR_SHIFT)) {
		if ((pathloss_data->cur_burst) && (!pathloss_data->cur_burstlen))
			goto done;
		WL_INFORM(("Pathloss Measurments not complete for current burst yet\n"));
		wlc_ccx_rm_pathloss(ci);
		return;
	}
	else
		force_done = TRUE;
done:
	WL_INFORM(("Pathloss Measurments complete for current burst\n"));
	/* done with pathloss measurements */
	wlc_ccx_rm_pathloss_complete((ccx_t *)ci, force_done);
}

static void
wlc_ccx_rm_send_pathlossmeasure_frame(wlc_ccx_info_t *ci)
{
	uint body_len;
	uint8 *pbody;
	struct ccx_rm_pathlossmeas_frame *frame;
	void *p;
	ccx_rm_pl_data_t *pathloss_data;
	ccx_iapp_hdr_t *iapphdr;
	struct dot11_header *h;

	pathloss_data = &ci->rm_state->ccx->pathloss_data;

	body_len = DOT11_LLC_SNAP_HDR_LEN + sizeof(ccx_iapp_hdr_t) +
		sizeof(struct ccx_rm_pathlossmeas_frame) - 1;

	p =  PKTGET(ci->pub->osh, (body_len + TXOFF), TRUE);
	if (p == NULL) {
		WL_ERROR(("Couldn't allocate data pakcet for the pathloss measurement\n"));
		return;
	}

	/* leave the space for phyheader and d11hdr */
	PKTPULL(ci->pub->osh, p, (D11_TXH_LEN + D11_PHY_HDR_LEN));
	PKTSETLEN(ci->pub->osh, p, (DOT11_A4_HDR_LEN + body_len));

	pbody = (uint8*)PKTDATA(ci->pub->osh, p);
	h = (struct dot11_header *)pbody;

	/* 802.11 hdr */
	h->fc = (FC_TODS|FC_FROMDS);
	h->fc |= (FC_TYPE_DATA << FC_TYPE_SHIFT);
	h->fc = htol16(h->fc);
	h->durid = 0;
	bcopy(ci->pub->cur_etheraddr.octet, (char *)&h->a2, ETHER_ADDR_LEN);

	bcopy(&pathloss_data->da, (char *)&h->a1, ETHER_ADDR_LEN);
	bcopy(&pathloss_data->da, (char *)&h->a3, ETHER_ADDR_LEN);
	pbody += DOT11_A3_HDR_LEN;

	/* fill the pathloss frame data */
	bcopy(CISCO_AIRONET_SNAP, pbody, DOT11_LLC_SNAP_HDR_LEN);

	iapphdr = (ccx_iapp_hdr_t *)(pbody + DOT11_LLC_SNAP_HDR_LEN);
	iapphdr->id_len = htol16(body_len - DOT11_LLC_SNAP_HDR_LEN);
	iapphdr->type = CCX_IAPP_TYPE_RM;
	iapphdr->subtype = CCX_RM_IAPP_SUBTYPE;
	bcopy(pathloss_data->da.octet, iapphdr->da.octet,  ETHER_ADDR_LEN);
	bcopy(ci->pub->cur_etheraddr.octet, iapphdr->sa.octet, ETHER_ADDR_LEN);

	frame = (struct ccx_rm_pathlossmeas_frame *)(&iapphdr->data);
	frame->seq = htol16(((pathloss_data->seq_number) & CCX_RM_PATHLOSS_SEQ_MASK));
	/* fill in the current txpower */
	frame->txpower = pathloss_data->req_txpower;
	frame->txchannel = pathloss_data->channels[pathloss_data->cur_chanidx];

	wlc_pcb_fn_register(ci->wlc->pcb, wlc_ccx_rm_pathloss_pktcallback, (void *)ci, p);
	PKTSETPRIO(p, MAXPRIO);
	/* Data fifo is suspended, so use ctrl fifo .. this is a data frame .. */
	wlc_sendctl(ci->wlc, p, ci->wlc->active_queue, WLC_BCMCSCB_GET(ci->wlc, ci->wlc->cfg),
	            TX_CTL_FIFO, 0, FALSE);
}

static void
wlc_ccx_rm_frm_list_free(wlc_ccx_info_t *ci)
{
	int i;
	ccx_rm_t *rm = ci->rm_state->ccx;
	wlc_ccx_rm_frm_elt_t *frm_elt, *nextp;

	/* free allocate memory */
	for (i = 0; i < WLC_NRMFRAMEHASH; i++) {
		for (frm_elt = rm->frmhash[i]; frm_elt; frm_elt = nextp) {
			nextp = frm_elt->next;
			MFREE(ci->pub->osh, frm_elt, sizeof(wlc_ccx_rm_frm_elt_t));
		}
		rm->frmhash[i] = NULL;
	}
}

void
wlc_ccx_rm_frm(ccx_t *ccxh, wlc_d11rxhdr_t *wrxh, struct dot11_header *h)
{
	struct ether_addr *ta, *bssid;
	uint16 dir, fc = ltoh16(h->fc);
	int indx;
	wlc_ccx_rm_frm_elt_t *elt;
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *rm = ci->rm_state->ccx;

	ASSERT(FC_TYPE(fc) == FC_TYPE_DATA);

	dir = fc & (FC_TODS | FC_FROMDS);
	if (dir == 0) {
		ta = &h->a2;
		bssid = &h->a3;
	} else if (dir == FC_TODS) {
		ta = &h->a2;
		bssid = &h->a1;
	} else if (dir == FC_FROMDS) {
		ta = &h->a3;
		bssid = &h->a2;
	} else {
		ta = bssid = &h->a2;
	}

	indx = RMFRMHASHINDEX((uchar *)ta);
	/* search for the frame element which corresponds to the tx station ea */
	for (elt = rm->frmhash[indx]; elt; elt = elt->next)
		if (!bcmp((char*)ta, (char*)&elt->ta, ETHER_ADDR_LEN) &&
			!bcmp((char*)bssid, (char*)&elt->bssid, ETHER_ADDR_LEN))
			break;

	if (!elt) {
		/* allocate memory for new element */
		elt = (wlc_ccx_rm_frm_elt_t *)MALLOCZ(ci->pub->osh, sizeof(wlc_ccx_rm_frm_elt_t));
		if (!elt) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
			return;
		}

		/* copy transmit address and bssid */
		bcopy(ta, &elt->ta, ETHER_ADDR_LEN);
		bcopy(bssid, &elt->bssid, ETHER_ADDR_LEN);

		/* link new element in link list */
		elt->next = rm->frmhash[indx];
		rm->frmhash[indx] = elt;
		rm->frm_elts++;
	} else if (elt->frames == MAXNBVAL(sizeof(elt->frames)))
		/* one byte length.  stop count if reach to max limit */
		return;

	/* update report fields */
	elt->rssi_sum += wlc_lq_rssi_pktrxh_cal(ci->wlc, wrxh);
	elt->frames++;
}

static void
wlc_ccx_rm_recv(wlc_ccx_info_t *ci, struct ether_addr* da,
	struct ether_addr* sa, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len)
{
	ccx_rm_req_ie_t* req_ie;
	int req_block_len;

#ifdef BCMDBG
	if (WL_INFORM_ON())
		wlc_ccx_rm_req_dump(ci, da, sa, req_pkt, req_len);
#endif /* BCMDBG */

	/* make sure this is a valid CCX RM Request packet */
	if (req_len < CCX_RM_REQ_LEN) {
		WL_ERROR(("wl%d: %s: radio request packet too short, "
			"len = %d expected at least %d", ci->pub->unit, __FUNCTION__,
			req_len, CCX_RM_REQ_LEN));
		return;
	}
	req_block_len = req_len - CCX_RM_REQ_LEN;
	req_ie = (ccx_rm_req_ie_t*)req_pkt->data;

	/* return if no recongnized RM requests */
	if (wlc_ccx_rm_ie_count(req_ie, req_block_len) == 0) {
		WL_INFORM(("no IE's found in rm req...\n"));
		return;
	}

	/* otherwise we have a new RM request ... */
	wlc_rm_abort(ci->wlc);

	if (wlc_ccx_rm_state_init(ci, ETHER_ISMULTI(da), rx_time, req_pkt, req_len)) {
		WL_ERROR(("wl%d: %s: failed to initialize radio "
			  "measurement state, dropping request\n",
			  ci->pub->unit, __FUNCTION__));
	}

	wlc_rm_start(ci->wlc);
}

int
wlc_ccx_rm_validate(ccx_t *ccxh, chanspec_t cur_chanspec, wlc_rm_req_t *req)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	ccx_rm_pl_data_t *pathloss_data = &rm_state->ccx->pathloss_data;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	/* reject rm request if AP active */
	if (AP_ACTIVE(wlc)) {
		WL_INFORM(("wl%d: %s: reject rm request since AP is active\n",
			wlc->pub->unit, __FUNCTION__));
		req->flags |= WLC_RM_FLAG_REFUSED;
		return -1;
	}

	/* check for off-channel measurements over the limit */
	if (req->chanspec && req->chanspec != cur_chanspec && wlc->pub->associated &&
	    req->dur > (int)ci->rm_limit) {
		WL_INFORM(("wl%d: wlc_rm_validate: refusing off channel "
			   "measurement with dur %d greater than "
			   "limit %d, RM type %d token %d channel %s\n",
			   wlc->pub->unit, req->dur, ci->rm_limit,
			   req->type, req->token, wf_chspec_ntoa_ex(req->chanspec, chanbuf)));
		req->flags |= WLC_RM_FLAG_REFUSED;
		return -1;
	}
	if (req->type == WLC_RM_TYPE_PATHLOSS) {
		if ((!pathloss_data) || (pathloss_data->nchannels != 1)) {
			WL_INFORM(("wl%d: %s: more than one channel specified for"
			   "offchannel pathloss measurements \n",
			   wlc->pub->unit, __FUNCTION__));
			req->flags |= WLC_RM_FLAG_REFUSED;
			return -1;
		}
	}
	return 0;
}

static int
wlc_ccx_rm_state_init(wlc_ccx_info_t *ci,
	bool bcast, uint16 rx_time,
	ccx_rm_req_t* req_pkt, int req_len)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	uint32 tsf_h, tsf_l;
	uint32 req_time_h;
	uint32 req_time_l;
	uint32 start_time_h;
	uint32 start_time_l;
	uint32 offset_us;
	uint32 tbtt_offset;
	uint32 total_offset;
	uint32 bi;
	int rm_req_size;
	int rm_req_count;
	wlc_rm_req_t* rm_req;
	ccx_rm_req_ie_t* req_ie;
	int req_block_len;

	req_block_len = req_len - CCX_RM_REQ_LEN;
	req_ie = (ccx_rm_req_ie_t*)req_pkt->data;

	rm_req_count = wlc_ccx_rm_ie_count(req_ie, req_block_len);
	rm_req_size = rm_req_count * sizeof(wlc_rm_req_t);
	rm_req = (wlc_rm_req_t*)MALLOCZ(ci->pub->osh, rm_req_size);
	if (rm_req == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return BCME_NORESOURCE;
	}

	rm_state->report_class = WLC_RM_CLASS_CCX;
	rm_state->broadcast = bcast;
	rm_state->token = ltoh16(req_pkt->token);
	rm_state->req_count = rm_req_count;
	rm_state->req = rm_req;

	/* clear out the pathloss data */
	rm_state->ccx->pathloss_active = FALSE;
	rm_state->ccx->pathloss_longrun = FALSE;
	wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_burst_timer);
	wl_del_timer(ci->wlc->wl, ci->rm_state->ccx->plm_dur_timer);

	bzero(&rm_state->ccx->pathloss_data, sizeof(ccx_rm_pl_data_t));

	/* Fill out the request blocks */
	wlc_ccx_rm_parse_requests(ci, req_ie, req_block_len, rm_req, rm_req_count);

	/* Compute the start time of the measurements,
	 * Req frame has a delay in TBTT times, and an offset in TUs
	 * from the delay time.
	 */
	wlc_read_tsf(ci->wlc, &tsf_l, &tsf_h);

	/* recover the tsf_l 32 bit value for the req frame rx time */
	req_time_l = (tsf_l & 0xffff0000) + rx_time;
	if (rx_time > (tsf_l & 0xffff)) {
		/* a greater rx_time indicates the low 16 bits of
		 * tsf_l wrapped, so decrement the high 16 bits.
		 */
		req_time_l = req_time_l - 0x10000;
	}

	/* recover the tsf_h 32 bit value for the req frame rx time */
	req_time_h = tsf_h;
	if (req_time_l > tsf_l) {
		/* a greater req_time_l indicates tsf_l wrapped
		 * since the req_time_l, so decrement the high value.
		 */
		req_time_h--;
	}

	offset_us = req_pkt->offset * DOT11_TU_TO_US;

	/* we now have the recovered TSF time of the request frame,
	 * compute the measurement start time based on the
	 * requested delay (TBTTs) and offset (TUs) from the request frame time.
	 */
	start_time_l = req_time_l;
	start_time_h = req_time_h;

	if (req_pkt->delay == 0) {
		/* no delay, use the request frame time plus offset */
		wlc_uint64_add(&start_time_h, &start_time_l, 0, offset_us);
	} else {
		/* compute the TSF time of the TBTT 'delay' Beacon intervals
		 * from when the request packet was sent, plus 'offset'
		 */

		/* first find the offset to the TBTT of the beacon interval
		 * that contained the request frame (offset back in time).
		 */
		bi = ci->wlc->cfg->current_bss->beacon_period;
		tbtt_offset = wlc_calc_tbtt_offset(bi, req_time_h, req_time_l);

		/* calculate the offset to the TBTT 'delay' beacon
		 * intervals from the reqest frame time.
		 */
		total_offset = req_pkt->delay * (bi * DOT11_TU_TO_US) - tbtt_offset;
		/* add in the request offset value */
		total_offset += offset_us;

		/* calculate the full TSF timer for the target measurement */
		wlc_uint64_add(&start_time_h, &start_time_l, 0, total_offset);
	}

	if (wlc_uint64_lt(start_time_h, start_time_l, tsf_h, tsf_l)) {
		/* we have already passed the offset time specified
		 * in the frame, so just use the current time
		 */
		start_time_h = tsf_h;
		start_time_l = tsf_l;
	}

	/* The CCX measurement request frame has one start time for the set of
	 * measurements, but the driver state allows for a start time for each
	 * measurement. Set the CCX measurement request start time into the
	 * first of the driver measurement requests, and leave the following
	 * start times as zero to indicate they happen as soon as possible.
	 */
	rm_req[0].tsf_h = start_time_h;
	rm_req[0].tsf_l = start_time_l;

	return 0;
}

void
wlc_ccx_rm_free(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_rm_req_state_t* rm_state = ci->rm_state;

	if (rm_state->report_class == WLC_RM_CLASS_CCX) {
		wlc_ccx_rm_rept_t *report;
		wlc_ccx_rm_rept_t *next;
		report = ci->rm_reports;
		ci->rm_reports = NULL;

		while (report != NULL) {
			next = report->next;
			MFREE(ci->pub->osh, report, sizeof(wlc_ccx_rm_rept_t));
			report = next;
		}
	}

	rm_state->ccx->scan_active = FALSE;
	wlc_bss_list_free(ci->wlc, &rm_state->ccx->scan_results);
	rm_state->ccx->frame_active = FALSE;
	wlc_ccx_rm_frm_list_free(ci);
}

static uint8
wlc_ccx_rm_get_mbssid_mask(wlc_bss_info_t *bi)
{
	ccx_radio_mgmt_t *rm_cap_ie;
	uint8 rm_cap_ver = 1;
	uint8 mbssid_mask = 0;
	int bcn_len;
	uint8 *bcn_ie = NULL;

	ASSERT(bi != NULL);
	bcn_len = bi->bcn_prb_len;
	if (bi->bcn_prb && bcn_len > DOT11_BCN_PRB_LEN) {
		bcn_ie = (uchar*)bi->bcn_prb + DOT11_BCN_PRB_LEN;
		bcn_len -= DOT11_BCN_PRB_LEN;
	}

	if ((rm_cap_ie = (ccx_radio_mgmt_t*)bcm_find_vendor_ie(bcn_ie, bcn_len, CISCO_AIRONET_OUI,
		&rm_cap_ver, 1)) &&
		rm_cap_ie->len >= DOT11_OUI_LEN + 3)
		mbssid_mask = (uint8)((ltoh16(rm_cap_ie->state) & CCX_RM_STATE_MBSSID_MASK) >>
			CCX_RM_STATE_MBSSID_SHIFT);

	return mbssid_mask;
}

/* CCXv4: only a single BSSID beacon in MBSSID AP should be reported. sec 59.3.2 */
static void
wlc_ccx_rm_mbssid_report(wlc_bss_info_t **results, uint8 *report, uint num)
{
	uint i, j;
	struct ether_addr mac_i, mac_j;
	uint8 mbssid_mask;
	const uint8 mask[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f};

	/* find out MBSSID's beacons and report for one BSSID only */
	for (i = 0; i < num; i++) {
		if (!report[i] || !(mbssid_mask = wlc_ccx_rm_get_mbssid_mask(results[i])))
			continue;
		/* MBSSID's found */
		WL_INFORM(("wl: wlc_ccx_rm_mbssid_report: mbssid to report"
			" %02x:%02x:%02x:%02x:%02x:%02x\n",
			results[i]->BSSID.octet[0], results[i]->BSSID.octet[1],
			results[i]->BSSID.octet[2],
			results[i]->BSSID.octet[3], results[i]->BSSID.octet[4],
			results[i]->BSSID.octet[5]));
		mac_i = results[i]->BSSID;
		mac_i.octet[ETHER_ADDR_LEN - 1] &= ~mask[mbssid_mask];
		for (j = i + 1; j < num; j++) {
			mac_j = results[j]->BSSID;
			mac_j.octet[ETHER_ADDR_LEN - 1] &= ~mask[mbssid_mask];
			if (!bcmp(&mac_i, &mac_j, ETHER_ADDR_LEN)) {
				/* do not report */
				report[j] = 0;
				WL_INFORM(("wl: wlc_ccx_rm_mbssid_report: mbssid to not report"
					" %02x:%02x:%02x:%02x:%02x:%02x\n",
					results[j]->BSSID.octet[0], results[j]->BSSID.octet[1],
					results[j]->BSSID.octet[2],
					results[j]->BSSID.octet[3], results[j]->BSSID.octet[4],
					results[j]->BSSID.octet[5]));
			}
		}
	}
}

static void
wlc_ccx_rm_parse_requests(wlc_ccx_info_t *ci, ccx_rm_req_ie_t* ie, int len,
	wlc_rm_req_t* req, int count)
{
	int idx = 0;
	int8 type;
	uint32 parallel_flag = 0;
	ccx_rm_pathlossreq_t *pathloss_req;
	ccx_rm_pl_data_t *pathloss_data = &ci->rm_state->ccx->pathloss_data;

	/* convert each CCX RM IE into a wlc_rm_req */
	for (; ie != NULL && idx < count; ie = wlc_ccx_rm_next_ie(ie, &len)) {
		/* skip IE if we do not recongnized the request,
		 * or it was !enable (not a autonomous report req)
		 */
		if (ltoh16(ie->id) != CCX_RM_ID_REQUEST ||
		    ltoh16(ie->len) < CCX_RM_REQ_IE_FIXED_LEN ||
		    0 != (ie->mode & CCX_RM_MODE_ENABLE))
			continue;

		/* in case we skip measurement requests, keep track of the
		 * parallel bit flag separately. Clear here at the beginning of
		 * every set of measurements and set below after the first
		 * measurement req we actually send down.
		 */
		if (!(ie->mode & CCX_RM_MODE_PARALLEL)) {
			parallel_flag = 0;
		}

		switch (ie->type) {
		case CCX_RM_TYPE_LOAD:
			type = WLC_RM_TYPE_CCA;
			break;
		case CCX_RM_TYPE_NOISE:
			type = WLC_RM_TYPE_RPI;
			break;
		case CCX_RM_TYPE_FRAME:
			type = WLC_RM_TYPE_FRAME;
			break;
		case CCX_RM_TYPE_BEACON:
			if (ie->param == CCX_RM_BEACON_PASSIVE_SCAN) {
				type = WLC_RM_TYPE_PASSIVE_SCAN;
			} else if (ie->param == CCX_RM_BEACON_ACTIVE_SCAN) {
				type = WLC_RM_TYPE_ACTIVE_SCAN;
			} else if (ie->param == CCX_RM_BEACON_TABLE) {
				/* no beacon table support for now, do not reply */
				/* type = WLC_RM_TYPE_BEACON_TABLE; */
				continue;
			} else {
				/* unknown beacon request, do not reply */
				continue;
			}
			break;
		case CCXv4_RM_TYPE_PATHLOSS:
		case CCX_RM_TYPE_PATHLOSS:
			if ((ci->wlc->cfg->current_bss->ccx_version <= 4 &&
				ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
				(ci->wlc->cfg->current_bss->ccx_version > 4 &&
				ie->type == CCX_RM_TYPE_PATHLOSS))
				type = WLC_RM_TYPE_PATHLOSS;
			else
				continue;
			break;
		default:
			/* unknown measurement type, do not reply */
			continue;
			break;
		}

		req[idx].type = type;
		req[idx].flags |= parallel_flag;
		req[idx].token = ltoh16(ie->token);
		req[idx].chanspec = CH20MHZ_CHSPEC(ie->channel);
		req[idx].dur = ltoh16(ie->duration);
		/* special handling for beacon table */
		if (type == WLC_RM_TYPE_BEACON_TABLE) {
			req[idx].chanspec = 0;
			req[idx].dur = 0;
		}
		else if (type == WLC_RM_TYPE_PATHLOSS) {
			pathloss_req = (ccx_rm_pathlossreq_t *)&(ie->channel);

			bzero(pathloss_data, sizeof(ccx_rm_pl_data_t));
			pathloss_data->nbursts = ltoh16(pathloss_req->nbursts);
			pathloss_data->burst_interval = ltoh16(pathloss_req->burstinterval);
			pathloss_data->burst_interval *= 1000;
			pathloss_data->burst_len = pathloss_req->burstlen;
			pathloss_data->duration = ltoh16(pathloss_req->duration);

			pathloss_data->req_txpower = pathloss_req->txpower;

			pathloss_data->nchannels = pathloss_req->nchannels;
			bcopy((char *)&pathloss_req->addr, (char *)&pathloss_data->da,
				sizeof(struct ether_addr));
			if (pathloss_data->nchannels > CCX_RMPATHLOSS_CHANNELMAX)
				pathloss_data->nchannels = CCX_RMPATHLOSS_CHANNELMAX;

			bcopy(&pathloss_req->channel[0], pathloss_data->channels,
				pathloss_data->nchannels);

			req[idx].chanspec = CH20MHZ_CHSPEC(pathloss_req->channel[0]);
			req[idx].dur = 0;

			bcopy(&req[idx], &pathloss_data->req, sizeof(wlc_rm_req_t));
		}

		/* set the parallel bit now that we output a request item so
		 * that other reqs in the same parallel set will have the bit
		 * set. The flag will be cleared above when a new parallel set
		 * starts.
		 */
		parallel_flag = WLC_RM_FLAG_PARALLEL;

		idx++;
	}
}

#ifdef BCMDBG
static void
wlc_ccx_rm_req_dump(wlc_ccx_info_t *ci, struct ether_addr* da, struct ether_addr* sa,
	ccx_rm_req_t* req_pkt, int req_len)
{
	ccx_rm_req_ie_t *ie;
	int buflen;
	char buf[2*ETHER_ADDR_LEN + 1];

	printf("CCX Radio Measure Request len=%d\n", req_len);
	bcm_format_hex(buf, da->octet, ETHER_ADDR_LEN);
	printf("DA: %s\n", buf);
	bcm_format_hex(buf, sa->octet, ETHER_ADDR_LEN);
	printf("SA: %s\n", buf);
	if (req_len < CCX_RM_REQ_LEN) {
		bcm_format_hex(buf, (int8*)req_pkt, req_len);
		printf("RM packet too short\n");
		printf("RM contents: %s\n", buf);
		return;
	}

	printf("RM token:  0x%04x\n", ltoh16(req_pkt->token));
	printf("RM delay:  %d TBTT\n", req_pkt->delay);
	printf("RM offset: %d TU\n", req_pkt->offset);

	buflen = req_len - CCX_RM_REQ_LEN;
	ie = (ccx_rm_req_ie_t*)req_pkt->data;

	while (buflen > 0) {
		wlc_ccx_rm_req_ie_dump(ci, ie, buflen);

		/* advance to the next IE */
		if (buflen > CCX_RM_IE_HDR_LEN) {
			buflen -= CCX_RM_IE_HDR_LEN + ltoh16(ie->len);
			ie = (ccx_rm_req_ie_t*)((int8*)ie + CCX_RM_IE_HDR_LEN + ltoh16(ie->len));
		} else {
			break;
		}
	}

	return;
}

static void
wlc_ccx_rm_req_ie_dump(wlc_ccx_info_t *ci, ccx_rm_req_ie_t *ie, int buflen)
{
	char buf[2*ETHER_ADDR_LEN + 1];
	const char *type_name;
	int8 *variable_buf;
	int variable_len;
	bool simple_req;

	if (buflen == 0)
		return;

	if (buflen < CCX_RM_IE_HDR_LEN) {
		bcm_format_hex(buf, (int8*)ie, buflen);
		printf("RM IE partial elt at end of packet, %d bytes: %s\n", buflen, buf);
		return;
	}

	if (ltoh16(ie->id) == CCX_RM_ID_REQUEST) {
		printf("RM IE ID %d (RM Request), len %d\n", ltoh16(ie->id), ltoh16(ie->len));
	} else {
		printf("RM IE ID %d (Unknown), len %d\n", ltoh16(ie->id), ltoh16(ie->len));
	}

	if (buflen < CCX_RM_IE_HDR_LEN + ltoh16(ie->len)) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, buflen - CCX_RM_IE_HDR_LEN);
		printf("RM IE partial elt at end of packet, %d bytes in data: %s\n",
		       buflen - CCX_RM_IE_HDR_LEN, buf);
		return;
	}

	/* check for unknown Request IE ID */
	if (ltoh16(ie->id) != CCX_RM_ID_REQUEST) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, ltoh16(ie->len));
		printf("RM IE contents: %s\n", buf);
		return;
	}

	/* we have a Radio Measurement request IE */

	/* check for minimum fixed size */
	if (ltoh16(ie->len) < CCX_RM_REQ_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN) {
		bcm_format_hex(buf, (int8*)ie + CCX_RM_IE_HDR_LEN, ltoh16(ie->len));
		printf("RM IE too short, expected at least %d bytes of IE data\n",
		       CCX_RM_REQ_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN);
		printf("RM IE contents: %s\n", buf);
		return;
	}

	printf("RM IE token: 0x%04x\n", ltoh16(ie->token));

	if (ie->mode & CCX_RM_MODE_ENABLE) {
		printf("RM IE mode:  0x%02x (autonomous reqs)\n", ie->mode);
		simple_req = FALSE;
	} else {
		printf("RM IE mode:  0x%02x\n", ie->mode);
		simple_req = TRUE;
	}

	switch (ie->type) {
	case CCX_RM_TYPE_LOAD:
		type_name = "(Channel Load)";
		break;
	case CCX_RM_TYPE_NOISE:
		type_name = "(Channel Noise)";
		break;
	case CCX_RM_TYPE_FRAME:
		type_name = "(Frame)";
		break;
	case CCX_RM_TYPE_BEACON:
		type_name = "(Beacon)";
		break;
	case CCXv4_RM_TYPE_PATHLOSS:
	case CCX_RM_TYPE_PATHLOSS:
		if ((ci->wlc->cfg->current_bss->ccx_version <= 4 &&
			ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
			(ci->wlc->cfg->current_bss->ccx_version > 4 &&
			ie->type == CCX_RM_TYPE_PATHLOSS))
			type_name = "(Pathloss)";
		else
			type_name = "(unknown)";
		break;
	default:
		type_name = "(unknown)";
		simple_req = FALSE;
		break;
	}
	printf("RM IE type:  0x%02x %s\n", ie->type, type_name);

	variable_buf = (int8*)ie + CCX_RM_REQ_IE_FIXED_LEN;
	variable_len = (ltoh16(ie->len) + CCX_RM_IE_HDR_LEN) - CCX_RM_REQ_IE_FIXED_LEN;

	if ((ci->wlc->cfg->current_bss->ccx_version <= 4 &&
		ie->type == CCXv4_RM_TYPE_PATHLOSS) ||
		(ci->wlc->cfg->current_bss->ccx_version > 4 &&
		ie->type == CCX_RM_TYPE_PATHLOSS)) {
		return;
	}

	if (!simple_req) {
		bcm_format_hex(buf, variable_buf, variable_len);
		printf("RM IE unexpected extra measure info: %s\n", buf);
		return;
	}

	if (variable_len < 4) {
		bcm_format_hex(buf, variable_buf, variable_len);
		printf("RM IE measure info too short, expected at least %d bytes of measure data:"
			" %s\n", 4, buf);
		return;
	}

	printf("RM IE channel:  %4d\n", ie->channel);
	printf("RM IE param:    0x%02x\n", ie->param);
	printf("RM IE duration: %4d\n", ltoh16(ie->duration));

	if (variable_len > 4) {
		bcm_format_hex(buf, variable_buf + 4, variable_len - 4);
		printf("RM IE unexpected extra measure info: %s\n", buf);
	}

	return;
}
#endif /* BCMDBG */

static int
wlc_ccx_rm_ie_count(ccx_rm_req_ie_t* ie, int len)
{
	int count = 0;

	/* make sure this is a valid CCX RM IE */
	if (len < CCX_RM_IE_HDR_LEN ||
	    len < CCX_RM_IE_HDR_LEN + ltoh16(ie->len)) {
		ie = NULL;
	}

	/* walk the req IEs counting valid RM Requests,
	 * skipping unknown IEs or ones that just have autonomous report flags
	 */
	while (ie) {
		if (ltoh16(ie->id) == CCX_RM_ID_REQUEST &&
		    ltoh16(ie->len) >= CCX_RM_REQ_IE_FIXED_LEN &&
		    0 == (ie->mode & CCX_RM_MODE_ENABLE)) {
			/* found a measurement request */
			count++;
		}

		ie = wlc_ccx_rm_next_ie(ie, &len);
	}

	return count;
}

static ccx_rm_req_ie_t*
wlc_ccx_rm_next_ie(ccx_rm_req_ie_t* ie, int* len)
{
	int buflen = *len;
	int ie_len;

	/* expect a valid CCX RM IE */
	ASSERT(buflen >= CCX_RM_IE_HDR_LEN);
	ASSERT(ie && (buflen >= CCX_RM_IE_HDR_LEN + ltoh16(ie->len)));

	ie_len = ltoh16(ie->len);

	do {
		/* advance to the next IE */
		buflen -= CCX_RM_IE_HDR_LEN + ie_len;
		ie = (ccx_rm_req_ie_t*)((int8*)ie + CCX_RM_IE_HDR_LEN + ie_len);

		/* make sure there is room for a valid CCX RM IE */
		if (buflen < CCX_RM_IE_HDR_LEN ||
		    buflen < CCX_RM_IE_HDR_LEN + (ie_len = ltoh16(ie->len))) {
			buflen = 0;
			ie = NULL;
			break;
		}

		if (ltoh16(ie->id) == CCX_RM_ID_REQUEST &&
		    ie_len >= CCX_RM_REQ_IE_FIXED_LEN) {
			/* found valid measurement request */
			break;
		}
	} while (ie);

	*len = buflen;
	return ie;
}

/* add a new report block to the ccx report list */
static wlc_ccx_rm_rept_t*
wlc_ccx_rm_new_report(wlc_ccx_info_t *ci, uchar** buf, int* avail_len)
{
	wlc_ccx_rm_rept_t *report;
	wlc_ccx_rm_rept_t *new_report;

	*avail_len = 0;
	*buf = NULL;

	/* Allocate a new report */
	new_report = MALLOCZ(ci->pub->osh, sizeof(wlc_ccx_rm_rept_t));
	if (new_report == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return NULL;
	}

	*avail_len = CCX_RM_REP_DATA_MAX_LEN;
	*buf = new_report->data;

	/* and hook it into the ccx report list */
	report = ci->rm_reports;

	if (report == NULL) {
		/* first report in the linked list */
		ci->rm_reports = new_report;
	} else {
		/* put the new report at the end of the linked list */
		while (report->next != NULL)
			report = report->next;

		report->next = new_report;
	}

	return new_report;
}

void
wlc_ccx_rm_report(ccx_t *ccxh, wlc_rm_req_t *req_block, int count)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	ccx_rm_t *rm = ci->rm_state->ccx;
	wlc_rm_req_t *req;
	wlc_ccx_rm_rept_t *report;
	uchar *rep_ptr = NULL;
	uint8 type;
	uint bss_count;
	wlc_bss_info_t **bss_ptrs;
	int bss_reported;
	int buflen = 0;
	bool measured;
	int i;
	int err;

	/* ongoing count BSSs already reported in beacon table report */
	bss_count = rm->scan_results.count;
	bss_ptrs = rm->scan_results.ptrs;

	report = ci->rm_reports;
	if (report == NULL) {
		/* create first report block on list */
		report = wlc_ccx_rm_new_report(ci, &rep_ptr, &buflen);
		if (report == NULL)
			return;
	} else {
		/* the current open report is the last on the list */
		while (report->next != NULL)
			report = report->next;

		buflen = CCX_RM_REP_DATA_MAX_LEN - report->len;
		rep_ptr = report->data + report->len;
	}

	/* format each report based on the request info and measured data */
	for (i = 0; i < count; i++) {
		err = 0;
		type = 0;
		req = req_block + i;

		if ((req->flags & (WLC_RM_FLAG_LATE |
			WLC_RM_FLAG_INCAPABLE |
			WLC_RM_FLAG_REFUSED))) {
			measured = FALSE;
		} else {
			measured = TRUE;
		}

		switch (req->type) {
		case WLC_RM_TYPE_CCA:
			if (!measured) {
				type = CCX_RM_TYPE_LOAD;
				break;
			}

			err = wlc_ccx_rm_append_load_report(ci, req, &rep_ptr, &buflen);

			break;
		case WLC_RM_TYPE_RPI:
			if (!measured) {
				type = CCX_RM_TYPE_NOISE;
				break;
			}

			err = wlc_ccx_rm_append_noise_report(ci, req, &rep_ptr, &buflen);

			break;
		case WLC_RM_TYPE_BEACON_TABLE:
			/* we do not implement or respond to this type,
			 * set measured in case it was flase so that we do
			 * generate a reply.
			 */
			measured = TRUE;
			/* if we want to respond, we should mark as CCX_RM_MODE_INCAPABLE
			 * in wlc_ccx_rm_parse_requests, and just fold this case with
			 * WLC_RM_TYPE_PASSIVE_SCAN and WLC_RM_TYPE_ACTIVE_SCAN.
			 */
			break;
		case WLC_RM_TYPE_PASSIVE_SCAN:
		case WLC_RM_TYPE_ACTIVE_SCAN:
			if (!measured) {
				type = CCX_RM_TYPE_BEACON;
				break;
			}

			if (bss_count > 0) {
				bss_reported = wlc_ccx_rm_beacon_table(bss_ptrs, bss_count, req,
					&rep_ptr, &buflen);
				bss_count -= bss_reported;
				bss_ptrs += bss_reported;

				if (bss_count > 0) {
					/* set the error since we overflowed the buffer */
					err = 1;
				}
			}
			break;
		case WLC_RM_TYPE_FRAME:
			if (!measured) {
				type = CCX_RM_TYPE_FRAME;
				break;
			}

			err = wlc_ccx_rm_append_frame_report(ci, req, &rep_ptr, &buflen);

			break;
		default:
			/* set measured so that we do not respond, even if a refusal flag is set */
			measured = TRUE;
			break;
		}

		if (!measured) {
			if (buflen < CCX_RM_REP_IE_FIXED_LEN) {
				err = 1;
			} else {
				wlc_ccx_rm_append_report_header(type, (uint16)req->token,
					req->flags, 0, rep_ptr);
				rep_ptr += CCX_RM_REP_IE_FIXED_LEN;
				buflen -= CCX_RM_REP_IE_FIXED_LEN;
			}
		}

		/* check for a report buffer overflow */
		if (err) {
			/* update the report block that filled up */
			report->len = (uint)(CCX_RM_REP_DATA_MAX_LEN - buflen);

			/* create a new report block to continue writing the remainin measurements
			 */
			report = wlc_ccx_rm_new_report(ci, &rep_ptr, &buflen);
			if (report == NULL) {
				WL_INFORM(("wl%d: wlc_ccx_rm_report: Unable to get new "
					"report block exiting\n",
					ci->pub->unit));
				break;
			}
			/* roll back for loop to previous request */
			i--;
		}

		ASSERT(buflen <= CCX_RM_REP_DATA_MAX_LEN);
		ASSERT(rep_ptr <= report->data + CCX_RM_REP_DATA_MAX_LEN);
	}

	if (report != NULL)
		report->len = (uint)(CCX_RM_REP_DATA_MAX_LEN - buflen);
}

static void
wlc_ccx_rm_append_report_header(uint8 type, uint16 token, uint8 flags, int data_len, uchar *buf)
{
	ccx_rm_rep_ie_t rep;

	bzero(&rep, sizeof(rep));
	rep.id = htol16(CCX_RM_ID_REPORT);
	rep.len = htol16((CCX_RM_REP_IE_FIXED_LEN - CCX_RM_IE_HDR_LEN) + data_len);
	rep.token = htol16(token);
	rep.mode = 0;
	if (flags & WLC_RM_FLAG_PARALLEL)
		rep.mode |= CCX_RM_MODE_PARALLEL;
	if (flags & WLC_RM_FLAG_LATE)
		rep.mode |= CCX_RM_MODE_REFUSED;
	if (flags & WLC_RM_FLAG_INCAPABLE)
		rep.mode |= CCX_RM_MODE_INCAPABLE;
	if (flags & WLC_RM_FLAG_REFUSED)
		rep.mode |= CCX_RM_MODE_REFUSED;
	rep.type = type;

	bcopy(&rep, buf, CCX_RM_REP_IE_FIXED_LEN);
}

static int
wlc_ccx_rm_append_load_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	int rep_len;
	int data_len;
	uchar *data;
	ccx_rm_load_rep_t load;

	data_len = CCX_RM_LOAD_REP_LEN;
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_load_report: Channel Load report len %d "
			"does not fit, %d bytes left in report frame.\n",
			ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_LOAD, (uint16)req->token, req->flags, data_len,
		*bufptr);
	data = *bufptr + CCX_RM_REP_IE_FIXED_LEN;

	load.channel = CHSPEC_CHANNEL(req->chanspec);
	load.spare = 0;
	load.duration = (uint16)htol16(req->dur);
	load.fraction = rm_state->cca_busy;

	bcopy(&load, data, data_len);

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

static int
wlc_ccx_rm_append_noise_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	int rep_len;
	int data_len;
	uchar *data;
	int bin;
	ccx_rm_noise_rep_t noise;

	data_len = CCX_RM_NOISE_REP_LEN;
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_noise_report: Noise report len %d "
			   "does not fit, %d bytes left in report frame.\n",
			   ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_NOISE, (uint16)req->token, req->flags, data_len,
		*bufptr);
	data = *bufptr + CCX_RM_REP_IE_FIXED_LEN;

	noise.channel = CHSPEC_CHANNEL(req->chanspec);
	noise.spare = 0;
	noise.duration = (uint16)htol16(req->dur);

	for (bin = 0; bin < 8; bin++)
		noise.rpi[bin] = (uint8)rm_state->rpi[bin];

	bcopy(&noise, data, data_len);

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

static int
wlc_ccx_rm_append_frame_report(wlc_ccx_info_t *ci, wlc_rm_req_t *req, uchar **bufptr, int *buflen)
{
	ccx_rm_t *rm = ci->rm_state->ccx;
	int rep_len;
	int data_len;
	wlc_ccx_rm_frm_elt_t *frm_elt;
	ccx_rm_frame_rep_t *frame;
	ccx_rm_frm_rep_elt_t *frm_rep_elt;
	int i;

	data_len = CCX_RM_FRAME_REP_FIXED_LEN + (rm->frm_elts *
		CCX_RM_FRAME_REP_ENTRY_LEN);
	rep_len = CCX_RM_REP_IE_FIXED_LEN + data_len;

	if (*buflen < rep_len) {
		WL_INFORM(("wl%d: wlc_ccx_rm_append_frame_report: Frame report len %d "
			   "does not fit, %d bytes left in report frame.\n",
			   ci->pub->unit, rep_len, *buflen));
		return 1;
	}

	wlc_ccx_rm_append_report_header(CCX_RM_TYPE_FRAME, (uint16)req->token,
		req->flags, data_len, *bufptr);
	frame = (ccx_rm_frame_rep_t *)(*bufptr + CCX_RM_REP_IE_FIXED_LEN);

	frame->channel = CHSPEC_CHANNEL(req->chanspec);
	frame->spare = 0;
	frame->duration = (uint16)htol16(req->dur);

	frm_rep_elt = (ccx_rm_frm_rep_elt_t *)&frame->elt[0];
	for (i = 0; i < WLC_NRMFRAMEHASH; i++) {
		for (frm_elt = rm->frmhash[i]; frm_elt; frm_elt = frm_elt->next,
			frm_rep_elt++) {
			bcopy(&frm_elt->ta, &frm_rep_elt->ta, ETHER_ADDR_LEN);
			bcopy(&frm_elt->bssid, &frm_rep_elt->bssid, ETHER_ADDR_LEN);
			frm_rep_elt->rssi = frm_elt->rssi_sum / frm_elt->frames;
			frm_rep_elt->frames = frm_elt->frames;
		}
	}

	*buflen -= rep_len;
	*bufptr += rep_len;

	return 0;
}

/* The set of IEs from beacons and probe responses that will be included in a CCX beacon report */
static const uint8 ccx_rm_beacon_table_ies[] = {
	DOT11_MNG_SSID_ID,
	DOT11_MNG_RATES_ID,
	DOT11_MNG_FH_PARMS_ID,
	DOT11_MNG_DS_PARMS_ID,
	DOT11_MNG_CF_PARMS_ID,
	DOT11_MNG_TIM_ID,
	DOT11_MNG_IBSS_PARMS_ID,
	DOT11_MNG_EXT_RATES_ID
};

/* Writes the set of Beacon Measurement Report IEs corresponding to the 'results' array of
 * bss_info's.
 * Results will be truncated if there is not enough room in buf.
 * Returns the number of bss_info's that remain after filling the buffer.
 */
static int
wlc_ccx_rm_beacon_table(wlc_bss_info_t **results, uint num, wlc_rm_req_t *req, uchar **bufptr,
	int *buflen)
{
	uint i;
	int elt_len;
	int bcn_rep_len;
	uint8 channel;
	uint16 dur;
	uchar *buf;
	int len;
	uint8 report[MAXBSS];

	buf = *bufptr;
	len = *buflen;

	channel = CHSPEC_CHANNEL(req->chanspec);
	dur = (uint16)req->dur;

	if (req->type == WLC_RM_TYPE_PASSIVE_SCAN ||
		req->type == WLC_RM_TYPE_ACTIVE_SCAN) {
		memset(report, 1, num);
		wlc_ccx_rm_mbssid_report(results, report, num);
	}

	for (i = 0; i < num; i++) {
		if ((req->type == WLC_RM_TYPE_PASSIVE_SCAN ||
			req->type == WLC_RM_TYPE_ACTIVE_SCAN) && !report[i])
			continue;

		elt_len = CCX_RM_REP_IE_FIXED_LEN;
		if (len < elt_len)
			break;

		bcn_rep_len = wlc_ccx_rm_beacon_table_entry(results[i], channel, dur,
			buf + elt_len, len - elt_len);
		if (bcn_rep_len == 0)
			break;

		elt_len += bcn_rep_len;

		wlc_ccx_rm_append_report_header(CCX_RM_TYPE_BEACON, (uint16)req->token, req->flags,
			bcn_rep_len, buf);

		ASSERT(len >= elt_len);

		buf += elt_len;
		len -= elt_len;
	}

	if (i != num) {
		WL_INFORM(("wlc_ccx_rm_beacon_table: truncating beacon report after "
		           "%d of %d results\n", i, num));
	}

	*bufptr = buf;
	*buflen = len;

	return i;
}

/* Writes the Measurement Report portion of a Beacon Measurement Report IE to 'buf', length
 * 'buflen'.
 * Returns the length of the becon report written to 'buf'
 * If the buffer is not large enough to contain the report, a length of zero is returned.
 */
static int
wlc_ccx_rm_beacon_table_entry(wlc_bss_info_t *bi, uint8 channel, uint16 dur, uchar *buf, int buflen)
{
	uint8 id;
	uint i;
	bool match;
	int len;
	int copy_len;
	int bcn_len;
	bcm_tlv_t *bcn_ie;
	bcm_tlv_t *rep_ie;
	ccx_rm_beacon_rep_t rep;

	bzero(&rep, sizeof(rep));

	rep_ie = NULL;
	bcn_ie = NULL;

	len = CCX_RM_BEACON_REP_FIXED_LEN;
	rep_ie = (bcm_tlv_t *)(buf + len);

	if (len > buflen)
		return 0;

	ASSERT(bi != NULL);
	bcn_len = bi->bcn_prb_len;
	if (bi->bcn_prb && bcn_len > DOT11_BCN_PRB_LEN) {
		bcn_ie = (bcm_tlv_t*)((int8*)bi->bcn_prb + DOT11_BCN_PRB_LEN);
		bcn_len -= DOT11_BCN_PRB_LEN;
	}

	while (bcn_ie && bcn_len > TLV_HDR_LEN && bcn_len >= bcn_ie->len + TLV_HDR_LEN) {
		id = bcn_ie->id;

		/* check for an ie we are interested in */
		match = FALSE;
		for (i = 0; i < ARRAYSIZE(ccx_rm_beacon_table_ies); i++) {
			if (id == ccx_rm_beacon_table_ies[i]) {
				match = TRUE;
				break;
			}
		}
		/* more checks for PROP_IE for radio cap ie */
		if (id == DOT11_MNG_PROPR_ID &&
		    bcn_ie->len >= (DOT11_OUI_LEN + 1) &&
		    !bcmp(((ccx_radio_mgmt_t*)bcn_ie)->oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN) &&
		    ((ccx_radio_mgmt_t*)bcn_ie)->ver == 1)
			match = TRUE;

		if (match) {
			copy_len = bcn_ie->len + TLV_HDR_LEN;
			/* TIM and CF IEs get truncated */
			if (id == DOT11_MNG_TIM_ID) {
				/* TIM must be trucated to 4 bytes */
				copy_len = MIN(4, copy_len);
			} else if (id == DOT11_MNG_CF_PARMS_ID) {
				/* CF IE must be trucated to 6 bytes */
				copy_len = MIN(6, copy_len);
			}

			len += copy_len;

			if (len > buflen)
				return 0;

			/* copy the orig ie from the saved bcn to the report ies */
			ASSERT(buflen >= copy_len + ((uchar*)rep_ie - buf));
			bcopy(bcn_ie, rep_ie, copy_len);
			/* adjust the rep_ie length if we trucated the bcn ie */
			if (copy_len != bcn_ie->len + TLV_HDR_LEN)
				rep_ie->len = copy_len - TLV_HDR_LEN;
			/* advance to the next report ie position */
			rep_ie = (bcm_tlv_t*)((int8*)rep_ie + copy_len);
		}

		/* advance to the next bcn ie */
		bcn_len -= bcn_ie->len + TLV_HDR_LEN;
		bcn_ie = (bcm_tlv_t*)(bcn_ie->data + bcn_ie->len);
	}

	ASSERT(buflen >= (uchar*)rep_ie - buf);

	rep.channel = channel;
	rep.spare = 0;
	rep.duration = htol16(dur);
	rep.phy_type = bi->ccx_phy_type;
	rep.rssi = (int8)bi->RSSI;
	bcopy(bi->BSSID.octet, rep.bssid.octet, ETHER_ADDR_LEN);
	rep.parent_tsf = bi->rx_tsf_l;
	if (bi->bcn_prb && bi->bcn_prb_len >= DOT11_BCN_PRB_LEN) {
		bcopy(bi->bcn_prb->timestamp, &rep.target_tsf_low, 2*sizeof(uint32));
		bcopy(&bi->bcn_prb->beacon_interval, &rep.beacon_interval, sizeof(uint16));
		bcopy(&bi->bcn_prb->capability, &rep.capability, sizeof(uint16));
	} else {
		rep.target_tsf_low = 0;
		rep.target_tsf_hi = 0;
		rep.beacon_interval = 0;
		rep.capability = 0;
	}

	bcopy(&rep, buf, CCX_RM_BEACON_REP_FIXED_LEN);

	return len;
}

/* send all the collected report blocks in sequence */
void
wlc_ccx_rm_end(ccx_t *ccxh)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rm_req_state_t* rm_state = ci->rm_state;
	wlc_ccx_rm_rept_t *report;

	while ((report = ci->rm_reports) != NULL) {
		wlc_ccx_rm_send_measure_report(ci, &wlc->cfg->BSSID,
		                               (uint16)rm_state->token,
		                               report->data, report->len);
		if (!report->len) {
			WL_INFORM(("wl%d: wlc_ccx_rm_end: empty CCX RM report block\n",
				ci->pub->unit));
		}

		ci->rm_reports = report->next;
		MFREE(ci->pub->osh, report, sizeof(wlc_ccx_rm_rept_t));
	}

	return;
}

static void
wlc_ccx_rm_send_measure_report(wlc_ccx_info_t *ci, struct ether_addr *da,
	uint16 token, uint8 *report, uint report_len)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_llc_snap_header *snap;
	ccx_iapp_hdr_t* ccx_iapp;
	ccx_rm_rep_t* rm_rep;
	int prio;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	WL_INFORM(("wl%d: wlc_ccx_rm_send_measure_report: sending Measure Report (token %d) "
		"to %s\n", ci->pub->unit, token, bcm_ether_ntoa(da, eabuf)));

	/* Measure Report frame is
	 * 16 bytes Cisco/Aironet IAPP header
	 * 2 bytes RM Report header, the Dialog Token
	 * variable len report IEs
	 */
	body_len = DOT11_LLC_SNAP_HDR_LEN + CCX_IAPP_HDR_LEN + CCX_RM_REP_LEN + report_len;

	p = wlc_dataget(ci->wlc, da, (uint16)body_len, ETHER_HDR_LEN + body_len);
	if (p == NULL)
		return;

	pbody = (uint8*)PKTDATA(ci->pub->osh, p);

	snap = (struct dot11_llc_snap_header *)(pbody + ETHER_HDR_LEN);
	bcopy(CISCO_AIRONET_SNAP, snap, DOT11_LLC_SNAP_HDR_LEN);

	ccx_iapp = (ccx_iapp_hdr_t *)((int8*)snap + DOT11_LLC_SNAP_HDR_LEN);
	ccx_iapp->id_len = hton16(CCX_IAPP_ID_CONTROL | (body_len - DOT11_LLC_SNAP_HDR_LEN));
	ccx_iapp->type = CCX_IAPP_TYPE_RM;
	ccx_iapp->subtype = CCX_IAPP_SUBTYPE_ROAM_REP;
	bzero(ccx_iapp->da.octet, ETHER_ADDR_LEN);
	bcopy(ci->pub->cur_etheraddr.octet, ccx_iapp->sa.octet, ETHER_ADDR_LEN);

	rm_rep = (ccx_rm_rep_t*)ccx_iapp->data;
	rm_rep->token = htol16(token);

	bcopy(report, rm_rep->data, report_len);

	/* Set high priority so that the packet will get enqueued at the end of
	 * higher precedence queue in order
	 */
	prio = wlc_ccx_get_txpkt_prio(ci, da);
	PKTSETPRIO(p, prio);
	wlc_sendpkt(ci->wlc, p, NULL);
}

#ifdef CCX_SDK
static void
wlc_ccx_process_apname(wlc_ccx_info_t *ci, bcm_tlv_t *aironet_ie)
{
	ci->apname[0] = '\0';
	if (aironet_ie) {
		/* parse for AP Name in aironet ie */
		if (aironet_ie->len > (AIRONET_IE_NAME + AIRONET_IE_MAX_NAME_LEN)) {
			strncpy(ci->apname, (char*)&aironet_ie->data[AIRONET_IE_NAME],
				AIRONET_IE_MAX_NAME_LEN);
			ci->apname[AIRONET_IE_MAX_NAME_LEN - 1] = '\0';
		}
	}
}
#endif /* CCX_SDK */

static void
wlc_ccx_get_updated_timestamp(wlc_ccx_info_t *ci, struct dot11_bcn_prb *bcn_prb,
	uint32 rx_tsf_l, uint32 *tsf_l, uint32 *tsf_h)
{
	uint32 delta;

	wlc_read_tsf(ci->wlc, tsf_l, tsf_h);
	delta = *tsf_l - rx_tsf_l;
	WL_ASSOC(("wl%d: %s: JOIN: cckm reassoc timestamp delta %d us\n",
		ci->pub->unit, __FUNCTION__, delta));
	*tsf_l = ltoh32(bcn_prb->timestamp[0]);
	*tsf_h = ltoh32(bcn_prb->timestamp[1]);
	wlc_uint64_add(tsf_h, tsf_l, 0, delta);
}

/* Merge in IHV CCX IEs if any */
static uint
wlc_ccx_calc_ihv_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	uint len = 0;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return 0;

#ifdef CCX_SDK
	if (IHV_ENAB(&ccx->pubci)) {
		uint8 *elt;

		/* Merge in IHV CCX IEs if any */
		for (elt = ccx->pubci.ccx_ihv_ies;
		     elt && (elt < &ccx->pubci.ccx_ihv_ies[ccx->pubci.ccx_ihv_ies_len]);
		     elt += TLV_HDR_LEN + elt[1]) {
			/* Sanity check */
			if ((uint)(elt - ccx->pubci.ccx_ihv_ies + TLV_HDR_LEN + elt[1]) >
			    ccx->pubci.ccx_ihv_ies_len)
				break;
			if (ccx->pubci.ccx_v4_only) {
				ccx_version_ie_t *ver_ie;
				uint8 ccx_version_msgtype = CCX_VERSION_IE_TYPE;
				uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;

				/* if v4 only and version IE, set version to 4 */
				if ((ver_ie = (ccx_version_ie_t *)
				    bcm_find_vendor_ie(elt, TLV_HDR_LEN + elt[1], CISCO_AIRONET_OUI,
				      &ccx_version_msgtype, sizeof(ccx_version_msgtype))) != NULL)
					;
				else
				if (bcm_find_vendor_ie(elt, TLV_HDR_LEN + elt[1], CISCO_AIRONET_OUI,
				      &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype))) {
					/* if v4 only, no SFA IE */
					continue;
				}
			}
			len += TLV_HDR_LEN + elt[1];
		}
	}
#endif /* CCX_SDK */

	return len;
}

static int
wlc_ccx_write_ihv_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return BCME_OK;

#ifdef CCX_SDK
	if (IHV_ENAB(&ccx->pubci)) {
		uint8 *elt;

		/* Merge in IHV CCX IEs if any */
		for (elt = ccx->pubci.ccx_ihv_ies;
		     elt && (elt < &ccx->pubci.ccx_ihv_ies[ccx->pubci.ccx_ihv_ies_len]);
		     elt += TLV_HDR_LEN + elt[1]) {
			/* Sanity check */
			if ((uint)(elt - ccx->pubci.ccx_ihv_ies + TLV_HDR_LEN + elt[1]) >
			    ccx->pubci.ccx_ihv_ies_len)
				break;
			if (ccx->pubci.ccx_v4_only) {
				ccx_version_ie_t *ver_ie;
				uint8 ccx_version_msgtype = CCX_VERSION_IE_TYPE;
				uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;

				/* if v4 only and version IE, set version to 4 */
				if ((ver_ie = (ccx_version_ie_t *)
				    bcm_find_vendor_ie(elt, TLV_HDR_LEN + elt[1], CISCO_AIRONET_OUI,
				      &ccx_version_msgtype, sizeof(ccx_version_msgtype))) != NULL)
					ver_ie->version = 4;
				else
				if (bcm_find_vendor_ie(elt, TLV_HDR_LEN + elt[1], CISCO_AIRONET_OUI,
				      &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype))) {
					/* if v4 only, no SFA IE */
					continue;
				}
			}
			bcopy(elt, data->buf, TLV_HDR_LEN + elt[1]);
			data->buf += TLV_HDR_LEN + elt[1];
		}
	}
#endif /* CCX_SDK */

	return BCME_OK;
}

/* CCXv2 Version IE */
static uint
wlc_ccx_calc_ver_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return 0;

	if (ftcbparm->assocreq.target->ccx_version)
		return TLV_HDR_LEN + ccx_version_ie[TLV_LEN_OFF];

	return 0;
}

static int
wlc_ccx_write_ver_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network) {
		return BCME_OK;
	}

	if (ftcbparm->assocreq.target->ccx_version) {
		bcm_copy_tlv(ccx_version_ie, data->buf);
	}

	return BCME_OK;
}

static int
wlc_ccx_scan_parse_ver_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	ccx_version_ie_t *ccx_ver_ie = (ccx_version_ie_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (ccx_ver_ie == NULL)
		return BCME_OK;

	if (ccx_ver_ie->len == CCX_VERSION_IE_LEN)
		bi->ccx_version = ccx_ver_ie->version;

	if (ccx_ver_ie->version >= 5) {
		ccx_sfa_ie_t *ccx_sfa_ie;
		uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;

		ccx_sfa_ie = (ccx_sfa_ie_t *)
		     bcm_find_vendor_ie(data->buf, data->buf_len, CISCO_AIRONET_OUI,
		                        &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype));
		if (ccx_sfa_ie == NULL ||
		    ccx_sfa_ie->length != sizeof(ccx_sfa_ie_t) - TLV_HDR_LEN) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
			wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
			wlc_info_t *wlc = ccx->wlc;
#endif // endif
			/* a valid SFA IE must be included.  If not, discard frame */
			WL_INFORM(("wl%d: %s: SFA IE is not found or length is invalid\n",
				wlc->pub->unit, __FUNCTION__));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

/* CCXv2 Radio Management Capability IE */
static uint
wlc_ccx_calc_rm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub))
		return 0;

	if (data->ft == FC_PROBE_REQ) {
		rm_info_t *rmi = wlc->rm_info;
		wlc_rm_req_state_t *rms = rmi->rm_state;

		if (rms->step == WLC_RM_IDLE ||
		    rms->req[rms->cur_req].type != WLC_RM_TYPE_ACTIVE_SCAN)
			return 0;
	}
	else if (!ccx->pubci.ccx_network)
		return 0;

	if (ccx->pubci.rm)
		return TLV_HDR_LEN + ccx_rm_capability_ie[TLV_LEN_OFF];

	return 0;
}

static int
wlc_ccx_write_rm_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;

	if (!CCX_ENAB(wlc->pub))
		return BCME_OK;

	if (data->ft == FC_PROBE_REQ) {
		rm_info_t *rmi = wlc->rm_info;
		wlc_rm_req_state_t *rms = rmi->rm_state;

		if (rms->step == WLC_RM_IDLE ||
		    rms->req[rms->cur_req].type != WLC_RM_TYPE_ACTIVE_SCAN)
			return BCME_OK;
	}
	else if (!ccx->pubci.ccx_network) {
		return BCME_OK;
	}

	if (ccx->pubci.rm) {
		bcm_copy_tlv(ccx_rm_capability_ie, data->buf);
	}

	return BCME_OK;
}

/* CCKM IE */
static uint
wlc_ccx_calc_cckm_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return 0;

#ifdef CCX_SDK
	if (!(IHV_ENAB(&ccx->pubci) && ccx->pubci.ccx_ihv_ies)) {
#endif /* CCX_SDK */
		if (data->ft == FC_REASSOC_REQ &&
		    IS_CCKM_AUTH(cfg->WPA_auth) && WLC_PORTOPEN(cfg)) {
			return sizeof(cckm_reassoc_req_ie_t);
		}
#ifdef CCX_SDK
	}
#endif /* CCX_SDK */

	return 0;
}

static bool
wlc_ccx_reassocreq_ins_cb(void *ctx, wlc_iem_ins_data_t *data)
{
	return FALSE;
}

static bool
wlc_ccx_reassocreq_mod_cb(void *ctx, wlc_iem_mod_data_t *data)
{
	return FALSE;
}

static uint8
wlc_ccx_reassocreq_vsie_cb(void *ctx, wlc_iem_cbvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

static int
wlc_ccx_get_wpa_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, uint buf_len)
{
	wlc_iem_uiel_t uiel, *puiel;
	wlc_assoc_t *as;

	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	uint ielen;
	int ret;

	if (!wlc || !cfg || !buf)
		return BCME_BADARG;

	as = cfg->assoc;

	puiel = NULL;
	if (as->ie != NULL) {
		bzero(&uiel, sizeof(uiel));
		uiel.ies = as->ie;
		uiel.ies_len = as->ie_len;
		uiel.ins_fn = wlc_ccx_reassocreq_ins_cb;
		uiel.mod_fn = wlc_ccx_reassocreq_mod_cb;
		uiel.vsie_fn = wlc_ccx_reassocreq_vsie_cb;
		uiel.ctx = wlc;
		puiel = &uiel;
	}

	bzero(&ftcbparm, sizeof(ftcbparm));
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	bzero(buf, buf_len);

	ret = BCME_NOTFOUND;
	if ((ielen = wlc_iem_vs_calc_ie_len(wlc->iemi, cfg, FC_REASSOC_REQ,
		WLC_IEM_VS_IE_PRIO_WPA,	puiel, &cbparm)) != 0) {
		if (ielen <= buf_len)
			ret = wlc_iem_vs_build_ie(wlc->iemi, cfg, FC_REASSOC_REQ,
				WLC_IEM_VS_IE_PRIO_WPA, puiel, &cbparm, buf, buf_len);
		else
			ret = BCME_BUFTOOSHORT;
	}
	return ret;
}

static int
wlc_ccx_write_cckm_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return BCME_OK;

#ifdef CCX_SDK
	if (!(IHV_ENAB(&ccx->pubci) && ccx->pubci.ccx_ihv_ies)) {
#endif /* CCX_SDK */
		/* Driver maintains rn value for external supplicant */
		if (!BSS_SUP_ENAB_WPA(wlc->idsup, cfg) && data->ft != FC_REASSOC_REQ &&
		    IS_CCKM_AUTH(cfg->WPA_auth) && !WLC_PORTOPEN(cfg)) {
			ccx->rn = 1;
		}
		if (data->ft == FC_REASSOC_REQ &&
		    IS_CCKM_AUTH(cfg->WPA_auth) && WLC_PORTOPEN(cfg)) {
			wpa_ie_fixed_t *wpa_ie;
			uint32 tsf_l, tsf_h;
			cckm_reassoc_req_ie_t *cckmie;
			uint8 iebuf[30] = {0};

			wpa_ie = NULL;
			if (cfg->WPA_auth == WPA2_AUTH_CCKM)
				wpa_ie = (wpa_ie_fixed_t*)ftcbparm->assocreq.wpa2_ie;
			else
				if (wlc_ccx_get_wpa_ie(wlc, cfg, iebuf, sizeof(iebuf)) == BCME_OK)
					wpa_ie = (wpa_ie_fixed_t*)iebuf;
			if (wpa_ie == NULL) {
				WL_ERROR(("wl%d: %s: no WPA or WPA2 IE\n",
				          ccx->pub->unit, __FUNCTION__));
				return BCME_ERROR;
			}

			/* copy WPA info element template */
			cckmie = (cckm_reassoc_req_ie_t *)data->buf;

			bcm_copy_tlv(CCKM_info_element, data->buf);

			wlc_ccx_get_updated_timestamp(ccx, bi->bcn_prb,
			                              bi->rx_tsf_l, &tsf_l, &tsf_h);
			/* fill in CCKM reassoc req IE */
#if defined(BCMINTSUP)
		if (BSS_SUP_ENAB_WPA(wlc->idsup, cfg))
				wlc_cckm_gen_reassocreq_IE(ccx->wlc->ccxsup, cfg, cckmie,
					tsf_h, tsf_l, &bi->BSSID, wpa_ie);
		else
#endif /* BCMINTSUP */
			{
			uint32 rn;
			uint32 timestamp[2];

			timestamp[0] = htol32(tsf_l);
			timestamp[1] = htol32(tsf_h);

			/* load timestamp from bcn_prb (< 1s) */
			bcopy(timestamp, cckmie->timestamp, DOT11_MNG_TIMESTAMP_LEN);

			/* increment and load RN */
			rn = ++ccx->rn;
			/* 80211 uses le byte order */
			rn = htol32(rn);
			bcopy((char *)&rn, (char *)&cckmie->rn, sizeof(rn));

			/* calculate and load MIC */
			wlc_cckm_calc_reassocreq_MIC(cckmie, &bi->BSSID, wpa_ie,
			                             &ftcbparm->assocreq.scb->ea,
			                             ccx->rn, ccx->key_refresh_key,
			                             cfg->WPA_auth);
			}
		}
#ifdef CCX_SDK
	}
#endif /* CCX_SDK */

	return BCME_OK;
}

/* Aironet IE */
static uint
wlc_ccx_calc_aironet_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return 0;

#ifdef CCX_SDK
	if (!(IHV_ENAB(&ccx->pubci) && ccx->pubci.ccx_ihv_ies)) {
#endif /* CCX_SDK */
		if (bi->aironet_ie_rx) {
			return sizeof(aironet_assoc_ie_t);
		}
#ifdef CCX_SDK
	}
#endif /* CCX_SDK */

	return 0;
}

static int
wlc_ccx_write_aironet_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bss_info_t *bi = ftcbparm->assocreq.target;

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return BCME_OK;

#ifdef CCX_SDK
	if (!(IHV_ENAB(&ccx->pubci) && ccx->pubci.ccx_ihv_ies)) {
#endif /* CCX_SDK */
		if (bi->aironet_ie_rx) {
			aironet_assoc_ie_t dev_ie;

			/* add Machine Name to (re)association request message */
			bzero(&dev_ie, sizeof(aironet_assoc_ie_t));
			dev_ie.id = DOT11_MNG_AIRONET_ID;
			dev_ie.len = sizeof(aironet_assoc_ie_t)-TLV_HDR_LEN;
			dev_ie.device = AIRONET_IE_DEVICE_ID;
			dev_ie.refresh_rate = KEEP_ALIVE_INTERVAL;
			dev_ie.flags = (CKIP_KP | CKIP_MIC);
			strcpy(dev_ie.name, ccx->staname);
			bcopy((uint8 *)&dev_ie, data->buf, sizeof(aironet_assoc_ie_t));
#ifdef BCMDBG
			WL_WSEC(("wl%d: %s: JOIN: requesting full CKIP support\n",
			         ccx->pub->unit, __FUNCTION__));
#endif /* BCMDBG */
		}
#ifdef CCX_SDK
	}
#endif /* CCX_SDK */

	return BCME_OK;
}

static int
wlc_ccx_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
#ifdef CCX_SDK
	wlc_iem_ft_pparm_t *ftpparm;
	struct scb *scb;
#endif // endif

	if (!CCX_ENAB(wlc->pub) || !ccx->pubci.ccx_network)
		return BCME_OK;

	if (data->ie == NULL)
		return BCME_OK;

#ifdef CCX_SDK
	switch (data->ft) {
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		ASSERT(data->pparm != NULL);
		ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);
		scb = ftpparm->assocresp.scb;
		ASSERT(scb != NULL);
		wlc_ccx_init_ckip(ccx, (bcm_tlv_t *)data->ie, data->cfg);
		wlc_ccx_process_apname(ccx, (bcm_tlv_t *)data->ie);
		if (IHV_ENAB(&ccx->pubci))
			wlc_ccx_feature_info(ccx, data->buf, data->buf_len);
		scb->flags2 &= ~SCB2_CCX_MFP;
		break;
	}
#endif /* CCX_SDK */

	return BCME_OK;
}

static int
wlc_ccx_scan_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (ie == NULL)
		return BCME_OK;

	/* CCX parameters */
	bi->aironet_ie_rx = FALSE;

	if (ie->len > AIRONET_IE_CKIP) {
		bi->ckip_support = ie->data[AIRONET_IE_CKIP] & (CKIP_MIC | CKIP_KP);
		bi->aironet_ie_rx = TRUE;
	}

	return BCME_OK;
}

static int
wlc_ccx_scan_parse_qbss_load_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	dot11_qbss_load_ie_t *qbss_load_ie = (dot11_qbss_load_ie_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	bi->qbss_load_chan_free = WLC_QBSS_LOAD_CHAN_FREE_LEGACY_AP;
	bi->qbss_load_aac = WLC_QBSS_LOAD_AAC_LEGACY_AP;

	if (qbss_load_ie == NULL)
		return BCME_OK;

	if (qbss_load_ie->length == (sizeof(dot11_qbss_load_ie_t) - TLV_HDR_LEN)) {
		/* convert channel utilization to channel free score */
		bi->qbss_load_chan_free =
		        (uint8)WLC_QBSS_LOAD_CHAN_FREE_MAX - qbss_load_ie->channel_utilization;
		bi->qbss_load_aac = ltoh16_ua((uint8 *)&qbss_load_ie->aac);
	}

	return BCME_OK;
}

static int
wlc_ccx_bcn_parse_qos_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	ccx_qos_params_t *ccx_qos_ie = (ccx_qos_params_t *)data->ie;
	uint16 cwmin, cwmax;

	if (ccx_qos_ie != NULL) {
		if ((ccx_qos_ie->len == CCX_QOS_IE_LEN) &&
		    (ccx_qos_ie->count > cfg->current_bss->ccx_qos_params_cnt)) {
			cwmin = (2 << ((ccx_qos_ie->ecw_0 & 0xf) - 1)) - 1;
			cwmax = (2 << (((ccx_qos_ie->ecw_0 >> 4) & 0xf) - 1)) - 1;
			wlc_set_cwmin(wlc, cwmin);
			wlc_set_cwmax(wlc, cwmax);
			cfg->current_bss->ccx_qos_params_cnt = ccx_qos_ie->count;
		}
	}

	return BCME_OK;
}

static int
wlc_ccx_bcn_parse_aironet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ccx_info_t *ccx = (wlc_ccx_info_t *)ctx;
	wlc_info_t *wlc = ccx->wlc;
	bcm_tlv_t *ckip_ie = (bcm_tlv_t *)data->ie;
	uint16 cwmin, cwmax;

	if (ckip_ie != NULL) {
		/* track for cwmin/cwmax in BCMCCX beacon, update haredware if necessary.
		 * CWMin and CWMax are uint16 values at offset AIRONET_IE_CWMIN and
		 * AIRONET_IE_CWMAX, make sure the ie length covers both uint16s before
		 * using
		 */
		if (ckip_ie->len > (AIRONET_IE_CWMAX + 1)) {
			cwmin = ltoh16_ua(&ckip_ie->data[AIRONET_IE_CWMIN]);
			cwmax = ltoh16_ua(&ckip_ie->data[AIRONET_IE_CWMAX]);

			/* validate cwmin and cwmax.  if validation fails, do not adopt
			 * either of them
			 */
			if (VALID_MASK(cwmin) && VALID_MASK(cwmax)) {
				if (cwmin != wlc->band->CWmin)
					wlc_set_cwmin(wlc, cwmin);
				if (cwmax != wlc->band->CWmax)
					wlc_set_cwmax(wlc, cwmax);
			} else
				WL_ERROR(("wl%d: %s: invalid cwmin or cwmax"
					" value\n", wlc->pub->unit, __FUNCTION__));
		}
	}

	return BCME_OK;
}

#ifdef CCX_SDK
/* indicate event to IHV */
static int
wlc_ccx_indicate_event(wlc_ccx_info_t *ci, int event, PTLV_STRUCT tlvs, uint tlv_cnt)
{
	uint i, len;
	PIHV_CCX_TLV tlv;
	PCCX_NIC_SPECIFIC_EXTENSION indication;
	wl_info_t *wl = ci->wlc->wl;

	ASSERT(tlvs && tlv_cnt);

	len = CCX_NIC_SPEC_EXT_HDR_LEN + CCX_TLV_DATA_HDR_LEN;
	for (i = 0; i < tlv_cnt; i++)
		len += CCX_TLV_HDR_LEN + tlvs[i].length;

	if (!(indication = (PCCX_NIC_SPECIFIC_EXTENSION)MALLOCZ(ci->pub->osh, len))) {
		WL_ERROR(("wl%d: %s: indication out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return BCME_NOMEM;
	}

	*(uint32*)indication->oui = OID_BCM_CCX;
	indication->event = event;
	indication->tlvData.tlvCount = tlv_cnt;

	tlv = (PIHV_CCX_TLV)&indication->tlvData.tlv[0];
	for (i = 0; i < tlv_cnt; i++) {
		tlv->type = tlvs[i].type;
		tlv->length = tlvs[i].length;
		bcopy(tlvs[i].value, tlv->value, tlv->length);

		WL_INFORM(("wl%d: %s: type %s, len %d indicated\n",
			ci->pub->unit, __FUNCTION__,
			wlc_ccx_tlvcode2str(tlv->type), tlv->length));

		tlv = (PIHV_CCX_TLV)((uint8*)tlv + CCX_TLV_HDR_LEN + tlv->length);
	}

	shared_indicate_status(wl, wl->sh.adapterhandle, 0,
		NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, indication, len);

	MFREE(ci->pub->osh, indication, len);

	return BCME_OK;
}

/* indicate NDIS status to IHV which MSM not forward to */
void
wlc_ccx_ind_ndis_status(ccx_t *ccxh, NDIS_STATUS status, void *status_buf, uint buf_len)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bss_info_t *bi;
	TLV_STRUCT tlv;
	PIHV_CCX_TLV_DATA tlv_data;
	PDOT11_ASSOCIATION_START_PARAMETERS params;
	uint data_len;
	wlc_bsscfg_t *cfg = wlc->cfg;

	/* do not indicate status for ibss or AP mode */
	if (!BSSCFG_STA(cfg) || !cfg->BSS)
		return;

	tlv.type = status;
	tlv.length = buf_len;
	tlv.value = status_buf;

	if (status == NDIS_STATUS_DOT11_ASSOCIATION_START) {
		ASSERT(buf_len == sizeof(DOT11_ASSOCIATION_START_PARAMETERS));
		/* include length of beacon/probe response frame tlv */
		bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
		data_len = CCX_TLV_DATA_HDR_LEN + CCX_TLV_HDR_LEN + bi->bcn_prb_len;
		tlv.length += data_len;

		if (!(tlv.value = MALLOC(ci->pub->osh, tlv.length))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
			return;
		}

		/* copy DOT11_ASSOCIATION_START_PARAMETERS */
		bcopy(status_buf, tlv.value, buf_len);
		params = (PDOT11_ASSOCIATION_START_PARAMETERS)tlv.value;
		params->uIHVDataOffset = buf_len;
		params->uIHVDataSize = data_len;

		/* include beacon/probe response frame tlv */
		tlv_data = (PIHV_CCX_TLV_DATA)((uint8*)params + params->uIHVDataOffset);
		tlv_data->tlvCount = 1;
		tlv_data->tlv[0].type = CCX_TLV_BEACON_IHV_DATA;
		tlv_data->tlv[0].length = bi->bcn_prb_len;
		bcopy(bi->bcn_prb, tlv_data->tlv[0].value, bi->bcn_prb_len);

		if (IS_CCKM_AUTH(cfg->WPA_auth) && cfg->assoc->type != AS_ASSOCIATION) {
			struct dot11_bcn_prb *bcn_prb;
			uint32 tsf_l, tsf_h;
			bcn_prb = (struct dot11_bcn_prb*)tlv_data->tlv[0].value;
			/* update timestamp for CCKM roam */
			wlc_ccx_get_updated_timestamp(ci, bcn_prb, bi->rx_tsf_l, &tsf_l, &tsf_h);
			bcn_prb->timestamp[0] = htol32(tsf_l);
			bcn_prb->timestamp[1] = htol32(tsf_h);
		}
	}

	if (wlc_ccx_indicate_event(ci, CCX_EVENT_STATUS_INDICATION, &tlv, 1) != BCME_OK) {
		WL_ERROR(("wl%d: %s: NDIS status %s indication failed\n",
			ci->pub->unit, __FUNCTION__, wlc_ccx_tlvcode2str(status)));
	} else
		WL_INFORM(("wl%d: %s: NDIS status %s indicated\n",
		ci->pub->unit, __FUNCTION__, wlc_ccx_tlvcode2str(status)));

	if (tlv.value != status_buf)
		MFREE(ci->pub->osh, tlv.value, tlv.length);
}

/* indicate IAPP packet which driver not handled to IHV */
static void
wlc_ccx_ind_iapp_pkt(wlc_ccx_info_t *ci, struct dot11_header *d11_pkt,
	uint d11_pkt_len, wlc_pkttag_t *pkttag)
{
	TLV_STRUCT tlv[2];
	DOT11_EXTSTA_RECV_CONTEXT rx;
	wl_oid_t *wl = ((wl_info_t *)ci->wlc->wl)->oid;

	/* d11 pkt tlv */
	tlv[0].type = CCX_TLV_802_11_PACKET;
	tlv[0].length = d11_pkt_len;
	tlv[0].value = d11_pkt;

	/* RX_CHARACTERISTICS tlv */
	tlv[1].type = CCX_TLV_RX_CHARACTERISTICS;
	tlv[1].length = sizeof(DOT11_EXTSTA_RECV_CONTEXT);
	tlv[1].value = (void *)&rx;

	/* fill out DOT11_EXTSTA_RECV_CONTEXT */
	bzero(&rx, sizeof(DOT11_EXTSTA_RECV_CONTEXT));
	rx.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	rx.Header.Revision = DOT11_EXTSTA_RECV_CONTEXT_REVISION_1;
	rx.Header.Size = sizeof(DOT11_EXTSTA_RECV_CONTEXT);
	rx.uPhyId = wl->cur_phy_id;
	rx.lRSSI = pkttag->pktinfo.misc.rssi;
	rx.ucDataRate = (uint8)RSPEC2KBPS(pkttag->rspec)/500;
	rx.uChCenterFrequency = (uint32)wf_channel2mhz(pkttag->rxchannel,
		(pkttag->rxchannel <= CH_MAX_2G_CHANNEL) ?
		WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);

	if (wlc_ccx_indicate_event(ci, CCX_EVENT_PACKET_RECEIVED, tlv, 2) != BCME_OK) {
		WL_ERROR(("wl%d: %s: indication IAPP frame failed\n",
			ci->pub->unit, __FUNCTION__));
	} else
		WL_INFORM(("wl%d: %s: an IAPP frame indicated\n", ci->pub->unit, __FUNCTION__));
}

/* indicate tx-ed IHV packet status */
static int
wlc_ccx_ind_tx_status(wlc_ccx_info_t *ci, uint txstatus, bool sent,
	bool max_retries, uint ihv_pkt_idx)
{
	TLV_STRUCT tlv;
	IHV_TX_CHARACTERISTICS tx_chars;
	uint tx_frame_count;
	int status;

	/* CCX_TLV_TX_CHARACTERISTICS tlv */
	tlv.type = CCX_TLV_TX_CHARACTERISTICS;
	tlv.length = sizeof(IHV_TX_CHARACTERISTICS);
	tlv.value = &tx_chars;

	/* fill out IHV_TX_CHARACTERISTICS */
	bzero(&tx_chars, sizeof(IHV_TX_CHARACTERISTICS));
	wlc_read_tsf(ci->wlc, (uint32*)&tx_chars.tsf, (uint32*)&tx_chars.tsf + 1);
	tx_frame_count = (txstatus & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT;
	tx_chars.retryCount = (tx_frame_count > 1) ? (tx_frame_count - 1) : 0;
	if (max_retries)
		tx_chars.txFlags |= CCX_TX_FLAG_MAX_RETRY;
	if (!sent)
		tx_chars.txFlags |= CCX_TX_FLAG_FAILED;

	tx_chars.hSrvLibCompletion = ci->ihv_pkt_arg[ihv_pkt_idx];

	status = wlc_ccx_indicate_event(ci, CCX_EVENT_STATUS_INDICATION, &tlv, 1);

	if (status != BCME_OK) {
		WL_ERROR(("wl%d: %s: indicating transmitted frame status failed\n",
			ci->pub->unit, __FUNCTION__));
	} else
		WL_INFORM(("wl%d: %s: a transmitted frame status indicated\n",
			ci->pub->unit, __FUNCTION__));

	return status;
}

/* indicate d11 tx packet completion */
static void
wlc_ccx_ind_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)arg;
	void *p = ci->pubci.ihv_txpkt;

	/* indicate tx-ed pkt status */
	wlc_ccx_ind_tx_status(ci, txstatus, ci->pubci.ihv_txpkt_sent,
		ci->pubci.ihv_txpkt_max_retries,
		(WLPKTTAG(p)->flags & WLF_TX_PKT_IDX_MASK) >> WLF_TX_PKT_IDX_SHIFT);

	/* reset parameters */
	ci->pubci.ihv_txpkt = NULL;
	ci->pubci.ihv_txpkt_sent = FALSE;
	ci->pubci.ihv_txpkt_max_retries = FALSE;
}

/* send 802.11 packet for IHV */
static int
wlc_ccx_send_d11_pkt(wlc_ccx_info_t *ci, void *d11_pkt, uint32 d11_pkt_len,
	void *ihv_reserved)
{
	void *p;
	struct ether_header *eh;
	struct dot11_header *h = (struct dot11_header *)d11_pkt;
	int prio;

	if (d11_pkt_len < DOT11_A3_HDR_LEN) {
		WL_ERROR(("wl%d: %s: bad d11 packet\n", ci->pub->unit, __FUNCTION__));
		return BCME_BADLEN;
	}

#ifdef BCMDBG
	if (WL_INFORM_ON()) {
		WL_INFORM(("dump 802.11 mac header: \n"));
		wlc_print_dot11_mac_hdr(d11_pkt, d11_pkt_len);
		prhex("dump 802.11 packet", d11_pkt, d11_pkt_len);
	}
#endif // endif

	/* must be from current STA */
	if (bcmp(&ci->pub->cur_etheraddr, &h->a2, ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d: %s: abort sending data due to a wrong STA address\n",
			ci->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	if ((p = PKTGET(ci->pub->osh, d11_pkt_len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: PKTGET error for len %d\n",
			ci->pub->unit, __FUNCTION__, (d11_pkt_len + TXOFF)));
		WLCNTINCR(ci->pub->_cnt->txnobuf);
		return BCME_NOMEM;
	}
	/* reserve TXOFF bytes of headroom */
	PKTPULL(ci->pub->osh, p, TXOFF);

	/* copy packet */
	bcopy(d11_pkt, PKTDATA(ci->pub->osh, p), d11_pkt_len);
	PKTSETLEN(ci->pub->osh, p, d11_pkt_len);

	/* XXX - it looks current CCX lib does not provide previous ap
	 * information in Adjacent AP Report.  use our information until
	 * the issue solved
	 */
	if (ci->prev_channel && d11_pkt_len >= (sizeof(ccx_roam_iapp_pkt_t) -
		ETHER_HDR_LEN + DOT11_A3_HDR_LEN)) {
		struct dot11_llc_snap_header *snap = (struct dot11_llc_snap_header *)
			((uint8*)PKTDATA(ci->pub->osh, p) + DOT11_A3_HDR_LEN);
		if (!bcmp((uint8*)snap, CISCO_AIRONET_SNAP, DOT11_LLC_SNAP_HDR_LEN)) {
			uint8 *p = (uint8*)(snap + 1);
			if (p[2] == CCXv2_IAPP_TYPE_ROAM) {
				ccx_roam_ap_ie_t *ie = (ccx_roam_ap_ie_t*)&p[16];
				wlc_bsscfg_t *cfg = ci->wlc->cfg;
				bcopy(ci->prev_ap_mac.octet, ie->mac.octet, ETHER_ADDR_LEN);
				ie->channel = hton16(ci->prev_channel);
				ie->ssid_len = hton16(cfg->current_bss->SSID_len);
				bcopy(cfg->current_bss->SSID, ie->ssid,
					cfg->current_bss->SSID_len);
				ie->disassoc_time = hton16((uint16)ci->disassoc_time);
				/* currently looks no NDIS status for TSPEC rejection for ihv */
				if (cfg->current_bss->ccx_version >= 4 &&
					(d11_pkt_len >= (sizeof(ccx_roam_iapp_pkt_t) - ETHER_HDR_LEN
					+ DOT11_A3_HDR_LEN + sizeof(ccx_roam_reason_ie_t)))) {
					ccx_roam_reason_ie_t *reason_ie =
						(ccx_roam_reason_ie_t*)((uint8*)(ie + 1));
					WL_ASSOC(("%s(): CCX lib reason_code = %d\n",
						__FUNCTION__, reason_ie->reason));
					reason_ie->reason = wlc_ccx_get_roam_reason(ci);
				}
			}
		}
	}

	wl_tx_convert_d11hdr(ci->wlc->wl, p, &eh);

	/* save ihv reserved field(ihv tx completion handler) */
	ci->ihv_pkt_arg[ci->ihv_pkt_arg_idx] = ihv_reserved;
	WLPKTTAG(p)->flags |= WLF_IHV_TX_PKT;
	WLPKTTAG(p)->flags |= ci->ihv_pkt_arg_idx << WLF_TX_PKT_IDX_SHIFT;

	/* Set high priority so that the packet will get enqueued at the end of
	 * higher precedence queue in order
	 */
	prio = wlc_ccx_get_txpkt_prio(ci, (struct ether_addr*)eh->ether_dhost);
	PKTSETPRIO(p, prio);

	wlc_pcb_fn_register(ci->wlc->pcb, wlc_ccx_ind_tx_complete, ci, p);

	wlc_sendpkt(ci->wlc, p, NULL);

	if (++ci->ihv_pkt_arg_idx == IHV_TX_Q_SIZE)
		ci->ihv_pkt_arg_idx = 0;

	WL_INFORM(("wl%d: %s: send an IHV packet\n", ci->pub->unit, __FUNCTION__));
	return BCME_OK;
}

/* log received frame and indicate it to IHV */
uint
wlc_ccx_log_rx_frame(ccx_t *ccxh, uint8 *d11_pkt, uint d11_pkt_len, struct wl_rxsts *rxsts)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wl_oid_t *wl = ((wl_info_t *)ci->wlc->wl)->oid;
	DOT11_EXTSTA_RECV_CONTEXT rx_ctxt;
	TLV_STRUCT tlv[2];
	uint status;

	/* d11 pkt tlv */
	tlv[0].type = CCX_TLV_802_11_FRAME;
	tlv[0].length = d11_pkt_len;
	tlv[0].value = d11_pkt;

	/* CCX_TLV_RX_CHARACTERISTICS */
	tlv[1].type = CCX_TLV_RX_CHARACTERISTICS;
	tlv[1].length = sizeof(DOT11_EXTSTA_RECV_CONTEXT);
	tlv[1].value = &rx_ctxt;

	/* fill out DOT11_EXTSTA_RECV_CONTEXT */
	bzero(&rx_ctxt, sizeof(DOT11_EXTSTA_RECV_CONTEXT));
	rx_ctxt.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	rx_ctxt.Header.Revision = DOT11_EXTSTA_RECV_CONTEXT_REVISION_1;
	rx_ctxt.Header.Size = sizeof(DOT11_EXTSTA_RECV_CONTEXT);
	rx_ctxt.uPhyId = wl->cur_phy_id;
	rx_ctxt.lRSSI = rxsts->signal;
	rx_ctxt.ucDataRate = (uint8)rxsts->datarate;
	if (CHSPEC_IS2G(rxsts->chanspec)) {
		rx_ctxt.uChCenterFrequency =
		        wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec), WF_CHAN_FACTOR_2_4_G);
	} else {
		rx_ctxt.uChCenterFrequency =
		        wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec), WF_CHAN_FACTOR_5_G);
	}
	if (rxsts->pkterror & WL_RXS_CRC_ERROR)
		rx_ctxt.uReceiveFlags |= DOT11_RECV_FLAG_RAW_PACKET_FCS_FAILURE;

	status = wlc_ccx_indicate_event(ci, CCX_EVENT_PACKET_RECEIVED, tlv, 2);

#ifdef BCMDBG
	if (status == BCME_OK) {
		if (WL_PRPKT_ON())
			printf("wl%d: %s: a logged rx frame indicated\n",
				ci->pub->unit, __FUNCTION__);
	} else
		WL_ERROR(("wl%d: %s: a logged rx frame indication failed\n",
			ci->pub->unit, __FUNCTION__));
#endif /* BCMDBG */

	return status;
}

/* log transmitted frame and indicate it to IHV */
uint
wlc_ccx_log_tx_frame(ccx_t *ccxh, uint8 *d11_pkt, uint d11_pkt_len, uint txstatus,
	bool max_retries, bool sent)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	IHV_TX_CHARACTERISTICS tx_chars;
	TLV_STRUCT tlv[2];
	uint tx_frame_count;
	int status;

	/* pkt tlv */
	tlv[0].type = CCX_TLV_802_11_FRAME;
	tlv[0].length = d11_pkt_len;
	tlv[0].value = d11_pkt;

	/* CCX_TLV_TX_CHARACTERISTICS tlv */
	tlv[1].type = CCX_TLV_TX_CHARACTERISTICS;
	tlv[1].length = sizeof(IHV_TX_CHARACTERISTICS);
	tlv[1].value = &tx_chars;

	/* fill out IHV_TX_CHARACTERISTICS */
	bzero(&tx_chars, sizeof(IHV_TX_CHARACTERISTICS));
	wlc_read_tsf(ci->wlc, (uint32*)&tx_chars.tsf, (uint32*)&tx_chars.tsf + 1);
	tx_frame_count = (txstatus & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT;
	tx_chars.retryCount = (tx_frame_count > 1) ? (tx_frame_count - 1) : 0;
	if (max_retries)
		tx_chars.txFlags |= CCX_TX_FLAG_MAX_RETRY;
	if (!sent)
		tx_chars.txFlags |= CCX_TX_FLAG_FAILED;

	status = wlc_ccx_indicate_event(ci, CCX_EVENT_PACKET_TRANSMITTED, tlv, 2);

#ifdef BCMDBG
	if (status == BCME_OK) {
		if (WL_PRPKT_ON())
			printf("wl%d: %s: a logged tx frame indicated\n",
				ci->pub->unit, __FUNCTION__);
	} else
		WL_ERROR(("wl%d: %s: a logged tx frame indication failed\n",
			ci->pub->unit, __FUNCTION__));
#endif /* BCMDBG */

	return status;
}

/* build driver neighbor ap list from the ihv's list */
static void
wlc_ccx_ihv2drvr_ap_list(wlc_ccx_info_t *ci, PCCX_NEIGHBOR_LIST ihv_ap_list,
	bool append)
{
	uint i;
	uint len;
	PCCX_NL_ELEMENT nl;
	ccx_neighbor_rept_ie_t *rept_ie, *ie;
	uint ie_len;

	WL_ASSOC(("wl%d: %s: append flag is %s\n", ci->pub->unit, __FUNCTION__,
		append ? "set" : "cleared"));

	if (ihv_ap_list->length < CCX_NEIGHBOR_LIST_HDR_LEN) {
		WL_ERROR(("wl%d: %s: Invalid neighbor list\n", ci->pub->unit, __FUNCTION__));
		return;
	}

	/* calculate total length needed to build ccx_neighbor_rept_ie_t IEs */
	ie_len = 0;
	len = ihv_ap_list->length - CCX_NEIGHBOR_LIST_HDR_LEN;
	nl = &ihv_ap_list->ccxElement[0];
	for (i = 0; i < ihv_ap_list->numOfEntries; i++) {
		if (len < sizeof(CCX_NL_ELEMENT) || len < nl->length)
			break;
		if (!ETHER_ISNULLADDR(nl->BSSID)) {
			ie_len += sizeof(ccx_neighbor_rept_ie_t);
			if (nl->subElemOffset[CCX_NL_SUBELEM_INDEX_RF_PARAMS])
				ie_len += sizeof(ccx_radio_param_subie_t);
		}
		len -= nl->length;
		nl = (PCCX_NL_ELEMENT)((uint8*)nl + nl->length);
	}
	if (!ie_len)
		return;
	if ((rept_ie = (ccx_neighbor_rept_ie_t *)MALLOCZ(ci->pub->osh, ie_len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
		return;
	}
	ie = rept_ie;

	len = ihv_ap_list->length - CCX_NEIGHBOR_LIST_HDR_LEN;
	nl = &ihv_ap_list->ccxElement[0];
	for (i = 0; i < ihv_ap_list->numOfEntries; i++) {
		if (len < sizeof(CCX_NL_ELEMENT) || len < nl->length)
			break;
		if (!ETHER_ISNULLADDR(nl->BSSID)) {
			rept_ie->id = CCX_ROAM_NEIGHBOR_REPT_ID;
			rept_ie->len = CCX_NEIGHBOR_REPT_IE_LEN - TLV_HDR_LEN;
			rept_ie->channel = nl->channel;
			rept_ie->band = nl->channelBand;
			rept_ie->phy_type = nl->phyType;
			bcopy((char *)&nl->BSSID, (char *)&rept_ie->mac, ETHER_ADDR_LEN);
			if (nl->subElemOffset[CCX_NL_SUBELEM_INDEX_RF_PARAMS]) {
				PCCX_NL_SUBELEM_RFPARAMS rf;
				ccx_radio_param_subie_t *rf_ie;
				rf = (PCCX_NL_SUBELEM_RFPARAMS)((uint8*)nl +
					nl->subElemOffset[CCX_NL_SUBELEM_INDEX_RF_PARAMS]);
				rf_ie = (ccx_radio_param_subie_t*)((uint8*)rept_ie +
					CCX_NEIGHBOR_REPT_IE_LEN);
				/* build rf ie */
				rf_ie->sub_id = CCX_ROAM_SUB_RF_PARAMS;
				rf_ie->len = CCX_RADIO_PARAM_SUBIE_LEN - TLV_HDR_LEN;
				rf_ie->min_rssi = rf->minRxSignalPower;
				rf_ie->ap_tx_pwr = rf->APTxPower;
				rf_ie->roam_time = rf->transitionTime;
				rf_ie->roam_trigger = rf->adaptScanThreshold;
				rf_ie->roam_delta = rf->roamingHysteresis;

				rept_ie->len += CCX_RADIO_PARAM_SUBIE_LEN;
			}

			rept_ie = (ccx_neighbor_rept_ie_t*)((uint8*)rept_ie + TLV_HDR_LEN +
				rept_ie->len);
		}

		len -= nl->length;
		nl = (PCCX_NL_ELEMENT)((uint8*)nl + nl->length);
	}

	/* process ihv neighbor ap list */
	wlc_ccx_process_roam_iapp(ci, ie, ie_len, append);

	MFREE(ci->pub->osh, ie, ie_len);
}

/* CCX query oid */
static NDIS_STATUS
wlc_ccx_query_oid(wlc_ccx_info_t *ci, void *wl_ctx, PIHV_QUERY_INFO req, uint len)
{
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_rev_info_t revinfo;
	CCX_OID oid = req->Oid;
	void *info_buf = req->InformationBuffer;
	uint info_len = req->InformationBufferLength;
	uint *bytes_written = &req->BytesWritten;
	uint *bytes_needed = &req->BytesNeeded;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	char buf[100];
	bool wlif_off;
	wlc_bsscfg_t *cfg = wlc->cfg;

	WL_TRACE(("wl%d: %s: oid 0x%x\n", ci->pub->unit, __FUNCTION__, oid));

	*bytes_written = 0;
	*bytes_needed = 0;

	/* validate request length */
	if (len < (CCX_QUERY_INFO_HDR_LEN + info_len)) {
		status = NDIS_STATUS_INVALID_LENGTH;
		WL_ERROR(("wl%d: %s: invalid query oid request length\n",
			ci->pub->unit, __FUNCTION__));
		goto exit_query_oid;
	}

	WL_OID(("wl%d: %s: OID: querying %s buflen %d\n",
		ci->pub->unit, __FUNCTION__, wlc_ccx_oid2str(oid), info_len));

	wlif_off = (DEVICEREMOVED(ci->wlc) || !ci->pub->up);

	switch (oid) {
	case OID_CCX_STA_STATISTICS_2: {
		PCCX_STA_STATS_GRP_2 stat2 = (PCCX_STA_STATS_GRP_2)info_buf;
		if (info_len < sizeof(CCX_STA_STATS_GRP_2)) {
			*bytes_needed = sizeof(CCX_STA_STATS_GRP_2);
			break;
		}
		memset(stat2, 0xff, sizeof(CCX_STA_STATS_GRP_2));

		if (WSEC_AES_ENABLED(cfg->wsec))
			stat2->RSNAStatsSelectedPairwiseCipher = DOT11_CIPHER_ALGO_CCMP;
		else if (WSEC_TKIP_ENABLED(cfg->wsec))
			stat2->RSNAStatsSelectedPairwiseCipher = DOT11_CIPHER_ALGO_TKIP;
		stat2->RSNAStatsTKIPICVErrors =
			WLCNTVAL(ci->pub->_cnt->tkipicverr);
		stat2->RSNAStatsTKIPLocalMICFailures =
			WLCNTVAL(ci->pub->_cnt->tkipmicfaill);
		stat2->RSNAStatsTKIPReplays =
			WLCNTVAL(ci->pub->_cnt->tkipreplay);
		stat2->RSNAMgmtStatsTKIPICVErrors =
			WLCNTVAL(ci->pubci.mgmt_cnt.tkipmgmticverr);
		stat2->RSNAMgmtStatsTKIPLocalMICFailures =
			WLCNTVAL(ci->pubci.mgmt_cnt.tkipmgmtlocalmicfail);
		stat2->RSNAMgmtStatsTKIPReplays =
			WLCNTVAL(ci->pubci.mgmt_cnt.tkipmgmtreplayerr);
		stat2->RSNAMgmtStatsTKIPMHDRErrors =
			WLCNTVAL(ci->pubci.mgmt_cnt.tkipmgmtmhdrerr);
		stat2->RSNAMgmtStatsTKIPNoEncryptErrors =
			WLCNTVAL(ci->pubci.mgmt_cnt.tkipmgmtnoencrypterr);
		stat2->RSNAStatsCCMPReplays =
			WLCNTVAL(ci->pub->_cnt->ccmpreplay);
		stat2->RSNAStatsCCMPDecryptErrors =
			WLCNTVAL(ci->pub->_cnt->ccmpundec);
		stat2->RSNAMgmtStatsCCMPReplays =
			WLCNTVAL(ci->pubci.mgmt_cnt.ccmpmgmtreplayerr);
		stat2->RSNAMgmtStatsCCMPDecryptErrors =
			WLCNTVAL(ci->pubci.mgmt_cnt.ccmpmgmtdecrypterr);
		stat2->RSNAMgmtStatsCCMPNoEncryptErrors =
			WLCNTVAL(ci->pubci.mgmt_cnt.ccmpmgmtnoenypterr);
		stat2->RSNAStatsBroadcastDisassociateCount =
			WLCNTVAL(ci->pubci.mgmt_cnt.bcastmgmtdisassoc);
		stat2->RSNAStatsBroadcastDeauthenticateCount =
			WLCNTVAL(ci->pubci.mgmt_cnt.bcastmgmtdeauth);
		stat2->RSNAStatsBroadcastActionFrameCount =
			WLCNTVAL(ci->pubci.mgmt_cnt.bcastmgmtaction);
		*bytes_written = sizeof(CCX_STA_STATS_GRP_2);
		break;
	}
	case OID_CCX_FW_VERSION: {
		PCCX_DRIVER_VERSION ver = (PCCX_DRIVER_VERSION)info_buf;
		wlc_ioctl(wlc, WLC_GET_REVINFO, &revinfo, sizeof(revinfo), NULL);
		sprintf(buf, "%d.%d", (revinfo.ucoderev >> 16) & 0xffff,
			revinfo.ucoderev & 0xffff);
		if (info_len < strlen(buf)) {
			*bytes_needed = strlen(buf);
			break;
		}
		bcopy(buf, ver->version, strlen(buf));
		ver->length = (CCXUINT16)strlen(buf);
		*bytes_written = sizeof(ver->length) + strlen(buf);
		break;
	}
	case OID_CCX_ANTENNA_DATA: {
		PCCX_ANTENNA_DATA at_data = (PCCX_ANTENNA_DATA)info_buf;
		if (info_len < sizeof(CCX_ANTENNA_DATA)) {
			*bytes_needed = sizeof(CCX_ANTENNA_DATA);
			break;
		}
		at_data->gain = (CCXUINT8)wlc->band->antgain;
		at_data->type = (CCXUINT8)CCX_ANTENNA_UNKNOWN;
		*bytes_written = sizeof(CCX_ANTENNA_DATA);
		break;
	}
	case OID_CCX_MANUFACTURER_SERIAL_NUM: {
		PCCX_MFG_SER_NUM serial_num = (PCCX_MFG_SER_NUM)info_buf;
		const char *sn = getvar(ci->pub->vars, "boardnum");
		if (sn == NULL) {
			status = NDIS_STATUS_FAILURE;
			break;
		}
		else if (info_len < strlen(sn)) {
			*bytes_needed = strlen(sn);
		}
		bcopy(sn, serial_num->serNum, strlen(sn));
		serial_num->length = (CCXUINT16)strlen(sn);
		*bytes_written = sizeof(serial_num->length) + strlen(sn);
		break;
	}
	case OID_CCX_MANUFACTURER_MODEL: {
		uint str_len;
		wl_info_t *wl = ci->wlc->wl;
		PCCX_MFG_MODEL model = (PCCX_MFG_MODEL)info_buf;
		str_len = strlen(wl->DriverDesc);
		if (info_len < (OFFSETOF(CCX_MFG_MODEL, model[0]) + str_len)) {
			*bytes_needed = (OFFSETOF(CCX_MFG_MODEL, model[0]) + str_len);
			break;
		}
		bcopy(wl->DriverDesc, model->model, str_len);
		model->length = (CCXUINT16)str_len;
		*bytes_written = sizeof(model->length) + str_len;
		break;
	}
	case OID_CCX_DRIVER_VERSION: {
		PCCX_DRIVER_VERSION ver = (PCCX_DRIVER_VERSION)info_buf;
		wlc_ioctl(wlc, WLC_GET_REVINFO, &revinfo, sizeof(revinfo), NULL);
		sprintf(buf, "%d.%d.%d.%d",
			(revinfo.driverrev >> 24) & 0xff,
			(revinfo.driverrev >> 16) & 0xff,
			(revinfo.driverrev >> 8) & 0xff,
			revinfo.driverrev & 0xff);
		if (info_len < strlen(buf)) {
			*bytes_needed = strlen(buf);
			break;
		}
		bcopy(buf, ver->version, strlen(buf));
		ver->length = (CCXUINT16)strlen(buf);
		*bytes_written = sizeof(ver->length) + strlen(buf);
		break;
	}
	case OID_CCX_LAST_BCN_TIME:
		if (info_len < (sizeof(uint32) * 2)) {
			*bytes_needed = sizeof(uint32) * 2;
			break;
		}
		if (BSSCFG_STA(cfg) && cfg->associated) {
			*(uint32*)info_buf = cfg->roam->tsf_l;
			*(uint32*)((uint32*)info_buf + 1) = cfg->roam->tsf_h;
			*bytes_written = sizeof(uint32) * 2;
		} else
			status = NDIS_STATUS_FAILURE;
		break;
	case OID_CCX_RADIO_CHANNELS: {
		wl_oid_t *wl = ((wl_info_t *)wlc->wl)->oid;
		uint phy_type = wl->cmn->phy.phy_id2type[wl->cur_phy_id];
		wl_uint32_list_t *list;
		char abbrev[WLC_CNTRY_BUF_SZ];
		uint i, buf_len;
		PCCX_RADIO_CHANNELS channel = (PCCX_RADIO_CHANNELS)info_buf;

		if (wlif_off) {
			WL_ERROR(("wl%d: %s: OID_CCX_RADIO_CHANNELS: failed since"
				" interface is off\n", ci->pub->unit,  __FUNCTION__));
			status = NDIS_STATUS_INVALID_STATE;
			break;
		}

		buf_len = sizeof(uint32) * (WL_NUMCHANSPECS + 1);
		list = (wl_uint32_list_t*)MALLOC(ci->pub->osh, buf_len);
		if (list == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				ci->pub->unit,  __FUNCTION__, MALLOCED(ci->pub->osh)));
			status = NDIS_STATUS_RESOURCES;
			break;
		}

		bzero(abbrev, WLC_CNTRY_BUF_SZ);
		list->count = 0;
		if (phy_type == PHY_TYPE_B || phy_type == PHY_TYPE_G ||
			((phy_type == PHY_TYPE_N || phy_type == PHY_TYPE_LP ||
			phy_type == PHY_TYPE_LCN || phy_type == PHY_TYPE_LCN40) &&
			BAND_2G(wl->cmn->phy.phy_band[wl->cur_phy_id])))
			/* get the supported chanspecs on 2G band */
			wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, TRUE, abbrev);
		else if (phy_type == PHY_TYPE_A ||
			((phy_type == PHY_TYPE_N || phy_type == PHY_TYPE_LP ||
			phy_type == PHY_TYPE_LCN || phy_type == PHY_TYPE_LCN40) &&
			BAND_5G(wl->cmn->phy.phy_band[wl->cur_phy_id])))
			/* get the supported chanspecs on 5G band */
			wlc_get_valid_chanspecs(wlc->cmi, list, WL_CHANSPEC_BW_20, FALSE, abbrev);
		else {
			WL_ERROR(("wl%d: %s: OID_CCX_RADIO_CHANNELS: no channel info\n",
				ci->pub->unit,  __FUNCTION__));
			MFREE(ci->pub->osh, (void*)list, len);
			status = NDIS_STATUS_FAILURE;
			ASSERT(0);
			break;
		}

		ASSERT(list->count < WL_NUMCHANSPECS);

		if (info_len < (sizeof(CCXUINT16) + (list->count * sizeof(CCXUINT8)))) {
			*bytes_needed = sizeof(CCXUINT16) + (list->count * sizeof(CCXUINT8));
			MFREE(ci->pub->osh, list, buf_len);
			break;
		}

		channel->length = list->count * sizeof(CCXUINT8);
		for (i = 0; i < list->count; i++)
			channel->channels[i] = CHSPEC_CHANNEL(list->element[i]);
		*bytes_written = sizeof(CCXUINT16) + channel->length;

		MFREE(ci->pub->osh, (void*)list, buf_len);
		break;
	}
	case OID_CCX_SERVICE_CAPABILITY: {
		PCCX_SERVICE_CAPABILITY cap = (PCCX_SERVICE_CAPABILITY)info_buf;
		if (info_len < sizeof(CCX_SERVICE_CAPABILITY)) {
			*bytes_needed = sizeof(CCX_SERVICE_CAPABILITY);
			break;
		}
		/* XXX - once chip with GPS feature is available,
		 * check chip type for GPS capability or future
		 * functionalities.
		 */
		/* support voice, streaming and interactive video */
		*cap = CAP_VOICE | CAP_UNIDIR_VIDEO | CAP_BIDIR_VIDEO;
		*bytes_written = sizeof(CCX_SERVICE_CAPABILITY);
		break;
	}
	case OID_CCX_TSF: {
		uint32 tsf_l, tsf_h;
		if (wlif_off) {
			WL_INFORM(("wl%d: %s: OID_CCX_TSF: failed since interface is off\n",
				ci->pub->unit,  __FUNCTION__));
			status = NDIS_STATUS_INVALID_STATE;
			break;
		}
		if (info_len < (sizeof(uint32) * 2)) {
			*bytes_needed = sizeof(uint32) * 2;
			break;
		}
		wlc_read_tsf(ci->wlc, &tsf_l, &tsf_h);
		*(uint32*)info_buf = tsf_l;
		*(uint32*)((uint32*)info_buf + 1) = tsf_h;
		*bytes_written = sizeof(uint32) * 2;
		break;
	}
	case OID_CCX_NUM_TX_BUFFER:
		if (info_len < sizeof(uint32)) {
			*bytes_needed = sizeof(uint32);
			break;
		}
		*(uint32*)info_buf = IHV_TX_Q_SIZE;
		*bytes_written = sizeof(uint32);
		break;
	default:
		status = wl_ndis_queryinfo((NDIS_HANDLE)wl_ctx, oid,
			info_buf, info_len, bytes_written, bytes_needed, NULL);
	}

exit_query_oid:
	if (*bytes_needed) {
		/* add IHV_QUERY_INFO header */
		*bytes_needed += CCX_QUERY_INFO_HDR_LEN;
		status = NDIS_STATUS_INVALID_LENGTH;
		WL_ERROR(("wl%d: %s: buffer size is too small.  need %d, got %d\n",
			ci->pub->unit, __FUNCTION__, *bytes_needed, info_len));
	}
	return status;
}

/* CCX set oid */
static NDIS_STATUS
wlc_ccx_set_oid(wlc_ccx_info_t *ci, void *wl_ctx, PIHV_SET_INFO req, uint len)
{
	CCX_OID oid = req->Oid;
	void *info_buf = req->InformationBuffer;
	uint info_len = req->InformationBufferLength;
	uint *bytes_read = &req->BytesRead;
	uint *bytes_needed = &req->BytesNeeded;
	int status = NDIS_STATUS_SUCCESS;
	bool wlif_off;
	wlc_bsscfg_t *cfg = ci->wlc->cfg;

	WL_TRACE(("wl%d: %s: oid 0x%x\n", ci->pub->unit, __FUNCTION__, oid));

	*bytes_read = 0;
	*bytes_needed = 0;

	/* validate request length */
	if (len < (CCX_SET_INFO_HDR_LEN + info_len)) {
		status = NDIS_STATUS_INVALID_LENGTH;
		WL_ERROR(("wl%d: %s: invalid set oid request length\n",
			ci->pub->unit, __FUNCTION__));
		goto exit_set_oid;
	}

	WL_OID(("wl%d: %s: OID: setting %s buflen %d\n",
		ci->pub->unit, __FUNCTION__, wlc_ccx_oid2str(oid), info_len));

	wlif_off = (DEVICEREMOVED(ci->wlc) || !ci->pub->up);

	switch (oid) {
	case OID_CCX_ASSOC_INFO: {
		PCCX_ASSOC_INFO assoc = (PCCX_ASSOC_INFO)info_buf;
		uint max_len;
		max_len = MAX(sizeof(CCX_ASSOC_INFO), (assoc->AssocIEOffset + assoc->AssocIELen));
		/* validate length */
		if (info_len < max_len || assoc->uSize < max_len) {
			*bytes_needed = max_len;
			WL_ERROR(("wl%d: %s: OID_CCX_ASSOC_INFO: invalid length\n",
				ci->pub->unit, __FUNCTION__));
			break;
		}
		/* check if in assoc wait state */
		if (cfg->assoc->state != AS_WAIT_IE) {
			status = NDIS_STATUS_REQUEST_ABORTED;
			WL_ERROR(("wl%d: %s: OID_CCX_ASSOC_INFO: ignored due to"
				" CCX association wait timeout\n",
					ci->pub->unit, __FUNCTION__));
			break;
		}
		/* check if bssid matches */
		if (bcmp((char*)&assoc->ccxBSSID,
			&ci->wlc->as_info->join_targets
			->ptrs[ci->wlc->as_info->join_targets_last]->BSSID,
			ETHER_ADDR_LEN)) {
			status = NDIS_STATUS_INVALID_DATA;
			WL_ERROR(("wl%d: %s: OID_CCX_ASSOC_INFO: bssid does not match\n",
				ci->pub->unit, __FUNCTION__));
			break;
		}

		ASSERT(ci->pubci.ccx_ihv_ies == NULL);

		/* done if no IEs */
		if (!(assoc->AssocIEOffset && assoc->AssocIELen))
			break;
		/* save a copy of the attached IEs */
		if (!(ci->pubci.ccx_ihv_ies = MALLOC(ci->pub->osh, assoc->AssocIELen))) {
			status = NDIS_STATUS_RESOURCES;
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				ci->pub->unit,  __FUNCTION__, MALLOCED(ci->pub->osh)));
		} else {
			bcopy((char*)assoc + assoc->AssocIEOffset, ci->pubci.ccx_ihv_ies,
				assoc->AssocIELen);
			ci->pubci.ccx_ihv_ies_len = assoc->AssocIELen;
			*bytes_read = assoc->AssocIEOffset + assoc->AssocIELen;
		}
		break;
	}
	case OID_CCX_ROAM: {
		/* called when 1. 802.1x auth fail and roam to next AP, 2. directed roam */
		PCCX_NEIGHBOR_LIST list = (PCCX_NEIGHBOR_LIST)info_buf;
		uint buf_len = info_len;
		bool append = FALSE;
		if (wlif_off) {
			WL_ERROR(("wl%d: %s: OID_CCX_ROAM: failed since interface is off\n",
				ci->pub->unit,  __FUNCTION__));
			status = NDIS_STATUS_INVALID_STATE;
			break;
		}
		if (info_len < CCX_NEIGHBOR_LIST_HDR_LEN) {
			*bytes_needed = CCX_NEIGHBOR_LIST_HDR_LEN;
			WL_ERROR(("wl%d: %s: OID_CCX_NEIGHBOR_LIST: invalid length\n",
				ci->pub->unit, __FUNCTION__));
			break;
		}
		while (buf_len > CCX_NEIGHBOR_LIST_HDR_LEN && buf_len >= list->length) {
			/* build driver neighbor ap list */
			wlc_ccx_ihv2drvr_ap_list(ci, list, append);
			buf_len -= list->length;
			list = (PCCX_NEIGHBOR_LIST)((uint8*)list + list->length);
			append = TRUE;
		}
		wlc_ccx_roam(&ci->pubci, WLC_E_REASON_DIRECTED_ROAM);
		*bytes_read = info_len;
		break;
	}
	case OID_CCX_NEIGHBOR_LIST: {
		PCCX_NEIGHBOR_LIST list = (PCCX_NEIGHBOR_LIST)info_buf;
		if (info_len < CCX_NEIGHBOR_LIST_HDR_LEN)
			*bytes_needed = CCX_NEIGHBOR_LIST_HDR_LEN;
		else if (info_len < list->length)
			*bytes_needed = list->length;
		if (*bytes_needed) {
			WL_ERROR(("wl%d: %s: OID_CCX_NEIGHBOR_LIST: invalid length\n",
				ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
			break;
		}
		/* build driver neighbor ap list */
		wlc_ccx_ihv2drvr_ap_list(ci, list, FALSE);
		*bytes_read = info_len;
		break;
	}
	case OID_CCX_RM_REQUEST:
		status = NDIS_STATUS_NOT_SUPPORTED;
		/* these features are decided to be supported by driver */
		WL_ERROR(("wl%d: %s: Unexpected OID 0x%x request\n",
			ci->pub->unit, __FUNCTION__, oid));
		ASSERT(0);
		break;
	case OID_CCX_KEEP_ALIVE_REFRESH:
		if (info_len == 0) {
			*bytes_needed = 1;
			WL_ERROR(("wl%d: %s: OID_CCX_KEEP_ALIVE_REFRESH: no input\n",
				ci->pub->unit,  __FUNCTION__, MALLOCED(ci->pub->osh)));
			break;
		}
		wlc_ap_keep_alive_count_update(ci->wlc, (uint16)(*(uint8*)info_buf));
		*bytes_read = 1;
		break;
	case OID_CCX_FRAME_LOGGING_MODE: {
		PCCX_FRAME_LOGGING_MODE frame_log = (PCCX_FRAME_LOGGING_MODE)info_buf;
		if (info_len < sizeof(CCX_FRAME_LOGGING_MODE)) {
			*bytes_needed = sizeof(CCX_FRAME_LOGGING_MODE);
			WL_ERROR(("wl%d: %s: OID_CCX_FRAME_LOGGING_MODE: invalid length\n",
				ci->pub->unit, __FUNCTION__));
			break;
		}

		if (frame_log->bEnabled) {
			/* no frame log in v4 only mode */
			if (!CCX_ENAB(ci->pub) || ci->pubci.ccx_v4_only) {
				status = NDIS_STATUS_NOT_SUPPORTED;
				break;
			}
			if (!ci->pubci.frame_log) {
				int monitor;
				WL_OID(("wl%d: %s: OID: enable frame log mode\n",
					ci->pub->unit, __FUNCTION__));
				/* save monitor mode before enabling it */
				wlc_ioctl(ci->wlc, WLC_GET_MONITOR, &ci->org_monitor,
					sizeof(ci->org_monitor), NULL);
				/* enable monitor mode */
				monitor = 1;
				wlc_ioctl(ci->wlc, WLC_SET_MONITOR, &monitor,
					sizeof(monitor), NULL);
				/* enable frame log */
				ci->pubci.frame_log = TRUE;
			}
		} else if (ci->pubci.frame_log) {
			WL_OID(("wl%d: %s: OID: disable frame log mode\n",
				ci->pub->unit, __FUNCTION__));
			/* restore monitor mode */
			wlc_ioctl(ci->wlc, WLC_SET_MONITOR, &ci->org_monitor,
				sizeof(ci->org_monitor), NULL);
			/* disable frame log */
			ci->pubci.frame_log = FALSE;
		}
		*bytes_read = sizeof(CCXBOOLEAN);
		break;
	}
	case OID_CCX_DIAGNOSTICS_MODE:
		if (info_len < sizeof(CCX_DIAG_MODE)) {
			*bytes_needed = sizeof(CCX_DIAG_MODE);
			WL_ERROR(("wl%d: %s: OID_CCX_DIAGNOSTICS_MODE: invalid length\n",
				ci->pub->unit, __FUNCTION__));
			break;
		}
		if (*(PCCX_DIAG_MODE)info_buf == CCX_DIAG_MODE_ON) {
			/* reject diag request if AP active */
			if (AP_ACTIVE(ci->wlc)) {
				WL_OID(("wl%d: %s: OID: reject diag request since AP is active\n",
					ci->pub->unit, __FUNCTION__));
				status = NDIS_STATUS_REQUEST_ABORTED;
				break;
			}
			/* no diag in v4 only mode */
			if (!CCX_ENAB(ci->pub) || ci->pubci.ccx_v4_only) {
				status = NDIS_STATUS_NOT_SUPPORTED;
				break;
			}
			if (!ci->pubci.diag_mode) {
				WL_OID(("wl%d: %s: OID: enable diag mode\n",
					ci->pub->unit, __FUNCTION__));
				/* save params to be changed in diag mode */
				ci->org_listen_interval = ci->wlc->bcn_li_bcn;
				/* enable diag mode */
				ci->pubci.diag_mode = TRUE;
			}
		} else if (ci->pubci.diag_mode) {
			WL_OID(("wl%d: %s: OID: disable diag mode\n", ci->pub->unit, __FUNCTION__));
			/* restore params changed in diag mode */
			ci->wlc->bcn_li_bcn = ci->org_listen_interval;
			if (ci->pub->up)
				wlc_bcn_li_upd(ci->wlc);
			/* disable diag mode */
			ci->pubci.diag_mode = FALSE;
		}
		*bytes_read = sizeof(CCX_DIAG_MODE);
		break;
	default:
		/* all non-ccx OIDs passed to NDIS OID handler */
		status = wl_ndis_setinfo((NDIS_HANDLE)wl_ctx, oid,
			info_buf, info_len, bytes_read, bytes_needed, NULL);
	}

exit_set_oid:
	if (*bytes_needed) {
		/* add IHV_SET_INFO header */
		*bytes_needed += CCX_SET_INFO_HDR_LEN;
		status = NDIS_STATUS_INVALID_LENGTH;
		WL_ERROR(("wl%d: %s: buffer size is too small.  need %d, got %d\n",
			ci->pub->unit, __FUNCTION__, *bytes_needed, info_len));
	}
	return status;
}

/* CCX method oid */
static NDIS_STATUS
wlc_ccx_method_oid(wlc_ccx_info_t *ci, void *wl_ctx, PIHV_METHOD_INFO req, uint len,
	void *oid_request)
{
	int status;

	if (len < CCX_METHOD_INFO_HDR_LEN ||
		req->InputBufferLength > (len - CCX_METHOD_INFO_HDR_LEN) ||
		req->InformationBufferLength > (len - CCX_METHOD_INFO_HDR_LEN)) {
		WL_ERROR(("wl%d: %s: invalid request length\n", ci->pub->unit, __FUNCTION__));
		return NDIS_STATUS_INVALID_LENGTH;
	}

	WL_TRACE(("wl%d: %s: oid 0x%x\n", ci->pub->unit, __FUNCTION__, req->Oid));

	req->BytesWritten = 0;
	req->BytesRead = 0;
	req->BytesNeeded = 0;

	status = wl_method_oid(wl_ctx, req->Oid, req->InformationBuffer,
		req->InputBufferLength, req->InformationBufferLength,
		&req->BytesWritten, &req->BytesRead, &req->BytesNeeded, oid_request);

	if (req->BytesNeeded)
		/* add IHV_METHOD_INFO header */
		req->BytesNeeded += CCX_METHOD_INFO_HDR_LEN;

	return status;
}

/* ccx oid handler */
NDIS_STATUS
wlc_ccx_oid(ccx_t *ccxh, void *wl_ctx, void *info_buf, uint inbuf_len,
	uint outbuf_len, uint *bytes_written, uint *bytes_read, uint *bytes_needed,
	void *oid_request)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	PCCX_NIC_SPECIFIC_EXTENSION req = (PCCX_NIC_SPECIFIC_EXTENSION)info_buf;
	int status = NDIS_STATUS_SUCCESS;
	uint i, len, info_len;
	PIHV_CCX_TLV tlv;
	bool wlif_off;
	wlc_bsscfg_t *cfg = ci->wlc->cfg;

	WL_TRACE(("wl%d: %s: event 0x%x\n", ci->pub->unit, __FUNCTION__, req->event));

	*bytes_read = 0;
	*bytes_written = 0;
	*bytes_needed = 0;

	ci->pubci.oidreq = TRUE;

	if (!BSSCFG_STA(cfg) || !cfg->BSS) {
		status = NDIS_STATUS_FAILURE;
		goto exit_ccx_oid;
	}

	/* request block should at least have event field and data header */
	if (inbuf_len < CCX_NIC_SPEC_EXT_HDR_LEN + CCX_TLV_DATA_HDR_LEN) {
		status = NDIS_STATUS_INVALID_LENGTH;
		WL_ERROR(("wl%d: %s: invalid request length\n", ci->pub->unit, __FUNCTION__));
		goto exit_ccx_oid;
	}

	WL_OID(("wl%d: %s: OID: event %s buflen %d\n",
		ci->pub->unit, __FUNCTION__, wlc_ccx_oid2str(req->event), inbuf_len));

	wlif_off = (DEVICEREMOVED(ci->wlc) || !ci->pub->up);

	*bytes_read = CCX_NIC_SPEC_EXT_HDR_LEN + CCX_TLV_DATA_HDR_LEN;
	switch (req->event) {
	case CCX_EVENT_OK_TO_ASSOCIATE:
		if (wlif_off) {
			WL_ERROR(("wl%d: %s: CCX_EVENT_OK_TO_ASSOCIATE: failed since"
				" interface is off\n", ci->pub->unit,  __FUNCTION__));
			status = NDIS_STATUS_INVALID_STATE;
			goto exit_ccx_oid;
		}

		if (!CCX_ENAB(ci->pub)) {
			status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		}

		if (cfg->assoc->state != AS_WAIT_IE) {
			/* too late...  ignore */
			WL_ASSOC(("wl%d: %s: JOIN: CCX_EVENT_OK_TO_ASSOCIATE:"
				" ignored due to association wait timeout\n",
					ci->pub->unit, __FUNCTION__));
			*bytes_read = inbuf_len;
			break;
		}

		/* should be OID_CCX_ASSOC_INFO tlv */
		if (inbuf_len > (*bytes_read + CCX_TLV_HDR_LEN + CCX_SET_INFO_HDR_LEN)) {
			PIHV_SET_INFO info = (PIHV_SET_INFO)req->tlvData.tlv[0].value;
			ASSERT(req->tlvData.tlvCount == 1);
			info_len = req->tlvData.tlv[0].length;
			status = wlc_ccx_set_oid(ci, wl_ctx, info, info_len);
			*bytes_read += CCX_TLV_HDR_LEN + CCX_SET_INFO_HDR_LEN +
				info->BytesRead;
			if (info->BytesNeeded)
				*bytes_needed = CCX_NIC_SPEC_EXT_HDR_LEN + CCX_TLV_DATA_HDR_LEN +
					CCX_TLV_HDR_LEN + CCX_SET_INFO_HDR_LEN + info->BytesNeeded;
		}

#ifdef BCMDBG
		if (ci->pubci.ccx_ihv_ies) {
			WL_ASSOC(("wl%d: %s: JOIN: IHV ok to association with CCX IEs:\n",
				ci->pub->unit, __FUNCTION__));
			if (WL_ASSOC_ON())
				wlc_print_ies(ci->wlc, ci->pubci.ccx_ihv_ies,
					ci->pubci.ccx_ihv_ies_len);
		} else {
			WL_ASSOC(("wl%d: %s: JOIN: IHV ok to association without CCX IEs\n",
				ci->pub->unit, __FUNCTION__));
		}
#endif /* BCMDBG */

		/* start join */
		if (status == NDIS_STATUS_SUCCESS)
			wlc_join_bss_prep(cfg);
		break;

	case CCX_EVENT_SEND_PACKET:
		if (wlif_off) {
			WL_ERROR(("wl%d: %s: CCX_EVENT_SEND_PACKET: failed since"
				" interface is off\n", ci->pub->unit,  __FUNCTION__));
			status = NDIS_STATUS_INVALID_STATE;
			goto exit_ccx_oid;
		}

		if (!CCX_ENAB(ci->pub)) {
			status = NDIS_STATUS_NOT_SUPPORTED;
			break;
		}

		/* must be associated */
		if (!BSSCFG_STA(cfg) || !cfg->associated) {
			status = NDIS_STATUS_REQUEST_ABORTED;
			break;
		}

		tlv = &req->tlvData.tlv[0];
		for (i = 0; i < req->tlvData.tlvCount; i++) {
			if (inbuf_len < (*bytes_read + CCX_TLV_HDR_LEN)) {
				status = NDIS_STATUS_INVALID_LENGTH;
				WL_ERROR(("wl%d: %s: CCX_EVENT_SEND_PACKET: tlv error\n",
					ci->pub->unit, __FUNCTION__));
				break;
			}
			if (tlv->type == CCX_TLV_802_11_PACKET) {
				PCCX_PACKET p = (PCCX_PACKET)tlv->value;
				len = *bytes_read + CCX_TLV_HDR_LEN + CCX_PACKET_HDR_LEN;
				if (inbuf_len < len || inbuf_len < (len + p->uSize)) {
					status = NDIS_STATUS_INVALID_LENGTH;
					WL_ERROR(("wl%d: %s: CCX_EVENT_SEND_PACKET: bad length\n",
					ci->pub->unit, __FUNCTION__));
					break;
				}
				*bytes_read = len + p->uSize;
				/* send pkt */
				status = wlc_ccx_send_d11_pkt(ci, p->data, p->uSize,
					p->ReservedIHV);
				if (status != BCME_OK) {
					/* indicate tx error status */
					ci->ihv_pkt_arg[ci->ihv_pkt_arg_idx] = p->ReservedIHV;
					wlc_ccx_ind_tx_status(ci, 0, FALSE, 0, ci->ihv_pkt_arg_idx);
					status = NDIS_STATUS_FAILURE;
					WL_ERROR(("wl%d: %s: CCX_EVENT_SEND_PACKET: sending"
						" failed\n", ci->pub->unit, __FUNCTION__));
					break;
				}
				/* next tx packet tlv */
				tlv = (PIHV_CCX_TLV)((uint8*)tlv + CCX_TLV_HDR_LEN + tlv->length);
			} else {
				status = NDIS_STATUS_NOT_SUPPORTED;
				WL_ERROR(("wl%d: %s: CCX_EVENT_SEND_PACKET: unsupported tlv type\n",
					ci->pub->unit, __FUNCTION__));
				break;
			}
		}
		break;

	case CCX_EVENT_OID: {
		uint i;
		uint BytesNeededDelta = 0;
		PIHV_CCX_TLV tlv = (PIHV_CCX_TLV)&req->tlvData.tlv[0];
		for (i = 0; i < req->tlvData.tlvCount; i++) {
			if (inbuf_len < (*bytes_read + CCX_TLV_HDR_LEN) ||
				inbuf_len < (*bytes_read + CCX_TLV_HDR_LEN + tlv->length)) {
				BytesNeededDelta += (inbuf_len < (*bytes_read + CCX_TLV_HDR_LEN)) ?
					*bytes_read + CCX_TLV_HDR_LEN - inbuf_len :
					*bytes_read + CCX_TLV_HDR_LEN + tlv->length - inbuf_len;
				status = NDIS_STATUS_INVALID_LENGTH;
				WL_ERROR(("wl%d: %s: CCX_EVENT_OID: invalid info length\n",
				ci->pub->unit, __FUNCTION__));
				break;
			}
			switch (tlv->type) {
			case CCX_TLV_SET_DATA: {
				PIHV_SET_INFO setinfo = (PIHV_SET_INFO)tlv->value;
				uint setinfo_len = tlv->length;
				if (setinfo_len < CCX_SET_INFO_HDR_LEN) {
					BytesNeededDelta += CCX_SET_INFO_HDR_LEN - setinfo_len;
					WL_ERROR(("wl%d: %s: CCX_TLV_SET_DATA: invalid info"
						" length\n", ci->pub->unit, __FUNCTION__));
					break;
				}
				status = wlc_ccx_set_oid(ci, wl_ctx, setinfo, setinfo_len);
				if (setinfo->BytesNeeded)
					BytesNeededDelta += setinfo->BytesNeeded -
						setinfo->InformationBufferLength;
				break;
			}
			case CCX_TLV_QUERY_DATA: {
				PIHV_QUERY_INFO queryinfo = (PIHV_QUERY_INFO)tlv->value;
				uint queryinfo_len = tlv->length;
				if (queryinfo_len < CCX_QUERY_INFO_HDR_LEN) {
					BytesNeededDelta += CCX_QUERY_INFO_HDR_LEN - queryinfo_len;
					WL_ERROR(("wl%d: %s: CCX_TLV_QUERY_DATA: invalid info"
						" length\n", ci->pub->unit, __FUNCTION__));
					break;
				}
				status = wlc_ccx_query_oid(ci, wl_ctx, queryinfo, queryinfo_len);
				if (queryinfo->BytesNeeded)
					BytesNeededDelta += queryinfo->BytesNeeded -
						queryinfo->InformationBufferLength;
				*bytes_written += queryinfo->BytesWritten;
				break;
			}
			case CCX_TLV_METHOD_DATA: {
				PIHV_METHOD_INFO methodinfo = (PIHV_METHOD_INFO)tlv->value;
				uint methodinfo_len = tlv->length;
				if (methodinfo_len < CCX_METHOD_INFO_HDR_LEN) {
					BytesNeededDelta +=
						CCX_METHOD_INFO_HDR_LEN - methodinfo_len;
					WL_ERROR(("wl%d: %s: CCX_TLV_METHOD_DATA: invalid info"
						" length\n", ci->pub->unit, __FUNCTION__));
					break;
				}
				status = wlc_ccx_method_oid(ci, wl_ctx,
					methodinfo, methodinfo_len, oid_request);
				if (methodinfo->BytesNeeded)
					BytesNeededDelta += methodinfo->BytesNeeded -
						methodinfo->InformationBufferLength;
				*bytes_written += methodinfo->BytesWritten;
				break;
			}
			default:
				status = NDIS_STATUS_REQUEST_ABORTED;
				WL_ERROR(("wl%d: %s: unsupported tlv type 0x%x\n",
					ci->pub->unit, __FUNCTION__, tlv->type));
				break;
			}

			*bytes_read += CCX_TLV_HDR_LEN + tlv->length;
			tlv = (PIHV_CCX_TLV)((uint8*)tlv + CCX_TLV_HDR_LEN + tlv->length);
		}
		if (BytesNeededDelta)
			*bytes_needed = inbuf_len + BytesNeededDelta;
		break;
	}

	default:
		status = NDIS_STATUS_NOT_SUPPORTED;
		WL_ERROR(("wl%d: %s: Unexpected CCX event 0x%x request\n",
			ci->pub->unit, __FUNCTION__, req->event));
	}

	ASSERT(*bytes_read <= inbuf_len);

	/* always return entire structure back to IHV Service - see sample code */
	if (*bytes_written)
		*bytes_written = inbuf_len;

exit_ccx_oid:
	ci->pubci.oidreq = FALSE;
	return status;
}

/* validate and decrypt received management frame */
int
wlc_ccx_mfp_rx(ccx_t *ccxh, wlc_d11rxhdr_t *wrxh,
	struct dot11_management_header *hdr, void *p, struct scb *scb)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	uint16 fc;
	uint8 algo;
	struct wlc_frminfo f;
	uint8 prio;
	uint offset;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	wlc_key_t *key;
	wlc_key_info_t key_info;
	struct ether_header eh;
	int err = BCME_OK;

	/* mark pkt as MFP frame */
	WLPKTTAG(p)->flags |= WLF_MFP;
	fc = ltoh16(hdr->fc);

	/* decrypt packet */
	bzero(&f, sizeof(struct wlc_frminfo));
	f.pbody = (uint8*)PKTDATA(ci->pub->osh, p);	/* start from iv */
	f.body_len = PKTLEN(ci->pub->osh, p);
	f.fc = fc;
	f.ismulti = FALSE;
	f.rx_wep = TRUE;
	f.h = (struct dot11_header *)hdr;
	f.wrxh = wrxh;
	f.rxh = &wrxh->rxhdr;
	f.prio = 0;
	f.WPA_auth = scb->WPA_auth;
	f.da = &hdr->da;
	f.sa = &hdr->sa;

	offset = (uint)((uint8*)f.pbody - (uint8*)hdr);
	PKTPUSH(ci->pub->osh, p, offset);
	f.p = p;
	f.len = PKTLEN(ci->pub->osh, p);
	f.totlen = pkttotlen(ci->pub->osh, p);
	if (wlc_keymgmt_recvdata(ci->wlc->keymgmt, &f) != BCME_OK) { /* decrypt pkt */
		err = BCME_DECERR;
		goto done;
	}

	prio = f.prio;
	algo = f.key_info.algo;

	/* for TKIP, check MHDR IE */
	if (algo == CRYPTO_ALGO_TKIP) {
		ccx_mhdr_ie_t *mhdr = (ccx_mhdr_ie_t*)((uint8*)f.pbody + f.body_len -
			(TKIP_MIC_SIZE + sizeof(ccx_mhdr_ie_t)));
		if (mhdr->id != DOT11_MNG_PROPR_ID ||
			mhdr->length != 12 || bcmp(mhdr->oui, CISCO_AIRONET_OUI, 3) ||
			mhdr->type != CCX_HMDR_IE_TYPE ||
			ltoh16(*(uint16*)mhdr->fc) != (fc & ~(FC_RETRY | FC_PM | FC_MOREDATA)) ||
			bcmp(&mhdr->bssid, &hdr->bssid, ETHER_ADDR_LEN)) {
			WLCNTINCR(ci->pubci.mgmt_cnt.tkipmgmtmhdrerr);
			err = BCME_IE_NOTFOUND;
			goto done;
		}
	}

	/* strip d11 header; f.pbody now points to octet after iv and insert ether hdr */
	memcpy(eh.ether_dhost, &hdr->da, ETHER_ADDR_LEN);
	memcpy(eh.ether_shost, &hdr->sa, ETHER_ADDR_LEN);
	eh.ether_type = 0;

	offset = (uint)((uint8*)f.pbody - (uint8*)hdr) - sizeof(eh);
	f.eh = (struct ether_header *)PKTPULL(ci->pub->osh, p, offset);
	memcpy(f.eh, &eh, sizeof(eh));
	err = wlc_key_rx_msdu(f.key, f.p, f.rxh);
	if (err != BCME_OK)
		goto done;

	/* strip ether header and MHDR IE, if any */
	PKTPULL(ci->pub->osh, p, sizeof(eh));
	if (algo == CRYPTO_ALGO_TKIP) {
		f.len -= sizeof(ccx_mhdr_ie_t);
		PKTSETLEN(ci->pub->osh, f.p, f.len);
	}

done:
	WL_WSEC(("wl%d: %s: returning status %d\n", ci->pub->unit, __FUNCTION__, err));
	return err;
}

/* validate and encrypt transmitting management frame */
int
wlc_ccx_mfp_tx(ccx_t *ccxh, void *p, struct scb *scb)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	struct dot11_management_header *hdr;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	uint16 fc, seq, saved_fc, saved_seq;
	uint offset;
	int err = BCME_OK;

	hdr = (struct dot11_management_header*)PKTDATA(ci->pub->osh, p);

	/* set fc field */
	fc = ltoh16(hdr->fc);

	/* save MAC header fc and seq */
	saved_fc = htol16(fc);
	saved_seq = hdr->seq;

	/* modify fc to construct MHDR ie for TKIP or AAD for AES/CCMP */
	fc &= ~(FC_RETRY | FC_PM | FC_MOREDATA);
	hdr->fc = htol16(fc);

	key = wlc_keymgmt_get_scb_key(ci->wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);

	/* for TKIP */
	if (key_info.algo == CRYPTO_ALGO_TKIP) {
		uint pkt_len;
		ccx_mhdr_ie_t *mhdr;
		struct ether_header *eh;
		uint8 buf[ETHER_HDR_LEN];
		/* set sequence control field to zero */
		hdr->seq = 0;
		/* add MHDR ie */
		pkt_len = PKTLEN(ci->pub->osh, p);
		mhdr = (ccx_mhdr_ie_t*)((char *)PKTDATA(ci->pub->osh, p) + pkt_len);
		mhdr->id = DOT11_MNG_PROPR_ID;
		mhdr->length = 12;
		bcopy(CISCO_AIRONET_OUI, mhdr->oui, 3);
		mhdr->type = CCX_HMDR_IE_TYPE;
		*(uint16*)mhdr->fc = hdr->fc;
		bcopy(&hdr->bssid, &mhdr->bssid, ETHER_ADDR_LEN);
		pkt_len += sizeof(ccx_mhdr_ie_t);
		PKTSETLEN(ci->pub->osh, p, pkt_len);

		offset = DOT11_MGMT_HDR_LEN + key_info.iv_len - ETHER_HDR_LEN;
		eh = (struct ether_header*)PKTPULL(ci->pub->osh, p, offset);
		/* save data before overwriting it */
		bcopy((uint8*)eh, buf, ETHER_HDR_LEN);
		bcopy(&hdr->da, &eh->ether_dhost, ETHER_ADDR_LEN);
		bcopy(&hdr->sa, &eh->ether_shost, ETHER_ADDR_LEN);
		eh->ether_type = hton16(PKTLEN(ci->pub->osh, p) - ETHER_HDR_LEN);

		/* calculate MIC */
		err = wlc_key_prep_tx_msdu(key, p, 0, CCX_MFP_TKIP_PRIO);
		if (err != BCME_OK)
			goto done;

		/* restore overwriten data */
		bcopy(buf, (uint8*)eh, ETHER_HDR_LEN);
		/* restore 802.11 mgmt header */
		hdr = (struct dot11_management_header*)PKTPUSH(ci->pub->osh, p, offset);
	} else { /* AES-CCMP */
		/* set Sequence Number in Sequence Control field to zero */
		seq = ltoh16(hdr->seq);
		seq &= FRAGNUM_MASK;
		hdr->seq = htol16(seq);
	}

	/* sw encrypt frame */
	err = wlc_key_prep_tx_mpdu(key, p, NULL /* txd */);
	if (err != BCME_OK)
		goto done;

	/* restore fc and seq in MAC header */
	hdr->fc = saved_fc;
	hdr->seq = saved_seq;

done:
	WL_WSEC(("wl%d: %s: returning status %d\n", ci->pub->unit, __FUNCTION__, err));
	return err;
}

/* reserve iv, icv and mic space for mfp */
void
wlc_ccx_mfp_tx_mod_len(ccx_t *ccxh, struct scb *scb, uint *iv_len, uint *tail_len)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_key_info_t key_info;

	(void)wlc_keymgmt_get_scb_key(ci->wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);

	/* reserve space for iv, icv, mic and MHDR if TKIP */
	*iv_len = key_info.iv_len;
	*tail_len = key_info.icv_len;
	if (key_info.algo == CRYPTO_ALGO_TKIP)
		*tail_len += sizeof(ccx_mhdr_ie_t) + TKIP_MIC_SIZE;
}

/* called when set MFP key after install hw key */
void
wlc_ccx_mfp_set_key(ccx_t *ccxh, wl_wsec_key_t *wl_key)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	struct scb *scb;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	wlc_bsscfg_t *cfg = ci->wlc->cfg;
	wlc_key_t *key;
	wlc_key_info_t key_info;

	/* no MFP support in v4 only mode */
	if (ci->pubci.ccx_v4_only)
		return;

	scb = wlc_scbfindband(ci->wlc, cfg, &wl_key->ea,
		CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
	if (!scb) {
		WL_ERROR(("wl%d: %s: wlc_scbfindband failed for BSSID %s\n",
			ci->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(&wl_key->ea, eabuf)));
		return;
	}

	/* if remove key */
	if (!wl_key->len) {
		scb->flags2 &= ~SCB2_CCX_MFP;
		WL_INFORM(("wl%d: %s: MFP key removed\n", ci->pub->unit, __FUNCTION__));
		return;
	}

	if (!(BSSCFG_STA(cfg) && cfg->BSS && cfg->associated &&
	      (ci->capability & CAP_MFP))) {
		WL_ERROR(("wl%d: %s: MFP not supported\n", ci->pub->unit, __FUNCTION__));
		return;
	}

	/* must have key */
	key = wlc_keymgmt_get_scb_key(ci->wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);

	/* key algo must be either TKIP or AES */
	if (key_info.algo != CRYPTO_ALGO_TKIP && key_info.algo != CRYPTO_ALGO_AES_CCM) {
		WL_ERROR(("wl%d: %s: unsupported key algorithm %d for MFP\n",
			ci->pub->unit, __FUNCTION__, key_info.algo));
		return;
	}

	/* set MFP flag to indicate MFP feature enabled */
	scb->flags2 |= SCB2_CCX_MFP;

	/* initialize counters after received key from 4-way handshake */
	WLCNTSET(ci->pubci.mgmt_cnt.tkipmgmticverr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.tkipmgmtnoencrypterr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.tkipmgmtlocalmicfail, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.tkipmgmtmhdrerr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.tkipmgmtreplayerr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.ccmpmgmtdecrypterr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.ccmpmgmtnoenypterr, 0);
	WLCNTSET(ci->pubci.mgmt_cnt.ccmpmgmtreplayerr, 0);

	WL_INFORM(("wl%d: %s: MFP key set\n", ci->pub->unit, __FUNCTION__));
}

/* parse (re)association response for capability information in SFA IE */
static void
wlc_ccx_feature_info(wlc_ccx_info_t *ci, uint8 *ie_body, uint ie_len)
{
	ccx_sfa_ie_t *ccx_sfa_ie;
	uint8 ccx_sfa_msgtype = CCX_SFA_IE_TYPE;

	ci->capability = 0;

	/* no SFA IE parsing in v4 only mode */
	if (ci->pubci.ccx_v4_only)
		return;

	ccx_sfa_ie = (ccx_sfa_ie_t *)bcm_find_vendor_ie(ie_body, ie_len,
		CISCO_AIRONET_OUI, &ccx_sfa_msgtype, sizeof(ccx_sfa_msgtype));

	if (ccx_sfa_ie) {
		if ((uint)((uint8*)ccx_sfa_ie - ie_body + sizeof(ccx_sfa_ie_t)) <= ie_len) {
			ci->capability = ccx_sfa_ie->capability;
			WL_INFORM(("wl%d: %s: negotiated capability 0x%x\n",
				ci->pub->unit, __FUNCTION__, ci->capability));
		} else
			WL_ERROR(("wl%d: %s: SFA IE corrupted\n", ci->pub->unit, __FUNCTION__));
	}
}

/* override association request and other params for diag mode */
void
wlc_ccx_diag_assoc_overrides(ccx_t *ccxh, struct dot11_assoc_req *assoc)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	uint16 capability = ltoh16(assoc->capability);

	/* no short preamble, PBCC, channel agility, spectrum management
	 * short slot time and privacy for diag mode
	 */
	capability &= ~(DOT11_CAP_SHORT | DOT11_CAP_PBCC | DOT11_CAP_AGILITY |
		DOT11_CAP_SPECTRUM | DOT11_CAP_SHORTSLOT | DOT11_CAP_PRIVACY);
	assoc->capability = htol16(capability);

	/* set listen interval */
	ci->wlc->bcn_li_bcn = DIAG_ASSOC_LISTEN;
	wlc_bcn_li_upd(ci->wlc);
	assoc->listen = htol16(DIAG_ASSOC_LISTEN);
}

/* CCXv5 S71 Interpretation of Status and Result Codes */
void
wlc_ccx_mgmt_status_hdlr(ccx_t *ccxh, struct ether_addr* bssid,
	int code, bool is_status, void *arg, int arg_len)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	uint8 row_number = 1;	/* no action required */
	uint wait_time;
	wlc_bss_info_t *bi;
	int i, j;
	/* status rows */
	uint8 row2_status[] = {10, 13, 15, 18, 19, 20, 21, 22, 23, 24, 25, 26, 35, 40, 44, 46, 48};
	uint8 row3_status[] = {17, 33};
	uint8 row4_status[] = {38, 39, 41, 42, 43, 45, 51};
	uint8 row5_status[] = {47};
	/* reason rows */
	uint8 row2_reason[] = {10, 11, 13, 21, 35};
	uint8 row3_reason[] = {3, 5, 8, 33, 34};
	uint8 row4_reason[] = {17, 18, 19, 20, 22, 24, 45};
	struct {
		uint8 *row;
		int size;
	} status_row[] = {
		{row2_status, sizeof(row2_status)}, {row3_status, sizeof(row3_status)},
		{row4_status, sizeof(row4_status)}, {row5_status, sizeof(row5_status)}
	}, reason_row[] = {
		{row2_reason, sizeof(row2_reason)}, {row3_reason, sizeof(row3_reason)},
		{row4_reason, sizeof(row4_reason)}
	}, *row;
	wlc_bsscfg_t *cfg = ci->wlc->cfg;

#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	/* no S71 support in v4 only mode */
	if (ci->pubci.ccx_v4_only)
		return;

	if (!BSSCFG_STA(cfg) || !cfg->BSS || !bssid)
		return;

	/* check CCX version */
	bi = cfg->target_bss;
	if (bi->ccx_version < 5)
		return;

	if (is_status) {	/* status code */
		row = status_row;
		j = ARRAYSIZE(status_row);
	} else {
		row = reason_row;
		j = ARRAYSIZE(reason_row);
	}

	for (i = 0; i < j; i++) {
		if (memchr(row[i].row, code, row[i].size)) {
			row_number = i + 2;
			break;
		}
	}

	switch (row_number) {
	case 2:
		/* require delay at least 5 seconds, or correct misconfiguration issue
		 * before attempting connect to the same AP per the spec.
		 */
	case 3:
		/* should try to roam to other AP or change configuration before
		 * attempting connect to the same AP
		 */
	case 4:
		/* correct misconfiguration issue before attempting connect to the same AP */
		wait_time = 5;
		break;
	case 5:
		/* wait the requested time before attempting the request again at the same AP
		 * or roam to another AP
		 */
		wait_time = *(uint*)arg;
		break;
	default:
		break;
	}

	if (row_number != 1) {
		wlc_ccx_status_row_t *entry;

		entry = (wlc_ccx_status_row_t*)MALLOCZ(ci->pub->osh, sizeof(wlc_ccx_status_row_t));
		if (entry == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			ci->pub->unit, __FUNCTION__, MALLOCED(ci->pub->osh)));
			return;
		}
		bcopy(bssid, &entry->bssid, ETHER_ADDR_LEN);
		entry->wait_time = wait_time;
		entry->status_row = row_number;

		entry->next = ci->blocked_ap;
		ci->blocked_ap = entry;

		WL_ASSOC(("wl%d: %s: bssid %s: status row %d added\n",
		ci->pub->unit, __FUNCTION__, bcm_ether_ntoa(bssid, eabuf),
		row_number));
	}
}

/* update blocked ap list in watchdog */
static void
wlc_ccx_update_blocked_ap_list(wlc_ccx_info_t *ci)
{
	wlc_ccx_status_row_t *p1, *p2;

	for (p1 = p2 = ci->blocked_ap; p2;) {
		/* update wait time counter */
		if (p2->wait_time)
			p2->wait_time--;
		/* if wait time counter reaches to zeor and status row is 2 or 5,
		 * unblock the AP by removing the entry from the block list
		 */
		if (!p2->wait_time && (p2->status_row == 2 || p2->status_row == 5)) {
			if (p1 == p2) {
				ci->blocked_ap = p1 = p2->next;
				MFREE(ci->pub->osh, p2, sizeof(wlc_ccx_status_row_t));
				p2 = p1;
			} else {
				p1->next = p2->next;
				MFREE(ci->pub->osh, p2, sizeof(wlc_ccx_status_row_t));
				p2 = p1->next;
			}
			continue;
		}
		if (p1 != p2)
			p1 = p1->next;
		p2 = p2->next;
	}
}

bool
wlc_ccx_in_addts_delay(ccx_t *ccxh, struct ether_addr *bssid)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_ccx_status_row_t *p;

	/* search blocked ap list for matched ssid */
	for (p = ci->blocked_ap; p; p = p->next) {
		if (!bcmp(&p->bssid, &bssid, ETHER_ADDR_LEN) && p->status_row == 5)
			return TRUE;
	}

	return FALSE;
}

bool
wlc_ccx_cckm_reassoc_key(ccx_t *ccxh, wl_wsec_key_t *key, bool mfp_key)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	wlc_bss_info_t *bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
	wlc_bsscfg_t *cfg = wlc->cfg;

	/* since in CCKM roam ihv sends down the pairwise key after CCX
	 * association indication and before reassociation completion,
	 * cache pairwise key for CCKM roam when roam to the same AP
	 * to avoid the key be deleted in post-reassociation cleanup.
	 * the key will be insert later
	 */
	if (IS_CCKM_AUTH(cfg->WPA_auth) && cfg->assoc->type != AS_ASSOCIATION &&
	    cfg->assoc->state == AS_WAIT_IE &&
	    !bcmp((char *)&bi->BSSID, (char *)&cfg->prev_BSSID, ETHER_ADDR_LEN) &&
	    !bcmp((char *)&bi->BSSID, &key->ea, ETHER_ADDR_LEN)) {
		ci->cckm_p_key = *key;
		ci->cckm_mfp_key = mfp_key;
		ci->cckm_p_key_cached = TRUE;
		return TRUE;
	}

	return FALSE;
}

#ifdef BCMDBG

static const char *wlc_ccx_oid2str(UINT oidval)
{
	static char buffer[100];

	sprintf(buffer, "<unknown> 0x%x", (uint)oidval);
	return buffer;
}

static const char *wlc_ccx_tlvcode2str(uint tlv_code)
{
	static char buffer[100];

	sprintf(buffer, "<unknown> 0x%x", (uint)tlv_code);
	return buffer;
}
#endif /* BCMDBG */
#endif /* CCX_SDK */
void
wlc_cckm_calc_reassocreq_MIC(cckm_reassoc_req_ie_t *cckmie,
	struct ether_addr *bssid, wpa_ie_fixed_t *rsnie, struct ether_addr *cur_ea,
	uint32 rn, uint8 *key_refresh_key, uint32 WPA_auth)
{
	uchar data[128], hash_buf[20];
	int data_len = 0;

	/* create the data portion (STA-ID | BSSID | RSNIE | Timestamp | RN) */
	bcopy((uint8 *)cur_ea, (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy((uint8 *)bssid, (char *)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy((uint8 *)rsnie, (char *)&data[data_len], TLV_HDR_LEN + rsnie->length);
	data_len += TLV_HDR_LEN + rsnie->length;
	bcopy((uint8 *)cckmie->timestamp, (char *)&data[data_len],
	      DOT11_MNG_TIMESTAMP_LEN);
	data_len += DOT11_MNG_TIMESTAMP_LEN;
	bcopy((uint8 *)&cckmie->rn, (char *)&data[data_len], sizeof(cckmie->rn));
	data_len += sizeof(cckmie->rn);

	/* generate the MIC */
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		printf("wlc_cckm_calc_reassocreq_MIC...\n");
		prhex("KRK looks like", key_refresh_key, 16);
	}
#endif /* BCMDBG || WLMSG_WSEC */
	if (WPA_auth == WPA2_AUTH_CCKM)
		hmac_sha1(data, data_len, key_refresh_key, CCKM_KRK_LEN, hash_buf);
	else
	hmac_md5(data, data_len, key_refresh_key, CCKM_KRK_LEN, hash_buf);
	bcopy(hash_buf, cckmie->mic, CCKM_MIC_LEN);
}

#if defined(CCX_SDK)
void*
wlc_ccx_frame_get_mgmt(ccx_t *ccxh, uint16 fc, uint8 cat,
    const struct ether_addr *da, const struct ether_addr *sa,
    const struct ether_addr *bssid, uint body_len, uint8 **pbody)
{
	wlc_ccx_info_t *ci = (wlc_ccx_info_t *)ccxh;
	wlc_info_t *wlc = (wlc_info_t *)ci->wlc;
	void *p = NULL;
	uint iv_len = 0, tail_len = 0;
	bool needs_mfp = FALSE;

	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx) &&
		BSSCFG_STA(wlc->cfg) && wlc->cfg->BSS && wlc->cfg->associated &&
		(((fc == FC_DEAUTH || fc == FC_DISASSOC) &&
		!bcmp(da, bssid, ETHER_ADDR_LEN)) ||
		fc == FC_ACTION) && !ETHER_ISMULTI(da)) {
		struct scb *scb = wlc_scbfindband(wlc, wlc->cfg, bssid,
			CHSPEC_WLCBANDUNIT(wlc->cfg->current_bss->chanspec));
		if (scb && SCB_CCX_MFP(scb)) {
			/* reserve iv, icv and mic space for mfp if enabled */
			wlc_ccx_mfp_tx_mod_len(wlc->ccx, scb, &iv_len, &tail_len);
			needs_mfp = TRUE;
			if (iv_len || tail_len)
				WL_WSEC(("wl%d: %s: allocate %d bytes for"
					" iv and %d bytes for tail\n",
					wlc->pub->unit, __FUNCTION__, iv_len, tail_len));
		}
	}

	p = wlc_frame_get_mgmt_ex(wlc, fc, da, sa, bssid, body_len, pbody,
		iv_len, tail_len);

	if (needs_mfp && (p != NULL))
		WLPKTTAG(p)->flags |= WLF_MFP;

	return (p);
}
#endif /* CCX_SDK */
#endif /* BCMCCX */
