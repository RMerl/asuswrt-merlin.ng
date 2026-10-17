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


#ifndef _XRDP_DRV_HASH_AG_H_
#define _XRDP_DRV_HASH_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t maskl;
    uint32_t maskh;
} hash_mask;

typedef struct
{
    uint8_t hash_table_threshold;
    uint8_t cam_threshold;
    uint8_t cntxt_in_threshold;
    uint8_t storm_threshold;
    bdmf_boolean storm_threshold_enable;
} hash_general_configuration_fifo_threshold;

typedef struct
{
    bdmf_boolean direct_read_en;
    bdmf_boolean cam_en;
    bdmf_boolean move_en;
    bdmf_boolean storm_en;
    bdmf_boolean aging_en;
    bdmf_boolean rslt128_en;
    uint32_t move_mask;
} hash_lkup_tbl_cfg_cntxt_cfg;

typedef struct
{
    uint32_t key_in[2];
} hash_cam_indirect_key_in;

typedef struct
{
    uint32_t key_out[2];
} hash_cam_indirect_key_out;


/**********************************************************************************************************************
 * maskl: 
 *     MASK Low applied on the 32 lsb of the current part of key for the current search table. + CFG [2:0] mask
 *     
 *     
 * maskh: 
 *     MASK High applied on the 32 msb of the current part of key for the current search table.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_mask_set(uint8_t hash_id, uint8_t tbl_idx, const hash_mask *mask);
bdmf_error_t ag_drv_hash_mask_get(uint8_t hash_id, uint8_t tbl_idx, hash_mask *mask);

/**********************************************************************************************************************
 * block_enable: 
 *     block enable
 * lkup_eng_search_disable: 
 *     Disable lkup engine search - to save power
 * storm_move_dis: 
 *     Disable move in Storm - move even if enabled will not effect storm logic
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_general_configuration_gen_cfg_set(uint8_t hash_id, bdmf_boolean block_enable, uint32_t lkup_eng_search_disable, bdmf_boolean storm_move_dis);
bdmf_error_t ag_drv_hash_general_configuration_gen_cfg_get(uint8_t hash_id, bdmf_boolean *block_enable, uint32_t *lkup_eng_search_disable, bdmf_boolean *storm_move_dis);

/**********************************************************************************************************************
 * hash_table_threshold: 
 *     hash table FIFO thresh hold
 * cam_threshold: 
 *     CAM FIFO thresh hold
 * cntxt_in_threshold: 
 *     CNTXT IN FIFO thresh hold
 * storm_threshold: 
 *     STORM FIFO thresh hold
 * storm_threshold_enable: 
 *     STORM FIFO thresh enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_general_configuration_fifo_threshold_set(uint8_t hash_id, const hash_general_configuration_fifo_threshold *general_configuration_fifo_threshold);
bdmf_error_t ag_drv_hash_general_configuration_fifo_threshold_get(uint8_t hash_id, hash_general_configuration_fifo_threshold *general_configuration_fifo_threshold);

/**********************************************************************************************************************
 * engine_val: 
 *     In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.
 * key_val: 
 *     In case of multiple match this reg captures the 17 key lower bits, 16:0.
 *     This is a read clear reg.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_general_configuration_mult_hit_err_get(uint8_t hash_id, uint32_t *engine_val, uint16_t *key_val);

/**********************************************************************************************************************
 * auto_init_en: 
 *     Enable auto init of memories:
 *     hash table, aging(MAC)
 *     Once set it will init the BD Ram memories. It will clear when finished
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_general_configuration_auto_init_en_set(uint8_t hash_id, bdmf_boolean auto_init_en);
bdmf_error_t ag_drv_hash_general_configuration_auto_init_en_get(uint8_t hash_id, bdmf_boolean *auto_init_en);

/**********************************************************************************************************************
 * auto_init_en: 
 *     Enable auto init of memory  storm (MAC).
 *     Once set it will init the BD Ram memories. It will clear when finished
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_general_configuration_storm_auto_init_en_set(uint8_t hash_id, bdmf_boolean auto_init_en);
bdmf_error_t ag_drv_hash_general_configuration_storm_auto_init_en_get(uint8_t hash_id, bdmf_boolean *auto_init_en);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_hits_get(uint8_t hash_id, uint32_t *cnt);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_srchs_get(uint8_t hash_id, uint32_t *cnt);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_miss_get(uint8_t hash_id, uint32_t *cnt);

/**********************************************************************************************************************
 * cnt: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_storm_window_size_set(uint8_t hash_id, uint32_t cnt);
bdmf_error_t ag_drv_hash_pm_counters_storm_window_size_get(uint8_t hash_id, uint32_t *cnt);

/**********************************************************************************************************************
 * storm_th: 
 *     storm trheshold
 * storm_window_enable: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_storm_cfg_set(uint8_t hash_id, uint16_t storm_th, bdmf_boolean storm_window_enable);
bdmf_error_t ag_drv_hash_pm_counters_storm_cfg_get(uint8_t hash_id, uint16_t *storm_th, bdmf_boolean *storm_window_enable);

/**********************************************************************************************************************
 * val: 
 *     Freezes counters update
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_set(uint8_t hash_id, bdmf_boolean val);
bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_get(uint8_t hash_id, bdmf_boolean *val);

/**********************************************************************************************************************
 * direct_read_en: 
 *     direct read enable
 * cam_en: 
 *     CAM enable in case of miss
 * move_en: 
 *     enable move enable in MAC table
 * storm_en: 
 *     in MAC mode only (SA)
 *     storm enable
 * aging_en: 
 *     in MAC mode only (SA)
 *     aging   enable
 * rslt128_en: 
 *     enable 128 bit only result on BroadBus
 *     in case we want better performance
 *     
 * move_mask: 
 *     in MAC mode, configure move bit mask
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(uint8_t hash_id, uint8_t tbl_idx, const hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(uint8_t hash_id, uint8_t tbl_idx, hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg);

/**********************************************************************************************************************
 * maskh: 
 *     MASK High applied on the 32 msb of the current part of key for the current search table.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t maskh);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *maskh);

/**********************************************************************************************************************
 * maskm: 
 *     MASK MID applied on the 32 mid of the current part of key for the current search table.
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t maskm);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *maskm);

/**********************************************************************************************************************
 * val: 
 *     16bit for tm control
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_configuration_tm_cfg_set(uint8_t hash_id, uint16_t val);
bdmf_error_t ag_drv_hash_cam_configuration_tm_cfg_get(uint8_t hash_id, uint16_t *val);

/**********************************************************************************************************************
 * cmd: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_op_set(uint8_t hash_id, uint8_t cmd);
bdmf_error_t ag_drv_hash_cam_indirect_op_get(uint8_t hash_id, uint8_t *cmd);

/**********************************************************************************************************************
 * val: 
 *     .
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_op_done_get(uint8_t hash_id, bdmf_boolean *val);

/**********************************************************************************************************************
 * key_ind: 
 *     This bit indicate if the operation (RD/WR) is performed on the key High, Med, Low part of the entry
 * entry_addr: 
 *     Address of the entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_addr_set(uint8_t hash_id, bdmf_boolean key_ind, uint8_t entry_addr);
bdmf_error_t ag_drv_hash_cam_indirect_addr_get(uint8_t hash_id, bdmf_boolean *key_ind, uint8_t *entry_addr);

/**********************************************************************************************************************
 * valid: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_set(uint8_t hash_id, bdmf_boolean valid);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_get(uint8_t hash_id, bdmf_boolean *valid);

/**********************************************************************************************************************
 * valid: 
 *     .
 *     CHANGED_BY_HW
 *     
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_set(uint8_t hash_id, uint8_t valid);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_get(uint8_t hash_id, uint8_t *valid);

/**********************************************************************************************************************
 * match: 
 *     indicate if a match was found
 * index: 
 *     index related to a match result
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_rslt_get(uint8_t hash_id, bdmf_boolean *match, uint8_t *index);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_key_in_set(uint8_t hash_id, uint8_t zero, const hash_cam_indirect_key_in *cam_indirect_key_in);
bdmf_error_t ag_drv_hash_cam_indirect_key_in_get(uint8_t hash_id, uint8_t zero, hash_cam_indirect_key_in *cam_indirect_key_in);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_indirect_key_out_get(uint8_t hash_id, uint8_t zero, hash_cam_indirect_key_out *cam_indirect_key_out);

/**********************************************************************************************************************
 * value: 
 *     .
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_bist_bist_status_get(uint8_t hash_id, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_bist_bist_control_set(uint8_t hash_id, uint32_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_control_get(uint8_t hash_id, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_bist_bist_cache_repair_set(uint8_t hash_id, uint32_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_cache_repair_get(uint8_t hash_id, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_control_set(uint8_t hash_id, uint32_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_control_get(uint8_t hash_id, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_set(uint8_t hash_id, uint32_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_get(uint8_t hash_id, uint32_t *value);

/**********************************************************************************************************************
 * mult_match: 
 *     During the search process same key was found a valid in multiple engines.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_intr_ctrl_isr_set(uint8_t hash_id, bdmf_boolean mult_match);
bdmf_error_t ag_drv_hash_intr_ctrl_isr_get(uint8_t hash_id, bdmf_boolean *mult_match);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_intr_ctrl_ism_get(uint8_t hash_id, uint8_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_intr_ctrl_ier_set(uint8_t hash_id, uint8_t iem);
bdmf_error_t ag_drv_hash_intr_ctrl_ier_get(uint8_t hash_id, uint8_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_intr_ctrl_itr_set(uint8_t hash_id, uint32_t ist);
bdmf_error_t ag_drv_hash_intr_ctrl_itr_get(uint8_t hash_id, uint32_t *ist);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg0_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg1_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg2_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg3_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg4_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg5_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg6_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg7_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg8_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg9_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg10_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg11_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg12_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg13_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg14_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg15_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg16_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg17_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg18_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg19_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     read debug register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg20_get(uint8_t hash_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     debug sel
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_debug_dbg_sel_set(uint8_t hash_id, uint8_t val);
bdmf_error_t ag_drv_hash_debug_dbg_sel_get(uint8_t hash_id, uint8_t *val);

/**********************************************************************************************************************
 * data: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_aging_ram_aging_set(uint8_t hash_id, uint16_t entry_idx, bdmf_boolean age);
bdmf_error_t ag_drv_hash_aging_ram_aging_get(uint8_t hash_id, uint16_t entry_idx, bdmf_boolean *age);

/**********************************************************************************************************************
 * data: 
 *     .
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_storm_ram_storm_set(uint8_t hash_id, uint32_t idx, uint32_t data);
bdmf_error_t ag_drv_hash_storm_ram_storm_get(uint8_t hash_id, uint32_t idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     MAC - 32 bits
 *     ICL/IPTV - 24 bits - 47:24
 *     FCL - 32 Bits LSB
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_cam_context_ram_context_cam_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data);
bdmf_error_t ag_drv_hash_cam_context_ram_context_cam_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     MAC - 32 bits
 *     ICL/IPTV - 24 bits - 47:24
 *     FCL - 32 Bits LSB
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_context_ram_context_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data);
bdmf_error_t ag_drv_hash_context_ram_context_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     MAC - 32 bits
 *     ICL/IPTV - 24 bits - 47:24
 *     FCL - 32 Bits LSB
 **********************************************************************************************************************/
