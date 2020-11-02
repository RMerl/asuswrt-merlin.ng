/*
 *      acs_channels.c
 *
 *      This module will calculate Candidate scores for all the channels.
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
 *	$Id: acs_channels.c 786918 2020-05-12 04:39:27Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"

/*
 * channel_pick_t: Return value from the channel pick (preference comparison) functions
 *	PICK_NONE	: Function made no choice, someone else is to decide.
 *	PICK_CANDIDATE	: Candidate chanspec preferred over current chanspec
 *	PICK_CURRENT	: Current chanspec is preferred over candidate.
 */
typedef enum { PICK_NONE = 0, PICK_CURRENT, PICK_CANDIDATE } channel_pick_t;

#define ACS_CHANNEL_1 1
#define ACS_CHANNEL_6 6
#define ACS_CHANNEL_11 11

bool acs_is_initial_selection(acs_chaninfo_t *c_info);
/*
* acs_cns_normalize_score() - Normalizes the CNS score
* cns:		The highest and lowest noise scores to use for normalization
* score:	The CNS score to normalize
*
* This function normalizes the score by computing ((score-lowest noise score)*10)
* and dividing it by (highest noise score - lowest noise score)
*
*/
static int
acs_cns_normalize_score(cns_score_t *cns, int score)
{
	int diff = score - cns->lowest_score;
	int range = cns->highest_score - cns->lowest_score;
	ACSD_DFSR("Score before normalization = %d lowest noise score"
		" = %d highest noise score = %d \n Numerator = %d"
		" Denominator = %d\n", score, cns->lowest_score,
		cns->highest_score, diff * 10, range);
	return (diff * 10) / range;
}

/*
 * acs_pick_best_possible_channel - ACSD channel selection algorithm
 * Returns the best possible channel from a list of candidates
 */
static channel_pick_t
acs_pick_best_possible_channel(acs_chaninfo_t *c_info, ch_candidate_t *current,
	ch_candidate_t *candidate, int score_type, cns_score_t *cns)
{
	int current_normalized_score, candidate_normalized_score;

	ACSD_INFO("%s: Current: %sDFS channel #%d (0x%x) score[%d] %d,"
			" candidate: %sDFS channel #%d (0x%x) score %d.\n",
			c_info->name,
			current->is_dfs ? "" : "non-",
			CHSPEC_CHANNEL(current->chspec), current->chspec,
			score_type,
			current->chscore[score_type].score,
			candidate->is_dfs ? "" : "non-",
			CHSPEC_CHANNEL(candidate->chspec), candidate->chspec,
			candidate->chscore[score_type].score);

	/* Pick the channel having the better score */
	if (candidate->chscore[score_type].score < current->chscore[score_type].score) {
		ACSD_INFO("%s: -- selecting candidate: better score.\n", c_info->name);
		return PICK_CANDIDATE;
	} else if (current->chscore[score_type].score <
			candidate->chscore[score_type].score) {
		ACSD_INFO("%s: -- keeping current: better score.\n", c_info->name);
		return PICK_CURRENT;
	}

	/* In the event of a tie, pick the channel with the lower noise for 5G
	 * This includes currently CCI, ACI.
	 * TBD - Non-Wifi Interference
	 */
	if (CHSPEC_IS5G(candidate->chspec)) {
			current_normalized_score = acs_cns_normalize_score(cns,
				current->chscore[CH_SCORE_CNS].score);
			candidate_normalized_score = acs_cns_normalize_score(cns,
				candidate->chscore[CH_SCORE_CNS].score);

			ACSD_INFO("%s: current channel normalized score = %d, "
				"candidate normal score = %d\n", c_info->name,
				current_normalized_score, candidate_normalized_score);

			/* lower noise wins */
			if (candidate_normalized_score < current_normalized_score) {
				ACSD_INFO("%s: -- selecting candidate: better CNS score.\n",
					c_info->name);
				return PICK_CANDIDATE;
			} else if (current_normalized_score < candidate_normalized_score) {
				ACSD_INFO("%s: -- keeping current : better CNS score.\n",
					c_info->name);
				return PICK_CURRENT;
			}

		/* Both same score - leave Channel selection to the other selection parameters
		*/
		return PICK_NONE;
	} else {
		/* For 2G if there is a tie in the first level score pick the channel
		 * with the least INTFADJ score
		 */
		int intadj_score = current->chscore[CH_SCORE_INTFADJ].score;
		if (candidate->chscore[CH_SCORE_INTFADJ].score < intadj_score) {
			ACSD_INFO("%s: -- selecting candidate: quieter 2G channel.\n",
				c_info->name);
			return PICK_CANDIDATE;
		} else {
			ACSD_INFO("%s: -- keeping current : quieter 2G channel.\n", c_info->name);
			return PICK_CURRENT;
		}
	}
}

/*
 * acs_remove_noisy_cns() - Find candidate with best CNS score and disable too distant candidates.
 *
 * candi:	pointer to the candidate array
 * c_count:	number of candidates in the array
 * distance:	value of configuration parameter acs_trigger_var
 *
 * This function looks up the candidate with the best (lowest) CNS score, and removes all
 * other candidates whose CNS score is further than a certain distance by disabling them.
 * It returns the best CNS noise score which is used to normalize the CNS scores
 */
static int
acs_remove_noisy_cns(ch_candidate_t *candi, int c_count, int distance)
{
	ch_score_t *best_score_p = NULL;
	int i;

	if (!candi) {
		return BCME_OK;
	}
	/* Determine candidate with the best (lowest) CNS score */
	for (i = 0; i < c_count; i++) {
		if (!candi[i].valid)
			continue;
		if (!best_score_p) {
			best_score_p = candi[i].chscore;
			continue;
		}
		if ((candi[i].chscore[CH_SCORE_CNS].score
			< best_score_p[CH_SCORE_CNS].score)) {
			best_score_p = candi[i].chscore;
		}
	}

	/* ban chanspec that are too far away from best figure */
	for (i = 0; i < c_count; i++) {
		if (!best_score_p) {
			return BCME_OK;
		}
		if (candi[i].valid &&
			(candi[i].chscore[CH_SCORE_CNS].score >=
			(best_score_p[CH_SCORE_CNS].score + distance))) {
			ACSD_INFO("banning chanspec %x because of interference \n",
				candi[i].chspec);
			candi[i].valid = FALSE;
			candi[i].reason |= ACS_INVALID_NOISE;
		}
	}
	if (!best_score_p) {
		return BCME_OK;
	}
	return best_score_p[CH_SCORE_CNS].score;
}

/* Select channel based on the following constraints
 * For EU - prefer DFS high power, DFS low power and then low power
 * For US - prefer high power, DFS, low power
 */
static int
acs_prioritize_channels(acs_chaninfo_t *c_info, ch_candidate_t *current,
	ch_candidate_t *candidate)
{
	bool is_cur_lp = acsd_is_lp_chan(c_info, current->chspec);
	bool is_candidate_lp = acsd_is_lp_chan(c_info, candidate->chspec);

	if (c_info->country_is_edcrs_eu) {
		if (!current->is_dfs && candidate->is_dfs) {
			ACSD_INFO("%s: -- selecting candidate: DFS channel. \n", c_info->name);
			goto pick_candidate;
		} else if (current->is_dfs && candidate->is_dfs) {
			if (is_cur_lp && !is_candidate_lp) {
				ACSD_INFO("%s: -- selecting candidate: High power channel. \n",
					c_info->name);
				goto pick_candidate;
			} else if (CHSPEC_CHANNEL(current->chspec) <
					CHSPEC_CHANNEL(candidate->chspec)) {
				ACSD_INFO("%s: -- selecting candidate: Higher channel. \n",
					c_info->name);
				goto pick_candidate;
			}
		} else if (!current->is_dfs && !candidate->is_dfs) {
			if (CHSPEC_CHANNEL(current->chspec) <
					CHSPEC_CHANNEL(candidate->chspec)) {
				ACSD_INFO("%s: -- selecting candidate: Higher channel. \n",
					c_info->name);
				goto pick_candidate;
			}

		}
	} else {
		if (is_cur_lp && !is_candidate_lp) {
			ACSD_INFO("%s: -- selecting candidate: High power channel. \n",
				c_info->name);
			goto pick_candidate;
		} else if (!is_cur_lp && !is_candidate_lp) {
			if (current->is_dfs && !candidate->is_dfs) {
				ACSD_INFO("%s: -- selecting candidate: Non-DFS channel. \n",
					c_info->name);
				goto pick_candidate;
			} else if ((current->is_dfs && candidate->is_dfs)||
					(!current->is_dfs && !candidate->is_dfs)) {
				if (CHSPEC_CHANNEL(current->chspec) <
						CHSPEC_CHANNEL(candidate->chspec)) {
					ACSD_INFO("%s: -- selecting candidate: Higher channel. \n",
						c_info->name);
					goto pick_candidate;
				}
			}
		} else if (is_cur_lp && is_candidate_lp) {
			if (CHSPEC_CHANNEL(current->chspec) <
					CHSPEC_CHANNEL(candidate->chspec)) {
				ACSD_INFO("%s: -- selecting candidate: Higher channel. \n",
					c_info->name);
				goto pick_candidate;
			}
		}
	}
	ACSD_INFO("%s: -- Keeping current channel. \n", c_info->name);
	return PICK_CURRENT;
pick_candidate:
	return PICK_CANDIDATE;
}

int
acs_check_for_txop_on_curchan(acs_chaninfo_t *c_info)
{
	acsd_chanim_query(c_info, WL_CHANIM_COUNT_ONE, 0);
	if (c_info->txop_score < c_info->acs_txop_limit) {
		ACSD_INFO("ifname: %s Allow BGDFS/DFS_REENTRY when txop score is:%d less than"
			"limit:%d\n", c_info->name, c_info->txop_score, c_info->acs_txop_limit);
		return TRUE;
	} else {
		ACSD_INFO("ifname: %s Don't allow BGDFS/DFS_REENTRY when txop score is:%d"
				"greater than limit:%d\n", c_info->name, c_info->txop_score,
				c_info->acs_txop_limit);
		return FALSE;
	}
}

/* For EU region, prefer weather over non-weather channels
 */
static int
acs_pick_eu_weather(acs_chaninfo_t *c_info, ch_candidate_t *current,
	ch_candidate_t *candidate)
{
	/* To break further ties, prefer non-weather over
	 * weather channels
	 */
	bool current_is_weather, candidate_is_weather;

	if (!c_info->country_is_edcrs_eu) {
		return PICK_NONE;
	}

	current_is_weather = acs_is_dfs_weather_chanspec(c_info,
			(current->chspec));
	candidate_is_weather = acs_is_dfs_weather_chanspec(c_info,
			(candidate->chspec));

	ACSD_INFO("%s: channel and weather:: current: 0x%x %d, candidate: 0x%x %d\n",
			c_info->name, current->chspec, current_is_weather,
			candidate->chspec, candidate_is_weather);

	/* Both DFS, same score - non weather wins. */
	if (!candidate_is_weather && current_is_weather) {
		ACSD_INFO("%s: -- selecting candidate: not weather.\n", c_info->name);
		return PICK_CANDIDATE;
	} else if (candidate_is_weather && !current_is_weather) {
		ACSD_INFO("%s: -- keeping current: not weather.\n", c_info->name);
		return PICK_CURRENT;
	}

	return PICK_NONE;
}

