/*
 * WAPI (WLAN Authentication and Privacy Infrastructure) source file
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
 * $Id: wlc_wapi.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * WAPI is a proposal from the Chinese National Body for a new 802.11 encryption standard.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [BroadcomWAPI]
 */

#if !defined(BCMWAPI_WPI) && !defined(BCMWAPI_WAI)
#error "BCMWAPI_WPI or BCMWAPI_WAI is not defined"
#endif // endif
#if defined(BCMWAPI_WAI) && !defined(BCMWAPI_WPI)
#error "BCMWAPI_WPI must enabled when BCMWPAI_WAI enabled"
#endif // endif

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmwpa.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_scb.h>
#include <wlc_ap.h>
#include <wlc_wapi.h>
#include <wlc_wapi_priv.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>

#include <bcmcrypto/sms4.h>

#ifdef BCMWAPI_WAI
/* IOVAR table ,no ordering is imposed */
enum {
	IOV_WAI_RESTRICT,
	IOV_WAI_REKEY,
	IOV_WAPIIE,
	IOV_LAST
};

static const bcm_iovar_t wlc_wapi_iovars[] = {
	{"wai_restrict", IOV_WAI_RESTRICT, (0), IOVT_BOOL, 0},
	{"wai_rekey", IOV_WAI_REKEY, (0), IOVT_BUFFER, ETHER_ADDR_LEN},
	{"wapiie", IOV_WAPIIE, (0), IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0}
};

static int wlc_wapi_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_wapi_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_wapi_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_wapi_wai_rekey(wlc_wapi_info_t *wapi, wlc_bsscfg_t *cfg, struct ether_addr *addr);
#else
#define wlc_wapi_iovars		NULL
#define wlc_wapi_doiovar	NULL
#endif /* BCMWAPI_WAI */

