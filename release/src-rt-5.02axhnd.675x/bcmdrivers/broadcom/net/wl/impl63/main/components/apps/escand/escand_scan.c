/*
 *	escand_scan.c
 *
 *      This module will initiate the escan and try to get the escan results
 *      from driver/firmware for all the channels.
 *
 *	Copyright 2020 Broadcom
 *
 *	This program is the proprietary software of Broadcom and/or
 *	its licensors, and may only be used, duplicated, modified or distributed
 *	pursuant to the terms and conditions of a separate, written license
 *	agreement executed between you and Broadcom (an "Authorized License").
 *	Except as set forth in an Authorized License, Broadcom grants no license
 *	(express or implied), right to use, or waiver of any kind with respect to
 *	the Software, and Broadcom expressly reserves all rights in and to the
 *	Software and all intellectual property rights therein.  IF YOU HAVE NO
 *	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *	WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *	THE SOFTWARE.
 *
 *	Except as expressly set forth in the Authorized License,
 *
 *	1. This program, including its structure, sequence and organization,
 *	constitutes the valuable trade secrets of Broadcom, and you shall use
 *	all reasonable efforts to protect the confidentiality thereof, and to
 *	use this information only in connection with your use of Broadcom
 *	integrated circuit products.
 *
 *	2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *	"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *	REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *	OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *	DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *	NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *	ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *	CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *	OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *	3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *	BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *	SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *	IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *	IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *	ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *	OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *	NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: escand_scan.c 768862 2018-10-30 06:15:40Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "escand_svr.h"

/*
 * This module retrieves the following information from the wl driver
 * 1) scan result (wl_scan_result_t)
 * 2) scan channel spec list
 * 3) channel spec candidate (all valid channel spec for the current band, bw, locale)
 * 4) band type, coex_enable, bw_cap.
 */

void
escand_ci_scan_update_idx(escand_scan_chspec_t *chspec_q, uint8 increment)
{
	uint8 idx = chspec_q->idx + increment;
	uint32 chan_flags;

	if (idx >= chspec_q->count)
		idx = 0;

	do {
		chan_flags = chspec_q->chspec_list[idx].flags;

		/* check if it is preferred channel and pref scanning requested */
		if ((chspec_q->ci_scan_running == ESCAND_CI_SCAN_RUNNING_PREF)) {
			if ((chan_flags & ESCAND_CI_SCAN_CHAN_PREF) && !(chan_flags & ESCAND_CI_SCAN_CHAN_EXCL))
				break;
		} else if (!(chan_flags & ESCAND_CI_SCAN_CHAN_EXCL))
			break;

		/* increment index */
		if (++idx == chspec_q->count)
			idx = 0;

	} while (idx != chspec_q->idx);

	chspec_q->idx = idx;
}

/* maybe we do not care about 11b anymore */
static bool
escand_bss_is_11b(wl_bss_info_t* bi)
{
	uint i;
	bool b = TRUE;

	for (i = 0; i < bi->rateset.count; i++) {
		b = bi->rateset.rates[i] & 0x80;
		if (!b)
			break;
	}
	return b;
}

