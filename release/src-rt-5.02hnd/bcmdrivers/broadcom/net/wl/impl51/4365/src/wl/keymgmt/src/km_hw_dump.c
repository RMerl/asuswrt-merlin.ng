/*
 * Key Management Module km_hw Implementation - dump support
 * Copyright (c) 2013 Broadcom Corporation, All rights reserved.
 * $Id: km_hw_dump.c 556100 2015-05-12 17:38:50Z $
 */

/* This file implements the wlc keymgmt functionality. It provides
 * hardware/memory dependent dump support
 */

#include "km_hw_impl.h"

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

static void
km_hw_key_dump(km_hw_t *hw, hw_idx_t hw_idx, struct bcmstrbuf *b)
{
	amt_idx_t amt_idx;
	skl_idx_t skl_idx;
	uint16 skl_val;
	uint8 data[KM_HW_MAX_DATA_LEN];
	size_t data_len;
	const km_hw_algo_entry_t *ae;
	bool txonly;
	wlc_key_hw_algo_t hw_algo;
	km_hw_dt_mask_t dt_mask;
	size_t i, j;
	int err;
	wlc_key_data_type_t dt;

	if (!KM_HW_IDX_VALID(hw, hw_idx))
		goto done;

	skl_idx = KM_HW_SKL_IDX(hw, hw_idx);
	if (skl_idx == KM_HW_SKL_IDX_INVALID) /* no hw key */
		goto done;

	amt_idx = KM_HW_AMT_IDX_INVALID;
	if (KM_HW_SKL_IDX_HAS_AMT(hw, skl_idx)) {
		amt_idx = KM_HW_SKL_IDX_TO_AMT(skl_idx);
	}

	txonly = skl_idx == KM_HW_SKL_IDX_TXONLY;
	bcm_bprintf(b, "\t0x%04x:0x%02x:0x%02x:%s:\n", hw_idx, skl_idx, amt_idx,
		(txonly ? "tx" : "rx"));
	if (txonly) {
		wlc_key_info_t key_info;
		km_get_hw_idx_key_info(KM_HW_KM(hw), hw_idx, &key_info);
		hw_algo = key_info.hw_algo;
	} else if (KM_HW_SKL_IDX_VALID(hw, skl_idx)) {
		skl_val = wlc_read_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx));
		hw_algo = (skl_val >> SKL_ALGO_SHIFT) & SKL_ALGO_MASK;
	} else {
		bcm_bprintf(b, "\n");
		goto done;
	}

	ae = km_hw_find_algo_entry(hw, hw_algo);
	if (ae == NULL || ae->impl.cb == NULL || ae->impl.cb->read == NULL) {
		bcm_bprintf(b, "\n");
		goto done;
	}

	dt_mask = ae->impl.dt_mask;
	for (i = 0, data_len = 0; i < KM_SIZE_BITS(dt_mask) && dt_mask != 0;
		++i, dt_mask >>= 1) {

		if (!(dt_mask & 0x1))
			continue;

		dt = (wlc_key_data_type_t)i;
		err = ae->impl.cb->read(hw, ae->impl.cb_ctx, hw_idx, dt, 0, FALSE, data,
			sizeof(data), &data_len);
		if (err != BCME_OK)
			continue;

		bcm_bprintf(b, "\t\t%s: ", wlc_key_get_data_type_name(dt));
		for (j = 0; j < data_len; ++j)
			bcm_bprintf(b, "%02x", data[j]);
		bcm_bprintf(b, "\n");
	}

#ifdef WOWL
	if (!KM_HW_WOWL_SUPPORTED(hw))
		goto wowl_done;

	dt = WLC_KEY_DATA_TYPE_SEQ;
	if (KM_HW_ALGO_DT_SUPPORTED(ae, dt)) {
		/* dump out per ac rx/seq for wowl */
		for (i = 1; i < (size_t)KM_HW_KEY_NUM_RX_SEQ(hw); ++i) {
			err = ae->impl.cb->read(hw, ae->impl.cb_ctx, hw_idx, dt, i, FALSE, data,
				sizeof(data), &data_len);
			if (err != BCME_OK)
				continue;
			bcm_bprintf(b, "\t\t%s[%d]: ", wlc_key_get_data_type_name(dt), i);
			for (j = 0; j < data_len; ++j)
				bcm_bprintf(b, "%02x", data[j]);
			bcm_bprintf(b, "\n");
		}

		err = ae->impl.cb->read(hw, ae->impl.cb_ctx, hw_idx, dt, 0, TRUE, data,
			sizeof(data), &data_len);
		if (err == BCME_OK) {
			bcm_bprintf(b, "\t\t%s[tx]: ", wlc_key_get_data_type_name(dt));
			for (j = 0; j < data_len; ++j)
				bcm_bprintf(b, "%02x", data[j]);
			bcm_bprintf(b, "\n");
		}
	}

	dt = WLC_KEY_DATA_TYPE_TKHASH_P1;
	if (KM_HW_ALGO_DT_SUPPORTED(ae, dt)) {
		err = ae->impl.cb->read(hw, ae->impl.cb_ctx, hw_idx, dt, 0, TRUE, data,
			sizeof(data), &data_len);
		if (err == BCME_OK) {
			bcm_bprintf(b, "\t\t%s[tx]: ", wlc_key_get_data_type_name(dt));
			for (j = 0; j < data_len; ++j)
				bcm_bprintf(b, "%02x", data[j]);
			bcm_bprintf(b, "\n");
		}
	}

	wowl_done:;
#endif /* WOWL */

done:;
}

