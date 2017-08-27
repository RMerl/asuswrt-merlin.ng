/*
 * Implementation of wlc_key operations
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key.c 678292 2017-01-09 02:10:56Z $
 */

#include "km_key_pvt.h"
#include <wlc_txc.h>

/* internal interface */

struct key_algo_entry {
	key_algo_t algo;
	key_algo_init_t init;
};
typedef struct key_algo_entry key_algo_entry_t;

#define KAE(algo, fn) {CRYPTO_ALGO_##algo, km_key_##fn##_init}

#ifndef BCM_OL_DEV
static const key_algo_entry_t key_algo_entries[] = {
	KAE(NONE, none),
	KAE(WEP1, wep),
	KAE(WEP128, wep),
#if defined(BCMCCX) || defined(BCMEXTCCX)
	KAE(CKIP, wep),
	KAE(CKIP_MMH, wep),
	KAE(WEP_MMH, wep),
#endif /* BCMCCX || BCMEXTCCX */
	KAE(AES_CCM, aes),
	KAE(AES_OCB_MSDU, aes),
	KAE(AES_OCB_MPDU, aes),
	KAE(AES_CCM256, aes),
	KAE(AES_GCM, aes),
	KAE(AES_GCM256, aes),
#ifdef MFP
	KAE(BIP, aes),
	KAE(BIP_CMAC256, aes),
	KAE(BIP_GMAC, aes),
	KAE(BIP_GMAC256, aes),
#endif /* MFP */
#ifndef LINUX_CRYPTO
	KAE(TKIP, tkip),
#else
	KAE(TKIP, tkip_linux),
#endif /* LINUX_CRYPTO */
#ifdef BCMWAPI_WPI
	KAE(SMS4, wapi),
#endif /* BCMWAPI_WPI */
};
#else
static const key_algo_entry_t key_algo_entries[] = {
	KAE(NONE, none),
	KAE(WEP1, wep),
	KAE(WEP128, wep),
	KAE(AES_CCM, aes),
	KAE(TKIP, tkip),
#ifdef MFP
	KAE(BIP, aes),
	KAE(BIP_CMAC256, aes),
	KAE(BIP_GMAC, aes),
	KAE(BIP_GMAC256, aes),
#endif /* MFP */
};
#endif /* !BCM_OL_DEV */

#undef KAE

static const key_algo_entry_t*
key_find_algo_entry(key_algo_t algo)
{
	size_t i;
	for (i = 0; i < ARRAYSIZE(key_algo_entries); ++i) {
		const key_algo_entry_t* ae = &key_algo_entries[i];
		if (ae->algo == algo)
			return ae;
	}
	return NULL;
}

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
static void
km_key_info_dump(wlc_key_t *key, key_info_t *key_info, struct bcmstrbuf *b)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	bcm_bprintf(b, "\tidx: %03d id: %01d hw idx %02d\n", key_info->key_idx,
		key_info->key_id, key->hw_idx);
	bcm_bprintf(b, "\talgo: %d (%s) hw algo: %d\n", key_info->algo,
		wlc_keymgmt_get_algo_name(KEY_KM(key), key_info->algo), key_info->hw_algo);
	bcm_bprintf(b, "\taddr: %s\n", bcm_ether_ntoa(&key_info->addr, eabuf));
	bcm_bprintf(b, "\tflags: 0x%08x\n", key_info->flags);
	bcm_bprintf(b, "\tlen: key %d, iv %d icv %d\n", key_info->key_len,
		key_info->iv_len, key_info->icv_len);
}
#endif /* BCMDBG || BCMDBG_DUMP */

enum km_key_pkt_type {
	KM_KEY_PKT_DOT11 = 0,
	KM_KEY_PKT_ETHER = 1
};
typedef enum km_key_pkt_type km_key_pkt_type_t;

/* locate body - it points at the snap header after ether header */
static void
km_key_get_body(void *pkt, void* hdr, size_t pkt_len,
	uint8 **body, int *body_len, km_key_pkt_type_t pkt_type,
	d11rxhdr_t *rxh)
{
	size_t body_offset;
	uint16 fc;

	switch (pkt_type) {
	case KM_KEY_PKT_DOT11:
		body_offset = DOT11_A3_HDR_LEN;
		fc  = ltoh16(((const struct dot11_header *)hdr)->fc);
		if (FC_TYPE(fc) == FC_TYPE_MNG) {
			if (rxh != NULL &&
				((rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
				(fc & FC_ORDER) && FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc))) {
				body_offset += DOT11_HTC_LEN;
			}
			break;
		}

		/* data or qos data */
		if ((fc & (FC_TODS|FC_FROMDS)) == (FC_TODS|FC_FROMDS)) /* wds ? */
			body_offset = DOT11_A4_HDR_LEN;
		if ((fc & FC_KIND_MASK) == FC_QOS_DATA) /* qos ? */
			body_offset += DOT11_QOS_LEN;
		break;
	case KM_KEY_PKT_ETHER:
		body_offset = ETHER_HDR_LEN;
		break;
	default:
		KM_DBG_ASSERT(FALSE);
		body_offset = 0;
		break;
	}

	KM_ASSERT(pkt_len >= body_offset);

	*body = ((uint8 *)hdr) + body_offset;
	*body_len = (int)(pkt_len - body_offset);
}

/* update algo, hw/key idx that depends on corerev.
 */
