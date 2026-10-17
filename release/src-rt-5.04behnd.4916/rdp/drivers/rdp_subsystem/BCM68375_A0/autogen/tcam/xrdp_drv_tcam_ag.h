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


#ifndef _XRDP_DRV_TCAM_AG_H_
#define _XRDP_DRV_TCAM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * srch_short_key: 
 *     .
 * hit_short_key: 
 *     .
 * srch_long_key: 
 *     .
 * hit_long_key: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_counters_get(uint32_t *srch_short_key, uint32_t *hit_short_key, uint32_t *srch_long_key, uint32_t *hit_long_key);

/**********************************************************************************************************************
 * data: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_context_set(uint16_t ctx_idx, uint32_t data);
bdmf_error_t ag_drv_tcam_context_get(uint16_t ctx_idx, uint32_t *data);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_bank_enable_set(uint16_t value);
bdmf_error_t ag_drv_tcam_bank_enable_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_tm_tcam0_set(uint16_t value);
bdmf_error_t ag_drv_tcam_cfg_tm_tcam0_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_tm_tcam1_set(uint16_t value);
bdmf_error_t ag_drv_tcam_cfg_tm_tcam1_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_global_mask_set(uint8_t word_idx, uint32_t value);
bdmf_error_t ag_drv_tcam_global_mask_get(uint8_t word_idx, uint32_t *value);

/**********************************************************************************************************************
 * cmd: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_op_set(uint8_t cmd);
bdmf_error_t ag_drv_tcam_op_get(uint8_t *cmd);

/**********************************************************************************************************************
 * done: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_op_done_get(bdmf_boolean *done);

/**********************************************************************************************************************
 * key1_ind: 
 *     This bit indicate if the operation (RD/WR) is performed on the key0 or key1 part of the entry
 * entry_addr: 
 *     Address of the entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_address_set(bdmf_boolean key1_ind, uint16_t entry_addr);
bdmf_error_t ag_drv_tcam_address_get(bdmf_boolean *key1_ind, uint16_t *entry_addr);

/**********************************************************************************************************************
 * valid: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_valid_in_set(bdmf_boolean valid);
bdmf_error_t ag_drv_tcam_valid_in_get(bdmf_boolean *valid);

/**********************************************************************************************************************
 * valid: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_valid_out_set(bdmf_boolean valid);
bdmf_error_t ag_drv_tcam_valid_out_get(bdmf_boolean *valid);

/**********************************************************************************************************************
 * match: 
 *     indicate if a match was found
 * index: 
 *     index related to a match result
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_result_get(bdmf_boolean *match, uint16_t *index);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_key_in_set(uint8_t word_idx, uint32_t value);
bdmf_error_t ag_drv_tcam_key_in_get(uint8_t word_idx, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_key_out_get(uint8_t word_idx, uint32_t *value);

/**********************************************************************************************************************
 * select_module: 
 *     selection
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_set(uint8_t select_module);
bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_get(uint8_t *select_module);

#ifdef USE_BDMF_SHELL
enum
{
    cli_tcam_counters,
    cli_tcam_context,
    cli_tcam_bank_enable,
    cli_tcam_cfg_tm_tcam0,
    cli_tcam_cfg_tm_tcam1,
    cli_tcam_global_mask,
    cli_tcam_op,
    cli_tcam_op_done,
    cli_tcam_address,
    cli_tcam_valid_in,
    cli_tcam_valid_out,
    cli_tcam_result,
    cli_tcam_key_in,
    cli_tcam_key_out,
    cli_tcam_tcam_debug_bus_select,
};

int bcm_tcam_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_tcam_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
