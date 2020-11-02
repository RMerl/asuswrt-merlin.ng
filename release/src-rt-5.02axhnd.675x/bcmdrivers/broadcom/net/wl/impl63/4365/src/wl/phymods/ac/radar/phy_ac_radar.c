/*
 * ACPHY RadarDetect module implementation
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
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_radar.h"
#include "phy_radar_shared.h"
#include <phy_ac.h>
#include <phy_ac_radar.h>

#include <phy_utils_reg.h>

#include <wlc_phyreg_ac.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_radar_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_radar_info_t *ri;
};

/* local functions */
static int phy_ac_radar_init(phy_type_radar_ctx_t *ctx, bool on);
static int phy_ac_radar_run(phy_type_radar_ctx_t *ctx, int PLL_idx, int BW80_80_mode);
static void phy_radar_init_st(phy_info_t *pi, phy_radar_st_t *st);

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_radar_info_t *
BCMATTACHFN(phy_ac_radar_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_radar_info_t *ri)
{
	phy_ac_radar_info_t *info;
	phy_type_radar_fns_t fns;
	phy_radar_st_t *st;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_radar_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ri = ri;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_ac_radar_init;
	fns.run = phy_ac_radar_run;
	fns.ctx = info;

	if (phy_radar_register_impl(ri, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_radar_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* init radar states */
	st = phy_radar_get_st(ri);
	ASSERT(st != NULL);

	phy_radar_init_st(pi, st);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_radar_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_radar_unregister_impl)(phy_ac_radar_info_t *info)
{
	phy_info_t *pi;
	phy_radar_info_t *ri;

	ASSERT(info);
	pi = info->pi;
	ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_radar_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_ac_radar_info_t));
}

//th0_bw20_lo, th1_bw20_lo, th0_bw40_lo, th1_bw40_lo, th0_bw80_lo, th1_bw80_lo
//th0_bw20_hi, th1_bw20_hi, th0_bw40_hi, th1_bw40_hi, th0_bw80_hi, th1_bw80_hi
//th0_bw160_lo, th1_bw160_lo, th0_bw160_hi, th1_bw160_hi,
static const wl_radar_thr_t BCMATTACHDATA(wlc_phy_radar_thresh_acphy_2cores) = {
	WL_RADAR_THR_VERSION,
	//0x6b8, 0x30, 0x6b8, 0x30, 0x6b8, 0x30, 0x6b0, 0x30, 0x6b0, 0x30, 0x6b0, 0x30
	//0x6c0, 0x30, 0x6c0, 0x30, 0x6c0, 0x30, 0x6b8, 0x30, 0x6b8, 0x30, 0x6c4, 0x30 // FCC
	//0x6b8, 0x30, 0x6c0, 0x30, 0x6c0, 0x30, 0x6b8, 0x30, 0x6b8, 0x30, 0x6c4, 0x30 // ETSI
	0x6c8, 0x30, 0x6c0, 0x30, 0x6c8, 0x30, 0x6d0, 0x30, 0x6d0, 0x30, 0x6d0, 0x30,
	0x6c8, 0x30, 0x6d0, 0x30,
};

static const wl_radar_thr_t BCMATTACHDATA(wlc_phy_radar_thresh_acphy_2cores_b1) = {
	WL_RADAR_THR_VERSION,
	0x6ac, 0x30, 0x6ac, 0x30, 0x6b4, 0x30, 0x6b4, 0x30, 0x6b4, 0x30, 0x6b4, 0x30,
	0x6b4, 0x30, 0x6b4, 0x30,
};

static const wl_radar_thr_t BCMATTACHDATA(wlc_phy_radar_thresh_acphy_2cores_lesi) = {
	WL_RADAR_THR_VERSION,
	0x6a8, 0x20, 0x6ac, 0x20, 0x6b4, 0x30, 0x6ac, 0x20, 0x6b4, 0x20, 0x6b4, 0x30,
	0x6b4, 0x30, 0x6b4, 0x30,
};

static const wl_radar_thr_t BCMATTACHDATA(wlc_phy_radar_thresh_acphy_1core) = {
	WL_RADAR_THR_VERSION,
	0x6b8, 0x30, 0x6b8, 0x30, 0x6b8, 0x30, 0x6ac, 0x30, 0x6ac, 0x30, 0x6b0, 0x30,
	0x6b8, 0x30, 0x6b0, 0x30,
};

