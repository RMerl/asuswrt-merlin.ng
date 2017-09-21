/*
 * Key Management Module Implementation - iovar support
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_iovars.c 578700 2015-08-12 05:41:02Z $
 */

#include "km_pvt.h"

#ifdef WL_TBOW
#include <wlc_tbow.h>
#endif

/* wsec info tlv support */
typedef int (*km_iov_wsec_tlv_get_cb_t)(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max);
typedef int (*km_iov_wsec_tlv_set_cb_t)(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const wl_wsec_info_tlv_t *req);

#define DECL_KM_IOV_WSEC_TLV_GET_CB(_name) \
static int km_iov_get_wsec_info_ ## _name(keymgmt_t *km, wlc_bsscfg_t *bsscfg, \
    const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_end)

#define DECL_KM_IOV_WSEC_TLV_SET_CB(_name) \
static int km_iov_set_wsec_info_ ## _name(keymgmt_t *km, wlc_bsscfg_t *bsscfg, \
    const wl_wsec_info_tlv_t *req)

DECL_KM_IOV_WSEC_TLV_GET_CB(max_keys);
DECL_KM_IOV_WSEC_TLV_GET_CB(bss_key_len);
DECL_KM_IOV_WSEC_TLV_GET_CB(bss_algo);
DECL_KM_IOV_WSEC_TLV_GET_CB(tx_key_id);

struct km_iov_wsec_tlv_ent {
	wl_wsec_info_type_t type;
	uint16 min_req_len;
	uint16 max_req_len;
	km_iov_wsec_tlv_get_cb_t get_cb;
	km_iov_wsec_tlv_set_cb_t set_cb;
};
typedef struct km_iov_wsec_tlv_ent km_iov_wsec_tlv_ent_t;

static int
km_iov_get_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint8 *outbuf, size_t buf_len);
static const km_iov_wsec_tlv_ent_t*
km_iov_get_tlv_ent(keymgmt_t *km, wl_wsec_info_type_t type);

/* wsec info tlv entries */
static const km_iov_wsec_tlv_ent_t km_iov_wsec_tlv_entries[] = {
	{WL_WSEC_INFO_MAX_KEYS, 0, 0, km_iov_get_wsec_info_max_keys, NULL},
	{WL_WSEC_INFO_BSS_KEY_LEN, 0, 0, km_iov_get_wsec_info_bss_key_len, NULL},
	{WL_WSEC_INFO_BSS_ALGO, 0, 0, km_iov_get_wsec_info_bss_algo, NULL},
	{WL_WSEC_INFO_BSS_TX_KEY_ID, 0, 0, km_iov_get_wsec_info_tx_key_id, NULL}
};

static const size_t km_iov_wsec_tlv_num_entries =
    sizeof(km_iov_wsec_tlv_entries)/sizeof(km_iov_wsec_tlv_entries[0]);

/* iovar table - includes vars that may be unsupported to minimize rom inval */
const bcm_iovar_t km_iovars[] = {
	{"wsec_key", IOV_WSEC_KEY, (IOVF_OPEN_ALLOW), IOVT_BUFFER, sizeof(wl_wsec_key_t)},
	{"wsec_key_seq", IOV_WSEC_KEY_SEQ, (IOVF_OPEN_ALLOW), IOVT_BUFFER, KEY_SEQ_SIZE},
	{"buf_key_b4_m4", IOV_BUF_KEY_B4_M4, (0), IOVT_BOOL, 0},
	{"wapi_hw_enabled", IOV_WAPI_HW_ENABLED, (0), IOVT_BOOL, 0},
	{"brcmapivtwo", IOV_BRCMAPIVTW_OVERRIDE, IOVF_RSDB_SET, IOVT_INT8, 0},
	{"wsec_info", IOV_WSEC_INFO, (0), IOVT_BUFFER, sizeof(wl_wsec_info_t)},
	{NULL, 0, 0, 0, 0}
};