static void
km_key_update_txd(wlc_key_t *key, void *pkt,
	const struct dot11_header *hdr, uint8 *body, wlc_txd_t *txd)
{
	uint16 mch;

	KM_DBG_ASSERT(txd != NULL);
	if (KEY_USE_AC_TXD(key)) {
		d11actxh_t *ac_txh = (d11actxh_t *)&txd->txd;
		d11actxh_cache_t	*cache_info;

		cache_info = WLC_TXD_CACHE_INFO_GET(ac_txh, KEY_PUB(key)->corerev);
		cache_info->BssIdEncAlg |= key->info.hw_algo << D11AC_ENCRYPT_ALG_SHIFT;
		cache_info->KeyIdx = (uint8)KM_KEY_HW_IDX_TO_SLOT(key, key->hw_idx);

#ifdef UCODE_SEQ
		mch = ltoh16(ac_txh->PktInfo.MacTxControlHigh) & ~D11AC_TXC_UCODE_SEQ;
#ifndef BCM_OL_DEV
		if (WLC_KEY_ARM_TX_ENABLED(&key->info)) {
			uint16 fifo = WLC_TXFID_GET_QUEUE(ltoh16(ac_txh->PktInfo.TxFrameID));
			if (fifo == TX_AC_BK_FIFO)
				mch |= D11AC_TXC_UCODE_SEQ;
		}
#else
		{
			uint16 fc;
			fc = ltoh16(hdr->fc);
			if ((fc & FC_KIND_MASK) == FC_QOS_DATA)
				mch |= D11AC_TXC_UCODE_SEQ;
		}
#endif /* !BCM_OL_DEV */
		ac_txh->PktInfo.MacTxControlHigh = htol16(mch);
#endif /* UCODE_SEQ */

	} else {
		d11txh_t *txh = (d11txh_t *)&txd->d11txh;
		mch = ltoh16(txh->MacTxControlHigh);
		mch &= ~(TXC_SECKEY_MASK | TXC_SECTYPE_MASK);
		mch |= (key->hw_idx << TXC_SECKEY_SHIFT) & TXC_SECKEY_MASK;
		mch |= (key->info.hw_algo << TXC_SECTYPE_SHIFT) & TXC_SECTYPE_MASK;
		txh->MacTxControlHigh = htol16(mch);
	}
}

/* public interface */
int
km_key_create(keymgmt_t *km, const key_info_t *key_info, wlc_key_t **out_key)
{
	const key_algo_entry_t *ae;
	wlc_key_t *key = NULL;
	int err = BCME_OK;
	wlc_info_t *wlc;

	wlc = wlc_keymgmt_get_wlc(km);
	KM_DBG_ASSERT(km != NULL && wlc != NULL &&
		key_info != NULL && out_key != NULL);

	ae = key_find_algo_entry(key_info->algo);
	if (ae == NULL) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	key = (wlc_key_t *)MALLOCZ(wlc->osh, sizeof(wlc_key_t));
	if (key == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	KEY_MAGIC_INIT(key);
	key->wlc = wlc;
	key->info = *key_info;

	key->info.flags |= WLC_KEY_FLAG_VALID;
#ifdef BCM_OL_DEV
	 key->info.flags |= WLC_KEY_FLAG_NO_HW_UPDATE;
#endif /* BCM_OL_DEV */
	if (KEY_COREREV_GE40(key))
		key->info.flags |= WLC_KEY_FLAG_USE_AC_TXD;

	key->hw_idx = WLC_KEY_INDEX_INVALID;

	/* depending on type - fill in rx/tx by calling init func */
	err = ae->init(key);
	if (err != BCME_OK)
		goto done;

done:
	if (err != BCME_OK) {
		if (key != NULL) {
			MFREE(wlc->osh, key, sizeof(wlc_key_t));
			key = NULL;
		}
	}

	*out_key = key;

	KEY_LOG(("wl%d: %s: exit with status %d key %p\n", WLCWLUNIT(wlc),
		__FUNCTION__, err, key));
	return err;
}

int
km_key_destroy(wlc_key_t **in_key)
{
	wlc_key_t *key;
	int err;

	KM_DBG_ASSERT(in_key != NULL && KEY_VALID(*in_key));
	key = *in_key;

	/* clear key before destroying */
	(void)wlc_key_set_data(key, key->info.algo, NULL, 0);

	km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_DELETE,
		NULL /* bsscfg */, NULL /* scb */, key, NULL /* pkt */);

	/* note: caller key address is cleared after notification */
	*in_key = NULL;

	KEY_ALGO_CB(err, key->algo_impl.cb->destroy, (key));
	if (err != BCME_OK && err != BCME_UNSUPPORTED) {
		KEY_LOG(("wl%d: %s: error %d destroying key@%p algo impl\n",
			KEY_WLUNIT(key), __FUNCTION__, err, key));
	}
	MFREE(KEY_OSH(key), key, sizeof(wlc_key_t));
	return BCME_OK;
}

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
void
km_key_dump(wlc_key_t *key, struct bcmstrbuf *b)
{
	int err;
	KM_DBG_ASSERT(KEY_VALID(key));

	bcm_bprintf(b, "begin key@%p dump:\n", key);
	bcm_bprintf(b, "\twlc: %p\n", KEY_WLC(key));
	km_key_info_dump(key, &key->info, b);
	bcm_bprintf(b, "\texp: %d\n", key->exp);
	KEY_ALGO_CB(err, key->algo_impl.cb->dump, (key, b));
	if (err != BCME_OK && err != BCME_UNSUPPORTED) {
		KEY_LOG(("wl%d: %s: error %d dumping key@%p algo impl\n",
			KEY_WLUNIT(key), __FUNCTION__, err, key));
	}
	bcm_bprintf(b, "end key@%p dump:\n", key);
}
#endif /* BCMDBG || BCMDBG_DUMP */

