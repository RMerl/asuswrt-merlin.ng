/*
 * Misc utility routines used by kernel or app-level.
 * Contents are wifi-specific, used by any kernel or app-level
 * software that might want wifi things as it grows.
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
 * $Id: bcmwifi_channels.c 832513 2023-11-08 14:10:36Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>

#ifdef BCMDRIVER
#include <osl.h>
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#else
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif /* BCMDRIVER */

#ifndef ASSERT
#define ASSERT(exp)
#endif

#include <bcmwifi_channels.h>
#include <802.11.h>

/* Chanspec ASCII representation:
 * [<band> 'g'] <channel> ['/'<bandwidth> [<primary-sideband>]['/'<1st80channel>'-'<2nd80channel>]]
 *
 * <band>:
 *      (optional) 2, 3, 4, 5 for 2.4GHz, 3GHz, 4GHz, and 5GHz respectively.
 *      Default value is 2g if channel <= 14, otherwise 5g.
 * <channel>:
 *      channel number of the 5MHz, 10MHz, 20MHz channel,
 *      or primary channel of 40MHz, 80MHz, 160MHz or 320MHz channel.
 * <bandwidth>:
 *      (optional) 20, 40, 80, 160 or 320. Default value is 20.
 * <primary-sideband>:
 *      (only for 2.4GHz band 40MHz) U for upper sideband primary, L for lower.
 *
 *      For 2.4GHz band 40MHz channels, the same primary channel may be the
 *      upper sideband for one 40MHz channel, and the lower sideband for an
 *      overlapping 40MHz channel.  The U/L disambiguates which 40MHz channel
 *      is being specified.
 *
 *      For 40MHz in the 5GHz band and all channel bandwidths greater than
 *      40MHz, the U/L specificaion is not allowed since the channels are
 *      non-overlapping and the primary sub-band is derived from its
 *      position in the wide bandwidth channel.
 *
 * In its simplest form, it is a 20MHz channel number, with the implied band
 * of 2.4GHz if channel number <= 14, and 5GHz otherwise.
 *
 * To allow for backward compatibility with scripts, the old form for
 * 40MHz channels is also allowed: <channel><primary-sideband>
 *
 * <channel>:
 *	primary channel of 40MHz, channel <= 14 is 2GHz, otherwise 5GHz
 * <primary-sideband>:
 *	"U" for upper, "L" for lower (or lower case "u" "l")
 *
 * 6 GHz Examples:
 *      Chanspec        BW        Center Ch  Channel Range  Primary Ch
 *      6g1             20MHz     1          1              1
 *      6g1/20          20MHz     1          1              1
 *      6g1/40          40MHz     3          1-5            1
 *      6g5/40          40MHz     3          1-5            5
 *      6g1/80          80MHz     7          1-13           1
 *      6g5/80          80MHz     7          1-13           5
 *      6g9/80          80MHz     7          1-13           9
 *      6g13/80         80MHz     7          1-13           13
 *      6g1/160         160MHz    15         1-29           1
 *      6g5/160         160MHz    15         1-29           5
 *      6g9/160         160MHz    15         1-29           9
 *      6g13/160        160MHz    15         1-29           13
 *      6g17/160        160MHz    15         1-29           17
 *      6g21/160        160MHz    15         1-29           21
 *      6g25/160        160MHz    15         1-29           25
 *      6g29/160        160MHz    15         1-29           29
 *
 * 5 GHz Examples:
 *      Chanspec        BW        Center Ch  Channel Range  Primary Ch
 *      5g52            20MHz     52         -              -
 *      52              20MHz     52         -              -
 *      52/40           40MHz     54         52-56          52
 *      56/40           40MHz     54         52-56          56
 *      52/80           80MHz     58         52-64          52
 *      56/80           80MHz     58         52-64          56
 *      60/80           80MHz     58         52-64          60
 *      64/80           80MHz     58         52-64          64
 *      52/160          160MHz    50         36-64          52
 *      36/160          160MGz    50         36-64          36
 *
 * 2 GHz Examples:
 *      Chanspec        BW        Center Ch  Channel Range  Primary Ch
 *      2g8             20MHz     8          -              -
 *      8               20MHz     8          -              -
 *      6               20MHz     6          -              -
 *      6/40l           40MHz     8          6-10           6
 *      6l              40MHz     8          6-10           6
 *      6/40u           40MHz     4          2-6            6
 *      6u              40MHz     4          2-6            6
 */

/* bandwidth ASCII string */
static const char *wf_chspec_bw_str[] =
{
	"0",
	"0",
	"20",
	"40",
	"80",
	"160",
	"320",
	"?"
};

/** bandwidth in [500KHz] units */
static const uint16 wf_chspec_bw_half_mhz[] = {
	0,	/* unused */
	0,	/* unused */
	40,	/*  20MHz (WL_CHANSPEC_BW_20) */
	80,	/*  40MHz (WL_CHANSPEC_BW_40) */
	160,	/*  80MHz (WL_CHANSPEC_BW_80) */
	320,	/* 160MHz (WL_CHANSPEC_BW_160) */
	640,	/* 320MHz (WL_CHANSPEC_BW_320) */
	0	/* reserved */
};

#define WF_NUM_BW_HALF_MHZ	ARRAYSIZE(wf_chspec_bw_half_mhz)

/* 40MHz center channels in 5GHz band */
static const uint8 wf_5g_40m_chans[] =
{38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159, 167, 175};
#define WF_NUM_5G_40M_CHANS \
	(sizeof(wf_5g_40m_chans)/sizeof(uint8))

/* 80MHz center channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] =
{42, 58, 106, 122, 138, 155, 171};
#define WF_NUM_5G_80M_CHANS \
	(sizeof(wf_5g_80m_chans)/sizeof(uint8))

/* 160MHz center channels in 5GHz band */
static const uint8 wf_5g_160m_chans[] =
{50, 114, 163};
#define WF_NUM_5G_160M_CHANS \
	(sizeof(wf_5g_160m_chans)/sizeof(uint8))

/** 40MHz center channels in 6GHz band */
static const uint8 wf_6g_40m_chans[] =
{3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131, 139, 147, 155, 163, 171,
 179, 187, 195, 203, 211, 219, 227};
#define WF_NUM_6G_40M_CHANS ARRAYSIZE(wf_6g_40m_chans)

/** 80MHz center channels in 6GHz band */
static const uint8 wf_6g_80m_chans[] =
{7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215};

#define WF_NUM_6G_80M_CHANS ARRAYSIZE(wf_6g_80m_chans)

/** 160MHz center channels in 6GHz band */
static const uint8 wf_6g_160m_chans[] =
{15, 47, 79, 111, 143, 175, 207};
#define WF_NUM_6G_160M_CHANS ARRAYSIZE(wf_6g_160m_chans)

/** 320MHz center channels in 6GHz band */
static const uint8 wf_6g_320m_chans[] =
{31, 63, 95, 127, 159, 191};
#define WF_NUM_6G_320M_CHANS ARRAYSIZE(wf_6g_320m_chans)

#if defined(WL11BE) || !defined(DONGLEBUILD)
/* Define the conditional macro to help with reducing the code size bloat
 * in other branches and in trunk targets that don't need 11BE features...
 */
#define WFC_2VALS_EQ(var, val)	((var) == (val))
/* compare bandwidth unconditionally for 11be related stuff */
#define WFC_BW_EQ(bw, val)	WFC_2VALS_EQ(bw, val)
#else
#define WFC_BW_EQ(bw, val)	(FALSE)
#endif

/* Get bandwidth of chanspec in half MHz;
 * works with 2.5MHz to 320MHz chanspecs
 *
 * @param	chspec		chanspec_t format
 *
 * @return	bandwidth of a chanspec in half MHz units
 */
