/*
 * bsd scheme  (Linux)
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
 * $Id: bsd_engine.c $
 */
#include "bsd.h"

static bsd_sta_select_policy_t predefined_policy[] = {
/* idle_rate rssi phyrate wprio wrssi wphy_rate wtx_failures wtx_rate wrx_rate flags */
/* 0: low rssi rssi BSD_POLICY_LOW_RSSI */
{0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
/* 1: high rssi rssi BSD_POLICY_HIGH_RSSI */
{0, 0, 0, 0, -1, 0, 0, 0, 0, 0},
/* 2: low phyrate BSD_POLICY_LOW_PHYRATE */
{0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
/* 3: high phyrate rssi BSD_POLICY_HIGH_PHYRATE */
{0, 0,	-75, 0, 0, -1, 0, 0, 0, 0},
/* 4: tx_failures */
{0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
/* 5: tx/rx rate */
{0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
/* End */
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

typedef bsd_sta_info_t * (*bsd_algo_t)(bsd_info_t *info, int ifidx, int to_ifidx);

/* RF Band Steering STA Selection Algorithm, should be only one and controlled by config */
static bsd_algo_t predefined_algo[] = {
	bsd_sta_select_5g,
	bsd_sta_select_policy,
	NULL
};

typedef void (*bsd_scheme_t)(bsd_info_t *info);

/* RF Band Steering Algorithm, should be only one and controlled by config */
static bsd_scheme_t predefined_scheme[] = {
	bsd_steer_scheme_5g,
	bsd_steer_scheme_balance,
	bsd_steer_scheme_policy,
	NULL
};

#define BSD_MAX_POLICY (sizeof(predefined_policy)/sizeof(bsd_sta_select_policy_t) - 1)
#define BSD_MAX_ALGO (sizeof(predefined_algo)/sizeof(bsd_algo_t) - 1)
#define BSD_MAX_SCHEME (sizeof(predefined_scheme)/sizeof(bsd_scheme_t) - 1)

#define BSD_INIT_SCORE_MAX	0x7FFFFFFF
#define BSD_INIT_SCORE_MIN	0

char ioctl_buf[BSD_IOCTL_MAXLEN];
char ret_buf[BSD_IOCTL_MAXLEN];
char cmd_buf[BSD_IOCTL_MAXLEN];
char maclist_buf[BSD_IOCTL_MAXLEN];

int bsd_get_max_policy(bsd_info_t *info)
{
	UNUSED_PARAMETER(info);
	return BSD_MAX_POLICY;
}

int bsd_get_max_algo(bsd_info_t *info)
{
	UNUSED_PARAMETER(info);
	return BSD_MAX_ALGO;
}

int bsd_get_max_scheme(bsd_info_t *info)
{
	UNUSED_PARAMETER(info);
	return BSD_MAX_SCHEME;
}

bsd_sta_select_policy_t *bsd_get_sta_select_cfg(bsd_bssinfo_t *bssinfo)
{
	return &predefined_policy[bssinfo->policy];
}

void bsd_check_steer(bsd_info_t *info)
{
	(predefined_scheme[info->scheme])(info);

	return;
}

/* select victim STA */
bsd_sta_info_t *bsd_select_sta(bsd_info_t *info)
{
	bsd_sta_info_t *sta = NULL;
	bsd_bssinfo_t *bssinfo;
	bsd_intf_info_t *intf_info;

	BSD_ENTER();

	if (info->over) {
		intf_info = &(info->intf_info[info->ifidx]);
		bssinfo = &(intf_info->bsd_bssinfo[info->bssidx]);
		if (info->over == BSD_CHAN_BUSY) { /* 5G over */
			BSD_INFO("Steer from %s: [%d][%d]\n",
				bssinfo->ifnames, info->ifidx, info->bssidx);
		}
		else { 	/* 5G under */
			bssinfo = bssinfo->steer_bssinfo;
			BSD_INFO("Steer from %s: [%d][%d]\n", bssinfo->ifnames,
				(bssinfo->intf_info)->idx, bssinfo->idx);
		}

		sta = (predefined_algo[bssinfo->algo])(info, 0, 0);
		/* bsd_sort_sta(info); */
	}

	BSD_EXIT();
	return sta;
}

/* Steer scheme: Ony based on 5G channel utilization */
void bsd_steer_scheme_5g(bsd_info_t *info)
{
	bsd_sta_info_t *sta;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	char tmp[100], *str, *endptr = NULL;
	bool flag = FALSE;

	BSD_ENTER();

	if (BSD_DUMP_ENAB) {
		BSD_PRINT("\nBefore steer Check: dump dbg info========= \n");
		bsd_dump_info(info);
		BSD_PRINT("\n============================= \n");
	}

	intf_info = &(info->intf_info[info->ifidx]);
	bssinfo = &(intf_info->bsd_bssinfo[info->bssidx]);

	info->over = (uint8)bsd_update_chan_state(info, intf_info, bssinfo);

	str = nvram_get(strcat_r(bssinfo->prefix, "bsd_over", tmp));
	if (str) {
		info->over = (uint8)strtoul(str, &endptr, 0);
		nvram_unset(strcat_r(bssinfo->prefix, "bsd_over", tmp));
	}

	BSD_STEER("======over[0x%x:%d]=========\n",
		info->over, info->over&(~(BSD_CHAN_STEER_MASK)));

	flag = bsd_check_oversub(info);

	BSD_STEER("bsd_check_oversub return %d\n", flag);
	BSD_STEER("bsd mode:%d. actframe:%d \n", info->mode, !nvram_match("bsd_actframe", "0"));

	if (info->mode == BSD_MODE_STEER) {
		if ((info->over == BSD_CHAN_IDLE) ||
			((info->over == BSD_CHAN_BUSY) && flag) ||
			(info->over & BSD_CHAN_STEER_MASK)) {
			info->over &= ~(BSD_CHAN_STEER_MASK);
			sta = bsd_select_sta(info);
			if (sta) {
				bssinfo = sta->bssinfo;
				bsd_steer_sta(info, sta, bssinfo->steer_bssinfo);
			}
			else
				BSD_STEER("No data STA steer to/from [%s]\n", bssinfo->ifnames);

			/* reset cca stats */
			bsd_reset_chan_busy(info, info->ifidx);
		}
	}

	if (BSD_DUMP_ENAB) {
		BSD_PRINT("\nAfter Steer Check: dump dbg info========= \n");
		bsd_dump_info(info);
		BSD_PRINT("\n============================= \n");
	}
	BSD_EXIT();
	return;
}

/* Default 5G oversubscription STA selction algo */
bsd_sta_info_t *bsd_sta_select_5g(bsd_info_t *info, int ifidx, int to_ifidx)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo, *steer_bssinfo;
	int idx, bssidx;
	bsd_sta_info_t *sta = NULL, *victim = NULL;
	uint score = (uint)(-1);
	int score_idx = -1, score_bssidx = -1;
	time_t now = time(NULL);
	bool idle = FALSE;

	BSD_ENTER();

	UNUSED_PARAMETER(idle);

	if(info->over == BSD_CHAN_BUSY) { /* 5G over */
		score_idx = info->ifidx;
		score_bssidx = info->bssidx;

		for (idx = 0; idx < info->max_ifnum; idx++) {
			BSD_STEER("idx[%d]\n", idx);
			intf_info = &(info->intf_info[idx]);
			for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
				bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
				if (!(bssinfo->valid) ||
					!BSD_BSS_BSD_ENABLED(bssinfo))
					continue;

				idle |= bssinfo->video_idle;
				BSD_STEER("ifnames[%s] [%d[%d] idle=%d\n",
					bssinfo->ifnames, idx, bssidx, idle);
			}
		}
	}
	else { /* 5G under */
		intf_info = &(info->intf_info[info->ifidx]);
		bssinfo = &(intf_info->bsd_bssinfo[info->bssidx]);
		bssinfo = bssinfo->steer_bssinfo;
		score_idx = bssinfo->intf_info->idx;
		score_bssidx = bssinfo->idx;
	}

	BSD_STEER("over=%d score_idx=%d score_bssidx=%d\n", info->over, score_idx, score_bssidx);

	for (idx = 0; idx < info->max_ifnum; idx++) {
		BSD_INFO("idx[%d]\n", idx);
		intf_info = &(info->intf_info[idx]);
		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (!(bssinfo->valid) ||
				!BSD_BSS_BSD_ENABLED(bssinfo))
				continue;

			if ((idx != score_idx) || (bssidx != score_bssidx)) {
				BSD_INFO("skip bssinfo[%s] [%d]{%d]\n",
					bssinfo->ifnames, idx, bssidx);
				continue;
			}

			BSD_STEER("intf:%d bssidx[%d] ifname:%s\n", idx, bssidx, bssinfo->ifnames);

			/* assoclist */
			sta = bssinfo->assoclist;
			BSD_INFO("sta[%p]\n", sta);
			while (sta) {
				/* skipped single band STA */
				if (!bsd_is_sta_dualband(info, &sta->addr)) {
					BSD_STEER("sta[%p]:"MACF" is not dualand. Skipped.\n",
						sta, ETHERP_TO_MACF(&sta->addr));
					goto next;
				}

				/* skipped non-steerable STA */
				if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
					BSD_STEER("sta[%p]:"MACF" is not steerable. Skipped.\n",
						sta, ETHERP_TO_MACF(&sta->addr));
					goto next;
				}

				/* if steer_no_deauth is set and sta did not respond to
				 * max_steer_req_cnt then skip this sta for steering
				*/
				if (info->steer_no_deauth && info->non11v_cfg.cnt &&
					bsd_check_non11v_sta(info, &sta->addr)) {
					BSD_STEER("Skip sta[%p]="MACF" - Max steer req count"
					" reched \n",
					sta, ETHERP_TO_MACF(&sta->addr));
					goto next;
				}

				/* Skipped macmode mismatch STA */
				steer_bssinfo = bssinfo->steer_bssinfo;
				if (bsd_aclist_steerable(steer_bssinfo, &sta->addr) == BSD_FAIL) {
					BSD_STEER("sta[%p]:"MACF" not steerable match "
						"w/ static maclist. Skipped.\n",
						sta, ETHERP_TO_MACF(&sta->addr));
					goto next;
				}

				/* Skipped idle, or active STA */
				if (bssinfo->sta_select_cfg.idle_rate != 0) {
					uint32 rate = sta->tx_bps + sta->rx_bps;
					if (rate <= bssinfo->sta_select_cfg.idle_rate) {
						BSD_STEER("Skip idle STA:"MACF" idle_rate[%d]"
							"tx+rx_rate[%d: %d+%d]\n",
							ETHERP_TO_MACF(&sta->addr),
							bssinfo->sta_select_cfg.idle_rate,
							sta->tx_bps+sta->rx_bps,
							sta->tx_bps, sta->rx_bps);
						goto next;
					}
				}

				/* Skipped bouncing STA */
				if (bsd_check_bouncing_sta(info, &sta->addr)) {
					BSD_BOUNCE("Skip bouncing STA:"MACF", skip ..\n",
						ETHERP_TO_MACF(&sta->addr));
					goto next;
				} else {
					BSD_BOUNCE("None bouncing STA:"MACF", further ...\n",
						ETHERP_TO_MACF(&sta->addr));
				}

				/* Skipped low rssi STA */
				if (bssinfo->sta_select_cfg.rssi != 0) {
					int32 est_rssi = sta->rssi;
					est_rssi += DIV_QUO(steer_bssinfo->txpwr.txpwr[0], 4);
					est_rssi -= DIV_QUO(bssinfo->txpwr.txpwr[0], 4);

					/* customize low rssi check */
					if (est_rssi < bssinfo->sta_select_cfg.rssi) {
						BSD_STEER("Skip low rssi STA:"MACF" sta_rssi"
							"[%d (%d-(%d-%d))] <  thld[%d]\n",
							ETHERP_TO_MACF(&sta->addr),
							est_rssi, sta->rssi,
							DIV_QUO(bssinfo->txpwr.txpwr[0], 4),
							DIV_QUO(steer_bssinfo->txpwr.txpwr[0], 4),
							bssinfo->sta_select_cfg.rssi);
						goto next;
					}
				}

				sta->score = sta->prio * bssinfo->sta_select_cfg.wprio +
					sta->rssi * bssinfo->sta_select_cfg.wrssi+
					sta->phyrate * bssinfo->sta_select_cfg.wphy_rate +
					sta->tx_failures * bssinfo->sta_select_cfg.wtx_failures +
					sta->tx_rate * bssinfo->sta_select_cfg.wtx_rate +
					sta->rx_rate * bssinfo->sta_select_cfg.wrx_rate;

				BSD_STEER("sta[%p]:"MACF"Score[%d] prio[%d], rssi[%d] "
					"phyrate[%d] tx_failures[%d] tx_rate[%d] rx_rate[%d]\n",
					sta, ETHERP_TO_MACF(&sta->addr), sta->score,
					sta->prio, sta->rssi, sta->phyrate,
					sta->tx_failures, sta->tx_bps, sta->rx_bps);

				if (sta->score < score) {
					/* timestamp check to avoid flip'n'flop ? */
					BSD_STEER("found victim:"MACF" now[%lu]- timestamp[%lu]"
						"= %lu timeout[%d] \n",
						ETHERP_TO_MACF(&sta->addr), now,
						sta->timestamp, now - sta->timestamp,
						info->steer_timeout);

					if (now - sta->timestamp > info->steer_timeout)	{
						BSD_STEER("found victim:"MACF"\n",
							ETHERP_TO_MACF(&sta->addr));
						victim = sta;
						score = sta->score;
					}
				}
next:
				BSD_INFO("next[%p]\n", sta->next);
				sta = sta->next;
			}
		}
	}

	if (victim) {
		BSD_STEER("Victim sta[%p]:"MACF"Score[%d]\n",
			victim, ETHERP_TO_MACF(&victim->addr), victim->score);
	}

	if (idle) {
		BSD_STEER("idle=%d no victim\n", idle);
		return NULL;
	}

	BSD_EXIT();
	return victim;
}

/* Steer scheme: Balance 5G and 2.4G channel load */
void bsd_steer_scheme_balance(bsd_info_t *info)
{
	BSD_PRINT("***** Not implemented yet\n");
}

static int bsd_get_preferred_if(bsd_info_t *info, int ifidx)
{
	int bssidx;
	int to_ifidx;
	bsd_intf_info_t *intf_info, *to_intf_info;
	bsd_bssinfo_t *bssinfo;
	bsd_if_bssinfo_list_t *if_bss_list;
	bool found = FALSE;
	uint cnt = 0;
	int ret, val = 0;

	intf_info = &(info->intf_info[ifidx]);

	BSD_MULTI_RF("ifidx:%d\n", ifidx);

	bssidx = bsd_get_steerable_bss(info, intf_info);
	if (bssidx == -1) {
		return -1;
	}

	bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

	/* enumerate multiple ifs from this if bss list, and qualify idle RF */
	if_bss_list = bssinfo->to_if_bss_list;
	while (if_bss_list) {
		cnt++;
		to_ifidx = if_bss_list->to_ifidx;
		to_intf_info = &(info->intf_info[to_ifidx]);

		BSD_MULTI_RF("to_ifidx:%d to_intf_info:%p bsd_bssinfo[0]:%p\n",
			ifidx, to_intf_info, &to_intf_info->bsd_bssinfo[0]);

		/* skip if to_intf_info's primary is down */
		ret = bsd_wl_ioctl(&to_intf_info->bsd_bssinfo[0], WLC_GET_UP, &val, sizeof(val));
		if (ret < 0) {
			goto next;
		}

		BSD_MULTI_RF("check bssidx:%d to_ifidx:%d - state:0x%x target if %s\n",
			bssidx, to_ifidx, to_intf_info->state, val ? "Up" : "Down");

		if (val &&
			((to_intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_NEXT_RF) ||
			(to_intf_info->state == BSD_CHAN_IDLE) ||
			(to_intf_info->qualify_cfg.min_bw == 0))) {
			BSD_MULTI_RF("found to_ifidx=%d, chan under subscription\n", to_ifidx);
			found = TRUE;
			break;
		}

next:
		if_bss_list = if_bss_list->next;
	}

	if (found == FALSE) {
		to_ifidx = -1;
	}
	BSD_MULTI_RF("ifidx=%d, to_ifidx=%d of total %d \n", ifidx, to_ifidx, cnt);

	return to_ifidx;
}

static uint32 bsd_get_steering_mask(bsd_steering_policy_t *cfg)
{
	uint32 mask = 0;

	if (cfg->flags & BSD_STEERING_POLICY_FLAG_RULE) {
		/* set VHT, NON_VHT STA feature based bits */
		mask = cfg->flags &
			(BSD_STEERING_POLICY_FLAG_VHT | BSD_STEERING_POLICY_FLAG_NON_VHT);

		mask |= (cfg->rssi != 0) ? BSD_STEERING_POLICY_FLAG_RSSI : 0;
		mask |= (cfg->phyrate != 0) ? BSD_STEERING_POLICY_FLAG_PHYRATE : 0;
	}

	return (mask);
}

static uint32 bsd_get_sta_select_mask(bsd_sta_select_policy_t *cfg)
{
	uint32 mask = 0;

	if (cfg->flags & BSD_STA_SELECT_POLICY_FLAG_RULE) {
		/* set VHT, NON_VHT STA feature based bits */
		mask = cfg->flags &
			(BSD_STA_SELECT_POLICY_FLAG_VHT | BSD_STA_SELECT_POLICY_FLAG_NON_VHT);

		mask |= (cfg->rssi != 0) ? BSD_STA_SELECT_POLICY_FLAG_RSSI : 0;
		mask |= (cfg->phyrate != 0) ? BSD_STA_SELECT_POLICY_FLAG_PHYRATE : 0;
	}

	return (mask);
}

bool bsd_bcm_special_sta_check(bsd_info_t *info, bsd_sta_info_t *sta)
{
	/* BRCM proxy sta */
	if (eacmp((&sta->paddr), &ether_null)) {
		BSD_STEER("brcm proxy sta:"MACF"\n", ETHER_TO_MACF(sta->addr));
		return TRUE;
	}

	/* picky sta */
	if (bsd_check_picky_sta(info, &sta->addr)) {
		BSD_STEER("picky sta: "MACF"\n", ETHER_TO_MACF(sta->addr));
		return TRUE;
	}

	/* DWDS sta */
	if ((sta->flags & WL_STA_DWDS_CAP) && (sta->flags & WL_STA_DWDS)) {
		BSD_STEER("DWDS sta: "MACF"\n", ETHER_TO_MACF(sta->addr));
		return TRUE;
	}

	return FALSE;
}

static bool bsd_if_qualify_sta(bsd_info_t *info, int to_ifidx, bsd_sta_info_t *sta)
{
	bool qualify = FALSE;
	bsd_intf_info_t *intf_info;
	int to_bssidx;
	bsd_bssinfo_t *bssinfo, *to_bssinfo;
	uint32 steer_reason;
	int vht;
	int band_detect = 0;

	BSD_ATENTER();

	BSD_STEER("qualifying sta[%p] to to_ifidx:%d addr:"MACF", paddr:"MACF"\n",
		sta, to_ifidx,
		ETHER_TO_MACF(sta->addr), ETHER_TO_MACF(sta->paddr));

	if ((sta == NULL) || (to_ifidx == -1)) {
		return qualify;
	}

	bssinfo = sta->bssinfo;
	intf_info = &(info->intf_info[to_ifidx]);
	to_bssidx = bsd_get_steerable_bss(info, intf_info);

	if (to_bssidx == -1) {
		return -1;
	}

	BSD_STEER("to_ifidx:%d to_bssidx:%d\n",
		to_ifidx, to_bssidx);

	sta->to_bssinfo = NULL;

	to_bssinfo = &(intf_info->bsd_bssinfo[to_bssidx]);

	/*  sta to target validation check */
	if (bssinfo->intf_info->band != to_bssinfo->intf_info->band) {
		/* different band, e.g. 5G <-> 2G */
		if (!bsd_is_sta_dualband(info, &sta->addr)) {
			BSD_STEER("STA "MACF" not dualband\n", ETHER_TO_MACF(sta->addr));
			band_detect = 1;
		}
	}
	else if (!bsd_qualify_sta_rf(info, bssinfo, to_bssinfo, &sta->addr)) {
		/* same band, e.g. 5GH <-> 5GL */
		BSD_STEER("STA "MACF" no activity on target (ifidx:%d)\n",
			ETHER_TO_MACF(sta->addr), to_bssidx);
		band_detect = 1;
	}

	if (band_detect) {
		/* if band detect count exceed limit or
		 * STA is NOT 11v capable, then
		 * just skip this STA and disqualify
		 */
		if ((sta->band_detect_cnt > BSD_BAND_DETECT_MAX_TRY) ||
			!(sta->wnm_cap & WL_WNM_BSSTRANS)) {
			BSD_STEER("Skip STA "MACF" reason: mutli-band detect fail\n",
				ETHER_TO_MACF(sta->addr));
			return FALSE;
		} else {
			BSD_STEER("%s: STA "MACF" band detect count: %d\n",
				__FUNCTION__, ETHERP_TO_MACF(&sta->addr),
				sta->band_detect_cnt);
		}
	}

	if ((intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_NEXT_RF) ||
		(intf_info->state == BSD_CHAN_IDLE) ||
		(intf_info->state == BSD_CHAN_NO_STATS) ||
		(intf_info->qualify_cfg.min_bw == 0)) {
		BSD_MULTI_RF("intf qualifies, check further\n");
	} else {
		BSD_STEER("Skip sta[%p]:"MACF" %s chan bw util percent not qualified.\n",
			sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames);
		return FALSE;
	}

	/* check macmode mismatch STA */
	if (bsd_aclist_steerable(to_bssinfo, &sta->addr) == BSD_FAIL) {
		BSD_STEER("Skip sta[%p]:"MACF" to %s not steerable match "
			"w/ static maclist.\n",
			sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames);
		return FALSE;
	}

	/* check sta balance */
	if (intf_info->qualify_cfg.flags & BSD_STEERING_POLICY_FLAG_LOAD_BAL) {
		goto qualify;
	}

	/* check RSSI, VHT, NON_VHT STAs on target bssinfo */
	/* if this STA violates VHT, NON_VHT check */
	/* check bsd_steering_policy, and bsd_if_qualify_policy(<min bw util%> <ext flag> */
	vht = (sta->flags & WL_STA_VHT_CAP) ? 1 : 0;
	if ((vht == 1) && (intf_info->qualify_cfg.flags & BSD_QUALIFY_POLICY_FLAG_VHT)) {
		BSD_STEER("sta[%p]:"MACF" %s VHT not qualified.\n",
			sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames);
		return FALSE;
	}

	if ((vht == 0) && (intf_info->qualify_cfg.flags & BSD_QUALIFY_POLICY_FLAG_NON_VHT)) {
		BSD_STEER("sta[%p]:"MACF" %s Non-VHT not qualified.\n",
			sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames);
		return FALSE;
	}

	/* check estimated RSSI if STA is on target RF */
	{
		int32 est_rssi = sta->rssi;
		int ret = 0;
		time_t now;
		bsd_maclist_t *prbsta;
		int i, prb_rssi_s = 0, prb_rssi_t = 0;

		/* use rssi log at previous steering in prbsta as delta to adjust */
		prbsta = bsd_prbsta_by_addr(info, &sta->addr, FALSE);
		if (prbsta) {
			now = time(NULL);
			BSD_STEER("**** %s: "MACF", from_ifidx=%d, to_ifidx=%d, now=%lu\n",
				__FUNCTION__,  ETHERP_TO_MACF(&sta->addr),
				bssinfo->intf_info->idx, to_ifidx, now);

			for (i = 0; i < BSD_MAX_INTF; i++) {

				BSD_STEER("i=%d, timestamp=%lu, rssi=%d\n",
					i, prbsta->rssi_info[i].timestamp,
					prbsta->rssi_info[i].rssi);

				if (i == bssinfo->intf_info->idx) {
					prb_rssi_s = prbsta->rssi_info[i].rssi;
				}
				else if (i == to_ifidx) {
					prb_rssi_t = prbsta->rssi_info[i].rssi;
					if ((now - prbsta->rssi_info[i].timestamp) <
						BSD_PROBEREQ_RSSI_VALID_PERIOD) {
						ret = 1;
					}
				}
			}
		}

		if (prb_rssi_s && prb_rssi_t) {
			if (ret)
				est_rssi = prb_rssi_t;
			else {
				est_rssi += prb_rssi_t;
				est_rssi -= prb_rssi_s;
			}
			BSD_STEER("ret=%d, rssi_s=%d, rssi_t=%d, sta->rssi=%d, est_rssi=%d\n",
				ret, prb_rssi_s, prb_rssi_t, sta->rssi, est_rssi);
		}
		else {
			BSD_STEER("rssi log not available, goto qualify\n");
			goto qualify;
		}

		if (intf_info->qualify_cfg.rssi && (intf_info->qualify_cfg.rssi > est_rssi)) {
			BSD_STEER("sta[%p]:"MACF" %s RSSI not qualified - "
				"rssi:%d est_rssi:%d cfg.rssi:%d.\n",
				sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames,
				sta->rssi, est_rssi, intf_info->qualify_cfg.rssi);
			return FALSE;
		}
		else {
			BSD_STEER("sta[%p]:"MACF" %s RSSI qualified - "
				"rssi:%d est_rssi:%d cfg.rssi:%d.\n",
				sta, ETHERP_TO_MACF(&sta->addr), to_bssinfo->ifnames,
				sta->rssi, est_rssi, intf_info->qualify_cfg.rssi);
		}
	}

qualify:

	sta->to_bssinfo = to_bssinfo;

	steer_reason = sta->bssinfo->intf_info->steering_flags;
	BSD_STEER("steer_reason=0x%x sta flags=0x%x rssi=%d "
		"to_bssinfo's bsd_trigger_policy min=%d max=%d rssi=%d flags=0x%x\n",
		steer_reason, sta->flags, sta->rssi,
		intf_info->qualify_cfg.min_bw,
		intf_info->steering_cfg.chan_busy_max,
		intf_info->steering_cfg.rssi,
		intf_info->steering_cfg.flags);

	if (steer_reason &
		(BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB | BSD_STEERING_POLICY_FLAG_RSSI |
		BSD_STEERING_POLICY_FLAG_VHT | BSD_STEERING_POLICY_FLAG_NON_VHT |
		BSD_STEERING_POLICY_FLAG_NEXT_RF |
		BSD_STEERING_POLICY_FLAG_PHYRATE |
		BSD_STEERING_POLICY_FLAG_LOAD_BAL |
		BSD_STEERING_POLICY_FLAG_STA_NUM_BAL)) {
		qualify = TRUE;
	}

	if (qualify) {
		BSD_STEER("selected sta[%p]:"MACF" on %s qualified for target if:%s\n",
			sta, ETHERP_TO_MACF(&sta->addr),
			sta->bssinfo->ifnames, to_bssinfo->ifnames);
	} else {
		BSD_STEER("selected sta[%p]:"MACF" DQ'd\n",
			sta, ETHERP_TO_MACF(&sta->addr));
	}

	BSD_ATEXIT();

	return qualify;
}

/* BSD Engine based implementation */
static void bsd_update_sta_triggering_policy(bsd_info_t *info, int ifidx)
{
	int bssidx, next_ifidx;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	bsd_sta_info_t *sta;
	bsd_steering_policy_t *steering_cfg;
	bool vht;
	bool check_rule;
	uint32 check_rule_mask; /* a steering config mask for AND logic */
	int ge_check;
	uint32 phyrate;

	BSD_ENTER();

	/* check this if, or next if invite channel oversubscription */
	intf_info = &(info->intf_info[ifidx]);
	next_ifidx = bsd_get_preferred_if(info, ifidx);
	steering_cfg = &intf_info->steering_cfg;

	if (steering_cfg->flags & BSD_STEERING_POLICY_FLAG_NEXT_RF) {
		BSD_STEER("legacy ifidx:%d, next ifidx:%d\n", ifidx, next_ifidx);
		if ((next_ifidx != -1) &&
			((info->intf_info[next_ifidx].state == BSD_CHAN_IDLE) ||
			(info->intf_info[next_ifidx].state == BSD_CHAN_NO_STATS))) {
			BSD_STEER("ifidx:%d, next ifidx:%d triggering set\n", ifidx, next_ifidx);

			/* set steering bit */
			intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_NEXT_RF;
			return;
		}
	} else {
		BSD_STEER("ifidx:%d state:%d\n", ifidx, intf_info->state);
		if ((intf_info->state == BSD_CHAN_BUSY) &&
			bsd_check_if_oversub(info, intf_info)) {
			intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB;
		}
	}

	check_rule = (steering_cfg->flags & BSD_STEERING_POLICY_FLAG_RULE) ? 1 : 0;
	check_rule_mask = bsd_get_steering_mask(steering_cfg);

	/* Parse assoc list and read all sta_info */
	for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
		if (!(bssinfo->valid) ||
			!BSD_BSS_BSD_ENABLED(bssinfo))
			continue;

		BSD_STEER("ifidx:%d bssidx:%d intf:%s steering_cfg flags:0x%x assoclist count:%d\n",
			ifidx, bssidx,  bssinfo->ifnames, steering_cfg->flags,
			bssinfo->assoc_cnt);

		/* assoclist */
		sta = bssinfo->assoclist;
		BSD_INFO("sta[%p]\n", sta);
		while (sta) {
			vht = sta->flags & WL_STA_VHT_CAP ? 1 : 0;
			BSD_STEER("sta[%p]:"MACF" - rssi:%d phyrate:%d tx_rate:%d rx_rate:%d "
				"datarate:%d at_ratio:%d vht:%d tx_pkts:%d rx_pkts:%d "
				"tx_bps:%d rx_bps:%d\n",
				sta, ETHERP_TO_MACF(&sta->addr),
				sta->rssi,
				sta->phyrate,
				sta->tx_rate,
				sta->rx_rate,
				sta->datarate,
				sta->at_ratio,
				vht,
				sta->tx_pkts,
				sta->rx_pkts,
				sta->tx_bps,
				sta->rx_bps);

			/* skipped non-steerable STA */
			if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
				BSD_STEER("Skip sta[%p]:"MACF" is not steerable.\n",
					sta, ETHERP_TO_MACF(&sta->addr));
				goto next;
			}

			if (bsd_bcm_special_sta_check(info, sta)) {
				BSD_STEER("Skip sta[%p]:"MACF" - special sta.\n",
					sta, ETHERP_TO_MACF(&sta->addr));

				goto next;
			}

			/* check this if's RSSI, VHT policy triggerings */
			if (steering_cfg->rssi != 0) {

				if (steering_cfg->flags & BSD_STEERING_POLICY_FLAG_RSSI) {
					ge_check = 1;
				} else {
					ge_check = 0;
				}

				BSD_STEER("adjusting RSSI ifidx:%d bssidx:%d "
					"intf:%s sta_rssi:%d ge_check:%d\n",
					ifidx, bssidx,
					bssinfo->ifnames, sta->rssi, ge_check);

				if (((ge_check == 0) && (sta->rssi < steering_cfg->rssi)) ||
					((ge_check == 1) && (sta->rssi >= steering_cfg->rssi))) {
					BSD_STEER("set RSSI ifidx:%d bssidx:%d intf:%s\n",
						ifidx, bssidx, bssinfo->ifnames);
					intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_RSSI;
					if (check_rule == 0)
						goto next;
				}
			}

			/* invalid STAs */
			if ((steering_cfg->flags & BSD_STEERING_POLICY_FLAG_VHT) && vht) {
				BSD_STEER("set VHT ifidx:%d bssidx:%d intf:%s\n",
					ifidx, bssidx, bssinfo->ifnames);

				intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_VHT;

				if (check_rule == 0)
					goto next;
			}

			if ((steering_cfg->flags & BSD_STEERING_POLICY_FLAG_NON_VHT) &&
				(vht == 0)) {
				BSD_STEER("set NON_VHT ifidx:%d bssidx:%d intf:%s\n",
					ifidx, bssidx, bssinfo->ifnames);
				intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_NON_VHT;

				if (check_rule == 0)
					goto next;
			}

			if (steering_cfg->phyrate != 0) {
				ge_check =
				(steering_cfg->flags & BSD_STEERING_POLICY_FLAG_PHYRATE) ? 1 : 0;

				/* adjust STA's phyrate, phyrate == 0 skip */
				/* phyrate = (sta->phyrate != 0) ? sta->phyrate : sta->tx_rate; */
				/* phyrate = sta->phyrate; */
				phyrate = sta->mcs_phyrate;

				if (phyrate &&
					(((ge_check == 0) && (phyrate < steering_cfg->phyrate)) ||
					((ge_check == 1) && (phyrate >= steering_cfg->phyrate)))) {
					BSD_STEER("set PHYRATE ifidx:%d bssidx:%d intf:%s\n",
						ifidx, bssidx, bssinfo->ifnames);

					intf_info->steering_flags |=
						BSD_STEERING_POLICY_FLAG_PHYRATE;

					if (check_rule == 0)
						goto next;
				}
			}

		if (check_rule) {
			if ((intf_info->steering_flags & check_rule_mask) ==
				intf_info->steering_flags) {
				/* AND logic met */
				BSD_STEER("%s sta[%p] POLICY_FLAG_RULE AND met: 0x%x\n",
					bssinfo->ifnames, sta, intf_info->steering_flags);
				break;
			} else {
				/* reset steering_flags for AND restriction triggering */
				intf_info->steering_flags = 0;
			}
		}

next:
			BSD_INFO("next[%p]\n", sta->next);
			sta = sta->next;
		}
	}

	if (intf_info->state == BSD_CHAN_BUSY) {
		intf_info->steering_flags |= BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB;
	}

	BSD_EXIT();
	return;
}

