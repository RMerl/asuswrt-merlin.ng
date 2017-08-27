/*
 * Key Management Module Implementation - dump support
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_dump.c 479458 2014-05-21 05:54:38Z $
 */

#include "km_pvt.h"

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

static int
km_dump_swkeys(keymgmt_t *km, struct bcmstrbuf *b)
{
	wlc_key_index_t key_idx;

	KM_DBG_ASSERT(KM_VALID(km));

	bcm_bprintf(b, "begin wl%d s/w key dump:\n", KM_UNIT(km));
	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key;
		wlc_key_info_t key_info;

		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;

		wlc_key_get_info(km_pvt_key->key, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF)
			continue;

		bcm_bprintf(b, "begin wlc_key@%d\n", key_idx);
		km_key_dump(km_pvt_key->key, b);
		bcm_bprintf(b, "end wlc_key@%d\n", key_idx);
	}
	bcm_bprintf(b, "end s/w key dump\n");
	return BCME_OK;
}

static int
km_dump_hwkeys(keymgmt_t *km, struct bcmstrbuf *b)
{
	KM_DBG_ASSERT(KM_VALID(km));
	if (!km->wlc->clk)
		return BCME_NOCLK;

	km_hw_dump(km->hw, b, KM_KEY_DUMP_HW_KEYS);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_HW_KEYS);
#endif
	return BCME_OK;
}

static int
km_dump_secalgo(keymgmt_t *km, struct bcmstrbuf *b)
{
	KM_DBG_ASSERT(KM_VALID(km));
	if (!km->wlc->clk)
		return BCME_NOCLK;

	km_hw_dump(km->hw, b, KM_KEY_DUMP_SECALGO);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_SECALGO);
#endif
	return BCME_OK;
}

static int
km_dump(keymgmt_t *km, struct bcmstrbuf *b)
{
	wlc_key_index_t key_idx;
	char eabuf[ETHER_ADDR_STR_LEN];

	KM_DBG_ASSERT(KM_VALID(km));

	bcm_bprintf(b, "begin wl%d keymgmt dump:\n", KM_UNIT(km));

	bcm_bprintf(b, "\tmagic: %08x\n", km->magic);
	bcm_bprintf(b, "\twlc: %p\n", km->wlc);
	bcm_bprintf(b, "\thw: %p\n", km->hw);
	bcm_bprintf(b, "\tflags: 0x%04x\n", km->flags);
	bcm_bprintf(b, "\th_bsscfg: %d\n", km->h_bsscfg);
	bcm_bprintf(b, "\th_scb: %d\n", km->h_scb);
	bcm_bprintf(b, "\th_notif: %p\n", km->h_notif);

	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key;
		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;
		bcm_bprintf(b, "begin km_pvt_key@%d\n", key_idx);

		bcm_bprintf(b, "\tkey: %p\n", km_pvt_key->key);
		bcm_bprintf(b, "\tflags: 0x%04x\n", km_pvt_key->flags);
		if (km_pvt_key->flags & KM_FLAG_SCB_KEY)
			bcm_bprintf(b, "\tscb@%p: %s in band %u\n", km_pvt_key->u.scb,
				bcm_ether_ntoa(&km_pvt_key->u.scb->ea, eabuf),
				km_pvt_key->u.scb->bandunit);
		if (km_pvt_key->flags & KM_FLAG_BSS_KEY)
			bcm_bprintf(b, "\tbss@%p bssidx %d\n",
				km_pvt_key->u.bsscfg, WLC_BSSCFG_IDX(km_pvt_key->u.bsscfg));
		bcm_bprintf(b, "\tkey algo: %d\n", km_pvt_key->key_algo);
		bcm_bprintf(b, "end km_pvt_key@%d\n", key_idx);
	}

	bcm_bprintf(b, "begin km stats\n");
	bcm_bprintf(b, "\tnum def bss wep: %d\n", km->stats.num_def_bss_wep);
	bcm_bprintf(b, "\tnum sw keys (algo != none): %d\n", km->stats.num_sw_keys);
	bcm_bprintf(b, "\tnum (km) hw keys: %d\n", km->stats.num_hw_keys);
	bcm_bprintf(b, "\tnum num bss up: %d\n", km->stats.num_bss_up);
	bcm_bprintf(b, "\tnum num pktfetch: %d\n", km->stats.num_pkt_fetch);
	bcm_bprintf(b, "end km stats\n");

	km_dump_swkeys(km, b);
	km_hw_dump(km->hw, b, KM_KEY_DUMP_ALL);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_ALL);
