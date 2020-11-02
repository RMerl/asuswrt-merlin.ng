/*
 * 802.11e CAC protocol implementation
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
 * $Id: wlc_cac.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * QoS / 802.11e related. Allows QoS to be maintained under various network loads.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [CallAdmissionControl]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef WLCAC
#error "WLCAC is not defined"
#endif	/* WLCAC */

/* CAC only works when WME is defined */
#ifndef WME
#error "WME is not defined"
#endif	/* WME */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#include <proto/vlan.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_frmutil.h>
#include <wlc_phy_hal.h>
#include <wl_export.h>
#include <wlc_rm.h>
#ifdef	BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include <wlc_cac.h>
#ifdef WLC_HIGH_ONLY
#include <wlc_rpctx.h>
#endif // endif
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_tx.h>
#include <wlc_nar.h>
#include <wlc_scb_ratesel.h>

#define DOT11E_TSPEC_IE	(WME_OUI"\x02\x02\x01")	/* oui, type, subtype, ver */
#define DOT11E_TSPEC_OUI_TYPE_LEN (DOT11_OUI_LEN + 3) /* include oui, type, subtype, ver */

#define USEC_PER_MSEC		1000L	/* convert usec time to ms */

#define WLC_CAC_TSPECARG_COPYSIZE	44	/* TSPEC from nom_msdu_size to delay_bound */

/* Minimum phy rate for b/ag mode */
#define CAC_B_MIN_PHYRATE_BPS	1000000	/* b mode minimum phy rate (bps) */
#define CAC_AG_MIN_PHYRATE_BPS	6000000	/* ag mode minimum phy rate (bps) */

/* lower and upper limit for bandwidth managed by admission control */
#define	AC_BW_DEFAULT 70		/* in terms of percentage */

/* ADDTS Timeout fudge factor when STA is in power save mode */
#define	CAC_ADDTS_RESP_TIMEOUT_FF	700

/* Suggested surplus bandwidth
 * surplus BW, 3 bits for integer & 13 bits for decimal (surplus BW
 * includes collision, retries, rate algorithm, etc)
 */
#define CAC_SURPLUS_BW_11	0x2200	/* surplus bandwidth fudge factor 1.1 */
#define CAC_SURPLUS_BW_13	0x2600	/* surplus bandwidth fudge factor 1.3 */
#define CAC_SURPLUS_BW_14	0x2800	/* surplus bandwidth fudge factor 1.4 */

#define	WLC_CAC_GET_INACTIVITY_INTERVAL(ts) \
	((ts)->inactivity_interval/1000000)

/* structure used to housekeeping TSPEC */
typedef struct tsentry {
	struct tsentry *next;	/* pointer to next structure */
	int ts_state;		/* state of TSPCE accept, reject, pending, etc. */
	tspec_t tspec;		/* tspec structure */
	uchar dialog_token;	/* dialog token */
} tsentry_t;

/* possible states for ts_state variable */
#define CAC_STATE_TSPEC_PENDING		0	/* TSPEC in pending state */
#define CAC_STATE_TSPEC_ACCEPTED	1	/* TSPEC in accepted state */
#define CAC_STATE_TSPEC_REJECTED	2	/* TSPEC got rejected */
#define CAC_STATE_TSPEC_WAIT_RESP	7	/* TSPEC in wait for ADDTS resp. state */
#define	CAC_STATE_TSPEC_UPDATE_PENDING	8	/* TSPEC awaiting update state */
#define	CAC_STATE_TSPEC_UPDATE_WAIT_RESP	9	/* TSPEC Update awaiting resp */
#define CAC_STATE_TSPEC_ANY		-1	/* TSPEC any state */

#define	CAC_MAX_TSPEC_PER_AC	1

/* 1 second interval represented in 32us units */
#define	AVAILABLE_MEDIUM_TIME_32US	(1000000 >> 5)

/* in 32us units */
#define MEDIUM_TIME_BE_32US \
	((AVAILABLE_MEDIUM_TIME_32US * (100 - AC_BW_DEFAULT))/100)

#define TSPEC_NOM_MSDU_MASK		0x7fff

#define USEC32_TO_USEC(x)	((x) << 5)
#define USEC_TO_USEC32(x)	((x) >> 5)

/* Set of duration and the lenght of pakcet cache in CAC module */
typedef struct wlc_cac_dur_len_map {
	uint dur;
	uint pktlen;
} wlc_cac_dur_len_map_t;

/* internal parameters for each AC */
typedef struct wlc_cac_dur_cache {
	wlc_cac_dur_len_map_t dur_latest[NUMPRIO];	/* Duration of last pkt, for each prio */
	wlc_cac_dur_len_map_t cached_dur[NUMPRIO];	/* cached duration, for each prio */
} wlc_cac_dur_cache_t;

/* internal parameters for each AC */
typedef struct wlc_cac_ac {
	uint tot_medium_time;		/* total medium time AP granted pre AC (us) */
	uint used_time;				/* amount medium time STA used (us) */
	wlc_cac_dur_cache_t *dur_cache;	/* For ROM invalidation */
	uint8 nom_phy_rate;		/* negotiated nominal phy rate */
	bool admitted;			/* Admission state */
	uint inactivity_interval;	/* Denotes inactivity interval in seconds */
	uint inactivity_limit;		/* Inactivity limit as given by the TSPEC */
	uint8 inactivity_tid;			/* Inactive TID */
#ifdef BCMCCX
	struct ccx_ts_metrics ccx_tsm;	/* CCX Traffic Stream Metric struct */
#endif	/* BCMCCX */
} wlc_cac_ac_t;

/* CAC main structure */
struct wlc_cac {
	wlc_info_t *wlc;					/* access to wlc_info_t structure */
	wlc_cac_ac_t ac_settings[AC_COUNT];	/* Admission control info each ac */
	bool waiting_resp;					/* tspec waiting for addts resp */
	uint ts_delay;						/* minimum inter- ADDTS time (TU) */
	uint addts_timeout;				/* timeout before sending DELTS (ms) */
	struct wl_timer *addts_timer;	/* timer for addts response timeout */
	struct ether_addr curr_bssid;	/* current bssid */
	wlc_bsscfg_t	*curr_bsscfg;	/* current bsscfg */
	int	scb_handle;					/* scb cubby handle */
	bool roamed;			/* roamed to a new AP */
	uint8 ccx_roam_cnt;		/* count # of roam between reporting interval */
	uint16 ccx_roam_delay;	/* latest roam delay in usec */
	uint32	flags;					/* bit map for indicating various states */
	uint16	available_medium_time;	/* Total available medium time AP specific 32us units */
	uint16	admctl_bw_percent;		/* Percentage of bw that is admission controlled */
	uint	cached_medium_time;		/* previously alloted medium time before update */
	tsentry_t *tsentryq;			/* pointer to tspec list, present only for STA */
	int	rfaware_lifetime;		/* RF awareness lifetime when no TSPEC */
};

static void wlc_cac_watchdog(void *hdl);

/* iovar table */
enum {
	IOV_CAC,
	IOV_CAC_ADDTS_TIMEOUT,
	IOV_CAC_ADDTS,
	IOV_CAC_DELTS,
	IOV_CAC_TSLIST,
	IOV_CAC_TSPEC,
	IOV_CAC_TSLIST_EA,
	IOV_CAC_TSPEC_EA,
	IOV_CAC_DELTS_EA,
	IOV_CAC_AC_BW
};

/* Admission control information */
struct cac_scb_acinfo {
	wlc_cac_ac_t cac_ac[AC_COUNT];	/* Admission control info each ac */
#ifdef	AP
	tsentry_t *tsentryq;			/* pointer to tspec list */
#endif // endif
};

typedef struct cac_scb_acinfo cac_scb_acinfo_t;

#define SCB_ACINFO(acinfo, scb) ((struct cac_scb_acinfo *)SCB_CUBBY(scb, (acinfo)->scb_handle))

static const bcm_iovar_t cac_iovars[] = {
	{"cac", IOV_CAC, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{"cac_addts_timeout", IOV_CAC_ADDTS_TIMEOUT, 0, IOVT_INT32, 0},
	{"cac_addts", IOV_CAC_ADDTS, 0, IOVT_BUFFER, sizeof(tspec_arg_t)},
	{"cac_delts", IOV_CAC_DELTS, 0, IOVT_BUFFER, sizeof(tspec_arg_t)},
	{"cac_tslist", IOV_CAC_TSLIST, 0, IOVT_BUFFER, 0},
	{"cac_tspec", IOV_CAC_TSPEC, 0, IOVT_BUFFER, sizeof(tspec_arg_t)},
	{"cac_tslist_ea", IOV_CAC_TSLIST_EA, 0, IOVT_BUFFER, 0},
	{"cac_tspec_ea", IOV_CAC_TSPEC_EA, 0, IOVT_BUFFER, sizeof(tspec_per_sta_arg_t)},
	{"cac_delts_ea", IOV_CAC_DELTS_EA, 0, IOVT_BUFFER, sizeof(tspec_per_sta_arg_t)},
	{"cac_ac_bw", IOV_CAC_AC_BW, (IOVF_SET_DOWN), IOVT_INT8, 0},
	{NULL, 0, 0, 0, 0}
};

static uint wlc_cac_calc_pkt_dur(wlc_cac_t *cac, struct scb *scb,  uint pktlen);
static int wlc_cac_iovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_cac_down(void *hdl);
static tsentry_t *wlc_cac_tsentry_find(tsentry_t *ts, uint8 tid);
static void wlc_cac_tsentry_append(wlc_cac_t *cac, tsentry_t *ts, tsentry_t **ptsentryq);
static int wlc_cac_tsentry_removefree(wlc_cac_t *cac, uint8 tid, uint event_status, uint code,
	tsentry_t **ptsentryq, bool call_event);
static void wlc_cac_medium_time_recal(wlc_cac_t *cac, struct cac_scb_acinfo *scb_acinfo);
static int wlc_cac_tspec_req_send(wlc_cac_t *cac, tspec_t *ts, uint8 action,
	uint8 dialog_token, int reason_code, void **pkt, struct scb *scb);
static int wlc_cac_addts_req_send(wlc_cac_t *cac, tspec_t *ts, uint8 dialog_token);
static int wlc_cac_tspec_send(wlc_cac_t *cac, tsentry_t *tsentryq);
static uint8 *wlc_cac_ie_find(uint8 *tlvs, uint *tlvs_len, uint8 id, uint max_len,
const	int8 *str, uint str_len);
static int wlc_cac_sta_addts(wlc_cac_t *cac, tspec_arg_t *tspec_arg);
static int wlc_cac_sta_delts(wlc_cac_t *cac, tspec_arg_t *tspec_arg, struct scb *scb,
	tsentry_t *tsentryq);
static uint8 *wlc_cac_tspec_rx_process(wlc_cac_t *cac, uint8 *body, uint *body_len,
	uint8 *tid, uint8 *user_prio, struct cac_scb_acinfo *scb_acinfo, int *err);
static int wlc_cac_ie_process(wlc_cac_t *cac, uint8 *tlvs, uint tlvs_len,
	uint8 *tid, uint8 *user_prio, struct scb *scb);
#ifdef BCMCCX
static bool wlc_ccx_ts_rateset_ie(wlc_cac_t *cac, uint8 *ie, int ac);
static int wlc_ccx_nom_phyrate(wlc_cac_t *cac, uint8 *tlvs, uint tlvs_len);
static uint8 *wlc_cac_ccx_ie_process(wlc_cac_t *cac, uint8 *tlvs, uint *tlvs_len,
	uint8 ac, int *err);
static void wlc_ccx_iapp_tsmetrics_send(wlc_cac_t *cac, uint8 ac);
#endif	/* BCMCCX */
static void wlc_cac_addts_resp(wlc_cac_t *cac, uint8 *body, uint body_len,
	struct scb *scb);
static void wlc_cac_delts_req(wlc_cac_t *cac, uint8 *body, uint body_len,
	struct scb *scb);
static void wlc_cac_addts_timeout(void *arg);
static void wlc_cac_addts_ind_complete(wlc_info_t *wlc, uint status, struct ether_addr *addr,
        uint wme_status, uint tid, uint ts_delay);
static void wlc_cac_delts_ind_complete(wlc_info_t *wlc, uint status, struct ether_addr *addr,
        uint wme_status, uint tid);
static void wlc_cac_tspec_state_change(wlc_cac_t *cac, int old_state,
	uint new_state, tsentry_t *tsentryq);
static void wlc_cac_ac_param_reset(wlc_cac_t *cac, uint8 ac,
	struct cac_scb_acinfo *scb_acinfo);
static int wlc_cac_scb_acinfo_init(void *context, struct scb *scb);
static void wlc_cac_scb_acinfo_deinit(void *context, struct scb *scb);
#ifdef BCMDBG
static void wlc_cac_scb_acinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b);
#else
#define wlc_cac_scb_acinfo_dump NULL
#endif /* BCMDBG */
static int wlc_cac_get_tsinfo_list(wlc_cac_t *cac, tsentry_t *ts, void *buff, uint32 bufflen);
static int
wlc_cac_get_tspec(wlc_cac_t *cac, tsentry_t *tsentryq, tspec_arg_t *tspec_arg, uint8 tid);

#ifdef	AP
static bool wlc_cac_is_tsparam_valid(wlc_cac_t *cac, tspec_t *ts, int ac, struct scb *scb);
static int wlc_cac_addts_resp_send(wlc_cac_t *cac, tspec_t *ts,
	struct scb *scb, uint32 status, uint8 dialog_token);
static int wlc_cac_check_tspec_for_admission(wlc_cac_t *cac,
	tspec_t *ts, uint16 tspec_medium_time, struct scb *scb);
static int wlc_cac_tspec_resp_send(wlc_cac_t *cac, tspec_t *ts, uint8 action,
	int reason_code, uint8 dialog_token, void **pkt, struct scb *scb);
static int wlc_cac_process_addts_req(wlc_cac_t *cac, uint8 *body,
	uint body_len, struct scb *scb);
static uint16 wlc_cac_generate_medium_time(wlc_cac_t *cac, tspec_t *ts, struct scb *scb);
#endif /* AP */

#ifdef BCMDBG
void wlc_print_tspec(tspec_t *ts);
#endif /* BCMDBG */

/* IE mgmt */
#ifdef BCMCCX
#ifdef STA
static uint wlc_cac_calc_tspec_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_cac_write_tspec_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_cac_calc_ts_rs_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_cac_write_ts_rs_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_cac_parse_tspec_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif /* STA */
#endif /* BCMCCX */

