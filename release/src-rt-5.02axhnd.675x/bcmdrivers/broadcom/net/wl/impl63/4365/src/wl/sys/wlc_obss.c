/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 Networking Device Driver
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
 * $Id$
 */

/**
 * @file
 * @brief
 * XXX Twiki: [ObssBw]
 */

#include <wlc_cfg.h>
#include <wlc_types.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <d11.h>
#include <wlc_bsscfg.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wlc_ap.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>

#include <wlc_scb.h>
#include <wlc_obss.h>
#include <wlc_11h.h>
#include <wlc_csa.h>
#include <wlc_bmac.h>
#include <wlc_ht.h>
#include <proto/802.11.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif /* WLMCHAN */
#include <wlc_prot_n.h>

static uint8 wlc_ht_coex_ie_chk(wlc_info_t *wlc, bcm_tlv_t *tlv);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_obss(wlc_obss_info_t *obss, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP */
static void wlc_ht_coex_trigger_chk(wlc_bsscfg_t *cfg);
static int wlc_ht_upd_coex_bits(wlc_bsscfg_t *cfg, uint8 bits, uint8 mask);

static int
wlc_ht_send_action_obss_coex(wlc_bsscfg_t *cfg, uint8 coex_bits, uint8 *coex_map);
static uint8*
wlc_write_obss_intol_chanlist_ie(wlc_info_t *wlc, uint8* cp, uint8 len,
	uint8 *coex_map);
static uint8*
wlc_write_obss_coex_ie(uint8* cp, uint8 coex_bits);
static uint wlc_ht_calc_obss_scan_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_write_obss_scan_ie(void *ctx, wlc_iem_build_data_t *data);
static void wlc_ht_chanlist_2g_init(wlc_obss_info_t *obss);
static uint
wlc_get_BSSintol_2Gchanlist_len(wlc_info_t *wlc, uint8 *coex_map);
#ifdef WL11N
static uint8
wlc_obss_get_num_chan(wlc_obss_info_t *obss);
static void wlc_ht_obss_scanparam_init(obss_params_t *params);
static void wlc_ht_obss_scanparams_hostorder(wlc_info_t *wlc, obss_params_t *param,
	bool host_order);
static void
wlc_obss_set_coex_enab(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg, bool setting);
#else
#define wlc_ht_obss_scanparam_init(a)
#define wlc_ht_obss_scanparams_hostorder(a, b, c)  do {(void)(a);} while (0)
#define wlc_obss_set_coex_enab(a, b, c)  do {BCM_REFERENCE(b), BCM_REFERENCE(c);} while (0)
#endif /* WL11N */
static bool
wlc_ht_obss_scanparams_upd(wlc_bsscfg_t *cfg, obss_params_t *obss_param);

static int
wlc_obss_bss_init(void *context, wlc_bsscfg_t *cfg);

static void
wlc_obss_bss_deinit(void *context, wlc_bsscfg_t *cfg);

static void
wlc_obss_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);

static int
wlc_obss_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int alen, int vsize, struct wlc_if *wlcif);

static void wlc_obss_watchdog(void *ctx);
#ifdef STA
static void wlc_ht_obss_scan_timer(wlc_bsscfg_t *cfg);
#endif /* STA */
static void wlc_ht_obss_coex_timeout(wlc_bsscfg_t *cfg);
static int8
wlc_obss_get_switch_bw_deferred(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg);
#ifdef AP
static uint32
wlc_obss_cfg_get_tr_delay(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg);
static void
wlc_ht_coex_switch_bw(wlc_bsscfg_t *cfg, bool downgrade, uint8 reason_code);
static bool
wlc_ht_ap_coex_tea_chk(wlc_bsscfg_t *cfg, ht_cap_ie_t *cap_ie);

static int
wlc_ht_ap_coex_scan_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data);

static void
wlc_ht_ap_coex_tebc_proc(wlc_bsscfg_t *cfg);

static int
wlc_ht_ap_coex_bcn_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data);

static bool
wlc_ht_ap_coex_ted_chk(wlc_bsscfg_t *cfg, bcm_tlv_t *bss_chanlist_tlv, uint8 coex_bits);

static uint32
wlc_obss_cfg_get_fid_time(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg);

#ifdef WL11N
static void
wlc_obss_scb_state_upd(bcm_notif_client_data ctx, bcm_notif_server_data scb_data);

static void
wlc_coex_hdl_40_intol_sta(wlc_obss_info_t *obss, wlc_bsscfg_t *bsscfg);
#endif /* WL11N */
#else
#define wlc_ht_coex_switch_bw(a, b, c) do { } while (0)
#define wlc_ht_ap_coex_tea_chk(a, b) 0
#define wlc_ht_ap_coex_tebc_proc(a) do {} while (0)
#define wlc_ht_ap_coex_ted_chk(a, b, c) 0
#endif /* AP */

#define WLC_WIDTH20_DET(wlc, cfg) ((wlc_obss_cfg_get_coex_det((wlc)->obss, (cfg)) & \
	WL_COEX_WIDTH20) != 0)
#define WLC_INTOL40_OVRD(wlc, cfg) ((wlc_obss_cfg_get_coex_ovrd((wlc)->obss, (cfg)) & \
		WL_COEX_40MHZ_INTOLERANT) != 0)
#define WLC_WIDTH20_OVRD(wlc, cfg) ((wlc_obss_cfg_get_coex_ovrd((wlc)->obss, (cfg)) & \
		WL_COEX_WIDTH20) != 0)

#define COEX_MASK_TEA	0x1
#define COEX_MASK_TEB	0x2
#define COEX_MASK_TEC	0x4
#define COEX_MASK_TED	0x8

	/* coex bw downgrade reason code */
#define COEX_UPGRADE_TIMER 	0
#define COEX_DGRADE_TEA		1
#define COEX_DGRADE_40INTOL	2
#define COEX_DGRADE_TEBC	3
#define COEX_DGRADE_TED		4

static uint8
wlc_obss_cfg_get_coex_ovrd(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg);

/* global */
struct wlc_obss_info {
	wlc_info_t *wlc;
	int			bss_handle;		/* per bsscfg cubby */
	uint8		num_chan;		/* num of 2G channels */
	uint8		chanvec[OBSS_CHANVEC_SIZE]; /* bitvec of channels in 2G */
	uint8		coex_map[CH_MAX_2G_CHANNEL + 1]; /* channel map of coexist in 2G */
};

/* per bsscfg cubby */
typedef struct wlc_obss_bss_info {
	bool		coex_enab;		/* 20/40 MHz BSS Management enabled */
	bool		coex_permit;		/* 20/40 operation permitted */
	uint8		coex_det;		/* 40 MHz Intolerant device detected */
	uint8		coex_ovrd;		/* 40 MHz Intolerant device detected */
	bool		switch_bw_deferred;	/* defer the switch */
	uint8		coex_bits_buffered;	/* buffer coexistence bits due to scan */
	uint16		scan_countdown;		/* timer for schedule next OBSS scan */
	uint32		fid_time;		/* time when 40MHz intolerant device detected */
	uint32		coex_te_mask;		/* mask for trigger events */
	obss_params_t	params;			/* Overlapping BSS scan parameters */
} wlc_obss_bss_info_t;

struct wlc_obss_bss_cubby {
	wlc_obss_bss_info_t *obss_bss_info;
};

#define BSS_OBSS_CUBBY(obss, cfg) \
	(struct wlc_obss_bss_cubby*)(BSSCFG_CUBBY((cfg), (obss)->bss_handle))
#define BSS_OBSS_INFO(obss, cfg) (BSS_OBSS_CUBBY(obss, cfg))->obss_bss_info

#define WL_OBSS_IOVAR_START 0
enum {
	IOV_OBSS_COEX_ACTIVE = WL_OBSS_IOVAR_START,
	IOV_OBSS_COEX = WL_OBSS_IOVAR_START + 1,
	IOV_OBSS_COEX_ACTION = WL_OBSS_IOVAR_START + 2,
	IOV_OBSS_TE_MASK = WL_OBSS_IOVAR_START + 3,
	IOV_OBSS_SCAN_PARAMS = WL_OBSS_IOVAR_START + 4,
	IOV_40_INTOLERANT = WL_OBSS_IOVAR_START + 5,
	IOV_OBSS_WIDTH20 = WL_OBSS_IOVAR_START + 6,
	IOV_OBSS_WIDTH20_DET = WL_OBSS_IOVAR_START + 7,

	IOV_OBSS_LAST
};