bdmf_error_t ag_drv_hash_ram_hash_ram_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data);
bdmf_error_t ag_drv_hash_ram_hash_ram_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_hash_mask,
    cli_hash_general_configuration_gen_cfg,
    cli_hash_general_configuration_fifo_threshold,
    cli_hash_general_configuration_mult_hit_err,
    cli_hash_general_configuration_auto_init_en,
    cli_hash_general_configuration_storm_auto_init_en,
    cli_hash_pm_counters_hits,
    cli_hash_pm_counters_srchs,
    cli_hash_pm_counters_miss,
    cli_hash_pm_counters_storm_window_size,
    cli_hash_pm_counters_storm_cfg,
    cli_hash_pm_counters_frz_cnt,
    cli_hash_lkup_tbl_cfg_cntxt_cfg,
    cli_hash_lkup_tbl_cfg_key_mask_highest,
    cli_hash_lkup_tbl_cfg_key_mask_mid,
    cli_hash_cam_configuration_tm_cfg,
    cli_hash_cam_indirect_op,
    cli_hash_cam_indirect_op_done,
    cli_hash_cam_indirect_addr,
    cli_hash_cam_indirect_vlid_in,
    cli_hash_cam_indirect_vlid_out,
    cli_hash_cam_indirect_rslt,
    cli_hash_cam_indirect_key_in,
    cli_hash_cam_indirect_key_out,
    cli_hash_cam_bist_bist_status,
    cli_hash_cam_bist_bist_control,
    cli_hash_cam_bist_bist_cache_repair,
    cli_hash_cam_bist_bist_dbg_control,
    cli_hash_cam_bist_bist_dbg_data,
    cli_hash_intr_ctrl_isr,
    cli_hash_intr_ctrl_ism,
    cli_hash_intr_ctrl_ier,
    cli_hash_intr_ctrl_itr,
    cli_hash_debug_dbg0,
    cli_hash_debug_dbg1,
    cli_hash_debug_dbg2,
    cli_hash_debug_dbg3,
    cli_hash_debug_dbg4,
    cli_hash_debug_dbg5,
    cli_hash_debug_dbg6,
    cli_hash_debug_dbg7,
    cli_hash_debug_dbg8,
    cli_hash_debug_dbg9,
    cli_hash_debug_dbg10,
    cli_hash_debug_dbg11,
    cli_hash_debug_dbg12,
    cli_hash_debug_dbg13,
    cli_hash_debug_dbg14,
    cli_hash_debug_dbg15,
    cli_hash_debug_dbg16,
    cli_hash_debug_dbg17,
    cli_hash_debug_dbg18,
    cli_hash_debug_dbg19,
    cli_hash_debug_dbg20,
    cli_hash_debug_dbg_sel,
    cli_hash_aging_ram_aging,
    cli_hash_storm_ram_storm,
    cli_hash_cam_context_ram_context_cam,
    cli_hash_context_ram_context,
    cli_hash_ram_hash_ram,
};

int bcm_hash_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_hash_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
