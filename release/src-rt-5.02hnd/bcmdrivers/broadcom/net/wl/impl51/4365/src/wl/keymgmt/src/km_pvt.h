/*
 * Key Management Module Implementation
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_pvt.h 672672 2016-11-29 10:58:39Z $
 */

/* This header file is private to keymgmt implementation */
#ifndef _km_pvt_h_
#define _km_pvt_h_

#include "km.h"
#include "km_hw.h"
#include "km_key.h"

#ifdef WOWL
#include "km_wowl_hw.h"
#endif /* WOWL */

#ifdef BRCMAPIVTW
#include "km_ivtw.h"
#else
typedef void km_ivtw_t;
#endif /* BRCMAPIVTW */

#ifdef WLRSDB
#define KM_SERIAL_SUPPORTED
#endif /* WLRSDB */

typedef struct km_bsscfg km_bsscfg_t;
typedef struct km_scb km_scb_t;

/* Begin constants */
#define KM_MAGIC		0x00736B6D
#define KM_BAD_MAGIC 	0xdeaddaed
#define KM_MODULE_NAME 	"keymgmt"

/* b4m4 support allows for buffering keys when 4-way M1 is received, and installing
 * the keys after M4 is transmitted
 */
#define KM_B4M4_NUM_KEYS		2
#define KM_B4M4_PAIRWISE_KEY_ID	0
#define KM_B4M4_GROUP_KEY_ID 	1

/* End constants */

/* Begin typedefs */

/* flags we track */
#define KM_FLAG_NONE			0x0000
#define KM_FLAG_VALID_KEY 		0x0001
#define KM_FLAG_BSS_KEY 		0x0002		/* belongs to BSS or IBSS */
#define KM_FLAG_TX_KEY 			0x0004
/*	no KM_FLAG_RX_KEY - always valid for RX */
#define KM_FLAG_STA_USE_BSSID_AMT	0x0008
#define KM_FLAG_IBSS_PEER_KEY	0x0010
#define KM_FLAG_SCB_KEY			0x0020
#define KM_FLAG_CREATED_KEY		0x0040	/* key creation complete */
#define KM_FLAG_SWONLY_KEY		0x0080
#define KM_FLAG_WLC_DOWN		0x0100  /* WLC is (going) down */
#define KM_FLAG_WOWL_DOWN		0x0200	/* WLC down w/ WOWL active */
#define KM_FLAG_IDX_ALLOC		0x4000	/* key idx allocation support */
#define KM_FLAG_DETACHING		0x8000	/* module detach support */
typedef uint16 km_flags_t;

struct km_pvt_key {
	wlc_key_t			*key;
	km_flags_t 			flags;
	union {
		wlc_bsscfg_t	*bsscfg;
		scb_t			*scb;
	} u;
	wlc_key_algo_t		key_algo;	/* key update support */
};
typedef struct km_pvt_key km_pvt_key_t;

typedef uint32 km_algo_mask_t;

/* Implementation data */
#ifndef BCM_OL_DEV
struct wlc_keymgmt {
	uint32			magic;
	wlc_info_t		*wlc;
	km_hw_t			*hw;
	km_flags_t		flags;
	uint16			max_keys;
	km_pvt_key_t	*keys;			/* points to array km_pvt_key_t[max_keys] */
	int				h_bsscfg;		/* bss cubby handle */
	int				h_scb;		/* scb cubby handle */
	bcm_notif_h		h_notif;
	wlc_key_t		*null_key;
	km_stats_t		stats;
	km_ivtw_t		*ivtw;
	km_hw_t			*wowl_hw;
	km_algo_mask_t	algo_unsup;		/* unsupported CRYPTO_* */
	km_algo_mask_t	algo_swonly;	/* swonly CRYPTO_* */
	km_key_cache_t	*key_cache;
};
#else
struct wlc_keymgmt {
	uint32			magic;
	wlc_info_t		*wlc;
	km_hw_t			*hw;
	km_flags_t		flags;
	uint16			max_keys;
	km_pvt_key_t	*keys;			/* points to array km_pvt_key_t[max_keys] */
	uint16			max_bsscfg;
	km_bsscfg_t		*bsscfg;
	uint16			max_scb;
	km_scb_t		*scb;
	wlc_key_t		*null_key;
	km_ivtw_t		*ivtw;
	km_algo_mask_t	algo_unsup;		/* unsupported CRYPTO_* */
	km_algo_mask_t	algo_swonly;	/* swonly CRYPTO_* */
};
#endif /* BCM_OL_DEV */