static const bcm_iovar_t obss_iovars[] = {
	{"obss_scan_params", IOV_OBSS_SCAN_PARAMS,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(wl_obss_scan_arg_t)
	},
	{"obss_coex", IOV_OBSS_COEX,
	(IOVF_OPEN_ALLOW), IOVT_INT32, 0
	},
	{"intol40", IOV_40_INTOLERANT,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"obss_coex_action", IOV_OBSS_COEX_ACTION,
	(IOVF_SET_UP|IOVF_OPEN_ALLOW), IOVT_BUFFER, 0
	},
	{"obss_te_mask", IOV_OBSS_TE_MASK,
	(0), IOVT_INT32, 0
	},
	{"obss_coex_enab", IOV_OBSS_COEX_ACTIVE,
	(0), IOVT_BOOL, 0
	},
	{"obss_width20", IOV_OBSS_WIDTH20,
	(0), IOVT_BOOL, 0
	},
	{"obss_width20_det", IOV_OBSS_WIDTH20_DET,
	(0), IOVT_BOOL, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_obss_info_t *
BCMATTACHFN(wlc_obss_attach)(wlc_info_t *wlc)
{
	wlc_obss_info_t *obss = NULL;
#ifdef AP
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
#endif /* AP */
	uint16 obssfstbmp =
		FT2BMP(FC_BEACON) |
		FT2BMP(FC_PROBE_RESP) |
#ifdef AP
		FT2BMP(FC_ASSOC_RESP) |
		FT2BMP(FC_REASSOC_RESP) |
#endif // endif
		0;

	/* allocate private states */
	if ((obss = (wlc_obss_info_t*)
	    MALLOCZ(wlc->osh, sizeof(wlc_obss_info_t))) == NULL) {
		goto fail;
	}
	obss->wlc = wlc;
	if (wlc_iem_add_build_fn_mft(wlc->iemi, obssfstbmp, DOT11_MNG_HT_OBSS_ID,
	      wlc_ht_calc_obss_scan_ie_len, wlc_ht_write_obss_scan_ie, obss) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, obss ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, obss_iovars, "obss", obss, wlc_obss_doiovar,
		wlc_obss_watchdog, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	obss->bss_handle = wlc_bsscfg_cubby_reserve(wlc, sizeof(struct wlc_obss_bss_cubby),
		wlc_obss_bss_init, wlc_obss_bss_deinit, wlc_obss_bss_dump, (void *)obss);

	if (obss->bss_handle < 0) {
		WL_ERROR(("wl%d:%s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "obss", (dump_fn_t)wlc_dump_obss, (void *)obss);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_HT_CAP,
		wlc_ht_ap_coex_scan_parse_cap_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ht cap in scan\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, WLC_IEM_FC_AP_BCN, DOT11_MNG_HT_CAP,
		wlc_ht_ap_coex_bcn_parse_cap_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ht cap in bcn\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WL11N
	if (wlc_scb_state_upd_register(wlc,
		wlc_obss_scb_state_upd, (void*)obss) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scb_state_upd_register failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WL11N */
#endif /* AP */

	return obss;

fail:
	if (obss != NULL) {
		MFREE(wlc->osh, obss, sizeof(wlc_obss_info_t));
	}
	return NULL;
}

void
BCMATTACHFN(wlc_obss_detach)(wlc_obss_info_t *obss)
{
	wlc_info_t *wlc;

	if (obss == NULL) {
		return;
	}
	wlc = obss->wlc;
#ifdef AP
#ifdef WL11N
	wlc_scb_state_upd_unregister(wlc, wlc_obss_scb_state_upd, (void*)obss);
#endif /* WL11N */
#endif /* AP */
	wlc_module_unregister(wlc->pub, "obss", obss);

	MFREE(wlc->osh, obss, sizeof(wlc_obss_info_t));
}

/* OBSS Scan */
static uint
wlc_ht_calc_obss_scan_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;

	if (!data->cbparm->ht)
		return 0;
	if (!COEX_ACTIVE(cfg->wlc->obss, cfg))
		return 0;

	return TLV_HDR_LEN + DOT11_OBSS_SCAN_IE_LEN;
}

static int
wlc_ht_write_obss_scan_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_obss_info_t *obss = (wlc_obss_info_t *)ctx;
	wlc_info_t *wlc = obss->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	obss_params_t params;

	if (!data->cbparm->ht)
		return BCME_OK;
	if (!COEX_ACTIVE(obss, cfg))
		return BCME_OK;

	/* need to convert to 802.11 little-endian format */
	bcopy((uint8 *)&(BSS_OBSS_INFO(obss, cfg)->params),
		(uint8 *)&params, WL_OBSS_SCAN_PARAM_LEN);

	/* convert params to 802.11 network order */
	wlc_ht_obss_scanparams_hostorder(wlc, &params, FALSE);

	bcm_write_tlv(DOT11_MNG_HT_OBSS_ID, &params, DOT11_OBSS_SCAN_IE_LEN, data->buf);

	/* Support for HT Information 20/40MHz Exchange */
	return BCME_OK;
}

static int
wlc_obss_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif)
{
	wlc_obss_bss_info_t *obss;
	int err = BCME_OK;
	wlc_obss_info_t *obssi = (wlc_obss_info_t *)context;
	wlc_info_t *wlc = obssi->wlc;
	int32 int_val = 0, int_val2 = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *bsscfg;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	obss = BSS_OBSS_INFO(obssi, bsscfg);

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)),
			&int_val2, sizeof(int_val));

	switch (actionid) {
		case IOV_GVAL(IOV_OBSS_COEX):
			*ret_int_ptr = (int8)wlc->pub->_coex;
			break;
		case IOV_SVAL(IOV_OBSS_COEX):
			if (!N_ENAB(wlc->pub))
				err = BCME_UNSUPPORTED;

			if ((int_val != AUTO) &&
				(int_val != OFF) &&
				(int_val != ON)) {
				err = BCME_RANGE;
				break;
			}
			if (wlc->pub->_coex == int_val)
				break;

			wlc_ht_update_coex_support(wlc, (int8)int_val);
			break;
		case IOV_GVAL(IOV_40_INTOLERANT):
			*ret_int_ptr = WLC_INTOL40_OVRD(wlc, bsscfg) ? 1 : 0;
			break;

		case IOV_SVAL(IOV_40_INTOLERANT):
		{
			uint8 bit = int_val ? WL_COEX_40MHZ_INTOLERANT : 0;

			if (!N_ENAB(wlc->pub)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			err = wlc_ht_upd_coex_bits(bsscfg, bit, WL_COEX_40MHZ_INTOLERANT);
			break;
		}

		case IOV_SVAL(IOV_OBSS_COEX_ACTION):
		{
			wl_action_obss_coex_req_t *req = (wl_action_obss_coex_req_t *)params;
			uint8 coex_map[CH_MAX_2G_CHANNEL + 1], i;

			if (!N_ENAB(wlc->pub)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			if (SCAN_IN_PROGRESS(wlc->scan)) {
				err = BCME_ERROR;
				break;
			}

			if (req->num > ARRAYSIZE(coex_map)) {
				err = BCME_BUFTOOLONG;
				break;
			}

			if (req->num) {
				/* chanlist_len include IE hdr + 1 or more BSS intolerant
				 * Channel Report IE.
				 */
				bzero(coex_map, CH_MAX_2G_CHANNEL + 1);
				for (i = 0; i < req->num; i++) {
					if ((req->ch_list[i] > CH_MAX_2G_CHANNEL) ||
						(req->ch_list[i] == 0))
						return BCME_BADBAND;
					coex_map[req->ch_list[i]] = 1;
				}
			}

			err = wlc_ht_send_action_obss_coex(bsscfg, req->info,
				(req->num ? coex_map : NULL));
			break;
		}

		case IOV_GVAL(IOV_OBSS_TE_MASK):
			*ret_int_ptr = obss->coex_te_mask;
			break;

		case IOV_SVAL(IOV_OBSS_TE_MASK):
			obss->coex_te_mask = int_val;
			break;

		case IOV_GVAL(IOV_OBSS_COEX_ACTIVE):
			*ret_int_ptr = (int8)(obss->coex_enab);
			break;

		case IOV_GVAL(IOV_OBSS_WIDTH20_DET):
			*ret_int_ptr = WLC_WIDTH20_DET(wlc, bsscfg) ? 1 : 0;
			break;

		case IOV_GVAL(IOV_OBSS_WIDTH20):
			*ret_int_ptr = WLC_WIDTH20_OVRD(wlc, bsscfg) ? 1 : 0;
			break;
#ifdef BCMDBG
		case IOV_SVAL(IOV_OBSS_WIDTH20):
		{
			uint8 bit = int_val ? WL_COEX_WIDTH20 : 0;

			if (!N_ENAB(wlc->pub) || !COEX_ENAB(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			err = wlc_ht_upd_coex_bits(bsscfg, bit, WL_COEX_WIDTH20);
			break;
		}
#endif /* BCMDBG */
		case IOV_GVAL(IOV_OBSS_SCAN_PARAMS):
		{
			obss_params_t *obss_params;

			if (len < (int)WL_OBSS_SCAN_PARAM_LEN) {
				err = BCME_BUFTOOSHORT;
				break;
			}
			obss_params = &obss->params;
			bcopy((uint8 *)obss_params, (uint8 *)arg, WL_OBSS_SCAN_PARAM_LEN);
			break;
		}

		case IOV_SVAL(IOV_OBSS_SCAN_PARAMS):
		{
			obss_params_t obss_params;

			if (BSSCFG_STA(bsscfg)) {
				err = BCME_NOTAP;
				break;
			}

			if (!N_ENAB(wlc->pub) || !COEX_ENAB(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			if (len < (int)WL_OBSS_SCAN_PARAM_LEN) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			bcopy(arg, (uint8 *)&obss_params, WL_OBSS_SCAN_PARAM_LEN);
			if (!wlc_ht_obss_scanparams_upd(bsscfg, &obss_params)) {
				err = BCME_BADARG;
				break;
			}

			if (bsscfg->associated) {
				wlc_bss_update_beacon(wlc, bsscfg);
				wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);
			}
			break;
		}
		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

void
wlc_recv_public_coex_action(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh)
{
	uint8 *tlvs;
	uint tlvs_len;
	bcm_tlv_t *obss_coex_tlv;
	uint8 coex_bits;
	int rx_bandunit;
	struct scb *scb;
	chanspec_t chspec = wlc->home_chanspec;
	int idx;
	wlc_bsscfg_t *cfg;
	bcm_tlv_t *obss_chanlist_tlv;

	if (!N_ENAB(wlc->pub))
		return;

	/* check validity of action frame before update */
	tlvs_len = body_len - DOT11_ACTION_HDR_LEN;
	tlvs = (uint8 *)body + DOT11_ACTION_HDR_LEN;

	obss_coex_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_HT_BSS_COEXINFO_ID);
	coex_bits = wlc_ht_coex_ie_chk(wlc, obss_coex_tlv);

	WL_COEX(("wl%d: %s: OBSS Coexistence Public Action frame received, Coex"
	         " Info bits 0x%02x\n", wlc->pub->unit, __FUNCTION__, coex_bits));

	/* update coexistence bits for current channel */
	if (coex_bits) {
		uint8 chan;

		chan = wf_chspec_ctlchan(chspec);

		if (CHSPEC_IS2G(chspec) && chan <= CH_MAX_2G_CHANNEL)
			wlc->obss->coex_map[chan] = coex_bits & ~WL_COEX_INFO_REQ;
	}

	/* need to notify the current AP, 40-intolerant detected
	 * from a STA or non-associated AP
	 */
	if ((coex_bits & WL_COEX_40MHZ_INTOLERANT) &&
	    !SCAN_IN_PROGRESS(wlc->scan)) {
		FOREACH_AS_STA(wlc, idx, cfg) {
		        if (!COEX_ACTIVE(wlc->obss, cfg))
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !_wlc_mchan_ovlp_chan(wlc->mchan, cfg, chspec, CH_20MHZ_APART))
				continue;
#endif // endif
			wlc_ht_send_action_obss_coex(cfg, WL_COEX_WIDTH20, NULL);
		}
	}

	/* AP received Trigger event B (TE-B) */
	if ((coex_bits & WL_COEX_40MHZ_INTOLERANT) &&
	    (wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da) != NULL || ETHER_ISMULTI(&hdr->da))) {
		FOREACH_UP_AP(wlc, idx, cfg) {
			if (!COEX_ACTIVE(wlc->obss, cfg))
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !_wlc_mchan_ovlp_chan(wlc->mchan, cfg, chspec, CH_20MHZ_APART))
				continue;
#endif // endif
			wlc_ht_ap_coex_tebc_proc(cfg);
		}
	}

	/* AP received Trigger event C (TE-C) */
	if (coex_bits & WL_COEX_WIDTH20) {
		rx_bandunit = CHSPEC_BANDUNIT(wrxh->rxhdr.RxChan);
		if ((scb = wlc_scbfindband(wlc, bsscfg, &hdr->sa, rx_bandunit)) != NULL &&
		    SCB_ASSOCIATED(scb) &&
		    (cfg = SCB_BSSCFG(scb)) != NULL &&
		    BSSCFG_AP(cfg) && cfg->associated &&
		    COEX_ACTIVE(wlc->obss, cfg))
			wlc_ht_ap_coex_tebc_proc(cfg);
	}

	/* AP process the channel list received. This is Trigger Event D (TE-D) */
	obss_chanlist_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_HT_BSS_CHANNEL_REPORT_ID);
	(void)obss_chanlist_tlv;
	if (wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da) != NULL || ETHER_ISMULTI(&hdr->da)) {
		FOREACH_UP_AP(wlc, idx, cfg) {
			if (!COEX_ACTIVE(wlc->obss, cfg))
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !_wlc_mchan_ovlp_chan(wlc->mchan, cfg, chspec, CH_20MHZ_APART))
				continue;
#endif // endif
			if (!wlc_ht_ap_coex_ted_chk(cfg, obss_chanlist_tlv, coex_bits))
				continue;
			wlc_ht_coex_update_permit(cfg, FALSE);
			wlc_ht_coex_update_fid_time(cfg);
			if (!CHSPEC_IS40(chspec))
				continue;
			wlc_ht_coex_switch_bw(cfg, TRUE, COEX_DGRADE_TED);
		}
	}

	/* reply if INFO_REQ bit is set and from associated AP */
	if (coex_bits & WL_COEX_INFO_REQ) {
		FOREACH_AS_STA(wlc, idx, cfg) {
			if (!COEX_ACTIVE(wlc->obss, cfg))
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !_wlc_mchan_ovlp_chan(wlc->mchan, cfg, chspec, CH_20MHZ_APART))
				continue;
#endif // endif
			coex_bits = 0;
			if (WLC_WIDTH20_DET(wlc, cfg))
				coex_bits |= WL_COEX_WIDTH20;
			if (WLC_INTOL40_DET(wlc, cfg))
				coex_bits |= WL_COEX_40MHZ_INTOLERANT;
			wlc_ht_send_action_obss_coex(cfg, coex_bits, NULL);
		}
	}

	return;
}

