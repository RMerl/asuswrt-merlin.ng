/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: broadcom.c,v 1.1.1.1 2010/10/15 02:24:15 shinjung Exp $
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */

#include <shared.h>
#include <typedefs.h>
#include <proto/ethernet.h>
#ifdef RTCONFIG_BCMWL6
#include <proto/wps.h>
#endif
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#ifdef HND_ROUTER
#include "bcmwifi_rates.h"
#include "wlioctl_defs.h"
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
#include <wlc_types.h>
#include <802.11ax.h>
//#include <bcmwifi_rspec.h>
#endif
#include <wlutils.h>
#include <linux/types.h>
#include <wlscan.h>
#include <sysinfo.h>
#ifdef RTCONFIG_BCMWL6
#include <dirent.h>
#ifdef __CONFIG_DHDAP__
#include <security_ipc.h>
#endif

#ifdef RTCONFIG_QTN
#include "web-qtn.h"
#endif

enum {
	NOTHING,
	REBOOT,
	RESTART,
};
#endif

#define EZC_FLAGS_READ		0x0001
#define EZC_FLAGS_WRITE		0x0002
#define EZC_FLAGS_CRYPT		0x0004

#define EZC_CRYPT_KEY		"620A83A6960E48d1B05D49B0288A2C1F"

#define EZC_SUCCESS	 	0
#define EZC_ERR_NOT_ENABLED 	1
#define EZC_ERR_INVALID_STATE 	2
#define EZC_ERR_INVALID_DATA 	3

#define NVRAM_BUFSIZE	100

#ifndef NOUSB
static const char * const apply_header =
"<head>"
"<title>Broadcom Home Gateway Reference Design: Apply</title>"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
"<style type=\"text/css\">"
"body { background: white; color: black; font-family: arial, sans-serif; font-size: 9pt }"
".title	{ font-family: arial, sans-serif; font-size: 13pt; font-weight: bold }"
".subtitle { font-family: arial, sans-serif; font-size: 11pt }"
".label { color: #306498; font-family: arial, sans-serif; font-size: 7pt }"
"</style>"
"</head>"
"<body>"
"<p>"
"<span class=\"title\">APPLY</span><br>"
"<span class=\"subtitle\">This screen notifies you of any errors "
"that were detected while changing the router's settings.</span>"
"<form method=\"get\" action=\"apply.cgi\">"
"<p>"
;

static const char * const apply_footer =
"<p>"
"<input type=\"button\" name=\"action\" value=\"Continue\" OnClick=\"document.location.href='%s';\">"
"</form>"
"<p class=\"label\">&#169;2001-2004 Broadcom Corporation. All rights reserved.</p>"
"</body>"
;
#endif

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#define sys_restart() kill(1, SIGHUP)
#define sys_reboot() kill(1, SIGTERM)
#define sys_stats(url) eval("stats", (url))

int
ej_wl_sta_status(int eid, webs_t wp, char *name)
{
	// TODO
	return 0;
}

#if defined(RTCONFIG_BCMWL6) && !defined(RTCONFIG_BCM_7114) && !defined(RTCONFIG_BCM7) && !defined(RTCONFIG_BCM9) && !defined(HND_ROUTER)
#include <wlu_common.h>
#endif
#include <bcmendian.h>
#include <bcmparams.h>		/* for DEV_NUMIFS */

/* The below macros handle endian mis-matches between wl utility and wl driver. */
#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_BCM7) || !defined(RTCONFIG_BCMWL6)
static bool g_swap = FALSE;
#ifndef htod16
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#endif
#ifndef htod32
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#endif
#ifndef dtoh16
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#endif
#ifndef dtoh32
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#endif
#ifndef dtohchanspec
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#endif
#endif

#define SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */
#define	MAX_STA_COUNT	128

#define CHANIMSTR(a, b, c, d) ((a) ? ((b) ? c : d) : "")

struct apinfo apinfos[MAX_NUMBER_OF_APINFO];
char buf[WLC_IOCTL_MAXLEN];
static char scan_result[WLC_SCAN_RESULT_BUF_LEN];

/* Helper routine to print the infrastructure mode while pretty printing the BSS list */
#if 0
static const char *
capmode2str(uint16 capability)
{
	capability &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);

	if (capability == DOT11_CAP_ESS)
		return "Managed";
	else if (capability == DOT11_CAP_IBSS)
		return "Ad Hoc";
	else
		return "<unknown>";
}
#endif
int
dump_rateset(int eid, webs_t wp, int argc, char_t **argv, uint8 *rates, uint count)
{
	uint i;
	uint r;
	bool b;
	int retval = 0;

	retval += websWrite(wp, "[ ");
	for (i = 0; i < count; i++) {
		r = rates[i] & 0x7f;
		b = rates[i] & 0x80;
		if (r == 0)
			break;
		retval += websWrite(wp, "%d%s%s ", (r / 2), (r % 2)?".5":"", b?"(b)":"");
	}
	retval += websWrite(wp, "]");

	return retval;
}

#ifdef RTCONFIG_BCMWL6

/* Definitions for D11AC capable Chanspec type */

/* Chanspec ASCII representation with 802.11ac capability:
 * [<band> 'g'] <channel> ['/'<bandwidth> [<ctl-sideband>]['/'<1st80channel>'-'<2nd80channel>]]
 *
 * <band>:
 *      (optional) 2, 3, 4, 5 for 2.4GHz, 3GHz, 4GHz, and 5GHz respectively.
 *      Default value is 2g if channel <= 14, otherwise 5g.
 * <channel>:
 *      channel number of the 5MHz, 10MHz, 20MHz channel,
 *      or primary channel of 40MHz, 80MHz, 160MHz, or 80+80MHz channel.
 * <bandwidth>:
 *      (optional) 5, 10, 20, 40, 80, 160, or 80+80. Default value is 20.
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
 * <1st80Channel>:
 * <2nd80Channel>:
 *      Required for 80+80, otherwise not allowed.
 *      Specifies the center channel of the first and second 80MHz band.
 *
 * In its simplest form, it is a 20MHz channel number, with the implied band
 * of 2.4GHz if channel number <= 14, and 5GHz otherwise.
 *
 * To allow for backward compatibility with scripts, the old form for
 * 40MHz channels is also allowed: <channel><ctl-sideband>
 *
 * <channel>:
 *      primary channel of 40MHz, channel <= 14 is 2GHz, otherwise 5GHz
 * <ctl-sideband>:
 *      "U" for upper, "L" for lower (or lower case "u" "l")
 *
 * 5 GHz Examples:
 *      Chanspec        BW        Center Ch  Channel Range  Primary Ch
 *      5g8             20MHz     8          -              -
 *      52              20MHz     52         -              -
 *      52/40           40MHz     54         52-56          52
 *      56/40           40MHz     54         52-56          56
 *      52/80           80MHz     58         52-64          52
 *      56/80           80MHz     58         52-64          56
 *      60/80           80MHz     58         52-64          60
 *      64/80           80MHz     58         52-64          64
 *      52/160          160MHz    50         36-64          52
 *      36/160          160MGz    50         36-64          36
 *      36/80+80/42-106 80+80MHz  42,106     36-48,100-112  36
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
	"5",
	"10",
	"20",
	"40",
	"80",
	"160",
	"80+80",
	"na"
};

static const uint8 wf_chspec_bw_mhz[] =
{5, 10, 20, 40, 80, 160, 160};

#define WF_NUM_BW \
	(sizeof(wf_chspec_bw_mhz)/sizeof(uint8))

/* 40MHz channels in 5GHz band */
static const uint8 wf_5g_40m_chans[] =
{38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159};
#define WF_NUM_5G_40M_CHANS \
	(sizeof(wf_5g_40m_chans)/sizeof(uint8))

/* 80MHz channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] =
{42, 58, 106, 122, 138, 155};
#define WF_NUM_5G_80M_CHANS \
	(sizeof(wf_5g_80m_chans)/sizeof(uint8))

/* 160MHz channels in 5GHz band */
static const uint8 wf_5g_160m_chans[] =
{50, 114};
#define WF_NUM_5G_160M_CHANS \
	(sizeof(wf_5g_160m_chans)/sizeof(uint8))

/* convert bandwidth from chanspec to MHz */
static uint
bw_chspec_to_mhz(chanspec_t chspec)
{
	uint bw;

	bw = (chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT;
	return (bw >= WF_NUM_BW ? 0 : wf_chspec_bw_mhz[bw]);
}

/* bw in MHz, return the channel count from the center channel to the
 * the channel at the edge of the band
 */
static uint8
center_chan_to_edge(uint bw)
{
	/* edge channels separated by BW - 10MHz on each side
	 * delta from cf to edge is half of that,
	 * MHz to channel num conversion is 5MHz/channel
	 */
	return (uint8)(((bw - 20) / 2) / 5);
}

/* return channel number of the low edge of the band
 * given the center channel and BW
 */
static uint8
channel_low_edge(uint center_ch, uint bw)
{
	return (uint8)(center_ch - center_chan_to_edge(bw));
}

/* return control channel given center channel and side band */
static uint8
channel_to_ctl_chan(uint center_ch, uint bw, uint sb)
{
	return (uint8)(channel_low_edge(center_ch, bw) + sb * 4);
}

/*
 * Verify the chanspec is using a legal set of parameters, i.e. that the
 * chanspec specified a band, bw, ctl_sb and channel and that the
 * combination could be legal given any set of circumstances.
 * RETURNS: TRUE is the chanspec is malformed, false if it looks good.
 */
bool
wf_chspec_malformed(chanspec_t chanspec)
{
	uint chspec_bw = CHSPEC_BW(chanspec);
	uint chspec_ch = CHSPEC_CHANNEL(chanspec);

	/* must be 2G or 5G band */
	if (CHSPEC_IS2G(chanspec)) {
		/* must be valid bandwidth */
		if (chspec_bw != WL_CHANSPEC_BW_20 &&
		    chspec_bw != WL_CHANSPEC_BW_40) {
			return TRUE;
		}
	} else if (CHSPEC_IS5G(chanspec)) {
		if (chspec_bw == WL_CHANSPEC_BW_8080) {
			uint ch1_id, ch2_id;

			/* channel number in 80+80 must be in range */
			ch1_id = CHSPEC_CHAN1(chanspec);
			ch2_id = CHSPEC_CHAN2(chanspec);
			if (ch1_id >= WF_NUM_5G_80M_CHANS || ch2_id >= WF_NUM_5G_80M_CHANS)
				return TRUE;

			/* ch2 must be above ch1 for the chanspec */
			if (ch2_id <= ch1_id)
				return TRUE;
		} else if (chspec_bw == WL_CHANSPEC_BW_20 || chspec_bw == WL_CHANSPEC_BW_40 ||
			   chspec_bw == WL_CHANSPEC_BW_80 || chspec_bw == WL_CHANSPEC_BW_160) {

			if (chspec_ch > MAXCHANNEL) {
				return TRUE;
			}
		} else {
			/* invalid bandwidth */
			return TRUE;
		}
	} else {
		/* must be 2G or 5G band */
		return TRUE;
	}
	/* side band needs to be consistent with bandwidth */
	if (chspec_bw == WL_CHANSPEC_BW_20) {
		if (CHSPEC_CTL_SB(chanspec) != WL_CHANSPEC_CTL_SB_LLL)
			return TRUE;
	} else if (chspec_bw == WL_CHANSPEC_BW_40) {
		if (CHSPEC_CTL_SB(chanspec) > WL_CHANSPEC_CTL_SB_LLU)
			return TRUE;
	} else if (chspec_bw == WL_CHANSPEC_BW_80) {
		if (CHSPEC_CTL_SB(chanspec) > WL_CHANSPEC_CTL_SB_LUU)
			return TRUE;
	}

	return FALSE;
}

/*
 * This function returns the channel number that control traffic is being sent on, for 20MHz
 * channels this is just the channel number, for 40MHZ, 80MHz, 160MHz channels it is the 20MHZ
 * sideband depending on the chanspec selected
 */
uint8
wf_chspec_ctlchan(chanspec_t chspec)
{
	uint center_chan;
	uint bw_mhz;
	uint sb;

	if (wf_chspec_malformed(chspec))
		return 0;

	/* Is there a sideband ? */
	if (CHSPEC_IS20(chspec)) {
		return CHSPEC_CHANNEL(chspec);
	} else {
		sb = CHSPEC_CTL_SB(chspec) >> WL_CHANSPEC_CTL_SB_SHIFT;

		if (CHSPEC_IS8080(chspec)) {
			bw_mhz = 80;

			if (sb < 4) {
				center_chan = CHSPEC_CHAN1(chspec);
			}
			else {
				center_chan = CHSPEC_CHAN2(chspec);
				sb -= 4;
			}

			/* convert from channel index to channel number */
			center_chan = wf_5g_80m_chans[center_chan];
		}
		else {
			bw_mhz = bw_chspec_to_mhz(chspec);
			center_chan = CHSPEC_CHANNEL(chspec) >> WL_CHANSPEC_CHAN_SHIFT;
		}

		return (channel_to_ctl_chan(center_chan, bw_mhz, sb));
	}
}

/* given a chanspec and a string buffer, format the chanspec as a
 * string, and return the original pointer a.
 * Min buffer length must be CHANSPEC_STR_LEN.
 * On error return ""
 */
char *
wf_chspec_ntoa(chanspec_t chspec, char *buf)
{
	const char *band;
	uint ctl_chan;

	if (wf_chspec_malformed(chspec))
		return "";

	band = "";

	/* check for non-default band spec */
	if ((CHSPEC_IS2G(chspec) && CHSPEC_CHANNEL(chspec) > CH_MAX_2G_CHANNEL) ||
	    (CHSPEC_IS5G(chspec) && CHSPEC_CHANNEL(chspec) <= CH_MAX_2G_CHANNEL))
		band = (CHSPEC_IS2G(chspec)) ? "2g" : "5g";

	/* ctl channel */
	if (!(ctl_chan = wf_chspec_ctlchan(chspec)))
		return "";

	/* bandwidth and ctl sideband */
	if (CHSPEC_IS20(chspec)) {
		snprintf(buf, CHANSPEC_STR_LEN, "%s%d", band, ctl_chan);
	} else if (!CHSPEC_IS8080(chspec)) {
		const char *bw;
		const char *sb = "";

		bw = wf_chspec_bw_str[(chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT];

#ifdef CHANSPEC_NEW_40MHZ_FORMAT
		/* ctl sideband string if needed for 2g 40MHz */
		if (CHSPEC_IS40(chspec) && CHSPEC_IS2G(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
		}

		snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s%s", band, ctl_chan, bw, sb);
#else
		/* ctl sideband string instead of BW for 40MHz */
		if (CHSPEC_IS40(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d%s", band, ctl_chan, sb);
		} else {
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s", band, ctl_chan, bw);
		}
#endif /* CHANSPEC_NEW_40MHZ_FORMAT */
	} else {
		/* 80+80 */
		uint chan1 = (chspec & WL_CHANSPEC_CHAN1_MASK) >> WL_CHANSPEC_CHAN1_SHIFT;
		uint chan2 = (chspec & WL_CHANSPEC_CHAN2_MASK) >> WL_CHANSPEC_CHAN2_SHIFT;

		/* convert to channel number */
		chan1 = (chan1 < WF_NUM_5G_80M_CHANS) ? wf_5g_80m_chans[chan1] : 0;
		chan2 = (chan2 < WF_NUM_5G_80M_CHANS) ? wf_5g_80m_chans[chan2] : 0;

		/* Outputs a max of CHANSPEC_STR_LEN chars including '\0'  */
		snprintf(buf, CHANSPEC_STR_LEN, "%d/80+80/%d-%d", ctl_chan, chan1, chan2);
	}

	return (buf);
}

#else

/* given a chanspec and a string buffer, format the chanspec as a
 * string, and return the original pointer a.
 * Min buffer length must be CHANSPEC_STR_LEN.
 * On error return NULL
 */
char *
wf_chspec_ntoa(chanspec_t chspec, char *buf)
{
	const char *band, *bw, *sb;
	uint channel;

	band = "";
	bw = "";
	sb = "";
	channel = CHSPEC_CHANNEL(chspec);
	/* check for non-default band spec */
	if ((CHSPEC_IS2G(chspec) && channel > CH_MAX_2G_CHANNEL) ||
	    (CHSPEC_IS5G(chspec) && channel <= CH_MAX_2G_CHANNEL))
		band = (CHSPEC_IS2G(chspec)) ? "b" : "a";
	if (CHSPEC_IS40(chspec)) {
		if (CHSPEC_SB_UPPER(chspec)) {
			sb = "u";
			channel += CH_10MHZ_APART;
		} else {
			sb = "l";
			channel -= CH_10MHZ_APART;
		}
	} else if (CHSPEC_IS10(chspec)) {
		bw = "n";
	}

	/* Outputs a max of 6 chars including '\0'  */
	snprintf(buf, 6, "%d%s%s%s", channel, band, bw, sb);
	return (buf);
}

#endif

static int
wlu_bcmp(const void *b1, const void *b2, int len)
{
	return (memcmp(b1, b2, len));
}

/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
static uint8 *
wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
	uint8 *cp;
	int totlen;

	cp = tlv_buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= 2) {
		uint tag;
		int len;

		tag = *cp;
		len = *(cp +1);

		/* validate remaining totlen */
		if ((tag == key) && (totlen >= (len + 2)))
			return (cp);

		cp += (len + 2);
		totlen -= (len + 2);
	}

	return NULL;
}

/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
static bool
wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
{
	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && !wlu_bcmp(&ie[2], WPA_OUI "\x01", 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

/* Validates and parses the RSN or WPA IE contents into a rsn_parse_info_t structure
 * Returns 0 on success, or 1 if the information in the buffer is not consistant with
 * an RSN IE or WPA IE.
 * The buf pointer passed in should be pointing at the version field in either an RSN IE
 * or WPA IE.
 */
static int
wl_rsn_ie_parse_info(uint8* rsn_buf, uint len, rsn_parse_info_t *rsn)
{
	uint16 count;

	memset(rsn, 0, sizeof(rsn_parse_info_t));

	/* version */
	if (len < sizeof(uint16))
		return 1;

	rsn->version = ltoh16_ua(rsn_buf);
	len -= sizeof(uint16);
	rsn_buf += sizeof(uint16);

	/* Multicast Suite */
	if (len < sizeof(wpa_suite_mcast_t))
		return 0;

	rsn->mcast = (wpa_suite_mcast_t*)rsn_buf;
	len -= sizeof(wpa_suite_mcast_t);
	rsn_buf += sizeof(wpa_suite_mcast_t);

	/* Unicast Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->ucast = (wpa_suite_ucast_t*)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* AKM Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->akm = (wpa_suite_auth_key_mgmt_t*)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* Capabilites */
	if (len < sizeof(uint16))
		return 0;

	rsn->capabilities = rsn_buf;

	return 0;
}

static uint
wl_rsn_ie_decode_cntrs(uint cntr_field)
{
	uint cntrs;

	switch (cntr_field) {
	case RSN_CAP_1_REPLAY_CNTR:
		cntrs = 1;
		break;
	case RSN_CAP_2_REPLAY_CNTRS:
		cntrs = 2;
		break;
	case RSN_CAP_4_REPLAY_CNTRS:
		cntrs = 4;
		break;
	case RSN_CAP_16_REPLAY_CNTRS:
		cntrs = 16;
		break;
	default:
		cntrs = 0;
		break;
	}

	return cntrs;
}

static int
wl_rsn_ie_dump(int eid, webs_t wp, int argc, char_t **argv, bcm_tlv_t *ie)
{
	int i;
	int rsn;
	wpa_ie_fixed_t *wpa = NULL;
	rsn_parse_info_t rsn_info;
	wpa_suite_t *suite;
	uint8 std_oui[3];
	int unicast_count = 0;
	int akm_count = 0;
	uint16 capabilities;
	uint cntrs;
	int err;
	int retval = 0;

	if (ie->id == DOT11_MNG_RSN_ID) {
		rsn = TRUE;
		memcpy(std_oui, WPA2_OUI, WPA_OUI_LEN);
		err = wl_rsn_ie_parse_info(ie->data, ie->len, &rsn_info);
	} else {
		rsn = FALSE;
		memcpy(std_oui, WPA_OUI, WPA_OUI_LEN);
		wpa = (wpa_ie_fixed_t*)ie;
		err = wl_rsn_ie_parse_info((uint8*)&wpa->version, wpa->length - WPA_IE_OUITYPE_LEN,
					   &rsn_info);
	}
	if (err || rsn_info.version != WPA_VERSION)
		return retval;

	if (rsn)
		retval += websWrite(wp, "RSN:\n");
	else
		retval += websWrite(wp, "WPA:\n");

	/* Check for multicast suite */
	if (rsn_info.mcast) {
		retval += websWrite(wp, "\tmulticast cipher: ");
		if (!wlu_bcmp(rsn_info.mcast->oui, std_oui, 3)) {
			switch (rsn_info.mcast->type) {
			case WPA_CIPHER_NONE:
				retval += websWrite(wp, "NONE\n");
				break;
			case WPA_CIPHER_WEP_40:
				retval += websWrite(wp, "WEP64\n");
				break;
			case WPA_CIPHER_WEP_104:
				retval += websWrite(wp, "WEP128\n");
				break;
			case WPA_CIPHER_TKIP:
				retval += websWrite(wp, "TKIP\n");
				break;
			case WPA_CIPHER_AES_OCB:
				retval += websWrite(wp, "AES-OCB\n");
				break;
			case WPA_CIPHER_AES_CCM:
				retval += websWrite(wp, "AES-CCMP\n");
				break;
			default:
				retval += websWrite(wp, "Unknown-%s(#%d)\n", rsn ? "RSN" : "WPA",
				       rsn_info.mcast->type);
				break;
			}
		}
		else {
			retval += websWrite(wp, "Unknown-%02X:%02X:%02X(#%d) ",
			       rsn_info.mcast->oui[0], rsn_info.mcast->oui[1],
			       rsn_info.mcast->oui[2], rsn_info.mcast->type);
		}
	}

	/* Check for unicast suite(s) */
	if (rsn_info.ucast) {
		unicast_count = ltoh16_ua(&rsn_info.ucast->count);
		retval += websWrite(wp, "\tunicast ciphers(%d): ", unicast_count);
		for (i = 0; i < unicast_count; i++) {
			suite = &rsn_info.ucast->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				switch (suite->type) {
				case WPA_CIPHER_NONE:
					retval += websWrite(wp, "NONE ");
					break;
				case WPA_CIPHER_WEP_40:
					retval += websWrite(wp, "WEP64 ");
					break;
				case WPA_CIPHER_WEP_104:
					retval += websWrite(wp, "WEP128 ");
					break;
				case WPA_CIPHER_TKIP:
					retval += websWrite(wp, "TKIP ");
					break;
				case WPA_CIPHER_AES_OCB:
					retval += websWrite(wp, "AES-OCB ");
					break;
				case WPA_CIPHER_AES_CCM:
					retval += websWrite(wp, "AES-CCMP ");
					break;
				default:
					retval += websWrite(wp, "WPA-Unknown-%s(#%d) ", rsn ? "RSN" : "WPA",
					       suite->type);
					break;
				}
			}
			else {
				retval += websWrite(wp, "Unknown-%02X:%02X:%02X(#%d) ",
					suite->oui[0], suite->oui[1], suite->oui[2],
					suite->type);
			}
		}
		retval += websWrite(wp, "\n");
	}
	/* Authentication Key Management */
	if (rsn_info.akm) {
		akm_count = ltoh16_ua(&rsn_info.akm->count);
		retval += websWrite(wp, "\tAKM Suites(%d): ", akm_count);
		for (i = 0; i < akm_count; i++) {
			suite = &rsn_info.akm->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				switch (suite->type) {
				case RSN_AKM_NONE:
					retval += websWrite(wp, "None ");
					break;
				case RSN_AKM_UNSPECIFIED:
					retval += websWrite(wp, "WPA ");
					break;
				case RSN_AKM_PSK:
					retval += websWrite(wp, "WPA-PSK ");
					break;
				default:
					retval += websWrite(wp, "Unknown-%s(#%d)  ",
					       rsn ? "RSN" : "WPA", suite->type);
					break;
				}
			}
			else {
				retval += websWrite(wp, "Unknown-%02X:%02X:%02X(#%d)  ",
					suite->oui[0], suite->oui[1], suite->oui[2],
					suite->type);
			}
		}
		retval += websWrite(wp, "\n");
	}

	/* Capabilities */
	if (rsn_info.capabilities) {
		capabilities = ltoh16_ua(rsn_info.capabilities);
		retval += websWrite(wp, "\tCapabilities(0x%04x): ", capabilities);
		if (rsn)
			retval += websWrite(wp, "%sPre-Auth, ", (capabilities & RSN_CAP_PREAUTH) ? "" : "No ");

		retval += websWrite(wp, "%sPairwise, ", (capabilities & RSN_CAP_NOPAIRWISE) ? "No " : "");

		cntrs = wl_rsn_ie_decode_cntrs((capabilities & RSN_CAP_PTK_REPLAY_CNTR_MASK) >>
					       RSN_CAP_PTK_REPLAY_CNTR_SHIFT);

		retval += websWrite(wp, "%d PTK Replay Ctr%s", cntrs, (cntrs > 1)?"s":"");

		if (rsn) {
			cntrs = wl_rsn_ie_decode_cntrs(
				(capabilities & RSN_CAP_GTK_REPLAY_CNTR_MASK) >>
				RSN_CAP_GTK_REPLAY_CNTR_SHIFT);

			retval += websWrite(wp, "%d GTK Replay Ctr%s\n", cntrs, (cntrs > 1)?"s":"");
		} else {
			retval += websWrite(wp, "\n");
		}
	} else {
		retval += websWrite(wp, "\tNo %s Capabilities advertised\n", rsn ? "RSN" : "WPA");
	}

	return retval;
}

static int
wl_dump_wpa_rsn_ies(int eid, webs_t wp, int argc, char_t **argv, uint8* cp, uint len)
{
	uint8 *parse = cp;
	uint parse_len = len;
	uint8 *wpaie;
	uint8 *rsnie;
	int retval = 0;

	while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
		if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
			break;
	if (wpaie)
		retval += wl_rsn_ie_dump(eid, wp, argc, argv, (bcm_tlv_t*)wpaie);

	rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
	if (rsnie)
		retval += wl_rsn_ie_dump(eid, wp, argc, argv, (bcm_tlv_t*)rsnie);

	return retval;
}

int
wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len)
{
	int i, c;
	char *p = ssid_buf;

	if (ssid_len > 32) ssid_len = 32;

	for (i = 0; i < ssid_len; i++) {
		c = (int)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
//			*p++ = '\\';
		} else if (isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += sprintf(p, "\\x%02X", c);
		}
	}
	*p = '\0';

	return p - ssid_buf;
}