void
km_hw_dump(km_hw_t *hw, struct bcmstrbuf *b, km_key_dump_type_t dump_type)
{
	int i;
	hw_algo_t hw_algo;

	if (!KM_HW_VALID(hw))
		return;

	if (!KM_HW_PUB(hw)->up) {
		KM_HW_LOG(("wl%d: %s: wlc not up\n",  KM_HW_UNIT(hw), __FUNCTION__));
		return;
	}

	if (!KM_HW_WLC(hw)->clk) {
		KM_HW_LOG(("wl%d: %s: no clock\n",  KM_HW_UNIT(hw), __FUNCTION__));
		return;
	}

	switch (dump_type) {
	case KM_KEY_DUMP_ALL:
		bcm_bprintf(b, "begin wl%d km %s h/w dump:\n", KM_HW_UNIT(hw),
			(KM_HW_WOWL_SUPPORTED(hw) ? "[wowl]" : ""));
		bcm_bprintf(b, "\tmax idx: %d\n", hw->max_idx);
		bcm_bprintf(b, "\tmax amt idx: %d\n", hw->max_amt_idx);
		bcm_bprintf(b, "\tmcnx start: %d mcnx count: %d\n",
			hw->amt_info.mcnx_start, hw->amt_info.mcnx_count);

		bcm_bprintf(b, "\tmax key size: %d\n", hw->max_key_size);
		bcm_bprintf(b, "\thw idx -> skl\n");
		for (i = 0; KM_HW_IDX_VALID(hw, (hw_idx_t)i); ++i) {
			skl_idx_t skl_idx = KM_HW_SKL_IDX(hw, i);
			if (KM_HW_SKL_IDX_VALID(hw, skl_idx) || (skl_idx == KM_HW_SKL_IDX_TXONLY))
				bcm_bprintf(b, "\t\tskl[%03d]: %d\n", i, hw->skl[i]);
		}

		bcm_bprintf(b, "\tamt used: ");
		for (i = CEIL(hw->max_amt_idx, NBBY) - 1; i >= 0; --i)
			bcm_bprintf(b, "%02x", hw->used[i]);
		bcm_bprintf(b, "\n");

		bcm_bprintf(b, "\tflags: 0x%02x\n", hw->flags);
		bcm_bprintf(b, "\tshm info:\n");
		bcm_bprintf(b, "\t\t key base: [%d:0x%04x]\n", hw->shm_info.key_base,
			hw->shm_info.key_base);

		bcm_bprintf(b, "\t\t skl idx base: [%d:0x%04x]\n", hw->shm_info.skl_idx_base,
			hw->shm_info.skl_idx_base);

#ifndef LINUX_CRYPTO
		bcm_bprintf(b, "\t\t tkip mic keys: max %d base: [%d:0x%04x]\n",
			hw->shm_info.max_tkip_mic_keys,
			hw->shm_info.tkip_mic_key_base, hw->shm_info.tkip_mic_key_base);
		bcm_bprintf(b, "\t\t tsc ttak max %d base: [%d:0x%04x]\n",
			hw->shm_info.max_tsc_ttak, hw->shm_info.tkip_tsc_ttak_base,
			hw->shm_info.tkip_tsc_ttak_base);
#endif /* !LINUX_CRYPTO */
#ifdef BCMWAPI_WPI
		bcm_bprintf(b, "\t\t wapi mic keys max %d base: [%d:0x%04x]\n",
			hw->shm_info.max_wapi_mic_keys,
			hw->shm_info.wapi_mic_key_base, hw->shm_info.wapi_mic_key_base);
#endif /* BCMWAPI_WPI */

		bcm_bprintf(b, "\t\t rx pn max %d, base: [%d:0x%04x]\n",
			hw->shm_info.max_rx_pn, hw->shm_info.rx_pn_base, hw->shm_info.rx_pn_base);
		bcm_bprintf(b, "\t\t tx pn max %d, base: [%d:0x%04x]\n",
			hw->shm_info.max_tx_pn, hw->shm_info.tx_pn_base, hw->shm_info.tx_pn_base);
		bcm_bprintf(b, "end km h/w dump:\n");
		/* fall through */
	case KM_KEY_DUMP_SECALGO:
		bcm_bprintf(b, "begin wl%d km %s h/w sec algo dump:\n", KM_HW_UNIT(hw),
			(KM_HW_WOWL_SUPPORTED(hw) ? "[wowl]" : ""));
		bcm_bprintf(b, "\t[skl]:algo:val\n");
		for (i = 0; i <  KM_HW_MAX_SKL_IDX(hw); ++i) {
			uint16 skl_val;
			skl_val = wlc_read_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, i));
			hw_algo = (skl_val >> SKL_ALGO_SHIFT) & SKL_ALGO_MASK;
			if (hw_algo == WSEC_ALGO_OFF)
				continue;
			bcm_bprintf(b, "\t[%03d]:%s:0x%04x\n", i,
				wlc_keymgmt_get_hw_algo_name(KM_HW_KM(hw), hw_algo, 0), skl_val);
		}
		bcm_bprintf(b, "end h/w sec algo dump:\n");

		if (dump_type != KM_KEY_DUMP_ALL)
			break;
	case KM_KEY_DUMP_HW_KEYS:
		bcm_bprintf(b, "begin wl%d km %s h/w key dump:\n", KM_HW_UNIT(hw),
			(KM_HW_WOWL_SUPPORTED(hw) ? "[wowl]" : ""));
		bcm_bprintf(b, "\thw idx:skl:amt:type:data...\n");
		for (i = 0; KM_HW_IDX_VALID(hw, (hw_idx_t)i); ++i)
			km_hw_key_dump(hw, (hw_idx_t)i, b);
		bcm_bprintf(b, "end h/w key dump:\n");
		break;
	default:
		break;
	}
}

#endif /* BCMDBG || BCMDBG_DUMP */
