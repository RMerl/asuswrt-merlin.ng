/*
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
 * $Id: acs.c 788365 2020-06-30 08:18:29Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>

#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlioctl_utils.h>
#include <wlutils.h>
#include <shutils.h>
#include <ethernet.h>

#include "acsd_svr.h"

#define PREFIX_LEN 32

/* some channel bounds */
#define ACS_CS_MIN_2G_CHAN	1	/* min channel # in 2G band */
#define ACS_CS_MIN_5G_CHAN	36	/* min channel # in 5G band */
#define ACS_CS_MAX_5G_CHAN	MAXCHANNEL	/* max channel # in 5G band */

/* possible min channel # in the band */
#define ACS_CS_MIN_CHAN(band)	((band == WLC_BAND_5G) ? ACS_CS_MIN_5G_CHAN : \
			(band == WLC_BAND_2G) ? ACS_CS_MIN_2G_CHAN : 0)
/* possible max channel # in the band */
#define ACS_CS_MAX_CHAN(band)	((band == WLC_BAND_5G) ? ACS_CS_MAX_5G_CHAN : \
			(band == WLC_BAND_2G) ? ACS_CS_MAX_2G_CHAN : 0)

#define ACS_DFLT_FLAGS ACS_FLAGS_LASTUSED_CHK

acs_policy_t predefined_policy[ACS_POLICY_MAX] = {
/* threshld    Channel score weigths values                                      chan */
/* bgn  itf  {  BSS  BUSY  INTF I-ADJ   FCS TXPWR NOISE TOTAL   CNS   ADJ TXOP  DFS}  pick */
/* --- ----   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----  ---  func */
{ -65,  40, {   10,    1,    1,    0,    0,    0,    1,    1,    1,    0,  10,	 0}, NULL}, /* 2G */
{   0, 100, {   10,    0,    1,    1,    0,   10,    1,    1,    1,    1,  10,   5}, NULL}, /* 5G */
{ -55,  45, { -200,    0, -100,  -50,    0,    0,  -50,    0,    1,    0,  -1,   0}, NULL}, /* CUST */
};

acs_info_t *acs_info;

/* get traffic information of the interface */
static int acs_get_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info);

/* get traffic information about TOAd video STAs (if any) */
static int acs_get_video_sta_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info);

/* To check whether bss is enabled for particaular interface or not */
static int
acs_check_bss_is_enabled(char *name, acs_chaninfo_t **c_info_ptr, char *prefix);

#ifdef DEBUG
void
acs_dump_config_extra(acs_chaninfo_t *c_info)
{
	uint8 intf_thld_setting = c_info->intfparams.thld_setting;

	ACSD_INFO("%s: acs_dump_config_extra:\n", c_info->name);
	ACSD_INFO("\t acs_txdelay_period: %d\n", c_info->acs_txdelay_period);
	ACSD_INFO("\t acs_txdelay_cnt: %d\n", c_info->acs_txdelay_cnt);
	ACSD_INFO("\t acs_txdelay_ratio: %d\n", c_info->acs_txdelay_ratio);
	ACSD_INFO("\t acs_dfs: %d\n", c_info->acs_dfs);
	ACSD_INFO("\t acs_far_sta_rssi: %d\n", c_info->acs_far_sta_rssi);
	ACSD_INFO("\t acs_nofcs_least_rssi: %d\n", c_info->acs_nofcs_least_rssi);
	ACSD_INFO("\t acs_chan_dwell_time: %d\n", c_info->acs_chan_dwell_time);
	ACSD_INFO("\t acs_chan_flop_period: %d\n", c_info->acs_chan_flop_period);
	ACSD_INFO("\t acs_tx_idle_cnt: %d\n", c_info->acs_tx_idle_cnt);
	ACSD_INFO("\t acs_cs_scan_timer: %d\n", c_info->acs_cs_scan_timer);
	ACSD_INFO("\t acs_ci_scan_timeout: %d\n", c_info->acs_ci_scan_timeout);
	ACSD_INFO("\t acs_ci_scan_timer: %d\n", c_info->acs_ci_scan_timer);
	ACSD_INFO("\t acs_scan_chanim_stats: %d\n", c_info->acs_scan_chanim_stats);
	ACSD_INFO("\t acs_ci_scan_chanim_stats: %d\n", c_info->acs_ci_scan_chanim_stats);
	ACSD_INFO("\t tcptxfail:%d\n",
		c_info->intfparams.acs_txfail_thresholds[intf_thld_setting].tcptxfail_thresh);
	ACSD_INFO("\t txfail:%d\n",
		c_info->intfparams.acs_txfail_thresholds[intf_thld_setting].txfail_thresh);
	ACSD_INFO("\t txop_weight: %d\n", c_info->txop_weight);

}
#endif /* DEBUG */

#ifdef ACS_DEBUG
static void
acs_dump_map(void)
{
	int i;
	ifname_idx_map_t* cur_map;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		cur_map = &acs_info->acs_ifmap[i];
		if (cur_map->in_use) {
			ACSD_PRINT("i: %d, name: %s, idx: %d, in_use: %d\n",
				i, cur_map->name, cur_map->idx, cur_map->in_use);
		}
	}
}
#endif /* ACS_DEBUG */

static void
acs_add_map(char *name)
{
	int i;
	ifname_idx_map_t* cur_map = acs_info->acs_ifmap;
	size_t length = strlen(name);

	ACSD_DEBUG("add map entry for ifname: %s\n", name);

	if (length >= sizeof(cur_map->name)) {
		ACSD_ERROR("Interface Name Length Exceeded\n");
	} else {
		for (i = 0; i < ACS_MAX_IF_NUM; cur_map++, i++) {
			if (!cur_map->in_use) {
				memcpy(cur_map->name, name, length + 1);
				cur_map->idx = i;
				cur_map->in_use = TRUE;
				break;
			}
		}
	}
#ifdef ACS_DEBUG
	acs_dump_map();
#endif // endif
}

int
acs_idx_from_map(char *name)
{
	int i;
	ifname_idx_map_t *cur_map;

#ifdef ACS_DEBUG
	acs_dump_map();
#endif // endif
	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		cur_map = &acs_info->acs_ifmap[i];
		if (cur_map->in_use && !strcmp(name, cur_map->name)) {
			ACSD_DEBUG("name: %s, cur_map->name: %s idx: %d\n",
				name, cur_map->name, cur_map->idx);
			return cur_map->idx;
		}
	}
	ACSD_WARNING("cannot find the mapped entry for ifname: %s\n", name);
	return -1;
}

/* radio setting information needed from the driver */
static int
acs_get_rs_info(acs_chaninfo_t * c_info, char* prefix)
{
	int ret = 0;
	char tmp[100];
	chanspec_t pref_chspec = 0;
	int band, coex;
	acs_rsi_t *rsi = &c_info->rs_info;
	char *str;
	char data_buf[100];
	acs_param_info_t param;
	/*
	 * Check if the user set the "chanspec" nvram. If not, check if
	 * the "channel" nvram is set for backward compatibility.
	 */
	if ((str = nvram_get(strcat_r(prefix, "chanspec", tmp))) == NULL) {
		str = nvram_get(strcat_r(prefix, "channel", tmp));
	}

	if (str && strcmp(str, "0")) {
		ret = acs_get_chanspec(c_info, &pref_chspec);
		ACS_ERR(ret, "failed to get chanspec");

		rsi->pref_chspec = dtoh32(pref_chspec);
	}

	ret = acs_get_obss_coex_info(c_info, &coex);
	ACS_ERR(ret, "failed to get obss_coex");

	rsi->coex_enb = dtoh32(coex);
	ACSD_INFO("%s: coex_enb: %d\n", c_info->name, rsi->coex_enb);

	ret = wl_ioctl(c_info->name, WLC_GET_BAND, &band, sizeof(band));
	ACS_ERR(ret, "failed to get band info");

	rsi->band_type = dtoh32(band);
	ACSD_INFO("%s: band_type: %d\n", c_info->name, rsi->band_type);

	memset(&param, 0, sizeof(param));
	param.band = band;

	ret = acs_get_bwcap_info(c_info, &param, sizeof(param),	data_buf, sizeof(data_buf));
	ACS_ERR(ret, "failed to get bw_cap");

	rsi->bw_cap = *((uint32 *)data_buf);
	ACSD_INFO("%s: bw_cap: %d\n", c_info->name, rsi->bw_cap);

	return ret;
}

