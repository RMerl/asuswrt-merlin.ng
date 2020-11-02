/*
 *      acs_bgdfs.c
 *
 *      This module will try to initiate bgdfs scan for the Selected target (DFS) channel.
 *      If bgdfs is succeeds it will move to target channel else it will switch back to
 *      regular chanspec.
 *
 *      Copyright 2020 Broadcom
 *
 *      This program is the proprietary software of Broadcom and/or
 *      its licensors, and may only be used, duplicated, modified or distributed
 *      pursuant to the terms and conditions of a separate, written license
 *      agreement executed between you and Broadcom (an "Authorized License").
 *      Except as set forth in an Authorized License, Broadcom grants no license
 *      (express or implied), right to use, or waiver of any kind with respect to
 *      the Software, and Broadcom expressly reserves all rights in and to the
 *      Software and all intellectual property rights therein.  IF YOU HAVE NO
 *      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *      WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *      THE SOFTWARE.
 *
 *      Except as expressly set forth in the Authorized License,
 *
 *      1. This program, including its structure, sequence and organization,
 *      constitutes the valuable trade secrets of Broadcom, and you shall use
 *      all reasonable efforts to protect the confidentiality thereof, and to
 *      use this information only in connection with your use of Broadcom
 *      integrated circuit products.
 *
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *      "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *      REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *      OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *      DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *      NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *      ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *      CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *      OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *      BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *      SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *      IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *      ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *      OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *      NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: acs_bgdfs.c 786918 2020-05-12 04:39:27Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"

/*
 * acs_bgdfs_idle_check - checks if ahead of time BGDFS scan could be done.
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when ahead of time scan is allowed. BCME_ERROR or BCME_BUSY otherwise.
 */
int
acs_bgdfs_idle_check(acs_chaninfo_t * c_info)
{
	acs_activity_info_t *acs_act = &c_info->acs_activity;
	acs_bgdfs_info_t *acs_bgdfs;
	uint32 th_frames, total_frames;
	int min_accumulate;

	if (!ACS_11H_AND_BGDFS(c_info)) {
		return BCME_ERROR;
	}

	acs_bgdfs = c_info->acs_bgdfs;

	min_accumulate = (acs_bgdfs->idle_interval / (ACS_TRAFFIC_INFO_UPDATE_INTERVAL(acs_bgdfs)));

	if (acs_act->num_accumulated < min_accumulate) {
		return BCME_OK;
	}

	th_frames = acs_bgdfs->idle_frames_thld;

	total_frames = acs_bgdfs_sw_sum(ACS_DFSR_CTX(c_info));

	/* avoid background scan if accumulated traffic exceeds threshold */
	if (total_frames > th_frames) {
		acs_bgdfs->idle = FALSE;
		ACSD_INFO("%s: Link not idle.Accumulated frames tx+rx:%u, th:%u, num_acc:%u\n",
			c_info->name, total_frames, th_frames, acs_act->num_accumulated);
		return BCME_BUSY;
	}

	ACSD_INFO("%s: Link idle. Initiating 3+1 bgdfs. Accumulated frames tx+rx:%u,"
		"th:%u, num_acc:%u\n", c_info->name, total_frames, th_frames,
		acs_act->num_accumulated);

	acs_bgdfs->idle = TRUE;

	return BCME_OK;
}

#ifdef ZDFS_2G
/*
 * try to initiate background DFS scan on the 2g interface; returns BCME_OK if successful
 */