static const wl_radar_thr_t BCMATTACHDATA(wlc_phy_radar_thresh_acphy_1core_4350) = {
	WL_RADAR_THR_VERSION,
	0x6a8, 0x30, 0x6a4, 0x30, 0x6a0, 0x30, 0x6ac, 0x30, 0x6a8, 0x30, 0x6a8, 0x30,
	0x6a0, 0x30, 0x6a8, 0x30,
	/* Assume 3dB antenna gain, targeting -64dBm at input of antenna port */
};

static const wl_radar_thr2_t BCMATTACHDATA(wlc_phy_radar_thresh2_acphy) = {
	WL_RADAR_THR_VERSION,
	0x6b4, 0x30, 0x6b4, 0x30, 0x6b4, 0x30, 0x6b4, 0x30, 0x6b4, 0x30, 0x6b8, 0x30, //SC core
	0xa, 0xa, 0xa, 0x258, 0x258, 0x258, 0x258, 0x4, 0xc
	// fc_varth_sb = 10, fc_varth_bin5_sb = 10
	// notradar_enb = 2'b1010, [3]: normal core lp, [2]: normal core non-lp
	// [1]: scan core lp, [0]: scan core non-lp
	// max_notradar_lp = 600, max_notradar = 600
	// max_notradar_lp_sc = 600, max_notradar_sc = 600
	// highpow_war_enb = 2'b0110, [0]: highpow WAR, [1]: chirp criterion
	// [2-3]: min_fm_lp of scan core
	// highpow_sp_ratio = 5
};

static void
BCMATTACHFN(phy_radar_init_st)(phy_info_t *pi, phy_radar_st_t *st)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* 20Mhz channel radar thresholds */
	st->rparams.radar_thrs = (GET_RDR_NANTENNAS(pi) == 1)
							? wlc_phy_radar_thresh_acphy_1core
							: wlc_phy_radar_thresh_acphy_2cores;
	st->rparams.radar_thrs2 = wlc_phy_radar_thresh2_acphy;
	if ((ACMINORREV_2(pi) && ACMAJORREV_32(pi->pubpi.phy_rev)) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) {
		st->rparams.radar_thrs = (GET_RDR_NANTENNAS(pi) == 1)
							? wlc_phy_radar_thresh_acphy_1core
							: wlc_phy_radar_thresh_acphy_2cores_b1;
		if (pi->u.pi_acphy->lesi == 1)
			st->rparams.radar_thrs = wlc_phy_radar_thresh_acphy_2cores_lesi;

	}

	if (ACMAJORREV_2(pi->pubpi.phy_rev))
		st->rparams.radar_thrs = wlc_phy_radar_thresh_acphy_1core_4350;

	/* 20Mhz channel radar params */
	st->rparams.min_tint = 3000;  /* 0.15ms (6.67 kHz) */
	st->rparams.max_tint = 120000; /* 6ms (167 Hz) - for Finland */
	st->rparams.min_blen = 100000;
	st->rparams.max_blen = 1500000; /* 75 ms */
	st->rparams.min_deltat_lp = 19000; /* 1e-3*20e6 - small error */
	st->rparams.max_deltat_lp = 90000;  /* 2*2e-3*20e6 + small error */
	st->rparams.max_type1_pw = 50;	  /* fcc type1 1*20 + 15 */
	st->rparams.max_jp1_2_pw = 70;	  /* jp-1-2 2.5*20+20 */
	st->rparams.jp4_intv = 6660;  /* jp4 hopping radar 333*20 */
	st->rparams.jp1_2_intv = 76923;  /* jp-1-2 3846.15*20 */
	st->rparams.jp2_3_intv = 80000;  /* jp-2-3 4000*20 */
	st->rparams.jp2_1_intv = 27780;  /* jp-2-1 1389*20 */
	st->rparams.type1_intv = 28571;  /* fcc type 1 1428.57*20 */
	st->rparams.max_type2_pw = 150;  /* fcc type 2, 5*20 + 50 */
	st->rparams.min_type3_pw = 70;  /* fcc type 3, 6*20 - 50 */
	st->rparams.max_type3_pw = 250;  /* fcc type 3, 10*20 + 50 */
	st->rparams.min_type4_pw = 160;  /* fcc type 4, 11*20 - 60 */
	st->rparams.max_type4_pw = 460;  /* fcc type 4, 20*20 + 60 */
	st->rparams.min_type2_intv = 3000;
	st->rparams.max_type2_intv = 4600;
	st->rparams.min_type3_4_intv = 4000;
	st->rparams.max_type3_4_intv = 10000;
	st->rparams.sdepth_extra_pulses = 2;
	st->rparams.radar_args.nskip_rst_lp = 2;
	st->rparams.radar_args.min_burst_intv_lp = 12000000;
	st->rparams.radar_args.max_burst_intv_lp = 50000000;
	st->rparams.fc_tol_bw40_sb = 2;
	st->rparams.fc_tol_bw40_bin5_sb = 5;
	st->rparams.fc_tol_bw80_sb = 2;
	st->rparams.fc_tol_bw80_bin5_sb = 5;
	st->rparams.pw_tol_highpowWAR_bin5 = 400;
	st->rparams.pw_chirp_adj_th_bin5 = 800;
	st->rparams.chirp_th1 = 4;
	st->rparams.chirp_th2 = 6;
	st->rparams.chirp_fc_th = 60;
