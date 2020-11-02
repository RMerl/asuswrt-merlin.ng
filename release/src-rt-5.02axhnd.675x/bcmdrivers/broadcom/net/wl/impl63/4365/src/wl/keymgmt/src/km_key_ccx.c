/*
 * Implementation of wlc_key algo 'wep' - ccx specific
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_ccx.c 431766 2013-10-24 16:26:34Z $
 */

#if defined(BCMCCX) || defined(BCMEXTCCX)
#include "km_key_wep_pvt.h"

int
km_key_wep_rx_ccx_msdu(wlc_key_t *key, void *pkt, struct ether_header *hdr,
    uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi)
{
	int err = BCME_OK;
	wep_key_t *wep_key;
	uint32 mic;
	uint32 rx_seq;
	int mmh_encap;
	bool rx_seq_check;

	if (!(key->info.flags & WLC_KEY_FLAG_CKIP_MIC))
		goto done;

	if (body_len < CKIP_LLC_SNAP_LEN) {
		WLCNTINCR(KEY_CNT(key)->rxrunt);
		err = BCME_BADLEN;
		goto done;
	}

	wep_key = (wep_key_t *)key->algo_impl.ctx;

	/* check CKIP MIC (pbody start from CKIP_LLC_SNAP) */
	mmh_encap = wsec_ckip_mic_check(wep_key->key, hdr->ether_dhost, hdr->ether_shost,
		body, body_len, (uint8 *)&mic);

	if (!mmh_encap) { /* mic check failed */
		wlc_key_t *prev_key;
		wlc_key_id_t key_id;
		wlc_bsscfg_t *bsscfg;
		wlc_key_info_t prev_key_info;
		wep_key_t *prev_wep_key;

		KEY_LOG(("wl%d: %s: ckip mic check with key idx %d failed. "
			"expected 0x%08x received 0x%08x \n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx, ntoh32(mic),
			ntoh32_ua(body + CKIP_LLC_SNAP_LEN)));

		if (!(key->info.flags & WLC_KEY_FLAG_GROUP)) {
			err = BCME_MICERR;
			goto done;
		}

		key_id = key->info.key_id ^ 1; /* previous key id */
		bsscfg = wlc_keymgmt_get_bsscfg(KEY_KM(key), key->info.key_idx);
		KM_DBG_ASSERT(bsscfg != NULL);

		prev_key = wlc_keymgmt_get_bss_key(KEY_KM(key), bsscfg, key_id, &prev_key_info);
		if (prev_key_info.algo != CRYPTO_ALGO_WEP1 &&
			prev_key_info.algo != CRYPTO_ALGO_WEP128) {
			err = BCME_MICERR;
			goto done;
		}

		if (!(prev_key_info.flags & WLC_KEY_FLAG_CKIP_MIC)) {
			err = BCME_MICERR;
			goto done;
		}

		KM_DBG_ASSERT(prev_key != NULL);

		prev_wep_key = (wep_key_t *)prev_key->algo_impl.ctx;

		KEY_LOG(("wl%d: %s: ckip mic check with prev key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, prev_key_info.key_idx));

		mmh_encap = wsec_ckip_mic_check(prev_wep_key->key,
			hdr->ether_dhost, hdr->ether_shost, body, body_len, (uint8 *)&mic);

		if (!mmh_encap) {
			KEY_LOG(("wl%d: %s: ckip mic check with prev key failed. expected 0x%08x\n",
				KEY_WLUNIT(key), __FUNCTION__, ntoh32(mic)));
			err = BCME_MICERR;
			goto done;
		}

		key = prev_key;
		wep_key = prev_wep_key;
	}

	if (mmh_encap == -1) {
		KEY_LOG(("wl%d: %s: iapp frame, no ckip mic\n", KEY_WLUNIT(key), __FUNCTION__));
		goto done; /* okay */
	}

	/* check ckip seq */
	if (body_len < (CKIP_LLC_SNAP_LEN + CKIP_MIC_SIZE)) {
		WLCNTINCR(KEY_CNT(key)->rxrunt);
		err = BCME_BADLEN;
		goto done;
	}

	rx_seq = ntoh32_ua(body + CKIP_LLC_SNAP_LEN + CKIP_MIC_SIZE);
	if (ETHER_ISMULTI(hdr->ether_dhost))
		rx_seq_check = bcm_ckip_rxseq_check(rx_seq, &wep_key->ckip_state.mcrx_seq_base,
			&wep_key->ckip_state.mcrx_seq_bitmap);
	else
		rx_seq_check = bcm_ckip_rxseq_check(rx_seq, &wep_key->ckip_state.rx_seq_base,
			&wep_key->ckip_state.rx_seq_bitmap);

	if (!rx_seq_check) {
		KEY_LOG(("wl%d: %s: seq check failed with rx seq %d\n",
			KEY_WLUNIT(key), __FUNCTION__, rx_seq));
		err = BCME_REPLAY;
		goto done;
	}

done:
	KEY_LOG(("wl%d: %s: exit with status %d key idx %d\n",
		KEY_WLUNIT(key), __FUNCTION__, err, key->info.key_idx));
	return err;
}

/*
 * CKIP packet hdr conversion for tx
 * 1) original pkt: ethernet					     | mic-->
 * ----------------------------------------------------------------------------------
 *                                                     |  DA  |  SA  | T |  Data... |
 * ----------------------------------------------------------------------------------
 * converted to 802.3					  6	 6     2
 * ----------------------------------------------------------------------------------
 * |                                    |  DA  |  SA  | L | LLC/SNAP | T |  Data... |
 * ----------------------------------------------------------------------------------
 * CKIP					   6      6     2      6       2
 * ----------------------------------------------------------------------------------
 * |                   |  DA  |  SA  | L | LLC/SNAP_CKIP | MIC | SEQ | T |  Data... |
 * ----------------------------------------------------------------------------------
 * 2) original pkt: 802.3			8          4      4    2
 * ----------------------------------------------------------------------------------
 * |                                    |  DA  |  SA  | L | LLC/SNAP | T |  Data... |
 * ----------------------------------------------------------------------------------
 * CKIP					   6	  6	2      6       2
 *                                                    | mic-->
 * ----------------------------------------------------------------------------------
 * |    |  DA  |  SA  | L | LLC/SNAP_CKIP | MIC | SEQ | L | LLC_SNAP | T |  Data... |
 * ----------------------------------------------------------------------------------
 *				8	     4	   4	2      6       2
 */
int
km_key_wep_tx_ccx_msdu(wlc_key_t *key, void *pkt, struct ether_header *hdr,
    uint8 *body, int body_len, size_t frag_length, uint8 prio)
{
	wep_key_t *wep_key;
	uint next_body_len = 0;
	uint8 *next_body = NULL;
	uint8 ckip_mic[4];
	uint32 tx_seq;
	struct ether_header *new_hdr;
	uint16 plen;
	char *ckip_hdr;
	osl_t *osh;
	void *next_data;
	int err = BCME_OK;

	if (!(key->info.flags & WLC_KEY_FLAG_CKIP_MIC))
		goto done;

	wep_key = (wep_key_t *)key->algo_impl.ctx;
	osh = KEY_OSH(key);

	/* for ckip body, include ether type/len */
	body -= 2;
	body_len += 2;

	/* update sequence number, saved in BIG Endian format */
	wep_key->ckip_state.tx_seq += 2;
	tx_seq = hton32(wep_key->ckip_state.tx_seq);

	/* support at most 2 chained buffers */
	if ((next_data = PKTNEXT(osh, pkt)) != NULL) {
		next_body = PKTDATA(osh, next_data);
		next_body_len = PKTLEN(osh, next_data);
		KM_ASSERT(!PKTNEXT(osh, next_data));
	}
	wsec_ckip_mic_compute(wep_key->key, hdr->ether_dhost, hdr->ether_shost,
		(uint8 *)&tx_seq, body, body_len, next_body, next_body_len, ckip_mic);

	/*
	 * CKIP MIC encapsulation
	 * headroom for MIC, SEQ and ckip_llc_snap has been reserved in wlc_sendpkt()
	 * for ethernet, overwrite normal llc_snap with CKIP one, keep ether_type field
	 * for 802.3, in front of normal 802.3 LLC_SNAP, pre-APPEND CKIP one, keep ether_len
	 * field
	 */
	if (WLPKTTAG(pkt)->flags & WLF_NON8023)
		plen = 2 + CKIP_MIC_SIZE + CKIP_SEQ_SIZE;
	else
		plen = 2 + CKIP_MIC_SIZE + CKIP_SEQ_SIZE + CKIP_LLC_SNAP_LEN;

	KM_ASSERT(PKTHEADROOM(osh, pkt) >= plen);

	new_hdr = (struct ether_header *)PKTPUSH(osh, pkt, plen);

	/* 802.3 MAC header, no overlap */
	memcpy((char*)new_hdr->ether_dhost, (char*)hdr->ether_dhost, ETHER_ADDR_LEN);
	memcpy((char*)new_hdr->ether_shost, (char*)hdr->ether_shost, ETHER_ADDR_LEN);
	plen = (uint16)pkttotlen(osh, pkt) - ETHER_HDR_LEN;
	new_hdr->ether_type = hton16(plen);

	/* filled in ckip_llc_snap, mic and seq */
	ckip_hdr = (char *)&new_hdr[1];
	memcpy(ckip_hdr, ckip_llc_snap, CKIP_LLC_SNAP_LEN);
	memcpy(ckip_hdr + CKIP_LLC_SNAP_LEN, ckip_mic, CKIP_MIC_SIZE);
	memcpy(ckip_hdr + CKIP_LLC_SNAP_LEN + CKIP_MIC_SIZE, &tx_seq, CKIP_SEQ_SIZE);

done:
	KEY_LOG(("wl%d: %s: exit with status %d key idx %d\n",
		KEY_WLUNIT(key), __FUNCTION__, err, key->info.key_idx));
	return err;
}

void
km_key_wep_ccx_dump(wep_key_ckip_state_t *state, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "\tckip state: ");
	bcm_bprintf(b, "\t\t tx seq %lu\n", state->tx_seq);
	bcm_bprintf(b, "\t\t rx seq base %lu\n", state->rx_seq_base);
	bcm_bprintf(b, "\t\t rx seq bitmap %lu\n", state->rx_seq_bitmap);
	bcm_bprintf(b, "\t\t rx seq multicast base %lu\n", state->mcrx_seq_base);
	bcm_bprintf(b, "\t\t rx seq multicast bitmap %lu\n", state->mcrx_seq_bitmap);
	bcm_bprintf(b, "\n");
}

#endif /* BCMCCX || BCMEXTCCX */