/* convenience typedefs */
typedef wlc_keymgmt_time_t km_time_t;

/* cubby note: B4M4 keys are for STA only; igtk keys are for MFP only.
 * IBSS key indicies are for IBSS only.  However from ROM invalidation
 * considerations a few fields are added unconditionally.
 * This overhead is similar to maintaining another cubby along with the feature.
 * and the numbers of these are not expected to change.
 */

/* bsscfg cubby */
enum {
	KM_BSSCFG_FLAG_NONE 	= 0x0000,
	KM_BSSCFG_FLAG_SWKEYS 	= 0x0001,
	KM_BSSCFG_FLAG_TKIP_CM	= 0x0002, 	/* tkip countermeasures are active */
	KM_BSSCFG_FLAG_B4M4		= 0x0004,
	KM_BSSCFG_FLAG_UP		= 0x0008,
	KM_BSSCFG_FLAG_M1RX		= 0x0010,
	KM_BSSCFG_FLAG_M4TX		= 0x0020,

	KM_BSSCFG_FLAG_WOWL_DOWN	= 0x0040,
	KM_BSSCFG_FLAG_CLEANUP		= 0x8000	/* in teardown */
};
typedef int16 km_bsscfg_flags_t;

struct km_bsscfg {
	uint32				wsec;
	wlc_key_index_t 	key_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
	wlc_key_id_t 		tx_key_id;
	km_bsscfg_flags_t	flags;

	wl_wsec_key_t 		*b4m4_keys[KM_B4M4_NUM_KEYS];	/* B4M4 support */

	wlc_key_id_t		igtk_tx_key_id;
	wlc_key_index_t		igtk_key_idx[WLC_KEYMGMT_NUM_BSS_IGTK];

	/* TKIP countermeasures */
	km_time_t	tkip_cm_detected;		/* time of detection */
	km_time_t	tkip_cm_blocked;		/* associations disallowed until */
	wlc_key_algo_t		algo;			/* multicast cipher for the BSS */
	wlc_key_index_t		scb_key_idx;	/* STA group key support */
	km_amt_idx_t		amt_idx;		/* amt allocated for bssid */
	km_amt_idx_t        cfg_amt_idx;	/* amt allocated for bsscfg */
};

enum {
	KM_BSSCFG_NO_CHANGE			= 0,
	KM_BSSCFG_WSEC_CHANGE		= 1,
	KM_BSSCFG_BSSID_CHANGE		= 2,
	KM_BSSCFG_ARM_TX			= 3,
	KM_BSSCFG_WOWL_DOWN			= 4,
	KM_BSSCFG_WOWL_UP			= 5
};

typedef int km_bsscfg_change_t;

/* serialization support */
#ifdef KM_SERIAL_SUPPORTED

#define KM_SERIAL_HDR_VERSION 0x0

#define KM_SERIAL_OBJ_VERSION_SHIFT 4
#define KM_SERIAL_OBJ_VERSION_MASK 0xf0
#define KM_SERIAL_OBJ_VERSION(_ver) (((_ver) & KM_SERIAL_OBJ_VERSION_MASK) >> \
	KM_SERIAL_OBJ_VERSION_SHIFT)
#define KM_SERIAL_VERSION(_hver, _over) (((_hver) | \
	(_over << KM_SERIAL_OBJ_VERSION_SHIFT)) & 0xff)

enum {
	KM_SERIAL_TLV_NONE			= 0,
	KM_SERIAL_TLV_KEY_ID 		= 1,
	KM_SERIAL_TLV_KEY_ALGO		= 2,
	KM_SERIAL_TLV_KEY_DATA		= 3,
	KM_SERIAL_TLV_KEY_ADDR 		= 4,
	KM_SERIAL_TLV_KEY_FLAGS 	= 5,
	KM_SERIAL_TLV_TX_SEQ		= 6,
	KM_SERIAL_TLV_RX_SEQ		= 7,
	KM_SERIAL_TLV_RSVD255 		= 255	/* add tlvs before this */
};

enum {
	KM_SERIAL_OBJ_TYPE_NONE = 0,
	KM_SERIAL_OBJ_TYPE_BSSCFG_STATIC = 1,
	KM_SERIAL_OBJ_TYPE_LAST		/* add types before this */
};

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct km_serial {
	uint8	version;
	uint8	obj_type;
	uint16	num_tlvs;
	uint8	tlvs[1];	/* variable length */
} BWL_POST_PACKED_STRUCT km_serial_t;
#include <packed_section_end.h>

