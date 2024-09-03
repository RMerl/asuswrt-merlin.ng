/*
 * Common [OS-independent] rate management
 * 802.11 Networking Adapter Device Driver.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Dual>>
 *
 * $Id: bcmwifi_rspec.c 835526 2024-01-19 17:36:29Z $
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#include <d11_pub.h>
#endif

#include <802.11ax.h>
#ifdef WL11BE
#include <802.11be.h>
#endif /* WL11BE */

#include <bcmwifi_rspec.h>
#include <bcmwifi_rates.h>
#include <bcmutils.h>

#ifndef ASSERT
#define ASSERT(exp)
#endif

/* TODO: Consolidate rspec utility functions from wlc_rate.c and bcmwifi_monitor.c
 * into here if they're shared by non wl layer as well...
 */

/* ============================================ */
/* ============================================ */

/**
 * Returns the rate in [Kbps] units, 0 for invalid ratespec.
 */
static uint
wf_he_rspec_to_rate(ratespec_t rspec)
{
	uint mcs = RSPEC_GE_VHT_MCS(rspec);
	uint nss = RSPEC_GE_VHT_NSS(rspec);
	bool dcm = (rspec & WL_RSPEC_HE_DCM) != 0;
	enum wl_bw_e bw =  RSPEC_BW(rspec) >> WL_RSPEC_BW_SHIFT;
	uint gi =  RSPEC_HE_LTF_GI(rspec);

	if (mcs <= WLC_MAX_HE_MCS && nss >= WLC_MIN_NSS && nss <= WLC_MAX_HE_NSS) {
		return wf_he_mcs_to_kbps(mcs, nss, bw, gi, dcm);
	}
#ifdef BCMDBG
	printf("%s: rspec 0x%x, mcs %u, nss %u\n", __FUNCTION__, rspec, mcs, nss);
#endif
	ASSERT(mcs <= WLC_MAX_HE_MCS);
	ASSERT(nss >= WLC_MIN_NSS && nss <= WLC_MAX_HE_NSS);
	return 0;
} /* wf_he_rspec_to_rate */

uint
wf_he_rspec_ru_type_to_rate(ratespec_t rspec, enum ru_type_e ru_type)
{
	uint mcs = RSPEC_GE_VHT_MCS(rspec);
	uint nss = RSPEC_GE_VHT_NSS(rspec);
	bool dcm = (rspec & WL_RSPEC_HE_DCM) != 0;
	uint gi =  RSPEC_HE_LTF_GI(rspec);
	uint nsd;

	ASSERT(mcs <= WLC_MAX_HE_MCS);
	ASSERT(nss >= WLC_MIN_NSS && nss <= WLC_MAX_HE_NSS);

	nsd = wf_ru_type_to_nsd(ru_type);
	if (mcs <= WLC_MAX_HE_MCS && nss >= WLC_MIN_NSS && nss <= WLC_MAX_HE_NSS) {
		return wf_he_mcs_nsd_to_kbps(mcs, nss, nsd, gi, dcm);
	}

	return 0;
} /* wf_he_rspec_ru_type_to_rate */

#ifdef WL11BE
/**
 * Returns the rate in [Kbps] units, 0 for invalid ratespec.
 */
static uint
wf_eht_rspec_to_rate(ratespec_t rspec)
{
	uint mcs = RSPEC_GE_VHT_MCS(rspec);
	uint nss = RSPEC_GE_VHT_NSS(rspec);
	enum wl_bw_e bw = (RSPEC_BW(rspec) >> WL_RSPEC_BW_SHIFT);
	uint gi =  RSPEC_EHT_LTF_GI(rspec);

	ASSERT(mcs <= WLC_MAX_EHT_MCS);
	ASSERT(nss >= WLC_MIN_NSS && nss <= WLC_MAX_EHT_NSS);

	if (mcs <= WLC_MAX_EHT_MCS && nss >= WLC_MIN_NSS && nss <= WLC_MAX_EHT_NSS) {
		return wf_eht_mcs_to_kbps(mcs, nss, bw, gi);
	}
#ifdef BCMDBG
	printf("%s: rspec 0x%x, mcs %u, nss %u, bw: %u\n", __FUNCTION__, rspec, mcs, nss, bw);
#endif
	return 0;
} /* wf_eht_rspec_to_rate */

/**
 * Returns the rate in [Kbps] units, 0 for invalid ratespec.
 */
