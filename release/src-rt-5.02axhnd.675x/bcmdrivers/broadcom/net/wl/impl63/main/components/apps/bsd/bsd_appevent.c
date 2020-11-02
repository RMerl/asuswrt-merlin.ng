/*
 * Wireless Application Event Service (Linux)
 * bsd send STA/Radio stats event to appeventd
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
 * $Id: bsd.c $
 */

#include "bsd.h"

/*
 * function routines for bsd - appeventd
 */

/* use bouncing table to log sta stats */
app_event_bsd_sta_t *
bsd_update_sta_stats(bsd_info_t *info, struct ether_addr *addr,
	char *src_ifname,
	char *dst_ifname,
	int steer_fail,
	uint32 tx_rate,
	uint8 at_ratio,
	int rssi,
	uint32 intf_steer_flags,
	uint8 sta_steer_response)
{
	int hash_key;
	bsd_sta_bounce_detect_t *entry;

	hash_key = BSD_BOUNCE_MAC_HASH(*addr);
	entry = info->sta_bounce_table[hash_key];

	while (entry) {
		if (eacmp(&(entry->addr), addr) == 0) {
			break;
		}
		entry = entry->next;
	}

	if (!entry) {
		BSD_APPEVENT("stats entry not found for sta "MACF"\n", ETHERP_TO_MACF(addr));
		return NULL;
	}

	if (src_ifname) {
		strncpy(entry->stats.src_ifname, src_ifname, IFNAMSIZ);
		entry->stats.src_ifname[IFNAMSIZ-1] = '\0';
	}
	if (dst_ifname) {
		strncpy(entry->stats.dst_ifname, dst_ifname, IFNAMSIZ);
		entry->stats.dst_ifname[IFNAMSIZ-1] = '\0';
	}
	if (steer_fail)
		entry->stats.steer_fail_cnt++;
	if (tx_rate)
		entry->stats.tx_rate = tx_rate;
	if (at_ratio)
		entry->stats.at_ratio = at_ratio;
	if (rssi)
		entry->stats.rssi = rssi;
	if (intf_steer_flags)
		entry->stats.steer_flags = intf_steer_flags;
	if (sta_steer_response & BSD_STA_BSSTRANS_UNSUPPORTED) {
		entry->non11v_run.cnt++;
	} else if (src_ifname && dst_ifname) {
		/* Reset the count to 0 if we get response for steer request.
		 * Above if condition is needed since this API can be called
		 * with all 0/NULL parameters to get the stats.
		*/
		entry->non11v_run.cnt = 0;
	}
	entry->stats.steer_cnt = entry->run.cnt;

	BSD_APPEVENT("update stats for sta "MACF" src=%s, dst=%s, "
		"steer_fail=%d,tx_rate=%d, at_ratio=%d, rssi=%d, intf_steer_flags=0x%x"
		" no_steer_resp_cnt=%d\n",
		ETHERP_TO_MACF(addr), src_ifname, dst_ifname,
		steer_fail, tx_rate, at_ratio, rssi, intf_steer_flags,
		entry->non11v_run.cnt);

	return &(entry->stats);
}

/* return length, one sta, from bouncing table */
int
bsd_appevent_sta_stats(bsd_info_t *info, struct ether_addr *addr, char *pkt)
{
	app_event_bsd_sta_t *sta_stats;

	if (pkt == NULL)
		return 0;

	/* all NULL/0 means search only */
	sta_stats = bsd_update_sta_stats(info, addr, NULL, NULL, 0, 0, 0, 0, 0, 0);
	if (sta_stats == NULL)
		return 0;

	memcpy(pkt, sta_stats, sizeof(app_event_bsd_sta_t));
	memcpy(pkt, addr, ETHER_ADDR_LEN);

	return sizeof(app_event_bsd_sta_t);
}

/* return length, for all participated radios */
int
bsd_appevent_radio_stats(bsd_info_t *info, char *pkt)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	app_event_bsd_radio_t *radio_stats;
	int len = 0;

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		if (intf_info->enabled != TRUE) {
			continue;
		}

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (bssinfo->valid && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
				radio_stats = (app_event_bsd_radio_t *)pkt;
				strncpy(radio_stats->ifname, bssinfo->ifnames, IFNAMSIZ);
				radio_stats->ifname[IFNAMSIZ-1] = '\0';
				radio_stats->bw_util = intf_info->bw_util;
				radio_stats->throughput = bssinfo->throughput;
				pkt += sizeof(app_event_bsd_radio_t);
				len += sizeof(app_event_bsd_radio_t);
			}
		}
	}

	return len;
}

