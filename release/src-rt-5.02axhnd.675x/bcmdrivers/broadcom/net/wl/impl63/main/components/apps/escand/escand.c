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
 * $Id: escand.c 769837 2018-11-28 06:31:29Z $
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

#include "escand_svr.h"

#define PREFIX_LEN 32

/* some channel bounds */
#define ESCAND_CS_MIN_2G_CHAN	1	/* min channel # in 2G band */
#define ESCAND_CS_MIN_5G_CHAN	36	/* min channel # in 5G band */
#define ESCAND_CS_MAX_5G_CHAN	MAXCHANNEL	/* max channel # in 5G band */

/* possible min channel # in the band */
#define ESCAND_CS_MIN_CHAN(band)	((band == WLC_BAND_5G) ? ESCAND_CS_MIN_5G_CHAN : \
			(band == WLC_BAND_2G) ? ESCAND_CS_MIN_2G_CHAN : 0)
/* possible max channel # in the band */
#define ESCAND_CS_MAX_CHAN(band)	((band == WLC_BAND_5G) ? ESCAND_CS_MAX_5G_CHAN : \
			(band == WLC_BAND_2G) ? ESCAND_CS_MAX_2G_CHAN : 0)

/* Need 13, strlen("per_chan_info"), +4, sizeof(uint32). Rounded to 20. */
#define ESCAND_PER_CHAN_INFO_BUF_LEN 20

#define ESCAND_DFLT_FLAGS ESCAND_FLAGS_LASTUSED_CHK

escand_info_t *escand_info;

/* get traffic information of the interface */
static int escand_get_traffic_info(escand_chaninfo_t * c_info, escand_traffic_info_t *t_info);

/* get traffic information about TOAd video STAs (if any) */
static int escand_get_video_sta_traffic_info(escand_chaninfo_t * c_info,
	escand_traffic_info_t *t_info);

/* To check whether bss is enabled for particaular interface or not */
static int
escand_check_bss_is_enabled(char *name, escand_chaninfo_t **c_info_ptr, char *prefix);

#ifdef DEBUG
static void
escand_dump_config_extra(escand_chaninfo_t *c_info)
{
	ESCAND_INFO("escand_dump_config_extra:\n");
	ESCAND_INFO("\t escand_far_sta_rssi: %d\n", c_info->escand_far_sta_rssi);
	ESCAND_INFO("\t escand_cs_scan_timer: %d\n", c_info->escand_cs_scan_timer);
	ESCAND_INFO("\t escand_ci_scan_timeout: %d\n", c_info->escand_ci_scan_timeout);
	ESCAND_INFO("\t escand_ci_scan_timer: %d\n\n", c_info->escand_ci_scan_timer);
}
#endif /* DEBUG */

#ifdef ESCAN_DEBUG
static void
escand_dump_map(void)
{
	int i;
	ifname_idx_map_t* cur_map;

	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		cur_map = &escand_info->escand_ifmap[i];
		if (cur_map->in_use) {
			ESCAND_PRINT("i: %d, name: %s, idx: %d, in_use: %d\n",
				i, cur_map->name, cur_map->idx, cur_map->in_use);
		}
	}
}
#endif /* ESCAN_DEBUG */

static void
escand_add_map(char *name)
{
	int i;
	ifname_idx_map_t* cur_map = escand_info->escand_ifmap;
	size_t length = strlen(name);

	ESCAND_DEBUG("add map entry for ifname: %s\n", name);

	if (length >= sizeof(cur_map->name)) {
		ESCAND_ERROR("Interface Name Length Exceeded\n");
	} else {
		for (i = 0; i < ESCAND_MAX_IF_NUM; cur_map++, i++) {
			if (!cur_map->in_use) {
				memcpy(cur_map->name, name, length + 1);
				cur_map->idx = i;
				cur_map->in_use = TRUE;
				break;
			}
		}
	}
#ifdef ESCAN_DEBUG
	escand_dump_map();
#endif // endif
}

int
escand_idx_from_map(char *name)
{
	int i;
	ifname_idx_map_t *cur_map;

#ifdef ESCAN_DEBUG
	escand_dump_map();
#endif // endif
	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		cur_map = &escand_info->escand_ifmap[i];
		if (cur_map->in_use && !strcmp(name, cur_map->name)) {
			ESCAND_DEBUG("name: %s, cur_map->name: %s idx: %d\n",
				name, cur_map->name, cur_map->idx);
			return cur_map->idx;
		}
	}
	ESCAND_ERROR("cannot find the mapped entry for ifname: %s\n", name);
	return -1;
}

