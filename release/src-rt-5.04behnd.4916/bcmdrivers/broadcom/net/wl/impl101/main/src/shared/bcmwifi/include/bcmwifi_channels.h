/*
 * Misc utility routines for WL and Apps
 * This header file housing the define and function prototype use by
 * both the wl driver, tools & Apps.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: bcmwifi_channels.h 825315 2023-05-17 14:05:59Z $
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

/* One channel is 5MHz wide. These defines return the number of channels in a given bandwidth: */
#define CH_320MHZ_APART                  64u
#define CH_160MHZ_APART                  32u
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
#define CH_MAX_5G_SUB1_CHANNEL           64u    /* Max channel in first 5G subband */
#define CH_MAX_5G_SUB2_CHANNEL		144u	/* Max channel in second 5G subband (5150-5730) */
#define CH_MAC_5G_LOW_CHANNEL		CH_MAX_5G_SUB2_CHANNEL
#define CH_MAX_5G_SUB1_CHANNEL		 64u	/* Max channel in first 5G subband */
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
#define CH_MIN_6G_320M_CHANNEL           31u    /* Min 320MHz center channel in 6G band */
#define CH_MAX_6G_320M_CHANNEL          199u    /* Max 320MHz center channel in 6G band */

/** MAXCHANNEL is the maximum number of any channel that is used in any band */
#define MAXCHANNEL                      254u /* DO NOT MAKE > 255: channels are uint8's all over */

#if defined(WL_SCAN_MEASUREMENT) || defined(WL_SCAN_BEACON_DELAY)
#define MAXCHANNEL_SCAN_MEAS		MAXCHANNEL
#endif /* WL_SCAN_MEASUREMENT */
#define MAXCHANNEL_NUM	(MAXCHANNEL - 1)	/* max channel number */

/* length of channel vector bitmap is the MAXCHANNEL we want to handle rounded up to a byte */
#define CHANVEC_LEN ((MAXCHANNEL + (8 - 1)) / 8)

#define INVCHANNEL              255u     /* error value for a bad channel */

#ifndef TPE_PSD_COUNT
#define TPE_PSD_COUNT 16
#endif /* TPE_PSD_COUNT */

/** channel bitvec: bit[1] represents channel number 1, etc. */
typedef struct {
	uint8   vec[CHANVEC_LEN]; /**< bitvec of either 2G+5G or of 6G channels */
} chanvec_t;

/**
 * Chanvec_t cannot hold 2g/5g and 6g channels at the same time, since channel numbers overlap.
 * Therefore, with the advent of 6G, the following type was introduced into the code:
 */
typedef struct {
	chanvec_t vec2g5g; /**< bitvec of 2G and 5G channels */
	chanvec_t vec6g;   /**< bitvec of 6G channels */
} chanvec2g5g6g_t;

#define GET_CHANVEC(chvec2g5g6g, bu) \
	((bu == BAND_2G_INDEX || bu == BAND_5G_INDEX) ? \
		&((chvec2g5g6g)->vec2g5g) : &((chvec2g5g6g)->vec6g))

/* make sure channel num is within valid range */
#define CH_NUM_VALID_RANGE(ch_num) ((ch_num) > 0 && (ch_num) <= MAXCHANNEL_NUM)

#define CHSPEC_CTLOVLP(sp1, sp2, sep)	\
	((uint)ABS(wf_chspec_ctlchan(sp1) - wf_chspec_ctlchan(sp2)) < (uint)(sep))

#define CHSPEC_CHNLOVLP(sp1, sp2, sep) \
	((uint)ABS(CHSPEC_CHANNEL(sp1) - CHSPEC_CHANNEL(sp2)) <= (uint)(sep))

/* All builds use the new 11ac ratespec/chanspec */
#undef  D11AC_IOTYPES
#define D11AC_IOTYPES

/* For contiguous channel bandwidth <= 160Mhz */
#define WL_CHANSPEC_CHAN_MASK		0x00ffu
#define WL_CHANSPEC_CHAN_SHIFT		0u

/**
 * For contiguous channel bandwidth >= 320MHz.
 * For BW=320, bits had to be made available in chanspec_t to contain the sideband
 * indication. This has been achieved by coding the 'chan' bitfield in units of 160MHz.
 */
#define WL_CHANSPEC_GE320_CHIDX_MASK	0x0003fu /**< 6 bits long, [160mhz] units */
#define WL_CHANSPEC_GE320_CHIDX_SHIFT	0u

