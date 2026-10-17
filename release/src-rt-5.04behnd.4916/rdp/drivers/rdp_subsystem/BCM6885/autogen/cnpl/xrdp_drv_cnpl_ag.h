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


#ifndef _XRDP_DRV_CNPL_AG_H_
#define _XRDP_DRV_CNPL_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean cn_double;
    uint8_t cn0_byts;
    uint32_t ba;
    bdmf_boolean wrap;
    bdmf_boolean clr;
} cnpl_counter_cfg;

typedef struct
{
    bdmf_boolean cn_rd_st;
    bdmf_boolean pl_plc_st;
    bdmf_boolean pl_rd_st;
} cnpl_sw_stat;


/**********************************************************************************************************************
 * cn_double: 
 *     1:each counter of the group is double sub-cntr (for 4B-double the format is 36b+28b)
 *     0:each counter of the group is single
 * cn0_byts: 
 *     number of bytes that will hold each sub-counter.
 *     0: 1B
 *     1: 2B
 *     2: 4B
 *     3: 8B (only single, no double supported)
 * ba: 
 *     counters group base address (in 8B resolution: 0 is 0x0, 1 is 0x8, ...)
 *     17b for 32K counters x3 (4B, dbl), and last bit for support of small bank_64 after the 32K.
 * wrap: 
 *     1:wrap at max value
 *     0:freeze at max value
 * clr: 
 *     1:read clear when reading
 *     0:read no-clear when reading
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_counter_cfg_set(uint8_t cnpl_id, uint32_t cnt_loc_profile, const cnpl_counter_cfg *counter_cfg);
bdmf_error_t ag_drv_cnpl_counter_cfg_get(uint8_t cnpl_id, uint32_t cnt_loc_profile, cnpl_counter_cfg *counter_cfg);

/**********************************************************************************************************************
 * cn_rd_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 *     CHANGED_BY_HW
 *     
 * pl_plc_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 *     CHANGED_BY_HW
 *     
 * pl_rd_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_stat_get(uint8_t cnpl_id, cnpl_sw_stat *sw_stat);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_main_memory_main_data_set(uint8_t cnpl_id, uint32_t mem_idx, uint32_t data);
bdmf_error_t ag_drv_cnpl_main_memory_main_data_get(uint8_t cnpl_id, uint32_t mem_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_num64_memory_b64_data_set(uint8_t cnpl_id, uint32_t mem_idx, uint32_t data);
bdmf_error_t ag_drv_cnpl_num64_memory_b64_data_get(uint8_t cnpl_id, uint32_t mem_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t data);
bdmf_error_t ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t data);
bdmf_error_t ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *data);

/**********************************************************************************************************************
 * mode: 
 *     mode:
 *     0: normal cnpl counters
 *     1: QM counters
 *     2: QM watermark counters
 *     In watermark counter -
 *      we get a counter index and a value in the command
 *      the engine reads current value
 *      if new value > current value, then write it to the memory
 *      clear is done like for normal counters through the sw_rd_cntr engine.
 *     
 *     the parsing for QM counters/watermark counters is:
 *     QM 128b = {4b0, 4b0, cntr[14:0], byte_size[31:0], 1b0, pkt_size[31:0], 40b0 }
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_counters_configurations_cn_glb_ctrl_set(uint8_t cnpl_id, uint8_t mode);
bdmf_error_t ag_drv_cnpl_counters_configurations_cn_glb_ctrl_get(uint8_t cnpl_id, uint8_t *mode);

/**********************************************************************************************************************
 * bk_ba: 
 *     buckets base address(in 16B resolution: 0 is 0x0, 1 is 0x10, ...), each addr correspnds to a memory entry.
 *     15b for 32K policers(4B, dbl), and last bit for support of small bank_64 after the 32K.
 * pa_ba: 
 *     params base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...).
 *     Not valid for lotus (since params are in 256 entry params mem)
 * doublle: 
 *     1:each policer is dual bucket
 *     0:each policer is single bucket
 * fc: 
 *     1:the policers are flow control type
 *     0:the policers are normal
 *     
 *     When it is fc, the double field has no effect.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t bk_ba, uint16_t pa_ba, bdmf_boolean doublle, bdmf_boolean fc);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *bk_ba, uint16_t *pa_ba, bdmf_boolean *doublle, bdmf_boolean *fc);

/**********************************************************************************************************************
 * pl_st: 
 *     Index of 1st policer of the group.
 *     In default: pol_start>pol_end, so the group is not valid
 *     
 *     Note: we have total of 32K + 64(fc) policers, so 16b are needed.
 * pl_end: 
 *     Index of last policer of the group.
 *     In default: pol_start>pol_end, so the group is not valid
 *     
 *     Note: we have total of 32K + 64(fc) policers, so 16b are needed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t pl_st, uint16_t pl_end);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *pl_st, uint16_t *pl_end);

/**********************************************************************************************************************
 * n: 
 *     shift left of 16k cycles quanta. 4b lsb only are valid.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(uint8_t cnpl_id, uint32_t conf_idx, uint8_t n);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(uint8_t cnpl_id, uint32_t conf_idx, uint8_t *n);

/**********************************************************************************************************************
 * vec: 
 *     32b vector for 32 policers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t vec);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *vec);

/**********************************************************************************************************************
 * prf0: 
 *     16b profile
 * prf1: 
 *     16b profile
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t prf0, uint16_t prf1);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *prf0, uint16_t *prf1);

/**********************************************************************************************************************
 * en: 
 *     1:enable periodic update
 *     0:disable periodic update
 * mtu: 
 *     MTU for calculation of bucket size
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_set(uint8_t cnpl_id, bdmf_boolean en, uint16_t mtu);
bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_get(uint8_t cnpl_id, bdmf_boolean *en, uint16_t *mtu);

/**********************************************************************************************************************
 * val: 
 *     32b value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t val);
bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     32b value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(uint8_t cnpl_id, uint32_t conf_idx, uint8_t val);
bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(uint8_t cnpl_id, uint32_t conf_idx, uint8_t *val);

/**********************************************************************************************************************
 * val: 
 *     value of register
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_h_set(uint8_t cnpl_id, uint32_t val);
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_h_get(uint8_t cnpl_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value of register
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_l_set(uint8_t cnpl_id, uint32_t val);
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_l_get(uint8_t cnpl_id, uint32_t *val);

/**********************************************************************************************************************
 * col: 
 *     red, yellow, green, non-active
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rslt_get(uint8_t cnpl_id, uint8_t *col);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rd_get(uint8_t cnpl_id, uint32_t bucket, uint32_t *rd);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_cnt_rd_get(uint8_t cnpl_id, uint32_t rd_idx, uint32_t *rd);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_rd_used_get(uint8_t cnpl_id, uint32_t *rd);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_pop_get(uint8_t cnpl_id, uint32_t *rd);

/**********************************************************************************************************************
 * val: 
 *     val
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_thr_set(uint8_t cnpl_id, uint8_t val);
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_thr_get(uint8_t cnpl_id, uint8_t *val);

/**********************************************************************************************************************
 * val: 
 *     val
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_set(uint8_t cnpl_id, bdmf_boolean val);
bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_get(uint8_t cnpl_id, bdmf_boolean *val);

/**********************************************************************************************************************
 * sw_prio: 
 *     0: fixed lower
 *     1: rr with fw (default)
 *     2: fixed higher
 *     TRIGGERS_HW
 *     
 * mem_addr_bit_sel: 
 *     11 possible options to select which address bit decodes to which cnpl memory the transaction goes.
 *     (32KB mem = 15b addr(in Bytes), 12b addr (in 8B words), 11 options to select 2 bits)
 *     Value of 0xF is kept to disable this feature (legacy mode), and all accesses will be directed to memory0.
 *     since we have a few sizes of cnpl, please use values of [3..1], so -
 *     bits [0..8] are valid values for all cnpl types.
 *     Note: if mem_addr_bit_sel (in CNPL_BLOCK.CNPL_Misc.ARB_PRM) > 8, only half the address space is accessible.
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_arb_prm_set(uint8_t cnpl_id, uint8_t sw_prio, uint8_t mem_addr_bit_sel);
bdmf_error_t ag_drv_cnpl_misc_arb_prm_get(uint8_t cnpl_id, uint8_t *sw_prio, uint8_t *mem_addr_bit_sel);

/**********************************************************************************************************************
 * en: 
 *     0: dis
 *     1: en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_col_awr_en_set(uint8_t cnpl_id, bdmf_boolean en);
bdmf_error_t ag_drv_cnpl_misc_col_awr_en_get(uint8_t cnpl_id, bdmf_boolean *en);

/**********************************************************************************************************************
 * main_init: 
 *     init for main mem (all banks)
 *     TRIGGERS_HW
 *     
 * plc_prof_sel_init: 
 *     init for policers profiles sel mem (all banks)
 *     TRIGGERS_HW
 *     Not relevant for counters cnpl
 *     
 * plc_profiles_init: 
 *     init for policers profiles mem (all banks)
 *     TRIGGERS_HW
 *     Not relevant for counters cnpl
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_mem_init_set(uint8_t cnpl_id, bdmf_boolean main_init, bdmf_boolean plc_prof_sel_init, bdmf_boolean plc_profiles_init);
bdmf_error_t ag_drv_cnpl_misc_mem_init_get(uint8_t cnpl_id, bdmf_boolean *main_init, bdmf_boolean *plc_prof_sel_init, bdmf_boolean *plc_profiles_init);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_pm_counters_err_bad_entry_get(uint8_t cnpl_id, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_pm_counters_ovflw_f_push_err_get(uint8_t cnpl_id, uint32_t *val);

/**********************************************************************************************************************
 * req: 
 *     still more commands for arbitration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_debug_err_bad_entry_data_get(uint8_t cnpl_id, uint8_t *req);

/**********************************************************************************************************************
 * mult_match: 
 *     During the search process same key was found a valid in multiple engines.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_intr_ctrl_isr_set(uint8_t cnpl_id, bdmf_boolean mult_match);
bdmf_error_t ag_drv_cnpl_intr_ctrl_isr_get(uint8_t cnpl_id, bdmf_boolean *mult_match);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_intr_ctrl_ism_get(uint8_t cnpl_id, uint8_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_intr_ctrl_ier_set(uint8_t cnpl_id, uint8_t iem);
bdmf_error_t ag_drv_cnpl_intr_ctrl_ier_get(uint8_t cnpl_id, uint8_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_intr_ctrl_itr_set(uint8_t cnpl_id, uint32_t ist);
bdmf_error_t ag_drv_cnpl_intr_ctrl_itr_get(uint8_t cnpl_id, uint32_t *ist);

#ifdef USE_BDMF_SHELL
enum
{
    cli_cnpl_counter_cfg,
    cli_cnpl_sw_stat,
    cli_cnpl_main_memory_main_data,
    cli_cnpl_num64_memory_b64_data,
    cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data,
    cli_cnpl_plc_profiles_mem_plc_profiles_data,
    cli_cnpl_counters_configurations_cn_glb_ctrl,
    cli_cnpl_policers_configurations_pl_loc_prof_a,
    cli_cnpl_policers_configurations_pl_loc_prof_b,
    cli_cnpl_policers_configurations_pl_loc_prof_c,
    cli_cnpl_policers_configurations_pl_calc_type,
    cli_cnpl_policers_configurations_pl_size_prof,
    cli_cnpl_policers_configurations_per_up,
    cli_cnpl_policers_fc_thr_fc_thr_l,
    cli_cnpl_policers_fc_thr_fc_thr_m,
    cli_cnpl_sw_if_sw_cmd_h,
    cli_cnpl_sw_if_sw_cmd_l,
    cli_cnpl_sw_if_sw_pl_rslt,
    cli_cnpl_sw_if_sw_pl_rd,
    cli_cnpl_sw_if_sw_cnt_rd,
    cli_cnpl_sw_if_sw_ovflw_f_rd_used,
    cli_cnpl_sw_if_sw_ovflw_f_pop,
    cli_cnpl_sw_if_sw_ovflw_f_thr,
    cli_cnpl_sw_if_sw_ovflw_f_full_mod,
    cli_cnpl_misc_arb_prm,
    cli_cnpl_misc_col_awr_en,
    cli_cnpl_misc_mem_init,
    cli_cnpl_pm_counters_err_bad_entry,
    cli_cnpl_pm_counters_ovflw_f_push_err,
    cli_cnpl_debug_err_bad_entry_data,
    cli_cnpl_intr_ctrl_isr,
    cli_cnpl_intr_ctrl_ism,
    cli_cnpl_intr_ctrl_ier,
    cli_cnpl_intr_ctrl_itr,
};

int bcm_cnpl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_cnpl_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
