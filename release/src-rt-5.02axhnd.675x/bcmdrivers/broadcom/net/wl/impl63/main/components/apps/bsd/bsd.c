/*
 * bsd deamon (Linux)
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
 * $Id: bsd.c $
 */

#include "bsd.h"

extern bsd_info_t *bsd_info;

/* Function to process bss-trans response. */
static int bsd_process_bss_trans_resp(void *data, int response);

/* Struct to store wlif lib callback data. */
typedef struct bsd_steerlib_callback_data {
	bsd_info_t *info;			/* bsd info.  */
	bsd_bssinfo_t *steer_bssinfo;		/* steer bssinfo ptr. */
	bsd_sta_info_t *sta;			/* sta ptr. */
} bsd_steerlib_callback_data_t;

/* add addr to maclist */
void bsd_addto_maclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr, bsd_bssinfo_t *to_bssinfo)
{
	bsd_maclist_t *ptr;

	BSD_ENTER();
	BSD_STEER("Add mac:"MACF" to %s: macmode: %d\n",
		ETHERP_TO_MACF(addr), bssinfo->ifnames, bssinfo->macmode);

	/* adding to maclist */
	ptr = bssinfo->maclist;
	while (ptr) {
		BSD_STEER("Sta:"MACF"\n", ETHER_TO_MACF(ptr->addr));
		if (eacmp(&(ptr->addr), addr) == 0) {
			break;
		}
		ptr = ptr->next;
	}

	if (!ptr) {
		/* add sta to maclist */
		ptr = malloc(sizeof(bsd_maclist_t));
		if (!ptr) {
			BSD_STEER("Err: Exiting %s@%d malloc failure\n", __FUNCTION__, __LINE__);
			return;
		}
		memset(ptr, 0, sizeof(bsd_maclist_t));
		memcpy(&ptr->addr, addr, sizeof(struct ether_addr));
		ptr->next = bssinfo->maclist;
		bssinfo->maclist = ptr;
	}

	ptr->timestamp = time(NULL);

	ptr->to_bssinfo = to_bssinfo;
	ptr->steer_state = to_bssinfo?BSD_STA_STEERING:BSD_STA_INVALID;

	bssinfo->macmode = WLC_MACMODE_DENY;

	if (BSD_DUMP_ENAB) {
		BSD_PRINT("prting bssinfo macmode:%d Maclist: \n", bssinfo->macmode);
		ptr = bssinfo->maclist;
		while (ptr) {
			BSD_PRINT("Sta:"MACF"\n", ETHER_TO_MACF(ptr->addr));
			ptr = ptr->next;
		}
	}

	BSD_EXIT();
	return;
}

/* remove addr from maclist */
void bsd_remove_maclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	bsd_maclist_t *ptr, *prev;

	BSD_ENTER();

	/* removing from steer-ed intf maclist */
	BSD_STEER("Remove mac:"MACF"from %s: macmode: %d\n",
		ETHERP_TO_MACF(addr), bssinfo->ifnames, bssinfo->macmode);

	ptr = bssinfo->maclist;
	if (!ptr) {
		BSD_STEER("%s Steer-ed maclist empty. Exiting....\n", __FUNCTION__);
		return;
	}

	if (eacmp(&(ptr->addr), addr) == 0) {
		BSD_STEER("foudn/free maclist: "MACF"\n", ETHER_TO_MACF(ptr->addr));
		bssinfo->maclist = ptr->next;
		free(ptr);
	} else {
		prev = ptr;
		ptr = ptr->next;

		while (ptr) {
			BSD_STEER("checking maclist"MACF"\n", ETHER_TO_MACF(ptr->addr));
			if (eacmp(&(ptr->addr), addr) == 0) {
				BSD_STEER("found/free maclist: "MACF"\n", ETHER_TO_MACF(ptr->addr));
				prev->next = ptr->next;
				free(ptr);
				break;
			}
			prev = ptr;
			ptr = ptr->next;
		}
	}

	BSD_STEER("prting steer-ed bssinfo macmode:%d Maclist: \n", bssinfo->macmode);
	ptr = bssinfo->maclist;
	while (ptr) {
		BSD_STEER("Sta:"MACF"\n", ETHER_TO_MACF(ptr->addr));
		ptr = ptr->next;
	}

	BSD_EXIT();
	return;
}

