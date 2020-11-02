/*
 * Common [OS-independent] rate management
 * 802.11 Networking Adapter Device Driver.
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
 * $Id: wlc_rate_def.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>

/* for short hand notation (shorter lines in source code) */
#define QAM256	PHY_TXC1_MOD_SCHEME_QAM256
#define R34	PHY_TXC1_CODE_RATE_3_4
#define R56	PHY_TXC1_CODE_RATE_5_6

/**
 * XXX see Twiki [PhyTxControlandPlcp] for definition of 'tx_phy_ctl3', see bits[31:16] of paragraph
 * '11N PHY TX Controlbytes'
 */
#define MOD_SHIFT 3
#define NSS_SHIFT 6
#define NSS1	0	/* one spatial stream */
#define NSS2	1
#define NSS3	2
#define NSS4	3

/**
 * Rate info per rate: tells for *pre* 802.11n rates whether a given rate is OFDM or not and its
 * phy_rate value. Table index is a rate in [500Kbps] units, from 0 to 54Mbps.
 * Contents of a table element:
 *     d[7] : 1=OFDM rate, 0=DSSS/CCK rate
 *     d[3:0] if DSSS/CCK rate:
 *         index into the 'M_RATE_TABLE_B' table maintained by ucode in shm
 *     d[3:0] if OFDM rate: encode rate per 802.11a-1999 sec 17.3.4.1, with lsb transmitted first.
 *         index into the 'M_RATE_TABLE_A' table maintained by ucode in shm
 */
