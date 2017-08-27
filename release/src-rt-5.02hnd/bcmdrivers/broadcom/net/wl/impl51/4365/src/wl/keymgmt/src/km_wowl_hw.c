/*
 * Key Management Module km_wowl_hw Implementation
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_wowl_hw.c 556100 2015-05-12 17:38:50Z $
 */

/* This file implements the wlc keymgmt functionality. It provides
 * hardware/memory layout dependent support for wowl
 */

#ifdef WOWL

#include "km_wowl_hw.h"
#include "km_hw_impl.h"

/* wowl algo entries */
#define HAE(algo) CRYPTO_ALGO_##algo
static const key_algo_t km_wowl_hw_algos[] = {
	HAE(WEP1),
	HAE(WEP128),
	HAE(AES_CCM),
	HAE(AES_OCB_MSDU),
	HAE(AES_OCB_MPDU),
#if defined(BCMCCX) || defined(BCMEXTCCX)
	HAE(CKIP),
	HAE(CKIP_MMH),
	HAE(WEP_MMH),
#endif /* BCMCCX || BCMEXTCCX */
#ifndef LINUX_CRYPTO
	HAE(TKIP)
#endif
#ifdef BCMWAPI_WPI
#endif /* BCMWAPI_WPI */
};
#undef HAE

/* internal interface */
static void
wowl_hw_init(km_hw_t *hw)
{
	if (!KM_HW_NEED_INIT(hw))
		return;

	/* shm rx keys */
	hw->shm_info.key_base = KM_HW_SHM_ADDR_FROM_PTR(hw, M_WOWL_SECRXKEYS_PTR);

	/* shm tkip mic keys */
	hw->shm_info.tkip_mic_key_base = KM_HW_SHM_ADDR_FROM_PTR(hw, M_WOWL_TKMICKEYS_PTR);

	hw->flags |= KM_HW_INITED;
}

static int
wowl_hw_key_set(km_hw_t *hw, hw_idx_t hw_idx, wlc_key_t *key, wlc_key_info_t *key_info)
{
	int err = BCME_OK;
	skl_idx_t skl_idx;
	amt_idx_t amt_idx;
	amt_attr_t amt_attr = 0;
	uint16 skl_val;
	const km_hw_algo_entry_t* ae;

	amt_idx = KM_HW_AMT_IDX_INVALID;
	if (!KM_HW_IDX_VALID(hw, hw_idx)) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	ae = km_hw_find_algo_entry(hw, key_info->algo);
	if (ae == NULL) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	skl_idx = (skl_idx_t)hw_idx;
	KM_HW_SKL_IDX(hw, hw_idx) = skl_idx;

	if (skl_idx >= WLC_KEYMGMT_NUM_GROUP_KEYS)
		amt_idx = skl_idx - WLC_KEYMGMT_NUM_GROUP_KEYS;

#ifdef BCMDBG
	/* ensure the address programmed into amt idx belongs to the key */
	if (amt_idx != KM_HW_AMT_IDX_INVALID) {
		struct ether_addr debug_ea;
		amt_attr_t debug_amt_attr;
		wlc_get_addrmatch(KM_HW_WLC(hw), amt_idx, &debug_ea, &debug_amt_attr);
		KM_DBG_ASSERT(!memcmp(&debug_ea, &key_info->addr, sizeof(debug_ea)));
	}
#endif

	/* clear pairwise key match in amt during key related shm writes */
	if (amt_idx != KM_HW_AMT_IDX_INVALID)
		amt_attr = wlc_clear_addrmatch(KM_HW_WLC(hw), amt_idx);

	/* update secalgo */
	skl_val = (KM_HW_SKL_HW_ALGO(hw, key_info) << SKL_ALGO_SHIFT) & SKL_ALGO_MASK;
	wlc_write_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx), skl_val);

	err = km_hw_algo_set_hw_key(hw, hw_idx, ae, ae->impl.dt_mask, key, key_info);
	if (err != BCME_OK)
		goto done;

