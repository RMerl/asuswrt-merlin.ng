/*
 * 802.11h TPC and wl power control module source file
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
 * $Id: wlc_tpc.c 745136 2018-02-07 06:59:01Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [PowerControl]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLTPC
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_channel.h>
#include <wlc_clm.h>
#include <wlc_stf.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_TPC_RPT_OVERRIDE,
	IOV_AP_TPC_MODE,	/* ap tpc mode */
	IOV_AP_TPC_PERIOD,	/* ap tpc periodicity */
	IOV_AP_TPC_LM,		/* ap tpc link margins */
	IOV_CONSTRAINT,
	IOV_CURPOWER,
	IOV_TARGETPOWER_MAX,
	IOV_CHANSPEC_TXPWR_MAX,
	IOV_ADJ_EST_POWER,
	IOV_LAST
};

static const bcm_iovar_t wlc_tpc_iovars[] = {
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	{"tpc_rpt_override", IOV_TPC_RPT_OVERRIDE, (0), IOVT_UINT16, 0},
#endif // endif
#ifdef WL_AP_TPC
	{"tpc_mode", IOV_AP_TPC_MODE, (0), IOVT_UINT8, 0},
	{"tpc_period", IOV_AP_TPC_PERIOD, (0), IOVT_UINT8, 0},
	{"tpc_lm", IOV_AP_TPC_LM, (0), IOVT_UINT16, 0},
#endif // endif
	{"constraint", IOV_CONSTRAINT, (0), IOVT_UINT8, 0},
#if defined(WL_EXPORT_CURPOWER)
	{"curpower", IOV_CURPOWER, (IOVF_GET_UP), IOVT_UINT8, 0},
#endif // endif
	{"txpwr_target_max", IOV_TARGETPOWER_MAX, (IOVF_GET_UP), IOVT_BUFFER, 0},
#ifdef WL_CHANSPEC_TXPWR_MAX
	{"chanspec_txpwr_max", IOV_CHANSPEC_TXPWR_MAX, (0), IOVT_BUFFER, WL_CHANSPEC_TXPWR_MAX_LEN},
#endif /* WL_CHANSPEC_TXPWR_MAX */
	{"txpwr_adj_est", IOV_ADJ_EST_POWER, (IOVF_GET_UP), IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0}
};

/* ioctl table */
static const wlc_ioctl_cmd_t wlc_tpc_ioctls[] = {
	{WLC_SEND_PWR_CONSTRAINT, 0, sizeof(int)}
};

/* TPC module info */
struct wlc_tpc_info {
	wlc_info_t *wlc;
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	uint16 tpc_rpt_override;	/* overrides for tpc report. */
#endif // endif
#ifdef WL_AP_TPC
	uint8 ap_tpc;
	uint8 ap_tpc_interval;
	int scbh;			/* scb cubby handle */
	int cfgh;			/* bsscfg cubby handle */
#endif // endif
	int8 txpwr_local_max;		/* regulatory local txpwr max */
	uint8 txpwr_local_constraint;	/* local power contraint in dB */
	uint8 pwr_constraint;
};

/* local functions */
static int wlc_tpc_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_tpc_watchdog(void *ctx);
#ifdef BCMDBG
static int wlc_tpc_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static int wlc_tpc_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

#ifdef WL_AP_TPC
static int wlc_ap_tpc_scb_init(void *ctx, struct scb *scb);
static void wlc_ap_tpc_scb_deinit(void *ctx, struct scb *scb);
static int wlc_ap_tpc_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_ap_tpc_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_ap_tpc_req(wlc_tpc_info_t *tpc);
static int wlc_ap_bss_tpc_get(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg);
#ifdef BCMDBG
static void wlc_ap_tpc_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
static void wlc_ap_tpc_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_ap_tpc_scb_dump NULL
#define wlc_ap_tpc_bsscfg_dump NULL
#endif // endif
#endif /* WL_AP_TPC */

#ifdef WL_AP_TPC
static int wlc_ap_tpc_setup(wlc_tpc_info_t *tpc, uint8 mode);
static void wlc_ap_tpc_rpt_upd(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr,
	dot11_tpc_rep_t *rpt, int8 rssi, ratespec_t rspec);
#else
#define wlc_ap_tpc_setup(tpc, mode) BCME_OK
#define wlc_ap_tpc_rpt_upd(tpc, cfg, hdr, rpt, rssi, rspec) do {} while (0)
#endif /* !WL_AP_TPC */

#ifndef WLC_NET80211
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
static void wlc_tpc_rpt_ovrd(wlc_tpc_info_t *tpc, dot11_tpc_rep_t *rpt);
#else
#define wlc_tpc_rpt_ovrd(tpc, rpt) do {} while (0)
#endif // endif
#endif /* WLC_NET80211 */

#if defined(WL_EXPORT_CURPOWER)
static int wlc_tpc_get_current(wlc_tpc_info_t *tpc, void *pwr, uint len, wlc_bsscfg_t *bsscfg);
#else
#define wlc_tpc_get_current(tpc, pwr, len) BCME_ERROR
#endif // endif

static int wlc_tpc_get_txpwr_target_max(wlc_tpc_info_t *tpc, void *pwr, uint len);
static int wlc_tpc_get_txpwr_adj_est(wlc_tpc_info_t *tpc, void *pwr, uint len);
#ifdef WL_CHANSPEC_TXPWR_MAX
static int wlc_tpc_get_txpwr_max(wlc_info_t *wlc,
	wl_chanspec_txpwr_max_t *arg, wl_chanspec_txpwr_max_t *params);
#endif /* WL_CHANSPEC_TXPWR_MAX */

/* IE mgmt */
#ifdef AP
static uint wlc_tpc_calc_pwr_const_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_tpc_write_pwr_const_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_tpc_calc_rpt_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_tpc_write_rpt_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
static uint wlc_tpc_calc_brcm_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_tpc_write_brcm_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef STA
static int wlc_tpc_bcn_parse_pwr_const_ie(void *ctx, wlc_iem_parse_data_t *data);
#ifdef BCMDBG
static int wlc_tpc_bcn_parse_rpt_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
#endif /* STA */

#ifdef WL_AP_TPC
/* XXX allocate the struct and reserve a pointer to the struct in the scb
 * as the scb cubby when this structure grows larger than a pointer...
 */
typedef struct {
	int8 sta_link_margin;	/* STAs present link margin */
	int8 ap_link_margin;	/* APs present link margin */
} ap_tpc_scb_cubby_t;

#define AP_TPC_SCB_CUBBY(tpc, scb) ((ap_tpc_scb_cubby_t *)SCB_CUBBY(scb, (tpc)->scbh))

/* XXX allocate the struct and reserve a pointer to the struct in the bsscfg
 * as the bsscfg cubby when this structure grows larger than a pointer...
 */
typedef struct {
	int8 sta_link_margin;	/* STAs present link margin */
	int8 ap_link_margin;	/* APs present link margin */
} ap_tpc_bsscfg_cubby_t;

