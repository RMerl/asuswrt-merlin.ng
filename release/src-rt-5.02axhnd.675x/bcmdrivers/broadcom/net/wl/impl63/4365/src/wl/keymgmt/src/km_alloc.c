/*
 * Key Management Module Implementation - allocation support
 *  Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_alloc.c 455413 2014-02-13 23:24:02Z $
 */

#include "km_pvt.h"

/* Allocation takes into consideration the layout requirements for
 * lower-level - in particular the following requirements
 *		Group keys for WLC default BSS use indicies 0..3
 *		STA group keys are contiguous - follow pairwise key
 *		IBSS peer group keys are offset by WLC_KEYMGMT_IBSS_MAX_PEERS from
		pairwise key and each other
 * Key management implementation also places a few requirements.
 *		-- STA group keys and IBSS peer group keys are allocated
 *		together
 *		-- Management group keys are allocated at high indicies
 */

/* Internal interface */
static int
km_alloc_contiguous(keymgmt_t *km, wlc_key_index_t key_idx_arr[],
	size_t num_keys, bool at_end)
{
	int err = BCME_NOMEM;
	wlc_key_index_t i, j;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(num_keys > 0 && num_keys < 65536);

	if (num_keys > km->max_keys)
		goto done;

	key_idx = WLC_KEY_INDEX_INVALID;
	i = at_end ? (km->max_keys - (uint16)num_keys) : 0;
	do {
		size_t num_found = 0;
		for (j = 0; j < num_keys && num_found < num_keys; ++j) {
			if ((i+j) >= km->max_keys)
				break;
			km_pvt_key = &km->keys[i+j];
			if (km_pvt_key->flags & KM_FLAG_IDX_ALLOC) { /* already allocated */
				++j; /* to skip to entry after this */
				break;
			}
			++num_found;
		}
		if (num_found == num_keys) {
			key_idx = i;
			break;
		}

		i = at_end ? (i - 1) : (i + j);
	} while ((at_end && i > 0) || (!at_end && i < km->max_keys));

	if (key_idx != WLC_KEY_INDEX_INVALID) {
		for (j = 0; j < num_keys; ++j, ++key_idx) {
			km_pvt_key = &km->keys[key_idx];
			km_pvt_key->flags |= KM_FLAG_IDX_ALLOC;
			key_idx_arr[j] = key_idx;
			KM_LOG(("wl%d: %s: allocated key index 0x%04x\n",
				KM_UNIT(km), __FUNCTION__, key_idx));
		}
		err = BCME_OK;
	}

done:
	return err;
}

/* External interface */
int
km_alloc_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
    wlc_key_index_t key_idx_arr[], size_t num_keys)
{
	int err = BCME_OK;
	bool at_end;
	size_t i;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_LOG(("wl%d: %s: enter flags 0x%08x num_keys %d\n",
		KM_UNIT(km), __FUNCTION__, key_flags, (int)num_keys));

	at_end = key_flags & WLC_KEY_FLAG_MGMT_GROUP;
	err = km_alloc_contiguous(km, key_idx_arr, num_keys, at_end);
	if (err == BCME_OK)
		goto done;

	/* contiguous block not available; allocate non-contiguous */
	for (i = 0; i < num_keys; ++i) {
		err = km_alloc_contiguous(km, &key_idx_arr[i], 1, at_end);
		if (err != BCME_OK)
			break;
	}

	if (err != BCME_OK && i > 0)
		km_free_key_block(km, key_flags, key_idx_arr, i);
done:
	KM_LOG(("wl%d: %s: exit status %d\n", KM_UNIT(km), __FUNCTION__, err));
	return err;
}

void
km_free_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
    wlc_key_index_t key_idx_arr[], size_t num_keys)
{
	size_t i;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(KM_VALID(km));

	KM_LOG(("wl%d: %s: enter flags 0x%08x num_keys %d\n",
		KM_UNIT(km), __FUNCTION__, key_flags, (int)num_keys));

	(void)key_flags;

	for (i = 0; i < num_keys; ++i) {
		key_idx = key_idx_arr[i];
		if (key_idx == WLC_KEY_INDEX_INVALID)
			continue;

		KM_LOG(("wl%d: %s: freeing key index 0x%04x\n",
			KM_UNIT(km), __FUNCTION__, key_idx));

		km_pvt_key = &km->keys[key_idx];

		/* must have been allocated */
		KM_ASSERT((km_pvt_key->flags & KM_FLAG_IDX_ALLOC) != 0);

		if (km_pvt_key->key != NULL) {
			KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
			KM_ASSERT(KM_VALID_KEY(&km->keys[key_idx]));
			km_key_destroy(&km_pvt_key->key);
			KM_ASSERT_KEY_DESTROYED(km, key_idx);
		}

		km_pvt_key->flags = KM_FLAG_NONE;
		key_idx_arr[i] = WLC_KEY_INDEX_INVALID;
	}
}

