/**
 * AKM (Authentication and Key Management) module source, 802.1x related.
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
 * $Id: wlc_akm_ie.c 782619 2019-12-27 08:38:47Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bsscfg.h>
#include <wlc_keymgmt.h>

#include <wlc_akm_ie.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <wlc_sup.h>
#endif // endif
#include <wlc_scb.h>
#ifdef MFP
#include <wlc_mfp.h>
#endif // endif
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#include <wlc_pmkid.h>
#if defined(WLFBT)
#include <wlc_fbt.h>
#endif // endif
#include <wlc_hs20.h>

/*
 * iovars
 */
enum {
	IOV_WPA_CAP,
	IOV_WPAIE,
	IOV_LAST
};

/* iovar table */
static const bcm_iovar_t akm_iovars[] = {
	{"wpa_cap", IOV_WPA_CAP, IOVF_OPEN_ALLOW, IOVT_UINT16, 0},
	{"wpaie", IOV_WPAIE, IOVF_OPEN_ALLOW, IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0}
};

/*
 * module private states
 */
struct wlc_akm_info {
	wlc_info_t *wlc;
	int cfgh;
};

/* bsscfg private states */
typedef struct {
	bool wpa2_preauth;	/* default is TRUE, wpa_cap sets value */
} bss_akm_info_t;

/* access macros */
#define BSS_AKM_INFO(akm, cfg) BSSCFG_CUBBY(cfg, (akm)->cfgh)

/*
 * local variables
 */
/* WPA/RSN IE template */
static const uint8 WPA_info_element[] = {
	DOT11_MNG_WPA_ID, 0x18,
	0x00, 0x50, 0xf2, 0x01, 0x01, 0x00,
	0x00, 0x50, 0xf2, 0xff,
	0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
	0x01, 0x00, 0x00, 0x50, 0xf2, 0xff,
	0x00, 0x00
};

static const uint8 WPA2_info_element[] = {
	DOT11_MNG_RSN_ID, 0x14, 0x01, 0x00,
	0x00, 0x0F, 0xAC, 0xff,
	0x01, 0x00, 0x00, 0x0F, 0xAC, 0xff,
	0x01, 0x00, 0x00, 0x0F, 0xAC, 0xff,
	0x00, 0x00
};

static const uint8 OSEN_info_element[] = {
	DOT11_MNG_VS_ID,		/* Element ID */
	0x1C,				/* Length = 4 + OSENE Payload */
	0x50, 0x6F, 0x9A, 0x12,		/* OI, Type Field [4] */
	0x00, 0x0F, 0xAC, 0x07,		/* Group Data Cipher Suite */
	0x01, 0x00,			/* Pairwise Cipher Suite Count */
	0x00, 0x0F, 0xAC, 0x04,		/* Pairwise Cipher Suite List */
	0x01, 0x00,			/* AKM Suite Count */
	0x50, 0x6F, 0x9A, 0x01,		/* AKM Suite List */
	0x00, 0x00,			/* RSN Capability */
	0x00, 0x00,			/* PKMID Count, No PMKID List */
	0x00, 0x00, 0x00, 0x00		/* Group Management Cipher Suite */
};

static int wlc_wpa_cap(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *cap, int len);
#ifdef AP
static uint8 *wlc_write_wpa_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, int buflen);
static uint8 *wlc_write_rsn_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, int buflen);
static int wlc_wpa_set_cap(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *cap, int len);
#endif /* AP */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* return WPA capabilities */
static int
wlc_wpa_cap(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *cap, int len)
{
	uint8 cntr;
	bss_akm_info_t *bai = BSS_AKM_INFO(wlc->akmi, cfg);

	ASSERT(len >= WPA_CAP_LEN);

	if (len < WPA_CAP_LEN)
		return -1;

	/* RSN capabilities */
	cntr = BSS_WME_ENAB(wlc, cfg) ? WLC_REPLAY_CNTRS_VALUE : 0;
	cap[0] = cntr << WPA_CAP_REPLAY_CNTR_SHIFT;
	if (BSSCFG_AP(cfg) &&
	    (cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) &&
	    (bai->wpa2_preauth == TRUE))
		cap[0] |= WPA_CAP_WPA2_PREAUTH;
	cap[1] = 0;

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub))
		cap[0] |= wlc_mfp_get_rsn_caps(wlc->mfp, cfg);
#endif // endif

	if (BSSCFG_IS_TDLS(cfg)) {
		cap[0] = 0;
		cap[1] |= WPA_CAP_PEER_KEY_ENABLE;
	}

	return 0;
}

#ifdef AP
static int
wlc_wpa_set_cap(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *cap, int len)
{
	bss_akm_info_t *bai = BSS_AKM_INFO(wlc->akmi, cfg);
	int ret_val = -1;

	ASSERT(len >= WPA_CAP_LEN);

	if (len < WPA_CAP_LEN)
		return ret_val;

	if (BSSCFG_AP(cfg)) {
		if (cap[0] & WPA_CAP_WPA2_PREAUTH)
			bai->wpa2_preauth = TRUE;
		else
			bai->wpa2_preauth = FALSE;
		WL_INFORM(("wl%d: wlc_wpa_set_cap: setting the wpa2 preauth %d\n", wlc->pub->unit,
			bai->wpa2_preauth));
		ret_val = 0;
	}
	return ret_val;
}

/* build IEs */
static uint8 *
wlc_write_wpa_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, int buflen)
{
	uint WPA_len = 0;	/* tag length */
	uint32 WPA_auth = cfg->WPA_auth;
	uint wsec = cfg->wsec;

	if (!bcmwpa_includes_wpa_auth(WPA_auth) || !WSEC_ENABLED(wsec))
		return buf;

	WL_WSEC(("wl%d: adding WPA IE, wsec = 0x%x\n", wlc->pub->unit, wsec));

	/* IBSS WPA info element */
	if (!cfg->current_bss->infra) {
		/* copy WPA info element template */
		WPA_len = WPA_info_element[1];

		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + WPA_len), buflen, buf);

		bcopy(WPA_info_element, buf, TLV_HDR_LEN + WPA_len);

		/* select strongest multicast cipher available */
		if (WSEC_AES_ENABLED(wsec))
		    buf[11] = WPA_CIPHER_AES_CCM;
		else if (WSEC_TKIP_ENABLED(wsec))
		    buf[11] = WPA_CIPHER_TKIP;
		else {
			/* - w/ v.22 client, IE must specify WEP key size
			 *   which isn't available from config, so use the key...
			 */
			wlc_key_info_t key_info;

			wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, cfg, FALSE, &key_info);
			switch (key_info.algo) {
			case CRYPTO_ALGO_WEP1:
				buf[11] = WPA_CIPHER_WEP_40;
				break;
			case CRYPTO_ALGO_WEP128:
				buf[11] = WPA_CIPHER_WEP_104;
				break;
			default:
				WL_ERROR(("wl%d: %s: key  algo %d" " doesn't match wsec 0x%x\n",
					wlc->pub->unit, __FUNCTION__, key_info.algo, wsec));
				buf[11] = WPA_CIPHER_WEP_40;
				break;
			}
		}

		/* set unicast cipher and authenticated key management */
		buf[17] = WPA_CIPHER_NONE;
		buf[23] = RSN_AKM_NONE;

		/* WPA capabilities */
		wlc_wpa_cap(wlc, cfg, &buf[24], WPA_CAP_LEN);
	} else {
		wpa_ie_fixed_t *wpaie = (wpa_ie_fixed_t *)buf;
		wpa_suite_mcast_t *mcast;
		wpa_suite_ucast_t *ucast;
		wpa_suite_auth_key_mgmt_t *auth;
		uint16 count;
		uint8 *orig_buf = buf;
		int totlen;

		/* fixed portion */

		/* length check */
		/* if buffer too small, return untouched buffer */
		totlen = (TLV_HDR_LEN + WPA_IE_TAG_FIXED_LEN) +
			WPA_SUITE_LEN + WPA_IE_SUITE_COUNT_LEN;
		BUFLEN_CHECK_AND_RETURN(totlen, buflen, orig_buf);
		buflen -= totlen;

		wpaie->tag = DOT11_MNG_WPA_ID;
		bcopy(WPA_OUI, wpaie->oui, sizeof(wpaie->oui));
		wpaie->oui_type = 1;
		wpaie->version.low = (uint8)WPA_VERSION;
		wpaie->version.high = (uint8)(WPA_VERSION>>8);
		WPA_len = WPA_IE_TAG_FIXED_LEN;

		/* multicast suite */
		mcast = (wpa_suite_mcast_t *)&wpaie[1];
		bcopy(WPA_OUI, mcast->oui, DOT11_OUI_LEN);
		mcast->type = wlc_wpa_mcast_cipher(wlc, cfg);
		WPA_len += WPA_SUITE_LEN;

		/* unicast suite list */
		ucast = (wpa_suite_ucast_t *)&mcast[1];
		count = 0;
		WPA_len += WPA_IE_SUITE_COUNT_LEN;

		/* XXX - exclude AES suite from WPA IE. Defined in customer
		 * builds to exclude announcement of AES suite.
		 */
#ifndef WLC_WPA_IE_EXCLUDE_AES_SUITE
		if (WSEC_AES_ENABLED(wsec)) {
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
			ucast->list[count++].type = WPA_CIPHER_AES_CCM;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
#endif /* WLC_WPA_MIXEDMODE_NO_AES */
		if (WSEC_TKIP_ENABLED(wsec)) {
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
			ucast->list[count++].type = WPA_CIPHER_TKIP;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
		ASSERT(count);
		ucast->count.low = (uint8)count;
		ucast->count.high = (uint8)(count>>8);

		/* authenticated key management suite list */
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_buf);
		auth = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
		count = 0;
		WPA_len += WPA_IE_SUITE_COUNT_LEN;
		buflen -= WPA_IE_SUITE_COUNT_LEN;

		ASSERT(!(WPA_auth & WPA_AUTH_NONE));
		if (WPA_auth & WPA_AUTH_UNSPECIFIED) {
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA_OUI, auth->list[count].oui, DOT11_OUI_LEN);
			auth->list[count++].type = RSN_AKM_UNSPECIFIED;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
		if (WPA_auth & WPA_AUTH_PSK) {
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA_OUI, auth->list[count].oui, DOT11_OUI_LEN);
			auth->list[count++].type = RSN_AKM_PSK;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
		ASSERT(count);
		auth->count.low = (uint8)count;
		auth->count.high = (uint8)(count>>8);

		/* update tag length */
		wpaie->length = (uint8)WPA_len;
	}

	if (WPA_len)
		buf += TLV_HDR_LEN + WPA_len;

	return (buf);
}

