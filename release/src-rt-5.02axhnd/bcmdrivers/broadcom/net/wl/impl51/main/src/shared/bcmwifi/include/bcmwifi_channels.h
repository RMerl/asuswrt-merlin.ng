/*
 * Misc utility routines for WL and Apps
 * This header file housing the define and function prototype use by
 * both the wl driver, tools & Apps.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcmwifi_channels.h 779980 2019-10-11 11:56:42Z $
 */

#ifndef	_bcmwifi_channels_h_
#define	_bcmwifi_channels_h_

/* A chanspec holds the channel number, band, bandwidth and primary 20MHz sub-band */
typedef uint16 chanspec_t;
typedef uint16 chanspec_band_t;
typedef uint16 chanspec_bw_t;
typedef uint16 chanspec_subband_t;

/* channel defines */
#define CH_UPPER_SB			0x01
#define CH_LOWER_SB			0x02
#define CH_EWA_VALID			0x04

#define CH_80MHZ_APART                   16u
#define CH_40MHZ_APART                    8u
#define CH_20MHZ_APART                    4u
#define CH_10MHZ_APART                    2u
#define CH_5MHZ_APART                     1u    /* 2G band channels are 5 Mhz apart */

#define CH_MIN_2G_CHANNEL                 1u    /* Min channel in 2G band */
#define CH_MAX_2G_CHANNEL                14u    /* Max channel in 2G band */
#define CH_MIN_2G_40M_CHANNEL             3u    /* Min 40MHz center channel in 2G band */
#define CH_MAX_2G_40M_CHANNEL            11u    /* Max 40MHz center channel in 2G band */

#define CH_MIN_5G_CHANNEL		 36u	/* Min channel in 5G band */
#define CH_MAX_5G_CHANNEL		181u	/* Max channel in 5G band */
#define CH_MIN_5G_40M_CHANNEL            38u    /* Min 40MHz center channel in 5G band */
#define CH_MAX_5G_40M_CHANNEL           179u    /* Max 40MHz center channel in 5G band */
#define CH_MIN_5G_80M_CHANNEL            42u    /* Min 80MHz center channel in 6G band */
#define CH_MAX_5G_80M_CHANNEL           171u    /* Max 80MHz center channel in 6G band */
#define CH_MIN_5G_160M_CHANNEL           50u    /* Min 160MHz center channel in 6G band */
#define CH_MAX_5G_160M_CHANNEL          163u    /* Max 160MHz center channel in 6G band */

#define CH_MIN_6G_CHANNEL                 1u    /* Min channel in 6G band */
#define CH_MAX_6G_CHANNEL               233u    /* Max channel in 6G band */
#define CH_MIN_6G_40M_CHANNEL             3u    /* Min 40MHz center channel in 6G band */
#define CH_MAX_6G_40M_CHANNEL           227u    /* Max 40MHz center channel in 6G band */
#define CH_MIN_6G_80M_CHANNEL             7u    /* Min 80MHz center channel in 6G band */
#define CH_MAX_6G_80M_CHANNEL           215u    /* Max 80MHz center channel in 6G band */
#define CH_MIN_6G_160M_CHANNEL           15u    /* Min 160MHz center channel in 6G band */
#define CH_MAX_6G_160M_CHANNEL          207u    /* Max 160MHz center channel in 6G band */

/* maximum # channels the s/w supports */
#define MAXCHANNEL                      254u    /* max # supported channels.
						 * DO NOT MAKE > 255: channels are uint8's all over
						 */

#define MAXCHANNEL_NUM	(MAXCHANNEL - 1)	/* max channel number */

/* length of channel vector bitmap is the MAXCHANNEL we want to handle rounded up to a byte */
#define CHANVEC_LEN ((MAXCHANNEL + (8 - 1)) / 8)

#define INVCHANNEL              255u     /* error value for a bad channel */

/* channel bitvec */
typedef struct {
	uint8   vec[CHANVEC_LEN];   /* bitvec of channels */
} chanvec_t;

/* make sure channel num is within valid range */
#define CH_NUM_VALID_RANGE(ch_num) ((ch_num) > 0 && (ch_num) <= MAXCHANNEL_NUM)