#define AP_TPC_BSSCFG_CUBBY(tpc, cfg) ((ap_tpc_bsscfg_cubby_t *)BSSCFG_CUBBY(cfg, (tpc)->cfgh))
#endif /* WL_AP_TPC */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_tpc_info_t *
BCMATTACHFN(wlc_tpc_attach)(wlc_info_t *wlc)
{
	wlc_tpc_info_t *tpc;
#ifdef AP
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif // endif

	if ((tpc = MALLOCZ(wlc->osh, sizeof(wlc_tpc_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	tpc->wlc = wlc;

#ifdef WL_AP_TPC
	/* reserve cubby in the scb container for per-scb private data */
	if ((tpc->scbh = wlc_scb_cubby_reserve(wlc, sizeof(ap_tpc_scb_cubby_t),
	                wlc_ap_tpc_scb_init, wlc_ap_tpc_scb_deinit, wlc_ap_tpc_scb_dump,
	                (void *)tpc)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((tpc->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(ap_tpc_bsscfg_cubby_t),
	                wlc_ap_tpc_bsscfg_init, wlc_ap_tpc_bsscfg_deinit, wlc_ap_tpc_bsscfg_dump,
	                (void *)tpc)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WL_AP_TPC */

	/* register IE mgmt callback */
	/* calc/build */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_PWR_CONSTRAINT_ID,
	            wlc_tpc_calc_pwr_const_ie_len, wlc_tpc_write_pwr_const_ie, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, pwr const in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_TPC_REPORT_ID,
	            wlc_tpc_calc_rpt_ie_len, wlc_tpc_write_rpt_ie, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, tpc rpt in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
	/* prbreq */
	if (wlc_iem_vs_add_build_fn(wlc->iemi, FC_PROBE_REQ, WLC_IEM_VS_IE_PRIO_BRCM_TPC,
	            wlc_tpc_calc_brcm_ie_len, wlc_tpc_write_brcm_ie, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn() failed, tpc rpt in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* parse */
#ifdef STA
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_PWR_CONSTRAINT_ID,
	                         wlc_tpc_bcn_parse_pwr_const_ie, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, pwr const in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef BCMDBG
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_TPC_REPORT_ID,
	                         wlc_tpc_bcn_parse_rpt_ie, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, tpc rpt in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
#endif /* STA */

#ifdef BCMDBG
	if (wlc_dump_register(wlc->pub, "tpc", wlc_tpc_dump, tpc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	if (wlc_module_register(wlc->pub, wlc_tpc_iovars, "tpc", tpc, wlc_tpc_doiovar,
	                        wlc_tpc_watchdog, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	if (wlc_module_add_ioctl_fn(wlc->pub, tpc, wlc_tpc_doioctl,
	                            ARRAYSIZE(wlc_tpc_ioctls), wlc_tpc_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return tpc;

	/* error handling */
fail:
	wlc_tpc_detach(tpc);
	return NULL;
}

void
BCMATTACHFN(wlc_tpc_detach)(wlc_tpc_info_t *tpc)
{
	wlc_info_t *wlc;

	if (tpc == NULL)
		return;

	wlc = tpc->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, tpc);
	wlc_module_unregister(wlc->pub, "tpc", tpc);

	MFREE(wlc->osh, tpc, sizeof(wlc_tpc_info_t));
}

#ifdef WL_CHANSPEC_TXPWR_MAX
static int
wlc_tpc_get_txpwr_max(wlc_info_t *wlc,
	wl_chanspec_txpwr_max_t *arg, wl_chanspec_txpwr_max_t *params)
{
	char abbrev[WLC_CNTRY_BUF_SZ] = {0};
	wl_uint32_list_t *list = NULL;
	uint16 alloc_len, cnt;
	chanspec_t chanspec;
	int err = 0;
	ppr_t *ppr_txpwr = NULL, *srommax = NULL;
	int8 min_srom;

	alloc_len = ((1 + WL_NUMCHANSPECS) * sizeof(uint32));

	list = (wl_uint32_list_t *)MALLOCZ(wlc->osh, alloc_len);
	if (list == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	chanspec = params->txpwr[0].chanspec;
	WL_INFORM(("input: chanspec: %x\n", chanspec));
	if (CHSPEC_IS2G(chanspec) || (chanspec == 0)) {

#ifdef WL11ULB
		if (ULB_ENAB(wlc->pub)) {
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_2P5, TRUE, abbrev);
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_5, TRUE, abbrev);
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_10, TRUE, abbrev);
				}
#endif /* WL11ULB */
		if (CHSPEC_IS20(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_20, TRUE, abbrev);
		if (CHSPEC_IS40(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_40, TRUE, abbrev);
	}
	if (CHSPEC_IS5G(chanspec) || (chanspec == 0)) {
#ifdef WL11ULB
			if (ULB_ENAB(wlc->pub)) {
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_2P5, TRUE, abbrev);
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_5, TRUE, abbrev);
				wlc_get_valid_chanspecs(wlc->cmi, list,
					WL_CHANSPEC_BW_10, TRUE, abbrev);
			}
#endif /* WL11ULB */
		if (CHSPEC_IS20(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_20, FALSE, abbrev);
		if (CHSPEC_IS40(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_40, FALSE, abbrev);
		if (CHSPEC_IS80(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_80, FALSE, abbrev);
		if (CHSPEC_IS160(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_160, FALSE, abbrev);
		if (CHSPEC_IS8080(chanspec) || (chanspec == 0))
			wlc_get_valid_chanspecs(wlc->cmi, list,
				WL_CHANSPEC_BW_8080, FALSE, abbrev);
	}

	arg->count = list->count;
	arg->len = WL_CHANSPEC_TXPWR_MAX_LEN;
	arg->ver = WL_CHANSPEC_TXPWR_MAX_VER;

	if ((ppr_txpwr = ppr_create(wlc->osh, ppr_get_max_bw())) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	if ((srommax = ppr_create(wlc->osh, ppr_get_max_bw())) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	for (cnt = 0; cnt < list->count; cnt++) {
		chanspec = (chanspec_t)(list->element[cnt]);

		ppr_clear(ppr_txpwr);
		ppr_clear(srommax);

		/* use the control channel to get the regulatory limits and srom max/min */
		wlc_channel_reg_limits(wlc->cmi, chanspec, ppr_txpwr);

		/* Fix core (last) argument */
		wlc_phy_txpower_sromlimit(WLC_PI(wlc), chanspec, (uint8*)&min_srom, srommax, 0);

		/* bound the regulatory limit by srom min/max */
		ppr_apply_vector_ceiling(ppr_txpwr, srommax);
		ppr_apply_min(ppr_txpwr, min_srom);

		arg->txpwr[cnt].chanspec = chanspec;
		arg->txpwr[cnt].txpwr_max = ppr_get_max(ppr_txpwr);
		WL_INFORM(("chspec:%x max: %d\n", chanspec, arg->txpwr[cnt].txpwr_max));
	}

done:
	if (ppr_txpwr)
		ppr_delete(wlc->osh, ppr_txpwr);
	if (srommax)
		ppr_delete(wlc->osh, srommax);
	if (list)
		MFREE(wlc->osh, list, alloc_len);

	return err;
}
#endif /* WL_CHANSPEC_TXPWR_MAX */

static int
wlc_tpc_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr = (int32 *)arg;	/* convenience int ptr for 4-byte gets
						 * (requires int aligned arg).
						 */
	bool bool_val;
	bool bool_val2;
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);

	BCM_REFERENCE(bsscfg);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(bool_val2);
	BCM_REFERENCE(ret_int_ptr);

	/* Do the actual parameter implementation */
	switch (actionid) {
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	case IOV_GVAL(IOV_TPC_RPT_OVERRIDE):
		*ret_int_ptr = (int32)tpc->tpc_rpt_override;
		break;
	case IOV_SVAL(IOV_TPC_RPT_OVERRIDE):
		tpc->tpc_rpt_override = (uint16)int_val;
		break;
#endif // endif
#ifdef WL_AP_TPC
	case IOV_GVAL(IOV_AP_TPC_MODE):
		*ret_int_ptr = tpc->ap_tpc;
		break;
	case IOV_SVAL(IOV_AP_TPC_MODE):
		err = wlc_ap_tpc_setup(tpc, (uint8)int_val);
		break;
	case IOV_GVAL(IOV_AP_TPC_PERIOD):
		*ret_int_ptr = tpc->ap_tpc_interval;
		break;
	case IOV_SVAL(IOV_AP_TPC_PERIOD):
		if (!WL11H_ENAB(wlc)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		tpc->ap_tpc_interval = (uint8)int_val;
		break;
	case IOV_GVAL(IOV_AP_TPC_LM):
		if (!tpc->ap_tpc) {
			err = BCME_EPERM;
			break;
		}

		*ret_int_ptr = wlc_ap_bss_tpc_get(tpc, bsscfg);
		break;
#endif /* WL_AP_TPC */
	case IOV_SVAL(IOV_CONSTRAINT):
		tpc->pwr_constraint = (uint8)int_val;
		if (wlc->pub->associated) {
			WL_APSTA_BCN(("wl%d: WLC_SEND_PWR_CONSTRAINT ->"
			              " wlc_update_beacon()\n", wlc->pub->unit));
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, TRUE);
		}
		break;
#if defined(WL_EXPORT_CURPOWER)
	case IOV_GVAL(IOV_CURPOWER):
		err = wlc_tpc_get_current(tpc, arg, len, bsscfg);
		break;
#endif // endif
	case IOV_GVAL(IOV_TARGETPOWER_MAX):
		err = wlc_tpc_get_txpwr_target_max(tpc, arg, len);
		break;
	case IOV_GVAL(IOV_ADJ_EST_POWER):
		err = wlc_tpc_get_txpwr_adj_est(tpc, arg, len);
		break;
#ifdef WL_CHANSPEC_TXPWR_MAX
	case IOV_GVAL(IOV_CHANSPEC_TXPWR_MAX):
	{
		wl_chanspec_txpwr_max_t txpwr_params;
		wl_chanspec_txpwr_max_t *txpwr_arg;
		uint16 max_len;

		/* to avoid unalign, copy input arguments */
		memcpy(&txpwr_params, params, sizeof(wl_chanspec_txpwr_max_t));

		if (((&txpwr_params)->ver != WL_CHANSPEC_TXPWR_MAX_VER) ||
			((&txpwr_params)->len != WL_CHANSPEC_TXPWR_MAX_LEN)) {
			WL_ERROR(("ver:%d (%d) len:%d(%zd)\n",
				(&txpwr_params)->ver, WL_CHANSPEC_TXPWR_MAX_VER,
				(&txpwr_params)->len, WL_CHANSPEC_TXPWR_MAX_LEN));
			err = BCME_VERSION;
			break;
		}

		/* check to make sure enough space for report */
		max_len = sizeof(wl_chanspec_txpwr_max_t);
		max_len += (sizeof(chanspec_txpwr_max_t) * (WL_NUMCHANSPECS - 1));
		if (len < max_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		txpwr_arg = (wl_chanspec_txpwr_max_t *)MALLOCZ(wlc->osh, max_len);
		if (txpwr_arg == NULL) {
			err = BCME_NOMEM;
			break;
		}

		err = wlc_tpc_get_txpwr_max(wlc, txpwr_arg, &txpwr_params);
		memcpy(arg, txpwr_arg, max_len);
		MFREE(wlc->osh, txpwr_arg, max_len);
		break;
	}
#endif /* WL_CHANSPEC_TXPWR_MAX */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wlc_tpc_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;
	int err = BCME_OK;

	switch (cmd) {
	case WLC_SEND_PWR_CONSTRAINT:
		err = wlc_iovar_op(wlc, "constraint", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_tpc_watchdog(void *ctx)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;

	(void)wlc;

#ifdef WL_AP_TPC
	if (tpc->ap_tpc_interval > 0 &&
	    (wlc->pub->now % tpc->ap_tpc_interval) == 0)
		wlc_ap_tpc_req(tpc);
#endif // endif
}

#ifdef BCMDBG
static int
wlc_tpc_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	bcm_bprintf(b, "tpc_rpt_override: %d\n", tpc->tpc_rpt_override);
#endif // endif

#ifdef WL_AP_TPC
	bcm_bprintf(b, "ap_tpc: %d ap_tpc_interval: %d\n", tpc->ap_tpc, tpc->ap_tpc_interval);
#endif // endif

	bcm_bprintf(b, "pwr_constraint %u txpwr_local_max %d txpwr_local_constraint %u\n",
	            tpc->pwr_constraint, tpc->txpwr_local_max, tpc->txpwr_local_constraint);

	return BCME_OK;
}
#endif /* BCMDBG */

#ifdef WL_AP_TPC
/* scb cubby */
static int
wlc_ap_tpc_scb_init(void *ctx, struct scb *scb)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

	wlc_ap_tpc_assoc_reset(tpc, scb);

	return BCME_OK;
}

static void
wlc_ap_tpc_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

	wlc_ap_tpc_assoc_reset(tpc, scb);
}

#ifdef BCMDBG
static void
wlc_ap_tpc_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	ap_tpc_scb_cubby_t *tpc_scb = AP_TPC_SCB_CUBBY(tpc, scb);

	ASSERT(tpc_scb != NULL);

	bcm_bprintf(b, "     ap_link_margin: %d sta_link_margin: %d\n",
	            tpc_scb->ap_link_margin, tpc_scb->sta_link_margin);
}
#endif // endif

/* bsscfg cubby */
static int
wlc_ap_tpc_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

	(void)tpc;

	return BCME_OK;
}

static void
wlc_ap_tpc_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

	(void)tpc;
}

#ifdef BCMDBG
static void
wlc_ap_tpc_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	ap_tpc_bsscfg_cubby_t *tpc_cfg = AP_TPC_BSSCFG_CUBBY(tpc, cfg);

	ASSERT(tpc_cfg != NULL);

	bcm_bprintf(b, "\tap_link_margin: %d sta_link_margin: %d\n",
	            tpc_cfg->ap_link_margin, tpc_cfg->sta_link_margin);
}
#endif // endif

void
wlc_ap_tpc_assoc_reset(wlc_tpc_info_t *tpc, struct scb *scb)
{
	ap_tpc_scb_cubby_t *tpc_scb = AP_TPC_SCB_CUBBY(tpc, scb);

	ASSERT(tpc_scb != NULL);

	/* set to max */
	tpc_scb->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
	tpc_scb->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;
}

static int
wlc_ap_bss_tpc_get(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg)
{
	ap_tpc_bsscfg_cubby_t *tpc_cfg = AP_TPC_BSSCFG_CUBBY(tpc, cfg);
	int tpc_val;

	ASSERT(tpc_cfg != NULL);

	tpc_val = (((uint16)tpc_cfg->ap_link_margin << 8) & 0xff00) |
		(tpc_cfg->sta_link_margin & 0x00ff);

	return tpc_val;
}

void
wlc_ap_bss_tpc_setup(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = tpc->wlc;
	ap_tpc_bsscfg_cubby_t *tpc_cfg = AP_TPC_BSSCFG_CUBBY(tpc, cfg);

	ASSERT(tpc_cfg != NULL);

	/* reset BSS pwr and AP pwr before enabling new mode. */
	tpc_cfg->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
	tpc_cfg->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;

	if (BSSCFG_AP(cfg) && cfg->up) {
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}
}

static int
wlc_ap_tpc_setup(wlc_tpc_info_t *tpc, uint8 mode)
{
	wlc_info_t *wlc = tpc->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	if (!WL11H_ENAB(wlc))
		return BCME_UNSUPPORTED;

	if (mode > 3)
		return BCME_RANGE;

	if (!tpc->ap_tpc_interval)
		tpc->ap_tpc_interval = 3;

	tpc->ap_tpc = mode;

	tpc->pwr_constraint = 0;

	FOREACH_BSS(wlc, idx, cfg) {
		wlc_ap_bss_tpc_setup(tpc, cfg);
	}

	tpc->txpwr_local_constraint = 0;
	if (wlc->pub->up && wlc->pub->associated &&
	    (wlc->chanspec == wlc->home_chanspec)) {
		uint8 qdbm = wlc_tpc_get_local_constraint_qdbm(tpc);
		if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP)
			qdbm -= wlc->band->antgain;
		wlc_channel_set_txpower_limit(wlc->cmi, qdbm);
	}

	return BCME_OK;
}

static void
wlc_ap_bss_tpc_req(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = tpc->wlc;
	struct scb_iter scbiter;
	struct scb *scb;

	if (BSSCFG_AP(cfg) && cfg->up) {

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			if (SCB_ASSOCIATED(scb) &&
			    (scb->cap & DOT11_CAP_SPECTRUM)) {
				/* send TPC request to all assoc STAs.
				 */
				if (!ETHER_ISMULTI(&scb->ea))
					wlc_send_tpc_request(tpc, cfg, &scb->ea);

				/* reset the margins when channel switch is
				 * pending.
				 */
				if (wlc_11h_get_spect_state(wlc->m11h, cfg) &
				    NEED_TO_SWITCH_CHANNEL) {
					ap_tpc_scb_cubby_t *tpc_scb = AP_TPC_SCB_CUBBY(tpc, scb);

					tpc_scb->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
					tpc_scb->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;
				}
			}
		}
	}
}

static void
wlc_ap_tpc_req(wlc_tpc_info_t *tpc)
{
	wlc_info_t *wlc = tpc->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	if (WL11H_ENAB(wlc) && tpc->ap_tpc) {

		FOREACH_BSS(wlc, idx, cfg) {
			wlc_ap_bss_tpc_req(tpc, cfg);
		}
	}
}

static void
wlc_ap_bss_tpc_upd(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = tpc->wlc;
	struct scb_iter scbiter;
	struct scb *scb;
	int8 sta_least_link_margin, ap_least_link_margin;

	if (BSSCFG_AP(cfg) && cfg->up) {
		ap_tpc_bsscfg_cubby_t *tpc_cfg = AP_TPC_BSSCFG_CUBBY(tpc, cfg);

		ASSERT(tpc_cfg != NULL);

		sta_least_link_margin = AP_TPC_MAX_LINK_MARGIN;
		ap_least_link_margin = AP_TPC_MAX_LINK_MARGIN;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			if (SCB_ASSOCIATED(scb) &&
			    (scb->cap & DOT11_CAP_SPECTRUM)) {
				ap_tpc_scb_cubby_t *tpc_scb = AP_TPC_SCB_CUBBY(tpc, scb);

				/* now record the least link margin
				 * from previous reports.
				 */
				if (sta_least_link_margin >= tpc_scb->sta_link_margin)
					sta_least_link_margin = tpc_scb->sta_link_margin;

				/* find the least link margin AP has
				 * with respect to all the associated
				 * STAs.
				 */
				if (ap_least_link_margin >= tpc_scb->ap_link_margin)
					ap_least_link_margin = tpc_scb->ap_link_margin;
			}
		}

		/* record the link margin info for record keeping. */
		tpc_cfg->sta_link_margin = sta_least_link_margin;
		tpc_cfg->ap_link_margin = ap_least_link_margin;
	}
}

static void
wlc_ap_tpc_upd(wlc_tpc_info_t *tpc)
{
	wlc_info_t *wlc = tpc->wlc;
	int idx;
	wlc_bsscfg_t *cfg;
	int8 sta_least_link_margin, ap_least_link_margin;

	if (WL11H_ENAB(wlc) && tpc->ap_tpc) {

		sta_least_link_margin = AP_TPC_MAX_LINK_MARGIN;
		ap_least_link_margin = AP_TPC_MAX_LINK_MARGIN;

		FOREACH_BSS(wlc, idx, cfg) {
			ap_tpc_bsscfg_cubby_t *tpc_cfg = AP_TPC_BSSCFG_CUBBY(tpc, cfg);

			ASSERT(tpc_cfg != NULL);

			wlc_ap_bss_tpc_upd(tpc, cfg);

			/* find the least link margin */
			if (sta_least_link_margin >= tpc_cfg->sta_link_margin)
				sta_least_link_margin = tpc_cfg->sta_link_margin;

			/* find the least link margin AP has
			 * with respect to all the associated
			 * STAs.
			 */
			if (ap_least_link_margin >= tpc_cfg->ap_link_margin)
				ap_least_link_margin = tpc_cfg->ap_link_margin;
		}

		/* reduce the AP power if stas have better link
		 * margin.
		 */
		if (tpc->ap_tpc == AP_TPC_AP_PWR || tpc->ap_tpc == AP_TPC_AP_BSS_PWR) {
			uint8 txpwr_local_constraint;

			/* now update the bcn and probe responses with new pwr
			 * constriant.
			 */
			if (sta_least_link_margin == AP_TPC_MAX_LINK_MARGIN) {
				txpwr_local_constraint = 0;
			} else if (sta_least_link_margin >= 9) {
				txpwr_local_constraint = 6;
			} else if (sta_least_link_margin >= 6) {
				txpwr_local_constraint = 3;
			} else {
				txpwr_local_constraint = 0;
			}

			WL_REGULATORY(("wl%d:%s STAs least link margin:%d "
				"txpwr_local_constraint:%d \n", wlc->pub->unit, __FUNCTION__,
				sta_least_link_margin, txpwr_local_constraint));

			if (txpwr_local_constraint != tpc->txpwr_local_constraint) {
				tpc->txpwr_local_constraint = txpwr_local_constraint;

				/* only update power targets if we are up and on the BSS
				 * home channel.
				 */
				if (wlc->chanspec == wlc->home_chanspec) {
					uint8 qdbm = wlc_tpc_get_local_constraint_qdbm(tpc);
					if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP)
						qdbm -= wlc->band->antgain;
					wlc_channel_set_txpower_limit(wlc->cmi, qdbm);
				}
			}
		}

		/* reduce the BS pwr based on how best link margin AP
		 * has.
		 */
		if (tpc->ap_tpc == AP_TPC_BSS_PWR || tpc->ap_tpc == AP_TPC_AP_BSS_PWR) {
			uint8 pwr_constraint;

			if ((ap_least_link_margin == AP_TPC_MAX_LINK_MARGIN))
				pwr_constraint = 0;
			else if (ap_least_link_margin >= 9)
				pwr_constraint = 6;
			else if (ap_least_link_margin >= 6)
				pwr_constraint = 3;
			else
				pwr_constraint = 0;

			WL_REGULATORY(("wl%d:%s APs least link margin:%d pwr_constraint:%d\n",
				wlc->pub->unit, __FUNCTION__,
				ap_least_link_margin, pwr_constraint));

			if (pwr_constraint != tpc->pwr_constraint) {
				tpc->pwr_constraint = pwr_constraint;

				wlc_update_beacon(wlc);
				wlc_update_probe_resp(wlc, TRUE);
			}
		}
	}
}

static void
wlc_ap_tpc_rpt_upd(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	dot11_tpc_rep_t *ie, int8 rssi, ratespec_t rspec)
{
	wlc_info_t *wlc = tpc->wlc;
	struct scb *scb;
	int nominal_req_pwr;
	uint8 reg_chan_pwr, cur_chan, txpwr_max;
	uint8 target_pwr;
	ap_tpc_scb_cubby_t *tpc_scb;

	nominal_req_pwr = wlc_find_nominal_req_pwr(rspec);

	cur_chan = wf_chspec_ctlchan(wlc->home_chanspec);

	reg_chan_pwr = wlc_get_reg_max_power_for_channel(wlc->cmi, cur_chan, TRUE);

	target_pwr = wlc_phy_txpower_get_target_max((wlc_phy_t *)WLC_PI(wlc));

	if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP)
		txpwr_max = (target_pwr + wlc->band->antgain) / WLC_TXPWR_DB_FACTOR;
	else
		txpwr_max = target_pwr / WLC_TXPWR_DB_FACTOR;

	WL_REGULATORY(("wl%d: %s: Nominal req pwr: %d RSSI of packet:%d current channel:%d "
		"regulatory pwr for channel:%d max tx pwr:%d\n",
		wlc->pub->unit, __FUNCTION__,
		nominal_req_pwr, rssi, cur_chan, reg_chan_pwr, txpwr_max));

	/* Record the STA link margin now */
	if ((scb = wlc_scbfind(wlc, cfg, &hdr->sa)) == NULL) {
		WL_INFORM(("did not find scb\n"));
		return;
	}

	tpc_scb = AP_TPC_SCB_CUBBY(tpc, scb);
	ASSERT(tpc_scb != NULL);

	/* record sta's link margin */
	tpc_scb->sta_link_margin = (int8)ie->margin + (reg_chan_pwr - txpwr_max);

	/* record ap's link margin */
	tpc_scb->ap_link_margin = rssi - nominal_req_pwr + (reg_chan_pwr - ie->tx_pwr);

	WL_REGULATORY(("wl%d:%s STAs link margin:%d APs link margin:%d\n", wlc->pub->unit,
	               __FUNCTION__, tpc_scb->sta_link_margin, tpc_scb->ap_link_margin));

	wlc_ap_tpc_upd(tpc);
}
#endif /* WL_AP_TPC */

#ifndef WLC_NET80211
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
static void
wlc_tpc_rpt_ovrd(wlc_tpc_info_t *tpc, dot11_tpc_rep_t *rpt)
{
	if (tpc->tpc_rpt_override != 0) {
		rpt->tx_pwr = (int8)((tpc->tpc_rpt_override >> 8) & 0xff);
		rpt->margin = (int8)(tpc->tpc_rpt_override & 0xff);
	}
}
#endif // endif

void
wlc_recv_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi, ratespec_t rspec)
{
	wlc_info_t *wlc = tpc->wlc;
	struct dot11_action_measure * action_hdr;
	struct ether_addr *ea = &hdr->sa;
#if defined(BCMDBG_ERR) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	(void)wlc;

	if (body_len < 3) {
		WL_ERROR(("wl%d: %s: got TPC Request from %s, but frame body len"
			" was %d, expected 3\n",
			wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(ea, eabuf), body_len));
		return;
	}

	action_hdr = (struct dot11_action_measure *)body;

	WL_INFORM(("wl%d: wlc_recv_tpc_request: got TPC Request (token %d) from %s\n",
	           wlc->pub->unit, action_hdr->token, bcm_ether_ntoa(ea, eabuf)));

	wlc_send_tpc_report(tpc, cfg, ea, action_hdr->token, rssi, rspec);
}

void
wlc_recv_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi, ratespec_t rspec)
{
	wlc_info_t *wlc = tpc->wlc;
	struct dot11_action_measure * action_hdr;
	int len;
	dot11_tpc_rep_t* rep_ie;
#if defined(BCMDBG) || (defined(BCMCONDITIONAL_LOGGING) && defined(WLMSG_DFS))
	char da[ETHER_ADDR_STR_LEN];
	char sa[ETHER_ADDR_STR_LEN];
	char bssid[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_DFS */

	(void)wlc;

	WL_REGULATORY(("Action Frame: DA %s SA %s BSSID %s\n",
	       bcm_ether_ntoa(&hdr->da, da), bcm_ether_ntoa(&hdr->sa, sa),
	       bcm_ether_ntoa(&hdr->bssid, bssid)));

	if (body_len < 3) {
		WL_INFORM(("Action frame body len was %d, expected > 3\n", body_len));
		return;
	}

	/* re-using action measure struct here also */
	action_hdr = (struct dot11_action_measure *)body;
	rep_ie = (dot11_tpc_rep_t*)action_hdr->data;
	len = body_len - DOT11_ACTION_MEASURE_LEN;

	WL_REGULATORY(("Action Frame: category %d action %d dialog token %d\n",
	       action_hdr->category, action_hdr->action, action_hdr->token));

	if (action_hdr->category != DOT11_ACTION_CAT_SPECT_MNG) {
		WL_INFORM(("Unexpected category, expected Spectrum Management %d\n",
			DOT11_ACTION_CAT_SPECT_MNG));
		return;
	}

	if (action_hdr->action != DOT11_SM_ACTION_TPC_REP) {
		WL_INFORM(("Unexpected action type (%d)\n", action_hdr->action));
		return;
	}

	if (len < 4) {
		WL_INFORM(("Malformed Action frame, less that an IE header length (4 bytes)"
			" remaining in buffer\n"));
		return;
	}

	if (rep_ie->id != DOT11_MNG_TPC_REPORT_ID) {
		WL_INFORM(("Unexpected IE (id %d len %d):\n", rep_ie->id, rep_ie->len));
		prhex(NULL, (uint8*)rep_ie + TLV_HDR_LEN, rep_ie->len);
		return;
	}

	if (rep_ie->len != 2) {
		WL_INFORM(("Unexpected TPC report IE len != 2\n"));
		return;
	}

	WL_REGULATORY(("%s (id %d len %d): tx_pwr:%d margin:%d\n", "TPC Report", rep_ie->id,
		rep_ie->len, (int8)rep_ie->tx_pwr, (int8)rep_ie->margin));

	wlc_ap_tpc_rpt_upd(wlc->tpc, cfg, hdr, rep_ie, rssi, rspec);
}

void
wlc_send_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da)
{
	wlc_info_t *wlc = tpc->wlc;
	void *p;
	uint8* pbody;
	uint8* end;
	uint body_len;
	struct dot11_action_measure * action_hdr;
	struct scb *scb = NULL;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending TPC Request to %s\n",
	           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);

	/* TPC Request frame is
	 * 3 bytes Action Measure Req frame
	 * 2 bytes empty TPC Request IE
	 */
	body_len = DOT11_ACTION_MEASURE_LEN + TLV_HDR_LEN;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for TPC Request\n",
		           wlc->pub->unit, __FUNCTION__));
		return;
	}

	action_hdr = (struct dot11_action_measure *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = DOT11_SM_ACTION_TPC_REQ;

	/* Token needs to be non-zero, so burn the high bit */
	action_hdr->token = (uint8)(wlc->counter | 0x80);
	end = bcm_write_tlv(DOT11_MNG_TPC_REQUEST_ID, NULL, 0, action_hdr->data);

	ASSERT((end - pbody) == (int)body_len);
	BCM_REFERENCE(end);

	if (!ETHER_ISMULTI(da)) {
		scb = wlc_scbfindband(wlc, cfg, da, CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
	}

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, scb);
}

void
wlc_send_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 token, int8 rssi, ratespec_t rspec)
{
	wlc_info_t *wlc = tpc->wlc;
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_measure * action_hdr;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	WL_INFORM(("wl%d: %s: sending TPC Report to %s\n",
	           wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(da, eabuf)));

	ASSERT(cfg != NULL);

	/* TPC Report frame is
	 * 3 bytes Action Measure Req frame
	 * 4 bytes TPC Report IE
	 */
	body_len = DOT11_ACTION_MEASURE_LEN + TLV_HDR_LEN + DOT11_MNG_IE_TPC_REPORT_LEN;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, da, &cfg->cur_etheraddr, &cfg->BSSID,
	                       body_len, &pbody);
	if (p == NULL) {
		WL_INFORM(("wl%d: %s: no memory for TPC Report\n",
		           wlc->pub->unit, __FUNCTION__));
		return;
	}

	action_hdr = (struct dot11_action_measure *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = DOT11_SM_ACTION_TPC_REP;
	action_hdr->token = token;

	wlc_tpc_rep_build(wlc, rssi, rspec, (dot11_tpc_rep_t *)&action_hdr->data[0]);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
}

void
wlc_tpc_rep_build(wlc_info_t *wlc, int8 rssi, ratespec_t rspec, dot11_tpc_rep_t *tpc_rep)
{
	int txpwr, link_margin;
	int nominal_req_pwr;
	uint8 target_pwr;
	uint8 txpwr_backoff = 0;

	target_pwr = wlc_phy_txpower_get_target_max((wlc_phy_t *)WLC_PI(wlc));

	txpwr_backoff = wlc_phy_get_txpwr_backoff((wlc_phy_t *)WLC_PI(wlc));
	/* tx power for the outgoing frame will be our current txpwr setting
	 * include the antenna gain value to get radiated power only for EIRP locales.
	 * Adjust from internal units to dbm.
	 */
	if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP) {
		txpwr = (target_pwr + txpwr_backoff + wlc->band->antgain) /
			WLC_TXPWR_DB_FACTOR;
	} else
		txpwr = (target_pwr + txpwr_backoff) / WLC_TXPWR_DB_FACTOR;

	nominal_req_pwr = wlc_find_nominal_req_pwr(rspec);

	link_margin = rssi;
	link_margin -= nominal_req_pwr;
	link_margin -= 3; /* TPC Report Safety Margin, 3 dB */
	/* clamp link_margin value if we overflow an int8 */
	link_margin = MIN(link_margin, 127);
	link_margin = MAX(link_margin, -128);

	tpc_rep->id = DOT11_MNG_TPC_REPORT_ID;
	tpc_rep->len = DOT11_MNG_IE_TPC_REPORT_LEN;
	tpc_rep->tx_pwr = (int8)txpwr;
	tpc_rep->margin = (int8)link_margin;

	wlc_tpc_rpt_ovrd(wlc->tpc, tpc_rep);

	WL_INFORM(("wl%d: wlc_build_tpc_report: TPC Report: txpwr %d, link margin %d\n",
		wlc->pub->unit, txpwr, link_margin));
}
#endif /* WLC_NET80211 */

#ifdef STA
#ifdef BCMDBG
static void
wlc_tpc_report(wlc_tpc_info_t *tpc, dot11_tpc_rep_t *ie)
{
	wlc_info_t *wlc = tpc->wlc;

	(void)wlc;

	if (ie->len < 2) {
		WL_ERROR(("wl%d: wlc_tpc_report: TPC Report IE len %d too short, expected 2.\n",
			wlc->pub->unit, ie->len));
		return;
	}

	WL_NONE(("wl%d: wlc_tpc_report: TPC Report TX Pwr %d dBm, Link Margin %d dB.\n",
		wlc->pub->unit, (int)ie->tx_pwr, (int)ie->margin));
}
#endif /* BCMDBG */

/* STA: Handle Power Constraint IE */
static void
wlc_tpc_set_local_constraint(wlc_tpc_info_t *tpc, dot11_power_cnst_t *pwr)
{
	wlc_info_t *wlc = tpc->wlc;
	uint8 local;

	if (pwr->len < 1)
		return;

	local = pwr->power;

	if (local == tpc->txpwr_local_constraint)
		return;

	WL_REGULATORY(("wl%d: adjusting local power constraint from %d to %d dBm\n",
		wlc->pub->unit, tpc->txpwr_local_constraint, local));

	tpc->txpwr_local_constraint = local;

	/* only update power targets if we are up and on the BSS home channel */
	if (wlc->pub->up && wlc->pub->associated &&
	    (wlc->chanspec == wlc->home_chanspec)) {
		uint8 constraint;

		/* Set the power limits for this locale after computing
		 * any 11h local tx power constraints.
		 */
		constraint = wlc_tpc_get_local_constraint_qdbm(tpc);
		wlc_channel_set_txpower_limit(wlc->cmi, constraint);

		/* wlc_phy_cal_txpower_recalc_sw(WLC_PI(wlc)); */
	}
}
#endif /* STA */

void
wlc_tpc_reset_all(wlc_tpc_info_t *tpc)
{
	wlc_info_t *wlc = tpc->wlc;

	(void)wlc;

	/* reset BSS local power limits */
	tpc->txpwr_local_max = WLC_TXPWR_MAX;

	/* Need to set the txpwr_local_max to external reg max for
	 * this channel as per the locale selected for AP.
	 */
#ifdef AP
	if (AP_ONLY(wlc->pub)) {
		uint8 chan =  wf_chspec_ctlchan(wlc->chanspec);
		tpc->txpwr_local_max =
		        wlc_get_reg_max_power_for_channel(wlc->cmi, chan, TRUE);
	}
#endif // endif

	tpc->txpwr_local_constraint = 0;
}

uint8
wlc_tpc_get_local_constraint_qdbm(wlc_tpc_info_t *tpc)
{
	wlc_info_t *wlc = tpc->wlc;
	uint8 local;
	int16 local_max;

	local = WLC_TXPWR_MAX;
	if ((tpc->txpwr_local_max != WL_RATE_DISABLED) && wlc->pub->associated &&
		(wf_chspec_ctlchan(wlc->chanspec) == wf_chspec_ctlchan(wlc->home_chanspec))) {

		/* get the local power constraint if we are on the AP's
		 * channel [802.11h, 7.3.2.13]
		 */
		/* Clamp the value between 0 and WLC_TXPWR_MAX w/o overflowing the target */
		local_max = tpc->txpwr_local_max * WLC_TXPWR_DB_FACTOR;
		local_max -= tpc->txpwr_local_constraint * WLC_TXPWR_DB_FACTOR;
		if (local_max > 0 && local_max < WLC_TXPWR_MAX)
			return (uint8)local_max;
		if (local_max < 0)
			return 0;
	}

	return local;
}

#ifdef AP
/* 802.11h Power Constraint */
static uint
wlc_tpc_calc_pwr_const_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(tpc->wlc, cfg) && BSSCFG_AP(cfg))
		return TLV_HDR_LEN + sizeof(uint8);

	return 0;
}