static uint8 *
wlc_write_rsn_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, int buflen)
{
	/* Infrastructure WPA info element */
	uint WPA_len = 0;	/* tag length */
	bcm_tlv_t *wpa2ie = (bcm_tlv_t *)buf;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *auth;
	uint16 count;
	uint8 *cap;
	uint8 *orig_buf = buf;
	int totlen;
	uint32 WPA_auth = cfg->WPA_auth;
	uint wsec = cfg->wsec;
	uint8 akm_type;

	if ((!bcmwpa_includes_rsn_auth(WPA_auth) &&
#ifdef WL_SAE
		!bcmwpa_includes_wpa3_auth(WPA_auth) &&
#endif // endif
		TRUE) ||
		!WSEC_ENABLED(wsec)) {
		return buf;
	}
	WL_WSEC(("wl%d: adding RSN IE, wsec = 0x%x\n", wlc->pub->unit, wsec));

	/* perform length check */
	/* if buffer too small, return untouched buffer */
	totlen = (int)(&wpa2ie->data[WPA2_VERSION_LEN] - &wpa2ie->id) +
		WPA_SUITE_LEN + WPA_IE_SUITE_COUNT_LEN;
	BUFLEN_CHECK_AND_RETURN(totlen, buflen, orig_buf);
	buflen -= totlen;

	/* fixed portion */
	wpa2ie->id = DOT11_MNG_RSN_ID;
	wpa2ie->data[0] = (uint8)WPA2_VERSION;
	wpa2ie->data[1] = (uint8)(WPA2_VERSION>>8);
	WPA_len = WPA2_VERSION_LEN;

	/* multicast suite */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];

#if defined(WLTDLS)
	if (WPA_auth == WPA2_AUTH_TPK) {
		bcopy(WPA2_OUI, mcast->oui, DOT11_OUI_LEN);
		mcast->type = WPA_CIPHER_TPK;
	} else
#endif /* WLTDLS */
	{
		bcopy(WPA2_OUI, mcast->oui, DOT11_OUI_LEN);
		mcast->type = wlc_wpa_mcast_cipher(wlc, cfg);
	}
	WPA_len += WPA_SUITE_LEN;

	/* unicast suite list */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = 0;

	WPA_len += WPA_IE_SUITE_COUNT_LEN;

	if (WSEC_AES_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);

		bcopy(WPA2_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WPA_CIPHER_AES_CCM;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	if (WSEC_TKIP_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);

		bcopy(WPA2_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WPA_CIPHER_TKIP;
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
	BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_buf);

	WPA_len += WPA_IE_SUITE_COUNT_LEN;
	buflen -= WPA_IE_SUITE_COUNT_LEN;

	if (WPA_auth & (WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_1X_SHA256)) {
		akm_type = RSN_AKM_UNSPECIFIED;
		if (WPA_auth & WPA2_AUTH_1X_SHA256)
			akm_type = RSN_AKM_SHA256_1X;
#ifdef MFP
		else if (BSSCFG_IS_MFP_REQUIRED(cfg))
			akm_type = RSN_AKM_SHA256_1X;
#endif /* MFP */
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = akm_type;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	if (WPA_auth & (WPA2_AUTH_PSK | WPA2_AUTH_PSK_SHA256)) {
		akm_type = RSN_AKM_PSK;
		if (WPA_auth & WPA2_AUTH_PSK_SHA256)
			akm_type = RSN_AKM_SHA256_PSK;
#ifdef MFP
		else if (BSSCFG_IS_MFP_REQUIRED(cfg))
			akm_type = RSN_AKM_SHA256_PSK;
#endif /* MFP */
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = akm_type;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
#ifdef WLFBT
	if (BSSCFG_IS_FBT(cfg)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = BSSCFG_IS_FBT_1X(cfg) ? RSN_AKM_FBT_1X : RSN_AKM_FBT_PSK;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
#endif /* WLFBT */
#ifdef WL_SAE
		if (WPA_auth & WPA3_AUTH_SAE_PSK) {
			akm_type = RSN_AKM_SAE_PSK;
			if (BSSCFG_STA(cfg)) {
				/* if auth algorithm is OPEN SYSTEM, demote the AKM to WPA2 PSK */
				if (cfg->auth == DOT11_OPEN_SYSTEM) {
					akm_type = RSN_AKM_PSK;
				}
			}

			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
			auth->list[count++].type = akm_type;
#ifdef MFP
			/* If pure SAE, set MFP to required */
			if (!(WPA_auth & WPA2_AUTH_PSK)) {
				BSSCFG_SET_AKM_REQUIRES_MFP(cfg);
			} else {
				BSSCFG_SET_MFP_CAPABLE(cfg);
			}
#endif /* MFP */
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
#endif /* WL_SAE */
#if defined(WLTDLS)
	if (WPA_auth & WPA2_AUTH_TPK) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = RSN_AKM_TPK;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
#endif /* WLTDLS */

#if defined(IBSS_PSK)
	if (WPA_auth & BRCM_AUTH_PSK) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(BRCM_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = BRCM_AKM_PSK;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
#endif /* IBSS_PSK */
	ASSERT(count);
	auth->count.low = (uint8)count;
	auth->count.high = (uint8)(count>>8);

	/* WPA capabilities */
	cap = (uint8 *)&auth->list[count];
	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_CAP_LEN, buflen, orig_buf);
	wlc_wpa_cap(wlc, cfg, cap, WPA_CAP_LEN);
	WPA_len += WPA_CAP_LEN;
	buflen -= WPA_CAP_LEN;

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub) && BSSCFG_IS_MFP_CAPABLE(cfg)) {
		wpa_suite_t bip;
		if (mfp_get_bip(wlc, cfg, &bip)) {
			/* PMKID: PMKID count = 0 for AP */
			BUFLEN_CHECK_AND_RETURN(WPA2_PMKID_COUNT_LEN, buflen, orig_buf);
			memset(cap + RSN_CAP_LEN, 0, WPA2_PMKID_COUNT_LEN);
			WPA_len += WPA2_PMKID_COUNT_LEN;
			buflen -= WPA2_PMKID_COUNT_LEN;

			/* BIP */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			/* Advertise only correct cipher suite for group management frames */
			memcpy(cap + RSN_CAP_LEN + WPA2_PMKID_COUNT_LEN, (uint8 *)&bip,
				WPA_SUITE_LEN);
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
	} else {
		/* If BIP is not plumbed by host, skip inclusion
		 * of pmkid field too (to be in sync with ext
		 * supplciant generated RSNIE. Otherwise it will lead
		 * to EAPOL 3/4 RSN IE mismatch.
		 */
		WL_WSEC(("wl%d: No BIP present. Skip BIP inclusion.\n",
				wlc->pub->unit));
	}
#endif /* MFP */

	/* update tag length */
	wpa2ie->len = (uint8)WPA_len;

	if (WPA_len)
		buf += TLV_HDR_LEN + WPA_len;

	return (buf);
}

/* RSN IE in bcn/prbrsp */
static uint
wlc_akm_bcn_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 buf[257];

	/* TODO: need to do better to calculate the IE length... */

	return (uint)(wlc_write_rsn_ie_safe(wlc, cfg, buf, sizeof(buf)) - buf);
}

static int
wlc_akm_bcn_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* RSN IE */
	wlc_write_rsn_ie_safe(wlc, cfg, data->buf, data->buf_len);

	return BCME_OK;
}

#ifdef WLOSEN
static uint
wlc_akm_bcn_calc_osen_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no RSN IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_bcn_calc_rsn_ie_len(ctx, data);
}

static int
wlc_akm_bcn_write_osen_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no RSN IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_bcn_write_rsn_ie(ctx, data);
}
#endif	/* WLOSEN */
#endif /* AP */

#ifdef STA
#ifdef IBSS_PEER_DISCOVERY_EVENT
static int
wlc_akm_bcn_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_pparm_t *pparm = data->pparm;
	wlc_iem_ft_pparm_t *ftpparm = pparm->ft;
	struct scb *scb = ftpparm->bcn.scb;

	if (cfg->BSS) {
		return BCME_OK;
	}

	/* IBSS peer discovery */
	if (!IBSS_PEER_DISCOVERY_EVENT_ENAB(wlc->pub) || BSS_TDLS_ENAB(wlc, cfg))
		return BCME_OK;

	if (scb != NULL && !SCB_IS_IBSS_PEER(scb)) {

		SCB_SET_IBSS_PEER(scb);

		wlc_mac_event(wlc, WLC_E_IBSS_ASSOC, &scb->ea, 0, 0,
		              DOT11_BSSTYPE_INDEPENDENT,
		              data->ie, data->ie_len);
	}

	return BCME_OK;
}
#endif /* IBSS_PEER_DISCOVERY_EVENT */
#endif /* STA */

#ifdef AP
/* WPA Information Element in bcn/prbrsp */

static uint
wlc_akm_bcn_calc_wpa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 buf[257];

	/* TODO: need to do better to calculate the IE length... */

	return (uint)(wlc_write_wpa_ie_safe(wlc, cfg, buf, sizeof(buf)) - buf);
}

static int
wlc_akm_bcn_write_wpa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	wlc_write_wpa_ie_safe(wlc, cfg, data->buf, data->buf_len);

	return BCME_OK;
}

#ifdef WLOSEN
static uint
wlc_akm_bcn_calc_osen_wpa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no WPA IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_bcn_calc_wpa_ie_len(ctx, data);
}

static int
wlc_akm_bcn_write_osen_wpa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no WPA IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return BCME_OK;

	return wlc_akm_bcn_write_wpa_ie(ctx, data);
}

/* OSEN Information Element in bcn/prbrsp */
static uint
wlc_akm_bcn_calc_osen_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return sizeof(OSEN_info_element);

	return 0;
}

static int
wlc_akm_bcn_write_osen_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		bcopy(OSEN_info_element, data->buf, sizeof(OSEN_info_element));

	return BCME_OK;
}
#endif	/* WLOSEN */
#endif /* AP */