#ifdef BCMDBG
static int wlc_wapi_dump(wlc_wapi_info_t *wapi, struct bcmstrbuf *b);
#ifdef BCMWAPI_WAI
static void wlc_wapi_bsscfg_dump(void *context, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#endif // endif
#else
#define wlc_wapi_dump		NULL
#ifdef BCMWAPI_WAI
#define wlc_wapi_bsscfg_dump	NULL
#endif // endif
#endif /* BCMDBG */

#ifdef BCMWAPI_WAI
/* IE mgmt */
#ifdef STA
static uint wlc_wapi_arq_calc_wapi_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_wapi_arq_write_wapi_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef AP
static int wlc_wapi_arq_parse_wapi_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
#ifdef AP
static uint wlc_wapi_bcn_calc_wapi_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_wapi_bcn_write_wapi_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef STA
static int wlc_wapi_scan_parse_wapi_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
#endif /* BCMWAPI_WAI */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* Module attach */
wlc_wapi_info_t *
BCMATTACHFN(wlc_wapi_attach)(wlc_info_t *wlc)
{
	wlc_wapi_info_t *wapi = NULL;
#ifdef BCMWAPI_WAI
	/* bsscfg private variables are appened to wlc_wapi_bsscfg_cubby_pub_t structure */
	uint priv_offset = ROUNDUP(sizeof(wlc_wapi_bsscfg_cubby_pub_t), PTRSZ);
#endif // endif
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

	if ((wapi = MALLOCZ(wlc->osh, sizeof(wlc_wapi_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	wapi->wlc = wlc;
	wapi->pub = wlc->pub;

	/* default enable wapi */
	wapi->pub->_wapi_hw_wpi = WAPI_HW_WPI_CAP(wlc);

#ifdef BCMWAPI_WAI
	/* reserve cubby in the bsscfg container for per-bsscfg public/private data */
	if ((wlc->wapi_cfgh = wlc_bsscfg_cubby_reserve(wlc,
		priv_offset + sizeof(wlc_wapi_bsscfg_cubby_priv_t),
		wlc_wapi_bsscfg_init, wlc_wapi_bsscfg_deinit, wlc_wapi_bsscfg_dump,
		(void *)wapi)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* save module internal useful variables */
	wapi->cfgh = wlc->wapi_cfgh;
	wapi->priv_offset = priv_offset;

	/* register IE mgmt calc/build callbacks */
	/* calc/build */
#ifdef STA
	/* assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_WAPI_ID,
	     wlc_wapi_arq_calc_wapi_ie_len, wlc_wapi_arq_write_wapi_ie, wapi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, wapi ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_WAPI_ID,
	     wlc_wapi_bcn_calc_wapi_ie_len, wlc_wapi_bcn_write_wapi_ie, wapi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, wapi ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
	/* pasre */
#ifdef AP
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_WAPI_ID,
	                             wlc_wapi_arq_parse_wapi_ie, wapi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, wapi ie in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
#ifdef STA
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_WAPI_ID,
	                             wlc_wapi_scan_parse_wapi_ie, wapi) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, wapi ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */
#endif /* BCMWAPI_WAI */

	/* keep the module registration the last other add module unregistratin
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_wapi_iovars, "wapi", (void *)wapi,
		wlc_wapi_doiovar, NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "wapi", (dump_fn_t)wlc_wapi_dump, (void *)wapi);
#endif // endif

	return wapi;

	/* error handling */
fail:
	if (wapi != NULL)
		MFREE(wlc->osh, wapi, sizeof(wlc_wapi_info_t));

	return NULL;
}

/* Module detach */
void
BCMATTACHFN(wlc_wapi_detach)(wlc_wapi_info_t *wapi)
{
	if (wapi == NULL)
		return;

	wlc_module_unregister(wapi->pub, "wapi", wapi);
	MFREE(wapi->wlc->osh, wapi, sizeof(wlc_wapi_info_t));
}

#ifdef BCMWAPI_WAI
static int
wlc_wapi_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_info_t *wlc = wapi->wlc;
	wlc_wapi_bsscfg_cubby_pub_t *cfg_pub;
	wlc_wapi_bsscfg_cubby_priv_t *cfg_priv;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	cfg_pub = WAPI_BSSCFG_PUB(wlc, bsscfg);
	cfg_priv = WAPI_BSSCFG_PRIV(wapi, bsscfg);
	ASSERT(cfg_pub != NULL);
	ASSERT(cfg_priv != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_WAI_RESTRICT):
		*ret_int_ptr = (int32)cfg_pub->wai_restrict;
		break;

	case IOV_SVAL(IOV_WAI_RESTRICT):
		cfg_pub->wai_restrict = bool_val;
		break;

	case IOV_SVAL(IOV_WAI_REKEY):
		wlc_wapi_wai_rekey(wapi, bsscfg, arg);
		break;

	case IOV_GVAL(IOV_WAPIIE):
		if (len < cfg_priv->wapi_ie_len)
			err = BCME_BUFTOOSHORT;
		else if (!cfg_priv->wapi_ie_len)
			err = BCME_NOTFOUND;
		else
			bcopy(cfg_priv->wapi_ie, (uint8 *)arg, cfg_priv->wapi_ie_len);
		break;

	case IOV_SVAL(IOV_WAPIIE):
		if (cfg_priv->wapi_ie) {
			MFREE(wapi->pub->osh, cfg_priv->wapi_ie,
				cfg_priv->wapi_ie_len);
			cfg_priv->wapi_ie = NULL;
			cfg_priv->wapi_ie_len = 0;
		}

		if (p_len == 0)
			break;

		if (!(cfg_priv->wapi_ie = MALLOC(wapi->pub->osh, p_len))) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wapi->pub->unit, __FUNCTION__,
				MALLOCED(wapi->pub->osh)));
			err = BCME_NOMEM;
			break;
		}
		cfg_priv->wapi_ie_len = p_len;
		bcopy((uint8*)params, cfg_priv->wapi_ie, p_len);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wlc_wapi_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_wapi_bsscfg_cubby_pub_t *cfg_pub = WAPI_BSSCFG_PUB(wapi->wlc, cfg);

	ASSERT(cfg_pub != NULL);

	/* disable WAI authentication by default */
	cfg_pub->wai_restrict = FALSE;

	return BCME_OK;
}

static void
wlc_wapi_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_wapi_bsscfg_cubby_priv_t *cfg_priv;

	cfg_priv = WAPI_BSSCFG_PRIV(wapi, cfg);
	ASSERT(cfg_priv != NULL);

	/* free wapi_ie */
	if (cfg_priv->wapi_ie) {
		MFREE(wapi->pub->osh, cfg_priv->wapi_ie, cfg_priv->wapi_ie_len);
		cfg_priv->wapi_ie = NULL;
		cfg_priv->wapi_ie_len = 0;
	}

	return;
}

/* WAI rekey for mcast/unicast key */
static void
wlc_wapi_wai_rekey(wlc_wapi_info_t *wapi, wlc_bsscfg_t *cfg, struct ether_addr *addr)
{
	/* sendup a rekey event */
	wlc_wapi_station_event(wapi, cfg, addr, NULL, NULL,
		ETHER_ISNULLADDR(addr) ? WAPI_MUTIL_REKEY : WAPI_UNICAST_REKEY);

	return;
}

/* decode wapi IE to retrieve mcast/unicast ciphers and auth modes */
static int
wlc_wapi_parse_ie(wlc_wapi_info_t *wapi, bcm_tlv_t *wapiie, wlc_bss_info_t *bi)
{
	int len = wapiie->len;		/* value length */
	wpa_suite_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint8 *cap;
	uint16 count;
	uint i, j;

	/* Check min length and version */
	if (len < WAPI_IE_MIN_LEN) {
		WL_ERROR(("wl%d: %s: WAPI IE illegally short %d\n",
		          wapi->pub->unit, __FUNCTION__, len));
		return BCME_UNSUPPORTED;
	}

	if ((len < WAPI_VERSION_LEN) ||
	    (ltoh16_ua(wapiie->data) != WAPI_VERSION)) {
		WL_ERROR(("wl%d: unsupported WAPI version %d\n", wapi->pub->unit,
			ltoh16_ua(wapiie->data)));
		return BCME_UNSUPPORTED;
	}
	len -= WAPI_VERSION_LEN;

	/* Default WAPI parameters */
	bi->wapi.flags = RSN_FLAGS_SUPPORTED;
	bi->wapi.multicast = WAPI_CIPHER_SMS4;
	bi->wapi.ucount = 1;
	bi->wapi.unicast[0] = WAPI_CIPHER_SMS4;
	bi->wapi.acount = 1;
	bi->wapi.auth[0] = RSN_AKM_UNSPECIFIED;

	/* Check for auth key management suite(s) */
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&wapiie->data[WAPI_VERSION_LEN];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wapi.auth) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WAPI_OUI, DOT11_OUI_LEN)) {
			if (IS_WAPI_AKM(mgmt->list[i].type))
				bi->wapi.auth[j++] = mgmt->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WAPI auth %d\n",
					wapi->pub->unit, mgmt->list[i].type));
		} else
			WL_INFORM(("wl%d: unsupported proprietary auth OUI "
				   "%02X:%02X:%02X\n", wapi->pub->unit,
				   mgmt->list[i].oui[0], mgmt->list[i].oui[1],
				   mgmt->list[i].oui[2]));
	}
	bi->wapi.acount = (uint8)j;
	bi->flags |= WLC_BSS_WAPI;

	/* jump to unicast suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for unicast suites */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: no unicast suite\n", wapi->pub->unit));
		return BCME_UNSUPPORTED;
	}

	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mgmt->list[count];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wapi.unicast) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WAPI_OUI, DOT11_OUI_LEN)) {
			if (IS_WAPI_CIPHER(ucast->list[i].type))
				bi->wapi.unicast[j++] = WAPI_CSE_WPI_2_CIPHER(ucast->list[i].type);
			else
				WL_INFORM(("wl%d: unsupported WAPI unicast cipher %d\n",
					wapi->pub->unit, ucast->list[i].type));
		} else
			WL_INFORM(("wl%d: unsupported proprietary unicast cipher OUI "
				   "%02X:%02X:%02X\n", wapi->pub->unit,
				   ucast->list[i].oui[0], ucast->list[i].oui[1],
				   ucast->list[i].oui[2]));
	}
	bi->wapi.ucount = (uint8)j;

	/* jump to mcast suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wapi->pub->unit));
		return BCME_UNSUPPORTED;
	}

	/* pick up multicast cipher if we know what it is */
	mcast = (wpa_suite_mcast_t *)&ucast->list[count];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WAPI_OUI, DOT11_OUI_LEN)) {
		if (IS_WAPI_CIPHER(mcast->type))
			bi->wapi.multicast = WAPI_CSE_WPI_2_CIPHER(mcast->type);
		else
			WL_INFORM(("wl%d: unsupported WAPI multicast cipher %d\n",
				wapi->pub->unit, mcast->type));
	} else
		WL_INFORM(("wl%d: unsupported proprietary multicast cipher OUI "
			   "%02X:%02X:%02X\n", wapi->pub->unit,
			   mcast->oui[0], mcast->oui[1], mcast->oui[2]));

	/* RSN capabilities is optional */
	if (len < RSN_CAP_LEN) {
		WL_INFORM(("wl%d: no rsn cap\n", wapi->pub->unit));
		/* it is ok to not have RSN Cap */
		return 0;
	}

	/* parse RSN capabilities */
	cap = (uint8 *)&mcast[1];
	if (cap[0] & WAPI_CAP_PREAUTH)
		bi->wapi.flags |= RSN_FLAGS_PREAUTH;

	return 0;
}