#ifdef RTCONFIG_BCMWL6
static int
bcm_wps_version(webs_t wp, uint8 *wps_ie)
{
	uint16 wps_len;
	uint16 wps_off, wps_suboff;
	uint16 wps_key;
	uint8 wps_field_len;
	int retval = 0;

	wps_len = (uint16)*(wps_ie+TLV_LEN_OFF);/* Get the length of the WPS OUI header */
	wps_off = WPS_OUI_FIXED_HEADER_OFF;	/* Skip the fixed headers */
	wps_field_len = 1;

	/* Parsing the OUI header looking for version number */
	while ((wps_len >= wps_off + 2) && (wps_field_len))
	{
		wps_key = (((uint8)wps_ie[wps_off]*256) + (uint8)wps_ie[wps_off+1]);
		if (wps_key == WPS_ID_VENDOR_EXT) {
			/* Key found */
			wps_suboff = wps_off + WPS_OUI_HEADER_SIZE;

			/* Looking for the Vendor extension code 0x00 0x37 0x2A
			 * and the Version 2 sudId 0x00
			 * if found then the next byte is the len of field which is always 1
			 * for version field the byte after is the version number
			 */
			if (!wlu_bcmp(&wps_ie[wps_suboff],  WFA_VENDOR_EXT_ID, WPS_OUI_LEN)&&
				(wps_ie[wps_suboff+WPS_WFA_SUBID_V2_OFF] == WPS_WFA_SUBID_VERSION2))
			{
				retval += websWrite(wp, "V%d.%d ", (wps_ie[wps_suboff+WPS_WFA_V2_OFF]>>4),
				(wps_ie[wps_suboff+WPS_WFA_V2_OFF] & 0x0f));
				return retval;
			}
		}
		/* Jump to next field */
		wps_field_len = wps_ie[wps_off+WPS_OUI_HEADER_LEN+1];
		wps_off += WPS_OUI_HEADER_SIZE + wps_field_len;
	}

	/* If nothing found from the parser then this is the WPS version 1.0 */
	retval += websWrite(wp, "V1.0 ");

	return retval;
}

static int
bcm_is_wps_configured(webs_t wp, uint8 *wps_ie)
{
	uint16 wps_key;
	int retval = 0;

	wps_key = (wps_ie[WPS_SCSTATE_OFF]*256) + wps_ie[WPS_SCSTATE_OFF+1];
	if ((wps_ie[TLV_LEN_OFF] > (WPS_SCSTATE_OFF+5))&&
		(wps_key == WPS_ID_SC_STATE))
	{
		switch (wps_ie[WPS_SCSTATE_OFF+WPS_OUI_HEADER_SIZE])
		{
			case WPS_SCSTATE_UNCONFIGURED:
				retval += websWrite(wp, "Unconfigured\n");
				break;
			case WPS_SCSTATE_CONFIGURED:
				retval += websWrite(wp, "Configured\n");
				break;
			default:
				retval += websWrite(wp, "Unknown State\n");
		}
	}
	return retval;
}

/* Looking for WPS OUI in the propriatary_ie */
static bool
bcm_is_wps_ie(uint8 *ie, uint8 **tlvs, uint32 *tlvs_len)
{
	bool retval = FALSE;
	/* If the contents match the WPS_OUI and type=4 */
	if ((ie[TLV_LEN_OFF] > (WPS_OUI_LEN+1)) &&
		!wlu_bcmp(&ie[TLV_BODY_OFF], WPS_OUI "\x04", WPS_OUI_LEN + 1)) {
		retval = TRUE;
	}

	/* point to the next ie */
	ie += ie[TLV_LEN_OFF] + TLV_HDR_LEN;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return retval;
}

static int
wl_dump_wps(webs_t wp, uint8* cp, uint len)
{
	uint8 *parse = cp;
	uint32 parse_len = len;
	uint8 *proprietary_ie;
	int retval = 0;

	while ((proprietary_ie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID))) {
		if (bcm_is_wps_ie(proprietary_ie, &parse, &parse_len)) {
			/* Print WPS status */
			retval += websWrite(wp, "WPS: ");
			/* Print the version get from vendor extension field */
			retval += bcm_wps_version(wp, proprietary_ie);
			/* Print the WPS configure or Unconfigure option */
			retval += bcm_is_wps_configured(wp, proprietary_ie);
			break;
		}
	}
	return retval;
}

static chanspec_t
wl_chspec_from_driver(chanspec_t chanspec)
{
	chanspec = dtohchanspec(chanspec);
	/*
	if (ioctl_version == 1) {
		chanspec = wl_chspec_from_legacy(chanspec);
	}
	*/
	return chanspec;
}

#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
#define VHT_PROP_MCS_MAP_NONE	3
#endif

#endif

static int
dump_bss_info(int eid, webs_t wp, int argc, char_t **argv, wl_bss_info_t *bi)
{
	char ssidbuf[SSID_FMT_BUF_LEN];
	char chspec_str[CHANSPEC_STR_LEN];
	wl_bss_info_107_t *old_bi;
	wl_bss_info_v109_1_t *new_bi = (wl_bss_info_v109_1_t *) bi;
#ifndef RTCONFIG_QTN
	int mcs_idx = 0;
#endif
	int retval = 0;

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
#ifdef RTCONFIG_BCMWL6
	} else {
		/* do endian swap and format conversion for chanspec if we have
		* not created it from legacy bi above
		*/
		bi->chanspec = wl_chspec_from_driver(bi->chanspec);
#endif
	}

	wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);

	retval += websWrite(wp, "SSID: \"%s\"\n", ssidbuf);

//	retval += websWrite(wp, "Mode: %s\t", capmode2str(dtoh16(bi->capability)));
	retval += websWrite(wp, "RSSI: %d dBm\t", (int16)(dtoh16(bi->RSSI)));

	/*
	 * SNR has valid value in only 109 version.
	 * So print SNR for 109 version only.
	 */
	if (dtoh32(bi->version) == WL_BSS_INFO_VERSION) {
		retval += websWrite(wp, "SNR: %d dB\t", (int16)(dtoh16(bi->SNR)));
	}

	retval += websWrite(wp, "noise: %d dBm\t", bi->phy_noise);
	if (bi->flags) {
		bi->flags = dtoh16(bi->flags);
		retval += websWrite(wp, "Flags: ");
		if (bi->flags & WL_BSS_FLAGS_FROM_BEACON) retval += websWrite(wp, "FromBcn ");
		if (bi->flags & WL_BSS_FLAGS_FROM_CACHE) retval += websWrite(wp, "Cached ");
		if (bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) retval += websWrite(wp, "RSSI on-channel ");
		retval += websWrite(wp, "\t");
	}
	retval += websWrite(wp, "Channel: %s\n", wf_chspec_ntoa(dtohchanspec(bi->chanspec), chspec_str));

	retval += websWrite(wp, "BSSID: %s\t", wl_ether_etoa(&bi->BSSID));

#ifndef RTCONFIG_QTN
	retval += websWrite(wp, "Capability: ");
	bi->capability = dtoh16(bi->capability);
	if (bi->capability & DOT11_CAP_ESS) retval += websWrite(wp, "ESS ");
	if (bi->capability & DOT11_CAP_IBSS) retval += websWrite(wp, "IBSS ");
	if (bi->capability & DOT11_CAP_POLLABLE) retval += websWrite(wp, "Pollable ");
	if (bi->capability & DOT11_CAP_POLL_RQ) retval += websWrite(wp, "PollReq ");
	if (bi->capability & DOT11_CAP_PRIVACY) retval += websWrite(wp, "WEP ");
	if (bi->capability & DOT11_CAP_SHORT) retval += websWrite(wp, "ShortPre ");
	if (bi->capability & DOT11_CAP_PBCC) retval += websWrite(wp, "PBCC ");
	if (bi->capability & DOT11_CAP_AGILITY) retval += websWrite(wp, "Agility ");
	if (bi->capability & DOT11_CAP_SHORTSLOT) retval += websWrite(wp, "ShortSlot ");
	if (bi->capability & DOT11_CAP_CCK_OFDM) retval += websWrite(wp, "CCK-OFDM ");
#endif
	retval += websWrite(wp, "\n");

	retval += websWrite(wp, "Supported Rates: ");
	retval += dump_rateset(eid, wp, argc, argv, bi->rateset.rates, dtoh32(bi->rateset.count));
	retval += websWrite(wp, "\n");
	if (dtoh32(bi->ie_length))
		retval += wl_dump_wpa_rsn_ies(eid, wp, argc, argv, (uint8 *)(((uint8 *)bi) + dtoh16(bi->ie_offset)),
				    dtoh32(bi->ie_length));
#ifndef RTCONFIG_QTN
	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
#ifdef RTCONFIG_BCMWL6
		if (bi->vht_cap)
			retval += websWrite(wp, "VHT Capable:\n");
		else
			retval += websWrite(wp, "HT Capable:\n");
		retval += websWrite(wp, "\tChanspec: %sGHz channel %d %dMHz (0x%x)\n",
			CHSPEC_IS2G(bi->chanspec)?"2.4":"5", CHSPEC_CHANNEL(bi->chanspec),
		       (CHSPEC_IS160(bi->chanspec) ?
			160 : CHSPEC_IS80(bi->chanspec) ?
				80 : (CHSPEC_IS40(bi->chanspec) ?
			      		40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10))),
			bi->chanspec);
		retval += websWrite(wp, "\tPrimary channel: %d\n", bi->ctl_ch);
		retval += websWrite(wp, "\tHT Capabilities: ");
#else
		retval += websWrite(wp, "802.11N Capable:\n");
		bi->chanspec = dtohchanspec(bi->chanspec);
		retval += websWrite(wp, "\tChanspec: %sGHz channel %d %dMHz (0x%x)\n",
			CHSPEC_IS2G(bi->chanspec)?"2.4":"5", CHSPEC_CHANNEL(bi->chanspec),
			CHSPEC_IS40(bi->chanspec) ? 40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10),
			bi->chanspec);
		retval += websWrite(wp, "\tControl channel: %d\n", bi->ctl_ch);
		retval += websWrite(wp, "\t802.11N Capabilities: ");
#endif
		if (dtoh32(bi->nbss_cap) & HT_CAP_40MHZ)
			retval += websWrite(wp, "40Mhz ");
		if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_20)
			retval += websWrite(wp, "SGI20 ");
		if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_40)
			retval += websWrite(wp, "SGI40 ");
		retval += websWrite(wp, "\n\tSupported MCS : [ ");
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++)
			if (isset(bi->basic_mcs, mcs_idx))
				retval += websWrite(wp, "%d ", mcs_idx);
		retval += websWrite(wp, "]\n");

#ifdef RTCONFIG_BCMWL6
		if (bi->vht_cap) {
			int i;
			uint mcs;
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
 			uint prop_mcs = VHT_PROP_MCS_MAP_NONE;
#endif
			retval += websWrite(wp, "\tVHT Capabilities: \n");
			retval += websWrite(wp, "\tSupported VHT (tx) Rates:\n");
			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i, dtoh16(bi->vht_txmcsmap));
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
				if (dtoh16(bi->length) >= (OFFSETOF(wl_bss_info_t,
					vht_txmcsmap_prop) +
					ROUNDUP(dtoh32(bi->ie_length), 4) +
					sizeof(uint16))) {
						prop_mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i,
						dtoh16(bi->vht_txmcsmap_prop));
				}
#endif
				if (mcs != VHT_CAP_MCS_MAP_NONE){
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
					if (prop_mcs != VHT_PROP_MCS_MAP_NONE)
						retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", i,
							(mcs == VHT_CAP_MCS_MAP_0_9 ? "0-11" :
							(mcs == VHT_CAP_MCS_MAP_0_8 ? "0-8, 10-11" : "0-7, 10-11")));
					else
#endif
					retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", i,
						(mcs == VHT_CAP_MCS_MAP_0_9 ? "0-9" :
						(mcs == VHT_CAP_MCS_MAP_0_8 ? "0-8" : "0-7")));
				}
			}
			retval += websWrite(wp, "\tSupported VHT (rx) Rates:\n");
			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i, dtoh16(bi->vht_rxmcsmap));
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
				if (dtoh16(bi->length) >= (OFFSETOF(wl_bss_info_t,
					vht_txmcsmap_prop) +
					ROUNDUP(dtoh32(bi->ie_length), 4) +
					sizeof(uint16))) {
						prop_mcs = VHT_MCS_MAP_GET_MCS_PER_SS(i,
						dtoh16(bi->vht_txmcsmap_prop));
				}
#endif

				if (mcs != VHT_CAP_MCS_MAP_NONE) {
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
					if (prop_mcs != VHT_PROP_MCS_MAP_NONE)
						retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", i,
							(mcs == VHT_CAP_MCS_MAP_0_9 ? "0-11" :
							(mcs == VHT_CAP_MCS_MAP_0_8 ? "0-8, 10-11" : "0-7, 10-11")));
					else
#endif
					retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", i,
						(mcs == VHT_CAP_MCS_MAP_0_9 ? "0-9" :
						(mcs == VHT_CAP_MCS_MAP_0_8 ? "0-8" : "0-7")));
				}
			}
		}
#ifdef RTCONFIG_HND_ROUTER_AX
		if (new_bi->he_cap) {
			int i, nss, neg = 0;
			static const char zero[sizeof(uint16) * WL_HE_CAP_MCS_MAP_NSS_MAX] = { 0 };
			uint16 *he_mcsmap;
			uint16 he_txmcsmap, he_rxmcsmap;
			char *mcs_str, *bw_str;
			uint rx_mcs, tx_mcs;

			retval += websWrite(wp, "\tHE Capabilities: \n");
			if (new_bi->he_neg_bw80_tx_mcs != 0xffff) {
				he_mcsmap = &new_bi->he_neg_bw80_tx_mcs;
				neg = 1;
			} else {
				he_mcsmap = &new_bi->he_sup_bw80_tx_mcs;
			}

			if (he_mcsmap == NULL || !memcmp(he_mcsmap, zero, sizeof(uint16) * WL_HE_CAP_MCS_MAP_NSS_MAX)) {
				goto SKIP;
			}

			if(neg)
				retval += websWrite(wp, "\tNegotiated HE (tx) Rates:\n");
			else
				retval += websWrite(wp, "\tSupported HE (tx) Rates:\n");
			for (i = 0; i < 3; i++) {
				if (i == 0) {
					bw_str = "80 Mhz";
				} else if (i == 1) {
					bw_str = "160 Mhz";
				} else {
					bw_str = "80+80 Mhz";
				}

				/* get he bw80, bw160, bw80p80 tx mcs from mcsset[0], mcsset[2], and mcsset[4] */
				he_txmcsmap = dtoh16(he_mcsmap[i * 2]);

				for (nss = 1; nss <= HE_CAP_MCS_MAP_NSS_MAX; nss++) {
					tx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(nss, he_txmcsmap);
					mcs_str =
						(tx_mcs == HE_CAP_MAX_MCS_0_11 ? "0-11" :
						(tx_mcs == HE_CAP_MAX_MCS_0_9 ? "0-9" :
						(tx_mcs == HE_CAP_MAX_MCS_0_7 ? "0-7" :
						"---")));
					if ((tx_mcs != HE_CAP_MAX_MCS_NONE)) {
						if (nss == 1)
							retval += websWrite(wp, "\t    %s:\n", bw_str);
						retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", nss, mcs_str);
					}
				}
			}

			if(neg)
				retval += websWrite(wp, "\tNegotiated HE (rx) Rates:\n");
			else
				retval += websWrite(wp, "\tSupported HE (rx) Rates:\n");
			for (i = 0; i < 3; i++) {
				if (i == 0) {
					bw_str = "80 Mhz";
				} else if (i == 1) {
					bw_str = "160 Mhz";
				} else {
					bw_str = "80+80 Mhz";
				}

				/* get he bw80, bw160, bw80p80 rx mcs from mcsset[1], mcsset[3], and mcsset[5] */
				he_rxmcsmap = dtoh16(he_mcsmap[(i * 2) + 1]);

				for (nss = 1; nss <= HE_CAP_MCS_MAP_NSS_MAX; nss++) {
					rx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(nss, he_rxmcsmap);
					mcs_str =
						(rx_mcs == HE_CAP_MAX_MCS_0_11 ? "0-11" :
						(rx_mcs == HE_CAP_MAX_MCS_0_9 ? "0-9" :
						(rx_mcs == HE_CAP_MAX_MCS_0_7 ? "0-7" :
						"---")));
					if ((rx_mcs != HE_CAP_MAX_MCS_NONE)) {
						if (nss == 1)
							retval += websWrite(wp, "\t    %s:\n", bw_str);
						retval += websWrite(wp, "\t\tNSS: %d MCS: %s\n", nss, mcs_str);
					}
				}
			}
SKIP:
		;
		}//he_cap
