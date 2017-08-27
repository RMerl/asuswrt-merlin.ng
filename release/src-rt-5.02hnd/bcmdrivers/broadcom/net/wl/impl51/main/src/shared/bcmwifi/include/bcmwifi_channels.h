/*
 * Misc utility routines for WL and Apps
 * This header file housing the define and function prototype use by
 * both the wl driver, tools & Apps.
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: bcmwifi_channels.h 309193 2012-01-19 00:03:57Z $
 */

#ifndef	_bcmwifi_channels_h_
#define	_bcmwifi_channels_h_


/* A chanspec holds the channel number, band, bandwidth and control sideband */
typedef uint16 chanspec_t;

/* channel defines */
#define CH_UPPER_SB			0x01
#define CH_LOWER_SB			0x02
#define CH_EWA_VALID			0x04
#define CH_80MHZ_APART			16
#define CH_40MHZ_APART			8
#define CH_20MHZ_APART			4
#define CH_10MHZ_APART			2
#define CH_5MHZ_APART			1	/* 2G band channels are 5 Mhz apart */
#define CH_MAX_2G_CHANNEL		14	/* Max channel in 2G band */
#define MAXCHANNEL		224	/* max # supported channels. The max channel no is above,
					 * this is that + 1 rounded up to a multiple of NBBY (8).
					 * DO NOT MAKE it > 255: channels are uint8's all over
					 */
#define MAXCHANNEL_NUM	(MAXCHANNEL - 1)	/* max channel number */

/* make sure channel num is within valid range */
#define CH_NUM_VALID_RANGE(ch_num) ((ch_num) > 0 && (ch_num) <= MAXCHANNEL_NUM)

#define CHSPEC_CTLOVLP(sp1, sp2, sep)	(ABS(wf_chspec_ctlchan(sp1) - wf_chspec_ctlchan(sp2)) < \
				  (sep))

/* All builds use the new 11ac ratespec/chanspec */
#undef  D11AC_IOTYPES
#define D11AC_IOTYPES

#define WL_CHANSPEC_CHAN_MASK		0x00ff
#define WL_CHANSPEC_CHAN_SHIFT		0
#define WL_CHANSPEC_CHAN1_MASK		0x000f
#define WL_CHANSPEC_CHAN1_SHIFT		0
#define WL_CHANSPEC_CHAN2_MASK		0x00f0
#define WL_CHANSPEC_CHAN2_SHIFT		4

#define WL_CHANSPEC_CTL_SB_MASK		0x0700
#define WL_CHANSPEC_CTL_SB_SHIFT	8
#define WL_CHANSPEC_CTL_SB_LLL		0x0000
#define WL_CHANSPEC_CTL_SB_LLU		0x0100
#define WL_CHANSPEC_CTL_SB_LUL		0x0200
#define WL_CHANSPEC_CTL_SB_LUU		0x0300
#define WL_CHANSPEC_CTL_SB_ULL		0x0400
#define WL_CHANSPEC_CTL_SB_ULU		0x0500
#define WL_CHANSPEC_CTL_SB_UUL		0x0600
#define WL_CHANSPEC_CTL_SB_UUU		0x0700
#define WL_CHANSPEC_CTL_SB_LL		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_LU		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_UL		WL_CHANSPEC_CTL_SB_LUL
#define WL_CHANSPEC_CTL_SB_UU		WL_CHANSPEC_CTL_SB_LUU
#define WL_CHANSPEC_CTL_SB_L		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_U		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_LOWER	WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_UPPER	WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_NONE		WL_CHANSPEC_CTL_SB_LLL

#define WL_CHANSPEC_BW_MASK		0x3800
#define WL_CHANSPEC_BW_SHIFT		11
#define WL_CHANSPEC_BW_5		0x0000
#define WL_CHANSPEC_BW_10		0x0800
#define WL_CHANSPEC_BW_20		0x1000
#define WL_CHANSPEC_BW_40		0x1800
#define WL_CHANSPEC_BW_80		0x2000
#define WL_CHANSPEC_BW_160		0x2800
#define WL_CHANSPEC_BW_8080		0x3000

#define WL_CHANSPEC_BAND_MASK		0xc000
#define WL_CHANSPEC_BAND_SHIFT		14
#define WL_CHANSPEC_BAND_2G		0x0000
#define WL_CHANSPEC_BAND_3G		0x4000
#define WL_CHANSPEC_BAND_4G		0x8000
#define WL_CHANSPEC_BAND_5G		0xc000
#define INVCHANSPEC			255