static int
acs_bgdfs_attempt_2g(acs_chaninfo_t * ci_2g, acs_chaninfo_t * ci_5g,
		chanspec_t chspec, bool stunt)
{
	int ret = 0;
	acs_bgdfs_info_t *acs_bgdfs = ci_2g->acs_bgdfs;
	time_t now = uptime();

	if (ci_5g->cac_mode != ACS_CAC_MODE_AUTO &&
			ci_5g->cac_mode != ACS_CAC_MODE_ZDFS_2G_ONLY) {
		ACSD_INFO("%s ZDFS 2G avoided as CAC mode is %d\n",
				ci_5g->name, ci_5g->cac_mode);
		return BCME_NOTENABLED;
	}

	if (acs_bgdfs == NULL) {
		return BCME_ERROR;
	}

	if (acs_bgdfs->state != BGDFS_STATE_IDLE) {
		/* already in progress; just return silently */
		return BCME_OK;
	}

	/* If tx duration (on 5G is) more than tx blanking threshold, avoid BGDFS */
	if (acs_get_txduration(ci_5g)) {
		ACSD_INFO("%s ZDFS 2G avoided as Tx duration exceeding threshold\n", ci_5g->name);
		return BCME_OK;
	}

	if (acs_bgdfs->cap == BGDFS_CAP_UNKNOWN) {
		/* to update capability if this is the first time */
		(void) acs_bgdfs_get(ci_2g);
	}
	if (acs_bgdfs->cap != BGDFS_CAP_TYPE0) {
		/* other types are not supported */
		return BCME_ERROR;
	}

	/* If setting channel, ensure chanspec is neighbor friendly */
	if (((int)chspec) > 0 && !stunt) {
		chspec = acs_adjust_ctrl_chan(ci_5g, chspec);
	}

	if (!ci_5g->bw_upgradable && !stunt && CHSPEC_CHANNEL(chspec) ==
			CHSPEC_CHANNEL(ci_5g->recent_prev_chspec)) {
		if (now - ci_5g->acs_prev_chan_at <
				2 * ci_5g->acs_chan_dwell_time) {
			ACSD_INFO("%s: staying on same channel because of"
					" prev_chspec dwell restrictions\n",
					ci_5g->name);
			return ret;
		}
	}

	ACSD_INFO("%s####Attempting BGDFS 2G on channel 0x%x stunt %d\n", ci_2g->name,
			chspec, stunt);
	if ((ret = acs_bgdfs_set(ci_2g, chspec)) == BCME_OK) {
		time_t now = uptime();
		bool is_dfs_weather = acs_is_dfs_weather_chanspec(ci_5g, chspec);
		acs_bgdfs->state = BGDFS_STATE_MOVE_REQUESTED;
		acs_bgdfs->timeout = now +
			(is_dfs_weather ? BGDFS_CCA_EU_WEATHER : BGDFS_CCA_FCC) +
			BGDFS_POST_CCA_WAIT;
		acs_bgdfs->next_scan_chan = chspec;
		acs_bgdfs->bgdfs_stunted = stunt;
		acs_bgdfs->acs_bgdfs_on_txfail = TRUE;
		if (acs_is_dfs_chanspec(ci_5g, ci_5g->cur_chspec)) {
			/* Fall back is not allowed from DFS to another DFS channel */
			acs_bgdfs->acs_bgdfs_on_txfail = FALSE;
			ci_5g->bw_upgradable_timeout = now + ACS_BW_UPGRADABLE_TIMEOUT;
		}
		acs_update_tx_dur_secs_start();
		/* let the 2g iface remember which 5g iface requested this bgdfs task */
		ci_2g->ci_5g = ci_5g; /* Null this on completion of this ZDFS_2G attempt */
		/* let the 5g iface remember the 2g iface to which the bgdfs task is delegated */
		ci_5g->ci_2g = ci_2g; /* Null this on completion of this ZDFS_2G attempt */
	}

	return ret;
}
#endif /* ZDFS_2G */

/*
 * try to initiate background DFS scan and move; returns BCME_OK if successful
 */
