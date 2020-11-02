/**
 * AP Channel/Chanspec Selection implementation.
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
 * $Id: wlc_apcs.c 771595 2019-02-01 02:49:51Z $
 */

/**
 * @file
 * @brief
 * 802.11 divides the allocated spectrum (2.4G or 5.2G) into multiple channels. In a crowded
 * environment figuring out which channel for the AP to operate on in order to get better
 * performance can be challenging and time consuming. To solve this issue we provide the 802.11a/b/g
 * channel selection feature to pick up a channel from a list of applicable candidate channels
 * automatically.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriverAPChannelSelection], [PreferredAPAutoChannel]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef APCS

#ifndef AP
#error "AP must be defined to include this module"
#endif  /* AP */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <wlioctl.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wlc_phy_hal.h>
#include <wlc_apcs.h>
#include <wlc_ap.h>
#include <wlc_scb.h>
#include <wlc_lq.h>
#include <wlc_11h.h>
#include <wlc_obss.h>

struct cs_info {
	wlc_info_t *wlc;
	void (*cb)(void *arg, int status);
	void *arg;
	bool bw40;
	uint8 band;
	uint8 reason;
	wlc_bsscfg_t *cfg;
	chanspec_t *scan_chanspec_list;
	int scan_chanspec_list_size;
	int scan_chanspec_count;
	uint cs_scan_timer;	/* periodic auto channel scan timer (seconds) */
	bool cs_scan_ini;	/* Channel Scan started flag */
};
/* Turn this on to treat bAPs equally with other APs
 * By default, if a bAP is detected on a channel, an attempt is made to skip this channel for
 * phytype G. This flag is to not use this policy. For customers who want to have this option,
 * this flag can be added in the makefile.
 *
 * #define APCS_NOBIAS_BMODE
 */

/* XXX - Turn this on for testing the autochannel algorithm using the test vectors in this file
 * Then fire up an AP and set the channel to "0" on the router UI to run the test.  Test output
 * will be sent to serial console.  If you have made changes to this autochannel code and want
 * to test your changes, please set your router to each brand so that the 2.4G and 5G tests
 * are run.
 *
 * Some of the existing tests are written to test against 20/40 coex, but this isn't enabled
 * by default (yet) so if you are having test failures, make sure you have 20/40 Coex enabled.
 *
 * NOTE:  Please do not re-arrange the order of the tests.  Some of the tests are described
 * in Twiki pages by test number.  If you need to add new tests, please add them at the end.
 *
 * #define APCS_TEST
 */

/* Info/stats that each scan channel cares about the BSSs running on that channel
 * and on adjacent channels (aka scan channel info). A scan channel is the channel
 * on which the wl driver passively listen to the beacons sent by the Access Points
 * running on that channel and on adjacent channels. In order to select a channel
 * or a chanspec in a given spectrum the driver needs to scan a selective list of
 * channels approperiate for that spectrum. The channel # in the scan list is always
 * ascending. # of channels to scan depends on the phy type, for example, the driver
 * by default scans channel 1, 6, 11 for 11g phy and all channels for 11n phy.
 */
typedef struct {
	uint8 channel;	/* scan channel # */
	uint8 aAPs;	/* # of 11a/n BSSs seen in 5.2G band */
	uint8 bAPs;	/* # of 11b BSSs seen in 2.4G band */
	uint8 gAPs;	/* # of 11g/n BSSs seen in 2.4G band */
	uint8 lSBs;	/* # of 40MHz 11n BSSs using this as "lower" ctl channel */
	uint8 uSBs;	/* # of 40MHz 11n BSSs using this as "upper" ctl channel */
	uint8 nEXs;	/* # of 40MHz 11n BSSs using this as extension channel */
} wlc_cs_chan_info_t;

/* max # of scan channels */
#define WLC_CS_MAX_SCAN_CHANS	32

/* some channel bounds */
#define WLC_CS_MIN_2G_CHAN	1	/* min channel # in 2G band */
#define WLC_CS_MAX_2G_CHAN	14	/* max channel # in 2G band */
#define WLC_CS_MIN_5G_CHAN	36	/* min channel # in 5G band */
#define WLC_CS_MAX_5G_CHAN	200	/* max channel # in 5G band */

/* default start (hi-power) channel in 5G band for auto-channel */
#define WLC_CS_START_5G_CHAN    52
/* default start (hi-power) channel in 2G band for auto-channel */
#define WLC_CS_START_2G_CHAN    6

/* # of channels on each side of a scan channel that
 * the scan channel can see if it wants to see.
 */
#define WLC_CS_ESOVLP_CHANS(band, phy)	(BAND_5G(band) ? 0 : \
	BAND_2G(band) ? (((phy) == PHY_TYPE_N ||(phy) == PHY_TYPE_SSN || (phy) == PHY_TYPE_LCN || \
		 (phy) == PHY_TYPE_LCN) ? 0 : 2) :	2)

/* min # of channels between any two adjacent scan channels */
#define WLC_CS_SCAN_APART(band, phy)	(WLC_CS_ESOVLP_CHANS(band, phy) * 2)

/* possible min channel # in the band */
#define WLC_CS_MIN_CHAN(band)	(BAND_5G(band) ? WLC_CS_MIN_5G_CHAN : \
				 BAND_2G(band) ? WLC_CS_MIN_2G_CHAN : \
				 0)
/* possible max channel # in the band */
#define WLC_CS_MAX_CHAN(band)	(BAND_5G(band) ? WLC_CS_MAX_5G_CHAN : \
				 BAND_2G(band) ? WLC_CS_MAX_2G_CHAN : \
				 0)

/* Forward delcarations */
#ifdef WL11N
static int wlc_cs_find_next_ciidx(wlc_info_t *wlc, wlc_cs_chan_info_t *ci, int ncis,
                                  int start, int apart);
static int wlc_cs_find_prev_ciidx(wlc_info_t *wlc, wlc_cs_chan_info_t *ci, int ncis,
                                  int start, int apart);
static bool wlc_cs_find_emptychan(cs_info_t *cs, wlc_cs_chan_info_t *ci, int ncis,
                                  int start, int *ciidx, uint8 *chan,
                                  bool slide, int lovlp, int rovlp, int step, int bw,
                                  uint *ch_scanned, int ttchan);
static bool wlc_cs_next_chan_ciidx(cs_info_t *cs, int ncis, int start, int *ciidx);
static bool wlc_cs_prev_chan_ciidx(cs_info_t *cs, int ncis, int start, int *ciidx);
static chanspec_t wlc_cs_sel_coexchanspec(cs_info_t *cs, wlc_cs_chan_info_t *ci, int ncis,
                                          chanspec_t user_chspec);
#endif	/* WL11N */
static chanspec_t wlc_cs_sel_chanspec(cs_info_t *cs, bool bw40, wlc_cs_chan_info_t *ci, int ncis);
static void wlc_cs_parse_scanresults(cs_info_t *cs, wlc_cs_chan_info_t *ci, int *ncis,
                                     int phy_type, chanspec_t *chanspec, int count);
static void wlc_cs_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg);
static void wlc_cs_build_chanspec(wlc_info_t *wlc, int bandtype, int phytype,
                                  chanspec_t *chanspec, int *count);
static void wlc_cs_build_chanspec_ext(wlc_info_t *wlc, int bandtype, int phy_type,
	int min_chan, int max_chan, chanspec_t *chanspec, int *size);
static bool wlc_cs_is_11gbss(wlc_info_t *wlc, wlc_bss_info_t *bi);

static void wlc_cs_watchdog(void *arg);
static int wlc_cs_ioctl(void *hdl, int cmd, void *arg, int len, struct wlc_if *wlcif);

#ifdef WL11N
/* Find backwards the scan channel info whose channel # is the <N>th
 * previous from the channel in the scan channel info indexed by 'ciidx'.
 */
static int
wlc_cs_find_prev_ciidx(wlc_info_t *wlc, wlc_cs_chan_info_t *ci, int ncis,
                       int ciidx, int nth)
{
	int i, last = ciidx;
	ASSERT(ciidx >= 0 && ciidx < ncis);
	ASSERT(nth > 0);
	for (i = ciidx - 1; i >= 0; i --) {
		if (ci[ciidx].channel - ci[i].channel > nth)
			break;
		last = i;
	}
	return last;
}

/* Find forwards the scan channel info whose channel # is the <N>th
 * next to the channel in the scan channel info indexed by 'ciidx'.
 */
static int
wlc_cs_find_next_ciidx(wlc_info_t *wlc, wlc_cs_chan_info_t *ci, int ncis,
                       int ciidx, int nth)
{
	int i, last = ciidx;
	ASSERT(ciidx >= 0 && ciidx < ncis);
	ASSERT(nth > 0);
	for (i = ciidx + 1; i < ncis; i ++) {
		if (ci[i].channel - ci[ciidx].channel > nth)
			break;
		last = i;
	}
	return last;
}

/* Find an empty <N>MHz channel beginning from the channel in the scan channel
 * info indexed by 'start'. The scan channel to look at will be moved if 'slide'
 * is set to 1. Search can be forwards or backwards depending on 'step'. Param.
 * 'ovlp' indicates how many adjacent channels should be checked for bleeding.
 * Return TRUE if an empty <N>MHz channel is found. Update 'chan' with the center
 * channel of the <N>MHz channel found and 'ciidx' with the last scan channel info
 * whose channel is the ending channel of the found <N>MHz channel.
 */
