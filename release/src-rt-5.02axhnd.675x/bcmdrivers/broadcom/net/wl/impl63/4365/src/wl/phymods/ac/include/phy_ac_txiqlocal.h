/*
 * ACPHY TXIQLO CAL module interface (to other PHY modules).
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

#ifndef _phy_ac_txiqlocal_h_
#define _phy_ac_txiqlocal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_txiqlocal.h>

/* forward declaration */
typedef struct phy_ac_txiqlocal_info phy_ac_txiqlocal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_txiqlocal_info_t *phy_ac_txiqlocal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_txiqlocal_info_t *mi);
void phy_ac_txiqlocal_unregister_impl(phy_ac_txiqlocal_info_t *info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* ************************************************************************* */
/* The following definitions shared between TSSI, Cache, RxIQCal and TxIQLOCal			*/
/* ************************************************************************* */
#define ACPHY_IQCAL_TONEFREQ_80MHz 8000
#define ACPHY_IQCAL_TONEFREQ_40MHz 4000
#define ACPHY_IQCAL_TONEFREQ_20MHz 2000
#define ACPHY_TXCAL_MAX_NUM_FREQ 4

#define CAL_COEFF_READ    0
#define CAL_COEFF_WRITE   1

/* defs for iqlo cal */
enum {  /* mode selection for reading/writing tx iqlo cal coefficients */
	TB_START_COEFFS_AB, TB_START_COEFFS_D, TB_START_COEFFS_E, TB_START_COEFFS_F,
	TB_BEST_COEFFS_AB,  TB_BEST_COEFFS_D,  TB_BEST_COEFFS_E,  TB_BEST_COEFFS_F,
	TB_OFDM_COEFFS_AB,  TB_OFDM_COEFFS_D,  TB_BPHY_COEFFS_AB,  TB_BPHY_COEFFS_D,
	PI_INTER_COEFFS_AB, PI_INTER_COEFFS_D, PI_INTER_COEFFS_E, PI_INTER_COEFFS_F,
	PI_FINAL_COEFFS_AB, PI_FINAL_COEFFS_D, PI_FINAL_COEFFS_E, PI_FINAL_COEFFS_F
};

typedef struct _acphy_txcal_radioregs {
	bool   is_orig;
	uint16 iqcal_cfg1[PHY_CORE_MAX];
	uint16 pa2g_tssi[PHY_CORE_MAX];
	uint16 OVR20[PHY_CORE_MAX];
	uint16 OVR21[PHY_CORE_MAX];
	uint16 tx5g_tssi[PHY_CORE_MAX];
	uint16 iqcal_cfg2[PHY_CORE_MAX];
	uint16 iqcal_cfg3[PHY_CORE_MAX];
	uint16 auxpga_cfg1[PHY_CORE_MAX];
	uint16 iqcal_ovr1[PHY_CORE_MAX];
	uint16 tx_top_5g_ovr1[PHY_CORE_MAX];
	uint16 adc_cfg10[PHY_CORE_MAX];
	uint16 adc_cfg18[PHY_CORE_MAX];
	uint16 auxpga_ovr1[PHY_CORE_MAX];
	uint16 testbuf_ovr1[PHY_CORE_MAX];
	uint16 spare_cfg6[PHY_CORE_MAX];
	uint16 pa2g_cfg1[PHY_CORE_MAX];
	uint16 adc_ovr1[PHY_CORE_MAX];
	uint16 tx5g_misc_cfg1[PHY_CORE_MAX];
	uint16 tx2g_misc_cfg1[PHY_CORE_MAX];
	uint16 testbuf_cfg1[PHY_CORE_MAX];
	uint16 tia_cfg5[PHY_CORE_MAX];
	uint16 tia_cfg9[PHY_CORE_MAX];
	uint16 auxpga_vmid[PHY_CORE_MAX];
	uint16 pmu_cfg4[PHY_CORE_MAX];
	uint16 tx_top_2g_ovr_north[PHY_CORE_MAX];
	uint16 tx_top_2g_ovr_east[PHY_CORE_MAX];
	uint16 pmu_ovr[PHY_CORE_MAX];
	uint16 tx_top_5g_ovr2[PHY_CORE_MAX];
	uint16 txmix5g_cfg2[PHY_CORE_MAX];
	uint16 pad5g_cfg1[PHY_CORE_MAX];
	uint16 pa5g_cfg1[PHY_CORE_MAX];
} acphy_txcal_radioregs_t;

typedef struct _acphy_txcal_phyregs {
	bool   is_orig;
	uint16 BBConfig;
	uint16 RxFeCtrl1;
	uint16 AfePuCtrl;

	uint16 RfctrlOverrideAfeCfg[PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg1[PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg2[PHY_CORE_MAX];
	uint16 RfctrlIntc[PHY_CORE_MAX];
	uint16 RfctrlOverrideRxPus[PHY_CORE_MAX];
	uint16 RfctrlCoreRxPus[PHY_CORE_MAX];
	uint16 RfctrlOverrideTxPus[PHY_CORE_MAX];
	uint16 RfctrlCoreTxPus[PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfSwtch[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfSwtch[PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfCT[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfCT[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfGmult[PHY_CORE_MAX];
	uint16 RfctrlCoreRCDACBuf[PHY_CORE_MAX];
	uint16 RfctrlOverrideAuxTssi[PHY_CORE_MAX];
	uint16 RfctrlCoreAuxTssi1[PHY_CORE_MAX];
	uint16 PapdEnable[PHY_CORE_MAX];
	uint16 RfseqCoreActv2059;
} acphy_txcal_phyregs_t;

typedef struct acphy_tx_fdiqi_ctl_struct {
	int8 slope[PHY_CORE_MAX];
	bool enabled;
} acphy_tx_fdiqi_ctl_t;

typedef struct acphy_fdiqi_struct {
	int32 freq;
	int32 angle[PHY_CORE_MAX];
	int32 mag[PHY_CORE_MAX];
} acphy_fdiqi_t;

extern void wlc_phy_poll_samps_WAR_acphy(phy_info_t *pi, int16 *samp, bool is_tssi,
                                         bool for_idle, txgain_setting_t *target_gains,
                                         bool for_iqcal, bool init_adc_inside, uint16 core,
                                         bool champ);
extern void wlc_phy_cal_txiqlo_coeffs_acphy(phy_info_t *pi, uint8 rd_wr, uint16 *coeff_vals,
	uint8 select, uint8 core);
extern void wlc_phy_ipa_set_bbmult_acphy(phy_info_t *pi, uint16 *m0, uint16 *m1,
	uint16 *m2, uint16 *m3, uint8 coremask);
extern void wlc_phy_precal_txgain_acphy(phy_info_t *pi, txgain_setting_t *target_gains);
extern int wlc_phy_cal_txiqlo_acphy(phy_info_t *pi, uint8 searchmode, uint8 mphase, uint8 Biq2byp);
extern void wlc_acphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
extern void wlc_acphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b);
extern void wlc_acphy_set_tx_locc(phy_info_t *pi, uint16 didq);
extern uint16 wlc_acphy_get_tx_locc(phy_info_t *pi);
extern uint16 wlc_phy_set_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);
extern void wlc_phy_tx_fdiqi_comp_acphy(phy_info_t *pi, bool enable, int fdiq_data_valid);
extern void wlc_phy_fdiqi_lin_reg_acphy(phy_info_t *pi, acphy_fdiqi_t *freq_ang_mag,
                                        uint16 num_data, int fdiq_data_valid);

#endif /* _phy_ac_txiqlocal_h_ */