bool bsd_check_bss_all_sta_same_type(bsd_bssinfo_t *bssinfo)
{
	int first_vht = -1; /* init */
	int vht;
	bsd_sta_info_t *sta = NULL;

	BSD_ENTER();

	/* assoclist */
	sta = bssinfo->assoclist;

	if (sta == NULL) {
		BSD_MULTI_RF("0 STA on bss\n");
		return FALSE;
	}

	BSD_MULTI_RF("sta[%p]\n", sta);
	while (sta) {
		vht = sta->flags & WL_STA_VHT_CAP ? 1 : 0;
		if (first_vht == -1) {
			first_vht = vht;
			continue;
		}

		/* same type compare */
		if (vht != first_vht) {
			return FALSE;
		}

		BSD_MULTI_RF("next[%p]\n", sta->next);
		sta = sta->next;
	}

	BSD_MULTI_RF("same type result: %d\n", first_vht);

	BSD_EXIT();

	return TRUE;
}

static void bsd_update_sta_balance(bsd_info_t *info)
{
	int idx, bssidx;
	int to_ifidx, to_bssidx;
	bsd_intf_info_t *intf_info, *to_intf_info;
	bsd_bssinfo_t *bssinfo, *to_bssinfo;

	BSD_ENTER();

	/* enumerate RFs, and only check for two RFs' condition */
	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);

		if (!(intf_info->steering_cfg.flags & (BSD_STEERING_POLICY_FLAG_LOAD_BAL |
				BSD_STEERING_POLICY_FLAG_STA_NUM_BAL))) {
			continue;
		}
		BSD_MULTI_RF("For [idx=%d] steering_flags=0x%x\n",
			idx, intf_info->steering_flags);
		bssidx = bsd_get_steerable_bss(info, intf_info);
		if (bssidx == -1) {
			BSD_MULTI_RF("[%d] bssidx == -1 skip\n", idx);
			continue;
		}
		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
		BSD_MULTI_RF("idx=%d bssidx=%d ifname=%s assoc_cnt=%d\n",
				idx, bssidx, bssinfo->ifnames, bssinfo->assoc_cnt);

		to_ifidx = bsd_get_preferred_if(info, idx);
		if (to_ifidx == -1) {
			BSD_MULTI_RF("[%d] to_ifidx == -1 skip\n", idx);
			continue;
		}

		BSD_MULTI_RF("idx=%d to_ifidx=%d\n", idx, to_ifidx);
		to_intf_info = &(info->intf_info[to_ifidx]);

		to_bssidx = bsd_get_steerable_bss(info, to_intf_info);

		if (to_bssidx == -1) {
			BSD_MULTI_RF("[%d] to_bssidx == -1 skip\n", idx);
			continue;
		}
		to_bssinfo = &(to_intf_info->bsd_bssinfo[to_bssidx]);

		if ((intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_STA_NUM_BAL) &&
			(to_intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_STA_NUM_BAL)) {

			BSD_MULTI_RF("bssinfo->assoc_cnt[%d] to_bssinfo->assoc_cnt[%d] \n",
				bssinfo->assoc_cnt, to_bssinfo->assoc_cnt);
			if ((bssinfo->assoc_cnt - to_bssinfo->assoc_cnt) > 1) {
				intf_info->steering_flags = BSD_STEERING_POLICY_FLAG_STA_NUM_BAL;
			}
		}

		if (intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_LOAD_BAL) {
			if (to_intf_info->steering_cfg.flags & BSD_STEERING_POLICY_FLAG_LOAD_BAL) {
				/* set BSD_STEERING_POLICY_FLAG_LOAD_BAL to intf_info */
				if ((bssinfo->assoc_cnt > 1) && (to_bssinfo->assoc_cnt == 0)) {
					intf_info->steering_flags |=
						BSD_STEERING_POLICY_FLAG_LOAD_BAL;
					BSD_MULTI_RF("set idx=%d flag LOAD_BAL (to_ifidx=%d)\n",
						idx, to_ifidx);
					break;
				}
			}
		}
	}

	BSD_EXIT();

	return;
}

