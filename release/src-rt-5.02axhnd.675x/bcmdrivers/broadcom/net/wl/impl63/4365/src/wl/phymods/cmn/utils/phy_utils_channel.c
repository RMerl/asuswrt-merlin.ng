/*
 * PHY utils - chanspec functions.
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
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <bcmutils.h>

#include <wlc_phy_int.h>

#include <phy_utils_channel.h>
#include <phy_utils_api.h>

/* channel info structure */
typedef struct phy_chan_info_basic {
	uint16	chan;		/* channel number */
	uint16	freq;		/* in Mhz */
} phy_chan_info_basic_t;

static const phy_chan_info_basic_t chan_info_all[] = {
	/* 11b/11g */
/* 0 */		{1,	2412},
/* 1 */		{2,	2417},
/* 2 */		{3,	2422},
/* 3 */		{4,	2427},
/* 4 */		{5,	2432},
/* 5 */		{6,	2437},
/* 6 */		{7,	2442},
/* 7 */		{8,	2447},
/* 8 */		{9,	2452},
/* 9 */		{10,	2457},
/* 10 */	{11,	2462},
/* 11 */	{12,	2467},
/* 12 */	{13,	2472},
/* 13 */	{14,	2484},

#ifdef BAND5G
/* 11a japan high */
/* 14 */	{34,	5170},
/* 15 */	{38,	5190},
/* 16 */	{42,	5210},
/* 17 */	{46,	5230},

/* 11a usa low */
/* 18 */	{36,	5180},
/* 19 */	{40,	5200},
/* 20 */	{44,	5220},
/* 21 */	{48,	5240},
/* 22 */	{52,	5260},
/* 23 */	{54,	5270},
/* 24 */	{56,	5280},
/* 25 */	{60,	5300},
/* 26 */	{62,	5310},
/* 27 */	{64,	5320},

/* 11a Europe */
/* 28 */	{100,	5500},
/* 29 */	{102,	5510},
/* 30 */	{104,	5520},
/* 31 */	{108,	5540},
/* 31 */	{110,	5550},
/* 32 */	{112,	5560},
/* 33 */	{116,	5580},
/* 34 */	{118,	5590},
/* 35 */	{120,	5600},
/* 36 */	{124,	5620},
/* 37 */	{126,	5630},
/* 38 */	{128,	5640},
/* 39 */	{132,	5660},
/* 39 */	{134,	5660},
/* 40 */	{136,	5680},
/* 41 */	{140,	5700},

#ifdef WL11AC
/* 42 */	{144,   5720},

/* 11a usa high, ref5 only */
/* 43 */	{149,   5745},
/* 44 */	{151,   5755},
/* 45 */	{153,   5765},
/* 46 */	{157,   5785},
/* 47 */	{159,   5795},
/* 48 */	{161,   5805},
/* 49 */	{165,   5825},

/* 11a japan */
/* 50 */	{184,   4920},
/* 51 */	{185,   4925},
/* 52 */	{187,   4935},
/* 53 */	{188,   4940},
/* 54 */	{189,   4945},
/* 55 */	{192,   4960},
/* 56 */	{196,   4980},
/* 57 */	{200,   5000},
/* 58 */	{204,   5020},
/* 59 */	{207,   5035},
/* 60 */	{208,   5040},
/* 61 */	{209,   5045},
/* 62 */	{210,   5050},
/* 63 */	{212,   5060},
/* 64 */	{216,   5080}

#else

/* 11a usa high, ref5 only */
/* 42 */	{149,	5745},
/* 43 */	{151,	5755},
/* 44 */	{153,	5765},
/* 45 */	{157,	5785},
/* 46 */	{159,	5795},
/* 47 */	{161,	5805},
/* 48 */	{165,	5825},

/* 11a japan */
/* 49 */	{184,	4920},
/* 50 */	{185,	4925},
/* 51 */	{187,	4935},
/* 52 */	{188,	4940},
/* 53 */	{189,	4945},
/* 54 */	{192,	4960},
/* 55 */	{196,	4980},
/* 56 */	{200,	5000},
/* 57 */	{204,	5020},
/* 58 */	{207,	5035},
/* 59 */	{208,	5040},
/* 60 */	{209,	5045},
/* 61 */	{210,	5050},
/* 62 */	{212,	5060},
/* 63 */	{216,	5080}
#endif /* WL11AC */

#endif /* BAND5G */
};