int
km_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params, uint p_len,
	void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_bsscfg_t *bsscfg;
	int err = BCME_OK;
	keymgmt_t *km = (keymgmt_t *)ctx;
	int val;
	int32 *ret_int_ptr;
	km_bsscfg_t *bss_km;
	wlc_info_t *wlc;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	wlc_key_flags_t key_flags;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_DBG_ASSERT(KM_VALID(km));

	KM_TRACE(("wl%d: %s: enter\n",  KM_UNIT(km), __FUNCTION__));

	wlc = km->wlc;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	KM_DBG_ASSERT(bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);

	ret_int_ptr = (int32*)arg;
	if (p_len >= sizeof(val))
		memcpy(&val, params, sizeof(val));
	else
		val = 0;

	BCM_REFERENCE(bss_km);
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
#ifdef STA
	case IOV_GVAL(IOV_BUF_KEY_B4_M4):
	{
		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (len < sizeof(int32)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		*ret_int_ptr = KM_BSSCFG_B4M4_ENABLED(bss_km);
		break;
	}
	case IOV_SVAL(IOV_BUF_KEY_B4_M4):
	{
		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (p_len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		km_b4m4_set(km, bsscfg, val != 0);
		break;
	}
#endif /* STA */
	case IOV_SVAL(IOV_WSEC_KEY):
	{
		wl_wsec_key_t wl_key, *out_wl_key = (wl_wsec_key_t *)arg;
		scb_t *scb;
		wlc_key_id_t key_id;
		struct ether_addr *ea;

		/* check arg */
		if (len < sizeof(wl_key)) {
			err = BCME_BADARG;
			break;
		}

		memcpy(&wl_key, arg, sizeof(wl_key));
		key_id = (wlc_key_id_t)wl_key.index;
		ea = &wl_key.ea;

		KM_LOG(("wl%d: %s: key id %02x addr %s\n",  KM_UNIT(km), __FUNCTION__,
			key_id,  bcm_ether_ntoa(ea, eabuf)));

		/* disallow multicast addresses - unicast and broadcast okay */
		if (ETHER_ISMULTI(ea) && !ETHER_ISBCAST(ea)) {
			err = BCME_BADADDR;
			break;
		}

		/* group keys have null addr, lookup key */
		memset(&key_info, 0, sizeof(key_info));
		key_info.key_idx = WLC_KEY_INDEX_INVALID;
		if (ETHER_ISNULLADDR(ea) ||
			(BSSCFG_AP(bsscfg) && !eacmp(ea->octet, bsscfg->BSSID.octet))) {
			key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID)
				key =  wlc_keymgmt_get_key(km, key_id, &key_info);
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
				err = BCME_BADKEYIDX;
				break;
			}
			key_id = key_info.key_id;
		}

#ifdef  MFP
		if (WLC_MFP_ENAB(wlc->pub) && KM_VALID_MGMT_KEY_ID(key_id)) {
			uint16 lo;
			uint32 hi;
			size_t seq_len;
			uint8 seq[DOT11_WPA_KEY_RSC_LEN];

			/* must have  valid key info */
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
				err = BCME_BADKEYIDX;
				break;
			}

			/* handle update and removal */
			if (wl_key.len)
				err = wlc_key_set_data(key, CRYPTO_ALGO_BIP,
					wl_key.data, wl_key.len);
			else
				err = wlc_key_set_data(key, key_info.algo, NULL, 0);

			if (err != BCME_OK)
				break;

			if (!wl_key.len) /* nothing more for removal */
				break;

			/* Note: lo and hi are interchanged between wsec_key and igtk.
			 * this is is due to layout of KDE where lo is 32 bits
			 */
			lo = wl_key.rxiv.hi & 0xffff;
			hi = (wl_key.rxiv.lo << 16) | ((wl_key.rxiv.hi & 0xffff0000) >> 16);
			seq_len = wlc_key_pn_to_seq(seq, sizeof(seq), lo, hi);
			err = wlc_key_set_seq(key, seq, seq_len, WLC_KEY_SEQ_ID_ALL /* seq_id */,
				/* tx */ BSSCFG_AP(bsscfg));
			if ((wl_key.flags & WL_PRIMARY_KEY) || BSSCFG_AP(bsscfg)) {
				err = wlc_keymgmt_set_bss_tx_key_id(km,
					bsscfg, key_info.key_id, TRUE);
				if (err != BCME_OK)
					break;
			}
			break;
		}
