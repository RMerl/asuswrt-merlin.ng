/*
 *      acs_iov.c
 *
 *      This module will try to set/get the data from/to driver using IOVAR.
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
 *	$Id: acs_iov.c 788365 2020-06-30 08:18:29Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"

int acs_get_perband_chanspecs(acs_chaninfo_t *c_info, chanspec_t input, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "chanspecs", &input, sizeof(chanspec_t), buf, length);
}

/*
 * Returns the channel info of the chspec passed (by combining per_chan_info of each 20MHz subband)
 */
uint32
acs_get_chanspec_info(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	char resbuf[ACS_PER_CHAN_INFO_BUF_LEN];
	int ret;
	uint8 sub_channel;
	chanspec_t sub_chspec;
	uint32 chinfo = 0, max_inactive = 0, sub_chinfo;

	FOREACH_20_SB(chspec, sub_channel) {
		sub_chspec = CH20MHZ_CHSPEC(sub_channel);
		ret = acs_get_per_chan_info(c_info, sub_chspec, resbuf, ACS_PER_CHAN_INFO_BUF_LEN);
		if (ret != BCME_OK) {
			ACSD_ERROR("%s Failed to get channel (0x%02x) info: %d\n",
				c_info->name, sub_chspec, ret);
			return 0;
		}

		sub_chinfo = dtoh32(*(uint32 *)resbuf);
		ACSD_DFSR("%s: sub_chspec 0x%04x info %08x (%s, %d minutes)\n",
			c_info->name, sub_chspec, sub_chinfo,
			(sub_chinfo & WL_CHAN_INACTIVE) ? "inactive" :
			((sub_chinfo & WL_CHAN_PASSIVE) ? "passive" : "active"),
			GET_INACT_TIME(sub_chinfo));

		/* combine subband chinfo (except inactive time) using bitwise OR */
		chinfo |= ((~INACT_TIME_MASK) & sub_chinfo);
		/* compute maximum inactive time amongst each subband */
		if (max_inactive < GET_INACT_TIME(sub_chinfo)) {
			max_inactive = GET_INACT_TIME(sub_chinfo);
		}
	}
	/* merge maximum inactive time computed into the combined chinfo */
	chinfo |= max_inactive << INACT_TIME_OFFSET;

	ACSD_DFSR("%s: chanspec 0x%04x info %08x (%s, %d minutes)\n",
		c_info->name, chspec, chinfo,
		(chinfo & WL_CHAN_INACTIVE) ? "inactive" :
		((chinfo & WL_CHAN_PASSIVE) ? "passive" : "active"),
		GET_INACT_TIME(chinfo));

	return chinfo;
}

int acs_get_per_chan_info(acs_chaninfo_t *c_info, chanspec_t sub_chspec, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "per_chan_info", &sub_chspec, sizeof(chanspec_t), buf,
		length);
}

/* change per_chan_info of 20MHz channels corresponding to a chanspec (chspec).
 * bitmap (bmp) provides per 20MHz channel information of type bmp_type
 */
int acs_set_chanspec_info(acs_chaninfo_t *c_info, chanspec_t chspec,
		uint8 bmp, acs_chan_info_bmp_t bmp_type)
{
	int ret = BCME_ERROR, i = 0;
	uint8 sub_channel;
	chanspec_t sub_chspec;
	uint32 chinfo = 0;

	if (bmp_type == ACS_CHAN_INFO_ACTIVE) {
		chinfo = 0; /* unset passive */
	} else if (bmp_type == ACS_CHAN_INFO_INACTIVE) {
		chinfo = WL_CHAN_INACTIVE;
	} else {
		ACSD_ERROR("%s %s unknown bmp_type 0x%02x\n",
			c_info->name, __FUNCTION__, bmp_type);
		return ret;
	}
	FOREACH_20_SB(chspec, sub_channel) {
		if (chinfo == WL_CHAN_INACTIVE && !(bmp & (1 << i++))) {
			continue;
		}
		sub_chspec = CH20MHZ_CHSPEC(sub_channel);
		if ((ret = acs_set_per_chan_info(c_info, sub_chspec, chinfo)) != BCME_OK) {
			ACSD_ERROR("%s Failed to set channel (0x%x) info: %d\n",
				c_info->name, sub_chspec, ret);
		}
	}
	return ret;
}

int acs_set_per_chan_info(acs_chaninfo_t *c_info, chanspec_t sub_chspec, uint32 chinfo)
{
	wl_set_chan_info_t chan_info;
	chan_info.version = htod16(WL_SET_CHAN_INFO_VER);
	chan_info.length = htod16(sizeof(wl_set_chan_info_t));
	chan_info.type = htod16(WL_SET_CHAN_INFO_TYPE);
	chan_info.chanspec = htod16(sub_chspec);
	chan_info.per_chan_info = htod32(chinfo);
	return wl_iovar_set(c_info->name, "per_chan_info", &chan_info, sizeof(wl_set_chan_info_t));
}