/* update the chanspec before calling csa for channel change */
int acs_csa_handle_request(acs_chaninfo_t *c_info)
{
	wl_bcmdcs_data_t dcs_data;
	int ret;

	dcs_data.reason = 0;
	dcs_data.chspec = c_info->selected_chspec;
	if ((ret = dcs_handle_request(c_info->name, &dcs_data,
			DOT11_CSA_MODE_ADVISORY, ACS_CSA_COUNT,
			c_info->acs_dcs_csa))) {
		ACSD_ERROR("%s: err from dcs_handle_request:%d\n",
				c_info->name, ret);
	} else { /* on CSA success, update if we have reached full bandwidth */
		if (c_info->bw_upgradable) {
			c_info->bw_upgradable = !ACS_CHSPEC_MAXED_BWCAP(c_info->selected_chspec,
					c_info->rs_info.bw_cap);
		}
	}
	return ret;
}

/* update the chanspec before calling csa for channel change */
int acs_csa_mode_handle_request(acs_chaninfo_t *c_info, wl_chan_switch_t *csa)
{
	wl_bcmdcs_data_t dcs_data;
	int ret;
	dcs_data.reason = 0;
	dcs_data.chspec = c_info->selected_chspec;
	if ((ret = dcs_handle_request(c_info->name, &dcs_data,
			csa->mode, ACS_CSA_COUNT,
			c_info->acs_dcs_csa))) {
		ACSD_ERROR("%s: err from dcs_handle_request:%d\n",
				c_info->name, ret);
	}
	return ret;
}

/*
 * acs_pick_chanspec_common() - shared function to pick a chanspec to switch to.
 *
 * c_info:	pointer to the acs_chaninfo_t for this interface.
 * bw:		bandwidth to chose from
 * score_type:	CS_SCORE_xxx (index into the candidate score array)
 *
 * Returned value:
 *	The returned value is the most preferred valid chanspec from the candidate array.
 *
 * This function starts by eliminating all candidates whose CNS is too far away from the best
 * CNS score, and then selects a chanspec to use by walking the list of valid candidates and
 * selecting the most preferred one. This selection is currently based on the score_type only,
 * further selection mechanisms are in the works.
 */
chanspec_t
acs_pick_chanspec_common(acs_chaninfo_t *c_info, int bw, int score_type)
{
	chanspec_t chspec = 0;
	int i, index = -1;
	ch_candidate_t *candi = c_info->candidate[bw];
	cns_score_t cns;
	ch_candidate_t *current, *candidate;
	wl_chanim_stats_t *chstats = c_info->chanim_stats;
	chanim_stats_v2_t *statsv2 = NULL;
	chanim_stats_t *stats = NULL;
	int txop_score = 0;

	ACSD_INFO("%s: Selecting channel, score type %d...\n", c_info->name, score_type);
	/* find the chanspec with best figure (cns) */
	cns.lowest_score = acs_remove_noisy_cns(candi, c_info->c_count[bw],
		c_info->chanim_info->config.acs_trigger_var);
	cns.highest_score = cns.lowest_score +
		c_info->chanim_info->config.acs_trigger_var;

	if (chstats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&chstats->stats;
	}

	if ((c_info->acs_dfs == ACS_DFS_DISABLED) && c_info->is160_bwcap) {
		ACSD_DEBUG("%s: Don't upgrade the bw when acs_dfs is %s and "
				" upgradable bw is 160MHz: %d\n", c_info->name,
				c_info->acs_dfs ? "Enabled" : "Disabled",
				c_info->is160_bwcap);
		c_info->bw_upgradable = 0;
	} else {
		/* ensure latest bw_upgradable status is available */
		acs_update_bw_status(c_info);
	}

	if (!acs_is_initial_selection(c_info) && !c_info->bw_upgradable &&
		!c_info->autochannel_through_cli &&
		!((c_info->switch_reason == APCS_DFS_REENTRY) && c_info->dfs_reentry)) {
		for (i = 0; i < chstats->count; i++) {
			if (chstats->version == WL_CHANIM_STATS_VERSION) {
				stats = &chstats->stats[i];
				chspec = wf_chspec_ctlchspec(c_info->cur_chspec);
				if (chspec == stats->chanspec) {
#ifdef ACSD_SEGMENT_CHANIM
					if (c_info->ch_avail && c_info->ch_avail_count > i) {
						txop_score = c_info->ch_avail[i];
					} else
#endif /* ACSD_SEGMENT_CHANIM */
					{
						txop_score = stats->ccastats[CCASTATS_TXOP] +
							stats->ccastats[CCASTATS_INBSS] +
							stats->ccastats[CCASTATS_TXDUR];
					}
					if (txop_score < c_info->acs_txop_limit) {
						if (c_info->switch_reason != APCS_TXFAIL) {
							c_info->txop_channel_select =
								ACS_TXOP_CHANNEL_SELECT;
						}
						c_info->timestamp = stats->timestamp;
						ACSD_INFO("%s: timestamp %d txop_score %d\n",
							c_info->name,
							c_info->timestamp, txop_score);
						break;
					} else {
						ACSD_INFO("%s: staying on cur channel%x score %d\n",
							c_info->name, c_info->cur_chspec,
							txop_score);
						return c_info->cur_chspec;
					}
				}
			} else if (chstats->version == WL_CHANIM_STATS_V2) {
				chspec = wf_chspec_ctlchspec(c_info->cur_chspec);
				if (chspec == statsv2->chanspec) {
#ifdef ACSD_SEGMENT_CHANIM
					if (c_info->ch_avail && c_info->ch_avail_count > i) {
						txop_score = c_info->ch_avail[i];
					} else
#endif /* ACSD_SEGMENT_CHANIM */
					{
						txop_score = statsv2->ccastats[CCASTATS_TXOP] +
							statsv2->ccastats[CCASTATS_INBSS] +
							statsv2->ccastats[CCASTATS_TXDUR];
					}
					if (txop_score < c_info->acs_txop_limit) {
						if (c_info->switch_reason != APCS_TXFAIL) {
							c_info->txop_channel_select =
								ACS_TXOP_CHANNEL_SELECT;
						}
						c_info->timestamp = statsv2->timestamp;
						ACSD_INFO("%s: timestamp %d txop_score %d\n",
							c_info->name,
							c_info->timestamp, txop_score);
						break;
					} else {
						ACSD_INFO("%s: staying on cur channel%x score %d\n",
							c_info->name, c_info->cur_chspec,
							txop_score);
						return c_info->cur_chspec;
					}
				}
				statsv2++;
			}
		}
	}

	/* Walk all candidate chanspecs and select the best one to use. */
	for (i = 0; i < c_info->c_count[bw]; i++) {
		channel_pick_t choice;

		if (!candi[i].valid)
			continue;

		if (index < 0) { /* No previous candi, avoid comparing against random memory */
			index = i; /* Select first valid candidate as a starting point */
			ACSD_INFO("%s: [%d] Default: %s channel #%d (0x%x) with score %d\n",
				c_info->name,
				i, (candi[i].is_dfs) ? "DFS" : "non-DFS",
				CHSPEC_CHANNEL(candi[i].chspec), candi[i].chspec,
				candi[i].chscore[score_type].score);
			continue;
		}

		ACSD_INFO("%s: [%d] Checking %s channel #%d (0x%x) with score %d\n",
			c_info->name,
			i, (candi[i].is_dfs) ? "DFS" : "non-DFS",
			CHSPEC_CHANNEL(candi[i].chspec), candi[i].chspec,
			candi[i].chscore[score_type].score);

		/*
		 * See if one of our choice mechanisms has a preferred candidate. Whoever picks
		 * a chanspec first wins.
		 */
		choice = PICK_NONE;
		current = &candi[index];
		candidate = &candi[i];

		if (choice == PICK_NONE) {
			choice = acs_pick_best_possible_channel(c_info, current, candidate,
					score_type, &cns);
		}

		if (choice == PICK_NONE) {
			/* EU region -- decide between weather and non-weather
			 */
			choice = acs_pick_eu_weather(c_info, current, candidate);
		}

		if (choice == PICK_NONE) {
			/* Use the default prioritzation schemes
			*/
			choice = acs_prioritize_channels(c_info, current, candidate);
		}

		if (choice == PICK_CANDIDATE) {
			index = i;
		}
	}

	/* reset monitoring state machine */
	chanim_mark(c_info->chanim_info).best_score = 0;

	if (index >= 0) {
		chspec = candi[index].chspec;
		ACSD_INFO("%s: Selected Channel #%d (0x%x)\n",
			c_info->name, CHSPEC_CHANNEL(chspec), chspec);
	}
	return chspec;
}

void
acs_parse_chanspec(chanspec_t chanspec, acs_channel_t* chan_ptr)
{
	bzero(chan_ptr, sizeof(acs_channel_t));
	GET_ALL_EXT(chanspec, (uint8 *) chan_ptr);
}

chanspec_t
acs_pick_chanspec(acs_chaninfo_t* c_info, int bw)
{
	chanspec_t chspec = 0;
	int i, index = -1;
	int score_type = CH_SCORE_TOTAL;
	ch_candidate_t *candi = c_info->candidate[bw];

	/* pick the chanspec with the highest total score */
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if (!candi[i].valid)
			continue;

		if (index < 0) { /* No previous candi, avoid comparing against random memory */
			index = i; /* Select first valid candidate as a starting point */
			ACSD_INFO("%s: [%d] Default: %s channel #%d (0x%x) with score %d\n",
				c_info->name,
				i, (candi[i].is_dfs) ? "DFS" : "non-DFS",
				CHSPEC_CHANNEL(candi[i].chspec), candi[i].chspec,
				candi[i].chscore[score_type].score);
			continue;
		}

		ACSD_INFO("%s: [%d] Checking %s channel #%d (0x%x) with score %d\n",
			c_info->name,
			i, (candi[i].is_dfs) ? "DFS" : "non-DFS",
			CHSPEC_CHANNEL(candi[i].chspec), candi[i].chspec,
			candi[i].chscore[score_type].score);

		if (candi[i].chscore[score_type].score >
		    candi[index].chscore[score_type].score) {
			ACSD_INFO("%s: -- selected higher (=better) score channel.\n",
				c_info->name);
			index = i;
		}
	}

	if (index < 0) {
		ACSD_ERROR("%s: No valid chanspec found\n", c_info->name);
	} else {
		chspec = candi[index].chspec;
		ACSD_INFO("%s: The highest score: %d, chspec: 0x%x\n",
			c_info->name,
			candi[index].chscore[score_type].score,
			chspec);
	}
	return chspec;
}