uint16
wf_bw_chspec_to_half_mhz(chanspec_t chspec)
{
	uint16 bw;

	bw = (chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT;
	return (bw >= WF_NUM_BW_HALF_MHZ ? 0 : wf_chspec_bw_half_mhz[bw]);
}

/* bw in MHz, return the channel count from the center channel to the
 * the channel at the edge of the band
 */
static uint
center_chan_to_edge(chanspec_bw_t bw)
{
	uint delta = 0;

	/* edge channels separated by BW - 10MHz on each side
	 * delta from cf to edge is half of that,
	 */
	if (bw == WL_CHANSPEC_BW_40) {
		/* 10 MHz */
		delta = 2;
	} else if (bw == WL_CHANSPEC_BW_80) {
		/* 30 MHz */
		delta = 6;
	} else if (bw == WL_CHANSPEC_BW_160) {
		/* 70 MHz */
		delta = 14;
	} else if (WFC_BW_EQ(bw, WL_CHANSPEC_BW_320)) {
		/* 150 MHz */
		delta = 30;
	}

	return delta;
}

/* return channel number of the low edge of the band
 * given the center channel and BW
 */
uint
channel_low_edge(uint center_ch, chanspec_bw_t bw)
{
	return (center_ch - center_chan_to_edge(bw));
}

/* return side band number given center channel and primary20 channel
 * return -1 on error
 */
static int
channel_to_sb(uint center_ch, uint primary_ch, chanspec_bw_t bw)
{
	uint lowest = channel_low_edge(center_ch, bw);
	uint sb;

	if (primary_ch < lowest ||
	    (primary_ch - lowest) % CH_20MHZ_APART) {
		/* bad primary channel lower than the low edge of the channel,
		 * or not mult 4.
		 */
		return -1;
	}

	sb = ((primary_ch - lowest) / CH_20MHZ_APART);
	/* sb must be a index to a 20MHz channel in range */
	if ((bw == WL_CHANSPEC_BW_20 && sb >= 1) ||
	    (bw == WL_CHANSPEC_BW_40 && sb >= 2) ||
	    (bw == WL_CHANSPEC_BW_80 && sb >= 4) ||
	    (bw == WL_CHANSPEC_BW_160 && sb >= 8) ||
	    (WFC_BW_EQ(bw, WL_CHANSPEC_BW_320) && sb >= 16)) {
		/* primary_ch must have been too high for the center_ch */
		return -1;
	}

	return sb;
}

/* return primary20 channel given center channel and side band */
uint
wf_center_to_primary20_chan(uint center_ch, chanspec_bw_t bw, uint sb)
{
	return (channel_low_edge(center_ch, bw) + sb * CH_20MHZ_APART);
}

/* return index of 80MHz channel from channel number
 * return -1 on error
 */
static int
channel_80mhz_to_id(uint ch)
{
	uint i;
	for (i = 0; i < WF_NUM_5G_80M_CHANS; i ++) {
		if (ch == wf_5g_80m_chans[i])
			return i;
	}

	return -1;
}

/* return index of the 6G 80MHz channel from channel number
 * return -1 on error
 */
static int
channel_6g_80mhz_to_id(uint ch)
{
	/* The 6GHz center channels start at 7, and have a spacing of 16 */
	if (ch >= CH_MIN_6G_80M_CHANNEL &&
	    ch <= CH_MAX_6G_80M_CHANNEL &&
	    ((ch - CH_MIN_6G_80M_CHANNEL) % 16) == 0) {  // even multiple of 16
		return (ch - CH_MIN_6G_80M_CHANNEL) / 16;
	}

	return -1;
}

/**
 * Converts a channel number into a 'channel index' that can be used inside of a 6G BW=320MHz
 * chanspec.
 *
 * @return   -1 on error
 */
static int
channel_6g_320mhz_to_id(uint ch)
{
	/* The 6GHz center channels start at 31, and have a spacing of 64 channels */
	if (ch >= CH_MIN_6G_320M_CHANNEL &&
	    ch <= CH_MAX_6G_320M_CHANNEL &&
	    ((ch - CH_MIN_6G_320M_CHANNEL) % CH_160MHZ_APART) == 0) {
		return (ch - CH_MIN_6G_320M_CHANNEL) / CH_160MHZ_APART;
	}

	return -1;
}

/**
 * Converts the 'channel index' inside of a 6G BW=320MHz chanspec into a channel number.
 *
 * @param    chan_320MHz_id    320MHz chanspec ID
 *
 * @return   Return the center channel number, or INVCHANNEL on error.
 *
 */
static uint8
wf_chspec_6G_id320_to_ch(uint8 chan_320MHz_id)
{
	uint8 ch = INVCHANNEL;

	if (chan_320MHz_id < WF_NUM_6G_320M_CHANS) {
		/* The 6GHz center channels have a spacing of 32
		 * starting from the first 320MHz center
		 */
		ch = CH_MIN_6G_320M_CHANNEL + (chan_320MHz_id * CH_160MHZ_APART);
	}

	return ch;
}

/**
 * Retrieve the chan_id and convert it to center channel.
 * To make room for additional bits, the chanspec definition for >=320MHz contains a channel index
 * instead of a channel number.
 *
 * @return on error: INVCHANNEL, otherwise a channel number.
 */
static uint8
wf_chspec_320_id2cch(chanspec_t chanspec)
{
	if (CHSPEC_BAND(chanspec) == WL_CHANSPEC_BAND_6G &&
	    CHSPEC_BW(chanspec) == WL_CHANSPEC_BW_320) {
		uint8 ch_id = CHSPEC_GE320_CHIDX(chanspec);

		return wf_chspec_6G_id320_to_ch(ch_id);
	}

	return INVCHANNEL;
}

/**
 * Convert chanspec to ascii string, or formats hex of an invalid chanspec.
 *
 * @param	chspec   chanspec to format
 * @param	buf      pointer to buf with room for at least CHANSPEC_STR_LEN bytes
 *
 * @return      Returns pointer to passed in buf. The buffer will have the ascii
 *              representation of the given chspec, or "invalid 0xHHHH" where
 *              0xHHHH is the hex representation of the invalid chanspec.
 *
 * @see		CHANSPEC_STR_LEN
 *
 * Wrapper function for wf_chspec_ntoa. In case of an error it puts
 * the original chanspec in the output buffer, prepended with "invalid".
 * Can be directly used in print routines as it takes care of null
 */
char *
wf_chspec_ntoa_ex(chanspec_t chspec, char *buf)
{
	if (wf_chspec_ntoa(chspec, buf) == NULL)
		snprintf(buf, CHANSPEC_STR_LEN, "invalid 0x%04x", chspec);
	return buf;
}

/**
 * Convert chanspec to ascii string, or return NULL on error.
 *
 * @param	chspec   chanspec to format
 * @param	buf      pointer to buf with room for at least CHANSPEC_STR_LEN bytes
 *
 * @return      Returns pointer to passed in buf or NULL on error. On sucess, the buffer
 *              will have the ascii representation of the given chspec.
 *
 * @see		CHANSPEC_STR_LEN
 *
 * Given a chanspec and a string buffer, format the chanspec as a
 * string, and return the original pointer buf.
 * Min buffer length must be CHANSPEC_STR_LEN.
 * On error return NULL.
 */
char *
wf_chspec_ntoa(chanspec_t chspec, char *buf)
{
	const char *band;
	uint pri_chan;

	if (wf_chspec_malformed(chspec))
		return NULL;

	band = "";

	/* check for non-default band spec */
	if (CHSPEC_IS2G(chspec) && CHSPEC_CHANNEL(chspec) > CH_MAX_2G_CHANNEL) {
		band = "2g";
	} else if (CHSPEC_IS5G(chspec) && CHSPEC_CHANNEL(chspec) <= CH_MAX_2G_CHANNEL) {
		band = "5g";
	} else if (CHSPEC_IS6G(chspec)) {
		band = "6g";
	}

	/* primary20 channel */
	pri_chan = wf_chspec_primary20_chan(chspec);

	/* bandwidth and primary20 sideband */
	if (CHSPEC_IS20(chspec)) {
		snprintf(buf, CHANSPEC_STR_LEN, "%s%d", band, pri_chan);
	} else if (CHSPEC_IS320(chspec)) {
		/* 320 */
		const char *bw;

		bw = wf_chspec_to_bw_str(chspec);

		snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s%s", band, pri_chan, bw,
			CHSPEC_IS320_1(chspec) ? "-1" : "-2");
	} else {
		const char *bw;
		const char *sb = "";

		bw = wf_chspec_to_bw_str(chspec);

#ifdef CHANSPEC_NEW_40MHZ_FORMAT
		/* primary20 sideband string if needed for 2g 40MHz */
		if (CHSPEC_IS40(chspec) && CHSPEC_IS2G(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
		}

		snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s%s", band, pri_chan, bw, sb);
#else
		/* primary20 sideband string instead of BW for 40MHz */
		if (CHSPEC_IS40(chspec) && !CHSPEC_IS6G(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d%s", band, pri_chan, sb);
		} else {
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s", band, pri_chan, bw);
		}
#endif /* CHANSPEC_NEW_40MHZ_FORMAT */
	}

	return (buf);
}

static int
read_uint(const char **p, unsigned int *num)
{
	unsigned long val;
	char *endp = NULL;

	val = strtoul(*p, &endp, 10);
	/* if endp is the initial pointer value, then a number was not read */
	if (endp == *p)
		return 0;

	/* advance the buffer pointer to the end of the integer string */
	*p = endp;
	/* return the parsed integer */
	*num = (unsigned int)val;

	return 1;
}

/**
 * Convert ascii string to chanspec
 *
 * @param	a     pointer to input string
 *
 * @return	Return > 0 if successful or 0 otherwise
 */
chanspec_t
wf_chspec_aton(const char *a)
{
	chanspec_t chspec;
	chanspec_band_t chspec_band;
	chanspec_subband_t chspec_sb;
	chanspec_bw_t chspec_bw;
	uint bw;
	uint num, pri_ch;
	char c, sb_ul = '\0';

	bw = 20;
	chspec_sb = 0;

	/* parse channel num or band */
	if (!read_uint(&a, &num))
		return 0;

	/* if we are looking at a 'g', then the first number was a band */
	c = bcm_tolower((int)a[0]);
	if (c == 'g') {
		a++; /* consume the char */

		/* band must be "2", "5", or "6" */
		if (num == 2)
			chspec_band = WL_CHANSPEC_BAND_2G;
		else if (num == 5)
			chspec_band = WL_CHANSPEC_BAND_5G;
		else if (num == 6)
			chspec_band = WL_CHANSPEC_BAND_6G;
		else
			return 0;

		/* read the channel number */
		if (!read_uint(&a, &pri_ch))
			return 0;

		c = bcm_tolower((int)a[0]);
	} else {
		/* first number is channel, use default for band */
		pri_ch = num;
		chspec_band = WL_CHANNEL_2G5G_BAND(pri_ch);
	}

	if (c == '\0') {
		/* default BW of 20MHz */
		chspec_bw = WL_CHANSPEC_BW_20;
		goto done_read;
	}

	a ++; /* consume the 'u','l', or '/' */

	/* check 'u'/'l' */
	if (c == 'u' || c == 'l') {
		sb_ul = c;
		chspec_bw = WL_CHANSPEC_BW_40;
		goto done_read;
	}

	/* next letter must be '/' */
	if (c != '/')
		return 0;

	/* read bandwidth */
	if (!read_uint(&a, &bw))
		return 0;

	/* convert to chspec value */
	if (bw == 20) {
		chspec_bw = WL_CHANSPEC_BW_20;
	} else if (bw == 40) {
		chspec_bw = WL_CHANSPEC_BW_40;
	} else if (bw == 80) {
		chspec_bw = WL_CHANSPEC_BW_80;
	} else if (bw == 160) {
		chspec_bw = WL_CHANSPEC_BW_160;
	} else if (WFC_BW_EQ(bw, 320)) {
		chspec_bw = WL_CHANSPEC_BW_320;
	} else {
		return 0;
	}

	/* So far we have <band>g<chan>/<bw>
	 * Can now be followed by u/l if bw = 40
	 */
	c = bcm_tolower((int)a[0]);

	/* if we have a 2g/40 channel, we should have a l/u spec now */
	if (chspec_band == WL_CHANSPEC_BAND_2G && bw == 40) {
		if (c == 'u' || c == 'l') {
			a ++; /* consume the u/l char */
			sb_ul = c;
			goto done_read;
		}
	}
	/* if we have a 6g/320 channel, we could have a -1/-2 spec now */
	if (chspec_band == WL_CHANSPEC_BAND_6G && bw == 320) {
		if (c == '-') {
			a ++; /* consume the '-' char */
			c = bcm_tolower((int)a[0]);
			if (c == '1' || c == '2') {
				a ++; /* consume the 1/2 char */
				sb_ul = c; /* reuse sb_ul */
				goto done_read;
			}
		} else if (c == 'o') {
			/* 'silent' util option for compatibility with mobility util */
			a ++; /* consume the 'o' char */
			sb_ul = c;
			goto done_read;
		}
	}

done_read:
	/* skip trailing white space */
	while (a[0] == ' ') {
		a ++;
	}

	/* must be end of string */
	if (a[0] != '\0')
		return 0;

	/* Now have all the chanspec string parts read;
	 * chspec_band, pri_ch, chspec_bw, sb_ul.
	 * chspec_band and chspec_bw are chanspec values.
	 * Need to convert pri_ch, and sb_ul into
	 * a center channel and sideband.
	 */

	/* if a sb u/l string was given, use it */
	if (sb_ul != '\0') {
		if ((sb_ul == 'l') || (sb_ul == 'u')) {
			if (sb_ul == 'l') {
				chspec_sb = WL_CHANSPEC_CTL_SB_LLL;
			} else if (sb_ul == 'u') {
				chspec_sb = WL_CHANSPEC_CTL_SB_LLU;
			}
			chspec = wf_create_40MHz_chspec_primary_sb(pri_ch, chspec_sb, chspec_band);
		} else if (chspec_band == WL_CHANSPEC_BAND_6G) {
			chspec = wf_create_320MHz_chspec(pri_ch,
				(sb_ul == '1') ? WF_SCHEME_320_1 :
				(sb_ul == '2' || sb_ul == 'o') ? WF_SCHEME_320_2 :
				WF_SCHEME_AUTO, chspec_band);
		} else {
			chspec = INVCHANSPEC;
		}
	} else if (chspec_bw == WL_CHANSPEC_BW_20) {
		/* if the bw is 20, only need the primary channel and band */
		chspec = wf_create_20MHz_chspec(pri_ch, chspec_band);
	} else if (chspec_bw == WL_CHANSPEC_BW_320) {
		/* no band scheme given, so default to lower possibility */
		chspec = wf_create_320MHz_chspec(pri_ch, WF_SCHEME_AUTO, chspec_band);
	} else {
		/* If the bw is 40/80/160/320 (and not 40MHz 2G), the channels are
		 * non-overlapping in 5G or 6G bands. Each primary channel is contained
		 * in only one higher bandwidth channel. The wf_create_chspec_from_primary()
		 * will create the chanspec. 2G 40MHz is handled just above, assuming a {u,l}
		 * sub-band spec was given.
		 */
		chspec = wf_create_chspec_from_primary(pri_ch, chspec_bw, chspec_band);
	}

	if (wf_chspec_malformed(chspec))
		return 0;

	return chspec;
}

/**
 * Verify the chanspec is using a legal set of parameters, i.e. that the
 * chanspec specified a band, bw, pri_sb and channel and that the
 * combination could be legal given any set of circumstances.
 *
 * @param  chanspec   the chanspec to check
 *
 * @return Returns TRUE if the chanspec is malformed, FALSE if it looks good.
 */
bool
wf_chspec_malformed(chanspec_t chanspec)
{
	uint chspec_bw = CHSPEC_BW(chanspec);
	uint chspec_sb = CHSPEC_CTL_SB(chanspec);

	/* INVCHANNEL value is never allowed in any proper chanspec */
	if (CHSPEC_CHANNEL(chanspec) == INVCHANNEL) {
		return TRUE;
	}

	if (CHSPEC_IS2G(chanspec)) {
		/* must be valid bandwidth for 2G */
#if defined(WL_AIR_IQ)
		/* AirIQ uses 2G chanspecs in 80MHz. Make exception */
		if (CHSPEC_IS2G(chanspec) && CHSPEC_IS80(chanspec)) {
			return FALSE;
		}
#endif /* WL_AIR_IQ */
		if (!BW_LE40(chspec_bw)) {
			return TRUE;
		}
	} else if (CHSPEC_IS5G(chanspec) || CHSPEC_IS6G(chanspec)) {
		if (WFC_BW_EQ(chspec_bw, WL_CHANSPEC_BW_320)) {
			uint ch_id;

			ch_id = CHSPEC_GE320_CHIDX(chanspec);

			/* channel IDs in 320 must be in range */
			if (CHSPEC_IS6G(chanspec)) {
				if (ch_id >= WF_NUM_6G_320M_CHANS) {
					/* bad 320MHz channel ID for the band */
					return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (chspec_bw == WL_CHANSPEC_BW_20 || chspec_bw == WL_CHANSPEC_BW_40 ||
		           chspec_bw == WL_CHANSPEC_BW_80 || chspec_bw == WL_CHANSPEC_BW_160) {
			/* check for invalid channel number */
			if (CHSPEC_CHANNEL(chanspec) == INVCHANNEL) {
				return TRUE;
			}
		} else {
			/* invalid bandwidth */
			return TRUE;
		}
	} else {
		/* invalid band */
		return TRUE;
	}

	/* retrieve sideband */
	if (WFC_BW_EQ(chspec_bw, WL_CHANSPEC_BW_320)) {
		chspec_sb = CHSPEC_GE320_SB(chanspec);
	} else {
		chspec_sb = CHSPEC_CTL_SB(chanspec);
	}

	/* side band needs to be consistent with bandwidth */
	if (chspec_bw == WL_CHANSPEC_BW_20) {
		if (chspec_sb != WL_CHANSPEC_CTL_SB_LLL) {
			return TRUE;
		}
	} else if (chspec_bw == WL_CHANSPEC_BW_40) {
		if (chspec_sb > WL_CHANSPEC_CTL_SB_LLU) {
			return TRUE;
		}
	} else if (chspec_bw == WL_CHANSPEC_BW_80) {
		if (chspec_sb > WL_CHANSPEC_CTL_SB_LUU) {
			return TRUE;
		}
	} else if (chspec_bw == WL_CHANSPEC_BW_160) {
		ASSERT(chspec_sb <= WL_CHANSPEC_CTL_SB_UUU);
	} else if (WFC_BW_EQ(chspec_bw, WL_CHANSPEC_BW_320)) {
		if (chspec_sb > WL_CHANSPEC_CTL_SB_LUUUU) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Verify the chanspec specifies a valid channel according to 802.11.
 *
 * @param   chanspec     the chanspec to check
 *
 * @return  Returns TRUE if the chanspec is a valid 802.11 channel
 */
bool
wf_chspec_valid(chanspec_t chanspec)
{
	chanspec_band_t chspec_band = CHSPEC_BAND(chanspec);
	chanspec_bw_t chspec_bw = CHSPEC_BW(chanspec);
	uint chspec_ch = -1;

	if (wf_chspec_malformed(chanspec)) {
		return FALSE;
	}

	if (WFC_BW_EQ(chspec_bw, WL_CHANSPEC_BW_320)) {
		if (CHSPEC_IS6G(chanspec)) {
			chspec_ch = wf_chspec_6G_id320_to_ch(CHSPEC_GE320_CHIDX(chanspec));
		} else {
			return FALSE;
		}
	} else {
		chspec_ch = CHSPEC_CHANNEL(chanspec);
	}

	if (chspec_ch == INVCHANNEL) {
		return FALSE;
	}

	/* After the malformed check, we know that we have
	 * a valid band field,
	 * a valid bandwidth for the band,
	 * and a valid sub-band value for the bandwidth.
	 *
	 * Since all sub-band specs are valid for any channel, the only thing remaining to
	 * check is that
	 *   the 20MHz channel,
	 *   or the center channel for higher BW,
	 * are valid for the specified band.
	 */

	if (chspec_bw == WL_CHANSPEC_BW_20) {

		return wf_valid_20MHz_chan(chspec_ch, chspec_band);

	} else if (chspec_bw == WL_CHANSPEC_BW_40) {

		return wf_valid_40MHz_center_chan(chspec_ch, chspec_band);

	} else if (chspec_bw == WL_CHANSPEC_BW_80) {

		return wf_valid_80MHz_center_chan(chspec_ch, chspec_band);

	} else if (chspec_bw == WL_CHANSPEC_BW_160) {

		return wf_valid_160MHz_center_chan(chspec_ch, chspec_band);

	} else if (WFC_BW_EQ(chspec_bw, WL_CHANSPEC_BW_320)) {
		return wf_valid_320MHz_center_chan(chspec_ch, chspec_band);
	}

	return FALSE;
}

/**
 * Verify that the channel is a valid 20MHz channel according to 802.11.
 *
 * @param  channel   20MHz channel number to validate
 * @param  band      chanspec band
 *
 * @return Return TRUE if valid
 */
bool
wf_valid_20MHz_chan(uint channel, chanspec_band_t band)
{
	if (band == WL_CHANSPEC_BAND_2G) {
		/* simple range check for 2GHz */
		return (channel >= CH_MIN_2G_CHANNEL &&
		        channel <= CH_MAX_2G_CHANNEL);
	} else if (band == WL_CHANSPEC_BAND_5G) {
		const uint8 *center_ch = wf_5g_40m_chans;
		uint num_ch = WF_NUM_5G_40M_CHANS;
		uint i;

		/* We don't have an array of legal 20MHz 5G channels, but they are
		 * each side of the legal 40MHz channels.  Check the chanspec
		 * channel against either side of the 40MHz channels.
		 */
		for (i = 0; i < num_ch; i ++) {
			if (channel == (uint)LOWER_20_SB(center_ch[i]) ||
			    channel == (uint)UPPER_20_SB(center_ch[i])) {
				break; /* match found */
			}
		}

		if (i == num_ch) {
			/* check for channel 173 which is not the side band
			 * of 40MHz 5G channel
			 */
			if (channel == 173) {
				i = 0;
			}

			/* check for legacy JP channels on failure */
			if (channel == 34 || channel == 38 ||
			    channel == 42 || channel == 46) {
				i = 0;
			}
		}

		if (i < num_ch) {
			/* match found */
			return TRUE;
		}
	}

	else if (band == WL_CHANSPEC_BAND_6G) {
		/* Use the simple pattern of 6GHz 20MHz channels */
		if ((channel >= CH_MIN_6G_CHANNEL &&
		     channel <= CH_MAX_6G_CHANNEL) &&
		    (((channel - CH_MIN_6G_CHANNEL) % CH_20MHZ_APART) == 0 ||
			channel == 2)) {  // even multiple of 4 or 6g2/20MHz
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Verify that the center channel is a valid 40MHz center channel according to 802.11.
 *
 * @param  center_channel   40MHz center channel to validate
 * @param  band             chanspec band
 *
 * @return Return TRUE if valid
 */
bool
wf_valid_40MHz_center_chan(uint center_channel, chanspec_band_t band)
{
	if (band == WL_CHANSPEC_BAND_2G) {
		/* simple range check for 2GHz */
		return (center_channel >= CH_MIN_2G_40M_CHANNEL &&
		        center_channel <= CH_MAX_2G_40M_CHANNEL);
	} else if (band == WL_CHANSPEC_BAND_5G) {
		uint i;

		/* use the 5GHz lookup of 40MHz channels */
		for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
			if (center_channel == wf_5g_40m_chans[i]) {
				return TRUE;
			}
		}
	}
	else if (band == WL_CHANSPEC_BAND_6G) {
		/* Use the simple pattern of 6GHz center channels */
		if ((center_channel >= CH_MIN_6G_40M_CHANNEL &&
		     center_channel <= CH_MAX_6G_40M_CHANNEL) &&
		    ((center_channel - CH_MIN_6G_40M_CHANNEL) % 8) == 0) {  // even multiple of 8
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Verify that the center channel is a valid 80MHz center channel according to 802.11.
 *
 * @param  center_channel   80MHz center channel to validate
 * @param  band             chanspec band
 *
 * @return Return TRUE if valid
 */
bool
wf_valid_80MHz_center_chan(uint center_channel, chanspec_band_t band)
{
	if (band == WL_CHANSPEC_BAND_2G) {
		return FALSE;
	} else if (band == WL_CHANSPEC_BAND_5G) {
		/* use the 80MHz ID lookup to validate the center channel */
		if (channel_80mhz_to_id(center_channel) >= 0) {
			return TRUE;
		}
	} else if (band == WL_CHANSPEC_BAND_6G) {
		/* use the 80MHz ID lookup to validate the center channel */
		if (channel_6g_80mhz_to_id(center_channel) >= 0) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Verify that the center channel is a valid 160MHz center channel according to 802.11.
 *
 * @param  center_channel   160MHz center channel to validate
 * @param  band             chanspec band
 *
 * @return Return TRUE if valid
 */
bool
wf_valid_160MHz_center_chan(uint center_channel, chanspec_band_t band)
{
	if (band == WL_CHANSPEC_BAND_2G) {
		return FALSE;
	} else if (band == WL_CHANSPEC_BAND_5G) {
		uint i;

		/* use the 5GHz lookup of 40MHz channels */
		for (i = 0; i < WF_NUM_5G_160M_CHANS; i++) {
			if (center_channel == wf_5g_160m_chans[i]) {
				return TRUE;
			}
		}
	}
	else if (band == WL_CHANSPEC_BAND_6G) {
		/* Use the simple pattern of 6GHz center channels */
		if ((center_channel >= CH_MIN_6G_160M_CHANNEL &&
		     center_channel <= CH_MAX_6G_160M_CHANNEL) &&
		    ((center_channel - CH_MIN_6G_160M_CHANNEL) % CH_160MHZ_APART) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Verify that the center channel is a valid 320MHz center channel according to 802.11.
 *
 * @param  center_channel   320MHz center channel to validate
 * @param  band             chanspec band
 *
 * @return Return TRUE if valid
 */
bool
wf_valid_320MHz_center_chan(uint center_channel, chanspec_band_t band)
{
	if (band == WL_CHANSPEC_BAND_6G) {
		/* Use the simple pattern of 6GHz center channels */
		/* In contrast to e.g. 160MHz channels, 320Mhz channels overlap
		 * (320-1, 320-2)
		 */
		if ((center_channel >= CH_MIN_6G_320M_CHANNEL &&
		     center_channel <= CH_MAX_6G_320M_CHANNEL) &&
		    ((center_channel - CH_MIN_6G_320M_CHANNEL) % CH_160MHZ_APART) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * This function returns TRUE if both the chanspec can co-exist in PHY.
 * Addition to primary20 channel, the function checks for side band for 2g 40 channels
 */
bool
wf_chspec_coexist(chanspec_t chspec1, chanspec_t chspec2)
{
	bool same_primary;

	same_primary = (wf_chspec_primary20_chspec(chspec1) == wf_chspec_primary20_chspec(chspec2));

	if (same_primary && CHSPEC_IS2G(chspec1) && CHSPEC_IS40(chspec1) && CHSPEC_IS40(chspec2)) {
		return (chspec1 == chspec2);
	}

	return same_primary;
}

/**
 * Create a 20MHz chanspec for the given band.
 *
 * This function returns a 20MHz chanspec in the given band.
 *
 * @param	channel   20MHz channel number
 * @param	band      a chanspec band (e.g. WL_CHANSPEC_BAND_2G)
 *
 * @return Returns a 20MHz chanspec, or IVNCHANSPEC in case of error.
 */
chanspec_t
wf_create_20MHz_chspec(uint channel, chanspec_band_t band)
{
	chanspec_t chspec;

	if (channel <= WL_CHANSPEC_CHAN_MASK &&
	    (band == WL_CHANSPEC_BAND_2G ||
	     band == WL_CHANSPEC_BAND_5G ||
	     band == WL_CHANSPEC_BAND_6G)) {
		chspec = band | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE | channel;
		if (!wf_chspec_valid(chspec)) {
			chspec = INVCHANSPEC;
		}
	} else {
		chspec = INVCHANSPEC;
	}

	return chspec;
}

/**
 * Returns the chanspec for a 40MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  center_channel   center channel of the 40MHz channel
 * @param  band             band of the 40MHz channel (chanspec_band_t value)
 *
 * The center_channel can be one of the 802.11 spec valid 40MHz chenter channels
 * in the given band.
 *
 * @return returns a 40MHz chanspec, or INVCHANSPEC in case of error
 */
chanspec_t
wf_create_40MHz_chspec(uint primary_channel, uint center_channel,
                       chanspec_band_t band)
{
	int sb;

	/* Calculate the sideband value for the center and primary channel.
	 * Will return -1 if not a valid pair for 40MHz
	 */
	sb = channel_to_sb(center_channel, primary_channel, WL_CHANSPEC_BW_40);

	/* return err if the sideband was bad or the center channel is not
	 * valid for the given band.
	 */
	if (sb < 0 || !wf_valid_40MHz_center_chan(center_channel, band)) {
		return INVCHANSPEC;
	}

	/* othewise construct and return the valid 40MHz chanspec */
	return (chanspec_t)(center_channel | WL_CHANSPEC_BW_40 | band |
	                    ((uint)sb << WL_CHANSPEC_CTL_SB_SHIFT));
}

/**
 * Returns the chanspec for a 40MHz channel given the primary 20MHz channel number,
 * the sub-band for the primary 20MHz channel, and the band.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  primary_subband  sub-band of the 20MHz primary channel (chanspec_subband_t value)
 * @param  band             band of the 40MHz channel (chanspec_band_t value)
 *
 * The primary channel and sub-band should describe one of the 802.11 spec valid
 * 40MHz channels in the given band.
 *
 * @return returns a 40MHz chanspec, or INVCHANSPEC in case of error
 */
chanspec_t
wf_create_40MHz_chspec_primary_sb(uint primary_channel, chanspec_subband_t primary_subband,
                                  chanspec_band_t band)
{
	uint center_channel;

	/* find the center channel */
	if (primary_subband == WL_CHANSPEC_CTL_SB_L) {
		center_channel = primary_channel + CH_10MHZ_APART;
	} else if (primary_subband == WL_CHANSPEC_CTL_SB_U) {
		center_channel = primary_channel - CH_10MHZ_APART;
	} else {
		return INVCHANSPEC;
	}

	return wf_create_40MHz_chspec(primary_channel, center_channel, band);
}

/**
 * Returns the chanspec for an 80MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  center_channel   center channel of the 80MHz channel
 * @param  band             band of the 80MHz channel (chanspec_band_t value)
 *
 * The center_channel can be one of {42, 58, 106, 122, 138, 155} for 5G,
 * or {7 + 16*X for 0 <= X <= 13} for 6G.
 *
 * @return returns an 80MHz chanspec, or INVCHANSPEC in case of error
 */
chanspec_t
wf_create_80MHz_chspec(uint primary_channel, uint center_channel,
                       chanspec_band_t band)
{
	int sb;

	/* Calculate the sideband value for the center and primary channel.
	 * Will return -1 if not a valid pair for 80MHz
	 */
	sb = channel_to_sb(center_channel, primary_channel, WL_CHANSPEC_BW_80);

	/* return err if the sideband was bad or the center channel is not
	 * valid for the given band.
	 */
	if (sb < 0 || !wf_valid_80MHz_center_chan(center_channel, band)) {
		return INVCHANSPEC;
	}

	/* othewise construct and return the valid 80MHz chanspec */
	return (chanspec_t)(center_channel | WL_CHANSPEC_BW_80 | band |
	                    ((uint)sb << WL_CHANSPEC_CTL_SB_SHIFT));
}

/**
 * Returns the chanspec for an 160MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  center_channel   center channel of the 160MHz channel
 * @param  band             band of the 160MHz channel (chanspec_band_t value)
 *
 * The center_channel can be one of {50, 114} for 5G,
 * or {15 + 32*X for 0 <= X <= 7} for 6G.
 *
 * @return returns an 160MHz chanspec, or INVCHANSPEC in case of error
 */
chanspec_t
wf_create_160MHz_chspec(uint primary_channel, uint center_channel, chanspec_band_t band)
{
	int sb;

	/* Calculate the sideband value for the center and primary channel.
	 * Will return -1 if not a valid pair for 160MHz
	 */
	sb = channel_to_sb(center_channel, primary_channel, WL_CHANSPEC_BW_160);

	/* return err if the sideband was bad or the center channel is not
	 * valid for the given band.
	 */
	if (sb < 0 || !wf_valid_160MHz_center_chan(center_channel, band)) {
		return INVCHANSPEC;
	}

	/* othewise construct and return the valid 160MHz chanspec */
	return (chanspec_t)(center_channel | WL_CHANSPEC_BW_160 | band |
	                    ((uint)sb << WL_CHANSPEC_CTL_SB_SHIFT));
}

/**
 * This function will be obsoleted.
 *
 * Returns the chanspec for an 80+80MHz channel given the primary 20MHz channel number,
 * the center channel numbers for each frequency segment, and the band.
 *
 * @param  primary_channel  primary 20 Mhz channel
 * @param  chan0            center channel number of one frequency segment
 * @param  chan1            center channel number of the other frequency segment
 * @param  band             band of the 80+80 MHz channel (chanspec_band_t value)
 *
 * Parameters chan0 and chan1 are valid 80 MHz center channel numbers for the given band.
 * The primary channel must be contained in one of the 80 MHz channels. This routine
 * will determine which frequency segment is the primary 80 MHz segment.
 *
 * @return returns an 80+80 MHz chanspec, or INVCHANSPEC in case of error
 *
 * Refer to 802.11-2016 section 21.3.14 "Channelization".
 */
chanspec_t
wf_create_8080MHz_chspec(uint primary_channel, uint chan0, uint chan1,
                       chanspec_band_t band)
{
	int sb = 0;
	chanspec_t chanspec = 0;
	int chan0_id = -1, chan1_id = -1;
	int seg0, seg1;

	/* frequency segments need to be non-contiguous, so the channel separation needs
	 * to be greater than 80MHz
	 */
	if ((uint)ABS((int)(chan0 - chan1)) <= CH_80MHZ_APART) {
		return INVCHANSPEC;
	}

	if (band == WL_CHANSPEC_BAND_5G) {
		chan0_id = channel_80mhz_to_id(chan0);
		chan1_id = channel_80mhz_to_id(chan1);
	} else if (band == WL_CHANSPEC_BAND_6G) {
		chan0_id = channel_6g_80mhz_to_id(chan0);
		chan1_id = channel_6g_80mhz_to_id(chan1);
	}

	/* make sure the channel numbers were valid */
	if (chan0_id == -1 || chan1_id == -1) {
		return INVCHANSPEC;
	}

	/* does the primary channel fit with the 1st 80MHz channel ? */
	sb = channel_to_sb(chan0, primary_channel, WL_CHANSPEC_BW_80);
	if (sb >= 0) {
		/* yes, so chan0 is frequency segment 0, and chan1 is seg 1 */
		seg0 = chan0_id;
		seg1 = chan1_id;
	} else {
		/* no, so does the primary channel fit with the 2nd 80MHz channel ? */
		sb = channel_to_sb(chan1, primary_channel, WL_CHANSPEC_BW_80);
		if (sb < 0) {
			/* no match for pri_ch to either 80MHz center channel */
			return INVCHANSPEC;
		}
		/* swapped, so chan1 is frequency segment 0, and chan0 is seg 1 */
		seg0 = chan1_id;
		seg1 = chan0_id;
	}

	chanspec = ((seg0 << WL_CHANSPEC_CHAN0_SHIFT) |
	            (seg1 << WL_CHANSPEC_CHAN1_SHIFT) |
	            (sb << WL_CHANSPEC_CTL_SB_SHIFT) |
	            WL_CHANSPEC_BW_8080 |
	            band);

	return chanspec;
}

/**
 * Returns the chanspec for an 320MHz channel given the primary 20MHz channel number,
 * the center channel number, and the band.
 *
 * @param  primary_channel  primary 20 Mhz channel
 * @param  scheme           indicator of band scheme (0 for auto, 1 for 320-1, 2 for 320-2)
 * @param  band             band of the 320 MHz channel (chanspec_band_t value)
 *
 * Parameters chan is valid 320 MHz center channel numbers for the given band.
 * The primary channel must be contained in one of the 320 MHz channels.
 *
 * @return returns an 320 MHz chanspec, or INVCHANSPEC in case of error
 *
 * Refer to <TBD> "Channelization".
 */
chanspec_t
wf_create_320MHz_chspec(uint primary_channel, wf_bw320_scheme_t scheme, chanspec_band_t band)
{
	int sb = -1;
	chanspec_t chanspec = INVCHANSPEC;
	uint idx;
	if (band != WL_CHANSPEC_BAND_6G) {
		return INVCHANSPEC;
	}
	if (scheme > WF_SCHEME_320_2) {
		return INVCHANSPEC;
	}
	/* Calculate the sideband value for the center and primary channel.
	 * Will return -1 if not a valid pair for 320MHz
	 */
	if (scheme != WF_SCHEME_320_2) {
		/* traverse 320-1 channels first, to ensure we 'prefer' 320-1 channels */
		for (idx = 0; idx < ARRAYSIZE(wf_6g_320m_chans); idx += 2) {
			sb = channel_to_sb(wf_6g_320m_chans[idx], primary_channel,
				WL_CHANSPEC_BW_320);
			if (sb >= 0) {
				break;
			}
		}
	}
	if (sb < 0 && scheme != WF_SCHEME_320_1) {
		/* traverse 320-2 channels next */
		for (idx = 1; idx < ARRAYSIZE(wf_6g_320m_chans); idx += 2) {
			sb = channel_to_sb(wf_6g_320m_chans[idx], primary_channel,
				WL_CHANSPEC_BW_320);
			if (sb >= 0) {
				break;
			}
		}
	}
	/* return err if the sideband was bad or the center channel is not
	 * valid for the given band.
	 */
	if (sb >= 0) {
		ASSERT(idx < ARRAYSIZE(wf_6g_320m_chans));
		chanspec = ((idx << WL_CHANSPEC_GE320_CHIDX_SHIFT) |
			(sb << WL_CHANSPEC_GE320_SB_SHIFT) | WL_CHANSPEC_BW_320 |
			band);
	}

	return chanspec;
}

/* Returns the 320MHz scheme given the channel center frequency index of the 320MHz bandwidth */
wf_bw320_scheme_t
wl_get_320MHz_scheme(uint8 center_channel)
{
	wf_bw320_scheme_t scheme = WF_SCHEME_AUTO;
	uint8 i;

	/* travers array of monotonously rising 320MHz center channels.
	 * even index (i) represents the 320-1 schemes and odd index (i) the 320-2 schemes.
	 */
	for (i = 0; i < WF_NUM_6G_320M_CHANS; i++) {
		if (center_channel == wf_6g_320m_chans[i]) {
			scheme = (i & 0x1) ? WF_SCHEME_320_2 : WF_SCHEME_320_1;
		}
	}
	return scheme;
}

/**
 * Returns the chanspec given the primary 20MHz channel number,
 * the center channel number, channel width, and the band. The channel width
 * must be 20, 40, 80, 160 or 320 MHz.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  center_channel   center channel of the channel
 * @param  bw               width of the channel (chanspec_bw_t)
 * @param  band             chanspec band of channel  (chanspec_band_t)
 *
 * The center_channel can be one of the 802.11 spec valid center channels
 * for the given bandwidth in the given band.
 *
 * @return returns a chanspec, or INVCHANSPEC in case of error
 */
chanspec_t
wf_create_chspec(uint primary_channel, uint center_channel,
                 chanspec_bw_t bw, chanspec_band_t band)
{
	chanspec_t chspec = INVCHANSPEC;
	int sb = -1;
	uint sb_shift;

	/* 20MHz channels have matching center and primary channels */
	if (bw == WL_CHANSPEC_BW_20 && primary_channel == center_channel) {
		sb = 0;
	} else if (bw == WL_CHANSPEC_BW_40 ||
		bw == WL_CHANSPEC_BW_80 ||
		bw == WL_CHANSPEC_BW_160 ||
		WFC_BW_EQ(bw, WL_CHANSPEC_BW_320)) {
		/* calculate the sub-band index */
		sb = channel_to_sb(center_channel, primary_channel, bw);
	}

	/* if we have a good sub-band, assemble the chanspec, and use wf_chspec_valid()
	 * to check it for correctness
	 */
	if (sb >= 0) {
		if (WFC_BW_EQ(bw, WL_CHANSPEC_BW_320)) {
			if (band == WL_CHANSPEC_BAND_6G) {
				center_channel = channel_6g_320mhz_to_id(center_channel);
				sb_shift = WL_CHANSPEC_GE320_SB_SHIFT;
			} else {
				return INVCHANSPEC;
			}
		} else {
			sb_shift = WL_CHANSPEC_CTL_SB_SHIFT;
		}

		chspec = (center_channel | band | bw | ((uint)sb << sb_shift));
		if (!wf_chspec_valid(chspec)) {
			chspec = INVCHANSPEC;
		}
	}

	return chspec;
}

/**
 * Returns the chanspec given the primary 20MHz channel number,
 * channel width, and the band.
 *
 * @param  primary_channel  primary 20Mhz channel
 * @param  bw               width of the channel (chanspec_bw_t)
 * @param  band             chanspec band of channel  (chanspec_band_t)
 *
 * @return returns a chanspec, or INVCHANSPEC in case of error
 *
 * This function is a similar to wf_create_chspec() but does not require the
 * center_channel parameter. As a result, it can not create 40MHz channels on
 * the 2G band.
 *
 * This function supports creating 20MHz bandwidth chanspecs on any band.
 *
 * For the 2GHz band, 40MHz channels overlap, so two 40MHz channels may
 * have the same primary 20MHz channel. This function will return INVCHANSPEC
 * whenever called with a bandwidth of 40MHz or wider for the 2GHz band.
 *
 * 5GHz and 6GHz bands have non-overlapping 40/80/160 MHz channels, so a
 * 20MHz primary channel uniquely specifies a wider channel in a given band.
 */
chanspec_t
wf_create_chspec_from_primary(uint primary_channel, chanspec_bw_t bw, chanspec_band_t band)
{
	chanspec_t chspec = INVCHANSPEC;

	if (bw == WL_CHANSPEC_BW_20) {
		chspec = wf_create_20MHz_chspec(primary_channel, band);
	} else if (band == WL_CHANSPEC_BAND_2G) {
		/* 2G 40MHz cannot be uniquely identified by the primary channel.
		 * Return INVAL for any channel given. Or if bw != 20
		 */
	} else if (band == WL_CHANSPEC_BAND_5G) {
		/* For 5GHz, use the lookup tables for valid 40/80/160 center channels
		 * and search for a center channel compatible with the given primary channel.
		 */
		const uint8 *center_ch = NULL;
		uint num_ch, i;

		if (bw == WL_CHANSPEC_BW_40) {
			center_ch = wf_5g_40m_chans;
			num_ch = WF_NUM_5G_40M_CHANS;
		} else if (bw == WL_CHANSPEC_BW_80) {
			center_ch = wf_5g_80m_chans;
			num_ch = WF_NUM_5G_80M_CHANS;
		} else if (bw == WL_CHANSPEC_BW_160) {
			center_ch = wf_5g_160m_chans;
			num_ch = WF_NUM_5G_160M_CHANS;
		} else {
			num_ch = 0;
		}

		for (i = 0; i < num_ch; i ++) {
			chspec = wf_create_chspec(primary_channel, center_ch[i], bw, band);
			if (chspec != INVCHANSPEC) {
				break;
			}
		}
	} else if (band == WL_CHANSPEC_BAND_6G) {
		/* For 6GHz, use a formula to calculate the valid 40/80/160 center channel from
		 * the primary channel.
		 */
		uint ch_per_block;  /* unit: [channels] */
		uint mask;
		uint base, center; /* unit: [channelnr] */

		if (bw == WL_CHANSPEC_BW_40) {
			ch_per_block = CH_40MHZ_APART;
		} else if (bw == WL_CHANSPEC_BW_80) {
			ch_per_block = CH_80MHZ_APART;
		} else if (bw == WL_CHANSPEC_BW_160) {
			ch_per_block = CH_160MHZ_APART;
		} else if (WFC_BW_EQ(bw, WL_CHANSPEC_BW_320)) {
			/* 6G 320MHz cannot be uniquely identified by the primary channel.
			 * Return INVAL for any channel given.
			 */
			ch_per_block = 0;
		} else {
			ch_per_block = 0;
		}

		if (ch_per_block) {
			/* calculate the base of the block of channel numbers
			 * covered by the given bw
			 */
			mask = ~(ch_per_block - 1);
			base = 1 + ((primary_channel - 1) & mask);

			/* calculate the center channel from the base channel */
			center = base + center_chan_to_edge(bw);
			chspec = wf_create_chspec(primary_channel, center, bw, band);
		}
	}

	return chspec;
}

/**
 * Retrun 6G center channel given primary 20MHz channel, scheme and bandwidth.
 */
uint
wf_6G_primary20_ch_to_center_ch(uint primary_channel, wf_bw320_scheme_t scheme, chanspec_bw_t bw)
{
	uint delta, i;

	if (!wf_valid_20MHz_chan(primary_channel, WL_CHANSPEC_BAND_6G)) {
		return INVCHANNEL;
	}

	/* Channel 2 and 233 only operate with BW20 */
	if (((primary_channel == 2) || (primary_channel == 233)) && bw != WL_CHANSPEC_BW_20) {
		return INVCHANNEL;
	}

	if (bw == WL_CHANSPEC_BW_20) {
		return primary_channel;
	} else if (bw == WL_CHANSPEC_BW_40) {
		delta = center_chan_to_edge(WL_CHANSPEC_BW_40);

		for (i = 0; i < WF_NUM_6G_40M_CHANS; i++) {
			if ((uint)ABS((int)(primary_channel - wf_6g_40m_chans[i])) <= delta) {
				return wf_6g_40m_chans[i];
			}
		}

	} else if (bw == WL_CHANSPEC_BW_80) {
		delta = center_chan_to_edge(WL_CHANSPEC_BW_80);

		for (i = 0; i < WF_NUM_6G_80M_CHANS; i++) {
			if ((uint)ABS((int)(primary_channel - wf_6g_80m_chans[i])) <= delta) {
				return wf_6g_80m_chans[i];
			}
		}
	} else if (bw == WL_CHANSPEC_BW_160) {
		delta = center_chan_to_edge(WL_CHANSPEC_BW_160);

		for (i = 0; i < WF_NUM_6G_160M_CHANS; i++) {
			if ((uint)ABS((int)(primary_channel - wf_6g_160m_chans[i])) <= delta) {
				return wf_6g_160m_chans[i];
			}
		}
	} else if (bw == WL_CHANSPEC_BW_320) {
		if (scheme <= WF_SCHEME_320_2) {
			delta = center_chan_to_edge(WL_CHANSPEC_BW_320);
			for (i = 0; i < WF_NUM_6G_320M_CHANS; i++) {
				if ((scheme == WF_SCHEME_320_1) && ((i & 0x1) == 0x1)) {
					continue;
				}
				if ((scheme == WF_SCHEME_320_2) && ((i & 0x1) == 0x0)) {
					continue;
				}
				if ((uint)ABS((int)(primary_channel - wf_6g_320m_chans[i])) <=
					delta) {
					return wf_6g_320m_chans[i];
				}
			}
		}
	}

	return INVCHANNEL;
}

/**
 * Return the primary 20MHz channel.
 *
 * This function returns the channel number of the primary 20MHz channel. For
 * 20MHz channels this is just the channel number. For 40MHz or wider channels
 * it is the primary 20MHz channel specified by the chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the channel number of the primary 20MHz channel, or INVCHANNEL on error.
 */
uint8
wf_chspec_primary20_chan(chanspec_t chspec)
{
	uint center_chan = INVCHANNEL;
	chanspec_bw_t bw;
	uint sb;

	ASSERT(!wf_chspec_malformed(chspec));

	/* Is there a sideband ? */
	if (CHSPEC_IS20(chspec)) {
		return CHSPEC_CHANNEL(chspec);
	} else {
		if (CHSPEC_IS320(chspec)) {
			sb = CHSPEC_GE320_SB(chspec) >> WL_CHANSPEC_GE320_SB_SHIFT;
		} else {
			sb = CHSPEC_CTL_SB(chspec) >> WL_CHANSPEC_CTL_SB_SHIFT;
		}

		if (CHSPEC_IS320(chspec)) {
			/* use bw 320MHz for the primary channel lookup */
			bw = WL_CHANSPEC_BW_320;

			/* convert from channel index to channel number */
			if (CHSPEC_IS6G(chspec)) {
				center_chan = wf_chspec_6G_id320_to_ch(CHSPEC_GE320_CHIDX(chspec));
			}
			if (center_chan == INVCHANNEL) {
				return INVCHANNEL;
			}
		} else {
			bw = CHSPEC_BW(chspec);
			center_chan = CHSPEC_CHANNEL(chspec) >> WL_CHANSPEC_CHAN_SHIFT;
		}

		return (uint8)(wf_center_to_primary20_chan((uint8)center_chan, bw, sb));
	}
}

/**
 * Return the bandwidth string for a given chanspec
 *
 * This function returns the bandwidth string for the passed chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the bandwidth string:
 *         "20", "40", "80", "160", "320"
 */
const char *
BCMRAMFN(wf_chspec_to_bw_str)(chanspec_t chspec)
{
	return wf_chspec_bw_str[(CHSPEC_BW(chspec) >> WL_CHANSPEC_BW_SHIFT)];
}

/**
 * Return the primary 20MHz chanspec of a given chanspec
 *
 * This function returns the chanspec of the primary 20MHz channel. For 20MHz
 * channels this is just the chanspec. For 40MHz or wider channels it is the
 * chanspec of the primary 20MHz channel specified by the chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the chanspec of the primary 20MHz channel
 */
chanspec_t
wf_chspec_primary20_chspec(chanspec_t chspec)
{
	chanspec_t pri_chspec = chspec;
	uint8 pri_chan;

	ASSERT(!wf_chspec_malformed(chspec));

	/* Is there a sideband ? */
	if (!CHSPEC_IS20(chspec)) {
		pri_chan = wf_chspec_primary20_chan(chspec);
		pri_chspec = pri_chan | WL_CHANSPEC_BW_20;
		pri_chspec |= CHSPEC_BAND(chspec);
	}
	return pri_chspec;
}

/* return chanspec given primary 20MHz channel, bandwidth and band
 * return 0 on error
 */
chanspec_t
wf_channel2chspec(uint pri_ch, uint bw, uint wl_chanspec_band)
{
	chanspec_t chspec;
	const uint8 *center_ch = NULL;
	int num_ch = 0;
	int sb = -1;
	int i = 0;

	ASSERT(wl_chanspec_band == WL_CHANSPEC_BAND_2G || wl_chanspec_band == WL_CHANSPEC_BAND_5G ||
	       wl_chanspec_band == WL_CHANSPEC_BAND_6G);
	ASSERT((wl_chanspec_band == WL_CHANSPEC_BAND_2G && pri_ch <= CH_MAX_2G_CHANNEL) ||
	       (wl_chanspec_band == WL_CHANSPEC_BAND_5G && pri_ch >= CH_MIN_5G_CHANNEL &&
	        pri_ch <= CH_MAX_5G_CHANNEL) ||
	       (wl_chanspec_band == WL_CHANSPEC_BAND_6G && pri_ch >= CH_MIN_6G_CHANNEL &&
	        pri_ch <= CH_MAX_6G_CHANNEL));
	chspec = (wl_chanspec_band | bw);

	if (bw == WL_CHANSPEC_BW_40) {
		switch (wl_chanspec_band) {
		case WL_CHANSPEC_BAND_2G: {
			/* 2G 40MHz is a special case; channel_to_sb() works for 5G only */
			/* In 2.4GHz, pri_ch 5, 6 & 7 can be used as both lower and upper sb.
			 * For such ambiguous cases, lower is chosen by default here.
			 * Japan center channels 10 and 11 are used in upper SB context only.
			 */
			const uint8 ctl2cent[] = {3, 4, 5, 6, 7, 8, 9, 6, 7, 8, 9, 10, 11};
			const uint8 len_c2c = ARRAYSIZE(ctl2cent);
			uint8 cent;
			if (pri_ch < 1 || pri_ch > len_c2c) {
				return 0;
			}
			cent = ctl2cent[pri_ch - 1];
			chspec |= cent;
			chspec |= (pri_ch < cent ?
					WL_CHANSPEC_CTL_SB_LOWER : WL_CHANSPEC_CTL_SB_UPPER);
			return chspec;
		}
		case WL_CHANSPEC_BAND_5G:
			center_ch = wf_5g_40m_chans;
			num_ch = WF_NUM_5G_40M_CHANS;
			break;
		case WL_CHANSPEC_BAND_6G:
			center_ch = wf_6g_40m_chans;
			num_ch = ARRAYSIZE(wf_6g_40m_chans);
			break;
		}
	} else if (bw == WL_CHANSPEC_BW_80) {
		switch (wl_chanspec_band) {
		case WL_CHANSPEC_BAND_2G:
			ASSERT(0);
			break;
		case WL_CHANSPEC_BAND_5G:
			center_ch = wf_5g_80m_chans;
			num_ch = WF_NUM_5G_80M_CHANS;
			break;
		case WL_CHANSPEC_BAND_6G:
			center_ch = wf_6g_80m_chans;
			num_ch = ARRAYSIZE(wf_6g_40m_chans);
			break;
		}
	} else if (bw == WL_CHANSPEC_BW_160) {
		switch (wl_chanspec_band) {
		case WL_CHANSPEC_BAND_2G:
			ASSERT(0);
			break;
		case WL_CHANSPEC_BAND_5G:
			center_ch = wf_5g_160m_chans;
			num_ch = WF_NUM_5G_160M_CHANS;
			break;
		case WL_CHANSPEC_BAND_6G:
			center_ch = wf_6g_160m_chans;
			num_ch = ARRAYSIZE(wf_6g_160m_chans);
			break;
		}
	} else if (bw == WL_CHANSPEC_BW_320) {
		switch (wl_chanspec_band) {
		case WL_CHANSPEC_BAND_2G:
			ASSERT(0);
			break;
		case WL_CHANSPEC_BAND_5G:
			ASSERT(0);
			break;
		case WL_CHANSPEC_BAND_6G:
			/* 6G 320MHz is a special case because of overlapping 320MHz channels
			 * For such ambiguous cases, lower is chosen by default here.
			 */
			center_ch = wf_6g_320m_chans;
			num_ch = ARRAYSIZE(wf_6g_320m_chans);
		}
	} else if (bw == WL_CHANSPEC_BW_20) {
		chspec |= pri_ch;
		return chspec;
	} else {
		return 0;
	}

	for (i = 0; i < num_ch; i ++) {
		sb = channel_to_sb(center_ch[i], pri_ch, (chanspec_bw_t)bw);
		if (sb >= 0) {
			if (bw == WL_CHANSPEC_BW_320) {
				chspec |= i; /* index iso channel */
				chspec |= (sb << WL_CHANSPEC_GE320_SB_SHIFT);
			} else {
				chspec |= center_ch[i];
				chspec |= (sb << WL_CHANSPEC_CTL_SB_SHIFT);
			}
			break;
		}
	}

	/* check for no matching sb/center */
	if (sb < 0) {
		return 0;
	}

	return chspec;
}

/**
 * Return the primary 40MHz chanspec or a 40MHz or wider channel
 *
 * This function returns the chanspec for the primary 40MHz of an 80MHz or wider channel.
 * The primary 40MHz channel is the 40MHz sub-band that contains the primary 20MHz channel.
 * The primary 20MHz channel of the returned 40MHz chanspec is the same as the primary 20MHz
 * channel of the input chanspec.
 *
 * @param	chspec    input chanspec
 *
 * @return Returns the chanspec of the primary 20MHz channel
 */
chanspec_t
wf_chspec_primary40_chspec(chanspec_t chspec)
{
	chanspec_t chspec40 = chspec;
	uint center_chan;
	uint sb;

	ASSERT(!wf_chspec_malformed(chspec));

	/* if the chanspec is > 80MHz, use the helper routines to find the primary 80MHz channel */
	if (CHSPEC_IS320(chspec)) {
		chspec = wf_chspec_primary160_chspec(chspec);
	}

	if (CHSPEC_IS160(chspec)) {
		chspec = wf_chspec_primary80_chspec(chspec);
	}

	/* determine primary 40 MHz sub-channel of an 80MHz chanspec */
	if (CHSPEC_IS80(chspec)) {
		center_chan = CHSPEC_CHANNEL(chspec);
		sb = CHSPEC_CTL_SB(chspec);

		if (sb < WL_CHANSPEC_CTL_SB_UL) {
			/* Primary 40MHz is on lower side */
			center_chan -= CH_20MHZ_APART;
			/* sideband bits are the same for LL/LU and L/U */
		} else {
			/* Primary 40MHz is on upper side */
			center_chan += CH_20MHZ_APART;
			/* sideband bits need to be adjusted by UL offset */
			sb -= WL_CHANSPEC_CTL_SB_UL;
		}

		/* Create primary 40MHz chanspec */
		chspec40 = (CHSPEC_BAND(chspec) | WL_CHANSPEC_BW_40 |
		            sb | center_chan);
	}

	return chspec40;
}

/**
 * Return the channel number for a given frequency and base frequency.
 *
 * @param   freq            frequency in MHz of the channel center
 * @param   start_factor    starting base frequency in 500 KHz units
 *
 * @return  Returns a channel number > 0, or -1 on error
 *
 * The returned channel number is relative to the given base frequency.
 *
 * The base frequency is specified as (start_factor * 500 kHz).
 * Constants WF_CHAN_FACTOR_2_4_G, WF_CHAN_FACTOR_5_G, and WF_CHAN_FACTOR_6_G are
 * defined for 2.4 GHz, 5 GHz, and 6 GHz bands.
 *
 * If the given base frequency is zero these base frequencies are assumed:
 *
 *              freq (GHz)  -> assumed base freq (GHz)
 *  2G band   2.4   - 2.5      2.407
 *  5G band   5.0   - 5.940    5.000
 *  6G band   5.940 - 7.205    5.940
 *
 * It is an error if the start_factor is zero and the freq is not in one of
 * these ranges.
 *
 * The returned channel will be in the range [1, 14] in the 2.4 GHz band,
 * [1, 253] for 6 GHz band, or [1, 200] otherwise.
 *
 * It is an error if the start_factor is WF_CHAN_FACTOR_2_4_G and the
 * frequency is not a 2.4 GHz channel. For any other start factor the frequency
 * must be an even 5 MHz multiple greater than the base frequency.
 *
 * For a start_factor WF_CHAN_FACTOR_6_G, the frequency may be up to 7.205 MHz
 * (channel 253). For any other start_factor, the frequence can be up to
 * 1 GHz from the base freqency (channel 200).
 *
 * Reference 802.11-2016, section 17.3.8.3 and section 16.3.6.3
 */
int
wf_mhz2channel(uint freq, uint start_factor)
{
	int ch = -1;
	uint base;
	int offset;

	/* take the default channel start frequency */
	if (start_factor == 0) {
		if (freq >= 2400 && freq <= 2500) {
			start_factor = WF_CHAN_FACTOR_2_4_G;
		} else if (freq >= 5000 && freq < 5930) {
			start_factor = WF_CHAN_FACTOR_5_G;
		} else if (freq >= 5930 && freq <= 7205) {
			start_factor = WF_CHAN_FACTOR_6_G;
		}
	}

	if (freq == 2484 && start_factor == WF_CHAN_FACTOR_2_4_G)
		return 14;

	if (freq == 5935 && start_factor == WF_CHAN_FACTOR_6_G)
		return 2;

	base = start_factor / 2;

	if (freq < base) {
		return -1;
	}

	offset = freq - base;
	ch = offset / 5;

	/* check that frequency is a 5MHz multiple from the base */
	if (offset != (ch * 5))
		return -1;

	/* channel range checks */
	if (start_factor == WF_CHAN_FACTOR_2_4_G) {
		/* 2G should only be up to 13 here as 14 is
		 * handled above as it is a non-5MHz offset
		 */
		if (ch > 13) {
			ch = -1;
		}
	}
	else if (start_factor == WF_CHAN_FACTOR_6_G) {
		/* 6G has a higher channel range than 5G channelization specifies [1,200] */
		if ((uint)ch > CH_MAX_6G_CHANNEL) {
			ch = -1;
		}
	} else if (ch > 200) {
			ch = -1;
	}

	return ch;
}

/**
 * Return the center frequency in MHz of the given channel and base frequency.
 *
 * The channel number is interpreted relative to the given base frequency.
 *
 * The valid channel range is [1, 14] in the 2.4 GHz band, [1,253] in the 6 GHz
 * band, and [1, 200] otherwise.
 * The base frequency is specified as (start_factor * 500 kHz).
 * Constants WF_CHAN_FACTOR_2_4_G, WF_CHAN_FACTOR_5_G, and WF_CHAN_FACTOR_6_G are
 * defined for 2.4 GHz, 5 GHz, and 6 GHz bands.
 * The channel range of [1, 14] is only checked for a start_factor of
 * WF_CHAN_FACTOR_2_4_G (4814).
 * Odd start_factors produce channels on .5 MHz boundaries, in which case
 * the answer is rounded down to an integral MHz.
 * -1 is returned for an out of range channel.
 *
 * Reference 802.11-2016, section 17.3.8.3 and section 16.3.6.3
 *
 * @param	channel       input channel number
 * @param	start_factor  base frequency in 500 kHz units, e.g. 10000 for 5 GHz
 *
 * @return Returns a frequency in MHz
 *
 * @see  WF_CHAN_FACTOR_2_4_G
 * @see  WF_CHAN_FACTOR_5_G
 * @see  WF_CHAN_FACTOR_6_G
 */
int
wf_channel2mhz(uint ch, uint start_factor)
{
	int freq;

	if ((start_factor == WF_CHAN_FACTOR_2_4_G && (ch < 1 || ch > 14)) ||
	    (start_factor == WF_CHAN_FACTOR_6_G && (ch < 1 || ch > 253)) ||
	    (start_factor != WF_CHAN_FACTOR_6_G && (ch < 1 || ch > 200))) {
		freq = -1;
	} else if ((start_factor == WF_CHAN_FACTOR_2_4_G) && (ch == 14)) {
		freq = 2484;
	} else if ((start_factor == WF_CHAN_FACTOR_6_G) && (ch == 2)) {
		freq = 5935;
	} else {
		freq = ch * 5 + start_factor / 2;
	}

	return freq;
}

/**
 * Return the center frequency in MHz of the given chanspec.
 * See wf_channel2mhz() for details.
 */
int
wf_chanspec2mhz(chanspec_t chanspec)
{
	const uint chan_factors_arr[] = {WF_CHAN_FACTOR_2_4_G, WF_CHAN_FACTOR_6_G,
		WF_CHAN_FACTOR_4_G, WF_CHAN_FACTOR_5_G}; /* ordered as WL_CHANSPEC_BAND_* macros */
	const uint8 channel_num = CHSPEC_CHANNEL(chanspec);
	const uint8 shifted_band = (CHSPEC_BAND(chanspec) >> WL_CHANSPEC_BAND_SHIFT) & 0x3;
	const uint chan_factor = chan_factors_arr[shifted_band];

	return wf_channel2mhz(channel_num, chan_factor);
}

chanspec_t
wf_chspec_get8080_chspec(uint8 primary_20mhz, uint8 chan0, uint8 chan1)
{
	int sb = 0;
	chanspec_t chanspec = 0;
	int chan0_id = 0, chan1_id = 0;
	int seg0, seg1;

	chan0_id = channel_80mhz_to_id(chan0);
	chan1_id = channel_80mhz_to_id(chan1);

	/* make sure the channel numbers were valid */
	if (chan0_id == -1 || chan1_id == -1)
		return INVCHANSPEC;

	/* does the primary channel fit with the 1st 80MHz channel ? */
	sb = channel_to_sb(chan0, primary_20mhz, WL_CHANSPEC_BW_80);
	if (sb >= 0) {
		/* yes, so chan0 is frequency segment 0, and chan1 is seg 1 */
		seg0 = chan0_id;
		seg1 = chan1_id;
	} else {
		/* no, so does the primary channel fit with the 2nd 80MHz channel ? */
		sb = channel_to_sb(chan1, primary_20mhz, WL_CHANSPEC_BW_80);
		if (sb < 0) {
			/* no match for pri_ch to either 80MHz center channel */
			return INVCHANSPEC;
		}
		/* swapped, so chan1 is frequency segment 0, and chan0 is seg 1 */
		seg0 = chan1_id;
		seg1 = chan0_id;
	}

	chanspec = ((seg0 << WL_CHANSPEC_CHAN0_SHIFT) |
	            (seg1 << WL_CHANSPEC_CHAN1_SHIFT) |
	            (sb << WL_CHANSPEC_CTL_SB_SHIFT) |
	            WL_CHANSPEC_BW_8080 |
	            WL_CHANSPEC_BAND_5G);

	return chanspec;
}

/*
 * Returns the center channel of the primary 80 MHz sub-band of the provided chanspec
 */
uint8
wf_chspec_primary80_channel(chanspec_t chanspec)
{
	chanspec_t primary80_chspec;
	uint8 primary80_chan;

	primary80_chspec = wf_chspec_primary80_chspec(chanspec);

	if (primary80_chspec == INVCHANSPEC) {
		primary80_chan = INVCHANNEL;
	} else {
		primary80_chan = CHSPEC_CHANNEL(primary80_chspec);
	}

	return primary80_chan;
}

/*
 * Returns the center channel of the secondary 80 MHz sub-band of the provided chanspec
 */
uint8
wf_chspec_secondary80_channel(chanspec_t chanspec)
{
	chanspec_t secondary80_chspec;
	uint8 secondary80_chan;

	secondary80_chspec = wf_chspec_secondary80_chspec(chanspec);

	if (secondary80_chspec == INVCHANSPEC) {
		secondary80_chan = INVCHANNEL;
	} else {
		secondary80_chan = CHSPEC_CHANNEL(secondary80_chspec);
	}

	return secondary80_chan;
}

/*
 * Returns the chanspec for the primary 80MHz sub-band of an 160MHz channel
 */
chanspec_t
wf_chspec_primary80_chspec(chanspec_t chspec)
{
	chanspec_t pri_chspec80;
	uint center_chan;
	uint sb;

	ASSERT(!wf_chspec_malformed(chspec));

	/* use helper routine to create the primary 160MHz chanspec */
	if (CHSPEC_IS320(chspec)) {
		chspec = wf_chspec_primary160_chspec(chspec);
	}

	if (CHSPEC_IS80(chspec)) {
		pri_chspec80 = chspec;
	} else if (CHSPEC_IS160(chspec)) {
		center_chan = CHSPEC_CHANNEL(chspec);
		sb = CHSPEC_CTL_SB(chspec);

		if (sb < WL_CHANSPEC_CTL_SB_ULL) {
			/* Primary 80MHz is on lower side */
			center_chan -= CH_40MHZ_APART;
		}
		else {
			/* Primary 80MHz is on upper side */
			center_chan += CH_40MHZ_APART;
			sb -= WL_CHANSPEC_CTL_SB_ULL;
		}

		/* Create primary 80MHz chanspec */
		pri_chspec80 = (CHSPEC_BAND(chspec) | WL_CHANSPEC_BW_80 | sb | center_chan);
	} else {
		pri_chspec80 = INVCHANSPEC;
	}

	return pri_chspec80;
}

chanspec_t
wf_chspec_secondary160_chspec(chanspec_t chspec)
{
	chanspec_t chspec160;
	uint center_chan;
	uint sb;

	ASSERT(!wf_chspec_malformed(chspec));

	if (CHSPEC_IS160(chspec)) {
		chspec160 = chspec;
	} else if (CHSPEC_IS320(chspec)) {
		center_chan = wf_chspec_320_id2cch(chspec);
		if (center_chan == INVCHANNEL) {
			return INVCHANSPEC;
		}

		sb = CHSPEC_GE320_SB(chspec) >> WL_CHANSPEC_GE320_SB_SHIFT;

		if (sb < 8u) {
			/* Primary 160MHz is on lower side */
			center_chan += CH_80MHZ_APART;
		}
		else {
			/* Primary 160MHz is on upper side */
			center_chan -= CH_80MHZ_APART;
			sb -= 8u;
		}

		/* Create secondary 160MHz chanspec */
		chspec160 = (CHSPEC_BAND(chspec) |
		             WL_CHANSPEC_BW_160 |
		             (sb << WL_CHANSPEC_CTL_SB_SHIFT) |
		             center_chan);
	}
	else {
		chspec160 = INVCHANSPEC;
	}

	return chspec160;
}

/*
 * Returns the chanspec for the secondary 80MHz sub-band of an 160MHz channel
 */
chanspec_t
wf_chspec_secondary80_chspec(chanspec_t chspec)
{
	chanspec_t sec_chspec80;
	uint center_chan;

	ASSERT(!wf_chspec_malformed(chspec));

	if (CHSPEC_IS160(chspec)) {
		center_chan = CHSPEC_CHANNEL(chspec);

		if (CHSPEC_CTL_SB(chspec) < WL_CHANSPEC_CTL_SB_ULL) {
			/* Secondary 80MHz is on upper side */
			center_chan += CH_40MHZ_APART;
		} else {
			/* Secondary 80MHz is on lower side */
			center_chan -= CH_40MHZ_APART;
		}

		/* Create secondary 80MHz chanspec */
		sec_chspec80 = (CHSPEC_BAND(chspec) |
		            WL_CHANSPEC_BW_80 |
		            WL_CHANSPEC_CTL_SB_LL |
		            center_chan);
	}
	else {
		sec_chspec80 = INVCHANSPEC;
	}

	return sec_chspec80;
}

/*
 * For 160MHz chanspec, set ch[0]/ch[1] to be the low/high 80 Mhz channels
 *
 * For 20/40/80MHz chanspec, set ch[0] to be the center freq, and chan[1]=-1
 */
void
wf_chspec_get_80p80_channels(chanspec_t chspec, uint8 *ch)
{

	if (CHSPEC_IS160(chspec)) {
		uint8 center_chan = CHSPEC_CHANNEL(chspec);
		ch[0] = center_chan - CH_40MHZ_APART;
		ch[1] = center_chan + CH_40MHZ_APART;
	}
	else {
		/* for 20, 40, and 80 Mhz */
		ch[0] = CHSPEC_CHANNEL(chspec);
		ch[1] = -1;
	}
	return;

}

/**
 * Returns the center channel of the primary 160MHz sub-band of a 160MHz or 320MHz channel
 */
uint8
wf_chspec_primary160_channel(chanspec_t chanspec)
{
	chanspec_t primary160_chspec;
	uint8 primary160_chan;

	primary160_chspec = wf_chspec_primary160_chspec(chanspec);

	if (primary160_chspec == INVCHANSPEC) {
		primary160_chan = INVCHANNEL;
	} else {
		primary160_chan = CHSPEC_CHANNEL(primary160_chspec);
	}

	return primary160_chan;
}

/**
 * Returns the chanspec for the primary 160MHz sub-band of a 160MHz or 320MHz channel
 */
chanspec_t
wf_chspec_primary160_chspec(chanspec_t chspec)
{
	chanspec_t chspec160;
	uint center_chan;
	uint sb;

	ASSERT(!wf_chspec_malformed(chspec));

	if (CHSPEC_IS160(chspec)) {
		chspec160 = chspec;
	} else if (CHSPEC_IS320(chspec)) {
		center_chan = wf_chspec_320_id2cch(chspec);
		if (center_chan == INVCHANNEL) {
			return INVCHANSPEC;
		}

		sb = CHSPEC_GE320_SB(chspec) >> WL_CHANSPEC_GE320_SB_SHIFT;

		if (sb < 8u) {
			/* Primary 160MHz is on lower side */
			center_chan -= CH_80MHZ_APART;
		}
		else {
			/* Primary 160MHz is on upper side */
			center_chan += CH_80MHZ_APART;
			sb -= 8u;
		}

		/* Create primary 160MHz chanspec */
		chspec160 = (CHSPEC_BAND(chspec) |
		             WL_CHANSPEC_BW_160 |
		             (sb << WL_CHANSPEC_CTL_SB_SHIFT) |
		             center_chan);
	}
	else {
		chspec160 = INVCHANSPEC;
	}

	return chspec160;
} /* wf_chspec_primary160_chspec */

/* Populates array with all 20MHz side bands of a given chanspec_t in the following order:
 *		primary20, secondary20, two secondary40s, four secondary80s.
 *    'chspec' is the chanspec of interest
 *    'pext' must point to an uint8 array of long enough to hold all side bands of the given chspec
 *
 * Works with 20, 40, 80, 160 and 320MHz chspec
 */
void
wf_get_all_ext(chanspec_t chspec, uint8 *pext)
{
	chanspec_t t = CHSPEC_IS320(chspec) ? wf_chspec_primary160_chspec(chspec):
		/* if bw > 80MHz */
		(CHSPEC_IS160(chspec) ? wf_chspec_primary80_chspec(chspec) :
		 (chspec)); /* extract primary 80 */
	/* primary20 channel as first element */
	uint8 pri_ch = (pext)[0] = wf_chspec_primary20_chan(t);
	if (CHSPEC_ISLE20(chspec)) return; /* nothing more to do since 20MHz chspec */
	/* 20MHz EXT */
	(pext)[1] = (IS_CTL_IN_L20(t) ? pri_ch + CH_20MHZ_APART : pri_ch - CH_20MHZ_APART);
	if (CHSPEC_IS40(chspec)) return; /* nothing more to do since 40MHz chspec */
	/* center 40MHz EXT */
	t = wf_channel2chspec((IS_CTL_IN_L40(chspec) ?
		pri_ch + CH_40MHZ_APART : pri_ch - CH_40MHZ_APART), WL_CHANSPEC_BW_40,
		CHSPEC_BAND(chspec));
	GET_ALL_SB(t, &((pext)[2])); /* get the 20MHz side bands in 40MHz EXT */
	if (CHSPEC_IS80(chspec)) return; /* nothing more to do since 80MHz chspec */
	if (CHSPEC_IS320(chspec)) {
		t = wf_channel2chspec((IS_CTL_IN_L80(chspec) ?
				pri_ch + CH_80MHZ_APART : pri_ch - CH_80MHZ_APART),
				WL_CHANSPEC_BW_80, CHSPEC_BAND(chspec));
	} else {
		t = CH80MHZ_CHSPEC(CHSPEC_BAND(chspec), wf_chspec_secondary80_channel(chspec),
			WL_CHANSPEC_CTL_SB_LLL);
	}
	/* get the 20MHz side bands in 80MHz EXT (primary(320MHz)/secondary(!320MHz)) */
	GET_ALL_SB(t, &((pext)[4]));
	if (CHSPEC_IS160(chspec)) return;
	t = CH160MHZ_CHSPEC(CHSPEC_BAND(chspec),
			CHSPEC_CHANNEL(wf_chspec_secondary160_chspec(chspec)),
		WL_CHANSPEC_CTL_SB_LLLLL);
	/* get the 20MHz side bands in 160MHz EXT (secondary) */
	GET_ALL_SB(t, &((pext)[8]));
}

/*
 * Given two chanspecs, returns true if they overlap.
 * (Overlap: At least one 20MHz subband is common between the two chanspecs provided)
 */
bool wf_chspec_overlap(chanspec_t chspec0, chanspec_t chspec1)
{
	uint8 ch0, ch1;
	uint8 last_channel0, last_channel1;

	if (CHSPEC_BAND(chspec0) != CHSPEC_BAND(chspec1)) {
		return FALSE;
	}

	FOREACH_20_SB_EFF(chspec0, ch0, last_channel0) {
		FOREACH_20_SB_EFF(chspec1, ch1, last_channel1) {
			if ((uint)ABS(ch0 - ch1) < CH_20MHZ_APART) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

uint8
channel_bw_to_width(chanspec_t chspec)
{
	uint8 channel_width;

	if (CHSPEC_IS80(chspec))
		channel_width = VHT_OP_CHAN_WIDTH_80;
	else if (CHSPEC_IS160(chspec))
		channel_width = VHT_OP_CHAN_WIDTH_160;
	else
		channel_width = VHT_OP_CHAN_WIDTH_20_40;

	return channel_width;
}

/** returns the 20Mhz control chanspec for the caller supplied chspec */
chanspec_t
wf_ctlchspec20_from_chspec(chanspec_t chspec)
{
	uint ch_nr = wf_chspec_ctlchan(chspec);

	return (WL_CHANSPEC_BW_20 | (chspec & WL_CHANSPEC_BAND_MASK) | ch_nr);
}
