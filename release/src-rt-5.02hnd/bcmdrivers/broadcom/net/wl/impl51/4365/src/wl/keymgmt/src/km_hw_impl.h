/*
 * Key Management Module  Implementation - private header shared by km_hw implementations
 * Copyright (c) 2013 Broadcom Corporation, All rights reserved.
 * $Id: km_hw_impl.h 556100 2015-05-12 17:38:50Z $
 */

#ifndef _km_hw_impl_h_
#define _km_hw_impl_h_

#include "km_hw.h"

#include <wlc_bmac.h>

/* hw_idx: this is similar to key_idx in wlc_key_info_t, except that it
 * is tightly coupled with hardware/ucode, where as the other one is not.
 * this implementation uses the index into h/w keys table. See
 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/Multi-MAC
 */
typedef wlc_key_hw_index_t hw_idx_t;
typedef km_amt_idx_t amt_idx_t;

typedef uint8	skl_idx_t;		/* index into SECKINDXALGO block */
typedef km_amt_attr_t	amt_attr_t;
typedef uint16	shm_addr_t;
typedef uint16	hwktab_addr_t;

#define KM_HW_SKL_IDX_INVALID 0xff
#define KM_HW_SKL_IDX_TXONLY 0xfe	/* reserved index */

#define KM_HW_MAX_DATA_LEN 32


typedef wlc_key_algo_t	key_algo_t;
typedef wlc_key_hw_algo_t hw_algo_t;

/*  max value for AMT/RCMTA.  */
#define KM_HW_IDX_MAX(_corerev) AMT_SIZE(_corerev)

/* bitmap of amt/rcmta index */
typedef amt_idx_t amt_idx_bitmap_t;

enum km_hw_flags {
	KM_HW_INITED		= 0x0001,	/* deferred init completed */
	KM_HW_DEFKEYS		= 0x0002,	/* defkeys are set for ucode */
	KM_HW_WOWL_HW		= 0x0004,	/* hw impl belongs to wowl */
	KM_HW_WOWL_ENABLED	= 0x0008,	/* wowl is currently enabled */
	KM_HW_RESET		= 0x0010,	/* h/w does not need reset */
	KM_HW_STA_BSSID_AMT	= 0x0020,	/* On a STA, use BSSID AMT for unicast */
	KM_HW_RXOE		= 0x0040,	/* non-legacy offloads */
};

typedef  uint16 km_hw_flags_t;

typedef struct km_hw_algo_entry km_hw_algo_entry_t;
struct km_hw_impl {
	km_hw_algo_entry_t	*algo_entries;
	uint8			num_algo_entries;
	void			*ctx;		/* additional impl context */
};

typedef struct km_hw_impl km_hw_impl_t;

struct km_hw {
	wlc_info_t	*wlc;
	hw_idx_t	max_idx;			/* 0.. max_idx - 1 are valid */
	amt_idx_t	max_amt_idx;
	uint8		max_key_size;
	skl_idx_t	unused[64];
	struct {
		uint8			unused[8];
		amt_idx_t		mcnx_start;
		uint8			mcnx_count;
	} amt_info;
	struct {
		shm_addr_t	key_base;
		shm_addr_t	skl_idx_base;
		shm_addr_t	tkip_mic_key_base;
		uint16		max_tkip_mic_keys;
		shm_addr_t	tkip_tsc_ttak_base;	/* tkip phase1 key + hi 32 of iv */
		uint16		max_tsc_ttak;
		shm_addr_t	rx_pn_base;		/* rxiv */
		uint16		max_rx_pn;
		shm_addr_t	tx_pn_base;		/* txiv */
		uint16		max_tx_pn;

		shm_addr_t	wapi_mic_key_base;
		uint16		max_wapi_mic_keys;
	} shm_info;

	uint8			flags;
	km_hw_impl_t		impl;
	amt_idx_bitmap_t	*used;			/* used slots */
	skl_idx_t		*skl;
	uint8 rand_def_key[KM_HW_MAX_DATA_LEN];
};

