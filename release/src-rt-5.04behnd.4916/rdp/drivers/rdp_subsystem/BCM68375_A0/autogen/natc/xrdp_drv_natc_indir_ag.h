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


#ifndef _XRDP_DRV_NATC_INDIR_AG_H_
#define _XRDP_DRV_NATC_INDIR_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * natc_entry: 
 *     NAT Cache Entry number.
 * w_r: 
 *     NAT Cache Memory and Statics Memory Transaction.1 : NAT Cache Memory and Statics Memory Write.0 : NAT Cache
 *     Memory and Statics Memory Read.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_indir_addr_reg_set(uint16_t natc_entry, bdmf_boolean w_r);
bdmf_error_t ag_drv_natc_indir_addr_reg_get(uint16_t *natc_entry, bdmf_boolean *w_r);

/**********************************************************************************************************************
 * flow_cntr_entry: 
 *     Flow counter address.
 * w_r: 
 *     Flow counter Transaction.1 : Flow counter write.0 : Flow counter read.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_set(uint8_t flow_cntr_entry, bdmf_boolean w_r);
bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_get(uint8_t *flow_cntr_entry, bdmf_boolean *w_r);

/**********************************************************************************************************************
 * data: 
 *     Flow Counter indirect register access data register,
 *     bits[31:0].-------------------------------------------------------For Flow Counter write operation,first, write
 *     all the data to Flow Counter Indirect Data Registers[1:0],Flow Counter Indirect Data register[0] is for flow
 *     Counter hit count and 4 lsb of Flow Counter byte count.Flow Counter Indirect Data register[0] bits 27:0 are
 *     28-bit Flow Counter hit count.Flow Counter Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit Flow
 *     Counter byte count.Flow Counter Indirect Data Register[1] is for 32 msb of 36-bit Flow Counter byte count.{Flow
 *     Counter Indirect Data Register[1], Flow Counter Indirect Data register[0][31:28]} is the 36-bit Flow Counter
 *     byte count.then followed by a write to Flow Counter Indirect Address Register to set upFlow Counter adress and
 *     W_R bit to 1, this will initiate the write
 *     operation.--------------------------------------------------------For Flow Counter read operation,first, write
 *     to Flow Counter Indirect Address Register to set upFlow Counter address and W_R bit to 0, this will initiate
 *     the read operation.the read data from Flow Counter will be loaded into Flow Counter Indirect Data
 *     Registers[1:0].then followed by read from Flow Counter Indirect Data Registers[1:0] for all data.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(uint32_t index, uint32_t data);
bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get(uint32_t index, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_indir_addr_reg,
    cli_natc_indir_natc_flow_cntr_indir_addr_reg,
    cli_natc_indir_natc_flow_cntr_indir_data_reg,
};

int bcm_natc_indir_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_indir_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
