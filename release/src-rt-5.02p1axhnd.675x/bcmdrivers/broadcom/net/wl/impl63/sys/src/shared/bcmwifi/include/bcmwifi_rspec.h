/*
 * Common OS-independent driver header for rate management.
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
 * $Id: bcmwifi_rspec.h 779517 2019-10-01 14:48:46Z $
 */

#ifndef _bcmwifi_rspec_h_
#define _bcmwifi_rspec_h_

#include <typedefs.h>

/**
 * ===================================================================================
 * rate spec : holds rate and mode specific information required to generate a tx frame.
 * Legacy CCK and OFDM information is held in the same manner as was done in the past.
 * (in the lower byte) the upper 3 bytes primarily hold MIMO specific information
 * ===================================================================================
 */
typedef uint32	ratespec_t;

/* Rate spec. definitions */
#define WL_RSPEC_RATE_MASK	0x000000FF	/**< Legacy rate or MCS or MCS + NSS */
#define WL_RSPEC_TXEXP_MASK	0x00000300	/**< Tx chain expansion beyond Nsts */
#define WL_RSPEC_TXEXP_SHIFT	8
#define WL_RSPEC_HE_GI_MASK	0x00000C00	/* HE GI indices */
#define WL_RSPEC_HE_GI_SHIFT	10
#define WL_RSPEC_BW_MASK	0x00070000	/**< Band width */
#define WL_RSPEC_BW_SHIFT	16
#define WL_RSPEC_DCM		0x00080000	/**< Dual Carrier Modulation */
#define WL_RSPEC_STBC		0x00100000	/**< STBC expansion, Nsts = 2 * Nss */
#define WL_RSPEC_TXBF		0x00200000
#define WL_RSPEC_LDPC		0x00400000
#define WL_RSPEC_SGI		0x00800000
#define WL_RSPEC_SHORT_PREAMBLE	0x00800000	/**< DSSS short preable - Encoding 0 */
#define WL_RSPEC_ENCODING_MASK	0x03000000	/**< Encoding of RSPEC_RATE field */
#define WL_RSPEC_ENCODING_SHIFT	24

#define WL_RSPEC_OVERRIDE_RATE	0x40000000	/**< override rate only */
#define WL_RSPEC_OVERRIDE_MODE	0x80000000	/**< override both rate & mode */

/* ======== RSPEC_HE_GI|RSPEC_SGI fields for HE ======== */

/* GI for HE */
#define RSPEC_HE_LTF_GI(rspec)	(((rspec) & WL_RSPEC_HE_GI_MASK) >> WL_RSPEC_HE_GI_SHIFT)
#define WL_RSPEC_HE_1x_LTF_GI_0_8us	(0x0)
#define WL_RSPEC_HE_2x_LTF_GI_0_8us	(0x1)
#define WL_RSPEC_HE_2x_LTF_GI_1_6us	(0x2)
#define WL_RSPEC_HE_4x_LTF_GI_3_2us	(0x3)
#define RSPEC_ISHEGI(rspec)	(RSPEC_HE_LTF_GI(rspec) > WL_RSPEC_HE_1x_LTF_GI_0_8us)
#define HE_GI_TO_RSPEC(gi)	(((gi) << WL_RSPEC_HE_GI_SHIFT) & WL_RSPEC_HE_GI_MASK)

#define WL_RSPEC_GI_MASK	(WL_RSPEC_HE_GI_MASK | WL_RSPEC_SGI)

/* HE GI value in sgi_tx iovar start from 2 */
#define WL_SGI_HEGI_DELTA	2
#define WL_HEGI_VAL(x)	(x + WL_SGI_HEGI_DELTA)

/* ======== RSPEC_RATE field ======== */

/* Encoding 0 - legacy rate */
/* DSSS, CCK, and OFDM rates in [500kbps] units */
#define WL_RSPEC_LEGACY_RATE_MASK	0x0000007F
#define WLC_RATE_1M	2
#define WLC_RATE_2M	4
#define WLC_RATE_5M5	11
#define WLC_RATE_11M	22
#define WLC_RATE_6M	12
#define WLC_RATE_9M	18
#define WLC_RATE_12M	24
#define WLC_RATE_18M	36
#define WLC_RATE_24M	48
#define WLC_RATE_36M	72
#define WLC_RATE_48M	96
#define WLC_RATE_54M	108

