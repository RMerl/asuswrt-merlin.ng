/*
 * Key Management Module Implementation
 * Copyright (c) 2012-2013 Broadcom Corporation, All rights reserved.
 * $Id: km_event.c 431766 2013-10-24 16:26:34Z $
 */

#include "km_pvt.h"

/* internal interface */
#define KM_EVENT_INIT(_data, _ev, _km, _bsscfg, _key, _pkt) do {\
	memset((_data), 0, sizeof(wlc_keymgmt_event_data_t)); \
	(_data)->event = _ev; \
	(_data)->km = _km; \
	(_data)->bsscfg = _bsscfg; \
	(_data)->key = _key; \
	(_data)->pkt = _pkt; \
} while (0)

static bool
km_event_valid(wlc_keymgmt_event_t event)
{
	bool valid = TRUE;
	switch (event) {
	case WLC_KEYMGMT_EVENT_KEY_CREATED:
	/*  note: rekey event is an alias for key updated */
	case WLC_KEYMGMT_EVENT_KEY_UPDATED:
	case WLC_KEYMGMT_EVENT_KEY_DESTROY:
	case WLC_KEYMGMT_EVENT_DECODE_ERROR:
	case WLC_KEYMGMT_EVENT_DECRYPT_ERROR:
	case WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR:
	case WLC_KEYMGMT_EVENT_REPLAY:
	case WLC_KEYMGMT_EVENT_RESET:
	case WLC_KEYMGMT_EVENT_TKIP_CM_ACTIVE:
		break;

	default:
		valid = FALSE;
	}

	return valid;
}

static void
km_event_compat_signal(keymgmt_t *km, wlc_keymgmt_event_t event,
	wlc_bsscfg_t *bsscfg, wlc_key_t *key,
	const wlc_key_info_t *key_info, void *pkt)
{
	wlc_event_t *wlc_event;
	wlc_info_t *wlc;
	const struct ether_addr *ea;

	KM_DBG_ASSERT(key_info != NULL && bsscfg != NULL);

	switch (event) {
	case WLC_KEYMGMT_EVENT_DECODE_ERROR:
	case WLC_KEYMGMT_EVENT_DECRYPT_ERROR:
	case WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR:
		break;
	default:
		KM_LOG(("wl%d.%d: %s: ignoring event %d [%s]\n",
			KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
			__FUNCTION__, event, wlc_keymgmt_event_name(event)));
		return;
	}

	wlc = km->wlc;
	wlc_event = (wlc->eventq != NULL ? wlc_event_alloc(wlc->eventq) : NULL);
	if (wlc_event == NULL) {
		KM_ERR(("wl%d: %s: wlc event allocation failed\n", KM_UNIT(km), __FUNCTION__));
		return;
	}

	if (key_info->flags & (WLC_KEY_FLAG_IBSS_PEER_GROUP|WLC_KEY_FLAG_GROUP)) {
		wlc_event->event.flags |= WLC_EVENT_MSG_GROUP;
		ea = &bsscfg->current_bss->BSSID;
	} else {
		ea = &key_info->addr;
	}

	switch (event) {
	case WLC_KEYMGMT_EVENT_DECODE_ERROR:
		if (key_info->flags & (WLC_KEY_FLAG_IBSS_PEER_GROUP|WLC_KEY_FLAG_GROUP))
			wlc_event->event.event_type = WLC_E_MULTICAST_DECODE_ERROR;
		else
			wlc_event->event.event_type = WLC_E_UNICAST_DECODE_ERROR;
		break;
	case WLC_KEYMGMT_EVENT_DECRYPT_ERROR:
		wlc_event->event.event_type = WLC_E_ICV_ERROR;
		break;
	case WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR:
		wlc_event->event.event_type = WLC_E_MIC_ERROR;
		if (wlc_keymgmt_tkip_cm_enabled(km, bsscfg))
			wlc_event->event.flags |= WLC_EVENT_MSG_FLUSHTXQ;
		break;
	default:
		break;
	}

	wlc_event_if(wlc, bsscfg, wlc_event, ea);

	KM_LOG(("wl%d.%d: %s: event %d wlc event %d key idx %d\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, event,
		wlc_event->event.event_type, key_info->key_idx));

	wlc_process_event(wlc, wlc_event);
}

/* external interface */