/* channel defines */
#define LOWER_20_SB(channel)		(((channel) > CH_10MHZ_APART) ? \
					((channel) - CH_10MHZ_APART) : 0)
#define UPPER_20_SB(channel)		(((channel) < (MAXCHANNEL - CH_10MHZ_APART)) ? \
					((channel) + CH_10MHZ_APART) : 0)

/* pass a 80MHz channel number (uint8) to get respective LL, UU, LU, UL */
#define LL_20_SB(channel) (((channel) > 3 * CH_10MHZ_APART) ? ((channel) - 3 * CH_10MHZ_APART) : 0)
#define UU_20_SB(channel) (((channel) < (MAXCHANNEL - 3 * CH_10MHZ_APART)) ? \
				((channel) + 3 * CH_10MHZ_APART) : 0)
#define LU_20_SB(channel) LOWER_20_SB(channel)
#define UL_20_SB(channel) UPPER_20_SB(channel)

#define LOWER_40_SB(channel)		((channel) - CH_20MHZ_APART)
#define UPPER_40_SB(channel)		((channel) + CH_20MHZ_APART)
#define CHSPEC_WLCBANDUNIT(chspec)	(CHSPEC_IS5G(chspec) ? BAND_5G_INDEX : BAND_2G_INDEX)
#define CH20MHZ_CHSPEC(channel)		(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
					(((channel) <= CH_MAX_2G_CHANNEL) ? \
					WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G))
#define NEXT_20MHZ_CHAN(channel)	(((channel) < (MAXCHANNEL - CH_20MHZ_APART)) ? \
					((channel) + CH_20MHZ_APART) : 0)
#define CH40MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | WL_CHANSPEC_BW_40 | \
					((channel) <= CH_MAX_2G_CHANNEL ? WL_CHANSPEC_BAND_2G : \
					WL_CHANSPEC_BAND_5G))
#define CH80MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | \
					 WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G)
#define CH160MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | \
					 WL_CHANSPEC_BW_160 | WL_CHANSPEC_BAND_5G)

#define CHBW_CHSPEC(bw, channel)	(chanspec_t)((chanspec_t)(channel) | (bw) | \
							(((channel) <= CH_MAX_2G_CHANNEL) ? \
							WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G))

/* simple MACROs to get different fields of chanspec */
#ifdef WL11AC_80P80
#define CHSPEC_CHANNEL(chspec)	wf_chspec_channel(chspec)
#else
#define CHSPEC_CHANNEL(chspec)	((uint8)((chspec) & WL_CHANSPEC_CHAN_MASK))
#endif
#define CHSPEC_CHAN1(chspec)	((chspec) & WL_CHANSPEC_CHAN1_MASK) >> WL_CHANSPEC_CHAN1_SHIFT
#define CHSPEC_CHAN2(chspec)	((chspec) & WL_CHANSPEC_CHAN2_MASK) >> WL_CHANSPEC_CHAN2_SHIFT
#define CHSPEC_BAND(chspec)		((chspec) & WL_CHANSPEC_BAND_MASK)
#define CHSPEC_CTL_SB(chspec)	((chspec) & WL_CHANSPEC_CTL_SB_MASK)
#define CHSPEC_BW(chspec)		((chspec) & WL_CHANSPEC_BW_MASK)

#ifdef WL11N_20MHZONLY

#define CHSPEC_IS10(chspec)	0
#define CHSPEC_IS20(chspec)	1
#ifndef CHSPEC_IS40
#define CHSPEC_IS40(chspec)	0
#endif
#ifndef CHSPEC_IS80
#define CHSPEC_IS80(chspec)	0
#endif
#ifndef CHSPEC_IS160
#define CHSPEC_IS160(chspec)	0
#endif
#ifndef CHSPEC_IS8080
#define CHSPEC_IS8080(chspec)	0
#endif
#ifndef CHSPEC_LT80
#define CHSPEC_LT80(chspec)	1
#endif

#else /* !WL11N_20MHZONLY */

#define CHSPEC_IS10(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_10)
#define CHSPEC_IS20(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)
#ifndef CHSPEC_IS40
#define CHSPEC_IS40(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40)
#endif
#ifndef CHSPEC_IS80
#define CHSPEC_IS80(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_80)
#endif
#ifndef CHSPEC_IS160
#define CHSPEC_IS160(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_160)
#endif
#ifndef CHSPEC_IS8080
#define CHSPEC_IS8080(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_8080)
#endif
#ifndef CHSPEC_LT80
#define CHSPEC_LT80(chspec) \
			(CHSPEC_IS10(chspec) | CHSPEC_IS20(chspec) | CHSPEC_IS40(chspec))