static bool
wlc_cs_find_emptychan(cs_info_t *cs, wlc_cs_chan_info_t *ci, int ncis,
                      int start, int *ciidx, uint8 *chan,
                      bool slide, int lovlp, int rovlp, int step, int bw,
                      uint *ch_scanned, int ttchan)
{
	wlc_info_t *wlc = cs->wlc;
	int first = start, last;
	int begin, end, i;
	uint chans = 0, aps;
	uint ttlchs, hidden;
	uint low, high;

	*ch_scanned = 0;

	ASSERT(start >= 0 && start < ncis);
	ASSERT(lovlp >= 0 && lovlp <= CH_10MHZ_APART);
	ASSERT(rovlp >= 0 && rovlp <= CH_10MHZ_APART);
	ASSERT(step == 1 || step == -1);

	WL_INFORM(("wl%d: find 20MHz empty channel from %d slide %d left %d right %d "
	           "step %d bw %d\n",
	           wlc->pub->unit, ci[first].channel, slide, lovlp, rovlp, step, bw));

	/* Find <n> consecutive empty channels required to make one <N>MHz channel */

	last = first - step; /*  adjust back to 0 */
	while ((int)(*ch_scanned) < ttchan) {
		*ch_scanned += 1;
		last += step;
		/* don't skip the last 20MHz on A band, it can't form 40Mhz channel */

		if ( slide ) {          /* Wraps around only if slide is set */
			if (last == ncis) {
				last = 0;
			}
			if (last < 0) {
				last = ncis - step;  /* wrapping from backward */
			}
		} else {
				if (last >= ncis || last < 0) {
					last -= step;
					break;
				}
		}

		/* Include adjacent overlapping/bleeding channels if needed */
		begin = lovlp ? wlc_cs_find_prev_ciidx(wlc, ci, ncis, last, lovlp) : last;
		end = rovlp ? wlc_cs_find_next_ciidx(wlc, ci, ncis, last, rovlp) : last;
		ASSERT(begin <= end);

		/* 5G and 2.4G are non overlapping bands so don't worry about
		 * the BSSs in different bands and just add <x>APs together.
		 * Also count 11n extensions.  Use the lovlp/rovlp with the IEEE channel
		 * numbers to make sure the APs counted actually do overlap.  This is
		 * necessary since 5G channels are seperated by 20MHz of bw and the
		 * spectrum is not contiguous.
		 */
		for (i = begin, aps = 0; i <= end; i ++)
			if (((int)ci[i].channel >= (int)(ci[last].channel-lovlp)) &&
			   ((int)ci[i].channel <= (int)(ci[last].channel+rovlp)))
				aps += ci[i].aAPs + ci[i].bAPs + ci[i].gAPs + ci[i].nEXs;

		/* Reset the channel count and move to the next scan channel
		 * if the current scan channel (plus adjacent) is not empty.
		 */
		if ((aps != 0) ||
			wlc_lq_chanim_interfered(wlc, CH20MHZ_CHSPEC(ci[last].channel))) {
			if (!slide)
				break;

			first = last + step;
			if (first > (ncis -1))
				first = 0;
			if (first  < 0)
				first = ncis - step;

			chans = 0;
			continue;
		}

		/* Found an empty channel in 5G band, take it as <N>MHz channel */
		if (BAND_5G(cs->band)) {
			ASSERT(bw == 20);
			*chan = ci[last].channel;
		}
		/* Found an empty channel in 2.4G band, check if it along with other
		 * channels can make a <N>MHz empty channel.
		 */
		else {
			/* Figure out how many channels can form a <N>MHz channel */
			switch (bw) {
			case 20:
				ttlchs = CH_20MHZ_APART + 1;
				break;
			default:
				ASSERT(!"unsupported bandwidth");
				return FALSE;
			}

			/* Normallize channel range to 'low' and 'high' */
			low = ci[first].channel < ci[last].channel ?
			        ci[first].channel : ci[last].channel;
			high = ci[last].channel > ci[first].channel ?
			        ci[last].channel : ci[first].channel;
			ASSERT(low <= high);

			/* The following code will not work unless we scan all channels!
			 * - We are counting each scan channel as a single channel
			 * - We are expecting ci[0] the first and ci[ncis-1] the last channel.
			 */
			chans ++;

			/* Count the 'hidden' channels at the top or bottom of the
			 * spectrum.
			 */
			if (low == ci[0].channel || high == ci[ncis - 1].channel)
				hidden = CH_10MHZ_APART;
			else
				hidden = 0;
			if (chans + hidden < ttlchs)
			{
				if ((last + step) > ncis) {
					first = 0;
					chans = 0;
				}
				if ((last + step) < 0) {
					first = ncis - step;
					chans = 0;
				}
				continue;
			}

			/* Adjust 'low' and 'high' if hidden channels are accounted
			 * for the <N>MHz channel to make sure the <N>MHz channel
			 * is properly centered.
			 */
			if (chans < ttlchs) {
				if (low == ci[0].channel)
					low -= ttlchs - chans;
				if (high == ci[ncis - 1].channel)
					high += ttlchs - chans;
			}

			/* Channels from 'low' to 'high' can form an empty <N>MHz channel! */
			*chan = (uint8)((low + high) / 2);
		}

		WL_INFORM(("wl%d: found %dMHz empty channel %d\n", wlc->pub->unit, bw, *chan));

		*ciidx = last;
		return TRUE;
	}

	*ciidx = last;
	return FALSE;
}

/* Point 'ciidx' to the scan channel info whose channel is the beginning
 * channel of the next <N>MHz channel. 'start' is the index of the scan
 * channel whose channel is the ending channel of the current <N>MHz
 * channel.
 * Return TRUE if the updated 'ciidx' is still within the range.
 */
static bool
wlc_cs_next_chan_ciidx(cs_info_t *cs, int ncis, int start, int *ciidx)
{
	int next = BAND_5G(cs->band) ? start + 1 : start;
	*ciidx = next;
	return next >= 0 && next < ncis;
}

/* Point 'ciidx' to the scan channel info whose channel is the ending
 * channel of the previous <N>MHz channel. 'start' is the index of the
 * scan channel whose channel is the beginning channel of the current
 * <N>MHz channel.
 * Return TRUE if the updated 'ciidx' is still within the range.
 */
static bool
wlc_cs_prev_chan_ciidx(cs_info_t *cs, int ncis, int start, int *ciidx)
{
	int next = BAND_5G(cs->band) ? start - 1 : start;
	*ciidx = next;
	return next >= 0 && next < ncis;
}

/* Return a 20/40 Coex compatible chanspec based on the scan data.
 * Verify that the 40MHz input_chspec passes 20/40 Coex rules.  If so, return the same chanspec
 * otherwise return a 20MHz chanspec which is centered on the input_chspec's control channel.
 */
static chanspec_t
wlc_cs_sel_coexchanspec(cs_info_t *cs, wlc_cs_chan_info_t *ci, int ninfo,
                        chanspec_t input_chspec)
{
	wlc_info_t *wlc = cs->wlc;
	wlc_bsscfg_t *cfg = cs->cfg;
	int forty_center, ctrl_ch, ext_ch;
	bool upperSB;
	chanspec_t chspec_out;
	int chan_index;
	bool interference = FALSE;
	char err_msg[128];

	(void)wlc;

	ASSERT(CHSPEC_IS40(input_chspec));

	/* XXX - Need to look into fixing this code and the autochannel algo to look for
	 *  40MHz intolerant bits!!!
	 */

	WL_COEX(("wlc_cs_sel_coexchanspec: Input chanspec 0x%x\n", input_chspec));

	/* the following code assumes that we're trying to be a 40MHz AP */
	ASSERT(WL_BW_CAP_40MHZ(wlc->band->bw_cap) == TRUE);

	/* this will get us the center of the input 40MHz channel */
	forty_center = CHSPEC_CHANNEL(input_chspec);

	upperSB = CHSPEC_SB_UPPER(input_chspec);

	/* Calculate the control channel based on chanspec being upper/lower sb */
	ctrl_ch = upperSB ? forty_center + CH_10MHZ_APART :
	        forty_center - CH_10MHZ_APART;
	ext_ch = upperSB ? forty_center - CH_10MHZ_APART :
	        forty_center + CH_10MHZ_APART;

	WL_COEX(("InputChanspec:  40Center %d, CtrlCenter %d, ExtCenter %d\n",
	          forty_center, ctrl_ch, ext_ch));

	/* Loop over scan data looking for interferance based on 20/40 Coex Rules. */
	for (chan_index = 0; chan_index < ninfo; chan_index++) {
		WL_COEX(("Examining ci[%d].channel = %d, forty_center-5 = %d, "
		          "forty_center+5 = %d\n",
		          chan_index, ci[chan_index].channel, forty_center-WLC_2G_25MHZ_OFFSET,
		          forty_center+WLC_2G_25MHZ_OFFSET));

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

		if (ci[chan_index].channel < forty_center-WLC_2G_25MHZ_OFFSET ||
		    ci[chan_index].channel > forty_center+WLC_2G_25MHZ_OFFSET) {
			WL_INFORM(("Not in range, continue.\n"));
			continue;
		}

		WL_INFORM(("In range.\n"));

		/* Is there an existing BSS? */
		if (ci[chan_index].bAPs || ci[chan_index].gAPs || ci[chan_index].lSBs ||
		    ci[chan_index].uSBs || ci[chan_index].nEXs) {
			WL_INFORM(("Existing BSSs on channel %d\n", ci[chan_index].channel));

			/* Interference is ONLY okay if:
			 * Our control channel is aligned with existing 20 or Control Channel
			 * Our extension channel is aligned with an existing extension channel
			 */
			if (ci[chan_index].channel == ctrl_ch) {
				WL_COEX(("Examining ctrl_ch\n"));

				/* Two problems that we need to detect here:
				 *
				 * 1:  If this channel is being used as a 40MHz extension.
				 * 2:  If this channel is being used as a control channel for an
				 *     existing 40MHz, we must both use the same CTRL sideband
				 */
				if (ci[chan_index].nEXs) {
					snprintf(err_msg, sizeof(err_msg), "ctrl channel: %d"
						" existing ext. channel", ctrl_ch);
					interference = TRUE;
					break;
				} else if ((upperSB && ci[chan_index].lSBs) ||
				           (!upperSB && ci[chan_index].uSBs)) {
					snprintf(err_msg, sizeof(err_msg), "ctrl channel %d"
						" SB not aligned with existing 40BSS", ctrl_ch);
					interference = TRUE;
					break;
				}
			} else if (ci[chan_index].channel == ext_ch) {
				WL_COEX(("Examining ext_ch\n"));

				/* Any BSS using this as it's center is an interferance */
				if (ci[chan_index].bAPs || ci[chan_index].gAPs ||
				    ci[chan_index].lSBs || ci[chan_index].uSBs) {
					snprintf(err_msg, sizeof(err_msg), "ext channel %d"
						" used as ctrl channel by existing BSSs", ext_ch);
					interference = TRUE;
					break;
				}
			} else {
				/* If anyone is using this channel, it's an interference */
				interference = TRUE;
				snprintf(err_msg, sizeof(err_msg),
					"channel %d used by exiting BSSs ", ci[chan_index].channel);
				break;
			}
		}
	}

	if (interference) {
		chspec_out = CH20MHZ_CHSPEC(ctrl_ch);
		if (COEX_ACTIVE(wlc->obss, cfg)) {
			wlc_ht_coex_update_permit(cfg, FALSE);
			wlc_ht_coex_update_fid_time(cfg);
		}
		WL_PRINT(("COEX: downgraded chanspec 0x%x to 0x%x: %s\n",
			input_chspec, chspec_out, err_msg));
	} else {
		chspec_out = input_chspec;
		if (COEX_ACTIVE(wlc->obss, cfg))
			wlc_ht_coex_update_permit(cfg, TRUE);
		WL_COEX(("No interference found, returning 40MHz chanspec 0x%x\n",
		          chspec_out));
	}
	return chspec_out;
}
#endif	/* WL11N */

