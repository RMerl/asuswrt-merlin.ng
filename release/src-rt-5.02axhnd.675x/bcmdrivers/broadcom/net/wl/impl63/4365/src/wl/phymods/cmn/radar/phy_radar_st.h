/*
 * RadarDetect module internal interface.
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

#ifndef _phy_radar_st_
#define _phy_radar_st_

#include <wlioctl.h>
#include <phy_radar_api.h>
#include <phy_radar.h>

/* PLEASE UPDATE THE FOLLOWING REVISION HISTORY AND VALUES OF #DEFINE'S */
/* DFS_SW_VERSION, DFS_SW_SUB_VERSION, DFS_SW_DATE_MONTH, AND */
/* DFS_SW_YEAR EACH TIME THE DFS CODE IS CHANGED. */
/* NO NEED TO CHANGE SW VERSION FOR RADAR THRESHOLD CHANGES */
/* Revision history: */
/* ver 2.001, 0612 2011: Overhauled short pulse detection to address EU types 1, 2, 4 */
/*     detection with traffic. Also increased npulses_stg2 to 5 */
/*     Tested on 4331 to pass EU and FCC radars. No false detection in 24 hours */
/* ver 2.002, 0712 2011: renamed functions wlc_phy_radar_detect_uniform_pw_check(..) */
/*     and wlc_phy_radar_detect_pri_pw_filter(..) from previous names to */
/*     better reflect functionality. Change npulses_fra to 33860,  */
/*     making the effective npulses of EU type 4 from 3 to 4 */
/* ver 2.003, 0912 2011: changed min_fm_lp of 20MHz from 45 to 25 */
/*     added Japan types 1_2, 2_3, 2_1, 4 detections. Modified radar types */
/*     in wlc_phy_shim.h and wlc_ap.c */
#ifdef NPHYREV7_HTPHY_DFS_WAR
/* ver 3.000, 0911 2013: update to improve false detection as in acphy */
/*     This include ucode to disable radar during Rx after RXstart, and */
/*     disable radar during Tx. Enable radar smoothing and turn off Reset */
/*     blanking (to not blank the smoothed sample). Set appropriate smoothing */
/*     lengths. Changed min_fm_lp to 25, npulses_lp to 9 for nphy and htphy. */
#define DFS_SW_VERSION	3
#define DFS_SW_SUB_VERSION	0
#define DFS_SW_DATE_MONTH	1109
#define DFS_SW_YEAR	2013
#else
#define DFS_SW_VERSION	2
#define DFS_SW_SUB_VERSION	3
#define DFS_SW_DATE_MONTH	1012
#define DFS_SW_YEAR	2011
#endif // endif
/* Radar detect scratchpad area, RDR_NTIER_SIZE must be bigger than RDR_TIER_SIZE */
#define RDR_NTIERS  1	   /* Number of tiers */
#define RDR_NTIERS_APHY  2	   /* Number of tiers for aphy only */
#define RDR_TIER_SIZE 64   /* Size per tier, aphy  */
#define RDR_LIST_SIZE (512/3 + 2)  /* Size of the list (rev 3 fifo size = 512) */
#define RDR_EPOCH_SIZE 40

#ifdef BCMPHYCORENUM
#  if BCMPHYCORENUM > 1
#    define RDR_NANTENNAS 2
#  else /* BCMPHYCORENUM == 1 */
#    define RDR_NANTENNAS 1
#  endif /* BCMPHYCORENUM > 1 */
#  define GET_RDR_NANTENNAS(pi) (BCM4350_CHIP((pi)->sh->chip) ? 1 : RDR_NANTENNAS)
#else /* !BCMPHYCORENUM */
#  define RDR_NANTENNAS 2
#  define GET_RDR_NANTENNAS(pi) ((((pi)->pubpi.phy_corenum > 1) && \
				  (!BCM4350_CHIP((pi)->sh->chip))) ? 2 : 1)
#endif /* BCMPHYCORENUM */

#define RDR_NTIER_SIZE  RDR_LIST_SIZE  /* Size per tier, nphy */
#define RDR_LP_BUFFER_SIZE 64
#define LP_LEN_HIS_SIZE 10
#define LP_BUFFER_SIZE 64
#define MAX_LP_BUFFER_SPAN_20MHZ 240000000
#define MAX_LP_BUFFER_SPAN_40MHZ 480000000
#define RDR_SDEPTH_EXTRA_PULSES 1
#define TONEDETECTION 1
#define LPQUANT 128

