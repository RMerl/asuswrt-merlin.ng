/*
 * EXTLOG Module implementation
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
 * $Id: wlc_extlog.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#ifdef WLC_HIGH_ONLY
#include <wlc_bmac.h>
#endif // endif
#include <wlc_extlog.h>

#ifndef WLEXTLOG
#error "WLEXTLOG is not defined"
#endif /* WLEXTLOG */

#ifndef MAX_EXTLOG_NUM
#define MAX_EXTLOG_NUM		32
#endif // endif

#ifdef WLC_HIGH
enum {
	IOV_EXTLOG_GET,
	IOV_EXTLOG_CLR,
	IOV_EXTLOG_CFG,
	};

static const bcm_iovar_t extlog_iovars[] = {
	{"extlog", IOV_EXTLOG_GET,
	(0), IOVT_BUFFER, 0,
	},
	{"extlog_clr", IOV_EXTLOG_CLR,
	(0), IOVT_VOID, 0
	},
	{"extlog_cfg", IOV_EXTLOG_CFG,
	(0), IOVT_BUFFER, sizeof(wlc_extlog_cfg_t),
	},
	{NULL, 0, 0, 0, 0 }
	};

static int wlc_extlog_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#endif /* WLC_HIGH */

void *
BCMATTACHFN(wlc_extlog_attach)(osl_t *osh, wlc_info_t *wlc)
{
	wlc_extlog_info_t *log_info = NULL;
	log_record_t *log_table = NULL;

	log_info = (wlc_extlog_info_t *)MALLOCZ(osh, sizeof(wlc_extlog_info_t));

	if (log_info == NULL)
		goto fail;

	log_info->wlc = wlc;
	log_info->osh = osh;
	log_info->cfg.max_number = MAX_EXTLOG_NUM;
	log_info->cfg.version = EXTLOG_CUR_VER;
	log_info->cfg.module = LOG_MODULE_COMMON | LOG_MODULE_ASSOC;
	log_info->cfg.level = WL_LOG_LEVEL_ERR;

#ifdef WLC_HIGH
	log_table = (log_record_t *)MALLOCZ(osh, MAX_EXTLOG_NUM * sizeof(log_record_t));
	if (log_table == NULL)
		goto fail;

	log_info->log_table = log_table;

	/* register module */
	if (wlc_module_register((wlc_pub_t*)wlc->pub, extlog_iovars, "extlog",
	                        log_info, wlc_extlog_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("%s: wlc_module_register failed!\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	return log_info;

fail:
	if (log_info) {
		MFREE(osh, log_info, sizeof(wlc_extlog_info_t));
	}
	if (log_table) {
		MFREE(osh, log_table, MAX_EXTLOG_NUM * sizeof(log_record_t));
	}

	return NULL;
}

int
BCMATTACHFN(wlc_extlog_detach)(wlc_extlog_info_t *wlc_extlog)
{
	if (!wlc_extlog)
		return -1;

#ifdef WLC_HIGH
	if (wlc_extlog->log_table) {
		MFREE(wlc_extlog->osh, wlc_extlog->log_table,
		sizeof(log_record_t) * wlc_extlog->cfg.max_number);
	}
	else
		return -1;

	wlc_module_unregister(wlc_extlog->wlc->pub, "extlog", wlc_extlog);
#endif // endif

	MFREE(wlc_extlog->osh, wlc_extlog, sizeof(wlc_extlog_info_t));

	return 0;
}

#ifdef WLC_HIGH
void
wlc_extlog_msg(wlc_info_t *wlc, uint16 module, uint8 id,
	uint8 level, uint8 sub_unit, int arg, char *str)
{
	wlc_extlog_info_t *wlc_extlog = (wlc_extlog_info_t *)wlc->extlog;
	log_record_t *cur_record;

	if ((module > (1 << (LOG_MODULE_MAX - 1))) || (level > WL_LOG_LEVEL_MAX)) {
		ASSERT(0);
		return;
	}

	if (!(wlc_extlog->cfg.module & module) || (level > wlc_extlog->cfg.level))
		return;

	cur_record = &wlc_extlog->log_table[wlc_extlog->cur_idx];
	cur_record->time = OSL_SYSUPTIME();
	cur_record->module = module;
	cur_record->id = id;
	cur_record->level = level;
	cur_record->sub_unit = sub_unit;
	cur_record->seq_num = wlc_extlog->seq_num++;
	cur_record->arg = arg;
	if (str) {
		memset(cur_record->str, 0, MAX_ARGSTR_LEN);
		strncpy(cur_record->str, str, MAX_ARGSTR_LEN - 1);
	}

	if (wlc_extlog->cfg.flag & LOG_FLAG_EVENT)
		wlc_mac_event(wlc_extlog->wlc, WLC_E_EXTLOG_MSG, NULL, 0, 0, 0,
			cur_record, sizeof(log_record_t));

	wlc_extlog->cur_idx = MODINC(wlc_extlog->cur_idx, MAX_EXTLOG_NUM);

	/* move the last_idx when overflow */
	if (wlc_extlog->cur_idx == wlc_extlog->last_idx)
		wlc_extlog->last_idx = MODINC(wlc_extlog->last_idx, MAX_EXTLOG_NUM);

	return;
}

static int
wlc_extlog_get(wlc_extlog_info_t *extlog, void *inbuf, void *outbuf, int len)
{
	wlc_extlog_req_t *req = (wlc_extlog_req_t *)inbuf;
	wlc_extlog_results_t *results = (wlc_extlog_results_t *)outbuf;
	uint8 last_idx = extlog->last_idx;
	uint8 cur_idx = extlog->cur_idx;
	bool from_last;
	uint32 req_num;
	uint32 allowed_num;
	log_record_t *log_record = &results->logs[0];
	uint8 num = 0;

	from_last = (bool)req->from_last;
	req_num = req->num;

	results->version = extlog->cfg.version;
	results->record_len = sizeof(log_record_t);
	allowed_num = IOBUF_ALLOWED_NUM_OF_LOGREC(log_record_t, len);
	allowed_num = MIN(allowed_num, req_num ? req_num : MAX_EXTLOG_NUM - 1);

	WL_TRACE(("%s(): from_last = %s, req_num=%d, len = %d, allowed_num=%d\n",
		__FUNCTION__, from_last ? "TRUE" : "FALSE", req_num, len, allowed_num));

	if (!extlog->seq_num || (from_last && (last_idx == cur_idx))) {
		results->num = 0;
		return 0;
	}

	if (!from_last) {
		if (extlog->seq_num < MAX_EXTLOG_NUM)
			last_idx = 0;
		else {
			if (allowed_num >= MAX_EXTLOG_NUM)
				allowed_num = MAX_EXTLOG_NUM - 1;
			last_idx = MODSUB(cur_idx, allowed_num, MAX_EXTLOG_NUM);
		}
	}

	while (num < allowed_num) {
		memcpy(&log_record[num], &extlog->log_table[last_idx],
			sizeof(log_record_t));
		last_idx = MODINC(last_idx, MAX_EXTLOG_NUM);
		num++;
		if ((last_idx == cur_idx) && (from_last ||
			(!from_last && (extlog->seq_num < MAX_EXTLOG_NUM))))
			break;
	}

	results->num = num;

	extlog->last_idx = last_idx;

	return 0;
}

static void
wlc_extlog_clear(wlc_extlog_info_t *extlog)
{
	extlog->cur_idx = extlog->last_idx = 0;
	extlog->seq_num = 0;
	/* clear log_table */
	memset(extlog->log_table, 0, MAX_EXTLOG_NUM * sizeof(log_record_t));
}

static int
wlc_extlog_cfg_get(wlc_extlog_info_t *extlog, wlc_extlog_cfg_t *cfg)
{
	memcpy(cfg, &extlog->cfg, sizeof(wlc_extlog_cfg_t));

	return 0;
}

static int
wlc_extlog_cfg_set(wlc_extlog_info_t *extlog, wlc_extlog_cfg_t *cfg)
{
	if ((cfg->module > ((1 << LOG_MODULE_MAX) - 1)) || (cfg->level > WL_LOG_LEVEL_MAX)) {
		return -1;
	}

	extlog->cfg.module = cfg->module;
	extlog->cfg.level = cfg->level;
	extlog->cfg.flag = cfg->flag;
#ifdef WLC_HIGH_ONLY
	wlc_bmac_extlog_cfg_set(extlog->wlc->hw, cfg);
#endif // endif

	return 0;
}

static int
wlc_extlog_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	int err = 0;
	wlc_extlog_info_t *extlog = (wlc_extlog_info_t *)hdl;

	switch (actionid) {
		case IOV_GVAL(IOV_EXTLOG_GET):
			err = wlc_extlog_get(extlog, params, arg, len);
			break;

		case IOV_SVAL(IOV_EXTLOG_CLR):
			wlc_extlog_clear(extlog);
			break;

		case IOV_GVAL(IOV_EXTLOG_CFG):
			wlc_extlog_cfg_get(extlog, (wlc_extlog_cfg_t *)arg);
			break;

		case IOV_SVAL(IOV_EXTLOG_CFG):
			wlc_extlog_cfg_set(extlog, (wlc_extlog_cfg_t *)params);
			break;

		default:
			err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#endif /* WLC_HIGH */