int
escand_build_scanlist(escand_chaninfo_t *c_info)
{
	wl_uint32_list_t *list;
	chanspec_t input = 0, c = 0;
	int ret = 0, i, j;
	int count = 0;
	scan_chspec_elemt_t *ch_list;
	escand_rsi_t *rsi = &c_info->rs_info;
	escand_conf_chspec_t *pref_chans = &(c_info->pref_chans);
	escand_conf_chspec_t *excl_chans = &(c_info->excl_chans);

	char *data_buf, *data_buf1 = NULL;
	data_buf = escand_malloc(ESCAND_SM_BUF_LEN);

	input |= WL_CHANSPEC_BW_20;

	if (BAND_5G(rsi->band_type)) {
		input |= WL_CHANSPEC_BAND_5G;
		ESCAND_INFO("input added 5G");
	} else {
		input |= WL_CHANSPEC_BAND_2G;
		ESCAND_INFO("input added 2G");
	}

	ret = escand_get_perband_chanspecs(c_info, input, data_buf, ESCAND_SM_BUF_LEN);

	if (ret < 0)
		ESCAND_FREE(data_buf);
	ESCAND_ERR(ret, "failed to get valid chanspec lists");

	list = (wl_uint32_list_t *)data_buf;
	count = dtoh32(list->count);

	c_info->scan_chspec_list.count = count;
	c_info->scan_chspec_list.idx = 0;
	c_info->scan_chspec_list.pref_count = 0;
	c_info->scan_chspec_list.excl_count = 0;

	if (!count) {
		ESCAND_ERROR("number of valid chanspecs is 0\n");
		ret = -1;
		goto cleanup_sl;
	}

	ESCAND_FREE(c_info->scan_chspec_list.chspec_list);

	ch_list = c_info->scan_chspec_list.chspec_list =
		(scan_chspec_elemt_t *)escand_malloc(count * sizeof(scan_chspec_elemt_t));

	data_buf1 = escand_malloc(ESCAND_SM_BUF_LEN);

	for (i = 0; i < count; i++) {
		c = (chanspec_t)dtoh32(list->element[i]);

		ch_list[i].chspec = c;

		if (BAND_5G(rsi->band_type)) {
			input = c;
			ret = escand_get_per_chan_info(c_info, input, data_buf1, ESCAND_SM_BUF_LEN);
			if (ret < 0) {
				ESCAND_FREE(data_buf);
				ESCAND_FREE(data_buf1);
			}
			ESCAND_ERR(ret, "failed to get per_chan_info");

			ch_list[i].chspec_info = dtoh32(*(uint32 *)data_buf1);

			/* Exclude DFS channels if 802.11h spectrum management is off */
			/*
			if (!rsi->reg_11h && (ch_list[i].chspec_info & WL_CHAN_RADAR)) {
				ch_list[i].flags |= ESCAND_CI_SCAN_CHAN_EXCL;
				c_info->scan_chspec_list.excl_count++;
			}
			*/
		}

		/* Update preferred channel attribute */
		if (pref_chans && pref_chans->count) {
			for (j = 0; j < pref_chans->count; j++) {
				if (c == pref_chans->clist[j]) {
					ch_list[i].flags |= ESCAND_CI_SCAN_CHAN_PREF;
					c_info->scan_chspec_list.pref_count++;
					break;
				}
			}
		}

		/* Update excluded channel attribute */
		if (excl_chans && excl_chans->count) {
			for (j = 0; j < excl_chans->count; j++) {
				if (c == excl_chans->clist[j]) {
					ch_list[i].flags |= ESCAND_CI_SCAN_CHAN_EXCL;
					c_info->scan_chspec_list.excl_count++;
					break;
				}
			}
		}

		ESCAND_PRINT("chanspec: (0x%04x), chspec_info: 0x%08x  chan flags: 0x%08x\n", c,
			ch_list[i].chspec_info, ch_list[i].flags);
	}
	escand_ci_scan_update_idx(&c_info->scan_chspec_list, 0);

cleanup_sl:
	ESCAND_FREE(data_buf);
	ESCAND_FREE(data_buf1);

	return ret;
}

static int
escand_scan_prep(escand_chaninfo_t *c_info, wl_scan_params_t *params, int *params_size)
{
	int ret = 0;
	int i, scount = 0;
	escand_scan_chspec_t* scan_chspec_p = &c_info->scan_chspec_list;

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = WL_SCANFLAGS_PASSIVE;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = ESCAND_CS_SCAN_DWELL;
	params->home_time = ESCAND_CS_HOME_DWELL;
	params->channel_num = 0;

	ret = escand_build_scanlist(c_info);
	ESCAND_ERR(ret, "failed to build scan chanspec list");

	for (i = 0; i < scan_chspec_p->count; i++) {
		if (scan_chspec_p->chspec_list[i].flags & ESCAND_CI_SCAN_CHAN_EXCL)
			continue;
		params->channel_list[scount++] = htodchanspec(scan_chspec_p->chspec_list[i].chspec);
	}
	params->channel_num = htod32(scount & WL_SCAN_PARAMS_COUNT_MASK);
	ESCAND_INFO("%s: number of channels to scan: %d\n", c_info->name, params->channel_num);

	*params_size = WL_SCAN_PARAMS_FIXED_SIZE + scount * sizeof(uint16);
	ESCAND_INFO("%s: params size: %d\n", c_info->name, *params_size);

	return ret;
}

/* channel information (quick) scan at run time */
int
escand_run_ci_scan(escand_chaninfo_t *c_info)
{
	int ret = 0;
	if (c_info->escand_escan->escand_use_escan) {
		ret = escand_run_escan(c_info, ESCAND_SCAN_TYPE_CI);

		if (ret == BCME_UNSUPPORTED) {
			/* Escan unsupported. Use normal scan */
			c_info->escand_escan->escand_use_escan = 0;
		}
	}

	if (!c_info->escand_escan->escand_use_escan) {
		ret = escand_run_normal_ci_scan(c_info);
	}

	return ret;
}

void
escand_expire_scan_entry(escand_chaninfo_t *c_info, time_t limit)
{
	time_t now;
	escand_bss_info_entry_t *curptr, *previous = NULL, *past;
	escand_bss_info_entry_t **rootp = &c_info->escand_bss_info_q;

	curptr = *rootp;
	now = time(NULL);

	while (curptr) {
		time_t diff = now - curptr->timestamp;
		if (diff > limit) {
			ESCAND_5G("Scan expire: %s diff %d sec chanspec 0x%x, SSID %s\n",
				c_info->name, (int)diff, curptr->binfo_local.chanspec,
				curptr->binfo_local.SSID);
			if (previous == NULL)
				*rootp = curptr->next;
			else
				previous->next = curptr->next;

			past = curptr;
			curptr = curptr->next;
			ESCAND_FREE(past);
			continue;
		}
		previous = curptr;
		curptr = curptr->next;
	}
}