/*
* Pick a channel based on the following rules:
*
* general:
*  - a channel that has the least # of BSSs
* 11g:
*  - a channel that has only 11g BSSs and has the least # of BSSs
*  - a channel using general selection
* 11n:
*  - a center channel of two adjacent, unoccupied, 20Mhz channels
*  - a center channel of an existing 40Mhz channel that has the least
*    # of 40MHz 11n BSSs
*  - a center channel of an existing 20Mhz channel and a non-occupied,
*    adjacent, 20Mhz channel
*  - a channel using 11g selection for 2.4GHz band, or a channel using
*    general selection for 5G band
*/
static chanspec_t
wlc_cs_sel_chanspec(cs_info_t *cs, bool bw40, wlc_cs_chan_info_t *cinfo, int ninfo)
{
	wlc_info_t *wlc = cs->wlc;
	wlc_bsscfg_t *cfg = cs->cfg;
	int i, j;
	uint8 chan;
	int bandtype = cs->band;
	int start_ch = 0;
#ifdef WL11N
	uint ch_scan1, ch_scan2, tot_ch = 0;
	int i1, i2;
	uint8 chan1 = 0, chan2;
	uint8 chan2020;
	bool inband = TRUE;
	int ttlchan;
#endif	/* WL11N */
	chanspec_t chspec;
	uint16 aps;
	wlc_cs_chan_info_t new_cinfo[WLC_CS_MAX_SCAN_CHANS];
	uint phy_type = wlc->band->phytype;
	wlc_cs_chan_info_t *ci = cinfo;
	int ncis = ninfo;

	(void)cfg;

#ifdef WL11N_20MHZONLY
	ASSERT(!bw40);
	bw40 = FALSE;	/* Override to force compiler optimizations */
#endif // endif

	/* select channel based on phy type */
sel:	chan = 0;
	chspec = 0;
	aps = MAXNBVAL(sizeof(aps));

	if (BAND_5G(bandtype)) {
		/* for 5G, start channel selection from a hi power channel */
		for (i = 0; i < ncis; i++) {
			if (ci[i].channel >= WLC_CS_START_5G_CHAN) {
				start_ch = i;
				break;
			}
		}
	}
	else {
		/* for 2G, start channel selection from a hi power channel */
		for (i = 0; i < ncis; i++) {
			if (ci[i].channel >= WLC_CS_START_2G_CHAN) {
				start_ch = i;
				break;
			}
		}
	}

	switch (phy_type) {
	case PHY_TYPE_G:
		/* choose the 11g only channel that has the least number of 11g BSSs */
		for (j = 0; j < ncis; j ++) {
			i = (start_ch+j)%ncis;
#ifdef APCS_NOBIAS_BMODE
			if (((ci[i].gAPs + ci[i].bAPs) < aps) &&
				!wlc_lq_chanim_interfered(wlc, CH20MHZ_CHSPEC(ci[i].channel))) {
				aps = ci[i].gAPs + ci[i].bAPs;
				chan = ci[i].channel;
			}
#else
			if (ci[i].bAPs != 0)
				continue;

			if ((ci[i].gAPs < aps) &&
				!wlc_lq_chanim_interfered(wlc, CH20MHZ_CHSPEC(ci[i].channel))) {
				aps = ci[i].gAPs;
				chan = ci[i].channel;
			}
#endif /* APCS_NOBIAS_BMODE */
		}
		/* found a 11g only channel */
		if (chan != 0) {
			chspec = CH20MHZ_CHSPEC(chan);
			break;
		}

		/* failed to find a 11g only channel because 11b BSSs are all over
		 * the place so let it fall here to mess 11g with 11b...
		 */

	case PHY_TYPE_A:
		/* choose the channel that has the least number of 11a/b/g BSSs */
		for (j = 0; j < ncis; j ++) {
			i = (start_ch+j)%ncis;
			/* 5G and 2.4G are non overlapping bands so don't worry about
			 * the BSSs in different bands and just add <x>APs together.
			 */
			if ((phy_type == PHY_TYPE_G) &&
			    wlc_lq_chanim_interfered(wlc, CH20MHZ_CHSPEC(ci[i].channel)))
				continue;

			if (ci[i].aAPs + ci[i].bAPs + ci[i].gAPs < aps) {
				aps = ci[i].aAPs + ci[i].bAPs + ci[i].gAPs;
				chan = ci[i].channel;
			}
		}

		/* pick the first channel in the list if not found */
		if (!chan)
			chan = ci[start_ch%ncis].channel;

		chspec = CH20MHZ_CHSPEC(chan);
		break;

	case PHY_TYPE_LP:
		CASECHECK(PHYTYPE, PHY_TYPE_LP);

		/* Reuse PHY_TYPE_A or PHY_TYPE_G channel selection algorithm */
		if (BAND_2G(bandtype))
			phy_type = PHY_TYPE_G;
		else
			phy_type = PHY_TYPE_A;
		goto sel;

	case PHY_TYPE_N:
#ifdef WL11N
		/* Find an empty 40MHz channel (two consecutive 20MHz)
		 * Only perform the check if we are enabled for 40MHz operation
		 */
		tot_ch = 0;
		if (BAND_5G(bandtype)) {
			i = start_ch;
			ttlchan = ncis;
		} else {
			/* 2G band is little tricky. To Avoid the hidden APs,
			 * we need start from channel (start_ch-2) and finish
			 * at  channel (end +2). In another word we need overlap
			 * the channels, 2 channels from left and 2 from right.
			 */
			i = start_ch - CH_10MHZ_APART;
			ttlchan = ncis + 2 * CH_10MHZ_APART;
		}

		if ((WL_BW_CAP_40MHZ(wlc->band->bw_cap)) && N_ENAB(wlc->pub) && bw40)  {
			while ((int)tot_ch < ttlchan) {
				if (wlc_cs_find_emptychan(cs, ci, ncis, i, &i, &chan1,
				          TRUE, CH_10MHZ_APART, CH_10MHZ_APART, 1, 20,
				          &ch_scan1, ttlchan) &&
				    (inband = wlc_cs_next_chan_ciidx(cs, ncis, i, &i1)) &&
				    wlc_cs_find_emptychan(cs, ci, ncis, i1, &i, &chan2,
				          FALSE, 0, CH_10MHZ_APART, 1, 20, &ch_scan2, ttlchan)) {
					chan = (chan1 + chan2) / 2;
					chspec = CH40MHZ_CHSPEC(chan, WL_CHANSPEC_CTL_SB_LOWER);
					if (!wlc_valid_chanspec_db(wlc->cmi, chspec)) {
						chan = 0;
						tot_ch += ch_scan1;
						continue;
					}
					break;
				}
				tot_ch += ch_scan1;
				if (!inband)
					break;
			}
			/* Found an empty 40MHz channel */
			if (chan) {
				WL_INFORM(("wl%d: found empty 40MHz channel %d\n",
				            wlc->pub->unit, chan));
				break;
			}

			WL_INFORM(("No Empty 40MHz channel, moving on...\n"));

			if (BAND_2G(bandtype)) {
				aps = MAXNBVAL(sizeof(aps));

				/* Find least occupied existing 40MHz channel that doesn't violate
				 * 20/40 Coex rules.
				 */
				for (j = 0; j < ncis; j++) {
					chanspec_t in = 0;
					chanspec_t out = 0;

					i = (start_ch+j)%ncis;

					/* Skip empty channels, they're not of interest in this
					 * part of the algorithm, we're looking for existing 40MHz
					 * Control channels
					 */
					if (ci[i].lSBs + ci[i].uSBs == 0)
						continue;

					/* If this channel is being used as an upperSB *AND*
					 * lowerSB then someone is in violation of coex rules, we
					 * can't land here
					 */
					if (ci[i].lSBs && ci[i].uSBs)
						continue;

					/* Make a chanspec that matches the 40MHz BSS we're trying
					 * to align ourselves with.  Then see if it passes coex
					 * rules
					 */
					if (ci[i].uSBs)
						in = CH40MHZ_CHSPEC(ci[i].channel-2,
						                        WL_CHANSPEC_CTL_SB_UPPER);
					else
						in = CH40MHZ_CHSPEC(ci[i].channel+2,
						                        WL_CHANSPEC_CTL_SB_LOWER);

					if (wlc_valid_chanspec_db(wlc->cmi, in)) {
						out = wlc_cs_sel_coexchanspec(cs, ci, ncis, in);
					} else {
						/* Hmmm... something weird happened.  Existing
						 * 40MHz BSS isn't a valid chanspec?  Move on...
						 */
						WL_INFORM(("wlc_cs_sel_chanspec:  invalid "
						          "chanspec!\n"));
						continue;
					}

					/* If this is a 20/40 Coex compatible chanspec.  Keep
					 * track of this info while we continue to look for a
					 * lesser occupied section of the band.
					 */
					if (out == in) {
						uint16 temp = ci[i].uSBs + ci[i].lSBs +
						        ci[i].gAPs + ci[i].bAPs;

						if (temp <= aps) {
							aps = temp;
							chspec = out;
							chan = CHSPEC_CHANNEL(out);
						}
					}
				}
				/* Found an existing 40MHz channel */
				if (chan) {
					WL_INFORM(("wl%d: found existing 40MHz channel %d\n",
					          wlc->pub->unit, chan));
					break;
				}
			}

			/* Find an existing 40MHz channel that has the least # of 11n BSSs */
			aps = MAXNBVAL(sizeof(aps));

			for (j = 0; j < ncis; j++) {
				uint8 tmp_chan;
				chanspec_t tmp_chspec;
				i = (start_ch+j)%ncis;

				/* Skip empty channels, they're not of interest, we're looking
				 * for existing 40MHz channels
				 * also skip channels where the number of APs is greater than
				 * the previous *best channel*
				 */
				if (ci[i].lSBs + ci[i].uSBs == 0 ||
				    ci[i].lSBs + ci[i].uSBs >= aps)
					continue;
				aps = ci[i].lSBs + ci[i].uSBs;
				if (ci[i].lSBs == 0 ||
				    (ci[i].uSBs != 0 && ci[i].uSBs < ci[i].lSBs)) {
					tmp_chan = ci[i].channel - CH_10MHZ_APART;
					tmp_chspec = CH40MHZ_CHSPEC(tmp_chan,
						WL_CHANSPEC_CTL_SB_UPPER);
				} else {
					tmp_chan = ci[i].channel + CH_10MHZ_APART;
					tmp_chspec = CH40MHZ_CHSPEC(tmp_chan,
						WL_CHANSPEC_CTL_SB_LOWER);
				}
				if (!wlc_valid_chanspec_db(wlc->cmi, tmp_chspec))
					continue;

				chan = tmp_chan;
				chspec = tmp_chspec;
			}
			/* Found an existing 40MHz channel */
			if (chan) {
				WL_INFORM(("wl%d: found existing 40MHz channel %d\n",
				            wlc->pub->unit, chan));
				/* Continue to the following search to find out a least occupied
				 * channel amongst the existing 40MHz BSSs and the existing 20MHz
				 * BSSs and adjacent empty 20MHz channels.
				 */
				/* break; */
			}

			/* Find an existing 20MHz and an empty 20MHz channel to form
			 * a 40 MHz channel
			 */
			/* aps = MAXNBVAL(sizeof(aps)); */
			chan2020 = 0;
			for (j = 0; j < ncis; j++) {
				i = (start_ch+j)%ncis;
				if (ci[i].lSBs + ci[i].uSBs != 0)
					continue;
				/* 5G and 2.4G are non overlapping bands so don't worry about
				 * the BSSs in different bands and just add <x>APs together.
				 */
				if (ci[i].aAPs + ci[i].bAPs + ci[i].gAPs == 0 ||
				    ci[i].aAPs + ci[i].bAPs + ci[i].gAPs >= aps)
					continue;
				chan1 = ci[i].channel;
				i1 = wlc_cs_find_next_ciidx(wlc, ci, ncis, i, CH_10MHZ_APART);
				i2 = wlc_cs_find_prev_ciidx(wlc, ci, ncis, i, CH_10MHZ_APART);
				if (wlc_cs_next_chan_ciidx(cs, ncis, i1, &i1) &&
				    wlc_cs_find_emptychan(cs, ci, ncis, i1, &i1, &chan2, FALSE,
				                        CH_10MHZ_APART - 1, CH_10MHZ_APART,
				                        1, 20, &ch_scan1, ncis)) {
					uint8 temp = (chan1 + chan2) / 2;
					chspec = CH40MHZ_CHSPEC(temp, WL_CHANSPEC_CTL_SB_LOWER);
					if (!wlc_valid_chanspec_db(wlc->cmi, chspec))
						continue;
					aps = ci[i].aAPs + ci[i].bAPs + ci[i].gAPs;
					chan2020 = temp;
				}
				else if (wlc_cs_prev_chan_ciidx(cs, ncis, i2, &i2) &&
				         wlc_cs_find_emptychan(cs, ci, ncis, i2, &i2, &chan2,
				                                FALSE, CH_10MHZ_APART,
				                                CH_10MHZ_APART - 1, -1, 20,
				                                &ch_scan2, ncis)) {
					uint8 temp = (chan1 + chan2) / 2;
					chspec = CH40MHZ_CHSPEC(temp, WL_CHANSPEC_CTL_SB_UPPER);
					if (!wlc_valid_chanspec_db(wlc->cmi, chspec))
						continue;
					aps = ci[i].aAPs + ci[i].bAPs + ci[i].gAPs;
					chan2020 = temp;
				}
			}
			/* Found an existing 20MHz channel and an empty 20MHz channel */
			if (chan2020) {
				WL_INFORM(("wl%d: found existing 20MHz channel %d and "
				           "empty 20MHz channel %d\n", wlc->pub->unit,
				           chan1, chan2));
				chan = chan2020;
				break;
			}

			if (BAND_2G(bandtype) && COEX_ACTIVE(wlc->obss, cfg)) {
				ASSERT(wlc->ap->pref_chanspec == 0);
				/* need to bail out here and let the G or A phy algorithm run to
				 * find the best 20MHz channel.
				 */
			} else {
				/* Honor existing 40MHz if no existing and empty 20MHz next
				 * to each other
				 */
				/* XXX - This behavior should go away soon, but marketing wants
				 * it to stay for a while longer.  We eventually want the
				 * autochannel algorithm to be 20/40 Coex compatible all the time.
				 */
				if (chan)
					break;
			}
		}
#endif	/* WL11N */

		/* Mess 11n with 11a or 11g based on the band type. Reuse
		 * PHY_TYPE_A or PHY_TYPE_G channel selection algorithm.
		 */
		if (BAND_2G(bandtype)) {
#ifdef APCS_TEST
			uint count = 0;
			int c, last;

			WL_INFORM(("wl%d: trying 11g channel selection...\n", wlc->pub->unit));

			phy_type = PHY_TYPE_G;

			last = WLC_CS_MIN_CHAN(bandtype) - 1;
			for (i = WLC_CS_MIN_CHAN(bandtype);
			     i <= WLC_CS_MAX_CHAN(bandtype) && count < WLC_CS_MAX_SCAN_CHANS;
			     i ++) {
				int i1, i2;
				if (last != WLC_CS_MIN_CHAN(bandtype) - 1 &&
				    i - last <= WLC_CS_SCAN_APART(bandtype, phy_type))
					continue;
				if (!VALID_CHANNEL20_DB(wlc, i))
					continue;
				bzero(&new_cinfo[count], sizeof(new_cinfo[count]));
				new_cinfo[count].channel = (uint8)i;
				i1 = i;
				if (i1 - WLC_CS_ESOVLP_CHANS(bandtype, phy_type) >=
				    WLC_CS_MIN_CHAN(bandtype))
					i1 -= WLC_CS_ESOVLP_CHANS(bandtype, phy_type);
				else {
					for (i1 = WLC_CS_MIN_CHAN(bandtype); i1 <= i; i1 ++)
						if (VALID_CHANNEL20_DB(wlc, i1))
							break;
				}
				i2 = i;
				if (i2 + WLC_CS_ESOVLP_CHANS(bandtype, phy_type) <=
				    WLC_CS_MAX_CHAN(bandtype))
					i2 += WLC_CS_ESOVLP_CHANS(bandtype, phy_type);
				else {
					for (i2 = WLC_CS_MAX_CHAN(bandtype); i2 >= i; i2 --)
						if (VALID_CHANNEL20_DB(wlc, i2))
							break;
				}
				for (c = 0; c < ncis; c ++) {
					if (ci[c].channel < i1 || ci[c].channel > i2)
						continue;
					new_cinfo[count].aAPs += ci[c].aAPs;
					new_cinfo[count].bAPs += ci[c].bAPs;
					new_cinfo[count].gAPs += ci[c].gAPs;
				}
				count ++;
				last = i;
			}

			ci = new_cinfo;
			ncis = count;
#else
			chanspec_t chanspec[WLC_CS_MAX_SCAN_CHANS];
			int count = WLC_CS_MAX_SCAN_CHANS;
			int cur_chan, min_chan, max_chan;

			WL_INFORM(("wl%d: trying 11g channel selection...\n", wlc->pub->unit));

			phy_type = PHY_TYPE_G;

			/* search min/max channel number from scan_chansepc_list */
			min_chan = max_chan = wf_chspec_ctlchan(cs->scan_chanspec_list[0]);
			for (i = 1; i < cs->scan_chanspec_count; i++) {
				cur_chan = wf_chspec_ctlchan(cs->scan_chanspec_list[i]);
				if (cur_chan < min_chan)
					min_chan = cur_chan;
				if (cur_chan > max_chan)
					max_chan = cur_chan;
			}

			wlc_cs_build_chanspec_ext(wlc, bandtype, PHY_TYPE_G,
				min_chan, max_chan, chanspec, &count);

			ci = new_cinfo;
			ncis = ARRAYSIZE(new_cinfo);
			wlc_cs_parse_scanresults(cs, ci, &ncis, PHY_TYPE_G, chanspec, count);
#endif	/* APCS_TEST */
			goto sel;
		}
		else if (BAND_5G(bandtype)) {
			WL_INFORM(("wl%d: trying 11a channel selection...\n", wlc->pub->unit));

			phy_type = PHY_TYPE_A;

			goto sel;
		}
		else {
			ASSERT(!"NPHY bandtype unknown");
			break;
		}

	case PHY_TYPE_SSN:
		CASECHECK(PHYTYPE, PHY_TYPE_SSN);

		/* Reuse PHY_TYPE_N channel selection algorithm */
		phy_type = PHY_TYPE_N;
		goto sel;

	case PHY_TYPE_LCN:
		CASECHECK(PHYTYPE, PHY_TYPE_LCN);

		/* Reuse PHY_TYPE_N channel selection algorithm */
		phy_type = PHY_TYPE_N;
		goto sel;

	case PHY_TYPE_LCN40:
		CASECHECK(PHYTYPE, PHY_TYPE_LCN40);

		/* Reuse PHY_TYPE_N channel selection algorithm */
		phy_type = PHY_TYPE_N;
		goto sel;

	case PHY_TYPE_AC:
		CASECHECK(PHYTYPE, PHY_TYPE_AC);

		/* Reuse PHY_TYPE_N channel selection algorithm */
		phy_type = PHY_TYPE_N;
		goto sel;

	default:
		WL_ERROR(("wl%d: unknown PHY type %d\n", wlc->pub->unit, phy_type));
		break;
	}

	/* return the channel chosen */
	WL_INFORM(("wl%d: selected channel %d bandwidth %s ctl %s for phy type %d\n",
	           wlc->pub->unit, chan, CHSPEC_IS40(chspec) ? "40MHz" : "20MHz",
	           CHSPEC_SB_LOWER(chspec) ? "lower" :
	           CHSPEC_SB_UPPER(chspec) ? "upper" : "none",
	           wlc->band->phytype));
	return chspec;
}

