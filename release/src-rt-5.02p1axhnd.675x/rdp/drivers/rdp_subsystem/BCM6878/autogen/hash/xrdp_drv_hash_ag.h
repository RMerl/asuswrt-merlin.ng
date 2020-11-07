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

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* maskl: MASKL - MASK LOW applied on the 32 lsb of the current part of key for the current searc */
/*        h table.The value used for padding purpose and comparison to the hash content.          */
/* maskh: MASKH - MASK HIGH applied on the 28 msb of the current part of key for the current sear */
/*        ch table.The value used for padding purpose and comparison to the hash content.         */
/**************************************************************************************************/
typedef struct
{
    uint32_t maskl;
    uint32_t maskh;
} hash_mask;


/**************************************************************************************************/
/* hash_base_addr: hash_base_addr - Base address of the hash ram per engine.Varies between 0 to 1 */
/*                 535Indicates from which address start looking the key.Note, base address must  */
/*                 be aligned to table size - table size of 128 cant get base 64                  */
/* tbl_size: Table_Size - Number of entries in the  table per engine.Total  entries should be mul */
/*           tiplied with the number of engines - by 4.                                           */
/* max_hop: Max_Hop - Max Search depth per engine.Supports up to 16 and cannot exceed table size. */
/*          For performance requirement it should be limited to 4                                 */
/* cam_en: cam_en - CAM Search is enabled.If the key not found in the hash table and this flag en */
/*         abled the key will be searched in the CAm.                                             */
/* direct_lkup_en: direct_lkup_en - Direct lookup enable.Allows accessing the table without hash  */
/*                 calculation- direct access.                                                    */
/* hash_type: Hash_Type - Hash function type                                                      */
/* int_ctx_size: int_cntx_size - If the key smaller than 60 bit, then it supported to store in th */
/*               e remaining bits an internal context data 3B or 6B.                              */
/**************************************************************************************************/
typedef struct
{
    uint16_t hash_base_addr;
    uint8_t tbl_size;
    uint8_t max_hop;
    bdmf_boolean cam_en;
    bdmf_boolean direct_lkup_en;
    bdmf_boolean hash_type;
    uint8_t int_ctx_size;
} hash_lkup_tbl_cfg_tbl_cfg;


/**************************************************************************************************/
/* base_address: Base_Address - Context table base address in the RAM (6Bytes X 3264entries) .Ind */
/*               icates from which address start looking at the context.The address varies betwee */
/*               n 0 to 3264 (including 196 CAM entries)It should be calculated according below f */
/*               ormula:Context_base_addr[12:0] = sum of (table_size_per_engine*num_of_eng*contex */
/*               t_size)/6 for all preceding tables                                               */
/* first_hash_idx: First_hash_index - Indicates the first entry of the particular table in the co */
/*                 ntext table.It should be calculated according to below formula:First_hash_inde */
/*                 x = sum of (table_size_per_engine*num_of_eng) for all preceding tables         */
/* ext_ctx_size: Context_size - Context entry size (in the context RAM).Varies between 0B to 12B  */
/*               in steps of 3BContext may also be extracted directly from Look-up Table (up to 6 */
/*               B).                                                                              */
/**************************************************************************************************/
typedef struct
{
    uint16_t base_address;
    uint16_t first_hash_idx;
    uint8_t ext_ctx_size;
} hash_lkup_tbl_cfg_cntxt_cfg;


/**************************************************************************************************/
/* value: Value - .                                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t key_in[2];
} hash_cam_indirect_key_in;


/**************************************************************************************************/
/* value: Value - .                                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t key_out[2];
} hash_cam_indirect_key_out;


/**************************************************************************************************/
/* invld_cmd: invalid_cmd - Command cfg field is invalid (equals to 0)                            */
/* mult_match: multiple_match - During the search process same key was found a valid in multiple  */
/*             engines.                                                                           */
/* hash_0_idx_ovflv: hash_0_idx_overflow - hash table index over flow at hash engine              */
/* hash_1_idx_ovflv: hash_1_idx_overflow - hash table over flow at hash engine                    */
/* hash_2_idx_ovflv: hash_2_idx_overflow - hash table index over flow at hash engine              */
/* hash_3_idx_ovflv: hash_3_idx_overflow - hash table index over flow at hash engine              */
/* cntxt_idx_ovflv: cntxt_idx_overflow - Context table index over flow                            */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean invld_cmd;
    bdmf_boolean mult_match;
    bdmf_boolean hash_0_idx_ovflv;
    bdmf_boolean hash_1_idx_ovflv;
    bdmf_boolean hash_2_idx_ovflv;
    bdmf_boolean hash_3_idx_ovflv;
    bdmf_boolean cntxt_idx_ovflv;
} hash_intr_ctrl_isr;

