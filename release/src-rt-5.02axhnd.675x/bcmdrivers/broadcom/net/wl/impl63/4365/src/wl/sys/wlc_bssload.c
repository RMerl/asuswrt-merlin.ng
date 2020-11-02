/*
 * BSS load IE source file
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_bssload.c 709556 2017-07-07 17:58:36Z $
 */

/**
 * @file
 * @brief
 * Provide a mechanism to report an associated AP's BSS Load information from the beacon BSS Load
 * IE. Report this on demand via a read-only iovar. Report this asynchronously via a new
 * WLC_E_BSS_LOAD event when the BSS Load channel utilization field changes.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [BssLoadReport]
 */

#include <wlc_cfg.h>
#if defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT)
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_bmac.h>
#include <wlc_bssload.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>

#define MAX_CHAN_UTIL 255

/* iovar table */
enum {
	IOV_BSSLOAD,	/* enable/disable BSS load IE */
	IOV_BSSLOAD_STATIC,
	IOV_BSSLOAD_REPORT,
	IOV_BSSLOAD_REPORT_EVENT,
	IOV_LAST
};

static const bcm_iovar_t bssload_iovars[] = {
#ifdef WLBSSLOAD
	{"bssload", IOV_BSSLOAD, (0), IOVT_BOOL, 0},
	{"bssload_static", IOV_BSSLOAD_STATIC,
	0, IOVT_BUFFER, OFFSETOF(wl_bssload_static_t, is_static)},
#endif /* WLBSSLOAD */
#if defined(STA) && defined(WLBSSLOAD_REPORT)
	{"bssload_report", IOV_BSSLOAD_REPORT,
	0, IOVT_BUFFER, sizeof(wl_bssload_t),
	},
	{"bssload_report_event", IOV_BSSLOAD_REPORT_EVENT,
	0, IOVT_BUFFER, sizeof(wl_bssload_t),
	},
#endif /* defined(STA) && defined(WLBSSLOAD_REPORT) */
	{NULL, 0, 0, 0, 0},
};

/* BSSLOAD module info */
struct wlc_bssload_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
#ifdef WLBSSLOAD
	uint32 unused0[7];
#ifdef ISID_STATS
	uint32 unused1[4];
#endif /* ISID_STATS */
	uint8 chan_util;	/* channel utilization */
	wl_bssload_static_t bssload_static;
	cca_ucode_counts_t cca_stats;	/* cca stats from ucode */
#endif /* WLBSSLOAD */
};

/* local functions */
/* module */
static int wlc_bssload_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#ifdef WLBSSLOAD
static void wlc_bssload_watchdog(void *ctx);

/* IE mgmt */
static uint wlc_bssload_calc_qbss_load_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_bssload_write_qbss_load_ie(void *ctx, wlc_iem_build_data_t *data);
#endif /* WLBSSLOAD */

#if defined(STA) && defined(WLBSSLOAD_REPORT)
static int wlc_bssload_get_load(wlc_bsscfg_t *cfg, void *dst, int dstlen);
static int wlc_bssload_get_event_cfg(wlc_bsscfg_t *cfg, wl_bssload_cfg_t* dst, int dstlen);
static int wlc_bssload_set_event_cfg(wlc_bsscfg_t *cfg, wl_bssload_cfg_t* src, int srclen);
static int wlc_bssload_process_bl_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif /* defined(STA) && defined(WLBSSLOAD_REPORT) */