void
wlc_key_get_info(const wlc_key_t *key, key_info_t *key_info)
{
	if (KEY_VALID(key)) {
		if (key_info != NULL)
			*key_info = key->info;
	} else {
		if (key_info) {
			memset(key_info, 0, sizeof(key_info_t));
			key_info->key_idx = WLC_KEY_INDEX_INVALID;
			key_info->key_id = WLC_KEY_ID_INVALID;
		}
	}
}

int wlc_key_prep_tx_mpdu(wlc_key_t *key, void *pkt, wlc_txd_t *txd)
{
	int err;
	struct dot11_header *hdr;
	uint8 *body;
	int body_len = 0;
	uint16 fc;
	uint16 fk;

	KM_ASSERT(KEY_VALID(key));

	KEY_LOG(("wl%d: %s: enter pkt@%p key idx %d algo %d[%s] flags 0x%08x\n",
		KEY_WLUNIT(key), __FUNCTION__, pkt, key->info.key_idx, key->info.algo,
		wlc_keymgmt_get_algo_name(KEY_KM(key), key->info.algo), key->info.flags));

	/* check if the key is valid for tx */
	if (!KEY_TX_ENABLED(key) || (key->info.algo == CRYPTO_ALGO_OFF)) {
		err = (key->info.algo == CRYPTO_ALGO_OFF) ? BCME_OK : BCME_NOTENABLED;
		goto done;
	}
#if defined(BCMDBG) && !defined(BCM_OL_DEV)
	else if (key->info.flags & (WLC_KEY_FLAG_GROUP | WLC_KEY_FLAG_MGMT_GROUP)) {
		scb_t *scb = WLPKTTAGSCBGET(pkt);
		KM_DBG_ASSERT(scb != NULL);
		if (scb != NULL) {
			wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
			BCM_REFERENCE(bsscfg);
			KM_DBG_ASSERT(bsscfg != NULL);
			KM_DBG_ASSERT(BSSCFG_AP(bsscfg) || KM_BSSCFG_IS_IBSS(bsscfg) ||
				(key->info.algo == CRYPTO_ALGO_WEP1) ||
				(key->info.algo == CRYPTO_ALGO_WEP128));
		}
	}
#endif /* BCMDBG && !BCM_OL_DEV */

	/* Map remaining unmapped pkt contents to cover the cases where
	 * we receive hw decrypted frame and have to send it using sw encr
	 */
	PKTCTFMAP(KEY_OSH(key), pkt);

	hdr = KEY_MPDU_HDR(key, pkt);

	/* debug: dump pkt */
	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

	fc  = ltoh16(hdr->fc);
	fk = fc & FC_KIND_MASK;
	KM_ASSERT((FC_TYPE(fc) == FC_TYPE_MNG) || (fk == FC_DATA) || (fk == FC_QOS_DATA));

	km_key_get_body(pkt, (void*)hdr, KEY_PKT_LEN(key, pkt),
		&body, &body_len, KM_KEY_PKT_DOT11, NULL);

	/* set protected frame bit for data frames, robust uc mgmt
	 * frames and mgmt auth frames. 802.11/2012 8.2.4.1.9, but
	 * not for mc mgmt frames
	 */
	if ((fk == FC_DATA) || (fk == FC_QOS_DATA) ||
		((FC_TYPE(fc) == FC_TYPE_MNG) && (!ETHER_ISMULTI(&hdr->a1) || (fk == FC_AUTH)))) {
		fc |= FC_WEP;
		hdr->fc = htol16(fc);

		/* ensure we have iv */
		if (body_len < key->info.iv_len) {
			KEY_LOG(("wl%d: %s: packet body of len %d has no space for iv\n",
				KEY_WLUNIT(key), __FUNCTION__, body_len));
			err = BCME_BUFTOOSHORT;
			goto done;
		}
	}

	/* if protecting mgmt packet, mark it as MFP and use sw/ enc */
	if (FC_TYPE(fc) == FC_TYPE_MNG && fk != FC_AUTH) {
		WLPKTTAG(pkt)->flags |= WLF_MFP;
		txd = NULL;
	}

	/* dispatch to algo impl which for h/w encr only updates iv */
	KEY_ALGO_CB(err, key->algo_impl.cb->tx_mpdu, (key, pkt, hdr, body, body_len, txd));
	if (err != BCME_OK)
		goto done;

#if defined(BCMDBG) && !defined(BCM_OL_DEV)
	/* ICV error generation support */
	if (key->info.flags & WLC_KEY_FLAG_GEN_ICV_ERR) {
		if (FC_TYPE(fc) != FC_TYPE_MNG || !ETHER_ISMULTI(&hdr->a1)) {
			key->info.flags &= ~WLC_KEY_FLAG_GEN_ICV_ERR;
			body[body_len] = ~body[body_len];
		}
	}
#endif /* BCMDBG && !BCM_OL_DEV */

	/* update pkt len except for mc mgmt frames unless linux crypto or hw enc
	 * if hw enc, update txd(txh) - hw/algo, key idx.
	 */
	if (!WLC_KEY_IN_HW(&key->info) || txd == NULL) {
		if (FC_TYPE(fc) != FC_TYPE_MNG || !ETHER_ISMULTI(&hdr->a1)) {
			if (!(WLC_KEY_IS_LINUX_CRYPTO(&key->info)))
				PKTSETLEN(KEY_OSH(key), pkt,
					PKTLEN(KEY_OSH(key), pkt) + key->info.icv_len);
		}
	} else {
		km_key_update_txd(key, pkt, hdr, body, txd);
	}

	/* note: algorithm dependent mic may have been added to the body of the packet.
	 * if body_len is needed beyond this point it must be adjusted appropriately.
	 * body_len = key->info.icv_len + mic_size
	 */

	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

done:
	KEY_LOG(("wl%d: %s: exit status %d\n", KEY_WLUNIT(key), __FUNCTION__, err));
	return err;
}