#ifdef ACSD_SEGMENT_CHANIM
int acs_segment_allocate(acs_chaninfo_t *c_info)
{
	if (c_info->segment_chanim && !c_info->ch_avail) {
		c_info->ch_avail = (uint8*) acsd_malloc(WL_NUMCHANNELS * sizeof(uint8));
		c_info->ch_avail_count = WL_NUMCHANNELS;
	}
	return 0;
}
#endif /* ACSD_SEGMENT_CHANIM */

int
acs_build_candidates(acs_chaninfo_t *c_info, int bw)
{
	wl_uint32_list_t *list;
	chanspec_t input = 0, c = 0;
	int ret = 0, i, j;
	int count = 0;
	ch_candidate_t *candi;
	acs_rsi_t *rsi = &c_info->rs_info;

	char *data_buf;
	data_buf = acsd_malloc(ACS_SM_BUF_LEN);

	int count2 = 0;
	int k = 0;
	char *data_buf2 = acsd_malloc(ACS_SM_BUF_LEN);

	char tmp[32], prefix[32];

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

	if (ret < 0)
		ACS_FREE(data_buf);
	ACS_ERR(ret, "failed to get valid chanspec lists");

	list = (wl_uint32_list_t *)data_buf;
	count = dtoh32(list->count);

	if (!count) {
		ACSD_ERROR("%s: number of valid chanspec is 0\n", c_info->name);
		ret = -1;
		goto cleanup;
	}
#ifdef ACSD_SEGMENT_CHANIM
	acs_segment_allocate(c_info);
#endif /* ACSD_SEGMENT_CHANIM */

	ACS_FREE(c_info->candidate[bw]);
#if 0
	c_info->candidate[bw] = (ch_candidate_t*)acsd_malloc(count * sizeof(ch_candidate_t));
#else
	c_info->candidate[bw] = (ch_candidate_t*)acsd_malloc(WL_NUMCHANSPECS * sizeof(ch_candidate_t));
#endif
	candi = c_info->candidate[bw];

	ACSD_DEBUG("%s: address of candi: %p\n", c_info->name, candi);
	for (i = 0; i < count; i++) {
		c = (chanspec_t)dtoh32(list->element[i]);
		candi[i].chspec = c;
		candi[i].valid = TRUE;

		if (acs_is_dfs_chanspec(c_info, candi[i].chspec)) {
			candi[i].is_dfs = TRUE;
			if (!rsi->reg_11h) {
				/* DFS Channels can be used only if 802.11h is enabled */
				candi[i].valid = FALSE;
				candi[i].reason |= ACS_INVALID_DFS_NO_11H;
			}
		} else {
			candi[i].is_dfs = FALSE;
		}

		/* assign weight based on config */
		for (j = 0; j < CH_SCORE_MAX; j++) {
			candi[i].chscore[j].weight = c_info->acs_policy.acs_weight[j];
			ACSD_DEBUG("%s: chanspec: (0x%04x) score: %d, weight: %d\n",
				c_info->name,
				c, candi[i].chscore[j].score, candi[i].chscore[j].weight);
		}
	}
	c_info->c_count[bw] = count;

	input = 0;
	snprintf(prefix, sizeof(prefix), "wl%d_", c_info->unit);
	if (BAND_5G(rsi->band_type) && bw == ACS_BW_160 && (nvram_get_int(strcat_r(prefix, "bw", tmp)) != 5)) {
		input |= WL_CHANSPEC_BAND_5G;
		input |= WL_CHANSPEC_BW_80;
		ret = acs_get_perband_chanspecs(c_info, input, data_buf2, ACS_SM_BUF_LEN);

		if (ret < 0)
			ACS_FREE(data_buf2);
		ACS_ERR(ret, "failed to get valid chanspec lists");

		list = (wl_uint32_list_t *)data_buf2;
		count2 = dtoh32(list->count);
		if (!count2) {
			ACSD_ERROR("number of valid chanspec is 0\n");
			ret = -2;
			goto cleanup;
		}

		acs_channel_t chan;
		for (i = 0; i < count2; i++) {
			c = (chanspec_t)dtoh32(list->element[i]);

			acs_parse_chanspec(c, &chan);

			if (c_info->country_is_edcrs_eu) {
				if ((chan.control < 100) || (chan.control >= 149))
					continue;
			} else if (chan.control < 149)
				continue;

			candi[count + k].chspec = c;
			candi[count + k].valid = TRUE;

			if (acs_is_dfs_chanspec(c_info, candi[count + k].chspec)) {
				candi[count + k].is_dfs = TRUE;
				if (!rsi->reg_11h) {
					/* DFS Channels can be used only if 802.11h is enabled */
					candi[count + k].valid = FALSE;
					candi[count + k].reason |= ACS_INVALID_DFS_NO_11H;
				}
			} else {
				candi[count + k].is_dfs = FALSE;
			}

			/* assign weight based on config */
			for (j = 0; j < CH_SCORE_MAX; j++) {
				candi[count + k].chscore[j].weight = c_info->acs_policy.acs_weight[j];
				ACSD_DEBUG("chanspec: (0x%04x %s) score: %d, weight: %d\n",
					c, wf_chspec_ntoa(c, chanspecbuf), candi[count + k].chscore[j].score, candi[count + k].chscore[j].weight);
			}

			k++;
		}

		c_info->c_count[bw] = count + k;
	}

cleanup:
	free(data_buf);
	free(data_buf2);
	return ret;
}

static bool
acs_has_valid_candidate(acs_chaninfo_t* c_info, int bw)
{
	int i;
	bool result = FALSE;
	ch_candidate_t* candi = c_info->candidate[bw];

	for (i = 0; i < c_info->c_count[bw]; i++) {
		if (candi[i].valid) {
			result = TRUE;
			break;
		}
	}
	ACSD_DEBUG("result: %d\n", result);
	return result;
}

/* Invalidate all channels from selection present in Exclusion list,
 */
void
acs_invalidate_exclusion_channels(ch_candidate_t *candi,
	int count, acs_conf_chspec_t *excl_chans)
{
	int i, j;
	for (i = 0; i < count; i++) {
		/* Exclude channels build candidate */
		if (excl_chans && excl_chans->count) {
			for (j = 0; j < excl_chans->count; j++) {
				if (candi[i].chspec == excl_chans->clist[j]) {
					candi[i].valid = FALSE;
					candi[i].reason |= ACS_INVALID_EXCL;
					break;
				}
			}
		}
	}
}

/* comparing ext20,ext40 and ext80 with ch, on success return TRUE else FALSE */
static int
acs_check_for_ext(acs_channel_t *chan, int ch)
{
	uint8 *ext_channel = (uint8 *) chan;
	int i;
	bool result = FALSE;
	for (i = 0; i < 8; i++) {
		if (ch == ext_channel[i]) {
			result = TRUE;
			break;
		}
	}
	return result;
}

/*
 * Individual scoring algorithm. It is subject to tuning or customization based on
 * testing results or customer requirement.
 *
 */
static int
acs_chan_score_bss(ch_candidate_t* candi, acs_chan_bssinfo_t* bss_info, int ncis)
{
	acs_channel_t chan;
	int score = 0, tmp_score = 0;
	int i, min, max;
	int ch;

	acs_parse_chanspec(candi->chspec, &chan);

	for (i = 0; i < ncis; i++) {
		ch = (int)bss_info[i].channel;

		/* control channel */
		min = max = (int)chan.control;

		ACSD_DEBUG("ch: %d, min: %d, max: %d\n", ch, min, max);
		ACSD_DEBUG("nCtrl=%2d nExt20=%2d nExt40=%2d nExt80=%2d\n",
				bss_info[i].nCtrl, bss_info[i].nExt20, bss_info[i].nExt40,
				bss_info[i].nExt80);

		if (ch == min || ch == max) {
			tmp_score = bss_info[i].nExt20 + bss_info[i].nExt40;
			if (tmp_score > 0) {
				if ((!CHSPEC_IS20(candi->chspec)) &&
						(nvram_match("acs_no_restrict_align", "0"))) {
					/* mark this candidate invalid if it is not used
					   or is 80MHz
					   */
					candi->valid = FALSE;
					candi->reason = ACS_INVALID_ALIGN;
					break;
				}
			}
			score += bss_info[i].nCtrl + bss_info[i].nExt20 + bss_info[i].nExt40 +
				bss_info[i].nExt80;
		}
	}
	ACSD_INFO("candidate: %x, score_bss: %d\n", candi->chspec, score);
	return score;
}

static void
acs_candidate_score_bss(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	acs_chan_bssinfo_t* bss_info = c_info->ch_bssinfo;
	int score = 0;
	int ncis = c_info->scan_chspec_list.count;

	score = acs_chan_score_bss(candi, bss_info, ncis);
	candi->chscore[CH_SCORE_BSS].score = score;
	ACSD_DEBUG("%s: bss score: %d for chanspec 0x%x\n", c_info->name, score, candi->chspec);
}

static void
acs_candidate_score_busy(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(candi->chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += stats->ccastats[CCASTATS_OBSS];
				hits ++;
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += statsv2->ccastats[CCASTATS_OBSS];
				hits ++;
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			done = TRUE;
			break;
		}
	}
	if (!done) {
		ACSD_ERROR("%s: busy check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}

	if (hits) {
		candi->chscore[CH_SCORE_BUSY].score = score/hits;
	}
	ACSD_DEBUG("%s: busy score: %d for chanspec 0x%x\n", c_info->name, score, chspec);
}

static void
acs_candidate_score_intf(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanspec_t chspec = candi->chspec;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	acs_channel_t chan;
	int i, ch, score = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += stats->ccastats[CCASTATS_NOPKT];
				hits ++;
			}

		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += statsv2->ccastats[CCASTATS_NOPKT];
				hits ++;
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			done = TRUE;
			break;
		}
	}
	if (!done) {
		ACSD_ERROR("%s: intf check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}

	if (hits) {
		candi->chscore[CH_SCORE_INTF].score = score/hits;
	}
	ACSD_DEBUG("%s: intf score: %d for chanspec 0x%x\n", c_info->name, score, chspec);
}

static void
acs_candidate_score_intfadj(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0;
	int dist, d_weight = 10;

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);
			ACSD_DEBUG("%s: channel: %d, ch: %d\n", c_info->name, chan.control, ch);

			if (ch != chan.control) {
				dist = ch - chan.control;
				dist = dist > 0 ? dist : dist * -1;
				score += stats->ccastats[CCASTATS_NOPKT] * d_weight / dist;

				ACSD_DEBUG("%s: dist: %d, count: %d, score: %d\n", c_info->name,
						dist, stats->ccastats[CCASTATS_NOPKT], score);
				if (chan.ext20 != 0 && ch != chan.ext20) {
					dist = ABS(ch - chan.ext20);
					score += stats->ccastats[CCASTATS_NOPKT] * d_weight / dist;
				}

				ACSD_DEBUG("%s: channel: %d, ch: %d score: %d\n", c_info->name,
						chan.control, ch, score);
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);
			ACSD_DEBUG("%s: channel: %d, ch: %d\n", c_info->name, chan.control, ch);

			if (ch != chan.control) {
				dist = ch - chan.control;
				dist = dist > 0 ? dist : dist * -1;
				score += statsv2->ccastats[CCASTATS_NOPKT] * d_weight / dist;

				ACSD_DEBUG("%s: dist: %d, count: %d, score: %d\n",
						c_info->name,
						dist, statsv2->ccastats[CCASTATS_NOPKT], score);
				if (chan.ext20 != 0 && ch != chan.ext20) {
					dist = ABS(ch - chan.ext20);
					score += statsv2->ccastats[CCASTATS_NOPKT]* d_weight / dist;
				}

				ACSD_DEBUG("%s: channel: %d, ch: %d score: %d\n",
						c_info->name,
						chan.control, ch, score);
			}
			statsv2++;
		}
	}

	candi->chscore[CH_SCORE_INTFADJ].score = score / d_weight;
	ACSD_DEBUG("%s: intf_adj score: %d for chanspec 0x%x\n", c_info->name, score, chspec);
}