#ifdef STA
static void
wlc_rsn_build_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct rsn_parms *rsn, void *pmcast)
{
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint8 *cap;

	/* copy multicast cipher */
	mcast = (wpa_suite_mcast_t *)pmcast;
	mcast->type = (uint8)rsn->multicast;
#ifdef BCMCCX
	if (CCX_ENAB(wlc->pub) && IS_CCX_CIPHER(mcast->type)) {
		bcopy(CISCO_AIRONET_OUI, mcast->oui, DOT11_OUI_LEN);
		mcast->type -= CISCO_BASE;
	}
#endif /* BCMCCX */

	/* select unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	ucast->list[0].type = WPA_CIPHER_NONE;
	if (WSEC_AES_ENABLED(cfg->wsec) && UCAST_AES(rsn))
		ucast->list[0].type = WPA_CIPHER_AES_CCM;
	else if (WSEC_TKIP_ENABLED(cfg->wsec) && UCAST_TKIP(rsn))
		ucast->list[0].type = WPA_CIPHER_TKIP;
#ifdef BCMCCX
	else if (cfg->WPA_auth & WPA_AUTH_CCKM) {
		if (wlc_rsn_ucast_lookup(rsn, WPA_CIPHER_CKIP_MMH)) {
			ucast->list[0].type = WPA_CIPHER_CKIP_MMH;
		} else if (wlc_rsn_ucast_lookup(rsn, WPA_CIPHER_CKIP)) {
			ucast->list[0].type = WPA_CIPHER_CKIP;
		} else if (wlc_rsn_ucast_lookup(rsn, WPA_CIPHER_WEP_MMH)) {
			ucast->list[0].type = WPA_CIPHER_WEP_MMH;
		} else if (wlc_rsn_ucast_lookup(rsn, WPA_CIPHER_WEP_104)) {
			ucast->list[0].type = WPA_CIPHER_WEP_104;
		} else if (wlc_rsn_ucast_lookup(rsn, WPA_CIPHER_WEP_40)) {
			ucast->list[0].type = WPA_CIPHER_WEP_40;
		}
		if (IS_CCX_CIPHER(ucast->list[0].type)) {
			bcopy(CISCO_AIRONET_OUI, ucast->list[0].oui, DOT11_OUI_LEN);
			ucast->list[0].type -= CISCO_BASE;
		}
	}
#endif /* BCMCCX */

	/* copy authenticated key management */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[1];
	if (cfg->WPA_auth & WPA_AUTH_UNSPECIFIED)
		mgmt->list[0].type = RSN_AKM_UNSPECIFIED;
	else if (cfg->WPA_auth & WPA_AUTH_PSK)
		mgmt->list[0].type = RSN_AKM_PSK;
	else if (cfg->WPA_auth & (WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_1X_SHA256)) {
#ifdef WLFBT
		if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
			(rsn->flags & RSN_FLAGS_FBT))
			mgmt->list[0].type = RSN_AKM_FBT_1X;
		else
#endif /* WLFBT */
		/* special test mode: ignore AP settings */
		if ((cfg->WPA_auth & WPA2_AUTH_1X_SHA256) &&
		    wlc_rsn_akm_lookup(rsn, RSN_AKM_SHA256_1X))
			mgmt->list[0].type = RSN_AKM_SHA256_1X;
#ifdef MFP
		else if (WLC_MFP_ENAB(wlc->pub) && wlc_rsn_akm_lookup(rsn, RSN_AKM_SHA256_1X)) {
			mgmt->list[0].type = RSN_AKM_SHA256_1X;
		}
#endif /* MFP */
		else
			mgmt->list[0].type = RSN_AKM_UNSPECIFIED;
	}
	else if (cfg->WPA_auth & (WPA2_AUTH_PSK | WPA2_AUTH_PSK_SHA256)) {
#ifdef WLFBT
		if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
			(rsn->flags & RSN_FLAGS_FBT)) {
			mgmt->list[0].type = RSN_AKM_FBT_PSK;
		} else
#endif /* WLFBT */
		/* special test mode: ignore AP settings */
		if ((cfg->WPA_auth & WPA2_AUTH_PSK_SHA256) &&
		    wlc_rsn_akm_lookup(rsn, RSN_AKM_SHA256_PSK))
			mgmt->list[0].type = RSN_AKM_SHA256_PSK;
#ifdef MFP
		else if (WLC_MFP_ENAB(wlc->pub) && wlc_rsn_akm_lookup(rsn, RSN_AKM_SHA256_PSK)) {
			mgmt->list[0].type = RSN_AKM_SHA256_PSK;
		}
#endif /* MFP */
		else
			mgmt->list[0].type = RSN_AKM_PSK;
	}
#ifdef BCMCCX
	else if (IS_CCKM_AUTH(cfg->WPA_auth)) {
		bcopy(CISCO_AIRONET_OUI, mgmt->list[0].oui, DOT11_OUI_LEN);
		mgmt->list[0].type = RSN_AKM_NONE;
	}
#endif /* BCMCCX */

	/* WPA capabilities */
	cap = (uint8 *)&mgmt->list[1];
	wlc_wpa_cap(wlc, cfg, cap, WPA_CAP_LEN);

#ifdef MFP
	/* Reset MFPC if not supported by AP, for interop */
	if (WLC_MFP_ENAB(wlc->pub)) {
		cap[0] &= (~RSN_CAP_MFPC | rsn->cap[0]);
	}
#endif // endif

}

/* WPA2 IE in assoc/reassoc */
static void
wlc_akm_build_rsn_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi,
	bool reassoc, uint8 *ie, uint8 *buf, uint len)
{
	bcm_tlv_t *wpa2_ie = (bcm_tlv_t *)ie;
#if defined(WLFBT)
	bool fbt = BSSCFG_IS_FBT(cfg) &&
		(cfg->WPA_auth & WPA2_AUTH_FT) && (bi->wpa2.flags & RSN_FLAGS_FBT);
#endif /* WLFBT */
	bcm_tlv_t *wpa2;
	uint8 *pmkid = NULL;
	bool putpmkid = TRUE;

	/* WPA2 IE */
	if (wpa2_ie != NULL &&
#if defined(WLFBT)
	    !(fbt && reassoc) &&
#endif // endif
	    TRUE) {
		bool remove_pmkid = reassoc;
#ifdef BCMCCX
		remove_pmkid = remove_pmkid || (cfg->WPA_auth & WPA2_AUTH_CCKM);
#endif // endif
		/* uint8 WPA_len = wpa2_ie->len; */
		/* We have to remove existed pmkid created from external supplicant */
		wpa2 = wpa2_ie;
		if (remove_pmkid) {
/* XXX
 * Rdar:13477961/Jira:29999
 * An external (or internal) supplicant can use the pmkid event/info
 * mechanism to plumb the pmkids.
 * MacOSX supplicant manages the pmkid by including it in RSN IE
 */
#if !defined(MACOSX)
			wlc_remove_wpa2_pmkid(wlc, wpa2);
#endif // endif
		} else {
			putpmkid = FALSE;
		}

		/* copy WPA2 info element */
		wpa2_ie = (bcm_tlv_t *)buf;
		bcm_copy_tlv(wpa2, buf);
		/* wpa2->len = WPA_len; */
	}
	else {
		/* copy WPA2 info element template */
		wpa2_ie = (bcm_tlv_t *)buf;
		bcm_copy_tlv(WPA2_info_element, buf);

		/* fill in cipher suites, AKM, etc. */
		wlc_rsn_build_ie(wlc, cfg, &(bi->wpa2), &wpa2_ie->data[WPA2_VERSION_LEN]);
#if defined(WLFBT)
		if (fbt && reassoc) {
			WL_WSEC(("FBT override preauth\n"));
			pmkid = wlc_fbt_get_pmkr1name(wlc->fbt, cfg);
		}
#endif // endif
	}

	/* Add cached pmkid for non-FT EAP associations and for FT reassoc.
	 * Do not add pmkid for initial FT associations.
	 */
	if (putpmkid &&
#if defined(WLFBT)
			!(fbt && (reassoc == 0)) &&
#endif /* WLFBT */
			bcmwpa_is_rsn_auth(cfg->WPA_auth)) {
		/* use cached PMKIDs */
		buf += wlc_pmkid_putpmkid(wlc->pmkid_info, cfg, &bi->BSSID,
			wpa2_ie, pmkid, cfg->WPA_auth);
	}

	/* Save the akm type being used for the current association */
#if defined(WLFBT)
	if (BSSCFG_IS_FBT(cfg))
		wlc_fbt_save_current_akm(wlc->fbt, cfg, bi);
#endif /* WLFBT */
}

/* RSN in assocreq/reassocreq */
static uint
wlc_akm_arq_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* TODO: needs a better way to calculate the IE length */

	if (bcmwpa_is_rsn_auth(cfg->WPA_auth) && WSEC_ENABLED(cfg->wsec)) {
		wlc_bss_info_t *bi = data->cbparm->ft->assocreq.target;
		bool reassoc = data->ft == FC_REASSOC_REQ;
		uint8 buf[257];

		wlc_akm_build_rsn_ie(wlc, cfg, bi, reassoc, data->ie, buf, sizeof(buf));
		return TLV_HDR_LEN + buf[TLV_LEN_OFF];
	}

	return 0;
}

static int
wlc_akm_arq_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (bcmwpa_is_rsn_auth(cfg->WPA_auth) && WSEC_ENABLED(cfg->wsec)) {
		wlc_bss_info_t *bi = data->cbparm->ft->assocreq.target;
		bool reassoc = data->ft == FC_REASSOC_REQ;

		wlc_akm_build_rsn_ie(wlc, cfg, bi, reassoc, data->ie, data->buf, data->buf_len);
		data->cbparm->ft->assocreq.wpa2_ie = data->buf;
	}

	return BCME_OK;
}

#ifdef WLOSEN
static uint
wlc_akm_arq_calc_osen_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no RSN IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_arq_calc_rsn_ie_len(ctx, data);
}

static int
wlc_akm_arq_write_osen_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no RSN IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_arq_write_rsn_ie(ctx, data);
}
#endif	/* WLOSEN */

static void
wlc_akm_build_wpa_ie(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wlc_iem_cbparm_t *cbparm,
	uint8 *buf, uint len)
{
	wlc_bss_info_t *bi = cbparm->ft->assocreq.target;
	wpa_ie_fixed_t *wpa_ie = (wpa_ie_fixed_t *)buf;

	/* copy WPA info element template */
	bcm_copy_tlv(WPA_info_element, buf);

	/* fill in cipher suites, AKM, etc. */
	wlc_rsn_build_ie(wlc, cfg, &bi->wpa, &wpa_ie[1]);
}

/* WPA IE in assocreq/reassocreq */
static uint
wlc_akm_arq_calc_wpa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* TODO: needs a better way to calculate the IE length */

	if (bcmwpa_is_wpa_auth(cfg->WPA_auth) && WSEC_ENABLED(cfg->wsec)) {
		uint8 buf[257];

		wlc_akm_build_wpa_ie(wlc, cfg, data->cbparm, buf, sizeof(buf));
		return TLV_HDR_LEN + buf[TLV_LEN_OFF];
	}

	return 0;
}

static int
wlc_akm_arq_write_wpa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (bcmwpa_is_wpa_auth(cfg->WPA_auth) && WSEC_ENABLED(cfg->wsec)) {
		wlc_akm_build_wpa_ie(wlc, cfg, data->cbparm, data->buf, data->buf_len);
		data->cbparm->ft->assocreq.wpa_ie = data->buf;
	}

	return BCME_OK;
}