/* radio setting information needed from the driver */
static int
escand_get_rs_info(escand_chaninfo_t * c_info, char* prefix)
{
	int ret = 0;
	char tmp[100];
	int band, pref_chspec = 0, coex;
	escand_rsi_t *rsi = &c_info->rs_info;
	char *str;
	union {
		char	data_buf[100];
		uint32	bw_cap;
	} data_buf_u;

	escand_param_info_t param;
	/*
	 * Check if the user set the "chanspec" nvram. If not, check if
	 * the "channel" nvram is set for backward compatibility.
	 */
	if ((str = nvram_get(strcat_r(prefix, "chanspec", tmp))) == NULL) {
		str = nvram_get(strcat_r(prefix, "channel", tmp));
	}

	if (str && strcmp(str, "0")) {
		ret = escand_get_chanspec(c_info, &pref_chspec);
		ESCAND_ERR(ret, "failed to get chanspec");

		rsi->pref_chspec = dtoh32(pref_chspec);
	}

	ret = escand_get_obss_coex_info(c_info, &coex);
	ESCAND_ERR(ret, "failed to get obss_coex");

	rsi->coex_enb = dtoh32(coex);
	ESCAND_INFO("coex_enb: %d\n",  rsi->coex_enb);

	ret = wl_ioctl(c_info->name, WLC_GET_BAND, &band, sizeof(band));
	ESCAND_ERR(ret, "failed to get band info");

	rsi->band_type = dtoh32(band);
	ESCAND_INFO("band_type: %d\n",  rsi->band_type);

	memset(&param, 0, sizeof(param));
	param.band = band;

	ret = escand_get_bwcap_info(c_info, &param, sizeof(param), data_buf_u.data_buf,
		sizeof(data_buf_u.data_buf));
	ESCAND_ERR(ret, "failed to get bw_cap");

	rsi->bw_cap = data_buf_u.bw_cap;
	ESCAND_INFO("bw_cap: %d\n",  rsi->bw_cap);

	return ret;
}

/* look for str in capability (wl cap) and return true if found */
bool
escand_check_cap(escand_chaninfo_t *c_info, char *str)
{
	char data_buf[WLC_IOCTL_MAXLEN];
	uint32 ret, param = 0;

	if (str == NULL || strlen(str) >= WLC_IOCTL_SMLEN) {
		ESCAND_ERROR("%s invalid needle to look for in cap\n", c_info->name);
		return FALSE;
	}

	ret = escand_get_cap_info(c_info, &param, sizeof(param), data_buf, sizeof(data_buf));

	if (ret != BCME_OK) {
		ESCAND_ERROR("%s Error %d in getting cap\n", c_info->name, ret);
		return FALSE;
	}

	data_buf[WLC_IOCTL_MAXLEN - 1] = '\0';
	if (strstr(data_buf, str) == NULL) {
		ESCAND_INFO("%s '%s' not found in cap\n", c_info->name, str);
		return FALSE;
	} else {
		ESCAND_INFO("%s '%s' found in cap\n", c_info->name, str);
		return TRUE;
	}
}

int
escand_start(char *name, escand_chaninfo_t *c_info)
{
	int unit;
	char prefix[PREFIX_LEN], tmp[100];
	escand_rsi_t* rsi;
	int ret = 0;

	ESCAND_PRINT("escand_start for interface %s\n", name);

	ret = wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
	escand_snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	/* check radio */
	if (nvram_match(strcat_r(prefix, "radio", tmp), "0")) {
		ESCAND_INFO("ifname %s: radio is off\n", name);
		c_info->mode = ESCAND_MODE_DISABLE;
		goto escand_start_done;
	}

	escand_retrieve_config(c_info, prefix);

	if ((ret = escand_get_country(c_info)) != BCME_OK)
		ESCAND_ERROR("Failed to get country info\n");

	rsi = &c_info->rs_info;
	BCM_REFERENCE(rsi);
	escand_get_rs_info(c_info, prefix);

	c_info->mode = ESCAND_MODE_MONITOR; /* default mode */

	/* When escand starts, retrieve current traffic stats since boot */
	escand_get_initial_traffic_stats(c_info);

	ret = escand_build_scanlist(c_info);
	ESCAND_ERR(ret, "failed to build scan chanspec list");

escand_start_done:
	return ret;
}