#endif	//AX
#endif
	}
#endif

#ifdef RTCONFIG_BCMWL6
	if (dtoh32(bi->ie_length))
	{
		retval += wl_dump_wps(wp, (uint8 *)(((uint8 *)bi) + dtoh16(bi->ie_offset)),
			dtoh32(bi->ie_length));
	}
#endif

	retval += websWrite(wp, "\n");

	return retval;
}

#if (WL_STA_VER >= 5)
#ifndef RATESET_ARGS_V1
#define RATESET_ARGS_V1		(1)
#endif

typedef union wl_rateset_args_u {
	wl_rateset_args_t rsv1;
#if (WL_STA_VER >= 7)
	wl_rateset_args_v2_t rsv2;
#endif
} wl_rateset_args_u_t;

/* get buffer for smaller sizes upto 256 bytes */
int
wlu_var_getbuf_sm(int unit, const char *iovar, void *param, int param_len, void **bufptr)
{
	char buf[WLC_IOCTL_SMLEN];
	int len;
	char ifname[NVRAM_BUFSIZE];

	memset(buf, 0, WLC_IOCTL_SMLEN);
	strcpy(buf, iovar);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len)
		memcpy(&buf[len], param, param_len);

	*bufptr = buf;
	wl_ifname(unit, 0, ifname);
	return wl_ioctl(ifname, WLC_GET_VAR, &buf[0], WLC_IOCTL_SMLEN);
}

#if (WL_STA_VER >= 7)
static int
wlu_var_getbuf_param_len(int unit, const char *iovar, void *param, int param_len, void **bufptr)
{
	char buf[WLC_IOCTL_MAXLEN];
	int len;
	char ifname[NVRAM_BUFSIZE];

	memset(buf, 0, param_len);
	strcpy(buf, iovar);

	/* include the null */
	len = strlen(iovar) + 1;

	if (param_len) {
		memcpy(&buf[len], param, param_len);
		*bufptr = buf;
		wl_ifname(unit, 0, ifname);
		return wl_ioctl(ifname, WLC_GET_VAR, &buf[0], len+param_len);
	}
	return (BCME_BADARG);
}
#endif

int
wl_get_rateset_args_info(int unit, int *rs_len, int *rs_ver)
{
	int err = 0;
#if (WL_STA_VER >= 7)
	wl_wlc_version_t *ver;
	struct wl_rateset_args_v2 wlrs;
	struct wl_rateset_args_v2 *wlrs2;
#endif
	void *ptr;

	/* first query wlc version. */
	err = wlu_var_getbuf_sm(unit, "wlc_ver", NULL, 0, &ptr);
	if (err == BCME_OK) {
#if (WL_STA_VER >= 7)
		ver = ptr;
		/* rateset args query is available from wlc_ver_major >= 9 */
		if ((ver->wlc_ver_major >= 9)) {
			/* Now, query the wl_rateset_args_t version, by giving version=0 and
			 * length as 4 bytes ssizeof(int32).
			 */
			memset(&wlrs, 0, sizeof(wlrs));

			err = wlu_var_getbuf_param_len(unit, "rateset", &wlrs, sizeof(int32),
				(void *)&wlrs2);

			if (err == BCME_OK) {
				*rs_ver = wlrs2->version;

				switch (*rs_ver) {
				case RATESET_ARGS_V2:
					*rs_len = sizeof(struct wl_rateset_args_v2);
					break;
				/* add new length returning here */
				default:
					*rs_len = 0; /* ERROR */
					err = BCME_UNSUPPORTED;
				}
			}
		} else
#endif
		{
			*rs_ver = RATESET_ARGS_V1;
#if (WL_STA_VER >= 7)
			*rs_len = sizeof(struct wl_rateset_args_v1);
#else
			*rs_len = sizeof(struct wl_rateset_args);
#endif
		}
	} else {
		/* for old branches which doesn't even support wlc_ver */
		*rs_ver = RATESET_ARGS_V1;
#if (WL_STA_VER >= 7)
		*rs_len = sizeof(struct wl_rateset_args_v1);
#else
		*rs_len = sizeof(struct wl_rateset_args);
#endif
		err = BCME_OK;
	}
	return err;
}

void
wl_rateset_get_fields(wl_rateset_args_u_t* rs, int rsver, uint32 **rscount, uint8 **rsrates,
	uint8 **rsmcs, uint16 **rsvht_mcs, uint16 **rshe_mcs)
{
	switch (rsver) {
		case RATESET_ARGS_V1:
			if (rscount)
				*rscount = &rs->rsv1.count;
			if (rsrates)
				*rsrates = rs->rsv1.rates;
			if (rsmcs)
				*rsmcs = rs->rsv1.mcs;
			if (rsvht_mcs)
				*rsvht_mcs = rs->rsv1.vht_mcs;
			break;
#if (WL_STA_VER >= 7)
		case RATESET_ARGS_V2:
			if (rscount)
				*rscount = &rs->rsv2.count;
			if (rsrates)
				*rsrates = rs->rsv2.rates;
			if (rsmcs)
				*rsmcs = rs->rsv2.mcs;
			if (rsvht_mcs)
				*rsvht_mcs = rs->rsv2.vht_mcs;
			if (rshe_mcs)
				*rshe_mcs = rs->rsv2.he_mcs;
			break;
#endif
		/* add new length returning here */
		default:
			/* nothing needed here */
			break;
	}
}

int
wl_ht_nss(char *mcsset)
{
	int i;
	int nss = 0;

	for (i = 0; i < (MCSSET_LEN * 8); i++) {
		if (isset(mcsset, i)) {
			if ((i % 8) == 0)
				nss++;
		}
	}

	return nss;
}

int
wl_vht_nss(uint16 *mcsset)
{
	int i;
	int nss = 0;

	for (i = 0; i < VHT_CAP_MCS_MAP_NSS_MAX; i++) {
		if (mcsset[i]) {
			nss++;
		} else {
			break;
		}
	}

	return nss;
}

#if (WL_STA_VER >= 7)
int
wl_he_nss(uint16 *mcsset)
{
	int i, nss;
	int nss_i_max, nss_max;
	static const char zero[sizeof(uint16) * WL_HE_CAP_MCS_MAP_NSS_MAX] = { 0 };

	uint rx_mcs, tx_mcs;
	uint16 he_txmcsmap, he_rxmcsmap;

	if (mcsset == NULL || !memcmp(mcsset, zero, sizeof(uint16) * WL_HE_CAP_MCS_MAP_NSS_MAX)) {
		return 0;
	}

	nss_max = 0;
	for (i = 0; i < 3; i++) {
		he_txmcsmap = dtoh16(mcsset[i * 2]);
		he_rxmcsmap = dtoh16(mcsset[(i * 2) + 1]);

		nss_i_max = 0;
		for (nss = 1; nss <= HE_CAP_MCS_MAP_NSS_MAX; nss++) {
			tx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(nss, he_txmcsmap);
			rx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(nss, he_rxmcsmap);
			if ((tx_mcs != HE_CAP_MAX_MCS_NONE) ||
				(rx_mcs != HE_CAP_MAX_MCS_NONE)) {
				if (nss > nss_i_max)
					nss_i_max = nss;
			}
		}

		if (nss_i_max > nss_max)
			nss_max = nss_i_max;
	}

	return nss_max;
}
#endif

int
wl_sta_info_nss(void *buf, int unit)
{
#if (WL_STA_VER >= 7)
	sta_info_v5_t *sta;
	sta_info_v7_t *sta_v7 = NULL;
#else
	sta_info_t *sta;
#endif
	int err = 0;
	wl_rateset_args_u_t *rateset_adv = NULL;
	bool have_rateset_adv = FALSE;

#if (WL_STA_VER >= 7)
	sta = (sta_info_v5_t *)buf;
#else
	sta = (sta_info_t *)buf;
#endif
	sta->ver = dtoh16(sta->ver);
	sta->len = dtoh16(sta->len);

	if (sta->ver < 5) {
		printf(" ERROR: unsupported driver station info version %d\n", sta->ver);
		return -1;
#if (WL_STA_VER >= 5)
	} else if (sta->ver == 5) {
#if (WL_STA_VER >= 7)
		sta = (sta_info_v5_t *)buf;
		rateset_adv = (wl_rateset_args_u_t *)&sta->rateset_adv;
#else
		sta = (sta_info_t *)buf;
		rateset_adv = (wl_rateset_args_u_t *)&sta->rateset_adv;
#endif
		have_rateset_adv = TRUE;
#endif
#if (WL_STA_VER >= 7)
	} else if (sta->ver >= 7) {
		sta_v7 = (sta_info_v7_t *)buf;
		rateset_adv = (wl_rateset_args_u_t *)&sta_v7->rateset_adv;
		have_rateset_adv = TRUE;
#endif
	} else {
		printf(" ERROR: unsupported driver station info version %d\n", sta->ver);
		return -1;
	}

#if (WL_STA_VER >= 5)
	/* Driver didn't return extended station info */
#if (WL_STA_VER >= 7)
	if (sta->len < sizeof(sta_info_v5_t)) {
#else
	if (sta->len < sizeof(sta_info_t)) {
#endif
		return 0;
	}
#endif

	if (rateset_adv && have_rateset_adv) {
		int rslen = 0, rsver = 0;
		uint8 *rs_mcs = NULL;
		uint16 *rs_vht_mcs = NULL;
#if (WL_STA_VER >= 7)
		uint16 *rs_he_mcs = NULL;

		if (sta->ver >= 7) {
			rs_mcs = rateset_adv->rsv2.mcs;
			rs_vht_mcs = rateset_adv->rsv2.vht_mcs;
			rs_he_mcs = rateset_adv->rsv2.he_mcs;
		}
		else
#endif
		{
			if ((err = wl_get_rateset_args_info(unit, &rslen, &rsver)) < 0)
				return (err);
			wl_rateset_get_fields(rateset_adv, rsver, NULL, NULL, &rs_mcs,
				&rs_vht_mcs, NULL);
		}

#if (WL_STA_VER >= 7)
		if (rs_he_mcs != NULL && rs_he_mcs[0] != 0xffff) {
			return wl_he_nss((uint16 *)rs_he_mcs);
		} else
#endif
		if (rs_vht_mcs != NULL && rs_vht_mcs[0]) {
			return wl_vht_nss((uint16 *)rs_vht_mcs);
		} else
		if (isset(rs_mcs, 0)) {
			return wl_ht_nss((char *)rs_mcs);
		}
	}

	return 1;
}
#endif


#define PHY_TYPE_A	0
#define PHY_TYPE_B	1
#define PHY_TYPE_G	2
#define PHY_TYPE_N	3
#define PHY_TYPE_AC	4
#define PHY_TYPE_AX	5
#define PHY_TYPE_MAX	6

const char *phy_type_str[PHY_TYPE_MAX] = {
	"a",
	"b",
	"g",
	"n",
	"ac",
	"ax"
};

int wl_sta_info_phy(void *buf, int unit)
{
	sta_info_t *sta = (sta_info_t *) buf;
	uint i;
	uint r;

#if (WL_STA_VER >= 7)
	if (sta->flags & WL_STA_HE_CAP)
		return 5;
#endif
#if (WL_STA_VER >= 4)
	if (sta->flags & WL_STA_VHT_CAP)
		return 4;
#endif
	if (sta->flags & WL_STA_N_CAP)
		return 3;

	/* parse rate set */
	for (i = 0; i < sta->rateset.count; i++) {
		r = sta->rateset.rates[i] & 0x7f;

		if (r == 0)
			break;

		if ((r/2) >= 12) {
			if (unit)
				return PHY_TYPE_A;
			else
				return PHY_TYPE_G;
		}
	}

	return PHY_TYPE_B;
}

#define WL_BW_UNDEFINED		0
#define WL_BW_20M		1
#define WL_BW_40M		2
#define WL_BW_80M		3
#define WL_BW_160M		4
#define WL_BW_MAX		5

const char *wl_bw_str[WL_BW_MAX] = {
	"",
	"20MHz",
	"40MHz",
	"80MHz",
	"160MHz",
};

int wl_sta_info_bw(void *buf)
{
#if (WL_STA_VER >= 7)
	sta_info_t *sta = (sta_info_t *) buf;
	uint32 tx_rspec = sta->tx_rspec;
	uint32 rx_rspec = sta->rx_rspec;
	uint bw_tx = 0, bw_rx = 0;

	bw_tx = ((tx_rspec & WL_RSPEC_BW_MASK) >> WL_RSPEC_BW_SHIFT);
	bw_rx = ((rx_rspec & WL_RSPEC_BW_MASK) >> WL_RSPEC_BW_SHIFT);

	return max(bw_tx, bw_rx);
#else
	return 0;
#endif
}

static int
wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret;
	struct ether_addr bssid;
	wlc_ssid_t ssid;
	char ssidbuf[SSID_FMT_BUF_LEN];
	wl_bss_info_t *bi;
	int retval = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	uint32 chanim_enab = 0;
	uint32 interference = 0;
	static union {
		char bufdata[WLC_IOCTL_SMLEN];
		uint32 alignme;
	} bufstruct;
	char *retbuf = (char*) &bufstruct.bufdata;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
			return 0;
		bi = (wl_bss_info_t*)(buf + 4);
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
		{
			retval += dump_bss_info(eid, wp, argc, argv, bi);

			if (wl_iovar_getint(name, "chanim_enab", (int*)(void*)&chanim_enab))
				chanim_enab = 0;

			if (chanim_enab && !wl_iovar_getbuf(name, "chanim_state", &bi->chanspec, sizeof(chanspec_t), retbuf, WLC_IOCTL_SMLEN)) {
				interference = *(int*)retbuf;
				ret += websWrite(wp, "Interference Level: %s\n", CHANIMSTR(chanim_enab, interference, "Severe", "Acceptable"));
			}
		}
		else
			retval += websWrite(wp, "Sorry, your driver has bss_info_version %d "
				"but this program supports only version %d.\n",
				bi->version, WL_BSS_INFO_VERSION);
	} else {
		retval += websWrite(wp, "Not associated. Last associated with ");

		if ((ret = wl_ioctl(name, WLC_GET_SSID, &ssid, sizeof(wlc_ssid_t))) < 0) {
			retval += websWrite(wp, "\n");
			return 0;
		}

		wl_format_ssid(ssidbuf, ssid.SSID, dtoh32(ssid.SSID_len));
		retval += websWrite(wp, "SSID: \"%s\"\n", ssidbuf);
	}

	return retval;
}

sta_info_t *
wl_sta_info(char *ifname, struct ether_addr *ea)
{
	static char buf[sizeof(sta_info_t)];
	sta_info_t *sta = NULL;

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf))) {
		sta = (sta_info_t *)buf;
		sta->ver = dtoh16(sta->ver);

		/* Report unrecognized version */
		if (sta->ver > WL_STA_VER) {
			dbg(" ERROR: unknown driver station info version %d\n", sta->ver);
			return NULL;
		}

		sta->len = dtoh16(sta->len);
		sta->cap = dtoh16(sta->cap);
#if (WL_STA_VER >= 4)
		sta->aid = dtoh16(sta->aid);
#endif
		sta->flags = dtoh32(sta->flags);
		sta->idle = dtoh32(sta->idle);
		sta->rateset.count = dtoh32(sta->rateset.count);
		sta->in = dtoh32(sta->in);
		sta->listen_interval_inms = dtoh32(sta->listen_interval_inms);
#if (WL_STA_VER >= 4)
		sta->ht_capabilities = dtoh16(sta->ht_capabilities);
		sta->vht_flags = dtoh16(sta->vht_flags);
#endif
	}

	return sta;
}

char *
print_rate_buf(int raw_rate, char *buf)
{
	if (!buf) return NULL;

	if (raw_rate == -1) sprintf(buf, "        ");
	else if ((raw_rate % 1000) == 0)
		sprintf(buf, "%6dM ", raw_rate / 1000);
	else
		sprintf(buf, "%6.1fM ", (double) raw_rate / 1000);

	return buf;
}

char *
print_rate_buf_compact(int raw_rate, char *buf)
{
	if (!buf) return NULL;

	if (raw_rate == -1)
		sprintf(buf, "        ");
	else if ((raw_rate % 1000) == 0)
		sprintf(buf, "%d", raw_rate / 1000);
	else
		sprintf(buf, "%.1f", (double) raw_rate / 1000);

	return buf;
}

int wl_control_channel(int unit);

