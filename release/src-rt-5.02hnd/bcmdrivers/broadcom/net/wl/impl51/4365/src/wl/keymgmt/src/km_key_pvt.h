/*
 * Private header - wireless security key implementation
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_key_pvt.h 524731 2015-01-07 23:03:31Z $
 */

#ifndef _km_key_pvt_h_
#define _km_key_pvt_h_

#include "km.h"
#include "km_key.h"

#define KEY_MAGIC 0x0077736B
#ifdef BCMDBG
#define KEY_MAGIC_DECL uint32 magic;
#define KEY_MAGIC_INIT(_k)  (_k)->magic = KEY_MAGIC
#define KEY_MAGIC_VALID(_k) ((_k)->magic == KEY_MAGIC)
#else
#define KEY_MAGIC_DECL
#define KEY_MAGIC_INIT(_k)
#define KEY_MAGIC_VALID(_k) TRUE
#endif

/* convenience typedefs */
typedef wlc_keymgmt_t keymgmt_t;
/* key_t is taken - sys/types.h */
typedef wlc_key_info_t key_info_t;
typedef wlc_key_algo_t key_algo_t;
typedef wlc_key_id_t key_id_t;
typedef wlc_key_index_t key_index_t;
typedef wlc_key_seq_id_t key_seq_id_t;
typedef wlc_key_data_type_t key_data_type_t;
typedef km_key_dump_type_t key_dump_type_t;

/* algorithm specific initialiazation support */
typedef int (*key_algo_init_t)(wlc_key_t *key);

#define DECL_KEY_ALGO_INIT(algo)  int km_key_##algo##_init(wlc_key_t *key)

DECL_KEY_ALGO_INIT(none); /* no algorithm, null key */
DECL_KEY_ALGO_INIT(wep); /* 40 and 104 bit keys */

DECL_KEY_ALGO_INIT(tkip);
DECL_KEY_ALGO_INIT(tkip_linux);

DECL_KEY_ALGO_INIT(aes); /* 128 and 256 (future) bit keys, incl BIP */
DECL_KEY_ALGO_INIT(nalg);

#ifdef BCMWAPI_WPI
DECL_KEY_ALGO_INIT(wapi);
#endif /* BCMWAPI_WPI */

#undef DECL_KEY_ALGO_INIT

/* a key algorithm is implemented via follwing callbacks */

/* destroy algorithm specific key */
typedef int (*key_algo_destroy_cb_t)(wlc_key_t *key);
/* get algorithm specific key info */
typedef int (*key_algo_get_cb_t)(wlc_key_t *key, uint8 *data,
	size_t data_size, size_t *data_len, key_data_type_t type,
	int instance, bool tx);
/* set algorithm specific key info. if data_len is 0, the key
 * material is cleared
 */
typedef int (*key_algo_set_cb_t)(wlc_key_t *key, const uint8 *data,
	size_t data_len, key_data_type_t type, int instance, bool tx);

/* packet handlers. packet data starts with 802.11 header for mpdu
 * callbacks and ethernet header for msdu callbacks.
 */

/* rx mpdu callback
 * 		decrypt mpdu if required - mgmt, data or qos data
 *		do replay checks
 *		update ivs/rx seq
 *		increment algo specific counters, both for h/w & s/w decr
 *		for h/w decr, only rx iv is updated.
 */

struct key_hw_rx_info {
	bool attempted;
	int status;
};
typedef struct key_hw_rx_info key_hw_rx_info_t;

typedef int (*key_rx_cb_t)(wlc_key_t *key, void* pkt, struct dot11_header *hdr,
	uint8* body, int body_len, const key_hw_rx_info_t *hw_rxi);

/* rx msdu callback
 *		do mic check over fully re-assembled 802.3 frame (after conversion from 802.11).
 */
typedef int (*key_rx_msdu_cb_t)(wlc_key_t *key, void* pkt, struct ether_header *hdr,
	uint8* body, int body_len, const key_hw_rx_info_t *hw_rxi);

/* tx mpdu callback
 *	 check key flags for error generation etc. and skip iv update (state)
 *	 otherwise update iv (state)
 *	 set iv in the pkt
 *	 if sw encryption, encrypt frame and add mpdu MIC
 *	 check key flags for error generation etc. generate icv error
 *	 update txh (ac or pre-ac) with h/w algo, hw idx etc. for h/w enc
 */
typedef int (*key_tx_cb_t)(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8* body, int body_len, wlc_txd_t *txd);

/* tx msdu callback
 *	process 802.3 msdu for mic, if applicable.
 */
typedef int (*key_tx_msdu_cb_t)(wlc_key_t *key, void *pkt, struct ether_header *hdr,
	uint8* body, int body_len, size_t frag_length, uint8 prio);

/* dump */
typedef int (*key_dump_cb_t)(const wlc_key_t *key, struct bcmstrbuf *b);

struct key_algo_callbacks {
	key_algo_destroy_cb_t		destroy;
	key_algo_get_cb_t			get_data;
	key_algo_set_cb_t			set_data;
	key_rx_cb_t					rx_mpdu;
	key_rx_msdu_cb_t			rx_msdu;
	key_tx_cb_t					tx_mpdu;
	key_tx_msdu_cb_t			tx_msdu;
	key_dump_cb_t				dump;
};
typedef struct key_algo_callbacks key_algo_callbacks_t;

#define KEY_ALGO_CB(_ret, _fn, _args) {\
	_ret = (((_fn) != NULL) ? ((*_fn) _args) : BCME_UNSUPPORTED); \
}