#define CHSPEC_CTLOVLP(sp1, sp2, sep)	\
	((uint)ABS(wf_chspec_ctlchan(sp1) - wf_chspec_ctlchan(sp2)) < (uint)(sep))

#define CHSPEC_CHNLOVLP(sp1, sp2, sep) \
	((uint)ABS(CHSPEC_CHANNEL(sp1) - CHSPEC_CHANNEL(sp2)) <= (uint)(sep))

/* All builds use the new 11ac ratespec/chanspec */
#undef  D11AC_IOTYPES
#define D11AC_IOTYPES

#define WL_CHANSPEC_CHAN_MASK		0x00ffu
#define WL_CHANSPEC_CHAN_SHIFT		0u
#define WL_CHANSPEC_CHAN0_MASK		0x000fu
#define WL_CHANSPEC_CHAN0_SHIFT		0u
#define WL_CHANSPEC_CHAN1_MASK		0x00f0u
#define WL_CHANSPEC_CHAN1_SHIFT		4u

#define WL_CHANSPEC_CTL_SB_MASK		0x0700u
#define WL_CHANSPEC_CTL_SB_SHIFT	8u
#define WL_CHANSPEC_CTL_SB_LLL		0x0000u
#define WL_CHANSPEC_CTL_SB_LLU		0x0100u
#define WL_CHANSPEC_CTL_SB_LUL		0x0200u
#define WL_CHANSPEC_CTL_SB_LUU		0x0300u
#define WL_CHANSPEC_CTL_SB_ULL		0x0400u
#define WL_CHANSPEC_CTL_SB_ULU		0x0500u
#define WL_CHANSPEC_CTL_SB_UUL		0x0600u
#define WL_CHANSPEC_CTL_SB_UUU		0x0700u
#define WL_CHANSPEC_CTL_SB_LL		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_LU		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_UL		WL_CHANSPEC_CTL_SB_LUL
#define WL_CHANSPEC_CTL_SB_UU		WL_CHANSPEC_CTL_SB_LUU
#define WL_CHANSPEC_CTL_SB_L		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_U		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_LOWER	WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_UPPER	WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_NONE		WL_CHANSPEC_CTL_SB_LLL

#define WL_CHANSPEC_BW_MASK		0x3800u
#define WL_CHANSPEC_BW_SHIFT		11u
#define WL_CHANSPEC_BW_5		0x0000u
#define WL_CHANSPEC_BW_10		0x0800u
#define WL_CHANSPEC_BW_20		0x1000u
#define WL_CHANSPEC_BW_40		0x1800u
#define WL_CHANSPEC_BW_80		0x2000u
#define WL_CHANSPEC_BW_160		0x2800u
#define WL_CHANSPEC_BW_8080		0x3000u
#define WL_CHANSPEC_BW_2P5		0x3800u

#define WL_CHANSPEC_BAND_MASK		0xc000u
#define WL_CHANSPEC_BAND_SHIFT		14u
#define WL_CHANSPEC_BAND_2G		0x0000u
#define WL_CHANSPEC_BAND_6G		0x4000u
#define WL_CHANSPEC_BAND_4G		0x8000u
#define WL_CHANSPEC_BAND_5G		0xc000u
#define INVCHANSPEC			255u
#define MAX_CHANSPEC			0xFFFFu

#define WL_CHANNEL_BAND(ch) (((uint)(ch) <= CH_MAX_2G_CHANNEL) ? \
	WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G)

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

/*
 * TODO:6GHZ:CH20MHZ_CHSPEC() does not return 6GHz chanspecs, is slowly converted into
 * CH20MHZ_CHSPEC2().
 */
#define CH20MHZ_CHSPEC(channel)		(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
						WL_CHANNEL_BAND(channel))
#define CH20MHZ_CHSPEC2(channel, bt)	(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
						BANDTYPE_CHSPEC(bt))
#define CH2P5MHZ_CHSPEC(channel)	(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_2P5 | \
						WL_CHANNEL_BAND(channel))
#define CH5MHZ_CHSPEC(channel)		(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_5 | \
						WL_CHANNEL_BAND(channel))
#define CH10MHZ_CHSPEC(channel)		(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_10 | \
						WL_CHANNEL_BAND(channel))