#ifdef RTCONFIG_QTN
extern int wl_status_5g(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_status_5g(int eid, webs_t wp, int argc, char_t **argv);
extern int wl_status_5g_array(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_wl_status_5g_array(int eid, webs_t wp, int argc, char_t **argv);
#endif

int
ej_wl_status(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	struct maclist *auth = NULL;
	char *macs = NULL, *next = NULL;
	char mac[100];
	int len = 0;
	sta_info_t *sta = NULL;
	char sta_buf[sizeof(sta_info_t)];
	int mac_list_size;
	int i, ii, val = 0, ret = 0;
	char ea[ETHER_ADDR_STR_LEN];
	scb_val_t scb_val;
	char rate_buf[8];
	int hr, min, sec;
#ifdef RTCONFIG_BCMWL6
	wl_dfs_status_t *dfs_status;
	char chanspec_str[CHANSPEC_STR_LEN];
	uint bitmap;
	uint channel;
	uint32 chanspec_arg;
	int first = 0, last = MAXCHANNEL, minutes;
	bool all = TRUE;
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#ifdef RTCONFIG_PROXYSTA
	if (psta_exist_except(unit))
	{
		ret += websWrite(wp, "%s radio is disabled\n",
			(wl_control_channel(unit) > 0) ?
			((wl_control_channel(unit) > CH_MAX_2G_CHANNEL) ? "5 GHz" : "2.4 GHz") :
			(nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz"));
		return ret;
	}
#endif
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
#ifdef RTCONFIG_QTN
	if (unit && rpc_qtn_ready())
	{
		ret = qcsapi_wifi_rfstatus((qcsapi_unsigned_int *) &val);
		if (ret < 0)
			dbG("qcsapi_wifi_rfstatus error, return: %d\n", ret);
		else
			val = !val;
	}
	else
#endif
	{
		wl_ioctl(name, WLC_GET_RADIO, &val, sizeof(val));
		val &= WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;
	}

#ifdef RTCONFIG_QTN
	if (unit && !rpc_qtn_ready())
	{
		ret += websWrite(wp, "5 GHz radio is not ready\n");
		return ret;
	}
	else
#endif
	if (val)
	{
		ret += websWrite(wp, "%s radio is disabled\n",
			(wl_control_channel(unit) > 0) ?
			((wl_control_channel(unit) > CH_MAX_2G_CHANNEL) ? "5 GHz" : "2.4 GHz") :
			(nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz"));
		return ret;
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "wds")) {
		// dump static info only for wds mode:
		// ret += websWrite(wp, "SSID: %s\n", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
		ret += websWrite(wp, "Channel: %d\n", wl_control_channel(unit));
	}
	else {
#ifdef RTCONFIG_QTN
		if (unit)
			ret += wl_status_5g(eid, wp, argc, argv);
		else
#endif
		ret += wl_status(eid, wp, argc, argv, unit);
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "ap"))
	{
		if (nvram_match(strcat_r(prefix, "lazywds", tmp), "1") ||
			nvram_invmatch(strcat_r(prefix, "wds", tmp), ""))
			ret += websWrite(wp, "Mode	: Hybrid\n");
		else	ret += websWrite(wp, "Mode	: AP Only\n");
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
	{
		ret += websWrite(wp, "Mode	: WDS Only\n");
		return ret;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "sta"))
	{
		ret += websWrite(wp, "Mode	: Stations\n");
		ret += ej_wl_sta_status(eid, wp, name);
		return ret;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wet"))
	{
//		ret += websWrite(wp, "Mode	: Ethernet Bridge\n");
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((sw_mode() == SW_MODE_REPEATER)
			&& (nvram_get_int("wlc_band") == unit))
			sprintf(prefix, "wl%d.%d_", unit, 1);
#endif
		ret += websWrite(wp, "Mode	: Repeater [ SSID local: \"%s\" ]\n", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
//		ret += ej_wl_sta_status(eid, wp, name);
//		return ret;
	}
#ifdef RTCONFIG_PROXYSTA
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "psta"))
	{
		if ((sw_mode() == SW_MODE_AP) &&
			(nvram_get_int("wlc_psta") == 1) &&
			(nvram_get_int("wlc_band") == unit))
		ret += websWrite(wp, "Mode	: Media Bridge\n");
	}
#endif

#ifdef RTCONFIG_QTN
	if (unit && rpc_qtn_ready())
	{
		ret += ej_wl_status_5g(eid, wp, argc, argv);
		return ret;
	}
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
	if ((sw_mode() == SW_MODE_REPEATER)
		&& (nvram_get_int("wlc_band") == unit))
	{
		sprintf(name_vif, "wl%d.%d", unit, 1);
		name = name_vif;
	}
#endif

	if (!strlen(name))
		goto exit;

#ifdef RTCONFIG_BCMWL6
	if (!nvram_match(strcat_r(prefix, "reg_mode", tmp), "h"))
		goto wds_list;

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "dfs_status");

	if (wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf)) < 0)
		goto wds_list;

	dfs_status = (wl_dfs_status_t *) buf;
	dfs_status->state = dtoh32(dfs_status->state);
	dfs_status->duration = dtoh32(dfs_status->duration);
	dfs_status->chanspec_cleared = wl_chspec_from_driver(dfs_status->chanspec_cleared);

	if (dfs_status->state >= WL_DFS_CACSTATES) {
		ret += websWrite(wp, "\nDFS status: Unknown DFS state %d\n", dfs_status->state);
	} else {
		const char *dfs_cacstate_str[WL_DFS_CACSTATES] = {
			"IDLE",
			"PRE-ISM Channel Availability Check (CAC)",
			"In-Service Monitoring (ISM)",
			"Channel Switching Announcement (CSA)",
			"POST-ISM Channel Availability Check",
			"PRE-ISM Ouf Of Channels (OOC)",
			"POST-ISM Out Of Channels (OOC)"
		};

		ret += websWrite(wp, "\nDFS status: state %s time elapsed %dms radar channel cleared by DFS ",
			dfs_cacstate_str[dfs_status->state], dfs_status->duration);

		if (dfs_status->chanspec_cleared) {
			ret += websWrite(wp, "channel %s (0x%04X)\n",
				wf_chspec_ntoa(dfs_status->chanspec_cleared, chanspec_str),
				dfs_status->chanspec_cleared);
		}
		else {
			ret += websWrite(wp, "none\n");
		}
	}

	ret += websWrite(wp, "\n");
	ret += websWrite(wp, "Channel Information                     \n");
	ret += websWrite(wp, "----------------------------------------\n");

	for (; first <= last; first++) {
		channel = first;
		chanspec_arg = CH20MHZ_CHSPEC(channel);

		strcpy(buf, "per_chan_info");
		memcpy((char *)(buf + strlen(buf) + 1), (char*)&chanspec_arg, sizeof(chanspec_arg));

		if (wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf)) < 0)
			break;

		bitmap = dtoh32(*(uint *)buf);
		minutes = (bitmap >> 24) & 0xff;

		if (!(bitmap & WL_CHAN_VALID_HW)) {
			if (!all)
				ret += websWrite(wp, "Invalid Channel\n");
			continue;
		}

		if (!(bitmap & WL_CHAN_VALID_SW)) {
			if (!all)
				ret += websWrite(wp, "Not supported in current locale\n");
			continue;
		}

		ret += websWrite(wp, "Channel %d\t", channel);

		if (bitmap & WL_CHAN_BAND_5G)
			ret += websWrite(wp, "A Band");
		else
			ret += websWrite(wp, "B Band");

		if (bitmap & WL_CHAN_RADAR) {
			ret += websWrite(wp, ", RADAR Sensitive");
		}
		if (bitmap & WL_CHAN_RESTRICTED) {
			ret += websWrite(wp, ", Restricted");
		}
		if (bitmap & WL_CHAN_PASSIVE) {
			ret += websWrite(wp, ", Passive");
		}
		if (bitmap & WL_CHAN_INACTIVE) {
			ret += websWrite(wp, ", Temporarily Out of Service for %d minutes", minutes);
		}
		ret += websWrite(wp, "\n");
	}

wds_list:
#endif
	if ((nvram_match(strcat_r(prefix, "mode", tmp), "ap")
	  || nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
		&& !nvram_match(strcat_r(prefix, "wds", tmp), "")) {
		ret += websWrite(wp, "\n");
		ret += websWrite(wp, "Bridge List                             \n");
		ret += websWrite(wp, "----------------------------------------\n");
		ret += websWrite(wp, "%-4s%-18s%-7s\n", "idx", "MAC", "Status");

		macs = nvram_safe_get(strcat_r(prefix, "wds", tmp));
		i = 1;
		foreach(mac, macs, next) {
			ret += websWrite(wp, "%-3d %17s ", i++, mac);

			len = snprintf(sta_buf, sizeof(sta_buf), "sta_info");
			ether_atoe(mac, (unsigned char *)&sta_buf[len + 1]);
			if (atoi(nvram_safe_get(strcat_r(prefix, "wds_timeout", tmp))) &&
			    !wl_ioctl(name, WLC_GET_VAR, sta_buf, sizeof(sta_buf))) {
				sta = (sta_info_t *)sta_buf;
				ret += websWrite(wp, "%-7s", (sta->flags & WL_STA_WDS_LINKUP) ? "up" : "down");
			}
			else
				ret += websWrite(wp, "%-7s", "unknown");
			ret += websWrite(wp, "\n");
		}
	}

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	memset(auth, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)auth, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
		goto exit;

	ret += websWrite(wp, "\n");
	ret += websWrite(wp, "Stations List                           \n");
	ret += websWrite(wp, "----------------------------------------\n");
	ret += websWrite(wp, "%-4s%-18s%-11s%-11s%-8s%-4s%-4s",
				"idx", "MAC", "Associated", "Authorized", "   RSSI ", "PHY", "PSM");
#ifndef RTCONFIG_QTN
#if (WL_STA_VER >= 4)
	ret += websWrite(wp, "%-4s%-5s",
				"SGI", "STBC");
#if (WL_STA_VER >= 5)
#ifdef RTCONFIG_BCM_7114
	if (!nvram_get_int("dhd24"))
#endif
	ret += websWrite(wp, "%-5s%-4s",
				"MUBF", "NSS");
#endif
#if (WL_STA_VER >= 7)
	ret += websWrite(wp, "%-5s",
				"  BW");
#endif
#endif
#endif
	ret += websWrite(wp, "%-8s%-8s%-12s\n",
				"Tx rate", "Rx rate", "Connect Time");

	/* build authenticated sta list */
	for (i = 0; i < auth->count; i ++) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) continue;
		if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

		ret += websWrite(wp, "    ");

		ret += websWrite(wp, "%s ", ether_etoa((void *)&auth->ea[i], ea));

		ret += websWrite(wp, "%-11s%-11s", (sta->flags & WL_STA_ASSOC) ? "Yes" : " ", (sta->flags & WL_STA_AUTHO) ? "Yes" : "");

		memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
		if (wl_ioctl(name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
			ret += websWrite(wp, "%-8s", "");
		else
			ret += websWrite(wp, " %3ddBm ", scb_val.val);

		ret += websWrite(wp, "%-4s", phy_type_str[wl_sta_info_phy(sta, unit)]);

		ret += websWrite(wp, "%-4s",
			(sta->flags & WL_STA_PS) ? "Yes" : "No");
#ifndef RTCONFIG_QTN
#if (WL_STA_VER >= 4)
		ret += websWrite(wp, "%-4s%-5s",
			((sta->ht_capabilities & WL_STA_CAP_SHORT_GI_20) || (sta->ht_capabilities & WL_STA_CAP_SHORT_GI_40)) ? "Yes" : "No",
			((sta->ht_capabilities & WL_STA_CAP_TX_STBC) || (sta->ht_capabilities & WL_STA_CAP_RX_STBC_MASK)) ? "Yes" : "No");
#if (WL_STA_VER >= 5)
#ifdef RTCONFIG_BCM_7114
		if (!nvram_get_int("dhd24"))
#endif
		{
			ret += websWrite(wp, "%-5s",
				((sta->vht_flags & WL_STA_MU_BEAMFORMER) || (sta->vht_flags & WL_STA_MU_BEAMFORMEE)) ? "Yes" : "No");
			ret += websWrite(wp, "%3d ", wl_sta_info_nss(sta, unit));
		}
#endif
#if (WL_STA_VER >= 7)
		if (sta->flags & WL_STA_SCBSTATS)
			ret += websWrite(wp, "%4s ", wl_bw_str[wl_sta_info_bw(sta)]);
		else
			ret += websWrite(wp, "%5s", "");
#endif
#endif
#endif
		if (sta->flags & WL_STA_SCBSTATS)
		{
			ret += websWrite(wp, "%8s", print_rate_buf(sta->tx_rate, rate_buf));
			ret += websWrite(wp, "%8s", print_rate_buf(sta->rx_rate, rate_buf));
		}
		else
			ret += websWrite(wp, "%-16s", "");

		hr = sta->in / 3600;
		min = (sta->in % 3600) / 60;
		sec = sta->in - hr * 3600 - min * 60;
		ret += websWrite(wp, "%02d:%02d:%02d", hr, min, sec);
		ret += websWrite(wp, "\n");
	}

	for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((sw_mode() == SW_MODE_REPEATER)
			&& (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;
#endif
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			sprintf(name_vif, "wl%d.%d", unit, i);

			memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;
				if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

				ret += websWrite(wp, "%-3d ", i);

				ret += websWrite(wp, "%s ", ether_etoa((void *)&auth->ea[ii], ea));

				ret += websWrite(wp, "%-11s%-11s", (sta->flags & WL_STA_ASSOC) ? "Yes" : " ", (sta->flags & WL_STA_AUTHO) ? "Yes" : "");

				memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
				if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
					ret += websWrite(wp, "%-8s", "");
				else
					ret += websWrite(wp, " %3ddBm ", scb_val.val);

				ret += websWrite(wp, "%-4s", phy_type_str[wl_sta_info_phy(sta, unit)]);

				ret += websWrite(wp, "%-4s",
					(sta->flags & WL_STA_PS) ? "Yes" : "No");
#ifndef RTCONFIG_QTN
#if (WL_STA_VER >= 4)
				ret += websWrite(wp, "%-4s%-5s",
					((sta->ht_capabilities & WL_STA_CAP_SHORT_GI_20) || (sta->ht_capabilities & WL_STA_CAP_SHORT_GI_40)) ? "Yes" : "No",
					((sta->ht_capabilities & WL_STA_CAP_TX_STBC) || (sta->ht_capabilities & WL_STA_CAP_RX_STBC_MASK)) ? "Yes" : "No");
#if (WL_STA_VER >= 5)
#ifdef RTCONFIG_BCM_7114
				if (!nvram_get_int("dhd24"))
#endif
				{
					ret += websWrite(wp, "%-5s",
						((sta->vht_flags & WL_STA_MU_BEAMFORMER) || (sta->vht_flags & WL_STA_MU_BEAMFORMEE)) ? "Yes" : "No");
					ret += websWrite(wp, "%3d ", wl_sta_info_nss(sta, unit));
				}
#endif
#if (WL_STA_VER >= 7)
				if (sta->flags & WL_STA_SCBSTATS)
					ret += websWrite(wp, "%4s ", wl_bw_str[wl_sta_info_bw(sta)]);
				else
					ret += websWrite(wp, "%5s", "");
#endif
#endif
#endif
				if (sta->flags & WL_STA_SCBSTATS)
				{
					ret += websWrite(wp, "%8s", print_rate_buf(sta->tx_rate, rate_buf));
					ret += websWrite(wp, "%8s", print_rate_buf(sta->rx_rate, rate_buf));
				}
				else
					ret += websWrite(wp, "%-16s", "");

				hr = sta->in / 3600;
				min = (sta->in % 3600) / 60;
				sec = sta->in - hr * 3600 - min * 60;
				ret += websWrite(wp, "%02d:%02d:%02d", hr, min, sec);

				ret += websWrite(wp, "\n");
			}
		}
	}

	/* error/exit */
exit:
	if (auth) free(auth);

	return ret;
}

int
ej_wl_status_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int ii = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char *temp;

	for (ii = 0; ii < DEV_NUMIFS; ii++) {
		sprintf(nv_param, "wl%d_unit", ii);
		temp = nvram_get(nv_param);

		if (temp && strlen(temp) > 0)
		{
			retval += ej_wl_status(eid, wp, argc, argv, ii);
			retval += websWrite(wp, "\n");
		}
	}

	return retval;
}

static int
wl_extent_channel(int unit)
{
	int ret;
	struct ether_addr bssid;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
#ifdef RTCONFIG_QTN
	qcsapi_unsigned_int bw;
#endif

        snprintf(prefix, sizeof(prefix), "wl%d_", unit);
        name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if ((unit == 1) || (unit == 2)) {
#ifdef RTCONFIG_QTN
		if (rpc_qcsapi_get_bw(&bw) >= 0) {
			return bw;
		} else {
			return 0;
		}
#else
		if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
			/* The adapter is associated. */
			*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
			if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
				return 0;
			bi = (wl_bss_info_t*)(buf + 4);

			return bw_chspec_to_mhz(bi->chanspec);
		} else {
			return 0;
		}
#endif

	}

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
			return 0;

		bi = (wl_bss_info_t*)(buf + 4);
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		   dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		   dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
		{
			/* Convert version 107 to 109 */
			if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
				old_bi = (wl_bss_info_107_t *)bi;
				bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
				bi->ie_length = old_bi->ie_length;
				bi->ie_offset = sizeof(wl_bss_info_107_t);
			}
			if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				return  CHSPEC_CHANNEL(bi->chanspec);
		}
	}
	return 0;
}

int
ej_wl_extent_channel(int eid, webs_t wp, int argc, char_t **argv)
{

#if defined(RTAC3200) || defined(RTAC5300)
	return websWrite(wp, "[\"%d\", \"%d\", \"%d\"]", wl_extent_channel(0), wl_extent_channel(1), wl_extent_channel(2));
#else
	return websWrite(wp, "[\"%d\", \"%d\"]", wl_extent_channel(0), wl_extent_channel(1));
#endif
}

int
wl_control_channel(int unit)
{
	int ret;
	struct ether_addr bssid;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
#ifdef RTCONFIG_QTN
	qcsapi_unsigned_int channel;
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
			return 0;

		bi = (wl_bss_info_t*)(buf + 4);
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
		{
			/* Convert version 107 to 109 */
			if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
				old_bi = (wl_bss_info_107_t *)bi;
				bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
				bi->ie_length = old_bi->ie_length;
				bi->ie_offset = sizeof(wl_bss_info_107_t);
			}

			if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				return bi->ctl_ch;
			else
				return (bi->chanspec & WL_CHANSPEC_CHAN_MASK);
		}
	}

#ifdef RTCONFIG_QTN
	ret = rpc_qcsapi_get_channel(&channel);
	if (ret < 0) return 0;
	else return channel;
#else
	return 0;
#endif
}

int
ej_wl_control_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	char word[256], *next;
	int count_wl_if = 0;

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		count_wl_if++;

	ret = websWrite(wp, "[\"%d\", \"%d\"", wl_control_channel(0), wl_control_channel(1));
	if (count_wl_if >= 3)
		ret += websWrite(wp, ", \"%d\"", wl_control_channel(2));
	ret += websWrite(wp, "]");

	return ret;
}

#define	IW_MAX_FREQUENCIES	32

static int ej_wl_channel_list(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int i, retval = 0;
	int channels[MAXCHANNEL+1];
	wl_uint32_list_t *list = (wl_uint32_list_t *) channels;
	char tmp[256], tmp1[256], tmp2[256], prefix[] = "wlXXXXXXXXXX_";
	char tmpx[256];
	char *name;
	uint ch;
	char word[256], *next;
	int unit_max = 0, count = 0;

	sprintf(tmp1, "[\"%d\"]", 0);

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		unit_max++;

	if (unit > (unit_max - 1))
		goto ERROR;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (is_wlif_up(name) != 1)
	{
		foreach (word, nvram_safe_get(strcat_r(prefix, "chlist", tmp)), next)
			count++;

		if (count < 2)
			goto ERROR;

		i = 0;
		foreach (word, nvram_safe_get(strcat_r(prefix, "chlist", tmp)), next) {
			if (i == 0)
			{
				sprintf(tmp1, "[\"%s\",", word);
			}
			else if (i == (count - 1))
			{
				sprintf(tmpx,  "%s \"%s\"]", tmp1, word);
				strlcpy(tmp1, tmpx, sizeof(tmp1));
			}
			else
			{
				sprintf(tmpx,  "%s \"%s\",", tmp1, word);
				strlcpy(tmp1, tmpx, sizeof(tmp1));
			}

			i++;
		}
		goto ERROR;
	}

	memset(channels, 0, sizeof(channels));
	list->count = htod32(MAXCHANNEL);
	if (wl_ioctl(name, WLC_GET_VALID_CHANNELS , channels, sizeof(channels)) < 0)
	{
		dbg("error doing WLC_GET_VALID_CHANNELS\n");
		sprintf(tmp1, "[\"%d\"]", 0);
		goto ERROR;
	}

	if (dtoh32(list->count) == 0)
	{
		sprintf(tmp1, "[\"%d\"]", 0);
		goto ERROR;
	}

	for (i = 0; i < dtoh32(list->count) && i < IW_MAX_FREQUENCIES; i++) {
		ch = dtoh32(list->element[i]);

		if (i == 0)
		{
			sprintf(tmp1, "[\"%d\",", ch);
			sprintf(tmp2, "%d", ch);
		}
		else if (i == (dtoh32(list->count) - 1))
		{
			sprintf(tmpx,  "%s \"%d\"]", tmp1, ch);
			strlcpy(tmp1, tmpx, sizeof(tmp1));
			sprintf(tmpx,  "%s %d", tmp2, ch);
			strlcpy(tmp2, tmpx, sizeof(tmp2));
		}
		else
		{
			sprintf(tmpx,  "%s \"%d\",", tmp1, ch);
			strlcpy(tmp1, tmpx, sizeof(tmp1));
			sprintf(tmpx,  "%s %d", tmp2, ch);
			strlcpy(tmp2, tmpx, sizeof(tmp2));
		}

		if (strlen(tmp2))
			nvram_set(strcat_r(prefix, "chlist", tmp), tmp2);
	}
ERROR:
	retval += websWrite(wp, "%s", tmp1);
	return retval;
}