void
km_event_signal(keymgmt_t *km, wlc_keymgmt_event_t event,
    wlc_bsscfg_t *bsscfg, wlc_key_t *key, void *pkt)
{
	wlc_keymgmt_event_data_t event_data;
	wlc_key_info_t key_info;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(key != NULL || bsscfg != NULL);
	if (!km_event_valid(event)) {
		KM_DBG_ASSERT(!"invalid event");
		return;
	}

	wlc_key_get_info(key, &key_info);
	if (key_info.key_idx != WLC_KEY_INDEX_INVALID && bsscfg == NULL) {
		KM_DBG_ASSERT(KM_VALID_KEY_IDX(km, key_info.key_idx));
		bsscfg = wlc_keymgmt_get_bsscfg(km, key_info.key_idx);
	}

	if (bsscfg == NULL)
		goto done;

#ifdef PSTA
	/* signal group key msdu mic error to each psta for tkip cm */
	if (PSTA_ENAB(KM_PUB(km)) && WLC_KEY_IS_GROUP(&key_info) &&
		event == WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR) {
		int psta_idx;
		wlc_bsscfg_t *psta_bsscfg;
		FOREACH_PSTA(km->wlc, psta_idx, psta_bsscfg) {
			km_event_compat_signal(km, event, psta_bsscfg, key,
				&key_info, pkt);
		}
	}
#endif

	km_event_compat_signal(km, event, bsscfg, key, &key_info, pkt);

	KM_EVENT_INIT(&event_data, event, km, bsscfg, key, pkt);
	(void)bcm_notif_signal(km->h_notif, &event_data);

done:
	KM_LOG(("wl%d.%d: %s: event %d [%s] key idx %d\n",
		KM_UNIT(km), (bsscfg != NULL ? WLC_BSSCFG_IDX(bsscfg) : -1), __FUNCTION__,
		event, wlc_keymgmt_event_name(event), key_info.key_idx));
}

int
wlc_keymgmt_event_register(keymgmt_t *km, wlc_keymgmt_event_t event,
	wlc_keymgmt_event_callback_t cb, void *cb_ctx)
{
	int err;

	KM_DBG_ASSERT(KM_VALID(km));

	if (!km_event_valid(event)) {
		err = BCME_BADARG;
		goto done;
	}

	err = bcm_notif_add_interest(km->h_notif,
		(bcm_notif_client_callback)cb, cb_ctx);

done:
	KM_LOG(("wl%d: %s: status %d: event %s, cb %p, ctx %p - done\n",
		KM_UNIT(km), __FUNCTION__, err,
		wlc_keymgmt_event_name(event), cb, cb_ctx));
	return err;
}

int wlc_keymgmt_event_unregister(keymgmt_t *km, wlc_keymgmt_event_t event,
    wlc_keymgmt_event_callback_t cb, void *cb_ctx)
{
	int err;

	KM_DBG_ASSERT(KM_VALID(km));

	if (!km_event_valid(event)) {
		err = BCME_BADARG;
		goto done;
	}

	err = bcm_notif_remove_interest(km->h_notif,
		(bcm_notif_client_callback)cb, cb_ctx);

done:
	KM_LOG(("wl%d: %s: status %d: event %s, cb %p, ctx %p - done\n",
		KM_UNIT(km), __FUNCTION__, err,
		wlc_keymgmt_event_name(event), cb, cb_ctx));
	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLMSG_WSEC)
const char*
wlc_keymgmt_event_name(wlc_keymgmt_event_t event)
{
	const char *name;

	switch (event) {
	case WLC_KEYMGMT_EVENT_KEY_CREATED: name =  "key-created"; break;
	/*  note: rekey event is an alias for key updated */
	case WLC_KEYMGMT_EVENT_KEY_UPDATED: name =  "key-updated"; break;
	case WLC_KEYMGMT_EVENT_KEY_DESTROY: name =  "key-destroy"; break;
	case WLC_KEYMGMT_EVENT_DECODE_ERROR: name =  "decode-error"; break;
	case WLC_KEYMGMT_EVENT_DECRYPT_ERROR: name =  "decrypt-error"; break;
	case WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR: name =  "msdu-mic-error"; break;
	case WLC_KEYMGMT_EVENT_REPLAY: name =  "replay"; break;
	case WLC_KEYMGMT_EVENT_RESET: name =  "reset"; break;
	case WLC_KEYMGMT_EVENT_TKIP_CM_ACTIVE: name =  "tkip-cm-active"; break;

	case WLC_KEYMGMT_EVENT_NONE:
	default:
		name = "invalid";
	}

	return name;
}
#endif /* BCMDBG || BCMDBG_DUMP || WLMSG_WSEC */