/* Multi-RF steering policy algorithm */
void bsd_steer_scheme_policy(bsd_info_t *info)
{
	int idx, to_ifidx, bssidx;
	bsd_sta_info_t *sta = NULL;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;

	BSD_ENTER();

	if (BSD_DUMP_ENAB) {
		BSD_PRINT("\nBefore steer Check: dump dbg info========= \n");
		bsd_dump_info(info);
		BSD_PRINT("\n============================= \n");
	}

	BSD_STEER("#### Policy BSD Start ####\n");
	/* update all RF interfaces' histogram chan state */
	for (idx = 0; idx < info->max_ifnum; idx++) {
		BSD_STEER("*** update chan state: idx=%d ***\n", idx);
		/* reset steering_flags */
		intf_info = &info->intf_info[idx];

		if (intf_info->enabled != TRUE) {
			BSD_INFO("Skip: idx %d is not enabled\n", idx);
			continue;
		}

		intf_info->steering_flags = 0;

		/* update channel utilization state */
		bssidx = bsd_get_steerable_bss(info, intf_info);
		if (bssidx == -1) {
			BSD_INFO("Skip: fail to get_steerable_bss (idx=%d)\n", idx);
			intf_info->state = BSD_CHAN_BUSY_UNKNOWN;
			continue;
		}

		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
		BSD_STEER("bssidx=%d, ifname=%s\n", bssidx, bssinfo->ifnames);
		bsd_update_chan_state(info, intf_info, bssinfo);

		/* check RSSI, phy-rate, VHT, Non-VHT STA triggers */
		bsd_update_sta_triggering_policy(info, idx);

		BSD_STEER("*** [idx=%d] steering_flags:0x%x state:%d band:%s ***\n",
			idx, intf_info->steering_flags, intf_info->state,
			(intf_info->band == BSD_BAND_5G) ? "5G" : "2G");
	}

	/* check and set STA balance triggering */
	bsd_update_sta_balance(info);

	/* check all RF interfaces' steering policy triggering condition, */
	/* and take steering action */
	for (idx = 0; idx < info->max_ifnum; idx++) {
		BSD_STEER("=== Check interface & sta select: idx=%d, steering_flags=0x%x ===\n",
			idx, info->intf_info[idx].steering_flags);

		if (info->intf_info[idx].steering_flags == 0) {
			BSD_STEER("[%d] steering_flags=0 skip\n", idx);
			continue;
		}

		to_ifidx = bsd_get_preferred_if(info, idx);
		if (to_ifidx == -1) {
			BSD_STEER("[%d] to_ifidx == -1 skip\n", idx);
			continue;
		}

		/* STA selection */
		sta = bsd_sta_select_policy(info, idx, to_ifidx);
		if (sta == NULL) {
			BSD_STEER("[%d] no STA selected, skip ..\n", idx);
			continue;
		}

		BSD_STEER("!!!STA "MACF" selected!!! steer from idx %d to %d\n",
			ETHER_TO_MACF(sta->addr), idx, to_ifidx);

		/* Handler if the STA is not detected as dualband so far */
		if ((((sta->bssinfo->intf_info->band != sta->to_bssinfo->intf_info->band)) &&
			(!bsd_is_sta_dualband(info, &sta->addr))) ||
			!bsd_qualify_sta_rf(info, sta->bssinfo, sta->to_bssinfo, &sta->addr)) {
			if (sta->band_detect_cnt <= BSD_BAND_DETECT_MAX_TRY) {
				BSD_STEER("%s: dualband STA detect count: %d\n",
				__FUNCTION__, sta->band_detect_cnt);
				sta->band_detect_cnt++;
			} else {
				BSD_STEER("Skip STA "MACF" reason: dualband invalid\n",
					ETHERP_TO_MACF(&sta->addr));
				continue;
			}
		}

		bsd_steer_sta(info, sta, sta->to_bssinfo);
	}

	if (BSD_DUMP_ENAB) {
		BSD_PRINT("\nAfter Steer Check: dump dbg info========= \n");
		bsd_dump_info(info);
		BSD_PRINT("\n============================= \n");
	}
	BSD_EXIT();
	return;
}

