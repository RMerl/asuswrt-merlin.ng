/*
 * Common OS-independent driver header for rate management.
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
 * $Id: bcmwifi_rspec.h 824001 2023-04-17 09:42:10Z $
 */

#ifndef _bcmwifi_rspec_h_
#define _bcmwifi_rspec_h_

#include <typedefs.h>

#if !defined(BCMDRIVER) || defined(WL11BE)
#define WF_EHT				/**< optimization for non-11be firmware or driver */
#endif /* !BCMDRIVER || WL11BE */

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
#define WL_RSPEC_EHT_GI_MASK	0x00000C00	/* EHT GI indices */
#define WL_RSPEC_EHT_GI_SHIFT	10
#define WL_RSPEC_BW_MASK	0x00070000	/**< Band width */
#define WL_RSPEC_BW_SHIFT	16
#define WL_RSPEC_HE_DCM		0x00080000	/**< HE Dual Carrier Modulation */
#define WL_RSPEC_DCM		WL_RSPEC_HE_DCM /**< legacy definition, will be obsoleted */
#define WL_RSPEC_STBC		0x00100000	/**< STBC expansion, Nsts = 2 * Nss */
#define WL_RSPEC_TXBF		0x00200000
#define WL_RSPEC_LDPC		0x00400000
#define WL_RSPEC_SGI		0x00800000
#define WL_RSPEC_SHORT_PREAMBLE	0x00800000	/**< DSSS short preable - Encoding 0 */
#define WL_RSPEC_ENCODING_MASK	0x07000000	/**< Encoding of RSPEC_RATE field */
#define WL_RSPEC_ENCODING_SHIFT	24

#define WL_RSPEC_OVERRIDE_RATE	0x40000000	/**< override rate only */
#define WL_RSPEC_OVERRIDE_MODE	0x80000000	/**< override both rate & mode */

/* ======== RSPEC_HE_GI|RSPEC_SGI fields for HE ======== */

/* GI for HE */
#define RSPEC_HE_LTF_GI(rspec)		(((rspec) & WL_RSPEC_HE_GI_MASK) >> WL_RSPEC_HE_GI_SHIFT)

enum wl_rspec_he_gi_e {
	WL_RSPEC_HE_1x_LTF_GI_0_8us = 0x0,
	WL_RSPEC_HE_2x_LTF_GI_0_8us = 0x1,
	WL_RSPEC_HE_2x_LTF_GI_1_6us = 0x2,
	WL_RSPEC_HE_4x_LTF_GI_3_2us = 0x3
};

#define RSPEC_IS_VALID_HEGI(rspec)	(RSPEC_HE_LTF_GI(rspec) > WL_RSPEC_HE_1x_LTF_GI_0_8us)
#define RSPEC_ISHESGI(rspec)		(RSPEC_HE_LTF_GI(rspec) <= WL_RSPEC_HE_2x_LTF_GI_0_8us)
#define HE_GI_TO_RSPEC(gi)		(((gi) << WL_RSPEC_HE_GI_SHIFT) & WL_RSPEC_HE_GI_MASK)

#define WL_RSPEC_GI_MASK		(WL_RSPEC_HE_GI_MASK | WL_RSPEC_SGI)

/* HE GI value in sgi_tx iovar start from 2 */
#define WL_SGI_HEGI_DELTA		2
#define WL_HEGI_VAL(x)			(x + WL_SGI_HEGI_DELTA)

/* ======== RSPEC_EHT_GI|RSPEC_SGI fields for EHT ======== */

/* GI for EHT */
#define RSPEC_EHT_LTF_GI(rspec)		(((rspec) & WL_RSPEC_EHT_GI_MASK) >> WL_RSPEC_EHT_GI_SHIFT)

enum wl_rspec_eht_gi_e {
	WL_RSPEC_EHT_2x_LTF_GI_0_8us = 0x0,
	WL_RSPEC_EHT_2x_LTF_GI_1_6us = 0x1,
	WL_RSPEC_EHT_4x_LTF_GI_0_8us = 0x2,
	WL_RSPEC_EHT_4x_LTF_GI_3_2us = 0x3
};

