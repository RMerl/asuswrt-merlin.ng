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
#include "xrdp_drv_nat_cache_ag.h"

#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_nat_cache_ctrl_status_set(const nat_cache_ctrl_status *ctrl_status)
{
    uint32_t reg_natc_control_status = 0;
    uint32_t reg_natc_control_status2 = 0;

#ifdef VALIDATE_PARMS
    if(!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctrl_status->natc_reset >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->regfile_fifo_reset >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_smem_clear_by_update_disable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_smem_increment_on_reg_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->nat_arb_st >= _2BITS_MAX_VAL_) ||
       (ctrl_status->decr_count_wraparound_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->multi_hash_limit >= _4BITS_MAX_VAL_) ||
       (ctrl_status->nat_hash_mode >= _3BITS_MAX_VAL_) ||
       (ctrl_status->ddr_disable_on_reg_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->cache_update_on_ddr_miss >= _1BITS_MAX_VAL_) ||
       (ctrl_status->pending_fifo_entry_check_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused1 >= _3BITS_MAX_VAL_) ||
       (ctrl_status->reg_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->reg_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->repeated_key_det_en >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_swap_all_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->smem_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->smem_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_64bit_in_128bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_init_done >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused0 >= _1BITS_MAX_VAL_) ||
       (ctrl_status->natc_add_command_speedup_mode >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused2 >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_lookup_pending_fifo_mode_disable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_replace_duplicated_cached_entry_enable >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_hash_swap >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_counter_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->cache_update_on_reg_ddr_lookup >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused3 >= _2BITS_MAX_VAL_) ||
       (ctrl_status->cache_algo >= _4BITS_MAX_VAL_) ||
       (ctrl_status->age_timer >= _5BITS_MAX_VAL_) ||
       (ctrl_status->age_timer_tick >= _1BITS_MAX_VAL_) ||
       (ctrl_status->cache_lookup_blocking_mode >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_8bit_in_32bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->ddr_32bit_in_64bit_swap_control >= _1BITS_MAX_VAL_) ||
       (ctrl_status->unused5 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_RESET, reg_natc_control_status, ctrl_status->natc_reset);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_ENABLE, reg_natc_control_status, ctrl_status->natc_enable);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, REGFILE_FIFO_RESET, reg_natc_control_status, ctrl_status->regfile_fifo_reset);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_SMEM_CLEAR_BY_UPDATE_DISABLE, reg_natc_control_status, ctrl_status->natc_smem_clear_by_update_disable);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_SMEM_INCREMENT_ON_REG_LOOKUP, reg_natc_control_status, ctrl_status->natc_smem_increment_on_reg_lookup);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NAT_ARB_ST, reg_natc_control_status, ctrl_status->nat_arb_st);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, DECR_COUNT_WRAPAROUND_ENABLE, reg_natc_control_status, ctrl_status->decr_count_wraparound_enable);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, MULTI_HASH_LIMIT, reg_natc_control_status, ctrl_status->multi_hash_limit);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NAT_HASH_MODE, reg_natc_control_status, ctrl_status->nat_hash_mode);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_DISABLE_ON_REG_LOOKUP, reg_natc_control_status, ctrl_status->ddr_disable_on_reg_lookup);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, CACHE_UPDATE_ON_DDR_MISS, reg_natc_control_status, ctrl_status->cache_update_on_ddr_miss);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, PENDING_FIFO_ENTRY_CHECK_ENABLE, reg_natc_control_status, ctrl_status->pending_fifo_entry_check_enable);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, UNUSED1, reg_natc_control_status, ctrl_status->unused1);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, REG_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status, ctrl_status->reg_8bit_in_32bit_swap_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, REG_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status, ctrl_status->reg_32bit_in_64bit_swap_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, REPEATED_KEY_DET_EN, reg_natc_control_status, ctrl_status->repeated_key_det_en);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_SWAP_ALL_CONTROL, reg_natc_control_status, ctrl_status->ddr_swap_all_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, SMEM_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status, ctrl_status->smem_8bit_in_32bit_swap_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, SMEM_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status, ctrl_status->smem_32bit_in_64bit_swap_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_64BIT_IN_128BIT_SWAP_CONTROL, reg_natc_control_status, ctrl_status->ddr_64bit_in_128bit_swap_control);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_INIT_DONE, reg_natc_control_status, ctrl_status->natc_init_done);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, UNUSED0, reg_natc_control_status, ctrl_status->unused0);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_natc_control_status, ctrl_status->natc_add_command_speedup_mode);
    reg_natc_control_status = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_ENABLE, reg_natc_control_status, ctrl_status->ddr_enable);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED2, reg_natc_control_status2, ctrl_status->unused2);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE, reg_natc_control_status2, ctrl_status->ddr_lookup_pending_fifo_mode_disable);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE, reg_natc_control_status2, ctrl_status->ddr_replace_duplicated_cached_entry_enable);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_HASH_SWAP, reg_natc_control_status2, ctrl_status->ddr_hash_swap);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status2, ctrl_status->ddr_counter_8bit_in_32bit_swap_control);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_UPDATE_ON_REG_DDR_LOOKUP, reg_natc_control_status2, ctrl_status->cache_update_on_reg_ddr_lookup);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED3, reg_natc_control_status2, ctrl_status->unused3);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED4, reg_natc_control_status2, ctrl_status->unused4);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_ALGO, reg_natc_control_status2, ctrl_status->cache_algo);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, AGE_TIMER, reg_natc_control_status2, ctrl_status->age_timer);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, AGE_TIMER_TICK, reg_natc_control_status2, ctrl_status->age_timer_tick);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_LOOKUP_BLOCKING_MODE, reg_natc_control_status2, ctrl_status->cache_lookup_blocking_mode);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status2, ctrl_status->ddr_8bit_in_32bit_swap_control);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status2, ctrl_status->ddr_32bit_in_64bit_swap_control);
    reg_natc_control_status2 = RU_FIELD_SET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED5, reg_natc_control_status2, ctrl_status->unused5);

    RU_REG_WRITE(0, NAT_CACHE, NATC_CONTROL_STATUS, reg_natc_control_status);
    RU_REG_WRITE(0, NAT_CACHE, NATC_CONTROL_STATUS2, reg_natc_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_ctrl_status_get(nat_cache_ctrl_status *ctrl_status)
{
    uint32_t reg_natc_control_status;
    uint32_t reg_natc_control_status2;

#ifdef VALIDATE_PARMS
    if (!ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_CONTROL_STATUS, reg_natc_control_status);
    RU_REG_READ(0, NAT_CACHE, NATC_CONTROL_STATUS2, reg_natc_control_status2);

    ctrl_status->natc_reset = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_RESET, reg_natc_control_status);
    ctrl_status->natc_enable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_ENABLE, reg_natc_control_status);
    ctrl_status->regfile_fifo_reset = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, REGFILE_FIFO_RESET, reg_natc_control_status);
    ctrl_status->natc_smem_clear_by_update_disable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_SMEM_CLEAR_BY_UPDATE_DISABLE, reg_natc_control_status);
    ctrl_status->natc_smem_increment_on_reg_lookup = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_SMEM_INCREMENT_ON_REG_LOOKUP, reg_natc_control_status);
    ctrl_status->nat_arb_st = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NAT_ARB_ST, reg_natc_control_status);
    ctrl_status->decr_count_wraparound_enable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, DECR_COUNT_WRAPAROUND_ENABLE, reg_natc_control_status);
    ctrl_status->multi_hash_limit = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, MULTI_HASH_LIMIT, reg_natc_control_status);
    ctrl_status->nat_hash_mode = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NAT_HASH_MODE, reg_natc_control_status);
    ctrl_status->ddr_disable_on_reg_lookup = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_DISABLE_ON_REG_LOOKUP, reg_natc_control_status);
    ctrl_status->cache_update_on_ddr_miss = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, CACHE_UPDATE_ON_DDR_MISS, reg_natc_control_status);
    ctrl_status->pending_fifo_entry_check_enable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, PENDING_FIFO_ENTRY_CHECK_ENABLE, reg_natc_control_status);
    ctrl_status->unused1 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, UNUSED1, reg_natc_control_status);
    ctrl_status->reg_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, REG_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status);
    ctrl_status->reg_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, REG_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status);
    ctrl_status->repeated_key_det_en = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, REPEATED_KEY_DET_EN, reg_natc_control_status);
    ctrl_status->ddr_swap_all_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_SWAP_ALL_CONTROL, reg_natc_control_status);
    ctrl_status->smem_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, SMEM_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status);
    ctrl_status->smem_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, SMEM_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status);
    ctrl_status->ddr_64bit_in_128bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_64BIT_IN_128BIT_SWAP_CONTROL, reg_natc_control_status);
    ctrl_status->natc_init_done = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_INIT_DONE, reg_natc_control_status);
    ctrl_status->unused0 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, UNUSED0, reg_natc_control_status);
    ctrl_status->natc_add_command_speedup_mode = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, NATC_ADD_COMMAND_SPEEDUP_MODE, reg_natc_control_status);
    ctrl_status->ddr_enable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS, DDR_ENABLE, reg_natc_control_status);
    ctrl_status->unused2 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED2, reg_natc_control_status2);
    ctrl_status->ddr_lookup_pending_fifo_mode_disable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_LOOKUP_PENDING_FIFO_MODE_DISABLE, reg_natc_control_status2);
    ctrl_status->ddr_replace_duplicated_cached_entry_enable = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_REPLACE_DUPLICATED_CACHED_ENTRY_ENABLE, reg_natc_control_status2);
    ctrl_status->ddr_hash_swap = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_HASH_SWAP, reg_natc_control_status2);
    ctrl_status->ddr_counter_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_COUNTER_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status2);
    ctrl_status->cache_update_on_reg_ddr_lookup = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_UPDATE_ON_REG_DDR_LOOKUP, reg_natc_control_status2);
    ctrl_status->unused3 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED3, reg_natc_control_status2);
    ctrl_status->unused4 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED4, reg_natc_control_status2);
    ctrl_status->cache_algo = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_ALGO, reg_natc_control_status2);
    ctrl_status->age_timer = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, AGE_TIMER, reg_natc_control_status2);
    ctrl_status->age_timer_tick = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, AGE_TIMER_TICK, reg_natc_control_status2);
    ctrl_status->cache_lookup_blocking_mode = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, CACHE_LOOKUP_BLOCKING_MODE, reg_natc_control_status2);
    ctrl_status->ddr_8bit_in_32bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_8BIT_IN_32BIT_SWAP_CONTROL, reg_natc_control_status2);
    ctrl_status->ddr_32bit_in_64bit_swap_control = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, DDR_32BIT_IN_64BIT_SWAP_CONTROL, reg_natc_control_status2);
    ctrl_status->unused5 = RU_FIELD_GET(0, NAT_CACHE, NATC_CONTROL_STATUS2, UNUSED5, reg_natc_control_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ctrs_set(const nat_cache_natc_ctrs *natc_ctrs)
{
    uint32_t reg_natc_cache_hit_count = 0;
    uint32_t reg_natc_cache_miss_count = 0;
    uint32_t reg_natc_ddr_request_count = 0;
    uint32_t reg_natc_ddr_evict_count = 0;
    uint32_t reg_natc_ddr_block_count = 0;

#ifdef VALIDATE_PARMS
    if(!natc_ctrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    reg_natc_cache_hit_count = RU_FIELD_SET(0, NAT_CACHE, NATC_CACHE_HIT_COUNT, CACHE_HIT_COUNT, reg_natc_cache_hit_count, natc_ctrs->cache_hit_count);
    reg_natc_cache_miss_count = RU_FIELD_SET(0, NAT_CACHE, NATC_CACHE_MISS_COUNT, CACHE_MISS_COUNT, reg_natc_cache_miss_count, natc_ctrs->cache_miss_count);
    reg_natc_ddr_request_count = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_REQUEST_COUNT, DDR_REQUEST_COUNT, reg_natc_ddr_request_count, natc_ctrs->ddr_request_count);
    reg_natc_ddr_evict_count = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_EVICT_COUNT, DDR_EVICT_COUNT, reg_natc_ddr_evict_count, natc_ctrs->ddr_evict_count);
    reg_natc_ddr_block_count = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BLOCK_COUNT, DDR_BLOCK_COUNT, reg_natc_ddr_block_count, natc_ctrs->ddr_block_count);

    RU_REG_WRITE(0, NAT_CACHE, NATC_CACHE_HIT_COUNT, reg_natc_cache_hit_count);
    RU_REG_WRITE(0, NAT_CACHE, NATC_CACHE_MISS_COUNT, reg_natc_cache_miss_count);
    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_REQUEST_COUNT, reg_natc_ddr_request_count);
    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_EVICT_COUNT, reg_natc_ddr_evict_count);
    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_BLOCK_COUNT, reg_natc_ddr_block_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ctrs_get(nat_cache_natc_ctrs *natc_ctrs)
{
    uint32_t reg_natc_cache_hit_count;
    uint32_t reg_natc_cache_miss_count;
    uint32_t reg_natc_ddr_request_count;
    uint32_t reg_natc_ddr_evict_count;
    uint32_t reg_natc_ddr_block_count;

#ifdef VALIDATE_PARMS
    if (!natc_ctrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_CACHE_HIT_COUNT, reg_natc_cache_hit_count);
    RU_REG_READ(0, NAT_CACHE, NATC_CACHE_MISS_COUNT, reg_natc_cache_miss_count);
    RU_REG_READ(0, NAT_CACHE, NATC_DDR_REQUEST_COUNT, reg_natc_ddr_request_count);
    RU_REG_READ(0, NAT_CACHE, NATC_DDR_EVICT_COUNT, reg_natc_ddr_evict_count);
    RU_REG_READ(0, NAT_CACHE, NATC_DDR_BLOCK_COUNT, reg_natc_ddr_block_count);

    natc_ctrs->cache_hit_count = RU_FIELD_GET(0, NAT_CACHE, NATC_CACHE_HIT_COUNT, CACHE_HIT_COUNT, reg_natc_cache_hit_count);
    natc_ctrs->cache_miss_count = RU_FIELD_GET(0, NAT_CACHE, NATC_CACHE_MISS_COUNT, CACHE_MISS_COUNT, reg_natc_cache_miss_count);
    natc_ctrs->ddr_request_count = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_REQUEST_COUNT, DDR_REQUEST_COUNT, reg_natc_ddr_request_count);
    natc_ctrs->ddr_evict_count = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_EVICT_COUNT, DDR_EVICT_COUNT, reg_natc_ddr_evict_count);
    natc_ctrs->ddr_block_count = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BLOCK_COUNT, DDR_BLOCK_COUNT, reg_natc_ddr_block_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_table_control_set(const nat_cache_natc_table_control *natc_table_control)
{
    uint32_t reg_natc_table_control = 0;

#ifdef VALIDATE_PARMS
    if(!natc_table_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_table_control->non_cacheable_tbl0 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl0 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl1 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl1 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl2 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl2 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl3 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl3 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl4 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl4 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl5 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl5 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl6 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl6 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->non_cacheable_tbl7 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->key_len_tbl7 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl0 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl1 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl2 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl3 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl4 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl5 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl6 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->var_context_len_en_tbl7 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl0 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl1 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl2 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl3 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl4 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl5 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl6 >= _1BITS_MAX_VAL_) ||
       (natc_table_control->smem_dis_tbl7 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL0, reg_natc_table_control, natc_table_control->non_cacheable_tbl0);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL0, reg_natc_table_control, natc_table_control->key_len_tbl0);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL1, reg_natc_table_control, natc_table_control->non_cacheable_tbl1);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL1, reg_natc_table_control, natc_table_control->key_len_tbl1);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL2, reg_natc_table_control, natc_table_control->non_cacheable_tbl2);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL2, reg_natc_table_control, natc_table_control->key_len_tbl2);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL3, reg_natc_table_control, natc_table_control->non_cacheable_tbl3);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL3, reg_natc_table_control, natc_table_control->key_len_tbl3);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL4, reg_natc_table_control, natc_table_control->non_cacheable_tbl4);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL4, reg_natc_table_control, natc_table_control->key_len_tbl4);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL5, reg_natc_table_control, natc_table_control->non_cacheable_tbl5);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL5, reg_natc_table_control, natc_table_control->key_len_tbl5);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL6, reg_natc_table_control, natc_table_control->non_cacheable_tbl6);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL6, reg_natc_table_control, natc_table_control->key_len_tbl6);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL7, reg_natc_table_control, natc_table_control->non_cacheable_tbl7);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL7, reg_natc_table_control, natc_table_control->key_len_tbl7);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL0, reg_natc_table_control, natc_table_control->var_context_len_en_tbl0);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL1, reg_natc_table_control, natc_table_control->var_context_len_en_tbl1);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL2, reg_natc_table_control, natc_table_control->var_context_len_en_tbl2);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL3, reg_natc_table_control, natc_table_control->var_context_len_en_tbl3);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL4, reg_natc_table_control, natc_table_control->var_context_len_en_tbl4);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL5, reg_natc_table_control, natc_table_control->var_context_len_en_tbl5);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL6, reg_natc_table_control, natc_table_control->var_context_len_en_tbl6);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL7, reg_natc_table_control, natc_table_control->var_context_len_en_tbl7);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL0, reg_natc_table_control, natc_table_control->smem_dis_tbl0);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL1, reg_natc_table_control, natc_table_control->smem_dis_tbl1);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL2, reg_natc_table_control, natc_table_control->smem_dis_tbl2);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL3, reg_natc_table_control, natc_table_control->smem_dis_tbl3);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL4, reg_natc_table_control, natc_table_control->smem_dis_tbl4);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL5, reg_natc_table_control, natc_table_control->smem_dis_tbl5);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL6, reg_natc_table_control, natc_table_control->smem_dis_tbl6);
    reg_natc_table_control = RU_FIELD_SET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL7, reg_natc_table_control, natc_table_control->smem_dis_tbl7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TABLE_CONTROL, reg_natc_table_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_table_control_get(nat_cache_natc_table_control *natc_table_control)
{
    uint32_t reg_natc_table_control;

#ifdef VALIDATE_PARMS
    if (!natc_table_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TABLE_CONTROL, reg_natc_table_control);

    natc_table_control->non_cacheable_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL0, reg_natc_table_control);
    natc_table_control->key_len_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL0, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL1, reg_natc_table_control);
    natc_table_control->key_len_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL1, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL2, reg_natc_table_control);
    natc_table_control->key_len_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL2, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL3, reg_natc_table_control);
    natc_table_control->key_len_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL3, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL4, reg_natc_table_control);
    natc_table_control->key_len_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL4, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL5, reg_natc_table_control);
    natc_table_control->key_len_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL5, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL6, reg_natc_table_control);
    natc_table_control->key_len_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL6, reg_natc_table_control);
    natc_table_control->non_cacheable_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, NON_CACHEABLE_TBL7, reg_natc_table_control);
    natc_table_control->key_len_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, KEY_LEN_TBL7, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL0, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL1, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL2, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL3, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL4, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL5, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL6, reg_natc_table_control);
    natc_table_control->var_context_len_en_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, VAR_CONTEXT_LEN_EN_TBL7, reg_natc_table_control);
    natc_table_control->smem_dis_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL0, reg_natc_table_control);
    natc_table_control->smem_dis_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL1, reg_natc_table_control);
    natc_table_control->smem_dis_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL2, reg_natc_table_control);
    natc_table_control->smem_dis_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL3, reg_natc_table_control);
    natc_table_control->smem_dis_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL4, reg_natc_table_control);
    natc_table_control->smem_dis_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL5, reg_natc_table_control);
    natc_table_control->smem_dis_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL6, reg_natc_table_control);
    natc_table_control->smem_dis_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_TABLE_CONTROL, SMEM_DIS_TBL7, reg_natc_table_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_command_status_set(const nat_cache_nat0_command_status *nat0_command_status)
{
    uint32_t reg_nat0_command_status = 0;

#ifdef VALIDATE_PARMS
    if(!nat0_command_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((nat0_command_status->command >= _3BITS_MAX_VAL_) ||
       (nat0_command_status->unused0 >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->busy >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->error >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->miss >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->cache_hit >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->multihash_count >= _4BITS_MAX_VAL_) ||
       (nat0_command_status->nat_tbl >= _3BITS_MAX_VAL_) ||
       (nat0_command_status->decr_count >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->cache_flush >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->del_cmd_mode >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->add_cmd_mode >= _1BITS_MAX_VAL_) ||
       (nat0_command_status->add_cmd_ddr_bin >= _3BITS_MAX_VAL_) ||
       (nat0_command_status->add_cmd_ddr_miss >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, COMMAND, reg_nat0_command_status, nat0_command_status->command);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, UNUSED0, reg_nat0_command_status, nat0_command_status->unused0);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, BUSY, reg_nat0_command_status, nat0_command_status->busy);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ERROR, reg_nat0_command_status, nat0_command_status->error);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, MISS, reg_nat0_command_status, nat0_command_status->miss);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, CACHE_HIT, reg_nat0_command_status, nat0_command_status->cache_hit);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, MULTIHASH_COUNT, reg_nat0_command_status, nat0_command_status->multihash_count);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, NAT_TBL, reg_nat0_command_status, nat0_command_status->nat_tbl);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DECR_COUNT, reg_nat0_command_status, nat0_command_status->decr_count);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, CACHE_FLUSH, reg_nat0_command_status, nat0_command_status->cache_flush);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DEL_CMD_MODE, reg_nat0_command_status, nat0_command_status->del_cmd_mode);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_MODE, reg_nat0_command_status, nat0_command_status->add_cmd_mode);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DEL_CMD_DDR_BIN, reg_nat0_command_status, nat0_command_status->del_cmd_ddr_bin);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_DDR_BIN, reg_nat0_command_status, nat0_command_status->add_cmd_ddr_bin);
    reg_nat0_command_status = RU_FIELD_SET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_DDR_MISS, reg_nat0_command_status, nat0_command_status->add_cmd_ddr_miss);

    RU_REG_WRITE(0, NAT_CACHE, NAT0_COMMAND_STATUS, reg_nat0_command_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_command_status_get(nat_cache_nat0_command_status *nat0_command_status)
{
    uint32_t reg_nat0_command_status;

#ifdef VALIDATE_PARMS
    if (!nat0_command_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NAT0_COMMAND_STATUS, reg_nat0_command_status);

    nat0_command_status->command = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, COMMAND, reg_nat0_command_status);
    nat0_command_status->unused0 = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, UNUSED0, reg_nat0_command_status);
    nat0_command_status->busy = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, BUSY, reg_nat0_command_status);
    nat0_command_status->error = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ERROR, reg_nat0_command_status);
    nat0_command_status->miss = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, MISS, reg_nat0_command_status);
    nat0_command_status->cache_hit = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, CACHE_HIT, reg_nat0_command_status);
    nat0_command_status->multihash_count = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, MULTIHASH_COUNT, reg_nat0_command_status);
    nat0_command_status->nat_tbl = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, NAT_TBL, reg_nat0_command_status);
    nat0_command_status->decr_count = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DECR_COUNT, reg_nat0_command_status);
    nat0_command_status->cache_flush = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, CACHE_FLUSH, reg_nat0_command_status);
    nat0_command_status->del_cmd_mode = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DEL_CMD_MODE, reg_nat0_command_status);
    nat0_command_status->add_cmd_mode = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_MODE, reg_nat0_command_status);
    nat0_command_status->del_cmd_ddr_bin = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, DEL_CMD_DDR_BIN, reg_nat0_command_status);
    nat0_command_status->add_cmd_ddr_bin = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_DDR_BIN, reg_nat0_command_status);
    nat0_command_status->add_cmd_ddr_miss = RU_FIELD_GET(0, NAT_CACHE, NAT0_COMMAND_STATUS, ADD_CMD_DDR_MISS, reg_nat0_command_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_hash_get(uint32_t *hash)
{
    uint32_t reg_nat0_hash;

#ifdef VALIDATE_PARMS
    if (!hash)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NAT0_HASH, reg_nat0_hash);

    *hash = RU_FIELD_GET(0, NAT_CACHE, NAT0_HASH, HASH, reg_nat0_hash);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_hit_count_get(uint32_t *hit_count)
{
    uint32_t reg_nat0_hit_count;

#ifdef VALIDATE_PARMS
    if (!hit_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NAT0_HIT_COUNT, reg_nat0_hit_count);

    *hit_count = RU_FIELD_GET(0, NAT_CACHE, NAT0_HIT_COUNT, HIT_COUNT, reg_nat0_hit_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_byte_count_get(uint32_t *byte_count)
{
    uint32_t reg_nat0_byte_count;

#ifdef VALIDATE_PARMS
    if (!byte_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NAT0_BYTE_COUNT, reg_nat0_byte_count);

    *byte_count = RU_FIELD_GET(0, NAT_CACHE, NAT0_BYTE_COUNT, BYTE_COUNT, reg_nat0_byte_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_pkt_len_set(uint16_t pkt_len, uint16_t unused)
{
    uint32_t reg_nat0_pkt_len = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_nat0_pkt_len = RU_FIELD_SET(0, NAT_CACHE, NAT0_PKT_LEN, PKT_LEN, reg_nat0_pkt_len, pkt_len);
    reg_nat0_pkt_len = RU_FIELD_SET(0, NAT_CACHE, NAT0_PKT_LEN, UNUSED, reg_nat0_pkt_len, unused);

    RU_REG_WRITE(0, NAT_CACHE, NAT0_PKT_LEN, reg_nat0_pkt_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_pkt_len_get(uint16_t *pkt_len, uint16_t *unused)
{
    uint32_t reg_nat0_pkt_len;

#ifdef VALIDATE_PARMS
    if (!pkt_len || !unused)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NAT0_PKT_LEN, reg_nat0_pkt_len);

    *pkt_len = RU_FIELD_GET(0, NAT_CACHE, NAT0_PKT_LEN, PKT_LEN, reg_nat0_pkt_len);
    *unused = RU_FIELD_GET(0, NAT_CACHE, NAT0_PKT_LEN, UNUSED, reg_nat0_pkt_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_key_result_set(uint8_t zero, const nat_cache_nat0_key_result *nat0_key_result)
{
#ifdef VALIDATE_PARMS
    if(!nat0_key_result)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, zero * 18 + 0, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[0]);
    RU_REG_RAM_WRITE(0, zero * 18 + 1, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[1]);
    RU_REG_RAM_WRITE(0, zero * 18 + 2, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[2]);
    RU_REG_RAM_WRITE(0, zero * 18 + 3, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[3]);
    RU_REG_RAM_WRITE(0, zero * 18 + 4, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[4]);
    RU_REG_RAM_WRITE(0, zero * 18 + 5, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[5]);
    RU_REG_RAM_WRITE(0, zero * 18 + 6, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[6]);
    RU_REG_RAM_WRITE(0, zero * 18 + 7, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[7]);
    RU_REG_RAM_WRITE(0, zero * 18 + 8, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[8]);
    RU_REG_RAM_WRITE(0, zero * 18 + 9, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[9]);
    RU_REG_RAM_WRITE(0, zero * 18 + 10, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[10]);
    RU_REG_RAM_WRITE(0, zero * 18 + 11, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[11]);
    RU_REG_RAM_WRITE(0, zero * 18 + 12, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[12]);
    RU_REG_RAM_WRITE(0, zero * 18 + 13, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[13]);
    RU_REG_RAM_WRITE(0, zero * 18 + 14, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[14]);
    RU_REG_RAM_WRITE(0, zero * 18 + 15, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[15]);
    RU_REG_RAM_WRITE(0, zero * 18 + 16, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[16]);
    RU_REG_RAM_WRITE(0, zero * 18 + 17, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[17]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_nat0_key_result_get(uint8_t zero, nat_cache_nat0_key_result *nat0_key_result)
{
#ifdef VALIDATE_PARMS
    if (!nat0_key_result)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 18 + 0, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[0]);
    RU_REG_RAM_READ(0, zero * 18 + 1, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[1]);
    RU_REG_RAM_READ(0, zero * 18 + 2, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[2]);
    RU_REG_RAM_READ(0, zero * 18 + 3, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[3]);
    RU_REG_RAM_READ(0, zero * 18 + 4, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[4]);
    RU_REG_RAM_READ(0, zero * 18 + 5, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[5]);
    RU_REG_RAM_READ(0, zero * 18 + 6, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[6]);
    RU_REG_RAM_READ(0, zero * 18 + 7, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[7]);
    RU_REG_RAM_READ(0, zero * 18 + 8, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[8]);
    RU_REG_RAM_READ(0, zero * 18 + 9, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[9]);
    RU_REG_RAM_READ(0, zero * 18 + 10, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[10]);
    RU_REG_RAM_READ(0, zero * 18 + 11, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[11]);
    RU_REG_RAM_READ(0, zero * 18 + 12, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[12]);
    RU_REG_RAM_READ(0, zero * 18 + 13, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[13]);
    RU_REG_RAM_READ(0, zero * 18 + 14, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[14]);
    RU_REG_RAM_READ(0, zero * 18 + 15, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[15]);
    RU_REG_RAM_READ(0, zero * 18 + 16, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[16]);
    RU_REG_RAM_READ(0, zero * 18 + 17, NAT_CACHE, NAT0_KEY_RESULT, nat0_key_result->data[17]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl0_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl0_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl0_ddr_key_base_address_lower, zeros);
    reg_natc_tbl0_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl0_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl0_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl0_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl0_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl0_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl0_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl0_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl0_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl0_ddr_key_base_address_upper, bar);
    reg_natc_tbl0_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl0_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl0_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl0_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl0_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl0_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl0_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl0_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl0_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl0_ddr_result_base_address_lower, zeros);
    reg_natc_tbl0_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl0_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl0_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl0_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl0_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl0_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl0_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl0_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl0_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl0_ddr_result_base_address_upper, bar);
    reg_natc_tbl0_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl0_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl0_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl0_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl0_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl0_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl0_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl1_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl1_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl1_ddr_key_base_address_lower, zeros);
    reg_natc_tbl1_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl1_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl1_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl1_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl1_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl1_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl1_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl1_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl1_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl1_ddr_key_base_address_upper, bar);
    reg_natc_tbl1_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl1_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl1_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl1_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl1_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl1_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl1_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl1_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl1_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl1_ddr_result_base_address_lower, zeros);
    reg_natc_tbl1_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl1_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl1_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl1_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl1_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl1_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl1_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl1_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl1_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl1_ddr_result_base_address_upper, bar);
    reg_natc_tbl1_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl1_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl1_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl1_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl1_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl1_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl1_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl2_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl2_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl2_ddr_key_base_address_lower, zeros);
    reg_natc_tbl2_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl2_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl2_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl2_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl2_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl2_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl2_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl2_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl2_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl2_ddr_key_base_address_upper, bar);
    reg_natc_tbl2_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl2_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl2_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl2_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl2_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl2_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl2_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl2_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl2_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl2_ddr_result_base_address_lower, zeros);
    reg_natc_tbl2_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl2_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl2_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl2_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl2_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl2_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl2_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl2_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl2_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl2_ddr_result_base_address_upper, bar);
    reg_natc_tbl2_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl2_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl2_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl2_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl2_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl2_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl2_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl3_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl3_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl3_ddr_key_base_address_lower, zeros);
    reg_natc_tbl3_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl3_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl3_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl3_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl3_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl3_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl3_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl3_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl3_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl3_ddr_key_base_address_upper, bar);
    reg_natc_tbl3_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl3_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl3_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl3_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl3_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl3_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl3_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl3_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl3_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl3_ddr_result_base_address_lower, zeros);
    reg_natc_tbl3_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl3_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl3_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl3_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl3_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl3_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl3_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl3_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl3_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl3_ddr_result_base_address_upper, bar);
    reg_natc_tbl3_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl3_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl3_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl3_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl3_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl3_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl3_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl4_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl4_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl4_ddr_key_base_address_lower, zeros);
    reg_natc_tbl4_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl4_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl4_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl4_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl4_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl4_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl4_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl4_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl4_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl4_ddr_key_base_address_upper, bar);
    reg_natc_tbl4_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl4_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl4_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl4_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl4_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl4_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl4_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl4_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl4_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl4_ddr_result_base_address_lower, zeros);
    reg_natc_tbl4_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl4_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl4_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl4_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl4_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl4_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl4_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl4_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl4_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl4_ddr_result_base_address_upper, bar);
    reg_natc_tbl4_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl4_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl4_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl4_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl4_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl4_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl4_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl5_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl5_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl5_ddr_key_base_address_lower, zeros);
    reg_natc_tbl5_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl5_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl5_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl5_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl5_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl5_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl5_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl5_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl5_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl5_ddr_key_base_address_upper, bar);
    reg_natc_tbl5_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl5_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl5_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl5_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl5_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl5_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl5_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl5_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl5_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl5_ddr_result_base_address_lower, zeros);
    reg_natc_tbl5_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl5_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl5_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl5_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl5_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl5_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl5_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl5_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl5_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl5_ddr_result_base_address_upper, bar);
    reg_natc_tbl5_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl5_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl5_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl5_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl5_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl5_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl5_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl6_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl6_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl6_ddr_key_base_address_lower, zeros);
    reg_natc_tbl6_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl6_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl6_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl6_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl6_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl6_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl6_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl6_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl6_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl6_ddr_key_base_address_upper, bar);
    reg_natc_tbl6_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl6_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl6_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl6_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl6_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl6_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl6_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl6_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl6_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl6_ddr_result_base_address_lower, zeros);
    reg_natc_tbl6_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl6_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl6_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl6_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl6_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl6_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl6_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl6_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl6_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl6_ddr_result_base_address_upper, bar);
    reg_natc_tbl6_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl6_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl6_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl6_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl6_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl6_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl6_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl7_ddr_key_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl7_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl7_ddr_key_base_address_lower, zeros);
    reg_natc_tbl7_ddr_key_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl7_ddr_key_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl7_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl7_ddr_key_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, reg_natc_tbl7_ddr_key_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl7_ddr_key_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl7_ddr_key_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl7_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl7_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl7_ddr_key_base_address_upper, bar);
    reg_natc_tbl7_ddr_key_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl7_ddr_key_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl7_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl7_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, reg_natc_tbl7_ddr_key_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl7_ddr_key_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl7_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set(uint8_t zeros, uint32_t bar)
{
    uint32_t reg_natc_tbl7_ddr_result_base_address_lower = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _3BITS_MAX_VAL_) ||
       (bar >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl7_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl7_ddr_result_base_address_lower, zeros);
    reg_natc_tbl7_ddr_result_base_address_lower = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl7_ddr_result_base_address_lower, bar);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl7_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_get(uint8_t *zeros, uint32_t *bar)
{
    uint32_t reg_natc_tbl7_ddr_result_base_address_lower;

#ifdef VALIDATE_PARMS
    if (!zeros || !bar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, reg_natc_tbl7_ddr_result_base_address_lower);

    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, ZEROS, reg_natc_tbl7_ddr_result_base_address_lower);
    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_natc_tbl7_ddr_result_base_address_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set(uint8_t bar, uint32_t zeros)
{
    uint32_t reg_natc_tbl7_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((zeros >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl7_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl7_ddr_result_base_address_upper, bar);
    reg_natc_tbl7_ddr_result_base_address_upper = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl7_ddr_result_base_address_upper, zeros);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl7_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_get(uint8_t *bar, uint32_t *zeros)
{
    uint32_t reg_natc_tbl7_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!bar || !zeros)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, reg_natc_tbl7_ddr_result_base_address_upper);

    *bar = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_natc_tbl7_ddr_result_base_address_upper);
    *zeros = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER, ZEROS, reg_natc_tbl7_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_0_set(uint8_t cache_hit_count_en, uint8_t cache_miss_count_en, uint8_t ddr_request_count_en, uint8_t ddr_evict_count_en)
{
    uint32_t reg_natc_stat_counter_control_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_natc_stat_counter_control_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_natc_stat_counter_control_0, cache_hit_count_en);
    reg_natc_stat_counter_control_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_natc_stat_counter_control_0, cache_miss_count_en);
    reg_natc_stat_counter_control_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_natc_stat_counter_control_0, ddr_request_count_en);
    reg_natc_stat_counter_control_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_natc_stat_counter_control_0, ddr_evict_count_en);

    RU_REG_WRITE(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, reg_natc_stat_counter_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_0_get(uint8_t *cache_hit_count_en, uint8_t *cache_miss_count_en, uint8_t *ddr_request_count_en, uint8_t *ddr_evict_count_en)
{
    uint32_t reg_natc_stat_counter_control_0;

#ifdef VALIDATE_PARMS
    if (!cache_hit_count_en || !cache_miss_count_en || !ddr_request_count_en || !ddr_evict_count_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, reg_natc_stat_counter_control_0);

    *cache_hit_count_en = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, CACHE_HIT_COUNT_EN, reg_natc_stat_counter_control_0);
    *cache_miss_count_en = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, CACHE_MISS_COUNT_EN, reg_natc_stat_counter_control_0);
    *ddr_request_count_en = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, DDR_REQUEST_COUNT_EN, reg_natc_stat_counter_control_0);
    *ddr_evict_count_en = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0, DDR_EVICT_COUNT_EN, reg_natc_stat_counter_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_1_set(uint8_t ddr_block_count_en, bdmf_boolean counter_wraparound_dis)
{
    uint32_t reg_natc_stat_counter_control_1 = 0;

#ifdef VALIDATE_PARMS
    if ((counter_wraparound_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_stat_counter_control_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_natc_stat_counter_control_1, ddr_block_count_en);
    reg_natc_stat_counter_control_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_natc_stat_counter_control_1, counter_wraparound_dis);

    RU_REG_WRITE(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, reg_natc_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_stat_counter_control_1_get(uint8_t *ddr_block_count_en, bdmf_boolean *counter_wraparound_dis)
{
    uint32_t reg_natc_stat_counter_control_1;

#ifdef VALIDATE_PARMS
    if (!ddr_block_count_en || !counter_wraparound_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, reg_natc_stat_counter_control_1);

    *ddr_block_count_en = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, DDR_BLOCK_COUNT_EN, reg_natc_stat_counter_control_1);
    *counter_wraparound_dis = RU_FIELD_GET(0, NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1, COUNTER_WRAPAROUND_DIS, reg_natc_stat_counter_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_size_set(const nat_cache_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg_natc_ddr_size = 0;

#ifdef VALIDATE_PARMS
    if(!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_ddr_size->ddr_size_tbl0 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl1 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl2 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl3 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl4 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl5 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl6 >= _3BITS_MAX_VAL_) ||
       (natc_ddr_size->ddr_size_tbl7 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL0, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl0);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL1, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl1);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL2, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl2);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL3, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl3);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL4, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl4);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL5, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl5);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL6, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl6);
    reg_natc_ddr_size = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL7, reg_natc_ddr_size, natc_ddr_size->ddr_size_tbl7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_SIZE, reg_natc_ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_size_get(nat_cache_natc_ddr_size *natc_ddr_size)
{
    uint32_t reg_natc_ddr_size;

#ifdef VALIDATE_PARMS
    if (!natc_ddr_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_DDR_SIZE, reg_natc_ddr_size);

    natc_ddr_size->ddr_size_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL0, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL1, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL2, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL3, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL4, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL5, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL6, reg_natc_ddr_size);
    natc_ddr_size->ddr_size_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_SIZE, DDR_SIZE_TBL7, reg_natc_ddr_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_set(uint8_t ddr_bins_per_bucket_tbl0, uint8_t ddr_bins_per_bucket_tbl1, uint8_t ddr_bins_per_bucket_tbl2, uint8_t ddr_bins_per_bucket_tbl3)
{
    uint32_t reg_natc_ddr_bins_per_bucket_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_natc_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg_natc_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl0);
    reg_natc_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg_natc_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl1);
    reg_natc_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg_natc_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl2);
    reg_natc_ddr_bins_per_bucket_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg_natc_ddr_bins_per_bucket_0, ddr_bins_per_bucket_tbl3);

    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, reg_natc_ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_get(uint8_t *ddr_bins_per_bucket_tbl0, uint8_t *ddr_bins_per_bucket_tbl1, uint8_t *ddr_bins_per_bucket_tbl2, uint8_t *ddr_bins_per_bucket_tbl3)
{
    uint32_t reg_natc_ddr_bins_per_bucket_0;

#ifdef VALIDATE_PARMS
    if (!ddr_bins_per_bucket_tbl0 || !ddr_bins_per_bucket_tbl1 || !ddr_bins_per_bucket_tbl2 || !ddr_bins_per_bucket_tbl3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, reg_natc_ddr_bins_per_bucket_0);

    *ddr_bins_per_bucket_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL0, reg_natc_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL1, reg_natc_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL2, reg_natc_ddr_bins_per_bucket_0);
    *ddr_bins_per_bucket_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0, DDR_BINS_PER_BUCKET_TBL3, reg_natc_ddr_bins_per_bucket_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_set(uint8_t ddr_bins_per_bucket_tbl4, uint8_t ddr_bins_per_bucket_tbl5, uint8_t ddr_bins_per_bucket_tbl6, uint8_t ddr_bins_per_bucket_tbl7)
{
    uint32_t reg_natc_ddr_bins_per_bucket_1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_natc_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg_natc_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl4);
    reg_natc_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg_natc_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl5);
    reg_natc_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg_natc_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl6);
    reg_natc_ddr_bins_per_bucket_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg_natc_ddr_bins_per_bucket_1, ddr_bins_per_bucket_tbl7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, reg_natc_ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_get(uint8_t *ddr_bins_per_bucket_tbl4, uint8_t *ddr_bins_per_bucket_tbl5, uint8_t *ddr_bins_per_bucket_tbl6, uint8_t *ddr_bins_per_bucket_tbl7)
{
    uint32_t reg_natc_ddr_bins_per_bucket_1;

#ifdef VALIDATE_PARMS
    if (!ddr_bins_per_bucket_tbl4 || !ddr_bins_per_bucket_tbl5 || !ddr_bins_per_bucket_tbl6 || !ddr_bins_per_bucket_tbl7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, reg_natc_ddr_bins_per_bucket_1);

    *ddr_bins_per_bucket_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL4, reg_natc_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL5, reg_natc_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL6, reg_natc_ddr_bins_per_bucket_1);
    *ddr_bins_per_bucket_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1, DDR_BINS_PER_BUCKET_TBL7, reg_natc_ddr_bins_per_bucket_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_total_len_set(const nat_cache_natc_total_len *natc_total_len)
{
    uint32_t reg_natc_total_len = 0;

#ifdef VALIDATE_PARMS
    if(!natc_total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_total_len->total_len_tbl0 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl1 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl2 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl3 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl4 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl5 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl6 >= _3BITS_MAX_VAL_) ||
       (natc_total_len->total_len_tbl7 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL0, reg_natc_total_len, natc_total_len->total_len_tbl0);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL1, reg_natc_total_len, natc_total_len->total_len_tbl1);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL2, reg_natc_total_len, natc_total_len->total_len_tbl2);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL3, reg_natc_total_len, natc_total_len->total_len_tbl3);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL4, reg_natc_total_len, natc_total_len->total_len_tbl4);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL5, reg_natc_total_len, natc_total_len->total_len_tbl5);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL6, reg_natc_total_len, natc_total_len->total_len_tbl6);
    reg_natc_total_len = RU_FIELD_SET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL7, reg_natc_total_len, natc_total_len->total_len_tbl7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_TOTAL_LEN, reg_natc_total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_total_len_get(nat_cache_natc_total_len *natc_total_len)
{
    uint32_t reg_natc_total_len;

#ifdef VALIDATE_PARMS
    if (!natc_total_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_TOTAL_LEN, reg_natc_total_len);

    natc_total_len->total_len_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL0, reg_natc_total_len);
    natc_total_len->total_len_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL1, reg_natc_total_len);
    natc_total_len->total_len_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL2, reg_natc_total_len);
    natc_total_len->total_len_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL3, reg_natc_total_len);
    natc_total_len->total_len_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL4, reg_natc_total_len);
    natc_total_len->total_len_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL5, reg_natc_total_len);
    natc_total_len->total_len_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL6, reg_natc_total_len);
    natc_total_len->total_len_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_TOTAL_LEN, TOTAL_LEN_TBL7, reg_natc_total_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_sm_status_set(const nat_cache_natc_sm_status *natc_sm_status)
{
    uint32_t reg_natc_sm_status = 0;

#ifdef VALIDATE_PARMS
    if(!natc_sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_sm_status->nat_state >= _15BITS_MAX_VAL_) ||
       (natc_sm_status->wb_state >= _1BITS_MAX_VAL_) ||
       (natc_sm_status->runner_cmd_state >= _1BITS_MAX_VAL_) ||
       (natc_sm_status->ddr_rep_state >= _3BITS_MAX_VAL_) ||
       (natc_sm_status->ddr_req_state >= _2BITS_MAX_VAL_) ||
       (natc_sm_status->apb_state >= _2BITS_MAX_VAL_) ||
       (natc_sm_status->debug_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, NAT_STATE, reg_natc_sm_status, natc_sm_status->nat_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, WB_STATE, reg_natc_sm_status, natc_sm_status->wb_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, RUNNER_CMD_STATE, reg_natc_sm_status, natc_sm_status->runner_cmd_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, DDR_REP_STATE, reg_natc_sm_status, natc_sm_status->ddr_rep_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, DDR_REQ_STATE, reg_natc_sm_status, natc_sm_status->ddr_req_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, APB_STATE, reg_natc_sm_status, natc_sm_status->apb_state);
    reg_natc_sm_status = RU_FIELD_SET(0, NAT_CACHE, NATC_SM_STATUS, DEBUG_SEL, reg_natc_sm_status, natc_sm_status->debug_sel);

    RU_REG_WRITE(0, NAT_CACHE, NATC_SM_STATUS, reg_natc_sm_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_sm_status_get(nat_cache_natc_sm_status *natc_sm_status)
{
    uint32_t reg_natc_sm_status;

#ifdef VALIDATE_PARMS
    if (!natc_sm_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_SM_STATUS, reg_natc_sm_status);

    natc_sm_status->nat_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, NAT_STATE, reg_natc_sm_status);
    natc_sm_status->wb_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, WB_STATE, reg_natc_sm_status);
    natc_sm_status->runner_cmd_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, RUNNER_CMD_STATE, reg_natc_sm_status);
    natc_sm_status->ddr_rep_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, DDR_REP_STATE, reg_natc_sm_status);
    natc_sm_status->ddr_req_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, DDR_REQ_STATE, reg_natc_sm_status);
    natc_sm_status->apb_state = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, APB_STATE, reg_natc_sm_status);
    natc_sm_status->debug_sel = RU_FIELD_GET(0, NAT_CACHE, NATC_SM_STATUS, DEBUG_SEL, reg_natc_sm_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_set(uint8_t regfile_fifo_start_addr_0, uint8_t regfile_fifo_start_addr_1, uint8_t regfile_fifo_start_addr_2, uint8_t regfile_fifo_start_addr_3)
{
    uint32_t reg_natc_regfile_fifo_start_addr_0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_natc_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_0, reg_natc_regfile_fifo_start_addr_0, regfile_fifo_start_addr_0);
    reg_natc_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_1, reg_natc_regfile_fifo_start_addr_0, regfile_fifo_start_addr_1);
    reg_natc_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_2, reg_natc_regfile_fifo_start_addr_0, regfile_fifo_start_addr_2);
    reg_natc_regfile_fifo_start_addr_0 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_3, reg_natc_regfile_fifo_start_addr_0, regfile_fifo_start_addr_3);

    RU_REG_WRITE(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, reg_natc_regfile_fifo_start_addr_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_get(uint8_t *regfile_fifo_start_addr_0, uint8_t *regfile_fifo_start_addr_1, uint8_t *regfile_fifo_start_addr_2, uint8_t *regfile_fifo_start_addr_3)
{
    uint32_t reg_natc_regfile_fifo_start_addr_0;

#ifdef VALIDATE_PARMS
    if (!regfile_fifo_start_addr_0 || !regfile_fifo_start_addr_1 || !regfile_fifo_start_addr_2 || !regfile_fifo_start_addr_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, reg_natc_regfile_fifo_start_addr_0);

    *regfile_fifo_start_addr_0 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_0, reg_natc_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_1 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_1, reg_natc_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_2 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_2, reg_natc_regfile_fifo_start_addr_0);
    *regfile_fifo_start_addr_3 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0, REGFILE_FIFO_START_ADDR_3, reg_natc_regfile_fifo_start_addr_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_set(uint8_t regfile_fifo_start_addr_4, uint8_t regfile_fifo_start_addr_5, uint8_t regfile_fifo_start_addr_6, uint8_t regfile_fifo_start_addr_7)
{
    uint32_t reg_natc_regfile_fifo_start_addr_1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_natc_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_4, reg_natc_regfile_fifo_start_addr_1, regfile_fifo_start_addr_4);
    reg_natc_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_5, reg_natc_regfile_fifo_start_addr_1, regfile_fifo_start_addr_5);
    reg_natc_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_6, reg_natc_regfile_fifo_start_addr_1, regfile_fifo_start_addr_6);
    reg_natc_regfile_fifo_start_addr_1 = RU_FIELD_SET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_7, reg_natc_regfile_fifo_start_addr_1, regfile_fifo_start_addr_7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, reg_natc_regfile_fifo_start_addr_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_get(uint8_t *regfile_fifo_start_addr_4, uint8_t *regfile_fifo_start_addr_5, uint8_t *regfile_fifo_start_addr_6, uint8_t *regfile_fifo_start_addr_7)
{
    uint32_t reg_natc_regfile_fifo_start_addr_1;

#ifdef VALIDATE_PARMS
    if (!regfile_fifo_start_addr_4 || !regfile_fifo_start_addr_5 || !regfile_fifo_start_addr_6 || !regfile_fifo_start_addr_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, reg_natc_regfile_fifo_start_addr_1);

    *regfile_fifo_start_addr_4 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_4, reg_natc_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_5 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_5, reg_natc_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_6 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_6, reg_natc_regfile_fifo_start_addr_1);
    *regfile_fifo_start_addr_7 = RU_FIELD_GET(0, NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1, REGFILE_FIFO_START_ADDR_7, reg_natc_regfile_fifo_start_addr_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl0_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl0_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL0_KEY_MASK, KEY_MASK, reg_natc_tbl0_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL0_KEY_MASK, reg_natc_tbl0_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl0_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl0_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL0_KEY_MASK, reg_natc_tbl0_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL0_KEY_MASK, KEY_MASK, reg_natc_tbl0_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl1_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl1_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL1_KEY_MASK, KEY_MASK, reg_natc_tbl1_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL1_KEY_MASK, reg_natc_tbl1_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl1_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl1_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL1_KEY_MASK, reg_natc_tbl1_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL1_KEY_MASK, KEY_MASK, reg_natc_tbl1_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl2_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl2_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL2_KEY_MASK, KEY_MASK, reg_natc_tbl2_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL2_KEY_MASK, reg_natc_tbl2_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl2_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl2_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL2_KEY_MASK, reg_natc_tbl2_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL2_KEY_MASK, KEY_MASK, reg_natc_tbl2_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl3_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl3_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL3_KEY_MASK, KEY_MASK, reg_natc_tbl3_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL3_KEY_MASK, reg_natc_tbl3_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl3_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl3_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL3_KEY_MASK, reg_natc_tbl3_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL3_KEY_MASK, KEY_MASK, reg_natc_tbl3_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl4_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl4_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL4_KEY_MASK, KEY_MASK, reg_natc_tbl4_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL4_KEY_MASK, reg_natc_tbl4_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl4_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl4_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL4_KEY_MASK, reg_natc_tbl4_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL4_KEY_MASK, KEY_MASK, reg_natc_tbl4_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl5_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl5_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL5_KEY_MASK, KEY_MASK, reg_natc_tbl5_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL5_KEY_MASK, reg_natc_tbl5_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl5_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl5_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL5_KEY_MASK, reg_natc_tbl5_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL5_KEY_MASK, KEY_MASK, reg_natc_tbl5_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl6_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl6_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL6_KEY_MASK, KEY_MASK, reg_natc_tbl6_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL6_KEY_MASK, reg_natc_tbl6_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl6_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl6_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL6_KEY_MASK, reg_natc_tbl6_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL6_KEY_MASK, KEY_MASK, reg_natc_tbl6_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_key_mask_set(uint32_t index, uint32_t key_mask)
{
    uint32_t reg_natc_tbl7_key_mask = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_tbl7_key_mask = RU_FIELD_SET(0, NAT_CACHE, NATC_TBL7_KEY_MASK, KEY_MASK, reg_natc_tbl7_key_mask, key_mask);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, NATC_TBL7_KEY_MASK, reg_natc_tbl7_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_tbl7_key_mask_get(uint32_t index, uint32_t *key_mask)
{
    uint32_t reg_natc_tbl7_key_mask;

#ifdef VALIDATE_PARMS
    if (!key_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, NATC_TBL7_KEY_MASK, reg_natc_tbl7_key_mask);

    *key_mask = RU_FIELD_GET(0, NAT_CACHE, NATC_TBL7_KEY_MASK, KEY_MASK, reg_natc_tbl7_key_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_hash_mode_set(const nat_cache_natc_ddr_hash_mode *natc_ddr_hash_mode)
{
    uint32_t reg_natc_ddr_hash_mode = 0;

#ifdef VALIDATE_PARMS
    if(!natc_ddr_hash_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_ddr_hash_mode->ddr_hash_mode_tbl0 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl1 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl2 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl3 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl4 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl5 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl6 >= _4BITS_MAX_VAL_) ||
       (natc_ddr_hash_mode->ddr_hash_mode_tbl7 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL0, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl0);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL1, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl1);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL2, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl2);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL3, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl3);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL4, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl4);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL5, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl5);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL6, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl6);
    reg_natc_ddr_hash_mode = RU_FIELD_SET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL7, reg_natc_ddr_hash_mode, natc_ddr_hash_mode->ddr_hash_mode_tbl7);

    RU_REG_WRITE(0, NAT_CACHE, NATC_DDR_HASH_MODE, reg_natc_ddr_hash_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_ddr_hash_mode_get(nat_cache_natc_ddr_hash_mode *natc_ddr_hash_mode)
{
    uint32_t reg_natc_ddr_hash_mode;

#ifdef VALIDATE_PARMS
    if (!natc_ddr_hash_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_DDR_HASH_MODE, reg_natc_ddr_hash_mode);

    natc_ddr_hash_mode->ddr_hash_mode_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL0, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL1, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL2, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL3, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL4, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL5, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL6, reg_natc_ddr_hash_mode);
    natc_ddr_hash_mode->ddr_hash_mode_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_DDR_HASH_MODE, DDR_HASH_MODE_TBL7, reg_natc_ddr_hash_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_flow_cntr_cntl_set(const nat_cache_natc_flow_cntr_cntl *natc_flow_cntr_cntl)
{
    uint32_t reg_natc_flow_cntr_cntl = 0;

#ifdef VALIDATE_PARMS
    if(!natc_flow_cntr_cntl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((natc_flow_cntr_cntl->context_offset >= _7BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl0 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl1 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl2 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl3 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl4 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl5 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl6 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_en_tbl7 >= _1BITS_MAX_VAL_) ||
       (natc_flow_cntr_cntl->flow_cntr_clr_on_rd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, CONTEXT_OFFSET, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->context_offset);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL0, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl0);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL1, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl1);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL2, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl2);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL3, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl3);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL4, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl4);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL5, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl5);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL6, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl6);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL7, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_en_tbl7);
    reg_natc_flow_cntr_cntl = RU_FIELD_SET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_CLR_ON_RD_EN, reg_natc_flow_cntr_cntl, natc_flow_cntr_cntl->flow_cntr_clr_on_rd_en);

    RU_REG_WRITE(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, reg_natc_flow_cntr_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_natc_flow_cntr_cntl_get(nat_cache_natc_flow_cntr_cntl *natc_flow_cntr_cntl)
{
    uint32_t reg_natc_flow_cntr_cntl;

#ifdef VALIDATE_PARMS
    if (!natc_flow_cntr_cntl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, reg_natc_flow_cntr_cntl);

    natc_flow_cntr_cntl->context_offset = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, CONTEXT_OFFSET, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl0 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL0, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl1 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL1, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl2 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL2, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl3 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL3, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl4 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL4, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl5 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL5, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl6 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL6, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_en_tbl7 = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_EN_TBL7, reg_natc_flow_cntr_cntl);
    natc_flow_cntr_cntl->flow_cntr_clr_on_rd_en = RU_FIELD_GET(0, NAT_CACHE, NATC_FLOW_CNTR_CNTL, FLOW_CNTR_CLR_ON_RD_EN, reg_natc_flow_cntr_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_set(uint16_t natc_entry, bdmf_boolean w_r)
{
    uint32_t reg_indir_rw_natc_indir_addr_reg = 0;

#ifdef VALIDATE_PARMS
    if ((natc_entry >= _10BITS_MAX_VAL_) ||
       (w_r >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indir_rw_natc_indir_addr_reg = RU_FIELD_SET(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, NATC_ENTRY, reg_indir_rw_natc_indir_addr_reg, natc_entry);
    reg_indir_rw_natc_indir_addr_reg = RU_FIELD_SET(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, W_R, reg_indir_rw_natc_indir_addr_reg, w_r);

    RU_REG_WRITE(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, reg_indir_rw_natc_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_get(uint16_t *natc_entry, bdmf_boolean *w_r)
{
    uint32_t reg_indir_rw_natc_indir_addr_reg;

#ifdef VALIDATE_PARMS
    if (!natc_entry || !w_r)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, reg_indir_rw_natc_indir_addr_reg);

    *natc_entry = RU_FIELD_GET(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, NATC_ENTRY, reg_indir_rw_natc_indir_addr_reg);
    *w_r = RU_FIELD_GET(0, NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG, W_R, reg_indir_rw_natc_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_set(uint8_t flow_cntr_entry, bdmf_boolean w_r)
{
    uint32_t reg_indir_rw_natc_flow_cntr_indir_addr_reg = 0;

#ifdef VALIDATE_PARMS
    if ((flow_cntr_entry >= _7BITS_MAX_VAL_) ||
       (w_r >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indir_rw_natc_flow_cntr_indir_addr_reg = RU_FIELD_SET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, FLOW_CNTR_ENTRY, reg_indir_rw_natc_flow_cntr_indir_addr_reg, flow_cntr_entry);
    reg_indir_rw_natc_flow_cntr_indir_addr_reg = RU_FIELD_SET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, W_R, reg_indir_rw_natc_flow_cntr_indir_addr_reg, w_r);

    RU_REG_WRITE(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, reg_indir_rw_natc_flow_cntr_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_get(uint8_t *flow_cntr_entry, bdmf_boolean *w_r)
{
    uint32_t reg_indir_rw_natc_flow_cntr_indir_addr_reg;

#ifdef VALIDATE_PARMS
    if (!flow_cntr_entry || !w_r)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, reg_indir_rw_natc_flow_cntr_indir_addr_reg);

    *flow_cntr_entry = RU_FIELD_GET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, FLOW_CNTR_ENTRY, reg_indir_rw_natc_flow_cntr_indir_addr_reg);
    *w_r = RU_FIELD_GET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG, W_R, reg_indir_rw_natc_flow_cntr_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(uint32_t index, uint32_t data)
{
    uint32_t reg_indir_rw_natc_flow_cntr_indir_data_reg = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indir_rw_natc_flow_cntr_indir_data_reg = RU_FIELD_SET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG, DATA, reg_indir_rw_natc_flow_cntr_indir_data_reg, data);

    RU_REG_RAM_WRITE(0, index, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG, reg_indir_rw_natc_flow_cntr_indir_data_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get(uint32_t index, uint32_t *data)
{
    uint32_t reg_indir_rw_natc_flow_cntr_indir_data_reg;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG, reg_indir_rw_natc_flow_cntr_indir_data_reg);

    *data = RU_FIELD_GET(0, NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG, DATA, reg_indir_rw_natc_flow_cntr_indir_data_reg);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_natc_control_status,
    bdmf_address_natc_control_status2,
    bdmf_address_natc_table_control,
    bdmf_address_nat0_command_status,
    bdmf_address_nat0_hash,
    bdmf_address_nat0_hit_count,
    bdmf_address_nat0_byte_count,
    bdmf_address_nat0_pkt_len,
    bdmf_address_nat0_key_result,
    bdmf_address_natc_tbl0_ddr_key_base_address_lower,
    bdmf_address_natc_tbl0_ddr_key_base_address_upper,
    bdmf_address_natc_tbl0_ddr_result_base_address_lower,
    bdmf_address_natc_tbl0_ddr_result_base_address_upper,
    bdmf_address_natc_tbl1_ddr_key_base_address_lower,
    bdmf_address_natc_tbl1_ddr_key_base_address_upper,
    bdmf_address_natc_tbl1_ddr_result_base_address_lower,
    bdmf_address_natc_tbl1_ddr_result_base_address_upper,
    bdmf_address_natc_tbl2_ddr_key_base_address_lower,
    bdmf_address_natc_tbl2_ddr_key_base_address_upper,
    bdmf_address_natc_tbl2_ddr_result_base_address_lower,
    bdmf_address_natc_tbl2_ddr_result_base_address_upper,
    bdmf_address_natc_tbl3_ddr_key_base_address_lower,
    bdmf_address_natc_tbl3_ddr_key_base_address_upper,
    bdmf_address_natc_tbl3_ddr_result_base_address_lower,
    bdmf_address_natc_tbl3_ddr_result_base_address_upper,
    bdmf_address_natc_tbl4_ddr_key_base_address_lower,
    bdmf_address_natc_tbl4_ddr_key_base_address_upper,
    bdmf_address_natc_tbl4_ddr_result_base_address_lower,
    bdmf_address_natc_tbl4_ddr_result_base_address_upper,
    bdmf_address_natc_tbl5_ddr_key_base_address_lower,
    bdmf_address_natc_tbl5_ddr_key_base_address_upper,
    bdmf_address_natc_tbl5_ddr_result_base_address_lower,
    bdmf_address_natc_tbl5_ddr_result_base_address_upper,
    bdmf_address_natc_tbl6_ddr_key_base_address_lower,
    bdmf_address_natc_tbl6_ddr_key_base_address_upper,
    bdmf_address_natc_tbl6_ddr_result_base_address_lower,
    bdmf_address_natc_tbl6_ddr_result_base_address_upper,
    bdmf_address_natc_tbl7_ddr_key_base_address_lower,
    bdmf_address_natc_tbl7_ddr_key_base_address_upper,
    bdmf_address_natc_tbl7_ddr_result_base_address_lower,
    bdmf_address_natc_tbl7_ddr_result_base_address_upper,
    bdmf_address_natc_stat_counter_control_0,
    bdmf_address_natc_stat_counter_control_1,
    bdmf_address_natc_cache_hit_count,
    bdmf_address_natc_cache_miss_count,
    bdmf_address_natc_ddr_request_count,
    bdmf_address_natc_ddr_evict_count,
    bdmf_address_natc_ddr_block_count,
    bdmf_address_natc_ddr_size,
    bdmf_address_natc_ddr_bins_per_bucket_0,
    bdmf_address_natc_ddr_bins_per_bucket_1,
    bdmf_address_natc_total_len,
    bdmf_address_natc_sm_status,
    bdmf_address_natc_regfile_fifo_start_addr_0,
    bdmf_address_natc_regfile_fifo_start_addr_1,
    bdmf_address_natc_tbl0_key_mask,
    bdmf_address_natc_tbl1_key_mask,
    bdmf_address_natc_tbl2_key_mask,
    bdmf_address_natc_tbl3_key_mask,
    bdmf_address_natc_tbl4_key_mask,
    bdmf_address_natc_tbl5_key_mask,
    bdmf_address_natc_tbl6_key_mask,
    bdmf_address_natc_tbl7_key_mask,
    bdmf_address_natc_ddr_hash_mode,
    bdmf_address_natc_flow_cntr_cntl,
    bdmf_address_indir_rw_natc_indir_addr_reg,
    bdmf_address_indir_rw_natc_indir_data_reg,
    bdmf_address_indir_rw_natc_flow_cntr_indir_addr_reg,
    bdmf_address_indir_rw_natc_flow_cntr_indir_data_reg,
}
bdmf_address;

static int ag_drv_nat_cache_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_nat_cache_ctrl_status:
    {
        nat_cache_ctrl_status ctrl_status = { .natc_reset = parm[1].value.unumber, .natc_enable = parm[2].value.unumber, .regfile_fifo_reset = parm[3].value.unumber, .natc_smem_clear_by_update_disable = parm[4].value.unumber, .natc_smem_increment_on_reg_lookup = parm[5].value.unumber, .nat_arb_st = parm[6].value.unumber, .decr_count_wraparound_enable = parm[7].value.unumber, .multi_hash_limit = parm[8].value.unumber, .nat_hash_mode = parm[9].value.unumber, .ddr_disable_on_reg_lookup = parm[10].value.unumber, .cache_update_on_ddr_miss = parm[11].value.unumber, .pending_fifo_entry_check_enable = parm[12].value.unumber, .unused1 = parm[13].value.unumber, .reg_8bit_in_32bit_swap_control = parm[14].value.unumber, .reg_32bit_in_64bit_swap_control = parm[15].value.unumber, .repeated_key_det_en = parm[16].value.unumber, .ddr_swap_all_control = parm[17].value.unumber, .smem_8bit_in_32bit_swap_control = parm[18].value.unumber, .smem_32bit_in_64bit_swap_control = parm[19].value.unumber, .ddr_64bit_in_128bit_swap_control = parm[20].value.unumber, .natc_init_done = parm[21].value.unumber, .unused0 = parm[22].value.unumber, .natc_add_command_speedup_mode = parm[23].value.unumber, .ddr_enable = parm[24].value.unumber, .unused2 = parm[25].value.unumber, .ddr_lookup_pending_fifo_mode_disable = parm[26].value.unumber, .ddr_replace_duplicated_cached_entry_enable = parm[27].value.unumber, .ddr_hash_swap = parm[28].value.unumber, .ddr_counter_8bit_in_32bit_swap_control = parm[29].value.unumber, .cache_update_on_reg_ddr_lookup = parm[30].value.unumber, .unused3 = parm[31].value.unumber, .unused4 = parm[32].value.unumber, .cache_algo = parm[33].value.unumber, .age_timer = parm[34].value.unumber, .age_timer_tick = parm[35].value.unumber, .cache_lookup_blocking_mode = parm[36].value.unumber, .ddr_8bit_in_32bit_swap_control = parm[37].value.unumber, .ddr_32bit_in_64bit_swap_control = parm[38].value.unumber, .unused5 = parm[39].value.unumber};
        ag_err = ag_drv_nat_cache_ctrl_status_set(&ctrl_status);
        break;
    }
    case cli_nat_cache_natc_ctrs:
    {
        nat_cache_natc_ctrs natc_ctrs = { .cache_hit_count = parm[1].value.unumber, .cache_miss_count = parm[2].value.unumber, .ddr_request_count = parm[3].value.unumber, .ddr_evict_count = parm[4].value.unumber, .ddr_block_count = parm[5].value.unumber};
        ag_err = ag_drv_nat_cache_natc_ctrs_set(&natc_ctrs);
        break;
    }
    case cli_nat_cache_natc_table_control:
    {
        nat_cache_natc_table_control natc_table_control = { .non_cacheable_tbl0 = parm[1].value.unumber, .key_len_tbl0 = parm[2].value.unumber, .non_cacheable_tbl1 = parm[3].value.unumber, .key_len_tbl1 = parm[4].value.unumber, .non_cacheable_tbl2 = parm[5].value.unumber, .key_len_tbl2 = parm[6].value.unumber, .non_cacheable_tbl3 = parm[7].value.unumber, .key_len_tbl3 = parm[8].value.unumber, .non_cacheable_tbl4 = parm[9].value.unumber, .key_len_tbl4 = parm[10].value.unumber, .non_cacheable_tbl5 = parm[11].value.unumber, .key_len_tbl5 = parm[12].value.unumber, .non_cacheable_tbl6 = parm[13].value.unumber, .key_len_tbl6 = parm[14].value.unumber, .non_cacheable_tbl7 = parm[15].value.unumber, .key_len_tbl7 = parm[16].value.unumber, .var_context_len_en_tbl0 = parm[17].value.unumber, .var_context_len_en_tbl1 = parm[18].value.unumber, .var_context_len_en_tbl2 = parm[19].value.unumber, .var_context_len_en_tbl3 = parm[20].value.unumber, .var_context_len_en_tbl4 = parm[21].value.unumber, .var_context_len_en_tbl5 = parm[22].value.unumber, .var_context_len_en_tbl6 = parm[23].value.unumber, .var_context_len_en_tbl7 = parm[24].value.unumber, .smem_dis_tbl0 = parm[25].value.unumber, .smem_dis_tbl1 = parm[26].value.unumber, .smem_dis_tbl2 = parm[27].value.unumber, .smem_dis_tbl3 = parm[28].value.unumber, .smem_dis_tbl4 = parm[29].value.unumber, .smem_dis_tbl5 = parm[30].value.unumber, .smem_dis_tbl6 = parm[31].value.unumber, .smem_dis_tbl7 = parm[32].value.unumber};
        ag_err = ag_drv_nat_cache_natc_table_control_set(&natc_table_control);
        break;
    }
    case cli_nat_cache_nat0_command_status:
    {
        nat_cache_nat0_command_status nat0_command_status = { .command = parm[1].value.unumber, .unused0 = parm[2].value.unumber, .busy = parm[3].value.unumber, .error = parm[4].value.unumber, .miss = parm[5].value.unumber, .cache_hit = parm[6].value.unumber, .multihash_count = parm[7].value.unumber, .nat_tbl = parm[8].value.unumber, .decr_count = parm[9].value.unumber, .cache_flush = parm[10].value.unumber, .del_cmd_mode = parm[11].value.unumber, .add_cmd_mode = parm[12].value.unumber, .del_cmd_ddr_bin = parm[13].value.unumber, .add_cmd_ddr_bin = parm[14].value.unumber, .add_cmd_ddr_miss = parm[15].value.unumber};
        ag_err = ag_drv_nat_cache_nat0_command_status_set(&nat0_command_status);
        break;
    }
    case cli_nat_cache_nat0_pkt_len:
        ag_err = ag_drv_nat_cache_nat0_pkt_len_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_nat0_key_result:
    {
        nat_cache_nat0_key_result nat0_key_result = { .data = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber}};
        ag_err = ag_drv_nat_cache_nat0_key_result_set(parm[1].value.unumber, &nat0_key_result);
        break;
    }
    case cli_nat_cache_natc_tbl0_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl0_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl0_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl0_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl1_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl1_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl1_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl1_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl2_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl2_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl2_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl2_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl3_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl3_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl3_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl3_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl4_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl4_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl4_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl4_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl5_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl5_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl5_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl5_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl6_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl6_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl6_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl6_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl7_ddr_key_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl7_ddr_key_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl7_ddr_result_base_address_lower:
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl7_ddr_result_base_address_upper:
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_stat_counter_control_0:
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_nat_cache_natc_stat_counter_control_1:
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_ddr_size:
    {
        nat_cache_natc_ddr_size natc_ddr_size = { .ddr_size_tbl0 = parm[1].value.unumber, .ddr_size_tbl1 = parm[2].value.unumber, .ddr_size_tbl2 = parm[3].value.unumber, .ddr_size_tbl3 = parm[4].value.unumber, .ddr_size_tbl4 = parm[5].value.unumber, .ddr_size_tbl5 = parm[6].value.unumber, .ddr_size_tbl6 = parm[7].value.unumber, .ddr_size_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_nat_cache_natc_ddr_size_set(&natc_ddr_size);
        break;
    }
    case cli_nat_cache_natc_ddr_bins_per_bucket_0:
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_nat_cache_natc_ddr_bins_per_bucket_1:
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_nat_cache_natc_total_len:
    {
        nat_cache_natc_total_len natc_total_len = { .total_len_tbl0 = parm[1].value.unumber, .total_len_tbl1 = parm[2].value.unumber, .total_len_tbl2 = parm[3].value.unumber, .total_len_tbl3 = parm[4].value.unumber, .total_len_tbl4 = parm[5].value.unumber, .total_len_tbl5 = parm[6].value.unumber, .total_len_tbl6 = parm[7].value.unumber, .total_len_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_nat_cache_natc_total_len_set(&natc_total_len);
        break;
    }
    case cli_nat_cache_natc_sm_status:
    {
        nat_cache_natc_sm_status natc_sm_status = { .nat_state = parm[1].value.unumber, .wb_state = parm[2].value.unumber, .runner_cmd_state = parm[3].value.unumber, .ddr_rep_state = parm[4].value.unumber, .ddr_req_state = parm[5].value.unumber, .apb_state = parm[6].value.unumber, .debug_sel = parm[7].value.unumber};
        ag_err = ag_drv_nat_cache_natc_sm_status_set(&natc_sm_status);
        break;
    }
    case cli_nat_cache_natc_regfile_fifo_start_addr_0:
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_nat_cache_natc_regfile_fifo_start_addr_1:
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_nat_cache_natc_tbl0_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl1_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl2_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl3_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl4_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl5_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl6_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_tbl7_key_mask:
        ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_natc_ddr_hash_mode:
    {
        nat_cache_natc_ddr_hash_mode natc_ddr_hash_mode = { .ddr_hash_mode_tbl0 = parm[1].value.unumber, .ddr_hash_mode_tbl1 = parm[2].value.unumber, .ddr_hash_mode_tbl2 = parm[3].value.unumber, .ddr_hash_mode_tbl3 = parm[4].value.unumber, .ddr_hash_mode_tbl4 = parm[5].value.unumber, .ddr_hash_mode_tbl5 = parm[6].value.unumber, .ddr_hash_mode_tbl6 = parm[7].value.unumber, .ddr_hash_mode_tbl7 = parm[8].value.unumber};
        ag_err = ag_drv_nat_cache_natc_ddr_hash_mode_set(&natc_ddr_hash_mode);
        break;
    }
    case cli_nat_cache_natc_flow_cntr_cntl:
    {
        nat_cache_natc_flow_cntr_cntl natc_flow_cntr_cntl = { .context_offset = parm[1].value.unumber, .flow_cntr_en_tbl0 = parm[2].value.unumber, .flow_cntr_en_tbl1 = parm[3].value.unumber, .flow_cntr_en_tbl2 = parm[4].value.unumber, .flow_cntr_en_tbl3 = parm[5].value.unumber, .flow_cntr_en_tbl4 = parm[6].value.unumber, .flow_cntr_en_tbl5 = parm[7].value.unumber, .flow_cntr_en_tbl6 = parm[8].value.unumber, .flow_cntr_en_tbl7 = parm[9].value.unumber, .flow_cntr_clr_on_rd_en = parm[10].value.unumber};
        ag_err = ag_drv_nat_cache_natc_flow_cntr_cntl_set(&natc_flow_cntr_cntl);
        break;
    }
    case cli_nat_cache_indir_rw_natc_indir_addr_reg:
        ag_err = ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg:
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg:
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_nat_cache_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_nat_cache_ctrl_status:
    {
        nat_cache_ctrl_status ctrl_status;
        ag_err = ag_drv_nat_cache_ctrl_status_get(&ctrl_status);
        bdmf_session_print(session, "natc_reset = %u = 0x%x\n", ctrl_status.natc_reset, ctrl_status.natc_reset);
        bdmf_session_print(session, "natc_enable = %u = 0x%x\n", ctrl_status.natc_enable, ctrl_status.natc_enable);
        bdmf_session_print(session, "regfile_fifo_reset = %u = 0x%x\n", ctrl_status.regfile_fifo_reset, ctrl_status.regfile_fifo_reset);
        bdmf_session_print(session, "natc_smem_clear_by_update_disable = %u = 0x%x\n", ctrl_status.natc_smem_clear_by_update_disable, ctrl_status.natc_smem_clear_by_update_disable);
        bdmf_session_print(session, "natc_smem_increment_on_reg_lookup = %u = 0x%x\n", ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.natc_smem_increment_on_reg_lookup);
        bdmf_session_print(session, "nat_arb_st = %u = 0x%x\n", ctrl_status.nat_arb_st, ctrl_status.nat_arb_st);
        bdmf_session_print(session, "decr_count_wraparound_enable = %u = 0x%x\n", ctrl_status.decr_count_wraparound_enable, ctrl_status.decr_count_wraparound_enable);
        bdmf_session_print(session, "multi_hash_limit = %u = 0x%x\n", ctrl_status.multi_hash_limit, ctrl_status.multi_hash_limit);
        bdmf_session_print(session, "nat_hash_mode = %u = 0x%x\n", ctrl_status.nat_hash_mode, ctrl_status.nat_hash_mode);
        bdmf_session_print(session, "ddr_disable_on_reg_lookup = %u = 0x%x\n", ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.ddr_disable_on_reg_lookup);
        bdmf_session_print(session, "cache_update_on_ddr_miss = %u = 0x%x\n", ctrl_status.cache_update_on_ddr_miss, ctrl_status.cache_update_on_ddr_miss);
        bdmf_session_print(session, "pending_fifo_entry_check_enable = %u = 0x%x\n", ctrl_status.pending_fifo_entry_check_enable, ctrl_status.pending_fifo_entry_check_enable);
        bdmf_session_print(session, "unused1 = %u = 0x%x\n", ctrl_status.unused1, ctrl_status.unused1);
        bdmf_session_print(session, "reg_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.reg_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "reg_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.reg_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "repeated_key_det_en = %u = 0x%x\n", ctrl_status.repeated_key_det_en, ctrl_status.repeated_key_det_en);
        bdmf_session_print(session, "ddr_swap_all_control = %u = 0x%x\n", ctrl_status.ddr_swap_all_control, ctrl_status.ddr_swap_all_control);
        bdmf_session_print(session, "smem_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.smem_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "smem_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "ddr_64bit_in_128bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_64bit_in_128bit_swap_control, ctrl_status.ddr_64bit_in_128bit_swap_control);
        bdmf_session_print(session, "natc_init_done = %u = 0x%x\n", ctrl_status.natc_init_done, ctrl_status.natc_init_done);
        bdmf_session_print(session, "unused0 = %u = 0x%x\n", ctrl_status.unused0, ctrl_status.unused0);
        bdmf_session_print(session, "natc_add_command_speedup_mode = %u = 0x%x\n", ctrl_status.natc_add_command_speedup_mode, ctrl_status.natc_add_command_speedup_mode);
        bdmf_session_print(session, "ddr_enable = %u = 0x%x\n", ctrl_status.ddr_enable, ctrl_status.ddr_enable);
        bdmf_session_print(session, "unused2 = %u = 0x%x\n", ctrl_status.unused2, ctrl_status.unused2);
        bdmf_session_print(session, "ddr_lookup_pending_fifo_mode_disable = %u = 0x%x\n", ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.ddr_lookup_pending_fifo_mode_disable);
        bdmf_session_print(session, "ddr_replace_duplicated_cached_entry_enable = %u = 0x%x\n", ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_replace_duplicated_cached_entry_enable);
        bdmf_session_print(session, "ddr_hash_swap = %u = 0x%x\n", ctrl_status.ddr_hash_swap, ctrl_status.ddr_hash_swap);
        bdmf_session_print(session, "ddr_counter_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.ddr_counter_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "cache_update_on_reg_ddr_lookup = %u = 0x%x\n", ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.cache_update_on_reg_ddr_lookup);
        bdmf_session_print(session, "unused3 = %u = 0x%x\n", ctrl_status.unused3, ctrl_status.unused3);
        bdmf_session_print(session, "unused4 = %u = 0x%x\n", ctrl_status.unused4, ctrl_status.unused4);
        bdmf_session_print(session, "cache_algo = %u = 0x%x\n", ctrl_status.cache_algo, ctrl_status.cache_algo);
        bdmf_session_print(session, "age_timer = %u = 0x%x\n", ctrl_status.age_timer, ctrl_status.age_timer);
        bdmf_session_print(session, "age_timer_tick = %u = 0x%x\n", ctrl_status.age_timer_tick, ctrl_status.age_timer_tick);
        bdmf_session_print(session, "cache_lookup_blocking_mode = %u = 0x%x\n", ctrl_status.cache_lookup_blocking_mode, ctrl_status.cache_lookup_blocking_mode);
        bdmf_session_print(session, "ddr_8bit_in_32bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.ddr_8bit_in_32bit_swap_control);
        bdmf_session_print(session, "ddr_32bit_in_64bit_swap_control = %u = 0x%x\n", ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.ddr_32bit_in_64bit_swap_control);
        bdmf_session_print(session, "unused5 = %u = 0x%x\n", ctrl_status.unused5, ctrl_status.unused5);
        break;
    }
    case cli_nat_cache_natc_ctrs:
    {
        nat_cache_natc_ctrs natc_ctrs;
        ag_err = ag_drv_nat_cache_natc_ctrs_get(&natc_ctrs);
        bdmf_session_print(session, "cache_hit_count = %u = 0x%x\n", natc_ctrs.cache_hit_count, natc_ctrs.cache_hit_count);
        bdmf_session_print(session, "cache_miss_count = %u = 0x%x\n", natc_ctrs.cache_miss_count, natc_ctrs.cache_miss_count);
        bdmf_session_print(session, "ddr_request_count = %u = 0x%x\n", natc_ctrs.ddr_request_count, natc_ctrs.ddr_request_count);
        bdmf_session_print(session, "ddr_evict_count = %u = 0x%x\n", natc_ctrs.ddr_evict_count, natc_ctrs.ddr_evict_count);
        bdmf_session_print(session, "ddr_block_count = %u = 0x%x\n", natc_ctrs.ddr_block_count, natc_ctrs.ddr_block_count);
        break;
    }
    case cli_nat_cache_natc_table_control:
    {
        nat_cache_natc_table_control natc_table_control;
        ag_err = ag_drv_nat_cache_natc_table_control_get(&natc_table_control);
        bdmf_session_print(session, "non_cacheable_tbl0 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl0, natc_table_control.non_cacheable_tbl0);
        bdmf_session_print(session, "key_len_tbl0 = %u = 0x%x\n", natc_table_control.key_len_tbl0, natc_table_control.key_len_tbl0);
        bdmf_session_print(session, "non_cacheable_tbl1 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl1, natc_table_control.non_cacheable_tbl1);
        bdmf_session_print(session, "key_len_tbl1 = %u = 0x%x\n", natc_table_control.key_len_tbl1, natc_table_control.key_len_tbl1);
        bdmf_session_print(session, "non_cacheable_tbl2 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl2, natc_table_control.non_cacheable_tbl2);
        bdmf_session_print(session, "key_len_tbl2 = %u = 0x%x\n", natc_table_control.key_len_tbl2, natc_table_control.key_len_tbl2);
        bdmf_session_print(session, "non_cacheable_tbl3 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl3, natc_table_control.non_cacheable_tbl3);
        bdmf_session_print(session, "key_len_tbl3 = %u = 0x%x\n", natc_table_control.key_len_tbl3, natc_table_control.key_len_tbl3);
        bdmf_session_print(session, "non_cacheable_tbl4 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl4, natc_table_control.non_cacheable_tbl4);
        bdmf_session_print(session, "key_len_tbl4 = %u = 0x%x\n", natc_table_control.key_len_tbl4, natc_table_control.key_len_tbl4);
        bdmf_session_print(session, "non_cacheable_tbl5 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl5, natc_table_control.non_cacheable_tbl5);
        bdmf_session_print(session, "key_len_tbl5 = %u = 0x%x\n", natc_table_control.key_len_tbl5, natc_table_control.key_len_tbl5);
        bdmf_session_print(session, "non_cacheable_tbl6 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl6, natc_table_control.non_cacheable_tbl6);
        bdmf_session_print(session, "key_len_tbl6 = %u = 0x%x\n", natc_table_control.key_len_tbl6, natc_table_control.key_len_tbl6);
        bdmf_session_print(session, "non_cacheable_tbl7 = %u = 0x%x\n", natc_table_control.non_cacheable_tbl7, natc_table_control.non_cacheable_tbl7);
        bdmf_session_print(session, "key_len_tbl7 = %u = 0x%x\n", natc_table_control.key_len_tbl7, natc_table_control.key_len_tbl7);
        bdmf_session_print(session, "var_context_len_en_tbl0 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl0, natc_table_control.var_context_len_en_tbl0);
        bdmf_session_print(session, "var_context_len_en_tbl1 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl1, natc_table_control.var_context_len_en_tbl1);
        bdmf_session_print(session, "var_context_len_en_tbl2 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl2, natc_table_control.var_context_len_en_tbl2);
        bdmf_session_print(session, "var_context_len_en_tbl3 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl3, natc_table_control.var_context_len_en_tbl3);
        bdmf_session_print(session, "var_context_len_en_tbl4 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl4, natc_table_control.var_context_len_en_tbl4);
        bdmf_session_print(session, "var_context_len_en_tbl5 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl5, natc_table_control.var_context_len_en_tbl5);
        bdmf_session_print(session, "var_context_len_en_tbl6 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl6, natc_table_control.var_context_len_en_tbl6);
        bdmf_session_print(session, "var_context_len_en_tbl7 = %u = 0x%x\n", natc_table_control.var_context_len_en_tbl7, natc_table_control.var_context_len_en_tbl7);
        bdmf_session_print(session, "smem_dis_tbl0 = %u = 0x%x\n", natc_table_control.smem_dis_tbl0, natc_table_control.smem_dis_tbl0);
        bdmf_session_print(session, "smem_dis_tbl1 = %u = 0x%x\n", natc_table_control.smem_dis_tbl1, natc_table_control.smem_dis_tbl1);
        bdmf_session_print(session, "smem_dis_tbl2 = %u = 0x%x\n", natc_table_control.smem_dis_tbl2, natc_table_control.smem_dis_tbl2);
        bdmf_session_print(session, "smem_dis_tbl3 = %u = 0x%x\n", natc_table_control.smem_dis_tbl3, natc_table_control.smem_dis_tbl3);
        bdmf_session_print(session, "smem_dis_tbl4 = %u = 0x%x\n", natc_table_control.smem_dis_tbl4, natc_table_control.smem_dis_tbl4);
        bdmf_session_print(session, "smem_dis_tbl5 = %u = 0x%x\n", natc_table_control.smem_dis_tbl5, natc_table_control.smem_dis_tbl5);
        bdmf_session_print(session, "smem_dis_tbl6 = %u = 0x%x\n", natc_table_control.smem_dis_tbl6, natc_table_control.smem_dis_tbl6);
        bdmf_session_print(session, "smem_dis_tbl7 = %u = 0x%x\n", natc_table_control.smem_dis_tbl7, natc_table_control.smem_dis_tbl7);
        break;
    }
    case cli_nat_cache_nat0_command_status:
    {
        nat_cache_nat0_command_status nat0_command_status;
        ag_err = ag_drv_nat_cache_nat0_command_status_get(&nat0_command_status);
        bdmf_session_print(session, "command = %u = 0x%x\n", nat0_command_status.command, nat0_command_status.command);
        bdmf_session_print(session, "unused0 = %u = 0x%x\n", nat0_command_status.unused0, nat0_command_status.unused0);
        bdmf_session_print(session, "busy = %u = 0x%x\n", nat0_command_status.busy, nat0_command_status.busy);
        bdmf_session_print(session, "error = %u = 0x%x\n", nat0_command_status.error, nat0_command_status.error);
        bdmf_session_print(session, "miss = %u = 0x%x\n", nat0_command_status.miss, nat0_command_status.miss);
        bdmf_session_print(session, "cache_hit = %u = 0x%x\n", nat0_command_status.cache_hit, nat0_command_status.cache_hit);
        bdmf_session_print(session, "multihash_count = %u = 0x%x\n", nat0_command_status.multihash_count, nat0_command_status.multihash_count);
        bdmf_session_print(session, "nat_tbl = %u = 0x%x\n", nat0_command_status.nat_tbl, nat0_command_status.nat_tbl);
        bdmf_session_print(session, "decr_count = %u = 0x%x\n", nat0_command_status.decr_count, nat0_command_status.decr_count);
        bdmf_session_print(session, "cache_flush = %u = 0x%x\n", nat0_command_status.cache_flush, nat0_command_status.cache_flush);
        bdmf_session_print(session, "del_cmd_mode = %u = 0x%x\n", nat0_command_status.del_cmd_mode, nat0_command_status.del_cmd_mode);
        bdmf_session_print(session, "add_cmd_mode = %u = 0x%x\n", nat0_command_status.add_cmd_mode, nat0_command_status.add_cmd_mode);
        bdmf_session_print(session, "del_cmd_ddr_bin = %u = 0x%x\n", nat0_command_status.del_cmd_ddr_bin, nat0_command_status.del_cmd_ddr_bin);
        bdmf_session_print(session, "add_cmd_ddr_bin = %u = 0x%x\n", nat0_command_status.add_cmd_ddr_bin, nat0_command_status.add_cmd_ddr_bin);
        bdmf_session_print(session, "add_cmd_ddr_miss = %u = 0x%x\n", nat0_command_status.add_cmd_ddr_miss, nat0_command_status.add_cmd_ddr_miss);
        break;
    }
    case cli_nat_cache_nat0_hash:
    {
        uint32_t hash;
        ag_err = ag_drv_nat_cache_nat0_hash_get(&hash);
        bdmf_session_print(session, "hash = %u = 0x%x\n", hash, hash);
        break;
    }
    case cli_nat_cache_nat0_hit_count:
    {
        uint32_t hit_count;
        ag_err = ag_drv_nat_cache_nat0_hit_count_get(&hit_count);
        bdmf_session_print(session, "hit_count = %u = 0x%x\n", hit_count, hit_count);
        break;
    }
    case cli_nat_cache_nat0_byte_count:
    {
        uint32_t byte_count;
        ag_err = ag_drv_nat_cache_nat0_byte_count_get(&byte_count);
        bdmf_session_print(session, "byte_count = %u = 0x%x\n", byte_count, byte_count);
        break;
    }
    case cli_nat_cache_nat0_pkt_len:
    {
        uint16_t pkt_len;
        uint16_t unused;
        ag_err = ag_drv_nat_cache_nat0_pkt_len_get(&pkt_len, &unused);
        bdmf_session_print(session, "pkt_len = %u = 0x%x\n", pkt_len, pkt_len);
        bdmf_session_print(session, "unused = %u = 0x%x\n", unused, unused);
        break;
    }
    case cli_nat_cache_nat0_key_result:
    {
        nat_cache_nat0_key_result nat0_key_result;
        ag_err = ag_drv_nat_cache_nat0_key_result_get(parm[1].value.unumber, &nat0_key_result);
        bdmf_session_print(session, "data[0] = %u = 0x%x\n", nat0_key_result.data[0], nat0_key_result.data[0]);
        bdmf_session_print(session, "data[1] = %u = 0x%x\n", nat0_key_result.data[1], nat0_key_result.data[1]);
        bdmf_session_print(session, "data[2] = %u = 0x%x\n", nat0_key_result.data[2], nat0_key_result.data[2]);
        bdmf_session_print(session, "data[3] = %u = 0x%x\n", nat0_key_result.data[3], nat0_key_result.data[3]);
        bdmf_session_print(session, "data[4] = %u = 0x%x\n", nat0_key_result.data[4], nat0_key_result.data[4]);
        bdmf_session_print(session, "data[5] = %u = 0x%x\n", nat0_key_result.data[5], nat0_key_result.data[5]);
        bdmf_session_print(session, "data[6] = %u = 0x%x\n", nat0_key_result.data[6], nat0_key_result.data[6]);
        bdmf_session_print(session, "data[7] = %u = 0x%x\n", nat0_key_result.data[7], nat0_key_result.data[7]);
        bdmf_session_print(session, "data[8] = %u = 0x%x\n", nat0_key_result.data[8], nat0_key_result.data[8]);
        bdmf_session_print(session, "data[9] = %u = 0x%x\n", nat0_key_result.data[9], nat0_key_result.data[9]);
        bdmf_session_print(session, "data[10] = %u = 0x%x\n", nat0_key_result.data[10], nat0_key_result.data[10]);
        bdmf_session_print(session, "data[11] = %u = 0x%x\n", nat0_key_result.data[11], nat0_key_result.data[11]);
        bdmf_session_print(session, "data[12] = %u = 0x%x\n", nat0_key_result.data[12], nat0_key_result.data[12]);
        bdmf_session_print(session, "data[13] = %u = 0x%x\n", nat0_key_result.data[13], nat0_key_result.data[13]);
        bdmf_session_print(session, "data[14] = %u = 0x%x\n", nat0_key_result.data[14], nat0_key_result.data[14]);
        bdmf_session_print(session, "data[15] = %u = 0x%x\n", nat0_key_result.data[15], nat0_key_result.data[15]);
        bdmf_session_print(session, "data[16] = %u = 0x%x\n", nat0_key_result.data[16], nat0_key_result.data[16]);
        bdmf_session_print(session, "data[17] = %u = 0x%x\n", nat0_key_result.data[17], nat0_key_result.data[17]);
        break;
    }
    case cli_nat_cache_natc_tbl0_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl0_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl0_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl0_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl1_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl1_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl1_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl1_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl2_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl2_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl2_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl2_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl3_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl3_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl3_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl3_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl4_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl4_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl4_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl4_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl5_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl5_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl5_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl5_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl6_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl6_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl6_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl6_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl7_ddr_key_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl7_ddr_key_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_tbl7_ddr_result_base_address_lower:
    {
        uint8_t zeros;
        uint32_t bar;
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_get(&zeros, &bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        break;
    }
    case cli_nat_cache_natc_tbl7_ddr_result_base_address_upper:
    {
        uint8_t bar;
        uint32_t zeros;
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_get(&bar, &zeros);
        bdmf_session_print(session, "bar = %u = 0x%x\n", bar, bar);
        bdmf_session_print(session, "zeros = %u = 0x%x\n", zeros, zeros);
        break;
    }
    case cli_nat_cache_natc_stat_counter_control_0:
    {
        uint8_t cache_hit_count_en;
        uint8_t cache_miss_count_en;
        uint8_t ddr_request_count_en;
        uint8_t ddr_evict_count_en;
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_0_get(&cache_hit_count_en, &cache_miss_count_en, &ddr_request_count_en, &ddr_evict_count_en);
        bdmf_session_print(session, "cache_hit_count_en = %u = 0x%x\n", cache_hit_count_en, cache_hit_count_en);
        bdmf_session_print(session, "cache_miss_count_en = %u = 0x%x\n", cache_miss_count_en, cache_miss_count_en);
        bdmf_session_print(session, "ddr_request_count_en = %u = 0x%x\n", ddr_request_count_en, ddr_request_count_en);
        bdmf_session_print(session, "ddr_evict_count_en = %u = 0x%x\n", ddr_evict_count_en, ddr_evict_count_en);
        break;
    }
    case cli_nat_cache_natc_stat_counter_control_1:
    {
        uint8_t ddr_block_count_en;
        bdmf_boolean counter_wraparound_dis;
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_1_get(&ddr_block_count_en, &counter_wraparound_dis);
        bdmf_session_print(session, "ddr_block_count_en = %u = 0x%x\n", ddr_block_count_en, ddr_block_count_en);
        bdmf_session_print(session, "counter_wraparound_dis = %u = 0x%x\n", counter_wraparound_dis, counter_wraparound_dis);
        break;
    }
    case cli_nat_cache_natc_ddr_size:
    {
        nat_cache_natc_ddr_size natc_ddr_size;
        ag_err = ag_drv_nat_cache_natc_ddr_size_get(&natc_ddr_size);
        bdmf_session_print(session, "ddr_size_tbl0 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl0);
        bdmf_session_print(session, "ddr_size_tbl1 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl1);
        bdmf_session_print(session, "ddr_size_tbl2 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl2);
        bdmf_session_print(session, "ddr_size_tbl3 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl3, natc_ddr_size.ddr_size_tbl3);
        bdmf_session_print(session, "ddr_size_tbl4 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl4);
        bdmf_session_print(session, "ddr_size_tbl5 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl5);
        bdmf_session_print(session, "ddr_size_tbl6 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl6);
        bdmf_session_print(session, "ddr_size_tbl7 = %u = 0x%x\n", natc_ddr_size.ddr_size_tbl7, natc_ddr_size.ddr_size_tbl7);
        break;
    }
    case cli_nat_cache_natc_ddr_bins_per_bucket_0:
    {
        uint8_t ddr_bins_per_bucket_tbl0;
        uint8_t ddr_bins_per_bucket_tbl1;
        uint8_t ddr_bins_per_bucket_tbl2;
        uint8_t ddr_bins_per_bucket_tbl3;
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_get(&ddr_bins_per_bucket_tbl0, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl3);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl0 = %u = 0x%x\n", ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl0);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl1 = %u = 0x%x\n", ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl1);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl2 = %u = 0x%x\n", ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl2);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl3 = %u = 0x%x\n", ddr_bins_per_bucket_tbl3, ddr_bins_per_bucket_tbl3);
        break;
    }
    case cli_nat_cache_natc_ddr_bins_per_bucket_1:
    {
        uint8_t ddr_bins_per_bucket_tbl4;
        uint8_t ddr_bins_per_bucket_tbl5;
        uint8_t ddr_bins_per_bucket_tbl6;
        uint8_t ddr_bins_per_bucket_tbl7;
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_get(&ddr_bins_per_bucket_tbl4, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl7);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl4 = %u = 0x%x\n", ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl4);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl5 = %u = 0x%x\n", ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl5);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl6 = %u = 0x%x\n", ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl6);
        bdmf_session_print(session, "ddr_bins_per_bucket_tbl7 = %u = 0x%x\n", ddr_bins_per_bucket_tbl7, ddr_bins_per_bucket_tbl7);
        break;
    }
    case cli_nat_cache_natc_total_len:
    {
        nat_cache_natc_total_len natc_total_len;
        ag_err = ag_drv_nat_cache_natc_total_len_get(&natc_total_len);
        bdmf_session_print(session, "total_len_tbl0 = %u = 0x%x\n", natc_total_len.total_len_tbl0, natc_total_len.total_len_tbl0);
        bdmf_session_print(session, "total_len_tbl1 = %u = 0x%x\n", natc_total_len.total_len_tbl1, natc_total_len.total_len_tbl1);
        bdmf_session_print(session, "total_len_tbl2 = %u = 0x%x\n", natc_total_len.total_len_tbl2, natc_total_len.total_len_tbl2);
        bdmf_session_print(session, "total_len_tbl3 = %u = 0x%x\n", natc_total_len.total_len_tbl3, natc_total_len.total_len_tbl3);
        bdmf_session_print(session, "total_len_tbl4 = %u = 0x%x\n", natc_total_len.total_len_tbl4, natc_total_len.total_len_tbl4);
        bdmf_session_print(session, "total_len_tbl5 = %u = 0x%x\n", natc_total_len.total_len_tbl5, natc_total_len.total_len_tbl5);
        bdmf_session_print(session, "total_len_tbl6 = %u = 0x%x\n", natc_total_len.total_len_tbl6, natc_total_len.total_len_tbl6);
        bdmf_session_print(session, "total_len_tbl7 = %u = 0x%x\n", natc_total_len.total_len_tbl7, natc_total_len.total_len_tbl7);
        break;
    }
    case cli_nat_cache_natc_sm_status:
    {
        nat_cache_natc_sm_status natc_sm_status;
        ag_err = ag_drv_nat_cache_natc_sm_status_get(&natc_sm_status);
        bdmf_session_print(session, "nat_state = %u = 0x%x\n", natc_sm_status.nat_state, natc_sm_status.nat_state);
        bdmf_session_print(session, "wb_state = %u = 0x%x\n", natc_sm_status.wb_state, natc_sm_status.wb_state);
        bdmf_session_print(session, "runner_cmd_state = %u = 0x%x\n", natc_sm_status.runner_cmd_state, natc_sm_status.runner_cmd_state);
        bdmf_session_print(session, "ddr_rep_state = %u = 0x%x\n", natc_sm_status.ddr_rep_state, natc_sm_status.ddr_rep_state);
        bdmf_session_print(session, "ddr_req_state = %u = 0x%x\n", natc_sm_status.ddr_req_state, natc_sm_status.ddr_req_state);
        bdmf_session_print(session, "apb_state = %u = 0x%x\n", natc_sm_status.apb_state, natc_sm_status.apb_state);
        bdmf_session_print(session, "debug_sel = %u = 0x%x\n", natc_sm_status.debug_sel, natc_sm_status.debug_sel);
        break;
    }
    case cli_nat_cache_natc_regfile_fifo_start_addr_0:
    {
        uint8_t regfile_fifo_start_addr_0;
        uint8_t regfile_fifo_start_addr_1;
        uint8_t regfile_fifo_start_addr_2;
        uint8_t regfile_fifo_start_addr_3;
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_get(&regfile_fifo_start_addr_0, &regfile_fifo_start_addr_1, &regfile_fifo_start_addr_2, &regfile_fifo_start_addr_3);
        bdmf_session_print(session, "regfile_fifo_start_addr_0 = %u = 0x%x\n", regfile_fifo_start_addr_0, regfile_fifo_start_addr_0);
        bdmf_session_print(session, "regfile_fifo_start_addr_1 = %u = 0x%x\n", regfile_fifo_start_addr_1, regfile_fifo_start_addr_1);
        bdmf_session_print(session, "regfile_fifo_start_addr_2 = %u = 0x%x\n", regfile_fifo_start_addr_2, regfile_fifo_start_addr_2);
        bdmf_session_print(session, "regfile_fifo_start_addr_3 = %u = 0x%x\n", regfile_fifo_start_addr_3, regfile_fifo_start_addr_3);
        break;
    }
    case cli_nat_cache_natc_regfile_fifo_start_addr_1:
    {
        uint8_t regfile_fifo_start_addr_4;
        uint8_t regfile_fifo_start_addr_5;
        uint8_t regfile_fifo_start_addr_6;
        uint8_t regfile_fifo_start_addr_7;
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_get(&regfile_fifo_start_addr_4, &regfile_fifo_start_addr_5, &regfile_fifo_start_addr_6, &regfile_fifo_start_addr_7);
        bdmf_session_print(session, "regfile_fifo_start_addr_4 = %u = 0x%x\n", regfile_fifo_start_addr_4, regfile_fifo_start_addr_4);
        bdmf_session_print(session, "regfile_fifo_start_addr_5 = %u = 0x%x\n", regfile_fifo_start_addr_5, regfile_fifo_start_addr_5);
        bdmf_session_print(session, "regfile_fifo_start_addr_6 = %u = 0x%x\n", regfile_fifo_start_addr_6, regfile_fifo_start_addr_6);
        bdmf_session_print(session, "regfile_fifo_start_addr_7 = %u = 0x%x\n", regfile_fifo_start_addr_7, regfile_fifo_start_addr_7);
        break;
    }
    case cli_nat_cache_natc_tbl0_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl1_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl2_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl3_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl4_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl5_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl6_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_tbl7_key_mask:
    {
        uint32_t key_mask;
        ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_get(parm[1].value.unumber, &key_mask);
        bdmf_session_print(session, "key_mask = %u = 0x%x\n", key_mask, key_mask);
        break;
    }
    case cli_nat_cache_natc_ddr_hash_mode:
    {
        nat_cache_natc_ddr_hash_mode natc_ddr_hash_mode;
        ag_err = ag_drv_nat_cache_natc_ddr_hash_mode_get(&natc_ddr_hash_mode);
        bdmf_session_print(session, "ddr_hash_mode_tbl0 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl0, natc_ddr_hash_mode.ddr_hash_mode_tbl0);
        bdmf_session_print(session, "ddr_hash_mode_tbl1 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl1, natc_ddr_hash_mode.ddr_hash_mode_tbl1);
        bdmf_session_print(session, "ddr_hash_mode_tbl2 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl2, natc_ddr_hash_mode.ddr_hash_mode_tbl2);
        bdmf_session_print(session, "ddr_hash_mode_tbl3 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl3, natc_ddr_hash_mode.ddr_hash_mode_tbl3);
        bdmf_session_print(session, "ddr_hash_mode_tbl4 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl4, natc_ddr_hash_mode.ddr_hash_mode_tbl4);
        bdmf_session_print(session, "ddr_hash_mode_tbl5 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl5, natc_ddr_hash_mode.ddr_hash_mode_tbl5);
        bdmf_session_print(session, "ddr_hash_mode_tbl6 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl6, natc_ddr_hash_mode.ddr_hash_mode_tbl6);
        bdmf_session_print(session, "ddr_hash_mode_tbl7 = %u = 0x%x\n", natc_ddr_hash_mode.ddr_hash_mode_tbl7, natc_ddr_hash_mode.ddr_hash_mode_tbl7);
        break;
    }
    case cli_nat_cache_natc_flow_cntr_cntl:
    {
        nat_cache_natc_flow_cntr_cntl natc_flow_cntr_cntl;
        ag_err = ag_drv_nat_cache_natc_flow_cntr_cntl_get(&natc_flow_cntr_cntl);
        bdmf_session_print(session, "context_offset = %u = 0x%x\n", natc_flow_cntr_cntl.context_offset, natc_flow_cntr_cntl.context_offset);
        bdmf_session_print(session, "flow_cntr_en_tbl0 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl0, natc_flow_cntr_cntl.flow_cntr_en_tbl0);
        bdmf_session_print(session, "flow_cntr_en_tbl1 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl1, natc_flow_cntr_cntl.flow_cntr_en_tbl1);
        bdmf_session_print(session, "flow_cntr_en_tbl2 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl2, natc_flow_cntr_cntl.flow_cntr_en_tbl2);
        bdmf_session_print(session, "flow_cntr_en_tbl3 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl3, natc_flow_cntr_cntl.flow_cntr_en_tbl3);
        bdmf_session_print(session, "flow_cntr_en_tbl4 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl4, natc_flow_cntr_cntl.flow_cntr_en_tbl4);
        bdmf_session_print(session, "flow_cntr_en_tbl5 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl5, natc_flow_cntr_cntl.flow_cntr_en_tbl5);
        bdmf_session_print(session, "flow_cntr_en_tbl6 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl6, natc_flow_cntr_cntl.flow_cntr_en_tbl6);
        bdmf_session_print(session, "flow_cntr_en_tbl7 = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_en_tbl7, natc_flow_cntr_cntl.flow_cntr_en_tbl7);
        bdmf_session_print(session, "flow_cntr_clr_on_rd_en = %u = 0x%x\n", natc_flow_cntr_cntl.flow_cntr_clr_on_rd_en, natc_flow_cntr_cntl.flow_cntr_clr_on_rd_en);
        break;
    }
    case cli_nat_cache_indir_rw_natc_indir_addr_reg:
    {
        uint16_t natc_entry;
        bdmf_boolean w_r;
        ag_err = ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_get(&natc_entry, &w_r);
        bdmf_session_print(session, "natc_entry = %u = 0x%x\n", natc_entry, natc_entry);
        bdmf_session_print(session, "w_r = %u = 0x%x\n", w_r, w_r);
        break;
    }
    case cli_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg:
    {
        uint8_t flow_cntr_entry;
        bdmf_boolean w_r;
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_get(&flow_cntr_entry, &w_r);
        bdmf_session_print(session, "flow_cntr_entry = %u = 0x%x\n", flow_cntr_entry, flow_cntr_entry);
        bdmf_session_print(session, "w_r = %u = 0x%x\n", w_r, w_r);
        break;
    }
    case cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg:
    {
        uint32_t data;
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_nat_cache_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        nat_cache_ctrl_status ctrl_status = {.natc_reset = gtmv(m, 1), .natc_enable = gtmv(m, 1), .regfile_fifo_reset = gtmv(m, 1), .natc_smem_clear_by_update_disable = gtmv(m, 1), .natc_smem_increment_on_reg_lookup = gtmv(m, 1), .nat_arb_st = gtmv(m, 2), .decr_count_wraparound_enable = gtmv(m, 1), .multi_hash_limit = gtmv(m, 4), .nat_hash_mode = gtmv(m, 3), .ddr_disable_on_reg_lookup = gtmv(m, 1), .cache_update_on_ddr_miss = gtmv(m, 1), .pending_fifo_entry_check_enable = gtmv(m, 1), .unused1 = gtmv(m, 3), .reg_8bit_in_32bit_swap_control = gtmv(m, 1), .reg_32bit_in_64bit_swap_control = gtmv(m, 1), .repeated_key_det_en = gtmv(m, 1), .ddr_swap_all_control = gtmv(m, 1), .smem_8bit_in_32bit_swap_control = gtmv(m, 1), .smem_32bit_in_64bit_swap_control = gtmv(m, 1), .ddr_64bit_in_128bit_swap_control = gtmv(m, 1), .natc_init_done = gtmv(m, 1), .unused0 = gtmv(m, 1), .natc_add_command_speedup_mode = gtmv(m, 1), .ddr_enable = gtmv(m, 1), .unused2 = gtmv(m, 1), .ddr_lookup_pending_fifo_mode_disable = gtmv(m, 1), .ddr_replace_duplicated_cached_entry_enable = gtmv(m, 1), .ddr_hash_swap = gtmv(m, 1), .ddr_counter_8bit_in_32bit_swap_control = gtmv(m, 1), .cache_update_on_reg_ddr_lookup = gtmv(m, 1), .unused3 = gtmv(m, 2), .unused4 = gtmv(m, 8), .cache_algo = gtmv(m, 4), .age_timer = gtmv(m, 5), .age_timer_tick = gtmv(m, 1), .cache_lookup_blocking_mode = gtmv(m, 1), .ddr_8bit_in_32bit_swap_control = gtmv(m, 1), .ddr_32bit_in_64bit_swap_control = gtmv(m, 1), .unused5 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_nat_cache_ctrl_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            ctrl_status.natc_reset, ctrl_status.natc_enable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_smem_clear_by_update_disable, 
            ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.nat_arb_st, ctrl_status.decr_count_wraparound_enable, ctrl_status.multi_hash_limit, 
            ctrl_status.nat_hash_mode, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.cache_update_on_ddr_miss, ctrl_status.pending_fifo_entry_check_enable, 
            ctrl_status.unused1, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.repeated_key_det_en, 
            ctrl_status.ddr_swap_all_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.ddr_64bit_in_128bit_swap_control, 
            ctrl_status.natc_init_done, ctrl_status.unused0, ctrl_status.natc_add_command_speedup_mode, ctrl_status.ddr_enable, 
            ctrl_status.unused2, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_hash_swap, 
            ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.unused3, ctrl_status.unused4, 
            ctrl_status.cache_algo, ctrl_status.age_timer, ctrl_status.age_timer_tick, ctrl_status.cache_lookup_blocking_mode, 
            ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.unused5);
        ag_err = ag_drv_nat_cache_ctrl_status_set(&ctrl_status);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_ctrl_status_get(&ctrl_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_ctrl_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                ctrl_status.natc_reset, ctrl_status.natc_enable, ctrl_status.regfile_fifo_reset, ctrl_status.natc_smem_clear_by_update_disable, 
                ctrl_status.natc_smem_increment_on_reg_lookup, ctrl_status.nat_arb_st, ctrl_status.decr_count_wraparound_enable, ctrl_status.multi_hash_limit, 
                ctrl_status.nat_hash_mode, ctrl_status.ddr_disable_on_reg_lookup, ctrl_status.cache_update_on_ddr_miss, ctrl_status.pending_fifo_entry_check_enable, 
                ctrl_status.unused1, ctrl_status.reg_8bit_in_32bit_swap_control, ctrl_status.reg_32bit_in_64bit_swap_control, ctrl_status.repeated_key_det_en, 
                ctrl_status.ddr_swap_all_control, ctrl_status.smem_8bit_in_32bit_swap_control, ctrl_status.smem_32bit_in_64bit_swap_control, ctrl_status.ddr_64bit_in_128bit_swap_control, 
                ctrl_status.natc_init_done, ctrl_status.unused0, ctrl_status.natc_add_command_speedup_mode, ctrl_status.ddr_enable, 
                ctrl_status.unused2, ctrl_status.ddr_lookup_pending_fifo_mode_disable, ctrl_status.ddr_replace_duplicated_cached_entry_enable, ctrl_status.ddr_hash_swap, 
                ctrl_status.ddr_counter_8bit_in_32bit_swap_control, ctrl_status.cache_update_on_reg_ddr_lookup, ctrl_status.unused3, ctrl_status.unused4, 
                ctrl_status.cache_algo, ctrl_status.age_timer, ctrl_status.age_timer_tick, ctrl_status.cache_lookup_blocking_mode, 
                ctrl_status.ddr_8bit_in_32bit_swap_control, ctrl_status.ddr_32bit_in_64bit_swap_control, ctrl_status.unused5);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ctrl_status.natc_reset != gtmv(m, 1) || ctrl_status.natc_enable != gtmv(m, 1) || ctrl_status.regfile_fifo_reset != gtmv(m, 1) || ctrl_status.natc_smem_clear_by_update_disable != gtmv(m, 1) || ctrl_status.natc_smem_increment_on_reg_lookup != gtmv(m, 1) || ctrl_status.nat_arb_st != gtmv(m, 2) || ctrl_status.decr_count_wraparound_enable != gtmv(m, 1) || ctrl_status.multi_hash_limit != gtmv(m, 4) || ctrl_status.nat_hash_mode != gtmv(m, 3) || ctrl_status.ddr_disable_on_reg_lookup != gtmv(m, 1) || ctrl_status.cache_update_on_ddr_miss != gtmv(m, 1) || ctrl_status.pending_fifo_entry_check_enable != gtmv(m, 1) || ctrl_status.unused1 != gtmv(m, 3) || ctrl_status.reg_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.reg_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.repeated_key_det_en != gtmv(m, 1) || ctrl_status.ddr_swap_all_control != gtmv(m, 1) || ctrl_status.smem_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.smem_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_64bit_in_128bit_swap_control != gtmv(m, 1) || ctrl_status.natc_init_done != gtmv(m, 1) || ctrl_status.unused0 != gtmv(m, 1) || ctrl_status.natc_add_command_speedup_mode != gtmv(m, 1) || ctrl_status.ddr_enable != gtmv(m, 1) || ctrl_status.unused2 != gtmv(m, 1) || ctrl_status.ddr_lookup_pending_fifo_mode_disable != gtmv(m, 1) || ctrl_status.ddr_replace_duplicated_cached_entry_enable != gtmv(m, 1) || ctrl_status.ddr_hash_swap != gtmv(m, 1) || ctrl_status.ddr_counter_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.cache_update_on_reg_ddr_lookup != gtmv(m, 1) || ctrl_status.unused3 != gtmv(m, 2) || ctrl_status.unused4 != gtmv(m, 8) || ctrl_status.cache_algo != gtmv(m, 4) || ctrl_status.age_timer != gtmv(m, 5) || ctrl_status.age_timer_tick != gtmv(m, 1) || ctrl_status.cache_lookup_blocking_mode != gtmv(m, 1) || ctrl_status.ddr_8bit_in_32bit_swap_control != gtmv(m, 1) || ctrl_status.ddr_32bit_in_64bit_swap_control != gtmv(m, 1) || ctrl_status.unused5 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_ctrs natc_ctrs = {.cache_hit_count = gtmv(m, 32), .cache_miss_count = gtmv(m, 32), .ddr_request_count = gtmv(m, 32), .ddr_evict_count = gtmv(m, 32), .ddr_block_count = gtmv(m, 32)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_ctrs_set( %u %u %u %u %u)\n",
            natc_ctrs.cache_hit_count, natc_ctrs.cache_miss_count, natc_ctrs.ddr_request_count, natc_ctrs.ddr_evict_count, 
            natc_ctrs.ddr_block_count);
        ag_err = ag_drv_nat_cache_natc_ctrs_set(&natc_ctrs);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_ctrs_get(&natc_ctrs);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_ctrs_get( %u %u %u %u %u)\n",
                natc_ctrs.cache_hit_count, natc_ctrs.cache_miss_count, natc_ctrs.ddr_request_count, natc_ctrs.ddr_evict_count, 
                natc_ctrs.ddr_block_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_ctrs.cache_hit_count != gtmv(m, 32) || natc_ctrs.cache_miss_count != gtmv(m, 32) || natc_ctrs.ddr_request_count != gtmv(m, 32) || natc_ctrs.ddr_evict_count != gtmv(m, 32) || natc_ctrs.ddr_block_count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_table_control natc_table_control = {.non_cacheable_tbl0 = gtmv(m, 1), .key_len_tbl0 = gtmv(m, 1), .non_cacheable_tbl1 = gtmv(m, 1), .key_len_tbl1 = gtmv(m, 1), .non_cacheable_tbl2 = gtmv(m, 1), .key_len_tbl2 = gtmv(m, 1), .non_cacheable_tbl3 = gtmv(m, 1), .key_len_tbl3 = gtmv(m, 1), .non_cacheable_tbl4 = gtmv(m, 1), .key_len_tbl4 = gtmv(m, 1), .non_cacheable_tbl5 = gtmv(m, 1), .key_len_tbl5 = gtmv(m, 1), .non_cacheable_tbl6 = gtmv(m, 1), .key_len_tbl6 = gtmv(m, 1), .non_cacheable_tbl7 = gtmv(m, 1), .key_len_tbl7 = gtmv(m, 1), .var_context_len_en_tbl0 = gtmv(m, 1), .var_context_len_en_tbl1 = gtmv(m, 1), .var_context_len_en_tbl2 = gtmv(m, 1), .var_context_len_en_tbl3 = gtmv(m, 1), .var_context_len_en_tbl4 = gtmv(m, 1), .var_context_len_en_tbl5 = gtmv(m, 1), .var_context_len_en_tbl6 = gtmv(m, 1), .var_context_len_en_tbl7 = gtmv(m, 1), .smem_dis_tbl0 = gtmv(m, 1), .smem_dis_tbl1 = gtmv(m, 1), .smem_dis_tbl2 = gtmv(m, 1), .smem_dis_tbl3 = gtmv(m, 1), .smem_dis_tbl4 = gtmv(m, 1), .smem_dis_tbl5 = gtmv(m, 1), .smem_dis_tbl6 = gtmv(m, 1), .smem_dis_tbl7 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_table_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            natc_table_control.non_cacheable_tbl0, natc_table_control.key_len_tbl0, natc_table_control.non_cacheable_tbl1, natc_table_control.key_len_tbl1, 
            natc_table_control.non_cacheable_tbl2, natc_table_control.key_len_tbl2, natc_table_control.non_cacheable_tbl3, natc_table_control.key_len_tbl3, 
            natc_table_control.non_cacheable_tbl4, natc_table_control.key_len_tbl4, natc_table_control.non_cacheable_tbl5, natc_table_control.key_len_tbl5, 
            natc_table_control.non_cacheable_tbl6, natc_table_control.key_len_tbl6, natc_table_control.non_cacheable_tbl7, natc_table_control.key_len_tbl7, 
            natc_table_control.var_context_len_en_tbl0, natc_table_control.var_context_len_en_tbl1, natc_table_control.var_context_len_en_tbl2, natc_table_control.var_context_len_en_tbl3, 
            natc_table_control.var_context_len_en_tbl4, natc_table_control.var_context_len_en_tbl5, natc_table_control.var_context_len_en_tbl6, natc_table_control.var_context_len_en_tbl7, 
            natc_table_control.smem_dis_tbl0, natc_table_control.smem_dis_tbl1, natc_table_control.smem_dis_tbl2, natc_table_control.smem_dis_tbl3, 
            natc_table_control.smem_dis_tbl4, natc_table_control.smem_dis_tbl5, natc_table_control.smem_dis_tbl6, natc_table_control.smem_dis_tbl7);
        ag_err = ag_drv_nat_cache_natc_table_control_set(&natc_table_control);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_table_control_get(&natc_table_control);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_table_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                natc_table_control.non_cacheable_tbl0, natc_table_control.key_len_tbl0, natc_table_control.non_cacheable_tbl1, natc_table_control.key_len_tbl1, 
                natc_table_control.non_cacheable_tbl2, natc_table_control.key_len_tbl2, natc_table_control.non_cacheable_tbl3, natc_table_control.key_len_tbl3, 
                natc_table_control.non_cacheable_tbl4, natc_table_control.key_len_tbl4, natc_table_control.non_cacheable_tbl5, natc_table_control.key_len_tbl5, 
                natc_table_control.non_cacheable_tbl6, natc_table_control.key_len_tbl6, natc_table_control.non_cacheable_tbl7, natc_table_control.key_len_tbl7, 
                natc_table_control.var_context_len_en_tbl0, natc_table_control.var_context_len_en_tbl1, natc_table_control.var_context_len_en_tbl2, natc_table_control.var_context_len_en_tbl3, 
                natc_table_control.var_context_len_en_tbl4, natc_table_control.var_context_len_en_tbl5, natc_table_control.var_context_len_en_tbl6, natc_table_control.var_context_len_en_tbl7, 
                natc_table_control.smem_dis_tbl0, natc_table_control.smem_dis_tbl1, natc_table_control.smem_dis_tbl2, natc_table_control.smem_dis_tbl3, 
                natc_table_control.smem_dis_tbl4, natc_table_control.smem_dis_tbl5, natc_table_control.smem_dis_tbl6, natc_table_control.smem_dis_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_table_control.non_cacheable_tbl0 != gtmv(m, 1) || natc_table_control.key_len_tbl0 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl1 != gtmv(m, 1) || natc_table_control.key_len_tbl1 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl2 != gtmv(m, 1) || natc_table_control.key_len_tbl2 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl3 != gtmv(m, 1) || natc_table_control.key_len_tbl3 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl4 != gtmv(m, 1) || natc_table_control.key_len_tbl4 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl5 != gtmv(m, 1) || natc_table_control.key_len_tbl5 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl6 != gtmv(m, 1) || natc_table_control.key_len_tbl6 != gtmv(m, 1) || natc_table_control.non_cacheable_tbl7 != gtmv(m, 1) || natc_table_control.key_len_tbl7 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl0 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl1 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl2 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl3 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl4 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl5 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl6 != gtmv(m, 1) || natc_table_control.var_context_len_en_tbl7 != gtmv(m, 1) || natc_table_control.smem_dis_tbl0 != gtmv(m, 1) || natc_table_control.smem_dis_tbl1 != gtmv(m, 1) || natc_table_control.smem_dis_tbl2 != gtmv(m, 1) || natc_table_control.smem_dis_tbl3 != gtmv(m, 1) || natc_table_control.smem_dis_tbl4 != gtmv(m, 1) || natc_table_control.smem_dis_tbl5 != gtmv(m, 1) || natc_table_control.smem_dis_tbl6 != gtmv(m, 1) || natc_table_control.smem_dis_tbl7 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_nat0_command_status nat0_command_status = {.command = gtmv(m, 3), .unused0 = gtmv(m, 1), .busy = gtmv(m, 1), .error = gtmv(m, 1), .miss = gtmv(m, 1), .cache_hit = gtmv(m, 1), .multihash_count = gtmv(m, 4), .nat_tbl = gtmv(m, 3), .decr_count = gtmv(m, 1), .cache_flush = gtmv(m, 1), .del_cmd_mode = gtmv(m, 1), .add_cmd_mode = gtmv(m, 1), .del_cmd_ddr_bin = gtmv(m, 8), .add_cmd_ddr_bin = gtmv(m, 3), .add_cmd_ddr_miss = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_nat_cache_nat0_command_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            nat0_command_status.command, nat0_command_status.unused0, nat0_command_status.busy, nat0_command_status.error, 
            nat0_command_status.miss, nat0_command_status.cache_hit, nat0_command_status.multihash_count, nat0_command_status.nat_tbl, 
            nat0_command_status.decr_count, nat0_command_status.cache_flush, nat0_command_status.del_cmd_mode, nat0_command_status.add_cmd_mode, 
            nat0_command_status.del_cmd_ddr_bin, nat0_command_status.add_cmd_ddr_bin, nat0_command_status.add_cmd_ddr_miss);
        ag_err = ag_drv_nat_cache_nat0_command_status_set(&nat0_command_status);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_command_status_get(&nat0_command_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_command_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                nat0_command_status.command, nat0_command_status.unused0, nat0_command_status.busy, nat0_command_status.error, 
                nat0_command_status.miss, nat0_command_status.cache_hit, nat0_command_status.multihash_count, nat0_command_status.nat_tbl, 
                nat0_command_status.decr_count, nat0_command_status.cache_flush, nat0_command_status.del_cmd_mode, nat0_command_status.add_cmd_mode, 
                nat0_command_status.del_cmd_ddr_bin, nat0_command_status.add_cmd_ddr_bin, nat0_command_status.add_cmd_ddr_miss);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (nat0_command_status.command != gtmv(m, 3) || nat0_command_status.unused0 != gtmv(m, 1) || nat0_command_status.busy != gtmv(m, 1) || nat0_command_status.error != gtmv(m, 1) || nat0_command_status.miss != gtmv(m, 1) || nat0_command_status.cache_hit != gtmv(m, 1) || nat0_command_status.multihash_count != gtmv(m, 4) || nat0_command_status.nat_tbl != gtmv(m, 3) || nat0_command_status.decr_count != gtmv(m, 1) || nat0_command_status.cache_flush != gtmv(m, 1) || nat0_command_status.del_cmd_mode != gtmv(m, 1) || nat0_command_status.add_cmd_mode != gtmv(m, 1) || nat0_command_status.del_cmd_ddr_bin != gtmv(m, 8) || nat0_command_status.add_cmd_ddr_bin != gtmv(m, 3) || nat0_command_status.add_cmd_ddr_miss != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t hash = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_hash_get(&hash);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_hash_get( %u)\n",
                hash);
        }
    }

    {
        uint32_t hit_count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_hit_count_get(&hit_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_hit_count_get( %u)\n",
                hit_count);
        }
    }

    {
        uint32_t byte_count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_byte_count_get(&byte_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_byte_count_get( %u)\n",
                byte_count);
        }
    }

    {
        uint16_t pkt_len = gtmv(m, 16);
        uint16_t unused = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_nat_cache_nat0_pkt_len_set( %u %u)\n",
            pkt_len, unused);
        ag_err = ag_drv_nat_cache_nat0_pkt_len_set(pkt_len, unused);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_pkt_len_get(&pkt_len, &unused);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_pkt_len_get( %u %u)\n",
                pkt_len, unused);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pkt_len != gtmv(m, 16) || unused != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zero = gtmv(m, 1);
        nat_cache_nat0_key_result nat0_key_result = {.data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_nat_cache_nat0_key_result_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero,
            nat0_key_result.data[0], nat0_key_result.data[1], nat0_key_result.data[2], nat0_key_result.data[3], 
            nat0_key_result.data[4], nat0_key_result.data[5], nat0_key_result.data[6], nat0_key_result.data[7], 
            nat0_key_result.data[8], nat0_key_result.data[9], nat0_key_result.data[10], nat0_key_result.data[11], 
            nat0_key_result.data[12], nat0_key_result.data[13], nat0_key_result.data[14], nat0_key_result.data[15], 
            nat0_key_result.data[16], nat0_key_result.data[17]);
        ag_err = ag_drv_nat_cache_nat0_key_result_set(zero, &nat0_key_result);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_nat0_key_result_get(zero, &nat0_key_result);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_key_result_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero,
                nat0_key_result.data[0], nat0_key_result.data[1], nat0_key_result.data[2], nat0_key_result.data[3], 
                nat0_key_result.data[4], nat0_key_result.data[5], nat0_key_result.data[6], nat0_key_result.data[7], 
                nat0_key_result.data[8], nat0_key_result.data[9], nat0_key_result.data[10], nat0_key_result.data[11], 
                nat0_key_result.data[12], nat0_key_result.data[13], nat0_key_result.data[14], nat0_key_result.data[15], 
                nat0_key_result.data[16], nat0_key_result.data[17]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (nat0_key_result.data[0] != gtmv(m, 32) || nat0_key_result.data[1] != gtmv(m, 32) || nat0_key_result.data[2] != gtmv(m, 32) || nat0_key_result.data[3] != gtmv(m, 32) || nat0_key_result.data[4] != gtmv(m, 32) || nat0_key_result.data[5] != gtmv(m, 32) || nat0_key_result.data[6] != gtmv(m, 32) || nat0_key_result.data[7] != gtmv(m, 32) || nat0_key_result.data[8] != gtmv(m, 32) || nat0_key_result.data[9] != gtmv(m, 32) || nat0_key_result.data[10] != gtmv(m, 32) || nat0_key_result.data[11] != gtmv(m, 32) || nat0_key_result.data[12] != gtmv(m, 32) || nat0_key_result.data[13] != gtmv(m, 32) || nat0_key_result.data[14] != gtmv(m, 32) || nat0_key_result.data[15] != gtmv(m, 32) || nat0_key_result.data[16] != gtmv(m, 32) || nat0_key_result.data[17] != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zeros = gtmv(m, 3);
        uint32_t bar = gtmv(m, 29);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set( %u %u)\n",
            zeros, bar);
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set(zeros, bar);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_get(&zeros, &bar);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_get( %u %u)\n",
                zeros, bar);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (zeros != gtmv(m, 3) || bar != gtmv(m, 29))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bar = gtmv(m, 8);
        uint32_t zeros = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set( %u %u)\n",
            bar, zeros);
        ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set(bar, zeros);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_get(&bar, &zeros);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_get( %u %u)\n",
                bar, zeros);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bar != gtmv(m, 8) || zeros != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t cache_hit_count_en = gtmv(m, 8);
        uint8_t cache_miss_count_en = gtmv(m, 8);
        uint8_t ddr_request_count_en = gtmv(m, 8);
        uint8_t ddr_evict_count_en = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_stat_counter_control_0_set( %u %u %u %u)\n",
            cache_hit_count_en, cache_miss_count_en, ddr_request_count_en, ddr_evict_count_en);
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_0_set(cache_hit_count_en, cache_miss_count_en, ddr_request_count_en, ddr_evict_count_en);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_stat_counter_control_0_get(&cache_hit_count_en, &cache_miss_count_en, &ddr_request_count_en, &ddr_evict_count_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_stat_counter_control_0_get( %u %u %u %u)\n",
                cache_hit_count_en, cache_miss_count_en, ddr_request_count_en, ddr_evict_count_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cache_hit_count_en != gtmv(m, 8) || cache_miss_count_en != gtmv(m, 8) || ddr_request_count_en != gtmv(m, 8) || ddr_evict_count_en != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_block_count_en = gtmv(m, 8);
        bdmf_boolean counter_wraparound_dis = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_stat_counter_control_1_set( %u %u)\n",
            ddr_block_count_en, counter_wraparound_dis);
        ag_err = ag_drv_nat_cache_natc_stat_counter_control_1_set(ddr_block_count_en, counter_wraparound_dis);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_stat_counter_control_1_get(&ddr_block_count_en, &counter_wraparound_dis);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_stat_counter_control_1_get( %u %u)\n",
                ddr_block_count_en, counter_wraparound_dis);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_block_count_en != gtmv(m, 8) || counter_wraparound_dis != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_ddr_size natc_ddr_size = {.ddr_size_tbl0 = gtmv(m, 3), .ddr_size_tbl1 = gtmv(m, 3), .ddr_size_tbl2 = gtmv(m, 3), .ddr_size_tbl3 = gtmv(m, 3), .ddr_size_tbl4 = gtmv(m, 3), .ddr_size_tbl5 = gtmv(m, 3), .ddr_size_tbl6 = gtmv(m, 3), .ddr_size_tbl7 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_size_set( %u %u %u %u %u %u %u %u)\n",
            natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, 
            natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        ag_err = ag_drv_nat_cache_natc_ddr_size_set(&natc_ddr_size);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_ddr_size_get(&natc_ddr_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_size_get( %u %u %u %u %u %u %u %u)\n",
                natc_ddr_size.ddr_size_tbl0, natc_ddr_size.ddr_size_tbl1, natc_ddr_size.ddr_size_tbl2, natc_ddr_size.ddr_size_tbl3, 
                natc_ddr_size.ddr_size_tbl4, natc_ddr_size.ddr_size_tbl5, natc_ddr_size.ddr_size_tbl6, natc_ddr_size.ddr_size_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_ddr_size.ddr_size_tbl0 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl1 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl2 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl3 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl4 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl5 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl6 != gtmv(m, 3) || natc_ddr_size.ddr_size_tbl7 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_bins_per_bucket_tbl0 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl1 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl2 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl3 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_set( %u %u %u %u)\n",
            ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_set(ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_get(&ddr_bins_per_bucket_tbl0, &ddr_bins_per_bucket_tbl1, &ddr_bins_per_bucket_tbl2, &ddr_bins_per_bucket_tbl3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_bins_per_bucket_0_get( %u %u %u %u)\n",
                ddr_bins_per_bucket_tbl0, ddr_bins_per_bucket_tbl1, ddr_bins_per_bucket_tbl2, ddr_bins_per_bucket_tbl3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_bins_per_bucket_tbl0 != gtmv(m, 8) || ddr_bins_per_bucket_tbl1 != gtmv(m, 8) || ddr_bins_per_bucket_tbl2 != gtmv(m, 8) || ddr_bins_per_bucket_tbl3 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_bins_per_bucket_tbl4 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl5 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl6 = gtmv(m, 8);
        uint8_t ddr_bins_per_bucket_tbl7 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_set( %u %u %u %u)\n",
            ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_set(ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_get(&ddr_bins_per_bucket_tbl4, &ddr_bins_per_bucket_tbl5, &ddr_bins_per_bucket_tbl6, &ddr_bins_per_bucket_tbl7);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_bins_per_bucket_1_get( %u %u %u %u)\n",
                ddr_bins_per_bucket_tbl4, ddr_bins_per_bucket_tbl5, ddr_bins_per_bucket_tbl6, ddr_bins_per_bucket_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_bins_per_bucket_tbl4 != gtmv(m, 8) || ddr_bins_per_bucket_tbl5 != gtmv(m, 8) || ddr_bins_per_bucket_tbl6 != gtmv(m, 8) || ddr_bins_per_bucket_tbl7 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_total_len natc_total_len = {.total_len_tbl0 = gtmv(m, 3), .total_len_tbl1 = gtmv(m, 3), .total_len_tbl2 = gtmv(m, 3), .total_len_tbl3 = gtmv(m, 3), .total_len_tbl4 = gtmv(m, 3), .total_len_tbl5 = gtmv(m, 3), .total_len_tbl6 = gtmv(m, 3), .total_len_tbl7 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_total_len_set( %u %u %u %u %u %u %u %u)\n",
            natc_total_len.total_len_tbl0, natc_total_len.total_len_tbl1, natc_total_len.total_len_tbl2, natc_total_len.total_len_tbl3, 
            natc_total_len.total_len_tbl4, natc_total_len.total_len_tbl5, natc_total_len.total_len_tbl6, natc_total_len.total_len_tbl7);
        ag_err = ag_drv_nat_cache_natc_total_len_set(&natc_total_len);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_total_len_get(&natc_total_len);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_total_len_get( %u %u %u %u %u %u %u %u)\n",
                natc_total_len.total_len_tbl0, natc_total_len.total_len_tbl1, natc_total_len.total_len_tbl2, natc_total_len.total_len_tbl3, 
                natc_total_len.total_len_tbl4, natc_total_len.total_len_tbl5, natc_total_len.total_len_tbl6, natc_total_len.total_len_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_total_len.total_len_tbl0 != gtmv(m, 3) || natc_total_len.total_len_tbl1 != gtmv(m, 3) || natc_total_len.total_len_tbl2 != gtmv(m, 3) || natc_total_len.total_len_tbl3 != gtmv(m, 3) || natc_total_len.total_len_tbl4 != gtmv(m, 3) || natc_total_len.total_len_tbl5 != gtmv(m, 3) || natc_total_len.total_len_tbl6 != gtmv(m, 3) || natc_total_len.total_len_tbl7 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_sm_status natc_sm_status = {.nat_state = gtmv(m, 15), .wb_state = gtmv(m, 1), .runner_cmd_state = gtmv(m, 1), .ddr_rep_state = gtmv(m, 3), .ddr_req_state = gtmv(m, 2), .apb_state = gtmv(m, 2), .debug_sel = gtmv(m, 2)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_sm_status_set( %u %u %u %u %u %u %u)\n",
            natc_sm_status.nat_state, natc_sm_status.wb_state, natc_sm_status.runner_cmd_state, natc_sm_status.ddr_rep_state, 
            natc_sm_status.ddr_req_state, natc_sm_status.apb_state, natc_sm_status.debug_sel);
        ag_err = ag_drv_nat_cache_natc_sm_status_set(&natc_sm_status);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_sm_status_get(&natc_sm_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_sm_status_get( %u %u %u %u %u %u %u)\n",
                natc_sm_status.nat_state, natc_sm_status.wb_state, natc_sm_status.runner_cmd_state, natc_sm_status.ddr_rep_state, 
                natc_sm_status.ddr_req_state, natc_sm_status.apb_state, natc_sm_status.debug_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_sm_status.nat_state != gtmv(m, 15) || natc_sm_status.wb_state != gtmv(m, 1) || natc_sm_status.runner_cmd_state != gtmv(m, 1) || natc_sm_status.ddr_rep_state != gtmv(m, 3) || natc_sm_status.ddr_req_state != gtmv(m, 2) || natc_sm_status.apb_state != gtmv(m, 2) || natc_sm_status.debug_sel != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t regfile_fifo_start_addr_0 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_1 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_2 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_3 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_set( %u %u %u %u)\n",
            regfile_fifo_start_addr_0, regfile_fifo_start_addr_1, regfile_fifo_start_addr_2, regfile_fifo_start_addr_3);
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_set(regfile_fifo_start_addr_0, regfile_fifo_start_addr_1, regfile_fifo_start_addr_2, regfile_fifo_start_addr_3);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_get(&regfile_fifo_start_addr_0, &regfile_fifo_start_addr_1, &regfile_fifo_start_addr_2, &regfile_fifo_start_addr_3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_regfile_fifo_start_addr_0_get( %u %u %u %u)\n",
                regfile_fifo_start_addr_0, regfile_fifo_start_addr_1, regfile_fifo_start_addr_2, regfile_fifo_start_addr_3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (regfile_fifo_start_addr_0 != gtmv(m, 8) || regfile_fifo_start_addr_1 != gtmv(m, 8) || regfile_fifo_start_addr_2 != gtmv(m, 8) || regfile_fifo_start_addr_3 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t regfile_fifo_start_addr_4 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_5 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_6 = gtmv(m, 8);
        uint8_t regfile_fifo_start_addr_7 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_set( %u %u %u %u)\n",
            regfile_fifo_start_addr_4, regfile_fifo_start_addr_5, regfile_fifo_start_addr_6, regfile_fifo_start_addr_7);
        ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_set(regfile_fifo_start_addr_4, regfile_fifo_start_addr_5, regfile_fifo_start_addr_6, regfile_fifo_start_addr_7);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_get(&regfile_fifo_start_addr_4, &regfile_fifo_start_addr_5, &regfile_fifo_start_addr_6, &regfile_fifo_start_addr_7);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_regfile_fifo_start_addr_1_get( %u %u %u %u)\n",
                regfile_fifo_start_addr_4, regfile_fifo_start_addr_5, regfile_fifo_start_addr_6, regfile_fifo_start_addr_7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (regfile_fifo_start_addr_4 != gtmv(m, 8) || regfile_fifo_start_addr_5 != gtmv(m, 8) || regfile_fifo_start_addr_6 != gtmv(m, 8) || regfile_fifo_start_addr_7 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_key_mask_set( %u %u)\n", index,
            key_mask);
        ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_set(index, key_mask);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_get(index, &key_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_key_mask_get( %u %u)\n", index,
                key_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_ddr_hash_mode natc_ddr_hash_mode = {.ddr_hash_mode_tbl0 = gtmv(m, 4), .ddr_hash_mode_tbl1 = gtmv(m, 4), .ddr_hash_mode_tbl2 = gtmv(m, 4), .ddr_hash_mode_tbl3 = gtmv(m, 4), .ddr_hash_mode_tbl4 = gtmv(m, 4), .ddr_hash_mode_tbl5 = gtmv(m, 4), .ddr_hash_mode_tbl6 = gtmv(m, 4), .ddr_hash_mode_tbl7 = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_hash_mode_set( %u %u %u %u %u %u %u %u)\n",
            natc_ddr_hash_mode.ddr_hash_mode_tbl0, natc_ddr_hash_mode.ddr_hash_mode_tbl1, natc_ddr_hash_mode.ddr_hash_mode_tbl2, natc_ddr_hash_mode.ddr_hash_mode_tbl3, 
            natc_ddr_hash_mode.ddr_hash_mode_tbl4, natc_ddr_hash_mode.ddr_hash_mode_tbl5, natc_ddr_hash_mode.ddr_hash_mode_tbl6, natc_ddr_hash_mode.ddr_hash_mode_tbl7);
        ag_err = ag_drv_nat_cache_natc_ddr_hash_mode_set(&natc_ddr_hash_mode);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_ddr_hash_mode_get(&natc_ddr_hash_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_ddr_hash_mode_get( %u %u %u %u %u %u %u %u)\n",
                natc_ddr_hash_mode.ddr_hash_mode_tbl0, natc_ddr_hash_mode.ddr_hash_mode_tbl1, natc_ddr_hash_mode.ddr_hash_mode_tbl2, natc_ddr_hash_mode.ddr_hash_mode_tbl3, 
                natc_ddr_hash_mode.ddr_hash_mode_tbl4, natc_ddr_hash_mode.ddr_hash_mode_tbl5, natc_ddr_hash_mode.ddr_hash_mode_tbl6, natc_ddr_hash_mode.ddr_hash_mode_tbl7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_ddr_hash_mode.ddr_hash_mode_tbl0 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl1 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl2 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl3 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl4 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl5 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl6 != gtmv(m, 4) || natc_ddr_hash_mode.ddr_hash_mode_tbl7 != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        nat_cache_natc_flow_cntr_cntl natc_flow_cntr_cntl = {.context_offset = gtmv(m, 7), .flow_cntr_en_tbl0 = gtmv(m, 1), .flow_cntr_en_tbl1 = gtmv(m, 1), .flow_cntr_en_tbl2 = gtmv(m, 1), .flow_cntr_en_tbl3 = gtmv(m, 1), .flow_cntr_en_tbl4 = gtmv(m, 1), .flow_cntr_en_tbl5 = gtmv(m, 1), .flow_cntr_en_tbl6 = gtmv(m, 1), .flow_cntr_en_tbl7 = gtmv(m, 1), .flow_cntr_clr_on_rd_en = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_nat_cache_natc_flow_cntr_cntl_set( %u %u %u %u %u %u %u %u %u %u)\n",
            natc_flow_cntr_cntl.context_offset, natc_flow_cntr_cntl.flow_cntr_en_tbl0, natc_flow_cntr_cntl.flow_cntr_en_tbl1, natc_flow_cntr_cntl.flow_cntr_en_tbl2, 
            natc_flow_cntr_cntl.flow_cntr_en_tbl3, natc_flow_cntr_cntl.flow_cntr_en_tbl4, natc_flow_cntr_cntl.flow_cntr_en_tbl5, natc_flow_cntr_cntl.flow_cntr_en_tbl6, 
            natc_flow_cntr_cntl.flow_cntr_en_tbl7, natc_flow_cntr_cntl.flow_cntr_clr_on_rd_en);
        ag_err = ag_drv_nat_cache_natc_flow_cntr_cntl_set(&natc_flow_cntr_cntl);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_natc_flow_cntr_cntl_get(&natc_flow_cntr_cntl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_flow_cntr_cntl_get( %u %u %u %u %u %u %u %u %u %u)\n",
                natc_flow_cntr_cntl.context_offset, natc_flow_cntr_cntl.flow_cntr_en_tbl0, natc_flow_cntr_cntl.flow_cntr_en_tbl1, natc_flow_cntr_cntl.flow_cntr_en_tbl2, 
                natc_flow_cntr_cntl.flow_cntr_en_tbl3, natc_flow_cntr_cntl.flow_cntr_en_tbl4, natc_flow_cntr_cntl.flow_cntr_en_tbl5, natc_flow_cntr_cntl.flow_cntr_en_tbl6, 
                natc_flow_cntr_cntl.flow_cntr_en_tbl7, natc_flow_cntr_cntl.flow_cntr_clr_on_rd_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_flow_cntr_cntl.context_offset != gtmv(m, 7) || natc_flow_cntr_cntl.flow_cntr_en_tbl0 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl1 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl2 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl3 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl4 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl5 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl6 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_en_tbl7 != gtmv(m, 1) || natc_flow_cntr_cntl.flow_cntr_clr_on_rd_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t natc_entry = gtmv(m, 10);
        bdmf_boolean w_r = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_set( %u %u)\n",
            natc_entry, w_r);
        ag_err = ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_set(natc_entry, w_r);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_get(&natc_entry, &w_r);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_indir_addr_reg_get( %u %u)\n",
                natc_entry, w_r);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_entry != gtmv(m, 10) || w_r != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t flow_cntr_entry = gtmv(m, 7);
        bdmf_boolean w_r = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_set( %u %u)\n",
            flow_cntr_entry, w_r);
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_set(flow_cntr_entry, w_r);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_get(&flow_cntr_entry, &w_r);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg_get( %u %u)\n",
                flow_cntr_entry, w_r);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow_cntr_entry != gtmv(m, 7) || w_r != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 1);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
            data);
        ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(index, data);
        if (!ag_err)
            ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get(index, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get( %u %u)\n", index,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 32))
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
static int ag_drv_nat_cache_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    bdmfmon_cmd_parm_t *p_start, *p_stop;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t ext_test_success_cnt = 0;
    uint32_t ext_test_failure_cnt = 0;
    uint32_t start_idx = 0;
    uint32_t stop_idx = 0;

    p_start = bdmfmon_cmd_find(session, "start_idx");
    p_stop = bdmfmon_cmd_find(session, "stop_idx");

    if (p_start)
        start_idx = p_start->value.unumber;
    if (p_stop)
        stop_idx = p_stop->value.unumber;

    if ((start_idx > stop_idx) && (stop_idx != 0))
    {
        bdmf_session_print(session, "ERROR: start_idx must be less than stop_idx\n");
        return BDMF_ERR_PARM;
    }

    m = bdmf_test_method_high; /* "Initialization" method */
    switch (parm[1].value.unumber)
    {
    case cli_nat_cache_nat0_key_result:
    {
        uint8_t max_zero = 2;
        uint8_t zero = gtmv(m, 1);
        nat_cache_nat0_key_result nat0_key_result = {.data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_key_result_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero,
                nat0_key_result.data[0], nat0_key_result.data[1], nat0_key_result.data[2], nat0_key_result.data[3], 
                nat0_key_result.data[4], nat0_key_result.data[5], nat0_key_result.data[6], nat0_key_result.data[7], 
                nat0_key_result.data[8], nat0_key_result.data[9], nat0_key_result.data[10], nat0_key_result.data[11], 
                nat0_key_result.data[12], nat0_key_result.data[13], nat0_key_result.data[14], nat0_key_result.data[15], 
                nat0_key_result.data[16], nat0_key_result.data[17]);
            ag_err = ag_drv_nat_cache_nat0_key_result_set(zero, &nat0_key_result);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        nat0_key_result.data[0] = gtmv(m, 32);
        nat0_key_result.data[1] = gtmv(m, 32);
        nat0_key_result.data[2] = gtmv(m, 32);
        nat0_key_result.data[3] = gtmv(m, 32);
        nat0_key_result.data[4] = gtmv(m, 32);
        nat0_key_result.data[5] = gtmv(m, 32);
        nat0_key_result.data[6] = gtmv(m, 32);
        nat0_key_result.data[7] = gtmv(m, 32);
        nat0_key_result.data[8] = gtmv(m, 32);
        nat0_key_result.data[9] = gtmv(m, 32);
        nat0_key_result.data[10] = gtmv(m, 32);
        nat0_key_result.data[11] = gtmv(m, 32);
        nat0_key_result.data[12] = gtmv(m, 32);
        nat0_key_result.data[13] = gtmv(m, 32);
        nat0_key_result.data[14] = gtmv(m, 32);
        nat0_key_result.data[15] = gtmv(m, 32);
        nat0_key_result.data[16] = gtmv(m, 32);
        nat0_key_result.data[17] = gtmv(m, 32);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_nat0_key_result_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero,
                nat0_key_result.data[0], nat0_key_result.data[1], nat0_key_result.data[2], nat0_key_result.data[3], 
                nat0_key_result.data[4], nat0_key_result.data[5], nat0_key_result.data[6], nat0_key_result.data[7], 
                nat0_key_result.data[8], nat0_key_result.data[9], nat0_key_result.data[10], nat0_key_result.data[11], 
                nat0_key_result.data[12], nat0_key_result.data[13], nat0_key_result.data[14], nat0_key_result.data[15], 
                nat0_key_result.data[16], nat0_key_result.data[17]);
            ag_err = ag_drv_nat_cache_nat0_key_result_set(zero, &nat0_key_result);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (zero = 0; zero < max_zero; zero++)
        {
            if (zero < start_idx || zero > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_nat0_key_result_get(zero, &nat0_key_result);

            bdmf_session_print(session, "ag_drv_nat_cache_nat0_key_result_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero,
                nat0_key_result.data[0], nat0_key_result.data[1], nat0_key_result.data[2], nat0_key_result.data[3], 
                nat0_key_result.data[4], nat0_key_result.data[5], nat0_key_result.data[6], nat0_key_result.data[7], 
                nat0_key_result.data[8], nat0_key_result.data[9], nat0_key_result.data[10], nat0_key_result.data[11], 
                nat0_key_result.data[12], nat0_key_result.data[13], nat0_key_result.data[14], nat0_key_result.data[15], 
                nat0_key_result.data[16], nat0_key_result.data[17]);

            if (nat0_key_result.data[0] != gtmv(m, 32) || 
                nat0_key_result.data[1] != gtmv(m, 32) || 
                nat0_key_result.data[2] != gtmv(m, 32) || 
                nat0_key_result.data[3] != gtmv(m, 32) || 
                nat0_key_result.data[4] != gtmv(m, 32) || 
                nat0_key_result.data[5] != gtmv(m, 32) || 
                nat0_key_result.data[6] != gtmv(m, 32) || 
                nat0_key_result.data[7] != gtmv(m, 32) || 
                nat0_key_result.data[8] != gtmv(m, 32) || 
                nat0_key_result.data[9] != gtmv(m, 32) || 
                nat0_key_result.data[10] != gtmv(m, 32) || 
                nat0_key_result.data[11] != gtmv(m, 32) || 
                nat0_key_result.data[12] != gtmv(m, 32) || 
                nat0_key_result.data[13] != gtmv(m, 32) || 
                nat0_key_result.data[14] != gtmv(m, 32) || 
                nat0_key_result.data[15] != gtmv(m, 32) || 
                nat0_key_result.data[16] != gtmv(m, 32) || 
                nat0_key_result.data[17] != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", zero);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of nat0_key_result completed. Number of tested entries %u.\n", max_zero);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl0_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl0_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl0_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl0_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl1_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl1_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl1_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl1_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl2_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl2_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl2_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl2_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl3_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl3_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl3_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl3_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl4_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl4_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl4_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl4_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl5_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl5_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl5_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl5_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl6_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl6_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl6_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl6_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_natc_tbl7_key_mask:
    {
        uint32_t max_index = 8;
        uint32_t index = gtmv(m, 3);
        uint32_t key_mask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        key_mask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_key_mask_set( %u %u)\n", index,
                key_mask);
            ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_set(index, key_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_natc_tbl7_key_mask_get(index, &key_mask);

            bdmf_session_print(session, "ag_drv_nat_cache_natc_tbl7_key_mask_get( %u %u)\n", index,
                key_mask);

            if (key_mask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_tbl7_key_mask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg:
    {
        uint32_t max_index = 2;
        uint32_t index = gtmv(m, 1);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
                data);
            ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(index, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
                data);
            ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_set(index, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get(index, &data);

            bdmf_session_print(session, "ag_drv_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg_get( %u %u)\n", index,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of indir_rw_natc_flow_cntr_indir_data_reg completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_nat_cache_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_natc_control_status: reg = &RU_REG(NAT_CACHE, NATC_CONTROL_STATUS); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_control_status2: reg = &RU_REG(NAT_CACHE, NATC_CONTROL_STATUS2); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_table_control: reg = &RU_REG(NAT_CACHE, NATC_TABLE_CONTROL); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_command_status: reg = &RU_REG(NAT_CACHE, NAT0_COMMAND_STATUS); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_hash: reg = &RU_REG(NAT_CACHE, NAT0_HASH); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_hit_count: reg = &RU_REG(NAT_CACHE, NAT0_HIT_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_byte_count: reg = &RU_REG(NAT_CACHE, NAT0_BYTE_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_pkt_len: reg = &RU_REG(NAT_CACHE, NAT0_PKT_LEN); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_nat0_key_result: reg = &RU_REG(NAT_CACHE, NAT0_KEY_RESULT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl0_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl0_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl0_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl0_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl1_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl1_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl1_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl1_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl2_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl2_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl2_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl2_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl3_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl3_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl3_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl3_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl4_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl4_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl4_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl4_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl5_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl5_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl5_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl5_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl6_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl6_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl6_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl6_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl7_ddr_key_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl7_ddr_key_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl7_ddr_result_base_address_lower: reg = &RU_REG(NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl7_ddr_result_base_address_upper: reg = &RU_REG(NAT_CACHE, NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_stat_counter_control_0: reg = &RU_REG(NAT_CACHE, NATC_STAT_COUNTER_CONTROL_0); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_stat_counter_control_1: reg = &RU_REG(NAT_CACHE, NATC_STAT_COUNTER_CONTROL_1); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_cache_hit_count: reg = &RU_REG(NAT_CACHE, NATC_CACHE_HIT_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_cache_miss_count: reg = &RU_REG(NAT_CACHE, NATC_CACHE_MISS_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_request_count: reg = &RU_REG(NAT_CACHE, NATC_DDR_REQUEST_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_evict_count: reg = &RU_REG(NAT_CACHE, NATC_DDR_EVICT_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_block_count: reg = &RU_REG(NAT_CACHE, NATC_DDR_BLOCK_COUNT); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_size: reg = &RU_REG(NAT_CACHE, NATC_DDR_SIZE); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_bins_per_bucket_0: reg = &RU_REG(NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_0); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_bins_per_bucket_1: reg = &RU_REG(NAT_CACHE, NATC_DDR_BINS_PER_BUCKET_1); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_total_len: reg = &RU_REG(NAT_CACHE, NATC_TOTAL_LEN); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_sm_status: reg = &RU_REG(NAT_CACHE, NATC_SM_STATUS); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_regfile_fifo_start_addr_0: reg = &RU_REG(NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_0); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_regfile_fifo_start_addr_1: reg = &RU_REG(NAT_CACHE, NATC_REGFILE_FIFO_START_ADDR_1); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl0_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL0_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl1_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL1_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl2_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL2_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl3_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL3_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl4_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL4_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl5_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL5_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl6_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL6_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_tbl7_key_mask: reg = &RU_REG(NAT_CACHE, NATC_TBL7_KEY_MASK); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_ddr_hash_mode: reg = &RU_REG(NAT_CACHE, NATC_DDR_HASH_MODE); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_natc_flow_cntr_cntl: reg = &RU_REG(NAT_CACHE, NATC_FLOW_CNTR_CNTL); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_indir_rw_natc_indir_addr_reg: reg = &RU_REG(NAT_CACHE, INDIR_RW_NATC_INDIR_ADDR_REG); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_indir_rw_natc_indir_data_reg: reg = &RU_REG(NAT_CACHE, INDIR_RW_NATC_INDIR_DATA_REG); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_indir_rw_natc_flow_cntr_indir_addr_reg: reg = &RU_REG(NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG); blk = &RU_BLK(NAT_CACHE); break;
    case bdmf_address_indir_rw_natc_flow_cntr_indir_data_reg: reg = &RU_REG(NAT_CACHE, INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG); blk = &RU_BLK(NAT_CACHE); break;
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

bdmfmon_handle_t ag_drv_nat_cache_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "nat_cache", "nat_cache", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_ctrl_status[] = {
            BDMFMON_MAKE_PARM("natc_reset", "natc_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_enable", "natc_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_reset", "regfile_fifo_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_clear_by_update_disable", "natc_smem_clear_by_update_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_smem_increment_on_reg_lookup", "natc_smem_increment_on_reg_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nat_arb_st", "nat_arb_st", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("decr_count_wraparound_enable", "decr_count_wraparound_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_hash_limit", "multi_hash_limit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nat_hash_mode", "nat_hash_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_disable_on_reg_lookup", "ddr_disable_on_reg_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_ddr_miss", "cache_update_on_ddr_miss", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pending_fifo_entry_check_enable", "pending_fifo_entry_check_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused1", "unused1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reg_8bit_in_32bit_swap_control", "reg_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reg_32bit_in_64bit_swap_control", "reg_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("repeated_key_det_en", "repeated_key_det_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_swap_all_control", "ddr_swap_all_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_8bit_in_32bit_swap_control", "smem_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_32bit_in_64bit_swap_control", "smem_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_64bit_in_128bit_swap_control", "ddr_64bit_in_128bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_init_done", "natc_init_done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused0", "unused0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("natc_add_command_speedup_mode", "natc_add_command_speedup_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_enable", "ddr_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused2", "unused2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_lookup_pending_fifo_mode_disable", "ddr_lookup_pending_fifo_mode_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_replace_duplicated_cached_entry_enable", "ddr_replace_duplicated_cached_entry_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_swap", "ddr_hash_swap", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_counter_8bit_in_32bit_swap_control", "ddr_counter_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_update_on_reg_ddr_lookup", "cache_update_on_reg_ddr_lookup", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused3", "unused3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused4", "unused4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_algo", "cache_algo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer", "age_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("age_timer_tick", "age_timer_tick", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_lookup_blocking_mode", "cache_lookup_blocking_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_8bit_in_32bit_swap_control", "ddr_8bit_in_32bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_32bit_in_64bit_swap_control", "ddr_32bit_in_64bit_swap_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused5", "unused5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_ctrs[] = {
            BDMFMON_MAKE_PARM("cache_hit_count", "cache_hit_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_miss_count", "cache_miss_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_request_count", "ddr_request_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_evict_count", "ddr_evict_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_block_count", "ddr_block_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_table_control[] = {
            BDMFMON_MAKE_PARM("non_cacheable_tbl0", "non_cacheable_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl0", "key_len_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl1", "non_cacheable_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl1", "key_len_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl2", "non_cacheable_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl2", "key_len_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl3", "non_cacheable_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl3", "key_len_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl4", "non_cacheable_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl4", "key_len_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl5", "non_cacheable_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl5", "key_len_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl6", "non_cacheable_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl6", "key_len_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_cacheable_tbl7", "non_cacheable_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_len_tbl7", "key_len_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl0", "var_context_len_en_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl1", "var_context_len_en_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl2", "var_context_len_en_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl3", "var_context_len_en_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl4", "var_context_len_en_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl5", "var_context_len_en_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl6", "var_context_len_en_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("var_context_len_en_tbl7", "var_context_len_en_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl0", "smem_dis_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl1", "smem_dis_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl2", "smem_dis_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl3", "smem_dis_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl4", "smem_dis_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl5", "smem_dis_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl6", "smem_dis_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smem_dis_tbl7", "smem_dis_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nat0_command_status[] = {
            BDMFMON_MAKE_PARM("command", "command", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused0", "unused0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("busy", "busy", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("error", "error", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("miss", "miss", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_hit", "cache_hit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multihash_count", "multihash_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nat_tbl", "nat_tbl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("decr_count", "decr_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_flush", "cache_flush", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("del_cmd_mode", "del_cmd_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("add_cmd_mode", "add_cmd_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("del_cmd_ddr_bin", "del_cmd_ddr_bin", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("add_cmd_ddr_bin", "add_cmd_ddr_bin", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("add_cmd_ddr_miss", "add_cmd_ddr_miss", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nat0_pkt_len[] = {
            BDMFMON_MAKE_PARM("pkt_len", "pkt_len", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unused", "unused", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nat0_key_result[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data0", "data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data1", "data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data2", "data2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data3", "data3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data4", "data4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data5", "data5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data6", "data6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data7", "data7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data8", "data8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data9", "data9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data10", "data10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data11", "data11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data12", "data12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data13", "data13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data14", "data14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data15", "data15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data16", "data16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data17", "data17", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl0_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl0_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl0_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl0_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl1_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl1_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl1_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl1_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl2_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl2_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl2_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl2_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl3_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl3_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl3_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl3_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl4_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl4_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl4_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl4_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl5_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl5_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl5_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl5_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl6_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl6_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl6_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl6_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl7_ddr_key_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl7_ddr_key_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl7_ddr_result_base_address_lower[] = {
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl7_ddr_result_base_address_upper[] = {
            BDMFMON_MAKE_PARM("bar", "bar", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zeros", "zeros", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_stat_counter_control_0[] = {
            BDMFMON_MAKE_PARM("cache_hit_count_en", "cache_hit_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_miss_count_en", "cache_miss_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_request_count_en", "ddr_request_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_evict_count_en", "ddr_evict_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_stat_counter_control_1[] = {
            BDMFMON_MAKE_PARM("ddr_block_count_en", "ddr_block_count_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counter_wraparound_dis", "counter_wraparound_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_ddr_size[] = {
            BDMFMON_MAKE_PARM("ddr_size_tbl0", "ddr_size_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl1", "ddr_size_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl2", "ddr_size_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl3", "ddr_size_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl4", "ddr_size_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl5", "ddr_size_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl6", "ddr_size_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_size_tbl7", "ddr_size_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_ddr_bins_per_bucket_0[] = {
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl0", "ddr_bins_per_bucket_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl1", "ddr_bins_per_bucket_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl2", "ddr_bins_per_bucket_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl3", "ddr_bins_per_bucket_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_ddr_bins_per_bucket_1[] = {
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl4", "ddr_bins_per_bucket_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl5", "ddr_bins_per_bucket_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl6", "ddr_bins_per_bucket_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bins_per_bucket_tbl7", "ddr_bins_per_bucket_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_total_len[] = {
            BDMFMON_MAKE_PARM("total_len_tbl0", "total_len_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl1", "total_len_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl2", "total_len_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl3", "total_len_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl4", "total_len_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl5", "total_len_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl6", "total_len_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("total_len_tbl7", "total_len_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_sm_status[] = {
            BDMFMON_MAKE_PARM("nat_state", "nat_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wb_state", "wb_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("runner_cmd_state", "runner_cmd_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_rep_state", "ddr_rep_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_req_state", "ddr_req_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("apb_state", "apb_state", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_sel", "debug_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_regfile_fifo_start_addr_0[] = {
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_0", "regfile_fifo_start_addr_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_1", "regfile_fifo_start_addr_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_2", "regfile_fifo_start_addr_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_3", "regfile_fifo_start_addr_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_regfile_fifo_start_addr_1[] = {
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_4", "regfile_fifo_start_addr_4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_5", "regfile_fifo_start_addr_5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_6", "regfile_fifo_start_addr_6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("regfile_fifo_start_addr_7", "regfile_fifo_start_addr_7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl0_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl1_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl2_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl3_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl4_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl5_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl6_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_tbl7_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_mask", "key_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_ddr_hash_mode[] = {
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl0", "ddr_hash_mode_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl1", "ddr_hash_mode_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl2", "ddr_hash_mode_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl3", "ddr_hash_mode_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl4", "ddr_hash_mode_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl5", "ddr_hash_mode_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl6", "ddr_hash_mode_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hash_mode_tbl7", "ddr_hash_mode_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_flow_cntr_cntl[] = {
            BDMFMON_MAKE_PARM("context_offset", "context_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl0", "flow_cntr_en_tbl0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl1", "flow_cntr_en_tbl1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl2", "flow_cntr_en_tbl2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl3", "flow_cntr_en_tbl3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl4", "flow_cntr_en_tbl4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl5", "flow_cntr_en_tbl5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl6", "flow_cntr_en_tbl6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_en_tbl7", "flow_cntr_en_tbl7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_cntr_clr_on_rd_en", "flow_cntr_clr_on_rd_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_indir_rw_natc_indir_addr_reg[] = {
            BDMFMON_MAKE_PARM("natc_entry", "natc_entry", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("w_r", "w_r", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_indir_rw_natc_flow_cntr_indir_addr_reg[] = {
            BDMFMON_MAKE_PARM("flow_cntr_entry", "flow_cntr_entry", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("w_r", "w_r", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_indir_rw_natc_flow_cntr_indir_data_reg[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ctrl_status", .val = cli_nat_cache_ctrl_status, .parms = set_ctrl_status },
            { .name = "natc_ctrs", .val = cli_nat_cache_natc_ctrs, .parms = set_natc_ctrs },
            { .name = "natc_table_control", .val = cli_nat_cache_natc_table_control, .parms = set_natc_table_control },
            { .name = "nat0_command_status", .val = cli_nat_cache_nat0_command_status, .parms = set_nat0_command_status },
            { .name = "nat0_pkt_len", .val = cli_nat_cache_nat0_pkt_len, .parms = set_nat0_pkt_len },
            { .name = "nat0_key_result", .val = cli_nat_cache_nat0_key_result, .parms = set_nat0_key_result },
            { .name = "natc_tbl0_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl0_ddr_key_base_address_lower, .parms = set_natc_tbl0_ddr_key_base_address_lower },
            { .name = "natc_tbl0_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl0_ddr_key_base_address_upper, .parms = set_natc_tbl0_ddr_key_base_address_upper },
            { .name = "natc_tbl0_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl0_ddr_result_base_address_lower, .parms = set_natc_tbl0_ddr_result_base_address_lower },
            { .name = "natc_tbl0_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl0_ddr_result_base_address_upper, .parms = set_natc_tbl0_ddr_result_base_address_upper },
            { .name = "natc_tbl1_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl1_ddr_key_base_address_lower, .parms = set_natc_tbl1_ddr_key_base_address_lower },
            { .name = "natc_tbl1_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl1_ddr_key_base_address_upper, .parms = set_natc_tbl1_ddr_key_base_address_upper },
            { .name = "natc_tbl1_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl1_ddr_result_base_address_lower, .parms = set_natc_tbl1_ddr_result_base_address_lower },
            { .name = "natc_tbl1_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl1_ddr_result_base_address_upper, .parms = set_natc_tbl1_ddr_result_base_address_upper },
            { .name = "natc_tbl2_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl2_ddr_key_base_address_lower, .parms = set_natc_tbl2_ddr_key_base_address_lower },
            { .name = "natc_tbl2_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl2_ddr_key_base_address_upper, .parms = set_natc_tbl2_ddr_key_base_address_upper },
            { .name = "natc_tbl2_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl2_ddr_result_base_address_lower, .parms = set_natc_tbl2_ddr_result_base_address_lower },
            { .name = "natc_tbl2_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl2_ddr_result_base_address_upper, .parms = set_natc_tbl2_ddr_result_base_address_upper },
            { .name = "natc_tbl3_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl3_ddr_key_base_address_lower, .parms = set_natc_tbl3_ddr_key_base_address_lower },
            { .name = "natc_tbl3_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl3_ddr_key_base_address_upper, .parms = set_natc_tbl3_ddr_key_base_address_upper },
            { .name = "natc_tbl3_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl3_ddr_result_base_address_lower, .parms = set_natc_tbl3_ddr_result_base_address_lower },
            { .name = "natc_tbl3_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl3_ddr_result_base_address_upper, .parms = set_natc_tbl3_ddr_result_base_address_upper },
            { .name = "natc_tbl4_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl4_ddr_key_base_address_lower, .parms = set_natc_tbl4_ddr_key_base_address_lower },
            { .name = "natc_tbl4_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl4_ddr_key_base_address_upper, .parms = set_natc_tbl4_ddr_key_base_address_upper },
            { .name = "natc_tbl4_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl4_ddr_result_base_address_lower, .parms = set_natc_tbl4_ddr_result_base_address_lower },
            { .name = "natc_tbl4_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl4_ddr_result_base_address_upper, .parms = set_natc_tbl4_ddr_result_base_address_upper },
            { .name = "natc_tbl5_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl5_ddr_key_base_address_lower, .parms = set_natc_tbl5_ddr_key_base_address_lower },
            { .name = "natc_tbl5_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl5_ddr_key_base_address_upper, .parms = set_natc_tbl5_ddr_key_base_address_upper },
            { .name = "natc_tbl5_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl5_ddr_result_base_address_lower, .parms = set_natc_tbl5_ddr_result_base_address_lower },
            { .name = "natc_tbl5_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl5_ddr_result_base_address_upper, .parms = set_natc_tbl5_ddr_result_base_address_upper },
            { .name = "natc_tbl6_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl6_ddr_key_base_address_lower, .parms = set_natc_tbl6_ddr_key_base_address_lower },
            { .name = "natc_tbl6_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl6_ddr_key_base_address_upper, .parms = set_natc_tbl6_ddr_key_base_address_upper },
            { .name = "natc_tbl6_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl6_ddr_result_base_address_lower, .parms = set_natc_tbl6_ddr_result_base_address_lower },
            { .name = "natc_tbl6_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl6_ddr_result_base_address_upper, .parms = set_natc_tbl6_ddr_result_base_address_upper },
            { .name = "natc_tbl7_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl7_ddr_key_base_address_lower, .parms = set_natc_tbl7_ddr_key_base_address_lower },
            { .name = "natc_tbl7_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl7_ddr_key_base_address_upper, .parms = set_natc_tbl7_ddr_key_base_address_upper },
            { .name = "natc_tbl7_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl7_ddr_result_base_address_lower, .parms = set_natc_tbl7_ddr_result_base_address_lower },
            { .name = "natc_tbl7_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl7_ddr_result_base_address_upper, .parms = set_natc_tbl7_ddr_result_base_address_upper },
            { .name = "natc_stat_counter_control_0", .val = cli_nat_cache_natc_stat_counter_control_0, .parms = set_natc_stat_counter_control_0 },
            { .name = "natc_stat_counter_control_1", .val = cli_nat_cache_natc_stat_counter_control_1, .parms = set_natc_stat_counter_control_1 },
            { .name = "natc_ddr_size", .val = cli_nat_cache_natc_ddr_size, .parms = set_natc_ddr_size },
            { .name = "natc_ddr_bins_per_bucket_0", .val = cli_nat_cache_natc_ddr_bins_per_bucket_0, .parms = set_natc_ddr_bins_per_bucket_0 },
            { .name = "natc_ddr_bins_per_bucket_1", .val = cli_nat_cache_natc_ddr_bins_per_bucket_1, .parms = set_natc_ddr_bins_per_bucket_1 },
            { .name = "natc_total_len", .val = cli_nat_cache_natc_total_len, .parms = set_natc_total_len },
            { .name = "natc_sm_status", .val = cli_nat_cache_natc_sm_status, .parms = set_natc_sm_status },
            { .name = "natc_regfile_fifo_start_addr_0", .val = cli_nat_cache_natc_regfile_fifo_start_addr_0, .parms = set_natc_regfile_fifo_start_addr_0 },
            { .name = "natc_regfile_fifo_start_addr_1", .val = cli_nat_cache_natc_regfile_fifo_start_addr_1, .parms = set_natc_regfile_fifo_start_addr_1 },
            { .name = "natc_tbl0_key_mask", .val = cli_nat_cache_natc_tbl0_key_mask, .parms = set_natc_tbl0_key_mask },
            { .name = "natc_tbl1_key_mask", .val = cli_nat_cache_natc_tbl1_key_mask, .parms = set_natc_tbl1_key_mask },
            { .name = "natc_tbl2_key_mask", .val = cli_nat_cache_natc_tbl2_key_mask, .parms = set_natc_tbl2_key_mask },
            { .name = "natc_tbl3_key_mask", .val = cli_nat_cache_natc_tbl3_key_mask, .parms = set_natc_tbl3_key_mask },
            { .name = "natc_tbl4_key_mask", .val = cli_nat_cache_natc_tbl4_key_mask, .parms = set_natc_tbl4_key_mask },
            { .name = "natc_tbl5_key_mask", .val = cli_nat_cache_natc_tbl5_key_mask, .parms = set_natc_tbl5_key_mask },
            { .name = "natc_tbl6_key_mask", .val = cli_nat_cache_natc_tbl6_key_mask, .parms = set_natc_tbl6_key_mask },
            { .name = "natc_tbl7_key_mask", .val = cli_nat_cache_natc_tbl7_key_mask, .parms = set_natc_tbl7_key_mask },
            { .name = "natc_ddr_hash_mode", .val = cli_nat_cache_natc_ddr_hash_mode, .parms = set_natc_ddr_hash_mode },
            { .name = "natc_flow_cntr_cntl", .val = cli_nat_cache_natc_flow_cntr_cntl, .parms = set_natc_flow_cntr_cntl },
            { .name = "indir_rw_natc_indir_addr_reg", .val = cli_nat_cache_indir_rw_natc_indir_addr_reg, .parms = set_indir_rw_natc_indir_addr_reg },
            { .name = "indir_rw_natc_flow_cntr_indir_addr_reg", .val = cli_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg, .parms = set_indir_rw_natc_flow_cntr_indir_addr_reg },
            { .name = "indir_rw_natc_flow_cntr_indir_data_reg", .val = cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg, .parms = set_indir_rw_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_nat_cache_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_nat0_key_result[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl0_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl1_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl2_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl3_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl4_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl5_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl6_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_tbl7_key_mask[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_indir_rw_natc_flow_cntr_indir_data_reg[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ctrl_status", .val = cli_nat_cache_ctrl_status, .parms = get_default },
            { .name = "natc_ctrs", .val = cli_nat_cache_natc_ctrs, .parms = get_default },
            { .name = "natc_table_control", .val = cli_nat_cache_natc_table_control, .parms = get_default },
            { .name = "nat0_command_status", .val = cli_nat_cache_nat0_command_status, .parms = get_default },
            { .name = "nat0_hash", .val = cli_nat_cache_nat0_hash, .parms = get_default },
            { .name = "nat0_hit_count", .val = cli_nat_cache_nat0_hit_count, .parms = get_default },
            { .name = "nat0_byte_count", .val = cli_nat_cache_nat0_byte_count, .parms = get_default },
            { .name = "nat0_pkt_len", .val = cli_nat_cache_nat0_pkt_len, .parms = get_default },
            { .name = "nat0_key_result", .val = cli_nat_cache_nat0_key_result, .parms = get_nat0_key_result },
            { .name = "natc_tbl0_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl0_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl0_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl0_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl0_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl0_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl0_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl0_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl1_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl1_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl1_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl1_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl1_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl1_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl1_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl1_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl2_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl2_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl2_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl2_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl2_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl2_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl2_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl2_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl3_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl3_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl3_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl3_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl3_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl3_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl3_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl3_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl4_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl4_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl4_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl4_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl4_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl4_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl4_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl4_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl5_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl5_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl5_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl5_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl5_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl5_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl5_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl5_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl6_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl6_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl6_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl6_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl6_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl6_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl6_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl6_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_tbl7_ddr_key_base_address_lower", .val = cli_nat_cache_natc_tbl7_ddr_key_base_address_lower, .parms = get_default },
            { .name = "natc_tbl7_ddr_key_base_address_upper", .val = cli_nat_cache_natc_tbl7_ddr_key_base_address_upper, .parms = get_default },
            { .name = "natc_tbl7_ddr_result_base_address_lower", .val = cli_nat_cache_natc_tbl7_ddr_result_base_address_lower, .parms = get_default },
            { .name = "natc_tbl7_ddr_result_base_address_upper", .val = cli_nat_cache_natc_tbl7_ddr_result_base_address_upper, .parms = get_default },
            { .name = "natc_stat_counter_control_0", .val = cli_nat_cache_natc_stat_counter_control_0, .parms = get_default },
            { .name = "natc_stat_counter_control_1", .val = cli_nat_cache_natc_stat_counter_control_1, .parms = get_default },
            { .name = "natc_ddr_size", .val = cli_nat_cache_natc_ddr_size, .parms = get_default },
            { .name = "natc_ddr_bins_per_bucket_0", .val = cli_nat_cache_natc_ddr_bins_per_bucket_0, .parms = get_default },
            { .name = "natc_ddr_bins_per_bucket_1", .val = cli_nat_cache_natc_ddr_bins_per_bucket_1, .parms = get_default },
            { .name = "natc_total_len", .val = cli_nat_cache_natc_total_len, .parms = get_default },
            { .name = "natc_sm_status", .val = cli_nat_cache_natc_sm_status, .parms = get_default },
            { .name = "natc_regfile_fifo_start_addr_0", .val = cli_nat_cache_natc_regfile_fifo_start_addr_0, .parms = get_default },
            { .name = "natc_regfile_fifo_start_addr_1", .val = cli_nat_cache_natc_regfile_fifo_start_addr_1, .parms = get_default },
            { .name = "natc_tbl0_key_mask", .val = cli_nat_cache_natc_tbl0_key_mask, .parms = get_natc_tbl0_key_mask },
            { .name = "natc_tbl1_key_mask", .val = cli_nat_cache_natc_tbl1_key_mask, .parms = get_natc_tbl1_key_mask },
            { .name = "natc_tbl2_key_mask", .val = cli_nat_cache_natc_tbl2_key_mask, .parms = get_natc_tbl2_key_mask },
            { .name = "natc_tbl3_key_mask", .val = cli_nat_cache_natc_tbl3_key_mask, .parms = get_natc_tbl3_key_mask },
            { .name = "natc_tbl4_key_mask", .val = cli_nat_cache_natc_tbl4_key_mask, .parms = get_natc_tbl4_key_mask },
            { .name = "natc_tbl5_key_mask", .val = cli_nat_cache_natc_tbl5_key_mask, .parms = get_natc_tbl5_key_mask },
            { .name = "natc_tbl6_key_mask", .val = cli_nat_cache_natc_tbl6_key_mask, .parms = get_natc_tbl6_key_mask },
            { .name = "natc_tbl7_key_mask", .val = cli_nat_cache_natc_tbl7_key_mask, .parms = get_natc_tbl7_key_mask },
            { .name = "natc_ddr_hash_mode", .val = cli_nat_cache_natc_ddr_hash_mode, .parms = get_default },
            { .name = "natc_flow_cntr_cntl", .val = cli_nat_cache_natc_flow_cntr_cntl, .parms = get_default },
            { .name = "indir_rw_natc_indir_addr_reg", .val = cli_nat_cache_indir_rw_natc_indir_addr_reg, .parms = get_default },
            { .name = "indir_rw_natc_flow_cntr_indir_addr_reg", .val = cli_nat_cache_indir_rw_natc_flow_cntr_indir_addr_reg, .parms = get_default },
            { .name = "indir_rw_natc_flow_cntr_indir_data_reg", .val = cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg, .parms = get_indir_rw_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_nat_cache_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_nat_cache_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "nat0_key_result", .val = cli_nat_cache_nat0_key_result, .parms = ext_test_default},
            { .name = "natc_tbl0_key_mask", .val = cli_nat_cache_natc_tbl0_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl1_key_mask", .val = cli_nat_cache_natc_tbl1_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl2_key_mask", .val = cli_nat_cache_natc_tbl2_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl3_key_mask", .val = cli_nat_cache_natc_tbl3_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl4_key_mask", .val = cli_nat_cache_natc_tbl4_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl5_key_mask", .val = cli_nat_cache_natc_tbl5_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl6_key_mask", .val = cli_nat_cache_natc_tbl6_key_mask, .parms = ext_test_default},
            { .name = "natc_tbl7_key_mask", .val = cli_nat_cache_natc_tbl7_key_mask, .parms = ext_test_default},
            { .name = "indir_rw_natc_flow_cntr_indir_data_reg", .val = cli_nat_cache_indir_rw_natc_flow_cntr_indir_data_reg, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_nat_cache_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "NATC_CONTROL_STATUS", .val = bdmf_address_natc_control_status },
            { .name = "NATC_CONTROL_STATUS2", .val = bdmf_address_natc_control_status2 },
            { .name = "NATC_TABLE_CONTROL", .val = bdmf_address_natc_table_control },
            { .name = "NAT0_COMMAND_STATUS", .val = bdmf_address_nat0_command_status },
            { .name = "NAT0_HASH", .val = bdmf_address_nat0_hash },
            { .name = "NAT0_HIT_COUNT", .val = bdmf_address_nat0_hit_count },
            { .name = "NAT0_BYTE_COUNT", .val = bdmf_address_nat0_byte_count },
            { .name = "NAT0_PKT_LEN", .val = bdmf_address_nat0_pkt_len },
            { .name = "NAT0_KEY_RESULT", .val = bdmf_address_nat0_key_result },
            { .name = "NATC_TBL0_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl0_ddr_key_base_address_lower },
            { .name = "NATC_TBL0_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl0_ddr_key_base_address_upper },
            { .name = "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl0_ddr_result_base_address_lower },
            { .name = "NATC_TBL0_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl0_ddr_result_base_address_upper },
            { .name = "NATC_TBL1_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl1_ddr_key_base_address_lower },
            { .name = "NATC_TBL1_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl1_ddr_key_base_address_upper },
            { .name = "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl1_ddr_result_base_address_lower },
            { .name = "NATC_TBL1_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl1_ddr_result_base_address_upper },
            { .name = "NATC_TBL2_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl2_ddr_key_base_address_lower },
            { .name = "NATC_TBL2_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl2_ddr_key_base_address_upper },
            { .name = "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl2_ddr_result_base_address_lower },
            { .name = "NATC_TBL2_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl2_ddr_result_base_address_upper },
            { .name = "NATC_TBL3_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl3_ddr_key_base_address_lower },
            { .name = "NATC_TBL3_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl3_ddr_key_base_address_upper },
            { .name = "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl3_ddr_result_base_address_lower },
            { .name = "NATC_TBL3_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl3_ddr_result_base_address_upper },
            { .name = "NATC_TBL4_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl4_ddr_key_base_address_lower },
            { .name = "NATC_TBL4_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl4_ddr_key_base_address_upper },
            { .name = "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl4_ddr_result_base_address_lower },
            { .name = "NATC_TBL4_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl4_ddr_result_base_address_upper },
            { .name = "NATC_TBL5_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl5_ddr_key_base_address_lower },
            { .name = "NATC_TBL5_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl5_ddr_key_base_address_upper },
            { .name = "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl5_ddr_result_base_address_lower },
            { .name = "NATC_TBL5_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl5_ddr_result_base_address_upper },
            { .name = "NATC_TBL6_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl6_ddr_key_base_address_lower },
            { .name = "NATC_TBL6_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl6_ddr_key_base_address_upper },
            { .name = "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl6_ddr_result_base_address_lower },
            { .name = "NATC_TBL6_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl6_ddr_result_base_address_upper },
            { .name = "NATC_TBL7_DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl7_ddr_key_base_address_lower },
            { .name = "NATC_TBL7_DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl7_ddr_key_base_address_upper },
            { .name = "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_natc_tbl7_ddr_result_base_address_lower },
            { .name = "NATC_TBL7_DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_natc_tbl7_ddr_result_base_address_upper },
            { .name = "NATC_STAT_COUNTER_CONTROL_0", .val = bdmf_address_natc_stat_counter_control_0 },
            { .name = "NATC_STAT_COUNTER_CONTROL_1", .val = bdmf_address_natc_stat_counter_control_1 },
            { .name = "NATC_CACHE_HIT_COUNT", .val = bdmf_address_natc_cache_hit_count },
            { .name = "NATC_CACHE_MISS_COUNT", .val = bdmf_address_natc_cache_miss_count },
            { .name = "NATC_DDR_REQUEST_COUNT", .val = bdmf_address_natc_ddr_request_count },
            { .name = "NATC_DDR_EVICT_COUNT", .val = bdmf_address_natc_ddr_evict_count },
            { .name = "NATC_DDR_BLOCK_COUNT", .val = bdmf_address_natc_ddr_block_count },
            { .name = "NATC_DDR_SIZE", .val = bdmf_address_natc_ddr_size },
            { .name = "NATC_DDR_BINS_PER_BUCKET_0", .val = bdmf_address_natc_ddr_bins_per_bucket_0 },
            { .name = "NATC_DDR_BINS_PER_BUCKET_1", .val = bdmf_address_natc_ddr_bins_per_bucket_1 },
            { .name = "NATC_TOTAL_LEN", .val = bdmf_address_natc_total_len },
            { .name = "NATC_SM_STATUS", .val = bdmf_address_natc_sm_status },
            { .name = "NATC_REGFILE_FIFO_START_ADDR_0", .val = bdmf_address_natc_regfile_fifo_start_addr_0 },
            { .name = "NATC_REGFILE_FIFO_START_ADDR_1", .val = bdmf_address_natc_regfile_fifo_start_addr_1 },
            { .name = "NATC_TBL0_KEY_MASK", .val = bdmf_address_natc_tbl0_key_mask },
            { .name = "NATC_TBL1_KEY_MASK", .val = bdmf_address_natc_tbl1_key_mask },
            { .name = "NATC_TBL2_KEY_MASK", .val = bdmf_address_natc_tbl2_key_mask },
            { .name = "NATC_TBL3_KEY_MASK", .val = bdmf_address_natc_tbl3_key_mask },
            { .name = "NATC_TBL4_KEY_MASK", .val = bdmf_address_natc_tbl4_key_mask },
            { .name = "NATC_TBL5_KEY_MASK", .val = bdmf_address_natc_tbl5_key_mask },
            { .name = "NATC_TBL6_KEY_MASK", .val = bdmf_address_natc_tbl6_key_mask },
            { .name = "NATC_TBL7_KEY_MASK", .val = bdmf_address_natc_tbl7_key_mask },
            { .name = "NATC_DDR_HASH_MODE", .val = bdmf_address_natc_ddr_hash_mode },
            { .name = "NATC_FLOW_CNTR_CNTL", .val = bdmf_address_natc_flow_cntr_cntl },
            { .name = "INDIR_RW_NATC_INDIR_ADDR_REG", .val = bdmf_address_indir_rw_natc_indir_addr_reg },
            { .name = "INDIR_RW_NATC_INDIR_DATA_REG", .val = bdmf_address_indir_rw_natc_indir_data_reg },
            { .name = "INDIR_RW_NATC_FLOW_CNTR_INDIR_ADDR_REG", .val = bdmf_address_indir_rw_natc_flow_cntr_indir_addr_reg },
            { .name = "INDIR_RW_NATC_FLOW_CNTR_INDIR_DATA_REG", .val = bdmf_address_indir_rw_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_nat_cache_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