static void
acs_candidate_score_fcs(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += stats->ccastats[CCASTATS_NOCTG];
				hits ++;
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score += statsv2->ccastats[CCASTATS_NOCTG];
				hits ++;
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			done = TRUE;
			break;
		}
	}
	if (!done) {
		ACSD_ERROR("%s: fcs check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}

	if (hits) {
		candi->chscore[CH_SCORE_FCS].score = score/hits;
	}
	ACSD_DEBUG("%s: fcs score: %d for chanspec 0x%x\n", c_info->name, score, chspec);
}

static void
acs_candidate_score_dfs(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	bool is_eu = c_info->country_is_edcrs_eu;	/* is in EDCRS_EU */
	bool is_dfs = acs_is_dfs_chanspec(c_info, candi->chspec);	/* is DFS/radar ch */
	int score = 0;

	if (is_eu) { /* ETSI/EDCRS_EU/Europe */
		bool is_we = FALSE; /* is TDWR (weather radar channel) */
		if (is_dfs) {
			is_we = acs_is_dfs_weather_chanspec(c_info, candi->chspec);
		}
		score = 3 - (1 + is_dfs - 3 * is_we);
	} else { /* FCC/US */
		score = 2 - (is_dfs);
	}
	candi->chscore[CH_SCORE_DFS].score = score;
	ACSD_INFO("%s: DFS score is %d chanspec %d\n", c_info->name, score, candi->chspec);
}

#define BAND_5G_NUM		6
#define TXPWR_POLICY_NUM	3
#define CHANNEL_5G_BAND_GROUP(c) \
	(c < 52) ? 1 : ((c < 100) ? 2 : ((c < 116) ? 3 : ((c < 132) ? 4 : ((c < 149) ? 5 : 6))))

struct tc {
	char *prefix;
	int index;
};

struct tc tc_list[] = {
#if defined(RTAX58U) || defined(TUFAX3000) || defined(RTAX82U) || defined(RTAX82_XD6)
	{ "US", 0 },
	{ "TW", 0 },
	{ "AA", 1 },
	{ "AU", 1 },
	{ "CX", 1 },
	{ "CA", 2 },
	{ "EU", 3 },
	{ "UK", 3 },
	{ "IL", 3 },
	{ "JP", 4 },
	{ "CN", 5 },
	{ "KR", 6 },
#elif defined(RTAX56_XD4)
	{ "US", 0 },
	{ "TW", 0 },
	{ "AA", 0 },
	{ "EU", 1 },
	{ "CN", 2 },
	{ "CA", 3 },
	{ "JP", 4 },
	{ "KR", 5 },
#endif
	{ NULL, -1}
};

enum {
	TXPWR_POLICY_POWER = 0,
	TXPWR_POLICY_DFS,
	TXPWR_POLICY_BW160M
};

struct txpwr_policy {
	double score[BAND_5G_NUM];
};

struct txpwr_policy txpwr_policy_all[][TXPWR_POLICY_NUM] = {
#if defined(RTAX58U) || defined(TUFAX3000) || defined(RTAX82U) || defined(RTAX82_XD6)
{{{ 21.0, 21.0, 21.0, 21.0, 21.0, 23.0 }}, {{ 20, 10, 10, 10, 10, 20 }}, {{ 10, 10, 10, 10, 0, 0 }}},	// US, TW
{{{ 21.0, 21.0, 21.0, 0.0, 21.0, 23.0 }}, {{ 20, 10, 10, 0, 10, 20 }}, {{ 10, 10, 0, 0, 0, 0 }}},	// AA
{{{ 18.0, 18.0, 17.0, 0.0, 17.0, 23.0 }}, {{ 20, 10, 10, 0, 10, 20 }}, {{ 15, 15, 0, 0, 0, 0 }}},	// CA
{{{ 18.0, 18.0, 25.0, 25.0, 25.0, 0.0 }}, {{ 20, 20, 20, 0, 0, 0 }}, {{ 10, 10, 10, 0, 0, 0 }}},	// EU
{{{ 15.0, 15.0, 17.0, 17.0, 17.0, 0.0 }}, {{ 10, 5, 5, 5, 0, 0 }}, {{ 5, 5, 5, 5, 0, 0 }}},		// JP
{{{ 18.0, 18.0, 0.0, 0.0, 0.0, 24.0 }}, {{ 20, 10, 0, 0, 0, 20 }}, {{ 10, 10, 0, 0, 0, 0 }}},		// CN
{{{ 15.0, 15.0, 17.0, 17.0, 17.0, 17.0 }}, {{ 10, 5, 5, 5, 0, 10 }}, {{ 5, 5, 5, 5, 0, 0 }}}		// KR
#elif defined(RTAX56_XD4)
{{{ 24.0, 0.0, 0.0, 0.0, 0.0, 26.0 }}, {{ 20, 0, 0, 0, 0, 14 }}, {{ 0, 0, 0, 0, 0, 0 }}},		// US, TW, AA
{{{ 17.0, 17.0, 24.0, 24.0, 24.0, 0.0 }}, {{ 20, 20, 20, 0, 0, 0 }}, {{ 0, 0, 0, 0, 0, 0 }}},		// EU
{{{ 20.0, 0.0, 0.0, 0.0, 0.0, 25.0 }}, {{ 20, 0, 0, 0, 0, 20 }}, {{ 0, 0, 0, 0, 0, 0 }}},		// CN
{{{ 17.0, 0.0, 0.0, 0.0, 0.0, 26.0 }}, {{ 20, 0, 0, 0, 0, 20 }}, {{ 0, 0, 0, 0, 0, 0 }}},		// CA
{{{ 19.0, 19.0, 20.0, 20.0, 0.0, 0.0 }}, {{ 10, 5, 5, 5, 0, 0 }}, {{ 0, 0, 0, 0, 0, 0 }}},		// JP
{{{ 14.5, 14.5, 17.0, 17.0, 17.0, 17.0 }}, {{ 10, 5, 5, 5, 5, 10 }}, {{ 0, 0, 0, 0, 0, 0 }}}		// KR
#endif
};

int get_tc_index(void)
{
	static int index = -1;
	char *prefix;
	const struct tc *p;

	if (index != -1)
		return index;

	prefix = nvram_safe_get("territory_code");
	for (p = &tc_list[0]; p->prefix; ++p) {
		if (!strncmp(prefix, p->prefix, 2)) {
			index = p->index;
			break;
		}
	}

	return index;
}

static void
acs_candidate_score_txpwr(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
#if defined(RTAX58U) || defined(TUFAX3000) || defined(RTAX82U) || defined(RTAX56_XD4) || defined(RTAX82_XD6)
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int tc_index = get_tc_index();
	int ch_5g_band_grp;
	int power, dfs = 0, bw160m = 0;
	int is_dfs;
	int score = 0;
	char tmp[32], prefix[32];

	if (BAND_5G(c_info->rs_info.band_type) && (tc_index != -1)) {
		is_dfs = (c_info->unit == 1) ? nvram_get_int("acs_dfs") : nvram_get_int("acs_band3");
		acs_parse_chanspec(chspec, &chan);
		ch_5g_band_grp = CHANNEL_5G_BAND_GROUP(chan.control);
		snprintf(prefix, sizeof(prefix), "wl%d_", c_info->unit);
		power = txpwr_policy_all[tc_index][TXPWR_POLICY_POWER].score[ch_5g_band_grp - 1];
		if (ch_5g_band_grp == 1 || ch_5g_band_grp == 6 || is_dfs)
		dfs = txpwr_policy_all[tc_index][TXPWR_POLICY_DFS].score[ch_5g_band_grp - 1];
		if (nvram_get_int(strcat_r(prefix, "bw_160", tmp)) && ((!is_dfs && (ch_5g_band_grp == 1)) || (is_dfs && (ch_5g_band_grp < 6))))
		bw160m = txpwr_policy_all[tc_index][TXPWR_POLICY_BW160M].score[ch_5g_band_grp - 1];
		score = -(power + dfs + bw160m);
	}
#else
	bool is_eu = c_info->country_is_edcrs_eu;	/* is in EDCRS_EU */
	bool is_hp = !ACS_IS_LOW_POW_CH(wf_chspec_ctlchan(candi->chspec),
			is_eu);		/* is high power ch */
	int score = 0;

	if (is_eu) { /* ETSI/EDCRS_EU/Europe */
		/* Order of preference DFS high power, DFS low power, low power channel, weather */
		score = 3 - (1 + is_hp);
	} else { /* FCC/US */
		/* order of preference High power (DFS/non), DFS, low power non-DFS channels */
		score = 2 - (2 * is_hp);
	}
#endif
	candi->chscore[CH_SCORE_TXPWR].score = score;
	ACSD_INFO("%s: txpower score is %d chanspec %d\n", c_info->name, score, candi->chspec);
}

static void
acs_candidate_score_bgnoise(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}
	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				if (stats->bgnoise && stats->bgnoise > ACS_BGNOISE_BASE) {
					score += MIN(10, stats->bgnoise - ACS_BGNOISE_BASE);
				}
				hits ++;
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				if (statsv2->bgnoise && statsv2->bgnoise > ACS_BGNOISE_BASE) {
					score += MIN(10, statsv2->bgnoise - ACS_BGNOISE_BASE);
				}
				hits ++;
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			done = TRUE;
			break;
		}
	}
	if (!done) {
		ACSD_ERROR("%s: bgnoise check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}
	if (hits) {
		candi->chscore[CH_SCORE_BGNOISE].score = score/hits;
	}
	ACSD_DEBUG("%s: bgnoise score: %d for chanspec 0x%x\n", c_info->name, score, chspec);
}

static void
acs_candidate_score_total(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	int i, total;
	ch_score_t *score_p;

	total = 0;
	score_p = candi->chscore;

	for (i = 0; i < CH_SCORE_TOTAL; i++)
		total += score_p[i].score * score_p[i].weight;

	if (BAND_5G(c_info->rs_info.band_type) && c_info->txop_weight) {
		total += score_p[CH_SCORE_TXOP].score * c_info->txop_weight;
	} else {
		total += score_p[CH_SCORE_TXOP].score * score_p[CH_SCORE_TXOP].weight;
	}
	/* Calculate DFS score seperately */
	if (BAND_5G(c_info->rs_info.band_type) && score_p[CH_SCORE_DFS].weight) {
		total += score_p[CH_SCORE_DFS].score * score_p[CH_SCORE_DFS].weight;
	}
	score_p[CH_SCORE_TOTAL].score = total;
}

static void
acs_candidate_score_cns(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0, max_score = -200, hits = 0;

	/*
	 * Check ch_stats to handle rare crash occurence due to ch_stats being NULL.
	 */
	if (!ch_stats) {
		ACSD_ERROR("%s: No chanim_stats available to calculate CNS scores\n",
			c_info->name);
		return;
	}

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score = stats->bgnoise;
				score += chanim_txop_to_noise(stats->chan_idle);
				hits ++;
				max_score = MAX(max_score, score);
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				score = statsv2->bgnoise;
				score += chanim_txop_to_noise(statsv2->chan_idle);
				hits ++;
				max_score = MAX(max_score, score);
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			break;
		}
	}

	/*
	 * Calculate the CNS based on the noise on any valid 20MHz subchannel of a 40 or 80MHz
	 * channel. NOTE that setting CNS=0 is a high noise value and effectively invalidates
	 * the channel. Only set CNS=0 if all the 20MHz subchannels are excluded.
	 */
	if (!hits) {
		ACSD_ERROR("%s: knoise check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}
	candi->chscore[CH_SCORE_CNS].score = max_score;
	ACSD_INFO("%s: Composite Noise Score (CNS): %d for chanspec 0x%x\n", c_info->name,
		max_score, chspec);
}

/* This function is used to determine whether the current channel is an adjacent channel
* to the candidate channel
*/
static bool
acs_check_adjacent_bss(acs_chaninfo_t *c_info, int ch, int channel_lower,
	int channel_higher)
{
	bool ret = FALSE;
	if (BAND_2G(c_info->rs_info.band_type)) {
		if ((ch == (channel_lower + (CH_20MHZ_APART + CH_5MHZ_APART))) ||
			(ch == (channel_lower - (CH_20MHZ_APART + CH_5MHZ_APART)))) {
			ret = TRUE;
		}
	} else {
		if ((ch == (channel_lower - CH_20MHZ_APART)) ||
			(ch == (channel_higher + CH_20MHZ_APART))) {
			ret = TRUE;
		}
	}
	return ret;
}

static void
acs_candidate_adjacent_bss(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	acs_chan_bssinfo_t* bss_info = c_info->ch_bssinfo;
	int i, ch, adjbss = 0;
	int ncis = c_info->scan_chspec_list.count;
	acs_channel_t chan;
	bool add_to_bss_check = FALSE;
	uint8 channel_sb_lower, channel_sb_upper;

	acs_parse_chanspec(candi->chspec, &chan);

	if (CHSPEC_IS160(candi->chspec)) {
		channel_sb_lower = LLL_20_SB_160(CHSPEC_CHANNEL(candi->chspec));
		channel_sb_upper = UUU_20_SB_160(CHSPEC_CHANNEL(candi->chspec));
	} else if (CHSPEC_IS8080(candi->chspec)) {
		channel_sb_lower = LLL_20_SB_8080(CHSPEC_CHANNEL(candi->chspec));
		channel_sb_upper = LUU_20_SB_8080(CHSPEC_CHANNEL(candi->chspec));
	} else if (CHSPEC_IS80(candi->chspec)) {
		channel_sb_lower = LL_20_SB(CHSPEC_CHANNEL(candi->chspec));
		channel_sb_upper = UU_20_SB(CHSPEC_CHANNEL(candi->chspec));
	} else if (CHSPEC_IS40(candi->chspec)) {
		channel_sb_lower = LOWER_20_SB(CHSPEC_CHANNEL(candi->chspec));
		channel_sb_upper = UPPER_20_SB(CHSPEC_CHANNEL(candi->chspec));
	} else {
		channel_sb_lower = channel_sb_upper = CHSPEC_CHANNEL(candi->chspec);
	}

	for (i = 0; i < ncis; i++) {
		ch = (int)bss_info[i].channel;
		if (BAND_2G(c_info->rs_info.band_type)) {
			if (CHSPEC_SB_LOWER(candi->chspec)) {
				add_to_bss_check =
					acs_check_adjacent_bss(c_info, ch, channel_sb_lower, 0);
			} else {
				add_to_bss_check =
					acs_check_adjacent_bss(c_info, ch, channel_sb_upper, 0);
			}
		} else {
			add_to_bss_check = acs_check_adjacent_bss(c_info, ch,
				channel_sb_lower, channel_sb_upper);
			if (CHSPEC_IS8080(candi->chspec)) {
				uint8 chan2_sb_lower, chan2_sb_upper;

				chan2_sb_lower = ULL_20_SB_8080(
						CHSPEC_CHANNEL(candi->chspec));
				chan2_sb_upper = UUU_20_SB_8080(
						CHSPEC_CHANNEL(candi->chspec));
				add_to_bss_check |= acs_check_adjacent_bss(
						c_info, ch, chan2_sb_lower, chan2_sb_upper);
			}
		}

		if (add_to_bss_check) {
			adjbss += bss_info[i].nCtrl;
			adjbss += bss_info[i].nExt20 + bss_info[i].nExt40 +
				bss_info[i].nExt80;
		}
	}

	candi->chscore[CH_SCORE_ADJ].score = adjbss;
	ACSD_5G("%s: adjacent bss score: %d for chanspec 0x%x\n", c_info->name,
		adjbss, candi->chspec);
}

static void
acs_candidate_score_txop(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, score = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(candi->chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				/* busy/unable-to-tx time */
#ifdef ACSD_SEGMENT_CHANIM
				if (c_info->ch_avail && c_info->ch_avail_count > i) {
					score = MAX(score, 100 - c_info->ch_avail[i]);
				} else
#endif /* ACSD_SEGMENT_CHANIM */
				{
					score = MAX(score, 100 - stats->ccastats[CCASTATS_TXOP] -
						stats->ccastats[CCASTATS_INBSS] -
						stats->ccastats[CCASTATS_TXDUR]);
				}
				hits ++;
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (acs_check_for_ext(&chan, ch)) {
				/* busy/unable-to-tx time */
#ifdef ACSD_SEGMENT_CHANIM
				if (c_info->ch_avail && c_info->ch_avail_count > i) {
					score = MAX(score, 100 - c_info->ch_avail[i]);
				} else
#endif /* ACSD_SEGMENT_CHANIM */
				{
					score = MAX(score, 100 - statsv2->ccastats[CCASTATS_TXOP] -
						statsv2->ccastats[CCASTATS_INBSS] -
						statsv2->ccastats[CCASTATS_TXDUR]);

				}
				hits ++;
			}
			statsv2++;
		}
		if ((hits == 8) || (hits == 4 && !chan.ext80[0]) ||
				(hits == 2 && !chan.ext40[0]) || (hits && !chan.ext20)) {
			done = TRUE;
			break;
		}
	}

	if (done) {
		candi->chscore[CH_SCORE_TXOP].score = score;
	} else {
		ACSD_ERROR("%s: txop check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}

	ACSD_DEBUG("%s: txop score: %d for chanspec 0x%x\n", c_info->name,
		candi->chscore[CH_SCORE_TXOP].score, chspec);
}
static void
acs_fill_score(acs_chaninfo_t* c_info, ch_candidate_t* candi)
{
	ch_score_t *score_p = candi->chscore;

	/* calculate the score for each factor */
	if (score_p[CH_SCORE_BSS].weight) {
		acs_candidate_score_bss(candi, c_info);
	}

	if (score_p[CH_SCORE_BUSY].weight) {
		acs_candidate_score_busy(candi, c_info);
	}

	if (score_p[CH_SCORE_INTF].weight) {
		acs_candidate_score_intf(candi, c_info);
	}

	if (score_p[CH_SCORE_INTFADJ].weight) {
		acs_candidate_score_intfadj(candi, c_info);
	}

	if (score_p[CH_SCORE_FCS].weight) {
		acs_candidate_score_fcs(candi, c_info);
	}

	if (score_p[CH_SCORE_TXPWR].weight) {
		acs_candidate_score_txpwr(candi, c_info);
	}

	if (score_p[CH_SCORE_BGNOISE].weight) {
		acs_candidate_score_bgnoise(candi, c_info);
	}

	if (score_p[CH_SCORE_TXOP].weight ||
			(BAND_5G(c_info->rs_info.band_type) && c_info->txop_weight)) {
		acs_candidate_score_txop(candi, c_info);
	}

	acs_candidate_score_cns(candi, c_info);

	if (score_p[CH_SCORE_ADJ].weight) {
		acs_candidate_adjacent_bss(candi, c_info);
	}

	if (score_p[CH_SCORE_DFS].weight) {
		acs_candidate_score_dfs(candi, c_info);
	}

	acs_candidate_score_total(candi, c_info);
}

/* given a chanspec returns its total score */
static int
acs_get_ch_score(acs_chaninfo_t* c_info, chanspec_t chspec)
{
	ch_candidate_t candi = {0};
	int i;

	candi.chspec = chspec;
	candi.valid = TRUE;
	candi.is_dfs = acs_is_dfs_chanspec(c_info, chspec);

	/* assign weight based on configured policy */
	for (i = 0; i < CH_SCORE_MAX; ++i) {
		candi.chscore[i].weight = c_info->acs_policy.acs_weight[i];
	}
	acs_fill_score(c_info, &candi);

	return candi.chscore[CH_SCORE_TOTAL].score;
}

static void
acs_candidate_score(acs_chaninfo_t* c_info, int bw)
{
	ch_candidate_t* candi;
	int i;

	for (i = 0; i < c_info->c_count[bw]; i++) {
		candi = &c_info->candidate[bw][i];
		if (!candi->valid)
			continue;

		ACSD_DEBUG("%s: calc score for candidate chanspec: 0x%x\n",
			c_info->name, candi->chspec);

		/* calculate the score for each factor */
		acs_fill_score(c_info, candi);
#ifdef ACS_DEBUG
		acs_dump_score(candi->chscore);
		acs_dump_score_csv(candi->chspec, candi->chscore);
#endif // endif
	}
}

static void
acs_candidate_check_intf(ch_candidate_t *candi, acs_chaninfo_t* c_info)
{
	wl_chanim_stats_t *ch_stats = c_info->chanim_stats;
	chanim_stats_t *stats = NULL;
	chanim_stats_v2_t *statsv2 = NULL;
	chanspec_t chspec = candi->chspec;
	acs_channel_t chan;
	int i, ch, intf = 0, hits = 0;
	bool done = FALSE;

	acs_parse_chanspec(chspec, &chan);
	if (ch_stats->version == WL_CHANIM_STATS_V2) {
		statsv2 = (chanim_stats_v2_t *)&ch_stats->stats;
	}

	for (i = 0; i < ch_stats->count; i++) {
		if (ch_stats->version == WL_CHANIM_STATS_VERSION) {
			stats = (chanim_stats_t *)&ch_stats->stats[i];
			ch = CHSPEC_CHANNEL(stats->chanspec);

			if (ch == chan.control || ch == chan.ext20) {
				if (c_info->flags & ACS_FLAGS_INTF_THRES_CCA) {
					intf = stats->ccastats[CCASTATS_NOPKT];
					if (intf > c_info->acs_policy.intf_threshold) {
						candi->valid = FALSE;
						candi->reason |= ACS_INVALID_INTF_CCA;
						break;
					}
				}
				if (c_info->flags & ACS_FLAGS_INTF_THRES_BGN) {
					intf = stats->bgnoise;
					if (intf && intf > c_info->acs_policy.bgnoise_thres) {
						candi->valid = FALSE;
						candi->reason |= ACS_INVALID_INTF_BGN;
						break;
					}
				}
				hits ++;
			}
		} else if (ch_stats->version == WL_CHANIM_STATS_V2) {
			ch = CHSPEC_CHANNEL(statsv2->chanspec);

			if (ch == chan.control || ch == chan.ext20) {
				if (c_info->flags & ACS_FLAGS_INTF_THRES_CCA) {
					intf = statsv2->ccastats[CCASTATS_NOPKT];
					if (intf > c_info->acs_policy.intf_threshold) {
						candi->valid = FALSE;
						candi->reason |= ACS_INVALID_INTF_CCA;
						break;
					}
				}
				if (c_info->flags & ACS_FLAGS_INTF_THRES_BGN) {
					intf = statsv2->bgnoise;
					if (intf && intf > c_info->acs_policy.bgnoise_thres) {
						candi->valid = FALSE;
						candi->reason |= ACS_INVALID_INTF_BGN;
						break;
					}
				}
				hits ++;
			}
			statsv2++;
		}
		if (hits == 2 || (hits && (chan.ext20 == 0))) {
			done = TRUE;
			break;
		}
	}
	if (!done) {
		ACSD_ERROR("%s: intf check failed for chanspec: 0x%x\n", c_info->name, chspec);
		return;
	}
}

/*
 * 20/40 Coex compliance check:
 * Return a 20/40 Coex compatible chanspec based on the scan data.
 * Verify that the 40MHz input_chspec passes 20/40 Coex rules.
 * If so, return the same chanspec.
 * Otherwise return a 20MHz chanspec which is centered on the
 * input_chspec's control channel.
 */
static chanspec_t
acs_coex_check(acs_chaninfo_t* c_info, chanspec_t input_chspec)
{
	int forty_center;
	uint ctrl_ch, ext_ch;
	acs_channel_t chan;
	chanspec_t chspec_out;
	int ci_index, ninfo = c_info->scan_chspec_list.count;
	bool conflict = FALSE;
	acs_chan_bssinfo_t *ci = c_info->ch_bssinfo;
	char err_msg[128];

	if (!CHSPEC_IS40(input_chspec))
		ACSD_ERROR("%s: input channel spec is not 40MHz!", c_info->name);

	/* this will get us the center of the input 40MHz channel */
	forty_center = CHSPEC_CHANNEL(input_chspec);

	acs_parse_chanspec(input_chspec, &chan);

	ctrl_ch = chan.control;
	ext_ch = chan.ext20;

	ACSD_DEBUG("InputChanspec:  40Center %d, CtrlCenter %d, ExtCenter %d\n",
	          forty_center, ctrl_ch, ext_ch);

	/* Loop over scan data looking for interferance based on 20/40 Coex Rules. */
	for (ci_index = 0; ci_index < ninfo; ci_index++) {
		ACSD_DEBUG("%s: Examining ci[%d].channel = %d, forty_center-5 = %d, "
		          "forty_center+5 = %d\n", c_info->name,
		          ci_index, ci[ci_index].channel, forty_center - WLC_2G_25MHZ_OFFSET,
		          forty_center+WLC_2G_25MHZ_OFFSET);

		/* Ignore any channels not within the range we care about.
		 * 20/40 Coex rules for 2.4GHz:
		 * Must look at all channels where a 20MHz BSS would overlap with our
		 * 40MHz BW + 5MHz on each side.  This means that we must inspect any channel
		 * within 5 5MHz channels of the center of our 40MHz chanspec.
		 *
		 * Example:
		 * 40MHz Chanspec centered on Ch.8
		 *              +5 ----------40MHz-------------  +5
		 *              |  |           |              |   |
		 * -1  0  1  2  3  4  5  6  7  8  9  10  11  12  13  14
		 *
		 * Existing 20MHz BSS on Ch. 1 (Doesn't interfere with our 40MHz AP)
		 *  -----20MHz---
		 *  |     |     |
		 * -1  0  1  2  3  4  5  6  7  8  9  10  11  12  13  14
		 *
		 * Existing 20MHz BSS on Ch. 3 (Does interfere our 40MHz AP)
		 *        -----20MHz---
		 *        |     |     |
		 * -1  0  1  2  3  4  5  6  7  8  9  10  11  12  13  14
		 *
		 *  In this example, we only pay attention to channels in the range of 3 thru 13.
		 */

		if (ci[ci_index].channel < forty_center - WLC_2G_25MHZ_OFFSET ||
		    ci[ci_index].channel > forty_center + WLC_2G_25MHZ_OFFSET) {
			ACSD_DEBUG("%s: Not in range, continue.\n", c_info->name);
			continue;
		}

		ACSD_DEBUG("%s: In range.\n", c_info->name);
		if (ci[ci_index].nCtrl || ci[ci_index].nExt20) {
			/* Is there an existing BSS? */
			ACSD_DEBUG("%s: Existing BSSs on channel %d\n", c_info->name,
				ci[ci_index].channel);

			/* Existing BSS is ONLY okay if:
			 * Our control channel is aligned with existing 20 or Control Channel
			 * Our extension channel is aligned with an existing extension channel
			 */
			if (ci[ci_index].channel == ctrl_ch) {
				ACSD_DEBUG("%s: Examining ctrl_ch\n", c_info->name);

				/* Two problems that we need to detect here:
				 *
				 * 1:  If this channel is being used as a 40MHz extension.
				 * 2:  If this channel is being used as a control channel for an
				 *     existing 40MHz, we must both use the same CTRL sideband
				 */

				if (ci[ci_index].nExt20) {
					acs_snprintf(err_msg, sizeof(err_msg), "ctrl channel: %d"
							" existing ext. channel", ctrl_ch);
					conflict = TRUE;
					break;
				} else if (ci[ci_index].nCtrl) {
					acs_snprintf(err_msg, sizeof(err_msg), "ctrl channel %d"
							" SB not aligned with existing 40BSS",
							ctrl_ch);
					conflict = TRUE;
					break;
				}
			} else if (ci[ci_index].channel == ext_ch) {
				ACSD_DEBUG("%s: Examining ext_ch\n", c_info->name);

				/* Any BSS using this as it's center is an interference */
				if (ci[ci_index].nCtrl) {
					acs_snprintf(err_msg, sizeof(err_msg), "ext channel %d"
							" used as ctrl channel by existing BSSs",
							ext_ch);
					conflict = TRUE;
					break;
				}
			} else {
				/* If anyone is using this channel, it's an conflict */
				conflict = TRUE;
				acs_snprintf(err_msg, sizeof(err_msg),
						"channel %d used by exiting BSSs ",
						ci[ci_index].channel);
				break;
			}
		}
	}

	if (conflict) {
		chspec_out = CH20MHZ_CHSPEC(ctrl_ch);
		if (c_info->rs_info.pref_chspec)
			ACSD_PRINT("%s: COEX: downgraded chanspec 0x%x to 0x%x: %s\n",
				c_info->name, input_chspec, chspec_out, err_msg);
	} else {
		chspec_out = input_chspec;
		ACSD_DEBUG("%s: No conflict found, returning 40MHz chanspec 0x%x\n",
		          c_info->name, chspec_out);
	}
	return chspec_out;
}

bool
acs_is_initial_selection(acs_chaninfo_t* c_info)
{
	bool initial_selection = FALSE;
	chanim_info_t * ch_info = c_info->chanim_info;
	uint8 cur_idx = chanim_mark(ch_info).record_idx;
	uint8 start_idx;
	chanim_acs_record_t *start_record;

	start_idx = MODSUB(cur_idx, 1, ACS_CHANIM_ACS_RECORD);
	start_record = &ch_info->record[start_idx];
	if ((start_idx == ACS_CHANIM_ACS_RECORD - 1) && (start_record->timestamp == 0))
		initial_selection = TRUE;

	ACSD_DFSR("%s: Initial selection is %d\n", c_info->name, initial_selection);
	return initial_selection;
}

static void
acs_invalidate_candidates(acs_chaninfo_t *c_info, ch_candidate_t *candi, int bw)
{
	acs_rsi_t *rsi = &c_info->rs_info;
	bool need_coex_check = BAND_2G(rsi->band_type) &&
	(rsi->bw_cap == WLC_BW_CAP_40MHZ) &&
	rsi->coex_enb;

	bool dfsr_disable = !(acs_dfsr_reentry_type(ACS_DFSR_CTX(c_info)) == DFS_REENTRY_IMMEDIATE);
#if 0
	bool non_dfs_present = FALSE;
#endif
	chanim_info_t * ch_info = c_info->chanim_info;
	time_t now = uptime();
	acs_conf_chspec_t *excl_chans;
	chanspec_t cur_chspec = 0;
	int i, j;
	chanspec_t tmp_chspec, in_chspec = 0, out_chspec = 0;

	excl_chans = &(c_info->excl_chans);
	acs_invalidate_exclusion_channels(c_info->candidate[bw], c_info->c_count[bw],
		excl_chans);

	if (acs_get_chanspec(c_info, &tmp_chspec) < 0) {
		cur_chspec = c_info->selected_chspec;
	} else {
		cur_chspec = tmp_chspec;
	}
#if 0
	if (c_info->acs_start_on_nondfs && BAND_5G(rsi->band_type)) {
		non_dfs_present = acs_check_for_nondfs_chan(c_info, bw);
	}
#endif
	for (i = 0; i < c_info->c_count[bw]; i++) {
		/* going through the  coex check if needed */
		if ((bw == ACS_BW_40) && need_coex_check) {
			in_chspec = candi[i].chspec;
			out_chspec = acs_coex_check(c_info, in_chspec);
			if (in_chspec != out_chspec) {
				candi[i].valid = FALSE;
				candi[i].reason |= ACS_INVALID_COEX;
				continue;
			}
		}

		/* going through the interference check if needed */
		if (c_info->flags & ACS_FLAGS_INTF_THRES_CCA ||
				c_info->flags & ACS_FLAGS_INTF_THRES_BGN) {
			acs_candidate_check_intf(&candi[i], c_info);
		}

		/*
		 * For 20MHz channels, only pick from 1, 6, 11
		 * For 40MHz channels, only pick from control channel being 1, 6, 11 (BT doc)
		 * Mark all the other candidates invalid
		 */

		if (BAND_2G(rsi->band_type)) {
			acs_channel_t chan;
			ACSD_DEBUG("%s: Filter chanspecs for 2G 40/20 MHz channels\n",
				c_info->name);
			acs_parse_chanspec(candi[i].chspec, &chan);

			ACSD_DEBUG("%s: channel: %d, ext: %d\n", c_info->name, chan.control,
				chan.ext20);

			if ((!nvram_match("acs_2g_ch_no_restrict", "1")) &&
					(chan.control != ACS_CHANNEL_1) &&
					(chan.control != ACS_CHANNEL_6) &&
					(chan.control != ACS_CHANNEL_11))  {
				candi[i].valid = FALSE;
				candi[i].reason |= ACS_INVALID_OVLP;
				continue;
			}
			ACSD_DEBUG("%s: valid channel: %d\n", c_info->name, chan.control);

		}

		if (BAND_5G(rsi->band_type)) {
			bool is_initial = FALSE;

			/* Invalidate Unusable DFS channels */
			if (candi[i].is_dfs &&
					!acs_dfs_channel_is_usable(c_info, candi[i].chspec)) {
				ACSD_DEBUG("%s: Invalidating %x - unusable DFS channel\n",
					c_info->name, candi[i].chspec);

				candi[i].valid = FALSE;
				candi[i].reason |= ACS_INVALID_DFS;
			}

			/* Invalidate weather radar channels for bw_upgradable case unless clear */
			if (c_info->bw_upgradable && candi[i].is_dfs && c_info->rs_info.reg_11h &&
					c_info->country_is_edcrs_eu &&
					acs_is_dfs_weather_chanspec(c_info, candi[i].chspec) &&
					ACS_CHINFO_IS_UNCLEAR(acs_get_chanspec_info(c_info,
					candi[i].chspec))) {
				candi[i].reason |= ACS_INVALID_DFS;
			}

			/* when dyn160 is enabled with DFS on FCC, allow ch 50o/subset only */
			if (ACS_11H(c_info) && c_info->dyn160_enabled &&
					CHSPEC_IS160(candi[i].chspec) &&
					!c_info->country_is_edcrs_eu &&
					CHSPEC_CHANNEL(candi[i].chspec) != ACS_DYN160_CENTER_CH) {
				candi[i].valid = FALSE;
				candi[i].reason |= ACS_INVALID_EXCL;
			}

			is_initial = acs_is_initial_selection(c_info);

			/* not boot up ch selection */
			if (!is_initial && (!c_info->bw_upgradable ||
					c_info->acs_dfs == ACS_DFS_DISABLED)) {

				/* Don't invalidate channels when already on 160MHz
				 */
				if (c_info->is160_bwcap && CHSPEC_IS160(c_info->cur_chspec)) {
					continue;
				}

				/* avoid select same channel */
				if (acs_check_for_overlap(cur_chspec, candi[i].chspec) &&
						(c_info->switch_reason != APCS_CSTIMER) &&
						(c_info->txop_weight == 0)) {
					ACSD_5G("%s: Skipping cur chan 0x%x\n",
						c_info->name, cur_chspec);
					candi[i].reason |= ACS_INVALID_SAMECHAN;
				}

				/* avoid ping pong on txfail if channel switched recently */
				if (ACS_11H_AND_BGDFS(c_info) &&
						candi[i].is_dfs &&
						c_info->switch_reason == APCS_TXFAIL &&
						candi[i].chspec == c_info->recent_prev_chspec &&
						(now - c_info->acs_prev_chan_at <
						 ACS_RECENT_CHANSWITCH_TIME)) {
					ACSD_5G("%s Skipping recent chan 0x%x\n", c_info->name,
							c_info->recent_prev_chspec);
					candi[i].reason |= ACS_INVALID_AVOID_PREV;
				}

				/* Use DFS channels if DFS reentry is OK */
				/* In ETSI, avoid weather chan if not pre-cleared on DFSRentry */
				if (!(candi[i].reason & ACS_INVALID_DFS) && candi[i].is_dfs &&
						(dfsr_disable ||
						(c_info->rs_info.reg_11h &&
						c_info->country_is_edcrs_eu &&
						acs_is_dfs_weather_chanspec(c_info,
						candi[i].chspec) &&
						ACS_CHINFO_IS_UNCLEAR(acs_get_chanspec_info(c_info,
						candi[i].chspec))))) {
					candi[i].reason |= ACS_INVALID_DFS;
				}

				if (!candi[i].is_dfs && !dfsr_disable) {
					/* DFS Re-entry - Invalidate Non-DFS channels */
					candi[i].reason |= ACS_INVALID_NONDFS;
				}

			}

			if (is_initial) { /* boot up ch selection (5G) */
				/*
				 * Use DFS channels if we are just coming up
				 * unless
				 *  - DFS is disabled
				 *  - DFS channel is inactive
				 *  - in EU and it is marked as a weather channel
				 */
				if (!(candi[i].reason & ACS_INVALID_DFS) &&
						candi[i].is_dfs &&
						((c_info->acs_dfs == ACS_DFS_DISABLED) ||
						(c_info->rs_info.reg_11h &&
						c_info->country_is_edcrs_eu &&
						acs_is_dfs_weather_chanspec(c_info,
						candi[i].chspec) && !nvram_get_int("acs_dfs_weather")))) {
					/* invalidate the candidate for the current trial */
					candi[i].reason |= ACS_INVALID_DFS;
				}

				if (c_info->country_is_edcrs_eu) {
					/* Invalidate weather radar channel on bootup in ETSI region
					*/
					if (acs_is_dfs_weather_chanspec(c_info, candi[i].chspec) && !nvram_get_int("acs_dfs_weather")) {
						candi[i].reason |= ACS_INVALID_DFS;
					}
				}
#if 0
				if (non_dfs_present && candi[i].is_dfs &&
						c_info->acs_start_on_nondfs) {
					candi[i].reason |= ACS_INVALID_DFS;
				}

				if (!c_info->acs_pref_max_bw && !candi[i].is_dfs &&
						c_info->acs_dfs != ACS_DFS_DISABLED &&
						!c_info->acs_start_on_nondfs) {
					/* Need to pick DFS channels on bootup -
					 * Invalidate Non-DFS channels
					 */
					candi[i].reason |= ACS_INVALID_NONDFS;
				}
#endif
			}
		}
		candi[i].valid = (candi[i].reason == 0);
	}

	if (BAND_5G(rsi->band_type)) {
		acs_dfsr_reentry_done(ACS_DFSR_CTX(c_info));
		/* DFS Re-entry has been done
		 * avoid channel flip and flop, skip the channel which selected in
		 * some amount of time. If txop_weight is set, allow flip-flop in order
		 * to consider broader list of channels.
		 */
		if (c_info->txop_weight == 0) {
			for (i = 0; i < c_info->c_count[bw]; i++) {
				for (j = 0; j < ACS_CHANIM_ACS_RECORD; j++) {
					if (candi[i].chspec == ch_info->record[j].selected_chspc) {
						if (now - ch_info->record[j].timestamp <
								c_info->acs_chan_flop_period) {
							candi[i].valid = FALSE;
							candi[i].reason |=
								ACS_INVALID_CHAN_FLOP_PERIOD;
							j = ACS_CHANIM_ACS_RECORD;
						}
					}
				}
			}
		}
	}
}

/* return TRUE only if the sel_ch chanspec is significantly better than cur_ch chanspec */
bool
acs_channel_compare(acs_chaninfo_t *c_info, chanspec_t cur_ch, chanspec_t sel_ch)
{
	if (!c_info->chanim_stats) {
		ACSD_DEBUG("%s: Don't calculate scores when chanim_stats are empty\n", c_info->name);
		return FALSE;
	}

	int cur_score = acs_get_ch_score(c_info, cur_ch);
	int sel_score = acs_get_ch_score(c_info, sel_ch);
	bool better = ((sel_score + c_info->acs_switch_score_thresh) < cur_score);

	ACSD_INFO("%s: cur:0x%04X score:%d, sel:0x%04X score:%d, sw_sc_th:%d, %s\n",
			c_info->name, cur_ch, cur_score, sel_ch, sel_score,
			c_info->acs_switch_score_thresh, (better ? "Better" : "Avoid"));

	return better;
}

bool
acs_channel_compare_before_fallback(acs_chaninfo_t *c_info, chanspec_t cur_ch, chanspec_t sel_ch)
{
	int cur_score = acs_get_ch_score(c_info, cur_ch);
	int sel_score = acs_get_ch_score(c_info, sel_ch);
	bool better = ((sel_score + c_info->acs_switch_score_thresh_hi) < cur_score);

	ACSD_INFO("%s: cur:0x%04X score:%d, sel:0x%04X score:%d, sw_sc_th:%d, %s\n",
			c_info->name, cur_ch, cur_score, sel_ch, sel_score,
			c_info->acs_switch_score_thresh_hi, (better ? "Better" : "Avoid"));

	return better;
}

bool
acs_select_chspec(acs_chaninfo_t *c_info)
{
	bool need_coex_check = FALSE;
	chanspec_t selected = 0;
	ch_candidate_t *candi;
	int bw = ACS_BW_20, iter = 0;
	acs_rsi_t *rsi = &c_info->rs_info;

	/* if given a chanspec but just need to pass coex check */
	need_coex_check = BAND_2G(rsi->band_type) &&
		(rsi->bw_cap == WLC_BW_CAP_40MHZ) &&
		rsi->coex_enb;

	/* Return when acsd is not in autochannel or coex mode */
	if ((BAND_5G(rsi->band_type) && !AUTOCHANNEL(c_info)) ||
			(BAND_2G(rsi->band_type) && !AUTOCHANNEL(c_info) &&
			!need_coex_check)) {
		ACSD_ERROR("%s: %s called when not in auto mode or not in coex mode (%d)",
				c_info->name, __func__,	c_info->mode);
		return FALSE;
	}

	if (!AUTOCHANNEL(c_info) && BAND_2G(rsi->band_type) && CHSPEC_IS20(rsi->pref_chspec)) {
		ACSD_INFO("%s: honour nvram settings and be on configured chspec\n", c_info->name);
		return FALSE;
	}

	if (CHSPEC_IS2G(rsi->pref_chspec) && CHSPEC_IS40(rsi->pref_chspec) &&
		need_coex_check) {
		selected = acs_coex_check(c_info, rsi->pref_chspec);
		goto done;
	}

	if (WL_BW_CAP_160MHZ(rsi->bw_cap)) {
			bw = ACS_BW_160;
	} else if (WL_BW_CAP_80MHZ(rsi->bw_cap)) {
			bw = ACS_BW_80;
	} else if (WL_BW_CAP_40MHZ(rsi->bw_cap)) {
			bw = ACS_BW_40;
	}

	if (BAND_2G(rsi->band_type) && WL_BW_CAP_40MHZ(rsi->bw_cap)) {
		if (c_info->acs_nonwifi_enable) {
			if (!acs_is_initial_selection(c_info) && c_info->glitch_cnt >
					c_info->acs_chanim_glitch_thresh) {
				if (CHSPEC_IS40(c_info->selected_chspec)) {
					bw = ACS_BW_40;
					goto reduce_bw;
				} else if (CHSPEC_IS20(c_info->selected_chspec)) {
					bw = ACS_BW_20;
				}
			}
		}
	}
recheck:
	ACSD_INFO("%s: Selected BW %d; 0-20Mhz, 3-160Mhz\n", c_info->name, bw);
	/* build the candidate chanspec list */
	acs_build_candidates(c_info, bw);
	candi = c_info->candidate[bw];

	if (!candi) {
		ACSD_DEBUG("%s: No candidates, try again in 2 sec.\n", c_info->name);
		sleep_ms(2000);

		if (iter++ < ACS_BW_DWNGRD_ITERATIONS) {
			bw = bw -1;
			ACSD_INFO("wl %s no channels for BW %d for %d secs; Downgraded BW %d\n",
				c_info->name, bw+1, iter*2, bw);
		}
		goto recheck;
	}

	acs_invalidate_candidates(c_info, candi, bw);

	/* compute channel scores */
	acs_candidate_score(c_info, bw);

	/* if there is at least one valid chanspec */
	if (acs_has_valid_candidate(c_info, bw)) {
		acs_policy_t *a_pol = &c_info->acs_policy;
		if (a_pol->chan_selector)
			selected = a_pol->chan_selector(c_info, bw);
		else
			ACSD_ERROR("%s: chan_selector is null for the selected policy",
				c_info->name);
		goto done;
	} else if (BAND_5G(rsi->band_type)) {
		ACSD_DEBUG("%s no valid channel to select. BW is not adjust. \n", c_info->name);
		/* In 5G we downgrade bandwidth if there is no valid channel can be selected
		 * on 40/80/160/80p80 Mhz.
		 */

		if (bw > ACS_BW_20) {
			ACSD_INFO("%s Downgrading bw to find a proper channel of operation.\n",
				c_info->name);
			goto reduce_bw;
		}

		/* DFSR if channel switch is due to packet loss */
		if (c_info->switch_reason == APCS_TXFAIL) {
			/* since we don't have any non-DFS channels left, allow immediate DFSR */
			acs_dfsr_set_reentry_type(ACS_DFSR_CTX(c_info), DFS_REENTRY_IMMEDIATE);
		}

		return FALSE;
	}

reduce_bw:
	/* if we failed to pick a chanspec, fall back to lower bw */
	if (bw > ACS_BW_20) {
		ACSD_DEBUG("%s: Failed to find a valid chanspec\n", c_info->name);
		bw = bw - 1;
		goto recheck;
	} /* pick a chanspec if we are here */
	else {
		if (BAND_5G(rsi->band_type) && c_info->c_count[bw])
			selected = candi[(c_info->c_count[bw])-1].chspec;
		else
			selected = candi[0].chspec;

		acs_conf_chspec_t *excl_chans = &(c_info->excl_chans);
		int i;
		if (excl_chans && excl_chans->count)
			for (i = 0; i < excl_chans->count; i++)
				if (selected == excl_chans->clist[i]) {
					c_info->selected_chspec = 0;
					return FALSE;
				}
	}

done:
	if (acs_is_initial_selection(c_info) || ((c_info->switch_reason == APCS_TXFAIL) &&
			(selected != c_info->cur_chspec))) {
		ACSD_PRINT("%s: selected channel spec: 0x%4x (%s)\n", c_info->name, selected, wf_chspec_ntoa(selected, chanspecbuf));
		selected = acs_adjust_ctrl_chan(c_info, selected);
		ACSD_PRINT("%s: Adjusted channel spec: 0x%4x (%s)\n", c_info->name, selected, wf_chspec_ntoa(selected, chanspecbuf));
		ACSD_PRINT("%s: selected channel spec: 0x%4x (%s)\n", c_info->name, selected, wf_chspec_ntoa(selected, chanspecbuf));
	}

	if (c_info->fallback_to_primary && CHSPEC_CHANNEL(selected) ==
			CHSPEC_CHANNEL(c_info->cur_chspec)) {
		ACSD_INFO("%s fallback_to_primary:%d matched selected:0x%04x cur:0x%04x\n",
				c_info->name, c_info->fallback_to_primary,
				selected, c_info->cur_chspec);
		c_info->selected_chspec = c_info->cur_chspec;
		return FALSE;
	}

	if (!selected) {
		ACSD_INFO("%s: selected chanspec is zero!!!\n", c_info->name);
		return FALSE;
	}

	if (c_info->cur_chspec == selected) {
		c_info->selected_chspec = selected;
		return FALSE;
	}

	/* compare score of current with selected; if not significantly better, avoid change */
	if (!acs_is_initial_selection(c_info)) {
		if (!acs_channel_compare(c_info, c_info->cur_chspec, selected)) {
			return FALSE;
		}
	}
	c_info->selected_chspec = selected;

	return TRUE;
}

static struct {
                uint reason;
                const char *reason_name;
        } reason_names[] = {
		{ APCS_INIT, "APCS_INIT" },
		{ APCS_IOCTL, "APCS_IOCTL" },
		{ APCS_CHANIM, "APCS_CHANIM" },
		{ APCS_CSTIMER, "APCS_CSTIMER" },
		{ APCS_TXDLY, "APCS_TXDLY" },
		{ APCS_NONACSD, "APCS_NONACSD" },
		{ APCS_DFS_REENTRY, "APCS_DFS_REENTRY" },
		{ APCS_TXFAIL, "APCS_TXFAIL" },
		{ APCS_ZDFS, "APCS_ZDFS" }
        };

const char *reason_string(int rc)
{
	int i;
	const char* reason_name = "UNKNOWN";

	for (i = 0; (i < ARRAYSIZE(reason_names)) && (i < APCS_MAX); i++) {
		if (reason_names[i].reason == rc)
			reason_name = reason_names[i].reason_name;
	}

	return reason_name;
}

void
acs_set_chspec(acs_chaninfo_t * c_info, bool update_dfs_params, int ch_chng_reason)
{
	int ret = 0;
	wl_chan_switch_t csa;
	chanspec_t chspec = c_info->selected_chspec;
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;
	wl_chan_change_reason_t reason;
	reason = (wl_chan_change_reason_t)ch_chng_reason;
	int unit = -1;
	char tmp[32], prefix[32];

	if (c_info->wet_enabled && acs_check_assoc_scb(c_info)) {
		ACSD_INFO("%s: skip acs_set_chspec when ACSD is in WET mode and scb associated\n", c_info->name);
		return;
	}

	if (c_info->txop_channel_select == 0) {
		if (chspec) {
			ACSD_PRINT("acs_set_chspec: 0x%4x (%s) for reason %s\n", chspec, wf_chspec_ntoa(chspec, chanspecbuf), reason_string(c_info->switch_reason));

			bool is_dfs = acs_is_dfs_chanspec(c_info, chspec);
			bool is_dfs_weather = acs_is_dfs_weather_chanspec(c_info, chspec);

			if (reason == WL_CHAN_REASON_CSA) {
				if (c_info->acs_use_csa) {
					ret = acs_csa_handle_request(c_info);
				} else {
					ret = acs_set_chanspec(c_info, chspec);
				}
			} else if (reason == WL_CHAN_REASON_CSA_TO_DFS_CHAN_FOR_CAC_ONLY) {
				memset(&csa, 0, sizeof(csa));
				csa.mode =
					DOT11_CSA_PROPRIETARY_MODE_DO_CAC_AND_RETURN_TO_OLD_CHAN;
				ret = acs_csa_mode_handle_request(c_info, &csa);
			} else if (reason == WL_CHAN_REASON_DFS_AP_MOVE_START) {

				c_info->switch_reason = APCS_DFS_REENTRY;
				/* set mode to WBD_HANDLE_REQUEST */
				c_info->mode = ACS_MODE_MONITOR;
				if (!is_dfs || c_info->switch_reason != APCS_DFS_REENTRY ||
						(ret = acs_bgdfs_attempt(c_info, chspec, FALSE))
						!= BCME_OK) {
					/* fallback to regular set chanspec */
					if (c_info->acs_use_csa) {
						ret = acs_csa_handle_request(c_info);
					} else {
						ret = acs_set_chanspec(c_info, chspec);
					}
				}
				/* revert the mode to fix chanspec */
				c_info->mode = ACS_MODE_FIXCHSPEC;
			} else if ((reason == WL_CHAN_REASON_DFS_AP_MOVE_STUNT) ||
					(reason == WL_CHAN_REASON_DFS_AP_MOVE_ABORTED) ||
					(reason == WL_CHAN_REASON_DFS_AP_MOVE_RADAR_FOUND)) {

				if ((acs_bgdfs->state != BGDFS_STATE_IDLE) &&
						(acs_bgdfs->cap == BGDFS_CAP_TYPE0)) {

					int arg = 0x00;
					arg = ((reason == WL_CHAN_REASON_DFS_AP_MOVE_STUNT) ?
							DFS_AP_MOVE_STUNT: DFS_AP_MOVE_CANCEL);

					ret = acs_bgdfs_set(c_info, arg);
					if (ret != BCME_OK) {
						ACSD_ERROR("%s: Failed dfs_ap_move option %d \n",
							c_info->name, arg);
					} else {
						acs_bgdfs->state = BGDFS_STATE_IDLE;
					}
				}
			} else {
				/* Default behavior for USE_ACSD_DEF_METHOD:
				 * if target channel is a DFS channel on DFS reentry,
				 * attempt bgdfs first.
				 */
				if (!is_dfs || c_info->switch_reason != APCS_DFS_REENTRY ||
					(ACS_11H_AND_BGDFS(c_info) &&
					(ret = acs_bgdfs_attempt(c_info,
						chspec, FALSE)) != BCME_OK)) {
						// fallback to regular set chanspec
						c_info->selected_chspec = chspec;
						ret = acs_set_chanspec(c_info, chspec);
				}
			}
			if (ret == 0 && ACS_11H_AND_BGDFS(c_info)) {
				c_info->cur_is_dfs = is_dfs;
				c_info->cur_is_dfs_weather = is_dfs_weather;
				if (update_dfs_params) {
					acs_dfsr_chanspec_update(ACS_DFSR_CTX(c_info), chspec,
							__FUNCTION__, c_info->name);
					acs_set_dfs_forced_chspec(c_info);
				}
			}
			else if (ret != 0 && ACS_11H_AND_BGDFS(c_info)) {
				ACSD_PRINT("%s: set chanspec 0x%4x (%s) failed!\n",
					c_info->name, chspec, wf_chspec_ntoa(chspec, chanspecbuf));
			}
		} else {
			wl_ioctl(c_info->name, WLC_GET_INSTANCE, &unit, sizeof(unit));
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);

			if (BAND_5G(c_info->rs_info.band_type) &&
				nvram_match(strcat_r(prefix, "mode", tmp), "ap") &&
				nvram_match("location_code", "RU") &&
				nvram_match("acs_band3", "0")) {
				int bw, count = 0;
				for (bw = 0; bw < ACS_BW_MAX; bw++)
					if (acs_has_valid_candidate(c_info, bw))
						count++;

				if (count == 0) {
					ACSD_PRINT("bring down %s for no valid chanspec!\n", c_info->name);
					wl_ioctl(c_info->name, WLC_DOWN, NULL, 0);
				}
			}
		}
	}
}
