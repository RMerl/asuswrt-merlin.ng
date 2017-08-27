/*
 * Implementation of wlc_key algo 'aes' for multicast mgmt frames
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_aes_mcmfp.c 466975 2014-04-02 03:21:28Z $
 */

#ifdef MFP

#include "km_key_aes_pvt.h"

#define BIP_AAD_SIZE (sizeof(uint16)+3*ETHER_ADDR_LEN)

/* internal interface, handles aad overlap with header */
static void aes_bip_calc_params(wlc_key_t *key,
	const struct dot11_management_header *hdr,
	uint8 *nonce, size_t *nonce_len, uint8 *out_aad, size_t aad_size, uint8 *seq)
{
	uint8 *tmp;
	uint16 fc;
	size_t i;
	uint8 aad[BIP_AAD_SIZE];

	KM_DBG_ASSERT(out_aad != NULL && aad_size == BIP_AAD_SIZE);

	fc = ltoh16(hdr->fc);
	fc &=  ~(FC_RETRY | FC_PM | FC_MOREDATA);

	tmp = aad;
	*tmp++ = fc & 0xff;
	*tmp++ = fc >> 8 & 0xff;
	memcpy(tmp, (uint8 *)&hdr->da, ETHER_ADDR_LEN);
	tmp += ETHER_ADDR_LEN;
	memcpy(tmp, (uint8 *)&hdr->sa, ETHER_ADDR_LEN);
	tmp += ETHER_ADDR_LEN;
	memcpy(tmp, (uint8 *)&hdr->bssid, ETHER_ADDR_LEN);
	tmp += ETHER_ADDR_LEN;

	if (key->info.algo == CRYPTO_ALGO_BIP_GMAC ||
		key->info.algo == CRYPTO_ALGO_BIP_GMAC256) {
		/* populate nonce */
		KM_DBG_ASSERT(nonce != NULL && nonce_len != NULL);
		memcpy(nonce, &hdr->sa, ETHER_ADDR_LEN);
		tmp =  &nonce[ETHER_ADDR_LEN];
		for (i = AES_KEY_SEQ_SIZE; i > 0; --i)
			*(tmp++) = seq[i - 1];
		*nonce_len = ETHER_ADDR_LEN + AES_KEY_SEQ_SIZE;
	} else {
		if (nonce_len != NULL)
			*nonce_len = 0;
	}

	memcpy(out_aad, aad, BIP_AAD_SIZE);
}

static int
aes_bip_cmac_mic(wlc_key_t *key, void *pkt, const struct dot11_management_header *hdr,
	uint8 *body, int body_len, uint8 *mic)
{
	uint8 *data;
	size_t data_len;
	uint8 tmp[BIP_AAD_SIZE];
	aes_igtk_t *aes_igtk;


	data = body - BIP_AAD_SIZE;
	KM_ASSERT(data >= (uint8 *)hdr);

	/* save data and generate aad */
	memcpy(tmp, data, BIP_AAD_SIZE);

	data_len = BIP_AAD_SIZE;
	aes_bip_calc_params(key, hdr, NULL, NULL, data, BIP_AAD_SIZE, NULL);

	data_len += body_len - key->info.icv_len;

	memset(&data[data_len], 0, key->info.icv_len); /* zero mic */
	data_len += key->info.icv_len;

	aes_igtk = (aes_igtk_t *)key->algo_impl.ctx;

	aes_cmac_calc(data, data_len, aes_igtk->key, key->info.key_len, mic, key->info.icv_len);

	/* restore hdr */
	memcpy(data, tmp, BIP_AAD_SIZE);

	return BCME_OK;
}