#ifdef WLFBT
static uint wlc_cac_fbt_calc_tspec_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_cac_fbt_write_tspec_ie(void *ctx, wlc_iem_build_data_t *data);
#endif /* WLFBT */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_cac_t *
BCMATTACHFN(wlc_cac_attach)(wlc_info_t *wlc)
{
	wlc_cac_t *cac;
	int i;
#ifdef BCMCCX
#ifdef STA
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 arsfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
#endif // endif
#endif /* BCMCCX */

	if (!(cac = (wlc_cac_t *)MALLOC(wlc->osh, sizeof(wlc_cac_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	bzero((char *)cac, sizeof(wlc_cac_t));
	cac->wlc = wlc;

	WL_CAC(("wl%d: wlc_cac_attach: registering CAC module\n", wlc->pub->unit));
	if (wlc_module_register(wlc->pub, cac_iovars, "cac", cac, wlc_cac_iovar,
		wlc_cac_watchdog, NULL, wlc_cac_down)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "cac", (dump_fn_t)wlc_dump_cac, (void *)cac);
#endif // endif
	/* initialize nominal rate based on band type and mode */
	for (i = 0; i < AC_COUNT; i++) {
		if ((wlc->band->bandtype == WLC_BAND_2G) &&
			(wlc->band->gmode == GMODE_LEGACY_B))
			cac->ac_settings[i].nom_phy_rate = WLC_RATE_11M;
		else
			cac->ac_settings[i].nom_phy_rate = WLC_RATE_24M;
	}

	if (!(cac->addts_timer = wl_init_timer(wlc->wl, wlc_cac_addts_timeout, wlc, "addts"))) {
		WL_ERROR(("wl%d: wlc_cac_attach: wl_init_timer for addts-timer failed\n",
			wlc->pub->unit));
		goto fail;
	}

	cac->addts_timeout = CAC_ADDTS_RESP_TIMEOUT;
	cac->tsentryq = NULL;

	cac->available_medium_time = AVAILABLE_MEDIUM_TIME_32US - MEDIUM_TIME_BE_32US;
	cac->admctl_bw_percent = AC_BW_DEFAULT ;	/* 70% by default */

	/* reserve the cubby in the scb container for per-scb private data */
	cac->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct cac_scb_acinfo),
		wlc_cac_scb_acinfo_init, wlc_cac_scb_acinfo_deinit,
		(scb_cubby_dump_t) wlc_cac_scb_acinfo_dump, (void *)wlc);

	if (cac->scb_handle < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt calc/build callbacks */
#ifdef BCMCCX
#ifdef STA
	/* calc/build */
	/* assocreq/reassocreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WME_TS,
	      wlc_cac_calc_tspec_ie_len, wlc_cac_write_tspec_ie, cac) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, tspec in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_CCX_TS_RS,
	      wlc_cac_calc_ts_rs_ie_len, wlc_cac_write_ts_rs_ie, cac) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, ts rs in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* parse */
	/* assocresp/reassocresp */
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arsfstbmp, WLC_IEM_VS_IE_PRIO_WME_TS,
	                                wlc_cac_parse_tspec_ie, cac) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, tspec in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */
#endif /* BCMCCX */

#ifdef WLFBT
	/* reassocreq */
	if (WLFBT_ENAB(wlc->pub)) {
		if (wlc_ier_vs_add_build_fn(wlc->ier_ric, WLC_IEM_VS_IE_PRIO_WME_TS,
		    wlc_cac_fbt_calc_tspec_ie_len, wlc_cac_fbt_write_tspec_ie, cac) != BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_vs_add_build_fn failed, tspec in reassocreq\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif /* WLFBT */
	return cac;
fail:
	wlc_cac_detach(cac);
	return NULL;
}

void
BCMATTACHFN(wlc_cac_detach)(wlc_cac_t *cac)
{
	tsentry_t *ts;
#ifdef	AP
	struct scb *scb = NULL;
	struct scb_iter scbiter;
	struct cac_scb_acinfo *scb_acinfo = NULL;
#endif // endif

	if (!cac) {
		return;
	}

	if (cac->addts_timer) {
		wl_free_timer(cac->wlc->wl, cac->addts_timer);
		cac->addts_timer = NULL;
	}

#ifdef	AP
	/* free all TSPEC for AP */
	if (AP_ENAB(cac->wlc->pub)) {
		FOREACHSCB(cac->wlc->scbstate, &scbiter, scb) {
			if (SCB_WME(scb)) {
				scb_acinfo = SCB_ACINFO(cac, scb);
				while ((ts = scb_acinfo->tsentryq)) {
					scb_acinfo->tsentryq = ts->next;
					MFREE(cac->wlc->osh, ts, sizeof(tsentry_t));
				}
			}
		}
	}
#endif /* AP */

#ifdef	STA
	/* free all TSPEC for STA */
	while ((ts = cac->tsentryq)) {
		cac->tsentryq = ts->next;
		MFREE(cac->wlc->osh, ts, sizeof(tsentry_t));
	}
#endif // endif

	wlc_module_unregister(cac->wlc->pub, "cac", cac);
	MFREE(cac->wlc->osh, cac, sizeof(wlc_cac_t));
}

static int
wlc_cac_down(void *hdl)
{
	int callbacks = 0;
	wlc_cac_t *cac = (wlc_cac_t *)hdl;

	if (cac->addts_timer && !wl_del_timer(cac->wlc->wl, cac->addts_timer))
		callbacks = 1;

	return callbacks;
}

/* count number of accepted TSPEC in queue */
static INLINE int
wlc_cac_num_ts_accepted(wlc_cac_t *cac, tsentry_t *tsentryq)
{
	tsentry_t *ts;
	int n = 0;

	for (ts = tsentryq; ts; ts = ts->next) {
		if (ts->ts_state == CAC_STATE_TSPEC_ACCEPTED)
			n++;
	}
	return n;
}

static int
wlc_cac_get_tspec(wlc_cac_t *cac, tsentry_t *tsentryq, tspec_arg_t *tspec_arg, uint8 tid)
{
	tsentry_t *ts;

	if (!(ts = wlc_cac_tsentry_find(tsentryq, tid)))
		return BCME_NOTFOUND;

	tspec_arg->version = TSPEC_ARG_VERSION;
	tspec_arg->length = sizeof(tspec_arg_t) - (sizeof(uint16) * 2);
	tspec_arg->flag = ts->ts_state;

	/*
	* copy from packed structure to un-packed structure, need
	* to break up into two separate bcopy
	*/
	bcopy((uint8 *)&ts->tspec.tsinfo, (uint8 *)&tspec_arg->tsinfo,
		sizeof(tsinfo_t));
	bcopy((uint8 *)&ts->tspec.nom_msdu_size, (uint8 *)&tspec_arg->nom_msdu_size,
		TSPEC_ARG_LENGTH - sizeof(tsinfo_t));

	return BCME_OK;

}

/* copies all the tsinfo info into the destination tslist */
static int
wlc_cac_get_tsinfo_list(wlc_cac_t *cac, tsentry_t *ts, void *buff, uint32 bufflen)
{
	int i = 0;
	struct tslist *tslist = (struct tslist *)buff;
	int max_tsinfo = (bufflen - sizeof(int))/sizeof(tsinfo_t);
	int num_ts_accepted = wlc_cac_num_ts_accepted(cac, ts);

	if (max_tsinfo < num_ts_accepted)
		return BCME_BUFTOOSHORT;

	tslist->count = num_ts_accepted;
	for (i = 0; ts && i < tslist->count;
			ts = ts->next, i++) {
		bcopy((uint8 *)&ts->tspec.tsinfo, (uint8 *)&tslist->tsinfo[i],
			sizeof(struct tsinfo));
	}

	return BCME_OK;
}

static int
wlc_cac_iovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_cac_t *cac = (wlc_cac_t *)hdl;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *)a;
	wlc_bsscfg_t *bsscfg;

	/* update bsscfg pointer */
	bsscfg = wlc_bsscfg_find_by_wlcif(cac->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
		case IOV_GVAL(IOV_CAC):
			*ret_int_ptr = cac->wlc->pub->_cac;
			break;

		case IOV_SVAL(IOV_CAC):
			if (!WME_ENAB(cac->wlc->pub))
				return BCME_WME_NOT_ENABLED;

			cac->wlc->pub->_cac = (int_val != 0);	/* set CAC enable/disable */
			break;

		case IOV_GVAL(IOV_CAC_ADDTS_TIMEOUT):
			*ret_int_ptr = cac->addts_timeout;
			break;

		case IOV_SVAL(IOV_CAC_ADDTS_TIMEOUT):
			cac->addts_timeout = int_val;
			break;

		case IOV_SVAL(IOV_CAC_ADDTS):
			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			err = wlc_cac_sta_addts(cac, (tspec_arg_t*)a);
			break;

		case IOV_SVAL(IOV_CAC_DELTS):
			{
				struct scb *scb;
				if (!CAC_ENAB(cac->wlc->pub))
					return BCME_UNSUPPORTED;

				if (!(scb = wlc_scbfind(cac->wlc, bsscfg, &cac->curr_bssid)))
					return BCME_NOTFOUND;

				err = wlc_cac_sta_delts(cac, (tspec_arg_t*)a, scb, cac->tsentryq);
			}
			break;

		case IOV_GVAL(IOV_CAC_TSLIST):
		{
			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			if (!bsscfg->BSS)
				return BCME_ERROR;

			if (BSSCFG_STA(bsscfg))
				err = wlc_cac_get_tsinfo_list(cac, cac->tsentryq, a, alen);
			else
				err = BCME_UNSUPPORTED;

			break;
		}
#ifdef AP
		case IOV_GVAL(IOV_CAC_TSLIST_EA):
		{
			struct ether_addr *ea;
			struct scb *scb;
			struct cac_scb_acinfo *scb_acinfo;

			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			/* extract ea */
			ea = (struct ether_addr *)p;

			if (ETHER_ISMULTI(ea))
				return BCME_BADARG;

			if (!(scb = wlc_scbfind(cac->wlc, bsscfg, ea)))
				return BCME_NOTFOUND;

			scb_acinfo = SCB_ACINFO(cac, scb);
			err = wlc_cac_get_tsinfo_list(cac, scb_acinfo->tsentryq, a, alen);
		}
		break;

		case IOV_GVAL(IOV_CAC_TSPEC_EA):
		{
			struct ether_addr *ea;
			struct scb *scb;
			struct cac_scb_acinfo *scb_acinfo;
			tspec_per_sta_arg_t *tsea;
			tspec_arg_t *tspec_arg;

			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			/* extract ea and tspec */
			tsea = (tspec_per_sta_arg_t *)p;
			ea = &tsea->ea;
			tspec_arg = &tsea->ts;

			if (ETHER_ISMULTI(ea))
				return BCME_BADARG;

			if (!(scb = wlc_scbfind(cac->wlc, bsscfg, ea)))
				return BCME_NOTFOUND;

			scb_acinfo = SCB_ACINFO(cac, scb);

			err = wlc_cac_get_tspec(cac, scb_acinfo->tsentryq, (tspec_arg_t *)a,
				WLC_CAC_GET_TID(tspec_arg->tsinfo));

		}
		break;

		case IOV_GVAL(IOV_CAC_DELTS_EA):
		{
			struct ether_addr *ea;
			struct scb *scb;
			struct cac_scb_acinfo *scb_acinfo;
			tspec_per_sta_arg_t *tsea;
			tspec_arg_t tspec_arg;

			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			/* extract ea and tspec */
			tsea = (tspec_per_sta_arg_t *)p;
			ea = &tsea->ea;

			if (ETHER_ISMULTI(ea))
				return BCME_BADARG;

			if (!(scb = wlc_scbfind(cac->wlc, bsscfg, ea)))
				return BCME_NOTFOUND;

			bcopy((uint8*)&tsea->ts, (uint8*)&tspec_arg, sizeof(tspec_arg));
			scb_acinfo = SCB_ACINFO(cac, scb);
			err = wlc_cac_sta_delts(cac, &tspec_arg, scb, scb_acinfo->tsentryq);
		}
		break;

		case IOV_SVAL(IOV_CAC_AC_BW):
			if (AP_ENAB(cac->wlc->pub)) {
				if ((int_val < 0) || (int_val > 100))
					return BCME_BADARG;
				cac->admctl_bw_percent = (uint8)int_val;
				cac->available_medium_time = AVAILABLE_MEDIUM_TIME_32US -
					(AVAILABLE_MEDIUM_TIME_32US * cac->admctl_bw_percent)/100;
			} else {
				return BCME_UNSUPPORTED;
			}
			break;

		case IOV_GVAL(IOV_CAC_AC_BW):
			if (AP_ENAB(cac->wlc->pub))
				*ret_int_ptr = cac->admctl_bw_percent;
			else
				return BCME_UNSUPPORTED;
			break;
#endif /* AP */

		case IOV_GVAL(IOV_CAC_TSPEC):
		{
			tspec_arg_t *tspec_arg;

			if (!CAC_ENAB(cac->wlc->pub))
				return BCME_UNSUPPORTED;
			if (!bsscfg->BSS)
				return BCME_ERROR;

			if (BSSCFG_STA(bsscfg) && bsscfg->associated) {
				tspec_arg = (tspec_arg_t *)p;

				err = wlc_cac_get_tspec(cac, cac->tsentryq, (tspec_arg_t *)a,
					WLC_CAC_GET_TID(tspec_arg->tsinfo));

			} else {
				return BCME_NOTASSOCIATED;
			}

			break;
		}

		default:
			err = BCME_BADARG;
	}
	return err;
}

static tsentry_t *
wlc_cac_tsentry_find(tsentry_t *ts, uint8 tid)
{
	for (; ts; ts = ts->next) {
		if (WLC_CAC_GET_TID(ts->tspec.tsinfo) == tid)
			return ts;		/* found */
	}
	return NULL;
}

/* find first TSPEC in given state */
static tsentry_t *
wlc_cac_tspec_state_find(tsentry_t *ts, int state)
{
	for (; ts; ts = ts->next) {
		if (ts->ts_state == state)
			return ts;		/* found */
	}
	return NULL;
}

static void
wlc_cac_tsentry_append(wlc_cac_t *cac, tsentry_t *ts, tsentry_t **ptsentryq)
{
	tsentry_t **ts_ptr;

	ts->next = NULL;

	/* goto end of list */
	for (ts_ptr = ptsentryq; *ts_ptr != NULL; ts_ptr = &(*ts_ptr)->next);

	*ts_ptr = ts;
}

static int
wlc_cac_tsentry_removefree(wlc_cac_t *cac, uint8 tid, uint event_status, uint code,
	tsentry_t **ptsentryq, bool call_event)
{
	tsentry_t *ts;
	tsentry_t **prev;

	for (prev = ptsentryq; *prev != NULL; prev = &(*prev)->next)
		if (WLC_CAC_GET_TID((*prev)->tspec.tsinfo) == tid)
			break;

	if (!(ts = *prev))
		return BCME_NOTFOUND;

	/* try to remove and free a TSPEC that has outstanding ADDTS
	* request. Disregard this action.
	* This can happen when Application issue ADDTS just before DELTS
	* is received by the STA. Let the ADDTS state machine to deal with it.
	*/
	if (ts->ts_state == CAC_STATE_TSPEC_WAIT_RESP)
		return BCME_ERROR;

	*prev = ts->next;

#ifdef	AP
	/* restore the global available medium time for all non downlink traffic */
	if (AP_ENAB(cac->wlc->pub))
		cac->available_medium_time += ts->tspec.medium_time;
#endif // endif

	MFREE(cac->wlc->osh, ts, sizeof(tsentry_t));
	WL_CAC(("wl%d: %s: remove and free TSPEC tid %d"
	        " status %d code %d\n", cac->wlc->pub->unit, __FUNCTION__,
	        tid, event_status, code));
	if (call_event)
		wlc_cac_delts_ind_complete(cac->wlc, event_status,
		        &cac->wlc->cfg->target_bss->BSSID,
			code, tid);

	return 0;
}

/* recalculate medium time for each AC */
static void
wlc_cac_medium_time_recal(wlc_cac_t *cac, struct cac_scb_acinfo *scb_acinfo)
{
	uint8 ac;
	tsentry_t *ts;
	int i;

	for (i = 0; i < AC_COUNT; i++)
		scb_acinfo->cac_ac[i].tot_medium_time = 0;

	for (ts = cac->tsentryq; ts; ts = ts->next) {
		ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));
		scb_acinfo->cac_ac[ac].tot_medium_time =
			USEC32_TO_USEC(ltoh16_ua(&ts->tspec.medium_time));
	}
}

/* Find the IE in tlvs (pattern pointed by str with strlen) */
static uint8 *
wlc_cac_ie_find(uint8 *tlvs, uint *tlvs_len, uint8 id, uint max_len,
const 	int8 *str, uint str_len)
{
	while ((tlvs = (uint8 *)bcm_parse_tlvs(tlvs, *tlvs_len, id)) != NULL) {
		if (tlvs[TLV_LEN_OFF] >= max_len &&
			!bcmp(&tlvs[TLV_BODY_OFF], str, str_len))
			return tlvs;
		/* calculate the length of the rest of the buffer */
		*tlvs_len -= tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
		/* point to the next ie */
		tlvs += tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
	}

	return NULL;
}

/* Send a action frame (tspec) request containing single WME TSPEC element */
static int
wlc_cac_tspec_req_send(wlc_cac_t *cac, tspec_t *ts, uint8 action,
	uint8 dialog_token, int reason_code, void **pkt, struct scb *scb)
{
	uint len = 0;
	struct dot11_management_notification *mgmt_hdr;
	uint8 *body, *cp;
#ifdef BCMCCX
	uint8 tid, user_prio;
#endif // endif
	wlc_info_t *wlc = cac->wlc;
	struct ether_addr *da = (struct ether_addr *)&scb->ea;
	uint16 orig_medium_time;

	WL_TRACE(("wl%d: wlc_cac_tspec_req_send\n", wlc->pub->unit));

	*pkt = NULL;

	if (!(wlc->pub->associated))
		return BCME_NOTASSOCIATED;

	ASSERT(scb);

	if (!SCB_WME(scb)) {
		WL_CAC(("wl%d: wlc_cac_tspec_req_send: WME Not Enabled\n",
			wlc->pub->unit));
		return BCME_NOT_WME_ASSOCIATION;
	}

	/*
	 * Management Action frame
	 * -----------------------------------------------------------------------------------------
	 * | MAC header | Category code | Action code | Dialog token | Status code | Elements | FCS
	 * -----------------------------------------------------------------------------------------
	 *	24/30               1                 1             1            1                 4
	 *
	 * WMM Tspec element
	 * -----------------------------------------------------------------------------------------
	 * ID | length | OUI | OUI type | OUI subtype | version | Tspec body
	 * -----------------------------------------------------------------------------------------
	 * 1	1		3		1			1			1
	 *
	 * XXX this has to be stripped off XXX CCX Stream element
	 * -----------------------------------------------------------------------------------------
	 * ID | length | OUI | OUI type | TID | Rateset (nominal rate to use)
	 * -----------------------------------------------------------------------------------------
	 * 1	1		3		1		 1		[1]
	 */

	/* TSPEC Request frame is 4 bytes Management Notification frame
	 * follow by WME TSPEC element
	 */
	len = DOT11_MGMT_NOTIFICATION_LEN +
		(TLV_HDR_LEN + WME_TSPEC_LEN);
#ifdef BCMCCX
	if (CCX_ENAB(cac->wlc->pub))
		len += sizeof(ccx_stream_rs_t) + TLV_HDR_LEN;
#endif	/* BCMCCX */

	if ((*pkt = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &wlc->pub->cur_etheraddr,
		&wlc->cfg->BSSID, len, &body)) == NULL)
		return BCME_ERROR;

	len = 0;
	mgmt_hdr = (struct dot11_management_notification *)body;
	mgmt_hdr->category = DOT11_ACTION_NOTIFICATION;
	mgmt_hdr->action = action;

	/* token must be non-zero for ADDTS - set high bit */
	if (action == WME_ADDTS_REQUEST)
		mgmt_hdr->token = dialog_token;
	else
		mgmt_hdr->token = 0;

	mgmt_hdr->status = 0;	/* always zero for STA */
	len += DOT11_MGMT_NOTIFICATION_LEN;
	cp = mgmt_hdr->data;

	WL_CAC(("wl%d: %s: construct action frame %s\n", wlc->pub->unit,
		__FUNCTION__, (action == WME_ADDTS_REQUEST)?"ADDTS":"DELTS"));

#ifdef BCMCCX
	tid = WLC_CAC_GET_TID(ts->tsinfo);
	user_prio = WLC_CAC_GET_USER_PRIO(ts->tsinfo);
#endif // endif

	/* Fill in TSPEC values
	 * WMM Section 3.5.3 mandates that medium_time in an ADDTS TSPEC must be 0.
	 */
	orig_medium_time = ts->medium_time;	/* save */
	ts->medium_time = 0;
	cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, ts, WME_TSPEC_LEN, cp);
	len += (TLV_HDR_LEN + WME_TSPEC_LEN);
	ts->medium_time = orig_medium_time;	/* restore */

#ifdef BCMCCX
	/* Fill in Stream rate values, if nominal Phy rate != minimum Phy
	 * rate and TID == Signal TID
	 */
	if (CCX_ENAB(cac->wlc->pub) &&
		(tid != CCX_CAC_SIGNAL_TID) && (action == WME_ADDTS_REQUEST)) {

		uint8 ac = WME_PRIO2AC(user_prio);
		if (WLC_RATE_500K_TO_BPS(cac->ac_settings[ac].nom_phy_rate) !=
			ts->min_phy_rate) {
			ccx_stream_rs_t rs;
			bcopy(CISCO_AIRONET_OUI, rs.oui, DOT11_OUI_LEN);
			rs.oui_type = CCX_CAC_TS_RATESET_TYPE;
			rs.tid = tid;
			if (scb->flags & SCB_NONERP) {
				rs.rateset[0] = WLC_RATE_11M;
			} else {
				rs.rateset[0] = cac->ac_settings[ac].nom_phy_rate;
			}

			cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, &rs,
				sizeof(ccx_stream_rs_t), cp);

			len += TLV_HDR_LEN + sizeof(ccx_stream_rs_t);
			WL_CAC(("wl%d: wlc_cac_tspec_req_send: rateset IE TID %d,"
				" USER PRIORITY %d\n", wlc->pub->unit, tid, user_prio));
		}
	}
#endif	/* BCMCCX */

	ASSERT((cp - body) == (int)len);
	/* adjust the packet length */
	PKTSETLEN(wlc->osh, *pkt,
		(uint)(body - (uint8 *)PKTDATA(cac->wlc->osh, *pkt)) + len);

	if (wlc_sendmgmt(wlc, *pkt, SCB_WLCIFP(scb)->qi, NULL))
		return 0;

	return BCME_ERROR;
}

/* This function send ADDTS request */
static int
wlc_cac_addts_req_send(wlc_cac_t *cac, tspec_t *ts, uint8 dialog_token)
{
	int err;
	void *pkt;
	struct scb *scb;

#ifdef MBSS
	if (MBSS_ENAB(cac->wlc->pub)) {
		/* There's a BSS id in wlc_cac_t; maybe use that? */
		if (!(scb = wlc_scbfind(cac->wlc, cac->curr_bsscfg, &cac->curr_bssid)) &&
		    !(scb = wlc_scbfind(cac->wlc, cac->wlc->cfg, &cac->wlc->cfg->BSSID))) {
			return BCME_NOTFOUND;
		}
	} else
#endif /* MBSS */
	if (!(scb = wlc_scbfind(cac->wlc, cac->wlc->cfg, &cac->wlc->cfg->BSSID))) {
		return BCME_NOTFOUND;
	}

	if ((err = wlc_cac_tspec_req_send(cac, ts, WME_ADDTS_REQUEST, dialog_token, 0, &pkt, scb)))
		return err;

	return 0;
}