/* Check if the BSS is 11g by checking if its rateset has OFDM rate(s) */
static bool
wlc_cs_is_11gbss(wlc_info_t *wlc, wlc_bss_info_t *bi)
{
	uint r;
	for (r = 0; r < bi->rateset.count; r ++)
		if (IS_OFDM(bi->rateset.rates[r]))
			return TRUE;
	return FALSE;
}

/* Collect channel info from scan results */
static void
wlc_cs_parse_scanresults(cs_info_t *cs, wlc_cs_chan_info_t *ci, int *ncis,
                         int phy_type, chanspec_t *chanspec, int count)
{
	wlc_info_t *wlc = cs->wlc;
	int c;
	uint b;
	uint low, high, channel, ext;
	wlc_bss_info_t *bi;
	int band = cs->band;
	int phy = phy_type;
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN];
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

#ifdef BCMDBG
	/* dump all scan results */
	for (b = 0; b < wlc->scan_results->count; b ++) {
		bi = wlc->scan_results->ptrs[b];
		wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
		WL_INFORM(("wl%d: %s: chspec %s bw %s ht %s ctl %s\n",
		           wlc->pub->unit, ssidbuf,
		           wf_chspec_ntoa_ex(bi->chanspec, chanbuf),
		           CHSPEC_IS40(bi->chanspec) ?
		           "40MHz" :
		           "20MHz",
		           (bi->flags & WLC_BSS_HT) ?
		           "yes" :
		           "no",
		           (bi->flags & WLC_BSS_HT) ?
		           (CHSPEC_SB_LOWER(bi->chanspec) ?
		            "lower" :
		            CHSPEC_SB_UPPER(bi->chanspec) ?
		            "upper" :
		            "none") :
		           "n/a"));
	}