/* For discontiguous channel bandwidth */
#define WL_CHANSPEC_CHAN0_MASK		0x000fu
#define WL_CHANSPEC_CHAN0_SHIFT		0u
#define WL_CHANSPEC_CHAN1_MASK		0x00f0u
#define WL_CHANSPEC_CHAN1_SHIFT		4u

/* bw <= 160MHz channel sideband indication */
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

/* channel sideband indication for bandwidth >= 320MHz */
#define WL_CHANSPEC_GE320_SB_MASK	0x07C0u /* 5 bits long */
#define WL_CHANSPEC_GE320_SB_SHIFT	6u
#define WL_CHANSPEC_CTL_SB_LLLLL	(0x00u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLLLU	(0x01u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLLUL	(0x02u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLLUU	(0x03u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLULL	(0x04u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLULU	(0x05u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLUUL	(0x06u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LLUUU	(0x07u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LULLL	(0x08u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LULLU	(0x09u << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LULUL	(0x0Au << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LULUU	(0x0Bu << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LUULL	(0x0Cu << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LUULU	(0x0Du << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LUUUL	(0x0Eu << WL_CHANSPEC_GE320_SB_SHIFT)
#define WL_CHANSPEC_CTL_SB_LUUUU	(0x0Fu << WL_CHANSPEC_GE320_SB_SHIFT)

/* Bandwidth field */
#define WL_CHANSPEC_BW_MASK		0x3800u
#define WL_CHANSPEC_BW_SHIFT		11u
/* WL_CHANSPEC_BW_160160 is not supported */
#define WL_CHANSPEC_BW_20		0x1000u
#define WL_CHANSPEC_BW_40		0x1800u
#define WL_CHANSPEC_BW_80		0x2000u
#define WL_CHANSPEC_BW_160		0x2800u
#define WL_CHANSPEC_BW_320		0x3000u
#define WL_CHANSPEC_BW_8080		WL_CHANSPEC_BW_320 /* obsoleted */

/* Band field */
#define WL_CHANSPEC_BAND_MASK		0xc000u
#define WL_CHANSPEC_BAND_SHIFT		14u
#define WL_CHANSPEC_BAND_2G		0x0000u
#define WL_CHANSPEC_BAND_6G		0x4000u
#define WL_CHANSPEC_BAND_4G		0x8000u
#define WL_CHANSPEC_BAND_5G		0xc000u
#define INVCHANSPEC			255u
#define MAX_CHANSPEC			0xFFFFu

/* to indentify code that does not take 6GHz into account */
#define WL_CHANNEL_2G5G_BAND(ch) (((uint)(ch) <= CH_MAX_2G_CHANNEL) ? \
	WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G)
#define WL_CHANNEL_2G5G_BANDTYPE(ch) (((uint)(ch) <= CH_MAX_2G_CHANNEL) ? \
	WLC_BAND_2G : WLC_BAND_5G)

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

#define CH20MHZ_CHSPEC(channel, band)	(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
						(band))

#define NEXT_20MHZ_CHAN(channel)	(((channel) < (MAXCHANNEL - CH_20MHZ_APART)) ? \
						((channel) + CH_20MHZ_APART) : 0)

#define CH40MHZ_CHSPEC(band, center_ch, ctlsb) \
		(chanspec_t)((center_ch) | (ctlsb) | WL_CHANSPEC_BW_40 | (band))
#define CH80MHZ_CHSPEC(band, center_ch, ctlsb) \
		(chanspec_t)((center_ch) | (ctlsb) | WL_CHANSPEC_BW_80 | (band))
#define CH160MHZ_CHSPEC(band, center_ch, ctlsb) \
		(chanspec_t)((center_ch) | (ctlsb) | WL_CHANSPEC_BW_160 | (band))

#define CHBW_CHSPEC(bw, center_ch, band) (chanspec_t)((chanspec_t)(center_ch) | (bw) | (band))

#define CHSPEC_GE320_CHIDX(chspec) \
	(((chspec) & WL_CHANSPEC_GE320_CHIDX_MASK) >> WL_CHANSPEC_GE320_CHIDX_SHIFT)

#define CHSPEC_GE320_SB(chspec)	((chspec) & WL_CHANSPEC_GE320_SB_MASK)

/** returns the center channel, starting at 1 */
#define CHSPEC_LT320_CHANNEL(chspec) \
	((chspec) & WL_CHANSPEC_CHAN_MASK)