int
acs_bgdfs_attempt(acs_chaninfo_t * c_info, chanspec_t chspec, bool stunt)
{
	int ret = 0;
	time_t now = uptime();
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;
	acs_chaninfo_t *zdfs_2g_ci = NULL;

	if (c_info->cac_mode == ACS_CAC_MODE_FULL_ONLY) {
		ACSD_INFO("%s BGDFS disabled due to CAC mode %d\n",
				c_info->name, c_info->cac_mode);
		return BCME_NOTENABLED;
	}

	if (acs_bgdfs == NULL) {
		return BCME_ERROR;
	}

	/* when mode is ACS_MODE_FIXCHSPEC (eg. used with WBD), BGDFS is allowed
	 *  - only in EU &
	 *  - without move (stunt to preclear only)
	 */
	if (FIXCHSPEC(c_info) || (c_info->wet_enabled && acs_check_assoc_scb(c_info))) {
		if (!c_info->country_is_edcrs_eu) {
			ACSD_INFO("%s BGDFS ch:0x%04x not allowed in ACS_MODE_FIXCHSPEC\n",
				c_info->name, chspec);
			return BCME_USAGE_ERROR;
		}
		if (!stunt) { /* in EU, downgrade to stunt when mode is ACS_MODE_FIXCHSPEC */
			stunt = TRUE;
			ACSD_INFO("%s BGDFS ch:0x%04x downgraded to stunt\n", c_info->name, chspec);
		}
	}

#ifdef ZDFS_2G
	if (c_info->cac_mode == ACS_CAC_MODE_AUTO ||
			c_info->cac_mode == ACS_CAC_MODE_ZDFS_2G_ONLY) {
		if ((zdfs_2g_ci = acs_get_zdfs_2g_ci()) != NULL && zdfs_2g_ci->acs_bgdfs &&
				zdfs_2g_ci->acs_bgdfs->state == BGDFS_STATE_IDLE) {
			return acs_bgdfs_attempt_2g(zdfs_2g_ci, c_info, chspec, stunt);
		}
		/* if ZDFS_2G is in progress on behalf of this 5G interface, return */
		if (zdfs_2g_ci && zdfs_2g_ci->ci_5g == c_info) {
			return BCME_OK;
		}

		/* if fallback to ZDFS_5G is not allowed, return */
		if ((c_info->cac_mode == ACS_CAC_MODE_ZDFS_2G_ONLY) ||
				(zdfs_2g_ci && acs_bgdfs && !acs_bgdfs->zdfs_2g_fallback_5g)) {
			return BCME_OK;
		}
	}
#else
	(void) zdfs_2g_ci;
#endif /* ZDFS_2G */

	if (c_info->cac_mode != ACS_CAC_MODE_AUTO &&
			c_info->cac_mode != ACS_CAC_MODE_ZDFS_5G_ONLY) {
		ACSD_INFO("%s BGDFS disabled due to CAC mode %d\n",
				c_info->name, c_info->cac_mode);
		return BCME_NOTENABLED;
	}

	if (acs_bgdfs->state != BGDFS_STATE_IDLE) {
		/* already in progress; just return silently */
		return BCME_OK;
	}

	/* In case of Far Stas, 3+1 DFS is not allowed */
	if (!acsd_is_lp_chan(c_info, c_info->cur_chspec) &&
			acs_bgdfs->bgdfs_avoid_on_far_sta &&
			(c_info->sta_status & ACS_STA_EXIST_FAR)) {
		ACSD_INFO("%s BGDFS ch:0x%04x rejected - far STA present\n", c_info->name, chspec);
		return BCME_OK;
	}

	/* If tx duration more than tx blanking threshold, avoid BGDFS */
	if (acs_get_txduration(c_info)) {
		ACSD_INFO("%s BGDFS avoided as Tx duration exceeding threshold\n", c_info->name);
		return BCME_OK;
	}

	if (acs_bgdfs->cap == BGDFS_CAP_UNKNOWN) {
		/* to update capability if this is the first time */
		(void) acs_bgdfs_get(c_info);
	}
	if (acs_bgdfs->cap != BGDFS_CAP_TYPE0) {
		/* other types are not supported */
		return BCME_ERROR;
	}

	/* If setting channel, ensure chanspec is neighbor friendly */
	if (((int)chspec) > 0 && !stunt) {
		chspec = acs_adjust_ctrl_chan(c_info, chspec);
		c_info->selected_chspec = chspec;
	}

	/* Overide channel dwell restrictions for acs_req_bw_upgrd */
	if (!c_info->bw_upgradable && !c_info->acs_req_bw_upgrd && !stunt &&
			CHSPEC_CHANNEL(c_info->selected_chspec) ==
			CHSPEC_CHANNEL(c_info->recent_prev_chspec)) {
		if (now - c_info->acs_prev_chan_at <
				2 * c_info->acs_chan_dwell_time) {
			ACSD_INFO("%s: staying on same channel because of"
					" prev_chspec dwell restrictions\n",
					c_info->name);
			return ret;
		}
	}

	ACSD_INFO("%s####Attempting 3+1 on channel 0x%x\n", c_info->name, chspec);
	if ((ret = acs_bgdfs_set(c_info, chspec)) == BCME_OK) {
		time_t now = uptime();
		bool is_dfs_weather = acs_is_dfs_weather_chanspec(c_info, chspec);
		acs_bgdfs->state = BGDFS_STATE_MOVE_REQUESTED;
		acs_bgdfs->timeout = now +
			(is_dfs_weather ? BGDFS_CCA_EU_WEATHER : BGDFS_CCA_FCC) +
			BGDFS_POST_CCA_WAIT;
		acs_bgdfs->acs_bgdfs_on_txfail = TRUE;
		if (acs_is_dfs_chanspec(c_info, c_info->cur_chspec)) {
			/* Fall back is not allowed from DFS to another DFS channel */
			acs_bgdfs->acs_bgdfs_on_txfail = FALSE;
			c_info->bw_upgradable_timeout = now + ACS_BW_UPGRADABLE_TIMEOUT;
		}
		if (stunt && (ret = acs_bgdfs_set(c_info, DFS_AP_MOVE_STUNT)) != BCME_OK) {
			ACSD_ERROR("%s: Failed to stunt dfs_ap_move", c_info->name);
		}
	}
	return ret;
}