#ifdef WLOSEN
static uint
wlc_akm_arq_calc_osen_wpa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no WPA IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_arq_calc_wpa_ie_len(ctx, data);
}

static int
wlc_akm_arq_write_osen_wpa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no WPA IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return 0;

	return wlc_akm_arq_write_wpa_ie(ctx, data);
}

/* OSEN IE in assocreq/reassocreq */
static uint
wlc_akm_arq_calc_osen_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return sizeof(OSEN_info_element);

	return 0;
}

static int
wlc_akm_arq_write_osen_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		bcopy(OSEN_info_element, data->buf, sizeof(OSEN_info_element));

	return BCME_OK;
}
#endif	/* WLOSEN */
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
	case WPA_CIPHER_NONE:
		ret = !WSEC_ENABLED(wsec);
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		ret = WSEC_WEP_ENABLED(wsec);
		if (ret && wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg)) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", wlc->pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_TKIP:
		ret = WSEC_TKIP_ENABLED(wsec);
		if (ret && wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg)) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", wlc->pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_AES_CCM:
		ret = WSEC_AES_ENABLED(wsec);
		break;
	case WPA_CIPHER_AES_OCB:
	default:
		ret = 0;
		break;
	}
	return ret;
}

static int
wpa_mcast_cipher_allowed(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int cipher)
{
	int ret = 0;
	bool mixed = FALSE;
	uint32 wsec = bsscfg->wsec;

	(void)wlc;

	switch (cipher) {
	case WPA_CIPHER_NONE:
		ret = !WSEC_ENABLED(wsec);
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		ret = WSEC_WEP_ENABLED(wsec);
		if (ret && wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg)) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", wlc->pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_TKIP:
		ret = WSEC_TKIP_ENABLED(wsec);
		if (ret && wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg)) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", wlc->pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_AES_CCM:
		mixed = WSEC_AES_ENABLED(wsec) && WSEC_TKIP_ENABLED(wsec);
		/* Do not allow CCMP for group cipher in mixed mode */
		if (mixed) {
			WL_WSEC(("wl%d: Reject CCMP for group cipher in mixed mode\n",
				wlc->pub->unit));
			ret = 0;
		} else {
			ret = WSEC_AES_ENABLED(wsec);
		}
		break;
	case WPA_CIPHER_AES_OCB:
	default:
		ret = 0;
		break;
	}
	return ret;
}

#define WPA_OUI_OK(oui) (bcmp(WPA_OUI, (oui), 3) == 0)

static int
wlc_check_wpaie(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *wpaie, uint32 *auth, uint32 *wsec)
{
	uint8 ie_len = wpaie[1];
	uint16 ver;
	int n_suites, offset;
	uint32 WPA_auth = bsscfg->WPA_auth;

	if (ie_len < WPA_IE_TAG_FIXED_LEN) {
		WL_ERROR(("wl%d: WPA IE illegally short\n", wlc->pub->unit));
		return 1;
	}
	ver = wpaie[6];
	if ((ver != WPA_VERSION) || !WPA_OUI_OK(wpaie+2)) {
		WL_ERROR(("wl%d: unexpected WPA IE version %d or OUI %02x:%02x:%02x\n",
			wlc->pub->unit, ver, wpaie[2], wpaie[3], wpaie[4]));
		return 1;
	}
	/* All the rest is optional.  Test the length to see what's there. */
	if (ie_len < 10) {
		/* Group suite (and all the rest) defaulted. */
		if (!(WPA_auth & WPA_AUTH_UNSPECIFIED) ||
		    (!wpa_cipher_enabled(wlc, bsscfg, WPA_CIPHER_TKIP))) {
			WL_ERROR(("wl%d: WPA IE defaults not enabled\n",
				wlc->pub->unit));
			return 1;
		}
		*auth = WPA_AUTH_UNSPECIFIED;
		*wsec = TKIP_ENABLED;
		return 0;
	}
	/* There's a group suite, so check it. */
	if (!WPA_OUI_OK(wpaie+8) || !wpa_mcast_cipher_allowed(wlc, bsscfg, wpaie[11])) {
		WL_ERROR(("wl%d: WPA group cipher %02x:%02x:%02x:%d not enabled\n",
			wlc->pub->unit, wpaie[8], wpaie[9], wpaie[10], wpaie[11]));
		return 1;
	}

	/* Update the potential crypto being used for the SCB */
	bcmwpa_cipher2wsec(&wpaie[8], wsec);

	/* Is enough IE left for a pairwise suite? */
	if (ie_len >= 16) {
		/* Check that *the* pairwise suite is enabled. */
		n_suites = wpaie[12];
		offset = 14;
		if (!WPA_OUI_OK(wpaie+offset) ||
		    !wpa_cipher_enabled(wlc, bsscfg, wpaie[offset+3]) || n_suites != 1) {
			WL_ERROR(("wl%d: bad pairwise suite in WPA IE.\n",
				wlc->pub->unit));
			return 1;
		}
		bcmwpa_cipher2wsec(&wpaie[offset], wsec);
	}
	/* Is enough IE left for a key management suite?
	 * (Remember IE length doesn't include first 2 bytes of the IE.)
	 */
	if (ie_len < 22) {
		/* It's defaulted.  Check that we default, too. */
		if (!(WPA_auth & WPA_AUTH_UNSPECIFIED)) {
			WL_ERROR(("wl%d: default WPA IE auth mode not enabled\n",
				wlc->pub->unit));
			return 1;
		}
		*auth = WPA_AUTH_UNSPECIFIED;
	} else {
		n_suites = wpaie[18];
		offset = 20;
		if ((n_suites != 1) ||
		    !(WPA_OUI_OK(wpaie+offset) &&
		      (((wpaie[offset+3] == RSN_AKM_UNSPECIFIED) &&
			(WPA_auth & WPA_AUTH_UNSPECIFIED)) ||
		       ((wpaie[offset+3] == RSN_AKM_PSK) &&
			(WPA_auth & WPA_AUTH_PSK))))) {
			WL_ERROR(("wl%d: bad key management suite in WPA IE.\n",
				wlc->pub->unit));
			return 1;
		}
		if (!bcmwpa_akm2WPAauth(&wpaie[offset], &WPA_auth, FALSE)) {
			WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
				wlc->pub->unit, wpaie[offset], wpaie[offset + 1],
				wpaie[offset + 2], wpaie[offset + 3]));
			return 1;
		}
		*auth = WPA_auth;
	}
	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return 0;
}

#undef WPA_OUI_OK

static uint16
wlc_check_wpa2ie(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bcm_tlv_t *wpa2ie, struct scb *scb)
{
	uint8 len = wpa2ie->len;
	uint32 WPA_auth = bsscfg->WPA_auth;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	wpa_pmkid_list_t *pmkid_list = NULL;
	uint16 count;
	uint16 pmkid_count;

	scb->flags2 &= ~(SCB2_MFP | SCB2_SHA256);

	/* Check min length and version */
	if ((len < WPA2_VERSION_LEN) ||
	    (ltoh16_ua(wpa2ie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA2 version %d\n", wlc->pub->unit,
			ltoh16_ua(wpa2ie->data)));
#ifdef BCMDBG
		WL_ERROR(("wl%d: RSN IE len %d\n", wlc->pub->unit, len));
#endif /* BCMDBG */
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= WPA2_VERSION_LEN;

	/* All the rest is optional.  Test the length to see what's there. */
	if (len < WPA_SUITE_LEN) {
		/* Group suite (and all the rest) defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(wlc, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: WPA2 IE defaults not enabled\n", wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return DOT11_SC_SUCCESS;
	}

	/* There's an mcast cipher, so check it. */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	if (bcmp(mcast->oui, WPA2_OUI, DOT11_OUI_LEN) ||
	    !wpa_mcast_cipher_allowed(wlc, bsscfg, mcast->type)) {
		WL_ERROR(("wl%d: WPA2 mcast cipher %02x:%02x:%02x:%d not enabled\n",
			wlc->pub->unit, mcast->oui[0], mcast->oui[1], mcast->oui[2],
			mcast->type));
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= WPA_SUITE_LEN;

	/* Update the potential crypto being used for the SCB */
	bcmwpa_cipher2wsec(mcast->oui, &scb->wsec);

	/* Again, the rest is optional.  Test the length to see what's there. */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* ucast cipher and AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(wlc, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: WPA2 IE defaults not enabled\n", wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return DOT11_SC_SUCCESS;
	}

	/* Check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	if (count != 1 ||
	    bcmp(ucast->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) ||
	    !wpa_cipher_enabled(wlc, bsscfg, ucast->list[0].type)) {
		WL_ERROR(("wl%d: bad pairwise suite in WPA2 IE.\n", wlc->pub->unit));
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	bcmwpa_cipher2wsec(ucast->list[0].oui, &scb->wsec);

	/* Again, the rest is optional, so test len to see what's there */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED)) {
			WL_ERROR(("wl%d: default WPA2 IE auth mode not enabled\n",
				wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		return DOT11_SC_SUCCESS;
	}

	/* Check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[1];
	count = ltoh16_ua(&mgmt->count);
	if ((count != 1) ||
	    !((bcmp(mgmt->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) == 0) &&
	      (((mgmt->list[0].type == RSN_AKM_UNSPECIFIED) &&
	        (WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
#ifdef WLFBT
		(BSSCFG_IS_FBT_1X(bsscfg) && (mgmt->list[0].type == RSN_AKM_FBT_1X) &&
		(WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
		(BSSCFG_IS_FBT_PSK(bsscfg) && (mgmt->list[0].type == RSN_AKM_FBT_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK)) ||
#endif /* WLFBT */
	       ((mgmt->list[0].type == RSN_AKM_SHA256_1X) &&
		(WPA_auth & WPA2_AUTH_1X_SHA256)) ||
	       ((mgmt->list[0].type == RSN_AKM_SHA256_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK_SHA256)) ||
#ifdef MFP
		(WLC_MFP_ENAB(wlc->pub) && BSSCFG_IS_MFP_REQUIRED(bsscfg) &&
		((mgmt->list[0].type == RSN_AKM_SHA256_PSK) ||
		(mgmt->list[0].type == RSN_AKM_SHA256_1X))) ||
#endif // endif
#ifdef WL_SAE
		((mgmt->list[0].type == RSN_AKM_SAE_PSK) &&
		(WPA_auth & WPA3_AUTH_SAE_PSK)) ||
#endif /* WL_SAE */
		((mgmt->list[0].type == RSN_AKM_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK))))) {
		WL_ERROR(("wl%d: bad AKM in WPA2 IE.\n", wlc->pub->unit));
		return DOT11_SC_ASSOC_FAIL;
	}
	if ((mgmt->list[0].type == RSN_AKM_SHA256_1X) ||
#ifdef WL_SAE
	/* If pure SAE, MFP should be required */
	((mgmt->list[0].type == RSN_AKM_SAE_PSK) && (WPA_auth == WPA3_AUTH_SAE_PSK)) ||
#endif /* WL_SAE */
	    (mgmt->list[0].type == RSN_AKM_SHA256_PSK)) {
		if ((BSSCFG_IS_MFP_REQUIRED(bsscfg)) ||
		    (WPA_auth & (WPA2_AUTH_PSK_SHA256 | WPA2_AUTH_1X_SHA256))) {
			scb->flags2 |= SCB2_SHA256;
		} else {
			WL_ERROR(("wl%d: bad AKM: AP has no SHA256 but STA does.\n",
				wlc->pub->unit));
			return DOT11_SC_ASSOC_MFP_VIOLATION;
		}
	}
	if ((BSSCFG_IS_MFP_REQUIRED(bsscfg)) && !SCB_SHA256(scb)) {
		WL_ERROR(("wl%d: MFP requred but peer selects SHA-1, reject\n",
		           wlc->pub->unit));
		return DOT11_SC_ASSOC_MFP_VIOLATION;
	}
	if (!bcmwpa_akm2WPAauth((uint8 *)&mgmt->list[0], &WPA_auth, FALSE)) {
		WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
			wlc->pub->unit, mgmt->list[0].oui[0], mgmt->list[0].oui[1],
			mgmt->list[0].oui[2], mgmt->list[0].type));
		return DOT11_SC_ASSOC_FAIL;
	}
	scb->WPA_auth = WPA_auth;

	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);

	/* In case RSN cap length is 1, it is not proper */
	if (len && len < RSN_CAP_LEN) {
		return DOT11_SC_INVALID_RSNIE_CAP;
	}

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub)) {
		bool scb_mfp;
		/* proceed with rsn_cap zero, if it is not present */
		uint16 rsn_cap = (len == 0) ? 0 : ltoh16_ua(&mgmt->list[count]);
		if (!wlc_mfp_check_rsn_caps(wlc->mfp, bsscfg, rsn_cap, &scb_mfp)) {
			WL_ERROR(("wl%d: invalid sta MFP setting rsn_cap: 0x%02x, wsec: 0x%02x\n",
				wlc->pub->unit, rsn_cap, bsscfg->wsec));
			return DOT11_SC_ASSOC_MFP_VIOLATION;
		}

#ifdef WL_SAE
		/* SAE capable STA must support MFP */
		if ((mgmt->list[0].type == RSN_AKM_SAE_PSK) && !(rsn_cap & RSN_CAP_MFPC)) {
			WL_ERROR(("wl%d: MFP is must for SAE capable STA, reject\n",
				wlc->pub->unit));
			return DOT11_SC_ASSOC_MFP_VIOLATION;
		}
#endif /* WL_SAE */

		if (scb_mfp) {
			scb->flags2 |= SCB2_MFP;
			WL_ERROR(("wl%d: turn sta MFP setting on with %s\n",
				wlc->pub->unit, (SCB_SHA256(scb) ? "sha256": "sha1")));
		}
	}