/* send next pending TSPEC in the tsentry queue */
static int
wlc_cac_tspec_send(wlc_cac_t *cac, tsentry_t *tsentryq)
{
	int err = 0;
	tsentry_t *ts;

	/* check if there is a TSPEC in progress */
	if (cac->waiting_resp)
		return 0;

	WL_CAC(("wl%d: %s: find next TSPEC pending queue\n",
		cac->wlc->pub->unit, __FUNCTION__));

	/* find next pending TSPEC */
	for (ts = tsentryq; ts; ts = ts->next) {
		if ((ts->ts_state == CAC_STATE_TSPEC_PENDING) ||
			(ts->ts_state == CAC_STATE_TSPEC_UPDATE_PENDING))
			break;
	}

	if (ts == NULL)
		return 0;

	if (!(err = wlc_cac_addts_req_send(cac, &ts->tspec, ts->dialog_token))) {
		uint32 timeout = cac->addts_timeout;
		wlc_bsscfg_t *cfg = cac->wlc->cfg;
		cac->waiting_resp = TRUE;
		if (cfg->pm->PMenabled)
			timeout += CAC_ADDTS_RESP_TIMEOUT_FF;
		if (ts->ts_state == CAC_STATE_TSPEC_UPDATE_PENDING)
			ts->ts_state = CAC_STATE_TSPEC_UPDATE_WAIT_RESP;
		else
			ts->ts_state = CAC_STATE_TSPEC_WAIT_RESP;

		WL_CAC(("wl%d: %s: start addts"
			" response timer\n", cac->wlc->pub->unit, __FUNCTION__));
		wl_add_timer(cac->wlc->wl, cac->addts_timer, timeout, 0);
	} else {
		uint8 tid = WLC_CAC_GET_TID(ts->tspec.tsinfo);

		WL_CAC(("wl%d: %s: tid %d Err %d\n",
			cac->wlc->pub->unit, __FUNCTION__, tid, err));
		ts->ts_state = CAC_STATE_TSPEC_REJECTED;
		wlc_cac_tsentry_removefree(cac, tid, WLC_E_STATUS_UNSOLICITED,
			DOT11E_STATUS_UNKNOWN_TS, &cac->tsentryq, TRUE);
	}
	return err;
}

/* IOVAR ADDTS. Construct TSPEC from tspec_arg
 * append the TSPEC to tsentry queue and
 * transmit action frame
 */
static int
wlc_cac_sta_addts(wlc_cac_t *cac, tspec_arg_t *tspec_arg)
{
	tspec_t *tspec;
	tsentry_t *ts;
	int err = 0;
	uint8 tid;
	wlc_bsscfg_t *cfg = cac->wlc->cfg;

	if (tspec_arg->version != TSPEC_ARG_VERSION)
		return BCME_BADARG;

	if (!tspec_arg->dialog_token)
		return BCME_BADARG;

	if (tspec_arg->length < sizeof(tspec_arg_t) - (sizeof(uint16) * 2))
		return BCME_BADLEN;

#ifdef	BCMCCX
	/* uni-direction is not allowed if CCX version is less than 5 */
	if ((CCX_ENAB(cac->wlc->pub)) && (cfg->current_bss->ccx_version < 5) &&
	    (WLC_CAC_GET_DIR(tspec_arg->tsinfo) !=
	     (TS_INFO_BIDIRECTIONAL >> TS_INFO_DIRECTION_SHIFT)))
		return BCME_BADARG;
#endif	/* BCMCCX */

#if defined(BCMCCX) && defined(CCX_SDK)
	/* check if still in delay time from last addts request */
	if ((CCX_ENAB(cac->wlc->pub)) &&
	    wlc_ccx_in_addts_delay(cac->wlc->ccx, &cac->curr_bssid))
		return BCME_NOTREADY;
#endif // endif

	/* check if we support APSD */
	if (WLC_CAC_GET_PSB(tspec_arg->tsinfo) && !cfg->wme->wme_apsd) {
		WL_CAC(("No support for APSD\n"));
		return BCME_BADARG;
	}

	tid = WLC_CAC_GET_TID(tspec_arg->tsinfo);
	if ((ts = wlc_cac_tsentry_find(cac->tsentryq, tid))) {
		/* check if target TSPEC is currently waiting for resp or roaming */
		if (ts->ts_state == CAC_STATE_TSPEC_WAIT_RESP)
			return BCME_BUSY;

		if (ts->ts_state == CAC_STATE_TSPEC_ACCEPTED) {
			/* TSPEC is already accepted and this request
			 * is an update to the existing one
			 */
			cac->cached_medium_time = (uint)ts->tspec.medium_time;
			tspec = &ts->tspec;
			WL_CAC(("Update the existing tspec : Previous medium time ix %d\n",
				cac->cached_medium_time));
		} else {
			return BCME_ERROR;
		}

	} else {
		/* not found, malloc and construct TSPEC frame */
		if (!(ts = (tsentry_t *)MALLOC(cac->wlc->osh, sizeof(tsentry_t)))) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				cac->wlc->pub->unit, __FUNCTION__, MALLOCED(cac->wlc->osh)));
			return BCME_NORESOURCE;
		}
		bzero((char *)ts, sizeof(tsentry_t));

		tspec = &ts->tspec;
		bcopy(WME_OUI, tspec->oui, WME_OUI_LEN);
		tspec->type = WME_OUI_TYPE;
		tspec->subtype = WME_SUBTYPE_TSPEC;
		tspec->version = WME_VER;

		wlc_cac_tsentry_append(cac, ts, &cac->tsentryq);
	}

	/* convert input to 802.11 little-endian order */
	tspec_arg->nom_msdu_size = htol16(tspec_arg->nom_msdu_size);
	tspec_arg->max_msdu_size = htol16(tspec_arg->max_msdu_size);
	tspec_arg->min_srv_interval = htol32(tspec_arg->min_srv_interval);
	tspec_arg->max_srv_interval = htol32(tspec_arg->max_srv_interval);
	tspec_arg->inactivity_interval = htol32(tspec_arg->inactivity_interval);
	tspec_arg->suspension_interval = htol32(tspec_arg->suspension_interval);
	tspec_arg->srv_start_time = htol32(tspec_arg->srv_start_time);
	tspec_arg->min_data_rate = htol32(tspec_arg->min_data_rate);
	tspec_arg->mean_data_rate = htol32(tspec_arg->mean_data_rate);
	tspec_arg->peak_data_rate = htol32(tspec_arg->peak_data_rate);
	tspec_arg->max_burst_size = htol32(tspec_arg->max_burst_size);
	tspec_arg->delay_bound = htol32(tspec_arg->delay_bound);
	tspec_arg->min_phy_rate = htol32(tspec_arg->min_phy_rate);
	tspec_arg->surplus_bw = htol16(tspec_arg->surplus_bw);

	/* XXX require to do 2 bcopy due to tspec_arg_t is aligned
	 * structure and tspec_t is a unaligned structure.
	 * first copy tsinfo octets then tspec arg
	 */
	bcopy(tspec_arg->tsinfo.octets, tspec->tsinfo.octets, sizeof(tsinfo_t));
	bcopy((uint8 *)&tspec_arg->nom_msdu_size, (uint8 *)&tspec->nom_msdu_size,
		WLC_CAC_TSPECARG_COPYSIZE);

	bcopy(&tspec_arg->min_phy_rate, &tspec->min_phy_rate, sizeof(uint));
	bcopy(&tspec_arg->surplus_bw, &tspec->surplus_bw, sizeof(uint16));

	ts->dialog_token = tspec_arg->dialog_token;

	WL_CAC(("wl%d: wlc_cac_sta_addts: IOVAR ADDTS tid %d,\n",
		cac->wlc->pub->unit, tid));
	if (ts->ts_state == CAC_STATE_TSPEC_ACCEPTED)
		ts->ts_state = CAC_STATE_TSPEC_UPDATE_PENDING;
	else
		ts->ts_state = CAC_STATE_TSPEC_PENDING;

	err = wlc_cac_tspec_send(cac, cac->tsentryq);

	return err;
}

/* IOVAR DELTS. Send TSPEC action frame DELTS and remove & free the TSPEC
 * from ts entry queue regardless if the send is successful.
 */
static int
wlc_cac_sta_delts(wlc_cac_t *cac, tspec_arg_t *tspec_arg, struct scb *scb,
	tsentry_t *tsentryq)
{
	tsentry_t *ts;
	uint8 tid, ac;
	void *pkt;
	struct cac_scb_acinfo *scb_acinfo;
	tsentry_t **ptsentryq = NULL;

	ASSERT(scb);

	scb_acinfo = SCB_ACINFO(cac, scb);

	if (tspec_arg->version != TSPEC_ARG_VERSION)
		return BCME_BADARG;

	if (tspec_arg->length < sizeof(tspec_arg_t) - (sizeof(uint16) * 2))
		return BCME_BADLEN;

	tid = WLC_CAC_GET_TID(tspec_arg->tsinfo);
	ts = wlc_cac_tsentry_find(tsentryq, tid);

	if (!ts)
		return BCME_NOTFOUND;

	if (ts->ts_state == CAC_STATE_TSPEC_WAIT_RESP)
		return BCME_BUSY;

	/* direction setting should match */
	if (WLC_CAC_GET_DIR(tspec_arg->tsinfo) !=
		WLC_CAC_GET_DIR(ts->tspec.tsinfo))
		return BCME_BADARG;

	wlc_cac_tspec_req_send(cac, &ts->tspec, WME_DELTS_REQUEST,
		ts->dialog_token, DOT11E_STATUS_QSTA_LEAVE_QBSS, &pkt, scb);

	/* get the ac before free the TSPEC */
	ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));

	/* removing a TSPEC that is not waiting on response, therefore no
	 * need to reset waiting_resp or restart TSPEC send.
	 */

	ASSERT(scb->bsscfg);
	if (!BSSCFG_AP(scb->bsscfg)) {
		ptsentryq = &cac->tsentryq;
	}
#ifdef AP
	else {
		ptsentryq = &scb_acinfo->tsentryq;
	}
#endif // endif

	if (ptsentryq) {
		wlc_cac_tsentry_removefree(cac, tid, WLC_E_STATUS_SUCCESS,
			DOT11E_STATUS_QSTA_LEAVE_QBSS, ptsentryq, TRUE);
	}

	if (CAC_ENAB(cac->wlc->pub)) {
		/* Reset the power save to default settings */
		if (scb->apsd.ac_defl & (1 << ac)) {
			scb->apsd.ac_delv |= (1 << ac);
			scb->apsd.ac_trig |= (1 << ac);
			WL_CAC(("Restoring the original APSD settings\n"));
		} else {
			scb->apsd.ac_delv &= ~(1 << ac);
			scb->apsd.ac_trig &= ~(1 << ac);
			WL_CAC(("Restoring the original Legacy PS settings\n"));
		}

		scb->flags &= ~SCB_APSDCAP;
		if (scb->apsd.ac_trig & AC_BITMAP_ALL)
			scb->flags |= SCB_APSDCAP;
	}

	/* reset the admitted flag */
	scb_acinfo->cac_ac[ac].admitted = FALSE;
	wlc_cac_ac_param_reset(cac, ac, scb_acinfo);

	return 0;
}

/* parse TSPEC and update medium time received tspec */
static uint8 *
wlc_cac_tspec_rx_process(wlc_cac_t *cac, uint8 *body, uint *body_len,
	uint8 *tid, uint8 *user_prio, struct cac_scb_acinfo *scb_acinfo, int *err)
{
	uint8 ac;
	tsentry_t *ts;
	tspec_t *tspec;

	WL_CAC(("%s: Entering\n", __FUNCTION__));

	if (*body_len < WME_TSPEC_LEN) {
		*err = BCME_ERROR;
		return body;
	}

	*err = 0;

	tspec = (tspec_t *)&body[TLV_BODY_OFF];		/* skip over ID & len field */

	*tid = WLC_CAC_GET_TID(tspec->tsinfo);
	*user_prio = WLC_CAC_GET_USER_PRIO(tspec->tsinfo);

	ac = WME_PRIO2AC(*user_prio);
	BCM_REFERENCE(ac);

	ts = wlc_cac_tsentry_find(cac->tsentryq, *tid);

#ifdef	BCMDBG
	if (ts && (ts->ts_state == CAC_STATE_TSPEC_UPDATE_WAIT_RESP))
		WL_CAC(("Previous alloted medium time is %d\n",
			cac->cached_medium_time));
#endif	/* BCMDBG */

	/* need to filter receiving duplicated response */
	if (ts && (ts->ts_state != CAC_STATE_TSPEC_ACCEPTED))
		ts->ts_state = CAC_STATE_TSPEC_ACCEPTED;

	if (ts != NULL)
		bcopy(tspec, &ts->tspec, WME_TSPEC_LEN);

	/* recal total medium time */
	wlc_cac_medium_time_recal(cac, scb_acinfo);

	WL_CAC(("wl%d: wlc_cac_tspec_rx_process: add TSPEC to AC %d queue,"
		" tid %d, user priority %d, total medium time %d (microseconds)\n",
		cac->wlc->pub->unit, ac, *tid, *user_prio, scb_acinfo->cac_ac[ac].tot_medium_time));

	/* update the length of the buffer */
	*body_len -= body[TLV_LEN_OFF] + TLV_HDR_LEN;
		/* point to the next ie */
	body += body[TLV_LEN_OFF] + TLV_HDR_LEN;

	return body;
}

/* handle TSPEC IE from ADDTS response and (re)assoc response
 * parse TSPEC IE and other IEs. Return error code
 */
static int
wlc_cac_ie_process(wlc_cac_t *cac, uint8 *tlvs, uint tlvs_len, uint8 *tid,
	uint8 *user_prio, struct scb *scb)
{
	int err;
	struct cac_scb_acinfo *scb_acinfo;

	scb_acinfo = SCB_ACINFO(cac, scb);
	ASSERT(scb_acinfo);

	while (tlvs_len > 0 && tlvs) {
		err = 0;
		tlvs = wlc_cac_ie_find(tlvs, &tlvs_len, DOT11_MNG_PROPR_ID, WME_TSPEC_LEN,
		(const int8 *)DOT11E_TSPEC_IE, DOT11E_TSPEC_OUI_TYPE_LEN);
		/* process tspec + all other IEs */
		if (tlvs) {
			uint8 ac;
			tlvs = wlc_cac_tspec_rx_process(cac, tlvs, &tlvs_len, tid,
				user_prio, scb_acinfo, &err);
			if (err)
				return err;

			ac = WME_PRIO2AC(*user_prio);

#ifdef BCMCCX
			if (CCX_ENAB(cac->wlc->pub))
			{
				/* process any CCX IEs that is included in a tspec */
				tlvs = wlc_cac_ccx_ie_process(cac, tlvs, &tlvs_len, ac, &err);
				if (err)
					return err;
			}
#endif	/* BCMCCX */

			AC_BITMAP_SET(cac->flags, ac);

			/* XXX NO MAC indication required for each TSPEC, caller
			 * require to handle the MAC event and restart of TSPEC send.
			 */
		}
	}

	return 0;
}

#ifdef BCMCCX
/* Process rateset give by AP and find suggested nominal rate */
static bool
wlc_ccx_ts_rateset_ie(wlc_cac_t *cac, uint8 *ie, int ac)
{
	wlc_info_t *wlc = cac->wlc;
	uint8 len = ie[TLV_LEN_OFF];
	ccx_stream_rs_t *rs = (ccx_stream_rs_t *)&ie[TLV_BODY_OFF];
	uint8 i;
	struct scb *scb;
	struct cac_scb_acinfo *scb_acinfo;
	wlc_rateset_t rateset;

	if (len < sizeof(ccx_stream_rs_t))
		return FALSE;

	if (!(scb = wlc_scbfind(wlc, wlc->cfg, &wlc->cfg->BSSID)))
		return FALSE;

	scb_acinfo = SCB_ACINFO(cac, scb);

	bzero((uint8 *)&rateset, sizeof(wlc_rateset_t));
	rateset.count = MIN((len - CAC_STREAM_RS_HDR_LEN), WLC_NUMRATES);
	bcopy(rs->rateset, rateset.rates, rateset.count);
	wlc_rateset_filter(&rateset /* src */, &scb->rateset /* dst */, FALSE, WLC_RATES_CCK_OFDM,
	                   RATE_MASK, wlc_get_mcsallow(wlc, NULL));
	/* filter-out unsupported rates */
	wlc_rate_hwrs_filter_sort_validate(&rateset /* [in+out] */,
		&wlc->band->hw_rateset /* [in] */, FALSE, wlc->stf->op_txstreams);

	for (i = 0; i < (len - CAC_STREAM_RS_HDR_LEN); i++) {
		if (rs->rateset[i] & WLC_RATE_FLAG)
			scb_acinfo->cac_ac[ac].nom_phy_rate = rs->rateset[i] & RATE_MASK;
	}

	return TRUE;
}

/* check if AP provided a set of nominal rate. If so,
 * find & save the highest nominal Phy rate recommended by the AP
 */
static int
wlc_ccx_nom_phyrate(wlc_cac_t *cac, uint8 *tlvs, uint tlvs_len)
{
	struct scb *scb;
	struct cac_scb_acinfo *scb_acinfo;
	ccx_stream_rs_t *rs;
	uint8 len, ac, i, highest;
	tspec_t *tspec;

	scb = wlc_scbfind(cac->wlc, cac->curr_bsscfg, &cac->curr_bssid);
	ASSERT(scb);
	scb_acinfo = SCB_ACINFO(cac, scb);
	ASSERT(scb_acinfo);

	/* find the TSPEC and get the user priority */
	tlvs = wlc_cac_ie_find(tlvs, &tlvs_len, DOT11_MNG_PROPR_ID, WME_TSPEC_LEN,
		(const int8*)	DOT11E_TSPEC_IE, DOT11E_TSPEC_OUI_TYPE_LEN);
	if (!tlvs)
		return BCME_NOTFOUND;	/* tspec not found, exit */

	tspec = (tspec_t *)tlvs;
	ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(tspec->tsinfo));

	/* find the rateset IE */
	tlvs = wlc_cac_ie_find(tlvs, &tlvs_len, DOT11_MNG_PROPR_ID, sizeof(ccx_stream_rs_t),
	(const int8*)CCX_CAC_TS_RATESET_IE, CCX_CAC_OUI_TYPE_LEN);
	if (!tlvs)
		return BCME_NOTFOUND;	/* rateset not found */

	len = tlvs[TLV_LEN_OFF];
	if (len < sizeof(ccx_stream_rs_t))
		return BCME_ERROR;

	rs = (ccx_stream_rs_t *)&tlvs[TLV_BODY_OFF];

	/* search the list of AP recommended rate.
	 * Find the highest nominal rate suggested by AP.
	 */
	highest = 0;
	for (i = 0; i < (len - CAC_STREAM_RS_HDR_LEN); i++) {
		if (rs->rateset[i] > highest)
			highest = rs->rateset[i];
	}
	/* valid rate before save it
	 * XXX rate may not be valid for all band
	 */
	scb_acinfo->cac_ac[ac].nom_phy_rate = highest & WLC_RATE_FLAG;

	return 0;
}