#ifdef BCMGCMP
static int
aes_bip_gmac_mic(wlc_key_t *key, void *pkt, const struct dot11_management_header *hdr,
	uint8 *body, int body_len, uint8 *seq, uint8 *mic)
{
	uint8 *data;
	size_t data_len;
	uint8 tmp[BIP_AAD_SIZE];
	uint8 nonce[ETHER_ADDR_LEN + AES_KEY_SEQ_SIZE];
	size_t nonce_len;
	aes_igtk_t *aes_igtk;

	data = body - BIP_AAD_SIZE;
	KM_ASSERT(data >= (uint8 *)hdr);

	/* save data and generate aad */
	memcpy(tmp, data, BIP_AAD_SIZE);

	data_len = BIP_AAD_SIZE;
	aes_bip_calc_params(key, hdr, nonce, &nonce_len, data, BIP_AAD_SIZE, seq);

	data_len += body_len - key->info.icv_len;

	memset(&data[data_len], 0, key->info.icv_len); /* zero mic */
	data_len += key->info.icv_len;

	aes_igtk = (aes_igtk_t *)key->algo_impl.ctx;
	aes_gcm_mac(aes_igtk->key, key->info.key_len, nonce, nonce_len,
		data, data_len, NULL, 0, mic, key->info.icv_len);

	/* restore hdr */
	memcpy(data, tmp, BIP_AAD_SIZE);
	return BCME_OK;
}
#endif /* BCMGCMP */

