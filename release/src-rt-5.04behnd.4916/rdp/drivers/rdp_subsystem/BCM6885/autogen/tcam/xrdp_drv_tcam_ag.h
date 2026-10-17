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
 * ram_line: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_dinm_set(uint8_t tcam_id, uint32_t cfg_dinm, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_cfg_dinm_get(uint8_t tcam_id, uint32_t cfg_dinm, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_blksel_set(uint8_t tcam_id, uint32_t cfg_blksel, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_cfg_blksel_get(uint8_t tcam_id, uint32_t cfg_blksel, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_tm_set(uint8_t tcam_id, uint32_t cfg_tm, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_cfg_tm_get(uint8_t tcam_id, uint32_t cfg_tm, uint32_t *ram_line);

/**********************************************************************************************************************
 * val: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_cfg_vreset_set(uint8_t tcam_id, uint16_t val);
bdmf_error_t ag_drv_tcam_cfg_vreset_get(uint8_t tcam_id, uint16_t *val);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_counters_searches_get(uint8_t tcam_id, uint32_t counters_searches, uint32_t *cnt);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_counters_hits_get(uint8_t tcam_id, uint8_t tcam_idx, uint32_t *cnt);

/**********************************************************************************************************************
 * cmd: 
 *     .
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_set(uint8_t tcam_id, uint8_t cmd);
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_get(uint8_t tcam_id, uint8_t *cmd);

/**********************************************************************************************************************
 * done: 
 *     .
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_done_get(uint8_t tcam_id, bdmf_boolean *done);

/**********************************************************************************************************************
 * entry_addr: 
 *     Address of the entry
 *     TRIGGERS_HW
 *     
 * mem_id: 
 *     MEM_ID
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_addr_set(uint8_t tcam_id, uint16_t entry_addr, uint8_t mem_id);
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_addr_get(uint8_t tcam_id, uint16_t *entry_addr, uint8_t *mem_id);

/**********************************************************************************************************************
 * valid: 
 *     .
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_in_set(uint8_t tcam_id, uint8_t valid);
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_in_get(uint8_t tcam_id, uint8_t *valid);

/**********************************************************************************************************************
 * valid: 
 *     .
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_out_get(uint8_t tcam_id, uint8_t *valid);

/**********************************************************************************************************************
 * value: 
 *     .
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_out_get(uint8_t tcam_id, uint8_t indirect_data, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_in_set(uint8_t tcam_id, uint8_t indirect_data_in, uint32_t value);
bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_in_get(uint8_t tcam_id, uint8_t indirect_data_in, uint32_t *value);

/**********************************************************************************************************************
 * select_module: 
 *     selection
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_set(uint8_t tcam_id, uint8_t select_module);
bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_get(uint8_t tcam_id, uint8_t *select_module);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_0_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_0_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_1_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_1_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_2_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_2_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_3_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_3_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_4_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_4_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_5_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_5_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_6_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_6_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_7_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_7_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_8_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_8_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_9_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_9_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_10_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_10_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_11_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_11_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_12_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_12_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_13_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_13_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_14_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_14_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_15_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_15_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_16_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_16_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_17_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_17_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_18_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_18_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_19_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_19_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_20_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_20_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_21_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_21_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_22_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_22_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_23_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_23_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_24_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_24_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_25_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_25_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_26_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_26_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_27_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_27_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_28_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_28_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_29_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_29_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_30_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_30_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_31_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_31_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_32_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_32_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_33_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_33_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_34_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_34_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_35_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_35_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_36_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_36_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_37_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_37_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_38_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_38_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_39_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_39_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_0_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_0_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_1_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_1_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_2_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_2_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_3_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_3_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_4_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_4_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_5_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_5_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_6_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_6_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_7_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_7_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_8_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_8_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_9_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_9_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_10_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_10_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_11_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_11_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_12_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_12_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_13_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_13_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_14_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_14_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_15_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_15_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_16_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_16_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_17_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_17_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_18_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_18_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_19_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_19_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_20_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_20_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_21_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_21_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_22_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_22_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_23_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_23_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_24_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_24_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_25_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_25_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_26_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_26_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_27_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_27_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_28_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_28_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_29_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_29_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_30_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_30_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_inner_shift_31_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_inner_shift_31_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_0_set(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_0_get(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_1_set(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_1_get(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_0_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_0_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_1_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_1_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_2_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_2_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_3_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_3_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_4_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_4_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_5_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_5_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_6_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_6_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_7_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_7_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_sp_mask_0_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_sp_mask_0_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_sp_mask_1_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_sp_mask_1_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_sp_mask_2_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_sp_mask_2_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_sp_mask_3_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_sp_mask_3_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_0_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_0_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_1_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_1_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_2_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_2_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_3_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_mux_3_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_0_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_0_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_1_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_1_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_2_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_2_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_3_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_3_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_4_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_4_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_5_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_5_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_6_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_6_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * ram_line: 
 *     RAM_LINE
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_7_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line);
bdmf_error_t ag_drv_tcam_profiles_rslt_builder_7_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line);

/**********************************************************************************************************************
 * en: 
 *     EN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_tcam_profiles_tcam_concat_set(uint8_t tcam_id, uint8_t profile_tcam_concat, uint16_t en);
bdmf_error_t ag_drv_tcam_profiles_tcam_concat_get(uint8_t tcam_id, uint8_t profile_tcam_concat, uint16_t *en);

#ifdef USE_BDMF_SHELL
enum
{
    cli_tcam_cfg_dinm,
    cli_tcam_cfg_blksel,
    cli_tcam_cfg_tm,
    cli_tcam_cfg_vreset,
    cli_tcam_counters_searches,
    cli_tcam_counters_hits,
    cli_tcam_tcam_mem_indirect_op,
    cli_tcam_tcam_mem_indirect_op_done,
    cli_tcam_tcam_mem_indirect_addr,
    cli_tcam_tcam_mem_indirect_valid_in,
    cli_tcam_tcam_mem_indirect_valid_out,
    cli_tcam_tcam_mem_indirect_data_out,
    cli_tcam_tcam_mem_indirect_data_in,
    cli_tcam_tcam_debug_bus_select,
    cli_tcam_profiles_key_builder_offsets_0,
    cli_tcam_profiles_key_builder_offsets_1,
    cli_tcam_profiles_key_builder_offsets_2,
    cli_tcam_profiles_key_builder_offsets_3,
    cli_tcam_profiles_key_builder_offsets_4,
    cli_tcam_profiles_key_builder_offsets_5,
    cli_tcam_profiles_key_builder_offsets_6,
    cli_tcam_profiles_key_builder_offsets_7,
    cli_tcam_profiles_key_builder_offsets_8,
    cli_tcam_profiles_key_builder_offsets_9,
    cli_tcam_profiles_key_builder_offsets_10,
    cli_tcam_profiles_key_builder_offsets_11,
    cli_tcam_profiles_key_builder_offsets_12,
    cli_tcam_profiles_key_builder_offsets_13,
    cli_tcam_profiles_key_builder_offsets_14,
    cli_tcam_profiles_key_builder_offsets_15,
    cli_tcam_profiles_key_builder_offsets_16,
    cli_tcam_profiles_key_builder_offsets_17,
    cli_tcam_profiles_key_builder_offsets_18,
    cli_tcam_profiles_key_builder_offsets_19,
    cli_tcam_profiles_key_builder_offsets_20,
    cli_tcam_profiles_key_builder_offsets_21,
    cli_tcam_profiles_key_builder_offsets_22,
    cli_tcam_profiles_key_builder_offsets_23,
    cli_tcam_profiles_key_builder_offsets_24,
    cli_tcam_profiles_key_builder_offsets_25,
    cli_tcam_profiles_key_builder_offsets_26,
    cli_tcam_profiles_key_builder_offsets_27,
    cli_tcam_profiles_key_builder_offsets_28,
    cli_tcam_profiles_key_builder_offsets_29,
    cli_tcam_profiles_key_builder_offsets_30,
    cli_tcam_profiles_key_builder_offsets_31,
    cli_tcam_profiles_key_builder_offsets_32,
    cli_tcam_profiles_key_builder_offsets_33,
    cli_tcam_profiles_key_builder_offsets_34,
    cli_tcam_profiles_key_builder_offsets_35,
    cli_tcam_profiles_key_builder_offsets_36,
    cli_tcam_profiles_key_builder_offsets_37,
    cli_tcam_profiles_key_builder_offsets_38,
    cli_tcam_profiles_key_builder_offsets_39,
    cli_tcam_profiles_inner_shift_0,
    cli_tcam_profiles_inner_shift_1,
    cli_tcam_profiles_inner_shift_2,
    cli_tcam_profiles_inner_shift_3,
    cli_tcam_profiles_inner_shift_4,
    cli_tcam_profiles_inner_shift_5,
    cli_tcam_profiles_inner_shift_6,
    cli_tcam_profiles_inner_shift_7,
    cli_tcam_profiles_inner_shift_8,
    cli_tcam_profiles_inner_shift_9,
    cli_tcam_profiles_inner_shift_10,
    cli_tcam_profiles_inner_shift_11,
    cli_tcam_profiles_inner_shift_12,
    cli_tcam_profiles_inner_shift_13,
    cli_tcam_profiles_inner_shift_14,
    cli_tcam_profiles_inner_shift_15,
    cli_tcam_profiles_inner_shift_16,
    cli_tcam_profiles_inner_shift_17,
    cli_tcam_profiles_inner_shift_18,
    cli_tcam_profiles_inner_shift_19,
    cli_tcam_profiles_inner_shift_20,
    cli_tcam_profiles_inner_shift_21,
    cli_tcam_profiles_inner_shift_22,
    cli_tcam_profiles_inner_shift_23,
    cli_tcam_profiles_inner_shift_24,
    cli_tcam_profiles_inner_shift_25,
    cli_tcam_profiles_inner_shift_26,
    cli_tcam_profiles_inner_shift_27,
    cli_tcam_profiles_inner_shift_28,
    cli_tcam_profiles_inner_shift_29,
    cli_tcam_profiles_inner_shift_30,
    cli_tcam_profiles_inner_shift_31,
    cli_tcam_profiles_cntxin_sp_id_0,
    cli_tcam_profiles_cntxin_sp_id_1,
    cli_tcam_profiles_cntxin_mask_0,
    cli_tcam_profiles_cntxin_mask_1,
    cli_tcam_profiles_cntxin_mask_2,
    cli_tcam_profiles_cntxin_mask_3,
    cli_tcam_profiles_cntxin_mask_4,
    cli_tcam_profiles_cntxin_mask_5,
    cli_tcam_profiles_cntxin_mask_6,
    cli_tcam_profiles_cntxin_mask_7,
    cli_tcam_profiles_sp_mask_0,
    cli_tcam_profiles_sp_mask_1,
    cli_tcam_profiles_sp_mask_2,
    cli_tcam_profiles_sp_mask_3,
    cli_tcam_profiles_rslt_mux_0,
    cli_tcam_profiles_rslt_mux_1,
    cli_tcam_profiles_rslt_mux_2,
    cli_tcam_profiles_rslt_mux_3,
    cli_tcam_profiles_rslt_builder_0,
    cli_tcam_profiles_rslt_builder_1,
    cli_tcam_profiles_rslt_builder_2,
    cli_tcam_profiles_rslt_builder_3,
    cli_tcam_profiles_rslt_builder_4,
    cli_tcam_profiles_rslt_builder_5,
    cli_tcam_profiles_rslt_builder_6,
    cli_tcam_profiles_rslt_builder_7,
    cli_tcam_profiles_tcam_concat,
};

int bcm_tcam_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_tcam_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