/*
 * acs_pick_chanspec_default2g() - default policy function to pick a chanspec to switch to.
 *
 * c_info:	pointer to the acs_chaninfo_t for this interface.
 * bw:		bandwidth to chose from
 *
 * Returned value:
 *	The returned value is the most preferred valid chanspec from the candidate array.
 *
 * This function picks the most preferred chanspec according to the default policy.
 */
static chanspec_t
acs_pick_chanspec_default2g(acs_chaninfo_t* c_info, int bw)
{
	return acs_pick_chanspec_common(c_info, bw, CH_SCORE_TOTAL);
}

/*
 * acs_pick_chanspec_default5g() - default policy function to pick a chanspec to switch to.
 *
 * c_info:	pointer to the acs_chaninfo_t for this interface.
 * bw:		bandwidth to chose from
 *
 * Returned value:
 *	The returned value is the most preferred valid chanspec from the candidate array.
 *
 * This function picks the most preferred chanspec according to the default policy.
 */
static chanspec_t
acs_pick_chanspec_default5g(acs_chaninfo_t* c_info, int bw)
{
	return acs_pick_chanspec_common(c_info, bw, CH_SCORE_TOTAL);
}

void
acs_default_policy(acs_policy_t *a_pol, acs_policy_index index)
{

	memcpy(a_pol, &predefined_policy[index], sizeof(acs_policy_t));

	if (index == ACS_POLICY_DEFAULT2G) {
		a_pol->chan_selector = acs_pick_chanspec_default2g;
	} else if (index == ACS_POLICY_DEFAULT5G) {
		a_pol->chan_selector = acs_pick_chanspec_default5g;
	} else {
		a_pol->chan_selector = acs_pick_chanspec;
	}
}

#ifdef DEBUG
void
acs_dump_policy(acs_policy_t *a_pol)
{
	printf("ACS Policy:\n");
	printf("Bg Noise threshold: %d\n", a_pol->bgnoise_thres);
	printf("Interference threshold: %d\n", a_pol->intf_threshold);
	printf("Channel Scoring Weights: \n");
	printf("\t BSS: %d\n", a_pol->acs_weight[CH_SCORE_BSS]);
	printf("\t BUSY: %d\n", a_pol->acs_weight[CH_SCORE_BUSY]);
	printf("\t INTF: %d\n", a_pol->acs_weight[CH_SCORE_INTF]);
	printf("\t INTFADJ: %d\n", a_pol->acs_weight[CH_SCORE_INTFADJ]);
	printf("\t FCS: %d\n", a_pol->acs_weight[CH_SCORE_FCS]);
	printf("\t TXPWR: %d\n", a_pol->acs_weight[CH_SCORE_TXPWR]);
	printf("\t BGNOISE: %d\n", a_pol->acs_weight[CH_SCORE_BGNOISE]);
	printf("\t CNS: %d\n", a_pol->acs_weight[CH_SCORE_CNS]);
	printf("\t TXOP: %d\n", a_pol->acs_weight[CH_SCORE_TXOP]);

}
#endif /* DEBUG */

/* look for str in capability (wl cap) and return true if found */
bool
acs_check_cap(acs_chaninfo_t *c_info, char *str)
{
	char data_buf[WLC_IOCTL_MAXLEN];
	uint32 ret, param = 0;

	if (str == NULL || strlen(str) >= WLC_IOCTL_SMLEN) {
		ACSD_ERROR("%s invalid needle to look for in cap\n", c_info->name);
		return FALSE;
	}

	ret = acs_get_cap_info(c_info, &param, sizeof(param), data_buf, sizeof(data_buf));

	if (ret != BCME_OK) {
		ACSD_ERROR("%s Error %d in getting cap\n", c_info->name, ret);
		return FALSE;
	}

	data_buf[WLC_IOCTL_MAXLEN - 1] = '\0';
	if (strstr(data_buf, str) == NULL) {
		ACSD_INFO("%s '%s' not found in cap\n", c_info->name, str);
		return FALSE;
	} else {
		ACSD_INFO("%s '%s' found in cap\n", c_info->name, str);
		return TRUE;
	}
}

static bool init_ch[3] = { FALSE, FALSE, FALSE };

void acs_set_initial_chanspec(acs_chaninfo_t *c_info)
{
	chanspec_t chanspec = WL_CHSPEC_DEF_5G_H;
	char prefix[PREFIX_LEN], tmp[100];
	int band5grp;

	if (c_info->rs_info.pref_chspec == 0) {
		if (c_info->rs_info.band_type == WLC_BAND_5G) {
			int ret = 0;

			if (!init_ch[c_info->unit])
				init_ch[c_info->unit] = TRUE;
			else
				return;

			snprintf(prefix, sizeof(prefix), "wl%d_", c_info->unit);
			band5grp = nvram_get_hex(strcat_r(prefix, "band5grp", tmp));
			if (!(band5grp & WL_5G_BAND_4)) {
				if (band5grp & WL_5G_BAND_1)
					chanspec = WL_CHSPEC_DEF_5G_L;
				else if (band5grp & WL_5G_BAND_2)
					chanspec = wf_chspec_aton("52");
				else if (band5grp & WL_5G_BAND_3)
					chanspec = wf_chspec_aton("100");
			}

			acs_set_chanspec(c_info, chanspec);
			ret = acs_update_driver(c_info);
			if (ret) {
				ACSD_ERROR("%s: update driver failed\n", c_info->name);
			}
		}
	}
}

static int
acs_start(char *name, acs_chaninfo_t *c_info, int skip_init_acs)
{
	int unit;
	char prefix[PREFIX_LEN], tmp[100];
	acs_rsi_t* rsi;
	int ret = 0;

	ACSD_INFO("acs_start for interface %s\n", name);

	ret = wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
	acs_snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	c_info->unit = unit;

	/* check radio */
	if (nvram_match(strcat_r(prefix, "radio", tmp), "0")) {
		ACSD_INFO("ifname %s: radio is off\n", name);
		c_info->mode = ACS_MODE_DISABLE;
		goto acs_start_done;
	}

	acs_retrieve_config(c_info, prefix);

	if ((ret = acs_get_country(c_info)) != BCME_OK)
		ACSD_ERROR("%s: Failed to get country info\n", c_info->name);

	rsi = &c_info->rs_info;
	acs_get_rs_info(c_info, prefix);
	if (!skip_init_acs)
	acs_set_initial_chanspec(c_info);

	if (rsi->pref_chspec == 0) {
		c_info->mode = ACS_MODE_SELECT;
	}
	else if (rsi->coex_enb &&
		nvram_match(strcat_r(prefix, "nmode", tmp), "-1")) {
		c_info->mode = ACS_MODE_COEXCHECK;
	}
	else
		c_info->mode = ACS_MODE_MONITOR; /* default mode */

	if (BAND_5G(rsi->band_type) &&
		(nvram_match(strcat_r(prefix, "reg_mode", tmp), "h") ||
		nvram_match(strcat_r(prefix, "reg_mode", tmp), "strict_h"))) {
		rsi->reg_11h = TRUE;
	}

	ret = acsd_chanim_init(c_info);
	ACS_ERR(ret, "chanim init failed\n");

	c_info->traffic_thresh = acs_check_cap(c_info, ACS_CAP_STRING_TRAFFIC_THRESH);

	if (c_info->acs_ignore_txfail_on_far_sta) {
		if (c_info->traffic_thresh && c_info->traffic_thresh_en) {
			ACSD_DEBUG("%s: setting far_sta rssi %d\n", c_info->name,
					c_info->acs_far_sta_rssi);
			acs_set_far_sta_rssi(c_info, c_info->acs_far_sta_rssi);
		}
	}

	if (AUTOCHANNEL(c_info) && BAND_5G(rsi->band_type)) {
		if (c_info->traffic_thresh && c_info->traffic_thresh_en) {
				acs_intfer_config_traffic_thresh(c_info, prefix);
		} else {
			acs_intfer_config_txfail(c_info);
		}
	}

	/* Do not even allocate a DFS Reentry context on 2.4GHz which does not have DFS channels */
	/* or if 802.11h spectrum management is not enabled. */
	if (BAND_2G(c_info->rs_info.band_type) || (rsi->reg_11h == FALSE)) {
		ACSD_DFSR("%s: DFS Reentry disabled %s\n", c_info->name,
			(BAND_2G(c_info->rs_info.band_type)) ?
			"on 2.4GHz band" : "as 802.11h is not enabled");
		c_info->acs_dfs = ACS_DFS_DISABLED;
	} else {
		ACS_DFSR_CTX(c_info) = acs_dfsr_init(prefix,
			(BAND_5G(rsi->band_type) && (c_info->acs_dfs == ACS_DFS_REENTRY)),
				c_info->acs_bgdfs);
		ret = (ACS_DFSR_CTX(c_info) == NULL) ? -1 : 0;
		ACS_ERR(ret, "Failed to allocate DFS Reentry context\n");
	}

	/* When acsd starts, retrieve current traffic stats since boot */
	acs_get_initial_traffic_stats(c_info);

	if (!AUTOCHANNEL(c_info) && !COEXCHECK(c_info))
		goto acs_start_done;

	c_info->dyn160_cap = acs_check_cap(c_info, ACS_CAP_STRING_DYN160);
	if (c_info->dyn160_cap) {
		acs_update_dyn160_status(c_info);
	}

	if (!(c_info->wet_enabled && acs_check_assoc_scb(c_info))) {
		ret = acs_run_cs_scan(c_info);
		if (ret < 0)
		ACSD_WARNING("%s: cs scan failed\n", c_info->name);
	}

	ACS_FREE(c_info->acs_bss_info_q);

#ifdef ACSD_SEGMENT_CHANIM
	acs_segment_allocate(c_info);
#endif /* ACSD_SEGMENT_CHANIM */

	ret = acs_request_data(c_info);
	ACS_ERR(ret, "request data failed\n");

acs_start_done:
        if (
                !nvram_get_int("x_Setting")
#ifndef RPAX56
                && is_router_mode()
                && !c_info->unit && !init_ch[0]
#endif
        )
        {
		init_ch[0] = TRUE;
		nvram_set_int("obd_allow_scan", 1);
	}

	return ret;
}