static int
escand_check_bss_is_enabled(char *name, escand_chaninfo_t **c_info_ptr, char *prefix)
{
	int index, ret;
	char buf[32] = { 0 }, *bss_check;

	if (strlen(name) >= sizeof((*c_info_ptr)->name)) {
		ESCAND_ERROR("Interface Name Length Exceeded\n");
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
		ESCAND_INFO("interface is disabled %s\n", name);
		return BCME_DISABLED;
	}

	escand_add_map(name);
	index = escand_idx_from_map(name);

	if (index < 0) {
		ret = ESCAND_FAIL;
		ESCAND_ERR(ret, "Mapped entry not present for interface");
	}

	/* allocate core data structure for this interface */
	*c_info_ptr = escand_info->chan_info[index] =
		(escand_chaninfo_t*)escand_malloc(sizeof(escand_chaninfo_t));
	strncpy((*c_info_ptr)->name, name, sizeof((*c_info_ptr)->name));
	(*c_info_ptr)->name[sizeof((*c_info_ptr)->name) - 1] = '\0';
	ESCAND_INFO("bss enabled for name :%s\n", (*c_info_ptr)->name);
	return BCME_OK;
}

/*
 * Returns the channel info of the chspec passed (by combining per_chan_info of each 20MHz subband)
 */
uint32
escand_channel_info(escand_chaninfo_t *c_info, chanspec_t chspec)
{
	union {
		char	resbuf[ESCAND_PER_CHAN_INFO_BUF_LEN];
		uint32	sub_chinfo;
	} sc;
	int ret;
	uint8 sub_channel;
	chanspec_t sub_chspec;
	uint32 chinfo = 0, max_inactive = 0, sub_chinfo;

	FOREACH_20_SB(chspec, sub_channel) {
		sub_chspec = (uint16) sub_channel;
		ret = escand_get_per_chan_info(c_info, sub_chspec, sc.resbuf,
			ESCAND_PER_CHAN_INFO_BUF_LEN);
		if (ret != BCME_OK) {
			ESCAND_ERROR("%s Failed to get channel (0x%02x) info: %d\n",
				c_info->name, sub_chspec, ret);
			return 0;
		}

		sub_chinfo = dtoh32(sc.sub_chinfo);
		ESCAND_INFO("%s: sub_chspec 0x%04x info %08x (%s, %d minutes)\n",
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

	ESCAND_INFO("%s: chanspec 0x%04x info %08x (%s, %d minutes)\n",
		c_info->name, chspec, chinfo,
		(chinfo & WL_CHAN_INACTIVE) ? "inactive" :
		((chinfo & WL_CHAN_PASSIVE) ? "passive" : "active"),
		GET_INACT_TIME(chinfo));

	return chinfo;
}

/* for 160M/80p80M/80M/40 bw chanspec,select control chan
 * with max AP number for neighbor friendliness
 *
 * For 80p80 - adjust ctrl chan within primary 80Mhz
 */
chanspec_t
escand_adjust_ctrl_chan(escand_chaninfo_t *c_info, chanspec_t chspec)
{
	chanspec_t selected = chspec;
	escand_chan_bssinfo_t* bss_info = c_info->ch_bssinfo;
	uint8 i, j, max_sb, ch, channel;
	uint8 ctrl_sb[8] = {0}, num_sb[8] = {0};
	uint8 selected_sb, last_chan_idx = 0;
	escand_conf_chspec_t *excl_chans = &(c_info->excl_chans);

	if (nvram_match("escand_ctrl_chan_adjust", "0"))
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
				ESCAND_INFO("sb:%d channel = %d num_sb = %d\n", i,
						channel, num_sb[i]);
				break;
			}
		}
		i++;
	}

	/* when dyn160 is enabled with DFS on FCC, control ch of 50o must be ULL or higher */
	if (max_sb == 8 && c_info->dyn160_enabled && ESCAND_11H(c_info) &&
		CHSPEC_IS160(selected) &&
		!c_info->country_is_edcrs_eu &&
		CHSPEC_CHANNEL(selected) == ESCAND_DYN160_CENTER_CH) {
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
			ESCAND_INFO("selected sb so far = %d n_sbs = %d\n",
					selected_sb, num_sb[selected_sb]);
		}
	}

	ESCAND_INFO("selected sb: %d\n", selected_sb);
	selected &=  ~(WL_CHANSPEC_CTL_SB_MASK);
	selected |= (selected_sb << WL_CHANSPEC_CTL_SB_SHIFT);
	ESCAND_INFO("Final selected chanspec: 0x%4x\n", selected);
	return selected;
}

