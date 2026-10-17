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


#ifndef _XRDP_DRV_GSU_MEM_AG_H_
#define _XRDP_DRV_GSU_MEM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_mem_pd_fifo_entry_set(uint8_t gsu_mem_id, uint32_t pd_fifo, uint32_t val);
bdmf_error_t ag_drv_gsu_mem_pd_fifo_entry_get(uint8_t gsu_mem_id, uint32_t pd_fifo, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_mem_queue_data_entry_set(uint8_t gsu_mem_id, uint32_t queue_data, uint32_t val);
bdmf_error_t ag_drv_gsu_mem_queue_data_entry_get(uint8_t gsu_mem_id, uint32_t queue_data, uint32_t *val);

/**********************************************************************************************************************
 * entry_0_val: 
 *     value
 * entry_1_val: 
 *     value
 * entry_2_val: 
 *     value
 * entry_3_val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_mem_node_mask_vector_entry_set(uint8_t gsu_mem_id, uint32_t node_mask, uint8_t entry_0_val, uint8_t entry_1_val, uint8_t entry_2_val, uint8_t entry_3_val);
bdmf_error_t ag_drv_gsu_mem_node_mask_vector_entry_get(uint8_t gsu_mem_id, uint32_t node_mask, uint8_t *entry_0_val, uint8_t *entry_1_val, uint8_t *entry_2_val, uint8_t *entry_3_val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_mem_context_data_entry_set(uint8_t gsu_mem_id, uint32_t context_data, uint32_t val);
bdmf_error_t ag_drv_gsu_mem_context_data_entry_get(uint8_t gsu_mem_id, uint32_t context_data, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_mem_input_info_entry_set(uint8_t gsu_mem_id, uint32_t input_info, uint32_t val);
bdmf_error_t ag_drv_gsu_mem_input_info_entry_get(uint8_t gsu_mem_id, uint32_t input_info, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_gsu_mem_pd_fifo_entry,
    cli_gsu_mem_queue_data_entry,
    cli_gsu_mem_node_mask_vector_entry,
    cli_gsu_mem_context_data_entry,
    cli_gsu_mem_input_info_entry,
};

int bcm_gsu_mem_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_gsu_mem_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