/* update tstamp */
void bsd_stamp_maclist(bsd_info_t *info, bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	bsd_maclist_t *ptr;

	BSD_ENTER();

	ptr = bssinfo->maclist;
	if (!ptr) {
		BSD_STEER("%s [%s] maclist empty. Exiting....\n", __FUNCTION__, bssinfo->ifnames);
		return;
	}

	while (ptr) {
		BSD_STEER("checking maclist"MACF"\n", ETHER_TO_MACF(ptr->addr));
		if (eacmp(&(ptr->addr), addr) == 0) {
			BSD_INFO("found maclist: "MACF"\n", ETHER_TO_MACF(ptr->addr));
			if (info->maclist_timeout >= 5)
				ptr->timestamp = info->maclist_timeout - 5;
			break;
		}
		ptr = ptr->next;
	}

	BSD_EXIT();
	return;
}

/* find maclist */
bsd_maclist_t *bsd_maclist_by_addr(bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	bsd_maclist_t *ptr;

	BSD_ENTER();

	ptr = bssinfo->maclist;
	if (!ptr) {
		BSD_STEER("%s [%s] maclist empty. Exiting....\n", __FUNCTION__, bssinfo->ifnames);
		return NULL;
	}

	while (ptr) {
		BSD_STEER("checking maclist"MACF"\n", ETHER_TO_MACF(ptr->addr));
		if (eacmp(&(ptr->addr), addr) == 0) {
			BSD_INFO("found maclist: "MACF"\n", ETHER_TO_MACF(ptr->addr));
			break;
		}
		ptr = ptr->next;
	}

	BSD_EXIT();
	return ptr;
}

/* find maclist */
static int bsd_static_maclist_by_addr(bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	struct maclist *static_maclist = bssinfo->static_maclist;
	int cnt;
	int ret = BSD_FAIL;

	BSD_ENTER();

	BSD_STEER("Check static_maclist with "MACF"\n", ETHERP_TO_MACF(addr));

	if (static_maclist) {
		BSD_STEER("static_mac: macmode[%d] cnt[%d]\n",
			bssinfo->static_macmode, static_maclist->count);
		for (cnt = 0; cnt < static_maclist->count; cnt++) {
			BSD_INFO("cnt[%d] mac:"MACF"\n", cnt,
				ETHER_TO_MACF(static_maclist->ea[cnt]));
			if (eacmp(&(static_maclist->ea[cnt]), addr) == 0) {
				BSD_INFO("found mac: "MACF"\n", ETHERP_TO_MACF(addr));
				ret = BSD_OK;
				break;
			}
		}
	}

	BSD_EXIT();
	return ret;
}

/* check if STA is deny in steerable intf */
int bsd_aclist_steerable(bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	int ret = BSD_OK;

	BSD_ENTER();

	switch (bssinfo->static_macmode) {
		case WLC_MACMODE_DENY:
			if (bsd_static_maclist_by_addr(bssinfo, addr) == BSD_OK) {
				BSD_STEER("Deny: skip STA:"MACF"\n", ETHERP_TO_MACF(addr));
				ret = BSD_FAIL;
			}
			break;
		case WLC_MACMODE_ALLOW:
			if (bsd_static_maclist_by_addr(bssinfo, addr) != BSD_OK) {
				BSD_STEER("Allow: skip STA:"MACF"\n", ETHERP_TO_MACF(addr));
				ret = BSD_FAIL;
			}
			break;
		default:
			break;
	}

	BSD_EXIT();
	return ret;
}

uint bsd_steer_rec_idx = 0;
bool bsd_steer_rec_wrapped = FALSE;
bsd_steer_record_t bsd_steer_records[BSD_MAX_STEER_REC];

typedef struct {
	uint reason;
	const char *name;
} bsd_reason_name_str_t;

static const bsd_reason_name_str_t bsd_reason_names[] = {
	{BSD_STEERING_RESULT_SUCC, "steer succ"},
	{BSD_STEERING_RESULT_FAIL, "steer fail"},
	{BSD_STEERING_POLICY_FLAG_RULE, "logic AND chk"},
	{BSD_STEERING_POLICY_FLAG_RSSI, "RSSI"},
	{BSD_STEERING_POLICY_FLAG_VHT, "VHT STA"},
	{BSD_STEERING_POLICY_FLAG_NON_VHT, "NON VHT STA"},
	{BSD_STEERING_POLICY_FLAG_NEXT_RF, "check next RF"},
	{BSD_STEERING_POLICY_FLAG_PHYRATE, "phyrate"},
	{BSD_STEERING_POLICY_FLAG_LOAD_BAL, "load balance"},
	{BSD_STEERING_POLICY_FLAG_STA_NUM_BAL, "sta num balance"},
	{BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB, "over subscription"},
};