int escand_ci_scan_check(escand_chaninfo_t *c_info)
{
	escand_scan_chspec_t* chspec_q = &c_info->scan_chspec_list;
	time_t now = time(NULL);
	static int started[10] = {0,0,0,0,0,0,0,0,0,0};
	int index;

	index = escand_idx_from_map(c_info->name);

	/* no chan to scan */
	if (chspec_q->count <= chspec_q->excl_count) {
		ESCAND_INFO("%s: no channels to scan", c_info->name);
		started[index] = 0;
		return 0;
	}

	/* start ci scan if no scan for timeout period
	*/

	/* check scan timeout, and trigger CI scan if timeout happened */
	if ((now - c_info->timestamp_escand_scan) >= c_info->escand_ci_scan_timeout) {
		if (started[index] == 1) {
			return 1;
		}
		c_info->escand_ci_scan_count = chspec_q->count - chspec_q->excl_count;
		chspec_q->ci_scan_running = (chspec_q->pref_count ? ESCAND_CI_SCAN_RUNNING_PREF :
			ESCAND_CI_SCAN_RUNNING_NORM);
		escand_ci_scan_update_idx(&c_info->scan_chspec_list, 0);
		ESCAND_INFO("escand_ci_scan_timeout start CI scan: %s, now %u, timestamp %u, scan_count %d, scan_running %d\n",
			c_info->name, (uint)now, c_info->timestamp_escand_scan,
			chspec_q->count - chspec_q->excl_count, chspec_q->ci_scan_running);
		started[index] = 1;
		return 1;
	}

	started[index] = 0;
	return 0;
}

int escand_ci_scan_finish_check(escand_chaninfo_t * c_info)
{
	escand_scan_chspec_t* chspec_q = &c_info->scan_chspec_list;

	/* do nothing for fcs mode or scanning not active  */
	if (!chspec_q->ci_scan_running)
		return 0;

	/* Check for end of scan: scanned all channels once */
	if ((c_info->escand_ci_scan_count) && (!(--c_info->escand_ci_scan_count))) {
		ESCAND_INFO("escand_ci_scan_timeout stop CI scan: %s, now %u \n",
			c_info->name, (uint)time(NULL));
		chspec_q->ci_scan_running = 0;
	}

	return 0;
}

int
escand_do_ci_update(uint ticks, escand_chaninfo_t * c_info)
{
	int ret = 0, isup = 0, skip = 0;

	if (ticks % c_info->escand_ci_scan_timer)
		return ret;

	escand_expire_scan_entry(c_info, (time_t)c_info->escand_scan_entry_expire);

	if (!(c_info->scan_chspec_list.ci_scan_running))
		return ret;

	ret = escand_get_isup(c_info, &isup);
	if (ret < 0) {
		ESCAND_INFO("Couldn't get up status on %s, skip scan and data request\n", c_info->name);
		skip = 1;
	}
	if (isup == 0 && skip == 0) {
		ESCAND_INFO("Interface %s not up, skip scan and data request\n", c_info->name);
		skip = 1;
	}

	if (!skip) {
		ret = escand_run_ci_scan(c_info);
		ESCAND_ERR(ret, "ci scan failed\n");

		ret = escand_request_data(c_info);
		ESCAND_ERR(ret, "request data failed\n");
	}

	escand_ci_scan_finish_check(c_info);

	if (!(c_info->scan_chspec_list.ci_scan_running)) {
		ESCAND_INFO("scan_stopped: ifname = %s\n", c_info->name);
	}

	return ret;
}

/* Default CI scan - Non EScan
 */