static int
acs_check_bss_is_enabled(char *name, acs_chaninfo_t **c_info_ptr, char *prefix)
{
	int index, ret;
	char buf[32] = { 0 }, *bss_check;

	if (strlen(name) >= sizeof((*c_info_ptr)->name)) {
		ACSD_ERROR("Interface Name Length Exceeded\n");
		return BCME_STRLEN;
	}

	if (prefix == NULL || prefix[0] == '\0') {
		strcat_r(name, "_bss_enabled", buf);
	} else {
		strcat_r(prefix, "_bss_enabled", buf);
	}

	bss_check = nvram_safe_get(buf);
	if (atoi(bss_check) != 1) {
		/* this interface is disabled */
		ACSD_INFO("interface is disabled %s\n", name);
		return BCME_DISABLED;
	}

	acs_add_map(name);
	index = acs_idx_from_map(name);

	if (index < 0) {
		ret = ACSD_FAIL;
		ACS_ERR(ret, "Mapped entry not present for interface");
	}

	/* allocate core data structure for this interface */
	*c_info_ptr = acs_info->chan_info[index] =
		(acs_chaninfo_t*)acsd_malloc(sizeof(acs_chaninfo_t));
	strncpy((*c_info_ptr)->name, name, sizeof((*c_info_ptr)->name));
	(*c_info_ptr)->name[sizeof((*c_info_ptr)->name) - 1] = '\0';
	ACSD_INFO("bss enabled for name :%s\n", (*c_info_ptr)->name);
	return BCME_OK;
}

bool
acsd_is_lp_chan(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	UNUSED_PARAMETER(c_info);

	/* Need to check with real txpwr */
	if (wf_chspec_ctlchan(chspec) <= LOW_POWER_CHANNEL_MAX) {
		/* <= 80MHz & primary of 160/80p80 */
		return TRUE;
	} else if (CHSPEC_BW_GT(chspec, WL_CHANSPEC_BW_80)) {
		return (wf_chspec_secondary80_channel(chspec) <= LOW_POWER_CHANNEL_MAX);
	}

	return FALSE;
}

bool acs_check_for_nondfs_chan(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t *candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if (!candi[i].is_dfs) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

bool acs_nondfs_chan_check_for_bgdfs_or_dfsr_trigger(acs_chaninfo_t *c_info, int bw)
{
	bool ret = FALSE;
	if (!c_info->country_is_edcrs_eu) {
		if (bw > ACS_BW_40 && acs_high_power_nondfs_chan_check(c_info, bw) &&
				acs_low_power_nondfs_chan_check(c_info, bw)) {
			ret = TRUE;
		} else if (bw <= ACS_BW_40) {
			ret = TRUE;
		}
	}
	return ret;
}

/* check for availability of high power channel present in the list of
 * valid channels to select
 */
bool acs_high_power_nondfs_chan_check(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t* candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if ((!candi[i].valid) || (candi[i].is_dfs)) {
			continue;
		}

		if (!acsd_is_lp_chan(c_info, candi[i].chspec)) {
			ret = TRUE;
			break;
		}
	}
	return ret;

}

/* check for availability of low power channel present in the list of
 * valid channels to select
 */