static uint8
wlc_ht_coex_ie_chk(wlc_info_t *wlc, bcm_tlv_t *obss_coex_tlv)
{
	dot11_obss_coex_t *obss_coex;
	uint8 coex_bits = 0;

	WL_TRACE(("wl%d: wlc_ht_coex_ie_chk\n", wlc->pub->unit));

	if (!obss_coex_tlv) {
		WL_ERROR(("wl%d: %s: 20/40 BSS Coexistence IE NOT found\n", wlc->pub->unit,
			__FUNCTION__));
		return coex_bits;
	}

	if (obss_coex_tlv->len < DOT11_OBSS_COEXINFO_LEN) {
		WL_ERROR(("wl%d: %s: Invalid 20/40 BSS Coexistence IE len %d\n", wlc->pub->unit,
			__FUNCTION__, obss_coex_tlv->len));
		return coex_bits;
	}

	obss_coex = (dot11_obss_coex_t *)obss_coex_tlv;

	if (obss_coex->info & DOT11_OBSS_COEX_40MHZ_INTOLERANT)
		coex_bits |= WL_COEX_40MHZ_INTOLERANT;
	if (obss_coex->info & DOT11_OBSS_COEX_20MHZ_WIDTH_REQ)
		coex_bits |= WL_COEX_WIDTH20;
	if (obss_coex->info & DOT11_OBSS_COEX_INFO_REQ)
		coex_bits |= WL_COEX_INFO_REQ;

	return coex_bits;
}