wlc_bssload_info_t *
BCMATTACHFN(wlc_bssload_attach)(wlc_info_t *wlc)
{
	wlc_bssload_info_t *mbssload;
#ifdef WLBSSLOAD
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif /* WLBSSLOAD */

	if (!wlc)
		return NULL;

	if ((mbssload = MALLOCZ(wlc->osh, sizeof(wlc_bssload_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	mbssload->wlc = wlc;

#ifdef WLBSSLOAD
	/* register IE mgmt callback */
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_QBSS_LOAD_ID,
	      wlc_bssload_calc_qbss_load_ie_len, wlc_bssload_write_qbss_load_ie,
	      mbssload) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, qbss load in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WLBSSLOAD */

	/* keep the module registration the last other add module unregistration
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, bssload_iovars, "bssload", mbssload, wlc_bssload_doiovar,
#ifdef WLBSSLOAD
		wlc_bssload_watchdog,
#else /* WLBSSLOAD */
		NULL,
#endif /* WLBSSLOAD */
		NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#ifdef WLBSSLOAD
	wlc->pub->_bssload = TRUE;
#endif /* WLBSSLOAD */

#if defined(STA) && defined(WLBSSLOAD_REPORT)
	{
		int err;
		if ((err = wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_QBSS_LOAD_ID,
			wlc_bssload_process_bl_ie, wlc)) != BCME_OK) {
			WL_ERROR(("wl%d: %s: add parse_fn failed, err %d\n",
				wlc->pub->unit, __FUNCTION__, err));
			goto fail;
		}
		wlc->pub->_bssload_report = TRUE;
	}
#endif /* defined(STA) && defined(WLBSSLOAD_REPORT) */
	return mbssload;

	/* error handling */
fail:
	if (mbssload != NULL)
		MFREE(wlc->osh, mbssload, sizeof(wlc_bssload_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_bssload_detach)(wlc_bssload_info_t *mbssload)
{
	wlc_info_t *wlc;

	if (mbssload) {
		wlc = mbssload->wlc;
		wlc_module_unregister(wlc->pub, "bssload", mbssload);
		MFREE(wlc->osh, mbssload, sizeof(wlc_bssload_info_t));
	}
}

static int
wlc_bssload_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_bssload_info_t *mbssload = (wlc_bssload_info_t *)ctx;
	wlc_info_t *wlc = mbssload->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
#ifdef WLBSSLOAD
	int32 *ret_int_ptr;
	bool bool_val;
#endif /* WLBSSLOAD */

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

#ifdef WLBSSLOAD
	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
#endif /* WLBSSLOAD */

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
#ifdef WLBSSLOAD
	case IOV_GVAL(IOV_BSSLOAD):
		*ret_int_ptr = (int32)wlc->pub->_bssload;
		break;
	case IOV_SVAL(IOV_BSSLOAD):
		wlc->pub->_bssload = bool_val;
		if (!wlc->pub->_bssload) {
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, TRUE);
		}
		break;
	case IOV_GVAL(IOV_BSSLOAD_STATIC): {
		wl_bssload_static_t *bssload_static = (wl_bssload_static_t *)arg;
		bcopy(&mbssload->bssload_static, bssload_static, sizeof(*bssload_static));
		break;
	}
	case IOV_SVAL(IOV_BSSLOAD_STATIC): {
		wl_bssload_static_t *bssload_static = (wl_bssload_static_t *)arg;
		bcopy(bssload_static, &mbssload->bssload_static, sizeof(*bssload_static));
		break;
	}
#endif /* WLBSSLOAD */

#if defined(STA) && defined(WLBSSLOAD_REPORT)
	case IOV_GVAL(IOV_BSSLOAD_REPORT):
		err = wlc_bssload_get_load(bsscfg, arg, len);
		break;

	case IOV_GVAL(IOV_BSSLOAD_REPORT_EVENT):
		err = wlc_bssload_get_event_cfg(bsscfg, (wl_bssload_cfg_t*) arg, len);
		break;
	case IOV_SVAL(IOV_BSSLOAD_REPORT_EVENT):
		err = wlc_bssload_set_event_cfg(bsscfg, (wl_bssload_cfg_t*) arg, len);
		break;
#endif /* defined(STA) && defined(WLBSSLOAD_REPORT) */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#ifdef WLBSSLOAD
static void wlc_bssload_get_chan_util(wlc_info_t *wlc, uint8 *chan_util,
	cca_ucode_counts_t *cca_stats)
{
	cca_ucode_counts_t tmp;
	cca_ucode_counts_t delta;
	uint32 cu;

	if (wlc_bmac_cca_stats_read(wlc->hw, &tmp))
		return;

	/* Calc delta */
	delta.txdur = tmp.txdur - cca_stats->txdur;
	delta.ibss = tmp.ibss - cca_stats->ibss;
	delta.obss = tmp.obss - cca_stats->obss;
	delta.noctg = tmp.noctg - cca_stats->noctg;
	delta.nopkt = tmp.nopkt - cca_stats->nopkt;
	delta.usecs = tmp.usecs - cca_stats->usecs;

	if (delta.usecs == 0)
		return;

	cu = delta.ibss + delta.txdur + delta.obss + delta.noctg + delta.nopkt;
	cu = cu * MAX_CHAN_UTIL / delta.usecs;
	if (cu > MAX_CHAN_UTIL)
		cu = MAX_CHAN_UTIL;
	*chan_util = (uint8)cu;

	/* Store raw values for next read */
	cca_stats->txdur = tmp.txdur;
	cca_stats->ibss = tmp.ibss;
	cca_stats->obss = tmp.obss;
	cca_stats->noctg = tmp.noctg;
	cca_stats->nopkt = tmp.nopkt;
	cca_stats->usecs = tmp.usecs;
}

static void wlc_bssload_watchdog(void *ctx)
{
	wlc_bssload_info_t *mbssload = (wlc_bssload_info_t *)ctx;
	wlc_info_t *wlc = mbssload->wlc;
	uint8 chan_util;	/* old channel utilization */

	if (WLBSSLOAD_ENAB(wlc->pub)) {
		chan_util = mbssload->chan_util;
		wlc_bssload_get_chan_util(wlc, &mbssload->chan_util, &mbssload->cca_stats);
		/* update beacon only when CU is changed */
		if ((chan_util != mbssload->chan_util) && wlc->pub->up) {
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, TRUE);
		}
	}
}

static uint
wlc_bssload_calc_qbss_load_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bssload_info_t *mbssload = (wlc_bssload_info_t *)ctx;
	wlc_info_t *wlc = mbssload->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!BSS_WLBSSLOAD_ENAB(wlc, cfg))
		return 0;

	return BSS_LOAD_IE_SIZE;
}