bool acs_low_power_nondfs_chan_check(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t* candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if ((!candi[i].valid) || (candi[i].is_dfs)) {
			continue;
		}
		if (acsd_is_lp_chan(c_info, candi[i].chspec)) {
			ret = TRUE;
			break;
		}
	}
	return ret;

}
/* check for overlap between the passed channel arguments */
bool acs_check_for_overlap(chanspec_t chspec1, chanspec_t chspec2)
{
	uint8 channel1, channel2;

	FOREACH_20_SB(chspec1, channel1) {
		FOREACH_20_SB(chspec2, channel2) {
			if (ABS(channel1 - channel2) < CH_20MHZ_APART) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* for 160M/80p80M/80M/40 bw chanspec,select control chan
 * with max AP number for neighbor friendliness
 *
 * For 80p80 - adjust ctrl chan within primary 80Mhz
 */
chanspec_t
acs_adjust_ctrl_chan(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	chanspec_t selected = chspec;
	acs_chan_bssinfo_t* bss_info = c_info->ch_bssinfo;
	uint8 i, j, max_sb, ch, channel;
	uint8 ctrl_sb[8] = {0}, num_sb[8] = {0};
	uint8 selected_sb, last_chan_idx = 0;
	acs_conf_chspec_t *excl_chans = &(c_info->excl_chans);

	if (nvram_match("acs_ctrl_chan_adjust", "0"))
		return selected;

	if (CHSPEC_ISLE20(selected))
		return selected;

	if (CHSPEC_IS160(selected)) {
		max_sb = 8;
	} else if (CHSPEC_IS80(selected) ||
		CHSPEC_IS8080(selected)) {
		max_sb = 4;
	} else {
		max_sb = 2;
	}

	i = 0;
	/* calulate no. APs for all 20M side bands */
	FOREACH_20_SB(selected, channel) {
		ctrl_sb[i] = channel;

		for (j = last_chan_idx; j < c_info->scan_chspec_list.count; j++) {
			ch = (int)bss_info[j].channel;

			if (ch == ctrl_sb[i]) {
				last_chan_idx = j;
				num_sb[i] = bss_info[j].nCtrl;
				ACSD_INFO("%s: sb:%d channel = %d num_sb = %d\n", c_info->name,
					i, channel, num_sb[i]);
				break;
			}
		}
		i++;
	}

	/* when dyn160 is enabled with DFS on FCC, control ch of 50o must be ULL or higher */
	if (max_sb == 8 && c_info->dyn160_enabled && ACS_11H(c_info) &&
		CHSPEC_IS160(selected) &&
		!c_info->country_is_edcrs_eu &&
		CHSPEC_CHANNEL(selected) == ACS_DYN160_CENTER_CH) {
		i = 4;
		selected_sb = WL_CHANSPEC_CTL_SB_ULL >> WL_CHANSPEC_CTL_SB_SHIFT;
	} else {
		i = 0;
		selected_sb = (selected & WL_CHANSPEC_CTL_SB_MASK) >>
			WL_CHANSPEC_CTL_SB_SHIFT;
	}

	/* find which valid sideband has max no. APs */
	for (; i < max_sb; i++) {
		bool excl = FALSE;
		selected &= ~(WL_CHANSPEC_CTL_SB_MASK);
		selected |= (i << WL_CHANSPEC_CTL_SB_SHIFT);

		if (excl_chans && excl_chans->count) {
			for (j = 0; j < excl_chans->count; j++) {
				if (selected == excl_chans->clist[j]) {
					excl = TRUE;
					break;
				}
			}
		}

		if (!excl && num_sb[i] > num_sb[selected_sb]) {
			selected_sb = i;
			ACSD_INFO("%s: selected sb so far = %d n_sbs = %d\n", c_info->name,
					selected_sb, num_sb[selected_sb]);
		}
	}

	ACSD_INFO("%s: selected sb: %d\n", c_info->name, selected_sb);
	selected &=  ~(WL_CHANSPEC_CTL_SB_MASK);
	selected |= (selected_sb << WL_CHANSPEC_CTL_SB_SHIFT);
	ACSD_INFO("%s: Final selected chanspec: 0x%04x\n", c_info->name, selected);
	return selected;
}
/* return TRUE if ACSD is in AP mode else return FALSE
 */
/*bool
acs_is_mode_check(char *osifname)
{
	char tmp[32], prefix[PREFIX_LEN];

	if (strstr(osifname, "eth")) {
		osifname_to_nvifname(osifname, prefix, sizeof(prefix));
		strcat(prefix, "_");
	} else {
		make_wl_prefix(prefix, sizeof(prefix), 1, osifname);
	}

	nvram_safe_get(strcat_r(prefix, "mode", tmp));

	if (nvram_match(tmp, "psr") || nvram_match(tmp, "sta") || nvram_match(tmp, "wet") ||
			nvram_match(tmp, "dwds") || nvram_match(tmp, "psta")) {
		return FALSE;
	}
	return TRUE;
}*/

/*
 * acs_get_txduration - get the overall tx duration
 * c_info - pointer to acs_chaninfo_t for an interface
 * Returns TRUE if tx duration is more than the txblanking threshold
 * Returns FALSE otherwise
 */
bool
acs_get_txduration(acs_chaninfo_t * c_info)
{
	int ret = 0;
	char *data_buf;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param;
	chanim_stats_t * stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	int buflen = ACS_CHANIM_BUF_LEN;
	uint32 count = WL_CHANIM_COUNT_ONE;
	uint8 tx_duration = 0;

	data_buf = acsd_malloc(ACS_CHANIM_BUF_LEN);
	list = (wl_chanim_stats_t *) data_buf;

	param.buflen = htod32(buflen);
	param.count = htod32(count);

	ret = acs_get_chanim_stats(c_info, &param, sizeof(wl_chanim_stats_t), data_buf, buflen);
	if (ret < 0) {
		ACS_FREE(data_buf);
		ACSD_ERROR("%s: failed to get chanim results", c_info->name);
		return FALSE;
	}

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);

	if (list->version == WL_CHANIM_STATS_VERSION) {
		stats = (chanim_stats_t *)list->stats;
		stats->chanspec = dtoh16(stats->chanspec);
		tx_duration = stats->ccastats[CCASTATS_TXDUR];
		ACSD_INFO("%s: chspec 0x%4x tx_duration %d txblank_th %d\n", c_info->name,
			stats->chanspec, tx_duration, c_info->acs_bgdfs->txblank_th);
	} else if (list->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)list->stats;
		statsv2->chanspec = dtoh16(statsv2->chanspec);
		tx_duration = statsv2->ccastats[CCASTATS_TXDUR];
		ACSD_INFO("%s: chspec 0x%4x tx_duration %d txblank_th %d\n", c_info->name,
			statsv2->chanspec, tx_duration, c_info->acs_bgdfs->txblank_th);
	}

	ACS_FREE(data_buf);
	return (tx_duration > c_info->acs_bgdfs->txblank_th) ? TRUE : FALSE;
}

static void
acs_init_info(acs_info_t ** acs_info_p)
{
	acs_info = (acs_info_t*)acsd_malloc(sizeof(acs_info_t));

	*acs_info_p = acs_info;
}
/* This function is used to get enabled virtual ifnames and
 * store it in local structure
 */
static void
acs_store_vifnames(void)
{
	ifname_idx_map_t *cur_map;
	char prefix[8], *ifnames[16] = {0};
	char *token, *delim = ",";
	char *vifname;
	int i, j = 0, ret = BCME_OK;
	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		if (!acs_info->chan_info[i]) {
			continue;
		}
		cur_map = &acs_info->acs_ifmap[i];
		memset(prefix, 0, sizeof(prefix));
		if ((ret = osifname_to_nvifname(cur_map->name, prefix,
				sizeof(prefix))) != BCME_OK) {
			return;
		}
		strcat(prefix, "_vifs");
		vifname = nvram_safe_get(prefix);
		token = strtok(vifname, delim);
		while (token != NULL) {
			ifnames[j] = token;
			acs_info->chan_info[i]->vifnames[j] = ifnames[j];
			ACSD_INFO("ifname %s and corresponding vifnames are %s\n",
				cur_map->name, acs_info->chan_info[i]->vifnames[j]);
			token = strtok(NULL, delim);
			j++;
		}
		j = 0;
	}
	return;
}

/* This function will get available exclude_ifname list by using nvram
 * (acs_exclude_ifnames) and copy to exclude_ifname variable
 */
void acs_get_exclude_interface_list(void)
{
	char *token;
	char *delim = ",";
	int i = 0;
	char *p = NULL, *excl_ifname[ACS_EXCLUDE_IFACE_LIST] = {0};
	p = nvram_safe_get("acs_exclude_ifnames");
	token = strtok(p, delim);
	while (token != NULL) {
		excl_ifname[i] = token;
		acs_info->exclude_ifnames[i] = excl_ifname[i];
		ACSD_INFO("exclude interface list is %s \n", acs_info->exclude_ifnames[i]);
		token = strtok(NULL, delim);
		i++;
	}
}