static int acs_bgdfs_build_candidates(acs_chaninfo_t *c_info, int bw)
{
	wl_uint32_list_t *list;
	chanspec_t input = 0, c = 0;
	int ret = 0, i;
	int count = 0;
	ch_candidate_t *bgdfs_candi;
	acs_rsi_t *rsi = &c_info->rs_info;

	char *data_buf;
	data_buf = acsd_malloc(ACS_SM_BUF_LEN);

	if (bw == ACS_BW_160) {
		input |= WL_CHANSPEC_BW_160;
	} else if (bw == ACS_BW_8080) {
		input |= WL_CHANSPEC_BW_8080;
	} else if (bw == ACS_BW_80) {
		input |= WL_CHANSPEC_BW_80;
	} else if (bw == ACS_BW_40) {
		input |= WL_CHANSPEC_BW_40;
	} else {
		input |= WL_CHANSPEC_BW_20;
	}

	if (BAND_5G(rsi->band_type)) {
		input |= WL_CHANSPEC_BAND_5G;
	} else {
		input |= WL_CHANSPEC_BAND_2G;
	}

	ret = acs_get_perband_chanspecs(c_info, input, data_buf, ACS_SM_BUF_LEN);

	if (ret < 0) {
		ACSD_ERROR("failed to get valid chanspec lists on %s: iface \n", c_info->name);
		goto cleanup;
	}

	list = (wl_uint32_list_t *)data_buf;
	count = dtoh32(list->count);

	if (!count) {
		ACSD_ERROR("%s: number of valid chanspec is zero\n", c_info->name);
		ret = -1;
		goto cleanup;
	}
	ACS_FREE(c_info->bgdfs_candidate[bw]);
	c_info->bgdfs_candidate[bw] = (ch_candidate_t*)acsd_malloc(count * sizeof(ch_candidate_t));
	bgdfs_candi = c_info->bgdfs_candidate[bw];

	ACSD_DEBUG("%s: address of candi: %p\n", c_info->name, bgdfs_candi);
	for (i = 0; i < count; i++) {
		c = (chanspec_t)dtoh32(list->element[i]);
		bgdfs_candi[i].chspec = c;
		bgdfs_candi[i].valid = TRUE;
		bgdfs_candi[i].is_dfs = acs_is_dfs_chanspec(c_info, bgdfs_candi[i].chspec);
	}
	c_info->c_count[bw] = count;

cleanup:
	free(data_buf);
	return ret;
}

int acs_bgdfs_check_candidates(acs_chaninfo_t *c_info, int bw)
{
	int ret = 0;
	ret = acs_bgdfs_build_candidates(c_info, bw);

	if (ret != BCME_OK) {
		ACSD_ERROR("%s: %s could not get list of candidates\n", c_info->name, __FUNCTION__);
		return BCME_ERROR;
	}

	ret = acs_nondfs_chan_check_for_bgdfs_or_dfsr_trigger(c_info, bw);

	return ret;
}

/*
 * acs_bgdfs_choose_channel - identifies the best channel to
 *   - do BGDFS scan ahead of time if include_unclear is TRUE
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 * include_unclear - DFS channels that can be cleared by BGDFS are considered
 * pick_160        - To pick a 160 Mhz chanspec for upgrading
 *
 * Returns BCME_OK when successful; error status otherwise.
 */