int acs_set_scanresults_minrssi(acs_chaninfo_t *c_info, int minrssi)
{
	return wl_iovar_setint(c_info->name, "scanresults_minrssi", minrssi);
}

int acs_set_escan_params(acs_chaninfo_t *c_info, wl_escan_params_t *params, int params_size)
{
	return wl_iovar_set(c_info->name, "escan", params, params_size);
}

int acs_get_chanspec(acs_chaninfo_t *c_info, chanspec_t *chanspec)
{
	return wl_iovar_get(c_info->name, "chanspec", chanspec, sizeof(chanspec_t));
}

int acs_set_chanspec(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	return wl_iovar_setint(c_info->name, "chanspec", htod32(chspec));
}

int acs_set_far_sta_rssi(acs_chaninfo_t *c_info, int rssi)
{
	return wl_iovar_setint(c_info->name, "far_sta_rssi", rssi);
}

int acs_get_obss_coex_info(acs_chaninfo_t *c_info, int *coex)
{
	return wl_iovar_getint(c_info->name, "obss_coex", coex);
}

int acs_get_bwcap_info(acs_chaninfo_t *c_info, acs_param_info_t *param, int param_len, char *buf,
	int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "bw_cap", param, param_len, buf, buf_len);
}

int acs_get_cap_info(acs_chaninfo_t *c_info, uint32 *param, int param_len, char *cap_buf,
	int cap_len)
{
	return wl_iovar_getbuf(c_info->name, "cap", param, param_len, cap_buf, cap_len);
}

int acs_get_dfs_forced_chspec(acs_chaninfo_t *c_info, char smbuf[WLC_IOCTL_SMLEN])
{
	wl_dfs_forced_t inp;
	int ret = 0;
	acs_rsi_t *rsi = &c_info->rs_info;

	if (BAND_2G(rsi->band_type) || !rsi->reg_11h ||
		((BAND_5G(rsi->band_type)) && (c_info->acs_dfs == ACS_DFS_DISABLED))) {
		return -1;
	}

	inp.version = DFS_PREFCHANLIST_VER;
	ret = wl_iovar_getbuf(c_info->name, "dfs_channel_forced", &inp, sizeof(wl_dfs_forced_t),
		smbuf, WLC_IOCTL_SMLEN);

	return ret;
}

int acs_set_dfs_chan_forced(acs_chaninfo_t *c_info, wl_dfs_forced_t *dfs_frcd, int dfs_frcd_len)
{
	return wl_iovar_set(c_info->name, "dfs_channel_forced", dfs_frcd, dfs_frcd_len);
}

int acs_get_chanim_stats(acs_chaninfo_t *c_info, wl_chanim_stats_t *param, int param_len,
	char *buf, int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "chanim_stats", param, param_len, buf, buf_len);
}

int acs_get_dfsr_counters(char *ifname, char cntbuf[ACSD_WL_CNTBUF_SIZE])
{
	return wl_iovar_get(ifname, "counters", cntbuf, ACSD_WL_CNTBUF_SIZE);
}

wl_dfs_sub_status_t *
acs_bgdfs_sub_at(wl_dfs_ap_move_status_v2_t *st, uint8 at)
{
	void * vsst = (void*)(&(st)->scan_status.dfs_sub_status);

	return (wl_dfs_sub_status_t *) (vsst + at * sizeof(wl_dfs_sub_status_t));
}

/* gets (updates) bgdfs capability and status of the interface; returns bgdfs capability */
uint16
acs_bgdfs_get(acs_chaninfo_t * c_info)
{
	int ret = 0;
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;

	if (acs_bgdfs == NULL) {
		ACSD_ERROR("%s: acs_bgdfs is NULL", c_info->name);
		return BCME_ERROR;
	}

	ret = wl_iovar_get(c_info->name, "dfs_ap_move", (void *)&acs_bgdfs->status,
		sizeof(acs_bgdfs->status) + sizeof(acs_bgdfs->pad));
	if (ret != BCME_OK) {
		ACSD_INFO("%s: get dfs_ap_move returned %d.\n", c_info->name, ret);
		return acs_bgdfs->cap = BGDFS_CAP_UNSUPPORTED;
	}
	return acs_bgdfs->cap = BGDFS_CAP_TYPE0;
}

