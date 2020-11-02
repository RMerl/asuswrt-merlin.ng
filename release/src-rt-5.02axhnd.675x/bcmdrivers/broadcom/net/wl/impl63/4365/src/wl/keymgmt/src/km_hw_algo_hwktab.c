/*
 * Key Management Module km_hw algo HW keytable implementation
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: km_hw_algo_hwktab.c 781617 2019-11-26 08:52:07Z $
 */

#include "km_hw_impl.h"
#include <bcmcrypto/tkhash.h>

#ifdef WL_HWKTAB
void
hw_algo_impl_init_hwktab(const km_hw_t *hw, km_hw_algo_impl_t *impl,
	key_algo_t algo, const km_hw_algo_callbacks_t *cb,
	void *cb_ctx, const km_hw_dt_mask_t dt_mask)
{
	hw_algo_t hw_algo;
	hw_algo = km_hw_algo_to_hw_algo(hw, algo);
	if (hw_algo != WSEC_ALGO_OFF) {
		impl->cb = cb;
		impl->hw_algo = hw_algo;
		impl->cb_ctx = cb_ctx;
		impl->dt_mask = dt_mask;
	}
}

/* write the key into shm slot and clear unused */
static void
km_hw_algo_write_key(km_hw_t *hw, hwktab_addr_t addr, const uint8 *data, size_t data_len)
{
	wlc_info_t *wlc = KM_HW_WLC(hw);

	if ((data_len > 0) && !data[0]) {
	        data = km_hw_fixup_null_hw_key(hw, data, data_len);
	}
	KM_HW_COPYTO_HWKTAB(wlc, addr, data, data_len & ~3);
	KM_DBG_ASSERT((data_len & 0x1) == 0);
	if (data_len & 0x2) {
		uint8 buf[4];
		buf[0] = data[data_len - 2];
		buf[1] = data[data_len - 1];
		buf[2] = buf[3] = 0;
		KM_HW_COPYTO_HWKTAB(wlc, addr + (data_len & ~3), buf, 4);
		data_len += 2;
	}

	if (data_len < hw->max_key_size)
		KM_HW_SET_HWKTAB(wlc, addr + (hwktab_addr_t)data_len, 0,
			hw->max_key_size - (uint8)data_len);

	KM_HW_LOG(("wl%d: %s: wrote h/w key data at hwktab addr %d[0x%04x], length %lu\n",
		KM_HW_UNIT(hw), __FUNCTION__, addr, addr, (unsigned long)data_len));
}

#if !defined(BCM_OL_DEV) && (defined(BCMDBG) || defined(BCMDBUG_DUMP) || defined(WOWL))
#define HW_READCB(_cb) _cb

static int
wep_read(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
    wlc_key_data_type_t data_type, int ins, bool tx, uint8 *data,
	size_t data_size, size_t *data_len)
{
	int err = BCME_OK;

	if (!data_len) {
		err = BCME_BADARG;
		goto done;
	}

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		*data_len = hw->max_key_size;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;

	case WLC_KEY_DATA_TYPE_SEQ:
		(void)tx;
		/* fall through */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}

static int
aes_read(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
    wlc_key_data_type_t data_type, int ins, bool tx, uint8 *data,
	size_t data_size, size_t *data_len)
{
	int err = BCME_OK;

	if (!data_len) {
		err = BCME_BADARG;
		goto done;
	}

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		*data_len = hw->max_key_size;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		*data_len = KEY_SEQ_SIZE;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
#ifdef WOWL
		if (tx) {
			shm_addr_t shm_addr;
			if (!hw->shm_info.tx_pn_base) {
				err = BCME_UNSUPPORTED;
				break;
			}
			shm_addr = hw->shm_info.tx_pn_base + OFFSETOF(wowl_templ_ctxt_t, seciv) +
				TKHASH_P1_KEY_SIZE;
			KM_HW_COPYFROM_SHM(KM_HW_WLC(hw), shm_addr, data, KEY_SEQ_SIZE);
			break;
		}
#endif /* WOWL */
		if (!hw->shm_info.rx_pn_base ||
			hw_idx >= hw->shm_info.max_rx_pn ||
			ins >= WLC_KEY_BASE_RX_SEQ) {
			err = BCME_UNSUPPORTED;
			break;
		}
		KM_HW_COPYFROM_SHM(KM_HW_WLC(hw), KM_HW_RX_SEQ_ADDR(hw, hw_idx, ins),
			data, (int)(*data_len));
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}

