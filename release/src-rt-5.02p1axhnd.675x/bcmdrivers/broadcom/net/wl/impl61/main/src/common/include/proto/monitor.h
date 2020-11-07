/*
 * Monitor Mode definitions.
 * This header file housing the define and function prototype use by
 * both the wl firmware and drivers.
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: monitor.h 512698 2016-02-07 13:12:15Z $
 */
#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <bcmwifi_channels.h>

#include <packed_section_start.h>
/* wl_monitor rx status per packet */
typedef struct BWL_PRE_PACKED_STRUCT wl_rxsts {
    uint    pkterror;       /* error flags per pkt */
    uint    phytype;        /* 802.11 A/B/G /N  */
    chanspec_t chanspec;        /* channel spec */
    uint16  datarate;       /* rate in 500kbps */
    uint8   mcs;            /* MCS for HT frame */
    uint8   htflags;        /* HT modulation flags */
    uint    antenna;        /* antenna pkts received on */
    uint    pktlength;      /* pkt length minus bcm phy hdr */
    uint32  mactime;        /* time stamp from mac, count per 1us */
    uint    sq;         /* signal quality */
    int32   signal;         /* in dBm */
    int32   noise;          /* in dBm */
    uint    preamble;       /* Unknown, short, long */
    uint    encoding;       /* Unknown, CCK, PBCC, OFDM, HT, VHT */
    uint    nfrmtype;       /* special 802.11 frames(AMPDU, AMSDU) or addition PPDU fromat */
    uint8   nss;            /* Number of spatial streams for VHT frame */
    uint8   coding;
    uint16  aid;            /* Partial AID for VHT frame */
    uint8   gid;            /* Group ID for VHT frame */
    uint8   bw;         /* Bandwidth for VHT frame */
    uint16  vhtflags;       /* VHT modulation flags */
    uint16	bw_nonht;       /* non-HT bw advertised in rts/cts */
    uint32  ampdu_counter;      /* AMPDU counter for sniffer */
    uint32  sig_a1;			/* HE  SIG-A1 field */
    uint32  sig_a2;			/* HE  SIG-A2 field */
} BWL_POST_PACKED_STRUCT wl_rxsts_t, wl_mon_rxsts_t;
#include <packed_section_end.h>

#define WLMONRXSTS_SIZE	sizeof(wl_rxsts_t)

/* phy type */
#define WL_RXS_PHY_N			0x00000004 /* N phy type */

/* encoding */
#define WL_RXS_ENCODING_UNKNOWN		0x00000000
#define WL_RXS_ENCODING_DSSS_CCK	0x00000001 /* DSSS/CCK encoding (1, 2, 5.5, 11) */
#define WL_RXS_ENCODING_OFDM		0x00000002 /* OFDM encoding */
#define WL_RXS_ENCODING_HT		0x00000003 /* HT encoding */
#define WL_RXS_ENCODING_VHT		0x00000004 /* VHT encoding */
#define WL_RXS_ENCODING_HE		0x00000005 /* HE encoding */

/* status per error RX pkt */
#define WL_RXS_CRC_ERROR		0x00000001 /* CRC Error in packet */
#define WL_RXS_RUNT_ERROR		0x00000002 /* Runt packet */
#define WL_RXS_ALIGN_ERROR		0x00000004 /* Misaligned packet */
#define WL_RXS_OVERSIZE_ERROR		0x00000008 /* packet bigger than RX_LENGTH (usually 1518) */
#define WL_RXS_WEP_ICV_ERROR		0x00000010 /* Integrity Check Value error */
#define WL_RXS_WEP_ENCRYPTED		0x00000020 /* Encrypted with WEP */
#define WL_RXS_PLCP_SHORT		0x00000040 /* Short PLCP error */
#define WL_RXS_DECRYPT_ERR		0x00000080 /* Decryption error */
#define WL_RXS_OTHER_ERR		0x80000000 /* Other errors */

/* preamble */
#define WL_RXS_UNUSED_STUB		0x0		/**< stub to match with wlc_ethereal.h */
#define WL_RXS_PREAMBLE_SHORT		0x00000001	/**< Short preamble */
#define WL_RXS_PREAMBLE_LONG		0x00000002	/**< Long preamble */
#define WL_RXS_PREAMBLE_HT_MM		0x00000003	/**< HT mixed mode preamble */
#define WL_RXS_PREAMBLE_HT_GF		0x00000004	/**< HT green field preamble */

/* htflags */
#define WL_RXS_HTF_BW_MASK		0x07
#define WL_RXS_HTF_40			0x01
#define WL_RXS_HTF_20L			0x02
#define WL_RXS_HTF_20U			0x04
#define WL_RXS_HTF_SGI			0x08
#define WL_RXS_HTF_STBC_MASK		0x30
#define WL_RXS_HTF_STBC_SHIFT		4
#define WL_RXS_HTF_LDPC			0x40

