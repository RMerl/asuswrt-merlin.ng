/*
 * Broadcom 802.11 gtk offload Driver
 *
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
 * $Id: wlc_gtkol.c 782389 2019-12-18 06:56:56Z $
 */

/**
 * @file
 * @brief
 * When Group Key Rotation Offload is enabled, ARM shall perform the necessary frame exchange to
 * establish a new group key for the wireless client. If a failure occurs during the re-keying
 * procedure, ARM shall generate a log message to capture the failure event and reason and wake up
 * the host.
 *
 * @brief
 * The host driver shall provide the necessary key material to the ARM so it can perform this
 * offload. Supported only in sleep/WOWL mode (sleep offloads).
 */

/**
 * @file
 * @brief
 * XXX Twiki: [GTKoffload]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <proto/802.11.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <proto/802.1d.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_export.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <bcmwpa.h>
#include <wlc_wpa.h>
#include <wlc_hw_priv.h>
#include <proto/802.3.h>
#include <proto/eapol.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmarp.h>
#include <bcmcrypto/rc4.h>
#include <bcmcrypto/tkmic.h>
#include <bcmcrypto/prf.h>
#include <wlc_keymgmt.h>
#include <bcmwpa.h>
#include <bcm_ol_msg.h>
#include <wl_export.h>
#include <wlc_gtkol.h>
#include <wlc_dngl_ol.h>
#include <wlc_wowlol.h>

struct wlc_dngl_ol_gtk_info {
	wlc_dngl_ol_info_t		*wlc_dngl_ol;
	wpapsk_t			wpa;
	bool				gtk_ol_enabled;
#ifdef MFP
	bool				igtk_ol_enabled;
#endif // endif
};

#define CHECK_EAPOL(body) (body->type == EAPOL_WPA_KEY || body->type == EAPOL_WPA2_KEY)
#define SCB_LEGACY_AES		0x0400		/* legacy AES device */

#ifdef MFP
static bool
wlc_dngl_ol_extract_igtk(wlc_dngl_ol_gtk_info_t *gtk, const eapol_header_t* eapol);
#endif /* MFP */

wlc_dngl_ol_gtk_info_t *
wlc_dngl_ol_gtk_attach(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_gtk_info_t *ol_gtk;
	ol_gtk = (wlc_dngl_ol_gtk_info_t *)MALLOC(wlc_dngl_ol->osh, sizeof(wlc_dngl_ol_gtk_info_t));
	if (!ol_gtk) {
		WL_ERROR((" ol_gtk malloc failed: %s\n", __FUNCTION__));
		return NULL;
	}
	bzero(ol_gtk, sizeof(wlc_dngl_ol_gtk_info_t));
	ol_gtk->wlc_dngl_ol = wlc_dngl_ol;
	return ol_gtk;
}