#endif /* BCMDBG */

	/* walk thru all scan channels to collect needed channel info */
	for (c = 0; c < count && c < *ncis; c ++) {
		bzero(&ci[c], sizeof(ci[0]));

		/* set channel range centered by the scan channel */
		ci[c].channel = wf_chspec_ctlchan(chanspec[c]);
		if (ci[c].channel - WLC_CS_ESOVLP_CHANS(band, phy) >= WLC_CS_MIN_CHAN(band))
			low = ci[c].channel - WLC_CS_ESOVLP_CHANS(band, phy);
		else {
			for (low = WLC_CS_MIN_CHAN(band); low <= ci[c].channel; low ++)
				if (VALID_CHANNEL20_DB(wlc, low))
					break;
		}
		if (ci[c].channel + WLC_CS_ESOVLP_CHANS(band, phy) <= WLC_CS_MAX_CHAN(band))
			high = ci[c].channel + WLC_CS_ESOVLP_CHANS(band, phy);
		else {
			for (high = WLC_CS_MAX_CHAN(band); high >= ci[c].channel; high --)
				if (VALID_CHANNEL20_DB(wlc, high))
					break;
		}
		WL_INFORM(("wl%d: collect info from channel %d to %d\n",
		           wlc->pub->unit, low, high));

		/* check BSSs that are on the scan channel or adjacent channels */
		for (b = 0; b < wlc->scan_results->count; b ++) {
			chanspec_t bss_chanspec;

			bi = wlc->scan_results->ptrs[b];
			bss_chanspec = wf_chspec_primary40_chspec(bi->chanspec);

			/* adjust ctl channel and ext channel */
			channel = CHSPEC_CHANNEL(bss_chanspec);
			ext = channel;
			if (bi->flags & WLC_BSS_HT) {
				if (CHSPEC_SB_LOWER(bss_chanspec)) {
					channel -= CH_10MHZ_APART;
					ext += CH_10MHZ_APART;
				}
				else if (CHSPEC_SB_UPPER(bss_chanspec)) {
					channel += CH_10MHZ_APART;
					ext -= CH_10MHZ_APART;
				}
			}

			/* skip bss not on the scan channel or adjacent channels */
			if (!((channel >= low && channel <= high) ||
			      (ext >= low && ext <= high)))
				continue;

#ifdef NOT_YET
			/* use RSSI to filter out Access Points in distance */
			...
#endif // endif

#ifdef BCMDBG
			wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
			WL_INFORM(("wl%d: %s: chspec %s bw %s ht %s ctl %s\n",
			           wlc->pub->unit, ssidbuf,
			           wf_chspec_ntoa_ex(bss_chanspec, chanbuf),
			           CHSPEC_IS40(bss_chanspec) ?
			           "40MHz" :
			           "20MHz",
			           (bi->flags & WLC_BSS_HT) ?
			           "yes" :
			           "no",
			           (bi->flags & WLC_BSS_HT) ?
			           (CHSPEC_SB_LOWER(bss_chanspec) ?
			            "lower" :
			            CHSPEC_SB_UPPER(bss_chanspec) ?
			            "upper" :
			            "none") :
			           "n/a"));
#endif /* BCMDBG */

			/* count 11n ctl sidebands */
			if ((bi->flags & WLC_BSS_HT) && channel >= low && channel <= high) {
				if (CHSPEC_SB_LOWER(bss_chanspec)) {
					if (ci[c].lSBs < MAXNBVAL(sizeof(ci[c].lSBs)))
						ci[c].lSBs ++;
				}
				else if (CHSPEC_SB_UPPER(bss_chanspec)) {
					if (ci[c].uSBs < MAXNBVAL(sizeof(ci[c].uSBs)))
						ci[c].uSBs ++;
				}
			}

			/* count 11n extensions */
			if ((bi->flags & WLC_BSS_HT) &&
			    ext >= low && ext <= high && ext != channel) {
				if (ci[c].nEXs < MAXNBVAL(sizeof(ci[c].nEXs)))
					ci[c].nEXs ++;
				continue;
			}

			/* count BSSs in 2.4G band */
			if (ci[c].channel <= WLC_CS_MAX_2G_CHAN) {
				if (wlc_cs_is_11gbss(wlc, bi)) {
					if (ci[c].gAPs < MAXNBVAL(sizeof(ci[c].gAPs)))
						ci[c].gAPs ++;
				}
				else {
					if (ci[c].bAPs < MAXNBVAL(sizeof(ci[c].bAPs)))
						ci[c].bAPs ++;
				}
			}
			/* count BSSs in 5G band */
			else {
				if (ci[c].aAPs < MAXNBVAL(sizeof(ci[c].aAPs)))
					ci[c].aAPs ++;
			}
		}
		WL_INFORM(("wl%d: channel %u: %u aAPs %u bAPs %u gAPs %u lSBs %u uSBs %u nEXs\n",
		           wlc->pub->unit, ci[c].channel,
		           ci[c].aAPs, ci[c].bAPs, ci[c].gAPs, ci[c].lSBs, ci[c].uSBs, ci[c].nEXs));
	}
	*ncis = c;
}