/* note: only return the first reason string */
const char *bsd_get_reason_name(uint bsd_reason)
{
	const char *reason_str = NULL;
	uint idx;

	for (idx = 0; idx < (uint)ARRAYSIZE(bsd_reason_names); idx++) {
		if ((bsd_reason_names[idx].reason & bsd_reason) == bsd_reason_names[idx].reason) {
			reason_str = bsd_reason_names[idx].name;
			break;
		}
	}

	return ((reason_str) ? reason_str : "Unknown Reason");
}

/* show sta info summary by "bsd -s" */
void bsd_dump_sta_info(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	bsd_sta_info_t *assoclist;
	bool bounce, picky, psta, dwds, dualband;
	FILE *out;

	if ((out = fopen(BSD_OUTPUT_FILE_STA_TMP, "w")) == NULL) {
		printf("Err: Open sta file.\n");
		return;
	}

	fprintf(out, "\n=== STA info summary ===\n");

	fprintf(out, "%-17s %-9s %-9s %-7s %-5s %-6s %-5s %-4s %-4s %-8s\n",
		"STA_MAC", "Interface", "TimeStamp", "Tx_rate", "RSSI",
		"Bounce", "Picky", "PSTA", "DWDS", "DUALBAND");

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if ((bssinfo->valid) && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
				/* assoclist */
				assoclist = bssinfo->assoclist;
				while (assoclist) {
					bounce = picky = psta = dwds = dualband = FALSE;

					/* bouncing */
					if (bsd_check_bouncing_sta(info, &assoclist->addr))
						bounce = TRUE;

					/* picky */
					if (bsd_check_picky_sta(info, &assoclist->addr))
						picky = TRUE;

					/* BRCM proxy sta */
					if (eacmp((&assoclist->paddr), &ether_null))
						psta = TRUE;

					/* BRCM DWDS sta */
					if ((assoclist->flags & WL_STA_DWDS_CAP) &&
						(assoclist->flags & WL_STA_DWDS))
						dwds = TRUE;

					/* dual band */
					if (bsd_is_sta_dualband(info, &assoclist->addr))
						dualband = TRUE;

					fprintf(out, ""MACF" %-9s %-9lu %-7d %-5d %-6s "
						"%-5s %-4s %-4s %-8s\n",
						ETHER_TO_MACF(assoclist->addr),
						bssinfo->ifnames,
						(unsigned long)(assoclist->timestamp),
						assoclist->mcs_phyrate,
						assoclist->rssi,
						bounce?"Yes":"No",
						picky?"Yes":"No",
						psta?"Yes":"No",
						dwds?"Yes":"No",
						dualband?"Yes":"No");
					assoclist = assoclist->next;
				}
			}
		}
	}

	fclose(out);
	if (rename(BSD_OUTPUT_FILE_STA_TMP, BSD_OUTPUT_FILE_STA) != 0) {
		perror("Err for sta data");
		unlink(BSD_OUTPUT_FILE_STA_TMP);
	}
}

/* for command: bsd -l */
void bsd_steering_record_display(void)
{
	int i, j, seq = 0;
	int start, end;
	FILE *out;

	if ((out = fopen(BSD_OUTPUT_FILE_LOG_TMP, "w")) == NULL) {
		printf("Err: Open log file.\n");
		return;
	}

	fprintf(out, "\n=== Band Steering Record ===\n");
	fprintf(out, "%3s %9s %-17s %-6s %-6s %-10s %-16s\n",
		"Seq", "TimeStamp", "STA_MAC", "Fm_ch", "To_ch", "Reason", "Description");

	if (bsd_steer_rec_wrapped) {
		start = bsd_steer_rec_idx;
		end = bsd_steer_rec_idx + BSD_MAX_STEER_REC;
	}
	else {
		start = 0;
		end = bsd_steer_rec_idx;
	}

	for (j = start; j < end; j++) {
		i = j % BSD_MAX_STEER_REC;
		fprintf(out, "%3d %9lu "MACF" 0x%4x 0x%4x 0x%08x %-16s\n",
			++seq,
			(unsigned long)(bsd_steer_records[i].timestamp),
			ETHER_TO_MACF(bsd_steer_records[i].addr),
			bsd_steer_records[i].from_chanspec,
			bsd_steer_records[i].to_chanspec,
			bsd_steer_records[i].reason,
			bsd_get_reason_name(bsd_steer_records[i].reason));
	}

	if (!seq) {
		fprintf(out, "<No BSD record>\n");
	}

	fclose(out);
	if (rename(BSD_OUTPUT_FILE_LOG_TMP, BSD_OUTPUT_FILE_LOG) != 0) {
		perror("Err for log data");
		unlink(BSD_OUTPUT_FILE_LOG_TMP);
	}
}

