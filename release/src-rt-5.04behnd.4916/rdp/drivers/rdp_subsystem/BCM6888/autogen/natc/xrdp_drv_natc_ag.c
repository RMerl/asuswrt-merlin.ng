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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_natc_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_ctrl_status_set(const natc_ctrl_status *ctrl_status)
{
    uint32_t reg_control_status = 0;
    uint32_t reg_control_status2 = 0;

#ifdef VALIDATE_PARMS
    if(!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctrl_status->ddr_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_add_command_speedup_mode >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused0 >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_init_done >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_64bit_in_128bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->smem_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->smem_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_swap_all_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->repeated_key_det_en >= _1BITS_MAX_VAL_) ||
       (ctrl_status->reg_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->reg_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_pending_hash_mode >= _3BITS_MAX_VAL_) ||
       (ctrl_status->pending_fifo_entry_check_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->cache_update_on_ddr_miss >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_disable_on_reg_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->nat_hash_mode >= _3BITS_MAX_VAL_) ||
       (ctrl_status->multi_hash_limit >= _4BITS_MAX_VAL_) ||
       (ctrl_status->decr_count_wraparound_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->nat_arb_st >= _2BITS_MAX_VAL_) ||
       (ctrl_status->natc_smem_increment_on_reg_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_smem_clear_by_update_disable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->regfile_fifo_reset >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_reset >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused3 >= _3BITS_MAX_VAL_) ||
       (ctrl_status->ddr_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->cache_lookup_blocking_mode >= _1BITS_MAX_VAL_) ||
       (ctrl_status->age_timer_tick >= _1BITS_MAX_VAL_) ||
       (ctrl_status->age_timer >= _5BITS_MAX_VAL_) ||
       (ctrl_status->cache_algo >= _4BITS_MAX_VAL_) ||
       (ctrl_status->unused1 >= _2BITS_MAX_VAL_) ||
       (ctrl_status->cache_update_on_reg_ddr_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_counter_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_hash_swap >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_replace_duplicated_cached_entry_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_lookup_pending_fifo_mode_disable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused4 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_ENABLE, reg_control_status, ctrl_status->ddr_enable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_control_status, ctrl_status->natc_add_command_speedup_mode);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, UNUSED0, reg_control_status, ctrl_status->unused0);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_INIT_DONE, reg_control_status, ctrl_status->natc_init_done);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_64BIT_IN_128BIT_SWAP_CONTROL, reg_control_status, ctrl_status->ddr_64bit_in_128bit_swap_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, SMEM_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status, ctrl_status->smem_32bit_in_64bit_swap_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, SMEM_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status, ctrl_status->smem_8bit_in_32bit_swap_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_SWAP_ALL_CONTROL, reg_control_status, ctrl_status->ddr_swap_all_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, REPEATED_KEY_DET_EN, reg_control_status, ctrl_status->repeated_key_det_en);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, REG_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status, ctrl_status->reg_32bit_in_64bit_swap_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, REG_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status, ctrl_status->reg_8bit_in_32bit_swap_control);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_PENDING_HASH_MODE, reg_control_status, ctrl_status->ddr_pending_hash_mode);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, PENDING_FIFO_ENTRY_CHECK_ENABLE, reg_control_status, ctrl_status->pending_fifo_entry_check_enable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, CACHE_UPDATE_ON_DDR_MISS, reg_control_status, ctrl_status->cache_update_on_ddr_miss);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_DISABLE_ON_REG_LOOKUP, reg_control_status, ctrl_status->ddr_disable_on_reg_lookup);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NAT_HASH_MODE, reg_control_status, ctrl_status->nat_hash_mode);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, MULTI_HASH_LIMIT, reg_control_status, ctrl_status->multi_hash_limit);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DECR_COUNT_WRAPAROUND_ENABLE, reg_control_status, ctrl_status->decr_count_wraparound_enable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NAT_ARB_ST, reg_control_status, ctrl_status->nat_arb_st);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_SMEM_INCREMENT_ON_REG_LOOKUP, reg_control_status, ctrl_status->natc_smem_increment_on_reg_lookup);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_SMEM_CLEAR_BY_UPDATE_DISABLE, reg_control_status, ctrl_status->natc_smem_clear_by_update_disable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, REGFILE_FIFO_RESET, reg_control_status, ctrl_status->regfile_fifo_reset);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_ENABLE, reg_control_status, ctrl_status->natc_enable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_RESET, reg_control_status, ctrl_status->natc_reset);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, UNUSED3, reg_control_status2, ctrl_status->unused3);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status2, ctrl_status->ddr_32bit_in_64bit_swap_control);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status2, ctrl_status->ddr_8bit_in_32bit_swap_control);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, CACHE_LOOKUP_BLOCKING_MODE, reg_control_status2, ctrl_status->cache_lookup_blocking_mode);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, AGE_TIMER_TICK, reg_control_status2, ctrl_status->age_timer_tick);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, AGE_TIMER, reg_control_status2, ctrl_status->age_timer);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, CACHE_ALGO, reg_control_status2, ctrl_status->cache_algo);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, UNUSED2, reg_control_status2, ctrl_status->unused2);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, UNUSED1, reg_control_status2, ctrl_status->unused1);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, CACHE_UPDATE_ON_REG_DDR_LOOKUP, reg_control_status2, ctrl_status->cache_update_on_reg_ddr_lookup);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status2, ctrl_status->ddr_counter_8bit_in_32bit_swap_control);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_HASH_SWAP, reg_control_status2, ctrl_status->ddr_hash_swap);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE, reg_control_status2, ctrl_status->ddr_replace_duplicated_cached_entry_enable);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE, reg_control_status2, ctrl_status->ddr_lookup_pending_fifo_mode_disable);
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, UNUSED4, reg_control_status2, ctrl_status->unused4);

    RU_REG_WRITE(0, NATC, CONTROL_STATUS, reg_control_status);
    RU_REG_WRITE(0, NATC, CONTROL_STATUS2, reg_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ctrl_status_get(natc_ctrl_status *ctrl_status)
{
    uint32_t reg_control_status;
    uint32_t reg_control_status2;

#ifdef VALIDATE_PARMS
    if (!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, CONTROL_STATUS, reg_control_status);
    RU_REG_READ(0, NATC, CONTROL_STATUS2, reg_control_status2);

    ctrl_status->ddr_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_ENABLE, reg_control_status);
    ctrl_status->natc_add_command_speedup_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_control_status);
    ctrl_status->unused0 = RU_FIELD_GET(0, NATC, CONTROL_STATUS, UNUSED0, reg_control_status);
    ctrl_status->natc_init_done = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_INIT_DONE, reg_control_status);
    ctrl_status->ddr_64bit_in_128bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_64BIT_IN_128BIT_SWAP_CONTROL, reg_control_status);
    ctrl_status->smem_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, SMEM_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status);
    ctrl_status->smem_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, SMEM_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status);
    ctrl_status->ddr_swap_all_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_SWAP_ALL_CONTROL, reg_control_status);
    ctrl_status->repeated_key_det_en = RU_FIELD_GET(0, NATC, CONTROL_STATUS, REPEATED_KEY_DET_EN, reg_control_status);
    ctrl_status->reg_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, REG_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status);
    ctrl_status->reg_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS, REG_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status);
    ctrl_status->ddr_pending_hash_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_PENDING_HASH_MODE, reg_control_status);
    ctrl_status->pending_fifo_entry_check_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, PENDING_FIFO_ENTRY_CHECK_ENABLE, reg_control_status);
    ctrl_status->cache_update_on_ddr_miss = RU_FIELD_GET(0, NATC, CONTROL_STATUS, CACHE_UPDATE_ON_DDR_MISS, reg_control_status);
    ctrl_status->ddr_disable_on_reg_lookup = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_DISABLE_ON_REG_LOOKUP, reg_control_status);
    ctrl_status->nat_hash_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NAT_HASH_MODE, reg_control_status);
    ctrl_status->multi_hash_limit = RU_FIELD_GET(0, NATC, CONTROL_STATUS, MULTI_HASH_LIMIT, reg_control_status);
    ctrl_status->decr_count_wraparound_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DECR_COUNT_WRAPAROUND_ENABLE, reg_control_status);
    ctrl_status->nat_arb_st = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NAT_ARB_ST, reg_control_status);
    ctrl_status->natc_smem_increment_on_reg_lookup = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_SMEM_INCREMENT_ON_REG_LOOKUP, reg_control_status);
    ctrl_status->natc_smem_clear_by_update_disable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_SMEM_CLEAR_BY_UPDATE_DISABLE, reg_control_status);
    ctrl_status->regfile_fifo_reset = RU_FIELD_GET(0, NATC, CONTROL_STATUS, REGFILE_FIFO_RESET, reg_control_status);
    ctrl_status->natc_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_ENABLE, reg_control_status);
    ctrl_status->natc_reset = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_RESET, reg_control_status);
    ctrl_status->unused3 = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, UNUSED3, reg_control_status2);
    ctrl_status->ddr_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_32BIT_IN_64BIT_SWAP_CONTROL, reg_control_status2);
    ctrl_status->ddr_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status2);
    ctrl_status->cache_lookup_blocking_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, CACHE_LOOKUP_BLOCKING_MODE, reg_control_status2);
    ctrl_status->age_timer_tick = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, AGE_TIMER_TICK, reg_control_status2);
    ctrl_status->age_timer = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, AGE_TIMER, reg_control_status2);
    ctrl_status->cache_algo = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, CACHE_ALGO, reg_control_status2);
    ctrl_status->unused2 = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, UNUSED2, reg_control_status2);
    ctrl_status->unused1 = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, UNUSED1, reg_control_status2);
    ctrl_status->cache_update_on_reg_ddr_lookup = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, CACHE_UPDATE_ON_REG_DDR_LOOKUP, reg_control_status2);
    ctrl_status->ddr_counter_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL, reg_control_status2);
    ctrl_status->ddr_hash_swap = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_HASH_SWAP, reg_control_status2);
    ctrl_status->ddr_replace_duplicated_cached_entry_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE, reg_control_status2);
    ctrl_status->ddr_lookup_pending_fifo_mode_disable = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE, reg_control_status2);
    ctrl_status->unused4 = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, UNUSED4, reg_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_table_control_set(const natc_table_control *table_control)
{
    uint32_t reg_table_control = 0;

#ifdef VALIDATE_PARMS
    if(!table_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((table_control->smem_dis_tbl7 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl6 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl5 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl4 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl3 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl2 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl1 >= _1BITS_MAX_VAL_) ||
       (table_control->smem_dis_tbl0 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl7 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl6 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl5 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl4 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl3 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl2 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl1 >= _1BITS_MAX_VAL_) ||
       (table_control->var_context_len_en_tbl0 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl7 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl7 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl6 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl6 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl5 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl5 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl4 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl4 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl3 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl3 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl2 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl2 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl1 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl1 >= _1BITS_MAX_VAL_) ||
       (table_control->key_len_tbl0 >= _1BITS_MAX_VAL_) ||
       (table_control->non_cacheable_tbl0 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL7, reg_table_control, table_control->smem_dis_tbl7);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL6, reg_table_control, table_control->smem_dis_tbl6);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL5, reg_table_control, table_control->smem_dis_tbl5);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL4, reg_table_control, table_control->smem_dis_tbl4);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL3, reg_table_control, table_control->smem_dis_tbl3);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL2, reg_table_control, table_control->smem_dis_tbl2);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL1, reg_table_control, table_control->smem_dis_tbl1);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL0, reg_table_control, table_control->smem_dis_tbl0);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL7, reg_table_control, table_control->var_context_len_en_tbl7);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL6, reg_table_control, table_control->var_context_len_en_tbl6);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL5, reg_table_control, table_control->var_context_len_en_tbl5);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL4, reg_table_control, table_control->var_context_len_en_tbl4);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL3, reg_table_control, table_control->var_context_len_en_tbl3);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL2, reg_table_control, table_control->var_context_len_en_tbl2);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL1, reg_table_control, table_control->var_context_len_en_tbl1);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL0, reg_table_control, table_control->var_context_len_en_tbl0);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL7, reg_table_control, table_control->key_len_tbl7);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL7, reg_table_control, table_control->non_cacheable_tbl7);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL6, reg_table_control, table_control->key_len_tbl6);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL6, reg_table_control, table_control->non_cacheable_tbl6);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL5, reg_table_control, table_control->key_len_tbl5);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL5, reg_table_control, table_control->non_cacheable_tbl5);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL4, reg_table_control, table_control->key_len_tbl4);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL4, reg_table_control, table_control->non_cacheable_tbl4);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL3, reg_table_control, table_control->key_len_tbl3);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL3, reg_table_control, table_control->non_cacheable_tbl3);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL2, reg_table_control, table_control->key_len_tbl2);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL2, reg_table_control, table_control->non_cacheable_tbl2);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL1, reg_table_control, table_control->key_len_tbl1);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL1, reg_table_control, table_control->non_cacheable_tbl1);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL0, reg_table_control, table_control->key_len_tbl0);
    reg_table_control = RU_FIELD_SET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL0, reg_table_control, table_control->non_cacheable_tbl0);

    RU_REG_WRITE(0, NATC, TABLE_CONTROL, reg_table_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_table_control_get(natc_table_control *table_control)
{
    uint32_t reg_table_control;

#ifdef VALIDATE_PARMS
    if (!table_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, TABLE_CONTROL, reg_table_control);

    table_control->smem_dis_tbl7 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL7, reg_table_control);
    table_control->smem_dis_tbl6 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL6, reg_table_control);
    table_control->smem_dis_tbl5 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL5, reg_table_control);
    table_control->smem_dis_tbl4 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL4, reg_table_control);
    table_control->smem_dis_tbl3 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL3, reg_table_control);
    table_control->smem_dis_tbl2 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL2, reg_table_control);
    table_control->smem_dis_tbl1 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL1, reg_table_control);
    table_control->smem_dis_tbl0 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, SMEM_DIS_TBL0, reg_table_control);
    table_control->var_context_len_en_tbl7 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL7, reg_table_control);
    table_control->var_context_len_en_tbl6 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL6, reg_table_control);
    table_control->var_context_len_en_tbl5 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL5, reg_table_control);
    table_control->var_context_len_en_tbl4 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL4, reg_table_control);
    table_control->var_context_len_en_tbl3 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL3, reg_table_control);
    table_control->var_context_len_en_tbl2 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL2, reg_table_control);
    table_control->var_context_len_en_tbl1 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL1, reg_table_control);
    table_control->var_context_len_en_tbl0 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL0, reg_table_control);
    table_control->key_len_tbl7 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL7, reg_table_control);
    table_control->non_cacheable_tbl7 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL7, reg_table_control);
    table_control->key_len_tbl6 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL6, reg_table_control);
    table_control->non_cacheable_tbl6 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL6, reg_table_control);
    table_control->key_len_tbl5 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL5, reg_table_control);
    table_control->non_cacheable_tbl5 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL5, reg_table_control);
    table_control->key_len_tbl4 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL4, reg_table_control);
    table_control->non_cacheable_tbl4 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL4, reg_table_control);
    table_control->key_len_tbl3 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL3, reg_table_control);
    table_control->non_cacheable_tbl3 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL3, reg_table_control);
    table_control->key_len_tbl2 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL2, reg_table_control);
    table_control->non_cacheable_tbl2 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL2, reg_table_control);
    table_control->key_len_tbl1 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL1, reg_table_control);
    table_control->non_cacheable_tbl1 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL1, reg_table_control);
    table_control->key_len_tbl0 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, KEY_LEN_TBL0, reg_table_control);
    table_control->non_cacheable_tbl0 = RU_FIELD_GET(0, NATC, TABLE_CONTROL, NON_CACHEABLE_TBL0, reg_table_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_stat_counter_control_0_set(uint8_t ddr_evict_count_en, uint8_t ddr_request_count_en, uint8_t cache_miss_count_en, uint8_t cache_hit_count_en)
{
    uint32_t reg_stat_counter_control_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_stat_counter_control_0, ddr_evict_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_stat_counter_control_0, ddr_request_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_stat_counter_control_0, cache_miss_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_stat_counter_control_0, cache_hit_count_en);

    RU_REG_WRITE(0, NATC, STAT_COUNTER_CONTROL_0, reg_stat_counter_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_stat_counter_control_0_get(uint8_t *ddr_evict_count_en, uint8_t *ddr_request_count_en, uint8_t *cache_miss_count_en, uint8_t *cache_hit_count_en)
{
    uint32_t reg_stat_counter_control_0;

#ifdef VALIDATE_PARMS
    if (!ddr_evict_count_en || !ddr_request_count_en || !cache_miss_count_en || !cache_hit_count_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, STAT_COUNTER_CONTROL_0, reg_stat_counter_control_0);

    *ddr_evict_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_stat_counter_control_0);
    *ddr_request_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_stat_counter_control_0);
    *cache_miss_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_stat_counter_control_0);
    *cache_hit_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_stat_counter_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_stat_counter_control_1_set(bdmf_boolean counter_wraparound_dis, uint8_t ddr_block_count_en)
{
    uint32_t reg_stat_counter_control_1 = 0;

#ifdef VALIDATE_PARMS
    if ((counter_wraparound_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_stat_counter_control_1 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_stat_counter_control_1, counter_wraparound_dis);
    reg_stat_counter_control_1 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_stat_counter_control_1, ddr_block_count_en);

    RU_REG_WRITE(0, NATC, STAT_COUNTER_CONTROL_1, reg_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_stat_counter_control_1_get(bdmf_boolean *counter_wraparound_dis, uint8_t *ddr_block_count_en)
{
    uint32_t reg_stat_counter_control_1;

#ifdef VALIDATE_PARMS
    if (!counter_wraparound_dis || !ddr_block_count_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, STAT_COUNTER_CONTROL_1, reg_stat_counter_control_1);

    *counter_wraparound_dis = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_stat_counter_control_1);
    *ddr_block_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_0_set(uint8_t regfile_fifo_start_addr_3, uint8_t regfile_fifo_start_addr_2, uint8_t regfile_fifo_start_addr_1, uint8_t regfile_fifo_start_addr_0)
{
    uint32_t reg_regfile_fifo_start_addr_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_3, reg_regfile_fifo_start_addr_0, regfile_fifo_start_addr_3);
    reg_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_2, reg_regfile_fifo_start_addr_0, regfile_fifo_start_addr_2);
    reg_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_1, reg_regfile_fifo_start_addr_0, regfile_fifo_start_addr_1);
    reg_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_0, reg_regfile_fifo_start_addr_0, regfile_fifo_start_addr_0);

    RU_REG_WRITE(0, NATC, REGFILE_FIFO_START_ADDR_0, reg_regfile_fifo_start_addr_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_0_get(uint8_t *regfile_fifo_start_addr_3, uint8_t *regfile_fifo_start_addr_2, uint8_t *regfile_fifo_start_addr_1, uint8_t *regfile_fifo_start_addr_0)
{
    uint32_t reg_regfile_fifo_start_addr_0;

#ifdef VALIDATE_PARMS
    if (!regfile_fifo_start_addr_3 || !regfile_fifo_start_addr_2 || !regfile_fifo_start_addr_1 || !regfile_fifo_start_addr_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, REGFILE_FIFO_START_ADDR_0, reg_regfile_fifo_start_addr_0);

    *regfile_fifo_start_addr_3 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_3, reg_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_2 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_2, reg_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_1 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_1, reg_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_0 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_0, reg_regfile_fifo_start_addr_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_1_set(uint8_t regfile_fifo_start_addr_7, uint8_t regfile_fifo_start_addr_6, uint8_t regfile_fifo_start_addr_5, uint8_t regfile_fifo_start_addr_4)
{
    uint32_t reg_regfile_fifo_start_addr_1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_7, reg_regfile_fifo_start_addr_1, regfile_fifo_start_addr_7);
    reg_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_6, reg_regfile_fifo_start_addr_1, regfile_fifo_start_addr_6);
    reg_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_5, reg_regfile_fifo_start_addr_1, regfile_fifo_start_addr_5);
    reg_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_4, reg_regfile_fifo_start_addr_1, regfile_fifo_start_addr_4);

    RU_REG_WRITE(0, NATC, REGFILE_FIFO_START_ADDR_1, reg_regfile_fifo_start_addr_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_regfile_fifo_start_addr_1_get(uint8_t *regfile_fifo_start_addr_7, uint8_t *regfile_fifo_start_addr_6, uint8_t *regfile_fifo_start_addr_5, uint8_t *regfile_fifo_start_addr_4)
{
    uint32_t reg_regfile_fifo_start_addr_1;

#ifdef VALIDATE_PARMS
    if (!regfile_fifo_start_addr_7 || !regfile_fifo_start_addr_6 || !regfile_fifo_start_addr_5 || !regfile_fifo_start_addr_4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, REGFILE_FIFO_START_ADDR_1, reg_regfile_fifo_start_addr_1);

    *regfile_fifo_start_addr_7 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_7, reg_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_6 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_6, reg_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_5 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_5, reg_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_4 = RU_FIELD_GET(0, NATC, REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_4, reg_regfile_fifo_start_addr_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_flow_cntr_cntl_set(const natc_flow_cntr_cntl *flow_cntr_cntl)
{
    uint32_t reg_flow_cntr_cntl = 0;

#ifdef VALIDATE_PARMS
    if(!flow_cntr_cntl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((flow_cntr_cntl->flow_cntr_en_tbl7 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl6 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl5 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl4 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl3 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl2 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl1 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->flow_cntr_en_tbl0 >= _1BITS_MAX_VAL_) ||
       (flow_cntr_cntl->context_offset >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL7, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl7);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL6, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl6);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL5, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl5);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL4, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl4);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL3, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl3);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL2, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl2);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL1, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl1);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL0, reg_flow_cntr_cntl, flow_cntr_cntl->flow_cntr_en_tbl0);
    reg_flow_cntr_cntl = RU_FIELD_SET(0, NATC, FLOW_CNTR_CNTL, CONTEXT_OFFSET, reg_flow_cntr_cntl, flow_cntr_cntl->context_offset);

    RU_REG_WRITE(0, NATC, FLOW_CNTR_CNTL, reg_flow_cntr_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_flow_cntr_cntl_get(natc_flow_cntr_cntl *flow_cntr_cntl)
{
    uint32_t reg_flow_cntr_cntl;

#ifdef VALIDATE_PARMS
    if (!flow_cntr_cntl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, FLOW_CNTR_CNTL, reg_flow_cntr_cntl);

    flow_cntr_cntl->flow_cntr_en_tbl7 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL7, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl6 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL6, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl5 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL5, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl4 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL4, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl3 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL3, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl2 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL2, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl1 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL1, reg_flow_cntr_cntl);
    flow_cntr_cntl->flow_cntr_en_tbl0 = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL0, reg_flow_cntr_cntl);
    flow_cntr_cntl->context_offset = RU_FIELD_GET(0, NATC, FLOW_CNTR_CNTL, CONTEXT_OFFSET, reg_flow_cntr_cntl);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_control_status,
    bdmf_address_control_status2,
    bdmf_address_table_control,
    bdmf_address_stat_counter_control_0,
    bdmf_address_stat_counter_control_1,
    bdmf_address_regfile_fifo_start_addr_0,
    bdmf_address_regfile_fifo_start_addr_1,
    bdmf_address_flow_cntr_cntl,
}
bdmf_address;