int escand_run_normal_ci_scan(escand_chaninfo_t *c_info)
{
	int ret = 0;
	int i;
	wl_scan_params_t *params = NULL;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + sizeof(uint16);
	escand_scan_chspec_t* scan_chspec_q = &c_info->scan_chspec_list;
	scan_chspec_elemt_t *scan_elemt = NULL;
	bool is_dfs = FALSE;
	channel_info_t ci;

	if ((scan_chspec_q->count - scan_chspec_q->excl_count) == 0) {
		ESCAND_INFO("%s: scan chanspec queue is empty, count %u, excl_count %u.\n",
			c_info->name, scan_chspec_q->count, scan_chspec_q->excl_count);
		return ret;
	}
	ESCAND_INFO("%s: scan chanspec queue count %u.\n",
		c_info->name, scan_chspec_q->count);

	scan_elemt = &scan_chspec_q->chspec_list[scan_chspec_q->idx];
	if (scan_elemt->chspec_info & WL_CHAN_RADAR)
		is_dfs = TRUE;
	BCM_REFERENCE(is_dfs);

	params = (wl_scan_params_t*)escand_malloc(params_size);

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = WL_SCANFLAGS_PASSIVE;
	params->nprobes = -1;
	params->active_time = ESCAND_CI_SCAN_DWELL;
	params->passive_time = ESCAND_CI_SCAN_DWELL;
	params->home_time = -1;
	params->channel_num = 1; /* 1 channel for each ci scan */

	params->channel_list[0] = htodchanspec(scan_elemt->chspec);

	ret = wl_ioctl(c_info->name, WLC_SCAN, params, params_size);
	ESCAND_INFO("%s: scan started on chspec: 0x%x\n", c_info->name, scan_elemt->chspec);
	if (ret < 0)
		ESCAND_FREE(params);
	ESCAND_ERR(ret, "WLC_SCAN failed");

	if (!ret) {
		escand_ci_scan_update_idx(scan_chspec_q, 1);
		c_info->timestamp_escand_scan = time(NULL);
		sleep_ms(ESCAND_CI_SCAN_DWELL * 5);
		for (i = 0; i < 10; i++) {
			ret = wl_ioctl(c_info->name, WLC_GET_CHANNEL, &ci, sizeof(channel_info_t));

			if (ret < 0)
				ESCAND_FREE(params);
			ESCAND_ERR(ret, "WLC_GET_CHANNEL failed");

			ci.scan_channel = dtoh32(ci.scan_channel);
			if (!ci.scan_channel)
				break;

			ESCAND_INFO("%s: ci scan in progress, ch %d ...\n", c_info->name, ci.scan_channel);
			sleep_ms(2);
		}
	}
	ESCAND_INFO("%s: ci scan on chspec: 0x%x\n", c_info->name, scan_elemt->chspec);
	ESCAND_FREE(params);
	return ret;
}

/* channel selection (full) scan at init/reset time */
int
escand_run_cs_scan(escand_chaninfo_t *c_info)
{
	int ret;
	if (c_info->escand_escan->escand_use_escan) {
		ret = escand_run_escan(c_info, ESCAND_SCAN_TYPE_CS);

		if (ret == BCME_UNSUPPORTED) {
			/* Escan unsupported. Use normal scan */
			c_info->escand_escan->escand_use_escan = 0;
		}
	}

	if (!c_info->escand_escan->escand_use_escan) {
		ret = escand_run_normal_cs_scan(c_info);
	}

	return ret;
}

/* Run full scan without using ESCAN
 */
int
escand_run_normal_cs_scan(escand_chaninfo_t *c_info)
{
	int ret = 0;
	int i;
	wl_scan_params_t *params = NULL;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + ESCAND_NUMCHANNELS * sizeof(uint16);
	channel_info_t ci;

	params = (wl_scan_params_t*)escand_malloc(params_size);
	ret = escand_scan_prep(c_info, params, &params_size);
	if (ret < 0) {
		ESCAND_FREE(params);
		ESCAND_ERR(ret, "failed to do scan prep");
	}

	ret = wl_ioctl(c_info->name, WLC_SCAN, params, params_size);
	if (ret < 0) {
		ESCAND_FREE(params);
		ESCAND_ERR(ret, "WLC_SCAN failed");
	}

	memset(&ci, 0, sizeof(channel_info_t));
	/* loop to check if cs scan is done, check for scan in progress */
	if (!ret) {
		c_info->timestamp_escand_scan = time(NULL);
		c_info->timestamp_tx_idle = c_info->timestamp_escand_scan;
		/* this time needs to be < 1000 to prevent mpc kicking in for 2nd radio */
		sleep_ms(ESCAND_CS_SCAN_DWELL);
		for (i = 0; i < 100; i++) {
			ret = wl_ioctl(c_info->name, WLC_GET_CHANNEL, &ci, sizeof(channel_info_t));
			if (ret < 0) {
				ESCAND_FREE(params);
				ESCAND_ERR(ret, "WLC_GET_CHANNEL failed");
			}

			ci.scan_channel = dtoh32(ci.scan_channel);
			if (!ci.scan_channel)
				break;

			ESCAND_INFO("cs scan in progress, ch %d ...\n", ci.scan_channel);
			sleep_ms(ESCAND_CS_SCAN_DWELL);
		}
	}
	ESCAND_FREE(params);
	return ret;
}