static void
wlc_ht_coex_enab(wlc_bsscfg_t *cfg, int8 setting)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_obss_info_t *obssi = wlc->obss;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(obssi, cfg);
	bool old_obss_coex_support;

	WL_TRACE(("wl%d.%d: wlc_ht_coex_enab\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

	if (!N_ENAB(wlc->pub))
		return;

	old_obss_coex_support = obss->coex_enab;

	if (setting == ON ||
	    (setting == AUTO && WL_BW_CAP_40MHZ(wlc->bandstate[BAND_2G_INDEX]->bw_cap)))
		obss->coex_enab = TRUE;
	else {
		obss->coex_enab = FALSE;
		if (cfg->associated) {
			wlc_ht_upd_coex_bits(cfg, 0, WL_COEX_WIDTH20);
		}
	}

	if (old_obss_coex_support == obss->coex_enab)
		return;

	WL_COEX(("wl%d.%d: %s: coex [%s] coex_enab %d\n",
	         wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
	         (setting == AUTO ? "AUTO" : (setting == ON ? "ON" : "OFF")),
	         obss->coex_enab));

	if (COEX_ACTIVE(wlc->obss, cfg)) {
		wlc_ht_coex_update_fid_time(cfg);
		wlc_ht_obss_scan_reset(cfg);
		/* update scan param first before update beacon, scan
		 * param is part of beacon IE
		 */
		wlc_ht_obss_scanparam_init(&obss->params);
		wlc_ht_chanlist_2g_init(obssi);
	}

	/* update extend capabilities */
	wlc_bsscfg_set_ext_cap(cfg, DOT11_EXT_CAP_OBSS_COEX_MGMT,
		COEX_ACTIVE(wlc->obss, cfg));
	if (cfg->up && wlc->clk &&
	    (BSSCFG_AP(cfg) || !cfg->BSS)) {
		/* update AP or IBSS beacons */
		wlc_bss_update_beacon(wlc, cfg);
		/* update AP or IBSS probe responses */
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}
}

void
wlc_ht_update_coex_support(wlc_info_t *wlc, int8 setting)
{
	int idx;
	wlc_bsscfg_t *cfg;

	WL_TRACE(("wl%d: wlc_ht_update_coex_support\n", wlc->pub->unit));

	if (!N_ENAB(wlc->pub))
		return;

	wlc->pub->_coex = setting;
	if (!wlc->obss) {
		return;
	}
	FOREACH_BSS(wlc, idx, cfg) {
		wlc_obss_set_coex_enab(wlc->obss, cfg, setting);
	}
}

bool
wlc_ht_obss_scanparams_upd(wlc_bsscfg_t *cfg, obss_params_t *new_params)
{
	wlc_info_t *wlc = cfg->wlc;
	obss_params_t *params;

	(void)wlc;

	WL_TRACE(("wl%d: wlc_ht_obss_scanparams_upd\n", wlc->pub->unit));

	params = &(BSS_OBSS_INFO(wlc->obss, cfg)->params);

	/* check if params are different */
	if (!bcmp((uint8 *)params, (uint8 *)new_params, DOT11_OBSS_SCAN_IE_LEN))
		return TRUE;

	if (new_params->passive_dwell < WLC_OBSS_SCAN_PASSIVE_DWELL_MIN ||
	    new_params->passive_dwell > WLC_OBSS_SCAN_PASSIVE_DWELL_MAX)
		return FALSE;

	if (new_params->active_dwell < WLC_OBSS_SCAN_ACTIVE_DWELL_MIN ||
	    new_params->active_dwell > WLC_OBSS_SCAN_ACTIVE_DWELL_MAX)
		return FALSE;

	if (new_params->bss_widthscan_interval < WLC_OBSS_SCAN_WIDTHSCAN_INTERVAL_MIN ||
	    new_params->bss_widthscan_interval > WLC_OBSS_SCAN_WIDTHSCAN_INTERVAL_MAX)
		return FALSE;

	if (new_params->chanwidth_transition_dly < WLC_OBSS_SCAN_CHANWIDTH_TRANSITION_DLY_MIN ||
	    new_params->chanwidth_transition_dly > WLC_OBSS_SCAN_CHANWIDTH_TRANSITION_DLY_MAX)
		return FALSE;

	if (new_params->passive_total < WLC_OBSS_SCAN_PASSIVE_TOTAL_PER_CHANNEL_MIN ||
	    new_params->passive_total > WLC_OBSS_SCAN_PASSIVE_TOTAL_PER_CHANNEL_MAX)
		return FALSE;

	if (new_params->active_total < WLC_OBSS_SCAN_ACTIVE_TOTAL_PER_CHANNEL_MIN ||
	    new_params->active_total > WLC_OBSS_SCAN_ACTIVE_TOTAL_PER_CHANNEL_MAX)
		return FALSE;

	if (new_params->activity_threshold > WLC_OBSS_SCAN_ACTIVITY_THRESHOLD_MAX)
		return FALSE;

	bcopy((uint8 *)new_params, (uint8 *)params, WL_OBSS_SCAN_PARAM_LEN);
	if (BSSCFG_STA(cfg))
		wlc_ht_obss_scan_reset(cfg);
	return TRUE;
}

#ifdef STA
static void
wlc_ht_obss_scan_timer(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(wlc->obss, cfg);
	wl_scan_params_t params;
	uint16 scan_interval;
	int err = 1;

	WL_TRACE(("wl%d: wlc_ht_obss_scan_timer\n", wlc->pub->unit));

	/* start obss scan only if condition meet */
	if (!obss->scan_countdown ||
	    !cfg->associated ||
	    !cfg->BSS)
		return;

	/* start obss scan only if join 11n BSS */
	if ((current_bss->flags & (WLC_BSS_HT | WLC_BSS_40MHZ)) != (WLC_BSS_HT | WLC_BSS_40MHZ))
		return;

	if (--obss->scan_countdown != 0)
		return;

	bzero((uint8 *)&params, WL_SCAN_PARAMS_FIXED_SIZE);
	memcpy(&params.bssid, &ether_bcast, ETHER_ADDR_LEN);
	params.bss_type = DOT11_BSSTYPE_ANY;
	params.scan_type = 0;
	params.nprobes = -1;
	params.active_time = obss->params.active_dwell;
	params.passive_time = obss->params.passive_dwell;
	params.home_time = -1;
	params.channel_num = 0;

	WL_COEX(("wl%d: %s: schedule a custom scan\n", wlc->pub->unit, __FUNCTION__));

	scan_interval = obss->params.bss_widthscan_interval;

	if (!ANY_SCAN_IN_PROGRESS(wlc->scan)) {
		err = wlc_custom_scan(wlc, (char *)&params, WL_SCAN_PARAMS_FIXED_SIZE,
		                      0, WLC_ACTION_SCAN, cfg);
		if (err) {
			scan_interval = 5;
			WL_COEX(("wl%d: %s: OBSS scan failed [%d], delay for %dsec\n",
				wlc->pub->unit, __FUNCTION__, err, scan_interval));
		} else {
			scan_interval = obss->params.bss_widthscan_interval;
		}
	} else {
		scan_interval = 5;
		WL_COEX(("wl%d: %s: SCAN_IN_PROGRESS, reschedule %dsec\n", wlc->pub->unit,
			__FUNCTION__, scan_interval));
	}
	WL_COEX(("wl%d: %s: arm next scan period %dsec\n", wlc->pub->unit, __FUNCTION__,
		scan_interval));
	obss->scan_countdown = scan_interval;
}
#endif /* STA */

void
wlc_ht_coex_exclusion_range(wlc_info_t *wlc, uint8 *ch_min, uint8 *ch_max, uint8 *ch_ctl)
{
	chanspec_t chanspec = wlc->home_chanspec;
	uint8 center_ch;

	if (!CHSPEC_IS2G(chanspec))
		return;

	if (CHSPEC_IS40(chanspec))
		chanspec = wlc->home_chanspec;
	/* if AP has been downgraded from bw40, use
	   default.
	*/
	else if (CHSPEC_IS40(wlc->default_bss->chanspec))
		chanspec = wlc->default_bss->chanspec;
	else {
		/* we don't want to do 40 anyway ...  */
		*ch_min = 0;
		*ch_max = 0;
		*ch_ctl = 0;
		return;
	}

	center_ch = CHSPEC_CHANNEL(chanspec);
	if (center_ch < WLC_2G_25MHZ_OFFSET) {
		*ch_min = 1;
		*ch_max = center_ch + WLC_2G_25MHZ_OFFSET;
	} else {
		*ch_min = center_ch - WLC_2G_25MHZ_OFFSET;
		*ch_max = center_ch + WLC_2G_25MHZ_OFFSET;
	}
	*ch_ctl = wf_chspec_ctlchan(chanspec);
}

/*
 * XXX : this function should take a channel array and a coex bitmap
 *  directly and decide if there is either a trigger to be notified to
 * the peer AP, if we are a STA, or if the update timer should be reset,
 * if we are an AP.
 * This is more or less duplicated by the wlc_ht_upd_coex_map in wlc_ap.c
 */

static void
wlc_ht_coex_trigger_chk(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	uint8 coex_bits = 0;
	chanspec_t chanspec = wlc->home_chanspec;
	uint8 center_ch, chan;

	(void)wlc;

	/* after a successful scan, both AP/STA enter this function to
	 * update the coexistence statemachine
	 */
	WL_TRACE(("wl%d: wlc_ht_coex_trigger_chk\n", wlc->pub->unit));

	if (!CHSPEC_IS2G(chanspec))
		return;

	if (!CHSPEC_IS40(chanspec) && BSSCFG_AP(cfg)) {
		/* if AP has been downgraded from bw40 need to continue
		 * process channel exclusion
		 */
		if (CHSPEC_IS40(wlc->default_bss->chanspec))
			chanspec = wlc->default_bss->chanspec;
	}

	ASSERT(COEX_ACTIVE(wlc->obss, cfg));

	/* check for trigger event, with updated obss.coex_map */
	if (N_ENAB(wlc->pub)) {
		center_ch = CHSPEC_CHANNEL(chanspec);
		if (CHSPEC_IS40(chanspec))
			chan = wf_chspec_ctlchan(chanspec);
		else
			chan = CHSPEC_CHANNEL(chanspec);

		ASSERT(chan <= CH_MAX_2G_CHANNEL);
		/* 40 Intolerant bit on current channel */
		if (wlc->obss->coex_map[chan] & WL_COEX_40MHZ_INTOLERANT)
			coex_bits |= WL_COEX_40MHZ_INTOLERANT;

		if (CHSPEC_IS40(chanspec)) {
			uint8 i, min_chan, max_chan;

			if (center_ch < WLC_2G_25MHZ_OFFSET) {
				min_chan = 1;
				max_chan = center_ch + WLC_2G_25MHZ_OFFSET;
			} else {
				min_chan = center_ch - WLC_2G_25MHZ_OFFSET;
				max_chan = center_ch + WLC_2G_25MHZ_OFFSET;
			}
			WL_COEX(("wl%d: %s: center chan %d; exclusion range %d ~ %d\n",
				wlc->pub->unit, __FUNCTION__, center_ch, min_chan, max_chan));

			for (i = 0; i <= CH_MAX_2G_CHANNEL; i++) {
				/* check exclusion channel range */
				if ((i >= min_chan) && (i <= max_chan)) {
					if (wlc->obss->coex_map[i] & WL_COEX_WIDTH20)
						coex_bits |= WL_COEX_WIDTH20;
				}
			}
		}
		WL_COEX(("wl%d: %s: exclusion chan %d coex_bits 0x%x\n", wlc->pub->unit,
			__FUNCTION__, chan, coex_bits));

		wlc_ht_upd_coex_state(cfg, coex_bits);
	}
}

void
wlc_ht_coex_filter_scanresult(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_bss_info_t *bi;
	uint chan, indx;

	WL_TRACE(("wl%d: wlc_ht_coex_filter_scanresult\n", wlc->pub->unit));

	if (!CHSPEC_IS2G(wlc->home_chanspec))
		return;
	if (!cfg->associated)
		return;

	bzero(wlc->obss->coex_map, CH_MAX_2G_CHANNEL + 1);
	/* inspect all BSS descriptor */
	for (indx = 0; indx < wlc->scan_results->count; indx++) {
		bi = wlc->scan_results->ptrs[indx];
		ASSERT(bi);
		if (CHSPEC_IS40(bi->chanspec))
			chan = wf_chspec_ctlchan(bi->chanspec);
		else
			chan = CHSPEC_CHANNEL(bi->chanspec);
		if (chan <= 0 || chan > CH_MAX_2G_CHANNEL) {
			WL_INFORM(("wl%d: %s: channel %d out side of 2.4G band\n", wlc->pub->unit,
				__FUNCTION__, chan));
			continue;
		}

		ASSERT(chan <= CH_MAX_2G_CHANNEL);
		if (bi) {
			if (!(bi->flags & WLC_BSS_HT) || !(bi->flags & WLC_BSS_40MHZ))
				/* legacy beacon || chan width 20 only */
				wlc->obss->coex_map[chan] |= WL_COEX_WIDTH20;
			if (bi->flags &	WLC_BSS_40INTOL)	/* 40 intolerant BSS */
				wlc->obss->coex_map[chan] |= WL_COEX_40MHZ_INTOLERANT;
			WL_COEX(("wl%d: %s: chan %d flag 0x%x\n", wlc->pub->unit, __FUNCTION__,
				chan, wlc->obss->coex_map[chan]));
		}
	}
	wlc_ht_coex_trigger_chk(cfg);
}

static int
wlc_ht_upd_coex_bits(wlc_bsscfg_t *cfg, uint8 coex_bits, uint8 mask)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(wlc->obss, cfg);

	/* Changes from IOVARs */
	WL_TRACE(("wl%d: wlc_ht_upd_coex_bits\n", wlc->pub->unit));

	if (coex_bits & ~mask)
		return BCME_RANGE;

	obss->coex_ovrd &= ~mask;
	if (coex_bits)
		obss->coex_ovrd |= coex_bits & mask;

	WL_COEX(("wl%d: %s: update coex 0x%x\n", wlc->pub->unit, __FUNCTION__, coex_bits));

	if (BSSCFG_AP(cfg) && wlc->clk) {
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	} else if (cfg->associated) {
		wlc_ht_upd_coex_state(cfg, coex_bits);
	}

	return 0;
}