int
wlc_key_prep_tx_msdu(wlc_key_t *key, void *pkt, size_t frag_length, uint8 prio)
{
	int err;
	struct ether_header *hdr = NULL;
	uint8 *body = NULL;
	int body_len = 0;

	KM_DBG_ASSERT(KEY_VALID(key));

	KEY_LOG(("wl%d: %s: enter pkt@%p key idx %d\n", KEY_WLUNIT(key),
		__FUNCTION__, pkt, key->info.key_idx));

	/* check if the key is valid for tx */
	if (!KEY_TX_ENABLED(key)) {
		err = (key->info.algo == CRYPTO_ALGO_OFF) ? BCME_OK : BCME_NOTENABLED;
		goto done;
	}

	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

	if (key->algo_impl.cb->tx_msdu != NULL) {
		hdr = KEY_MSDU_HDR(key, pkt);
		km_key_get_body(pkt, (void*)hdr, KEY_PKT_LEN(key, pkt),
			&body, &body_len, KM_KEY_PKT_ETHER, NULL);
	}

	KEY_ALGO_CB(err, key->algo_impl.cb->tx_msdu, (key, pkt, hdr,
		body, body_len, frag_length, prio));
	/* some algorithms don't support tx msdu callback */
	if (err == BCME_UNSUPPORTED)
		err = BCME_OK;

done:
	KEY_LOG(("wl%d: %s: exit with status %d\n", KEY_WLUNIT(key), __FUNCTION__, err));
	return err;
}

