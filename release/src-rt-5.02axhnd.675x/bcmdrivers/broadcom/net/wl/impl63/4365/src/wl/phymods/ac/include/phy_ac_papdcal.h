/*
 * ACPHY PAPD CAL module interface (to other PHY modules).
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

#ifndef _phy_ac_papdcal_h_
#define _phy_ac_papdcal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_papdcal.h>

/* forward declaration */
typedef struct phy_ac_papdcal_info phy_ac_papdcal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_papdcal_info_t *phy_ac_papdcal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_papdcal_info_t *mi);
void phy_ac_papdcal_unregister_impl(phy_ac_papdcal_info_t *info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
extern uint16 papd_gainctrl_pga[];

/* used for PAPD cal */
typedef struct _acphy_txgains {
	uint16 txlpf;
	uint16 txgm;
	uint16 pga;
	uint16 pad;
	uint16 ipa;
} acphy_txgains_t;

#define ACPHY_PAPD_EPS_TBL_SIZE		64
extern uint32 acphy_papd_scaltbl[];

extern void wlc_phy_papd_phy_cleanup_acphy(phy_info_t *pi);
extern void wlc_phy_papd_cal_acphy(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex);
extern void wlc_phy_papd_set_rfpwrlut(phy_info_t *pi);
extern void wlc_phy_txpwr_papd_cal_run_acphy(phy_info_t *pi, uint8 tx_pre_cal_pwr_ctrl_state);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void wlc_phy_papd_dump_eps_trace_acphy(phy_info_t *pi, struct bcmstrbuf *b);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
extern void wlc_phy_papd_phy_setup_acphy(phy_info_t *pi);
extern void wlc_phy_tiny_papd_cal_run_acphy(phy_info_t *pi,
	uint8 tx_pre_cal_pwr_ctrl_state);

#endif /* _phy_ac_papdcal_h_ */