typedef struct {
	wl_radar_args_t radar_args;	/* radar detection parametners */
	wl_radar_thr_t radar_thrs;	/* radar thresholds */
	wl_radar_thr2_t radar_thrs2;	/* radar thresholds for subband and 3+1 */
	int min_tint;			/* minimum t_int (1/prf) (20 MHz clocks) */
	int max_tint;			/* maximum t_int (20 MHz clocks) */
	int min_blen;			/* minimum burst length (20 MHz clocks) */
	int max_blen;			/* maximum burst length (20 MHz clocks) */
	int sdepth_extra_pulses;
	int min_deltat_lp;
	int max_deltat_lp;
	int max_type1_pw;		/* max fcc type 1 radar pulse width */
	int jp2_1_intv;		/* min fcc type 1 radar pulse repetition interval */
	int jp4_intv;
	int type1_intv;
	int max_type2_pw;
	int max_type2_intv;
	int min_type2_intv;
	int min_type4_pw;
	int max_type4_pw;
	int min_type3_pw;
	int max_type3_pw;
	int min_type3_4_intv;
	int max_type3_4_intv;
	int max_jp1_2_pw;
	int jp1_2_intv;
	int jp2_3_intv;
	int fc_tol_bw40_sb;
	int fc_tol_bw40_bin5_sb;
	int fc_tol_bw80_sb;
	int fc_tol_bw80_bin5_sb;
	int pw_tol_highpowWAR_bin5;
	int pw_chirp_adj_th_bin5;
	int chirp_th1;
	int chirp_th2;
	int chirp_fc_th;
} radar_params_t;

typedef struct {
	uint16 length;
	uint32 tstart_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	uint16 width_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	int16 fm_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	int16 fc_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	uint16 chirp_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	uint16 notradar_list[RDR_NANTENNAS * RDR_LIST_SIZE];
	uint16 epoch_start[RDR_EPOCH_SIZE];
	uint16 epoch_finish[RDR_EPOCH_SIZE];
	int tiern_list[RDR_NTIERS][RDR_NTIER_SIZE]; /* increased size of tiern list */
	uint16 tiern_pw[RDR_NTIERS][RDR_NTIER_SIZE];
	int16 tiern_fm[RDR_NTIERS][RDR_NTIER_SIZE];
	int16 tiern_fc[RDR_NTIERS][RDR_NTIER_SIZE];
	uint16 tiern_chirp[RDR_NTIERS][RDR_NTIER_SIZE];
	uint16 tiern_notradar[RDR_NTIERS][RDR_NTIER_SIZE];
	uint16 nepochs;
	uint16 nphy_length[RDR_NANTENNAS];
	uint32 tstart_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 width_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	int16 fm_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	int16 fc_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 chirp_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 notradar_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 nphy_length_bin5[RDR_NANTENNAS];
	uint32 tstart_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 width_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	int16 fm_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	int16 fc_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 chirp_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint16 notradar_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	uint8 lp_length;
	uint16 min_detected_chirp_bin5;
	uint16 max_detected_chirp_bin5;
	int16 min_detected_fc_bin5;
	int16	max_detected_fc_bin5;
	int16	avg_detected_fc_bin5;
	int16 var_fc_bin5;
	int16	subband_result;
	uint32 lp_buffer[RDR_LP_BUFFER_SIZE];
	uint32 last_tstart;
	uint8 lp_cnt;
	uint8 lp_skip_cnt;
	int lp_pw_fm_matched;
	uint16 lp_pw[3];
	int16 lp_fm[3];
	int lp_n_non_single_pulses;
	bool lp_just_skipped;
	uint16 lp_skipped_pw;
	int16 lp_skipped_fm;
	uint8 lp_skip_tot;
	uint8 lp_csect_single;
	uint32 lp_timer;
	uint32 last_detection_time;
	uint32 last_detection_time_lp;
	uint32 last_skipped_time;
	uint8 lp_len_his[LP_LEN_HIS_SIZE];
	uint8 lp_len_his_idx;
	uint32 tstart_list_tail[2];
	uint16 width_list_tail[2];
	int16 fm_list_tail[2];
	int16 fc_list_tail[2];
	uint16 chirp_list_tail[2];
	uint16 notradar_list_tail[2];
} radar_work_t;

/* RADAR data structure */
typedef struct {
	radar_work_t	radar_work;	/* radar work area */
	radar_params_t	rparams;
	phy_radar_detect_mode_t rdm;    /* current radar detect mode FCC/EU */
	wl_radar_status_t radar_status;	/* dump/clear radar status */
	bool first_radar_indicator;	/* first radar indicator */

	radar_work_t	radar_work_sc;	/* scan core radar work area */
	wl_radar_status_t radar_status_sc;	/* dump/clear radar status */
	bool first_radar_indicator_sc;	/* first radar indicator */
} phy_radar_st_t;

/*
 * Query the radar states pointer.
 */
phy_radar_st_t *phy_radar_get_st(phy_radar_info_t *ri);

#endif /* _phy_radar_st_ */