#ifdef BIN5_RADAR_DETECT_WAR
	st->rparams.radar_args.nskip_rst_lp = 3;
#endif // endif
#ifdef BIN5_RADAR_DETECT_WAR_J28
	st->rparams.radar_args.nskip_rst_lp = 3;
#endif // endif
	st->rparams.radar_args.quant = 16;
	st->rparams.radar_args.ncontig = 54832; /* 45616; 37424;37411; */

	/* [100 011 1000 100011]=[1000 1110 0010 0011]=0x8e23 = 36387
	 * bits 15-13: JP2_1, JP4 npulses = 4
	 * bits 12-10: JP1_2_JP2_3 npulses = 3
	 * bits 9-6: EU-t4 fm tol = 8, (8/16)
	 * bit 5-0: max detection index = 35
	 * [100 100 1000 100011]=[1001 0010 0010 0011]=0x9223 = 37411
	 * bits 15-13: JP2_1, JP4 npulses = 4
	 * bits 12-10: JP1_2_JP2_3 npulses = 4
	 * bits 9-6: EU-t4 fm tol = 8, (8/16)
	 * bit 5-0: max detection index = 35
	 * [101 100 1000 110000]=[1011 0010 0011 0000]=0xb230 = 45596
	 * bits 15-13: JP2_1, JP4 npulses = 5
	 * bits 12-10: FCC_1, JP1_2_JP2_3 npulses = 4
	 * bits 9-6: EU-t4 fm tol = 8, (8/16)
	 * bit 5-0: max detection index = 48
	 */

	st->rparams.radar_args.max_pw = 690;  /* 30us + 15% */
	st->rparams.radar_args.thresh0 = st->rparams.radar_thrs.thresh0_20_lo;
	st->rparams.radar_args.thresh1 = st->rparams.radar_thrs.thresh1_20_lo;
	st->rparams.radar_args.thresh0_sc = st->rparams.radar_thrs2.thresh0_sc_20_lo;
	st->rparams.radar_args.thresh1_sc = st->rparams.radar_thrs2.thresh1_sc_20_lo;
	st->rparams.radar_args.fmdemodcfg = 0x7f09;
	/* autocorr[15:5] = PRI var check for stagger radar; autocorr[4:0] = unused */
	st->rparams.radar_args.autocorr = 0x1900;
	/* it is used to check pw for acphy. if pw >200, then check fm */
	//st->rparams.radar_args.st_level_time = 0x8190;
	st->rparams.radar_args.st_level_time = 0x8258;
	st->rparams.radar_args.min_pw = 6;
	st->rparams.radar_args.max_pw_tol = 12;
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) && (pi->u.pi_acphy->lesi == 1)) {
		st->rparams.radar_args.npulses = 6; /* 6; */
		st->rparams.radar_args.npulses_lp = 9; /* 8; */
	} else {
		st->rparams.radar_args.npulses = 7; /* 6; */
		st->rparams.radar_args.npulses_lp = 10; /* 8; */
	}
	st->rparams.radar_args.t2_min = 30528;	/* 0x7740 */
#ifdef BIN5_RADAR_DETECT_WAR
	st->rparams.radar_args.npulses_lp = 6;
	st->rparams.radar_args.t2_min = 31488;
#endif // endif
#ifdef BIN5_RADAR_DETECT_WAR_J28
	st->rparams.radar_args.npulses_lp = 8;
	st->rparams.radar_args.st_level_time = 0x0190;