static int ej_wl_chanspecs(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int i, retval = 0;
	char tmp[1024], tmp1[1024], tmp2[1024], tmpx[1024], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int unit_max = 0, count = 0;
	wl_uint32_list_t *list;
	chanspec_t c = 0;
	char data_buf[WLC_IOCTL_MAXLEN];
	char chanbuf[CHANSPEC_STR_LEN];

	sprintf(tmp1, "[\"%d\"]", 0);

#ifdef RTCONFIG_QTN
	 if (unit) goto ERROR;
#endif

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		unit_max++;

	if (unit > (unit_max - 1))
		goto ERROR;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	if (is_wlif_up(name) != 1) {
		foreach (word, nvram_safe_get(strcat_r(prefix, "chansps", tmp)), next)
			count++;

		if (count < 2)
			goto ERROR;

		i = 0;
		foreach (word, nvram_safe_get(strcat_r(prefix, "chansps", tmp)), next) {
			if (i == 0) {
				sprintf(tmp1, "[\"%s\",", word);
			} else if (i == (count - 1)) {
				sprintf(tmpx,  "%s \"%s\"]", tmp1, word);
				strlcpy(tmp1, tmpx, sizeof(tmp1));
			} else {
				sprintf(tmpx,  "%s \"%s\",", tmp1, word);
				strlcpy(tmp1, tmpx, sizeof(tmp1));
			}

			i++;
		}

		goto ERROR;
	}

	memset(data_buf, 0, WLC_IOCTL_MAXLEN);
	if (wl_iovar_getbuf(name, "chanspecs", &c, sizeof(chanspec_t), data_buf, WLC_IOCTL_MAXLEN) < 0) {
		dbg("error doing WLC_GET_VAR chanspecs\n");
		sprintf(tmp1, "[\"%d\"]", 0);
		goto ERROR;
	}

	list = (wl_uint32_list_t *)data_buf;
	count = dtoh32(list->count);

	if (!count) {
		dbg("number of valid chanspec is 0\n");
		sprintf(tmp1, "[\"%d\"]", 0);
		goto ERROR;
	} else
	for (i = 0; i < count; i++) {
		c = (chanspec_t)dtoh32(list->element[i]);
		wf_chspec_ntoa(c, chanbuf);

		if (i == 0)
		{
			sprintf(tmp1, "[\"%s\",", chanbuf);
			sprintf(tmp2, "%s", chanbuf);
		}
		else if (i == (count - 1))
		{
			sprintf(tmpx,  "%s \"%s\"]", tmp1, chanbuf);
			strlcpy(tmp1, tmpx, sizeof(tmp1));
			sprintf(tmpx,  "%s %s", tmp2, chanbuf);
			strlcpy(tmp2, tmpx, sizeof(tmp2));
		}
		else
		{
			sprintf(tmpx,  "%s \"%s\",", tmp1, chanbuf);
			strlcpy(tmp1, tmpx, sizeof(tmp1));
			sprintf(tmpx,  "%s %s", tmp2, chanbuf);
			strlcpy(tmp2, tmpx, sizeof(tmp2));
		}

		if (strlen(tmp2))
			nvram_set(strcat_r(prefix, "chansps", tmp), tmp2);
	}

ERROR:
	if(argc == 0)
		retval += websWrite(wp, "%s", tmp1);
	return retval;
}

int
ej_wl_channel_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 0);
}

#ifndef RTCONFIG_QTN
int
ej_wl_channel_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 1);
}
#endif

int
ej_wl_channel_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_channel_list(eid, wp, argc, argv, 2);
}

int
ej_wl_chanspecs_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_chanspecs(eid, wp, argc, argv, 0);
}

int
ej_wl_chanspecs_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_chanspecs(eid, wp, argc, argv, 1);
}

int
ej_wl_chanspecs_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_chanspecs(eid, wp, argc, argv, 2);
}

#define	WL_IW_RSSI_NO_SIGNAL	-91	/* NDIS RSSI link quality cutoffs */

static int ej_wl_rssi(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int unit_max = 0, unit_cur = -1;
	char rssi_buf[32];
#ifdef RTCONFIG_QTN
	int rssi_by_chain[4];
#endif
	char *mode = NULL;
	int sta = 0, wet = 0, psta = 0, psr = 0;
	int rssi = WL_IW_RSSI_NO_SIGNAL;

	memset(rssi_buf, 0, sizeof(rssi_buf));

#ifdef RTCONFIG_QTN
	if (unit != 0) {
		if (!rpc_qtn_ready())
			goto ERROR;

		qcsapi_wifi_get_rssi_by_chain(WIFINAME, 0, &rssi_by_chain[0]);
		qcsapi_wifi_get_rssi_by_chain(WIFINAME, 1, &rssi_by_chain[1]);
		qcsapi_wifi_get_rssi_by_chain(WIFINAME, 2, &rssi_by_chain[2]);
		qcsapi_wifi_get_rssi_by_chain(WIFINAME, 3, &rssi_by_chain[3]);
		rssi = (rssi_by_chain[0] + rssi_by_chain[1] + rssi_by_chain[2] + rssi_by_chain[3]) / 4;

		retval += websWrite(wp, "%d dBm", rssi);

		return retval;
	}
#endif

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		unit_max++;

	if (unit > (unit_max - 1))
		goto ERROR;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	mode = nvram_safe_get(strcat_r(prefix, "mode", tmp));
	sta = !strcmp(mode, "sta");
	wet = !strcmp(mode, "wet");
	psta = !strcmp(mode, "psta");
	psr = !strcmp(mode, "psr");

	wl_ioctl(name, WLC_GET_INSTANCE, &unit_cur, sizeof(unit_cur));
	if (unit != unit_cur)
		goto ERROR;
	else if (!(wet || sta || psta || psr))
		goto ERROR;
	else if (wl_ioctl(name, WLC_GET_RSSI, &rssi, sizeof(rssi))) {
		dbg("can not get rssi info of %s\n", name);
		goto ERROR;
	} else {
		rssi = dtoh32(rssi);
		sprintf(rssi_buf, "%d dBm", rssi);
	}

ERROR:
	retval += websWrite(wp, "%s", rssi_buf);
	return retval;
}

int
ej_wl_rssi_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rssi(eid, wp, argc, argv, 0);
}

int
ej_wl_rssi_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rssi(eid, wp, argc, argv, 1);
}

int
ej_wl_rssi_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rssi(eid, wp, argc, argv, 2);
}

static int ej_wl_rate(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int retval = 0;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char word[256], *next;
	int unit_max = 0, unit_cur = -1;
	int rate = 0;
	char rate_buf[32];
	struct ether_addr bssid;
	unsigned char bssid_null[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	int sta_rate;
	int from_app = 0;

	from_app = check_user_agent(user_agent);

#ifdef RTCONFIG_BCMWL6
	int s = -1;
#endif
#ifdef RTCONFIG_QTN
	uint32_t count = 0, speed;
#endif

	sprintf(rate_buf, "0 Mbps");

#ifdef RTCONFIG_QTN
	if (unit != 0) {
		if (!rpc_qtn_ready()) {
			goto ERROR;
		}
		// if ssid associated, check associations
		if (qcsapi_wifi_get_link_quality(WIFINAME, count, &speed) < 0) {
			// dbg("fail to get link status index %d\n", (int)count);
		} else {
			speed = speed ;  /* 4 antenna? */
			if ((int)speed < 1) {
				sprintf(rate_buf, "auto");
			} else {
				sprintf(rate_buf, "%d Mbps", (int)speed);
			}
		}

		retval += websWrite(wp, "%s", rate_buf);
		return retval;
	}
#endif

	foreach (word, nvram_safe_get("wl_ifnames"), next)
		unit_max++;

	if (unit > (unit_max - 1))
		goto ERROR;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	wl_ioctl(name, WLC_GET_INSTANCE, &unit_cur, sizeof(unit_cur));
	if (unit != unit_cur)
		goto ERROR;
	else if (wl_ioctl(name, WLC_GET_RATE, &rate, sizeof(int))) {
		dbg("can not get rate info of %s\n", name);
		goto ERROR;
	} else {
		rate = dtoh32(rate);
		if ((rate == -1) || (rate == 0))
			sprintf(rate_buf, "auto");
		else
			sprintf(rate_buf, "%d%s Mbps", (rate / 2), (rate & 1) ? ".5" : "");
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "wet")) {
		if (wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0)
			goto ERROR;
		else if (!memcmp(&bssid, bssid_null, 6))
			goto ERROR;

		sta_info_t *sta = wl_sta_info(name, &bssid);
		if (sta && (sta->flags & WL_STA_SCBSTATS)) {

			if ((dtoh32(sta->tx_rate) == -1) &&
				(dtoh32(sta->rx_rate) == -1))
				goto ERROR;

			sta_rate = max(sta->tx_rate, sta->rx_rate);
			rate = max(rate * 500, sta_rate);

			if ((rate % 1000) == 0)
				sprintf(rate_buf, "%6d Mbps", rate / 1000);
			else
				sprintf(rate_buf, "%6.1f Mbps", (double) rate / 1000);
		}
#ifdef RTCONFIG_BCMWL6
	} else if (nvram_match(strcat_r(prefix, "mode", tmp), "psta") ||
		nvram_match(strcat_r(prefix, "mode", tmp), "psr")) {
#if 0
		char eabuf[32];
#endif
		struct ifreq ifr;
		unsigned char wlta[6];

		if (wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0)
			goto ERROR;
		else if (!memcmp(&bssid, bssid_null, 6))
			goto ERROR;

		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
			goto ERROR;

		strcpy(ifr.ifr_name, "br0");
		if (ioctl(s, SIOCGIFHWADDR, &ifr))
			goto ERROR;

		memcpy(wlta, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
		if (nvram_match(strcat_r(prefix, "mode", tmp), "psr"))
			wlta[0] |= 0x02;
#if 0
		dbg("%s TA: %s\n", name, ether_etoa((const unsigned char *)wlta, eabuf));
#endif
		DIR *dir_to_open = NULL;
		char dir_path[128];
		int n, j;
		struct dirent **namelist;

		sprintf(dir_path, "/sys/class/net");
		dir_to_open = opendir(dir_path);
		if (dir_to_open) {
			closedir(dir_to_open);
			n = scandir(dir_path, &namelist, 0, alphasort);

			snprintf(prefix, sizeof(prefix), "wl%d.", unit);

			for (j= 0; j< n; j++) {
				if (namelist[j]->d_name[0] == '.')
				{
					free(namelist[j]);
					continue;
				}
				else if (strncmp(prefix, namelist[j]->d_name, 4))
				{
					free(namelist[j]);
					continue;
				}

				strcpy(tmp, namelist[j]->d_name);
				free(namelist[j]);

				strcpy(ifr.ifr_name, tmp);
				if (ioctl(s, SIOCGIFHWADDR, &ifr))
					goto ERROR;
#if 0
				dbg("%s macaddr: %s\n", tmp, ether_etoa((const unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
#endif
				if (!memcmp(wlta, ifr.ifr_hwaddr.sa_data, 6)) {
					sta_info_t *sta = wl_sta_info(tmp, &bssid);
					if (sta && (sta->flags & WL_STA_SCBSTATS)) {
						if ((dtoh32(sta->tx_rate) == -1) &&
							(dtoh32(sta->rx_rate) == -1))
							goto ERROR;

						sta_rate = max(sta->tx_rate, sta->rx_rate);
						rate = max(rate * 500, sta_rate);

						if ((rate % 1000) == 0)
							sprintf(rate_buf, "%6d Mbps", rate / 1000);
						else
							sprintf(rate_buf, "%6.1f Mbps", (double) rate / 1000);
					}

					break;
				}
			}
		}
#endif
	}

ERROR:
#ifdef RTCONFIG_BCMWL6
	close(s);
#endif
	if(from_app == 0)
		retval += websWrite(wp, "%s", rate_buf);
	else
		retval += websWrite(wp, "\"%s\"", rate_buf);
	return retval;
}

int
ej_wl_rate_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rate(eid, wp, argc, argv, 0);
}

int
ej_wl_rate_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rate(eid, wp, argc, argv, 1);
}

int
ej_wl_rate_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return ej_wl_rate(eid, wp, argc, argv, 2);
}

static int wps_stop_count = 0;

static void reset_wps_status()
{
	if (++wps_stop_count > 30)
	{
		wps_stop_count = 0;
		nvram_set("wps_proc_status_x", "0");
	}
}

char *
getWscStatusStr()
{
	char *status = nvram_safe_get("wps_proc_status_x");

	switch (atoi(status)) {
	case 1: /* WPS_ASSOCIATED */
		wps_stop_count = 0;
		return "Start WPS Process";
		break;
	case 2: /* WPS_OK */
	case 7: /* WPS_MSGDONE */
		reset_wps_status();
		return "Success";
		break;
	case 3: /* WPS_MSG_ERR */
		reset_wps_status();
		return "Fail due to WPS message exchange error!";
		break;
	case 4: /* WPS_TIMEOUT */
		reset_wps_status();
		return "Fail due to WPS time out!";
		break;
	case 5: /* WPS_UI_SENDM2 */
		return "Send M2";
		break;
	case 6: /* WPS_UI_SENDM7 */
		return "Send M7";
		break;
	case 8: /* WPS_PBCOVERLAP */
		reset_wps_status();
		return "Fail due to PBC session overlap!";
		break;
	case 9: /* WPS_UI_FIND_PBC_AP */
		return "Finding a PBC access point...";
		break;
	case 10: /* WPS_UI_ASSOCIATING */
		return "Assciating with access point...";
		break;
	default:
		wps_stop_count = 0;
		if (nvram_match("wps_enable", "1"))
			return "Idle";
		else
			return "Not used";
		break;
	}
}

int
wps_is_oob()
{
	char tmp[16];

	snprintf(tmp, sizeof(tmp), "lan_wps_oob");

	/*
	 * OOB: enabled
	 * Configured: disabled
	 */
	if (nvram_match(tmp, "disabled"))
		return 0;

	return 1;
}

void getWPSAuthMode(int unit, char *ret_str)
{
	char tmp[128], prefix[]="wlXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "shared"))
		strcpy(ret_str, "Shared Key");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "psk"))
		strcpy(ret_str, "WPA-Personal");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "psk2"))
		strcpy(ret_str, "WPA2-Personal");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "pskpsk2"))
		strcpy(ret_str, "WPA-Auto-Personal");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpa"))
		strcpy(ret_str, "WPA-Enterprise");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpa2"))
		strcpy(ret_str, "WPA2-Enterprise");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "wpawpa2"))
		strcpy(ret_str, "WPA-Auto-Enterprise");
	else if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "radius"))
		strcpy(ret_str, "802.1X");
	else
		strcpy(ret_str, "Open System");
}

void getWPSEncrypType(int unit, char *ret_str)
{
	char tmp[128], prefix[]="wlXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") &&
		nvram_match(strcat_r(prefix, "wep_x", tmp), "0"))
		strcpy(ret_str, "None");
	else if ((nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") && !nvram_match(strcat_r(prefix, "wep_x", tmp), "0")) ||
		nvram_match("wl_auth_mode", "shared") ||
		nvram_match("wl_auth_mode", "radius"))
		strcpy(ret_str, "WEP");

	if (nvram_match(strcat_r(prefix, "crypto", tmp), "tkip"))
		strcpy(ret_str, "TKIP");
	else if (nvram_match(strcat_r(prefix, "crypto", tmp), "aes"))
		strcpy(ret_str, "AES");
	else if (nvram_match(strcat_r(prefix, "crypto", tmp), "tkip+aes"))
		strcpy(ret_str, "TKIP+AES");
}

int wl_wps_info(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmpstr[256];
	int retval = 0;
	char tmp[128], prefix[]="wlXXXXXXX_";
	char *wps_sta_pin;
#ifdef RTCONFIG_QTN
	int ret;
	qcsapi_SSID ssid;
#if 0
	string_64 key_passphrase;
	char wps_pin[16];
#endif
#endif


	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	retval += websWrite(wp, "<wps>\n");

	//0. WSC Status
	if (!strcmp(nvram_safe_get(strcat_r(prefix, "wps_mode", tmp)), "enabled"))
	{
#ifdef RTCONFIG_QTN
		if (unit)
		{
			if (!rpc_qtn_ready())
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "5 GHz radio is not ready");
			else
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWscStatusStr_qtn());
		}
		else
#endif
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWscStatusStr());
	}
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "Not used");

	//1. WPSConfigured
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", get_WPSConfiguredStr_qtn());
	}
	else
#endif
	if (!wps_is_oob())
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "Yes");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "No");

	//2. WPSSSID
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			memset(&ssid, 0, sizeof(qcsapi_SSID));
			ret = rpc_qcsapi_get_SSID(WIFINAME, &ssid);
			if (ret < 0)
				dbG("rpc_qcsapi_get_SSID %s error, return: %d\n", WIFINAME, ret);

			memset(tmpstr, 0, sizeof(tmpstr));
			char_to_ascii(tmpstr, ssid);
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
		}
	}
	else
#endif
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//3. WPSAuthMode
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWPSAuthMode_qtn());
	}
	else
#endif
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		getWPSAuthMode(unit, tmpstr);
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//4. EncrypType
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", getWPSEncrypType_qtn());
	}
	else
#endif
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		getWPSEncrypType(unit, tmpstr);
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//5. DefaultKeyIdx
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
			retval += websWrite(wp, "<wps_info>%d</wps_info>\n", 1);
	}
	else
#endif
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s", nvram_safe_get(strcat_r(prefix, "key", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//6. WPAKey
#if 0	//hide for security
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			memset(&key_passphrase, 0, sizeof(key_passphrase));
			ret = rpc_qcsapi_get_key_passphrase(WIFINAME, (char *) &key_passphrase);
			if (ret < 0)
				dbG("rpc_qcsapi_get_key_passphrase %s error, return: %d\n", WIFINAME, ret);

			if (!strlen(key_passphrase))
				retval += websWrite(wp, "<wps_info>None</wps_info>\n");
			else
			{
				memset(tmpstr, 0, sizeof(tmpstr));
				char_to_ascii(tmpstr, key_passphrase);
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
			}
		}
	}
	else
#endif
	if (!strlen(nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp))))
		retval += websWrite(wp, "<wps_info>None</wps_info>\n");
	else
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}
#else
	retval += websWrite(wp, "<wps_info></wps_info>\n");
#endif

	//7. AP PIN Code
#ifdef RTCONFIG_QTNBAK
	/* QTN get wps_device_pin from BRCM */
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			wps_pin[0] = 0;
			ret = rpc_qcsapi_wps_get_ap_pin(WIFINAME, wps_pin, 0);
			if (ret < 0)
				dbG("rpc_qcsapi_wps_get_ap_pin %s error, return: %d\n", WIFINAME, ret);

			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", wps_pin);
		}
	}
	else
#endif
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s", nvram_safe_get("wps_device_pin"));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}

	//8. Saved WPAKey
#if 0	//hide for security
#ifdef RTCONFIG_QTN
	if (unit)
	{
		if (!rpc_qtn_ready())
			retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "");
		else
		{
			if (!strlen(key_passphrase))
				retval += websWrite(wp, "<wps_info>None</wps_info>\n");
			else
			{
				memset(tmpstr, 0, sizeof(tmpstr));
				char_to_ascii(tmpstr, key_passphrase);
				retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
			}
		}
	}
	else
#endif
	if (!strlen(nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp))))
	{
		retval += websWrite(wp, "<wps_info>None</wps_info>\n");
	}
	else
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		char_to_ascii(tmpstr, nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", tmpstr);
	}
#else
	retval += websWrite(wp, "<wps_info></wps_info>\n");
#endif
	//9. WPS enable?
	if (!strcmp(nvram_safe_get(strcat_r(prefix, "wps_mode", tmp)), "enabled"))
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "1");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "0");

	//A. WPS mode
	wps_sta_pin = nvram_safe_get("wps_sta_pin");
	if (strlen(wps_sta_pin) && strcmp(wps_sta_pin, "00000000"))
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "1");
	else
		retval += websWrite(wp, "<wps_info>%s</wps_info>\n", "2");

	//B. current auth mode
	retval += websWrite(wp, "<wps_info>%s</wps_info>\n", nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp)));

	//C. WPS band
	retval += websWrite(wp, "<wps_info>%d</wps_info>\n", nvram_get_int("wps_band_x"));

	retval += websWrite(wp, "</wps>");

	return retval;
}

int
ej_wps_info(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 1);
}

int
ej_wps_info_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_wps_info(eid, wp, argc, argv, 0);
}

int
ej_urelease(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;

	if (    nvram_match("sw_mode_ex", "2") &&
		nvram_invmatch("lan_ipaddr_new", "") &&
		nvram_invmatch("lan_netmask_new", "") &&
		nvram_invmatch("lan_gateway_new", ""))
	{
		retval += websWrite(wp, "[");
		retval += websWrite(wp, "\"%s\", ", nvram_safe_get("lan_ipaddr_new"));
		retval += websWrite(wp, "\"%s\", ", nvram_safe_get("lan_netmask_new"));
		retval += websWrite(wp, "\"%s\"", nvram_safe_get("lan_gateway_new"));
		retval += websWrite(wp, "];");

		kill_pidfile_s("/var/run/ure_monitor.pid", SIGUSR1);
	}
	else
	{
		retval += websWrite(wp, "[");
		retval += websWrite(wp, "\"\", ");
		retval += websWrite(wp, "\"\", ");
		retval += websWrite(wp, "\"\"");
		retval += websWrite(wp, "];");
	}

	return retval;
}

#if 0
static bool find_ethaddr_in_list(void *ethaddr, struct maclist *list) {
	int i;

	for (i = 0; i < list->count; ++i)
		if (!bcmp(ethaddr, (void *)&list->ea[i], ETHER_ADDR_LEN))
			return TRUE;

	return FALSE;
}
#endif

