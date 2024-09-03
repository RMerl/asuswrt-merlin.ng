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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcmwifi_rates.c 823980 2023-04-17 02:48:52Z $
 */

#include <typedefs.h>
#include <802.11.h>
#include <802.11ax.h>
#ifdef BCMDRIVER
#include <osl.h>
#include <d11_pub.h>
#endif
#include <bcmwifi_rspec.h>
#include <bcmwifi_rates.h>
#include <bcmutils.h>

#ifndef ASSERT
#define ASSERT(exp)
#endif

/* TODO: Consolidate rate utility functions from wlc_rate.c and bcmwifi_monitor.c
 * into here if they're shared by non wl layer as well...
 */

enum dcm_cap_e {
	DCM_CAP_NONE,
	DCM_CAP_HE,     /**< HE DCM capable */
	DCM_CAP_EHT,
	DCM_CAP_EHT_DUP /**< DCM + DUP */
};

/** HT/VHT/HE/EHT modulation and coding rate info to calculate rate */
struct ieee_80211_mcs_rate_info {
	uint8 constellation_bits; /* nbpscs */
	uint8 coding_n;		/* coding numerator */
	uint8 coding_d;		/* coding denominator */
	enum dcm_cap_e dcm_cap;
};

/** HT/VHT/HE/EHT modulation and coding rate info to calculate rate */
static const struct ieee_80211_mcs_rate_info wlc_mcs_info[] = {
	{ 1, 1, 2, DCM_CAP_HE },      /* MCS  0: MOD: BPSK,   CR 1/2, dcm capable */
	{ 2, 1, 2, DCM_CAP_HE },      /* MCS  1: MOD: QPSK,   CR 1/2, dcm capable */
	{ 2, 3, 4, DCM_CAP_NONE },    /* MCS  2: MOD: QPSK,   CR 3/4, NOT dcm capable */
	{ 4, 1, 2, DCM_CAP_HE },      /* MCS  3: MOD: 16QAM,  CR 1/2, dcm capable */
	{ 4, 3, 4, DCM_CAP_HE },      /* MCS  4: MOD: 16QAM,  CR 3/4, dcm capable */
	{ 6, 2, 3, DCM_CAP_NONE },    /* MCS  5: MOD: 64QAM,  CR 2/3, NOT dcm capable */
	{ 6, 3, 4, DCM_CAP_NONE },    /* MCS  6: MOD: 64QAM,  CR 3/4, NOT dcm capable */
	{ 6, 5, 6, DCM_CAP_NONE },    /* MCS  7: MOD: 64QAM,  CR 5/6, NOT dcm capable */
	{ 8, 3, 4, DCM_CAP_NONE },    /* MCS  8: MOD: 256QAM, CR 3/4, NOT dcm capable */
	{ 8, 5, 6, DCM_CAP_NONE },    /* MCS  9: MOD: 256QAM, CR 5/6, NOT dcm capable */
	{ 10, 3, 4, DCM_CAP_NONE },   /* MCS 10: MOD: 1024QAM, CR 3/4, NOT dcm capable */
	{ 10, 5, 6, DCM_CAP_NONE },   /* MCS 11: MOD: 1024QAM, CR 5/6, NOT dcm capable */
	{ 12, 3, 4, DCM_CAP_NONE },   /* MCS 12: MOD: 4096QAM, CR 3/4, NOT dcm capable */
	{ 12, 5, 6, DCM_CAP_NONE },   /* MCS 13: MOD: 4096QAM, CR 5/6, NOT dcm capable */
	{ 1, 1, 2, DCM_CAP_EHT_DUP }, /* MCS 14: MOD: BPSK, CR 1/2, EHT DCM+DUP */
	{ 1, 1, 2, DCM_CAP_EHT },     /* MCS 15: MOD: BPSK, CR 1/2, EHT DCM */
};

/**
 * Number of data subcarriers for a full bandwidth non-OFDMA HE/EHT PPDU,
 * .11be D1.1 Table 36-19. Type of the index is enum wl_bw_e.
 */