#endif

/* pass a center channel and get channel offset from it by 10MHz */
#define CH_OFF_10MHZ_MULTIPLES(channel, offset)  ((uint8) (((offset) < 0) ? \
		(((channel) > (WL_CHANSPEC_CHAN_MASK & ((uint16)((-(offset)) * CH_10MHZ_APART)))) ?\
			((channel) + (offset) * CH_10MHZ_APART) : 0) : \
			(((channel) < (uint16)(MAXCHANNEL - (offset) * CH_10MHZ_APART)) ? \
				((channel) + (offset) * CH_10MHZ_APART) : 0)))

#if defined(WL11AC_80P80) || defined(WL11AC_160)
/* pass a 160MHz center channel to get 20MHz subband channel numbers */
#define LLL_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel, -7)
#define LLU_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel, -5)
#define LUL_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel, -3)
#define LUU_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel, -1)
#define ULL_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel,  1)
#define ULU_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel,  3)
#define UUL_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel,  5)
#define UUU_20_SB_160(channel)  CH_OFF_10MHZ_MULTIPLES(channel,  7)

/* given an 80p80 channel, return the lower 80MHz sideband */
#define LOWER_80_SB(chspec)  (wf_chspec_primary80_channel(chspec) < \
		wf_chspec_secondary80_channel(chspec) ? \
		wf_chspec_primary80_channel(chspec) : wf_chspec_secondary80_channel(chspec))

/* given an 80p80 channel, return the upper 80MHz sideband */
#define UPPER_80_SB(chspec)  (wf_chspec_primary80_channel(chspec) > \
		wf_chspec_secondary80_channel(chspec) ? \
		wf_chspec_primary80_channel(chspec) : wf_chspec_secondary80_channel(chspec))

/* pass an 80P80 chanspec (not channel) to get 20MHz subnand channel numbers */
#define LLL_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(LOWER_80_SB(chspec), -3)
#define LLU_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(LOWER_80_SB(chspec), -1)
#define LUL_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(LOWER_80_SB(chspec),  1)
#define LUU_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(LOWER_80_SB(chspec),  3)
#define ULL_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(UPPER_80_SB(chspec), -3)
#define ULU_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(UPPER_80_SB(chspec), -1)
#define UUL_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(UPPER_80_SB(chspec),  1)
#define UUU_20_SB_8080(chspec)  CH_OFF_10MHZ_MULTIPLES(UPPER_80_SB(chspec),  3)

/* get lowest 20MHz sideband of a given chspec
 * (works with 20, 40, 80, 160, 80p80)
 */
#define CH_FIRST_20_SB(chspec)  ((uint8) (\
		CHSPEC_IS160(chspec) ? LLL_20_SB_160(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS8080(chspec) ? LLL_20_SB_8080(chspec) : (\
			CHSPEC_IS80(chspec) ? LL_20_SB(CHSPEC_CHANNEL(chspec)) : (\
				CHSPEC_IS40(chspec) ? LOWER_20_SB(CHSPEC_CHANNEL(chspec)) : \
					CHSPEC_CHANNEL(chspec))))))

/* get upper most 20MHz sideband of a given chspec
 * (works with 20, 40, 80, 160, 80p80)
 */
#define CH_LAST_20_SB(chspec)  ((uint8) (\
		CHSPEC_IS160(chspec) ? UUU_20_SB_160(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS8080(chspec) ? UUU_20_SB_8080(chspec) : (\
			CHSPEC_IS80(chspec) ? UU_20_SB(CHSPEC_CHANNEL(chspec)) : (\
				CHSPEC_IS40(chspec) ? UPPER_20_SB(CHSPEC_CHANNEL(chspec)) : \
					CHSPEC_CHANNEL(chspec))))))

/* call this with chspec and a valid 20MHz sideband of this channel to get the next 20MHz sideband
 * (works with 80p80 only)
 * resolves to 0 if called with upper most channel
 */
#define CH_NEXT_20_SB_IN_8080(chspec, channel)  ((uint8) (\
		((uint8) ((channel) + CH_20MHZ_APART) > CH_LAST_20_SB(chspec) ? 0 : \
			((channel) == LUU_20_SB_8080(chspec) ? ULL_20_SB_8080(chspec) : \
				(channel) + CH_20MHZ_APART))))

/* call this with chspec and a valid 20MHz sideband of this channel to get the next 20MHz sideband
 * (works with 20, 40, 80, 160, 80p80)
 * resolves to 0 if called with upper most channel
 */
