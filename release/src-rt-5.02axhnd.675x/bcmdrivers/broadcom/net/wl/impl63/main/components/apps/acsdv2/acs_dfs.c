/*
 *      acs_dfs.c
 *
 *      This module is mainly gives information about dfs channels list and by
 *      using this can give preference to dfs channels over non dfs channels
 *      during channel selection.
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
 *	$Id: acs_dfs.c 778480 2019-09-03 12:42:35Z $
 */
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"

/*
 * acs_dfs_channel_is_usable() - check whether a specific DFS channel is usable right now.
 *
 * Returns TRUE if so, FALSE if not (ie because the channel is currently out of service).
 *
 */
bool
acs_dfs_channel_is_usable(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	return (acs_get_chanspec_info(c_info, chspec) & WL_CHAN_INACTIVE) ? FALSE : TRUE;
}

/* is chanspec DFS channel */
bool
acs_is_dfs_chanspec(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	return (acs_get_chanspec_info(c_info, chspec) & WL_CHAN_RADAR) ? TRUE : FALSE;
}

/* is chanspec DFS weather channel */
bool
acs_is_dfs_weather_chanspec(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	return (acs_get_chanspec_info(c_info, chspec) & WL_CHAN_RADAR_EU_WEATHER) ? TRUE : FALSE;
}

static int
acs_ret_larger_bw(acs_chaninfo_t *c_info, chanspec_t cur, chanspec_t next)
{
	if (CHSPEC_IS160(cur) && !CHSPEC_IS160(next)) {
		return -1;
	} else if (CHSPEC_IS160(next) && !CHSPEC_IS160(cur)) {
		return 1;
	}

	if (CHSPEC_IS80(cur) && !CHSPEC_IS80(next)) {
		return -1;
	} else if (CHSPEC_IS80(next) && !CHSPEC_IS80(cur)) {
		return 1;
	}

	if (CHSPEC_IS40(cur) && !CHSPEC_IS40(next)) {
		return -1;
	} else if (CHSPEC_IS40(next) && !CHSPEC_IS40(cur)) {
		return 1;
	}

	if (CHSPEC_IS20(cur) && !CHSPEC_IS20(next)) {
		return -1;
	} else if (CHSPEC_IS20(next) && !CHSPEC_IS20(next)) {
		return 1;
	}

	return 0;
}

/* Sort the dfs forced chspec list */
static void
acs_sort_dfs_frcd_list(acs_chaninfo_t *c_info, wl_dfs_forced_t *dfs_frcd)
{
	int i, j;
	chanspec_list_t *chspec_list = &dfs_frcd->chspec_list;

	for (i = 0; i < chspec_list->num - 1; i++) {
		for (j = 0; j < chspec_list->num - i - 1; j++) {
			bool swp = FALSE;
			int ret = (acs_ret_larger_bw(c_info, chspec_list->list[j],
				chspec_list->list[j+1]));

			if (ret > 0) {
				swp = TRUE;
			} else if (ret < 0) {
				swp = FALSE;
			} else if (!acs_is_dfs_chanspec(c_info, chspec_list->list[j]) &&
				acs_is_dfs_chanspec(c_info, chspec_list->list[j+1])) {
				swp = TRUE;
			} else if (!acs_is_dfs_chanspec(c_info, chspec_list->list[j+1]) &&
				acs_is_dfs_chanspec(c_info, chspec_list->list[j])) {
				swp = FALSE;
			} else if (CHSPEC_CHANNEL(chspec_list->list[j]) <
					CHSPEC_CHANNEL(chspec_list->list[j+1])) {
				swp = TRUE;
			} else if (wf_chspec_ctlchan(chspec_list->list[j]) <
					wf_chspec_ctlchan(chspec_list->list[j+1])) {
				swp = TRUE;
			}

			if (swp) {
				chanspec_t chspec = chspec_list->list[j];
				chspec_list->list[j] = chspec_list->list[j+1];
				chspec_list->list[j+1] = chspec;
			}
		}
	}
}

static int
acs_return_curr_bw(acs_rsi_t *rsi)
{
	int bw = WL_CHANSPEC_BW_20;

	if (WL_BW_CAP_160MHZ(rsi->bw_cap)) {
		bw = WL_CHANSPEC_BW_160;
	} else if (WL_BW_CAP_80MHZ(rsi->bw_cap)) {
		bw = WL_CHANSPEC_BW_80;
	} else if (WL_BW_CAP_40MHZ(rsi->bw_cap)) {
		bw = WL_CHANSPEC_BW_40;
	}

	return bw;
}