#endif // endif
	/* t2_min[15:12] = x; if n_non_single >= x && lp_length >
	 * npulses_lp => bin5 detected
	 * t2_min[11:10] = # times combining adjacent pulses < min_pw_lp
	 * t2_min[9] = fm_tol enable
	 * t2_min[8] = skip_type 5 enable
	 * t2_min[7:4] = y; bin5 remove pw <= 10*y
	 * t2_min[3:0] = t; non-bin5 remove pw <= 5*y
	 * st_level_time[11:0] =  pw criterion for short pluse noise filter
	 * st_level_time[15:12] =  2^x-1 as FMOFFSET
	 */
	st->rparams.radar_args.min_pw_lp = 700;
#ifdef BIN5_RADAR_DETECT_WAR
	st->rparams.radar_args.min_pw_lp = 50;
#endif // endif
	st->rparams.radar_args.max_pw_lp = 2000;
	if (TONEDETECTION)
		st->rparams.radar_args.min_fm_lp = 500 - 256;
	else
#ifdef NPHYREV7_HTPHY_DFS_WAR
		st->rparams.radar_args.min_fm_lp = 25;
#else
		st->rparams.radar_args.min_fm_lp = 45;
#endif // endif

#ifdef BIN5_RADAR_DETECT_WAR_J28
	st->rparams.radar_args.min_fm_lp  = 20;
#endif // endif
	st->rparams.radar_args.max_span_lp = 61708;  /* 0xf20c; 15, 1, 12 */
	/* max_span_lp[15:12] = skip_tot max */
	/* max_span_lp[11:8] = x, x/16 = % alowed fm tollerance bin5 */
	/* max_span_lp[7:0] = alowed pw tollerance bin5 */

	st->rparams.radar_args.min_deltat = 2000;
	st->rparams.radar_args.max_deltat = 3000000;
	st->rparams.radar_args.version = WL_RADAR_ARGS_VERSION;
	if (TONEDETECTION) {
		st->rparams.radar_args.fra_pulse_err = 65282; /* 0x1002, */
		/* bits 15-8: EU-t4 min_fm = 255 */
		/* bits 7-0: time from last det = 2 minute */
	} else {
		st->rparams.radar_args.fra_pulse_err = 4098; /* 0x1002, */
		/* bits 15-8: EU-t4 min_fm = 16 */
		/* bits 7-0: time from last det = 2 minute */
	}

	/* 0x8444, bits 15:14 low_intv_eu_t2 */
	/* bits 13:12 low_intv_eu_t1; npulse -- bit 11:8 for EU type 4, */
	/* bits 7:4 = 4 for EU type 2, bits 3:0= 4 for EU type 1 */
	/* 11 11 0100 0100 0100 */
	st->rparams.radar_args.npulses_fra = 1638; /* 33860; */
	st->rparams.radar_args.npulses_stg2 = 7; /* 5; */
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) && (pi->u.pi_acphy->lesi == 1)) {
		st->rparams.radar_args.npulses_stg3 = 5; /* 5; */
	} else {
		st->rparams.radar_args.npulses_stg3 = 6; /* 5; */
	}
	st->rparams.radar_args.percal_mask = 0x31;
	st->rparams.radar_args.feature_mask = 0xa800;
#ifdef NPHYREV7_HTPHY_DFS_WAR
	st->rparams.radar_args.blank = 0x2c19;
#else
	st->rparams.radar_args.blank = 0x6419;
#endif // endif

	/* feature mask bit definitions:
	 * bit-0  : (off) : when feature_mask bit-3 is on, 0=>bin5 data, 1=>short pulse data
	 * bit-1  : (off) : output before-fitlered pulse data
	 * bit-2  : (off) : output # pulses at each antenna (if # pulse > 5)
	 * bit-3  : (off) : output radar pulses data at various points (tstart, intv, pw, fm)
	 * bit-4  : (off) : output wlc_phy_radar_detect_uniform_pw_check(..) debug messages
	 * bit-5  : (off) : output staggered reset
	 * bit-6  : (off) : output fifo output
	 * bit-7  : (off) : output intv and pruned pw even No detection
	 * bit-8  : (on)  : enable UK radar detection
	 * bit-9  : (off) : output EU type debug messages
	 * bit-10 : (off) : disable/enable the new staggered 2/3 detection
	 * bit-11 : (on)  : enable fcc radar detection
	 * bit-12 : (on)  : enable etsi radar detection
	 * bit-13 : (on)  : for combining pulse use max of pw(i) & pw(i-1) inlieu of pw(i-1)+pw(i)
	 * bit-14 : (off) : output the Skipped radar if delta2 is greated than 2 mins
	 * bit-15 : (off) : use 384 of 511 FIFO data
	 */
}