done:
	if (amt_idx != KM_HW_AMT_IDX_INVALID)
		 wlc_set_addrmatch(KM_HW_WLC(hw), amt_idx, &key_info->addr, amt_attr);

	return err;
}

static int
wowl_hw_key_get(km_hw_t *hw, hw_idx_t hw_idx, km_hw_dt_mask_t dt_mask, wlc_key_t *key,
	wlc_key_info_t *key_info)
{
	int err = BCME_OK;
	const km_hw_algo_entry_t* ae;
	skl_idx_t skl_idx;

	if (!KM_HW_IDX_VALID(hw, hw_idx)) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	ae = km_hw_find_algo_entry(hw, key_info->algo);
	if (ae == NULL) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	/* clear hw idx entry and skl, update sw key */
	skl_idx = (skl_idx_t)hw_idx;
	KM_HW_SKL_IDX(hw, hw_idx) = KM_HW_SKL_IDX_INVALID;
	wlc_write_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx), 0);
	err = km_hw_algo_update_sw_key(hw, hw_idx, ae, dt_mask, key, key_info);
	if (err != BCME_OK)
		goto done;

done:
	return err;
}

static void
wowl_hw_enable(km_hw_t *hw, scb_t *scb)
{
	int err = BCME_OK;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	wlc_bsscfg_t *bsscfg;
	uint16 secsuite = 0;
	KM_HW_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_ASSERT(scb != NULL);
	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	key = wlc_keymgmt_get_scb_key(KM_HW_KM(hw), scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);

	if (key_info.algo == CRYPTO_ALGO_OFF) {
		key = wlc_keymgmt_get_bss_tx_key(KM_HW_KM(hw), SCB_BSSCFG(scb), FALSE, &key_info);
		if (!KM_WEP_ALGO(key_info.algo)) {
			err = BCME_UNSUPPORTED;
			goto done;
		}
		err = wowl_hw_key_set(hw, key_info.key_id, key, &key_info);
		if (err != BCME_OK)
			goto done;

		secsuite |= (key_info.hw_algo << WOWL_SECSUITE_GRP_ALGO_SHIFT) &
			WOWL_SECSUITE_GRP_ALGO_MASK;
	} else {
		wlc_key_id_t key_id;

		/* set pairwise */
		err = wowl_hw_key_set(hw, WLC_KEYMGMT_NUM_GROUP_KEYS, key, &key_info);
		if (err != BCME_OK)
			goto done;

		secsuite |= (key_info.hw_algo << WOWL_SECSUITE_ALGO_SHIFT) &
			WOWL_SECSUITE_ALGO_MASK;

		for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
			key = wlc_keymgmt_get_bss_key(KM_HW_KM(hw), bsscfg, key_id, &key_info);
			err =  wowl_hw_key_set(hw, key_id, key, &key_info);
			if (err != BCME_OK) {
				goto done;
			}
		}

		secsuite |= (key_info.hw_algo << WOWL_SECSUITE_GRP_ALGO_SHIFT) &
			WOWL_SECSUITE_GRP_ALGO_MASK;
	}

	wlc_write_shm(KM_HW_WLC(hw), M_WOWL_SECSUITE, secsuite);

	wlc_mhf(KM_HW_WLC(hw), MHF1, MHF1_DEFKEYVALID, MHF1_DEFKEYVALID, WLC_BAND_ALL);
	hw->flags |= KM_HW_DEFKEYS;

done:
	if (err == BCME_OK) {
		hw->flags |= KM_HW_WOWL_ENABLED;
	}

	KM_HW_LOG(("wl%d.%d: %s: status %d, enabled %d for scb %s\n", KM_HW_UNIT(hw),
		WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err, KM_HW_WOWL_ENABLED(hw),
		bcm_ether_ntoa(&scb->ea, eabuf)));
}