static const uint wlc_he_eht_fbw_nsd[] = {
	0,	/* Undefined */
	234,	/* (WL_RSPEC_BW_20MHZ >> WL_RSPEC_BW_SHIFT) = BW_20MHZ */
	468,	/* BW_40MHZ */
	980,	/* BW_80MHZ */
	1960,	/* BW_160MHZ */
	3920	/* BW_320MHZ */
};

/* sym_len = 12.8 us. For calculation purpose, *10 */
#define HE_SYM_LEN_FACTOR		(128)

/* GI values = 0.8 , 1.6 or 3.2 us. For calculation purpose, *10 */
#define HE_GI_800us_FACTOR		(8)
#define HE_GI_1600us_FACTOR		(16)
#define HE_GI_3200us_FACTOR		(32)

#define ksps		250 /* kilo symbols per sec, 4 us sym */

/* below defines number of data carriers per frequency segment for HT, VHT */
#define Nsd_HT_VHT_20MHZ	52
#define Nsd_HT_VHT_40MHZ	108
#define Nsd_VHT_80MHZ		234
#define Nsd_VHT_160MHZ		468

/** @return   The factor by which dcm (for robustness) decreases tput */
static uint
wf_dcm_factor(bool he_dcm, const struct ieee_80211_mcs_rate_info *ri)
{
	switch (ri->dcm_cap) {
	case DCM_CAP_NONE:
		return 1;
	case DCM_CAP_HE:
		if (he_dcm) {
			return 2;
		}
		return 1;
	case DCM_CAP_EHT:
		return 2;
	case DCM_CAP_EHT_DUP:
		return 4;
	}

	ASSERT(0);

	return 1;
}

/**
 * This function unifies common code called by multiple functions.
 *
 * @param[in] nsd  The number of data subcarriers.
 * @param[in] nss  Must be > 0
 *
 * @return         The number of uncoded data bits for one symbol per (mcs/nss/nsd/he_dcm)
 *                 combination.
 */
static uint
wf_he_eht_uncoded_data_bits_per_sym(uint mcs, uint nss, uint nsd, bool he_dcm)
{
	const struct ieee_80211_mcs_rate_info *ri;
	uint ret;

	ASSERT(mcs < ARRAYSIZE(wlc_mcs_info));
	ri = &wlc_mcs_info[mcs];

	ret = nsd * ri->constellation_bits;
	ret *= ri->coding_n; /* adjust for the coding rate nominator */
	ret *= nss; /* adjust for the number of spatial streams */
	ret /= ri->coding_d; /* adjust for the coding rate denominator */
	ret /= wf_dcm_factor(he_dcm, ri); /* adjust for dcm */

	return ret;
}

/**
 * @param[in] nss  Must be > 0
 *
 * @return number of data bits per symbol
 */
uint
wf_he_mcs_to_Ndbps(uint mcs, uint nss, enum wl_rspec_bw_e rspec_bw, bool dcm)
{
	uint Nsd = 0;
	uint Ndbps;
	enum wl_bw_e bw = (rspec_bw >> WL_RSPEC_BW_SHIFT);

	ASSERT(bw > 0 && bw < ARRAYSIZE(wlc_he_eht_fbw_nsd));
	/* find the number of complex numbers per symbol */
	if (bw < ARRAYSIZE(wlc_he_eht_fbw_nsd)) {
		Nsd = wlc_he_eht_fbw_nsd[bw];
	}

	Ndbps = wf_he_eht_uncoded_data_bits_per_sym(mcs, nss, Nsd, dcm);

	return Ndbps;
}

uint
wf_he_mcs_nsd_to_kbps(uint mcs, uint nss, uint nsd, enum wl_rspec_he_gi_e gi, bool dcm)
{
	uint rate, rate_deno;
	uint Ndbps;

	Ndbps = wf_he_eht_uncoded_data_bits_per_sym(mcs, nss, nsd, dcm);
	/* add sym len factor */
	rate_deno = HE_SYM_LEN_FACTOR;

	/* get GI for denominator */
	if (HE_IS_GI_3_2us(gi)) {
		rate_deno += HE_GI_3200us_FACTOR;
	} else if (HE_IS_GI_1_6us(gi)) {
		rate_deno += HE_GI_1600us_FACTOR;
	} else {
		/* assuming HE_GI_0_8us */
		rate_deno += HE_GI_800us_FACTOR;
	}

	/* as per above formula */
	rate = 1000 * Ndbps;	/* factor of 10. *100 to accommodate 2 places */
	rate /= rate_deno;
	rate *= 10; /* *100 was already done above. Splitting is done to avoid overflow. */

	return rate;
}