int
acs_bgdfs_choose_channel(acs_chaninfo_t * c_info, bool include_unclear,	bool pick_160)
{
	chanspec_t cand_ch = 0, best_ch = 0;
	int ret, i, count, considered = 0, chbw, bw;
	bool cand_is_weather = FALSE, best_is_weather = FALSE;
	bool cand_attempted = FALSE, best_attempted = FALSE;
	uint64 cand_ts = 0, best_ts = 0; /* recent time stamp in acs record */
	acs_conf_chspec_t *excl_chans;
	uint32 cand_chinfo;
	uint32 requisite = WL_CHAN_VALID_HW | WL_CHAN_VALID_SW | WL_CHAN_BAND_5G | WL_CHAN_RADAR;
	uint64 now = (uint64)(uptime());

	ch_candidate_t *cand_arr;

	if (WL_BW_CAP_160MHZ(c_info->rs_info.bw_cap)) {
		chbw = WL_CHANSPEC_BW_160;
		pick_160 = TRUE;
	} else if (WL_BW_CAP_80MHZ(c_info->rs_info.bw_cap)) {
		chbw = WL_CHANSPEC_BW_80;
	} else if (WL_BW_CAP_40MHZ(c_info->rs_info.bw_cap)) {
		chbw = WL_CHANSPEC_BW_40;
	} else {
		chbw = CHSPEC_BW(c_info->cur_chspec);
	}

	switch (chbw) {
		case WL_CHANSPEC_BW_160:
			bw = ACS_BW_160;
			break;
		case WL_CHANSPEC_BW_8080:
			bw = ACS_BW_8080;
			break;
		case WL_CHANSPEC_BW_80:
			bw = ACS_BW_80;
			break;
		case WL_CHANSPEC_BW_40:
			bw = ACS_BW_40;
			break;
		case WL_CHANSPEC_BW_20:
			bw = ACS_BW_20;
			break;
		default:
			ACSD_ERROR("%s: bandwidth unsupported\n", c_info->name);
			return BCME_UNSUPPORTED;
	}
reduce_bw:
	ACSD_INFO("%s: Selected BW %d; 0-20Mhz, 3-160Mhz bw: %x\n", c_info->name, bw,
			CHSPEC_BW(c_info->cur_chspec));
	ret = acs_bgdfs_build_candidates(c_info, bw); /* build the candidate chanspec list */
	if (ret != BCME_OK) {
		ACSD_ERROR("%s: %s could not get list of candidates\n", c_info->name, __FUNCTION__);
		return BCME_ERROR;
	}
	excl_chans = &(c_info->excl_chans);
	acs_invalidate_exclusion_channels(c_info->bgdfs_candidate[bw],
			c_info->c_count[bw], excl_chans);
	cand_arr = c_info->bgdfs_candidate[bw];

	if (!c_info->rs_info.reg_11h || !c_info->acs_dfs) {
		ACSD_ERROR("%s: %s called when 11h is not enabled or dfs is disabled\n",
				c_info->name, __FUNCTION__);
		return BCME_ERROR;
	}

	chbw = acs_derive_bw_from_given_chspec(c_info);

	count = c_info->c_count[bw];

	for (i = 0; i < count; i++) {
		cand_ch = cand_arr[i].chspec;
		if (!cand_ch || !cand_arr[i].valid) {
			ACSD_DEBUG("%s:invalidating exclude channels 0x%04x\n", c_info->name,
				cand_ch);
			continue;
		}
		if (!cand_arr[i].is_dfs) {
			continue;
		}
		cand_ts = acs_get_recent_timestamp(c_info, cand_ch);

		cand_chinfo = acs_get_chanspec_info(c_info, cand_ch);
		cand_is_weather = ((cand_chinfo & WL_CHAN_WEATHER_RADAR) != 0);
		cand_attempted = (cand_ch == c_info->acs_bgdfs->last_attempted) ||
			(((~WL_CHANSPEC_CTL_SB_MASK) & cand_ch) ==
			((~WL_CHANSPEC_CTL_SB_MASK) & c_info->acs_bgdfs->last_attempted));

		ACSD_INFO("%s: %s Candidate %d: 0x%x, chinfo: 0x%x, weather:%d, attempted:%d\n",
			c_info->name, __FUNCTION__,
			i, cand_ch, cand_chinfo, cand_is_weather, cand_attempted);

		/* reject if already the current channel */
		if (c_info->cur_chspec == cand_ch) {
			continue;
		}

		/* reject overlap with current; just match center channel ignoring control offset */
		if (((~WL_CHANSPEC_CTL_SB_MASK) & c_info->cur_chspec) ==
			((~WL_CHANSPEC_CTL_SB_MASK) & cand_ch)) {
			continue;
		}

		/* reject if all requisites aren't met */
		if ((cand_chinfo & requisite) != requisite) {
			continue;
		}

		/* reject if marked inactive */
		if (ACS_CHINFO_IS_INACTIVE(cand_chinfo)) {
			continue;
		}

		/* reject if already cleared */
		if (include_unclear && ACS_CHINFO_IS_CLEARED(cand_chinfo)) {
			continue;
		}

		/* avoid frequent flip flop; reject recently used ones */
		if ((now - cand_ts) < c_info->acs_chan_flop_period) {
			continue;
		}

		/* avoid recent BGDFS attempted channel */
		if (c_info->country_is_edcrs_eu && cand_attempted &&
				(now - c_info->acs_bgdfs->last_attempted_at) <
				(c_info->acs_bgdfs->idle_interval * 2)) {
			continue;
		}

		ACSD_DEBUG("%s: %s Considered %d: 0x%x\n", c_info->name, __FUNCTION__, i, cand_ch);

		/* passed all checks above; now it may be considered for rating best */
		considered ++;

		if (considered == 1) {
			best_ch = cand_ch;
			best_attempted = cand_attempted;
			best_ts = cand_ts;
			best_is_weather = cand_is_weather;
		}

		/* start comparison after we have more than one per_chan_info */
		if (considered < 2) continue;

		/* reject if candidate is a low power channel and chosen best is high power */
		if (acsd_is_lp_chan(c_info, cand_ch) &&
			!acsd_is_lp_chan(c_info, best_ch)) {
			continue;
		}

		if ((best_ts > cand_ts) || /* prefer least recently used */
				/* prefer a channel different from recently BGDFS attempted one */
				(best_attempted && !cand_attempted) ||
				/* when both are non-weather, prefer higher channel */
				(!best_is_weather && !cand_is_weather &&
				 CHSPEC_CHANNEL(cand_ch) > CHSPEC_CHANNEL(best_ch))) {
			/* update best with candidate since better */
			best_ch = cand_ch;
			best_attempted = cand_attempted;
			best_ts = cand_ts;
			best_is_weather = cand_is_weather;
		}
	}

	if (considered == 0 && c_info->bgdfs160) {
		if (bw > ACS_BW_20 && bw > chbw) {
			ACSD_INFO("reducing bw as there are no valid channels to preclear on "
					"current bw ifname: %s bw:%d\n", c_info->name, bw);
			bw = bw - 1;
			goto reduce_bw;
		}
	}

	if (considered > 0) {
		best_ch = acs_adjust_ctrl_chan(c_info, best_ch);
		ACSD_INFO("%s: %s best_ch: 0x%x\n", c_info->name, __FUNCTION__, best_ch);
		c_info->acs_bgdfs->next_scan_chan = best_ch;
		if (pick_160) {
			c_info->selected_chspec = best_ch;
		}
	}

	return BCME_OK;
}

