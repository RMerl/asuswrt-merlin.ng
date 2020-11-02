/*
 * NPHY specific defines and declarations
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
 * $Id: wlc_phy_extended_n.h $
 */

#ifndef _wlc_phy_extended_n_h_
#define _wlc_phy_extended_n_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phy_types.h>

/* ------------------- */
/*  MACRO definitions  */
/* ------------------- */

#define NPHY_INTF_RSSI_ENAB(pi)	((CHIPID_43236X_FAMILY(pi)) || \
	(CHIPID(pi->sh->chip) == BCM43239_CHIP_ID) || \
	(CHIPID(pi->sh->chip) == BCM4322_CHIP_ID))

#define MOD_PHYREG3(pi, phy_type, reg, field, value)	\
	phy_utils_mod_phyreg(pi, phy_type##_##reg, \
	phy_type##_##reg##_##field##_##MASK, (value) << phy_type##_##reg##_##field##_##SHIFT);

#define READ_PHYREG3(pi, phy_type, reg, field)	\
	((phy_utils_read_phyreg(pi, phy_type##_##reg) \
	& phy_type##_##reg##_##field##_##MASK) >> phy_type##_##reg##_##field##_##SHIFT)

#define WRITE_PHY_REG3(pi, phy_type, reg, value)	\
	phy_utils_write_phyreg(pi, phy_type##_##reg, value);

#define	READ_RADIO_REG2(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1))
#define	WRITE_RADIO_REG2(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1), value);
#define	WRITE_RADIO_SYN(pi, radio_type, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##SYN##_##reg_name, value);

#define	READ_RADIO_REG3(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name));
#define	WRITE_RADIO_REG3(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name), value);
#define	READ_RADIO_REG4(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1));
#define	WRITE_RADIO_REG4(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1), value);

#define wlc_phy_set_target_tx_pwr_nphy(pi, target) \
	phy_utils_write_phyreg(pi, \
		      NPHY_TxPwrCtrlTargetPwr,				\
		      ((uint16)MAX(pi->u.pi_nphy->tssi_minpwr_limit,	\
				   (MIN(pi->u.pi_nphy->tssi_maxpwr_limit, (uint16)(target)))) \
		       << NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT) |	\
		      ((uint16)MAX(pi->u.pi_nphy->tssi_minpwr_limit,	\
				   (MIN(pi->u.pi_nphy->tssi_maxpwr_limit, (uint16)(target)))) \
		       << NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT))

#define wlc_phy_get_target_tx_pwr_nphy(pi) \
	((phy_utils_read_phyreg(pi, NPHY_TxPwrCtrlTargetPwr) & \
		NPHY_TxPwrCtrlTargetPwr_targetPwr0_MASK) >> \
		NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define NPHY_ACI_MAX_UNDETECT_WINDOW_SZ 8 /* max window of aci detect */
#define NPHY_ACI_CHANNEL_DELTA 5   /* How far a signal can bleed */
#define NPHY_ACI_CHANNEL_SKIP 4	   /* Num of immediately surrounding channels to skip */
#define NPHY_ACI_40MHZ_CHANNEL_DELTA 6
#define NPHY_ACI_40MHZ_CHANNEL_SKIP 5
#define NPHY_ACI_40MHZ_CHANNEL_DELTA_GE_REV3 6
#define NPHY_ACI_40MHZ_CHANNEL_SKIP_GE_REV3 5
#define NPHY_ACI_CHANNEL_DELTA_GE_REV3 4   /* How far a signal can bleed */
#define NPHY_ACI_CHANNEL_SKIP_GE_REV3 3	   /* Num of immediately surrounding channels to skip */

/* noise immunity raise/lowered using crsminpwr or init gain value */
/* before assoc, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_NOASSOC_GLITCH_TH_UP 2

/* before assoc, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_NOASSOC_GLITCH_TH_DN 2

/* after assoc, no aci, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_ASSOC_GLITCH_TH_UP 2

/* after assoc, no aci, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_ASSOC_GLITCH_TH_DN 2

/* after assoc, aci on, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_ASSOC_ACI_GLITCH_TH_UP 2

/* after assoc, aci on, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_ASSOC_ACI_GLITCH_TH_DN 2

/* not associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define NPHY_NOISE_NOASSOC_ENTER_TH  400
#define NPHY_NOISE_NOASSOC_ENTER_TH_REV7  500
#define NPHY_NOISE_NOASSOC_ENTER_TH_REV19  200

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define NPHY_NOISE_ASSOC_ENTER_TH  400
#define NPHY_NOISE_ASSOC_ENTER_TH_REV7  500
#define NPHY_NOISE_ASSOC_ENTER_TH_REV19  200

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (for both ofdm and bphy) */
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH  400
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_REV7  400
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_REV16  150
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_REV19 100
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_ELNA 100

#define W3_NB_THRESH 20
#define W3_NB_CNT_THRESH 1

/* wl interference 4 array for crs min pwr index  */
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX 44
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_7 120
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_17 120
/* intf., mode 1/4 crs max index corresponds to -77dBm CWJ */
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_16_ACI_OFF 96
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_16_ACI_ON 72
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_19 80

/* wl interference 4, no assoc, crsminpwr index increment */
#define NPHY_NOISE_NOASSOC_CRSIDX_INCR 16

/* wl interference 4, assoc, crsminpwr index increment */
#define NPHY_NOISE_ASSOC_CRSIDX_INCR 8
/* wl interference 4, crsminpwr index decr */
#define NPHY_NOISE_CRSIDX_DECR   1

/* BPHY desense params */
#ifdef BPHY_DESENSE
#define BPHY_DESENSE_NOISE_ENTER_TH	40 /* (nphy_th/5) */
#define BPHY_DESENSE_NOISE_CRSIDX_INCR	1
#define BPHY_DESENSE_NOISE_CRSIDX_DECR	1
#define BPHY_DESENSE_CRSMINPWR_BASELINE	0x46
#define BPHY_DESENSE_CRSMINPWR_ARRAY_MAX_INDEX 15
#endif // endif

#ifdef NOISE_CAL_LCNXNPHY /* NOISE CAL RELATED DEFINES */
#define OLD_NOISE_AVG_SCHEME
#define NPHY_NOISE_MEASURE_WINDOW_2G 1800 /* In uS */
#define NPHY_NOISE_MEASURE_WINDOW_5G 1400 /* In uS */
#define NPHY_NPWR_MINLMT 1
#define NPHY_INIT_NOISE_CAL_TMOUT 38000 /* In uS */
#define NPHY_NPWR_LGC_MINLMT_20MHZ 12
#define NPHY_NPWR_LGC_MAXLMT_20MHZ 38
#define NPHY_NPWR_LGC_MINLMT_40MHZ_2G 12
#define NPHY_NPWR_LGC_MAXLMT_40MHZ_2G 38
#define NPHY_NPWR_LGC_MINLMT_20MHZ_20671 5
#define NPHY_NPWR_LGC_MAXLMT_20MHZ_20671 19
#define NPHY_NPWR_LGC_MINLMT_40MHZ_2G_20671 5
#define NPHY_NPWR_LGC_MAXLMT_40MHZ_2G_20671 19
#define NPHY_NPWR_MAXLMT_2G 50
#define NPHY_NPWR_LGC_MINLMT_20MHZ_5G 9
#define NPHY_NPWR_LGC_MAXLMT_20MHZ_5G 36
#define NPHY_NPWR_LGC_MINLMT_40MHZ_5G 9
#define NPHY_NPWR_LGC_MAXLMT_40MHZ_5G 36
#define NPHY_NPWR_LGC_MINLMT_20MHZ_5G_20671 3
#define NPHY_NPWR_LGC_MAXLMT_20MHZ_5G_20671 12
#define NPHY_NPWR_LGC_MINLMT_40MHZ_5G_20671 3
#define NPHY_NPWR_LGC_MAXLMT_40MHZ_5G_20671 12
#define NPHY_NPWR_MAXLMT_5G 200
#define NPHY_MAX_GAIN_CHANGE_LMT_2G 6 /* 3 index = 9dB */
#define NPHY_MAX_GAIN_CHANGE_LMT_5G 6 /* 5 index = 15 db */
/* XXX 20671 NOTE: if we want to support more than two notch of gain change
 * we may have to revist the lgc_gain_change block to rework the if/else
 * check boundary conditions
 */
#define NPHY_MAX_GAIN_CHANGE_LMT_2G_20671 6 /* 3 index = 9dB */
#define NPHY_MAX_GAIN_CHANGE_LMT_5G_20671 6 /* 5 index = 15 db */
#define NPHY_MAX_RXPO_CHANGE_LMT_2G 16
#define NPHY_MAX_RXPO_CHANGE_LMT_5G 16
#define CRSMINPWR_BASELINE_4324x_2G	0x46
#define CRSMINPWR_BASELINE_4324x_5G	0x3d
#endif /* NOISE CAL RELATED DEFINES */

#define NPHY_IS_SROM_REINTERPRET NREV_GE(pi->pubpi.phy_rev, 5)

/* rssi cal defines */
#define NPHY_RSSICAL_MAXREAD 31 /* max possible reading from 6-bit ADC */

#define NPHY_RSSICAL_NPOLL 8
#define NPHY_RSSICAL_MAXD  (1<<20)
#define NPHY_MIN_RXIQ_PWR 2
#define ENABLE_RXIQCAL_DBG 0

#define NPHY_RSSICAL_W1_TARGET 25
#define NPHY_RSSICAL_W2_TARGET NPHY_RSSICAL_W1_TARGET
#define NPHY_RSSICAL_NB_TARGET 0

#define NPHY_RSSICAL_W1_TARGET_REV3 29
#define NPHY_RSSICAL_W2_TARGET_REV3 NPHY_RSSICAL_W1_TARGET_REV3

#define NPHY_CALSANITY_RSSI_NB_MAX_POS  9
#define NPHY_CALSANITY_RSSI_NB_MAX_NEG -9
#define NPHY_CALSANITY_RSSI_W1_MAX_POS  12
#define NPHY_CALSANITY_RSSI_W1_MAX_NEG (NPHY_RSSICAL_W1_TARGET - NPHY_RSSICAL_MAXREAD)
#define NPHY_CALSANITY_RSSI_W2_MAX_POS  NPHY_CALSANITY_RSSI_W1_MAX_POS
#define NPHY_CALSANITY_RSSI_W2_MAX_NEG (NPHY_RSSICAL_W2_TARGET - NPHY_RSSICAL_MAXREAD)
#define NPHY_RSSI_SXT(x) ( (int8) ( -((x) & 0x20)  +  ((x) & 0x1f) ) ) /* sign ext 6 to 8 */
#define NPHY_RSSI_NB_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_NB_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_NB_MAX_NEG))
#define NPHY_RSSI_W1_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_W1_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_W1_MAX_NEG))
#define NPHY_RSSI_W2_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_W2_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_W2_MAX_NEG))

#define NPHY_IQCAL_NUMGAINS 9
#define NPHY_N_GCTL 0x66

#define NPHY_PAPD_EPS_TBL_SIZE 64
#define NPHY_PAPD_SCL_TBL_SIZE 64
#define	NPHY_DIG_FILT_COEFFS_CCK	2
#define	NPHY_DIG_FILT_COEFFS_OFDM22	7
#define	NPHY_DIG_FILT_COEFFS_43239CCK	10
#define	NPHY_DIG_FILT_COEFFS_OFDM26	11
#define	NPHY_DIG_FILT_COEFFS_OFDM5	12
#define NPHY_DIG_FILT_COEFFS_OFDM4_43217	14
#define NPHY_NUM_DIG_FILT_COEFFS 15

#define NPHY_PAPD_COMP_OFF 0
#define NPHY_PAPD_COMP_ON  1

#define NPHY_SROM_TEMPSHIFT		32
#define NPHY_SROM_MAXTEMPOFFSET		16
#define NPHY_SROM_MINTEMPOFFSET		-16

#define NPHY_CAL_MAXTEMPDELTA		64

/* Length of noisevar table */
#define NPHY_NOISEVAR_TBLLEN40 256
#define NPHY_NOISEVAR_TBLLEN20 128

/* Start and end address for noise-variance offset in NPHY_TBL_ID_NOISEVAR */
#define NPHY_RATE_BASED_NV_OFFSET_START	256
#define NPHY_RATE_BASED_NV_OFFSET_END	271

/* Factor for reducing the analog Rx LPF b/w for Ch11
 * 40 MHz spur avoidance on 4322
 */
#define NPHY_ANARXLPFBW_REDUCTIONFACT 7

/* Min CRS power for Ch11 40 MHz spur avoidance WAR for 4322 */
#define NPHY_ADJUSTED_MINCRSPOWER 0x1e

/* Draconian Power Limits for Sulley */
#define NPHY_TSSI_SET_MAX_LIMIT 1
#define NPHY_TSSI_SET_MIN_LIMIT 2
#define NPHY_TSSI_SET_MIN_MAX_LIMIT 3

/* To set eLNA gain in clip lo region */
#define NPHY_DELTGAIN_LEVEL_24 24
#define NPHY_DELTGAIN_LEVEL_21 21
#define NPHY_DELTGAIN_LEVEL_18 18
#define NPHY_DELTGAIN_LEVEL_15 15
#define NPHY_DELTGAIN_LEVEL_12 12
#define NPHY_DELTGAIN_LEVEL_9 9
#define NPHY_gainType_HI 1
#define NPHY_gainType_MD 2
#define NPHY_gainType_LO 3
#define NPHY_gainType_INIT 4

/* contains settings from BCM20671_JTAG.xls */
#define PLL_20671_NDIV			65
#define PLL_20671_KVCO			50
#define PLL_20671_LOW_END_VCO		3200
#define PLL_20671_LOW_END_KVCO_Q16	1514349
#define PLL_20671_HIGH_END_VCO		4000
#define PLL_20671_HIGH_END_KVCO_Q16	3146447
#define PLL_20671_LOOP_BW_DESIRED	300
#define PLL_20671_LOOP_BW		266
#define PLL_20671_LF_R1			4250
#define PLL_20671_LF_R2			2000
#define PLL_20671_LF_R3			2000
#define PLL_20671_LF_C1			408
#define PLL_20671_LF_C2_X10		272
#define PLL_20671_LF_C3_X100		816
#define PLL_20671_LF_C4_X100		816
#define PLL_20671_CP_CURRENT		384
#define PLL_20671_D35			408
#define PLL_20671_D32			5178

/* LOFT COEFF OFFSET FOR BPHY BCM4324B1 */
#define NPHY_BPHY_LOFT_OFFSET -36

#define wlc_phy_get_papd_nphy(pi) \
	(phy_utils_read_phyreg((pi), NPHY_TxPwrCtrlCmd) & \
		(NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
		NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
		NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))

/* RXIQCAL Params */
#define NPHY_RXCAL_TONEAMP 181
#define NPHY_RXCAL_TONEFREQ_40MHz 4000
#define NPHY_RXCAL_TONEFREQ_20MHz 2000
#define NPHY_RXCAL_TONEFREQ_40MHz_20671_2G 4000 /* was 14000 */
#define NPHY_RXCAL_TONEFREQ_20MHz_20671_2G 2000 /* was 7000  */
#define NPHY_RXCAL_TONEFREQ_40MHz_20671_5G 8000
#define NPHY_RXCAL_TONEFREQ_20MHz_20671_5G 2000

/* spectrum shaping filter for intPA (ofdm2, ofdm40, cck) */
#define TXFILT_SHAPING_OFDM20   0
#define TXFILT_SHAPING_OFDM40   1
#define TXFILT_SHAPING_CCK      2
#define TXFILT_DEFAULT_OFDM20   3
#define TXFILT_DEFAULT_OFDM40   4

/* ADC MODES */
#define NPHY_REV19_ADC_MODE_20M	0
#define NPHY_REV19_ADC_MODE_40M	1

#define NPHY_IPA_RXCAL_MAXGAININDEX (6 - 1)
#define NPHY_NUM_LOWPWR_LO_COEFFS 40

/* includes crsminpwr calc offset w.r.t initgain and headroom */
#define NPHY_RSSI_TO_CRS_GAIN_OFFSET 12

/* ------------------- */
/*  fn() declarations  */
/* ------------------- */

extern bool wlc_phy_chan2freq_nphy(phy_info_t *pi, uint channel, int *f,
	chan_info_nphy_radio2057_t **t0, chan_info_nphy_radio205x_t **t1,
	chan_info_nphy_radio2057_rev5_t **t2, chan_info_nphy_2055_t **t3,
	chan_info_nphy_radio20671_t **t4);
#ifdef ENABLE_FDS
extern void wlc_phy_4324x_fds_nphy(phy_info_t *pi, bool enable);
#endif // endif
extern void wlc_phy_copy_ppr_offsets_nphy(phy_info_t *pi);
extern void wlc_phy_rfseq_nphy_rev19(phy_info_t *pi);
extern void wlc_phy_oclscd_setup_cleanup_nphy(phy_info_t *pi, bool setup);
extern void wlc_phy_oclinit_nphy_rev19(phy_info_t *pi, uint8 ocl_en, uint8 scd_en);
extern void wlc_phy_rfctrl_override_nphy_rev19(phy_info_t *pi, uint16 field, uint16 value,
	uint8 core_mask, uint8 off, uint8 override_id);
extern void wlc_phy_rev3_pwrctrl_rfctrl_override_signals_nphy(phy_info_t *pi, uint8 cores,
	uint16 en_field, uint16 override_val, uint16 ovr);
extern int32 wlc_phy_tempsense_nphy_rev19(phy_info_t *pi);
extern int32 wlc_phy_tempsense_from_statusbyte_nphy_rev19(phy_info_t *pi);
extern void wlc_phy_rev3_tssisel_nphy(phy_info_t *pi, uint8 core_code, uint8 rssi_type);
/* save_radio_reg[0,1] : RADIO_20671_IQCAL_CFG1_CORE[0,1] */
extern void wlc_phy_rev3_setup_rfiqcal_mux(phy_info_t *pi, uint16 attenuator, bool override,
	uint8 tssi0_iqcal1);

extern void wlc_phy_noise_home_channel_nphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_gain_override_enable_disable(phy_info_t *pi, bool enable);
extern void wlc_phy_rxfe_ctrl_nphy(phy_info_t *pi);
extern void wlc_nphy_btc_adjust(phy_info_t *pi, bool btactive);
#ifdef TWO_PWR_RANGE
extern void wlc_phy_get_paparams_lo_nphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1);
#endif // endif
extern void wlc_phy_cck_acpr_war_nphy(phy_info_t *pi);
extern void wlc_phy_program_rx_initgain_rfseq_nphy(phy_info_t *pi, uint32 init_gain);
extern void wlc_phy_lq_based_crsminpwr_limiting_nphy(phy_info_t *pi, uint16 *crsminpwr);
#endif	/* _wlc_phy_extended_n_h_ */