/* Parse for CCX IE and update CAC param per AC */
static uint8 *
wlc_cac_ccx_ie_process(wlc_cac_t *cac, uint8 *tlvs, uint *tlvs_len, uint8 ac, int *err)
{

	struct scb *scb;
	struct cac_scb_acinfo *scb_acinfo;

	scb = wlc_scbfind(cac->wlc, cac->curr_bsscfg, &cac->curr_bssid);
	ASSERT(scb);
	scb_acinfo = SCB_ACINFO(cac, scb);
	ASSERT(scb_acinfo);

	/* CCX TSPEC may contain one or more IE */

	/* TS Rate IE */
	if ((*tlvs_len > (uint)TLV_HDR_LEN) &&
		(*tlvs_len >= (uint)(tlvs[TLV_LEN_OFF] + TLV_HDR_LEN)) &&
		(tlvs[TLV_LEN_OFF] >= CCX_CAC_OUI_TYPE_LEN) &&
		!bcmp(&tlvs[TLV_BODY_OFF], CCX_CAC_TS_RATESET_IE, CCX_CAC_OUI_TYPE_LEN)) {
		if (!wlc_ccx_ts_rateset_ie(cac, tlvs, ac))
			*err = BCME_ERROR;
		WL_CAC(("wl%d: wlc_cac_ccx_ie_process: update Rateset"
			" to AC %d, nominal rate 0x%02x\n",
			cac->wlc->pub->unit, ac, scb_acinfo->cac_ac[ac].nom_phy_rate));
		/* calculate the length of the rest of the buffer */
		*tlvs_len -= tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
		/* point to the next ie */
		tlvs += tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
	}

	/* MSDU Lifetime IE */
	if ((*tlvs_len > (uint)TLV_HDR_LEN) &&
		(*tlvs_len >= (uint)(tlvs[TLV_LEN_OFF] + TLV_HDR_LEN)) &&
		(tlvs[TLV_LEN_OFF] >= sizeof(ccx_msdu_lifetime_t)) &&
		!bcmp(&tlvs[TLV_BODY_OFF], CCX_CAC_MSDU_LIFETIME_IE, CCX_CAC_OUI_TYPE_LEN)) {
		wl_lifetime_t lifetm;
		ccx_msdu_lifetime_t *lifetime = (ccx_msdu_lifetime_t *)&tlvs[TLV_BODY_OFF];
		scb_acinfo->cac_ac[ac].ccx_tsm.msdu_lifetime = ltoh16_ua(&lifetime->msdu_lifetime);
		/* save current RF awareness before turn it off */
		if (!(cac->flags & AC_BITMAP_ALL))
			wlc_iovar_getint(cac->wlc, "rfaware_lifetime", &cac->rfaware_lifetime);
		/* set lifetime. this turns off RF awareness too */
		lifetm.ac = (uint32)ac;
		lifetm.lifetime = (scb_acinfo->cac_ac[ac].ccx_tsm.msdu_lifetime << 10) / 1000;
		if (wlc_iovar_op(cac->wlc, "lifetime", NULL, 0,
			&lifetm, sizeof(lifetm), IOV_SET, NULL)) {
			WL_ERROR(("wl%d: %s: setting lifetime failed\n",
				cac->wlc->pub->unit, __FUNCTION__));
		}
		WL_CAC(("wl%d: wlc_cac_ccx_ie_process: update MSDU Lifetime for AC %d,"
		        " MSDU Lifetime %d TU\n", cac->wlc->pub->unit, ac,
		        lifetime->msdu_lifetime));
		/* calculate the length of the rest of the buffer */
		*tlvs_len -= tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
		/* point to the next ie */
		tlvs += tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
	}

	/* TS Metrics IE */
	if ((*tlvs_len > (uint)TLV_HDR_LEN) &&
		(*tlvs_len >= (uint)(tlvs[TLV_LEN_OFF] + TLV_HDR_LEN)) &&
		(tlvs[TLV_LEN_OFF] >= sizeof(ccx_ts_metrics_t)) &&
		!bcmp(&tlvs[TLV_BODY_OFF], CCX_CAC_TS_METRICS_IE, CCX_CAC_OUI_TYPE_LEN)) {
		ccx_ts_metrics_t *metrics = (ccx_ts_metrics_t *)&tlvs[TLV_BODY_OFF];
		scb_acinfo->cac_ac[ac].ccx_tsm.ts_metrics = (metrics->state != 0);
		scb_acinfo->cac_ac[ac].ccx_tsm.tid = metrics->tid;
		if (metrics->measure_interval < CCX_TSMETRIC_INTERVAL_MIN ||
			metrics->measure_interval > CCX_TSMETRIC_INTERVAL_MAX)
			metrics->measure_interval = CCX_TSMETRIC_INTERVAL_DEFAULT;
		scb_acinfo->cac_ac[ac].ccx_tsm.ts_metrics_interval =
			ltoh16_ua(&metrics->measure_interval);
		WL_CAC(("wl%d: wlc_cac_ccx_ie_process: update Traffic Stream Metrics"
			" for AC %d, State %d, Interval %d\n",
			cac->wlc->pub->unit, ac, metrics->state, metrics->measure_interval));
		/* calculate the length of the rest of the buffer */
		*tlvs_len -= tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
		/* point to the next ie */
		tlvs += tlvs[TLV_LEN_OFF] + TLV_HDR_LEN;
	} else {
		/* if there isn't a traffic stream metrics IE after TSPEC,
		 * then consider ts_metrics in not active
		 */
		wlc_cac_ac_param_reset(cac, ac, scb_acinfo);
	}
	return tlvs;
}
#endif	/* BCMCCX */

/* processes ADDTS response frame from the AP */
static void
wlc_cac_addts_resp(wlc_cac_t *cac, uint8 *body, uint body_len, struct scb *scb)
{
#ifdef	BCMCCX
	bcm_tlv_t *ie;
#endif // endif
	uint8 ts_status, status;
	tsentry_t *ts;
	uint8 user_prio = 0;
	uint8 tid = -1;
	uint ts_delay = 0;
	struct cac_scb_acinfo *scb_acinfo = SCB_ACINFO(cac, scb);

	if (body_len < (WME_TSPEC_LEN + DOT11_MGMT_NOTIFICATION_LEN))
		return;

	/* If reject due to invalid param, check and see if is related to
	 * the nominal phy rate not supported by the AP
	 */
	ts_status = body[WME_STATUS_CODE_OFFSET];
	body += DOT11_MGMT_NOTIFICATION_LEN;
	body_len -= DOT11_MGMT_NOTIFICATION_LEN;

	/* no TSPEC waiting for ADDTS response */
	if ((ts = wlc_cac_tspec_state_find(cac->tsentryq,
		CAC_STATE_TSPEC_WAIT_RESP)) == NULL) {

		/* no TSPEC waiting for update */
		if ((ts = wlc_cac_tspec_state_find(cac->tsentryq,
			CAC_STATE_TSPEC_UPDATE_WAIT_RESP)) == NULL) {
			return;
		}
	}

	/* handle specific error */
	if (ts_status == DOT11E_STATUS_ADDTS_INVALID_PARAM) {
		WL_CAC(("wl%d: wlc_cac_addts_resp: CAC_ADDTS_INVALID_PARAM"
			" status %d\n", cac->wlc->pub->unit, ts_status));
#ifdef BCMCCX
		/* check if is rejected due to Phy rate */
		if (CCX_ENAB(cac->wlc->pub))
			wlc_ccx_nom_phyrate(cac, body, body_len);
#endif	/* BCMCCX */
	}

#ifdef BCMCCX
	if (CCX_ENAB(cac->wlc->pub)) {
		/* force roam when TSPEC gets rejected is required by CCX test spec. */
		if (ts_status == DOT11E_STATUS_ADDTS_REFUSED_NSBW) {
			WL_CAC(("wl%d: wlc_cac_addts_resp: DOT11E_STATUS_ADDTS_REFUSED_NSBW"
				" status %d\n", cac->wlc->pub->unit, ts_status));
			wlc_ccx_roam(cac->wlc->ccx, WLC_E_REASON_TSPEC_REJECTED);
		}

		ie = bcm_parse_tlvs(body, body_len, DOT11_MNG_TS_DELAY_ID);

		/* CCX have TS_DELAY IE before TSPEC IE */
		if (ie && ie->len == TS_DELAY_FIELD_SIZE) {
			cac->ts_delay = ltoh32_ua(ie->data);	/* update most reason */
			if (ts_status == DOT11E_STATUS_ADDTS_REFUSED_AWHILE) {
				ts_delay = (cac->ts_delay * 1024) / 1000000;
				if ((cac->ts_delay * 1024) % 1000000)
					ts_delay++;
			}
			/* point to the next ie */
			body += TLV_HDR_LEN + ie->len;
			body_len -= (TLV_HDR_LEN + ie->len);
			WL_CAC(("wl%d: wlc_cac_addts_resp: parse TS_DELAY_TS %d\n",
				cac->wlc->pub->unit, cac->ts_delay));
		}
	}
#endif	/* BCMCCX */

	/* handle all errors */
	if (ts_status != DOT11E_STATUS_ADMISSION_ACCEPTED) {
		status = WLC_E_STATUS_FAIL;
		goto event;
	}

	WL_CAC(("wl%d: wlc_cac_addts_resp: CAC_ADMISSION_ACCEPTED"
		" status %d\n", cac->wlc->pub->unit, ts_status));

	/* TSPEC admission accepted */
	if (wlc_cac_ie_process(cac, body, body_len, &tid, &user_prio, scb) == 0) {
		/* cancel timer only if we received the TSPEC with matching TID */
		if (tid == WLC_CAC_GET_TID(ts->tspec.tsinfo)) {
			wl_del_timer(cac->wlc->wl, cac->addts_timer);
			WL_CAC(("wl%d: wlc_cac_addts_resp: Kill timer tid %d"
				" Admission Accepted\n", cac->wlc->pub->unit, tid));
			cac->waiting_resp = FALSE;
			scb_acinfo->cac_ac[WME_PRIO2AC(user_prio)].admitted = TRUE;

			/* update the power save behavior */
			if (CAC_ENAB(cac->wlc->pub)) {
				int ac;
				wlc_bsscfg_t *cfg;
				cfg = SCB_BSSCFG(scb);
				ASSERT(cfg != NULL);
				if (WLC_CAC_GET_PSB(ts->tspec.tsinfo) && cfg->wme->wme_apsd) {
					ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));
					switch (WLC_CAC_GET_DIR(ts->tspec.tsinfo)) {
						case (TS_INFO_UPLINK >> TS_INFO_DIRECTION_SHIFT):
							AC_BITMAP_SET(scb->apsd.ac_trig, ac);
							AC_BITMAP_RESET(scb->apsd.ac_delv, ac);
							WL_CAC(("AC[%d] : Trigger enabled\n", ac));
							break;

						case (TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT):
							AC_BITMAP_SET(scb->apsd.ac_delv, ac);
							AC_BITMAP_RESET(scb->apsd.ac_trig, ac);
							WL_CAC(("AC[%d] : Delivery enabled\n", ac));
							break;

						case (TS_INFO_BIDIRECTIONAL >>
							TS_INFO_DIRECTION_SHIFT):
							AC_BITMAP_SET(scb->apsd.ac_trig, ac);
							AC_BITMAP_SET(scb->apsd.ac_delv, ac);
							WL_CAC(("AC[%d] : Trig & Delv enabled\n",
								ac));
							break;
						}
				} else {
					ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));
					AC_BITMAP_RESET(scb->apsd.ac_delv, ac);
					AC_BITMAP_RESET(scb->apsd.ac_trig, ac);
					WL_CAC(("AC [%d] : Legacy Power save\n", ac));
				}
			}
		}

		status = WLC_E_STATUS_SUCCESS;
	} else {
		status = WLC_E_STATUS_FAIL;
	}

event:
	wlc_cac_addts_ind_complete(cac->wlc, status, &cac->wlc->cfg->target_bss->BSSID,
		ts_status, tid, ts_delay);

	/* if status != WLC_E_STATUS_SUCCESS need to check if this response is for
	 * outstanding TSPEC that are waiting for response
	 */
	if (status != WLC_E_STATUS_SUCCESS) {
		body = wlc_cac_ie_find(body, &body_len, DOT11_MNG_PROPR_ID, WME_TSPEC_LEN,
		(const int8 *)DOT11E_TSPEC_IE, DOT11E_TSPEC_OUI_TYPE_LEN);
		if (body) {
			tspec_t *tspec = (tspec_t *)&body[TLV_BODY_OFF];
			tid = WLC_CAC_GET_TID(tspec->tsinfo);
			/* cancel timer only if we received the TSPEC with matching TID */
			if (tid == WLC_CAC_GET_TID(ts->tspec.tsinfo)) {
				wl_del_timer(cac->wlc->wl, cac->addts_timer);
				WL_CAC(("wl%d: wlc_cac_addts_resp: Kill timer tid %d"
					" Admission Failed\n", cac->wlc->pub->unit, tid));

				if (ts->ts_state != CAC_STATE_TSPEC_UPDATE_WAIT_RESP) {
					/* change ts_state so that removefree
					 * can get rid of it
					 */
					ts->ts_state = CAC_STATE_TSPEC_REJECTED;
					wlc_cac_tsentry_removefree(cac, tid,
						WLC_E_STATUS_UNSOLICITED, ts_status,
						&cac->tsentryq, TRUE);
					cac->waiting_resp = FALSE;
				}
#ifndef	BCMCCX
				else {
					WL_CAC(("Restore the previous tspec settings\n"));
					ASSERT(cac->cached_medium_time);
					scb_acinfo->cac_ac[WME_PRIO2AC(user_prio)].tot_medium_time =
						cac->cached_medium_time;
					ts->tspec.medium_time = (uint16)cac->cached_medium_time;
					ts->ts_state = CAC_STATE_TSPEC_ACCEPTED;
					cac->cached_medium_time = 0;
				}
#endif	/* BCMCCX */
			}
		}
		WL_CAC(("wl%d: wlc_cac_addts_resp: addts admission not accepted"
			" tid %d\n", cac->wlc->pub->unit, tid));
	}

	/* send next tspec pending */
	wlc_cac_tspec_send(cac, cac->tsentryq);
}

/* On DELTS request, delete the TSPEC */
static void
wlc_cac_delts_req(wlc_cac_t *cac, uint8 *body, uint body_len, struct scb *scb)
{
	struct dot11_management_notification* mgmt_hdr;
	struct cac_scb_acinfo *scb_acinfo;
	tspec_t *tspec;
	tsentry_t **ptsentryq = NULL;
	uint8 tid, ac;

	scb_acinfo = SCB_ACINFO(cac, scb);

	if (body_len < WME_TSPEC_LEN) {
		WLCNTINCR(cac->wlc->pub->_cnt->rxbadcm);
		return;
	}

	WL_CAC(("wl%d: %s: receive DELTS\n", cac->wlc->pub->unit, __FUNCTION__));

	mgmt_hdr = (struct dot11_management_notification *)body;
	tspec = (tspec_t *)&mgmt_hdr->data[TLV_BODY_OFF];	/* skip ID & len */
	tid = WLC_CAC_GET_TID(tspec->tsinfo);
	ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(tspec->tsinfo));

	wlc_cac_ac_param_reset(cac, ac, scb_acinfo);

	/* Reset the power save to default settings */
	if (scb->apsd.ac_defl & (1 << ac)) {
		scb->apsd.ac_delv |= (1 << ac);
		scb->apsd.ac_trig |= (1 << ac);
	} else {
		scb->apsd.ac_delv &= ~(1 << ac);
		scb->apsd.ac_trig &= ~(1 << ac);
	}

	scb->flags &= ~SCB_APSDCAP;
	if (scb->apsd.ac_trig & AC_BITMAP_ALL)
		scb->flags |= SCB_APSDCAP;

	ASSERT(scb->bsscfg);
	if (!BSSCFG_AP(scb->bsscfg)) {
		ptsentryq = &cac->tsentryq;
	}
#ifdef	AP
	else {
		ptsentryq = &scb_acinfo->tsentryq;
	}
#endif // endif

	/* not found or try to remove TSPEC in progress, exit */
	if (wlc_cac_tsentry_removefree(cac, tid, WLC_E_STATUS_UNSOLICITED,
		DOT11E_STATUS_END_TS, ptsentryq, TRUE))
		return;

	wlc_cac_medium_time_recal(cac, scb_acinfo);
}

/* addts response timeout handler. This function shall send a DELTS to
 * the AP to clean-up the TSPEC. This may happen when AP received the
 * ADDTS successfully, but the ACK is lost
 */
static void
wlc_cac_addts_timeout(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	uint err;
	uint8 tid;
	tsentry_t *ts;
	void *pkt;
	struct scb *scb;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	WL_CAC(("wl%d: %s: send DELTS\n", wlc->pub->unit, __FUNCTION__));

	if (!(scb = wlc_scbfind(wlc, wlc->cac->curr_bsscfg, &wlc->cac->curr_bssid)))
		return;

	/* ADDTS timeout triggered; get the TSPEC waiting for response */
	ts = wlc_cac_tspec_state_find(wlc->cac->tsentryq, CAC_STATE_TSPEC_WAIT_RESP);
	if (!ts) {
		wlc_cac_tspec_send(wlc->cac, wlc->cac->tsentryq);
		return;
	}

	/* send delts TS */
	err = wlc_cac_tspec_req_send(wlc->cac, &ts->tspec, WME_DELTS_REQUEST,
		ts->dialog_token, DOT11E_STATUS_QSTA_REQ_TIMEOUT, &pkt, scb);
	BCM_REFERENCE(err);

	/* get ts tid before removing ts */
	tid = WLC_CAC_GET_TID(ts->tspec.tsinfo);

	/* change ts entry state inorder to removefree */
	ts->ts_state = CAC_STATE_TSPEC_REJECTED;
	wlc_cac_tsentry_removefree(wlc->cac, tid, 0, 0, &wlc->cac->tsentryq, FALSE);
	wlc->cac->waiting_resp = FALSE;

	wlc_cac_addts_ind_complete(wlc, WLC_E_STATUS_TIMEOUT, &wlc->cfg->target_bss->BSSID,
		0, tid, 0);

	wlc_cac_tspec_send(wlc->cac, wlc->cac->tsentryq);
}

static void
wlc_cac_addts_ind_complete(wlc_info_t *wlc, uint status, struct ether_addr *addr,
        uint wme_status, uint tid, uint ts_delay)
{
	if (status == WLC_E_STATUS_SUCCESS)
		WL_CAC(("ADDTS: success ...\n"));
	else if (status == WLC_E_STATUS_TIMEOUT)
		WL_CAC(("ADDTS: timeout waiting for ADDTS response\n"));
	else if (status == WLC_E_STATUS_FAIL)
		WL_CAC(("ADDTS: failure, TSPEC rejected\n"));
	else
		WL_ERROR(("MACEVENT: ADDTS, unexpected status param %d\n", (int)status));
#if defined(BCMCCX) && defined(CCX_SDK)
		if (status != WLC_E_STATUS_SUCCESS && CCX_ENAB(wlc->pub) && IHV_ENAB(wlc->ccx))
			wlc_ccx_mgmt_status_hdlr(wlc->ccx, addr, wme_status, TRUE,
				&ts_delay, sizeof(ts_delay));
#endif // endif

	wlc_mac_event(wlc, WLC_E_ADDTS_IND, addr, status, wme_status,
		tid, &ts_delay, sizeof(ts_delay));
}