uint
wf_eht_rspec_ru_type_to_rate(ratespec_t rspec, enum ru_type_e ru_type)
{
	uint mcs = RSPEC_GE_VHT_MCS(rspec);
	uint nss = RSPEC_GE_VHT_NSS(rspec);
	uint gi =  RSPEC_EHT_LTF_GI(rspec);
	uint nsd;

	ASSERT(mcs <= WLC_MAX_EHT_MCS);
	ASSERT(nss >= WLC_MIN_NSS && nss <= WLC_MAX_EHT_NSS);

	nsd = wf_ru_type_to_nsd(ru_type);
	if (mcs <= WLC_MAX_EHT_MCS && nss >= WLC_MIN_NSS && nss <= WLC_MAX_EHT_NSS) {
		return wf_eht_mcs_nsd_to_kbps(mcs, nss, nsd, gi);
	}

	return 0;
} /* wf_eht_rspec_ru_type_to_rate */
#endif /* WL11BE */

/** take a well formed ratespec_t arg and return phy rate in [Kbps] units */
uint
wf_rspec_to_rate(ratespec_t rspec)
{
	uint rate = (uint)(-1);
	uint mcs, nss;

	switch (rspec & WL_RSPEC_ENCODING_MASK) {
#ifdef WL11BE
		case WL_RSPEC_ENCODE_EHT:
			rate = wf_eht_rspec_to_rate(rspec);
			break;
#endif /* WL11BE */
		case WL_RSPEC_ENCODE_HE:
			rate = wf_he_rspec_to_rate(rspec);
			break;
		case WL_RSPEC_ENCODE_VHT:
			mcs = RSPEC_GE_VHT_MCS(rspec);
			nss = RSPEC_GE_VHT_NSS(rspec);
#ifdef BCMDBG
			if (mcs > WLC_MAX_VHT_MCS || nss < WLC_MIN_NSS || nss > WLC_MAX_VHT_NSS) {
				printf("%s: rspec=%x\n", __FUNCTION__, rspec);
			}
#endif /* BCMDBG */
			ASSERT(mcs <= WLC_MAX_VHT_MCS);
			ASSERT(nss >= WLC_MIN_NSS && nss <= WLC_MAX_VHT_NSS);
			rate = wf_ht_vht_mcs_to_kbps(mcs, nss,
				RSPEC_BW(rspec), RSPEC_ISSGI(rspec));
			break;
		case WL_RSPEC_ENCODE_HT:
			mcs = RSPEC_HT_MCS(rspec);
#ifdef BCMDBG
			if (mcs > 32) {
				printf("%s: rspec=%x\n", __FUNCTION__, rspec);
			}
#endif /* BCMDBG */
			ASSERT(mcs <= 32);
			if (mcs == 32) {
				rate = wf_ht_vht_mcs_to_kbps(mcs, WLC_MIN_NSS, WL_RSPEC_BW_40MHZ,
					RSPEC_ISSGI(rspec));
			} else {
				nss = 1 + (mcs / 8);
				mcs = mcs % 8;
				rate = wf_ht_vht_mcs_to_kbps(mcs, nss, RSPEC_BW(rspec),
					RSPEC_ISSGI(rspec));
			}
			break;
		case WL_RSPEC_ENCODE_RATE:
			/* return rate in [Kbps] */
			rate = 500 * RSPEC_IS_LEGACY2RATE(rspec);
			break;
		default:
			ASSERT(0);
			break;
	}

	return (rate == 0) ? (uint)(-1) : rate;
}

#ifdef BCMDBG
/* Return the rate in 500Kbps units if the rspec is legacy rate, assert otherwise */
uint
wf_rspec_to_rate_legacy(ratespec_t rspec)
{
	ASSERT(RSPEC_ISLEGACY(rspec));

	return rspec & WL_RSPEC_LEGACY_RATE_MASK;
}
#endif

#ifdef BCMDRIVER
/**
 * Calculate ratespec from PLCP header.
 *
 * @param ft	Frame type.
 * @param fmt	(HE/EHT specific) PPDU format.
 * @param plcp	Pointer to PLCP. For EHT: RX PLCP!
 * @param rspec	Pointer to ratespec storage variable. Only modified if a ratespec was calculated.
 * @return	TRUE if a valid frame type was specified.
 */