#define KM_SERIAL_TLV_SIZE(_len) (TLV_HDR_LEN + (_len))

/* minimum size for the serialized obj; no empty tlvs */
#define KM_SERIAL_MIN_SIZE(_ser, _num_tlvs) (OFFSETOF(km_serial_t, tlvs) + \
	(_num_tlvs) * KM_SERIAL_TLV_SIZE(1))

#define KM_SERIAL_ENAB(_km) RSDB_ENAB(KM_PUB(_km))
#endif /* KM_SERIAL_SUPPORTED */

/* scb cubby */

struct km_ibss_scb {
	wlc_key_index_t	key_idx[WLC_KEYMGMT_NUM_GROUP_KEYS];
};

enum {
	KM_SCB_FLAG_NONE	= 0x0000,
	KM_SCB_FLAG_OWN_AMT	= 0x0001,
	KM_SCB_FLAG_INIT	= 0x0002
};
typedef int16 km_scb_flags_t;

struct km_scb {
	km_scb_flags_t flags;
	wlc_key_index_t	key_idx;
	struct km_ibss_scb ibss_info;
	wlc_key_index_t prev_key_idx;		/* WAPI */
	km_amt_idx_t amt_idx;
};

/* iov decls */
enum {
	IOV_WSEC_KEY 		= 1,
	IOV_WSEC_KEY_SEQ 	= 2,
	IOV_BUF_KEY_B4_M4 	= 3,
	IOV_WAPI_HW_ENABLED = 4,
	IOV_BRCMAPIVTW_OVERRIDE = 5,
	IOV_WSEC_INFO = 6
};

/* End typedefs */

/* Begin helper macros */
#define KM_UNIT(km) WLCWLUNIT((km)->wlc)
#define KM_OSH(km) ((km != NULL && km->wlc != NULL) ?  (km)->wlc->osh : NULL)
#define KM_PUB(km) ((km)->wlc->pub)
#define KM_CNT(km) (KM_PUB(km)->_cnt)
#define KM_HW(km) (km)->wlc->hw
#define KM_MCNX(km) (km)->wlc->mcnx
#define KM_PSTA(km) (km)->wlc->psta
#define KM_PCB(km) (km)->wlc->pcb
#define KM_WLC(km) ((km)->wlc)

#ifndef BCM_OL_DEV
#define KM_COREREV(_km) KM_PUB(_km)->corerev
#else
#define KM_COREREV(_km) si_corerev(KM_PUB(km)->sih)
#endif

#define KM_COREREV_GE40(km) D11REV_GE(KM_COREREV(km), 40)

#ifndef BCM_OL_DEV
#define KM_BSSCFG(_km, _bsscfg) (((_km) != NULL && (_bsscfg) != NULL) ? \
	(km_bsscfg_t *)BSSCFG_CUBBY((_bsscfg), (_km)->h_bsscfg) : NULL)
#define KM_CONST_BSSCFG(_km, _bsscfg) (((_km) != NULL && (_bsscfg) != NULL) ? \
	(const km_bsscfg_t *)CONST_BSSCFG_CUBBY((_bsscfg), (_km)->h_bsscfg) : NULL)

#define KM_SCB(_km, _scb) (((_km) != NULL && (_scb) != NULL) ? \
	(km_scb_t *)SCB_CUBBY((_scb), (_km)->h_scb) : NULL)
#define KM_NBANDS(_km) NBANDS((_km)->wlc)
#define KM_DEFAULT_BSSCFG(_km) (_km)->wlc->cfg
#define KM_IS_DEFAULT_BSSCFG(_km, _bsscfg) (KM_DEFAULT_BSSCFG(_km) == (_bsscfg))
#define KM_BSSCFG_ASSOCIATED(_bsscfg)((_bsscfg)->associated)
#else

#define KM_BSSCFG(_km, _bsscfg) (((_km) != NULL && (_bsscfg) != NULL && \
	(_bsscfg)->idx < (_km)->max_bsscfg) ?  &(_km)->bsscfg[(_bsscfg)->idx] : NULL)
#define KM_CONST_BSSCFG(_km, _bsscfg) (((_km) != NULL && (_bsscfg) != NULL && \
	 (_bsscfg)->idx < (_km)->max_bsscfg) ? \
		(const km_bsscfg_t *)&(_km)->bsscfg[(_bsscfg)->idx] : NULL)