/** returns the center channel, starting at 31 */
#define CHSPEC_GE320_CHANNEL(chspec) \
	(CHSPEC_GE320_CHIDX(chspec) * CH_160MHZ_APART + CH_MIN_6G_320M_CHANNEL)

/** returns the center channel */
#define CHSPEC_CHANNEL(chspec)	((uint8)(CHSPEC_IS320(chspec) ? \
	CHSPEC_GE320_CHANNEL(chspec) : \
	CHSPEC_LT320_CHANNEL(chspec)))

#define CHSPEC_CHAN0(chspec)	((chspec) & WL_CHANSPEC_CHAN0_MASK) >> WL_CHANSPEC_CHAN0_SHIFT
#define CHSPEC_CHAN1(chspec)	((chspec) & WL_CHANSPEC_CHAN1_MASK) >> WL_CHANSPEC_CHAN1_SHIFT
#define CHSPEC_BAND(chspec)	((chspec) & WL_CHANSPEC_BAND_MASK)

#define CHSPEC_CTL_SB(chspec)	(CHSPEC_IS320(chspec) ? \
	((chspec) & WL_CHANSPEC_GE320_SB_MASK) : \
	((chspec) & WL_CHANSPEC_CTL_SB_MASK))

#define CHSPEC_SB(chspec)	(CHSPEC_IS320(chspec) ? \
	((chspec) & WL_CHANSPEC_GE320_SB_MASK) >> WL_CHANSPEC_GE320_SB_SHIFT: \
	((chspec) & WL_CHANSPEC_CTL_SB_MASK) >> WL_CHANSPEC_CTL_SB_SHIFT)

#define CHSPEC_BW(chspec)	((chspec) & WL_CHANSPEC_BW_MASK)

#define CHSPEC_IS20(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)
#define CHSPEC_IS20_5G(chspec)	((((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20) && \
								CHSPEC_IS5G(chspec))
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
#define CHSPEC_IS8080(chspec)	FALSE /* unsupported by BCA HW */
#endif

#ifndef CHSPEC_IS320
#if defined(WL11BE) || !defined(DONGLEBUILD)
#define CHSPEC_IS320(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_320)
#define CHSPEC_IS320_1(chspec)	(CHSPEC_IS320(chspec) && ((CHSPEC_GE320_CHIDX(chspec) & 0x1) == 0))
#define CHSPEC_IS320_2(chspec)	(CHSPEC_IS320(chspec) && ((CHSPEC_GE320_CHIDX(chspec) & 0x1) == 1))
#else
#define CHSPEC_IS320(chspec)	(FALSE)
#define CHSPEC_IS320_1(chspec)	(FALSE)
#define CHSPEC_IS320_2(chspec)	(FALSE)
#endif /* WL11BE || !DONGLEBUILD */
#endif /* CHSPEC_IS320 */

/* pass a center channel and get channel offset from it by 10MHz */
#define CH_OFF_10MHZ_MULTIPLES(center_ch, offset)				\
((uint8) (((offset) < 0) ?						\
	(((center_ch) > (WL_CHANSPEC_CHAN_MASK & ((uint16)((-(offset)) * CH_10MHZ_APART)))) ? \
		((center_ch) + (offset) * CH_10MHZ_APART) : 0) :		\
	((((uint16)(center_ch) + (uint16)(offset) * CH_10MHZ_APART) < (uint16)MAXCHANNEL) ? \
		((center_ch) + (offset) * CH_10MHZ_APART) : 0)))

#if defined(WL11AC_160)
/* pass a 320MHz center channel to get 20MHz subband channel numbers */
#define LLLLL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -15)
#define LLLLU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -13)
#define LLLUL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -11)
#define LLLUU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -9)
#define LLULL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -7)
#define LLULU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -5)
#define LLUUL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -3)
#define LLUUU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, -1)
#define LULLL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 1)
#define LULLU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 3)
#define LULUL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 5)
#define LULUU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 7)
#define LUULL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 9)
#define LUULU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 11)
#define LUUUL_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 13)
#define LUUUU_20_SB_320(center_ch) CH_OFF_10MHZ_MULTIPLES(center_ch, 15)

/* pass a 160MHz center channel to get 20MHz subband channel numbers */
#define LLL_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch, -7)
#define LLU_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch, -5)
#define LUL_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch, -3)
#define LUU_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch, -1)
#define ULL_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch,  1)
#define ULU_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch,  3)
#define UUL_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch,  5)
#define UUU_20_SB_160(center_ch)  CH_OFF_10MHZ_MULTIPLES(center_ch,  7)

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
 * (works with 20, 40, 80, 160, 320)
 */