void
acs_init_run(acs_info_t ** acs_info_p)
{
	char name[16], prefix[PREFIX_LEN] = "", name_enab_if[32] = { 0 };
	char *next, *vifname, *vif_next;
	acs_chaninfo_t * c_info;
	int ret = 0;
#ifdef ZDFS_2G
	int i, tmpint;
	bool acs_zdfs_2g = nvram_match("acs_zdfs_2g", "1");
	bool radar_ignore = nvram_match("acs_zdfs_2g_ignore_radar", "1");
	bool avail_5g = FALSE;
#endif /* ZDSF_2G */
	int skip_init_acs = 0;

	acs_init_info(acs_info_p);

#ifdef ZDFS_2G
	(*acs_info_p)->ci_zdfs_2g  = NULL;
#endif /* ZDSF_2G */

	if (nvram_get_int("acs_skip_init_acs")) {
		skip_init_acs = 1;
		nvram_set_int("acs_skip_init_acs", 0);
	}

	foreach(name, nvram_safe_get("acs_ifnames"), next) {
		c_info = NULL;
		osifname_to_nvifname(name, prefix, sizeof(prefix));
		if (acs_check_bss_is_enabled(name, &c_info, prefix) != BCME_OK) {
			strcat(prefix, "_vifs");
			vifname = nvram_safe_get(prefix);
			foreach(name_enab_if, vifname, vif_next) {
				if (acs_check_bss_is_enabled(name_enab_if,
					&c_info, NULL) == BCME_OK) {
					break;
				}
			}
		}
		memset(name, 0, sizeof(name));
		if (c_info != NULL) {
			memcpy(name, c_info->name, strlen(c_info->name) + 1);
		} else {
			continue;
		}
		ret = acs_start(name, c_info, skip_init_acs);

		if (ret) {
			ACSD_ERROR("acs_start failed for ifname: %s\n", name);
			continue;
		}

#ifdef ZDFS_2G
		if (BAND_5G(c_info->rs_info.band_type)) {
			/* Set if there is atleast one 5G interface */
			avail_5g = TRUE;
		}
#endif /* ZDSF_2G */

		if ((AUTOCHANNEL(c_info) || COEXCHECK(c_info)) &&
			!(c_info->wet_enabled && acs_check_assoc_scb(c_info))) {

			/* First call to pick the chanspec for exit DFS chan */
			c_info->switch_reason = APCS_INIT;

			/* call to pick up init cahnspec */
			acs_select_chspec(c_info);

			if (skip_init_acs) {
				ACSD_INFO("%s: skip init ACS for linkage with AVBLCHAN\n", c_info->name);
				continue;
			}

			/* Other APP can request to change the channel via acsd, in that
			 * case proper reason will be provided by requesting APP, For ACSD
			 * USE_ACSD_DEF_METHOD: ACSD's own default method to set channel
			 */
			acs_set_chspec(c_info, TRUE, ACSD_USE_DEF_METHOD);

			ret = acs_update_driver(c_info);
			if (ret)
				ACSD_ERROR("%s: update driver failed\n", name);

			ACSD_DEBUG("ifname %s - mode: %s\n", name,
			   AUTOCHANNEL(c_info)? "SELECT" :
			   COEXCHECK(c_info)? "COEXCHECK" :
			   ACS11H(c_info)? "11H" : "MONITOR");

			chanim_upd_acs_record(c_info->chanim_info,
				c_info->selected_chspec, APCS_INIT);
		}

		if (c_info->acs_boot_only) {
			c_info->mode = ACS_MODE_DISABLE;
		}
	}
#ifdef ZDFS_2G
	/* Iterate through the 2G interface and enable zdfs_2g
	 * only if atleast one 5G interface is available
	 */
	if (acs_zdfs_2g) {
		int num_5g = 0;
		for (i = 0; i < ACS_MAX_IF_NUM; i++) {
			bool is_5g = FALSE;
			acs_chaninfo_t * tc_info = (*acs_info_p)->chan_info[i];

			if (!tc_info) {
				continue;
			}
			is_5g = BAND_5G(tc_info->rs_info.band_type);
			if (is_5g) {
				if (num_5g < ACSD_MAX_INTERFACES) {
					(*acs_info_p)->ci_5g_all[num_5g++] = tc_info;
				}
				continue;
			}
			if (tc_info->bgdfs160 && avail_5g) {
				acs_set_zdfs_2g(tc_info, TRUE);
			}
			if (ACS_CI_HAS_ZDFS_2G(tc_info, tmpint) &&
					!((*acs_info_p)->ci_zdfs_2g)) {
				(*acs_info_p)->ci_zdfs_2g = tc_info;
				(*acs_info_p)->ci_zdfs_2g->acs_zdfs_2g_ignore_radar = radar_ignore;
			}
		}
	}
#endif /* ZDSF_2G */
	acs_store_vifnames();
	acs_get_exclude_interface_list();
}

int
acs_update_status(acs_chaninfo_t * c_info)
{
	int ret = 0;
	int cur_chspec;
	chanim_info_t * ch_info = c_info->chanim_info;
	uint8 cur_idx = chanim_mark(ch_info).record_idx;
	uint8 start_idx;
	chanim_acs_record_t *start_record;

	ret = wl_iovar_getint(c_info->name, "chanspec", &cur_chspec);
	ACS_ERR(ret, "acs get chanspec failed\n");

	cur_chspec = (chanspec_t)dtoh32(cur_chspec);
	/* return if the channel hasn't changed */
	if (CHSPEC_CHANNEL(cur_chspec) == CHSPEC_CHANNEL(c_info->cur_chspec)) {
		return ret;
	}

	/* To add a acs_record when finding out channel change isn't made by ACSD */
	c_info->cur_chspec = cur_chspec;
	c_info->cur_is_dfs = acs_is_dfs_chanspec(c_info, cur_chspec);
	c_info->cur_is_dfs_weather = acs_is_dfs_weather_chanspec(c_info, cur_chspec);
	c_info->is160_bwcap = WL_BW_CAP_160MHZ((c_info->rs_info).bw_cap);

	if (CHSPEC_CHANNEL(c_info->selected_chspec) != CHSPEC_CHANNEL(c_info->cur_chspec)) {
		ACSD_INFO("%s selected_chspec: 0x%04x != 0x%04x :cur_chspec\n", c_info->name,
				c_info->selected_chspec, c_info->cur_chspec);
		chanim_upd_acs_record(c_info->chanim_info, c_info->cur_chspec, APCS_NONACSD);
		c_info->bw_upgradable_timeout = time(NULL) + ACS_BW_UPGRADABLE_TIMEOUT;
		ACSD_INFO("bw upgradable timeout is: %u\n", c_info->bw_upgradable_timeout);
		acs_update_bw_status(c_info);
		c_info->recent_prev_chspec = c_info->selected_chspec;
		c_info->selected_chspec = c_info->cur_chspec;
		c_info->acs_prev_chan_at = uptime();
	}

	start_idx = MODSUB(cur_idx, 1, ACS_CHANIM_ACS_RECORD);
	start_record = &ch_info->record[start_idx];
	/* Avoiding the duplicate entries only if cur_chspec and start_record->selected_chspec
	 * are same else adding the entry into acs_record.
	 */
	if ((c_info->switch_reason_type == APCS_DFS_REENTRY) && c_info->cur_is_dfs &&
			CHSPEC_CHANNEL(start_record->selected_chspc) !=
			CHSPEC_CHANNEL(c_info->cur_chspec)) {
		chanim_upd_acs_record(c_info->chanim_info, c_info->cur_chspec, APCS_DFS_REENTRY);
		c_info->switch_reason_type = 0;
	}

	if (AUTOCHANNEL(c_info)) {
		acs_dfsr_chanspec_update(ACS_DFSR_CTX(c_info), c_info->cur_chspec,
				__FUNCTION__, c_info->name);
	}

	ACSD_INFO("%s: chanspec: 0x%x is160_bwcap %d is160_upgradable %d, is160_downgradable %d\n",
		c_info->name, c_info->cur_chspec, c_info->is160_bwcap,
		c_info->is160_upgradable, c_info->is160_downgradable);

	return ret;
}

/**
 * check dyn160_enabled through iovar and if enabled, update phy_dyn_switch score
 */
int
acs_update_dyn160_status(acs_chaninfo_t * c_info)
{
	int ret = 0;
	int dyn160_enabled, phy_dyn_switch;

	/* fetch `wl dyn160` */
	ret = acs_get_dyn160_status(c_info->name, &dyn160_enabled);
	ACS_ERR(ret, "acs get dyn160 failed\n");

	c_info->dyn160_enabled = (dyn160_enabled != 0);

	if (!c_info->dyn160_enabled) {
		c_info->phy_dyn_switch = 0;
		return BCME_OK;
	}

	/* if dyn160 is enabled fetch metric `wl phy_dyn_switch` */
	ret = acs_get_phydyn_switch_status(c_info->name, &phy_dyn_switch);
	ACS_ERR(ret, "acs get phy_dyn_switch failed\n");

	c_info->phy_dyn_switch = (uint8) (phy_dyn_switch & 0xFFu);

	c_info->is160_upgradable = c_info->is160_bwcap && !CHSPEC_IS160(c_info->cur_chspec) &&
		!c_info->is_mu_active && c_info->phy_dyn_switch != 1;
	c_info->is160_downgradable = c_info->is160_bwcap && CHSPEC_IS160(c_info->cur_chspec) &&
		c_info->phy_dyn_switch == 1;

	ACSD_INFO("%s phy_dyn_switch: %d is160_upgradable %d is160_downgradable %d \n",
		c_info->name, c_info->phy_dyn_switch, c_info->is160_upgradable,
		c_info->is160_downgradable);

	return BCME_OK;
}

/* Marks c_info->bw_upgradable to TRUE if bw_cap is higher than current channel bandwidth.
 * Avoids when dyn160 or other forms of OMN are in progress.
 * Returns zero on sucess, non-zero (negative) values on error.
 */