void
bsd_steer_appevent(bsd_info_t *info, struct ether_addr *addr, int eventid, int status)
{
	char app_data[BSD_APPEVENT_BUFSIZE], *pkt = app_data;
	int len, total_len;

	/* the sta's stats first */
	len = bsd_appevent_sta_stats(info, addr, pkt);
	total_len = len;
	pkt += len;

	if ((len + info->max_ifnum * sizeof(app_event_bsd_radio_t)) > BSD_APPEVENT_BUFSIZE) {
		BSD_ERROR("len (%d) for sta+radio (%d) is too long\n", len, info->max_ifnum);
		return;
	}
	/* append radio's stats */
	total_len += bsd_appevent_radio_stats(info, pkt);

	BSD_APPEVENT("len %d, total_len %d\n", len, total_len);

	app_event_sendup(eventid, status, (unsigned char *)app_data, total_len);
}

/* return length, one sta, from assoclist */
int
bsd_appevent_sta_stats2(bsd_sta_info_t *sta, char *pkt)
{
	app_event_bsd_sta_t *sta_stats;

	if (sta == NULL || pkt == NULL)
		return 0;

	sta_stats = (app_event_bsd_sta_t *)pkt;

	memcpy(&(sta_stats->addr), &(sta->addr), ETHER_ADDR_LEN);
	strncpy(sta_stats->src_ifname, sta->bssinfo->ifnames, IFNAMSIZ);
	sta_stats->src_ifname[IFNAMSIZ-1] = '\0';
	sta_stats->dst_ifname[0] = 0;

	sta_stats->steer_cnt = 0; /* never steered */
	sta_stats->steer_fail_cnt = 0;
	sta_stats->tx_rate = sta->mcs_phyrate;
	sta_stats->at_ratio = sta->at_ratio;
	sta_stats->rssi = sta->rssi;
	sta_stats->steer_flags = 0;

	return sizeof(app_event_bsd_sta_t);
}

/* use cli bsd -S [MAC] to query sta stats */
void
bsd_query_sta_stats(bsd_info_t *info)
{
	char nvram_str[32] = "";
	struct ether_addr ea;
	char app_data[BSD_APPEVENT_BUFSIZE], *pkt = app_data;
	int len = 0, tlen = 0; /* total len */
	bsd_sta_info_t *sta = NULL;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	int ulen = sizeof(app_event_bsd_sta_t);
	FILE* fp;

	if ((fp = fopen(BSD_APPEVENT_STA_MAC, "r")) == NULL) {
		BSD_ERROR("Err: failed to open sta mac file!\n");
		return;
	}

	fgets(nvram_str, sizeof(nvram_str), fp);
	fclose(fp);

	BSD_APPEVENT("sta: %s len=%zd\n", nvram_str, strlen(nvram_str));

	if (ether_atoe(nvram_str, (unsigned char *)(&(ea.octet)))) {
		BSD_APPEVENT("Poll stats for sta "MACF" \n", ETHERP_TO_MACF(&ea));
		/* a single specified STA, try bouncing table first */
		if ((tlen = bsd_appevent_sta_stats(info, &ea, pkt)) == 0) {
			/* if not exist in bounce table, try assoclist */
			for (idx = 0; idx < info->max_ifnum; idx++) {
				intf_info = &(info->intf_info[idx]);
				for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
					bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
					if (bssinfo->valid && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
						/* assoclist */
						if ((sta = bsd_add_assoclist(bssinfo, &ea, FALSE))
							!= NULL) {
							break;
						}
					}
				}
				if (sta)
					break;
			}

			if (sta) {
				tlen = bsd_appevent_sta_stats2(sta, pkt);
			}
			else {
				BSD_APPEVENT("Poll sta "MACF" not exist\n", ETHERP_TO_MACF(&ea));
			}
		}
	}
	else {
		if (strlen(nvram_str) != 0) {
			BSD_APPEVENT("illegal MAC: %s\n", nvram_str);
			return;
		}

		/* STA MAC is not specified, return all */
		for (idx = 0; idx < info->max_ifnum; idx++) {
			intf_info = &(info->intf_info[idx]);
			for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
				bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
				if (bssinfo->valid && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
					/* assoclist */
					sta = bssinfo->assoclist;
					while (sta && ((tlen + ulen) <= BSD_APPEVENT_BUFSIZE)) {
						len = bsd_appevent_sta_stats(info,
							&(sta->addr), pkt);
						if (len == 0) {
							len = bsd_appevent_sta_stats2(sta, pkt);
						}
						tlen += len;
						pkt += len;
						sta = sta->next;
					}
				}
			}
		}
	}

	if (tlen) {
		app_event_sendup(APP_E_BSD_STATS_QUERY, APP_E_BSD_STATUS_QUERY_STA,
			(unsigned char *)app_data, tlen);
	}
}

/* use cli "bsd -r" to query radio stats */
void
bsd_query_radio_stats(bsd_info_t *info)
{
	char app_data[BSD_APPEVENT_BUFSIZE], *pkt = app_data;
	int total_len;

	BSD_APPEVENT("in bsd_poll_radio_stats\n");

	/* send radio stats for all participated radio interface */
	total_len = bsd_appevent_radio_stats(info, pkt);
	app_event_sendup(APP_E_BSD_STATS_QUERY, APP_E_BSD_STATUS_QUERY_RADIO,
		(unsigned char *)app_data, total_len);
}
