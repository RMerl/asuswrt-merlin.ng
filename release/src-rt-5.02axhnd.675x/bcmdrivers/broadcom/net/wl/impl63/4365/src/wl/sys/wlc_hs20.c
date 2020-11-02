/*
 * Hotspot 2.0 module
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
 * $Id: wlc_hs20.c 708017 2017-06-29 14:11:45Z $
 *
 */

/**
 * @file
 * @brief
 * Hotspot2.0 is a new protocol defined by WFA which allows a Hotspot2.0 capable STA to exchange
 * pre-association information with a HotSpot2.0 capable AP using 802.11u GAS action frames. The
 * information obtained allows the STA to decide and/or choose the AP to associate (based on domain,
 * service provider, credentials, etc). Hotspot2.0 only supports enterprise-based authentication is
 * supported: EAP-TLS (certificate credential), EAP-SIM (GSM SIM credential), EAP-AKA
 * (UMTS USIM credential), or EAP-TTLS with MS-CHAPv2 (username/password credential and server-side
 * certificate). The user experience objective is to provide seamless switchover from 3G/4G/WLAN to
 * WLAN once the Hotspot2.0 capable STA comes within to range of the HotSpot2.0 capable AP.
 * Hotspot2.0 is also known as Passpoint.
 */

/**
 * @file
 * @brief
 * XXX  Twiki [Hotspot20]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <wl_dbg.h>
#include <osl.h>
#include <siutils.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_helper.h>
#include <wlc_hs20.h>

/* hotspot private info structure */
struct wlc_hs20_info {
	wlc_info_t *wlc;			/* pointer back to wlc structure */

	uint8 hs20_ie_len;
	uint8 *hs20_ie;

	int cfgh;				/* bsscfg cubby handle */
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)

enum {
	IOV_HS20_IE,
	IOV_OSEN
};

static const bcm_iovar_t hs20_iovars[] = {
	{"hs20_ie", IOV_HS20_IE,
	0, IOVT_BUFFER, OFFSETOF(tlv_t, data)},
	{"osen", IOV_OSEN,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* cubby */
static int wlc_hs20_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_hs20_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
#ifdef BCMDBG
static void wlc_hs20_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_hs20_bsscfg_dump NULL
#endif // endif

typedef struct {
	bool	osen;	/* enable/disable */
} wlc_hs20_bsscfg_cubby_t;

#define WLC_HS20_BSSCFG_CUBBY(hs20, cfg) \
	((wlc_hs20_bsscfg_cubby_t *)BSSCFG_CUBBY((cfg), (hs20)->cfgh))

/* ----------------------------------------------------------- */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* handling hotspot related iovars */
static int
hs20_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_hs20_info_t *hs20 = hdl;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	wlc_hs20_bsscfg_cubby_t *cubby_hs20;
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	ASSERT(hs20);

#ifndef BCMROMOFFLOAD
	WL_INFORM(("wl%d: hs20_doiovar()\n", WLCUNIT(hs20)));
#endif /* !BCMROMOFFLOAD */

	wlc = hs20->wlc;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* cubby */
	cubby_hs20 = WLC_HS20_BSSCFG_CUBBY(hs20, bsscfg);
	ASSERT(cubby_hs20 != NULL);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_SVAL(IOV_HS20_IE):
	{
		tlv_t *tlv = (tlv_t *)a;

		if (hs20->hs20_ie_len > 0) {
			/* free previous IE */
			MFREE(WLCOSH(hs20), hs20->hs20_ie, hs20->hs20_ie_len);
			hs20->hs20_ie_len = 0;
		}

		if (tlv->len > 0) {
			hs20->hs20_ie = MALLOC(WLCOSH(hs20), tlv->len);
			if (hs20->hs20_ie == NULL) {
				err = BCME_NOMEM;
			}
			else {
				hs20->hs20_ie_len = tlv->len;
				bcopy(tlv->data, hs20->hs20_ie, hs20->hs20_ie_len);
			}
		}

		break;
	}

	case IOV_GVAL(IOV_OSEN):
		if (WLOSEN_ENAB(wlc->pub))
			*((uint*)a) = cubby_hs20->osen;
		else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_SVAL(IOV_OSEN):
		if (WLOSEN_ENAB(wlc->pub)) {
			/* if bss configuration is down then return error */
			if (BSSCFG_AP(bsscfg) && (!bsscfg->up)) {
				err = BCME_NOTUP;
				break;
			}
			cubby_hs20->osen = bool_val;

			/* force IE update */
			if (BSSCFG_AP(bsscfg)) {
				wlc_bss_update_beacon(wlc, bsscfg);
				wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);
			}
		}
		else
			err = BCME_UNSUPPORTED;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static uint
wlc_hs20_assoc_calc_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bss_info_t *bi = wlc_iem_calc_get_assocreq_target(calc);

	if ((bi->flags2 & WLC_BSS_HS20) && wlc->hs20->hs20_ie_len > 0) {
		return TLV_HDR_LEN + wlc->hs20->hs20_ie_len;
	}

	return 0;
}