/* Encoding 1 - HT MCS */
#define WL_RSPEC_HT_MCS_MASK	0x0000007F	/**< HT MCS value mask in rspec */

#define RSPEC_HT_MCS(rspec)	((rspec) & WL_RSPEC_HT_MCS_MASK)

/* Encoding 2 - VHT MCS + NSS */
#define WL_RSPEC_VHT_MCS_MASK	0x0000000F	/**< VHT MCS value mask in rspec */
#define WL_RSPEC_VHT_NSS_MASK	0x000000F0	/**< VHT Nss value mask in rspec */
#define WL_RSPEC_VHT_NSS_SHIFT	4		/**< VHT Nss value shift in rspec */

#define RSPEC_VHT_MCS(rspec)	((rspec) & WL_RSPEC_VHT_MCS_MASK)
#define RSPEC_VHT_NSS(rspec)	(((rspec) & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT)

/* Encoding 3 - HE MCS + NSS */
#define WL_RSPEC_HE_MCS_MASK	0x0000000F	/**< HE MCS value mask in rspec */
#define WL_RSPEC_HE_NSS_MASK	0x000000F0	/**< HE Nss value mask in rspec */
#define WL_RSPEC_HE_NSS_SHIFT	4		/**< HE Nss value shift in rpsec */

#define RSPEC_HE_MCS(rspec)	((rspec) & WL_RSPEC_HE_MCS_MASK)
#define RSPEC_HE_NSS(rspec)	(((rspec) & WL_RSPEC_HE_NSS_MASK) >> WL_RSPEC_HE_NSS_SHIFT)

/* ======== RSPEC_BW field ======== */

#define WL_RSPEC_BW_UNSPECIFIED	0
#define WL_RSPEC_BW_20MHZ	0x00010000
#define WL_RSPEC_BW_40MHZ	0x00020000
#define WL_RSPEC_BW_80MHZ	0x00030000
#define WL_RSPEC_BW_160MHZ	0x00040000

/* ======== RSPEC_ENCODING field ======== */

#define WL_RSPEC_ENCODE_RATE	0x00000000	/**< Legacy rate is stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_HT	0x01000000	/**< HT MCS is stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_VHT	0x02000000	/**< VHT MCS and NSS are stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_HE	0x03000000	/**< HE MCS and NSS are stored in RSPEC_RATE */

/**
 * ===============================
 * Handy macros to parse rate spec
 * ===============================
 */
#define RSPEC_BW(rspec)		((rspec) & WL_RSPEC_BW_MASK)
#define RSPEC_IS20MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_20MHZ)
#define RSPEC_IS40MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_40MHZ)
#define RSPEC_IS80MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_80MHZ)
#define RSPEC_IS160MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_160MHZ)

#define RSPEC_ISSGI(rspec)	(((rspec) & WL_RSPEC_SGI) != 0)
#define RSPEC_ISLDPC(rspec)	(((rspec) & WL_RSPEC_LDPC) != 0)
#define RSPEC_ISSTBC(rspec)	(((rspec) & WL_RSPEC_STBC) != 0)
#define RSPEC_ISTXBF(rspec)	(((rspec) & WL_RSPEC_TXBF) != 0)

#define RSPEC_TXEXP(rspec)	(((rspec) & WL_RSPEC_TXEXP_MASK) >> WL_RSPEC_TXEXP_SHIFT)

#define RSPEC_ENCODE(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) >> WL_RSPEC_ENCODING_SHIFT)
#define RSPEC_ISLEGACY(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_RATE)
#define	RSPEC_ISCCK(rspec)	(RSPEC_ISLEGACY(rspec) && \
				 (int8)rate_info[(rspec) & WL_RSPEC_LEGACY_RATE_MASK] > 0)
#define	RSPEC_ISOFDM(rspec)	(RSPEC_ISLEGACY(rspec) && \
				 (int8)rate_info[(rspec) & WL_RSPEC_LEGACY_RATE_MASK] < 0)
#define RSPEC_ISHT(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HT)
#ifdef WL11AC
#define RSPEC_ISVHT(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_VHT)
#else /* WL11AC */
#define RSPEC_ISVHT(rspec)	0
#endif /* WL11AC */
#ifdef WL11AX
#define RSPEC_ISHE(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HE)
#else /* WL11AX */
#define RSPEC_ISHE(rspec)	0
#endif /* WL11AX */