int
acs_update_bw_status(acs_chaninfo_t * c_info)
{
	acs_chaninfo_t *zdfs_2g_ci = NULL;
	bool maxed_bwcap = ACS_CHSPEC_MAXED_BWCAP(c_info->cur_chspec, c_info->rs_info.bw_cap);

	c_info->bw_upgradable = FALSE;

	if (!c_info->cur_chspec) {
		return BCME_OK;
	}

	/* Don't allow bw_upgrade when bgdfs is in progress */
	if (ACS_11H_AND_BGDFS(c_info) && c_info->acs_bgdfs->state != BGDFS_STATE_IDLE) {
		return BCME_OK;
	}

	/* Don't allow bw_upgrade when zdfs_2g is in progress */
	if ((zdfs_2g_ci = acs_get_zdfs_2g_ci()) != NULL && zdfs_2g_ci->acs_bgdfs &&
			zdfs_2g_ci->acs_bgdfs->state != BGDFS_STATE_IDLE) {
		return BCME_OK;
	}

	if (maxed_bwcap) {
		ACSD_DEBUG("%s maxed_bwcap:%d, cur_chspec:0x%04x, bw_cap:0x%02x\n", c_info->name,
				maxed_bwcap, c_info->cur_chspec, c_info->rs_info.bw_cap);
		return BCME_OK;
	}

	c_info->bw_upgradable = !ACS_CHSPEC_MAXED_BWCAP(c_info->cur_chspec, c_info->rs_info.bw_cap);

	ACSD_DEBUG("%s bw_upgradable:%d\n", c_info->name, c_info->bw_upgradable);

	return BCME_OK;
}

int acs_tx_idle_check(acs_chaninfo_t *c_info)
{
	uint timer = c_info->acs_cs_scan_timer;
	time_t now = uptime();
	char cntbuf[ACSD_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	int full_scan = 0;
	int ret = 0;
	uint32 acs_txframe;

	/* Check for idle period "acs_cs_scan_timer" */
	if ((now - c_info->timestamp_tx_idle) < timer)
		return full_scan;

	ACSD_INFO("%s: acs_tx_idle: now %u(%u)\n", c_info->name,
		(uint)now, c_info->timestamp_tx_idle);

	/* Check wl transmit activity and trigger full scan if it is idle */
	ret = acs_get_dfsr_counters(c_info->name, cntbuf);
	if (ret < 0) {
		ACSD_ERROR("%s: wl counters failed (%d)\n", c_info->name, ret);
		return full_scan;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	/* As we need only wlc layer ctrs here, no need to input corerev.  */
	ret = wl_cntbuf_to_xtlv_format(NULL, cntbuf, ACSD_WL_CNTBUF_SIZE, 0);
	if (ret < 0) {
		ACSD_ERROR("%s: wl_cntbuf_to_xtlv_format failed (%d)\n", c_info->name, ret);
		return full_scan;
	}

	if (!(wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntbuf))) {
		ACSD_ERROR("%s: wlc_cnt NULL\n", c_info->name);
		return full_scan;
	}

	ACSD_INFO("%s: acs_tx_idle: txframe %d(%d)\n", c_info->name,
			wlc_cnt->txframe, c_info->acs_txframe);

	if (wlc_cnt->txframe > c_info->acs_txframe)
		acs_txframe = wlc_cnt->txframe - c_info->acs_txframe;
	else
		acs_txframe = wlc_cnt->txframe + ((uint32)0xFFFFFFFF - c_info->acs_txframe);

	if (acs_txframe < (c_info->acs_tx_idle_cnt * (now - c_info->timestamp_tx_idle))) {
		ACSD_INFO("%s: acs_tx_idle fullscan: %d\n", c_info->name, c_info->acs_txframe);
		full_scan = 1;
	}

	c_info->acs_txframe = wlc_cnt->txframe;
	c_info->timestamp_tx_idle = now;
	return full_scan;
}

void
acs_cleanup(acs_info_t ** acs_info_p)
{
	int i;

	if (!*acs_info_p)
		return;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		acs_chaninfo_t* c_info = (*acs_info_p)->chan_info[i];

		if (c_info) {
			ACS_FREE(c_info->scan_results);

			if (c_info->acs_escan && c_info->acs_escan->acs_use_escan)
				acs_escan_free(c_info->acs_escan->escan_bss_head);

			acs_cleanup_scan_entry(c_info);
			ACS_FREE(c_info->ch_bssinfo);
			ACS_FREE(c_info->chanim_stats);
			ACS_FREE(c_info->scan_chspec_list.chspec_list);
			ACS_FREE(c_info->candidate[ACS_BW_20]);
			ACS_FREE(c_info->candidate[ACS_BW_40]);
			ACS_FREE(c_info->chanim_info);
			ACS_FREE(c_info->acs_bgdfs);

			acs_dfsr_exit(ACS_DFSR_CTX(c_info));

			ACS_FREE(c_info);
		}
	}
	ACS_FREE(acs_info);
	*acs_info_p = NULL;
}

/* This function is used to check whether ifname is virtual or primary,
 * if ifname is virtual convert it from virtual to corresponding
 * primary interface
 */
void
acs_check_ifname_is_virtual(char **ifname)
{
	ifname_idx_map_t *cur_map;
	int i, j;
	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		cur_map = &acs_info->acs_ifmap[i];
		for (j = 0; j < ACS_MAX_VIFNAMES; j++) {
			if (acs_info->chan_info[i] != NULL &&
				acs_info->chan_info[i]->vifnames[j] != NULL && *ifname != NULL) {
				if (!strncmp(acs_info->chan_info[i]->vifnames[j], *ifname,
					strlen(*ifname))) {
					ACSD_INFO("found vir intf %s converting to corresponding "
					"primary intf %s and iface length is %zu\n",
					acs_info->chan_info[i]->vifnames[j], cur_map->name,
					strlen(*ifname));
					*ifname = cur_map->name;
					break;
				}
			}
		}
	}
}

/* configure intfer params for virtual interfaces,currently
 * copying default params of primary iface to virtual iface
 */
void acs_virtual_intfer_config(acs_chaninfo_t *c_info, void *vir_params)
{
	char prefix[PREFIX_LEN], name_enab_if[32] = { 0 }, *vifname, *vifnext;
	char buf[32] = { 0 }, *bss_check;
	wl_intfer_params_t *params;

	params = (wl_intfer_params_t *)vir_params;
	osifname_to_nvifname(c_info->name, prefix, sizeof(prefix));
	strcat(prefix, "_vifs");
	vifname = nvram_safe_get(prefix);
	foreach(name_enab_if, vifname, vifnext) {
		strcat_r(name_enab_if, "_bss_enabled", buf);
		bss_check = nvram_safe_get(buf);
		if (atoi(bss_check) == 1) {
			wl_iovar_set(name_enab_if, "intfer_params", (void *)params,
					sizeof(wl_intfer_params_t));
			ACSD_5G("ifname: %s cnt:%d period:%d tcptxfail:%d txfail:%d\n",
				name_enab_if, params->period, params->cnt,
				params->tcptxfail_thresh, params->txfail_thresh);
		} else {
			ACSD_INFO("interface is diabled %s\n", name_enab_if);
			continue;
		}

	}
}

/* set intfer trigger params */
int acs_intfer_config_txfail(acs_chaninfo_t *c_info)
{
	wl_intfer_params_t params;
	acs_intfer_params_t *intfer = &(c_info->intfparams);
	int err = 0;
	uint8 thld_setting = ACSD_INTFER_PARAMS_80_THLD;

	ACSD_INFO("%s@%d\n", __FUNCTION__, __LINE__);

	/*
	 * When running 80MBW high chan, and far STA exists
	 * we will use the high threshold for txfail trigger
	 */
	if (!acsd_is_lp_chan(c_info, c_info->cur_chspec) &&
			(c_info->sta_status & ACS_STA_EXIST_FAR)) {
		if (CHSPEC_IS80(c_info->cur_chspec)) {
			thld_setting = ACSD_INTFER_PARAMS_80_THLD_HI;
		} else if (CHSPEC_BW_GE(c_info->cur_chspec,
				WL_CHANSPEC_BW_160)) {
			thld_setting = ACSD_INTFER_PARAMS_160_THLD_HI;
		}
	} else if (CHSPEC_BW_GE(c_info->cur_chspec, WL_CHANSPEC_BW_160)) {
		thld_setting = ACSD_INTFER_PARAMS_160_THLD;
	}

	if (thld_setting == intfer->thld_setting) {
		ACSD_5G("%s: Same Setting intfer[%d].\n", c_info->name, thld_setting);
		return err;
	}
	intfer->thld_setting = thld_setting;

	params.version = INTFER_VERSION;
	params.period = intfer->period;
	params.cnt = intfer->cnt;
	params.txfail_thresh =
		intfer->acs_txfail_thresholds[intfer->thld_setting].txfail_thresh;
	params.tcptxfail_thresh =
		intfer->acs_txfail_thresholds[intfer->thld_setting].tcptxfail_thresh;

	err = acs_set_intfer_params(c_info->name, &params, sizeof(wl_intfer_params_t));

	if (err < 0) {
		ACSD_ERROR("%s: intfer_params error! ret code: %d\n", c_info->name, err);
	}

	ACSD_5G("%s: Setting intfer[%d]: cnt:%d period:%d tcptxfail:%d txfail:%d\n", c_info->name,
			thld_setting, params.period, params.cnt,
			params.tcptxfail_thresh, params.txfail_thresh);

	acs_virtual_intfer_config(c_info, &params);

	return err;
}