#define NEXT_20MHZ_CHAN(channel)	(((channel) < (MAXCHANNEL - CH_20MHZ_APART)) ? \
						((channel) + CH_20MHZ_APART) : 0)
#define CH40MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
						((channel) | (ctlsb) | WL_CHANSPEC_BW_40 | \
						WL_CHANNEL_BAND(channel))
#define CH80MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | \
						 WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G)
#define CH160MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | \
						 WL_CHANSPEC_BW_160 | WL_CHANSPEC_BAND_5G)
#define CHBW_CHSPEC(bw, channel)	(chanspec_t)((chanspec_t)(channel) | (bw) | \
						WL_CHANNEL_BAND(channel))

/* simple MACROs to get different fields of chanspec */
#ifdef WL11AC_80P80
#define CHSPEC_CHANNEL(chspec)	wf_chspec_channel(chspec)
#else
#define CHSPEC_CHANNEL(chspec)	((uint8)((chspec) & WL_CHANSPEC_CHAN_MASK))
#endif // endif
#define CHSPEC_CHAN0(chspec)	((chspec) & WL_CHANSPEC_CHAN0_MASK) >> WL_CHANSPEC_CHAN0_SHIFT
#define CHSPEC_CHAN1(chspec)	((chspec) & WL_CHANSPEC_CHAN1_MASK) >> WL_CHANSPEC_CHAN1_SHIFT
#define CHSPEC_BAND(chspec)	((chspec) & WL_CHANSPEC_BAND_MASK)
#define CHSPEC_CTL_SB(chspec)	((chspec) & WL_CHANSPEC_CTL_SB_MASK)
#define CHSPEC_SB(chspec)	((chspec) & WL_CHANSPEC_CTL_SB_MASK) >> WL_CHANSPEC_CTL_SB_SHIFT
#define CHSPEC_BW(chspec)	((chspec) & WL_CHANSPEC_BW_MASK)

#define CHSPEC_IS2P5(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_2P5)
#define CHSPEC_IS5(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_5)
#define CHSPEC_IS10(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_10)
#define CHSPEC_IS20(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)
#define CHSPEC_IS20_5G(chspec)	((((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20) && \
								CHSPEC_IS5G(chspec))
#ifndef CHSPEC_IS40
#define CHSPEC_IS40(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40)
#endif // endif
#ifndef CHSPEC_IS80
#define CHSPEC_IS80(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_80)
#endif // endif
#ifndef CHSPEC_IS160
#define CHSPEC_IS160(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_160)
#endif // endif
#ifndef CHSPEC_IS8080
#define CHSPEC_IS8080(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_8080)
#endif // endif

/* pass a center channel and get channel offset from it by 10MHz */
#define CH_OFF_10MHZ_MULTIPLES(channel, offset)				\
((uint8) (((offset) < 0) ?						\
	(((channel) > (WL_CHANSPEC_CHAN_MASK & ((uint16)((-(offset)) * CH_10MHZ_APART)))) ? \
		((channel) + (offset) * CH_10MHZ_APART) : 0) :		\
	((((uint16)(channel) + (uint16)(offset) * CH_10MHZ_APART) < (uint16)MAXCHANNEL) ? \
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

/* given a chanspec of any bw, tests if primary20 SB is in lower 20, 40, 80 respectively */
#define IS_CTL_IN_L20(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_U) /* CTL SB is in low 20 of any 40 */
#define IS_CTL_IN_L40(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_UL)	/* in low 40 of any 80 */
#define IS_CTL_IN_L80(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_ULL)	/* in low 80 of 80p80/160 */

#define BW_LE20(bw)		((bw) == WL_CHANSPEC_BW_20)
#define CHSPEC_ISLE20(chspec)	(CHSPEC_IS20(chspec))

#define BW_LE40(bw)		(BW_LE20(bw) || ((bw) == WL_CHANSPEC_BW_40))
#define BW_LE80(bw)		(BW_LE40(bw) || ((bw) == WL_CHANSPEC_BW_80))
#define BW_LE160(bw)		(BW_LE80(bw) || ((bw) == WL_CHANSPEC_BW_160))
#define CHSPEC_BW_LE20(chspec)	(BW_LE20(CHSPEC_BW(chspec)))

#define CHSPEC_IS6G(chspec)	(((chspec) & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_6G)
#define CHSPEC_IS5G(chspec)	(((chspec) & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_5G)
#define CHSPEC_IS2G(chspec)	(((chspec) & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_2G)
#define CHSPEC_ISPHY5G6G(chspec)   (CHSPEC_IS5G(chspec) || CHSPEC_IS6G(chspec))
#define CHSPEC_SB_UPPER(chspec)	\
	((((chspec) & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_UPPER) && \
	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40))
#define CHSPEC_SB_LOWER(chspec)	\
	((((chspec) & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER) && \
	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40))
#define CHSPEC_BANDTYPE(chspec) \
	(CHSPEC_IS2G(chspec) ? WLC_BAND_2G : \
	 CHSPEC_IS6G(chspec) ? WLC_BAND_6G : WLC_BAND_5G)
#define BANDTYPE_CHSPEC(bt) (\
	(bt) == WLC_BAND_2G ? WL_CHANSPEC_BAND_2G: \
	((bt) == WLC_BAND_5G ? WL_CHANSPEC_BAND_5G: WL_CHANSPEC_BAND_6G))

/**
 * Number of chars needed for wf_chspec_ntoa() destination character buffer.
 */
#define CHANSPEC_STR_LEN    20

/*
 * This function returns TRUE if both the chanspec can co-exist in PHY.
 * Addition to primary20 channel, the function checks for side band for 2g 40 channels
 */
extern bool wf_chspec_coexist(chanspec_t chspec1, chanspec_t chspec2);

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
		((bw) == WL_CHANSPEC_BW_160 || (bw) == WL_CHANSPEC_BW_8080)) ||\
		(CHSPEC_BW(chspec) >= (bw)))

