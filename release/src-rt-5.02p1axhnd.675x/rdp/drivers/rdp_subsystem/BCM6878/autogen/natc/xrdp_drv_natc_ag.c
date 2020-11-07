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

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_natc_ag.h"

bdmf_error_t ag_drv_natc_ctrl_status_set(const natc_ctrl_status *ctrl_status)
{
    uint32_t reg_control_status=0;
    uint32_t reg_control_status2=0;

#ifdef VALIDATE_PARMS
    if(!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctrl_status->ddr_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_add_command_speedup_mode >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused4 >= _2BITS_MAX_VAL_) ||
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
       (ctrl_status->ddr_hash_mode >= _3BITS_MAX_VAL_) ||
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
       (ctrl_status->unused0 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, DDR_ENABLE, reg_control_status, ctrl_status->ddr_enable);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_control_status, ctrl_status->natc_add_command_speedup_mode);
    reg_control_status = RU_FIELD_SET(0, NATC, CONTROL_STATUS, UNUSED4, reg_control_status, ctrl_status->unused4);
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
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, DDR_HASH_MODE, reg_control_status2, ctrl_status->ddr_hash_mode);
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
    reg_control_status2 = RU_FIELD_SET(0, NATC, CONTROL_STATUS2, UNUSED0, reg_control_status2, ctrl_status->unused0);

    RU_REG_WRITE(0, NATC, CONTROL_STATUS, reg_control_status);
    RU_REG_WRITE(0, NATC, CONTROL_STATUS2, reg_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ctrl_status_get(natc_ctrl_status *ctrl_status)
{
    uint32_t reg_control_status;
    uint32_t reg_control_status2;

#ifdef VALIDATE_PARMS
    if(!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, CONTROL_STATUS, reg_control_status);
    RU_REG_READ(0, NATC, CONTROL_STATUS2, reg_control_status2);

    ctrl_status->ddr_enable = RU_FIELD_GET(0, NATC, CONTROL_STATUS, DDR_ENABLE, reg_control_status);
    ctrl_status->natc_add_command_speedup_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_control_status);
    ctrl_status->unused4 = RU_FIELD_GET(0, NATC, CONTROL_STATUS, UNUSED4, reg_control_status);
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
    ctrl_status->ddr_hash_mode = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, DDR_HASH_MODE, reg_control_status2);
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
    ctrl_status->unused0 = RU_FIELD_GET(0, NATC, CONTROL_STATUS2, UNUSED0, reg_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ctrs_control_set(const natc_ctrs_control *ctrs_control)
{
    uint32_t reg_stat_counter_control_0=0;
    uint32_t reg_stat_counter_control_1=0;

#ifdef VALIDATE_PARMS
    if(!ctrs_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctrs_control->counter_wraparound_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_stat_counter_control_0, ctrs_control->ddr_evict_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_stat_counter_control_0, ctrs_control->ddr_request_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_stat_counter_control_0, ctrs_control->cache_miss_count_en);
    reg_stat_counter_control_0 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_stat_counter_control_0, ctrs_control->cache_hit_count_en);
    reg_stat_counter_control_1 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_stat_counter_control_1, ctrs_control->counter_wraparound_dis);
    reg_stat_counter_control_1 = RU_FIELD_SET(0, NATC, STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_stat_counter_control_1, ctrs_control->ddr_block_count_en);

    RU_REG_WRITE(0, NATC, STAT_COUNTER_CONTROL_0, reg_stat_counter_control_0);
    RU_REG_WRITE(0, NATC, STAT_COUNTER_CONTROL_1, reg_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ctrs_control_get(natc_ctrs_control *ctrs_control)
{
    uint32_t reg_stat_counter_control_0;
    uint32_t reg_stat_counter_control_1;

#ifdef VALIDATE_PARMS
    if(!ctrs_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, STAT_COUNTER_CONTROL_0, reg_stat_counter_control_0);
    RU_REG_READ(0, NATC, STAT_COUNTER_CONTROL_1, reg_stat_counter_control_1);

    ctrs_control->ddr_evict_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_stat_counter_control_0);
    ctrs_control->ddr_request_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_stat_counter_control_0);
    ctrs_control->cache_miss_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_stat_counter_control_0);
    ctrs_control->cache_hit_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_stat_counter_control_0);
    ctrs_control->counter_wraparound_dis = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_stat_counter_control_1);
    ctrs_control->ddr_block_count_en = RU_FIELD_GET(0, NATC, STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_table_control_set(uint8_t tbl_idx, uint8_t data)
{
    uint32_t reg_table_control=0;

#ifdef VALIDATE_PARMS
    if((data >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, NATC, TABLE_CONTROL, reg_table_control);

    FIELD_SET(reg_table_control, (tbl_idx % 16) *2, 0x2, data);

    RU_REG_WRITE(0, NATC, TABLE_CONTROL, reg_table_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_table_control_get(uint8_t tbl_idx, uint8_t *data)
{
    uint32_t reg_table_control;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC, TABLE_CONTROL, reg_table_control);

    *data = FIELD_GET(reg_table_control, (tbl_idx % 16) *2, 0x2);

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
}
bdmf_address;

static int bcm_natc_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_ctrl_status:
    {
        natc_ctrl_status ctrl_status = { .ddr_enable=parm[1].value.unumber, .natc_add_command_speedup_mode=parm[2].value.unumber, .unused4=parm[3].value.unumber, .ddr_64bit_in_128bit_swap_control=parm[4].value.unumber, .smem_32bit_in_64bit_swap_control=parm[5].value.unumber, .smem_8bit_in_32bit_swap_control=parm[6].value.unumber, .ddr_swap_all_control=parm[7].value.unumber, .repeated_key_det_en=parm[8].value.unumber, .reg_32bit_in_64bit_swap_control=parm[9].value.unumber, .reg_8bit_in_32bit_swap_control=parm[10].value.unumber, .ddr_pending_hash_mode=parm[11].value.unumber, .pending_fifo_entry_check_enable=parm[12].value.unumber, .cache_update_on_ddr_miss=parm[13].value.unumber, .ddr_disable_on_reg_lookup=parm[14].value.unumber, .nat_hash_mode=parm[15].value.unumber, .multi_hash_limit=parm[16].value.unumber, .decr_count_wraparound_enable=parm[17].value.unumber, .nat_arb_st=parm[18].value.unumber, .natc_smem_increment_on_reg_lookup=parm[19].value.unumber, .natc_smem_clear_by_update_disable=parm[20].value.unumber, .regfile_fifo_reset=parm[21].value.unumber, .natc_enable=parm[22].value.unumber, .natc_reset=parm[23].value.unumber, .ddr_hash_mode=parm[24].value.unumber, .ddr_32bit_in_64bit_swap_control=parm[25].value.unumber, .ddr_8bit_in_32bit_swap_control=parm[26].value.unumber, .cache_lookup_blocking_mode=parm[27].value.unumber, .age_timer_tick=parm[28].value.unumber, .age_timer=parm[29].value.unumber, .cache_algo=parm[30].value.unumber, .unused2=parm[31].value.unumber, .unused1=parm[32].value.unumber, .cache_update_on_reg_ddr_lookup=parm[33].value.unumber, .ddr_counter_8bit_in_32bit_swap_control=parm[34].value.unumber, .ddr_hash_swap=parm[35].value.unumber, .ddr_replace_duplicated_cached_entry_enable=parm[36].value.unumber, .ddr_lookup_pending_fifo_mode_disable=parm[37].value.unumber, .unused0=parm[38].value.unumber};
        err = ag_drv_natc_ctrl_status_set(&ctrl_status);
        break;
    }
    case cli_natc_ctrs_control:
    {
        natc_ctrs_control ctrs_control = { .ddr_evict_count_en=parm[1].value.unumber, .ddr_request_count_en=parm[2].value.unumber, .cache_miss_count_en=parm[3].value.unumber, .cache_hit_count_en=parm[4].value.unumber, .counter_wraparound_dis=parm[5].value.unumber, .ddr_block_count_en=parm[6].value.unumber};
        err = ag_drv_natc_ctrs_control_set(&ctrs_control);
        break;
    }
    case cli_natc_table_control:
        err = ag_drv_natc_table_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_natc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_ctrl_status:
    {
        natc_ctrl_status ctrl_status;
        err = ag_drv_natc_ctrl_status_get(&ctrl_status);
        bdmf_session_print(session, "ddr_enable = %u (0x%x)\n", ctrl_status.ddr_enable, ctrl_status.ddr_enable);
        bdmf_session_print(session, "natc_add_command_speedup_mode = %u (0x%x)\n", ctrl_status.natc_add_command_speedup_mode, ctrl_status.natc_add_command_speedup_mode);
        bdmf_session_print(session, "unused4 = %u (0x%x)\n", ctrl_status.unused4, ctrl_status.unused4);
        bdmf_session_print(session, "ddr_64bit_in_128bit_swap_control = %u (0x%x)\n", ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.ddr_64bit_in_128bit_swap_control);
        bdmf_session_print(session, "smem_32bit_in_64bit_swap_control = %u (0x%x)\n", ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "smem_8bit_in_32bit_swap_control = %u (0x%x)\n", ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_swap_all_control = %u (0x%x)\n", ctrl_status.ddr_swap_all_control, ctrl_status.ddr_swap_all_control);
        bdmf_session_print(session, "repeated_key_det_en = %u (0x%x)\n", ctrl_status.repeated_key_det_en, ctrl_status.repeated_key_det_en);
        bdmf_session_print(session, "reg_32bit_in_64bit_swap_control = %u (0x%x)\n", ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "reg_8bit_in_32bit_swap_control = %u (0x%x)\n", ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_pending_hash_mode = %u (0x%x)\n", ctrl_status.ddr_pending_hash_mode, ctrl_status.ddr_pending_hash_mode);
        bdmf_session_print(session, "pending_fifo_entry_check_enable = %u (0x%x)\n", ctrl_status.pending_fifo_entry_check_enable, ctrl_status.pending_fifo_entry_check_enable);
        bdmf_session_print(session, "cache_update_on_ddr_miss = %u (0x%x)\n", ctrl_status.cache_update_on_ddr_miss, ctrl_status.cache_update_on_ddr_miss);
        bdmf_session_print(session, "ddr_disable_on_reg_lookup = %u (0x%x)\n", ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.ddr_disable_on_reg_lookup);
        bdmf_session_print(session, "nat_hash_mode = %u (0x%x)\n", ctrl_status.nat_hash_mode, ctrl_status.nat_hash_mode);
        bdmf_session_print(session, "multi_hash_limit = %u (0x%x)\n", ctrl_status.multi_hash_limit, ctrl_status.multi_hash_limit);
        bdmf_session_print(session, "decr_count_wraparound_enable = %u (0x%x)\n", ctrl_status.decr_count_wraparound_enable, ctrl_status.decr_count_wraparound_enable);
        bdmf_session_print(session, "nat_arb_st = %u (0x%x)\n", ctrl_status.nat_arb_st, ctrl_status.nat_arb_st);
        bdmf_session_print(session, "natc_smem_increment_on_reg_lookup = %u (0x%x)\n", ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.natc_smem_increment_on_reg_lookup);
        bdmf_session_print(session, "natc_smem_clear_by_update_disable = %u (0x%x)\n", ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.natc_smem_clear_by_update_disable);
        bdmf_session_print(session, "regfile_fifo_reset = %u (0x%x)\n", ctrl_status.regfile_fifo_reset, ctrl_status.regfile_fifo_reset);
        bdmf_session_print(session, "natc_enable = %u (0x%x)\n", ctrl_status.natc_enable, ctrl_status.natc_enable);
        bdmf_session_print(session, "natc_reset = %u (0x%x)\n", ctrl_status.natc_reset, ctrl_status.natc_reset);
        bdmf_session_print(session, "ddr_hash_mode = %u (0x%x)\n", ctrl_status.ddr_hash_mode, ctrl_status.ddr_hash_mode);
        bdmf_session_print(session, "ddr_32bit_in_64bit_swap_control = %u (0x%x)\n", ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "ddr_8bit_in_32bit_swap_control = %u (0x%x)\n", ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "cache_lookup_blocking_mode = %u (0x%x)\n", ctrl_status.cache_lookup_blocking_mode, ctrl_status.cache_lookup_blocking_mode);
        bdmf_session_print(session, "age_timer_tick = %u (0x%x)\n", ctrl_status.age_timer_tick, ctrl_status.age_timer_tick);
        bdmf_session_print(session, "age_timer = %u (0x%x)\n", ctrl_status.age_timer, ctrl_status.age_timer);
        bdmf_session_print(session, "cache_algo = %u (0x%x)\n", ctrl_status.cache_algo, ctrl_status.cache_algo);
        bdmf_session_print(session, "unused2 = %u (0x%x)\n", ctrl_status.unused2, ctrl_status.unused2);
        bdmf_session_print(session, "unused1 = %u (0x%x)\n", ctrl_status.unused1, ctrl_status.unused1);
        bdmf_session_print(session, "cache_update_on_reg_ddr_lookup = %u (0x%x)\n", ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.cache_update_on_reg_ddr_lookup);
        bdmf_session_print(session, "ddr_counter_8bit_in_32bit_swap_control = %u (0x%x)\n", ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_counter_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_hash_swap = %u (0x%x)\n", ctrl_status.ddr_hash_swap, ctrl_status.ddr_hash_swap);
        bdmf_session_print(session, "ddr_replace_duplicated_cached_entry_enable = %u (0x%x)\n", ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_replace_duplicated_cached_entry_enable);
        bdmf_session_print(session, "ddr_lookup_pending_fifo_mode_disable = %u (0x%x)\n", ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.ddr_lookup_pending_fifo_mode_disable);
        bdmf_session_print(session, "unused0 = %u (0x%x)\n", ctrl_status.unused0, ctrl_status.unused0);
        break;
    }
    case cli_natc_ctrs_control:
    {
        natc_ctrs_control ctrs_control;
        err = ag_drv_natc_ctrs_control_get(&ctrs_control);
        bdmf_session_print(session, "ddr_evict_count_en = %u (0x%x)\n", ctrs_control.ddr_evict_count_en, ctrs_control.ddr_evict_count_en);
        bdmf_session_print(session, "ddr_request_count_en = %u (0x%x)\n", ctrs_control.ddr_request_count_en, ctrs_control.ddr_request_count_en);
        bdmf_session_print(session, "cache_miss_count_en = %u (0x%x)\n", ctrs_control.cache_miss_count_en, ctrs_control.cache_miss_count_en);
        bdmf_session_print(session, "cache_hit_count_en = %u (0x%x)\n", ctrs_control.cache_hit_count_en, ctrs_control.cache_hit_count_en);
        bdmf_session_print(session, "counter_wraparound_dis = %u (0x%x)\n", ctrs_control.counter_wraparound_dis, ctrs_control.counter_wraparound_dis);
        bdmf_session_print(session, "ddr_block_count_en = %u (0x%x)\n", ctrs_control.ddr_block_count_en, ctrs_control.ddr_block_count_en);
        break;
    }
    case cli_natc_table_control:
    {
        uint8_t data;
        err = ag_drv_natc_table_control_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_natc_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        natc_ctrl_status ctrl_status = {.ddr_enable=gtmv(m, 1), .natc_add_command_speedup_mode=gtmv(m, 1), .unused4=gtmv(m, 2), .ddr_64bit_in_128bit_swap_control=gtmv(m, 1), .smem_32bit_in_64bit_swap_control=gtmv(m, 1), .smem_8bit_in_32bit_swap_control=gtmv(m, 1), .ddr_swap_all_control=gtmv(m, 1), .repeated_key_det_en=gtmv(m, 1), .reg_32bit_in_64bit_swap_control=gtmv(m, 1), .reg_8bit_in_32bit_swap_control=gtmv(m, 1), .ddr_pending_hash_mode=gtmv(m, 3), .pending_fifo_entry_check_enable=gtmv(m, 1), .cache_update_on_ddr_miss=gtmv(m, 1), .ddr_disable_on_reg_lookup=gtmv(m, 1), .nat_hash_mode=gtmv(m, 3), .multi_hash_limit=gtmv(m, 4), .decr_count_wraparound_enable=gtmv(m, 1), .nat_arb_st=gtmv(m, 2), .natc_smem_increment_on_reg_lookup=gtmv(m, 1), .natc_smem_clear_by_update_disable=gtmv(m, 1), .regfile_fifo_reset=gtmv(m, 1), .natc_enable=gtmv(m, 1), .natc_reset=gtmv(m, 1), .ddr_hash_mode=gtmv(m, 3), .ddr_32bit_in_64bit_swap_control=gtmv(m, 1), .ddr_8bit_in_32bit_swap_control=gtmv(m, 1), .cache_lookup_blocking_mode=gtmv(m, 1), .age_timer_tick=gtmv(m, 1), .age_timer=gtmv(m, 5), .cache_algo=gtmv(m, 4), .unused2=gtmv(m, 8), .unused1=gtmv(m, 2), .cache_update_on_reg_ddr_lookup=gtmv(m, 1), .ddr_counter_8bit_in_32bit_swap_control=gtmv(m, 1), .ddr_hash_swap=gtmv(m, 1), .ddr_replace_duplicated_cached_entry_enable=gtmv(m, 1), .ddr_lookup_pending_fifo_mode_disable=gtmv(m, 1), .unused0=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_ctrl_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctrl_status.ddr_enable, ctrl_status.natc_add_command_speedup_mode, ctrl_status.unused4, ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.ddr_swap_all_control, ctrl_status.repeated_key_det_en, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.ddr_pending_hash_mode, ctrl_status.pending_fifo_entry_check_enable, ctrl_status.cache_update_on_ddr_miss, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.nat_hash_mode, ctrl_status.multi_hash_limit, ctrl_status.decr_count_wraparound_enable, ctrl_status.nat_arb_st, ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_enable, ctrl_status.natc_reset, ctrl_status.ddr_hash_mode, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.cache_lookup_blocking_mode, ctrl_status.age_timer_tick, ctrl_status.age_timer, ctrl_status.cache_algo, ctrl_status.unused2, ctrl_status.unused1, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_hash_swap, ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.unused0);
        if(!err) ag_drv_natc_ctrl_status_set(&ctrl_status);
        if(!err) ag_drv_natc_ctrl_status_get( &ctrl_status);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ctrl_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctrl_status.ddr_enable, ctrl_status.natc_add_command_speedup_mode, ctrl_status.unused4, ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.ddr_swap_all_control, ctrl_status.repeated_key_det_en, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.ddr_pending_hash_mode, ctrl_status.pending_fifo_entry_check_enable, ctrl_status.cache_update_on_ddr_miss, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.nat_hash_mode, ctrl_status.multi_hash_limit, ctrl_status.decr_count_wraparound_enable, ctrl_status.nat_arb_st, ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_enable, ctrl_status.natc_reset, ctrl_status.ddr_hash_mode, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.cache_lookup_blocking_mode, ctrl_status.age_timer_tick, ctrl_status.age_timer, ctrl_status.cache_algo, ctrl_status.unused2, ctrl_status.unused1, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_hash_swap, ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.unused0);
        if(err || ctrl_status.ddr_enable!=gtmv(m, 1) || ctrl_status.natc_add_command_speedup_mode!=gtmv(m, 1) || ctrl_status.unused4!=gtmv(m, 2) || ctrl_status.ddr_64bit_in_128bit_swap_control!=gtmv(m, 1) || ctrl_status.smem_32bit_in_64bit_swap_control!=gtmv(m, 1) || ctrl_status.smem_8bit_in_32bit_swap_control!=gtmv(m, 1) || ctrl_status.ddr_swap_all_control!=gtmv(m, 1) || ctrl_status.repeated_key_det_en!=gtmv(m, 1) || ctrl_status.reg_32bit_in_64bit_swap_control!=gtmv(m, 1) || ctrl_status.reg_8bit_in_32bit_swap_control!=gtmv(m, 1) || ctrl_status.ddr_pending_hash_mode!=gtmv(m, 3) || ctrl_status.pending_fifo_entry_check_enable!=gtmv(m, 1) || ctrl_status.cache_update_on_ddr_miss!=gtmv(m, 1) || ctrl_status.ddr_disable_on_reg_lookup!=gtmv(m, 1) || ctrl_status.nat_hash_mode!=gtmv(m, 3) || ctrl_status.multi_hash_limit!=gtmv(m, 4) || ctrl_status.decr_count_wraparound_enable!=gtmv(m, 1) || ctrl_status.nat_arb_st!=gtmv(m, 2) || ctrl_status.natc_smem_increment_on_reg_lookup!=gtmv(m, 1) || ctrl_status.natc_smem_clear_by_update_disable!=gtmv(m, 1) || ctrl_status.regfile_fifo_reset!=gtmv(m, 1) || ctrl_status.natc_enable!=gtmv(m, 1) || ctrl_status.natc_reset!=gtmv(m, 1) || ctrl_status.ddr_hash_mode!=gtmv(m, 3) || ctrl_status.ddr_32bit_in_64bit_swap_control!=gtmv(m, 1) || ctrl_status.ddr_8bit_in_32bit_swap_control!=gtmv(m, 1) || ctrl_status.cache_lookup_blocking_mode!=gtmv(m, 1) || ctrl_status.age_timer_tick!=gtmv(m, 1) || ctrl_status.age_timer!=gtmv(m, 5) || ctrl_status.cache_algo!=gtmv(m, 4) || ctrl_status.unused2!=gtmv(m, 8) || ctrl_status.unused1!=gtmv(m, 2) || ctrl_status.cache_update_on_reg_ddr_lookup!=gtmv(m, 1) || ctrl_status.ddr_counter_8bit_in_32bit_swap_control!=gtmv(m, 1) || ctrl_status.ddr_hash_swap!=gtmv(m, 1) || ctrl_status.ddr_replace_duplicated_cached_entry_enable!=gtmv(m, 1) || ctrl_status.ddr_lookup_pending_fifo_mode_disable!=gtmv(m, 1) || ctrl_status.unused0!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        natc_ctrs_control ctrs_control = {.ddr_evict_count_en=gtmv(m, 8), .ddr_request_count_en=gtmv(m, 8), .cache_miss_count_en=gtmv(m, 8), .cache_hit_count_en=gtmv(m, 8), .counter_wraparound_dis=gtmv(m, 1), .ddr_block_count_en=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_ctrs_control_set( %u %u %u %u %u %u)\n", ctrs_control.ddr_evict_count_en, ctrs_control.ddr_request_count_en, ctrs_control.cache_miss_count_en, ctrs_control.cache_hit_count_en, ctrs_control.counter_wraparound_dis, ctrs_control.ddr_block_count_en);
        if(!err) ag_drv_natc_ctrs_control_set(&ctrs_control);
        if(!err) ag_drv_natc_ctrs_control_get( &ctrs_control);
        if(!err) bdmf_session_print(session, "ag_drv_natc_ctrs_control_get( %u %u %u %u %u %u)\n", ctrs_control.ddr_evict_count_en, ctrs_control.ddr_request_count_en, ctrs_control.cache_miss_count_en, ctrs_control.cache_hit_count_en, ctrs_control.counter_wraparound_dis, ctrs_control.ddr_block_count_en);
        if(err || ctrs_control.ddr_evict_count_en!=gtmv(m, 8) || ctrs_control.ddr_request_count_en!=gtmv(m, 8) || ctrs_control.cache_miss_count_en!=gtmv(m, 8) || ctrs_control.cache_hit_count_en!=gtmv(m, 8) || ctrs_control.counter_wraparound_dis!=gtmv(m, 1) || ctrs_control.ddr_block_count_en!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tbl_idx=gtmv(m, 4);
        uint8_t data=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_natc_table_control_set( %u %u)\n", tbl_idx, data);
        if(!err) ag_drv_natc_table_control_set(tbl_idx, data);
        if(!err) ag_drv_natc_table_control_get( tbl_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_natc_table_control_get( %u %u)\n", tbl_idx, data);
        if(err || data!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_natc_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_control_status : reg = &RU_REG(NATC, CONTROL_STATUS); blk = &RU_BLK(NATC); break;
    case bdmf_address_control_status2 : reg = &RU_REG(NATC, CONTROL_STATUS2); blk = &RU_BLK(NATC); break;
    case bdmf_address_table_control : reg = &RU_REG(NATC, TABLE_CONTROL); blk = &RU_BLK(NATC); break;
    case bdmf_address_stat_counter_control_0 : reg = &RU_REG(NATC, STAT_COUNTER_CONTROL_0); blk = &RU_BLK(NATC); break;
    case bdmf_address_stat_counter_control_1 : reg = &RU_REG(NATC, STAT_COUNTER_CONTROL_1); blk = &RU_BLK(NATC); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%lX\n", j, (blk->addr[i] + reg->addr + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%lX\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_natc_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "natc"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "natc", "natc", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_ctrl_status[]={
            BDMFMON_MAKE_PARM("ddr_enable", "ddr_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_add_command_speedup_mode", "natc_add_command_speedup_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unused4", "unused4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_64bit_in_128bit_swap_control", "ddr_64bit_in_128bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("smem_32bit_in_64bit_swap_control", "smem_32bit_in_64bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("smem_8bit_in_32bit_swap_control", "smem_8bit_in_32bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_swap_all_control", "ddr_swap_all_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("repeated_key_det_en", "repeated_key_det_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reg_32bit_in_64bit_swap_control", "reg_32bit_in_64bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reg_8bit_in_32bit_swap_control", "reg_8bit_in_32bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pending_hash_mode", "ddr_pending_hash_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pending_fifo_entry_check_enable", "pending_fifo_entry_check_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_ddr_miss", "cache_update_on_ddr_miss", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_disable_on_reg_lookup", "ddr_disable_on_reg_lookup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nat_hash_mode", "nat_hash_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_hash_limit", "multi_hash_limit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("decr_count_wraparound_enable", "decr_count_wraparound_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nat_arb_st", "nat_arb_st", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_increment_on_reg_lookup", "natc_smem_increment_on_reg_lookup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_clear_by_update_disable", "natc_smem_clear_by_update_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_reset", "regfile_fifo_reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_enable", "natc_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_reset", "natc_reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode", "ddr_hash_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_32bit_in_64bit_swap_control", "ddr_32bit_in_64bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_8bit_in_32bit_swap_control", "ddr_8bit_in_32bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_lookup_blocking_mode", "cache_lookup_blocking_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer_tick", "age_timer_tick", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer", "age_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_algo", "cache_algo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unused2", "unused2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unused1", "unused1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_reg_ddr_lookup", "cache_update_on_reg_ddr_lookup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_counter_8bit_in_32bit_swap_control", "ddr_counter_8bit_in_32bit_swap_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_swap", "ddr_hash_swap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_replace_duplicated_cached_entry_enable", "ddr_replace_duplicated_cached_entry_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_lookup_pending_fifo_mode_disable", "ddr_lookup_pending_fifo_mode_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unused0", "unused0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ctrs_control[]={
            BDMFMON_MAKE_PARM("ddr_evict_count_en", "ddr_evict_count_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_request_count_en", "ddr_request_count_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_miss_count_en", "cache_miss_count_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_hit_count_en", "cache_hit_count_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("counter_wraparound_dis", "counter_wraparound_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_block_count_en", "ddr_block_count_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_table_control[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ctrl_status", .val=cli_natc_ctrl_status, .parms=set_ctrl_status },
            { .name="ctrs_control", .val=cli_natc_ctrs_control, .parms=set_ctrs_control },
            { .name="table_control", .val=cli_natc_table_control, .parms=set_table_control },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_natc_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ctrl_status", .val=cli_natc_ctrl_status, .parms=set_default },
            { .name="ctrs_control", .val=cli_natc_ctrs_control, .parms=set_default },
            { .name="table_control", .val=cli_natc_table_control, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_natc_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CONTROL_STATUS" , .val=bdmf_address_control_status },
            { .name="CONTROL_STATUS2" , .val=bdmf_address_control_status2 },
            { .name="TABLE_CONTROL" , .val=bdmf_address_table_control },
            { .name="STAT_COUNTER_CONTROL_0" , .val=bdmf_address_stat_counter_control_0 },
            { .name="STAT_COUNTER_CONTROL_1" , .val=bdmf_address_stat_counter_control_1 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_natc_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