#define CH_NEXT_20_SB(chspec, channel)  ((uint8) (\
		(CHSPEC_IS8080(chspec) ? CH_NEXT_20_SB_IN_8080((chspec), (channel)) : \
			((uint8) ((channel) + CH_20MHZ_APART) > CH_LAST_20_SB(chspec) ? 0 : \
				((channel) + CH_20MHZ_APART)))))

#else /* WL11AC_80P80, WL11AC_160 */

#define LLL_20_SB_160(channel)  0
#define LLU_20_SB_160(channel)  0
#define LUL_20_SB_160(channel)  0
#define LUU_20_SB_160(channel)  0
#define ULL_20_SB_160(channel)  0
#define ULU_20_SB_160(channel)  0
#define UUL_20_SB_160(channel)  0
#define UUU_20_SB_160(channel)  0

#define LOWER_80_SB(chspec)	0

#define UPPER_80_SB(chspec)	0

#define LLL_20_SB_8080(chspec)	0
#define LLU_20_SB_8080(chspec)	0
#define LUL_20_SB_8080(chspec)	0
#define LUU_20_SB_8080(chspec)	0
#define ULL_20_SB_8080(chspec)	0
#define ULU_20_SB_8080(chspec)	0
#define UUL_20_SB_8080(chspec)	0
#define UUU_20_SB_8080(chspec)	0

/* get lowest 20MHz sideband of a given chspec
 * (works with 20, 40, 80)
 */
#define CH_FIRST_20_SB(chspec)  ((uint8) (\
			CHSPEC_IS80(chspec) ? LL_20_SB(CHSPEC_CHANNEL(chspec)) : (\
				CHSPEC_IS40(chspec) ? LOWER_20_SB(CHSPEC_CHANNEL(chspec)) : \
					CHSPEC_CHANNEL(chspec))))
/* get upper most 20MHz sideband of a given chspec
 * (works with 20, 40, 80, 160, 80p80)
 */
#define CH_LAST_20_SB(chspec)  ((uint8) (\
			CHSPEC_IS80(chspec) ? UU_20_SB(CHSPEC_CHANNEL(chspec)) : (\
				CHSPEC_IS40(chspec) ? UPPER_20_SB(CHSPEC_CHANNEL(chspec)) : \
					CHSPEC_CHANNEL(chspec))))

/* call this with chspec and a valid 20MHz sideband of this channel to get the next 20MHz sideband
 * (works with 20, 40, 80, 160, 80p80)
 * resolves to 0 if called with upper most channel
 */
#define CH_NEXT_20_SB(chspec, channel)  ((uint8) (\
			((uint8) ((channel) + CH_20MHZ_APART) > CH_LAST_20_SB(chspec) ? 0 : \
				((channel) + CH_20MHZ_APART))))

#endif /* WL11AC_80P80, WL11AC_160 */

/* Iterator for 20MHz side bands of a chanspec: (chanspec_t chspec, uint8 channel)
 * 'chspec' chanspec_t of interest (used in loop, better to pass a resolved value than a macro)
 * 'channel' must be a variable (not an expression).
 */
#define FOREACH_20_SB(chspec, channel) \
		for (channel = CH_FIRST_20_SB(chspec); channel; \
			channel = CH_NEXT_20_SB((chspec), channel))

/* Uses iterator to populate array with all side bands involved (sorted lower to upper).
 *     'chspec' chanspec_t of interest
 *     'psb' pointer to uint8 array of enough size to hold all side bands for the given chspec
 */
#define GET_ALL_SB(chspec, psb) do { \
		uint8 channel, idx = 0; \
		chanspec_t chspec_local = chspec; \
		FOREACH_20_SB(chspec_local, channel) \
			(psb)[idx++] = channel; \
} while (0)

#define BW_LE20(bw)	((bw) == WL_CHANSPEC_BW_20)
#define CHSPEC_IS20(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)
#define CHSPEC_ISST20(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)
#define CHSPEC_ISLE20(chspec)	(CHSPEC_IS20(chspec))

/* given a chanspec_t of any bw, returns if control SB is in lower 20, 40, 80 or not respectively */
#define IS_CTL_IN_L20(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_U) /* CTL SB is in low 20 of any 40 */
#define IS_CTL_IN_L40(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_UL)	/* in low 40 of any 80 */
#define IS_CTL_IN_L80(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_ULL)	/* in low 80 of 80p80/160 */