static int
wlc_tpc_write_pwr_const_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(tpc->wlc, cfg) && BSSCFG_AP(cfg)) {
		uint8 pwr = tpc->pwr_constraint;

		bcm_write_tlv(DOT11_MNG_PWR_CONSTRAINT_ID, &pwr, sizeof(pwr), data->buf);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_tpc_bcn_parse_pwr_const_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	/* too short ies are possible: guarded against here */
	if (data->ie != NULL && data->ie_len >= sizeof(dot11_power_cnst_t)) {
		wlc_tpc_set_local_constraint(tpc, (dot11_power_cnst_t *)data->ie);
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef AP
/* 802.11h TPC report */
static uint
wlc_tpc_calc_rpt_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(tpc->wlc, cfg) && BSSCFG_AP(cfg)) {
		return TLV_HDR_LEN + DOT11_MNG_IE_TPC_REPORT_LEN;
	}

	return 0;
}

static int
wlc_tpc_write_rpt_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSS_WL11H_ENAB(wlc, cfg) && BSSCFG_AP(cfg)) {
		int8 tpc_report[DOT11_MNG_IE_TPC_REPORT_LEN];
		uint8 target_pwr;

		target_pwr = wlc_phy_txpower_get_target_max((wlc_phy_t *)WLC_PI(wlc));

		/* 802.11H Sec 7.3.2.18 */
		/* Current target + ant_gain converted to dbm */
		if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP)
			tpc_report[0] = (target_pwr + wlc->band->antgain) / WLC_TXPWR_DB_FACTOR;
		else
			tpc_report[0] = target_pwr / WLC_TXPWR_DB_FACTOR;

		tpc_report[1] = 0;  /* Link Margin set to 0 */

		bcm_write_tlv(DOT11_MNG_TPC_REPORT_ID, tpc_report,
			DOT11_MNG_IE_TPC_REPORT_LEN, data->buf);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