void bsd_update_steering_record(bsd_info_t *info, struct ether_addr *addr,
	bsd_bssinfo_t *bssinfo, bsd_bssinfo_t *to_bssinfo, uint32 reason)
{
	BSD_ENTER();

	BSD_STEER("steer_rec_idx: %d, to_bssinfo: %p\n", bsd_steer_rec_idx, to_bssinfo);

	bsd_steer_records[bsd_steer_rec_idx].timestamp = time(NULL);
	memcpy(&bsd_steer_records[bsd_steer_rec_idx].addr,
		(char *)addr, ETHER_ADDR_LEN);
	bsd_steer_records[bsd_steer_rec_idx].from_chanspec = bssinfo->chanspec;
	bsd_steer_records[bsd_steer_rec_idx].to_chanspec = to_bssinfo->chanspec;
	bsd_steer_records[bsd_steer_rec_idx].reason = reason;

	bsd_steer_rec_idx = (bsd_steer_rec_idx + 1) % BSD_MAX_STEER_REC;

	/* checking records wrapped */
	if (bsd_steer_rec_idx == 0) {
		bsd_steer_rec_wrapped = TRUE;
	}

	BSD_EXIT();
}

void bsd_log_rssi(bsd_info_t *info, bsd_sta_info_t *sta, bool reset)
{
	bsd_maclist_t *sta_mlist;
	int i;

	sta_mlist = bsd_prbsta_by_addr(info, &(sta->addr), FALSE);
	if (sta_mlist) {
		for (i = 0; i < BSD_MAX_INTF; i++) {
			if (reset == TRUE)
				sta_mlist->rssi_info[i].rssi = 0;

			if ((sta_mlist->rssi_info[i].rssi == 0) &&
				(i == sta->bssinfo->intf_info->idx)) {
				sta_mlist->rssi_info[i].timestamp = time(NULL);
				sta_mlist->rssi_info[i].rssi = sta->rssi;
				BSD_STEER("** %s: "MACF" i=%d, timestamp=%lu, rssi=%d\n",
					__FUNCTION__,  ETHERP_TO_MACF(&(sta->addr)),
					i, sta_mlist->rssi_info[i].timestamp, sta->rssi);
			}
		}
	}
}