static void
wowl_hw_disable(km_hw_t *hw, scb_t *scb)
{
	int err = BCME_OK;
	wlc_bsscfg_t *bsscfg;
	uint16 rot_idx_mask;
	km_hw_dt_mask_t dt_mask;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	wlc_key_id_t key_id;

	KM_ASSERT(scb != NULL);
	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	wlc_mhf(KM_HW_WLC(hw), MHF1, MHF1_DEFKEYVALID, 0, WLC_BAND_ALL);
	hw->flags &= ~KM_HW_DEFKEYS;

	dt_mask = KM_HW_DT_MASK(WLC_KEY_DATA_TYPE_SEQ);
	key = wlc_keymgmt_get_scb_key(KM_HW_KM(hw), scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);

	err = wowl_hw_key_get(hw, WLC_KEYMGMT_NUM_GROUP_KEYS, dt_mask, key, &key_info);
	if (err != BCME_OK)
		goto done;

	/* update group key seq/iv. upon key rotation, also update key data. sw computes
	 * tkip phase1 keys as necessary
	 */
	rot_idx_mask = wlc_read_shm(KM_HW_WLC(hw), M_GROUP_KEY_IDX);
	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		dt_mask = KM_HW_DT_MASK(WLC_KEY_DATA_TYPE_SEQ);
		if (rot_idx_mask & (1 << key_id)) {
			dt_mask |= KM_HW_DT_MASK(WLC_KEY_DATA_TYPE_KEY);
			if (key_info.algo == CRYPTO_ALGO_TKIP &&
				!WLC_KEY_IS_LINUX_CRYPTO(&key_info)) {
				dt_mask |= KM_HW_DT_MASK2(WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS,
					WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS);
			}
		}

		key = wlc_keymgmt_get_bss_key(KM_HW_KM(hw), bsscfg, key_id, &key_info);
		err = wowl_hw_key_get(hw, key_id, dt_mask, key, &key_info);
		if (err != BCME_OK)
			goto done;
	}

done:
	/* disable regardless of error */
	hw->flags &= ~KM_HW_WOWL_ENABLED;

	KM_HW_LOG(("wl%d: %s: status %d, enabled %d\n", KM_HW_UNIT(hw),
		__FUNCTION__, err, KM_HW_WOWL_ENABLED(hw)));
}