#endif /* MFP */

		/* if removal, lookup based on key and remove it */
		if (!wl_key.len) {
			if (WLC_KEY_IS_GROUP(&key_info)) {
				if (!KM_VALID_DATA_KEY_ID(key_id)) {
					err = BCME_BADKEYIDX;
					break;
				}
				err = wlc_key_reset(key);
			} else if (ETHER_ISBCAST(ea)) { /* reset all */
				struct scb_iter scbiter;
				FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
					wlc_keymgmt_reset(km, NULL, scb);
				}
				wlc_keymgmt_reset(km, bsscfg, NULL);
			} else {
				scb = km_find_scb(km, bsscfg, ea, FALSE /* !create */);
				if (!scb) {
					err = BCME_NOTFOUND;
					break;
				}

				key_flags = WLC_KEY_FLAG_NONE;
#ifdef IBSS_PEER_GROUP_KEY
				if (KM_IBSS_PGK_ENABLED(km) &&
					(wl_key.flags & WL_IBSS_PEER_GROUP_KEY))
					key_flags |= WLC_KEY_FLAG_IBSS_PEER_GROUP;
#endif /* IBSS_PEER_GROUP_KEY */
				key = wlc_keymgmt_get_scb_key(km, scb, key_id, key_flags, NULL);
				err = wlc_key_reset(key);
				if (err != BCME_OK)
					break;
			}
			break;
		}

		/* handle key addition/update */
		key = km_find_key(km, bsscfg, ea, key_id,
			((wl_key.flags & WL_IBSS_PEER_GROUP_KEY) ?
			WLC_KEY_FLAG_IBSS_PEER_GROUP : 0), &key_info);

		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			KM_ERR(("wl%d: %s: status %d, key lookup failed.\n",
				KM_UNIT(km), __FUNCTION__, err));
			break;
		}

#ifdef BCMWAPI_WPI
		/* WAPI key rotation support */
		if ((wl_key.algo == CRYPTO_ALGO_SMS4) && WLC_KEY_IS_PAIRWISE(&key_info)) {
			scb_t *scb;
			km_scb_t *scb_km;
			wlc_key_t *cur_key;

			scb = wlc_keymgmt_get_scb(km, key_info.key_idx);
			KM_DBG_ASSERT(scb != NULL);

			/* nothing special if updating the primary key */
			scb_km = KM_SCB(km, scb);
			if (scb_km->key_idx != key_info.key_idx) {
				cur_key = wlc_keymgmt_get_key(km, scb_km->key_idx, NULL);
				key = km_key_wapi_rotate_key(cur_key, key, &key_info);
			}
		}
#endif /* BCMWAPI_WPI */

		if  (wl_key.algo == CRYPTO_ALGO_OFF) {
			switch (wl_key.len) {
			case WEP1_KEY_SIZE:  wl_key.algo = CRYPTO_ALGO_WEP1; break;
			case WEP128_KEY_SIZE:  wl_key.algo = CRYPTO_ALGO_WEP128; break;
			case TKIP_KEY_SIZE: wl_key.algo = CRYPTO_ALGO_TKIP; break;
			case AES_KEY_SIZE: wl_key.algo = CRYPTO_ALGO_AES_CCM; break;
			default:
				err = BCME_BADLEN;
				KM_ERR(("wl%d: %s: status %d, "
					"can not deduce algo from key len %d.\n",
					KM_UNIT(km), __FUNCTION__, err, wl_key.len));
				break;
			}
		}

		if (err != BCME_OK)
			break;

#ifdef STA
		/* b4m4: keys set before M4 is sent may be buffered */
		if (BSSCFG_STA(bsscfg) && (bsscfg->WPA_auth != WPA_AUTH_DISABLED)) {
			err = km_b4m4_buffer_key(km, bsscfg, &wl_key);
			if (err == BCME_OK)
				break;
			/* not enabled or failed, install the key */
		}
#endif /* STA */

		/* update key algorithm and data */
		err = wlc_key_set_data(key, (wlc_key_algo_t)wl_key.algo, wl_key.data, wl_key.len);
		if (err != BCME_OK)
			break;

#ifdef WL_TBOW
		if (TBOW_ENAB(wlc->pub)) {
			uint32 frate;
			frate = tbow_ho_connection_done(wlc->tbow_info, bsscfg, &wl_key);
			if (frate) {
				wlc_set_iovar_ratespec_override(wlc, WLC_BAND_2G, frate, FALSE);
			}
		}