/* return TRUE if ESCAND is in AP mode else return FALSE
 */
bool
escand_is_mode_check(char *osifname)
{
	char tmp[32], prefix[PREFIX_LEN];

	if (strstr(osifname, "eth")) {
		osifname_to_nvifname(osifname, prefix, sizeof(prefix));
		strcat(prefix, "_");
	} else {
		make_wl_prefix(prefix, sizeof(prefix), 0, osifname);
	}

	nvram_safe_get(strcat_r(prefix, "mode", tmp));

	if (nvram_match(tmp, "psr") || nvram_match(tmp, "sta") || nvram_match(tmp, "wet") ||
			nvram_match(tmp, "dwds") || nvram_match(tmp, "psta")) {
		return FALSE;
	}
	return TRUE;
}

static void
escand_init_info(escand_info_t ** escand_info_p)
{
	escand_info = (escand_info_t*)escand_malloc(sizeof(escand_info_t));

	*escand_info_p = escand_info;
}

/* This function is used to get enabled virtual ifnames and
 * store it in local structure
 */
static void
escand_store_vifnames(void)
{
	ifname_idx_map_t *cur_map;
	char prefix[8], *ifnames[16] = {0};
	char *token, *delim = ",";
	char *vifname;
	int i, j = 0;
	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		cur_map = &escand_info->escand_ifmap[i];
		memset(prefix, 0, sizeof(prefix));
		osifname_to_nvifname(cur_map->name, prefix, sizeof(prefix));
		strcat(prefix, "_vifs");
		vifname = nvram_safe_get(prefix);
		token = strtok(vifname, delim);
		while (token != NULL) {
			ifnames[j] = token;
			escand_info->chan_info[i]->vifnames[j] = ifnames[j];
			ESCAND_INFO("ifname %s and corresponding vifnames are %s\n",
				cur_map->name, escand_info->chan_info[i]->vifnames[j]);
			token = strtok(NULL, delim);
			j++;
		}
		j = 0;
	}
}

/* This function will get available exclude_ifname list by using nvram
 * (escand_exclude_ifnames) and copy to exclude_ifname variable
 */
void escand_get_exclude_interface_list(void)
{
	char *token;
	char *delim = ",";
	int i = 0;
	char *p = NULL, *excl_ifname[ESCAND_EXCLUDE_IFACE_LIST] = {0};
	p = nvram_safe_get("escand_exclude_ifnames");
	token = strtok(p, delim);
	while (token != NULL) {
		excl_ifname[i] = token;
		escand_info->exclude_ifnames[i] = excl_ifname[i];
		ESCAND_INFO("exclude interface list is %s \n", escand_info->exclude_ifnames[i]);
		token = strtok(NULL, delim);
		i++;
	}
}

