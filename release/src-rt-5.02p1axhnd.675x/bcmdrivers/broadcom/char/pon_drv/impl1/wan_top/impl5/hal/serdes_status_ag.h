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

#ifndef _SERDES_STATUS_AG_H_
#define _SERDES_STATUS_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint8_t pll0_refin_en;
    uint8_t pll0_refout_en;
    uint8_t pll0_lcref_sel;
    uint8_t pll1_refin_en;
    uint8_t pll1_refout_en;
    uint8_t pll1_lcref_sel;
} serdes_status_pll_ctl;

typedef struct
{
    uint8_t cfg_gpon_rx_clk;
    uint8_t txfifo_rd_legacy_mode;
    uint8_t txlbe_ser_en;
    uint8_t txlbe_ser_init_val;
    uint8_t txlbe_ser_order;
} serdes_status_oversample_ctrl;

int ag_drv_serdes_status_pll_ctl_set(const serdes_status_pll_ctl *pll_ctl);
int ag_drv_serdes_status_pll_ctl_get(serdes_status_pll_ctl *pll_ctl);
int ag_drv_serdes_status_temp_ctl_get(uint16_t *wan_temperature_read);
int ag_drv_serdes_status_pram_ctl_set(uint16_t pram_address, uint8_t pram_we, uint8_t pram_go);
int ag_drv_serdes_status_pram_ctl_get(uint16_t *pram_address, uint8_t *pram_we, uint8_t *pram_go);
int ag_drv_serdes_status_pram_val_low_set(uint32_t val);
int ag_drv_serdes_status_pram_val_low_get(uint32_t *val);
int ag_drv_serdes_status_pram_val_high_set(uint32_t val);
int ag_drv_serdes_status_pram_val_high_get(uint32_t *val);
int ag_drv_serdes_status_oversample_ctrl_set(const serdes_status_oversample_ctrl *oversample_ctrl);
int ag_drv_serdes_status_oversample_ctrl_get(serdes_status_oversample_ctrl *oversample_ctrl);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_serdes_status_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