/* Steer STA */
void bsd_steer_sta(bsd_info_t *info, bsd_sta_info_t *sta, bsd_bssinfo_t *to_bss)
{
	bsd_bssinfo_t *bssinfo;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *steer_bssinfo;
	bsd_maclist_t *sta_ptr;
	wl_wlif_bss_trans_data_t ioctl_data;
	bsd_steerlib_callback_data_t *callback_data = NULL;

	BSD_ENTER();

	BSD_STEER("Steering sta[%p], to_bss[%p]\n", sta, to_bss);

	if (sta == NULL) {
		/* no action */
		return;
	}

	bssinfo = sta->bssinfo;
	if (bssinfo == NULL) {
		/* no action */
		return;
	}

	intf_info = bssinfo->intf_info;

	/* get this interface's bssinfo */
	steer_bssinfo = (to_bss != NULL) ? to_bss : bssinfo->steer_bssinfo;

	/* adding STA to maclist and set mode to deny */
	/* deauth STA */

	/* avoid flip-flop if sta under steering from target interface */
	sta_ptr = bsd_maclist_by_addr(steer_bssinfo, &(sta->addr));
	if (sta_ptr && (sta_ptr->steer_state == BSD_STA_STEERING)) {
		BSD_STEER("Skip STA:"MACF" under steering\n", ETHER_TO_MACF(sta->addr));
		return;
	}

	/* Fill bsd callback data to steer lib. */
	callback_data = (bsd_steerlib_callback_data_t*)calloc(1, sizeof(*callback_data));
	if (!callback_data) {
		BSD_STEER("Calloc failed for steer lib callback data.\n");
		return;
	}

	callback_data->info = info;
	callback_data->steer_bssinfo = steer_bssinfo;
	callback_data->sta = sta;

	/* removing from steer-ed (target) intf maclist */
	bsd_remove_maclist(steer_bssinfo, &(sta->addr));
	wl_wlif_unblock_mac(steer_bssinfo->wlif_hdl, sta->addr, 0);

	/* Fill bss-trans action frame data and invoke bss-trans API */
	memset(&ioctl_data, 0, sizeof(ioctl_data));
	ioctl_data.rclass = steer_bssinfo->rclass;
	ioctl_data.chanspec = steer_bssinfo->chanspec;
	eacopy(&(steer_bssinfo->bssid), &(ioctl_data.bssid));
	eacopy(&(sta->addr), &(ioctl_data.addr));
	ioctl_data.timeout = (intf_info->band == BSD_BAND_5G) ?
		BSD_BLOCK_STA_TIMEOUT : info->block_sta_timeout;
	ioctl_data.bssid_info = steer_bssinfo->bssid_info;
	ioctl_data.phytype = steer_bssinfo->phytype;
	ioctl_data.resp_hndlr = bsd_process_bss_trans_resp;
	ioctl_data.resp_hndlr_data = callback_data;
	ioctl_data.flags |= WLIFU_BSS_TRANS_FLAGS_BTM_ABRIDGED;

	BSD_STEER("BSD Sending BTM REQ for Steering sta:"MACF" from "
		"%s[%d][%d]["MACF"[ to %s[%d][%d]["MACF"] RClass[%d]"
		"Chanspec[0x%x] Timeout[%d] BSSID_Info[0x%x] PHY_Type[0x%x] BTM_flags[0x%x]\n",
		ETHER_TO_MACF(sta->addr), bssinfo->prefix, intf_info->idx, bssinfo->idx,
		ETHER_TO_MACF(bssinfo->bssid),
		bssinfo->steer_prefix, (steer_bssinfo->intf_info)->idx, steer_bssinfo->idx,
		ETHER_TO_MACF(steer_bssinfo->bssid), steer_bssinfo->rclass,
		steer_bssinfo->chanspec, ioctl_data.timeout,
		steer_bssinfo->bssid_info, steer_bssinfo->phytype, ioctl_data.flags);

	(void)wl_wlif_do_bss_trans(sta->bssinfo->wlif_hdl, &ioctl_data, info->event_fd2, NULL);

	BSD_EXIT();
	return;
}

/* update if chan busy detection, and return current chan state */
bsd_chan_state_t  bsd_update_chan_state(bsd_info_t *info,
	bsd_intf_info_t *intf_info, bsd_bssinfo_t *bssinfo)
{
	uint8 idx, cnt, num;

	bsd_steering_policy_t *steering_cfg;
	bsd_if_qualify_policy_t *qualify_cfg;
	bsd_chanim_stats_t *rec;
	int min, max;
	int txop, bw_util; /* percent */
	uint8 over, under;
	int bw_util_only = 0; /* calculate bw_util for appevent (radio stats) */

	BSD_CCAENTER();

	steering_cfg = &intf_info->steering_cfg;
	qualify_cfg = &intf_info->qualify_cfg;
	cnt = steering_cfg->cnt;
	max = steering_cfg->chan_busy_max;

	min = qualify_cfg->min_bw;

	/* no over subscription check */
	if (!min || !max || !cnt) {
		BSD_CCAEXIT();
		intf_info->state = BSD_CHAN_NO_STATS;
		if (cnt)
			bw_util_only = 1;
		else
			return intf_info->state;
	}

	/* reset to BSD_CHAN_BUSY_UNKNOWN for default no op */
	intf_info->state = BSD_CHAN_BUSY_UNKNOWN;

	idx = MODSUB(intf_info->chan_util.idx, cnt, BSD_CHANIM_STATS_MAX);

	/* detect over/under */
	over = under = 0;
	if (cnt)
		intf_info->bw_util = 0;
	if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t *statsv2;
		for (num = 0; num < cnt; num++) {
			rec = &(intf_info->chan_util.rec[idx]);
			statsv2 = (chanim_stats_v2_t *)&rec->stats;

			txop = statsv2->ccastats[CCASTATS_TXOP];
			txop = (txop > 100) ? 100 : txop;
			bw_util = 100 - txop;
			intf_info->bw_util += bw_util;

			BSD_CCA("cca idx[%d] idle:%d[v:%d] bw_util:%d\n",
				idx, txop, rec->valid, bw_util);
			if (!bw_util_only) {
				if (rec->valid && (bw_util > max))
					over++;

				if (rec->valid && (bw_util < min))
					under++;
			}
			idx = MODINC(idx, BSD_CHANIM_STATS_MAX);
		}
	}
	else if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t *statsv3;
		for (num = 0; num < cnt; num++) {
			rec = &(intf_info->chan_util.rec[idx]);
			statsv3 = (chanim_stats_t *)&rec->stats;

			txop = statsv3->ccastats[CCASTATS_TXOP];
			txop = (txop > 100) ? 100 : txop;
			bw_util = 100 - txop;
			intf_info->bw_util += bw_util;

			BSD_CCA("cca idx[%d] idle:%d[v:%d] bw_util:%d\n",
				idx, txop, rec->valid, bw_util);
			if (!bw_util_only) {
				if (rec->valid && (bw_util > max))
					over++;

				if (rec->valid && (bw_util < min))
					under++;
			}
			idx = MODINC(idx, BSD_CHANIM_STATS_MAX);
		}
	}

	if (cnt)
		intf_info->bw_util = intf_info->bw_util/cnt;

	if (bw_util_only) {
		/* use original state */
		intf_info->state = BSD_CHAN_NO_STATS;
		return intf_info->state;
	}

	BSD_CCA("ifname:%s[remote:%d] over:%d under:%d min:%d max:%d cnt:%d\n",
		bssinfo->ifnames, intf_info->remote, over, under, min, max, cnt);

	if (over >= cnt)
		intf_info->state = BSD_CHAN_BUSY;

	if (under >= cnt)
		intf_info->state = BSD_CHAN_IDLE;

	BSD_CCA("intf_info state:%d\n", intf_info->state);

	BSD_CCAEXIT();
	return intf_info->state;
}