void wlc_dngl_ol_gtk_send_proc(wlc_dngl_ol_gtk_info_t *ol_gtk,
	void *buf, int len)
{
	uchar *pktdata;
	olmsg_header *msg_hdr;
	olmsg_gtk_enable *gtk_enable;
	wlc_dngl_ol_info_t *wlc_dngl_ol = ol_gtk->wlc_dngl_ol;

	pktdata = (uint8 *) buf;
	msg_hdr = (olmsg_header *) pktdata;
	wpapsk_t *wpa = (wpapsk_t *)&ol_gtk->wpa;
	rsn_rekey_params *rekey;

	switch (msg_hdr->type) {
		case BCM_OL_GTK_ENABLE:
			WL_ERROR(("BCM_OL_GTK_ENABLE\n"));
			gtk_enable = (olmsg_gtk_enable *)pktdata;
			rekey = (rsn_rekey_params *)&gtk_enable->rekey;
			ol_gtk->gtk_ol_enabled = TRUE;
#ifdef MFP
			if (gtk_enable->igtk_enabled)
				ol_gtk->igtk_ol_enabled = TRUE;
#endif // endif
			bcopy(rekey->kck, wpa->eapol_mic_key, WPA_MIC_KEY_LEN);
			bcopy(rekey->kek, wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
			bcopy(rekey->replay_counter, wpa->last_replay,
				EAPOL_KEY_REPLAY_LEN);
			RXOEUPDREPLAYCNT(wlc_dngl_ol, rekey->replay_counter);
			wpa->WPA_auth = gtk_enable->sec_info.WPA_auth;
			wpa->mcipher = gtk_enable->gtk_algo;

			wlc_dngl_ol_sec_info_from_host(wlc_dngl_ol, &gtk_enable->cur_etheraddr,
				&gtk_enable->BSSID, &gtk_enable->sec_info);
			break;
		default:
			WL_ERROR(("%s: INVALID message type:%d\n", __FILE__, msg_hdr->type));
			break;
	}
}

static void
wlc_dngl_ol_plumb_gtk(wlc_dngl_ol_gtk_info_t *ol_gtk, uint8 *gtk, uint32 gtk_len,
	wlc_key_id_t key_id, uint32 cipher, uint8 *rsc, bool primary_key)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol;
	wlc_info_t *wlc;
	wlc_keymgmt_t *km;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	int retval;

	wlc_dngl_ol = ol_gtk->wlc_dngl_ol;
	wlc = wlc_dngl_ol->wlc;
	km = wlc->keymgmt;

	key = wlc_keymgmt_get_bss_key(km, &wlc_dngl_ol->bsscfg[0], key_id, &key_info);
	if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
		retval = BCME_BADKEYIDX;
		goto done;
	}

	retval = wlc_key_set_data(key, cipher, gtk, gtk_len);
	if (retval != BCME_OK)
		goto done;

	ASSERT(WOWL_TSCPN_SIZE <= EAPOL_WPA_KEY_REPLAY_LEN);

	retval = wlc_key_set_seq(key, rsc, WOWL_TSCPN_SIZE, WLC_KEY_SEQ_ID_ALL, FALSE);
	if (retval != BCME_OK)
		goto done;

	if (primary_key) {
		retval = wlc_keymgmt_set_bss_tx_key_id(km, &wlc_dngl_ol->bsscfg[0],
			key_info.key_id, FALSE);
		if (retval != BCME_OK)
			goto done;
	}

	/* update h/w now. h/w upd is disabled for offloads and selectively enabled */
	key_info.flags &= ~WLC_KEY_FLAG_NO_HW_UPDATE;
	retval = wlc_key_set_flags(key, key_info.flags);
	if (retval != BCME_OK)
		goto done;

	key_info.flags |= WLC_KEY_FLAG_NO_HW_UPDATE;
	retval = wlc_key_set_flags(key, key_info.flags);
	if (retval != BCME_OK)
		goto done;

	/* mark this key to be updated to host */
	RXOEUPDKEYROT(wlc_dngl_ol, 1 << key_id);