int
wlc_key_rx_mpdu(wlc_key_t *key, void *pkt, d11rxhdr_t *rxh)
{
	struct dot11_header *hdr;
	uint8 *body;
	int body_len = 0;
	uint16 fc;
	uint16 fk;
	uint16 qc;
	bool hwdec = TRUE;
	int err = BCME_OK;

	KM_DBG_ASSERT(KEY_VALID(key) && rxh != NULL);

	KEY_LOG(("wl%d: %s: enter pkt@%p rxs %04x DECATMPT %d DECERR %d\n",
		KEY_WLUNIT(key), __FUNCTION__, pkt, rxh->RxStatus1,
		((rxh->RxStatus1 & RXS_DECATMPT) ? 1 : 0),
		((rxh->RxStatus1 & RXS_DECERR) ? 1 : 0)));

	hdr = KEY_MPDU_HDR(key, pkt);

	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

	km_key_get_body(pkt, (void*)hdr, KEY_PKT_LEN(key, pkt),
		&body, &body_len, KM_KEY_PKT_DOT11, rxh);

	fc  = ltoh16(hdr->fc);
	fk = fc & FC_KIND_MASK;
	KM_ASSERT((FC_TYPE(fc) == FC_TYPE_MNG) ||
		(fk == FC_DATA) || (fk == FC_QOS_DATA));

	if (fk == FC_QOS_DATA) /* qos ? */
		qc = ltoh16_ua(body - DOT11_QOS_LEN);
	else
		qc = 0;

	/* check if the key is valid for rx */
	if (!KEY_RX_ENABLED(key)) {
		err = BCME_NOTENABLED;
		goto done;
	}

	/* wep must be set except for  bc/mc mgmt frames */
	if (!(fc & FC_WEP)) {
		if ((FC_TYPE(fc) != FC_TYPE_MNG) || !ETHER_ISMULTI(&hdr->a1)) {
			err = BCME_UNSUPPORTED;
			goto done;
		}
	}

	if (body_len < (key->info.iv_len + key->info.icv_len)) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	/* handle h/w decrypted */
#ifdef BCMWAPI_WPI
	if ((rxh->RxStatus1 & RXS_DECATMPT) &&
		(key->info.algo == CRYPTO_ALGO_SMS4) &&
		KEY_WLC(key)->pr80838_war &&
		(key->info.flags & WLC_KEY_FLAG_DEFAULT_BSS)) {
		rxh->RxStatus1 &= ~RXS_DECATMPT;
	}
#endif /* BCMWAPI_WPI */

#ifdef BCMDBG
	/* mfp currently done in s/w */
	if (WLPKTFLAG_PMF(WLPKTTAG(pkt)))
		KM_DBG_ASSERT(!(rxh->RxStatus1 & RXS_DECATMPT));
#endif /* BCMDBG */

#ifndef BCM_OL_DEV
	if ((rxh->RxStatus1 & RXS_DECATMPT) &&
		(FC_TYPE(fc) == FC_TYPE_MNG) && ((fc & FC_KIND_MASK) == FC_AUTH) &&
		KM_WEP_ALGO(key->info.algo) && !WLC_KEY_IS_DEFAULT_BSS(&key->info)) {
		err = km_key_wep_rx_defkey_fixup(key, body, body_len);
		if (err != BCME_OK)
			goto done;
		rxh->RxStatus1 &= ~(RXS_DECATMPT | RXS_DECERR);
	}
#endif /* !BCM_OL_DEV */

	/* algorithm specific processing */
	if (KEY_SW_ENC_DEC(key, pkt, rxh)) {
		hwdec = FALSE;
		if (WLPKTTAG(pkt)->flags & (WLF_AMSDU | WLF_RX_KM)) {
			KEY_LOG(("wl%d: %s: no sw support for A-MSDU deagg or ICV is trimmed!"
				"-  key idx %d, pkt flags %d\n", KEY_WLUNIT(key), __FUNCTION__,
				key->info.key_idx, WLPKTTAG(pkt)->flags));
			err = BCME_UNSUPPORTED;
		}

	} else if (rxh->RxStatus1 & RXS_DECERR) {
		err = BCME_DECERR;
		if (key->info.algo == CRYPTO_ALGO_TKIP &&
			body_len <= (DOT11_IV_TKIP_LEN + TKIP_MIC_SIZE + DOT11_ICV_LEN)) {
			hwdec = FALSE;
		}
	}

	if (err == BCME_OK || err == BCME_DECERR) {
		key_hw_rx_info_t hw_rxi;

		/* ensure we have the full frame if required for s/w decrypt or mic */
		if ((PKTFRAGUSEDLEN(KEY_OSH(key), pkt) > 0) &&
			(!hwdec || ((WLC_KEY_ALGO_HAS_MIC(key->info.algo) &&
			!WLC_KEY_MIC_IN_HW(&key->info))))) {
			err = BCME_DATA_NOTFOUND;
			goto done;
		}

		hw_rxi.attempted = hwdec;
		hw_rxi.status = err;
		KEY_ALGO_CB(err, key->algo_impl.cb->rx_mpdu,
			(key, pkt, hdr, body, body_len, &hw_rxi));
	}

	/* if algorithm does not support rx/decrypt, signal an error.
	 * also, per spec increment wep error counts
	 */
	if (err == BCME_UNSUPPORTED) {
		WLCNTINCR(KEY_CNT(key)->wepundec);
		if (ETHER_ISMULTI(&hdr->a1))
			WLCNTINCR(KEY_CNT(key)->wepundec_mcst);
	} else {
		rxh->RxStatus1 |= RXS_DECATMPT; /* attempted h/w or s/w */
		if (!hwdec)
			KEY_WLCNINC(key, swdecrypt);
		if (err == BCME_DECERR || err == BCME_MICERR)
			rxh->RxStatus1 |= RXS_DECERR;
	}

	/* strip off icv */
	if (!(WLC_KEY_IS_LINUX_CRYPTO(&key->info))) {

		KM_ASSERT(!(WLPKTTAG(pkt)->flags & WLF_RX_KM));

		if (FC_TYPE(fc) != FC_TYPE_MNG || !ETHER_ISMULTI(&hdr->a1)) {
			void *last_pkt = pkt;
			if ((qc & QOS_AMSDU_MASK))
				last_pkt = pktlast(KEY_OSH(key), pkt);
			/* SPLIT RX: icv is in the host */
			PKTFRAG_TRIM_TAILBYTES(KEY_OSH(key), last_pkt, key->info.icv_len,
				TAIL_BYTES_TYPE_ICV);
			WLPKTTAG(pkt)->flags |= WLF_RX_KM;
		}
	}

	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

done:
#ifdef CTFMAP
	if (PKTISFAST(KEY_OSH(key), pkt)) {
		DMA_MAP(KEY_OSH(key), PKTDATA(KEY_OSH(key), pkt),
			PKTLEN(KEY_OSH(key), pkt), DMA_TX, NULL, NULL);
	}
#endif

	if (err != BCME_OK) {
		if (fc & FC_WEP) {
			WLCNTINCR(KEY_CNT(key)->rxundec);
			if (ETHER_ISMULTI(&hdr->a1))
				WLCNTINCR(KEY_CNT(key)->rxundec_mcst);

			/* notify keymgmt of error */
			switch (err) {
			case BCME_DATA_NOTFOUND:
				km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_NEED_PKTFETCH,
					NULL, NULL, key, pkt);
				break;
			case BCME_DECERR: /* fall through */
			case BCME_MICERR: /* mpdu MIC error is treated as a decrypt error */
				km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_DECRYPT_ERROR,
					NULL, NULL, key, pkt);
				break;
			case BCME_BUFTOOSHORT:
			case BCME_BADLEN:
				WLCNTINCR(KEY_CNT(key)->rxrunt);
				/* fall through */
			default:
				km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_DECODE_ERROR,
					NULL, NULL, key, pkt);
				break;
			}
		} else { /*  unencrypted */
			scb_t *scb = WLPKTTAGSCBGET(pkt);
			KM_DBG_ASSERT(scb != NULL);
			if (!km_allow_unencrypted(KEY_KM(key), &key->info, scb,
				hdr, qc, body, body_len)) {
				KEY_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);
				WLCNTINCR(KEY_CNT(key)->rxbadproto);
				WLCNTINCR(KEY_CNT(key)->wepexcluded);
				if (ETHER_ISMULTI(&hdr->a1))
					WLCNTINCR(KEY_CNT(key)->wepexcluded_mcst);

#if defined(BCMCCX) && defined(CCX_SDK)
				if (FC_TYPE(fc) == FC_TYPE_MNG) {
					if (key->info.algo == CRYPTO_ALGO_TKIP)
						WLCNTINCR(
						KEY_CCX(key)->mgmt_cnt.tkipmgmtnoencrypterr);
					else if (key->info.algo == CRYPTO_ALGO_AES_CCM)
						WLCNTINCR(
						KEY_CCX(key)->mgmt_cnt.tkipmgmtnoencrypterr);
				}
#endif
				KEY_LOG(("wl%d.%d: %s: disallowed unencrypted frame from %s\n",
					KEY_WLUNIT(key), WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
					__FUNCTION__, bcm_ether_ntoa(&hdr->a2, eabuf)));
			} else {
				err = BCME_OK;
			}
		}
	} else {
		WLCNTINCR(KEY_CNT(key)->decsuccess);
		if (ETHER_ISMULTI(&hdr->a1))
			WLCNTINCR(KEY_CNT(key)->decsuccess_mcst);
	}

	KEY_LOG(("wl%d: %s: exit status %d rxs %04x DECATMPT %d DECERR %d wep %d\n",
		KEY_WLUNIT(key), __FUNCTION__, err, rxh->RxStatus1,
		((rxh->RxStatus1 & RXS_DECATMPT) ? 1 : 0), ((rxh->RxStatus1 & RXS_DECERR) ? 1 : 0),
		((fc & FC_WEP) ? 1 : 0)));
	return err;
}