#endif /* MFP */
#ifdef WLWNM_AP
	/* disabled DMS service if SPP support conflict */
	if (WLWNM_ENAB(wlc->pub) && WNM_SLEEP_ENABLED(wlc_wnm_get_cap(wlc, bsscfg))) {
		/* proceed with rsn_cap zero, if it is not present */
		uint16 rsn_cap = (len == 0) ? 0 : ltoh16_ua(&mgmt->list[count]);

		if ((rsn_cap & RSN_CAP_SPPR) && !(rsn_cap & RSN_CAP_SPPC)) {
			wlc_wnm_dms_spp_conflict(wlc, scb);
		}
	}
#endif /* WLWNM_AP */

	if (len >= RSN_CAP_LEN) {
		len -= RSN_CAP_LEN;
	}

	if (len == 0) {
		/* Nothing to parse. Return success. */
		return DOT11_SC_SUCCESS;
	}

	if (len < WPA2_PMKID_COUNT_LEN) {
		return DOT11_SC_INVALID_PMKID;
	}

	/* PMKID */
	pmkid_list = (wpa_pmkid_list_t *)((uint8 *)&mgmt->list[count] + RSN_CAP_LEN);
	pmkid_count = ltoh16_ua(&pmkid_list->count);

	len -= WPA2_PMKID_COUNT_LEN;

	if (pmkid_count == 0 && len == 0) {
		/* Nothing to parse. Return success. */
		return DOT11_SC_SUCCESS;
	} else if (pmkid_count != 0 && (len < (pmkid_count * WPA2_PMKID_LEN))) {
		return DOT11_SC_INVALID_PMKID;
	}
#ifdef WL_SAE
	if (pmkid_count == 0) {
		scb->pmkid_included = 0;
	} else {
		scb->pmkid_included = 1;
	}
#endif /* WL_SAE */

	len -= (pmkid_count * WPA2_PMKID_LEN);

	if (len == 0) {
		/* Parse PMKID.  Nothing else to parse. Return success */
		return DOT11_SC_SUCCESS;
	}
#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub)) {
		wpa_suite_mcast_t *gmcs = NULL; /* Group Management Cipher Suite */

		if (len < WPA_SUITE_LEN) {
			return DOT11_SC_ASSOC_FAIL;
		}

		/* There is a Groupt Mgmt Cipher Suite.check it. */
		gmcs = (wpa_suite_mcast_t *)&pmkid_list->list[pmkid_count];

		if (bcmp(gmcs->oui, WPA2_OUI, DOT11_OUI_LEN) ||
			gmcs->type != WPA_CIPHER_BIP) {
			WL_ERROR(("wl%d: WPA2 Grp Mgmt Ciphr Suite %02x:%02x:%02x:%d not enabled\n",
				wlc->pub->unit, gmcs->oui[0], gmcs->oui[1], gmcs->oui[2],
				gmcs->type));
			return DOT11_SC_ASSOC_FAIL;
		}

		/* Successfully parsed GMCS. Decrement length */
		len -= WPA_SUITE_LEN;
	}
#endif /* MFP */

	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return DOT11_SC_SUCCESS;
}

#ifdef WLOSEN
static uint16
wlc_check_osenie(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bcm_tlv_t *osenie,
	struct scb *scb)
{
	uint8 len = osenie->len;
	uint32 WPA_auth = bsscfg->WPA_auth;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;
	scb->flags2 &= ~(SCB2_MFP | SCB2_SHA256);

	/* Check min length */
	if (len < WFA_OUI_LEN + 1) {
		WL_ERROR(("wl%d: invalid OSEN IE len %d\n", wlc->pub->unit, len));
#ifdef BCMDBG
		WL_ERROR(("wl%d: OSEN IE len %d\n", wlc->pub->unit, len));
#endif /* BCMDBG */
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= WFA_OUI_LEN + 1;

	/* check OSEN IE */
	if (!(bcmp(osenie->data, WFA_OUI, WFA_OUI_LEN) == 0 &&
		osenie->data[WFA_OUI_LEN] == WFA_OUI_TYPE_OSEN)) {
		return DOT11_SC_ASSOC_FAIL;
	}

	/* All the rest is optional.  Test the length to see what's there. */
	if (len < WPA_SUITE_LEN) {
		/* Group suite (and all the rest) defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(wlc, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: OSEN IE defaults not enabled\n", wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return DOT11_SC_SUCCESS;
	}

	/* There's an mcast cipher, so check it. */
	mcast = (wpa_suite_mcast_t *)&osenie->data[WFA_OUI_LEN + 1];
	if (bcmp(mcast->oui, WPA2_OUI, DOT11_OUI_LEN) ||
		(mcast->type != WPA_CIPHER_TPK &&
		!wpa_mcast_cipher_allowed(wlc, bsscfg, mcast->type))) {
		WL_ERROR(("wl%d: OSEN mcast cipher %02x:%02x:%02x:%d not enabled\n",
			wlc->pub->unit, mcast->oui[0], mcast->oui[1], mcast->oui[2],
			mcast->type));
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= WPA_SUITE_LEN;

	/* Update the potential crypto being used for the SCB */
	bcmwpa_cipher2wsec(mcast->oui, &scb->wsec);

	/* Again, the rest is optional.  Test the length to see what's there. */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* ucast cipher and AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(wlc, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: OSEN IE defaults not enabled\n", wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return DOT11_SC_SUCCESS;
	}

	/* Check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	if (count != 1 ||
		bcmp(ucast->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) ||
		!wpa_cipher_enabled(wlc, bsscfg, ucast->list[0].type)) {
		WL_ERROR(("wl%d: bad pairwise suite in OSEN IE.\n", wlc->pub->unit));
		return DOT11_SC_ASSOC_FAIL;
	}
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	bcmwpa_cipher2wsec(ucast->list[0].oui, &scb->wsec);

	/* Again, the rest is optional, so test len to see what's there */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED)) {
			WL_ERROR(("wl%d: default OSEN IE auth mode not enabled\n",
				wlc->pub->unit));
			return DOT11_SC_ASSOC_FAIL;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		return DOT11_SC_SUCCESS;
	}

	/* Check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[1];
	count = ltoh16_ua(&mgmt->count);
	if ((count != 1) ||
	    !((bcmp(mgmt->list[0].oui, WFA_OUI, WFA_OUI_LEN) == 0) &&
	    (((mgmt->list[0].type == OSEN_AKM_UNSPECIFIED) &&
		(WPA_auth & (WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_PSK))) ||
		((mgmt->list[0].type == RSN_AKM_SHA256_1X) &&
		(WPA_auth & WPA2_AUTH_1X_SHA256)) ||
		((mgmt->list[0].type == RSN_AKM_SHA256_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK_SHA256)) ||
#ifdef MFP
		(WLC_MFP_ENAB(wlc->pub) && BSSCFG_IS_MFP_REQUIRED(bsscfg) &&
		((mgmt->list[0].type == RSN_AKM_SHA256_PSK) ||
		(mgmt->list[0].type == RSN_AKM_SHA256_1X))) ||
#endif // endif
	    ((mgmt->list[0].type == RSN_AKM_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK))))) {
		WL_ERROR(("wl%d: bad AKM in OSEN IE.\n", wlc->pub->unit));
		return DOT11_SC_ASSOC_FAIL;
	}
	if ((mgmt->list[0].type == RSN_AKM_SHA256_1X) ||
		(mgmt->list[0].type == RSN_AKM_SHA256_PSK)) {
		if (BSSCFG_IS_MFP_REQUIRED(bsscfg) ||
		    (WPA_auth & (WPA2_AUTH_1X_SHA256 | WPA2_AUTH_PSK_SHA256)))
			scb->flags2 |= SCB2_SHA256;
		else {
			WL_ERROR(("wl%d: bad AKM: AP has no SHA256 but STA does.\n",
				wlc->pub->unit));
			return DOT11_SC_ASSOC_MFP_VIOLATION;
		}
	}
	if ((BSSCFG_IS_MFP_REQUIRED(bsscfg)) && !SCB_SHA256(scb)) {
		WL_ERROR(("wl%d: MFP requred but peer selects SHA-1, reject\n", wlc->pub->unit));
		return DOT11_SC_ASSOC_MFP_VIOLATION;
	}
	if (!bcmwpa_akm2WPAauth((uint8 *)&mgmt->list[0], &WPA_auth, FALSE)) {
		WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
			wlc->pub->unit, mgmt->list[0].oui[0], mgmt->list[0].oui[1],
			mgmt->list[0].oui[2], mgmt->list[0].type));
		return DOT11_SC_ASSOC_FAIL;
	}
	scb->WPA_auth = WPA_auth;

	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub)) {
		uint16 cap;
		bool scb_mfp;
		cap = (len < RSN_CAP_LEN) ? 0 : ltoh16_ua(&mgmt->list[count]);
		if (!wlc_mfp_check_rsn_caps(wlc->mfp, bsscfg, cap, &scb_mfp)) {
			WL_ERROR(("wl%d: invalid sta MFP setting cap: 0x%02x, wsec: 0x%02x\n",
				wlc->pub->unit, cap, bsscfg->wsec));
			return DOT11_SC_ASSOC_MFP_VIOLATION;
		}
		if (scb_mfp) {
			scb->flags2 |= SCB2_MFP;
			WL_ERROR(("wl%d: turn sta MFP setting on with %s\n",
				wlc->pub->unit, (SCB_SHA256(scb) ? "sha256": "sha1")));
		}
	}
#endif /* MFP */

	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return DOT11_SC_SUCCESS;
}
#endif	/* WLOSEN */

/* WPA */
static int
wlc_akm_arq_parse_wpa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (((cfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(cfg->wsec)) ||
		WSEC_SES_OW_ENABLED(cfg->wsec))) {
		uint8 *wpaie = data->ie;
		struct scb *scb;

		scb = ftpparm->assocreq.scb;
		ASSERT(scb != NULL);

		/* check WPA AKM */
		if (wpaie == NULL)
			return BCME_OK;

		if (wlc_check_wpaie(wlc, cfg, (uint8 *)wpaie, &scb->WPA_auth, &scb->wsec)) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			WL_ERROR(("wl%d: %s: unsupported request in WPA IE from %s\n",
			          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));
			ftpparm->assocreq.status = DOT11_SC_ASSOC_FAIL;
			return BCME_ERROR;
		}

