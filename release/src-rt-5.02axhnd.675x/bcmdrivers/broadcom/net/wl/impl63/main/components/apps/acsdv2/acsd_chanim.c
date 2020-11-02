/*
 * ACS deamon chanim module (Linux)
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
 * $Id: acsd_chanim.c 781115 2019-11-12 05:40:05Z $
 */

#include "acsd_svr.h"

#define ACS_CHANIM_BUF_LEN (2*1024)

#ifdef ACSD_SEGMENT_CHANIM
static uint8 acsd_get_segment_rep(acs_chaninfo_t * c_info, uint8 val);
#endif /* ACSD_SEGMENT_CHANIM */

static void
chanim_config_init(chanim_info_t * ch_info)
{
	char* intf_threshold;
	int trigger;

	ch_info->config.flags = CHANIM_FLAGS_RELATIVE_THRES;

	ch_info->config.sample_period = CHANIM_DFLT_SAMPLE_PERIOD;
	ch_info->config.threshold_time = CHANIM_DFLT_THRESHOLD_TIME;
	ch_info->config.max_acs = CHANIM_DFLT_MAX_ACS;
	ch_info->config.lockout_period = CHANIM_DFLT_LOCKOUT_PERIOD;

	ch_info->config.scb_max_probe = CHANIM_DFLT_SCB_MAX_PROBE;
	ch_info->config.scb_timeout = CHANIM_DFLT_SCB_TIMEOUT;
	ch_info->config.scb_activity_time = CHANIM_DFLT_SCB_ACTIVITY_TIME;

	ch_info->config.acs_trigger_var = 40;

	intf_threshold = nvram_get("acs_def_plcy_intf_threshold");
	if (intf_threshold) {
		trigger = atoi(intf_threshold);
		if (trigger >= 0)
			ch_info->config.acs_trigger_var = trigger;
	}

	/* range set up */
	chanim_range(ch_info).sample_period.min_val =
		CHANIM_SAMPLE_PERIOD_MIN;
	chanim_range(ch_info).sample_period.max_val =
		CHANIM_SAMPLE_PERIOD_MAX;
	chanim_range(ch_info).threshold_time.min_val =
		CHANIM_THRESHOLD_TIME_MIN;
	chanim_range(ch_info).threshold_time.max_val =
		CHANIM_THRESHOLD_TIME_MAX;
	chanim_range(ch_info).max_acs.min_val =
		CHANIM_MAX_ACS_MIN;
	chanim_range(ch_info).max_acs.max_val =
		CHANIM_MAX_ACS_MAX;
	chanim_range(ch_info).lockout_period.min_val =
		CHANIM_LOCKOUT_PERIOD_MIN;
	chanim_range(ch_info).lockout_period.max_val =
		CHANIM_LOCKOUT_PERIOD_MAX;

	chanim_range(ch_info).acs_trigger_var.min_val = 1;
	chanim_range(ch_info).acs_trigger_var.max_val =	50;

	/* if some driver configeration is needed, add here */
}

int
acsd_chanim_init(acs_chaninfo_t *c_info)
{
	chanim_info_t * ch_info;
	int ret;

	ch_info = (chanim_info_t *) acsd_malloc(sizeof(chanim_info_t));
	chanim_config_init(ch_info);

	ret = acs_set_chanim_sample_period(c_info->name, chanim_config(ch_info).sample_period);

	if (ret < 0) {
		ACSD_ERROR("%s: failed to set chanim_sample_period", c_info->name);
	}

	chanim_mark(ch_info).wl_sample_period =
		chanim_config(ch_info).sample_period;

	ret = acs_set_noise_metric(c_info->name, NOISE_MEASURE_KNOISE);

	if (ret < 0) {
		ACSD_ERROR("%s: failed to set noise metric", c_info->name);
	}

	ACS_FREE(c_info->chanim_info);
	c_info->chanim_info = ch_info;

	return ret;
}