bool bsd_check_if_oversub(bsd_info_t *info, bsd_intf_info_t *intf_info)
{
	bool ret = FALSE;

	bsd_bssinfo_t *bssinfo;
	int bssidx;
	bsd_sta_info_t *sta = NULL;
	uint8 at_ratio = 0, at_ratio_lowest_phyrate = 0, at_ratio_highest_phyrate = 0;
	uint32 min_phyrate = (uint32) -1, max_phyrate = 0, delta_phyrate = 0;
	uint8	assoc = 0;

	BSD_ATENTER();

	for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
		if (!(bssinfo->valid) && !BSD_BSS_BSD_ENABLED(bssinfo))
			continue;

		BSD_AT("bssidx=%d intf:%s\n", bssidx, bssinfo->ifnames);

		sta = bssinfo->assoclist;

		while (sta) {
			BSD_AT("sta[%p]:"MACF" steer_flag=%d at_ratio=%d phyrate=%d\n",
				sta, ETHER_TO_MACF(sta->addr),
				sta->steerflag, sta->at_ratio, sta->phyrate);
			if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
				at_ratio += sta->at_ratio;
			}
			else {
				assoc++;
				/* calc data STA phyrate and at_ratio */
				if ((sta->phyrate < min_phyrate) &&
					(sta->at_ratio > info->slowest_at_ratio)) {
					min_phyrate = sta->phyrate;
					at_ratio_lowest_phyrate = sta->at_ratio;

					BSD_AT("lowest[phyrate:%d at_ratio:%d]\n",
						min_phyrate, at_ratio_lowest_phyrate);
				}

				if (sta->phyrate > max_phyrate) {
					max_phyrate = sta->phyrate;
					at_ratio_highest_phyrate = sta->at_ratio;

					BSD_AT("highest[phyrate:%d at_ratio:%d]\n",
						max_phyrate, at_ratio_highest_phyrate);
				}
			}

			sta = sta->next;
		}
		BSD_AT("ifnames:%s Video at_ratio=%d\n", bssinfo->ifnames, at_ratio);
		BSD_AT("lowest[phyrate:%d at_ratio:%d] highest[phyrate:%d "
			"at_ratio:%d]\n", min_phyrate, at_ratio_lowest_phyrate,
			max_phyrate, at_ratio_highest_phyrate);
	}

	/* algo 1: This algo is to check when Video takes most of airtime.
	 * v/(v+d) threshold. video_at_ratio[n] is threshold for n+1 data-stas
	 * n data-sta actively assoc, v/(v+d) > video_at_ratio[n]. steer
	 */
	if (assoc >= BSD_MAX_AT_SCB)
		assoc = BSD_MAX_AT_SCB;

	if (assoc < 1) {
		BSD_AT("No data sta. No steer\n");
		BSD_ATEXIT();
		return FALSE;
	}

	assoc--;

	if (at_ratio > info->video_at_ratio[assoc])
		ret = TRUE;

	/* Algo 2: This algo is to check for all data sta case
	 * for all data-STA, if delta(phyrate) > phyrate_delat
	 * && at_time(lowest phyrate sta) > at_rati: steer
	 * slowest data-sta airtime ratio
	 */
	delta_phyrate = 0;
	if (min_phyrate < max_phyrate) {
		delta_phyrate = max_phyrate - min_phyrate;
		BSD_AT("delta_phyrate[%d\n", delta_phyrate);
	}
	if ((delta_phyrate > info->phyrate_delta) &&
		at_ratio_lowest_phyrate > info->slowest_at_ratio)
		ret = TRUE;

	BSD_AT("ret:%d assoc:%d at_ratio:%d[%d] delta_phyrate:%d[%d] "
		"at_ratio_slowest_phyrate:%d[%d]\n",
		ret, assoc, at_ratio, info->video_at_ratio[assoc],
		delta_phyrate, info->phyrate_delta,
		at_ratio_lowest_phyrate, info->slowest_at_ratio);

	BSD_ATEXIT();
	return ret;
}

