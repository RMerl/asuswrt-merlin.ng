/*
 * Implementation of wlc_key algo 'tkip'
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_tkip.c 534939 2015-02-16 19:17:55Z $
 */

#include "km_key_pvt.h"

#include <bcmcrypto/rc4.h>
#include <bcmcrypto/tkmic.h>
#include <bcmcrypto/tkhash.h>

/* internal interface */

#define TKIP_KEY_TK_SIZE TKIP_TK_SIZE
#define TKIP_KEY_MIC_KEY_SIZE TKIP_MIC_KEY_SIZE
#define TKIP_KEY_IV_SIZE DOT11_IV_TKIP_LEN		/* extended iv */
#define TKIP_KEY_ICV_SIZE DOT11_ICV_LEN			/* wep icv */
#define TKIP_KEY_SEQ_SIZE 6						/* 48 bit pn */

#define TKIP_KEY_VALID(_key) (((_key)->info.algo == CRYPTO_ALGO_TKIP) &&\
		((_key)->info.key_len == TKIP_KEY_TK_SIZE) &&\
		((_key)->info.iv_len == TKIP_KEY_IV_SIZE) &&\
		((_key)->info.icv_len == TKIP_KEY_ICV_SIZE))

#define TKIP_KEY_VALID_INS(_key, _tx, _ins) (\
	((_tx) && (_ins) == 0) ||\
	(!(_tx) && (_ins) < KEY_NUM_RX_SEQ(_key)))

#define TKIP_KEY_SEQ(_key, _tx, _ins) ((_tx) ?\
	&(_key)->tx_seq[0] : &(_key)->rx_seq[_ins][0])

#define TKIP_KEY_PHASE2(_key, _p2key, _tkip_key, _p1key, _seq) do {\
	tkhash_phase2((_p2key), (_tkip_key)->key, (_p1key), ltoh16_ua(_seq)); \
	KEY_LOG_DUMP(prhex("p2 key : ", (uint8*)(_p2key), TKHASH_P2_KEY_SIZE)); \
} while (0)

#define TKIP_BODY_WEP_SEED_OKAY(_body) ((_body)[1] == (((_body)[0] | 0x20) & 0x7f))

#define TKIP_MIC(_l, _r, _n, _m) do {\
	tkip_mic(_l, _r, _n, _m, &(_l), &(_r)); \
} while (0)

typedef uint16 tkip_phase1_key_t[TKHASH_P1_KEY_SIZE/sizeof(uint16)];
typedef uint8 tkip_phase2_key_t[TKHASH_P2_KEY_SIZE];

/* tkip tx key state supports phase1 keys and computing sdu mic. mic_off keeps
 * track of where mic starts
 */
struct tkip_key_tx_state {
	tkip_phase1_key_t phase1_key;
	uint32	frag_length;
	uint8	mic[TKIP_MIC_SIZE];
	uint8	mic_off;
};

typedef struct tkip_key_tx_state tkip_key_tx_state_t;

/* tkip rx key state includes support avoiding recomputing phase1 key */
struct tkip_key_rx_state {
	tkip_phase1_key_t cur_phase1_key;
	wlc_key_seq_id_t cur_seq_id;		/* corresponding to current phase1 key */
	wlc_key_seq_id_t next_p1k_seq_id;	/* pending update - seq id, phase 1 and seq */
	tkip_phase1_key_t next_phase1_key;
	uint8 next_seq[TKIP_KEY_SEQ_SIZE];
};
typedef struct tkip_key_rx_state tkip_key_rx_state_t;

/* context data type for tkip. mic keys must follow tk in order specified */
struct tkip_key {
	uint8 key[TKIP_KEY_TK_SIZE];
	struct {
		uint8 from_ds[TKIP_KEY_MIC_KEY_SIZE];
		uint8 to_ds[TKIP_KEY_MIC_KEY_SIZE];
	} mic_keys;

	tkip_key_tx_state_t tx_state;
	tkip_key_rx_state_t rx_state;
	uint8 tx_seq[TKIP_KEY_SEQ_SIZE];
	uint8 rx_seq[1][TKIP_KEY_SEQ_SIZE];
};

typedef struct tkip_key tkip_key_t;
#define SIZEOF_TKIP_KEY(_key) (OFFSETOF(tkip_key_t, rx_seq) +\
	(KEY_NUM_RX_SEQ(_key) * TKIP_KEY_SEQ_SIZE))

#define TKIP_KEY_AUTH_TX(_k) ((_k)->mic_keys.from_ds)
#define TKIP_KEY_AUTH_RX(_k) ((_k)->mic_keys.to_ds)
#define TKIP_KEY_SUP_TX(_k) ((_k)->mic_keys.to_ds)
#define TKIP_KEY_SUP_RX(_k) ((_k)->mic_keys.from_ds)

/* seq from body */
#define TKIP_KEY_SEQ_FROM_BODY(_seq, _body) {\
	(_seq)[0] = (_body)[2]; \
	(_seq)[1] = (_body)[0]; \
	(_seq)[2] = (_body)[4]; \
	(_seq)[3] = (_body)[5]; \
	(_seq)[4] = (_body)[6]; \
	(_seq)[5] = (_body)[7]; }