#ifdef DEBUG
static void
acsd_dump_chanim(wl_chanim_stats_t* chanim_stats)
{
	int count = dtoh32(chanim_stats->count);
	int i, j;
	if (chanim_stats->version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t *stats;

		printf("Chanim Stats Dump: count: %d\n", count);
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze     txop     "
				"goodtx  badtx   glitch   badplcp  knoise  timestamp\n");

		for (i = 0; i < count; i++) {
			stats = (chanim_stats_t *)&chanim_stats->stats[i];
			printf("0x%4x\t", stats->chanspec);
			for (j = 0; j < CCASTATS_MAX; j++)
				printf("%d\t", stats->ccastats[j]);
			printf("%d\t%d\t%d\t%d", dtoh32(stats->glitchcnt), dtoh32(stats->badplcp),
					stats->bgnoise, dtoh32(stats->timestamp));
			printf("\n");
		}
	} else if (chanim_stats->version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t *stats;

		printf("Chanim Stats Dump: count: %d\n", count);
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze     txop     "
				"goodtx  badtx   glitch   badplcp  knoise  timestamp\n");

		stats = (chanim_stats_v2_t *)chanim_stats->stats;
		for (i = 0; i < count; i++) {
			printf("0x%4x\t", stats->chanspec);
			for (j = 0; j < CCASTATS_V2_MAX; j++)
				printf("%d\t", stats->ccastats[j]);
			printf("%d\t%d\t%d\t%d", dtoh32(stats->glitchcnt), dtoh32(stats->badplcp),
					stats->bgnoise, dtoh32(stats->timestamp));
			printf("\n");
			stats++;
		}
	}
}
#endif /* DEBUG */

static void
acsd_display_chanim(wl_chanim_stats_t* chanim_stats)
{
	int j;
	if (chanim_stats->version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t *stats = (chanim_stats_t *)chanim_stats->stats;
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze     txop     "
				"goodtx  badtx   glitch   badplcp  knoise  timestamp\n");
		printf("0x%4x\t", stats->chanspec);
		for (j = 0; j < CCASTATS_MAX; j++)
			printf("%d\t", stats->ccastats[j]);
		printf("%d\t%d\t%d\t%d", dtoh32(stats->glitchcnt), dtoh32(stats->badplcp),
				stats->bgnoise, dtoh32(stats->timestamp));
		printf("\n");
	} else if (chanim_stats->version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t *stats = (chanim_stats_v2_t *)chanim_stats->stats;
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze     txop     "
				"goodtx  badtx   glitch   badplcp  knoise  timestamp\n");
		printf("0x%4x\t", stats->chanspec);
		for (j = 0; j < CCASTATS_V2_MAX; j++)
			printf("%d\t", stats->ccastats[j]);
		printf("%d\t%d\t%d\t%d", dtoh32(stats->glitchcnt), dtoh32(stats->badplcp),
				stats->bgnoise, dtoh32(stats->timestamp));
		printf("\n");
	}
}

static bool
chanim_intf_detected(chanim_info_t *ch_info, uint8 version)
{
	bool detected = FALSE;
	chanim_stats_t* latest = NULL;
	chanim_stats_v2_t *latestv2 = NULL;
	chanim_config_t* config = &ch_info->config;

	bool noise_detect = FALSE;
	int score = 0;
	if (version == WL_CHANIM_STATS_V2) {
		latestv2 = (chanim_stats_v2_t *)&ch_info->stats[chanim_mark(ch_info).stats_idx];
		/* if the detection is not supported, return false */
		if (latestv2->bgnoise == 0)
			return detected;

		score = latestv2->bgnoise;
		score += chanim_txop_to_noise(latestv2->chan_idle);

	} else if (version == WL_CHANIM_STATS_VERSION) {
		latest = (chanim_stats_t *)&ch_info->stats[chanim_mark(ch_info).stats_idx];
		/* if the detection is not supported, return false */
		if (latest->bgnoise == 0)
			return detected;

		score = latest->bgnoise;
		score += chanim_txop_to_noise(latest->chan_idle);

	}

	if (score < chanim_mark(ch_info).best_score)
		chanim_mark(ch_info).best_score = score;

	if (acsd_debug_level & ACSD_DEBUG_CHANIM)
		printf("acs monitor: score = %d (ref %d)\n",
			score, chanim_mark(ch_info).best_score);

	if ((score - chanim_mark(ch_info).best_score) >= config->acs_trigger_var)
		noise_detect = TRUE;

	if (version == WL_CHANIM_STATS_V2) {
		if (noise_detect && (latestv2->ccastats[CCASTATS_INBSS] < 2)) {
			detected = TRUE;
		}
	} else if (version == WL_CHANIM_STATS_VERSION) {

		if (noise_detect && (latest->ccastats[CCASTATS_INBSS] < 2)) {
			detected = TRUE;
		}
	}
	return detected;
}