void
escand_init_run(escand_info_t ** escand_info_p)
{
	char name[16], *next, prefix[PREFIX_LEN], name_enab_if[32] = { 0 }, *vifname, *vif_next;
	escand_chaninfo_t * c_info;
	int ret = 0;

	escand_init_info(escand_info_p);

	foreach(name, nvram_safe_get("escand_ifnames"), next) {
		c_info = NULL;
		osifname_to_nvifname(name, prefix, sizeof(prefix));
		if (escand_check_bss_is_enabled(name, &c_info, prefix) != BCME_OK) {
			strcat(prefix, "_vifs");
			vifname = nvram_safe_get(prefix);
			foreach(name_enab_if, vifname, vif_next) {
				if (escand_check_bss_is_enabled(name_enab_if,
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
		ret = escand_start(name, c_info);

		if (ret) {
			ESCAND_ERROR("escand_start failed for ifname: %s\n", name);
			continue;
		}

		if (c_info->escand_boot_only) {
			c_info->mode = ESCAND_MODE_DISABLE;
		}
	}
	escand_store_vifnames();
	escand_get_exclude_interface_list();
}

int
escand_update_status(escand_chaninfo_t * c_info)
{
	int ret = 0;
	int cur_chspec;

	ret = wl_iovar_getint(c_info->name, "chanspec", &cur_chspec);
	ESCAND_ERR(ret, "escand get chanspec failed\n");

	/* return if the channel hasn't changed */
	if ((chanspec_t)dtoh32(cur_chspec) == c_info->cur_chspec) {
		return ret;
	}

	/* To add a escand_record when finding out channel change isn't made by ESCAND */
	c_info->cur_chspec = (chanspec_t)dtoh32(cur_chspec);
	c_info->cur_is_dfs = escand_is_dfs_chanspec(c_info, cur_chspec);
	c_info->cur_is_dfs_weather = escand_is_dfs_weather_chanspec(c_info, cur_chspec);
	c_info->is160_bwcap = WL_BW_CAP_160MHZ((c_info->rs_info).bw_cap);

	ESCAND_INFO("%s: chanspec:0x%x is160_bwcap %d is160_upgradable %d, is160_downgradable %d\n",
		c_info->name, c_info->cur_chspec, c_info->is160_bwcap,
		c_info->is160_upgradable, c_info->is160_downgradable);

	return ret;
}

int escand_idle_check(escand_chaninfo_t *c_info)
{
	uint timer = c_info->escand_cs_scan_timer;
	time_t now = time(NULL);
	int full_scan = 0;

	/* Check for idle period "escand_cs_scan_timer" */
	if ((now - c_info->timestamp_tx_idle) < timer)
		return full_scan;

	ESCAND_INFO("escand_idle: now %u(%u)\n", (uint)now, c_info->timestamp_tx_idle);

	c_info->timestamp_tx_idle = now;
	return full_scan;
}

void
escan_cleanup(escand_info_t ** escand_info_p)
{
	int i;

	if (!*escand_info_p)
		return;

	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		escand_chaninfo_t* c_info = (*escand_info_p)->chan_info[i];

		if (!c_info)
			continue;

		ESCAND_FREE(c_info->scan_results);

		if (c_info->escand_escan && c_info->escand_escan->escand_use_escan)
			escand_escan_free(c_info->escand_escan->escan_bss_head);

		escand_cleanup_scan_entry(c_info);
		ESCAND_FREE(c_info->ch_bssinfo);
		ESCAND_FREE(c_info->scan_chspec_list.chspec_list);

		ESCAND_FREE(c_info);
	}
	ESCAND_FREE(escand_info);
	*escand_info_p = NULL;
}

/* This function is used to check whether ifname is virtual or primary,
 * if ifname is virtual convert it from virtual to corresponding
 * primary interface
 */
void
escand_check_ifname_is_virtual(char **ifname)
{
	ifname_idx_map_t *cur_map;
	int i, j;
	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		cur_map = &escand_info->escand_ifmap[i];
		for (j = 0; j < ESCAND_MAX_VIFNAMES; j++) {
			if (escand_info->chan_info[i] != NULL &&
				escand_info->chan_info[i]->vifnames[j] != NULL && *ifname != NULL) {
				if (!strncmp(escand_info->chan_info[i]->vifnames[j], *ifname,
					strlen(*ifname))) {
					ESCAND_INFO("found vir intf %s converting to corresponding "
					"primary intf %s and iface length is %zu\n",
					escand_info->chan_info[i]->vifnames[j], cur_map->name,
					strlen(*ifname));
					*ifname = cur_map->name;
					break;
				}
			}
		}
	}
}

int escand_update_rssi(escand_chaninfo_t *c_info, unsigned char *addr)
{
	int ret = 0;
	scb_val_t scb_val;

	/* reset assoc STA staus */
	c_info->sta_status = ESCAND_STA_NONE;

	memset(&scb_val, 0, sizeof(scb_val));
	memcpy(&scb_val.ea, addr, ETHER_ADDR_LEN);

	ret = wl_ioctl(c_info->name, WLC_GET_RSSI, &scb_val, sizeof(scb_val));

	if (ret < 0) {
		ESCAND_ERROR("Err: reading intf:%s STA:"MACF" RSSI %d\n",
			c_info->name, ETHERP_TO_MACF(&scb_val.ea),
			dtoh32(scb_val.val));
		return ret;
	}
	ESCAND_5G("txfail sta intf:%s Mac:%02x:%02x:%02x:%02x:%02x:%02x RSSI: %d\n",
		c_info->name, addr[0], addr[1], addr[2], addr[3],
		addr[4], addr[5], dtoh32(scb_val.val));

	if (dtoh32(scb_val.val) < c_info->escand_far_sta_rssi) {
		c_info->sta_status |= ESCAND_STA_EXIST_FAR;
	} else {
		c_info->sta_status |= ESCAND_STA_EXIST_CLOSE;
	}

	ESCAND_5G("%s@%d sta_status:0x%x\n", __FUNCTION__, __LINE__,
		c_info->sta_status);
	return ret;
}

int escand_update_assoc_info(escand_chaninfo_t *c_info)
{
	struct maclist *list;
	escand_assoclist_t *escand_assoclist;
	int ret = 0, cnt, size;

	/* reset assoc STA staus */
	c_info->sta_status = ESCAND_STA_NONE;

	ESCAND_INFO("%s: %s@%d\n", c_info->name, __FUNCTION__, __LINE__);

	/* read assoclist */
	list = (struct maclist *)escand_malloc(ESCAND_BUFSIZE_4K);
	memset(list, 0, ESCAND_BUFSIZE_4K);
	ESCAND_INFO("%s: WLC_GET_ASSOCLIST\n", c_info->name);
	list->count = htod32((ESCAND_BUFSIZE_4K - sizeof(int)) / ETHER_ADDR_LEN);
	ret = wl_ioctl(c_info->name, WLC_GET_ASSOCLIST, list, ESCAND_BUFSIZE_4K);
	if (ret < 0) {
		ESCAND_ERROR("WLC_GET_ASSOCLIST failure\n");
		ESCAND_FREE(list);
		return ret;
	}

	ESCAND_FREE(c_info->escand_assoclist);
	list->count = dtoh32(list->count);
	if (list->count <= 0) {
		ESCAND_FREE(list);
		return ret;
	}

	size = sizeof(escand_assoclist_t) + (list->count)* sizeof(escand_sta_info_t);
	escand_assoclist = (escand_assoclist_t *)escand_malloc(size);

	c_info->escand_assoclist = escand_assoclist;
	escand_assoclist->count = list->count;

	for (cnt = 0; cnt < list->count; cnt++) {
		scb_val_t scb_val;

		memset(&scb_val, 0, sizeof(scb_val));
		memcpy(&scb_val.ea, &list->ea[cnt], ETHER_ADDR_LEN);

		ret = wl_ioctl(c_info->name, WLC_GET_RSSI, &scb_val, sizeof(scb_val));

		if (ret < 0) {
			ESCAND_ERROR("Err: reading intf:%s STA:"MACF" RSSI\n",
					c_info->name, ETHER_TO_MACF(list->ea[cnt]));
			ESCAND_FREE(c_info->escand_assoclist);
			break;
		}

		escand_assoclist->sta_info[cnt].rssi = dtoh32(scb_val.val);
		ether_copy(&(list->ea[cnt]), &(escand_assoclist->sta_info[cnt].ea));
		ESCAND_INFO("%s: %s@%d sta_info sta:"MACF" rssi:%d [%d]\n",
				c_info->name, __FUNCTION__, __LINE__,
				ETHER_TO_MACF(list->ea[cnt]), dtoh32(scb_val.val),
				c_info->escand_far_sta_rssi);

		if (escand_assoclist->sta_info[cnt].rssi < c_info->escand_far_sta_rssi)
			c_info->sta_status |= ESCAND_STA_EXIST_FAR;
		else
			c_info->sta_status |= ESCAND_STA_EXIST_CLOSE;

		ESCAND_INFO("%s: %s@%d sta_status:0x%x\n", c_info->name,
			__FUNCTION__, __LINE__, c_info->sta_status);
	}
	ESCAND_FREE(list);

	return ret;
}

/* get traffic information of the interface */
static int
escand_get_traffic_info(escand_chaninfo_t * c_info, escand_traffic_info_t *t_info)
{
	char cntbuf[ESCAND_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	int ret = BCME_OK;

	if (escand_get_counters(c_info->name, cntbuf) < 0) {
		ESCAND_INFO("Failed to fetch interface counters for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	if (wl_cntbuf_to_xtlv_format(NULL, cntbuf, ESCAND_WL_CNTBUF_SIZE, 0)
		!= BCME_OK) {
		ESCAND_INFO("wl_cntbuf_to_xtlv_format failed for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	if ((wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntinfo)) == NULL) {
		ESCAND_INFO("GET_WLCCNT_FROM_CNTBUF NULL for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	t_info->timestamp = time(NULL);
	t_info->txbyte = wlc_cnt->txbyte;
	t_info->rxbyte = wlc_cnt->rxbyte;
	t_info->txframe = wlc_cnt->txframe;
	t_info->rxframe = wlc_cnt->rxframe;
exit:
	return ret;
}

/* get traffic information about TOAd video STAs (if any) */
static int
escand_get_video_sta_traffic_info(escand_chaninfo_t * c_info, escand_traffic_info_t *t_info)
{
	char stabuf[ESCAND_MAX_STA_INFO_BUF];
	sta_info_v6_t *sta;
	int i, ret = BCME_OK;
	int index = c_info->video_sta_idx;

	struct ether_addr ea;
	escand_traffic_info_t total;

	memset(&total, 0, sizeof(escand_traffic_info_t));
	/* Consolidate the traffic info of all video stas */
	for (i = 0; i < index; i++) {
		memset(stabuf, 0, sizeof(stabuf));
		memcpy(&ea, &c_info->vid_sta[i].ea, sizeof(ea));
		if (escand_get_stainfo(c_info->name, &ea, sizeof(ea), stabuf,
				ESCAND_MAX_STA_INFO_BUF) < 0) {
			ESCAND_ERROR("sta_info for %s failed\n", c_info->vid_sta[i].vid_sta_mac);
			return BCME_ERROR;
		}
		sta = (sta_info_v6_t *)stabuf;
		total.txbyte = total.txbyte + dtoh64(sta->tx_tot_bytes);
		total.rxbyte = total.rxbyte + dtoh64(sta->rx_tot_bytes);
		total.txframe = total.txframe + dtoh32(sta->tx_tot_pkts);
		total.rxframe = total.rxframe + dtoh32(sta->rx_tot_pkts);
	}
	t_info->timestamp = time(NULL);
	t_info->txbyte = total.txbyte;
	t_info->rxbyte = total.rxbyte;
	t_info->txframe = total.txframe;
	t_info->rxframe = total.rxframe;
	return ret;
}

/*
 * escand_get_initial_traffic_stats - retrieve and store traffic activity info when escand starts
 *
 * c_info - pointer to escand_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
escand_get_initial_traffic_stats(escand_chaninfo_t *c_info)
{
	escand_activity_info_t *escand_act = &c_info->escand_activity;
	escand_traffic_info_t *t_prev = &escand_act->prev_bss_traffic;
	escand_traffic_info_t t_curr;
	int ret;

	if (!c_info->escand_toa_enable) {
		if ((ret = escand_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ESCAND_ERROR("Failed to get traffic information\n");
			return ret;
		}
	} else {
		if ((ret = escand_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ESCAND_ERROR("Failed to get video sta traffic information\n");
			return ret;
		}
	}

	t_prev->txframe = t_curr.txframe;
	t_prev->rxframe = t_curr.rxframe;

	return BCME_OK;
}

/*
 * escand_activity_update - updates traffic activity information
 *
 * c_info - pointer to escand_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
escand_activity_update(escand_chaninfo_t * c_info)
{
	escand_activity_info_t *escand_act = &c_info->escand_activity;
	time_t now = time(NULL);
	escand_traffic_info_t t_curr;
	escand_traffic_info_t *t_prev = &escand_act->prev_bss_traffic;
	escand_traffic_info_t *t_accu_diff = &escand_act->accu_diff_bss_traffic;
	escand_traffic_info_t *t_prev_diff = &escand_act->prev_diff_bss_traffic;
	uint32 total_frames; /* total tx and rx frames on link */
	int ret;

	if (!c_info->escand_toa_enable) {
		if ((ret = escand_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ESCAND_ERROR("Failed to get traffic information\n");
			return ret;
		}
	} else {
		if ((ret = escand_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ESCAND_ERROR("Failed to get video sta traffic information\n");
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

	escand_act->num_accumulated++;

	total_frames =  t_prev_diff->txframe + t_prev_diff->rxframe;
	BCM_REFERENCE(total_frames);

	/* save current in t_prev (previous) to help with next time delta calculation */
	memcpy(t_prev, &t_curr, sizeof(*t_prev));

	return BCME_OK;
}