static void
wlc_ht_obss_coex_timeout(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(wlc->obss, cfg);

	(void)wlc;

	WL_TRACE(("wl%d: wlc_ht_obss_coex_watchdog\n", wlc->pub->unit));

	/* obss coexistence not triggered, no check needed */
	if (!obss->coex_det)
		return;

	/* process switch_bw_deferred first before processing timeout */
	if (obss->switch_bw_deferred) {
		uint8 detected;
		ASSERT(obss->coex_bits_buffered);
		detected = obss->coex_bits_buffered;
		obss->coex_bits_buffered = 0;
		obss->switch_bw_deferred = FALSE;
		WL_COEX(("wl%d: %s: switch_bw_deferred, buffered 0x%x\n", wlc->pub->unit,
		         __FUNCTION__, detected));
		wlc_ht_upd_coex_state(cfg, detected);
		/* don't move on until switch_bw_deferred is cleared */
		if (obss->switch_bw_deferred)
			return;
	}
}

void
wlc_ht_upd_coex_state(wlc_bsscfg_t *cfg, uint8 detected)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(wlc->obss, cfg);

	chanspec_t cur_chanspec = wlc->home_chanspec;
	uint8 coex_det = 0;

	WL_TRACE(("wl%d: wlc_ht_upd_coex_state\n", wlc->pub->unit));

	if (!CHSPEC_IS2G(cur_chanspec) || !COEX_ACTIVE(wlc->obss, cfg))
		return;

	/* if scan in progress, schedule coexistence check for later.
	 * the code below may require of channel bw switching and is
	 * not good do it in the middle of scan
	 */
	if (detected && SCAN_IN_PROGRESS(wlc->scan)) {
		WL_COEX(("wl%d: %s: rescheduled coexistence check\n",
		         wlc->pub->unit, __FUNCTION__));
		obss->switch_bw_deferred = TRUE;
		/* there may be multiple call to this function during scan */
		obss->coex_bits_buffered |= detected;
		return;
	}

	WL_COEX(("wl%d: %s: check for changes to current coex state, incoming coex flag 0x%x\n",
		wlc->pub->unit, __FUNCTION__, detected));

	if (detected)
		coex_det |= WL_COEX_WIDTH20;

	if (WLC_INTOL40_OVRD(wlc, cfg))
		coex_det |= WL_COEX_40MHZ_INTOLERANT;

	WL_COEX(("wl%d: %s: result coex_det 0x%02x\n", wlc->pub->unit, __FUNCTION__, coex_det));

	obss->coex_det = coex_det;

	/* check if STA need to send report to AP */

	if (cfg->associated) {
		uint8 coex_bits = 0;

		if (WLC_INTOL40_DET(wlc, cfg))
			coex_bits |= WL_COEX_40MHZ_INTOLERANT;
		if (WLC_WIDTH20_DET(wlc, cfg))
			coex_bits |= WL_COEX_WIDTH20;

		WL_COEX(("wl%d: %s: action frame send bits %02x\n", wlc->pub->unit,
			__FUNCTION__, coex_bits));

		/* send the action frame to notify the AP */
		wlc_ht_send_action_obss_coex(cfg, coex_bits, wlc->obss->coex_map);
	}
}

