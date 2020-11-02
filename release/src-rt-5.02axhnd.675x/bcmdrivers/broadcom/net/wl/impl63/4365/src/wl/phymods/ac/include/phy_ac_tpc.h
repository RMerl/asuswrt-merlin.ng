/*
 * ACPHY TxPowerCtrl module interface (to other PHY modules).
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

#ifndef _phy_ac_tpc_h_
#define _phy_ac_tpc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tpc.h>

/* forward declaration */
typedef struct phy_ac_tpc_info phy_ac_tpc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tpc_info_t *phy_ac_tpc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tpc_info_t *ti);
void phy_ac_tpc_unregister_impl(phy_ac_tpc_info_t *info);

void phy_ac_tpc_shortwindow_upd(phy_info_t *pi, bool new_channel);

#define TSSI_DIVWAR_INDX (2)

/* #ifdef PREASSOC_PWRCTRL */
typedef struct phy_pwr_ctrl_save_acphy {
	bool status_idx_carry_2g[PHY_CORE_MAX];
	bool status_idx_carry_5g[PHY_CORE_MAX];
	uint8 status_idx_2g[PHY_CORE_MAX];
	uint8 status_idx_5g[PHY_CORE_MAX];
	uint16 last_chan_stored_2g;
	uint16 last_chan_stored_5g;
	int8   pwr_qdbm_2g[PHY_CORE_MAX];
	int8   pwr_qdbm_5g[PHY_CORE_MAX];
	bool   stored_not_restored_2g[PHY_CORE_MAX];
	bool   stored_not_restored_5g[PHY_CORE_MAX];

} phy_pwr_ctrl_s;
/* #endif */  /* PREASSOC_PWRCTRL */

extern uint8 wlc_phy_tssi2dbm_acphy(phy_info_t *pi, int32 tssi, int32 a1, int32 b0, int32 b1);
extern void wlc_phy_get_paparams_for_band_acphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1);
extern void wlc_phy_read_txgain_acphy(phy_info_t *pi);
extern void wlc_phy_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);
extern void wlc_phy_get_txgain_settings_by_index_acphy(phy_info_t *pi,
	txgain_setting_t *txgain_settings, int8 txpwrindex);
extern void wlc_phy_get_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern void wlc_phy_set_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern uint32 wlc_phy_txpwr_idx_get_acphy(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_enable_acphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_fixpower_acphy(phy_info_t *pi);
extern void wlc_phy_txpower_sromlimit_get_acphy(phy_info_t *pi,
	chanspec_t chanspec, ppr_t *max_pwr, uint8 core);
extern void wlc_phy_txpower_sromlimit_get_srom12_acphy(phy_info_t *pi,
	chanspec_t chanspec, ppr_t *max_pwr, uint8 core);
extern void wlc_phy_txpwr_est_pwr_acphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_adj);
extern uint16 * wlc_phy_get_tx_pwrctrl_tbl_2069(phy_info_t *pi);
extern int8 wlc_phy_tone_pwrctrl(phy_info_t *pi, int8 tx_idx, uint8 core);

#ifdef PREASSOC_PWRCTRL
extern void wlc_phy_store_tx_pwrctrl_setting_acphy(phy_info_t *pi, chanspec_t previous_channel);
#endif // endif

extern void wlc_phy_txpwrctrl_set_target_acphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core);
extern void wlc_phy_txpwrctrl_config_acphy(phy_info_t *pi);
extern int wlc_phy_txpower_core_offset_set_acphy(phy_info_t *pi,
	struct phy_txcore_pwr_offsets *offsets);
extern int wlc_phy_txpower_core_offset_get_acphy(phy_info_t *pi,
	struct phy_txcore_pwr_offsets *offsets);

#if defined(WL_SARLIMIT) || defined(BCM_OL_DEV) || defined(WL_SAR_SIMPLE_CONTROL)
extern void wlc_phy_set_sarlimit_acphy(phy_info_t *pi);
#endif /* WL_SARLIMIT || BCM_OL_DEV || WL_SAR_SIMPLE_CONTROL */

#if defined(WLTEST)
extern void wlc_phy_iovar_patrim_acphy(phy_info_t *pi, int32 *ret_int_ptr);
#endif // endif
extern int8 wlc_phy_txpwrctrl_update_minpwr_acphy(phy_info_t *pi);
#endif /* _phy_ac_tpc_h_ */
