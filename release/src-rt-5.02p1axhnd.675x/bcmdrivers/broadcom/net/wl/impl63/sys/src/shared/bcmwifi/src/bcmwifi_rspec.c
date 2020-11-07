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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bcmwifi_rspec.c 787915 2020-06-16 09:14:08Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <d11.h>
#include <802.11ax.h>

#include <bcmwifi_rspec.h>
#include <bcmwifi_rates.h>
#include <bcmutils.h>

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
	uint mcs = RSPEC_HE_MCS(rspec);
	uint nss = RSPEC_HE_NSS(rspec);
	bool dcm = (rspec & WL_RSPEC_DCM) != 0;
	uint bw =  RSPEC_BW(rspec) >> WL_RSPEC_BW_SHIFT;
	uint gi =  RSPEC_HE_LTF_GI(rspec);

	if (mcs <= WLC_MAX_HE_MCS && nss != 0 && nss <= 8) {
		return wf_he_mcs_to_rate(mcs, nss, bw, gi, dcm);
	}
#ifdef BCMDBG
	printf("%s: rspec %x, mcs %u, nss %u\n", __FUNCTION__, rspec, mcs, nss);
#endif // endif
	ASSERT(mcs <= WLC_MAX_HE_MCS);
	ASSERT(nss != 0 && nss <= 8);
	return 0;
} /* wf_he_rspec_to_rate */

uint
wf_he_rspec_ru_type_to_rate(ratespec_t rspec, ru_type_t ru_type)
{
	uint mcs = RSPEC_HE_MCS(rspec);
	uint nss = RSPEC_HE_NSS(rspec);
	bool dcm = (rspec & WL_RSPEC_DCM) != 0;
	uint gi =  RSPEC_HE_LTF_GI(rspec);
	uint nsd;

	ASSERT(mcs <= WLC_MAX_HE_MCS);
	ASSERT(nss != 0 && nss <= 8);

	nsd = wf_he_ru_type_to_nsd(ru_type);
	if (mcs <= WLC_MAX_HE_MCS && nss != 0 && nss <= 8) {
		return wf_he_mcs_nsd_to_rate(mcs, nss, nsd, gi, dcm);
	}

	return 0;
} /* wf_he_rspec_to_rate */