static int
wlc_ht_send_action_obss_coex(wlc_bsscfg_t *cfg, uint8 coex_bits, uint8 *coex_map)
{
	wlc_info_t *wlc = cfg->wlc;
	void *p;
	uint8 *end, *pbody;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
	uint chanlist_len = 0;
	struct scb *scb;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!CHSPEC_IS2G(wlc->home_chanspec)) {
		WL_ERROR(("wl%d: %s: Not in 2G band\n", wlc->pub->unit, __FUNCTION__));
	        return BCME_BADBAND;
	}

	if (!BSSCFG_STA(cfg)) {
		WL_ERROR(("wl%d: %s: Not STA\n", wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	if ((scb = wlc_scbfind(wlc, cfg, &cfg->BSSID)) == NULL) {
		WL_ERROR(("wl%d: %s: scb = NULL\n", wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	if (!SCB_COEX_CAP(scb)) {
		WL_COEX(("wl%d: %s: scb don't support Coexistence\n",
		         wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	/* Coexistence action frame consist of 20/40 BSS Coexistence IE +
	 * 20/40 BSS Intolerant Channel Report.
	 * 20/40 BSS Coexistence IE:
	 * -------------------
	 * | ID | LEN | INFO |
	 * -------------------
	 * 20/40 BSS Intolerant Channel Report IE:
	 * ------------------------------------
	 * | ID | LEN | RClass | Channel list |
	 * ------------------------------------
	 * Variable len : Channel list
	 */
	body_len = DOT11_ACTION_HDR_LEN + sizeof(dot11_obss_coex_t);
	/* chanlist_len include IE hdr + 1 or more BSS intolerant
	 * Channel Report IE.
	 */
	chanlist_len = wlc_get_BSSintol_2Gchanlist_len(wlc, coex_map);
	body_len += chanlist_len;

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, &scb->ea, &cfg->cur_etheraddr,
		&cfg->BSSID, body_len, &pbody)) == NULL)
		return -1;

	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_PUBLIC;
	action_hdr->action = DOT11_PUB_ACTION_BSS_COEX_MNG;
	end = wlc_write_obss_coex_ie(action_hdr->data, coex_bits);
	end = wlc_write_obss_intol_chanlist_ie(wlc, end, (uint8)chanlist_len, coex_map);

	ASSERT((end - pbody) == (int)body_len);

#ifdef BCMDBG
	{
	char da[ETHER_ADDR_STR_LEN];
	WL_COEX(("wl%d: %s: send action frame to %s\n", wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, da)));
	}
#endif // endif

	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

	return 0;
}

static uint8*
wlc_write_obss_coex_ie(uint8* cp, uint8 coex_bits)
{
	dot11_obss_coex_t *coex_ie = (dot11_obss_coex_t *)cp;

	coex_ie->id = DOT11_MNG_HT_BSS_COEXINFO_ID;
	coex_ie->len = DOT11_OBSS_COEXINFO_LEN;
	coex_ie->info = 0;
	if (coex_bits & WL_COEX_INFO_REQ)
		coex_ie->info |= DOT11_OBSS_COEX_INFO_REQ;
	if (coex_bits & WL_COEX_40MHZ_INTOLERANT)
		coex_ie->info |= DOT11_OBSS_COEX_40MHZ_INTOLERANT;
	if (coex_bits & WL_COEX_WIDTH20)
		coex_ie->info |= DOT11_OBSS_COEX_20MHZ_WIDTH_REQ;

	cp += (TLV_HDR_LEN + DOT11_OBSS_COEXINFO_LEN);
	return cp;
}

static uint8*
wlc_write_obss_intol_chanlist_ie(wlc_info_t *wlc, uint8* cp, uint8 len, uint8 *coex_map)
{
	uint8 *p, buf_len;
	dot11_obss_chanlist_t *chanlist_ie = (dot11_obss_chanlist_t *)cp;

	ASSERT(CHSPEC_IS2G(wlc->home_chanspec));

	if (!coex_map || len == 0) {
		WL_ERROR(("NO channel list\n"));
		return cp;
	}

	p = cp;
	BCM_REFERENCE(p);
	buf_len = len;
	BCM_REFERENCE(buf_len);

	if ((wlc_japan(wlc) == TRUE) && coex_map[14] && len) {
		/* Japan can have 2 regclass in 2G band,
		 * take care first IE (chan 14)
		 */
		chanlist_ie->id = DOT11_MNG_HT_BSS_CHANNEL_REPORT_ID;
		chanlist_ie->len = DOT11_OBSS_CHANLIST_FIXED_LEN + 1;
		chanlist_ie->regclass = wlc_get_regclass(wlc->cmi, CH20MHZ_CHSPEC(14));
		chanlist_ie->chanlist[0] = 14;
		len -= (TLV_HDR_LEN + chanlist_ie->len);	/* first IE takes 4 bytes */
		cp += (TLV_HDR_LEN + chanlist_ie->len);		/* adjust the pointer */
	}
	/* take care all remaining channel in 2G band */
	chanlist_ie = (dot11_obss_chanlist_t *)cp;
	chanlist_ie->id = DOT11_MNG_HT_BSS_CHANNEL_REPORT_ID;
	/* the len includes id & len */
	chanlist_ie->len = len - TLV_HDR_LEN;
	chanlist_ie->regclass = wlc_get_regclass(wlc->cmi, CH20MHZ_CHSPEC(1));
	cp = chanlist_ie->chanlist;
	if (chanlist_ie->len > DOT11_OBSS_CHANLIST_FIXED_LEN) {
		uint8 i;
		for (i = 0; i <= CH_MAX_2G_CHANNEL; i++) {
			if (coex_map[i]) {
				*cp++ = i;
				WL_ERROR(("coex channel %d\n", i));
			}
		}
	}
	ASSERT((cp - p) == (int)buf_len);
	return cp;
}

static uint
wlc_get_BSSintol_2Gchanlist_len(wlc_info_t *wlc, uint8 *coex_map)
{
	uint i, len;

	ASSERT(CHSPEC_IS2G(wlc->home_chanspec));

	WL_TRACE(("wl%d: wlc_get_BSSintol_2Gchanlist_len\n", wlc->pub->unit));
	if (!coex_map)
		return 0;
	/* regulatory class in 2.4G band, for chan 1-13 */
	for (i = 0, len = 0; i <= 13; i++) {
		if (coex_map[i])
			len++;
	}
	/* include header if there are interference channels to report */
	if (len)
		len += TLV_HDR_LEN + DOT11_OBSS_CHANLIST_FIXED_LEN;

	/* Japan has second regulatory class in 2.4G band, chan 14 & regulatory class */
	if ((wlc_japan(wlc) == TRUE) && coex_map[14])
		len += TLV_HDR_LEN + DOT11_OBSS_CHANLIST_FIXED_LEN + 1;

	return len;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_obss(wlc_obss_info_t *obssi, struct bcmstrbuf *b)
{
	int idx;
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "num chans: %u\n", obssi->num_chan);
	bcm_bprhex(b, "chanvec: ", TRUE, obssi->chanvec, OBSS_CHANVEC_SIZE);
	bcm_bprhex(b, "map: ", TRUE, obssi->coex_map, CH_MAX_2G_CHANNEL);

	FOREACH_AS_BSS(obssi->wlc, idx, cfg) {
		wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(obssi, cfg);

		bcm_bprintf(b, "\nbsscfg: %d\n", WLC_BSSCFG_IDX(cfg));
		bcm_bprintf(b, "enab: %d permit: %d detected: %u det ovrd: %u "
		            "switch_bw_def: %d bits_buf: %u "
		            "next scan: %u secs fid: %u te mask: 0x%x\n",
		            obss->coex_enab, obss->coex_permit, obss->coex_det,
		            obss->coex_ovrd,
		            obss->switch_bw_deferred, obss->coex_bits_buffered,
		            obss->scan_countdown, obss->fid_time, obss->coex_te_mask);
		bcm_bprintf(b, "param: passive dwell %u active dwell %u bss width scan %u "
		            "passive total %u active total %u chan width tran dly %u "
		            "activity threshold %u\n",
		            obss->params.passive_dwell, obss->params.active_dwell,
		            obss->params.bss_widthscan_interval,
		            obss->params.passive_total, obss->params.active_total,
		            obss->params.chanwidth_transition_dly,
		            obss->params.activity_threshold);
#ifdef STA
		/* AID */
		bcm_bprintf(b, "\nAID = 0x%x\n", cfg->AID);
#endif // endif
	}

	return 0;
}
#endif	/* BCMDBG || BCMDBG_DUMP */

static void
wlc_ht_chanlist_2g_init(wlc_obss_info_t *obss)
{
	uint i;

	/* Initialize chanlist only once. */
	if (obss->num_chan != 0)
		return;

	for (i = 1; i <= CH_MAX_2G_CHANNEL; i++) {
		if (VALID_CHANNEL20_IN_BAND(obss->wlc, BAND_2G_INDEX, i)) {
			setbit(obss->chanvec, i);
			obss->num_chan++;
		}
	}

	WL_COEX(("wl%d: %s: %d channels in 2G band\n", obss->wlc->pub->unit, __FUNCTION__,
		obss->num_chan));
}

/* bsscfg cubby */
static int
wlc_obss_bss_init(void *context, wlc_bsscfg_t *cfg)
{
	wlc_obss_info_t *obss = (wlc_obss_info_t *)context;
	if ((BSS_OBSS_INFO(obss, cfg) =
		MALLOCZ(obss->wlc->osh, sizeof(struct wlc_obss_bss_info))) == NULL) {
		return BCME_NOMEM;
	}
	/* obss init */
	wlc_ht_obss_scanparam_init(&(BSS_OBSS_INFO(obss, cfg)->params));
	/* coex enable/disable */
	wlc_ht_coex_enab(cfg, COEX_ENAB(obss->wlc));
	return BCME_OK;
}

static void
wlc_obss_bss_deinit(void *context, wlc_bsscfg_t *cfg)
{
	wlc_obss_info_t *obss = (wlc_obss_info_t *)context;
	if (BSS_OBSS_INFO(obss, cfg)) {
		MFREE(obss->wlc->osh, BSS_OBSS_INFO(obss, cfg),
			sizeof(struct wlc_obss_bss_info));
		BSS_OBSS_INFO(obss, cfg) = NULL;
	}
}

static void
wlc_obss_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
}

#ifdef WL11N
static uint8
wlc_obss_get_num_chan(wlc_obss_info_t *obss)
{
	return obss->num_chan;
}

bool
wlc_obss_is_scan_complete(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg,
	bool status_success,
	int active_time, int passive_time)
{
	bool done = TRUE;
	obss_params_t *params;

	wlc_obss_bss_info_t *obss_bss_info = BSS_OBSS_INFO(obss, cfg);
	params = &obss_bss_info->params;

	/* need to reset obss scan timer if expired and scan abort */
	if (!status_success && !obss_bss_info->scan_countdown) {
		obss_bss_info->scan_countdown = 5;
		done = FALSE;
	} else if (active_time < (int)params->active_dwell) {
		WL_COEX(("wl%d: %s: active time %d < %d\n", cfg->wlc->pub->unit, __FUNCTION__,
			active_time, params->active_dwell));
		done = FALSE;
	} else if (passive_time < (int)params->passive_dwell) {
		WL_COEX(("wl%d: %s: passive time %d < %d\n", cfg->wlc->pub->unit, __FUNCTION__,
			passive_time, params->passive_dwell));
		done = FALSE;
	}
	return done;
}

void
wlc_obss_scan_update_countdown(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg,
	uint8* chanvec, int num_chan)
{
	wlc_obss_bss_info_t *obss_bss_info = BSS_OBSS_INFO(obss, cfg);

	if (num_chan < wlc_obss_get_num_chan(obss)) {
		WL_COEX(("wl%d: %s: scanned channel [%d] < OBSS Scan Params [%d]\n",
			obss->wlc->pub->unit, __FUNCTION__, num_chan, wlc_obss_get_num_chan(obss)));
		return;
	}
	if (bcmp(chanvec, obss->chanvec, OBSS_CHANVEC_SIZE)) {
		WL_COEX(("wl%d: %s: scanned channel list don't match to OBSS Scan requirement\n",
			obss->wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* XXX if OBSS scan period is < default scan, it will cause
	 * extra scan occure when default scan is scheduled
	 */
	if (cfg->associated && IS_SCAN_COEX_ACTIVE(obss, cfg)) {
		/* valid scan, analysis the result */
		wlc_ht_coex_filter_scanresult(cfg);
		obss_bss_info->scan_countdown =
			obss_bss_info->params.bss_widthscan_interval;
		WL_COEX(("wl%d: %s: Arm scan for %dsec\n", cfg->wlc->pub->unit, __FUNCTION__,
			obss_bss_info->scan_countdown));
	}
}

bool
wlc_obss_scan_fields_valid(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (cfg == NULL || BSS_OBSS_INFO(obss, cfg) == NULL)
	{
		WL_COEX(("%s: %s is null; aborting obss scan update",
			__FUNCTION__, (cfg == NULL) ? "bsscfg" : "cfg->obss"));
		return FALSE;
	}
	/* abort if STA is not OBSS feature enabled */
	if (!IS_SCAN_COEX_ACTIVE(obss, cfg))
		return FALSE;

	return TRUE;
}

bool
wlc_obss_cfg_get_coex_enab(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	return BSS_OBSS_INFO(obss, cfg)->coex_enab;
}

static void
wlc_obss_set_coex_enab(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg, bool setting)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return;
	}
	wlc_ht_coex_enab(cfg, setting);
}

uint8
wlc_obss_cfg_get_coex_det(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return 0;
	}

	return BSS_OBSS_INFO(obss, cfg)->coex_det;
}

void
wlc_ht_obss_scan_reset(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_obss_bss_info_t *obss = BSS_OBSS_INFO(wlc->obss, cfg);
	wlc_bss_info_t *current_bss = cfg->current_bss;

	WL_TRACE(("wl%d: wlc_ht_obss_scan_reset\n", wlc->pub->unit));

	/* at each association, reset tracking variables */
	obss->coex_det = 0;
	obss->coex_bits_buffered = 0;
	obss->switch_bw_deferred = FALSE;

	/* start obss coexistence if associated */
	/* support obss coexistence for infra mode */
	if (!COEX_ACTIVE(wlc->obss, cfg) ||
	    !cfg->associated ||
	    !cfg->BSS)
		return;

	/* start obss scan only if join 11n BSS */
	if ((current_bss->flags & (WLC_BSS_HT | WLC_BSS_40MHZ)) != (WLC_BSS_HT | WLC_BSS_40MHZ))
		return;

	obss->scan_countdown = obss->params.bss_widthscan_interval;
	WL_COEX(("wl%d: %s: start obss_scan_timer\n", wlc->pub->unit, __FUNCTION__));
}

static void
wlc_ht_obss_scanparam_init(obss_params_t *params)
{
	/* set Overlapping BSS scan parameters default */
	params->passive_dwell = WLC_OBSS_SCAN_PASSIVE_DWELL_DEFAULT;
	params->active_dwell = WLC_OBSS_SCAN_ACTIVE_DWELL_DEFAULT;
	params->bss_widthscan_interval = WLC_OBSS_SCAN_WIDTHSCAN_INTERVAL_DEFAULT;
	params->chanwidth_transition_dly = WLC_OBSS_SCAN_CHANWIDTH_TRANSITION_DLY_DEFAULT;
	params->passive_total = WLC_OBSS_SCAN_PASSIVE_TOTAL_PER_CHANNEL_DEFAULT;
	params->active_total = WLC_OBSS_SCAN_ACTIVE_TOTAL_PER_CHANNEL_DEFAULT;
	params->activity_threshold = WLC_OBSS_SCAN_ACTIVITY_THRESHOLD_DEFAULT;
}

void
wlc_ht_obss_scanparams_hostorder(wlc_info_t *wlc, obss_params_t *param, bool to_host_order)
{
	WL_TRACE(("wl%d: wlc_ht_obss_scanparams_hostorder\n", wlc->pub->unit));
	if (to_host_order) {
		/* convert to host order */
		param->passive_dwell = ltoh16(param->passive_dwell);
		param->active_dwell = ltoh16(param->active_dwell);
		param->bss_widthscan_interval = ltoh16(param->bss_widthscan_interval);
		param->passive_total = ltoh16(param->passive_total);
		param->active_total = ltoh16(param->active_total);
		param->chanwidth_transition_dly = ltoh16(param->chanwidth_transition_dly);
		param->activity_threshold = ltoh16(param->activity_threshold);
	} else {
		/* convert to 802.11 little-endian order */
		param->passive_dwell = htol16(param->passive_dwell);
		param->active_dwell = htol16(param->active_dwell);
		param->bss_widthscan_interval = htol16(param->bss_widthscan_interval);
		param->passive_total = htol16(param->passive_total);
		param->active_total = htol16(param->active_total);
		param->chanwidth_transition_dly = htol16(param->chanwidth_transition_dly);
		param->activity_threshold = htol16(param->activity_threshold);
	}
}

void
wlc_obss_coex_checkadd_40intol(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg,
	bool is2G, uint16* cap)
{
	wlc_info_t *wlc = obss->wlc;
	/* Set 40MHz Intolerant bit if
	 * - On 2G Band and
	 * - User has it configured or STA detected a intolerant device
	 * Disable 40MHz capability if an intolerant device is detected
	 */
	if (is2G && WLC_INTOL40_OVRD(wlc, cfg)) {
		*cap |= HT_CAP_40MHZ_INTOLERANT;
	}
}

#endif /* WL11N */

static int8
wlc_obss_get_switch_bw_deferred(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return 0;
	}
	return BSS_OBSS_INFO(obss, cfg)->switch_bw_deferred;
}

#ifdef AP
static uint32
wlc_obss_cfg_get_tr_delay(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return 0;
	}

	return BSS_OBSS_INFO(obss, cfg)->params.bss_widthscan_interval *
		BSS_OBSS_INFO(obss, cfg)->params.chanwidth_transition_dly;
}

static uint32
wlc_obss_cfg_get_fid_time(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return 0;
	}

	return BSS_OBSS_INFO(obss, cfg)->fid_time;
}
#endif /* AP */