/* public interface */
km_hw_t*
BCMATTACHFN(km_wowl_hw_attach)(wlc_info_t *wlc, wlc_keymgmt_t *km)
{
	km_hw_t *hw;
	int err = BCME_OK;
	size_t i;

	STATIC_ASSERT(AMT_SIZE_64 >= RCMTA_SIZE);
	STATIC_ASSERT(TKIP_KEY_SIZE == 32);
	STATIC_ASSERT(WOWL_TSCPN_SIZE == KEY_SEQ_SIZE);
	STATIC_ASSERT(WOWL_TSCPN_BLK_SIZE == KM_HW_RX_SEQ_BLOCK_SIZE);

	KM_DBG_ASSERT(km != NULL && wlc != NULL);
	KM_DBG_ASSERT(wlc->keymgmt == NULL || wlc->keymgmt == km);

	hw = (km_hw_t *)MALLOCZ(wlc->osh, sizeof(km_hw_t));
	if (!hw) {
		err = BCME_NOMEM;
		goto done;
	}

	hw->wlc = wlc;
	hw->max_idx = WLC_KEYMGMT_NUM_WOWL_KEYS;
	hw->max_amt_idx = WLC_KEYMGMT_NUM_WOWL_KEYS - WLC_KEYMGMT_NUM_GROUP_KEYS;

	if (KM_HW_COREREV_GE40(hw)) {
		hw->shm_info.skl_idx_base = D11AC_M_WOWL_SECKINDXALGO_BLK;
		hw->shm_info.tkip_tsc_ttak_base = D11AC_M_WOWL_TKIP_TSC_TTAK;
		hw->shm_info.rx_pn_base = D11AC_M_WOWL_TSCPN_BLK;
		hw->shm_info.tx_pn_base = D11AC_M_WOWL_OFFLOAD_CTX;
	} else {
		hw->shm_info.skl_idx_base = M_WOWL_SECKINDXALGO_BLK;
		hw->shm_info.tkip_tsc_ttak_base = M_WOWL_TKIP_TSC_TTAK;
		hw->shm_info.rx_pn_base = M_WOWL_TSCPN_BLK;
		hw->shm_info.tx_pn_base = M_WOWL_OFFLOAD_CTX;
	}

	hw->shm_info.max_tkip_mic_keys = hw->max_idx;
	hw->shm_info.max_tsc_ttak = WLC_KEYMGMT_NUM_WOWL_KEYS;
	hw->shm_info.max_rx_pn = hw->max_idx;
	hw->shm_info.max_tx_pn = 1;

	hw->max_key_size = D11_MAX_KEY_SIZE;

	/* note: algo init needs this to be done first */
	hw->flags |= KM_HW_WOWL_HW;

	/* initialize hw/algo */
	hw->impl.num_algo_entries  = ARRAYSIZE(km_wowl_hw_algos);
	hw->impl.algo_entries = MALLOCZ(wlc->osh,
		sizeof(km_hw_algo_entry_t) * hw->impl.num_algo_entries);
	if (hw->impl.algo_entries == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	for (i = 0; i < hw->impl.num_algo_entries; ++i) {
		km_hw_algo_entry_t *ae = & hw->impl.algo_entries[i];
		ae->algo = km_wowl_hw_algos[i];
		err = km_hw_algo_init(hw, ae->algo, &ae->impl);
		KM_HW_LOG(("wl%d: %s: initialized algo %d[%s], status %d\n",
			WLCWLUNIT(wlc), __FUNCTION__,
			ae->algo, wlc_keymgmt_get_algo_name(KM_HW_KM(hw), ae->algo), err));
		/* on err hw algo will be disabled - continue */
	}

	/* init allocation */
	for (i = 0; i < hw->max_idx; ++i) {
		KM_HW_SKL_IDX(hw, i) = KM_HW_SKL_IDX_INVALID;
	}

done:
	if (err != BCME_OK)
		km_wowl_hw_detach(&hw);

	KM_HW_LOG(("wl%d: %s: done withe status %d\n",  WLCWLUNIT(wlc), __FUNCTION__, err));
	return hw;
}

void
BCMATTACHFN(km_wowl_hw_detach)(km_hw_t **hw_in)
{
	km_hw_t *hw;
	wlc_info_t *wlc = NULL;

	if (!hw_in || !*hw_in)
		goto done;

	hw = *hw_in;
	*hw_in = NULL;

	wlc = KM_HW_WLC(hw);

	 /* destroy hw/algo */
	if (hw->impl.algo_entries != NULL) {
		km_hw_algo_destroy_algo_entries(hw);
		MFREE(wlc->osh, hw->impl.algo_entries,
			sizeof(km_hw_algo_entry_t) * hw->impl.num_algo_entries);
	}

	MFREE(wlc->osh, hw, sizeof(km_hw_t));

done:
	KM_HW_LOG(("wl%d: %s: done\n", wlc != NULL ? WLCWLUNIT(wlc) : 0, __FUNCTION__));
}

void
km_wowl_hw_set_mode(km_hw_t *hw, scb_t *scb, bool enable)
{
	bool is_enabled;

	KM_DBG_ASSERT(KM_HW_VALID(hw));
	KM_DBG_ASSERT(KM_HW_WOWL_SUPPORTED(hw));

	if (KM_HW_NEED_INIT(hw))
		wowl_hw_init(hw);

	is_enabled = KM_HW_WOWL_ENABLED(hw);
	if ((is_enabled && enable) || (!is_enabled && !enable))
		goto done;

	if (enable)
		wowl_hw_enable(hw, scb);
	else
		wowl_hw_disable(hw, scb);

done:
	KM_HW_LOG(("wl%d: %s: enable %d, enabled %d\n",  KM_HW_UNIT(hw), __FUNCTION__,
		enable, KM_HW_WOWL_ENABLED(hw)));
}

#endif /* WOWL */