static uint8 *
wlc_wapi_write_ie_safe(wlc_wapi_info_t *wapi, uint8 *cp, int buflen, uint32 WPA_auth,
	uint32 wsec, wlc_bsscfg_t *bsscfg)
{
	/* Infrastructure WAPI info element */
	uint WPA_len = 0;	/* tag length */
	bcm_tlv_t *wapiie = (bcm_tlv_t *)cp;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *auth;
	uint16 count;
	uint8 *cap;
	uint8 *orig_cp = cp;
	wlc_wapi_bsscfg_cubby_priv_t *cfg_priv = WAPI_BSSCFG_PRIV(wapi, bsscfg);

	ASSERT(cfg_priv != NULL);

	if (!INCLUDES_WAPI_AUTH(bsscfg->WPA_auth) || !WSEC_ENABLED(bsscfg->wsec))
		return cp;

	WL_WSEC(("wl%d: adding RSN IE, wsec = 0x%x\n", wapi->pub->unit, wsec));

	/* perform length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN((&wapiie->data[WAPI_VERSION_LEN] - &wapiie->id), buflen, orig_cp);

	/* fixed portion */
	wapiie->id = DOT11_MNG_WAPI_ID;
	wapiie->data[0] = (uint8)WAPI_VERSION;
	wapiie->data[1] = (uint8)(WAPI_VERSION>>8);
	WPA_len = WAPI_VERSION_LEN;

	/* authenticated key management suite list */
	auth = (wpa_suite_auth_key_mgmt_t *)&wapiie->data[WAPI_VERSION_LEN];
	count = 0;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_cp);

	WPA_len += WPA_IE_SUITE_COUNT_LEN;
	buflen -= WPA_IE_SUITE_COUNT_LEN;

	if (WPA_auth & WAPI_AUTH_UNSPECIFIED) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);
		bcopy(WAPI_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = RSN_AKM_UNSPECIFIED;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	if (WPA_auth & WAPI_AUTH_PSK) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);
		bcopy(WAPI_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = RSN_AKM_PSK;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}

	ASSERT(count);
	auth->count.low = (uint8)count;
	auth->count.high = (uint8)(count>>8);

	/* unicast suite list */
	ucast = (wpa_suite_ucast_t *)&auth->list[count];
	count = 0;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_cp);

	WPA_len += WPA_IE_SUITE_COUNT_LEN;
	buflen -= WPA_IE_SUITE_COUNT_LEN;

	if (WSEC_SMS4_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);
		bcopy(WAPI_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WAPI_CIPHER_2_CSE_WPI(WAPI_CIPHER_SMS4);
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	ASSERT(count);
	ucast->count.low = (uint8)count;
	ucast->count.high = (uint8)(count>>8);

	/* multicast suite */
	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_cp);
	mcast = (wpa_suite_mcast_t *)&ucast->list[count];
	bcopy(WAPI_OUI, mcast->oui, DOT11_OUI_LEN);
	mcast->type = WAPI_CIPHER_2_CSE_WPI(wlc_wpa_mcast_cipher(wapi->wlc, bsscfg));
	WPA_len += WPA_SUITE_LEN;
	buflen -= WPA_SUITE_LEN;

	/* WAPI capabilities */
	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_CAP_LEN, buflen, orig_cp);

	cap = (uint8 *)&mcast[1];
	cap[0] = 0;
	cap[1] = 0;
	if (BSSCFG_AP(bsscfg) && (bsscfg->WPA_auth & WAPI_AUTH_UNSPECIFIED) &&
	    (cfg_priv->wai_preauth == TRUE))
	    cap[0] = WAPI_CAP_PREAUTH;
	WPA_len += WPA_CAP_LEN;
	buflen -= WPA_CAP_LEN;

	/* update tag length */
	wapiie->len = (uint8)WPA_len;

	if (WPA_len)
		cp += TLV_HDR_LEN + WPA_len;

	return (cp);
}