#define CH_FIRST_20_SB(chspec)  ((uint8) (\
		CHSPEC_IS320(chspec) ? LLLLL_20_SB_320(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS160(chspec) ? LLL_20_SB_160(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS80(chspec) ? LL_20_SB(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS40(chspec) ? LOWER_20_SB(CHSPEC_CHANNEL(chspec)) : \
		CHSPEC_CHANNEL(chspec))))))

/* get upper most 20MHz sideband of a given chspec
 * (works with 20, 40, 80, 160, 320)
 */
#define CH_LAST_20_SB(chspec)  ((uint8) (\
		CHSPEC_IS320(chspec) ? LUUUU_20_SB_320(CHSPEC_CHANNEL(chspec)) : (\
		CHSPEC_IS160(chspec) ? UUU_20_SB_160(CHSPEC_CHANNEL(chspec)) : (\
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
 * (works with 20, 40, 80, 160)
 * resolves to 0 if called with upper most channel
 */
#define CH_NEXT_20_SB(chspec, channel)  ((uint8) (\
			((uint8) ((channel) + CH_20MHZ_APART) > CH_LAST_20_SB(chspec) ? 0 : \
				((channel) + CH_20MHZ_APART))))

#else /* WL11AC_160 */

#define LLL_20_SB_160(center_ch)  0
#define LLU_20_SB_160(center_ch)  0
#define LUL_20_SB_160(center_ch)  0
#define LUU_20_SB_160(center_ch)  0
#define ULL_20_SB_160(center_ch)  0
#define ULU_20_SB_160(center_ch)  0
#define UUL_20_SB_160(center_ch)  0
#define UUU_20_SB_160(center_ch)  0

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

#endif /* WL11AC_160 */

/* Iterator for 20MHz side bands of a chanspec: (chanspec_t chspec, uint8 channel)
 * 'chspec' chanspec_t of interest (used in loop, better to pass a resolved value than a macro)
 * 'channel' must be a variable (not an expression).
 */
#define FOREACH_20_SB(chspec, channel) \
		for (channel = CH_FIRST_20_SB(chspec); channel; \
			channel = CH_NEXT_20_SB((chspec), channel))

/* Below is the efficient itterator as replacement for FOREACH_20_SB which is being used by
 * some none wlc component. So new MACRO was necessary to be able to update it
 */
#define FOREACH_20_SB_EFF(chspec, channel, last_channel) \
		for (channel = CH_FIRST_20_SB(chspec), \
			last_channel = CH_LAST_20_SB(chspec); channel <= last_channel; \
			channel += CH_20MHZ_APART)

/* Uses iterator to populate array with all side bands involved (sorted lower to upper).
 *     'chspec' chanspec_t of interest
 *     'psb' pointer to uint8 array of enough size to hold all side bands for the given chspec
 */
#define GET_ALL_SB(chspec, psb) do { \
		uint8 channel, last_channel, idx = 0; \
		chanspec_t chspec_local = chspec; \
		FOREACH_20_SB_EFF(chspec_local, channel, last_channel) \
			(psb)[idx++] = channel; \
} while (0)

/* given a chanspec of any bw, tests if primary20 SB is in lower 20, 40, 80 respectively */
#define IS_CTL_IN_L20(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_U) /* CTL SB is in low 20 of any 40 */
#define IS_CTL_IN_L40(chspec) !((CHSPEC_IS320(chspec)? ((chspec) & (WL_CHANSPEC_CTL_SB_LLLUL)): \
			(chspec) & WL_CHANSPEC_CTL_SB_UL))	/* in low 40 of any 80 */
#define IS_CTL_IN_L80(chspec) !((chspec) & WL_CHANSPEC_CTL_SB_LLULL)	/* in low 80 of any 160 */

#define BW_LE20(bw)		((bw) == WL_CHANSPEC_BW_20)
#define CHSPEC_ISLE20(chspec)	(CHSPEC_IS20(chspec))

