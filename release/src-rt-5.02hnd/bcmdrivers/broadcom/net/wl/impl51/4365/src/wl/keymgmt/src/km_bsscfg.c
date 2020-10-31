/*
 * Key Management Module Implementation - bsscfg support
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_bsscfg.c 672672 2016-11-29 10:58:39Z $
 */

#include "km_pvt.h"
#include "km_hw_impl.h"

#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif /* WLOFFLD */

/* Internal interface */
#ifndef BCM_OL_DEV
static void
km_bsscfg_sync_bssid(keymgmt_t *km, wlc_bsscfg_t *bsscfg);
#endif /* !BCM_OL_DEV */

static void
km_bsscfg_cleanup(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;

	KM_LOG(("wl%d.%d: %s: enter\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));

	bss_km = KM_BSSCFG(km, bsscfg);
	bss_km->flags |= KM_BSSCFG_FLAG_CLEANUP;

	km_free_key_block(km, WLC_KEY_FLAG_NONE, bss_km->key_idx,
		WLC_KEYMGMT_NUM_GROUP_KEYS);

#ifdef MFP
	{
		wlc_key_id_t key_id;

		bss_km->igtk_tx_key_id = WLC_KEY_ID_INVALID;
		for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
			if (bss_km->igtk_key_idx[KM_BSSCFG_IGTK_IDX_POS(key_id)] ==
				WLC_KEY_INDEX_INVALID) {
				continue;
			}

			km_free_key_block(km, WLC_KEY_FLAG_NONE,
				&bss_km->igtk_key_idx[KM_BSSCFG_IGTK_IDX_POS(key_id)], 1);
			KM_DBG_ASSERT(bss_km->igtk_key_idx[KM_BSSCFG_IGTK_IDX_POS(key_id)] ==
				WLC_KEY_INDEX_INVALID);
		}
	}
#endif /* MFP */

#ifdef STA
	/* clean up b4m4 keys, if applicable */
	if (KM_BSSCFG_B4M4_ENABLED(bss_km))
		km_b4m4_reset_keys(km, bsscfg);
#endif /* STA */

	/* bss_km->sta_key_idx will be cleared below */

#ifndef BCM_OL_DEV
	km_bsscfg_sync_bssid(km, bsscfg);
#endif /* !BCM_OL_DEV */

	memset(bss_km, 0, sizeof(*bss_km));
	KM_LOG(("wl%d.%d: %s: exit\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));
}

static int
km_bsscfg_init_internal(wlc_keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	int err = BCME_OK;
	km_bsscfg_t *bss_km;
	wlc_key_index_t key_idx_arr[WLC_KEYMGMT_NUM_GROUP_KEYS];
	wlc_key_index_t key_idx;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	wlc_key_flags_t alloc_flags;
	km_pvt_key_t *km_pvt_key;
	wlc_key_id_t key_id;
	size_t num_keys;
	km_flags_t km_flags;

	KM_LOG(("wl%d.%d: %s: enter\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));

	bss_km = KM_BSSCFG(km, bsscfg);
	bss_km->wsec = bsscfg->wsec;
	bss_km->tx_key_id = WLC_KEY_ID_INVALID;
	bss_km->flags = KM_BSSCFG_FLAG_NONE;
	bss_km->flags |= (bsscfg->wsec & WSEC_SWFLAG) ? KM_BSSCFG_FLAG_SWKEYS : 0;
	bss_km->algo = CRYPTO_ALGO_NONE;
	bss_km->amt_idx = KM_HW_AMT_IDX_INVALID;
	bss_km->cfg_amt_idx = KM_HW_AMT_IDX_INVALID;
	bss_km->tkip_cm_detected = -(WPA_TKIP_CM_DETECT + 1);

	/* initialize key indicies so bailing on errors is safe */
	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		bss_km->key_idx[key_id] = WLC_KEY_INDEX_INVALID;
		key_idx_arr[key_id] = WLC_KEY_INDEX_INVALID;
	}

#ifdef MFP
	bss_km->igtk_tx_key_id = WLC_KEY_ID_INVALID;
	for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
		bss_km->igtk_key_idx[KM_BSSCFG_IGTK_IDX_POS(key_id)]
				= WLC_KEY_INDEX_INVALID;
	}
#endif /* MFP */

	bss_km->scb_key_idx = WLC_KEY_INDEX_INVALID;

	/* reserve a key block for bss keys, not all of them are used by
	 * all features. PSTA does not use them, STA uses only 2 group keys
	 */
	alloc_flags = WLC_KEY_FLAG_NONE;
	num_keys = WLC_KEYMGMT_NUM_GROUP_KEYS;
	alloc_flags |= WLC_KEY_FLAG_GROUP;
	alloc_flags |= KM_WLC_DEFAULT_BSSCFG(km, bsscfg) ?
		WLC_KEY_FLAG_DEFAULT_BSS : 0;
	if (BSSCFG_AP(bsscfg))
		alloc_flags |= WLC_KEY_FLAG_AP;

	err = km_alloc_key_block(km, alloc_flags, key_idx_arr, num_keys);
	if (err != BCME_OK)
		goto done;

	/* create bss keys, as necessary */
	for (key_id = 0; key_id < num_keys; ++key_id) {
		key_idx = key_idx_arr[key_id];
		if (key_idx == WLC_KEY_INDEX_INVALID)
			continue;

		/* create the key */
		memset(&key_info, 0, sizeof(key_info));
		key_info.key_idx = key_idx;
		/* address NULL for group keys */

		/* set key flags. All default keys can be used for RX on a STA , but
		 * tx key is designated and only on AP. This  may be changed
		 * later if key algo changes to WEP
		 */
		key_info.flags = alloc_flags;
		if (BSSCFG_STA(bsscfg))
			key_info.flags |= WLC_KEY_FLAG_RX;
		if (BSSCFG_AP(bsscfg) && (key_id == bss_km->tx_key_id))
			key_info.flags |= WLC_KEY_FLAG_TX;
		if (KM_BSSCFG_IS_IBSS(bsscfg))
			key_info.flags |= (WLC_KEY_FLAG_IBSS | WLC_KEY_FLAG_TX);

		key_info.key_id = key_id;
		err = km_key_create(km, &key_info, &key);
		if (err != BCME_OK) {
			KM_ERR(("wl%d.%d: %s: key create status %d\n",  KM_UNIT(km),
				WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
			km_free_key_block(km, WLC_KEY_FLAG_NONE, &key_idx_arr[key_id], 1);
			continue; /* so we initialize all bss keys */
		}

		/* assign the key to  BSS and key table */
		bss_km->key_idx[key_id] = key_idx;

		km_pvt_key = &km->keys[key_idx];
		km_flags = KM_FLAG_NONE;
		if (key_info.flags & WLC_KEY_FLAG_TX)
			km_flags |= KM_FLAG_TX_KEY;
		km_flags |= KM_FLAG_BSS_KEY;
		if (bss_km->flags & KM_BSSCFG_FLAG_SWKEYS)
			km_flags |= KM_FLAG_SWONLY_KEY;

		km_init_pvt_key(km, km_pvt_key, CRYPTO_ALGO_NONE, key, km_flags, bsscfg, NULL);
	}

#ifdef MFP
	if (KM_MFP_ENAB(km)) {
		int mfp_err = BCME_OK;

		if (BSSCFG_PSTA(bsscfg))
			goto mfp_done;

		num_keys = WLC_KEYMGMT_NUM_BSS_IGTK;
		alloc_flags = WLC_KEY_FLAG_NONE;
		if (BSSCFG_AP(bsscfg))
			alloc_flags |= WLC_KEY_FLAG_AP;

		alloc_flags |= WLC_KEY_FLAG_MGMT_GROUP;
		alloc_flags |= KM_WLC_DEFAULT_BSSCFG(km, bsscfg) ?
			WLC_KEY_FLAG_DEFAULT_BSS : 0;

		mfp_err = km_alloc_key_block(km, alloc_flags, key_idx_arr, num_keys);
		if (mfp_err != BCME_OK)
			goto done;

		/* create bss igtk default keys */
		for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
			km_pvt_key_t *km_pvt_key;
			size_t pos = KM_BSSCFG_IGTK_IDX_POS(key_id);

			key_idx = key_idx_arr[pos];
			if (key_idx == WLC_KEY_INDEX_INVALID)
				continue;

			/* create the key */
			memset(&key_info, 0, sizeof(key_info));
			key_info.key_idx = key_idx;
			/* address is NULL for default keys */
			key_info.flags = alloc_flags;

			/* IGTK is used only for TX on AP, and RX on non-AP STA */
			if (BSSCFG_AP(bsscfg)) {
				if (key_id == bss_km->igtk_tx_key_id)
					 key_info.flags |= WLC_KEY_FLAG_TX;
			} else {
				key_info.flags |= WLC_KEY_FLAG_RX;
			}
			key_info.key_id = key_id;
			mfp_err = km_key_create(km, &key_info, &key);
			if (mfp_err != BCME_OK) {
				KM_ERR(("wl%d.%d: %s: igtk key create status %d\n",
					KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
					mfp_err));
				km_free_key_block(km, WLC_KEY_FLAG_NONE, &key_idx_arr[pos], 1);
				if (err == BCME_OK)
					err = mfp_err;
				continue; /* so we initialize all default keys */
			}

			/* assign the key to  BSS and key table */
			bss_km->igtk_key_idx[pos] = key_idx;
			km_pvt_key = &km->keys[key_idx];
			km_flags = KM_FLAG_NONE;
			if (key_info.flags & WLC_KEY_FLAG_TX)
				km_flags |= KM_FLAG_TX_KEY;

			km_flags |= KM_FLAG_BSS_KEY;
			/* mfp keys are sw-only for now */
			km_flags |= KM_FLAG_SWONLY_KEY;

			km_init_pvt_key(km, km_pvt_key, CRYPTO_ALGO_NONE,
				key, km_flags, bsscfg, NULL);
		}
	}

mfp_done:

#endif /* MFP */

done:
	KM_LOG(("wl%d.%d: %s: exit status %d\n",  KM_UNIT(km),
		WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
	return err;
}

#ifndef BCM_OL_DEV
static uint32
km_bsscfg_sync_wsec(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;
	uint32 wsec;

	wsec = bsscfg->wsec;
	bss_km = KM_BSSCFG(km, bsscfg);

	if (wsec == bss_km->wsec)
		goto done;

	/* if transitioning to open or from open, scb key state needs to be reset.
	 * this allows lazy allocation of key blocks and not waste keys for scbs
	 * belonging to open bss.
	 */
	if (!bss_km->wsec || !wsec) {
		scb_t *scb;
		struct scb_iter scbiter;
		FOREACH_BSS_SCB(km->wlc->scbstate, &scbiter, bsscfg, scb) {
			km_scb_reset(km, scb);
		}
	}

	KM_SWAP(uint32, wsec, bss_km->wsec);
	bss_km->flags &= ~KM_BSSCFG_FLAG_SWKEYS;
	bss_km->flags |= (bss_km->wsec & WSEC_SWFLAG) ? KM_BSSCFG_FLAG_SWKEYS : 0;

done:
	return wsec;
}

static void
km_bsscfg_sync_bssid(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;

	bss_km = KM_BSSCFG(km, bsscfg);
	if (KM_WLC_DEFAULT_BSSCFG(km, bsscfg) || MCNX_ENAB(KM_PUB(km)) ||
		!BSSCFG_STA(bsscfg) || !bsscfg->BSS) {
		goto done;
	}

	/* just in case bssid has changed, release and alloc another */
	if (bss_km->amt_idx != KM_HW_AMT_IDX_INVALID)
		km_hw_amt_release(km->hw, &bss_km->amt_idx);

	KM_DBG_ASSERT(bss_km->amt_idx == KM_HW_AMT_IDX_INVALID);

	/* when cleaning up or bssid is cleared, disable bssid filtering if there is
	 * no non-default bss with associated stations
	 */
	if ((bss_km->flags & KM_BSSCFG_FLAG_CLEANUP) ||
		ETHER_ISNULLADDR(&bsscfg->BSSID)) {
		uint i;
		wlc_bsscfg_t *bsscfg2;
		bool clear_mhf4;

		clear_mhf4 = TRUE;
		FOREACH_AS_STA(km->wlc, i, bsscfg2) {
			if (bsscfg2->BSS && bsscfg != bsscfg2 &&
				!KM_WLC_DEFAULT_BSSCFG(km, bsscfg2)) {
				clear_mhf4 = FALSE;
				break;
			}
		}

		if (clear_mhf4)
			wlc_mhf(km->wlc, MHF4, MHF4_RCMTA_BSSID_EN, 0, WLC_BAND_ALL);
		goto done;
	}

	/* reserve an amt for this bssid, update amt attributes */
	bss_km->amt_idx = km_hw_amt_alloc(km->hw, &bsscfg->BSSID);
	if (bss_km->amt_idx == KM_HW_AMT_IDX_INVALID) {
		KM_ERR(("wl%d.%d: %s: amt idx alloc failed\n", KM_UNIT(km),
			WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		goto done;
	}

	/* Need to clear amt first to prevent double linking CFP flow ID */
	if (isset(km->hw->used, bss_km->amt_idx)) {
		wlc_clear_addrmatch(km->wlc, bss_km->amt_idx);
	}
	/* set A3 to indicate bssid. km_hw only sets A2 as required */
	wlc_set_addrmatch(km->wlc, bss_km->amt_idx, &bsscfg->BSSID,
		AMT_ATTR_VALID | AMT_ATTR_A3 | AMT_ATTR_A2);
	wlc_mhf(km->wlc, MHF4, MHF4_RCMTA_BSSID_EN, MHF4_RCMTA_BSSID_EN, WLC_BAND_ALL);

done:
	KM_LOG(("wl%d.%d: %s: amt idx %d\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__, (int)bss_km->amt_idx));
}

#ifdef WLOFFLD
void
km_bsscfg_sync_arm_tx(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	wlc_key_id_t key_id;
	bool arm_tx;

	arm_tx = wlc_ol_get_arm_txtstatus(km->wlc->ol);
	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		wlc_key_t *key;
		wlc_key_info_t key_info;

		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
		if (arm_tx)
			key_info.flags |= WLC_KEY_FLAG_ARM_TX_ENABLED;
		else
			key_info.flags &= ~WLC_KEY_FLAG_ARM_TX_ENABLED;

		km_key_set_flags(key, key_info.flags);
	}
}
#endif /* WLOFFLD */

#ifdef WOWL
static void
km_bsscfg_wowl(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool going_down)
{
	km_bsscfg_t *bss_km;
	bss_km = KM_BSSCFG(km, bsscfg);
	if (going_down)
		bss_km->flags |= KM_BSSCFG_FLAG_WOWL_DOWN;
	else
		bss_km->flags &= ~KM_BSSCFG_FLAG_WOWL_DOWN;
}
#endif

km_amt_idx_t
km_bsscfg_get_amt_idx(keymgmt_t *km, const wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;
	km_amt_idx_t amt_idx;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	amt_idx = KM_HW_AMT_IDX_INVALID;
	if (!BSSCFG_STA(bsscfg))
		goto done;

	if (KM_IS_DEFAULT_BSSCFG(km, bsscfg) && KM_COREREV_GE40(km) &&
		!PSTA_ENAB(KM_PUB(km)) && KM_STA_USE_BSSID_AMT(km)) {
		amt_idx = AMT_IDX_BSSID;
		goto done;
	}

#ifdef WLMCNX
	if (MCNX_ENAB(KM_PUB(km))) {
		amt_idx = (km_amt_idx_t)wlc_mcnx_rcmta_bssid_idx(KM_MCNX(km), bsscfg);
		if (km_hw_amt_idx_valid(km->hw, amt_idx))
			goto done;
	}
#endif /* WLMCNX */

#ifdef PSTA
	/* amt reservation support for psta */
	if (PSTA_ENAB(KM_PUB(km))) {
		/* Don't reserve in repeater mode then real-MAC bsscfg can use 0-24
		 * for amt_idx.
		 */
		if (!PSTA_IS_REPEATER(KM_WLC(km)))
			km_hw_amt_reserve(km->hw, PSTA_TA_STRT_INDX, PSTA_RA_PRIM_INDX, TRUE);
		km_hw_amt_reserve(km->hw, PSTA_RA_PRIM_INDX, 1, TRUE);

		amt_idx =  (km_amt_idx_t)wlc_psta_rcmta_idx(KM_PSTA(km), bsscfg);
		if (km_hw_amt_idx_valid(km->hw, amt_idx)) {
			km_hw_amt_reserve(km->hw, amt_idx, 1, TRUE);
			goto done;
		}
	}
#endif /* PSTA */

	bss_km = KM_BSSCFG(km, bsscfg);
#if defined(WET) || defined(WET_DONGLE)
	/* amt reservation support for wet */
	if (WET_ENAB(km->wlc) || WET_DONGLE_ENAB(km->wlc)) {
		km_hw_amt_reserve(km->hw, WET_TA_STRT_INDX, WET_RA_PRIM_INDX, TRUE);
		km_hw_amt_reserve(km->hw, WET_RA_PRIM_INDX, 1, TRUE);
		/* To support URE_MBSS, need more amt_idx for other bsscfg,
		 * not just only WET_RA_STRT_INDX
		 */
		if (bss_km->cfg_amt_idx == KM_HW_AMT_IDX_INVALID)
			bss_km->cfg_amt_idx = km_hw_amt_find_and_resrv(km->hw);

		amt_idx = bss_km->cfg_amt_idx;

		if (km_hw_amt_idx_valid(km->hw, amt_idx)) {
			goto done;
		}
	}
#endif /* WET || WET_DONGLE */

	amt_idx = bss_km->amt_idx;

done:
	return amt_idx;
}

#endif /* !BCM_OL_DEV */

/* public interface */

int
km_bsscfg_init(void *ctx, wlc_bsscfg_t *bsscfg)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	memset(bss_km, 0, sizeof(*bss_km));
	return km_bsscfg_init_internal(km, bsscfg);
}

void
km_bsscfg_deinit(void *ctx,  wlc_bsscfg_t *bsscfg)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	km_bsscfg_cleanup(km, bsscfg);
}

#ifndef BCM_OL_DEV
void
km_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_bsscfg_t *bss_km;
	wlc_keymgmt_notif_t notif;

	KM_DBG_ASSERT(KM_VALID(km));
	if (evt_data == NULL || evt_data->bsscfg == NULL) {
		KM_DBG_ASSERT(evt_data != NULL && evt_data->bsscfg != NULL);
		return;
	}

	notif = WLC_KEYMGMT_NOTIF_NONE;
	bss_km = KM_BSSCFG(km, evt_data->bsscfg);
	if (KM_BSS_IS_UP(bss_km)) {
		 if (!evt_data->up)
			notif = WLC_KEYMGMT_NOTIF_BSS_DOWN;
	} else {
		if (evt_data->up)
			notif = WLC_KEYMGMT_NOTIF_BSS_UP;
	}

	if (notif == WLC_KEYMGMT_NOTIF_NONE)
		goto done;

	km_notify(km, notif, evt_data->bsscfg, NULL, NULL, NULL);
	if (notif == WLC_KEYMGMT_NOTIF_BSS_UP)
		bss_km->flags |= KM_BSSCFG_FLAG_UP;
	else
		bss_km->flags &= ~KM_BSSCFG_FLAG_UP;

done:
	KM_LOG(("wl%d.%d: %s: exit up: %d, notif: %d \n",  KM_UNIT(km),
		WLC_BSSCFG_IDX(evt_data->bsscfg), __FUNCTION__, evt_data->up, notif));
}
#endif /* BCM_OL_DEV */

void km_bsscfg_reset(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool all)
{
	wlc_key_index_t key_idx;
	wlc_key_id_t key_id;
	km_bsscfg_t *bss_km;
	km_pvt_key_t *km_pvt_key;
	wlc_key_info_t key_info;
	int err;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	KM_LOG(("wl%d.%d: %s: enter\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));

	if (all) {
		/* reset tx key id */
		err = wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, 0, FALSE);
		if (err !=  BCME_OK)
		{
			KM_ERR(("wl%d.%d: %s: wlc_keymgmt_set_bss_tx_key_id status %d\n",
				KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
			KM_DBG_ASSERT(0);
		}
	}

	bss_km = KM_BSSCFG(km, bsscfg);
	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		key_idx = bss_km->key_idx[key_id];
		if (key_idx == WLC_KEY_INDEX_INVALID)
			continue;

		KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
		KM_DBG_ASSERT(KM_VALID_KEY(&km->keys[key_idx]));

		km_pvt_key = &km->keys[key_idx];

		/* update any flags to take bsscfg change into account  */
		wlc_key_get_info(km_pvt_key->key, &key_info);

		bss_km->flags &= ~KM_BSSCFG_FLAG_SWKEYS;
		bss_km->flags |= (bsscfg->wsec & WSEC_SWFLAG) ? KM_BSSCFG_FLAG_SWKEYS : 0;

		km_pvt_key->flags &= ~KM_FLAG_SWONLY_KEY;
		km_pvt_key->flags |= (bss_km->flags & KM_BSSCFG_FLAG_SWKEYS) ?
			KM_FLAG_SWONLY_KEY : 0;
		key_info.flags &= ~WLC_KEY_FLAG_TX;
		if (key_id == bss_km->tx_key_id) {
			if (BSSCFG_AP(bsscfg) ||
				(bsscfg->WPA_auth == WPA_AUTH_DISABLED) ||
				KM_BSSCFG_IS_BSS(bsscfg)) {
				key_info.flags |= WLC_KEY_FLAG_TX;
			}
		}

		if (KM_BSSCFG_IS_IBSS(bsscfg))
			key_info.flags |= (WLC_KEY_FLAG_IBSS | WLC_KEY_FLAG_TX);
		else
			key_info.flags &= ~WLC_KEY_FLAG_IBSS;

		key_info.flags &= ~WLC_KEY_FLAG_NO_REPLAY_CHECK;
		if (KM_BSSCFG_IS_IBSS(bsscfg) && (bsscfg->WPA_auth == WPA_AUTH_NONE)) {
			 key_info.flags |= WLC_KEY_FLAG_NO_REPLAY_CHECK;
		}

		km_key_set_flags(km_pvt_key->key, key_info.flags);

		if (km_pvt_key->key_algo != CRYPTO_ALGO_OFF) {
			bool clear_key = all;

			/* always clear wpa keys on infra sta */
			if (!BSSCFG_AP(bsscfg) && KM_BSSCFG_IS_BSS(bsscfg) &&
				(bsscfg->WPA_auth != WPA_AUTH_DISABLED) &&
				!(bsscfg->WPA_auth & WPA_AUTH_NONE)) {
				clear_key = TRUE;
			}

			if (clear_key) {
				wlc_key_set_data(km_pvt_key->key, km_pvt_key->key_algo, NULL, 0);
			} else {
				/* reevaluate h/w key - e.g. default wep */
				km_notify(km, WLC_KEYMGMT_NOTIF_KEY_UPDATE, bsscfg, NULL,
					km_pvt_key->key, NULL);
			}
		}
	}

#ifdef MFP
	if (all) {
		/* reset igtk tx key id */
		err = wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, WLC_KEY_ID_IGTK_1, TRUE);
		if (err !=  BCME_OK)
		{
			KM_ERR(("wl%d.%d: %s: wlc_keymgmt_set_bss_tx_key_id status %d\n",
				KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
			KM_DBG_ASSERT(FALSE);
		}
	}

	for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
		size_t pos = KM_BSSCFG_IGTK_IDX_POS(key_id);
		key_idx = bss_km->igtk_key_idx[pos];
		if (key_idx == WLC_KEY_INDEX_INVALID)
			continue;

		km_pvt_key = &km->keys[key_idx];
		wlc_key_get_info(km_pvt_key->key, &key_info);
		if (km_pvt_key->key_algo != CRYPTO_ALGO_OFF) {
			wlc_key_set_data(km_pvt_key->key, km_pvt_key->key_algo, NULL, 0);
		}

		if (KM_BSSCFG_IS_IBSS(bsscfg))
			key_info.flags |= WLC_KEY_FLAG_IBSS;
		else
			key_info.flags &= ~WLC_KEY_FLAG_IBSS;

		km_key_set_flags(km_pvt_key->key, key_info.flags);
	}
#endif /* MFP */

	/* reset b4m4 state */
	bss_km->flags &= ~(KM_BSSCFG_FLAG_M1RX | KM_BSSCFG_FLAG_M4TX);

	KM_LOG(("wl%d.%d: %s: exit\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));
}

void km_bsscfg_reset_sta_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wlc_key_info_t *scb_key_info)
{
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	KM_LOG(("wl%d.%d: %s: enter\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__));

	bss_km = KM_BSSCFG(km, bsscfg);
	bss_km->scb_key_idx = WLC_KEY_INDEX_INVALID;

	if (!KM_BSSCFG_NEED_STA_GROUP_KEYS(km, bsscfg))
		goto done;

	if (scb_key_info != NULL)
		bss_km->scb_key_idx = scb_key_info->key_idx;

	km_bsscfg_reset(km, bsscfg, FALSE);

done:
	KM_LOG(("wl%d.%d: %s: exit\n",  KM_UNIT(km),
		WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
}

wlc_key_t*
wlc_keymgmt_get_bss_key(keymgmt_t *km, const wlc_bsscfg_t *bsscfg,
	wlc_key_id_t key_id, wlc_key_info_t *key_info)
{
	wlc_key_index_t key_idx = WLC_KEY_INDEX_INVALID;
	wlc_key_t *key;
	const km_bsscfg_t *bss_km;
	size_t pos;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	key = km->null_key;
	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	if (bss_km == NULL)
		goto done;

	if (KM_VALID_DATA_KEY_ID(key_id)) {
		pos = KM_BSSCFG_GTK_IDX_POS(km, bsscfg, key_id);
		key_idx = bss_km->key_idx[pos];
	}

#ifdef MFP
	else if (KM_VALID_MGMT_KEY_ID(key_id)) {
		pos = KM_BSSCFG_IGTK_IDX_POS(key_id);
		key_idx = bss_km->igtk_key_idx[pos];
	}
#endif
	if (key_idx == WLC_KEY_INDEX_INVALID)
		goto done;

	KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
	KM_DBG_ASSERT(KM_VALID_KEY(&km->keys[key_idx]));
	key = km->keys[key_idx].key;

done:
	wlc_key_get_info(key, key_info);
	return key;
}

wlc_key_t*
wlc_keymgmt_get_bss_tx_key(keymgmt_t *km, const wlc_bsscfg_t *bsscfg,
	bool igtk, wlc_key_info_t *key_info)
{
	wlc_key_id_t key_id;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	key_id = wlc_keymgmt_get_bss_tx_key_id(km, bsscfg, igtk);
	return wlc_keymgmt_get_bss_key(km, bsscfg, key_id, key_info);
}

wlc_key_id_t
wlc_keymgmt_get_bss_tx_key_id(keymgmt_t *km,
	const wlc_bsscfg_t *bsscfg, bool igtk)
{
	const km_bsscfg_t *bss_km;
	wlc_key_id_t key_id;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	key_id = WLC_KEY_ID_INVALID;
	if (!bsscfg->wsec)		/* security not configured */
		goto done;

	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	if (bss_km == NULL)
		goto done;

	if (igtk) {
#ifdef MFP
		key_id = bss_km->igtk_tx_key_id;
#endif
	} else {
		key_id = bss_km->tx_key_id;
	}
done:
	return key_id;
}

int
wlc_keymgmt_set_bss_tx_key_id(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wlc_key_id_t key_id, bool igtk)
{
	km_bsscfg_t *bss_km;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	wlc_key_id_t prev_key_id;
	km_pvt_key_t *km_pvt_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	if (bss_km == NULL) {
		err = BCME_BADARG;
		goto done;
	}

	prev_key_id = WLC_KEY_ID_INVALID;
	if (igtk) {
#ifdef MFP
		if (KM_VALID_MGMT_KEY_ID(key_id)) {
			prev_key_id =  bss_km->igtk_tx_key_id;
			bss_km->igtk_tx_key_id = key_id;
		} else
			err = BCME_BADARG;
#else
		err = BCME_UNSUPPORTED;
#endif /* MFP */
	} else {
		if (KM_VALID_DATA_KEY_ID(key_id)) {
			prev_key_id = bss_km->tx_key_id;
			bss_km->tx_key_id = key_id;
		} else
			err = BCME_BADARG;
	}

	if (err != BCME_OK)
		goto done;

	/* could bail if prev & cur are same, but take this opportunity to fix up */

	/* now update TX flag - add to new and remove from current, if any */
	key  = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);

	if (!(KM_VALID_KEY_IDX(km, key_info.key_idx))) {
		KM_ERR(("wl%d.%d: %s:key id %02x, key index %d is out of range.\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, key_id, key_info.key_idx));
		goto done;
	}

	km_pvt_key = &km->keys[key_info.key_idx];
	km_pvt_key->flags |= KM_FLAG_TX_KEY;
	km_key_set_flags(key, key_info.flags | WLC_KEY_FLAG_TX);

	if (prev_key_id == key_id || prev_key_id == WLC_KEY_ID_INVALID)
		goto done;

	key = wlc_keymgmt_get_bss_key(km, bsscfg, prev_key_id, &key_info);
	km_pvt_key = &km->keys[key_info.key_idx];
	km_pvt_key->flags &= ~KM_FLAG_TX_KEY;
	km_key_set_flags(key, (key_info.flags & ~WLC_KEY_FLAG_TX));
done:
	return err;
}

wlc_key_algo_t
wlc_keymgmt_get_bss_key_algo(wlc_keymgmt_t *km,
	const wlc_bsscfg_t *bsscfg, bool igtk)
{
	const km_bsscfg_t *bss_km;
	wlc_key_id_t key_id;
	wlc_key_algo_t key_algo;
	wlc_key_info_t key_info;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	key_algo = CRYPTO_ALGO_NONE;
	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	if (igtk) {
#ifdef MFP
		for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
			wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
			key_algo = key_info.algo;
			if (key_algo != CRYPTO_ALGO_NONE)
				goto done;
		}
#endif /* MFP */
	} else {
		wlc_bsscfg_t *w_bsscfg;
		km_bsscfg_t *w_bss_km;

		/* use cached algorithm for the BSS. Otherwise use any available key */
		key_algo = bss_km->algo;
		if (key_algo != CRYPTO_ALGO_NONE)
			goto done;

		w_bsscfg = KM_WLC_BSSCFG(km->wlc, WLC_BSSCFG_IDX(bsscfg));
		KM_ASSERT(bsscfg == w_bsscfg);
		w_bss_km = KM_BSSCFG(km, w_bsscfg);

		for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
			if (!WLC_KEY_ID_IS_STA_GROUP(key_id))
				continue;
			wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
			w_bss_km->algo = key_algo = key_info.algo;
			if (key_algo != CRYPTO_ALGO_NONE)
				goto done;
		}

		wlc_keymgmt_get_bss_key(km, bsscfg, 0, &key_info);
		w_bss_km->algo = key_algo = key_info.algo;
	}

done:
	return key_algo;
}

bool
km_bsscfg_swkeys(wlc_keymgmt_t *km, const struct wlc_bsscfg *bsscfg)
{
	const km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	return (bss_km->flags & KM_BSSCFG_FLAG_SWKEYS);
}

#ifdef BRCMAPIVTW
int
km_bsscfg_ivtw_enable(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool enable)
{
	wlc_key_id_t key_id;
	wlc_key_index_t key_idx;
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km));

	bss_km = KM_BSSCFG(km, bsscfg);
	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		key_idx = bss_km->key_idx[key_id];
		if (!WLC_KEY_ID_IS_STA_GROUP(key_id))
			continue;
		km_ivtw_enable(km->ivtw, key_idx, enable);
	}

	return BCME_OK;
}
#endif /* BRCMAPIVTW */

uint32
km_bsscfg_get_wsec(keymgmt_t *km, const wlc_bsscfg_t *bsscfg)
{
	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(bsscfg != NULL);
	return KM_CONST_BSSCFG(km, bsscfg)->wsec;
}

#ifndef BCM_OL_DEV
#ifdef WOWL
bool
km_bsscfg_wowl_down(keymgmt_t *km, const wlc_bsscfg_t *bsscfg)
{
	const km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(bsscfg != NULL);
	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	return (bss_km->flags & KM_BSSCFG_FLAG_WOWL_DOWN);
}
#endif /* WOWL */

void
km_bsscfg_update(keymgmt_t *km, wlc_bsscfg_t *bsscfg, km_bsscfg_change_t change)
{
	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(bsscfg != NULL);

	switch (change) {
	case KM_BSSCFG_WSEC_CHANGE:
		km_bsscfg_sync_wsec(km, bsscfg);
		break;
	case KM_BSSCFG_BSSID_CHANGE:
		km_bsscfg_sync_bssid(km, bsscfg);
		break;
#ifdef WLOFFLD
	case KM_BSSCFG_ARM_TX:
		km_bsscfg_sync_arm_tx(km, bsscfg);
		break;
#endif /* WLOFFLD */
#ifdef WOWL
	case KM_BSSCFG_WOWL_DOWN:
		km_bsscfg_wowl(km, bsscfg, TRUE);
		break;
	case KM_BSSCFG_WOWL_UP:
		km_bsscfg_wowl(km, bsscfg, FALSE);
		break;
#endif
	default:
		break;
	}

	KM_LOG(("wl%d.%d: %s: change type %d\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__, change));
}
#endif /* !BCM_OL_DEV */

#ifdef KM_SERIAL_SUPPORTED
/* serialized static bsscfg of keymgmt related info is as follows
 * km_serial_t with header - version, obj type and #tlvs
		tx_key_id:KEY_ID
		bss_key[1..4] { - WEP or WPA_NONE keys only
			KEY_ID | KEY_ALGO | KEY_DATA | KEY_FLAGS
		}
 */

#define KM_SERIAL_BSSCFG_STATIC_VERSION 0x0

size_t
km_bsscfg_get_max_static_config_size(keymgmt_t *km)
{
	size_t sz = OFFSETOF(km_serial_t, tlvs);
	sz += KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t));
	sz += WLC_KEYMGMT_NUM_GROUP_KEYS * (
		KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t)) +
		KM_SERIAL_TLV_SIZE(sizeof(wlc_key_algo_t)) +
		KM_SERIAL_TLV_SIZE(KM_KEY_MAX_DATA_LEN) +
		KM_SERIAL_TLV_SIZE(sizeof(wlc_key_flags_t)));
	return sz;
}

int
km_bsscfg_get_static_config(void *ctx, wlc_bsscfg_t *bsscfg,
	uint8 *buf, int *in_len)
{
	keymgmt_t *km;
	wlc_key_id_t key_id;
	int err = BCME_OK;
	uint16 num_tlvs = 0;
	uint8 *buf_num_tlvs = NULL;
	int buf_len = 0;
	km_serial_t *ser = NULL;

	km = (keymgmt_t *)ctx;
	KM_DBG_ASSERT(KM_VALID(km));

	if (!in_len) {
		err = BCME_BADARG;
		goto done;
	}

	buf_len = *in_len;

	if (!KM_SERIAL_ENAB(km)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	/* if there are no static wep keys, and WPA_AUTH_NONE is not enabled,
	 * return 0 length output
	 */
	if (!(bsscfg->wsec & WEP_ENABLED) && !(bsscfg->WPA_auth & WPA_AUTH_NONE))
		goto done;

	if (buf_len >= OFFSETOF(km_serial_t, tlvs)) {
		ser = (km_serial_t *)buf;
		ser->version = KM_SERIAL_VERSION(KM_SERIAL_HDR_VERSION,
			KM_SERIAL_BSSCFG_STATIC_VERSION);
		ser->obj_type = KM_SERIAL_OBJ_TYPE_BSSCFG_STATIC & 0xff;
		buf_num_tlvs = (uint8 *)&ser->num_tlvs;
	}

	/* note: we decrement buf_len whether or not data is copied to buf */
	buf += OFFSETOF(km_serial_t, tlvs);
	buf_len -= OFFSETOF(km_serial_t, tlvs);

	/* tx key id */
	if (buf_len >= KM_SERIAL_TLV_SIZE(sizeof(key_id))) {
		key_id = wlc_keymgmt_get_bss_tx_key_id(km, bsscfg, FALSE);
		buf = bcm_write_tlv_safe(KM_SERIAL_TLV_KEY_ID, &key_id, sizeof(key_id),
			buf, buf_len);
		num_tlvs++;
	}
	buf_len -= KM_SERIAL_TLV_SIZE(sizeof(key_id));

	for (key_id = 0; key_id < (wlc_key_id_t)WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		wlc_key_t *key;
		wlc_key_info_t key_info;
		uint8 key_data[KM_KEY_MAX_DATA_LEN];
		size_t key_data_len;

		/* non-default infra BSS supports only two group keys; keep one key id
		 * for each mapped slot.
		 */
		if (KM_BSSCFG_GTK_IDX_POS(km, bsscfg, key_id) != key_id)
			continue;

		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
		KM_DBG_ASSERT(key_info.key_id == key_id);

		/* omit algo off */
		if (key_info.algo == CRYPTO_ALGO_OFF)
			continue;

		if (buf_len >= KM_SERIAL_TLV_SIZE(sizeof(key_id))) {
			buf = bcm_write_tlv_safe(KM_SERIAL_TLV_KEY_ID, &key_id, sizeof(key_id),
				buf, buf_len);
			num_tlvs++;
		}
		buf_len -= KM_SERIAL_TLV_SIZE(sizeof(key_id));

		if (buf_len >= KM_SERIAL_TLV_SIZE(sizeof(key_info.algo))) {
			buf = bcm_write_tlv_safe(KM_SERIAL_TLV_KEY_ALGO,
				&key_info.algo, sizeof(key_info.algo), buf, buf_len);
			num_tlvs++;
		}
		buf_len -= KM_SERIAL_TLV_SIZE(sizeof(key_info.algo));

		err = wlc_key_get_data(key, key_data, sizeof(key_data), &key_data_len);
		if (err != BCME_OK) {
			KM_ASSERT(err != BCME_BUFTOOSHORT);
			break;
		}

		if (buf_len >= KM_SERIAL_TLV_SIZE((int)key_data_len)) {
			buf = bcm_write_tlv_safe(KM_SERIAL_TLV_KEY_DATA,
				key_data, (int)key_data_len, buf, buf_len);
			num_tlvs++;
		}
		buf_len -= KM_SERIAL_TLV_SIZE((int)key_data_len);

		if (buf_len >= KM_SERIAL_TLV_SIZE(sizeof(uint32))) {
			key_info.flags = htol32(key_info.flags);
			buf = bcm_write_tlv_safe(KM_SERIAL_TLV_KEY_FLAGS,
				&key_info.flags, sizeof(key_info.flags), buf, buf_len);
			key_info.flags = ltoh32(key_info.flags);
			num_tlvs++;
		}
		buf_len -= KM_SERIAL_TLV_SIZE(sizeof(wlc_key_flags_t));
	}

done:
	if (buf_num_tlvs)
		htol16_ua_store(num_tlvs, buf_num_tlvs);
	if (in_len)
		*in_len -= buf_len;
	if (err == BCME_OK && buf_len < 0)
			err = BCME_BUFTOOSHORT;

	KM_LOG(("wl%d.%d: %s: status %d\n",  KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__, err));
	return err;
}

int
km_bsscfg_set_static_config(void *ctx, wlc_bsscfg_t *bsscfg,
	const uint8 *buf, int len)
{
	keymgmt_t *km;
	int err = BCME_OK;
	int num_tlvs = 0;
	bcm_tlv_t *tlv;
	wlc_key_id_t tx_key_id;

	km = (keymgmt_t *)ctx;
	KM_DBG_ASSERT(KM_VALID(km));

	if (!KM_SERIAL_ENAB(km)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	/* empty is okay */
	if ((len <= 0) || !buf)
		goto done;

	err = km_serial_parse_hdr(km, KM_SERIAL_OBJ_TYPE_BSSCFG_STATIC,
		KM_SERIAL_BSSCFG_STATIC_VERSION, (uint8*)buf, (size_t)len, &num_tlvs);
	if ((err != BCME_OK) || !num_tlvs)
		goto done;

	buf += OFFSETOF(km_serial_t, tlvs);
	len -= OFFSETOF(km_serial_t, tlvs);

	if (len < KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	tlv = (bcm_tlv_t *)buf;
	if (tlv->id != KM_SERIAL_TLV_KEY_ID ||
		tlv->len != sizeof(wlc_key_id_t)) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	tx_key_id = tlv->data[0];
	num_tlvs--;
	buf += KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t));
	len -= KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t));

	for (; num_tlvs >= 4; num_tlvs -= 4) {
		wlc_key_t *key;
		wlc_key_id_t key_id;
		wlc_key_algo_t key_algo;
		wlc_key_flags_t key_flags;
		uint8 *key_data;
		size_t key_data_len;

		/* key id */
		if (len < KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t))) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		tlv = (bcm_tlv_t *)buf;
		if (tlv->id != KM_SERIAL_TLV_KEY_ID ||
			tlv->len != sizeof(wlc_key_id_t)) {
			err = BCME_UNSUPPORTED;
			goto done;
		}

		key_id = tlv->data[0];
		buf += KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t));
		len -= KM_SERIAL_TLV_SIZE(sizeof(wlc_key_id_t));

		/* key algo */
		if (len < KM_SERIAL_TLV_SIZE(sizeof(wlc_key_algo_t))) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		tlv = (bcm_tlv_t *)buf;
		if (tlv->id != KM_SERIAL_TLV_KEY_ALGO ||
			tlv->len != sizeof(wlc_key_algo_t)) {
			err = BCME_UNSUPPORTED;
			goto done;
		}

		key_algo = tlv->data[0];
		buf += KM_SERIAL_TLV_SIZE(sizeof(wlc_key_algo_t));
		len -= KM_SERIAL_TLV_SIZE(sizeof(wlc_key_algo_t));

		/* key data */
		if (len < BCM_TLV_HDR_SIZE) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		tlv = (bcm_tlv_t *)buf;
		if (len < KM_SERIAL_TLV_SIZE(tlv->len)) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		if (tlv->id != KM_SERIAL_TLV_KEY_DATA || tlv->len == 0) {
			err = BCME_UNSUPPORTED;
			goto done;
		}

		key_data = tlv->data;
		key_data_len = (size_t)tlv->len;

		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, NULL);
		err = wlc_key_set_data(key, key_algo, key_data, key_data_len);
		if (err != BCME_OK)
			goto done;

		buf += KM_SERIAL_TLV_SIZE(key_data_len);
		len -= KM_SERIAL_TLV_SIZE((int)key_data_len);

		if (len < KM_SERIAL_TLV_SIZE(sizeof(wlc_key_flags_t))) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		tlv = (bcm_tlv_t *)buf;
		if (tlv->id != KM_SERIAL_TLV_KEY_FLAGS ||
			tlv->len != sizeof(wlc_key_flags_t)) {
			err = BCME_UNSUPPORTED;
			goto done;
		}

		key_flags = ltoh32_ua(tlv->data);
		err = wlc_key_set_flags(key, key_flags);
		if (err != BCME_OK)
			goto done;

		buf += KM_SERIAL_TLV_SIZE(sizeof(wlc_key_flags_t));
		len -= KM_SERIAL_TLV_SIZE(sizeof(wlc_key_flags_t));
	}

	err = wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, tx_key_id, FALSE);

done:
	KM_LOG(("wl%d.%d: %s: status %d, ignored %d tlvs\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err, num_tlvs));
	return err;

}
#endif /* KM_SERIAL_SUPPORTED */