/* algorithm support */
typedef int (*km_hw_algo_read_data_t)(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
	wlc_key_data_type_t data_type, int ins, bool tx, uint8 *data,
	size_t data_size, size_t *data_len);
typedef int (*km_hw_algo_write_data_t)(km_hw_t *hw, void *ctx, hw_idx_t hw_idx,
	wlc_key_data_type_t data_type, int ins, bool tx,
	const wlc_key_info_t *key_info, const uint8 *data, size_t data_len);
typedef int (*km_hw_algo_destroy_t)(const km_hw_t *hw, void **ctx);

struct km_hw_algo_callbacks {
	km_hw_algo_read_data_t	read;
	km_hw_algo_write_data_t	write;
	km_hw_algo_destroy_t	destroy;
};

typedef struct km_hw_algo_callbacks km_hw_algo_callbacks_t;

typedef uint16 km_hw_dt_mask_t;

struct km_hw_algo_impl {
    hw_algo_t               hw_algo;
	const km_hw_algo_callbacks_t *cb;
	void *cb_ctx;
	km_hw_dt_mask_t dt_mask; /* supported data types */
};
typedef struct km_hw_algo_impl km_hw_algo_impl_t;

struct km_hw_algo_entry {
	key_algo_t		algo;
	km_hw_algo_impl_t	impl;
};

/* end algorithm support */

#define KM_HW_WLC(_hw) ((_hw)->wlc)
#define KM_HW_KM(_hw) (KM_HW_WLC(_hw)->keymgmt)
#define KM_HW_VALID(_hw) ((_hw) != NULL && \
	KM_HW_WLC(_hw) != NULL && \
	KM_HW_KM(_hw) != NULL)

#define KM_HW_UNIT(_hw) WLCWLUNIT(KM_HW_WLC(_hw))
#define KM_HW_PUB(_hw) (KM_HW_WLC(_hw)->pub)
#define KM_HW_REGS(_hw) (KM_HW_WLC(_hw)->regs)
#define KM_HW_OSH(_hw)  KM_HW_WLC(_hw)->osh

#ifndef BCM_OL_DEV
#define KM_HW_COREREV(_hw) (KM_HW_PUB(_hw)->corerev)
#define KM_HW_COPYTO_SHM(_wlc, _off, _buf, _len) \
	wlc_copyto_shm(_wlc, _off, _buf, _len)
#define KM_HW_COPYFROM_SHM(_wlc, _off, _buf, _len) \
	wlc_copyfrom_shm(_wlc, _off, _buf, _len)
#define KM_HW_SET_SHM(_wlc, _off, _val, _len) \
	wlc_set_shm(_wlc, _off, _val, _len)
#define KM_HW_WRITE_SHM(_wlc, _off, _val) \
	wlc_write_shm(_wlc, _off, _val)
#else
#define KM_HW_COREREV(_hw) si_corerev(KM_HW_PUB(_hw)->sih)
#define KM_HW_COPYTO_SHM(_wlc, _off, _buf, _len) \
	wlc_bmac_copyto_shm((_wlc)->hw, _off, _buf, _len)
#define KM_HW_COPYFROM_SHM(_wlc, _off, _buf, _len) \
	wlc_bmac_copyfrom_shm((_wlc)->hw, _off, _buf, _len)
#define KM_HW_SET_SHM(_wlc, _off, _val, _len) \
	wlc_bmac_set_shm((_wlc)->hw, _off, _val, _len)
#define KM_HW_WRITE_SHM(_wlc, _off, _val) \
	wlc_bmac_write_shm((_wlc)->hw, _off, _val)
#endif /* !BCM_OL_DEV */

#ifdef WL_HWKTAB
#define KM_HW_COPYTO_HWKTAB(_wlc, _off, _buf, _len) \
	wlc_copyto_keytbl(_wlc, _off, _buf, _len)
