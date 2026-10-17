/* $FreeBSD: src/sys/net80211/ieee80211_radiotap.h,v 1.11 2007/12/13 01:23:40 sam Exp $ */
/* $NetBSD: ieee80211_radiotap.h,v 1.16 2007/01/06 05:51:15 dyoung Exp $ */
/* FILE-CSTYLED */

/*
 * Copyright (c) 2003, 2004 David Young.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of David Young may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DAVID YOUNG ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL DAVID
 * YOUNG BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

/*
 * <<Broadcom-WL-IPTag/Open:>>
 */

#ifndef _NET80211_IEEE80211_RADIOTAP_H_
#define _NET80211_IEEE80211_RADIOTAP_H_

/* A generic radio capture format is desirable. It must be
 * rigidly defined (e.g., units for fields should be given),
 * and easily extensible.
 *
 * The following is an extensible radio capture format. It is
 * based on a bitmap indicating which fields are present.
 *
 * I am trying to describe precisely what the application programmer
 * should expect in the following, and for that reason I tell the
 * units and origin of each measurement (where it applies), or else I
 * use sufficiently weaselly language ("is a monotonically nondecreasing
 * function of...") that I cannot set false expectations for lawyerly
 * readers.
 */
#if defined(__KERNEL__) || defined(_KERNEL)
#ifndef DLT_IEEE802_11_RADIO
#define	DLT_IEEE802_11_RADIO	127	/* 802.11 plus WLAN header */
#endif
#endif /* defined(__KERNEL__) || defined(_KERNEL) */

#define	IEEE80211_RADIOTAP_HDRLEN	64

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/*
 * The radio capture header precedes the 802.11 header.
 *
 * Note well: all radiotap fields are little-endian.
 */