#if !defined(WLNOEIND)
/* Send BRCM encapsulated WAI Events to applications. */
void
wlc_wapi_bss_wai_event(wlc_wapi_info_t *wapi, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea,
	uint8 *data, uint32 len)
{
	wlc_event_t *e;

	/* 'data' should point to a WAI header */
	if (data == NULL || len == 0) {
		WL_ERROR(("wl%d: wai missing", wapi->pub->unit));
		return;
	}

	e = wlc_event_alloc(wapi->wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s wlc_event_alloc failed\n", wapi->pub->unit, __FUNCTION__));
		return;
	}

	e->event.event_type = WLC_E_WAI_MSG;
	e->event.status = WLC_E_STATUS_SUCCESS;
	e->event.reason = 0;
	e->event.auth_type = 0;

	e->event.datalen = len;
	e->data = MALLOCZ(wapi->pub->osh, e->event.datalen);
	if (e->data == NULL) {
		wlc_event_free(wapi->wlc->eventq, e);
		WL_ERROR(("wl%d: %s MALLOc failed\n", wapi->pub->unit, __FUNCTION__));
		return;
	}

	wlc_event_if(wapi->wlc, bsscfg, e, ea);

	WL_WSEC(("wl%d: notify WAPID of WAI frame data len %d\n", wapi->pub->unit, len));
	wlc_process_event(wapi->wlc, e);
}
#endif /* !WLNOEIND */