#define KM_HW_COPYFROM_HWKTAB(_wlc, _off, _buf, _len) \
	wlc_copyfrom_keytbl(_wlc, _off, _buf, _len)
#define KM_HW_SET_HWKTAB(_wlc, _off, _val, _len) \
	wlc_set_keytbl(_wlc, _off, _val, _len)
#define KM_HW_WRITE_HWKTAB(_wlc, _off, _val) \
	wlc_write_hwktab(_wlc, _off, _val)
#endif /* WL_HWKTAB */

#define KM_HW_KEYTAB_SUPPORTED(_hw) D11REV_GE(KM_HW_COREREV(_hw), 64)
#define KM_HW_COREREV_GE40(_hw) D11REV_GE(KM_HW_COREREV(_hw), 40)

#define KM_HW_IDX_VALID(_hw, _idx) ((_idx) < (_hw)->max_idx)

#define KM_HW_SKL_IDX(_hw, _idx)  ((_hw)->skl[(_idx)])
#define KM_HW_MAX_SKL_IDX(_hw) ((_hw)->max_amt_idx + WLC_KEYMGMT_NUM_GROUP_KEYS)
#define KM_HW_SKL_IDX_VALID(_hw, _idx) ((_idx) < KM_HW_MAX_SKL_IDX(_hw))
#define KM_HW_SKL_IDX_HAS_AMT(_hw, _idx) (\
	KM_HW_SKL_IDX_VALID((_hw), (_idx)) && \
	(_idx) != KM_HW_SKL_IDX_TXONLY && \
	(_idx)  >= WLC_KEYMGMT_NUM_GROUP_KEYS)
#define  KM_HW_SKL_IDX_TO_AMT(_skl_idx) ((_skl_idx) - WLC_KEYMGMT_NUM_GROUP_KEYS)

/* AMT/RCMTA validity - currently number of h/w keys slots is the same as
 * that of AMT/RCMTA table. Also, there are only 6 bits allocated for
 * h/w key index in skl table.
 */
#define KM_HW_AMT_IDX_VALID(_hw, _idx) ((_idx) < (_hw)->max_amt_idx)

/* address read from a ptr in shm */
#define KM_HW_SHM_ADDR_FROM_PTR(_km_hw, _ptr) (wlc_bmac_read_shm(\
	KM_HW_WLC(_km_hw)->hw, (_ptr)) << 1)

/* Address in shm for a given sec lookup index */
#define KM_HW_SKL_IDX_ADDR(_hw, _skl_idx) (((_hw)->shm_info.skl_idx_base) + \
	((_skl_idx) << 1))

/* address of key in shm for a given hw idx */
#define KM_HW_KEY_ADDR(_hw, _idx) (((_hw)->shm_info.key_base) + \
	D11_MAX_KEY_SIZE * (_idx))

#define KM_HWKTAB_DEF_SLOT_SIZE	16
#ifdef WL_HWKTAB
/* Use two slots to store keys */
#define KM_HWKTAB_SLOT_SIZE	(KM_HWKTAB_DEF_SLOT_SIZE << 1)	/* (2 * KM_HWKTAB_DEF_SLOT_SIZE) */
#define KM_HWKTAB_KEY_ADDR(_hw, _idx) (KM_HWKTAB_SLOT_SIZE * (_idx))
#else
#define KM_HWKTAB_SLOT_SIZE	KM_HWKTAB_DEF_SLOT_SIZE
#endif /* WL_HWKTAB */

/* address of rx seq shm for given hw idx - non WAPI, non-MFP */
#define  KM_HW_RX_SEQ_BLOCK_SIZE (KEY_SEQ_SIZE * WLC_KEY_BASE_RX_SEQ)
#define KM_HW_RX_SEQ_ADDR(_hw, _idx, _ins) (((_hw)->shm_info.rx_pn_base) + \
	(KM_HW_RX_SEQ_BLOCK_SIZE * (_idx)) + ((_ins) * KEY_SEQ_SIZE))