#define RSPEC_IS_VALID_EHTGI(rspec)	(RSPEC_EHT_LTF_GI(rspec) != WL_RSPEC_EHT_4x_LTF_GI_0_8us)
#define RSPEC_ISEHTSGI(rspec)		(RSPEC_EHT_LTF_GI(rspec) == WL_RSPEC_EHT_2x_LTF_GI_0_8us)
#define EHT_GI_TO_RSPEC(gi)		(((gi) << WL_RSPEC_EHT_GI_SHIFT) & WL_RSPEC_EHT_GI_MASK)
#define WL_SGI_EHTGI_DELTA		WL_SGI_HEGI_DELTA
#define WL_EHTGI_VAL(x)			WL_HEGI_VAL(x)

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

/* Encoding 1 - HT MCS + NSS */
#define WL_RSPEC_HT_MCS_MASK	0x0000007F	/**< HT MCS value mask in rspec */
#define RSPEC_HT_MCS(rspec)	((rspec) & WL_RSPEC_HT_MCS_MASK)
#define WL_RSPEC_HT_1SS_MCS_MASK 0x00000007	/**< single stream HT MCS value mask in rspec */
#define RSPEC_HT_1SS_MCS(rspec)	((rspec) & WL_RSPEC_HT_1SS_MCS_MASK)
#define WL_RSPEC_HT_NSS_MASK	0x00000078	/**< HT Nss value mask in rate field */
#define WL_RSPEC_HT_NSS_SHIFT	3		/**< HT Nss value shift in rate field */
#define RSPEC_HT_NSS(rspec)		(((rspec) & WL_RSPEC_HT_NSS_MASK) >> WL_RSPEC_HT_NSS_SHIFT)

/* Encoding 2 - VHT MCS + NSS */
#define WL_RSPEC_GE_VHT_MCS_MASK 0x0000000F	/**< VHT/HE/EHT/.. MCS value mask in rspec */
#define WL_RSPEC_VHT_MCS_MASK	WL_RSPEC_GE_VHT_MCS_MASK /**< obsolete definition */
#define WL_RSPEC_GE_VHT_NSS_MASK 0x000000F0	/**< VHT/HE/EHT/.. Nss value mask in rspec */
#define WL_RSPEC_VHT_NSS_MASK	WL_RSPEC_GE_VHT_NSS_MASK  /**< obsolete definition */
#define WL_RSPEC_GE_VHT_NSS_SHIFT	4	/**< VHT/HE/EHT/.. Nss value shift in rspec */
#define WL_RSPEC_VHT_NSS_SHIFT	WL_RSPEC_GE_VHT_NSS_SHIFT	/**< obsolete definition */
#define RSPEC_GE_VHT_MCS(rspec)	((rspec) & WL_RSPEC_GE_VHT_MCS_MASK)
#define RSPEC_VHT_MCS(rspec)	RSPEC_GE_VHT_MCS(rspec)		/**< obsolete definition */
/** @RSPEC_GE_VHT_NSS() always returns a value > 0 */
#define RSPEC_GE_VHT_NSS(rspec)	(((rspec) & WL_RSPEC_GE_VHT_NSS_MASK) >> WL_RSPEC_GE_VHT_NSS_SHIFT)
#define RSPEC_VHT_NSS(rspec)	RSPEC_GE_VHT_NSS(rspec)		/**< obsolete definition */

/* Encoding 3 - HE MCS + NSS */
#define WL_RSPEC_HE_MCS_MASK	WL_RSPEC_GE_VHT_MCS_MASK	/**< obsolete definition */
#define WL_RSPEC_HE_NSS_MASK	WL_RSPEC_GE_VHT_NSS_MASK	/**< obsolete definition */
#define WL_RSPEC_HE_NSS_SHIFT	WL_RSPEC_GE_VHT_NSS_SHIFT	/**< obsolete definition */

#define RSPEC_HE_MCS(rspec)	RSPEC_GE_VHT_MCS(rspec)		/**< obsolete definition */
#define RSPEC_HE_NSS(rspec)	RSPEC_GE_VHT_NSS(rspec)		/**< obsolete definition */

/* Encoding 4 - EHT MCS + NSS */
#define WL_RSPEC_EHT_MCS_MASK	WL_RSPEC_GE_VHT_MCS_MASK	/**< obsolete definition */
#define WL_RSPEC_EHT_NSS_MASK	WL_RSPEC_GE_VHT_NSS_MASK	/**< obsolete definition */
#define WL_RSPEC_EHT_NSS_SHIFT	WL_RSPEC_GE_VHT_NSS_SHIFT	/**< obsolete definition */