static void wlc_cac_delts_ind_complete(wlc_info_t *wlc, uint status, struct ether_addr *addr,
        uint wme_status, uint tid)
{
	if (status == WLC_E_STATUS_SUCCESS)
		WL_CAC(("DELTS: success ...\n"));
	else if (status == WLC_E_STATUS_UNSOLICITED)
		WL_CAC(("DELTS: unsolicited\n"));
	else
		WL_ERROR(("MACEVENT: DELTS, unexpected status param %d\n", (int)status));

	wlc_mac_event(wlc, WLC_E_DELTS_IND, addr, status, wme_status, tid, 0, 0);
}

/* Change all TSPEC with from_state to pending state */
static void
wlc_cac_tspec_state_change(wlc_cac_t *cac, int old_state, uint new_state, tsentry_t *tsentryq)
{
	tsentry_t *ts;

	for (ts = tsentryq; ts; ts = ts->next) {
		if (old_state == CAC_STATE_TSPEC_ANY)
			ts->ts_state = new_state;
		else if (ts->ts_state == old_state)
			ts->ts_state = new_state;
	}
}

/* reset all ac parameters */
static void
wlc_cac_ac_param_reset(wlc_cac_t *cac, uint8 ac, struct cac_scb_acinfo *scb_acinfo)
{
	wlc_cac_ac_t *cac_ac;
	cac_ac = &scb_acinfo->cac_ac[ac];

	cac_ac->tot_medium_time = 0;
	cac_ac->used_time = 0;
	bzero(cac_ac->dur_cache, sizeof(*cac_ac->dur_cache));

#ifdef BCMCCX
	if (CCX_ENAB(cac->wlc->pub))
		bzero(&cac_ac->ccx_tsm, sizeof(struct ccx_ts_metrics));
#endif // endif

	/* reset the admitted flag */
	cac_ac->admitted = FALSE;

	/* reset nominal phy rate */
	if ((cac->wlc->band->bandtype == WLC_BAND_2G) &&
		(cac->wlc->band->gmode == GMODE_LEGACY_B))
		cac_ac->nom_phy_rate = WLC_RATE_11M;
	else
		cac_ac->nom_phy_rate = WLC_RATE_24M;

	/* if there is a TS */
	if (AC_BITMAP_TST(cac->flags, ac)) {
		wl_lifetime_t lifetime;
		/* clear lifetime */
		lifetime.ac = (uint32)ac;
		lifetime.lifetime = 0;
		if (wlc_iovar_op(cac->wlc, "lifetime", NULL, 0,
			&lifetime, sizeof(lifetime), IOV_SET, NULL))
			WL_ERROR(("wl%d: %s: setting lifetime failed\n",
				cac->wlc->pub->unit, __FUNCTION__));

		AC_BITMAP_RESET(cac->flags, ac);

		/* if no more TS */
		if (!(cac->flags & AC_BITMAP_ALL) && cac->rfaware_lifetime) {
			/* restore RF awareness */
			if (wlc_iovar_setint(cac->wlc, "rfaware_lifetime", cac->rfaware_lifetime))
				WL_ERROR(("wl%d: %s: setting RF awareness lifetime failed\n",
					cac->wlc->pub->unit, __FUNCTION__));
		}
	}
}

/* change all roaming ts_state to pending when roam failed
 * and have the ADDTS state machine to handle TSPEC rejects
 */
void
wlc_cac_tspec_state_reset(wlc_cac_t *cac)
{
	WL_CAC(("wl%d: wlc_cac_roam_tspec: change all roaming state to pending"
		" state\n", cac->wlc->pub->unit));
	/* change all TSPEC from ROAMING to PENDING */
	wlc_cac_tspec_state_change(cac, CAC_STATE_TSPEC_ACCEPTED,
		CAC_STATE_TSPEC_PENDING, cac->tsentryq);
	wlc_cac_tspec_send(cac, cac->tsentryq);
}

/* Reset all cac parameters after disassoc, including release all
 * tspec. Don't required to send delts when disassoc, AP will do
 * its cleanup of tspec.
 */
void
wlc_cac_param_reset_all(wlc_cac_t *cac, struct scb *scb)
{
	uint8 ac;
	tsentry_t *ts;
	uint8 tid;
	struct cac_scb_acinfo *scb_acinfo;

	scb_acinfo = SCB_ACINFO(cac, scb);

	WL_CAC(("wl%d: %s: free all TSPEC & reset AC parameters\n",
		cac->wlc->pub->unit, __FUNCTION__));

	/* timer start when any TSPEC is in state WAIT_RESP */
	if ((ts = wlc_cac_tspec_state_find(cac->tsentryq,
		CAC_STATE_TSPEC_WAIT_RESP)) == NULL)
		wl_del_timer(cac->wlc->wl, cac->addts_timer);

	for (ac = 0; ac < AC_COUNT; ac++)
		wlc_cac_ac_param_reset(cac, ac, scb_acinfo);

	while ((ts = cac->tsentryq)) {
		tid = WLC_CAC_GET_TID(ts->tspec.tsinfo);
		/* indicated up all TSPEC has been deleted */
		wlc_cac_delts_ind_complete(cac->wlc, WLC_E_STATUS_UNSOLICITED,
			&cac->wlc->cfg->target_bss->BSSID, DOT11E_STATUS_QSTA_LEAVE_QBSS, tid);
		cac->tsentryq = ts->next;
		MFREE(cac->wlc->osh, ts, sizeof(tsentry_t));
	}
}

/* CAC watchdog routine, call by wlc_watchdog */
static void
wlc_cac_watchdog(void *hdl)
{
	uint8 ac;
	wlc_cac_ac_t *cac_ac;
	wlc_cac_t *cac = (wlc_cac_t *)hdl;
	struct scb *scb = NULL;
	struct cac_scb_acinfo *scb_acinfo = NULL;
#ifdef	AP
	struct scb_iter scbiter;
#endif	/* AP */

	if (!(WME_ENAB(cac->wlc->pub) && CAC_ENAB(cac->wlc->pub)))
		return;

	if (cac->curr_bsscfg &&
		(scb = wlc_scbfind(cac->wlc, cac->curr_bsscfg, &cac->curr_bssid)) &&
		!BSSCFG_AP(scb->bsscfg) &&
		SCB_WME(scb)) {
		scb_acinfo = SCB_ACINFO(cac, scb);

		/* reset used time for each AC */
		for (ac = 0; ac < AC_COUNT; ac++) {
			cac_ac = &scb_acinfo->cac_ac[ac];

			if (cac_ac->used_time <= cac_ac->tot_medium_time) {
				cac_ac->used_time = 0;
			} else {
				/* carry over-used time to next tx window */
				cac_ac->used_time -= cac_ac->tot_medium_time;
				/* Indicate to host that used time has been exceeded */
				wlc_mac_event(cac->wlc, WLC_E_EXCEEDED_MEDIUM_TIME,
					&cac->wlc->cfg->target_bss->BSSID, ac, 0, 0, 0, 0);
			}

#ifdef BCMCCX
			if (CCX_ENAB(cac->wlc->pub) && cac_ac->ccx_tsm.ts_metrics) {
				if (cac_ac->ccx_tsm.ts_metrics_interval <=
					(CCX_TSMETRIC_INTERVAL_MIN *
					cac_ac->ccx_tsm.ts_metrics_count)) {
					wlc_ccx_iapp_tsmetrics_send(cac, ac);
					cac_ac->ccx_tsm.ts_metrics_count = 0;
				}
				cac_ac->ccx_tsm.ts_metrics_count++;
			}
#endif	/* BCMCCX */
		}

	}

#ifdef AP
	if (AP_ENAB(cac->wlc->pub)) {
		FOREACHSCB(cac->wlc->scbstate, &scbiter, scb) {
			if (!SCB_WME(scb) || !SCB_ASSOCIATED(scb))
				continue;
			scb_acinfo = SCB_ACINFO(cac, scb);
			if (!scb_acinfo)
				continue;
			/* reset used time for each AC */
			for (ac = 0; ac < AC_COUNT; ac++) {
				if (!AC_BITMAP_TST(scb->bsscfg->wme->wme_admctl, ac))
					continue;
				cac_ac = &scb_acinfo->cac_ac[ac];

				if (cac_ac->used_time <= cac_ac->tot_medium_time) {
					cac_ac->used_time = 0;
				} else {
					/* carry over-used time to next tx window */
					cac_ac->used_time -= cac_ac->tot_medium_time;
				}
				wlc_cac_handle_inactivity(cac, ac, scb);
			}
		}
	}
#endif	/* AP */
}

/* Update or prepare cache duration data, to use it later from fast path */
void
wlc_cac_update_dur_cache(wlc_cac_t *cac, int ac, int prio,
	struct scb *scb, uint dur, uint pktlen, uint actionid)
{
	wlc_cac_ac_t *cac_ac;
	struct cac_scb_acinfo *scb_acinfo;
	wlc_bsscfg_t *cfg;
	wlc_cac_dur_cache_t *dur_cache;

	cfg = SCB_BSSCFG(scb);

	/* Check for admission control */
	if (!AC_BITMAP_TST(cfg->wme->wme_admctl, ac))
		return;

	scb_acinfo = SCB_ACINFO(cac, scb);
	cac_ac = &scb_acinfo->cac_ac[ac];
	dur_cache = cac_ac->dur_cache;

	switch (actionid) {
		case WLC_CAC_DUR_CACHE_PREP:
			dur_cache->dur_latest[prio].dur = dur;
			dur_cache->dur_latest[prio].pktlen = pktlen;
			WL_NONE(("%s: ac:%d, prio:%d, dur:%d, pktlen:%d, actionid:%d\n",
				__FUNCTION__, ac, prio, dur, pktlen, actionid));
			break;
		case WLC_CAC_DUR_CACHE_REFRESH:
			dur_cache->cached_dur[prio].dur = dur_cache->dur_latest[prio].dur;
			dur_cache->cached_dur[prio].pktlen = dur_cache->dur_latest[prio].pktlen;
			WL_NONE(("wl%d: For prio:%d, cached_dur(%d) updated to %d\n",
				cac->wlc->pub->unit, prio, dur_cache->cached_dur[prio].dur,
				dur_cache->dur_latest[prio].dur));
			break;
		default:
			ASSERT(0);
			break;
	}
}

/* Calculate approx time to transmit a pkt, based on current txrate */
static uint
wlc_cac_calc_pkt_dur(wlc_cac_t *cac, struct scb *scb,  uint pktlen)
{
	ratespec_t ratespec;
	uint phyrate;
	uint usec = 0;

	ratespec = wlc_scb_ratesel_get_primary(cac->wlc, scb, NULL);
	phyrate = RSPEC2KBPS(ratespec); /* Return phyrate in kbit/s */

	/* Calculation for time(in microsec) taken to transmit pktlen bytes:

	              8 bits      sec      Kbits        1,000,000 us   Bytes x 8 x 1000
	usec = Bytes x--------- x ------x-----------x------------- = ---------------------
	              byte       Kbits    1000 bits       sec            Kbps
	 */

	usec = (pktlen * 8 * 1000) / phyrate;

	WL_NONE(("wl%d: pktlen:%d, phyrate:%d, usec:%d\n", cac->wlc->pub->unit,
		pktlen, phyrate, usec));
	return usec;
}

/* Set the use of duration cache in the next call of wlc_cac_update_used_time.
 *  It should be called everytime if duration cache should be used(wlc_txfast)
 *  Fast path gives D11 header cache hit even if packet duration is different,
 *  so recalculate the approx duration.
 */
bool
wlc_cac_use_dur_cache(wlc_cac_t *cac, int ac, int prio,
	struct scb *scb, uint pktlen)
{
	wlc_cac_ac_t *cac_ac;
	struct cac_scb_acinfo *scb_acinfo;
	wlc_bsscfg_t *cfg;
	uint dur;

	cfg = SCB_BSSCFG(scb);
	scb_acinfo = SCB_ACINFO(cac, scb);
	cac_ac = &scb_acinfo->cac_ac[ac];

	/* Check for admission control */
	if (!AC_BITMAP_TST(cfg->wme->wme_admctl, ac))
		return FALSE;

	if (cac_ac->dur_cache->cached_dur[prio].pktlen == pktlen) {
		dur = cac_ac->dur_cache->cached_dur[prio].dur;
	}
	else {
		dur = wlc_cac_calc_pkt_dur(cac, scb, pktlen);
		/* Should we update the cache also? The calculation may be an approximate one */
	}

	return (wlc_cac_update_used_time(cac, ac, dur, scb));
}

/* Function to update used time. return TRUE if run out of time */
bool
wlc_cac_update_used_time(wlc_cac_t *cac, int ac, int dur, struct scb *scb)
{
	wlc_cac_ac_t *cac_ac;
	struct cac_scb_acinfo *scb_acinfo;
	wlc_bsscfg_t *cfg;
	struct pktq *q = NULL;
	uint16 prec_map = 0;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* Check for admission control */
	if (!AC_BITMAP_TST(cfg->wme->wme_admctl, ac))
		return FALSE;

	scb_acinfo = SCB_ACINFO(cac, scb);
	cac_ac = &scb_acinfo->cac_ac[ac];

	/* update used time */
	cac_ac->used_time += dur;

#ifdef BCMCCX
	/* only collect traffic stream metrics data when enabled */
	if (CCX_ENAB(cac->wlc->pub) && cac_ac->ccx_tsm.ts_metrics)
		cac_ac->ccx_tsm.total_used_time += (uint16)dur;
#endif // endif

	if (cac_ac->tot_medium_time && (cac_ac->used_time > cac_ac->tot_medium_time)) {
		WL_CAC(("wl%d: used time(%d us) is over admitted time(%d us)\n",
			cac->wlc->pub->unit, cac_ac->used_time,
			cac_ac->tot_medium_time));

		/* Admitted time for this AC is over, no new packet will be accepted now in.
		 * (wlc_sendpkt), but there are packets lying in Qs of various TXMODs, may pool
		 * up and cause CAC accounting problems. Trigger scrub of all TXMOD pkts for this
		 * scb+AC(all precedences).
		 * TODO: Make a generic function to flush all registered TXMODs holding relevant
		 *  pkts.
		 */
		q = WLC_GET_TXQ(cac->wlc->active_queue);
		prec_map = cac->wlc->fifo2prec_map[ac];

		wlc_txq_pktq_scb_filter(cac->wlc, prec_map, q, scb);
#if defined WLNAR
		wlc_nar_flush_scb_pqueues(cac->wlc, prec_map, NULL, scb);
#endif /* WLNAR */
		return TRUE;
	}

	WL_CAC(("%s: AC[%d] = %d\n", __FUNCTION__, ac, cac_ac->used_time));

	return FALSE;
}

#ifdef BCMCCX
/* handle CCX piggyback TSPEC IE in (re)assoc response */
static void
wlc_cac_assoc_tspec(wlc_cac_t *cac, uint8 *tlvs, uint tlvs_len, uint16 status, struct scb *scb)
{
	uint8 tid_dummy, user_prio_dummy;
	uint err;

	/* if no active TSPEC in queue, don't care about piggyback TSPECs */
	if (!cac->tsentryq)
		return;

	/* process piggyback TSPEC IE */
	err = wlc_cac_ie_process(cac, tlvs, tlvs_len, &tid_dummy, &user_prio_dummy, scb);
	BCM_REFERENCE(err);

	WL_CAC(("wl%d: wlc_cac_assoc_tspec: ASSOCIATION"
		" status %d\n", cac->wlc->pub->unit, status));

	wlc_cac_tspec_send(cac, cac->tsentryq);
}

/* handle association status for CCX */
uint16
wlc_cac_assoc_status(wlc_cac_t *cac, uint16 status)
{
	switch (status) {
		case DOT11_SC_SUCCESS:
			return status;
#ifdef BCMDBG
		case DOT11E_STATUS_ADDTS_REFUSED_NSBW:
		case CCX_STATUS_ASSOC_DENIED_NO_BW:
			WL_CAC(("wl%d: wlc_cac_assoc_status:"
				" Insufficient bandwidth 0x%02x\n", cac->wlc->pub->unit, status));
			break;
		case CCX_STATUS_ASSOC_DENIED_BAD_PARAM:
		case DOT11E_STATUS_ADDTS_INVALID_PARAM:
			WL_CAC(("wl%d: wlc_cac_assoc_status:"
				" CAC_ADDTS_INVALID_PARAM 0x%02x\n", cac->wlc->pub->unit, status));
			break;
		case CCX_STATUS_ASSOC_DENIED_AP_POLICY:
		case CCX_STATUS_ASSOC_DENIED_UNKNOWN:
			WL_CAC(("wl%d: wlc_cac_assoc_status:"
				" Association denied 0x%02x\n", cac->wlc->pub->unit, status));
			break;
		default:
			WL_CAC(("wl%d: wlc_cac_assoc_status:"
				" Unknown status 0x%02x\n", cac->wlc->pub->unit, status));
			break;
#endif	/* BCMDBG */
	}
	/* XXX handle assoc reject due to insufficient bandwidth and other
	 * error code. Call roam function and try next AP in list.
	wlc_ccx_roam(cac->wlc->ccx, status);
	 */
	return status;
}

#ifdef STA
/* TSPEC for CCX version larger or equal to 4 */
static uint
wlc_cac_calc_tspec_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_cac_t *cac = (wlc_cac_t *)ctx;
	wlc_info_t *wlc = cac->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	tsentry_t *ts;
	uint len = 0;

	if (!CAC_ENAB(wlc->pub))
		return 0;

	if (!CCX_ENAB(wlc->pub) || !wlc->ccx->ccx_network ||
	    ftcbparm->assocreq.target->ccx_version < 4)
		return 0;

	/* Append TSPEC to (re)assoc request */
	for (ts = cac->tsentryq; ts != NULL; ts = ts->next) {
		if (ts->ts_state != CAC_STATE_TSPEC_WAIT_RESP) {
			len += TLV_HDR_LEN + WME_TSPEC_LEN;
		}
	}

	return len;
}

static int
wlc_cac_write_tspec_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_cac_t *cac = (wlc_cac_t *)ctx;
	wlc_info_t *wlc = cac->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	tsentry_t *ts;
	uint8 *cp = data->buf;

	if (!CAC_ENAB(wlc->pub))
		return BCME_OK;

	if (!CCX_ENAB(wlc->pub) || !wlc->ccx->ccx_network ||
	    ftcbparm->assocreq.target->ccx_version < 4)
		return BCME_OK;

	/* Append TSPEC to (re)assoc request */
	for (ts = cac->tsentryq; ts != NULL; ts = ts->next) {

		if (ts->ts_state != CAC_STATE_TSPEC_WAIT_RESP) {

			cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, &ts->tspec, WME_TSPEC_LEN, cp);

			WL_CAC(("wl%d: wlc_cac_tspec_append: append TSPEC to (re)assoc message"
			        " tid %d\n",
			        cac->wlc->pub->unit, WLC_CAC_GET_TID(ts->tspec.tsinfo)));
		}
	}

	return BCME_OK;
}