static int
escand_get_scan(char* name, char *scan_buf, uint buf_len)
{
	wl_scan_results_t *list = (wl_scan_results_t*)scan_buf;
	int ret = 0;

	list->buflen = htod32(buf_len);
	ret = wl_ioctl(name, WLC_SCAN_RESULTS, scan_buf, buf_len);
	if (ret)
		ESCAND_ERROR("err from WLC_SCAN_RESULTS: %d\n", ret);

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);
	if (list->buflen == 0) {
		list->version = 0;
		list->count = 0;
	} else if (list->version != WL_BSS_INFO_VERSION &&
	           list->version != LEGACY2_WL_BSS_INFO_VERSION &&
	           list->version != LEGACY_WL_BSS_INFO_VERSION) {
		fprintf(stderr, "Sorry, your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			list->version, WL_BSS_INFO_VERSION);
		list->buflen = 0;
		list->count = 0;
	}
	ESCAND_INFO("list->count: %d, list->buflen: %d\n", list->count, list->buflen);

	return ret;
}

/* channel selection (full) scan at init/reset time */
int
escand_run_escan(escand_chaninfo_t *c_info, uint8 scan_type)
{
	int params_size = (WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params)) +
		(WL_NUMCHANNELS * sizeof(uint16));
	wl_escan_params_t *params;
	int err;
	struct timeval tv, tv_tmp;
	time_t escan_timeout;

	params = (wl_escan_params_t*)escand_malloc(params_size);
	if (params == NULL) {
		ESCAND_ERROR("Error allocating %d bytes for scan params\n", params_size);
		return BCME_NOMEM;
	}
	memset(params, 0, params_size);

	if (scan_type == ESCAND_SCAN_TYPE_CS) {
		tv.tv_usec = 0;
		tv.tv_sec = 1;
		err = escand_escan_prep_cs(c_info, &params->params, &params_size);
		escan_timeout = time(NULL) + WL_CS_SCAN_TIMEOUT;
	} else if (scan_type == ESCAND_SCAN_TYPE_CI) {
		tv.tv_sec = 0;
		tv.tv_usec = WL_CI_SCAN_TIMEOUT;
		err = escand_escan_prep_ci(c_info, &params->params, &params_size);
		escan_timeout = time(NULL) + 1;
	} else {
		ESCAND_ERROR("%s Unknown scan type %d\n", c_info->name, scan_type);
		return BCME_ERROR;
	}

	params->version = htod32(ESCAN_REQ_VERSION);
	params->action = htod16(WL_SCAN_ACTION_START);

	srand((unsigned)time(NULL));
	params->sync_id = htod16(random() & 0xffff);

	params_size += OFFSETOF(wl_escan_params_t, params);
	err = escand_set_escan_params(c_info, params, params_size);
	if (err != 0)
		goto exit2;

	c_info->escand_escan->scan_type = scan_type;
	c_info->escand_escan->escand_escan_inprogress = TRUE;

	escand_escan_free(c_info->escand_escan->escan_bss_head);
	c_info->escand_escan->escan_bss_head = NULL;
	c_info->escand_escan->escan_bss_tail = NULL;

	ESCAND_INFO("Escan start \n");
	while (time(NULL) < escan_timeout && c_info->escand_escan->escand_escan_inprogress) {
		memcpy(&tv_tmp, &tv, sizeof(tv));
		escand_main_loop(&tv_tmp);
	}
	c_info->escand_escan->escand_escan_inprogress = FALSE;
exit2:
	free(params);

	return err;
}

/* channel information (quick) scan at run time */
int
escand_escan_prep_ci(escand_chaninfo_t *c_info, wl_scan_params_t *params, int *params_size)
{
	int ret = 0;
	escand_scan_chspec_t* scan_chspec_q = &c_info->scan_chspec_list;
	scan_chspec_elemt_t *scan_elemt = NULL;
	bool is_dfs = FALSE;

	if ((scan_chspec_q->count - scan_chspec_q->excl_count) == 0) {
		ESCAND_INFO("scan chanspec queue is empty.\n");
		return ret;
	}

	scan_elemt = &scan_chspec_q->chspec_list[scan_chspec_q->idx];
	if (scan_elemt->chspec_info & WL_CHAN_RADAR)
		is_dfs = TRUE;

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = is_dfs ? WL_SCANFLAGS_PASSIVE : 0;
	params->nprobes = -1;
	params->active_time = ESCAND_CI_SCAN_DWELL;
	params->passive_time = ESCAND_CI_SCAN_DWELL;
	params->home_time = -1;
	params->channel_num = 1; /* 1 channel for each ci scan */

	params->channel_list[0] = htodchanspec(scan_elemt->chspec);

	escand_ci_scan_update_idx(scan_chspec_q, 1);
	c_info->timestamp_escand_scan = time(NULL);

	*params_size = WL_SCAN_PARAMS_FIXED_SIZE + params->channel_num * sizeof(uint16);
	ESCAND_INFO("ci scan on chspec: 0x%x\n", scan_elemt->chspec);

	return ret;
}