#define RSPEC_EHT_MCS(rspec)	RSPEC_GE_VHT_MCS(rspec)		/**< obsolete definition */
#define RSPEC_EHT_NSS(rspec)	RSPEC_GE_VHT_NSS(rspec)		/**< obsolete definition */

/* ======== RSPEC_BW field ======== */

enum wl_rspec_bw_e {
	WL_RSPEC_BW_UNSPECIFIED = 0,
	WL_RSPEC_BW_20MHZ	= 0x00010000,
	WL_RSPEC_BW_40MHZ	= 0x00020000,
	WL_RSPEC_BW_80MHZ	= 0x00030000,
	WL_RSPEC_BW_160MHZ	= 0x00040000,
	WL_RSPEC_BW_320MHZ	= 0x00050000
};

/* ======== RSPEC_ENCODING field ======== */

#define WL_RSPEC_ENCODE_RATE	0x00000000	/**< Legacy rate is stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_HT	0x01000000	/**< HT MCS is stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_VHT	0x02000000	/**< VHT MCS and NSS are stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_HE	0x03000000	/**< HE MCS and NSS are stored in RSPEC_RATE */
#define WL_RSPEC_ENCODE_EHT	0x04000000	/**< EHT MCS and NSS are stored in RSPEC_RATE */

/**
 * ===============================
 * Handy macros to parse rate spec
 * ===============================
 */
#define RSPEC_BW(rspec)		((rspec) & WL_RSPEC_BW_MASK) /**< returns wl_rspec_bw_e */
#define RSPEC_IS20MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_20MHZ)
#define RSPEC_IS40MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_40MHZ)
#define RSPEC_IS80MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_80MHZ)
#define RSPEC_IS160MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_160MHZ)

#ifdef WF_EHT
#define RSPEC_IS320MHZ(rspec)	(RSPEC_BW(rspec) == WL_RSPEC_BW_320MHZ)
#else
#define RSPEC_IS320MHZ(rspec)	FALSE
#endif /* WF_EHT */

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
#define RSPEC_ISVHT(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_VHT)
#define RSPEC_ISHE(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HE)

#ifdef WF_EHT
#define RSPEC_ISEHT(rspec)	(((rspec) & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_EHT)
#else
#define RSPEC_ISEHT(rspec)	FALSE
#endif /* WF_EHT */

#define RSPEC_GE_VHT(rspec)	(RSPEC_ISVHT(rspec) || RSPEC_ISHE(rspec) || RSPEC_ISEHT(rspec))

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
				 (((nss) << WL_RSPEC_GE_VHT_NSS_SHIFT) & \
				  WL_RSPEC_GE_VHT_NSS_MASK) | \
				 ((mcs) & WL_RSPEC_GE_VHT_MCS_MASK))
#define HE_RSPEC(mcs, nss)	(WL_RSPEC_ENCODE_HE | \
				 (((nss) << WL_RSPEC_GE_VHT_NSS_SHIFT) & \
				  WL_RSPEC_GE_VHT_NSS_MASK) | \
				 ((mcs) & WL_RSPEC_GE_VHT_MCS_MASK))
#define EHT_RSPEC(mcs, nss)	(WL_RSPEC_ENCODE_EHT | \
				 (((nss) << WL_RSPEC_GE_VHT_NSS_SHIFT) & \
				  WL_RSPEC_GE_VHT_NSS_MASK) | \
				 ((mcs) & WL_RSPEC_GE_VHT_MCS_MASK))

/* convenience MACRO for lowest possible SU HE RATE */
#define LOWEST_RATE_HE_RSPEC	(HE_RSPEC(0, 1) | WL_RSPEC_BW_20MHZ | \
				HE_GI_TO_RSPEC(WL_RSPEC_HE_2x_LTF_GI_1_6us))

/* convenience MACRO for lowest possible SU EHT RATE */
#define LOWEST_RATE_EHT_RSPEC	(EHT_RSPEC(0, 1) | WL_RSPEC_BW_20MHZ | \
				 EHT_GI_TO_RSPEC(WL_RSPEC_EHT_2x_LTF_GI_1_6us))

/**
 * ==================
 * Other handy macros
 * ==================
 */
/* return rate in unit of Kbps */
#define RSPEC2KBPS(rspec)	wf_rspec_to_rate(rspec)

/* legacy format rspec to rate convertion for
 * DSSS, CCK and OFDM rates [1M..54M] in units of 500Kbps
 */