#endif
		key_info.algo = (wlc_key_algo_t)wl_key.algo;

		/* update key id in key if necessary */
		if (key_id != key_info.key_id) {
			err = km_key_set_key_id(key, key_id);
			if (err != BCME_OK)
				break;
			key_info.key_id = key_id;
		}

		/* update rxiv */
		if (wl_key.iv_initialized) {
			uint8 seq[DOT11_IV_MAX_LEN];
			size_t seq_len;
			
			seq_len = wlc_key_pn_to_seq(seq, sizeof(seq),
				wl_key.rxiv.lo, wl_key.rxiv.hi);
			err = wlc_key_set_seq(key, seq, seq_len,
				WLC_KEY_SEQ_ID_ALL, FALSE /* !tx */);
			if (err != BCME_OK)
				break;
		}

		if (WLC_KEY_IS_GROUP(&key_info) &&
			((wl_key.flags & WL_PRIMARY_KEY) || BSSCFG_AP(bsscfg) || !bsscfg->BSS ||
			(WLC_KEY_IS_DEFAULT_BSS(&key_info) && KM_WEP_ALGO(key_info.algo)))) {
			bool tx_key_upd;
			wlc_key_info_t bss_ki;

			(void)wlc_keymgmt_get_bss_tx_key(km, bsscfg, FALSE, &bss_ki);
			tx_key_upd = ((wl_key.flags & WL_PRIMARY_KEY) ||
				bss_ki.algo == CRYPTO_ALGO_NONE);
			if (tx_key_upd) {
				err = wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, key_info.key_id,
					FALSE);
				if (err != BCME_OK)
					break;
			}
		}

		/* update flags if needed, after sync'ing since they may have changed above  */
		wlc_key_get_info(key, &key_info);
		key_flags = key_info.flags;

		if (bsscfg->BSS)
			key_flags &= ~WLC_KEY_FLAG_IBSS;
		else
			key_flags |= WLC_KEY_FLAG_IBSS;

#if defined(BCMCCX) || defined(BCMEXTCCX)
		key_flags &= ~WLC_KEY_CCX_SETTABLE_FLAGS;
		if (wl_key.flags & (WL_CKIP_KP|WL_CKIP_MMH)) {
			if (wl_key.flags & WL_CKIP_KP)
				key_flags |= WLC_KEY_FLAG_CKIP_KP;
			if (wl_key.flags & WL_CKIP_MMH)
				key_flags |= WLC_KEY_FLAG_CKIP_MMH;
		}
#endif

		if ((key_info.algo != CRYPTO_ALGO_OFF) && WLC_KEY_IS_GROUP(&key_info)) {
			/* update rx flag. tx is updated by WL_PRIMARY_KEY handling above */
			if ((bsscfg->WPA_auth == WPA_AUTH_DISABLED) || BSSCFG_STA(bsscfg))
				key_flags |= WLC_KEY_FLAG_RX;
		}

		if (key_flags != key_info.flags)
			km_key_set_flags(key, key_flags);

		if (wl_key.index != key_info.key_idx) {
			KM_ASSERT(sizeof(out_wl_key->index) >= sizeof(wlc_key_index_t));
			memcpy(&out_wl_key->index, &key_info.key_idx,
				sizeof(wlc_key_index_t));
		}

		break;
	}
	case IOV_GVAL(IOV_WSEC_KEY_SEQ):
	{
		uint8 *seq = arg;
		wlc_key_id_t key_id;
		int seq_len;

		if (p_len < sizeof(val))
			return BCME_BADARG;

		key_id = (wlc_key_id_t)val;

		/* try bss first - including MFP/igtk; if invalid attempt to use
		 * arg as key index
		 */
		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID)
			key = wlc_keymgmt_get_key(km, (wlc_key_index_t)(val), &key_info);

		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		memset(seq, 0, len);
		seq_len = wlc_key_get_seq(key, seq, len, 0, TRUE /* tx */);
		if (seq_len < 0)
			err = BCME_BUFTOOSHORT;
		/* else okay */

		break;
	}
