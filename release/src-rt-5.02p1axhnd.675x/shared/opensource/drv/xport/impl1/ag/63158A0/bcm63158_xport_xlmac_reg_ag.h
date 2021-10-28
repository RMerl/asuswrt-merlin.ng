/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#ifndef _BCM63158_XPORT_XLMAC_REG_AG_H_
#define _BCM63158_XPORT_XLMAC_REG_AG_H_

//#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_xlmac_reg_indir_acc_addr_0;

typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_xlmac_reg_indir_acc_addr_1;

typedef struct
{
    uint8_t xlmac_reset;
    uint8_t rx_dual_cycle_tdm_en;
    uint8_t rx_non_linear_quad_tdm_en;
    uint8_t rx_flex_tdm_enable;
    uint8_t mac_mode;
    uint8_t osts_timer_disable;
    uint8_t bypass_osts;
    uint8_t egr_1588_timestamping_mode;
} xport_xlmac_reg_config;

typedef struct
{
    uint8_t read_threshold;
    uint8_t tx_port_id;
    uint8_t tx_port_sel;
    uint8_t rxerr_en;
    uint8_t tx_crc_err;
    uint8_t tx_crc_mode;
    uint8_t rmt_loopback_en;
} xport_xlmac_reg_rmt_lpbk_cntrl;

int ag_drv_xport_xlmac_reg_dir_acc_data_write_set(uint32_t write_data);
int ag_drv_xport_xlmac_reg_dir_acc_data_write_get(uint32_t *write_data);
int ag_drv_xport_xlmac_reg_dir_acc_data_read_set(uint32_t read_data);
int ag_drv_xport_xlmac_reg_dir_acc_data_read_get(uint32_t *read_data);
int ag_drv_xport_xlmac_reg_indir_acc_addr_0_set(const xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_xlmac_reg_indir_acc_addr_0_get(xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_set(uint32_t data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_get(uint32_t *data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_set(uint32_t data_high);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_get(uint32_t *data_high);
int ag_drv_xport_xlmac_reg_indir_acc_addr_1_set(const xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_xlmac_reg_indir_acc_addr_1_get(xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_set(uint32_t data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_get(uint32_t *data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_set(uint32_t data_high);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_get(uint32_t *data_high);
int ag_drv_xport_xlmac_reg_config_set(const xport_xlmac_reg_config *config);
int ag_drv_xport_xlmac_reg_config_get(xport_xlmac_reg_config *config);
int ag_drv_xport_xlmac_reg_interrupt_check_set(uint8_t xlmac_intr_check);
int ag_drv_xport_xlmac_reg_interrupt_check_get(uint8_t *xlmac_intr_check);
int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_set(uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_get(uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_set(uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_get(uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_set(uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_get(uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_set(uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_get(uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_set(const xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl);
int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_get(xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_xlmac_reg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