int
escand_escan_prep_cs(escand_chaninfo_t *c_info, wl_scan_params_t *params, int *params_size)
{
	int ret = 0;
	int i, scount = 0;
	escand_scan_chspec_t* scan_chspec_p = &c_info->scan_chspec_list;

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = 0; /* ACTIVE SCAN; */
	params->nprobes = -1;
	params->active_time = ESCAND_CS_SCAN_DWELL_ACTIVE;
	params->passive_time = ESCAND_CS_SCAN_DWELL;

	params->home_time = -1;
	params->channel_num = 0;

	ret = escand_build_scanlist(c_info);
	ESCAND_ERR(ret, "failed to build scan chanspec list");

	for (i = 0; i < scan_chspec_p->count; i++) {
		if (scan_chspec_p->chspec_list[i].flags & ESCAND_CI_SCAN_CHAN_EXCL)
			continue;
		params->channel_list[scount++] = htodchanspec(scan_chspec_p->chspec_list[i].chspec);
	}
	params->channel_num = htod32(scount & WL_SCAN_PARAMS_COUNT_MASK);
	ESCAND_INFO("number of channels to scan: %d\n", params->channel_num);

	*params_size = WL_SCAN_PARAMS_FIXED_SIZE + scount * sizeof(uint16);
	ESCAND_INFO("params size: %d\n", *params_size);

	return ret;
}

void
escand_escan_free(struct escan_bss *node)
{
	struct escan_bss *tmp;

	while (node) {
		tmp = node->next;
		free(node);
		node = tmp;
	}
}

void
escand_cleanup_scan_entry(escand_chaninfo_t *c_info)
{
	escand_bss_info_entry_t *headptr = c_info->escand_bss_info_q;
	escand_bss_info_entry_t *curptr;

	while (headptr) {
		curptr = headptr;
		headptr = headptr->next;
		ESCAND_FREE(curptr);
	}
	c_info->escand_bss_info_q = NULL;
}

static void
display_scan_entry_local(escand_bss_info_sm_t * bsm)
{
	char ssidbuf[SSID_FMT_BUF_LEN];
	wl_format_ssid(ssidbuf, bsm->SSID, bsm->SSID_len);

	printf("SSID: \"%s\"\n", ssidbuf);
	printf("BSSID: %s\t", wl_ether_etoa(&bsm->BSSID));
	printf("chanspec: 0x%x\n", bsm->chanspec);
	printf("RSSI: %d dBm\t", (int16)bsm->RSSI);
	printf("Type: %s", ((bsm->type == ESCAND_BSS_TYPE_11A) ? "802.11A" :
		((bsm->type == ESCAND_BSS_TYPE_11G) ? "802.11G" : "802.11B")));
	printf("\n");
}

void
escand_dump_scan_entry(escand_chaninfo_t *c_info)
{
	escand_bss_info_entry_t *curptr = c_info->escand_bss_info_q;

	while (curptr) {
		display_scan_entry_local(&curptr->binfo_local);
		printf("timestamp: %u\n", (uint32)curptr->timestamp);
		curptr = curptr->next;
	}
}

static int
escand_insert_scan_entry(escand_chaninfo_t *c_info, escand_bss_info_entry_t * new)
{
	escand_bss_info_entry_t *curptr, *previous = NULL;
	escand_bss_info_entry_t **rootp = &c_info->escand_bss_info_q;

	curptr = *rootp;
	previous = curptr;

	while (curptr &&
	   memcmp(&curptr->binfo_local.BSSID, &new->binfo_local.BSSID, sizeof(struct ether_addr))) {
		previous = curptr;
		curptr = curptr->next;
	}
	new->next = curptr;
	if (previous == NULL)
		*rootp = new;
	else {
		if (curptr == NULL)
			previous->next = new;
		else /* find an existing entry */ {
			curptr->timestamp = new->timestamp;
			memcpy(&curptr->binfo_local, &new->binfo_local, sizeof(escand_bss_info_sm_t));
			ESCAND_FREE(new);
		}
	}
	return 0;
}