static uint8
wlc_obss_cfg_get_coex_ovrd(wlc_obss_info_t *obss, wlc_bsscfg_t *cfg)
{
	if (!BSS_OBSS_INFO(obss, cfg)) {
		return 0;
	}

	return BSS_OBSS_INFO(obss, cfg)->coex_ovrd;
}

#ifdef AP
static void
wlc_ht_coex_switch_bw(wlc_bsscfg_t *cfg, bool downgrade, uint8 reason_code)
{
	chanspec_t chanspec, newchanspec;
	wlc_info_t *wlc = cfg->wlc;
#ifdef WL11H
	wl_chan_switch_t csa;
#endif // endif

	if (!cfg->associated || !cfg->current_bss)
		return;

	chanspec = cfg->current_bss->chanspec;

	ASSERT(cfg->BSS);
	ASSERT(wlc_obss_cfg_get_coex_enab(wlc->obss, cfg));

	if ((!CHSPEC_IS40(chanspec) && downgrade) ||
	    (CHSPEC_IS40(chanspec) && !downgrade)) {
		WL_COEX(("wl%d: %s: cannot %s already at BW %s\n", wlc->pub->unit, __FUNCTION__,
			downgrade ? "downgrade":"upgrade",
			(CHSPEC_IS40(chanspec)? "40MHz":"20MHz")));
		return;
	}

	/* return if  a channel switch is already scheduled */
	if (BSS_WL11H_ENAB(wlc, cfg) &&
	    wlc_11h_get_spect_state(wlc->m11h, cfg) & NEED_TO_SWITCH_CHANNEL)
		return;

	/* upgrade AP only if previous has been downgraded */
	if (!downgrade && !CHSPEC_IS40(wlc->default_bss->chanspec))
		return;

	if (downgrade) {
		newchanspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec));
		WL_COEX(("wl%d: wlc_ht_coex_downgrad_bw\n", wlc->pub->unit));
	}
	else {
		/* If the default bss chanspec is now invalid then pick a valid one */
		if (!wlc_valid_chanspec(wlc->cmi, wlc->default_bss->chanspec))
			wlc->default_bss->chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
		newchanspec = wlc->default_bss->chanspec;
	}

	if (newchanspec == chanspec)
		return;

	WL_COEX(("wl%d: %s: switching chanspec 0x%x to 0x%x reason %d\n",
		wlc->pub->unit, __FUNCTION__, chanspec, newchanspec, reason_code));

	/* XXX need to send legacy CSA and new 11n Ext-CSA if
	 * is n-enabled.  This can conflict with 11h
	 * statemachine.
	 * This code currently is used for testing!!!!
	 */
#ifdef WL11H
	csa.mode = DOT11_CSA_MODE_ADVISORY;
	/* downgrade/upgrade with same control channel, do it immediately */
	csa.count =
		(wf_chspec_ctlchan(chanspec) == wf_chspec_ctlchan(newchanspec)) ? 0 : 10;
	csa.chspec = newchanspec;
	csa.reg = wlc_get_regclass(wlc->cmi, newchanspec);
	csa.frame_type = CSA_BROADCAST_ACTION_FRAME;
	wlc_csa_do_csa(wlc->csa, cfg, &csa, csa.count == 0);
#endif // endif
#if NCONF
	/* Update edcrs on bw change */
	wlc_bmac_ifsctl_edcrs_set(wlc->hw, WLCISHTPHY(wlc->band));
#endif /* NCONF */
}

void
wlc_ht_coex_update_permit(wlc_bsscfg_t *cfg, bool permit)
{
	if (!BSS_OBSS_INFO(cfg->wlc->obss, cfg)) {
		return;
	}

	ASSERT(COEX_ACTIVE(cfg->wlc->obss, cfg));
	BSS_OBSS_INFO(cfg->wlc->obss, cfg)->coex_permit = permit;
}

void
wlc_ht_coex_update_fid_time(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;

	if (!BSS_OBSS_INFO(wlc->obss, cfg)) {
		return;
	}

	ASSERT(COEX_ACTIVE(wlc->obss, cfg));
	BSS_OBSS_INFO(cfg->wlc->obss, cfg)->fid_time = wlc->pub->now;
}

/* check for coex trigger event A (TE-A)
 * a non-ht beacon
 */
static bool
wlc_ht_ap_coex_tea_chk(wlc_bsscfg_t *cfg, ht_cap_ie_t *cap_ie)
{
	chanspec_t chspec = cfg->current_bss->chanspec;

	if (!BSSCFG_AP(cfg) || !cfg->associated)
		return FALSE;

	if (!CHSPEC_IS2G(chspec))
		return FALSE;

	/* get a non-ht beacon when the bss is up */
	if (!cap_ie &&
	    !(BSS_OBSS_INFO(cfg->wlc->obss, cfg)->coex_te_mask & COEX_MASK_TEA))
		return TRUE;

	return FALSE;
}

/* Trigger event B (TE-B)
 * For HT APs with COEX, check for Trigger Event B (TE-B) :
 * an HT beacon with HT_CAP_40MHZ_INTOLERANT bit set in HT Cap IE
 */
/* XXX Do we need to handle the BRCM IE that encapsulate the HT CAP IE
 * as Trigger event B?
 */
/* XXX Do we need to process a regular beacon as 20/40 BSS coex TE-B?
 */
static int
wlc_ht_ap_coex_scan_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	int idx;
	wlc_bsscfg_t *cfg;
	ht_cap_ie_t *cap;
	uint16 ht_cap;
	chanspec_t rxchspec;

	if (!N_ENAB(wlc->pub))
		return BCME_OK;

	if (data->ie == NULL)
		return BCME_OK;

	if ((cap = wlc_read_ht_cap_ie(wlc, data->ie, data->ie_len)) == NULL)
		return BCME_OK;

	if (((ht_cap = ltoh16_ua(&cap->cap)) & HT_CAP_40MHZ_INTOLERANT) == 0)
		return BCME_OK;

	rxchspec = CH20MHZ_CHSPEC(data->pparm->ft->bcn.chan);

	/* for AP, this is trigger event B (TE-B) */
	FOREACH_UP_AP(wlc, idx, cfg) {
		chanspec_t chspec = cfg->current_bss->chanspec;

		if (!COEX_ACTIVE(wlc->obss, cfg))
			continue;
		if (!CHSPEC_CTLOVLP(chspec, rxchspec, CH_20MHZ_APART))
			continue;

		wlc_ht_ap_coex_tebc_proc(cfg);
	}

	return BCME_OK;
}

