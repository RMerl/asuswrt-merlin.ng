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
#include "xrdp_drv_fpm_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_fpm_init_mem_set(bdmf_boolean init_mem)
{
    uint32_t reg_ctl_1 = 0;

#ifdef VALIDATE_PARMS
    if ((init_mem >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, CTL_1, reg_ctl_1);

    reg_ctl_1 = RU_FIELD_SET(0, FPM, CTL_1, INIT_MEM, reg_ctl_1, init_mem);

    RU_REG_WRITE(0, FPM, CTL_1, reg_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_init_mem_get(bdmf_boolean *init_mem)
{
    uint32_t reg_ctl_1;

#ifdef VALIDATE_PARMS
    if (!init_mem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CTL_1, reg_ctl_1);

    *init_mem = RU_FIELD_GET(0, FPM, CTL_1, INIT_MEM, reg_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_en_set(bdmf_boolean pool_enable)
{
    uint32_t reg_ctl_1 = 0;

#ifdef VALIDATE_PARMS
    if ((pool_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, CTL_1, reg_ctl_1);

    reg_ctl_1 = RU_FIELD_SET(0, FPM, CTL_1, POOL_ENABLE, reg_ctl_1, pool_enable);

    RU_REG_WRITE(0, FPM, CTL_1, reg_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_en_get(bdmf_boolean *pool_enable)
{
    uint32_t reg_ctl_1;

#ifdef VALIDATE_PARMS
    if (!pool_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CTL_1, reg_ctl_1);

    *pool_enable = RU_FIELD_GET(0, FPM, CTL_1, POOL_ENABLE, reg_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_stat_set(const fpm_pool_stat *pool_stat)
{
    uint32_t reg_stat1 = 0;
    uint32_t reg_stat2 = 0;
    uint32_t reg_stat3 = 0;
    uint32_t reg_stat4 = 0;
    uint32_t reg_stat6 = 0;
    uint32_t reg_stat7 = 0;
    uint32_t reg_stat8 = 0;

#ifdef VALIDATE_PARMS
    if(!pool_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_stat->pool_full >= _1BITS_MAX_VAL_) ||
       (pool_stat->free_fifo_full >= _1BITS_MAX_VAL_) ||
       (pool_stat->free_fifo_empty >= _1BITS_MAX_VAL_) ||
       (pool_stat->alloc_fifo_full >= _1BITS_MAX_VAL_) ||
       (pool_stat->alloc_fifo_empty >= _1BITS_MAX_VAL_) ||
       (pool_stat->num_of_tokens_available >= _22BITS_MAX_VAL_) ||
       (pool_stat->num_of_not_valid_token_frees >= _22BITS_MAX_VAL_) ||
       (pool_stat->num_of_not_valid_token_multi >= _22BITS_MAX_VAL_) ||
       (pool_stat->tokens_available_low_wtmk >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_stat1 = RU_FIELD_SET(0, FPM, STAT1, OVRFL, reg_stat1, pool_stat->ovrfl);
    reg_stat1 = RU_FIELD_SET(0, FPM, STAT1, UNDRFL, reg_stat1, pool_stat->undrfl);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, POOL_FULL, reg_stat2, pool_stat->pool_full);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, FREE_FIFO_FULL, reg_stat2, pool_stat->free_fifo_full);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, FREE_FIFO_EMPTY, reg_stat2, pool_stat->free_fifo_empty);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, ALLOC_FIFO_FULL, reg_stat2, pool_stat->alloc_fifo_full);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, ALLOC_FIFO_EMPTY, reg_stat2, pool_stat->alloc_fifo_empty);
    reg_stat2 = RU_FIELD_SET(0, FPM, STAT2, NUM_OF_TOKENS_AVAILABLE, reg_stat2, pool_stat->num_of_tokens_available);
    reg_stat3 = RU_FIELD_SET(0, FPM, STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_stat3, pool_stat->num_of_not_valid_token_frees);
    reg_stat4 = RU_FIELD_SET(0, FPM, STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_stat4, pool_stat->num_of_not_valid_token_multi);
    reg_stat6 = RU_FIELD_SET(0, FPM, STAT6, INVALID_FREE_TOKEN, reg_stat6, pool_stat->invalid_free_token);
    reg_stat7 = RU_FIELD_SET(0, FPM, STAT7, INVALID_MCAST_TOKEN, reg_stat7, pool_stat->invalid_mcast_token);
    reg_stat8 = RU_FIELD_SET(0, FPM, STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_stat8, pool_stat->tokens_available_low_wtmk);

    RU_REG_WRITE(0, FPM, STAT1, reg_stat1);
    RU_REG_WRITE(0, FPM, STAT2, reg_stat2);
    RU_REG_WRITE(0, FPM, STAT3, reg_stat3);
    RU_REG_WRITE(0, FPM, STAT4, reg_stat4);
    RU_REG_WRITE(0, FPM, STAT6, reg_stat6);
    RU_REG_WRITE(0, FPM, STAT7, reg_stat7);
    RU_REG_WRITE(0, FPM, STAT8, reg_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_stat_get(fpm_pool_stat *pool_stat)
{
    uint32_t reg_stat1;
    uint32_t reg_stat2;
    uint32_t reg_stat3;
    uint32_t reg_stat4;
    uint32_t reg_stat6;
    uint32_t reg_stat7;
    uint32_t reg_stat8;

#ifdef VALIDATE_PARMS
    if (!pool_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, STAT1, reg_stat1);
    RU_REG_READ(0, FPM, STAT2, reg_stat2);
    RU_REG_READ(0, FPM, STAT3, reg_stat3);
    RU_REG_READ(0, FPM, STAT4, reg_stat4);
    RU_REG_READ(0, FPM, STAT6, reg_stat6);
    RU_REG_READ(0, FPM, STAT7, reg_stat7);
    RU_REG_READ(0, FPM, STAT8, reg_stat8);

    pool_stat->ovrfl = RU_FIELD_GET(0, FPM, STAT1, OVRFL, reg_stat1);
    pool_stat->undrfl = RU_FIELD_GET(0, FPM, STAT1, UNDRFL, reg_stat1);
    pool_stat->pool_full = RU_FIELD_GET(0, FPM, STAT2, POOL_FULL, reg_stat2);
    pool_stat->free_fifo_full = RU_FIELD_GET(0, FPM, STAT2, FREE_FIFO_FULL, reg_stat2);
    pool_stat->free_fifo_empty = RU_FIELD_GET(0, FPM, STAT2, FREE_FIFO_EMPTY, reg_stat2);
    pool_stat->alloc_fifo_full = RU_FIELD_GET(0, FPM, STAT2, ALLOC_FIFO_FULL, reg_stat2);
    pool_stat->alloc_fifo_empty = RU_FIELD_GET(0, FPM, STAT2, ALLOC_FIFO_EMPTY, reg_stat2);
    pool_stat->num_of_tokens_available = RU_FIELD_GET(0, FPM, STAT2, NUM_OF_TOKENS_AVAILABLE, reg_stat2);
    pool_stat->num_of_not_valid_token_frees = RU_FIELD_GET(0, FPM, STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_stat3);
    pool_stat->num_of_not_valid_token_multi = RU_FIELD_GET(0, FPM, STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_stat4);
    pool_stat->invalid_free_token = RU_FIELD_GET(0, FPM, STAT6, INVALID_FREE_TOKEN, reg_stat6);
    pool_stat->invalid_mcast_token = RU_FIELD_GET(0, FPM, STAT7, INVALID_MCAST_TOKEN, reg_stat7);
    pool_stat->tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ctl_set(uint8_t tp_mux_cntrl)
{
    uint32_t reg_ctl = 0;

#ifdef VALIDATE_PARMS
    if ((tp_mux_cntrl >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ctl = RU_FIELD_SET(0, FPM, CTL, TP_MUX_CNTRL, reg_ctl, tp_mux_cntrl);

    RU_REG_WRITE(0, FPM, CTL, reg_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ctl_get(uint8_t *tp_mux_cntrl)
{
    uint32_t reg_ctl;

#ifdef VALIDATE_PARMS
    if (!tp_mux_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CTL, reg_ctl);

    *tp_mux_cntrl = RU_FIELD_GET(0, FPM, CTL, TP_MUX_CNTRL, reg_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_weight_set(uint8_t core_0_alloc_weight, uint8_t core_0_free_weight, uint8_t core_1_alloc_weight, uint8_t core_1_free_weight)
{
    uint32_t reg_weight = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_weight = RU_FIELD_SET(0, FPM, WEIGHT, CORE_0_ALLOC_WEIGHT, reg_weight, core_0_alloc_weight);
    reg_weight = RU_FIELD_SET(0, FPM, WEIGHT, CORE_0_FREE_WEIGHT, reg_weight, core_0_free_weight);
    reg_weight = RU_FIELD_SET(0, FPM, WEIGHT, CORE_1_ALLOC_WEIGHT, reg_weight, core_1_alloc_weight);
    reg_weight = RU_FIELD_SET(0, FPM, WEIGHT, CORE_1_FREE_WEIGHT, reg_weight, core_1_free_weight);

    RU_REG_WRITE(0, FPM, WEIGHT, reg_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_weight_get(uint8_t *core_0_alloc_weight, uint8_t *core_0_free_weight, uint8_t *core_1_alloc_weight, uint8_t *core_1_free_weight)
{
    uint32_t reg_weight;

#ifdef VALIDATE_PARMS
    if (!core_0_alloc_weight || !core_0_free_weight || !core_1_alloc_weight || !core_1_free_weight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, WEIGHT, reg_weight);

    *core_0_alloc_weight = RU_FIELD_GET(0, FPM, WEIGHT, CORE_0_ALLOC_WEIGHT, reg_weight);
    *core_0_free_weight = RU_FIELD_GET(0, FPM, WEIGHT, CORE_0_FREE_WEIGHT, reg_weight);
    *core_1_alloc_weight = RU_FIELD_GET(0, FPM, WEIGHT, CORE_1_ALLOC_WEIGHT, reg_weight);
    *core_1_free_weight = RU_FIELD_GET(0, FPM, WEIGHT, CORE_1_FREE_WEIGHT, reg_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_bb_cfg_set(uint8_t bb_core_sel, bdmf_boolean bb_fast_ack, bdmf_boolean bb_soft_reset)
{
    uint32_t reg_bb_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((bb_core_sel >= _2BITS_MAX_VAL_) ||
       (bb_fast_ack >= _1BITS_MAX_VAL_) ||
       (bb_soft_reset >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bb_cfg = RU_FIELD_SET(0, FPM, BB_CFG, BB_CORE_SEL, reg_bb_cfg, bb_core_sel);
    reg_bb_cfg = RU_FIELD_SET(0, FPM, BB_CFG, BB_FAST_ACK, reg_bb_cfg, bb_fast_ack);
    reg_bb_cfg = RU_FIELD_SET(0, FPM, BB_CFG, BB_SOFT_RESET, reg_bb_cfg, bb_soft_reset);

    RU_REG_WRITE(0, FPM, BB_CFG, reg_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_bb_cfg_get(uint8_t *bb_core_sel, bdmf_boolean *bb_fast_ack, bdmf_boolean *bb_soft_reset)
{
    uint32_t reg_bb_cfg;

#ifdef VALIDATE_PARMS
    if (!bb_core_sel || !bb_fast_ack || !bb_soft_reset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, BB_CFG, reg_bb_cfg);

    *bb_core_sel = RU_FIELD_GET(0, FPM, BB_CFG, BB_CORE_SEL, reg_bb_cfg);
    *bb_fast_ack = RU_FIELD_GET(0, FPM, BB_CFG, BB_FAST_ACK, reg_bb_cfg);
    *bb_soft_reset = RU_FIELD_GET(0, FPM, BB_CFG, BB_SOFT_RESET, reg_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_spare_set(uint32_t spare_bits)
{
    uint32_t reg_spare = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_spare = RU_FIELD_SET(0, FPM, SPARE, SPARE_BITS, reg_spare, spare_bits);

    RU_REG_WRITE(0, FPM, SPARE, reg_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_spare_get(uint32_t *spare_bits)
{
    uint32_t reg_spare;

#ifdef VALIDATE_PARMS
    if (!spare_bits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SPARE, reg_spare);

    *spare_bits = RU_FIELD_GET(0, FPM, SPARE, SPARE_BITS, reg_spare);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_prbs_invalid_gen_set(uint32_t mask, bdmf_boolean enable)
{
    uint32_t reg_prbs_invalid_gen = 0;

#ifdef VALIDATE_PARMS
    if ((mask >= _31BITS_MAX_VAL_) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_prbs_invalid_gen = RU_FIELD_SET(0, FPM, PRBS_INVALID_GEN, MASK, reg_prbs_invalid_gen, mask);
    reg_prbs_invalid_gen = RU_FIELD_SET(0, FPM, PRBS_INVALID_GEN, ENABLE, reg_prbs_invalid_gen, enable);

    RU_REG_WRITE(0, FPM, PRBS_INVALID_GEN, reg_prbs_invalid_gen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_prbs_invalid_gen_get(uint32_t *mask, bdmf_boolean *enable)
{
    uint32_t reg_prbs_invalid_gen;

#ifdef VALIDATE_PARMS
    if (!mask || !enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, PRBS_INVALID_GEN, reg_prbs_invalid_gen);

    *mask = RU_FIELD_GET(0, FPM, PRBS_INVALID_GEN, MASK, reg_prbs_invalid_gen);
    *enable = RU_FIELD_GET(0, FPM, PRBS_INVALID_GEN, ENABLE, reg_prbs_invalid_gen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg1_set(const fpm_cfg1 *cfg1)
{
    uint32_t reg_cfg1 = 0;

#ifdef VALIDATE_PARMS
    if(!cfg1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cfg1->pool_search_mode >= _1BITS_MAX_VAL_) ||
       (cfg1->enable_jumbo_support >= _1BITS_MAX_VAL_) ||
       (cfg1->pool_0_disable >= _1BITS_MAX_VAL_) ||
       (cfg1->pool_1_disable >= _1BITS_MAX_VAL_) ||
       (cfg1->pool_2_disable >= _1BITS_MAX_VAL_) ||
       (cfg1->pool_3_disable >= _1BITS_MAX_VAL_) ||
       (cfg1->mark_en >= _1BITS_MAX_VAL_) ||
       (cfg1->buf_size >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, POOL_SEARCH_MODE, reg_cfg1, cfg1->pool_search_mode);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, ENABLE_JUMBO_SUPPORT, reg_cfg1, cfg1->enable_jumbo_support);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, POOL_0_DISABLE, reg_cfg1, cfg1->pool_0_disable);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, POOL_1_DISABLE, reg_cfg1, cfg1->pool_1_disable);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, POOL_2_DISABLE, reg_cfg1, cfg1->pool_2_disable);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, POOL_3_DISABLE, reg_cfg1, cfg1->pool_3_disable);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, MARK_EN, reg_cfg1, cfg1->mark_en);
    reg_cfg1 = RU_FIELD_SET(0, FPM, CFG1, BUF_SIZE, reg_cfg1, cfg1->buf_size);

    RU_REG_WRITE(0, FPM, CFG1, reg_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg1_get(fpm_cfg1 *cfg1)
{
    uint32_t reg_cfg1;

#ifdef VALIDATE_PARMS
    if (!cfg1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CFG1, reg_cfg1);

    cfg1->pool_search_mode = RU_FIELD_GET(0, FPM, CFG1, POOL_SEARCH_MODE, reg_cfg1);
    cfg1->enable_jumbo_support = RU_FIELD_GET(0, FPM, CFG1, ENABLE_JUMBO_SUPPORT, reg_cfg1);
    cfg1->pool_0_disable = RU_FIELD_GET(0, FPM, CFG1, POOL_0_DISABLE, reg_cfg1);
    cfg1->pool_1_disable = RU_FIELD_GET(0, FPM, CFG1, POOL_1_DISABLE, reg_cfg1);
    cfg1->pool_2_disable = RU_FIELD_GET(0, FPM, CFG1, POOL_2_DISABLE, reg_cfg1);
    cfg1->pool_3_disable = RU_FIELD_GET(0, FPM, CFG1, POOL_3_DISABLE, reg_cfg1);
    cfg1->mark_en = RU_FIELD_GET(0, FPM, CFG1, MARK_EN, reg_cfg1);
    cfg1->buf_size = RU_FIELD_GET(0, FPM, CFG1, BUF_SIZE, reg_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg2_set(uint32_t pool_base_address)
{
    uint32_t reg_cfg2 = 0;

#ifdef VALIDATE_PARMS
    if ((pool_base_address >= _30BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg2 = RU_FIELD_SET(0, FPM, CFG2, POOL_BASE_ADDRESS, reg_cfg2, pool_base_address);

    RU_REG_WRITE(0, FPM, CFG2, reg_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg2_get(uint32_t *pool_base_address)
{
    uint32_t reg_cfg2;

#ifdef VALIDATE_PARMS
    if (!pool_base_address)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CFG2, reg_cfg2);

    *pool_base_address = RU_FIELD_GET(0, FPM, CFG2, POOL_BASE_ADDRESS, reg_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_intr_msk_set(const fpm_intr_msk *intr_msk)
{
    uint32_t reg_intr_msk = 0;

#ifdef VALIDATE_PARMS
    if(!intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_intr_msk, intr_msk->alloc_fifo_full_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, FREE_FIFO_FULL_MSK, reg_intr_msk, intr_msk->free_fifo_full_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, POOL_FULL_MSK, reg_intr_msk, intr_msk->pool_full_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_intr_msk, intr_msk->free_token_no_valid_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_intr_msk, intr_msk->free_token_index_out_of_range_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_intr_msk, intr_msk->multi_token_no_valid_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_intr_msk, intr_msk->multi_token_index_out_of_range_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_intr_msk, intr_msk->pool_dis_free_multi_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, MEMORY_CORRUPT_MSK, reg_intr_msk, intr_msk->memory_corrupt_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, XOFF_MSK, reg_intr_msk, intr_msk->xoff_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, XON_MSK, reg_intr_msk, intr_msk->xon_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_intr_msk, intr_msk->illegal_address_access_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_intr_msk, intr_msk->illegal_alloc_request_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_intr_msk, intr_msk->expired_token_det_msk);
    reg_intr_msk = RU_FIELD_SET(0, FPM, INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_intr_msk, intr_msk->expired_token_recov_msk);

    RU_REG_WRITE(0, FPM, INTR_MSK, reg_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_intr_msk_get(fpm_intr_msk *intr_msk)
{
    uint32_t reg_intr_msk;

#ifdef VALIDATE_PARMS
    if (!intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, INTR_MSK, reg_intr_msk);

    intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_intr_msk);
    intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, INTR_MSK, FREE_FIFO_FULL_MSK, reg_intr_msk);
    intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, INTR_MSK, POOL_FULL_MSK, reg_intr_msk);
    intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_intr_msk);
    intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_intr_msk);
    intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_intr_msk);
    intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_intr_msk);
    intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_intr_msk);
    intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, INTR_MSK, MEMORY_CORRUPT_MSK, reg_intr_msk);
    intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, INTR_MSK, XOFF_MSK, reg_intr_msk);
    intr_msk->xon_msk = RU_FIELD_GET(0, FPM, INTR_MSK, XON_MSK, reg_intr_msk);
    intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_intr_msk);
    intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_intr_msk);
    intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_intr_msk);
    intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_intr_sts_set(const fpm_intr_sts *intr_sts)
{
    uint32_t reg_intr_sts = 0;

#ifdef VALIDATE_PARMS
    if(!intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, ALLOC_FIFO_FULL_STS, reg_intr_sts, intr_sts->alloc_fifo_full_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, FREE_FIFO_FULL_STS, reg_intr_sts, intr_sts->free_fifo_full_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, POOL_FULL_STS, reg_intr_sts, intr_sts->pool_full_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_intr_sts, intr_sts->free_token_no_valid_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_intr_sts, intr_sts->free_token_index_out_of_range_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_intr_sts, intr_sts->multi_token_no_valid_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_intr_sts, intr_sts->multi_token_index_out_of_range_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_intr_sts, intr_sts->pool_dis_free_multi_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, MEMORY_CORRUPT_STS, reg_intr_sts, intr_sts->memory_corrupt_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, XOFF_STATE_STS, reg_intr_sts, intr_sts->xoff_state_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, XON_STATE_STS, reg_intr_sts, intr_sts->xon_state_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_intr_sts, intr_sts->illegal_address_access_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_intr_sts, intr_sts->illegal_alloc_request_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, EXPIRED_TOKEN_DET_STS, reg_intr_sts, intr_sts->expired_token_det_sts);
    reg_intr_sts = RU_FIELD_SET(0, FPM, INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_intr_sts, intr_sts->expired_token_recov_sts);

    RU_REG_WRITE(0, FPM, INTR_STS, reg_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_intr_sts_get(fpm_intr_sts *intr_sts)
{
    uint32_t reg_intr_sts;

#ifdef VALIDATE_PARMS
    if (!intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, INTR_STS, reg_intr_sts);

    intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, INTR_STS, ALLOC_FIFO_FULL_STS, reg_intr_sts);
    intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, INTR_STS, FREE_FIFO_FULL_STS, reg_intr_sts);
    intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, INTR_STS, POOL_FULL_STS, reg_intr_sts);
    intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_intr_sts);
    intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_intr_sts);
    intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_intr_sts);
    intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_intr_sts);
    intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_intr_sts);
    intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, INTR_STS, MEMORY_CORRUPT_STS, reg_intr_sts);
    intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, INTR_STS, XOFF_STATE_STS, reg_intr_sts);
    intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, INTR_STS, XON_STATE_STS, reg_intr_sts);
    intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_intr_sts);
    intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_intr_sts);
    intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, INTR_STS, EXPIRED_TOKEN_DET_STS, reg_intr_sts);
    intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_stall_msk_set(const fpm_stall_msk *stall_msk)
{
    uint32_t reg_stall_msk = 0;

#ifdef VALIDATE_PARMS
    if(!stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_stall_msk = RU_FIELD_SET(0, FPM, STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_stall_msk, stall_msk->free_token_no_valid_stall_msk);
    reg_stall_msk = RU_FIELD_SET(0, FPM, STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_stall_msk, stall_msk->free_token_index_out_of_range_stall_msk);
    reg_stall_msk = RU_FIELD_SET(0, FPM, STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_stall_msk, stall_msk->multi_token_no_valid_stall_msk);
    reg_stall_msk = RU_FIELD_SET(0, FPM, STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_stall_msk, stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_stall_msk = RU_FIELD_SET(0, FPM, STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_stall_msk, stall_msk->memory_corrupt_stall_msk);

    RU_REG_WRITE(0, FPM, STALL_MSK, reg_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_stall_msk_get(fpm_stall_msk *stall_msk)
{
    uint32_t reg_stall_msk;

#ifdef VALIDATE_PARMS
    if (!stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, STALL_MSK, reg_stall_msk);

    stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_stall_msk);
    stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_stall_msk);
    stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_stall_msk);
    stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_stall_msk);
    stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg3_set(uint16_t halt_alloc_timer)
{
    uint32_t reg_cfg3 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_cfg3 = RU_FIELD_SET(0, FPM, CFG3, HALT_ALLOC_TIMER, reg_cfg3, halt_alloc_timer);

    RU_REG_WRITE(0, FPM, CFG3, reg_cfg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cfg3_get(uint16_t *halt_alloc_timer)
{
    uint32_t reg_cfg3;

#ifdef VALIDATE_PARMS
    if (!halt_alloc_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CFG3, reg_cfg3);

    *halt_alloc_timer = RU_FIELD_GET(0, FPM, CFG3, HALT_ALLOC_TIMER, reg_cfg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_stat5_set(uint32_t mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_stat5 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_stat5 = RU_FIELD_SET(0, FPM, STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_stat5, mem_corrupt_sts_related_alloc_token);

    RU_REG_WRITE(0, FPM, STAT5, reg_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_stat5_get(uint32_t *mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_stat5;

#ifdef VALIDATE_PARMS
    if (!mem_corrupt_sts_related_alloc_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, STAT5, reg_stat5);

    *mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_xon_xoff_cfg_set(uint16_t xoff_threshold, uint16_t xon_threshold)
{
    uint32_t reg_xon_xoff_cfg = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_xon_xoff_cfg = RU_FIELD_SET(0, FPM, XON_XOFF_CFG, XOFF_THRESHOLD, reg_xon_xoff_cfg, xoff_threshold);
    reg_xon_xoff_cfg = RU_FIELD_SET(0, FPM, XON_XOFF_CFG, XON_THRESHOLD, reg_xon_xoff_cfg, xon_threshold);

    RU_REG_WRITE(0, FPM, XON_XOFF_CFG, reg_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_xon_xoff_cfg_get(uint16_t *xoff_threshold, uint16_t *xon_threshold)
{
    uint32_t reg_xon_xoff_cfg;

#ifdef VALIDATE_PARMS
    if (!xoff_threshold || !xon_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, XON_XOFF_CFG, reg_xon_xoff_cfg);

    *xoff_threshold = RU_FIELD_GET(0, FPM, XON_XOFF_CFG, XOFF_THRESHOLD, reg_xon_xoff_cfg);
    *xon_threshold = RU_FIELD_GET(0, FPM, XON_XOFF_CFG, XON_THRESHOLD, reg_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_not_empty_cfg_set(uint8_t not_empty_threshold)
{
    uint32_t reg_not_empty_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((not_empty_threshold >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_not_empty_cfg = RU_FIELD_SET(0, FPM, NOT_EMPTY_CFG, NOT_EMPTY_THRESHOLD, reg_not_empty_cfg, not_empty_threshold);

    RU_REG_WRITE(0, FPM, NOT_EMPTY_CFG, reg_not_empty_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_not_empty_cfg_get(uint8_t *not_empty_threshold)
{
    uint32_t reg_not_empty_cfg;

#ifdef VALIDATE_PARMS
    if (!not_empty_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, NOT_EMPTY_CFG, reg_not_empty_cfg);

    *not_empty_threshold = RU_FIELD_GET(0, FPM, NOT_EMPTY_CFG, NOT_EMPTY_THRESHOLD, reg_not_empty_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_ctl_set(const fpm_token_recover_ctl *token_recover_ctl)
{
    uint32_t reg_token_recover_ctl = 0;

#ifdef VALIDATE_PARMS
    if(!token_recover_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((token_recover_ctl->token_recover_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->single_pass_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->token_remark_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->token_reclaim_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->force_token_reclaim >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->clr_expired_token_count >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->clr_recovered_token_count >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECOVER_ENA, reg_token_recover_ctl, token_recover_ctl->token_recover_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, SINGLE_PASS_ENA, reg_token_recover_ctl, token_recover_ctl->single_pass_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_REMARK_ENA, reg_token_recover_ctl, token_recover_ctl->token_remark_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECLAIM_ENA, reg_token_recover_ctl, token_recover_ctl->token_reclaim_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, FORCE_TOKEN_RECLAIM, reg_token_recover_ctl, token_recover_ctl->force_token_reclaim);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, CLR_EXPIRED_TOKEN_COUNT, reg_token_recover_ctl, token_recover_ctl->clr_expired_token_count);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, CLR_RECOVERED_TOKEN_COUNT, reg_token_recover_ctl, token_recover_ctl->clr_recovered_token_count);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_CTL, reg_token_recover_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_ctl_get(fpm_token_recover_ctl *token_recover_ctl)
{
    uint32_t reg_token_recover_ctl;

#ifdef VALIDATE_PARMS
    if (!token_recover_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_CTL, reg_token_recover_ctl);

    token_recover_ctl->token_recover_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECOVER_ENA, reg_token_recover_ctl);
    token_recover_ctl->single_pass_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, SINGLE_PASS_ENA, reg_token_recover_ctl);
    token_recover_ctl->token_remark_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_REMARK_ENA, reg_token_recover_ctl);
    token_recover_ctl->token_reclaim_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECLAIM_ENA, reg_token_recover_ctl);
    token_recover_ctl->force_token_reclaim = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, FORCE_TOKEN_RECLAIM, reg_token_recover_ctl);
    token_recover_ctl->clr_expired_token_count = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, CLR_EXPIRED_TOKEN_COUNT, reg_token_recover_ctl);
    token_recover_ctl->clr_recovered_token_count = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, CLR_RECOVERED_TOKEN_COUNT, reg_token_recover_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_short_aging_timer_set(uint32_t timer)
{
    uint32_t reg_short_aging_timer = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_short_aging_timer = RU_FIELD_SET(0, FPM, SHORT_AGING_TIMER, TIMER, reg_short_aging_timer, timer);

    RU_REG_WRITE(0, FPM, SHORT_AGING_TIMER, reg_short_aging_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_short_aging_timer_get(uint32_t *timer)
{
    uint32_t reg_short_aging_timer;

#ifdef VALIDATE_PARMS
    if (!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SHORT_AGING_TIMER, reg_short_aging_timer);

    *timer = RU_FIELD_GET(0, FPM, SHORT_AGING_TIMER, TIMER, reg_short_aging_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_long_aging_timer_set(uint32_t timer)
{
    uint32_t reg_long_aging_timer = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_long_aging_timer = RU_FIELD_SET(0, FPM, LONG_AGING_TIMER, TIMER, reg_long_aging_timer, timer);

    RU_REG_WRITE(0, FPM, LONG_AGING_TIMER, reg_long_aging_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_long_aging_timer_get(uint32_t *timer)
{
    uint32_t reg_long_aging_timer;

#ifdef VALIDATE_PARMS
    if (!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, LONG_AGING_TIMER, reg_long_aging_timer);

    *timer = RU_FIELD_GET(0, FPM, LONG_AGING_TIMER, TIMER, reg_long_aging_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cache_recycle_timer_set(uint16_t recycle_timer)
{
    uint32_t reg_cache_recycle_timer = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_cache_recycle_timer = RU_FIELD_SET(0, FPM, CACHE_RECYCLE_TIMER, RECYCLE_TIMER, reg_cache_recycle_timer, recycle_timer);

    RU_REG_WRITE(0, FPM, CACHE_RECYCLE_TIMER, reg_cache_recycle_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_cache_recycle_timer_get(uint16_t *recycle_timer)
{
    uint32_t reg_cache_recycle_timer;

#ifdef VALIDATE_PARMS
    if (!recycle_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CACHE_RECYCLE_TIMER, reg_cache_recycle_timer);

    *recycle_timer = RU_FIELD_GET(0, FPM, CACHE_RECYCLE_TIMER, RECYCLE_TIMER, reg_cache_recycle_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_expired_token_count_get(uint32_t *count)
{
    uint32_t reg_expired_token_count;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, EXPIRED_TOKEN_COUNT, reg_expired_token_count);

    *count = RU_FIELD_GET(0, FPM, EXPIRED_TOKEN_COUNT, COUNT, reg_expired_token_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_recovered_token_count_get(uint32_t *count)
{
    uint32_t reg_recovered_token_count;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, RECOVERED_TOKEN_COUNT, reg_recovered_token_count);

    *count = RU_FIELD_GET(0, FPM, RECOVERED_TOKEN_COUNT, COUNT, reg_recovered_token_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_set(uint16_t end_index, uint16_t start_index)
{
    uint32_t reg_token_recover_start_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_token_recover_start_end = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END, END_INDEX, reg_token_recover_start_end, end_index);
    reg_token_recover_start_end = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END, START_INDEX, reg_token_recover_start_end, start_index);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_START_END, reg_token_recover_start_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_get(uint16_t *end_index, uint16_t *start_index)
{
    uint32_t reg_token_recover_start_end;

#ifdef VALIDATE_PARMS
    if (!end_index || !start_index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_START_END, reg_token_recover_start_end);

    *end_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END, END_INDEX, reg_token_recover_start_end);
    *start_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END, START_INDEX, reg_token_recover_start_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_0_alloc_dealloc_set(uint16_t token_size, uint32_t token_index, uint8_t size_id)
{
    uint32_t reg_size_0_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _11BITS_MAX_VAL_) ||
       (token_index >= _19BITS_MAX_VAL_) ||
       (size_id >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_size_0_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_0_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_0_alloc_dealloc, token_size);
    reg_size_0_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_0_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_0_alloc_dealloc, token_index);
    reg_size_0_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_0_ALLOC_DEALLOC, SIZE_ID, reg_size_0_alloc_dealloc, size_id);

    RU_REG_WRITE(0, FPM, SIZE_0_ALLOC_DEALLOC, reg_size_0_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_0_alloc_dealloc_get(uint16_t *token_size, uint32_t *token_index, uint8_t *size_id)
{
    uint32_t reg_size_0_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !size_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SIZE_0_ALLOC_DEALLOC, reg_size_0_alloc_dealloc);

    *token_size = RU_FIELD_GET(0, FPM, SIZE_0_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_0_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, SIZE_0_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_0_alloc_dealloc);
    *size_id = RU_FIELD_GET(0, FPM, SIZE_0_ALLOC_DEALLOC, SIZE_ID, reg_size_0_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_1_alloc_dealloc_set(uint16_t token_size, uint32_t token_index, uint8_t size_id)
{
    uint32_t reg_size_1_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _10BITS_MAX_VAL_) ||
       (token_index >= _20BITS_MAX_VAL_) ||
       (size_id >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_size_1_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_1_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_1_alloc_dealloc, token_size);
    reg_size_1_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_1_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_1_alloc_dealloc, token_index);
    reg_size_1_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_1_ALLOC_DEALLOC, SIZE_ID, reg_size_1_alloc_dealloc, size_id);

    RU_REG_WRITE(0, FPM, SIZE_1_ALLOC_DEALLOC, reg_size_1_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_1_alloc_dealloc_get(uint16_t *token_size, uint32_t *token_index, uint8_t *size_id)
{
    uint32_t reg_size_1_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !size_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SIZE_1_ALLOC_DEALLOC, reg_size_1_alloc_dealloc);

    *token_size = RU_FIELD_GET(0, FPM, SIZE_1_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_1_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, SIZE_1_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_1_alloc_dealloc);
    *size_id = RU_FIELD_GET(0, FPM, SIZE_1_ALLOC_DEALLOC, SIZE_ID, reg_size_1_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_2_alloc_dealloc_set(uint16_t token_size, uint32_t token_index, uint8_t size_id)
{
    uint32_t reg_size_2_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _9BITS_MAX_VAL_) ||
       (token_index >= _21BITS_MAX_VAL_) ||
       (size_id >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_size_2_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_2_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_2_alloc_dealloc, token_size);
    reg_size_2_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_2_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_2_alloc_dealloc, token_index);
    reg_size_2_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_2_ALLOC_DEALLOC, SIZE_ID, reg_size_2_alloc_dealloc, size_id);

    RU_REG_WRITE(0, FPM, SIZE_2_ALLOC_DEALLOC, reg_size_2_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_2_alloc_dealloc_get(uint16_t *token_size, uint32_t *token_index, uint8_t *size_id)
{
    uint32_t reg_size_2_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !size_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SIZE_2_ALLOC_DEALLOC, reg_size_2_alloc_dealloc);

    *token_size = RU_FIELD_GET(0, FPM, SIZE_2_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_2_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, SIZE_2_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_2_alloc_dealloc);
    *size_id = RU_FIELD_GET(0, FPM, SIZE_2_ALLOC_DEALLOC, SIZE_ID, reg_size_2_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_3_alloc_dealloc_set(uint8_t token_size, uint32_t token_index, uint8_t size_id)
{
    uint32_t reg_size_3_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_index >= _22BITS_MAX_VAL_) ||
       (size_id >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_size_3_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_3_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_3_alloc_dealloc, token_size);
    reg_size_3_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_3_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_3_alloc_dealloc, token_index);
    reg_size_3_alloc_dealloc = RU_FIELD_SET(0, FPM, SIZE_3_ALLOC_DEALLOC, SIZE_ID, reg_size_3_alloc_dealloc, size_id);

    RU_REG_WRITE(0, FPM, SIZE_3_ALLOC_DEALLOC, reg_size_3_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_size_3_alloc_dealloc_get(uint8_t *token_size, uint32_t *token_index, uint8_t *size_id)
{
    uint32_t reg_size_3_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !size_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, SIZE_3_ALLOC_DEALLOC, reg_size_3_alloc_dealloc);

    *token_size = RU_FIELD_GET(0, FPM, SIZE_3_ALLOC_DEALLOC, TOKEN_SIZE, reg_size_3_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, SIZE_3_ALLOC_DEALLOC, TOKEN_INDEX, reg_size_3_alloc_dealloc);
    *size_id = RU_FIELD_GET(0, FPM, SIZE_3_ALLOC_DEALLOC, SIZE_ID, reg_size_3_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_multi_set(const fpm_multi *multi)
{
    uint32_t reg_multi = 0;

#ifdef VALIDATE_PARMS
    if(!multi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((multi->token_multi >= _7BITS_MAX_VAL_) ||
       (multi->update_type >= _1BITS_MAX_VAL_) ||
       (multi->token_index >= _17BITS_MAX_VAL_) ||
       (multi->ddr >= _1BITS_MAX_VAL_) ||
       (multi->token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_multi = RU_FIELD_SET(0, FPM, MULTI, TOKEN_MULTI, reg_multi, multi->token_multi);
    reg_multi = RU_FIELD_SET(0, FPM, MULTI, UPDATE_TYPE, reg_multi, multi->update_type);
    reg_multi = RU_FIELD_SET(0, FPM, MULTI, TOKEN_INDEX, reg_multi, multi->token_index);
    reg_multi = RU_FIELD_SET(0, FPM, MULTI, DDR, reg_multi, multi->ddr);
    reg_multi = RU_FIELD_SET(0, FPM, MULTI, TOKEN_VALID, reg_multi, multi->token_valid);

    RU_REG_WRITE(0, FPM, MULTI, reg_multi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_1_set(uint32_t search1, uint16_t data)
{
    uint32_t reg_search_data_1 = 0;

#ifdef VALIDATE_PARMS
    if ((search1 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, DATA, reg_search_data_1, data);

    RU_REG_RAM_WRITE(0, search1, FPM, SEARCH_DATA_1, reg_search_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_1_get(uint32_t search1, uint16_t *data)
{
    uint32_t reg_search_data_1;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search1, FPM, SEARCH_DATA_1, reg_search_data_1);

    *data = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, DATA, reg_search_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_2_set(uint32_t search1, uint32_t data)
{
    uint32_t reg_search_data_2 = 0;

#ifdef VALIDATE_PARMS
    if ((search1 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, DATA, reg_search_data_2, data);

    RU_REG_RAM_WRITE(0, search1, FPM, SEARCH_DATA_2, reg_search_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_2_get(uint32_t search1, uint32_t *data)
{
    uint32_t reg_search_data_2;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search1, FPM, SEARCH_DATA_2, reg_search_data_2);

    *data = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, DATA, reg_search_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_3_set(uint32_t search3, uint16_t data)
{
    uint32_t reg_search_data_3 = 0;

#ifdef VALIDATE_PARMS
    if ((search3 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, DATA, reg_search_data_3, data);

    RU_REG_RAM_WRITE(0, search3, FPM, SEARCH_DATA_3, reg_search_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_3_get(uint32_t search3, uint16_t *data)
{
    uint32_t reg_search_data_3;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search3 >= 529))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search3, FPM, SEARCH_DATA_3, reg_search_data_3);

    *data = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, DATA, reg_search_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_multicast_data_set(uint32_t multicast_data, uint32_t data)
{
    uint32_t reg_multicast_data = 0;

#ifdef VALIDATE_PARMS
    if ((multicast_data >= 16384))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_multicast_data = RU_FIELD_SET(0, FPM, MULTICAST_DATA, DATA, reg_multicast_data, data);

    RU_REG_RAM_WRITE(0, multicast_data, FPM, MULTICAST_DATA, reg_multicast_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_multicast_data_get(uint32_t multicast_data, uint32_t *data)
{
    uint32_t reg_multicast_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((multicast_data >= 16384))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, multicast_data, FPM, MULTICAST_DATA, reg_multicast_data);

    *data = RU_FIELD_GET(0, FPM, MULTICAST_DATA, DATA, reg_multicast_data);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_ctl,
    bdmf_address_weight,
    bdmf_address_bb_cfg,
    bdmf_address_spare,
    bdmf_address_prbs_invalid_gen,
    bdmf_address_ctl_1,
    bdmf_address_cfg1,
    bdmf_address_cfg2,
    bdmf_address_intr_msk,
    bdmf_address_intr_sts,
    bdmf_address_stall_msk,
    bdmf_address_cfg3,
    bdmf_address_stat1,
    bdmf_address_stat2,
    bdmf_address_stat3,
    bdmf_address_stat4,
    bdmf_address_stat5,
    bdmf_address_stat6,
    bdmf_address_stat7,
    bdmf_address_stat8,
    bdmf_address_xon_xoff_cfg,
    bdmf_address_not_empty_cfg,
    bdmf_address_token_recover_ctl,
    bdmf_address_short_aging_timer,
    bdmf_address_long_aging_timer,
    bdmf_address_cache_recycle_timer,
    bdmf_address_expired_token_count,
    bdmf_address_recovered_token_count,
    bdmf_address_token_recover_start_end,
    bdmf_address_size_0_alloc_dealloc,
    bdmf_address_size_1_alloc_dealloc,
    bdmf_address_size_2_alloc_dealloc,
    bdmf_address_size_3_alloc_dealloc,
    bdmf_address_multi,
    bdmf_address_search_data_0,
    bdmf_address_search_data_1,
    bdmf_address_search_data_2,
    bdmf_address_search_data_3,
    bdmf_address_multicast_data,
}
bdmf_address;

static int ag_drv_fpm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_fpm_init_mem:
        ag_err = ag_drv_fpm_init_mem_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool_en:
        ag_err = ag_drv_fpm_pool_en_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool_stat:
    {
        fpm_pool_stat pool_stat = { .ovrfl = parm[1].value.unumber, .undrfl = parm[2].value.unumber, .pool_full = parm[3].value.unumber, .free_fifo_full = parm[4].value.unumber, .free_fifo_empty = parm[5].value.unumber, .alloc_fifo_full = parm[6].value.unumber, .alloc_fifo_empty = parm[7].value.unumber, .num_of_tokens_available = parm[8].value.unumber, .num_of_not_valid_token_frees = parm[9].value.unumber, .num_of_not_valid_token_multi = parm[10].value.unumber, .invalid_free_token = parm[11].value.unumber, .invalid_mcast_token = parm[12].value.unumber, .tokens_available_low_wtmk = parm[13].value.unumber};
        ag_err = ag_drv_fpm_pool_stat_set(&pool_stat);
        break;
    }
    case cli_fpm_ctl:
        ag_err = ag_drv_fpm_ctl_set(parm[1].value.unumber);
        break;
    case cli_fpm_weight:
        ag_err = ag_drv_fpm_weight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_bb_cfg:
        ag_err = ag_drv_fpm_bb_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_spare:
        ag_err = ag_drv_fpm_spare_set(parm[1].value.unumber);
        break;
    case cli_fpm_prbs_invalid_gen:
        ag_err = ag_drv_fpm_prbs_invalid_gen_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_cfg1:
    {
        fpm_cfg1 cfg1 = { .pool_search_mode = parm[1].value.unumber, .enable_jumbo_support = parm[2].value.unumber, .pool_0_disable = parm[3].value.unumber, .pool_1_disable = parm[4].value.unumber, .pool_2_disable = parm[5].value.unumber, .pool_3_disable = parm[6].value.unumber, .mark_en = parm[7].value.unumber, .buf_size = parm[8].value.unumber};
        ag_err = ag_drv_fpm_cfg1_set(&cfg1);
        break;
    }
    case cli_fpm_cfg2:
        ag_err = ag_drv_fpm_cfg2_set(parm[1].value.unumber);
        break;
    case cli_fpm_intr_msk:
    {
        fpm_intr_msk intr_msk = { .alloc_fifo_full_msk = parm[1].value.unumber, .free_fifo_full_msk = parm[2].value.unumber, .pool_full_msk = parm[3].value.unumber, .free_token_no_valid_msk = parm[4].value.unumber, .free_token_index_out_of_range_msk = parm[5].value.unumber, .multi_token_no_valid_msk = parm[6].value.unumber, .multi_token_index_out_of_range_msk = parm[7].value.unumber, .pool_dis_free_multi_msk = parm[8].value.unumber, .memory_corrupt_msk = parm[9].value.unumber, .xoff_msk = parm[10].value.unumber, .xon_msk = parm[11].value.unumber, .illegal_address_access_msk = parm[12].value.unumber, .illegal_alloc_request_msk = parm[13].value.unumber, .expired_token_det_msk = parm[14].value.unumber, .expired_token_recov_msk = parm[15].value.unumber};
        ag_err = ag_drv_fpm_intr_msk_set(&intr_msk);
        break;
    }
    case cli_fpm_intr_sts:
    {
        fpm_intr_sts intr_sts = { .alloc_fifo_full_sts = parm[1].value.unumber, .free_fifo_full_sts = parm[2].value.unumber, .pool_full_sts = parm[3].value.unumber, .free_token_no_valid_sts = parm[4].value.unumber, .free_token_index_out_of_range_sts = parm[5].value.unumber, .multi_token_no_valid_sts = parm[6].value.unumber, .multi_token_index_out_of_range_sts = parm[7].value.unumber, .pool_dis_free_multi_sts = parm[8].value.unumber, .memory_corrupt_sts = parm[9].value.unumber, .xoff_state_sts = parm[10].value.unumber, .xon_state_sts = parm[11].value.unumber, .illegal_address_access_sts = parm[12].value.unumber, .illegal_alloc_request_sts = parm[13].value.unumber, .expired_token_det_sts = parm[14].value.unumber, .expired_token_recov_sts = parm[15].value.unumber};
        ag_err = ag_drv_fpm_intr_sts_set(&intr_sts);
        break;
    }
    case cli_fpm_stall_msk:
    {
        fpm_stall_msk stall_msk = { .free_token_no_valid_stall_msk = parm[1].value.unumber, .free_token_index_out_of_range_stall_msk = parm[2].value.unumber, .multi_token_no_valid_stall_msk = parm[3].value.unumber, .multi_token_index_out_of_range_stall_msk = parm[4].value.unumber, .memory_corrupt_stall_msk = parm[5].value.unumber};
        ag_err = ag_drv_fpm_stall_msk_set(&stall_msk);
        break;
    }
    case cli_fpm_cfg3:
        ag_err = ag_drv_fpm_cfg3_set(parm[1].value.unumber);
        break;
    case cli_fpm_stat5:
        ag_err = ag_drv_fpm_stat5_set(parm[1].value.unumber);
        break;
    case cli_fpm_xon_xoff_cfg:
        ag_err = ag_drv_fpm_xon_xoff_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_not_empty_cfg:
        ag_err = ag_drv_fpm_not_empty_cfg_set(parm[1].value.unumber);
        break;
    case cli_fpm_token_recover_ctl:
    {
        fpm_token_recover_ctl token_recover_ctl = { .token_recover_ena = parm[1].value.unumber, .single_pass_ena = parm[2].value.unumber, .token_remark_ena = parm[3].value.unumber, .token_reclaim_ena = parm[4].value.unumber, .force_token_reclaim = parm[5].value.unumber, .clr_expired_token_count = parm[6].value.unumber, .clr_recovered_token_count = parm[7].value.unumber};
        ag_err = ag_drv_fpm_token_recover_ctl_set(&token_recover_ctl);
        break;
    }
    case cli_fpm_short_aging_timer:
        ag_err = ag_drv_fpm_short_aging_timer_set(parm[1].value.unumber);
        break;
    case cli_fpm_long_aging_timer:
        ag_err = ag_drv_fpm_long_aging_timer_set(parm[1].value.unumber);
        break;
    case cli_fpm_cache_recycle_timer:
        ag_err = ag_drv_fpm_cache_recycle_timer_set(parm[1].value.unumber);
        break;
    case cli_fpm_token_recover_start_end:
        ag_err = ag_drv_fpm_token_recover_start_end_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_size_0_alloc_dealloc:
        ag_err = ag_drv_fpm_size_0_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_size_1_alloc_dealloc:
        ag_err = ag_drv_fpm_size_1_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_size_2_alloc_dealloc:
        ag_err = ag_drv_fpm_size_2_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_size_3_alloc_dealloc:
        ag_err = ag_drv_fpm_size_3_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_multi:
    {
        fpm_multi multi = { .token_multi = parm[1].value.unumber, .update_type = parm[2].value.unumber, .token_index = parm[3].value.unumber, .ddr = parm[4].value.unumber, .token_valid = parm[5].value.unumber};
        ag_err = ag_drv_fpm_multi_set(&multi);
        break;
    }
    case cli_fpm_search_data_1:
        ag_err = ag_drv_fpm_search_data_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_search_data_2:
        ag_err = ag_drv_fpm_search_data_2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_search_data_3:
        ag_err = ag_drv_fpm_search_data_3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_multicast_data:
        ag_err = ag_drv_fpm_multicast_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_fpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_fpm_init_mem:
    {
        bdmf_boolean init_mem;
        ag_err = ag_drv_fpm_init_mem_get(&init_mem);
        bdmf_session_print(session, "init_mem = %u = 0x%x\n", init_mem, init_mem);
        break;
    }
    case cli_fpm_pool_en:
    {
        bdmf_boolean pool_enable;
        ag_err = ag_drv_fpm_pool_en_get(&pool_enable);
        bdmf_session_print(session, "pool_enable = %u = 0x%x\n", pool_enable, pool_enable);
        break;
    }
    case cli_fpm_pool_stat:
    {
        fpm_pool_stat pool_stat;
        ag_err = ag_drv_fpm_pool_stat_get(&pool_stat);
        bdmf_session_print(session, "ovrfl = %u = 0x%x\n", pool_stat.ovrfl, pool_stat.ovrfl);
        bdmf_session_print(session, "undrfl = %u = 0x%x\n", pool_stat.undrfl, pool_stat.undrfl);
        bdmf_session_print(session, "pool_full = %u = 0x%x\n", pool_stat.pool_full, pool_stat.pool_full);
        bdmf_session_print(session, "free_fifo_full = %u = 0x%x\n", pool_stat.free_fifo_full, pool_stat.free_fifo_full);
        bdmf_session_print(session, "free_fifo_empty = %u = 0x%x\n", pool_stat.free_fifo_empty, pool_stat.free_fifo_empty);
        bdmf_session_print(session, "alloc_fifo_full = %u = 0x%x\n", pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_full);
        bdmf_session_print(session, "alloc_fifo_empty = %u = 0x%x\n", pool_stat.alloc_fifo_empty, pool_stat.alloc_fifo_empty);
        bdmf_session_print(session, "num_of_tokens_available = %u = 0x%x\n", pool_stat.num_of_tokens_available, pool_stat.num_of_tokens_available);
        bdmf_session_print(session, "num_of_not_valid_token_frees = %u = 0x%x\n", pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_frees);
        bdmf_session_print(session, "num_of_not_valid_token_multi = %u = 0x%x\n", pool_stat.num_of_not_valid_token_multi, pool_stat.num_of_not_valid_token_multi);
        bdmf_session_print(session, "invalid_free_token = %u = 0x%x\n", pool_stat.invalid_free_token, pool_stat.invalid_free_token);
        bdmf_session_print(session, "invalid_mcast_token = %u = 0x%x\n", pool_stat.invalid_mcast_token, pool_stat.invalid_mcast_token);
        bdmf_session_print(session, "tokens_available_low_wtmk = %u = 0x%x\n", pool_stat.tokens_available_low_wtmk, pool_stat.tokens_available_low_wtmk);
        break;
    }
    case cli_fpm_ctl:
    {
        uint8_t tp_mux_cntrl;
        ag_err = ag_drv_fpm_ctl_get(&tp_mux_cntrl);
        bdmf_session_print(session, "tp_mux_cntrl = %u = 0x%x\n", tp_mux_cntrl, tp_mux_cntrl);
        break;
    }
    case cli_fpm_weight:
    {
        uint8_t core_0_alloc_weight;
        uint8_t core_0_free_weight;
        uint8_t core_1_alloc_weight;
        uint8_t core_1_free_weight;
        ag_err = ag_drv_fpm_weight_get(&core_0_alloc_weight, &core_0_free_weight, &core_1_alloc_weight, &core_1_free_weight);
        bdmf_session_print(session, "core_0_alloc_weight = %u = 0x%x\n", core_0_alloc_weight, core_0_alloc_weight);
        bdmf_session_print(session, "core_0_free_weight = %u = 0x%x\n", core_0_free_weight, core_0_free_weight);
        bdmf_session_print(session, "core_1_alloc_weight = %u = 0x%x\n", core_1_alloc_weight, core_1_alloc_weight);
        bdmf_session_print(session, "core_1_free_weight = %u = 0x%x\n", core_1_free_weight, core_1_free_weight);
        break;
    }
    case cli_fpm_bb_cfg:
    {
        uint8_t bb_core_sel;
        bdmf_boolean bb_fast_ack;
        bdmf_boolean bb_soft_reset;
        ag_err = ag_drv_fpm_bb_cfg_get(&bb_core_sel, &bb_fast_ack, &bb_soft_reset);
        bdmf_session_print(session, "bb_core_sel = %u = 0x%x\n", bb_core_sel, bb_core_sel);
        bdmf_session_print(session, "bb_fast_ack = %u = 0x%x\n", bb_fast_ack, bb_fast_ack);
        bdmf_session_print(session, "bb_soft_reset = %u = 0x%x\n", bb_soft_reset, bb_soft_reset);
        break;
    }
    case cli_fpm_spare:
    {
        uint32_t spare_bits;
        ag_err = ag_drv_fpm_spare_get(&spare_bits);
        bdmf_session_print(session, "spare_bits = %u = 0x%x\n", spare_bits, spare_bits);
        break;
    }
    case cli_fpm_prbs_invalid_gen:
    {
        uint32_t mask;
        bdmf_boolean enable;
        ag_err = ag_drv_fpm_prbs_invalid_gen_get(&mask, &enable);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_fpm_cfg1:
    {
        fpm_cfg1 cfg1;
        ag_err = ag_drv_fpm_cfg1_get(&cfg1);
        bdmf_session_print(session, "pool_search_mode = %u = 0x%x\n", cfg1.pool_search_mode, cfg1.pool_search_mode);
        bdmf_session_print(session, "enable_jumbo_support = %u = 0x%x\n", cfg1.enable_jumbo_support, cfg1.enable_jumbo_support);
        bdmf_session_print(session, "pool_0_disable = %u = 0x%x\n", cfg1.pool_0_disable, cfg1.pool_0_disable);
        bdmf_session_print(session, "pool_1_disable = %u = 0x%x\n", cfg1.pool_1_disable, cfg1.pool_1_disable);
        bdmf_session_print(session, "pool_2_disable = %u = 0x%x\n", cfg1.pool_2_disable, cfg1.pool_2_disable);
        bdmf_session_print(session, "pool_3_disable = %u = 0x%x\n", cfg1.pool_3_disable, cfg1.pool_3_disable);
        bdmf_session_print(session, "mark_en = %u = 0x%x\n", cfg1.mark_en, cfg1.mark_en);
        bdmf_session_print(session, "buf_size = %u = 0x%x\n", cfg1.buf_size, cfg1.buf_size);
        break;
    }
    case cli_fpm_cfg2:
    {
        uint32_t pool_base_address;
        ag_err = ag_drv_fpm_cfg2_get(&pool_base_address);
        bdmf_session_print(session, "pool_base_address = %u = 0x%x\n", pool_base_address, pool_base_address);
        break;
    }
    case cli_fpm_intr_msk:
    {
        fpm_intr_msk intr_msk;
        ag_err = ag_drv_fpm_intr_msk_get(&intr_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u = 0x%x\n", intr_msk.alloc_fifo_full_msk, intr_msk.alloc_fifo_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u = 0x%x\n", intr_msk.free_fifo_full_msk, intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "pool_full_msk = %u = 0x%x\n", intr_msk.pool_full_msk, intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u = 0x%x\n", intr_msk.free_token_no_valid_msk, intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u = 0x%x\n", intr_msk.free_token_index_out_of_range_msk, intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u = 0x%x\n", intr_msk.multi_token_no_valid_msk, intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u = 0x%x\n", intr_msk.multi_token_index_out_of_range_msk, intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u = 0x%x\n", intr_msk.pool_dis_free_multi_msk, intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u = 0x%x\n", intr_msk.memory_corrupt_msk, intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "xoff_msk = %u = 0x%x\n", intr_msk.xoff_msk, intr_msk.xoff_msk);
        bdmf_session_print(session, "xon_msk = %u = 0x%x\n", intr_msk.xon_msk, intr_msk.xon_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u = 0x%x\n", intr_msk.illegal_address_access_msk, intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u = 0x%x\n", intr_msk.illegal_alloc_request_msk, intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u = 0x%x\n", intr_msk.expired_token_det_msk, intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u = 0x%x\n", intr_msk.expired_token_recov_msk, intr_msk.expired_token_recov_msk);
        break;
    }
    case cli_fpm_intr_sts:
    {
        fpm_intr_sts intr_sts;
        ag_err = ag_drv_fpm_intr_sts_get(&intr_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u = 0x%x\n", intr_sts.alloc_fifo_full_sts, intr_sts.alloc_fifo_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u = 0x%x\n", intr_sts.free_fifo_full_sts, intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "pool_full_sts = %u = 0x%x\n", intr_sts.pool_full_sts, intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u = 0x%x\n", intr_sts.free_token_no_valid_sts, intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u = 0x%x\n", intr_sts.free_token_index_out_of_range_sts, intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u = 0x%x\n", intr_sts.multi_token_no_valid_sts, intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u = 0x%x\n", intr_sts.multi_token_index_out_of_range_sts, intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u = 0x%x\n", intr_sts.pool_dis_free_multi_sts, intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u = 0x%x\n", intr_sts.memory_corrupt_sts, intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "xoff_state_sts = %u = 0x%x\n", intr_sts.xoff_state_sts, intr_sts.xoff_state_sts);
        bdmf_session_print(session, "xon_state_sts = %u = 0x%x\n", intr_sts.xon_state_sts, intr_sts.xon_state_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u = 0x%x\n", intr_sts.illegal_address_access_sts, intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u = 0x%x\n", intr_sts.illegal_alloc_request_sts, intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u = 0x%x\n", intr_sts.expired_token_det_sts, intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u = 0x%x\n", intr_sts.expired_token_recov_sts, intr_sts.expired_token_recov_sts);
        break;
    }
    case cli_fpm_stall_msk:
    {
        fpm_stall_msk stall_msk;
        ag_err = ag_drv_fpm_stall_msk_get(&stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u = 0x%x\n", stall_msk.free_token_no_valid_stall_msk, stall_msk.free_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u = 0x%x\n", stall_msk.free_token_index_out_of_range_stall_msk, stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u = 0x%x\n", stall_msk.multi_token_no_valid_stall_msk, stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u = 0x%x\n", stall_msk.multi_token_index_out_of_range_stall_msk, stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u = 0x%x\n", stall_msk.memory_corrupt_stall_msk, stall_msk.memory_corrupt_stall_msk);
        break;
    }
    case cli_fpm_cfg3:
    {
        uint16_t halt_alloc_timer;
        ag_err = ag_drv_fpm_cfg3_get(&halt_alloc_timer);
        bdmf_session_print(session, "halt_alloc_timer = %u = 0x%x\n", halt_alloc_timer, halt_alloc_timer);
        break;
    }
    case cli_fpm_stat5:
    {
        uint32_t mem_corrupt_sts_related_alloc_token;
        ag_err = ag_drv_fpm_stat5_get(&mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u = 0x%x\n", mem_corrupt_sts_related_alloc_token, mem_corrupt_sts_related_alloc_token);
        break;
    }
    case cli_fpm_xon_xoff_cfg:
    {
        uint16_t xoff_threshold;
        uint16_t xon_threshold;
        ag_err = ag_drv_fpm_xon_xoff_cfg_get(&xoff_threshold, &xon_threshold);
        bdmf_session_print(session, "xoff_threshold = %u = 0x%x\n", xoff_threshold, xoff_threshold);
        bdmf_session_print(session, "xon_threshold = %u = 0x%x\n", xon_threshold, xon_threshold);
        break;
    }
    case cli_fpm_not_empty_cfg:
    {
        uint8_t not_empty_threshold;
        ag_err = ag_drv_fpm_not_empty_cfg_get(&not_empty_threshold);
        bdmf_session_print(session, "not_empty_threshold = %u = 0x%x\n", not_empty_threshold, not_empty_threshold);
        break;
    }
    case cli_fpm_token_recover_ctl:
    {
        fpm_token_recover_ctl token_recover_ctl;
        ag_err = ag_drv_fpm_token_recover_ctl_get(&token_recover_ctl);
        bdmf_session_print(session, "token_recover_ena = %u = 0x%x\n", token_recover_ctl.token_recover_ena, token_recover_ctl.token_recover_ena);
        bdmf_session_print(session, "single_pass_ena = %u = 0x%x\n", token_recover_ctl.single_pass_ena, token_recover_ctl.single_pass_ena);
        bdmf_session_print(session, "token_remark_ena = %u = 0x%x\n", token_recover_ctl.token_remark_ena, token_recover_ctl.token_remark_ena);
        bdmf_session_print(session, "token_reclaim_ena = %u = 0x%x\n", token_recover_ctl.token_reclaim_ena, token_recover_ctl.token_reclaim_ena);
        bdmf_session_print(session, "force_token_reclaim = %u = 0x%x\n", token_recover_ctl.force_token_reclaim, token_recover_ctl.force_token_reclaim);
        bdmf_session_print(session, "clr_expired_token_count = %u = 0x%x\n", token_recover_ctl.clr_expired_token_count, token_recover_ctl.clr_expired_token_count);
        bdmf_session_print(session, "clr_recovered_token_count = %u = 0x%x\n", token_recover_ctl.clr_recovered_token_count, token_recover_ctl.clr_recovered_token_count);
        break;
    }
    case cli_fpm_short_aging_timer:
    {
        uint32_t timer;
        ag_err = ag_drv_fpm_short_aging_timer_get(&timer);
        bdmf_session_print(session, "timer = %u = 0x%x\n", timer, timer);
        break;
    }
    case cli_fpm_long_aging_timer:
    {
        uint32_t timer;
        ag_err = ag_drv_fpm_long_aging_timer_get(&timer);
        bdmf_session_print(session, "timer = %u = 0x%x\n", timer, timer);
        break;
    }
    case cli_fpm_cache_recycle_timer:
    {
        uint16_t recycle_timer;
        ag_err = ag_drv_fpm_cache_recycle_timer_get(&recycle_timer);
        bdmf_session_print(session, "recycle_timer = %u = 0x%x\n", recycle_timer, recycle_timer);
        break;
    }
    case cli_fpm_expired_token_count:
    {
        uint32_t count;
        ag_err = ag_drv_fpm_expired_token_count_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_fpm_recovered_token_count:
    {
        uint32_t count;
        ag_err = ag_drv_fpm_recovered_token_count_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_fpm_token_recover_start_end:
    {
        uint16_t end_index;
        uint16_t start_index;
        ag_err = ag_drv_fpm_token_recover_start_end_get(&end_index, &start_index);
        bdmf_session_print(session, "end_index = %u = 0x%x\n", end_index, end_index);
        bdmf_session_print(session, "start_index = %u = 0x%x\n", start_index, start_index);
        break;
    }
    case cli_fpm_size_0_alloc_dealloc:
    {
        uint16_t token_size;
        uint32_t token_index;
        uint8_t size_id;
        ag_err = ag_drv_fpm_size_0_alloc_dealloc_get(&token_size, &token_index, &size_id);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "size_id = %u = 0x%x\n", size_id, size_id);
        break;
    }
    case cli_fpm_size_1_alloc_dealloc:
    {
        uint16_t token_size;
        uint32_t token_index;
        uint8_t size_id;
        ag_err = ag_drv_fpm_size_1_alloc_dealloc_get(&token_size, &token_index, &size_id);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "size_id = %u = 0x%x\n", size_id, size_id);
        break;
    }
    case cli_fpm_size_2_alloc_dealloc:
    {
        uint16_t token_size;
        uint32_t token_index;
        uint8_t size_id;
        ag_err = ag_drv_fpm_size_2_alloc_dealloc_get(&token_size, &token_index, &size_id);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "size_id = %u = 0x%x\n", size_id, size_id);
        break;
    }
    case cli_fpm_size_3_alloc_dealloc:
    {
        uint8_t token_size;
        uint32_t token_index;
        uint8_t size_id;
        ag_err = ag_drv_fpm_size_3_alloc_dealloc_get(&token_size, &token_index, &size_id);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "size_id = %u = 0x%x\n", size_id, size_id);
        break;
    }
    case cli_fpm_search_data_1:
    {
        uint16_t data;
        ag_err = ag_drv_fpm_search_data_1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_search_data_2:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_search_data_2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_search_data_3:
    {
        uint16_t data;
        ag_err = ag_drv_fpm_search_data_3_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_multicast_data:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_multicast_data_get(parm[1].value.unumber, &data);
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
static int ag_drv_fpm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        bdmf_boolean init_mem = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_init_mem_set( %u)\n",
            init_mem);
        ag_err = ag_drv_fpm_init_mem_set(init_mem);
        if (!ag_err)
            ag_err = ag_drv_fpm_init_mem_get(&init_mem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_init_mem_get( %u)\n",
                init_mem);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init_mem != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pool_enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool_en_set( %u)\n",
            pool_enable);
        ag_err = ag_drv_fpm_pool_en_set(pool_enable);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool_en_get(&pool_enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool_en_get( %u)\n",
                pool_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool_stat pool_stat = {.ovrfl = gtmv(m, 16), .undrfl = gtmv(m, 16), .pool_full = gtmv(m, 1), .free_fifo_full = gtmv(m, 1), .free_fifo_empty = gtmv(m, 1), .alloc_fifo_full = gtmv(m, 1), .alloc_fifo_empty = gtmv(m, 1), .num_of_tokens_available = gtmv(m, 22), .num_of_not_valid_token_frees = gtmv(m, 22), .num_of_not_valid_token_multi = gtmv(m, 22), .invalid_free_token = gtmv(m, 32), .invalid_mcast_token = gtmv(m, 32), .tokens_available_low_wtmk = gtmv(m, 22)};
        bdmf_session_print(session, "ag_drv_fpm_pool_stat_set( %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool_stat.ovrfl, pool_stat.undrfl, pool_stat.pool_full, pool_stat.free_fifo_full, 
            pool_stat.free_fifo_empty, pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_empty, pool_stat.num_of_tokens_available, 
            pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_multi, pool_stat.invalid_free_token, pool_stat.invalid_mcast_token, 
            pool_stat.tokens_available_low_wtmk);
        ag_err = ag_drv_fpm_pool_stat_set(&pool_stat);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool_stat_get(&pool_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool_stat_get( %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool_stat.ovrfl, pool_stat.undrfl, pool_stat.pool_full, pool_stat.free_fifo_full, 
                pool_stat.free_fifo_empty, pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_empty, pool_stat.num_of_tokens_available, 
                pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_multi, pool_stat.invalid_free_token, pool_stat.invalid_mcast_token, 
                pool_stat.tokens_available_low_wtmk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_stat.ovrfl != gtmv(m, 16) || pool_stat.undrfl != gtmv(m, 16) || pool_stat.pool_full != gtmv(m, 1) || pool_stat.free_fifo_full != gtmv(m, 1) || pool_stat.free_fifo_empty != gtmv(m, 1) || pool_stat.alloc_fifo_full != gtmv(m, 1) || pool_stat.alloc_fifo_empty != gtmv(m, 1) || pool_stat.num_of_tokens_available != gtmv(m, 22) || pool_stat.num_of_not_valid_token_frees != gtmv(m, 22) || pool_stat.num_of_not_valid_token_multi != gtmv(m, 22) || pool_stat.invalid_free_token != gtmv(m, 32) || pool_stat.invalid_mcast_token != gtmv(m, 32) || pool_stat.tokens_available_low_wtmk != gtmv(m, 22))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tp_mux_cntrl = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_fpm_ctl_set( %u)\n",
            tp_mux_cntrl);
        ag_err = ag_drv_fpm_ctl_set(tp_mux_cntrl);
        if (!ag_err)
            ag_err = ag_drv_fpm_ctl_get(&tp_mux_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_ctl_get( %u)\n",
                tp_mux_cntrl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tp_mux_cntrl != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t core_0_alloc_weight = gtmv(m, 8);
        uint8_t core_0_free_weight = gtmv(m, 8);
        uint8_t core_1_alloc_weight = gtmv(m, 8);
        uint8_t core_1_free_weight = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_fpm_weight_set( %u %u %u %u)\n",
            core_0_alloc_weight, core_0_free_weight, core_1_alloc_weight, core_1_free_weight);
        ag_err = ag_drv_fpm_weight_set(core_0_alloc_weight, core_0_free_weight, core_1_alloc_weight, core_1_free_weight);
        if (!ag_err)
            ag_err = ag_drv_fpm_weight_get(&core_0_alloc_weight, &core_0_free_weight, &core_1_alloc_weight, &core_1_free_weight);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_weight_get( %u %u %u %u)\n",
                core_0_alloc_weight, core_0_free_weight, core_1_alloc_weight, core_1_free_weight);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (core_0_alloc_weight != gtmv(m, 8) || core_0_free_weight != gtmv(m, 8) || core_1_alloc_weight != gtmv(m, 8) || core_1_free_weight != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bb_core_sel = gtmv(m, 2);
        bdmf_boolean bb_fast_ack = gtmv(m, 1);
        bdmf_boolean bb_soft_reset = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_bb_cfg_set( %u %u %u)\n",
            bb_core_sel, bb_fast_ack, bb_soft_reset);
        ag_err = ag_drv_fpm_bb_cfg_set(bb_core_sel, bb_fast_ack, bb_soft_reset);
        if (!ag_err)
            ag_err = ag_drv_fpm_bb_cfg_get(&bb_core_sel, &bb_fast_ack, &bb_soft_reset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_bb_cfg_get( %u %u %u)\n",
                bb_core_sel, bb_fast_ack, bb_soft_reset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bb_core_sel != gtmv(m, 2) || bb_fast_ack != gtmv(m, 1) || bb_soft_reset != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t spare_bits = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_spare_set( %u)\n",
            spare_bits);
        ag_err = ag_drv_fpm_spare_set(spare_bits);
        if (!ag_err)
            ag_err = ag_drv_fpm_spare_get(&spare_bits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_spare_get( %u)\n",
                spare_bits);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (spare_bits != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t mask = gtmv(m, 31);
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_prbs_invalid_gen_set( %u %u)\n",
            mask, enable);
        ag_err = ag_drv_fpm_prbs_invalid_gen_set(mask, enable);
        if (!ag_err)
            ag_err = ag_drv_fpm_prbs_invalid_gen_get(&mask, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_prbs_invalid_gen_get( %u %u)\n",
                mask, enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 31) || enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_cfg1 cfg1 = {.pool_search_mode = gtmv(m, 1), .enable_jumbo_support = gtmv(m, 1), .pool_0_disable = gtmv(m, 1), .pool_1_disable = gtmv(m, 1), .pool_2_disable = gtmv(m, 1), .pool_3_disable = gtmv(m, 1), .mark_en = gtmv(m, 1), .buf_size = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_fpm_cfg1_set( %u %u %u %u %u %u %u %u)\n",
            cfg1.pool_search_mode, cfg1.enable_jumbo_support, cfg1.pool_0_disable, cfg1.pool_1_disable, 
            cfg1.pool_2_disable, cfg1.pool_3_disable, cfg1.mark_en, cfg1.buf_size);
        ag_err = ag_drv_fpm_cfg1_set(&cfg1);
        if (!ag_err)
            ag_err = ag_drv_fpm_cfg1_get(&cfg1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_cfg1_get( %u %u %u %u %u %u %u %u)\n",
                cfg1.pool_search_mode, cfg1.enable_jumbo_support, cfg1.pool_0_disable, cfg1.pool_1_disable, 
                cfg1.pool_2_disable, cfg1.pool_3_disable, cfg1.mark_en, cfg1.buf_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg1.pool_search_mode != gtmv(m, 1) || cfg1.enable_jumbo_support != gtmv(m, 1) || cfg1.pool_0_disable != gtmv(m, 1) || cfg1.pool_1_disable != gtmv(m, 1) || cfg1.pool_2_disable != gtmv(m, 1) || cfg1.pool_3_disable != gtmv(m, 1) || cfg1.mark_en != gtmv(m, 1) || cfg1.buf_size != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t pool_base_address = gtmv(m, 30);
        bdmf_session_print(session, "ag_drv_fpm_cfg2_set( %u)\n",
            pool_base_address);
        ag_err = ag_drv_fpm_cfg2_set(pool_base_address);
        if (!ag_err)
            ag_err = ag_drv_fpm_cfg2_get(&pool_base_address);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_cfg2_get( %u)\n",
                pool_base_address);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_base_address != gtmv(m, 30))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_intr_msk intr_msk = {.alloc_fifo_full_msk = gtmv(m, 1), .free_fifo_full_msk = gtmv(m, 1), .pool_full_msk = gtmv(m, 1), .free_token_no_valid_msk = gtmv(m, 1), .free_token_index_out_of_range_msk = gtmv(m, 1), .multi_token_no_valid_msk = gtmv(m, 1), .multi_token_index_out_of_range_msk = gtmv(m, 1), .pool_dis_free_multi_msk = gtmv(m, 1), .memory_corrupt_msk = gtmv(m, 1), .xoff_msk = gtmv(m, 1), .xon_msk = gtmv(m, 1), .illegal_address_access_msk = gtmv(m, 1), .illegal_alloc_request_msk = gtmv(m, 1), .expired_token_det_msk = gtmv(m, 1), .expired_token_recov_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            intr_msk.alloc_fifo_full_msk, intr_msk.free_fifo_full_msk, intr_msk.pool_full_msk, intr_msk.free_token_no_valid_msk, 
            intr_msk.free_token_index_out_of_range_msk, intr_msk.multi_token_no_valid_msk, intr_msk.multi_token_index_out_of_range_msk, intr_msk.pool_dis_free_multi_msk, 
            intr_msk.memory_corrupt_msk, intr_msk.xoff_msk, intr_msk.xon_msk, intr_msk.illegal_address_access_msk, 
            intr_msk.illegal_alloc_request_msk, intr_msk.expired_token_det_msk, intr_msk.expired_token_recov_msk);
        ag_err = ag_drv_fpm_intr_msk_set(&intr_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_intr_msk_get(&intr_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                intr_msk.alloc_fifo_full_msk, intr_msk.free_fifo_full_msk, intr_msk.pool_full_msk, intr_msk.free_token_no_valid_msk, 
                intr_msk.free_token_index_out_of_range_msk, intr_msk.multi_token_no_valid_msk, intr_msk.multi_token_index_out_of_range_msk, intr_msk.pool_dis_free_multi_msk, 
                intr_msk.memory_corrupt_msk, intr_msk.xoff_msk, intr_msk.xon_msk, intr_msk.illegal_address_access_msk, 
                intr_msk.illegal_alloc_request_msk, intr_msk.expired_token_det_msk, intr_msk.expired_token_recov_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (intr_msk.alloc_fifo_full_msk != gtmv(m, 1) || intr_msk.free_fifo_full_msk != gtmv(m, 1) || intr_msk.pool_full_msk != gtmv(m, 1) || intr_msk.free_token_no_valid_msk != gtmv(m, 1) || intr_msk.free_token_index_out_of_range_msk != gtmv(m, 1) || intr_msk.multi_token_no_valid_msk != gtmv(m, 1) || intr_msk.multi_token_index_out_of_range_msk != gtmv(m, 1) || intr_msk.pool_dis_free_multi_msk != gtmv(m, 1) || intr_msk.memory_corrupt_msk != gtmv(m, 1) || intr_msk.xoff_msk != gtmv(m, 1) || intr_msk.xon_msk != gtmv(m, 1) || intr_msk.illegal_address_access_msk != gtmv(m, 1) || intr_msk.illegal_alloc_request_msk != gtmv(m, 1) || intr_msk.expired_token_det_msk != gtmv(m, 1) || intr_msk.expired_token_recov_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_intr_sts intr_sts = {.alloc_fifo_full_sts = gtmv(m, 1), .free_fifo_full_sts = gtmv(m, 1), .pool_full_sts = gtmv(m, 1), .free_token_no_valid_sts = gtmv(m, 1), .free_token_index_out_of_range_sts = gtmv(m, 1), .multi_token_no_valid_sts = gtmv(m, 1), .multi_token_index_out_of_range_sts = gtmv(m, 1), .pool_dis_free_multi_sts = gtmv(m, 1), .memory_corrupt_sts = gtmv(m, 1), .xoff_state_sts = gtmv(m, 1), .xon_state_sts = gtmv(m, 1), .illegal_address_access_sts = gtmv(m, 1), .illegal_alloc_request_sts = gtmv(m, 1), .expired_token_det_sts = gtmv(m, 1), .expired_token_recov_sts = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            intr_sts.alloc_fifo_full_sts, intr_sts.free_fifo_full_sts, intr_sts.pool_full_sts, intr_sts.free_token_no_valid_sts, 
            intr_sts.free_token_index_out_of_range_sts, intr_sts.multi_token_no_valid_sts, intr_sts.multi_token_index_out_of_range_sts, intr_sts.pool_dis_free_multi_sts, 
            intr_sts.memory_corrupt_sts, intr_sts.xoff_state_sts, intr_sts.xon_state_sts, intr_sts.illegal_address_access_sts, 
            intr_sts.illegal_alloc_request_sts, intr_sts.expired_token_det_sts, intr_sts.expired_token_recov_sts);
        ag_err = ag_drv_fpm_intr_sts_set(&intr_sts);
        if (!ag_err)
            ag_err = ag_drv_fpm_intr_sts_get(&intr_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                intr_sts.alloc_fifo_full_sts, intr_sts.free_fifo_full_sts, intr_sts.pool_full_sts, intr_sts.free_token_no_valid_sts, 
                intr_sts.free_token_index_out_of_range_sts, intr_sts.multi_token_no_valid_sts, intr_sts.multi_token_index_out_of_range_sts, intr_sts.pool_dis_free_multi_sts, 
                intr_sts.memory_corrupt_sts, intr_sts.xoff_state_sts, intr_sts.xon_state_sts, intr_sts.illegal_address_access_sts, 
                intr_sts.illegal_alloc_request_sts, intr_sts.expired_token_det_sts, intr_sts.expired_token_recov_sts);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (intr_sts.alloc_fifo_full_sts != gtmv(m, 1) || intr_sts.free_fifo_full_sts != gtmv(m, 1) || intr_sts.pool_full_sts != gtmv(m, 1) || intr_sts.free_token_no_valid_sts != gtmv(m, 1) || intr_sts.free_token_index_out_of_range_sts != gtmv(m, 1) || intr_sts.multi_token_no_valid_sts != gtmv(m, 1) || intr_sts.multi_token_index_out_of_range_sts != gtmv(m, 1) || intr_sts.pool_dis_free_multi_sts != gtmv(m, 1) || intr_sts.memory_corrupt_sts != gtmv(m, 1) || intr_sts.xoff_state_sts != gtmv(m, 1) || intr_sts.xon_state_sts != gtmv(m, 1) || intr_sts.illegal_address_access_sts != gtmv(m, 1) || intr_sts.illegal_alloc_request_sts != gtmv(m, 1) || intr_sts.expired_token_det_sts != gtmv(m, 1) || intr_sts.expired_token_recov_sts != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_stall_msk stall_msk = {.free_token_no_valid_stall_msk = gtmv(m, 1), .free_token_index_out_of_range_stall_msk = gtmv(m, 1), .multi_token_no_valid_stall_msk = gtmv(m, 1), .multi_token_index_out_of_range_stall_msk = gtmv(m, 1), .memory_corrupt_stall_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_stall_msk_set( %u %u %u %u %u)\n",
            stall_msk.free_token_no_valid_stall_msk, stall_msk.free_token_index_out_of_range_stall_msk, stall_msk.multi_token_no_valid_stall_msk, stall_msk.multi_token_index_out_of_range_stall_msk, 
            stall_msk.memory_corrupt_stall_msk);
        ag_err = ag_drv_fpm_stall_msk_set(&stall_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_stall_msk_get(&stall_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_stall_msk_get( %u %u %u %u %u)\n",
                stall_msk.free_token_no_valid_stall_msk, stall_msk.free_token_index_out_of_range_stall_msk, stall_msk.multi_token_no_valid_stall_msk, stall_msk.multi_token_index_out_of_range_stall_msk, 
                stall_msk.memory_corrupt_stall_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (stall_msk.free_token_no_valid_stall_msk != gtmv(m, 1) || stall_msk.free_token_index_out_of_range_stall_msk != gtmv(m, 1) || stall_msk.multi_token_no_valid_stall_msk != gtmv(m, 1) || stall_msk.multi_token_index_out_of_range_stall_msk != gtmv(m, 1) || stall_msk.memory_corrupt_stall_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t halt_alloc_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_cfg3_set( %u)\n",
            halt_alloc_timer);
        ag_err = ag_drv_fpm_cfg3_set(halt_alloc_timer);
        if (!ag_err)
            ag_err = ag_drv_fpm_cfg3_get(&halt_alloc_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_cfg3_get( %u)\n",
                halt_alloc_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (halt_alloc_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t mem_corrupt_sts_related_alloc_token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_stat5_set( %u)\n",
            mem_corrupt_sts_related_alloc_token);
        ag_err = ag_drv_fpm_stat5_set(mem_corrupt_sts_related_alloc_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_stat5_get(&mem_corrupt_sts_related_alloc_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_stat5_get( %u)\n",
                mem_corrupt_sts_related_alloc_token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mem_corrupt_sts_related_alloc_token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t xoff_threshold = gtmv(m, 16);
        uint16_t xon_threshold = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_xon_xoff_cfg_set( %u %u)\n",
            xoff_threshold, xon_threshold);
        ag_err = ag_drv_fpm_xon_xoff_cfg_set(xoff_threshold, xon_threshold);
        if (!ag_err)
            ag_err = ag_drv_fpm_xon_xoff_cfg_get(&xoff_threshold, &xon_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_xon_xoff_cfg_get( %u %u)\n",
                xoff_threshold, xon_threshold);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (xoff_threshold != gtmv(m, 16) || xon_threshold != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t not_empty_threshold = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_fpm_not_empty_cfg_set( %u)\n",
            not_empty_threshold);
        ag_err = ag_drv_fpm_not_empty_cfg_set(not_empty_threshold);
        if (!ag_err)
            ag_err = ag_drv_fpm_not_empty_cfg_get(&not_empty_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_not_empty_cfg_get( %u)\n",
                not_empty_threshold);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (not_empty_threshold != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_token_recover_ctl token_recover_ctl = {.token_recover_ena = gtmv(m, 1), .single_pass_ena = gtmv(m, 1), .token_remark_ena = gtmv(m, 1), .token_reclaim_ena = gtmv(m, 1), .force_token_reclaim = gtmv(m, 1), .clr_expired_token_count = gtmv(m, 1), .clr_recovered_token_count = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_token_recover_ctl_set( %u %u %u %u %u %u %u)\n",
            token_recover_ctl.token_recover_ena, token_recover_ctl.single_pass_ena, token_recover_ctl.token_remark_ena, token_recover_ctl.token_reclaim_ena, 
            token_recover_ctl.force_token_reclaim, token_recover_ctl.clr_expired_token_count, token_recover_ctl.clr_recovered_token_count);
        ag_err = ag_drv_fpm_token_recover_ctl_set(&token_recover_ctl);
        if (!ag_err)
            ag_err = ag_drv_fpm_token_recover_ctl_get(&token_recover_ctl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_token_recover_ctl_get( %u %u %u %u %u %u %u)\n",
                token_recover_ctl.token_recover_ena, token_recover_ctl.single_pass_ena, token_recover_ctl.token_remark_ena, token_recover_ctl.token_reclaim_ena, 
                token_recover_ctl.force_token_reclaim, token_recover_ctl.clr_expired_token_count, token_recover_ctl.clr_recovered_token_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_recover_ctl.token_recover_ena != gtmv(m, 1) || token_recover_ctl.single_pass_ena != gtmv(m, 1) || token_recover_ctl.token_remark_ena != gtmv(m, 1) || token_recover_ctl.token_reclaim_ena != gtmv(m, 1) || token_recover_ctl.force_token_reclaim != gtmv(m, 1) || token_recover_ctl.clr_expired_token_count != gtmv(m, 1) || token_recover_ctl.clr_recovered_token_count != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_short_aging_timer_set( %u)\n",
            timer);
        ag_err = ag_drv_fpm_short_aging_timer_set(timer);
        if (!ag_err)
            ag_err = ag_drv_fpm_short_aging_timer_get(&timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_short_aging_timer_get( %u)\n",
                timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_long_aging_timer_set( %u)\n",
            timer);
        ag_err = ag_drv_fpm_long_aging_timer_set(timer);
        if (!ag_err)
            ag_err = ag_drv_fpm_long_aging_timer_get(&timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_long_aging_timer_get( %u)\n",
                timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t recycle_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_cache_recycle_timer_set( %u)\n",
            recycle_timer);
        ag_err = ag_drv_fpm_cache_recycle_timer_set(recycle_timer);
        if (!ag_err)
            ag_err = ag_drv_fpm_cache_recycle_timer_get(&recycle_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_cache_recycle_timer_get( %u)\n",
                recycle_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (recycle_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_expired_token_count_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_expired_token_count_get( %u)\n",
                count);
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_recovered_token_count_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_recovered_token_count_get( %u)\n",
                count);
        }
    }

    {
        uint16_t end_index = gtmv(m, 16);
        uint16_t start_index = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_set( %u %u)\n",
            end_index, start_index);
        ag_err = ag_drv_fpm_token_recover_start_end_set(end_index, start_index);
        if (!ag_err)
            ag_err = ag_drv_fpm_token_recover_start_end_get(&end_index, &start_index);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_get( %u %u)\n",
                end_index, start_index);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end_index != gtmv(m, 16) || start_index != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 11);
        uint32_t token_index = gtmv(m, 19);
        uint8_t size_id = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_fpm_size_0_alloc_dealloc_set( %u %u %u)\n",
            token_size, token_index, size_id);
        ag_err = ag_drv_fpm_size_0_alloc_dealloc_set(token_size, token_index, size_id);
        if (!ag_err)
            ag_err = ag_drv_fpm_size_0_alloc_dealloc_get(&token_size, &token_index, &size_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_size_0_alloc_dealloc_get( %u %u %u)\n",
                token_size, token_index, size_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 11) || token_index != gtmv(m, 19) || size_id != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 10);
        uint32_t token_index = gtmv(m, 20);
        uint8_t size_id = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_fpm_size_1_alloc_dealloc_set( %u %u %u)\n",
            token_size, token_index, size_id);
        ag_err = ag_drv_fpm_size_1_alloc_dealloc_set(token_size, token_index, size_id);
        if (!ag_err)
            ag_err = ag_drv_fpm_size_1_alloc_dealloc_get(&token_size, &token_index, &size_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_size_1_alloc_dealloc_get( %u %u %u)\n",
                token_size, token_index, size_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 10) || token_index != gtmv(m, 20) || size_id != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 9);
        uint32_t token_index = gtmv(m, 21);
        uint8_t size_id = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_fpm_size_2_alloc_dealloc_set( %u %u %u)\n",
            token_size, token_index, size_id);
        ag_err = ag_drv_fpm_size_2_alloc_dealloc_set(token_size, token_index, size_id);
        if (!ag_err)
            ag_err = ag_drv_fpm_size_2_alloc_dealloc_get(&token_size, &token_index, &size_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_size_2_alloc_dealloc_get( %u %u %u)\n",
                token_size, token_index, size_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 9) || token_index != gtmv(m, 21) || size_id != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t token_size = gtmv(m, 8);
        uint32_t token_index = gtmv(m, 22);
        uint8_t size_id = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_fpm_size_3_alloc_dealloc_set( %u %u %u)\n",
            token_size, token_index, size_id);
        ag_err = ag_drv_fpm_size_3_alloc_dealloc_set(token_size, token_index, size_id);
        if (!ag_err)
            ag_err = ag_drv_fpm_size_3_alloc_dealloc_get(&token_size, &token_index, &size_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_size_3_alloc_dealloc_get( %u %u %u)\n",
                token_size, token_index, size_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 8) || token_index != gtmv(m, 22) || size_id != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_multi multi = {.token_multi = gtmv(m, 7), .update_type = gtmv(m, 1), .token_index = gtmv(m, 17), .ddr = gtmv(m, 1), .token_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_multi_set( %u %u %u %u %u)\n",
            multi.token_multi, multi.update_type, multi.token_index, multi.ddr, 
            multi.token_valid);
        ag_err = ag_drv_fpm_multi_set(&multi);
    }

    {
        uint32_t search1 = gtmv(m, 9);
        uint16_t data = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u)\n", search1,
            data);
        ag_err = ag_drv_fpm_search_data_1_set(search1, data);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_1_get(search1, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_get( %u %u)\n", search1,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t search1 = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u)\n", search1,
            data);
        ag_err = ag_drv_fpm_search_data_2_set(search1, data);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_2_get(search1, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_get( %u %u)\n", search1,
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

    {
        uint32_t search3 = gtmv(m, 9);
        uint16_t data = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u)\n", search3,
            data);
        ag_err = ag_drv_fpm_search_data_3_set(search3, data);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_3_get(search3, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_get( %u %u)\n", search3,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t multicast_data = gtmv(m, 14);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
            data);
        ag_err = ag_drv_fpm_multicast_data_set(multicast_data, data);
        if (!ag_err)
            ag_err = ag_drv_fpm_multicast_data_get(multicast_data, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_get( %u %u)\n", multicast_data,
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
static int ag_drv_fpm_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case cli_fpm_search_data_1:
    {
        uint32_t max_search1 = 529;
        uint32_t search1 = gtmv(m, 9);
        uint16_t data = gtmv(m, 16);

        if ((start_idx >= max_search1) || (stop_idx >= max_search1))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search1);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search1 = 0; search1 < max_search1; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u)\n", search1,
                data);
            ag_err = ag_drv_fpm_search_data_1_set(search1, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 16);

        for (search1 = start_idx; search1 <= stop_idx; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u)\n", search1,
                data);
            ag_err = ag_drv_fpm_search_data_1_set(search1, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (search1 = 0; search1 < max_search1; search1++)
        {
            if (search1 < start_idx || search1 > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_fpm_search_data_1_get(search1, &data);

            bdmf_session_print(session, "ag_drv_fpm_search_data_1_get( %u %u)\n", search1,
                data);

            if (data != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", search1);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of search_data_1 completed. Number of tested entries %u.\n", max_search1);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_fpm_search_data_2:
    {
        uint32_t max_search1 = 529;
        uint32_t search1 = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_search1) || (stop_idx >= max_search1))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search1);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search1 = 0; search1 < max_search1; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u)\n", search1,
                data);
            ag_err = ag_drv_fpm_search_data_2_set(search1, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (search1 = start_idx; search1 <= stop_idx; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u)\n", search1,
                data);
            ag_err = ag_drv_fpm_search_data_2_set(search1, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (search1 = 0; search1 < max_search1; search1++)
        {
            if (search1 < start_idx || search1 > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_fpm_search_data_2_get(search1, &data);

            bdmf_session_print(session, "ag_drv_fpm_search_data_2_get( %u %u)\n", search1,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", search1);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of search_data_2 completed. Number of tested entries %u.\n", max_search1);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_fpm_search_data_3:
    {
        uint32_t max_search3 = 529;
        uint32_t search3 = gtmv(m, 9);
        uint16_t data = gtmv(m, 16);

        if ((start_idx >= max_search3) || (stop_idx >= max_search3))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search3);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search3 = 0; search3 < max_search3; search3++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u)\n", search3,
                data);
            ag_err = ag_drv_fpm_search_data_3_set(search3, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search3);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 16);

        for (search3 = start_idx; search3 <= stop_idx; search3++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u)\n", search3,
                data);
            ag_err = ag_drv_fpm_search_data_3_set(search3, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", search3);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (search3 = 0; search3 < max_search3; search3++)
        {
            if (search3 < start_idx || search3 > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_fpm_search_data_3_get(search3, &data);

            bdmf_session_print(session, "ag_drv_fpm_search_data_3_get( %u %u)\n", search3,
                data);

            if (data != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", search3);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of search_data_3 completed. Number of tested entries %u.\n", max_search3);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_fpm_multicast_data:
    {
        uint32_t max_multicast_data = 16384;
        uint32_t multicast_data = gtmv(m, 14);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_multicast_data) || (stop_idx >= max_multicast_data))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_multicast_data);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (multicast_data = 0; multicast_data < max_multicast_data; multicast_data++)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
                data);
            ag_err = ag_drv_fpm_multicast_data_set(multicast_data, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", multicast_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (multicast_data = start_idx; multicast_data <= stop_idx; multicast_data++)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
                data);
            ag_err = ag_drv_fpm_multicast_data_set(multicast_data, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", multicast_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (multicast_data = 0; multicast_data < max_multicast_data; multicast_data++)
        {
            if (multicast_data < start_idx || multicast_data > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_fpm_multicast_data_get(multicast_data, &data);

            bdmf_session_print(session, "ag_drv_fpm_multicast_data_get( %u %u)\n", multicast_data,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", multicast_data);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of multicast_data completed. Number of tested entries %u.\n", max_multicast_data);
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
static int ag_drv_fpm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_ctl: reg = &RU_REG(FPM, CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_weight: reg = &RU_REG(FPM, WEIGHT); blk = &RU_BLK(FPM); break;
    case bdmf_address_bb_cfg: reg = &RU_REG(FPM, BB_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_spare: reg = &RU_REG(FPM, SPARE); blk = &RU_BLK(FPM); break;
    case bdmf_address_prbs_invalid_gen: reg = &RU_REG(FPM, PRBS_INVALID_GEN); blk = &RU_BLK(FPM); break;
    case bdmf_address_ctl_1: reg = &RU_REG(FPM, CTL_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_cfg1: reg = &RU_REG(FPM, CFG1); blk = &RU_BLK(FPM); break;
    case bdmf_address_cfg2: reg = &RU_REG(FPM, CFG2); blk = &RU_BLK(FPM); break;
    case bdmf_address_intr_msk: reg = &RU_REG(FPM, INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_intr_sts: reg = &RU_REG(FPM, INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_stall_msk: reg = &RU_REG(FPM, STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_cfg3: reg = &RU_REG(FPM, CFG3); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat1: reg = &RU_REG(FPM, STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat2: reg = &RU_REG(FPM, STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat3: reg = &RU_REG(FPM, STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat4: reg = &RU_REG(FPM, STAT4); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat5: reg = &RU_REG(FPM, STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat6: reg = &RU_REG(FPM, STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat7: reg = &RU_REG(FPM, STAT7); blk = &RU_BLK(FPM); break;
    case bdmf_address_stat8: reg = &RU_REG(FPM, STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_xon_xoff_cfg: reg = &RU_REG(FPM, XON_XOFF_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_not_empty_cfg: reg = &RU_REG(FPM, NOT_EMPTY_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_ctl: reg = &RU_REG(FPM, TOKEN_RECOVER_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_short_aging_timer: reg = &RU_REG(FPM, SHORT_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_long_aging_timer: reg = &RU_REG(FPM, LONG_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_cache_recycle_timer: reg = &RU_REG(FPM, CACHE_RECYCLE_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_expired_token_count: reg = &RU_REG(FPM, EXPIRED_TOKEN_COUNT); blk = &RU_BLK(FPM); break;
    case bdmf_address_recovered_token_count: reg = &RU_REG(FPM, RECOVERED_TOKEN_COUNT); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_start_end: reg = &RU_REG(FPM, TOKEN_RECOVER_START_END); blk = &RU_BLK(FPM); break;
    case bdmf_address_size_0_alloc_dealloc: reg = &RU_REG(FPM, SIZE_0_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_size_1_alloc_dealloc: reg = &RU_REG(FPM, SIZE_1_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_size_2_alloc_dealloc: reg = &RU_REG(FPM, SIZE_2_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_size_3_alloc_dealloc: reg = &RU_REG(FPM, SIZE_3_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_multi: reg = &RU_REG(FPM, MULTI); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_0: reg = &RU_REG(FPM, SEARCH_DATA_0); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_1: reg = &RU_REG(FPM, SEARCH_DATA_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_2: reg = &RU_REG(FPM, SEARCH_DATA_2); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_3: reg = &RU_REG(FPM, SEARCH_DATA_3); blk = &RU_BLK(FPM); break;
    case bdmf_address_multicast_data: reg = &RU_REG(FPM, MULTICAST_DATA); blk = &RU_BLK(FPM); break;
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

bdmfmon_handle_t ag_drv_fpm_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "fpm", "fpm", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_init_mem[] = {
            BDMFMON_MAKE_PARM("init_mem", "init_mem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_en[] = {
            BDMFMON_MAKE_PARM("pool_enable", "pool_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_stat[] = {
            BDMFMON_MAKE_PARM("ovrfl", "ovrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("undrfl", "undrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_full", "pool_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_full", "free_fifo_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_empty", "free_fifo_empty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_fifo_full", "alloc_fifo_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_fifo_empty", "alloc_fifo_empty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_of_tokens_available", "num_of_tokens_available", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_of_not_valid_token_frees", "num_of_not_valid_token_frees", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_of_not_valid_token_multi", "num_of_not_valid_token_multi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("invalid_free_token", "invalid_free_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("invalid_mcast_token", "invalid_mcast_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tokens_available_low_wtmk", "tokens_available_low_wtmk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ctl[] = {
            BDMFMON_MAKE_PARM("tp_mux_cntrl", "tp_mux_cntrl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_weight[] = {
            BDMFMON_MAKE_PARM("core_0_alloc_weight", "core_0_alloc_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("core_0_free_weight", "core_0_free_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("core_1_alloc_weight", "core_1_alloc_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("core_1_free_weight", "core_1_free_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_cfg[] = {
            BDMFMON_MAKE_PARM("bb_core_sel", "bb_core_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb_fast_ack", "bb_fast_ack", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb_soft_reset", "bb_soft_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_spare[] = {
            BDMFMON_MAKE_PARM("spare_bits", "spare_bits", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_prbs_invalid_gen[] = {
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg1[] = {
            BDMFMON_MAKE_PARM("pool_search_mode", "pool_search_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_jumbo_support", "enable_jumbo_support", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_0_disable", "pool_0_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_1_disable", "pool_1_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_2_disable", "pool_2_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_3_disable", "pool_3_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mark_en", "mark_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("buf_size", "buf_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg2[] = {
            BDMFMON_MAKE_PARM("pool_base_address", "pool_base_address", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_msk[] = {
            BDMFMON_MAKE_PARM("alloc_fifo_full_msk", "alloc_fifo_full_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_full_msk", "free_fifo_full_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_full_msk", "pool_full_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_no_valid_msk", "free_token_no_valid_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_msk", "free_token_index_out_of_range_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_msk", "multi_token_no_valid_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_msk", "multi_token_index_out_of_range_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_dis_free_multi_msk", "pool_dis_free_multi_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_msk", "memory_corrupt_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xoff_msk", "xoff_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xon_msk", "xon_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_address_access_msk", "illegal_address_access_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_alloc_request_msk", "illegal_alloc_request_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_det_msk", "expired_token_det_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_recov_msk", "expired_token_recov_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_sts[] = {
            BDMFMON_MAKE_PARM("alloc_fifo_full_sts", "alloc_fifo_full_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_full_sts", "free_fifo_full_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_full_sts", "pool_full_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_no_valid_sts", "free_token_no_valid_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_sts", "free_token_index_out_of_range_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_sts", "multi_token_no_valid_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_sts", "multi_token_index_out_of_range_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_dis_free_multi_sts", "pool_dis_free_multi_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_sts", "memory_corrupt_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xoff_state_sts", "xoff_state_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xon_state_sts", "xon_state_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_address_access_sts", "illegal_address_access_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_alloc_request_sts", "illegal_alloc_request_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_det_sts", "expired_token_det_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_recov_sts", "expired_token_recov_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_stall_msk[] = {
            BDMFMON_MAKE_PARM("free_token_no_valid_stall_msk", "free_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_stall_msk", "free_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_stall_msk", "multi_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_stall_msk", "multi_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_stall_msk", "memory_corrupt_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg3[] = {
            BDMFMON_MAKE_PARM("halt_alloc_timer", "halt_alloc_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_stat5[] = {
            BDMFMON_MAKE_PARM("mem_corrupt_sts_related_alloc_token", "mem_corrupt_sts_related_alloc_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xon_xoff_cfg[] = {
            BDMFMON_MAKE_PARM("xoff_threshold", "xoff_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xon_threshold", "xon_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_not_empty_cfg[] = {
            BDMFMON_MAKE_PARM("not_empty_threshold", "not_empty_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_ctl[] = {
            BDMFMON_MAKE_PARM("token_recover_ena", "token_recover_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("single_pass_ena", "single_pass_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_remark_ena", "token_remark_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_reclaim_ena", "token_reclaim_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("force_token_reclaim", "force_token_reclaim", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("clr_expired_token_count", "clr_expired_token_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("clr_recovered_token_count", "clr_recovered_token_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_short_aging_timer[] = {
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_long_aging_timer[] = {
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cache_recycle_timer[] = {
            BDMFMON_MAKE_PARM("recycle_timer", "recycle_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_start_end[] = {
            BDMFMON_MAKE_PARM("end_index", "end_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_index", "start_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_size_0_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_id", "size_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_size_1_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_id", "size_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_size_2_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_id", "size_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_size_3_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_id", "size_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_multi[] = {
            BDMFMON_MAKE_PARM("token_multi", "token_multi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("update_type", "update_type", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_1[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_2[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_3[] = {
            BDMFMON_MAKE_PARM("search3", "search3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_multicast_data[] = {
            BDMFMON_MAKE_PARM("multicast_data", "multicast_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "init_mem", .val = cli_fpm_init_mem, .parms = set_init_mem },
            { .name = "pool_en", .val = cli_fpm_pool_en, .parms = set_pool_en },
            { .name = "pool_stat", .val = cli_fpm_pool_stat, .parms = set_pool_stat },
            { .name = "ctl", .val = cli_fpm_ctl, .parms = set_ctl },
            { .name = "weight", .val = cli_fpm_weight, .parms = set_weight },
            { .name = "bb_cfg", .val = cli_fpm_bb_cfg, .parms = set_bb_cfg },
            { .name = "spare", .val = cli_fpm_spare, .parms = set_spare },
            { .name = "prbs_invalid_gen", .val = cli_fpm_prbs_invalid_gen, .parms = set_prbs_invalid_gen },
            { .name = "cfg1", .val = cli_fpm_cfg1, .parms = set_cfg1 },
            { .name = "cfg2", .val = cli_fpm_cfg2, .parms = set_cfg2 },
            { .name = "intr_msk", .val = cli_fpm_intr_msk, .parms = set_intr_msk },
            { .name = "intr_sts", .val = cli_fpm_intr_sts, .parms = set_intr_sts },
            { .name = "stall_msk", .val = cli_fpm_stall_msk, .parms = set_stall_msk },
            { .name = "cfg3", .val = cli_fpm_cfg3, .parms = set_cfg3 },
            { .name = "stat5", .val = cli_fpm_stat5, .parms = set_stat5 },
            { .name = "xon_xoff_cfg", .val = cli_fpm_xon_xoff_cfg, .parms = set_xon_xoff_cfg },
            { .name = "not_empty_cfg", .val = cli_fpm_not_empty_cfg, .parms = set_not_empty_cfg },
            { .name = "token_recover_ctl", .val = cli_fpm_token_recover_ctl, .parms = set_token_recover_ctl },
            { .name = "short_aging_timer", .val = cli_fpm_short_aging_timer, .parms = set_short_aging_timer },
            { .name = "long_aging_timer", .val = cli_fpm_long_aging_timer, .parms = set_long_aging_timer },
            { .name = "cache_recycle_timer", .val = cli_fpm_cache_recycle_timer, .parms = set_cache_recycle_timer },
            { .name = "token_recover_start_end", .val = cli_fpm_token_recover_start_end, .parms = set_token_recover_start_end },
            { .name = "size_0_alloc_dealloc", .val = cli_fpm_size_0_alloc_dealloc, .parms = set_size_0_alloc_dealloc },
            { .name = "size_1_alloc_dealloc", .val = cli_fpm_size_1_alloc_dealloc, .parms = set_size_1_alloc_dealloc },
            { .name = "size_2_alloc_dealloc", .val = cli_fpm_size_2_alloc_dealloc, .parms = set_size_2_alloc_dealloc },
            { .name = "size_3_alloc_dealloc", .val = cli_fpm_size_3_alloc_dealloc, .parms = set_size_3_alloc_dealloc },
            { .name = "multi", .val = cli_fpm_multi, .parms = set_multi },
            { .name = "search_data_1", .val = cli_fpm_search_data_1, .parms = set_search_data_1 },
            { .name = "search_data_2", .val = cli_fpm_search_data_2, .parms = set_search_data_2 },
            { .name = "search_data_3", .val = cli_fpm_search_data_3, .parms = set_search_data_3 },
            { .name = "multicast_data", .val = cli_fpm_multicast_data, .parms = set_multicast_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_fpm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_search_data_1[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_search_data_2[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_search_data_3[] = {
            BDMFMON_MAKE_PARM("search3", "search3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_multicast_data[] = {
            BDMFMON_MAKE_PARM("multicast_data", "multicast_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "init_mem", .val = cli_fpm_init_mem, .parms = get_default },
            { .name = "pool_en", .val = cli_fpm_pool_en, .parms = get_default },
            { .name = "pool_stat", .val = cli_fpm_pool_stat, .parms = get_default },
            { .name = "ctl", .val = cli_fpm_ctl, .parms = get_default },
            { .name = "weight", .val = cli_fpm_weight, .parms = get_default },
            { .name = "bb_cfg", .val = cli_fpm_bb_cfg, .parms = get_default },
            { .name = "spare", .val = cli_fpm_spare, .parms = get_default },
            { .name = "prbs_invalid_gen", .val = cli_fpm_prbs_invalid_gen, .parms = get_default },
            { .name = "cfg1", .val = cli_fpm_cfg1, .parms = get_default },
            { .name = "cfg2", .val = cli_fpm_cfg2, .parms = get_default },
            { .name = "intr_msk", .val = cli_fpm_intr_msk, .parms = get_default },
            { .name = "intr_sts", .val = cli_fpm_intr_sts, .parms = get_default },
            { .name = "stall_msk", .val = cli_fpm_stall_msk, .parms = get_default },
            { .name = "cfg3", .val = cli_fpm_cfg3, .parms = get_default },
            { .name = "stat5", .val = cli_fpm_stat5, .parms = get_default },
            { .name = "xon_xoff_cfg", .val = cli_fpm_xon_xoff_cfg, .parms = get_default },
            { .name = "not_empty_cfg", .val = cli_fpm_not_empty_cfg, .parms = get_default },
            { .name = "token_recover_ctl", .val = cli_fpm_token_recover_ctl, .parms = get_default },
            { .name = "short_aging_timer", .val = cli_fpm_short_aging_timer, .parms = get_default },
            { .name = "long_aging_timer", .val = cli_fpm_long_aging_timer, .parms = get_default },
            { .name = "cache_recycle_timer", .val = cli_fpm_cache_recycle_timer, .parms = get_default },
            { .name = "expired_token_count", .val = cli_fpm_expired_token_count, .parms = get_default },
            { .name = "recovered_token_count", .val = cli_fpm_recovered_token_count, .parms = get_default },
            { .name = "token_recover_start_end", .val = cli_fpm_token_recover_start_end, .parms = get_default },
            { .name = "size_0_alloc_dealloc", .val = cli_fpm_size_0_alloc_dealloc, .parms = get_default },
            { .name = "size_1_alloc_dealloc", .val = cli_fpm_size_1_alloc_dealloc, .parms = get_default },
            { .name = "size_2_alloc_dealloc", .val = cli_fpm_size_2_alloc_dealloc, .parms = get_default },
            { .name = "size_3_alloc_dealloc", .val = cli_fpm_size_3_alloc_dealloc, .parms = get_default },
            { .name = "search_data_1", .val = cli_fpm_search_data_1, .parms = get_search_data_1 },
            { .name = "search_data_2", .val = cli_fpm_search_data_2, .parms = get_search_data_2 },
            { .name = "search_data_3", .val = cli_fpm_search_data_3, .parms = get_search_data_3 },
            { .name = "multicast_data", .val = cli_fpm_multicast_data, .parms = get_multicast_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_fpm_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_fpm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "search_data_1", .val = cli_fpm_search_data_1, .parms = ext_test_default},
            { .name = "search_data_2", .val = cli_fpm_search_data_2, .parms = ext_test_default},
            { .name = "search_data_3", .val = cli_fpm_search_data_3, .parms = ext_test_default},
            { .name = "multicast_data", .val = cli_fpm_multicast_data, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_fpm_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CTL", .val = bdmf_address_ctl },
            { .name = "WEIGHT", .val = bdmf_address_weight },
            { .name = "BB_CFG", .val = bdmf_address_bb_cfg },
            { .name = "SPARE", .val = bdmf_address_spare },
            { .name = "PRBS_INVALID_GEN", .val = bdmf_address_prbs_invalid_gen },
            { .name = "CTL_1", .val = bdmf_address_ctl_1 },
            { .name = "CFG1", .val = bdmf_address_cfg1 },
            { .name = "CFG2", .val = bdmf_address_cfg2 },
            { .name = "INTR_MSK", .val = bdmf_address_intr_msk },
            { .name = "INTR_STS", .val = bdmf_address_intr_sts },
            { .name = "STALL_MSK", .val = bdmf_address_stall_msk },
            { .name = "CFG3", .val = bdmf_address_cfg3 },
            { .name = "STAT1", .val = bdmf_address_stat1 },
            { .name = "STAT2", .val = bdmf_address_stat2 },
            { .name = "STAT3", .val = bdmf_address_stat3 },
            { .name = "STAT4", .val = bdmf_address_stat4 },
            { .name = "STAT5", .val = bdmf_address_stat5 },
            { .name = "STAT6", .val = bdmf_address_stat6 },
            { .name = "STAT7", .val = bdmf_address_stat7 },
            { .name = "STAT8", .val = bdmf_address_stat8 },
            { .name = "XON_XOFF_CFG", .val = bdmf_address_xon_xoff_cfg },
            { .name = "NOT_EMPTY_CFG", .val = bdmf_address_not_empty_cfg },
            { .name = "TOKEN_RECOVER_CTL", .val = bdmf_address_token_recover_ctl },
            { .name = "SHORT_AGING_TIMER", .val = bdmf_address_short_aging_timer },
            { .name = "LONG_AGING_TIMER", .val = bdmf_address_long_aging_timer },
            { .name = "CACHE_RECYCLE_TIMER", .val = bdmf_address_cache_recycle_timer },
            { .name = "EXPIRED_TOKEN_COUNT", .val = bdmf_address_expired_token_count },
            { .name = "RECOVERED_TOKEN_COUNT", .val = bdmf_address_recovered_token_count },
            { .name = "TOKEN_RECOVER_START_END", .val = bdmf_address_token_recover_start_end },
            { .name = "SIZE_0_ALLOC_DEALLOC", .val = bdmf_address_size_0_alloc_dealloc },
            { .name = "SIZE_1_ALLOC_DEALLOC", .val = bdmf_address_size_1_alloc_dealloc },
            { .name = "SIZE_2_ALLOC_DEALLOC", .val = bdmf_address_size_2_alloc_dealloc },
            { .name = "SIZE_3_ALLOC_DEALLOC", .val = bdmf_address_size_3_alloc_dealloc },
            { .name = "MULTI", .val = bdmf_address_multi },
            { .name = "SEARCH_DATA_0", .val = bdmf_address_search_data_0 },
            { .name = "SEARCH_DATA_1", .val = bdmf_address_search_data_1 },
            { .name = "SEARCH_DATA_2", .val = bdmf_address_search_data_2 },
            { .name = "SEARCH_DATA_3", .val = bdmf_address_search_data_3 },
            { .name = "MULTICAST_DATA", .val = bdmf_address_multicast_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_fpm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