/* set interface and bss configuration to get txfail event from driver */
int acs_intfer_config_traffic_thresh(acs_chaninfo_t *c_info, char *prefix)
{
	wl_traffic_thresh_t trfdata;
	int err = 0, i;

	ACSD_INFO("%s: %s@%d\n", c_info->name, __FUNCTION__, __LINE__);
	memset(&trfdata, 0, sizeof(wl_traffic_thresh_t));

	/* enabling interface configuration */
	trfdata.mode = ACS_INTF_CFG;
	trfdata.enable = 1;

	err = acs_set_intfer_traffic_thresh(c_info->name, &trfdata,
			sizeof(wl_traffic_thresh_t));
	ACS_ERR(err, "intferface config error! ret code\n");

	trfdata.mode = ACS_AP_CFG;
	for (i = 0; i < WL_TRF_MAX_QUEUE; i++) {
		trfdata.type = i | ACS_TRF_AE;
		trfdata.enable = 0;
		if (isset((uint8 *)&c_info->acs_ac_flag, i)) {
			trfdata.enable = 1;
		}
		err = acs_set_intfer_traffic_thresh(c_info->name, &trfdata,
				sizeof(wl_traffic_thresh_t));
	}
	return err;
}

int acs_update_rssi(acs_chaninfo_t *c_info, unsigned char *addr)
{
	int ret = 0;
	scb_val_t scb_val;

	/* reset assoc STA staus */
	c_info->sta_status = ACS_STA_NONE;

	memset(&scb_val, 0, sizeof(scb_val));
	memcpy(&scb_val.ea, addr, ETHER_ADDR_LEN);

	ret = wl_ioctl(c_info->name, WLC_GET_RSSI, &scb_val, sizeof(scb_val));

	if (ret < 0) {
		ACSD_ERROR("Err: reading intf:%s STA:"MACF" RSSI %d\n",
			c_info->name, ETHERP_TO_MACF(&scb_val.ea),
			dtoh32(scb_val.val));
		return ret;
	}
	ACSD_5G("txfail sta intf:%s Mac:%02x:%02x:%02x:%02x:%02x:%02x RSSI: %d\n",
		c_info->name, addr[0], addr[1], addr[2], addr[3],
		addr[4], addr[5], dtoh32(scb_val.val));

	if (dtoh32(scb_val.val) < c_info->acs_far_sta_rssi) {
		c_info->sta_status |= ACS_STA_EXIST_FAR;
	} else {
		c_info->sta_status |= ACS_STA_EXIST_CLOSE;
	}

	ACSD_5G("%s: %s@%d sta_status:0x%x\n", c_info->name, __FUNCTION__, __LINE__,
		c_info->sta_status);
	return ret;
}

int acs_update_assoc_info(acs_chaninfo_t *c_info)
{
	struct maclist *list;
	acs_assoclist_t *acs_assoclist;
	int ret = 0, cnt, size;

	/* reset assoc STA staus */
	c_info->sta_status = ACS_STA_NONE;

	ACSD_INFO("%s: %s@%d\n", c_info->name, __FUNCTION__, __LINE__);

	/* read assoclist */
	list = (struct maclist *)acsd_malloc(ACSD_BUFSIZE_4K);
	memset(list, 0, ACSD_BUFSIZE_4K);
	ACSD_INFO("%s: WLC_GET_ASSOCLIST\n", c_info->name);
	list->count = htod32((ACSD_BUFSIZE_4K - sizeof(int)) / ETHER_ADDR_LEN);
	ret = wl_ioctl(c_info->name, WLC_GET_ASSOCLIST, list, ACSD_BUFSIZE_4K);
	if (ret < 0) {
		ACSD_ERROR("%s: WLC_GET_ASSOCLIST failure\n", c_info->name);
		ACS_FREE(list);
		return ret;
	}

	ACS_FREE(c_info->acs_assoclist);
	list->count = dtoh32(list->count);
	if (list->count <= 0) {
		ACS_FREE(list);
		return ret;
	}

	size = sizeof(acs_assoclist_t) + (list->count)* sizeof(acs_sta_info_t);
	acs_assoclist = (acs_assoclist_t *)acsd_malloc(size);

	c_info->acs_assoclist = acs_assoclist;
	acs_assoclist->count = list->count;

	for (cnt = 0; cnt < list->count; cnt++) {
		scb_val_t scb_val;

		memset(&scb_val, 0, sizeof(scb_val));
		memcpy(&scb_val.ea, &list->ea[cnt], ETHER_ADDR_LEN);

		ret = wl_ioctl(c_info->name, WLC_GET_RSSI, &scb_val, sizeof(scb_val));

		if (ret < 0) {
			ACSD_ERROR("Err: reading intf:%s STA:"MACF" RSSI\n",
					c_info->name, ETHER_TO_MACF(list->ea[cnt]));
			ACS_FREE(c_info->acs_assoclist);
			break;
		}

		acs_assoclist->sta_info[cnt].rssi = dtoh32(scb_val.val);
		ether_copy(&(list->ea[cnt]), &(acs_assoclist->sta_info[cnt].ea));
		ACSD_INFO("%s: %s@%d sta_info sta:"MACF" rssi:%d [%d]\n", c_info->name,
				__FUNCTION__, __LINE__,
				ETHER_TO_MACF(list->ea[cnt]), dtoh32(scb_val.val),
				c_info->acs_far_sta_rssi);

		if (acs_assoclist->sta_info[cnt].rssi < c_info->acs_far_sta_rssi)
			c_info->sta_status |= ACS_STA_EXIST_FAR;
		else
			c_info->sta_status |= ACS_STA_EXIST_CLOSE;

		ACSD_INFO("%s: %s@%d sta_status:0x%x\n", c_info->name,
			__FUNCTION__, __LINE__, c_info->sta_status);
	}
	ACS_FREE(list);

	if (!ret) {
		/* check to see if we need to update intfer params */
		if (!c_info->traffic_thresh && (c_info->sta_status & ACS_STA_EXIST_FAR)) {
			acs_intfer_config_txfail(c_info);
		}
	}

	return ret;
}

/*
 *  check if need to switch chan:
 * (1) if run in hi-chan, all STA are far, DFS-reentry is disabled,
 *  chan switch is needed
 */
bool
acsd_need_chan_switch(acs_chaninfo_t *c_info)
{
	acs_assoclist_t *acs_assoclist = c_info->acs_assoclist;
	bool dfsr_disable = (c_info->acs_dfs != ACS_DFS_REENTRY);
	bool is_dfs = c_info->cur_is_dfs;
	int bw = CHSPEC_BW(c_info->cur_chspec);

	ACSD_5G("%s: sta_status:0x%x chanspec:0x%x acs_dfs:%d acs_assoclist:%p is_dfs:%d\n",
			c_info->name,
			c_info->sta_status, c_info->cur_chspec,
			c_info->acs_dfs, c_info->acs_assoclist, is_dfs);

	if ((bw > WL_CHANSPEC_BW_40) &&
			!acsd_is_lp_chan(c_info, c_info->cur_chspec) &&
			acs_assoclist &&
			(c_info->sta_status & ACS_STA_EXIST_FAR) &&
			dfsr_disable &&
			!is_dfs) {
		ACSD_5G("No chan switch is needed.\n");
		return FALSE;
	}
	return TRUE;
}