#ifdef BCMDBG
static int
wlc_tpc_bcn_parse_rpt_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;

	if (data->ie != NULL)
		wlc_tpc_report(tpc, (dot11_tpc_rep_t *)data->ie);

	return BCME_OK;
}
#endif // endif
#endif /* STA */

static uint
wlc_tpc_calc_brcm_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;

	if (WL11K_ENAB(wlc->pub))
		return TLV_HDR_LEN + 8;
	return 0;
}

static int
wlc_tpc_write_brcm_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_tpc_info_t *tpc = (wlc_tpc_info_t *)ctx;
	wlc_info_t *wlc = tpc->wlc;

	if (WL11K_ENAB(wlc->pub)) {
		uint8 target_pwr = wlc_phy_txpower_get_target_max((wlc_phy_t *)WLC_PI(wlc));
		uint8 *cp = data->buf;

		*cp++ = DOT11_MNG_VS_ID;
		*cp++ = 8;
		bcopy(WPA_OUI, cp, WPA_OUI_LEN);
		cp += WPA_OUI_LEN;
		*cp++ = WFA_OUI_TYPE_TPC;
		*cp++ = 0; /* subtype = 0 */
		if (wlc_channel_locale_flags(wlc->cmi) & WLC_EIRP)
			*cp++ = (target_pwr + wlc->band->antgain) / WLC_TXPWR_DB_FACTOR;
		else
			*cp++ = target_pwr / WLC_TXPWR_DB_FACTOR;
		*cp++ = 0; /* link margin = 0 */
	}
	return BCME_OK;
}

