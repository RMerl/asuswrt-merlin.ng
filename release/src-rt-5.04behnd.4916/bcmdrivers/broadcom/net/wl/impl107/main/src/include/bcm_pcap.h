/*
 * +--------------------------------------------------------------------------+
 * bcm_pcap.h
 *
 * External Interface for Packet Capture module, shared
 * between dongle firmware (producer) and host driver (consumer).
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id$
 *
 * +--------------------------------------------------------------------------+
 */
#ifndef __bcm_pcap_h__
#define __bcm_pcap_h__
#include <bcmwifi_rspec.h>
#include <802.11.h>

/* packet header and metadata */
#define MAX_MACHDR_LEN		58
#define D11RXHDR_LEN		40
#define D11PLCP_LEN		14
#define PHYRXSTS_LEN		152
#define PCAPF_MSDU_FIRST	0x1
#define PCAPF_MSDU_LAST		0x2
#define PCAP_PKT_BUFSZ		2048
#define PCAP_MAX_RX_RECV_BYTES	11454 /* Max RX size, multi-buffer */
#define PCAP_MAXLEN_BYTES	1800  /* Max size of maxlen config */
#define PCAP_POISON_VALUE	0xa9

#define PCAP_ANT_MAX	4	/* Max 4x4 WLAN interface */
#define PCAP_MAX_D11_HDR_LEN	(DOT11_A4_HDR_LEN + DOT11_QOS_LEN + \
					DOT11_HTC_LEN + DOT11_IV_MAX_LEN)
#define PCAP_RXCPY_MAX_D11HDR	(PCAP_MAX_D11_HDR_LEN + D11PLCP_LEN)
/* header packet element */
#define PCAP_RXHDR_HEADROOM	128
/* PCAP RX header to be stitched in each RX packet */
typedef struct pcap_rxcopy_hdr {
	uint32		pkt_id;
	uint16		pkt_flags;
	uint16		d11hdr_len;
	uint8		d11rxshdr[D11RXHDR_LEN];	/* ucode/hw status header */
	uint8		d11hdr[PCAP_RXCPY_MAX_D11HDR];	/* Mac header */
	int8		noise;				/* noise */
} pcap_rxcopy_hdr_t;

/* type */
#define PCAP_MU_VHT	1
#define PCAP_MU_HE	2
#define PCAP_MU_EHT	3

/* MU type */
#define	PCAP_HE_EHT_MMU	1	/* MU MIMO */
#define	PCAP_HE_EHT_OMU	2	/* non-MU MIMO (OFDMA) */

/* HE PPDU format */
#define PCAP_HE_SU	0
#define PCAP_HE_EXT_SU	1
#define PCAP_HE_MU	2
#define PCAP_HE_TRIG	3

#define PCAP_MAX_TX_MSDU 8

/* PCAP TX header to be stitched before the local TX packets */
typedef struct pcap_tx_hdr {
	uint16		txtype;			/* distinguish local from txcpl */
	uint16		seq;			/* counter */
	uint16		payload_offset;		/* byte offset from this struct to payload */
	uint16		reserved;
	chanspec_t	chanspec;
	uint16		datarate;		/* 500 kbps units */
	ratespec_t	rspec;			/* detailed rate info */
	uint32		tsf_hi;			/* MAC timestamp (hi 32bits) */
	uint32		tsf_lo;			/* MAC timestamp (lo 32 bits) */
	uint16		pkt_size[PCAP_MAX_TX_MSDU]; /* size of each msdu */
	uint8		pkt_cnt;		/* number of msdu's */
	int8		txpwr[PCAP_ANT_MAX];	/* power in dBm */
	uint8		txcore_mask;		/* bit map of TX cores used */
	uint8		retries;		/* Number of retries */
	uint8		rts_cnt;		/* Number of RTS transmitted */
	uint8		acked;			/* Ack received */
	uint8		bss_color;

	uint8		tx_type;		/* vht/he/eht pkt */
	uint8		tx_mu_type;		/* mu mimo/ofdma type */
	ratespec_t	mu_nss_mcs;
	uint8		mu_user_id;
	uint8		gid;			/* Group id */
} pcap_tx_hdr_t;

/* Extended PCAP TX header to be used for non-local TX packets */
typedef struct pcap_tx_hdr_ext {
	pcap_tx_hdr_t	txhdr;
	uint8		d11hdr_len;
	uint8		d11hdr[PCAP_MAX_D11_HDR_LEN];
} pcap_tx_hdr_ext_t;

#define PCAP_TXHDR_TYPE_TXCPL	0xcafe	/* stored in txtype for TXCPL */
#define PCAP_TXHDR_TYPE_SFRM	0x5a9e	/* stored in txtype for TXCPL subframes */
#define PCAP_TXHDR_TYPE_LOCAL	0x3501	/* stored in txtype for local packets */
#define PCAP_TXCPL_HDR_ALIGN	0x40	/* align pcap_tx_hdr on boundary in txcpl hdrm */