static int
aes_bip_mic(wlc_key_t *key, void *pkt, struct dot11_management_header *hdr,
	uint8 *body, int body_len, mmic_ie_t *ie)
{
	int err;

	switch (key->info.algo) {
	case CRYPTO_ALGO_BIP:
	case CRYPTO_ALGO_BIP_CMAC256:
		err = aes_bip_cmac_mic(key, pkt, hdr, body, body_len, ie->mic);
		break;
#ifdef BCMGCMP
	case CRYPTO_ALGO_BIP_GMAC:
	case CRYPTO_ALGO_BIP_GMAC256:
		err = aes_bip_gmac_mic(key, pkt, hdr, body, body_len, ie->ipn, ie->mic);
		break;
#endif /* BCMGCMP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}


/* public interface */
int
km_key_aes_rx_mmpdu_mcmfp(wlc_key_t *key, void *pkt, const struct dot11_header *hdr,
	uint8 *body, int body_len,  const key_hw_rx_info_t *hw_rxi)
{
	struct dot11_management_header *mgmt_hdr;
	uint8 rcv_mic[AES_BLOCK_SZ];
	int err = BCME_OK;
	aes_igtk_t *aes_igtk;
	mmic_ie_t *ie;
	int ie_len;
	size_t icv_len;
	STATIC_ASSERT(sizeof(ie->ipn) == AES_KEY_SEQ_SIZE);

	KM_ASSERT(WLC_KEY_IS_MGMT_GROUP(&key->info));

	ie_len = WLC_KEY_MMIC_IE_LEN(&key->info); /* incl TLV hdr */
	KM_ASSERT(ie_len <= sizeof(mmic_ie_t));

	mgmt_hdr = (struct dot11_management_header *)hdr;
	aes_igtk = (aes_igtk_t *)key->algo_impl.ctx;

	if (body_len < ie_len) {
		err = BCME_BADLEN;
		goto done;
	}

	ie = (mmic_ie_t *)(body + body_len - ie_len);
	if (ie->id != DOT11_MNG_MMIE_ID) {
		err = BCME_IE_NOTFOUND;
		goto done;
	}

	if (ie->len != (ie_len - TLV_HDR_LEN)) {
		err = BCME_BADLEN;
		goto done;
	}

	if (ltoh16_ua(&ie->key_id) != (uint16)key->info.key_id) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	/* replay check */
	if (km_is_replay(KEY_KM(key), &key->info, 0, aes_igtk->seq,
		ie->ipn, AES_KEY_SEQ_SIZE)) {
		err = BCME_REPLAY;
		goto done;
	}

	if (hw_rxi->attempted) {
		err = hw_rxi->status;
		goto done;
	}

	/* save received mic, as mic computation zeros the mic in frame */
	icv_len = key->info.icv_len;
	memcpy(rcv_mic, ie->mic, icv_len);
	err = aes_bip_mic(key, pkt, mgmt_hdr, body, body_len, ie);
	if (err == BCME_OK) {
		/* check mic and update (tx) seq on success */
		if (memcmp(ie->mic, rcv_mic, icv_len)) {
			KEY_LOG(("wl%d: %s: mic mismatch for key idx %d\n", KEY_WLUNIT(key),
				__FUNCTION__, key->info.key_idx));
			KEY_LOG_DUMP(prhex("expected mic: ", ie->mic, icv_len));
			KEY_LOG_DUMP(prhex("received mic: ", rcv_mic, icv_len));

			memcpy(ie->mic, rcv_mic, icv_len); /* restore mic in pkt */
			err = BCME_DECERR;
		} else  {
			memcpy(aes_igtk->seq, ie->ipn, AES_KEY_SEQ_SIZE);
			/* leave the ie in the pkt */
		}
	}

done:
	return err;
}

int
km_key_aes_tx_mmpdu_mcmfp(wlc_key_t *key, void *pkt, const struct dot11_header *hdr,
	uint8 *body, int body_len, wlc_txd_t *txd)
{
	struct dot11_management_header *mgmt_hdr;
	int err = BCME_OK;
	mmic_ie_t *ie;
	aes_igtk_t *aes_igtk;
	size_t ie_len;
	STATIC_ASSERT(sizeof(ie->ipn) == AES_KEY_SEQ_SIZE);

	KM_ASSERT(WLC_KEY_IS_MGMT_GROUP(&key->info));
	KM_DBG_ASSERT(key->info.icv_len <= AES_BLOCK_SZ);

	ie_len = WLC_KEY_MMIC_IE_LEN(&key->info); /* incl TLV hdr */
	KM_ASSERT(ie_len <= sizeof(mmic_ie_t));

	aes_igtk = (aes_igtk_t *)key->algo_impl.ctx;
	mgmt_hdr = (struct dot11_management_header *)hdr;

	/* get start of mmic ie; note that mic (icv) length depends on algo */
	ie = (mmic_ie_t *)(body + body_len);
	ie->id = DOT11_MNG_MMIE_ID;
	ie->len = ie_len - TLV_HDR_LEN;
	htol16_ua_store((uint16)key->info.key_id, (uint8 *)&ie->key_id);

#ifdef  BCMDBG
	if (key->info.flags & WLC_KEY_FLAG_GEN_REPLAY) {
		key->info.flags &= ~WLC_KEY_FLAG_GEN_REPLAY;
	} else
#else
	{
		KEY_SEQ_INCR(aes_igtk->seq, AES_KEY_SEQ_SIZE);
	}
#endif
	memcpy(ie->ipn, aes_igtk->seq, AES_KEY_SEQ_SIZE);
	memset(ie->mic, 0, key->info.icv_len);

	if (!WLC_KEY_IN_HW(&key->info) || txd == NULL) {
		size_t pkt_len;
		body_len += ie_len;
		err = aes_bip_mic(key, pkt, mgmt_hdr, body, body_len, ie);
		if (err != BCME_OK)
			goto done;

		/* adjust pkt len to include ie */
		pkt_len = (body - (uint8 *)mgmt_hdr) + body_len;
		PKTSETLEN(KEY_OSH(key), pkt, pkt_len);
	}

#ifdef BCMDBG
	if (key->info.flags & (WLC_KEY_FLAG_GEN_ICV_ERR|WLC_KEY_FLAG_GEN_MIC_ERR)) {
		ie->mic[0] =  ~ie->mic[0];
		key->info.flags &= ~(WLC_KEY_FLAG_GEN_ICV_ERR|WLC_KEY_FLAG_GEN_MIC_ERR);
	}
#endif

done:
	return err;
}

#endif /* MFP */