		return wlc_scb_save_wpa_ie(wlc, scb, (bcm_tlv_t *)wpaie);
	}

	return BCME_OK;
}

#ifdef WLOSEN
static int
wlc_akm_arq_parse_osen_wpa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no WPA IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return BCME_OK;

	return wlc_akm_arq_parse_wpa_ie(ctx, data);
}

/* OSEN */
static int
wlc_akm_arq_parse_osen_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg)) {
		bcm_tlv_t *ie = (bcm_tlv_t *)data->ie;
		struct scb *scb;

		scb = ftpparm->assocreq.scb;
		ASSERT(scb != NULL);

		/* check OSEN AKM */
		if (ie == NULL)
			return BCME_OK;

		if ((ftpparm->assocreq.status =
		     wlc_check_osenie(wlc, cfg, ie, scb)) != DOT11_SC_SUCCESS) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			WL_ERROR(("wl%d: %s: unsupported request in OSEN IE from %s\n",
			          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}
#endif	/* WLOSEN */
#endif /* AP */

#ifdef STA
/* decode wpa IE to retrieve mcast/unicast ciphers and auth modes */
static int
wlc_parse_wpa_ie(wlc_info_t *wlc, wpa_ie_fixed_t *wpaie, wlc_bss_info_t *bi)
{
	int len = wpaie->length;	/* value length */
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;
	uint i, j;

	/*
	* The TLV tag is generic for all vendor-specific IEs. We need
	* to check OUI as well to make sure this TLV is indeed WPA,
	* and quietly discard the IE if it is not WPA IE.
	*/
	if ((len < WPA_IE_OUITYPE_LEN) ||
	    bcmp((uint8 *)wpaie->oui, WPA_OUI"\01", WPA_IE_OUITYPE_LEN))
		return 0;

	/*
	 * XXX - probably need more checking here...
	 *		 For now, assume that subsequent version subsumes
	 *		 WPA_VERSION since the spec sort of requires this...
	 */
	if ((len < WPA_IE_TAG_FIXED_LEN) ||
	    (ltoh16_ua(&wpaie->version) != WPA_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA version %d\n", wlc->pub->unit,
			ltoh16_ua(&wpaie->version)));
		return BCME_UNSUPPORTED;
	}
	len -= WPA_IE_TAG_FIXED_LEN;

	/* Default WPA parameters */
	bi->wpa.flags = RSN_FLAGS_SUPPORTED;
	bi->wpa.multicast = WPA_CIPHER_TKIP;
	bi->wpa.ucount = 1;
	bi->wpa.unicast[0] = WPA_CIPHER_TKIP;
	bi->wpa.acount = 1;
	bi->wpa.auth[0] = RSN_AKM_UNSPECIFIED;

	/* Check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wlc->pub->unit));
		/* it is ok to not have multicast cipher */
		return 0;
	}
	/* pick up multicast cipher if we know what it is */
	mcast = (wpa_suite_mcast_t *)&wpaie[1];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA_OUI, WPA_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type))
			bi->wpa.multicast = mcast->type;
		else
			WL_INFORM(("wl%d: unsupported WPA multicast cipher %d\n",
				wlc->pub->unit, mcast->type));
	} else
#ifdef BCMCCX
	if (!bcmp(mcast->oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN)) {
		if (CCX_ENAB(wlc->pub) && IS_CCX_CIPHER((CISCO_BASE + mcast->type)))
			bi->wpa.multicast = CISCO_BASE + mcast->type;
		else
			WL_INFORM(("wl%d: unsupported CCX multicast cipher %d\n",
				wlc->pub->unit, mcast->type));
	} else
#endif /* BCMCCX */
		WL_INFORM(("wl%d: unsupported proprietary multicast cipher OUI "
			   "%02X:%02X:%02X\n", wlc->pub->unit,
			   mcast->oui[0], mcast->oui[1], mcast->oui[2]));

	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc->pub->unit));
		/* it is ok to not have unicast cipher(s) */
		return 0;
	}
	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa.unicast) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type))
				bi->wpa.unicast[j++] = ucast->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA unicast cipher %d\n",
					wlc->pub->unit, ucast->list[i].type));
		} else
#ifdef BCMCCX
		if (!bcmp(ucast->list[i].oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN)) {
			if (CCX_ENAB(wlc->pub) && IS_CCX_CIPHER((CISCO_BASE + ucast->list[i].type)))
				bi->wpa.unicast[j++] = CISCO_BASE + ucast->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported CCX unicast cipher %d\n",
					wlc->pub->unit, ucast->list[i].type));
		} else
#endif /* BCMCCX */
			WL_INFORM(("wl%d: unsupported proprietary unicast cipher OUI "
				   "%02X:%02X:%02X\n", wlc->pub->unit,
				   ucast->list[i].oui[0], ucast->list[i].oui[1],
				   ucast->list[i].oui[2]));
	}
	bi->wpa.ucount = (uint8)j;

	/* jump to auth key mgmt suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc->pub->unit));
		/* it is ok to not have auth key mgmt suites */
		return 0;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa.auth) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_AKM(mgmt->list[i].type))
				bi->wpa.auth[j++] = mgmt->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA auth %d\n",
					wlc->pub->unit, mgmt->list[i].type));
		} else
#ifdef BCMCCX
		if (!bcmp(mgmt->list[i].oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN)) {
			/* CCKM uses RSN AKM None value (0) */
			if (CCX_ENAB(wlc->pub) && mgmt->list[i].type == RSN_AKM_NONE)
				bi->wpa.auth[j++] = RSN_AKM_NONE;
			else
				WL_INFORM(("wl%d: unsupported CCX auth %d\n",
					wlc->pub->unit, ucast->list[i].type));
		} else
#endif /* BCMCCX */
			WL_INFORM(("wl%d: unsupported proprietary auth OUI "
				   "%02X:%02X:%02X\n", wlc->pub->unit,
				   mgmt->list[i].oui[0], mgmt->list[i].oui[1],
				   mgmt->list[i].oui[2]));
	}
	bi->wpa.acount = (uint8)j;

	bi->flags |= WLC_BSS_WPA;

	/* XXX - Don't we need to parse RSN capabilities here to get the
	 * number of replay counters?
	 */

	return 0;
}

static int
wlc_akm_scan_parse_wpa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	if (data->ie == NULL)
		return BCME_OK;

	/* WPA parameters */
	wlc_parse_wpa_ie(wlc, (wpa_ie_fixed_t *)data->ie, bi);
	return BCME_OK;
}
#endif /* STA */

#ifdef AP
/* RSN */
static int
wlc_akm_arq_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (((cfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(cfg->wsec)) ||
		WSEC_SES_OW_ENABLED(cfg->wsec))) {
		bcm_tlv_t *wpaie = (bcm_tlv_t *)data->ie;
		struct scb *scb;

		scb = ftpparm->assocreq.scb;
		ASSERT(scb != NULL);

		/* check RSN AKM */
		if (wpaie == NULL)
			return BCME_OK;

		if ((ftpparm->assocreq.status =
		     wlc_check_wpa2ie(wlc, cfg, wpaie, scb)) != DOT11_SC_SUCCESS) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
			WL_ERROR(("wl%d: %s: unsupported request in WPA2 IE from %s\n",
			          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&scb->ea, eabuf)));
#ifdef BCMDBG
			WL_ERROR(("wl%d: %s: tlvs_len=%d\n",
			          wlc->pub->unit, __FUNCTION__, data->ie_len));
			prhex("All TLVs IE data", data->ie, data->ie_len);
#endif /* BCMDBG */
			return BCME_ERROR;
		}

		return wlc_scb_save_wpa_ie(wlc, scb, wpaie);
	}

	return BCME_OK;
}