const uint8 rate_info[WLC_MAXRATE + 1] = {
	/*  0     1     2     3     4     5     6     7     8     9 */
/*   0 */ 0x00, 0x00, 0x0a, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  10 */ 0x00, 0x37, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0x00,
/*  20 */ 0x00, 0x00, 0x6e, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  30 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x00, 0x00, 0x00,
/*  40 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x00,
/*  50 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  60 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  70 */ 0x00, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  80 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  90 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
/* 100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c
};

/**
 * MCS indices are used for 802.11n rates. Given an MCS index, this table returns phy rates in
 * [Kbps] units for different bandwidths, sgi's, etc.
 */
const mcs_info_t mcs_table[] = {
	/* phy_rate_20, phy_rate_40, phy_rate_20_sgi, phy_rate_40_sgi, tx_phy_ctl3, 11a/g rate */
	/* MCS  0: SS 1, MOD: BPSK,  CR 1/2 */
	{6500,   13500,  CEIL(6500*10, 9),   CEIL(13500*10, 9),  0x00, WLC_RATE_6M},
	/* MCS  1: SS 1, MOD: QPSK,  CR 1/2 */
	{13000,  27000,  CEIL(13000*10, 9),  CEIL(27000*10, 9),  0x08, WLC_RATE_12M},
	/* MCS  2: SS 1, MOD: QPSK,  CR 3/4 */
	{19500,  40500,  CEIL(19500*10, 9),  CEIL(40500*10, 9),  0x0A, WLC_RATE_18M},
	/* MCS  3: SS 1, MOD: 16QAM, CR 1/2 */
	{26000,  54000,  CEIL(26000*10, 9),  CEIL(54000*10, 9),  0x10, WLC_RATE_24M},
	/* MCS  4: SS 1, MOD: 16QAM, CR 3/4 */
	{39000,  81000,  CEIL(39000*10, 9),  CEIL(81000*10, 9),  0x12, WLC_RATE_36M},
	/* MCS  5: SS 1, MOD: 64QAM, CR 2/3 */
	{52000,  108000, CEIL(52000*10, 9),  CEIL(108000*10, 9), 0x19, WLC_RATE_48M},
	/* MCS  6: SS 1, MOD: 64QAM, CR 3/4 */
	{58500,  121500, CEIL(58500*10, 9),  CEIL(121500*10, 9), 0x1A, WLC_RATE_54M},
	/* MCS  7: SS 1, MOD: 64QAM, CR 5/6 */
	{65000,  135000, CEIL(65000*10, 9),  CEIL(135000*10, 9), 0x1C, WLC_RATE_54M},
	/* MCS  8: SS 2, MOD: BPSK,  CR 1/2 */
	{13000,  27000,  CEIL(13000*10, 9),  CEIL(27000*10, 9),  0x40, WLC_RATE_6M},
	/* MCS  9: SS 2, MOD: QPSK,  CR 1/2 */
	{26000,  54000,  CEIL(26000*10, 9),  CEIL(54000*10, 9),  0x48, WLC_RATE_12M},
	/* MCS 10: SS 2, MOD: QPSK,  CR 3/4 */
	{39000,  81000,  CEIL(39000*10, 9),  CEIL(81000*10, 9),  0x4A, WLC_RATE_18M},
	/* MCS 11: SS 2, MOD: 16QAM, CR 1/2 */
	{52000,  108000, CEIL(52000*10, 9),  CEIL(108000*10, 9), 0x50, WLC_RATE_24M},
	/* MCS 12: SS 2, MOD: 16QAM, CR 3/4 */
	{78000,  162000, CEIL(78000*10, 9),  CEIL(162000*10, 9), 0x52, WLC_RATE_36M},
	/* MCS 13: SS 2, MOD: 64QAM, CR 2/3 */
	{104000, 216000, CEIL(104000*10, 9), CEIL(216000*10, 9), 0x59, WLC_RATE_48M},
	/* MCS 14: SS 2, MOD: 64QAM, CR 3/4 */
	{117000, 243000, CEIL(117000*10, 9), CEIL(243000*10, 9), 0x5A, WLC_RATE_54M},
	/* MCS 15: SS 2, MOD: 64QAM, CR 5/6 */
	{130000, 270000, CEIL(130000*10, 9), CEIL(270000*10, 9), 0x5C, WLC_RATE_54M},
	/* MCS 16: SS 3, MOD: BPSK,  CR 1/2 */
	{19500,  40500,  CEIL(19500*10, 9),  CEIL(40500*10, 9),  0x80, WLC_RATE_6M},
	/* MCS 17: SS 3, MOD: QPSK,  CR 1/2 */
	{39000,  81000,  CEIL(39000*10, 9),  CEIL(81000*10, 9),  0x88, WLC_RATE_12M},
	/* MCS 18: SS 3, MOD: QPSK,  CR 3/4 */
	{58500,  121500, CEIL(58500*10, 9),  CEIL(121500*10, 9), 0x8A, WLC_RATE_18M},
	/* MCS 19: SS 3, MOD: 16QAM, CR 1/2 */
	{78000,  162000, CEIL(78000*10, 9),  CEIL(162000*10, 9), 0x90, WLC_RATE_24M},
	/* MCS 20: SS 3, MOD: 16QAM, CR 3/4 */
	{117000, 243000, CEIL(117000*10, 9), CEIL(243000*10, 9), 0x92, WLC_RATE_36M},
	/* MCS 21: SS 3, MOD: 64QAM, CR 2/3 */
	{156000, 324000, CEIL(156000*10, 9), CEIL(324000*10, 9), 0x99, WLC_RATE_48M},
	/* MCS 22: SS 3, MOD: 64QAM, CR 3/4 */
	{175500, 364500, CEIL(175500*10, 9), CEIL(364500*10, 9), 0x9A, WLC_RATE_54M},
	/* MCS 23: SS 3, MOD: 64QAM, CR 5/6 */
	{195000, 405000, CEIL(195000*10, 9), CEIL(405000*10, 9), 0x9B, WLC_RATE_54M},
	/* MCS 24: SS 4, MOD: BPSK,  CR 1/2 */
	{26000,  54000,  CEIL(26000*10, 9),  CEIL(54000*10, 9), 0xC0, WLC_RATE_6M},
	/* MCS 25: SS 4, MOD: QPSK,  CR 1/2 */
	{52000,  108000, CEIL(52000*10, 9),  CEIL(108000*10, 9), 0xC8, WLC_RATE_12M},
	/* MCS 26: SS 4, MOD: QPSK,  CR 3/4 */
	{78000,  162000, CEIL(78000*10, 9),  CEIL(162000*10, 9), 0xCA, WLC_RATE_18M},
	/* MCS 27: SS 4, MOD: 16QAM, CR 1/2 */
	{104000, 216000, CEIL(104000*10, 9), CEIL(216000*10, 9), 0xD0, WLC_RATE_24M},
	/* MCS 28: SS 4, MOD: 16QAM, CR 3/4 */
	{156000, 324000, CEIL(156000*10, 9), CEIL(324000*10, 9), 0xD2, WLC_RATE_36M},
	/* MCS 29: SS 4, MOD: 64QAM, CR 2/3 */
	{208000, 432000, CEIL(208000*10, 9), CEIL(432000*10, 9), 0xD9, WLC_RATE_48M},
	/* MCS 30: SS 4, MOD: 64QAM, CR 3/4 */
	{234000, 486000, CEIL(234000*10, 9), CEIL(486000*10, 9), 0xDA, WLC_RATE_54M},
	/* MCS 31: SS 4, MOD: 64QAM, CR 5/6 */
	{260000, 540000, CEIL(260000*10, 9), CEIL(540000*10, 9), 0xDB, WLC_RATE_54M},
	/* MCS 32: SS 1, MOD: BPSK,  CR 1/2 */
	{0,      6000,   0, CEIL(6000*10, 9), 0x00, WLC_RATE_6M},

#if defined(WLPROPRIETARY_11N_RATES)
	/*
	 * MCS 33..86 are unused but still defined in this table. Reasons: to speed up lookup, to
	 * reduce the number of code modifications for this new feature and to make it more straight
	 * forward in the future to add more proprietary MCS'es.
	 */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 33 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 34 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 35 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 36 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 37 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 38 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 39 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 40 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 41 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 42 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 43 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 44 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 45 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 46 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 47 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 48 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 49 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 50 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 51 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 52 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 53 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 54 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 55 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 56 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 57 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 58 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 59 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 60 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 61 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 62 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 63 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 64 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 65 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 66 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 67 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 68 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 69 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 70 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 71 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 72 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 73 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 74 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 75 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 76 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 77 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 78 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 79 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 80 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 81 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 82 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 83 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 84 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 85 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 86 unused */
	/* MCS 87: SS 1, MOD: 256QAM, CR 3/4 */
	{78000,  162000, CEIL(78000*10, 9),  CEIL(162000*10, 9), /* */
	(NSS1 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R34, WLC_RATE_54M },
	/* MCS 88: SS 1, MOD: 256QAM, CR 5/6 */
	{86500,  180000, CEIL(86500*10, 9),  CEIL(180000*10, 9),
	(NSS1 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R56, WLC_RATE_54M },
	{0, 0, 0, 0, 0x00, 0},	/* MCS 89 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 90 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 91 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 92 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 93 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 94 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 95 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 96 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 97 unused */
	{0, 0, 0, 0, 0x00, 0},	/* MCS 98 unused */
	/* MCS 99: SS 2, MOD: 256QAM, CR 3/4 */
	{156000, 324000, CEIL(156000*10, 9), CEIL(324000*10, 9),
	(NSS2 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R34, WLC_RATE_54M },
	/* MCS 100: SS 2, MOD: 256QAM, CR 5/6 */
	{173250, 360000, CEIL(173250*10, 9), CEIL(360000*10, 9),
	(NSS2 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R56, WLC_RATE_54M },
	/* MCS 101: SS 3, MOD: 256QAM, CR 3/4 */
	{234000, 486000, CEIL(234000*10, 9), CEIL(486000*10, 9),
	(NSS3 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R34, WLC_RATE_54M },
	/* MCS 102: SS 3, MOD: 256QAM, CR 5/6 */
	{260000, 540000, CEIL(260000*10, 9), CEIL(540000*10, 9),
	(NSS3 << NSS_SHIFT) | (QAM256 << MOD_SHIFT) | R56, WLC_RATE_54M },
#endif /* WLPROPRIETARY_11N_RATES */
};

/** Hardware rates (also encodes default basic rates) */
const wlc_rateset_t cck_ofdm_mimo_rates = {
	12, /* number of rates following */
	/* rates in 500kbps units w/hi bit set if basic: */
	{ /* 1b,   2b,   5.5b, 6,    9,    11b,  12,   18,   24,   36,   48,   54 Mbps */
	   0x82, 0x84, 0x8b, 0x0c, 0x12, 0x96, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
	},
	0x00, /* htphy_membership */
	/* supported mcs index bit map (runs from MCS 0..128), LS bit in byte is lowest MCS */
	{
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#if !defined(WLPROPRIETARY_11N_RATES)
		0x00, 0x00, 0x00,
#else /* adds MCS87, 88, 99, 100, 101, 102 to bit map */
		0x80, 0x01, 0x78,
#endif /* WLPROPRIETARY_11N_RATES */
		0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_0_9_NSS4, /* supported vht mcs nss bit map */

	VHT_PROP_MCS_MAP_10_11_NSS4 /* prop vht mcs nss bit map */
};

const wlc_rateset_t ofdm_mimo_rates = {
	8,
	{ /*	6b,   9,    12b,  18,   24b,  36,   48,   54 Mbps */
		0x8c, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c
	},
	0x00,
	{
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#if !defined(WLPROPRIETARY_11N_RATES)
		0x00, 0x00, 0x00,
#else /* adds MCS87, 88, 99, 100, 101, 102 to bit map */
		0x80, 0x01, 0x78,
#endif /* WLPROPRIETARY_11N_RATES */
		0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_0_9_NSS4,

	VHT_PROP_MCS_MAP_10_11_NSS4 /* prop vht mcs nss bit map */
};

/**
 * '_40bw_' : default rate sets that include MCS32 for 40BW channels.
 * MCS 32 is an 802.11n frame format that duplicates a data frame in both halves of the 40 MHz
 * channel. By duplicating the same frame in both halves of the channel, the reliability is improved
 * at the receiver.
 */

const wlc_rateset_t cck_ofdm_40bw_mimo_rates = {
	12,
	{ /*	1b,   2b,   5.5b, 6,    9,    11b,  12,   18,   24,   36,   48,   54 Mbps */
		0x82, 0x84, 0x8b, 0x0c, 0x12, 0x96, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
	},
	0x00,
	{
		0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
#if !defined(WLPROPRIETARY_11N_RATES)
		0x00, 0x00, 0x00,
#else /* adds MCS87, 88, 99, 100, 101, 102 to bit map */
		0x80, 0x01, 0x78,
#endif /* WLPROPRIETARY_11N_RATES */
		0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_0_9_NSS4,

	VHT_PROP_MCS_MAP_10_11_NSS4 /* prop vht mcs nss bit map */
};

const wlc_rateset_t ofdm_40bw_mimo_rates = {
	8,
	{ /*	6b,   9,    12b,  18,   24b,  36,   48,   54 Mbps */
		0x8c, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c
	},
	0x00,
	{
		0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
#if !defined(WLPROPRIETARY_11N_RATES)
		0x00, 0x00, 0x00,
#else /* adds MCS87, 88, 99, 100, 101, 102 to bit map */
		0x80, 0x01, 0x78,
#endif /* WLPROPRIETARY_11N_RATES */
		0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_0_9_NSS4,

	VHT_PROP_MCS_MAP_10_11_NSS4 /* prop vht mcs nss bit map */
};

const wlc_rateset_t cck_ofdm_rates = {
	12,
	{ /*	1b,   2b,   5.5b, 6,    9,    11b,  12,   18,   24,   36,   48,   54 Mbps */
		0x82, 0x84, 0x8b, 0x0c, 0x12, 0x96, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

const wlc_rateset_t gphy_legacy_rates = {
	4,
	{ /*	1b,   2b,   5.5b,  11b Mbps */
		0x82, 0x84, 0x8b, 0x96
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

const wlc_rateset_t ofdm_rates = {
	8,
	{ /*	6b,   9,    12b,  18,   24b,  36,   48,   54 Mbps */
		0x8c, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

const wlc_rateset_t cck_rates = {
	4,
	{ /*	1b,   2b,   5.5,  11 Mbps */
		0x82, 0x84, 0x0b, 0x16
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

/**
 * set of rates in supported rates elt when splitting rates between it and the Extended Rates elt
 */
const wlc_rateset_t wlc_lrs_rates = {
	8,
	{ /*	1,    2,    5.5,  11,   18,   24,   36,   54 Mbps */
		0x02, 0x04, 0x0b, 0x16, 0x24, 0x30, 0x48, 0x6c
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

/**
 * Rateset including only 1 and 2 Mbps, without basic rate annotation.
 * This is used for a wlc_rate_hwrs_filter_sort_validate param when limiting a rateset for
 * Japan Channel 14 regulatory compliance using early radio revs
 */
const wlc_rateset_t rate_limit_1_2 = {
	2,
	{ /*	1,    2  Mbps */
		0x02, 0x04
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	},
	VHT_CAP_MCS_MAP_NONE_ALL,
	VHT_PROP_MCS_MAP_NONE_ALL
};

/**
 * Some functions require a single stream MCS as an input parameter. Given an MCS, this function
 * returns the single spatial stream MCS equivalent.
 */
uint8 wlc_rate_get_single_stream_mcs(uint mcs)
{
	if (mcs < 32)
		return mcs % 8;
	switch (mcs) {
		case 32:
			return 32;
		case 87:
		case 99:
		case 101:
			return 87;	/* MCS 87: SS 1, MOD: 256QAM, CR 3/4 */
		default:
			return 88;	/* MCS 88: SS 1, MOD: 256QAM, CR 5/6 */
	}
}

/**
 * A rateset contains an HT MCS bitmap, sometimes the need arises to clear all proprietary MCS'es
 * in that bitmap in a fast(er) way than clearing individual bits.
 */
void
wlc_rate_clear_prop_11n_mcses(uint8 mcs_bitmap[])
{
	int i;
	for (i = WLC_11N_FIRST_PROP_MCS / NBBY; i <= WLC_11N_LAST_PROP_MCS / NBBY; i++)
		mcs_bitmap[i] = 0;
}