/* start to poll the scbs frequently */
static int
chanim_speedup_scbprobe(acs_chaninfo_t * c_info)
{
	chanim_info_t * ch_info = c_info->chanim_info;
	wl_scb_probe_t scb_probe;
	int ret = 0;

	/* get the current setting from the driver */
	ret = acs_get_scb_probe(c_info->name, &scb_probe, sizeof(wl_scb_probe_t));
	ACS_ERR(ret, "failed to get scb_probe results");

	ACSD_DEBUG("%s: scb_probe: scb_timeout: %d, scb_max_probe: %d, scb_activity_time: %d\n",
		c_info->name,
		scb_probe.scb_timeout, scb_probe.scb_max_probe, scb_probe.scb_activity_time);

	chanim_mark(ch_info).scb_timeout = dtoh32(scb_probe.scb_timeout);
	chanim_mark(ch_info).scb_max_probe = dtoh32(scb_probe.scb_max_probe);
	chanim_mark(ch_info).scb_activity_time = dtoh32(scb_probe.scb_activity_time);

	/* set with the chanim setting */
	scb_probe.scb_timeout = htod32(chanim_config(ch_info).scb_timeout);
	scb_probe.scb_max_probe = htod32(chanim_config(ch_info).scb_max_probe);
	scb_probe.scb_activity_time = htod32(chanim_config(ch_info).scb_activity_time);

	/* get the current setting from the driver */
	ret = acs_set_scb_probe(c_info->name, &scb_probe, sizeof(wl_scb_probe_t));
	ACS_ERR(ret, "failed to set scb_probe results");

	return ret;
}

static int
chanim_restore_scbprobe(acs_chaninfo_t * c_info)
{
	chanim_info_t * ch_info = c_info->chanim_info;
	wl_scb_probe_t scb_probe;
	int ret = 0;

	/* set with the stored driver setting */
	scb_probe.scb_timeout = htod32(chanim_mark(ch_info).scb_timeout);
	scb_probe.scb_max_probe = htod32(chanim_mark(ch_info).scb_max_probe);
	scb_probe.scb_activity_time = htod32(chanim_mark(ch_info).scb_activity_time);

	/* get the current setting from the driver */
	ret = acs_set_scb_probe(c_info->name, &scb_probe, sizeof(wl_scb_probe_t));
	ACS_ERR(ret, "failed to get chanim results");

	return ret;
}

void
chanim_upd_acs_record(chanim_info_t *ch_info, chanspec_t selected, uint8 trigger)
{
	chanim_acs_record_t* cur_record = &ch_info->record[chanim_mark(ch_info).record_idx];
	chanim_stats_t *cur_stats = &ch_info->stats[chanim_mark(ch_info).stats_idx];
	time_t now = uptime();

	bzero(cur_record, sizeof(chanim_acs_record_t));

	cur_record->trigger = trigger;
	cur_record->timestamp = now;
	cur_record->selected_chspc = selected;
	cur_record->valid = TRUE;

	if (cur_stats) {
		cur_record->bgnoise = cur_stats->bgnoise;
		cur_record->glitch_cnt = cur_stats->glitchcnt;
		cur_record->ccastats = cur_stats->ccastats[CCASTATS_NOPKT];
		cur_record->chan_idle = cur_stats->ccastats[CCASTATS_TXOP];
	}

	chanim_mark(ch_info).record_idx++;
	if (chanim_mark(ch_info).record_idx == ACS_CHANIM_ACS_RECORD)
		chanim_mark(ch_info).record_idx = 0;
}

bool
chanim_chk_lockout(chanim_info_t *ch_info)
{
	uint8 cur_idx = chanim_mark(ch_info).record_idx;
	uint8 start_idx;
	chanim_acs_record_t *start_record;
	time_t now = uptime();
	time_t passed = 0;
	int i, j;

	if (!chanim_config(ch_info).max_acs)
		return TRUE;

	for (i = 0, j = 0; i < CHANIM_STATS_RECORD; i++) {

		start_idx = MODSUB(cur_idx, i+1, ACS_CHANIM_ACS_RECORD);
		start_record = &ch_info->record[start_idx];

		if (start_record->trigger == APCS_INIT)
			return FALSE;

		if (start_record->trigger == APCS_CHANIM)
			j++;

		if (j == chanim_config(ch_info).max_acs)
			break;
	}

	if (j != chanim_config(ch_info).max_acs)
		return FALSE;

	passed = now - start_record->timestamp;

	if (start_record->valid && (passed <
			chanim_config(ch_info).lockout_period)) {
		ACSD_DEBUG("chanim lockout true\n");
		return TRUE;
	}
	return FALSE;
}