#ifndef BCM_OL_DEV
void
km_get_alloc_key_info(wlc_keymgmt_t *km, wlc_key_index_t key_idx,
	km_alloc_key_info_t *alloc_key_info)
{
	km_pvt_key_t *km_pvt_key;
	wlc_bsscfg_t *bsscfg = NULL;
	scb_t *scb = NULL;
	km_bsscfg_t *bss_km = NULL;
	km_scb_t *scb_km = NULL;
	wlc_key_id_t key_id;

	STATIC_ASSERT(WLC_KEYMGMT_NUM_STA_GROUP_KEYS == 2);

	KM_DBG_ASSERT(KM_VALID(km) && alloc_key_info != NULL &&
		KM_VALID_KEY_IDX(km, key_idx));

	KM_LOG(("wl%d: %s: key idx 0x%04x, alloc info@%p\n", KM_UNIT(km), __FUNCTION__,
		key_idx, alloc_key_info));

	/* lookup bsscfg and scb */
	km_pvt_key = &km->keys[key_idx];
	if (km_pvt_key->flags & KM_FLAG_BSS_KEY) {
		bsscfg = km_pvt_key->u.bsscfg;
		KM_DBG_ASSERT(bsscfg != NULL);
		bss_km = KM_BSSCFG(km, bsscfg);
		if (bss_km->scb_key_idx != WLC_KEY_INDEX_INVALID) {
			key_idx = bss_km->scb_key_idx;
			KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
			km_pvt_key = &km->keys[key_idx];
			scb = km_pvt_key->u.scb;
			KM_DBG_ASSERT(scb != NULL && bsscfg == SCB_BSSCFG(scb));
		}
	} if (km_pvt_key->flags & KM_FLAG_SCB_KEY) {
		scb = km_pvt_key->u.scb;
		KM_DBG_ASSERT(scb != NULL);
		bsscfg = SCB_BSSCFG(scb);
		KM_DBG_ASSERT(bsscfg != NULL);
		bss_km = KM_BSSCFG(km, bsscfg);
	}

	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		alloc_key_info->bss_info.key_idx[key_id] = WLC_KEY_INDEX_INVALID;
		alloc_key_info->bss_info.hw_idx[key_id] = WLC_KEY_INDEX_INVALID;
		if (bsscfg != NULL) {
			key_idx = bss_km->key_idx[key_id];
			alloc_key_info->bss_info.key_idx[key_id] = key_idx;
			alloc_key_info->bss_info.hw_idx[key_id] = km_get_hw_idx(km, key_idx);
		}

#if defined(IBSS_PEER_GROUP_KEY)
		alloc_key_info->scb_info.ibss_info.key_idx[key_id] = WLC_KEY_INDEX_INVALID;
		alloc_key_info->scb_info.ibss_info.hw_idx[key_id] = WLC_KEY_INDEX_INVALID;
#endif /* IBSS_PEER_GROUP_KEY */
	}

	alloc_key_info->scb_info.key_idx = WLC_KEY_INDEX_INVALID;
	alloc_key_info->scb_info.hw_idx = WLC_KEY_INDEX_INVALID;

	if (scb != NULL) {
		scb_km = KM_SCB(km, scb);
		key_idx = scb_km->key_idx;
		alloc_key_info->scb_info.key_idx = key_idx;
		alloc_key_info->scb_info.hw_idx = km_get_hw_idx(km, key_idx);
#if defined(IBSS_PEER_GROUP_KEY)
		if (KM_IBSS_PGK_ENABLED(km) && !bsscfg->BSS) {
			key_idx = scb_km->ibss_info.key_idx[0];
			alloc_key_info->scb_info.ibss_info.key_idx[0] = key_idx;
			alloc_key_info->scb_info.ibss_info.hw_idx[0] = km_get_hw_idx(km, key_idx);

			key_idx = scb_km->ibss_info.key_idx[1];
			alloc_key_info->scb_info.ibss_info.key_idx[1] = key_idx;
			alloc_key_info->scb_info.ibss_info.hw_idx[1] = km_get_hw_idx(km, key_idx);
		}
#endif /* IBSS_PEER_GROUP_KEY */
	}

	alloc_key_info->scb_info.scb = scb;
	alloc_key_info->bss_info.bsscfg = bsscfg;
	alloc_key_info->bss_info.amt_idx = bss_km->amt_idx;
}
#endif /* !BCM_OL_DEV */