#ifndef LINUX_CRYPTO
static int
tkip_read(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
    wlc_key_data_type_t data_type, int ins, bool tx, uint8 *data,
	size_t data_size, size_t *data_len)
{
	int err = BCME_OK;

	if (!data_len) {
		err = BCME_BADARG;
		goto done;
	}

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_TKIP_TK:
		*data_len = hw->max_key_size;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS:
		if (hw_idx >= hw->shm_info.max_tkip_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		*data_len = TKIP_MIC_KEY_SIZE;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		/* no context to determine from/to AP, assume AP for debug support */
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_TKIP_TX_MIC_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS:
		if (hw_idx >= hw->shm_info.max_tkip_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		*data_len = TKIP_MIC_KEY_SIZE;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		/* no context to determine from/to AP, assume AP for debug support */
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_TKIP_RX_MIC_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_TKHASH_P1:
		*data_len = TKHASH_P1_KEY_SIZE + sizeof(uint32);
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (!KM_HW_TKIP_TSC_TTAK_SUPPORTED(hw, hw_idx)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		KM_HW_COPYFROM_SHM(KM_HW_WLC(hw),
			KM_HW_TKIP_TSC_TTAK_ADDR(hw, hw_idx), data, (int)(*data_len));
		break;

	case WLC_KEY_DATA_TYPE_SEQ:
		*data_len = KEY_SEQ_SIZE;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
#ifdef WOWL
		if (tx) {
			shm_addr_t shm_addr;
			if (!hw->shm_info.tx_pn_base) {
				err = BCME_UNSUPPORTED;
				break;
			}
			shm_addr = hw->shm_info.tx_pn_base + OFFSETOF(wowl_templ_ctxt_t, seciv) +
				TKHASH_P1_KEY_SIZE;
			KM_HW_COPYFROM_SHM(KM_HW_WLC(hw), shm_addr, data, KEY_SEQ_SIZE);
			KM_SWAP(uint8, data[0], data[1]); /* convert to LE - shm has TSC1.TSC0 */
			break;
		}
#endif /* WOWL */

		if (!hw->shm_info.rx_pn_base ||
			hw_idx >= hw->shm_info.max_rx_pn ||
			ins >= WLC_KEY_BASE_RX_SEQ) {
			err = BCME_UNSUPPORTED;
			break;
		}
		KM_HW_COPYFROM_SHM(KM_HW_WLC(hw), KM_HW_RX_SEQ_ADDR(hw, hw_idx, ins),
			data, (int)(*data_len));
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}
#endif /* !LINUX_CRYPTO */

#ifdef BCMWAPI_WPI
static int
sms4_read(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
    wlc_key_data_type_t data_type, int ins, bool tx, uint8 *data,
	size_t data_size, size_t *data_len)
{
	int err = BCME_OK;

	if (!data_len) {
		err = BCME_BADARG;
		goto done;
	}

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		*data_len = hw->max_key_size;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY:
		if (hw_idx >= hw->shm_info.max_wapi_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		*data_len = KM_HW_SMS4_MIC_KEY_SIZE;
		if (data_size < *data_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		KM_HW_COPYFROM_HWKTAB(KM_HW_WLC(hw), KM_HWKTAB_SMS4_MIC_KEY_ADDR(hw, hw_idx),
			data, (int)(*data_len));
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		(void)tx;
		/* fall through */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

done:
	return err;
}
#endif /* BCMWAPI_WPI */
#else
#define HW_READCB(_cb) NULL
#endif /* !BCM_OL_DEV && (BCMDBG || BCMDBUG_DUMP || WOWL) */

/* wep support, no support for seq/rxiv */
static int wep_write(km_hw_t *hw, void *ctx, hw_idx_t hw_idx, wlc_key_data_type_t dt,
	int ins, bool tx, const wlc_key_info_t *key_info, const uint8 *data, size_t data_len)
{
	int err = BCME_OK;
	switch (dt) {
	case WLC_KEY_DATA_TYPE_KEY:
		km_hw_algo_write_key(hw, KM_HWKTAB_KEY_ADDR(hw, hw_idx), data, data_len);
		break;
	default:
		(void)tx;
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

const km_hw_algo_callbacks_t wep_callbacks_hwktab = {
	HW_READCB(wep_read),
	wep_write,
	NULL
};

/* aes support */
static int aes_write(km_hw_t *hw, void *ctx, hw_idx_t hw_idx, wlc_key_data_type_t dt,
	int ins, bool tx, const wlc_key_info_t *key_info, const uint8 *data, size_t data_len)
{
	int err = BCME_OK;
	shm_addr_t shm_addr;

	switch (dt) {
	case WLC_KEY_DATA_TYPE_KEY:
		km_hw_algo_write_key(hw, KM_HWKTAB_KEY_ADDR(hw, hw_idx), data, data_len);
		break;

	case WLC_KEY_DATA_TYPE_SEQ:
		if (data_len != KEY_SEQ_SIZE) {
			err = BCME_BADARG;
			break;
		}

		if (tx) {
#if defined(WOWL) || defined(BCM_OL_DEV)
			if (!hw->shm_info.tx_pn_base) {
				err = BCME_UNSUPPORTED;
				break;
			}
			shm_addr = hw->shm_info.tx_pn_base;
#ifdef WOWL
			shm_addr += OFFSETOF(wowl_templ_ctxt_t, seciv) + TKHASH_P1_KEY_SIZE;
#endif // endif
			KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, data, KEY_SEQ_SIZE);
#endif /* WOWL || BCM_OL_DEV */
			break;
		}

		if (!hw->shm_info.rx_pn_base ||
			hw_idx >= hw->shm_info.max_rx_pn ||
			ins >= WLC_KEY_BASE_RX_SEQ) {
			err = BCME_UNSUPPORTED;
			break;
		}
		shm_addr = KM_HW_RX_SEQ_ADDR(hw, hw_idx, ins);
		KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, data, (int)data_len);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

const km_hw_algo_callbacks_t aes_callbacks_hwktab = {
	HW_READCB(aes_read),
	aes_write,
	NULL
};

/* tkip support */
#ifndef LINUX_CRYPTO
static int
tkip_write(km_hw_t *hw, void *ctx, hw_idx_t hw_idx, wlc_key_data_type_t data_type,
	int ins, bool tx, const wlc_key_info_t *key_info, const uint8 *data, size_t data_len)
{
	int err = BCME_OK;
	shm_addr_t shm_addr;
	hwktab_addr_t	addr;

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_TKIP_TK:
		km_hw_algo_write_key(hw, KM_HWKTAB_KEY_ADDR(hw, hw_idx), data, data_len);
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS:
		if (hw_idx >= hw->shm_info.max_tkip_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (WLC_KEY_IS_AP(key_info) || WLC_KEY_IS_IBSS(key_info))
			addr = KM_HWKTAB_TKIP_TX_MIC_KEY_ADDR(hw, hw_idx);
		else
			addr = KM_HWKTAB_TKIP_RX_MIC_KEY_ADDR(hw, hw_idx);

		KM_ASSERT(data_len <= TKIP_MIC_KEY_SIZE);
		KM_HW_COPYTO_HWKTAB(KM_HW_WLC(hw), addr, data, (int)data_len);
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS:
		if (hw_idx >= hw->shm_info.max_tkip_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (WLC_KEY_IS_AP(key_info) || WLC_KEY_IS_IBSS(key_info))
			addr = KM_HWKTAB_TKIP_RX_MIC_KEY_ADDR(hw, hw_idx);
		else
			addr = KM_HWKTAB_TKIP_TX_MIC_KEY_ADDR(hw, hw_idx);

		KM_ASSERT(data_len <= TKIP_MIC_KEY_SIZE);
		KM_HW_COPYTO_HWKTAB(KM_HW_WLC(hw), addr, data, (int)data_len);
		break;
	case WLC_KEY_DATA_TYPE_TKHASH_P1:
		if (!KM_HW_TKIP_TSC_TTAK_SUPPORTED(hw, hw_idx)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (tx) {
#if defined(WOWL)
		/* In case of BCM_OL_DEV, phase1 key is sent in tx descriptor */
			if (!hw->shm_info.tx_pn_base) {
				err = BCME_UNSUPPORTED;
				break;
			}
			shm_addr = hw->shm_info.tx_pn_base +  OFFSETOF(wowl_templ_ctxt_t, seciv);
			KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, data, TKHASH_P1_KEY_SIZE);
#endif /* WOWL  */
			break;
		}

		shm_addr = KM_HW_TKIP_TSC_TTAK_ADDR(hw, hw_idx);

		KM_DBG_ASSERT(data_len == (TKHASH_P1_KEY_SIZE + sizeof(uint32)));

		KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, data, (int)data_len - sizeof(uint32));
		shm_addr += TKHASH_P1_KEY_SIZE;
		KM_HW_WRITE_SHM(KM_HW_WLC(hw), shm_addr, ltoh16_ua(data + TKHASH_P1_KEY_SIZE));
		shm_addr += sizeof(uint16);
		KM_HW_WRITE_SHM(KM_HW_WLC(hw), shm_addr,
			ltoh16_ua(data + TKHASH_P1_KEY_SIZE + sizeof(uint16)));
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (data_len != KEY_SEQ_SIZE) {
			err = BCME_BADARG;
			break;
		}
		if (tx) {
#if defined(WOWL) || defined(BCM_OL_DEV)
			uint8 tx_seq[KEY_SEQ_SIZE];
			if (!hw->shm_info.tx_pn_base) {
				err = BCME_UNSUPPORTED;
				break;
			}
#ifdef WOWL
			shm_addr = hw->shm_info.tx_pn_base +  OFFSETOF(wowl_templ_ctxt_t, seciv)
				+ TKHASH_P1_KEY_SIZE;
			memcpy(tx_seq, data, KEY_SEQ_SIZE);
			KM_SWAP(uint8, tx_seq[0], tx_seq[1]); /* tsc1.ts0 in shm */
			KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, tx_seq, KEY_SEQ_SIZE);
#else
			BCM_REFERENCE(tx_seq);
			shm_addr = hw->shm_info.tx_pn_base;
			KM_HW_COPYTO_SHM(KM_HW_WLC(hw), shm_addr, data, KEY_SEQ_SIZE);
#endif /* WOWL */
#endif /* WOWL  || BCM_OL_DEV */
			break;
		}
		if (!hw->shm_info.rx_pn_base ||
			hw_idx >= hw->shm_info.max_rx_pn ||
			ins >= WLC_KEY_BASE_RX_SEQ) {
			err = BCME_UNSUPPORTED;
			break;
		}
		KM_HW_COPYTO_SHM(KM_HW_WLC(hw), KM_HW_RX_SEQ_ADDR(hw, hw_idx, ins),
			data, (int)data_len);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

const km_hw_algo_callbacks_t tkip_callbacks_hwktab = {
	HW_READCB(tkip_read),
	tkip_write,
	NULL
};
#endif /* !LINUX_CRYPTO */

/* sms4 support */
#ifdef BCMWAPI_WPI
static int
sms4_write(km_hw_t *hw, void *ctx, hw_idx_t hw_idx, wlc_key_data_type_t data_type,
	int ins, bool tx, const wlc_key_info_t *key_info, const uint8 *data, size_t data_len)
{
	hwktab_addr_t addr;
	int err = BCME_OK;

	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		km_hw_algo_write_key(hw, KM_HWKTAB_KEY_ADDR(hw, hw_idx), data, data_len);
		break;
	case WLC_KEY_DATA_TYPE_MIC_KEY:
		if (hw_idx >= hw->shm_info.max_wapi_mic_keys) {
			err = BCME_UNSUPPORTED;
			break;
		}

		addr = KM_HWKTAB_SMS4_MIC_KEY_ADDR(hw, hw_idx);

		KM_ASSERT(data_len == KM_HWKTAB_SMS4_MIC_KEY_SIZE);
		KM_HW_COPYTO_HWKTAB(KM_HW_WLC(hw), addr, data, (int)data_len);
		break;

	case WLC_KEY_DATA_TYPE_SEQ:
		(void)tx;
		/* fall through */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

const km_hw_algo_callbacks_t sms4_callbacks_hwktab = {
	HW_READCB(sms4_read),
	sms4_write,
	NULL
};
#endif /* BCMWAPI_WPI */

#endif /* WL_HWKTAB */