#ifdef APCS_TEST
/* Test vectors and results */
typedef struct {
	char *test;
	wlc_cs_chan_info_t ci[WLC_CS_MAX_SCAN_CHANS];
	int ncis;
	chanspec_t sel;
} apcs_test_vec_t;
static apcs_test_vec_t apcs_test_11n_2G[] =
{
	{ /* Test 1 */
		"all channels are empty, create 40MHz channel on 8",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(8, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 2 */
		"11n BSSs on 3 ctl on 1, create 40MHz channel on 3",
		{
		     /* ch, a, b, g, l, u, x */
			{1, 0, 0, 1, 1, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 1},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 3 */
		"11n BSSs on 9 ctl on 11, create 40MHz channel on 9",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 1},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 1, 0, 1, 0}
		},
		11,
		CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 4 */
		"11g BSSs on channel 1, create 40MHz channel on 8",
		{
			{1, 0, 0, 1, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(8, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 5 */
		"11g BSSs on channel 2, create 40MHz channel on 9",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 1, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 6 */
		"11g BSSs on channel 3, create 40MHz channel on 5",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 1, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(5, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 7 */
		"11g BSSs on channel 5, create 40MHz channel on 7",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 1, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 8 */
		"11g BSSs on channel 7, create 40MHz channel on 9",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 1, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 9 */
		"11g BSSs on channel 11, create 40MHz channel on 3",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 1, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 10 */
		"11g BSSs on channel 10, create 40MHz channel on 3",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 1, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 11 */
		"11g BSSs on channel 9, create 40MHz channel on 7",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 1, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 12 */
		"11g BSSs on channel 1 and 11, create 40MHz channel on 9",
		{
			{1, 0, 0, 1, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 1, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 13 */
		"11g BSSs on channel 6 and 11, create 40MHz channel on 4",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 1, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 1, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(4, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 14 */
		"11g BSSs on 5 and on 10, create 40MHz channel on 3",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 1, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 1, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 15 */
		"11g BSSs on channel 1 and 6, create 40MHz channel on 8",
		{
			{1, 0, 0, 1, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 1, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(8, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 16 */
		"11g BSSs on channel 2 and 7, create 40MHz channel on 9",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{2, 0, 0, 1, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 1, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 17 */
		"11n BSSs on 3 ctl on 1, 11g BSSs on 8, create 40MHz channel on 3",
		{
			{1, 0, 0, 1, 1, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 1},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 1, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 18 */
		"11n BSSs on 3 ctl on 5, 11g BSSs on 8, create 40MHz channel on 3",
		{
			{1, 0, 0, 0, 0, 0, 1},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 1, 0, 1, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 1, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 19 */
		"11n BSSs on 3 ctl on 1, 11g BSS on 10, create 40MHz channel on 3",
		{
			{1, 0, 0, 2, 2, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 2},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 1, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		11,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 20 */
		"11g BSSs on 1, 6, 11, create 20MHz channel on 6",
		{
			{1, 0, 0, 2, 0, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 1, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 0},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 2, 0, 0, 0}
		},
		11,
		CH20MHZ_CHSPEC(6)
	},
	{ /* test 21 */
		"11n BSSs on 3 AND 11n BSS on 11, create 40MHz channel on 11\n"
		"NOTE:  THIS TEST REQUIRES COUNTRY TO BE SET TO THAILAND",
		{
		     /* ch, a, b, g, l, u, x */
			{1, 0, 0, 2, 2, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 2},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 1},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0},
			{12, 0, 0, 0, 0, 0, 0},
			{13, 0, 0, 1, 0, 1, 0}
		},
		13,
		CH40MHZ_CHSPEC(11, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* test 22 */
		"11n BSS on 3 AND 11n BSSs on 11, create 40MHz channel on 3\n"
		"NOTE:  THIS TEST REQUIRES COUNTRY TO BE SET TO THAILAND",
		{
		     /* ch, a, b, g, l, u, x */
			{1, 0, 0, 1, 1, 0, 0},
			{2, 0, 0, 0, 0, 0, 0},
			{3, 0, 0, 0, 0, 0, 0},
			{4, 0, 0, 0, 0, 0, 0},
			{5, 0, 0, 0, 0, 0, 1},
			{6, 0, 0, 0, 0, 0, 0},
			{7, 0, 0, 0, 0, 0, 0},
			{8, 0, 0, 0, 0, 0, 0},
			{9, 0, 0, 0, 0, 0, 2},
			{10, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0},
			{12, 0, 0, 0, 0, 0, 0},
			{13, 0, 0, 2, 0, 2, 0}
		},
		13,
		CH40MHZ_CHSPEC(3, WL_CHANSPEC_CTL_SB_LOWER)
	}

};
static apcs_test_vec_t apcs_test_11g[] =
{
	{
		"all channels are empty, create 20MHz channel on 6",
		{
			{1, 0, 0, 0, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 0, 0, 0, 0}
		},
		3,
		CH20MHZ_CHSPEC(6)
	},
	{
		"11g BSS on channel 1 to 3 and 8 to 11, create 20MHz channel on 6",
		{
			{1, 0, 0, 1, 0, 0, 0},
			{6, 0, 0, 0, 0, 0, 0},
			{11, 0, 0, 1, 0, 0, 0}
		},
		3,
		CH20MHZ_CHSPEC(6)
	},
	{
		"11b/g BSS are all over, create 20MHz channel on 11",
		{
			{1, 0, 5, 0, 0, 0, 0},
			{6, 0, 2, 2, 0, 0, 0},
			{11, 0, 3, 0, 0, 0, 0}
		},
		3,
		CH20MHZ_CHSPEC(11)
	}
};
static apcs_test_vec_t apcs_test_11n_5G[] =
{
	{ /* Test 1 */
		"all channels are empty, create 40MHz channel on 54",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 0, 0, 0, 0, 0, 0},
			{44, 0, 0, 0, 0, 0, 0},
			{48, 0, 0, 0, 0, 0, 0},
			{52, 0, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 0, 0, 0, 0, 0, 0},
			{64, 0, 0, 0, 0, 0, 0},
			{149, 0, 0, 0, 0, 0, 0},
			{153, 0, 0, 0, 0, 0, 0},
			{157, 0, 0, 0, 0, 0, 0},
			{161, 0, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(54, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 2 */
		"11n 40MHz BSS on 38, create 40MHz channel on 38",
		{
			{36, 1, 0, 0, 1, 0, 0},
			{40, 0, 0, 0, 0, 0, 1},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 3 */
		"11n 40MHz BSS on 38, create 40MHz channel on 38",
		{
			{36, 0, 0, 0, 0, 0, 1},
			{40, 1, 0, 0, 0, 1, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 4 */
		"11n 40MHz BSS on 62, create 40MHz channel on 62",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 1, 0, 0},
			{64, 0, 0, 0, 0, 0, 1},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(62, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 5 */
		"11n 40MHz BSS on 62, create 40MHz channel on 62",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 0, 0, 0, 0, 0, 1},
			{64, 1, 0, 0, 0, 1, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(62, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 6 */
		"11n 40MHz BSS on 159, create 40MHz channel on 159",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 1, 0, 0},
			{161, 0, 0, 0, 0, 0, 1}
		},
		12,
		CH40MHZ_CHSPEC(159, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 7 */
		"11n 40MHz BSS on 159, create 40MHz channel on 159",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 0, 0, 0, 0, 0, 1},
			{161, 1, 0, 0, 0, 1, 0}
		},
		12,
		CH40MHZ_CHSPEC(159, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 8 */
		"11a BSS on channel 52, create 40MHz channel on 62",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 0, 0, 0, 0, 0, 0},
			{44, 0, 0, 0, 0, 0, 0},
			{48, 0, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 0, 0, 0, 0, 0, 0},
			{64, 0, 0, 0, 0, 0, 0},
			{149, 0, 0, 0, 0, 0, 0},
			{153, 0, 0, 0, 0, 0, 0},
			{157, 0, 0, 0, 0, 0, 0},
			{161, 0, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(62, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 9 */
		"11a BSS on all but 36, create 40MHz channel on 38",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 10 */
		"11a BSS on channel 40 44 56, create 40MHz channel on 62",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 0, 0, 0, 0, 0, 0},
			{52, 0, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 0, 0, 0, 0, 0, 0},
			{64, 0, 0, 0, 0, 0, 0},
			{149, 0, 0, 0, 0, 0, 0},
			{153, 0, 0, 0, 0, 0, 0},
			{157, 0, 0, 0, 0, 0, 0},
			{161, 0, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(62, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 11 */
		"11n 40MHz BSS on 38, 11a BSS on 52 and empty 56, create 40MHz channel on 38",
		{
			{36, 0, 0, 0, 0, 0, 1},
			{40, 1, 0, 0, 0, 1, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 2, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 12 */
		"11n 40MHz BSS on 38, 11a BSS on 52 and empty 56, create 40MHz channel on 54",
		{
			{36, 0, 0, 0, 0, 0, 2},
			{40, 2, 0, 0, 0, 2, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(54, WL_CHANSPEC_CTL_SB_LOWER)
	},
	{ /* Test 13 */
		"11a BSS on 40 and empty 36, create 40MHz channel on 38",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_UPPER)
	},
	{ /* Test 14 */
		"11a BSSs are all over, create 20MHz channel on 48",
		{
			{36, 4, 0, 0, 0, 0, 0},
			{40, 3, 0, 0, 0, 0, 0},
			{44, 3, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 5, 0, 0, 0, 0, 0},
			{56, 6, 0, 0, 0, 0, 0},
			{60, 5, 0, 0, 0, 0, 0},
			{64, 4, 0, 0, 0, 0, 0},
			{149, 3, 0, 0, 0, 0, 0},
			{153, 2, 0, 0, 0, 0, 0},
			{157, 4, 0, 0, 0, 0, 0},
			{161, 3, 0, 0, 0, 0, 0}
		},
		12,
		CH20MHZ_CHSPEC(48)
	}
};
static apcs_test_vec_t apcs_test_11a[] =
{
	{
		"all channel are empty, create 20MHz channel on 36",
		{
			{36, 0, 0, 0, 0, 0, 0},
			{40, 0, 0, 0, 0, 0, 0},
			{44, 0, 0, 0, 0, 0, 0},
			{48, 0, 0, 0, 0, 0, 0},
			{52, 0, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 0, 0, 0, 0, 0, 0},
			{64, 0, 0, 0, 0, 0, 0},
			{149, 0, 0, 0, 0, 0, 0},
			{153, 0, 0, 0, 0, 0, 0},
			{157, 0, 0, 0, 0, 0, 0},
			{161, 0, 0, 0, 0, 0, 0}
		},
		12,
		CH20MHZ_CHSPEC(36)
	},
	{
		"all channel but 56 are occupied, create 20MHz channel on 56",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 0, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 1, 0, 0, 0, 0, 0}
		},
		12,
		CH20MHZ_CHSPEC(56)
	},
	{
		"all channel but 161 are occupied, create 20MHz channel on 161",
		{
			{36, 1, 0, 0, 0, 0, 0},
			{40, 1, 0, 0, 0, 0, 0},
			{44, 1, 0, 0, 0, 0, 0},
			{48, 1, 0, 0, 0, 0, 0},
			{52, 1, 0, 0, 0, 0, 0},
			{56, 1, 0, 0, 0, 0, 0},
			{60, 1, 0, 0, 0, 0, 0},
			{64, 1, 0, 0, 0, 0, 0},
			{149, 1, 0, 0, 0, 0, 0},
			{153, 1, 0, 0, 0, 0, 0},
			{157, 1, 0, 0, 0, 0, 0},
			{161, 0, 0, 0, 0, 0, 0}
		},
		12,
		CH20MHZ_CHSPEC(161)
	}
};
#endif	/* APCS_TEST */

/* Scan completion callback */
static void
wlc_cs_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg)
{
	cs_info_t *cs = (cs_info_t *)arg;
	wlc_info_t *wlc = cs->wlc;
	bool bw40 = cs->bw40;

#ifdef WL11N_20MHZONLY
	ASSERT(!bw40);
	bw40 = FALSE;	/* Override to force compiler optimizations */
#endif // endif

	/* wlc->ap should not be null here. Just assert in case to avoid invalid
	 * use and free
	 */
	ASSERT(wlc->ap && cs->scan_chanspec_list);

	if ((status == WLC_E_STATUS_SUCCESS) || (status == WLC_E_STATUS_PARTIAL)) {
		wlc_cs_chan_info_t ci[WLC_CS_MAX_SCAN_CHANS];
		int ncis = WLC_CS_MAX_SCAN_CHANS;

		wlc_cs_parse_scanresults(cs, ci, &ncis, wlc->band->phytype,
		                         cs->scan_chanspec_list, cs->scan_chanspec_count);

#ifdef WL11N
		/* If user provided a "preferred 40MHz chanspec" honor that request
		 * if it passes 20/40 Coex rules. We'll either be able to use it as is,
		 * or we'll form a 20MHz AP on the defined control channel.
		 */
		if (BAND_2G(cs->band) && COEX_ACTIVE(wlc->obss, cfg) &&
		    CHSPEC_IS40(wlc->ap->pref_chanspec)) {
			wlc->ap->chanspec_selected =
			        wlc_cs_sel_coexchanspec(cs, ci, ncis, wlc->ap->pref_chanspec);
		} else
#endif /* WL11N */
			wlc->ap->chanspec_selected = wlc_cs_sel_chanspec(cs, bw40, ci, ncis);

		wlc_lq_chanim_upd_acs_record(wlc->chanim_info, wlc->home_chanspec,
			wlc->ap->chanspec_selected, cs->reason);
	}

	if (cs->cb != NULL)
		(cs->cb)(cs->arg, status);

#ifdef APCS_TEST
{
	uint msg_level = wl_msg_level;
	apcs_test_vec_t *apcs_test_vec;
	uint apcs_test_len;
	chanspec_t chanspec;
	uint i, f = 0;
	int bandtype = cs->band;

	wl_msg_level |= WL_INFORM_VAL;

	WL_INFORM(("\nwl%d: running builtin tests...\n", wlc->pub->unit));

	if (BAND_2G(bandtype)) {
		if (wlc->band->phytype == PHY_TYPE_N) {
			apcs_test_vec = apcs_test_11n_2G;
			apcs_test_len = ARRAYSIZE(apcs_test_11n_2G);
		}
		else {
			apcs_test_vec = apcs_test_11g;
			apcs_test_len = ARRAYSIZE(apcs_test_11g);
		}
	}
	else if (BAND_5G(bandtype)) {
		if (wlc->band->phytype == PHY_TYPE_N) {
			apcs_test_vec = apcs_test_11n_5G;
			apcs_test_len = ARRAYSIZE(apcs_test_11n_5G);
		}
		else {
			apcs_test_vec = apcs_test_11a;
			apcs_test_len = ARRAYSIZE(apcs_test_11a);
		}
	}
	else {
		ASSERT(!"unknown phy type");
		return;
	}

	for (i = 0; i < apcs_test_len; i ++) {
		/* XXX Test %d print is now using i+1 so that these test numbers match the tests
		 * described in the APAutoChannel Twiki page test pictures.
		 */
		WL_INFORM(("wl%d: test %d: '%s' \n", wlc->pub->unit, i+1, apcs_test_vec[i].test));
		chanspec =
		        wlc_cs_sel_chanspec(cs, bw40, apcs_test_vec[i].ci, apcs_test_vec[i].ncis);
		WL_INFORM(("wl%d: test %d: %s result %x %s expected %x\n",
		           wlc->pub->unit, i+1, chanspec == apcs_test_vec[i].sel ? "PASS" : "FAIL",
		           chanspec, chanspec == apcs_test_vec[i].sel ? "==" : "!=",
		           apcs_test_vec[i].sel));
		f += chanspec != apcs_test_vec[i].sel;
	}

	WL_INFORM(("wl%d: builtin tests done. %u failed\n", wlc->pub->unit, f));

	wl_msg_level = msg_level;
}
#endif	/* APCS_TEST */

	WL_INFORM(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));
}

/* Build default chanspec list for scanning */
static void
wlc_cs_build_chanspec(wlc_info_t *wlc, int bandtype, int phy_type, chanspec_t *chanspec, int *size)
{
	int last, i;
	uint count = 0;
	chanspec_t chspec;

	ASSERT(*size >= WLC_CS_MAX_SCAN_CHANS);

	/* select from valid channels */
	last = WLC_CS_MIN_CHAN(bandtype) - 1;
	for (i = WLC_CS_MIN_CHAN(bandtype);
	     i <= WLC_CS_MAX_CHAN(bandtype) && count < WLC_CS_MAX_SCAN_CHANS;
	     i ++) {
		if (last != WLC_CS_MIN_CHAN(bandtype) - 1 && i - last <=
		    WLC_CS_SCAN_APART(bandtype, phy_type))
			continue;
		chspec = CH20MHZ_CHSPEC(i);
		if (wlc_valid_chanspec_db(wlc->cmi, chspec)) {
			chanspec[count++] = chspec;
			last = i;
		}
	}

	*size = (int)count;
}

/* Build chanspec list based on min/max channel value */
static void
wlc_cs_build_chanspec_ext(wlc_info_t *wlc, int bandtype, int phy_type,
	int min_chan, int max_chan, chanspec_t *chanspec, int *size)
{
	int last, i;
	uint count = 0;
	chanspec_t chspec;

	ASSERT(min_chan >= WLC_CS_MIN_CHAN(bandtype));
	ASSERT(max_chan <= WLC_CS_MAX_CHAN(bandtype));

	/* select from valid channels */
	last = min_chan - 1;
	for (i = min_chan;
		i <= max_chan && count < WLC_CS_MAX_SCAN_CHANS;
		i++) {
		if (last != min_chan - 1 && i - last <=
			WLC_CS_SCAN_APART(bandtype, phy_type))
			continue;
		chspec = CH20MHZ_CHSPEC(i);
		if (wlc_valid_chanspec_db(wlc->cmi, chspec)) {
			chanspec[count++] = chspec;
			last = i;
		}
	}

	*size = (int)count;
}

static void
wlc_cs_watchdog(void *arg)
{
	cs_info_t *cs = (cs_info_t *) arg;
	wlc_info_t *wlc = cs->wlc;

	/* periodic auto channel selection timer */
	if (!WL11H_ENAB(wlc) &&
		/* this channel selection disabled for 11h-enabled AP */
		AP_ENAB(wlc->pub) &&
		cs->cs_scan_timer &&
		!(wlc->block_datafifo & DATA_BLOCK_QUIET) &&
		(((wlc->pub->now % cs->cs_scan_timer) == 0) ||
		cs->cs_scan_ini || WLC_CHANIM_ACT(wlc->chanim_info))) {
		wlc_cs_scan_timer(wlc->cfg);
	}

}

static int
wlc_cs_ioctl(void *hdl, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	cs_info_t *cs = (cs_info_t *)hdl;
	wlc_info_t *wlc = cs->wlc;
	int val = 0;
	int *pval;
	int bcmerror = 0;

	/* default argument is generic integer */
	pval = (int *) arg;
	/* prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));

	switch (cmd) {
	case WLC_START_CHANNEL_SEL:
		/* irrelevant to 802.11h-enabled AP */
		if (WL11H_AP_ENAB(wlc)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		if (AP_ACTIVE(wlc) || SCAN_IN_PROGRESS(wlc->scan)) {
			bcmerror = BCME_BUSY;
			break;
		}

		if (arg) {
			bcmerror = wlc_cs_scan_start(wlc->cfg, arg, TRUE, FALSE, TRUE,
				wlc->band->bandtype, APCS_IOCTL, NULL, NULL);
		} else {
			bcmerror = BCME_BADARG;
		}

		break;

	case WLC_SET_CS_SCAN_TIMER:
		if (AP_ENAB(wlc->pub)) {
			/* for 802.11h-enabled AP, cs_scan is disallowed */
			cs->cs_scan_timer = WL11H_ENAB(wlc) ? 0 : val * 60;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_GET_CS_SCAN_TIMER:
		if (pval) {
			*pval = cs->cs_scan_timer / 60;
		} else {
			bcmerror = BCME_BADARG;
		}
		break;

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	return (bcmerror);
}

/* Validate ioctl parameters and initiate scan process */
/* It can be an explicit request where 'request' lists all chanspecs that participate
 * in this Channel Select process, or an implicit request where 'request' is empty.
 * It can be used to select a 40MHz channel if possible when 'bw40' is TRUE.
 * It can send Probe Requests in scan process if 'active' is TRUE.
 * It can be periodic channel selection if 'periodic' is TRUE.
 */
int
wlc_cs_scan_start(wlc_bsscfg_t *cfg, wl_uint32_list_t *request, bool bw40, bool active,
	bool periodic, int bandtype, uint8 reason, void (*cb)(void *arg, int status), void *arg)
{
	wlc_info_t *wlc = cfg->wlc;
	chanspec_t chspec;
	uint i;
	wlc_ssid_t req_ssid;
	cs_info_t *cs = wlc->cs;
	int status;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

#ifdef WL11N_20MHZONLY
	ASSERT(!bw40);
	bw40 = FALSE;	/* Override to force compiler optimizations */
#endif // endif

	WL_INFORM(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

	if (!active) {
		ASSERT(AP_ENAB(wlc->pub));
		ASSERT(!WL11H_AP_ENAB(wlc));
	}

	/* Allocate chanspec vector and save for benefit of completion routine */
	cs->scan_chanspec_count = 0;

	/* compile default scan chanspec list if not specified */
	if (!request->count) {
		cs->scan_chanspec_count = WLC_CS_MAX_SCAN_CHANS;
		wlc_cs_build_chanspec(wlc, bandtype, wlc->band->phytype,
		                      cs->scan_chanspec_list, &cs->scan_chanspec_count);
	}
	/* NPHY needs to scan all channels in order to make the right decision to select
	 * channel and channel bandwidth and HT control sideband. Locale info is required
	 * to figure out if the specified channel list includes all channels. Let's not
	 * allow that before we figure out how to use the locale info.
	 */
	else if (wlc->band->phytype == PHY_TYPE_N && bw40) {
		WL_ERROR(("wl%d: NPHY does not support explicit channel list\n", wlc->pub->unit));
		status = BCME_BADARG;
		return status;
	}
	/* check and validate specified chanspec list */
	else if (request->count <= WLC_CS_MAX_SCAN_CHANS) {
		for (i = 0; i < request->count; i++) {
			chspec = (chanspec_t)request->element[i];
			if (!wlc_valid_chanspec_db(wlc->cmi, chspec)) {
				WL_ERROR(("wl%d: invalid scan chanspec %x at element %d\n",
				          wlc->pub->unit, chspec, i));
				status = BCME_BADARG;
				return status;
			}
			if (i > 0 &&
			    CHSPEC_CHANNEL(chspec) <=
			    CHSPEC_CHANNEL((chanspec_t)request->element[i - 1])) {
				WL_ERROR(("wl%d: scan chanspec %s at element %d is out of order\n",
				          wlc->pub->unit, wf_chspec_ntoa_ex(chspec, chanbuf), i));
				status = BCME_BADARG;
				return status;
			}
			cs->scan_chanspec_list[cs->scan_chanspec_count++] = chspec;
		}
	}
	/* too many scan channels specified in chanspec list */
	else {
		WL_ERROR(("wl%d: too many scan channels specified (%d), max %d channels\n",
		          wlc->pub->unit, request->count, WLC_CS_MAX_SCAN_CHANS));
		status = BCME_BADARG;
		return status;
	}

	/* can't move forward if wlc->ap->scan_chanspec_count is empty */
	if (cs->scan_chanspec_count == 0) {
		WL_ERROR(("wl%d: %s: no scan channels\n", wlc->pub->unit, __FUNCTION__));
		status = BCME_BADARG;
		return status;
	}

	/* invoke the callback only when scan request is successfully submitted */
	cs->cb = NULL;
	cs->arg = arg;
	cs->cfg = cfg;
	cs->bw40 = bw40;
	cs->band = (uint8)bandtype;
	cs->reason = reason;

	/* start scan process */
	bzero(&req_ssid, sizeof(req_ssid));
	status = wlc_scan_request_ex(wlc, DOT11_BSSTYPE_ANY, &ether_bcast,
	                        1, &req_ssid,
	                        active ? DOT11_SCANTYPE_ACTIVE : DOT11_SCANTYPE_PASSIVE,
	                        -1, -1, -1, -1, cs->scan_chanspec_list,
	                        cs->scan_chanspec_count, 0,
	                        FALSE, wlc_cs_scan_complete, cs,
	                        WLC_ACTION_SCAN, FALSE, cfg, NULL, NULL);
	if (status == BCME_OK) {
		cs->cb = cb;
		cs->cs_scan_ini = periodic;
	}
	/* cs and scan_chanspec_list are freed in wlc_cs_scan_complete()
	 * which is invoked by wlc_scan_request() regardless the return status
	 */
	return status;

}

void
wlc_cs_scan_timer(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	cs_info_t *cs = wlc->cs;

	ASSERT(!WL11H_AP_ENAB(wlc));

	/* If the channel select scan has been initiated, change the channel to
	 * selected channel.
	 */
	if (cs->cs_scan_ini) {
		/* wait for the scan to complete */
		if (SCAN_IN_PROGRESS(wlc->scan))
			return;

		cs->cs_scan_ini = FALSE;
		if (wlc->pub->up && (wlc->ap->chanspec_selected != 0) &&
		    (WLC_BAND_PI_RADIO_CHANSPEC != wlc->ap->chanspec_selected)) {
			WL_INFORM(("wl%d: %s: changing chanspec to %d\n",
				wlc->pub->unit, __FUNCTION__, wlc->ap->chanspec_selected));
			wlc_set_home_chanspec(wlc, wlc->ap->chanspec_selected);
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, wlc->ap->chanspec_selected);
			if (AP_ENAB(wlc->pub)) {
				wlc_bss_info_t *current_bss = cfg->current_bss;
#ifdef WLMCHAN
				chanspec_t chanspec = current_bss->chanspec;
#else
				chanspec_t chanspec = wlc->chanspec;
#endif /* WLMCHAN */
				wlc->bcn_rspec = wlc_lowest_basic_rspec(wlc, &current_bss->rateset);
				ASSERT(wlc_valid_rate(wlc, wlc->bcn_rspec,
				       CHSPEC_IS2G(current_bss->chanspec) ?
				       WLC_BAND_2G : WLC_BAND_5G, TRUE));
				wlc_beacon_phytxctl(wlc, wlc->bcn_rspec, chanspec);
				wlc_beacon_upddur(wlc, wlc->bcn_rspec, 0);
			}
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, FALSE);
			wlc_enable_mac(wlc);
		}
	} else if (wlc->pub->up && !SCAN_IN_PROGRESS(wlc->scan)) {
		/* If there are any stations associated, skip the channel select scan */
		struct scb *scb;
		struct scb_iter scbiter;

		if (WLC_CHANIM_ACT(wlc->chanim_info)) {
			wlc_lq_chanim_action(wlc);
			return;
		}

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb))
				break;
		}

		if (scb == NULL) {
			wl_uint32_list_t request;

			request.count = 0;
			(void)wlc_cs_scan_start(cfg, &request, TRUE, FALSE, TRUE,
				wlc->band->bandtype, APCS_CSTIMER, NULL, NULL);
		}
	}
}

cs_info_t*
BCMATTACHFN(wlc_apcs_attach)(wlc_info_t *wlc)
{
	cs_info_t *cs;

	ASSERT(wlc != NULL);

	wlc->pub->_apcs = TRUE;

	/* allocate info structure */
	if ((cs = (cs_info_t *)MALLOCZ(wlc->osh, sizeof(cs_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	cs->wlc = wlc;

	cs->scan_chanspec_list_size = (int)sizeof(chanspec_t) * WLC_CS_MAX_SCAN_CHANS;
	cs->scan_chanspec_list = (chanspec_t *)MALLOC(wlc->osh, cs->scan_chanspec_list_size);
	if (!cs->scan_chanspec_list) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	cs->cs_scan_ini = FALSE;

	/* register module */
	if (wlc_module_register(wlc->pub, NULL, "apcs",
		cs, NULL, wlc_cs_watchdog, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* add ioctl */
	if (wlc_module_add_ioctl_fn(wlc->pub, (void *)cs, wlc_cs_ioctl, 0, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return cs;

fail:
	wlc_apcs_detach(cs);

	return NULL;
}

void
BCMATTACHFN(wlc_apcs_detach)(cs_info_t *cs)
{
	wlc_info_t *wlc;

	if (cs != NULL) {
		wlc = cs->wlc;

		ASSERT(wlc != NULL && wlc->pub != NULL);

		/* remove ioctl */
		(void)wlc_module_remove_ioctl_fn(wlc->pub, (void *)cs);

		/* unregister */
		(void)wlc_module_unregister(wlc->pub, "apcs", cs);

		/* free info structure */
		if (cs->scan_chanspec_list) {
			MFREE(wlc->osh, cs->scan_chanspec_list, cs->scan_chanspec_list_size);
			cs->scan_chanspec_list = NULL;
		}

		MFREE(wlc->osh, cs, sizeof(cs_info_t));
	}
}

#endif /* APCS */