/*
 * process trigger event B/C (TE-B, TE-C)
 */
static void
wlc_ht_ap_coex_tebc_proc(wlc_bsscfg_t *cfg)
{
	chanspec_t chspec = cfg->current_bss->chanspec;

	if (!BSSCFG_AP(cfg) || !cfg->associated)
		return;

	if (!CHSPEC_IS2G(chspec))
		return;

	WL_COEX(("wl%d: %s: obss_coex trigger event B/C detected\n",
		cfg->wlc->pub->unit, __FUNCTION__));

	wlc_ht_coex_update_fid_time(cfg);

	if (!CHSPEC_IS40(chspec))
		return;

	wlc_ht_coex_switch_bw(cfg, TRUE, COEX_DGRADE_TEBC);
}

/* Trigger event A (TE-A)
 * For HT APs with COEX, check for Trigger Event A (TE-A) :
 * a non-HT beacon is detected
 */
static int
wlc_ht_ap_coex_bcn_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	int idx;
	wlc_bsscfg_t *cfg;
	chanspec_t rxchspec;

	if (!N_ENAB(wlc->pub) || SCAN_IN_PROGRESS(wlc->scan))
		return BCME_OK;

	/* An optimization - bail out if HT Cap IE is present */
	if (data->ie != NULL)
		return BCME_OK;

	rxchspec = CH20MHZ_CHSPEC(data->pparm->ft->bcn.chan);

	FOREACH_UP_AP(wlc, idx, cfg) {
		chanspec_t chspec = cfg->current_bss->chanspec;

		if (!COEX_ACTIVE(wlc->obss, cfg))
			continue;
		if (!CHSPEC_CTLOVLP(chspec, rxchspec, CH_20MHZ_APART))
			continue;

		if (!wlc_ht_ap_coex_tea_chk(cfg, (ht_cap_ie_t *)data->ie))
			continue;
		wlc_ht_coex_update_permit(cfg, FALSE);
		wlc_ht_coex_update_fid_time(cfg);
		if (CHSPEC_IS20(chspec))
			continue;
		wlc_ht_coex_switch_bw(cfg, TRUE, COEX_DGRADE_TEA);
	}

	return BCME_OK;
}

/* check for coex trigger event D (TE-D)
 * a 20/40 BSS coex frame
 */
static bool
wlc_ht_ap_coex_ted_chk(wlc_bsscfg_t *cfg, bcm_tlv_t *bss_chanlist_tlv, uint8 coex_bits)
{
	wlc_info_t *wlc = cfg->wlc;
	dot11_obss_chanlist_t *chanlist_ie = (dot11_obss_chanlist_t *)bss_chanlist_tlv;
	uint8 chlist_len, chan, i;
	uint8 ch_min, ch_max, ch_ctl;

	WL_TRACE(("wl%d: wlc_ht_upd_coex_map\n", wlc->pub->unit));

	if (!CHSPEC_IS2G(wlc->home_chanspec))
		return FALSE;

	if (!chanlist_ie) {
		WL_ERROR(("wl%d: %s: 20/40 BSS Intolerant Channel Report IE NOT found\n",
			wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* minimum len is 3 bytes (IE Hdr + regulatory class) */
	if (bss_chanlist_tlv->len < DOT11_OBSS_CHANLIST_FIXED_LEN) {
		WL_ERROR(("wl%d: %s: Invalid 20/40 BSS Intolerant Channel Report IE len %d\n",
			wlc->pub->unit, __FUNCTION__, bss_chanlist_tlv->len));
		return FALSE;
	}
	/* this is a trigger irrelevant of the channel in use  */
	if (coex_bits & ~WL_COEX_INFO_REQ) {
		return TRUE;
	}
	/* exclude regulatory class */
	chlist_len = chanlist_ie->len - DOT11_OBSS_CHANLIST_FIXED_LEN;

	WL_COEX(("wl%d: wlc_ht_upd_coex_map : %d 20MHZ channels\n", wlc->pub->unit, chlist_len));

	/* retrieve exclusion range and ctl channel */
	wlc_ht_coex_exclusion_range(wlc, &ch_min, &ch_max, &ch_ctl);
	/* If we do not do 40 mhz */
	if (!ch_min)
		return FALSE;

	for (i = 0; i < chlist_len; i++) {
		chan = chanlist_ie->chanlist[i];
		if (chan <= CH_MAX_2G_CHANNEL) {
			/*  Here we should store the date for each channel
			    It could be used in the future for auto channel
			    purposes.
			*/

			WL_COEX(("wl%d: %s: chan %d flag 0x%x\n", wlc->pub->unit, __FUNCTION__,
			         chan, WL_COEX_WIDTH20));
			if (chan >= ch_min && chan <= ch_max && chan != ch_ctl) {
				/* trigger */
				return TRUE;
			}
		}
	}
	return FALSE;
}

#ifdef WL11N
static void
wlc_coex_hdl_40_intol_sta(wlc_obss_info_t *obss, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = obss->wlc;
	wlc_bss_info_t *current_bss = bsscfg->current_bss;
	if (N_ENAB(wlc->pub) && COEX_ENAB(wlc)) {
		wlc_ht_upd_coex_state(bsscfg, DOT11_OBSS_COEX_40MHZ_INTOLERANT);

		if (CHSPEC_IS40(current_bss->chanspec)) {
			wlc_ht_coex_update_fid_time(bsscfg);
			wlc_ht_coex_switch_bw(bsscfg, TRUE, COEX_DGRADE_40INTOL);
		}
	}
}

/* called from ap assocreq_done function via scb state notif */
static void
wlc_obss_scb_state_upd(bcm_notif_client_data ctx, bcm_notif_server_data scb_data)
{
	wlc_obss_info_t *obss = (wlc_obss_info_t *)ctx;
	scb_state_upd_data_t *data = (scb_state_upd_data_t *)scb_data;
	struct scb *scb = data->scb;
	/* hndl transition from unassoc to assoc */
	if (!(data->oldstate & ASSOCIATED) && (scb->state & ASSOCIATED)) {
		/* Check for association of a 40 intolerant STA */
		if ((scb->flags & SCB_HT40INTOLERANT)) {
			wlc_coex_hdl_40_intol_sta(obss, SCB_BSSCFG(scb));
		}
	}
}
#endif /* WL11N */
#endif /* AP */

static void
wlc_obss_watchdog(void *ctx)
{
	wlc_obss_info_t *obss = (wlc_obss_info_t *)ctx;
	wlc_info_t *wlc = obss->wlc;
	int i;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, i, cfg) {
		/* check obss scan timeout if feature is enabled */
		if (N_ENAB(wlc->pub) && COEX_ACTIVE(obss, cfg)) {
#ifdef STA
			if (BSSCFG_STA(cfg))
				wlc_ht_obss_scan_timer(cfg);
#endif /* STA */
			/* check if switch was deferred or bss 20/40 restore time is up */
			if (BSSCFG_AP(cfg) || wlc_obss_get_switch_bw_deferred(wlc->obss, cfg))
				wlc_ht_obss_coex_timeout(cfg);
		}
	}

#ifdef AP
	FOREACH_UP_AP(wlc, i, cfg) {
		/* OBSS COEX check for possible upgrade
		 * only do it when it is currently 20MHz
		 */
		if (COEX_ACTIVE(obss, cfg) && CHSPEC_IS20(cfg->current_bss->chanspec)) {
			/* if any 40 intolerant still associated, don't upgrade */
			if (wlc_prot_n_ht40int(wlc->prot_n, cfg))
				wlc_ht_coex_update_fid_time(cfg);
			else {
				uint32 tr_delay = wlc_obss_cfg_get_tr_delay(wlc->obss, cfg);
				if ((wlc->pub->now - wlc_obss_cfg_get_fid_time(wlc->obss, cfg)) >
					tr_delay) {
					/* The AP udates the fid_time everytime a new trigger
					 * happens. If we get here, we are good to upgrade.
					 */
					wlc_ht_coex_update_permit(cfg, TRUE);
					wlc_ht_coex_switch_bw(cfg, FALSE, COEX_UPGRADE_TIMER);
				}
			}
		}
	}
#endif /* AP */
}

void
wlc_obss_update_scbstate(wlc_obss_info_t *obss, wlc_bsscfg_t *bsscfg,
	obss_params_t *obss_params)
{
	wlc_info_t *wlc = obss->wlc;

	if (COEX_ENAB(wlc) && obss_params && BSSCFG_STA(bsscfg)) {
		obss_params_t params;
		bcopy((uint8 *)obss_params, (uint8 *)&params, WL_OBSS_SCAN_PARAM_LEN);
		/* convert params to host order */
		wlc_ht_obss_scanparams_hostorder(wlc, &params, TRUE);
		if (!wlc_ht_obss_scanparams_upd(bsscfg, &params)) {
			WL_ERROR(("Invalid OBSS Scan parameters (out-of-range): "
				" %d %d %d %d %d %d %d\n", params.passive_dwell,
				params.active_dwell, params.bss_widthscan_interval,
				params.passive_total, params.active_total,
				params.chanwidth_transition_dly,
				params.activity_threshold));
		}
	}
}
