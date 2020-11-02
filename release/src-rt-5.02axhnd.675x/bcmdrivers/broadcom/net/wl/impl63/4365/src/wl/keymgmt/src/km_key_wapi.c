/*
 * Implementation of wlc_key algo 'wapi'
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_wapi.c 431766 2013-10-24 16:26:34Z $
 */

#ifdef BCMWAPI_WPI

#include "km_key_pvt.h"

#include <bcmcrypto/sms4.h>
#include <wlc_wapi.h>
#include <proto/wpa.h>

/* internal interface */

#define WAPI_KEY_SIZE SMS4_KEY_LEN
#define WAPI_IKEY_SIZE SMS4_WPI_CBC_MAC_LEN
#define WAPI_KEY_IV_SIZE SMS4_WPI_IV_LEN
#define WAPI_KEY_ICV_SIZE SMS4_WPI_CBC_MAC_LEN
#define WAPI_KEY_SEQ_SIZE SMS4_WPI_PN_LEN

#define WAPI_KEY_VALID(_key) (((_key)->info.algo == CRYPTO_ALGO_SMS4) &&\
		((_key)->info.key_len == WAPI_KEY_SIZE) &&\
		((_key)->info.iv_len == WAPI_KEY_IV_SIZE) &&\
		((_key)->info.icv_len == WAPI_KEY_ICV_SIZE))

/* context data type for wapi */
struct wapi_key {
	uint8 key[WAPI_KEY_SIZE];
	uint8 ikey[WAPI_IKEY_SIZE];
	uint8 tx_seq[WAPI_KEY_SEQ_SIZE];
	uint8 rx_seq[WAPI_KEY_SEQ_SIZE];
	uint32 tx_count;
	uint32 rx_count;
};

typedef struct wapi_key wapi_key_t;

static void
wapi_tx_iv_init(wlc_key_t *key, wapi_key_t *wapi_key, wlc_bsscfg_t *bsscfg)
{
	int i;

	memset(wapi_key->tx_seq, 0, sizeof(wapi_key->tx_seq));

	/* AP is AE and STA is ASUE */
	for (i = 0; i < WAPI_KEY_SEQ_SIZE;) {
		wapi_key->tx_seq [i++] = 0x36;
		wapi_key->tx_seq[i++] = 0x5C;
	}

	if (BSSCFG_AP(bsscfg))
		wapi_key->tx_seq[0] = 0x37;
	else if (!bsscfg->BSS) {
		int val;
		/* Compare the mac addresses to figure which one is AE which is ASE */
		val = bcm_cmp_bytes((uchar *)&key->info.addr, (uchar *)&bsscfg->cur_etheraddr,
			ETHER_ADDR_LEN);
		if (val < 0)
			wapi_key->tx_seq[0] = 0x37;
	}
}

static void
wapi_rx_iv_init(wlc_key_t *key, wapi_key_t *wapi_key, wlc_bsscfg_t *bsscfg)
{
	int i;

	memset(wapi_key->rx_seq, 0, sizeof(wapi_key->rx_seq));

	/* AP is AE and STA is ASUE */
	for (i = 0; i < WAPI_KEY_SEQ_SIZE;) {
		wapi_key->rx_seq[i++] = 0x36;
		wapi_key->rx_seq[i++] = 0x5C;
	}

	if (!BSSCFG_AP(bsscfg)) {
		if (bsscfg->BSS)
			wapi_key->rx_seq[0] = 0x37;
		else {
			int val;
			/* Compare the mac addresses to figure which one is AE which is ASE */
			val = bcm_cmp_bytes((uchar *)&key->info.addr,
				(uchar *)&bsscfg->cur_etheraddr, ETHER_ADDR_LEN);
			if (val >= 0)
				wapi_key->rx_seq[0] = 0x37;
		}
	}
}