/* seq to body */
#define TKIP_KEY_IV_TO_BODY(_seq, _id, _body) {\
    STATIC_ASSERT(KEY_ID_BODY_OFFSET == 3); \
	(_body)[0] = (_seq)[1]; \
	(_body)[1] = ((_seq)[1] | 0x20) & 0x7f; \
	(_body)[2] = (_seq[0]); \
	(_body)[3] = (_id) << DOT11_KEY_INDEX_SHIFT | DOT11_EXT_IV_FLAG; \
	(_body)[4] = (_seq)[2]; \
	(_body)[5] = (_seq)[3]; \
	(_body)[6] = (_seq)[4]; \
	(_body)[7] = (_seq)[5]; }

/* support for incremental computation of tkip mic. this works with tkip_mic
 * restriction for input length to be multiple of 4 and avoids writing the pad
 * to frame buffer (avoids some DMA issues)
 */
struct tkip_mic_ctx {
	uint32 micl;
	uint32 micr;
	uint8 rem[sizeof(uint32)];
	uint8 rem_off;
};

typedef struct tkip_mic_ctx tkip_mic_ctx_t;

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
static int tkip_dump(const wlc_key_t *key, struct bcmstrbuf *b);
#define TKIP_DUMP tkip_dump

/*
 * debugging support, disabled 'coz not typically needed
 *	#define KM_TKIP_DUMP_KEYS_ON_MICERR
 *
*/

#else
#define TKIP_DUMP NULL
#endif /* BCMDBG || BCMDBG_DUMP */

static void
tkip_mic_init(tkip_mic_ctx_t *ctx, uint32 l, uint32 r)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->micl = l;
	ctx->micr = r;
}

static void
tkip_mic_update(tkip_mic_ctx_t *ctx, uint8 *buf, size_t buf_len)
{
	KM_DBG_ASSERT(ctx != NULL);

	if (buf == NULL || buf_len == 0)
		return;

	KM_DBG_ASSERT(ctx->rem_off < sizeof(uint32));
	if ((ctx->rem_off + buf_len) < sizeof(uint32)) {
		memcpy(&ctx->rem[ctx->rem_off], buf, buf_len);
		ctx->rem_off += (uint8)buf_len;
		return;
	} else {
		size_t clen = sizeof(uint32) - ctx->rem_off;
		memcpy(&ctx->rem[ctx->rem_off], buf, clen);
		buf += clen;
		buf_len -= clen;

		TKIP_MIC(ctx->micl, ctx->micr, sizeof(uint32), ctx->rem);
	}

	ctx->rem_off = (uint8)(buf_len & 0x3);
	buf_len &= ~0x3;

	if (buf_len) {
		TKIP_MIC(ctx->micl, ctx->micr, (int)buf_len, buf);
		buf += buf_len;
	}

	if (ctx->rem_off)
		memcpy(&ctx->rem[0], buf, ctx->rem_off);
}

static void
tkip_mic_final(tkip_mic_ctx_t *ctx, uint32 *l, uint32 *r)
{
	uint8 pad[8];
	uint8 off;

	KM_DBG_ASSERT(ctx != 0 && l != 0 && r != 0);
	off = ctx->rem_off;
	if (off)
		memcpy(pad, ctx->rem, off);
	pad[off++] = 0x5a;
	while (off < sizeof(pad))
		pad[off++] = 0;

	TKIP_MIC(ctx->micl, ctx->micr, sizeof(pad), pad);
	*l = ctx->micl;
	*r = ctx->micr;

	memset(ctx, 0, sizeof(*ctx));
}

/* end tkip mic incremental computation support */

/* get the appropriate mic key */
static void
tkip_get_mic_key(wlc_key_t *key, void *pkt, tkip_key_t *tkip_key, bool tx,
	uint32 *micl, uint32 *micr)
{
	scb_t *scb;
	wlc_bsscfg_t *bsscfg;
	const uint8* mic_key;

	scb = WLPKTTAGSCBGET(pkt);
	KM_DBG_ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	if ((BSSCFG_AP(bsscfg) && (!scb || !KM_SCB_WDS(scb) || KM_SCB_WPA_SUP(scb))) ||
		(BSSCFG_STA(bsscfg) && KM_BSSCFG_IS_IBSS(bsscfg))) {
#ifdef EXT_STA
		if (WLEXTSTA_ENAB(KEY_PUB(key)) && KM_BSSCFG_HAS_NATIVEIF(bsscfg)) {
			mic_key = tx ? tkip_key->mic_keys.to_ds : tkip_key->mic_keys.from_ds;
		} else
#endif /* EXT_STA */
			mic_key = tx ? tkip_key->mic_keys.from_ds : tkip_key->mic_keys.to_ds;
	} else
		mic_key = tx ? tkip_key->mic_keys.to_ds : tkip_key->mic_keys.from_ds;

	*micl = ltoh32_ua(mic_key);
	*micr = ltoh32_ua(mic_key + sizeof(uint32));
}

static void
tkip_key_phase1(wlc_key_t *key, tkip_phase1_key_t p1k, tkip_key_t *tkip_key,
	wlc_bsscfg_t *bsscfg, uint8 *seq, bool tx)
{
	uint8 *ta;
	uint32 hi;

	if (tx)
		ta  = bsscfg->cur_etheraddr.octet;
	else if (!ETHER_ISNULLADDR(&key->info.addr))
		ta = key->info.addr.octet;
	else
		ta = bsscfg->BSSID.octet;

	hi = ltoh32_ua((uint8 *)seq + sizeof(uint16));

	tkhash_phase1(p1k, tkip_key->key, ta, hi);
	KEY_LOG_DUMP(prhex("p1 key : ", (uint8*)p1k, TKHASH_P1_KEY_SIZE));
}