uint
wf_he_mcs_to_kbps(uint mcs, uint nss, enum wl_bw_e bw, enum wl_rspec_he_gi_e gi, bool dcm)
{
	uint nsd = 0;

	STATIC_ASSERT(BW_20MHZ == (WL_RSPEC_BW_20MHZ >> WL_RSPEC_BW_SHIFT) &&
	              BW_40MHZ == (WL_RSPEC_BW_40MHZ >> WL_RSPEC_BW_SHIFT) &&
	              BW_80MHZ == (WL_RSPEC_BW_80MHZ >> WL_RSPEC_BW_SHIFT) &&
	              BW_160MHZ == (WL_RSPEC_BW_160MHZ >> WL_RSPEC_BW_SHIFT) &&
	              BW_320MHZ == (WL_RSPEC_BW_320MHZ >> WL_RSPEC_BW_SHIFT));

	if (bw < ARRAYSIZE(wlc_he_eht_fbw_nsd)) {
		nsd = wlc_he_eht_fbw_nsd[bw];
	}

	return wf_he_mcs_nsd_to_kbps(mcs, nss, nsd, gi, dcm);
}

uint
wf_ht_vht_mcs_to_Ndbps(uint mcs, uint nss, enum wl_rspec_bw_e bw)
{
	uint Nsd = 0;
	uint Ndbps;

	/* This calculation works for 11n HT if the HT mcs values are decomposed into a
	 * base MCS = MCS % 8, and Nss = 1 + MCS / 8.
	 * That is, HT MCS 23 is a base MCS = 7, Nss = 3
	 */

	/* find the number of complex numbers per symbol */
	if (bw == WL_RSPEC_BW_20MHZ) {
		Nsd = Nsd_HT_VHT_20MHZ;
	} else if (bw == WL_RSPEC_BW_40MHZ) {
		Nsd = Nsd_HT_VHT_40MHZ;
	} else if (bw == WL_RSPEC_BW_80MHZ) {
		Nsd = Nsd_VHT_80MHZ;
	} else if (bw == WL_RSPEC_BW_160MHZ) {
		Nsd = Nsd_VHT_160MHZ;
	} else if (bw == WL_RSPEC_BW_320MHZ) {
		ASSERT(0); /* this function is not meant for EHT (nor HE) */
	}

	ASSERT(mcs < ARRAYSIZE(wlc_mcs_info));

	/* multiply number of spatial streams,
	 * bits per number from the constellation,
	 * and coding rate quotient
	 */
	Ndbps = Nsd * nss *
		wlc_mcs_info[mcs].coding_n * wlc_mcs_info[mcs].constellation_bits;

	/* adjust for the coding rate denominator */
	Ndbps /= wlc_mcs_info[mcs].coding_d;

	return Ndbps;
}

/**
 * For HT or VHT, returns the rate in [Kbps] units for a caller supplied MCS/bandwidth/Nss/Sgi
 * combination.
 *
 * @param[in] mcs    A *single* spatial stream MCS (11n or 11ac)
 */
