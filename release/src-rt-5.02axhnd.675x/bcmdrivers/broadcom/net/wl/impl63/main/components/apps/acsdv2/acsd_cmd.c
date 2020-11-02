/*
 * ACS deamon command module (Linux)
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
 * $Id: acsd_cmd.c 784556 2020-03-02 10:22:00Z $
 */

#include "acsd_svr.h"

extern void acs_cleanup_scan_entry(acs_chaninfo_t *c_info);

#define CHANIM_GET(field, unit)						\
	{ \
		if (!strcmp(param, #field)) { \
			chanim_info_t* ch_info = c_info->chanim_info; \
			if (ch_info) \
				*r_size = sprintf(buf, "%d %s", \
					chanim_config(ch_info).field, unit); \
			else \
				*r_size = sprintf(buf, "ERR: Requested info not available"); \
			goto done; \
		} \
	}

#define CHANIM_SET(field, unit, type)				\
	{ \
		if (!strcmp(param, #field)) { \
			chanim_info_t* ch_info = c_info->chanim_info; \
			if (!ch_info) { \
				*r_size = sprintf(buf, "ERR: set action not successful"); \
				goto done; \
			} \
			if ((setval > (type)chanim_range(ch_info).field.max_val) || \
				(setval < (type)chanim_range(ch_info).field.min_val)) { \
				*r_size = sprintf(buf, "Value out of range (min: %d, max: %d)\n", \
					chanim_range(ch_info).field.min_val, \
					chanim_range(ch_info).field.max_val); \
				goto done; \
			} \
			chanim_config(ch_info).field = setval; \
			*r_size = sprintf(buf, "%d %s", chanim_config(ch_info).field, unit); \
			goto done; \
		} \
	}

#define ACS_CMD_TEST_DFSR		"test_dfsr"
#define ACS_CMD_TEST_PRECLEAR		"test_preclear"
#define ACS_CMD_ZDFS_5G_MOVE		"zdfs_5g_move"
#define ACS_CMD_ZDFS_5G_PRECLEAR	"zdfs_5g_preclear"
#define ACS_CMD_ZDFS_2G_MOVE		"zdfs_2g_move"
#define ACS_CMD_ZDFS_2G_PRECLEAR	"zdfs_2g_preclear"

static int
acsd_extract_token_val(char* data, const char *token, char *output, int len)
{
	char *p, *c;
	char copydata[512];
	char *val;

	if (data == NULL)
		goto err;

	strncpy(copydata, data, sizeof(copydata));
	copydata[sizeof(copydata) - 1] = '\0';

	ACSD_DEBUG("copydata: %s\n", copydata);

	p = strstr(copydata, token);
	if (!p)
		goto err;

	if ((c = strchr(p, '&')))
		*c++ = '\0';

	val = strchr(p, '=');
	if (!val)
		goto err;

	val += 1;
	ACSD_DEBUG("token_val: %s\n", val);

	strncpy(output, val, len);
	output[len - 1] = '\0';

	return strlen(output);

err:
	return -1;
}

static int
acsd_pass_candi(ch_candidate_t * candi, int count, char** buf, uint* r_size)
{
	int totsize = 0;
	int i, j;
	ch_candidate_t *cptr;

	for (i = 0; i < count; i++) {

		ACSD_DEBUG("candi->chspec: 0x%x\n", candi->chspec);

		cptr = (ch_candidate_t *)*buf;

		/* Copy over all fields, converting to network byte order if needed */
		cptr->chspec = htons(candi->chspec);
		cptr->valid = candi->valid;
		cptr->is_dfs = candi->is_dfs;
		cptr->reason = htons(candi->reason);

		if (candi->valid) {
			for (j = 0; j < CH_SCORE_MAX; j++) {
				cptr->chscore[j].score = htonl(candi->chscore[j].score);
				cptr->chscore[j].weight = htonl(candi->chscore[j].weight);
			}
		}

		*r_size += sizeof(ch_candidate_t);
		*buf += sizeof(ch_candidate_t);
		totsize += sizeof(ch_candidate_t);
		candi ++;
	}

	return totsize;
}

static const char *
acs_policy_name(acs_policy_index i)
{
	switch (i) {
		case ACS_POLICY_DEFAULT2G:	return "Default2g";
		case ACS_POLICY_DEFAULT5G:	return "Default5g";
		case ACS_POLICY_CUSTOMIZED1:	return "Customized1";
		case ACS_POLICY_USER:		return "User";
		/* No default so compiler will complain if new definitions are missing */
	}
	return "(unknown)";
}

/* processes DFS move/preclear related commands including
 *	test_dfsr, test_preclear,
 *	zdfs_5g_move, zdfs_5g_preclear,
 *	zdfs_2g_move, zdfs_2g_preclear,
 */
static int
acsd_dfs_cmd(acs_chaninfo_t *c_info, char *buf, char *param, char *val, uint *r_size,
		bool move, acs_cac_mode_t cac_mode)
{
	int ret = BCME_OK;
	acs_chaninfo_t *zdfs_2g_ci = NULL;
	chanspec_t chspec = wf_chspec_aton(val);

	/* request must come from a 5GHz interface only */
	if (!BAND_5G(c_info->rs_info.band_type)) {
		*r_size = sprintf(buf, "ERR: not a 5Hz interface");
		return BCME_BADBAND;
	}
	if (!move && !c_info->country_is_edcrs_eu) {
		*r_size = sprintf(buf, "ERR: Country code does not support preclearance");
		return BCME_UNSUPPORTED;
	}
	if (!move || cac_mode == ACS_CAC_MODE_ZDFS_5G_ONLY ||
			cac_mode == ACS_CAC_MODE_ZDFS_2G_ONLY) {
		/* ensure bgdfs is enabled */
		if (!c_info->acs_bgdfs) {
			*r_size = sprintf(buf, "ERR: ZDFS not enabled");
			return BCME_NOTENABLED;
		}
	}
	if (cac_mode == ACS_CAC_MODE_ZDFS_5G_ONLY &&
			c_info->acs_bgdfs->state != BGDFS_STATE_IDLE) {
		/* avoid ZDFS if the 5Hz interface is busy with ZDFS */
		if (c_info->acs_bgdfs->state != BGDFS_STATE_IDLE) {
			*r_size = sprintf(buf, "ERR: ZDFS_5G is not idle on this interface");
			return BCME_BUSY;
		}
	}
	if (ACS_CAC_MODE_ZDFS_2G_ONLY == cac_mode) {
		if ((zdfs_2g_ci = acs_get_zdfs_2g_ci()) == NULL || !zdfs_2g_ci->acs_bgdfs) {
			*r_size = sprintf(buf, "ERR: ZDFS_2G is not available");
			return BCME_NOTENABLED;
		}
		if (zdfs_2g_ci->acs_bgdfs->state != BGDFS_STATE_IDLE) {
			*r_size = sprintf(buf, "ERR: ZDFS_2G is not idle");
			return BCME_BUSY;
		}
	}

	c_info->cac_mode = cac_mode;
	if (move) {
		c_info->selected_chspec = chspec;
		acs_set_chspec(c_info, FALSE, WL_CHAN_REASON_DFS_AP_MOVE_START);
		c_info->switch_reason_type = APCS_DFS_REENTRY;
	} else { /* preclear */
		c_info->acs_bgdfs->next_scan_chan = chspec;
		ret = acs_bgdfs_ahead_trigger_scan(c_info);
	}

	*r_size = sprintf(buf, "%s chanspec: %7s (status:%d)", param, val, ret);

	return ret;
}

/* buf should be null terminated. rcount doesn;t include the terminuating null */
int
acsd_proc_cmd(acsd_wksp_t* d_info, char* buf, uint rcount, uint* r_size)
{
	char *c, *data;
	int err = 0, ret;
	char ifname[IFNAMSIZ];

	/* Check if we have command and data in the expected order */
	if (!(c = strchr(buf, '&'))) {
		ACSD_ERROR("Missing Command\n");
		err = -1;
		goto done;
	}
	*c++ = '\0';
	data = c;

	if (!strcmp(buf, "info")) {
		time_t ltime;
		int i;
		const char *mode_str[] = {"disabled", "monitor", "auto", "coex", "11h",
		       "fixchspec"};
		d_info->stats.valid_cmds++;

		time(&ltime);
		*r_size = sprintf(buf, "time: %s \n", ctime(&ltime));
		*r_size += sprintf(buf+ *r_size, "acsd version: %d\n", d_info->version);
		*r_size += sprintf(buf+ *r_size, "acsd ticks: %d\n", d_info->ticks);
		*r_size += sprintf(buf+ *r_size, "acsd poll_interval: %d seconds\n",
			d_info->poll_interval);

		for (i = 0; i < ACS_MAX_IF_NUM; i++) {
			acs_chaninfo_t *c_info = NULL;

			c_info = d_info->acs_info->chan_info[i];

			if (!c_info)
				continue;

			*r_size += sprintf(buf+ *r_size, "ifname: %s, mode: %s\n",
				c_info->name, mode_str[c_info->mode]);
		}

		goto done;
	}

	if (!strcmp(buf, "csscan") || (!strcmp(buf, "autochannel"))) {
		acs_chaninfo_t *c_info = NULL;
		int index;
		bool pick = FALSE;

		d_info->stats.valid_cmds++;
		pick = !strcmp(buf, "autochannel");

		if ((ret = acsd_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ACSD_DEBUG("cmd: %s, data: %s, ifname: %s\n",
			buf, data, ifname);

		index = acs_idx_from_map(ifname);
		if (index != -1) {
			c_info = d_info->acs_info->chan_info[index];
		}

		if (!c_info) {
			*r_size = sprintf(buf, "Request not permitted: "
				"Interface was not intialized properly");
			goto done;
		}

		if (!AUTOCHANNEL(c_info)) {
			*r_size = sprintf(buf, "Request not permitted: "
				"Interface is not in autochannel mode");
			goto done;
		}

		if (c_info->acs_bgdfs != NULL && c_info->acs_bgdfs->state != BGDFS_STATE_IDLE) {
			*r_size = sprintf(buf, "Request not permitted: "
				"BGDFS in progress");
			goto done;
		}

		err = acs_run_cs_scan(c_info);
		if (err) {
			ACSD_ERROR("ifname: %s scan is failed due to: %d\n", c_info->name, err);
			return err;
		}

		acs_cleanup_scan_entry(c_info);
		err = acs_request_data(c_info);

		if (pick) {
			c_info->autochannel_through_cli = TRUE;
			acs_select_chspec(c_info);
			if (c_info->acs_use_csa) {
				err = acs_csa_handle_request(c_info);
			} else {
				err = acs_set_chanspec(c_info, c_info->selected_chspec);
			}
			c_info->autochannel_through_cli = FALSE;
			if (!err) {
				chanim_upd_acs_record(c_info->chanim_info,
						c_info->selected_chspec, APCS_IOCTL);
			}
		}

		*r_size = sprintf(buf, "Request finished");
		goto done;
	}

	if (!strcmp(buf, "dump")) {
		char param[128];
		int index;
		acs_chaninfo_t *c_info = NULL;

		if ((ret = acsd_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if ((ret = acsd_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ACSD_DEBUG("cmd: %s, data: %s, param: %s, ifname: %s\n",
			buf, data, param, ifname);

		index = acs_idx_from_map(ifname);

		ACSD_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->acs_info->chan_info[index];

		ACSD_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested info not available");
			goto done;
		}

		d_info->stats.valid_cmds++;
		if (!strcmp(param, "help")) {
			*r_size = sprintf(buf,
				"dump: acs_record acsd_stats bss candidate chanim cscore"
				" dfsr scanresults");
		}
		else if (!strcmp(param, "dfsr")) {
			*r_size = acs_dfsr_dump(ACS_DFSR_CTX(c_info), buf, ACSD_BUFSIZE_4K);
		}
		else if (!strcmp(param, "chanim")) {
			wl_chanim_stats_t * chanim_stats = c_info->chanim_stats;
			wl_chanim_stats_t tmp_stats = {0};
			int count;
			int i;
			chanim_stats_t *stats;
			chanim_stats_v2_t *statsv2;
			chanim_stats_t loc;
			chanim_stats_v2_t loc2;

			if (!chanim_stats) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			count = chanim_stats->count;
			tmp_stats.version = htonl(chanim_stats->version);
			tmp_stats.buflen = htonl(chanim_stats->buflen);
			tmp_stats.count = htonl(chanim_stats->count);

			memcpy((void*)buf, (void*)&tmp_stats, WL_CHANIM_STATS_FIXED_LEN);
			*r_size = WL_CHANIM_STATS_FIXED_LEN;
			buf += *r_size;
			if (chanim_stats->version == WL_CHANIM_STATS_VERSION) {
				stats = (chanim_stats_t *)chanim_stats->stats;
				for (i = 0; i < count; i++) {
					memcpy((void*)&loc, (void*)stats, sizeof(chanim_stats_t));
					loc.glitchcnt = htonl(stats->glitchcnt);
					loc.badplcp = htonl(stats->badplcp);
					loc.chanspec = htons(stats->chanspec);
					loc.timestamp = htonl(stats->timestamp);

					memcpy((void*) buf, (void*)&loc, sizeof(chanim_stats_t));
					*r_size += sizeof(chanim_stats_t);
					buf += sizeof(chanim_stats_t);
					stats++;
				}
			} else if (chanim_stats->version == WL_CHANIM_STATS_V2) {
				statsv2 = (chanim_stats_v2_t *)chanim_stats->stats;
				for (i = 0; i < count; i++) {
					memcpy((void*)&loc2, (void*)statsv2,
						sizeof(chanim_stats_v2_t));
					loc2.glitchcnt = htonl(statsv2->glitchcnt);
					loc2.badplcp = htonl(statsv2->badplcp);
					loc2.chanspec = htons(statsv2->chanspec);
					loc2.timestamp = htonl(statsv2->timestamp);

					memcpy((void*) buf, (void*)&loc2,
						sizeof(chanim_stats_v2_t));
					*r_size += sizeof(chanim_stats_v2_t);
					buf += sizeof(chanim_stats_v2_t);
					statsv2++;
				}
			}
		} else if (!strcmp(param, "candidate") || !strcmp(param, "cscore")) {
			ch_candidate_t * candi[ACS_BW_MAX];
			int i;
			bool is_null = TRUE;

			for (i = ACS_BW_20; i < ACS_BW_MAX; i++) {
				candi[i] = c_info->candidate[i];

				if (!candi[i]) {
					continue;
				}

				if (i == ACS_BW_160) {
					ACSD_INFO("160 MHz candidates: count %d\n",
							c_info->c_count[i]);
				} else if (i == ACS_BW_8080) {
					ACSD_INFO("80p80 MHz candidates: count %d\n",
							c_info->c_count[i]);
				} else if (i == ACS_BW_80) {
					ACSD_INFO("80 MHz candidates: count %d\n",
							c_info->c_count[i]);
				} else if (i == ACS_BW_40) {
					ACSD_INFO("40 MHz candidates: count %d\n",
							c_info->c_count[i]);
				} else {
					ACSD_INFO("20 MHz candidates: count %d\n",
							c_info->c_count[i]);
				}

				is_null = FALSE;
				acsd_pass_candi(candi[i], c_info->c_count[i], &buf, r_size);
			}

			if (is_null) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

		} else if (!strcmp(param, "bss")) {
			acs_chan_bssinfo_t *bssinfo = c_info->ch_bssinfo;

			if (!bssinfo) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			memcpy((void*)buf, (void*)bssinfo, sizeof(acs_chan_bssinfo_t) *
				c_info->scan_chspec_list.count);
			*r_size = sizeof(acs_chan_bssinfo_t) * c_info->scan_chspec_list.count;

		} else if (!strcmp(param, "acs_record")) {
			chanim_info_t * ch_info = c_info->chanim_info;
			chanim_acs_record_t record;
			uint8 idx;
			int i, count = 0;

			if (!ch_info) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			idx = chanim_mark(ch_info).record_idx;

			for (i = 0; i < ACS_CHANIM_ACS_RECORD; i++) {
				if (ch_info->record[idx].valid) {
					bcopy(&ch_info->record[idx], &record,
						sizeof(chanim_acs_record_t));
					record.selected_chspc =
						htons(record.selected_chspc);
					record.glitch_cnt =
						htonl(record.glitch_cnt);
					record.timestamp =
						htonl(record.timestamp);

					memcpy((void*) buf, (void*)&record,
						sizeof(chanim_acs_record_t));
					*r_size += sizeof(chanim_acs_record_t);
					buf += sizeof(chanim_acs_record_t);

					count++;
					ACSD_DEBUG("count: %d trigger: %d\n",
						count, record.trigger);
				}
				idx = (idx + 1) % ACS_CHANIM_ACS_RECORD;
			}

			ACSD_DEBUG("rsize: %d, sizeof: %zd\n", *r_size,
					sizeof(chanim_acs_record_t));

		} else if (!strcmp(param, "acsd_stats")) {
			acsd_stats_t * d_stats = &d_info->stats;

			if (!d_stats) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			*r_size = sprintf(buf, "ACSD stats:\n");
			*r_size += sprintf(buf + *r_size, "Total cmd: %d, Valid cmd: %d\n",
				d_stats->num_cmds, d_stats->valid_cmds);
			*r_size += sprintf(buf + *r_size, "Total events: %d, Valid events: %d\n",
				d_stats->num_events, d_stats->valid_events);

			goto done;

		} else if (!strcmp(param, "scanresults")) {

			acs_dump_scan_entry(c_info);
		} else {
			*r_size = sprintf(buf, "Unsupported dump command (try \"dump help\")");
		}
		goto done;
	}

	if (!strcmp(buf, "get")) {
		char param[128];
		int index;
		acs_chaninfo_t *c_info = NULL;

		if ((ret = acsd_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if ((ret = acsd_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ACSD_DEBUG("cmd: %s, data: %s, param: %s, ifname: %s\n",
			buf, data, param, ifname);

		index = acs_idx_from_map(ifname);

		ACSD_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->acs_info->chan_info[index];

		ACSD_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested info not available");
			goto done;
		}

		d_info->stats.valid_cmds++;
		if (!strcmp(param, "msglevel")) {
			*r_size = sprintf(buf, "%d", acsd_debug_level);
			goto done;
		}

		if (!strcmp(param, "mode")) {
			const char *mode_str[] = {"disabled", "monitor", "select", "coex",
				"11h", "fixchspec"};
			int acs_mode = c_info->mode;
			*r_size = sprintf(buf, "%d: %s", acs_mode, mode_str[acs_mode]);
			goto done;
		}

		if (!strcmp(param, "acs_cs_scan_timer")) {
			if (c_info->acs_cs_scan_timer)
				*r_size = sprintf(buf, "%d sec", c_info->acs_cs_scan_timer);
			else
				*r_size = sprintf(buf, "OFF");
			goto done;
		}

		if (!strcmp(param, "acs_policy")) {
			*r_size = sprintf(buf, "index: %d : %s", c_info->policy_index,
				acs_policy_name(c_info->policy_index));
			goto done;
		}

		if (!strcmp(param, "acs_flags")) {
			*r_size = sprintf(buf, "0x%x", c_info->flags);
			goto done;
		}

		if (!strcmp(param, "chanim_flags")) {
			chanim_info_t* ch_info = c_info->chanim_info;
			if (ch_info)
				*r_size = sprintf(buf, "0x%x", chanim_config(ch_info).flags);
			else
				*r_size = sprintf(buf, "ERR: Requested info not available");
			goto done;
		}

		if (!strcmp(param, "acs_tx_idle_cnt")) {
			*r_size = sprintf(buf, "%d tx packets", c_info->acs_tx_idle_cnt);
			goto done;
		}

		if (!strcmp(param, "acs_ci_scan_timeout")) {
			*r_size = sprintf(buf, "%d sec", c_info->acs_ci_scan_timeout);
			goto done;
		}

		if (!strcmp(param, "acs_far_sta_rssi")) {
			*r_size = sprintf(buf, "%d", c_info->acs_far_sta_rssi);
			goto done;
		}
		if (!strcmp(param, "acs_nofcs_least_rssi")) {
			*r_size = sprintf(buf, "%d", c_info->acs_nofcs_least_rssi);
			goto done;
		}
		if (!strcmp(param, "acs_scan_chanim_stats")) {
			*r_size = sprintf(buf, "%d", c_info->acs_scan_chanim_stats);
			goto done;
		}
		if (!strcmp(param, "acs_ci_scan_chanim_stats")) {
			*r_size = sprintf(buf, "%d", c_info->acs_ci_scan_chanim_stats);
			goto done;
		}
		if (!strcmp(param, "acs_chan_dwell_time")) {
			*r_size = sprintf(buf, "%d", c_info->acs_chan_dwell_time);
			goto done;
		}
		if (!strcmp(param, "acs_chan_flop_period")) {
			*r_size = sprintf(buf, "%d", c_info->acs_chan_flop_period);
			goto done;
		}
		if (!strcmp(param, "acs_dfs")) {
			*r_size = sprintf(buf, "%d", c_info->acs_dfs);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_period")) {
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_period);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_cnt")) {
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_cnt);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_ratio")) {
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_ratio);
			goto done;
		}
		if (!strcmp(param, "acs_scan_entry_expire")) {
			*r_size = sprintf(buf, "%d sec", c_info->acs_scan_entry_expire);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_TEST_DFSR) || !strcmp(param, ACS_CMD_ZDFS_2G_MOVE) ||
				!strcmp(param, ACS_CMD_ZDFS_5G_MOVE)) {
			char ch_str[CHANSPEC_STR_LEN];
			*r_size = sprintf(buf, "chanspec: %7s (0x%04x) for dfsr (CAC mode:%d)",
					wf_chspec_ntoa(c_info->selected_chspec, ch_str),
					c_info->selected_chspec, c_info->cac_mode);
			goto done;
		}
		if ((!strcmp(param, ACS_CMD_TEST_PRECLEAR) ||
				!strcmp(param, ACS_CMD_ZDFS_2G_PRECLEAR) ||
				!strcmp(param, ACS_CMD_ZDFS_5G_PRECLEAR)) &&
				c_info->acs_bgdfs != NULL) {
			char ch_str[CHANSPEC_STR_LEN];
			*r_size = sprintf(buf, "chanspec: %7s (0x%04x) to preclear (CAC mode:%d)",
					wf_chspec_ntoa(c_info->acs_bgdfs->next_scan_chan, ch_str),
					c_info->acs_bgdfs->next_scan_chan, c_info->cac_mode);
			goto done;
		}

		if (!strcmp(param, "acs_switch_score_thresh")) {
			*r_size = sprintf(buf, "%d", c_info->acs_switch_score_thresh);
			goto done;
		}

		if (!strcmp(param, "acs_switch_score_thresh_hi")) {
			*r_size = sprintf(buf, "%d", c_info->acs_switch_score_thresh_hi);
			goto done;
		}

		if (!strcmp(param, "acs_txop_limit_hi")) {
			*r_size = sprintf(buf, "%d", c_info->acs_txop_limit_hi);
			goto done;
		}

		if (!strcmp(param, "acs_ci_scan_timer")) {
			*r_size = sprintf(buf, "%d sec", c_info->acs_ci_scan_timer);
			goto done;
		}

		if (!strcmp(param, "bw_upgradable")) {
			*r_size = sprintf(buf, "%d", c_info->bw_upgradable);
			goto done;
		}

		if (!strcmp(param, "fallback_to_primary")) {
			*r_size = sprintf(buf, "%d", c_info->fallback_to_primary);
			goto done;
		}

		if (!strcmp(param, "ci_scan_txop_limit")) {
			*r_size = sprintf(buf, "%d", c_info->ci_scan_txop_limit);
			goto done;
		}

		if (!strcmp(param, "acs_txop_limit")) {
			*r_size = sprintf(buf, "%d", c_info->acs_txop_limit);
			goto done;
		}

		CHANIM_GET(max_acs, "");
		CHANIM_GET(lockout_period, "sec");
		CHANIM_GET(sample_period, "sec");
		CHANIM_GET(threshold_time, "sample period");
		CHANIM_GET(acs_trigger_var, "dBm");

		*r_size = sprintf(buf, "GET: Unknown variable \"%s\".", param);
		err = -1;
		goto done;
	}

	if (!strcmp(buf, "set")) {
		char param[128];
		char val[16];
		int setval = 0;
		int index;
		acs_chaninfo_t *c_info = NULL;

		if ((ret = acsd_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if ((ret = acsd_extract_token_val(data, "val", val, sizeof(val))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing val");
			goto done;
		}

		setval = atoi(val);

		if ((ret = acsd_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		index = acs_idx_from_map(ifname);

		ACSD_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->acs_info->chan_info[index];

		ACSD_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested ifname not available");
			goto done;
		}

		ACSD_DEBUG("cmd: %s, data: %s, param: %s val: %d, ifname: %s\n",
			buf, data, param, setval, ifname);

		d_info->stats.valid_cmds++;

		if (c_info->acs_bgdfs != NULL && c_info->acs_bgdfs->state != BGDFS_STATE_IDLE &&
			(strcmp(param, "chanspec"))) {
			*r_size = sprintf(buf, "Request not permitted: "
				"BGDFS in progress");
			goto done;
		}

		if (!strcmp(param, "msglevel")) {
			acsd_debug_level = setval;
			*r_size = sprintf(buf, "%d", acsd_debug_level);
			goto done;
		}

		if (!strcmp(param, "mode")) {
			const char *mode_str[] = {"disabled", "monitor", "select", "coex",
				"11h", "fixchspec"};

			if (setval < ACS_MODE_DISABLE || setval > ACS_MODE_FIXCHSPEC) {
				*r_size = sprintf(buf, "Out of range");
				goto done;
			}

			c_info->mode = setval;
			*r_size = sprintf(buf, "%d: %s", setval, mode_str[setval]);
			ACSD_DEBUG("Setting ACSD mode = %d: %s\n", setval, mode_str[setval]);
			goto done;
		}

		if (!strcmp(param, "acs_cs_scan_timer")) {
			if (setval != 0 && setval < ACS_CS_SCAN_TIMER_MIN) {
				*r_size = sprintf(buf, "Out of range");
				goto done;
			}

			c_info->acs_cs_scan_timer = setval;

			if (setval)
				*r_size = sprintf(buf, "%d sec", c_info->acs_cs_scan_timer);
			else
				*r_size = sprintf(buf, "OFF");
			goto done;
		}

		if (!strcmp(param, "acs_policy")) {
			if (setval > ACS_POLICY_MAX || setval < 0)  {
				*r_size = sprintf(buf, "Out of range");
				goto done;
			}

			c_info->policy_index = setval;

			if (setval != ACS_POLICY_USER)
				acs_default_policy(&c_info->acs_policy, setval);

			*r_size = sprintf(buf, "index: %d : %s", setval, acs_policy_name(setval));
			goto done;
		}

		if (!strcmp(param, "acs_flags")) {

			c_info->flags = setval;

			*r_size = sprintf(buf, "flags: 0x%x", c_info->flags);
			goto done;
		}

		if (!strcmp(param, "chanim_flags")) {
			chanim_info_t* ch_info = c_info->chanim_info;
			if (!ch_info) {
				*r_size = sprintf(buf, "ERR: set action not successful");
				goto done;
			}
			chanim_config(ch_info).flags = setval;
			*r_size = sprintf(buf, "0x%x", chanim_config(ch_info).flags);
			goto done;

		}

		if (!strcmp(param, "acs_tx_idle_cnt")) {
			c_info->acs_tx_idle_cnt = setval;
			*r_size = sprintf(buf, "%d tx packets", c_info->acs_tx_idle_cnt);
			goto done;
		}

		if (!strcmp(param, "acs_ci_scan_timeout")) {
			c_info->acs_ci_scan_timeout = setval;
			*r_size = sprintf(buf, "%d sec", c_info->acs_ci_scan_timeout);
			goto done;
		}

		if (!strcmp(param, "acs_far_sta_rssi")) {
			c_info->acs_far_sta_rssi = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_far_sta_rssi);
			goto done;
		}
		if (!strcmp(param, "acs_nofcs_least_rssi")) {
			c_info->acs_nofcs_least_rssi = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_nofcs_least_rssi);
			goto done;
		}
		if (!strcmp(param, "acs_scan_chanim_stats")) {
			c_info->acs_scan_chanim_stats = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_scan_chanim_stats);
			goto done;
		}
		if (!strcmp(param, "acs_ci_scan_chanim_stats")) {
			c_info->acs_ci_scan_chanim_stats = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_ci_scan_chanim_stats);
			goto done;
		}
		if (!strcmp(param, "acs_chan_dwell_time")) {
			c_info->acs_chan_dwell_time = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_chan_dwell_time);
			goto done;
		}
		if (!strcmp(param, "acs_chan_flop_period")) {
			c_info->acs_chan_flop_period = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_chan_flop_period);
			goto done;
		}
		if (!strcmp(param, "acs_dfs")) {
			c_info->acs_dfs = setval;
			acs_dfsr_enable(ACS_DFSR_CTX(c_info), (setval == ACS_DFS_REENTRY));
			*r_size = sprintf(buf, "%d", c_info->acs_dfs);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_period")) {
			c_info->acs_txdelay_period = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_period);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_cnt")) {
			c_info->acs_txdelay_cnt = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_cnt);
			goto done;
		}
		if (!strcmp(param, "acs_txdelay_ratio")) {
			c_info->acs_txdelay_ratio = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_txdelay_ratio);
			goto done;
		}
		if (!strcmp(param, "acs_scan_entry_expire")) {
			c_info->acs_scan_entry_expire = setval;
			*r_size = sprintf(buf, "%d sec", c_info->acs_scan_entry_expire);
			goto done;
		}
		if (!strcmp(param, "acs_ci_scan_timer")) {
			c_info->acs_ci_scan_timer = setval;
			*r_size = sprintf(buf, "%d sec", c_info->acs_ci_scan_timer);
			goto done;
		}

		if (!strcmp(param, "ci_scan_txop_limit")) {
			c_info->ci_scan_txop_limit = setval;
			*r_size = sprintf(buf, "%d", c_info->ci_scan_txop_limit);
			goto done;
		}

		if (!strcmp(param, "acs_txop_limit")) {
			c_info->acs_txop_limit = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_txop_limit);
			goto done;
		}

		if (!strcmp(param, "acs_switch_score_thresh")) {
			c_info->acs_switch_score_thresh = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_switch_score_thresh);
			goto done;
		}

		if (!strcmp(param, "acs_switch_score_thresh_hi")) {
			c_info->acs_switch_score_thresh_hi = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_switch_score_thresh_hi);
			goto done;
		}

		if (!strcmp(param, "acs_txop_limit_hi")) {
			c_info->acs_txop_limit_hi = setval;
			*r_size = sprintf(buf, "%d", c_info->acs_txop_limit_hi);
			goto done;
		}

		if (!strcmp(param, ACS_CMD_TEST_DFSR)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, TRUE,
					ACS_CAC_MODE_AUTO);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_TEST_PRECLEAR)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, FALSE,
					ACS_CAC_MODE_AUTO);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_ZDFS_5G_MOVE)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, TRUE,
					ACS_CAC_MODE_ZDFS_5G_ONLY);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_ZDFS_2G_MOVE)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, TRUE,
					ACS_CAC_MODE_ZDFS_2G_ONLY);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_ZDFS_5G_PRECLEAR)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, FALSE,
					ACS_CAC_MODE_ZDFS_5G_ONLY);
			goto done;
		}
		if (!strcmp(param, ACS_CMD_ZDFS_2G_PRECLEAR)) {
			acsd_dfs_cmd(c_info, buf, param, val, r_size, FALSE,
					ACS_CAC_MODE_ZDFS_2G_ONLY);
			goto done;
		}

		if (!strcmp(param, "chanspec")) {
			unsigned int tmp;
			char option[16] = {0};
			int do_dfs_ap_move = FALSE;

			if ((ret = acsd_extract_token_val(data, "option", option,
				sizeof(option))) < 0) {
				*r_size = sprintf(buf, "Request failed: missing option");
				goto done;
			}
			sscanf(val, "%d", &tmp);
			sscanf(option, "%d", &do_dfs_ap_move);
			acs_process_cmd(c_info, (chanspec_t)tmp, do_dfs_ap_move);
			goto done;
		}
		CHANIM_SET(max_acs, "", uint8);
		CHANIM_SET(acs_trigger_var, "dBm", int8);
		CHANIM_SET(lockout_period, "sec", uint32);
		CHANIM_SET(sample_period, "sec", uint8);
		CHANIM_SET(threshold_time, "sample period", uint8);

		*r_size = sprintf(buf, "SET: Unknown variable \"%s\".", param);
		err = -1;
		goto done;
	}
done:
	return err;
}