void
wlc_wapi_station_event(wlc_wapi_info_t *wapi, wlc_bsscfg_t *bsscfg, const struct ether_addr *addr,
	void *ie, uint8 *gsn, uint16 msg_type)
{
	uint8 ie_len;
	struct wapi_sta_msg_t wapi_sta_msg;
	uint32 *dst_iv, *src_iv;
	int i, swap_len = IV_LEN / sizeof(uint32) - 1;

	memset(&wapi_sta_msg, 0, sizeof(struct wapi_sta_msg_t));

	switch (msg_type) {
	case WAPI_STA_AGING:
	case WAPI_UNICAST_REKEY:
	case WAPI_MUTIL_REKEY:
	case WAPI_STA_STATS:
		break;

	case WAPI_WAI_REQUEST:
		ASSERT(gsn);
		ASSERT(ie);
		dst_iv = (uint32 *)wapi_sta_msg.gsn;
		src_iv = (uint32 *)gsn;
		src_iv += swap_len;
		for (i = 0; i <= swap_len; i++) {
			memcpy(dst_iv, src_iv, sizeof(uint32));
			dst_iv++;
			src_iv--;
		}
		ie_len = ((uint8*)ie)[1] + 2; /* +2: wapi ie id+ wapi ie len */
		memcpy(wapi_sta_msg.wie, ie, ie_len);
		break;

	default:
		WL_ERROR(("wl%d: %s failed, unknown msg_type %d\n",
			wapi->pub->unit, __FUNCTION__, msg_type));
		return;
	}

	wapi_sta_msg.msg_type = msg_type;
	wapi_sta_msg.datalen = sizeof(struct wapi_sta_msg_t);
	memcpy(wapi_sta_msg.vap_mac, bsscfg->cur_etheraddr.octet, 6);
	memcpy(wapi_sta_msg.sta_mac, addr, 6);

	wlc_bss_mac_event(wapi->wlc, bsscfg, WLC_E_WAI_STA_EVENT, addr, 0, 0, 0,
		&wapi_sta_msg, wapi_sta_msg.datalen);

	return;
}

#ifdef STA
/* WAPI IE */
static uint
wlc_wapi_arq_calc_wapi_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (IS_WAPI_AUTH(cfg->WPA_auth)) {
		wlc_wapi_bsscfg_cubby_priv_t *cfg_priv;

		cfg_priv = WAPI_BSSCFG_PRIV(wapi, cfg);
		ASSERT(cfg_priv != NULL);

		return cfg_priv->wapi_ie_len;
	}
	return 0;
}

