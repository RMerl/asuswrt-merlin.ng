/*
 * Security and Key Management Module
 * Internal interface to key management module
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km.h 607219 2015-12-18 07:03:46Z $
 */

#ifndef _km_h_
#define _km_h_

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <d11.h>
#include <siutils.h>
#include <bcmwpa.h>
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif /* WLBTAMP */

#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_alloc.h>
#include <wlc_pcb.h>

#ifndef BCM_OL_DEV
#include <bcm_notif_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_assoc.h>
#include <wlc_addrmatch.h>
#include <wlc_frmutil.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif /* WLMCNX */
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#if defined(WET) || defined(WET_DONGLE)
#include <wlc_wet.h>
#endif /* WET || WET_DONGLE */
#ifdef BCMCCX
#include <wlc_rm.h>
#include <wlc_ccx.h>
#endif /* BCMCCX */

#else
#include "km_ol.h"
#endif /* BCM_OL_DEV */

#include <wlc_keymgmt.h>
#include <wlc_key.h>

/* helper macros */

/* key id in body of pkt - non wapi */
#define KM_PKT_KEY_ID_BODY_OFFSET 3
#define KM_PKT_KEY_ID(_body) ((_body)[KM_PKT_KEY_ID_BODY_OFFSET]  >> \
	DOT11_KEY_INDEX_SHIFT)
#define KM_PKT_WAPI_KEY_ID_BODY_OFFSET 0
#define KM_PKT_WAPI_KEY_ID(_body) ((_body)[KM_PKT_WAPI_KEY_ID_BODY_OFFSET] & 0x1)
#define KM_SWAP(_type, _x, _y) {_type _tmp; _tmp = (_x); (_x) = (_y); (_y) = _tmp;}

#ifndef BCM_OL_DEV
#define KM_ERR(args) WL_ERROR(args)
#define KM_LOG(args) WL_WSEC(args)	/* use WSEC for logging */
#define KM_NONE(args) WL_NONE(args)
#define KM_TRACE(args) WL_TRACE(args)
#define KM_PRINTF(args) printf args
#if defined(BCMDBG) || defined(WLMSG_WSEC)
#define KM_LOG_DECL(stmt) stmt
#else
#define KM_LOG_DECL(stmt)
#endif /* BCMDBG || WLMSG_WSEC */

#if defined(BCMDBG_DUMP) || defined(BCMDBG)
#define KM_LOG_DUMP(stmt) if (WL_WSEC_DUMP_ON()) { stmt; }
#define KM_LOG_DUMP_PKT(_msg, _wlc, _pkt) if (WL_WSEC_DUMP_ON()) {\
	void *__pkt_tmp = (_pkt); \
	while (__pkt_tmp != NULL) {\
		uchar *__pkt_tmp_data = PKTDATA((_wlc)->osh, __pkt_tmp); \
		int __pkt_tmp_len = PKTLEN((_wlc)->osh, __pkt_tmp); \
		KM_LOG_DUMP(prhex(_msg, __pkt_tmp_data, __pkt_tmp_len)); \
		__pkt_tmp = PKTNEXT((_wlc)->osh, __pkt_tmp); \
	}\
}
#else
#define KM_LOG_DUMP(stmt)
#define KM_LOG_DUMP_PKT(_msg, _wlc, _pkt)
#endif /* BCMDBG_DUMP */
#endif /* !BCM_OL_DEV */

#define KM_WEP_ALGO(_algo) ((_algo) == CRYPTO_ALGO_WEP1 ||\
	 (_algo) == CRYPTO_ALGO_WEP128)

#define KM_SIZE_BITS(_type) (sizeof(_type) * NBBY)

#define KM_ADDR_IS_BCMC(_ea) (ETHER_ISMULTI(_ea) || ETHER_ISNULLADDR(_ea))

#define KM_IGNORED_SCB(_scb) (SCB_INTERNAL(_scb) ||\
	KM_ADDR_IS_BCMC(&(_scb)->ea))

#ifndef BCM_OL_DEV
#define KM_BSSCFG_IS_BSS(_bsscfg) (_bsscfg)->BSS
#define KM_BSSCFG_IS_IBSS(_bsscfg) (!KM_BSSCFG_IS_BSS(_bsscfg))
#define KM_BSSCFG_HAS_NATIVEIF(_bsscfg) BSSCFG_HAS_NATIVEIF(_bsscfg)
#define KM_BSSCFG_UP(_bsscfg) (_bsscfg)->up
#define KM_BSSCFG_NOBCMC(_bsscfg) ((_bsscfg)->flags & WLC_BSSCFG_NOBCMC)
#define KM_BSSCFG_WIN7PLUS(_pub, _bsscfg) WLEXTSTA_ENAB(_pub)

#define KM_SCB_LEGACY_AES(_scb) ((_scb != NULL) &&\
	((_scb)->flags & SCB_LEGACY_AES))
#define KM_SCB_MFP(_scb) SCB_MFP(_scb)
#define KM_SCB_CCX_MFP(_scb) SCB_CCX_MFP(_scb)
#define KM_SCB_WDS(_scb) SCB_WDS(_scb)
#define KM_SCB_WPA_SUP(_scb) ((_scb)->flags & SCB_WPA_SUP)
#define KM_WLC_BSSCFG(_wlc, _idx) WLC_BSSCFG(_wlc, _idx)