#define KM_SCB(_km, _scb) (((_km) != NULL && (_scb) != NULL && \
	(_scb)->idx < (_km)->max_scb) ? &(_km)->scb[(_scb)->idx] : NULL)
#define KM_NBANDS(_km) KM_HW(_km)->_nbands
#define KM_DEFAULT_BSSCFG(_km) KM_WLC_BSSCFG((_km)->wlc, 0)
#define KM_BSSCFG_ASSOCIATED(_bsscfg) ((_bsscfg)->flags & WLC_OL_BSSCFG_F_ASSOCIATED)
#endif /* !BCM_OL_DEV */

#define KM_VALID(_km) ((_km) != NULL && (_km)->magic == KM_MAGIC &&\
	(_km)->wlc != NULL)
#define KM_VALID_DATA_KEY_ID(x) ((x) < WLC_KEYMGMT_NUM_GROUP_KEYS)
#define KM_VALID_MGMT_KEY_ID(x) ((x)  == WLC_KEY_ID_IGTK_1 || \
	(x) == WLC_KEY_ID_IGTK_2)
#define KM_SCB_RESOLVE_GROUP_KEY_ID(x) ((x) >> 1) /* only two group keys */
#define KM_VALID_KEY_IDX(km, ix) ((ix) != WLC_KEY_INDEX_INVALID &&\
	km != NULL && (ix) < (km)->max_keys)
#define KM_BSSCFG_TKIP_CM_ACTIVE(_km_bss, _now) ((_now) < (_km_bss)->tkip_cm_blocked)

#define KM_BSSCFG_B4M4_ENABLED(_km_bss) (((_km_bss)->flags & KM_BSSCFG_FLAG_B4M4) != 0)
#define KM_BSSCFG_M4TX_DONE(_km_bss) (((_km_bss)->flags & KM_BSSCFG_FLAG_M4TX) != 0)
#define KM_BSSCFG_M1RX_DONE(_km_bss) (((_km_bss)->flags & KM_BSSCFG_FLAG_M1RX) != 0)

#define KM_MFP_ENAB(_km) (WLC_MFP_ENAB(KM_PUB(_km)))

#define KM_VALID_KEY(k) ((k) != NULL && ((k)->flags & KM_FLAG_VALID_KEY) && \
	 ((k)->key != NULL))
#define KM_WLC_DEFAULT_BSSCFG(_km, _bsscfg) ((_km) != NULL &&\
	(_bsscfg) != NULL &&\
	KM_DEFAULT_BSSCFG(_km) == (_bsscfg))
#define KM_STA_USE_BSSID_AMT(_km) (((_km)->flags & KM_FLAG_STA_USE_BSSID_AMT) != 0)

/* Check that all pvt key data is clean when key is destroyed */
#define KM_ASSERT_KEY_DESTROYED(km, ix) do {\
	KM_DBG_ASSERT(km->keys[ix].key == NULL); \
	KM_DBG_ASSERT(km->keys[ix].flags == KM_FLAG_NONE); \
	KM_DBG_ASSERT(km->keys[ix].u.bsscfg == NULL); \
	KM_DBG_ASSERT(km->keys[ix].u.scb == NULL); \
} while (0)

/* position of key index based on key id */
#define KM_BSSCFG_GTK_IDX_POS(km, bsscfg, key_id) (KM_BSSCFG_IS_BSS(bsscfg) ? (\
	(KM_WLC_DEFAULT_BSSCFG(km, bsscfg) || (bsscfg)->WPA_auth == WPA_AUTH_DISABLED ||\
		!BSSCFG_STA(bsscfg)) ? (key_id) :\
		(1 + ((key_id) >> 1))) : (key_id))
#define KM_BSSCFG_IGTK_IDX_POS(key_id) ((key_id) - WLC_KEY_ID_IGTK_1)
#define KM_SCB_IBSS_PEER_GTK_IDX_POS(km, bsscfg, key_id) ((key_id) >> 1)
#define KM_SCB_OWN_AMT(_km_scb) (((_km_scb)->flags & KM_SCB_FLAG_OWN_AMT) != 0)

/* whether STA group keys are needed, ibss sta also uses these for tx */
#define KM_BSSCFG_NEED_STA_GROUP_KEYS(km, bsscfg) (\
	!KM_WLC_DEFAULT_BSSCFG(km, bsscfg) && BSSCFG_STA(bsscfg) && \
	!BSSCFG_PSTA(bsscfg) && !KM_BSSCFG_NOBCMC(bsscfg) && \
	!BSS_TDLS_ENAB((km)->wlc, bsscfg))