static int
wlc_hs20_assoc_write_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bss_info_t *bi = wlc_iem_build_get_assocreq_target(build);

	/* add hotspot indication IE only if target AP supports hotspot */
	if ((bi->flags2 & WLC_BSS_HS20) && wlc->hs20->hs20_ie_len > 0) {

		bcm_write_tlv(DOT11_MNG_VS_ID, wlc->hs20->hs20_ie,
			wlc->hs20->hs20_ie_len, build->buf);
	}

	return BCME_OK;
}

static int
wlc_hs20_scan_parse_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_iem_pparm_t *pparm = wlc_iem_parse_get_parm(parse);
	wlc_bss_info_t *bi = pparm->ft->scan.result;
	bcm_tlv_t *ie = (bcm_tlv_t *)parse->ie;

	/* is hotspot AP */
	if (ie != NULL) {
		bi->flags2 |= WLC_BSS_HS20;
	}

	return BCME_OK;
}

static int
wlc_hs20_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	return BCME_OK;
}

static void
wlc_hs20_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
}

#ifdef BCMDBG
static void
wlc_hs20_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_hs20_info_t *hs20 = (wlc_hs20_info_t *)ctx;
	wlc_hs20_bsscfg_cubby_t *cubby_hs20 = WLC_HS20_BSSCFG_CUBBY(hs20, cfg);
	ASSERT(cubby_hs20 != NULL);

	bcm_bprintf(b, "OSEN: %s\n", cubby_hs20->osen ? "enabled" : "disabled");
}
#endif /* BCMDBG */

/*
 * initialize hotspot private context.
 * returns a pointer to the hotspot private context, NULL on failure.
 */
static const char BCMATTACHDATA(rstr_hs20)[] = "hs20";
wlc_hs20_info_t *
BCMATTACHFN(wlc_hs20_attach)(wlc_info_t *wlc)
{
	wlc_hs20_info_t *hs20;
	int err;

	/* allocate hotspot private info struct */
	hs20 = MALLOCZ(wlc->osh, sizeof(wlc_hs20_info_t));
	if (!hs20) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init hotspot private info struct */
	hs20->wlc = wlc;
	wlc->hs20 = hs20;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((hs20->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_hs20_bsscfg_cubby_t),
		wlc_hs20_bsscfg_init, wlc_hs20_bsscfg_deinit, wlc_hs20_bsscfg_dump,
		hs20)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, hs20_iovars, rstr_hs20,
		hs20, hs20_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		goto fail;
	}

	/* register to add hotspot IE to (re)assoc request */
	if ((err = wlc_iem_vs_add_build_fn_mft(wlc->iemi,
		FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ), WLC_IEM_VS_IE_PRIO_HS20,
		wlc_hs20_assoc_calc_ie_len, wlc_hs20_assoc_write_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn_mft failed, err %d, hs20 ie\n",
			WLCWLUNIT(wlc), __FUNCTION__, err));
		goto fail;
	}

	/* register to parse scan result for hotspot IE */
	if ((err = wlc_iem_vs_add_parse_fn_mft(wlc->iemi,
		FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP),
		WLC_IEM_VS_IE_PRIO_HS20, wlc_hs20_scan_parse_ie, wlc)) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, err %d, brcm in scan\n",
			WLCWLUNIT(wlc), __FUNCTION__, err));
		goto fail;
	}

#if defined(WLOSEN) && !defined(WLOSEN_DISABLED)
	wlc->pub->_osen = TRUE;
#endif	/* WLOSEN */

	return hs20;

	/* error handling */
fail:
	if (hs20 != NULL)
		MFREE(wlc->osh, hs20, sizeof(wlc_hs20_info_t));
	return NULL;
}

/* cleanup hotspot private context */
void
BCMATTACHFN(wlc_hs20_detach)(wlc_hs20_info_t *hs20)
{
	WL_INFORM(("wl%d: hs20_detach()\n", WLCUNIT(hs20)));

	if (!hs20)
		return;

	if (hs20->hs20_ie_len > 0) {
		MFREE(WLCOSH(hs20), hs20->hs20_ie, hs20->hs20_ie_len);
		hs20->hs20_ie_len = 0;
	}
	wlc_module_unregister(hs20->wlc->pub, rstr_hs20, hs20);
	MFREE(WLCOSH(hs20), hs20, sizeof(wlc_hs20_info_t));

	hs20 = NULL;
}

/* return TRUE if OSEN enabled */
bool
wlc_hs20_is_osen(wlc_hs20_info_t *hs20, wlc_bsscfg_t *cfg)
{
	bool result = FALSE;

	if (hs20) {
		wlc_info_t *wlc = hs20->wlc;
		BCM_REFERENCE(wlc);
		if (WLOSEN_ENAB(wlc->pub)) {
			wlc_hs20_bsscfg_cubby_t *cubby_hs20 = WLC_HS20_BSSCFG_CUBBY(hs20, cfg);
			ASSERT(cubby_hs20);
			result = cubby_hs20->osen;
		}
	}

	return result;
}