static int wl_sta_list(int eid, webs_t wp, int argc, char_t **argv, int unit) {
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	struct maclist *auth = NULL;
	int mac_list_size;
	int i, firstRow = 1;
	char ea[ETHER_ADDR_STR_LEN];
	scb_val_t scb_val;
	char *value;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	int ii;
	int ret = 0;
	sta_info_t *sta;
	int from_app = 0;

	from_app = check_user_agent(user_agent);

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_WIRELESSREPEATER
	if ((sw_mode() == SW_MODE_REPEATER)
		&& (nvram_get_int("wlc_band") == unit))
	{
		sprintf(name_vif, "wl%d.%d", unit, 1);
		name = name_vif;
	}
#endif

	if (!strlen(name))
		goto exit;

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	memset(auth, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)auth, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
		goto exit;

	/* build authenticated sta list */
	for (i = 0; i < auth->count; ++i) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) continue;
		if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

		if (firstRow == 1)
			firstRow = 0;
		else
			ret += websWrite(wp, ", ");

		if (from_app == 0)
			ret += websWrite(wp, "[");

		ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[i], ea));

		if (from_app != 0) {
			ret += websWrite(wp, ":{");
			ret += websWrite(wp, "\"isWL\":");
		}

		value = (sta->flags & WL_STA_ASSOC) ? "Yes" : "No";
		if (from_app == 0)
			ret += websWrite(wp, ", \"%s\"", value);
		else
			ret += websWrite(wp, "\"%s\"", value);

		value = (sta->flags & WL_STA_AUTHO) ? "Yes" : "No";
		if (from_app == 0)
			ret += websWrite(wp, ", \"%s\"", value);

		if (from_app != 0) {
			ret += websWrite(wp, ",\"rssi\":");
		}

		memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
		if (wl_ioctl(name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t))) {
			if (from_app == 0)
				ret += websWrite(wp, ", \"%d\"", 0);
			else
				ret += websWrite(wp, "\"%d\"", 0);
		} else {
			if (from_app == 0)
				ret += websWrite(wp, ", \"%d\"", scb_val.val);
			else
				ret += websWrite(wp, "\"%d\"", scb_val.val);
		}
		if (from_app == 0)
			ret += websWrite(wp, "]");
		else
			ret += websWrite(wp, "}");
	}

	for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((sw_mode() == SW_MODE_REPEATER)
			&& (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;
#endif
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			sprintf(name_vif, "wl%d.%d", unit, i);

			memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;
				if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

				if (firstRow == 1)
					firstRow = 0;
				else
					ret += websWrite(wp, ", ");

				if (from_app == 0)
					ret += websWrite(wp, "[");

				ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[ii], ea));

				if (from_app != 0) {
					ret += websWrite(wp, ":{");
					ret += websWrite(wp, "\"isWL\":");
				}

				value = (sta->flags & WL_STA_ASSOC) ? "Yes" : "No";
				if (from_app == 0)
					ret += websWrite(wp, ", \"%s\"", value);
				else
					ret += websWrite(wp, "\"%s\"", value);

				value = (sta->flags & WL_STA_AUTHO) ? "Yes" : "No";
				if (from_app == 0)
					ret += websWrite(wp, ", \"%s\"", value);

				if (from_app != 0) {
					ret += websWrite(wp, ",\"rssi\":");
				}

				memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
				if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t))) {
					if (from_app == 0)
						ret += websWrite(wp, ", \"%d\"", 0);
					else
						ret += websWrite(wp, "\"%d\"", 0);
				} else {
					if (from_app == 0)
						ret += websWrite(wp, ", \"%d\"", scb_val.val);
					else
						ret += websWrite(wp, "\"%d\"", scb_val.val);
				}
				if (from_app == 0)
					ret += websWrite(wp, "]");
				else
					ret += websWrite(wp, "}");
			}
		}
	}

	/* error/exit */
exit:
	if (auth) free(auth);

	return ret;
}

#ifdef RTCONFIG_STAINFO
static int wl_stainfo_list(int eid, webs_t wp, int argc, char_t **argv, int unit) {
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	struct maclist *auth = NULL;
	int mac_list_size;
	int i, firstRow = 1;
	char ea[ETHER_ADDR_STR_LEN];
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	int ii;
	int ret = 0;
	sta_info_t *sta;
	char rate_buf[8];
	int hr, min, sec;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_WIRELESSREPEATER
	if ((sw_mode() == SW_MODE_REPEATER)
		&& (nvram_get_int("wlc_band") == unit))
	{
		sprintf(name_vif, "wl%d.%d", unit, 1);
		name = name_vif;
	}
#endif

	if (!strlen(name))
		goto exit;

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	memset(auth, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)auth, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
		goto exit;

	/* build authenticated sta list */
	for (i = 0; i < auth->count; ++i) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) continue;
		if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

		if (firstRow == 1)
			firstRow = 0;
		else
			ret += websWrite(wp, ", ");

		ret += websWrite(wp, "[");

		ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[i], ea));

		ret += websWrite(wp, ", \"%s\"", print_rate_buf_compact(sta->tx_rate, rate_buf));
		ret += websWrite(wp, ", \"%s\"", print_rate_buf_compact(sta->rx_rate, rate_buf));

		hr = sta->in / 3600;
		min = (sta->in % 3600) / 60;
		sec = sta->in - hr * 3600 - min * 60;
		ret += websWrite(wp, ", \"%02d:%02d:%02d\"", hr, min, sec);

		ret += websWrite(wp, "]");
	}

	for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((sw_mode() == SW_MODE_REPEATER)
			&& (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;
#endif
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			sprintf(name_vif, "wl%d.%d", unit, i);

			memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;
				if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

				if (firstRow == 1)
					firstRow = 0;
				else
					ret += websWrite(wp, ", ");

				ret += websWrite(wp, "[");

				ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[ii], ea));

				ret += websWrite(wp, ", \"%s\"", print_rate_buf_compact(sta->tx_rate, rate_buf));
				ret += websWrite(wp, ", \"%s\"", print_rate_buf_compact(sta->rx_rate, rate_buf));

				hr = sta->in / 3600;
				min = (sta->in % 3600) / 60;
				sec = sta->in - hr * 3600 - min * 60;
				ret += websWrite(wp, ", \"%02d:%02d:%02d\"", hr, min, sec);

				ret += websWrite(wp, "]");
			}
		}
	}

	/* error/exit */
exit:
	if (auth) free(auth);

	return ret;
}
#endif

int
ej_wl_sta_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_sta_list(eid, wp, argc, argv, 0);
}

#ifndef RTCONFIG_QTN
int
ej_wl_sta_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_sta_list(eid, wp, argc, argv, 1);
}

int
ej_wl_sta_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_sta_list(eid, wp, argc, argv, 2);
}

#else
extern int ej_wl_sta_list_5g(int eid, webs_t wp, int argc, char_t **argv);
#endif

#ifdef RTCONFIG_STAINFO
int
ej_wl_stainfo_list_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(eid, wp, argc, argv, 0);
}

#ifndef RTCONFIG_QTN
int
ej_wl_stainfo_list_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(eid, wp, argc, argv, 1);
}

int
ej_wl_stainfo_list_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_stainfo_list(eid, wp, argc, argv, 2);
}

#else
extern int ej_wl_stainfo_list_5g(int eid, webs_t wp, int argc, char_t **argv);
#endif
#endif

int ej_get_wlstainfo_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char word[64], *next;
	int unit = 0;
	int haveInfo = 0;

	websWrite(wp, "{");

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		char tmp[128], prefix[] = "wlXXXXXXXXXX_";
		char *name;
		struct maclist *auth = NULL;
		int mac_list_size;
		int i, firstRow = 1;
		char ea[ETHER_ADDR_STR_LEN];
		scb_val_t scb_val;
		char name_vif[] = "wlX.Y_XXXXXXXXXX";
		int ii;
		sta_info_t *sta;
		char alias[16];
		int rssi;

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_WIRELESSREPEATER
		if ((sw_mode() == SW_MODE_REPEATER)
			&& (nvram_get_int("wlc_band") == unit))
		{
			sprintf(name_vif, "wl%d.%d", unit, 1);
			name = name_vif;
		}
#endif

		if (!strlen(name))
			goto exit;

		/* buffers and length */
		mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
		auth = malloc(mac_list_size);

		if (!auth)
			goto exit;

		memset(auth, 0, mac_list_size);

		/* query wl for authenticated sta list */
		strcpy((char*)auth, "authe_sta_list");
		if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
			goto exit;

		if (auth->count) {
			memset(alias, 0, sizeof(alias));
			snprintf(alias, sizeof(alias), "%s", unit ? (unit == 2 ? "5G1" : "5G") : "2G");

			if (haveInfo)
				websWrite(wp, ",");
			websWrite(wp, "\"%s\":[", alias);
			haveInfo = 1;
		}

		/* build authenticated sta list */
		for (i = 0; i < auth->count; ++i) {
			sta = wl_sta_info(name, &auth->ea[i]);
			if (!sta) continue;
			if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

			if (firstRow == 1)
				firstRow = 0;
			else
				websWrite(wp, ",");

			memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
			if (wl_ioctl(name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
				rssi = 0;
			else
				rssi =  scb_val.val;

			websWrite(wp, "{\"mac\":\"%s\",\"rssi\":%d}", ether_etoa((void *)&auth->ea[i], ea), rssi);
		}

		if (!firstRow)
			websWrite(wp, "]");

		for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
			if ((sw_mode() == SW_MODE_REPEATER)
				&& (unit == nvram_get_int("wlc_band")) && (i == 1))
				break;
#endif
			sprintf(prefix, "wl%d.%d_", unit, i);
			if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			{
				sprintf(name_vif, "wl%d.%d", unit, i);

				memset(auth, 0, mac_list_size);

				/* query wl for authenticated sta list */
				strcpy((char*)auth, "authe_sta_list");
				if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
					goto exit;

				if (auth->count) {
					memset(alias, 0, sizeof(alias));
					snprintf(alias, sizeof(alias), "%s_%d", unit ? (unit == 2 ? "5G1" : "5G") : "2G", i);

					if (haveInfo)
						websWrite(wp, ",");
					websWrite(wp, "\"%s\":[", alias);
					haveInfo = 1;
				}

				for (ii = 0; ii < auth->count; ii++) {
					sta = wl_sta_info(name_vif, &auth->ea[ii]);
					if (!sta) continue;
					if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

					if (firstRow == 1)
						firstRow = 0;
					else
						websWrite(wp, ",");

					memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
					if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
						rssi = 0;
					else
						rssi =  scb_val.val;

					websWrite(wp, "{\"mac\":\"%s\",\"rssi\":%d}", ether_etoa((void *)&auth->ea[ii], ea), rssi);
				}

				if (!firstRow)
					websWrite(wp, "]");
			}
		}
exit:
		if (auth) free(auth);

		unit++;
	}

	websWrite(wp, "}");

	return 0;
}

// no WME in WL500gP V2
// MAC/associated/authorized
int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv) {
	int unit = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	struct maclist *auth = NULL;
	int mac_list_size;
	int i, firstRow = 1;
	char ea[ETHER_ADDR_STR_LEN];
	char *value;
	char word[256], *next;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	int ii;
	int ret = 0;
	sta_info_t *sta;

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);
	//wme = malloc(mac_list_size);

	//if (!auth || !wme)
	if (!auth)
		goto exit;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
#ifdef RTCONFIG_QTN
		if (unit) {
			if (rpc_qtn_ready()) {
				if (firstRow == 1)
					firstRow = 0;
				else
					ret += websWrite(wp, ", ");
				ret += ej_wl_sta_list_5g(eid, wp, argc, argv);
			}
			goto exit;
		}
#endif
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

		memset(auth, 0, mac_list_size);
		//memset(wme, 0, mac_list_size);

		/* query wl for authenticated sta list */
		strcpy((char*)auth, "authe_sta_list");
		if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
			goto exit;

		/* query wl for WME sta list */
		/*strcpy((char*)wme, "wme_sta_list");
		if (wl_ioctl(name, WLC_GET_VAR, wme, mac_list_size))
			goto exit;*/

		/* build authenticated/associated sta list */
		for (i = 0; i < auth->count; ++i) {
			sta = wl_sta_info(name, &auth->ea[i]);
			if (!sta) continue;
			if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

			if (firstRow == 1)
				firstRow = 0;
			else
				ret += websWrite(wp, ", ");

			ret += websWrite(wp, "[");

			ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[i], ea));

			value = (sta->flags & WL_STA_ASSOC) ? "Yes" : "No";
			ret += websWrite(wp, ", \"%s\"", value);

			value = (sta->flags & WL_STA_AUTHO) ? "Yes" : "No";
			ret += websWrite(wp, ", \"%s\"", value);

			/*value = (find_ethaddr_in_list((void *)&auth->ea[i], wme))?"Yes":"No";
			ret += websWrite(wp, ", \"%s\"", value);*/

			ret += websWrite(wp, "]");
		}

		for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
			if ((sw_mode() == SW_MODE_REPEATER)
				&& (unit == nvram_get_int("wlc_band")) && (i == 1))
				break;
#endif
			sprintf(prefix, "wl%d.%d_", unit, i);
			if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			{
				sprintf(name_vif, "wl%d.%d", unit, i);

				memset(auth, 0, mac_list_size);

				/* query wl for authenticated sta list */
				strcpy((char*)auth, "authe_sta_list");
				if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
					goto exit;

				for (ii = 0; ii < auth->count; ii++) {
					sta = wl_sta_info(name_vif, &auth->ea[ii]);
					if (!sta) continue;
					if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

					if (firstRow == 1)
						firstRow = 0;
					else
						ret += websWrite(wp, ", ");

					ret += websWrite(wp, "[");

					ret += websWrite(wp, "\"%s\"", ether_etoa((void *)&auth->ea[ii], ea));

					value = (sta->flags & WL_STA_ASSOC) ? "Yes" : "No";
					ret += websWrite(wp, ", \"%s\"", value);

					value = (sta->flags & WL_STA_AUTHO) ? "Yes" : "No";
					ret += websWrite(wp, ", \"%s\"", value);

					ret += websWrite(wp, "]");
				}
			}
		}

		unit++;
	}

	/* error/exit */
exit:
	if (auth) free(auth);
	//if (wme) free(wme);

	return ret;
}

/* WPS ENR mode APIs */
typedef struct wlc_ap_list_info
{
#if 0
	bool	used;
#endif
	uint8	ssid[33];
	uint8	ssidLen;
	uint8	BSSID[6];
#if 0
	uint8	*ie_buf;
	uint32	ie_buflen;
#endif
	uint8	channel;
#if 0
	uint8	wep;
	bool	scstate;
#endif
} wlc_ap_list_info_t;

#define WLC_MAX_AP_SCAN_LIST_LEN	128
#define WLC_SCAN_RETRY_TIMES		5

static wlc_ap_list_info_t ap_list[WLC_MAX_AP_SCAN_LIST_LEN];

#ifdef __CONFIG_DHDAP__
#define MAX_SSID_LEN		32
#define WL_EVENT_TIMEOUT	10

typedef struct escan_wksp_s {
	uint8 packet[4096];
	fd_set fdset;
	int fdmax;
	int event_fd;
} escan_wksp_t;

escan_wksp_t *d_info;

bool escan_inprogress;

struct escan_bss {
	struct escan_bss *next;
	wl_bss_info_t bss[1];
};

struct escan_bss *escan_bss_head; /* raw escan results */
struct escan_bss *escan_bss_tail;

/* open a UDP packet to event dispatcher for receiving/sending data */
static int
escan_open_eventfd()
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int fd = -1;

	/* open loopback socket to communicate with event dispatcher */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(EAPD_WKSP_WLEVENT_UDP_SPORT);

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		dbg("Unable to create loopback socket\n");
		goto exit;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		dbg("Unable to setsockopt to loopback socket %d.\n", fd);
		goto exit;
	}

	if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		dbg("Unable to bind to loopback socket %d\n", fd);
		goto exit;
	}

	d_info->event_fd = fd;

	return 0;

	/* error handling */
exit:
	if (fd != -1) {
		close(fd);
	}

	return errno;
}