/*
 * acs_bgdfs_check_status - checks status of previous BGDFS scan
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when previous scan result matches the channel requested by
 * ACSD and indicates it as radar free; other error statuses otherwise.
 */
int
acs_bgdfs_check_status(acs_chaninfo_t * c_info, bool bgdfs_on_txfail)
{
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;
	wl_dfs_ap_move_status_v2_t *status;
	chanspec_t scan_ch;

	if (acs_bgdfs == NULL) {
		return BCME_UNSUPPORTED;
	}

	if (bgdfs_on_txfail) {
		scan_ch = c_info->selected_chspec;
	} else if (acs_bgdfs->next_scan_chan != 0) {
		scan_ch = acs_bgdfs->next_scan_chan;
	} else {
		return BCME_ERROR;
	}

	/* if channel is cleared, don't bother to verify move status for error */
	if (ACS_CHINFO_IS_CLEARED(acs_get_chanspec_info(c_info, scan_ch))) {
		return BCME_OK;
	}

	if (acs_bgdfs_get(c_info) != BGDFS_CAP_TYPE0) {
		return BCME_UNSUPPORTED;
	}

	status = &acs_bgdfs->status;

	/* sanity checks */
	if (status->version != WL_DFS_AP_MOVE_VERSION ||
		status->scan_status.version != WL_DFS_STATUS_ALL_VERSION ||
		status->scan_status.num_sub_status < BGDFS_STATES_MIN_SUB_STATES) {
		return BCME_UNSUPPORTED;
	}
	if (BGDFS_SUB_CHAN(status, BGDFS_SUB_MAIN_CORE) != scan_ch &&
		BGDFS_SUB_LAST(status, BGDFS_SUB_MAIN_CORE) != scan_ch &&
		BGDFS_SUB_CHAN(status, BGDFS_SUB_SCAN_CORE) != scan_ch &&
		BGDFS_SUB_LAST(status, BGDFS_SUB_SCAN_CORE) != scan_ch) {
		ACSD_ERROR("%s: background scan channel 0x%x mismatch [0x%x, 0x%x, 0x%x, 0x%x]\n",
			c_info->name,
			scan_ch,
			BGDFS_SUB_CHAN(status, BGDFS_SUB_MAIN_CORE),
			BGDFS_SUB_LAST(status, BGDFS_SUB_MAIN_CORE),
			BGDFS_SUB_CHAN(status, BGDFS_SUB_MAIN_CORE),
			BGDFS_SUB_LAST(status, BGDFS_SUB_MAIN_CORE));
		return BCME_ERROR;
	}

	if (status->move_status != DFS_SCAN_S_RADAR_FREE) {
		return BCME_ERROR;
	}

	return BCME_OK;
}