struct key_algo_impl {
	const key_algo_callbacks_t	*cb;
	void *ctx;
};
typedef struct key_algo_impl key_algo_impl_t;

/* key definition */
struct wlc_key {
	KEY_MAGIC_DECL
	wlc_info_t				*wlc;
	key_info_t  			info;
	wlc_key_expiration_t 	exp;
	wlc_key_hw_index_t		hw_idx;
	key_algo_impl_t 		algo_impl;
};

/* helper macros */
#define KEY_VALID(k) ((k) != NULL &&\
	KEY_MAGIC_VALID(k) &&\
	(k)->wlc != NULL &&\
	(k)->wlc->keymgmt != NULL &&\
	(k)->algo_impl.cb != NULL)

#define KEY_WLC(_key) (_key)->wlc
#define KEY_KM(_key) KEY_WLC(_key)->keymgmt
#define KEY_WLUNIT(_key) WLCWLUNIT(KEY_WLC(_key))
#define KEY_OSH(_key) KEY_WLC(_key)->osh
#define KEY_PUB(_key) KEY_WLC(_key)->pub
#define KEY_CNT(_key) KEY_PUB(_key)->_cnt
#define KEY_MPDU_HDR(_key, _pkt) (struct dot11_header *)\
	PKTDATA(KEY_OSH(_key), (_pkt))
#define KEY_MSDU_HDR(_key, _pkt) (struct ether_header *)\
	PKTDATA(KEY_OSH(_key), (_pkt))
#define KEY_PKT_LEN(_key, _pkt) PKTLEN(KEY_OSH(_key), _pkt)

#ifndef BCM_OL_DEV
#define KEY_COREREV(_key) KEY_PUB(_key)->corerev
#define KEY_WLCNINC(_key, _ctr) WLCNINC(KEY_PUB(_key)->_ctr)
#define KEY_DEFAULT_BSSCFG(_key) KEY_WLC(_key)->cfg
#else
#define KEY_COREREV(_key) si_corerev(KEY_PUB(_key)->sih)
#define KEY_WLCNINC(_key, _ctr)
#define KEY_DEFAULT_BSSCFG(_key) KM_WLC_BSSCFG(KEY_WLC(_key), 0)
#endif /* !BCM_OL_DEV */

#define KEY_COREREV_GE40(_key) D11REV_GE(KEY_COREREV(_key), 40)
#define KEY_USE_AC_TXD(_key) (((_key)->info.flags & WLC_KEY_FLAG_USE_AC_TXD) != 0)

#define KEY_ERR(args) KM_ERR(args)
#define KEY_LOG(args) KM_LOG(args)
#define KEY_PRINTF(args) KM_PRINTF(args)
#define KEY_LOG_DECL(stmt) KM_LOG_DECL(stmt)
#define KEY_LOG_DUMP(stmt) KM_LOG_DUMP(stmt)
#define KEY_LOG_DUMP_PKT(_msg, _key, _pkt) KM_LOG_DUMP_PKT(_msg, KEY_WLC(_key), _pkt)

#if defined(BCMCCX)
#define KEY_CCX(_key) KEY_WLC(_key)->ccx
#else
#define KEY_CCX(_key) NULL
#endif

#define KEY_MPDU_LEN(_key, _pkt) PKTLEN(KEY_OSH(_key), _pkt)

/* whether to use sw/enc, dec - see km_util.c:km_needs_hw_key() */
#define KEY_SW_ENC_DEC(_key, _pkt, _rxh) (\
	!((_key)->info.flags & WLC_KEY_FLAG_IN_HW) ||\
	!((_rxh)->RxStatus1 & RXS_DECATMPT))

#define KEY_ID_BODY_OFFSET KM_PKT_KEY_ID_BODY_OFFSET

#ifndef BCM_OL_DEV
#define KEY_WLC_UP(_key) KEY_WLC(_key)->pub->up
#else
#define KEY_WLC_UP(_key) TRUE
#endif /* !BCM_OL_DEV */

/* whether or not tx/rx are enabled */
#define KEY_TX_ENABLED(_key) (KEY_WLC_UP(_key) && \
	WLC_KEY_TX_ALLOWED(&(_key)->info))

#define KEY_RX_ENABLED(_key) (KEY_WLC_UP(_key) && \
	WLC_KEY_RX_ALLOWED(&(_key)->info))

#define KEY_NUM_RX_SEQ(_key) KEY_PUB(_key)->tunables->num_rxivs

#ifdef BCM_OL_DEV
#define KEY_OL_IV_UPDATE(_key, _seq, _seq_id, _tx) do { \
	wlc_dngl_ol_iv_update(KEY_WLC(_key)->wlc_dngl_ol, \
		&(_key)->info, (_seq), KEY_SEQ_SIZE, (_seq_id), (_tx)); \
} while (0)
#else
#define KEY_OL_IV_UPDATE(_key, _seq, _seq_id, _tx)
#endif

#define KEY_RESOLVE_SEQ(_key, _ins, _tx, _ins_begin, _ins_end) {\
	_ins_begin = _ins; \
	_ins_end = (_ins) + 1; \
	if ((_ins) == WLC_KEY_SEQ_ID_ALL) {\
		KM_DBG_ASSERT(!(_tx) && !WLC_KEY_IS_MGMT_GROUP(&(_key)->info)); \
		_ins_begin = 0; \
		_ins_end = KEY_NUM_RX_SEQ(_key); \
	}\
}

#define KM_KEY_HW_IDX_TO_SLOT(_key, _hw_idx) WLC_KM_HW_IDX_TO_SLOT((_key)->wlc, _hw_idx)

#endif /* _km_key_pvt_h_ */