static int
wlc_bssload_write_qbss_load_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_bssload_info_t *mbssload = (wlc_bssload_info_t *)ctx;
	wlc_info_t *wlc = mbssload->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	uint8 *cp = data->buf;
	uint16 sta_count;
	uint8 chan_util;
	uint16 aac;

	if (!wlc->pub->_bssload || !BSS_WLBSSLOAD_ENAB(wlc, cfg))
		return BCME_OK;

	if (data->buf_len < BSS_LOAD_IE_SIZE) {
		WL_ERROR(("wl%d: %s: buffer is too small\n",
		          wlc->pub->unit, __FUNCTION__));
		return BCME_BUFTOOSHORT;
	}

	sta_count = wlc_bss_assocscb_getcnt(wlc, cfg);
	chan_util = mbssload->chan_util;
	aac = 0;

	if (mbssload->bssload_static.is_static) {
		sta_count = mbssload->bssload_static.sta_count;
		chan_util = mbssload->bssload_static.chan_util;
		aac = mbssload->bssload_static.aac;
	}

	cp[0] = DOT11_MNG_QBSS_LOAD_ID;
	cp[1] = BSS_LOAD_IE_SIZE - TLV_HDR_LEN;
	cp[2] = (uint8)(sta_count & 0xFF);	/* LSB - station count */
	cp[3] = (uint8)(sta_count >> 8);	/* MSB - station count */
	cp[4] = chan_util;	/* channel utilization */
	cp[5] = (uint8)(aac & 0xFF);	/* LSB - available admission capacity */
	cp[6] = (uint8)(aac >> 8);		/* MSB - available admission capacity */
	cp += BSS_LOAD_IE_SIZE;
	return BCME_OK;
}
#endif /* WLBSSLOAD */

#if defined(STA) && defined(WLBSSLOAD_REPORT)

/* Special value of event_util_level that indicates no previous stored value */
#define BSSLOAD_UTIL_LEVEL_UNDEFINED (MAX_BSSLOAD_RANGES + 1)

/* Reset saved data from the last received BSS Load IE and the last sent event */
void
wlc_bssload_reset_saved_data(wlc_bsscfg_t *cfg)
{
	if (cfg && cfg->bssload) {
		bzero(&cfg->bssload->load, sizeof(cfg->bssload->load));
		cfg->bssload->event_tstamp = OSL_SYSUPTIME();
		cfg->bssload->event_util_level = BSSLOAD_UTIL_LEVEL_UNDEFINED;
	}
}