/* Populates array with all 20MHz side bands of a given chanspec_t in the following order:
 *		control, ext20, two ext40s, four ext80s.
 *     'chspec' is the chanspec of interest
 *     'pext' must point to an uint8 array of long enough to hold all side bands of the given chspec
 *
 * Works with 20, 40, 80, 80p80 and 160MHz chspec
 */
#define GET_ALL_EXT(chspec, pext) do { \
	chanspec_t t = (CHSPEC_IS160(chspec) || CHSPEC_IS8080(chspec)) ? /* if bw > 80MHz */\
			wf_chspec_primary80_chspec(chspec) : (chspec); /* extract primary 80 */\
	uint8 ctl_ch = (pext)[0] = wf_chspec_ctlchan(t); /* control channel as first element */\
	if (CHSPEC_ISLE20(chspec)) break; /* nothing more to do since 20MHz chspec */\
	(pext)[1] = ctl_ch + (IS_CTL_IN_L20(t) ? CH_20MHZ_APART : -CH_20MHZ_APART); /* 20MHz EXT */\
	if (CHSPEC_IS40(chspec)) break; /* nothing more to do since 40MHz chspec */\
	t = wf_channel2chspec(ctl_ch + (IS_CTL_IN_L40(chspec) ? CH_40MHZ_APART : -CH_40MHZ_APART), \
			WL_CHANSPEC_BW_40); /* center 40MHz EXT */\
	GET_ALL_SB(t, &((pext)[2])); /* get the 20MHz side bands in 40MHz EXT */\
	if (CHSPEC_IS80(chspec)) break; /* nothing more to do since 80MHz chspec */\
	t = CH80MHZ_CHSPEC(wf_chspec_secondary80_channel(chspec), WL_CHANSPEC_CTL_SB_LLL); \
	GET_ALL_SB(t, &((pext)[4])); /* get the 20MHz side bands in 80MHz EXT (secondary) */\
} while (0)

#endif /* !WL11N_20MHZONLY */

#define CHSPEC_IS5G(chspec)	(((chspec) & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_5G)
#define CHSPEC_IS2G(chspec)	(((chspec) & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_2G)
#define CHSPEC_SB_UPPER(chspec)	\
	((((chspec) & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_UPPER) && \
	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40))
#define CHSPEC_SB_LOWER(chspec)	\
	((((chspec) & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER) && \
	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40))
#define CHSPEC2WLC_BAND(chspec) (CHSPEC_IS5G(chspec) ? WLC_BAND_5G : WLC_BAND_2G)

/**
 * Number of chars needed for wf_chspec_ntoa() destination character buffer.
 */
#define CHANSPEC_STR_LEN    20


#define CHSPEC_IS_BW_160_WIDE(chspec) (CHSPEC_BW(chspec) == WL_CHANSPEC_BW_160 ||\
	CHSPEC_BW(chspec) == WL_CHANSPEC_BW_8080)

/* BW inequality comparisons, LE (<=), GE (>=), LT (<), GT (>), comparisons can be made
* as simple numeric comparisons, with the exception that 160 is the same BW as 80+80,
* but have different numeric values; (WL_CHANSPEC_BW_160 < WL_CHANSPEC_BW_8080).
*
* The LT/LE/GT/GE macros check first checks whether both chspec bandwidth and bw are 160 wide.
* If both chspec bandwidth and bw is not 160 wide, then the comparison is made.
*/
#define CHSPEC_BW_GE(chspec, bw) \
	((CHSPEC_IS_BW_160_WIDE(chspec) &&\
	(bw == WL_CHANSPEC_BW_160 || bw == WL_CHANSPEC_BW_8080)) ||\
	(CHSPEC_BW(chspec) >= bw))

#define CHSPEC_BW_LE(chspec, bw) \
	((CHSPEC_IS_BW_160_WIDE(chspec) &&\
	(bw == WL_CHANSPEC_BW_160 || bw == WL_CHANSPEC_BW_8080)) ||\
	(CHSPEC_BW(chspec) <= bw))

#define CHSPEC_BW_GT(chspec, bw) \
	(!(CHSPEC_IS_BW_160_WIDE(chspec) &&\
	(bw == WL_CHANSPEC_BW_160 || bw == WL_CHANSPEC_BW_8080)) &&\
	(CHSPEC_BW(chspec) > bw))

#define CHSPEC_BW_LT(chspec, bw) \
	(!(CHSPEC_IS_BW_160_WIDE(chspec) &&\
	(bw == WL_CHANSPEC_BW_160 || bw == WL_CHANSPEC_BW_8080)) &&\
	(CHSPEC_BW(chspec) < bw))

/* Legacy Chanspec defines
 * These are the defines for the previous format of the chanspec_t
 */