static int
validate_wlpvt_message(int bytes, uint8 *dpkt)
{
	bcm_event_t *pvt_data;

	/* the message should be at least the header to even look at it */
	if (bytes < sizeof(bcm_event_t) + 2) {
		dbg("Invalid length of message\n");
		goto error_exit;
	}
	pvt_data = (bcm_event_t *)dpkt;
	if (ntohs(pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG) {
		dbg("%s: not vendor specifictype\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (pvt_data->bcm_hdr.version != BCMILCP_BCM_SUBTYPEHDR_VERSION) {
		dbg("%s: subtype header version mismatch\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (ntohs(pvt_data->bcm_hdr.length) < BCMILCP_BCM_SUBTYPEHDR_MINLENGTH) {
		dbg("%s: subtype hdr length not even minimum\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (bcmp(&pvt_data->bcm_hdr.oui[0], BRCM_OUI, DOT11_OUI_LEN) != 0) {
		dbg("%s: validate_wlpvt_message: not BRCM OUI\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	/* check for wl dcs message types */
	switch (ntohs(pvt_data->bcm_hdr.usr_subtype)) {
		case BCMILCP_BCM_SUBTYPE_EVENT:
			break;
		default:
			goto error_exit;
			break;
	}
	return 0; /* good packet may be this is destined to us */
error_exit:
	return -1;
}

void
escan_main_loop(struct timeval *tv)
{
	fd_set fdset;
	int width, status = 0, bytes, len;
	uint8 *pkt;
	bcm_event_t *pvt_data;
	int err;
	uint32 escan_event_status;
	wl_escan_result_t *escan_data = NULL;
	struct escan_bss *result;

	/* init file descriptor set */
	FD_ZERO(&d_info->fdset);
	d_info->fdmax = -1;

	/* build file descriptor set now to save time later */
	if (d_info->event_fd != -1) {
		FD_SET(d_info->event_fd, &d_info->fdset);
		d_info->fdmax = d_info->event_fd;
	}

	pkt = d_info->packet;
	len = sizeof(d_info->packet);
	width = d_info->fdmax + 1;
	fdset = d_info->fdset;

	/* listen to data availible on all sockets */
	status = select(width, &fdset, NULL, NULL, tv);

	if ((status == -1 && errno == EINTR) || (status == 0))
		return;

	if (status <= 0) {
		dbg("err from select: %s", strerror(errno));
		return;
	}

	/* handle brcm event */
	if (d_info->event_fd != -1 && FD_ISSET(d_info->event_fd, &fdset)) {
		char *ifname = (char *)pkt;
		struct ether_header *eth_hdr = (struct ether_header *)(ifname + IFNAMSIZ);
		uint16 ether_type = 0;
		uint32 evt_type;

		if ((bytes = recv(d_info->event_fd, pkt, len, 0)) <= 0)
			return;

		bytes -= IFNAMSIZ;

		if ((ether_type = ntohs(eth_hdr->ether_type) != ETHER_TYPE_BRCM)) {
			return;
		}

		if ((err = validate_wlpvt_message(bytes, (uint8 *)eth_hdr)))
			return;

		pvt_data = (bcm_event_t *)(ifname + IFNAMSIZ);
		evt_type = ntoh32(pvt_data->event.event_type);

		switch (evt_type) {
			case WLC_E_ESCAN_RESULT:
				{
					if (!escan_inprogress) {
						dbg("Escan not triggered from rc\n");
						return;
					}

					escan_event_status = ntoh32(pvt_data->event.status);
					escan_data = (wl_escan_result_t*)(pvt_data + 1);

					if (escan_event_status == WLC_E_STATUS_PARTIAL) {
						wl_bss_info_t *bi = &escan_data->bss_info[0];
						wl_bss_info_t *bss;

						/* check if we've received info of same BSSID */
						for (result = escan_bss_head;
								result;	result = result->next) {
							bss = result->bss;

							if (!memcmp(bi->BSSID.octet,
								bss->BSSID.octet,
								ETHER_ADDR_LEN) &&
								CHSPEC_BAND(bi->chanspec) ==
								CHSPEC_BAND(bss->chanspec) &&
								bi->SSID_len ==	bss->SSID_len &&
								! memcmp(bi->SSID, bss->SSID,
								bi->SSID_len)) {
								break;
							}
						}

						if (!result) {
							/* New BSS. Allocate memory and save it */
							struct escan_bss *ebss = (struct escan_bss *)malloc(
								OFFSETOF(struct escan_bss, bss)
								+ bi->length);

							if (!ebss) {
								dbg("can't allocate memory"
										"for escan bss");
								break;
							}

							ebss->next = NULL;
							memcpy(&ebss->bss, bi, bi->length);

							if (escan_bss_tail) {
								escan_bss_tail->next = ebss;
							} else {
								escan_bss_head =
								ebss;
							}

							escan_bss_tail = ebss;
						} else if (bi->RSSI != WLC_RSSI_INVALID) {
							/* We've got this BSS. Update RSSI
							   if necessary
							   */
							bool preserve_maxrssi = FALSE;
							if (((bss->flags &
								WL_BSS_FLAGS_RSSI_ONCHANNEL) ==
								(bi->flags &
								WL_BSS_FLAGS_RSSI_ONCHANNEL)) &&
								((bss->RSSI == WLC_RSSI_INVALID) ||
								(bss->RSSI < bi->RSSI))) {
								/* Preserve max RSSI if the
								   measurements are both
								   on-channel or both off-channel
								   */
								preserve_maxrssi = TRUE;
							} else if ((bi->flags &
								WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
								(bss->flags &
								WL_BSS_FLAGS_RSSI_ONCHANNEL) == 0) {
								/* Preserve the on-channel RSSI
								   measurement if the
								   new measurement is off channel
								   */
								preserve_maxrssi = TRUE;
								bss->flags |=
								WL_BSS_FLAGS_RSSI_ONCHANNEL;
							}

							if (preserve_maxrssi) {
								bss->RSSI = bi->RSSI;
								bss->SNR = bi->SNR;
								bss->phy_noise = bi->phy_noise;
							}
						}
					} else if (escan_event_status == WLC_E_STATUS_SUCCESS) {
						escan_inprogress = FALSE;
					} else {
						dbg("sync_id: %d, status:%d, misc."
							"error/abort\n",
							escan_data->sync_id, status);

						escan_bss_head = NULL;
						escan_bss_tail = NULL;
						escan_inprogress = FALSE;
					}
					break;
				}
			default:
				break;
		}
	}
}

/* listen to sockets and receive escan results */
static int
get_scan_escan(char *scan_buf, uint buf_len)
{
	int err;
	struct timeval tv, tv_tmp;
	time_t timeout;
	int len;
	struct escan_bss *result;
	struct escan_bss *next;
	wl_scan_results_t* s_result = (wl_scan_results_t*)scan_buf;
	wl_bss_info_t *bi = s_result->bss_info;
	wl_bss_info_t *bss;

	d_info = (escan_wksp_t*)malloc(sizeof(escan_wksp_t));
	d_info->fdmax = -1;
	d_info->event_fd = -1;
	err = escan_open_eventfd();
	if (err) return -1;

	tv.tv_usec = 0;
	tv.tv_sec = WL_EVENT_TIMEOUT;
	timeout = uptime() + WL_EVENT_TIMEOUT;

	escan_inprogress = TRUE;

	escan_bss_head = NULL;
	escan_bss_tail = NULL;

	while ((uptime() < timeout) && escan_inprogress) {
		memcpy(&tv_tmp, &tv, sizeof(tv));
		escan_main_loop(&tv_tmp);
	}

	escan_inprogress = FALSE;

	s_result->count = 0;
	len = buf_len - WL_SCAN_RESULTS_FIXED_SIZE;
	for (result = escan_bss_head; result; result = result->next) {
		bss = result->bss;
		if (len < bss->length) {
			dbg("Memory not enough for scan results\n");
			break;
		}
		memcpy(bi, bss, bss->length);
		bi = (wl_bss_info_t*)((int8*)bi + bss->length);
		len -= bss->length;
		s_result->count++;
	}

	for (result = escan_bss_head; result; result = next) {
		next = result->next;
		free(result);
	}

	/* close event dispatcher socket */
	if (d_info->event_fd != -1) {
		close(d_info->event_fd);
	}

	if (d_info)
		free(d_info);

	return 0;
}

static char *
wl_get_scan_results_escan(char *ifname, chanspec_t chanspec, int ctl_ch, int ctl_ch_tmp)
{
	int ret, retry_times = 0;
	wl_escan_params_t *params = NULL;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params) + NUMCHANS * sizeof(uint16);
	int org_scan_time = 20, scan_time = 40;
	int wlscan_debug = 0;
	char chanbuf[CHANSPEC_STR_LEN];
	int unit, i;
	int count, scount = 0;
	wl_uint32_list_t *list;
	char data_buf[WLC_IOCTL_MAXLEN];
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	chanspec_t c = WL_CHANSPEC_BW_20;

	if (nvram_match("wlscan_debug", "1"))
		wlscan_debug = 1;

	if (wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return NULL;

	params = (wl_escan_params_t*)malloc(params_size);
	if (params == NULL) {
		return NULL;
	}

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	memset(params, 0, params_size);
	params->params.bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
	memcpy(&params->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->params.scan_type = (nvram_match(strcat_r(prefix, "reg_mode", tmp), "h") && !nvram_match(strcat_r(prefix, "mode", tmp), "psta")) ? WL_SCANFLAGS_PASSIVE : 0;
	params->params.nprobes = -1;
	params->params.active_time = -1;
	params->params.passive_time = -1;
	params->params.home_time = -1;
	params->params.channel_num = 0;

	c |= unit ? WL_CHANSPEC_BAND_5G : WL_CHANSPEC_BAND_2G;
	memset(data_buf, 0, WLC_IOCTL_MAXLEN);
	ret = wl_iovar_getbuf(ifname, "chanspecs", &c, sizeof(chanspec_t),
		data_buf, WLC_IOCTL_MAXLEN);
	if (ret < 0)
		dbg("failed to get valid chanspec list\n");
	else {
		list = (wl_uint32_list_t *)data_buf;
		count = dtoh32(list->count);

		if (count && !(count > (data_buf + sizeof(data_buf) - (char *)&list->element[0])/sizeof(list->element[0]))) {
			for (i = 0; i < count; i++) {
				c = (chanspec_t)dtoh32(list->element[i]);
				params->params.channel_list[scount++] = c;
			}

			params->params.channel_num = htod32(scount & WL_SCAN_PARAMS_COUNT_MASK);
			params_size = WL_SCAN_PARAMS_FIXED_SIZE + scount * sizeof(uint16);
		}
	}

	params->version = htod32(ESCAN_REQ_VERSION);
	params->action = htod16(WL_SCAN_ACTION_START);

	srand((unsigned int)uptime());
	params->sync_id = htod16(rand() & 0xffff);

	params_size += OFFSETOF(wl_escan_params_t, params);

	/* extend scan channel time to get more AP probe resp */
	wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
	if (org_scan_time < scan_time)
		wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time,	sizeof(scan_time));

	while ((ret = wl_iovar_set(ifname, "escan", params, params_size)) < 0 &&
				retry_times++ < WLC_SCAN_RETRY_TIMES) {
		if (wlscan_debug)
		dbg("set escan command failed, retry %d\n", retry_times);
		sleep(1);
	}

	free(params);

	/* restore original scan channel time */
	wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));

	if (ret == 0)
		ret = get_scan_escan(scan_result, WLC_SCAN_RESULT_BUF_LEN);

	if (chanspec != 0) {
		dbg("restore original chanspec: %s (0x%x)\n", wf_chspec_ntoa(chanspec, chanbuf), chanspec);
		if (wl_cap(unit, "bgdfs") && (((ctl_ch >= 100) && (ctl_ch_tmp <= 48)) || ((ctl_ch < 100) && (ctl_ch_tmp >= 149))))
			wl_iovar_setint(ifname, "dfs_ap_move", chanspec);
		else
		{
			wl_iovar_setint(ifname, "chanspec", chanspec);
			wl_iovar_setint(ifname, "acs_update", -1);
		}
	}

	if (ret < 0)
		return NULL;

	return scan_result;
}
#endif

static char *
wl_get_scan_results(char *ifname, chanspec_t chanspec, int ctl_ch, int ctl_ch_tmp)
{
	int ret, retry_times = 0;
	wl_scan_params_t *params;
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + NUMCHANS * sizeof(uint16);
	int org_scan_time = 20, scan_time = 40;
	char chanbuf[CHANSPEC_STR_LEN];
	int unit;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";

	if (wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return NULL;

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		return NULL;
	}

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = (nvram_match(strcat_r(prefix, "reg_mode", tmp), "h") && !nvram_match(strcat_r(prefix, "mode", tmp), "psta")) ? WL_SCANFLAGS_PASSIVE : 0;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	/* extend scan channel time to get more AP probe resp */
	wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
	if (org_scan_time < scan_time)
		wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time,	sizeof(scan_time));

	while ((ret = wl_ioctl(ifname, WLC_SCAN, params, params_size)) < 0 &&
				retry_times++ < WLC_SCAN_RETRY_TIMES) {
		dbg("set scan command failed, retry %d\n", retry_times);
		sleep(1);
	}

	free(params);

	/* restore original scan channel time */
	wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));

	sleep(2);

	if (ret == 0) {
		list->buflen = htod32(WLC_SCAN_RESULT_BUF_LEN);
		ret = wl_ioctl(ifname, WLC_SCAN_RESULTS, scan_result, WLC_SCAN_RESULT_BUF_LEN);
		if (ret < 0)
			printf("get scan result failed\n");
	}

	if (chanspec != 0) {
		dbg("restore original chanspec: %s (0x%x)\n", wf_chspec_ntoa(chanspec, chanbuf), chanspec);
		if (wl_cap(unit, "bgdfs") && (((ctl_ch >= 100) && (ctl_ch_tmp <= 48)) || ((ctl_ch < 100) && (ctl_ch_tmp >= 149))))
			wl_iovar_setint(ifname, "dfs_ap_move", chanspec);
		else
		{
			wl_iovar_setint(ifname, "chanspec", chanspec);
			wl_iovar_setint(ifname, "acs_update", -1);
		}
	}

	if (ret < 0)
		return NULL;

	return scan_result;
}

int
ej_nat_accel_status(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;

	retval += websWrite(wp, "%d", nvram_get_int("ctf_fa_mode"));

	return retval;
}

static int
wl_scan(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char *name = NULL;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	uint i, ap_count = 0;
	char ssid_str[128];
	char macstr[18];
	int retval = 0, ctl_ch;
	char chanbuf[CHANSPEC_STR_LEN];
	chanspec_t chspec_cur = 0, chanspec = 0;
	chanspec_t chspec_tmp = 0;
	int ctl_ch_tmp = 0;
#if defined(RTCONFIG_DHDAP) && !defined(RTCONFIG_BCM7)
	chanspec_t chspec_tar = 0;
	char buf_sm[WLC_IOCTL_SMLEN];
	wl_dfs_ap_move_status_t *status = (wl_dfs_ap_move_status_t*) buf_sm;
#endif
#ifdef __CONFIG_DHDAP__
	int is_dhd = 0;
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
#ifdef __CONFIG_DHDAP__
	is_dhd = !dhd_probe(name);
#endif

	ctl_ch = wl_control_channel(unit);
	if (nvram_match(strcat_r(prefix, "reg_mode", tmp), "h") &&
		!nvram_match(strcat_r(prefix, "mode", tmp), "psta")) {
		if (wl_iovar_get(name, "chanspec", &chspec_cur, sizeof(chanspec_t)) < 0) {
			dbg("get current chanpsec failed\n");
			return 0;
		}

		if (((ctl_ch > 48) && (ctl_ch < 149))
#ifdef RTCONFIG_BW160M
			|| ((ctl_ch <= 48) && CHSPEC_IS160(chspec_cur))
#endif
		) {
#ifdef RTCONFIG_HND_ROUTER_AX
			char buf[WLC_IOCTL_MAXLEN];
			wl_dfs_status_t *dfs_status;

			memset(buf, 0, sizeof(buf));
			strcpy(buf, "dfs_status");
			wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf));

			dfs_status = (wl_dfs_status_t *) buf;
			dfs_status->state = dtoh32(dfs_status->state);

			if (dfs_status->state != WL_DFS_CACSTATE_IDLE && dfs_status->state != WL_DFS_CACSTATE_ISM)
#endif
			{
				if (!with_non_dfs_chspec(name))
				{
					dbg("%s scan rejected under DFS mode\n", name);
					return 0;
				}
				else
				{
					dbg("current chanspec: %s (0x%x)\n", wf_chspec_ntoa(chspec_cur, chanbuf), chspec_cur);

					chspec_tmp = (((nvram_get_hex(strcat_r(prefix, "band5grp", tmp)) & WL_5G_BAND_4) && (ctl_ch < 100)) ? select_chspec_with_band_bw(name, 4, 3, chspec_cur) : select_chspec_with_band_bw(name, 1, 3, chspec_cur));
					if (!chspec_tmp && (nvram_get_hex(strcat_r(prefix, "band5grp", tmp)) & WL_5G_BAND_4))
						chspec_tmp = select_chspec_with_band_bw(name, 4, 3, chspec_cur);

					if (chspec_tmp != 0) {
						dbg("switch to chanspec: %s (0x%x)\n", wf_chspec_ntoa(chspec_tmp, chanbuf), chspec_tmp);
						wl_iovar_setint(name, "chanspec", chspec_tmp);
						wl_iovar_setint(name, "acs_update", -1);

						chanspec = chspec_cur;
						ctl_ch_tmp = wf_chspec_ctlchan(chspec_tmp);
					}
				}
			}
		}
#if defined(RTCONFIG_DHDAP) && !defined(RTCONFIG_BCM7)
		else if (wl_cap(unit, "bgdfs")) {
			if (wl_iovar_get(name, "dfs_ap_move", &buf_sm[0], WLC_IOCTL_SMLEN) < 0) {
				dbg("get dfs_ap_move status failure\n");
				return 0;
			}

			if (status->version != WL_DFS_AP_MOVE_VERSION)
				return 0;

			if (status->move_status != (int8) DFS_SCAN_S_IDLE) {
				chspec_tar = status->chanspec;
				if (chspec_tar != 0 && chspec_tar != INVCHANSPEC) {
					chanspec = chspec_tar;
					wf_chspec_ntoa(chspec_tar, chanbuf);
					dbg("AP Target Chanspec %s (0x%x)\n", chanbuf, chspec_tar);
				}

				if (status->move_status == (int8) DFS_SCAN_S_INPROGESS)
					wl_iovar_setint(name, "dfs_ap_move", -2);
			}
		}
#endif
	}

#ifdef __CONFIG_DHDAP__
	if (is_dhd && !nvram_match(strcat_r(prefix, "mode", tmp), "wds")) {
		if (wl_get_scan_results_escan(name, chanspec, ctl_ch, ctl_ch_tmp) == NULL) {
			return 0;
		}
	}
	else
#endif
	if (wl_get_scan_results(name, chanspec, ctl_ch, ctl_ch_tmp) == NULL) {
		return 0;
	}

	if (list->count == 0)
		return 0;
	else if (
#ifdef __CONFIG_DHDAP__
			!is_dhd &&
#endif
			list->version != WL_BSS_INFO_VERSION
			&& list->version != LEGACY_WL_BSS_INFO_VERSION
#ifdef RTCONFIG_BCMWL6
			&& list->version != LEGACY2_WL_BSS_INFO_VERSION
#endif
	) {
		dbg("Sorry, your driver has bss_info_version %d "
		    "but this program supports only version %d.\n",
		    list->version, WL_BSS_INFO_VERSION);
		return 0;
	}

	memset(ap_list, 0, sizeof(ap_list));
	bi = list->bss_info;
	for (i = 0; i < list->count; i++) {
		/* Convert version 107 to 109 */
		if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}

		if (bi->ie_length) {
			if (ap_count < WLC_MAX_AP_SCAN_LIST_LEN) {
#if 0
				ap_list[ap_count].used = TRUE;
#endif
				memcpy(ap_list[ap_count].BSSID, (uint8 *)&bi->BSSID, 6);
				strncpy((char *)ap_list[ap_count].ssid, (char *)bi->SSID, bi->SSID_len);
				ap_list[ap_count].ssid[bi->SSID_len] = '\0';
				ap_list[ap_count].ssidLen= bi->SSID_len;
#if 0
				ap_list[ap_count].ie_buf = (uint8 *)(((uint8 *)bi) + bi->ie_offset);
				ap_list[ap_count].ie_buflen = bi->ie_length;
#endif
				if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
					ap_list[ap_count].channel = bi->ctl_ch;
				else
					ap_list[ap_count].channel = bi->chanspec & WL_CHANSPEC_CHAN_MASK;
#if 0
				ap_list[ap_count].wep = bi->capability & DOT11_CAP_PRIVACY;
#endif
				ap_count++;

				if (ap_count >= WLC_MAX_AP_SCAN_LIST_LEN)
					break;
			}
		}
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

	sprintf(tmp, "%-4s%-33s%-18s\n", "Ch", "SSID", "BSSID");
	dbg("\n%s", tmp);
	if (ap_count)
	{
		retval += websWrite(wp, "[");
		for (i = 0; i < ap_count; i++)
		{
			memset(ssid_str, 0, sizeof(ssid_str));
			char_to_ascii(ssid_str, trim_r((char *)ap_list[i].ssid));

			ether_etoa((const unsigned char *) &ap_list[i].BSSID, macstr);

			dbg("%-4d%-33s%-18s\n",
				ap_list[i].channel,
				ap_list[i].ssid,
				macstr
			);

			if (!i)
				retval += websWrite(wp, "[\"%s\", \"%s\"]", ssid_str, macstr);
			else
				retval += websWrite(wp, ", [\"%s\", \"%s\"]", ssid_str, macstr);
		}
		retval += websWrite(wp, "]");
		dbg("\n");
	}
	else
		retval += websWrite(wp, "[]");

//	return ap_list;
	return retval;
}

int
ej_wl_scan(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

int
ej_wl_scan_2g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 0);
}

#ifndef RTCONFIG_QTN
int
ej_wl_scan_5g(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 1);
}
#endif

int
ej_wl_scan_5g_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return wl_scan(eid, wp, argc, argv, 2);
}

#ifdef RTCONFIG_PROXYSTA
static int
wl_autho(char *name, struct ether_addr *ea)
{
	char buf[sizeof(sta_info_t)];

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf))) {
		sta_info_t *sta = (sta_info_t *)buf;
		uint32 f = sta->flags;

		if (f & WL_STA_AUTHO)
			return 1;
	}

	return 0;
}

static int psta_auth = 0;

int
ej_wl_auth_psta(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	struct maclist *mac_list = NULL;
	int mac_list_size, i, unit;
	int retval = 0, psta = 0;
	struct ether_addr bssid;
	unsigned char bssid_null[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	wlc_ssid_t ssid = { 0, "" };
	int psta_debug = 0;

	if (nvram_match("psta_debug", "1"))
		psta_debug = 1;

	unit = nvram_get_int("wlc_band");
#ifdef RTCONFIG_QTN
	if (unit == 1) {
		return ej_wl_auth_psta_qtn(eid, wp, argc, argv);
	}
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (!nvram_match(strcat_r(prefix, "mode", tmp), "psta") &&
	    !nvram_match(strcat_r(prefix, "mode", tmp), "psr")
#ifdef RTCONFIG_HND_ROUTER_AX
	    && !nvram_match(strcat_r(prefix, "mode", tmp), "wet")
#endif
	)
		goto PSTA_ERR;

	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (wl_ioctl(name, WLC_GET_SSID, &ssid, sizeof(ssid)))
		goto PSTA_ERR;
	else if (!nvram_match(strcat_r(prefix, "ssid", tmp), (const char *) ssid.SSID))
		goto PSTA_ERR;

	if (wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0)
		goto PSTA_ERR;
	else if (!memcmp(&bssid, bssid_null, 6))
		goto PSTA_ERR;

	/* buffers and length */
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);

	if (!mac_list)
		goto PSTA_ERR;

	memset(mac_list, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)mac_list, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
		free(mac_list);
		goto PSTA_ERR;
	}

	/* query sta_info for each STA */
	if (mac_list->count)
	{
		if (nvram_match(strcat_r(prefix, "akm", tmp), ""))
			psta = 1;
		else
		for (i = 0, psta = 2; i < mac_list->count; i++) {
			if (wl_autho(name, &mac_list->ea[i]))
			{
				psta = 1;
				break;
			}
		}
	}

	free(mac_list);
PSTA_ERR:
	if (nvram_match(strcat_r(prefix, "mode", tmp), "psta") ||
	    nvram_match(strcat_r(prefix, "mode", tmp), "psr")
#ifdef RTCONFIG_HND_ROUTER_AX
            || nvram_match(strcat_r(prefix, "mode", tmp), "wet")
#endif
	) {
		if (psta == 1)
		{
			if (psta_debug) dbg("connected\n");
			psta_auth = 0;
		}
		else if (psta == 2)
		{
			if (psta_debug) dbg("authorization failed\n");
			psta_auth = 1;
		}
		else
		{
			if (psta_debug) dbg("disconnected\n");
		}
	}

	if(json_support){
		retval += websWrite(wp, "{");
		retval += websWrite(wp, "\"wlc_state\":\"%d\"", psta);
		retval += websWrite(wp, ",\"wlc_state_auth\":\"%d\"", psta_auth);
		retval += websWrite(wp, "}");
	}else{
		retval += websWrite(wp, "wlc_state=%d;", psta);
		retval += websWrite(wp, "wlc_state_auth=%d;", psta_auth);
	}
	return retval;
}
#endif


int
ej_wl_status_2g_array(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int ii = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char *temp;

	for (ii = 0; ii < DEV_NUMIFS; ii++) {
		sprintf(nv_param, "wl%d_unit", ii);
		temp = nvram_get(nv_param);

		if (temp && strlen(temp) > 0)
		{
			retval += ej_wl_status_array(eid, wp, argc, argv, ii);
			retval += websWrite(wp, "\n");
		}
	}

	return retval;
}