/**
 * ================================
 * Handy macros to create rate spec
 * ================================
 */
/* create ratespecs */
#define LEGACY_RSPEC(rate)	(WL_RSPEC_ENCODE_RATE | WL_RSPEC_BW_20MHZ | \
				 ((rate) & WL_RSPEC_LEGACY_RATE_MASK))
#define CCK_RSPEC(cck)		LEGACY_RSPEC(cck)
#define OFDM_RSPEC(ofdm)	LEGACY_RSPEC(ofdm)
#define HT_RSPEC(mcs)		(WL_RSPEC_ENCODE_HT | ((mcs) & WL_RSPEC_HT_MCS_MASK))
#define VHT_RSPEC(mcs, nss)	(WL_RSPEC_ENCODE_VHT | \
				 (((nss) << WL_RSPEC_VHT_NSS_SHIFT) & WL_RSPEC_VHT_NSS_MASK) | \
				 ((mcs) & WL_RSPEC_VHT_MCS_MASK))
#define HE_RSPEC(mcs, nss)	(WL_RSPEC_ENCODE_HE | \
				 (((nss) << WL_RSPEC_HE_NSS_SHIFT) & WL_RSPEC_HE_NSS_MASK) | \
				 ((mcs) & WL_RSPEC_HE_MCS_MASK))

/* convenience MACRO for lowest possible SU HE RATE */
#define LOWEST_RATE_HE_RSPEC	(HE_RSPEC(0, 1) | WL_RSPEC_BW_20MHZ | \
				HE_GI_TO_RSPEC(WL_RSPEC_HE_2x_LTF_GI_1_6us))

/**
 * ==================
 * Other handy macros
 * ==================
 */
/* return rate in unit of Kbps */
#define RSPEC2KBPS(rspec)	wf_rspec_to_rate(rspec)

/* return rate in unit of 500Kbps */
#ifdef BCMDBG
#define RSPEC2RATE(rspec)	wf_rspec_to_rate_legacy(rspec)
#else
#define RSPEC2RATE(rspec)	((rspec) & WL_RSPEC_LEGACY_RATE_MASK)
#endif // endif

/**
 * =================================
 * Macros to use the rate_info table
 * =================================
 */
/* phy_rate table index is in [500kbps] units */
#define WLC_MAXRATE	108	/**< in 500kbps units */
extern const uint8 rate_info[];
/* phy_rate table value is encoded */
#define	RATE_INFO_OFDM_MASK	0x80	/* ofdm mask */
#define	RATE_INFO_RATE_MASK	0x7f	/* rate signal index mask */
#define	RATE_INFO_M_RATE_MASK	0x0f	/* M_RATE_TABLE index mask */
#define	RATE_INFO_RATE_ISCCK(r)	((r) <= WLC_MAXRATE && (int8)rate_info[r] > 0)
#define	RATE_INFO_RATE_ISOFDM(r) ((r) <= WLC_MAXRATE && (int8)rate_info[r] < 0)

typedef enum {
	D11AX_RU26_TYPE,
	D11AX_RU52_TYPE,
	D11AX_RU106_TYPE,
	D11AX_RU242_TYPE,
	D11AX_RU484_TYPE,
	D11AX_RU996_TYPE,
	D11AX_RU1992_TYPE,
	D11AX_RU_MAX_TYPE
} ru_type_t;

/**
 * ===================
 * function prototypes
 * ===================
 */
bool wf_plcp_to_rspec(uint16 ft_fmt, uint8 *plcp, ratespec_t *rspec);
ratespec_t wf_vht_plcp_to_rspec(uint8 *plcp);
ratespec_t wf_he_plcp_to_rspec(uint8 *plcp, uint16 ft_fmt);
ratespec_t wf_ht_plcp_to_rspec(uint8 *plcp);

#ifdef BCMDBG
uint wf_rspec_to_rate_legacy(ratespec_t rspec);
#endif // endif

uint wf_rspec_to_rate(ratespec_t rspec);

ru_type_t wf_he_ruidx_to_ru_type(uint32 ruidx);
uint wf_he_rspec_ru_type_to_rate(ratespec_t rspec, ru_type_t ru_type);
uint32 wf_he_ru_type_to_nsd(ru_type_t ru_type);
uint32 wf_he_ru_type_to_tones(ru_type_t ru_type);

#endif /* _bcmwifi_rspec_h_ */