#ifdef WLTXMONITOR
/* reuse bw-bits in ht for vht */
#define WL_RXS_VHTF_BW_MASK		0x87
#define WL_RXS_VHTF_40			0x01
#define WL_RXS_VHTF_20L			WL_RXS_VHTF_20LL
#define WL_RXS_VHTF_20U			WL_RXS_VHTF_20LU
#define WL_RXS_VHTF_80			0x02
#define WL_RXS_VHTF_20LL		0x03
#define WL_RXS_VHTF_20LU		0x04
#define WL_RXS_VHTF_20UL		0x05
#define WL_RXS_VHTF_20UU		0x06
#define WL_RXS_VHTF_40L			0x07
#define WL_RXS_VHTF_40U			0x80
#endif /* WLTXMONITOR */

/* vhtflags */
#define WL_RXS_VHTF_STBC		0x01
#define WL_RXS_VHTF_TXOP_PS		0x02
#define WL_RXS_VHTF_SGI			0x04
#define WL_RXS_VHTF_SGI_NSYM_DA		0x08
#define WL_RXS_VHTF_LDPC_EXTRA		0x10
#define WL_RXS_VHTF_BF			0x20
#define WL_RXS_VHTF_DYN_BW_NONHT	0x40
#define WL_RXS_VHTF_CODING_LDCP		0x01

#define WL_RXS_VHT_BW_20		0
#define WL_RXS_VHT_BW_40		1
#define WL_RXS_VHT_BW_20L		2
#define WL_RXS_VHT_BW_20U		3
#define WL_RXS_VHT_BW_80		4
#define WL_RXS_VHT_BW_40L		5
#define WL_RXS_VHT_BW_40U		6
#define WL_RXS_VHT_BW_20LL		7
#define WL_RXS_VHT_BW_20LU		8
#define WL_RXS_VHT_BW_20UL		9
#define WL_RXS_VHT_BW_20UU		10
#define WL_RXS_VHT_BW_160		11
#define WL_RXS_VHT_BW_80L		12
#define WL_RXS_VHT_BW_80U		13
#define WL_RXS_VHT_BW_40LL		14
#define WL_RXS_VHT_BW_40LU		15
#define WL_RXS_VHT_BW_40UL		16
#define WL_RXS_VHT_BW_40UU		17
#define WL_RXS_VHT_BW_20LLL		18
#define WL_RXS_VHT_BW_20LLU		19
#define WL_RXS_VHT_BW_20LUL		20
#define WL_RXS_VHT_BW_20LUU		21
#define WL_RXS_VHT_BW_20ULL		22
#define WL_RXS_VHT_BW_20ULU		23
#define WL_RXS_VHT_BW_20UUL		24
#define WL_RXS_VHT_BW_20UUU		25

#define WL_RXS_NFRM_AMPDU_FIRST		0x00000001 /* first MPDU in A-MPDU */
#define WL_RXS_NFRM_AMPDU_SUB		0x00000002 /* subsequent MPDU(s) in A-MPDU */
#define WL_RXS_NFRM_AMSDU_FIRST		0x00000004 /* first MSDU in A-MSDU */
#define WL_RXS_NFRM_AMSDU_SUB		0x00000008 /* subsequent MSDU(s) in A-MSDU */
#define WL_RXS_NFRM_SMPDU			0x00000080 /* MRXS2 b7 : S-MPDU */
#define WL_RXS_NFRM_HE_ER_SU		0x00000800 /* MRXS2 b12:11 :
									HE PPDU additional format
									*/
#define WL_RXS_NFRM_HE_MU			0x00001000 /* MRXS2 b12:11 :
									HE PPDU additional format
									*/
#define WL_RXS_NFRM_HE_TB			0x00001800 /* MRXS2 b12:11 :
									HE PPDU additional format
									*/
#define WL_RXS_NFRM_HE_EXT_MASK		0x00001800 /* MRXS2 b12:11 mask */
#define WL_RXS_NFRM_HE_EXT_SHIFT	11         /* MRXS2 b12:11 shift */

#include <packed_section_start.h>
typedef struct BWL_PRE_PACKED_STRUCT wl_txsts {
	uint	pkterror;		/**< error flags per pkt */
	uint	phytype;		/**< 802.11 A/B/G /N */
	chanspec_t chanspec;		/**< channel spec */
	uint16	datarate;		/**< rate in 500kbps */
	uint8	mcs;			/**< MCS for HT frame */
	uint8	htflags;		/**< HT modulation flags */
	uint	antenna;		/**< antenna pkt transmitted on */
	uint	pktlength;		/**< pkt length minus bcm phy hdr */
	uint32	mactime;		/**< ? time stamp from mac, count per 1us */
	uint	preamble;		/**< Unknown, short, long */
	uint	encoding;		/**< Unknown, CCK, PBCC, OFDM, HT */
	uint	nfrmtype;		/**< special 802.11n frames(AMPDU, AMSDU) */
	uint	txflags;		/**< As defined in radiotap field 15 */
	uint	retries;		/**< Number of retries */
	struct wl_if *wlif;		/**< wl interface */
} BWL_POST_PACKED_STRUCT wl_txsts_t;
#include <packed_section_end.h>

#define WL_TXS_TXF_FAIL		0x01	/**< TX failed due to excessive retries */
#define WL_TXS_TXF_CTS		0x02	/**< TX used CTS-to-self protection */
#define WL_TXS_TXF_RTSCTS	0x04	/**< TX used RTS/CTS */

#endif /* _MONITOR_H_ */
