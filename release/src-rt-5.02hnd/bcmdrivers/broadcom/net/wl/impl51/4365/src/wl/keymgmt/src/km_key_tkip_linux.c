/*
 * Implementation of wlc_key algo 'tkip' using linux crypto
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_tkip_linux.c 431766 2013-10-24 16:26:34Z $
 */

#ifdef LINUX_CRYPTO

#include "km_key_pvt.h"
#include <wl_export.h>

/* internal interface */
#define TKIP_KEY_TK_SIZE (TKIP_KEY_SIZE >> 1)
#define TKIP_KEY_MIC_KEY_SIZE (TKIP_KEY_SIZE >> 2)
#define TKIP_KEY_IV_SIZE DOT11_IV_TKIP_LEN		/* extended iv */
#define TKIP_KEY_ICV_SIZE DOT11_ICV_LEN			/* wep icv */
#define TKIP_KEY_SEQ_SIZE 6						/* 48 bit pn */

#define TKIP_KEY_VALID(_key) (((_key)->info.algo == CRYPTO_ALGO_TKIP) &&\
		((_key)->info.key_len == TKIP_KEY_SIZE) &&\
		((_key)->info.iv_len == TKIP_KEY_IV_SIZE) &&\
		((_key)->info.icv_len == TKIP_KEY_ICV_SIZE))

#define TKIP_KEY_VALID_INS(_key, _tx, _ins) (((_tx) != TRUE) &&\
	((_ins) < KEY_NUM_RX_SEQ(_key)))

#define TKIP_KEY_SEQ(_key, _tx, _ins) (&(_key)->rx_seq[_ins][0])

#define TKIP_BODY_WEP_SEED_OKAY(_body) ((_body)[1] == (((_body)[0] | 0x20) | 0x7f))

/* seq from body */
#define TKIP_KEY_SEQ_FROM_BODY(_seq, _body) {\
	(_seq)[0] = (_body)[2]; \
	(_seq)[1] = (_body)[0]; \
	(_seq)[2] = (_body)[4]; \
	(_seq)[3] = (_body)[5]; \
	(_seq)[4] = (_body)[6]; \
	(_seq)[5] = (_body)[7]; }

struct tkip_key {
    uint8 key[TKIP_KEY_TK_SIZE];
    struct {
		uint8 from_ds[TKIP_KEY_MIC_KEY_SIZE];
		uint8 to_ds[TKIP_KEY_MIC_KEY_SIZE];
	} mic_keys;
	uint8 rx_seq[1][TKIP_KEY_SEQ_SIZE];
};

typedef struct tkip_key tkip_key_t;

#define SIZEOF_TKIP_KEY(_key) (OFFSETOF(tkip_key_t, rx_seq) + \
	(KEY_NUM_RX_SEQ(_key) * TKIP_KEY_SEQ_SIZE))

static int
tkip_destroy(wlc_key_t *key)
{
	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	key->algo_impl.cb = NULL;
	key->algo_impl.ctx = NULL;
	key->info.key_len = 0;
	key->info.iv_len = 0;
	key->info.icv_len = 0;

	(void)wl_tkip_keyset(KEY_WLC(key)->wl, &key->info, NULL, 0, NULL, 0);
	return BCME_OK;
}

