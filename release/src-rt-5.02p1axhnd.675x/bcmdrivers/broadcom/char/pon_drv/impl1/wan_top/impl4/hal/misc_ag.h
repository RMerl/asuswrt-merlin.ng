/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifndef _MISC_AG_H_
#define _MISC_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint16_t cr_xgwan_top_wan_misc_pmd_lane_mode;
    uint8_t cr_xgwan_top_wan_misc_onu2g_phya;
    uint8_t cr_xgwan_top_wan_misc_mdio_fast_mode;
    uint8_t cr_xgwan_top_wan_misc_mdio_mode;
    uint8_t cr_xgwan_top_wan_misc_refout_en;
    uint8_t cr_xgwan_top_wan_misc_refin_en;
    uint8_t cr_xgwan_top_wan_misc_onu2g_pmd_status_sel;
} misc_misc_0;

typedef struct
{
    uint8_t cr_xgwan_top_wan_misc_pmd_rx_osr_mode;
    uint8_t cr_xgwan_top_wan_misc_pmd_tx_mode;
    uint8_t cr_xgwan_top_wan_misc_pmd_tx_osr_mode;
    uint8_t cr_xgwan_top_wan_misc_pmd_tx_disable;
    uint8_t cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn;
    uint8_t cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn;
    uint8_t cr_xgwan_top_wan_misc_pmd_ext_los;
    uint8_t cr_xgwan_top_wan_misc_pmd_por_h_rstb;
    uint8_t cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb;
    uint8_t cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb;
    uint8_t cr_xgwan_top_wan_misc_pmd_ln_h_rstb;
    uint8_t cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb;
    uint8_t cr_xgwan_top_wan_misc_pmd_rx_mode;
} misc_misc_2;

typedef struct
{
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel;
    uint8_t cfg_ntr_periph_pulse_bypass;
    uint8_t cfg_ntr_gpio_pulse_bypass;
    uint8_t cfg_ntr_src;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_laser_oe;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_laser_mode;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_laser_invert;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_mem_reb;
} misc_misc_3;

typedef struct
{
    uint16_t cfg_ntr_pulse_width;
    /* FIXME! TBD */
} misc_misc_4;

int ag_drv_misc_misc_0_set(const misc_misc_0 *misc_0);
int ag_drv_misc_misc_0_get(misc_misc_0 *misc_0);
int ag_drv_misc_misc_1_set(uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode);
int ag_drv_misc_misc_1_get(uint16_t *cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t *cr_xgwan_top_wan_misc_pmd_core_0_mode);
int ag_drv_misc_misc_2_set(const misc_misc_2 *misc_2);
int ag_drv_misc_misc_2_get(misc_misc_2 *misc_2);
int ag_drv_misc_misc_3_set(const misc_misc_3 *misc_3);
int ag_drv_misc_misc_3_get(misc_misc_3 *misc_3);
int ag_drv_misc_misc_4_set(const misc_misc_4 *misc_4);
int ag_drv_misc_misc_4_get(misc_misc_4 *misc_4);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_misc_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