static int ag_drv_natc_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ctrl_status:
    {
        natc_ctrl_status ctrl_status = { .ddr_enable = parm[1].value.unumber, .natc_add_command_speedup_mode = parm[2].value.unumber, .unused0 = parm[3].value.unumber, .natc_init_done = parm[4].value.unumber, .ddr_64bit_in_128bit_swap_control = parm[5].value.unumber, .smem_32bit_in_64bit_swap_control = parm[6].value.unumber, .smem_8bit_in_32bit_swap_control = parm[7].value.unumber, .ddr_swap_all_control = parm[8].value.unumber, .repeated_key_det_en = parm[9].value.unumber, .reg_32bit_in_64bit_swap_control = parm[10].value.unumber, .reg_8bit_in_32bit_swap_control = parm[11].value.unumber, .ddr_pending_hash_mode = parm[12].value.unumber, .pending_fifo_entry_check_enable = parm[13].value.unumber, .cache_update_on_ddr_miss = parm[14].value.unumber, .ddr_disable_on_reg_lookup = parm[15].value.unumber, .nat_hash_mode = parm[16].value.unumber, .multi_hash_limit = parm[17].value.unumber, .decr_count_wraparound_enable = parm[18].value.unumber, .nat_arb_st = parm[19].value.unumber, .natc_smem_increment_on_reg_lookup = parm[20].value.unumber, .natc_smem_clear_by_update_disable = parm[21].value.unumber, .regfile_fifo_reset = parm[22].value.unumber, .natc_enable = parm[23].value.unumber, .natc_reset = parm[24].value.unumber, .unused3 = parm[25].value.unumber, .ddr_32bit_in_64bit_swap_control = parm[26].value.unumber, .ddr_8bit_in_32bit_swap_control = parm[27].value.unumber, .cache_lookup_blocking_mode = parm[28].value.unumber, .age_timer_tick = parm[29].value.unumber, .age_timer = parm[30].value.unumber, .cache_algo = parm[31].value.unumber, .unused2 = parm[32].value.unumber, .unused1 = parm[33].value.unumber, .cache_update_on_reg_ddr_lookup = parm[34].value.unumber, .ddr_counter_8bit_in_32bit_swap_control = parm[35].value.unumber, .ddr_hash_swap = parm[36].value.unumber, .ddr_replace_duplicated_cached_entry_enable = parm[37].value.unumber, .ddr_lookup_pending_fifo_mode_disable = parm[38].value.unumber, .unused4 = parm[39].value.unumber};
        ag_err = ag_drv_natc_ctrl_status_set(&ctrl_status);
        break;
    }
    case cli_natc_table_control:
    {
        natc_table_control table_control = { .smem_dis_tbl7 = parm[1].value.unumber, .smem_dis_tbl6 = parm[2].value.unumber, .smem_dis_tbl5 = parm[3].value.unumber, .smem_dis_tbl4 = parm[4].value.unumber, .smem_dis_tbl3 = parm[5].value.unumber, .smem_dis_tbl2 = parm[6].value.unumber, .smem_dis_tbl1 = parm[7].value.unumber, .smem_dis_tbl0 = parm[8].value.unumber, .var_context_len_en_tbl7 = parm[9].value.unumber, .var_context_len_en_tbl6 = parm[10].value.unumber, .var_context_len_en_tbl5 = parm[11].value.unumber, .var_context_len_en_tbl4 = parm[12].value.unumber, .var_context_len_en_tbl3 = parm[13].value.unumber, .var_context_len_en_tbl2 = parm[14].value.unumber, .var_context_len_en_tbl1 = parm[15].value.unumber, .var_context_len_en_tbl0 = parm[16].value.unumber, .key_len_tbl7 = parm[17].value.unumber, .non_cacheable_tbl7 = parm[18].value.unumber, .key_len_tbl6 = parm[19].value.unumber, .non_cacheable_tbl6 = parm[20].value.unumber, .key_len_tbl5 = parm[21].value.unumber, .non_cacheable_tbl5 = parm[22].value.unumber, .key_len_tbl4 = parm[23].value.unumber, .non_cacheable_tbl4 = parm[24].value.unumber, .key_len_tbl3 = parm[25].value.unumber, .non_cacheable_tbl3 = parm[26].value.unumber, .key_len_tbl2 = parm[27].value.unumber, .non_cacheable_tbl2 = parm[28].value.unumber, .key_len_tbl1 = parm[29].value.unumber, .non_cacheable_tbl1 = parm[30].value.unumber, .key_len_tbl0 = parm[31].value.unumber, .non_cacheable_tbl0 = parm[32].value.unumber};
        ag_err = ag_drv_natc_table_control_set(&table_control);
        break;
    }
    case cli_natc_stat_counter_control_0:
        ag_err = ag_drv_natc_stat_counter_control_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_stat_counter_control_1:
        ag_err = ag_drv_natc_stat_counter_control_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_natc_regfile_fifo_start_addr_0:
        ag_err = ag_drv_natc_regfile_fifo_start_addr_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_regfile_fifo_start_addr_1:
        ag_err = ag_drv_natc_regfile_fifo_start_addr_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_natc_flow_cntr_cntl:
    {
        natc_flow_cntr_cntl flow_cntr_cntl = { .flow_cntr_en_tbl7 = parm[1].value.unumber, .flow_cntr_en_tbl6 = parm[2].value.unumber, .flow_cntr_en_tbl5 = parm[3].value.unumber, .flow_cntr_en_tbl4 = parm[4].value.unumber, .flow_cntr_en_tbl3 = parm[5].value.unumber, .flow_cntr_en_tbl2 = parm[6].value.unumber, .flow_cntr_en_tbl1 = parm[7].value.unumber, .flow_cntr_en_tbl0 = parm[8].value.unumber, .context_offset = parm[9].value.unumber};
        ag_err = ag_drv_natc_flow_cntr_cntl_set(&flow_cntr_cntl);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ctrl_status:
    {
        natc_ctrl_status ctrl_status;
        ag_err = ag_drv_natc_ctrl_status_get(&ctrl_status);
        bdmf_session_print(session, "ddr_enable = %u = 0x%x\n", ctrl_status.ddr_enable, ctrl_status.ddr_enable);
        bdmf_session_print(session, "natc_add_command_speedup_mode = %u = 0x%x\n", ctrl_status.natc_add_command_speedup_mode, ctrl_status.natc_add_command_speedup_mode);
        bdmf_session_print(session, "unused0 = %u = 0x%x\n", ctrl_status.unused0, ctrl_status.unused0);
        bdmf_session_print(session, "natc_init_done = %u = 0x%x\n", ctrl_status.natc_init_done, ctrl_status.natc_init_done);
        bdmf_session_print(session, "ddr_64bit_in_128bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.ddr_64bit_in_128bit_swap_control);
        bdmf_session_print(session, "smem_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "smem_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_swap_all_control = %u = 0x%x\n", ctrl_status.ddr_swap_all_control, ctrl_status.ddr_swap_all_control);
        bdmf_session_print(session, "repeated_key_det_en = %u = 0x%x\n", ctrl_status.repeated_key_det_en, ctrl_status.repeated_key_det_en);
        bdmf_session_print(session, "reg_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "reg_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_pending_hash_mode = %u = 0x%x\n", ctrl_status.ddr_pending_hash_mode, ctrl_status.ddr_pending_hash_mode);
        bdmf_session_print(session, "pending_fifo_entry_check_enable = %u = 0x%x\n", ctrl_status.pending_fifo_entry_check_enable, ctrl_status.pending_fifo_entry_check_enable);
        bdmf_session_print(session, "cache_update_on_ddr_miss = %u = 0x%x\n", ctrl_status.cache_update_on_ddr_miss, ctrl_status.cache_update_on_ddr_miss);
        bdmf_session_print(session, "ddr_disable_on_reg_lookup = %u = 0x%x\n", ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.ddr_disable_on_reg_lookup);
        bdmf_session_print(session, "nat_hash_mode = %u = 0x%x\n", ctrl_status.nat_hash_mode, ctrl_status.nat_hash_mode);
        bdmf_session_print(session, "multi_hash_limit = %u = 0x%x\n", ctrl_status.multi_hash_limit, ctrl_status.multi_hash_limit);
        bdmf_session_print(session, "decr_count_wraparound_enable = %u = 0x%x\n", ctrl_status.decr_count_wraparound_enable, ctrl_status.decr_count_wraparound_enable);
        bdmf_session_print(session, "nat_arb_st = %u = 0x%x\n", ctrl_status.nat_arb_st, ctrl_status.nat_arb_st);
        bdmf_session_print(session, "natc_smem_increment_on_reg_lookup = %u = 0x%x\n", ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.natc_smem_increment_on_reg_lookup);
        bdmf_session_print(session, "natc_smem_clear_by_update_disable = %u = 0x%x\n", ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.natc_smem_clear_by_update_disable);
        bdmf_session_print(session, "regfile_fifo_reset = %u = 0x%x\n", ctrl_status.regfile_fifo_reset, ctrl_status.regfile_fifo_reset);
        bdmf_session_print(session, "natc_enable = %u = 0x%x\n", ctrl_status.natc_enable, ctrl_status.natc_enable);
        bdmf_session_print(session, "natc_reset = %u = 0x%x\n", ctrl_status.natc_reset, ctrl_status.natc_reset);
        bdmf_session_print(session, "unused3 = %u = 0x%x\n", ctrl_status.unused3, ctrl_status.unused3);
        bdmf_session_print(session, "ddr_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "ddr_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "cache_lookup_blocking_mode = %u = 0x%x\n", ctrl_status.cache_lookup_blocking_mode, ctrl_status.cache_lookup_blocking_mode);
        bdmf_session_print(session, "age_timer_tick = %u = 0x%x\n", ctrl_status.age_timer_tick, ctrl_status.age_timer_tick);
        bdmf_session_print(session, "age_timer = %u = 0x%x\n", ctrl_status.age_timer, ctrl_status.age_timer);
        bdmf_session_print(session, "cache_algo = %u = 0x%x\n", ctrl_status.cache_algo, ctrl_status.cache_algo);
        bdmf_session_print(session, "unused2 = %u = 0x%x\n", ctrl_status.unused2, ctrl_status.unused2);
        bdmf_session_print(session, "unused1 = %u = 0x%x\n", ctrl_status.unused1, ctrl_status.unused1);
        bdmf_session_print(session, "cache_update_on_reg_ddr_lookup = %u = 0x%x\n", ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.cache_update_on_reg_ddr_lookup);
        bdmf_session_print(session, "ddr_counter_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_counter_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_hash_swap = %u = 0x%x\n", ctrl_status.ddr_hash_swap, ctrl_status.ddr_hash_swap);
        bdmf_session_print(session, "ddr_replace_duplicated_cached_entry_enable = %u = 0x%x\n", ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_replace_duplicated_cached_entry_enable);
        bdmf_session_print(session, "ddr_lookup_pending_fifo_mode_disable = %u = 0x%x\n", ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.ddr_lookup_pending_fifo_mode_disable);
        bdmf_session_print(session, "unused4 = %u = 0x%x\n", ctrl_status.unused4, ctrl_status.unused4);
        break;
    }
    case cli_natc_table_control:
    {
        natc_table_control table_control;
        ag_err = ag_drv_natc_table_control_get(&table_control);
        bdmf_session_print(session, "smem_dis_tbl7 = %u = 0x%x\n", table_control.smem_dis_tbl7, table_control.smem_dis_tbl7);
        bdmf_session_print(session, "smem_dis_tbl6 = %u = 0x%x\n", table_control.smem_dis_tbl6, table_control.smem_dis_tbl6);
        bdmf_session_print(session, "smem_dis_tbl5 = %u = 0x%x\n", table_control.smem_dis_tbl5, table_control.smem_dis_tbl5);
        bdmf_session_print(session, "smem_dis_tbl4 = %u = 0x%x\n", table_control.smem_dis_tbl4, table_control.smem_dis_tbl4);
        bdmf_session_print(session, "smem_dis_tbl3 = %u = 0x%x\n", table_control.smem_dis_tbl3, table_control.smem_dis_tbl3);
        bdmf_session_print(session, "smem_dis_tbl2 = %u = 0x%x\n", table_control.smem_dis_tbl2, table_control.smem_dis_tbl2);
        bdmf_session_print(session, "smem_dis_tbl1 = %u = 0x%x\n", table_control.smem_dis_tbl1, table_control.smem_dis_tbl1);
        bdmf_session_print(session, "smem_dis_tbl0 = %u = 0x%x\n", table_control.smem_dis_tbl0, table_control.smem_dis_tbl0);
        bdmf_session_print(session, "var_context_len_en_tbl7 = %u = 0x%x\n", table_control.var_context_len_en_tbl7, table_control.var_context_len_en_tbl7);
        bdmf_session_print(session, "var_context_len_en_tbl6 = %u = 0x%x\n", table_control.var_context_len_en_tbl6, table_control.var_context_len_en_tbl6);
        bdmf_session_print(session, "var_context_len_en_tbl5 = %u = 0x%x\n", table_control.var_context_len_en_tbl5, table_control.var_context_len_en_tbl5);
        bdmf_session_print(session, "var_context_len_en_tbl4 = %u = 0x%x\n", table_control.var_context_len_en_tbl4, table_control.var_context_len_en_tbl4);
        bdmf_session_print(session, "var_context_len_en_tbl3 = %u = 0x%x\n", table_control.var_context_len_en_tbl3, table_control.var_context_len_en_tbl3);
        bdmf_session_print(session, "var_context_len_en_tbl2 = %u = 0x%x\n", table_control.var_context_len_en_tbl2, table_control.var_context_len_en_tbl2);
        bdmf_session_print(session, "var_context_len_en_tbl1 = %u = 0x%x\n", table_control.var_context_len_en_tbl1, table_control.var_context_len_en_tbl1);
        bdmf_session_print(session, "var_context_len_en_tbl0 = %u = 0x%x\n", table_control.var_context_len_en_tbl0, table_control.var_context_len_en_tbl0);
        bdmf_session_print(session, "key_len_tbl7 = %u = 0x%x\n", table_control.key_len_tbl7, table_control.key_len_tbl7);
        bdmf_session_print(session, "non_cacheable_tbl7 = %u = 0x%x\n", table_control.non_cacheable_tbl7, table_control.non_cacheable_tbl7);
        bdmf_session_print(session, "key_len_tbl6 = %u = 0x%x\n", table_control.key_len_tbl6, table_control.key_len_tbl6);
        bdmf_session_print(session, "non_cacheable_tbl6 = %u = 0x%x\n", table_control.non_cacheable_tbl6, table_control.non_cacheable_tbl6);
        bdmf_session_print(session, "key_len_tbl5 = %u = 0x%x\n", table_control.key_len_tbl5, table_control.key_len_tbl5);
        bdmf_session_print(session, "non_cacheable_tbl5 = %u = 0x%x\n", table_control.non_cacheable_tbl5, table_control.non_cacheable_tbl5);
        bdmf_session_print(session, "key_len_tbl4 = %u = 0x%x\n", table_control.key_len_tbl4, table_control.key_len_tbl4);
        bdmf_session_print(session, "non_cacheable_tbl4 = %u = 0x%x\n", table_control.non_cacheable_tbl4, table_control.non_cacheable_tbl4);
        bdmf_session_print(session, "key_len_tbl3 = %u = 0x%x\n", table_control.key_len_tbl3, table_control.key_len_tbl3);
        bdmf_session_print(session, "non_cacheable_tbl3 = %u = 0x%x\n", table_control.non_cacheable_tbl3, table_control.non_cacheable_tbl3);
        bdmf_session_print(session, "key_len_tbl2 = %u = 0x%x\n", table_control.key_len_tbl2, table_control.key_len_tbl2);
        bdmf_session_print(session, "non_cacheable_tbl2 = %u = 0x%x\n", table_control.non_cacheable_tbl2, table_control.non_cacheable_tbl2);
        bdmf_session_print(session, "key_len_tbl1 = %u = 0x%x\n", table_control.key_len_tbl1, table_control.key_len_tbl1);
        bdmf_session_print(session, "non_cacheable_tbl1 = %u = 0x%x\n", table_control.non_cacheable_tbl1, table_control.non_cacheable_tbl1);
        bdmf_session_print(session, "key_len_tbl0 = %u = 0x%x\n", table_control.key_len_tbl0, table_control.key_len_tbl0);
        bdmf_session_print(session, "non_cacheable_tbl0 = %u = 0x%x\n", table_control.non_cacheable_tbl0, table_control.non_cacheable_tbl0);
        break;
    }
    case cli_natc_stat_counter_control_0:
    {
        uint8_t ddr_evict_count_en;
        uint8_t ddr_request_count_en;
        uint8_t cache_miss_count_en;
        uint8_t cache_hit_count_en;
        ag_err = ag_drv_natc_stat_counter_control_0_get(&ddr_evict_count_en, &ddr_request_count_en, &cache_miss_count_en, &cache_hit_count_en);
        bdmf_session_print(session, "ddr_evict_count_en = %u = 0x%x\n", ddr_evict_count_en, ddr_evict_count_en);
        bdmf_session_print(session, "ddr_request_count_en = %u = 0x%x\n", ddr_request_count_en, ddr_request_count_en);
        bdmf_session_print(session, "cache_miss_count_en = %u = 0x%x\n", cache_miss_count_en, cache_miss_count_en);
        bdmf_session_print(session, "cache_hit_count_en = %u = 0x%x\n", cache_hit_count_en, cache_hit_count_en);
        break;
    }
    case cli_natc_stat_counter_control_1:
    {
        bdmf_boolean counter_wraparound_dis;
        uint8_t ddr_block_count_en;
        ag_err = ag_drv_natc_stat_counter_control_1_get(&counter_wraparound_dis, &ddr_block_count_en);
        bdmf_session_print(session, "counter_wraparound_dis = %u = 0x%x\n", counter_wraparound_dis, counter_wraparound_dis);
        bdmf_session_print(session, "ddr_block_count_en = %u = 0x%x\n", ddr_block_count_en, ddr_block_count_en);
        break;
    }
    case cli_natc_regfile_fifo_start_addr_0:
    {
        uint8_t regfile_fifo_start_addr_3;
        uint8_t regfile_fifo_start_addr_2;
        uint8_t regfile_fifo_start_addr_1;
        uint8_t regfile_fifo_start_addr_0;
        ag_err = ag_drv_natc_regfile_fifo_start_addr_0_get(&regfile_fifo_start_addr_3, &regfile_fifo_start_addr_2, &regfile_fifo_start_addr_1, &regfile_fifo_start_addr_0);
        bdmf_session_print(session, "regfile_fifo_start_addr_3 = %u = 0x%x\n", regfile_fifo_start_addr_3, regfile_fifo_start_addr_3);
        bdmf_session_print(session, "regfile_fifo_start_addr_2 = %u = 0x%x\n", regfile_fifo_start_addr_2, regfile_fifo_start_addr_2);
        bdmf_session_print(session, "regfile_fifo_start_addr_1 = %u = 0x%x\n", regfile_fifo_start_addr_1, regfile_fifo_start_addr_1);
        bdmf_session_print(session, "regfile_fifo_start_addr_0 = %u = 0x%x\n", regfile_fifo_start_addr_0, regfile_fifo_start_addr_0);
        break;
    }
    case cli_natc_regfile_fifo_start_addr_1:
    {
        uint8_t regfile_fifo_start_addr_7;
        uint8_t regfile_fifo_start_addr_6;
        uint8_t regfile_fifo_start_addr_5;
        uint8_t regfile_fifo_start_addr_4;
        ag_err = ag_drv_natc_regfile_fifo_start_addr_1_get(&regfile_fifo_start_addr_7, &regfile_fifo_start_addr_6, &regfile_fifo_start_addr_5, &regfile_fifo_start_addr_4);
        bdmf_session_print(session, "regfile_fifo_start_addr_7 = %u = 0x%x\n", regfile_fifo_start_addr_7, regfile_fifo_start_addr_7);
        bdmf_session_print(session, "regfile_fifo_start_addr_6 = %u = 0x%x\n", regfile_fifo_start_addr_6, regfile_fifo_start_addr_6);
        bdmf_session_print(session, "regfile_fifo_start_addr_5 = %u = 0x%x\n", regfile_fifo_start_addr_5, regfile_fifo_start_addr_5);
        bdmf_session_print(session, "regfile_fifo_start_addr_4 = %u = 0x%x\n", regfile_fifo_start_addr_4, regfile_fifo_start_addr_4);
        break;
    }
    case cli_natc_flow_cntr_cntl:
    {
        natc_flow_cntr_cntl flow_cntr_cntl;
        ag_err = ag_drv_natc_flow_cntr_cntl_get(&flow_cntr_cntl);
        bdmf_session_print(session, "flow_cntr_en_tbl7 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl7, flow_cntr_cntl.flow_cntr_en_tbl7);
        bdmf_session_print(session, "flow_cntr_en_tbl6 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl6, flow_cntr_cntl.flow_cntr_en_tbl6);
        bdmf_session_print(session, "flow_cntr_en_tbl5 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl5, flow_cntr_cntl.flow_cntr_en_tbl5);
        bdmf_session_print(session, "flow_cntr_en_tbl4 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl4, flow_cntr_cntl.flow_cntr_en_tbl4);
        bdmf_session_print(session, "flow_cntr_en_tbl3 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl3, flow_cntr_cntl.flow_cntr_en_tbl3);
        bdmf_session_print(session, "flow_cntr_en_tbl2 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl2, flow_cntr_cntl.flow_cntr_en_tbl2);
        bdmf_session_print(session, "flow_cntr_en_tbl1 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl1, flow_cntr_cntl.flow_cntr_en_tbl1);
        bdmf_session_print(session, "flow_cntr_en_tbl0 = %u = 0x%x\n", flow_cntr_cntl.flow_cntr_en_tbl0, flow_cntr_cntl.flow_cntr_en_tbl0);
        bdmf_session_print(session, "context_offset = %u = 0x%x\n", flow_cntr_cntl.context_offset, flow_cntr_cntl.context_offset);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        natc_ctrl_status ctrl_status = {.ddr_enable = gtmv(m, 1), .natc_add_command_speedup_mode = gtmv(m, 1), .unused0 = gtmv(m, 1), .natc_init_done = gtmv(m, 1), .ddr_64bit_in_128bit_swap_control = gtmv(m, 1), .smem_32bit_in_64bit_swap_control = gtmv(m, 1), .smem_8bit_in_32bit_swap_control = gtmv(m, 1), .ddr_swap_all_control = gtmv(m, 1), .repeated_key_det_en = gtmv(m, 1), .reg_32bit_in_64bit_swap_control = gtmv(m, 1), .reg_8bit_in_32bit_swap_control = gtmv(m, 1), .ddr_pending_hash_mode = gtmv(m, 3), .pending_fifo_entry_check_enable = gtmv(m, 1), .cache_update_on_ddr_miss = gtmv(m, 1), .ddr_disable_on_reg_lookup = gtmv(m, 1), .nat_hash_mode = gtmv(m, 3), .multi_hash_limit = gtmv(m, 4), .decr_count_wraparound_enable = gtmv(m, 1), .nat_arb_st = gtmv(m, 2), .natc_smem_increment_on_reg_lookup = gtmv(m, 1), .natc_smem_clear_by_update_disable = gtmv(m, 1), .regfile_fifo_reset = gtmv(m, 1), .natc_enable = gtmv(m, 1), .natc_reset = gtmv(m, 1), .unused3 = gtmv(m, 3), .ddr_32bit_in_64bit_swap_control = gtmv(m, 1), .ddr_8bit_in_32bit_swap_control = gtmv(m, 1), .cache_lookup_blocking_mode = gtmv(m, 1), .age_timer_tick = gtmv(m, 1), .age_timer = gtmv(m, 5), .cache_algo = gtmv(m, 4), .unused2 = gtmv(m, 8), .unused1 = gtmv(m, 2), .cache_update_on_reg_ddr_lookup = gtmv(m, 1), .ddr_counter_8bit_in_32bit_swap_control = gtmv(m, 1), .ddr_hash_swap = gtmv(m, 1), .ddr_replace_duplicated_cached_entry_enable = gtmv(m, 1), .ddr_lookup_pending_fifo_mode_disable = gtmv(m, 1), .unused4 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_natc_ctrl_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            ctrl_status.ddr_enable, ctrl_status.natc_add_command_speedup_mode, ctrl_status.unused0, ctrl_status.natc_init_done, 
            ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.ddr_swap_all_control, 
            ctrl_status.repeated_key_det_en, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.ddr_pending_hash_mode, 
            ctrl_status.pending_fifo_entry_check_enable, ctrl_status.cache_update_on_ddr_miss, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.nat_hash_mode, 
            ctrl_status.multi_hash_limit, ctrl_status.decr_count_wraparound_enable, ctrl_status.nat_arb_st, ctrl_status.natc_smem_increment_on_reg_lookup, 
            ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_enable, ctrl_status.natc_reset, 
            ctrl_status.unused3, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.cache_lookup_blocking_mode, 
            ctrl_status.age_timer_tick, ctrl_status.age_timer, ctrl_status.cache_algo, ctrl_status.unused2, 
            ctrl_status.unused1, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_hash_swap, 
            ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.unused4);
        ag_err = ag_drv_natc_ctrl_status_set(&ctrl_status);
        if (!ag_err)
            ag_err = ag_drv_natc_ctrl_status_get(&ctrl_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ctrl_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                ctrl_status.ddr_enable, ctrl_status.natc_add_command_speedup_mode, ctrl_status.unused0, ctrl_status.natc_init_done, 
                ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.ddr_swap_all_control, 
                ctrl_status.repeated_key_det_en, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.ddr_pending_hash_mode, 
                ctrl_status.pending_fifo_entry_check_enable, ctrl_status.cache_update_on_ddr_miss, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.nat_hash_mode, 
                ctrl_status.multi_hash_limit, ctrl_status.decr_count_wraparound_enable, ctrl_status.nat_arb_st, ctrl_status.natc_smem_increment_on_reg_lookup, 
                ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_enable, ctrl_status.natc_reset, 
                ctrl_status.unused3, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.cache_lookup_blocking_mode, 
                ctrl_status.age_timer_tick, ctrl_status.age_timer, ctrl_status.cache_algo, ctrl_status.unused2, 
                ctrl_status.unused1, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_hash_swap, 
                ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.unused4);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ctrl_status.ddr_enable != gtmv(m, 1) || ctrl_status.natc_add_command_speedup_mode != gtmv(m, 1) || ctrl_status.unused0 != gtmv(m, 1) || ctrl_status.natc_init_done != gtmv(m, 1) || ctrl_status.ddr_64bit_in_128bit_swap_control != gtmv(m, 1) || ctrl_status.smem_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.smem_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_swap_all_control != gtmv(m, 1) || ctrl_status.repeated_key_det_en != gtmv(m, 1) || ctrl_status.reg_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.reg_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_pending_hash_mode != gtmv(m, 3) || ctrl_status.pending_fifo_entry_check_enable != gtmv(m, 1) || ctrl_status.cache_update_on_ddr_miss != gtmv(m, 1) || ctrl_status.ddr_disable_on_reg_lookup != gtmv(m, 1) || ctrl_status.nat_hash_mode != gtmv(m, 3) || ctrl_status.multi_hash_limit != gtmv(m, 4) || ctrl_status.decr_count_wraparound_enable != gtmv(m, 1) || ctrl_status.nat_arb_st != gtmv(m, 2) || ctrl_status.natc_smem_increment_on_reg_lookup != gtmv(m, 1) || ctrl_status.natc_smem_clear_by_update_disable != gtmv(m, 1) || ctrl_status.regfile_fifo_reset != gtmv(m, 1) || ctrl_status.natc_enable != gtmv(m, 1) || ctrl_status.natc_reset != gtmv(m, 1) || ctrl_status.unused3 != gtmv(m, 3) || ctrl_status.ddr_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.cache_lookup_blocking_mode != gtmv(m, 1) || ctrl_status.age_timer_tick != gtmv(m, 1) || ctrl_status.age_timer != gtmv(m, 5) || ctrl_status.cache_algo != gtmv(m, 4) || ctrl_status.unused2 != gtmv(m, 8) || ctrl_status.unused1 != gtmv(m, 2) || ctrl_status.cache_update_on_reg_ddr_lookup != gtmv(m, 1) || ctrl_status.ddr_counter_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_hash_swap != gtmv(m, 1) || ctrl_status.ddr_replace_duplicated_cached_entry_enable != gtmv(m, 1) || ctrl_status.ddr_lookup_pending_fifo_mode_disable != gtmv(m, 1) || ctrl_status.unused4 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        natc_table_control table_control = {.smem_dis_tbl7 = gtmv(m, 1), .smem_dis_tbl6 = gtmv(m, 1), .smem_dis_tbl5 = gtmv(m, 1), .smem_dis_tbl4 = gtmv(m, 1), .smem_dis_tbl3 = gtmv(m, 1), .smem_dis_tbl2 = gtmv(m, 1), .smem_dis_tbl1 = gtmv(m, 1), .smem_dis_tbl0 = gtmv(m, 1), .var_context_len_en_tbl7 = gtmv(m, 1), .var_context_len_en_tbl6 = gtmv(m, 1), .var_context_len_en_tbl5 = gtmv(m, 1), .var_context_len_en_tbl4 = gtmv(m, 1), .var_context_len_en_tbl3 = gtmv(m, 1), .var_context_len_en_tbl2 = gtmv(m, 1), .var_context_len_en_tbl1 = gtmv(m, 1), .var_context_len_en_tbl0 = gtmv(m, 1), .key_len_tbl7 = gtmv(m, 1), .non_cacheable_tbl7 = gtmv(m, 1), .key_len_tbl6 = gtmv(m, 1), .non_cacheable_tbl6 = gtmv(m, 1), .key_len_tbl5 = gtmv(m, 1), .non_cacheable_tbl5 = gtmv(m, 1), .key_len_tbl4 = gtmv(m, 1), .non_cacheable_tbl4 = gtmv(m, 1), .key_len_tbl3 = gtmv(m, 1), .non_cacheable_tbl3 = gtmv(m, 1), .key_len_tbl2 = gtmv(m, 1), .non_cacheable_tbl2 = gtmv(m, 1), .key_len_tbl1 = gtmv(m, 1), .non_cacheable_tbl1 = gtmv(m, 1), .key_len_tbl0 = gtmv(m, 1), .non_cacheable_tbl0 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_natc_table_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            table_control.smem_dis_tbl7, table_control.smem_dis_tbl6, table_control.smem_dis_tbl5, table_control.smem_dis_tbl4, 
            table_control.smem_dis_tbl3, table_control.smem_dis_tbl2, table_control.smem_dis_tbl1, table_control.smem_dis_tbl0, 
            table_control.var_context_len_en_tbl7, table_control.var_context_len_en_tbl6, table_control.var_context_len_en_tbl5, table_control.var_context_len_en_tbl4, 
            table_control.var_context_len_en_tbl3, table_control.var_context_len_en_tbl2, table_control.var_context_len_en_tbl1, table_control.var_context_len_en_tbl0, 
            table_control.key_len_tbl7, table_control.non_cacheable_tbl7, table_control.key_len_tbl6, table_control.non_cacheable_tbl6, 
            table_control.key_len_tbl5, table_control.non_cacheable_tbl5, table_control.key_len_tbl4, table_control.non_cacheable_tbl4, 
            table_control.key_len_tbl3, table_control.non_cacheable_tbl3, table_control.key_len_tbl2, table_control.non_cacheable_tbl2, 
            table_control.key_len_tbl1, table_control.non_cacheable_tbl1, table_control.key_len_tbl0, table_control.non_cacheable_tbl0);
        ag_err = ag_drv_natc_table_control_set(&table_control);
        if (!ag_err)
            ag_err = ag_drv_natc_table_control_get(&table_control);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_table_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                table_control.smem_dis_tbl7, table_control.smem_dis_tbl6, table_control.smem_dis_tbl5, table_control.smem_dis_tbl4, 
                table_control.smem_dis_tbl3, table_control.smem_dis_tbl2, table_control.smem_dis_tbl1, table_control.smem_dis_tbl0, 
                table_control.var_context_len_en_tbl7, table_control.var_context_len_en_tbl6, table_control.var_context_len_en_tbl5, table_control.var_context_len_en_tbl4, 
                table_control.var_context_len_en_tbl3, table_control.var_context_len_en_tbl2, table_control.var_context_len_en_tbl1, table_control.var_context_len_en_tbl0, 
                table_control.key_len_tbl7, table_control.non_cacheable_tbl7, table_control.key_len_tbl6, table_control.non_cacheable_tbl6, 
                table_control.key_len_tbl5, table_control.non_cacheable_tbl5, table_control.key_len_tbl4, table_control.non_cacheable_tbl4, 
                table_control.key_len_tbl3, table_control.non_cacheable_tbl3, table_control.key_len_tbl2, table_control.non_cacheable_tbl2, 
                table_control.key_len_tbl1, table_control.non_cacheable_tbl1, table_control.key_len_tbl0, table_control.non_cacheable_tbl0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (table_control.smem_dis_tbl7 != gtmv(m, 1) || table_control.smem_dis_tbl6 != gtmv(m, 1) || table_control.smem_dis_tbl5 != gtmv(m, 1) || table_control.smem_dis_tbl4 != gtmv(m, 1) || table_control.smem_dis_tbl3 != gtmv(m, 1) || table_control.smem_dis_tbl2 != gtmv(m, 1) || table_control.smem_dis_tbl1 != gtmv(m, 1) || table_control.smem_dis_tbl0 != gtmv(m, 1) || table_control.var_context_len_en_tbl7 != gtmv(m, 1) || table_control.var_context_len_en_tbl6 != gtmv(m, 1) || table_control.var_context_len_en_tbl5 != gtmv(m, 1) || table_control.var_context_len_en_tbl4 != gtmv(m, 1) || table_control.var_context_len_en_tbl3 != gtmv(m, 1) || table_control.var_context_len_en_tbl2 != gtmv(m, 1) || table_control.var_context_len_en_tbl1 != gtmv(m, 1) || table_control.var_context_len_en_tbl0 != gtmv(m, 1) || table_control.key_len_tbl7 != gtmv(m, 1) || table_control.non_cacheable_tbl7 != gtmv(m, 1) || table_control.key_len_tbl6 != gtmv(m, 1) || table_control.non_cacheable_tbl6 != gtmv(m, 1) || table_control.key_len_tbl5 != gtmv(m, 1) || table_control.non_cacheable_tbl5 != gtmv(m, 1) || table_control.key_len_tbl4 != gtmv(m, 1) || table_control.non_cacheable_tbl4 != gtmv(m, 1) || table_control.key_len_tbl3 != gtmv(m, 1) || table_control.non_cacheable_tbl3 != gtmv(m, 1) || table_control.key_len_tbl2 != gtmv(m, 1) || table_control.non_cacheable_tbl2 != gtmv(m, 1) || table_control.key_len_tbl1 != gtmv(m, 1) || table_control.non_cacheable_tbl1 != gtmv(m, 1) || table_control.key_len_tbl0 != gtmv(m, 1) || table_control.non_cacheable_tbl0 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_evict_count_en = gtmv(m, 8);
        uint8_t ddr_request_count_en = gtmv(m, 8);
        uint8_t cache_miss_count_en = gtmv(m, 8);
        uint8_t cache_hit_count_en = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_stat_counter_control_0_set( %u %u %u %u)\n",
            ddr_evict_count_en, ddr_request_count_en, cache_miss_count_en, cache_hit_count_en);
        ag_err = ag_drv_natc_stat_counter_control_0_set(ddr_evict_count_en, ddr_request_count_en, cache_miss_count_en, cache_hit_count_en);
        if (!ag_err)
            ag_err = ag_drv_natc_stat_counter_control_0_get(&ddr_evict_count_en, &ddr_request_count_en, &cache_miss_count_en, &cache_hit_count_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_stat_counter_control_0_get( %u %u %u %u)\n",
                ddr_evict_count_en, ddr_request_count_en, cache_miss_count_en, cache_hit_count_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_evict_count_en != gtmv(m, 8) || ddr_request_count_en != gtmv(m, 8) || cache_miss_count_en != gtmv(m, 8) || cache_hit_count_en != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean counter_wraparound_dis = gtmv(m, 1);
        uint8_t ddr_block_count_en = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_stat_counter_control_1_set( %u %u)\n",
            counter_wraparound_dis, ddr_block_count_en);
        ag_err = ag_drv_natc_stat_counter_control_1_set(counter_wraparound_dis, ddr_block_count_en);
        if (!ag_err)
            ag_err = ag_drv_natc_stat_counter_control_1_get(&counter_wraparound_dis, &ddr_block_count_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_stat_counter_control_1_get( %u %u)\n",
                counter_wraparound_dis, ddr_block_count_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (counter_wraparound_dis != gtmv(m, 1) || ddr_block_count_en != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t regfile_fifo_start_addr_3 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_2 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_1 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_0 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_regfile_fifo_start_addr_0_set( %u %u %u %u)\n",
            regfile_fifo_start_addr_3, regfile_fifo_start_addr_2, regfile_fifo_start_addr_1, regfile_fifo_start_addr_0);
        ag_err = ag_drv_natc_regfile_fifo_start_addr_0_set(regfile_fifo_start_addr_3, regfile_fifo_start_addr_2, regfile_fifo_start_addr_1, regfile_fifo_start_addr_0);
        if (!ag_err)
            ag_err = ag_drv_natc_regfile_fifo_start_addr_0_get(&regfile_fifo_start_addr_3, &regfile_fifo_start_addr_2, &regfile_fifo_start_addr_1, &regfile_fifo_start_addr_0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_regfile_fifo_start_addr_0_get( %u %u %u %u)\n",
                regfile_fifo_start_addr_3, regfile_fifo_start_addr_2, regfile_fifo_start_addr_1, regfile_fifo_start_addr_0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (regfile_fifo_start_addr_3 != gtmv(m, 8) || regfile_fifo_start_addr_2 != gtmv(m, 8) || regfile_fifo_start_addr_1 != gtmv(m, 8) || regfile_fifo_start_addr_0 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t regfile_fifo_start_addr_7 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_6 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_5 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_4 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_regfile_fifo_start_addr_1_set( %u %u %u %u)\n",
            regfile_fifo_start_addr_7, regfile_fifo_start_addr_6, regfile_fifo_start_addr_5, regfile_fifo_start_addr_4);
        ag_err = ag_drv_natc_regfile_fifo_start_addr_1_set(regfile_fifo_start_addr_7, regfile_fifo_start_addr_6, regfile_fifo_start_addr_5, regfile_fifo_start_addr_4);
        if (!ag_err)
            ag_err = ag_drv_natc_regfile_fifo_start_addr_1_get(&regfile_fifo_start_addr_7, &regfile_fifo_start_addr_6, &regfile_fifo_start_addr_5, &regfile_fifo_start_addr_4);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_regfile_fifo_start_addr_1_get( %u %u %u %u)\n",
                regfile_fifo_start_addr_7, regfile_fifo_start_addr_6, regfile_fifo_start_addr_5, regfile_fifo_start_addr_4);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (regfile_fifo_start_addr_7 != gtmv(m, 8) || regfile_fifo_start_addr_6 != gtmv(m, 8) || regfile_fifo_start_addr_5 != gtmv(m, 8) || regfile_fifo_start_addr_4 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        natc_flow_cntr_cntl flow_cntr_cntl = {.flow_cntr_en_tbl7 = gtmv(m, 1), .flow_cntr_en_tbl6 = gtmv(m, 1), .flow_cntr_en_tbl5 = gtmv(m, 1), .flow_cntr_en_tbl4 = gtmv(m, 1), .flow_cntr_en_tbl3 = gtmv(m, 1), .flow_cntr_en_tbl2 = gtmv(m, 1), .flow_cntr_en_tbl1 = gtmv(m, 1), .flow_cntr_en_tbl0 = gtmv(m, 1), .context_offset = gtmv(m, 7)};
        bdmf_session_print(session, "ag_drv_natc_flow_cntr_cntl_set( %u %u %u %u %u %u %u %u %u)\n",
            flow_cntr_cntl.flow_cntr_en_tbl7, flow_cntr_cntl.flow_cntr_en_tbl6, flow_cntr_cntl.flow_cntr_en_tbl5, flow_cntr_cntl.flow_cntr_en_tbl4, 
            flow_cntr_cntl.flow_cntr_en_tbl3, flow_cntr_cntl.flow_cntr_en_tbl2, flow_cntr_cntl.flow_cntr_en_tbl1, flow_cntr_cntl.flow_cntr_en_tbl0, 
            flow_cntr_cntl.context_offset);
        ag_err = ag_drv_natc_flow_cntr_cntl_set(&flow_cntr_cntl);
        if (!ag_err)
            ag_err = ag_drv_natc_flow_cntr_cntl_get(&flow_cntr_cntl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_flow_cntr_cntl_get( %u %u %u %u %u %u %u %u %u)\n",
                flow_cntr_cntl.flow_cntr_en_tbl7, flow_cntr_cntl.flow_cntr_en_tbl6, flow_cntr_cntl.flow_cntr_en_tbl5, flow_cntr_cntl.flow_cntr_en_tbl4, 
                flow_cntr_cntl.flow_cntr_en_tbl3, flow_cntr_cntl.flow_cntr_en_tbl2, flow_cntr_cntl.flow_cntr_en_tbl1, flow_cntr_cntl.flow_cntr_en_tbl0, 
                flow_cntr_cntl.context_offset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow_cntr_cntl.flow_cntr_en_tbl7 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl6 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl5 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl4 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl3 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl2 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl1 != gtmv(m, 1) || flow_cntr_cntl.flow_cntr_en_tbl0 != gtmv(m, 1) || flow_cntr_cntl.context_offset != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_control_status: reg = &RU_REG(NATC, CONTROL_STATUS); blk = &RU_BLK(NATC); break;
    case bdmf_address_control_status2: reg = &RU_REG(NATC, CONTROL_STATUS2); blk = &RU_BLK(NATC); break;
    case bdmf_address_table_control: reg = &RU_REG(NATC, TABLE_CONTROL); blk = &RU_BLK(NATC); break;
    case bdmf_address_stat_counter_control_0: reg = &RU_REG(NATC, STAT_COUNTER_CONTROL_0); blk = &RU_BLK(NATC); break;
    case bdmf_address_stat_counter_control_1: reg = &RU_REG(NATC, STAT_COUNTER_CONTROL_1); blk = &RU_BLK(NATC); break;
    case bdmf_address_regfile_fifo_start_addr_0: reg = &RU_REG(NATC, REGFILE_FIFO_START_ADDR_0); blk = &RU_BLK(NATC); break;
    case bdmf_address_regfile_fifo_start_addr_1: reg = &RU_REG(NATC, REGFILE_FIFO_START_ADDR_1); blk = &RU_BLK(NATC); break;
    case bdmf_address_flow_cntr_cntl: reg = &RU_REG(NATC, FLOW_CNTR_CNTL); blk = &RU_BLK(NATC); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_natc_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc", "natc", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_ctrl_status[] = {
            BDMFMON_MAKE_PARM("ddr_enable", "ddr_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_add_command_speedup_mode", "natc_add_command_speedup_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused0", "unused0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_init_done", "natc_init_done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_64bit_in_128bit_swap_control", "ddr_64bit_in_128bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_32bit_in_64bit_swap_control", "smem_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_8bit_in_32bit_swap_control", "smem_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_swap_all_control", "ddr_swap_all_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("repeated_key_det_en", "repeated_key_det_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reg_32bit_in_64bit_swap_control", "reg_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reg_8bit_in_32bit_swap_control", "reg_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pending_hash_mode", "ddr_pending_hash_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pending_fifo_entry_check_enable", "pending_fifo_entry_check_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_ddr_miss", "cache_update_on_ddr_miss", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_disable_on_reg_lookup", "ddr_disable_on_reg_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nat_hash_mode", "nat_hash_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_hash_limit", "multi_hash_limit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("decr_count_wraparound_enable", "decr_count_wraparound_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nat_arb_st", "nat_arb_st", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_increment_on_reg_lookup", "natc_smem_increment_on_reg_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_clear_by_update_disable", "natc_smem_clear_by_update_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_reset", "regfile_fifo_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_enable", "natc_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_reset", "natc_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused3", "unused3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_32bit_in_64bit_swap_control", "ddr_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_8bit_in_32bit_swap_control", "ddr_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_lookup_blocking_mode", "cache_lookup_blocking_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer_tick", "age_timer_tick", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer", "age_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_algo", "cache_algo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused2", "unused2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused1", "unused1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_reg_ddr_lookup", "cache_update_on_reg_ddr_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_counter_8bit_in_32bit_swap_control", "ddr_counter_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_swap", "ddr_hash_swap", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_replace_duplicated_cached_entry_enable", "ddr_replace_duplicated_cached_entry_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_lookup_pending_fifo_mode_disable", "ddr_lookup_pending_fifo_mode_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused4", "unused4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_table_control[] = {
            BDMFMON_MAKE_PARM("smem_dis_tbl7", "smem_dis_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl6", "smem_dis_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl5", "smem_dis_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl4", "smem_dis_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl3", "smem_dis_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl2", "smem_dis_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl1", "smem_dis_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl0", "smem_dis_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl7", "var_context_len_en_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl6", "var_context_len_en_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl5", "var_context_len_en_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl4", "var_context_len_en_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl3", "var_context_len_en_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl2", "var_context_len_en_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl1", "var_context_len_en_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl0", "var_context_len_en_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl7", "key_len_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl7", "non_cacheable_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl6", "key_len_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl6", "non_cacheable_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl5", "key_len_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl5", "non_cacheable_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl4", "key_len_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl4", "non_cacheable_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl3", "key_len_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl3", "non_cacheable_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl2", "key_len_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl2", "non_cacheable_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl1", "key_len_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl1", "non_cacheable_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl0", "key_len_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl0", "non_cacheable_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_stat_counter_control_0[] = {
            BDMFMON_MAKE_PARM("ddr_evict_count_en", "ddr_evict_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_request_count_en", "ddr_request_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_miss_count_en", "cache_miss_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_hit_count_en", "cache_hit_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_stat_counter_control_1[] = {
            BDMFMON_MAKE_PARM("counter_wraparound_dis", "counter_wraparound_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_block_count_en", "ddr_block_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regfile_fifo_start_addr_0[] = {
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_3", "regfile_fifo_start_addr_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_2", "regfile_fifo_start_addr_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_1", "regfile_fifo_start_addr_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_0", "regfile_fifo_start_addr_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_regfile_fifo_start_addr_1[] = {
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_7", "regfile_fifo_start_addr_7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_6", "regfile_fifo_start_addr_6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_5", "regfile_fifo_start_addr_5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_4", "regfile_fifo_start_addr_4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_cntr_cntl[] = {
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl7", "flow_cntr_en_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl6", "flow_cntr_en_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl5", "flow_cntr_en_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl4", "flow_cntr_en_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl3", "flow_cntr_en_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl2", "flow_cntr_en_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl1", "flow_cntr_en_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl0", "flow_cntr_en_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("context_offset", "context_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ctrl_status", .val = cli_natc_ctrl_status, .parms = set_ctrl_status },
            { .name = "table_control", .val = cli_natc_table_control, .parms = set_table_control },
            { .name = "stat_counter_control_0", .val = cli_natc_stat_counter_control_0, .parms = set_stat_counter_control_0 },
            { .name = "stat_counter_control_1", .val = cli_natc_stat_counter_control_1, .parms = set_stat_counter_control_1 },
            { .name = "regfile_fifo_start_addr_0", .val = cli_natc_regfile_fifo_start_addr_0, .parms = set_regfile_fifo_start_addr_0 },
            { .name = "regfile_fifo_start_addr_1", .val = cli_natc_regfile_fifo_start_addr_1, .parms = set_regfile_fifo_start_addr_1 },
            { .name = "flow_cntr_cntl", .val = cli_natc_flow_cntr_cntl, .parms = set_flow_cntr_cntl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ctrl_status", .val = cli_natc_ctrl_status, .parms = get_default },
            { .name = "table_control", .val = cli_natc_table_control, .parms = get_default },
            { .name = "stat_counter_control_0", .val = cli_natc_stat_counter_control_0, .parms = get_default },
            { .name = "stat_counter_control_1", .val = cli_natc_stat_counter_control_1, .parms = get_default },
            { .name = "regfile_fifo_start_addr_0", .val = cli_natc_regfile_fifo_start_addr_0, .parms = get_default },
            { .name = "regfile_fifo_start_addr_1", .val = cli_natc_regfile_fifo_start_addr_1, .parms = get_default },
            { .name = "flow_cntr_cntl", .val = cli_natc_flow_cntr_cntl, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CONTROL_STATUS", .val = bdmf_address_control_status },
            { .name = "CONTROL_STATUS2", .val = bdmf_address_control_status2 },
            { .name = "TABLE_CONTROL", .val = bdmf_address_table_control },
            { .name = "STAT_COUNTER_CONTROL_0", .val = bdmf_address_stat_counter_control_0 },
            { .name = "STAT_COUNTER_CONTROL_1", .val = bdmf_address_stat_counter_control_1 },
            { .name = "REGFILE_FIFO_START_ADDR_0", .val = bdmf_address_regfile_fifo_start_addr_0 },
            { .name = "REGFILE_FIFO_START_ADDR_1", .val = bdmf_address_regfile_fifo_start_addr_1 },
            { .name = "FLOW_CNTR_CNTL", .val = bdmf_address_flow_cntr_cntl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
