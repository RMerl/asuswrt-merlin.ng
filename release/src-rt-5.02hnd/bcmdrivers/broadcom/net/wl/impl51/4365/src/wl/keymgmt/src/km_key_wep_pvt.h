/*
 * private interface for wlc_key algo 'wep'
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_wep_pvt.h 419446 2013-08-21 03:19:29Z $
 */

#ifndef km_key_wep_pvt_h_
#define km_key_wep_pvt_h_

#include "km_key_pvt.h"

#include <bcmcrypto/rc4.h>
#include <bcmcrypto/wep.h>

#if defined(BCMCCX) || defined(BCMEXTCCX)
#include <bcmcrypto/ccx.h>
#include <bcmcrypto/bcmccx.h>
#endif

#define WEP_KEY_ALLOC_SIZE ROUNDUP(WEP128_KEY_SIZE, 16)
#define WEP_RC4_IV_SIZE (DOT11_IV_LEN - 1)
#define WEP_RC4_ALLOC_SIZE (WEP_RC4_IV_SIZE + WEP_KEY_ALLOC_SIZE)

#define WEP_KEY_VALID(_key) ((((_key)->info.algo == CRYPTO_ALGO_WEP1) ||\
		((_key)->info.algo == CRYPTO_ALGO_WEP128)) &&\
		((_key)->info.key_len <= WEP_KEY_ALLOC_SIZE) &&\
		((_key)->info.iv_len <= DOT11_IV_LEN))

#if defined(BCMCCX) || defined(BCMEXTCCX)
struct wep_key_ckip_state {
	uint32      tx_seq;
	uint32      rx_seq_base;		/* unicast rx seq */
	uint32      rx_seq_bitmap;		/* consumed rx seq */
    uint32      mcrx_seq_base;		/* multicast rx seq base */
    uint32      mcrx_seq_bitmap;  	/* multicast consumed rx seq */
};

typedef struct wep_key_ckip_state wep_key_ckip_state_t;
#endif /* BCMCCX or BCMEXTCCX */


/* context data type for wep. note that wep has two key sizes
 * as selected by key algo, and no replay protection. hence there
 * is no need to allocate rx seq (iv, replay counter).
 */
struct wep_key {
	uint8 key[WEP_KEY_ALLOC_SIZE];		/* key data */
	uint8 tx_seq[DOT11_IV_LEN];			/* LE order - need only 24 bits */

#if defined(BCMCCX) || defined(BCMEXTCCX)
	wep_key_ckip_state_t ckip_state;
#endif /* BCMCCX or BCMEXTCCX */
};

typedef struct wep_key wep_key_t;

#if defined(BCMCCX) || defined(BCMEXTCCX)
int km_key_wep_rx_ccx_msdu(wlc_key_t *key, void *pkt,
	struct ether_header *hdr, uint8 *body, int body_len,
	const key_hw_rx_info_t *hw_rxi);
int km_key_wep_tx_ccx_msdu(wlc_key_t *key, void *pkt,
	struct ether_header *hdr, uint8 *body, int body_len,
	size_t frag_length, uint8 prio);
void km_key_wep_ccx_dump(wep_key_ckip_state_t *ckip_state, struct bcmstrbuf *b);
#endif /*  BCMCCX || BCMEXTCCX */

#endif /* km_key_wep_pvt_h_ */
