/*
 * Key Management Module km_hw Implementation - allocation support
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_hw_alloc.c 594757 2015-10-23 12:35:49Z $
 */

#include "km_hw_pvt.h"
#include "km_pvt.h"

/* functions exported to km */
void
km_hw_amt_reserve(km_hw_t *hw, amt_idx_t amt_idx, size_t count, bool val)
{
	amt_idx_t end_idx;

	KM_DBG_ASSERT(KM_HW_AMT_IDX_VALID(hw, amt_idx));
	end_idx = amt_idx + (amt_idx_t)count;
	KM_ASSERT(KM_HW_AMT_IDX_VALID(hw, end_idx - 1));

	for (; amt_idx < end_idx; ++amt_idx) {
		if (val)
			setbit(hw->used, amt_idx);
		else
			clrbit(hw->used, amt_idx);
	}
}

bool
km_hw_amt_idx_isset(km_hw_t *hw, int amt_idx)
{
	if (isset(hw->used, amt_idx))
		return TRUE;
	return FALSE;
}

amt_idx_t
km_hw_amt_find_and_resrv(km_hw_t *hw)
{
	amt_idx_t amt_idx;
	for (amt_idx = 0; KM_HW_AMT_IDX_VALID(hw, amt_idx); ++amt_idx) {
		if (!km_hw_amt_idx_isset(hw, amt_idx))
			break;
	}
	if (KM_HW_AMT_IDX_VALID(hw, amt_idx)) {
		km_hw_amt_reserve(hw, amt_idx, 1, TRUE);
		return amt_idx;
	}
	return KM_HW_AMT_IDX_INVALID;
}

#ifdef ACKSUPR_MAC_FILTER
amt_idx_t
km_hw_amt_alloc_acksupr(km_hw_t *hw, scb_t *scb)
{
	int amt_idx = -1;
	uint16 attr;
	wlc_info_t *wlc = KM_HW_WLC(hw);

	if (D11REV_GE(wlc->pub->corerev, 40))
		attr = (AMT_ATTR_VALID | AMT_ATTR_A2);
	else
		attr = AMT_ATTR_VALID;

	amt_idx = wlc_macfltr_find_and_add_addrmatch(wlc, SCB_BSSCFG(scb),
		&scb->ea, attr);
	if (amt_idx == BCME_NOTFOUND || !KM_HW_AMT_IDX_VALID(hw, amt_idx))
		return KM_HW_AMT_IDX_INVALID;

	km_hw_amt_reserve(hw, (km_amt_idx_t)amt_idx, 1, TRUE);

	return amt_idx;
}
#endif /* ACKSUPR_MAC_FILTER */

amt_idx_t
km_hw_amt_alloc(km_hw_t *hw, const struct ether_addr *ea)
{
	amt_idx_t  amt_idx;

	/* we expect hw to be initialized at this point */
	KM_DBG_ASSERT(KM_HW_VALID(hw) && !KM_HW_NEED_INIT(hw));

	for (amt_idx = 0; KM_HW_AMT_IDX_VALID(hw, amt_idx); ++amt_idx) {
		if (!isset(hw->used, amt_idx))
			break;
	}

	/* reserve the chosen amt idx. */
	if (KM_HW_AMT_IDX_VALID(hw, amt_idx)) {
		km_hw_amt_reserve(hw, amt_idx, 1, TRUE);
		hw_amt_update(hw, amt_idx, ea);
	} else {
		amt_idx = KM_HW_AMT_IDX_INVALID;
	}

	return amt_idx;
}

void
km_hw_amt_release(km_hw_t *hw, amt_idx_t *amt_idx)
{
	KM_DBG_ASSERT(amt_idx != NULL);

	if (!KM_HW_AMT_IDX_VALID(hw, *amt_idx))
		return;

	km_hw_amt_reserve(hw, *amt_idx, 1, FALSE);
	hw_amt_update(hw, *amt_idx, NULL);
	*amt_idx = KM_HW_AMT_IDX_INVALID;
}

/* functions local to km_hw */
void
hw_amt_update(km_hw_t *hw, amt_idx_t amt_idx, const struct ether_addr *ea)
{
	uint16 amt_attr;

	if (!KM_HW_WLC(hw)->clk)
		return;

	amt_attr = wlc_clear_addrmatch(KM_HW_WLC(hw), amt_idx);
	if (isset(hw->used, amt_idx)) {
		amt_attr |= AMT_ATTR_VALID;
	} else {
		amt_attr &= ~AMT_ATTR_VALID;
	}

	if ((amt_attr & AMT_ATTR_VALID) && (ea != NULL))
		(void)wlc_set_addrmatch(KM_HW_WLC(hw), amt_idx, ea, amt_attr);
}