/*
 * acs_bgdfs_ahead_trigger_scan - triggers ahead of time BGDFS scan
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise.
 */
int
acs_bgdfs_ahead_trigger_scan(acs_chaninfo_t * c_info)
{
	int ret;
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;
	chanspec_t chosen_chspec = 0;
	bool is_etsi = c_info->country_is_edcrs_eu;
	bool is_dfs = c_info->cur_is_dfs;
	bool chan_least_dwell = FALSE;

	if (c_info->cac_mode == ACS_CAC_MODE_FULL_ONLY) {
		ACSD_INFO("%s BGDFS disabled due to CAC mode %d\n",
				c_info->name, c_info->cac_mode);
		return BCME_NOTENABLED;
	}

	if (acs_bgdfs == NULL) {
		return BCME_UNSUPPORTED;
	}

	/* In FCC, and already on a DFS channel return silently */
	if (!is_etsi && is_dfs && (!c_info->bw_upgradable ||
			c_info->bw_upgradable_timeout > uptime())) {
		return BCME_OK;
	}

	/* Allow for pre-clearance when operating on etsi country codes and dfs channel */
	if ((!c_info->bw_upgradable || c_info->bw_upgradable_timeout > uptime()) &&
			(!is_etsi || !is_dfs)) {
		chan_least_dwell = chanim_record_chan_dwell(c_info,
				c_info->chanim_info);

		/* If last channel change happened within the dwell time period, not allowing
		 * scan or channel change when operating in FCC/non-dfs channel.
		 */
		if (!chan_least_dwell) {
			ACSD_5G("%s: chan_least_dwell is FALSE\n", c_info->name);
			return FALSE;
		}

		/* avoid if txop is good on the current channel */
		if (!acs_check_for_txop_on_curchan(c_info)) {
			return BCME_OK;
		}
	}

	/* attempt to find a channel to do DFS scan on */
	/* find best excluding precleared channels */
	if (acs_bgdfs->next_scan_chan == 0 &&
		(ret = acs_bgdfs_choose_channel(c_info, TRUE, FALSE)) != BCME_OK) {
		ACSD_INFO("%s: acs_bgdfs_choose_channel returned %d\n", c_info->name, ret);
	}

	chosen_chspec = acs_bgdfs->next_scan_chan;
	ACSD_INFO("%s: chosen_chspec = 0x%0x\n", c_info->name, chosen_chspec);

	/* if still chosen_chspec is 0, return */
	if (chosen_chspec == 0) {
		return BCME_OK;
	}

	/* if chosen chosen_chspec is in use; mark and return */
	if (chosen_chspec == c_info->cur_chspec) {
		acs_bgdfs->next_scan_chan = 0;
		return BCME_OK;
	}

	if (((!c_info->bw_upgradable || c_info->bw_upgradable_timeout > uptime()) &&
		(!is_etsi || !is_dfs)) &&
		!acs_channel_compare(c_info, c_info->cur_chspec, chosen_chspec)) {
		acs_bgdfs->next_scan_chan = 0;
		return BCME_OK;
	}

	/* In FCC/ETSI, if on a low power Non-DFS, attempt a DFS 3+1 move */
	if (!(FIXCHSPEC(c_info) || (c_info->wet_enabled && acs_check_assoc_scb(c_info)) || MONITORCHECK(c_info)) &&
			!(acs_is_dfs_chanspec(c_info, c_info->cur_chspec) &&
			(c_info->bw_upgradable_timeout > uptime() ||
			!c_info->bw_upgradable)) &&
			((c_info->acs_enable_dfsr_on_highpwr &&
			(c_info->acs_dfs == ACS_DFS_REENTRY) &&
			!(c_info->sta_status & ACS_STA_EXIST_FAR)) ||
			acsd_is_lp_chan(c_info, c_info->cur_chspec))) {
		ACSD_INFO("%s: moving to DFS channel 0x%0x\n", c_info->name, chosen_chspec);
		if ((ret = acs_bgdfs_attempt(c_info, chosen_chspec, FALSE)) != BCME_OK) {
			ACSD_ERROR("dfs_ap_move Failed\n");
			return ret;
		}
		return BCME_OK;
	}

	if (is_etsi) {
		/* Pre-clearing/stunt the selected channel for future use */
		ACSD_INFO("%s: pre-clearing DFS channel 0x%0x\n", c_info->name, chosen_chspec);
		if ((ret = acs_bgdfs_attempt(c_info, chosen_chspec, TRUE)) != BCME_OK) {
			return ret;
		}
	}
	return BCME_OK;
}

