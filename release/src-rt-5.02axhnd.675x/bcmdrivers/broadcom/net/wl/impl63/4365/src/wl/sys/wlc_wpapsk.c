/*
 * wlc_wpapsk.c -- idsup & idauth common routines.
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
 * $Id: wlc_wpapsk.c 782389 2019-12-18 06:56:56Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlanSwArchitectureIdsup]
 */

#if defined(BCMINTSUP) || defined(BCMAUTH_PSK)
/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#endif /* BCMINTSUP */

#if defined(BCMINTSUP) || defined(BCMAUTH_PSK)
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#ifdef	BCMSUP_PSK
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/sha1.h>
#endif /* BCMSUP_PSK */

#include <proto/802.11.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_keymgmt.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_led.h>
#include <wlc_rm.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include <wl_export.h>
#include <wlc_scb.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX) || defined(WLFBT) || defined(BCMAUTH_PSK)
#include <wlc_wpa.h>
#endif /* BCMSUP_PSK || BCMCCX || WLFBT */
#include <wlc_sup.h>
#else /* external supplicant */
#include <stdio.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <proto/eapol.h>
#include <proto/eap.h>
#include <bcmwpa.h>
#include <sup_dbg.h>
#include <bcmutils.h>
#include <string.h>
#include <bcmendian.h>
#include <bcmcrypto/prf.h>
#include <proto/eapol.h>
#include <bcm_osl.h>
#include "bcm_supenv.h"
#ifdef BCMEXTCCX
#include <wlc_ccx.h>
#include <proto/802.11_ccx.h>
#include <wlc_security.h>
#endif // endif
#include "wpaif.h"
#include "wlc_sup.h"
#include "wlc_wpa.h"

#endif /* BCMINTSUP || BCMAUTH_PSK */

#if defined(BCMCCX) || defined(BCMSUP_PSK) || defined(BCMAUTH_PSK)