#define WL_LCHANSPEC_CHAN_MASK		0x00ff
#define WL_LCHANSPEC_CHAN_SHIFT		     0

#define WL_LCHANSPEC_CTL_SB_MASK	0x0300
#define WL_LCHANSPEC_CTL_SB_SHIFT	     8
#define WL_LCHANSPEC_CTL_SB_LOWER	0x0100
#define WL_LCHANSPEC_CTL_SB_UPPER	0x0200
#define WL_LCHANSPEC_CTL_SB_NONE	0x0300

#define WL_LCHANSPEC_BW_MASK		0x0C00
#define WL_LCHANSPEC_BW_SHIFT		    10
#define WL_LCHANSPEC_BW_10		0x0400
#define WL_LCHANSPEC_BW_20		0x0800
#define WL_LCHANSPEC_BW_40		0x0C00

#define WL_LCHANSPEC_BAND_MASK		0xf000
#define WL_LCHANSPEC_BAND_SHIFT		    12
#define WL_LCHANSPEC_BAND_5G		0x1000
#define WL_LCHANSPEC_BAND_2G		0x2000

#define LCHSPEC_CHANNEL(chspec)	((uint8)((chspec) & WL_LCHANSPEC_CHAN_MASK))
#define LCHSPEC_BAND(chspec)	((chspec) & WL_LCHANSPEC_BAND_MASK)
#define LCHSPEC_CTL_SB(chspec)	((chspec) & WL_LCHANSPEC_CTL_SB_MASK)
#define LCHSPEC_BW(chspec)	((chspec) & WL_LCHANSPEC_BW_MASK)
#define LCHSPEC_IS10(chspec)	(((chspec) & WL_LCHANSPEC_BW_MASK) == WL_LCHANSPEC_BW_10)
#define LCHSPEC_IS20(chspec)	(((chspec) & WL_LCHANSPEC_BW_MASK) == WL_LCHANSPEC_BW_20)
#define LCHSPEC_IS40(chspec)	(((chspec) & WL_LCHANSPEC_BW_MASK) == WL_LCHANSPEC_BW_40)
#define LCHSPEC_IS5G(chspec)	(((chspec) & WL_LCHANSPEC_BAND_MASK) == WL_LCHANSPEC_BAND_5G)
#define LCHSPEC_IS2G(chspec)	(((chspec) & WL_LCHANSPEC_BAND_MASK) == WL_LCHANSPEC_BAND_2G)

#define LCHSPEC_SB_UPPER(chspec)	\
	((((chspec) & WL_LCHANSPEC_CTL_SB_MASK) == WL_LCHANSPEC_CTL_SB_UPPER) && \
	(((chspec) & WL_LCHANSPEC_BW_MASK) == WL_LCHANSPEC_BW_40))
#define LCHSPEC_SB_LOWER(chspec)	\
	((((chspec) & WL_LCHANSPEC_CTL_SB_MASK) == WL_LCHANSPEC_CTL_SB_LOWER) && \
	(((chspec) & WL_LCHANSPEC_BW_MASK) == WL_LCHANSPEC_BW_40))

#define LCHSPEC_CREATE(chan, band, bw, sb)  ((uint16)((chan) | (sb) | (bw) | (band)))

#define CH20MHZ_LCHSPEC(channel) \
	(chanspec_t)((chanspec_t)(channel) | WL_LCHANSPEC_BW_20 | \
	WL_LCHANSPEC_CTL_SB_NONE | (((channel) <= CH_MAX_2G_CHANNEL) ? \
	WL_LCHANSPEC_BAND_2G : WL_LCHANSPEC_BAND_5G))

/*
 * WF_CHAN_FACTOR_* constants are used to calculate channel frequency
 * given a channel number.
 * chan_freq = chan_factor * 500Mhz + chan_number * 5
 */

/**
 * Channel Factor for the starting frequence of 2.4 GHz channels.
 * The value corresponds to 2407 MHz.
 */
#define WF_CHAN_FACTOR_2_4_G		4814	/* 2.4 GHz band, 2407 MHz */

/**
 * Channel Factor for the starting frequence of 5 GHz channels.
 * The value corresponds to 5000 MHz.
 */
#define WF_CHAN_FACTOR_5_G		10000	/* 5   GHz band, 5000 MHz */

/**
 * Channel Factor for the starting frequence of 4.9 GHz channels.
 * The value corresponds to 4000 MHz.
 */
#define WF_CHAN_FACTOR_4_G		8000	/* 4.9 GHz band for Japan */