int
wlc_key_rx_msdu(wlc_key_t *key, void *pkt, d11rxhdr_t *rxh)
{
	int err;
	struct ether_header *hdr;
	uint8 *body;
	int body_len = 0;
	key_hw_rx_info_t hw_rxi;

	KM_ASSERT(KEY_VALID(key));

	KEY_LOG(("wl%d: %s: enter pkt@%p rxs status1 0x%04x  status2 0x%04x\n",
		KEY_WLUNIT(key), __FUNCTION__, pkt, rxh->RxStatus1, rxh->RxStatus2));

	/* check if the key is valid for rx */
	if (!KEY_RX_ENABLED(key)) {
		err = BCME_NOTENABLED;
		goto done;
	}

	hdr = KEY_MSDU_HDR(key, pkt);

	KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);

	km_key_get_body(pkt, (void*)hdr, KEY_PKT_LEN(key, pkt),
		&body, &body_len, KM_KEY_PKT_ETHER, rxh);

	/* algorithm specific processing */
	hw_rxi.attempted = WLC_KEY_MIC_IN_HW(&key->info) &&
		key->info.algo == CRYPTO_ALGO_TKIP &&
		(rxh->RxStatus2 & RXS_TKMICATMPT);
	hw_rxi.status = BCME_OK;
	if (hw_rxi.attempted && (rxh->RxStatus2 & RXS_TKMICERR)) {
		hw_rxi.status = BCME_MICERR;
	}

	KEY_ALGO_CB(err, key->algo_impl.cb->rx_msdu, (key, pkt, hdr, body, body_len, &hw_rxi));
	if (err == BCME_OK) {
		WLCNTINCR(KEY_CNT(key)->decsuccess);
		if (ETHER_ISMULTI(&hdr->ether_dhost[0]))
			WLCNTINCR(KEY_CNT(key)->decsuccess_mcst);
	}

#if defined(BCMDBG) && !defined(BCM_OL_DEV)
	if (err != BCME_UNSUPPORTED) {
		KEY_LOG_DUMP_PKT(__FUNCTION__, key, pkt);
	}
#endif

done:
	/* check error and notify keymgmt */
	switch (err) {
	case BCME_OK:
		break;
	case BCME_BUFTOOSHORT:
	case BCME_BADLEN:
		WLCNTINCR(KEY_CNT(key)->rxrunt);
		break;
	case BCME_MICERR:
		km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_MSDU_MIC_ERROR, NULL, NULL, key, pkt);
		break;
	case BCME_UNSUPPORTED: /* rx_msdu is optional, mask error if unsupported */
		err = BCME_OK;
		break;
	default:
		km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_DECODE_ERROR, NULL, NULL, key, pkt);
		break;
	}

	KEY_LOG(("wl%d: %s: exit status %d\n", KEY_WLUNIT(key), __FUNCTION__, err));
	return err;
}

int
wlc_key_get_seq(wlc_key_t *key, uint8 *buf, size_t buf_len,
    key_seq_id_t seq_id, bool tx)
{
	size_t data_len = 0;
	int err;

	KM_DBG_ASSERT(KEY_VALID(key));
	KEY_ALGO_CB(err,  key->algo_impl.cb->get_data,
		(key, buf, buf_len, &data_len, WLC_KEY_DATA_TYPE_SEQ, seq_id, tx));

	if (err == BCME_OK)
		return (int)data_len;
	else
		return -((int)data_len);
}

int
wlc_key_set_seq(wlc_key_t *key, const uint8 *seq, size_t seq_len,
    key_seq_id_t seq_id, bool tx)
{
	int err;

	KM_DBG_ASSERT(KEY_VALID(key));

	/* allow only one tx seq, and for rx, setting the seq can be selected */
	if (tx || WLC_KEY_IS_MGMT_GROUP(&key->info))
		seq_id = 0;

	KEY_ALGO_CB(err, key->algo_impl.cb->set_data,
		(key, seq, seq_len, WLC_KEY_DATA_TYPE_SEQ, seq_id, tx));

	KEY_LOG(("wl%d: %s: key@%p key_idx 0x%04x seq len %d status %d\n",
		KEY_WLUNIT(key), __FUNCTION__, key, key->info.key_idx, (int)seq_len, err));

	return err;
}