/* packet capture macros */
#define	WLC_PCAP_PROMISC_EN	0x1
#define	WLC_PCAP_RXCOPY_EN	0x2
#define	WLC_PCAP_TXCOPY_EN	0x4
#define WLC_PCAP_MODE_EN_MASK (WLC_PCAP_PROMISC_EN | WLC_PCAP_RXCOPY_EN | WLC_PCAP_TXCOPY_EN)

/* rx config */
#define	WLC_PCAP_RX_FIRST_MSDU	0x100
#define	WLC_PCAP_RX_FIRST_MPDU	0x200
#define	WLC_PCAP_RX_LAST_MPDU	0x400
#define	WLC_PCAP_RX_KEEP_BADFCS	0x800
#define	WLC_PCAP_RX_KEEP_BYTES	0x1000
/* tx config */
#define	WLC_PCAP_TX_FIRST_MSDU	0x10000
#define	WLC_PCAP_TX_FIRST_MPDU	0x20000
#define	WLC_PCAP_TX_LAST_MPDU	0x40000
#define	WLC_PCAP_TX_KEEP_BYTES	0x80000
/* eapol config */
#define	WLC_PCAP_EAPOL		0x100000

#define WLC_PCAP_CONFIG_DEFAULT (WLC_PCAP_MODE_EN_MASK | WLC_PCAP_RX_FIRST_MSDU |\
				WLC_PCAP_RX_FIRST_MPDU | WLC_PCAP_RX_KEEP_BYTES |\
				WLC_PCAP_TX_FIRST_MSDU | WLC_PCAP_TX_FIRST_MPDU |\
				WLC_PCAP_TX_KEEP_BYTES)

/* PCAP Config macros */
#define PCAP_CFG_ANY_MODE(cfg)		(((cfg) & WLC_PCAP_MODE_EN_MASK)   != 0)
#define PCAP_CFG_PROMISC(cfg)		(((cfg) & WLC_PCAP_PROMISC_EN)     != 0)
#define PCAP_CFG_RXCOPY(cfg)		(((cfg) & WLC_PCAP_RXCOPY_EN)      != 0)
#define PCAP_CFG_TXCOPY(cfg)		(((cfg) & WLC_PCAP_TXCOPY_EN)      != 0)
#define PCAP_CFG_RX_FIRST_MSDU(cfg)	(((cfg) & WLC_PCAP_RX_FIRST_MSDU)  != 0)
#define PCAP_CFG_RX_FIRST_MPDU(cfg)	(((cfg) & WLC_PCAP_RX_FIRST_MPDU)  != 0)
#define PCAP_CFG_RX_KEEP_BADFCS(cfg)	(((cfg) & WLC_PCAP_RX_KEEP_BADFCS) != 0)
#define PCAP_CFG_RX_KEEP_BYTES(cfg)	(((cfg) & WLC_PCAP_RX_KEEP_BYTES)  != 0)
#define PCAP_CFG_TX_FIRST_MSDU(cfg)	(((cfg) & WLC_PCAP_TX_FIRST_MSDU)  != 0)
#define PCAP_CFG_TX_FIRST_MPDU(cfg)	(((cfg) & WLC_PCAP_TX_FIRST_MPDU)  != 0)
#define PCAP_CFG_TX_KEEP_BADFCS(cfg)	(((cfg) & WLC_PCAP_TX_KEEP_BADFCS) != 0)
#define PCAP_CFG_TX_KEEP_BYTES(cfg)	(((cfg) & WLC_PCAP_TX_KEEP_BYTES)  != 0)
#define PCAP_CFG_EAPOL(cfg)		(((cfg) & WLC_PCAP_EAPOL)          != 0)

#define WLC_PCAP_MAXLEN_DEFAULT	100

/* Type and sub type frames to capture */
#define	PCAP_CAPT_MGMT_FRAMES	0x000000000000FFFF
#define	PCAP_CAPT_CTL_FRAMES	0x00000000FFFF0000
#define	PCAP_CAPT_DATA_FRAMES	0x0000FFFF00000000
#define	PCAP_CAPT_EXT_FRAMES	0xFFFF000000000000
#define PCAP_CAPT_ALL_FRAMES	(PCAP_CAPT_MGMT_FRAMES | PCAP_CAPT_CTL_FRAMES \
				| PCAP_CAPT_DATA_FRAMES | PCAP_CAPT_EXT_FRAMES)

/* dhd->pcap.pcap_filter holds type and subtype frames to be captured.
 * user can select particular type and subtype frame to be captured using wl cmd.
 * From 802.11 spec, type is 2-bits and subtype is 4-bits, total 64 bit combinations are possible.
 * (1 << (((type) << 4) | (subtype))) - Will provide filter combination selected by user.
 *
 */
#define PCAP_TY_SBTY_BMAP(type, subty)	(((uint64)1) << (((type) << 4) | (subtype)))

/* external protype */
bool wlc_pcap_pkt_8021x(void *p);
#endif /* __bcm_pcap_h__ */