static int
escand_update_escanresult_queue(escand_chaninfo_t *c_info)
{
	struct escan_bss *escan_bss_head;
	wl_bss_info_t *bi;
	escand_bss_info_entry_t * new_entry = NULL;
	escand_channel_t chan;
	chanspec_t cur_chspec;

	for (escan_bss_head = c_info->escand_escan->escan_bss_head;
		escan_bss_head != NULL;
		escan_bss_head = escan_bss_head->next) {

		new_entry = (escand_bss_info_entry_t*)escand_malloc(sizeof(escand_bss_info_entry_t));
		bi = escan_bss_head->bss;
		new_entry->binfo_local.chanspec = cur_chspec = dtoh16(bi->chanspec);
		new_entry->binfo_local.RSSI = dtoh16(bi->RSSI);
		new_entry->binfo_local.SSID_len = bi->SSID_len;
		memcpy(new_entry->binfo_local.SSID, bi->SSID, bi->SSID_len);
		memcpy(&new_entry->binfo_local.BSSID, &bi->BSSID, sizeof(struct ether_addr));
		new_entry->timestamp = time(NULL);
		escand_parse_chanspec(cur_chspec, &chan);
		ESCAND_INFO("%s: Scan: chanspec 0x%x, control %x SSID %s\n", c_info->name,
			cur_chspec, chan.control, new_entry->binfo_local.SSID);
		/* BSS type in 2.4G band */
		if (chan.control <= ESCAND_CS_MAX_2G_CHAN) {
			if (escand_bss_is_11b(bi))
				new_entry->binfo_local.type = ESCAND_BSS_TYPE_11B;
			else
				new_entry->binfo_local.type = ESCAND_BSS_TYPE_11G;
		}
		else
			new_entry->binfo_local.type = ESCAND_BSS_TYPE_11A;
		escand_insert_scan_entry(c_info, new_entry);
	}
	return 0;
}

static int
escand_update_scanresult_queue(escand_chaninfo_t *c_info)
{
	wl_scan_results_t* s_result = c_info->scan_results;
	wl_bss_info_t *bi = s_result->bss_info;
	int b, len = 0;
	escand_bss_info_entry_t * new_entry = NULL;
	escand_channel_t chan;
	chanspec_t cur_chspec;

	for (b = 0; b < s_result->count; b ++, bi = (wl_bss_info_t*)((int8*)bi + len)) {

		len = dtoh32(bi->length);
		new_entry = (escand_bss_info_entry_t*)escand_malloc(sizeof(escand_bss_info_entry_t));

		new_entry->binfo_local.chanspec = cur_chspec = dtoh16(bi->chanspec);
		new_entry->binfo_local.RSSI = dtoh16(bi->RSSI);
		new_entry->binfo_local.SSID_len = bi->SSID_len;
		memcpy(new_entry->binfo_local.SSID, bi->SSID, bi->SSID_len);
		memcpy(&new_entry->binfo_local.BSSID, &bi->BSSID, sizeof(struct ether_addr));
		new_entry->timestamp = time(NULL);
		escand_parse_chanspec(cur_chspec, &chan);
		ESCAND_INFO("%s: Scan: chanspec 0x%x, control %x SSID %s\n", c_info->name,
			cur_chspec, chan.control, new_entry->binfo_local.SSID);
		/* BSS type in 2.4G band */
		if (chan.control <= ESCAND_CS_MAX_2G_CHAN) {
			if (escand_bss_is_11b(bi))
				new_entry->binfo_local.type = ESCAND_BSS_TYPE_11B;
			else
				new_entry->binfo_local.type = ESCAND_BSS_TYPE_11G;
		}
		else
			new_entry->binfo_local.type = ESCAND_BSS_TYPE_11A;
		escand_insert_scan_entry(c_info, new_entry);
	}
	return 0;
}

#ifdef ESCAN_DEBUG
static void
escand_dump_chan_bss(escand_chan_bssinfo_t* bssinfo, int ncis)
{
	int c;
	escand_chan_bssinfo_t *cur;

	printf("channel nCtrl nExt20 nExt40 nExt80\n");
	for (c = 0; c < ncis; c++) {
		cur = &bssinfo[c];
		printf("%3d  %5d%6d%7d%7d\n", cur->channel, cur->nCtrl,
			cur->nExt20, cur->nExt40, cur->nExt80);
	}
}
#endif /* ESCAN_DEBUG */

static void
escand_incr_bss_count(escand_chan_bssinfo_t *bss_info,
	escand_channel_t *chan_p, uint8 channel)
{
	int i = 0;
	uint8 *chan_parse = (uint8 *) chan_p;
	uint8 *bss_info_chan = (uint8 *) bss_info;
	uint8 channel_info_length = sizeof(escand_channel_t)/sizeof(uint8);

	for (i = 0; i < channel_info_length; i++) {
		if (*(chan_parse + i * sizeof(*chan_parse)) == channel) {
			int j = 1;
			int k = i;
			/* Find the bss count index to increment
			 * index - 0		- increase nCtrl
			 * index - 1		- increase nExt20
			 * index - 2,3		- increase nExt40
			 * index - 4,5,6,7	- increase nExt80
			 */
			while (k) {
				k >>= 1;
				j++;
			}
			*(bss_info_chan + j * sizeof(*chan_parse)) += 1;
		}
	}
}