#ifdef BCMDBG
#define KM_DBG_ASSERT(_exp) ASSERT(_exp)
#define KM_ASSERT(_exp) ASSERT(_exp)
#else
#define KM_DBG_ASSERT(_exp)
#ifdef BCMDBG_ASSERT
#define KM_ASSERT(_exp) ASSERT(_exp)
#else
#define KM_ASSERT(_exp)
#endif /* BCMDBG_ASSERT */
#endif /* BCMDBG */

#endif /* !BCM_OL_DEV */

typedef uint8 km_amt_idx_t; /* index into AMT or rcmta */
typedef uint16 km_amt_attr_t;

#define WLC_KM_HW_IDX_TO_SLOT(_wlc, _hw_idx) (D11REV_GE((_wlc)->pub->corerev, 64) ? \
	((_hw_idx) << 1) : (_hw_idx))

#define WLC_KM_HW_SKL_INDEX_MASK(_wlc) (SKL_INDEX_MASK((_wlc)->pub->corerev))
#define WLC_KM_HW_SKL_GRP_ALGO_SHIFT(_wlc) (SKL_GRP_ALGO_SHIFT((_wlc)->pub->corerev))
#define WLC_KM_HW_SKL_GRP_ALGO_MASK(_wlc) (SKL_GRP_ALGO_MASK((_wlc)->pub->corerev))
/* end helper macros */

/* helper struct for h/w idx allocation */
struct km_alloc_key_info {
	struct {
		scb_t *scb;
		wlc_key_index_t key_idx;
		wlc_key_hw_index_t hw_idx;

		/* note: allocating space for all (4) keys to allow for future expansion
		 * (unlikely) without invalidating ROM code. Currently only two will be used and
		 * this structure is only used temporarily to aid h/w index allocation.
		 */
		struct {
			wlc_key_index_t key_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
			wlc_key_hw_index_t hw_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
		} ibss_info;
	} scb_info;
	struct {
		wlc_bsscfg_t *bsscfg;
		wlc_key_index_t key_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
		wlc_key_hw_index_t hw_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
		km_amt_idx_t amt_idx;
	} bss_info;
};

typedef struct km_alloc_key_info km_alloc_key_info_t;

/* internal stats - not currently exported from km */
typedef uint32 km_counter_t;

struct km_stats {
	km_counter_t num_def_bss_wep;	/* wep keys in default bss */
	km_counter_t num_sw_keys;		/* keys with non-none algo */
	km_counter_t num_hw_keys;		/* number of h/w keys created */
	km_counter_t num_bss_up;
	km_counter_t num_pkt_fetch;	/* number of pktfetch needed */
};

typedef struct km_stats km_stats_t;

/* interface */

/* check if unencrypted frame is allowed. for example 802.1x frames
 * need to be allowed before key are plumbed
 */
bool km_allow_unencrypted(wlc_keymgmt_t *km, const wlc_key_info_t *key_info,
	scb_t *scb, const struct dot11_header *hdr, uint16 qc,
	const uint8 *body, int body_len);

/* internal version of notify */
void km_notify(wlc_keymgmt_t *km, wlc_keymgmt_notif_t notif,
	struct wlc_bsscfg *bsscfg, scb_t *scb, wlc_key_t *key,
	void *pkt);

/* get h/w  idx for a key */
wlc_key_hw_index_t km_get_hw_idx(wlc_keymgmt_t *km, wlc_key_index_t key_idx);

/* get related key allocation info */
void km_get_alloc_key_info(wlc_keymgmt_t *km, wlc_key_index_t key_idx,
	km_alloc_key_info_t *alloc_key_info);

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLMSG_WSEC)
void km_get_hw_idx_key_info(wlc_keymgmt_t *km, wlc_key_hw_index_t hw_idx,
	wlc_key_info_t *key_info);
#endif /* BCMDBG || BCMDBG_DUMP || WLMSG_WSEC */

bool km_rxucdefkeys(wlc_keymgmt_t *km);

/* checks for replay. may clear GTK_RESET flag */
bool km_is_replay(wlc_keymgmt_t *km, wlc_key_info_t *key_info, int ins,
	uint8 *key_seq, uint8 *rx_seq, size_t seq_len);

/* update ivtw */
void km_update_ivtw(wlc_keymgmt_t *km, wlc_key_info_t *key_info, int ins,
	uint8 *rx_seq, size_t seq_len);

#ifdef BRCMAPIVTW
#define KM_UPDATE_IVTW(_km, _ki, _ins, _seq, _seq_len) km_update_ivtw(\
	_km, _ki, _ins, _seq, _seq_len)
#else
#define KM_UPDATE_IVTW(_km, _ki, _ins, _seq, _seq_len)
#endif /* BRCMAPIVTW */

/* get max keys supported */
size_t km_get_max_keys(wlc_keymgmt_t *km);

/* check if algo is supported */
bool km_algo_is_supported(wlc_keymgmt_t *km, wlc_key_algo_t algo);

/* check if algo is swonly */
bool km_algo_is_swonly(wlc_keymgmt_t *km, wlc_key_algo_t algo);

/* allocate amt idx for scb */
km_amt_idx_t km_scb_amt_alloc(wlc_keymgmt_t *km, scb_t *scb);

#if defined(PKTC) || defined(PKTC_DONGLE)
void km_reset_key_cache(wlc_keymgmt_t *km);
#define KM_RESET_KEY_CACHE(_km) km_reset_key_cache(_km);
#else
#define KM_RESET_KEY_CACHE(_km)
#endif /* PKTC || PKTC_DONGLE */

#endif /* _km_h_ */