static int
tkip_get_data(wlc_key_t *key, uint8 *data, size_t data_size,
	size_t *data_len, key_data_type_t data_type, int ins, bool tx)
{
	tkip_key_t *tkip_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));
	KM_ASSERT(!WLC_KEY_IS_MGMT_GROUP(&key->info));

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		if (data_len != NULL) {
			*data_len = key->info.key_len;
		}

		if (data_size < key->info.key_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(data, tkip_key->key, key->info.key_len);
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (data_len != NULL) {
			*data_len = TKIP_KEY_SEQ_SIZE;
		}
		if (data_size < TKIP_KEY_SEQ_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (!TKIP_KEY_VALID_INS(key, tx, ins)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(data, TKIP_KEY_SEQ(tkip_key, tx, ins), TKIP_KEY_SEQ_SIZE);
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS:
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS:
	{
		uint8 *key_data;
		if (data_len != NULL) {
			*data_len = TKIP_KEY_MIC_KEY_SIZE;
		}

		if (data_size < TKIP_KEY_MIC_KEY_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (data_type == WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS)
			key_data = tkip_key->mic_keys.from_ds;
		else if (data_type == WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS)
			key_data = tkip_key->mic_keys.to_ds;
		else {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(data, key_data, TKIP_KEY_MIC_KEY_SIZE);
		break;
	}
	case WLC_KEY_DATA_TYPE_TKHASH_P1:
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
tkip_set_data(wlc_key_t *key, const uint8 *data,
    size_t data_len, key_data_type_t data_type, int ins, bool tx)
{
	tkip_key_t *tkip_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	/* no tkip for mgmt group keys */
	if (key->info.flags & WLC_KEY_FLAG_MGMT_GROUP) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		if (data_len > TKIP_KEY_SIZE)
			data_len = TKIP_KEY_SIZE;

		if (data_len != TKIP_KEY_SIZE) { /* includes mic keys */
			if (data_len != 0) {
				err = BCME_BADLEN;
				break;
			}
			memset(tkip_key, 0, SIZEOF_TKIP_KEY(key));
			err = wl_tkip_keyset(KEY_WLC(key)->wl, &key->info, NULL, 0, NULL, 0);
		} else if (!data) {
			err = BCME_BADARG;
			break;
		} else {
			memset(tkip_key, 0, SIZEOF_TKIP_KEY(key));
			memcpy(tkip_key->key, data, TKIP_KEY_SIZE);
			err = wl_tkip_keyset(KEY_WLC(key)->wl, &key->info,
				tkip_key->key, TKIP_KEY_SIZE,
				TKIP_KEY_SEQ(tkip_key, tx, ins), TKIP_KEY_SEQ_SIZE);
		}
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (!TKIP_KEY_VALID_INS(key, tx, ins)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (!data_len) {
			memset(tkip_key->rx_seq, 0, TKIP_KEY_SEQ_SIZE);
		} else  if (!data || (data_len < TKIP_KEY_SEQ_SIZE)) {
			err = BCME_BADLEN;
			break;
		} else {
			memcpy(tkip_key->rx_seq, data, TKIP_KEY_SEQ_SIZE);
		}

		err = wl_tkip_keyset(KEY_WLC(key)->wl, &key->info, tkip_key->key, TKIP_KEY_SIZE,
			TKIP_KEY_SEQ(tkip_key, tx, ins), TKIP_KEY_SEQ_SIZE);
		break;

	case WLC_KEY_DATA_TYPE_TKHASH_P1:
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS:
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS:
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}

static int
tkip_rx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi)
{
	tkip_key_t *tkip_key;
	int err = BCME_OK;
	uint8 rx_seq[TKIP_KEY_SEQ_SIZE];
	key_seq_id_t ins = 0;
	uint16 fc;
	uint16 qc;
	int lx_err;

	KM_ASSERT(TKIP_KEY_VALID(key));
	KM_DBG_ASSERT(hw_rxi != NULL);

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;

	fc =  ltoh16(hdr->fc);

	/* no mfp support w/ linux crypto */
	if ((FC_TYPE(fc) == FC_TYPE_MNG)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	KM_ASSERT(!WLC_KEY_IS_MGMT_GROUP(&key->info));

	/* check ext iv bit */
	if (!(body[KEY_ID_BODY_OFFSET] & DOT11_EXT_IV_FLAG)) {
		KEY_LOG(("wl%d: %s: EXT IV is not set for pkt, key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx));
		err = BCME_BADOPTION;
		goto done;
	}

	/* check wep seed */
	if (!TKIP_BODY_WEP_SEED_OKAY(body)) {
		KEY_LOG(("wl%d: %s: incorrect wep seed for tkip pkt, key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx));
		/* allow it */
	}

	/* check for replay */
	qc =  ((fc & FC_KIND_MASK) == FC_QOS_DATA) ?  ltoh16_ua(body - DOT11_QOS_LEN) : 0;
	ins = PRIO2IVIDX(QOS_PRIO(qc));
	KM_DBG_ASSERT(ins < KEY_NUM_RX_SEQ(key));

	TKIP_KEY_SEQ_FROM_BODY(rx_seq, body);
	if (km_is_replay(KEY_KM(key), &key->info, ins,
			TKIP_KEY_SEQ(tkip_key, FALSE /* rx */, ins),
			rx_seq, TKIP_KEY_SEQ_SIZE)) {
		err = BCME_REPLAY;
		goto done;
	}

	/* nothing else for hw decrypt. caller checks hw decr status */
	if (hw_rxi->attempted) {
		err = hw_rxi->status;
		goto done;
	}

	lx_err = wl_tkip_decrypt(KEY_WLC(key)->wl, pkt,
		(body - (uint8 *)hdr), ETHER_ISMULTI(&hdr->a1));

	if (lx_err != 0) {
		err = BCME_DECERR;
		goto done;
	}

done:
	if (err == BCME_OK) { /* on success, update rx iv */
		memcpy(TKIP_KEY_SEQ(tkip_key, FALSE /* rx */, ins),
			rx_seq, TKIP_KEY_SEQ_SIZE);
	} else { /* update error counters */
		switch (err) {
		case BCME_REPLAY:
			WLCNTINCR(KEY_CNT(key)->tkipreplay);
			if (ETHER_ISMULTI(&hdr->a1))
				WLCNTINCR(KEY_CNT(key)->tkipreplay_mcst);
			break;

		case BCME_DECERR:
			WLCNTINCR(KEY_CNT(key)->tkipicverr);
			if (ETHER_ISMULTI(&hdr->a1))
				WLCNTINCR(KEY_CNT(key)->tkipicverr_mcst);
			break;
		default:
			break;
		}
	}

	return err;
}

static int
tkip_tx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, wlc_txd_t *txd)
{
	int err = BCME_OK;
	int lx_err;

	KM_ASSERT(TKIP_KEY_VALID(key));

	/* no support for fragmentation and linux crypto requires 802.11
	 * header for computing mic (on sdu)
	 */
	lx_err = wl_tkip_micadd(KEY_WLC(key)->wl, pkt, (body - (uint8 *)hdr));
	if (lx_err != 0) {
		err = BCME_ENCERR;
		KEY_LOG(("wl%d: %s: wl_tkip_micadd failed pkt@%p key idx %d status %d\n",
			KEY_WLUNIT(key), __FUNCTION__, pkt, key->info.key_idx, lx_err));
		goto done;
	}

	/* if body_len is used beyond this point, it must be incremented by
	 * TKIP_MIC_SIZE as the above operation extends the sdu (pdu)
	 * body_len += TKIP_MIC_SIZE;
	 */

	KM_DBG_ASSERT(!WLC_KEY_IN_HW(&key->info));
	lx_err = wl_tkip_encrypt(KEY_WLC(key)->wl, pkt, (body  - (uint8 *)hdr));
	if (lx_err != 0) {
		err = BCME_ENCERR;
		goto done;
	}

done:
	return err;
}

static int
tkip_rx_msdu(wlc_key_t *key, void *pkt, struct ether_header *hdr,
	uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi)
{
	int err = BCME_OK;
	int lx_err;

	KM_ASSERT(TKIP_KEY_VALID(key));
	KM_DBG_ASSERT(!WLC_KEY_MIC_IN_HW(&key->info));

#ifdef CTFMAP
	/* Map the remaining unmapped frame contents before doing sw tkip mic check.
	 * Note that MIC is included in the mapping.
	 */
#if defined(linux) && defined(__ARM_ARCH_7A__)
	if (PKTISCTF(KEY_OSH(key), pkt))
		CTFMAPPTR(KEY_OSH(key), pkt) = (void *)PKTDATA(KEY_OSH(key), pkt);
#endif
	PKTCTFMAP(KEY_OSH(key), pkt);
#endif  /* CTFMAP */

	lx_err =  wl_tkip_miccheck(KEY_WLC(key)->wl, pkt,
		(body - (uint8 *)hdr), ETHER_ISMULTI(&hdr->ether_dhost[0]), key->info.key_id);

	if (lx_err != 0) {
		wl_tkip_printstats(KEY_WLC(key)->wl, ETHER_ISMULTI(&hdr->ether_dhost[0]));
		err = BCME_MICERR;
		goto done;
	}

done:
	/* strip mic upon success */
	if (err == BCME_OK) {
		PKTSETLEN(KEY_OSH(key), pkt, PKTLEN(KEY_OSH(key), pkt) - TKIP_MIC_SIZE);
	}

	KEY_LOG(("wl%d: %s: sw tkip mic check pkt@%p- key idx %d mic err status %d\n",
		KEY_WLUNIT(key), __FUNCTION__, pkt, key->info.key_idx, err));
	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
tkip_dump(const wlc_key_t *key, struct bcmstrbuf *b)
{
	tkip_key_t *tkip_key;
	size_t i;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	bcm_bprintf(b, "\ttkip key: ");
	for (i = 0; i < TKIP_KEY_TK_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->key[i]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\ttk, mic key from_ds: ");
	for (i = 0; i < TKIP_KEY_MIC_KEY_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->mic_keys.from_ds[i]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\ttk, mic key to ds: ");
	for (i = 0; i < TKIP_KEY_MIC_KEY_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->mic_keys.to_ds[i]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\ttkip rx_seq: ");
	for (i = 0; i < KEY_NUM_RX_SEQ(key); ++i) {
		size_t j;
		bcm_bprintf(b, "\t\t%d: 0x", i);
		for (j = TKIP_KEY_SEQ_SIZE; j > 0; --j)
			bcm_bprintf(b, "%02x", tkip_key->rx_seq[i][j-1]);
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}

#define TKIP_DUMP tkip_dump
#else
#define TKIP_DUMP NULL
#endif /* BCMDBG || BCMDBG_DUMP */

static const key_algo_callbacks_t key_tkip_callbacks = {
    tkip_destroy,	/* destroy */
    tkip_get_data,	/* get data */
    tkip_set_data,	/* set data */
    tkip_rx_mpdu,	/* rx mpdu */
    tkip_rx_msdu,	/* rx msdu */
    tkip_tx_mpdu,	/* tx mpdu */
    NULL,			/* tx msdu  - folded into tx mpdu */
    TKIP_DUMP		/* dump */
};

/* public interface */
int
km_key_tkip_linux_init(wlc_key_t *key)
{
	STATIC_ASSERT(TKIP_KEY_SIZE ==
		(TKIP_KEY_TK_SIZE + 2*TKIP_KEY_MIC_KEY_SIZE));

	KM_DBG_ASSERT(key->info.algo == CRYPTO_ALGO_TKIP);

	key->info.key_len = TKIP_KEY_SIZE;
	key->info.iv_len = TKIP_KEY_IV_SIZE;
	key->info.icv_len = TKIP_KEY_ICV_SIZE;
	key->info.flags |= WLC_KEY_FLAG_LINUX_CRYPTO;
	key->algo_impl.cb = &key_tkip_callbacks;
	key->algo_impl.ctx = MALLOCZ(KEY_OSH(key), SIZEOF_TKIP_KEY(key));
	return (key->algo_impl.ctx != NULL) ? BCME_OK : BCME_NOMEM;
}

#endif /* LINUX_CRYPTO */
