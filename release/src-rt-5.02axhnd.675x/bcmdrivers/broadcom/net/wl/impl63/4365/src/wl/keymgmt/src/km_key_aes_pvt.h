/*
 * private interface for wlc_key algo 'aes'
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_aes_pvt.h 419077 2013-08-19 18:01:30Z $
 */

#ifndef km_key_aes_pvt_h_
#define km_key_aes_pvt_h_

#include "km_key_pvt.h"

#include <bcmcrypto/aes.h>
#include <bcmcrypto/aesgcm.h>

#define AES_KEY_AAD_MAX_SIZE AES_CCMP_AAD_MAX_LEN
#define AES_KEY_AAD_QOS_MASK AES_CCMP_QOS_MASK

#define AES_KEY_BIP_DATA_SZ (256 + sizeof(mmic_ie_t))

#define AES_KEY_MIN_SIZE AES_BLOCK_SZ
#define AES_KEY_MAX_SIZE (AES_BLOCK_SZ << 1)

#define AES_KEY_IV_SIZE	DOT11_IV_AES_CCM_LEN
#define AES_KEY_SEQ_SIZE KEY_SEQ_SIZE

#define AES_KEY_MIN_ICV_SIZE DOT11_ICV_AES_LEN
#define AES_KEY_MAX_ICV_SIZE AES_BLOCK_SZ

#define AES_KEY128_ALGO_VALID(_key) (\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_CCM) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_OCB_MSDU) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_OCB_MPDU) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_GCM) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_BIP) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_BIP_GMAC))

#define AES_KEY256_ALGO_VALID(_key) (\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_CCM256) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_AES_GCM256) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_BIP_CMAC256) ||\
	(((_key)->info.algo) == CRYPTO_ALGO_BIP_GMAC256))

#define AES_KEY_ALGO_VALID(_key) (\
	AES_KEY128_ALGO_VALID(_key) ||\
	AES_KEY256_ALGO_VALID(_key))

#define AES_KEY_ALGO_CCMXX(_key) (\
	((_key)->info.algo == CRYPTO_ALGO_AES_CCM) ||\
	((_key)->info.algo == CRYPTO_ALGO_AES_CCM256))

#define AES_KEY_ALGO_OCBXX(_key) (\
	((_key)->info.algo == CRYPTO_ALGO_AES_OCB_MSDU) ||\
	((_key)->info.algo == CRYPTO_ALGO_AES_OCB_MPDU))

#define AES_KEY_ALGO_GCMXX(_key) (\
	((_key)->info.algo == CRYPTO_ALGO_AES_GCM) ||\
	((_key)->info.algo == CRYPTO_ALGO_AES_GCM256))

#define AES_KEY_ALGO_BIPXX(_key) WLC_KEY_ALGO_IS_BIPXX((_key)->info.algo)

#define AES_KEY_VALID_INS(_key, _tx, _ins) (\
	((_tx) && (_ins) == 0) ||\
	(!(_tx) && (_ins) < KEY_NUM_RX_SEQ(_key)))

/* context data type for aes - except for broadcast mgmt frames.
 * seq is in LE order and key allocation extends beyond struct
 * definition to accommodate varying length keys
 * XXX when MFP is enabled, one less rx seq can be allocated for GTK.
 * however, allocating the extra rx seq makes the code simpler.
 */
struct aes_key {
	uint8 tx_seq[AES_KEY_SEQ_SIZE];
	uint8 key[AES_KEY_MIN_SIZE];
	/*
	 *  if key len is > AES_KEY_MIN_SIZE rest follows.
	 *	followed by	uint8 rx_seq[*tunable*][AES_KEY_SEQ_SIZE];
	 */
};

typedef struct aes_key aes_key_t;

/* context data type for aes igtk - used for broadcast management
 * frames. there is only one seq - used for rx or STA and tx on AP
 */
struct aes_igtk {
	uint8 seq[AES_KEY_SEQ_SIZE];
	uint8 key[1];
};
typedef struct aes_igtk aes_igtk_t;

#define AES_KEY_STRUCT_SIZE(_key) (\
	((_key)->info.flags & WLC_KEY_FLAG_MGMT_GROUP) ? \
		(OFFSETOF(aes_igtk_t, key) + ((_key)->info.key_len)) : \
		(OFFSETOF(aes_key_t, key) + \
			(_key)->info.key_len + (KEY_NUM_RX_SEQ(_key) * AES_KEY_SEQ_SIZE)))

#define AES_KEY_RX_SEQ(_key, _aes_key, _ins) ((_aes_key)->key + (_key)->info.key_len +\
	((_ins) * AES_KEY_SEQ_SIZE))

#define AES_KEY_SEQ(_key, _aes_key, _tx, _ins) ((_tx) ?\
	&(_aes_key)->tx_seq[0] : AES_KEY_RX_SEQ((_key), (_aes_key), (_ins)))

#ifdef MFP
int km_key_aes_rx_mmpdu_mcmfp(wlc_key_t *key, void *pkt, const struct dot11_header *hdr,
	uint8 *body, int body_len, const key_hw_rx_info_t *hw_rxi);
int km_key_aes_tx_mmpdu_mcmfp(wlc_key_t *key, void *pkt, const struct dot11_header *hdr,
	uint8 *body, int body_len, wlc_txd_t *txd);
#else
#define km_key_aes_rx_mmpdu_mcmfp(key, pkt, hdr, body, body_len, hw_rxi) \
	BCME_UNSUPPORTED
#define km_key_aes_tx_mmpdu_mcmfp(key, pkt, hdr, body, body_len, txd) \
	BCME_UNSUPPORTED
#endif /* MFP */

#endif /* km_key_aes_pvt_h_ */