#ifdef WLC_HIGH
static void
wlc_bssload_event_update(wlc_bsscfg_t *cfg)
{
	int level;
	wlc_info_t *wlc = cfg->wlc;
	wlc_bssload_t *bssld = cfg->bssload;
	uint32 curr_ms;
	uint32 delta_ms;

	/* If no utilization levels are configured then do not generate this event */
	if (bssld->event_cfg.num_util_levels == 0) {
		return;
	}

	/* If the time since the last sent event is < the configured rate limit,
	 * return without sending a BSS load change event.
	 * (This will be always false if no rate limit is configured which is
	 * represented by rate_limit_msec == 0.)
	 */
	curr_ms = OSL_SYSUPTIME();
	delta_ms = curr_ms - bssld->event_tstamp;
	if (delta_ms < bssld->event_cfg.rate_limit_msec) {
		return;
	}

	/* find channel utilization level */
	for (level = 0; level < bssld->event_cfg.num_util_levels; level++) {
		if (bssld->load.chan_util <= bssld->event_cfg.util_levels[level])
			break;
	}

	/* If the channel utilization level changed then post a bssload event */
	if (level != bssld->event_util_level) {
		wl_bssload_t value;

		value.sta_count = bssld->load.sta_count;
		value.aac = bssld->load.aac;
		value.chan_util = bssld->load.chan_util;
		bssld->event_util_level = (uint8)level;
		bssld->event_tstamp = curr_ms;
		WL_NONE(("WLC_E_BSS_LOAD c=%u u=%u a=%u, l=%u delta=%u\n",
			value.sta_count, value.chan_util, value.aac,
			bssld->event_util_level, delta_ms));
		wlc_bss_mac_event(wlc, cfg, WLC_E_BSS_LOAD, NULL, 0, 0, 0,
			&value, sizeof(value));
	}
}
#endif /* WLC_HIGH */

/* process BSS Load IE in beacon */
static int
wlc_bssload_process_bl_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_bsscfg_t *cfg = data->cfg;
	dot11_qbss_load_ie_t *ie;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!cfg->BSS)
		return BCME_OK;
	if (data->ie_len < 7)
		/* A valid BSS Load IE is 7 bytes long */
		return BCME_OK;
	if (data->ie == NULL)
		return BCME_OK;
	ie = (dot11_qbss_load_ie_t*) data->ie;

	/* Save the BSS Load IE data in our bsscfg */
	cfg->bssload->load.sta_count = ie->station_count;
	cfg->bssload->load.chan_util = ie->channel_utilization;
	cfg->bssload->load.aac = ie->aac;
	WL_NONE(("process BSS Load IE: c=%u u=%u a=%u\n",
		cfg->bssload->load.sta_count, cfg->bssload->load.chan_util,
		cfg->bssload->load.aac));

#ifdef WLC_HIGH
	wlc_bssload_event_update(cfg);
#endif /* WLC_HIGH */

	return BCME_OK;
}

/* Get the stored received beacon BSS Load IE data */
static int
wlc_bssload_get_load(wlc_bsscfg_t *cfg, void *dst, int dstlen)
{
	int err = BCME_OK;
	int srclen = sizeof(wl_bssload_t);

	if (!cfg->associated)
		return BCME_NOTASSOCIATED;

	if (dstlen < srclen) {
		err = BCME_BUFTOOSHORT;
	} else {
		bcopy(&cfg->bssload->load, dst, srclen);
	}
	return err;
}

/* Get the BSS Load reporting threshold */
static int
wlc_bssload_get_event_cfg(wlc_bsscfg_t *cfg, wl_bssload_cfg_t* dst, int dstlen)
{
	memcpy(dst, &cfg->bssload->event_cfg, sizeof(wl_bssload_cfg_t));
	return BCME_OK;
}

/* Set the BSS Load reporting threshold */
static int
wlc_bssload_set_event_cfg(wlc_bsscfg_t *cfg, wl_bssload_cfg_t* src, int srclen)
{
	if (src->num_util_levels > MAX_BSSLOAD_LEVELS)
		return BCME_RANGE;

	/* Reset the last received BSS Load IE and the last sent event */
	wlc_bssload_reset_saved_data(cfg);

	/* Store the new bssload event configuration */
	memcpy(&cfg->bssload->event_cfg, src, sizeof(wl_bssload_cfg_t));

	return BCME_OK;
}
#endif /* defined(STA) && defined(WLBSSLOAD_REPORT) */
#endif /* defined(WLBSSLOAD) || defined(WLBSSLOAD_REPORT) */