#ifdef WLOSEN
static int
wlc_akm_arq_parse_osen_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akmi = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akmi->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	/* if OSEN enabled, no RSN IE */
	if (WLOSEN_ENAB(wlc->pub) && wlc_hs20_is_osen(wlc->hs20, cfg))
		return BCME_OK;

	return wlc_akm_arq_parse_rsn_ie(ctx, data);
}
#endif	/* WLOSEN */
#endif /* AP */

#ifdef STA
/* decode wpa2 IE to retrieve mcast/unicast ciphers and auth modes */
static int
wlc_parse_wpa2_ie(wlc_info_t *wlc, bcm_tlv_t *wpa2ie, wlc_bss_info_t *bi)
{
	int len = wpa2ie->len;		/* value length */
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	wpa_pmkid_list_t *pmkid_list = NULL;
	uint8 *cap;
	uint16 count;
	uint16 pmkid_count;
	uint i, j;

	/* Check min length and version */
	if ((len < WPA2_VERSION_LEN) ||
	    (ltoh16_ua(wpa2ie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA2 version %d\n", wlc->pub->unit,
			ltoh16_ua(wpa2ie->data)));
		return BCME_UNSUPPORTED;
	}
	len -= WPA2_VERSION_LEN;

	/* Default WPA2 parameters */
	bi->wpa2.flags = RSN_FLAGS_SUPPORTED;
	bi->wpa2.multicast = WPA_CIPHER_AES_CCM;
	bi->wpa2.ucount = 1;
	bi->wpa2.unicast[0] = WPA_CIPHER_AES_CCM;
	bi->wpa2.acount = 1;
	bi->wpa2.auth[0] = RSN_AKM_UNSPECIFIED;

	/* Check for multicast cipher suite */
	if (len == 0) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wlc->pub->unit));
		/* it is ok to not have multicast cipher */
		return DOT11_SC_SUCCESS;
	}

	if (len < WPA_SUITE_LEN) {
		bi->wpa2.multicast = WPA_CIPHER_NONE;
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: unsupported WPA2 multicast cipher %d\n", wlc->pub->unit,
			bi->wpa2.multicast));
		return BCME_UNSUPPORTED;
	}
	/* pick up multicast cipher if we know what it is */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA2_OUI, DOT11_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type))
			bi->wpa2.multicast = mcast->type;
		else
			WL_INFORM(("wl%d: unsupported WPA2 multicast cipher %d\n",
				wlc->pub->unit, mcast->type));
	} else
		WL_INFORM(("wl%d: unsupported proprietary multicast cipher OUI "
			   "%02X:%02X:%02X\n", wlc->pub->unit,
			   mcast->oui[0], mcast->oui[1], mcast->oui[2]));

	/* Check for unicast suite(s) */
	if (len == 0) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc->pub->unit));
		/* it is ok to not have unicast cipher(s) */
		return DOT11_SC_SUCCESS;
	}

	if (len < WPA_IE_SUITE_COUNT_LEN) {
		bi->wpa2.unicast[0] = WPA_CIPHER_NONE;
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: unsupported WPA2 unicast cipher %d\n", wlc->pub->unit,
			bi->wpa2.unicast[0]));
		return BCME_UNSUPPORTED;
	}

	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	if (len == 0) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc->pub->unit));
		/* it is ok to not have unicast cipher(s) */
		return DOT11_SC_SUCCESS;
	}

	if (len < (count*WPA_SUITE_LEN)) {
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: WPA2 number of unicast cipher suite present %d is less than the "
				"count %d\n", wlc->pub->unit, len/WPA_SUITE_LEN, count));
		return BCME_UNSUPPORTED;
	}

	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa2.unicast) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA2_OUI, DOT11_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type))
				bi->wpa2.unicast[j++] = ucast->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA2 unicast cipher %d\n",
					wlc->pub->unit, ucast->list[i].type));
		} else
			WL_INFORM(("wl%d: unsupported proprietary unicast cipher OUI "
				   "%02X:%02X:%02X\n", wlc->pub->unit,
				   ucast->list[i].oui[0], ucast->list[i].oui[1],
				   ucast->list[i].oui[2]));
	}
	bi->wpa2.ucount = (uint8)j;

	/* jump to auth key mgmt suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for auth key management suite(s) */
	if (len == 0) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc->pub->unit));
		/* it is ok to not have auth key mgmt suites */
		return DOT11_SC_SUCCESS;
	}
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		bi->wpa2.auth[0] = RSN_AKM_NONE;
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: unsupported WPA2 akm %d\n", wlc->pub->unit,
			bi->wpa2.auth[0]));
		return BCME_UNSUPPORTED;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	if (len == 0) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc->pub->unit));
		/* it is ok to not have auth key mgmt suites */
		return DOT11_SC_SUCCESS;
	}

	if (len < (count*WPA_SUITE_LEN)) {
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: WPA2 number of akm suite present %d is less than the count %d\n",
			wlc->pub->unit, len/WPA_SUITE_LEN, count));
		return BCME_UNSUPPORTED;
	}

	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa2.auth) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA2_OUI, DOT11_OUI_LEN)) {
			if (IS_RSN_AKM(mgmt->list[i].type))
				bi->wpa2.auth[j++] = mgmt->list[i].type;
			else if (WLFBT_ENAB(wlc->pub) && IS_FBT_AKM(mgmt->list[i].type)) {
				bi->wpa2.auth[j++] = mgmt->list[i].type;
				bi->wpa2.flags |= RSN_FLAGS_FBT;
			}
#ifdef MFP
			else if (WLC_MFP_ENAB(wlc->pub) && IS_MFP_AKM(mgmt->list[i].type)) {
				bi->wpa2.auth[j++] = mgmt->list[i].type;
				bi->wpa2.flags |= RSN_FLAGS_SHA256;
			}
#endif // endif
#ifdef WLTDLS
			else if (TDLS_SUPPORT(wlc->pub) && IS_TDLS_AKM(mgmt->list[i].type)) {
				/* TDLS uses TPK handshake */
				bi->wpa2.auth[j++] = mgmt->list[i].type;
			}
#endif /* WLTDLS */
			else
				WL_INFORM(("wl%d: unsupported WPA2 auth %d\n",
					wlc->pub->unit, mgmt->list[i].type));
		} else
#ifdef BCMCCX
		if (!bcmp(mgmt->list[i].oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN)) {
			/* CCKM uses RSN AKM None value (0) */
			if (CCX_ENAB(wlc->pub) && mgmt->list[i].type == RSN_AKM_NONE)
				bi->wpa2.auth[j++] = RSN_AKM_NONE;
			else
				WL_INFORM(("wl%d: unsupported CCX auth %d\n",
					wlc->pub->unit, ucast->list[i].type));
		} else
#endif /* BCMCCX */
#if defined(IBSS_PSK)
		if (!bcmp(mgmt->list[i].oui, BRCM_OUI, DOT11_OUI_LEN)) {
			switch (mgmt->list[i].type) {
#if defined(IBSS_PSK)
			case BRCM_AKM_PSK:
				bi->wpa2.auth[j++] = mgmt->list[i].type;
				break;
#endif /* IBSS_PSK */
			default:
				WL_INFORM(("wl%d: unsupported BRCM auth %d\n",
					wlc->pub->unit, mgmt->list[i].type));
			}
		} else
#endif /* defined(IBSS_PSK) */
			WL_INFORM(("wl%d: unsupported proprietary auth OUI "
				   "%02X:%02X:%02X\n", wlc->pub->unit,
				   mgmt->list[i].oui[0], mgmt->list[i].oui[1],
				   mgmt->list[i].oui[2]));
	}
	bi->wpa2.acount = (uint8)j;

	bi->flags |= WLC_BSS_WPA2;

	/* jump to RSN Cap */
	len -= (count - i) * WPA_SUITE_LEN;

	if (len == 0) {
		WL_INFORM(("wl%d: no rsn cap\n", wlc->pub->unit));
		/* it is ok to not have RSN Cap */
		return DOT11_SC_SUCCESS;
	}

	if (len < RSN_CAP_LEN) {
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: unsupported RSN Cap\n", wlc->pub->unit));
		return BCME_UNSUPPORTED;
	}

	/* parse RSN capabilities */
	cap = (uint8 *)&mgmt->list[count];
	if (cap[0] & WPA_CAP_WPA2_PREAUTH)
		bi->wpa2.flags |= RSN_FLAGS_PREAUTH;
	if (cap[1] & WPA_CAP_PEER_KEY_ENABLE)
		bi->wpa2.flags |= RSN_FLAGS_PEER_KEY_ENAB;

#ifdef MFP
	/* pick up MFP flags */
	if (WLC_MFP_ENAB(wlc->pub)) {
		bi->wpa2.flags |= wlc_mfp_rsn_caps_to_flags(wlc->mfp, cap[0]);
		WL_INFORM(("wl%d: %s: peer rsn caps 0x%02x 0x%02x, "
			"bss wpa2 flags 0x%02x\n", WLCWLUNIT(wlc), __FUNCTION__,
				cap[1], cap[0],  bi->wpa2.flags));
	}
#endif /* MFP */

	bi->wpa2.cap[0] = cap[0];

	len -= RSN_CAP_LEN;

	if (len == 0) {
		WL_INFORM(("wl%d: no PMKID\n", wlc->pub->unit));
		/* it is ok to not have PMKID */
		return DOT11_SC_SUCCESS;
	}

	if (len < WPA2_PMKID_COUNT_LEN) {
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		WL_ERROR(("wl%d: unsupported PMKID count\n", wlc->pub->unit));
		return BCME_UNSUPPORTED;
	}

	/* PMKID */
	pmkid_list = (wpa_pmkid_list_t *)((uint8 *)&mgmt->list[count] + RSN_CAP_LEN);
	pmkid_count = ltoh16_ua(&pmkid_list->count);

	len -= WPA2_PMKID_COUNT_LEN;

	if (pmkid_count == 0 && len == 0) {
		/* Nothing to parse. Return success. */
		return DOT11_SC_SUCCESS;
	} else if (pmkid_count != 0 && (len < (pmkid_count * WPA2_PMKID_LEN))) {
		/* Clear the RSN_FLAGS_SUPPORTED flag */
		bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
		return BCME_UNSUPPORTED;
	} else {
		WL_INFORM(("wl%d: set RSN_FLAGS_PMKID_COUNT_PRESENT.\n", wlc->pub->unit));
		bi->wpa2.flags |= RSN_FLAGS_PMKID_COUNT_PRESENT;
	}

	len -= (pmkid_count * WPA2_PMKID_LEN);

	if (len == 0) {
		/* PMKID parse done.  Nothing else to parse. Return success */
		return DOT11_SC_SUCCESS;
	}