bool BCMFASTPATH
wf_plcp_to_rspec(uint8 ft, uint8 fmt, uint8 *plcp, ratespec_t *rspec)
{
	ASSERT(plcp != NULL);
	ASSERT(rspec != NULL);

	switch (ft) {
#ifdef WL11BE
		case FT_EHT:
			/* function is only valid for RX EHT PLCP */
			*rspec = wf_eht_rx_plcp_to_rspec(plcp, ft, fmt);
			return TRUE;
#endif /* WL11BE */
#ifdef WL11AX
		case FT_HE:
			*rspec = wf_he_plcp_to_rspec(plcp, ft, fmt);
			return TRUE;
#endif /* WL11AX */
#ifdef WL11AC
		case FT_VHT:
			*rspec = wf_vht_plcp_to_rspec(plcp);
			return TRUE;
#endif /* WL11AC */
		case FT_HT:
			*rspec = wf_ht_plcp_to_rspec(plcp);
			return TRUE;

		case FT_OFDM:
			*rspec = OFDM_RSPEC(OFDM_PHY2MAC_RATE(((ofdm_phy_hdr_t *)plcp)->rlpt[0]));
			*rspec |= WL_RSPEC_BW_20MHZ;
			return TRUE;

		case FT_CCK:
			*rspec = CCK_RSPEC(CCK_PHY2MAC_RATE(((cck_phy_hdr_t *)plcp)->signal));
			*rspec |= WL_RSPEC_BW_20MHZ;
			return TRUE;

		default:
			/* Don't modify *rspec */
			return FALSE;
	}
}

#ifdef WL11BE
/**
 * Function returning ratespec derived from EHT RX(!) PLCP header.
 *
 * @param plcp		Pointer to RX PLCP, points to EHT-USIG field.
 * @param ft		Frame type.
 * @param ehtfmt	EHT specific PPDU format.
 * @return		rspec computed from PLCP
 */
ratespec_t BCMFASTPATH
wf_eht_rx_plcp_to_rspec(uint8 *plcp, uint8 ft, uint8 ehtfmt)
{
	uint8 bw, gi;
	ratespec_t rspec;
	uint8 eht_usig;
	uint8 eht_sig; /* aka USIG-overflow */
	enum wl_rspec_bw_e bw_map[] = {WL_RSPEC_BW_20MHZ,
		WL_RSPEC_BW_40MHZ, WL_RSPEC_BW_80MHZ, WL_RSPEC_BW_160MHZ,
		WL_RSPEC_BW_320MHZ /* 320-1 */, WL_RSPEC_BW_320MHZ /* 320-2 */};
	uint8 gi_map[] = {WL_RSPEC_EHT_2x_LTF_GI_0_8us,
		WL_RSPEC_EHT_2x_LTF_GI_1_6us, WL_RSPEC_EHT_4x_LTF_GI_0_8us,
		WL_RSPEC_EHT_4x_LTF_GI_3_2us};

	ASSERT(plcp);

	eht_usig = plcp[0]; /* only interested in first byte of EHT-USIG */
	bw = (eht_usig & EHT_PLCP1_USIG_1_BW_MASK) >> EHT_PLCP1_USIG_1_BW_SHIFT;
	bw = (bw < ARRAYSIZE(bw_map)) ? bw : 0;

	rspec = WL_RSPEC_ENCODE_EHT | bw_map[bw] | wf_eht_rx_plcp_to_rspec_rate(plcp, ft, ehtfmt);

	switch (ehtfmt) {
		case HE_EHT_FMT_SU:
		case HE_EHT_FMT_SUER:
		case HE_EHT_FMT_MU:
			eht_sig = plcp[6]; /* only interested in first byte of EHT-SIG */
			gi = (eht_sig & EHT_PLCP_USIG_OVFL_CPLTF_MASK) >>
				EHT_PLCP_USIG_OVFL_CPLTF_SHIFT;
			gi = (gi < ARRAYSIZE(gi_map)) ? gi : 0;
			rspec |= EHT_GI_TO_RSPEC(gi_map[gi]);
			/* TXBF/LDPC found at end of user info so skip 6 (EHT-USIG) + 3 (EHT-SIG) */
			rspec |= ((plcp[9] & BCM_BIT(5) /* beamformed */) ? WL_RSPEC_TXBF : 0);
			rspec |= ((plcp[9] & BCM_BIT(6) /* coding */) ? WL_RSPEC_LDPC : 0);
			break;
		case HE_EHT_FMT_TB:
			rspec |= ((plcp[6] & BCM_BIT(4) /* coding */) ? WL_RSPEC_LDPC : 0);
			break;
		default:
#ifdef BCMDBG
			printf("%s: not supported EHT format %x\n", __FUNCTION__, ehtfmt);
			ASSERT(0);
#endif /* BCMDBG */
			break;
	}
	return rspec;
}
#endif /* WL11BE */