#define BW_LE40(bw)		(BW_LE20(bw) || ((bw) == WL_CHANSPEC_BW_40))
#define BW_LE80(bw)		(BW_LE40(bw) || ((bw) == WL_CHANSPEC_BW_80))
#define BW_LE160(bw)		(BW_LE80(bw) || ((bw) == WL_CHANSPEC_BW_160))
#define BW_LE320(bw)		(BW_LE160(bw) || ((bw) == WL_CHANSPEC_BW_320))
#define CHSPEC_BW_LE20(chspec)	(BW_LE20(CHSPEC_BW(chspec)))
#define CHSPEC_BW_LE40(chspec)	(BW_LE40(CHSPEC_BW(chspec)))
#define CHSPEC_BW_LE80(chspec)	(BW_LE80(CHSPEC_BW(chspec)))
#define CHSPEC_BW_LE160(chspec)	(BW_LE160(CHSPEC_BW(chspec)))
#define CHSPEC_BW_LE320(chspec)	(BW_LE320(CHSPEC_BW(chspec)))
#define CHSPEC_BW_GE40(chspec)	(!CHSPEC_BW_LE20(chspec))
#define CHSPEC_BW_GE80(chspec)	(!CHSPEC_BW_LE40(chspec))
#define CHSPEC_BW_GE160(chspec)	(!CHSPEC_BW_LE80(chspec))
#define CHSPEC_BW_GE320(chspec)	(!CHSPEC_BW_LE160(chspec))

#define CHSPEC_PRIM20_EQUAL(chspec1, chspec2) \
	(wf_chspec_primary20_chspec(chspec1) == wf_chspec_primary20_chspec(chspec2))

/**
 * Definition of Preferred Scan Channel (26.17.2.3.3 Non-AP STA scanning behavior)
 * set of 20 MHz channels in the 6 GHz band, with channel center frequency,
 * ch_a = (channel starting frequency + 5) + 5 * 16 * (n - 1), where n = 1, ..., 15
 */
#define WF_IS_6G_PSC_CHAN(channel) (((channel) % 16u) == 5u)
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

#define CHSPEC_IS_BW_160_WIDE(chspec) (CHSPEC_BW(chspec) == WL_CHANSPEC_BW_160)

#define CHSPEC_BW_GE(chspec, bw) (CHSPEC_BW(chspec) >= (bw))

#define CHSPEC_BW_LE(chspec, bw) (CHSPEC_BW(chspec) <= (bw))

#define CHSPEC_BW_GT(chspec, bw) (CHSPEC_BW(chspec) > (bw))

#define CHSPEC_BW_LT(chspec, bw) (CHSPEC_BW(chspec) < (bw))

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

