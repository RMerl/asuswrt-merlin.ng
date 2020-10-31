/*
 * Key Management Module Implementation
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_hw.h 594757 2015-10-23 12:35:49Z $
 */

#ifndef _km_hw_h_
#define _km_hw_h_


#include "km.h"
#include "km_key.h"

struct km_hw;
typedef struct km_hw km_hw_t;

#define KM_HW_AMT_IDX_INVALID 0xff

typedef wlc_keymgmt_t keymgmt_t;

km_hw_t *km_hw_attach(wlc_info_t *wlc, wlc_keymgmt_t *km);
void km_hw_detach(km_hw_t **hw);
void km_hw_init(km_hw_t *hw);
void km_hw_reset(km_hw_t *hw);

void km_hw_key_create(km_hw_t *km_hw, const wlc_key_t *key,
	const wlc_key_info_t *key_info, wlc_key_hw_index_t *hw_idx);
void km_hw_key_destroy(km_hw_t *km_hw, wlc_key_hw_index_t *hw_idx,
	const wlc_key_info_t *key_info);

void km_hw_key_update(km_hw_t *km_hw, wlc_key_hw_index_t hw_idx,
	wlc_key_t *key, const wlc_key_info_t *key_info);

bool km_hw_key_hw_mic(km_hw_t *km_hw, wlc_key_hw_index_t hw_idx,
	wlc_key_info_t *key_info);

void km_hw_dump(km_hw_t *hm_hw, struct bcmstrbuf *b,
	km_key_dump_type_t dump_type);

wlc_key_hw_algo_t km_hw_algo_to_hw_algo(const km_hw_t *hw, wlc_key_algo_t algo);
bool km_hw_amt_idx_isset(km_hw_t *hw, int amt_idx);
km_amt_idx_t km_hw_amt_find_and_resrv(km_hw_t *hw);
#ifdef ACKSUPR_MAC_FILTER
km_amt_idx_t km_hw_amt_alloc_acksupr(km_hw_t *hw, scb_t *scb);
#endif /* ACKSUPR_MAC_FILTER */
km_amt_idx_t km_hw_amt_alloc(km_hw_t *hw, const struct ether_addr *ea);
void km_hw_amt_release(km_hw_t *hw, km_amt_idx_t *amt_idx);
void km_hw_amt_reserve(km_hw_t *hw, km_amt_idx_t amt_idx,
	size_t count, bool reserve);
bool km_hw_amt_idx_valid(km_hw_t *hw, km_amt_idx_t amt_idx);

#endif /* _km_hw_h_ */