/* update the chanim state machine */
static int
chanim_upd_state(acs_chaninfo_t * c_info, uint8 version, uint ticks)
{
	int ret = 0;
	chanim_info_t * ch_info = c_info->chanim_info;
	uint8 cur_state = chanim_mark(ch_info).state;
	time_t now = uptime();
	bool detected = chanim_intf_detected(ch_info, version);

	ACSD_DEBUG("%s: current time: %ld\n", c_info->name, now);

	if (chanim_mark(ch_info).detected != detected) {
		chanim_update_state(c_info, detected);
		chanim_mark(ch_info).detected = detected;
	}

	switch (cur_state) {
	case CHANIM_STATE_DETECTING:
		if (detected) {
			cur_state = CHANIM_STATE_DETECTED;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			chanim_mark(ch_info).detecttime = now;
		}
		break;

	case CHANIM_STATE_DETECTED:

		if (!detected) {
			cur_state = CHANIM_STATE_DETECTING;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			chanim_mark(ch_info).detecttime = 0;
			break;
		}
		/* if detect only, break */
		if (AUTOCHANNEL(c_info)) {
			time_t passed = now - chanim_mark(ch_info).detecttime;

			if ((uint8)passed > chanim_act_delay(ch_info)) {
				cur_state = CHANIM_STATE_ACTON;
				ACSD_CHANIM("%s: state changed: from %d to %d\n",
					c_info->name,
					chanim_mark(ch_info).state, cur_state);
				chanim_speedup_scbprobe(c_info);
			}
		}
		break;

	case CHANIM_STATE_ACTON: {

		if (!detected) {
			cur_state = CHANIM_STATE_DETECTING;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			goto post_act;
		}
			/* check for lockout */
		if (chanim_chk_lockout(ch_info)) {
			cur_state = CHANIM_STATE_LOCKOUT;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			goto post_act;
		}

		if (c_info->flags & ACS_FLAGS_LASTUSED_CHK) {
			uint lastused = 0;
			lastused = acs_get_chanim_scb_lastused(c_info);

			ACSD_DEBUG("%s lastused: %d\n", c_info->name, lastused);

			if (lastused < (uint)chanim_act_delay(ch_info)) {
				cur_state = CHANIM_STATE_DETECTING;
				ACSD_DEBUG("%s: state changed: from %d to %d\n",
					c_info->name,
					chanim_mark(ch_info).state, cur_state);
				goto post_act;
			}
		}

		/* taking action */
		if (acs_allow_scan(c_info, ACS_SCAN_TYPE_CS, ticks)) {
			c_info->last_scan_type = ACS_SCAN_TYPE_CS;
			ret = acs_run_cs_scan(c_info);
			if (ret < 0)
			ACSD_WARNING("%s: cs scan failed\n", c_info->name);

			ret = acs_request_data(c_info);
			ACS_ERR(ret, "request data failed\n");

			if (acs_select_chspec(c_info)) {
				/* some other can request to change the channel via acsd, in that
				 * case proper reason will be provided by requesting APP, For ACSD
				 * USE_ACSD_DEF_METHOD: ACSD's own default method to set channel
				 */
				c_info->switch_reason = APCS_CHANIM;
				if (c_info->acs_use_csa) {
					ret = acs_csa_handle_request(c_info);
				} else {
					acs_set_chspec(c_info, TRUE, ACSD_USE_DEF_METHOD);
					ret = acs_update_driver(c_info);
					ACS_ERR(ret, "update driver failed\n");
				}

				cur_state = CHANIM_STATE_DETECTING;
				ACSD_CHANIM("%s: state changed: from %d to %d\n",
						c_info->name,
						chanim_mark(ch_info).state, cur_state);
			}
		}

post_act:
		chanim_mark(ch_info).detecttime = 0;
		chanim_restore_scbprobe(c_info);

		break;
	}

	case CHANIM_STATE_LOCKOUT:
		if (!detected) {
			cur_state = CHANIM_STATE_DETECTING;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			break;
		}

		if (!chanim_chk_lockout(ch_info)) {
			cur_state = CHANIM_STATE_DETECTED;
			ACSD_CHANIM("%s: state changed: from %d to %d\n",
				c_info->name,
				chanim_mark(ch_info).state, cur_state);
			chanim_mark(ch_info).detecttime = now;
		}
		break;

	default:
		ACSD_ERROR("%s: Invalid chanim state: %d\n", c_info->name, cur_state);
		break;
	}

	chanim_mark(ch_info).state = cur_state;
	return ret;
}