/* Get an EAPOL packet and fill in some of the common fields */
void *
wlc_eapol_pktget(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *da,
	uint len)
{
	osl_t *osh = wlc->osh;
	void *p;
	eapol_header_t *eapol_hdr;

	if ((p = PKTGET(osh, len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: pktget error for len %d\n",
		          wlc->pub->unit, __FUNCTION__, len));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return (NULL);
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	/* fill in common header fields */
	eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
	bcopy(da, &eapol_hdr->eth.ether_dhost, ETHER_ADDR_LEN);
	bcopy(&bsscfg->cur_etheraddr, &eapol_hdr->eth.ether_shost, ETHER_ADDR_LEN);
	eapol_hdr->eth.ether_type = hton16(ETHER_TYPE_802_1X);
#ifdef BCMSUP_PSK
	if (bcmwpa_is_rsn_auth(bsscfg->WPA_auth)) {
		if (SUP_ENAB(wlc->pub))
			eapol_hdr->version = wlc_sup_geteaphdrver(wlc->idsup, bsscfg);
	} else
#endif /* defined(BCMSUP_PSK) */
	eapol_hdr->version = WPA_EAPOL_VERSION;
	return p;
}
#endif	/* defined(BCMCCX) || defined(BCMSUP_PSK) || defined(BCMAUTH_PSK) */

#if defined(BCMSUP_PSK) || defined(BCMAUTH_PSK)
void
wlc_wpa_senddeauth(wlc_bsscfg_t *bsscfg, char *da, int reason)
{
#if defined(BCMINTSUP) || defined(BCMAUTH_PSK)
	scb_val_t scb_val;

	bzero(&scb_val, sizeof(scb_val_t));
	bcopy(da, &scb_val.ea, ETHER_ADDR_LEN);
	scb_val.val = (uint32) reason;
	wlc_ioctl(bsscfg->wlc, WLC_SCB_DEAUTHENTICATE_FOR_REASON,
	          &scb_val, sizeof(scb_val_t), bsscfg->wlcif);
#else
	DEAUTHENTICATE(bsscfg, reason);
#endif /* BCMINTSUP */
}
#endif /* BCMSUP_PSK */

/* plumb the group key */
uint32
wlc_wpa_plumb_gtk(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *gtk, uint32 gtk_len,
	uint32 key_index, uint32 cipher, uint8 *rsc, bool primary_key)
{
	wl_wsec_key_t *key;
	uint32 ret_index;

	WL_WSEC(("wlc_wpa_plumb_gtk\n"));

	if (!(key = MALLOC(wlc->osh, sizeof(wl_wsec_key_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		return (uint32)(-1);
	}

	bzero(key, sizeof(wl_wsec_key_t));
	key->index = key_index;
	/* NB: wlc_insert_key() will re-infer key->algo from key_len */
	key->algo = cipher;
	key->len = MIN(gtk_len, sizeof(key->data));
	bcopy(gtk, key->data, key->len);

	if (primary_key)
		key->flags |= WL_PRIMARY_KEY;

#ifdef BCMCCX
	if (!bcmwpa_is_rsn_auth(bsscfg->WPA_auth)) {
		/* drop cipher CKIP and MMH quals and set key flags bits */
		switch (cipher) {
		case CRYPTO_ALGO_CKIP_MMH:
			key->flags |= (WL_CKIP_MMH | WL_CKIP_KP);
			break;
		case CRYPTO_ALGO_CKIP:
			key->flags |= WL_CKIP_KP;
			break;
		case CRYPTO_ALGO_WEP_MMH:
			key->flags |= WL_CKIP_MMH;
			break;
		default:
			break;
		}
	}
#endif /* BCMCCX */

	/* Extract the Key RSC in an Endian independent format */
	key->iv_initialized = 1;
	if (rsc != NULL) {
		/* Extract the Key RSC in an Endian independent format */
		key->rxiv.lo = (((rsc[1] << 8) & 0xFF00) |
			(rsc[0] & 0x00FF));
		key->rxiv.hi = (((rsc[5] << 24) & 0xFF000000) |
			((rsc[4] << 16) & 0x00FF0000) |
			((rsc[3] << 8) & 0x0000FF00) |
			((rsc[2]) & 0x000000FF));
	} else {
		wlc_key_t *wlc_key;
		wlc_key_info_t wlc_key_info;
		uint8 seq[DOT11_WPA_KEY_RSC_LEN];
		int seq_len = 0;

		wlc_key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &wlc_key_info);
		ASSERT(wlc_key != NULL);
		seq_len = wlc_key_get_seq(wlc_key, seq, sizeof(seq), wlc_key_info.key_id, TRUE);
		if (seq_len > (sizeof(key->rxiv.lo) + sizeof(key->rxiv.hi))) {
			key->rxiv.lo = ltoh16_ua(seq);
			key->rxiv.hi = ltoh32_ua(seq+sizeof(uint16));
		} else {
			key->iv_initialized = 0;
		}
	}

	WL_WSEC(("wl%d: wlc_wpa_plumb_gtk: Group Key is stored as Low :0x%x,"
	         " High: 0x%x\n", wlc->pub->unit, key->rxiv.lo, key->rxiv.hi));

#if defined(BCMINTSUP) || defined(BCMAUTH_PSK)
	wlc_ioctl(wlc, WLC_SET_KEY, key, sizeof(wl_wsec_key_t), bsscfg->wlcif);
	if (key->index != key_index) {
		WL_WSEC(("%s(): key_index changed from %d to %d\n",
			__FUNCTION__, key_index, key->index));
	}
#else
	(void)PLUMB_GTK(key, bsscfg);
#endif // endif
	ret_index = key->index;
	MFREE(wlc->osh, key, sizeof(wl_wsec_key_t));
	return ret_index;
}

#if defined(BCMSUP_PSK) || defined(BCMAUTH_PSK) || defined(WLFBT)
void
wlc_wpapsk_free(wlc_info_t *wlc, wpapsk_t *wpa)
{
	/* Toss IEs if there are any */
	if (wpa) {
		if (wpa->auth_wpaie != NULL)
			MFREE(wlc->osh, wpa->auth_wpaie, wpa->auth_wpaie_len);
		if (wpa->sup_wpaie != NULL)
			MFREE(wlc->osh, wpa->sup_wpaie, wpa->sup_wpaie_len);
		bzero(wpa, sizeof(wpapsk_t));
	}
}

bool
wlc_wpapsk_start(wlc_info_t *wlc, wpapsk_t *wpa, uint8 *sup_ies, uint sup_ies_len,
	uint8 *auth_ies, uint auth_ies_len)
{
	uchar *auth_wpaie, *sup_wpaie;
	wpa_suite_t *cipher_suite;
	bool wep_ok = FALSE;
	bool ret = TRUE;

	/* get STA's WPA IE */
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		sup_wpaie = (uchar *)bcm_parse_tlvs(sup_ies, sup_ies_len, DOT11_MNG_RSN_ID);
		if (sup_wpaie == NULL) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: STA RSN IE not found in association"
				" request\n", wlc->pub->unit));
			return FALSE;
		}
	} else {
		sup_wpaie = (uchar *)bcm_find_wpaie(sup_ies, sup_ies_len);
		if (sup_wpaie == NULL) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: STA WPA IE not found in sup_ies\n",
				wlc->pub->unit));
			return FALSE;
		}
	}

	/* get AP's WPA IE */
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		auth_wpaie = (uchar *)bcm_parse_tlvs(auth_ies, auth_ies_len, DOT11_MNG_RSN_ID);
		if (auth_wpaie == NULL) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: AP RSN IE not found in auth_ies\n",
			         wlc->pub->unit));
			return FALSE;
		}
	} else {
		auth_wpaie = (uchar *)bcm_find_wpaie(auth_ies, auth_ies_len);
		if (auth_wpaie == NULL) {
			WL_WSEC(("wl%d:  wlc_wpapsk_start: AP WPA IE not found in probe response\n",
				wlc->pub->unit));
			return FALSE;
		}
	}

	/* initialize with default ciphers */
	wpa->ucipher = CRYPTO_ALGO_TKIP;
	wpa->mcipher = CRYPTO_ALGO_TKIP;
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		wpa->ucipher = CRYPTO_ALGO_AES_CCM;
		wpa->mcipher = CRYPTO_ALGO_AES_CCM;
	}

	/* get ciphers from STA's WPA IE if it's long enough */
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		bcm_tlv_t *wpa2ie;
		wpa_suite_mcast_t *mcast;
		wpa_suite_ucast_t *ucast_suites;

		wpa2ie = (bcm_tlv_t *)sup_wpaie;
		mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
		ucast_suites = (wpa_suite_ucast_t *)&mcast[1];
		cipher_suite = ucast_suites->list;
		if (!wpa2_cipher(cipher_suite, &wpa->ucipher, wep_ok)) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: unexpected unicast cipher"
			         " %02x:%02x:%02x:%02x\n",
			         wlc->pub->unit, cipher_suite->oui[0],
			         cipher_suite->oui[1], cipher_suite->oui[2],
			         cipher_suite->type));
			return FALSE;
		}
	} else
	if (sup_wpaie[1] + TLV_HDR_LEN > WPA_IE_FIXED_LEN + WPA_SUITE_LEN) {
		wpa_suite_ucast_t *ucast_suites;
#if defined(BCMCCX) || defined(BCMEXTCCX)
		wep_ok = (wpa->WPA_auth == WPA_AUTH_CCKM);
#endif /* BCMCCX || BCMEXTCCX */

		/* room for ucast cipher, so use that */
		ucast_suites = (wpa_suite_ucast_t *)
			((uint8 *)sup_wpaie + WPA_IE_FIXED_LEN + WPA_SUITE_LEN);
		cipher_suite = ucast_suites->list;
		if (!wpa_cipher(cipher_suite, &wpa->ucipher, wep_ok)) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: unexpected unicast cipher"
			         " %02x:%02x:%02x:%02x\n",
			         wlc->pub->unit, cipher_suite->oui[0],
			         cipher_suite->oui[1], cipher_suite->oui[2],
			         cipher_suite->type));
			return FALSE;
		}
	}
	if (!wlc_wpa_set_ucipher(wpa, wpa->ucipher, wep_ok)) {
		return FALSE;
	}

	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		bcm_tlv_t *wpa2ie;

		wpa2ie = (bcm_tlv_t *)sup_wpaie;
		cipher_suite = (wpa_suite_t *)&wpa2ie->data[WPA2_VERSION_LEN];
		wep_ok = TRUE;
		if (!wpa2_cipher(cipher_suite, &wpa->mcipher, wep_ok)) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: unexpected multicast cipher"
			         " %02x:%02x:%02x:%02x\n",
			         wlc->pub->unit, cipher_suite->oui[0],
			         cipher_suite->oui[1], cipher_suite->oui[2],
			         cipher_suite->type));
			return FALSE;
		}
	} else
	if (sup_wpaie[1] + TLV_HDR_LEN >= WPA_IE_FIXED_LEN + WPA_SUITE_LEN) {
		/* room for a mcast cipher, so use that */
		cipher_suite = (wpa_suite_t *)(sup_wpaie + WPA_IE_FIXED_LEN);
#if defined(BCMCCX) || defined(BCMEXTCCX)
		wep_ok = (wep_ok || cipher_suite->type == WPA_CIPHER_WEP_40 ||
		          cipher_suite->type == WPA_CIPHER_WEP_104);
#else
		wep_ok = TRUE;
#endif /* BCMCCX || BCMEXTCCX */
		if (!wpa_cipher(cipher_suite, &wpa->mcipher, wep_ok)) {
			WL_WSEC(("wl%d: wlc_wpapsk_start: unexpected multicast cipher"
			         " %02x:%02x:%02x:%02x\n",
			         wlc->pub->unit, cipher_suite->oui[0],
			         cipher_suite->oui[1], cipher_suite->oui[2],
			         cipher_suite->type));
			return FALSE;
		}
	}

	/* Save copy of AP's WPA IE */
	wpa->auth_wpaie_len = (uint16) (auth_wpaie[1] + TLV_HDR_LEN);
	wpa->auth_wpaie = (uchar *) MALLOC(wlc->osh, wpa->auth_wpaie_len);
	if (!wpa->auth_wpaie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return FALSE;
	}
	bcopy(auth_wpaie, wpa->auth_wpaie, wpa->auth_wpaie_len);

	/* Save copy of STA's WPA IE */
	wpa->sup_wpaie_len = (uint16) (sup_wpaie[1] + TLV_HDR_LEN);
	wpa->sup_wpaie = (uchar *) MALLOC(wlc->osh, wpa->sup_wpaie_len);
	if (!wpa->sup_wpaie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return FALSE;
	}
	bcopy(sup_wpaie, wpa->sup_wpaie, wpa->sup_wpaie_len);

	return ret;
}