#if defined(BCMDBG)
	case IOV_SVAL(IOV_WSEC_KEY_SEQ): /* params <key idx:4x><tx:1x><seq id:2x>|<seq(LE):12> */
	{
		wlc_key_index_t key_idx;
		uint8 seq[KEY_SEQ_SIZE];
		wlc_key_seq_id_t seq_id;
		int i;
		bool tx;
		uint8 *p;

		if (p_len < (2*KEY_SEQ_SIZE + 7))  {
			err = BCME_BUFTOOSHORT;
			break;
		}

		p = (uint8 *)params;
		key_idx = km_hex2int(p[3], p[2]);
		key_idx |= km_hex2int(p[1], p[0]) << 8;

		tx = km_hex2int(p[4], '0');
		seq_id = km_hex2int(p[6], p[5]);

		for (i = 0; i < KEY_SEQ_SIZE; ++i) {
			int j;
			j =  7 + (i << 1);
			seq[i] = km_hex2int(p[j+1], p[j]);
		}

		key = wlc_keymgmt_get_key(km, key_idx, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		err = wlc_key_set_seq(key, seq, KEY_SEQ_SIZE, seq_id, tx);
		break;
	}
#endif /* BCMDBG */
#if defined(BCMDBG)
	case IOV_GVAL(IOV_WSEC_KEY):
	{
		int wl_idx;
		wl_wsec_key_t wl_key;
		size_t wl_key_len;

		if (len < sizeof(wl_key)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(&wl_key, arg, sizeof(wl_key));
		wl_idx = wl_key.index;

		key = wlc_keymgmt_get_key(km, (wlc_key_index_t)wl_idx, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		memset(&wl_key, 0, sizeof(wl_key));
		wl_key.index = key_info.key_id;
		wl_key.len = sizeof(wl_key.data);
		wl_key.algo = key_info.algo;
		wl_key.ea = key_info.addr;

		err = wlc_key_get_data(key, wl_key.data, wl_key.len, &wl_key_len);
		if (err == BCME_UNSUPPORTED) {
			wl_key.len = 0;
			err = BCME_OK;
		} else if (err == BCME_OK) {
			wl_key.len = (uint32)wl_key_len;
		} else {
			break;
		}

		if (!WLC_KEY_IN_HW(&key_info))
			wl_key.flags |= WL_SOFT_KEY;
		if (WLC_KEY_IS_PRIMARY(&key_info))
			wl_key.flags |= WL_PRIMARY_KEY;
#ifdef BCMCCX
		if (key_info.flags & WLC_KEY_FLAG_CKIP_KP)
			wl_key.flags |= WL_CKIP_KP;
		if (key_info.flags & WLC_KEY_FLAG_CKIP_MMH)
			wl_key.flags |= WL_CKIP_MMH;
#endif /* BCMCCX */

		/* no iv returned */

		memcpy(arg, &wl_key, sizeof(wl_key));
		break;
	}
#endif 

#ifdef BCMWAPI_WPI
	case IOV_GVAL(IOV_WAPI_HW_ENABLED):
		*ret_int_ptr = (int32)WAPI_HW_WPI_ENAB(wlc->pub);
		break;
#endif

#ifdef BRCMAPIVTW
	case IOV_GVAL(IOV_BRCMAPIVTW_OVERRIDE):
		*ret_int_ptr = km_ivtw_get_mode(km->ivtw);
		break;
	case IOV_SVAL(IOV_BRCMAPIVTW_OVERRIDE):
		err = km_ivtw_set_mode(km->ivtw, val);
		break;
#endif /* BRCMAPIVTW */

	case IOV_GVAL(IOV_WSEC_INFO):
		err = km_iov_get_wsec_info(km, bsscfg, params, arg, (size_t)len);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	KM_TRACE(("wl%d: %s: exit status %d\n",  KM_UNIT(km), __FUNCTION__, err));
	return err;
}


static int
km_iov_get_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint8 *outbuf, size_t buf_len)
{
	wl_wsec_info_t *req;
	wl_wsec_info_t *rsp;
	int err;
	int i;
	uint8 *req_end;
	uint8 *rsp_max;
	size_t rsp_len;
	wl_wsec_info_tlv_t *req_tlv;
	wl_wsec_info_tlv_t *rsp_tlv;

	/* make a copy of request, and return response in the buffer */
	req = (wl_wsec_info_t *)MALLOCZ(KM_OSH(km), (uint)buf_len);
	if (!req) {
		err = BCME_NOMEM;
		goto done;
	}

	memcpy(req, params, buf_len);
	req_end = (uint8 *)req + buf_len;

	if (req->version != WL_WSEC_INFO_VERSION) {
		err = BCME_VERSION;
		goto done;
	}

	rsp = (wl_wsec_info_t *)outbuf;
	memset(rsp, 0, buf_len);
	rsp->version = WL_WSEC_INFO_VERSION;
	rsp_len = OFFSETOF(wl_wsec_info_t, tlvs);
	rsp_max = (uint8 *)rsp + buf_len;

	req_tlv = &req->tlvs[0];
	for (i = 0, err = BCME_OK; i < (int)req->num_tlvs; ++i) {
		uint16 tlv_len;
		wl_wsec_info_type_t tlv_type;
		const km_iov_wsec_tlv_ent_t *tlv_ent;

		if (req_end < req_tlv->data) {
			err = BCME_BADLEN;
			break;
		}

		tlv_type = (wl_wsec_info_type_t)ltoh16_ua(&req_tlv->type);
		tlv_len = ltoh16_ua(&req_tlv->len);
		tlv_ent = km_iov_get_tlv_ent(km, tlv_type);
		if  (!tlv_ent || !tlv_ent->get_cb) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (tlv_len < tlv_ent->min_req_len || tlv_len > tlv_ent->max_req_len) {
			err = BCME_BADLEN;
			break;
		}

		rsp_tlv = (wl_wsec_info_tlv_t *)((uint8 *)rsp + rsp_len);
		if (rsp_max < rsp_tlv->data) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		rsp_tlv->type = tlv_type;
		err = (*tlv_ent->get_cb)(km, bsscfg, req_tlv, rsp_tlv, rsp_max);
		if (err != BCME_OK)
			break;

		req_tlv = (wl_wsec_info_tlv_t *)((uint8 *)req_tlv +
			OFFSETOF(wl_wsec_info_tlv_t, data) + tlv_len);
		rsp_len += OFFSETOF(wl_wsec_info_tlv_t, data) + rsp_tlv->len;

		rsp_tlv->type = htol16(rsp_tlv->type);
		rsp_tlv->len = htol16(rsp_tlv->len);
		++(rsp->num_tlvs);
	}

done:
	if (req) {
		MFREE(KM_OSH(km), req, (uint)buf_len);
	}

	KM_LOG(("wl%d: %s: exit status %d\n",  KM_UNIT(km), __FUNCTION__, err));
	return err;
}

static const km_iov_wsec_tlv_ent_t *
km_iov_get_tlv_ent(keymgmt_t *km, wl_wsec_info_type_t type)
{
	size_t i;
	const km_iov_wsec_tlv_ent_t *ent = NULL;
	for (i = 0; i < km_iov_wsec_tlv_num_entries; ++i) {
		if (km_iov_wsec_tlv_entries[i].type == type) {
			ent = &km_iov_wsec_tlv_entries[i];
			break;
		}
	}
	return ent;
}

static int
km_iov_get_wsec_info_max_keys(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 max_keys;

	if (rsp_max < (rsp->data + sizeof(uint32))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	max_keys = (uint32)km_get_max_keys(km);
	rsp->len = (uint16)sizeof(max_keys);
	htol32_ua_store(max_keys, rsp->data);
done:
	return err;
}

static int
km_iov_get_wsec_info_bss_key_len(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 key_len;
	wlc_key_info_t key_info;

	if (rsp_max < (rsp->data + sizeof(uint32))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	wlc_keymgmt_get_bss_tx_key(km, bsscfg, FALSE, &key_info);
	key_len = (uint32)key_info.key_len;

	rsp->len = (uint16)sizeof(key_len);
	htol32_ua_store(key_len, rsp->data);
done:
	return err;
}

static int
km_iov_get_wsec_info_bss_algo(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 key_algo;

	if (rsp_max < (rsp->data + sizeof(uint32))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	key_algo = (uint32)wlc_keymgmt_get_bss_key_algo(km, bsscfg, FALSE);
	rsp->len = (uint16)sizeof(key_algo);
	htol32_ua_store(key_algo, rsp->data);
done:
	return err;
}

static int
km_iov_get_wsec_info_tx_key_id(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const wl_wsec_info_tlv_t *req, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 key_id;

	if (rsp_max < (rsp->data + sizeof(uint32))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	key_id = (uint32)wlc_keymgmt_get_bss_tx_key_id(km, bsscfg, FALSE);
	rsp->len = (uint16)sizeof(key_id);
	htol32_ua_store(key_id, rsp->data);
done:
	return err;
}