hw_idx_t
hw_idx_alloc_specific(km_hw_t *hw, hw_idx_t hw_idx, skl_idx_t skl_idx)
{
	if (!KM_HW_IDX_VALID(hw, hw_idx))
		return WLC_KEY_INDEX_INVALID;

	if (KM_HW_SKL_IDX(hw, hw_idx)  != KM_HW_SKL_IDX_INVALID)
		return WLC_KEY_INDEX_INVALID; /* in use */

	KM_ASSERT(KM_HW_SKL_IDX_VALID(hw, skl_idx));
	KM_HW_SKL_IDX(hw, hw_idx) = skl_idx;

	/* assuming caller allocated/updated any required amt */
	return hw_idx;
}

void
hw_idx_release(km_hw_t *hw, hw_idx_t hw_idx_start, size_t count,
	const struct ether_addr *ea)
{
	size_t i;
	for (i = 0; i < count; ++i) {
		hw_idx_t hw_idx;

		hw_idx = hw_idx_start + (hw_idx_t)i;
		if (!KM_HW_IDX_VALID(hw, hw_idx))
			break;
		KM_HW_SKL_IDX(hw, hw_idx) = KM_HW_SKL_IDX_INVALID;
	}
}

/* allocate a block of hw idx,  and assign all of them the same skl index.
 * always allocates a pairwise idx, followed by requested (count - 1)
 * contiguous group keys. this call is not used to allocate ibss peer
 * group keys.  assigns an skl index, reserves AMT if required. returns the
 * first h/w index (pairwise) assigned.
 */
hw_idx_t
hw_idx_alloc(km_hw_t *hw, size_t count, km_alloc_key_info_t *alloc_info)
{
	hw_idx_t hw_idx;
	hw_idx_t i = 0;
	hw_idx_t j;
	skl_idx_t skl_idx;
	amt_idx_t amt_idx;
	scb_t *scb;
	wlc_bsscfg_t *bsscfg;
	km_bsscfg_t *bss_km;
	bool at_end;

	KM_HW_LOG(("wl%d: %s: req count %lu\n", KM_HW_UNIT(hw),
		__FUNCTION__, (unsigned long)count));

	KM_DBG_ASSERT(count != 0);

	hw_idx = WLC_KEY_INDEX_INVALID;
	amt_idx = KM_HW_AMT_IDX_INVALID;
	skl_idx = KM_HW_SKL_IDX_INVALID;

	/* for non-tx only alloc, skip default BSS slots, otherwise start at the end */
	at_end = (alloc_info == NULL);

	i = (at_end ? (hw->max_idx - (hw_idx_t)count) : WLC_KEYMGMT_NUM_GROUP_KEYS);
	do {
		size_t num_found = 0;
		for (j = 0; j < (hw_idx_t)count && num_found < count; ++j) {
			hw_idx_t slot;
			skl_idx_t slot_skl;

			slot = i + j;
			if (!KM_HW_IDX_VALID(hw, slot))
				break;

			/* a slot with valid skl or tx only is allocated */
			slot_skl = KM_HW_SKL_IDX(hw, slot);
			if (KM_HW_SKL_IDX_VALID(hw, slot_skl) ||
				(slot_skl == KM_HW_SKL_IDX_TXONLY)) {
				j++;
				break;
			}
			num_found++;
		}

		if (num_found == count) {
			hw_idx = i;
			break;
		}

		i = at_end ? (i - 1) : (i + j);
	} while (at_end ? (i >= WLC_KEYMGMT_NUM_GROUP_KEYS) : KM_HW_IDX_VALID(hw, i));

	if (hw_idx == WLC_KEY_INDEX_INVALID)
		goto done;

	if (!alloc_info) { /* tx only keys */
		skl_idx = KM_HW_SKL_IDX_TXONLY;
		goto set_skl;
	}

	scb = alloc_info->scb_info.scb;
	KM_ASSERT(scb != NULL);
	amt_idx = km_scb_amt_alloc(KM_HW_KM(hw), scb);

	bsscfg = alloc_info->bss_info.bsscfg;
	if (BSSCFG_STA(bsscfg) && !KM_HW_IS_DEFAULT_BSSCFG(hw, bsscfg)) {
		bss_km = KM_BSSCFG(KM_HW_KM(hw), bsscfg);

		if (bss_km->amt_idx != KM_HW_AMT_IDX_INVALID)
			amt_idx = bss_km->amt_idx;
	}

	if (!KM_HW_AMT_IDX_VALID(hw, amt_idx)) {
		hw_idx = WLC_KEY_INDEX_INVALID;
		goto done;
	}

	KM_DBG_ASSERT(isset(hw->used, amt_idx));
	skl_idx =  amt_idx + WLC_KEYMGMT_NUM_GROUP_KEYS;

set_skl:
	for (j = 0; j < count; ++j) {
		KM_HW_SKL_IDX(hw, hw_idx + j) = skl_idx;
	}

done:
	KM_HW_LOG(("wl%d: %s: hw idx %d skl %d amt %d\n", KM_HW_UNIT(hw), __FUNCTION__,
		hw_idx, skl_idx, amt_idx));
	return hw_idx;
}