static int
tkip_destroy(wlc_key_t *key)
{
	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	if (key->algo_impl.ctx != NULL) {
		MFREE(KEY_OSH(key), key->algo_impl.ctx, SIZEOF_TKIP_KEY(key));
	}

	key->algo_impl.cb = NULL;
	key->algo_impl.ctx = NULL;
	key->info.key_len = 0;
	key->info.iv_len = 0;
	key->info.icv_len = 0;
	return BCME_OK;
}

static int
tkip_get_data(wlc_key_t *key, uint8 *data, size_t data_size,
	size_t *data_len, key_data_type_t data_type, int ins, bool tx)
{
	tkip_key_t *tkip_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));
	KM_DBG_ASSERT(!WLC_KEY_IS_MGMT_GROUP(&key->info));

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		if (data_len != NULL)
			*data_len = TKIP_KEY_SIZE; /* mic keys included */

		if (data_size < TKIP_KEY_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(data, tkip_key->key, TKIP_KEY_SIZE);
		break;
	case WLC_KEY_DATA_TYPE_TKIP_TK:
		if (data_len != NULL)
			*data_len = TKIP_KEY_TK_SIZE;

		if (data_size < TKIP_KEY_TK_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(data, tkip_key->key, TKIP_KEY_TK_SIZE);
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
		else
			break;

		memcpy(data, key_data, TKIP_KEY_MIC_KEY_SIZE);
		break;
	}
	case WLC_KEY_DATA_TYPE_TKHASH_P1:
		if (data_len != NULL) {
			*data_len = TKHASH_P1_KEY_SIZE + sizeof(uint32) /* seq hi32 */;
		}

		if (data_size < (TKHASH_P1_KEY_SIZE + sizeof(uint32))) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* return pre-computed phase1 keys for tx and the available one for rx.
		 * otherwise compute the phase1 key requested. no notification of
		 * ph1 key update here.
		 */
		if (tx) {
			memcpy(data, tkip_key->tx_state.phase1_key, TKHASH_P1_KEY_SIZE);
		} else if (ins == tkip_key->rx_state.cur_seq_id) {
			memcpy(data, tkip_key->rx_state.cur_phase1_key, TKHASH_P1_KEY_SIZE);
		} else {
			wlc_bsscfg_t *bsscfg;
			tkip_phase1_key_t p1key;
			bsscfg = wlc_keymgmt_get_bsscfg(KEY_KM(key), key->info.key_idx);
			if (bsscfg == NULL) {
				err = BCME_BADKEYIDX;
				break;
			}
			tkip_key_phase1(key, p1key, tkip_key, bsscfg,
				TKIP_KEY_SEQ(tkip_key, tx, ins), tx);
			memcpy(data, p1key, TKHASH_P1_KEY_SIZE);
		}

		/* copy out seq hi32 */
		memcpy(&data[TKHASH_P1_KEY_SIZE], TKIP_KEY_SEQ(tkip_key, tx, ins) + sizeof(uint16),
			sizeof(uint32));
		break;
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
	wlc_bsscfg_t *bsscfg;
	int ins_begin;
	int ins_end;
	int seq_id;
	uint8 old_seq[TKIP_KEY_SEQ_SIZE];

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	/* no tkip for mgmt group keys */
	if (key->info.flags & WLC_KEY_FLAG_MGMT_GROUP) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	bsscfg = wlc_keymgmt_get_bsscfg(KEY_KM(key), key->info.key_idx);
	if (bsscfg == NULL) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	KEY_RESOLVE_SEQ(key, ins, tx, ins_begin, ins_end);

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
		} else if (!data) {
			err = BCME_BADARG;
			break;
		} else {
			memset(tkip_key, 0, SIZEOF_TKIP_KEY(key));
			memcpy(tkip_key->key, data, TKIP_KEY_SIZE);
		}
		break;
	case WLC_KEY_DATA_TYPE_TKIP_TK:
		if (data_len > TKIP_TK_SIZE)
			data_len = TKIP_TK_SIZE;

		if (data_len != TKIP_TK_SIZE) {
			if (data_len != 0) {
				err = BCME_BADLEN;
				break;
			}
			/* clearing TK clears all */
			memset(tkip_key, 0, SIZEOF_TKIP_KEY(key));
		} else if (!data) {
			err = BCME_BADARG;
			break;
		} else {
			memcpy(tkip_key->key, data, TKIP_TK_SIZE);
		}
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (!TKIP_KEY_VALID_INS(key, tx, ins_begin)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		memcpy(old_seq, TKIP_KEY_SEQ(tkip_key, tx, ins_begin), TKIP_KEY_SEQ_SIZE);
		if (!data_len) {
			for (seq_id = ins_begin; seq_id < ins_end; ++seq_id) {
				memset(TKIP_KEY_SEQ(tkip_key, tx, seq_id), 0, TKIP_KEY_SEQ_SIZE);
			}
		} else  if (!data || (data_len < TKIP_KEY_SEQ_SIZE)) { /* allows truncation */
			err = BCME_BADARG;
			break;
		} else {
			for (seq_id = ins_begin; seq_id < ins_end; ++seq_id) {
				memcpy(TKIP_KEY_SEQ(tkip_key, tx, seq_id), data, TKIP_KEY_SEQ_SIZE);
			}
		}
		break;
	case WLC_KEY_DATA_TYPE_TKHASH_P1:
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS:
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS:
	default:
		/* phase1 key can  not be set. mic keys only received along with tk */
		err = BCME_UNSUPPORTED;
		break;
	}

	/* keep phase1 keys in sync */
	if (err == BCME_OK) {
		bool tx_upd = TRUE;
		bool rx_upd = TRUE;

		if (data_type == WLC_KEY_DATA_TYPE_SEQ) {
			uint8 *new_seq;
			bool hi32_upd;

			new_seq = TKIP_KEY_SEQ(tkip_key, tx, ins_begin);
			hi32_upd = (KEY_SEQ_HI32(old_seq) != KEY_SEQ_HI32(new_seq));
			tx_upd = tx && hi32_upd;
			rx_upd = !tx && hi32_upd;
		}

		if (tx_upd) {
			tkip_key_phase1(key, tkip_key->tx_state.phase1_key, tkip_key, bsscfg,
				tkip_key->tx_seq, TRUE);
		}

		if (rx_upd) {
			tkip_key_phase1(key, tkip_key->rx_state.cur_phase1_key, tkip_key, bsscfg,
				TKIP_KEY_SEQ(tkip_key, FALSE, ins_begin), FALSE);
			tkip_key->rx_state.cur_seq_id = (wlc_key_seq_id_t)ins_begin;
		}

		/* notify interested parties of seq update update. if key is
		 * updated km_key will do the notification (but not for sequence updates)
		 */
		if ((data_type == WLC_KEY_DATA_TYPE_SEQ) && (tx_upd || rx_upd)) {
			km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE,
				NULL, NULL, key, NULL);
		}
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

	KM_ASSERT(TKIP_KEY_VALID(key));
	KM_DBG_ASSERT(hw_rxi != NULL);

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;

	fc =  ltoh16(hdr->fc);