int
wlc_key_set_data(wlc_key_t *key, wlc_key_algo_t algo,
    const uint8 *data, size_t data_len)
{
	const key_algo_entry_t *ae;
	int err;

	KM_DBG_ASSERT(KEY_VALID(key));
	KM_ASSERT(data_len == 0 || data != NULL);

	KEY_LOG(("wl%d: %s: enter key@%p key idx 0x%04x algo %d[%s] data len %d\n",
		KEY_WLUNIT(key), __FUNCTION__, key, key->info.key_idx, algo,
		wlc_keymgmt_get_algo_name(KEY_KM(key), algo), (int)data_len));

	/* algo must be supported, and key must have a valid key index */
	if (!km_algo_is_supported(KEY_KM(key), algo) ||
		(key->info.key_idx == WLC_KEY_INDEX_INVALID)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	/* null data is treated as changing to algo 'none' */
	if (!data_len) {
		algo = CRYPTO_ALGO_NONE;
	}

	/* handle algorithm change. */
	if (key->info.algo != algo) {
		/* reset key to clear any data - any h/w data associated with
		 * the key is cleared when keymgmt notification is handled.
		 * destroy key algorithm specific data
		 * update the algo and init algo implementation
		 */
		ae = key_find_algo_entry(algo);
		if (!ae) {
			err = BCME_UNSUPPORTED;
			goto done;
		}

		KEY_ALGO_CB(err, key->algo_impl.cb->set_data,
			(key, NULL, 0, WLC_KEY_DATA_TYPE_KEY, 0, TRUE));
		if (err != BCME_OK && err != BCME_UNSUPPORTED) {
			KEY_LOG(("wl%d: %s: error %d setting data for key@%p algo impl\n",
				KEY_WLUNIT(key), __FUNCTION__, err, key));
		}

		/* this notification is for clearing external state (e.g. hw key data) if any */
		km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE,
			NULL /* bsscfg */, NULL /* scb */, key, NULL /* pkt */);

		KEY_ALGO_CB(err, key->algo_impl.cb->destroy, (key));
		if (err != BCME_OK && err != BCME_UNSUPPORTED) {
			KEY_LOG(("wl%d: %s: error %d destroying key@%p algo impl\n",
				KEY_WLUNIT(key), __FUNCTION__, err, key));
		}

		key->info.algo = algo;
		key->info.hw_algo = wlc_keymgmt_algo_to_hw_algo(KEY_KM(key), algo);
		err = ae->init(key);
		KM_DBG_ASSERT(err == BCME_OK);
		if (err != BCME_OK)
			goto done;
	}

	KEY_ALGO_CB(err, key->algo_impl.cb->set_data,
		(key, data, data_len, WLC_KEY_DATA_TYPE_KEY, 0, TRUE));
	if (err != BCME_OK) {
		if  (err != BCME_UNSUPPORTED)
			goto done;
		err = BCME_OK;
	}

#ifdef GTK_RESET
	if (!WLC_KEY_IS_AP(&key->info) && WLC_KEY_IS_GROUP(&key->info))
		key->info.flags |= WLC_KEY_FLAG_GTK_RESET;
#endif
	key->exp = 0; /* new data, so clear expiration */
	km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE,
		NULL /* bsscfg */, NULL /* scb */, key, NULL /* pkt */);

done:
	KEY_LOG(("wl%d: %s: exit status %d\n", KEY_WLUNIT(key), __FUNCTION__, err));
	return err;
}

/* Reset data and any other state for the key. Algorithm changes to none . */
int
wlc_key_reset(wlc_key_t *key)
{
	return wlc_key_set_data(key, key->info.algo, NULL, 0);
}

/* internal to keymgmt implementation - used to access key data. in
 * debug builds, the data is made available via wlc_key_get_data...
 */
int km_key_get_data(wlc_key_t *key, uint8* data, size_t data_size,
    size_t *data_len, wlc_key_data_type_t data_type, int instance, bool tx)
{
	int err;

	STATIC_ASSERT(sizeof(key->info.flags) == sizeof(uint32));

	KM_DBG_ASSERT(KEY_VALID(key));

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_FLAGS:
		if (data_len)
			*data_len = sizeof(key->info.flags);
		if (data_size < sizeof(key->info.flags)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		htol32_ua_store(key->info.flags, data);
		err = BCME_OK;
		break;
	default:
		KEY_ALGO_CB(err, key->algo_impl.cb->get_data,
			(key, data, data_size, data_len, data_type, instance, tx));
		break;
	}

	return err;
}

void
km_key_set_flags(wlc_key_t *key, wlc_key_flags_t flags)
{
	KM_DBG_ASSERT(KEY_VALID(key));

	flags = (key->info.flags & ~KM_KEY_SETTABLE_FLAGS) |
		(flags & KM_KEY_SETTABLE_FLAGS);

	if (flags != key->info.flags) {
		bool do_notify = KM_KEY_FLAGS_NOTIFY_MASK(key->info.flags) !=
			KM_KEY_FLAGS_NOTIFY_MASK(flags);
		key->info.flags = flags;
		if (do_notify)
			km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE,
				NULL /* bsscfg */, NULL /* scb */, key, NULL /* pkt */);
	}
}

int
wlc_key_set_flags(wlc_key_t *key, wlc_key_flags_t flags)
{
	int err = BCME_OK;

	KM_DBG_ASSERT(KEY_VALID(key));

	if (key->info.key_idx == WLC_KEY_INDEX_INVALID) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	flags &= WLC_KEY_SETTABLE_FLAGS;
	flags |= (key->info.flags & ~WLC_KEY_SETTABLE_FLAGS);
	km_key_set_flags(key, flags);

done:
	return err;
}

int
wlc_key_get_data(wlc_key_t *key, uint8* data, size_t data_size,
	size_t *data_len)
{
	return km_key_get_data(key, data, data_size, data_len,
		WLC_KEY_DATA_TYPE_KEY, 0, TRUE);
}

int
wlc_key_get_data_ex(wlc_key_t *key, uint8* data, size_t data_size,
    size_t *data_len, key_data_type_t data_type, int instance, bool tx)
{
	return km_key_get_data(key, data, data_size, data_len, data_type,
		instance, tx);
}

int
wlc_key_set_data_ex(wlc_key_t *key, uint8* data, size_t data_len,
    key_data_type_t data_type, int instance, bool tx)
{
	int err;

	KM_DBG_ASSERT(KEY_VALID(key));

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		err = wlc_key_set_data(key, key->info.algo, data, data_len);
		break;
	default:
		KEY_ALGO_CB(err, key->algo_impl.cb->set_data,
			(key, data, data_len, data_type, instance, tx));
		break;
	}

	return err;
}