static bool
acs_prep_dfs_forced_chspec_list(acs_chaninfo_t *c_info, wl_dfs_forced_t **dfs_frcd)
{
	int i, j, ret, count = 0;
	wl_uint32_list_t *list;
	chanspec_t input = 0, c = 0;
	acs_rsi_t *rsi = &c_info->rs_info;
	acs_conf_chspec_t *excl_chans = &(c_info->excl_chans);
	int bw = acs_return_curr_bw(rsi);

	char *data_buf;
	data_buf = acsd_malloc(ACS_SM_BUF_LEN);

	ret = acs_get_perband_chanspecs(c_info, input, data_buf, ACS_SM_BUF_LEN);

	if (ret < 0) {
		ACS_FREE(data_buf);
		ACSD_ERROR("%s: failed to get valid chanspec lists", c_info->name);
		return FALSE;
	}

	list = (wl_uint32_list_t *)data_buf;
	count = dtoh32(list->count);

	if (count == 0) {
		ACSD_INFO("%s: No channels to list \n", c_info->name);
		return FALSE;
	}

	(*dfs_frcd) = (wl_dfs_forced_t *)acsd_realloc((char *)(*dfs_frcd),
		WL_DFS_FORCED_PARAMS_SIZE(count));

	for (i = 0; i < count; i++) {
		bool excl = FALSE;
		c = (chanspec_t)dtoh32(list->element[i]);

		if (BAND_5G(rsi->band_type) && !CHSPEC_IS5G(c)) {
			excl = TRUE;
		}

		if (!excl && excl_chans &&
			excl_chans->count) {
			for (j = 0; j < excl_chans->count; j++) {
				if (c == excl_chans->clist[j]) {
					excl = TRUE;
					break;
				}
			}
		}

		if (bw < CHSPEC_BW(c)) {
			excl = TRUE;
		}

		if (excl) {
			continue;
		}

		(*dfs_frcd)->chspec_list.list[(*dfs_frcd)->chspec_list.num++] = c;
	}

	(*dfs_frcd) = (wl_dfs_forced_t *)acsd_realloc((char *)(*dfs_frcd),
			WL_DFS_FORCED_PARAMS_SIZE((*dfs_frcd)->chspec_list.num));

	/* Arrange this list in order of higher bw channels with dfs channels
	 * prioritzed over non-dfs channels
	 */
	acs_sort_dfs_frcd_list(c_info, *dfs_frcd);
	return TRUE;
}

void
acs_get_best_dfs_forced_chspec(acs_chaninfo_t *c_info)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int ret = 0;
	wl_dfs_forced_t *inp;

	ret = acs_get_dfs_forced_chspec(c_info, smbuf);

	if (ret < 0) {
		ACSD_ERROR("%s: Get dfs chanspec forced fails \n", c_info->name);
		c_info->dfs_forced_chspec = 0;
		return;
	}

	inp = (wl_dfs_forced_t *)smbuf;
	c_info->dfs_forced_chspec = inp->chspec_list.list[0];
}

void
acs_set_dfs_forced_chspec(acs_chaninfo_t * c_info)
{
	int ret = 0;
	chanspec_t chspec = c_info->dfs_forced_chspec;
	acs_rsi_t *rsi = &c_info->rs_info;
	wl_dfs_forced_t *dfs_frcd;
	char smbuf[WLC_IOCTL_SMLEN];

	ACSD_INFO("%s: Setting forced chanspec: 0x%x!\n", c_info->name, chspec);

	if (BAND_2G(rsi->band_type) || !rsi->reg_11h ||
		((BAND_5G(rsi->band_type)) && (c_info->acs_dfs == ACS_DFS_DISABLED))) {
		return;
	}

	ret = acs_get_dfs_forced_chspec(c_info, smbuf);

	if (ret < 0) {
		ACSD_ERROR("%s: get dfs forced chanspec fails!\n", c_info->name);
		return;
	}

	dfs_frcd = (wl_dfs_forced_t *) smbuf;

	if (dfs_frcd->chspec_list.num) {
		ACSD_INFO("%s: User has already issued a dfs_forced_chanspec. Keep that\n",
			c_info->name);
		return;
	}

	dfs_frcd = (wl_dfs_forced_t *)acsd_malloc(WL_DFS_FORCED_PARAMS_SIZE(1));

	/* overwrite with latest forced */
	if (chspec) {
		dfs_frcd->chspec_list.num = 0;
		chspec = htod32(chspec);
		dfs_frcd->chspec_list.list[dfs_frcd->chspec_list.num++] = chspec;
	} else {
		dfs_frcd->version = DFS_PREFCHANLIST_VER;
		if (!acs_prep_dfs_forced_chspec_list(c_info, &dfs_frcd)) {
			ACSD_ERROR("%s: Prep dfs forced list error\n", c_info->name);
			goto exit;
		}
	}
	ret = acs_set_dfs_chan_forced(c_info, dfs_frcd, WL_DFS_FORCED_PARAMS_FIXED_SIZE +
		(dfs_frcd->chspec_list.num * sizeof(chanspec_t)));
	ACSD_INFO("%s: set dfs forced chanspec 0x%x %s!\n",
		c_info->name, chspec, ret? "Fail" : "Succ");

exit:
	ACS_FREE(dfs_frcd);
}

/*
 * Check to see if we need to enable DFS reentry for
 * (1) all the STA are far
 * (2) We running in high power chan
 */
int
acsd_trigger_dfsr_check(acs_chaninfo_t *c_info)
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
			!(c_info->sta_status & ACS_STA_EXIST_FAR) &&
			!dfsr_disable &&
			!is_dfs && c_info->acs_enable_dfsr_on_highpwr) {
		ACSD_5G("%s goto DFSR.\n", c_info->name);
		return TRUE;
	}

	return FALSE;
}