/* address in shm for a tkip mic key */
#define KM_HW_TKIP_MIC_KEY_SIZE (TKIP_KEY_SIZE >> 2)
#define KM_HW_TKIP_MIC_KEY_ADDR(_hw, _idx) (((_hw)->shm_info.tkip_mic_key_base) + \
    KM_HW_TKIP_MIC_KEY_SIZE * ((_idx) << 1))
#define KM_HW_TKIP_TX_MIC_KEY_ADDR(_hw, _idx) KM_HW_TKIP_MIC_KEY_ADDR(_hw, _idx)
#define KM_HW_TKIP_RX_MIC_KEY_ADDR(_hw, _idx) (KM_HW_TKIP_MIC_KEY_ADDR(_hw, _idx) + \
	KM_HW_TKIP_MIC_KEY_SIZE)

#ifdef WL_HWKTAB
#define KM_HWKTAB_TKIP_MIC_KEY_ADDR(_hw, _idx) (KM_HWKTAB_KEY_ADDR(_hw, _idx) + TKIP_TK_SIZE)
#define KM_HWKTAB_TKIP_TX_MIC_KEY_ADDR(_hw, _idx) KM_HWKTAB_TKIP_MIC_KEY_ADDR(_hw, _idx)
#define KM_HWKTAB_TKIP_RX_MIC_KEY_ADDR(_hw, _idx) (KM_HWKTAB_TKIP_MIC_KEY_ADDR(_hw, _idx) + \
	KM_HW_TKIP_MIC_KEY_SIZE)
#endif /* WL_HWKTAB */

#define KM_HW_RXOE(_hw) (((_hw)->flags & KM_HW_RXOE) != 0)
#define KM_HW_WOWL_SUPPORTED(_hw) ((_hw)->flags & KM_HW_WOWL_HW)
#define KM_HW_WOWL_ENABLED(_hw) ((_hw)->flags & KM_HW_WOWL_ENABLED)
#define KM_HW_LEGACY_WOWL(_hw) (KM_HW_WOWL_SUPPORTED(_hw) && !KM_HW_RXOE(_hw))

/* tkip tsc ttak - default keys have no ttak except for wowl */

#define KM_HW_TKIP_MAX_TSC_TTAK 50
#define KM_HW_TKIP_TSC_TTAK_IDX(_hw, _idx) (KM_HW_LEGACY_WOWL(_hw) ?  (_idx) :\
	((_idx) - WLC_KEYMGMT_NUM_GROUP_KEYS))

#define KM_HW_TKIP_TSC_TTAK_SUPPORTED(_hw, _idx) (\
	KM_HW_LEGACY_WOWL(_hw) ? ((uint16)(_idx) < (_hw)->shm_info.max_tsc_ttak) :\
	(((_idx) >= WLC_KEYMGMT_NUM_GROUP_KEYS) && \
		(_idx) <  (hw_idx_t)((_hw)->shm_info.max_tsc_ttak + WLC_KEYMGMT_NUM_GROUP_KEYS)))

#define KM_HW_TKIP_TSC_TTAK_SIZE (TKHASH_P1_KEY_SIZE + sizeof(uint32))
#define KM_HW_TKIP_TSC_TTAK_ADDR(_hw, _idx) ((_hw)->shm_info.tkip_tsc_ttak_base + \
	KM_HW_TKIP_TSC_TTAK_IDX(_hw, _idx) * KM_HW_TKIP_TSC_TTAK_SIZE)