static int
escand_update_chan_bssinfo(escand_chaninfo_t *c_info)
{
	escand_bss_info_entry_t *biq;
	scan_chspec_elemt_t* chspec_list;
	char * new_buf = NULL;
	escand_channel_t chan;
	escand_channel_t *chan_p = &chan;
	chanspec_t cur_chspec;
	int count = 0, buf_size, c;
	escand_chan_bssinfo_t *bss_info;

	count = c_info->scan_chspec_list.count;
	chspec_list = c_info->scan_chspec_list.chspec_list;

	if (count == 0)
		return 0;

	buf_size = sizeof(escand_chan_bssinfo_t) * count;
	new_buf = escand_malloc(buf_size);

	bss_info = (escand_chan_bssinfo_t *) new_buf;

	for (c = 0; c < count; c ++) {
		bzero(&bss_info[c], sizeof(escand_chan_bssinfo_t));

		biq = c_info->escand_bss_info_q;
		/* set channel range centered by the scan channel */
		bss_info[c].channel = CHSPEC_CHANNEL(chspec_list[c].chspec);
		ESCAND_DEBUG("count: %d, channel: %d\n", c, bss_info[c].channel);

		while (biq) {
			assert(biq);
			cur_chspec = biq->binfo_local.chanspec;
			escand_parse_chanspec(cur_chspec, chan_p);

			/* Find and increase bss counts per channel */
			escand_incr_bss_count(&bss_info[c], chan_p, bss_info[c].channel);
			biq = biq->next;
		}
		ESCAND_DEBUG(" channel %u: %u nCtrl %u nExt20 %u nExt40 %u nExt80\n",
			bss_info[c].channel, bss_info[c].nCtrl, bss_info[c].nExt20,
			bss_info[c].nExt40, bss_info[c].nExt80);
	}

	ESCAND_FREE(c_info->ch_bssinfo);
	c_info->ch_bssinfo = (escand_chan_bssinfo_t *) new_buf;

#ifdef ESCAN_DEBUG
	escand_dump_chan_bss(c_info->ch_bssinfo, c_info->scan_chspec_list.count);
	escand_dump_scan_entry(c_info);
#endif /* ESCAN_DEBUG */

	return 0;
}

int
escand_request_data(escand_chaninfo_t *c_info)
{
	if (c_info->escand_escan->escand_use_escan)
		return escand_request_escan_data(c_info);
	else
		return escand_request_normal_scan_data(c_info);
}

int
escand_request_escan_data(escand_chaninfo_t *c_info)
{
	int ret;

	ret = escand_update_escanresult_queue(c_info);
	escand_update_chan_bssinfo(c_info);

	return ret;
}

int
escand_request_normal_scan_data(escand_chaninfo_t *c_info)
{
	int ret = 0;

	char *dump_buf = escand_malloc(ESCAND_SRSLT_BUF_LEN);

	ret = escand_get_scan(c_info->name, dump_buf, ESCAND_SRSLT_BUF_LEN);

	ESCAND_FREE(c_info->scan_results);
	c_info->scan_results = (wl_scan_results_t *)dump_buf;

	escand_update_scanresult_queue(c_info);
	escand_update_chan_bssinfo(c_info);

	return ret;
}

/* Derive bandwidth from a given chanspec(i.e cur_chspec) */
int
escand_derive_bw_from_given_chspec(escand_chaninfo_t * c_info)
{
	int bw, chbw;

	chbw = CHSPEC_BW(c_info->cur_chspec);
	switch (chbw) {
		case WL_CHANSPEC_BW_160:
			bw = ESCAND_BW_160;
			break;
		case WL_CHANSPEC_BW_8080:
			bw = ESCAND_BW_8080;
			break;
		case WL_CHANSPEC_BW_80:
			bw = ESCAND_BW_80;
			break;
		case WL_CHANSPEC_BW_40:
			bw = ESCAND_BW_40;
			break;
		case WL_CHANSPEC_BW_20:
			bw = ESCAND_BW_20;
			break;
		default:
			ESCAND_ERROR("bandwidth unsupported ");
			return BCME_UNSUPPORTED;
	}
	return bw;
}

/*
 * escand_scan_timer_check() - check for scan timer
 *
 * This function checks whether we need to scan because of CS scan timer expiration
 * and does the scan if so.
 */

int
escand_scan_timer_check(escand_chaninfo_t * c_info)
{
	uint cs_scan_timer;
	int ret = 0;

	/* Check whether we should scan now because of the CS scan timer */
	cs_scan_timer = c_info->escand_cs_scan_timer;

	if (SCAN_TIMER_ON(c_info) && cs_scan_timer) {

		ESCAND_DEBUG(" timer: %d\n", cs_scan_timer);

		if (escand_idle_check(c_info) ||
			(!escand_check_assoc_scb(c_info))) {

			/* start scan */
			ret = escand_run_cs_scan(c_info);
			ESCAND_ERR(ret, "cs scan failed\n");
			escand_cleanup_scan_entry(c_info);

			ret = escand_request_data(c_info);
			ESCAND_ERR(ret, "request data failed\n");
		}
	}

	return ret;
}