BWL_PRE_PACKED_STRUCT struct ieee80211_radiotap_header {
	uint8		it_version;	/* Version 0. Only increases
					 * for drastic changes,
					 * introduction of compatible
					 * new fields does not count.
					 */
	uint8		it_pad;
	uint16		it_len;		/* length of the whole
					 * header in bytes, including
					 * it_version, it_pad,
					 * it_len, and data fields.
					 */
	uint32		it_present;	/* A bitmap telling which
					 * fields are present. Set bit 31
					 * (0x80000000) to extend the
					 * bitmap by another 32 bits.
					 * Additional extensions are made
					 * by setting bit 31.
					 */
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

/*
 * Name                                 Data type       Units
 * ----                                 ---------       -----
 *
 * IEEE80211_RADIOTAP_TSFT              uint64_t        microseconds
 *
 *      Value in microseconds of the MAC's 64-bit 802.11 Time
 *      Synchronization Function timer when the first bit of the
 *      MPDU arrived at the MAC. For received frames, only.
 *
 * IEEE80211_RADIOTAP_CHANNEL           2 x uint16_t    MHz, bitmap
 *
 *      Tx/Rx frequency in MHz, followed by flags (see below).
 *
 * IEEE80211_RADIOTAP_FHSS              uint16_t        see below
 *
 *      For frequency-hopping radios, the hop set (first byte)
 *      and pattern (second byte).
 *
 * IEEE80211_RADIOTAP_RATE              uint8_t         500kb/s or index
 *
 *      Tx/Rx data rate.  If bit 0x80 is set then it represents an
 *	an MCS index and not an IEEE rate.
 *
 * IEEE80211_RADIOTAP_DBM_ANTSIGNAL     int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      RF signal power at the antenna, decibel difference from
 *      one milliwatt.
 *
 * IEEE80211_RADIOTAP_DBM_ANTNOISE      int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      RF noise power at the antenna, decibel difference from one
 *      milliwatt.
 *
 * IEEE80211_RADIOTAP_DB_ANTSIGNAL      uint8_t         decibel (dB)
 *
 *      RF signal power at the antenna, decibel difference from an
 *      arbitrary, fixed reference.
 *
 * IEEE80211_RADIOTAP_DB_ANTNOISE       uint8_t         decibel (dB)
 *
 *      RF noise power at the antenna, decibel difference from an
 *      arbitrary, fixed reference point.
 *
 * IEEE80211_RADIOTAP_TXFLAGS           uint16_t        txflags
 *      Properties of Transmitted frames
 *
 * IEEE80211_RADIOTAP_RETRIES           uint8_t         retries
 *      Number of retries
 *
 * IEEE80211_RADIOTAP_LOCK_QUALITY      uint16_t        unitless
 *
 *      Quality of Barker code lock. Unitless. Monotonically
 *      nondecreasing with "better" lock strength. Called "Signal
 *      Quality" in datasheets.  (Is there a standard way to measure
 *      this?)
 *
 * IEEE80211_RADIOTAP_TX_ATTENUATION    uint16_t        unitless
 *
 *      Transmit power expressed as unitless distance from max
 *      power set at factory calibration.  0 is max power.
 *      Monotonically nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DB_TX_ATTENUATION uint16_t        decibels (dB)
 *
 *      Transmit power expressed as decibel distance from max power
 *      set at factory calibration.  0 is max power.  Monotonically
 *      nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DBM_TX_POWER      int8_t          decibels from
 *                                                      one milliwatt (dBm)
 *
 *      Transmit power expressed as dBm (decibels from a 1 milliwatt
 *      reference). This is the absolute power level measured at
 *      the antenna port.
 *
 * IEEE80211_RADIOTAP_FLAGS             uint8_t         bitmap
 *
 *      Properties of transmitted and received frames. See flags
 *      defined below.
 *
 * IEEE80211_RADIOTAP_ANTENNA           uint8_t         antenna index
 *
 *      Unitless indication of the Rx/Tx antenna for this packet.
 *      The first antenna is antenna 0.
 *
 * IEEE80211_RADIOTAP_XCHANNEL          uint32_t        bitmap
 *                                      uint16_t        MHz
 *                                      uint8_t         channel number
 *                                      int8_t          .5 dBm
 *
 *      Extended channel specification: flags (see below) followed by
 *      frequency in MHz, the corresponding IEEE channel number, and
 *      finally the maximum regulatory transmit power cap in .5 dBm
 *      units.  This property supersedes IEEE80211_RADIOTAP_CHANNEL
 *      and only one of the two should be present.
 *
 * IEEE80211_RADIOTAP_MCS       u8, u8, u8              unitless
 *
 *     Contains a bitmap of known fields/flags, the flags, and
 *     the MCS index.
 *
 */
enum ieee80211_radiotap_type {
	IEEE80211_RADIOTAP_TSFT = 0,
	IEEE80211_RADIOTAP_FLAGS = 1,
	IEEE80211_RADIOTAP_RATE = 2,
	IEEE80211_RADIOTAP_CHANNEL = 3,
	IEEE80211_RADIOTAP_FHSS = 4,
	IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
	IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
	IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
	IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
	IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
	IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
	IEEE80211_RADIOTAP_ANTENNA = 11,
	IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
	IEEE80211_RADIOTAP_DB_ANTNOISE = 13,
	/* NB: gap for netbsd definitions */
	IEEE80211_RADIOTAP_TXFLAGS = 15,
	IEEE80211_RADIOTAP_RETRIES = 17,
	IEEE80211_RADIOTAP_XCHANNEL = 18,
	IEEE80211_RADIOTAP_MCS = 19,
	IEEE80211_RADIOTAP_AMPDU = 20,
	IEEE80211_RADIOTAP_VHT = 21,
	IEEE80211_RADIOTAP_TIMESTAMP = 22,
	IEEE80211_RADIOTAP_HE = 23,
	IEEE80211_RADIOTAP_TLVS = 28,
	IEEE80211_RADIOTAP_RADIOTAP_NAMESPACE = 29,
	IEEE80211_RADIOTAP_VENDOR_NAMESPACE = 30,
	IEEE80211_RADIOTAP_EXT = 31,
	IEEE80211_RADIOTAP_TLV_S1G = 32,
	IEEE80211_RADIOTAP_TLV_U_SIG = 33,
	IEEE80211_RADIOTAP_TLV_EHT = 34
	};

/* TLV lengths */
#define IEEE80211_RADIOTAP_EHT_TLV_LEN	0x2C
#define IEEE80211_RADIOTAP_USIG_TLV_LEN	0xC

#ifndef _KERNEL
/* channel attributes */
#define	IEEE80211_CHAN_TURBO	0x00000010 /* Turbo channel */
#define	IEEE80211_CHAN_CCK	0x00000020 /* CCK channel */
#define	IEEE80211_CHAN_OFDM	0x00000040 /* OFDM channel */
#define	IEEE80211_CHAN_2GHZ	0x00000080 /* 2 GHz spectrum channel. */
#define	IEEE80211_CHAN_5GHZ	0x00000100 /* 5 GHz spectrum channel */
#define	IEEE80211_CHAN_PASSIVE	0x00000200 /* Only passive scan allowed */
#define	IEEE80211_CHAN_DYN	0x00000400 /* Dynamic CCK-OFDM channel */
#define	IEEE80211_CHAN_GFSK	0x00000800 /* GFSK channel (FHSS PHY) */
#define	IEEE80211_CHAN_GSM	0x00001000 /* 900 MHz spectrum channel */
#define	IEEE80211_CHAN_STURBO	0x00002000 /* 11a static turbo channel only */
#define	IEEE80211_CHAN_HALF	0x00004000 /* Half rate channel */
#define	IEEE80211_CHAN_QUARTER	0x00008000 /* Quarter rate channel */
#define	IEEE80211_CHAN_HT20	0x00010000 /* HT 20 channel */
#define	IEEE80211_CHAN_HT40U	0x00020000 /* HT 40 channel w/ ext above */
#define	IEEE80211_CHAN_HT40D	0x00040000 /* HT 40 channel w/ ext below */
#endif /* !_KERNEL */

/* For IEEE80211_RADIOTAP_FLAGS */
#define	IEEE80211_RADIOTAP_F_CFP	0x01	/* sent/received
						 * during CFP
						 */
#define	IEEE80211_RADIOTAP_F_SHORTPRE	0x02	/* sent/received
						 * with short
						 * preamble
						 */
#define	IEEE80211_RADIOTAP_F_WEP	0x04	/* sent/received
						 * with WEP encryption
						 */
#define	IEEE80211_RADIOTAP_F_FRAG	0x08	/* sent/received
						 * with fragmentation
						 */
#define	IEEE80211_RADIOTAP_F_FCS	0x10	/* frame includes FCS */
#define	IEEE80211_RADIOTAP_F_DATAPAD	0x20	/* frame has padding between
						 * 802.11 header and payload
						 * (to 32-bit boundary)
						 */
#define	IEEE80211_RADIOTAP_F_BADFCS	0x40	/* does not pass FCS check */

/* For IEEE80211_RADIOTAP_MCS */
#define IEEE80211_RADIOTAP_MCS_HAVE_BW          0x01
#define IEEE80211_RADIOTAP_MCS_HAVE_MCS         0x02
#define IEEE80211_RADIOTAP_MCS_HAVE_GI          0x04
#define IEEE80211_RADIOTAP_MCS_HAVE_FMT         0x08
#define IEEE80211_RADIOTAP_MCS_HAVE_FEC         0x10

#define IEEE80211_RADIOTAP_MCS_BW_MASK          0x03
#define IEEE80211_RADIOTAP_MCS_BW_20		0
#define IEEE80211_RADIOTAP_MCS_BW_40		1
#define IEEE80211_RADIOTAP_MCS_BW_20L		2
#define IEEE80211_RADIOTAP_MCS_BW_20U		3
#define IEEE80211_RADIOTAP_MCS_SGI              0x04
#define IEEE80211_RADIOTAP_MCS_FMT_GF           0x08
#define IEEE80211_RADIOTAP_MCS_FEC_LDPC         0x10

#define IEEE80211_RADIOTAP_MCS_BW_80		0x20
#define IEEE80211_RADIOTAP_MCS_BW_20LL		0x40
#define IEEE80211_RADIOTAP_MCS_BW_20LU		0x60
#define IEEE80211_RADIOTAP_MCS_BW_20UL		0x80
#define IEEE80211_RADIOTAP_MCS_BW_20UU		0xa0
#define IEEE80211_RADIOTAP_MCS_BW_40L		0xc0
#define IEEE80211_RADIOTAP_MCS_BW_40U		0xe0

/* For IEEE80211_RADIOTAP_VHT */
#define IEEE80211_RADIOTAP_VHT_HAVE_STBC	0x0001
#define IEEE80211_RADIOTAP_VHT_HAVE_TXOP_PS	0x0002
#define IEEE80211_RADIOTAP_VHT_HAVE_GI		0x0004
#define IEEE80211_RADIOTAP_VHT_HAVE_SGI_NSYM_DA	0x0008
#define IEEE80211_RADIOTAP_VHT_HAVE_LDPC_EXTRA	0x0010
#define IEEE80211_RADIOTAP_VHT_HAVE_BF		0x0020
#define IEEE80211_RADIOTAP_VHT_HAVE_BW		0x0040
#define IEEE80211_RADIOTAP_VHT_HAVE_GID		0x0080
#define IEEE80211_RADIOTAP_VHT_HAVE_PAID	0x0100

#define IEEE80211_RADIOTAP_VHT_STBC		0x01
#define IEEE80211_RADIOTAP_VHT_TXOP_PS		0x02
#define IEEE80211_RADIOTAP_VHT_SGI		0x04
#define IEEE80211_RADIOTAP_VHT_SGI_NSYM_DA	0x08
#define IEEE80211_RADIOTAP_VHT_LDPC_EXTRA	0x10
#define IEEE80211_RADIOTAP_VHT_BF		0x20

#define IEEE80211_RADIOTAP_VHT_NSS		0x0f
#define IEEE80211_RADIOTAP_VHT_MCS		0xf0
#define IEEE80211_RADIOTAP_VHT_MCS_SHIFT	4

#define IEEE80211_RADIOTAP_VHT_CODING_LDPC	0x01

#define IEEE80211_RADIOTAP_VHT_BW_20		IEEE80211_RADIOTAP_MCS_BW_20
#define IEEE80211_RADIOTAP_VHT_BW_40		IEEE80211_RADIOTAP_MCS_BW_40
#define IEEE80211_RADIOTAP_VHT_BW_20L		IEEE80211_RADIOTAP_MCS_BW_20L
#define IEEE80211_RADIOTAP_VHT_BW_20U		IEEE80211_RADIOTAP_MCS_BW_20U
#define IEEE80211_RADIOTAP_VHT_BW_80		4
#define IEEE80211_RADIOTAP_VHT_BW_40L		5
#define IEEE80211_RADIOTAP_VHT_BW_40U		6
#define IEEE80211_RADIOTAP_VHT_BW_20LL		7
#define IEEE80211_RADIOTAP_VHT_BW_20LU		8
#define IEEE80211_RADIOTAP_VHT_BW_20UL		9
#define IEEE80211_RADIOTAP_VHT_BW_20UU		10
#define IEEE80211_RADIOTAP_VHT_BW_160		11
#define IEEE80211_RADIOTAP_VHT_BW_80L		12
#define IEEE80211_RADIOTAP_VHT_BW_80U		13
#define IEEE80211_RADIOTAP_VHT_BW_40LL		14
#define IEEE80211_RADIOTAP_VHT_BW_40LU		15
#define IEEE80211_RADIOTAP_VHT_BW_40UL		16
#define IEEE80211_RADIOTAP_VHT_BW_40UU		17
#define IEEE80211_RADIOTAP_VHT_BW_20LLL		18
#define IEEE80211_RADIOTAP_VHT_BW_20LLU		19
#define IEEE80211_RADIOTAP_VHT_BW_20LUL		20
#define IEEE80211_RADIOTAP_VHT_BW_20LUU		21
#define IEEE80211_RADIOTAP_VHT_BW_20ULL		22
#define IEEE80211_RADIOTAP_VHT_BW_20ULU		23
#define IEEE80211_RADIOTAP_VHT_BW_20UUL		24
#define IEEE80211_RADIOTAP_VHT_BW_20UUU		25

/* For IEEE80211_RADIOTAP_HE */
/* IEEE80211_RADIOTAP_HE data[1] format */
#define IEEE80211_RADIOTAP_HE_PPDU_FMT_MASK	0x0003
#define IEEE80211_RADIOTAP_HE_PPDU_FMT_SHIFT	0
#define IEEE80211_RADIOTAP_HE_HAVE_BSS_COLOR	0x0004
#define IEEE80211_RADIOTAP_HE_HAVE_BEAM_CHANGE	0x0008
#define IEEE80211_RADIOTAP_HE_HAVE_UL_DL	0x0010
#define IEEE80211_RADIOTAP_HE_HAVE_MCS		0x0020
#define IEEE80211_RADIOTAP_HE_HAVE_DCM		0x0040
#define IEEE80211_RADIOTAP_HE_HAVE_CODING	0x0080
#define IEEE80211_RADIOTAP_HE_HAVE_LDPC_EXTSYM	0x0100
#define IEEE80211_RADIOTAP_HE_HAVE_STBC		0x0200
#define IEEE80211_RADIOTAP_HE_HAVE_SPTL_REUSE	0x0400
#define IEEE80211_RADIOTAP_HE_HAVE_SPTL_REUSE2	0x0800
#define IEEE80211_RADIOTAP_HE_HAVE_SPTL_REUSE3	0x1000
#define IEEE80211_RADIOTAP_HE_HAVE_SPTL_REUSE4	0x2000
#define IEEE80211_RADIOTAP_HE_HAVE_BW		0x4000
#define IEEE80211_RADIOTAP_HE_HAVE_DOPPLER	0x8000

/* IEEE80211_RADIOTAP_HE data[2] format */
#define IEEE80211_RADIOTAP_HE_HAVE_PRI_SEC_80M      0x0001
#define IEEE80211_RADIOTAP_HE_HAVE_GI               0x0002
#define IEEE80211_RADIOTAP_HE_HAVE_LTF              0x0004
#define IEEE80211_RADIOTAP_HE_HAVE_FEC              0x0008
#define IEEE80211_RADIOTAP_HE_HAVE_TXBF             0x0010
#define IEEE80211_RADIOTAP_HE_HAVE_PED              0x0020
#define IEEE80211_RADIOTAP_HE_HAVE_TXOP             0x0040
#define IEEE80211_RADIOTAP_HE_HAVE_MIDAMBLE         0x0080
#define IEEE80211_RADIOTAP_HE_RU_ALLOC_OFFSET       0x3F00
#define IEEE80211_RADIOTAP_HE_HAVE_RU_ALLOC_OFFSET  0x4000

/* IEEE80211_RADIOTAP_HE data[3] format */
#define IEEE80211_RADIOTAP_HE_BSS_COLOR_MASK	0x3F
#define IEEE80211_RADIOTAP_HE_BSS_COLOR_SHIFT	0
#define IEEE80211_RADIOTAP_HE_BEAM_CHANGE_SHIFT	6
#define IEEE80211_RADIOTAP_HE_UL_DL_SHIFT	7
#define IEEE80211_RADIOTAP_HE_MCS_MASK		0xF00
#define IEEE80211_RADIOTAP_HE_MCS_SHIFT		8
#define IEEE80211_RADIOTAP_HE_DCM_SHIFT		12
#define IEEE80211_RADIOTAP_HE_CODING_SHIFT	13
#define IEEE80211_RADIOTAP_HE_LDPC_EXTSYM_SHIFT	14
#define IEEE80211_RADIOTAP_HE_STBC_SHIFT	15

/* IEEE80211_RADIOTAP_HE data[4] format */
#define IEEE80211_RADIOTAP_HE_SPTL_REUSE_SHIFT      0
#define IEEE80211_RADIOTAP_HE_SPTL_REUSE2_SHIFT     4
#define IEEE80211_RADIOTAP_HE_SPTL_REUSE3_SHIFT     8
#define IEEE80211_RADIOTAP_HE_SPTL_REUSE4_SHIFT     12
#define IEEE80211_RADIOTAP_HE_STA_ID_SHIFT          4
#define IEEE80211_RADIOTAP_HE_STA_ID_MASK           0x07FF

/* IEEE80211_RADIOTAP_HE data[5] format */
#define IEEE80211_RADIOTAP_HE_GI_SHIFT		4
#define IEEE80211_RADIOTAP_HE_LTF_SHIFT		6
#define IEEE80211_RADIOTAP_HE_NUM_LTF_SHIFT	8
#define IEEE80211_RADIOTAP_HE_FEC_SHIFT		12
#define IEEE80211_RADIOTAP_HE_TXBF_SHIFT	14
#define IEEE80211_RADIOTAP_HE_PED_SHIFT		15

/* IEEE80211_RADIOTAP_HE data[6] format */
#define IEEE80211_RADIOTAP_HE_NSTS_MASK		0x000F
#define IEEE80211_RADIOTAP_HE_DOPPLER_SHIFT	5
#define IEEE80211_RADIOTAP_HE_TXOP_SHIFT	8
#define IEEE80211_RADIOTAP_HE_MIDAMBLE_SHIFT	15

#define IEEE80211_RADIOTAP_HE_SU		0
#define IEEE80211_RADIOTAP_HE_EXT_SU		1
#define IEEE80211_RADIOTAP_HE_MU		2
#define IEEE80211_RADIOTAP_HE_TRIG		3
#define IEEE80211_RADIOTAP_HE_LTF_SIZE_UNKNOWN	0
#define IEEE80211_RADIOTAP_HE_LTF_SIZE_1x	1
#define IEEE80211_RADIOTAP_HE_LTF_SIZE_2x	2
#define IEEE80211_RADIOTAP_HE_LTF_SIZE_4x	3
#define IEEE80211_RADIOTAP_HE_LTF_1x		0
#define IEEE80211_RADIOTAP_HE_LTF_2x		1
#define IEEE80211_RADIOTAP_HE_LTF_4x		2
#define IEEE80211_RADIOTAP_HE_LTF_6x		3
#define IEEE80211_RADIOTAP_HE_LTF_8x		4
#define IEEE80211_RADIOTAP_HE_GI_0_8us		0
#define IEEE80211_RADIOTAP_HE_GI_1_6us		1
#define IEEE80211_RADIOTAP_HE_GI_3_2us		2

/* For IEEE80211_RADIOTAP_TXFLAGS */
#define IEEE80211_RADIOTAP_TXF_FAIL	0x0001	/* TX failed due to excessive retries */
#define IEEE80211_RADIOTAP_TXF_CTS	0x0002	/* TX used CTS-to-self protection */
#define IEEE80211_RADIOTAP_TXF_RTSCTS	0x0004	/* TX used RTS/CTS */
#define IEEE80211_RADIOTAP_TXF_NOACK	0x0008	/* For injected TX: don't expect ACK */
#define IEEE80211_RADIOTAP_TXF_SEQOVR	0x0010	/* For injected TX: use pre-configured seq */

/* For IEEE80211_RADIOTAP_AMPDU_STATUS */
#define IEEE80211_RADIOTAP_AMPDU_REPORT_ZEROLEN		0x0001
#define IEEE80211_RADIOTAP_AMPDU_IS_ZEROLEN		0x0002
#define IEEE80211_RADIOTAP_AMPDU_LAST_KNOWN		0x0004
#define IEEE80211_RADIOTAP_AMPDU_IS_LAST		0x0008
#define IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_ERR		0x0010
#define IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_KNOWN	0x0020
#define IEEE80211_RADIOTAP_AMPDU_EOF			0x0040
#define IEEE80211_RADIOTAP_AMPDU_EOF_KNOWN		0x0080

/* For IEEE80211_RADIOTAP_EHT */
/* IEEE80211_RADIOTAP_EHT known fields */
#define IEEE80211_RADIOTAP_EHT_HAVE_SPATIAL_REUSE		0x00000002
#define IEEE80211_RADIOTAP_EHT_HAVE_GI				0x00000004
#define IEEE80211_RADIOTAP_EHT_HAVE_NUMBER_LTF_SYM		0x00000010
#define IEEE80211_RADIOTAP_EHT_HAVE_LDPC_EXTRA_SYM_SEG		0x00000020
#define IEEE80211_RADIOTAP_EHT_HAVE_PRE_FEC_PADDING_FACTOR	0x00000040
#define IEEE80211_RADIOTAP_EHT_HAVE_PE_DISAMBIGUITY		0x00000080
#define IEEE80211_RADIOTAP_EHT_HAVE_DISREGARD_OFDMA		0x00000100
#define IEEE80211_RADIOTAP_EHT_HAVE_DISREGARD_SOUNDING		0x00000200
#define IEEE80211_RADIOTAP_EHT_HAVE_CRC1			0x00002000
#define IEEE80211_RADIOTAP_EHT_HAVE_TAIL1			0x00004000
#define IEEE80211_RADIOTAP_EHT_HAVE_CRC2			0x00008000
#define IEEE80211_RADIOTAP_EHT_HAVE_TAIL2			0x00010000
#define IEEE80211_RADIOTAP_EHT_HAVE_NSS_SOUNDING		0x00020000
#define IEEE80211_RADIOTAP_EHT_HAVE_BF_SOUNDING			0x00040000
#define IEEE80211_RADIOTAP_EHT_HAVE_NUMBER_NON_OFDMA_USERS	0x00080000
#define IEEE80211_RADIOTAP_EHT_HAVE_USER_ENC_BLOCK_CRC		0x00100000
#define IEEE80211_RADIOTAP_EHT_HAVE_USER_ENC_BLOCK_TAIL		0x00200000
#define IEEE80211_RADIOTAP_EHT_HAVE_RU_MRU_SIZE			0x00400000
#define IEEE80211_RADIOTAP_EHT_HAVE_RU_MRU_INDEX		0x00800000
#define IEEE80211_RADIOTAP_EHT_HAVE_TB_RU_ALLOC			0x01000000
#define IEEE80211_RADIOTAP_EHT_HAVE_PRIM_80MHZ_CH_POS		0x02000000

/* IEEE80211_RADIOTAP_EHT data[0] format */
#define IEEE80211_RADIOTAP_EHT_SPATIAL_REUSE_MASK		0x00000078
#define IEEE80211_RADIOTAP_EHT_SPATIAL_REUSE_SHIFT		3
#define IEEE80211_RADIOTAP_EHT_GI_MASK				0x00000180
#define IEEE80211_RADIOTAP_EHT_GI_SHIFT				7
#define IEEE80211_RADIOTAP_EHT_GI_0_8				0
#define IEEE80211_RADIOTAP_EHT_GI_1_6				1
#define IEEE80211_RADIOTAP_EHT_GI_3_2				2
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_MASK			0x00000600
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_SHIFT			9
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_UNKNOWN		0
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_1X			1
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_2X			2
#define IEEE80211_RADIOTAP_EHT_LTF_SYM_SZ_4X			3
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_MASK			0x00003800
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_SHIFT		11
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_1X			0
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_2X			1
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_4X			2
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_6X			3
#define IEEE80211_RADIOTAP_EHT_NUM_LTF_SYM_8X			4
#define IEEE80211_RADIOTAP_EHT_LDPC_EXTRA_SYM_MASK		0x00004000
#define IEEE80211_RADIOTAP_EHT_LDPC_EXTRA_SYM_SHIFT		14
#define IEEE80211_RADIOTAP_EHT_PREFEC_PAD_FACTOR_MASK		0x00018000
#define IEEE80211_RADIOTAP_EHT_PREFEC_PAD_FACTOR_SHIFT		15
#define IEEE80211_RADIOTAP_EHT_PE_DISAMBIGUITY_MASK		0x00020000
#define IEEE80211_RADIOTAP_EHT_PE_DISAMBIQUITY_SHIFT		17
#define IEEE80211_RADIOTAP_EHT_DISREGARD_SOUNDING_MASK		0x000C0000
#define IEEE80211_RADIOTAP_EHT_DISREGARD_SOUNDING_SHIFT		18
#define IEEE80211_RADIOTAP_EHT_DISREGARD_NON_SOUNDING_MASK	0x003C0000
#define IEEE80211_RADIOTAP_EHT_DISREGARD_NON_SOUNDING_SHIFT	18
#define IEEE80211_RADIOTAP_EHT_CRC1_MASK			0x03C00000
#define IEEE80211_RADIOTAP_EHT_CRC1_SHIFT			22
#define IEEE80211_RADIOTAP_EHT_TAIL1_MASK			0xFC000000
#define IEEE80211_RADIOTAP_EHT_TAIL1_SHIFT			26

/* IEEE80211_RADIOTAP_EHT data[1] format */
#define IEEE80211_RADIOTAP_EHT_RU_MRU_SIZE_MASK		0x0000001F
#define IEEE80211_RADIOTAP_EHT_RU_MRU_SIZE_SHIFT	0
#define IEEE80211_RADIOTAP_EHT_RU_MRU_INDEX_MASK	0x00001FE0
#define IEEE80211_RADIOTAP_EHT_RU_MRU_INDEX_SHIFT	5
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC1_MASK		0x003FE000
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC1_SHIFT		13
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC1_KNOWN		0x00400000
#define IEEE80211_RADIOTAP_EHT_PRIM_80MHZ_CH_POS_MASK	0xC0000000
#define IEEE80211_RADIOTAP_EHT_PRIM_80MHZ_CH_POS_SHIFT	30

/* IEEE80211_RADIOTAP_EHT data[2]-data[6] format */
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_MASK		0x000001FF
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_SHIFT		0
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_KNOWN		0x00000200
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_1_MASK	0x0007FC00
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_1_SHIFT	10
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_1_KNOWN	0x00080000
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_2_MASK	0x1FF00000
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_2_SHIFT	20
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_X_2_KNOWN	0x20000000

/* IEEE80211_RADIOTAP_EHT data[7] format */
#define IEEE80211_RADIOTAP_EHT_CRC2_MASK			0x0000000F
#define IEEE80211_RADIOTAP_EHT_CRC2_SHIFT			0
#define IEEE80211_RADIOTAP_EHT_TAIL2_MASK			0x000003F0
#define IEEE80211_RADIOTAP_EHT_TAIL2_SHIFT			4
#define IEEE80211_RADIOTAP_EHT_NSS_SOUNDING_MASK		0x0000F000
#define IEEE80211_RADIOTAP_EHT_NSS_SOUNDING_SHIFT		12
#define IEEE80211_RADIOTAP_EHT_BF_SOUNDING_MASK			0x00010000
#define IEEE80211_RADIOTAP_EHT_BF_SOUNDING_SHIFT		16
#define IEEE80211_RADIOTAP_EHT_NUM_NONOFDMA_MASK		0x000E0000
#define IEEE80211_RADIOTAP_EHT_NUM_NONOFDMA_SHIFT		17
#define IEEE80211_RADIOTAP_EHT_USER_ENC_BLOCK_CRC_MASK		0x00F00000
#define IEEE80211_RADIOTAP_EHT_USER_ENC_BLOCK_CRC_SHIFT		20
#define IEEE80211_RADIOTAP_EHT_USER_ENC_BLOCK_TAIL_MASK		0x3F000000
#define IEEE80211_RADIOTAP_EHT_USER_ENC_BLOCK_TAIL_SHIFT	24

/* IEEE80211_RADIOTAP_EHT data[8] format */
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_TB_PS160_MASK	0x00000001
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_TB_PS160_SHIFT	0
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_TB_B7_B0_MASK	0x000001FE
#define IEEE80211_RADIOTAP_EHT_RU_ALLOC_TB_B7_B0_SHIFT	1

/* For IEEE80211_RADIOTAP_EHT USER_INFO */
#define IEEE80211_RADIOTAP_EHT_HAVE_STAID			0x00000001
#define IEEE80211_RADIOTAP_EHT_HAVE_MCS				0x00000002
#define IEEE80211_RADIOTAP_EHT_HAVE_CODING			0x00000004
#define IEEE80211_RADIOTAP_EHT_HAVE_NSS				0x00000010
#define IEEE80211_RADIOTAP_EHT_HAVE_BF				0x00000020
#define IEEE80211_RADIOTAP_EHT_HAVE_SPATIAL_CFG			0x00000040
#define IEEE80211_RADIOTAP_EHT_USR_DATA_CAPTURED		0x00000080
#define IEEE80211_RADIOTAP_EHT_STAID_MASK			0x0007FF00
#define IEEE80211_RADIOTAP_EHT_STAID_SHIFT			8
#define IEEE80211_RADIOTAP_EHT_CODING_MASK			0x00080000
#define IEEE80211_RADIOTAP_EHT_CODING_SHIFT			19
#define IEEE80211_RADIOTAP_EHT_MCS_USR_INF_MASK			0x00F00000
#define IEEE80211_RADIOTAP_EHT_MCS_USR_INF_SHIFT		20
#define IEEE80211_RADIOTAP_EHT_NSS_USR_INF_MASK			0x0F000000
#define IEEE80211_RADIOTAP_EHT_NSS_USR_INF_SHIFT		24
#define IEEE80211_RADIOTAP_EHT_BF_USR_INF_MASK			0x20000000
#define IEEE80211_RADIOTAP_EHT_BF_USR_INF_SHIFT			29
#define IEEE80211_RADIOTAP_EHT_SPATIAL_CFG_USR_INF_MASK		0x3F000000
#define IEEE80211_RADIOTAP_EHT_SPATIAL_CFG_USR_INF_SHIFT	24

/* USIG - https://www.radiotap.org/fields/U-SIG.html */
#define WL_RXS_EHT_USIG_PHY_VER_KNOWN           0x01
#define WL_RXS_EHT_USIG_BW_KNOWN                0x02
#define WL_RXS_EHT_USIG_UL_DL_KNOWN             0x04
#define WL_RXS_EHT_USIG_BSS_COLOR_KNOWN         0x08
#define WL_RXS_EHT_USIG_TXOP_KNOWN              0x10

#endif /* !_NET80211_IEEE80211_RADIOTAP_H_ */