#define KM_OTHERBANDUNIT(_bandunit) (((_bandunit) == BAND_5G_INDEX) ? BAND_2G_INDEX : \
	BAND_5G_INDEX)

#define KM_IBSS_PGK_ENABLED(_km) IBSS_PEER_GROUP_KEY_ENAB(KM_PUB(_km))

#define KM_BSS_IS_UP(_bss_km) (((_bss_km)->flags & KM_BSSCFG_FLAG_UP) != 0)


#define KM_RXS_SECKINDX_MASK(_km) RXS_SECKINDX_MASK(KM_COREREV(_km))
#define KM_RXS_SECKINDX(_km, _hw_idx) (D11REV_GE(KM_COREREV(_km), 64) ?\
	((_hw_idx) >> 1) : (_hw_idx))

#define KM_BEFORE_NOW(_km, _t) ((_t) < (km_time_t)KM_PUB(_km)->now)
#define KM_AFTER_NOW(_km, _t) ((_t) > (km_time_t)KM_PUB(_km)->now)

typedef uint32 km_notif_mask_t;
#define KM_NOTIF_MASK(_n) (1 << (_n))

#ifdef STA
	#define KM_NOTIF_MASK_INTERNAL_STA 0
#else
	#define KM_NOTIF_MASK_INTERNAL_STA KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_M1_RX) |\
		KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_M4_TX)
#endif /* STA */

#ifdef WOWL
	#define KM_NOTIF_MASK_INTERNAL_WOWL 0
#else
	#define KM_NOTIF_MASK_INTERNAL_WOWL KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_WOWL) |\
		KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_WOWL_MICERR)
#endif /* WOWL */

#ifdef WLOFFLD
	#define KM_NOTIF_MASK_INTERNAL_WLOFFLD 0
#else
	#define KM_NOTIF_MASK_INTERNAL_WLOFFLD KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_OFFLOAD)
#endif /* WLOFFLD */

#define KM_NOTIF_MASK_INTERNAL (KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_BSS_UP) | \
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_BSS_DOWN) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_KEY_DELETE) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_KEY_UPDATE) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_DECODE_ERROR) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_DECRYPT_ERROR) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_MSDU_MIC_ERROR) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_SCB_CREATE) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_SCB_DESTROY) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_BSS_CREATE) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_BSS_DESTROY) |\
	KM_NOTIF_MASK(WLC_KEYMGMT_NOTIF_NONE) |\
	(KM_NOTIF_MASK_INTERNAL_STA) |\
	(KM_NOTIF_MASK_INTERNAL_WOWL) |\
	(KM_NOTIF_MASK_INTERNAL_WLOFFLD) |\
	0)

#define KM_NOTIF_MASK_EXTERNAL (~KM_NOTIF_MASK_INTERNAL)
#define KM_NOTIIF_IS_INTERNAL(_n)  ((KM_NOTIF_MASK_INTERNAL & KM_NOTIF_MASK(_n)) != 0)
#define KM_NOTIIF_IS_EXTERNAL(_n)  (!KM_NOTIIF_IS_INTERNAL(_n))

/* End helper macros */

/* km internal variables */
extern const bcm_iovar_t km_iovars[];

/* Internal prototypes */

/* wlc watchdog/iovar callbacks */
int km_wdog(void *h);
int km_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid,
    const char *name, void *params, uint p_len,
    void *arg, int len, int val_size, struct wlc_if *wlcif);

/* ioctl support */
int km_register_ioctl(keymgmt_t *km);
int km_unregister_ioctl(keymgmt_t *km);

/* dump support */
int km_register_dump(keymgmt_t *km);

/* keymgmt for bsscfg */
int km_bsscfg_init(void *ctx, wlc_bsscfg_t *bsscfg);
void km_bsscfg_deinit(void *ctx,  wlc_bsscfg_t *bsscfg);
km_amt_idx_t km_bsscfg_get_amt_idx(keymgmt_t *km, const wlc_bsscfg_t *bsscfg);
#if defined BCMDBG || defined(BCMDBG_DUMP)
void km_bsscfg_dump(void *ctx, wlc_bsscfg_t *bsscfg, struct bcmstrbuf *b);
#else
#define km_bsscfg_dump NULL
#endif

#ifdef KM_SERIAL_SUPPORTED
size_t km_bsscfg_get_max_static_config_size(keymgmt_t *km);
int km_bsscfg_get_static_config(void *ctx, wlc_bsscfg_t *bsscfg,
	uint8 *data, int *len);