static void
chanim_update_base(chanim_stats_t * base, chanim_stats_t *tmp)
{
	if (base->glitchcnt > tmp->glitchcnt)
		base->glitchcnt = tmp->glitchcnt;
	if (base->badplcp > tmp->badplcp)
		base->badplcp = tmp->badplcp;
	if (base->ccastats[CCASTATS_NOPKT] > tmp->ccastats[CCASTATS_NOPKT])
		base->ccastats[CCASTATS_NOPKT] = tmp->ccastats[CCASTATS_NOPKT];
	if (base->bgnoise > tmp->bgnoise)
		base->bgnoise = tmp->bgnoise;
}

static void
chanim_update_base_v2(chanim_stats_t * base, chanim_stats_v2_t *tmp)
{
	if (base->glitchcnt > tmp->glitchcnt)
		base->glitchcnt = tmp->glitchcnt;
	if (base->badplcp > tmp->badplcp)
		base->badplcp = tmp->badplcp;
	if (base->ccastats[CCASTATS_NOPKT] > tmp->ccastats[CCASTATS_NOPKT])
		base->ccastats[CCASTATS_NOPKT] = tmp->ccastats[CCASTATS_NOPKT];
	if (base->bgnoise > tmp->bgnoise)
		base->bgnoise = tmp->bgnoise;
}

static int
acsd_update_chanim(acs_chaninfo_t * c_info, wl_chanim_stats_t * chanim_stats, uint ticks)
{
	int ret = 0;
	chanim_info_t * ch_info = c_info->chanim_info;
	if (chanim_stats->version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t tmp;
		chanim_stats_t *stats_v3 = (chanim_stats_t *)chanim_stats->stats;

		memcpy(&tmp, chanim_stats->stats, sizeof(chanim_stats_t));

		tmp.glitchcnt = htod32(tmp.glitchcnt);
		tmp.badplcp = htod32(tmp.badplcp);
		tmp.chanspec = htod16(tmp.chanspec);
		tmp.timestamp = htod32(tmp.timestamp);

		memcpy(&ch_info->stats[chanim_mark(ch_info).stats_idx], &tmp,
				sizeof(chanim_stats_t));

		if (acsd_debug_level & ACSD_DEBUG_CHANIM) {
			acsd_display_chanim(chanim_stats);
		}

		if (chanim_base(ch_info).chanspec != tmp.chanspec) {
			if (chanim_base(ch_info).chanspec != 0) {
				ch_info->ticks = ticks;
			}
			memcpy(&ch_info->base, &tmp, sizeof(chanim_stats_t));
		} else {
			chanim_update_base(&ch_info->base, &tmp);
		}
		/* Skip default chan switch SM for fast cs mode */
		if (BAND_5G(c_info->rs_info.band_type)) {
			/* check if perf ci scan is needed */
			if (stats_v3->chan_idle < c_info->acs_scan_chanim_stats) {
				c_info->scan_chspec_list.ci_pref_scan_request = TRUE;
			}
		}
		else {
			chanim_upd_state(c_info, chanim_stats->version, ticks);
		}
		c_info->cur_timestamp = tmp.timestamp;
		c_info->txop_score = stats_v3->ccastats[CCASTATS_TXOP] +
			stats_v3->ccastats[CCASTATS_TXDUR] +
			stats_v3->ccastats[CCASTATS_INBSS];
		c_info->txrx_score = stats_v3->ccastats[CCASTATS_TXDUR] +
			stats_v3->ccastats[CCASTATS_INBSS];
		c_info->channel_free = stats_v3->ccastats[CCASTATS_TXOP];
		if (c_info->acs_nonwifi_enable) {
			c_info->glitch_cnt = stats_v3->glitchcnt;
		}
	} else if (chanim_stats->version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t tmp;
		chanim_stats_v2_t *stats_v2 = (chanim_stats_v2_t *)chanim_stats->stats;

		memcpy(&tmp, stats_v2, sizeof(chanim_stats_v2_t));

		tmp.glitchcnt = htod32(tmp.glitchcnt);
		tmp.badplcp = htod32(tmp.badplcp);
		tmp.chanspec = htod16(tmp.chanspec);
		tmp.timestamp = htod32(tmp.timestamp);

		memcpy(&ch_info->stats[chanim_mark(ch_info).stats_idx], &tmp,
				sizeof(chanim_stats_v2_t));

		if (acsd_debug_level & ACSD_DEBUG_CHANIM) {
			acsd_display_chanim(chanim_stats);
		}

		if (chanim_base(ch_info).chanspec != tmp.chanspec) {
			if (chanim_base(ch_info).chanspec != 0) {
				ch_info->ticks = ticks;
			}
			memcpy(&ch_info->base, &tmp, sizeof(chanim_stats_v2_t));
		}
		else {
			chanim_update_base_v2(&ch_info->base, &tmp);
		}
		/* Skip default chan switch SM for fast cs mode */
		if (BAND_5G(c_info->rs_info.band_type)) {
			/* check if perf ci scna is needed */
			if (stats_v2->chan_idle < c_info->acs_scan_chanim_stats) {
				c_info->scan_chspec_list.ci_pref_scan_request = TRUE;
			}
		}
		else {
			chanim_upd_state(c_info, chanim_stats->version, ticks);
		}
		c_info->cur_timestamp = tmp.timestamp;
		c_info->txop_score = stats_v2->ccastats[CCASTATS_TXOP] +
			stats_v2->ccastats[CCASTATS_TXDUR] +
			stats_v2->ccastats[CCASTATS_INBSS];
		c_info->txrx_score = stats_v2->ccastats[CCASTATS_TXDUR] +
			stats_v2->ccastats[CCASTATS_INBSS];
		c_info->channel_free = stats_v2->ccastats[CCASTATS_TXOP];
		if (c_info->acs_nonwifi_enable) {
			c_info->glitch_cnt = stats_v2->glitchcnt;
		}
	}

	ch_info->mark.stats_idx ++;
	if (chanim_mark(ch_info).stats_idx == CHANIM_STATS_RECORD)
		chanim_mark(ch_info).stats_idx = 0;

	ACSD_DEBUG("%s: stats_idx: %d\n", c_info->name, chanim_mark(ch_info).stats_idx);
#ifdef ACSD_SEGMENT_CHANIM
	if (c_info->ch_avail && c_info->ch_avail_count) {
		c_info->txop_score = acsd_get_segment_rep(c_info, c_info->txop_score);
	}
#endif /* ACSD_SEGMENT_CHANIM */

	return ret;
}

