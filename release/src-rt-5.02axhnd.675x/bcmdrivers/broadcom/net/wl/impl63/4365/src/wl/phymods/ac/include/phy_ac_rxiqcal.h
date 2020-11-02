/*
 * ACPHY RXIQ CAL module interface (to other PHY modules).
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

#ifndef _phy_ac_rxiqcal_h_
#define _phy_ac_rxiqcal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_rxiqcal.h>

/* forward declaration */
typedef struct phy_ac_rxiqcal_info phy_ac_rxiqcal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_rxiqcal_info_t *phy_ac_rxiqcal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_rxiqcal_info_t *mi);
void phy_ac_rxiqcal_unregister_impl(phy_ac_rxiqcal_info_t *info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
typedef struct _acphy_2069_rxcal_radioregs {
	bool   is_orig;
	uint16 rf_2069_txrx2g_cal_tx[PHY_CORE_MAX];
	uint16 rf_2069_txrx5g_cal_tx[PHY_CORE_MAX];
	uint16 rf_2069_txrx2g_cal_rx[PHY_CORE_MAX];
	uint16 rf_2069_txrx5g_cal_rx[PHY_CORE_MAX];
	uint16 rf_2069_rxrf2g_cfg2[PHY_CORE_MAX];
	uint16 rf_2069_rxrf5g_cfg2[PHY_CORE_MAX];
} acphy_2069_rxcal_radioregs_t;

typedef struct _acphy_tiny_rxcal_radioregs {
	bool   is_orig;
	uint16 rf_tiny_rfrx_top_2g_ovr_east[PHY_CORE_MAX];
} acphy_tiny_rxcal_radioregs_t;

typedef struct _acphy_rxcal_phyregs {
	bool   is_orig;
	uint16 RfctrlOverrideTxPus [PHY_CORE_MAX];
	uint16 RfctrlCoreTxPus [PHY_CORE_MAX];
	uint16 RfctrlOverrideRxPus [PHY_CORE_MAX];
	uint16 RfctrlCoreRxPus [PHY_CORE_MAX];
	uint16 RfctrlOverrideGains [PHY_CORE_MAX];
	uint16 Dac_gain [PHY_CORE_MAX];
	uint16 forceFront [PHY_CORE_MAX];
	uint16 RfctrlCoreTXGAIN1 [PHY_CORE_MAX];
	uint16 RfctrlCoreTXGAIN2 [PHY_CORE_MAX];
	uint16 RfctrlCoreRXGAIN1 [PHY_CORE_MAX];
	uint16 RfctrlCoreRXGAIN2 [PHY_CORE_MAX];
	uint16 RfctrlCoreLpfGain [PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfCT [PHY_CORE_MAX];
	uint16 RfctrlCoreLpfCT [PHY_CORE_MAX];
	uint16 RfctrlCoreLpfGmult [PHY_CORE_MAX];
	uint16 RfctrlCoreRCDACBuf [PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfSwtch [PHY_CORE_MAX];
	uint16 RfctrlCoreLpfSwtch [PHY_CORE_MAX];
	uint16 RfctrlOverrideAfeCfg [PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg1 [PHY_CORE_MAX];
	uint16 RfctrlCoreAfeCfg2 [PHY_CORE_MAX];
	uint16 RfctrlOverrideLowPwrCfg [PHY_CORE_MAX];
	uint16 RfctrlCoreLowPwr [PHY_CORE_MAX];
	uint16 RfctrlOverrideAuxTssi [PHY_CORE_MAX];
	uint16 RfctrlCoreAuxTssi1 [PHY_CORE_MAX];
	uint16 RfctrlCoreAuxTssi2[PHY_CORE_MAX];
	uint16 RfctrlOverrideGlobalPus;
	uint16 RfctrlCoreGlobalPus;
	uint16 RxSdFeConfig1;
	uint16 RxSdFeConfig6;
	uint16 bbmult[PHY_CORE_MAX];
	uint16 rfseq_txgain[3 * PHY_CORE_NUM_4];
	uint16 RfseqCoreActv2059;
	uint16 RfctrlIntc[PHY_CORE_NUM_4];
	uint16 PapdEnable[PHY_CORE_MAX];
	uint16 AfePuCtrl;

	uint8 txpwridx[PHY_CORE_MAX];
	uint16 lbFarrowCtrl;
	uint16 spur_can_s1_en[PHY_CORE_MAX];
	uint16 spur_can_en[PHY_CORE_MAX];
} acphy_rxcal_phyregs_t;

typedef struct acphy_rx_fdiqi_ctl_struct {
	bool forced;
	uint16 forced_val;
	bool enabled;
	int32 slope[PHY_CORE_MAX];
	uint8 leakage_comp_mode;
} acphy_rx_fdiqi_ctl_t;

extern void wlc_phy_rx_iq_comp_acphy(phy_info_t *pi, uint8 write,
	phy_iq_comp_t *pcomp, uint8 rx_core);
extern void wlc_phy_rx_fdiqi_comp_acphy(phy_info_t *pi, bool enable);
extern int  wlc_phy_cal_rx_fdiqi_acphy(phy_info_t *pi);
extern void wlc_phy_rx_iq_est_acphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs, bool rxiq_cal);
#if defined(BCMDBG)
extern void wlc_phy_force_fdiqi_acphy(phy_info_t *pi, uint16 int_val);
#endif // endif
extern void wlc_phy_dig_lpf_override_acphy(phy_info_t *pi, uint8 dig_lpf_ht);

#endif /* _phy_ac_rxiqcal_h_ */