static int
wlc_tpc_alloc_txpower_data(wlc_tpc_info_t *tpc, phy_tx_power_t **power, ppr_t **reg_limits)
{
	int err = 0;
	wlc_info_t *wlc = tpc->wlc;
	wl_tx_bw_t ppr_bw;
	phy_tx_power_t *phy_txpwr;

	if (!(phy_txpwr = (phy_tx_power_t*)MALLOCZ(wlc->osh, sizeof(phy_tx_power_t)))) {
		err = BCME_NOMEM;
		goto done;
	}

	ppr_bw = ppr_get_max_bw();
	if ((phy_txpwr->ppr_target_powers = ppr_create(wlc->osh, ppr_bw)) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}
	if ((phy_txpwr->ppr_board_limits = ppr_create(wlc->osh, ppr_bw)) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	if (PHYTYPE_HT_CAP(wlc->band)) {
		phy_txpwr->flags |= WL_TX_POWER_F_HT;
	}

	phy_txpwr->chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
	if (wlc->pub->associated)
		phy_txpwr->local_chanspec = wlc->home_chanspec;

	if ((*reg_limits = ppr_create(wlc->osh, PPR_CHSPEC_BW(phy_txpwr->chanspec))) == NULL) {
		err = BCME_ERROR;
		goto done;
	}

done:
	*power = phy_txpwr;

	return err;
}

static void
wlc_tpc_free_txpower_data(wlc_tpc_info_t *tpc, phy_tx_power_t *power, ppr_t *reg_limits)
{
	wlc_info_t *wlc = tpc->wlc;

	if (power) {
		if (power->ppr_board_limits)
			ppr_delete(wlc->osh, power->ppr_board_limits);
		if (power->ppr_target_powers)
			ppr_delete(wlc->osh, power->ppr_target_powers);
		MFREE(wlc->osh, power, sizeof(phy_tx_power_t));
	}
	if (reg_limits)
		ppr_delete(wlc->osh, reg_limits);
}

static int
wlc_tpc_get_txpwr_target_max(wlc_tpc_info_t *tpc, void *pwr, uint len)
{
	wlc_info_t *wlc = tpc->wlc;
	phy_tx_power_t *power = NULL;
	ppr_t *reg_limits = NULL;
	txpwr_target_max_t *target_pwr = (txpwr_target_max_t *)pwr;
	int err = 0;

	if (len < sizeof(txpwr_target_max_t))
		return BCME_BUFTOOSHORT;

	if (!pwr)
		return BCME_BADARG;

	err = wlc_tpc_alloc_txpower_data(tpc, &power, &reg_limits);

	if (err)
		goto done;

	wlc_channel_reg_limits(wlc->cmi, power->chanspec, reg_limits);

	wlc_phy_txpower_get_current(WLC_PI(wlc), reg_limits, power);

	target_pwr->rf_cores = power->rf_cores;
	target_pwr->version = TXPWR_TARGET_VERSION;
	target_pwr->chanspec = power->chanspec;
	memcpy(target_pwr->txpwr, power->tx_power_max, sizeof(target_pwr->txpwr));

done:
	wlc_tpc_free_txpower_data(tpc, power, reg_limits);

	return err;
}

static int
wlc_tpc_get_txpwr_adj_est(wlc_tpc_info_t *tpc, void *pwr, uint len)
{
	wlc_info_t *wlc = tpc->wlc;
	phy_tx_power_t *power = NULL;
	ppr_t *reg_limits = NULL;
	txpwr_adj_est_t *adj_est_pwr = (txpwr_adj_est_t *)pwr;
	int err = 0;

	if (len < sizeof(txpwr_adj_est_t)) {
		return BCME_BUFTOOSHORT;
	}

	if (!pwr) {
		return BCME_BADARG;
	}

	err = wlc_tpc_alloc_txpower_data(tpc, &power, &reg_limits);

	if (err) {
		goto done;
	}

	wlc_channel_reg_limits(wlc->cmi, power->chanspec, reg_limits);

	wlc_phy_txpower_get_current(WLC_PI(wlc), reg_limits, power);

	adj_est_pwr->rf_cores = power->rf_cores;
	adj_est_pwr->version = TXPWR_ADJ_EST_VERSION;
	adj_est_pwr->chanspec = power->chanspec;
	memcpy(adj_est_pwr->estpwr, &power->est_Pout_act, 4);

done:
	wlc_tpc_free_txpower_data(tpc, power, reg_limits);

	return err;
}
#if defined(WL_EXPORT_CURPOWER)
static int
wlc_tpc_get_current(wlc_tpc_info_t *tpc, void *pwr, uint len, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = tpc->wlc;
	phy_tx_power_t *power = NULL;
	tx_power_legacy_t *old_power    = NULL;
	bool override;
	ppr_t *reg_limits = NULL;
	uint8 *pserbuf;
	tx_pwr_rpt_t *pwr_to_wl = (tx_pwr_rpt_t*)pwr;

	chanspec_t chanspec = wlc->chanspec;
	/* convert chanspec into CLM units */
	clm_bandwidth_t bw = CLM_BW_20;
	int err = 0;
	uint8 local_constraint;
	bool hwtable_txpwr = TRUE;
#if defined(WLTXPWR1_SIGNED)
	int8 user_target;
#endif // endif

	if (pwr_to_wl->version != TX_POWER_T_VERSION) {
		WL_ERROR(("wlc_tpc_get_current: version mismatch - wl executable %d,"
				  " driver was expecting %d\n",
				  pwr_to_wl->version, TX_POWER_T_VERSION));
		return BCME_VERSION;
	}

	ASSERT(WL_BW_20MHZ == CLM_BW_20);
	ASSERT(WL_BW_40MHZ == CLM_BW_40);
#ifdef WL11AC
	ASSERT(WL_BW_80MHZ == CLM_BW_80);
	ASSERT(WL_BW_160MHZ == CLM_BW_160);
	ASSERT(WL_BW_8080MHZ == CLM_BW_80_80);
#endif // endif
	if (CHSPEC_IS40(chanspec))
		bw = CLM_BW_40;
#ifdef WL11AC
	else if (CHSPEC_IS80(chanspec))
		bw = CLM_BW_80;
	else if (CHSPEC_IS160(chanspec))
		bw = CLM_BW_160;
	else if (CHSPEC_IS8080(chanspec))
		bw = CLM_BW_80_80;
#endif // endif

	BCM_REFERENCE(chanspec);

	ASSERT(!len || pwr != NULL);
	if (len == sizeof(tx_power_legacy_t)) {
		old_power = (tx_power_legacy_t*)pwr;
		pwr_to_wl = (tx_pwr_rpt_t*)MALLOC(wlc->osh, sizeof(*pwr_to_wl));
		if (!pwr_to_wl)
			return BCME_NOMEM;
	} else if (len < sizeof(*pwr_to_wl) + pwr_to_wl->ppr_len * WL_TXPPR_SER_BUF_NUM) {
		/* Buffer size should be at least structure size + 3 ppr serialization blocks */
		return BCME_BUFTOOSHORT;
	}

	err = wlc_tpc_alloc_txpower_data(tpc, &power, &reg_limits);

	if (err)
		goto free_power;

	pwr_to_wl->version = TX_POWER_T_VERSION;

	/* Tell wlu.c if we're using 20MHz or 40MHz. */
	/* This only works if we only support 20MHz and 40MHz bandwidths. */
	pwr_to_wl->channel_bandwidth = bw;

	/* Return the user target tx power limits for the various rates. */
#if defined(WLTXPWR1_SIGNED)
	wlc_phy_txpower_get(WLC_PI(wlc), &user_target, &override);
	pwr_to_wl->user_target = (uint)user_target;
#else
	wlc_phy_txpower_get(WLC_PI(wlc), &pwr_to_wl->user_target, &override);
#endif // endif

	pwr_to_wl->local_max = tpc->txpwr_local_max * WLC_TXPWR_DB_FACTOR;
	pwr_to_wl->local_constraint = tpc->txpwr_local_constraint * WLC_TXPWR_DB_FACTOR;

	pwr_to_wl->antgain[0] = wlc->bandstate[BAND_2G_INDEX]->antgain;
	pwr_to_wl->antgain[1] = wlc->bandstate[BAND_5G_INDEX]->antgain;

	local_constraint = wlc_tpc_get_local_constraint_qdbm(tpc);
	wlc_channel_reg_limits(wlc->cmi, power->chanspec, reg_limits);
	if (!AP_ONLY(wlc->pub)) {
		ppr_apply_constraint_total_tx(reg_limits, local_constraint);
	}
	wlc_phy_txpower_get_current(WLC_PI(wlc), reg_limits, power);
	/* copy the tx_ppr_t struct to the return buffer,
	 * or convert to a tx_power_legacy_t struct
	 */
	if (old_power) {
		uint r;
		bzero(old_power, sizeof(tx_power_legacy_t));

		old_power->txpwr_local_max = pwr_to_wl->local_max;

		for (r = 0; r < NUM_PWRCTRL_RATES; r++) {
			old_power->txpwr_band_max[r] = (uint8)pwr_to_wl->user_target;
		}

	} else {
		uint8 idx;
		int8 tssisen_min[PHY_MAX_CORES];
		uint8 txcore = 1;
		memset(tssisen_min, (int8)WL_RATE_DISABLED, sizeof(tssisen_min));
		wlc_phy_get_tssi_sens_min(WLC_PI(wlc), tssisen_min);
		pwr_to_wl->flags = power->flags;
		pwr_to_wl->chanspec = power->chanspec;
		pwr_to_wl->local_chanspec = power->local_chanspec;
		pwr_to_wl->display_core = power->display_core;
		pwr_to_wl->rf_cores = power->rf_cores;
		memcpy(&pwr_to_wl->est_Pout_act, &power->est_Pout_act, 4);
		pwr_to_wl->est_Pout_cck = power->est_Pout_cck;
		memcpy(&pwr_to_wl->tx_power_max, &power->tx_power_max, 4);
		memcpy(&pwr_to_wl->tx_power_max_rate_ind, &power->tx_power_max_rate_ind, 4);
		pwr_to_wl->last_tx_ratespec = wlc_get_rspec_history(bsscfg);
#ifdef WL11N
		txcore = wlc_stf_txcore_get(wlc, pwr_to_wl->last_tx_ratespec);
#endif // endif
#ifdef WL_SARLIMIT
		memcpy(&pwr_to_wl->SARLIMIT, &power->SARLIMIT, MAX_STREAMS_SUPPORTED);
#else
		memset(&pwr_to_wl->SARLIMIT, WLC_TXPWR_MAX, MAX_STREAMS_SUPPORTED);
#endif // endif
		memset(pwr_to_wl->target_offsets, (int8)WL_RATE_DISABLED,
			sizeof(pwr_to_wl->target_offsets));
		memcpy(&pwr_to_wl->est_Pout, &power->est_Pout, 4);

		if (PHYTYPE_MIMO_CAP(wlc->band->phytype) &&
			!(WLCISNPHY(wlc->band) && (wlc->pub->sromrev <= 8)))
			hwtable_txpwr = FALSE;

		if (!hwtable_txpwr)
		{
			uint8 offset = 0;
			txpwr204080_t txpwrs;
			ratespec_t rspec = pwr_to_wl->last_tx_ratespec;
			wl_tx_chains_t txbf_chains = 0;
			if (RSPEC_ISTXBF(rspec)) {
				txbf_chains =  wlc_ratespec_nss(rspec) +
					((rspec & RSPEC_TXEXP_MASK) >> RSPEC_TXEXP_SHIFT);
			} else {
				uint txexp, nsts;
				wl_tx_chains_t chains;
				/* For non-TxBF rates, need to check if the ratespec has correct
				 * expension set, if not, work it out by current chains and nsts.
				 */
				chains = wlc_stf_txchain_get(wlc, pwr_to_wl->last_tx_ratespec);
				nsts = wlc_ratespec_nsts(pwr_to_wl->last_tx_ratespec);
				txexp = RSPEC_TXEXP(pwr_to_wl->last_tx_ratespec);
				if ((txexp + nsts) != chains) {
					txexp = chains - nsts;
					pwr_to_wl->last_tx_ratespec &= ~RSPEC_TXEXP_MASK;
					pwr_to_wl->last_tx_ratespec |= (txexp << RSPEC_TXEXP_SHIFT);
				}

			}
			if ((err = wlc_stf_get_204080_pwrs(wlc,
				pwr_to_wl->last_tx_ratespec, &txpwrs, txbf_chains)) != BCME_OK) {
				goto free_power;
			}
			offset = txpwrs.pbw[((rspec & RSPEC_BW_MASK) >> RSPEC_BW_SHIFT) -
				BW_20MHZ][RSPEC_ISTXBF(rspec)]*2;

			/* Handle arbitrary core masks */
			for (idx = 0; (uint)(1<<idx) <= wlc->stf->valid_txchain_mask; idx++) {
				if ((txcore >> idx) & 0x1) {
					int8 current_target = pwr_to_wl->tx_power_max[idx] - offset;
					pwr_to_wl->target_offsets[idx] = offset;
					/* If open loop Tx power ctrl is on, we should ignore
					 * the est.Power
					 */
					if (tssisen_min[idx] != (int8)WL_RATE_DISABLED &&
						current_target < tssisen_min[idx]) {
						pwr_to_wl->est_Pout[idx] = current_target;
						pwr_to_wl->flags |= WL_TX_POWER_F_OPENLOOP;
					}
				}
			}
		} else {
			uint8 txpwr = get_pwr_from_targets(wlc, pwr_to_wl->last_tx_ratespec,
				power->ppr_target_powers);
			for (idx = 0; (uint) idx < WLC_BITSCNT(wlc->stf->hw_txchain); idx++) {
				if ((txcore >> idx) & 0x1 && txpwr != (uint8)WL_RATE_DISABLED) {
					pwr_to_wl->target_offsets[idx] =
						pwr_to_wl->tx_power_max[idx] - txpwr;
				}
			}
		}

		pserbuf = pwr_to_wl->pprdata;
		ppr_convert_to_tlv(power->ppr_board_limits, ppr_chanspec_bw(chanspec),
			pserbuf, pwr_to_wl->ppr_len, WLC_BITSCNT(wlc->stf->hw_txchain));
		pserbuf += pwr_to_wl->ppr_len;
		ppr_convert_to_tlv(power->ppr_target_powers, ppr_chanspec_bw(chanspec),
			pserbuf, pwr_to_wl->ppr_len, WLC_BITSCNT(wlc->stf->hw_txchain));
		pserbuf += pwr_to_wl->ppr_len;
		ppr_convert_to_tlv(reg_limits, ppr_chanspec_bw(chanspec), pserbuf,
			pwr_to_wl->ppr_len, WLC_BITSCNT(wlc->stf->hw_txchain));
	}
free_power:
	wlc_tpc_free_txpower_data(tpc, power, reg_limits);
	return err;
}
#endif /* WL_EXPORT_CURPOWER */

/* accessors */
void
wlc_tpc_set_local_max(wlc_tpc_info_t *tpc, uint8 pwr)
{
	tpc->txpwr_local_max = pwr;
}

#endif /* WLTPC */