/* request bgdfs set; for valid values of 'arg' see help page of dfs_ap_move iovar */
int
acs_bgdfs_set(acs_chaninfo_t * c_info, int arg)
{
	int ret = 0;
	ret = wl_iovar_setint(c_info->name, "dfs_ap_move",
		(int)(htod32(arg)));
	if (arg > 0 && c_info->acs_bgdfs != NULL) {
		c_info->acs_bgdfs->last_attempted = arg;
		c_info->acs_bgdfs->last_attempted_at = (uint64) uptime();
	}
	if (ret != BCME_OK) {
		ACSD_ERROR("%s: set dfs_ap_move %d returned %d.\n", c_info->name, arg, ret);
	}
	return ret;
}

/* acs_set_oper_mode set the oper_mode */
int
acs_set_oper_mode(acs_chaninfo_t * c_info, uint16 oper_mode)
{
	int ret = BCME_ERROR;

	if ((ret = wl_iovar_setint(c_info->name, "oper_mode", oper_mode)) != BCME_OK) {
		ACSD_ERROR("%s setting oper_mode (0x%02x) failed with %d\n", c_info->name,
			oper_mode, ret);
		return ret;
	}

	c_info->oper_mode = oper_mode;
	ACSD_INFO("%s setting oper_mode succeeded 0x%02x\n", c_info->name, oper_mode);

	return ret;
}

int acs_get_dyn160_status(char *name, int *dyn160_status)
{
	return wl_iovar_getint(name, "dyn160", dyn160_status);
}

int acs_get_phydyn_switch_status(char *name, int *phy_dyn_switch)
{
	return wl_iovar_getint(name, "phy_dyn_switch", phy_dyn_switch);
}

int acs_set_intfer_traffic_thresh(char *name, wl_traffic_thresh_t *params, int size)
{
	return wl_iovar_set(name, "traffic_thresh", (void *)params, size);
}

int acs_set_intfer_params(char *name, wl_intfer_params_t *params, int size)
{
	return wl_iovar_set(name, "intfer_params", (void *)params, size);
}

int acs_get_stainfo(char *name, struct ether_addr *ea, int ether_len,
	char *stabuf, int buf_len)
{
	return wl_iovar_getbuf(name, "sta_info", &ea, sizeof(ea), stabuf, buf_len);
}

int acs_set_chanim_sample_period(char *name, uint sample_period)
{
	return wl_iovar_setint(name, "chanim_sample_period", sample_period);
}

int acs_set_noise_metric(char *name, uint8 knoise)
{
	return wl_iovar_setint(name, "noise_metric", knoise);
}

int acs_get_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size)
{
	return wl_iovar_get(ifname, "scb_probe", scb_probe, size);
}

int acs_set_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size_probe)
{
	return wl_iovar_set(ifname, "scb_probe", scb_probe, size_probe);
}

uint acs_get_chanim_scb_lastused(acs_chaninfo_t* c_info)
{
	uint lastused = 0;
	int ret;

	ret = wl_iovar_get(c_info->name, "scb_lastused", &lastused, sizeof(uint));

	if (ret < 0) {
		ACSD_ERROR("%s: failed to get scb_lastused", c_info->name);
		return 0;
	}

	ACSD_DEBUG("%s: lastused: %d\n", c_info->name, lastused);
	return lastused;
}

/* get country details for an interface */
int acs_get_country(acs_chaninfo_t * c_info)
{
	int ret = BCME_OK;

	ret = wl_iovar_get(c_info->name, "country", &c_info->country,
		sizeof(c_info->country));

	/* ensure null termination before logging/using */
	c_info->country.country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
	c_info->country.ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';

	if (ret != BCME_OK) {
		ACSD_ERROR("get country on %s returned %d.\n", c_info->name, ret);
	} else {
		int is_edcrs_eu;
		ret = wl_iovar_getint(c_info->name, "is_edcrs_eu", &is_edcrs_eu);
		c_info->country_is_edcrs_eu = dtoh32(is_edcrs_eu);
		ACSD_INFO("get country on %s returned %d. ca=%s, cr=%d, cc=%s is_edcrs_eu %d\n",
			c_info->name, ret,
			c_info->country.country_abbrev,
			c_info->country.rev, c_info->country.ccode,
			c_info->country_is_edcrs_eu);
	}

	return ret;
}

/* check if there is still associated scbs. reture value: TRUE if yes. */
bool acs_check_assoc_scb(acs_chaninfo_t * c_info)
{
	bool connected = TRUE;
	int result = 0;
	int ret = 0;

	ret = wl_iovar_getint(c_info->name, "scb_assoced", &result);
	if (ret) {
		ACSD_ERROR("%s: failed to get scb_assoced\n", c_info->name);
		return connected;
	}

	connected = dtoh32(result) ? TRUE : FALSE;
	ACSD_DEBUG("%s: connected: %d\n", c_info->name, connected);

	return connected;
}