static int
wlc_wapi_arq_write_wapi_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (IS_WAPI_AUTH(cfg->WPA_auth)) {
		wlc_wapi_bsscfg_cubby_priv_t *cfg_priv;

		cfg_priv = WAPI_BSSCFG_PRIV(wapi, cfg);
		ASSERT(cfg_priv != NULL);

		bcm_copy_tlv(cfg_priv->wapi_ie, data->buf);
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef AP
/* Return non-zero if cipher is enabled. */
static int
wpa_cipher_enabled(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int cipher)
{
	int ret = 0;
	uint32 wsec = bsscfg->wsec;

	(void)wlc;

	switch (cipher) {
	case WAPI_CIPHER_SMS4:
		ret = WSEC_SMS4_ENABLED(wsec);
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

static int
wlc_wapi_check_ie(wlc_wapi_info_t *wapi, wlc_bsscfg_t *bsscfg, bcm_tlv_t *wapiie, uint32 *auth,
	uint32 *wsec)
{
	uint8 len = wapiie->len;
	uint32 WPA_auth = bsscfg->WPA_auth;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;

	/* Check min length and version */
	if (len < WAPI_IE_MIN_LEN) {
		WL_ERROR(("wl%d: %s: WAPI IE illegally short %d\n",
		          wapi->pub->unit, __FUNCTION__, len));
		return 1;
	}

	if ((len < WAPI_VERSION_LEN) ||
	    (ltoh16_ua(wapiie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WAPI version %d\n", wapi->pub->unit,
			ltoh16_ua(wapiie->data)));
		return 1;
	}
	len -= WAPI_VERSION_LEN;

	/* Check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&wapiie->data[WAPI_VERSION_LEN];
	count = ltoh16_ua(&mgmt->count);
	if ((count != 1) ||
	    !((bcmp(mgmt->list[0].oui, WAPI_OUI, DOT11_OUI_LEN) == 0) &&
	      (((mgmt->list[0].type == RSN_AKM_UNSPECIFIED) &&
		(WPA_auth & WAPI_AUTH_UNSPECIFIED)) ||
	       ((mgmt->list[0].type == RSN_AKM_PSK) &&
		(WPA_auth & WAPI_AUTH_PSK))))) {
		WL_ERROR(("wl%d: bad AKM in WAPI IE.\n", wapi->pub->unit));
		return 1;
	}
	if (!bcmwpa_akm2WPAauth((uint8 *)&mgmt->list[0], &WPA_auth, FALSE)) {
		WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
			wapi->pub->unit, mgmt->list[0].oui[0], mgmt->list[0].oui[1],
			mgmt->list[0].oui[2], mgmt->list[0].type));
		return 1;
	}
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	*auth = WPA_auth;

	/* Check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mgmt->list[1];
	count = ltoh16_ua(&ucast->count);
	if (count != 1 ||
		bcmp(ucast->list[0].oui, WAPI_OUI, DOT11_OUI_LEN) ||
		!wpa_cipher_enabled(wapi->wlc, bsscfg,
		WAPI_CSE_WPI_2_CIPHER(ucast->list[0].type))) {
		WL_ERROR(("wl%d: bad unicast suite in WAPI IE.\n", wapi->pub->unit));
		return 1;
	}
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	bcmwpa_cipher2wsec(ucast->list[0].oui, wsec);

	/* Check the mcast cipher */
	mcast = (wpa_suite_mcast_t *)&ucast->list[1];
	if (bcmp(mcast->oui, WAPI_OUI, DOT11_OUI_LEN) ||
		!wpa_cipher_enabled(wapi->wlc, bsscfg, WAPI_CSE_WPI_2_CIPHER(mcast->type))) {
		WL_ERROR(("wl%d: WAPI mcast cipher %02x:%02x:%02x:%d not enabled\n",
			wapi->pub->unit, mcast->oui[0], mcast->oui[1], mcast->oui[2],
			mcast->type));
		return 1;
	}
	len -= WPA_SUITE_LEN;

	/* Optional RSN capabilities */
	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return 0;
}

static void
wlc_wapi_wai_req_ind(wlc_wapi_info_t *wapi, wlc_bsscfg_t *bsscfg, struct scb *scb)
{
	int i;
	wlc_key_t *defkey;
	wlc_key_info_t defkey_info;
	uint8 defPN[SMS4_WPI_PN_LEN];

	defkey = wlc_keymgmt_get_bss_tx_key(wapi->wlc->keymgmt, bsscfg,
		FALSE, &defkey_info);
	if (defkey_info.algo == CRYPTO_ALGO_SMS4) {
		(void)wlc_key_get_seq(defkey, defPN, sizeof(defPN), 0, TRUE);
	} else {
		/* AP is AE and STA is ASUE */
		for (i = 0; i < SMS4_WPI_PN_LEN;) {
			defPN[i++] = 0x36;
			defPN[i++] = 0x5C;
		}
		defPN[0] = 0x37;
		defPN[0] = 0x37;
	}

	wlc_wapi_station_event(wapi, bsscfg, &scb->ea, scb->wpaie,
		defPN, WAPI_WAI_REQUEST);
}
#endif /* AP */
#endif /* BCMWAPI_WAI */

#ifdef BCMDBG
static int
wlc_wapi_dump(wlc_wapi_info_t *wapi, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "wapi_hw_wpi %d\n", wapi->pub->_wapi_hw_wpi);

	return 0;
}

#ifdef BCMWAPI_WAI
static void
wlc_wapi_bsscfg_dump(void *context, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)context;
	wlc_wapi_bsscfg_cubby_pub_t *cfg_pub = WAPI_BSSCFG_PUB(wapi->wlc, cfg);
	wlc_wapi_bsscfg_cubby_priv_t *cfg_priv = WAPI_BSSCFG_PRIV(wapi, cfg);
	bcm_tlv_t *ie;

	ASSERT(cfg_pub != NULL);
	ASSERT(cfg_priv != NULL);

	bcm_bprintf(b, "wai_restrict: %d wai_preauth: %d\n",
	            cfg_pub->wai_restrict, cfg_priv->wai_preauth);

	if ((ie = (bcm_tlv_t *)cfg_priv->wapi_ie) != NULL) {
		int remain = cfg_priv->wapi_ie_len;

		bcm_bprintf(b, "  Assoc Req IEs:\n");
		while (remain >= TLV_HDR_LEN) {
			int ie_len = ie->len + TLV_HDR_LEN;

			if (ie_len <= remain) {
				bcm_bprintf(b, "    ");
				wlc_dump_ie(wapi->wlc, ie, b);
				bcm_bprintf(b, "\n");
			}

			ie = (bcm_tlv_t *)((uint8 *)ie + ie_len);

			remain -= ie_len;
		}
	}
}
#endif /* BCMWAPI_WAI */
#endif /* BCMDBG */

#ifdef BCMWAPI_WAI
#ifdef AP
static uint
wlc_wapi_bcn_calc_wapi_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 buf[257];

	/* TODO: needs a better way to calculate the IE length */

	if (!IS_WAPI_AUTH(cfg->WPA_auth))
		return 0;

	return (uint)(wlc_wapi_write_ie_safe(wapi, buf, sizeof(buf),
	                                     cfg->WPA_auth, cfg->wsec, cfg) - buf);
}