int km_bsscfg_set_static_config(void *ctx, wlc_bsscfg_t *bsscfg,
	const uint8 *data, int len);
int km_serial_parse_hdr(keymgmt_t *km, int obj_type, int obj_ver,
	uint8 *buf, size_t buf_len, int *num_tlvs);
#endif /* KM_SERIAL_SUPPORTED */

#ifndef BCM_OL_DEV
void km_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data);
#endif /* !BCM_OL_DEV */

void km_bsscfg_reset(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool all);
void km_bsscfg_reset_sta_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wlc_key_info_t *scb_key_info);
bool km_bsscfg_swkeys(keymgmt_t *km, const wlc_bsscfg_t *bsscfg);
void km_bsscfg_update(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	km_bsscfg_change_t change);

/* keymgmt for scb */
int km_scb_init(void *ctx, scb_t *scb);
void km_scb_deinit(void *ctx,  scb_t *scb);
void km_scb_reset(keymgmt_t *km, scb_t *scb);
km_amt_idx_t km_scb_amt_alloc(keymgmt_t *km, scb_t *scb);
void km_scb_amt_release(keymgmt_t *km, scb_t *scb);

#ifndef BCM_OL_DEV
void km_scb_state_upd(keymgmt_t *km, scb_state_upd_data_t *data);
#endif /* !BCM_OL_DEV */

#if defined BCMDBG || defined(BCMDBG_DUMP)
void km_scb_dump(void *ctx, scb_t *scb, struct bcmstrbuf *b);
#else
#define km_scb_dump NULL
#endif

/* b4m4 support - STA only */
void km_b4m4_set(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool enable);
void km_b4m4_reset_keys(keymgmt_t *km, wlc_bsscfg_t *bsscfg);
int km_b4m4_buffer_key(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const wl_wsec_key_t *wl_key);
void km_b4m4_notify(keymgmt_t *km, wlc_keymgmt_notif_t notif,
    wlc_bsscfg_t *bsscfg, scb_t *scb, wlc_key_t *key, void *pkt);

/* event support */
void km_event_signal(keymgmt_t *km, wlc_keymgmt_event_t event,
	wlc_bsscfg_t *bsscfg, wlc_key_t *key, void *p);

/* allocation support */
int km_alloc_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
	wlc_key_index_t key_idx[], size_t num_keys);
void km_free_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
	wlc_key_index_t key_idx[], size_t num_keys);
bool km_needs_hw_key(keymgmt_t *km, km_pvt_key_t *km_pvt_key,
	wlc_key_info_t *key_info);
void km_init_pvt_key(keymgmt_t *km, km_pvt_key_t *km_pvt_key, wlc_key_algo_t algo,
	wlc_key_t *key, km_flags_t flags, wlc_bsscfg_t *bsscfg, scb_t *scb);

/* utils */
void km_sync_scb_wsec(keymgmt_t *km, scb_t *scb,
	wlc_key_algo_t key_algo);
void km_null_key_deauth(keymgmt_t *km, scb_t *scb, void *pkt);
/* finds an scb in either band */
scb_t* km_find_scb(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *addr, bool create);
/* finds a key for an address. creates scb for unicast, if needed */
wlc_key_t* km_find_key(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *addr, wlc_key_id_t key_id,
	wlc_key_flags_t key_flags, wlc_key_info_t *key_info);

/* tkip countermeasures */
void km_tkip_mic_error(keymgmt_t *km, void *pkt, wlc_key_t *key,
	const wlc_key_info_t *key_info);
void km_tkip_cm_reported(keymgmt_t *km, wlc_bsscfg_t *bsscfg, scb_t *scb);

#ifdef BRCMAPIVTW
/* ivtw */
int km_bsscfg_ivtw_enable(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool enable);
#endif /* BRCMAPIVTW */

uint32 km_bsscfg_get_wsec(keymgmt_t *km, const wlc_bsscfg_t *bsscfg);
bool km_wsec_allows_algo(keymgmt_t *km, uint32 wsec, wlc_key_algo_t algo);

uint8 km_hex2int(uchar lo, uchar hi);

#ifdef WOWL
bool km_bsscfg_wowl_down(keymgmt_t *km, const wlc_bsscfg_t *bsscfg);
#else
#define km_bsscfg_wowl_down(_km, _bsscfg) FALSE
#endif /* WOWL */

#endif /* _km_pvt_h_ */
