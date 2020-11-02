/*
 * Internal interface to BRCMAPIVTW support
 * Copyright (c) 2013 Broadcom Corporation, All rights reserved.
 * $Id: km_ivtw.h 778962 2019-09-16 11:01:31Z $
 */

#ifndef _km_ivtw_h_
#define _km_ivtw_h_

#include "km.h"

typedef struct km_ivtw km_ivtw_t;

km_ivtw_t* km_ivtw_attach(wlc_info_t *wlc, wlc_keymgmt_t *km);
void km_ivtw_detach(km_ivtw_t **ivtw);

int km_ivtw_get_mode(km_ivtw_t *ivtw);
int km_ivtw_set_mode(km_ivtw_t *ivtw, int val);

int km_ivtw_enable(km_ivtw_t *ivtw, wlc_key_index_t key_idx, bool enable);

bool km_ivtw_is_replay(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
	uint8 *key_seq, const uint8 *rx_seq, size_t seq_len);

int km_ivtw_reset(km_ivtw_t *ivtw, wlc_key_index_t key_idx);

void km_ivtw_update(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
    const uint8 *rx_seq, size_t seq_len);

int km_ivtw_set(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
    const uint8 *rx_seq, size_t seq_len);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void km_ivtw_dump(km_ivtw_t *ivtw, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP */

#endif /* _km_ivtw_h_ */