done:
	if (retval != BCME_OK)
		WL_ERROR(("wl%d: %s: error %d, cipher %d, key len %d, key_id %d\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval, cipher, (int)gtk_len, key_id));
}

/* Get an EAPOL packet and fill in some of the common fields */
static void *
wlc_dngl_ol_eapol_pktget(wlc_dngl_ol_info_t *wlc_dngl_ol, uint len)
{
	osl_t *osh = wlc_dngl_ol->osh;
	void *p;
	eapol_header_t *eapol_hdr;

	if ((p = PKTGET(osh, len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("%s: pktget error for len %d\n",
		          __FUNCTION__, len));
		return (NULL);
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	/* fill in common header fields */
	eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
	bcopy((char *)&RXOETXINFO(wlc_dngl_ol)->BSSID, (char *)&eapol_hdr->eth.ether_dhost,
	      ETHER_ADDR_LEN);
	bcopy((char *)&RXOETXINFO(wlc_dngl_ol)->cur_etheraddr,
		(char *)&eapol_hdr->eth.ether_shost, ETHER_ADDR_LEN);
	eapol_hdr->eth.ether_type = hton16(ETHER_TYPE_802_1X);
	eapol_hdr->eth.ether_type = hton16(ETHER_TYPE_802_1X);
	/* fill the right version */
	if (bcmwpa_is_rsn_auth(wlc_dngl_ol->ol_gtk->wpa.WPA_auth))
		eapol_hdr->version = WPA2_EAPOL_VERSION;
	else
		eapol_hdr->version = WPA_EAPOL_VERSION;
	return p;
}

static void *
wlc_dngl_ol_prepeapol(wlc_dngl_ol_info_t *wlc_dngl_ol, uint16 flags, wpa_msg_t msg)
{
	uint16 len, key_desc;
	void *p = NULL;
	eapol_header_t *eapol_hdr = NULL;
	eapol_wpa_key_header_t *wpa_key = NULL;
	uchar mic[PRF_OUTBUF_LEN];
	osl_t *osh = wlc_dngl_ol->osh;
	wpapsk_t *wpa = (wpapsk_t *)&wlc_dngl_ol->ol_gtk->wpa;

BCM_REFERENCE(osh);
	len = EAPOL_HEADER_LEN + EAPOL_WPA_KEY_LEN;
	switch (msg) {
		case GMSG2:	       /* group msg 2 */
			WL_ERROR(("%s\n", __FUNCTION__));
		if ((p = wlc_dngl_ol_eapol_pktget(wlc_dngl_ol, len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero(wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | GMSG2_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->gtk_len, (uint8 *)&wpa_key->key_len);
		break;
		default:
		WL_ERROR(("%s : unexpected message type %d\n",
		         __FUNCTION__, msg));
		break;
	}

	if (p != NULL) {
		/* do common message fields here; make and copy MIC last. */
		eapol_hdr->type = EAPOL_KEY;
		if (bcmwpa_is_rsn_auth(wpa->WPA_auth))
			wpa_key->type = EAPOL_WPA2_KEY;
		else
			wpa_key->type = EAPOL_WPA_KEY;
		bcopy(wpa->replay, wpa_key->replay, EAPOL_KEY_REPLAY_LEN);
		/* If my counter is one greater than the last one of his I
		 * used, then a ">=" test on receipt works AND the problem
		 * of zero at the beginning goes away.  Right?
		 */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		key_desc = flags & (WPA_KEY_DESC_V1 |  WPA_KEY_DESC_V2);
		if (!wpa_make_mic(eapol_hdr, key_desc, wpa->eapol_mic_key,
			mic)) {
			WL_ERROR(("%s: wlc_wpa_sup_sendeapol: MIC generation failed\n",
			         __FUNCTION__));
			return FALSE;
		}
		bcopy(mic, wpa_key->mic, EAPOL_WPA_KEY_MIC_LEN);
	}
		return p;
}

static bool
wlc_dngl_ol_sendeapol(wlc_dngl_ol_gtk_info_t *ol_gtk, uint16 flags, wpa_msg_t msg)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol = ol_gtk->wlc_dngl_ol;

	wlc_info_t *wlc = wlc_dngl_ol->wlc;
	void * p;
	WL_ERROR(("%s\n", __FUNCTION__));
	p = wlc_dngl_ol_prepeapol(wlc_dngl_ol, flags, msg);

	if (p != NULL) {
		wlc_sendpkt(wlc, p, NULL);
		return TRUE;
	}
	return FALSE;
}

static bool wlc_dngl_ol_process_eapol(wlc_dngl_ol_gtk_info_t *ol_gtk,
	eapol_header_t *eapol_hdr, bool encrypted)
{
	uint16 key_info, key_len, data_len;
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol_hdr->body;
	key_info = ntoh16_ua(&body->key_info);
	wpapsk_t *wpa = (wpapsk_t *)&ol_gtk->wpa;
	wlc_dngl_ol_info_t *wlc_dngl_ol = ol_gtk->wlc_dngl_ol;
	uint32 wowl_flags = wlc_dngl_ol->wowl_cfg.wowl_flags;
	ENTER();

	/* Handle wake up host on M1 */
	if ((key_info & WPA_KEY_PAIRWISE) && (key_info & WPA_KEY_ACK) &&
		!(key_info & WPA_KEY_MIC))
	{
		WL_WOWL(("%s: M1 rcvd. wake up host if WL_WOWL_M1 is enabled\n", __FUNCTION__));
		wlc_wowl_ol_wake_host(
				wlc_dngl_ol->wowl_ol,
				NULL, 0,
				NULL, 0, WL_WOWL_M1);
		return TRUE;
	}

	/* check for replay */
	if (wpa_array_cmp(MAX_ARRAY, body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN) ==
		wpa->replay) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		uchar *g = body->replay, *s = wpa->replay;
	    WL_ERROR(("%s: wlc_wpa_sup_eapol: ignoring replay "
	          "(got %02x%02x%02x%02x%02x%02x%02x%02x"
	          " last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n", __FUNCTION__,
	          g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
	          s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
#endif /* BCMDBG || BCMDBG_ERR */
		return TRUE;
	}

	if ((key_info & GMSG1_REQUIRED) != GMSG1_REQUIRED) {
		WL_ERROR(("wlc_wpa_sup_eapol: unexpected key_info (0x%04x)in"
			 "WPA group key message\n",
			 (uint)key_info));
		return TRUE;
	}

	/* If GTK not offloaded, then consider receiving a GTK message as GTK
	 * failure
	 */
	if (!ol_gtk->gtk_ol_enabled) {
		if (wowl_flags & WL_WOWL_GTK_FAILURE) {
			return FALSE;
		} else {
			WL_WOWL(("Returning as both WL_WOWL_KEYROT and"
						"WL_WOWLGTK_FAILURE are not set \n"));
			return TRUE;
		}
	}

	/* check message MIC */
	if ((key_info & WPA_KEY_MIC) &&
	    !wpa_check_mic(eapol_hdr, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
	                   wpa->eapol_mic_key)) {
		/* 802.11-2007 clause 8.5.3.3 - silently discard MIC failure */
		WL_ERROR(("%s : MIC failure, discarding pkt\n",
		         __FUNCTION__));
		return FALSE;
	}

	bcopy(body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN);
	bcopy(body->replay, wpa->last_replay, EAPOL_KEY_REPLAY_LEN);
	RXOEUPDREPLAYCNT(wlc_dngl_ol, body->replay);
	/* decrypt key data field */
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth) &&
	    (key_info & WPA_KEY_ENCRYPTED_DATA)) {

		uint8 *data, *encrkey;
		rc4_ks_t *rc4key;
		bool decr_status;
		WL_ERROR(("WPA2 WPA ENCRYPTED\n"));
		if (!(data = MALLOC(wlc_dngl_ol->osh, WPA_KEY_DATA_LEN_256))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			/* for now assert */
			ASSERT(FALSE);
			return FALSE;
		}
		if (!(encrkey = MALLOC(wlc_dngl_ol->osh, WPA_MIC_KEY_LEN*2))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
			/* for now assert */
			ASSERT(FALSE);
			return FALSE;
		}
		if (!(rc4key = MALLOC(wlc_dngl_ol->osh, sizeof(rc4_ks_t)))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(wlc_dngl_ol->osh, encrkey, WPA_MIC_KEY_LEN*2);
			/* for now assert */
			ASSERT(FALSE);
			return FALSE;
		}

		decr_status = wpa_decr_key_data(body, key_info,
		                       wpa->eapol_encr_key, NULL, data, encrkey, rc4key);

		MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
		MFREE(wlc_dngl_ol->osh, encrkey, WPA_MIC_KEY_LEN*2);
		MFREE(wlc_dngl_ol->osh, rc4key, sizeof(rc4_ks_t));

		if (!decr_status) {
			WL_ERROR(("%s: decryption of key"
					"data failed\n", __FUNCTION__));
			return FALSE;
		}
	}

	key_len = ntoh16_ua(&body->key_len);
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
		eapol_wpa2_encap_data_t *data_encap;
		eapol_wpa2_key_gtk_encap_t *gtk_kde;
		WL_ERROR(("2. wpa_auth: %d\n", wpa->WPA_auth));

		/* extract GTK */
		data_len = ntoh16_ua(&body->data_len);
		data_encap = wpa_find_gtk_encap(body->data, data_len);
		if (!data_encap) {
			WL_ERROR(("%s: encapsulated GTK missing from"
				" group message 1\n", __FUNCTION__));
			return FALSE;
		}
		wpa->gtk_len = data_encap->length - ((EAPOL_WPA2_ENCAP_DATA_HDR_LEN -
		                                          TLV_HDR_LEN) +
		                                         EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN);
		gtk_kde = (eapol_wpa2_key_gtk_encap_t *)data_encap->data;
		wpa->gtk_index = (gtk_kde->flags & WPA2_GTK_INDEX_MASK) >>
		    WPA2_GTK_INDEX_SHIFT;
		bcopy(gtk_kde->gtk, wpa->gtk, wpa->gtk_len);

		/* plumb GTK */
		wlc_dngl_ol_plumb_gtk(ol_gtk, wpa->gtk, wpa->gtk_len,
			(wlc_key_id_t)wpa->gtk_index, wpa->mcipher, body->rsc,
			gtk_kde->flags & WPA2_GTK_TRANSMIT);

	}
	 else {
		uint8 *data, *encrkey;
		rc4_ks_t *rc4key;
		bool decr_status;
		WL_ERROR(("3. wpa_auth: %d\n", wpa->WPA_auth));

		if (!(data = MALLOC(wlc_dngl_ol->osh, WPA_KEY_DATA_LEN_256))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			ASSERT(FALSE);
			return FALSE;
		}
		if (!(encrkey = MALLOC(wlc_dngl_ol->osh, WPA_MIC_KEY_LEN*2))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
			ASSERT(FALSE);
			return FALSE;
		}
		if (!(rc4key = MALLOC(wlc_dngl_ol->osh, sizeof(rc4_ks_t)))) {
			WL_ERROR(("%s: out of memory, malloced %d bytes\n",
				__FUNCTION__,  MALLOCED(wlc_dngl_ol->osh)));
			MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(wlc_dngl_ol->osh, encrkey, WPA_MIC_KEY_LEN*2);
			ASSERT(FALSE);
			return FALSE;
		}

		decr_status = wpa_decr_gtk(body, key_info, wpa->eapol_encr_key,
		                  wpa->gtk, data, encrkey, rc4key);

		MFREE(wlc_dngl_ol->osh, data, WPA_KEY_DATA_LEN_256);
		MFREE(wlc_dngl_ol->osh, encrkey, WPA_MIC_KEY_LEN*2);
		MFREE(wlc_dngl_ol->osh, rc4key, sizeof(rc4_ks_t));

		wpa->gtk_len = key_len;
		if (!decr_status) {
			WL_ERROR(("%s : GTK decrypt failure\n",
			         __FUNCTION__));
			return FALSE;
		}

		/* plumb GTK */
		wlc_dngl_ol_plumb_gtk(ol_gtk, wpa->gtk, wpa->gtk_len,
			(wlc_key_id_t)((key_info & WPA_KEY_INDEX_MASK) >> WPA_KEY_INDEX_SHIFT),
			wpa->mcipher, body->rsc, key_info & WPA_KEY_INSTALL);
	}

#ifdef MFP
	if (ol_gtk->igtk_ol_enabled)
		wlc_dngl_ol_extract_igtk(ol_gtk, eapol_hdr);
#endif // endif

	/* send group message 2 */
	if (wlc_dngl_ol_sendeapol(ol_gtk, (key_info & GMSG2_MATCH_FLAGS), GMSG2)) {
		wpa->state = WPA_SUP_KEYUPDATE;
		WL_ERROR(("%s : key update complete\n", __FUNCTION__));
		return TRUE;
	} else {
		WL_ERROR(("%s : send grp msg 2 failed\n",
		         __FUNCTION__));
		return FALSE;
	}
}

bool
wlc_dngl_ol_eapol(wlc_dngl_ol_gtk_info_t *ol_gtk,
	eapol_header_t *eapol_hdr, bool encrypted)
{
	bool status = FALSE;
	uint32 wowl_flags = ol_gtk->wlc_dngl_ol->wowl_cfg.wowl_flags;

	if (!ol_gtk->gtk_ol_enabled && !(wowl_flags & (WL_WOWL_GTK_FAILURE | WL_WOWL_M1)))
		return TRUE;
	if (eapol_hdr->type == EAPOL_KEY) {
		eapol_wpa_key_header_t *body;
		body = (eapol_wpa_key_header_t *)eapol_hdr->body;

		if (CHECK_EAPOL(body)) {
			status = wlc_dngl_ol_process_eapol(ol_gtk, eapol_hdr, encrypted);
			if (status == FALSE) {
				/* wake up host if wake on gtk fail is enabled */
				wlc_wowl_ol_wake_host(
					ol_gtk->wlc_dngl_ol->wowl_ol,
					NULL, 0,
					NULL, 0, WL_WOWL_GTK_FAILURE);
			}
		}
	}

	return status;
}

#ifdef MFP
static bool
wlc_dngl_ol_extract_igtk(wlc_dngl_ol_gtk_info_t *gtk, const eapol_header_t* eapol)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uint16 data_len = ntoh16_ua(&body->data_len);
	eapol_wpa2_encap_data_t *data_encap;
	eapol_wpa2_key_igtk_encap_t *igtk_kde;
	wlc_dngl_ol_info_t *wlc_dngl_ol = gtk->wlc_dngl_ol;
	uint16 key_len;
	wlc_info_t *wlc = wlc_dngl_ol->wlc;
	wlc_keymgmt_t *km = wlc->keymgmt;
	wlc_key_info_t key_info;
	int ret;
	wlc_key_t *key;

	data_encap = wpa_find_kde(body->data, data_len, WPA2_KEY_DATA_SUBTYPE_IGTK);
	if (!data_encap) {
		WL_ERROR(("%s: IGTK KDE not found in EAPOL\n", __FUNCTION__));
		return FALSE;
	}

	key_len = data_encap->length - ((EAPOL_WPA2_ENCAP_DATA_HDR_LEN - TLV_HDR_LEN) +
		EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN);

	if (key_len < AES_TK_LEN) {
		WL_ERROR(("%s: IGTK length is not %d\n", __FUNCTION__, AES_TK_LEN));
		return FALSE;
	}

	igtk_kde = (eapol_wpa2_key_igtk_encap_t *)data_encap->data;
	if (!WLC_KEY_ID_IS_IGTK(igtk_kde->key_id)) {
		return FALSE;
	}

	key_len = AES_TK_LEN;

	WL_WSEC(("Regular IGTK update id:%d\n", wlc_dngl_ol->txinfo.igtk.id));

	key = wlc_keymgmt_get_bss_key(km, &wlc_dngl_ol->bsscfg[0], igtk_kde->key_id, &key_info);
	ret = wlc_key_set_seq(key, igtk_kde->ipn, WOWL_TSCPN_SIZE, 0, FALSE);
	if (ret != BCME_OK) {
		WL_ERROR(("%s: wlc_key_set_data returned %d\n", __FUNCTION__, ret));
		return FALSE;
	}

	ret = wlc_key_set_data(key, CRYPTO_ALGO_BIP, igtk_kde->key, key_len);
	if (ret != BCME_OK) {
		WL_ERROR(("%s: wlc_key_set_data returned %d\n", __FUNCTION__, ret));
		return FALSE;
	}

	/* mark this key to be updated to host */
	RXOEUPDKEYROT(wlc_dngl_ol, 1 << igtk_kde->key_id);

	return TRUE;
}
#endif /* MFP */