bool bsd_check_oversub(bsd_info_t *info)
{
	int idx;
	bsd_intf_info_t *intf_info;

	BSD_ATENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		BSD_AT("idx=%d, band=%d\n", idx, intf_info->band);

		if (!(intf_info->band & BSD_BAND_5G))
			continue;

		return bsd_check_if_oversub(info, intf_info);
	}

	BSD_ATENTER();

	return FALSE;
}

/* retrieve chann busy state */
bsd_chan_state_t bsd_get_chan_busy_state(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;

	BSD_CCAENTER();
	intf_info = &(info->intf_info[info->ifidx]);

	BSD_CCA("state:%d\n", intf_info->state);

	BSD_CCAEXIT();
	return intf_info->state;
}

/* ioctl callback handler for bsd. */
int
bsd_ioctl_callback_func(char *ifname, int cmd, void *buf, int len, void *data)
{
	return bsd_do_wl_ioctl(ifname, cmd, buf, len, *((uint8*)data));
}

/* Function to process the bss-trans response. */
static int
bsd_process_bss_trans_resp(void *data, int response)
{
	int ret = 1, intfidx, bssidx, wbd_ret = 0;
	bsd_steerlib_callback_data_t *callback_data = NULL;
	bsd_bssinfo_t *bssinfo, *tmp_bssinfo;
	bsd_intf_info_t *intf_info, *tmp_intf_info;
	bsd_info_t *info = NULL;
	bsd_sta_info_t *sta = NULL;

	BCM_REFERENCE(wbd_ret);

	if (data == NULL) {
		goto end;
	}

	callback_data = (bsd_steerlib_callback_data_t*)data;
	bssinfo = callback_data->sta->bssinfo;
	intf_info = bssinfo->intf_info;
	info = callback_data->info;
	sta = callback_data->sta;

	/* Each steering attempt to be added in bounce table
	 * irrespective of steering result.
	*/
	bsd_add_sta_to_bounce_table(info, &(sta->addr));

	if (response == BSD_BSS_RESPONSE_REJECT) {
		sta->steerflag |= BSD_STA_BSS_REJECT;
		/* Unblock mac on current interface */
		wl_wlif_unblock_mac(bssinfo->wlif_hdl, sta->addr, 0);
		BSD_STEER("Skip STA:"MACF" reject BSSID\n",
			ETHER_TO_MACF(sta->addr));
		goto end;
	}

	if (response == BSD_BSS_RESPONSE_UNKNOWN) {
		if (bssinfo->intf_info->band != callback_data->steer_bssinfo->intf_info->band) {
			if (!bsd_is_sta_dualband(info, &sta->addr)) {
				/* Unblock mac on current interface */
				wl_wlif_unblock_mac(bssinfo->wlif_hdl, sta->addr, 0);
				BSD_STEER("Skip STA "MACF" no response for single-band sta\n",
					ETHER_TO_MACF(sta->addr));
				goto end;
			}
		} else if (!bsd_qualify_sta_rf(info, bssinfo, sta->to_bssinfo, &sta->addr)) {
			/* Unblock mac on current interface */
			wl_wlif_unblock_mac(bssinfo->wlif_hdl, sta->addr, 0);
			BSD_STEER("Skip STA "MACF" no response for not-at-target sta\n",
				ETHER_TO_MACF(sta->addr));
			goto end;
		}

		if (info->steer_no_deauth) {
			/* Unblock mac on current interface */
			wl_wlif_unblock_mac(bssinfo->wlif_hdl, sta->addr, 0);
			/* Update sta stats to record no bss transition response */
			bsd_update_sta_stats(info, &(sta->addr), bssinfo->ifnames,
				callback_data->steer_bssinfo->ifnames, 0, sta->mcs_phyrate,
				sta->at_ratio, sta->rssi, intf_info->steering_flags,
				BSD_STA_BSSTRANS_UNSUPPORTED);
			BSD_STEER("NOT steer STA "MACF" from %s by steer_no_deauth\n",
				ETHER_TO_MACF(sta->addr), bssinfo->ifnames);
			goto end;
		}
	}

	/* adding to maclist */
	for (intfidx = 0; intfidx < info->max_ifnum; intfidx++) {
		/* search bands other than steer-ed intf */
		if (intfidx != callback_data->steer_bssinfo->intf_info->idx) {
			tmp_intf_info = &(info->intf_info[intfidx]);

			for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
				tmp_bssinfo = &tmp_intf_info->bsd_bssinfo[bssidx];
				if (!(tmp_bssinfo->valid) ||
					(tmp_bssinfo->steerflag & BSD_BSSCFG_NOTSTEER) ||
					!BSD_BSS_BSD_ENABLED(bssinfo)) {
					continue;
				}

				if (!strcmp(tmp_bssinfo->ssid,
					callback_data->steer_bssinfo->ssid)) {
					int timeout = (intf_info->band == BSD_BAND_5G) ?
						BSD_BLOCK_STA_TIMEOUT : info->block_sta_timeout;
					if (tmp_bssinfo == bssinfo) {
						bsd_addto_maclist(tmp_bssinfo, &(sta->addr),
							callback_data->steer_bssinfo);
					} else {
						bsd_addto_maclist(tmp_bssinfo, &(sta->addr), NULL);
					}

					/* block the sta at current intf.  */
					wl_wlif_block_mac(tmp_bssinfo->wlif_hdl, sta->addr, timeout);
				}
			}
		}
	}