bdmf_error_t ag_drv_hash_key_padding_set(uint32_t key_pad_h, uint32_t key_pad_l);
bdmf_error_t ag_drv_hash_key_padding_get(uint32_t *key_pad_h, uint32_t *key_pad_l);
bdmf_error_t ag_drv_hash_mask_set(uint8_t tbl_idx, const hash_mask *mask);
bdmf_error_t ag_drv_hash_mask_get(uint8_t tbl_idx, hash_mask *mask);
bdmf_error_t ag_drv_hash_general_configuration_pwr_sav_en_set(bdmf_boolean value);
bdmf_error_t ag_drv_hash_general_configuration_pwr_sav_en_get(bdmf_boolean *value);
bdmf_error_t ag_drv_hash_general_configuration_mult_hit_err_get(uint8_t *val);
bdmf_error_t ag_drv_hash_general_configuration_undo_fix_set(bdmf_boolean frst_mul_hit);
bdmf_error_t ag_drv_hash_general_configuration_undo_fix_get(bdmf_boolean *frst_mul_hit);
bdmf_error_t ag_drv_hash_pm_counters_hits_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_srchs_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_miss_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_hit_1st_acs_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_hit_2nd_acs_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_hit_3rd_acs_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_hit_4th_acs_get(uint32_t *cnt);
bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_set(bdmf_boolean val);
bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_get(bdmf_boolean *val);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(uint8_t tbl_idx, const hash_lkup_tbl_cfg_tbl_cfg *lkup_tbl_cfg_tbl_cfg);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get(uint8_t tbl_idx, hash_lkup_tbl_cfg_tbl_cfg *lkup_tbl_cfg_tbl_cfg);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(uint8_t tbl_idx, const hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg);
bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(uint8_t tbl_idx, hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg);
bdmf_error_t ag_drv_hash_cam_base_addr_set(uint16_t base_address);
bdmf_error_t ag_drv_hash_cam_base_addr_get(uint16_t *base_address);
bdmf_error_t ag_drv_hash_cam_indirect_op_set(uint8_t cmd);
bdmf_error_t ag_drv_hash_cam_indirect_op_get(uint8_t *cmd);
bdmf_error_t ag_drv_hash_cam_indirect_op_done_get(bdmf_boolean *val);
bdmf_error_t ag_drv_hash_cam_indirect_addr_set(bdmf_boolean key1_ind, uint8_t entry_addr);
bdmf_error_t ag_drv_hash_cam_indirect_addr_get(bdmf_boolean *key1_ind, uint8_t *entry_addr);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_set(bdmf_boolean valid);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_get(bdmf_boolean *valid);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_set(bdmf_boolean valid);
bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_get(bdmf_boolean *valid);
bdmf_error_t ag_drv_hash_cam_indirect_rslt_get(bdmf_boolean *match, uint8_t *index);
bdmf_error_t ag_drv_hash_cam_indirect_key_in_set(uint8_t zero, const hash_cam_indirect_key_in *cam_indirect_key_in);
bdmf_error_t ag_drv_hash_cam_indirect_key_in_get(uint8_t zero, hash_cam_indirect_key_in *cam_indirect_key_in);
bdmf_error_t ag_drv_hash_cam_indirect_key_out_get(uint8_t zero, hash_cam_indirect_key_out *cam_indirect_key_out);
bdmf_error_t ag_drv_hash_cam_bist_bist_status_get(uint32_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_compare_en_set(bdmf_boolean value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_compare_en_get(bdmf_boolean *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_set(uint32_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_get(uint32_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_set(uint8_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_get(uint8_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_valid_set(bdmf_boolean value);
bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_valid_get(bdmf_boolean *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_en_set(uint16_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_en_get(uint16_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_mode_set(uint8_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_mode_get(uint8_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_rst_l_set(bdmf_boolean value);
bdmf_error_t ag_drv_hash_cam_bist_bist_rst_l_get(bdmf_boolean *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_skip_error_cnt_set(uint8_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_skip_error_cnt_get(uint8_t *value);
bdmf_error_t ag_drv_hash_cam_bist_dbg_en_set(uint16_t value);
bdmf_error_t ag_drv_hash_cam_bist_dbg_en_get(uint16_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_cascade_select_set(uint8_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_cascade_select_get(uint8_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_block_select_set(uint8_t value);
bdmf_error_t ag_drv_hash_cam_bist_bist_block_select_get(uint8_t *value);
bdmf_error_t ag_drv_hash_cam_bist_bist_repair_enable_set(bdmf_boolean value);
bdmf_error_t ag_drv_hash_cam_bist_bist_repair_enable_get(bdmf_boolean *value);
bdmf_error_t ag_drv_hash_intr_ctrl_isr_set(const hash_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_hash_intr_ctrl_isr_get(hash_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_hash_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_hash_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_hash_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_hash_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_hash_intr_ctrl_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_hash_debug_dbg0_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg1_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg2_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg3_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg4_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg5_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg6_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg7_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg8_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg9_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg10_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg11_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg12_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg13_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg14_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg15_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg16_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg17_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg18_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg19_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg20_get(uint32_t *val);
bdmf_error_t ag_drv_hash_debug_dbg_sel_set(uint8_t val);
bdmf_error_t ag_drv_hash_debug_dbg_sel_get(uint8_t *val);
bdmf_error_t ag_drv_hash_aging_ram_aging_set(uint16_t entry_idx, bdmf_boolean age);
bdmf_error_t ag_drv_hash_aging_ram_aging_get(uint16_t entry_idx, bdmf_boolean *age);
bdmf_error_t ag_drv_hash_context_ram_context_47_24_set(uint16_t ctx_idx, uint32_t bits);
bdmf_error_t ag_drv_hash_context_ram_context_47_24_get(uint16_t ctx_idx, uint32_t *bits);
bdmf_error_t ag_drv_hash_context_ram_context_23_0_set(uint16_t ctx_idx, uint32_t bits);
bdmf_error_t ag_drv_hash_context_ram_context_23_0_get(uint16_t ctx_idx, uint32_t *bits);
bdmf_error_t ag_drv_hash_ram_eng_high_set(uint16_t idx, uint32_t key_59_28_or_dat);
bdmf_error_t ag_drv_hash_ram_eng_high_get(uint16_t idx, uint32_t *key_59_28_or_dat);
bdmf_error_t ag_drv_hash_ram_eng_low_set(uint16_t idx, bdmf_boolean skp, uint8_t cfg, uint16_t key_11_0, uint16_t key_27_12_or_dat);
bdmf_error_t ag_drv_hash_ram_eng_low_get(uint16_t idx, bdmf_boolean *skp, uint8_t *cfg, uint16_t *key_11_0, uint16_t *key_27_12_or_dat);

#ifdef USE_BDMF_SHELL
enum
{
    cli_hash_key_padding,
    cli_hash_mask,
    cli_hash_general_configuration_pwr_sav_en,
    cli_hash_general_configuration_mult_hit_err,
    cli_hash_general_configuration_undo_fix,
    cli_hash_pm_counters_hits,
    cli_hash_pm_counters_srchs,
    cli_hash_pm_counters_miss,
    cli_hash_pm_counters_hit_1st_acs,
    cli_hash_pm_counters_hit_2nd_acs,
    cli_hash_pm_counters_hit_3rd_acs,
    cli_hash_pm_counters_hit_4th_acs,
    cli_hash_pm_counters_frz_cnt,
    cli_hash_lkup_tbl_cfg_tbl_cfg,
    cli_hash_lkup_tbl_cfg_cntxt_cfg,
    cli_hash_cam_base_addr,
    cli_hash_cam_indirect_op,
    cli_hash_cam_indirect_op_done,
    cli_hash_cam_indirect_addr,
    cli_hash_cam_indirect_vlid_in,
    cli_hash_cam_indirect_vlid_out,
    cli_hash_cam_indirect_rslt,
    cli_hash_cam_indirect_key_in,
    cli_hash_cam_indirect_key_out,
    cli_hash_cam_bist_bist_status,
    cli_hash_cam_bist_bist_dbg_compare_en,
    cli_hash_cam_bist_bist_dbg_data,
    cli_hash_cam_bist_bist_dbg_data_slice_or_status_sel,
    cli_hash_cam_bist_bist_dbg_data_valid,
    cli_hash_cam_bist_bist_en,
    cli_hash_cam_bist_bist_mode,
    cli_hash_cam_bist_bist_rst_l,
    cli_hash_cam_bist_bist_skip_error_cnt,
    cli_hash_cam_bist_dbg_en,
    cli_hash_cam_bist_bist_cascade_select,
    cli_hash_cam_bist_bist_block_select,
    cli_hash_cam_bist_bist_repair_enable,
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
    cli_hash_context_ram_context_47_24,
    cli_hash_context_ram_context_23_0,
    cli_hash_ram_eng_high,
    cli_hash_ram_eng_low,
};

int bcm_hash_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_hash_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