static int
wlc_cac_parse_tspec_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_cac_t *cac = (wlc_cac_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	wlc_cac_assoc_tspec(cac, data->buf, data->buf_len,
	                    ftpparm->assocresp.status, ftpparm->assocresp.scb);
	return BCME_OK;
}

/* TSPEC Rateset for CCX version larger or equal to 4 */
static uint
wlc_cac_calc_ts_rs_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_cac_t *cac = (wlc_cac_t *)ctx;
	wlc_info_t *wlc = cac->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct cac_scb_acinfo *scb_acinfo;
	tsentry_t *ts;
	uint len = 0;

	if (!CAC_ENAB(wlc->pub))
		return 0;

	if (!CCX_ENAB(wlc->pub) || !wlc->ccx->ccx_network ||
	    ftcbparm->assocreq.target->ccx_version < 4)
		return 0;

	scb_acinfo = SCB_ACINFO(cac, ftcbparm->assocreq.scb);
	ASSERT(scb_acinfo != NULL);

	/* Append TSPEC Rateset to (re)assoc request */
	for (ts = cac->tsentryq; ts != NULL; ts = ts->next) {
		if (ts->ts_state != CAC_STATE_TSPEC_WAIT_RESP) {
			/* Fill in Stream rate values, if nominal Phy rate !=
			 * minimum Phy rate and tid != Signal TID
			 */
			if (WLC_CAC_GET_TID(ts->tspec.tsinfo) != CCX_CAC_SIGNAL_TID) {
				uint8 ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));
				if (WLC_RATE_500K_TO_BPS(scb_acinfo->cac_ac[ac].nom_phy_rate) !=
				    ts->tspec.min_phy_rate) {
					len += TLV_HDR_LEN + sizeof(ccx_stream_rs_t);
				}
			}
		}
	}

	return len;
}

static int
wlc_cac_write_ts_rs_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_cac_t *cac = (wlc_cac_t *)ctx;
	wlc_info_t *wlc = cac->wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct cac_scb_acinfo *scb_acinfo;
	tsentry_t *ts;
	uint8 *cp = data->buf;

	if (!CAC_ENAB(wlc->pub))
		return BCME_OK;

	if (!CCX_ENAB(wlc->pub) || !wlc->ccx->ccx_network ||
	    ftcbparm->assocreq.target->ccx_version < 4)
		return BCME_OK;

	scb_acinfo = SCB_ACINFO(cac, ftcbparm->assocreq.scb);
	ASSERT(scb_acinfo != NULL);

	/* Append TSPEC Rateset to (re)assoc request */
	for (ts = cac->tsentryq; ts; ts = ts->next) {
		if (ts->ts_state != CAC_STATE_TSPEC_WAIT_RESP) {
			/* Fill in Stream rate values, if nominal Phy rate !=
			 * minimum Phy rate and tid != Signal TID
			 */
			if (WLC_CAC_GET_TID(ts->tspec.tsinfo) != CCX_CAC_SIGNAL_TID) {
				uint8 tid = WLC_CAC_GET_TID(ts->tspec.tsinfo);
				uint8 ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tspec.tsinfo));
				if (WLC_RATE_500K_TO_BPS(scb_acinfo->cac_ac[ac].nom_phy_rate) !=
				    ts->tspec.min_phy_rate) {
					ccx_stream_rs_t rs;
					bcopy(CISCO_AIRONET_OUI, rs.oui, DOT11_OUI_LEN);
					rs.oui_type = CCX_CAC_TS_RATESET_TYPE;
					rs.tid = tid;
					rs.rateset[0] = scb_acinfo->cac_ac[ac].nom_phy_rate;

					cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, &rs,
						sizeof(ccx_stream_rs_t), cp);

					WL_CAC(("wl%d: wlc_cac_tspec_append: append rate info to"
						" (re)assoc message tid %d\n",
						cac->wlc->pub->unit, tid));
				}
			}
		}
	}

	return BCME_OK;
}
#endif /* STA */
#endif	/* BCMCCX */

/*
 * Frame received, frame type FC_ACTION,
 * action_category DOT11_ACTION_NOTIFICATION
 */
void
wlc_frameaction_cac(wlc_bsscfg_t *cfg, uint action_id, wlc_cac_t *cac,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	struct scb *scb;

	if (!(scb = wlc_scbfind(cac->wlc, cfg, &hdr->sa))) {
		WL_ERROR(("%s: SCB not found\n", __FUNCTION__));
		return;
	}

	wlc_cac_action_frame(cac, action_id, hdr, body, body_len, scb);

}

/* handle received action frame, frame type == FC_ACTION,
 * action_category = DOT11_ACTION_NOTIFICATION
 */
void
wlc_cac_action_frame(wlc_cac_t *cac, uint action_id,
	struct dot11_management_header *hdr, uint8 *body, int body_len, struct scb *scb)
{
	switch (action_id) {
		case WME_ADDTS_REQUEST:		/* AP only */
#ifdef	AP
			if (AP_ENAB(cac->wlc->pub))
				wlc_cac_process_addts_req(cac, body, body_len, scb);
#endif	/* AP */
			break;
		case WME_ADDTS_RESPONSE:	/* STA only */
			if (!BSSCFG_AP(scb->bsscfg))
				wlc_cac_addts_resp(cac, body, body_len, scb);
			break;
		case WME_DELTS_REQUEST:		/* AP and STA */
			wlc_cac_delts_req(cac, body, body_len, scb);
			break;
		default:
			WL_ERROR(("FC_ACTION, Invalid WME action code 0x%x\n", action_id));
			break;
	}
}

/* calculate total medium time granted by the AP */
uint32
wlc_cac_medium_time_total(wlc_cac_t *cac, struct scb *scb)
{
	int ac;
	struct cac_scb_acinfo *scb_acinfo;
	uint32 total = 0;

	scb_acinfo = SCB_ACINFO(cac, scb);

	/* calculate total medium time */
	for (ac = 0; ac < AC_COUNT; ac++)
		total += scb_acinfo->cac_ac[ac].tot_medium_time;
	return total;
}

static int
wlc_cac_scb_acinfo_init(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct cac_scb_acinfo *scb_acinfo;
	uint32 i = 0;
	uint ret = BCME_OK;

	WL_CAC(("%s: Entering\n", __FUNCTION__));

	scb_acinfo = SCB_ACINFO(wlc->cac, scb);
	ASSERT(scb_acinfo);

	if (CAC_ENAB(wlc->pub) && !SCB_ISMULTI(scb) && !SCB_ISPERMANENT(scb) &&
		!ETHER_ISNULLADDR(scb->ea.octet)) {
		for (i = 0; i < AC_COUNT; i++) {

			/* Init cache duration pointers */
			if (!(scb_acinfo->cac_ac[i].dur_cache =
					(wlc_cac_dur_cache_t *)MALLOCZ(wlc->osh,
					sizeof(wlc_cac_dur_cache_t)))) {
				WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				ret = BCME_ERROR;
				ASSERT(0);
			}
			scb_acinfo->cac_ac[i].admitted = FALSE;
			if ((wlc->band->bandtype == WLC_BAND_2G) &&
				(wlc->band->gmode == GMODE_LEGACY_B))
				scb_acinfo->cac_ac[i].nom_phy_rate = WLC_RATE_11M;
			else
				scb_acinfo->cac_ac[i].nom_phy_rate = WLC_RATE_24M;
		}
	}
	return ret;
}

static void
wlc_cac_scb_acinfo_deinit(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
#ifdef	AP
	tsentry_t *ts;
#endif // endif
	struct cac_scb_acinfo *scb_acinfo;
	uint ac;

	WL_CAC(("%s: Entering\n", __FUNCTION__));

	if (!(scb_acinfo = SCB_ACINFO(wlc->cac, scb)))
		return;

	for (ac = 0; ac < AC_COUNT; ac++) {
		/* Init cache duration pointers */
		if (scb_acinfo->cac_ac[ac].dur_cache) {
			MFREE(wlc->osh, scb_acinfo->cac_ac[ac].dur_cache,
				sizeof(wlc_cac_dur_cache_t));
		}
	}

#ifdef	AP
	/* free the tsentryq if present */
	while ((ts = scb_acinfo->tsentryq)) {
		scb_acinfo->tsentryq = ts->next;
		MFREE(wlc->osh, ts, sizeof(tsentry_t));
	}
#endif // endif
}

#ifdef BCMDBG
static void
wlc_cac_scb_acinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct cac_scb_acinfo *scb_acinfo;
	wlc_cac_ac_t *ac;
	uint32 i = 0;

	scb_acinfo = SCB_ACINFO(wlc->cac, scb);
	ASSERT(scb_acinfo);

	for (i = 0; i < AC_COUNT; i++) {
		ac = &scb_acinfo->cac_ac[i];
		if (ac->admitted) {
			bcm_bprintf(b, "%s: admt time 0x%x :used time 0x%x "
					":nom phy rate 0x%x ac %d\n",
					aci_names[i],
					ac->tot_medium_time, ac->used_time,
					ac->nom_phy_rate, i);
		}
	}
}
#endif /* BCMDBG */

#ifdef	BCMDBG
int
wlc_dump_cac(wlc_cac_t *cac, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "Use dump scb to get per scb related cac information\n");
	return 0;
}
#endif	/* BCMDBG */

/*
 * AP feature : Process the ADDTS request from the associated STA.
 */

#ifdef	AP
static int
wlc_cac_process_addts_req(wlc_cac_t *cac, uint8 *body, uint body_len, struct scb *scb)
{
	struct dot11_management_notification *mgmt_hdr;
	struct cac_scb_acinfo *scb_acinfo;
	uint8 tid, ac, dialog_token;
	uint16 tspec_medium_time = 0;
	tspec_t *ts = NULL;
	uint32	status = WME_ADMISSION_REFUSED;
	tspec_t ts_param;
	wlc_info_t *wlc = cac->wlc;
	wlc_bsscfg_t *cfg;
#ifdef	BCMDBG
	char buff[ETHER_ADDR_STR_LEN];

	WL_CAC(("%s: ADDTS Request from %s\n", __FUNCTION__, bcm_ether_ntoa(&scb->ea, buff)));
#endif // endif

	if (body_len < (WME_TSPEC_LEN + DOT11_MGMT_NOTIFICATION_LEN))
		return -1;

	/* init the various pointers and length */
	mgmt_hdr = (struct dot11_management_notification *)body;
	body += DOT11_MGMT_NOTIFICATION_LEN;
	body_len -= DOT11_MGMT_NOTIFICATION_LEN;

	/* look for tspec ie */
	if (!(body = wlc_cac_ie_find(body, &body_len, DOT11_MNG_PROPR_ID, WME_TSPEC_LEN,
		(const int8 *)DOT11E_TSPEC_IE, DOT11E_TSPEC_OUI_TYPE_LEN)))
		return -1;

	/* extract the tspec and related info */
	ts = (tspec_t *)&body[TLV_BODY_OFF];
	tid = WLC_CAC_GET_TID(ts->tsinfo);
	dialog_token = mgmt_hdr->token;
	ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tsinfo));

	scb_acinfo = SCB_ACINFO(cac, scb);

	/* copy the received tspec to local tspec */
	bcopy(ts, &ts_param, sizeof(tspec_t));

	/* Check tspec params for validity */
	if (wlc_cac_is_tsparam_valid(cac, ts, ac, scb) == FALSE) {
		status = WME_INVALID_PARAMETERS;
		goto done;
	}

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* Is admission enabled on given ac ? */
	if (!AC_BITMAP_TST(cfg->wme->wme_admctl, ac)) {
		status = WME_ADMISSION_ACCEPTED;
		tspec_medium_time = 0;
	} else {
		/* calculate the required medium time for this tspec */
		tspec_medium_time = wlc_cac_generate_medium_time(cac, ts, scb);
		/* check if new tspec can be allowed ? */
		status = wlc_cac_check_tspec_for_admission(cac, ts, tspec_medium_time, scb);
	}

	if (status == WME_ADMISSION_ACCEPTED) {
		tsentry_t *new_ts_entry;

		/* mark the admitted status */
		scb_acinfo->cac_ac[ac].admitted = TRUE;

		/* check if TSPEC already exists for given tid */
		if (!(new_ts_entry = wlc_cac_tsentry_find(scb_acinfo->tsentryq, tid))) {

			if (!(new_ts_entry =
				(tsentry_t *)MALLOC(wlc->osh, sizeof(tsentry_t)))) {
				WL_ERROR(("wl%d: %s: out of memory %d bytes\n",
				          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				return -1;
			}

			/* change the TSPEC state */
			bcopy(ts, &new_ts_entry->tspec, sizeof(tspec_t));
			new_ts_entry->ts_state = CAC_STATE_TSPEC_ACCEPTED;
			wlc_cac_tsentry_append(cac, new_ts_entry, &scb_acinfo->tsentryq);
		} else {
			/* update the old entry */
			bcopy(ts, &new_ts_entry->tspec, sizeof(tspec_t));
			new_ts_entry->ts_state = CAC_STATE_TSPEC_ACCEPTED;
			/* restore the available medium time */
			cac->available_medium_time +=
				USEC_TO_USEC32((uint16)scb_acinfo->cac_ac[ac].tot_medium_time);
		}

		/* update the medium time for the tspec */
		new_ts_entry->tspec.medium_time = tspec_medium_time;
		ts_param.medium_time = new_ts_entry->tspec.medium_time;

		/* get the direction of traffic stream */
		if (WLC_CAC_GET_DIR(ts->tsinfo) == (TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT))
			ts_param.medium_time = 0;

		/* update the remote station cac information */
		scb_acinfo->cac_ac[ac].inactivity_limit = WLC_CAC_GET_INACTIVITY_INTERVAL(ts);
		scb_acinfo->cac_ac[ac].inactivity_tid = WLC_CAC_GET_TID(ts->tsinfo);
		scb_acinfo->cac_ac[ac].tot_medium_time = USEC32_TO_USEC(ts_param.medium_time);

		/* decrease the global medium time */
		cac->available_medium_time -= ts_param.medium_time;

		/* update the power save behavior */
		if (WLC_CAC_GET_PSB(ts->tsinfo) && cfg->wme->wme_apsd) {
			switch (WLC_CAC_GET_DIR(ts->tsinfo)) {
				case (TS_INFO_UPLINK >> TS_INFO_DIRECTION_SHIFT):
					WL_CAC(("AC trigger enabled\n"));
					AC_BITMAP_SET(scb->apsd.ac_trig, ac);
					AC_BITMAP_RESET(scb->apsd.ac_delv, ac);
					break;

				case (TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT):
					WL_CAC(("AC delivery enabled\n"));
					AC_BITMAP_SET(scb->apsd.ac_delv, ac);
					AC_BITMAP_RESET(scb->apsd.ac_trig, ac);
					break;

				case (TS_INFO_BIDIRECTIONAL >> TS_INFO_DIRECTION_SHIFT):
					WL_CAC(("AC trigger and delivery enabled\n"));
					AC_BITMAP_SET(scb->apsd.ac_trig, ac);
					AC_BITMAP_SET(scb->apsd.ac_delv, ac);
					break;
				}
		}

		WL_CAC(("Inactivity interval %d\n", scb_acinfo->cac_ac[ac].inactivity_limit));
		WL_CAC(("Total medium time %d\n", scb_acinfo->cac_ac[ac].tot_medium_time));
		WL_CAC(("Available medium time %d\n", cac->available_medium_time));
		WL_CAC(("Tspec medium time %d\n", ts_param.medium_time));
	}

	/*
	 * XXX : Implement WME_ADMISSION_SUGGEST and WME_ADMISSION_RETRY
	 * conditions.
	 */

done:
	/* send the addts response to the associated sta */
	return	wlc_cac_addts_resp_send(cac, &ts_param, scb, status, dialog_token);
}

/*
 * check to see if new TSPEC maintains the existing QoS
 *
 * Use TOKEN value in mgmt frame to check resend/new
 *
 */

static int
wlc_cac_check_tspec_for_admission(wlc_cac_t *cac, tspec_t *ts,
	uint16 tspec_medium_time, struct scb *scb)
{
	struct cac_scb_acinfo *scb_acinfo;
	uint8 ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tsinfo));

	scb_acinfo = SCB_ACINFO(cac, scb);

	/* Is admission enabled on given ac ? */
	if (!AC_BITMAP_TST(scb->bsscfg->wme->wme_admctl, ac))
		return WME_ADMISSION_REFUSED;

	ASSERT(tspec_medium_time);

	/* refuse admission if already a TSPEC is admitted */
	if (scb_acinfo->cac_ac[ac].admitted) {
		if (!(wlc_cac_tsentry_find(scb_acinfo->tsentryq, WLC_CAC_GET_TID(ts->tsinfo)))) {
			WL_CAC(("%s: Admission Refused\n", __FUNCTION__));
			return WME_ADMISSION_REFUSED;
		} else {
			/* check if tspec update is possible */
			if (tspec_medium_time >
					(cac->available_medium_time +
					 USEC_TO_USEC32(scb_acinfo->cac_ac[ac].tot_medium_time)))
				return WME_ADMISSION_REFUSED;
			else
				return WME_ADMISSION_ACCEPTED;
		}
	}

	/* early protection : reject if we cannot support the tspec */
	if (tspec_medium_time >= cac->available_medium_time) {
		WL_CAC(("%s: Not enough budget resource\n", __FUNCTION__));
		return WME_ADMISSION_REFUSED;
	}

	/* check if the max limit is reached for the given ac */
	return WME_ADMISSION_ACCEPTED;
}

/*
 * AP feature : Send ADDTS Response with the TSPEC
 * TODO : Prune this function.
 *
 */
static int
wlc_cac_addts_resp_send(wlc_cac_t *cac, tspec_t *ts, struct scb *scb,
	uint32 status, uint8 dialog_token)
{
	void *pkt;
#ifdef	BCMDBG
	char buff[ETHER_ADDR_STR_LEN];
	WL_CAC(("%s: ADDTS response (%s)\n", __FUNCTION__, bcm_ether_ntoa(&scb->ea, buff)));
#endif // endif
	return wlc_cac_tspec_resp_send(cac, ts, WME_ADDTS_RESPONSE,
		status, dialog_token, &pkt, scb);
}

/*
 * AP feature : Send a action frame (tspec) response
 * containing a single WME TSPEC element
 */