static int
acs_check_for_valid_dfs_channels(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t *candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if (candi[i].is_dfs && acs_dfs_channel_is_usable(c_info, candi[i].chspec)) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

/*
 * acs_bgdfs_attempt_on_txfail - attempts BGDFS scan on txfail
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns TRUE when BGDFS attempt is success in ETSI;
 * FALSE otherwise.
 */
bool
acs_bgdfs_attempt_on_txfail(acs_chaninfo_t * c_info)
{
	int ret = BCME_OK;
	chanspec_t chspec = 0;
	int bw = ACS_BW_20;
	int chbw = CHSPEC_BW(c_info->cur_chspec);
	time_t now = uptime();
	acs_chaninfo_t *zdfs_2g_ci = NULL;

	if (c_info->cac_mode == ACS_CAC_MODE_FULL_ONLY) {
		ACSD_INFO("%s BGDFS disabled due to CAC mode %d\n",
				c_info->name, c_info->cac_mode);
		return BCME_NOTENABLED;
	}

	switch (chbw) {
		case WL_CHANSPEC_BW_160:
			bw = ACS_BW_160;
			break;
		case WL_CHANSPEC_BW_8080:
			bw = ACS_BW_8080;
			break;
		case WL_CHANSPEC_BW_80:
			bw = ACS_BW_80;
			break;
		case WL_CHANSPEC_BW_40:
			bw = ACS_BW_40;
			break;
		case WL_CHANSPEC_BW_20:
			bw = ACS_BW_20;
			break;
		default:
			ACSD_ERROR("%s: bandwidth unsupported\n", c_info->name);
			return FALSE;
	}
	/* Don't attempt dfs-reentry(bgdfs) if there are no valid dfs channels available.
	 */
	if (!acs_check_for_valid_dfs_channels(c_info, bw)) {
		ACSD_INFO("%s: Not attempting dfs-reentry due to no dfs channels\n",
				c_info->name);
		return FALSE;
	}
	if (ACS_11H_AND_BGDFS(c_info) && !c_info->cur_is_dfs &&
		(c_info->country_is_edcrs_eu || !acs_bgdfs_check_candidates(c_info, bw))) {
		acs_dfsr_set_reentry_type(ACS_DFSR_CTX(c_info), DFS_REENTRY_IMMEDIATE);
		c_info->switch_reason = APCS_DFS_REENTRY;
		if (!acs_select_chspec(c_info)) {
			return TRUE;
		}
		chspec = c_info->selected_chspec;

		if (CHSPEC_CHANNEL(c_info->selected_chspec) ==
				CHSPEC_CHANNEL(c_info->recent_prev_chspec)) {
			if (now - c_info->acs_prev_chan_at < 2 * c_info->acs_chan_dwell_time) {
			ACSD_INFO("%s: staying on same channel because of prev_chanspec dwell"
					"restrictions\n", c_info->name);
				return TRUE;
			}
		}

		if (!acs_check_for_txop_on_curchan(c_info)) {
			return TRUE;
		}

		ACSD_INFO("%s Selected chan 0x%x for attempting bgdfs\n", c_info->name, chspec);
		if (chspec) {
			ret = acs_bgdfs_attempt(c_info, chspec, FALSE);
			if (ret != BCME_OK) {
				ACSD_ERROR("%s: Failed bgdfs on %x\n", c_info->name, chspec);
				return FALSE;
			}
			if ((zdfs_2g_ci = acs_get_zdfs_2g_ci()) != NULL && zdfs_2g_ci->acs_bgdfs &&
					zdfs_2g_ci->acs_bgdfs->state != BGDFS_STATE_IDLE) {
				zdfs_2g_ci->acs_bgdfs->acs_bgdfs_on_txfail = TRUE;
			} else {
				c_info->acs_bgdfs->acs_bgdfs_on_txfail = TRUE;
			}
			c_info->switch_reason_type = APCS_DFS_REENTRY;
			c_info->recent_prev_chspec = c_info->cur_chspec;
			c_info->acs_prev_chan_at = uptime();
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * acs_bgdfs_capable - checks if the radio is bgdfs capable
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns TRUE when BGDFS is capable; FALSE otherwise
 */
bool
acs_bgdfs_capable(acs_chaninfo_t * c_info)
{
	bool bgdfs_cap = FALSE;

	bgdfs_cap = acs_check_cap(c_info, ACS_CAP_STRING_BGDFS);
	c_info->bgdfs160 = acs_check_cap(c_info, ACS_CAP_STRING_BGDFS160);
	return bgdfs_cap;
}
