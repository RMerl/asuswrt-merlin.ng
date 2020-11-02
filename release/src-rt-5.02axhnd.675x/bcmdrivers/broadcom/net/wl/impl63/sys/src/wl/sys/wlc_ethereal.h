/*
 * Structures and defines for the prism-style rx header that Ethereal
 * understands.
 * Broadcom 802.11abg Networking Device Driver
 *  Derived from http://airsnort.shmoo.com/orinoco-09b-packet-1.diff
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
 * $Id: wlc_ethereal.h 523117 2014-12-26 18:32:49Z $
 */

#ifndef _WLC_ETHEREAL_H_
#define _WLC_ETHEREAL_H_

#ifndef ETH_P_80211_RAW
#define ETH_P_80211_RAW			(ETH_P_ECONET + 1)
#endif /* ETH_P_80211_RAW */

#ifndef ARPHRD_ETHER
#define ARPHRD_ETHER			1 /* ARP Header */
#endif /* ARPHRD_ETHER */

#ifndef ARPHRD_IEEE80211_PRISM
#define ARPHRD_IEEE80211_PRISM		802 /* ARP Hdr for Prism-style Rx header */
#endif /* ARPHRD_IEEE80211_PRISM */

#define DNAMELEN			16  /* Dev name length */

#define WL_MON_FRAME			0x0041	/* Monitor Frame */
#define WL_MON_FRAME_HOSTTIME		0x1041	/* Host time element */
#define WL_MON_FRAME_MACTIME		0x2041	/* Mac time element */
#define WL_MON_FRAME_CHANNEL		0x3041	/* Channel element */
#define WL_MON_FRAME_RSSI		0x4041	/* RSSI element */
#define WL_MON_FRAME_SQ			0x5041	/* SQ element */
#define WL_MON_FRAME_SIGNAL		0x6041	/* Signal element */
#define WL_MON_FRAME_NOISE		0x7041	/* Noise element */
#define WL_MON_FRAME_RATE		0x8041	/* Rate element */
#define WL_MON_FRAME_ISTX		0x9041	/* Is Tx frame */
#define WL_MON_FRAME_FRMLEN		0xA041	/* Frame length */

#define P80211ITEM_OK			0	/* Prism 802.11 Item OK */
#define P80211ITEM_NO_VALUE		1	/* Prism 802.11 No value */

typedef struct p80211item
{
	uint32		did;
	uint16		status;
	uint16		len;
	uint32		data;
} p80211item_t;

typedef struct p80211msg
{
	uint32	msgcode;
	uint32	msglen;
	uint8		devname[DNAMELEN];
	p80211item_t	hosttime;
	p80211item_t	mactime;
	p80211item_t	channel;
	p80211item_t	rssi;
	p80211item_t	sq;
	p80211item_t	signal;
	p80211item_t	noise;
	p80211item_t	rate;
	p80211item_t	istx;
	p80211item_t	frmlen;
} p80211msg_t;

#define WLANCAP_MAGIC_COOKIE_V1 0x80211001  /* Wlan Magic Cookie */

#define WLANCAP_PHY_UNKOWN		0	/* Wlan cap Unknown PHY */
#define WLANCAP_PHY_FHSS_97		1	/* Wlan cap FHSS 97 PHY */
#define WLANCAP_PHY_DSSS_97		2	/* Wlan cap DSSS 97 PHY */
#define WLANCAP_PHY_IR			3	/* Wlan cap IR PHY */
#define WLANCAP_PHY_DSSS_11B		4	/* Wlan cap DSSS 11.b PHY */
#define WLANCAP_PHY_PBCC_11B		5	/* Wlan cap PBCC 11.b PHY */
#define WLANCAP_PHY_OFDM_11G		6	/* Wlan cap OFDM 11.g PHY */
#define WLANCAP_PHY_PBCC_11G		7	/* Wlan cap PBCC 11.g PHY */
#define WLANCAP_PHY_OFDM_11A		8	/* Wlan cap OFDM 11.a PHY */
#define WLANCAP_PHY_OFDM_11N		9	/* Wlan cap OFDM 11.n PHY */

#define WLANCAP_ENCODING_UNKNOWN	0	/* Unknown encoding */
#define WLANCAP_ENCODING_CCK		1	/* CCK encoding */
#define WLANCAP_ENCODING_PBCC		2	/* PBCC encoding */
#define WLANCAP_ENCODING_OFDM		3	/* OFDM encoding */

#define WLANCAP_SSI_TYPE_NONE		0	/* No SSI */
#define WLANCAP_SSI_TYPE_NORM		1	/* Normal SSI */
#define WLANCAP_SSI_TYPE_DBM		2	/* dBM SSI */
#define WLANCAP_SSI_TYPE_RAW		3	/* RAW SSI */

#define WLANCAP_PREAMBLE_UNKNOWN	0	/* Unknown Preamble */
#define WLANCAP_PREAMBLE_SHORT		1	/* Short preamble */
#define WLANCAP_PREAMBLE_LONG		2	/* Long preamble */
#define WLANCAP_PREAMBLE_MIMO_MM	3	/* MIMO MM preamble */
#define WLANCAP_PREAMBLE_MIMO_GF	4	/* MIMO GF preamble */

/* wlan monitor mode
 * all values are in network order
*/
typedef struct wlan_header_v1 {
	uint32	version;
	uint32	length;
	uint32	mactime_h;
	uint32	mactime_l;
	uint32	hosttime_h;
	uint32	hosttime_l;
	uint32	phytype;
	uint32	channel;
	uint32	datarate;
	uint32	antenna;
	uint32	priority;
	uint32	ssi_type;
	int32	ssi_signal;
	int32	ssi_noise;
	uint32	preamble;
	uint32	encoding;
} wlan_header_v1_t;

#endif /* _WLC_ETHEREAL_H_ */