#define CH20MHZ_LCHSPEC(center_ch) \
	(chanspec_t)((chanspec_t)(center_ch) | WL_LCHANSPEC_BW_20 | \
	WL_LCHANSPEC_CTL_SB_NONE | (((center_ch) <= CH_MAX_2G_CHANNEL) ? \
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
 */
#define WF_CHAN_FACTOR_6_G		11900u	/* 6   GHz band, 5950 MHz */

#define WLC_2G_25MHZ_OFFSET		5	/* 2.4GHz band channel offset */

/**
 *  No of sub-band vlaue of the specified Mhz chanspec
 */
#define WF_NUM_SIDEBANDS_40MHZ   2u
#define WF_NUM_SIDEBANDS_80MHZ   4u
#define WF_NUM_SIDEBANDS_8080MHZ 4u
#define WF_NUM_SIDEBANDS_160MHZ  8u
#define WF_NUM_SIDEBANDS_320MHZ  16u

/** e.g. useful when referencing per-bw array elements */
enum wlc_bw_e {
	BW20_IDX = 0,
	BW40_IDX,
	BW80_IDX,
	BW160_IDX,
	BW320_IDX,
	BW_N_ELS
};

/** @return 0:20MHz, 1:40MHz, 2:80MHz, ... */
#define CHSPEC2WLC_BW(chanspec) \
	((enum wlc_bw_e)((CHSPEC_BW(chanspec) - WL_CHANSPEC_BW_20) >> WL_CHANSPEC_BW_SHIFT))

/* return channel number of the low edge of the band
 * given the center channel and BW
 */
uint channel_low_edge(uint center_ch, chanspec_bw_t bw);

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
 * Verify that the center channel is a valid 320MHz center channel according to 802.11.
 */
bool wf_valid_320MHz_center_chan(uint center_channel, chanspec_band_t band);

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
 * Returns the chanspec for an 160MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 */
chanspec_t wf_create_160MHz_chspec(uint primary_channel, uint center_channel,
	chanspec_band_t band);

typedef enum {
	WF_SCHEME_AUTO	= 0,
	WF_SCHEME_320_1	= 1,
	WF_SCHEME_320_2	= 2
} wf_bw320_scheme_t;

/**
 * Returns the chanspec for an 320MHz channel given the primary 20MHz channel number,
 * the band scheme and the band. Value auto for scheme means the driver picks any band
 * that fits (in case both schemes have a match, driver will have preference for 320-1 over 320-2)
 */
chanspec_t wf_create_320MHz_chspec(uint primary_channel, wf_bw320_scheme_t scheme,
	chanspec_band_t band);

/* Returns the 320MHz scheme given the channel center frequency index of the 320MHz bandwidth */
wf_bw320_scheme_t wl_get_320MHz_scheme(uint8 center_channel);

/**
 * This function is going to be obsoleted.
 * Returns the chanspec for an 80+80MHz channel given the primary 20MHz channel number,
 * the center channel numbers for each frequency segment, and the band.
 */
chanspec_t wf_create_8080MHz_chspec(uint primary_channel, uint chan0, uint chan1,
                                    chanspec_band_t band);

/**
 * Returns the chanspec given the primary 20MHz channel number,
 * the center channel number, channel width, and the band.
 *
 * The channel width must be 20, 40, 80, 160 or 320 MHz. 80+80 MHz chanspec creation
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
 * Return 6G center channel given primary 20MHz channel, bandwidth and band scheme.
 * Scheme is relevant for 320MHz BW channels only and is ignored for BWs other than 320MHz.
 * Value auto for scheme means the driver picks any band that fits (in case both schemes have
 * a match, driver will have preference for 320-1 over 320-2)
 */
uint wf_6G_primary20_ch_to_center_ch(uint primary_channel, wf_bw320_scheme_t scheme,
	chanspec_bw_t bw);

/**
 * return primary20 channel given center channel and side band
 */
uint
wf_center_to_primary20_chan(uint center_ch, chanspec_bw_t bw, uint sb);

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
 * Returns the chanspec for the primary 160MHz sub-band of an 320MHz channel
 *
 * @param	chspec    input chanspec
 *
 * @return	An 160MHz chanspec describing the primary 160MHz sub-band of the input.
 *		Will return an input 160MHz chspec as is.
 *		Will return INVCHANSPEC if the chspec is malformed or less than 160MHz bw.
 */
chanspec_t wf_chspec_primary160_chspec(chanspec_t chspec);
/**
 * Returns the chanspec for the secondary 160MHz sub-band of an 320MHz channel
 *
 * @param	chspec    input chanspec
 *
 * @return	An 160MHz chanspec describing the secondary 160MHz sub-band of the input.
 *		Will return an input 160MHz chspec as is.
 *		Will return INVCHANSPEC if the chspec is malformed or less than 160MHz bw.
 */
chanspec_t wf_chspec_secondary160_chspec(chanspec_t chspec);

/**
 * Return the channel number for a given frequency and base frequency
 */
int wf_mhz2channel(uint freq, uint start_factor);

/**
 * Return the center frequency in MHz of the given channel and base frequency.
 */
int wf_channel2mhz(uint channel, uint start_factor);

/**
 * Return the center frequency in MHz of the given chanspec.
 */
int wf_chanspec2mhz(chanspec_t chanspec);

/**
 * Convert ctl chan and bw to chanspec
 *
 * @param	ctl_ch		channel
 * @param	bw	        bandwidth
 *
 * @return	> 0 if successful or 0 otherwise
 *
 */
extern chanspec_t wf_channel2chspec(uint ctl_ch, uint bw, uint wl_chanspec_band);

/*
 * This function is going to be obsoleted.
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
 * For 160MHz chanspec, set ch[0]/ch[1] to be the low/high 80 Mhz channels
 *
 * For 20/40/80MHz chanspec, set ch[0] to be the center freq, and chan[1]=1
 */
extern void wf_chspec_get_80p80_channels(chanspec_t chspec, uint8 *ch);

/**
 * Returns the center channel of the primary 160MHz sub-band of the provided chanspec
 *
 * @param	chspec    input chanspec
 *
 * @return	center channel number of the primary 160MHz sub-band of the input.
 *		Will return the center channel of an input 160MHz chspec.
 *		Will return INVCHANNEL if the chspec is malformed or less than 160MHz bw.
 */
extern uint8 wf_chspec_primary160_channel(chanspec_t chanspec);

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
