/*
 * Key Management Module Implementation - ioctl support
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_ioctl.c 578389 2015-08-11 09:04:31Z $
 */

#include "km_pvt.h"

/* internal interface */

#define KM_IOCTL_IOCF_FLAGS (WLC_IOCF_OPEN_ALLOW)

static const wlc_ioctl_cmd_t km_ioctls[] = {
	{WLC_GET_KEY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_KEY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_GET_KEY_SEQ, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_GET_KEY_PRIMARY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_KEY_PRIMARY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_WSEC_TEST, KM_IOCTL_IOCF_FLAGS, sizeof(int)}
};

static int
km_ioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	keymgmt_t *km = (wlc_keymgmt_t*)ctx;
	wlc_info_t *wlc;
	int err = BCME_OK;
	int val;
	int *pval;
	wlc_bsscfg_t *bsscfg;
	wlc_key_id_t key_id;

	KM_DBG_ASSERT(KM_VALID(km)); /* wlcif may be NULL (primary) */
	wlc = km->wlc;
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	KM_DBG_ASSERT(bsscfg != NULL);

	pval = (int *)arg;
	if (pval != NULL && (uint32)len >= sizeof(val))
		memcpy(&val, pval, sizeof(val));
	else
		val = 0;

	KM_TRACE(("wl%d.%d: %s: cmd %d, val 0x%x (%d) len %d\n",
		WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
		cmd, val, val, len));

	switch (cmd) {
	case WLC_SET_KEY:
		err = km_doiovar(km, NULL, IOV_SVAL(IOV_WSEC_KEY), NULL,
			NULL, 0, arg, len, 0, wlcif);
		break;

#if defined(BCMDBG)
	case WLC_GET_KEY: {
		wl_wsec_key_t wl_key;

		if (len < sizeof(wl_key)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memset(&wl_key, 0, sizeof(wl_key));
		wl_key.index = val;
		err = km_doiovar(km, NULL, IOV_GVAL(IOV_WSEC_KEY), NULL, NULL, 0,
			&wl_key, sizeof(wl_key), 0, wlcif);
		if (err != BCME_OK)
			break;

		memcpy(arg, &wl_key, sizeof(wl_key));
		break;
	}
#endif 

#ifdef BCMDBG
	case WLC_SET_WSEC_TEST: {
		wl_wsec_key_t wl_key;
		wlc_key_t *key;
		wlc_key_info_t key_info;
		wlc_key_flags_t key_flags;

		if (!wlc->pub->up)  {
			err = BCME_NOTUP;
			break;
		}

		memcpy(&wl_key, ((uchar*)arg)+sizeof(val), sizeof(wl_key));

		key = wlc_keymgmt_get_key_by_addr(wlc->keymgmt, bsscfg, &wl_key.ea,
			wl_key.flags & WL_IBSS_PEER_GROUP_KEY, &key_info);

		key_flags = key_info.flags;
		switch (val) {
		case WSEC_GEN_MIC_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MIC_ERR;
			break;
		case WSEC_GEN_REPLAY:
			key_flags |= WLC_KEY_FLAG_GEN_REPLAY;
			break;
		case WSEC_GEN_ICV_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_ICV_ERR;
			break;
		case WSEC_GEN_MFP_ACT_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_ACT_ERR;
			break;
		case WSEC_GEN_MFP_DISASSOC_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_DISASSOC_ERR;
			break;
		case WSEC_GEN_MFP_DEAUTH_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_DEAUTH_ERR;
			break;
		default:
			err = BCME_RANGE;
			break;
		}

		if (err != BCME_OK)
			break;

		err = wlc_key_set_flags(key, key_flags);
		break;
	}
#endif /* BCMDBG */
	case WLC_GET_KEY_SEQ:
		err = km_doiovar(km, NULL, IOV_GVAL(IOV_WSEC_KEY_SEQ), NULL, &val, sizeof(val),
			arg, len, 0, wlcif);
		break;
	case WLC_GET_KEY_PRIMARY:
		if (len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		key_id = wlc_keymgmt_get_bss_tx_key_id(wlc->keymgmt, bsscfg, FALSE);
		if (pval != NULL)
			*pval = key_id == val ? TRUE : FALSE;
		else
			err = BCME_BADARG;

		break;
	case WLC_SET_KEY_PRIMARY:
		if (len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		err = wlc_keymgmt_set_bss_tx_key_id(wlc->keymgmt, bsscfg, (wlc_key_id_t)val, FALSE);
		break;
	}

	if (err != BCME_OK) {
		if (VALID_BCMERROR(err))
			wlc->pub->bcmerror = err;
	}
	return err;
}

/* public interface */
int
BCMATTACHFN(km_register_ioctl)(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return wlc_module_add_ioctl_fn(KM_PUB(km), km, km_ioctl,
		ARRAYSIZE(km_ioctls), km_ioctls);
}

int
BCMATTACHFN(km_unregister_ioctl)(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return wlc_module_remove_ioctl_fn(KM_PUB(km), km);
}