static int
wlc_cac_tspec_resp_send(wlc_cac_t *cac, tspec_t *ts, uint8 action,
	int reason_code, uint8 dialog_token, void **pkt, struct scb *scb)
{
	uint len = 0;
	uint8 *body, *cp;
	wlc_info_t *wlc = cac->wlc;
	struct dot11_management_notification *mgmt_hdr;

	WL_CAC(("%s: Entering\n", __FUNCTION__));

	ASSERT(AP_ENAB(wlc->pub));	/* AP support only */
	ASSERT(scb);

	*pkt = NULL;

	ASSERT(wlc->pub->associated);

	if (!SCB_WME(scb)) {
		WL_CAC(("wl%d: %s : WME Not Enabled\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_NOT_WME_ASSOCIATION;
	}

/*
 * Management Action frame
 * -----------------------------------------------------------------------------------
 * | MAC hdr | Category code | Action code | Dialog token | Status | Elements | FCS
 * -----------------------------------------------------------------------------------
 *	24/30         1                 1             1            1                 4
 *
 * WMM Tspec element
 * -----------------------------------------------------------------------------------
 * ID | length | OUI | OUI type | OUI subtype | version | Tspec body
 * -----------------------------------------------------------------------------------
 * 1	1		3		1			1			1
 *
 */

	/* TSPEC Response frame is 4 bytes Management Notification frame
	 * follow by WME TSPEC element
	 */
	len = DOT11_MGMT_NOTIFICATION_LEN +
		(TLV_HDR_LEN + WME_TSPEC_LEN);

	/* Format the TSPEC ADDTS response frame */
	if ((*pkt = wlc_frame_get_mgmt(wlc, FC_ACTION, &scb->ea, &wlc->pub->cur_etheraddr,
	                               &wlc->cfg->BSSID, len, &body)) == NULL)
		return BCME_ERROR;

	/* update the mgmt notification header fields */
	mgmt_hdr = (struct dot11_management_notification *)body;
	mgmt_hdr->category = DOT11_ACTION_NOTIFICATION;
	mgmt_hdr->action = action;

	/* Update the dialog token from the received frame */
	mgmt_hdr->token = dialog_token;

	mgmt_hdr->status = (uint8)reason_code;
	cp = mgmt_hdr->data;

	/* Fill in TSPEC values */
	cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, ts, WME_TSPEC_LEN, cp);

	ASSERT((cp - body) == (int)len);
	/* adjust the packet length */
	PKTSETLEN(wlc->osh, *pkt, (DOT11_MGMT_HDR_LEN+len));

	if (wlc_sendmgmt(wlc, *pkt, SCB_WLCIFP(scb)->qi, scb))
		return 0;

	return BCME_ERROR;
}

/* get the new medium time
 *
 * TODO : Directly update the medium_time in TSPEC rather
 * than returning the result
 */

#define SBW_FP_PRECISION 13

static uint32
do_fpm(uint32 duration, uint32 sbw)
{
	ASSERT(sbw);
	return ((duration * sbw)/(2 << (SBW_FP_PRECISION - 1)));
}

/*
 * Recommended method to derive the medium time (wmmac spec, Appendix A.3)
 *
 * medium_time = surplus_bandwidth * pps * medium_time_per_frame_exchange
 * where
 * - pps = ceiling(mean_data_rate/8)/(nominal_msdu_size)
 * - medium_time_per_frame_exchange =
 *	duration(nominal_msdu_size, min_phy_rate) + SIFS + ACK time
 * - surplus_bandwidth, mean_data_rate, nominal_msdu_size, min_phy_rate
 *	are obtained from tspec
 *
 */

static uint16
wlc_cac_generate_medium_time(wlc_cac_t *cac, tspec_t *ts, struct scb *scb)
{

	uint32 medium_time;
	uint16 duration;
	uint32 min_phy_rate;
	uint8 preamble_type;

	ASSERT(ts);
	ASSERT(scb);

	preamble_type = ((scb->flags & SCB_SHORTPREAMBLE)? WLC_SHORT_PREAMBLE:
	                 WLC_LONG_PREAMBLE);

	/* convert bps to 500kbps */
	min_phy_rate = (ts->min_phy_rate / (500000));
	duration = wlc_wme_get_frame_medium_time(cac->wlc, min_phy_rate,
		preamble_type, ts->nom_msdu_size & TSPEC_NOM_MSDU_MASK);

	ASSERT(ts->nom_msdu_size & TSPEC_NOM_MSDU_MASK);
	medium_time = CEIL((ts->mean_data_rate/8),
		(ts->nom_msdu_size & TSPEC_NOM_MSDU_MASK)) * duration;

	/* scale it up */
	medium_time = do_fpm(medium_time, (uint32)ts->surplus_bw);

	/* convert to 32us units */
	medium_time = USEC_TO_USEC32(medium_time);
	return (uint16)medium_time;
}

static bool
wlc_cac_is_tsparam_valid(wlc_cac_t *cac, tspec_t *ts, int ac, struct scb *scb)
{
	uint min_phy_rate;
	uint8 i;
	bool found;
	wlc_bsscfg_t *cfg;

	/* convert bps to 500kbps */
	min_phy_rate = (ts->min_phy_rate / (500000));
	/* Must be in scb's association rateset */
	for (i = 0, found = FALSE; i < scb->rateset.count; i++) {
		if ((scb->rateset.rates[i] & RSPEC_RATE_MASK) == min_phy_rate) {
			found = TRUE;
			break;
		}
	}
	if (!found)
		return FALSE;

	/* Validate the rates in tspec */
	if ((ts->mean_data_rate == 0) ||
		((ts->nom_msdu_size & TSPEC_NOM_MSDU_MASK) == 0) ||
		(ts->surplus_bw == 0))
		return FALSE;

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* Reject if we do not support APSD */
	if (WLC_CAC_GET_PSB(ts->tsinfo) && !cfg->wme->wme_apsd)
		return FALSE;

	return TRUE;

}

/*
 * If the remote STA is inactive for more the interval we rip
 * the admitted TSPEC. An AP feature
 */
void
wlc_cac_handle_inactivity(wlc_cac_t *cac, int ac,
	struct scb *scb)
{
	wlc_cac_ac_t *cac_ac;
	struct cac_scb_acinfo *scb_acinfo = NULL;
	scb_acinfo = SCB_ACINFO(cac, scb);
	cac_ac = &scb_acinfo->cac_ac[ac];

	if (AP_ENAB(cac->wlc->pub) && cac_ac->admitted &&
		cac_ac->inactivity_limit) {
		/* Increment the inactivity interval */
		cac_ac->inactivity_interval++;

		if (cac_ac->inactivity_interval >=
			cac_ac->inactivity_limit) {
			tspec_arg_t tspec_arg;
			tsentry_t * ts = wlc_cac_tsentry_find(scb_acinfo->tsentryq,
				scb_acinfo->cac_ac[ac].inactivity_tid);
			WL_CAC(("Inactivity limit reached for TID %d\n",
				scb_acinfo->cac_ac[ac].inactivity_tid));
			ASSERT(ts);
			ASSERT(ac < AC_COUNT);

			/* Format the tspec */
			tspec_arg.version = TSPEC_ARG_VERSION;
			tspec_arg.length = sizeof(tspec_arg_t) - (2 * sizeof(uint16));
			bcopy((uint8 *)&ts->tspec.tsinfo, (uint8 *)&tspec_arg.tsinfo,
				sizeof(struct tsinfo));

			/* Send DELTS request to the STA */
			wlc_cac_sta_delts(cac, &tspec_arg, scb, scb_acinfo->tsentryq);

		}
	}
}

#endif	/* AP */

#ifdef BCMCCX
/* form the header of the Traffic Stream Metrics IAPP packet */
static uint8 *
wlc_ccx_iapp_hdr(wlc_cac_t *cac, ccx_iapp_hdr_t *iapp_pkt)
{
	if (cac->wlc->cfg->current_bss->ccx_version == 4 || cac->wlc->ccx->ccx_v4_only)
		iapp_pkt->id_len = hton16(CCX_TSM_IAPP_V4_LEN);
	else
		iapp_pkt->id_len = hton16(CCX_TSM_IAPP_LEN);
	iapp_pkt->type = CCX_IAPP_TYPE_RM;
	iapp_pkt->subtype = CCX_TSM_IAPP_SUBTYPE;
	bzero((char *)&iapp_pkt->da, ETHER_ADDR_LEN);
	bcopy((char *)&cac->wlc->pub->cur_etheraddr, (char *)&iapp_pkt->sa, ETHER_ADDR_LEN);
	return ((uint8 *)iapp_pkt + CCX_IAPP_HDR_LEN);
}

/* form the guts of the Traffic Stream Metrics IAPP packet */
static uint8 *
wlc_ccx_metrics_ie(wlc_cac_t *cac, ccx_tsm_param_t *metrics_ie,
	uint8 ac, struct cac_scb_acinfo *scb_acinfo)
{
	struct ccx_ts_metrics *ccx_tsm = &scb_acinfo->cac_ac[ac].ccx_tsm;

	uint tot_cnt, pkt_loss = 0;
	uint8 ccx_version = cac->wlc->cfg->current_bss->ccx_version;
	/* build traffic stream metrics IE header */
	bzero((char *)metrics_ie, sizeof(ccx_tsm_param_t));
	metrics_ie->id = htol16(CCX_TSM_ID);
	metrics_ie->token = htol16(CCX_TSM_TOKEN);
	metrics_ie->mode = CCX_TSM_MODE;
	if (ccx_version == 4 || cac->wlc->ccx->ccx_v4_only) {
		metrics_ie->len = htol16(CCX_TSM_IE_V4_LEN);	/* excluding ID & len */
		metrics_ie->type = CCX_TSM_V4_TYPE;
	} else {
		metrics_ie->len = htol16(CCX_TSM_IE_LEN);	/* excluding ID & len */
		metrics_ie->type = CCX_TSM_TYPE;
	}

	/* calculate average delay */
	tot_cnt = ccx_tsm->cnt_delay10 + ccx_tsm->cnt_delay20 +
		ccx_tsm->cnt_delay40 + ccx_tsm->cnt_delay;
	metrics_ie->avg_delay = 0;
	metrics_ie->media_delay = 0;
	if (tot_cnt) {
		uint32 avg = ccx_tsm->total_pkt_delay/tot_cnt;
		/* round to nearest integer */
		avg = (avg + (USEC_PER_MSEC / 2)) / USEC_PER_MSEC;
		metrics_ie->avg_delay = htol16((uint16)avg);
		metrics_ie->media_delay = htol32(ccx_tsm->total_media_delay/tot_cnt);
	}

	metrics_ie->cnt_delay10 = htol16(ccx_tsm->cnt_delay10);
	metrics_ie->cnt_delay20 = htol16(ccx_tsm->cnt_delay20);
	metrics_ie->cnt_delay40 = htol16(ccx_tsm->cnt_delay40);
	metrics_ie->cnt_delay = htol16(ccx_tsm->cnt_delay);

	metrics_ie->pkt_cnt = htol16((uint16)(ccx_tsm->pkt_tx));

	/* since we don't know how many pkts are waiting in hw,
	 * therefor after tsm reported we reset the counter which can
	 * cause ccx_tsm->pkt_tx < ccx_tsm->pkt_tx_ok (result is
	 * negative)
	 */
	if (ccx_tsm->pkt_tx >= ccx_tsm->pkt_tx_ok)
		pkt_loss = ccx_tsm->pkt_tx - ccx_tsm->pkt_tx_ok;

	metrics_ie->pkt_loss = htol16((uint16)pkt_loss);

	metrics_ie->roam_cnt = cac->ccx_roam_cnt;
	htol16_ua_store(cac->ccx_roam_delay, (uint8 *)&metrics_ie->roam_delay);

	if (ccx_version > 4 && !cac->wlc->ccx->ccx_v4_only) {
		metrics_ie->used_time = htol16(ccx_tsm->total_used_time);
		metrics_ie->tid = ccx_tsm->tid;
	}

	if (ccx_version > 4 && !cac->wlc->ccx->ccx_v4_only) {
		WL_CAC(("wl%d: wlc_ccx_metrics_ie: AC %d pkt delay %d %d %d %d %d media delay %d"
			" pkt lost %d pkt cnt %d roam cnt %d roam delay %d used time %d tid %d\n",
			cac->wlc->pub->unit, ac, ltoh16(metrics_ie->avg_delay),
			ccx_tsm->cnt_delay10, ccx_tsm->cnt_delay20,
			ccx_tsm->cnt_delay40, ccx_tsm->cnt_delay,
			ltoh32(metrics_ie->media_delay), ltoh16(metrics_ie->pkt_loss),
			ltoh16(metrics_ie->pkt_cnt), metrics_ie->roam_cnt, cac->ccx_roam_delay,
			ltoh16(metrics_ie->used_time), metrics_ie->tid));
	} else {
		WL_CAC(("wl%d: wlc_ccx_metrics_ie: AC %d pkt delay %d %d %d %d %d media delay %d"
			" pkt lost %d pkt cnt %d roam cnt %d roam delay %d\n",
			cac->wlc->pub->unit, ac, ltoh16(metrics_ie->avg_delay),
			ccx_tsm->cnt_delay10, ccx_tsm->cnt_delay20,
			ccx_tsm->cnt_delay40, ccx_tsm->cnt_delay,
			ltoh32(metrics_ie->media_delay), ltoh16(metrics_ie->pkt_loss),
			ltoh16(metrics_ie->pkt_cnt), metrics_ie->roam_cnt, cac->ccx_roam_delay));
	}

	/* reset and update after each report */
	ccx_tsm->total_pkt_delay = 0;
	ccx_tsm->cnt_delay10 = 0;
	ccx_tsm->cnt_delay20 = 0;
	ccx_tsm->cnt_delay40 = 0;
	ccx_tsm->cnt_delay = 0;
	ccx_tsm->total_media_delay = 0;
	ccx_tsm->pkt_tx = 0;
	ccx_tsm->pkt_tx_ok = 0;
	ccx_tsm->total_used_time = 0;
	cac->ccx_roam_cnt = 0;
	cac->ccx_roam_delay = 0;

	if (ccx_version == 4 || cac->wlc->ccx->ccx_v4_only)
		return ((uint8 *)metrics_ie + CCX_TSM_V4_LEN);
	else
		return ((uint8 *)metrics_ie + CCX_TSM_LEN);
}

/* This function form the IAPP packet to the AP with Traffic Stream
 * Metrics data embedded in the packet
 */
static void
wlc_ccx_iapp_tsmetrics_send(wlc_cac_t *cac, uint8 ac)
{
	void *p;
	uint len;
	uint8 *body;
	tsentry_t *ts;
	uint priority = 0;
	wlc_info_t *wlc = cac->wlc;
	struct scb *scb;
	struct cac_scb_acinfo *scb_acinfo;

	scb = wlc_scbfind(cac->wlc, cac->curr_bsscfg, &cac->curr_bssid);
	scb_acinfo = SCB_ACINFO(cac, scb);

	/* Traffic Stream Metrics supported by CCX version 4 or greater */
	if (wlc->cfg->current_bss->ccx_version < 4)
		return;

	if (wlc->cfg->current_bss->ccx_version == 4 || cac->wlc->ccx->ccx_v4_only)
		len = CCX_TSM_IAPP_PKT_V4_LEN;
	else
		len = CCX_TSM_IAPP_PKT_LEN;

	p = wlc_dataget(wlc, &wlc->cfg->BSSID, (uint16)len, ETHER_HDR_LEN + len);
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
			cac->wlc->pub->unit, __FUNCTION__, len));
		return;
	}

	body = (uint8 *)PKTDATA(cac->wlc->osh, p);

	/* form snap header */
	bcopy(CISCO_AIRONET_SNAP, (body + ETHER_HDR_LEN),
		DOT11_LLC_SNAP_HDR_LEN);
	body = (body + ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);

	body = wlc_ccx_iapp_hdr(cac, (ccx_iapp_hdr_t *)body);

	body = wlc_ccx_metrics_ie(cac, (ccx_tsm_param_t *)body, ac, scb_acinfo);

	/* if NO signal TSPEC setup; use default priority, otherwise
	 * use SIGNAL TSPEC priority
	 */

	if ((ts = wlc_cac_tsentry_find(cac->tsentryq, CCX_CAC_SIGNAL_TID))) {
		if (ts->ts_state == CAC_STATE_TSPEC_ACCEPTED)
			priority = CCX_CAC_SIGNAL_USER_PRIO;
	}
	PKTSETPRIO(p, priority);

	wlc_sendpkt(wlc, p, NULL);
}

/* This function update the last_tsf_timestamp when AC queue is empty */
void
wlc_ccx_tsm_mediadelay(wlc_cac_t *cac, uint fifo, void *p, struct scb *scb)
{
	wlc_info_t *wlc = cac->wlc;
	struct dot11_header *h;
	uint8 *plcp, ac;
	uint16 fc;
	uint32 tsf_l, tsf_h;
	struct cac_scb_acinfo *scb_acinfo;

	ASSERT(p);
	ASSERT(PKTPRIO(p) <= MAXPRIO);

	ac = WME_PRIO2AC(PKTPRIO(p));
	scb_acinfo = SCB_ACINFO(cac, scb);

	/* only collect traffic stream metrics data when enabled */
	if (!scb_acinfo->cac_ac[ac].ccx_tsm.ts_metrics)
		return;

	plcp = PKTDATA(wlc->osh, p) + sizeof(d11txh_t);
	h = (struct dot11_header *)(plcp + D11_PHY_HDR_LEN);
	fc = ltoh16(h->fc);
	if ((FC_TYPE(fc) != FC_TYPE_DATA))
		return;

	if (TXPKTPENDGET(wlc, fifo) == 0) {
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		scb_acinfo->cac_ac[ac].ccx_tsm.last_tsf_timestamp = tsf_l;
	}
}

/* This function collect and update Traffic Stream Metrics and is
 * called when packet Tx completed.
 */
void
wlc_ccx_tsm_pktdelay(wlc_cac_t *cac, void *p, tx_status_t *txs,
	struct dot11_header *h, struct scb *scb)
{
	wlc_info_t *wlc = cac->wlc;
	struct ccx_ts_metrics *ccx_tsm;
	uint32 tsf_l, tsf_h, media_delay, pkt_delay;
	uint16 fc;
	uint8 ac;
	struct cac_scb_acinfo *scb_acinfo;

	ASSERT(p);
	ASSERT(PKTPRIO(p) <= MAXPRIO);

	ac = WME_PRIO2AC(PKTPRIO(p));

	scb_acinfo = SCB_ACINFO(cac, scb);
	if (!scb_acinfo->cac_ac[ac].ccx_tsm.ts_metrics)
		return;

	wlc_read_tsf(wlc, &tsf_l, &tsf_h);

	fc = ltoh16(h->fc);
	ccx_tsm = &scb_acinfo->cac_ac[ac].ccx_tsm;