bool
hw_idx_delete_ok(km_hw_t *hw, hw_idx_t hw_idx, const km_alloc_key_info_t *info)
{
	int i;
	bool ok = TRUE;

	KM_DBG_ASSERT(hw_idx != WLC_KEY_INDEX_INVALID);

	do {

		/* scb keys of default bss on a infra sta are okay to delete */
		if ((info->bss_info.bsscfg->BSS) && (hw_idx == info->scb_info.hw_idx) &&
			BSSCFG_STA(info->bss_info.bsscfg) &&
			KM_HW_IS_DEFAULT_BSSCFG(hw, info->bss_info.bsscfg)) {
				break;
		}

		/* valid scb h/w idx that is not being deleted, not okay to delete */
		if (info->scb_info.hw_idx != WLC_KEY_INDEX_INVALID) {
			if (hw_idx != info->scb_info.hw_idx) {
				ok = FALSE;
				break;
			}
		}

		/* if sta group keys are allocated, not okay if there is another valid one */
		if (KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(hw, info->bss_info.bsscfg)) {
			/* valid bss h/w idx that is not being deleted, not okay to delete */
			for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
				if (info->bss_info.hw_idx[i] != WLC_KEY_INDEX_INVALID) {
					if (hw_idx !=  info->bss_info.hw_idx[i]) {
						ok = FALSE;
						break;
					}
				}
			}
			if (!ok)
				break;
		}

#if defined(IBSS_PEER_GROUP_KEY)
		for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
			hw_idx_t hw_idx2 = info->scb_info.ibss_info.hw_idx[i];
			if (hw_idx2 != WLC_KEY_INDEX_INVALID && hw_idx != hw_idx2) {
				ok = FALSE;
				break;
			}
		}
#endif /* IBSS_PEER_GROUP_KEY */
	} while (0);

	return ok;
}

/* compute and return the h/w idx to use in skl from alloc_info */
void
hw_get_hw_idx_from_alloc_info(km_hw_t *hw, const wlc_key_info_t *key_info,
	km_alloc_key_info_t *alloc_info, hw_idx_t *out_hw_idx)
{
	int i;
	hw_idx_t hw_idx;

	KM_DBG_ASSERT(out_hw_idx != NULL && key_info != NULL && alloc_info != NULL);

	hw_idx = alloc_info->scb_info.hw_idx;
	if (hw_idx != WLC_KEY_INDEX_INVALID)
		goto done;

	KM_DBG_ASSERT(alloc_info->bss_info.bsscfg != NULL);

	/* for default BSS use h/w idx index from bss keys only if rx is allowed */
	if (WLC_KEY_IS_DEFAULT_BSS(key_info)) {
		if (WLC_KEY_RX_ALLOWED(key_info))
			hw_idx = alloc_info->bss_info.hw_idx[key_info->key_id];
		goto done;
	}

#if defined(IBSS_PEER_GROUP_KEY)
	if (WLC_KEY_IS_IBSS(key_info)) {
		for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
			hw_idx_t hw_idx2 = alloc_info->scb_info.ibss_info.hw_idx[i];
			if (hw_idx2 != WLC_KEY_INDEX_INVALID) {
				hw_idx = hw_idx2 - ((i + 1) * WLC_KEYMGMT_IBSS_MAX_PEERS);
				KM_DBG_ASSERT(KM_HW_IDX_VALID(hw, hw_idx));
				break;
			}
		}
		goto done;
	}
#endif /* IBSS_PEER_GROUP_KEY */

	/* No hw idx for skl AP group key for non-default BSS */
	if (WLC_KEY_IS_AP(key_info)) {
		if (!WLC_KEY_IS_PAIRWISE(key_info) && WLC_KEY_RX_ALLOWED(key_info))
			hw_idx = alloc_info->bss_info.hw_idx[key_info->key_id];
		goto done;
	}

	if (KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(hw, alloc_info->bss_info.bsscfg)) {
		for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
			if (alloc_info->bss_info.hw_idx[i] != WLC_KEY_INDEX_INVALID) {
				KM_ASSERT(WLC_KEY_ID_IS_STA_GROUP(i));
				hw_idx = alloc_info->bss_info.hw_idx[i] - i;
				KM_DBG_ASSERT(KM_HW_IDX_VALID(hw, hw_idx));
				break;
			}
		}
	}

done:
	*out_hw_idx = hw_idx;
}

const struct ether_addr*
hw_amt_get_addr(km_hw_t *hw, const wlc_key_info_t *key_info,
	const wlc_bsscfg_t *bsscfg, const scb_t *scb)
{
	const struct ether_addr *ea;

	ea = &key_info->addr;
	if (ETHER_ISNULLADDR(ea)) {
		if (scb != NULL)    /* STA group keys */
			ea = &scb->ea;
		else
			ea = &bsscfg->BSSID;
	} else {
		/* psta entry gets primary ra */
		if (PSTA_ENAB(KM_HW_PUB(hw)) && BSSCFG_STA(bsscfg))
			ea = &bsscfg->cur_etheraddr;
	}

	return ea;
}