#define CHSPEC_BW_LE(chspec, bw) \
		((CHSPEC_IS_BW_160_WIDE(chspec) &&\
		((bw) == WL_CHANSPEC_BW_160 || (bw) == WL_CHANSPEC_BW_8080)) ||\
		(CHSPEC_BW(chspec) <= (bw)))

#define CHSPEC_BW_GT(chspec, bw) \
		(!(CHSPEC_IS_BW_160_WIDE(chspec) &&\
		((bw) == WL_CHANSPEC_BW_160 || (bw) == WL_CHANSPEC_BW_8080)) &&\
		(CHSPEC_BW(chspec) > (bw)))

#define CHSPEC_BW_LT(chspec, bw) \
		(!(CHSPEC_IS_BW_160_WIDE(chspec) &&\
		((bw) == WL_CHANSPEC_BW_160 || (bw) == WL_CHANSPEC_BW_8080)) &&\
		(CHSPEC_BW(chspec) < (bw)))

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

#define GET_ALL_EXT wf_get_all_ext

/*
 * WF_CHAN_FACTOR_* constants are used to calculate channel frequency
 * given a channel number.
 * chan_freq = chan_factor * 500Mhz + chan_number * 5
 */

/**
 * Channel Factor for the starting frequence of 2.4 GHz channels.
 * The value corresponds to 2407 MHz.
 */
#define WF_CHAN_FACTOR_2_4_G		4814u	/* 2.4 GHz band, 2407 MHz */

/**
 * Channel Factor for the starting frequence of 4.9 GHz channels.
 * The value corresponds to 4000 MHz.
 */
#define WF_CHAN_FACTOR_4_G		8000u	/* 4.9 GHz band for Japan */

/**
 * Channel Factor for the starting frequence of 5 GHz channels.
 * The value corresponds to 5000 MHz.
 */
#define WF_CHAN_FACTOR_5_G		10000u	/* 5   GHz band, 5000 MHz */

/**
 * Channel Factor for the starting frequence of 6 GHz channels.
 * The value corresponds to 5940 MHz.
 */
#define WF_CHAN_FACTOR_6_G		11880u	/* 6   GHz band, 5940 MHz */

#define WLC_2G_25MHZ_OFFSET		5	/* 2.4GHz band channel offset */

/**
 *  No of sub-band vlaue of the specified Mhz chanspec
 */
#define WF_NUM_SIDEBANDS_40MHZ   2u
#define WF_NUM_SIDEBANDS_80MHZ   4u
#define WF_NUM_SIDEBANDS_8080MHZ 4u
#define WF_NUM_SIDEBANDS_160MHZ  8u