ratespec_t BCMFASTPATH
wf_he_plcp_to_rspec(uint8 *plcp, uint8 ft, uint8 hefmt)
{
	uint8 bw, gi;
	ratespec_t rspec;

	/* HE plcp - 6 B */
	uint32 plcp0;
	uint16 plcp1;
	uint32 sigb;

	ASSERT(plcp);

	gi = 0;
	bw = 0;
	rspec = WL_RSPEC_ENCODE_HE | wf_he_plcp_to_rspec_rate(plcp, ft, hefmt);

	switch (hefmt) {
		case HE_EHT_FMT_SU:
		case HE_EHT_FMT_SUER:
			plcp0 = (plcp[2] << 16) | plcp[0];	/* Don't need plcp[1] or plcp[3] */
			plcp1 = plcp[4];			/* Don't need plcp[5] */

			/* GI info comes from CP/LTF */
			gi = (plcp0 & HE_PLCP0_CPLTF_MASK) >> HE_PLCP0_CPLTF_SHIFT;
			bw = ((plcp0 & HE_PLCP0_BW_MASK) >> HE_PLCP0_BW_SHIFT) + 1;

			rspec |= ((plcp1 & HE_PLCP1_TXBF_MASK) ? WL_RSPEC_TXBF : 0);
			rspec |= ((plcp1 & HE_PLCP1_CODING_MASK) ? WL_RSPEC_LDPC : 0);
			rspec |= ((plcp1 & HE_PLCP1_STBC_MASK)? WL_RSPEC_STBC : 0);
			rspec |= ((plcp0 & HE_PLCP0_DCM_MASK)? WL_RSPEC_HE_DCM : 0);
			break;

		case HE_EHT_FMT_MU:
			sigb = (plcp[9] << 24) | (plcp[8] << 16) | (plcp[7] << 8) | plcp[6];
			plcp0 = (plcp[3] << 24) | (plcp[2] << 16) | plcp[0];
			plcp1 = plcp[4];

			gi = HEMU_PLCP0_CPLTF(plcp0);
			bw = HEMU_PLCP0_BW(plcp0);

			rspec |= ((sigb & HEMU_SIGB_TXBF_MASK) ? WL_RSPEC_TXBF : 0);
			rspec |= ((sigb & HEMU_SIGB_CODING_MASK) ? WL_RSPEC_LDPC : 0);
			rspec |= ((sigb & HEMU_SIGB_DCM_MASK) ? WL_RSPEC_HE_DCM : 0);
			rspec |= ((plcp1 & HEMU_PLCP1_STBC_MASK)? WL_RSPEC_STBC :0);
			break;

		case HE_EHT_FMT_TB:
			rspec |= ((plcp[3] & 0x3) + 1) << WL_RSPEC_BW_SHIFT;
			rspec |= (plcp[6] & 0x10) ? WL_RSPEC_LDPC : 0;
			break;

		default:
#ifdef BCMDBG
			printf("%s: not supported HE format %x\n", __FUNCTION__, hefmt);
			ASSERT(0);
#endif /* BCMDBG */
			break;
	}

	rspec |= HE_GI_TO_RSPEC(gi);
	rspec |= (bw << WL_RSPEC_BW_SHIFT);

	return rspec;
}
#endif /* BCMDRIVER */