/** take a well formed ratespec_t arg and return phy rate in [Kbps] units */
uint
wf_rspec_to_rate(ratespec_t rspec)
{
	uint rate = (uint)(-1);
	uint mcs, nss;

	switch (rspec & WL_RSPEC_ENCODING_MASK) {
		case WL_RSPEC_ENCODE_HE:
			rate = wf_he_rspec_to_rate(rspec);
			break;
		case WL_RSPEC_ENCODE_VHT:
			mcs = RSPEC_VHT_MCS(rspec);
			nss = RSPEC_VHT_NSS(rspec);
#ifdef BCMDBG
			if (mcs > WLC_MAX_VHT_MCS || nss == 0 || nss > 8) {
				printf("%s: rspec=%x\n", __FUNCTION__, rspec);
			}
#endif /* BCMDBG */
			ASSERT(mcs <= WLC_MAX_VHT_MCS);
			ASSERT(nss != 0 && nss <= 8);
			rate = wf_mcs_to_rate(mcs, nss,
				RSPEC_BW(rspec), RSPEC_ISSGI(rspec));
			break;
		case WL_RSPEC_ENCODE_HT:
			mcs = RSPEC_HT_MCS(rspec);
#ifdef BCMDBG
			if (mcs > 32 && !IS_PROPRIETARY_11N_MCS(mcs)) {
				printf("%s: rspec=%x\n", __FUNCTION__, rspec);
			}
#endif /* BCMDBG */
			ASSERT(mcs <= 32 || IS_PROPRIETARY_11N_MCS(mcs));
			if (mcs == 32) {
				rate = wf_mcs_to_rate(mcs, 1, WL_RSPEC_BW_40MHZ,
					RSPEC_ISSGI(rspec));
			} else {
#if defined(WLPROPRIETARY_11N_RATES)
				nss = GET_11N_MCS_NSS(mcs);
				mcs = wf_get_single_stream_mcs(mcs);
#else /* this ifdef prevents ROM abandons */
				nss = 1 + (mcs / 8);
				mcs = mcs % 8;
#endif /* WLPROPRIETARY_11N_RATES */
				rate = wf_mcs_to_rate(mcs, nss, RSPEC_BW(rspec),
					RSPEC_ISSGI(rspec));
			}
			break;
		case WL_RSPEC_ENCODE_RATE:	/* Legacy */
			rate = 500 * RSPEC2RATE(rspec);
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
#endif // endif

/**
 * Calculate ratespec from PLCP header.
 *
 * @param ft	Frame type.
 * @param plcp	Pointer to PLCP.
 * @param rspec	Pointer to ratespec storage variable. Only modified if a ratespec was calculated.
 * @return	TRUE if a valid frame type was specified.
 */
bool BCMFASTPATH
wf_plcp_to_rspec(uint16 ft_fmt, uint8 *plcp, ratespec_t *rspec)
{
	uint16 ft;

	ASSERT(plcp != NULL);
	ASSERT(rspec != NULL);

	ft = D11_FT(ft_fmt);

	switch (ft) {
#ifdef WL11AX
		case FT_HE:
			*rspec = wf_he_plcp_to_rspec(plcp, ft_fmt);
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

ratespec_t BCMFASTPATH
wf_he_plcp_to_rspec(uint8 *plcp, uint16 ft_fmt)
{
	uint8 bw, gi;
	ratespec_t rspec;
	uint16 hefmt;

	/* HE plcp - 6 B */
	uint32 plcp0;
	uint16 plcp1;
	uint32 sigb;

	ASSERT(plcp);

	gi = 0;
	bw = 0;
	hefmt = D11_HEFMT(ft_fmt);
	rspec = WL_RSPEC_ENCODE_HE | wf_he_plcp_to_rspec_rate(plcp, ft_fmt);

	switch (hefmt) {
		case HE_FMT_HESU:
		case HE_FMT_HESURT:
			plcp0 = (plcp[2] << 16) | plcp[0];	/* Don't need plcp[1] or plcp[3] */
			plcp1 = plcp[4];			/* Don't need plcp[5] */

			/* GI info comes from CP/LTF */
			gi = (plcp0 & HE_PLCP0_CPLTF_MASK) >> HE_PLCP0_CPLTF_SHIFT;
			bw = ((plcp0 & HE_PLCP0_BW_MASK) >> HE_PLCP0_BW_SHIFT) + 1;

			rspec |= ((plcp1 & HE_PLCP1_TXBF_MASK) ? WL_RSPEC_TXBF : 0);
			rspec |= ((plcp1 & HE_PLCP1_CODING_MASK) ? WL_RSPEC_LDPC : 0);
			rspec |= ((plcp1 & HE_PLCP1_STBC_MASK)? WL_RSPEC_STBC : 0);
			rspec |= ((plcp0 & HE_PLCP0_DCM_MASK)? WL_RSPEC_DCM : 0);
			break;

		case HE_FMT_HEMU:
			sigb = (plcp[9] << 24) | (plcp[8] << 16) | (plcp[7] << 8) | plcp[6];
			plcp0 = (plcp[3] << 24) | (plcp[2] << 16) | plcp[0];
			plcp1 = plcp[4];

			gi = HEMU_PLCP0_CPLTF(plcp0);
			bw = HEMU_PLCP0_BW(plcp0);

			rspec |= ((sigb & HEMU_SIGB_TXBF_MASK) ? WL_RSPEC_TXBF : 0);
			rspec |= ((sigb & HEMU_SIGB_CODING_MASK) ? WL_RSPEC_LDPC : 0);
			rspec |= ((sigb & HEMU_SIGB_DCM_MASK) ? WL_RSPEC_DCM : 0);
			rspec |= ((plcp1 & HEMU_PLCP1_STBC_MASK)? WL_RSPEC_STBC :0);
			break;

		case HE_FMT_HETB:
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

ratespec_t BCMFASTPATH
wf_vht_plcp_to_rspec(uint8 *plcp)
{
	uint vht_sig_a1, vht_sig_a2;
	ratespec_t rspec;

	rspec = WL_RSPEC_ENCODE_VHT | wf_vht_plcp_to_rspec_rate(plcp);

	vht_sig_a1 = plcp[0] | (plcp[1] << 8);
	vht_sig_a2 = plcp[3] | (plcp[4] << 8);

#if ((((VHT_SIGA1_20MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  != WL_RSPEC_BW_20MHZ) || \
	(((VHT_SIGA1_40MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  != WL_RSPEC_BW_40MHZ) || \
	(((VHT_SIGA1_80MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  != WL_RSPEC_BW_80MHZ) || \
	(((VHT_SIGA1_160MHZ_VAL + 1) << WL_RSPEC_BW_SHIFT)  != WL_RSPEC_BW_160MHZ))
#error "VHT SIGA BW mapping to RSPEC BW needs correction"
#endif // endif
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

ru_type_t
wf_he_ruidx_to_ru_type(uint32 ruidx)
{
	uint32 ru_type;

	if (ruidx <= HE_MAX_26_TONE_RU_INDX) {
		ru_type = D11AX_RU26_TYPE;
	} else if (ruidx <= HE_MAX_52_TONE_RU_INDX) {
		ru_type = D11AX_RU52_TYPE;
	} else if (ruidx <= HE_MAX_106_TONE_RU_INDX) {
		ru_type = D11AX_RU106_TYPE;
	} else if (ruidx <= HE_MAX_242_TONE_RU_INDX) {
		ru_type = D11AX_RU242_TYPE;
	} else if (ruidx <= HE_MAX_484_TONE_RU_INDX) {
		ru_type = D11AX_RU484_TYPE;
	} else if (ruidx <= HE_MAX_996_TONE_RU_INDX) {
		ru_type = D11AX_RU996_TYPE;
	} else if (ruidx <= HE_MAX_2x996_TONE_RU_INDX) {
		ru_type = D11AX_RU1992_TYPE;
	} else {
		ru_type = D11AX_RU_MAX_TYPE;
	}

	return (ru_type);
}

uint32
wf_he_ru_type_to_nsd(ru_type_t ru_type)
{
	const uint32 ru_type_to_nsd[] = {
		24,		/* HE_RU_26_TONE */
		48,		/* HE_RU_52_TONE */
		102,	/* HE_RU_106_TONE */
		234,	/* HE_RU_242_TONE */
		468,	/* HE_RU_484_TONE */
		980,	/* HE_RU_996_TONE */
		1960	/* HE_RU_2x996_TONE */
	};
	uint32 nsd = 0;

	if (ru_type < ARRAYSIZE(ru_type_to_nsd)) {
		nsd = ru_type_to_nsd[ru_type];
	}

	return nsd;
}

uint32
wf_he_ru_type_to_tones(ru_type_t ru_type)
{
	const uint32 ru_type_to_tone[] = {
		HE_RU_26_TONE,		/* HE_RU_26_TONE */
		HE_RU_52_TONE,		/* HE_RU_52_TONE */
		HE_RU_106_TONE,		/* HE_RU_106_TONE */
		HE_RU_242_TONE,		/* HE_RU_242_TONE */
		HE_RU_484_TONE,		/* HE_RU_484_TONE */
		HE_RU_996_TONE,		/* HE_RU_996_TONE */
		HE_RU_2x996_TONE	/* HE_RU_2x996_TONE */
	};
	uint32 tone = 0;

	if (ru_type < ARRAYSIZE(ru_type_to_tone)) {
		tone = ru_type_to_tone[ru_type];
	}

	return tone;
}