uint
wf_ht_vht_mcs_to_kbps(uint mcs, uint nss, enum wl_rspec_bw_e bw, int sgi)
{
	uint rate;

	if (mcs == 32) {
		/* just return fixed values for mcs32 instead of trying to parameterize */
		rate = (sgi == 0) ? 6000 : 6778;
	} else if (mcs <= WLC_MAX_VHT_MCS) {
		const struct ieee_80211_mcs_rate_info *ri;

		/* This calculation works for 11n HT, 11ac VHT and 11ax HE if the HT mcs values
		 * are decomposed into a base MCS = MCS % 8, and Nss = 1 + MCS / 8.
		 * That is, HT MCS 23 is a base MCS = 7, Nss = 3
		 */

		/* find the number of complex numbers per symbol */
		if (RSPEC_IS20MHZ(bw)) {
			rate = Nsd_HT_VHT_20MHZ;
		} else if (RSPEC_IS40MHZ(bw)) {
			rate = Nsd_HT_VHT_40MHZ;
		} else if (bw == WL_RSPEC_BW_80MHZ) {
			rate = Nsd_VHT_80MHZ;
		} else if (bw == WL_RSPEC_BW_160MHZ) {
			rate = Nsd_VHT_160MHZ;
		} else if (bw == WL_RSPEC_BW_320MHZ) {
			ASSERT(0); /* this function does not handle EHT (nor HE) */
			rate = 0;
		} else {
			rate = 0;
		}

		ri = &wlc_mcs_info[mcs];

		/* multiply by bits per number from the constellation in use */
		rate = rate * ri->constellation_bits;

		/* adjust for the number of spatial streams */
		rate = rate * nss;

		/* adjust for the coding rate quotient */
		rate = (rate * ri->coding_n) / ri->coding_d;

		/* multiply by Kilo symbols per sec to get Kbps */
		rate = rate * ksps;

		/* adjust the symbols per sec for SGI
		 * symbol duration is 4 us without SGI, and 3.6 us with SGI,
		 * so ratio is 10 / 9
		 */
		if (sgi) {
			/* add 4 for rounding of division by 9 */
			rate = ((rate * 10) + 4) / 9;
		}
	} else {
		rate = 0;
	}

	return rate;
} /* wf_ht_vht_mcs_to_kbps */

uint8
wf_vht_plcp_to_rspec_rate(uint8 *plcp)
{
	uint8 rate, nss, gid;

	uint32 plcp0 = (plcp[1] << 8) | plcp[0];	/* Don't need plcp[2] */

	gid = (plcp0 & VHT_SIGA1_GID_MASK) >> VHT_SIGA1_GID_SHIFT;
	if (gid > VHT_SIGA1_GID_TO_AP && gid < VHT_SIGA1_GID_NOT_TO_AP) {
		/* for MU packet we hacked Signal Tail field in VHT-SIG-A2 to save nss and mcs,
		 * copy from murate in d11 rx header.
		 * nss = bit 18:19 (for 11ac 2 bits to indicate maximum 4 nss)
		 * mcs = 20:23
		 */
		rate = (plcp[5] & 0xF0) >> 4;
		nss = ((plcp[5] & 0x0C) >> 2) + 1;
	} else {
		rate = (plcp[3] >> VHT_SIGA2_MCS_SHIFT);
		nss = ((plcp0 & VHT_SIGA1_NSTS_SHIFT_MASK_USER0) >>
			VHT_SIGA1_NSTS_SHIFT) + 1;
		if (plcp0 & VHT_SIGA1_STBC) {
			nss = nss / 2;
			nss = MAX(nss, 1);
		}
	}

	/* NSS is guaranteed to be > 0 */
	return rate | (nss << WL_RSPEC_GE_VHT_NSS_SHIFT);
}

#ifdef BCMDRIVER
uint8 BCMFASTPATH
wf_he_plcp_to_rspec_rate(uint8 *plcp, uint8 ft, uint8 hefmt)
{
	uint8 mcs, nsts, nss;
	uint32 plcp0;
	uint16 plcp1;

	ASSERT(plcp);

	switch (hefmt) {
		case HE_EHT_FMT_SU:
		case HE_EHT_FMT_SUER:
			plcp0 = (plcp[3] << 24) | (plcp[2] << 16) | plcp[0];
			plcp1 = plcp[4];

			mcs  = (plcp0 & HE_PLCP0_MCS_MASK) >> HE_PLCP0_MCS_SHIFT;
			nsts = ((plcp0 & HE_PLCP0_NSTS_MASK) >> HE_PLCP0_NSTS_SHIFT) + 1;

			nss = nsts;
			if (plcp1 & HE_PLCP1_STBC_MASK) {
				/* HE STBC only allowed with 1 ss and 2 sts (28.3.11.10) */
				nss = 1;
			}
			break;
		case HE_EHT_FMT_MU:
			/* plcp0 is sigb */
			plcp0 = (plcp[9] << 24) | (plcp[8] << 16) | (plcp[7] << 8) | plcp[6];
			mcs = (plcp0 & HEMU_SIGB_MCS_MASK) >> HEMU_SIGB_MCS_SHIFT;
			nsts = ((plcp0 & HEMU_SIGB_NSTS_MASK) >> HEMU_SIGB_NSTS_SHIFT) + 1;
			nss = nsts;
			break;
		case HE_EHT_FMT_TB:
			plcp1 = (plcp[7] << 8) | plcp[6];
			mcs = ULRTINFO_MCS(plcp1);
			nsts = ULRTINFO_NSS(plcp1);
			nss = nsts;
			break;
		default:
			mcs = 0;
			nss = nsts = 1;
#ifdef BCMDBG
			printf("%s: HE format %d is not supported\n", __FUNCTION__, hefmt);
#endif /* BCMDBG */
			ASSERT(0);
			break;
	}

	ASSERT(mcs <= WLC_MAX_HE_MCS);
	ASSERT(nss <= 8);

	/* NSS is guaranteed to be > 0 */
	return mcs | (nss << WL_RSPEC_GE_VHT_NSS_SHIFT);
}

