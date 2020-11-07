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
    bdmf_boolean onu2g_pmd_status_sel;
    bdmf_boolean refin_en;
    bdmf_boolean refout_en;
    bdmf_boolean mdio_mode;
    bdmf_boolean mdio_fast_mode;
    bdmf_boolean epon_tx_fifo_off_ld;
    uint8_t onu2g_phya;
    bdmf_boolean epon_gbox_pon_rx_width_mode;
    bdmf_boolean epon_ae_2p5_full_rate_mode;
    uint16_t pmd_lane_mode;
} misc_misc_0;

typedef struct
{
    bdmf_boolean pmd_rx_mode;
    bdmf_boolean pmd_ln_dp_h_rstb;
    bdmf_boolean pmd_ln_h_rstb;
    bdmf_boolean pmd_core_0_dp_h_rstb;
    bdmf_boolean pmd_core_1_dp_h_rstb;
    bdmf_boolean pmd_por_h_rstb;
    bdmf_boolean pmd_ext_los;
    bdmf_boolean pmd_ln_tx_h_pwrdn;
    bdmf_boolean pmd_ln_rx_h_pwrdn;
    bdmf_boolean pmd_tx_disable;
    uint8_t pmd_tx_osr_mode;
    uint8_t pmd_tx_mode;
    uint8_t pmd_rx_osr_mode;
    bdmf_boolean cfgactiveethernet2p5;
    uint8_t cfgngpontxclk;
    uint8_t cfgngponrxclk;
    bdmf_boolean cfg_apm_mux_sel_0;
    bdmf_boolean cfg_apm_mux_sel_1;
} misc_misc_2;

typedef struct
{
    bdmf_boolean mem_reb;
    bdmf_boolean laser_invert;
    uint8_t laser_mode;
    bdmf_boolean wan_interface_select_hs;
    bdmf_boolean wan_interface_select;
    bdmf_boolean laser_oe;
    uint8_t ntr_sync_period_sel;
    uint8_t wan_debug_sel;
    uint8_t epon_debug_sel;
    uint8_t epon_tx_fifo_off;
    bdmf_boolean epon_power_zone_sel;
} misc_misc_3;

bdmf_error_t ag_drv_misc_misc_0_set(const misc_misc_0 *misc_0);
bdmf_error_t ag_drv_misc_misc_0_get(misc_misc_0 *misc_0);
bdmf_error_t ag_drv_misc_misc_1_set(uint16_t pmd_core_1_mode, uint16_t pmd_core_0_mode);
bdmf_error_t ag_drv_misc_misc_1_get(uint16_t *pmd_core_1_mode, uint16_t *pmd_core_0_mode);
bdmf_error_t ag_drv_misc_misc_2_set(const misc_misc_2 *misc_2);
bdmf_error_t ag_drv_misc_misc_2_get(misc_misc_2 *misc_2);
bdmf_error_t ag_drv_misc_misc_3_set(const misc_misc_3 *misc_3);
bdmf_error_t ag_drv_misc_misc_3_get(misc_misc_3 *misc_3);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_misc_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