size_t
wlc_key_pn_to_seq(uint8 *seq, size_t seq_size, uint16 lo, uint32 hi)
{
	const size_t seq_len = sizeof(lo) + sizeof(hi);

	if (seq_size < seq_len)
		return 0;

	memset(seq, 0, seq_size);
	htol16_ua_store(lo, seq);
	htol32_ua_store(hi, seq + sizeof(lo));
	return seq_len;
}

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBG_DUMP) || \
	defined(WLMSG_WSEC))
const char*
wlc_key_get_data_type_name(wlc_key_data_type_t data_type)
{
	const char *name = "none";
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY: name = "key"; break;
	case WLC_KEY_DATA_TYPE_SEQ: name = "seq"; break;
	case WLC_KEY_DATA_TYPE_TKHASH_P1: name = "tkip-hash-phase1"; break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS: name = "mic-key-from-ds"; break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS: name = "mic-key-to-ds"; break;
	case WLC_KEY_DATA_TYPE_FLAGS: name = "flags"; break;
	case WLC_KEY_DATA_TYPE_MIC_KEY: name = "mic-key"; break;
	case WLC_KEY_DATA_TYPE_TKIP_TK: name = "key"; break;
	default:
		break;
	}
	return name;
}
#endif /* BCMDBG || BCMDBG_DUMP || WLMSG_WSEC */

#ifdef BCMWAPI_WPI
wlc_key_expiration_t
wlc_key_get_expiration(wlc_key_t *key)
{
	KM_DBG_ASSERT(KEY_VALID(key));
	return key->exp;
}

wlc_key_expiration_t
wlc_key_set_expiration(wlc_key_t *key, wlc_key_expiration_t exp)
{
	KM_DBG_ASSERT(KEY_VALID(key));
	KM_SWAP(wlc_key_expiration_t, exp, key->exp);
	return exp;
}
#endif /* BCMWAPI_WPI */

bool
km_key_seq_less(const uint8 *seq1, const uint8* seq2, size_t seq_len)
{
	size_t i;
	bool less = FALSE;
	for (i = seq_len; i > 0; --i) {
		if (seq1[i-1] < seq2[i-1]) {
			less = TRUE;
			break;
		} else if (seq1[i-1] > seq2[i-1])
			break;
	}
	return less;
}

bool
wlc_key_seq_less(wlc_key_t *key, const uint8 *seq, size_t seq_len,
	wlc_key_seq_id_t seq_id, bool tx)
{
	uint8 key_seq[KM_KEY_MAX_DATA_LEN];
	int key_seq_len;
	key_seq_len = wlc_key_get_seq(key, key_seq, sizeof(key_seq), seq_id, tx);
	KM_DBG_ASSERT(key_seq_len >= 0 && key_seq_len <= sizeof(key_seq));
	return km_key_seq_less(key_seq, seq, (size_t)MIN(key_seq_len, (int)seq_len));
}

int
wlc_key_advance_seq(wlc_key_t *key, const uint8 *seq, size_t seq_len,
	wlc_key_seq_id_t seq_id, bool tx)
{
	int err = BCME_OK;
	if (wlc_key_seq_less(key, seq, seq_len, seq_id, tx))
		err = wlc_key_set_seq(key, seq, seq_len, seq_id, tx);
	return err;
}

wlc_key_hw_index_t
wlc_key_get_hw_idx(wlc_key_t *key)
{
	KM_DBG_ASSERT(KEY_VALID(key));
	return key->hw_idx;
}

wlc_key_hw_index_t
km_key_set_hw_idx(wlc_key_t *key, wlc_key_hw_index_t hw_idx, bool hw_mic)
{
	KM_DBG_ASSERT(KEY_VALID(key));

	key->info.flags &= ~(WLC_KEY_FLAG_IN_HW | WLC_KEY_FLAG_HW_MIC);
	if (hw_idx != WLC_KEY_INDEX_INVALID) {
		key->info.flags |= WLC_KEY_FLAG_IN_HW;
		if (hw_mic)
			key->info.flags |= WLC_KEY_FLAG_HW_MIC;
	}

	KM_SWAP(wlc_key_hw_index_t, hw_idx, key->hw_idx);

	/* h/w index for a key changed. reset keymgmt key cache */
	KM_RESET_KEY_CACHE(KEY_KM(key));

	return hw_idx;
}

#ifdef BCM_OL_DEV
wlc_key_hw_index_t
wlc_key_set_hw_idx(wlc_key_t *key, wlc_key_hw_index_t hw_idx, bool hw_mic)
{
	return km_key_set_hw_idx(key, hw_idx, hw_mic);
}
#endif /* BCM_OL_DEV */

#if defined(PKTC) || defined(PKTC_DONGLE)
void
km_key_update_key_cache(wlc_key_t *key, km_key_cache_t *kc)
{
	/* note: can't use KEY_VALID - called during attach */
	KM_DBG_ASSERT((key != NULL) && KEY_MAGIC_VALID(key));
	KM_DBG_ASSERT(kc != NULL);

	memset(kc, 0, sizeof(*kc));
	kc->hw_idx = key->hw_idx;
	kc->key = key;
	kc->key_info = &key->info;
}
#endif /* PKTC || PKTC_DONGLE */


int km_key_set_key_id(wlc_key_t *key, wlc_key_id_t key_id)
{
	int err = BCME_OK;

	KM_DBG_ASSERT(KEY_VALID(key));

	/* allow only pairwise key id to be set to support dynamic 802.1x/wep */
	if (!WLC_KEY_ID_IS_DEFAULT(key_id) ||
			!WLC_KEY_IS_PAIRWISE(&key->info)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	key->info.key_id = key_id;
done:
	return err;
}