static int
wapi_destroy(wlc_key_t *key)
{
	KM_DBG_ASSERT(WAPI_KEY_VALID(key));

	if (key->algo_impl.ctx != NULL) {
		MFREE(KEY_OSH(key), key->algo_impl.ctx, sizeof(wapi_key_t));
	}

	key->algo_impl.cb = NULL;
	key->algo_impl.ctx = NULL;
	key->info.key_len = 0;
	key->info.iv_len = 0;
	key->info.icv_len = 0;
	return BCME_OK;
}

static int
wapi_get_data(wlc_key_t *key, uint8 *data, size_t data_size,
	size_t *data_len, key_data_type_t data_type, int ins, bool tx)
{
	wapi_key_t *wapi_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(WAPI_KEY_VALID(key));
	KM_ASSERT(!WLC_KEY_IS_MGMT_GROUP(&key->info));

	wapi_key = (wapi_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		if (data_len != NULL) {
			*data_len = key->info.key_len;
		}

		if (data_size < key->info.key_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(data, wapi_key->key, key->info.key_len);
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (data_len != NULL) {
			*data_len = WAPI_KEY_SEQ_SIZE;
		}

		if (data_size < WAPI_KEY_SEQ_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (ins != 0) {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(data, (tx ? wapi_key->tx_seq : wapi_key->rx_seq), WAPI_KEY_SEQ_SIZE);
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY:
		if (data_len != NULL) {
			*data_len = WAPI_IKEY_SIZE;
		}

		if (data_size < WAPI_IKEY_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		memcpy(data, wapi_key->ikey, WAPI_IKEY_SIZE);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wapi_set_data(wlc_key_t *key, const uint8 *data,
    size_t data_len, key_data_type_t data_type, int ins, bool tx)
{
	wapi_key_t *wapi_key;
	int err = BCME_OK;
	wlc_bsscfg_t *bsscfg;

	KM_DBG_ASSERT(WAPI_KEY_VALID(key));

	/* no wapi for mgmt group keys */
	if (key->info.flags & WLC_KEY_FLAG_MGMT_GROUP) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	bsscfg = wlc_keymgmt_get_bsscfg(KEY_KM(key), key->info.key_idx);
	KM_DBG_ASSERT(bsscfg != NULL);

	wapi_key = (wapi_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		/* key and ikey must be set together */
		if (data_len != (key->info.key_len + WAPI_IKEY_SIZE)) {
			if (!data_len) {
				memset(wapi_key, 0, sizeof(wapi_key_t));
				break;
			}
			if (data_len < (size_t)(key->info.key_len + WAPI_IKEY_SIZE)) {
				err = BCME_BADLEN;
				break;
			}
			data_len = key->info.key_len + WAPI_IKEY_SIZE;
		}

		if (!data) {
			err = BCME_BADARG;
			break;
		}

		memcpy(wapi_key->key, data, key->info.key_len);
		memcpy(wapi_key->ikey, &data[key->info.key_len], WAPI_IKEY_SIZE);
		/* re-init iv, because key changed */
		wapi_tx_iv_init(key, wapi_key, bsscfg);
		wapi_rx_iv_init(key, wapi_key, bsscfg);
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (ins != 0) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (!data_len) {
			if (tx)
				wapi_tx_iv_init(key, wapi_key, bsscfg);
			else
				wapi_rx_iv_init(key, wapi_key, bsscfg);
		} else  if (!data || (data_len < WAPI_KEY_SEQ_SIZE)) { /* allows truncation */
			err = BCME_BADARG;
			break;
		} else {
			memcpy((tx ? wapi_key->tx_seq : wapi_key->rx_seq), data, WAPI_KEY_SEQ_SIZE);
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}

static int
wapi_rx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi)
{
	wapi_key_t *wapi_key;
	int err = BCME_OK;
	struct wpi_iv *rx_iv;
	uint16 fc;
	size_t pkt_len;

	KM_ASSERT(WAPI_KEY_VALID(key));
	KM_DBG_ASSERT(hw_rxi != NULL);

	wapi_key = (wapi_key_t *)key->algo_impl.ctx;
	rx_iv = (struct wpi_iv *)body;

	fc =  ltoh16(hdr->fc);
	if ((FC_TYPE(fc) == FC_TYPE_MNG) && ETHER_ISMULTI(&hdr->a1)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	if (km_is_replay(KEY_KM(key), &key->info, 0,
		wapi_key->rx_seq, rx_iv->PN, WAPI_KEY_SEQ_SIZE)) {
		err = BCME_REPLAY;
		goto done;
	}

	/* for unicast sequence increments by 2; make sure sender complies */
	if (!ETHER_ISMULTI(&hdr->a1) && (wapi_key->rx_seq[0] ^ rx_iv->PN[0])) {
		err = BCME_DECERR;
		goto done;
	}

	if (hw_rxi->attempted) {
		err = hw_rxi->status;
		goto done;
	}

	pkt_len = (body + body_len) - (uint8 *)hdr;
	if (sms4_wpi_pkt_decrypt(wapi_key->key, wapi_key->ikey, pkt_len,
		(uint8 *)hdr) != SMS4_WPI_SUCCESS) {
		err = BCME_DECERR;
		goto done;
	}

	wapi_key->rx_count++;

#ifdef BCMWAPI_WAI
	if (!(wapi_key->rx_count % WAPI_USK_REKEY_COUNT)) {
		scb_t *scb;
		wlc_bsscfg_t *bsscfg;

		scb = WLPKTTAGSCBGET(pkt);
		KM_ASSERT(scb != NULL);

		bsscfg = SCB_BSSCFG(scb);
		KM_DBG_ASSERT(bsscfg != NULL);

		wlc_wapi_station_event(KEY_WLC(key)->wapi, bsscfg, &key->info.addr,
			NULL, NULL, WAPI_UNICAST_REKEY);
	}
#endif /* BCMWAPI_WAI */

done:
	if (err == BCME_OK) {
		memcpy(wapi_key->rx_seq, rx_iv->PN, WAPI_KEY_SEQ_SIZE);

		/* if there was a previous key, delete it now */
		if (WLC_KEY_SMS4_HAS_PREV_KEY(&key->info)) {
			wlc_key_t *prev_key;
			scb_t *scb;

			scb = WLPKTTAGSCBGET(pkt);
			KM_ASSERT(scb != NULL);

			prev_key = wlc_keymgmt_get_scb_key(KEY_KM(key), scb,
				(key->info.key_id ^ 0x1), WLC_KEY_FLAG_NONE, NULL);
			(void)wlc_key_set_data(prev_key, CRYPTO_ALGO_NONE, NULL, 0);
			key->info.flags &= ~WLC_KEY_FLAG_WAPI_HAS_PREV_KEY;
		}
	}

	return err;
}

static int
wapi_tx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, wlc_txd_t *txd)
{
	int err = BCME_OK;
	wapi_key_t *wapi_key;
	uint16 fc;
	struct wpi_iv *tx_iv;
	size_t pkt_len;

	KM_DBG_ASSERT(WAPI_KEY_VALID(key));
	fc =  ltoh16(hdr->fc);
	if ((FC_TYPE(fc) == FC_TYPE_MNG) && ETHER_ISMULTI(&hdr->a1)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	wapi_key = (wapi_key_t *)key->algo_impl.ctx;
#ifdef BCMDBG
	if (key->info.flags & WLC_KEY_FLAG_GEN_REPLAY) {
		key->info.flags &= ~WLC_KEY_FLAG_GEN_REPLAY;
	} else
#endif /* BCMDBG */
	{
		/* for unicast seq increments by 2 */
		KEY_SEQ_INCR(wapi_key->tx_seq, WAPI_KEY_SEQ_SIZE);
		if (!ETHER_ISMULTI(&hdr->a1))
			KEY_SEQ_INCR(wapi_key->tx_seq, WAPI_KEY_SEQ_SIZE);
	}

	tx_iv = (struct wpi_iv *)body;
	tx_iv->key_idx = key->info.key_id;
	tx_iv->reserved = 0;
	memcpy(tx_iv->PN, wapi_key->tx_seq, WAPI_KEY_SEQ_SIZE);

	if (WLC_KEY_IN_HW(&key->info) && txd != NULL) {
		if (!KEY_USE_AC_TXD(key)) {
			d11txh_t *txh = (d11txh_t *)&txd->d11txh;
			memcpy(txh->IV, wapi_key->tx_seq, WAPI_KEY_SEQ_SIZE);
		}
		goto done;
	}

	pkt_len = (body + body_len) - (uint8 *)hdr;
	if (sms4_wpi_pkt_encrypt(wapi_key->key, wapi_key->ikey, pkt_len,
		(uint8 *)hdr) != SMS4_WPI_SUCCESS) {
		err = BCME_ENCERR;
		goto done;
	}

done:
	return err;
}

/* key rotation support */
wlc_key_t*
km_key_wapi_rotate_key(wlc_key_t *cur_key, wlc_key_t *prev_key, wlc_key_info_t *key_info)
{
	KM_DBG_ASSERT(cur_key && prev_key);

	/* swap all but the key idx and hw idx not owned by us */
	KM_SWAP(wlc_key_t, *cur_key, *prev_key);
	KM_SWAP(key_index_t, cur_key->info.key_idx, prev_key->info.key_idx);
	KM_SWAP(wlc_key_hw_index_t, cur_key->hw_idx, prev_key->hw_idx);

	/* expiration */
	cur_key->exp = KEY_PUB(cur_key)->now + SMS4_OLD_KEY_MAXVALIDTIME;

	/* update prev key flag */
	cur_key->info.flags &= ~WLC_KEY_FLAG_WAPI_HAS_PREV_KEY;
	prev_key->info.flags |= WLC_KEY_FLAG_WAPI_HAS_PREV_KEY;

	if (key_info)
		*key_info = prev_key->info;
	return prev_key;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wapi_dump(const wlc_key_t *key, struct bcmstrbuf *b)
{
	wapi_key_t *wapi_key;
	size_t i;

	KM_DBG_ASSERT(WAPI_KEY_VALID(key));

	wapi_key = (wapi_key_t *)key->algo_impl.ctx;
	bcm_bprintf(b, "\twapi key: ");
	for (i = 0; i < WAPI_KEY_SIZE; ++i)
		bcm_bprintf(b, "%02x", wapi_key->key[i]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\twapi tx_seq: 0x");
	for (i = WAPI_KEY_SEQ_SIZE; i > 0; --i)
		bcm_bprintf(b, "%02x", wapi_key->tx_seq[i-1]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\twapi rx_seq: ");
	for (i = WAPI_KEY_SEQ_SIZE; i > 0; --i)
		bcm_bprintf(b, "%02x", wapi_key->rx_seq[i]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\twapi counts - tx: %d rx: %d\n",
		wapi_key->tx_count, wapi_key->rx_count);
	return BCME_OK;
}

#define WAPI_DUMP wapi_dump
#else
#define WAPI_DUMP NULL
#endif /* BCMDBG || BCMDBG_DUMP */

static const key_algo_callbacks_t key_wapi_callbacks = {
    wapi_destroy,	/* destroy */
    wapi_get_data,	/* get data */
    wapi_set_data,	/* set data */
    wapi_rx_mpdu,	/* rx mpdu */
    NULL,			/* rx msdu */
    wapi_tx_mpdu,	/* tx mpdu */
    NULL,			/* tx msdu */
    WAPI_DUMP		/* dump */
};

/* public interface */
int
km_key_wapi_init(wlc_key_t *key)
{
	KM_DBG_ASSERT(key->info.algo == CRYPTO_ALGO_SMS4);

	key->info.key_len = WAPI_KEY_SIZE;
	key->info.iv_len = WAPI_KEY_IV_SIZE;
	key->info.icv_len = WAPI_KEY_ICV_SIZE;
	key->algo_impl.cb = &key_wapi_callbacks;
	key->algo_impl.ctx = MALLOCZ(KEY_OSH(key), sizeof(wapi_key_t));
	return (key->algo_impl.ctx != NULL) ? BCME_OK : BCME_NOMEM;
}

#endif /* BCMWAPI_WPI */