#ifdef ACSD_SEGMENT_CHANIM
#define ACSD_INFO_LOG_ARR(header, arr, len) do { if (acsd_debug_level & ACSD_DEBUG_INFO) { \
	int i; \
	ACSD_INFO("\nPrinting array: %s of len %d\n", (header), (len)); \
	for (i = 0; i < (len); i++) { \
		printf("%4d%c", (arr)[i], ((!((i + 1) % 25) || (i + 1) == (len)) ? '\n' : '\t')); \
	} \
} } while (0)

#define ACSD_TXOP_MAX 100

static void acsd_segment_divide(acs_chaninfo_t *c_info)
{
	uint8 ndivs, seg_span;
	const int arr_len = c_info->ch_avail_count;
	uint8 *div = c_info->div, *arr = c_info->ch_avail;
	int abs_freq[ACSD_TXOP_MAX + 1] = {0};		/* absolute frequency */
	int loc_cumu_freq[ACSD_TXOP_MAX + 1] = {0};	/* local cumulative frequency */
	int i;

	if (!c_info->segment_chanim || !c_info->ch_avail || !c_info->num_seg ||
			!c_info->ch_avail_count) {
		ACSD_INFO("%s: exit seg_chanim:%d, ch_avail:%p, num_seg:%d, ch_avail_count:%d\n",
				c_info->name,
				c_info->segment_chanim, c_info->ch_avail, c_info->num_seg,
				c_info->ch_avail_count);
		return;
	}
	ndivs = c_info->num_seg - 1;
	seg_span = ACSD_TXOP_MAX / c_info->num_seg;

	/* build absolute frequency map per txop index 0 - 100 */
	for (i = 0; i < arr_len; i++) {
		if (arr[i] > ACSD_TXOP_MAX) {
			ACSD_DEBUG("%s: Err: Must be 0 - %d. Not %d\n", c_info->name,
				ACSD_TXOP_MAX, arr[i]);
			arr[i] = ACSD_TXOP_MAX;
		}
		abs_freq[arr[i]]++;
	}

	/* build local cumulative frequency map for a segment sized window around each index */
	for (i = 0; i <= ACSD_TXOP_MAX; i++) {
		int j, from = i - seg_span/2, to = from + seg_span - 1;
		from = from < 0 ? 0 : from;
		to = to > ACSD_TXOP_MAX ? ACSD_TXOP_MAX : to;
		loc_cumu_freq[i] = 0;
		for (j = from; j < to; j++) {
			loc_cumu_freq[i] += abs_freq[j];
		}
	}

	/* build divider array; div at an index is the minimum loc_cumu_freq around the index */
	for (i = 0; i < ndivs; i++) {
		int j, from = (i + 1) * seg_span - seg_span/2, to = from + seg_span - 1;
		div[i] = (from + to) /2 + 1;		/* default divider */
		for (j = from + 1; j < to; j++) {	/* change divider to local minima */
			if (loc_cumu_freq[j] < loc_cumu_freq[div[i]]) {
				div[i] = j;
			}
		}
	}

	/* override div if abs_freq at a div is much higher than abs_freq elsewhere in the span */
	for (i = 0; i < ndivs; i++) {
		int j, from = (i + 1) * seg_span - seg_span/2, to = from + seg_span - 1;
		for (j = from + 1; j < to; j++) {
			if ((abs_freq[div[i]] - abs_freq[j]) > abs_freq[div[i]]/4) {
				div[i] = j;
			}
		}
	}
	ACSD_INFO_LOG_ARR("div post", div, ndivs);
}