#define EHT_SIG_UINFO_MCS_MASK		0x00F00000
#define EHT_SIG_UINFO_MCS_SHIFT		20
#define EHT_SIG_UINFO_NSS_MASK		0x1E000000
#define EHT_SIG_UINFO_NSS_SHIFT		25
/**
 * Function returning MCS/NSS info (fit for ratespec), derived from EHT RX(!) PLCP header.
 *
 * @param plcp		Pointer to RX PLCP, points to EHT-USIG field.
 * @param ft		Frame type.
 * @param ehtfmt	EHT specific PPDU format.
 * @return		mcs and nss info, formatted for RSPEC as computed from PLCP
 */
uint8
wf_eht_rx_plcp_to_rspec_rate(uint8 *plcp, uint8 ft, uint8 ehtfmt)
{
	uint8 mcs, nss;
	uint32 eht_sig;
	uint16 plcp1;

	ASSERT(plcp);
	/* Skip EHT-USIG */
	plcp += 6;

	switch (ehtfmt) {
		case HE_EHT_FMT_SU:
		case HE_EHT_FMT_SUER:
		case HE_EHT_FMT_MU:
			/* pick up EHT_SIG, ignore lower part (= SR/GI/LTF and STA_ID) */
			eht_sig = (plcp[3] << 24) | (plcp[2] << 16);
			mcs  = (eht_sig & EHT_SIG_UINFO_MCS_MASK) >> EHT_SIG_UINFO_MCS_SHIFT;
			nss = ((eht_sig & EHT_SIG_UINFO_NSS_MASK) >> EHT_SIG_UINFO_NSS_SHIFT) + 1;
			break;
		case HE_EHT_FMT_TB:
			plcp1 = (plcp[1] << 8) | plcp[0];
			mcs = ULRTINFO_MCS(plcp1);
			nss = ULRTINFO_NSS(plcp1);
			break;
		default:
#ifdef BCMDBG
			printf("%s: EHT format %d is not supported\n", __FUNCTION__, ehtfmt);
#endif /* BCMDBG */
			ASSERT(0); /* fallthrough */
			mcs = 0;
			nss = 1;
			break;
	}

	ASSERT(mcs <= WLC_MAX_EHT_MCS);
	ASSERT(nss <= 16);

	/* NSS is guaranteed to be > 0 */
	return mcs | (nss << WL_RSPEC_GE_VHT_NSS_SHIFT);
}
#endif /* BCMDRIVER */

/* ============================================ */
/* ============================================ */

/**
 * Some functions require a single stream MCS as an input parameter. Given an MCS, this function
 * returns the single spatial stream MCS equivalent.
 */
uint8
wf_get_single_stream_mcs(uint mcs)
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

/* ============================================ */
/* ============================================ */

const uint8 plcp_ofdm_rate_tbl[] = {
	DOT11_RATE_48M, /* 8: 48Mbps */
	DOT11_RATE_24M, /* 9: 24Mbps */
	DOT11_RATE_12M, /* A: 12Mbps */
	DOT11_RATE_6M,  /* B:  6Mbps */
	DOT11_RATE_54M, /* C: 54Mbps */
	DOT11_RATE_36M, /* D: 36Mbps */
	DOT11_RATE_18M, /* E: 18Mbps */
	DOT11_RATE_9M   /* F:  9Mbps */
};