#endif
#ifdef BCMAPIVTW
	km_ivtw_dump(km->ivtw, b);
#endif
	bcm_bprintf(b, "end keymgmt dump\n");
	return BCME_OK;
}

/* public interface */
void
km_bsscfg_dump(void *ctx, wlc_bsscfg_t *bsscfg, struct bcmstrbuf *b)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_bsscfg_t *bss_km;
	int i;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);

	bcm_bprintf(b, "\twsec: 0x%08x\n", bss_km->wsec);
	for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
		bcm_bprintf(b, "\tdef_key_idx[%d]: %d\n", i, bss_km->key_idx[i]);
	}
	bcm_bprintf(b, "\ttx_key_id: %d\n", bss_km->tx_key_id);
	bcm_bprintf(b, "\tflags: %02x\n", bss_km->flags);


#ifdef MFP
	bcm_bprintf(b, "\tigtk_tx_key_id: %d\n", bss_km->igtk_tx_key_id);
	for (i = 0; i < WLC_KEYMGMT_NUM_BSS_IGTK; ++i) {
		bcm_bprintf(b, "\tigtk_key_idx[%d]: %d\n", i, bss_km->igtk_key_idx[i]);
	}
#endif

	bcm_bprintf(b, "\ttkip_cm_detected: %d\n", bss_km->tkip_cm_detected);
	bcm_bprintf(b, "\ttkip_cm_blocked: %d\n", bss_km->tkip_cm_blocked);
	bcm_bprintf(b, "\talgo (cached): %d\n", bss_km->algo);
	bcm_bprintf(b, "\tscb key idx: %d\n", bss_km->scb_key_idx);
	bcm_bprintf(b, "\tamt idx: %d\n", bss_km->amt_idx);
}

void
km_scb_dump(void *ctx, scb_t *scb, struct bcmstrbuf *b)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_scb_t *scb_km;

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);
	scb_km = KM_SCB(km, scb);

	bcm_bprintf(b, "\tflags 0x%04x\n", scb_km->flags);
	bcm_bprintf(b, "\tkey_idx %d\n", scb_km->key_idx);
#ifdef IBSS_PEER_GROUP_KEY
	if (KM_IBSS_PGK_ENABLED(km)) {
		int i;
		for (i = 0; i < WLC_KEYMGMT_NUM_STA_GROUP_KEYS; ++i) {
			int ix = scb_km->ibss_info.key_idx[i];
			bcm_bprintf(b, "\tibss key_idx[%d]: %d\n", i, ix);
		}
	}
#endif /* IBSS_PEER_GROUP_KEY */
	bcm_bprintf(b, "\tamt_idx %d\n", scb_km->amt_idx);
#ifdef BCMWAPI_WPI
	bcm_bprintf(b, "\tprev_key_idx (wapi) %d\n", scb_km->prev_key_idx);
#endif /* BCMWAPI_WPI */
}

int
km_register_dump(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));

#define REG(_km, _name, _func) (void)wlc_dump_register((_km)->wlc->pub, \
	(_name), (dump_fn_t)(_func), (void *)(_km))
	REG(km, KM_MODULE_NAME, km_dump);
	REG(km, "hwkeys", km_dump_hwkeys);
	REG(km, "swkeys", km_dump_swkeys);
	REG(km, "secalgo", km_dump_secalgo);
#undef REG
	return BCME_OK;
};

#endif /* BCMDBG || BCMDBG_DUMP */