static int
wlc_wapi_bcn_write_wapi_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!IS_WAPI_AUTH(cfg->WPA_auth))
		return BCME_OK;

	wlc_wapi_write_ie_safe(wapi, data->buf, data->buf_len, cfg->WPA_auth, cfg->wsec, cfg);
	return BCME_OK;
}

static int
wlc_wapi_arq_parse_wapi_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_info_t *wlc = wapi->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (cfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(cfg->wsec)) {
		bcm_tlv_t *wpaie = (bcm_tlv_t *)data->ie;
		struct scb *scb = ftpparm->assocreq.scb;

		/* check WAPI AKM */
		if (wpaie == NULL)
			return BCME_OK;

		if (wlc_wapi_check_ie(wapi, cfg, wpaie, &scb->WPA_auth,	&scb->wsec)) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			WL_ERROR(("wl%d: %s: unsupported request in WAPI IE from %s\n",
			          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));
			ftpparm->assocreq.status = DOT11_SC_ASSOC_FAIL;
			return BCME_ERROR;
		}

		wlc_wapi_wai_req_ind(wapi, cfg, scb);

		return wlc_scb_save_wpa_ie(wlc, scb, wpaie);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_wapi_scan_parse_wapi_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_wapi_info_t *wapi = (wlc_wapi_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (data->ie == NULL)
		return BCME_OK;

	wlc_wapi_parse_ie(wapi, (bcm_tlv_t *)data->ie, ftpparm->scan.result);

	return BCME_OK;
}
#endif /* STA */
#endif /* BCMWAPI_WAI */