/* sym_len = 12.8 us. For calculation purpose, *10 */
#define EHT_SYM_LEN_FACTOR		(128)

/* GI values = 0.8 , 1.6 or 3.2 us. For calculation purpose, *10 */
#define EHT_GI_800us_FACTOR		(8)
#define EHT_GI_1600us_FACTOR		(16)
#define EHT_GI_3200us_FACTOR		(32)

#define WL_RSPEC_BW_ENC2BWI(bw)	((enum wl_bw_e)(((bw) & WL_RSPEC_BW_MASK) >> WL_RSPEC_BW_SHIFT))

/**
 * returns number of data subcarriers for full bandwidth RU and corrects for DCM
 * (Dual Carrier Modulation) and DUP (DUPlicate) modulation schemes.
 *
 * @param	mcs	Modulation and coding scheme index
 *		nss	Number of Spatial Streams
 *		bw	Bandwidth index, e.g. (WL_RSPEC_BW_40MHZ >> WL_RSPEC_BW_SHIFT)
 *
 * @return	nsd	Number of Data Subcarriers
 */
static uint
wf_eht_nsd(uint mcs, uint nss, enum wl_bw_e bw)
{
	uint nsd;

	ASSERT(bw >= BW_20MHZ && bw < ARRAYSIZE(wlc_he_eht_fbw_nsd));
	nsd = wlc_he_eht_fbw_nsd[bw];

	/* correct rate for bpsk-dcm-dup (MCS14) and bpsk-dcm (MCS15)
	 * BPSK-DCM and BPSK-DCM-DUP are supported only with Nss=1
	 */
	if (nss == WLC_MIN_NSS) {
		if (mcs == 14) {
			 /* for DCM-DUP the rate is effectively divided by four, except for 80MHz */
			if (bw <= BW_80MHZ) {
				/* for 80MHz, the NSD happens to be equal to the normal 20MHz NSD */
				nsd = wlc_he_eht_fbw_nsd[BW_20MHZ];
			} else {
				nsd /= 4;
			}
			/* MCS14 / DCM-DUP supported in BW 80, 160 and 320MHz only */
			ASSERT(bw >= BW_80MHZ);
		} else if (mcs == 15) {
			/* for DCM the rate is effectively divide by two */
			nsd /= 2;
		}
	} else if (mcs >= 14) {
		/* DCM/DUP support 1 spatial stream only */
		ASSERT(0);
	}

	return nsd;
}

/**
 * Calculates the number of uncoded bits per Symbol
 *
 * @param	mcs	Modulation and Coding Scheme index
 *		nss	Number of Spatial Streams
 *		bw      e.g. WL_RSPEC_BW_80MHZ
 *		nsd	Number of Data Subcarriers
 *
 * @return	ndbps	Number of uncoded bits per symbol
 */
uint
wf_eht_mcs_to_Ndbps(uint mcs, uint nss, enum wl_rspec_bw_e rspec_bw)
{
	uint nsd; /* number of data subcarriers for full band width RU */
	uint ndbps;
	enum wl_bw_e bw = (rspec_bw >> WL_RSPEC_BW_SHIFT);
	const struct ieee_80211_mcs_rate_info *ri;

	ASSERT(mcs < ARRAYSIZE(wlc_mcs_info));
	ASSERT(bw > 0 && bw < ARRAYSIZE(wlc_he_eht_fbw_nsd));

	ri = &wlc_mcs_info[mcs];
	nsd = wf_eht_nsd(mcs, nss, bw); /* takes DCM/DUP into account */

	/* multiply number of spatial streams,
	 * number of data subcarriers,
	 * number of coded bits per subcarrier per spatial stream from the constellation,
	 * adjust for the coding rate quotient.
	 */
	ndbps = nss * nsd * ri->constellation_bits * ri->coding_n / ri->coding_d;

	return ndbps;
}

/**
 * Converts Ndbps to rate in [Kbps] using a fixed Symbol length and Guard Interval in use
 *
 * @param	ndbps	Number of bits per Symbol
 *		gi	Guard Interval
 *
 * @return	rate	in [Kbps]
 */
