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

#ifndef _BCM6858_XLMAC_CONF_AG_H_
#define _BCM6858_XLMAC_CONF_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xlmac_conf_indir_acc_addr_0;

typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xlmac_conf_indir_acc_addr_1;

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
} xlmac_conf_config;

int ag_drv_xlmac_conf_dir_acc_data_write_set(uint8_t xlmac_id, uint32_t write_data);
int ag_drv_xlmac_conf_dir_acc_data_write_get(uint8_t xlmac_id, uint32_t *write_data);
int ag_drv_xlmac_conf_dir_acc_data_read_set(uint8_t xlmac_id, uint32_t read_data);
int ag_drv_xlmac_conf_dir_acc_data_read_get(uint8_t xlmac_id, uint32_t *read_data);
int ag_drv_xlmac_conf_indir_acc_addr_0_set(uint8_t xlmac_id, const xlmac_conf_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xlmac_conf_indir_acc_addr_0_get(uint8_t xlmac_id, xlmac_conf_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xlmac_conf_indir_acc_data_low_0_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xlmac_conf_indir_acc_data_low_0_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xlmac_conf_indir_acc_data_high_0_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xlmac_conf_indir_acc_data_high_0_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xlmac_conf_indir_acc_addr_1_set(uint8_t xlmac_id, const xlmac_conf_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xlmac_conf_indir_acc_addr_1_get(uint8_t xlmac_id, xlmac_conf_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xlmac_conf_indir_acc_data_low_1_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xlmac_conf_indir_acc_data_low_1_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xlmac_conf_indir_acc_data_high_1_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xlmac_conf_indir_acc_data_high_1_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xlmac_conf_config_set(uint8_t xlmac_id, const xlmac_conf_config *config);
int ag_drv_xlmac_conf_config_get(uint8_t xlmac_id, xlmac_conf_config *config);
int ag_drv_xlmac_conf_interrupt_check_set(uint8_t xlmac_id, uint8_t xlmac_intr_check);
int ag_drv_xlmac_conf_interrupt_check_get(uint8_t xlmac_id, uint8_t *xlmac_intr_check);
int ag_drv_xlmac_conf_port_0_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xlmac_conf_port_0_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xlmac_conf_port_1_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xlmac_conf_port_1_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xlmac_conf_port_2_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xlmac_conf_port_2_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xlmac_conf_port_3_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xlmac_conf_port_3_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xlmac_conf_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