/* plumb the pairwise key */
void
wlc_wpa_plumb_tk(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *tk, uint32 tk_len,
	uint32 cipher, struct ether_addr *ea)
{
	wl_wsec_key_t *key;
#if defined(BCMINTSUP) || defined(BCMAUTH_PSK)
	int err;
#endif // endif

	if (!(key = MALLOC(wlc->osh, sizeof(wl_wsec_key_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
		return;
	}

	WL_WSEC(("wlc_wpa_plumb_tk\n"));

	bzero(key, sizeof(wl_wsec_key_t));
	key->len = tk_len;
	bcopy(tk, key->data, key->len);
	bcopy(ea, &key->ea, ETHER_ADDR_LEN);
	/* NB: wlc_insert_key() will re-infer key.algo from key_len */
	key->algo = cipher;
	key->flags = WL_PRIMARY_KEY;
#if defined(BCMCCX) || defined(BCMEXTCCX)
	/* fold cipher CKIP and MMH quals into key flags bits */
	switch (cipher) {
	case CRYPTO_ALGO_CKIP_MMH:
		key->flags |= (WL_CKIP_MMH | WL_CKIP_KP);
		break;
	case CRYPTO_ALGO_CKIP:
		key->flags |= WL_CKIP_KP;
		break;
	case CRYPTO_ALGO_WEP_MMH:
		key->flags |= WL_CKIP_MMH;
		break;
	default:
		break;
	}
#endif /* BCMCCX  || BCMEXTCCX */
#if !defined(BCMINTSUP) && !defined(WLFBT) && !defined(BCMAUTH_PSK)
	PLUMB_TK(key, bsscfg);
#else
	err = wlc_iovar_op(wlc, "wsec_key", NULL, 0, key, sizeof(wl_wsec_key_t),
	                   IOV_SET, bsscfg->wlcif);
	if (err) {
		WL_ERROR(("wl%d: ERROR %d calling wlc_iovar_op with iovar \"wsec_key\"\n",
		          wlc->pub->unit, err));
	}
#endif /* BCMINTSUP */
	MFREE(wlc->osh, key, sizeof(wl_wsec_key_t));
	return;
}

/* setup cipher info in supplicant stucture */
bool
wlc_wpa_set_ucipher(wpapsk_t *wpa, ushort ucipher, bool wep_ok)
{
	/* update sta supplicant info */
	switch (ucipher) {
	case CRYPTO_ALGO_AES_CCM:
		wpa->ptk_len = AES_PTK_LEN;
		wpa->tk_len = AES_TK_LEN;
		wpa->desc = WPA_KEY_DESC_V2;
		break;
	case CRYPTO_ALGO_TKIP:
		wpa->ptk_len = TKIP_PTK_LEN;
		wpa->tk_len = TKIP_TK_LEN;
		wpa->desc = WPA_KEY_DESC_V1;
		break;
#if defined(BCMCCX) || defined(BCMEXTCCX)
	case CRYPTO_ALGO_CKIP_MMH:
	case CRYPTO_ALGO_CKIP:
		if (wep_ok) {
			wpa->ptk_len = CKIP_PTK_LEN;
			wpa->tk_len = CKIP_TK_LEN;
			wpa->desc = WPA_KEY_DESC_V1;
			break;
		} else {
			WL_WSEC(("wlc_wpa_set_ucipher: illegal unicast cipher (%d)\n",
			         (int)ucipher));
			return FALSE;
		}
	case CRYPTO_ALGO_WEP_MMH:
	case CRYPTO_ALGO_WEP128:
		if (wep_ok) {
			wpa->ptk_len = WEP128_PTK_LEN;
			wpa->tk_len = WEP128_TK_LEN;
			wpa->desc = WPA_KEY_DESC_V1;
			break;
		} else {
			WL_WSEC(("wlc_wpa_set_ucipher: illegal unicast cipher (%d)\n",
			         (int)ucipher));
			return FALSE;
		}
	case CRYPTO_ALGO_WEP1:
		if (wep_ok) {
			wpa->ptk_len = WEP1_PTK_LEN;
			wpa->tk_len = WEP1_TK_LEN;
			wpa->desc = WPA_KEY_DESC_V1;
			break;
		} else {
			WL_WSEC(("wlc_wpa_set_ucipher: illegal unicast cipher (%d)\n",
			         (int)ucipher));
			return FALSE;
		}
#endif /* BCMCCX || BCMEXTCCX */
	default:
		WL_WSEC(("wlc_wpa_set_ucipher: unexpected unicast cipher (%d)\n",
			(int)ucipher));
		return FALSE;
	}
	return TRUE;
}
#endif /* defined(BCMSUP_PSK) || defined(BCMAUTH_PSK) */

#if defined(BCMINTSUP) || defined(WLFBT) || defined(BCMAUTH_PSK)
/* Make a PMK from a pre-shared key */
/* Return 0 when pmk calculation is done and 1 when pmk calculation is in progress.
 * Return -1 when any error happens.
 */
int
wlc_wpa_cobble_pmk(wpapsk_info_t *info, char *psk, size_t psk_len, uchar *ssid, uint ssid_len)
{
	uchar in_key[DOT11_MAX_KEY_SIZE*2];

	/* There must be a PSK of at least 8 characters and no more than 64
	 * characters.  If it's 64 characters, they must all be legible hex.
	 * (It could be 66 characters if the first are the hex radix.)
	 */
	if ((psk == NULL) || (psk_len < WPA_MIN_PSK_LEN)) {
		WL_WSEC(("wl%d: wlc_wpa_cobble_pmk: insufficient key material for PSK\n",
			info->wlc->pub->unit));
		return -1;
	}
	/* check for hex radix if long enough for one */
	if ((psk_len ==  WSEC_MAX_PSK_LEN + 2) && (psk[0] == '0') &&
	    ((psk[1] == 'x') || (psk[1] == 'X'))) {
		psk += 2;
		psk_len -= 2;
	}

	/* If it's the right size for a hex PSK, check that it is
	 * really all ASCII hex characters.
	 */
	if (psk_len == WSEC_MAX_PSK_LEN) {
		char hex[] = "XX";
		int i = 0;

		do {
			hex[0] = *psk++;
			hex[1] = *psk++;
			if (!bcm_isxdigit(hex[0]) || !bcm_isxdigit(hex[1])) {
				WL_WSEC(("wl%d: wlc_wpa_cobble_pmk: numeric PSK is not 256-bit hex"
					" number\n", info->wlc->pub->unit));
				return -1;
			}
			/* okay so far; make this piece a number */
			in_key[i] = (uint8) bcm_strtoul(hex, NULL, 16);
		} while (++i < DOT11_MAX_KEY_SIZE);
		bcopy(in_key, info->pmk, sizeof(info->pmk));

	} else if (psk_len < WSEC_MAX_PSK_LEN) {

		/* Make certain the PSK string is NULL-terminated */
		psk[psk_len] = '\0';

		/* It's something that needs hashing */

		if (init_passhash(&info->passhash_states, psk, (int)psk_len, ssid, ssid_len))
			return -1;

		/* remove timer left from previous run */
		wl_del_timer(info->wlc->wl, info->passhash_timer);

		/* start the timer */
		wl_add_timer(info->wlc->wl, info->passhash_timer, 0, 0);
		return 1;
	} else {
		WL_WSEC(("wl%d: wlc_wpa_cobble_pmk: illegal PSK length (%u)\n",
		         info->wlc->pub->unit, (uint)psk_len));
		return -1;
	}
	info->pmk_len = PMK_LEN;
	return 0;
}
#endif /* BCMINTSUP */