#ifdef BCM_WBD
	/* If wbd is enabled block the sta in all slave nodes except tbss. */
	if (info->enable_flag & BSD_FLAG_WBD_ENABLED) {
		bsd_wbd_send_block_client_req(info, callback_data->steer_bssinfo, sta, &wbd_ret);
	}
#endif /* BCM_WBD */

	/* remember the rssi at this steering point */
	bsd_log_rssi(info, sta, TRUE);

	bsd_update_steering_record(info, &(sta->addr), bssinfo, callback_data->steer_bssinfo,
		intf_info->steering_flags);

	/* Set ret to 0 so that steer lib can send deauth */
	if (response == BSD_BSS_RESPONSE_UNKNOWN) {
		ret = 0;
	}

	BSD_APPEVENT("Update stats for STA "MACF" from %s to %s "
		"phrate %d, at_ratio %d, rssi %d, steering_flags=0x%x\n",
		ETHER_TO_MACF(sta->addr), bssinfo->ifnames, callback_data->steer_bssinfo->ifnames,
		sta->mcs_phyrate, sta->at_ratio, sta->rssi, intf_info->steering_flags);

	bsd_update_sta_stats(info, &(sta->addr), bssinfo->ifnames,
		callback_data->steer_bssinfo->ifnames, 0, sta->mcs_phyrate, sta->at_ratio,
		sta->rssi, intf_info->steering_flags, sta->steerflag);
	bsd_steer_appevent(info, &(sta->addr), APP_E_BSD_STEER_START, APP_E_BSD_STATUS_STEER_START);

	bsd_remove_sta_reason(info, bssinfo->ifnames, bssinfo->intf_info->remote,
		&(sta->addr), BSD_STA_STEERED);

	/* reset cca stats */
	bsd_reset_chan_busy(info, intf_info->idx);

end:
	/* Free call back data. */
	if (callback_data) {
		free(callback_data);
		callback_data = NULL;
	}

	return ret;
}
