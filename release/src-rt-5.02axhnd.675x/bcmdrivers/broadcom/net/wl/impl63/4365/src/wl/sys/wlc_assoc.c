/*
 * Association/Roam related routines
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
 * $Id: wlc_assoc.c 788228 2020-06-25 05:48:31Z $
 */

/**
 * XXX Related Twiki's:
 * [Dept3339SwBobsCorner#wlc_assoc_c] Sample function call sequences
 * [BSSTransition11vOlympic] Olympic BSS Transition feature: AP load balancing related
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <sbchipc.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <proto/wpa.h>
#include <bcmwpa.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <bcmdevs.h>
#include <d11.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_hw.h>
#include <wlc_bmac.h>
#include <wlc_scb.h>
#include <wlc_led.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif // endif
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#include <wlc_bta.h>
#endif // endif
#include <wlc_scb_ratesel.h>
#include <wl_export.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <wlc_wpa.h>
#include <wlc_sup.h>
#endif // endif
#include <wlc_pmkid.h>
#include <wlc_rm.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include <wlc_cac.h>
#include <wlc_extlog.h>
#include <wlc_ap.h>
#include <wlc_apps.h>
#include <wlc_scan.h>
#include <wlc_assoc.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_p2p.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#ifdef WLWNM
#include <wlc_wnm.h>
#endif /* WLWNM */
#ifdef PSTA
#include <wlc_psta.h>
#endif // endif
#ifdef WLAWDL
#include <wlc_awdl.h>
#endif // endif

#include <wlioctl.h>
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif // endif
#include <wlc_11h.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <wlc_prot_g.h>
#include <wlc_prot_n.h>
#include <wlc_utils.h>
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#endif // endif
#include <wlc_pcb.h>
#include <wlc_vht.h>
#include <wlc_ht.h>
#include <wlc_txbf.h>
#include <wlc_macfltr.h>

#ifdef WLPFN
#include <wl_pfn.h>
#endif // endif
#define TK_CM_BLOCKED(_wlc, _bsscfg) (\
	wlc_keymgmt_tkip_cm_enabled((_wlc)->keymgmt, (_bsscfg)))

#ifdef WL11K
#include <wlc_rrm.h>
#endif /* WL11K */
#include <wlc_btcx.h>
#ifdef WLAMPDU
#include <wlc_ampdu_cmn.h>
#endif // endif
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#ifdef WLFBT
#include <wlc_fbt.h>
#endif // endif
#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif // endif
#ifdef WLRCC
#include <wlc_okc.h>
#endif // endif

#ifdef WL_BCN_COALESCING
#include <wlc_bcn_clsg.h>
#endif /* WL_BCN_COALESCING */

#include <wlc_ht.h>
#include <wlc_obss.h>
#include <wlc_tx.h>

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif // endif

#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */

#if defined(WLBSSLOAD_REPORT)
#include <wlc_bssload.h>
#endif // endif

#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif // endif

#include <wlc_hs20.h>

#include <wlc_rx.h>

#ifdef WLTPC
#include <wlc_tpc.h>
#endif // endif

#include <wlc_dfs.h>
#include <phy_radar_api.h>
#include <event_trace.h>
#ifdef STA
/* join pref width in bits */
#define WLC_JOIN_PREF_BITS_TRANS_PREF	8 /* # of bits in weight for AP Transition Join Pref */
#define WLC_JOIN_PREF_BITS_RSSI		8 /* # of bits in weight for RSSI Join Pref */
#define WLC_JOIN_PREF_BITS_WPA		4 /* # of bits in weight for WPA Join Pref */
#define WLC_JOIN_PREF_BITS_BAND		1 /* # of bits in weight for BAND Join Pref */
#define WLC_JOIN_PREF_BITS_RSSI_DELTA	0 /* # of bits in weight for RSSI Delta Join Pref */

/* Fixed join pref start bits */
#define WLC_JOIN_PREF_START_TRANS_PREF	(32 - WLC_JOIN_PREF_BITS_TRANS_PREF)

/* join pref formats */
#define WLC_JOIN_PREF_OFF_COUNT		1 /* Tuple count field offset in WPA Join Pref TLV value */
#define WLC_JOIN_PREF_OFF_BAND		1 /* Band field offset in BAND Join Pref TLV value */
#define WLC_JOIN_PREF_OFF_DELTA_RSSI	0 /* RSSI delta value offset */

/* handy macros */
#define WLCAUTOWPA(cfg)		((cfg)->join_pref != NULL && (cfg)->join_pref->wpas > 0)
#define WLCTYPEBMP(type)	(1 << (type))
#define WLCMAXCNT(type)		(1 << (type))
#define WLCMAXVAL(bits)		((1 << (bits)) - 1)
#define WLCBITMASK(bits)	((1 << (bits)) - 1)

#if MAXWPACFGS != WLCMAXCNT(WLC_JOIN_PREF_BITS_WPA)
#error "MAXWPACFGS != (1 << WLC_JOIN_PREF_BITS_WPA)"
#endif // endif

/* roaming trigger step value */
#define WLC_ROAM_TRIGGER_STEP	10	/* roaming trigger step in dB */

#define WLC_BSSID_SCAN_NPROBES                         4
#define WLC_BSSID_SCAN_ACTIVE_TIME                     120

#define WLC_SCAN_RECREATE_TIME	50	/* default assoc recreate scan time */
#define WLC_NLO_SCAN_RECREATE_TIME	30	/* nlo enabled assoc recreate scan time */

#ifdef BCMQT_CPU
#define WECA_ASSOC_TIMEOUT	1500	/* qt is slow */
#else
#define WECA_ASSOC_TIMEOUT	300	/* authentication or association timeout in ms */
#endif // endif
#define WLC_IE_WAIT_TIMEOUT	200	/* assoc. ie waiting timeout in ms */

#define WECA_AUTH_TIMEOUT	300	/* authentication timeout in ms */
#define DELAY_10MS		10	/* delay time in ms */

#ifdef WLABT
#define ABT_MIN_TIMEOUT		5
#define ABT_HIGH_RSSI		-65
#endif // endif

#ifdef BCMDBG
#define WLC_EXT_AUTH_TIMEOUT    450     /* External supplicant takes time in debug mode */
#else
#define WLC_EXT_AUTH_TIMEOUT    350     /* external auth timeout */
#endif // endif

/* local routine declarations */
static int wlc_setssid_disassociate_client(wlc_bsscfg_t *cfg);
static void wlc_setssid_disassoc_complete(wlc_info_t *wlc, uint txstatus, void *arg);
static int wlc_assoc_bsstype(wlc_bsscfg_t *cfg);
static void wlc_assoc_scan_prep(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	const struct ether_addr *bssid, const chanspec_t* chanspec_list, int channel_num);
static void wlc_assoc_scan_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	const struct ether_addr *bssid, const chanspec_t* chanspec_list, int channel_num);
#ifdef WLSCANCACHE
static void wlc_assoc_cache_eval(wlc_info_t *wlc,
	const struct ether_addr *BSSID, const wlc_ssid_t *SSID,
	int bss_type, const chanspec_t *chanspec_list, uint chanspec_num,
	wlc_bss_list_t *bss_list, chanspec_t **target_list, uint *target_num);
static void wlc_assoc_cache_fail(wlc_bsscfg_t *cfg);
#else
#define wlc_assoc_cache_eval(wlc, BSSID, SSID, bt, cl, cn, bl, tl, tn)	((void)tl, (void)tn)
#define wlc_assoc_cache_fail(cfg)	((void)cfg)
#endif // endif
static void wlc_assoc_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg);
static void wlc_assoc_success(wlc_bsscfg_t *cfg, struct scb *scb);
#ifdef WL_ASSOC_RECREATE
static void wlc_speedy_recreate_fail(wlc_bsscfg_t *cfg);
#endif /* WL_ASSOC_RECREATE */
static void wlc_assoc_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint assoc_status, bool reassoc, uint bss_type);
static void wlc_set_ssid_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint bss_type);
static void wlc_assoc_recreate_timeout(wlc_bsscfg_t *cfg);

static void wlc_roamscan_complete(wlc_bsscfg_t *cfg);
static void wlc_roam_set_env(wlc_bsscfg_t *cfg, uint entries);
static void wlc_roam_release_flow_cntrl(wlc_bsscfg_t *cfg);

#ifdef AP
static bool wlc_join_check_ap_need_csa(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, uint state);
static bool wlc_join_ap_do_csa(wlc_info_t *wlc, chanspec_t tgt_chanspec);
#endif /* AP */
static void wlc_join_attempt(wlc_bsscfg_t *cfg);
static void wlc_join_bss_start(wlc_bsscfg_t *cfg);
static bool wlc_join_chanspec_filter(wlc_bsscfg_t *cfg, chanspec_t chanspec);
static void wlc_cook_join_targets(wlc_bsscfg_t *cfg, bool roam, int cur_rssi);
static int _wlc_join_start_ibss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static bool wlc_join_basicrate_supported(wlc_info_t *wlc, wlc_rateset_t *rs, int band);
static void wlc_join_adopt_bss(wlc_bsscfg_t *cfg);
static void wlc_join_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params);
static int wlc_join_pref_tlv_len(wlc_info_t *wlc, uint8 *pref, int len);
static int wlc_bss_list_expand(wlc_bsscfg_t *cfg, wlc_bss_list_t *from, wlc_bss_list_t *to);

static int wlc_assoc_req_add_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint req, bool top);
static int wlc_assoc_req_remove_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_assoc_req_process_next(wlc_info_t *wlc);

static void wlc_merge_bcn_prb(wlc_info_t *wlc, struct dot11_bcn_prb *p1, int p1_len,
	struct dot11_bcn_prb *p2, int p2_len, struct dot11_bcn_prb **merged, int *merged_len);
static int wlc_merged_ie_len(wlc_info_t *wlc, uint8 *tlvs1, int tlvs1_len, uint8 *tlvs2,
	int tlvs2_len);
static bool wlc_find_ie_match(bcm_tlv_t *ie, bcm_tlv_t *ies, int len);
static void wlc_merge_ies(uint8 *tlvs1, int tlvs1_len, uint8 *tlvs2, int tlvs2_len, uint8* merge);
#ifdef WLABT
static void wlc_check_adaptive_bcn_timeout(wlc_bsscfg_t *cfg);
#endif // endif

static void wlc_roam_period_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#if defined(CLIENT_CSA) && defined(WLDFS)
static bool wlc_assoc_do_dfs_reentry_cac_if_required(wlc_bsscfg_t *cfg);
#endif /* CLIENT_CSA && WLDFS */

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
static void wlc_print_roam_status(wlc_bsscfg_t *cfg, uint roam_reason, bool printcache);
#endif // endif
#endif /* STA */

#ifdef WL_SAE
static int wlc_assoc_start_extauth(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
		struct ether_addr *bssid, int auth_alg, uint state);
#endif /* WL_SAE */

typedef struct join_pref {
	uint32 score;
	uint32 rssi;
} join_pref_t;

#ifdef STA
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
static const char *join_pref_name[] = {"rsvd", "rssi", "wpa", "band", "band_rssi_delta"};
#define WLCJOINPREFN(type)	join_pref_name[type]
#endif	/* BCMDBG || WLMSG_ASSOC */

/* XXX mSTA: Phase 1 takes a simple approach - seriallize join/roam requests.
 * revisit here when simultaneous multiple join/roam requests are supported.
 */
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_INFORM)
static const char *as_type_name[] = {
	"NONE", "JOIN", "ROAM", "RECREATE"
};
#define WLCASTYPEN(type)	as_type_name[type]
#endif /* BCMDBG || WLMSG_ASSOC || WLMSG_INFORM */

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ASSOC_LT)

/* When an AS_ state is added, add a string translation to the table below */
#if AS_LAST_STATE != 35 /* don't change this without adding to the table below!!! */
#error "You need to add an assoc state name string to as_st_names for the new assoc state"
#endif // endif

const char * as_st_names[] = {
	"IDLE",
	"JOIN_INIT",
	"SCAN",
	"JOIN_START",
	"WAIT_IE",
	"WAIT_IE_TIMEOUT",
	"WAIT_TX_DRAIN",
	"WAIT_TX_DRAIN_TIMEOUT",
	"SENT_AUTH_1",
	"SENT_AUTH_2",
	"SENT_AUTH_3",
	"SENT_ASSOC",
	"REASSOC_RETRY",
	"JOIN_ADOPT",
	"WAIT_RCV_BCN",
	"SYNC_RCV_BCN",
	"WAIT_DISASSOC",
	"WAIT_PASSHASH",
	"RECREATE_WAIT_RCV_BCN",
	"ASSOC_VERIFY",
	"LOSS_ASSOC",
	"JOIN_CACHE_DELAY",
	"WAIT_FOR_AP_CSA",
	"WAIT_FOR_AP_CSA_ROAM_FAIL",
	"AS_SENT_FTREQ",
	"AS_MODE_SWITCH_START",
	"AS_MODE_SWITCH_COMPLETE",
	"AS_MODE_SWITCH_FAILED",
	"AS_DISASSOC_TIMEOUT",
	"AS_IBSS_CREATE",
	"AS_DFS_CAC_START",
	"AS_DFS_CAC_FAIL",
	"AS_DFS_ISM_INIT",
	"AS_EXT_AUTH_START",
	"AS_SENT_AUTH_UP",

};

static const char *
wlc_as_st_name(uint state)
{
	const char * result;

	if (state >= ARRAYSIZE(as_st_names))
		result = "UNKNOWN";
	else {
		result = as_st_names[state];
	}

	return result;
}

#endif /* BCMDBG || WLMSG_ASSOC || WLMSG_ASSOC_LT */

/* state(s) that can yield to other association requests */
#define AS_CAN_YIELD(bss, st)	((bss) && (st) == AS_SYNC_RCV_BCN)

static int
wlc_assoc_req_add_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint type, bool top)
{
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char ssidbuf[SSID_FMT_BUF_LEN];
#endif // endif
	int i, j;
	wlc_bsscfg_t *bc;
	wlc_assoc_t *as;
	bool as_in_progress_already;

	/* check the current state of assoc_req array */
	/* mark to see whether it's null or not */
	as_in_progress_already = AS_IN_PROGRESS(wlc);

	if (type == AS_ROAM) {
		if (AS_IN_PROGRESS(wlc))
			return BCME_BUSY;
		goto find_entry;
	}
	/* else if (type == AS_ASSOCIATION || type == AS_RECREATE) { */
	/* XXX At this point the list is either empty, or has only one ROAM request,
	 * or has one or more JOIN request(s). Remove the ROAM request if any. Append
	 * the new JOIN/RECREATE request at the first available entry.
	 */
	/* remove all other low priority requests */
	for (i = 0; i < WLC_MAXBSSCFG; i ++) {
		if ((bc = wlc->as_info->assoc_req[i]) == NULL)
			break;
		as = bc->assoc;
		/* Do not pre-empt when in state AS_XX_WAIT_RCV_BCN. In this state
		 * association is already completed, if we abort in this state then all
		 * those tasks which are performed as part of wlc_join_complete will be
		 * skipped but still maintain the association.
		 * This leaves power save and 11h/d in wrong state
		 */
		if (as->type == AS_ASSOCIATION || as->type == AS_RECREATE ||
			(as->type == AS_ROAM && as->state == AS_WAIT_RCV_BCN)) {
			continue;
		}
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		wlc_format_ssid(ssidbuf, bc->SSID, bc->SSID_len);
#endif // endif
		WL_ASSOC(("wl%d.%d: remove %s request in state %s for SSID '%s "
		          "from assoc_req list slot %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), WLCASTYPEN(type),
		          wlc_as_st_name(as->state), ssidbuf, i));
		wlc->as_info->assoc_req[i] = NULL;
	}
	/* } */

find_entry:
	/* find the first empty entry or entry with bsscfg in state AS_CAN_YIELD() */
	/* XXX AS_CAN_YIELD() can be removed since we are removing the entry in
	 * wlc_assoc_change_state() once as->state is in one of AS_CAN_YIELD() state
	 */
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
#endif // endif

	for (i = 0; i < WLC_MAXBSSCFG; i ++) {
		if ((bc = wlc->as_info->assoc_req[i]) == NULL)
			goto ins_entry;
		if (bc == cfg) {
			WL_ASSOC(("wl%d.%d: %s request in state %s for SSID '%s' exists "
			          "in assoc_req list at slot %d\n", wlc->pub->unit,
			          WLC_BSSCFG_IDX(cfg), WLCASTYPEN(type),
			          wlc_as_st_name(cfg->assoc->state), ssidbuf, i));
			return i;
		}
		else if (AS_CAN_YIELD(bc->BSS, bc->assoc->state) &&
			!AS_CAN_YIELD(cfg->BSS, cfg->assoc->state))
			goto ins_entry;
	}

	return BCME_NORESOURCE;

ins_entry:
	/* insert the bsscfg in the list at slot i */
	WL_ASSOC(("wl%d.%d: insert %s request in state %s for SSID '%s' "
	          "in assoc_req list at slot %d\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	          WLCASTYPEN(type), wlc_as_st_name(cfg->assoc->state), ssidbuf, i));

	ASSERT(i < WLC_MAXBSSCFG);

	j = i;
	bc = cfg;
	do {
		wlc_bsscfg_t *temp = wlc->as_info->assoc_req[i];
		wlc->as_info->assoc_req[i] = bc;
		bc = temp;
		if (bc == NULL || ++i >= WLC_MAXBSSCFG)
			break;
		as = bc->assoc;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		wlc_format_ssid(ssidbuf, bc->SSID, bc->SSID_len);
#endif // endif
		WL_ASSOC(("wl%d.%d: move %s request in state %s for SSID '%s' "
		          "in assoc_req list to slot %d\n", wlc->pub->unit,
		          WLC_BSSCFG_IDX(cfg), WLCASTYPEN(as->type),
		          wlc_as_st_name(as->state), ssidbuf, i));
	}
	while (TRUE);
	ASSERT(i < WLC_MAXBSSCFG);

	/* as_in_progress now changed, update ps_ctrl */
	if (as_in_progress_already != AS_IN_PROGRESS(wlc)) {
		wlc_set_wake_ctrl(wlc);
	}

	return j;
} /* wlc_assoc_req_add_entry */

static void
wlc_assoc_req_process_next(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char ssidbuf[SSID_FMT_BUF_LEN];
#endif // endif
	wlc_assoc_t *as;

	if ((cfg = wlc->as_info->assoc_req[0]) == NULL) {
		WL_ASSOC(("wl%d: all assoc requests in assoc_req list have been processed\n",
		          wlc->pub->unit));
		wlc_set_wake_ctrl(wlc);
		return;
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
#endif // endif

	as = cfg->assoc;
	ASSERT(as != NULL);

	WL_ASSOC(("wl%d.%d: process %s request in assoc_req list for SSID '%s'\n",
	          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), WLCASTYPEN(as->type), ssidbuf));

	switch (as->type) {
		case AS_ASSOCIATION:
			wlc_join_start(cfg, wlc_bsscfg_scan_params(cfg),
				wlc_bsscfg_assoc_params(cfg));
			break;

		case AS_RECREATE:
			wlc_join_recreate(wlc, cfg);
			break;

		default:
#ifdef WLAWDL
			if (BSSCFG_AWDL(wlc, cfg)) {
				wlc_join_start_ibss(wlc, cfg);
				break;
			}
#endif // endif
			ASSERT(0);
			break;
	}
}

int
wlc_remove_assoc_req(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char ssidbuf[SSID_FMT_BUF_LEN];
#endif // endif
	int i;
	int err = BCME_ERROR;

	for (i = 0; i < WLC_MAXBSSCFG; i ++) {
		if (wlc->as_info->assoc_req[i] != cfg)
			continue;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
#endif // endif
		WL_ASSOC(("wl%d.%d: remove %s request in state %s in assoc_req list "
		          "for SSID %s at slot %d\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		          WLCASTYPEN(cfg->assoc->type), wlc_as_st_name(cfg->assoc->state),
		          ssidbuf, i));
		/* move assoc requests up the list by 1 and stop at the first empty entry */
		for (; i < WLC_MAXBSSCFG - 1; i ++) {
			if ((wlc->as_info->assoc_req[i] = wlc->as_info->assoc_req[i + 1]) == NULL) {
				i = i + 1;
				break;
			}
		}
		wlc->as_info->assoc_req[i] = NULL;
		err = BCME_OK;
		break;
	}
	return err;
}

static int
wlc_assoc_req_remove_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int err = BCME_ERROR;
	bool as_in_progress_already;

	/* check the current state of assoc_req array */
	/* mark to see whether it's null or not */
	as_in_progress_already = AS_IN_PROGRESS(wlc);

	err = wlc_remove_assoc_req(wlc, cfg);

	/* if we cleared the wlc->as_info->assoc_req[] list, update ps_ctrl */
	if (as_in_progress_already != AS_IN_PROGRESS(wlc)) {
		wlc_set_wake_ctrl(wlc);
	}
	return err;
}

/** start a join process by broadcast scanning all channels */
void
wlc_set_ssid(wlc_info_t *wlc, uchar SSID[], int len)
{
	wlc_join(wlc, wlc->cfg, SSID, len, NULL, NULL, 0);
}

/**
 * prepare and start a join process including:
 * - abort any ongoing association or scan process if necessary
 * - enable the bsscfg (set the flags, ..., not much for STA)
 * - start the disassoc process if already associated to a different SSID, otherwise
 * - start the scan process (broadcast on all channels, or direct on specific channels)
 * - start the association process (assoc with a BSS, start an IBSS, or coalesce with an IBSS)
 * - mark the bsscfg to be up if the association succeeds otherwise try the next BSS
 *
 * bsscfg stores the desired SSID and/or bssid and/or chanspec list for later use in
 * a different execution context for example timer callback.
 */
int
wlc_join(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len)
{
	wlc_assoc_t *as;
	int ret;

	ASSERT(bsscfg != NULL);
	ASSERT(BSSCFG_STA(bsscfg));

	if (bsscfg != wlc->cfg &&
#ifdef WLBTAMP
	    !BSS_BTA_ENAB(wlc, bsscfg) &&
#endif // endif
	    wlc->pub->corerev < 15) {
		WL_ERROR(("wl%d.%d: JOIN on non-primary bsscfg is not supported for d11 revid %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), wlc->pub->corerev));
		return BCME_UNSUPPORTED;
	}

#ifdef WLOFFLD
	if (wlc_ol_bcn_is_enable(wlc->ol) || wlc_ol_arp_nd_enabled(wlc->ol))
		wlc_ol_disable(wlc->ol, wlc->cfg);
#endif // endif
	/* add the join request in the assoc_req list
	 * if someone is already in the process of association
	 */
	if ((ret = wlc_mac_request_entry(wlc, bsscfg, WLC_ACTION_ASSOC)) > 0) {
		WL_ASSOC(("wl%d.%d: JOIN request queued at slot %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), ret));
		return BCME_OK;
	}
	else if (ret < 0) {
		WL_ERROR(("wl%d.%d: JOIN request failed, err = %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), ret));
		return ret;
	}

	as = bsscfg->assoc;
	ASSERT(as != NULL);

	as->ess_retries = 0;
	as->type = AS_ASSOCIATION;

	/* save SSID and assoc params for later use in a different context and retry */
	wlc_bsscfg_SSID_set(bsscfg, SSID, len);
	wlc_bsscfg_scan_params_set(wlc, bsscfg, scan_params);
	wlc_bsscfg_assoc_params_set(wlc, bsscfg, assoc_params, assoc_params_len);

#ifdef WME
	wlc_wme_initparams_sta(wlc, &bsscfg->wme->wme_param_ie);
#endif // endif

	wlc_join_start(bsscfg, wlc_bsscfg_scan_params(bsscfg), wlc_bsscfg_assoc_params(bsscfg));
	return BCME_OK;
} /* wlc_join */

static void
wlc_assoc_timer_del(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_assoc_t *as = cfg->assoc;

	wl_del_timer(wlc->wl, as->timer);
	as->rt = FALSE;
}

/**
 * prepare for join by using the default parameters in wlc->default_bss
 * except SSID which comes from the cfg->SSID
 */
void
wlc_join_start_prep(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_bss_info_t *target_bss = cfg->target_bss;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char *ssidbuf;
	const char *ssidstr;

	if (cfg->SSID_len == 0)
		WL_ASSOC(("wl%d: SCAN: wlc_join_start, Setting SSID to NULL...\n", WLCWLUNIT(wlc)));
	else
	{
		ssidbuf = (char *) MALLOC(wlc->osh, SSID_FMT_BUF_LEN);
		if (ssidbuf) {
			wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
			ssidstr = ssidbuf;
		} else
			ssidstr = "???";
		WL_ASSOC(("wl%d: SCAN: wlc_join_start, Setting SSID to \"%s\"...\n", WLCWLUNIT(wlc),
		          ssidstr));
		if (ssidbuf != NULL)
			 MFREE(wlc->osh, (void *)ssidbuf, SSID_FMT_BUF_LEN);
	}
#endif /* #if defined(BCMDBG) || defined(WLMSG_ASSOC) */

#ifdef WLAWDL
	/* AWDL bsscfg has already pre-programmed the target_bss BSS params, no need to copy
	* For non-AWDL bsscfg, adopt the default BSS params as the target's BSS params
	*/
	if (!(BSSCFG_AWDL(wlc, cfg)))
#endif // endif
		/* adopt the default BSS params as the target's BSS params */
		bcopy(wlc->default_bss, target_bss, sizeof(wlc_bss_info_t));

	/* update the target_bss with the ssid */
	target_bss->SSID_len = cfg->SSID_len;
	if (cfg->SSID_len > 0)
		bcopy(cfg->SSID, target_bss->SSID, cfg->SSID_len);

	/* this is STA only, no aps_associated issues */
	wlc_bsscfg_enable(wlc, cfg);
	if (cfg->assoc->state != AS_WAIT_DISASSOC) {
		wlc_assoc_change_state(cfg, AS_JOIN_INIT);
		wlc_bss_assoc_state_notif(wlc, cfg, cfg->assoc->type, cfg->assoc->state);

	}

} /* wlc_join_start_prep */

void wlc_try_join_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params)
{
	/* Handle only APSTA case */
	if (!BSSCFG_IS_PRIMARY(cfg) || !BSSCFG_STA(cfg) || !APSTA_ENAB(cfg->wlc->pub)) {
		return;
	}

	if (cfg->SSID_len == 0) {
		/* boot up time, let userspace app to come up with explicit WLC_SET_SSID
		 * or IOV_JOIN request
		 */
		WL_INFORM(("wl%d: Invalid SSID len[%d] to join, return \n", cfg->wlc->pub->unit,
				cfg->SSID_len));
		return;
	}
	if (cfg->roam && cfg->roam->off) {
		/* Run time case, user maintains client's association state machine */
		WL_INFORM(("wl%d: roaming disabled in firmware, skip try join start\n",
			cfg->wlc->pub->unit));
		return;
	}

	WL_INFORM(("wl%d: try join for ssid[%s] with cfg index[%d] \n", cfg->wlc->pub->unit,
		cfg->SSID, WLC_BSSCFG_IDX(cfg)));
	wlc_join_start(cfg, scan_params, assoc_params);
}
/** start a join process. SSID is saved in bsscfg. */
static void
wlc_join_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	as->ess_retries ++;

	ASSERT(as->state == AS_IDLE || as->state == AS_WAIT_DISASSOC);

	wlc_join_start_prep(wlc, cfg);

	if (as->state == AS_WAIT_DISASSOC) {
		/* we are interrupting an earlier wlc_set_ssid call and wlc_assoc_scan_start is
		 * scheduled to run as the wlc_disassociate_client callback. The target_bss has
		 * been updated above so just allow the wlc_assoc_scan_start callback to pick up
		 * the set_ssid work with the new target.
		 */
	} else {
		wlc_bss_info_t *current_bss = cfg->current_bss;
		wlc_assoc_init(cfg, AS_ASSOCIATION);
		if (cfg->associated &&
		    (cfg->SSID_len != current_bss->SSID_len ||
		     bcmp(cfg->SSID, (char*)current_bss->SSID, cfg->SSID_len))) {

			WL_ASSOC(("wl%d: SCAN: wlc_join_start, Disassociating from %s first\n",
			          WLCWLUNIT(wlc), bcm_ether_ntoa(&cfg->prev_BSSID, eabuf)));
			wlc_assoc_change_state(cfg, AS_WAIT_DISASSOC);
			wlc_setssid_disassociate_client(cfg);
		}
		else {
			const struct ether_addr *bssid = NULL;
			const chanspec_t *chanspec_list = NULL;
			int chanspec_num = 0;
			/* make sure the association timer is not pending */
			wlc_assoc_timer_del(wlc, cfg);
			/* use assoc params if any to limit the scan hence to speed up
			 * the join process.
			 */
			if (assoc_params != NULL) {
				int bccnt;
				bssid = &assoc_params->bssid;
				chanspec_list = assoc_params->chanspec_list;
				chanspec_num = assoc_params->chanspec_num;
				bccnt = assoc_params->bssid_cnt;

				WL_ASSOC(("wl%d: SCAN: wlc_join_start, chanspec_num %d "
					"bssid_cnt %d\n", WLCWLUNIT(wlc), chanspec_num, bccnt));

				/* if BSSID/channel pairs specified, adjust accordingly */
				if (bccnt) {
					/* channel number field is index for channel and bssid */
					bssid = (const struct ether_addr*)&chanspec_list[bccnt];
					chanspec_list += chanspec_num;
					bssid += chanspec_num;

					/* check if we reached the end */
					if (chanspec_num >= bccnt) {
						WL_ASSOC(("wl: wlc_join_start: pairs done\n"));
							wlc_bsscfg_assoc_params_reset(wlc, cfg);
						return;
					}

					WL_ASSOC(("wl%d: JOIN: wlc_join_start: pair %d of %d,"
						" BSS %s, chanspec %s\n", WLCWLUNIT(wlc),
						chanspec_num, bccnt,
						bcm_ether_ntoa(bssid, eabuf),
						wf_chspec_ntoa_ex(*chanspec_list, chanbuf)));

					/* force count to one (only this channel) */
					chanspec_num = 1;
				}
			}
			/* continue the join process by starting the association scan */
			wlc_assoc_scan_prep(cfg, scan_params, bssid, chanspec_list, chanspec_num);
		}
	}
} /* wlc_join_start */

/** disassoc from the associated BSS first and then start a join process */
static void
wlc_setssid_disassoc_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	const struct ether_addr *bssid = NULL;
	const chanspec_t *chanspec_list = NULL;
	int chanspec_num = 0;
	wl_join_scan_params_t *scan_params;
	wl_join_assoc_params_t *assoc_params;
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(wlc, (uint16)(uintptr)arg);
	wlc_assoc_t *as;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	/* in case bsscfg is freed before this callback is invoked */
	if (cfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
		          wlc->pub->unit, __FUNCTION__, arg));
		return;
	}

	as = cfg->assoc;

	/* Check for aborted scans */
	if (as->type != AS_ASSOCIATION) {
		WL_ASSOC(("wl%d: wlc_setssid_disassoc_complete, as->type "
		          "was changed to %d\n", WLCWLUNIT(wlc), as->type));
		return;
	}

	if (as->state != AS_WAIT_DISASSOC) {
		WL_ASSOC(("wl%d: wlc_setssid_disassoc_complete, as->state "
		          "was changed to %d\n", WLCWLUNIT(wlc), as->state));
		return;
	}

	WL_ASSOC(("wl%d: SCAN: disassociation complete, call wlc_join_start\n", WLCWLUNIT(wlc)));

	/* use assoc params if any to limit the scan hence to speed up
	 * the join process.
	 */
	scan_params = wlc_bsscfg_scan_params(cfg);
	if ((assoc_params = wlc_bsscfg_assoc_params(cfg)) != NULL) {
		int bccnt;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
		bssid = &assoc_params->bssid;
		chanspec_list = assoc_params->chanspec_list;
		chanspec_num = assoc_params->chanspec_num;
		bccnt = assoc_params->bssid_cnt;

		WL_ASSOC(("wl%d: SCAN: disassoc_complete, chanspec_num %d bssid_cnt %d\n",
		WLCWLUNIT(wlc), chanspec_num, bccnt));

		/* if BSSID/channel pairs specified, adjust accordingly */
		if (bccnt) {
			/* channel number field is index for channel and bssid */
			bssid = (const struct ether_addr*)&chanspec_list[bccnt];
			chanspec_list += chanspec_num;
			bssid += chanspec_num;

			WL_ASSOC(("wl%d: JOIN: disassoc_complete: pair %d of %d,"
				" BSS %s, chanspec %s\n", WLCWLUNIT(wlc),
				chanspec_num, bccnt,
				bcm_ether_ntoa(bssid, eabuf),
				wf_chspec_ntoa_ex(*chanspec_list, chanbuf)));

			/* force count to one (only this channel) */
			chanspec_num = 1;
		}
	}
	/* continue the join process by starting the association scan */
	wlc_assoc_scan_prep(cfg, scan_params, bssid, chanspec_list, chanspec_num);
} /* wlc_setssid_disassoc_complete */

/**
 * Roaming related, STA chooses the best prospect AP to associate with.
 * If bss-transition scoring is not enabled, rssi is used for scoring. Else, bss-transition score is
 * used along with score_delta and rssi is used with roam_delta.
 * input: bsscfg, bss_info, -ve rssi.
 * output: prssi (positive rssi) and score (return value)
 */
static uint32
wlc_bss_pref_score_rssi(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi, int16 rssi, uint32 *prssi)
{
#ifdef WLWNM
	uint32 score;
#endif /* WLWNM */

	/* convert negative value to positive */
	*prssi = WLCMAXVAL(WLC_JOIN_PREF_BITS_RSSI) + rssi;

#ifdef WLWNM
	if (WLWNM_ENAB(cfg->wlc->pub)) {
		if ((wlc_wnm_bss_pref_score_rssi(cfg->wlc, bi,
			MAX(WLC_RSSI_MINVAL_INT8, rssi), &score) == BCME_OK))
			return score;
	}
#endif /* WLWNM */
	return *prssi;
}

/** Roaming related, STA chooses the best prospect AP to associate with */
uint32
wlc_bss_pref_score(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi, bool band_rssi_boost, uint32 *prssi)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	int j;
	int16 rssi;
	uint32 weight, value;
	uint chband;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	(void)wlc;

	/* clamp RSSI to the range 0 > rssi > WLC_RSSI_MINVAL */
	rssi = MIN(0, bi->RSSI);
	rssi = MAX(rssi, WLC_RSSI_MINVAL);

	chband = CHSPEC2WLC_BAND(bi->chanspec);

	/* Give target a better RSSI based on delta preference as long as it is already
	 * has a strong enough signal.
	 */
	if (cfg->join_pref_rssi_delta.band == chband && band_rssi_boost &&
	    rssi >= cfg->roam->roam_rssi_boost_thresh) {

		WL_ASSOC(("wl%d: Boost RSSI for AP on band %d by %d db from %d db to %d db\n",
		          wlc->pub->unit,
		          cfg->join_pref_rssi_delta.band,
		          cfg->join_pref_rssi_delta.rssi,
		          rssi, MIN(0, rssi+cfg->join_pref_rssi_delta.rssi)));

		rssi += cfg->join_pref_rssi_delta.rssi;

		/* clamp RSSI again to the range 0 > rssi > WLC_RSSI_MINVAL */
		rssi = MIN(0, rssi);

		WL_SRSCAN(("  pref: boost rssi: band=%d delta=%d rssi=%d",
			cfg->join_pref_rssi_delta.band,
			cfg->join_pref_rssi_delta.rssi, rssi));
		WL_ASSOC(("  pref: boost rssi: band=%d delta=%d rssi=%d\n",
			cfg->join_pref_rssi_delta.band,
			cfg->join_pref_rssi_delta.rssi, rssi));
	}

	for (j = 0, weight = 0; j < (int)join_pref->fields; j ++) {
		switch (join_pref->field[j].type) {
		case WL_JOIN_PREF_RSSI:
			value = wlc_bss_pref_score_rssi(cfg, bi, rssi, prssi);
			break;
		case WL_JOIN_PREF_WPA:
			/* use index as preference weight */
			value = bi->wpacfg;
			break;
		case WL_JOIN_PREF_BAND:
			/* use 1 for preferred band */
			if (join_pref->band == WLC_BAND_AUTO) {
				value = 0;
				break;
			}
			value = (chband == join_pref->band) ? 1 : 0;
			break;

		default:
			/* quiet compiler, should not come to here! */
			value = 0;
			break;
		}
		value &= WLCBITMASK(join_pref->field[j].bits);
		WL_ASSOC(("wl%d: wlc_bss_pref_score: field %s entry %d value 0x%x offset %d\n",
			WLCWLUNIT(wlc), WLCJOINPREFN(join_pref->field[j].type),
			j, value, join_pref->field[j].start));
		weight += value << join_pref->field[j].start;

		WL_SRSCAN(("  pref: apply: type=%d start=%d bits=%d",
			join_pref->field[j].type, join_pref->field[j].start,
			join_pref->field[j].bits));
		WL_SRSCAN(("  pref: apply: delta=%d score=%d",
			value << join_pref->field[j].start, weight));
		WL_ASSOC(("  pref: apply: type=%d start=%d bits=%d\n",
			join_pref->field[j].type, join_pref->field[j].start,
			join_pref->field[j].bits));
		WL_ASSOC(("  pref: apply: delta=%d score=%d\n",
			value << join_pref->field[j].start, weight));
	}

	/* pref fields may not be set; use rssi only to get the positive number */
	if (join_pref->fields == 0) {
		weight = wlc_bss_pref_score_rssi(cfg, bi, rssi, prssi);
		WL_SRSCAN(("  pref: rssi only: delta=%d score=%d",
			weight, weight));
		WL_ASSOC(("  pref: rssi only: delta=%d score=%d\n",
			weight, weight));
	}

	WL_ASSOC(("wl%d: %s: RSSI is %d in BSS %s with preference score 0x%x (qbss_load_aac 0x%x "
	          "and qbss_load_chan_free 0x%x)\n", WLCWLUNIT(wlc), __FUNCTION__, bi->RSSI,
	          bcm_ether_ntoa(&bi->BSSID, eabuf), weight, bi->qbss_load_aac,
	          bi->qbss_load_chan_free));

	return weight;
} /* wlc_bss_pref_score */

/** Roaming related */
static void
wlc_zero_pref(join_pref_t *join_pref)
{
	join_pref->score = 0;
	join_pref->rssi = 0;
}

/** Rates the candidate APs available to a STA, to subsequently make a join/roam decision */
static void
wlc_populate_join_pref_score(wlc_bsscfg_t *cfg, bool for_roam, join_pref_t *join_pref)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	wlc_bss_info_t **bip = wlc->as_info->join_targets->ptrs;
	int i, j;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */

	for (i = 0; i < (int)wlc->as_info->join_targets->count; i++) {
		if ((roam->reason == WLC_E_REASON_BSSTRANS_REQ) &&
			(WLC_IS_CURRENT_BSSID(cfg, &bip[i]->BSSID)) &&
#ifdef WLWNM
			(!WLWNM_ENAB(wlc->pub) || wlc_wnm_bsstrans_zero_assoc_bss_score(wlc)) &&
#endif /* WLWNM */
			TRUE) {
				wlc_zero_pref(&join_pref[i]);
		}
		else {
			WL_SRSCAN(("  targ: bssid=%02x:%02x ch=%d rssi=%d",
				bip[i]->BSSID.octet[4], bip[i]->BSSID.octet[5],
				wf_chspec_ctlchan(bip[i]->chanspec), bip[i]->RSSI));
			WL_ASSOC(("  targ: bssid=%02x:%02x ch=%d rssi=%d\n",
				bip[i]->BSSID.octet[4], bip[i]->BSSID.octet[5],
				wf_chspec_ctlchan(bip[i]->chanspec), bip[i]->RSSI));
			WL_ROAM(("ROAM: %02d %02x:%02x:%02x ch=%d rssi=%d\n",
				i, bip[i]->BSSID.octet[0], bip[i]->BSSID.octet[4],
				bip[i]->BSSID.octet[5],	wf_chspec_ctlchan(bip[i]->chanspec),
				bip[i]->RSSI));

			join_pref[i].score =
				wlc_bss_pref_score(cfg, bip[i], TRUE, &join_pref[i].rssi);
		}

		/* Iterate through the roam cache and check that the target is valid */
		if (!for_roam || !roam->cache_valid || (roam->cache_numentries == 0))
			continue;

		for (j = 0; j < (int) roam->cache_numentries; j++) {
			struct ether_addr* bssid = &bip[i]->BSSID;
			if (!bcmp(bssid, &roam->cached_ap[j].BSSID, ETHER_ADDR_LEN) &&
			    (roam->cached_ap[j].chanspec ==
			     wf_chspec_ctlchspec(bip[i]->chanspec))) {
				if (roam->cached_ap[j].time_left_to_next_assoc > 0) {
					wlc_zero_pref(&join_pref[i]);
					WL_SRSCAN(("  excluded: bssid=%02x:%02x "
						"next_assoc=%ds",
						bssid->octet[4], bssid->octet[5],
						roam->cached_ap[j].time_left_to_next_assoc));
					WL_ASSOC(("  excluded: AP with BSSID %s marked "
					          "as an invalid roam target\n",
					          bcm_ether_ntoa(bssid, eabuf)));
				}
			}
		}
	}
} /* wlc_populate_join_pref_score */

/** Rates the candidate APs available to a STA, to subsequently make a join/roam decision */
static void
wlc_sort_on_join_pref_score(wlc_bsscfg_t *cfg, join_pref_t *join_pref)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t **bip = wlc->as_info->join_targets->ptrs;
	wlc_bss_info_t *tmp_bi;
	int j, k;
	join_pref_t tmp_join_pref;
	bool done;

	/* no need to sort if we only see one AP */
	if (wlc->as_info->join_targets->count > 1) {
		/* sort join_targets by join preference score in increasing order */
		for (k = (int)wlc->as_info->join_targets->count; --k >= 0;) {
			done = TRUE;
			for (j = 0; j < k; j++) {
				if (join_pref[j].score > join_pref[j+1].score) {
					/* swap join_pref */
					tmp_join_pref = join_pref[j];
					join_pref[j] = join_pref[j+1];
					join_pref[j+1] = tmp_join_pref;
					/* swap bip */
					tmp_bi = bip[j];
					bip[j] = bip[j+1];
					bip[j+1] = tmp_bi;
					done = FALSE;
				}
			}
			if (done)
				break;
		}
	}
}

/** Rates the candidate APs available to a STA, to subsequently make a join/roam decision */
static void
wlc_cook_join_targets(wlc_bsscfg_t *cfg, bool for_roam, int cur_rssi)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	int i, j;
	wlc_bss_info_t **bip, *tmp_bi;
	uint roam_delta = 0;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	join_pref_t *join_pref;
	uint32 join_pref_score_size = wlc->as_info->join_targets->count * sizeof(*join_pref);
	uint32 cur_pref_score = 0, cur_pref_rssi, score_delta = 0;

	WL_SRSCAN(("wl%d: RSSI is %d; %d roaming target[s]; Join preference fields "
		"are 0x%x", WLCWLUNIT(wlc), cur_rssi, wlc->as_info->join_targets->count,
		cfg->join_pref->fields));
	WL_ASSOC(("wl%d: RSSI is %d; %d roaming target[s]; Join preference fields "
		"are 0x%x\n", WLCWLUNIT(wlc), cur_rssi, wlc->as_info->join_targets->count,
		cfg->join_pref->fields));
	WL_ROAM(("ROAM: RSSI %d, %d roaming targets\n", cur_rssi,
		wlc->as_info->join_targets->count));

	join_pref = (join_pref_t *)MALLOCZ(wlc->osh, join_pref_score_size);
	if (!join_pref) {
		WL_ERROR(("wl%d: %s: MALLOC failure\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return;
	}

	bip = wlc->as_info->join_targets->ptrs;

#ifdef WLWNM
	if (WLWNM_ENAB(cfg->wlc->pub)) {
		wlc_wnm_process_join_trgts_bsstrans(wlc, bip, wlc->as_info->join_targets->count);
	}
#endif /* WLWNM */

	wlc_populate_join_pref_score(cfg, for_roam, join_pref);

	wlc_sort_on_join_pref_score(cfg, join_pref);

	if (for_roam &&
#ifdef OPPORTUNISTIC_ROAM
	    (roam->reason != WLC_E_REASON_BETTER_AP) &&
#endif /* OPPORTUNISTIC_ROAM */
	    TRUE) {
		wlc_bss_info_t *current_bss = cfg->current_bss;
		/*
		 * - We're here because our current AP's JSSI fell below some threshold
		 *   or we haven't heard from him for a while.
		 */

		if (wlc_bss_connected(cfg)) {
			cur_pref_score =
				wlc_bss_pref_score(cfg, current_bss, FALSE, &cur_pref_rssi);
		}

		/*
		 * Use input cur_rssi if we didn't get a probe response because we were
		 * unlucky or we're out of contact; otherwise, use the RSSI of the probe
		 * response. The channel of the probe response/beacon is checked in case
		 * the AP switched channels. In that case we do not want to update cur_rssi
		 * and instead consider the AP as a roam target.
		 */
		if (wlc_bss_connected(cfg) && roam->reason == WLC_E_REASON_LOW_RSSI) {
			WL_SRSCAN(("  curr: bssid=%02x:%02x ch=%d rssi=%d",
				current_bss->BSSID.octet[4], current_bss->BSSID.octet[5],
				wf_chspec_ctlchan(current_bss->chanspec), cur_rssi));
			WL_ASSOC(("  curr: bssid=%02x:%02x ch=%d rssi=%d\n",
				current_bss->BSSID.octet[4], current_bss->BSSID.octet[5],
				wf_chspec_ctlchan(current_bss->chanspec), cur_rssi));
			roam_delta = wlc->band->roam_delta;
#ifdef BCMCCX
		} else if (wlc_bss_connected(cfg) &&
		           CCX_ENAB(wlc->pub) &&
		           wlc->ccx->ccx_network &&
		           (current_bss->ccx_version >= 4) &&
		           (wlc->ccx->orig_reason == WLC_E_REASON_LOW_RSSI) &&
		           (roam->reason == WLC_E_REASON_FAST_ROAM_FAILED)) {
			roam_delta = wlc->band->roam_delta;
#endif /* BCMCCX */
		} else {
			cur_pref_score = cur_pref_rssi = 0;
			roam_delta = 0;
		}

		/* search join target in decreasing order to pick the highest score */
		for (i = (int)wlc->as_info->join_targets->count - 1; i >= 0; i--) {
			if (WLC_IS_CURRENT_BSSID(cfg, &bip[i]->BSSID) &&
			    (wf_chspec_ctlchan(bip[i]->chanspec) ==
			     wf_chspec_ctlchan(current_bss->chanspec))) {
				cur_pref_score = join_pref[i].score;
				cur_pref_rssi = join_pref[i].rssi;
				cur_rssi = bip[i]->RSSI;
				WL_SRSCAN(("  curr(scan): bssid=%02x:%02x ch=%d rssi=%d",
					current_bss->BSSID.octet[4], current_bss->BSSID.octet[5],
					wf_chspec_ctlchan(current_bss->chanspec), cur_rssi));
				WL_ASSOC(("  curr(scan): bssid=%02x:%02x ch=%d rssi=%d\n",
					current_bss->BSSID.octet[4], current_bss->BSSID.octet[5],
					wf_chspec_ctlchan(current_bss->chanspec), cur_rssi));
				break;
			}
		}

#ifdef BCMCCX
		/* disable roam delta-based prune for certain reasons of roam */
		if ((roam->reason == WLC_E_REASON_TSPEC_REJECTED) ||
			(roam->reason == WLC_E_REASON_DIRECTED_ROAM) ||
			(roam->reason == WLC_E_REASON_BSSTRANS_REQ)) {
#ifdef WLWNM
			if (WLWNM_ENAB(wlc->pub)) {
				if (wlc_wnm_bsstrans_zero_assoc_bss_score(wlc)) {
					cur_pref_score = cur_pref_rssi = 0;
				}
			} else
#endif /* WLWNM */
				cur_pref_score = cur_pref_rssi = 0;
			WL_ASSOC(("wl%d: ROAM: %s: cur_pref_score forced to 0x%x for roam_type "
			      "0x%x\n", WLCWLUNIT(wlc), __FUNCTION__, cur_pref_score,
			      roam->reason));
		}
#endif /* BCMCCX */

		WL_ASSOC(("wl%d: ROAM: wlc_cook_join_targets, roam_metric[%s] = 0x%x\n",
			WLCWLUNIT(wlc), bcm_ether_ntoa(&(cfg->BSSID), eabuf),
			cur_pref_score));

		/* Prune candidates not "significantly better" than our current AP.
		 * Notes:
		 * - The definition of "significantly better" may not be the same for
		 *   all candidates.  For example, Jason suggests using a different
		 *   threshold for our previous AP, if we roamed before.
		 * - The metric will undoubtedly need to change.  First pass, just use
		 *   the RSSI of the probe response.
		 */

		/* Try to make roam_delta adaptive. lower threshold when our AP is very weak,
		 * falling the edge of sensitivity. Hold threshold when our AP is relative strong.
		 * (hold a minimum 10dB delta to avoid ping-pong roaming)
		 *
		 *	roam_trigger+10	roam_trigger	roam_trigger-10	roam_trigger-20
		 *	|		|		|		|
		 *	| roam_delta    | roam_delta	| roam_delta-10 | roam_delta-10
		 */
		if (!wlc_roam_scan_islazy(wlc, cfg, FALSE) &&
		    (cur_rssi <= wlc->band->roam_trigger) && (roam_delta > 10)) {
			WL_SRSCAN(("  adapting roam delta"));
			WL_ASSOC(("  adapting roam delta\n"));
			roam_delta =
				MAX((int)(roam_delta - (wlc->band->roam_trigger - cur_rssi)), 10);
		}

		WL_SRSCAN(("  criteria: rssi=%d roam_delta=%d",
			cur_pref_rssi, roam_delta));
		WL_ASSOC(("  criteria: rssi=%d roam_delta=%d\n",
			cur_pref_rssi, roam_delta));

		/* TODO
		 * It may be advisable not to apply roam delta-based prune once
		 * preference score is above a certain threshold.
		 */

		WL_ASSOC(("wl%d: ROAM: wlc_cook_join_targets, roam_delta = %d\n",
		      WLCWLUNIT(wlc), roam_delta));
		WL_ROAM(("ROAM: cur score %d(%d), trigger/delta %d/%d\n", cur_pref_score,
		(int)cur_pref_score-255, wlc->band->roam_trigger, roam_delta));

#ifdef WLWNM
		if (WLWNM_ENAB(wlc->pub) && wlc_wnm_bsstrans_is_product_policy(wlc)) {
			/* Zero trgt score if:
			 * It does not meet roam_delta improvement OR
			 * The sta_cnt is maxed and hence trgt does not accept new connections.
			 */
			for (i = 0; i < (int)wlc->as_info->join_targets->count; i++) {
				WL_WNM_BSSTRANS_LOG("bssid %02x:%02x rssi: cur:%d trgt:%d\n",
					bip[i]->BSSID.octet[4], bip[i]->BSSID.octet[5],
					cur_pref_rssi, join_pref[i].rssi);
				if (roam->reason == WLC_E_REASON_BSSTRANS_REQ) {
					bool is_below_rssi_thresh = FALSE;
					if (wlc_wnm_btm_get_rssi_thresh(wlc) != 0) {
						/* convert negative rssi to positive */
						uint32 rssi_thresh =
							WLCMAXVAL(WLC_JOIN_PREF_BITS_RSSI) +
							wlc_wnm_btm_get_rssi_thresh(wlc);
						if (join_pref[i].rssi < rssi_thresh) {
							/* below rssi thresh */
							is_below_rssi_thresh = TRUE;
							WL_WNM_BSSTRANS_LOG("bss %02x:%02x "
								"rssi below thresh:%d trgt:%d\n",
								bip[i]->BSSID.octet[4],
								bip[i]->BSSID.octet[5],
								rssi_thresh,
								join_pref[i].rssi);
						}
					}
					/* For BTM-req initiated roam, ignore candidates worse
					   than current AP and also lower than rssi_thresh, if it
					   is configured.
					 */
					if (join_pref[i].rssi < cur_pref_rssi &&
						is_below_rssi_thresh) {
						WL_WNM_BSSTRANS_LOG("bss %02x:%02x "
							"rssi below current:%d trgt:%d\n",
							bip[i]->BSSID.octet[4],
							bip[i]->BSSID.octet[5],
							cur_pref_rssi,
							join_pref[i].rssi);
						wlc_zero_pref(&join_pref[i]);
					}
				}
				else if ((join_pref[i].rssi < (cur_pref_rssi + roam_delta))) {
					wlc_zero_pref(&join_pref[i]);
				}
				if (bip[i]->flags2 & WLC_BSS_MAX_STA_CNT) {
					WL_WNM_BSSTRANS_LOG("zero score; sta_cnt_maxed: %d\n",
						!!(bip[i]->flags2 & WLC_BSS_MAX_STA_CNT), 0, 0, 0);
					wlc_zero_pref(&join_pref[i]);
				}
			}
			/* Sort once more to pull zero score targets to beginning */
			wlc_sort_on_join_pref_score(cfg, join_pref);

			if (wlc_bss_connected(cfg)) {
				score_delta = wlc_wnm_bsstrans_get_scoredelta(wlc);
			}
			WL_WNM_BSSTRANS_LOG("score_delta %d roam_delta:%d\n", score_delta,
				roam_delta, 0, 0);
		} else
#endif /* WLWNM */
			score_delta = roam_delta;

		/* find cutoff point for pruning.
		 * already sorted in increasing order of join_pref_score
		 */
		for (i = 0; i < (int)wlc->as_info->join_targets->count; i++) {
			if (join_pref[i].score > (cur_pref_score + score_delta)) {
				break;
			}
#ifdef WLWNM
			else if (WLWNM_ENAB(wlc->pub) && wlc_wnm_bsstrans_is_product_policy(wlc)) {
				WL_WNM_BSSTRANS_LOG("bss %02x:%02x score not met cur:%d trgt:%d\n",
					bip[i]->BSSID.octet[4], bip[i]->BSSID.octet[5],
					cur_pref_score, join_pref[i].score);

			}
#endif /* WLWNM */
		}

		/* Prune, finally.
		 * - move qualifying APs to the beginning of list
		 * - note the boundary of the qualifying AP list
		 * - ccx-based pruning is done in wlc_join_attempt()
		 */
		for (j = 0; i < (int)wlc->as_info->join_targets->count; i++) {
			/* one last check:
			 * when roaming in the same band, the band RSSI boost should be disabled.
			 */
			if (wlc_bss_connected(cfg) &&
				(CHSPEC2WLC_BAND(bip[i]->chanspec) ==
				CHSPEC2WLC_BAND(current_bss->chanspec))) {
				uint32 rssi_no_boost = 0;
				wlc_bss_pref_score(cfg, bip[i], FALSE, &rssi_no_boost);
				if (rssi_no_boost <= (cur_pref_rssi + roam_delta)) {
					continue;
				}
			}
#if defined(SLAVE_RADAR) || defined(CLIENT_CSA)
			/* With slave radar on 4366 (or any chip with sub-band radar detection
			 * capability), control channel of an AP might be accessible but some other
			 * (non-control) sub-band of the APs chanspec might be blocked/inactive due
			 * to recent radar. Prune such APs from our list of valid APs.
			 */
			{
				wlcband_t *band = wlc->bandstate[CHSPEC_IS2G(bip[i]->chanspec) ?
					BAND_2G_INDEX : BAND_5G_INDEX];
				chanspec_t chspec = wlc_max_valid_bw_chanspec(wlc,
						band, cfg, bip[i]->chanspec);
				if (chspec == INVCHANSPEC ||
						(wlc_radar_chanspec(wlc->cmi, chspec) &&
						!wlc_valid_dfs_chanspec(wlc, chspec))) {
					WL_ASSOC(("wl%d: JOIN: prune blocked DFS ch (0x%x)\n",
							WLCWLUNIT(wlc), bip[i]->chanspec));
					continue;
				}
			}
#endif /* SLAVE_RADAR || CLIENT_CSA */
			/* move join_pref[i].score to join_pref[j].score */
			join_pref[j].score = join_pref[i].score;
			/* swap bip[i] with bip[j]
			 * moving bip[i] to bip[j] alone without swapping causes memory leak.
			 * by swapping, the ones below threshold are still left in bip
			 * so that they can be freed at later time.
			 */
			tmp_bi = bip[j];
			bip[j] = bip[i];
			bip[i] = tmp_bi;
			WL_ASSOC(("wl%d: ROAM: cook_join_targets, after prune, roam_metric[%s] "
				"= 0x%x\n", WLCWLUNIT(wlc),
				bcm_ether_ntoa(&bip[j]->BSSID, eabuf), join_pref[j].score));
			WL_ROAM(("ROAM:%d %s %d(%d) Q%d\n", j,
				bcm_ether_ntoa(&bip[j]->BSSID, eabuf), join_pref[j].score,
				(int)join_pref[j].score - 255, bip[j]->qbss_load_chan_free));
			j++;
		}
		wlc->as_info->join_targets_last = j;
		WL_SRSCAN(("  result: %d target(s)", wlc->as_info->join_targets_last));
		WL_ASSOC(("  result: %d target(s)\n", wlc->as_info->join_targets_last));
		WL_ROAM(("ROAM: %d targets after prune\n", j));
	}

#ifdef BCMCCX
	/* Broadcom AP Selection Method calls for Channel Free Sorting
	 * for all APs with
	 * 	(join pref. score >=
	 *		highest join pref. score - WLC_CHANNEL_FREE_SORTING_RANGE)
	 * the scores are in dB
	 * See CCXv4 Test Plan 4.1.1
	 */
	if (CCX_ENAB(wlc->pub) && wlc->as_info->join_targets_last > 1) {
		uint32 max, threshold;
		max = join_pref[wlc->as_info->join_targets_last - 1].score;
		threshold = max - WLC_CHANNEL_FREE_SORTING_RANGE;
		WL_ASSOC(("wl%d: %s: ROAM: Channel Free Sort - max 0x%x threshold 0x%x\n",
			wlc->pub->unit, __FUNCTION__, max, threshold));
		for (i = 0; i < (int)wlc->as_info->join_targets_last; i++) {
			if (join_pref[i].score >= threshold) {
				break;
			}
		}

		WL_ASSOC(("wl%d: cutoff for channel free sorting in BSS %s\n",
			WLCWLUNIT(wlc), bcm_ether_ntoa(&bip[i]->BSSID, eabuf)));

		for (j = (int)wlc->as_info->join_targets_last; --j >= 0;) {
			int k;
			bool done = TRUE;
			done = TRUE;
			for (k = i; k < j; k++) {
				if (bip[k]->qbss_load_chan_free > bip[k+1]->qbss_load_chan_free) {
					WL_ROAM(("ROAM: swap by QBSS %d(Q%d) <> %d(Q%d)\n", k,
						bip[k]->qbss_load_chan_free, k+1,
						bip[k+1]->qbss_load_chan_free));
					tmp_bi = bip[k];
					bip[k] = bip[k+1];
					bip[k+1] = tmp_bi;
					done = FALSE;
				}
			}
			if (done == TRUE) {
				break;
			}
		}
	}
#endif /* BCMCCX */

	MFREE(wlc->osh, join_pref, join_pref_score_size);

	/* Now sort pruned list using per-port criteria */
	if (wlc->pub->wlfeatureflag & WL_SWFL_WLBSSSORT)
		(void)wl_sort_bsslist(wlc->wl, bip);

#ifdef OPPORTUNISTIC_ROAM
	if (memcmp(cfg->join_bssid.octet, BSSID_INVALID, sizeof(cfg->join_bssid.octet)) != 0) {
		j = (int)wlc->as_info->join_targets->count - 1;
		for (i = j; i >= 0; i--) {
			int k;
			if (memcmp(cfg->join_bssid.octet, bip[i]->BSSID.octet,
			           sizeof(bip[i]->BSSID.octet)) == 0) {
				tmp_bi = bip[i];

				for (k = i; k < j; k++)
				{
					bip[k] = bip[k+1];
				}
				bip[j] = tmp_bi;
				break;
			}
		}
		memcpy(cfg->join_bssid.octet, BSSID_INVALID, sizeof(cfg->join_bssid.octet));
	}
#endif /* OPPORTUNISTIC_ROAM */
} /* wlc_cook_join_targets */

/** use to reassoc to a BSSID on a particular channel list */
int
wlc_reassoc(wlc_bsscfg_t *cfg, wl_reassoc_params_t *reassoc_params)
{
	wlc_info_t *wlc = cfg->wlc;
	chanspec_t* chanspec_list = NULL;
	int channel_num = reassoc_params->chanspec_num;
	struct ether_addr *bssid = &(reassoc_params->bssid);
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_assoc_t *as = cfg->assoc;
	int ret;

	if (!BSSCFG_STA(cfg) || !cfg->BSS) {
		WL_ERROR(("wl%d: %s: bad argument STA %d BSS %d\n",
		          wlc->pub->unit, __FUNCTION__, BSSCFG_STA(cfg), cfg->BSS));
		return BCME_BADARG;
	}

	/* add the reassoc request in the assoc_req list
	 * if someone is already in the process of association
	 */
	if ((ret = wlc_mac_request_entry(wlc, cfg, WLC_ACTION_REASSOC)) > 0) {
		WL_ASSOC(("wl%d.%d: REASSOC request queued at slot %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ret));
		return BCME_OK;
	}
	else if (ret < 0) {
		WL_ERROR(("wl%d.%d: REASSOC request failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		return ret;
	}

	WL_ASSOC(("wl%d : %s: Attempting association to %s\n",
	          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(bssid, eabuf)));

	/* fix dangling pointers */
	if (channel_num)
		chanspec_list = reassoc_params->chanspec_list;

	wlc_assoc_init(cfg, AS_ROAM);
	cfg->roam->reason = WLC_E_REASON_INITIAL_ASSOC;

	if (!bcmp(cfg->BSSID.octet, bssid->octet, ETHER_ADDR_LEN)) {
		/* Clear the current BSSID info to allow a roam-to-self */
		wlc_bss_clear_bssid(cfg);
	}

	/* Since doing a directed roam, use the cache if available */
	if (ASSOC_CACHE_ASSIST_ENAB(wlc))
		as->flags |= AS_F_CACHED_ROAM;

	wlc_assoc_scan_prep(cfg, NULL, bssid, chanspec_list, channel_num);
	return BCME_OK;
} /* wlc_reassoc */

static void
wlc_assoc_scan_prep(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	const struct ether_addr *bssid, const chanspec_t *chanspec_list, int channel_num)
{
#ifdef EXT_STA
	if (WLEXTSTA_ENAB(cfg->wlc->pub)) {
		if (cfg->assoc->type == AS_ASSOCIATION) {
			/* for BSS assoc generate event only once */
			if (!cfg->BSS || cfg->assoc->ess_retries == 1)
				/* N.B: bss_type passed through auth_type event field */
				wlc_bss_mac_event(cfg->wlc, cfg, WLC_E_JOIN_START, NULL, 0, 0,
					wlc_assoc_bsstype(cfg), NULL, 0);
		} else
			wlc_bss_mac_event(cfg->wlc, cfg, WLC_E_ROAM_START, NULL, 0,
				cfg->roam->reason, 0, 0, 0);
	}
#endif // endif

	wlc_assoc_scan_start(cfg, scan_params, bssid, chanspec_list, channel_num);
}

#ifdef EXT_STA
static bool
wlc_wps_pbcactive(wlc_info_t *wlc)
{
	uint8 *parse;
	int parse_len;
	uint i;
	uint8* wps_ie;

	for (i = 0; i < wlc->scan_results->count; i++) {
		parse = (uint8*)(wlc->scan_results->ptrs[i]->bcn_prb) + DOT11_BCN_PRB_LEN;
		parse_len = wlc->scan_results->ptrs[i]->bcn_prb_len - DOT11_BCN_PRB_LEN;
		while ((wps_ie = (uint8 *)bcm_find_wpsie(parse, parse_len)) != NULL) {
				int wps_len = wps_ie[TLV_LEN_OFF] + TLV_HDR_LEN;
				uint8 *wps_attrs = wps_ie + WPS_IE_FIXED_LEN;
				int wps_attr_len = wps_len - WPS_IE_FIXED_LEN;
				wps_at_fixed_t *wps_attr;

				WL_INFORM(("%s(): found WPS ie\n", __FUNCTION__));

				parse_len -= (int)(wps_ie - parse) + wps_len;
				parse = wps_ie + wps_len;

				while ((wps_attr =
					bcm_wps_find_at((wps_at_fixed_t *)wps_attrs, wps_attr_len,
					WPS_ATID_SEL_REGISTRAR)) != NULL) {
					WL_INFORM(("%s(): found WPS_ATID_SEL_REGISTRAR, val=%d\n",
						__FUNCTION__, wps_attr->data[0]));
					if (wps_attr->data[0] == 1)
						return TRUE;
					else
						return FALSE;

				}
		}
	}

	return FALSE;
}
#endif /* EXT_STA */

static int
wlc_assoc_bsstype(wlc_bsscfg_t *cfg)
{
	if (cfg->assoc->type == AS_ASSOCIATION)
		if (cfg->target_bss->infra == 0)
			return DOT11_BSSTYPE_INDEPENDENT;
		else if (cfg->target_bss->infra == 1)
			return DOT11_BSSTYPE_INFRASTRUCTURE;
		else
			return DOT11_BSSTYPE_ANY;
	else
		return DOT11_BSSTYPE_INFRASTRUCTURE;
}

/** check if connections other than 'cfg' are all doing excursion */
static bool
wlc_assoc_excursion_query_others(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_bsscfg_t *bc;
	int idx;

	FOREACH_AS_STA(wlc, idx, bc) {
		if (bc == cfg)
			continue;
		if (!BSS_EXCRX_ENAB(wlc, bc))
			break;
	}

	return bc == NULL;
}

/** chanspec_list being NULL/channel_num being 0 means all available chanspecs */
static void
wlc_assoc_scan_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	const struct ether_addr *bssid, const chanspec_t *chanspec_list, int channel_num)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_roam_t *roam = cfg->roam;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	int bss_type = wlc_assoc_bsstype(cfg), idx;
	int err = BCME_ERROR;
	bool assoc = (as->type == AS_ASSOCIATION);
	wlc_ssid_t ssid;
	chanspec_t* chanspecs = NULL;
	uint chanspecs_size = 0;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	char *ssidbuf;
#endif /* BCMDBG || WLMSG_ASSOC || WLEXTLOG */
	chanspec_t *target_list = NULL;
	uint target_num = 0;
	int active_time = -1;
	int nprobes = -1;
	uint chn_cache_count = bcm_bitcount(roam->roam_chn_cache.vec, sizeof(chanvec_t));

	/* specific bssid is optional */
	if (bssid == NULL)
		bssid = &ether_bcast;
	else {
		wl_assoc_params_t *assoc_params = wlc_bsscfg_assoc_params(cfg);
		if (assoc_params && assoc_params->bssid_cnt) {
			/* When bssid list is specified, increase the default dwell times */
			nprobes = WLC_BSSID_SCAN_NPROBES;
			active_time = WLC_BSSID_SCAN_ACTIVE_TIME;
		}
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	ssidbuf = (char *)MALLOC(wlc->osh, SSID_FMT_BUF_LEN);
#endif /* BCMDBG || WLMSG_ASSOC || WLEXTLOG */

	if (assoc) {
		/* standard association */
		ssid.SSID_len = target_bss->SSID_len;
		bcopy(target_bss->SSID, ssid.SSID, ssid.SSID_len);

		WL_SRSCAN(("starting assoc scan"));
		WL_ASSOC(("starting assoc scan\n"));
	} else {
		wlc_bss_info_t *current_bss = cfg->current_bss;
		bool partial_scan_ok;

		/* roaming */
		ssid.SSID_len = current_bss->SSID_len;
		bcopy(current_bss->SSID, ssid.SSID, ssid.SSID_len);

		/* Assume standard full roam. */
		WL_SRSCAN(("roam scan: reason=%d rssi=%d", roam->reason, cfg->link->rssi));
		WL_ASSOC(("roam scan: reason=%d rssi=%d\n", roam->reason, cfg->link->rssi));
		roam->roam_type = ROAM_FULL;

		/* Force full scan for any other roam reason but these */
		partial_scan_ok = ((roam->reason == WLC_E_REASON_LOW_RSSI) ||
			(roam->reason == WLC_E_REASON_BCNS_LOST) ||
			(roam->reason == WLC_E_REASON_MINTXRATE) ||
			(roam->reason == WLC_E_REASON_TXFAIL));

#ifdef BCMCCX
		/* AP-assisted roaming
		 * Do not adopt the ccx channel list when the app has specified
		 * the channel list.
		 */
		if (CCX_ENAB(wlc->pub) && wlc->ccx->fast_roam && !channel_num) {
			WL_ASSOC(("wl%d: ROAM: CCX FAST, using AP channel list (%d channels)\n",
				WLCWLUNIT(wlc), wlc->ccx->ap_channel_num));
			chanspec_list = wlc->ccx->ap_chanspec_list;
			channel_num = wlc->ccx->ap_channel_num;
			WL_SRSCAN(("starting CCX directed scan"));
			WL_ASSOC(("starting CCX directed scan\n"));
		} else
#endif /* BCMCCX */
#ifdef WLRCC
		/* Roam scan only on selected channels */
		if (WLRCC_ENAB(wlc->pub) && roam->rcc_valid) {
			/* Scan list will be overwritten later */
			WL_SRSCAN(("starting RCC directed scan"));
			WL_ASSOC(("starting RCC directed scan\n"));
		} else
#endif /* WLRCC */
		if (channel_num != 0) {
			ASSERT(chanspec_list);
			WL_SRSCAN(("starting user directed scan"));
			WL_ASSOC(("starting user directed scan\n"));
		} else if (partial_scan_ok && roam->cache_valid && roam->cache_numentries > 0) {
			uint i, chidx = 0;

			roam->roam_type = ROAM_PARTIAL;

			chanspecs_size = roam->cache_numentries * sizeof(chanspec_t);
			chanspecs = (chanspec_t *)MALLOC(wlc->pub->osh, chanspecs_size);
			if (chanspecs == NULL) {
				err = BCME_NOMEM;
				goto fail;
			}

			WL_SRSCAN(("starting partial scan"));
			WL_ASSOC(("starting partial scan\n"));

			/* We have exactly one AP in our roam candidate list, so scan it
			 * whether or not we are blocked from associated to it due to a prior
			 * roam event
			 */
			if (roam->cache_numentries == 1) {
				WL_SRSCAN(("  channel %d",
					wf_chspec_ctlchan(roam->cached_ap[0].chanspec)));
				WL_ASSOC(("  channel %d\n",
					wf_chspec_ctlchan(roam->cached_ap[0].chanspec)));
				chanspecs[0] = roam->cached_ap[0].chanspec;
				chidx = 1;
			}
			/* can have multiple entries on one channel */
			else {
				for (idx = 0; idx < (int)roam->cache_numentries; idx++) {
					/* If not valid, don't add it to the chanspecs to scan */
					if (roam->cached_ap[idx].time_left_to_next_assoc)
						continue;

					/* trim multiple APs on the same channel */
					for (i = 0; chidx && (i < chidx); i++) {
						if (chanspecs[i] == roam->cached_ap[idx].chanspec)
							break;
					}

					if (i == chidx) {
						WL_SRSCAN(("  channel %d",
							wf_chspec_ctlchan(roam->
								cached_ap[idx].chanspec)));
						WL_ASSOC(("  channel %d\n",
							wf_chspec_ctlchan(roam->
								cached_ap[idx].chanspec)));
						chanspecs[chidx++] = roam->cached_ap[idx].chanspec;
					}
				}
			}
			chanspec_list = chanspecs;
			channel_num = chidx;

			WL_ASSOC(("wl%d: SCAN: using the cached scan results list (%d channels)\n",
			          WLCWLUNIT(wlc), channel_num));
		}
		/* Split roam scan. */
		else if (roam->split_roam_scan) {
			uint8 chn_list;
			uint32 i, j, count, hot_count;

			roam->roam_type = ROAM_SPLIT_PHASE;

			/* Invalidate the roam cache so that we don't somehow transition
			 * to partial roam scans before all phases complete.
			 */
			roam->cache_valid = FALSE;

			/* Limit to max value */
			if (roam->roam_chn_cache_locked)
				chn_cache_count = MIN(roam->roam_chn_cache_limit, chn_cache_count);

			/* Allocate the chanspec list. */
			chanspecs_size = NBBY * sizeof(chanvec_t) * sizeof(chanspec_t);
			chanspecs = (chanspec_t *)MALLOC(wlc->pub->osh, chanspecs_size);
			if (chanspecs == NULL) {
				err = BCME_NOMEM;
				goto fail;
			}

			WL_SRSCAN(("starting split scan: phase %d",
				roam->split_roam_phase + 1));
			WL_ASSOC(("starting split scan: phase %d\n",
				roam->split_roam_phase + 1));

			/* Always include home channel as the first to be scanned */
			count = 0;
			chanspecs[count++] = CH20MHZ_CHSPEC(
				wf_chspec_ctlchan(cfg->current_bss->chanspec));
			hot_count = 1;

			/* Fill in the chanspec list. */
			for (i = 0; i < sizeof(chanvec_t); i++) {
				chn_list = roam->roam_chn_cache.vec[i];
				idx = i << 3;
				for (j = 0; j < NBBY; j++, idx++, chn_list >>= 1) {
					if (!wlc_valid_chanspec_db(wlc->cmi,
						CH20MHZ_CHSPEC(idx)))
						continue;

					/* Home channel is already included */
					if (chanspecs[0] == CH20MHZ_CHSPEC(idx))
						continue;

					if ((chn_list & 0x1) &&
					    (hot_count <= chn_cache_count)) {
						hot_count++;

						/* Scan hot channels in phase 1 */
						if (roam->split_roam_phase == 0) {
							chanspecs[count++] =
								CH20MHZ_CHSPEC(idx);
							WL_SRSCAN(("  channel %d", idx));
							WL_ASSOC(("  channel %d\n", idx));
						}
					} else if (roam->split_roam_phase != 0) {
						/* Scan all other channels in phase 2 */
						chanspecs[count++] = CH20MHZ_CHSPEC(idx);
						WL_SRSCAN(("  channel %d", idx));
						WL_ASSOC(("  channel %d\n", idx));
					}
				}
			}
			chanspec_list = chanspecs;
			channel_num = count;

			/* Increment the split roam scan phase counter (cyclic). */
			roam->split_roam_phase =
				(roam->split_roam_phase == 0) ? 1 : 0;
		}
		else {
			WL_SRSCAN(("starting full scan"));
			WL_ASSOC(("starting full scan\n"));
		}
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	WL_ASSOC(("wl%d: SCAN: wlc_assoc_scan_start, starting a %s%s scan for SSID %s\n",
	          WLCWLUNIT(wlc), !ETHER_ISMULTI(bssid) ? "Directed " : "", assoc ? "JOIN" : "ROAM",
	          ssidbuf ? (wlc_format_ssid(ssidbuf, ssid.SSID, ssid.SSID_len), ssidbuf) : "???"));
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	/* DOT11_MAX_SSID_LEN check added so that we do not create ext logs for bogus
	 * joins of garbage ssid issued on XP
	 */
	if (assoc && (ssid.SSID_len != DOT11_MAX_SSID_LEN)) {
		WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_JOIN_START_ID,
			WL_LOG_LEVEL_ERR, 0, 0, ssidbuf ?
			(wlc_format_ssid(ssidbuf, ssid.SSID, ssid.SSID_len), ssidbuf)
			: "???");
	}
#endif /* BCMDBG || WLMSG_ASSOC || WLEXTLOG */

	/* if driver is down due to mpc, turn it on, otherwise abort */
	wlc->mpc_scan = TRUE;
	wlc_radio_mpc_upd(wlc);

	if (!wlc->pub->up) {
		WL_ASSOC(("wl%d: wlc_assoc_scan_start, can't scan while driver is down\n",
		          wlc->pub->unit));
		goto stop;
	}

#ifdef WLAWDL
		if (AWDL_ENAB(wlc->pub))
			;	/* empty */
		else
#endif /* WLAWDL */
#ifdef WLMCHAN
	/* XXX mChannel: Go wherever we are asked to if we are the only one
	 * associating/reassociating, otherwise limit it to the wlc->home_chanspec
	 * until multiple channel is supported.
	 */
	if (MCHAN_ENAB(wlc->pub)) {
		;	/* empty */
	}
	else
#endif /* WLMCHAN */
	if (BSS_EXCRX_ENAB(wlc, cfg) ||
	    wlc_assoc_excursion_query_others(wlc, cfg))
		;	/* empty */
	else
	if (wlc->stas_associated > 0 &&
#ifdef WLBTAMP
	    (BTA_ENAB(wlc->pub) && !wlc_bta_active(wlc->bta)) &&
#endif // endif
	    (wlc->stas_associated > 1 || !cfg->associated)) {
		uint32 ctlchan, home_ctlchan;

		home_ctlchan = wf_chspec_ctlchan(wlc->home_chanspec);
		for (idx = 0; idx < channel_num; idx ++) {
			ctlchan = wf_chspec_ctlchan(chanspec_list[idx]);
			if (CH20MHZ_CHSPEC(ctlchan) == CH20MHZ_CHSPEC(home_ctlchan))
				break;
		}
		if (channel_num == 0 || idx < channel_num) {
			WL_ASSOC(("wl%d: wlc_assoc_scan_start: use shared chanspec "
			          "wlc->home_chanspec 0x%x\n",
			          wlc->pub->unit, wlc->home_chanspec));
			chanspec_list = &wlc->home_chanspec;
			channel_num = 1;
		}
		else {
			WL_ASSOC(("wl%d: wlc_assoc_scan_start, no chanspec\n",
			          wlc->pub->unit));
			goto stop;
		}
	}

	/* clear scan_results in case there are some left over from a prev scan */
	wlc_bss_list_free(wlc, wlc->scan_results);

	wlc_set_mac(cfg);

#ifdef WLP2P
	/* init BSS block and ADDR_BMP entry to allow ucode to follow
	 * the necessary chains of states in the transmit direction
	 * prior to association.
	 */
	if (BSS_P2P_ENAB(wlc, cfg))
		wlc_p2p_prep_bss(wlc->p2p, cfg);
#endif // endif

	/* If association scancache use is enabled, check for a hit in the scancache.
	 * Do not check the cache if we are re-running the assoc scan after a cached
	 * assoc failure
	 */
	if (ASSOC_CACHE_ASSIST_ENAB(wlc) &&
	    (assoc || (as->flags & AS_F_CACHED_ROAM)) &&
	    !(as->flags & (AS_F_CACHED_CHANNELS | AS_F_CACHED_RESULTS))) {
		wlc_assoc_cache_eval(wlc, bssid, &ssid, bss_type,
		                     chanspec_list, channel_num,
		                     wlc->scan_results, &target_list, &target_num);
	}

	/* reset the assoc cache flags since this may be a retry of a cached attempt */
	as->flags &= ~(AS_F_CACHED_CHANNELS | AS_F_CACHED_RESULTS);

	/* narrow down the channel list if the cache eval came up with a short list */
	if (ASSOC_CACHE_ASSIST_ENAB(wlc) && target_num > 0) {
		WL_ASSOC(("wl%d: JOIN: using the cached scan results "
		          "to create a channel list len %u\n",
		          WLCWLUNIT(wlc), target_num));
		as->flags |= AS_F_CACHED_CHANNELS;
		chanspec_list = target_list;
		channel_num = target_num;
	}

	/* Use cached results if there was a hit instead of performing a scan */
	if (ASSOC_CACHE_ASSIST_ENAB(wlc) && wlc->scan_results->count > 0) {
		WL_ASSOC(("wl%d: JOIN: using the cached scan results for assoc (%d hits)\n",
		          WLCWLUNIT(wlc), wlc->scan_results->count));

		as->flags |= AS_F_CACHED_RESULTS;
#if defined(EXT_STA) && (NDISVER < 0x0630)
		if (WLEXTSTA_ENAB(wlc->pub) &&
		    wlc_wps_pbcactive(wlc)) {
			WL_INFORM(("%s(): delay 0.5sec...\n", __FUNCTION__));
			wlc_assoc_change_state(cfg, AS_JOIN_CACHE_DELAY);
			wl_add_timer(wlc->wl, as->timer, 500, 0);
		}
		else
#endif /* EXT_STA && (NDISVER < 0x0630) */
		{
			wlc_assoc_change_state(cfg, AS_SCAN);
			wlc_assoc_scan_complete(wlc, WLC_E_STATUS_SUCCESS, cfg);
		}

		err = BCME_OK;
	} else {
		bool scan_suppress_ssid = FALSE;
		int passive_time = -1;
		int home_time = -1;
		int scan_type = 0;
		int away_ch_limit = 0;
#ifdef EXT_STA
		if (WLEXTSTA_ENAB(wlc->pub)) {
			scan_suppress_ssid = wlc->scan_suppress_ssid;
			if ((as->type == AS_ROAM) &&
			    (roam->reason == WLC_E_REASON_BCNS_LOST) &&
			    !roam->bcns_lost) {
				WL_ASSOC(("%s(): fast roam scan on active channels only...\n",
				          __FUNCTION__));
				/* set passive_time to 0 to bypass the passive channels */
				/* and use the default scan params in wlc_scan() */
				passive_time = 0;
				wlc_iovar_getint(wlc, "scan_assoc_time_default", &active_time);
				wlc_iovar_getint(wlc, "scan_home_time_default", &home_time);
				WL_ASSOC(("%s(): set default active_time=%d, home_time=%d\n",
				          __FUNCTION__, active_time, home_time));
			}
		} else
#endif /* EXT_STA */
		/* override default scan params */
		if (scan_params != NULL) {
			scan_type = scan_params->scan_type;
			nprobes = scan_params->nprobes;
			active_time = scan_params->active_time;
			passive_time = scan_params->passive_time;
			home_time = scan_params->home_time;
		}
		if (WLRCC_ENAB(wlc->pub)) {
			if (
#ifdef BCMCCX
				!(CCX_ENAB(wlc->pub) && wlc->ccx->fast_roam) &&
#endif /* BCMCCX */
				(chanspec_list == NULL) && roam->rcc_valid &&
				!roam->roam_fullscan) {
				chanspec_list = roam->rcc_channels;
				channel_num = roam->n_rcc_channels; /* considering to use 1 */
				roam->roam_scan_started = TRUE;
				WL_ROAM(("RCC: %d scan channels\n", channel_num));
			} else {
				WL_ROAM(("RCC: chanspec_list 0x%X, rcc valid %d, roam_fullscan %d, "
					"%d scan channels\n", (int)chanspec_list, roam->rcc_valid,
					roam->roam_fullscan, channel_num));
			}
			roam->roam_fullscan = FALSE;
		}

		if (channel_num) {
			WL_SRSCAN(("scan start: %d channel(s)", channel_num));
			WL_ASSOC(("scan start: %d channel(s)\n", channel_num));
		} else {
			WL_SRSCAN(("scan start: all channels"));
			WL_ASSOC(("scan start: all channels\n"));
		}

		/* active time for association recreation */
#ifdef NLO
		if (cfg->nlo && as->type == AS_RECREATE) {
			active_time	= WLC_NLO_SCAN_RECREATE_TIME;
		}
		else
#endif /* NLO */
		{
			if (as->flags & AS_F_SPEEDY_RECREATE)
				active_time = WLC_SCAN_RECREATE_TIME;
		}

		/* Extend home time for lazy roam scan
		 * Must be longer than pm2_sleep_ret_time and beacon period.
		 * Set it to 2*MAX(pm2_sleep_ret_time, 1.5*beaocn_period).
		 * Also, limit to 2 active scan per home_away_time
		 * (assume beacon period = 100, and scan_assoc_time=20).
		 */
		if ((roam->reason == WLC_E_REASON_LOW_RSSI) &&
		    wlc_roam_scan_islazy(wlc, cfg, FALSE)) {
			home_time = 2 * MAX((int)cfg->pm->pm2_sleep_ret_time,
				(int)cfg->current_bss->beacon_period * 3 / 2);
			away_ch_limit = 2;
		}

		/* kick off a scan for the requested SSID, possibly broadcast SSID. */
		err = wlc_scan(wlc->scan, bss_type, bssid, 1, &ssid, scan_type, nprobes,
			active_time, passive_time, home_time, chanspec_list, channel_num, 0, TRUE,
			wlc_assoc_scan_complete, wlc, away_ch_limit, FALSE, scan_suppress_ssid,
			FALSE, FALSE, cfg, SCAN_ENGINE_USAGE_NORM, NULL, NULL);

		if (err == BCME_OK) {
			wlc_assoc_change_state(cfg, AS_SCAN);
		}

		/* when the scan is done, wlc_assoc_scan_complete() will be called to copy
		 * scan_results to join_targets and continue the association process
		 */
	}

	/* clean up short channel list if one was returned from wlc_assoc_cache_eval() */
	if (target_list != NULL)
		MFREE(wlc->osh, target_list, target_num * sizeof(chanspec_t));

stop:
	if (chanspecs != NULL)
		MFREE(wlc->pub->osh, chanspecs, chanspecs_size);

	wlc->mpc_scan = FALSE;
	wlc_radio_mpc_upd(wlc);

fail:
	if (err != BCME_OK) {
		/* Reset the state to IDLE if down, no chanspec or unable to
		 * scan.
		 */
		as->type = AS_NONE;
		wlc_assoc_change_state(cfg, AS_IDLE);

		wlc_assoc_req_remove_entry(wlc, cfg);
#ifdef EXT_STA
		if ((WLEXTSTA_ENAB(wlc->pub) && cfg->assoc_params) ||
#ifdef WLAWDL
			BSSCFG_AWDL(wlc, cfg) ||
#endif // endif
			FALSE) {
			wlc_bss_mac_event(wlc, cfg, WLC_E_SET_SSID, bssid,
			                  WLC_E_STATUS_FAIL, 0,
			                  WLC_DOT11_BSSTYPE(cfg->target_bss->infra),
			                  cfg->SSID, cfg->SSID_len);
		}
#endif /* EXT_STA */
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	if (ssidbuf != NULL)
		MFREE(wlc->osh, (void *)ssidbuf, SSID_FMT_BUF_LEN);
#endif // endif
	return;
} /* wlc_assoc_scan_start */

#ifdef WL_ASSOC_RECREATE
static void
wlc_speedy_recreate_fail(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;

	as->flags &= ~AS_F_SPEEDY_RECREATE;

	wlc_bss_list_free(wlc, wlc->as_info->join_targets);

	WL_ASSOC(("wl%d: %s: ROAM: Doing full scan since assoc-recreate failed\n",
	          WLCWLUNIT(wlc), __FUNCTION__));

	wlc_iovar_setint(wlc, "scan_passive_time", 125);
	wlc_iovar_setint(wlc, "scan_unassoc_time", 75);
	wlc_iovar_setint(wlc, "scan_assoc_time", 75);
	wlc_iovar_setint(wlc, "scan_home_time", 0);

#ifdef NLO
	/* event to get nlo assoc and scan params into bsscfg if nlo enabled */
	wlc_bss_mac_event(wlc, cfg, WLC_E_SPEEDY_RECREATE_FAIL, NULL, 0, 0, 0, 0, 0);
	if (cfg->nlo) {
		/* start assoc full scan using nlo parameters */
		wlc_assoc_scan_start(cfg, NULL, NULL,
			(cfg->assoc_params ? cfg->assoc_params->chanspec_list : NULL),
			(cfg->assoc_params ? cfg->assoc_params->chanspec_num : 0));
		wlc_bsscfg_assoc_params_reset(wlc, cfg);
	} else
#endif /* NLO */
	{
		wlc_assoc_scan_start(cfg, NULL, NULL, NULL, 0);
	}
}
#endif /* WL_ASSOC_RECREATE */

#ifdef WLSCANCACHE
static void
wlc_assoc_cache_eval(wlc_info_t *wlc, const struct ether_addr *BSSID, const wlc_ssid_t *SSID,
                     int bss_type, const chanspec_t *chanspec_list, uint chanspec_num,
                     wlc_bss_list_t *bss_list, chanspec_t **target_list, uint *target_num)
{
	chanspec_t *target_chanspecs;
	uint target_chanspec_alloc_num;
	uint target_chanspec_num;
	uint i, j;
	osl_t *osh;

	osh = wlc->osh;

	*target_list = NULL;
	*target_num = 0;

	if (SCANCACHE_ENAB(wlc->scan))
		wlc_scan_get_cache(wlc->scan, BSSID, 1, SSID, bss_type,
			chanspec_list, chanspec_num, bss_list);

	/* if there are no hits, just return with no cache assistance */
	if (bss_list->count == 0)
		return;

	if (wlc_assoc_cache_validate_timestamps(wlc, bss_list))
		return;

	WL_ASSOC(("wl%d: %s: %d hits, creating a channel list\n",
	          wlc->pub->unit, __FUNCTION__, bss_list->count));

	/* If the results are too old they might have stale information, so use a chanspec
	 * list instead to speed association.
	 */
	target_chanspec_num = 0;
	target_chanspec_alloc_num = bss_list->count;
	target_chanspecs = MALLOC(osh, sizeof(chanspec_t) * bss_list->count);
	if (target_chanspecs == NULL) {
		/* out of memory, skip cache assistance */
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(osh)));
		goto cleanup;
	}

	for (i = 0; i < bss_list->count; i++) {
		chanspec_t chanspec;
		uint8 ctl_ch;

		chanspec = bss_list->ptrs[i]->chanspec;

		/* convert a 40MHz or 80/160/8080Mhz chanspec to a 20MHz chanspec of the
		 * control channel since this is where we should scan for the BSS.
		 */
		if (CHSPEC_IS40(chanspec) || CHSPEC_IS80(chanspec) ||
			CHSPEC_IS160(chanspec) || CHSPEC_IS8080(chanspec)) {
			ctl_ch = wf_chspec_ctlchan(chanspec);
			chanspec = (chanspec_t)(ctl_ch | WL_CHANSPEC_BW_20 |
			                        CHSPEC_BAND(chanspec));
		}

		/* look for this bss's chanspec in the list we are building */
		for (j = 0; j < target_chanspec_num; j++)
			if (chanspec == target_chanspecs[j])
				break;

		/* if the chanspec is not already on the list, add it */
		if (j == target_chanspec_num)
			target_chanspecs[target_chanspec_num++] = chanspec;
	}

	/* Resize the chanspec list to exactly what it needed */
	if (target_chanspec_num != target_chanspec_alloc_num) {
		chanspec_t *new_list;

		new_list = MALLOC(osh, sizeof(chanspec_t) * target_chanspec_num);
		if (new_list != NULL) {
			memcpy(new_list, target_chanspecs,
			       sizeof(chanspec_t) * target_chanspec_num);
		} else {
			/* out of memory, skip cache assistance */
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(osh)));
			target_chanspec_num = 0;
		}

		MFREE(osh, target_chanspecs, sizeof(chanspec_t) * target_chanspec_alloc_num);
		target_chanspecs = new_list;
	}

	*target_list = target_chanspecs;
	*target_num = target_chanspec_num;

cleanup:
	/* clear stale scan_results */
	wlc_bss_list_free(wlc, wlc->scan_results);

	return;
} /* wlc_assoc_cache_eval */

/** If a cache assisted association attempt fails, retry with a regular assoc scan */
static void
wlc_assoc_cache_fail(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wl_join_scan_params_t *scan_params;
	const wl_join_assoc_params_t *assoc_params;
	const struct ether_addr *bssid = NULL;
	const chanspec_t *chanspec_list = NULL;
	int chanspec_num = 0;

	WL_ASSOC(("wl%d: %s: Association from cache failed, starting regular association scan\n",
	          WLCWLUNIT(wlc), __FUNCTION__));

	/* reset join_targets for new join attempt */
	wlc_bss_list_free(wlc, wlc->as_info->join_targets);

	scan_params = wlc_bsscfg_scan_params(cfg);
	if ((assoc_params = wlc_bsscfg_assoc_params(cfg)) != NULL) {
		bssid = &assoc_params->bssid;
		chanspec_list = assoc_params->chanspec_list;
		chanspec_num = assoc_params->chanspec_num;
	}

	wlc_assoc_scan_start(cfg, scan_params, bssid, chanspec_list, chanspec_num);
}

#endif /* WLSCANCACHE */

void
wlc_pmkid_build_cand_list(wlc_bsscfg_t *cfg, bool check_SSID)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint i;
	wlc_bss_info_t *bi;

	WL_WSEC(("wl%d: building PMKID candidate list\n", wlc->pub->unit));

	/* Merge scan results and pmkid cand list */
	for (i = 0; i < wlc->scan_results->count; i++) {
#if defined(BCMDBG) || defined(WLMSG_WSEC)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif	/* BCMDBG || WLMSG_WSEC */

		bi = wlc->scan_results->ptrs[i];

		/* right network? if not, move on */
		if (check_SSID &&
			((bi->SSID_len != target_bss->SSID_len) ||
			bcmp(bi->SSID, target_bss->SSID, bi->SSID_len)))
			continue;

		WL_WSEC(("wl%d: wlc_pmkid_build_cand_list(): processing %s...",
			wlc->pub->unit, bcm_ether_ntoa(&bi->BSSID, eabuf)));

		wlc_pmkid_prep_list(wlc->pmkid_info, cfg, &bi->BSSID, bi->wpa2.flags);
	}

	/* if port's open, request PMKID cache plumb */
	ASSERT(cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED);
	if (cfg->wsec_portopen) {
		WL_WSEC(("wl%d: wlc_pmkid_build_cand_list(): requesting PMKID cache plumb...\n",
			wlc->pub->unit));
		wlc_pmkid_cache_req(wlc->pmkid_info, cfg);
	}
}

/**
 * Assoc/roam completion routine - must be called at the end of an assoc/roam process
 * no matter it finishes with success or failure or it doesn't finish due to abort.
 * wlc_set_ssid_complate() and wlc_roam_complete can be called individually whenever
 * it fits.
 */
static void
wlc_assoc_done(wlc_bsscfg_t *cfg, int status)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	uint type = cfg->assoc->type;
	int8 infra = cfg->target_bss->infra;

	if (type == AS_ASSOCIATION) {
		wlc_set_ssid_complete(cfg, status, &cfg->BSSID, WLC_DOT11_BSSTYPE(infra));
	} else {
		WL_SRSCAN(("assoc done: status %d", status));
		WL_ASSOC(("assoc done: status %d\n", status));

		if ((roam->roam_type == ROAM_SPLIT_PHASE) &&
		    (status == WLC_E_STATUS_NO_NETWORKS)) {

			/* Start the next phase of the split roam scan.
			 * (roam scan phase is incremented upon scan start)
			 */
			if (roam->split_roam_phase) {
				wlc_bss_list_free(wlc, wlc->as_info->join_targets);
				wlc_assoc_scan_prep(cfg, NULL, NULL, NULL, 0);
				return;
			}
		}

		wlc_roam_complete(cfg, status, &cfg->BSSID, WLC_DOT11_BSSTYPE(infra));
	}
}

/* XXX whoever causes an abort (with status WLC_E_STATUS_ABORT) must handle the completion
 * task (make call to wlc_roam_complete()/wlc_set_ssid_complete()) itself.
 */
static void
wlc_assoc_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg)
{
	int idx;
	wlc_bsscfg_t *valid_cfg;
	bool cfg_ok = FALSE;
	wlc_info_t *wlc;
	wlc_assoc_t *as;
	wlc_roam_t *roam;
	wlc_bss_info_t *target_bss;
	bool for_roam;
	uint8 ap_24G = 0;
	uint8 ap_5G = 0;
	uint i;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char *ssidbuf = NULL;
	const char *ssidstr;
	const char *msg_name;
	const char *msg_pref;
#endif /* BCMDBG || WLMSG_ASSOC */
#ifdef WLRCC
	bool cache_valid = cfg->roam->cache_valid;
	bool roam_active = cfg->roam->active;
#endif /* WLRCC */

	/* We have seen instances of where this function was called on a cfg that was freed.
	 * Verify cfg first before proceeding.
	 */
	wlc = (wlc_info_t *)arg;
	/* Must find a match in global bsscfg array before continuing */
	FOREACH_BSS(wlc, idx, valid_cfg) {
		if (valid_cfg == cfg) {
			cfg_ok = TRUE;
			break;
		}
	}
	if (!cfg_ok) {
		WL_ERROR(("wl%d: %s: no valid bsscfg matches cfg %p, exit\n",
		          WLCWLUNIT(wlc), __FUNCTION__, cfg));
		goto exit;
	}
	/* cfg has been validated, continue with rest of function */
	as = cfg->assoc;
	roam = cfg->roam;
	target_bss = cfg->target_bss;
	for_roam = (as->type != AS_ASSOCIATION);
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	msg_name = for_roam ? "ROAM" : "JOIN";
	msg_pref = !(ETHER_ISMULTI(&wlc->scan->bssid)) ? "Directed " : "";
	ssidbuf = (char *)MALLOC(wlc->osh, SSID_FMT_BUF_LEN);
	if (ssidbuf) {
		wlc_format_ssid(ssidbuf, target_bss->SSID, target_bss->SSID_len);
		ssidstr = ssidbuf;
	} else
		ssidstr = "???";
#endif /* BCMDBG || WLMSG_ASSOC */

	WL_ASSOC(("wl%d: SCAN: wlc_assoc_scan_complete\n", WLCWLUNIT(wlc)));
#ifdef WL_EVENT_LOG_COMPILE
	{
		wl_event_log_tlv_hdr_t tlv_log = {{0, 0}};

		tlv_log.tag = TRACE_TAG_STATUS;
		tlv_log.length = sizeof(uint);

		WL_EVENT_LOG(EVENT_LOG_TAG_TRACE_WL_INFO,
			for_roam ? TRACE_ROAM_SCAN_COMPLETE : WLC_E_SCAN_COMPLETE,
			tlv_log.t, status);
	}
#endif /* WL_EVENT_LOG_COMPILE */

	/* Delay roam scan by scan_settle_time watchdog tick(s) to allow
	 * other events like calibrations to run
	 */
	if (WLFMC_ENAB(wlc->pub) && roam->active)
		roam->scan_block = MAX(roam->scan_block, roam->partialscan_period);
	roam->scan_block = MAX(roam->scan_block, wlc->pub->tunables->scan_settle_time);

	if (status == WLC_E_STATUS_SUCCESS && roam->reassoc_param != NULL) {
		MFREE(wlc->osh, roam->reassoc_param, roam->reassoc_param_len);
		roam->reassoc_param = NULL;
	}

	if (status != WLC_E_STATUS_SUCCESS) {

		/* If roam scan is aborted, reset split roam scans.
		 * Do not clear the scan block to avoid continuous scan.
		 */
		if (for_roam && status == WLC_E_STATUS_ABORT) {
			roam->split_roam_phase = 0;
		}

		if (target_bss->infra == 1 || status != WLC_E_STATUS_SUPPRESS) {
#ifdef EXT_STA
			/* XXX PR57361 - if scan suppressed, we need inform the NDIS layer that
			 * it can close any outstanding blocks
			 */
			if (WLEXTSTA_ENAB(wlc->pub) && (status == WLC_E_STATUS_SUPPRESS)) {
				WL_ASSOC(("wl%d: scan suppressed, change status to "
				          "WLC_E_STATUS_NO_NETWORKS\n", WLCWLUNIT(wlc)));
				status = WLC_E_STATUS_NO_NETWORKS;
			}
#endif // endif
			/* XXX wlc_assoc_abort() or whoever issues the abort will need to
			 * perform the completion task. This way it gives callers chance
			 * to do any other tasks before perform the completion task.
			 */
			if (status != WLC_E_STATUS_ABORT)
				wlc_assoc_done(cfg, status);

			/* Exit association in progress, and let scan state machine restart the
			 * join process later.
			 */
			if ((status == WLC_E_STATUS_ABORT) && (as->type == AS_ASSOCIATION)) {
				WL_ERROR(("wl%d: Aborting in progress association, error in"
					"scan \n", wlc->pub->unit));

				wlc_assoc_abort(cfg);
			}
			goto exit;
		}
	}

	/* register scan results as possible PMKID candidates */
	if ((cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) &&
#ifdef WLFBT
		(!BSSCFG_IS_FBT(cfg) || !(cfg->WPA_auth & WPA2_AUTH_FT)) &&
#endif /* WLFBT */
		TRUE)
		wlc_pmkid_build_cand_list(cfg, FALSE);

	/* copy scan results to join_targets for the reassociation process */
	if (WLCAUTOWPA(cfg)) {
		if (wlc_bss_list_expand(cfg, wlc->scan_results, wlc->as_info->join_targets)) {
			WL_ERROR(("wl%d: wlc_bss_list_expand failed\n", WLCWLUNIT(wlc)));
			wlc_assoc_done(cfg, WLC_E_STATUS_FAIL);
			goto exit;
		}
	}
	else
		wlc_bss_list_xfer(wlc->scan_results, wlc->as_info->join_targets);

	wlc->as_info->join_targets_last = wlc->as_info->join_targets->count;

	if (wlc->as_info->join_targets->count > 0) {
		WL_ASSOC(("wl%d: SCAN for %s%s: SSID scan complete, %d results for \"%s\"\n",
			WLCWLUNIT(wlc), msg_pref, msg_name,
			wlc->as_info->join_targets->count, ssidstr));
	} else if (target_bss->SSID_len > 0) {
		WL_ASSOC(("wl%d: SCAN for %s%s: SSID scan complete, no matching SSIDs found "
			"for \"%s\"\n", WLCWLUNIT(wlc), msg_pref, msg_name, ssidstr));
	} else {
		WL_ASSOC(("wl%d: SCAN for %s: SSID scan complete, no SSIDs found for broadcast "
			"scan\n", WLCWLUNIT(wlc), msg_name));
	}

	/* sort join targets by signal strength */
	/* for roam, prune targets if they do not have signals stronger than our current AP */
	if (wlc->as_info->join_targets->count > 0) {
		wlc_bss_info_t **bip = wlc->as_info->join_targets->ptrs;

		if (!cfg->associated || !for_roam) {
			/* New Assoc, Clear cached channels */
			memset(roam->roam_chn_cache.vec, 0, sizeof(chanvec_t));
			roam->roam_chn_cache_locked = FALSE;

			/* wipe roam cache clean */
			roam->cache_valid = FALSE;
			roam->cache_numentries = 0;
		}

		/* Identify Multi AP Environment */
		for (i = 0; i < wlc->as_info->join_targets->count; i++) {
			if (CHSPEC_IS5G(bip[i]->chanspec)) {
				ap_5G++;
			}
			else {
				ap_24G++;
			}

			/* Build the split roam scan channel cache. */
			if (!roam->roam_chn_cache_locked) {
				setbit(roam->roam_chn_cache.vec,
					wf_chspec_ctlchan(bip[i]->chanspec));
			}
		}

		if (ap_24G > 1 || ap_5G > 1) {
			WL_ASSOC(("wl%d: ROAM: Multi AP Environment \r\n", WLCWLUNIT(wlc)));
			roam->multi_ap = TRUE;
		} else if (!cfg->associated) {
			/* Sticky multi_ap flag:
			 * During association, the multi_ap can only be set, not cleared.
			 */
			WL_ASSOC(("wl%d: ROAM: Single AP Environment \r\n", WLCWLUNIT(wlc)));
			roam->multi_ap = FALSE;
		} else {
			WL_ASSOC(("wl%d: ROAM: Keep %s AP Environment \r\n", WLCWLUNIT(wlc),
				(roam->multi_ap ? "multi" : "single")));
		}

		/* Update roam profile for possible multi_ap state change */
		wlc_roam_prof_update_default(wlc, cfg);

		/* No pruning to be done if this is a directed, cache assisted ROAM */
		if (for_roam && !(as->flags & AS_F_CACHED_ROAM)) {
			wlc_cook_join_targets(cfg, TRUE, cfg->link->rssi);
			WL_ASSOC(("wl%d: ROAM: %d roam target%s after pruning\n",
				WLCWLUNIT(wlc), wlc->as_info->join_targets_last,
				(wlc->as_info->join_targets_last == 1) ? "" : "s"));
		} else {
			wlc_cook_join_targets(cfg, FALSE, 0);
		}
		/* no results */
	} else if (for_roam && roam->cache_valid && roam->cache_numentries > 0) {
		/* We need to blow away our cache if we were roaming for entries that ended
		 * not existing. Maybe our AP changed channels?
		 */
		WL_ASSOC(("wl%d: %s: Forcing a new roam scan becasue we found no APs "
		          "from our partial scan results list\n",
		          wlc->pub->unit, __FUNCTION__));
		roam->cache_valid = FALSE;
		roam->active = FALSE;
		roam->scan_block = 0;
	}
#ifdef WLRCC
	if (WLRCC_ENAB(wlc->pub)) {
		if (roam->roam_scan_started) {
			roam->roam_scan_started = FALSE;
			if (wlc->as_info->join_targets_last == 0) {
				if (roam->fullscan_count == roam->nfullscans) {
					/* first roam scan failed to find a join target,
					 * then, retry the full roam scan immediately
					 */
					cfg->roam->scan_block = 0;
					roam->fullscan_count = 1;
					/* force a full scan */
					roam->time_since_upd = roam->fullscan_period;
					roam->cache_valid = FALSE;
					roam->rcc_valid = FALSE;
				}
			}
		} else {
			if (for_roam && roam_active && (cache_valid == FALSE)) {
				roam->roam_fullscan = FALSE;
				if (roam->rcc_mode != RCC_MODE_FORCE) {
					/* full roaming scan done, update the roam channel cache */
					rcc_update_from_join_targets(roam,
					wlc->as_info->join_targets);
				}
			}
		}
	}
#endif /* WLRCC */

	/* Clear the flag */
	as->flags &= ~AS_F_CACHED_ROAM;

	if (wlc->as_info->join_targets_last > 0) {
		wlc_ap_mute(wlc, TRUE, cfg, -1);
		if (for_roam && (!WLFMC_ENAB(wlc->pub) || (WLFMC_ENAB(wlc->pub) &&
			cfg->roam->reason != WLC_E_REASON_INITIAL_ASSOC)))
			wlc_roam_set_env(cfg, wlc->as_info->join_targets->count);
		wlc_join_attempt(cfg);
	} else if (target_bss->infra != 1 &&
	           !wlc->IBSS_join_only &&
	           target_bss->SSID_len > 0) {
		/* no join targets */
		/* Create an IBSS if we are IBSS or AutoUnknown mode,
		 * and we had a non-Null SSID
		 */
		WL_ASSOC(("wl%d: JOIN: creating an IBSS with SSID \"%s\"\n",
			WLCWLUNIT(wlc), ssidstr));

		/* zero out BSSID in order for _wlc_join_start_ibss() to assign a random one */
		bzero(&cfg->BSSID, ETHER_ADDR_LEN);

		if (_wlc_join_start_ibss(wlc, cfg) != BCME_OK) {
			wlc_set_ssid_complete(cfg, WLC_E_STATUS_FAIL, NULL,
				DOT11_BSSTYPE_INDEPENDENT);
		}
	} else {
		/* no join targets */
		/* see if the target channel information could be cached, if caching is desired */
		if (for_roam && roam->active && roam->partialscan_period)
			wlc_roamscan_complete(cfg);

		/* Retry the scan if we used the scan cache for the initial attempt,
		 * Otherwise, report the failure
		 */
		if (!for_roam && ASSOC_CACHE_ASSIST_ENAB(wlc) &&
		    (as->flags & AS_F_CACHED_CHANNELS))
			wlc_assoc_cache_fail(cfg);
#ifdef WL_ASSOC_RECREATE
		else if (as->flags & AS_F_SPEEDY_RECREATE)
			wlc_speedy_recreate_fail(cfg);
#endif /* WL_ASSOC_RECREATE */
		else {
			WL_ASSOC(("scan found no target to send WLC_E_DISASSOC\n"));

#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg) &&
				for_roam &&
				(as->type == AS_ROAM)) {

				/* indicate DISASSOC due to unreachability */
				wlc_handle_ap_lost(wlc, cfg);

				WL_ASSOC(("wl%d %s: terminating roaming for p2p\n",
					WLCWLUNIT(wlc), __FUNCTION__));
				wlc_assoc_done(cfg, WLC_E_STATUS_NO_NETWORKS);
				wlc_bsscfg_disable(wlc, cfg);
				goto exit;
			}
#endif /* WLP2P */
#ifdef EXT_STA
			if (WLEXTSTA_ENAB(wlc->pub) &&
				for_roam &&
				(as->type == AS_ROAM) &&
				(!roam->bcns_lost) &&
				(roam->reason == WLC_E_REASON_BCNS_LOST)) {

				/* indicate DISASSOC due to unreachability */
				wlc_handle_ap_lost(wlc, cfg);

				/* reset join_targets for new attempt */
				wlc_bss_list_free(wlc, wlc->as_info->join_targets);
				WL_ASSOC(("start full roam scan...\n"));
				wlc_assoc_scan_prep(cfg, NULL, NULL, NULL, 0);
				goto exit;
			}
#endif /* EXT_STA */

#ifdef SLAVE_RADAR
			if (WL11H_STA_ENAB(wlc) &&
				(roam->reason == WLC_E_REASON_RADAR_DETECTED) &&
				(as->state != AS_DFS_CAC_START))
			{
				wlc_bss_list_free(wlc, wlc->as_info->join_targets);
				WL_ASSOC(("radar detected, so roam untill we find the AP...\n"));
				wlc_assoc_scan_prep(cfg, NULL, NULL, NULL, 0);
				goto exit;
			}
#endif /* SLAVE_RADAR */
			wlc_assoc_done(cfg, WLC_E_STATUS_NO_NETWORKS);
#ifdef WLEXTLOG
			if (!for_roam &&
			    target_bss->SSID_len != DOT11_MAX_SSID_LEN) {
				WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_NO_NETWORKS_ID,
				           WL_LOG_LEVEL_ERR, 0, 0, NULL);
			}
#endif // endif
		}
	}
exit:
#ifdef WLLED
	wlc_led_event(wlc->ledh);
#endif // endif
	/* This return is explicitly added so the "led:" label has something that follows. */

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	if (ssidbuf != NULL)
		MFREE(wlc->osh, (void *)ssidbuf, SSID_FMT_BUF_LEN);
#endif // endif
	return;
} /* wlc_assoc_scan_complete */

/**
 * cases to roam
 * - wlc_watchdog() found beacon lost for too long
 * - wlc_recvdata found low RSSI in received frames
 * - AP DEAUTH/DISASSOC sta
 * - CCX initiated roam - directed roam, TSPEC rejection
 * it will end up in wlc_assoc_scan_complete and WLC_E_ROAM
 */
int
wlc_roam_scan(wlc_bsscfg_t *cfg, uint reason, chanspec_t *list, uint32 channum)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
#ifdef BCMCCX
	uint32 tsf_h;
#endif // endif
	int ret;

	if (roam->off) {
		WL_INFORM(("wl%d: roam scan is disabled\n", wlc->pub->unit));
		return BCME_EPERM;
	}

	if (!cfg->associated) {
#ifdef SLAVE_RADAR
		if (reason != WLC_E_REASON_RADAR_DETECTED)
#endif // endif
			return BCME_NOTASSOCIATED;
	}

	if ((ret = wlc_mac_request_entry(wlc, cfg, WLC_ACTION_ROAM)) != BCME_OK) {
		WL_ERROR(("wl%d.%d: ROAM request failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		return ret;
	}

	if (roam->original_reason == WLC_E_REASON_INITIAL_ASSOC) {
		WL_ASSOC(("wl%d %s: update original roam reason to %u\n", WLCWLUNIT(wlc),
			__FUNCTION__, reason));
		roam->original_reason = reason;
	}

	wlc_assoc_init(cfg, AS_ROAM);
	roam->reason = reason;

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub)) {
		WL_ASSOC(("wl%d: %s: ROAM: roam_reason 0x%x fast_roam 0x%x\n",
			wlc->pub->unit, __FUNCTION__, reason, wlc->ccx->fast_roam));
		WL_ROAM(("ROAM: Start roamscan reason %d fast_roam %d\n",
			reason, wlc->ccx->fast_roam));

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		wlc_print_roam_status(cfg, roam->reason, TRUE);
#endif // endif
		/* for CCXv4 - S56 roaming delay measurement */
		if (reason != WLC_E_REASON_FAST_ROAM_FAILED)
			wlc_read_tsf(wlc, &wlc->ccx->roaming_start_time, &tsf_h);
	} else {
		WL_ROAM(("ROAM: Start roamscan reason %d CCX disable\n", reason));
	}
#else
	WL_ROAM(("ROAM: Start roamscan reason %d\n", reason));
#endif /* BCMCCX */

#ifdef WL_EVENT_LOG_COMPILE
	{
		wl_event_log_tlv_hdr_t tlv_log = {{0, 0}};

		tlv_log.tag = TRACE_TAG_VENDOR_SPECIFIC;
		tlv_log.length = sizeof(uint);

		WL_EVENT_LOG(EVENT_LOG_TAG_TRACE_WL_INFO, TRACE_ROAM_SCAN_STARTED,
			tlv_log.t, reason);
	}
#endif /* WL_EVENT_LOG_COMPILE */

	/* start the re-association process by starting the association scan */

	/* This relies on COPIES of the scan parameters being made
	 * somewhere below in the call chain.
	 * If not there can be trouble!
	 */
	if (cfg->roam_scan_params && cfg->roam_scan_params->active_time) {
		wl_join_scan_params_t jsc;

		bzero(&jsc, sizeof(jsc));

		jsc.scan_type = cfg->roam_scan_params->scan_type;
		jsc.nprobes = cfg->roam_scan_params->nprobes;
		jsc.active_time = cfg->roam_scan_params->active_time;
		jsc.passive_time = cfg->roam_scan_params->passive_time;
		jsc.home_time = cfg->roam_scan_params->home_time;

		wlc_assoc_scan_prep(cfg, &jsc, NULL,
			cfg->roam_scan_params->channel_list,
			cfg->roam_scan_params->channel_num);
	} else {
		/* original */
		wlc_assoc_scan_prep(cfg, NULL, NULL, list, channum);
	}

	return 0;
}

/*
 * check encryption settings: return TRUE if security mismatch
 *
 * common_wsec = common ciphers between AP/IBSS and STA
 *
 * if (WSEC_AES_ENABLED(common_wsec)) {
 *    if (((AP ucast is None) && (AP mcast is AES)) ||
 *         (!(AP mcast is None) && (AP ucast includes AES))) {
 *         keep();
 *     }
 * } else if (WSEC_TKIP_ENABLED(common_wsec)) {
 *     if (((AP ucast is None) && (AP mcast is TKIP)) ||
 *         (!(AP mcast is None) && (AP ucast includes TKIP))) {
 *         keep();
 *     }
 * } else if (WSEC_WEP_ENABLED(common_wsec)) {
 *     if ((AP ucast is None) && (AP mcast is WEP)) {
 *         keep();
 *     }
 * }
 * prune();
 *
 * TKIP countermeasures:
 * - prune non-WPA
 * - prune WPA with encryption <= TKIP
 */
static bool
wlc_join_wsec_filter(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	struct rsn_parms *rsn;
	bool prune = TRUE;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	bool akm_match = TRUE;
	uint32 ap_wsec; /* Ciphers supported by AP/IBSS in bitmap format */
	uint32 common_wsec; /* Common ciphers supported between AP/IBSS and STA */

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return FALSE;

#ifdef WLFBT
	/* Check AKM suite in target AP against the one STA is currently configured for */
	if (BSSCFG_IS_FBT(cfg))
		akm_match = wlc_fbt_akm_match(wlc->fbt, cfg, bi);
#endif /* WLFBT */

	/* check authentication mode */
	if (bcmwpa_is_wpa_auth(cfg->WPA_auth) && (bi->wpa.flags & RSN_FLAGS_SUPPORTED) &&
		/* AKM count zero is not acceptable */
		(bi->wpa.acount != 0)) {
		rsn = &(bi->wpa);
	}
	/* Prune BSSID when STA is moving to a different security type */
	else if ((bcmwpa_is_rsn_auth(cfg->WPA_auth) && (bi->wpa2.flags & RSN_FLAGS_SUPPORTED) &&
		/* AKM count zero is not acceptable */
		(bi->wpa2.acount != 0)) && akm_match) {
		rsn = &(bi->wpa2);
	}
#ifdef BCMWAPI_WAI
	else if (IS_WAPI_AUTH(cfg->WPA_auth) && (bi->wapi.flags & RSN_FLAGS_SUPPORTED))
		rsn = &(bi->wapi);
#endif /* BCMWAPI_WAI */
	else
	{
		WL_ASSOC(("wl%d: JOIN: BSSID %s pruned for security reasons\n",
			WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
		wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
			WLC_E_RSN_MISMATCH, 0, 0, 0);
		return prune;
	}

#ifdef MFP
	/* do MFP checking before more complicated algorithm checks below */
	if ((BSSCFG_IS_MFP_REQUIRED(cfg)) && !(bi->wpa2.flags & RSN_FLAGS_MFPC)) {
		/* We require MFP , but peer is not MFP capable */
		WL_ASSOC(("wl%d: %s: BSSID %s pruned, MFP required but peer does"
			" not advertise MFP\n",
			WLCWLUNIT(wlc), __FUNCTION__, bcm_ether_ntoa(&bi->BSSID, eabuf)));
		return prune;
	}
	if ((bi->wpa2.flags & RSN_FLAGS_MFPR) && !(BSSCFG_IS_MFP_CAPABLE(cfg))) {
		/* Peer requires MFP , but we don't have MFP enabled */
		WL_ASSOC(("wl%d: %s: BSSID %s pruned, peer requires MFP but MFP not"
			" enabled locally\n",
			WLCWLUNIT(wlc), __FUNCTION__, bcm_ether_ntoa(&bi->BSSID, eabuf)));
		return prune;
	}
#endif /* MFP */

	/* Get the AP/IBSS RSN (ciphers) to bitmap wsec format */
	ap_wsec = wlc_convert_rsn_to_wsec_bitmap(rsn);

	/* Find the common ciphers between AP/IBSS and STA */
	common_wsec = ap_wsec & cfg->wsec;

	WL_ASSOC(("wl%d: %s: AP/IBSS wsec %04x, STA wsec %04x, Common wsec %04x \n",
		WLCWLUNIT(wlc), __FUNCTION__, ap_wsec, cfg->wsec, common_wsec));

	/* Clear TKIP Countermeasures Block Timer, if timestamped earlier
	 * than WPA_TKIP_CM_BLOCK (=60 seconds).
	 */

	if (WSEC_AES_ENABLED(common_wsec)) {

		if ((UCAST_NONE(rsn) && MCAST_AES(rsn)) || (MCAST_AES(rsn) && UCAST_AES(rsn)) ||
			(!TK_CM_BLOCKED(wlc, cfg) && (!MCAST_NONE(rsn) && UCAST_AES(rsn))))
			prune = FALSE;
		else {
			WL_ASSOC(("wl%d: JOIN: BSSID %s AES: no AES support ot TKIP cm bt \n",
				WLCWLUNIT(wlc),	bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
		}

	} else if (WSEC_TKIP_ENABLED(common_wsec)) {

		if (!TK_CM_BLOCKED(wlc, cfg) && ((UCAST_NONE(rsn) && MCAST_TKIP(rsn)) ||
			(!MCAST_NONE(rsn) && UCAST_TKIP(rsn))))
			prune = FALSE;
		else {
			WL_ASSOC(("wl%d: JOIN: BSSID %s TKIP: no TKIP support or TKIP cm bt\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
		}

	} else
#ifdef BCMCCX
	if (WSEC_WEP_ENABLED(common_wsec)) {
		if (!TK_CM_BLOCKED(wlc, cfg) && (cfg->WPA_auth == WPA_AUTH_CCKM) &&
			((UCAST_NONE(rsn) && MCAST_CKIP(rsn)) ||
			(!MCAST_NONE(rsn) && UCAST_CKIP(rsn)) ||
			(UCAST_NONE(rsn) && MCAST_WEP(rsn)) ||
			(!MCAST_NONE(rsn) && UCAST_WEP(rsn))))
			prune = FALSE;
		else {
			WL_ASSOC(("wl%d: JOIN: BSSID %s WEP: no WEP support or TKIP cm bt\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
		}
	} else
#endif /* BCMCCX */
#if defined(BCMWAPI_WAI)
	if (WSEC_SMS4_ENABLED(common_wsec)) {
		if (WAPI_RSN_UCAST_LOOKUP(rsn) && WAPI_RSN_MCAST_LOOKUP(rsn))
			prune = FALSE;
		else {
			WL_ASSOC(("wl%d: JOIN: BSSID %s SMS4: no SMS4 support\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
		}
	} else
#endif /* BCMWAPI_WAI */
	{
		if (!TK_CM_BLOCKED(wlc, cfg) && (UCAST_NONE(rsn) && MCAST_WEP(rsn)))
			prune = FALSE;
		else {
			WL_ASSOC(("wl%d: JOIN: BSSID %s no WEP support or TKIP cm\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
		}
	}

	if (prune) {
		WL_ASSOC(("wl%d: JOIN: BSSID %s pruned for security reasons\n",
			WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
	}

	return prune;
} /* wlc_join_wsec_filter */

/** check the channel against the channels in chanspecs list in assoc params */
static bool
wlc_join_chanspec_filter(wlc_bsscfg_t *bsscfg, chanspec_t chanspec)
{
	wl_join_assoc_params_t *assoc_params = wlc_bsscfg_assoc_params(bsscfg);
	int i, num;

	/* no restrictions */
	if (assoc_params == NULL)
		return TRUE;

	num = assoc_params->chanspec_num;

	/* if specified pairs, check only the current chanspec */
	if (assoc_params->bssid_cnt)
		return (chanspec == assoc_params->chanspec_list[num]);

	if (assoc_params != NULL && assoc_params->chanspec_num > 0) {
		for (i = 0; i < assoc_params->chanspec_num; i ++)
			if (chanspec == assoc_params->chanspec_list[i])
				return TRUE;
		return FALSE;
	}

	return TRUE;
}

#if defined(WLMSG_ROAM) && !defined(WLMSG_ASSOC)
#undef WL_ASSOC
#define WL_ASSOC(args) printf args
#endif /* WLMSG_ROAM && !WLMSG_ASSOC */

#ifdef SLAVE_RADAR
/*
 * This function checks, if STA is going to associate in a DFS channel,
 * should the DFS state machine begin CAC or ISM.
 * For a non-DFS channel, it would simply continue with join.
 */
static void
wlc_join_chkdfs_cac_ism(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *bi;

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	as->bss_retries = 0;

	bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];

	if (bi == NULL) {
		WL_ASSOC(("wl%d: JOIN: BSS info == NULL skipping, join_targets_last[%d]\n",
		WLCWLUNIT(wlc), wlc->as_info->join_targets_last));
		return;
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	WL_ASSOC(("wl%d: JOIN: checking CAC or ISM  BSSID: %s\n", WLCWLUNIT(wlc),
	            bcm_ether_ntoa(&bi->BSSID, eabuf)));
#endif /* BCMDBG || WLMSG_ASSOC */
	if (WL11H_STA_ENAB(wlc) && wlc_radar_chanspec(wlc->cmi, bi->chanspec)) {
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, bi->chanspec);
			wlc_enable_mac(wlc);
			phy_radar_detect_enable((phy_info_t *)WLC_PI(wlc), TRUE);

			/* Check if we need to begin pre ISM CAC */
			if (!(wlc_cac_is_clr_chanspec(wlc->dfs, bi->chanspec)))
			{
				/* Begin CAC */
				WL_ASSOC(("%s: Setting AS_DFS_CAC_START\n", __FUNCTION__));
				wlc_assoc_timer_del(wlc, cfg);
				wl_add_timer(wlc->wl, as->timer,
					(wlc_dfs_get_cactime_ms(wlc->dfs) + DELAY_10MS), FALSE);
				wlc_assoc_change_state(cfg, AS_DFS_CAC_START);
			}
			else {
				/* Begin ISM */
				WL_ASSOC(("%s: Setting AS_DFS_ISM_INIT\n", __FUNCTION__));
				wlc_assoc_timer_del(wlc, cfg);
				wl_add_timer(wlc->wl, as->timer,
					(WECA_ASSOC_TIMEOUT + DELAY_10MS), FALSE);
				wlc_assoc_change_state(cfg, AS_DFS_ISM_INIT);
			}
	}
	else
	{
		bool from_radar = (wlc_radar_chanspec(wlc->cmi,
				cfg->current_bss->chanspec) == TRUE);
		bool to_radar = wlc_radar_chanspec(wlc->cmi, bi->chanspec);

		if (wlc_dfs_get_radar(wlc->dfs) && (from_radar && !to_radar)) {
			cfg->pm->PMmodeChangeDisabled = FALSE;
			wlc_set_pm_mode(wlc, cfg->pm->PM_oldvalue, cfg);
			wlc->mpc = TRUE;
			wlc_radio_mpc_upd(wlc);
		}
		wlc_assoc_change_state(cfg, AS_JOIN_START);
		wlc_join_bss_start(cfg);
	}
}
#endif /* SLAVE_RADAR */

/**
 * scan finished with valid join_targets
 * loop through join targets and run all prune conditions
 *    if there is a one surviving, start join process.
 *    this function will be called if the join fails so next target(s) will be tried.
 */
static void
wlc_join_attempt(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	wlc_bss_info_t *bi;
	uint i;
	wlcband_t *target_band;
#if defined(BCMDBG) || defined(WLMSG_ASSOC) || defined(WLMSG_ROAM)
	char ssidbuf[SSID_FMT_BUF_LEN];
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	uint32 wsec, WPA_auth;
#ifdef BCMCCX
	uint status;
#endif // endif
	int addr_match;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	/* keep core awake until join finishes, as->state != AS_IDLE */
	ASSERT(STAY_AWAKE(wlc));
	wlc_set_wake_ctrl(wlc);

	WL_ASSOC(("wl%d.%d %s Join to \"%s\"\n", wlc->pub->unit, cfg->_idx, __FUNCTION__,
		target_bss->SSID));
	wlc_assoc_timer_del(wlc, cfg);

	/* walk the list until there is a valid join target */
	for (; wlc->as_info->join_targets_last > 0; wlc->as_info->join_targets_last--) {
		wlc_rateset_t rateset;
		chanspec_t chanspec;
		wlc_bsscfg_t *bc = NULL;

		bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last - 1];
		ASSERT(bi != NULL);
		if (bi == NULL) {
			/* Validity check */
			WL_ASSOC(("wl%d: JOIN: BSS info == NULL skipping, join_targets_last[%d]\n",
				WLCWLUNIT(wlc), wlc->as_info->join_targets_last));
			continue;
		}

		target_band = wlc->bandstate[CHSPEC_WLCBANDUNIT(bi->chanspec)];
		WL_ROAM(("JOIN: checking [%d] %s\n", wlc->as_info->join_targets_last - 1,
			bcm_ether_ntoa(&bi->BSSID, eabuf)));
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
#endif // endif

		/* prune invalid chanspec based on STA capibility */
		if ((chanspec = wlc_max_valid_bw_chanspec(wlc, target_band, cfg, bi->chanspec))
		    == INVCHANSPEC)
		{
			WL_ASSOC(("wl%d: JOIN: Skipping invalid chanspec(0x%x):",
				WLCWLUNIT(wlc), bi->chanspec));
			continue;
		}

		/* validate BSS channel */
		if (!wlc_join_chanspec_filter(cfg,
			CH20MHZ_CHSPEC(wf_chspec_ctlchan(bi->chanspec)))) {
			WL_ASSOC(("wl%d: JOIN: Skipping BSS %s, mismatch chanspec %x\n",
			          WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf),
			          bi->chanspec));
			continue;
		}

		/* mSTA: Check here to make sure we don't end up with multiple bsscfgs
		 * associated to the same BSS. Skip this when PSTA mode is enabled.
		 */
		if (!PSTA_ENAB(wlc->pub)) {
			FOREACH_AS_STA(wlc, i, bc) {
				if (bc != cfg &&
				    bcmp(&bi->BSSID, &bc->BSSID, ETHER_ADDR_LEN) == 0)
					break;
				bc = NULL;
			}
			if (bc != NULL) {
				WL_ASSOC(("wl%d.%d: JOIN: Skipping BSS %s, "
				          "associated by bsscfg %d\n",
				          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
				          bcm_ether_ntoa(&bi->BSSID, eabuf),
				          WLC_BSSCFG_IDX(bc)));
				continue;
			}
		}

		/* derive WPA config if WPA auto config is on */
		if (WLCAUTOWPA(cfg)) {
			ASSERT(bi->wpacfg < join_pref->wpas);
			/* auth */
			cfg->auth = DOT11_OPEN_SYSTEM;
			/* WPA_auth */
#ifdef BCMCCX
			bcmwpa_akm2WPAauth(join_pref->wpa[bi->wpacfg].akm, &WPA_auth,
				!(bi->flags & WLC_BSS_WPA2));
#else
			bcmwpa_akm2WPAauth(join_pref->wpa[bi->wpacfg].akm, &WPA_auth, FALSE);
#endif // endif
			cfg->WPA_auth = WPA_auth;

			/* wsec - unicast */
			bcmwpa_cipher2wsec(join_pref->wpa[bi->wpacfg].ucipher, &wsec);
			/*
			 * use multicast cipher only when unicast cipher is none, otherwise
			 * the next block (OID_802_11_ENCRYPTION_STATUS related) takes care of it
			 */
			if (!wsec) {
				uint8 mcs[WPA_SUITE_LEN];
				bcopy(join_pref->wpa[bi->wpacfg].mcipher, mcs, WPA_SUITE_LEN);
				if (!bcmp(mcs, WL_WPA_ACP_MCS_ANY, WPA_SUITE_LEN)) {
#ifdef BCMCCX
					if (IS_CCX_CIPHER(bi->mcipher)) {
						if (!CCX_ENAB(wlc->pub))
							continue;
						bcopy(CISCO_AIRONET_OUI, mcs, DOT11_OUI_LEN);
						mcs[DOT11_OUI_LEN] = bi->mcipher - CISCO_BASE;
					}
					else
#endif /* BCMCCX */
						mcs[DOT11_OUI_LEN] = bi->mcipher;
				}
				if (!bcmwpa_cipher2wsec(mcs, &wsec)) {
					WL_ASSOC(("JOIN: Skip BSS %s WPA cfg %d, cipher2wsec"
						" failed\n",
						bcm_ether_ntoa(&bi->BSSID, eabuf),
						bi->wpacfg));
					wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID,
						0, WLC_E_PRUNE_CIPHER_NA,
						CHSPEC_CHANNEL(bi->chanspec), 0, 0);
					continue;
				}
			}
#ifdef BCMSUP_PSK
			if (SUP_ENAB(wlc->pub) &&
				BSS_SUP_ENAB_WPA(wlc->idsup, cfg)) {
				if (WSEC_AES_ENABLED(wsec))
					wsec |= TKIP_ENABLED;
				if (WSEC_TKIP_ENABLED(wsec))
					wsec |= WEP_ENABLED;
				/* merge rest flags */
#ifdef BCMWAPI_WPI
				wsec |= cfg->wsec & ~(AES_ENABLED | TKIP_ENABLED |
				                      WEP_ENABLED | SMS4_ENABLED);
#else
				wsec |= cfg->wsec & ~(AES_ENABLED | TKIP_ENABLED |
				                      WEP_ENABLED);
#endif /* BCMWAPI_WPI */
			}
#endif /* BCMSUP_PSK */
			wlc_iovar_op(wlc, "wsec", NULL, 0, &wsec, sizeof(wsec),
				IOV_SET, cfg->wlcif);
			WL_ASSOC(("wl%d: JOIN: BSS %s wpa cfg %d WPA_auth 0x%x wsec 0x%x\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf),
				bi->wpacfg, cfg->WPA_auth, cfg->wsec));
			wlc_bss_mac_event(wlc, cfg, WLC_E_AUTOAUTH, &bi->BSSID,
				WLC_E_STATUS_ATTEMPT, 0, bi->wpacfg, 0, 0);
		}
#if defined(BCMCCX)
		else if (CCX_ENAB(wlc->pub) && (cfg->WPA_auth == WPA_AUTH_CCKM) &&
		         (bi->flags & WLC_BSS_WPA2)) {
			for (i = 0; i < bi->wpa2.acount; i++) {
				/* CCKM uses RSN AKM None value (0) */
				if (bi->wpa2.auth[i] == RSN_AKM_NONE) {
					cfg->WPA_auth = WPA2_AUTH_CCKM;
					break;
				}
			}
		}
#endif /* BCMCCX */

		/* check Privacy (encryption) in target BSS */
		/*
		 * WiFi: A STA with WEP off should never attempt to associate with
		 * an AP that has WEP on
		 */
		if ((bi->capability & DOT11_CAP_PRIVACY) && !WSEC_ENABLED(cfg->wsec)) {
			if (cfg->is_WPS_enrollee) {
				WL_ASSOC(("wl%d: JOIN: Assuming join to BSSID %s is for WPS "
				          " credentials, so allowing unencrypted EAPOL frames\n",
				          WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			} else {
				WL_ASSOC(("wl%d: JOIN: Skipping BSSID %s, encryption mandatory "
				          "in BSS, but encryption off for us.\n", WLCWLUNIT(wlc),
				          bcm_ether_ntoa(&bi->BSSID, eabuf)));
				wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				                  WLC_E_PRUNE_ENCR_MISMATCH,
				                  CHSPEC_CHANNEL(bi->chanspec), 0, 0);
				continue;
			}
		}
#ifdef BCMCCX
		/*
		 * CCX Mixed cell support: When cfg->wsec_restrict is TRUE, a STA
		 * with WEP on should not attempt to associate with an AP that has
		 * WEP off
		 */
		if (CCX_ENAB(wlc->pub) && !(bi->capability & DOT11_CAP_PRIVACY) &&
			cfg->wsec_restrict && WSEC_ENABLED(cfg->wsec)) {
			WL_ASSOC(("wl%d: JOIN: Skipping BSSID %s, encryption not "
			      "mandatory in BSS, but encryption on and "
			      "aExcludeUnencrypted set for us.\n",
			       WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_ENCR_MISMATCH,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
#endif /* BCMCCX */
		/* skip broadcast bssid */
		if (ETHER_ISBCAST(bi->BSSID.octet)) {
			WL_ASSOC(("wl%d: JOIN: Skipping BSS with broadcast BSSID\n",
				WLCWLUNIT(wlc)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_BCAST_BSSID,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
		/* prune join_targets based on allow/deny list */
		addr_match = wlc_macfltr_addr_match(wlc->macfltr, cfg, &bi->BSSID);
		if (addr_match == WLC_MACFLTR_ADDR_DENY) {
			WL_ASSOC(("wl%d: JOIN: pruning BSSID %s because it "
			          "was on the MAC Deny list\n", WLCWLUNIT(wlc),
			          bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID,
			                  0, WLC_E_PRUNE_MAC_DENY,
			                  CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
		else if (addr_match == WLC_MACFLTR_ADDR_NOT_ALLOW) {
			WL_ASSOC(("wl%d: JOIN: pruning BSSID %s because it "
			          "was not on the MAC Allow list\n", WLCWLUNIT(wlc),
			          bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID,
			                  0, WLC_E_PRUNE_MAC_NA,
			                  CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}

#ifdef EXT_STA
		if (WLEXTSTA_ENAB(wlc->pub)) {
			/* prune if target is not the desired BSSID */
			if (wlc->allow_mode == 1) {
				if (bcmp(&bi->BSSID.octet, &wlc->desired_BSSID.octet,
				         ETHER_ADDR_LEN)) {
					WL_ASSOC(("wl%d: JOIN: pruning undesirable BSSID\n",
					          WLCWLUNIT(wlc)));
					continue;
				}
			} else if (wlc->allow_mode == 2) {
				WL_ASSOC(("wl%d: JOIN: pruning BSSID because none are allowed\n",
				          WLCWLUNIT(wlc)));
				continue;
			}
		}
#endif /* EXT_STA */

		/* prune if we are in strict SpectrumManagement mode, the AP is not advertising
		 * support, and the locale requires 802.11h SpectrumManagement
		 */
		if (WL11H_ENAB(wlc) && (wlc_11h_get_spect(wlc->m11h) == SPECT_MNGMT_STRICT_11H) &&
			bi->infra && (bi->capability & DOT11_CAP_SPECTRUM) == 0 &&
		    (wlc_channel_locale_flags_in_band(wlc->cmi, target_band->bandunit) &
		     WLC_DFS_TPC)) {
			WL_ASSOC(("wl%d: JOIN: pruning AP %s (SSID \"%s\", chanspec %s). "
			          "Current locale \"%s\" requires spectrum management "
			          "but AP does not have SpectrumManagement.\n",
			          WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf), ssidbuf,
			          wf_chspec_ntoa_ex(bi->chanspec, chanbuf),
			          wlc_channel_country_abbrev(wlc->cmi)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_SPCT_MGMT,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}

		if (WL11H_ENAB(wlc) && bi->infra &&
		    wlc_csa_quiet_mode(wlc->csa, (uint8 *)bi->bcn_prb, bi->bcn_prb_len)) {
			WL_ASSOC(("JOIN: pruning AP %s (SSID \"%s\", chanspec %s). "
				"AP is CSA quiet period.\n",
				  bcm_ether_ntoa(&bi->BSSID, eabuf), ssidbuf,
				  wf_chspec_ntoa_ex(bi->chanspec, chanbuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_SPCT_MGMT,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}

		/* prune if in 802.11h SpectrumManagement mode and the IBSS is on radar channel */
		if ((WL11H_ENAB(wlc)) && (bi->infra == 0) &&
			wlc_radar_chanspec(wlc->cmi, bi->chanspec)) {
			WL_ASSOC(("wl%d: JOIN: pruning IBSS \"%s\" chanspec %s since "
			      "we are in 802.11h mode and IBSS is on a radar channel in "
			      "locale \"%s\"\n", WLCWLUNIT(wlc),
			      ssidbuf, wf_chspec_ntoa_ex(bi->chanspec, chanbuf),
			      wlc_channel_country_abbrev(wlc->cmi)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_RADAR,
			        CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
		/* prune if the IBSS is on a restricted channel */
		if ((bi->infra == 0) && wlc_restricted_chanspec(wlc->cmi, bi->chanspec)) {
			WL_ASSOC(("wl%d: JOIN: pruning IBSS \"%s\" chanspec %s since is is "
			          "on a restricted channel in locale \"%s\"\n",
			          WLCWLUNIT(wlc), ssidbuf,
			          wf_chspec_ntoa_ex(bi->chanspec, chanbuf),
			          wlc_channel_country_abbrev(wlc->cmi)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_REG_PASSV,
				CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub) && bi->aironet_ie_rx &&
			(status = wlc_ccx_prune(wlc->ccx, bi))) {
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0, status,
			CHSPEC_CHANNEL(bi->chanspec), 0, 0);
			continue;
		}
#endif /* BCMCCX */

		/* prune join targets based on security settings */
		if (cfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(cfg->wsec)) {
			if (wlc_join_wsec_filter(wlc, cfg, bi))
				continue;
		}

		/* prune based on rateset, bail out if no common rates with the BSS/IBSS
		 *  copy rateset to preserve original (CCK only if LegacyB)
		 *  Adjust Hardware rateset for target channel based on the channel bandwidth
		 *  filter-out unsupported rates.
		 */
		if (BAND_2G(target_band->bandtype) && (target_band->gmode == GMODE_LEGACY_B))
			wlc_rateset_filter(&bi->rateset /* src */, &rateset /* dst */,
				FALSE, WLC_RATES_CCK, RATE_MASK_FULL, 0);
		else
			wlc_rateset_filter(&bi->rateset, &rateset, FALSE, WLC_RATES_CCK_OFDM,
			                   RATE_MASK_FULL, wlc_get_mcsallow(wlc, cfg));
#ifdef WL11N
		wlc_rateset_bw_mcs_filter(&target_band->hw_rateset,
			WL_BW_CAP_40MHZ(wlc->band->bw_cap)?CHSPEC_WLC_BW(bi->chanspec):0);
#endif // endif
		if (!wlc_rate_hwrs_filter_sort_validate(&rateset /* [in+out] */,
			&target_band->hw_rateset /* [in] */,
			FALSE, wlc->stf->op_txstreams)) {
			WL_ASSOC(("wl%d: JOIN: BSSID %s pruned because we don't have any rates "
			          "in common with the BSS\n", WLCWLUNIT(wlc),
			          bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_NO_COMMON_RATES, 0, 0, 0);
			continue;
		}

		/* pruned if device don't supported HT PHY membership */
		if (rateset.htphy_membership != wlc_ht_get_phy_membership(wlc->hti)) {
			WL_ASSOC(("wl%d: JOIN: BSSID %s pruned because HT PHY support don't match",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			continue;
		}

		/* skip any IBSS having basic rates which we do not support */
		if (bi->infra == 0 &&
		    !wlc_join_basicrate_supported(wlc, &bi->rateset,
			CHSPEC2WLC_BAND(bi->chanspec))) {
			WL_ASSOC(("wl%d: JOIN: BSSID %s pruned because we do not support all "
			      "Basic Rates of the BSS\n", WLCWLUNIT(wlc),
			      bcm_ether_ntoa(&bi->BSSID, eabuf)));
			wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				WLC_E_PRUNE_BASIC_RATES, 0, 0, 0);
			continue;
		}

		/* APSTA prune associated or peer STA2710s (no ambiguous roles) */
		if (APSTA_ENAB(wlc->pub) && (!WLEXTSTA_ENAB(wlc->pub) || AP_ACTIVE(wlc))) {
			struct scb *scb;
			scb = wlc_scbfind(wlc, cfg, &bi->BSSID);
			if ((scb == NULL) && (NBANDS(wlc) > 1))
				scb = wlc_scbfindband(wlc, cfg, &bi->BSSID, OTHERBANDUNIT(wlc));
			if (scb && SCB_ASSOCIATED(scb) && !(scb->flags & SCB_MYAP)) {
				WL_ASSOC(("wl%d: JOIN: BSSID %s pruned, it's a known STA\n",
				            WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
				wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
					WLC_E_PRUNE_KNOWN_STA, 0, 0, 0);
				continue;
			}
			if (scb && SCB_LEGACY_WDS(scb)) {
				WL_ASSOC(("wl%d: JOIN: BSSID %s pruned, it's a WDS peer\n",
				          WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
				wlc_bss_mac_event(wlc, cfg, WLC_E_PRUNE, &bi->BSSID, 0,
				              WLC_E_PRUNE_WDS_PEER, 0, 0, 0);
				continue;
			}
		}

		/* reach here means BSS passed all the pruning tests, so break the loop to join */
		break;
	}

	if (wlc->as_info->join_targets_last > 0) {
		as->bss_retries = 0;

		bi = wlc->as_info->join_targets->ptrs[--wlc->as_info->join_targets_last];
		WL_ASSOC(("wl%d: JOIN: attempting BSSID: %s\n", WLCWLUNIT(wlc),
		            bcm_ether_ntoa(&bi->BSSID, eabuf)));
#if defined(WLRSDB)
		if (RSDB_ENAB(wlc->pub)) {
			/* Try to see if a mode switch is required */
			if (wlc_rsdb_assoc_mode_change(&cfg, bi) == BCME_NOTREADY)
				return;
			/* Replace the current WLC pointer with the probably new wlc */
			wlc = cfg->wlc;
			join_pref = cfg->join_pref;
			target_bss = cfg->target_bss;
		}
#endif // endif

		if (as->type != AS_ASSOCIATION)
			wlc_bss_mac_event(wlc, cfg, WLC_E_ROAM_PREP, &bi->BSSID, 0, 0, 0, NULL, 0);

#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub)) {
			wlc->ccx->ccx_network = bi->aironet_ie_rx;
			WL_ASSOC(("wl%d: JOIN: join a %s network\n", WLCWLUNIT(wlc),
				wlc->ccx->ccx_network ? "CCX" : "non-CCX"));
		}
#endif /* BCMCCX */

#if defined(BCMCCX) && defined(CCX_SDK)
		/* IHV interface is blocked(by IHV_ENAB) for non-CCX network */
		if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx)) {
			wlc_assoc_change_state(cfg, AS_JOIN_START);
			/* indicate CCX ASSOC start */
			wlc_bss_mac_event(wlc, cfg, WLC_E_CCX_ASSOC_START, &bi->BSSID, 0,
				(as->type != AS_ASSOCIATION ? cfg->roam->reason : 0),
				WLC_DOT11_BSSTYPE(bi->infra), NULL, 0);
			/* waiting for CCX IEs from ihv */
			wlc_assoc_change_state(cfg, AS_WAIT_IE);
			wl_add_timer(wlc->wl, as->timer, WLC_IE_WAIT_TIMEOUT, 0);
			WL_ASSOC(("wl%d: JOIN: waiting for IHV CCX IEs to join BSSID %s\n",
				WLCWLUNIT(wlc), bcm_ether_ntoa(&bi->BSSID, eabuf)));
			/* remove stored ies if exists */
			if (wlc->ccx->ccx_ihv_ies) {
				MFREE(wlc->osh, wlc->ccx->ccx_ihv_ies, wlc->ccx->ccx_ihv_ies_len);
				wlc->ccx->ccx_ihv_ies = NULL;
				wlc->ccx->ccx_ihv_ies_len = 0;
			}
		} else
#endif /* BCMCCX && CCX_SDK */
		{
#ifdef BCMSUP_PSK
			/* XXX at this point we may not know the auth. type (WPA vs. PSK)
			 * until the BSS is chosen when we are doing auto WPA. So call
			 * wlc_sup_set_ssid() as long as we choose in-driver PSK supplicant
			 * and let wlc_sup_set_ssid() to decide if passhash algorithm is
			 * needed based on if the pass phrase is empty or not.
			 */
			if (SUP_ENAB(wlc->pub) &&
				(BSS_SUP_TYPE(wlc->idsup, cfg) == SUP_WPAPSK)) {
				switch (wlc_sup_set_ssid(wlc->idsup, cfg, bi->SSID, bi->SSID_len)) {
				case 1:
					/* defer association till psk passhash is done */
					wlc_assoc_change_state(cfg, AS_WAIT_PASSHASH);
					return;
				case 0:
					/* psk supplicant is config'd so continue */
					break;
				case -1:
				default:
					/* XXX what do we do when wlc_sup_set_ssid() fails?
					 * should we continue with the association progress?
					 */
					WL_ASSOC(("wl%d: wlc_sup_set_ssid failed, stop assoc\n",
					          wlc->pub->unit));
					wlc_assoc_done(cfg, WLC_E_STATUS_FAIL);
					return;
				}
			}
#endif /* BCMSUP_PSK */

#ifdef WLTDLS
			/* cleanup the TDLS peers for roam */
			if (TDLS_ENAB(cfg->wlc->pub) && (as->type == AS_ROAM))
				wlc_tdls_cleanup(cfg->wlc->tdls, cfg);
#endif /* WLTDLS */

#ifdef WLWNM
			if (WLWNM_ENAB(wlc->pub)) {
				bool delay_join;
				struct ether_addr *bssid;
				wlc_assoc_info_t *ai = wlc->as_info;
				/* Decided to join; send bss-trans-response here with accept */
				bssid = &ai->join_targets->ptrs[ai->join_targets_last]->BSSID;
				delay_join = wlc_wnm_bsstrans_roamscan_complete(wlc,
					DOT11_BSSTRANS_RESP_STATUS_ACCEPT, bssid);
				if (delay_join == 0) {
					wlc_join_bss_prep(cfg);
				}
			} else
#endif /* WLWNM */
				wlc_join_bss_prep(cfg);
		}
		return;
	}

	WL_ASSOC(("wl%d.%d: JOIN: no more join targets available\n",
	          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));

#ifdef WLP2P
	/* reset p2p assoc states only for non-roam cases */
	if (BSS_P2P_ENAB(wlc, cfg) && !cfg->associated)
		wlc_p2p_reset_bss(wlc->p2p, cfg);
#endif // endif

#ifdef WLMCHAN
	/*	In case this bsscfg is still associated and this was roam association trial,
	*	do not delete the mchan context as they need to be still used.
	*/
	if (MCHAN_ENAB(wlc->pub) && !cfg->associated) {
		WL_MCHAN(("wl%d.%d: %s: Delete chanctx for join failure\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		wlc_mchan_delete_bss_chan_context(wlc, cfg);
	}
#endif /* WLMCHAN */

	/* handle reassociation failure */
	if (cfg->associated) {
		wlc_roam_complete(cfg, WLC_E_STATUS_FAIL, NULL,
			WLC_DOT11_BSSTYPE(target_bss->infra));
	}
	/* handle association failure from a cache assisted assoc */
	else if (ASSOC_CACHE_ASSIST_ENAB(wlc) &&
	         (as->flags & (AS_F_CACHED_CHANNELS | AS_F_CACHED_RESULTS))) {
		/* on a cached assoc failure, retry after a full scan */
		wlc_assoc_cache_fail(cfg);
		return;
	}
	/* handle association failure */
	else {
		wlc_set_ssid_complete(cfg, WLC_E_STATUS_FAIL, NULL,
			WLC_DOT11_BSSTYPE(target_bss->infra));
	}

#if defined(BCMSUP_PSK)
	if (SUP_ENAB(wlc->pub)) {
		wlc_wpa_send_sup_status(wlc->idsup, cfg, WLC_E_SUP_OTHER);
	}
#endif /* defined(BCMSUP_PSK) */

	wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, 0);
	wlc_roam_release_flow_cntrl(cfg);
} /* wlc_join_attempt */

#if defined(WLMSG_ROAM) && !defined(WLMSG_ASSOC)
#undef WL_ASSOC
#define WL_ASSOC(args)
#endif // endif

void
wlc_join_bss_prep(wlc_bsscfg_t *cfg)
{
#if defined(CLIENT_CSA) && defined(WLDFS)
	bool dfs_cac_started = FALSE;
	if (!BSSCFG_AP(cfg) && (MAP_ENAB(cfg) || DWDS_ENAB(cfg) ||
#if defined(WET) || defined(WET_DONGLE)
		(WET_ENAB(cfg->wlc)) ||
#endif /* WET || WET_DONGLE */
		FALSE)) {
		dfs_cac_started = wlc_assoc_do_dfs_reentry_cac_if_required(cfg);
		if (dfs_cac_started) {
			/* join after cac finish */
			return;
		}

	}
#endif /* CLIENT_CSA && WL_DFS */
#ifdef SLAVE_RADAR
	wlc_assoc_t *as = cfg->assoc;
	if (as->state != AS_DFS_ISM_INIT)
		wlc_join_chkdfs_cac_ism(cfg);
	else
#endif	/* SLAVE_RADAR */
	{
		wlc_assoc_change_state(cfg, AS_JOIN_START);
		wlc_join_bss_start(cfg);
	}
}

static void
wlc_join_bss_start(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_txq_info_t *qi = cfg->wlcif->qi;
	struct pktq *q = WLC_GET_TXQ(qi);

#if defined(BCMCCX) && defined(CCX_SDK)
	if (as->state == AS_WAIT_IE) {
		wlc_assoc_timer_del(wlc, cfg);
	}
#endif /* BCMCCX && CCX_SDK */
#if defined(SLAVE_RADAR) || defined(CLIENT_CSA)
	/* Delete the assoc timer, if we are done with CAC */
	if (as->state == AS_DFS_ISM_INIT) {
		wlc_assoc_timer_del(wlc, cfg);
	}
#endif	/* SLAVE_RADAR || CLIENT_CSA */
	/* if roaming, make sure tx queue is drain out */
	if (as->type == AS_ROAM && wlc_bss_connected(cfg) &&
	    !wlc->block_datafifo &&
	    (!pktq_empty(q) || TXPKTPENDTOT(wlc) > 0)) {
		/* block tx path and roam to a new target AP only after all
		 * the pending tx packets sent out
		 */
		wlc_txflowcontrol_override(wlc, qi, ON, TXQ_STOP_FOR_PKT_DRAIN);
		wlc_assoc_change_state(cfg, AS_WAIT_TX_DRAIN);
		wl_add_timer(wlc->wl, as->timer, WLC_TXQ_DRAIN_TIMEOUT, 0);
		WL_ASSOC(("ROAM: waiting for %d tx packets drained"" out before roam\n",
			pktq_len(q) + TXPKTPENDTOT(wlc)));
	} else {
		/* When a STA tries to join an AP on non-radar channel disable the PHY radar
		 * detection. This can happen when a STA (associated to 5G AP on radar channel)
		 * tries to join another 2G AP or any other 5G AP on non-radar channel.
		 */
#if defined(STA) && defined(SLAVE_RADAR)
		wlc_bss_list_t	*jointargets = wlc->as_info->join_targets;
		if (WL11H_STA_ENAB(wlc) &&
			!wlc_radar_chanspec(wlc->cmi,
				jointargets->ptrs[wlc->as_info->join_targets_last]->chanspec)) {
			phy_radar_detect_enable((phy_info_t *)WLC_PI(wlc), FALSE);
		}
#endif /* defined(STA) && defined(SLAVE_RADAR) */
		wlc_join_BSS(cfg,
		wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last]);
	}
}

#ifdef AP
/**
 * Checks to see if we need to send out CSA for local ap cfgs before attempting to join a new AP.
 * Parameter description:
 * wlc - global wlc structure
 * cfg - the STA cfg that is about to perform a join a new AP
 * chanspec - the new chanspec the new AP is on
 * state - can be either AS_WAIT_FOR_AP_CSA or AS_WAIT_FOR_AP_CSA_ROAM_FAIL
 */
static bool
wlc_join_check_ap_need_csa(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec, uint state)
{
	wlc_assoc_t *as = cfg->assoc;
	bool need_csa = FALSE;
	chanspec_t curr_chanspec = WLC_BAND_PI_RADIO_CHANSPEC;

	/* If we're not in AS_WAIT_FOR_AP_CSA states, about to change channel
	 * and AP is active, allow AP to send CSA before performing the channel switch.
	 */
	if ((as->state != state) &&
	    (curr_chanspec != chanspec) &&
	    AP_ACTIVE(wlc)) {
		/* check if any stations associated to our APs */
		if (!wlc_scb_associated_to_ap(wlc, NULL)) {
			/* no stations associated to our APs, return false */
			WL_ASSOC(("wl%d.%d: %s: not doing ap CSA, no stas associated to ap(s)\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			return (FALSE);
		}
		if (wlc_join_ap_do_csa(wlc, chanspec)) {
			wlc_ap_mute(wlc, FALSE, cfg, -1);
			WL_ASSOC(("wl%d.%d: %s: doing ap CSA\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			wlc_assoc_change_state(cfg, state);
			need_csa = TRUE;
		}
	}
	else {
		WL_ASSOC(("wl%d.%d: %s: ap CSA not needed\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
	}
	return need_csa;
}

static bool
wlc_join_ap_do_csa(wlc_info_t *wlc, chanspec_t tgt_chanspec)
{
	wlc_bsscfg_t *apcfg;
	int apidx;
	bool ap_do_csa = FALSE;

	if (WL11H_ENAB(wlc)) {
		FOREACH_UP_AP(wlc, apidx, apcfg) {
			/* find all ap's on current channel */
			if ((WLC_BAND_PI_RADIO_CHANSPEC == apcfg->current_bss->chanspec) &&
#ifdef WLBTAMP
				/* Do not announce CSA for BT-AMP */
				!BSS_BTA_ENAB(wlc, apcfg) &&
#endif // endif
#ifdef WLMCHAN
				(!MCHAN_ENAB(wlc->pub) || wlc_mchan_stago_is_disabled(wlc->mchan) ||
				BSSCFG_AP_MCHAN_DISABLED(wlc, apcfg))) {
#else
				1) {
#endif // endif
				wlc_csa_do_switch(wlc->csa, apcfg, tgt_chanspec);
				ap_do_csa = TRUE;
				WL_ASSOC(("wl%d.%d: apply channel switch\n", wlc->pub->unit,
					apidx));
			}
		}
	}

	return ap_do_csa;
}
#endif /* AP */

static void
wlc_ibss_set_bssid_hw(wlc_bsscfg_t *cfg, struct ether_addr *bssid)
{
	struct ether_addr save;

	save = cfg->BSSID;
	cfg->BSSID = *bssid;
	wlc_set_bssid(cfg);
	cfg->BSSID = save;
}

static void
wlc_join_noscb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_assoc_t *as)
{
	/* No SCB: move state to fake an auth anyway so FSM moves */
	WL_ASSOC(("wl%d.%d: can't create scb\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
	wlc_assoc_change_state(cfg, AS_SENT_AUTH_1);
	as->bss_retries = as->retry_max;
	wl_del_timer(wlc->wl, as->timer);
	wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
}

static void *
wlc_join_assoc_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	wlc_bss_info_t *bi, bool associated)
{
	uint wsec_cfg;
	uint wsec_scb;
	bool allow_reassoc = TRUE;

	ASSERT(cfg != NULL);
	ASSERT(scb != NULL);

	wsec_cfg = cfg->wsec;

	/* Update SCB crypto based on highest WSEC settings */
	if (WSEC_AES_ENABLED(wsec_cfg))
		wsec_scb = AES_ENABLED;
	else if (WSEC_TKIP_ENABLED(wsec_cfg))
		wsec_scb = TKIP_ENABLED;
	else if (WSEC_WEP_ENABLED(wsec_cfg))
		wsec_scb = WEP_ENABLED;
#ifdef BCMWAPI_WAI
	else if (WSEC_SMS4_ENABLED(wsec_cfg))
		wsec_scb = SMS4_ENABLED;
#endif /* BCMWAPI_WAI */
	else
		wsec_scb = 0;

	scb->wsec = wsec_scb;

	/* Based on wsec for the link, update AMPDU feature in the transmission path
	 * By spec, 11n device can send AMPDU only with Open or CCMP crypto
	 */
	if (N_ENAB(wlc->pub)) {
		if ((wsec_scb == WEP_ENABLED || wsec_scb == TKIP_ENABLED) &&
		    SCB_AMPDU(scb)) {
#ifdef WLAMPDU
			wlc_scb_ampdu_disable(wlc, scb);
#endif // endif
		}
#ifdef MFP
		else if (WLC_MFP_ENAB(wlc->pub) && SCB_MFP(scb)) {
#ifdef WLAMPDU
			wlc_scb_ampdu_disable(wlc, scb);
#endif // endif
		}
#endif /* MFP */
		else if (SCB_AMPDU(scb)) {
#ifdef WLAMPDU
			wlc_scb_ampdu_enable(wlc, scb);
#endif /* WLAMPDU */
		}
	}

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && (bi->ccx_version < 4) && CAC_ENAB(wlc->pub)) {
		WL_CAC(("wl%d: wlc_cac_tspec_append: CAC not supported by"
			" CCX_VERSION %d\n", wlc->pub->unit, bi->ccx_version));
		wlc_cac_param_reset_all(wlc->cac, scb);
	}
#endif // endif
#if defined(WLFBT)
	/* For FT cases, send out assoc request when
	* - doing initial FT association (roaming from a different security type)
	* - FT AKM suite for current and next association do not match
	* - there is link down or disassoc, as indicated by null current_bss->BSSID
	* - switching to a different FT mobility domain
	* Send out reassoc request
	* - when FT AKM suite for the current and next association matches
	* - for all non-FT associations
	*/
	if (!(cfg->WPA_auth & WPA2_AUTH_FT))
		allow_reassoc = TRUE;
	else if (BSSCFG_IS_FBT(cfg) && !ETHER_ISNULLADDR(&cfg->current_bss->BSSID) &&
		wlc_fbt_is_fast_reassoc(wlc->fbt, cfg, bi)) {
		allow_reassoc = TRUE;
	} else
		allow_reassoc = FALSE;
#endif /* WLFBT */

	if (wlc->vhti && wlc_vht_is_omn_enabled(wlc->vhti, scb)) {
		/* disallow reassociation since oper mode changes must be redone */
		allow_reassoc = FALSE;
		wlc_vht_disable_scb_oper_mode(wlc->vhti, scb);
	}

	return wlc_sendassocreq(wlc, bi, scb, associated && allow_reassoc);
} /* wlc_join_assoc_start */

void
wlc_join_BSS(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_phy_t *pi = WLC_PI(wlc);
	wlc_assoc_t *as = cfg->assoc;
	wlc_roam_t *roam = cfg->roam;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	struct scb *scb;
	void *pkt;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	bool cck_only;
	wlc_rateset_t *rs_hw;
	chanspec_t chanspec;
	wlcband_t *band;
	bool ht_wsec = TRUE;
	bool bss_ht, bss_vht;
	bool ch_changed = FALSE;
#if defined(WLFBT)
	bool ft_band_changed = TRUE;
#endif /* WLFBT */
	uint8 mcsallow = 0;

	BCM_REFERENCE(bss_vht);

	ASSERT(bi != NULL);
	if (bi == NULL) {
		/* Validity check */
		WL_ASSOC(("wl%d: %s: JOIN: BSS info == NULL, join_targets_last[%d]\n",
			WLCWLUNIT(wlc), __FUNCTION__, wlc->as_info->join_targets_last));
		return;
	}

	WL_ASSOC(("wl%d.%d: JOIN: %s: selected BSSID: %s\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
	      __FUNCTION__, bcm_ether_ntoa(&bi->BSSID, eabuf)));

	if (bi->flags & WLC_BSS_HT) {
		/* HT and TKIP(WEP) cannot be used for HT STA. When AP advertises HT and TKIP */
		/* (or WEP)  only,	downgrade to leagacy STA */
		if (WSEC_TKIP_ENABLED(cfg->wsec)) {
			if (!WSEC_AES_ENABLED(cfg->wsec) || (!UCAST_AES(&bi->wpa) &&
				!UCAST_AES(&bi->wpa2))) {
				if (wlc_ht_get_wsec_restriction(wlc->hti) &
						WLC_HT_TKIP_RESTRICT)
					ht_wsec = FALSE;
			}
		} else if (WSEC_WEP_ENABLED(cfg->wsec)) {
#if defined(NDIS)
				/* In WPS Exchage, Win7/Win8 will set WEP as default cipher no
				 * matter what ciphers AP used. In this case, downgrading to
				 * leagacy STA will prevent our association to 11n only AP.
				 */
				if (!(bi->wpa.flags & RSN_FLAGS_SUPPORTED) &&
					!(bi->wpa2.flags & RSN_FLAGS_SUPPORTED))
#endif /* NDIS */
				{
					if (wlc_ht_get_wsec_restriction(wlc->hti) &
							WLC_HT_WEP_RESTRICT)
						ht_wsec = FALSE;
				}
		}
		/* safe mode: if AP is HT capable, downgrade to legacy mode because */
		/* NDIS won't associate to HT AP */
		if (!ht_wsec || BSSCFG_SAFEMODE(cfg)) {
			if (BSSCFG_SAFEMODE(cfg)) {
				WL_INFORM(("%s(): wl%d.%d: safe mode enabled, disable HT!\n",
					__FUNCTION__, WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
			}
			bi->flags &= ~(WLC_BSS_HT | WLC_BSS_40INTOL | WLC_BSS_SGI_20 |
				WLC_BSS_SGI_40 | WLC_BSS_40MHZ);
			bi->chanspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(bi->chanspec));

			/* disable VHT and SGI80 also */
			bi->flags2 &= ~(WLC_BSS_VHT | WLC_BSS_SGI_80);
		}
	}

#ifdef AP
	if (wlc_join_check_ap_need_csa(wlc, cfg, bi->chanspec, AS_WAIT_FOR_AP_CSA)) {
		WL_ASSOC(("wl%d.%d: JOIN: %s delayed due to ap active, wait for ap CSA\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return;
	}
#endif /* AP */

#ifdef EXT_STA
	/* N.B.: bss_type passed through auth_type event field
	*  as->type passed through result field and roam reason
	*  passed through status field
	*/
	if (WLEXTSTA_ENAB(wlc->pub)) {
		wlc_bss_mac_event(wlc, cfg, WLC_E_ASSOC_START, &bi->BSSID, as->type,
			((as->type == AS_ROAM) || (as->type == AS_RECREATE)
			? roam->reason : 0),
			WLC_DOT11_BSSTYPE(bi->infra), NULL, 0);
	}
#endif /* EXT_STA */

	band = wlc->bandstate[CHSPEC_IS2G(bi->chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];

	/* prune invalid chanspec based on STA capibility */
	if ((chanspec = wlc_max_valid_bw_chanspec(wlc, band, cfg, bi->chanspec))
	    == INVCHANSPEC)
	{
		WL_ASSOC(("wl%d: JOIN: Skipping invalid chanspec(0x%x):",
			WLCWLUNIT(wlc), bi->chanspec));
		return;
	}

	wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, DATA_BLOCK_JOIN);

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		WL_MCHAN(("wl%d.%d: %s: Creating chanctx for 0x%x\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, chanspec));
		if (wlc_mchan_create_bss_chan_context(wlc, cfg, chanspec)) {
			WL_ERROR(("wl%d: JOIN: Creating a new bss_chan_context failed\n",
				WLCWLUNIT(wlc)));
			return;
		}
	}
#endif // endif

/* Change the radio channel to match the target_bss.
	 * FBToverDS fallback: change the channel while switching to FBT over-the-air if no response
	 * received for FT Request frame from the current AP or if current AP
	 * is not reachable.
	 */
	if (wlc_chk_chanspec_update(wlc, chanspec) &&
#ifdef WLFBT
	    ((as->type != AS_ROAM) ||
	     !BSSCFG_IS_FBT(cfg) || !(bi->flags2 & WLC_BSS_OVERDS_FBT) ||
		(as->state == AS_SENT_FTREQ) || (roam->reason == WLC_E_REASON_BCNS_LOST)) &&
#endif /* WLFBT */
		TRUE) {
		/* clear the quiet bit on the dest channel */
		wlc_clr_quiet_chanspec(wlc->cmi, chanspec);
		wlc_suspend_mac_and_wait(wlc);

#if defined(WLFBT)
		/* If we're likely to do FT reassoc, check if we're going to change bands */
		if (BSSCFG_IS_FBT(cfg) &&
			(as->type == AS_ROAM) && (cfg->auth == DOT11_OPEN_SYSTEM) &&
			wlc_fbt_is_cur_mdid(wlc->fbt, cfg, bi)) {
			if (CHSPEC_BAND(cfg->current_bss->chanspec) == CHSPEC_BAND(chanspec))
				ft_band_changed = FALSE;
		}
#endif /* WLFBT */
		wlc_set_chanspec(wlc, chanspec);
		wlc_enable_mac(wlc);
		ch_changed = TRUE;
	} else if (wlc_quiet_chanspec(wlc->cmi, chanspec)) {
		/* clear the quiet bit on our channel and un-quiet */
		wlc_clr_quiet_chanspec(wlc->cmi, chanspec);
		wlc_mute(wlc, OFF, 0);
	}

#ifdef WLMCHAN
	/* if we did not change channels we still might need to update
	 * the primay and active queue
	 */
	if (MCHAN_ENAB(wlc->pub)) {
		wlc_mchan_set_priq(wlc->mchan, cfg);
	}
#endif // endif

	rs_hw = &wlc->band->hw_rateset;

	/* select target bss info */
	bcopy((char*)bi, (char*)target_bss, sizeof(wlc_bss_info_t));

	/* update the target_bss->chanspec after possibe narrowing to 20MHz */
	target_bss->chanspec = chanspec;

	/* Keep only CCK if gmode == GMODE_LEGACY_B */
	if (BAND_2G(wlc->band->bandtype) && wlc->band->gmode == GMODE_LEGACY_B)
		cck_only = TRUE;
	else
		cck_only = FALSE;

	/* Make sure to verify AP also advertises WMM IE before updating HT IE
	 * Some AP's advertise HT IE in beacon without WMM IE, but AP's behaviour
	 * is unpredictable on using HT/AMPDU.
	 */
	bss_ht = (bi->flags & WLC_BSS_HT) &&
		(bi->flags & WLC_BSS_WME) && BSS_N_ENAB(wlc, cfg);
	bss_vht = (bi->flags2 & WLC_BSS_VHT) &&
		(bi->flags & WLC_BSS_WME) && BSS_VHT_ENAB(wlc, cfg);

	if (bss_ht)
		mcsallow |= WLC_MCS_ALLOW;
	if (bss_vht)
		mcsallow |= WLC_MCS_ALLOW_VHT;

	if (WLPROPRIETARY_11N_RATES_ENAB(wlc->pub) &&
	    wlc->pub->ht_features != WLC_HT_FEATURES_PROPRATES_DISAB)
		mcsallow |= WLC_MCS_ALLOW_PROP_HT;

	if ((mcsallow & WLC_MCS_ALLOW_VHT) &&
		WLC_VHT_FEATURES_GET(wlc->pub, WL_VHT_FEATURES_1024QAM))
		mcsallow |= WLC_MCS_ALLOW_1024QAM;

	wlc_rateset_filter(&target_bss->rateset /* src */, &target_bss->rateset /* dst */,
	                   FALSE, cck_only ? WLC_RATES_CCK : WLC_RATES_CCK_OFDM,
	                   RATE_MASK_FULL, cck_only ? 0 : mcsallow);

	/* XXX PR 102333: Filter out 36/48/54 from target_bss rateset because 43142 has
	 * problem receiving short packets at those rates
	 */
	if (CHIPID(wlc->pub->sih->chip) == BCM43142_CHIP_ID) {
		uint i;
		for (i = 0; i < target_bss->rateset.count; i++) {
			uint8 rate;
			rate = target_bss->rateset.rates[i] & RATE_MASK;
			if ((rate == WLC_RATE_36M) || (rate == WLC_RATE_48M) ||
				(rate == WLC_RATE_54M))
				target_bss->rateset.rates[i] &= ~WLC_RATE_FLAG;
		}
	}

	/* apply default rateset to invalid rateset */
	if (!wlc_rate_hwrs_filter_sort_validate(&target_bss->rateset /* [in+out] */,
		rs_hw /* [in] */, TRUE,
		wlc->stf->op_txstreams)) {
		WL_RATE(("wl%d: %s: invalid rateset in target_bss. bandunit 0x%x phy_type 0x%x "
			"gmode 0x%x\n", wlc->pub->unit, __FUNCTION__, wlc->band->bandunit,
			wlc->band->phytype, wlc->band->gmode));
#ifdef BCMDBG
		wlc_rateset_show(wlc, &target_bss->rateset, &bi->BSSID);
#endif // endif

		wlc_rateset_default(&target_bss->rateset, rs_hw, wlc->band->phytype,
			wlc->band->bandtype, cck_only, RATE_MASK_FULL,
			cck_only ? 0 : wlc_get_mcsallow(wlc, NULL),
			CHSPEC_WLC_BW(target_bss->chanspec), wlc->stf->op_rxstreams);
	}

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, cfg)) {
		wlc_rateset_filter(&target_bss->rateset, &target_bss->rateset, FALSE,
		                   WLC_RATES_OFDM, RATE_MASK_FULL, wlc_get_mcsallow(wlc, cfg));
	}
#endif // endif

#ifdef BCMDBG
	wlc_rateset_show(wlc, &target_bss->rateset, &bi->BSSID);
#endif // endif

	wlc_rate_lookup_init(wlc, &target_bss->rateset);

#if defined(PHYCAL_CACHING)
	/* update channel cache ctx for BSS */
	wlc_phy_create_chanctx(pi, bi->chanspec);
#endif // endif

	/* XXX Call Cals unconditionally on join BSS for ACPHY
	 * this is needed as the Calibration coeffs are zeroed out
	 * during a scan if there is a band switch. Dont do this during
	 * assoc recreation since full phy cal is already done
	 */
	/* Skip phy calibration here for FBT over-the DS. This is mainly for
	 * reducing transition time for over-the-DS case where channel switching
	 * happens only after receiving FT response.
	 */
	if (WLCISACPHY(wlc->band) &&
		!(BSSCFG_IS_FBT(cfg) && (target_bss->flags2 & WLC_BSS_OVERDS_FBT))) {
		/* skip cal for PSTAs; doing it on primary is good enough */
		if (!(cfg->BSS && as->type == AS_RECREATE &&
		      as->flags & AS_F_SPEEDY_RECREATE) && !BSSCFG_PSTA(cfg)) {
			WL_ASSOC(("wl%d: Call full phy cal from join_BSS\n",
			          WLCWLUNIT(wlc)));
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
			wlc_phy_interference_set(WLC_PI(wlc), TRUE);
			wlc_phy_acimode_noisemode_reset(WLC_PI(wlc),
			                                chanspec, FALSE, TRUE, FALSE);
		}
	}
	else if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) &&
#if defined(WLFBT)
		(BSSCFG_IS_FBT(cfg) && ft_band_changed) &&
#endif /* WLFBT */
	    (as->type != AS_ROAM || ch_changed)) {
		/* phy full cal takes time and slow down reconnection after sleep.
		 * avoid double phy full cal if assoc recreation already did so
		 * and no channel changed(if AS_F_SPEEDY_RECREATE is still set)
		 */
		if (!(cfg->BSS && as->type == AS_RECREATE &&
			as->flags & AS_F_SPEEDY_RECREATE))
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
		wlc_phy_interference_set(pi, TRUE);
		wlc_phy_acimode_noisemode_reset(pi,
			chanspec, FALSE, TRUE, FALSE);
	}

	/* set infra mode */
	wlc_bsscfg_set_infra_mode(wlc, cfg, bi->infra != 0);

	/* attempt to associate with the selected BSS */
	if (bi->infra == 0) {
		/* IBSS join */
		int beacon_wait_time;
#ifdef WLMCNX
		if (MCNX_ENAB(wlc->pub)) {
			cfg->current_bss->dtim_period = 1;
			wlc_mcnx_bss_upd(wlc->mcnx, cfg, TRUE);
		}
#endif // endif

		/*
		 * nothing more to do from a protocol point of view...
		 * join the BSS when the next beacon is received...
		 */
		WL_ASSOC(("wl%d: JOIN: IBSS case, wait for a beacon\n", WLCWLUNIT(wlc)));
		/* configure the BSSID addr for STA w/o promisc beacon */
		wlc_ibss_set_bssid_hw(cfg, &bi->BSSID);
		wlc_assoc_change_state(cfg, AS_WAIT_RCV_BCN);

		/* Start the timer for beacon receive timeout handling to ensure we move
		 * out of AS_WAIT_RCV_BCN state in case we never receive the first beacon
		 */
		wlc_assoc_timer_del(wlc, cfg);
		beacon_wait_time = as->recreate_bi_timeout *
			(bi->beacon_period * DOT11_TU_TO_US) / 1000;

		/* Validate beacon_wait_time, usually it will be 1024 with beacon interval 100TUs */
		beacon_wait_time = beacon_wait_time > 10000 ? 10000 :
			beacon_wait_time < 1000 ? 1024 : beacon_wait_time;

		wl_add_timer(wlc->wl, as->timer, beacon_wait_time, 0);

		wlc_bsscfg_up(wlc, cfg);

		/* notifying interested parties of the state... */
		wlc_bss_assoc_state_notif(wlc, cfg, as->type, as->state);

	} else {
		/* BSS join */

#ifdef WLP2P
		/* P2P: configure the BSS TSF, NoA, CTWindow for early adoption */
		if (BSS_P2P_ENAB(wlc, cfg))
			wlc_p2p_adopt_bss(wlc->p2p, cfg, target_bss);
#endif // endif

		/* running out of scbs is fatal here */
		if (!(scb = wlc_scblookup(wlc, cfg, &bi->BSSID))) {
			WL_ERROR(("wl%d: %s: out of scbs\n", wlc->pub->unit, __FUNCTION__));
			wlc_join_noscb(wlc, cfg, as);
			return;
		}

		/* the cap and additional IE are only in the bcn/prb response pkts,
		 * when joining a bss parse the bcn_prb to check for these IE's.
		 * else check if SCB state needs to be cleared if AP might have changed its mode
		 */
		{
#ifdef WL11N
			ht_cap_ie_t *cap_ie = NULL;
			ht_add_ie_t *add_ie = NULL;

			if (bss_ht) {
				obss_params_t *obss_ie = NULL;
				uint len = target_bss->bcn_prb_len - sizeof(struct dot11_bcn_prb);
				uint8 *parse = (uint8*)target_bss->bcn_prb +
					sizeof(struct dot11_bcn_prb);

				/* extract ht cap and additional ie */
				cap_ie = wlc_read_ht_cap_ies(wlc, parse, len);
				add_ie = wlc_read_ht_add_ies(wlc, parse, len);
				if (COEX_ENAB(wlc))
					obss_ie = wlc_ht_read_obss_scanparams_ie(wlc, parse, len);
				wlc_ht_update_scbstate(wlc->hti, scb, cap_ie, add_ie, obss_ie);
			} else if ((scb->flags & SCB_HTCAP) &&
			           ((bi->flags & WLC_BSS_HT) != WLC_BSS_HT)) {
				wlc_ht_update_scbstate(wlc->hti, scb, NULL, NULL, NULL);
			}
#endif /* WL11N */
#ifdef WL11AC
			if (bss_vht) {
				vht_cap_ie_t *vht_cap_ie_p;
				vht_cap_ie_t vht_cap_ie;

				vht_op_ie_t *vht_op_ie_p;
				vht_op_ie_t vht_op_ie;

				uint8 *prop_tlv = NULL;
				int prop_tlv_len = 0;
				uint8 vht_ratemask = 0;
				int target_bss_band = CHSPEC2WLC_BAND(chanspec);

				uint len = target_bss->bcn_prb_len - sizeof(struct dot11_bcn_prb);
				uint8 *parse = (uint8*)target_bss->bcn_prb +
					sizeof(struct dot11_bcn_prb);

				/*
				 * Extract ht cap and additional ie
				 * Encapsulated Prop VHT IE appears if we are running VHT in 2.4G or
				 * the extended rates are enabled
				 */
				if (BAND_2G(target_bss_band) || WLC_VHT_FEATURES_RATES(wlc->pub)) {
					prop_tlv = wlc_read_vht_features_ie(wlc->vhti, parse, len,
						&vht_ratemask, &prop_tlv_len, target_bss);
				}

				if  (prop_tlv) {
					vht_cap_ie_p = wlc_read_vht_cap_ie(wlc->vhti, prop_tlv,
						prop_tlv_len, &vht_cap_ie);
					vht_op_ie_p = wlc_read_vht_op_ie(wlc->vhti, prop_tlv,
						prop_tlv_len, &vht_op_ie);
				} else {
					vht_cap_ie_p =
						wlc_read_vht_cap_ie(wlc->vhti,
						parse, len, &vht_cap_ie);
					vht_op_ie_p =
						wlc_read_vht_op_ie(wlc->vhti,
						parse, len, &vht_op_ie);
				}
				wlc_vht_update_scb_state(wlc->vhti, target_bss_band, scb, cap_ie,
					vht_cap_ie_p, vht_op_ie_p, vht_ratemask);
			} else if ((scb->flags2 & SCB2_VHTCAP) &&
			           ((bi->flags2 & WLC_BSS_VHT) != WLC_BSS_VHT)) {
					wlc_vht_update_scb_state(wlc->vhti,
					CHSPEC2WLC_BAND(chanspec), scb, NULL, NULL, NULL, 0);
			}
#endif /* WL11AC */
		}

		/* just created or assigned an SCB for the AP, flag as MYAP */
		ASSERT(!(SCB_ASSOCIATED(scb) && !(scb->flags & SCB_MYAP)));
		scb->flags |= SCB_MYAP;

		/* replace any old scb rateset with new target rateset */
#ifdef WLP2P
		if (BSS_P2P_ENAB(wlc, cfg))
			wlc_rateset_filter(&target_bss->rateset /* src */, &scb->rateset /* dst */,
				FALSE, WLC_RATES_OFDM, RATE_MASK, mcsallow);
		else
#endif // endif
		wlc_rateset_filter(&target_bss->rateset /* src */, &scb->rateset /* dst */, FALSE,
		                   WLC_RATES_CCK_OFDM, RATE_MASK, mcsallow);
		wlc_scb_ratesel_init(wlc, scb);

		wlc_assoc_timer_del(wlc, cfg);

#ifdef NOT_YET
		/* send (re)association request */
		if ((scb->state & (AUTHENTICATED | PENDING_ASSOC)) == AUTHENTICATED) {
			WL_ASSOC(("wl%d: JOIN: BSS case, sending %s REQ ...\n", WLCWLUNIT(wlc),
				(wlc->pub->associated ? "REASSOC" : "ASSOC")));
			wlc_scb_setstatebit(scb, PENDING_ASSOC);
			wlc_assoc_change_state(cfg, AS_SENT_ASSOC);
			pkt = wlc_join_assoc_start(wlc, cfg, scb, bi, cfg->associated);
			if (pkt != NULL)
				wlc_pcb_fn_register(wlc->pcb, wlc_assocreq_complete,
				                    (void *)(uintptr)cfg->ID, pkt);
			else
				wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
		}
		else
#endif /* NOT_YET */

		/* send authentication request */
		if (!(scb->state & (PENDING_AUTH | PENDING_ASSOC))) {
			int auth = cfg->auth;
			cfg->auth_atmptd = auth;
#ifdef BCMCCX
			if (CCX_ENAB(wlc->pub) && wlc->ccx->ccx_network)
				auth = wlc_ccx_set_auth(wlc->ccx, auth);
#endif /* BCMCCX */
#if defined(WLFBT)
			/* Make sure STA is associated to a valid BSSID before doing
			 * a fast transition.
			 */
			if (BSSCFG_IS_FBT(cfg) && (as->type == AS_ROAM) &&
				(auth == DOT11_OPEN_SYSTEM) &&
				wlc_fbt_is_fast_reassoc(wlc->fbt, cfg, bi) &&
				!ETHER_ISNULLADDR(&cfg->current_bss->BSSID)) {
				auth = cfg->auth_atmptd = DOT11_FAST_BSS;
#ifdef BCMSUP_PSK
				if (SUP_ENAB(wlc->pub))
					wlc_sup_set_ea(wlc->idsup, cfg, &bi->BSSID);
#endif /* BCMSUP_PSK */
				wlc_fbt_set_ea(wlc->fbt, cfg, &bi->BSSID);
			}
#endif /* WLFBT */
			WL_ASSOC(("wl%d: JOIN: BSS case, sending AUTH REQ alg=%d ...\n",
			            WLCWLUNIT(wlc), auth));
			wlc_scb_setstatebit(scb, PENDING_AUTH | PENDING_ASSOC);

			/* if we were roaming, mark the old BSSID so we don't thrash back to it */
			if (as->type == AS_ROAM &&
			    (roam->reason == WLC_E_REASON_MINTXRATE ||
			     roam->reason == WLC_E_REASON_TXFAIL)) {
				uint idx;
				for (idx = 0; idx < roam->cache_numentries; idx++) {
					if (!bcmp(&roam->cached_ap[idx].BSSID,
					          &cfg->current_bss->BSSID,
					          ETHER_ADDR_LEN)) {
						roam->cached_ap[idx].time_left_to_next_assoc =
						        ROAM_REASSOC_TIMEOUT;
						WL_ASSOC(("wl%d: %s: Marking current AP as "
						          "unavailable for reassociation for %d "
						          "seconds due to roam reason %d\n",
						          wlc->pub->unit, __FUNCTION__,
						          ROAM_REASSOC_TIMEOUT, roam->reason));
						roam->thrash_block_active = TRUE;
					}
				}
			}

#if defined(WLFBT)
			/* Skip FBT over-the-DS if current AP is not reachable or does not
			 * respond to FT Request frame and instead send auth frame to target AP.
			 */
			if (BSSCFG_IS_FBT(cfg) && (as->type == AS_ROAM) && cfg->associated &&
				(auth == DOT11_FAST_BSS) && (bi->flags2 & WLC_BSS_OVERDS_FBT) &&
				(roam->reason != WLC_E_REASON_BCNS_LOST) &&
				(as->state != AS_SENT_FTREQ)) {
				/* If not initial assoc (as->state != JOIN_START)
				   and AP is Over-the-DS-capable
				*/
				if (PS_ALLOWED(cfg))
					wlc_set_pmstate(cfg, FALSE);
				wlc_assoc_change_state(cfg, AS_SENT_FTREQ);

				pkt = wlc_fbt_send_overds_req(wlc->fbt, cfg,
					&cfg->current_bss->BSSID, scb,
					((bi->capability & DOT11_CAP_SHORT) != 0));
			}
			else
#endif /* WLFBT */
			{
#ifdef WL_SAE
				if (BSSCFG_EXT_AUTH(cfg) && auth == DOT11_SAE) {
					if (wlc_assoc_start_extauth(wlc, cfg,
						&bi->BSSID, auth, AS_SENT_AUTH_1)) {
						WL_ASSOC(("wl%d: JOIN: External"
						       "auth Trigger Failed", WLCWLUNIT(wlc)));
					}
					return;
				}
#endif /* WL_SAE */

				wlc_assoc_change_state(cfg, AS_SENT_AUTH_1);
				pkt = wlc_sendauth(cfg, &scb->ea, &bi->BSSID, scb,
					auth, 1, DOT11_SC_SUCCESS, NULL,
					((bi->capability & DOT11_CAP_SHORT) != 0));

				/* XXX Adding a 300ms timer to clear the AS in progress
				* if the wlc_tx_auth_complete doesn't come if burst of
				* scan request is received from the OS
				* Radar 11662086
				*/
				wl_add_timer(wlc->wl, as->timer, WECA_AUTH_TIMEOUT, 0);
			}

#if defined(WLP2P) && defined(BCMDBG)
			if (WL_P2P_ON()) {
				int bss = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
				uint16 state = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_ST(bss));
				uint16 next_noa = wlc_mcnx_read_shm(wlc->mcnx,
				                                   M_P2P_BSS_N_NOA(bss));
				uint16 hps = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_HPS);

				WL_P2P(("wl%d: %s: queue AUTH at tick 0x%x ST 0x%04X "
				        "N_NOA 0x%X HPS 0x%04X\n",
				        wlc->pub->unit, __FUNCTION__,
				        R_REG(wlc->osh, &wlc->regs->tsf_timerlow),
				        state, next_noa, hps));
			}
#endif /* WLP2P && BCMDBG */
			if (pkt == NULL)
				wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
		}
	}

	as->bss_retries ++;
} /* wlc_join_BSS */

void
wlc_assoc_timeout(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;

	WL_TRACE(("wl%d: wlc_timer", wlc->pub->unit));

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	if (ASSOC_RECREATE_ENAB(wlc->pub) &&
	    as->type == AS_RECREATE && as->state == AS_RECREATE_WAIT_RCV_BCN) {
		/* We reached the timeout waiting for a beacon from our former AP so take
		 * further action to reestablish our association.
		 */
		wlc_assoc_recreate_timeout(cfg);
	} else if (ASSOC_RECREATE_ENAB(wlc->pub) &&
	           as->type == AS_RECREATE && as->state == AS_ASSOC_VERIFY) {
		/* reset the association state machine */
		as->type = AS_NONE;
		wlc_assoc_change_state(cfg, AS_IDLE);
#ifdef NLO
		wlc_bss_mac_event(wlc, cfg, WLC_E_ASSOC_RECREATED, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
#endif /* NLO */
	} else if (as->state == AS_SENT_AUTH_1 ||
	           as->state == AS_SENT_AUTH_3 ||
	           as->state == AS_SENT_FTREQ) {
		wlc_auth_complete(cfg, WLC_E_STATUS_TIMEOUT, &target_bss->BSSID,
			0, cfg->auth);
#if defined(WL_SAE)
	} else if ((as->state == AS_SENT_AUTH_UP) || (as->state == AS_START_EXT_AUTH)) {

		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID, 0, cfg->auth);
		/* going to retry sending AUTH_REQ. don't delete the assoc timeslot */
#endif /* WL_SAE */
	} else if (as->state == AS_SENT_ASSOC ||
		as->state == AS_REASSOC_RETRY ||
		as->state == AS_WAIT_RCV_BCN) {
		wlc_assoc_complete(cfg, WLC_E_STATUS_TIMEOUT, &target_bss->BSSID, 0,
			as->type != AS_ASSOCIATION,
			WLC_DOT11_BSSTYPE(target_bss->infra));
	} else if (as->state == AS_IDLE && wlc->as_info->sta_retry_time &&
	           BSSCFG_STA(cfg) && cfg->enable && !cfg->associated) {
		wlc_ssid_t ssid;
		/* STA retry: armed from wlc_set_ssid_complete() */
		WL_ASSOC(("wl%d: Retrying failed association\n", wlc->pub->unit));
		WL_APSTA_UPDN(("wl%d: wlc_assoc_timeout -> wlc_join()\n", wlc->pub->unit));
		/* XXX It was decided to retry STA join with broadcast scanning all channels
		 * because of its nature - upstream AP was unavailable during previous retries
		 * and we wouldn't know which channel it will be running when it comes up later.
		 */
		ssid.SSID_len = cfg->SSID_len;
		bcopy(cfg->SSID, ssid.SSID, ssid.SSID_len);
		wlc_join(wlc, cfg, ssid.SSID, ssid.SSID_len, NULL, NULL, 0);
	} else if (as->state == AS_WAIT_TX_DRAIN) {
		WL_ASSOC(("ROAM: abort txq drain after %d ms\n", WLC_TXQ_DRAIN_TIMEOUT));
		wlc_assoc_change_state(cfg, AS_WAIT_TX_DRAIN_TIMEOUT);
		wlc_join_BSS(cfg,
			wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last]);
	}
#if defined(BCMCCX) && defined(CCX_SDK)
	else if (as->state == AS_WAIT_IE) {
		WL_ASSOC(("wl%d: abort waiting CCX IEs after %d ms\n",
			wlc->pub->unit, WLC_IE_WAIT_TIMEOUT));
		wlc_assoc_change_state(cfg, AS_WAIT_IE_TIMEOUT);
		wlc_join_bss_start(cfg);
	}
#endif /* BCMCCX && CCX_SDK */
#ifdef EXT_STA
	else if (WLEXTSTA_ENAB(wlc->pub) &&
	         (as->state == AS_JOIN_CACHE_DELAY) && (as->flags & AS_F_CACHED_RESULTS)) {
		WL_ASSOC(("wl%d: delay 0.5sec for cached scan\n", wlc->pub->unit));
		wlc_assoc_change_state(cfg, AS_SCAN);
		wlc_assoc_scan_complete(wlc, WLC_E_STATUS_SUCCESS, cfg);
	}
#endif // endif
#if defined(WLRSDB) && defined(WL_MODESW)
	else if (WLC_MODESW_ENAB(wlc->pub) && RSDB_ENAB(wlc->pub) &&
		(as->state == AS_MODE_SWITCH_START)) {
		WL_ERROR(("wl:%d.%d MODE SWITCH FAILED. Timedout\n",
			WLCWLUNIT(wlc), cfg->_idx));
		wlc_assoc_change_state(cfg, AS_MODE_SWITCH_FAILED);
	}
#endif // endif

	return;
} /* wlc_assoc_timeout */

static void
wlc_assoc_recreate_timeout(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;

	WL_ASSOC(("wl%d: JOIN: RECREATE timeout waiting for former AP's beacon\n",
	          WLCWLUNIT(wlc)));

	/* Clear software BSSID information so that the current AP will be a valid roam target */
	wlc_bss_clear_bssid(cfg);

	wlc_update_bcn_info(cfg, FALSE);

	roam->reason = WLC_E_REASON_BCNS_LOST;

	if (WOWL_ENAB(wlc->pub) && cfg == wlc->cfg)
		roam->roam_on_wowl = TRUE;

	wlc_mac_request_entry(wlc, cfg, WLC_ACTION_RECREATE_ROAM);

	/* start the roam scan */
	wlc_assoc_scan_prep(cfg, NULL, NULL, NULL, 0);

}

void
wlc_join_recreate(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb *scb = NULL;
	wlc_assoc_t *as = bsscfg->assoc;
	wlc_bss_info_t *bi = bsscfg->current_bss;
	int beacon_wait_time;
	wlc_roam_t *roam = bsscfg->roam;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
	char ssidbuf[SSID_FMT_BUF_LEN];
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	/* validate that bsscfg and current_bss infrastructure/ibss mode flag match */
	ASSERT((bsscfg->BSS == (bi->infra == 1)));
	if (!(bsscfg->BSS == (bi->infra == 1))) {
		return;
	}

	/* these should already be set */
	if (!wlc->pub->associated || wlc->stas_associated == 0) {
		WL_ERROR(("wl%d: %s: both should have been TRUE: stas_assoc %d associated %d\n",
		          wlc->pub->unit, __FUNCTION__,
		          wlc->stas_associated, wlc->pub->associated));
	}

	/* Declare that driver is preserving the assoc */
	as->preserved = TRUE;

	if (bsscfg->BSS) {
		scb = wlc_scbfindband(wlc, bsscfg, &bi->BSSID, CHSPEC_WLCBANDUNIT(bi->chanspec));

		if (scb == NULL || (WLEXTSTA_ENAB(wlc->pub) && bsscfg->ar_disassoc)) {
			/* we lost our association - the AP no longer exists */
			wlc_assoc_init(bsscfg, AS_RECREATE);
			wlc_assoc_change_state(bsscfg, AS_LOSS_ASSOC);
		}
	}

	/* recreating an association to an AP */
	if (bsscfg->BSS && scb != NULL) {
		WL_ASSOC(("wl%d: JOIN: %s: recreating BSS association to ch %s %s %s\n",
		          WLCWLUNIT(wlc), __FUNCTION__,
		          wf_chspec_ntoa(bi->chanspec, chanbuf),
		          bcm_ether_ntoa(&bi->BSSID, eabuf),
		          (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));

		WL_ASSOC(("wl%d: %s: scb %s found\n", wlc->pub->unit, __FUNCTION__, eabuf));

		WL_ASSOC(("wl%d: JOIN: scb     State:%d Used:%d(%d)\n",
		          WLCWLUNIT(wlc),
		          scb->state, scb->used, (int)(scb->used - wlc->pub->now)));

		WL_ASSOC(("wl%d: JOIN: scb     Band:%s Flags:0x%x Cfg %p\n",
		          WLCWLUNIT(wlc),
		          ((scb->bandunit == BAND_2G_INDEX) ? BAND_2G_NAME : BAND_5G_NAME),
		          scb->flags, scb->bsscfg));

		WL_ASSOC(("wl%d: JOIN: scb     WPA_auth %d\n",
		          WLCWLUNIT(wlc), scb->WPA_auth));

		/* update scb state */
		/* 	scb->flags &= ~SCB_SHORTPREAMBLE; */
		/* 	if ((bi->capability & DOT11_CAP_SHORT) != 0) */
		/* 		scb->flags |= SCB_SHORTPREAMBLE; */
		/* 	scb->flags |= SCB_MYAP; */
		/* 	wlc_scb_setstatebit(scb, AUTHENTICATED | ASSOCIATED); */

		if (!(scb->flags & SCB_MYAP))
			WL_ERROR(("wl%d: %s: SCB_MYAP 0x%x not set in flags 0x%x!\n",
			          WLCWLUNIT(wlc), __FUNCTION__, SCB_MYAP, scb->flags));
		if ((scb->state & (AUTHENTICATED | ASSOCIATED)) != (AUTHENTICATED | ASSOCIATED))
			WL_ERROR(("wl%d: %s: (AUTHENTICATED | ASSOCIATED) 0x%x "
				"not set in scb->state 0x%x!\n",
				WLCWLUNIT(wlc), __FUNCTION__,
				(AUTHENTICATED | ASSOCIATED), scb->state));

		WL_ASSOC(("wl%d: JOIN: AID 0x%04x\n", WLCWLUNIT(wlc), bsscfg->AID));

		/*
		 * security setup
		 */
		if (scb->WPA_auth != bsscfg->WPA_auth)
			WL_ERROR(("wl%d: %s: scb->WPA_auth 0x%x "
				  "does not match bsscfg->WPA_auth 0x%x!",
				  WLCWLUNIT(wlc), __FUNCTION__, scb->WPA_auth, bsscfg->WPA_auth));

		/* disable txq processing until we are fully resynced with AP */
		wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, DATA_BLOCK_JOIN);
	} else if (!bsscfg->BSS) {
		WL_ASSOC(("wl%d: JOIN: %s: recreating IBSS association to ch %s %s %s\n",
		          WLCWLUNIT(wlc), __FUNCTION__,
		          wf_chspec_ntoa(bi->chanspec, chanbuf),
		          bcm_ether_ntoa(&bi->BSSID, eabuf),
		          (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));

		/* keep IBSS link indication up during recreation by resetting
		 * time_since_bcn
		 */
		roam->time_since_bcn = 0;
		roam->bcns_lost = FALSE;
	}

	/* suspend the MAC and configure the BSS/IBSS */
	wlc_suspend_mac_and_wait(wlc);
	wlc_BSSinit(wlc, bi, bsscfg, bsscfg->BSS ? WLC_BSS_JOIN : WLC_BSS_START);
	wlc_enable_mac(wlc);

#ifdef WLMCNX
	/* init multi-connection assoc states */
	if (MCNX_ENAB(wlc->pub) && bsscfg->BSS)
		wlc_mcnx_assoc_upd(wlc->mcnx, bsscfg, TRUE);
#endif // endif

	wlc_scb_ratesel_init_bss(wlc, bsscfg);

	/* clean up assoc recreate preserve flag */
	bsscfg->flags &= ~WLC_BSSCFG_PRESERVE;

#if defined(PHYCAL_CACHING)
	/* update channel cache ctx for rejoin */
	wlc_phy_create_chanctx(wlc->pi, bi->chanspec);
#endif // endif

	/* force a PHY cal on the current BSS/IBSS channel (channel set in wlc_BSSinit() */
	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band))
		wlc_full_phy_cal(wlc, bsscfg,
			(bsscfg->BSS ? PHY_PERICAL_JOIN_BSS : PHY_PERICAL_START_IBSS));

	wlc_bsscfg_up(wlc, bsscfg);

	/* if we are recreating an IBSS, we are done */
	if (!bsscfg->BSS) {
		/* should bump the first beacon TBTT out a few BIs to give the opportunity
		 * of a coalesce before our first beacon
		 */
		return;
	}

	if (as->type == AS_RECREATE && as->state == AS_LOSS_ASSOC) {
		/* Clear software BSSID information so that the current AP
		 * will be a valid roam target
		 */
		wlc_bss_clear_bssid(bsscfg);

		wlc_update_bcn_info(bsscfg, FALSE);

		roam->reason = WLC_E_REASON_INITIAL_ASSOC;

		wlc_mac_request_entry(wlc, bsscfg, WLC_ACTION_RECREATE);

		/* make sure the association retry timer is not pending */
		wlc_assoc_timer_del(wlc, bsscfg);

		/* start the roam scan */
#if defined(NDIS) && (NDISVER == 0x0620)
		/* to speed up fast-reconnect, should skip assoc-recreate full scan here */
		if (!wl_fast_scan_enabled(wlc->wl, NULL))
#endif // endif
		as->flags |= AS_F_SPEEDY_RECREATE;
		wlc_assoc_scan_prep(bsscfg, NULL, NULL, &bi->chanspec, 1);
	}
	else {
		/* if we are recreating a BSS, wait for the first beacon */
		wlc_assoc_init(bsscfg, AS_RECREATE);
		wlc_assoc_change_state(bsscfg, AS_RECREATE_WAIT_RCV_BCN);

		wlc_set_ps_ctrl(bsscfg);

		/* Set the timer to move the assoc recreate along
		 * if we do not see a beacon in our BSS/IBSS.
		 * Allow assoc_recreate_bi_timeout beacon intervals plus some slop to allow
		 * for medium access delay for the last beacon.
		 */
		beacon_wait_time = as->recreate_bi_timeout *
			(bi->beacon_period * DOT11_TU_TO_US) / 1000;
		beacon_wait_time += wlc->bcn_wait_prd; /* allow medium access delay */

		wlc_assoc_timer_del(wlc, bsscfg);
		wl_add_timer(wlc->wl, as->timer, beacon_wait_time, 0);
	}
} /* wlc_join_recreate */

#ifdef WL_SAE
static int wlc_assoc_start_extauth(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
		struct ether_addr *bssid, int auth_alg, uint state)
{
	wl_ext_auth_evt_t ext_auth_event;
	wlc_assoc_t *as = cfg->assoc;

	if (BSSCFG_AP(cfg)) {
		return BCME_NOTSTA;
	}
	if (auth_alg != WL_AUTH_SAE_KEY)
		return BCME_UNSUPPORTED;

	if (!bssid && !cfg->SSID)
		return BCME_ERROR;
	/* Send an event to upper layer to start External authentication */
	WL_ASSOC(("wl%d: %s: External Auth Initiated\n", wlc->pub->unit, __FUNCTION__));

	wlc_assoc_change_state(cfg, AS_START_EXT_AUTH);
	memcpy(ext_auth_event.ssid.SSID, cfg->SSID, cfg->SSID_len);
	ext_auth_event.ssid.SSID_len = cfg->SSID_len;
	memcpy(&ext_auth_event.bssid, bssid, ETHER_ADDR_LEN);
	wlc_bss_mac_event(wlc, cfg, WLC_E_START_AUTH, bssid,
			0, 0, 0,  &ext_auth_event, sizeof(ext_auth_event));
	wl_add_timer(wlc->wl, as->timer, WECA_AUTH_TIMEOUT + 10, 0);
	return BCME_OK;
}
#endif /* WL_SAE */

int
wlc_assoc_abort(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	struct scb *scb;
	int callbacks = 0;
#ifdef MACOSX
	bool is_macos = TRUE;
#elif defined(EXT_STA)
	bool is_macos = FALSE;
#endif // endif
#if defined(EXT_STA) || defined(MACOSX)
	bool is_recreate;
#endif // endif
	bool do_cmplt = FALSE;
	/* Save current band unit */
	int curr_bandunit = (int)wlc->band->bandunit;

	/* WES FIXME: check that all assoc states are cleaned up. take care with
	 * recent additions to state machine: TX_DRAIN, PASSHASH, JOIN_START, DISASSOC
	 */
	if (as->state == AS_IDLE || as->state == AS_JOIN_INIT) {
		/* assoc req may be idle but still have req in assoc req list */
		/* clean it up! */
		if (as->state == AS_JOIN_INIT)
			wlc_assoc_change_state(cfg, AS_IDLE);
		goto cleanup_assoc_req;
	}
	WL_ASSOC(("wl%d: aborting %s in progress\n", WLCWLUNIT(wlc), WLCASTYPEN(as->type)));
	WL_ROAM(("wl%d: aborting %s in progress\n", WLCWLUNIT(wlc), WLCASTYPEN(as->type)));

	if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d: wlc_assoc_abort: aborting association scan in process\n",
		            WLCWLUNIT(wlc)));
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
	}

	if (as->state == AS_SENT_AUTH_1 ||
	    as->state == AS_SENT_AUTH_3 ||
		as->state == AS_SENT_FTREQ ||
	    as->state == AS_SENT_ASSOC ||
	    as->state == AS_REASSOC_RETRY ||
	    (ASSOC_RECREATE_ENAB(wlc->pub) &&
	     (as->state == AS_RECREATE_WAIT_RCV_BCN ||
	      as->state == AS_ASSOC_VERIFY))) {
		if (!wl_del_timer(wlc->wl, as->timer)) {
			as->rt = FALSE;
			callbacks ++;
		}
	}
#if (defined(BCMCCX) || defined(BCMSUP_PSK)) && defined(BCMINTSUP)
	else if (SUP_ENAB(wlc->pub) && (as->state == AS_WAIT_PASSHASH))
		callbacks += wlc_sup_down(wlc->idsup, cfg);
#endif // endif

#if defined(EXT_STA) || defined(MACOSX)
	if (WLEXTSTA_ENAB(wlc->pub) || is_macos) {
		is_recreate = (ASSOC_RECREATE_ENAB(wlc->pub) &&
		               as->type == AS_RECREATE &&
		               (as->state == AS_RECREATE_WAIT_RCV_BCN ||
		                as->state == AS_ASSOC_VERIFY));
		if (!is_recreate) {
			if (as->state > AS_SCAN && as->state < AS_WAIT_DISASSOC) {
				/* indicate association completion */
				/* N.B.: assoc_state check passed through assoc_status parameter */
				wlc_assoc_complete(cfg, WLC_E_STATUS_ABORT, &target_bss->BSSID,
					as->state > AS_WAIT_TX_DRAIN && as->state < AS_WAIT_RCV_BCN,
					as->type != AS_ASSOCIATION,
					WLC_DOT11_BSSTYPE(target_bss->infra));
			}

			/* indicate connection or roam completion */
			wlc_assoc_done(cfg, WLC_E_STATUS_ABORT);
			do_cmplt = TRUE;

#if defined(BCMCCX) && defined(CCX_SDK)
			if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx) &&
			    (as->state >= AS_JOIN_START && as->state < AS_SENT_AUTH_1)) {
				/* indicate CCX association abort */
				wlc_bss_mac_event(wlc, cfg, WLC_E_CCX_ASSOC_ABORT, NULL,
					WLC_E_STATUS_ABORT, 0, 0, 0, 0);
			}
#endif /* BCMCCX && CCX_SDK */
		}
	}
	else
#endif /* EXT_STA || MACOSX */
	/* WES FIXME: why is this wrapped with == AS_ASSOCIATTION? Seems like the AUTH
	 * cases would be fine if roams usually send auth, and the final event has a
	 * check for AS state.  Maybe it is just the last check that should go. Or maybe
	 * we should send up roam events?
	 */
	if (as->type == AS_ASSOCIATION) {
		if (as->state == AS_SENT_AUTH_1 ||
			as->state == AS_SENT_AUTH_3 ||
			as->state == AS_SENT_FTREQ) {
			wlc_auth_complete(cfg, WLC_E_STATUS_ABORT, &target_bss->BSSID,
			                  0, cfg->auth);
		} else if (as->state == AS_SENT_ASSOC ||
		           as->state == AS_REASSOC_RETRY) {
			wlc_assoc_complete(cfg, WLC_E_STATUS_ABORT, &target_bss->BSSID,
			                   0, as->type != AS_ASSOCIATION, 0);
		}
		/* Indicate connection completion */
		wlc_assoc_done(cfg, WLC_E_STATUS_ABORT);
		do_cmplt = TRUE;
	}

	if (curr_bandunit != (int)wlc->band->bandunit) {
		WL_INFORM(("wl%d: wlc->band changed since entering %s\n",
		   WLCWLUNIT(wlc), __FUNCTION__));
		WL_INFORM(("wl%d: %s: curr_bandunit = %d, wlc->band->bandunit = %d\n",
		   WLCWLUNIT(wlc), __FUNCTION__, curr_bandunit, wlc->band->bandunit));
	}
	/* Use wlc_scbfindband here in case wlc->band has changed since entering this function */
	/*
	 * When roaming, function wlc_assoc_done() called above can change the wlc->band *
	 * if AP and APSTA are both enabled. *
	 * In such a case, we will not be able to locate the correct scb entry if we use *
	 * wlc_scbfind() instead of wlc_scbfindband(). *
	 */
	if ((scb = wlc_scbfindband(wlc, cfg, &target_bss->BSSID, curr_bandunit)))
		/* Clear pending bits */
		wlc_scb_clearstatebit(scb, PENDING_AUTH | PENDING_ASSOC);

	wlc_roam_release_flow_cntrl(cfg);

	if (!do_cmplt) {
		wlc_bss_list_free(wlc, wlc->as_info->join_targets);
		wlc->as_info->join_targets_last = 0;

		as->type = AS_NONE;
		wlc_assoc_change_state(cfg, AS_IDLE);

		/* APSTA: complete any deferred AP bringup */
		if (AP_ENAB(wlc->pub) && APSTA_ENAB(wlc->pub))
			wlc_restart_ap(wlc->ap);
	}
	/* check for join data block, if set, clear it */
	if (wlc->block_datafifo & DATA_BLOCK_JOIN) {
		wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, 0);
	}
cleanup_assoc_req:
	/* ensure the assoc_req list no longer has ref to this bsscfg */
	/* sometimes next op is wlc_bsscfg_free */
	if (wlc_assoc_req_remove_entry(wlc, cfg) == BCME_OK) {
		WL_ASSOC(("wl%d.%d: %s: assoc req entry removed\n",
			WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
	}
#ifdef WLWNM
	if (WLWNM_ENAB(wlc->pub)) {
		wlc_wnm_bsstrans_reset_pending_join(wlc);
	}
#endif /* WLWNM */
	return callbacks;
} /* wlc_assoc_abort */

void
wlc_assoc_init(wlc_bsscfg_t *cfg, uint type)
{
	wlc_assoc_t *as = cfg->assoc;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	wlc_info_t *wlc = cfg->wlc;

	ASSERT(type == AS_ROAM || type == AS_ASSOCIATION || type == AS_RECREATE);

	WL_ASSOC(("wl%d.%d: %s: assoc state machine init to assoc->type %d %s\n",
	          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__, type, WLCASTYPEN(type)));
#endif /* BCMDBG || WLMSG_ASSOC */

	as->type = type;
	as->flags = 0;

#ifdef PSTA
	/* Clean up the proxy STAs before starting scan */
	if (PSTA_ENAB(cfg->wlc->pub) && (type == AS_ROAM) &&
	    (cfg == wlc_bsscfg_primary(cfg->wlc)))
		wlc_psta_disassoc_all(cfg->wlc->psta);
#endif /* PSTA */

#ifdef WL_BCN_COALESCING
	wlc_bcn_clsg_disable(cfg->wlc->bc, BCN_CLSG_ASSOC_MASK, BCN_CLSG_ASSOC_MASK);
#endif /* WL_BCN_COALESCING */

	return;
}

void
wlc_assoc_change_state(wlc_bsscfg_t *cfg, uint newstate)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_phy_t *pi = WLC_PI(wlc);
	wlc_assoc_t *as = cfg->assoc;
	uint oldstate = as->state;

#if defined(WLRSDB) && defined(WL_MODESW)
		wlc_bsscfg_t *icfg;
		int index;
		uint8 chip_mode;
#endif /* WLRSDB && WL_MODESW */

	if (newstate > AS_LAST_STATE) {
		WL_ERROR(("wl%d.%d: %s: out of bounds assoc state %d\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__, newstate));
		ASSERT(0);
	}

	if (newstate == oldstate)
		return;

	WL_ASSOC_LT(("wl%d.%d: wlc_assoc_change_state: change assoc_state from %s to %s\n",
	          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
	          wlc_as_st_name(oldstate), wlc_as_st_name(newstate)));

#ifdef WL_PWRSTATS
	/* Update connect time for primary infra sta only */
	if ((cfg == wlc->cfg) &&
		newstate == AS_JOIN_START)
		wlc_pwrstats_connect_start(wlc->pwrstats);

	/* Connection is over on port open and AS_IDLE OR assoc failed and AS_IDLE */
	if ((cfg == wlc->cfg) && cfg->BSS &&
		newstate == AS_IDLE && (wlc_portopen(cfg) || !cfg->associated))
		wlc_connect_time_upd(wlc);
#endif /* WL_PWRSTATS */

	if (newstate == AS_JOIN_START) {
		wlc->ebd->assoc_start_time = OSL_SYSUPTIME();
		wlc->ebd->earliest_offset = 0;
		wlc->ebd->detect_done = FALSE;
	}

	as->state = newstate;

#ifdef WLOFFLD
	if (WLOFFLD_CAP(cfg->wlc) && (cfg == cfg->wlc->cfg)) {
		if ((as->state == AS_WAIT_RCV_BCN) ||
			(as->state == AS_RECREATE_WAIT_RCV_BCN) ||
			(as->state == AS_SYNC_RCV_BCN)) {
				WL_ERROR(("%s: disabling offloads to receive bcn, as->state %d\n",
					__FUNCTION__, as->state));
				wlc_ol_disable(cfg->wlc->ol, cfg);
			}
	}
#endif /* WLOFFLD */

	/* optimization: nothing to be done between AS_IDLE and AS_CAN_YIELD() states */
	if ((oldstate == AS_IDLE && AS_CAN_YIELD(cfg->BSS, newstate)) ||
	    (AS_CAN_YIELD(cfg->BSS, oldstate) && newstate == AS_IDLE)) {
		/* update wake control */
		wlc_set_wake_ctrl(wlc);
		return;
	}
#ifdef	CLIENT_CSA
	/* if radar detected during DFS re-entry logic, abort Association state
	 * machine and let wlc_try_join_start initiate join state machine from
	 * scan stage
	 */
	if (!BSSCFG_AP(cfg) && (MAP_ENAB(cfg) || DWDS_ENAB(cfg) ||
#if defined(WET) || defined(WET_DONGLE)
		(WET_ENAB(cfg->wlc)) ||
#endif /* WET || WET_DONGLE */
		FALSE) && (newstate == AS_DFS_CAC_FAIL)) {

		wlc_assoc_abort(cfg);
	}
#endif /* CLIENT_CSA */
#ifdef SLAVE_RADAR
	if (WL11H_STA_ENAB(wlc)) {
		bool from_radar = FALSE;
		bool to_radar = FALSE;
		wlc_bss_info_t *bi;
		bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
		/* Slave CAC begins */
		if (newstate == AS_DFS_CAC_START) {
			from_radar = (wlc_radar_chanspec(wlc->cmi,
					cfg->current_bss->chanspec) == TRUE);
			to_radar = wlc_radar_chanspec(wlc->cmi, bi->chanspec);
			if (wlc_dfs_get_radar(wlc->dfs) && (!from_radar && to_radar)) {
				cfg->pm->PM_oldvalue = cfg->pm->PM;
				wlc_set_pm_mode(wlc, PM_OFF, cfg);
				cfg->pm->PMmodeChangeDisabled = TRUE;
				wlc->mpc = FALSE;
				wlc_radio_mpc_upd(wlc);
			}
			wlc_set_dfs_cacstate(wlc->dfs, ON);
		}

		/* DFS state machine detected radar during CAC */
		if (newstate == AS_DFS_CAC_FAIL) {
			wlc_assoc_abort(cfg);
			wlc_roamscan_start(wlc->cfg, WLC_E_REASON_RADAR_DETECTED);
		}

		/* Notify DFS state machine to begin ISM */
		if (newstate == AS_DFS_ISM_INIT) {
			from_radar = (wlc_radar_chanspec(wlc->cmi,
					cfg->current_bss->chanspec) == TRUE);
			to_radar = wlc_radar_chanspec(wlc->cmi, bi->chanspec);
			if (wlc_dfs_get_radar(wlc->dfs) && ((!from_radar && to_radar))) {
				cfg->pm->PM_oldvalue = cfg->pm->PM;
				wlc_set_pm_mode(wlc, PM_OFF, cfg);
				cfg->pm->PMmodeChangeDisabled = TRUE;
				wlc->mpc = FALSE;
				wlc_radio_mpc_upd(wlc);
			}
			wlc_set_dfs_cacstate(wlc->dfs, ON);
		}
	}

#endif /* SLAVE_RADAR */
		/* transition from IDLE or from a state equavilent to IDLE. */
	if (oldstate == AS_IDLE || AS_CAN_YIELD(cfg->BSS, oldstate)) {
		/* move out of IDLE */
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_ASSOC, TRUE);

		/* add the new request on top, or move existing request back on top
		 * in case the abort operation has removed it
		 */
		wlc_assoc_req_add_entry(wlc, cfg, as->type, TRUE);
	}
	/* transition to IDLE or to a state equavilent to IDLE */
	else if ((newstate == AS_IDLE || AS_CAN_YIELD(cfg->BSS, newstate)) &&
	         wlc_assoc_req_remove_entry(wlc, cfg) == BCME_OK) {
		/* the current assoc process does no longer need these variables/states,
		 * reset them to prepare for the next one.
		 * - join targets
		 */
		wlc_bss_list_free(wlc, wlc->as_info->join_targets);

		/* move into IDLE */
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_ASSOC, FALSE);

		/* start the next request processing if any */
		wlc_assoc_req_process_next(wlc);
	}
#if defined(WLRSDB) && defined(WL_MODESW)
	else if (WLC_MODESW_ENAB(wlc->pub) && RSDB_ENAB(wlc->pub) &&
		(newstate == AS_MODE_SWITCH_START)) {
		chip_mode = WLC_RSDB_CURR_MODE(wlc);
		/* Upgrade the Chip mode from RSDB
		 * to either MIMO or 80p80 depending
		 * upon the current values of Nss and Bw.
		 */
		if (WLC_RSDB_DUAL_MAC_MODE(chip_mode)) {
			wlc_rsdb_upgrade_wlc(wlc);
		}
		/* Switch to RSDB if in 2X2 mode.
		 * The need for this switch is analysed
		 * by the caller. We just need to switch
		 * the current mode to RSDB
		 */
		else if (WLC_RSDB_SINGLE_MAC_MODE(chip_mode)) {
			wlc_rsdb_downgrade_wlc(wlc);
		}
	}
	else if (RSDB_ENAB(wlc->pub) && (newstate == AS_MODE_SWITCH_COMPLETE)) {
		WL_MODE_SWITCH(("wl:%d.%d MODE SWITCH SUCCESS..\n", WLCWLUNIT(wlc),
			cfg->_idx));
		wlc_assoc_change_state(cfg, AS_SCAN);
		wlc_join_attempt(cfg);
	}
	else if (RSDB_ENAB(wlc->pub) && (newstate == AS_MODE_SWITCH_FAILED)) {
		FOREACH_BSS(wlc, index, icfg) {
			wlc_modesw_clear_context(wlc->modesw, icfg);
		}
		wlc_join_attempt(cfg);
	}
#endif /* WLRSDB && WL_MODESW */

} /* wlc_assoc_change_state */

/** given the current chanspec, select a non-radar chansepc */
static chanspec_t
wlc_sradar_ap_select_channel(wlc_info_t *wlc, chanspec_t curr_chanspec)
{
	uint rand_idx;
	int listlen, noradar_listlen = 0, i;
	chanspec_t newchspec = 0;
	const char *abbrev = wlc_channel_country_abbrev(wlc->cmi);
	wl_uint32_list_t *list, *noradar_list = NULL;
	bool bw20 = FALSE, ch2g = FALSE;

	/* if curr_chanspec is non-radar, just return it and do nothing */
	if (!wlc_radar_chanspec(wlc->cmi, curr_chanspec)) {
		return (curr_chanspec);
	}

	/* use current chanspec to determine which valid */
	/* channels to look for */
	if (CHSPEC_IS2G(curr_chanspec) || (curr_chanspec == 0)) {
		ch2g = TRUE;
	}
	if (CHSPEC_IS5G(curr_chanspec) || (curr_chanspec == 0)) {
		ch2g = FALSE;
	}
	if (CHSPEC_IS20(curr_chanspec) || (curr_chanspec == 0)) {
		bw20 = TRUE;
	}
	if (CHSPEC_IS40(curr_chanspec) || (curr_chanspec == 0)) {
		bw20 = FALSE;
	}
	/* allocate memory for list */
	listlen =
		OFFSETOF(wl_uint32_list_t, element)
		+ sizeof(list->element[0]) * MAXCHANNEL;

	if ((list = MALLOC(wlc->osh, listlen)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate list\n",
		  wlc->pub->unit, __FUNCTION__));
	} else {
		/* get a list of valid channels */
		list->count = 0;
		wlc_get_valid_chanspecs(wlc->cmi, list,
			(bw20 ? WL_CHANSPEC_BW_20 : WL_CHANSPEC_BW_40), ch2g, abbrev);
	}

	/* This code builds a non-radar channel list out of the valid list */
	/* and picks one randomly (preferred option) */
	if (list && list->count) {
		/* build a noradar_list */
		noradar_listlen =
			OFFSETOF(wl_uint32_list_t, element) +
			sizeof(list->element[0]) * list->count;

		if ((noradar_list = MALLOC(wlc->osh, noradar_listlen)) == NULL) {
			WL_ERROR(("wl%d: %s: failed to allocate noradar_list\n",
			  wlc->pub->unit, __FUNCTION__));
		} else {
			noradar_list->count = 0;
			for (i = 0; i < (int)list->count; i++) {
				if (!wlc_radar_chanspec(wlc->cmi,
					(chanspec_t)list->element[i])) {
					/* add to noradar_list */
					noradar_list->element[noradar_list->count++]
						= list->element[i];
				}
			}
		}
		if (noradar_list && noradar_list->count) {
			/* randomly pick a channel in noradar_list */
			rand_idx = R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random)
				% noradar_list->count;
			newchspec = (chanspec_t)noradar_list->element[rand_idx];
		}
	}
	if (list) {
		/* free list */
		MFREE(wlc->osh, list, listlen);
	}
	if (noradar_list) {
		/* free noradar_list */
		MFREE(wlc->osh, noradar_list, noradar_listlen);
	}

	return (newchspec);
} /* wlc_sradar_ap_select_channel */

/* checks for sradar enabled AP. */
/* if one found, check current_bss chanspec */
/* if chanspec on radar channel, randomly select non-radar channel */
/* and move to it */
static void
wlc_sradar_ap_update(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg;
	int idx;
	chanspec_t newchspec, chanspec = 0;
	bool move_ap = FALSE;

	/* See if we have a sradar ap on radar channel */
	FOREACH_UP_AP(wlc, idx, cfg) {
		if (BSSCFG_SRADAR_ENAB(cfg) &&
		    wlc_radar_chanspec(wlc->cmi, cfg->current_bss->chanspec)) {
			/* use current chanspec to determine which valid */
			/* channels to look for */
			chanspec = cfg->current_bss->chanspec;
			/* set flag to move ap */
			move_ap = TRUE;
			/* only need to do this for first matching up ap */
			break;
		}
	}

	if (move_ap == FALSE) {
		/* no sradar ap on radar channel, do nothing */
		return;
	}

	/* find a non-radar channel to move to */
	/* XXX: MCHAN (This is assuming multiple AP's all on same channel)
	 * Can't have multiple AP's on different channels.
	 * If want multiple AP's on different channels, need to rework.
	 */
	newchspec = wlc_sradar_ap_select_channel(wlc, chanspec);
	/* if no non-radar channel found, disable sradar ap on radar channel */
	FOREACH_UP_AP(wlc, idx, cfg) {
		if (BSSCFG_SRADAR_ENAB(cfg) &&
		    wlc_radar_chanspec(wlc->cmi, cfg->current_bss->chanspec)) {
			if (newchspec) {
				/* This code performs a channel switch immediately */
				/* w/o sending csa action frame and csa IE in beacon, prb_resp */
				wlc_do_chanswitch(cfg, newchspec);
			} else {
				/* can't find valid non-radar channel */
				/* shutdown ap */
				WL_ERROR(("%s: no radar channel found, disable ap\n",
				          __FUNCTION__));
				wlc_bsscfg_disable(wlc, cfg);
			}
		}
	}
} /* wlc_sradar_ap_update */

/** update STA association state */
void
wlc_sta_assoc_upd(wlc_bsscfg_t *cfg, bool state)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_phy_t  *pi	= WLC_PI(wlc);
	int idx;
	wlc_bsscfg_t *bc;
	bool prev_associated = cfg->associated;

	cfg->associated = state;

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		if (state == FALSE) {
			wlc_mchan_delete_bss_chan_context(wlc, cfg);
		}
		/* If chan_context doesn't exist at this point, create one. */
		/* chan ctx may have wrong channel, modify ctx if needed */
		else {
			wlc_mchan_update_bss_chan_context(wlc, cfg, cfg->current_bss->chanspec,
			                                  TRUE);
		}
	}
#endif /* WLMCHAN */

	/* XXX need the old state as wlc_update_pmstate works only in associated state,
	 *  chance for more improvement
	 */
	cfg->associated = prev_associated;

	/* STA is no longer associated, reset related states.
	 */
	if (!state) {
		wlc_reset_pmstate(cfg);
	}
	/* STA is assoicated, set related states that could have been
	 * missing before cfg->associated is set...
	 */
	else {
		wlc_set_pmawakebcn(cfg, wlc->PMawakebcn);
#if defined(WLBSSLOAD_REPORT)
		if (BSSLOAD_REPORT_ENAB(wlc->pub)) {
			wlc_bssload_reset_saved_data(cfg);
		}
#endif /* defined(WLBSSLOAD_REPORT) */
	}

	cfg->associated = state;

	wlc->stas_associated = 0;
	wlc->ibss_bsscfgs = 0;
	FOREACH_AS_STA(wlc, idx, bc) {
	        wlc->stas_associated ++;
		if (!bc->BSS)
			wlc->ibss_bsscfgs ++;
	}

	wlc->pub->associated = (wlc->stas_associated != 0 || wlc->aps_associated != 0);

	wlc_phy_hold_upd(pi, PHY_HOLD_FOR_NOT_ASSOC,
	                 wlc->pub->associated ? FALSE : TRUE);

	wlc->stas_connected = wlc_stas_connected(wlc);

#ifdef WLTPC
	/* Reset TPC on disconnect with AP to avoid using incorrect values when
	 * we connect to non 11d AP next time
	 */
	if ((wlc->stas_connected == 0) && (wlc->aps_associated == 0)) {
		wlc_tpc_reset_all(wlc->tpc);
	}
#endif // endif

	wlc_ap_sta_onradar_upd(cfg);

	/* Update maccontrol PM related bits */
	wlc_set_ps_ctrl(cfg);

	/* change the watchdog driver */
	wlc_watchdog_upd(cfg, PS_ALLOWED(cfg));

	wlc_enable_btc_ps_protection(wlc, cfg, state);

	/* if no station associated and we have ap up, check channel */
	/* if radar channel, move off to non-radar channel */
	if (WL11H_ENAB(wlc) && AP_ACTIVE(wlc) && wlc->stas_associated == 0) {
		wlc_sradar_ap_update(wlc);
	}

#ifdef WLBTAMP
	/* notify PAL of possible channel change */
	if (BTA_ENAB(wlc->pub))
		wlc_bta_assoc_complete(wlc->bta, cfg);
#endif /* WLBTAMP */

#ifdef WL_MODESW
	/* Delete ModeSW Context */
	if (WLC_MODESW_ENAB(wlc->pub)) {
		wlc_bss_assoc_state_notif(wlc, cfg, cfg->assoc->type, cfg->assoc->state);
	}
#endif // endif

#ifdef WLRSDB
	if (RSDB_ENAB(wlc->pub))
		wlc_rsdb_update_active(wlc, NULL);
#endif /* WLRSDB */
}

static void
wlc_join_adopt_bss(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_roam_t *roam = cfg->roam;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint reason = 0;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char chanbuf[CHANSPEC_STR_LEN];
	char ssidbuf[SSID_FMT_BUF_LEN];
	wlc_format_ssid(ssidbuf, target_bss->SSID, target_bss->SSID_len);
#endif /* BCMDBG || WLMSG_ASSOC */

	if (target_bss->infra) {
		WL_ASSOC(("wl%d: JOIN: join BSS \"%s\" on chanspec %s\n", WLCWLUNIT(wlc),
			ssidbuf, wf_chspec_ntoa_ex(target_bss->chanspec, chanbuf)));
	} else {
		WL_ASSOC(("wl%d: JOIN: join IBSS \"%s\" on chanspec %s\n", WLCWLUNIT(wlc),
			ssidbuf, wf_chspec_ntoa_ex(target_bss->chanspec, chanbuf)));
	}

	WL_SRSCAN(("join complete: bssid %02x:%02x",
		target_bss->BSSID.octet[4], target_bss->BSSID.octet[5]));
	WL_ASSOC(("join complete: bssid %02x:%02x\n",
		target_bss->BSSID.octet[4], target_bss->BSSID.octet[5]));

	wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, 0);

	roam->RSSIref = 0; /* this will be reset on the first incoming frame */

	if (as->type == AS_ASSOCIATION) {
		roam->reason = WLC_E_REASON_INITIAL_ASSOC;
		WL_ASSOC(("wl%d: ROAM: roam_reason cleared to 0x%x\n",
			wlc->pub->unit, WLC_E_REASON_INITIAL_ASSOC));

		if (roam->reassoc_param != NULL) {
			MFREE(wlc->osh, roam->reassoc_param, roam->reassoc_param_len);
			roam->reassoc_param = NULL;
		}
	}

	roam->prev_rssi = target_bss->RSSI;
	WL_ASSOC(("wl%d: ROAM: initial rssi %d\n", WLCWLUNIT(wlc), roam->prev_rssi));
	WL_SRSCAN(("wl%d: ROAM: initial rssi %d\n", WLCWLUNIT(wlc), roam->prev_rssi));

	/* Reset split roam scan state so next scan will be on cached channels. */
	roam->split_roam_phase = 0;
	roam->roam_chn_cache_locked = FALSE;
#ifdef WLABT
	if (WLABT_ENAB(wlc->pub) && cfg->roam->prev_bcn_timeout != 0) {
		cfg->roam->bcn_timeout = cfg->roam->prev_bcn_timeout;
		cfg->roam->prev_bcn_timeout = 0;
		WL_ASSOC(("Reset bcn_timeout to %d\n", cfg->roam->bcn_timeout));
	}
#endif /* WLABT */
#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && target_bss->infra)
		wlc_ccx_on_join_adopt_bss(wlc->ccx);
#endif /* BCMCCX */

#ifdef WLMCNX
#ifndef WLTSFSYNC
	/* stop TSF adjustment - do it before setting BSSID to prevent ucode
	 * from adjusting TSF before we program the TSF block
	 */
	if (MCNX_ENAB(wlc->pub) && cfg->BSS)
		wlc_skip_adjtsf(wlc, TRUE, cfg, -1, WLC_BAND_ALL);
#endif // endif
#endif /* WLMCNX */

	/* suspend the MAC and join the BSS */
	wlc_suspend_mac_and_wait(wlc);
	wlc_BSSinit(wlc, target_bss, cfg, WLC_BSS_JOIN);
	wlc_enable_mac(wlc);

	wlc_sta_assoc_upd(cfg, TRUE);
	WL_RTDC(wlc, "wlc_join_adopt_bss: associated", 0, 0);

	/* Apply the STA AC params sent by AP */
	if (BSS_WME_AS(wlc, cfg)) {
		WL_ASSOC(("wl%d.%d: adopting WME AC params...\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		wlc_edcf_acp_apply(wlc, cfg, TRUE);
	}

#ifdef WLMCNX
	/* init multi-connection assoc states */
	if (MCNX_ENAB(wlc->pub) && cfg->BSS)
		wlc_mcnx_assoc_upd(wlc->mcnx, cfg, TRUE);
#endif // endif

	if (N_ENAB(wlc->pub) && COEX_ENAB(wlc))
		wlc_ht_obss_scan_reset(cfg);

	WL_APSTA_UPDN(("wl%d: Reporting link up on config 0 for STA joining a BSS\n",
	            WLCWLUNIT(wlc)));

#if defined(WLFBT)
	if (BSSCFG_IS_FBT(cfg) && (bcmwpa_is_rsn_auth(cfg->WPA_auth)) &&
		(cfg->auth_atmptd == DOT11_FAST_BSS)) {
		reason = WLC_E_REASON_BSSTRANS_REQ;  /* any non-zero reason is okay here */
	}
#endif /* WLFBT */

	wlc_link(wlc, TRUE, &cfg->BSSID, cfg, reason);

#ifdef WLBTAMP
	if (BSS_BTA_ENAB(wlc, cfg)) {
		struct scb *scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);
		if (scb != NULL) {
			/*
			 * BT-AMP join post-processing:
			 * on success, configure peer for WDS
			 * generate Physical Link Complete
			 * [N.B.: don't wait for next beacon]
			 */
			wlc_bta_join_complete(wlc->bta, scb, 0);
		} else {
			wlc_bta_assoc_status_upd(wlc->bta, cfg, WLC_BTA_AP_ASSOC);
		}
	}
#endif /* WLBTAMP */

	wlc_assoc_change_state(cfg, AS_JOIN_ADOPT);
	wlc_bss_assoc_state_notif(wlc, cfg, cfg->assoc->type, cfg->assoc->state);

#ifdef DWDS
	if (DWDS_ENAB(cfg) || MAP_ENAB(cfg)) {
		struct scb *scb;

		scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
			CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));

		if ((scb != NULL) && (SCB_DWDS_CAP(scb) || SCB_MAP_CAP(scb))) {
			/*
			 * make this scb to do 4-addr data frame from now.
			 * SCB_DWDS_CAP | SCB_MAP_CAP being TRUE implies that DWDS|MAP is enabled.
			 */
			SCB_A4_DATA_ENABLE(scb);
			SCB_DWDS_ACTIVATE(scb);
		}
	}
#endif /* DWDS */

	if (target_bss->infra) {
		int beacon_wait_time;

		/* infrastructure join needs a BCN for TBTT coordination */
		WL_ASSOC(("wl%d: JOIN: BSS case...waiting for the next beacon...\n",
		            WLCWLUNIT(wlc)));
		wlc_assoc_change_state(cfg, AS_WAIT_RCV_BCN);

		/* Start the timer for beacon receive timeout handling to ensure we move
		 * out of AS_WAIT_RCV_BCN state in case we never receive the first beacon
		 */
		beacon_wait_time = as->recreate_bi_timeout *
			(target_bss->beacon_period * DOT11_TU_TO_US) / 1000;

		/* Validate beacon_wait_time, usually it will be 1024 with beacon interval 100TUs */
		beacon_wait_time = beacon_wait_time > 10000 ? 10000 :
			beacon_wait_time < 1000 ? 1024 : beacon_wait_time;

		wlc_assoc_timer_del(wlc, cfg);
		wl_add_timer(wlc->wl, as->timer, beacon_wait_time, 0);
	}

	wlc_roam_release_flow_cntrl(cfg);

	wlc_bsscfg_SSID_set(cfg, target_bss->SSID, target_bss->SSID_len);
	wlc_roam_set_env(cfg, wlc->as_info->join_targets->count);

	/* Reset roam profile for new association */
	wlc_roam_prof_update(wlc, cfg, TRUE);

#ifdef WLLED
	wlc_led_event(wlc->ledh);
#endif // endif
} /* wlc_join_adopt_bss */

/** return true if we support all of the target basic rates */
static bool
wlc_join_basicrate_supported(wlc_info_t *wlc, wlc_rateset_t *rs, int band)
{
	uint i;
	uint8 rate;
	bool only_cck = FALSE;

#ifdef FULL_LEGACY_B_RATE_CHECK
	wlcband_t* pband;

	/* determine if we need to do addtional gmode == GMODE_LEGACY_B checking of rates */
	if (band == WLC_BAND_2G) {
		if (BAND_2G(wlc->band->bandtype))
			pband = wlc->band;
		else if (NBANDS(wlc) > 1 && BAND_2G(wlc->bandstate[OTHERBANDUNIT(wlc)]->bandtype))
			pband = wlc->bandstate[OTHERBANDUNIT(wlc)];
		else
			pband = NULL;

		if (pband && pband->gmode == GMODE_LEGACY_B)
			only_cck = TRUE;
	}
#endif // endif
	for (i = 0; i < rs->count; i++)
		if (rs->rates[i] & WLC_RATE_FLAG) {
			rate = rs->rates[i] & RATE_MASK;
			if (!wlc_valid_rate(wlc, rate, band, FALSE))
				return (FALSE);
			if (only_cck && !IS_CCK(rate))
				return (FALSE);
		}
	return (TRUE);
}

void
wlc_join_recreate_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	uint8* tlvs;
	int tlvs_len;
	bcm_tlv_t *tim;

	/* In a standard join attempt this is done in wlc_join_BSS() as the
	 * process begins. For a recreate, we wait until we have confirmation that
	 * we are in the presence of our former AP before unmuting a channel.
	 */
	if (wlc_quiet_chanspec(wlc->cmi, current_bss->chanspec)) {
		wlc_clr_quiet_chanspec(wlc->cmi, current_bss->chanspec);
		if (WLC_BAND_PI_RADIO_CHANSPEC == current_bss->chanspec) {
			wlc_mute(wlc, OFF, 0);
		}
	}

	/* In a standard join attempt this is done on the WLC_E_ASSOC/REASSOC event to
	 * allow traffic to flow to the newly associated AP. For a recreate, we wait until
	 * we have confirmation that we are in the presence of our former AP.
	 */
	wlc_block_datafifo(wlc, DATA_BLOCK_JOIN, 0);

	/* TSF adoption and PS transition is done just as in a standard association in
	 * wlc_join_complete()
	 */
	wlc_tsf_adopt_bcn(cfg, wrxh, plcp, bcn);

	tlvs = (uint8*)bcn + DOT11_BCN_PRB_LEN;
	tlvs_len = bcn_len - DOT11_BCN_PRB_LEN;

	tim = NULL;
	if (cfg->BSS) {
		/* adopt DTIM period for PS-mode support */
		tim = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_TIM_ID);

		if (tim) {
			current_bss->dtim_period = tim->data[DOT11_MNG_TIM_DTIM_PERIOD];
#ifdef WLMCNX
			if (MCNX_ENAB(wlc->pub))
				; /* empty */
			else
#endif // endif
			if (cfg == wlc->cfg)
				wlc_write_shm(wlc, M_DOT11_DTIMPERIOD, current_bss->dtim_period);
			wlc_update_bcn_info(cfg, TRUE);
#ifdef WLMCNX
			if (MCNX_ENAB(wlc->pub))
				wlc_mcnx_dtim_upd(wlc->mcnx, cfg, TRUE);
#endif // endif
		}else { /*  (!tim) illed AP, prevent going to power-save mode */
			wlc_set_pmoverride(cfg, TRUE);
		}
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		/* Cancel the suppress-ssid-after-hibernate flag for Vista */
		wlc->scan_suppress_ssid = FALSE;
	}
#endif /* EXT_STA */

	if (cfg->BSS) {
		/* when recreating an association, send a null data to the AP to verify
		 * that we are still associated and wait a generous amount amount of time
		 * to allow the AP to send a disassoc/deauth if it does not consider us
		 * associated.
		 */
		if (!wlc_sendnulldata(wlc, cfg, &cfg->current_bss->BSSID, 0, 0,
			PRIO_8021D_BE, NULL, NULL))
			WL_ERROR(("wl%d: %s: wlc_sendnulldata() failed\n",
			          wlc->pub->unit, __FUNCTION__));

		/* kill the beacon timeout timer and reset for association verification */
		wlc_assoc_timer_del(wlc, cfg);
		wl_add_timer(wlc->wl, as->timer, as->verify_timeout, FALSE);

		wlc_assoc_change_state(cfg, AS_ASSOC_VERIFY);
	} else {
		/* for an IBSS recreate, we are done */
		as->type = AS_NONE;
		wlc_assoc_change_state(cfg, AS_IDLE);
	}
} /* wlc_join_recreate_complete */

static void
wlc_merge_bcn_prb(wlc_info_t *wlc,
	struct dot11_bcn_prb *p1, int p1_len,
	struct dot11_bcn_prb *p2, int p2_len,
	struct dot11_bcn_prb **merged, int *merged_len)
{
	uint8 *tlvs1, *tlvs2;
	int tlvs1_len, tlvs2_len;

	*merged = NULL;
	*merged_len = 0;

	if (p1) {
		ASSERT(p1_len >= DOT11_BCN_PRB_LEN);
		/* fixup for non-assert builds */
		if (p1_len < DOT11_BCN_PRB_LEN)
			p1 = NULL;
	}
	if (p2) {
		ASSERT(p2_len >= DOT11_BCN_PRB_LEN);
		/* fixup for non-assert builds */
		if (p2_len < DOT11_BCN_PRB_LEN)
			p2 = NULL;
	}

	/* check for simple cases of one or the other of the source packets being null */
	if (p1 == NULL && p2 == NULL) {
		return;
	} else if (p2 == NULL) {
		*merged = (struct dot11_bcn_prb *) MALLOC(wlc->osh, p1_len);
		if (*merged != NULL) {
			*merged_len = p1_len;
			bcopy((char*)p1, (char*)*merged, p1_len);
		}
		return;
	} else if (p1 == NULL) {
		*merged = (struct dot11_bcn_prb *) MALLOC(wlc->osh, p2_len);
		if (*merged != NULL) {
			*merged_len = p2_len;
			bcopy((char*)p2, (char*)*merged, p2_len);
		}
		return;
	}

	/* both source packets are present, so do the merge work */
	tlvs1 = (uint8*)p1 + DOT11_BCN_PRB_LEN;
	tlvs1_len = p1_len - DOT11_BCN_PRB_LEN;

	tlvs2 = (uint8*)p2 + DOT11_BCN_PRB_LEN;
	tlvs2_len = p2_len - DOT11_BCN_PRB_LEN;

	/* allocate a buffer big enough for the merged ies */
	*merged_len = DOT11_BCN_PRB_LEN + wlc_merged_ie_len(wlc, tlvs1, tlvs1_len, tlvs2,
		tlvs2_len);
	*merged = (struct dot11_bcn_prb *) MALLOC(wlc->osh, *merged_len);
	if (*merged == NULL) {
		*merged_len = 0;
		return;
	}

	/* copy the fixed portion of the second packet so the latest TSF, cap, etc is kept */
	bcopy(p2, *merged, DOT11_BCN_PRB_LEN);

	/* merge the ies from both packets */
	wlc_merge_ies(tlvs1, tlvs1_len, tlvs2, tlvs2_len, (uint8*)*merged + DOT11_BCN_PRB_LEN);
} /* wlc_merge_bcn_prb */

static int
wlc_merged_ie_len(wlc_info_t *wlc, uint8 *tlvs1, int tlvs1_len, uint8 *tlvs2, int tlvs2_len)
{
	bcm_tlv_t *tlv1 = (bcm_tlv_t*)tlvs1;
	bcm_tlv_t *tlv2 = (bcm_tlv_t*)tlvs2;
	int total;
	int len;

	/* treat an empty list or malformed list as empty */
	if (!bcm_valid_tlv(tlv1, tlvs1_len)) {
		tlv1 = NULL;
		tlvs1 = NULL;
	}
	if (!bcm_valid_tlv(tlv2, tlvs2_len)) {
		tlv2 = NULL;
		tlvs2 = NULL;
	}

	total = 0;

	len = tlvs2_len;
	while (tlv2) {
		total += TLV_HDR_LEN + tlv2->len;
		tlv2 = bcm_next_tlv(tlv2, &len);
	}

	len = tlvs1_len;
	while (tlv1) {
		if (!wlc_find_ie_match(tlv1, (bcm_tlv_t*)tlvs2, tlvs2_len))
			total += TLV_HDR_LEN + tlv1->len;

		tlv1 = bcm_next_tlv(tlv1, &len);
	}

	return total;
}

static bool
wlc_find_ie_match(bcm_tlv_t *ie, bcm_tlv_t *ies, int len)
{
	uint8 ie_len;

	ie_len = ie->len;

	while (ies) {
		if (ie_len == ies->len && !bcmp(ie, ies, (TLV_HDR_LEN + ie_len))) {
			return TRUE;
		}
		ies = bcm_next_tlv(ies, &len);
	}

	return FALSE;
}

static void
wlc_merge_ies(uint8 *tlvs1, int tlvs1_len, uint8 *tlvs2, int tlvs2_len, uint8* merge)
{
	bcm_tlv_t *tlv1 = (bcm_tlv_t*)tlvs1;
	bcm_tlv_t *tlv2 = (bcm_tlv_t*)tlvs2;
	int len;

	/* treat an empty list or malformed list as empty */
	if (!bcm_valid_tlv(tlv1, tlvs1_len)) {
		tlv1 = NULL;
		tlvs1 = NULL;
	}
	if (!bcm_valid_tlv(tlv2, tlvs2_len)) {
		tlv2 = NULL;
		tlvs2 = NULL;
	}

	/* copy in the ies from the second set */
	len = tlvs2_len;
	while (tlv2) {
		bcopy(tlv2, merge, TLV_HDR_LEN + tlv2->len);
		merge += TLV_HDR_LEN + tlv2->len;

		tlv2 = bcm_next_tlv(tlv2, &len);
	}

	/* merge in the ies from the first set */
	len = tlvs1_len;
	while (tlv1) {
		if (!wlc_find_ie_match(tlv1, (bcm_tlv_t*)tlvs2, tlvs2_len)) {
			bcopy(tlv1, merge, TLV_HDR_LEN + tlv1->len);
			merge += TLV_HDR_LEN + tlv1->len;
		}
		tlv1 = bcm_next_tlv(tlv1, &len);
	}
}

/** adopt IBSS parameters in cfg->target_bss */
void
wlc_join_adopt_ibss_params(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	/* adopt the IBSS parameters */
	wlc_join_adopt_bss(cfg);

	cfg->roam->time_since_bcn = 1;
	cfg->roam->bcns_lost = TRUE;

#ifdef WLMCNX
	/* use p2p ucode and support driver code... */
	if (MCNX_ENAB(wlc->pub)) {
		cfg->current_bss->dtim_period = 1;
		wlc_mcnx_dtim_upd(wlc->mcnx, cfg, TRUE);
	}
#endif // endif
}

void
wlc_adopt_dtim_period(wlc_bsscfg_t *cfg, uint8 dtim_period)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	current_bss->dtim_period = dtim_period;
#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub))
		; /* empty */
	else
#endif // endif
	if (cfg == wlc->cfg)
		wlc_write_shm(wlc, M_DOT11_DTIMPERIOD, current_bss->dtim_period);
	wlc_update_bcn_info(cfg, TRUE);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub))
		wlc_mcnx_dtim_upd(wlc->mcnx, cfg, TRUE);
#endif // endif
}

void
wlc_join_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint8* tlvs;
	int tlvs_len;
	bcm_tlv_t *tim;
	struct dot11_bcn_prb *merged;
	int merged_len;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_pm_st_t *pm = cfg->pm;

#ifdef DEBUG_TBTT
	wlc->bad_TBTT = FALSE;
#endif /* DEBUG_TBTT */

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && as->type != AS_ASSOCIATION) {
		uint32 tsf_l, tsf_h;
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		wlc->ccx->roaming_delay_tu = tsf_l - wlc->ccx->roaming_start_time;
		wlc->ccx->roaming_count++; /* free running count */
	}
#endif /* BCMCCX */

	wlc_tsf_adopt_bcn(cfg, wrxh, plcp, bcn);

	tlvs = (uint8*)bcn + DOT11_BCN_PRB_LEN;
	tlvs_len = bcn_len - DOT11_BCN_PRB_LEN;

	tim = NULL;
	if (target_bss->infra == 1) {
		/* adopt DTIM period for PS-mode support */
		tim = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_TIM_ID);
		if (tim) {
			wlc_adopt_dtim_period(cfg, tim->data[DOT11_MNG_TIM_DTIM_PERIOD]);
		}
		/* illed AP, prevent going to power-save mode */
		else {
			wlc_set_pmoverride(cfg, TRUE);
		}
	}
	else /* if (target_bss->infra == 0) */ {
		if (BSS_WME_ENAB(wlc, cfg)) {
			bcm_tlv_t *wme_ie = wlc_find_wme_ie(tlvs, tlvs_len);

			if (wme_ie) {
				cfg->flags |= WLC_BSSCFG_WME_ASSOC;
			}
		}
		wlc_join_adopt_ibss_params(wlc, cfg);
	}

	/* Merge the current saved probe response IEs with the beacon's in case the
	 * beacon is missing some info.
	 */
	wlc_merge_bcn_prb(wlc, current_bss->bcn_prb, current_bss->bcn_prb_len,
	                  bcn, bcn_len, &merged, &merged_len);

	/* save bcn's fixed and tagged parameters in current_bss */
	if (merged != NULL) {
		if (current_bss->bcn_prb)
			MFREE(wlc->osh, current_bss->bcn_prb, current_bss->bcn_prb_len);
		current_bss->bcn_prb = merged;
		current_bss->bcn_prb_len = (uint16)merged_len;
	} else {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
	}

	/* Grab and use the Country IE from the AP we are joining and override any Country IE
	 * we may have obtained from somewhere else.
	 */
	if ((BSS_WL11H_ENAB(wlc, cfg) || WLC_AUTOCOUNTRY_ENAB(wlc)) &&
	    (target_bss->infra == 1)) {
		wlc_cntry_adopt_country_ie(wlc->cntry, cfg, tlvs, tlvs_len);
	}

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && wlc->ccx->ccx_network)
		wlc_ccx_tx_pwr_ctl(wlc->ccx, tlvs, tlvs_len);
#endif /* BCMCCX */

	/* reset wd_run_flag */
	if (BSSCFG_STA(cfg))
		wlc->wd_run_flag = TRUE;

	/* If the PM2 Receive Throttle Duty Cycle feature is active, reset it */
	if (PM2_RCV_DUR_ENAB(cfg) && pm->PM == PM_FAST) {
		WL_RTDC(wlc, "wlc_join_complete: PMep=%02u AW=%02u",
			(pm->PMenabled ? 10 : 0) | pm->PMpending,
			(PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
		wlc_pm2_rcv_reset(cfg);
	}

	/* 802.11 PM can be entered now that we have synchronized with
	 * the BSS's TSF and DTIM count.
	 */
	if (PS_ALLOWED(cfg))
		wlc_set_pmstate(cfg, TRUE);
	/* sync/reset h/w */
	else {
		if (pm->PM == PM_FAST) {
			WL_RTDC(wlc, "wlc_join_complete: start srtmr, PMep=%02u AW=%02u",
			        (pm->PMenabled ? 10 : 0) | pm->PMpending,
			        (PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
			wlc_pm2_sleep_ret_timer_start(cfg);
		}
		wlc_set_ps_ctrl(cfg);
	}

	/* If assoc_state is not AS_IDLE, then this join event is completing a roam or a
	 * set_ssid operation.
	 * If assoc_state is AS_IDLE, then we are here due to an IBSS coalesce and there is
	 * no more follow on work.
	 */
	if (as->state != AS_IDLE) {
		/* send event to indicate final step in joining BSS */
		wlc_bss_mac_event(
			wlc, cfg, WLC_E_JOIN, &cfg->BSSID, WLC_E_STATUS_SUCCESS, 0,
			WLC_DOT11_BSSTYPE(target_bss->infra), 0, 0);
		wlc_assoc_done(cfg, WLC_E_STATUS_SUCCESS);
	}
	else {
		/* Upon IBSS coalescence, send ROAMING_START and ROAMING_COMPLETION events */
		if (WLEXTSTA_ENAB(wlc->pub)) {
			wlc_bss_mac_event(wlc, cfg, WLC_E_IBSS_COALESCE, &cfg->BSSID,
			                  WLC_E_STATUS_SUCCESS, WLC_E_STATUS_SUCCESS,
			                  WLC_DOT11_BSSTYPE(cfg->target_bss->infra),
			                  cfg->SSID, cfg->SSID_len);
		}
	}

#ifdef WLEXTLOG
	{
	char log_eabuf[ETHER_ADDR_STR_LEN];

	bcm_ether_ntoa(&cfg->BSSID, log_eabuf);

	WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_JOIN_COMPLETE_ID,
	           WL_LOG_LEVEL_ERR, 0, 0, log_eabuf);
	}
#endif // endif
} /* wlc_join_complete */

static int
wlc_join_pref_tlv_len(wlc_info_t *wlc, uint8 *pref, int len)
{
	if (len < TLV_HDR_LEN)
		return 0;

	switch (pref[TLV_TAG_OFF]) {
	case WL_JOIN_PREF_RSSI:
	case WL_JOIN_PREF_BAND:
	case WL_JOIN_PREF_RSSI_DELTA:
		return TLV_HDR_LEN + pref[TLV_LEN_OFF];
	case WL_JOIN_PREF_WPA:
		if (len < TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED) {
			WL_ERROR(("wl%d: mulformed WPA cfg in join pref\n", WLCWLUNIT(wlc)));
			return -1;
		}
		return TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED +
		        pref[TLV_HDR_LEN + WLC_JOIN_PREF_OFF_COUNT] * 12;
	default:
		WL_ERROR(("wl%d: unknown join pref type\n", WLCWLUNIT(wlc)));
		return -1;
	}

	return 0;
}

int
wlc_join_pref_parse(wlc_bsscfg_t *cfg, uint8 *pref, int len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	int tlv_pos, tlv_len;
	uint8 type;
	uint8 bits;
	uint8 start;
	uint i;
	uint8 f, c;
	uint p;
	uint8 band;

	WL_TRACE(("wl%d: wlc_join_pref_parse: pref len = %d\n", WLCWLUNIT(wlc), len));

	if (join_pref == NULL)
		return BCME_ERROR;

	band = join_pref->band;
	bzero(join_pref, sizeof(wlc_join_pref_t));
	join_pref->band = band;
	if (!len) {
		wlc_join_pref_band_upd(cfg);
		return 0;
	}

	/*
	* Each join target carries a 'weight', consisting of a number
	* of info such as akm and cipher. The weight is represented by
	* a N bit number. The bigger the number is the more likely the
	* target becomes the join candidate. Each info in the weight
	* is called a field, which is made of a type defined in wlioctl.h
	* and a bit offset assigned by parsing user-supplied "join_pref"
	* iovar. The fields are ordered from the most significant field to
	* the least significant field.
	*/
	/* count # tlvs # bits first */
	for (tlv_pos = 0, f = 0, start = 0, p = 0;
	     (tlv_len = wlc_join_pref_tlv_len(wlc, &pref[tlv_pos],
	                                      len - tlv_pos)) >= TLV_HDR_LEN &&
	             tlv_pos + tlv_len <= len;
	     tlv_pos += tlv_len) {
		type = pref[tlv_pos + TLV_TAG_OFF];
		if (p & WLCTYPEBMP(type)) {
			WL_ERROR(("wl%d: multiple join pref type %d\n", WLCWLUNIT(wlc), type));
			goto err;
		}
		switch (type) {
		case WL_JOIN_PREF_RSSI:
			bits = WLC_JOIN_PREF_BITS_RSSI;
			break;
		case WL_JOIN_PREF_WPA:
			bits = WLC_JOIN_PREF_BITS_WPA;
			break;
		case WL_JOIN_PREF_BAND:
			bits = WLC_JOIN_PREF_BITS_BAND;
			break;
		case WL_JOIN_PREF_RSSI_DELTA:
			bits = WLC_JOIN_PREF_BITS_RSSI_DELTA;
			break;
		default:
			WL_ERROR(("wl%d: invalid join pref type %d\n", WLCWLUNIT(wlc), type));
			goto err;
		}
		f++;
		start += bits;
		p |= WLCTYPEBMP(type);
	}
	/* rssi field is mandatory! */
	if (!(p & WLCTYPEBMP(WL_JOIN_PREF_RSSI))) {
		WL_ERROR(("wl%d: WL_JOIN_PREF_RSSI (type %d) is not present\n",
			WLCWLUNIT(wlc), WL_JOIN_PREF_RSSI));
		goto err;
	}

	/* RSSI Delta is not maintained in the join_pref fields */
	if (p & WLCTYPEBMP(WL_JOIN_PREF_RSSI_DELTA))
		f--;

	/* other sanity checks */
	if (start > sizeof(wlc->as_info->join_targets->ptrs[0]->RSSI) * 8) {
		WL_ERROR(("wl%d: too many bits %d max %u\n", WLCWLUNIT(wlc), start,
			(uint)sizeof(wlc->as_info->join_targets->ptrs[0]->RSSI) * 8));
		goto err;
	}
	if (f > MAXJOINPREFS) {
		WL_ERROR(("wl%d: too many tlvs/prefs %d\n", WLCWLUNIT(wlc), f));
		goto err;
	}
	WL_ASSOC(("wl%d: wlc_join_pref_parse: total %d fields %d bits\n", WLCWLUNIT(wlc), f,
		start));
	/* parse user-supplied join pref list */
	/* reverse the order so that most significant pref goes to the last entry */
	join_pref->fields = f;
	join_pref->prfbmp = p;
	for (tlv_pos = 0;
	     (tlv_len = wlc_join_pref_tlv_len(wlc, &pref[tlv_pos],
	                                      len - tlv_pos)) >= TLV_HDR_LEN &&
	             tlv_pos + tlv_len <= len;
	     tlv_pos += tlv_len) {
		bits = 0;
		switch ((type = pref[tlv_pos + TLV_TAG_OFF])) {
		case WL_JOIN_PREF_RSSI:
			bits = WLC_JOIN_PREF_BITS_RSSI;
			break;
		case WL_JOIN_PREF_WPA:
			c = pref[tlv_pos + TLV_HDR_LEN + WLC_JOIN_PREF_OFF_COUNT];
			bits = WLC_JOIN_PREF_BITS_WPA;
			/* sanity check */
			if (c > WLCMAXCNT(bits)) {
				WL_ERROR(("wl%d: two many wpa configs %d max %d\n",
					WLCWLUNIT(wlc), c, WLCMAXCNT(bits)));
				goto err;
			}
			else if (!c) {
				WL_ERROR(("wl%d: no wpa config specified\n", WLCWLUNIT(wlc)));
				goto err;
			}
			/* user-supplied list is from most favorable to least favorable */
			/* reverse the order so that the bigger the index the more favorable the
			 * config is
			 */
			for (i = 0; i < c; i ++)
				bcopy(&pref[tlv_pos + TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED +
				            i * sizeof(join_pref->wpa[0])],
				      &join_pref->wpa[c - 1 - i],
				      sizeof(join_pref->wpa[0]));
			join_pref->wpas = c;
			break;
		case WL_JOIN_PREF_BAND:
			bits = WLC_JOIN_PREF_BITS_BAND;
			/* honor use what WLC_SET_ASSOC_PREFER says first */
			if (pref[tlv_pos + TLV_HDR_LEN + WLC_JOIN_PREF_OFF_BAND] ==
				WLJP_BAND_ASSOC_PREF)
				break;
			/* overwrite with this setting */
			join_pref->band = pref[tlv_pos + TLV_HDR_LEN + WLC_JOIN_PREF_OFF_BAND];
			break;
		case WL_JOIN_PREF_RSSI_DELTA:
			bits = WLC_JOIN_PREF_BITS_RSSI_DELTA;
			cfg->join_pref_rssi_delta.rssi = pref[tlv_pos + TLV_HDR_LEN +
			                                 WLC_JOIN_PREF_OFF_DELTA_RSSI];

			cfg->join_pref_rssi_delta.band = pref[tlv_pos + TLV_HDR_LEN +
			                                 WLC_JOIN_PREF_OFF_BAND];
			break;
		}
		if (!bits)
			continue;

		f--;
		start -= bits;

		join_pref->field[f].type = type;
		join_pref->field[f].start = start;
		join_pref->field[f].bits = bits;
		WL_ASSOC(("wl%d: wlc_join_pref_parse: added field %s entry %d offset %d bits %d\n",
			WLCWLUNIT(wlc), WLCJOINPREFN(type), f, start, bits));
	}

	/* band preference can be from a different source */
	if (!(p & WLCTYPEBMP(WL_JOIN_PREF_BAND)))
		wlc_join_pref_band_upd(cfg);

	return 0;

	/* error handling */
err:
	band = join_pref->band;
	bzero(join_pref, sizeof(wlc_join_pref_t));
	join_pref->band = band;
	return BCME_ERROR;
} /* wlc_join_pref_parse */

int
wlc_join_pref_build(wlc_bsscfg_t *cfg, uint8 *pref, int len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	int total, wpalen = 0;
	uint i, j;

	(void)wlc;

	WL_TRACE(("wl%d: wlc_join_pref_build: buffer len %d\n", WLCWLUNIT(wlc), len));

	if (join_pref == NULL)
		return BCME_ERROR;

	if (!ISALIGNED(pref, sizeof(total))) {
		WL_ERROR(("wl%d: %s: buffer not %d byte aligned\n",
			WLCWLUNIT(wlc), __FUNCTION__, (int)sizeof(total)));
		return BCME_BADARG;
	}

	/* calculate buffer length */
	total = 0;
	for (i = 0; i < join_pref->fields; i ++) {
		switch (join_pref->field[i].type) {
		case WL_JOIN_PREF_RSSI:
			total += TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED;
			break;
		case WL_JOIN_PREF_WPA:
			wpalen = join_pref->wpas * sizeof(join_pref->wpa[0]);
			total += TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED + wpalen;
			break;
		case WL_JOIN_PREF_BAND:
			total += TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED;
			break;
		}
	}

	/* Add separately maintained RSSI Delta entry */
	if (cfg->join_pref_rssi_delta.rssi != 0 && cfg->join_pref_rssi_delta.band != WLC_BAND_AUTO)
		total += TLV_HDR_LEN + WLC_JOIN_PREF_LEN_FIXED;

	if (len < total + (int)sizeof(total)) {
		WL_ERROR(("wl%d: %s: buffer too small need %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, (int)(total + sizeof(total))));
		return BCME_BUFTOOSHORT;
	}

	/* build join pref */
	bcopy(&total, pref, sizeof(total));
	pref += sizeof(total);
	/* reverse the order so that it is same as what user supplied */
	for (i = 0; i < join_pref->fields; i ++) {
		switch (join_pref->field[join_pref->fields - 1 - i].type) {
		case WL_JOIN_PREF_RSSI:
			*pref++ = WL_JOIN_PREF_RSSI;
			*pref++ = WLC_JOIN_PREF_LEN_FIXED;
			*pref++ = 0;
			*pref++ = 0;
			break;
		case WL_JOIN_PREF_WPA:
			*pref++ = WL_JOIN_PREF_WPA;
			*pref++ = WLC_JOIN_PREF_LEN_FIXED +
				(uint8)(sizeof(join_pref->wpa[0]) * join_pref->wpas);
			*pref++ = 0;
			*pref++ = (uint8)join_pref->wpas;
			/* reverse the order so that it is same as what user supplied */
			for (j = 0; j < join_pref->wpas; j ++) {
				bcopy(&join_pref->wpa[join_pref->wpas - 1 - j],
					pref, sizeof(join_pref->wpa[0]));
				pref += sizeof(join_pref->wpa[0]);
			}
			break;
		case WL_JOIN_PREF_BAND:
			*pref++ = WL_JOIN_PREF_BAND;
			*pref++ = WLC_JOIN_PREF_LEN_FIXED;
			*pref++ = 0;
			*pref++ = join_pref->band;
			break;
		}
	}

	/* Add the RSSI Delta information. Note that order is NOT important for this field
	 * as it's always applied
	 */
	if (cfg->join_pref_rssi_delta.rssi != 0 &&
	    cfg->join_pref_rssi_delta.band != WLC_BAND_AUTO) {
		*pref++ = WL_JOIN_PREF_RSSI_DELTA;
		*pref++ = WLC_JOIN_PREF_LEN_FIXED;
		*pref++ = cfg->join_pref_rssi_delta.rssi;
		*pref++ = cfg->join_pref_rssi_delta.band;
	}

	return 0;
} /* wlc_join_pref_build */

void
wlc_join_pref_band_upd(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	uint i;
	uint8 start = 0;
	uint p = 0;

	(void)wlc;

	WL_ASSOC(("wl%d: wlc_join_pref_band_upd: band pref is %d\n",
		WLCWLUNIT(wlc), join_pref->band));
	if (join_pref->band == WLC_BAND_AUTO)
		return;
	/* find band field first. rssi field should be set too if found */
	for (i = 0; i < join_pref->fields; i ++) {
		if (join_pref->field[i].type == WL_JOIN_PREF_BAND) {
			WL_ASSOC(("wl%d: found field %s entry %d\n",
				WLCWLUNIT(wlc), WLCJOINPREFN(WL_JOIN_PREF_BAND), i));
			return;
		}
		start += join_pref->field[i].bits;
		p |= WLCTYPEBMP(join_pref->field[i].type);
	}
	/* rssi field is mandatory. fields should be empty when rssi field is not set */
	if (!(p & WLCTYPEBMP(WL_JOIN_PREF_RSSI))) {
		ASSERT(join_pref->fields == 0);
		join_pref->field[0].type = WL_JOIN_PREF_RSSI;
		join_pref->field[0].start = 0;
		join_pref->field[0].bits = WLC_JOIN_PREF_BITS_RSSI;
		WL_ASSOC(("wl%d: wlc_join_pref_band_upd: added field %s entry 0 offset 0\n",
			WLCWLUNIT(wlc), WLCJOINPREFN(WL_JOIN_PREF_RSSI)));
		start = WLC_JOIN_PREF_BITS_RSSI;
		p |= WLCTYPEBMP(WL_JOIN_PREF_RSSI);
		i = 1;
	}
	/* add band field */
	join_pref->field[i].type = WL_JOIN_PREF_BAND;
	join_pref->field[i].start = start;
	join_pref->field[i].bits = WLC_JOIN_PREF_BITS_BAND;
	WL_ASSOC(("wl%d: wlc_join_pref_band_upd: added field %s entry %d offset %d\n",
		WLCWLUNIT(wlc), WLCJOINPREFN(WL_JOIN_PREF_BAND), i, start));
	p |= WLCTYPEBMP(WL_JOIN_PREF_BAND);
	join_pref->prfbmp = p;
	join_pref->fields = i + 1;
}

void
wlc_join_pref_reset(wlc_bsscfg_t *cfg)
{
	uint8 band;
	wlc_join_pref_t *join_pref = cfg->join_pref;

	band = join_pref->band;
	bzero(join_pref, sizeof(wlc_join_pref_t));
	join_pref->band = band;
	wlc_join_pref_band_upd(cfg);
}

void
wlc_auth_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(wlc, (uint16)(uintptr)arg);
	wlc_assoc_t *as;
	wlc_bss_info_t *target_bss;

	/* in case bsscfg is freed before this callback is invoked */
	if (cfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
		          wlc->pub->unit, __FUNCTION__, arg));
		return;
	}

	as = cfg->assoc;
	target_bss = cfg->target_bss;

	/* assoc aborted? */
	if (!(as->state == AS_SENT_AUTH_1 || as->state == AS_SENT_AUTH_3 ||
		as->state == AS_SENT_FTREQ))
		return;

	/* no ack */
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		wlc_auth_complete(cfg, WLC_E_STATUS_NO_ACK, &target_bss->BSSID, 0, 0);
		return;
	}
	wlc_assoc_timer_del(wlc, cfg);
	wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
}

void
wlc_assocreq_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(wlc, (uint16)(uintptr)arg);
	wlc_assoc_t *as;
	wlc_bss_info_t *target_bss;

	/* in case bsscfg is freed before this callback is invoked */
	if (cfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
		          wlc->pub->unit, __FUNCTION__, arg));
		return;
	}

	as = cfg->assoc;
	target_bss = cfg->target_bss;

	/* assoc aborted? */
	if (!(as->state == AS_SENT_ASSOC || as->state == AS_REASSOC_RETRY))
		return;

	/* no ack */
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		wlc_assoc_complete(cfg, WLC_E_STATUS_NO_ACK, &target_bss->BSSID, 0,
			as->type != AS_ASSOCIATION, 0);
		return;
	}
	wlc_assoc_timer_del(wlc, cfg);
	wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
}

static wlc_bss_info_t *
wlc_bss_info_dup(wlc_info_t *wlc, wlc_bss_info_t *bi)
{
	wlc_bss_info_t *bss = MALLOC(wlc->osh, sizeof(wlc_bss_info_t));
	if (!bss) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	bcopy(bi, bss, sizeof(wlc_bss_info_t));
	if (bi->bcn_prb) {
		if (!(bss->bcn_prb = MALLOC(wlc->osh, bi->bcn_prb_len))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
			MFREE(wlc->osh, bss, sizeof(wlc_bss_info_t));
			return NULL;
		}
		bcopy(bi->bcn_prb, bss->bcn_prb, bi->bcn_prb_len);
	}
	return bss;
}

static int
wlc_bss_list_expand(wlc_bsscfg_t *cfg, wlc_bss_list_t *from, wlc_bss_list_t *to)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_join_pref_t *join_pref = cfg->join_pref;
	uint i, j, k, c;
	wlc_bss_info_t *bi;
	struct rsn_parms *rsn;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wpa_suite_t *akm, *uc, *mc;

	WL_ASSOC(("wl%d: wlc_bss_list_expand: scan results %d\n", WLCWLUNIT(wlc), from->count));

	ASSERT(to->count == 0);
	if (WLCAUTOWPA(cfg)) {
		/* duplicate each bss to multiple copies, one for each wpa config */
		for (i = 0, c = 0; i < from->count && c < (uint) wlc->pub->tunables->maxbss; i ++) {
			/* ignore the bss if it does not support wpa */
			if (!(from->ptrs[i]->flags & (WLC_BSS_WPA | WLC_BSS_WPA2)))
			{
				WL_ASSOC(("wl%d: ignored BSS %s, it does not do WPA!\n",
					WLCWLUNIT(wlc),
					bcm_ether_ntoa(&from->ptrs[i]->BSSID, eabuf)));
				continue;
			}

			/*
			* walk thru all wpa configs, move/dup the bss to join targets list
			* if it supports the config.
			*/
			for (j = 0, rsn = NULL, bi = NULL;
			    j < join_pref->wpas && c < (uint) wlc->pub->tunables->maxbss; j ++) {
				WL_ASSOC(("wl%d: WPA cfg %d:"
				          " %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
				          WLCWLUNIT(wlc), j,
				          join_pref->wpa[j].akm[0],
				          join_pref->wpa[j].akm[1],
				          join_pref->wpa[j].akm[2],
				          join_pref->wpa[j].akm[3],
				          join_pref->wpa[j].ucipher[0],
				          join_pref->wpa[j].ucipher[1],
				          join_pref->wpa[j].ucipher[2],
				          join_pref->wpa[j].ucipher[3],
				          join_pref->wpa[j].mcipher[0],
				          join_pref->wpa[j].mcipher[1],
				          join_pref->wpa[j].mcipher[2],
				          join_pref->wpa[j].mcipher[3]));
				/* check if the AP supports the wpa config */
				akm = (wpa_suite_t*)join_pref->wpa[j].akm;
				uc = (wpa_suite_t*)join_pref->wpa[j].ucipher;

				if (!bcmp(akm, WPA_OUI, DOT11_OUI_LEN))
					rsn = bi ? &bi->wpa : &from->ptrs[i]->wpa;
				else if (!bcmp(akm, WPA2_OUI, DOT11_OUI_LEN))
					rsn = bi ? &bi->wpa2 : &from->ptrs[i]->wpa2;
#ifdef BCMCCX
				else if (CCX_ENAB(wlc->pub) &&
					!bcmp(akm, CISCO_AIRONET_OUI, DOT11_OUI_LEN)) {
					/* CCKM AKM does not provide WPA/WPA2 info.
					 * take ucipher in prefer list entry to select
					 * corresponding rsn location to compare AP rsn
					 * parameters
					 */
					if (!bcmp(uc->oui, WPA2_OUI, DOT11_OUI_LEN))
						rsn = bi ? &bi->wpa2 : &from->ptrs[i]->wpa2;
					else
						rsn = bi ? &bi->wpa : &from->ptrs[i]->wpa;
				}
#endif /* BCMCCX */
				else {
				/*
				* something has gone wrong, or need to add
				* new code to handle the new akm here!
				*/
					WL_ERROR(("wl%d: unknown akm suite %02x%02x%02x%02x in WPA"
						" cfg\n",
						WLCWLUNIT(wlc),
						akm->oui[0], akm->oui[1], akm->oui[2], akm->type));
					continue;
				}
#ifdef BCMDBG
				if (WL_ASSOC_ON()) {
					prhex("rsn parms", (uint8 *)rsn, sizeof(*rsn));
				}
#endif /* BCMDBG */
				/* check if the AP offers the akm */
				for (k = 0; k < rsn->acount; k ++) {
					if (akm->type == rsn->auth[k])
						break;
				}
				/* the AP does not offer the akm! */
				if (k >= rsn->acount) {
					WL_ASSOC(("wl%d: skip WPA cfg %d: akm not match\n",
						WLCWLUNIT(wlc), j));
					continue;
				}
				/* check if the AP offers the unicast cipher */
				for (k = 0; k < rsn->ucount; k ++) {
#ifdef BCMCCX
					if (IS_CCX_CIPHER(rsn->unicast[k])) {
						if (!bcmp(uc->oui, CISCO_AIRONET_OUI,
							DOT11_OUI_LEN) &&
							(uc->type + CISCO_BASE ==
							rsn->unicast[k])) {
							if (CCX_ENAB(wlc->pub))
								break;
						}
					}
					else
#endif /* BCMCCX */
					if (uc->type == rsn->unicast[k])
						break;
				}
				/* AP does not offer the cipher! */
				if (k >= rsn->ucount)
					continue;
				/* check if the AP offers the multicast cipher */
				mc = (wpa_suite_t*)join_pref->wpa[j].mcipher;
				if (bcmp(mc, WL_WPA_ACP_MCS_ANY, WPA_SUITE_LEN)) {
#ifdef BCMCCX
					if (IS_CCX_CIPHER(rsn->multicast)) {
						if (!bcmp(mc->oui, CISCO_AIRONET_OUI,
							DOT11_OUI_LEN)) {
							if (!CCX_ENAB(wlc->pub))
								continue;
							if (mc->type + CISCO_BASE !=
								rsn->multicast) {
								WL_ASSOC(("wl%d: skip WPA cfg %d: "
								"mc not match\n",
								WLCWLUNIT(wlc), j));
								continue;
							}
						}
					}
					else
#endif /* BCMCCX */
					if (mc->type != rsn->multicast) {
						WL_ASSOC(("wl%d: skip WPA cfg %d: mc not match\n",
							WLCWLUNIT(wlc), j));
						continue;
					}
				}
				/* move/duplicate the BSS */
				if (!bi) {
					to->ptrs[c] = bi = from->ptrs[i];
					from->ptrs[i] = NULL;
				}
				else if (!(to->ptrs[c] = wlc_bss_info_dup(wlc, bi))) {
					WL_ERROR(("wl%d: failed to duplicate bss info\n",
						WLCWLUNIT(wlc)));
					goto err;
				}
				WL_ASSOC(("wl%d: BSS %s and WPA cfg %d match\n", WLCWLUNIT(wlc),
					bcm_ether_ntoa(&bi->BSSID, eabuf), j));
				/* save multicast cipher for WPA config derivation */
				if (!bcmp(mc, WL_WPA_ACP_MCS_ANY, WPA_SUITE_LEN))
					to->ptrs[c]->mcipher = rsn->multicast;
				/* cache the config index as preference weight */
				to->ptrs[c]->wpacfg = (uint8)j;
				/* mask off WPA or WPA2 flag to match the selected entry */
				if (!bcmp(uc->oui, WPA2_OUI, DOT11_OUI_LEN))
					to->ptrs[c]->flags &= ~WLC_BSS_WPA;
				else
					to->ptrs[c]->flags &= ~WLC_BSS_WPA2;
				c ++;
			}
			/* the BSS does not support any of our wpa configs */
			if (!bi) {
				WL_ASSOC(("wl%d: ignored BSS %s, it does not offer expected WPA"
					" cfg!\n",
					WLCWLUNIT(wlc),
					bcm_ether_ntoa(&from->ptrs[i]->BSSID, eabuf)));
				continue;
			}
		}
	}
	else {
		c = 0;
		WL_ERROR(("wl%d: don't know how to expand the list\n", WLCWLUNIT(wlc)));
		goto err;
	}

	/* what if the join_target list is too big */
	if (c >= (uint) wlc->pub->tunables->maxbss) {
		WL_ERROR(("wl%d: two many entries, scan results may not be fully expanded\n",
			WLCWLUNIT(wlc)));
	}

	/* done */
	to->count = c;
	WL_ASSOC(("wl%d: wlc_bss_list_expand: join targets %d\n", WLCWLUNIT(wlc), c));

	/* clean up the source list */
	wlc_bss_list_free(wlc, from);
	return 0;

	/* error handling */
err:
	to->count = c;
	wlc_bss_list_free(wlc, to);
	wlc_bss_list_free(wlc, from);
	return BCME_ERROR;
} /* wlc_bss_list_expand */

static void
wlc_assoc_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_assoc_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

static void wlc_assocresp_client_next(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	assoc_decision_t * assoc_decision, struct scb *scb);

void wlc_process_assocresp_decision(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, assoc_decision_t *dc)
{
	wlc_assocresp_client_next(wlc, bsscfg, dc, NULL);
}

static void wlc_assocresp_client_next(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	assoc_decision_t * dc, struct scb *scb)
{
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint16 status;
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN], *da = bcm_ether_ntoa(&dc->da, eabuf);
#endif // endif

	scb = wlc_scbfind(wlc, cfg, &dc->da);
	if (scb == NULL) {
		WL_ERROR(("wl%d.%d %s could not find scb\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), da));
		return;
	}

	status = (dc->assoc_approved == TRUE) ? DOT11_SC_SUCCESS : dc->reject_reason;

	if (status != DOT11_SC_SUCCESS) {
		wlc_assoc_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID,
			status, as->type != AS_ASSOCIATION,
			WLC_DOT11_BSSTYPE(target_bss->infra));
		return;
	}

#if defined(SPLIT_ASSOC)
	if (SPLIT_ASSOC_RESP(cfg)) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		WL_ASSOC(("wl%d: %s: da(%s) systime(%u)\n",
			wlc->pub->unit, __FUNCTION__, da, OSL_SYSUPTIME()));
#endif // endif
		wlc_assoc_dc_dispatch(wlc, cfg, dc, scb);
	}
#endif /* SPLIT_ASSOC */

#ifdef EXT_STA
	/* XXX: vista events are synchronous, so need to call assoc_success before
	 * posting event. This really needs cleaning...
	 */
	if (WLEXTSTA_ENAB(wlc->pub)) {
		wlc_assoc_success(cfg, scb);

		wlc_assoc_complete(cfg, WLC_E_STATUS_SUCCESS, &target_bss->BSSID,
			status, as->type != AS_ASSOCIATION,
			WLC_DOT11_BSSTYPE(target_bss->infra));
	} else
#endif /* EXT_STA */
	{
		wlc_assoc_complete(cfg, WLC_E_STATUS_SUCCESS, &target_bss->BSSID,
			status, as->type != AS_ASSOCIATION,
			WLC_DOT11_BSSTYPE(target_bss->infra));

		/* XXX REVISIT should this be after or before assoc_complete (see above),
		 * now that processing has been pulled from wlc_event_handle()?
		 */
		wlc_assoc_success(cfg, scb);
#ifdef PSTA
		/* Enable PM if primary BSS is wlancoex STA interface */
		if (PSTA_ENAB(wlc->pub) && wlc_wlancoex_on(wlc)) {
			int force_pm = PM_MAX;
			if (wlc_ioctl(wlc, WLC_SET_PM, (void *)&force_pm,
				sizeof(int), cfg->wlcif))
				WL_PSTA(("wl%d.%d: wlancoex sta entering to PM error\n",
					WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
		}
#endif /* PSTA */
#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub) && wlc->ccx->ccx_network) {
#ifdef BCMSUP_PSK
			if (SUP_ENAB(wlc->pub) &&
				(BSS_SUP_TYPE(wlc->idsup, cfg) == SUP_UNUSED))
#endif // endif
			{
				wlc_ccx_iapp_roam_rpt(wlc->ccx);
			}
		}
#endif /* BCMCCX */
	}

	WL_ASSOC(("wl%d.%d: Checking if key needs to be inserted\n",
	          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
	/* If Multi-SSID is enabled, and Legacy WEP is in use for this bsscfg,
	 * a "pairwise" key must be created by copying the default key from the bsscfg.
	 */

	if (cfg->WPA_auth == WPA_AUTH_DISABLED)
		WL_ASSOC(("wl%d: WPA disabled\n", WLCWLUNIT(wlc)));
	if (WSEC_WEP_ENABLED(cfg->wsec))
		WL_ASSOC(("wl%d: WEP enabled\n", WLCWLUNIT(wlc)));
	if (MBSS_ENAB(wlc->pub))
		WL_ASSOC(("wl%d: MBSS on\n", WLCWLUNIT(wlc)));
	if ((MBSS_ENAB(wlc->pub) || PSTA_ENAB(wlc->pub) || cfg != wlc->cfg) &&
	    cfg->WPA_auth == WPA_AUTH_DISABLED && WSEC_WEP_ENABLED(cfg->wsec)) {
		wlc_key_t *key;
		wlc_key_info_t key_info;
		uint8 data[WEP128_KEY_SIZE];
		size_t data_len;
		wlc_key_algo_t algo;
		int err;

#ifdef PSTA
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt,
			(BSSCFG_PSTA(cfg) ? wlc_bsscfg_primary(wlc) : cfg), FALSE, &key_info);
#else /* PSTA */
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, cfg, FALSE, &key_info);
#endif /* PSTA */

		algo = key_info.algo;
		if (algo != CRYPTO_ALGO_OFF) {
			WL_ASSOC(("wl%d: Def key installed\n", WLCWLUNIT(wlc)));
			if (algo == CRYPTO_ALGO_WEP1 || algo == CRYPTO_ALGO_WEP128) {
				wlc_key_t *bss_key;

				WL_ASSOC(("wl%d: Inserting key for %s\n", wlc->pub->unit, eabuf));
				err = wlc_key_get_data(key, data, sizeof(data), &data_len);
				if (err == BCME_OK) {
					bss_key = wlc_keymgmt_get_key_by_addr(wlc->keymgmt,
						cfg, &target_bss->BSSID, 0, NULL);
					err = wlc_key_set_data(bss_key, algo, data, data_len);
				}
				if (err != BCME_OK)
					WL_ERROR(("wl%d.%d: Error %d inserting key for bss %s\n",
						WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), err, eabuf));
			}
		}
	}
}

void
wlc_assocresp_client(wlc_bsscfg_t *cfg, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, uint body_len)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	struct dot11_assoc_resp *assoc = (struct dot11_assoc_resp *) body;
	uint16 fk;
	uint16 status;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	assoc_decision_t decision;
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];

	bcm_ether_ntoa(&hdr->sa, eabuf);
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_ASSOC */

	ASSERT(BSSCFG_STA(cfg));

	fk = ltoh16(hdr->fc) & FC_KIND_MASK;
	status = ltoh16(assoc->status);

	if (!(as->state == AS_SENT_ASSOC || as->state == AS_REASSOC_RETRY) ||
		bcmp(hdr->bssid.octet, target_bss->BSSID.octet, ETHER_ADDR_LEN)) {
		WL_ERROR(("wl%d.%d: unsolicited association response from %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), eabuf));
		wlc_assoc_complete(cfg, WLC_E_STATUS_UNSOLICITED, &hdr->sa, 0,
			fk != FC_ASSOC_RESP, 0);
		return;
	}

	/* capability */
#ifdef BCMDBG
	if (WL_ERROR_ON()) {
		if (!(ltoh16(assoc->capability) & DOT11_CAP_ESS)) {
			WL_ERROR(("wl%d: association response without ESS set from %s\n",
				wlc->pub->unit, eabuf));
		}
		if (ltoh16(assoc->capability) & DOT11_CAP_IBSS) {
			WL_ERROR(("wl%d: association response with IBSS set from %s\n",
				wlc->pub->unit, eabuf));
		}
	}
#endif /* BCMDBG */

	/* save last (re)association response */
	if (as->resp) {
		MFREE(wlc->osh, as->resp, as->resp_len);
		as->resp_len = 0;
	}
	if (!(as->resp = MALLOC(wlc->osh, body_len)))
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
	else {
		as->resp_len = body_len;
		bcopy((char*)assoc, (char*)as->resp, body_len);
	}

#ifdef BCMCCX
	if (CAC_ENAB(wlc->pub))
		assoc->status = wlc_cac_assoc_status(wlc->cac, assoc->status);
#endif	/* BCMCCX */

	/* association denied */
	if (status == DOT11_SC_REASSOC_FAIL &&
	    as->type != AS_ASSOCIATION &&
	    as->state != AS_REASSOC_RETRY) {
		ASSERT(scb != NULL);

		wlc_join_assoc_start(wlc, cfg, scb, target_bss, FALSE);
		wlc_assoc_change_state(cfg, AS_REASSOC_RETRY);
		WL_ASSOC(("wl%d: Retrying with Assoc Req frame "
			"due to Reassoc Req failure (DOT11_SC_REASSOC_FAIL) from %s\n",
			wlc->pub->unit, eabuf));
		return;
#ifdef WL_SAE
	} else if (status == DOT11_SC_INVALID_PMKID) {
		wlc_assoc_abort(cfg);
		return;
#endif /* WL_SAE */
	} else if (status != DOT11_SC_SUCCESS) {
		wlc_assoc_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID,
			status, as->type != AS_ASSOCIATION,
			WLC_DOT11_BSSTYPE(target_bss->infra));
		return;
	}

	ASSERT(scb != NULL);

	/* Mark assoctime for use in roam calculations */
	scb->assoctime = wlc->pub->now;

	body += sizeof(struct dot11_assoc_resp);
	body_len -= sizeof(struct dot11_assoc_resp);

	wlc_bss_mac_event(wlc, cfg, WLC_E_ASSOC_RESP_IE, NULL, 0, 0, 0,
	                  body, (int)body_len);

	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_assoc_nhdlr_cb;
	upp.vsie_fn = wlc_assoc_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.assocresp.scb = scb;
	ftpparm.assocresp.status = status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	if (wlc_iem_parse_frame(wlc->iemi, cfg, fk, &upp, &pparm,
	                        body, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return;
	}
	status = ftpparm.assocresp.status;

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, cfg)) {
		wlc_p2p_process_assocresp(wlc->p2p, scb, body, body_len);
	}
#endif // endif

#ifdef WL11K
	if (WL11K_ENAB(wlc->pub) && wlc_rrm_enabled(wlc->rrm_info, cfg)) {
		/* update the AP's scb for RRM */
		scb->flags3 &= ~SCB3_RRM;
		if (ltoh16(assoc->capability) & DOT11_CAP_RRM)
			scb->flags3 |= SCB3_RRM;
	}
#endif /* WL11K */

	/* Association success */
	cfg->AID = ltoh16(assoc->aid);
	wlc->AID = cfg->AID;

#if defined(SPLIT_ASSOC)
	if (SPLIT_ASSOC_RESP(cfg)) {
		/* send event */
		wlc_bss_mac_event(wlc, cfg, WLC_E_PRE_ASSOC_RSEP_IND,
			&hdr->sa, WLC_E_STATUS_SUCCESS, status, 0, body, body_len);
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC)
		WL_ASSOC(("wl%d: %s: recv assoc resp from (%s) systime(%u)\n",
			wlc->pub->unit, __FUNCTION__, eabuf, OSL_SYSUPTIME()));
#endif // endif
		return;

	} else
#endif /* SPLIT_ASSOC */
	{
		decision.assoc_approved = (status == DOT11_SC_SUCCESS) ? TRUE : FALSE;
		decision.reject_reason = (status == DOT11_SC_SUCCESS) ? 0 : status;
		bcopy(&scb->ea, &decision.da, ETHER_ADDR_LEN);
		wlc_assocresp_client_next(wlc, cfg, &decision, scb);
	}
}

static void
wlc_auth_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_auth_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

void
wlc_authresp_client(wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, uint body_len, bool short_preamble)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif /* BCMDBG_ERR */
#ifdef WL_SAE
	uint8 *body_start = body;
	uint body_len_start = body_len;
#endif /* WL_SAE */
	struct dot11_auth *auth = (struct dot11_auth *) body;
	uint16 auth_alg, auth_seq, auth_status;
	uint cfg_auth_alg;
	bool unsolicited_pkt = FALSE;
	struct scb *scb;
	void *pkt;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;

	WL_ASSOC(("wl%d: wlc_authresp_client\n", wlc->pub->unit));

	auth_alg = ltoh16(auth->alg);
	auth_seq = ltoh16(auth->seq);
	auth_status = ltoh16(auth->status);

	/* ignore authentication frames from other stations */
	if (bcmp((char*)&hdr->sa, (char*)&target_bss->BSSID, ETHER_ADDR_LEN) ||
	    (scb = wlc_scbfind(wlc, cfg, (struct ether_addr *)&hdr->sa)) == NULL ||
	    (as->state != AS_SENT_AUTH_1 && as->state != AS_SENT_AUTH_3)) {
		WL_ERROR(("wl%d.%d: unsolicited authentication response from %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), sa));
		wlc_auth_complete(cfg, WLC_E_STATUS_UNSOLICITED, &hdr->sa, 0, 0);
		return;
	}

	ASSERT(scb != NULL);

#if defined(AP) && defined(STA)
	if (SCB_ASSOCIATED(scb) || !(scb->flags & SCB_MYAP)) {
		WL_APSTA(("wl%d: got AUTH from %s, associated but not AP, forcing AP flag\n",
			wlc->pub->unit, sa));
	}
#endif /* APSTA */
	scb->flags |= SCB_MYAP;

	/* For SAE AUTH_1 and AUTH_2 will have seq. no. 1
	 * and AUTH_3, AUTH_4 will have seq. no. 2.
	 */
#ifdef WL_SAE
	if (auth_alg == DOT11_SAE) {
		if ((as->state == AS_SENT_AUTH_1 && auth_seq != 1) ||
				(as->state == AS_SENT_AUTH_3 && auth_seq != 2))
			unsolicited_pkt = TRUE;
	} else
#endif /* WL_SAE */
	{

		if ((as->state == AS_SENT_AUTH_1 && auth_seq != 2) ||
				(as->state == AS_SENT_AUTH_3 && auth_seq != 4)) {
			unsolicited_pkt = TRUE;
		}
	}

	if (unsolicited_pkt == TRUE) {
		WL_ERROR(("wl%d: out-of-sequence authentication response from %s\n",
			wlc->pub->unit, sa));
		wlc_auth_complete(cfg, WLC_E_STATUS_UNSOLICITED, &hdr->sa, 0, 0);
	}
	/* authentication error */
	if (auth_status != DOT11_SC_SUCCESS) {
		if (!(BSSCFG_EXT_AUTH(cfg) && auth_status == DOT11_SC_ANTICLOG_TOCKEN_REQUIRED)) {
			wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &hdr->sa, auth_status, auth_alg);
			return;
		}
	}

	/* invalid authentication algorithm number */
#ifdef WLFBT
	if (!BSSCFG_IS_FBT(cfg)) {
#endif // endif
#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && wlc->ccx->ccx_network)
		cfg_auth_alg = wlc_ccx_set_auth(wlc->ccx, cfg->auth);
	else
#endif // endif
		cfg_auth_alg = cfg->auth;

	if (auth_alg != cfg_auth_alg && !cfg->openshared) {
		WL_ERROR(("wl%d: invalid authentication algorithm number, got %d, expected %d\n",
		          wlc->pub->unit, auth_alg, cfg_auth_alg));
		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &hdr->sa, auth_status, auth_alg);
		return;
	}
#ifdef WLFBT
	}
#endif // endif

	body += sizeof(struct dot11_auth);
	body_len -= sizeof(struct dot11_auth);

	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_auth_nhdlr_cb;
	upp.vsie_fn = wlc_auth_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.auth.alg = auth_alg;
	ftpparm.auth.seq = auth_seq;
	ftpparm.auth.scb = scb;
	ftpparm.auth.status = (uint16)auth_status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	if (wlc_iem_parse_frame(wlc->iemi, cfg, FC_AUTH, &upp, &pparm,
	                        body, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return;
	}
	auth_status = ftpparm.auth.status;

#if defined(WL_SAE)
	/* If running SAE algorithm, by-pass these tests that might not apply */
	if (BSSCFG_EXT_AUTH(cfg)) {

		/* Don't process the Sae Auth packet , if the
		 cfg->auth and auth_alg is != DOT11_SAE
		*/
		if (auth_alg != DOT11_SAE)
			return;

		if ((auth_status == DOT11_SC_SUCCESS) &&
				(auth_seq == WL_SAE_CONFIRM)) {
			wlc_auth_complete(cfg, BCME_OK, &hdr->sa, auth_status, auth_alg);
		}

		WL_ASSOC(("wl%d: Sae External Auth: auth_seq = %d\n", WLCWLUNIT(wlc),
			auth_seq));
		wlc_assoc_change_state(cfg, AS_SENT_AUTH_UP);
		wlc_assoc_timer_del(wlc, cfg);
		wl_add_timer(wlc->wl, cfg->assoc->timer, WLC_EXT_AUTH_TIMEOUT, 0);

		wlc_bss_mac_event(wlc, cfg, WLC_E_AUTH, &hdr->sa, BCME_OK,
			auth_status, auth_alg, body_start, body_len_start);
		return;
	}

#endif /* WL_SAE */

	if (auth_status == DOT11_SC_SUCCESS &&
	    auth_alg == DOT11_SHARED_KEY && auth_seq == 2) {
		uint8 *challenge;

		WL_ASSOC(("wl%d: JOIN: got authentication response seq 2 ...\n",
		            WLCWLUNIT(wlc)));
		wlc_assoc_change_state(cfg, AS_SENT_AUTH_3);

		challenge = ftpparm.auth.challenge;
		ASSERT(challenge != NULL);

		pkt = wlc_sendauth(cfg, &scb->ea, &target_bss->BSSID, scb, DOT11_SHARED_KEY, 3,
			DOT11_SC_SUCCESS, challenge, short_preamble);
		wlc_assoc_timer_del(wlc, cfg);
		if (pkt == NULL)
			wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
	}
	else {
		uint status = auth_status == DOT11_SC_SUCCESS ?
		        WLC_E_STATUS_SUCCESS : WLC_E_STATUS_FAIL;

		ASSERT(auth_seq == 2 || (auth_alg == DOT11_SHARED_KEY && auth_seq == 4));
		/* authentication success or failure */
		wlc_auth_complete(cfg, status, &hdr->sa, auth_status, auth_alg);
	}
} /* wlc_authresp_client */

void
wlc_clear_hw_association(wlc_bsscfg_t *cfg, bool mute_mode)
{
	wlc_info_t *wlc = cfg->wlc;
	d11regs_t *regs = wlc->regs;
	wlc_rateset_t rs;

	wlc_set_pmoverride(cfg, FALSE);

	/* zero the BSSID so the core will not process TSF updates */
	wlc_bss_clear_bssid(cfg);

	/* Clear any possible Channel Switch Announcement */
	if (WL11H_ENAB(wlc))
		wlc_csa_reset_all(wlc->csa, cfg);

	cfg->assoc->rt = FALSE;
	/* reset quiet channels to the unassociated state */
	wlc_quiet_channels_reset(wlc->cmi);

	if (!DEVICEREMOVED(wlc)) {
		wlc_suspend_mac_and_wait(wlc);

		if (mute_mode == ON &&
		    wlc_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC))
			wlc_mute(wlc, ON, 0);

		/* clear BSSID in PSM */
		wlc_clear_bssid(cfg);

		/* write the beacon interval to the TSF block */
		W_REG(wlc->osh, &regs->tsf_cfprep, 0x80000000);

		/* gphy, aphy use the same CWMIN */
		wlc_bmac_set_cwmin(wlc->hw, APHY_CWMIN);
		wlc_bmac_set_cwmax(wlc->hw, PHY_CWMAX);

		if (BAND_2G(wlc->band->bandtype))
			wlc_bmac_set_shortslot(wlc->hw, wlc->shortslot);

		/* Restore the frequency tracking bandwidth of the PHY, if we modified it */
		wlc_freqtrack_reset(wlc);

		wlc_enable_mac(wlc);

		/* Reset the basic rate lookup table to phy mandatory rates */
		rs.count = 0;
		wlc_rate_lookup_init(wlc, &rs);
		wlc_set_ratetable(wlc);
	}
} /* wlc_clear_hw_association */

/**
 * start an IBSS with all parameters in cfg->target_bss except SSID and BSSID.
 * SSID comes from cfg->SSID; BSSID comes from cfg->BSSID if it is not null,
 * generate a random BSSID otherwise.
 */
static int
_wlc_join_start_ibss(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_bss_info_t bi;
	wlcband_t *band;
	uint channel;
	wlc_assoc_t *as;

	(void)as;

	/* set up default BSS params */
	bcopy(cfg->target_bss, &bi, sizeof(bi));

	bcopy(&cfg->BSSID, &bi.BSSID, ETHER_ADDR_LEN);

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub) && (wlc->allow_mode == 1)) {
		bcopy(&wlc->desired_BSSID.octet, &bi.BSSID.octet, ETHER_ADDR_LEN);
	}
#endif // endif

	if (ETHER_ISNULLADDR(&bi.BSSID)) {
		/* create IBSS BSSID using a random number */
		wlc_getrand(wlc, &bi.BSSID.octet[0], ETHER_ADDR_LEN);

		/* Set the first 2 MAC addr bits to "locally administered" */
		ETHER_SET_LOCALADDR(&bi.BSSID.octet[0]);
		ETHER_SET_UNICAST(&bi.BSSID.octet[0]);
	}
	if (!ETHER_ISNULLADDR(&wlc->desired_BSSID))
		bcopy(&wlc->desired_BSSID.octet, &bi.BSSID.octet, ETHER_ADDR_LEN);
	if (BSSCFG_AWDL(wlc, cfg))
		bcopy(&cfg->BSSID.octet, &bi.BSSID.octet, ETHER_ADDR_LEN);

	/* adopt previously specified params */
	bi.infra = 0;
	bzero(bi.SSID, sizeof(bi.SSID));
	bi.SSID_len = cfg->SSID_len;
	bcopy(cfg->SSID, bi.SSID, MIN(bi.SSID_len, sizeof(bi.SSID)));

	/* Check if 40MHz channel bandwidth is allowed in current locale and band and
	 * convert to 20MHz if not allowed
	 */
	band = wlc->bandstate[CHSPEC_IS2G(bi.chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];
	if (CHSPEC_IS40(bi.chanspec) &&
	    (!N_ENAB(wlc->pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_40MHZ) ||
	     !WL_BW_CAP_40MHZ(band->bw_cap))) {
		channel = wf_chspec_ctlchan(bi.chanspec);
		bi.chanspec = CH20MHZ_CHSPEC(channel);
	}

	/*
	   Validate or fixup default channel value.
	   Don't want to create ibss on quiet channel since it hasn't be
	   verified as radar-free.
	*/
	if (!wlc_valid_chanspec_db(wlc->cmi, bi.chanspec) ||
	    wlc_quiet_chanspec(wlc->cmi, bi.chanspec)) {
		chanspec_t chspec =
		        wlc_next_chanspec(wlc->cmi, bi.chanspec, CHAN_TYPE_CHATTY, TRUE);
		if (chspec == INVCHANSPEC) {
			wlc_set_ssid_complete(cfg, WLC_E_STATUS_NO_NETWORKS, &bi.BSSID,
				DOT11_BSSTYPE_INDEPENDENT);
			return BCME_ERROR;
		}
		bi.chanspec = chspec;
	}

#ifdef CREATE_IBSS_ON_QUIET_WITH_11H
	/* For future reference, if we do support ibss creation
	 * on radar channels, need to unmute and clear quiet bit.
	 */
#ifdef BAND5G /* RADAR */
	if (WLC_BAND_PI_RADIO_CHANSPEC != bi.chanspec) {
		wlc_clr_quiet_chanspec(wlc->cmi, bi.chanspec);
		/* set_channel() will unmute */
	} else if (wlc_quiet_chanspec(wlc->cmi, bi.chanspec)) {
		wlc_clr_quiet_chanspec(wlc->cmi, bi.chanspec);
		wlc_mute(wlc, OFF, 0);
	}
#endif /* BAND5G */
#endif /* CREATE_IBSS_ON_QUIET_WITH_11H */

#ifdef WL11N
	/* BSS rateset needs to be adjusted to account for channel bandwidth */
	wlc_rateset_bw_mcs_filter(&bi.rateset,
		WL_BW_CAP_40MHZ(wlc->band->bw_cap)?CHSPEC_WLC_BW(bi.chanspec):0);
#endif // endif

	if (WSEC_ENABLED(cfg->wsec) && (cfg->wsec_restrict)) {
		WL_WSEC(("%s(): set bi->capability DOT11_CAP_PRIVACY bit.\n", __FUNCTION__));
		bi.capability |= DOT11_CAP_PRIVACY;
	}

	bi.capability |= DOT11_CAP_IBSS;

	/* Set the ATIM window */
	if (AIBSS_ENAB(wlc->pub) && AIBSS_PS_ENAB(wlc->pub)) {
		if (wlc->default_bss->atim_window) {
			cfg->flags2 |= WLC_BSSCFG_FL2_AIBSS_PS;
		}
		cfg->current_bss->atim_window = wlc->default_bss->atim_window;
	}
	else {
		cfg->current_bss->atim_window = bi.atim_window = 0;
		cfg->target_bss->atim_window = 0;
	}

	wlc_suspend_mac_and_wait(wlc);
	wlc_BSSinit(wlc, &bi, cfg, WLC_BSS_START);
	wlc_enable_mac(wlc);

	/* initialize link state tracking to the lost beacons condition */
	cfg->roam->time_since_bcn = 1;
	cfg->roam->bcns_lost = TRUE;

	wlc_sta_assoc_upd(cfg, TRUE);
	WL_RTDC(wlc, "wlc_join_start_ibss: associated", 0, 0);

#if defined(PHYCAL_CACHING)
	/* update channel cache ctx for IBSS */
	wlc_phy_create_chanctx(wlc->pi, bi.chanspec);
#endif // endif

	/* force a PHY cal on the current IBSS channel */
	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band))
		wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_START_IBSS);

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		cfg->current_bss->dtim_period = 1;
		wlc_mcnx_bss_upd(wlc->mcnx, cfg, TRUE);
	}
#endif // endif

	wlc_bsscfg_up(wlc, cfg);

	/* Apply the STA WME Parameters */
	if (BSS_WME_ENAB(wlc, cfg)) {
		/* To be consistent with IBSS Start and join set WLC_BSSCFG_WME_ASSOC */
		cfg->flags |= WLC_BSSCFG_WME_ASSOC;

		/* Starting IBSS so apply local EDCF parameters */
		wlc_edcf_acp_apply(wlc, cfg, TRUE);
	}

	as = cfg->assoc;
	ASSERT(as != NULL);

	/* notifying interested parties of the state... */
	wlc_bss_assoc_state_notif(wlc, cfg, as->type, as->state);

	WL_ERROR(("wl%d: IBSS started\n", wlc->pub->unit));
	/* N.B.: bss_type passed through auth_type event field */
	wlc_bss_mac_event(wlc, cfg, WLC_E_START, &bi.BSSID,
	            WLC_E_STATUS_SUCCESS, 0, DOT11_BSSTYPE_INDEPENDENT,
	            NULL, 0);

	WL_APSTA_UPDN(("wl%d: Reporting link up on config 0 for IBSS starting "
	               "a BSS\n", WLCWLUNIT(wlc)));
	wlc_link(wlc, TRUE, &bi.BSSID, cfg, 0);

	wlc_set_ssid_complete(cfg, WLC_E_STATUS_SUCCESS, &bi.BSSID, DOT11_BSSTYPE_INDEPENDENT);

	return BCME_OK;
} /* _wlc_join_start_ibss */

#ifdef WLAWDL
void wlc_join_awdl_bsscfg(wlc_bsscfg_t *cfg)
{
	int ret = 0;

	/* add the join request in the assoc_req list
	 * if someone is already in the process of association
	 */
	if ((ret = wlc_mac_request_entry(cfg->wlc, cfg, WLC_ACTION_ASSOC)) > 0) {
		WL_ASSOC(("wl%d.%d: JOIN request queued at slot %d\n",
		          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ret));
		return;
	}
	else if (ret < 0) {
		WL_ERROR(("wl%d.%d: JOIN request failed, err = %d\n",
		          cfg->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ret));
		return;
	}
	wlc_join_start_ibss(cfg->wlc, cfg);
}
#endif /* WLAWDL */

/**
 * start an IBSS with all parameters in wlc->default_bss except:
 * - SSID
 * - BSSID
 * SSID comes from cfg->SSID and cfg->SSID_len.
 * BSSID comes from cfg->BSSID, a random BSSID is generated according to IBSS BSSID rule
 * if BSSID is null.
 */
int
wlc_join_start_ibss(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bool mpc_join = wlc->mpc_join;
	int8 infra;
	uint8 dtim;
	int err;

	/* reuse the default configuration to create an IBSS */
	wlc->default_bss->infra = 0;
	wlc->default_bss->dtim_period = 1;

#ifdef WME
	wlc_wme_initparams_sta(wlc, &cfg->wme->wme_param_ie);
#endif // endif
	wlc_join_start_prep(wlc, cfg);

	wlc->mpc_join = TRUE;
	wlc_radio_mpc_upd(wlc);

	if (!wlc->pub->up) {
		WL_ERROR(("wl%d: %s: unable to start IBSS while driver is down\n",
		          wlc->pub->unit, __FUNCTION__));
		return BCME_NOTUP;
	}

	infra = wlc->default_bss->infra;
	dtim = wlc->default_bss->dtim_period;
	err = _wlc_join_start_ibss(wlc, cfg);
	wlc->default_bss->infra = infra;
	wlc->default_bss->dtim_period = dtim;

	wlc->mpc_join = mpc_join;
	wlc_radio_mpc_upd(wlc);

	return err;
} /* wlc_join_start_ibss */

static void
wlc_disassoc_done_quiet_chl(wlc_info_t *wlc, uint txstatus, void *arg)
{
	if (wlc->pub->up &&
	    wlc_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC)) {
		WL_ASSOC(("%s: muting the channel \n", __FUNCTION__));
		wlc_mute(wlc, ON, 0);
	}
}

#ifdef ROBUST_DISASSOC_TX
static void wlc_disassoc_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg);

static void
wlc_disassoc_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(wlc, (uint16)(uintptr)arg);

	/* in case bsscfg is freed before this callback is invoked */
	if (cfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
		          wlc->pub->unit, __FUNCTION__, arg));
		return;
	}

	if (txstatus & TX_STATUS_ACK_RCV) {
		cfg->assoc->block_disassoc_tx = TRUE;
		wlc_disassoc_tx(cfg, FALSE);
	} else {	/* no ack */
		wlc_disassoc_tx(cfg, TRUE);
	}

	return;
}

void
wlc_disassoc_tx(wlc_bsscfg_t *cfg, bool send_disassociate)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	struct scb *scb;
	struct ether_addr BSSID;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_bss_info_t *current_bss = cfg->current_bss;

	WL_TRACE(("wl%d: wlc_disasso_tx\n", wlc->pub->unit));

	if (wlc->pub->associated == FALSE)
		goto exit;
	if (!cfg->associated)
		goto exit;

	if (cfg->BSS) {
		bcopy(&cfg->prev_BSSID, &BSSID, ETHER_ADDR_LEN);
	} else {
		goto exit;
	}

#ifdef EXT_STA
	if (as->disassoc_tx_retry == 0 && WLEXTSTA_ENAB(wlc->pub)) {
		/* Cancel the suppress-ssid-after-hibernate flag for Vista */
		wlc->scan_suppress_ssid = FALSE;
	}
#endif /* EXT_STA */

	if (DEVICEREMOVED(wlc))
		goto exit;

	/* BSS STA */
	if (cfg->BSS) {
		/* abort any association state machine in process */
		if ((as->state != AS_IDLE) && (as->state != AS_WAIT_DISASSOC))
			wlc_assoc_abort(cfg);

		scb = wlc_scbfind(wlc, cfg, &BSSID);

		/* clear PM state, (value used for wake shouldn't matter) */
		wlc_update_bcn_info(cfg, FALSE);

		wlc_pspoll_timer_upd(cfg, FALSE);
		wlc_apsd_trigger_upd(cfg, FALSE);

#ifdef QUIET_DISASSOC
		if (send_disassociate) {
			send_disassociate = FALSE;
		}
#endif /* QUIET_DISASSOC */
#ifdef WLTDLS
		if (TDLS_ENAB(wlc->pub) && wlc_tdls_quiet_down(wlc->tdls)) {
			WL_ASSOC(("wl%d: JOIN: skipping DISASSOC to %s since we are "
					    "quite down.\n", WLCWLUNIT(wlc), eabuf));
			send_disassociate = FALSE;
		}
#endif // endif
		/* Send disassociate packet and (attempt to) schedule callback */
		if (send_disassociate) {
			if (ETHER_ISNULLADDR(cfg->BSSID.octet)) {
				/* a NULL BSSID indicates that we have given up on our AP connection
				 * to the point that we will reassociate to it if we ever see it
				 * again. In this case, we should not send a disassoc
				 */
				WL_ASSOC(("wl%d: JOIN: skipping DISASSOC to %s since we lost "
					    "contact.\n", WLCWLUNIT(wlc), eabuf));
			} else if (wlc_radar_chanspec(wlc->cmi, current_bss->chanspec) ||
			        wlc_restricted_chanspec(wlc->cmi, current_bss->chanspec)) {
			} else if (as->disassoc_tx_retry < 7) {
				void *pkt;
				WL_ASSOC(("wl%d: JOIN: sending DISASSOC to %s\n",
					WLCWLUNIT(wlc), eabuf));
				pkt = wlc_senddisassoc(wlc, cfg, scb, &BSSID, &BSSID,
					&cfg->cur_etheraddr, DOT11_RC_DISASSOC_LEAVING);
				if (pkt != NULL)
					wlc_pcb_fn_register(wlc->pcb,
						wlc_disassoc_tx_complete,
						(void *)(uintptr)cfg->ID, pkt);
				else {
					WL_ASSOC(("wl%d: JOIN: error sending DISASSOC\n",
						WLCWLUNIT(wlc)));
					goto exit;
				}
				as->disassoc_tx_retry++;
				return;
			}
		}
	}

exit:
	as->disassoc_tx_retry = 0;
	wlc_bsscfg_disable(wlc, cfg);
	cfg->assoc->block_disassoc_tx = FALSE;
	return;
} /* wlc_disassoc_tx */
#endif /* ROBUST_DISASSOC_TX */

static int
_wlc_disassociate_client(wlc_bsscfg_t *cfg, bool send_disassociate, pkcb_fn_t fn, void *arg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	struct scb *scb;
	struct ether_addr BSSID;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	uint bsstype;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	bool mute_mode = ON;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	WL_TRACE(("wl%d: wlc_disassociate_client\n", wlc->pub->unit));

	if (wlc->pub->associated == FALSE)
		return (-1);
	if (!cfg->associated)
		return (-1);

	if (cfg->BSS) {
		bcopy(&cfg->prev_BSSID, &BSSID, ETHER_ADDR_LEN);
	} else {
		bcopy(&current_bss->BSSID, &BSSID, ETHER_ADDR_LEN);
	}
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	bcm_ether_ntoa(&BSSID, eabuf);
#endif /* BCMDBG || WLMSG_ASSOC */

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		/* Cancel the suppress-ssid-after-hibernate flag for Vista */
		wlc->scan_suppress_ssid = FALSE;
	}
#endif /* EXT_STA */

#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && cfg->BSS)
		wlc_ccx_on_leave_bss(wlc->ccx);
#endif /* BCMCCX */

#ifdef	WLCAC
	if (CAC_ENAB(wlc->pub))
		wlc_cac_on_leave_bss(wlc->cac);
#endif	/* WLCAC */

#ifdef WLTDLS
	/* cleanup the TDLS peers which require an association */
	if (TDLS_ENAB(wlc->pub))
		wlc_tdls_cleanup(wlc->tdls, cfg);
#endif /* WLTDLS */

#ifdef WLMCNX
	/* reset multi-connection assoc states */
	if (MCNX_ENAB(wlc->pub)) {
		if (cfg->BSS)
			wlc_mcnx_assoc_upd(wlc->mcnx, cfg, FALSE);
		else
			wlc_mcnx_bss_upd(wlc->mcnx, cfg, FALSE);
	}
#endif // endif

	if (DEVICEREMOVED(wlc)) {
		wlc_sta_assoc_upd(cfg, FALSE);
		wlc_clear_hw_association(cfg, mute_mode);
		wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS, &BSSID,
			DOT11_RC_DISASSOC_LEAVING, WLC_DOT11_BSSTYPE(cfg->BSS));
		return (0);
	}

	/* BSS STA */
	if (cfg->BSS) {
		/* cache BSS type for disassoc indication */
		bsstype = DOT11_BSSTYPE_INFRASTRUCTURE;

		/* abort any association state machine in process */
		if ((as->state != AS_IDLE) && (as->state != AS_WAIT_DISASSOC))
			wlc_assoc_abort(cfg);

		scb = wlc_scbfind(wlc, cfg, &BSSID);

		/* clear PM state, (value used for wake shouldn't matter) */
		wlc_update_bcn_info(cfg, FALSE);

		wlc_pspoll_timer_upd(cfg, FALSE);
		wlc_apsd_trigger_upd(cfg, FALSE);

#ifdef QUIET_DISASSOC
		if (send_disassociate) {
			send_disassociate = FALSE;
		}
#endif /* QUIET_DISASSOC */
#ifdef WLTDLS
		if (TDLS_ENAB(wlc->pub) && wlc_tdls_quiet_down(wlc->tdls)) {
			WL_ASSOC(("wl%d: JOIN: skipping DISASSOC to %s since we are "
					    "quite down.\n", WLCWLUNIT(wlc), eabuf));
			send_disassociate = FALSE;
		}
#endif // endif
		/* Send disassociate packet and (attempt to) schedule callback */
		if (send_disassociate) {
			if (ETHER_ISNULLADDR(cfg->BSSID.octet)) {
				/* a NULL BSSID indicates that we have given up on our AP connection
				 * to the point that we will reassociate to it if we ever see it
				 * again. In this case, we should not send a disassoc
				 */
				WL_ASSOC(("wl%d: JOIN: skipping DISASSOC to %s since we lost "
					    "contact.\n", WLCWLUNIT(wlc), eabuf));
			} else if (wlc_radar_chanspec(wlc->cmi, current_bss->chanspec) ||
			        wlc_restricted_chanspec(wlc->cmi, current_bss->chanspec)) {
				/* note that if the channel is a radar or restricted channel,
				 * Permit sending disassoc packet if no subsequent processing
				 * is waiting  (indicated by the presence of callbcak routine)
				 */
				if (fn != NULL) {

					WL_ASSOC(("wl%d: JOIN: skipping DISASSOC to %s"
						"since chanspec %s is quiet and call back\n",
						WLCWLUNIT(wlc), eabuf,
						wf_chspec_ntoa_ex(current_bss->chanspec, chanbuf)));

				} else if (!wlc_quiet_chanspec(wlc->cmi, current_bss->chanspec) &&
					(current_bss->chanspec == WLC_BAND_PI_RADIO_CHANSPEC)) {

					void *pkt;

					WL_ASSOC(("wl%d: JOIN: sending DISASSOC to %s on "
					          "radar/restricted channel \n",
					          WLCWLUNIT(wlc), eabuf));

					pkt = wlc_senddisassoc(wlc, cfg, scb, &BSSID, &BSSID,
						&cfg->cur_etheraddr, DOT11_RC_DISASSOC_LEAVING);

					if (wlc_pcb_fn_register(wlc->pcb,
					      wlc_disassoc_done_quiet_chl, NULL, pkt)) {
						WL_ERROR(("wl%d: %s out of pkt callbacks\n",
						          wlc->pub->unit, __FUNCTION__));
					} else {
						mute_mode = OFF;
					}
				} else {
					WL_ASSOC(("wl%d: JOIN: Skipping DISASSOC to %s since "
						"present channel not home channel \n",
						WLCWLUNIT(wlc), eabuf));
				}
			} else if (current_bss->chanspec == WLC_BAND_PI_RADIO_CHANSPEC) {
				void *pkt;

				WL_ASSOC(("wl%d: JOIN: sending DISASSOC to %s\n",
				    WLCWLUNIT(wlc), eabuf));

				pkt = wlc_senddisassoc(wlc, cfg, scb, &BSSID, &BSSID,
					&cfg->cur_etheraddr, DOT11_RC_DISASSOC_LEAVING);
				if (pkt == NULL)
					WL_ASSOC(("wl%d: JOIN: error sending DISASSOC\n",
					          WLCWLUNIT(wlc)));
				else if (fn != NULL) {
					if (wlc_pcb_fn_register(wlc->pcb, fn, arg, pkt)) {
						WL_ERROR(("wl%d: %s out of pkt callbacks\n",
							wlc->pub->unit, __FUNCTION__));
					} else {
						WLPKTTAGBSSCFGSET(pkt, WLC_BSSCFG_IDX(cfg));
						/* the callback was registered, so clear fn local
						 * so it will not be called at the end of this
						 * function
						 */
						fn = NULL;
					}
				}
			}
		}

		/* reset scb state */
		if (scb) {

			wlc_scb_clearstatebit(scb, ASSOCIATED | PENDING_AUTH | PENDING_ASSOC);

#ifdef BCMCCX
			if (CAC_ENAB(wlc->pub))
				wlc_cac_param_reset_all(wlc->cac, scb);
#endif	/* BCMCCX */

			wlc_scb_disassoc_cleanup(wlc, scb);
		}

#if NCONF
		/* XXX stop the temporary WAR to improve Tx throughput in a non-N mode association
		 * to share the medium with other B-only STA: disable EDCRS when disassoc
		 */
		if (WLCISHTPHY(wlc->band) ||
		    (WLCISNPHY(wlc->band) && (D11REV_GE(wlc->pub->corerev, 16)))) {
			wlc_bmac_ifsctl_edcrs_set(wlc->hw, TRUE);
		}
#endif /* NCONF */
#if defined(WLAWDL)
		/* Update channel cache ctx for BSS connection */
		if (BSSCFG_STA(cfg)) {
			wlc_update_phy_chan_cache(cfg, cfg->current_bss->chanspec, FALSE);
			if (AWDL_SUPPORT(wlc->pub))
				wlc_awdl_set_infra_dfs_status(wlc,
					cfg->current_bss->chanspec, FALSE);
		}
#endif /* #if defined(WLAWDL) */
	}
	/* IBSS STA */
	else {
#ifdef EXT_STA
		if (WLEXTSTA_ENAB(wlc->pub)) {
			struct scb_iter scbiter;
			/* clear ibss peer flags */
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				if (SCB_IS_IBSS_PEER(scb))
					SCB_UNSET_IBSS_PEER(scb);

			}
		}
#endif /* EXT_STA */
		/* cache BSS type for disassoc indication */
		bsstype = DOT11_BSSTYPE_INDEPENDENT;

		wlc_ibss_disable(cfg);
	}

	/* Switch to BSS STA mode */
	wlc_bsscfg_set_infra_mode(wlc, cfg, TRUE);

	/* update association states */
#ifdef PSTA
	if (PSTA_ENAB(wlc->pub) && (cfg == wlc_bsscfg_primary(wlc))) {
		wlc_bsscfg_t *bsscfg;
		int32 idx;

		FOREACH_PSTA(wlc, idx, bsscfg) {
			/* Cleanup the proxy client state */
			wlc_sta_assoc_upd(bsscfg, FALSE);
		}
	}
#endif /* PSTA */
	wlc_sta_assoc_upd(cfg, FALSE);

	if (!wlc->pub->associated) {
		/* if auto shortslot, switch back to 11b compatible long slot */
		if (wlc->shortslot_override == WLC_SHORTSLOT_AUTO)
			wlc->shortslot = FALSE;
	}

	/* init protection configuration */
	wlc_prot_g_cfg_init(wlc->prot_g, cfg);
#ifdef WL11N
	if (N_ENAB(wlc->pub))
		wlc_prot_n_cfg_init(wlc->prot_n, cfg);
#endif // endif

	if (!ETHER_ISNULLADDR(cfg->BSSID.octet) &&
#ifdef WLAWDL
		!BSSCFG_AWDL(wlc, cfg) &&
#endif // endif
		TRUE) {
		wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS, &BSSID,
			DOT11_RC_DISASSOC_LEAVING, bsstype);
	}

	if (!AP_ACTIVE(wlc) && cfg == wlc->cfg) {
		WL_APSTA_UPDN(("wl%d: wlc_disassociate_client: wlc_clear_hw_association\n",
			wlc->pub->unit));
		wlc_clear_hw_association(cfg, mute_mode);
	} else {
		WL_APSTA_BSSID(("wl%d: wlc_disassociate_client -> wlc_clear_bssid\n",
			wlc->pub->unit));
		wlc_clear_bssid(cfg);
		wlc_bss_clear_bssid(cfg);
	}

	if (current_bss->bcn_prb) {
		MFREE(wlc->osh, current_bss->bcn_prb, current_bss->bcn_prb_len);
		current_bss->bcn_prb = NULL;
		current_bss->bcn_prb_len = 0;
	}

	WL_APSTA_UPDN(("wl%d: Reporting link down on config 0 (STA disassociating)\n",
	               WLCWLUNIT(wlc)));

	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) {
		wlc_phy_acimode_noisemode_reset(WLC_PI(wlc),
			current_bss->chanspec, TRUE, FALSE, TRUE);
	}

#ifdef WLAWDL
	if (!BSSCFG_AWDL(wlc, cfg))
#endif // endif
	{
		wlc_link(wlc, FALSE, &BSSID, cfg, WLC_E_LINK_DISASSOC);
#ifdef WLAWDL
		WL_APSTA_UPDN(("wl%d: Reporting link down on config %d (STA disassociating)\n",
		WLCWLUNIT(wlc), cfg->_idx));
#endif // endif
	}

	/* reset rssi moving average */
	wlc_lq_rssi_snr_noise_reset_ma(wlc, cfg, WLC_RSSI_INVALID,
		WLC_SNR_INVALID, WLC_NOISE_INVALID);
	wlc_lq_rssi_event_update(cfg);

#ifdef WLLED
	wlc_led_event(wlc->ledh);
#endif // endif

	/* notifying interested parties of the event... */
	wlc_bss_assoc_state_notif(wlc, cfg, AS_NONE, AS_IDLE);

	/* disable radio due to end of association */
	WL_MPC(("wl%d: disassociation wlc->pub->associated==FALSE, update mpc\n", wlc->pub->unit));
	wlc_radio_mpc_upd(wlc);

	/* call the given callback fn if it has not been taken care of with
	 * a disassoc packet callback.
	 */
	if (fn)
		(*fn)(wlc, TX_STATUS_NO_ACK, arg);

	/* clean up... */
	bzero(target_bss->BSSID.octet, ETHER_ADDR_LEN);
	bzero(cfg->BSSID.octet, ETHER_ADDR_LEN);

	return (0);
} /* _wlc_disassociate_client */

static int
wlc_setssid_disassociate_client(wlc_bsscfg_t *cfg)
{
	return _wlc_disassociate_client(cfg, TRUE,
	        wlc_setssid_disassoc_complete, (void *)(uintptr)cfg->ID);
}

int
wlc_disassociate_client(wlc_bsscfg_t *cfg, bool send_disassociate)
{
	return _wlc_disassociate_client(cfg, send_disassociate, NULL, NULL);
}

static void
wlc_assoc_success(wlc_bsscfg_t *cfg, struct scb *scb)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *prev_scb;

	ASSERT(scb != NULL);

	wlc_scb_clearstatebit(scb, PENDING_AUTH | PENDING_ASSOC);

	wlc_assoc_timer_del(wlc, cfg);

	/* clean up before leaving the BSS */
	if (cfg->BSS && cfg->associated) {
		prev_scb = wlc_scbfindband(wlc, cfg, &cfg->prev_BSSID,
			CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
		if (prev_scb) {
#ifdef WLFBT
			if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
				CAC_ENAB(wlc->pub) && (wlc->cac != NULL)) {
				wlc_cac_copy_state(wlc->cac, prev_scb, scb);
			}
#endif /* WLFBT */
			wlc_scb_clearstatebit(prev_scb, ASSOCIATED | AUTHORIZED);

			/* XXX APSTA: Should we clear SCB_MYAP here?  Still
			 * authenticated...
			 */

			/* delete old AP's pairwise key */
			wlc_scb_disassoc_cleanup(wlc, prev_scb);
		}
	}

#ifdef MFP
	/* MFP flags settings after reassoc when prev_scb and scb are same */
	/* we have a valid combination of MFP flags */
	scb->flags2 &= ~(SCB2_MFP | SCB2_SHA256);
	if ((BSSCFG_IS_MFP_CAPABLE(cfg)) &&
		(cfg->target_bss->wpa2.flags & RSN_FLAGS_MFPC))
		scb->flags2 |= SCB2_MFP;
	if ((cfg->WPA_auth & (WPA2_AUTH_1X_SHA256 | WPA2_AUTH_PSK_SHA256)) &&
		(cfg->target_bss->wpa2.flags & RSN_FLAGS_SHA256))
		scb->flags2 |= SCB2_SHA256;
	WL_ASSOC(("wl%d: %s: turn MFP on %s\n", wlc->pub->unit, __FUNCTION__,
		((scb->flags2 & SCB2_SHA256) ? "with sha256" : "")));
#endif /* MFP */
	/* clear PM state */
	wlc_set_pmoverride(cfg, FALSE);
	wlc_update_bcn_info(cfg, FALSE);

	/* update scb state */
	wlc_scb_setstatebit(scb, ASSOCIATED);

	/* init per scb WPA_auth */
	scb->WPA_auth = cfg->WPA_auth;
	WL_WSEC(("wl%d: WPA_auth 0x%x\n", wlc->pub->unit, scb->WPA_auth));

	/* adopt the BSS parameters */
	wlc_join_adopt_bss(cfg);

	/* 11g hybrid coex cause jerky mouse, disable for now. do not apply for ECI chip for now */
	if (!SCB_HT_CAP(scb) && !BCMECICOEX_ENAB(wlc))
		wlc_mhf(wlc, MHF3, MHF3_BTCX_SIM_RSP, 0, WLC_BAND_2G);

#if NCONF
	/* temporary WAR to improve Tx throughput in a non-N mode association */
	/* to share the medium with other B-only STA: Enable EDCRS when assoc */
	if (WLCISHTPHY(wlc->band) ||
	    (WLCISNPHY(wlc->band) && (D11REV_GE(wlc->pub->corerev, 16)))) {
		wlc_bmac_ifsctl_edcrs_set(wlc->hw, ((scb->flags & SCB_HTCAP) != 0));
	}
#endif /* NCONF */

#ifdef WLWNM
	/* Check DMS req for primary infra STA */
	if (WLWNM_ENAB(wlc->pub)) {
		if ((!WSEC_ENABLED(scb->wsec) || WSEC_WEP_ENABLED(scb->wsec)) &&
			(cfg == wlc->cfg)) {
			wlc_wnm_check_dms_req(wlc, &scb->ea);
		}
		wlc_wnm_scb_assoc(wlc, scb);
	}
#endif /* WLWNM */
} /* wlc_assoc_success */

void
wlc_auth_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr* addr,
	uint auth_status, uint auth_type)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	bool more_to_do_after_event = FALSE;
	struct scb *scb;
	void *pkt;
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_ASSOC */
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	if (addr != NULL)
		bcm_ether_ntoa(addr, eabuf);
	else
		strncpy(eabuf, "<NULL>", sizeof(eabuf) - 1);
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_ASSOC */

	if (status == WLC_E_STATUS_UNSOLICITED)
		goto do_event;

	/* SAE case upper layer auth requst timeout may happen */
	if ((as->state != AS_SENT_AUTH_UP) && (as->state != AS_START_EXT_AUTH)) {
		if (!(as->state == AS_SENT_AUTH_1 || as->state == AS_SENT_AUTH_3 ||
			as->state == AS_SENT_FTREQ))
			goto do_event;
	}

	/* Clear pending bits */
	scb = addr ? wlc_scbfind(wlc, cfg, addr): NULL;
	if (scb)
		wlc_scb_clearstatebit(scb, PENDING_AUTH | PENDING_ASSOC);

	if (status != WLC_E_STATUS_TIMEOUT) {
		wlc_assoc_timer_del(wlc, cfg);
	}
	if (status == WLC_E_STATUS_SUCCESS) {
		WL_ASSOC(("wl%d: JOIN: authentication success\n",
		          WLCWLUNIT(wlc)));
		ASSERT(scb != NULL);
		wlc_scb_setstatebit(scb, AUTHENTICATED | PENDING_ASSOC);
		/* FIXME: should probably move the shortpreamble update furthur on
		 * to the assoc resp handling using cap bits from the assoc resp
		 * frame
		 */
		if (!BSSCFG_EXT_AUTH(cfg))
		{
			scb->flags &= ~SCB_SHORTPREAMBLE;
			if ((target_bss->capability & DOT11_CAP_SHORT) != 0)
				scb->flags |= SCB_SHORTPREAMBLE;
			WL_APSTA(("wl%d: WLC_E_AUTH for %s, forcing MYAP flag.\n",
				wlc->pub->unit, eabuf));
			scb->flags |= SCB_MYAP;
			wlc_assoc_change_state(cfg, AS_SENT_ASSOC);
			pkt = wlc_join_assoc_start(wlc, cfg, scb, target_bss, cfg->associated);

#if defined(WLP2P) && defined(BCMDBG)
			if (WL_P2P_ON()) {
				int bss = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
				uint16 state = wlc_mcnx_read_shm(wlc->mcnx,
						M_P2P_BSS_ST(bss));
				uint16 next_noa = wlc_mcnx_read_shm(wlc->mcnx,
						M_P2P_BSS_N_NOA(bss));
				uint16 hps = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_HPS);

				WL_P2P(("wl%d: %s: queue ASSOC at tick 0x%x ST 0x%04X "
							"N_NOA 0x%X HPS 0x%04X\n",
							wlc->pub->unit, __FUNCTION__,
							R_REG(wlc->osh, &wlc->regs->tsf_timerlow),
							state, next_noa, hps));
			}
#endif /* WLP2P && BCMDBG */
			wlc_assoc_timer_del(wlc, cfg);
			if (pkt != NULL)
				wlc_pcb_fn_register(wlc->pcb, wlc_assocreq_complete,
						(void *)(uintptr)cfg->ID, pkt);
			else
				wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
		}
		goto do_event;
	} else if (status == WLC_E_STATUS_TIMEOUT) {
		WL_ASSOC(("wl%d: JOIN: timeout waiting for authentication "
			"response, assoc_state %d\n",
			WLCWLUNIT(wlc), as->state));
#ifdef WL_AUTH_METHOD_SHARED_OPEN
		/* XXX WAR: Some APs do not support DOT11_SHARED_KEY.
		 * So there is 802.11 ACK but it does not send response and timeout occurs.
		 * Try to send auth frame with OPEN System.
		 */
		if (cfg->openshared && cfg->auth_atmptd == DOT11_SHARED_KEY &&
			(target_bss->capability & DOT11_CAP_PRIVACY) &&
			WSEC_WEP_ENABLED(cfg->wsec) &&
			WSEC_BSS_DEFAULT_KEY(cfg) &&
			(WSEC_BSS_DEFAULT_KEY(cfg)->algo == CRYPTO_ALGO_WEP1 ||
			WSEC_BSS_DEFAULT_KEY(cfg)->algo == CRYPTO_ALGO_WEP128)) {
			wlc_bss_info_t* bi =
			wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
			/* Try the current target BSS with DOT11_OPEN_SYSTEM */
			cfg->auth_atmptd = DOT11_OPEN_SYSTEM;
			wlc_assoc_change_state(cfg, AS_SENT_AUTH_1);
			pkt = wlc_sendauth(cfg, &scb->ea, &bi->BSSID, scb,
				cfg->auth_atmptd, 1, DOT11_SC_SUCCESS,
				NULL, ((bi->capability & DOT11_CAP_SHORT) != 0));
			if (pkt == NULL)
				wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
			return;
		}
#endif /* WL_AUTH_METHOD_SHARED_OPEN */
	} else if (status == WLC_E_STATUS_NO_ACK) {
		WL_ASSOC(("wl%d.%d: JOIN: authentication failure, no ack from %s\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), eabuf));
#if defined(WLFBT)
		/* No response from current AP for FT Request frame,
		 * join target AP using FBT over-the-air.
		 */
		if (BSSCFG_IS_FBT(cfg) && (target_bss->flags2 & WLC_BSS_OVERDS_FBT) &&
		    (as->state == AS_SENT_FTREQ)) {
			wlc_join_BSS(cfg,
			wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last]);
			return;
		}
#endif /* WLFBT */
	} else if (status == WLC_E_STATUS_FAIL) {
		wlc_key_info_t key_info;
#ifdef PSTA
		wlc_keymgmt_get_bss_tx_key(wlc->keymgmt,
			(BSSCFG_PSTA(cfg) ? wlc_bsscfg_primary(wlc) : cfg), FALSE, &key_info);
#else /* PSTA */
		wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, cfg, FALSE, &key_info);
#endif /* PSTA */
		WL_ASSOC(("wl%d: JOIN: authentication failure status %d from %s\n",
		          WLCWLUNIT(wlc), (int)auth_status, eabuf));
		ASSERT(scb != NULL);
		wlc_scb_clearstatebit(scb, AUTHENTICATED);

#if defined(WLFBT)
		if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
			((auth_status == DOT11_SC_ASSOC_R0KH_UNREACHABLE) ||
			(auth_status == DOT11_SC_INVALID_PMKID))) {
			wlc_fbt_clear_ies(wlc->fbt, cfg);
			if (auth_status == DOT11_SC_ASSOC_R0KH_UNREACHABLE)
				wlc_assoc_abort(cfg);
		}
#endif /* WLFBT */
		if (BSSCFG_EXT_AUTH(cfg) && (auth_status == DOT11_SC_ANTICLOG_TOCKEN_REQUIRED))
			goto do_event;

		if (cfg->openshared &&
#ifdef WL_AUTH_METHOD_SHARED_OPEN
			cfg->auth_atmptd == DOT11_SHARED_KEY &&
#else
			cfg->auth_atmptd == DOT11_OPEN_SYSTEM &&
#endif /* WL_AUTH_METHOD_SHARED_OPEN */
			auth_status == DOT11_SC_AUTH_MISMATCH &&
			(target_bss->capability & DOT11_CAP_PRIVACY) &&
			WSEC_WEP_ENABLED(cfg->wsec) &&
			(key_info.algo == CRYPTO_ALGO_WEP1 ||
			key_info.algo == CRYPTO_ALGO_WEP128)) {
			wlc_bss_info_t* bi =
			wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
#ifdef WL_AUTH_METHOD_SHARED_OPEN
			/* Try the current target BSS with DOT11_OPEN_SYSTEM */
			cfg->auth_atmptd = DOT11_OPEN_SYSTEM;
#else
			/* Try the current target BSS with DOT11_SHARED_KEY */
			cfg->auth_atmptd = DOT11_SHARED_KEY;
#endif /* WL_AUTH_METHOD_SHARED_OPEN */
			wlc_assoc_change_state(cfg, AS_SENT_AUTH_1);
			pkt = wlc_sendauth(cfg, &scb->ea, &bi->BSSID, scb,
				cfg->auth_atmptd, 1, DOT11_SC_SUCCESS,
				NULL, ((bi->capability & DOT11_CAP_SHORT) != 0));
			if (pkt == NULL)
				wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT + 10, 0);
			return;
		}
#if defined(BCMCCX) && defined(BCMINTSUP)
		if (SUP_ENAB(wlc->pub) && CCX_ENAB(wlc->pub) && wlc->ccx->leap_on &&
			(auth_status == DOT11_SC_AUTH_MISMATCH))
			wlc_ccx_rogueap_update(wlc->ccxsup, cfg, CCX_ROGUE_INVALID_AUTH, addr);
#endif /* BCMCCX && BCMINTSUP */
	} else if (status == WLC_E_STATUS_ABORT) {
		WL_ASSOC(("wl%d: JOIN: authentication aborted\n", WLCWLUNIT(wlc)));
		goto do_event;
	} else {
		WL_ERROR(("wl%d: %s, unexpected status %d\n",
		          WLCWLUNIT(wlc), __FUNCTION__, (int)status));
		goto do_event;
	}
	more_to_do_after_event = TRUE;

do_event:
#ifdef WL_SAE
	if (!(BSSCFG_EXT_AUTH(cfg) && auth_status == DOT11_SC_ANTICLOG_TOCKEN_REQUIRED))
#endif /* WL_SAE */
		wlc_bss_mac_event(wlc, cfg, WLC_E_AUTH, addr, status, auth_status, auth_type, 0, 0);

	if (!more_to_do_after_event)
		return;

	/* This is when status != WLC_E_STATUS_SUCCESS... */

#if defined(BCMCCX) && defined(CCX_SDK)
	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx))
		wlc_ccx_mgmt_status_hdlr(wlc->ccx, addr, auth_status, TRUE, NULL, 0);
#endif	/* BCMCCX && CCX_SDK */

	/* Try current BSS again */
	if ((status == WLC_E_STATUS_NO_ACK || status == WLC_E_STATUS_TIMEOUT) &&
	    (as->bss_retries < as->retry_max)) {
		WL_ASSOC(("wl%d: Retrying authentication (%d)...\n",
		          WLCWLUNIT(wlc), as->bss_retries));
		wlc_join_bss_start(cfg);
	}
	else { /* Try next BSS */
		WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_AUTH_FAIL_ID,
		           WL_LOG_LEVEL_ERR, 0, status, eabuf);
		wlc_join_attempt(cfg);
	}
} /* wlc_auth_complete */

static void
wlc_assoc_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr* addr,
	uint assoc_status, bool reassoc, uint bss_type)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	bool more_to_do_after_event = FALSE;
	struct scb *scb;
#if defined(BCMDBG_ERR) || defined(WLMSG_ASSOC)
	const char* action = (reassoc)?"reassociation":"association";
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_ASSOC */
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLEXTLOG)
	if (addr != NULL)
		bcm_ether_ntoa(addr, eabuf);
	else
		strncpy(eabuf, "<NULL>", sizeof(eabuf) - 1);
#endif /* BCMDBG || BCMDBG_ERR || WLMSG_ASSOC */

	if (status == WLC_E_STATUS_UNSOLICITED)
		goto do_event;

	if (status == WLC_E_STATUS_SUCCESS) {
		WL_ASSOC(("wl%d: JOIN: %s success ...\n", WLCWLUNIT(wlc), action));
		if (!reassoc)
			wlc_bsscfg_up(wlc, cfg);
#ifdef SLAVE_RADAR
		if (WL11H_STA_ENAB(wlc) && wlc_dfs_get_radar(wlc->dfs) &&
			(cfg->roam->reason == WLC_E_REASON_RADAR_DETECTED)) {
			if (!wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC)) {
				cfg->pm->PMmodeChangeDisabled = FALSE;
				wlc_set_pm_mode(wlc, cfg->pm->PM_oldvalue, cfg);
				wlc->mpc = TRUE;
				wlc_radio_mpc_upd(wlc);
			}
		}
#endif	/* SLAVE_RADAR */
		if (WOWL_ENAB(wlc->pub) && cfg == wlc->cfg)
			cfg->roam->roam_on_wowl = FALSE;
		/* Restart the ap's in case of a band change */
		if (AP_ACTIVE(wlc)) {
			/* performing a channel change,
			 * all up ap scb's need to be cleaned up
			 */
			wlc_bsscfg_t *apcfg;
			int idx;
			bool mchan_stago_disab =
#ifdef WLMCHAN
				!MCHAN_ENAB(wlc->pub) ||
				wlc_mchan_stago_is_disabled(wlc->mchan);
#else
				TRUE;
#endif // endif

#ifdef AP
#ifdef WLMCHAN
			if (!MCHAN_ENAB(wlc->pub))
#endif // endif
				FOREACH_UP_AP(wlc, idx, apcfg) {
					/* Clean up scbs only when there is a chanspec change */
					if (WLC_BAND_PI_RADIO_CHANSPEC !=
						apcfg->current_bss->chanspec)
							wlc_ap_bsscfg_scb_cleanup(wlc, apcfg);
				}
#endif /* AP */

			FOREACH_UP_AP(wlc, idx, apcfg) {
				if (BSSCFG_AP_MCHAN_DISABLED(wlc, apcfg) || mchan_stago_disab) {
					wlc_txflowcontrol_override(wlc, apcfg->wlcif->qi, OFF,
						TXQ_STOP_FOR_PKT_DRAIN);
					wlc_scb_update_band_for_cfg(wlc, apcfg,
						WLC_BAND_PI_RADIO_CHANSPEC);
				}
			}
			wlc_restart_ap(wlc->ap);
		}
#if defined(BCMSUP_PSK) && defined(WLFBT)
		if (BSSCFG_IS_FBT(cfg) && SUP_ENAB(wlc->pub) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
			reassoc)
			wlc_sup_clear_replay(wlc->idsup, cfg);
#endif /* BCMSUP_PSK && WLFBT */
		goto do_event;
	}

	if (!(as->state == AS_SENT_ASSOC || as->state == AS_REASSOC_RETRY))
		goto do_event;

	/* Clear pending bits */
	scb = addr ? wlc_scbfind(wlc, cfg, addr) : NULL;
	if (scb)
		wlc_scb_clearstatebit(scb, PENDING_AUTH | PENDING_ASSOC);

	if (status != WLC_E_STATUS_TIMEOUT) {
		wlc_assoc_timer_del(wlc, cfg);
	}
	if (status == WLC_E_STATUS_TIMEOUT) {
		WL_ASSOC(("wl%d: JOIN: timeout waiting for %s response\n",
		    WLCWLUNIT(wlc), action));
	} else if (status == WLC_E_STATUS_NO_ACK) {
		WL_ASSOC(("wl%d: JOIN: association failure, no ack from %s\n",
		    WLCWLUNIT(wlc), eabuf));
	} else if (status == WLC_E_STATUS_FAIL) {
		WL_ASSOC(("wl%d: JOIN: %s failure %d\n",
		    WLCWLUNIT(wlc), action, (int)assoc_status));
#if defined(BCMCCX) && defined(BCMINTSUP)
		/* Some IOS APs report auth failure in the assoc response... */
		if (SUP_ENAB(wlc->pub) && CCX_ENAB(wlc->pub) && wlc->ccx->leap_on &&
		    (assoc_status == DOT11_SC_AUTH_MISMATCH) && (addr != NULL))
			wlc_ccx_rogueap_update(wlc->ccxsup, cfg, CCX_ROGUE_INVALID_AUTH, addr);
#endif /* BCMCCX && BCMINTSUP */
	} else if (status == WLC_E_STATUS_ABORT) {
		WL_ASSOC(("wl%d: JOIN: %s aborted\n", wlc->pub->unit, action));
		goto do_event;
	} else {
		WL_ERROR(("wl%d: %s: %s, unexpected status %d\n",
		    WLCWLUNIT(wlc), __FUNCTION__, action, (int)status));
		goto do_event;
	}
	more_to_do_after_event = TRUE;

do_event:
	wlc_bss_mac_event(wlc, cfg, reassoc ? WLC_E_REASSOC : WLC_E_ASSOC, addr,
		status, assoc_status, bss_type, as->resp, as->resp_len);

	if (!more_to_do_after_event)
		return;

	/* This is when status != WLC_E_STATUS_SUCCESS... */

#if defined(BCMCCX) && defined(CCX_SDK)
	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx))
		wlc_ccx_mgmt_status_hdlr(wlc->ccx, addr, assoc_status, TRUE, NULL, 0);
#endif	/* BCMCCX && CCX_SDK */

	/* Try current BSS again */
	if ((status == WLC_E_STATUS_NO_ACK || status == WLC_E_STATUS_TIMEOUT) &&
	    (as->bss_retries < as->retry_max)) {
		WL_ASSOC(("wl%d: Retrying association (%d)...\n",
		          WLCWLUNIT(wlc), as->bss_retries));
		wlc_join_bss_start(cfg);
	}
	/* Try next BSS */
	else {
		WLC_EXTLOG(wlc, LOG_MODULE_ASSOC, FMTSTR_ASSOC_FAIL_ID,
			WL_LOG_LEVEL_ERR, 0, status, eabuf);
		wlc_join_attempt(cfg);
	}
} /* wlc_assoc_complete */

static void
wlc_set_ssid_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr, uint bss_type)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	bool retry = FALSE;
	bool assoc_state = FALSE;

	/* Association state machine is halting, clear state */
	wlc_bss_list_free(wlc, wlc->as_info->join_targets);

	/* flag to indicate connection completion when abort */
	if (status == WLC_E_STATUS_ABORT)
		assoc_state = ((bss_type == DOT11_BSSTYPE_INDEPENDENT) ?
		               TRUE :
		               (cfg->assoc->state > AS_IDLE &&
		                cfg->assoc->state < AS_WAIT_RCV_BCN));

	/* when going from scan to idle, due to failure  */
	if (as->state == AS_SCAN && status == WLC_E_STATUS_FAIL) {
		/* if here due to failure and we're still in scan state, then abort our assoc */
		if (SCAN_IN_PROGRESS(wlc->scan)) {
			WL_ASSOC(("wl%d:%s: abort association in process\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			/* assoc was in progress when this condition occurred */
			/* if it's not, assert to catch the unexpected condition */
			ASSERT(AS_IN_PROGRESS(wlc));
			wlc_assoc_abort(cfg);
		}
	}

	/* Association state machine is halting, clear state and allow core to sleep */
	as->type = AS_NONE;
	wlc_assoc_change_state(cfg, AS_IDLE);

	/* Stop the timer started while entering AS_WAIT_RCV_BCN state */
	wlc_assoc_timer_del(wlc, cfg);

	if ((status != WLC_E_STATUS_SUCCESS) && (status != WLC_E_STATUS_FAIL) &&
	    (status != WLC_E_STATUS_NO_NETWORKS) && (status != WLC_E_STATUS_ABORT))
		WL_ERROR(("wl%d: %s: unexpected status %d\n",
		          WLCWLUNIT(wlc), __FUNCTION__, (int)status));

	if (status != WLC_E_STATUS_SUCCESS) {
		wl_assoc_params_t *assoc_params;
		assoc_params = wlc_bsscfg_assoc_params(cfg);

		WL_ASSOC(("wl%d: %s: failed status %u\n",
		          WLCWLUNIT(wlc), __FUNCTION__, status));

		/*
		 * If we are here because of a status abort don't check mpc,
		 * it is responsibility of the caller
		*/
		if (status != WLC_E_STATUS_ABORT)
			wlc_radio_mpc_upd(wlc);

		if ((status == WLC_E_STATUS_NO_NETWORKS ||
			status == WLC_E_STATUS_FAIL) &&
		    cfg->enable) {
			/* retry if configured */

			/* XXX These two kinds of retries should not be enabled
			 * at the same time. Join (ESS) retry is to overcome
			 * the possible interference in an BTC environment, vs.
			 * assoc timer based (STA) retry is to connect the STA in APSTA
			 * mode to an AP if the AP is available later after the STA is started.
			 * In case they are indeed all configured let the Join
			 * retry takes place first.
			 */
			/* XXX Retry Join only when we haven't tried 802.11
			 * association procedures...
			 */
			if (as->ess_retries < as->retry_max) {
				WL_ASSOC(("wl%d: Retrying join (%d)...\n",
				          WLCWLUNIT(wlc), as->ess_retries));
				wlc_join_start(cfg, wlc_bsscfg_scan_params(cfg),
				               wlc_bsscfg_assoc_params(cfg));
				retry = TRUE;
			}
			else if ((assoc_params != NULL) && assoc_params->bssid_cnt) {
				int chidx = assoc_params->chanspec_num;
				int bccnt = assoc_params->bssid_cnt;

				WL_ASSOC(("wl%d: join failed, index %d of %d (%s)\n",
					WLCWLUNIT(wlc), chidx, bccnt,
					((chidx + 1) < bccnt) ? "continue" : "fail"));

				if (++chidx < bccnt) {
					assoc_params->chanspec_num = chidx;
					as->ess_retries = 0;
					wlc_join_start(cfg, wlc_bsscfg_scan_params(cfg),
					wlc_bsscfg_assoc_params(cfg));
					retry = TRUE;
				}
			}
			else if ((wlc->as_info->sta_retry_time > 0) &&
					(cfg->auth != WL_AUTH_SAE_KEY)) {
				/* Sae Auth Start is handled by external supplicant
				 * So auth retry have to be initiated by upper layer
				 */
				wl_del_timer(wlc->wl, as->timer);
				wl_add_timer(wlc->wl, as->timer,
					wlc->as_info->sta_retry_time*1000, 0);
				as->rt = TRUE;
			}
		} else if (status != WLC_E_STATUS_ABORT &&
			cfg->enable && (assoc_params != NULL) && assoc_params->bssid_cnt) {
			int chidx = assoc_params->chanspec_num;
			int bccnt = assoc_params->bssid_cnt;

			WL_ASSOC(("wl%d: join failed, index %d of %d (%s)\n",
				WLCWLUNIT(wlc), chidx, bccnt,
				((chidx + 1) < bccnt) ? "continue" : "fail"));

			if (++chidx < bccnt) {
				assoc_params->chanspec_num = chidx;
				as->ess_retries = 0;
				wlc_join_start(cfg, wlc_bsscfg_scan_params(cfg),
					wlc_bsscfg_assoc_params(cfg));
				retry = TRUE;
			}
		}
	}

	/* no more processing if we are going to retry */
	if (retry)
		return;

#ifdef WLBTAMP
	if (BSS_BTA_ENAB(wlc, cfg) && status != WLC_E_STATUS_SUCCESS) {
		wl_join_assoc_params_t *assoc_params;
		struct scb *scb;

		/* XXX at this point cfg->BSSID may not have been set so use
		 * the one we saved from wlc_join_start().
		 */
		assoc_params = wlc_bsscfg_assoc_params(cfg);
		ASSERT(assoc_params != NULL);

		scb = wlc_scbfind(wlc, cfg, &assoc_params->bssid);
		if (scb != NULL) {
			/* BT-AMP join post-processing:
			 * on failure
			 * generate Physical Link Complete with status.
			 */
			wlc_bta_join_complete(wlc->bta, scb, 1);
		}
	}
#endif /* WLBTAMP */

	/* free join scan/assoc params */
	if (status == WLC_E_STATUS_SUCCESS) {
		wlc_bsscfg_scan_params_reset(wlc, cfg);
		wlc_bsscfg_assoc_params_reset(wlc, cfg);
	}

	/* APSTA: complete any deferred AP bringup */
	if (AP_ENAB(wlc->pub) && APSTA_ENAB(wlc->pub))
		wlc_restart_ap(wlc->ap);

	/* allow AP to beacon and respond to probe requests */
	if (AP_ACTIVE(wlc)) {
		/* validate the phytxctl for the beacon before turning it on */
		wlc_validate_bcn_phytxctl(wlc, NULL);
	}

	/* AIBSS requires MCTL_AP for beacon generation */
	if (!AIBSS_ENAB(wlc->pub) || !BSSCFG_IBSS(cfg)) {
		wlc_ap_mute(wlc, FALSE, cfg, -1);
	}

	/* N.B.: assoc_state check passed through status event field */
	/* N.B.: bss_type passed through auth_type event field */
	wlc_bss_mac_event(wlc, cfg, WLC_E_SET_SSID, addr, status, assoc_state, bss_type,
	                  cfg->SSID, cfg->SSID_len);

} /* wlc_set_ssid_complete */

void
wlc_roam_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr, uint bss_type)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_roam_t *roam = cfg->roam;
	bool assoc_recreate = FALSE;
	wlc_bsscfg_t* apcfg = NULL;
	int i = 0;

#ifdef WLMCHAN
	int ret = BCME_NOMEM;
#endif // endif
#ifdef WLFBT
	uint8 ft_key[WPA_MIC_KEY_LEN + WPA_ENCR_KEY_LEN];
#endif // endif

	if (status == WLC_E_STATUS_SUCCESS) {
		WL_ASSOC(("wl%d: JOIN: roam success\n", WLCWLUNIT(wlc)));
#if defined(PSTA)
		/* Re-associate PSTA clients after primary has roamed */
		if (PSTA_ENAB(wlc->pub) && (cfg == wlc_bsscfg_primary(wlc))) {
			wlc_psta_reassoc_all(wlc->psta, cfg);
		}
#endif /* PSTA */
	} else if (status == WLC_E_STATUS_FAIL) {
		WL_ASSOC(("wl%d: JOIN: roam failure\n", WLCWLUNIT(wlc)));
#ifdef AP
		if (wlc_join_check_ap_need_csa(wlc, cfg, cfg->current_bss->chanspec,
		                               AS_WAIT_FOR_AP_CSA_ROAM_FAIL)) {
			WL_ASSOC(("wl%d.%d: ROAM FAIL: "
			          "%s delayed due to ap active, wait for ap CSA\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			return;
		}
#endif /* AP */
	} else if (status == WLC_E_STATUS_NO_NETWORKS) {
		WL_ASSOC(("wl%d: JOIN: roam found no networks\n", WLCWLUNIT(wlc)));

		if (AP_ENAB(wlc->pub) && APSTA_ENAB(wlc->pub) &&
			(MAP_ENAB(cfg) || DWDS_ENAB(cfg) ||
#if defined(WET) || defined(WET_DONGLE)
			(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
			(cfg->_psta)) &&
			((cfg->disable_ap_up == FALSE))) {
			/* condition where STA interface of Radio started to roam
			 * but could not able to find target BSS's beacon, In this
			 * scenario down all AP interface as MBSS
			 */
			FOREACH_UP_AP(wlc, i, apcfg) {
				if ((apcfg->enable) && (apcfg->up)) {
					wlc_bsscfg_down(wlc, apcfg);
					cfg->disable_ap_up = TRUE;
				}
			}

			wlc_set_dfs_cacstate(wlc->dfs, OFF);
			wlc_disassociate_client(cfg, FALSE);
			cfg->roam->active = FALSE;
		}
	} else if (status == WLC_E_STATUS_ABORT) {
		WL_ASSOC(("wl%d: JOIN: roam aborted\n", WLCWLUNIT(wlc)));
	} else {
		WL_ERROR(("wl%d: %s: unexpected status %d\n",
		    WLCWLUNIT(wlc), __FUNCTION__, (int)status));
	}
#ifdef WLWNM
	if (WLWNM_ENAB(wlc->pub)) {
		/* If bsstrans_resp is pending here then we don't have a roam candidate.
		 * Send a bsstrans_reject.
		*/
		wlc_wnm_bsstrans_roamscan_complete(wlc,
			DOT11_BSSTRANS_RESP_STATUS_REJ_BSS_LIST_PROVIDED, NULL);
	}
#endif /* WLWNM */
#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		/* Cancel the suppress-ssid-after-hibernate flag for Vista */
		if (status == WLC_E_STATUS_SUCCESS)
			wlc->scan_suppress_ssid = FALSE;
	}
#endif /* EXT_STA */

	/* Association state machine is halting, clear state */
	wlc_bss_list_free(wlc, wlc->as_info->join_targets);

	if (ASSOC_RECREATE_ENAB(wlc->pub))
		assoc_recreate = (as->type == AS_RECREATE);

	as->type = AS_NONE;
	wlc_assoc_change_state(cfg, AS_IDLE);

#ifdef BCMCCX
	/* start normal roam scan if fast roam failed and not aborted */
	if (status != WLC_E_STATUS_SUCCESS && status != WLC_E_STATUS_ABORT) {
		if (CCX_ENAB(wlc->pub) && wlc->ccx->fast_roam && !roam->off) {
			WL_ASSOC(("wl%d: ROAM: CCX FAST, failed...issue normal "
				    "roam scan\n", WLCWLUNIT(wlc)));
			wlc->ccx->fast_roam = FALSE;
			/* save original roaming reason code */
			wlc->ccx->orig_reason = roam->reason;
			wlc_roam_scan(cfg, WLC_E_REASON_FAST_ROAM_FAILED, NULL, 0);
			goto do_event;
		}
	}
#endif /* BCMCCX */

	/* If a roam fails, restore state to that of our current association */
	if (status == WLC_E_STATUS_FAIL) {
		wlc_bss_info_t *current_bss = cfg->current_bss;
		chanspec_t chanspec = current_bss->chanspec;

		/* restore old channel */
#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub)) {
			WL_MCHAN(("wl%d.%d: %s: Restore chanctx for 0x%x\n",
			          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, chanspec));
			ret = wlc_mchan_create_bss_chan_context(wlc, cfg, chanspec);
		}
#endif // endif
		if ((WLC_BAND_PI_RADIO_CHANSPEC != chanspec)) {
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, chanspec);
			wlc_enable_mac(wlc);
		}
#ifdef WLMCHAN
		/* if we did not change channels we still might need to update
		 * the primay and active queue
		 */
		if (MCHAN_ENAB(wlc->pub) && !ret) {
			wlc_mchan_set_priq(wlc->mchan, cfg);
		}
#endif // endif

		/* restore old basic rates */
		wlc_rate_lookup_init(wlc, &current_bss->rateset);
	}

	/* APSTA: complete any deferred AP bringup */
	if (AP_ENAB(wlc->pub) && APSTA_ENAB(wlc->pub))
		wlc_restart_ap(wlc->ap);

	/* allow AP to beacon and respond to probe requests */
	if (AP_ACTIVE(wlc)) {
		/* validate the phytxctl for the beacon before turning it on */
		wlc_validate_bcn_phytxctl(wlc, NULL);
	}
	wlc_ap_mute(wlc, FALSE, cfg, -1);

#ifdef BCMCCX
do_event:
#endif /* BCMCCX */

	/* N.B.: roam reason passed through status event field */
	/* N.B.: bss_type passed through auth_type event field */
	if (status == WLC_E_STATUS_SUCCESS) {
#ifdef WLFBT
		/* Send up ft_key in case of FT AUTH, and the supplicant will take the information
		 * only when it is valid at the FBT roaming.
		 */
		if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
			wlc_fbt_get_kck_kek(wlc->fbt, cfg, ft_key);
			wlc_bss_mac_event(wlc, cfg, WLC_E_BSSID, addr, status,
				roam->reason, bss_type, ft_key, sizeof(ft_key));
		} else
#endif /* WLFBT */
		wlc_bss_mac_event(wlc, cfg, WLC_E_BSSID, addr, 0, status, bss_type, 0, 0);
	}
	wlc_bss_mac_event(wlc, cfg, WLC_E_ROAM, addr, status, roam->reason, bss_type, 0, 0);
	/* XXX REVISIT WHERE THIS IS SUPPOSE TO GO: BEFORE OR AFTER WLC_E_ROAM EVENT.
	 * I believe I have left this the same as it was before removing processing
	 * from wlc_event_handle().
	 *
	 * Note there are two events, one in wlc_link and the explicit WLC_E_DISASOC for
	 * EXT_STA.  I think these events use to happen after the WLC_E_ROAM event because
	 * you were in the event handler when they got called and thus got queued and
	 * executed in the next iteration of the the do/while loop in wlc_process_event().
	 */

	/* if this was the roam scan for an association recreation, then
	 * declare link down immediately instead of letting bcn_timeout
	 * happen later.
	 */
	if (ASSOC_RECREATE_ENAB(wlc->pub) && assoc_recreate) {
	    if (status != WLC_E_STATUS_SUCCESS) {
			WL_ASSOC(("wl%d: ROAM: RECREATE failed, link down\n", WLCWLUNIT(wlc)));
			wlc_link(wlc, FALSE, &cfg->prev_BSSID, cfg, WLC_E_LINK_ASSOC_REC);
			roam->bcns_lost = TRUE;
			roam->time_since_bcn = 1;
#ifdef EXT_STA
			/* Under Win7 the driver could indicates disassoc when driver down
			 * before sleep.  do not indicate disassociation again if it happens
			 */
			if (WLEXTSTA_ENAB(wlc->pub)) {
				if (!cfg->ar_disassoc)
					/* indicate DISASSOC due to unreachability */
					wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS,
						&cfg->prev_BSSID, DOT11_RC_INACTIVITY,
						DOT11_BSSTYPE_INFRASTRUCTURE);
				if (cfg->ar_disassoc)
					cfg->ar_disassoc = FALSE;
			}
#endif /* EXT_STA */
		}

#ifdef  NLO
		wlc_bss_mac_event(wlc, cfg, WLC_E_ASSOC_RECREATED, NULL, status, 0, 0, 0, 0);
#endif /* NLO */
	}
} /* wlc_roam_complete */

void
wlc_disassoc_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint disassoc_reason, uint bss_type)
{
	wlc_info_t *wlc = cfg->wlc;

#ifdef SLAVE_RADAR
	if (WL11H_STA_ENAB(wlc) && wlc_dfs_get_radar(wlc->dfs)) {
		cfg->pm->PMmodeChangeDisabled = FALSE;
		wlc_set_pm_mode(wlc, cfg->pm->PM_oldvalue, cfg);
		wlc->mpc = TRUE;
		wlc_radio_mpc_upd(wlc);
	}
#endif /* SLAVE_RADAR */
#ifdef WLRM
	if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: abort RM due to disassoc\n", WLCWLUNIT(wlc)));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */

#ifdef WL11K
	if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: abort RRM due to disassoc\n", WLCWLUNIT(wlc)));
		wlc_rrm_stop(wlc);
	}
#endif /* WL11K */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		struct scb *scbd;

		if ((scbd = wlc_scbfind(wlc, cfg, addr)) == NULL)
			WL_ERROR(("%s: null SCB\n", __FUNCTION__));
		else {
			if (scbd->mac_address_handle) {
			wlfc_MAC_table_update(wlc->wl, &addr->octet[0],
			                      WLFC_CTL_TYPE_MACDESC_DEL,
			                      scbd->mac_address_handle,
			                      ((cfg->wlcif == NULL) ? 0 : cfg->wlcif->index));
			wlfc_release_MAC_descriptor_handle(wlc->wlfc_data,
			                                   scbd->mac_address_handle);
			WLFC_DBGMESG(("STA: MAC-DEL for [%02x:%02x:%02x:%02x:%02x:%02x], "
			              "handle: [%d], if:%d\n",
			              addr->octet[0], addr->octet[1], addr->octet[2],
			              addr->octet[3], addr->octet[4], addr->octet[5],
			              scbd->mac_address_handle,
			              ((cfg->wlcif == NULL) ? 0 : cfg->wlcif->index)));
			scbd->mac_address_handle = 0;
			}
		}
	}
#endif /* PROP_TXSTATUS */

	wlc_bss_mac_event(wlc, cfg, WLC_E_DISASSOC, addr, status,
	                  disassoc_reason, bss_type, 0, 0);
} /* wlc_disassoc_complete */

void
wlc_roamscan_start(wlc_bsscfg_t *cfg, uint roam_reason)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_roam_t *roam = cfg->roam;
	bool roamscan_full, roamscan_new;
	int err;

#ifdef WLABT
	if (WLABT_ENAB(wlc->pub) && roam_reason == WLC_E_REASON_LOW_RSSI) {
		wlc_check_adaptive_bcn_timeout(cfg);
	}
#endif /* WLABT */
	if (roam_reason == WLC_E_REASON_DEAUTH || roam_reason == WLC_E_REASON_DISASSOC ||
		roam_reason == WLC_E_REASON_RADAR_DETECTED) {
#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub) && wlc->ccx->ccx_network)
			wlc_ccx_on_roam_start(wlc->ccx, roam_reason == WLC_E_REASON_DEAUTH ?
			                      WLC_E_DEAUTH_IND : WLC_E_DISASSOC_IND);
#endif	/* BCMCCX */

		wlc_update_bcn_info(cfg, FALSE);

		/* Don't block this scan */
		roam->scan_block = 0;

		/* bzero(cfg->BSSID.octet, ETHER_ADDR_LEN); */
		wlc_bss_clear_bssid(cfg);
	}

	/* Turning off partials scans will restore original 'dumb' roaming algorithms */
	if (roam->partialscan_period) {
		uint idx, num_invalid_aps = 0;
		/* make sure that we don't have a valid cache full of APs we have just tried to
		 * associate with - this prevents thrashing
		 * If there's only one AP in the cache, we should consider it a valid target even
		 * if we are "blocked" from it
		 */
		if (roam->cache_numentries > 1 &&
		    (roam_reason == WLC_E_REASON_MINTXRATE || roam_reason == WLC_E_REASON_TXFAIL)) {
			for (idx = 0; idx < roam->cache_numentries; idx++)
				if (roam->cached_ap[idx].time_left_to_next_assoc > 0)
					num_invalid_aps++;

			if (roam->cache_numentries > 0 &&
			    num_invalid_aps == roam->cache_numentries) {
				WL_ASSOC(("wl%d: %s: Unable start roamscan because there are no "
				          "valid entries in the roam cache\n", wlc->pub->unit,
				          __FUNCTION__));
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
				wlc_print_roam_status(cfg, roam_reason, TRUE);
#endif // endif
				return;
			}
		}

		/* Check the cache for the RSSI and make sure that we didn't have a significant
		 * change. If we did, invalidate the cache and run a full scan, because we're
		 * walking somewhere.
		 */
		if (roam_reason == WLC_E_REASON_LOW_RSSI) {
			int rssi = cfg->link->rssi;

			/* Update roam profile at run-time */
			if (wlc->band->roam_prof)
				wlc_roam_prof_update(wlc, cfg, FALSE);

			if ((roam->prev_rssi - rssi > roam->ci_delta) ||
			    (roam->cache_valid && (rssi - roam->prev_rssi > roam->ci_delta))) {
				/* Ignore RSSI change if roaming is already running */
				if (as->type == AS_ROAM && SCAN_IN_PROGRESS(wlc->scan))
					return;

				WL_ASSOC(("wl%d: %s: Detecting a significant RSSI change, "
				          "forcing full scans, p %d, c %d\n",
				          wlc->pub->unit, __FUNCTION__, roam->prev_rssi, rssi));
				WL_SRSCAN(("wl%d: Detecting a significant RSSI change, "
				          "forcing full scans, p %d, c %d\n",
				          wlc->pub->unit, roam->prev_rssi, rssi));
				roam->cache_valid = FALSE;
				roam->scan_block = 0;
				roam->prev_rssi = rssi;
			}
		}

		/* We are above the threshold and should stop roaming now */
		if (roam_reason == WLC_E_REASON_LOW_RSSI &&
		    cfg->link->rssi >=  wlc->band->roam_trigger) {
			/* Stop roam scans */
			if (roam->active && roam->reason == WLC_E_REASON_LOW_RSSI) {
				/* Ignore RSSI thrashing about the roam_trigger */
				if ((cfg->link->rssi - wlc->band->roam_trigger) <
				    wlc->roam_rssi_cancel_hysteresis)
					return;

				WL_ASSOC(("wl%d: %s: Finished with roaming\n",
				          wlc->pub->unit, __FUNCTION__));

				if (as->type == AS_ROAM && SCAN_IN_PROGRESS(wlc->scan)) {
					uint scan_block_save = roam->scan_block;

					WL_ASSOC(("wl%d: %s: Aborting the roam scan in progress "
					          "since we received a strong signal RSSI: %d\n",
					          wlc->pub->unit, __FUNCTION__, cfg->link->rssi));
					WL_ROAM(("ROAM: Aborting the roam scan by RSSI %d\n",
						cfg->link->rssi));

					wlc_assoc_abort(cfg);
					roam->scan_block = scan_block_save;

					/* clear reason */
					roam->reason = WLC_E_REASON_INITIAL_ASSOC;
				}

				/* Just stop scan
				 * Keep roam cache and scan_block to avoid thrashing
				 */
				roam->active = FALSE;
			}
			return;
		}

		if (roam->reason == WLC_E_REASON_MINTXRATE ||
		    roam->reason == WLC_E_REASON_TXFAIL) {
			if (roam->txpass_cnt > roam->txpass_thresh) {
				WL_ASSOC(("wl%d: %s: Canceling the roam action because we have "
				          "sent %d packets at a good rate\n",
				          wlc->pub->unit, __FUNCTION__, roam->txpass_cnt));

				if (as->type == AS_ROAM && SCAN_IN_PROGRESS(wlc->scan)) {
					WL_ASSOC(("wl%d: %s: Aborting the roam scan in progress\n",
					          wlc->pub->unit, __FUNCTION__));

					wlc_assoc_abort(cfg);
				}
				roam->cache_valid = FALSE;
				roam->active = FALSE;
				roam->scan_block = 0;
				roam->txpass_cnt = 0;
				roam->reason = WLC_E_REASON_INITIAL_ASSOC; /* clear reason */

				return;
			}
		}

		/* Already roaming, come back in another watchdog tick  */
		if (roam->scan_block) {
			return;
		}

		/* Should initiate the roam scan now */
		roamscan_full = !roam->cache_valid;
		roamscan_new = !roam->active && !roam->cache_valid;

#ifdef BCMCCX
		/* Issue a CCX fast roam scan request */
		if (CCX_ENAB(wlc->pub) && wlc->ccx->ap_channel_num) {
			wlc->ccx->fast_roam = TRUE;
			roam->cache_valid = FALSE;
		}
		else
#endif /* BCMCCX */
		if (roam->time_since_upd >= roam->fullscan_period) {
#ifdef WLRCC
			if (WLRCC_ENAB(wlc->pub)) {
				if (roam->rcc_mode != RCC_MODE_FORCE) {
					roamscan_full = TRUE;
					roam->roam_fullscan = TRUE;
				}
			} else
#endif /* WLRCC */
				roamscan_full = TRUE;

			/* wlc_roam_scan() uses this info to decide on a full scan or a partial
			 * scan.
			 */
			if (roamscan_full)
				roam->cache_valid = FALSE;
		}

		/*
		 * If a roam scan is currently in progress, do not start a new one just yet.
		 *
		 * The current roam scan/re-association may however be waiting for a beacon,
		 * which we do not get because the AP changed channel or the signal faded away.
		 * In that case, start the new roam scan nevertheless.
		 */
		if (as->type == AS_ROAM && as->state != AS_WAIT_RCV_BCN) {
			WL_ASSOC(("wl%d: %s: Not starting roam scan for reason %u because roaming "
				"already in progress for reason %u, as->state %d\n",
				wlc->pub->unit, __FUNCTION__, roam_reason, roam->reason,
				as->state));
			return;
		}

		WL_ASSOC(("wl%d: %s: Start roam scan: Doing a %s scan with a scan period of %d "
		          "seconds for reason %u\n", wlc->pub->unit, __FUNCTION__,
		          roamscan_new ? "new" :
		          !(roam->cache_valid && roam->cache_numentries > 0) ? "full" : "partial",
		          roamscan_new || !(roam->cache_valid && roam->cache_numentries > 0) ?
		          roam->fullscan_period : roam->partialscan_period,
		          roam_reason));

		/* Kick off the roam scan */
		if (!(err = wlc_roam_scan(cfg, roam_reason, NULL, 0))) {
			roam->active = TRUE;
			roam->scan_block = roam->partialscan_period;

			if (roamscan_full || roamscan_new) {
				if (
#ifdef WLRCC
					(WLRCC_ENAB(wlc->pub) &&
					 (roam->rcc_mode != RCC_MODE_FORCE)) &&
#endif // endif
					roamscan_new && roam->multi_ap) {
					/* Do RSSI_ROAMSCAN_FULL_NTIMES number of scans before */
					/* kicking in partial scans */
					roam->fullscan_count = roam->nfullscans;
				}
				else
					roam->fullscan_count = 1;
				roam->time_since_upd = 0;
			}
		}
		else {
			if (err == BCME_EPERM)
				WL_ASSOC(("wl%d: %s: Couldn't start the roam with error %d\n",
				          wlc->pub->unit, __FUNCTION__, err));
			else
				WL_ERROR(("wl%d: %s: Couldn't start the roam with error %d\n",
				          wlc->pub->unit,  __FUNCTION__, err));
		}
		return;
	}
	/* Original roaming */
	else {
		/* roam if metric is worse than "good enough" threshold, which is radio dependent
		 * XXX - Should we really do this every frame?  How do we keep from constantly
		 *       scanning if we're almost out of range of the only AP ?
		 */
		int roam_metric;
		if (roam_reason == WLC_E_REASON_LOW_RSSI)
			roam_metric = cfg->link->rssi;
		else
			roam_metric = WLC_RSSI_MINVAL;

		if (roam_metric < wlc->band->roam_trigger) {
			if (roam->scan_block || roam->off) {
				WL_ASSOC(("ROAM: roam_metric=%d; block roam scan request(%u,%d)\n",
				          roam_metric, roam->scan_block, roam->off));
			} else {
				WL_ASSOC(("ROAM: RSSI = %d; request roam scan\n",
				          roam_metric));

				roam->scan_block = roam->fullscan_period;
#ifdef BCMCCX
				/* issue a CCX fast roam scan request */
				if (CCX_ENAB(wlc->pub) && wlc->ccx->ap_channel_num)
					wlc->ccx->fast_roam = TRUE;
#endif /* BCMCCX */
				wlc_roam_scan(cfg, WLC_E_REASON_LOW_RSSI, NULL, 0);
			}
		}
	}
} /* wlc_roamscan_start */

static void
wlc_roamscan_complete(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;

	WL_TRACE(("wl%d: %s: enter\n", wlc->pub->unit, __FUNCTION__));

	/* Not going to cache channels for any other roam reason but these */
	if (!((roam->reason == WLC_E_REASON_LOW_RSSI) ||
	      (roam->reason == WLC_E_REASON_BCNS_LOST) ||
	      (roam->reason == WLC_E_REASON_MINTXRATE) ||
	      (roam->reason == WLC_E_REASON_TXFAIL) ||
	      (roam->reason == WLC_E_REASON_RADAR_DETECTED)))
		return;

	if (roam->roam_type == ROAM_PARTIAL) {
		/* Update roam period with back off */
		wlc_roam_period_update(wlc, cfg);
		return;
	}

	if (roam->fullscan_count == 1) {
		WL_ASSOC(("wl%d.%d: %s: Building roam candidate cache from driver roam scans\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		wlc_build_roam_cache(cfg, wlc->as_info->join_targets);
	}

	if ((roam->fullscan_count > 0) &&
		((roam->roam_type == ROAM_FULL) ||
		(roam->split_roam_phase == 0))) {
			roam->fullscan_count--;
	}
}

void
wlc_build_roam_cache(wlc_bsscfg_t *cfg, wlc_bss_list_t *candidates)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	int32 i;
	uint32 nentries, j;
	chanspec_t chanspec;
	struct ether_addr *bssid;

	(void)wlc;

	/* For legacy roam scan or phase 1 split roam scan, rebuild the cache.
	 * For phase 2 split roam scan, accumulate results.
	 */
	if ((roam->roam_type == ROAM_FULL) || (roam->split_roam_phase == 1)) {
		roam->cache_numentries = 0;

		/* If associated, add the current AP to the cache. */
		if (cfg->associated && !ETHER_ISNULLADDR(&cfg->BSSID)) {
			bssid = &cfg->current_bss->BSSID;
			chanspec = CH20MHZ_CHSPEC(
				wf_chspec_ctlchan(cfg->current_bss->chanspec));
			WL_SRSCAN(("cache add: idx %d: bssid %02x:%02x",
				roam->cache_numentries, bssid->octet[4], bssid->octet[5]));
			WL_ASSOC(("cache add: idx %d: bssid %02x:%02x\n",
				roam->cache_numentries, bssid->octet[4], bssid->octet[5]));
			bcopy(bssid, &roam->cached_ap[0].BSSID, ETHER_ADDR_LEN);
			roam->cached_ap[0].chanspec = chanspec;
			roam->cached_ap[0].time_left_to_next_assoc = 0;
			roam->cache_numentries = 1;
		}
	}

	nentries = roam->cache_numentries;

	/* fill in the cache entries, avoid duplicates */
	for (i = candidates->count - 1; (i >= 0) &&
		(nentries < ROAM_CACHELIST_SIZE); i--) {

		bssid = &candidates->ptrs[i]->BSSID;
		chanspec = CH20MHZ_CHSPEC(
			wf_chspec_ctlchan(candidates->ptrs[i]->chanspec));

		for (j = 0; j < nentries; j++) {
			if (chanspec == roam->cached_ap[j].chanspec &&
			    !bcmp(bssid, &roam->cached_ap[j].BSSID, ETHER_ADDR_LEN))
				break;
		}
		if (j == nentries) {
			WL_SRSCAN(("cache add: idx %d: bssid %02x:%02x",
				nentries, bssid->octet[4], bssid->octet[5]));
			WL_ASSOC(("cache add: idx %d: bssid %02x:%02x\n",
				nentries, bssid->octet[4], bssid->octet[5]));
			bcopy(bssid, &roam->cached_ap[nentries].BSSID, ETHER_ADDR_LEN);
			roam->cached_ap[nentries].chanspec = chanspec;
			roam->cached_ap[nentries].time_left_to_next_assoc = 0;
			nentries++;
		}
	}

	roam->cache_numentries = nentries;

	/* Mark the cache valid. */
	if ((roam->cache_numentries) &&
		((roam->roam_type == ROAM_FULL) ||
		(roam->split_roam_phase == 0))) {
		bool cache_valid = TRUE;

		WL_SRSCAN(("wl%d: Full roam scans completed, starting partial scans with %d "
		          "entries and rssi %d", wlc->pub->unit, nentries, cfg->link->rssi));
		WL_ASSOC(("wl%d: Full roam scans completed, starting partial scans with %d "
		          "entries and rssi %d\n", wlc->pub->unit, nentries, cfg->link->rssi));
#ifdef WLRCC
		if (WLRCC_ENAB(wlc->pub)) {
			cache_valid = roam->rcc_mode != RCC_MODE_FORCE;
		}
#endif /* WLRCC */
		if (cache_valid) {
			roam->prev_rssi = cfg->link->rssi;
			roam->cache_valid = TRUE;
		}
		wlc_roam_set_env(cfg, nentries);
	}

	/* print status */
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	wlc_print_roam_status(cfg, roam->reason, TRUE);
#endif // endif
} /* wlc_build_roam_cache */

/** Set the AP environment */
static void
wlc_roam_set_env(wlc_bsscfg_t *cfg, uint nentries)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	wlcband_t *this_band = wlc->band;
	wlcband_t *other_band = wlc->bandstate[OTHERBANDUNIT(wlc)];

	if (roam->ap_environment != AP_ENV_DETECT_NOT_USED) {
		if (nentries > 1 && roam->ap_environment != AP_ENV_DENSE) {
			roam->ap_environment = AP_ENV_DENSE;
			WL_ASSOC(("wl%d: %s: Auto-detecting dense AP environment with "
			          "%d targets\n", wlc->pub->unit, __FUNCTION__, nentries));
				/* if the roam trigger isn't set to the default, or to the value
				 * of the sparse environment setting, don't change it -- we don't
				 * want to overrride manually set triggers
				 */
			/* this means we are transitioning from a sparse environment */
			if (this_band->roam_trigger == WLC_AUTO_ROAM_TRIGGER -
			    WLC_ROAM_TRIGGER_STEP) {
				this_band->roam_trigger = WLC_AUTO_ROAM_TRIGGER;
				WL_ASSOC(("wl%d: %s: Setting roam trigger on bandunit %u "
				          "to %d\n", wlc->pub->unit, __FUNCTION__,
				          this_band->bandunit, WLC_AUTO_ROAM_TRIGGER));
#ifdef WLABT
				if (WLABT_ENAB(wlc->pub) && (NBANDS(wlc) > 1) &&
					(roam->prev_bcn_timeout != 0)) {
					roam->bcn_timeout = roam->prev_bcn_timeout;
					roam->prev_bcn_timeout = 0;
					WL_ASSOC(("wl%d: Restore bcn_timeout to %d\n",
						wlc->pub->unit, roam->bcn_timeout));
				}
#endif /* WLABT */
			}
			else if (this_band->roam_trigger != WLC_AUTO_ROAM_TRIGGER)
				WL_ASSOC(("wl%d: %s: Not modifying manually-set roam "
				          "trigger on bandunit %u from %d to %d\n",
				          wlc->pub->unit, __FUNCTION__, this_band->bandunit,
				          this_band->roam_trigger, WLC_AUTO_ROAM_TRIGGER));

			/* do the same for the other band */
			if ((NBANDS(wlc) > 1 && other_band->roam_trigger == WLC_AUTO_ROAM_TRIGGER -
			     WLC_ROAM_TRIGGER_STEP)) {
				other_band->roam_trigger = WLC_AUTO_ROAM_TRIGGER;
				WL_ASSOC(("wl%d: %s: Setting roam trigger on bandunit %u to %d\n",
				          wlc->pub->unit, __FUNCTION__, other_band->bandunit,
				          WLC_AUTO_ROAM_TRIGGER));
			} else if (NBANDS(wlc) > 1 && other_band->roam_trigger !=
			           WLC_AUTO_ROAM_TRIGGER)
				WL_ASSOC(("wl%d: %s: Not modifying manually-set band roam "
				          "trigger on bandunit %u from %d to %d\n",
				          wlc->pub->unit, __FUNCTION__, other_band->bandunit,
				          other_band->roam_trigger, WLC_AUTO_ROAM_TRIGGER));

			/* this means we are transitioning into a sparse environment
			 * from either an INDETERMINATE or dense one
			 */
		} else if (nentries == 1 && roam->ap_environment != AP_ENV_SPARSE) {
			WL_ASSOC(("wl%d: %s: Auto-detecting sparse AP environment with "
			          "one roam target\n", wlc->pub->unit, __FUNCTION__));
			roam->ap_environment = AP_ENV_SPARSE;

			if (this_band->roam_trigger == WLC_AUTO_ROAM_TRIGGER) {
				this_band->roam_trigger -= WLC_ROAM_TRIGGER_STEP;
				WL_ASSOC(("wl%d: %s: Setting roam trigger on bandunit %u "
				          "to %d\n", wlc->pub->unit, __FUNCTION__,
				          this_band->bandunit, this_band->roam_trigger));
#ifdef WLABT
				if (WLABT_ENAB(wlc->pub) && (NBANDS(wlc) > 1) && !P2P_ACTIVE(wlc)) {
					roam->prev_bcn_timeout = roam->bcn_timeout;
					roam->bcn_timeout = ABT_MIN_TIMEOUT;
					WL_ASSOC(("wl%d: Setting bcn_timeout to %d\n",
						wlc->pub->unit, roam->bcn_timeout));
				}
#endif /* WLABT */
			} else
				WL_ASSOC(("wl%d: %s: Not modifying manually-set roam "
				          "trigger on bandunit %u from %d to %d\n",
				          wlc->pub->unit, __FUNCTION__, this_band->bandunit,
				          this_band->roam_trigger,
				          this_band->roam_trigger - WLC_ROAM_TRIGGER_STEP));

			if (NBANDS(wlc) > 1 && other_band->roam_trigger == WLC_AUTO_ROAM_TRIGGER) {
				other_band->roam_trigger -= WLC_ROAM_TRIGGER_STEP;
				WL_ASSOC(("wl%d: %s: Setting roam trigger on bandunit %u "
				          "to %d\n", wlc->pub->unit, __FUNCTION__,
				          other_band->bandunit, other_band->roam_trigger));
			} else if (NBANDS(wlc) > 1)
				WL_ASSOC(("wl%d: %s: Not modifying manually-set band roam "
				          "trigger on bandunit %u from %d to %d\n",
				          wlc->pub->unit, __FUNCTION__, other_band->bandunit,
				          other_band->roam_trigger,
				          other_band->roam_trigger - WLC_ROAM_TRIGGER_STEP));
		}
	}
} /* wlc_roam_set_env */

void
wlc_roam_motion_detect(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;

#ifdef BCMCCX
	if (wlc_ccx_is_droam(wlc->ccx))
		return;
#endif // endif
	/* Check for motion */
	if (!roam->motion &&
	    ABS(cfg->current_bss->RSSI - roam->RSSIref) >= roam->motion_rssi_delta) {
		roam->motion = TRUE;

		/* force a full scan on the next iteration */
		roam->cache_valid = FALSE;
		roam->scan_block = 0;

		wlc->band->roam_trigger += MOTION_DETECT_TRIG_MOD;
		wlc->band->roam_delta -= MOTION_DETECT_DELTA_MOD;

		if (NBANDS(wlc) > 1) {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_trigger += MOTION_DETECT_TRIG_MOD;
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_delta -=
			        MOTION_DETECT_DELTA_MOD;
		}

		WL_ASSOC(("wl%d.%d: Motion detected, invalidating roaming cache and "
		          "moving roam_delta to %d and roam_trigger to %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), wlc->band->roam_delta,
		          wlc->band->roam_trigger));
	}

	if (roam->motion && ++roam->motion_dur > roam->motion_timeout) {
		roam->motion = FALSE;
		/* update RSSIref in next watchdog */
		roam->RSSIref = 0;
		roam->motion_dur = 0;

		if ((wlc->band->roam_trigger <= wlc->band->roam_trigger_def) ||
			(wlc->band->roam_delta >= wlc->band->roam_delta_def))
			return;

		wlc->band->roam_trigger -= MOTION_DETECT_TRIG_MOD;
		wlc->band->roam_delta += MOTION_DETECT_DELTA_MOD;

		if (NBANDS(wlc) > 1) {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_trigger -= MOTION_DETECT_TRIG_MOD;
			wlc->bandstate[OTHERBANDUNIT(wlc)]->roam_delta +=
			        MOTION_DETECT_DELTA_MOD;
		}

		WL_ASSOC(("wl%d.%d: Motion timeout, restoring default values of "
		          "roam_delta to %d and roam_trigger to %d, new RSSI ref is %d\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), wlc->band->roam_delta,
		          wlc->band->roam_trigger, roam->RSSIref));
	}
} /* wlc_roam_motion_detect */

static void
wlc_roam_release_flow_cntrl(wlc_bsscfg_t *cfg)
{
	wlc_txflowcontrol_override(cfg->wlc, cfg->wlcif->qi, OFF, TXQ_STOP_FOR_PKT_DRAIN);
}

void
wlc_assoc_roam(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_join_pref_t *join_pref = cfg->join_pref;

	if (!cfg->roam->assocroam)
		return;

	/* if assocroam is enabled, need to consider roam to band pref band */
	WL_ASSOC(("wlc_assoc_roam, assocroam enabled, band pref = %d\n", join_pref->band));

	if (BSSCFG_STA(cfg) &&
	    (as->type != AS_ROAM && as->type != AS_RECREATE) &&
	    cfg->associated && (join_pref->band != WLC_BAND_AUTO)) {
		uint ssid_cnt = 0, i;
		wlc_bss_info_t **bip, *tmp;
		wlc_bss_info_t *current_bss = cfg->current_bss;

		if (join_pref->band ==
		    CHSPEC2WLC_BAND(current_bss->chanspec))
			return;

		bip = wlc->scan_results->ptrs;

		/* sort current_bss->ssid to the front */
		for (i = 0; i < wlc->scan_results->count; i++) {
			if (WLC_IS_CURRENT_SSID(cfg, (char*)bip[i]->SSID, bip[i]->SSID_len) &&
			    join_pref->band == CHSPEC2WLC_BAND(bip[i]->chanspec)) {
				if (i > ssid_cnt) {	/* swap if not self */
					tmp = bip[ssid_cnt];
					bip[ssid_cnt] = bip[i];
					bip[i] = tmp;
				}
				ssid_cnt++;
			}
		}

		if (ssid_cnt > 0) {
			WL_ASSOC(("assoc_roam: found %d matching ssid with current bss\n",
				ssid_cnt));
			/* prune itself */
			for (i = 0; i < ssid_cnt; i++)
				if (WLC_IS_CURRENT_BSSID(cfg, &bip[i]->BSSID) &&
				    current_bss->chanspec == bip[i]->chanspec) {
					ssid_cnt--;

					tmp = bip[ssid_cnt];
					bip[ssid_cnt] = bip[i];
					bip[i] = tmp;
				}
		}

		if (ssid_cnt > 0) {
			/* hijack this scan to start roaming completion after free memory */
			uint indx;
			wlc_bss_info_t *bi;

			WL_ASSOC(("assoc_roam: consider asoc roam with %d targets\n", ssid_cnt));

			/* free other scan_results since scan_results->count will be reduced */
			for (indx = ssid_cnt; indx < wlc->scan_results->count; indx++) {
				bi = wlc->scan_results->ptrs[indx];
				if (bi) {
					if (bi->bcn_prb)
						MFREE(wlc->osh, bi->bcn_prb, bi->bcn_prb_len);

					MFREE(wlc->osh, bi, sizeof(wlc_bss_info_t));
					wlc->scan_results->ptrs[indx] = NULL;
				}
			}

			wlc->scan_results->count = ssid_cnt;

			wlc_assoc_init(cfg, AS_ROAM);
			wlc_assoc_change_state(cfg, AS_SCAN);
			wlc_assoc_scan_complete(wlc, WLC_E_STATUS_SUCCESS, cfg);
		}
	}
} /* wlc_assoc_roam */

void
wlc_roam_bcns_lost(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		if (wlc->wakeup_scan) {
			WL_ASSOC(("%s(): wakeup_scan is TRUE, return\n", __FUNCTION__));
			return;
		}
	}
#endif // endif

	/* XXX If we've received no beacons at all since association, give up.
	 * For a roam this will simply continue roaming (it will attempt to go
	 * back to prev, but it's too late...) but it allows the LINK DOWN and
	 * another roam attempt.  For a first association, aborting here will
	 * simply start roaming, so instead explicitly fail to make sure the
	 * E_SET_SSID is sent and disable to stop further attempts.
	 */
	if (cfg->assoc->state == AS_WAIT_RCV_BCN) {
		if (cfg->assoc->type == AS_ASSOCIATION) {
			/* Fail, then force a disassoc */
			WL_ASSOC(("wl%d.%d: ASSOC: bcns_lost in WAIT_RCV_BCN\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
			WL_ASSOC(("Using assoc_done()/bsscfg_disable() to fail/stop.\n"));
			wlc_assoc_done(cfg, WLC_E_STATUS_FAIL);
			wlc_bsscfg_disable(wlc, cfg);
			return;
		}
		if (cfg->assoc->type == AS_ROAM) {
			WL_ASSOC(("wl%d.%d: ROAM: bcns_lost in WAIT_RCV_BCN, abort assoc\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg)));
			wlc_assoc_abort(cfg);
			return;
		}
		WL_ASSOC(("wl%d.%d: ASSOC: bcns_lost in WAIT_RCV_BCN, type %d\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), cfg->assoc->type));
	}

	if (!roam->off &&
	    ((cfg->assoc->state == AS_IDLE) || AS_CAN_YIELD(cfg->BSS, cfg->assoc->state)) &&
	    !ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d.%d: ROAM: time_since_bcn %d, request roam scan\n",
		          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), roam->time_since_bcn));

		/* No longer need to track PM states since we may have lost our AP */
		wlc_reset_pmstate(cfg);

		/* Allow consecutive roam scans without delay for the first
		 * ROAM_CONSEC_BCNS_LOST_THRESH beacon lost events after
		 * initially losing beacons
		 */
		if (roam->consec_roam_bcns_lost <= ROAM_CONSEC_BCNS_LOST_THRESH) {
			/* clear scan_block so that the roam scan will happen w/o delay */
			roam->scan_block = 0;
			roam->consec_roam_bcns_lost++;
			WL_ASSOC(("wl%d.%d: ROAM %s #%d w/o delay, setting scan_block to 0\n",
			          WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			          roam->consec_roam_bcns_lost));
		}

#ifdef WLBTAMP
		/* notify PAL of possible channel change */
		if (BTA_ENAB(wlc->pub))
			wlc_bta_assoc_status_upd(wlc->bta, cfg, WLC_BTA_RADIO_ENABLE);
#endif // endif

		wlc_roamscan_start(cfg, WLC_E_REASON_BCNS_LOST);
	}
} /* wlc_roam_bcns_lost */

int
wlc_roam_trigger_logical_dbm(wlc_info_t *wlc, wlcband_t *band, int val)
{
	int trigger_dbm = WLC_NEVER_ROAM_TRIGGER;

	if (val == WLC_ROAM_TRIGGER_DEFAULT)
		trigger_dbm = band->roam_trigger_init_def;
	else if (val == WLC_ROAM_TRIGGER_BANDWIDTH)
		trigger_dbm = band->roam_trigger_init_def + WLC_ROAM_TRIGGER_STEP;
	else if (val == WLC_ROAM_TRIGGER_DISTANCE)
		trigger_dbm = band->roam_trigger_init_def - WLC_ROAM_TRIGGER_STEP;
	else if (val == WLC_ROAM_TRIGGER_AUTO)
		trigger_dbm = WLC_AUTO_ROAM_TRIGGER;
	else
		ASSERT(0);

	return trigger_dbm;
}

/** Make decisions about roaming based upon feedback from the tx rate */
void
wlc_txrate_roam(wlc_info_t *wlc, struct scb *scb, tx_status_t *txs, bool pkt_sent,
	bool pkt_max_retries, uint8 ac)
{
	wlc_bsscfg_t *cfg;
	wlc_roam_t *roam;

	/* this code doesn't work if we have an override rate */
	if (wlc->band->rspec_override || wlc->band->mrspec_override)
		return;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	if (cfg == NULL)
		return;

	roam = cfg->roam;

	/* prevent roaming for tx rate issues too frequently */
	if (roam->ratebased_roam_block > 0)
		return;

	if (pkt_sent && !wlc_scb_ratesel_minrate(wlc->wrsi, scb, txs, ac))
		roam->txpass_cnt++;
	else
		roam->txpass_cnt = 0;

	/* should we roam on too many packets at the min tx rate? */
	if (roam->minrate_txpass_thresh) {
		if (pkt_sent) {
			if (wlc_scb_ratesel_minrate(wlc->wrsi, scb, txs, ac))
				roam->minrate_txpass_cnt++;
			else
				roam->minrate_txpass_cnt = 0;

			if (roam->minrate_txpass_cnt > roam->minrate_txpass_thresh &&
			    !roam->active) {
				WL_ASSOC(("wl%d: %s: Starting roam scan due to %d "
				          "packets at the most basic rate\n",
				          WLCWLUNIT(wlc), __FUNCTION__,
				          roam->minrate_txpass_cnt));
#ifdef WLP2P
				if (!BSS_P2P_ENAB(wlc, cfg))
#endif // endif
					wlc_roamscan_start(cfg, WLC_E_REASON_MINTXRATE);
				roam->minrate_txpass_cnt = 0;
				roam->ratebased_roam_block = ROAM_REASSOC_TIMEOUT;
			}
		}
	}

	/* should we roam on too many tx failures at the min rate? */
	if (roam->minrate_txfail_thresh) {
		if (pkt_sent)
			roam->minrate_txfail_cnt = 0;
		else if (pkt_max_retries && wlc_scb_ratesel_minrate(wlc->wrsi, scb, txs, ac)) {
			if (++roam->minrate_txfail_cnt > roam->minrate_txfail_thresh &&
			    !roam->active) {
				WL_ASSOC(("wl%d: starting roamscan for txfail\n",
				          WLCWLUNIT(wlc)));
#ifdef WLP2P
				if (!BSS_P2P_ENAB(wlc, cfg))
#endif // endif
					wlc_roamscan_start(cfg, WLC_E_REASON_TXFAIL);
				roam->minrate_txfail_cnt = 0; /* throttle roaming */
				roam->ratebased_roam_block = ROAM_REASSOC_TIMEOUT;
			}
		}
	}
} /* wlc_txrate_roam */

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
static void
wlc_print_roam_status(wlc_bsscfg_t *cfg, uint roam_reason, bool printcache)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_roam_t *roam = cfg->roam;
	uint idx;
	const char* event_name;
	char eabuf[ETHER_ADDR_STR_LEN];

	static struct {
		uint event;
		const char *event_name;
	} event_names[] = {
		{WLC_E_REASON_INITIAL_ASSOC, "INITIAL ASSOCIATION"},
		{WLC_E_REASON_LOW_RSSI, "LOW_RSSI"},
		{WLC_E_REASON_DEAUTH, "RECEIVED DEAUTHENTICATION"},
		{WLC_E_REASON_DISASSOC, "RECEIVED DISASSOCATION"},
		{WLC_E_REASON_BCNS_LOST, "BEACONS LOST"},
		{WLC_E_REASON_FAST_ROAM_FAILED, "FAST ROAM FAILED"},
		{WLC_E_REASON_DIRECTED_ROAM, "DIRECTED ROAM"},
		{WLC_E_REASON_TSPEC_REJECTED, "TSPEC REJECTED"},
		{WLC_E_REASON_BETTER_AP, "BETTER AP FOUND"},
		{WLC_E_REASON_MINTXRATE, "STUCK AT MIN TX RATE"},
		{WLC_E_REASON_BSSTRANS_REQ, "REQUESTED ROAM"},
		{WLC_E_REASON_TXFAIL, "TOO MANY TXFAILURES"}
	};

	event_name = "UNKNOWN";
	for (idx = 0; idx < ARRAYSIZE(event_names); idx++) {
		if (event_names[idx].event == roam_reason)
		    event_name = event_names[idx].event_name;
	}

	WL_ASSOC(("wl%d: Current roam reason is %s. The cache has %u entries.\n", wlc->pub->unit,
	          event_name, roam->cache_numentries));
	if (printcache) {
		for (idx = 0; idx < roam->cache_numentries; idx++) {
			WL_ASSOC(("\t Entry %u => chanspec 0x%x (BSSID: %s)\t", idx,
			          roam->cached_ap[idx].chanspec,
			          bcm_ether_ntoa(&roam->cached_ap[idx].BSSID, eabuf)));
			if (roam->cached_ap[idx].time_left_to_next_assoc)
				WL_ASSOC(("association blocked for %d more seconds\n",
				          roam->cached_ap[idx].time_left_to_next_assoc));
			else
				WL_ASSOC(("assocation not blocked\n"));
		}
	}
}
#endif /* defined(BCMDBG) || defined(WLMSG_ASSOC) */
#endif /* STA */

#ifdef STA
/** association request on different bsscfgs will be queued up in wlc->assco_req[] */
/** association request has the highest priority over scan (af, scan, excursion),
 * roam, rm.
 */
static int
wlc_mac_request_assoc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	int err = BCME_OK;
	wlc_assoc_t *as;

	ASSERT(cfg != NULL);

	/* abort assoc/roam except disassoc in progress on this bsscfg */
	as = cfg->assoc;
	ASSERT(as != NULL);

	if (req == WLC_ACTION_ASSOC) {
		if (as->state != AS_IDLE && as->state != AS_WAIT_DISASSOC) {
			WL_INFORM(("wl%d: assoc request while association is in progress, "
					   "aborting association\n", wlc->pub->unit));
			wlc_assoc_abort(cfg);
		}
	}

	/* abort any roam in progress */
	if (AS_IN_PROGRESS(wlc)) {
		wlc_bsscfg_t *bc = wlc->as_info->assoc_req[0];

		ASSERT(bc != NULL);

		as = bc->assoc;
		ASSERT(as != NULL);

		/* Do not abort when in state AS_XX_WAIT_RCV_BCN. In this state
		 * association is already completed, if we abort in this state then all
		 * those tasks which are performed as part of wlc_join_complete will be
		 * skipped but still maintain the association.
		 * This leaves power save and 11h/d in wrong state
		 */
		if (as->state != AS_IDLE &&
			(as->type == AS_ROAM && as->state != AS_WAIT_RCV_BCN)) {
			WL_INFORM(("wl%d: assoc request while roam is in progress, "
			           "aborting roam\n", wlc->pub->unit));
			wlc_assoc_abort(bc);
		}
	}

	/* abort any scan engine usage */
	if (ANY_SCAN_IN_PROGRESS(wlc->scan) &&
#ifdef WLAWDL
		!BSSCFG_AWDL(wlc, cfg) &&
#endif // endif
		TRUE) {
		wlc_info_t *cur_wlc;
		int idx;
		WL_INFORM(("wl%d: assoc request while scan is in progress, "
		           "aborting scan\n", wlc->pub->unit));
		FOREACH_WLC(wlc->cmn, idx, cur_wlc) {
			wlc_scan_abort(cur_wlc->scan, WLC_E_STATUS_NEWASSOC);
		}
	}

#ifdef WLRM
	if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: association request while radio "
		           "measurement is in progress, aborting measurement\n",
		           wlc->pub->unit));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */

#ifdef WL11K
	if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: association request while radio "
			   "measurement is in progress, aborting RRM\n",
			   wlc->pub->unit));
		wlc_rrm_stop(wlc);
	}
#endif /* WL11K */

	return err;
}

/**
 * roam request will be granted only when there is no outstanding association requests
 * and no other roam requests pending.
 *
 * roam has lower priority than association and scan (af, scan);
 * has higher priority than rm and excursion
 */
static int
wlc_mac_request_roam(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	int err = BCME_OK;

	ASSERT(cfg != NULL);

	/* deny if any assoc/roam is in progress */
	if (AS_IN_PROGRESS(wlc)) {
		WL_INFORM(("wl%d: roam scan request blocked for association/roam in progress\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
	}
	/* abort low priority excursion in progress */
	else if (LPRIO_EXCRX_IN_PROGRESS(wlc->scan)) {
		WL_INFORM(("wl%d: roam scan request while LP excursion is in progress, "
		           "aborting LP excursion\n", wlc->pub->unit));
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWASSOC);
	}
	/* deny if any scan engine is in use */
	else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
#ifdef BCMCCX
		if (CCX_ENAB(wlc->pub) && wlc->ccx->fast_roam) {
			/* CCX fast roam scan should abort current scan */
			WL_INFORM(("wl%d: CCX fast roam scan request while scan in progress,"
				" aborting scan\n", wlc->pub->unit));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_CCXFASTRM);
		}
		else
#endif /* BCMCCX */
		{
		WL_INFORM(("wl%d: roam scan blocked for scan in progress\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
		}
	}
	else if (cfg->pm->PSpoll) {
		WL_INFORM(("wl%d: roam scan blocked for outstanding PS-Poll\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
	}
	else if (BSS_QUIET_STATE(wlc->quiet, cfg) & SILENCE) {
		WL_INFORM(("wl%d: roam scan blocked for 802.11h Quiet Period\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
	}
#ifdef WLRM
	else if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: roam scan while radio measurement is in progress, "
		           "aborting measurement\n",
		           wlc->pub->unit));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */
#ifdef WL11K
	else if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: roam scan while radio measurement is in progress,"
			" aborting RRM\n",
			wlc->pub->unit));
		wlc_rrm_stop(wlc);
	}
#endif /* WL11K */

	return err;
} /* wlc_mac_request_roam */
#endif /* STA */

static bool
wlc_mac_excursion_allowed(wlc_info_t *wlc, wlc_bsscfg_t *req)
{
	int idx;
	wlc_bsscfg_t *cfg;

	/* don't allow the excursion if anyone isn't completely done with the assoc/roam */
	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg == req ||
		    !BSSCFG_STA(cfg) ||
		    cfg->assoc == NULL)
			continue;
		if (cfg->assoc->state != AS_IDLE)
			return FALSE;
	}

	/* don't allow the excursion if the scan engine is busy */
	if (ANY_SCAN_IN_PROGRESS(wlc->scan))
		return FALSE;

	return TRUE;
}

/**
 * scan engine request will be granted only when there is no association in progress
 * among different priorities within the scan request:
 *     af > scan > iscan|escan > pno scan > excursion;
 *     scan request has higher priority than roam and rm;
 */
static int
wlc_mac_request_scan(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	int err = BCME_OK;
	wlc_bsscfg_t *other_cfg;
	int idx = 0;
	bool p2p_go_flag = FALSE;

	if (req == WLC_ACTION_SCAN || req == WLC_ACTION_ISCAN || req == WLC_ACTION_ESCAN) {
		FOREACH_BSS(wlc, idx, other_cfg) {
			if (P2P_GO(wlc, other_cfg)) {
				p2p_go_flag = TRUE;
			}
		}
		/* When 11h is enabled, prevent AP from leaving a DFS
		 * channel while in the in-service-monitor mode.
		 * Exception - ETSI/EDCRS_EU - where scan is allowed as returning to clear
		 * DFS radar channel won't cost a CAC again.
		 */
		if (wlc_radar_chanspec(wlc->cmi, wlc->home_chanspec) && (
#ifndef WL_SCAN_DFS_HOME
				!wlc->is_edcrs_eu ||
#endif /* WL_SCAN_DFS_HOME */
				wlc_quiet_chanspec(wlc->cmi, wlc->home_chanspec)) &&
				((!WLC_APSTA_ON_RADAR_CHANNEL(wlc) && AP_ACTIVE(wlc) &&
				  WL11H_AP_ENAB(wlc) && !p2p_go_flag) ||
#ifdef SLAVE_RADAR
				 (WL11H_STA_ENAB(wlc) && STA_ACTIVE(wlc)) ||
#endif /* SLAVE_RADAR */
				 FALSE)) {
			WL_INFORM(("wl%d: %s: On radar channel (not ETSI), WLC_SCAN ignored\n",
					wlc->pub->unit, __FUNCTION__));
			err = BCME_SCANREJECT;
			return err;
		}
	}

	if (FALSE) {
		; /* empty */
	}
#ifdef STA
	/* is any assoc/roam in progress? */
	else if (AS_IN_PROGRESS(wlc)) {
		wlc_bsscfg_t *bc = wlc->as_info->assoc_req[0];
		wlc_assoc_t *as;

		ASSERT(bc != NULL);

		as = bc->assoc;
		ASSERT(as != NULL);

		/* is any assoc in progress? */
		if (as->state != AS_IDLE && as->type != AS_ROAM) {
			WL_ERROR(("wl%d: scan request blocked for association in progress\n",
				wlc->pub->unit));
			err = BCME_BUSY;
		}
#ifdef BCMCCX
		else if (CCX_ENAB(wlc->pub) && wlc->ccx->fast_roam) {
			/* don't abort CCX fast roam...block scan (hanging deferral) */
			WL_INFORM(("wl%d: scan request blocked for CCX fast roam in"
			           " progress\n", wlc->pub->unit));
			err = BCME_NOTREADY;
		}
#endif /* BCMCCX */
		else {
			WL_INFORM(("wl%d: scan request while roam is in progress, aborting"
			           " roam\n", wlc->pub->unit));
			wlc_assoc_abort(bc);
		}
	}
#ifdef WLRM
	else if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: scan request while radio measurement is in progress,"
		           " aborting measurement\n",
		           wlc->pub->unit));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */
#ifdef WL11K
	else if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: scan request while radio measurement is in progress,"
			" aborting RRM\n",
			wlc->pub->unit));
		wlc_rrm_stop(wlc);
	}
#endif /* WL11K */
#endif /* STA */
	else if (req == WLC_ACTION_ACTFRAME) {
		if (cfg != NULL && (BSS_QUIET_STATE(wlc->quiet, cfg) & SILENCE)) {
			WL_INFORM(("wl%d: AF request blocked for 802.11h Quiet Period\n",
			           wlc->pub->unit));
			err = BCME_EPERM;
		}
		else if (ACT_FRAME_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("wl%d: AF request blocked for AF in progress\n",
			           wlc->pub->unit));
			err = BCME_BUSY;
		}
		else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("aborting scan due to AF tx request\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
	}
	else if (req == WLC_ACTION_ISCAN || req == WLC_ACTION_ESCAN) {
		if (ISCAN_IN_PROGRESS(wlc) || ESCAN_IN_PROGRESS(wlc->scan)) {
			/* i|e scans preempt in-progress i|e scans */
			WL_INFORM(("escan/iscan aborting escan/iscan\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
#ifdef WLPFN
		else if (WLPFN_ENAB(wlc->pub) &&
		         wl_pfn_scan_in_progress(wlc->pfn)) {
			/* iscan/escan also preempts PNO scan */
			WL_INFORM(("escan/iscan request %d while PNO scan in progress, "
				   "aborting PNO scan\n", req));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
#endif /* WLPFN */
		else if (LPRIO_EXCRX_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("escan/iscan request %d while LP excursion is in progress, "
			           "aborting LP excursion\n", req));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
		else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
			/* other scans have precedence over e/iscans */
			WL_INFORM(("escan/iscan blocked due to another kind of scan\n"));
			err = BCME_NOTREADY;
		}
	}
#ifdef WLPFN
	else if (WLPFN_ENAB(wlc->pub) && req == WLC_ACTION_PNOSCAN) {
		if (wl_pfn_scan_in_progress(wlc->pfn)) {
			WL_INFORM(("New PNO scan request while PNO scan in progress, "
				   "aborting PNO scan in progress\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
		else if (LPRIO_EXCRX_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("PNO scan request while LP excursion is in progress, "
				   "aborting LP excursion\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
		else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
			/* other scans have precedence over PNO scan */
			WL_INFORM(("PNO scan blocked due to another kind of scan\n"));
			err = BCME_NOTREADY;
		}
	}
#endif /* WLPFN */
	else if (req == WLC_ACTION_LPRIO_EXCRX) {
		if (!wlc_mac_excursion_allowed(wlc, cfg)) {
			WL_INFORM(("wl%d: LP excursion request blocked\n",
			           wlc->pub->unit));
			err = BCME_BUSY;
		}
	}
	else if (req == WLC_ACTION_SCAN) {
		if (ACT_FRAME_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("wl%d: scan request blocked for AF in progress\n",
			           wlc->pub->unit));
			err = BCME_BUSY;
		}
		else if (ISCAN_IN_PROGRESS(wlc) || ESCAN_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("escan/iscan aborting due to another kind of scan request\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
#ifdef WLPFN
		else if (WLPFN_ENAB(wlc->pub) &&
		    wl_pfn_scan_in_progress(wlc->pfn)) {
			WL_INFORM(("Aborting PNO scan due to scan request\n"));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
#endif // endif
		else if (LPRIO_EXCRX_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("wl%d: scan request %d while LP excursion is in progress, "
				   "aborting LP excursion\n", wlc->pub->unit, req));
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWSCAN);
		}
		else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
			WL_INFORM(("wl%d: scan request blocked for scan in progress\n",
			           wlc->pub->unit));
			err = BCME_NOTREADY;
		}
	}

	return err;
} /* wlc_mac_request_scan */
#if defined(WLTEST) && defined(STA)
void
wlc_assoc_auth_txstatus(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 fc)
{
	wlc_bss_info_t *target_bss = NULL;
	wlc_assoc_t *as;

	if (cfg == NULL)
		return;

	if (fc == FC_AUTH) {
		target_bss = cfg->target_bss;
		wlc_auth_complete(cfg, WLC_E_STATUS_TIMEOUT, &target_bss->BSSID, 0, 0);
	} else if (fc == FC_ASSOC_REQ) {
		as = cfg->assoc;
		wlc_assoc_complete(cfg, WLC_E_STATUS_TIMEOUT, &target_bss->BSSID, 0,
			as->type != AS_ASSOCIATION, 0);
	}
}
#endif // endif
#ifdef STA
#ifdef WL11H
/** Quiet request */
/* XXX
 * is it BSS specific or channel specific? how should we handle multiple BSS
 * if it is channel specific?
 */
static int
wlc_mac_request_quiet(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	wlc_assoc_t *as;
	int err = BCME_OK;

	ASSERT(cfg != NULL);

	as = cfg->assoc;
	ASSERT(as != NULL);

	if (as->state != AS_IDLE) {
		if (as->type == AS_ASSOCIATION) {
			WL_INFORM(("wl%d: should not be attempting to enter Quiet Period "
			           "while an association is in progress, blocking Quiet\n",
			           wlc->pub->unit));
			err = BCME_ERROR;
			ASSERT(0);
		}
		else if (as->type == AS_ROAM) {
			WL_INFORM(("wl%d: Quiet Period starting while roam is in progress, "
			           "aborting roam\n", wlc->pub->unit));
			wlc_assoc_abort(cfg);
		}
	}
	else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		WL_INFORM(("wl%d: Quiet Period starting while scan is in progress, "
		           "aborting scan\n", wlc->pub->unit));
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_11HQUIET);
	}
	else if (!cfg->associated) {
		WL_ERROR(("wl%d: should not be attempting to enter Quiet Period "
		          "if not associated, blocking Quiet\n",
		          wlc->pub->unit));
		err = BCME_ERROR;
	}

	return err;
} /* wlc_mac_request_quiet */
#endif /* WL11H */

/**
 * reassociation request
 * reassoc has lower priority than association, scan (af, scan);
 * has higher priority than roam, rm, and excursion
 */
static int
wlc_mac_request_reassoc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	wlc_assoc_t *as;
	int err = BCME_OK;

	ASSERT(cfg != NULL);

	as = cfg->assoc;
	ASSERT(as != NULL);

	/* bail out if we are in quiet period */
	if ((BSS_QUIET_STATE(wlc->quiet, cfg) & SILENCE)) {
		WL_INFORM(("wl%d: reassoc request blocked for 802.11h Quiet Period\n",
		           wlc->pub->unit));
		err = BCME_EPERM;
	}
	else if (AS_IN_PROGRESS(wlc)) {
		wlc_bsscfg_t *bc = wlc->as_info->assoc_req[0];

		ASSERT(bc != NULL);

		as = bc->assoc;
		ASSERT(as != NULL);

		/* abort any roam in progress */
		if (as->state != AS_IDLE && as->type == AS_ROAM) {
			WL_INFORM(("wl%d: reassoc request while roam is in progress, "
			           "aborting roam\n", wlc->pub->unit));
			wlc_assoc_abort(bc);
		}
		/* bail out if someone is in the process of association */
		else {
			WL_INFORM(("wl%d: reassoc request while association is in progress",
			           wlc->pub->unit));
			err = BCME_BUSY;
		}
	}
	else if (LPRIO_EXCRX_IN_PROGRESS(wlc->scan)) {
		WL_INFORM(("wl%d: reassoc request while LP excursion is in progress, "
		           "aborting LP excursion\n", wlc->pub->unit));
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_NEWASSOC);
	}
	else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		WL_INFORM(("wl%d: reassoc request while scan is in progress",
		           wlc->pub->unit));
		err = BCME_BUSY;
	}
#ifdef WLRM
	else if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: reassoc request while radio measurement is in progress,"
		           " aborting measurement\n", wlc->pub->unit));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */

	return err;
} /* wlc_mac_request_reassoc */
#endif /* STA */

#if defined(WLRM) || defined(WL11K)
/** RM request has lower priority than association/roam/scan */
static int
wlc_mac_request_rm(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	int err = BCME_OK;

#ifdef STA
	/* is any assoc/roam in progress? */
	if (AS_IN_PROGRESS(wlc)) {
		WL_INFORM(("wl%d: radio measure blocked for association in progress\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
	}
	/* is scan engine busy? */
	else if (ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		WL_INFORM(("wl%d: radio measure blocked for scan in progress\n",
		           wlc->pub->unit));
		err = BCME_ERROR;
	}
#endif /* STA */

	return err;
}
#endif /* WLRM || WL11K */

/**
 * Arbitrate off-home channel activities (assoc, roam, rm, af, excursion, scan, etc.)
 *
 * The priorities of different activities are listed below from high to low:
 *
 * - assoc, recreate
 * - af (action frame)
 * - scan
 * - iscan, escan
 * - reassoc
 * - roam
 * - excursion
 * - rm (radio measurement)
 *
 * Returns BCME_OK if the request can proceed, or a specific BCME_ code if the
 * request is blocked. This routine will make sure any lower priority MAC action
 * is aborted. Return > 0 if request is queued (for association).
 */
/* XXX mSTA: What should be done here to support simultaneous association and roam
 * requests? For now seriallize all requests of the same kind and follow the same
 * priorities as they have been (ASSOC > SCAN > ROAM > RM).
 *
 * Note: Only one roam request is allowed.
 */
int
wlc_mac_request_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req)
{
	int err = BCME_OK;
#ifdef STA
	uint type;
#endif // endif

	switch (req) {
#ifdef STA
	case WLC_ACTION_RECREATE_ROAM:
		/* Roam as part of an assoc_recreate process.
		 * Happens when the former AP was not found and we need to look for
		 * another AP supporting the same SSID.
		 */
		ASSERT(cfg != NULL);
		ASSERT(cfg->assoc != NULL);
		ASSERT(cfg->assoc->type == AS_RECREATE);
		ASSERT(cfg->assoc->state == AS_RECREATE_WAIT_RCV_BCN);
		/* FALLSTHRU */
	case WLC_ACTION_ASSOC:
	case WLC_ACTION_RECREATE:
		err = wlc_mac_request_assoc(wlc, cfg, req);
		break;

	case WLC_ACTION_ROAM:
		err = wlc_mac_request_roam(wlc, cfg, req);
		break;

#if defined(WL11H)
	case WLC_ACTION_QUIET:
		err = wlc_mac_request_quiet(wlc, cfg, req);
		break;
#endif /* WL11H */

	case WLC_ACTION_REASSOC:
		err = wlc_mac_request_reassoc(wlc, cfg, req);
		break;
#endif /* STA */

#if defined(WLRM) || defined(WL11K)
	case WLC_ACTION_RM:
		err = wlc_mac_request_rm(wlc, cfg, req);
		break;
#endif /* WLRM || WL11K */

	case WLC_ACTION_SCAN:
	case WLC_ACTION_ISCAN:
	case WLC_ACTION_ESCAN:
	case WLC_ACTION_PNOSCAN:
	case WLC_ACTION_ACTFRAME:
	case WLC_ACTION_LPRIO_EXCRX:
		err = wlc_mac_request_scan(wlc, cfg, req);
		break;

	default:
		err = BCME_ERROR;
		ASSERT(0);
	}

#ifdef STA
	if (err != BCME_OK)
		return err;

	/* TODO: pass assoc type along with mac req so that we can eliminate the switch statement.
	 */

	/* we are granted MAC access! */
	switch (req) {
	case WLC_ACTION_ASSOC:
		type = AS_ASSOCIATION;
		goto request;
	case WLC_ACTION_RECREATE_ROAM:
	case WLC_ACTION_RECREATE:
		type = AS_RECREATE;
		goto request;
	case WLC_ACTION_ROAM:
	case WLC_ACTION_REASSOC:
		type = AS_ROAM;
		/* FALLSTHRU */
	request:
		ASSERT(type != AS_NONE);
		ASSERT(cfg != NULL);

		/* add the request into assoc req list */
		if ((err = wlc_assoc_req_add_entry(wlc, cfg, type, FALSE)) < 0) {
#if defined(BCMDBG) || defined(WLMSG_INFORM)
			char ssidbuf[SSID_FMT_BUF_LEN];
			wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
			WL_INFORM(("wl%d.%d: %s request not granted for SSID %s\n",
			           wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			           WLCASTYPEN(cfg->assoc->type), ssidbuf));
#endif // endif
		}
		/* inform the caller of the request has been queued at index 'err'
		 * when err > 0...
		 */
	}
#endif /* STA */

	return err;
} /* wlc_mac_request_entry */

/** get roam default parameters */
void
BCMATTACHFN(wlc_roam_defaults)(wlc_info_t *wlc, wlcband_t *band, int *roam_trigger,
	uint *roam_delta)
{
	/* set default roam parameters */
	switch (band->radioid) {
	case BCM2050_ID:
		if (band->radiorev == 1) {
			*roam_trigger = WLC_2053_ROAM_TRIGGER;
			*roam_delta = WLC_2053_ROAM_DELTA;
		} else {
			*roam_trigger = WLC_2050_ROAM_TRIGGER;
			*roam_delta = WLC_2050_ROAM_DELTA;
		}
		if (wlc->pub->boardflags & BFL_EXTLNA) {
			*roam_trigger -= 2;
		}
		break;
	case BCM2055_ID:
	case BCM2056_ID:
		*roam_trigger = WLC_2055_ROAM_TRIGGER;
		*roam_delta = WLC_2055_ROAM_DELTA;
		break;
	case BCM2060_ID:
		*roam_trigger = WLC_2060WW_ROAM_TRIGGER;
		*roam_delta = WLC_2060WW_ROAM_DELTA;
		break;

	case NORADIO_ID:
		*roam_trigger = WLC_NEVER_ROAM_TRIGGER;
		*roam_delta = WLC_NEVER_ROAM_DELTA;
		break;

	default:
		*roam_trigger = BAND_5G(band->bandtype) ? WLC_5G_ROAM_TRIGGER :
		        WLC_2G_ROAM_TRIGGER;
		*roam_delta = BAND_5G(band->bandtype) ? WLC_5G_ROAM_DELTA :
		        WLC_2G_ROAM_DELTA;
		WL_INFORM(("wl%d: wlc_roam_defaults: USE GENERIC ROAM THRESHOLD "
		           "(%d %d) FOR RADIO %04x IN BAND %s\n", wlc->pub->unit,
		           *roam_trigger, *roam_delta, band->radioid,
		           BAND_5G(band->bandtype) ? "5G" : "2G"));
		break;
	}

	/* Fill up the default roam profile */
	if (band->roam_prof) {
		memset(band->roam_prof, 0, WL_MAX_ROAM_PROF_BRACKETS * sizeof(wl_roam_prof_t));
		band->roam_prof->roam_flags = WL_ROAM_PROF_DEFAULT;
		band->roam_prof->roam_trigger = (int8)(*roam_trigger);
		band->roam_prof->rssi_lower = WLC_RSSI_MINVAL_INT8;
		band->roam_prof->roam_delta = (int8)(*roam_delta);
		band->roam_prof->rssi_boost_thresh = WLC_JOIN_PREF_RSSI_BOOST_MIN;
		band->roam_prof->nfscan = ROAM_FULLSCAN_NTIMES;
		band->roam_prof->fullscan_period = WLC_FULLROAM_PERIOD;
		band->roam_prof->init_scan_period = WLC_ROAM_SCAN_PERIOD;
		band->roam_prof->backoff_multiplier = 1;
		band->roam_prof->max_scan_period = WLC_ROAM_SCAN_PERIOD;
	}
} /* wlc_roam_defaults */

void
wlc_deauth_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	const struct ether_addr *addr, uint deauth_reason, uint bss_type)
{
#if defined(STA) && defined(WLRM)
	if (WLRM_ENAB(wlc->pub) && (bsscfg && BSSCFG_STA(bsscfg)) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: abort RM due to deauth\n", WLCWLUNIT(wlc)));
		wlc_rm_stop(wlc);
	}
#endif /* STA && WLRM */

#if defined(STA) && defined(WL11K)
	if ((bsscfg && BSSCFG_STA(bsscfg)) && wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: abort RRM due to deauth\n", WLCWLUNIT(wlc)));
		wlc_rrm_stop(wlc);
	}
#endif /* STA && WL11K */
#ifdef PSTA
	/* If the deauthenticated client is downstream, then deauthenticate
	 * the corresponding proxy client from our AP.
	 */
	if (PSTA_ENAB(wlc->pub))
		wlc_psta_deauth_client(wlc->psta, (struct ether_addr *)addr);
#endif /* PSTA */
	wlc_bss_mac_event(wlc, bsscfg, WLC_E_DEAUTH, addr, status, deauth_reason,
		bss_type, 0, 0);
}

void
wlc_deauth_sendcomplete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	wlc_deauth_send_cbargs_t *cbarg = arg;

	/* Is this scb still around */
	bsscfg = WLC_BSSCFG(wlc, cbarg->_idx);
	if (bsscfg == NULL)
		return;

	if ((scb = wlc_scbfind(wlc, bsscfg, &cbarg->ea)) == NULL) {
		MFREE(wlc->osh, arg, sizeof(wlc_deauth_send_cbargs_t));
		return;
	}

#ifdef AP
	/* Reset PS state if needed */
	if (SCB_PS(scb))
		wlc_apps_scb_ps_off(wlc, scb, TRUE);
#endif /* AP */

	WL_ASSOC(("wl%d: %s: deauth complete\n", wlc->pub->unit, __FUNCTION__));
	wlc_bss_mac_event(wlc, SCB_BSSCFG(scb), WLC_E_DEAUTH, (struct ether_addr *)arg,
	              WLC_E_STATUS_SUCCESS, DOT11_RC_DEAUTH_LEAVING, 0, 0, 0);

	if (cbarg->pkt) {
		WLPKTTAGSCBSET(cbarg->pkt, NULL);
	}

	MFREE(wlc->osh, arg, sizeof(wlc_deauth_send_cbargs_t));

	if (BSSCFG_STA(bsscfg)) {
		/* Do this last: ea_arg points inside scb */
		wlc_scbfree(wlc, scb);
	} else {
		/* Clear states and mark the scb for deletion. SCB free will happen */
		/* from the inactivity timeout context in wlc_ap_stastimeout() */
		wlc_scb_clearstatebit(scb, AUTHENTICATED | ASSOCIATED | AUTHORIZED);
		wlc_scb_setstatebit(scb, MARKED_FOR_DELETION);
	}
} /* wlc_deauth_sendcomplete */

void
wlc_disassoc_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint disassoc_reason, uint bss_type,
	uint8 *body, int body_len)
{
#if defined(STA) && defined(BCMCCX) && defined(CCX_SDK)
	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx))
		wlc_ccx_mgmt_status_hdlr(wlc->ccx, addr, disassoc_reason, FALSE, NULL, 0);
#endif	/* STA && BCMCCX */

#if defined(STA) && defined(WLRM)
	if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: abort RM due to receiving disassoc request\n",
		           WLCWLUNIT(wlc)));
		wlc_rm_stop(wlc);
	}
#endif /* STA && WLRM */

#if defined(STA) && defined(WL11K)
	if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: abort RRM due to receiving disassoc request\n",
		           WLCWLUNIT(wlc)));
		wlc_rrm_stop(wlc);
	}
#endif /* STA && WL11K */

#ifdef AP
	if (BSSCFG_AP(bsscfg)) {
#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			struct scb *scbd;

			if ((scbd = wlc_scbfind(wlc, bsscfg, addr)) == NULL)
				WL_ERROR(("%s: null SCB\n", __FUNCTION__));
			else {
				wlfc_MAC_table_update(wlc->wl, &addr->octet[0],
				                      WLFC_CTL_TYPE_MACDESC_DEL,
				                      scbd->mac_address_handle,
				                      ((bsscfg->wlcif == NULL) ? 0 :
				                       bsscfg->wlcif->index));
				wlfc_release_MAC_descriptor_handle(wlc->wlfc_data,
				                                   scbd->mac_address_handle);
				WLFC_DBGMESG(("AP: MAC-DEL for [%02x:%02x:%02x:%02x:%02x:%02x], "
				              "handle: [%d], if:%d\n",
				              addr->octet[0], addr->octet[1], addr->octet[2],
				              addr->octet[3], addr->octet[4], addr->octet[5],
				              scbd->mac_address_handle,
				              ((bsscfg->wlcif == NULL) ? 0 :
				               bsscfg->wlcif->index)));
				scbd->mac_address_handle = 0;
			}
		}
#endif /* PROP_TXSTATUS */

		wlc_enable_btc_ps_protection(wlc, bsscfg, FALSE);

#ifdef WLP2P
		/* reenable P2P in case a non-P2P STA leaves the BSS and
		 * all other associated STAs are P2P client
		 */
		if (BSS_P2P_ENAB(wlc, bsscfg))
			wlc_p2p_enab_upd(wlc->p2p, bsscfg);
#endif // endif

		if (DYNBCN_ENAB(bsscfg) && wlc_bss_assocscb_getcnt(wlc, bsscfg) == 0)
			wlc_bsscfg_bcn_disable(wlc, bsscfg);
	}
#endif /* AP */

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, addr, status, disassoc_reason,
	                  bss_type, body, body_len);

#ifdef BCMWAPI_WAI
	wlc_wapi_station_event(wlc->wapi, bsscfg, addr, NULL, NULL, WAPI_STA_AGING);
#endif // endif
} /* wlc_disassoc_ind_complete */

void
wlc_deauth_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint deauth_reason, uint bss_type,
	uint8 *body, int body_len)
{
	struct scb *scb = wlc_scbfindband(wlc, bsscfg, addr,
		CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	if (scb)
		wlc_scb_clearstatebit(scb, AUTHENTICATED);

#if defined(STA) && defined(BCMCCX) && defined(CCX_SDK)
	if (CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx))
		wlc_ccx_mgmt_status_hdlr(wlc->ccx, addr, deauth_reason, FALSE, NULL, 0);
#endif	/* STA && BCMCCX */

#if defined(STA) && defined(WLRM)
	if (WLRM_ENAB(wlc->pub) && wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: abort RM due to receiving deauth request\n",
		           WLCWLUNIT(wlc)));
		wlc_rm_stop(wlc);
	}
#endif /* STA && WLRM */

#if defined(STA) && defined(WL11K)
	if (wlc_rrm_inprog(wlc)) {
		WL_INFORM(("wl%d: abort RRM due to receiving deauth request\n",
		           WLCWLUNIT(wlc)));
		wlc_rrm_stop(wlc);
	}
#endif /* STA && WL11K */

#ifdef AP
	if (BSSCFG_AP(bsscfg)) {
		wlc_enable_btc_ps_protection(wlc, bsscfg, FALSE);
#ifdef WLP2P
		/* reenable P2P in case a non-P2P STA leaves the BSS and
		 * all other associated STAs are P2P client
		 */
		if (BSS_P2P_ENAB(wlc, bsscfg))
			wlc_p2p_enab_upd(wlc->p2p, bsscfg);
#endif // endif
	}
#endif /* AP */

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_DEAUTH_IND, addr, status, deauth_reason,
	                  bss_type, body, body_len);

#ifdef BCMWAPI_WAI
	wlc_wapi_station_event(wlc->wapi, bsscfg, addr, NULL, NULL, WAPI_STA_AGING);
#endif // endif
} /* wlc_deauth_ind_complete */

void
wlc_assoc_bcn_mon_off(wlc_bsscfg_t *cfg, bool off, uint user)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_cxn_t *cxn = cfg->cxn;

	(void)wlc;

	WL_ASSOC(("wl%d.%d: %s: off %d user 0x%x\n",
	          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, off, user));

	if (off)
		mboolset(cxn->ign_bcn_lost_det, user);
	else
		mboolclr(cxn->ign_bcn_lost_det, user);
}

#ifdef STA
int
wlc_assoc_iswpaenab(wlc_bsscfg_t *cfg, bool wpa)
{
	int ret = 0;

	if WLCAUTOWPA(cfg)
		ret = TRUE;
	else if (wpa) {
		if (cfg->WPA_auth != WPA_AUTH_DISABLED)
			ret = TRUE;
	}
	else if (cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED)
		ret = TRUE;

	return ret;
}

bool
wlc_assoc_check_aplost_ok(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	return TRUE;
}

void
wlc_roam_timer_expiry(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;

	cfg->roam->timer_active = FALSE;
	/* A beacon was received since uatbtt */
	if (cfg->roam->time_since_bcn * 1000u < ((cfg->current_bss->beacon_period <<
		10) / 1000u) * UATBTT_TO_ROAM_BCN)
		return;

	if (cfg->pm->check_for_unaligned_tbtt) {
		WL_ASSOC(("wl%d: ROAM: done for unaligned TBTT, "
			"time_since_bcn %d Sec\n",
			WLCWLUNIT(cfg->wlc), cfg->roam->time_since_bcn));
		wlc_set_uatbtt(cfg, FALSE);
	}
	wlc_roam_bcns_lost(cfg);
}

#ifdef WLABT

static void
wlc_check_adaptive_bcn_timeout(wlc_bsscfg_t *cfg)
{
	wlc_roam_t *roam = cfg->roam;

	if (roam->prev_bcn_timeout != 0) {
		/* adaptive bcn_timeout is applied */
		bool is_high_rssi = cfg->link->rssi > ABT_HIGH_RSSI;
		if (is_high_rssi || P2P_ACTIVE(cfg->wlc)) {
			roam->bcn_timeout = roam->prev_bcn_timeout;
			roam->prev_bcn_timeout = 0;
			WL_ASSOC(("wl%d: Restore bcn_timeout to %d, RSSI %d, P2P %d\n",
				cfg->wlc->pub->unit, roam->bcn_timeout,
				cfg->link->rssi, P2P_ACTIVE(cfg->wlc)));
		}
	} else {
		/* adaptive bcn_timeout is NOT applied, so check the status */
		wlcband_t *band = cfg->wlc->band;
		bool is_low_trigger = band->roam_trigger ==
			(WLC_AUTO_ROAM_TRIGGER - WLC_ROAM_TRIGGER_STEP);
		bool is_low_rssi = cfg->link->rssi < WLC_AUTO_ROAM_TRIGGER;
		if (is_low_trigger && is_low_rssi && !P2P_ACTIVE(cfg->wlc) &&
			(roam->bcn_timeout > ABT_MIN_TIMEOUT)) {
			roam->prev_bcn_timeout = roam->bcn_timeout;
			roam->bcn_timeout = ABT_MIN_TIMEOUT;
			WL_ASSOC(("wl%d: Setting bcn_timeout to %d, RSSI %d, P2P %d\n",
				cfg->wlc->pub->unit, roam->bcn_timeout,
				cfg->link->rssi, P2P_ACTIVE(cfg->wlc)));
		}
	}
}
#endif /* WLABT */

static
void wlc_roam_period_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	if (wlc->band->roam_prof) {
		wlc_roam_t *roam = cfg->roam;
		int idx = roam->roam_prof_idx;
		wl_roam_prof_t *roam_prof = &wlc->band->roam_prof[idx];

		ASSERT((idx >= 0) && (idx < WL_MAX_ROAM_PROF_BRACKETS));

		if (roam_prof->max_scan_period && roam_prof->backoff_multiplier) {
			roam->partialscan_period *= roam_prof->backoff_multiplier;
			roam->partialscan_period =
				MIN(roam->partialscan_period, roam_prof->max_scan_period);
		}
	}
}

/** Return TRUE if current roam scan is background roam scan */
bool wlc_roam_scan_islazy(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool roam_scan_isactive)
{
	wlcband_t *band = wlc->band;

	if (NBANDS(wlc) > 1) {
		band = wlc->bandstate[CHSPEC_IS2G(cfg->current_bss->chanspec) ?
			BAND_2G_INDEX : BAND_5G_INDEX];
	}

	/* Check for active roam scan when requested */
	if (roam_scan_isactive) {
		if (!AS_IN_PROGRESS(wlc) || !SCAN_IN_PROGRESS(wlc->scan))
			return FALSE;

		if ((cfg->assoc->type != AS_ROAM) || (cfg->roam->reason != WLC_E_REASON_LOW_RSSI))
			return FALSE;
	}

	/* More checks */
	if ((cfg->pm->PM != PM_FAST) || !cfg->associated || !band->roam_prof)
		return FALSE;

	return (band->roam_prof[cfg->roam->roam_prof_idx].roam_flags & WL_ROAM_PROF_LAZY);
}

/** Assume already checked that this is background roam scan */
bool wlc_lazy_roam_scan_suspend(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlcband_t *band = wlc->band;

	if (NBANDS(wlc) > 1) {
		band = wlc->bandstate[CHSPEC_IS2G(cfg->current_bss->chanspec) ?
			BAND_2G_INDEX : BAND_5G_INDEX];
	}

	ASSERT(wlc_roam_scan_islazy(wlc, cfg, FALSE));
	return (band->roam_prof[cfg->roam->roam_prof_idx].roam_flags & WL_ROAM_PROF_SUSPEND);
}

/** Assume already checked that this is background roam scan */
bool wlc_lazy_roam_scan_sync_dtim(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlcband_t *band = wlc->band;

	if (NBANDS(wlc) > 1) {
		band = wlc->bandstate[CHSPEC_IS2G(cfg->current_bss->chanspec) ?
			BAND_2G_INDEX : BAND_5G_INDEX];
	}

	ASSERT(wlc_roam_scan_islazy(wlc, cfg, FALSE));
	return (band->roam_prof[cfg->roam->roam_prof_idx].roam_flags & WL_ROAM_PROF_SYNC_DTIM);
}

void wlc_roam_prof_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool reset)
{
	wlc_roam_t *roam = cfg->roam;
	wlcband_t *band = wlc->band;

	if (wlc->band->roam_prof) {
		/* Full multiple roam profile support */
		int16 idx = roam->roam_prof_idx;
		int rssi = cfg->link->rssi;
		wl_roam_prof_t *roam_prof;
		wlcband_t *otherband = NULL;

		/* The current band/channel may not be the same as the current BSS */
		if (NBANDS(wlc) > 1) {
			band = wlc->bandstate[CHSPEC_IS2G(cfg->current_bss->chanspec) ?
				BAND_2G_INDEX : BAND_5G_INDEX];
			otherband = wlc->bandstate[CHSPEC_IS2G(cfg->current_bss->chanspec) ?
				BAND_5G_INDEX : BAND_2G_INDEX];
		}

		ASSERT((idx >= 0) && (idx < WL_MAX_ROAM_PROF_BRACKETS));
		ASSERT((rssi >= WLC_RSSI_MINVAL_INT8) && (rssi <= WLC_RSSI_INVALID));

		/* Check and update roam profile according to RSSI */
		if (reset) {
			idx = 0;
		} else {
			if ((rssi >= (band->roam_trigger + wlc->roam_rssi_cancel_hysteresis)) &&
			    (idx > 0))
				idx--;
			else if ((band->roam_prof[idx].rssi_lower != WLC_RSSI_MINVAL_INT8) &&
			         (rssi < band->roam_prof[idx].rssi_lower) &&
			         (idx < (WL_MAX_ROAM_PROF_BRACKETS - 1)))
				idx++;
			else
				return;
		}

		roam->roam_prof_idx = idx;
		roam_prof = &band->roam_prof[idx];

		/* Update roam parameters when switching roam profile */
		if (roam_prof->roam_flags & WL_ROAM_PROF_NO_CI)
			roam->ci_delta |= ROAM_CACHEINVALIDATE_DELTA_MAX;
		else
			roam->ci_delta &= (ROAM_CACHEINVALIDATE_DELTA_MAX - 1);
		band->roam_trigger = roam_prof->roam_trigger;
		band->roam_delta = roam_prof->roam_delta;
		if (NBANDS(wlc) > 1) {
			cfg->join_pref_rssi_delta.band = (uint8)otherband->bandtype;
			cfg->join_pref_rssi_delta.rssi = roam_prof->rssi_boost_delta;
			roam->roam_rssi_boost_thresh = roam_prof->rssi_boost_thresh;
		}
		roam->nfullscans = (uint8)roam_prof->nfscan;
		roam->fullscan_period = roam_prof->fullscan_period;
		roam->partialscan_period = roam_prof->init_scan_period;

		/* Trigger immediate roam scan when:
		 *	Switching to non-lazy profile
		 *	Upon roam profile reset (new association & new profile configuration)
		 * (minimum delay enforced)
		 */
		if ((roam_prof->roam_flags & WL_ROAM_PROF_LAZY) && !reset)
			roam->scan_block = roam->partialscan_period;
		else
			roam->scan_block = 0;	/* Immediate scan */
		roam->scan_block = MAX(roam->scan_block, wlc->pub->tunables->scan_settle_time);

		/* Basic update to other bands upon reset: This is not really useful.
		 * It just keeps legacy roam trigger/delta read command in-sync.
		 */
		if ((NBANDS(wlc) > 1) && reset) {
			if (otherband->roam_prof) {
				otherband->roam_trigger = otherband->roam_prof->roam_trigger;
				otherband->roam_delta = otherband->roam_prof->roam_delta;
			}
		}

		WL_SRSCAN(("ROAM prof[%d:%d]: trigger=%d delta=%d\n",
			band->bandtype, idx, band->roam_trigger, band->roam_delta));
		WL_ASSOC(("ROAM prof[%d:%d]: trigger=%d delta=%d\n",
			band->bandtype, idx, band->roam_trigger, band->roam_delta));
	}
} /* wlc_roam_prof_update */

/** This is to maintain the backward compatibility */
void wlc_roam_prof_update_default(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_roam_t *roam = cfg->roam;
	wlcband_t *band;
	uint i;

	for (i = 0; i < NBANDS(wlc); i++) {
		band = wlc->bandstate[i];

		/* Only for no roam profile support and single default profile */
		if (band->roam_prof && !(band->roam_prof->roam_flags & WL_ROAM_PROF_DEFAULT))
			return;
	}

	for (i = 0; i < NBANDS(wlc); i++) {
		band = wlc->bandstate[i];

		/* Simple roam profile depending on multi_ap state:
		 * The multi_ap state is set at the end of association scan,
		 * so it is used here to set up roaming parameters.
		 */
		if (roam->multi_ap && roam->roam_delta_aggr && roam->roam_trigger_aggr) {
			band->roam_trigger = roam->roam_trigger_aggr;
			band->roam_delta = roam->roam_delta_aggr;
		} else {
			/* restore roam parameters to default */
			band->roam_trigger = band->roam_trigger_def;
			band->roam_delta = band->roam_delta_def;
		}

		/* Now update the single default profile for compatibility */
		if (band->roam_prof) {
			band->roam_prof->roam_trigger = (int8)band->roam_trigger;
			band->roam_prof->roam_delta = (int8)band->roam_delta;

			/* Update the following when BSS is known */
			band->roam_prof->nfscan = roam->nfullscans;
			band->roam_prof->fullscan_period = (uint16)roam->fullscan_period;
			band->roam_prof->init_scan_period = (uint16)roam->partialscan_period;
			band->roam_prof->backoff_multiplier = 1;
			band->roam_prof->max_scan_period = (uint16)roam->partialscan_period;

			ASSERT((band->bandtype == WLC_BAND_2G) ||
			       (band->bandtype == WLC_BAND_5G));
			ASSERT(cfg->join_pref_rssi_delta.band < WLC_BAND_ALL);
			if ((NBANDS(wlc) > 1) &&
			    (cfg->join_pref_rssi_delta.band != WLC_BAND_AUTO) &&
			    (cfg->join_pref_rssi_delta.band != band->bandtype)) {
				band->roam_prof->rssi_boost_delta = cfg->join_pref_rssi_delta.rssi;
				band->roam_prof->rssi_boost_thresh = roam->roam_rssi_boost_thresh;
			}
		}
	}

	WL_SRSCAN(("ROAM prof[multi_ap=%d]: trigger=%d delta=%d\n",
		roam->multi_ap, wlc->band->roam_trigger, wlc->band->roam_delta));
	WL_ASSOC(("ROAM prof[multi_ap=%d]: trigger=%d delta=%d\n",
		roam->multi_ap, wlc->band->roam_trigger, wlc->band->roam_delta));
}
#endif /* STA */

int8
wlc_assoc_get_as_state(wlc_bsscfg_t *cfg)
{
	if (!BSSCFG_STA(cfg)) {
		return -1;
	} else {
		if (cfg->assoc == NULL) {
			return -1;
		}
		return cfg->assoc->state;
	}
}

#if defined(CLIENT_CSA) && defined(WLDFS)
static bool
wlc_assoc_do_dfs_reentry_cac_if_required(wlc_bsscfg_t *cfg)
{
	wlc_bss_info_t *bi;
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	bool dfs_cac_started = FALSE;

	bi = wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last];
	if ((wlc_radar_chanspec(wlc->cmi, bi->chanspec)) &&
		!(wlc_cac_is_clr_chanspec(wlc->dfs, bi->chanspec))) {
		/* DFS cac is required on this channel, start CAC */
		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, bi->chanspec);
		wlc_enable_mac(wlc);
		phy_radar_detect_enable((phy_info_t *)WLC_PI(wlc), TRUE);
		wlc_assoc_timer_del(wlc, cfg);
		wl_add_timer(wlc->wl, as->timer,
			(wlc_dfs_get_cactime_ms(wlc->dfs) + DELAY_10MS), FALSE);
		wlc_assoc_change_state(cfg, AS_DFS_CAC_START);
		wlc_set_dfs_cacstate(wlc->dfs, ON);
		dfs_cac_started = TRUE;
	}
	return dfs_cac_started;
}
#endif /* CLIENT_CSA && WLDFS */

#if defined(WL_SAE) && defined(STA)
int
wlc_ext_auth_tx_complete(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt,
		void *arg)
{
	/* Once receive the Auth request 1 or 3 from upper layer,
	 * stop the timer and start a new timer once ACK received
	 */
	wlc_assoc_timer_del(wlc, cfg);

	if (wlc_pcb_fn_register(wlc->pcb, wlc_auth_tx_complete,
		arg, pkt)) {
		WL_ERROR(("wl%d: %s out of pkt callbacks\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}
	return BCME_OK;
}

void
wlc_assoc_continue_post_auth1(wlc_bsscfg_t *cfg, struct scb *scb)
{
	wlc_info_t *wlc = cfg->wlc;
	void *pkt = NULL;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	wlc_assoc_t *as = cfg->assoc;

	wlc_assoc_change_state(cfg, AS_SENT_ASSOC);
	pkt = wlc_join_assoc_start(wlc, cfg, scb, target_bss, cfg->associated);

#if defined(WLP2P) && defined(BCMDBG)
	if (WL_P2P_ON()) {
		int bss = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
		uint16 state = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_ST(wlc, bss));
		uint16 next_noa = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_N_NOA(wlc, bss));
		uint16 hps = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_HPS_OFFSET(wlc));

		WL_P2P(("wl%d: %s: queue ASSOC at tick 0x%x ST 0x%04X "
					"N_NOA 0x%X HPS 0x%04X\n",
					wlc->pub->unit, __FUNCTION__,
					R_REG(wlc->osh, D11_TSFTimerLow(wlc)),
					state, next_noa, hps));
	}
#endif /* WLP2P && BCMDBG */

	wlc_assoc_timer_del(wlc, cfg);

	if (pkt != NULL) {
		if (wlc_pcb_fn_register(wlc->pcb, wlc_assocreq_complete,
			(void *)(uintptr)cfg->ID, pkt)) {
			WL_ERROR(("wl%d: %s out of pkt callbacks\n",
				wlc->pub->unit, __FUNCTION__));
			wlc_assoc_abort(cfg);
			return;
		}
	}
	/* Fall back to timer method */
	wl_add_timer(wlc->wl, as->timer, WECA_ASSOC_TIMEOUT +10, 0);
}
/* Trigger the Assoc from upper layer */
int
wlc_start_assoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea)
{
	struct scb *scb = NULL;
	if ((ea == NULL) ||
			ETHER_ISMULTI(ea) || ETHER_ISNULLADDR(ea)) {
		WL_ASSOC(("wl%d: %s:  Wrong bssid, Couldn't start Assoc \n",
			wlc->pub->unit, __FUNCTION__));
		return (BCME_BADARG);
	}
	wlc_assoc_timer_del(wlc, bsscfg);
	scb = wlc_scbfind(wlc, bsscfg, ea);

	if (!scb) {
		WL_ASSOC(("wl%d: %s:  Couldn't start Assoc \n",
			wlc->pub->unit, __FUNCTION__));
		wlc_assoc_abort(bsscfg);
		return BCME_NOTREADY;
	}
	wlc_assoc_continue_post_auth1(bsscfg, scb);
	return BCME_OK;
}
#endif /* WL_SAE */