static int
WLBANDINITFN(phy_ac_radar_init)(phy_type_radar_ctx_t *ctx, bool on)
{
	phy_ac_radar_info_t *info = (phy_ac_radar_info_t *)ctx;
	phy_radar_info_t *ri = info->ri;
	phy_info_t *pi = info->pi;
	phy_radar_st_t *st;
	//int BCM4365_TAG = 1;
	//int PLL_Num = 1;
	//uint16 phymode = phy_get_phymode(pi);

	PHY_TRACE(("%s: init %d\n", __FUNCTION__, on));

	st = phy_radar_get_st(ri);
	ASSERT(st != NULL);

	/* Update radar_args according to the chanspec */
	if (CHSPEC_CHANNEL(pi->radio_chanspec) <= WL_THRESHOLD_LO_BAND) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_20_lo;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_20_lo;
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_40_lo;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_40_lo;
		} else if (CHSPEC_IS80(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_80_lo;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_80_lo;
		} else {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_160_lo;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_160_lo;
		}
	} else {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_20_hi;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_20_hi;
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_40_hi;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_40_hi;
		} else if (CHSPEC_IS80(pi->radio_chanspec)) {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_80_hi;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_80_hi;
		} else {
			st->rparams.radar_args.thresh0 =
				st->rparams.radar_thrs.thresh0_160_hi;
			st->rparams.radar_args.thresh1 =
				st->rparams.radar_thrs.thresh1_160_hi;
		}
	}

	if (on) {
		/* phy_utils_write_phyreg(pi, ACPHY_RadarBlankCtrl, */
		/*   (st->rparams.radar_args.blank | (0x0000))); */
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh0(pi->pubpi.phy_rev),
			(uint16)((int16)st->rparams.radar_args.thresh0));
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh1(pi->pubpi.phy_rev),
			(uint16)((int16)st->rparams.radar_args.thresh1));
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh0_core1(pi->pubpi.phy_rev),
			(uint16)((int16)st->rparams.radar_args.thresh0));
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh1_core1(pi->pubpi.phy_rev),
			(uint16)((int16)st->rparams.radar_args.thresh1));
		phy_utils_write_phyreg(pi, ACPHY_Radar_t2_min(pi->pubpi.phy_rev),
			0);

		phy_utils_write_phyreg(pi, ACPHY_FMDemodConfig(pi->pubpi.phy_rev),
			st->rparams.radar_args.fmdemodcfg);
		phy_utils_write_phyreg(pi, ACPHY_RadarBlankCtrl2(pi->pubpi.phy_rev), 0x5f88);

		wlapi_bmac_write_shm(pi->sh->physhim,
			M_RADAR_REG, st->rparams.radar_args.thresh1);

		/* percal_mask to disable radar detection during selected period cals */
		pi->radar_percal_mask = st->rparams.radar_args.percal_mask;

		if (TONEDETECTION) {
			PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(ACPHY, RadarSearchCtrl(pi->pubpi.phy_rev), 1)
			PHY_REG_WRITE_ENTRY(ACPHY, RadarDetectConfig1(pi->pubpi.phy_rev), 0x3206)
			/* enable new fm */
			PHY_REG_WRITE_ENTRY(ACPHY, RadarDetectConfig2(pi->pubpi.phy_rev), 0x141)
			PHY_REG_LIST_EXECUTE(pi);
		}
		else {
			PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(ACPHY, RadarSearchCtrl(pi->pubpi.phy_rev), 1)
			PHY_REG_WRITE_ENTRY(ACPHY, RadarDetectConfig1(pi->pubpi.phy_rev), 0x3206)
			/* enable new fm */
			PHY_REG_WRITE_ENTRY(ACPHY, RadarDetectConfig2(pi->pubpi.phy_rev), 0x40)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} /* if (on) */
	else { /* handling radar disable request */
		PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(ACPHY, RadarSearchCtrl(pi->pubpi.phy_rev), 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	if (TINY_RADIO(pi)) {
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			phy_utils_write_phyreg(pi, ACPHY_Radar_adc_to_dbm(pi->pubpi.phy_rev),
				0x34a3);

			if (CHSPEC_IS8080(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
				phy_utils_write_phyreg(pi, ACPHY_RadarDetectConfig1
					(pi->pubpi.phy_rev), 0x32f7);
			} else	 {
				phy_utils_write_phyreg(pi, ACPHY_RadarDetectConfig1
					(pi->pubpi.phy_rev), 0x3237);
			}
			phy_utils_write_phyreg(pi, ACPHY_RadarT3BelowMin(pi->pubpi.phy_rev),
				0x14);
			if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarMaLength(pi->pubpi.phy_rev), 0x08)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 200)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarResetBlankingDelay(pi->pubpi.phy_rev), 25)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarBlankCtrl(pi->pubpi.phy_rev), 0xac19)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 0x258)
			PHY_REG_LIST_EXECUTE(pi);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarMaLength(pi->pubpi.phy_rev), 0x10)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 400)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarResetBlankingDelay(pi->pubpi.phy_rev), 50)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarBlankCtrl(pi->pubpi.phy_rev), 0xac32)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 0x4b0)
			PHY_REG_LIST_EXECUTE(pi);
			} else {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarMaLength(pi->pubpi.phy_rev), 0x1f)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 800)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarResetBlankingDelay(pi->pubpi.phy_rev), 100)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarBlankCtrl(pi->pubpi.phy_rev), 0xac64)
				PHY_REG_WRITE_ENTRY(ACPHY,
					RadarT3Timeout(pi->pubpi.phy_rev), 0x960)
			PHY_REG_LIST_EXECUTE(pi);
			}
		} else {
			phy_utils_write_phyreg(pi, ACPHY_Radar_adc_to_dbm(pi->pubpi.phy_rev),
				0x4ac);
			phy_utils_write_phyreg(pi, ACPHY_RadarDetectConfig1(pi->pubpi.phy_rev),
				0x2c06);
		}
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				phy_utils_write_phyreg(pi,
					ACPHY_RadarMaLength_SC(pi->pubpi.phy_rev), 0x8);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarT3Timeout_SC(pi->pubpi.phy_rev), 600);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarResetBlankingDelay_SC(pi->pubpi.phy_rev), 25);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarBlankCtrl_SC(pi->pubpi.phy_rev), 0x8019);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				phy_utils_write_phyreg(pi,
					ACPHY_RadarMaLength_SC(pi->pubpi.phy_rev), 0x10);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarT3Timeout_SC(pi->pubpi.phy_rev), 1200);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarResetBlankingDelay_SC(pi->pubpi.phy_rev), 50);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarBlankCtrl_SC(pi->pubpi.phy_rev), 0x8032);
			} else {
				phy_utils_write_phyreg(pi,
					ACPHY_RadarMaLength_SC(pi->pubpi.phy_rev), 0x1f);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarT3Timeout_SC(pi->pubpi.phy_rev), 2400);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarResetBlankingDelay_SC(pi->pubpi.phy_rev), 100);
				phy_utils_write_phyreg(pi,
					ACPHY_RadarBlankCtrl_SC(pi->pubpi.phy_rev), 0x8064);
			}
			phy_utils_write_phyreg(pi,
				ACPHY_Radar_adc_to_dbm_SC(pi->pubpi.phy_rev), 0x34a0);
			phy_utils_write_phyreg(pi,
				ACPHY_RadarDetectConfig1_SC(pi->pubpi.phy_rev), 0x3207);
			phy_utils_write_phyreg(pi,
				ACPHY_RadarT3BelowMin_SC(pi->pubpi.phy_rev), 0x14);
			phy_utils_write_phyreg(pi,
				ACPHY_RadarGainOverride_SC(pi->pubpi.phy_rev), 0x43d);
			phy_utils_write_phyreg(pi,
				ACPHY_RadarBlankCtrl2_SC(pi->pubpi.phy_rev), 0xa000);
		}
	}

	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_RADARWAR, (on ? MHF1_RADARWAR : 0), FALSE);

	return BCME_OK;
}

static int
phy_ac_radar_run(phy_type_radar_ctx_t *ctx, int PLL_idx, int BW80_80_mode)
{
	phy_ac_radar_info_t *info = (phy_ac_radar_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return phy_radar_run_nphy(pi, PLL_idx, BW80_80_mode);
}