#define WLC_2G_25MHZ_OFFSET		5	/* 2.4GHz band channel offset */

/**
 *  No of sub-band vlaue of the specified Mhz chanspec
 */
#define WF_NUM_SIDEBANDS_40MHZ   2
#define WF_NUM_SIDEBANDS_80MHZ   4
#define WF_NUM_SIDEBANDS_8080MHZ 4
#define WF_NUM_SIDEBANDS_160MHZ  8

/* Get bandwidth of chanspec in half MHz;
 * works with 2.5MHz to 160MHz (including 80p80) chanspecs
 *
 * @param	chspec		chanspec_t format
 *
 * @return	bandwidth of a chanspec in half MHz units
 */
extern uint wf_bw_chspec_to_half_mhz(chanspec_t chspec);

/**
 * Convert chanspec to ascii string
 *
 * @param	chspec		chanspec format
 * @param	buf		ascii string of chanspec
 *
 * @return	pointer to buf with room for at least CHANSPEC_STR_LEN bytes
 *
 * @see		CHANSPEC_STR_LEN
 */
extern char * wf_chspec_ntoa(chanspec_t chspec, char *buf);

/**
 * Convert ascii string to chanspec
 *
 * @param	a     pointer to input string
 *
 * @return	>= 0 if successful or 0 otherwise
 */
extern chanspec_t wf_chspec_aton(const char *a);

/**
 * Verify the chanspec fields are valid.
 *
 * Verify the chanspec is using a legal set field values, i.e. that the chanspec
 * specified a band, bw, ctl_sb and channel and that the combination could be
 * legal given some set of circumstances.
 *
 * @param	chanspec   input chanspec to verify
 *
 * @return TRUE if the chanspec is malformed, FALSE if it looks good.
 */
extern bool wf_chspec_malformed(chanspec_t chanspec);

/**
 * Verify the chanspec specifies a valid channel according to 802.11.
 *
 * @param	chanspec   input chanspec to verify
 *
 * @return TRUE if the chanspec is a valid 802.11 channel
 */
extern bool wf_chspec_valid(chanspec_t chanspec);

/**
 * Return the primary (control) channel.
 *
 * This function returns the channel number of the primary 20MHz channel. For
 * 20MHz channels this is just the channel number. For 40MHz or wider channels
 * it is the primary 20MHz channel specified by the chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the channel number of the primary 20MHz channel
 */
extern uint8 wf_chspec_ctlchan(chanspec_t chspec);

/**
 * Return the primary (control) chanspec.
 *
 * This function returns the chanspec of the primary 20MHz channel. For 20MHz
 * channels this is just the chanspec. For 40MHz or wider channels it is the
 * chanspec of the primary 20MHZ channel specified by the chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the chanspec of the primary 20MHz channel
 */
extern chanspec_t wf_chspec_ctlchspec(chanspec_t chspec);

/**
 * Return a channel number corresponding to a frequency.
 *
 * This function returns the chanspec for the primary 40MHz of an 80MHz channel.
 * The control sideband specifies the same 20MHz channel that the 80MHz channel is using
 * as the primary 20MHz channel.
 */
extern chanspec_t wf_chspec_primary40_chspec(chanspec_t chspec);

/*
 * Return the channel number for a given frequency and base frequency.
 * The returned channel number is relative to the given base frequency.
 * If the given base frequency is zero, a base frequency of 5 GHz is assumed for
 * frequencies from 5 - 6 GHz, and 2.407 GHz is assumed for 2.4 - 2.5 GHz.
 *
 * Frequency is specified in MHz.
 * The base frequency is specified as (start_factor * 500 kHz).
 * Constants WF_CHAN_FACTOR_2_4_G, WF_CHAN_FACTOR_5_G are defined for
 * 2.4 GHz and 5 GHz bands.
 *
 * The returned channel will be in the range [1, 14] in the 2.4 GHz band
 * and [0, 200] otherwise.
 * -1 is returned if the start_factor is WF_CHAN_FACTOR_2_4_G and the
 * frequency is not a 2.4 GHz channel, or if the frequency is not and even
 * multiple of 5 MHz from the base frequency to the base plus 1 GHz.
 *
 * Reference 802.11 REVma, section 17.3.8.3, and 802.11B section 18.4.6.2
 *
 * @param	freq          frequency in MHz
 * @param	start_factor  base frequency in 500 kHz units, e.g. 10000 for 5 GHz
 *
 * @return Returns a channel number
 *
 * @see  WF_CHAN_FACTOR_2_4_G
 * @see  WF_CHAN_FACTOR_5_G
 */