#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub)) {
		wpa_suite_mcast_t *gmcs = NULL; /* Group Management Cipher Suite */

		if (len < WPA_SUITE_LEN) {
			/* Clear the RSN_FLAGS_SUPPORTED flag */
			bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
			return BCME_UNSUPPORTED;
		}

		/* There is a Groupt Mgmt Cipher Suite.check it. */
		gmcs = (wpa_suite_mcast_t *)&pmkid_list->list[pmkid_count];

		if (bcmp(gmcs->oui, WPA2_OUI, DOT11_OUI_LEN) ||
			gmcs->type != WPA_CIPHER_BIP) {
			WL_ERROR(("wl%d: WPA2 Grp Mgmt Ciphr Suite %02x:%02x:%02x:%d not enabled\n",
				wlc->pub->unit, gmcs->oui[0], gmcs->oui[1], gmcs->oui[2],
				gmcs->type));
			/* Clear the RSN_FLAGS_SUPPORTED flag */
			bi->wpa2.flags &= ~RSN_FLAGS_SUPPORTED;
			return BCME_UNSUPPORTED;
		}

		/* Successfully parsed GMCS. Decrement length */
		len -= WPA_SUITE_LEN;
	}
#endif /* MFP */

	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return DOT11_SC_SUCCESS;
}

static int
wlc_akm_scan_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_akm_info_t *akm = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akm->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;

	/* WPA2 parameters */
	if (data->ie == NULL)
		return BCME_OK;

	wlc_parse_wpa2_ie(wlc, (bcm_tlv_t *)data->ie, bi);
	return BCME_OK;
}
#endif /* STA */

#ifdef WLTDLS
static int
wlc_akm_tdls_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_akm_info_t *akm = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akm->wlc;
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;

	/* WPA2 parameters not present */
	if (parse->ie == NULL)
		return BCME_OK;

	wlc_parse_wpa2_ie(wlc, (bcm_tlv_t *)parse->ie, bi);
	return BCME_OK;
}
#endif /* WLTDLS */

/* module entries */
static int
wlc_akm_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_akm_info_t *akm = (wlc_akm_info_t *)ctx;
	wlc_info_t *wlc = akm->wlc;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *bsscfg;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_WPA_CAP): {
		uint32  cap = 0;
		/* wlc_wpa_cap() only sets a few bytes, so clear the
		 * return buffer before calling
		 */
		bzero(arg, len);
		err = wlc_wpa_cap(wlc, bsscfg, (uint8*)&cap, sizeof(cap));
		*ret_int_ptr = ltoh32(cap);
		break;
	}

#ifdef AP
	case IOV_SVAL(IOV_WPA_CAP): {
		uint8  cap[WPA_CAP_LEN];
		cap[0] = (uint8)(int_val & 0xff);
		cap[1] = (uint8)((int_val >> 8) & 0xff);
		err = wlc_wpa_set_cap(wlc, bsscfg, cap, sizeof(cap));
		break;
	}

	case IOV_GVAL(IOV_WPAIE): {
		struct ether_addr *ea;
		bcm_tlv_t *wpaie;
		uint8 *buf = (uint8 *)arg;
		int buf_len = len;
		struct scb *scb;

		if (!BSSCFG_AP(bsscfg)) {
			err = BCME_NOTAP;
			break;
		}

		ea = (struct ether_addr *)params;

		/* Is it the bsscfg bssid? If so, return wpaie and rsnie  */
		if (memcmp(ea, &bsscfg->BSSID, 6) == 0) {
			buf = wlc_write_wpa_ie_safe(wlc, bsscfg, buf, buf_len);
			buf_len = len - (int)(buf - (uint8 *)arg);
			buf = wlc_write_rsn_ie_safe(wlc, bsscfg, buf, buf_len);
			break;
		}

		/* looking for the assoc request wpaie of an associated STA  */
		if (!(scb = wlc_scbfind(wlc, bsscfg, ea))) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
			char ea_str[ETHER_ADDR_STR_LEN];
#endif // endif
			WL_ERROR(("could not find scb %s\n", bcm_ether_ntoa(ea, ea_str)));
			err = BCME_NOTFOUND;
			break;
		}

		if (scb->wpaie == NULL) {
			err = BCME_NOTFOUND;
			break;
		}

		if (len < (int)scb->wpaie_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		wpaie = (bcm_tlv_t *)scb->wpaie;
		bcopy(scb->wpaie, buf, wpaie->len + TLV_HDR_LEN);
		/* clear length of next IE to indicate there is no more  */
		wpaie = (bcm_tlv_t *)(buf + wpaie->len + TLV_HDR_LEN);
		wpaie->len = 0;

		break;
	}
#endif /* AP */
#ifdef STA
	case IOV_SVAL(IOV_WPAIE): {
		wlc_assoc_t *as = bsscfg->assoc;

		/* Mostly used by external supplicants to set the IE
		 * on the STA, so set the assoc ie for the STA
		 */
		if (BSSCFG_AP(bsscfg)) {
			err = BCME_NOTSTA;
			break;
		}

		if (as->ie) {
			MFREE(wlc->osh, as->ie, as->ie_len);
			as->ie = NULL;
			as->ie_len = 0;
		}

		/* Just want to cleanup the elts */
		if ((p_len == 0) || (((uint8*)params)[TLV_LEN_OFF] == 0))
			break;

		if (!(as->ie = MALLOC(wlc->osh, p_len))) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			err = BCME_NOMEM;
			break;
		}

		as->ie_len = p_len;
		bcopy((uint8*)params, as->ie, p_len);

		break;
	}
#endif /* STA */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_akm_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_akm_info_t *akm = (wlc_akm_info_t *)ctx;
	bss_akm_info_t *bai = BSS_AKM_INFO(akm, cfg);

	bai->wpa2_preauth = TRUE;

	return BCME_OK;
}

static void
wlc_akm_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
}

#ifdef BCMDBG
static void
wlc_akm_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_akm_info_t *akm = (wlc_akm_info_t *)ctx;
	bss_akm_info_t *bai = BSS_AKM_INFO(akm, cfg);

	bcm_bprintf(b, "\twpa2_preauth %d\n", bai->wpa2_preauth);
}
#else
#define wlc_akm_bsscfg_dump NULL
#endif // endif

static const char BCMATTACHDATA(rstr_akm)[] = "akm";
wlc_akm_info_t *
BCMATTACHFN(wlc_akm_attach)(wlc_info_t *wlc)
{
	wlc_akm_info_t *akm;
#ifdef AP
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif // endif
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
#ifdef STA
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
#endif // endif

	if ((akm = MALLOCZ(wlc->osh, sizeof(wlc_akm_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	akm->wlc = wlc;

	/* register IE mgmt callbacks */
	/* calc/build */
#ifdef AP
	/* bcn/prbrsp */
#ifdef WLOSEN
	/* RSN and WPA IEs not added if OSEN enabled */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_RSN_ID,
	      wlc_akm_bcn_calc_osen_rsn_ie_len, wlc_akm_bcn_write_osen_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, osen/rsn in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, bcnfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	      wlc_akm_bcn_calc_osen_wpa_ie_len, wlc_akm_bcn_write_osen_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, osen/wpa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, bcnfstbmp, WLC_IEM_VS_IE_PRIO_OSEN,
		wlc_akm_bcn_calc_osen_ie_len, wlc_akm_bcn_write_osen_ie, akm) != BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, osen in bcn\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#else
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_RSN_ID,
	      wlc_akm_bcn_calc_rsn_ie_len, wlc_akm_bcn_write_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rsn in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, bcnfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	      wlc_akm_bcn_calc_wpa_ie_len, wlc_akm_bcn_write_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, wpa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif	/* WLOSEN */
#endif /* AP */
#ifdef STA
	/* assocreq/reassocreq */
#ifdef WLOSEN
	/* RSN and WPA IEs not added if OSEN enabled */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_RSN_ID,
	      wlc_akm_arq_calc_osen_rsn_ie_len, wlc_akm_arq_write_osen_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, osen/rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	      wlc_akm_arq_calc_osen_wpa_ie_len, wlc_akm_arq_write_osen_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, osen/wpa in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_OSEN,
		wlc_akm_arq_calc_osen_ie_len, wlc_akm_arq_write_osen_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, osen in assocreq\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#else
	if (wlc_iem_add_build_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_RSN_ID,
	      wlc_akm_arq_calc_rsn_ie_len, wlc_akm_arq_write_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	      wlc_akm_arq_calc_wpa_ie_len, wlc_akm_arq_write_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, wpa in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif	/* WLOSEN */
#endif /* STA */
	/* parse */
#ifdef AP
	/* assocreq/reassocreq */
#ifdef WLOSEN
	/* RSN and WPA IEs not added if OSEN enabled */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_RSN_ID,
	                             wlc_akm_arq_parse_osen_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, osen/rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	                                wlc_akm_arq_parse_osen_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, osen/wpa in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_OSEN,
		wlc_akm_arq_parse_osen_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, osen in assocreq\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#else
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, arqfstbmp, DOT11_MNG_RSN_ID,
	                             wlc_akm_arq_parse_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	                                wlc_akm_arq_parse_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, wpa in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif	/* WLOSEN */
#endif /* AP */
#ifdef STA
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_RSN_ID,
	                             wlc_akm_scan_parse_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_WPA,
	                                wlc_akm_scan_parse_wpa_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, wpa in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef IBSS_PEER_DISCOVERY_EVENT
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_RSN_ID,
	                         wlc_akm_bcn_parse_rsn_ie, akm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, rsn in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
#endif /* STA */
#ifdef WLTDLS
	/* setupreq */
	if (TDLS_SUPPORT(wlc->pub)) {
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_RSN_ID,
			wlc_akm_tdls_parse_rsn_ie, akm) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, rsn in setupreq\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* setupresp */
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_RSN_ID,
			wlc_akm_tdls_parse_rsn_ie, akm) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, rsn in setupresp\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* setupconfirm */
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_RSN_ID,
			wlc_akm_tdls_parse_rsn_ie, akm) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, rsn in setupconfirm\n",
				wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif /* WLTDLS */

	/* reserve bsscfg cubby */
	if ((akm->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_akm_info_t),
	                wlc_akm_bsscfg_init, wlc_akm_bsscfg_deinit, wlc_akm_bsscfg_dump,
	                (void *)akm)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register wlc module */
	if (wlc_module_register(wlc->pub, akm_iovars, rstr_akm, akm, wlc_akm_doiovar,
	                        NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	return akm;

fail:
	wlc_akm_detach(akm);
	return NULL;
}

void
BCMATTACHFN(wlc_akm_detach)(wlc_akm_info_t *akm)
{
	wlc_info_t *wlc;

	if (akm == NULL)
		return;

	wlc = akm->wlc;

	wlc_module_unregister(wlc->pub, rstr_akm, akm);

	MFREE(wlc->osh, akm, sizeof(wlc_akm_info_t));
}