ratespec_t BCMFASTPATH
wf_vht_plcp_to_rspec(uint8 *plcp)
{
	uint vht_sig_a1, vht_sig_a2;
	ratespec_t rspec;

	rspec = WL_RSPEC_ENCODE_VHT | wf_vht_plcp_to_rspec_rate(plcp);

	vht_sig_a1 = plcp[0] | (plcp[1] << 8);
	vht_sig_a2 = plcp[3] | (plcp[4] << 8);

	STATIC_ASSERT( /* VHT SIGA BW mapping to RSPEC BW needs correction */
		((VHT_SIGA1_20MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  == WL_RSPEC_BW_20MHZ &&
		((VHT_SIGA1_40MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  == WL_RSPEC_BW_40MHZ &&
		((VHT_SIGA1_80MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  == WL_RSPEC_BW_80MHZ &&
		((VHT_SIGA1_160MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  == WL_RSPEC_BW_160MHZ);

	rspec |= ((vht_sig_a1 & VHT_SIGA1_160MHZ_VAL) + 1) << WL_RSPEC_BW_SHIFT;
	if (vht_sig_a1 & VHT_SIGA1_STBC)
		rspec |= WL_RSPEC_STBC;
	if (vht_sig_a2 & VHT_SIGA2_GI_SHORT)
		rspec |= WL_RSPEC_SGI;
	if (vht_sig_a2 & VHT_SIGA2_CODING_LDPC)
		rspec |= WL_RSPEC_LDPC;

	return rspec;
}

ratespec_t BCMFASTPATH
wf_ht_plcp_to_rspec(uint8 *plcp)
{
	uint stbc;
	uint32 ht_sig1, ht_sig2;
	ratespec_t rspec;

	ht_sig1 = plcp[0];			/* Don't need plcp[1] or plcp[2] */
	ht_sig2 = (plcp[4] << 8) | plcp[3];	/* Don't need plcp[5] */

	rspec = HT_RSPEC(ht_sig1 & HT_SIG1_MCS_MASK);
	if (ht_sig1 & HT_SIG1_CBW) {
		rspec |= WL_RSPEC_BW_40MHZ;
	} else {
		rspec |= WL_RSPEC_BW_20MHZ;
	}
	if (ht_sig2 & HT_SIG2_SHORT_GI)
		rspec |= WL_RSPEC_SGI;
	if (ht_sig2 & HT_SIG2_FEC_CODING)
		rspec |= WL_RSPEC_LDPC;
	stbc = (ht_sig2 & HT_SIG2_STBC_MASK) >> HT_SIG2_STBC_SHIFT;
	if (stbc != 0)
		rspec |= WL_RSPEC_STBC;

	return rspec;
}

/* ============================================ */
/* ============================================ */

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
/* Note: make this table 128 elements so the result of (rspec & 0x7f) can be safely
 * used as the index into this table...
 */
const uint8 rate_info[128] = {
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
/* 100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c,
/* ------------- guard ------------ */                          0x00,
/* 110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

enum ru_type_e
wf_ofdma_ruidx_to_ru_type(uint8 ruidx_7b)
{
	uint32 ru_type;

	if (ruidx_7b <= HE_MAX_26_TONE_RU_INDX) {
		ru_type = D11AX_RU26_TYPE;
	} else if (ruidx_7b <= HE_MAX_52_TONE_RU_INDX) {
		ru_type = D11AX_RU52_TYPE;
	} else if (ruidx_7b <= HE_MAX_106_TONE_RU_INDX) {
		ru_type = D11AX_RU106_TYPE;
	} else if (ruidx_7b <= HE_MAX_242_TONE_RU_INDX) {
		ru_type = D11AX_RU242_TYPE;
	} else if (ruidx_7b <= HE_MAX_484_TONE_RU_INDX) {
		ru_type = D11AX_RU484_TYPE;
	} else if (ruidx_7b == HE_996_TONE_RU_INDX) {
		ru_type = D11AX_RU996_TYPE;
	} else if (ruidx_7b == HE_2x996_TONE_RU_INDX) {
		ru_type = D11AX_RU1992_TYPE;
#ifdef WL11BE
	} else if (ruidx_7b == EHT_4x996_TONE_RU_INDX) {
		ru_type = D11BE_RU3984_TYPE;
	} else if (ruidx_7b <= EHT_MAX_52_26_TONE_RU_INDX) {
		ru_type = D11BE_MRU52_26_TYPE;
	} else if (ruidx_7b <= EHT_MAX_106_26_TONE_RU_INDX) {
		ru_type = D11BE_MRU106_26_TYPE;
	} else if (ruidx_7b <= EHT_MAX_484_242_TONE_RU_INDX) {
		ru_type = D11BE_MRU484_242_TYPE;
	} else if (ruidx_7b <= EHT_MAX_996_484_RU_INDX) {
		ru_type = D11BE_MRU996_484_TYPE;
	} else if (ruidx_7b <= EHT_MAX_996_484_242_RU_INDX) {
		ru_type = D11BE_MRU996_484_242_TYPE;
	} else if (ruidx_7b <= EHT_MAX_2x996_484_RU_INDX) {
		ru_type = D11BE_MRU2x996_484_TYPE;
	} else if (ruidx_7b == EHT_3x996_RU_INDX) {
		ru_type = D11BE_MRU3x996_TYPE;
	} else if (ruidx_7b <= EHT_MAX_3x996_484_RU_INDX) {
		ru_type = D11BE_MRU3x996_484_TYPE;
#endif /* WL11BE */
	} else {
		ASSERT(0);
		ru_type = D11_RU_MRU_UNKNOWN_TYPE;
	}

	return (ru_type);
} /* wf_ofdma_ruidx_to_ru_type */

/**
 * returns Number of Data Subcarriers for HE and EHT per RU type
 *
 * @param	ru_type	HE/EHT RU tone type
 *
 * @return	Nsd	Number of Data Subcarriers
 */
uint32
wf_ru_type_to_nsd(enum ru_type_e ru_type)
{
	const uint32 ru_type_to_nsd[] = {
		24,	/* HE/EHT_RU_26_TONE */
		48,	/* HE/EHT_RU_52_TONE */
		102,	/* HE/EHT_RU_106_TONE */
		234,	/* HE/EHT_RU_242_TONE */
		468,	/* HE/EHT_RU_484_TONE */
		980,	/* HE/EHT_RU_996_TONE */
		1960,	/* HE/EHT_RU_2x996_TONE */
#if WL11BE
		3920,	/* EHT_RU_4x996_TONE */
		72,	/* EHT_MRU_52_26_TONE */
		126,	/* EHT_MRU_106_26_TONE */
		702,	/* EHT_MRU_484_242_TONE */
		1448,	/* EHT_MRU_996_484_TONE */
		1682,	/* EHT_MRU_996_484_242_TONE */
		2428,	/* EHT_MRU_2x996_484_TONE */
		2940,	/* EHT_MRU_3x996_TONE */
		3408,	/* EHT_MRU_3x996_484_TONE */
#endif
	};
	uint32 nsd = 0;

	if (ru_type < ARRAYSIZE(ru_type_to_nsd)) {
		nsd = ru_type_to_nsd[ru_type];
	} else {
		ASSERT(0);
	}

	return nsd;
}

/**
 * returns Total number subcarriers for EHT per RU type
 *
 * @param	ru_type	EHT RU tone type
 *
 * @return	Nst	Total Number of Subcarriers (data subcarriers + pilot subcarriers)
 */
uint32
wf_ru_type_to_tones(enum ru_type_e ru_type)
{
	const uint32 ru_type_to_tone[] = {
		HE_RU_26_TONE,		/* HE/EHT_RU_26_TONE */
		HE_RU_52_TONE,		/* HE/EHT_RU_52_TONE */
		HE_RU_106_TONE,		/* HE/EHT_RU_106_TONE */
		HE_RU_242_TONE,		/* HE/EHT_RU_242_TONE */
		HE_RU_484_TONE,		/* HE/EHT_RU_484_TONE */
		HE_RU_996_TONE,		/* HE/EHT_RU_996_TONE */
		HE_RU_2x996_TONE,	/* HE/EHT_RU_2x996_TONE */
#if WL11BE
		EHT_RU_4x996_TONE,	/* EHT_RU_4x996_TONE */
		EHT_MRU_52_26_TONE,	/* EHT_MRU_52_26_TONE */
		EHT_MRU_106_26_TONE,	/* EHT_MRU_106_26_TONE */
		EHT_MRU_484_242_TONE,	/* EHT_MRU_484_242_TONE */
		EHT_MRU_996_484_TONE,	/* EHT_MRU_996_484_TONE */
		EHT_MRU_996_484_242_TONE, /* EHT_MRU_996_484_242_TONE */
		EHT_MRU_2x996_484_TONE,	/* EHT_MRU_2x996_484_TONE */
		EHT_MRU_3x996_TONE,	/* EHT_MRU_3x996_TONE */
		EHT_MRU_3x996_484_TONE	/* EHT_MRU_3x996_484_TONE */
#endif
	};
	uint32 tone = 0;

	if (ru_type < ARRAYSIZE(ru_type_to_tone)) {
		tone = ru_type_to_tone[ru_type];
	} else {
		ASSERT(0);
	}

	return tone;
}