/* get traffic information of the interface */
static int
acs_get_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info)
{
	char cntbuf[ACSD_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	int ret = BCME_OK;

	if (acs_get_dfsr_counters(c_info->name, cntbuf) < 0) {
		ACSD_DFSR("Failed to fetch interface counters for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	if (wl_cntbuf_to_xtlv_format(NULL, cntbuf, ACSD_WL_CNTBUF_SIZE, 0)
		!= BCME_OK) {
		ACSD_DFSR("wl_cntbuf_to_xtlv_format failed for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	if ((wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntbuf)) == NULL) {
		ACSD_DFSR("GET_WLCCNT_FROM_CNTBUF NULL for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	t_info->timestamp = uptime();
	t_info->txbyte = wlc_cnt->txbyte;
	t_info->rxbyte = wlc_cnt->rxbyte;
	t_info->txframe = wlc_cnt->txframe;
	t_info->rxframe = wlc_cnt->rxframe;
exit:
	return ret;
}

/* get traffic information about TOAd video STAs (if any) */
static int
acs_get_video_sta_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info)
{
	char stabuf[ACS_MAX_STA_INFO_BUF];
	sta_info_v6_t *sta;
	int i, ret = BCME_OK;
	int index = c_info->video_sta_idx;

	struct ether_addr ea;
	acs_traffic_info_t total;

	memset(&total, 0, sizeof(acs_traffic_info_t));
	/* Consolidate the traffic info of all video stas */
	for (i = 0; i < index; i++) {
		memset(stabuf, 0, sizeof(stabuf));
		memcpy(&ea, &c_info->vid_sta[i].ea, sizeof(ea));
		if (acs_get_stainfo(c_info->name, &ea, sizeof(ea), stabuf,
				ACS_MAX_STA_INFO_BUF) < 0) {
			ACSD_ERROR("%s: sta_info for %s failed\n", c_info->name,
				c_info->vid_sta[i].vid_sta_mac);
			return BCME_ERROR;
		}
		sta = (sta_info_v6_t *)stabuf;
		total.txbyte = total.txbyte + dtoh64(sta->tx_tot_bytes);
		total.rxbyte = total.rxbyte + dtoh64(sta->rx_tot_bytes);
		total.txframe = total.txframe + dtoh32(sta->tx_tot_pkts);
		total.rxframe = total.rxframe + dtoh32(sta->rx_tot_pkts);
	}
	t_info->timestamp = uptime();
	t_info->txbyte = total.txbyte;
	t_info->rxbyte = total.rxbyte;
	t_info->txframe = total.txframe;
	t_info->rxframe = total.rxframe;
	return ret;
}

/*
 * acs_get_initial_traffic_stats - retrieve and store traffic activity info when acsd starts
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
acs_get_initial_traffic_stats(acs_chaninfo_t *c_info)
{
	acs_activity_info_t *acs_act = &c_info->acs_activity;
	acs_traffic_info_t *t_prev = &acs_act->prev_bss_traffic;
	acs_traffic_info_t t_curr;
	int ret;

	if (!c_info->acs_toa_enable) {
		if ((ret = acs_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("%s: Failed to get traffic information\n", c_info->name);
			return ret;
		}
	} else {
		if ((ret = acs_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("%s: Failed to get video sta traffic information\n",
				c_info->name);
			return ret;
		}
	}

	t_prev->txframe = t_curr.txframe;
	t_prev->rxframe = t_curr.rxframe;

	return BCME_OK;
}

/*
 * acs_activity_update - updates traffic activity information
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
acs_activity_update(acs_chaninfo_t * c_info)
{
	acs_activity_info_t *acs_act = &c_info->acs_activity;
	time_t now = uptime();
	acs_traffic_info_t t_curr;
	acs_traffic_info_t *t_prev = &acs_act->prev_bss_traffic;
	acs_traffic_info_t *t_accu_diff = &acs_act->accu_diff_bss_traffic;
	acs_traffic_info_t *t_prev_diff = &acs_act->prev_diff_bss_traffic;
	uint32 total_frames; /* total tx and rx frames on link */
	int ret;

	if (!c_info->acs_toa_enable) {
		if ((ret = acs_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("%s: Failed to get traffic information\n", c_info->name);
			return ret;
		}
	} else {
		if ((ret = acs_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("%s: Failed to get video sta traffic information\n",
				c_info->name);
			return ret;
		}
	}

	/* update delta between current and previous fetched */
	t_prev_diff->timestamp = now - t_prev->timestamp;
	t_prev_diff->txbyte = DELTA_FRAMES((t_prev->txbyte), (t_curr.txbyte));
	t_prev_diff->rxbyte = DELTA_FRAMES((t_prev->rxbyte), (t_curr.rxbyte));
	t_prev_diff->txframe = DELTA_FRAMES((t_prev->txframe), (t_curr.txframe));
	t_prev_diff->rxframe = DELTA_FRAMES((t_prev->rxframe), (t_curr.rxframe));

	/* add delta (calculated above) to accumulated deltas */
	t_accu_diff->timestamp += t_prev_diff->timestamp;
	t_accu_diff->txbyte += t_prev_diff->txbyte;
	t_accu_diff->rxbyte += t_prev_diff->rxbyte;
	t_accu_diff->txframe += t_prev_diff->txframe;
	t_accu_diff->rxframe += t_prev_diff->rxframe;

	acs_act->num_accumulated++;

	total_frames =  t_prev_diff->txframe + t_prev_diff->rxframe;

	acs_bgdfs_sw_add(ACS_DFSR_CTX(c_info), now, total_frames);

	/* save current in t_prev (previous) to help with next time delta calculation */
	memcpy(t_prev, &t_curr, sizeof(*t_prev));

	return BCME_OK;
}

/* acs_get_recent_timestamp - gets timestamp of the recent most acs record
 * (or returns zero when record isn't found)
 * c_info - pointer to acs_chaninfo_t for an interface
 * chspec - channel spec to find in acs record
 *
 * Returns timestamp if acs record is found (or zero)
 */
uint64
acs_get_recent_timestamp(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	uint64 timestamp = 0;
	int i;
	chanim_info_t * ch_info = c_info->chanim_info;

	for (i = ACS_CHANIM_ACS_RECORD - 1; i >= 0; i--) {
		if (chspec == ch_info->record[i].selected_chspc) {
			if (ch_info->record[i].timestamp > timestamp) {
				timestamp = (uint64) ch_info->record[i].timestamp;
			}
		}
	}

	return timestamp;
}

void
acs_process_cmd(acs_chaninfo_t * c_info, chanspec_t chspec, int dfs_ap_move)
{
	int ret = 0;
	wl_chan_change_reason_t reason;

	reason = (wl_chan_change_reason_t)dfs_ap_move;

	c_info->selected_chspec = chspec;
	c_info->cur_chspec = chspec;
	acs_set_chspec(c_info, FALSE, dfs_ap_move);

	if (c_info->acs_use_csa) {
		/* wbd commands already processed, return */
		goto end;
	}

	/* No need to update the driver for reason DFS_AP_MOVE stop and
	 * stunt operation as it can stop running dfs cac state machine
	 * for stunt operation.
	 */
	if ((reason != WL_CHAN_REASON_DFS_AP_MOVE_RADAR_FOUND) &&
		(reason != WL_CHAN_REASON_DFS_AP_MOVE_STUNT) &&
		(reason != WL_CHAN_REASON_DFS_AP_MOVE_ABORTED)) {

		ret = acs_update_driver(c_info);

		if (ret)
			ACSD_ERROR("update driver failed\n");
	}

end:
	ACSD_DEBUG("ifname %s - mode: %s\n", c_info->name,
		AUTOCHANNEL(c_info)? "SELECT" :
		COEXCHECK(c_info)? "COEXCHECK" :
		ACS11H(c_info)? "11H" : "MONITOR");

	chanim_upd_acs_record(c_info->chanim_info,
		c_info->selected_chspec, APCS_IOCTL);
}

/*
 * acs_upgrade_to160 - upgrade to 160Mhz BW
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise.
 */
int
acs_upgrade_to160(acs_chaninfo_t * c_info)
{
	int ret;
	if ((ret = acs_bgdfs_choose_channel(c_info, TRUE, TRUE)) != BCME_OK) {
		ACSD_ERROR("%s Picking a 160Mhz channel failed\n", c_info->name);
		return BCME_ERROR;
	}
	return BCME_OK;
}