static uint
wf_eht_ndbps_to_kbps(uint ndbps, enum wl_rspec_eht_gi_e gi)
{
	uint rate;
	uint rate_deno;

	/* add Symbol length for denominator */
	rate_deno = EHT_SYM_LEN_FACTOR;

	/* get GI for denominator */
	if (gi == WL_RSPEC_EHT_4x_LTF_GI_3_2us) {
		rate_deno += EHT_GI_3200us_FACTOR;
	} else if (gi == WL_RSPEC_EHT_2x_LTF_GI_1_6us) {
		rate_deno += EHT_GI_1600us_FACTOR;
	} else {
		/* assuming EHT_GI_0_8us */
		rate_deno += EHT_GI_800us_FACTOR;
	}

	rate = ndbps * 1000;	/* to get units of [Kbps] */
	rate /= rate_deno;
	rate *= 10;		/* to correct for (SYM LEN and GI) *10 in rate_deno */

	return rate;
}

/**
 * Calculates the rate in [Kbps]
 *
 * @param	mcs	Modulation and Coding Scheme index
 *		nss	Number of Spatial Streams
 *		bw	Bandwidth index: 1=20MHz, 2=40MHz, ..
 *		gi	Guard Interval
 *
 * @return	rate	in [Kbps]
 */
uint
wf_eht_mcs_to_kbps(uint mcs, uint nss, enum wl_bw_e bw, enum wl_rspec_eht_gi_e gi)
{
	uint ndbps;

	/* number of data bits per symbol */
	ndbps = wf_eht_mcs_to_Ndbps(mcs, nss, bw << WL_RSPEC_BW_SHIFT);

	/* return rate in [Kbps] */
	return wf_eht_ndbps_to_kbps(ndbps, gi);
}

/**
 * Calculates the rate in [Kbps]
 *
 * @param	mcs	Modulation and Coding Scheme index
 *		nss	Number of Spatial Streams
 *		nsd	Number of Data Subcarriers
 *		gi	Guard Interval
 *
 * @return	rate	in [Kbps]
 */
uint
wf_eht_mcs_nsd_to_kbps(uint mcs, uint nss, uint nsd, enum wl_rspec_eht_gi_e gi)
{
	uint rate, rate_deno;
	uint Ndbps;
	bool he_dcm = FALSE;

	Ndbps = wf_he_eht_uncoded_data_bits_per_sym(mcs, nss, nsd, he_dcm);
	/* add sym len factor */
	rate_deno = EHT_SYM_LEN_FACTOR;

	/* get GI for denominator */
	if (EHT_IS_GI_3_2us(gi)) {
		rate_deno += EHT_GI_3200us_FACTOR;
	} else if (EHT_IS_GI_1_6us(gi)) {
		rate_deno += EHT_GI_1600us_FACTOR;
	} else {
		/* assuming EHT_GI_0_8us */
		rate_deno += EHT_GI_800us_FACTOR;
	}

	/* as per above formula */
	rate = 1000 * Ndbps;	/* factor of 10. *100 to accommodate 2 places */
	rate /= rate_deno;
	rate *= 10; /* *100 was already done above. Splitting is done to avoid overflow. */

	return rate;
}

/**
 * Retrieves the maximum mcs_code in a caller supplied EHT mcs_map
 *
 * @param[in] mcs_map    EHT mcs map in 20MHzonlySTA format
 * @param[in] nss        Number of spatial streams for which mcs_code is determined, must be > 0
 * @param[in] tx         TRUE if mcs_code for transmit needs to be returned
 *
 * @return               enum wlc_eht_mcs_code_e
 */
enum wlc_eht_mcs_code_e
wf_eht_mcs_map_get_max_mcs_code(uint32 mcs_map, uint nss, bool tx)
{
	enum wlc_eht_mcs_code_e ret = EHT_MCS_CODE_NONE;
	uint mcs_code_idx;
	uint max_nss;

	for (mcs_code_idx = EHT_MCS_CODE_0_7; mcs_code_idx <= EHT_MCS_CODE_0_13; mcs_code_idx++) {
		max_nss = WLC_EHT_RS_GET_NSS(mcs_map, tx, mcs_code_idx);
		if (max_nss >= nss) {
			ret = mcs_code_idx;
		}
	}

	return ret;
}
