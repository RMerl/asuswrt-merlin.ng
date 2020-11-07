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

#ifndef _WAN_SERDES_AG_H_
#define _WAN_SERDES_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    bdmf_boolean pll0_refin_en;
    bdmf_boolean pll0_refout_en;
    bdmf_boolean pll0_lcref_sel;
    bdmf_boolean pll1_refin_en;
    bdmf_boolean pll1_refout_en;
    bdmf_boolean pll1_lcref_sel;
} wan_serdes_wan_serdes_pll_ctl;

typedef struct
{
    uint8_t top_osr_control_cfg_gpon_rx_clk;
    bdmf_boolean top_osr_control_txfifo_rd_legacy_mode;
    bdmf_boolean top_osr_control_txlbe_ser_en;
    uint8_t top_osr_control_txlbe_ser_init_val;
    bdmf_boolean top_osr_control_txlbe_ser_order;
} wan_serdes_top_osr_ctrl;

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pll_ctl_set(const wan_serdes_wan_serdes_pll_ctl *wan_serdes_pll_ctl);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pll_ctl_get(wan_serdes_wan_serdes_pll_ctl *wan_serdes_pll_ctl);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_temp_ctl_get(uint16_t *wan_temperature_read);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_ctl_set(bdmf_boolean pram_go, bdmf_boolean pram_we, uint16_t pram_address);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_ctl_get(bdmf_boolean *pram_go, bdmf_boolean *pram_we, uint16_t *pram_address);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_low_set(uint32_t val);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_low_get(uint32_t *val);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_high_set(uint32_t val);
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_high_get(uint32_t *val);
bdmf_error_t ag_drv_wan_serdes_top_osr_ctrl_set(const wan_serdes_top_osr_ctrl *top_osr_ctrl);
bdmf_error_t ag_drv_wan_serdes_top_osr_ctrl_get(wan_serdes_top_osr_ctrl *top_osr_ctrl);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_wan_serdes_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