#if !defined(BCMCCX) || !defined(CCX_SDK)
	if ((FC_TYPE(fc) == FC_TYPE_MNG)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}
#endif /* !CCX || !CCX_SDK */

	KM_ASSERT(!WLC_KEY_IS_MGMT_GROUP(&key->info));

	/* check ext iv bit */
	if (!(body[KEY_ID_BODY_OFFSET] & DOT11_EXT_IV_FLAG)) {
		KEY_LOG(("wl%d: %s: EXT IV is not set for pkt, key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx));
		err = BCME_BADOPTION;
		goto done;
	}

#ifdef BCMDBG
	/* check wep seed */
	if (!TKIP_BODY_WEP_SEED_OKAY(body)) {
		KEY_LOG(("wl%d: %s: incorrect wep seed for tkip pkt, key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx));
		/* allow it */
	}
#endif /* BCMDBG */

	/* check for replay */

	if (FC_TYPE(fc) == FC_TYPE_MNG) {
#ifdef MFP
		ins = KEY_NUM_RX_SEQ(key) - 1;
#else
		err = BCME_UNSUPPORTED;
		goto done;
#endif
	} else {
		uint16 qc;
		qc =  ((fc & FC_KIND_MASK) == FC_QOS_DATA) ?  ltoh16_ua(body - DOT11_QOS_LEN) : 0;
		ins = PRIO2IVIDX(QOS_PRIO(qc));
		KM_ASSERT(ins < KEY_NUM_RX_SEQ(key));
	}

	TKIP_KEY_SEQ_FROM_BODY(rx_seq, body);
	if (km_is_replay(KEY_KM(key), &key->info, ins,
			TKIP_KEY_SEQ(tkip_key, FALSE /* rx */, ins),
			rx_seq, TKIP_KEY_SEQ_SIZE)) {
		err = BCME_REPLAY;
		goto done;
	}

	/* recompute (next) rx phase1 key, if necessary. the actual update to state
	 * happens after mic check
	 */
	tkip_key->rx_state.next_p1k_seq_id = WLC_KEY_SEQ_ID_INVALID;
	memcpy(tkip_key->rx_state.next_seq, rx_seq, TKIP_KEY_SEQ_SIZE);
	if (WLC_KEY_IS_IBSS_GROUP(&key->info) ||
		(KEY_SEQ_HI32(rx_seq) > KEY_SEQ_HI32(TKIP_KEY_SEQ(tkip_key, FALSE, ins))) ||
		((KEY_SEQ_HI32(rx_seq) != KEY_SEQ_HI32(TKIP_KEY_SEQ(tkip_key, FALSE, ins))) &&
		(ins != tkip_key->rx_state.cur_seq_id))) {

		scb_t *scb;
		wlc_bsscfg_t *bsscfg;

		scb = WLPKTTAGSCBGET(pkt);
		KM_ASSERT(scb != NULL);

		bsscfg = SCB_BSSCFG(scb);
		KM_DBG_ASSERT(bsscfg != NULL);

		tkip_key_phase1(key, tkip_key->rx_state.next_phase1_key,
			tkip_key, bsscfg, rx_seq, FALSE);
		tkip_key->rx_state.next_p1k_seq_id = ins;
	}

	/* nothing else for hw decrypt. caller checks hw decr status */
	if (hw_rxi->attempted) {
		err = hw_rxi->status;
		goto done;
	}

	{
		tkip_phase2_key_t p2key;
		rc4_ks_t rc4_ks;

		TKIP_KEY_PHASE2(key, p2key, tkip_key,
			((tkip_key->rx_state.next_p1k_seq_id != WLC_KEY_SEQ_ID_INVALID) ?
			tkip_key->rx_state.next_phase1_key :
			tkip_key->rx_state.cur_phase1_key), rx_seq);

		KEY_LOG(("wl%d: %s: computed rx phase2 key for key idx %d seq "
			KEY_SEQ_FORMAT "\n", KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx,
			KEY_SEQ_FORMAT_ARG(rx_seq)));

		prepare_key(p2key, TKHASH_P2_KEY_SIZE, &rc4_ks);
		rc4(body + key->info.iv_len, body_len - key->info.iv_len, &rc4_ks);
		if (hndcrc32(body + key->info.iv_len, body_len - key->info.iv_len,
			CRC32_INIT_VALUE) != CRC32_GOOD_VALUE) {
			err = BCME_DECERR;
		}
	}

done:
	if (err != BCME_OK) { /* update counters */
		switch (err) {
		case BCME_REPLAY:
#if defined(BCMCCX) && defined(CCX_SDK)
			if (FC_TYPE(fc) == FC_TYPE_MNG) {
				WLCNTINCR(KEY_CCX(key)->mgmt_cnt.tkipmgmtreplayerr);
			} else
#endif /* BCMCCX && CCX_SDK */
			{
				WLCNTINCR(KEY_CNT(key)->tkipreplay);
				if (ETHER_ISMULTI(&hdr->a1))
					WLCNTINCR(KEY_CNT(key)->tkipreplay_mcst);
			}
			break;

		case BCME_DECERR:
#if defined(BCMCCX) && defined(CCX_SDK)
			if (FC_TYPE(fc) == FC_TYPE_MNG) {
				WLCNTINCR(KEY_CCX(key)->mgmt_cnt.tkipmgmticverr);
			} else
#endif /* BCMCCX && CCX_SDK */
			{
				WLCNTINCR(KEY_CNT(key)->tkipicverr);
				if (ETHER_ISMULTI(&hdr->a1))
					WLCNTINCR(KEY_CNT(key)->tkipicverr_mcst);
			}
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
	tkip_key_t *tkip_key;
	uint16 fc;
	int mic_len;
	int frag_length;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	fc =  ltoh16(hdr->fc);
	if ((FC_TYPE(fc) == FC_TYPE_MNG) && ETHER_ISMULTI(&hdr->a1)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;

#ifdef BCMDBG
	if (key->info.flags & WLC_KEY_FLAG_GEN_REPLAY) {
		key->info.flags &= ~WLC_KEY_FLAG_GEN_REPLAY;
	} else
#endif /* BCMDBG */
	{
		if (KEY_SEQ_IS_MAX(tkip_key->tx_seq)) { /* rollover */
			err = BCME_BADKEYIDX;
			goto done;
		}

		KEY_SEQ_INCR(tkip_key->tx_seq, TKIP_KEY_SEQ_SIZE);
		KEY_OL_IV_UPDATE(key, tkip_key->tx_seq, 0, TRUE);
		if (!KEY_SEQ_LO16(tkip_key->tx_seq)) { /* phase1 key update */
			scb_t *scb;
			wlc_bsscfg_t *bsscfg;

			scb  = WLPKTTAGSCBGET(pkt);
			KM_ASSERT(scb != NULL);

			bsscfg = SCB_BSSCFG(scb);
			KM_DBG_ASSERT(bsscfg != NULL);

			tkip_key_phase1(key, tkip_key->tx_state.phase1_key,
				tkip_key, bsscfg, tkip_key->tx_seq, TRUE);

			KEY_LOG(("wl%d: %s: computed and updated tx phase1 key for key idx %d seq "
				KEY_SEQ_FORMAT "\n", KEY_WLUNIT(key), __FUNCTION__,
				key->info.key_idx, KEY_SEQ_FORMAT_ARG(tkip_key->tx_seq)));

			/* notify keymgmt - since we updated the key tx state and h/w may need to be
			 * re-programmed.
			 */
			km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE, NULL, NULL, key, NULL);
		}
	}

	TKIP_KEY_IV_TO_BODY(tkip_key->tx_seq, key->info.key_id, body);

	/* msdu mic support: add mic and adjust pkt/body length as necessary
	 * if unfragmented or last frag, we must add the remaining mic, otherwise
	 * mic added will be as permitted by fragment length
	 */
	KM_ASSERT(tkip_key->tx_state.mic_off <= TKIP_MIC_SIZE);
	mic_len = TKIP_MIC_SIZE - tkip_key->tx_state.mic_off;
	frag_length = tkip_key->tx_state.frag_length;
	if (mic_len > 0) {
		const uint8 mic_off = tkip_key->tx_state.mic_off;

		if (frag_length && (fc & FC_MOREFRAG))
			mic_len = MIN((frag_length - (body_len - key->info.iv_len)), mic_len);

		KM_DBG_ASSERT(mic_len >= 0 && mic_len <= TKIP_MIC_SIZE);
		if (mic_len > 0) {
			uint8 *tx_micp;
			void* mic_pkt;

			/* add mic to last buffer of the mpdu  */
			mic_pkt = pktlast(KEY_OSH(key), pkt);
			KM_DBG_ASSERT(mic_pkt != NULL);
			KM_ASSERT(PKTTAILROOM(KEY_OSH(key), mic_pkt) >= mic_len);

			tx_micp = (mic_pkt == pkt) ? (body + body_len) :
				(uint8 *)PKTDATA(KEY_OSH(key), mic_pkt) +
				PKTLEN(KEY_OSH(key), mic_pkt);

			memcpy(tx_micp, &tkip_key->tx_state.mic[mic_off], mic_len);
			tkip_key->tx_state.mic_off += (uint8)mic_len;
			PKTSETLEN(KEY_OSH(key), mic_pkt, PKTLEN(KEY_OSH(key), mic_pkt) + mic_len);
		}
	}

	if (WLC_KEY_IN_HW(&key->info) && txd != NULL) {
		uint8 *ph1;
		uint8 *pn;
		int i, j;
		bool hwmic;

		hwmic = WLC_KEY_MIC_IN_HW(&key->info) && !frag_length;
		if (KEY_USE_AC_TXD(key)) {
			d11actxh_t *txh = (d11actxh_t *)&txd->txd;
			d11actxh_cache_t	*cache_info;

			cache_info = WLC_TXD_CACHE_INFO_GET(txh, KEY_PUB(key)->corerev);
			ph1 = cache_info->TkipPH1Key;
			pn = cache_info->TSCPN;
			if (hwmic)
				txh->PktInfo.MacTxControlLow =
					htol16(ltoh16(txh->PktInfo.MacTxControlLow) |
					D11AC_TXC_AMIC);
		} else {
			d11txh_t *txh = (d11txh_t *)&txd->d11txh;
			ph1 = txh->IV;
			pn = &txh->IV[TKHASH_P1_KEY_SIZE];
			if (hwmic)
				txh->MacTxControlLow =
					htol16(ltoh16(txh->MacTxControlLow) | TXC_AMIC);
		}

		for (i = 0, j = 0; i < TKHASH_P1_KEY_SIZE / 2; ++i) {
			uint16 val = tkip_key->tx_state.phase1_key[i];
			ph1[j++] = val & 0xff;
			ph1[j++] = val >> 8;
		}
		memcpy(pn, body, 3);

		goto done;
	}

	/* sw encryption */
	{
		tkip_phase2_key_t p2key;
		uint32 icv;
		rc4_ks_t rc4_ks;

		body_len += mic_len;
		TKIP_KEY_PHASE2(key, p2key, tkip_key, tkip_key->tx_state.phase1_key,
			tkip_key->tx_seq);

		KEY_LOG(("wl%d: %s: computed tx phase2 key for key idx %d seq "
			KEY_SEQ_FORMAT "\n", KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx,
			KEY_SEQ_FORMAT_ARG(tkip_key->tx_seq)));

		KM_DBG_ASSERT(sizeof(icv) == key->info.icv_len);

		icv = htol32(~hndcrc32(body + key->info.iv_len,
			body_len - key->info.iv_len, CRC32_INIT_VALUE));
		memcpy(body + body_len, &icv, sizeof(icv));
		body_len += sizeof(icv);
		/* now body starts at icv spans to end-of icv. */

		prepare_key(p2key, TKHASH_P2_KEY_SIZE, &rc4_ks);
		rc4(body + key->info.iv_len, body_len - key->info.iv_len, &rc4_ks);
	}

done:
	return err;
}

static int
tkip_rx_msdu(wlc_key_t *key, void *pkt, struct ether_header *hdr,
	uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi)
{
	int err = BCME_OK;
	tkip_key_t *tkip_key;
	uint8 *rx_micp;
	uint32 rx_micl;
	uint32 rx_micr;
	uint32 micl;
	uint32 micr;
	tkip_mic_ctx_t mic_ctx;
	uint32 qos;

	KM_ASSERT(TKIP_KEY_VALID(key));
	KM_DBG_ASSERT(hw_rxi != NULL);

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	if (body_len < TKIP_MIC_SIZE) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	if (hw_rxi->attempted) {
		err = hw_rxi->status;
		goto done;
	}

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

	rx_micp = (uint8 *)(body + body_len - TKIP_MIC_SIZE);
	rx_micl = ltoh32_ua(rx_micp);
	rx_micr = ltoh32_ua(rx_micp + sizeof(uint32));

	/* compute tkip mic over DA.SA.prio.body.pad */
	tkip_get_mic_key(key, pkt, tkip_key, FALSE /* rx */, &micl, &micr);
	tkip_mic_init(&mic_ctx, micl, micr);
	tkip_mic_update(&mic_ctx, (uint8 *)hdr, (ETHER_ADDR_LEN << 1));

	qos = htol32(PKTPRIO(pkt));
	tkip_mic_update(&mic_ctx, (uint8 *)&qos, sizeof(qos));
	tkip_mic_update(&mic_ctx, body, body_len - TKIP_MIC_SIZE);
	tkip_mic_final(&mic_ctx, &micl, &micr);

	if (micl != rx_micl || micr != rx_micr) {
		err = BCME_MICERR;
		KEY_ERR(("wl%d: %s: TKIP MIC failure for frame"
			" from " MACDBG " to " MACDBG
			", key idx %d, expected %08x.%08x got %08x.%08x\n",
			KEY_WLUNIT(key), __FUNCTION__,
			MAC2STRDBG(hdr->ether_shost), MAC2STRDBG(hdr->ether_dhost),
			key->info.key_idx, micl, micr, rx_micl, rx_micr));

#ifdef KM_TKIP_DUMP_KEYS_ON_MICERR
		{
			struct bcmstrbuf b;
			char tkip_key_buf[256];
			bcm_binit(&b, tkip_key_buf, sizeof(tkip_key_buf));
			tkip_dump(key, &b);
			tkip_key_buf[sizeof(tkip_key_buf)-1] = 0;
			KEY_LOG(("wl%d: %s: tkip key info:\n%s\n",
				KEY_WLUNIT(key), __FUNCTION__, tkip_key_buf));
		}
#endif
		goto done;
	}

done:
	/* strip mic upon success */
	if (err == BCME_OK) {
		wlc_key_seq_id_t rx_seq_id;

		/* SPLIT RX: mic could be in the host */
		PKTFRAG_TRIM_TAILBYTES(KEY_OSH(key), pkt, TKIP_MIC_SIZE, TAIL_BYTES_TYPE_MIC);

		/* update rx iv and phase1 key if it changed. notify km as needed */
		rx_seq_id = (tkip_key->rx_state.next_p1k_seq_id != WLC_KEY_SEQ_ID_INVALID) ?
			tkip_key->rx_state.next_p1k_seq_id : tkip_key->rx_state.cur_seq_id;

		if (!WLC_KEY_USE_IVTW(&key->info)) {
			memcpy(TKIP_KEY_SEQ(tkip_key, FALSE /* rx */, rx_seq_id),
				tkip_key->rx_state.next_seq, TKIP_KEY_SEQ_SIZE);
			KEY_OL_IV_UPDATE(key, tkip_key->rx_state.next_seq, rx_seq_id, FALSE);
		} else {
			KM_UPDATE_IVTW(KEY_KM(key), &key->info, rx_seq_id,
				tkip_key->rx_state.next_seq, TKIP_KEY_SEQ_SIZE);
		}

		if (tkip_key->rx_state.next_p1k_seq_id != WLC_KEY_SEQ_ID_INVALID) {
			tkip_key->rx_state.cur_seq_id = tkip_key->rx_state.next_p1k_seq_id;
			memcpy(tkip_key->rx_state.cur_phase1_key,
				tkip_key->rx_state.next_phase1_key, sizeof(tkip_phase1_key_t));
#ifdef BCM_OL_DEV
			key->info.flags &= ~WLC_KEY_FLAG_NO_HW_UPDATE;
#endif /* BCM_OL_DEV */
			/* notify keymgmt - h/w may need to be re-programmed */
			km_notify(KEY_KM(key), WLC_KEYMGMT_NOTIF_KEY_UPDATE, NULL, NULL, key, NULL);
#ifdef BCM_OL_DEV
			key->info.flags |= WLC_KEY_FLAG_NO_HW_UPDATE;
#endif /* BCM_OL_DEV */
			tkip_key->rx_state.next_p1k_seq_id = WLC_KEY_SEQ_ID_INVALID;
		}
	} else if (err == BCME_MICERR) {
#if defined(BCMCCX) && defined(CCX_SDK)
		if (WLPKTFLAG_PMF(WLPKTTAG(pkt))) {
			WLCNTINCR(KEY_CCX(key)->mgmt_cnt.tkipmgmtlocalmicfail);
		}
		else
#endif /* BCMCCX && CCX_SDK */
		{
			WLCNTINCR(KEY_CNT(key)->tkipmicfaill);
			if (ETHER_ISMULTI(hdr->ether_dhost))
				WLCNTINCR(KEY_CNT(key)->tkipmicfaill_mcst);
		}
	}

	KEY_LOG(("wl%d: %s: %s tkip mic check pkt@%p- key idx %d mic err status %d\n",
		KEY_WLUNIT(key), __FUNCTION__, (hw_rxi->attempted ? "hw" : "sw"),
		pkt, key->info.key_idx, err));
	return err;
}

static int
tkip_tx_msdu(wlc_key_t *key, void *pkt, struct ether_header *hdr,
	uint8 *body, int body_len, size_t frag_length, uint8 prio)
{
	tkip_key_t *tkip_key;
	tkip_mic_ctx_t mic_ctx;
	uint32 micl, micr;
	uint32 qos;
	STATIC_ASSERT(TKIP_MIC_SIZE == (sizeof(uint32) << 1));

	KM_ASSERT(TKIP_KEY_VALID(key));

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;

	/* use h/w mic if supported for unfragmented pkts */
	tkip_key->tx_state.frag_length = (uint32)frag_length;
	if (!frag_length && WLC_KEY_MIC_IN_HW(&key->info)) {
		tkip_key->tx_state.mic_off = TKIP_MIC_SIZE;
		goto done;
	}

	tkip_get_mic_key(key, pkt, tkip_key, TRUE, &micl, &micr);

#ifdef BCMDBG
	if (key->info.flags & WLC_KEY_FLAG_GEN_MIC_ERR) {
		micl++;
		key->info.flags &= ~WLC_KEY_FLAG_GEN_MIC_ERR;
	}
#endif

	tkip_mic_init(&mic_ctx, micl, micr);
	tkip_mic_update(&mic_ctx, (uint8 *)hdr, (ETHER_ADDR_LEN << 1));

	qos = htol32(prio);
	tkip_mic_update(&mic_ctx, (uint8 *)&qos, sizeof(qos));

	/* update the sdu mic for the body; it may span multiple pkts */
	tkip_mic_update(&mic_ctx, body, body_len);
	while ((pkt = PKTNEXT(KEY_OSH(key), pkt)) != NULL) {
		uint8 *data = PKTDATA(KEY_OSH(key), pkt);
		int data_len = PKTLEN(KEY_OSH(key), pkt);
		tkip_mic_update(&mic_ctx, data, data_len);
	}

	tkip_mic_final(&mic_ctx, &micl, &micr);
	KEY_LOG(("wl%d: %s: computed mic l 0x%08x r 0x%08x\n",
		KEY_WLUNIT(key), __FUNCTION__, micl, micr));

	htol32_ua_store(micl, tkip_key->tx_state.mic);
	htol32_ua_store(micr, tkip_key->tx_state.mic + sizeof(uint32));
	tkip_key->tx_state.mic_off = 0;

done:
	return BCME_OK;
}

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
static int
tkip_dump(const wlc_key_t *key, struct bcmstrbuf *b)
{
	tkip_key_t *tkip_key;
	size_t i, j;

	KM_DBG_ASSERT(TKIP_KEY_VALID(key));

	tkip_key = (tkip_key_t *)key->algo_impl.ctx;
	bcm_bprintf(b, "\ttkip key: ");
	for (i = 0; i < TKIP_KEY_TK_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->key[i]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\ttkip mic key from_ds: ");
	for (i = 0; i < TKIP_KEY_MIC_KEY_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->mic_keys.from_ds[i]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\ttkip mic key to ds: ");
	for (i = 0; i < TKIP_KEY_MIC_KEY_SIZE; ++i)
		bcm_bprintf(b, "%02x", tkip_key->mic_keys.to_ds[i]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\ttkip tx seq: 0x");
	for (i = TKIP_KEY_SEQ_SIZE; i > 0; --i)
		bcm_bprintf(b, "%02x", tkip_key->tx_seq[i-1]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\ttkip rx seq:\n");
	for (i = 0; i < (size_t)KEY_NUM_RX_SEQ(key); ++i) {
		bcm_bprintf(b, "\t\t%d: 0x", i);
		for (j = TKIP_KEY_SEQ_SIZE; j > 0; --j)
			bcm_bprintf(b, "%02x", tkip_key->rx_seq[i][j-1]);
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "\ttkip tx state:\n");
	bcm_bprintf(b, "\t\tphase1 key: ");
	for (i = 0; i < TKHASH_P1_KEY_SIZE/sizeof(uint16); ++i) {
		uint16 x = htol16(tkip_key->tx_state.phase1_key[i]);
		bcm_bprintf(b, "%02x%02x", (x & 0xff), (x >> 8 & 0xff));
	}
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\t\tfrag_length: %d\n", tkip_key->tx_state.frag_length);
	bcm_bprintf(b, "\t\tmic_off: %d\n", tkip_key->tx_state.mic_off);
	bcm_bprintf(b, "\t\tmic: ");
	bcm_bprintf(b, "%08x%08x",  ltoh32_ua(tkip_key->tx_state.mic),
		ltoh32_ua(tkip_key->tx_state.mic + sizeof(uint32)));
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\ttkip rx state:\n");
	bcm_bprintf(b, "\t\tcur phase1 key: ");
	for (i = 0; i < TKHASH_P1_KEY_SIZE/sizeof(uint16); ++i) {
		uint16 x = htol16(tkip_key->rx_state.cur_phase1_key[i]);
		bcm_bprintf(b, "%02x%02x", (x & 0xff), (x >> 8 & 0xff));
	}
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\t\tcur seq id: %d\n", tkip_key->rx_state.cur_seq_id);

	bcm_bprintf(b, "\t\tnext p1k seq id: %d\n", tkip_key->rx_state.next_p1k_seq_id);
	bcm_bprintf(b, "\t\tnext phase1 key: ");
	for (i = 0; i < TKHASH_P1_KEY_SIZE/sizeof(uint16); ++i) {
		uint16 x = htol16(tkip_key->rx_state.next_phase1_key[i]);
		bcm_bprintf(b, "%02x%02x", (x & 0xff), (x >> 8 & 0xff));
	}
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\t\tnext seq: 0x");
	for (j = TKIP_KEY_SEQ_SIZE; j > 0; --j)
		bcm_bprintf(b, "%02x", tkip_key->rx_state.next_seq[j-1]);
	bcm_bprintf(b, "\n");

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

static const key_algo_callbacks_t key_tkip_callbacks = {
    tkip_destroy,	/* destroy */
    tkip_get_data,	/* get data */
    tkip_set_data,	/* set data */
    tkip_rx_mpdu,	/* rx mpdu */
    tkip_rx_msdu,	/* rx msdu */
    tkip_tx_mpdu,	/* tx mpdu */
    tkip_tx_msdu,	/* tx msdu */
    TKIP_DUMP		/* dump */
};

/* public interface */
int
km_key_tkip_init(wlc_key_t *key)
{
	STATIC_ASSERT(TKIP_KEY_SIZE ==
		(TKIP_KEY_TK_SIZE + 2*TKIP_KEY_MIC_KEY_SIZE));

	KM_DBG_ASSERT(key->info.algo == CRYPTO_ALGO_TKIP);

	key->info.key_len = TKIP_KEY_TK_SIZE;
	key->info.iv_len = TKIP_KEY_IV_SIZE;
	key->info.icv_len = TKIP_KEY_ICV_SIZE;
	key->algo_impl.cb = &key_tkip_callbacks;
	key->algo_impl.ctx = MALLOCZ(KEY_OSH(key), SIZEOF_TKIP_KEY(key));
#ifdef BRCMAPIVTW
	key->info.flags |= WLC_KEY_FLAG_USE_IVTW; /* if enabled */
#endif /* BRCMAPIVTW */
	return (key->algo_impl.ctx != NULL) ? BCME_OK : BCME_NOMEM;
}