/* Multi-RF SmartConnect STA selection policy algorithm */
bsd_sta_info_t *bsd_sta_select_policy(bsd_info_t *info, int ifidx, int to_ifidx)
{
	bsd_intf_info_t *intf_info, *to_intf_info;
	bsd_bssinfo_t *bssinfo;
	bsd_sta_select_policy_t *sta_select_cfg;
	int bssidx, to_bssidx;
	bsd_sta_info_t *sta = NULL, *victim = NULL;
	int32 score;
	bool score_check;
	time_t now = time(NULL);
	bool vht;
	bool check_rule;
	uint32 check_rule_mask; /* a sta select config mask for AND logic */
	uint32 sta_rule_met; /* processed mask for AND logic */
	bool active_sta_check;
	/* determine the target interface is primary or secondary */
	bsd_if_bssinfo_list_t *if_bss_list;
	int32 rssi_adj;
	uint32 not_timeout_sta_cnt = 0;
	uint32 special_sta_cnt = 0;
	uint32 special_sta_rate = 0;

	BSD_ENTER();

	intf_info = &(info->intf_info[ifidx]);
	to_intf_info = &(info->intf_info[to_ifidx]);
	BSD_STEER("ifidx=%d to_ifidx=%d steering_flags:0x%x\n",
		ifidx, to_ifidx, intf_info->steering_flags);

	bssidx = bsd_get_steerable_bss(info, intf_info);
	if (bssidx == -1) {
		BSD_STEER("skip: bssidx not found\n");
		return NULL;
	}

	to_bssidx = bsd_get_steerable_bss(info, to_intf_info);
	if (to_bssidx == -1) {
		BSD_STEER("skip: to_bssidx not found\n");
		return NULL;
	}

	bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

	sta_select_cfg = &bssinfo->sta_select_cfg;

	/* sta balance check */
	if ((sta_select_cfg->flags & BSD_QUALIFY_POLICY_FLAG_LOAD_BAL) &&
		(bssinfo->assoc_cnt <= 1)) {
		BSD_STEER("skip: ifname=%s, assoc_cnt=%d\n", bssinfo->ifnames, bssinfo->assoc_cnt);
		return NULL;
	}

	check_rule = sta_select_cfg->flags & BSD_STA_SELECT_POLICY_FLAG_RULE ? 1 : 0;

	check_rule_mask = bsd_get_sta_select_mask(sta_select_cfg);

	/* active sta check if bit is 0, not check if bit is 1 */
	active_sta_check =
		(sta_select_cfg->flags & BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA) ? 0 : 1;

	/* use score bit to determine how to pickup (small or big) score */
	if_bss_list = bssinfo->to_if_bss_list; /* interface list in nvram bsd_if_select_policy  */

	if (if_bss_list->to_ifidx == to_ifidx) {
		/* use bit10 for score comparison for the primary target interface */
		score_check =
			(sta_select_cfg->flags & BSD_STA_SELECT_POLICY_FLAG_SCORE) ? 1 : 0;
	}
	else {
		/* use bit11 for score comparison for all other target interface */
		score_check =
			(sta_select_cfg->flags & BSD_STA_SELECT_POLICY_FLAG_SCORE2) ? 1 : 0;
	}

	if (score_check)
		score = BSD_INIT_SCORE_MIN; /* set small init value, pickup sta with bigger score */
	else
		score = BSD_INIT_SCORE_MAX; /* set big init value, pickup sta with smaller score */

	BSD_STEER("intf:%d bssidx[%d] ifname:%s flags:0x%x "
		"sta selection logic='%s mask 0x%x' active_sta_check:%d score_check:%d\n",
		ifidx, bssidx, bssinfo->ifnames,
		sta_select_cfg->flags,
		check_rule ? "AND" : "OR",
		check_rule_mask,
		active_sta_check, score_check);

	/* assoclist */
	sta = bssinfo->assoclist;
	BSD_INFO("--- Start from sta[%p] on ifname %s\n", sta, bssinfo->ifnames);
	while (sta) {
		uint32 rate = sta->datarate;

		if (bsd_bcm_special_sta_check(info, sta)) {
			special_sta_cnt++;
			special_sta_rate += rate;
			BSD_STEER("Skip sta[%p]="MACF" - special sta, cnt+:%d, rate+:%u\n",
				sta, ETHERP_TO_MACF(&sta->addr),
				special_sta_cnt, special_sta_rate);
			goto next;
		}

		/* skip non-steerable STA */
		if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
			BSD_STEER("sta[%p]:"MACF" is not steerable. Skipped.\n",
				sta, ETHERP_TO_MACF(&sta->addr));
			goto next;
		}

		/* suspend one time if sta's bss response to reject the target BSSID */
		if (sta->steerflag & BSD_STA_BSS_REJECT) {
			sta->steerflag &= ~BSD_STA_BSS_REJECT;
			BSD_STEER("Skip sta[%p]="MACF" - STA BSS Reject\n",
				sta, ETHERP_TO_MACF(&sta->addr));
			goto next;
		}

		/* if steer_no_deauth is set and sta did not respond to
		 * max_steer_req_cnt then skip this sta for steering
		*/
		if (info->steer_no_deauth && info->non11v_cfg.cnt &&
			bsd_check_non11v_sta(info, &sta->addr)) {
			BSD_STEER("Skip sta[%p]="MACF" - Max steer req count reched \n",
				sta, ETHERP_TO_MACF(&sta->addr));
			goto next;
		}

		BSD_STEER("STA="MACF" active_sta_check:%d rate:%d cfg->idle_rate:%d\n",
			ETHERP_TO_MACF(&sta->addr),
			active_sta_check, rate, sta_select_cfg->idle_rate);

		if (active_sta_check && (rate > sta_select_cfg->idle_rate)) {
				BSD_STEER("Skip %s STA:"MACF" idle_rate[%d] "
					"tx+rx_rate[%d: %d+%d]\n",
					active_sta_check ? "active" : "idle",
					ETHERP_TO_MACF(&sta->addr),
					sta_select_cfg->idle_rate,
					rate,
					sta->tx_bps, sta->rx_bps);
				goto next;
		}

		BSD_BOUNCE("STA:"MACF", check ...\n",  ETHERP_TO_MACF(&sta->addr));

		if (bsd_check_bouncing_sta(info, &sta->addr)) {
			BSD_BOUNCE("Skip bouncing STA:"MACF", skip ..\n",
				ETHERP_TO_MACF(&sta->addr));
			goto next;
		} else {
			BSD_BOUNCE("None bouncing STA:"MACF", further ...\n",
				ETHERP_TO_MACF(&sta->addr));
		}

		if (bsd_if_qualify_sta(info, to_ifidx, sta) == FALSE) {
			BSD_STEER("Skip sta[%p] not qualify for to_ifidx:%d.\n", sta, to_ifidx);
			goto next;
		}

		/* Policy based on sta num balance pre-empt all other rules */
		if (intf_info->steering_flags & BSD_STEERING_POLICY_FLAG_STA_NUM_BAL) {
			BSD_STEER("Candidate STA [%p]: STA_NUM_BAL\n", sta);
			goto scoring;
		}

		/* sta balance pre-empt all other policy rules */
		if ((intf_info->steering_flags & BSD_STEERING_POLICY_FLAG_LOAD_BAL) &&
			(sta_select_cfg->flags & BSD_QUALIFY_POLICY_FLAG_LOAD_BAL)) {
			BSD_STEER("Candidate STA [%p]: LOAD_BAL\n", sta);
			goto scoring;
		}

		/* reset sta rule bits */
		sta_rule_met = 0;

		if (intf_info->steering_flags & BSD_STEERING_POLICY_FLAG_NEXT_RF) {
			BSD_STEER("selected legacy NEXT_RF sta[%p]\n", sta);
			if (check_rule == 0) {
				BSD_STEER("Candidate STA [%p]: NEXT_RF\n", sta);
				goto scoring;
			}
		}

		vht = sta->flags & WL_STA_VHT_CAP ? 1 : 0;

		/* VHT vs. Non_VHT STAs  */
		if (intf_info->steering_flags & BSD_STA_SELECT_POLICY_FLAG_VHT) {
			if (vht == 1) {
				BSD_STEER("select VHT sta[%p]\n", sta);

				sta_rule_met |= BSD_STA_SELECT_POLICY_FLAG_VHT;
				if (check_rule == 0) {
					BSD_STEER("Candidate STA [%p]: VHT\n", sta);
					goto scoring;
				}
			} else {
				BSD_STEER("skip VHT sta[%p]\n", sta);
				goto next;
			}
		}

		if (intf_info->steering_flags & BSD_STA_SELECT_POLICY_FLAG_NON_VHT) {
			if (vht == 0) {
				BSD_STEER("select NON_VHT sta[%p]\n", sta);

				sta_rule_met |= BSD_STA_SELECT_POLICY_FLAG_NON_VHT;
				if (check_rule == 0) {
					BSD_STEER("Candidate STA [%p]: NON_VHT\n", sta);
					goto scoring;
				}
			} else {
				BSD_STEER("skip NON_VHT sta[%p]\n", sta);
				goto next;
			}
		}

		/* PHY RATE */
		if (intf_info->steering_flags & BSD_STEERING_POLICY_FLAG_PHYRATE) {
			if (sta_select_cfg->phyrate == 0) {
				BSD_STEER("STA [%p]: PHYRATE threshold not configured\n", sta);
				sta_rule_met |= BSD_STA_SELECT_POLICY_FLAG_PHYRATE;
				if (check_rule == 0) {
					BSD_STEER("Candidate STA [%p]: PHYRATE to score\n", sta);
					goto scoring;
				}
			}
			else {
			int phy_check;
			uint32 adj_phyrate;

			phy_check =
			(sta_select_cfg->flags & BSD_STA_SELECT_POLICY_FLAG_PHYRATE) ? 1 : 0;

			/* adjust STA's phyrate, phyrate == 0 skip */
			/* adj_phyrate = (sta->phyrate != 0) ? sta->phyrate : sta->tx_rate; */
			adj_phyrate = sta->mcs_phyrate;

			BSD_MULTI_RF("sta[%p] adj_phyrate:%d tx_rate:%d phyrate:%d\n",
				sta, adj_phyrate, sta->tx_rate, sta->phyrate);

			if (adj_phyrate != 0) {
			if ((((phy_check == 0) && (adj_phyrate < sta_select_cfg->phyrate)) ||
				((phy_check == 1) && (adj_phyrate >= sta_select_cfg->phyrate)))) {
					BSD_STEER("selected PHYRATE sta[%p]\n", sta);

					sta_rule_met |= BSD_STA_SELECT_POLICY_FLAG_PHYRATE;
					if (check_rule == 0) {
						BSD_STEER("Candidate STA [%p]: PHYRATE\n", sta);
						goto scoring;
					}
				} else {
					BSD_STEER("phyrate checked, skip sta[%p]\n", sta);
					goto next;
				}
			} else {
				BSD_STEER("skip phyrate 0 sta[%p]\n", sta);
				goto next;
			}
			}
		}

		/* New RSSI change, pick the lowest RSSI STA */
		if (sta_select_cfg->rssi != 0) {
			int ge_check;

			ge_check =
			(intf_info->steering_cfg.flags & BSD_STA_SELECT_POLICY_FLAG_RSSI) ? 1 : 0;

			BSD_STEER("RSSI check: low rssi STA:"MACF" sta_rssi"
					"[%d] <  thld[%d]\n",
					ETHERP_TO_MACF(&sta->addr),
					sta->rssi, bssinfo->sta_select_cfg.rssi);

			/* customize low, or high rssi check */
			if (((ge_check == 0) && (sta->rssi < intf_info->steering_cfg.rssi)) ||
				((ge_check == 1) && (sta->rssi >= intf_info->steering_cfg.rssi))) {
				BSD_STEER("Found %s rssi STA:"MACF" sta_rssi"
					"[%d] <  thld[%d]\n",
					ge_check ? "low" : "high",
					ETHERP_TO_MACF(&sta->addr),
					sta->rssi, bssinfo->sta_select_cfg.rssi);
				sta_rule_met |= BSD_STA_SELECT_POLICY_FLAG_RSSI;
				if (check_rule == 0) {
					BSD_STEER("Candidate STA [%p]: RSSI\n", sta);
					goto scoring;
				}
			} else {
				BSD_STEER("Skip sta ["MACF" ] rssi checked.\n",
					ETHERP_TO_MACF(&sta->addr));
				goto next;
			}
		}

		/* logic AND all for the above */
		if (check_rule) {
			BSD_STEER("sta[%p] AND check check_rule_mask:0x%x\n",
				sta, check_rule_mask);
			if ((sta_rule_met & check_rule_mask) == check_rule_mask) {
				BSD_STEER("sta[%p] AND logic rule met - 0x%x\n",
					sta, sta_rule_met);
			} else {
				BSD_STEER("sta[%p] AND logic rule not met - 0x%x, skip\n",
					sta, sta_rule_met);
				goto next;
			}
		}

		BSD_STEER("Candidate STA [%p]: others\n", sta);

scoring:
		/* adjust rssi to a positive value before counting it to score */
		rssi_adj = 100 + sta->rssi;
		if (rssi_adj < 0)
			rssi_adj = 0;

		sta->score = sta->prio * bssinfo->sta_select_cfg.wprio +
			rssi_adj * bssinfo->sta_select_cfg.wrssi+
			sta->mcs_phyrate * bssinfo->sta_select_cfg.wphy_rate +
			/* sta->tx_failures * bssinfo->sta_select_cfg.wtx_failures + */
			sta->tx_rate * bssinfo->sta_select_cfg.wtx_rate +
			sta->rx_rate * bssinfo->sta_select_cfg.wrx_rate;

		BSD_STEER("sta[%p] "MACF" Score[%d] prio[%d], rssi[%d] "
			"phyrate[%d] tx_failures[%d] tx_rate[%d] rx_rate[%d]\n",
			sta, ETHERP_TO_MACF(&sta->addr), sta->score,
			sta->prio, sta->rssi, sta->mcs_phyrate,
			sta->tx_failures, sta->tx_bps, sta->rx_bps);

		if (((score_check == 0) && (sta->score < score)) ||
			((score_check == 1) && (sta->score >= score))) {
			/* timestamp check to avoid flip'n'flop ? */
			BSD_STEER("found victim "MACF" now[%lu]- timestamp[%lu]"
				"= %lu timeout[%d] \n",
				ETHERP_TO_MACF(&sta->addr), now,
				sta->timestamp, now - sta->timestamp,
				info->steer_timeout);

			if (now - sta->timestamp > info->steer_timeout)	{
				BSD_STEER("found victim="MACF" score=%d\n",
					ETHERP_TO_MACF(&sta->addr), sta->score);

				if (victim) {
					BSD_STEER("swap victim: old="MACF" score=%d\n",
						ETHERP_TO_MACF(&victim->addr), score);
				}

				victim = sta;
				score = sta->score;
			}
			else {
				not_timeout_sta_cnt++;
				BSD_STEER("sta[%p]: waiting for timeout[%d], skip\n",
					sta, info->steer_timeout);
			}
		}

next:
		BSD_INFO("next[%p]\n", sta->next);
		sta = sta->next;
	}

	BSD_STEER("assoc=%d, not_timeout=%d, special=%d, lb_idle_rate=%u, rate=%u\n",
		bssinfo->assoc_cnt, not_timeout_sta_cnt, special_sta_cnt,
		info->lb_idle_rate, special_sta_rate);

	if (victim) {
		if ((sta_select_cfg->flags & BSD_QUALIFY_POLICY_FLAG_LOAD_BAL) &&
			(bssinfo->assoc_cnt - special_sta_cnt - not_timeout_sta_cnt <= 1) &&
			(special_sta_rate < info->lb_idle_rate)) {
				victim = NULL;
				BSD_STEER("No sta selected: cnt <=1 && rate < lb_idle_rate\n");
		} else {
			BSD_STEER("Victim sta[%p]="MACF" Score=%d\n",
				victim, ETHERP_TO_MACF(&victim->addr), victim->score);
		}
	}

	BSD_EXIT();
	return victim;
}

int bsd_get_steerable_bss(bsd_info_t *info, bsd_intf_info_t *intf_info)
{
	int bssidx;
	bsd_bssinfo_t *bssinfo;
	bool found = FALSE;

	for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
		if (bssinfo->valid && (bssinfo->steerflag != BSD_BSSCFG_NOTSTEER) &&
			BSD_BSS_BSD_ENABLED(bssinfo)) {
			if (CHSPEC_IS2G(bssinfo->chanspec)) {
				found = TRUE;
				break;
			} else if (!bsd_is_chan_passive(bssinfo)) {
				found = TRUE;
				break;
			}
		}
	}

	BSD_MULTI_RF("ifidx:%d found:%d, bssidx:%d\n", intf_info->idx, found, bssidx);
	if (found == FALSE) {
		return -1;
	}

	return bssidx;
}
