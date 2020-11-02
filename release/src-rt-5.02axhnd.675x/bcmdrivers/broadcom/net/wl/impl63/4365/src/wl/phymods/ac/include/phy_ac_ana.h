/*
 * ACPHY ANAcore control module interface (to other PHY modules).
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

#ifndef _phy_ac_ana_h_
#define _phy_ac_ana_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_ana.h>

/* forward declaration */
typedef struct phy_ac_ana_info phy_ac_ana_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_ana_info_t *phy_ac_ana_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_ana_info_t *ani);
void phy_ac_ana_unregister_impl(phy_ac_ana_info_t *info);

extern uint16 sdadc_cfg20;
extern uint16 sdadc_cfg40;
/* High SNR 40M */
extern uint16 sdadc_cfg40hs;
extern uint16 sdadc_cfg80;

/**
 * Front End Control table contents for a single radio core. For some chips, the per-core tables are
 * not of equal size. A single table in PHYHW contains a 'subtable' per radio core. This structure
 * is not used in the case of 'sparse' tables.
 */
typedef struct acphy_fe_ctrl_table {
	uint8 *subtable;	/* containing values to copy into PHY hardware table */
	uint32 n_entries;	/* number of entries in the subtable */
	uint32 hw_offset;	/* index to write to in the HW table */
} acphy_fe_ctrl_table_t;

extern void wlc_phy_init_adc_read(phy_info_t *pi, uint16 *save_afePuCtrl, uint16 *save_gpio,
	uint32 *save_chipc, uint16 *fval2g_orig, uint16 *fval5g_orig,
                                  uint16 *fval2g, uint16 *fval5g, uint8 *stall_val,
                                  uint16 *save_gpioHiOutEn);

extern void wlc_phy_restore_after_adc_read(phy_info_t *pi, uint16 *save_afePuCtrl,
	uint16 *save_gpio, uint32 *save_chipc,
	uint16 *fval2g_orig, uint16 *fval5g_orig,
                                           uint16 *fval2g, uint16 *fval5g, uint8 *stall_val,
                                           uint16 *save_gpioHiOutEn);

extern void wlc_phy_pulse_adc_reset_acphy(phy_info_t *pi);
extern void wlc_tiny_dc_static_WAR(phy_info_t *pi);

#endif /* _phy_ac_ana_h_ */