extern int wf_mhz2channel(uint freq, uint start_factor);

/**
 * Return the center frequency in MHz of the given channel and base frequency.
 *
 * Return the center frequency in MHz of the given channel and base frequency.
 * The channel number is interpreted relative to the given base frequency.
 *
 * The valid channel range is [1, 14] in the 2.4 GHz band and [0, 200] otherwise.
 * The base frequency is specified as (start_factor * 500 kHz).
 * Constants WF_CHAN_FACTOR_2_4_G, WF_CHAN_FACTOR_5_G are defined for
 * 2.4 GHz and 5 GHz bands.
 * The channel range of [1, 14] is only checked for a start_factor of
 * WF_CHAN_FACTOR_2_4_G (4814).
 * Odd start_factors produce channels on .5 MHz boundaries, in which case
 * the answer is rounded down to an integral MHz.
 * -1 is returned for an out of range channel.
 *
 * Reference 802.11 REVma, section 17.3.8.3, and 802.11B section 18.4.6.2
 *
 * @param	channel       input channel number
 * @param	start_factor  base frequency in 500 kHz units, e.g. 10000 for 5 GHz
 *
 * @return Returns a frequency in MHz
 *
 * @see  WF_CHAN_FACTOR_2_4_G
 * @see  WF_CHAN_FACTOR_5_G
 */
extern int wf_channel2mhz(uint channel, uint start_factor);

/**
 * Returns the chanspec 80Mhz channel corresponding to the following input
 * parameters
 *
 *	primary_channel - primary 20Mhz channel
 *	center_channel   - center frequecny of the 80Mhz channel
 *
 * The center_channel can be one of {42, 58, 106, 122, 138, 155}
 *
 * returns INVCHANSPEC in case of error
 */
extern chanspec_t wf_chspec_80(uint8 center_channel, uint8 primary_channel);

/**
 * Convert ctl chan and bw to chanspec
 *
 * @param	ctl_ch		channel
 * @param	bw	        bandwidth
 *
 * @return	> 0 if successful or 0 otherwise
 *
 */
extern uint16 wf_channel2chspec(uint ctl_ch, uint bw);

extern uint wf_channel2freq(uint channel);
extern uint wf_freq2channel(uint freq);

/*
 * Returns the 80+80 MHz chanspec corresponding to the following input parameters
 *
 *    primary_20mhz - Primary 20 MHz channel
 *    chan0_80MHz - center channel number of one frequency segment
 *    chan1_80MHz - center channel number of the other frequency segment
 *
 * Parameters chan0_80MHz and chan1_80MHz are channel numbers in {42, 58, 106, 122, 138, 155}.
 * The primary channel must be contained in one of the 80MHz channels. This routine
 * will determine which frequency segment is the primary 80 MHz segment.
 *
 * Returns INVCHANSPEC in case of error.
 *
 * Refer to IEEE802.11ac section 22.3.14 "Channelization".
 */
extern chanspec_t wf_chspec_get8080_chspec(uint8 primary_20mhz,
	uint8 chan0_80Mhz, uint8 chan1_80Mhz);

/*
 * Returns the primary 80 Mhz channel for the provided chanspec
 *
 *    chanspec - Input chanspec for which the 80MHz primary channel has to be retrieved
 *
 *  returns -1 in case the provided channel is 20/40 Mhz chanspec
 */
extern uint8 wf_chspec_primary80_channel(chanspec_t chanspec);

/*
 * Returns the secondary 80 Mhz channel for the provided chanspec
 *
 *    chanspec - Input chanspec for which the 80MHz secondary channel has to be retrieved
 *
 *  returns -1 in case the provided channel is 20/40 Mhz chanspec
 */
extern uint8 wf_chspec_secondary80_channel(chanspec_t chanspec);

/*
 * This function returns the chanspec for the primary 80MHz of an 160MHz or 80+80 channel.
 */
extern chanspec_t wf_chspec_primary80_chspec(chanspec_t chspec);

#ifdef WL11AC_80P80
/*
 * This function returns the centre chanel for the given chanspec.
 * In case of 80+80 chanspec it returns the primary 80 Mhz centre channel
 */
extern uint8 wf_chspec_channel(chanspec_t chspec);
#endif

/*
 * Given two chanspecs, returns true if they overlap.
 * (Overlap: At least one 20MHz subband is common between the two chanspecs provided)
 */
extern bool wf_chspec_overlap(chanspec_t chspec0, chanspec_t chspec1);
#endif	/* _bcmwifi_channels_h_ */