/**
 * Return the chanspec bandwidth in half MHz
 */
uint16 wf_bw_chspec_to_half_mhz(chanspec_t chspec);

/**
 * Return the bandwidth string for a given chanspec
 */
const char *wf_chspec_to_bw_str(chanspec_t chspec);

/**
 * Convert chanspec to ascii string, or formats hex of an invalid chanspec.
 */
char * wf_chspec_ntoa_ex(chanspec_t chspec, char *buf);

/**
 * Convert chanspec to ascii string, or returns NULL on error.
 */
char * wf_chspec_ntoa(chanspec_t chspec, char *buf);

/**
 * Convert ascii string to chanspec
 */
chanspec_t wf_chspec_aton(const char *a);

/**
 * Verify the chanspec fields are valid for a chanspec_t
 */
bool wf_chspec_malformed(chanspec_t chanspec);

/**
 * Verify the chanspec specifies a valid channel according to 802.11.
 */
bool wf_chspec_valid(chanspec_t chanspec);

/**
 * Verify that the channel is a valid 20MHz channel according to 802.11.
 */
bool wf_valid_20MHz_chan(uint channel, chanspec_band_t band);

/**
 * Verify that the center channel is a valid 40MHz center channel according to 802.11.
 */
bool wf_valid_40MHz_center_chan(uint center_channel, chanspec_band_t band);

/**
 * Verify that the center channel is a valid 80MHz center channel according to 802.11.
 */
bool wf_valid_80MHz_center_chan(uint center_channel, chanspec_band_t band);

/**
 * Verify that the center channel is a valid 160MHz center channel according to 802.11.
 */
bool wf_valid_160MHz_center_chan(uint center_channel, chanspec_band_t band);

/**
 * Create a 20MHz chanspec for the given band.
 */
chanspec_t wf_create_20MHz_chspec(uint channel, chanspec_band_t band);

/**
 * Returns the chanspec for a 40MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 */
chanspec_t wf_create_40MHz_chspec(uint primary_channel, uint center_channel,
                                  chanspec_band_t band);

/**
 * Returns the chanspec for a 40MHz channel given the primary 20MHz channel number,
 * the sub-band for the primary 20MHz channel, and the band.
 */
chanspec_t wf_create_40MHz_chspec_primary_sb(uint primary_channel,
                                             chanspec_subband_t primary_subband,
                                             chanspec_band_t band);
/**
 * Returns the chanspec for an 80MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 */
chanspec_t wf_create_80MHz_chspec(uint primary_channel, uint center_channel,
                                  chanspec_band_t band);

/**
 * Returns the chanspec for an 80+80MHz channel given the primary 20MHz channel number,
 * the center channel numbers for each frequency segment, and the band.
 */
chanspec_t wf_create_8080MHz_chspec(uint primary_channel, uint chan0, uint chan1,
                                    chanspec_band_t band);

/**
 * Returns the chanspec given the primary 20MHz channel number,
 * the center channel number, channel width, and the band.
 *
 * The channel width must be 20, 40, 80, or 160 MHz. 80+80 MHz chanspec creation
 * is not handled by this function, use  wf_create_8080MHz_chspec() instead.
 */
chanspec_t wf_create_chspec(uint primary_channel, uint center_channel,
                            chanspec_bw_t bw, chanspec_band_t band);

/**
 * Returns the chanspec given the primary 20MHz channel number,
 * channel width, and the band.
 */
chanspec_t wf_create_chspec_from_primary(uint primary_channel, chanspec_bw_t bw,
                                         chanspec_band_t band);

/**
 * Return the primary 20MHz channel.
 */
uint8 wf_chspec_primary20_chan(chanspec_t chspec);

/* alias for old function name */
#define wf_chspec_ctlchan(c) wf_chspec_primary20_chan(c)

/**
 * Return the primary 20MHz chanspec of a given chanspec
 */
chanspec_t wf_chspec_primary20_chspec(chanspec_t chspec);

/* alias for old function name */
#define wf_chspec_ctlchspec(c) wf_chspec_primary20_chspec(c)

/**
 * Return the primary 40MHz chanspec for a 40MHz or wider channel
 */
chanspec_t wf_chspec_primary40_chspec(chanspec_t chspec);