#define KM_HW_MAX_TKMIC_KEYS(_hw) (WSEC_MAX_TKMIC_ENGINE_KEYS(KM_HW_COREREV(_hw)))
#define KM_HW_MAX_SMS4MIC_KEYS(_hw) (WSEC_MAX_SMS4MIC_ENGINE_KEYS(KM_HW_COREREV(_hw)))
/* address in shm for a wapi mic key */
#define KM_HW_SMS4_MIC_KEY_SIZE SMS4_WPI_CBC_MAC_LEN
#define KM_HW_SMS4_MIC_KEY_ADDR(_hw, _idx) (((_hw)->shm_info.wapi_mic_key_base) + \
	 KM_HW_SMS4_MIC_KEY_SIZE * (_idx))

#ifdef WL_HWKTAB
#define KM_HWKTAB_SMS4_MIC_KEY_SIZE (SMS4_WPI_CBC_MAC_LEN >> 1)
#define KM_HWKTAB_SMS4_MIC_KEY_ADDR(_hw, _idx) (KM_HWKTAB_KEY_ADDR(_hw, _idx) + \
	KM_HWKTAB_DEF_SLOT_SIZE)
#endif /* WL_HWKTAB */

#define KM_HW_DT_HW_MIC(_dt) (\
	(_dt) == WLC_KEY_DATA_TYPE_MIC_KEY_FROM_DS || \
	(_dt) == WLC_KEY_DATA_TYPE_MIC_KEY_TO_DS || \
	(_dt) == WLC_KEY_DATA_TYPE_MIC_KEY)

#define KM_HW_PRINT(args) WL_PRINT(args)
#define KM_HW_ERR(args) KM_ERR(args)
#define KM_HW_LOG(args) KM_LOG(args)
#define KM_HW_LOG_DECL(stmt) KM_LOG_DECL(stmt)
#define KM_HW_LOG_DUMP(stmt) KM_LOG_DUMP(stmt)

#define KM_HW_DT_MASK(_dt) (1 << (_dt))
#define KM_HW_DT_MASK2(_dt1, _dt2) (KM_HW_DT_MASK(_dt1) | KM_HW_DT_MASK(_dt2))
#define KM_HW_DT_MASK4(_dt1, _dt2, _dt3, _dt4) (KM_HW_DT_MASK2((_dt1), (_dt2)) | \
	KM_HW_DT_MASK2((_dt3), (_dt4)))
#define KM_HW_ALGO_DT_SUPPORTED(_ae, _dt) ((_ae)->impl.dt_mask & KM_HW_DT_MASK(_dt))

#define KM_HW_NEED_INIT(_hw) (!((_hw)->flags & KM_HW_INITED))
#define KM_HW_NEED_RESET(_hw) (!((_hw)->flags & KM_HW_RESET))

/* need h/w sta group keys, if s/w sta group keys are required except if the bss
 * has tkip as group algo, but we do not know that in advance when pairwise is
 * h/w key is created, so we allocate anyway. where as s/w allocation uses
 * sta group keys for ibss, h/w allocation does not use these keys that are allocated
 * along with pairwise h/w key
 */
#define KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(_hw, _bsscfg) ((_bsscfg)->BSS && \
	KM_HW_WLC(_hw)->cfg != (_bsscfg) && \
	BSSCFG_STA(_bsscfg) && !BSS_TDLS_ENAB(KM_HW_WLC(_hw), _bsscfg) && \
	!BSSCFG_PSTA(_bsscfg) && !((_bsscfg)->flags & WLC_BSSCFG_NOBCMC))

#define KM_HW_IS_DEFAULT_BSSCFG(_hw, _bsscfg) (KM_HW_WLC(_hw)->cfg == (_bsscfg))

/* for aes legacy, we program a slightly different hw algo into skl */
#define KM_HW_SKL_HW_ALGO(_hw, _key_info) (WLC_KEY_IS_AES_LEGACY(_key_info) ? \
	WSEC_ALGO_AES_LEGACY : (_key_info)->hw_algo)

#define KM_HW_IBSS_PGK_ENABLED(_hw) IBSS_PEER_GROUP_KEY_ENAB(KM_HW_PUB(_hw))