/** Given a ch_avail value, find the segment it belongs to using divider array and
 * return the representative midpoint of the respective segment
 *
 * Before calling this, acsd_segment_divide() call must have filled c_info->div array
 * for the latest chanim all read from the driver
 */
static uint8 acsd_get_segment_rep(acs_chaninfo_t * c_info, uint8 val)
{
	uint8 * div = c_info->div;
	uint8 ndivs, segoff, divoff, ret = val;
	int8 i;

	if (!c_info->segment_chanim || !c_info->ch_avail || !c_info->num_seg ||
			!c_info->ch_avail_count) {
		ACSD_INFO("%s: No divs seg_chanim:%d, ch_avail:%p, num_seg:%d, ch_avail_count:%d\n",
				c_info->name,
				c_info->segment_chanim, c_info->ch_avail, c_info->num_seg,
				c_info->ch_avail_count);
		return ret; /* return value itself without change */
	}

	ndivs = (c_info->num_seg - 1);
	segoff = (ACSD_TXOP_MAX / c_info->num_seg);
	divoff = segoff/2;
	ret = segoff * (ndivs) + divoff;

	for (i = ndivs - 1; i >= 0; i--) {
		if (val > div[i]) {
			break;
		}
		ret = segoff * i + divoff;
	}
	return ret;
}

/** takes c_info->ch_avail, divides the values into segments and replaces
 * original ch_avail values with segment representatives instead
 */
int acsd_segmentize_chanim(acs_chaninfo_t * c_info)
{
	int i;

	acsd_segment_divide(c_info);	/* get dividers (c_info->div) based on c_info->ch_avail */

	ACSD_INFO_LOG_ARR("ch_avail pre", c_info->ch_avail, c_info->ch_avail_count);

	for (i = 0; i < c_info->ch_avail_count; i++) {
		c_info->ch_avail[i] = acsd_get_segment_rep(c_info, c_info->ch_avail[i]);
	}

	ACSD_INFO_LOG_ARR("ch_avail post", c_info->ch_avail, c_info->ch_avail_count);

	return 0;
}
#endif /* ACSD_SEGMENT_CHANIM */

int
acsd_chanim_query(acs_chaninfo_t * c_info, uint32 count, uint32 ticks)
{
	int i, j = 0, ret = 0;
	char *data_buf;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param;
	int buflen = ACS_CHANIM_BUF_LEN;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t input_band = 0;

	data_buf = acsd_malloc(ACS_CHANIM_BUF_LEN);
	list = (wl_chanim_stats_t *) data_buf;

	param.buflen = htod32(buflen);
	param.count = htod32(count);

	ret = acs_get_chanim_stats(c_info, &param, sizeof(wl_chanim_stats_t), data_buf, buflen);
	if (ret < 0)
		ACS_FREE(data_buf);
	ACS_ERR(ret, "failed to get chanim results");

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);

	ACSD_DEBUG("%s: buflen: %d, version: %d count: %d\n",
		c_info->name, list->buflen, list->version, list->count);

	if (list->buflen == 0) {
		list->version = 0;
		list->count = 0;
	} else if (!((list->version == WL_CHANIM_STATS_VERSION) ||
			(list->version == WL_CHANIM_STATS_V2))) {
		fprintf(stderr, "Sorry, your driver has wl_chanim_stats version %d "
				"but this program supports only version %d.\n",
				list->version, WL_CHANIM_STATS_VERSION);
		list->buflen = 0;
		list->count = 0;
	}

	if (count == WL_CHANIM_COUNT_ALL) {
		ACS_FREE(c_info->chanim_stats);
		c_info->chanim_stats = list;
#ifdef DEBUG
		acsd_dump_chanim(c_info->chanim_stats);
#endif // endif
	}
	else {
		ret = acsd_update_chanim(c_info, list, ticks);
		ACS_FREE(list);
		return ret;
	}