/**
 * Return the channel number for a given frequency and base frequency
 */
int wf_mhz2channel(uint freq, uint start_factor);

/**
 * Return the center frequency in MHz of the given channel and base frequency.
 */
int wf_channel2mhz(uint channel, uint start_factor);

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
 * Refer to 802.11-2016 section 22.3.14 "Channelization".
 */
extern chanspec_t wf_chspec_get8080_chspec(uint8 primary_20mhz,
	uint8 chan0_80Mhz, uint8 chan1_80Mhz);

/**
 * Returns the center channel of the primary 80 MHz sub-band of the provided chanspec
 *
 * @param	chspec    input chanspec
 *
 * @return	center channel number of the primary 80MHz sub-band of the input.
 *		Will return the center channel of an input 80MHz chspec.
 *		Will return INVCHANNEL if the chspec is malformed or less than 80MHz bw.
 */
extern uint8 wf_chspec_primary80_channel(chanspec_t chanspec);

/**
 * Returns the center channel of the secondary 80 MHz sub-band of the provided chanspec
 *
 * @param	chspec    input chanspec
 *
 * @return	center channel number of the secondary 80MHz sub-band of the input.
 *		Will return INVCHANNEL if the chspec is malformed or bw is not greater than 80MHz.
 */
extern uint8 wf_chspec_secondary80_channel(chanspec_t chanspec);

/**
 * Returns the chanspec for the primary 80MHz sub-band of an 160MHz or 80+80 channel
 *
 * @param	chspec    input chanspec
 *
 * @return	An 80MHz chanspec describing the primary 80MHz sub-band of the input.
 *		Will return an input 80MHz chspec as is.
 *		Will return INVCHANSPEC if the chspec is malformed or less than 80MHz bw.
 */
extern chanspec_t wf_chspec_primary80_chspec(chanspec_t chspec);

/**
 * Returns the chanspec for the secondary 80MHz sub-band of an 160MHz or 80+80 channel
 * The sideband in the chanspec is always set to WL_CHANSPEC_CTL_SB_LL since this sub-band
 * does not contain the primary 20MHz channel.
 *
 * @param	chspec    input chanspec
 *
 * @return	An 80MHz chanspec describing the secondary 80MHz sub-band of the input.
 *		Will return INVCHANSPEC if the chspec is malformed or bw is not greater than 80MHz.
 */
extern chanspec_t wf_chspec_secondary80_chspec(chanspec_t chspec);

/*
 * For 160MHz or 80P80 chanspec, set ch[0]/ch[1] to be the low/high 80 Mhz channels
 *
 * For 20/40/80MHz chanspec, set ch[0] to be the center freq, and chan[1]=-1
 */
extern void wf_chspec_get_80p80_channels(chanspec_t chspec, uint8 *ch);

#ifdef WL11AC_80P80
/*
 * This function returns the centre chanel for the given chanspec.
 * In case of 80+80 chanspec it returns the primary 80 Mhz centre channel
 */
extern uint8 wf_chspec_channel(chanspec_t chspec);
#endif // endif

/* Populates array with all 20MHz side bands of a given chanspec_t in the following order:
 *		primary20, ext20, two ext40s, four ext80s.
 *     'chspec' is the chanspec of interest
 *     'pext' must point to an uint8 array of long enough to hold all side bands of the given chspec
 *
 * Works with 20, 40, 80, 80p80 and 160MHz chspec
 */

extern void wf_get_all_ext(chanspec_t chspec, uint8 *chan_ptr);

/*
 * Given two chanspecs, returns true if they overlap.
 * (Overlap: At least one 20MHz subband is common between the two chanspecs provided)
 */
extern bool wf_chspec_overlap(chanspec_t chspec0, chanspec_t chspec1);

extern uint8 channel_bw_to_width(chanspec_t chspec);

/** returns the 20Mhz control chanspec for the caller supplied chspec */
extern chanspec_t wf_ctlchspec20_from_chspec(chanspec_t chspec);

#define CHAN_COULD_BE_5G(channel) \
	(((channel) >= CH_MIN_5G_CHANNEL) && ((channel) <= CH_MAX_5G_CHANNEL))

#endif	/* _bcmwifi_channels_h_ */