	if ((FC_TYPE(fc) == FC_TYPE_DATA)) {
		/* XXX check if packet timestamp is greater then
		 * tsf_l => tsf_l wrap. This may happen every 1.25
		 * hour we drop a static.
		 */
		if ((WLPKTTAG(p)->u.exptime - wlc->lifetime[ac]) > tsf_l)
			return;

		/* calculate front of queue delay */
		media_delay = tsf_l - ccx_tsm->last_tsf_timestamp;
		ccx_tsm->total_media_delay += media_delay;

		/* check if roam has happened */
		if (cac->roamed) {
			cac->ccx_roam_delay = (uint16)(media_delay / USEC_PER_MSEC);
			cac->ccx_roam_cnt++;
			cac->roamed = FALSE;
			WL_CAC(("wl%d: wlc_ccx_delay_cal: roam cnt %d roam delay %d\n",
				wlc->pub->unit, cac->ccx_roam_cnt, cac->ccx_roam_delay));
		}

		/* calculate packet delay */
		pkt_delay = tsf_l - (WLPKTTAG(p)->u.exptime - wlc->lifetime[ac]);
		ccx_tsm->total_pkt_delay += pkt_delay;

		/* update histogram for each bucket */
		if (pkt_delay <= CCX_TSM_10MS_DELAY)
			ccx_tsm->cnt_delay10++;
		else if (pkt_delay <= CCX_TSM_20MS_DELAY)
			ccx_tsm->cnt_delay20++;
		else if (pkt_delay <= CCX_TSM_40MS_DELAY)
			ccx_tsm->cnt_delay40++;
		else
			ccx_tsm->cnt_delay++;

		if (txs->status.was_acked)
			ccx_tsm->pkt_tx_ok++;
	}

	/* update tsf timestamp for next front of queue calculation */
	ccx_tsm->last_tsf_timestamp = tsf_l;
}

void
wlc_ccx_tsm_pktcnt(wlc_cac_t *cac, uint8 ac, struct scb *scb)
{
	struct cac_scb_acinfo *scb_acinfo;

	scb_acinfo = SCB_ACINFO(cac, scb);

	/* only collect traffic stream metrics data when enabled */
	if (!scb_acinfo->cac_ac[ac].ccx_tsm.ts_metrics)
		return;

	scb_acinfo->cac_ac[ac].ccx_tsm.pkt_tx++;
}
#endif	/* BCMCCX */

uint8
wlc_cac_is_traffic_admitted(wlc_cac_t *cac, int ac, struct scb *scb)
{
	struct cac_scb_acinfo *scb_acinfo;
	wlc_cac_ac_t *cac_ac;

	if ((!SCB_BSSCFG(scb)->BSS) || SCB_ISMULTI(scb) || !SCB_WME(scb))
		return TRUE;

	ASSERT(ac < AC_COUNT);
	scb_acinfo = SCB_ACINFO(cac, scb);
	cac_ac = &scb_acinfo->cac_ac[ac];

	/* AP does not make admission control mandatory for this AC */
	if (!AC_BITMAP_TST(scb->bsscfg->wme->wme_admctl, ac))
		return WLC_CAC_NO_ADM_CTRL;
	/* TS is not yet requested, or rejected */
	if (!scb_acinfo->cac_ac[ac].admitted) {
		WL_CAC(("%s: acm on for ac %d but admitted 0x%x\n",
			__FUNCTION__, ac, scb_acinfo->cac_ac[ac].admitted));
		return WLC_CAC_NOT_ADMITTED;
	}
	/* Check if allowed medium time is remaining, or exhausted */
	if ((cac_ac->tot_medium_time) &&
	    (cac_ac->used_time > cac_ac->tot_medium_time)) {
		WL_CAC(("%s: used_time 0x%x for ac %d, exceeds tot_medium_time 0x%x\n",
			__FUNCTION__, cac_ac->used_time, ac, cac_ac->tot_medium_time));
		return WLC_CAC_ALLOWED_TXOP_ISOVER;
	}

	return TRUE;
}

/*
 * Reset the inactivity interval on any tx/rx activity
 */
void
wlc_cac_reset_inactivity_interval(wlc_cac_t *cac, int ac, struct scb *scb)
{
	struct cac_scb_acinfo *scb_acinfo;

	ASSERT(scb);
	ASSERT(ac < AC_COUNT);
	ASSERT(!SCB_ISMULTI(scb));

	scb_acinfo = SCB_ACINFO(cac, scb);
	scb_acinfo->cac_ac[ac].inactivity_interval = 0;
}

void
wlc_cac_on_join_bss(wlc_cac_t *cac, wlc_bsscfg_t *bsscfg, struct ether_addr *bssid, bool roam)
{
#if defined(BCMCCX)
	uint8 ac;
	struct scb *scb_join, *scb_leave;
	struct cac_scb_acinfo *scb_acinfo_join, *scb_acinfo_leave;
	wlc_info_t *wlc = cac->wlc;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char buff[ETHER_ADDR_STR_LEN];
#endif // endif
#endif /* BCMCCX */

	/* update curr_bsscfg */
	cac->curr_bsscfg = wlc_bsscfg_find_by_bssid(cac->wlc, bssid);
	ASSERT(cac->curr_bsscfg != NULL);

#if defined(BCMCCX)
	if (CCX_ENAB(wlc->pub)) {
		cac->roamed = FALSE;
		/* if roam to a new AP, set roamed flag if ts metrics enabled */
		if (roam && bcmp((uint8*)bssid, (uint8*)&cac->curr_bssid, ETHER_ADDR_LEN)) {
			scb_join = wlc_scbfind(wlc, cac->curr_bsscfg, bssid);
			ASSERT(scb_join);
			if (!scb_join) {
				WL_ERROR(("wl%d: %s: find scb for joining bss %s failed\n",
					wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(bssid, buff)));
				return;
			}
			scb_leave = wlc_scbfind(wlc, cac->curr_bsscfg, &cac->curr_bssid);
			ASSERT(scb_leave);
			if (!scb_leave) {
				WL_ERROR(("wl%d: %s: find scb for leaving bss %s failed\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa(&cac->curr_bssid, buff)));
				return;
			}

			/* move ac info from leaving scb to joining scb */
			scb_acinfo_join = SCB_ACINFO(cac, scb_join);
			ASSERT(scb_acinfo_join);
			scb_acinfo_leave = SCB_ACINFO(cac, scb_leave);
			ASSERT(scb_acinfo_leave);
			*scb_acinfo_join = *scb_acinfo_leave;
			bzero(scb_acinfo_leave, sizeof(struct cac_scb_acinfo));

			wlc_cac_scb_acinfo_init(wlc, scb_leave);

			for (ac = 0; ac < AC_COUNT; ac++) {
				if (scb_acinfo_join->cac_ac[ac].ccx_tsm.ts_metrics) {
					cac->roamed = TRUE;
					break;
				}
			}
		}
	}
#endif /* BCMCCX */

	/* update curr_bssid */
	bcopy((uint8*)bssid, (uint8*)&cac->curr_bssid, ETHER_ADDR_LEN);
}

void
wlc_cac_on_leave_bss(wlc_cac_t *cac)
{
	tsentry_t *ts;

	while ((ts = cac->tsentryq)) {
		cac->tsentryq = ts->next;
		MFREE(cac->wlc->osh, ts, sizeof(tsentry_t));
	}
}

#ifdef	BCMDBG
void
wlc_print_tspec(tspec_t *ts)
{

	ASSERT(ts);

	if (ts->version != TSPEC_ARG_VERSION) {
		printf("\tIncorrect version of TSPEC struct: expected %d; got %d\n",
			TSPEC_ARG_VERSION, ts->version);
	}

	/* TODO : Change to bcm_bprintf */

	printf("TID %d \n", WLC_CAC_GET_TID(ts->tsinfo));
	printf("tsinfo 0x%02x 0x%02x 0x%02x\n", ts->tsinfo.octets[0],
		ts->tsinfo.octets[1], ts->tsinfo.octets[2]);
	printf("nom_msdu_size %d %s\n", (ts->nom_msdu_size & 0x7fff),
		(ts->nom_msdu_size & 0x8000) ? "fixed size" : "");
	printf("max_msdu_size %d\n", ts->max_msdu_size);
	printf("min_srv_interval %d\n", ts->min_srv_interval);
	printf("max_srv_interval %d\n", ts->max_srv_interval);
	printf("inactivity_interval %d\n", ts->inactivity_interval);
	printf("suspension_interval %d\n", ts->suspension_interval);
	printf("srv_start_time %d\n", ts->srv_start_time);
	printf("min_data_rate %d\n", ts->min_data_rate);
	printf("mean_data_rate %d\n", ts->mean_data_rate);
	printf("peak_data_rate %d\n", ts->peak_data_rate);
	printf("max_burst_size %d\n", ts->max_burst_size);
	printf("delay_bound %d\n", ts->delay_bound);
	printf("min_phy_rate %d\n", ts->min_phy_rate);
	printf("surplus_bw %d\n", ts->surplus_bw);
	printf("medium_time %d\n", ts->medium_time);
}
#endif /* DEBUG */

#if defined(WLFBT)
/* Calculate length of RIC IEs */
uint
wlc_cac_calc_ric_len(wlc_cac_t *cac, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cac->wlc;
	tsentry_t *ts;
	uint len = 0;

	if (!CAC_ENAB(wlc->pub))
		return 0;

	/* calc IEs' length */
	for (ts = cac->tsentryq; ts != NULL; ts = ts->next) {
		len += wlc_ier_calc_len(wlc->ier_ric, cfg, 0, NULL);
	}
	return len;
}

/* Build RIC IEs */
bool
wlc_cac_write_ric(wlc_cac_t *cac, wlc_bsscfg_t *cfg, uint8 *cp, int *ricie_count)
{
	wlc_info_t *wlc = cac->wlc;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	int cp_len;
	tsentry_t *ts;

	if (!CAC_ENAB(wlc->pub))
		return FALSE;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.fbtric.ts_count = 0;
	ftcbparm.fbtric.rde_count = 0;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	/* RIC can have multiple Resource requests.
	 * Each Resource request consists of (RDE IE + WME TSPEC IE)
	 */
	/* length of a single Resource request in RIC */
	cp_len = wlc_ier_calc_len(wlc->ier_ric, cfg, 0, NULL);

	for (ts = cac->tsentryq; ts != NULL; ts = ts->next) {
		ftcbparm.fbtric.ts_count += 1;
		ftcbparm.fbtric.ts = (uint8 *)ts;
		if (wlc_ier_build_frame(wlc->ier_ric, cfg, WLC_IEM_FC_UNK,
			&cbparm, cp, cp_len) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
				wlc->pub->unit, __FUNCTION__));
			return FALSE;
		}
		/* Point to the next Resource request in RIC */
		cp += cp_len;
	}
	/* Calculate the total no of IEs in the RIC, used for MIC in FT IE */
	*ricie_count = ftcbparm.fbtric.ts_count + ftcbparm.fbtric.rde_count;
	return TRUE;
}

static uint
wlc_cac_fbt_calc_tspec_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	/* Add size for TSPEC IEs. */
	if (CAC_ENAB(((wlc_cac_t *)ctx)->wlc->pub))
		return (TLV_HDR_LEN + WME_TSPEC_LEN);

	return 0;
}

/* Add TSPEC to RIC in reassoc request */
static int
wlc_cac_fbt_write_tspec_ie(void *ctx, wlc_iem_build_data_t *data)
{
	if (CAC_ENAB(((wlc_cac_t *)ctx)->wlc->pub)) {
	/* TODO Include BlockACK RDIEs. Not required for VE tests yet. */
		wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
		uint8 *cp;
		tspec_t *copied_tspec;
		uint8 *bptr;
		tsentry_t *ts = (tsentry_t*)ftcbparm->fbtric.ts;
		const tspec_t *tspec = &ts->tspec;

		cp = data->buf;
		/* copy TSPECs. */
		copied_tspec = (tspec_t*)(((bcm_tlv_t*)cp)->data);
		cp = bcm_write_tlv(DOT11_MNG_PROPR_ID, tspec, WME_TSPEC_LEN, cp);

		/* make sure medium time is zero, avoiding alignment hassles */
		bptr = (uint8*)&copied_tspec->medium_time;
		*bptr++ = 0;
		*bptr = 0;
	}
	return BCME_OK;
}

/*
 * For V-E fast transition any TSPECs are transferred from the previous AP to the new one.
 * So copy the CAC state.
 */
void wlc_cac_copy_state(wlc_cac_t *cac, struct scb *prev_scb, struct scb *scb)
{
#ifdef BCMCCX
	/* CAC can be enabled with or without CCX. When CCX is on, CAC state is copied in
	 * inside wlc_cac_on_join_bss() else it is copied here.
	 */
	if (CCX_ENAB(cac->wlc->pub))
		return;
#endif /* BCMCCX */

	if (scb != prev_scb) {
		struct cac_scb_acinfo *prev_scb_acinfo = SCB_ACINFO(cac, prev_scb);
		struct cac_scb_acinfo *scb_acinfo = SCB_ACINFO(cac, scb);

		*scb_acinfo = *prev_scb_acinfo;
		bzero(prev_scb_acinfo, sizeof(*prev_scb_acinfo));
	}
}

#ifdef AP
uint
wlc_cac_ap_write_ricdata(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct scb *scb, uint8 *tlvs, uint tlvs_len,
	wlc_iem_ft_cbparm_t *ftcbparm)
{
	wlc_cac_t *cac;
	tspec_t *ts = NULL;
	uint8 *ptr = NULL;
	struct cac_scb_acinfo *scb_acinfo;
	uint8 tid, ac;
	uint32	status = WME_ADMISSION_REFUSED;
	uint16 tspec_medium_time = 0;
	tspec_t ts_param;

	if (!CAC_ENAB(wlc->pub))
		return FALSE;

	cac = wlc->cac;
	scb_acinfo = SCB_ACINFO(cac, scb);
	ASSERT(scb_acinfo);

	ptr = tlvs;

	if (tlvs_len > 0 && ptr) {
		ptr = wlc_cac_ie_find(ptr, &tlvs_len, DOT11_MNG_PROPR_ID, WME_TSPEC_LEN,
		(const int8 *)DOT11E_TSPEC_IE, DOT11E_TSPEC_OUI_TYPE_LEN);

		if (ptr == NULL) {
			return WME_INVALID_PARAMETERS;
		}

		/* extract the tspec and related info */
		ts = (tspec_t *)&ptr[TLV_BODY_OFF];
		tid = WLC_CAC_GET_TID(ts->tsinfo);
		ac = WME_PRIO2AC(WLC_CAC_GET_USER_PRIO(ts->tsinfo));

		scb_acinfo = SCB_ACINFO(cac, scb);

		/* copy the received tspec to local tspec */
		bcopy(ts, &ts_param, sizeof(tspec_t));

		/* Check tspec params for validity */
		if (wlc_cac_is_tsparam_valid(cac, ts, ac, scb) == FALSE) {
			return  WME_INVALID_PARAMETERS;
		}

		ASSERT(cfg != NULL);

		/* Is admission enabled on given ac ? */
		if (!AC_BITMAP_TST(cfg->wme->wme_admctl, ac)) {
			tspec_medium_time = 0;
			return  WME_INVALID_PARAMETERS;
		} else {
			/* calculate the required medium time for this tspec */
			tspec_medium_time = wlc_cac_generate_medium_time(cac, ts, scb);

			/* Refer WMM SPEC V1.2.0 Section 3.5.2
			 * An AP shall return a Medium Time equal to zero when accepting a
			 * unidirectional, downlink TS request
			*/
			if (WLC_CAC_GET_DIR(ts->tsinfo) ==
				(TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT))
				tspec_medium_time = 0;
			/* check if new tspec can be allowed ? */
			status = wlc_cac_check_tspec_for_admission(cac, ts, tspec_medium_time, scb);
		}

		if (status == WME_ADMISSION_ACCEPTED) {
			tsentry_t *new_ts_entry;

			/* mark the admitted status */
			scb_acinfo->cac_ac[ac].admitted = TRUE;

			/* check if TSPEC already exists for given tid */
			if (!(new_ts_entry = wlc_cac_tsentry_find(scb_acinfo->tsentryq, tid))) {
				if (!(new_ts_entry =
					(tsentry_t *)MALLOC(wlc->osh, sizeof(tsentry_t)))) {
					WL_ERROR(("wl%d: %s: out of memory %d bytes\n",
						wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
					return WME_ADMISSION_REFUSED;
				}

				/* change the TSPEC state */
				bcopy(ts, &new_ts_entry->tspec, sizeof(tspec_t));
				new_ts_entry->ts_state = CAC_STATE_TSPEC_ACCEPTED;
				wlc_cac_tsentry_append(cac, new_ts_entry, &scb_acinfo->tsentryq);
			} else {
				/* update the old entry */
				bcopy(ts, &new_ts_entry->tspec, sizeof(tspec_t));
				new_ts_entry->ts_state = CAC_STATE_TSPEC_ACCEPTED;
				/* restore the available medium time */
				cac->available_medium_time +=
				USEC_TO_USEC32((uint16)scb_acinfo->cac_ac[ac].tot_medium_time);
			}

			/* update the medium time for the tspec */
			new_ts_entry->tspec.medium_time = tspec_medium_time;
			ts_param.medium_time = new_ts_entry->tspec.medium_time;

			/* get the direction of traffic stream */
			if (WLC_CAC_GET_DIR(ts->tsinfo) ==
				(TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT))
				ts_param.medium_time = 0;

			/* update the remote station cac information */
			scb_acinfo->cac_ac[ac].inactivity_limit =
			WLC_CAC_GET_INACTIVITY_INTERVAL(ts);
			scb_acinfo->cac_ac[ac].inactivity_tid = WLC_CAC_GET_TID(ts->tsinfo);
			scb_acinfo->cac_ac[ac].tot_medium_time =
			USEC32_TO_USEC(ts_param.medium_time);

			/* decrease the global medium time */
			cac->available_medium_time -= ts_param.medium_time;

			/* update the power save behavior */
			if (WLC_CAC_GET_PSB(ts->tsinfo) && cfg->wme->wme_apsd) {
				switch (WLC_CAC_GET_DIR(ts->tsinfo)) {
					case (TS_INFO_UPLINK >> TS_INFO_DIRECTION_SHIFT):
						WL_CAC(("AC trigger enabled\n"));
						AC_BITMAP_SET(scb->apsd.ac_trig, ac);
						AC_BITMAP_RESET(scb->apsd.ac_delv, ac);
						break;

					case (TS_INFO_DOWNLINK >> TS_INFO_DIRECTION_SHIFT):
						WL_CAC(("AC delivery enabled\n"));
						AC_BITMAP_SET(scb->apsd.ac_delv, ac);
						AC_BITMAP_RESET(scb->apsd.ac_trig, ac);
						break;

					case (TS_INFO_BIDIRECTIONAL >> TS_INFO_DIRECTION_SHIFT):
						WL_CAC(("AC trigger and delivery enabled\n"));
						AC_BITMAP_SET(scb->apsd.ac_trig, ac);
						AC_BITMAP_SET(scb->apsd.ac_delv, ac);
						break;
					}
			}

			WL_CAC(("Inactivity interval %d\n",
				scb_acinfo->cac_ac[ac].inactivity_limit));
			WL_CAC(("Total medium time %d\n", scb_acinfo->cac_ac[ac].tot_medium_time));
			WL_CAC(("Available medium time %d\n", cac->available_medium_time));
			WL_CAC(("Tspec medium time %d\n", ts_param.medium_time));
		}

		ts->medium_time = tspec_medium_time;
		if (status == WME_ADMISSION_ACCEPTED) {
			ftcbparm->fbtric.ts_count += 1;
			ftcbparm->fbtric.ts = tlvs;
		}
	}
	return status;
}
#endif /* AP */
#endif /* WLFBT */
