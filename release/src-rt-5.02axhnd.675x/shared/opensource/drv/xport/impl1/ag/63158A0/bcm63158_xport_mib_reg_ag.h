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

#ifndef _BCM63158_XPORT_MIB_REG_AG_H_
#define _BCM63158_XPORT_MIB_REG_AG_H_

//#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_mib_reg_indir_acc_addr_0;

typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_mib_reg_indir_acc_addr_1;

typedef struct
{
    uint8_t force_tx_mem3_serr;
    uint8_t force_tx_mem2_serr;
    uint8_t force_tx_mem1_serr;
    uint8_t force_tx_mem0_serr;
    uint8_t force_rx_mem4_serr;
    uint8_t force_rx_mem3_serr;
    uint8_t force_rx_mem2_serr;
    uint8_t force_rx_mem1_serr;
    uint8_t force_rx_mem0_serr;
} xport_mib_reg_force_sb_ecc_err;

typedef struct
{
    uint8_t force_tx_mem3_derr;
    uint8_t force_tx_mem2_derr;
    uint8_t force_tx_mem1_derr;
    uint8_t force_tx_mem0_derr;
    uint8_t force_rx_mem4_derr;
    uint8_t force_rx_mem3_derr;
    uint8_t force_rx_mem2_derr;
    uint8_t force_rx_mem1_derr;
    uint8_t force_rx_mem0_derr;
} xport_mib_reg_force_db_ecc_err;

int ag_drv_xport_mib_reg_dir_acc_data_write_set(uint32_t write_data);
int ag_drv_xport_mib_reg_dir_acc_data_write_get(uint32_t *write_data);
int ag_drv_xport_mib_reg_dir_acc_data_read_set(uint32_t read_data);
int ag_drv_xport_mib_reg_dir_acc_data_read_get(uint32_t *read_data);
int ag_drv_xport_mib_reg_indir_acc_addr_0_set(const xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_mib_reg_indir_acc_addr_0_get(xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_mib_reg_indir_acc_data_low_0_set(uint32_t data_low);
int ag_drv_xport_mib_reg_indir_acc_data_low_0_get(uint32_t *data_low);
int ag_drv_xport_mib_reg_indir_acc_data_high_0_set(uint32_t data_high);
int ag_drv_xport_mib_reg_indir_acc_data_high_0_get(uint32_t *data_high);
int ag_drv_xport_mib_reg_indir_acc_addr_1_set(const xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_mib_reg_indir_acc_addr_1_get(xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_mib_reg_indir_acc_data_low_1_set(uint32_t data_low);
int ag_drv_xport_mib_reg_indir_acc_data_low_1_get(uint32_t *data_low);
int ag_drv_xport_mib_reg_indir_acc_data_high_1_set(uint32_t data_high);
int ag_drv_xport_mib_reg_indir_acc_data_high_1_get(uint32_t *data_high);
int ag_drv_xport_mib_reg_cntrl_set(uint8_t eee_cnt_mode, uint8_t saturate_en, uint8_t cor_en, uint8_t cnt_rst);
int ag_drv_xport_mib_reg_cntrl_get(uint8_t *eee_cnt_mode, uint8_t *saturate_en, uint8_t *cor_en, uint8_t *cnt_rst);
int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_set(uint8_t cnt);
int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_get(uint8_t *cnt);
int ag_drv_xport_mib_reg_gport0_max_pkt_size_set(uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport0_max_pkt_size_get(uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport1_max_pkt_size_set(uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport1_max_pkt_size_get(uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport2_max_pkt_size_set(uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport2_max_pkt_size_get(uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport3_max_pkt_size_set(uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport3_max_pkt_size_get(uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_ecc_cntrl_set(uint8_t tx_mib_ecc_en, uint8_t rx_mib_ecc_en);
int ag_drv_xport_mib_reg_ecc_cntrl_get(uint8_t *tx_mib_ecc_en, uint8_t *rx_mib_ecc_en);
int ag_drv_xport_mib_reg_force_sb_ecc_err_set(const xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err);
int ag_drv_xport_mib_reg_force_sb_ecc_err_get(xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err);
int ag_drv_xport_mib_reg_force_db_ecc_err_set(const xport_mib_reg_force_db_ecc_err *force_db_ecc_err);
int ag_drv_xport_mib_reg_force_db_ecc_err_get(xport_mib_reg_force_db_ecc_err *force_db_ecc_err);
int ag_drv_xport_mib_reg_rx_mem0_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem1_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem2_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem3_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem4_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem0_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem1_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem2_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem3_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_mib_reg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