#ifdef BCMDBG
#define RSPEC_IS_LEGACY2RATE(rspec)	wf_rspec_to_rate_legacy(rspec)
#else
#define RSPEC_IS_LEGACY2RATE(rspec)	((rspec) & WL_RSPEC_LEGACY_RATE_MASK)
#endif

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

/** ru type, used for >= HE (so including EHT RUs) */
typedef enum ru_type_e {
	/* RUs */
	D11AX_RU26_TYPE			= 0,
	D11AX_RU52_TYPE			= 1,
	D11AX_RU106_TYPE		= 2,
	D11AX_RU242_TYPE		= 3,
	D11AX_RU484_TYPE		= 4,
	D11AX_RU996_TYPE		= 5,
	D11AX_RU1992_TYPE		= 6,
#ifdef WL11BE
	D11BE_RU3984_TYPE		= 7,	/** = 4x996, only for >= EHT */
	/* MRUs */
	D11BE_MRU52_26_TYPE		= 8,
	D11BE_MRU106_26_TYPE		= 9,
	D11BE_MRU484_242_TYPE		= 10,
	D11BE_MRU996_484_TYPE		= 11,
	D11BE_MRU996_484_242_TYPE	= 12,
	D11BE_MRU2x996_484_TYPE		= 13,
	D11BE_MRU3x996_TYPE		= 14,
	D11BE_MRU3x996_484_TYPE		= 15,
#endif /* WL11BE */
	D11_RU_MRU_UNKNOWN_TYPE		= 100  /** to indicate an illegal type */
} ru_type_t;

#define	D11AX_N_RU_TYPES		(D11AX_RU1992_TYPE + 1)

#ifdef WL11BE
#define D11BE_N_RU_TYPES		(D11BE_RU3984_TYPE + 1)
#define D11BE_N_RU_MRU_TYPES		(D11BE_MRU3x996_484_TYPE + 1)
#endif /* WL11BE */

#ifdef WL11BE
#define N_RU_TYPES			D11BE_N_RU_TYPES
#define N_RU_MRU_TYPES			D11BE_N_RU_MRU_TYPES
#else
#define N_RU_TYPES			D11AX_N_RU_TYPES
#define N_RU_MRU_TYPES			D11AX_N_RU_TYPES
#endif /* WL11BE */

#define MAX_TONE_RU_IDX(corerev) \
	(D11REV_GE(corerev, 133) ? EHT_MAX_3x996_484_RU_INDX : HE_MAX_2x996_TONE_RU_INDX)

/**
 * ===================
 * function prototypes
 * ===================
 */
#ifdef BCMDRIVER
bool wf_plcp_to_rspec(uint8 ft, uint8 fmt, uint8 *plcp, ratespec_t *rspec);
ratespec_t wf_he_plcp_to_rspec(uint8 *plcp, uint8 ft, uint8 hefmt);
#ifdef WL11BE
ratespec_t wf_eht_rx_plcp_to_rspec(uint8 *plcp, uint8 ft, uint8 ehtfmt);
#endif /* WL11BE */
#endif /* BCMDRIVER */
ratespec_t wf_vht_plcp_to_rspec(uint8 *plcp);
ratespec_t wf_ht_plcp_to_rspec(uint8 *plcp);

#ifdef BCMDBG
uint wf_rspec_to_rate_legacy(ratespec_t rspec);
#endif

uint wf_rspec_to_rate(ratespec_t rspec);

#define wf_he_ruidx_to_ru_type wf_ofdma_ruidx_to_ru_type /* wf_he_ruidx_to_ru_type is obsolete */
ru_type_t wf_ofdma_ruidx_to_ru_type(uint8 ruidx_7b);

uint wf_he_rspec_ru_type_to_rate(ratespec_t rspec, enum ru_type_e ru_type);
uint32 wf_he_ru_type_to_nsd(enum ru_type_e ru_type);
uint32 wf_he_ru_type_to_tones(enum ru_type_e ru_type);
#ifdef WL11BE
uint32 wf_eht_rspec_ru_type_to_rate(ratespec_t rspec, enum ru_type_e ru_type);
uint32 wf_eht_ru_type_to_nsd(enum ru_type_e ru_type);
uint32 wf_eht_ru_type_to_tones(enum ru_type_e ru_type);
#endif /* WL11BE */

#endif /* _bcmwifi_rspec_h_ */