int acs_update_driver(acs_chaninfo_t * c_info)
{
	int ret = 0;
	bool param = TRUE;
	/* if we are already beaconing, after the acs scan and new chanspec selection,
	   we need to ask the driver to do some updates (beacon, probes, etc..).
	*/
	if (c_info->txop_channel_select == 0) {
		if (c_info->wet_enabled && acs_check_assoc_scb(c_info)) {
			ACSD_INFO("%s: skip acs_update when ACSD is in WET mode and scb associated\n", c_info->name);
			return BCME_ASSOCIATED;
		}

		ret = wl_iovar_setint(c_info->name, "acs_update", htod32((uint)param));
		ACS_ERR(ret, "acs update failed\n");
	}

	return ret;
}

int chanim_update_state(acs_chaninfo_t *c_info, bool state)
{
	int ret;

	ret = wl_iovar_setint(c_info->name, "chanim_state", (uint)state);
	ACSD_CHANIM("%s: set chanim_state: %d\n", c_info->name, state);

	if (ret < 0) {
		ACSD_ERROR("%s: failed to set chanim_state", c_info->name);
	}
	return ret;
}

int dcs_handle_request(char* ifname, wl_bcmdcs_data_t *dcs_data,
	uint8 mode, uint8 count, uint8 csa_mode)
{
	wl_chan_switch_t csa;
	int err = ACSD_OK;

	ACSD_INFO("ifname: %s, reason: %d, chanspec: 0x%x, csa:%x\n",
		ifname, dcs_data->reason, dcs_data->chspec, csa_mode);

	csa.mode = mode;
	csa.count = count;
	csa.chspec = dcs_data->chspec;
	csa.reg = 0;
	csa.frame_type = csa_mode;

	err = wl_iovar_set(ifname, "csa", &csa, sizeof(wl_chan_switch_t));

	return err;
}

#ifdef ZDFS_2G
int acs_get_zdfs_2g(acs_chaninfo_t *c_info, int *zdfs_2g)
{
	return wl_iovar_getint(c_info->name, "zdfs_2g", zdfs_2g);
}

int acs_set_zdfs_2g(acs_chaninfo_t *c_info, int zdfs_2g)
{
	return wl_iovar_setint(c_info->name, "zdfs_2g", zdfs_2g);
}

/* chanim_stats us */
static int
acs_get_chanim_stats_us(acs_chaninfo_t *c_info, wl_chanim_stats_us_v1_t *param, int param_len,
	char *buf, int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "chanim_stats", param, param_len, buf, buf_len);
}

/* get txduration in secs from chanim_stats us */
int
acs_get_tx_dur_secs(acs_chaninfo_t *c_info)
{
	int ret = 0;
	char *data_buf;
	wl_chanim_stats_us_v1_t *list;
	wl_chanim_stats_us_v1_t param;
	int buflen = ACS_CHANIM_BUF_LEN;
	uint32 count = WL_CHANIM_COUNT_US_ONE;
	uint32 dur_sec = 0;

	data_buf = acsd_malloc(ACS_CHANIM_BUF_LEN);
	list = (wl_chanim_stats_us_v1_t *) data_buf;

	param.buflen = htod32(buflen);
	param.count = htod32(count);

	ret = acs_get_chanim_stats_us(c_info, &param, sizeof(wl_chanim_stats_us_v1_t), data_buf,
			buflen);
	if (ret < 0) {
		ACS_FREE(data_buf);
		ACSD_ERROR("%s FAILED to get chanim us results\n", c_info->name);
		return 0;
	}

	list->version = dtoh32(list->version);

	if (list->version == WL_CHANIM_STATS_US_VERSION_1) {
		chanim_stats_us_v1_t *stats_us_v1 = NULL;
		stats_us_v1 = (chanim_stats_us_v1_t *)list->stats_us_v1;
		dur_sec = stats_us_v1->ccastats_us[CCASTATS_TXDUR];
		ACSD_INFO("%s chanim ver:%d acs_get_tx_dur_secs dur_sec %u sec %d\n",
			c_info->name, list->version, dur_sec, dur_sec/1000000);
	} else if (list->version == WL_CHANIM_STATS_US_VERSION_2) {
		chanim_stats_us_v2_t *stats_us_v2 = NULL;
		stats_us_v2 = (chanim_stats_us_v2_t *)
			((wl_chanim_stats_us_v2_t *)list)->stats_us_v2;
		dur_sec = stats_us_v2->ccastats_us[CCASTATS_TXDUR];
		ACSD_INFO("%s chanim ver:%d acs_get_tx_dur_secs dur_sec %u sec %d\n",
			c_info->name, list->version, dur_sec, dur_sec/1000000);
	} else {
		ACSD_ERROR("%s chanim us Ver %d NOT matching with FW ver %d\n",
			c_info->name, list->version, WL_CHANIM_STATS_US_VERSION);
	}

	ACS_FREE(data_buf);

	return (dur_sec/1000000);
}
#endif /* ZDFS_2G */