chanspec_t
phy_utils_get_chanspec(phy_info_t *pi)
{
	return pi->radio_chanspec;
}

/*
 * Converts channel number to channel frequency.
 * Returns 0 if the channel is out of range.
 * Also used by some code in wlc_iw.c
 */
int
phy_utils_channel2freq(uint channel)
{
	uint i;

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		if (chan_info_all[i].chan == channel)
			return (chan_info_all[i].freq);
	}

	return (0);
}

/* Converts channel number into the wlc_phy_chan_info index */
uint
phy_utils_channel2idx(uint channel)
{
	uint i;

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		if (chan_info_all[i].chan == channel)
			return i;
	}

	ASSERT(FALSE);
	return (0);
}

/* fill out a chanvec_t with all the supported channels for the band. */
void
phy_utils_chanspec_band_validch(phy_info_t *pi, uint band, chanvec_t *channels)
{
	uint i;
	uint channel;

	ASSERT((band == WLC_BAND_2G) || (band == WLC_BAND_5G));

	bzero(channels, sizeof(chanvec_t));

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		channel = chan_info_all[i].chan;

		/* disable the high band channels [149-165] for srom ver 1 */
		if ((pi->a_band_high_disable) && (channel >= FIRST_REF5_CHANNUM) &&
		    (channel <= LAST_REF5_CHANNUM))
			continue;

		/* Disable channel 144 unless it's an ACPHY */
		if ((channel == 144) && (!ISACPHY(pi)))
			continue;

		if (CHIPID_4324X_MEDIA_FAMILY(pi) &&
			((channel == 34) || (channel == 38) || (channel == 42) || (channel == 46)))
			continue;

		if (((band == WLC_BAND_2G) && (channel <= CH_MAX_2G_CHANNEL)) ||
		    ((band == WLC_BAND_5G) && (channel > CH_MAX_2G_CHANNEL)))
			setbit(channels->vec, channel);
	}
}

/* returns the first hw supported channel in the band */
chanspec_t
phy_utils_chanspec_band_firstch(phy_info_t *pi, uint band)
{
	uint i;
	uint channel;
	chanspec_t chspec;

	ASSERT((band == WLC_BAND_2G) || (band == WLC_BAND_5G));

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		channel = chan_info_all[i].chan;

		/* If 40MHX b/w then check if there is an upper 20Mhz adjacent channel */
		if (IS40MHZ(pi)) {
			uint j;
			/* check if the upper 20Mhz channel exists */
			for (j = 0; j < ARRAYSIZE(chan_info_all); j++) {
				if (chan_info_all[j].chan == channel + CH_10MHZ_APART)
					break;
			}
			/* did we find an adjacent channel */
			if (j == ARRAYSIZE(chan_info_all))
				continue;
			/* Convert channel from 20Mhz num to 40 Mhz number */
			channel = UPPER_20_SB(channel);
			chspec = channel | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_LOWER;
			if (band == WLC_BAND_2G)
				chspec |= WL_CHANSPEC_BAND_2G;
			else
				chspec |= WL_CHANSPEC_BAND_5G;
		}
		else
			chspec = CH20MHZ_CHSPEC(channel);

		/* disable the high band channels [149-165] for srom ver 1 */
		if ((pi->a_band_high_disable) && (channel >= FIRST_REF5_CHANNUM) &&
		    (channel <= LAST_REF5_CHANNUM))
			continue;

		if (((band == WLC_BAND_2G) && (channel <= CH_MAX_2G_CHANNEL)) ||
		    ((band == WLC_BAND_5G) && (channel > CH_MAX_2G_CHANNEL)))
			return chspec;
	}

	/* should never come here */
	ASSERT(0);

	/* to avoid warning */
	return (chanspec_t)INVCHANSPEC;
}