#ifdef ACSD_SEGMENT_CHANIM
	if (!c_info->segment_chanim) {
		return ret;
	}
	if (count != WL_CHANIM_COUNT_ALL) {
		ACSD_DEBUG("%s: count:%d\n", c_info->name, count);
		return ret;
	}
	if (!c_info->ch_avail) {
		ACSD_DEBUG("%s: ch_avail is NULL\n", c_info->name);
		return ret;
	}
	if (!c_info->ch_avail_count) {
		ACSD_DEBUG("%s: ch_avail_count is zero\n", c_info->name);
		return ret;
	}
	ACSD_INFO("ACSD: %s: list count: %d, ch_avail_count: %d\n", c_info->name, list->count,
			c_info->ch_avail_count);
	if (list->version == WL_CHANIM_STATS_VERSION)
	{
		stats = (chanim_stats_t *)list->stats;
	} else if (list->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)list->stats;
	}
	for (i = 0; i < c_info->ch_avail_count; i++) {
		c_info->ch_avail[i] = 100;
	}
	if (c_info->ch_avail_count < list->count)
	{
		ACSD_DEBUG("%s: ch_avail_count less than chanim count\n", c_info->name);
		return ret;
	}

	if (BAND_5G(c_info->rs_info.band_type)) {
		input_band = WL_CHANSPEC_BAND_5G;
	} else {
		input_band = WL_CHANSPEC_BAND_2G;
	}

	for (i = 0; i < list->count; i++) {
		if (list->version == WL_CHANIM_STATS_VERSION) {
			uint8 *cc = stats[i].ccastats;
			if (!stats[i].chanspec || input_band != CHSPEC_BAND(stats[i].chanspec)) {
				continue;
			}
			c_info->ch_avail[j] = cc[CCASTATS_TXOP] + cc[CCASTATS_INBSS] +
				cc[CCASTATS_TXDUR];
			j++;
		}
		else if (list->version == WL_CHANIM_STATS_V2) {
			uint8 *cc = statsv2[i].ccastats;
			if (!statsv2[i].chanspec ||
					input_band != CHSPEC_BAND(statsv2[i].chanspec)) {
				continue;
			}
			c_info->ch_avail[j] = cc[CCASTATS_TXOP] + cc[CCASTATS_INBSS] +
				cc[CCASTATS_TXDUR];
			j++;
		}
	}

	if (list->count > 1) {
		acsd_segmentize_chanim(c_info);
	}
#endif /* ACSD_SEGMENT_CHANIM */
	return ret;
}

void
acsd_chanim_check(uint ticks, acs_chaninfo_t *c_info)
{
	chanim_info_t* ch_info = c_info->chanim_info;
	uint8 period = chanim_config(ch_info).sample_period;
	int ret;

	/* sync the sample period in driver if it is changed */
	if (period != chanim_mark(ch_info).wl_sample_period) {
		ret = acs_set_chanim_sample_period(c_info->name, (uint)period);
		if (ret < 0) {
			ACSD_ERROR("%s: failed to set chanim_sample_period", c_info->name);
			return;
		}
		chanim_mark(ch_info).wl_sample_period = period;
	}

	ACSD_DEBUG("%s: ticks: %d, period: %d\n", c_info->name, ticks, period);

	/* start the query after a number of  ticks (since start or channel change,
	 * since bgnoise is  not accurate before that.
	 */
	if (period && (ticks - ch_info->ticks > CHANIM_CHECK_START) &&
		(ticks % period == 0)) {
		acsd_chanim_query(c_info, WL_CHANIM_COUNT_ONE, ticks);
	}
}

int
chanim_txop_to_noise(uint8 txop)
{
	int noise = 0;
	uint8 val = 0;

	if (txop > CHANIM_TXOP_BASE)
		return noise;

	val = CHANIM_TXOP_BASE - txop;
	noise = (int)val * val * val * val / 51200;

	return noise;
}