/* group keys on AP/IBSS used only for tx, no rx is expected. default bss tkip
 * group keys are tx only because there is no slot for phase1 key
 */
#define KM_HW_KEY_IS_TXONLY(_hw, _ki, _bsscfg) (WLC_KEY_IS_GROUP(_ki) && (\
	(WLC_KEY_IS_AP(_ki) && \
		(!WLC_KEY_IS_DEFAULT_BSS(_ki) || \
			((_bsscfg)->WPA_auth != WPA_AUTH_DISABLED))) ||\
	(WLC_KEY_IS_IBSS_GROUP(_ki) && \
			((_bsscfg)->WPA_auth != WPA_AUTH_DISABLED) &&\
			!((_bsscfg)->WPA_auth & WPA_AUTH_NONE)) ||\
	(((_ki)->algo == CRYPTO_ALGO_TKIP) && WLC_KEY_IS_DEFAULT_BSS(_ki))))

#define KM_HW_STA_USE_BSSID_AMT(_hw) (((_hw)->flags & KM_HW_STA_BSSID_AMT) != 0)

#define KM_HW_KEY_NUM_RX_SEQ(_hw) (KM_HW_PUB(_hw)->tunables->num_rxivs)

/* ibss peer group key slot, key ids map to slots 0..1 */
#define KM_HW_IBSS_PEER_GTK_IDX_OFFSET(_hw, _key_id) ((1 + ((_key_id) >> 1)) * \
	WLC_KEYMGMT_IBSS_MAX_PEERS)

#define KM_HW_IDX_TO_SLOT(_hw, _hw_idx) WLC_KM_HW_IDX_TO_SLOT(KM_HW_WLC(_hw), _hw_idx)

#define KM_HW_SKL_INDEX_MASK(_hw) WLC_KM_HW_SKL_INDEX_MASK(KM_HW_WLC(_hw))
#define KM_HW_SKL_GRP_ALGO_SHIFT(_hw) WLC_KM_HW_SKL_GRP_ALGO_SHIFT(KM_HW_WLC(_hw))
#define KM_HW_SKL_GRP_ALGO_MASK(_hw) WLC_KM_HW_SKL_GRP_ALGO_MASK(KM_HW_WLC(_hw))
/* hw algo support */
int km_hw_algo_init(const km_hw_t *hw, hw_algo_t hw_algo, km_hw_algo_impl_t *impl);
const km_hw_algo_entry_t* km_hw_find_algo_entry(const km_hw_t *hw, hw_algo_t hw_algo);
void km_hw_algo_destroy_algo_entries(km_hw_t *hw);
int km_hw_algo_set_hw_key(km_hw_t *hw, hw_idx_t hw_idx, const km_hw_algo_entry_t *ae,
    km_hw_dt_mask_t dt_mask, wlc_key_t *key, const wlc_key_info_t *key_info);
int km_hw_algo_update_sw_key(km_hw_t *hw, hw_idx_t hw_idx, const km_hw_algo_entry_t *ae,
    km_hw_dt_mask_t dt_mask, wlc_key_t *key, const wlc_key_info_t *key_info);
const uint8 * km_hw_fixup_null_hw_key(km_hw_t *hw, const uint8 *data, size_t data_len);
#ifdef WL_HWKTAB
extern const km_hw_algo_callbacks_t sms4_callbacks_hwktab;
extern const km_hw_algo_callbacks_t tkip_callbacks_hwktab;
extern const km_hw_algo_callbacks_t aes_callbacks_hwktab;
extern const km_hw_algo_callbacks_t wep_callbacks_hwktab;

void hw_algo_impl_init_hwktab(const km_hw_t *hw, km_hw_algo_impl_t *impl,
	key_algo_t algo, const km_hw_algo_callbacks_t *cb,
	void *cb_ctx, const km_hw_dt_mask_t dt_mask);
#endif /* WL_HWKTAB */

#endif /* _km_hw_impl_h_ */
