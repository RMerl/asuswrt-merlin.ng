/*
   Copyright (c) 2015 Broadcom
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

#ifndef _BCM4912_XPORT_MIB_REG_AG_H_
#define _BCM4912_XPORT_MIB_REG_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"

/**************************************************************************************************/
/* err:  - Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1)  */
/*      and this bit is set it indicates that register transaction completed with error.          */
/* start_busy:  - START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indire */
/*             ct register read/write transaction. When transaction completes hardware clears thi */
/*             s bit.                                                                             */
/* r_w:  - Register transaction:
0 : Register Write.
'1 : Register Read.
                         */
/* reg_port_id:  - Register Port ID.                                                              */
/* reg_offset:  - Register offset.
Note: Bit 7 is ignored by HW. Write it as 0.                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_mib_reg_indir_acc_addr_0;


/**************************************************************************************************/
/* err:  - Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1)  */
/*      and this bit is set it indicates that register transaction completed with error.          */
/* start_busy:  - START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indire */
/*             ct register read/write transaction. When transaction completes hardware clears thi */
/*             s bit.                                                                             */
/* r_w:  - Register transaction:
0 : Register Write.
'1 : Register Read.
                         */
/* reg_port_id:  - Register Port ID.                                                              */
/* reg_offset:  - Register offset.
Note: Bit 7 is ignored by HW. Write it as 0.                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_mib_reg_indir_acc_addr_1;


/**************************************************************************************************/
/* force_tx_mem3_serr:  - Self-clearing. Force Tx MIB memory instance 3 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_tx_mem2_serr:  - Self-clearing. Force Tx MIB memory instance 2 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_tx_mem1_serr:  - Self-clearing. Force Tx MIB memory instance 1 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_tx_mem0_serr:  - Self-clearing. Force Tx MIB memory instance 0 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_rx_mem4_serr:  - Self-clearing. Force Rx MIB memory instance 4 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_rx_mem3_serr:  - Self-clearing. Force Rx MIB memory instance 3 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_rx_mem2_serr:  - Self-clearing. Force Rx MIB memory instance 2 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_rx_mem1_serr:  - Self-clearing. Force Rx MIB memory instance 1 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/* force_rx_mem0_serr:  - Self-clearing. Force Rx MIB memory instance 0 single bit ECC error. Do  */
/*                     not assert together with force double bit ECC error.                       */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* force_tx_mem3_derr:  - Self-clearing. Force Tx MIB memory instance 3 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_tx_mem2_derr:  - Self-clearing. Force Tx MIB memory instance 2 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_tx_mem1_derr:  - Self-clearing. Force Tx MIB memory instance 1 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_tx_mem0_derr:  - Self-clearing. Force Tx MIB memory instance 0 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_rx_mem4_derr:  - Self-clearing. Force Rx MIB memory instance 4 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_rx_mem3_derr:  - Self-clearing. Force Rx MIB memory instance 3 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_rx_mem2_derr:  - Self-clearing. Force Rx MIB memory instance 2 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_rx_mem1_derr:  - Self-clearing. Force Rx MIB memory instance 1 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/* force_rx_mem0_derr:  - Self-clearing. Force Rx MIB memory instance 0 double bit ECC error. Do  */
/*                     not assert together with force single bit ECC error.                       */
/**************************************************************************************************/
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

int ag_drv_xport_mib_reg_dir_acc_data_write_set(uint8_t xlmac_id, uint32_t write_data);
int ag_drv_xport_mib_reg_dir_acc_data_write_get(uint8_t xlmac_id, uint32_t *write_data);
int ag_drv_xport_mib_reg_dir_acc_data_read_get(uint8_t xlmac_id, uint32_t *read_data);
int ag_drv_xport_mib_reg_indir_acc_addr_0_set(uint8_t xlmac_id, const xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_mib_reg_indir_acc_addr_0_get(uint8_t xlmac_id, xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_mib_reg_indir_acc_data_low_0_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xport_mib_reg_indir_acc_data_low_0_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xport_mib_reg_indir_acc_data_high_0_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xport_mib_reg_indir_acc_data_high_0_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xport_mib_reg_indir_acc_addr_1_set(uint8_t xlmac_id, const xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_mib_reg_indir_acc_addr_1_get(uint8_t xlmac_id, xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_mib_reg_indir_acc_data_low_1_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xport_mib_reg_indir_acc_data_low_1_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xport_mib_reg_indir_acc_data_high_1_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xport_mib_reg_indir_acc_data_high_1_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xport_mib_reg_control_set(uint8_t xlmac_id, uint8_t eee_cnt_mode, uint8_t saturate_en, uint8_t cor_en, uint8_t cnt_rst);
int ag_drv_xport_mib_reg_control_get(uint8_t xlmac_id, uint8_t *eee_cnt_mode, uint8_t *saturate_en, uint8_t *cor_en, uint8_t *cnt_rst);
int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_set(uint8_t xlmac_id, uint8_t cnt);
int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_get(uint8_t xlmac_id, uint8_t *cnt);
int ag_drv_xport_mib_reg_gport0_max_pkt_size_set(uint8_t xlmac_id, uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport0_max_pkt_size_get(uint8_t xlmac_id, uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport1_max_pkt_size_set(uint8_t xlmac_id, uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport1_max_pkt_size_get(uint8_t xlmac_id, uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport2_max_pkt_size_set(uint8_t xlmac_id, uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport2_max_pkt_size_get(uint8_t xlmac_id, uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_gport3_max_pkt_size_set(uint8_t xlmac_id, uint16_t max_pkt_size);
int ag_drv_xport_mib_reg_gport3_max_pkt_size_get(uint8_t xlmac_id, uint16_t *max_pkt_size);
int ag_drv_xport_mib_reg_ecc_cntrl_set(uint8_t xlmac_id, uint8_t tx_mib_ecc_en, uint8_t rx_mib_ecc_en);
int ag_drv_xport_mib_reg_ecc_cntrl_get(uint8_t xlmac_id, uint8_t *tx_mib_ecc_en, uint8_t *rx_mib_ecc_en);
int ag_drv_xport_mib_reg_force_sb_ecc_err_set(uint8_t xlmac_id, const xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err);
int ag_drv_xport_mib_reg_force_sb_ecc_err_get(uint8_t xlmac_id, xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err);
int ag_drv_xport_mib_reg_force_db_ecc_err_set(uint8_t xlmac_id, const xport_mib_reg_force_db_ecc_err *force_db_ecc_err);
int ag_drv_xport_mib_reg_force_db_ecc_err_get(uint8_t xlmac_id, xport_mib_reg_force_db_ecc_err *force_db_ecc_err);
int ag_drv_xport_mib_reg_rx_mem0_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_rx_mem0_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem1_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_rx_mem1_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem2_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_rx_mem2_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem3_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_rx_mem3_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_rx_mem4_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_rx_mem4_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem0_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_tx_mem0_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem1_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_tx_mem1_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem2_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_tx_mem2_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);
int ag_drv_xport_mib_reg_tx_mem3_ecc_status_set(uint8_t xlmac_id, uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err);
int ag_drv_xport_mib_reg_tx_mem3_ecc_status_get(uint8_t xlmac_id, uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_mib_reg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