static int
#ifdef RTCONFIG_HND_ROUTER_AX
dump_bss_info_array(int eid, webs_t wp, int argc, char_t **argv, wl_bss_info_v109_1_t *bi)
#else
dump_bss_info_array(int eid, webs_t wp, int argc, char_t **argv, wl_bss_info_t *bi)
#endif
{
	char ssidbuf[SSID_FMT_BUF_LEN*2], ssidbuftmp[SSID_FMT_BUF_LEN];
	char chspec_str[CHANSPEC_STR_LEN];
	wl_bss_info_107_t *old_bi;
	int retval = 0;

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
	} else {
		/* do endian swap and format conversion for chanspec if we have
		* not created it from legacy bi above
		*/
		bi->chanspec = wl_chspec_from_driver(bi->chanspec);
	}

	wl_format_ssid(ssidbuftmp, bi->SSID, bi->SSID_len);

	if (str_escape_quotes(ssidbuf, ssidbuftmp, sizeof(ssidbuf)) == 0 )
		strlcpy(ssidbuf, ssidbuftmp, sizeof(ssidbuf));

	retval += websWrite(wp, "\"%s\",", ssidbuf);
	retval += websWrite(wp, "\"%d\",", (int16)(dtoh16(bi->RSSI)));

	/*
	 * SNR has valid value in only 109 version.
	 * So print SNR for 109 version only.
	 */
	if (dtoh32(bi->version) == WL_BSS_INFO_VERSION) {
		retval += websWrite(wp, "\"%d\",", (int16)(dtoh16(bi->SNR)));
	} else {
		retval += websWrite(wp, "\"?\",");
	}

	retval += websWrite(wp, "\"%d\",", bi->phy_noise);
	retval += websWrite(wp, "\"%s\",", wf_chspec_ntoa(dtohchanspec(bi->chanspec), chspec_str));
	retval += websWrite(wp, "\"%s\",", wl_ether_etoa(&bi->BSSID));

	return retval;
}

static int
wl_status_array(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret;
	struct ether_addr bssid;
	wlc_ssid_t ssid;
	char ssidbuf[SSID_FMT_BUF_LEN*2], ssidbuftmp[SSID_FMT_BUF_LEN];
#ifdef RTCONFIG_HND_ROUTER_AX
	wl_bss_info_v109_1_t *bi;
#else
	wl_bss_info_t *bi;
#endif
	int retval = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0) {
			retval += websWrite(wp, "\"?\",\"?\",\"?\",\"?\",\"?\",\"?\",");
			return retval;
		}
#ifdef RTCONFIG_HND_ROUTER_AX
		bi = (wl_bss_info_v109_1_t*)(buf + 4);
#else
		bi = (wl_bss_info_t*)(buf + 4);
#endif
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
			retval += dump_bss_info_array(eid, wp, argc, argv, bi);
		else
			retval += websWrite(wp, "\"<error>\",\"\",\"\",\"\",\"\",\"\",");
	} else {
		retval += websWrite(wp, "\"\",\"\",\"\",\"\",\"\",\"\",");
	}

	return retval;
}

#ifdef RTCONFIG_IPV6
#define IPV6_CLIENT_LIST        "/tmp/ipv6_client_list"
#endif

int
ej_wl_status_array(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	struct maclist *auth;
	int mac_list_size;
	int i, ii, val = 0, ret = 0;
	char *arplist = NULL, *arplistptr;
	char *leaselist = NULL, *leaselistptr;
	char *ipv6list = NULL, *ipv6listptr;
	char *line;
	char hostnameentry[65];
	char ipentry[42], macentry[18];
	int found, foundipv6 = 0, noclients = 0;
	char rxrate[12], txrate[12];
	char ea[ETHER_ADDR_STR_LEN];
	scb_val_t scb_val;
	int hr, min, sec;
	sta_info_t *sta;
#ifdef RTCONFIG_BCMWL6
	wl_dfs_status_t *dfs_status;
	char chanspec_str[CHANSPEC_STR_LEN];
#endif
#ifdef RTCONFIG_QTN
	int res;
#endif

#ifdef RTCONFIG_PROXYSTA
	if (psta_exist_except(unit))
		return ret;
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_QTN
	if (unit && rpc_qtn_ready())
	{
		res = qcsapi_wifi_rfstatus((qcsapi_unsigned_int *) &val);
		if (res < 0)
			dbG("qcsapi_wifi_rfstatus error, return: %d\n", res);
		else
			val = !val;
	}
	else
#endif
	{
		wl_ioctl(name, WLC_GET_RADIO, &val, sizeof(val));
		val &= WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;
	}
#ifdef RTCONFIG_QTN
	if (unit && !rpc_qtn_ready())
		return ret;
	else
#endif
	if (val)
		return ret;

        switch (unit) {
        case 0:
		ret += websWrite(wp, "dataarray24 = [");
                break;
        case 1:
		ret += websWrite(wp, "dataarray5 = [");
		break;
        case 2:
		ret += websWrite(wp, "dataarray52 = [");
                break;
        }

	if (nvram_match(strcat_r(prefix, "mode", tmp), "wds")) {
		ret += websWrite(wp, "\"\",\"\",\"\",\"\",\"%d\",\"\",", wl_control_channel(unit));
	}
	else {
#ifdef RTCONFIG_QTN
		if (unit)
			ret += wl_status_5g_array(eid, wp, argc, argv);
		else
#endif
			ret += wl_status_array(eid, wp, argc, argv, unit);
	}

	if (nvram_match(strcat_r(prefix, "mode", tmp), "ap"))
	{
		if (nvram_match(strcat_r(prefix, "lazywds", tmp), "1") ||
			nvram_invmatch(strcat_r(prefix, "wds", tmp), ""))
			ret += websWrite(wp, "\"Hybrid\"");
		else	ret += websWrite(wp, "\"AP\"");
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
	{
		ret += websWrite(wp, "\"WDS\"");
		noclients = 1;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "sta"))
	{
		ret += websWrite(wp, "\"Stations\"");
		noclients = 1;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wet"))
	{
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
			&& (nvram_get_int("wlc_band") == unit))
			sprintf(prefix, "wl%d.%d_", unit, 1);
#endif
		ret += websWrite(wp, "\"Repeater ( SSID local: %s )\"", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
	}
#ifdef RTCONFIG_PROXYSTA
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "psta"))
	{
		if ((nvram_get_int("sw_mode") == SW_MODE_AP) &&
			(nvram_get_int("wlc_psta") == 1) &&
			(nvram_get_int("wlc_band") == unit))
		ret += websWrite(wp, "\"Media Bridge\"");
	}
#endif

// Close dataarray
	ret += websWrite(wp, "];\n");

// DFS status
#ifdef RTCONFIG_BCMWL6
	if (unit != 1)
		goto sta_list;

	if (nvram_match(strcat_r(prefix, "reg_mode", tmp), "off"))
		goto sta_list;

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "dfs_status");

	if (wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf)) < 0)
		goto sta_list;

	dfs_status = (wl_dfs_status_t *) buf;
	dfs_status->state = dtoh32(dfs_status->state);
	dfs_status->duration = dtoh32(dfs_status->duration);
	dfs_status->chanspec_cleared = wl_chspec_from_driver(dfs_status->chanspec_cleared);

	const char *dfs_cacstate_str[WL_DFS_CACSTATES] = {
		"Idle",
		"PRE-ISM Channel Availability Check (CAC)",
		"In-Service Monitoring (ISM)",
		"Channel Switching Announcement (CSA)",
		"POST-ISM Channel Availability Check",
		"PRE-ISM Ouf Of Channels (OOC)",
		"POST-ISM Out Of Channels (OOC)"
	};

	ret += websWrite(wp, "var dfs_statusarray = [\"%s\", \"%d s\", \"%s\"];\n",
		(dfs_status->state >= WL_DFS_CACSTATES ? "Unknown" : dfs_cacstate_str[dfs_status->state]),
		dfs_status->duration/1000,
		(dfs_status->chanspec_cleared ? wf_chspec_ntoa(dfs_status->chanspec_cleared, chanspec_str) : "None"));
#endif

	if (noclients)
		return ret;

sta_list:

// Open client array
	switch(unit) {
	case 0:
		ret += websWrite(wp, "wificlients24 = [");
		break;
	case 1:
		ret += websWrite(wp, "wificlients5 = [");
		break;
	case 2:
		ret += websWrite(wp, "wificlients52 = [");
		break;
	}

#ifdef RTCONFIG_QTN
	if (unit && rpc_qtn_ready())
	{
		ret += ej_wl_status_5g_array(eid, wp, argc, argv);
		ret += websWrite(wp, "\"-1\"];");
		return ret;
	}
#endif



#ifdef RTCONFIG_WIRELESSREPEATER
	if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
		&& (nvram_get_int("wlc_band") == unit))
	{
		sprintf(name_vif, "wl%d.%d", unit, 1);
		name = name_vif;
	}
#endif

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	memset(auth, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)auth, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
		goto exit;

	/* Obtain mac + IP list */
	arplist = read_whole_file("/proc/net/arp");
	/* Obtain lease list - we still need the arp list for
	   cases where a device uses a static IP rather than DHCP */
	leaselist = read_whole_file("/var/lib/misc/dnsmasq.leases");

#ifdef RTCONFIG_IPV6
	/* Obtain IPv6 info */
	if (ipv6_enabled()) {
		get_ipv6_client_info();
		get_ipv6_client_list();
		ipv6list = read_whole_file(IPV6_CLIENT_LIST);
	}
#endif

	/* build authenticated sta list */
	for (i = 0; i < auth->count; i ++) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) continue;

		ret += websWrite(wp, "[\"%s\",", ether_etoa((void *)&auth->ea[i], ea));

		found = 0;
		if (arplist) {
			arplistptr = strdup(arplist);
			line = strtok(arplistptr, "\n");
			while (line) {
				if ( (sscanf(line,"%15s %*s %*s %17s",ipentry,macentry) == 2) &&
				     (!strcasecmp(macentry, ether_etoa((void *)&auth->ea[i], ea))) ) {
					found = 1;
					break;
				} else
					line  = strtok(NULL, "\n");
			}
			if (arplistptr)	free(arplistptr);

			if (found || !leaselist)
				ret += websWrite(wp, "\"%s\",", (found ? ipentry : "<unknown>"));
		} else {
			ret += websWrite(wp, "\"<unknown>\",");
		}

		// Retrieve hostname from dnsmasq leases
		if (leaselist) {
			leaselistptr = strdup(leaselist);
			line = strtok(leaselistptr, "\n");
			while (line) {
				if ( (sscanf(line,"%*s %17s %15s %32s %*s", macentry, ipentry, tmp) == 3) &&
				     (!strcasecmp(macentry, ether_etoa((void *)&auth->ea[i], ea))) ) {
					found += 2;
					break;
				} else
					line = strtok(NULL, "\n");
			}
			if (leaselistptr) free(leaselistptr);

			if ((found) && (str_escape_quotes(hostnameentry, tmp, sizeof(hostnameentry)) == 0 ))
				strlcpy(hostnameentry, tmp, sizeof(hostnameentry));

			switch (found) {
			case 0:	// Not in arplist nor in leaselist
				ret += websWrite(wp, "\"<unknown>\",\"<unknown>\",");
				break;
			case 1:	// Only in arplist (static IP)
				ret += websWrite(wp, "\"<unknown>\",");
				break;
			case 2:	// Only in leaselist (dynamic IP that has not communicated with router for a while)
				ret += websWrite(wp, "\"%s\", \"%s\",", ipentry, hostnameentry);
				break;
			case 3:	// In both arplist and leaselist (dynamic IP)
				ret += websWrite(wp, "\"%s\",", hostnameentry);
				break;
			}
		} else {
			ret += websWrite(wp, "\"<unknown>\",");
		}

#ifdef RTCONFIG_IPV6
// Retrieve IPv6
		if (ipv6list) {
			ipv6listptr = ipv6list;
			foundipv6 = 0;
			while ((ipv6listptr < ipv6list+strlen(ipv6list)-2) && (sscanf(ipv6listptr,"%*s %17s %40s", macentry, ipentry) == 2)) {
				if (strcasecmp(macentry, ether_etoa((void *)&auth->ea[i], ea)) == 0) {
					ret += websWrite(wp, "\"%s\",", ipentry);
					foundipv6 = 1;
					break;
				} else {
					ipv6listptr = strstr(ipv6listptr,"\n")+1;
				}
			}
		}
#endif

		if (foundipv6 == 0) {
			ret += websWrite(wp, "\"\",");
		}

// RSSI
		memcpy(&scb_val.ea, &auth->ea[i], ETHER_ADDR_LEN);
		if (wl_ioctl(name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
			ret += websWrite(wp, "\"?\",");
		else
			ret += websWrite(wp, "\"%d\",", scb_val.val);

		if (sta->flags & WL_STA_SCBSTATS)
		{
// Rate
			if ((int)sta->rx_rate > 0)
				sprintf(rxrate,"%d", sta->rx_rate / 1000);
			else
				strcpy(rxrate,"??");

			if ((int)sta->tx_rate > 0)
				sprintf(txrate,"%d", sta->tx_rate / 1000);
			else
				sprintf(txrate,"??");

			ret += websWrite(wp, "\"%s\", \"%s\",", rxrate, txrate);

// Connect time
			hr = sta->in / 3600;
			min = (sta->in % 3600) / 60;
			sec = sta->in - hr * 3600 - min * 60;
			ret += websWrite(wp, "\"%3d:%02d:%02d\",", hr, min, sec);

// NSS
#if (WL_STA_VER >= 5)
			val =  wl_sta_info_nss(sta, unit);
			if (val > 0)
				ret += websWrite(wp, "\"%d\",", val);
			else
#endif
			ret += websWrite(wp, "\"\",");

// PHY
			ret += websWrite(wp, "\"%s\",", phy_type_str[wl_sta_info_phy(sta, unit)]);

// Bandwidth
#if (WL_STA_VER >= 7)
			if (sta->flags & WL_STA_SCBSTATS)
				ret += websWrite(wp, "\"%s\",", wl_bw_str[wl_sta_info_bw(sta)]);
			else
				ret += websWrite(wp, "\"\",");
#endif

// Flags
#ifdef RTCONFIG_BCMARM
			ret += websWrite(wp, "\"%s%s%s",
				(sta->flags & WL_STA_PS) ? "P" : "_",
				((sta->ht_capabilities & WL_STA_CAP_SHORT_GI_20) || (sta->ht_capabilities & WL_STA_CAP_SHORT_GI_40)) ? "S" : "_",
				((sta->ht_capabilities & WL_STA_CAP_TX_STBC) || (sta->ht_capabilities & WL_STA_CAP_RX_STBC_MASK)) ? "T" : "_");
#ifdef RTCONFIG_MUMIMO
			ret += websWrite(wp, "%s",
				((sta->vht_flags & WL_STA_MU_BEAMFORMER) || (sta->vht_flags & WL_STA_MU_BEAMFORMEE)) ? "M" : "_");
#endif
#else
			ret += websWrite(wp, "\"%s",
				(sta->flags & WL_STA_PS) ? "P" : "_");
#endif
		}
		ret += websWrite(wp, "%s%s\"],",
			(sta->flags & WL_STA_ASSOC) ? "A" : "_",
			(sta->flags & WL_STA_AUTHO) ? "U" : "_");
	}

	for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
			&& (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;
#endif
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			sprintf(name_vif, "wl%d.%d", unit, i);
			memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;

				ret += websWrite(wp, "[\"%s\",", ether_etoa((void *)&auth->ea[ii], ea));

				found = 0;
				if (arplist) {
					arplistptr = strdup(arplist);
					line = strtok(arplistptr, "\n");
					while (line) {
						if ( (sscanf(line,"%15s %*s %*s %17s",ipentry,macentry) == 2) &&
						     (!strcasecmp(macentry, ether_etoa((void *)&auth->ea[ii], ea))) ) {
							found = 1;
							break;
						} else
							line  = strtok(NULL, "\n");
					}
					if (arplistptr) free(arplistptr);

					if (found || !leaselist)
						ret += websWrite(wp, "\"%s\",", (found ? ipentry : "<unknown>"));
				} else {
					ret += websWrite(wp, "\"<unknown>\",");
				}

				// Retrieve hostname from dnsmasq leases
				if (leaselist) {
					leaselistptr = strdup(leaselist);
					line = strtok(leaselistptr, "\n");
					while (line) {
						if ( (sscanf(line,"%*s %17s %15s %32s %*s", macentry, ipentry, tmp) == 3) &&
						     (!strcasecmp(macentry, ether_etoa((void *)&auth->ea[ii], ea))) ) {
							found += 2;
							break;
						} else
							line = strtok(NULL, "\n");
					}
					if (leaselistptr) free(leaselistptr);

					if ((found) && (str_escape_quotes(hostnameentry, tmp,sizeof(hostnameentry)) == 0 ))
						strlcpy(hostnameentry, tmp, sizeof(hostnameentry));

					switch (found) {
					case 0:	// Not in arplist nor in leaselist
						ret += websWrite(wp, "\"<not found>\",\"<not found>\",");
						break;
					case 1:	// Only in arplist (static IP)
						ret += websWrite(wp, "\"<not found>\",");
						break;
					case 2:	// Only in leaselist (dynamic IP that has not communicated with router for a while)
						ret += websWrite(wp, "\"%s\",\"%s\",", ipentry, hostnameentry);
						break;
					case 3:	// In both arplist and leaselist (dynamic IP)
						ret += websWrite(wp, "\"%s\",", hostnameentry);
						break;
					}
				} else {
					ret += websWrite(wp, "\"<unknown>\",");
				}

#ifdef RTCONFIG_IPV6
// Retrieve IPv6
				if (ipv6list) {
					ipv6listptr = ipv6list;
					foundipv6 = 0;
					while ((ipv6listptr < ipv6list+strlen(ipv6list)-2) && (sscanf(ipv6listptr,"%*s %17s %40s", macentry, ipentry) == 2)) {
						if (strcasecmp(macentry, ether_etoa((void *)&auth->ea[i], ea)) == 0) {
							ret += websWrite(wp, "\"%s\",", ipentry);
							foundipv6 = 1;
							break;
						} else {
							ipv6listptr = strstr(ipv6listptr,"\n")+1;
						}
					}
				}
#endif

				if (foundipv6 == 0) {
					ret += websWrite(wp, "\"\",");
				}
// RSSI
				memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
				if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
					ret += websWrite(wp, "\"?\",");
				else
					ret += websWrite(wp, "\"%d\",", scb_val.val);

				if (sta->flags & WL_STA_SCBSTATS)
				{
// Rate
					if ((int)sta->rx_rate > 0)
						sprintf(rxrate,"%d", sta->rx_rate / 1000);
					else
						strcpy(rxrate,"??");

					if ((int)sta->tx_rate > 0)
						sprintf(txrate,"%d", sta->tx_rate / 1000);
					else
						sprintf(txrate,"??");

					ret += websWrite(wp, "\"%s\",\"%s\",", rxrate, txrate);

// Connect time
					hr = sta->in / 3600;
					min = (sta->in % 3600) / 60;
					sec = sta->in - hr * 3600 - min * 60;
					ret += websWrite(wp, "\"%3d:%02d:%02d\",", hr, min, sec);

// NSS
#if (WL_STA_VER >= 5)
					val =  wl_sta_info_nss(sta, unit);
					if (val > 0)
						ret += websWrite(wp, "\"%d\",", val);
					else
#endif
						ret += websWrite(wp, "\"\",");

// PHY
					ret += websWrite(wp, "\"%s\",", phy_type_str[wl_sta_info_phy(sta, unit)]);

// Bandwidth
#if (WL_STA_VER >= 7)
					if (sta->flags & WL_STA_SCBSTATS)
						ret += websWrite(wp, "\"%s\",", wl_bw_str[wl_sta_info_bw(sta)]);
					else
						ret += websWrite(wp, "\"\",");
#endif


// Flags
#ifdef RTCONFIG_BCMARM
					ret += websWrite(wp, "\"%s%s%s",
						(sta->flags & WL_STA_PS) ? "P" : "_",
						((sta->ht_capabilities & WL_STA_CAP_SHORT_GI_20) || (sta->ht_capabilities & WL_STA_CAP_SHORT_GI_40)) ? "S" : "_",
						((sta->ht_capabilities & WL_STA_CAP_TX_STBC) || (sta->ht_capabilities & WL_STA_CAP_RX_STBC_MASK)) ? "T" : "_");
#ifdef RTCONFIG_MUMIMO
					ret += websWrite(wp, "%s",
						((sta->vht_flags & WL_STA_MU_BEAMFORMER) || (sta->vht_flags & WL_STA_MU_BEAMFORMEE)) ? "M" : "_");
#endif
#else
					ret += websWrite(wp, "\"%s",
						(sta->flags & WL_STA_PS) ? "P" : "_");
#endif
				}

// Auth/Ass (and Guest) flags
				ret += websWrite(wp, "%s%s%d\"],",
					(sta->flags & WL_STA_ASSOC) ? "A" : "_",
					(sta->flags & WL_STA_AUTHO) ? "U" : "_",
					i);
			}
		}
	}
	/* error/exit */
exit:
	ret += websWrite(wp, "\"-1\"];");
	if (auth) free(auth);
	if (arplist) free(arplist);
	if (leaselist) free(leaselist);
	if (ipv6list) free(ipv6list);

	return ret;
}
