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
    uint32_t reg_fpm_ctl = 0;

#ifdef VALIDATE_PARMS
    if ((init_mem >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    reg_fpm_ctl = RU_FIELD_SET(0, FPM, FPM_CTL, INIT_MEM, reg_fpm_ctl, init_mem);

    RU_REG_WRITE(0, FPM, FPM_CTL, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_init_mem_get(bdmf_boolean *init_mem)
{
    uint32_t reg_fpm_ctl;

#ifdef VALIDATE_PARMS
    if (!init_mem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    *init_mem = RU_FIELD_GET(0, FPM, FPM_CTL, INIT_MEM, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_en_set(bdmf_boolean pool1_enable)
{
    uint32_t reg_fpm_ctl = 0;

#ifdef VALIDATE_PARMS
    if ((pool1_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    reg_fpm_ctl = RU_FIELD_SET(0, FPM, FPM_CTL, POOL1_ENABLE, reg_fpm_ctl, pool1_enable);

    RU_REG_WRITE(0, FPM, FPM_CTL, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_en_get(bdmf_boolean *pool1_enable)
{
    uint32_t reg_fpm_ctl;

#ifdef VALIDATE_PARMS
    if (!pool1_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    *pool1_enable = RU_FIELD_GET(0, FPM, FPM_CTL, POOL1_ENABLE, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_bb_reset_set(bdmf_boolean fpm_bb_soft_reset)
{
    uint32_t reg_fpm_ctl = 0;

#ifdef VALIDATE_PARMS
    if ((fpm_bb_soft_reset >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    reg_fpm_ctl = RU_FIELD_SET(0, FPM, FPM_CTL, FPM_BB_SOFT_RESET, reg_fpm_ctl, fpm_bb_soft_reset);

    RU_REG_WRITE(0, FPM, FPM_CTL, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_bb_reset_get(bdmf_boolean *fpm_bb_soft_reset)
{
    uint32_t reg_fpm_ctl;

#ifdef VALIDATE_PARMS
    if (!fpm_bb_soft_reset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    *fpm_bb_soft_reset = RU_FIELD_GET(0, FPM, FPM_CTL, FPM_BB_SOFT_RESET, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ddr0_weight_set(uint8_t ddr0_alloc_weight, uint8_t ddr0_free_weight)
{
    uint32_t reg_fpm_weight = 0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    reg_fpm_weight = RU_FIELD_SET(0, FPM, FPM_WEIGHT, DDR0_ALLOC_WEIGHT, reg_fpm_weight, ddr0_alloc_weight);
    reg_fpm_weight = RU_FIELD_SET(0, FPM, FPM_WEIGHT, DDR0_FREE_WEIGHT, reg_fpm_weight, ddr0_free_weight);

    RU_REG_WRITE(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ddr0_weight_get(uint8_t *ddr0_alloc_weight, uint8_t *ddr0_free_weight)
{
    uint32_t reg_fpm_weight;

#ifdef VALIDATE_PARMS
    if (!ddr0_alloc_weight || !ddr0_free_weight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    *ddr0_alloc_weight = RU_FIELD_GET(0, FPM, FPM_WEIGHT, DDR0_ALLOC_WEIGHT, reg_fpm_weight);
    *ddr0_free_weight = RU_FIELD_GET(0, FPM, FPM_WEIGHT, DDR0_FREE_WEIGHT, reg_fpm_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ddr1_weight_set(uint8_t ddr1_alloc_weight, uint8_t ddr1_free_weight)
{
    uint32_t reg_fpm_weight = 0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    reg_fpm_weight = RU_FIELD_SET(0, FPM, FPM_WEIGHT, DDR1_ALLOC_WEIGHT, reg_fpm_weight, ddr1_alloc_weight);
    reg_fpm_weight = RU_FIELD_SET(0, FPM, FPM_WEIGHT, DDR1_FREE_WEIGHT, reg_fpm_weight, ddr1_free_weight);

    RU_REG_WRITE(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ddr1_weight_get(uint8_t *ddr1_alloc_weight, uint8_t *ddr1_free_weight)
{
    uint32_t reg_fpm_weight;

#ifdef VALIDATE_PARMS
    if (!ddr1_alloc_weight || !ddr1_free_weight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_WEIGHT, reg_fpm_weight);

    *ddr1_alloc_weight = RU_FIELD_GET(0, FPM, FPM_WEIGHT, DDR1_ALLOC_WEIGHT, reg_fpm_weight);
    *ddr1_free_weight = RU_FIELD_GET(0, FPM, FPM_WEIGHT, DDR1_FREE_WEIGHT, reg_fpm_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_cfg_set(const fpm_pool_cfg *pool_cfg)
{
    uint32_t reg_pool1_cfg1 = 0;
    uint32_t reg_pool1_cfg2 = 0;
    uint32_t reg_pool1_cfg3 = 0;

#ifdef VALIDATE_PARMS
    if(!pool_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_cfg->fp_buf_size >= _3BITS_MAX_VAL_) ||
       (pool_cfg->pool_base_address >= _30BITS_MAX_VAL_) ||
       (pool_cfg->pool_base_address_pool2 >= _30BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool1_cfg1 = RU_FIELD_SET(0, FPM, POOL1_CFG1, FP_BUF_SIZE, reg_pool1_cfg1, pool_cfg->fp_buf_size);
    reg_pool1_cfg2 = RU_FIELD_SET(0, FPM, POOL1_CFG2, POOL_BASE_ADDRESS, reg_pool1_cfg2, pool_cfg->pool_base_address);
    reg_pool1_cfg3 = RU_FIELD_SET(0, FPM, POOL1_CFG3, POOL_BASE_ADDRESS_POOL2, reg_pool1_cfg3, pool_cfg->pool_base_address_pool2);

    RU_REG_WRITE(0, FPM, POOL1_CFG1, reg_pool1_cfg1);
    RU_REG_WRITE(0, FPM, POOL1_CFG2, reg_pool1_cfg2);
    RU_REG_WRITE(0, FPM, POOL1_CFG3, reg_pool1_cfg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_cfg_get(fpm_pool_cfg *pool_cfg)
{
    uint32_t reg_pool1_cfg1;
    uint32_t reg_pool1_cfg2;
    uint32_t reg_pool1_cfg3;

#ifdef VALIDATE_PARMS
    if (!pool_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_CFG1, reg_pool1_cfg1);
    RU_REG_READ(0, FPM, POOL1_CFG2, reg_pool1_cfg2);
    RU_REG_READ(0, FPM, POOL1_CFG3, reg_pool1_cfg3);

    pool_cfg->fp_buf_size = RU_FIELD_GET(0, FPM, POOL1_CFG1, FP_BUF_SIZE, reg_pool1_cfg1);
    pool_cfg->pool_base_address = RU_FIELD_GET(0, FPM, POOL1_CFG2, POOL_BASE_ADDRESS, reg_pool1_cfg2);
    pool_cfg->pool_base_address_pool2 = RU_FIELD_GET(0, FPM, POOL1_CFG3, POOL_BASE_ADDRESS_POOL2, reg_pool1_cfg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_stat_set(const fpm_pool_stat *pool_stat)
{
    uint32_t reg_pool1_stat1 = 0;
    uint32_t reg_pool1_stat2 = 0;
    uint32_t reg_pool1_stat3 = 0;
    uint32_t reg_pool1_stat4 = 0;
    uint32_t reg_pool1_stat6 = 0;
    uint32_t reg_pool1_stat7 = 0;
    uint32_t reg_pool1_stat8 = 0;

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

    reg_pool1_stat1 = RU_FIELD_SET(0, FPM, POOL1_STAT1, OVRFL, reg_pool1_stat1, pool_stat->ovrfl);
    reg_pool1_stat1 = RU_FIELD_SET(0, FPM, POOL1_STAT1, UNDRFL, reg_pool1_stat1, pool_stat->undrfl);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, POOL_FULL, reg_pool1_stat2, pool_stat->pool_full);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, FREE_FIFO_FULL, reg_pool1_stat2, pool_stat->free_fifo_full);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, FREE_FIFO_EMPTY, reg_pool1_stat2, pool_stat->free_fifo_empty);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, ALLOC_FIFO_FULL, reg_pool1_stat2, pool_stat->alloc_fifo_full);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, ALLOC_FIFO_EMPTY, reg_pool1_stat2, pool_stat->alloc_fifo_empty);
    reg_pool1_stat2 = RU_FIELD_SET(0, FPM, POOL1_STAT2, NUM_OF_TOKENS_AVAILABLE, reg_pool1_stat2, pool_stat->num_of_tokens_available);
    reg_pool1_stat3 = RU_FIELD_SET(0, FPM, POOL1_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool1_stat3, pool_stat->num_of_not_valid_token_frees);
    reg_pool1_stat4 = RU_FIELD_SET(0, FPM, POOL1_STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_pool1_stat4, pool_stat->num_of_not_valid_token_multi);
    reg_pool1_stat6 = RU_FIELD_SET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN, reg_pool1_stat6, pool_stat->invalid_free_token);
    reg_pool1_stat7 = RU_FIELD_SET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN, reg_pool1_stat7, pool_stat->invalid_mcast_token);
    reg_pool1_stat8 = RU_FIELD_SET(0, FPM, POOL1_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool1_stat8, pool_stat->tokens_available_low_wtmk);

    RU_REG_WRITE(0, FPM, POOL1_STAT1, reg_pool1_stat1);
    RU_REG_WRITE(0, FPM, POOL1_STAT2, reg_pool1_stat2);
    RU_REG_WRITE(0, FPM, POOL1_STAT3, reg_pool1_stat3);
    RU_REG_WRITE(0, FPM, POOL1_STAT4, reg_pool1_stat4);
    RU_REG_WRITE(0, FPM, POOL1_STAT6, reg_pool1_stat6);
    RU_REG_WRITE(0, FPM, POOL1_STAT7, reg_pool1_stat7);
    RU_REG_WRITE(0, FPM, POOL1_STAT8, reg_pool1_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_stat_get(fpm_pool_stat *pool_stat)
{
    uint32_t reg_pool1_stat1;
    uint32_t reg_pool1_stat2;
    uint32_t reg_pool1_stat3;
    uint32_t reg_pool1_stat4;
    uint32_t reg_pool1_stat6;
    uint32_t reg_pool1_stat7;
    uint32_t reg_pool1_stat8;

#ifdef VALIDATE_PARMS
    if (!pool_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_STAT1, reg_pool1_stat1);
    RU_REG_READ(0, FPM, POOL1_STAT2, reg_pool1_stat2);
    RU_REG_READ(0, FPM, POOL1_STAT3, reg_pool1_stat3);
    RU_REG_READ(0, FPM, POOL1_STAT4, reg_pool1_stat4);
    RU_REG_READ(0, FPM, POOL1_STAT6, reg_pool1_stat6);
    RU_REG_READ(0, FPM, POOL1_STAT7, reg_pool1_stat7);
    RU_REG_READ(0, FPM, POOL1_STAT8, reg_pool1_stat8);

    pool_stat->ovrfl = RU_FIELD_GET(0, FPM, POOL1_STAT1, OVRFL, reg_pool1_stat1);
    pool_stat->undrfl = RU_FIELD_GET(0, FPM, POOL1_STAT1, UNDRFL, reg_pool1_stat1);
    pool_stat->pool_full = RU_FIELD_GET(0, FPM, POOL1_STAT2, POOL_FULL, reg_pool1_stat2);
    pool_stat->free_fifo_full = RU_FIELD_GET(0, FPM, POOL1_STAT2, FREE_FIFO_FULL, reg_pool1_stat2);
    pool_stat->free_fifo_empty = RU_FIELD_GET(0, FPM, POOL1_STAT2, FREE_FIFO_EMPTY, reg_pool1_stat2);
    pool_stat->alloc_fifo_full = RU_FIELD_GET(0, FPM, POOL1_STAT2, ALLOC_FIFO_FULL, reg_pool1_stat2);
    pool_stat->alloc_fifo_empty = RU_FIELD_GET(0, FPM, POOL1_STAT2, ALLOC_FIFO_EMPTY, reg_pool1_stat2);
    pool_stat->num_of_tokens_available = RU_FIELD_GET(0, FPM, POOL1_STAT2, NUM_OF_TOKENS_AVAILABLE, reg_pool1_stat2);
    pool_stat->num_of_not_valid_token_frees = RU_FIELD_GET(0, FPM, POOL1_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool1_stat3);
    pool_stat->num_of_not_valid_token_multi = RU_FIELD_GET(0, FPM, POOL1_STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_pool1_stat4);
    pool_stat->invalid_free_token = RU_FIELD_GET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN, reg_pool1_stat6);
    pool_stat->invalid_mcast_token = RU_FIELD_GET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN, reg_pool1_stat7);
    pool_stat->tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, POOL1_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool1_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool4_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (token_size >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool4_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_VALID, reg_pool4_alloc_dealloc, token_valid);
    reg_pool4_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC, DDR, reg_pool4_alloc_dealloc, ddr);
    reg_pool4_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool4_alloc_dealloc, token_index);
    reg_pool4_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool4_alloc_dealloc, token_size);

    RU_REG_WRITE(0, FPM, POOL4_ALLOC_DEALLOC, reg_pool4_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size)
{
    uint32_t reg_pool4_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_valid || !ddr || !token_index || !token_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL4_ALLOC_DEALLOC, reg_pool4_alloc_dealloc);

    *token_valid = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_VALID, reg_pool4_alloc_dealloc);
    *ddr = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC, DDR, reg_pool4_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool4_alloc_dealloc);
    *token_size = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool4_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool3_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (token_size >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_VALID, reg_pool3_alloc_dealloc, token_valid);
    reg_pool3_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC, DDR, reg_pool3_alloc_dealloc, ddr);
    reg_pool3_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool3_alloc_dealloc, token_index);
    reg_pool3_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool3_alloc_dealloc, token_size);

    RU_REG_WRITE(0, FPM, POOL3_ALLOC_DEALLOC, reg_pool3_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size)
{
    uint32_t reg_pool3_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_valid || !ddr || !token_index || !token_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_ALLOC_DEALLOC, reg_pool3_alloc_dealloc);

    *token_valid = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_VALID, reg_pool3_alloc_dealloc);
    *ddr = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC, DDR, reg_pool3_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool3_alloc_dealloc);
    *token_size = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool3_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool2_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (token_size >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_VALID, reg_pool2_alloc_dealloc, token_valid);
    reg_pool2_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC, DDR, reg_pool2_alloc_dealloc, ddr);
    reg_pool2_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool2_alloc_dealloc, token_index);
    reg_pool2_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool2_alloc_dealloc, token_size);

    RU_REG_WRITE(0, FPM, POOL2_ALLOC_DEALLOC, reg_pool2_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size)
{
    uint32_t reg_pool2_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_valid || !ddr || !token_index || !token_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_ALLOC_DEALLOC, reg_pool2_alloc_dealloc);

    *token_valid = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_VALID, reg_pool2_alloc_dealloc);
    *ddr = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC, DDR, reg_pool2_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool2_alloc_dealloc);
    *token_size = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool2_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool1_alloc_dealloc = 0;

#ifdef VALIDATE_PARMS
    if ((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (token_size >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool1_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_VALID, reg_pool1_alloc_dealloc, token_valid);
    reg_pool1_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC, DDR, reg_pool1_alloc_dealloc, ddr);
    reg_pool1_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool1_alloc_dealloc, token_index);
    reg_pool1_alloc_dealloc = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool1_alloc_dealloc, token_size);

    RU_REG_WRITE(0, FPM, POOL1_ALLOC_DEALLOC, reg_pool1_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size)
{
    uint32_t reg_pool1_alloc_dealloc;

#ifdef VALIDATE_PARMS
    if (!token_valid || !ddr || !token_index || !token_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_ALLOC_DEALLOC, reg_pool1_alloc_dealloc);

    *token_valid = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_VALID, reg_pool1_alloc_dealloc);
    *ddr = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC, DDR, reg_pool1_alloc_dealloc);
    *token_index = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_INDEX, reg_pool1_alloc_dealloc);
    *token_size = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC, TOKEN_SIZE, reg_pool1_alloc_dealloc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_back_door_mem_set(uint32_t mem_data1, uint32_t mem_data2)
{
    uint32_t reg_mem_data1 = 0;
    uint32_t reg_mem_data2 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_mem_data1 = RU_FIELD_SET(0, FPM, MEM_DATA1, MEM_DATA1, reg_mem_data1, mem_data1);
    reg_mem_data2 = RU_FIELD_SET(0, FPM, MEM_DATA2, MEM_DATA2, reg_mem_data2, mem_data2);

    RU_REG_WRITE(0, FPM, MEM_DATA1, reg_mem_data1);
    RU_REG_WRITE(0, FPM, MEM_DATA2, reg_mem_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_back_door_mem_get(uint32_t *mem_data1, uint32_t *mem_data2)
{
    uint32_t reg_mem_data1;
    uint32_t reg_mem_data2;

#ifdef VALIDATE_PARMS
    if (!mem_data1 || !mem_data2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, MEM_DATA1, reg_mem_data1);
    RU_REG_READ(0, FPM, MEM_DATA2, reg_mem_data2);

    *mem_data1 = RU_FIELD_GET(0, FPM, MEM_DATA1, MEM_DATA1, reg_mem_data1);
    *mem_data2 = RU_FIELD_GET(0, FPM, MEM_DATA2, MEM_DATA2, reg_mem_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_count_get(uint32_t *expired_count, uint32_t *recovered_count)
{
    uint32_t reg_expired_token_count_pool1;
    uint32_t reg_recovered_token_count_pool1;

#ifdef VALIDATE_PARMS
    if (!expired_count || !recovered_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, EXPIRED_TOKEN_COUNT_POOL1, reg_expired_token_count_pool1);
    RU_REG_READ(0, FPM, RECOVERED_TOKEN_COUNT_POOL1, reg_recovered_token_count_pool1);

    *expired_count = RU_FIELD_GET(0, FPM, EXPIRED_TOKEN_COUNT_POOL1, COUNT, reg_expired_token_count_pool1);
    *recovered_count = RU_FIELD_GET(0, FPM, RECOVERED_TOKEN_COUNT_POOL1, COUNT, reg_recovered_token_count_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_count_get(uint32_t *expired_count, uint32_t *recovered_count)
{
    uint32_t reg_expired_token_count_pool2;
    uint32_t reg_recovered_token_count_pool2;

#ifdef VALIDATE_PARMS
    if (!expired_count || !recovered_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, EXPIRED_TOKEN_COUNT_POOL2, reg_expired_token_count_pool2);
    RU_REG_READ(0, FPM, RECOVERED_TOKEN_COUNT_POOL2, reg_recovered_token_count_pool2);

    *expired_count = RU_FIELD_GET(0, FPM, EXPIRED_TOKEN_COUNT_POOL2, COUNT, reg_expired_token_count_pool2);
    *recovered_count = RU_FIELD_GET(0, FPM, RECOVERED_TOKEN_COUNT_POOL2, COUNT, reg_recovered_token_count_pool2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_timer_set(const fpm_timer *timer)
{
    uint32_t reg_long_aging_timer = 0;
    uint32_t reg_short_aging_timer = 0;
    uint32_t reg_cache_recycle_timer = 0;

#ifdef VALIDATE_PARMS
    if(!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    reg_long_aging_timer = RU_FIELD_SET(0, FPM, LONG_AGING_TIMER, TIMER, reg_long_aging_timer, timer->long_aging_timer);
    reg_short_aging_timer = RU_FIELD_SET(0, FPM, SHORT_AGING_TIMER, TIMER, reg_short_aging_timer, timer->short_aging_timer);
    reg_cache_recycle_timer = RU_FIELD_SET(0, FPM, CACHE_RECYCLE_TIMER, RECYCLE_TIMER, reg_cache_recycle_timer, timer->recycle_timer);

    RU_REG_WRITE(0, FPM, LONG_AGING_TIMER, reg_long_aging_timer);
    RU_REG_WRITE(0, FPM, SHORT_AGING_TIMER, reg_short_aging_timer);
    RU_REG_WRITE(0, FPM, CACHE_RECYCLE_TIMER, reg_cache_recycle_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_timer_get(fpm_timer *timer)
{
    uint32_t reg_long_aging_timer;
    uint32_t reg_short_aging_timer;
    uint32_t reg_cache_recycle_timer;

#ifdef VALIDATE_PARMS
    if (!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, LONG_AGING_TIMER, reg_long_aging_timer);
    RU_REG_READ(0, FPM, SHORT_AGING_TIMER, reg_short_aging_timer);
    RU_REG_READ(0, FPM, CACHE_RECYCLE_TIMER, reg_cache_recycle_timer);

    timer->long_aging_timer = RU_FIELD_GET(0, FPM, LONG_AGING_TIMER, TIMER, reg_long_aging_timer);
    timer->short_aging_timer = RU_FIELD_GET(0, FPM, SHORT_AGING_TIMER, TIMER, reg_short_aging_timer);
    timer->recycle_timer = RU_FIELD_GET(0, FPM, CACHE_RECYCLE_TIMER, RECYCLE_TIMER, reg_cache_recycle_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_cfg1_set(bdmf_boolean pool1_search_mode, bdmf_boolean enable_jumbo_support)
{
    uint32_t reg_fpm_cfg1 = 0;

#ifdef VALIDATE_PARMS
    if ((pool1_search_mode >= _1BITS_MAX_VAL_) ||
       (enable_jumbo_support >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL1_SEARCH_MODE, reg_fpm_cfg1, pool1_search_mode);
    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, ENABLE_JUMBO_SUPPORT, reg_fpm_cfg1, enable_jumbo_support);

    RU_REG_WRITE(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_cfg1_get(bdmf_boolean *pool1_search_mode, bdmf_boolean *enable_jumbo_support)
{
    uint32_t reg_fpm_cfg1;

#ifdef VALIDATE_PARMS
    if (!pool1_search_mode || !enable_jumbo_support)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    *pool1_search_mode = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL1_SEARCH_MODE, reg_fpm_cfg1);
    *enable_jumbo_support = RU_FIELD_GET(0, FPM, FPM_CFG1, ENABLE_JUMBO_SUPPORT, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_cfg_set(uint8_t bb_ddr_sel)
{
    uint32_t reg_fpm_bb_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((bb_ddr_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_cfg = RU_FIELD_SET(0, FPM, FPM_BB_CFG, BB_DDR_SEL, reg_fpm_bb_cfg, bb_ddr_sel);

    RU_REG_WRITE(0, FPM, FPM_BB_CFG, reg_fpm_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_cfg_get(uint8_t *bb_ddr_sel)
{
    uint32_t reg_fpm_bb_cfg;

#ifdef VALIDATE_PARMS
    if (!bb_ddr_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_CFG, reg_fpm_bb_cfg);

    *bb_ddr_sel = RU_FIELD_GET(0, FPM, FPM_BB_CFG, BB_DDR_SEL, reg_fpm_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_msk_set(const fpm_pool2_intr_msk *pool2_intr_msk)
{
    /* Identical to pool2_intr_msk */
    uint32_t reg_pool2_intr_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->alloc_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_dis_free_multi_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk, pool2_intr_msk->memory_corrupt_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk, pool2_intr_msk->xoff_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk, pool2_intr_msk->xon_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_address_access_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_alloc_request_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_det_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_recov_msk);

    RU_REG_WRITE(0, FPM, POOL1_INTR_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk)
{
    /* Identical to pool2_intr_msk */
    uint32_t reg_pool2_intr_msk;

#ifdef VALIDATE_PARMS
    if (!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_INTR_MSK, reg_pool2_intr_msk);

    pool2_intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xon_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts)
{
    /* Identical to pool2_intr_sts */
    uint32_t reg_pool2_intr_sts = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->alloc_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->free_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_dis_free_multi_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts, pool2_intr_sts->memory_corrupt_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xoff_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xon_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_address_access_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_alloc_request_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_det_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_recov_sts);

    RU_REG_WRITE(0, FPM, POOL1_INTR_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts)
{
    /* Identical to pool2_intr_sts */
    uint32_t reg_pool2_intr_sts;

#ifdef VALIDATE_PARMS
    if (!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_INTR_STS, reg_pool2_intr_sts);

    pool2_intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts);
    pool2_intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk)
{
    /* Identical to pool2_stall_msk */
    uint32_t reg_pool2_stall_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->memory_corrupt_stall_msk);

    RU_REG_WRITE(0, FPM, POOL1_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk)
{
    /* Identical to pool2_stall_msk */
    uint32_t reg_pool2_stall_msk;

#ifdef VALIDATE_PARMS
    if (!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_STALL_MSK, reg_pool2_stall_msk);

    pool2_stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_msk_set(const fpm_pool2_intr_msk *pool2_intr_msk)
{
    uint32_t reg_pool2_intr_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->alloc_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_dis_free_multi_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk, pool2_intr_msk->memory_corrupt_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk, pool2_intr_msk->xoff_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk, pool2_intr_msk->xon_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_address_access_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_alloc_request_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_det_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_recov_msk);

    RU_REG_WRITE(0, FPM, POOL2_INTR_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk)
{
    uint32_t reg_pool2_intr_msk;

#ifdef VALIDATE_PARMS
    if (!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_INTR_MSK, reg_pool2_intr_msk);

    pool2_intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xon_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts)
{
    uint32_t reg_pool2_intr_sts = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->alloc_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->free_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_dis_free_multi_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts, pool2_intr_sts->memory_corrupt_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xoff_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xon_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_address_access_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_alloc_request_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_det_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_recov_sts);

    RU_REG_WRITE(0, FPM, POOL2_INTR_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts)
{
    uint32_t reg_pool2_intr_sts;

#ifdef VALIDATE_PARMS
    if (!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_INTR_STS, reg_pool2_intr_sts);

    pool2_intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts);
    pool2_intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk)
{
    uint32_t reg_pool2_stall_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool2_stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->memory_corrupt_stall_msk);

    RU_REG_WRITE(0, FPM, POOL2_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk)
{
    uint32_t reg_pool2_stall_msk;

#ifdef VALIDATE_PARMS
    if (!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STALL_MSK, reg_pool2_stall_msk);

    pool2_stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_intr_msk_set(const fpm_pool3_intr_msk *pool3_intr_msk)
{
    uint32_t reg_pool3_intr_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool3_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool3_intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (pool3_intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool3_intr_msk, pool3_intr_msk->alloc_fifo_full_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool3_intr_msk, pool3_intr_msk->free_fifo_full_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, POOL_FULL_MSK, reg_pool3_intr_msk, pool3_intr_msk->pool_full_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool3_intr_msk, pool3_intr_msk->free_token_no_valid_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool3_intr_msk, pool3_intr_msk->free_token_index_out_of_range_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool3_intr_msk, pool3_intr_msk->multi_token_no_valid_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool3_intr_msk, pool3_intr_msk->multi_token_index_out_of_range_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool3_intr_msk, pool3_intr_msk->pool_dis_free_multi_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool3_intr_msk, pool3_intr_msk->memory_corrupt_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, XOFF_MSK, reg_pool3_intr_msk, pool3_intr_msk->xoff_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, XON_MSK, reg_pool3_intr_msk, pool3_intr_msk->xon_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool3_intr_msk, pool3_intr_msk->illegal_address_access_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool3_intr_msk, pool3_intr_msk->illegal_alloc_request_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool3_intr_msk, pool3_intr_msk->expired_token_det_msk);
    reg_pool3_intr_msk = RU_FIELD_SET(0, FPM, POOL3_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool3_intr_msk, pool3_intr_msk->expired_token_recov_msk);

    RU_REG_WRITE(0, FPM, POOL3_INTR_MSK, reg_pool3_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_intr_msk_get(fpm_pool3_intr_msk *pool3_intr_msk)
{
    uint32_t reg_pool3_intr_msk;

#ifdef VALIDATE_PARMS
    if (!pool3_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_INTR_MSK, reg_pool3_intr_msk);

    pool3_intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, POOL_FULL_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, XOFF_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->xon_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, XON_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool3_intr_msk);
    pool3_intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, POOL3_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool3_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_intr_sts_set(const fpm_pool3_intr_sts *pool3_intr_sts)
{
    uint32_t reg_pool3_intr_sts = 0;

#ifdef VALIDATE_PARMS
    if(!pool3_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool3_intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (pool3_intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool3_intr_sts, pool3_intr_sts->alloc_fifo_full_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, FREE_FIFO_FULL_STS, reg_pool3_intr_sts, pool3_intr_sts->free_fifo_full_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, POOL_FULL_STS, reg_pool3_intr_sts, pool3_intr_sts->pool_full_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool3_intr_sts, pool3_intr_sts->free_token_no_valid_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool3_intr_sts, pool3_intr_sts->free_token_index_out_of_range_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool3_intr_sts, pool3_intr_sts->multi_token_no_valid_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool3_intr_sts, pool3_intr_sts->multi_token_index_out_of_range_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool3_intr_sts, pool3_intr_sts->pool_dis_free_multi_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, MEMORY_CORRUPT_STS, reg_pool3_intr_sts, pool3_intr_sts->memory_corrupt_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, XOFF_STATE_STS, reg_pool3_intr_sts, pool3_intr_sts->xoff_state_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, XON_STATE_STS, reg_pool3_intr_sts, pool3_intr_sts->xon_state_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool3_intr_sts, pool3_intr_sts->illegal_address_access_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool3_intr_sts, pool3_intr_sts->illegal_alloc_request_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool3_intr_sts, pool3_intr_sts->expired_token_det_sts);
    reg_pool3_intr_sts = RU_FIELD_SET(0, FPM, POOL3_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool3_intr_sts, pool3_intr_sts->expired_token_recov_sts);

    RU_REG_WRITE(0, FPM, POOL3_INTR_STS, reg_pool3_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_intr_sts_get(fpm_pool3_intr_sts *pool3_intr_sts)
{
    uint32_t reg_pool3_intr_sts;

#ifdef VALIDATE_PARMS
    if (!pool3_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_INTR_STS, reg_pool3_intr_sts);

    pool3_intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool3_intr_sts);
    pool3_intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, FREE_FIFO_FULL_STS, reg_pool3_intr_sts);
    pool3_intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, POOL_FULL_STS, reg_pool3_intr_sts);
    pool3_intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool3_intr_sts);
    pool3_intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool3_intr_sts);
    pool3_intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool3_intr_sts);
    pool3_intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool3_intr_sts);
    pool3_intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool3_intr_sts);
    pool3_intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, MEMORY_CORRUPT_STS, reg_pool3_intr_sts);
    pool3_intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, XOFF_STATE_STS, reg_pool3_intr_sts);
    pool3_intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, XON_STATE_STS, reg_pool3_intr_sts);
    pool3_intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool3_intr_sts);
    pool3_intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool3_intr_sts);
    pool3_intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool3_intr_sts);
    pool3_intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, POOL3_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool3_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stall_msk_set(const fpm_pool3_stall_msk *pool3_stall_msk)
{
    uint32_t reg_pool3_stall_msk = 0;

#ifdef VALIDATE_PARMS
    if(!pool3_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool3_stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool3_stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool3_stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool3_stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool3_stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_stall_msk = RU_FIELD_SET(0, FPM, POOL3_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool3_stall_msk, pool3_stall_msk->free_token_no_valid_stall_msk);
    reg_pool3_stall_msk = RU_FIELD_SET(0, FPM, POOL3_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool3_stall_msk, pool3_stall_msk->free_token_index_out_of_range_stall_msk);
    reg_pool3_stall_msk = RU_FIELD_SET(0, FPM, POOL3_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool3_stall_msk, pool3_stall_msk->multi_token_no_valid_stall_msk);
    reg_pool3_stall_msk = RU_FIELD_SET(0, FPM, POOL3_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool3_stall_msk, pool3_stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_pool3_stall_msk = RU_FIELD_SET(0, FPM, POOL3_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool3_stall_msk, pool3_stall_msk->memory_corrupt_stall_msk);

    RU_REG_WRITE(0, FPM, POOL3_STALL_MSK, reg_pool3_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stall_msk_get(fpm_pool3_stall_msk *pool3_stall_msk)
{
    uint32_t reg_pool3_stall_msk;

#ifdef VALIDATE_PARMS
    if (!pool3_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STALL_MSK, reg_pool3_stall_msk);

    pool3_stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL3_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool3_stall_msk);
    pool3_stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL3_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool3_stall_msk);
    pool3_stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL3_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool3_stall_msk);
    pool3_stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL3_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool3_stall_msk);
    pool3_stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, POOL3_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool3_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_cfg4_set(uint32_t pool_base_address_pool3)
{
    uint32_t reg_pool1_cfg4 = 0;

#ifdef VALIDATE_PARMS
    if ((pool_base_address_pool3 >= _30BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool1_cfg4 = RU_FIELD_SET(0, FPM, POOL1_CFG4, POOL_BASE_ADDRESS_POOL3, reg_pool1_cfg4, pool_base_address_pool3);

    RU_REG_WRITE(0, FPM, POOL1_CFG4, reg_pool1_cfg4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_cfg4_get(uint32_t *pool_base_address_pool3)
{
    uint32_t reg_pool1_cfg4;

#ifdef VALIDATE_PARMS
    if (!pool_base_address_pool3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_CFG4, reg_pool1_cfg4);

    *pool_base_address_pool3 = RU_FIELD_GET(0, FPM, POOL1_CFG4, POOL_BASE_ADDRESS_POOL3, reg_pool1_cfg4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stat5_set(uint32_t mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool1_stat5 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool1_stat5 = RU_FIELD_SET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool1_stat5, mem_corrupt_sts_related_alloc_token);

    RU_REG_WRITE(0, FPM, POOL1_STAT5, reg_pool1_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stat5_get(uint32_t *mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool1_stat5;

#ifdef VALIDATE_PARMS
    if (!mem_corrupt_sts_related_alloc_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_STAT5, reg_pool1_stat5);

    *mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool1_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat1_set(uint16_t undrfl, uint16_t ovrfl)
{
    uint32_t reg_pool2_stat1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool2_stat1 = RU_FIELD_SET(0, FPM, POOL2_STAT1, UNDRFL, reg_pool2_stat1, undrfl);
    reg_pool2_stat1 = RU_FIELD_SET(0, FPM, POOL2_STAT1, OVRFL, reg_pool2_stat1, ovrfl);

    RU_REG_WRITE(0, FPM, POOL2_STAT1, reg_pool2_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat1_get(uint16_t *undrfl, uint16_t *ovrfl)
{
    uint32_t reg_pool2_stat1;

#ifdef VALIDATE_PARMS
    if (!undrfl || !ovrfl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT1, reg_pool2_stat1);

    *undrfl = RU_FIELD_GET(0, FPM, POOL2_STAT1, UNDRFL, reg_pool2_stat1);
    *ovrfl = RU_FIELD_GET(0, FPM, POOL2_STAT1, OVRFL, reg_pool2_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat2_get(fpm_pool2_stat2 *pool2_stat2)
{
    uint32_t reg_pool2_stat2;

#ifdef VALIDATE_PARMS
    if (!pool2_stat2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT2, reg_pool2_stat2);

    pool2_stat2->num_of_tokens_available = RU_FIELD_GET(0, FPM, POOL2_STAT2, NUM_OF_TOKENS_AVAILABLE, reg_pool2_stat2);
    pool2_stat2->alloc_fifo_empty = RU_FIELD_GET(0, FPM, POOL2_STAT2, ALLOC_FIFO_EMPTY, reg_pool2_stat2);
    pool2_stat2->alloc_fifo_full = RU_FIELD_GET(0, FPM, POOL2_STAT2, ALLOC_FIFO_FULL, reg_pool2_stat2);
    pool2_stat2->free_fifo_empty = RU_FIELD_GET(0, FPM, POOL2_STAT2, FREE_FIFO_EMPTY, reg_pool2_stat2);
    pool2_stat2->free_fifo_full = RU_FIELD_GET(0, FPM, POOL2_STAT2, FREE_FIFO_FULL, reg_pool2_stat2);
    pool2_stat2->pool_full = RU_FIELD_GET(0, FPM, POOL2_STAT2, POOL_FULL, reg_pool2_stat2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat3_set(uint32_t num_of_not_valid_token_frees)
{
    uint32_t reg_pool2_stat3 = 0;

#ifdef VALIDATE_PARMS
    if ((num_of_not_valid_token_frees >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stat3 = RU_FIELD_SET(0, FPM, POOL2_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool2_stat3, num_of_not_valid_token_frees);

    RU_REG_WRITE(0, FPM, POOL2_STAT3, reg_pool2_stat3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat3_get(uint32_t *num_of_not_valid_token_frees)
{
    uint32_t reg_pool2_stat3;

#ifdef VALIDATE_PARMS
    if (!num_of_not_valid_token_frees)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT3, reg_pool2_stat3);

    *num_of_not_valid_token_frees = RU_FIELD_GET(0, FPM, POOL2_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool2_stat3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat4_set(uint32_t num_of_not_valid_token_multi)
{
    uint32_t reg_pool2_stat4 = 0;

#ifdef VALIDATE_PARMS
    if ((num_of_not_valid_token_multi >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stat4 = RU_FIELD_SET(0, FPM, POOL2_STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_pool2_stat4, num_of_not_valid_token_multi);

    RU_REG_WRITE(0, FPM, POOL2_STAT4, reg_pool2_stat4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat4_get(uint32_t *num_of_not_valid_token_multi)
{
    uint32_t reg_pool2_stat4;

#ifdef VALIDATE_PARMS
    if (!num_of_not_valid_token_multi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT4, reg_pool2_stat4);

    *num_of_not_valid_token_multi = RU_FIELD_GET(0, FPM, POOL2_STAT4, NUM_OF_NOT_VALID_TOKEN_MULTI, reg_pool2_stat4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat5_set(uint32_t mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool2_stat5 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool2_stat5 = RU_FIELD_SET(0, FPM, POOL2_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool2_stat5, mem_corrupt_sts_related_alloc_token);

    RU_REG_WRITE(0, FPM, POOL2_STAT5, reg_pool2_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat5_get(uint32_t *mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool2_stat5;

#ifdef VALIDATE_PARMS
    if (!mem_corrupt_sts_related_alloc_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT5, reg_pool2_stat5);

    *mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, POOL2_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool2_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat6_set(uint32_t invalid_free_token)
{
    uint32_t reg_pool2_stat6 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool2_stat6 = RU_FIELD_SET(0, FPM, POOL2_STAT6, INVALID_FREE_TOKEN, reg_pool2_stat6, invalid_free_token);

    RU_REG_WRITE(0, FPM, POOL2_STAT6, reg_pool2_stat6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat6_get(uint32_t *invalid_free_token)
{
    uint32_t reg_pool2_stat6;

#ifdef VALIDATE_PARMS
    if (!invalid_free_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT6, reg_pool2_stat6);

    *invalid_free_token = RU_FIELD_GET(0, FPM, POOL2_STAT6, INVALID_FREE_TOKEN, reg_pool2_stat6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat7_set(uint32_t invalid_mcast_token)
{
    uint32_t reg_pool2_stat7 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool2_stat7 = RU_FIELD_SET(0, FPM, POOL2_STAT7, INVALID_MCAST_TOKEN, reg_pool2_stat7, invalid_mcast_token);

    RU_REG_WRITE(0, FPM, POOL2_STAT7, reg_pool2_stat7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat7_get(uint32_t *invalid_mcast_token)
{
    uint32_t reg_pool2_stat7;

#ifdef VALIDATE_PARMS
    if (!invalid_mcast_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT7, reg_pool2_stat7);

    *invalid_mcast_token = RU_FIELD_GET(0, FPM, POOL2_STAT7, INVALID_MCAST_TOKEN, reg_pool2_stat7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat8_set(uint32_t tokens_available_low_wtmk)
{
    uint32_t reg_pool2_stat8 = 0;

#ifdef VALIDATE_PARMS
    if ((tokens_available_low_wtmk >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stat8 = RU_FIELD_SET(0, FPM, POOL2_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool2_stat8, tokens_available_low_wtmk);

    RU_REG_WRITE(0, FPM, POOL2_STAT8, reg_pool2_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat8_get(uint32_t *tokens_available_low_wtmk)
{
    uint32_t reg_pool2_stat8;

#ifdef VALIDATE_PARMS
    if (!tokens_available_low_wtmk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT8, reg_pool2_stat8);

    *tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, POOL2_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool2_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat1_set(uint16_t undrfl, uint16_t ovrfl)
{
    uint32_t reg_pool3_stat1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool3_stat1 = RU_FIELD_SET(0, FPM, POOL3_STAT1, UNDRFL, reg_pool3_stat1, undrfl);
    reg_pool3_stat1 = RU_FIELD_SET(0, FPM, POOL3_STAT1, OVRFL, reg_pool3_stat1, ovrfl);

    RU_REG_WRITE(0, FPM, POOL3_STAT1, reg_pool3_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat1_get(uint16_t *undrfl, uint16_t *ovrfl)
{
    uint32_t reg_pool3_stat1;

#ifdef VALIDATE_PARMS
    if (!undrfl || !ovrfl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT1, reg_pool3_stat1);

    *undrfl = RU_FIELD_GET(0, FPM, POOL3_STAT1, UNDRFL, reg_pool3_stat1);
    *ovrfl = RU_FIELD_GET(0, FPM, POOL3_STAT1, OVRFL, reg_pool3_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat2_get(fpm_pool3_stat2 *pool3_stat2)
{
    uint32_t reg_pool3_stat2;

#ifdef VALIDATE_PARMS
    if (!pool3_stat2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT2, reg_pool3_stat2);

    pool3_stat2->num_of_tokens_available = RU_FIELD_GET(0, FPM, POOL3_STAT2, NUM_OF_TOKENS_AVAILABLE, reg_pool3_stat2);
    pool3_stat2->alloc_fifo_empty = RU_FIELD_GET(0, FPM, POOL3_STAT2, ALLOC_FIFO_EMPTY, reg_pool3_stat2);
    pool3_stat2->alloc_fifo_full = RU_FIELD_GET(0, FPM, POOL3_STAT2, ALLOC_FIFO_FULL, reg_pool3_stat2);
    pool3_stat2->free_fifo_empty = RU_FIELD_GET(0, FPM, POOL3_STAT2, FREE_FIFO_EMPTY, reg_pool3_stat2);
    pool3_stat2->free_fifo_full = RU_FIELD_GET(0, FPM, POOL3_STAT2, FREE_FIFO_FULL, reg_pool3_stat2);
    pool3_stat2->pool_full = RU_FIELD_GET(0, FPM, POOL3_STAT2, POOL_FULL, reg_pool3_stat2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat3_set(uint32_t num_of_not_valid_token_frees)
{
    uint32_t reg_pool3_stat3 = 0;

#ifdef VALIDATE_PARMS
    if ((num_of_not_valid_token_frees >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_stat3 = RU_FIELD_SET(0, FPM, POOL3_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool3_stat3, num_of_not_valid_token_frees);

    RU_REG_WRITE(0, FPM, POOL3_STAT3, reg_pool3_stat3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat3_get(uint32_t *num_of_not_valid_token_frees)
{
    uint32_t reg_pool3_stat3;

#ifdef VALIDATE_PARMS
    if (!num_of_not_valid_token_frees)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT3, reg_pool3_stat3);

    *num_of_not_valid_token_frees = RU_FIELD_GET(0, FPM, POOL3_STAT3, NUM_OF_NOT_VALID_TOKEN_FREES, reg_pool3_stat3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat5_set(uint32_t mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool3_stat5 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool3_stat5 = RU_FIELD_SET(0, FPM, POOL3_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool3_stat5, mem_corrupt_sts_related_alloc_token);

    RU_REG_WRITE(0, FPM, POOL3_STAT5, reg_pool3_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat5_get(uint32_t *mem_corrupt_sts_related_alloc_token)
{
    uint32_t reg_pool3_stat5;

#ifdef VALIDATE_PARMS
    if (!mem_corrupt_sts_related_alloc_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT5, reg_pool3_stat5);

    *mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, POOL3_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool3_stat5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat6_set(uint32_t invalid_free_token)
{
    uint32_t reg_pool3_stat6 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool3_stat6 = RU_FIELD_SET(0, FPM, POOL3_STAT6, INVALID_FREE_TOKEN, reg_pool3_stat6, invalid_free_token);

    RU_REG_WRITE(0, FPM, POOL3_STAT6, reg_pool3_stat6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat6_get(uint32_t *invalid_free_token)
{
    uint32_t reg_pool3_stat6;

#ifdef VALIDATE_PARMS
    if (!invalid_free_token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT6, reg_pool3_stat6);

    *invalid_free_token = RU_FIELD_GET(0, FPM, POOL3_STAT6, INVALID_FREE_TOKEN, reg_pool3_stat6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat8_set(uint32_t tokens_available_low_wtmk)
{
    uint32_t reg_pool3_stat8 = 0;

#ifdef VALIDATE_PARMS
    if ((tokens_available_low_wtmk >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_stat8 = RU_FIELD_SET(0, FPM, POOL3_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool3_stat8, tokens_available_low_wtmk);

    RU_REG_WRITE(0, FPM, POOL3_STAT8, reg_pool3_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_stat8_get(uint32_t *tokens_available_low_wtmk)
{
    uint32_t reg_pool3_stat8;

#ifdef VALIDATE_PARMS
    if (!tokens_available_low_wtmk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_STAT8, reg_pool3_stat8);

    *tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, POOL3_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool3_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_set(uint16_t xoff_threshold, uint16_t xon_threshold)
{
    uint32_t reg_pool1_xon_xoff_cfg = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool1_xon_xoff_cfg = RU_FIELD_SET(0, FPM, POOL1_XON_XOFF_CFG, XOFF_THRESHOLD, reg_pool1_xon_xoff_cfg, xoff_threshold);
    reg_pool1_xon_xoff_cfg = RU_FIELD_SET(0, FPM, POOL1_XON_XOFF_CFG, XON_THRESHOLD, reg_pool1_xon_xoff_cfg, xon_threshold);

    RU_REG_WRITE(0, FPM, POOL1_XON_XOFF_CFG, reg_pool1_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_get(uint16_t *xoff_threshold, uint16_t *xon_threshold)
{
    uint32_t reg_pool1_xon_xoff_cfg;

#ifdef VALIDATE_PARMS
    if (!xoff_threshold || !xon_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_XON_XOFF_CFG, reg_pool1_xon_xoff_cfg);

    *xoff_threshold = RU_FIELD_GET(0, FPM, POOL1_XON_XOFF_CFG, XOFF_THRESHOLD, reg_pool1_xon_xoff_cfg);
    *xon_threshold = RU_FIELD_GET(0, FPM, POOL1_XON_XOFF_CFG, XON_THRESHOLD, reg_pool1_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_set(uint8_t not_empty_threshold)
{
    uint32_t reg_fpm_not_empty_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((not_empty_threshold >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_not_empty_cfg = RU_FIELD_SET(0, FPM, FPM_NOT_EMPTY_CFG, NOT_EMPTY_THRESHOLD, reg_fpm_not_empty_cfg, not_empty_threshold);

    RU_REG_WRITE(0, FPM, FPM_NOT_EMPTY_CFG, reg_fpm_not_empty_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_get(uint8_t *not_empty_threshold)
{
    uint32_t reg_fpm_not_empty_cfg;

#ifdef VALIDATE_PARMS
    if (!not_empty_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_NOT_EMPTY_CFG, reg_fpm_not_empty_cfg);

    *not_empty_threshold = RU_FIELD_GET(0, FPM, FPM_NOT_EMPTY_CFG, NOT_EMPTY_THRESHOLD, reg_fpm_not_empty_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_mem_ctl_set(uint16_t mem_addr, uint8_t mem_sel, bdmf_boolean mem_rd, bdmf_boolean mem_wr)
{
    uint32_t reg_mem_ctl = 0;

#ifdef VALIDATE_PARMS
    if ((mem_sel >= _2BITS_MAX_VAL_) ||
       (mem_rd >= _1BITS_MAX_VAL_) ||
       (mem_wr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_ADDR, reg_mem_ctl, mem_addr);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_SEL, reg_mem_ctl, mem_sel);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_RD, reg_mem_ctl, mem_rd);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_WR, reg_mem_ctl, mem_wr);

    RU_REG_WRITE(0, FPM, MEM_CTL, reg_mem_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_mem_ctl_get(uint16_t *mem_addr, uint8_t *mem_sel, bdmf_boolean *mem_rd, bdmf_boolean *mem_wr)
{
    uint32_t reg_mem_ctl;

#ifdef VALIDATE_PARMS
    if (!mem_addr || !mem_sel || !mem_rd || !mem_wr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, MEM_CTL, reg_mem_ctl);

    *mem_addr = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_ADDR, reg_mem_ctl);
    *mem_sel = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_SEL, reg_mem_ctl);
    *mem_rd = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_RD, reg_mem_ctl);
    *mem_wr = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_WR, reg_mem_ctl);

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

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_set(uint16_t end_index, uint16_t start_index)
{
    uint32_t reg_token_recover_start_end_pool1 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_token_recover_start_end_pool1 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL1, END_INDEX, reg_token_recover_start_end_pool1, end_index);
    reg_token_recover_start_end_pool1 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL1, START_INDEX, reg_token_recover_start_end_pool1, start_index);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_START_END_POOL1, reg_token_recover_start_end_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_get(uint16_t *end_index, uint16_t *start_index)
{
    uint32_t reg_token_recover_start_end_pool1;

#ifdef VALIDATE_PARMS
    if (!end_index || !start_index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_START_END_POOL1, reg_token_recover_start_end_pool1);

    *end_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL1, END_INDEX, reg_token_recover_start_end_pool1);
    *start_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL1, START_INDEX, reg_token_recover_start_end_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_set(uint16_t end_index, uint16_t start_index)
{
    uint32_t reg_token_recover_start_end_pool2 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_token_recover_start_end_pool2 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL2, END_INDEX, reg_token_recover_start_end_pool2, end_index);
    reg_token_recover_start_end_pool2 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL2, START_INDEX, reg_token_recover_start_end_pool2, start_index);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_START_END_POOL2, reg_token_recover_start_end_pool2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_get(uint16_t *end_index, uint16_t *start_index)
{
    uint32_t reg_token_recover_start_end_pool2;

#ifdef VALIDATE_PARMS
    if (!end_index || !start_index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_START_END_POOL2, reg_token_recover_start_end_pool2);

    *end_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL2, END_INDEX, reg_token_recover_start_end_pool2);
    *start_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL2, START_INDEX, reg_token_recover_start_end_pool2);

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

bdmf_error_t ag_drv_fpm_pool_multi_set(const fpm_pool_multi *pool_multi)
{
    uint32_t reg_pool_multi = 0;

#ifdef VALIDATE_PARMS
    if(!pool_multi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_multi->token_multi >= _7BITS_MAX_VAL_) ||
       (pool_multi->update_type >= _1BITS_MAX_VAL_) ||
       (pool_multi->token_index >= _18BITS_MAX_VAL_) ||
       (pool_multi->ddr >= _1BITS_MAX_VAL_) ||
       (pool_multi->token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_MULTI, reg_pool_multi, pool_multi->token_multi);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, UPDATE_TYPE, reg_pool_multi, pool_multi->update_type);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_INDEX, reg_pool_multi, pool_multi->token_index);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, DDR, reg_pool_multi, pool_multi->ddr);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_VALID, reg_pool_multi, pool_multi->token_valid);

    RU_REG_WRITE(0, FPM, POOL_MULTI, reg_pool_multi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_1_set(uint16_t token_size, uint32_t token_index, bdmf_boolean ddr, bdmf_boolean token_valid)
{
    uint32_t reg_pool1_alloc_dealloc_1 = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _12BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool1_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool1_alloc_dealloc_1, token_size);
    reg_pool1_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool1_alloc_dealloc_1, token_index);
    reg_pool1_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC_1, DDR, reg_pool1_alloc_dealloc_1, ddr);
    reg_pool1_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool1_alloc_dealloc_1, token_valid);

    RU_REG_WRITE(0, FPM, POOL1_ALLOC_DEALLOC_1, reg_pool1_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_1_get(uint16_t *token_size, uint32_t *token_index, bdmf_boolean *ddr, bdmf_boolean *token_valid)
{
    uint32_t reg_pool1_alloc_dealloc_1;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !ddr || !token_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_ALLOC_DEALLOC_1, reg_pool1_alloc_dealloc_1);

    *token_size = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool1_alloc_dealloc_1);
    *token_index = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool1_alloc_dealloc_1);
    *ddr = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC_1, DDR, reg_pool1_alloc_dealloc_1);
    *token_valid = RU_FIELD_GET(0, FPM, POOL1_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool1_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_1_set(uint16_t token_size, uint32_t token_index, bdmf_boolean ddr, bdmf_boolean token_valid)
{
    uint32_t reg_pool2_alloc_dealloc_1 = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _12BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool2_alloc_dealloc_1, token_size);
    reg_pool2_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool2_alloc_dealloc_1, token_index);
    reg_pool2_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC_1, DDR, reg_pool2_alloc_dealloc_1, ddr);
    reg_pool2_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool2_alloc_dealloc_1, token_valid);

    RU_REG_WRITE(0, FPM, POOL2_ALLOC_DEALLOC_1, reg_pool2_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_1_get(uint16_t *token_size, uint32_t *token_index, bdmf_boolean *ddr, bdmf_boolean *token_valid)
{
    uint32_t reg_pool2_alloc_dealloc_1;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !ddr || !token_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_ALLOC_DEALLOC_1, reg_pool2_alloc_dealloc_1);

    *token_size = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool2_alloc_dealloc_1);
    *token_index = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool2_alloc_dealloc_1);
    *ddr = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC_1, DDR, reg_pool2_alloc_dealloc_1);
    *token_valid = RU_FIELD_GET(0, FPM, POOL2_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool2_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_1_set(uint16_t token_size, uint32_t token_index, bdmf_boolean ddr, bdmf_boolean token_valid)
{
    uint32_t reg_pool3_alloc_dealloc_1 = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _12BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool3_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool3_alloc_dealloc_1, token_size);
    reg_pool3_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool3_alloc_dealloc_1, token_index);
    reg_pool3_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC_1, DDR, reg_pool3_alloc_dealloc_1, ddr);
    reg_pool3_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool3_alloc_dealloc_1, token_valid);

    RU_REG_WRITE(0, FPM, POOL3_ALLOC_DEALLOC_1, reg_pool3_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_1_get(uint16_t *token_size, uint32_t *token_index, bdmf_boolean *ddr, bdmf_boolean *token_valid)
{
    uint32_t reg_pool3_alloc_dealloc_1;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !ddr || !token_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL3_ALLOC_DEALLOC_1, reg_pool3_alloc_dealloc_1);

    *token_size = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool3_alloc_dealloc_1);
    *token_index = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool3_alloc_dealloc_1);
    *ddr = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC_1, DDR, reg_pool3_alloc_dealloc_1);
    *token_valid = RU_FIELD_GET(0, FPM, POOL3_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool3_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_1_set(uint16_t token_size, uint32_t token_index, bdmf_boolean ddr, bdmf_boolean token_valid)
{
    uint32_t reg_pool4_alloc_dealloc_1 = 0;

#ifdef VALIDATE_PARMS
    if ((token_size >= _12BITS_MAX_VAL_) ||
       (token_index >= _18BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool4_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool4_alloc_dealloc_1, token_size);
    reg_pool4_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool4_alloc_dealloc_1, token_index);
    reg_pool4_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC_1, DDR, reg_pool4_alloc_dealloc_1, ddr);
    reg_pool4_alloc_dealloc_1 = RU_FIELD_SET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool4_alloc_dealloc_1, token_valid);

    RU_REG_WRITE(0, FPM, POOL4_ALLOC_DEALLOC_1, reg_pool4_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_1_get(uint16_t *token_size, uint32_t *token_index, bdmf_boolean *ddr, bdmf_boolean *token_valid)
{
    uint32_t reg_pool4_alloc_dealloc_1;

#ifdef VALIDATE_PARMS
    if (!token_size || !token_index || !ddr || !token_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL4_ALLOC_DEALLOC_1, reg_pool4_alloc_dealloc_1);

    *token_size = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_SIZE, reg_pool4_alloc_dealloc_1);
    *token_index = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_INDEX, reg_pool4_alloc_dealloc_1);
    *ddr = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC_1, DDR, reg_pool4_alloc_dealloc_1);
    *token_valid = RU_FIELD_GET(0, FPM, POOL4_ALLOC_DEALLOC_1, TOKEN_VALID, reg_pool4_alloc_dealloc_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_multi_1_set(const fpm_pool_multi_1 *pool_multi_1)
{
    uint32_t reg_pool_multi_1 = 0;

#ifdef VALIDATE_PARMS
    if(!pool_multi_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_multi_1->token_multi >= _7BITS_MAX_VAL_) ||
       (pool_multi_1->update_type >= _1BITS_MAX_VAL_) ||
       (pool_multi_1->token_index >= _18BITS_MAX_VAL_) ||
       (pool_multi_1->ddr >= _1BITS_MAX_VAL_) ||
       (pool_multi_1->token_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool_multi_1 = RU_FIELD_SET(0, FPM, POOL_MULTI_1, TOKEN_MULTI, reg_pool_multi_1, pool_multi_1->token_multi);
    reg_pool_multi_1 = RU_FIELD_SET(0, FPM, POOL_MULTI_1, UPDATE_TYPE, reg_pool_multi_1, pool_multi_1->update_type);
    reg_pool_multi_1 = RU_FIELD_SET(0, FPM, POOL_MULTI_1, TOKEN_INDEX, reg_pool_multi_1, pool_multi_1->token_index);
    reg_pool_multi_1 = RU_FIELD_SET(0, FPM, POOL_MULTI_1, DDR, reg_pool_multi_1, pool_multi_1->ddr);
    reg_pool_multi_1 = RU_FIELD_SET(0, FPM, POOL_MULTI_1, TOKEN_VALID, reg_pool_multi_1, pool_multi_1->token_valid);

    RU_REG_WRITE(0, FPM, POOL_MULTI_1, reg_pool_multi_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_1_set(uint32_t search1, const fpm_search_data_1 *search_data_1)
{
    uint32_t reg_search_data_1 = 0;

#ifdef VALIDATE_PARMS
    if(!search_data_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 273) ||
       (search_data_1->searchdata10_m >= _1BITS_MAX_VAL_) ||
       (search_data_1->searchdata11 >= _3BITS_MAX_VAL_) ||
       (search_data_1->searchdata12 >= _3BITS_MAX_VAL_) ||
       (search_data_1->searchdata13 >= _3BITS_MAX_VAL_) ||
       (search_data_1->searchdata14 >= _3BITS_MAX_VAL_) ||
       (search_data_1->searchdata15 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA10_M, reg_search_data_1, search_data_1->searchdata10_m);
    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA11, reg_search_data_1, search_data_1->searchdata11);
    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA12, reg_search_data_1, search_data_1->searchdata12);
    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA13, reg_search_data_1, search_data_1->searchdata13);
    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA14, reg_search_data_1, search_data_1->searchdata14);
    reg_search_data_1 = RU_FIELD_SET(0, FPM, SEARCH_DATA_1, SEARCHDATA15, reg_search_data_1, search_data_1->searchdata15);

    RU_REG_RAM_WRITE(0, search1, FPM, SEARCH_DATA_1, reg_search_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_1_get(uint32_t search1, fpm_search_data_1 *search_data_1)
{
    uint32_t reg_search_data_1;

#ifdef VALIDATE_PARMS
    if (!search_data_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 273))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search1, FPM, SEARCH_DATA_1, reg_search_data_1);

    search_data_1->searchdata10_m = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA10_M, reg_search_data_1);
    search_data_1->searchdata11 = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA11, reg_search_data_1);
    search_data_1->searchdata12 = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA12, reg_search_data_1);
    search_data_1->searchdata13 = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA13, reg_search_data_1);
    search_data_1->searchdata14 = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA14, reg_search_data_1);
    search_data_1->searchdata15 = RU_FIELD_GET(0, FPM, SEARCH_DATA_1, SEARCHDATA15, reg_search_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_2_set(uint32_t search1, const fpm_search_data_2 *search_data_2)
{
    uint32_t reg_search_data_2 = 0;

#ifdef VALIDATE_PARMS
    if(!search_data_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 273) ||
       (search_data_2->searchdata0 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata1 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata2 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata3 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata4 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata5 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata6 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata7 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata8 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata9 >= _3BITS_MAX_VAL_) ||
       (search_data_2->searchdata10_l >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA0, reg_search_data_2, search_data_2->searchdata0);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA1, reg_search_data_2, search_data_2->searchdata1);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA2, reg_search_data_2, search_data_2->searchdata2);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA3, reg_search_data_2, search_data_2->searchdata3);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA4, reg_search_data_2, search_data_2->searchdata4);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA5, reg_search_data_2, search_data_2->searchdata5);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA6, reg_search_data_2, search_data_2->searchdata6);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA7, reg_search_data_2, search_data_2->searchdata7);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA8, reg_search_data_2, search_data_2->searchdata8);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA9, reg_search_data_2, search_data_2->searchdata9);
    reg_search_data_2 = RU_FIELD_SET(0, FPM, SEARCH_DATA_2, SEARCHDATA10_L, reg_search_data_2, search_data_2->searchdata10_l);

    RU_REG_RAM_WRITE(0, search1, FPM, SEARCH_DATA_2, reg_search_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_2_get(uint32_t search1, fpm_search_data_2 *search_data_2)
{
    uint32_t reg_search_data_2;

#ifdef VALIDATE_PARMS
    if (!search_data_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search1 >= 273))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search1, FPM, SEARCH_DATA_2, reg_search_data_2);

    search_data_2->searchdata0 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA0, reg_search_data_2);
    search_data_2->searchdata1 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA1, reg_search_data_2);
    search_data_2->searchdata2 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA2, reg_search_data_2);
    search_data_2->searchdata3 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA3, reg_search_data_2);
    search_data_2->searchdata4 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA4, reg_search_data_2);
    search_data_2->searchdata5 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA5, reg_search_data_2);
    search_data_2->searchdata6 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA6, reg_search_data_2);
    search_data_2->searchdata7 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA7, reg_search_data_2);
    search_data_2->searchdata8 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA8, reg_search_data_2);
    search_data_2->searchdata9 = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA9, reg_search_data_2);
    search_data_2->searchdata10_l = RU_FIELD_GET(0, FPM, SEARCH_DATA_2, SEARCHDATA10_L, reg_search_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_3_set(uint32_t search3, const fpm_search_data_3 *search_data_3)
{
    uint32_t reg_search_data_3 = 0;

#ifdef VALIDATE_PARMS
    if(!search_data_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search3 >= 273) ||
       (search_data_3->searchdata10_m >= _1BITS_MAX_VAL_) ||
       (search_data_3->searchdata11 >= _3BITS_MAX_VAL_) ||
       (search_data_3->searchdata12 >= _3BITS_MAX_VAL_) ||
       (search_data_3->searchdata13 >= _3BITS_MAX_VAL_) ||
       (search_data_3->searchdata14 >= _3BITS_MAX_VAL_) ||
       (search_data_3->searchdata15 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA10_M, reg_search_data_3, search_data_3->searchdata10_m);
    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA11, reg_search_data_3, search_data_3->searchdata11);
    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA12, reg_search_data_3, search_data_3->searchdata12);
    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA13, reg_search_data_3, search_data_3->searchdata13);
    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA14, reg_search_data_3, search_data_3->searchdata14);
    reg_search_data_3 = RU_FIELD_SET(0, FPM, SEARCH_DATA_3, SEARCHDATA15, reg_search_data_3, search_data_3->searchdata15);

    RU_REG_RAM_WRITE(0, search3, FPM, SEARCH_DATA_3, reg_search_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_search_data_3_get(uint32_t search3, fpm_search_data_3 *search_data_3)
{
    uint32_t reg_search_data_3;

#ifdef VALIDATE_PARMS
    if (!search_data_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((search3 >= 273))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, search3, FPM, SEARCH_DATA_3, reg_search_data_3);

    search_data_3->searchdata10_m = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA10_M, reg_search_data_3);
    search_data_3->searchdata11 = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA11, reg_search_data_3);
    search_data_3->searchdata12 = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA12, reg_search_data_3);
    search_data_3->searchdata13 = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA13, reg_search_data_3);
    search_data_3->searchdata14 = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA14, reg_search_data_3);
    search_data_3->searchdata15 = RU_FIELD_GET(0, FPM, SEARCH_DATA_3, SEARCHDATA15, reg_search_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_multicast_data_set(uint32_t multicast_data, uint32_t multicast)
{
    uint32_t reg_multicast_data = 0;

#ifdef VALIDATE_PARMS
    if ((multicast_data >= 8192))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_multicast_data = RU_FIELD_SET(0, FPM, MULTICAST_DATA, MULTICAST, reg_multicast_data, multicast);

    RU_REG_RAM_WRITE(0, multicast_data, FPM, MULTICAST_DATA, reg_multicast_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_multicast_data_get(uint32_t multicast_data, uint32_t *multicast)
{
    uint32_t reg_multicast_data;

#ifdef VALIDATE_PARMS
    if (!multicast)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((multicast_data >= 8192))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, multicast_data, FPM, MULTICAST_DATA, reg_multicast_data);

    *multicast = RU_FIELD_GET(0, FPM, MULTICAST_DATA, MULTICAST, reg_multicast_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_compute_pool_data_get(uint32_t compute_pool_data, uint8_t *poolid)
{
    uint32_t reg_compute_pool_data;

#ifdef VALIDATE_PARMS
    if (!poolid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((compute_pool_data >= 16384))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, compute_pool_data, FPM, COMPUTE_POOL_DATA, reg_compute_pool_data);

    *poolid = RU_FIELD_GET(0, FPM, COMPUTE_POOL_DATA, POOLID, reg_compute_pool_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_force_set(bdmf_boolean force)
{
    uint32_t reg_fpm_bb_force = 0;

#ifdef VALIDATE_PARMS
    if ((force >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_force = RU_FIELD_SET(0, FPM, FPM_BB_FORCE, FORCE, reg_fpm_bb_force, force);

    RU_REG_WRITE(0, FPM, FPM_BB_FORCE, reg_fpm_bb_force);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_force_get(bdmf_boolean *force)
{
    uint32_t reg_fpm_bb_force;

#ifdef VALIDATE_PARMS
    if (!force)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_FORCE, reg_fpm_bb_force);

    *force = RU_FIELD_GET(0, FPM, FPM_BB_FORCE, FORCE, reg_fpm_bb_force);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_set(uint16_t ctrl)
{
    uint32_t reg_fpm_bb_forced_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((ctrl >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_forced_ctrl = RU_FIELD_SET(0, FPM, FPM_BB_FORCED_CTRL, CTRL, reg_fpm_bb_forced_ctrl, ctrl);

    RU_REG_WRITE(0, FPM, FPM_BB_FORCED_CTRL, reg_fpm_bb_forced_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_get(uint16_t *ctrl)
{
    uint32_t reg_fpm_bb_forced_ctrl;

#ifdef VALIDATE_PARMS
    if (!ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_FORCED_CTRL, reg_fpm_bb_forced_ctrl);

    *ctrl = RU_FIELD_GET(0, FPM, FPM_BB_FORCED_CTRL, CTRL, reg_fpm_bb_forced_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_set(uint16_t ta_addr, uint8_t dest_addr)
{
    uint32_t reg_fpm_bb_forced_addr = 0;

#ifdef VALIDATE_PARMS
    if ((dest_addr >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_forced_addr = RU_FIELD_SET(0, FPM, FPM_BB_FORCED_ADDR, TA_ADDR, reg_fpm_bb_forced_addr, ta_addr);
    reg_fpm_bb_forced_addr = RU_FIELD_SET(0, FPM, FPM_BB_FORCED_ADDR, DEST_ADDR, reg_fpm_bb_forced_addr, dest_addr);

    RU_REG_WRITE(0, FPM, FPM_BB_FORCED_ADDR, reg_fpm_bb_forced_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_get(uint16_t *ta_addr, uint8_t *dest_addr)
{
    uint32_t reg_fpm_bb_forced_addr;

#ifdef VALIDATE_PARMS
    if (!ta_addr || !dest_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_FORCED_ADDR, reg_fpm_bb_forced_addr);

    *ta_addr = RU_FIELD_GET(0, FPM, FPM_BB_FORCED_ADDR, TA_ADDR, reg_fpm_bb_forced_addr);
    *dest_addr = RU_FIELD_GET(0, FPM, FPM_BB_FORCED_ADDR, DEST_ADDR, reg_fpm_bb_forced_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_set(uint32_t data)
{
    uint32_t reg_fpm_bb_forced_data = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_fpm_bb_forced_data = RU_FIELD_SET(0, FPM, FPM_BB_FORCED_DATA, DATA, reg_fpm_bb_forced_data, data);

    RU_REG_WRITE(0, FPM, FPM_BB_FORCED_DATA, reg_fpm_bb_forced_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_forced_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_FORCED_DATA, reg_fpm_bb_forced_data);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_FORCED_DATA, DATA, reg_fpm_bb_forced_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_set(uint8_t dest_id, bdmf_boolean override_en, uint16_t route_addr)
{
    uint32_t reg_fpm_bb_decode_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dest_id >= _6BITS_MAX_VAL_) ||
       (override_en >= _1BITS_MAX_VAL_) ||
       (route_addr >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_decode_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DECODE_CFG, DEST_ID, reg_fpm_bb_decode_cfg, dest_id);
    reg_fpm_bb_decode_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DECODE_CFG, OVERRIDE_EN, reg_fpm_bb_decode_cfg, override_en);
    reg_fpm_bb_decode_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DECODE_CFG, ROUTE_ADDR, reg_fpm_bb_decode_cfg, route_addr);

    RU_REG_WRITE(0, FPM, FPM_BB_DECODE_CFG, reg_fpm_bb_decode_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_get(uint8_t *dest_id, bdmf_boolean *override_en, uint16_t *route_addr)
{
    uint32_t reg_fpm_bb_decode_cfg;

#ifdef VALIDATE_PARMS
    if (!dest_id || !override_en || !route_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DECODE_CFG, reg_fpm_bb_decode_cfg);

    *dest_id = RU_FIELD_GET(0, FPM, FPM_BB_DECODE_CFG, DEST_ID, reg_fpm_bb_decode_cfg);
    *override_en = RU_FIELD_GET(0, FPM, FPM_BB_DECODE_CFG, OVERRIDE_EN, reg_fpm_bb_decode_cfg);
    *route_addr = RU_FIELD_GET(0, FPM, FPM_BB_DECODE_CFG, ROUTE_ADDR, reg_fpm_bb_decode_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_set(uint8_t rxfifo_sw_addr, uint8_t txfifo_sw_addr, bdmf_boolean rxfifo_sw_rst, bdmf_boolean txfifo_sw_rst)
{
    uint32_t reg_fpm_bb_dbg_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rxfifo_sw_addr >= _4BITS_MAX_VAL_) ||
       (txfifo_sw_addr >= _4BITS_MAX_VAL_) ||
       (rxfifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (txfifo_sw_rst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_dbg_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DBG_CFG, RXFIFO_SW_ADDR, reg_fpm_bb_dbg_cfg, rxfifo_sw_addr);
    reg_fpm_bb_dbg_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DBG_CFG, TXFIFO_SW_ADDR, reg_fpm_bb_dbg_cfg, txfifo_sw_addr);
    reg_fpm_bb_dbg_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DBG_CFG, RXFIFO_SW_RST, reg_fpm_bb_dbg_cfg, rxfifo_sw_rst);
    reg_fpm_bb_dbg_cfg = RU_FIELD_SET(0, FPM, FPM_BB_DBG_CFG, TXFIFO_SW_RST, reg_fpm_bb_dbg_cfg, txfifo_sw_rst);

    RU_REG_WRITE(0, FPM, FPM_BB_DBG_CFG, reg_fpm_bb_dbg_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_get(uint8_t *rxfifo_sw_addr, uint8_t *txfifo_sw_addr, bdmf_boolean *rxfifo_sw_rst, bdmf_boolean *txfifo_sw_rst)
{
    uint32_t reg_fpm_bb_dbg_cfg;

#ifdef VALIDATE_PARMS
    if (!rxfifo_sw_addr || !txfifo_sw_addr || !rxfifo_sw_rst || !txfifo_sw_rst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_CFG, reg_fpm_bb_dbg_cfg);

    *rxfifo_sw_addr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_CFG, RXFIFO_SW_ADDR, reg_fpm_bb_dbg_cfg);
    *txfifo_sw_addr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_CFG, TXFIFO_SW_ADDR, reg_fpm_bb_dbg_cfg);
    *rxfifo_sw_rst = RU_FIELD_GET(0, FPM, FPM_BB_DBG_CFG, RXFIFO_SW_RST, reg_fpm_bb_dbg_cfg);
    *txfifo_sw_rst = RU_FIELD_GET(0, FPM, FPM_BB_DBG_CFG, TXFIFO_SW_RST, reg_fpm_bb_dbg_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(fpm_fpm_bb_dbg_rxfifo_sts *fpm_bb_dbg_rxfifo_sts)
{
    uint32_t reg_fpm_bb_dbg_rxfifo_sts;

#ifdef VALIDATE_PARMS
    if (!fpm_bb_dbg_rxfifo_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_RXFIFO_STS, reg_fpm_bb_dbg_rxfifo_sts);

    fpm_bb_dbg_rxfifo_sts->fifo_empty = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_STS, FIFO_EMPTY, reg_fpm_bb_dbg_rxfifo_sts);
    fpm_bb_dbg_rxfifo_sts->fifo_full = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_STS, FIFO_FULL, reg_fpm_bb_dbg_rxfifo_sts);
    fpm_bb_dbg_rxfifo_sts->fifo_used_words = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_STS, FIFO_USED_WORDS, reg_fpm_bb_dbg_rxfifo_sts);
    fpm_bb_dbg_rxfifo_sts->fifo_rd_cntr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_STS, FIFO_RD_CNTR, reg_fpm_bb_dbg_rxfifo_sts);
    fpm_bb_dbg_rxfifo_sts->fifo_wr_cntr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_STS, FIFO_WR_CNTR, reg_fpm_bb_dbg_rxfifo_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(fpm_fpm_bb_dbg_txfifo_sts *fpm_bb_dbg_txfifo_sts)
{
    uint32_t reg_fpm_bb_dbg_txfifo_sts;

#ifdef VALIDATE_PARMS
    if (!fpm_bb_dbg_txfifo_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_TXFIFO_STS, reg_fpm_bb_dbg_txfifo_sts);

    fpm_bb_dbg_txfifo_sts->fifo_empty = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_STS, FIFO_EMPTY, reg_fpm_bb_dbg_txfifo_sts);
    fpm_bb_dbg_txfifo_sts->fifo_full = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_STS, FIFO_FULL, reg_fpm_bb_dbg_txfifo_sts);
    fpm_bb_dbg_txfifo_sts->fifo_used_words = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_STS, FIFO_USED_WORDS, reg_fpm_bb_dbg_txfifo_sts);
    fpm_bb_dbg_txfifo_sts->fifo_rd_cntr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_STS, FIFO_RD_CNTR, reg_fpm_bb_dbg_txfifo_sts);
    fpm_bb_dbg_txfifo_sts->fifo_wr_cntr = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_STS, FIFO_WR_CNTR, reg_fpm_bb_dbg_txfifo_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_dbg_rxfifo_data1;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_RXFIFO_DATA1, reg_fpm_bb_dbg_rxfifo_data1);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_DATA1, DATA, reg_fpm_bb_dbg_rxfifo_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_dbg_rxfifo_data2;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_RXFIFO_DATA2, reg_fpm_bb_dbg_rxfifo_data2);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_RXFIFO_DATA2, DATA, reg_fpm_bb_dbg_rxfifo_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_dbg_txfifo_data1;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_TXFIFO_DATA1, reg_fpm_bb_dbg_txfifo_data1);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_DATA1, DATA, reg_fpm_bb_dbg_txfifo_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_dbg_txfifo_data2;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_TXFIFO_DATA2, reg_fpm_bb_dbg_txfifo_data2);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_DATA2, DATA, reg_fpm_bb_dbg_txfifo_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(uint32_t *data)
{
    uint32_t reg_fpm_bb_dbg_txfifo_data3;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_TXFIFO_DATA3, reg_fpm_bb_dbg_txfifo_data3);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_DATA3, DATA, reg_fpm_bb_dbg_txfifo_data3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_misc_set(const fpm_fpm_bb_misc *fpm_bb_misc)
{
    uint32_t reg_fpm_bb_misc = 0;

#ifdef VALIDATE_PARMS
    if(!fpm_bb_misc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((fpm_bb_misc->old_task_num >= _1BITS_MAX_VAL_) ||
       (fpm_bb_misc->alc_fre_arb_rr >= _1BITS_MAX_VAL_) ||
       (fpm_bb_misc->alc_fst_ack >= _1BITS_MAX_VAL_) ||
       (fpm_bb_misc->pool_0_size >= _2BITS_MAX_VAL_) ||
       (fpm_bb_misc->pool_1_size >= _2BITS_MAX_VAL_) ||
       (fpm_bb_misc->poolx_en >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, OLD_TASK_NUM, reg_fpm_bb_misc, fpm_bb_misc->old_task_num);
    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, ALC_FRE_ARB_RR, reg_fpm_bb_misc, fpm_bb_misc->alc_fre_arb_rr);
    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, ALC_FST_ACK, reg_fpm_bb_misc, fpm_bb_misc->alc_fst_ack);
    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, POOL_0_SIZE, reg_fpm_bb_misc, fpm_bb_misc->pool_0_size);
    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, POOL_1_SIZE, reg_fpm_bb_misc, fpm_bb_misc->pool_1_size);
    reg_fpm_bb_misc = RU_FIELD_SET(0, FPM, FPM_BB_MISC, POOLX_EN, reg_fpm_bb_misc, fpm_bb_misc->poolx_en);

    RU_REG_WRITE(0, FPM, FPM_BB_MISC, reg_fpm_bb_misc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_misc_get(fpm_fpm_bb_misc *fpm_bb_misc)
{
    uint32_t reg_fpm_bb_misc;

#ifdef VALIDATE_PARMS
    if (!fpm_bb_misc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_MISC, reg_fpm_bb_misc);

    fpm_bb_misc->old_task_num = RU_FIELD_GET(0, FPM, FPM_BB_MISC, OLD_TASK_NUM, reg_fpm_bb_misc);
    fpm_bb_misc->alc_fre_arb_rr = RU_FIELD_GET(0, FPM, FPM_BB_MISC, ALC_FRE_ARB_RR, reg_fpm_bb_misc);
    fpm_bb_misc->alc_fst_ack = RU_FIELD_GET(0, FPM, FPM_BB_MISC, ALC_FST_ACK, reg_fpm_bb_misc);
    fpm_bb_misc->pool_0_size = RU_FIELD_GET(0, FPM, FPM_BB_MISC, POOL_0_SIZE, reg_fpm_bb_misc);
    fpm_bb_misc->pool_1_size = RU_FIELD_GET(0, FPM, FPM_BB_MISC, POOL_1_SIZE, reg_fpm_bb_misc);
    fpm_bb_misc->poolx_en = RU_FIELD_GET(0, FPM, FPM_BB_MISC, POOLX_EN, reg_fpm_bb_misc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_clk_gate_cntrl_set(const fpm_clk_gate_cntrl *clk_gate_cntrl)
{
    uint32_t reg_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_gate_cntrl = RU_FIELD_SET(0, FPM, CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_cntrl, clk_gate_cntrl->bypass_clk_gate);
    reg_clk_gate_cntrl = RU_FIELD_SET(0, FPM, CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_cntrl, clk_gate_cntrl->timer_val);
    reg_clk_gate_cntrl = RU_FIELD_SET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_en);
    reg_clk_gate_cntrl = RU_FIELD_SET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_intrvl);
    reg_clk_gate_cntrl = RU_FIELD_SET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, FPM, CLK_GATE_CNTRL, reg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_clk_gate_cntrl_get(fpm_clk_gate_cntrl *clk_gate_cntrl)
{
    uint32_t reg_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, CLK_GATE_CNTRL, reg_clk_gate_cntrl);

    clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, FPM, CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_cntrl);
    clk_gate_cntrl->timer_val = RU_FIELD_GET(0, FPM, CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, FPM, CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_fpm_ctl,
    bdmf_address_fpm_cfg1,
    bdmf_address_fpm_weight,
    bdmf_address_fpm_bb_cfg,
    bdmf_address_pool1_intr_msk,
    bdmf_address_pool1_intr_sts,
    bdmf_address_pool1_stall_msk,
    bdmf_address_pool2_intr_msk,
    bdmf_address_pool2_intr_sts,
    bdmf_address_pool2_stall_msk,
    bdmf_address_pool3_intr_msk,
    bdmf_address_pool3_intr_sts,
    bdmf_address_pool3_stall_msk,
    bdmf_address_pool1_cfg1,
    bdmf_address_pool1_cfg2,
    bdmf_address_pool1_cfg3,
    bdmf_address_pool1_cfg4,
    bdmf_address_pool1_stat1,
    bdmf_address_pool1_stat2,
    bdmf_address_pool1_stat3,
    bdmf_address_pool1_stat4,
    bdmf_address_pool1_stat5,
    bdmf_address_pool1_stat6,
    bdmf_address_pool1_stat7,
    bdmf_address_pool1_stat8,
    bdmf_address_pool2_stat1,
    bdmf_address_pool2_stat2,
    bdmf_address_pool2_stat3,
    bdmf_address_pool2_stat4,
    bdmf_address_pool2_stat5,
    bdmf_address_pool2_stat6,
    bdmf_address_pool2_stat7,
    bdmf_address_pool2_stat8,
    bdmf_address_pool3_stat1,
    bdmf_address_pool3_stat2,
    bdmf_address_pool3_stat3,
    bdmf_address_pool3_stat5,
    bdmf_address_pool3_stat6,
    bdmf_address_pool3_stat8,
    bdmf_address_pool1_xon_xoff_cfg,
    bdmf_address_fpm_not_empty_cfg,
    bdmf_address_mem_ctl,
    bdmf_address_mem_data1,
    bdmf_address_mem_data2,
    bdmf_address_spare,
    bdmf_address_token_recover_ctl,
    bdmf_address_short_aging_timer,
    bdmf_address_long_aging_timer,
    bdmf_address_cache_recycle_timer,
    bdmf_address_expired_token_count_pool1,
    bdmf_address_recovered_token_count_pool1,
    bdmf_address_expired_token_count_pool2,
    bdmf_address_recovered_token_count_pool2,
    bdmf_address_token_recover_start_end_pool1,
    bdmf_address_token_recover_start_end_pool2,
    bdmf_address_prbs_invalid_gen,
    bdmf_address_pool1_alloc_dealloc,
    bdmf_address_pool2_alloc_dealloc,
    bdmf_address_pool3_alloc_dealloc,
    bdmf_address_pool4_alloc_dealloc,
    bdmf_address_pool_multi,
    bdmf_address_pool1_alloc_dealloc_1,
    bdmf_address_pool2_alloc_dealloc_1,
    bdmf_address_pool3_alloc_dealloc_1,
    bdmf_address_pool4_alloc_dealloc_1,
    bdmf_address_pool_multi_1,
    bdmf_address_search_data_0,
    bdmf_address_search_data_1,
    bdmf_address_search_data_2,
    bdmf_address_search_data_3,
    bdmf_address_multicast_data,
    bdmf_address_compute_pool_data,
    bdmf_address_fpm_bb_force,
    bdmf_address_fpm_bb_forced_ctrl,
    bdmf_address_fpm_bb_forced_addr,
    bdmf_address_fpm_bb_forced_data,
    bdmf_address_fpm_bb_decode_cfg,
    bdmf_address_fpm_bb_dbg_cfg,
    bdmf_address_fpm_bb_dbg_rxfifo_sts,
    bdmf_address_fpm_bb_dbg_txfifo_sts,
    bdmf_address_fpm_bb_dbg_rxfifo_data1,
    bdmf_address_fpm_bb_dbg_rxfifo_data2,
    bdmf_address_fpm_bb_dbg_txfifo_data1,
    bdmf_address_fpm_bb_dbg_txfifo_data2,
    bdmf_address_fpm_bb_dbg_txfifo_data3,
    bdmf_address_fpm_bb_misc,
    bdmf_address_clk_gate_cntrl,
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
    case cli_fpm_pool1_en:
        ag_err = ag_drv_fpm_pool1_en_set(parm[1].value.unumber);
        break;
    case cli_fpm_bb_reset:
        ag_err = ag_drv_fpm_bb_reset_set(parm[1].value.unumber);
        break;
    case cli_fpm_ddr0_weight:
        ag_err = ag_drv_fpm_ddr0_weight_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_ddr1_weight:
        ag_err = ag_drv_fpm_ddr1_weight_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool_cfg:
    {
        fpm_pool_cfg pool_cfg = { .fp_buf_size = parm[1].value.unumber, .pool_base_address = parm[2].value.unumber, .pool_base_address_pool2 = parm[3].value.unumber};
        ag_err = ag_drv_fpm_pool_cfg_set(&pool_cfg);
        break;
    }
    case cli_fpm_pool_stat:
    {
        fpm_pool_stat pool_stat = { .ovrfl = parm[1].value.unumber, .undrfl = parm[2].value.unumber, .pool_full = parm[3].value.unumber, .free_fifo_full = parm[4].value.unumber, .free_fifo_empty = parm[5].value.unumber, .alloc_fifo_full = parm[6].value.unumber, .alloc_fifo_empty = parm[7].value.unumber, .num_of_tokens_available = parm[8].value.unumber, .num_of_not_valid_token_frees = parm[9].value.unumber, .num_of_not_valid_token_multi = parm[10].value.unumber, .invalid_free_token = parm[11].value.unumber, .invalid_mcast_token = parm[12].value.unumber, .tokens_available_low_wtmk = parm[13].value.unumber};
        ag_err = ag_drv_fpm_pool_stat_set(&pool_stat);
        break;
    }
    case cli_fpm_pool4_alloc_dealloc:
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool3_alloc_dealloc:
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool2_alloc_dealloc:
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool1_alloc_dealloc:
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_back_door_mem:
        ag_err = ag_drv_fpm_back_door_mem_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_timer:
    {
        fpm_timer timer = { .long_aging_timer = parm[1].value.unumber, .short_aging_timer = parm[2].value.unumber, .recycle_timer = parm[3].value.unumber};
        ag_err = ag_drv_fpm_timer_set(&timer);
        break;
    }
    case cli_fpm_fpm_cfg1:
        ag_err = ag_drv_fpm_fpm_cfg1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_bb_cfg:
        ag_err = ag_drv_fpm_fpm_bb_cfg_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool1_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk = { .alloc_fifo_full_msk = parm[1].value.unumber, .free_fifo_full_msk = parm[2].value.unumber, .pool_full_msk = parm[3].value.unumber, .free_token_no_valid_msk = parm[4].value.unumber, .free_token_index_out_of_range_msk = parm[5].value.unumber, .multi_token_no_valid_msk = parm[6].value.unumber, .multi_token_index_out_of_range_msk = parm[7].value.unumber, .pool_dis_free_multi_msk = parm[8].value.unumber, .memory_corrupt_msk = parm[9].value.unumber, .xoff_msk = parm[10].value.unumber, .xon_msk = parm[11].value.unumber, .illegal_address_access_msk = parm[12].value.unumber, .illegal_alloc_request_msk = parm[13].value.unumber, .expired_token_det_msk = parm[14].value.unumber, .expired_token_recov_msk = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool1_intr_msk_set(&pool2_intr_msk);
        break;
    }
    case cli_fpm_pool1_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts = { .alloc_fifo_full_sts = parm[1].value.unumber, .free_fifo_full_sts = parm[2].value.unumber, .pool_full_sts = parm[3].value.unumber, .free_token_no_valid_sts = parm[4].value.unumber, .free_token_index_out_of_range_sts = parm[5].value.unumber, .multi_token_no_valid_sts = parm[6].value.unumber, .multi_token_index_out_of_range_sts = parm[7].value.unumber, .pool_dis_free_multi_sts = parm[8].value.unumber, .memory_corrupt_sts = parm[9].value.unumber, .xoff_state_sts = parm[10].value.unumber, .xon_state_sts = parm[11].value.unumber, .illegal_address_access_sts = parm[12].value.unumber, .illegal_alloc_request_sts = parm[13].value.unumber, .expired_token_det_sts = parm[14].value.unumber, .expired_token_recov_sts = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool1_intr_sts_set(&pool2_intr_sts);
        break;
    }
    case cli_fpm_pool1_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk = { .free_token_no_valid_stall_msk = parm[1].value.unumber, .free_token_index_out_of_range_stall_msk = parm[2].value.unumber, .multi_token_no_valid_stall_msk = parm[3].value.unumber, .multi_token_index_out_of_range_stall_msk = parm[4].value.unumber, .memory_corrupt_stall_msk = parm[5].value.unumber};
        ag_err = ag_drv_fpm_pool1_stall_msk_set(&pool2_stall_msk);
        break;
    }
    case cli_fpm_pool2_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk = { .alloc_fifo_full_msk = parm[1].value.unumber, .free_fifo_full_msk = parm[2].value.unumber, .pool_full_msk = parm[3].value.unumber, .free_token_no_valid_msk = parm[4].value.unumber, .free_token_index_out_of_range_msk = parm[5].value.unumber, .multi_token_no_valid_msk = parm[6].value.unumber, .multi_token_index_out_of_range_msk = parm[7].value.unumber, .pool_dis_free_multi_msk = parm[8].value.unumber, .memory_corrupt_msk = parm[9].value.unumber, .xoff_msk = parm[10].value.unumber, .xon_msk = parm[11].value.unumber, .illegal_address_access_msk = parm[12].value.unumber, .illegal_alloc_request_msk = parm[13].value.unumber, .expired_token_det_msk = parm[14].value.unumber, .expired_token_recov_msk = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool2_intr_msk_set(&pool2_intr_msk);
        break;
    }
    case cli_fpm_pool2_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts = { .alloc_fifo_full_sts = parm[1].value.unumber, .free_fifo_full_sts = parm[2].value.unumber, .pool_full_sts = parm[3].value.unumber, .free_token_no_valid_sts = parm[4].value.unumber, .free_token_index_out_of_range_sts = parm[5].value.unumber, .multi_token_no_valid_sts = parm[6].value.unumber, .multi_token_index_out_of_range_sts = parm[7].value.unumber, .pool_dis_free_multi_sts = parm[8].value.unumber, .memory_corrupt_sts = parm[9].value.unumber, .xoff_state_sts = parm[10].value.unumber, .xon_state_sts = parm[11].value.unumber, .illegal_address_access_sts = parm[12].value.unumber, .illegal_alloc_request_sts = parm[13].value.unumber, .expired_token_det_sts = parm[14].value.unumber, .expired_token_recov_sts = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool2_intr_sts_set(&pool2_intr_sts);
        break;
    }
    case cli_fpm_pool2_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk = { .free_token_no_valid_stall_msk = parm[1].value.unumber, .free_token_index_out_of_range_stall_msk = parm[2].value.unumber, .multi_token_no_valid_stall_msk = parm[3].value.unumber, .multi_token_index_out_of_range_stall_msk = parm[4].value.unumber, .memory_corrupt_stall_msk = parm[5].value.unumber};
        ag_err = ag_drv_fpm_pool2_stall_msk_set(&pool2_stall_msk);
        break;
    }
    case cli_fpm_pool3_intr_msk:
    {
        fpm_pool3_intr_msk pool3_intr_msk = { .alloc_fifo_full_msk = parm[1].value.unumber, .free_fifo_full_msk = parm[2].value.unumber, .pool_full_msk = parm[3].value.unumber, .free_token_no_valid_msk = parm[4].value.unumber, .free_token_index_out_of_range_msk = parm[5].value.unumber, .multi_token_no_valid_msk = parm[6].value.unumber, .multi_token_index_out_of_range_msk = parm[7].value.unumber, .pool_dis_free_multi_msk = parm[8].value.unumber, .memory_corrupt_msk = parm[9].value.unumber, .xoff_msk = parm[10].value.unumber, .xon_msk = parm[11].value.unumber, .illegal_address_access_msk = parm[12].value.unumber, .illegal_alloc_request_msk = parm[13].value.unumber, .expired_token_det_msk = parm[14].value.unumber, .expired_token_recov_msk = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool3_intr_msk_set(&pool3_intr_msk);
        break;
    }
    case cli_fpm_pool3_intr_sts:
    {
        fpm_pool3_intr_sts pool3_intr_sts = { .alloc_fifo_full_sts = parm[1].value.unumber, .free_fifo_full_sts = parm[2].value.unumber, .pool_full_sts = parm[3].value.unumber, .free_token_no_valid_sts = parm[4].value.unumber, .free_token_index_out_of_range_sts = parm[5].value.unumber, .multi_token_no_valid_sts = parm[6].value.unumber, .multi_token_index_out_of_range_sts = parm[7].value.unumber, .pool_dis_free_multi_sts = parm[8].value.unumber, .memory_corrupt_sts = parm[9].value.unumber, .xoff_state_sts = parm[10].value.unumber, .xon_state_sts = parm[11].value.unumber, .illegal_address_access_sts = parm[12].value.unumber, .illegal_alloc_request_sts = parm[13].value.unumber, .expired_token_det_sts = parm[14].value.unumber, .expired_token_recov_sts = parm[15].value.unumber};
        ag_err = ag_drv_fpm_pool3_intr_sts_set(&pool3_intr_sts);
        break;
    }
    case cli_fpm_pool3_stall_msk:
    {
        fpm_pool3_stall_msk pool3_stall_msk = { .free_token_no_valid_stall_msk = parm[1].value.unumber, .free_token_index_out_of_range_stall_msk = parm[2].value.unumber, .multi_token_no_valid_stall_msk = parm[3].value.unumber, .multi_token_index_out_of_range_stall_msk = parm[4].value.unumber, .memory_corrupt_stall_msk = parm[5].value.unumber};
        ag_err = ag_drv_fpm_pool3_stall_msk_set(&pool3_stall_msk);
        break;
    }
    case cli_fpm_pool1_cfg4:
        ag_err = ag_drv_fpm_pool1_cfg4_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool1_stat5:
        ag_err = ag_drv_fpm_pool1_stat5_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat1:
        ag_err = ag_drv_fpm_pool2_stat1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool2_stat3:
        ag_err = ag_drv_fpm_pool2_stat3_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat4:
        ag_err = ag_drv_fpm_pool2_stat4_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat5:
        ag_err = ag_drv_fpm_pool2_stat5_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat6:
        ag_err = ag_drv_fpm_pool2_stat6_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat7:
        ag_err = ag_drv_fpm_pool2_stat7_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool2_stat8:
        ag_err = ag_drv_fpm_pool2_stat8_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool3_stat1:
        ag_err = ag_drv_fpm_pool3_stat1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool3_stat3:
        ag_err = ag_drv_fpm_pool3_stat3_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool3_stat5:
        ag_err = ag_drv_fpm_pool3_stat5_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool3_stat6:
        ag_err = ag_drv_fpm_pool3_stat6_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool3_stat8:
        ag_err = ag_drv_fpm_pool3_stat8_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool1_xon_xoff_cfg:
        ag_err = ag_drv_fpm_pool1_xon_xoff_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_not_empty_cfg:
        ag_err = ag_drv_fpm_fpm_not_empty_cfg_set(parm[1].value.unumber);
        break;
    case cli_fpm_mem_ctl:
        ag_err = ag_drv_fpm_mem_ctl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_token_recover_ctl:
    {
        fpm_token_recover_ctl token_recover_ctl = { .token_recover_ena = parm[1].value.unumber, .single_pass_ena = parm[2].value.unumber, .token_remark_ena = parm[3].value.unumber, .token_reclaim_ena = parm[4].value.unumber, .force_token_reclaim = parm[5].value.unumber, .clr_expired_token_count = parm[6].value.unumber, .clr_recovered_token_count = parm[7].value.unumber};
        ag_err = ag_drv_fpm_token_recover_ctl_set(&token_recover_ctl);
        break;
    }
    case cli_fpm_token_recover_start_end_pool1:
        ag_err = ag_drv_fpm_token_recover_start_end_pool1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_token_recover_start_end_pool2:
        ag_err = ag_drv_fpm_token_recover_start_end_pool2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_prbs_invalid_gen:
        ag_err = ag_drv_fpm_prbs_invalid_gen_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool_multi:
    {
        fpm_pool_multi pool_multi = { .token_multi = parm[1].value.unumber, .update_type = parm[2].value.unumber, .token_index = parm[3].value.unumber, .ddr = parm[4].value.unumber, .token_valid = parm[5].value.unumber};
        ag_err = ag_drv_fpm_pool_multi_set(&pool_multi);
        break;
    }
    case cli_fpm_pool1_alloc_dealloc_1:
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool2_alloc_dealloc_1:
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool3_alloc_dealloc_1:
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool4_alloc_dealloc_1:
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool_multi_1:
    {
        fpm_pool_multi_1 pool_multi_1 = { .token_multi = parm[1].value.unumber, .update_type = parm[2].value.unumber, .token_index = parm[3].value.unumber, .ddr = parm[4].value.unumber, .token_valid = parm[5].value.unumber};
        ag_err = ag_drv_fpm_pool_multi_1_set(&pool_multi_1);
        break;
    }
    case cli_fpm_search_data_1:
    {
        fpm_search_data_1 search_data_1 = { .searchdata10_m = parm[2].value.unumber, .searchdata11 = parm[3].value.unumber, .searchdata12 = parm[4].value.unumber, .searchdata13 = parm[5].value.unumber, .searchdata14 = parm[6].value.unumber, .searchdata15 = parm[7].value.unumber};
        ag_err = ag_drv_fpm_search_data_1_set(parm[1].value.unumber, &search_data_1);
        break;
    }
    case cli_fpm_search_data_2:
    {
        fpm_search_data_2 search_data_2 = { .searchdata0 = parm[2].value.unumber, .searchdata1 = parm[3].value.unumber, .searchdata2 = parm[4].value.unumber, .searchdata3 = parm[5].value.unumber, .searchdata4 = parm[6].value.unumber, .searchdata5 = parm[7].value.unumber, .searchdata6 = parm[8].value.unumber, .searchdata7 = parm[9].value.unumber, .searchdata8 = parm[10].value.unumber, .searchdata9 = parm[11].value.unumber, .searchdata10_l = parm[12].value.unumber};
        ag_err = ag_drv_fpm_search_data_2_set(parm[1].value.unumber, &search_data_2);
        break;
    }
    case cli_fpm_search_data_3:
    {
        fpm_search_data_3 search_data_3 = { .searchdata10_m = parm[2].value.unumber, .searchdata11 = parm[3].value.unumber, .searchdata12 = parm[4].value.unumber, .searchdata13 = parm[5].value.unumber, .searchdata14 = parm[6].value.unumber, .searchdata15 = parm[7].value.unumber};
        ag_err = ag_drv_fpm_search_data_3_set(parm[1].value.unumber, &search_data_3);
        break;
    }
    case cli_fpm_multicast_data:
        ag_err = ag_drv_fpm_multicast_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_bb_force:
        ag_err = ag_drv_fpm_fpm_bb_force_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_ctrl:
        ag_err = ag_drv_fpm_fpm_bb_forced_ctrl_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_addr:
        ag_err = ag_drv_fpm_fpm_bb_forced_addr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_data:
        ag_err = ag_drv_fpm_fpm_bb_forced_data_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_decode_cfg:
        ag_err = ag_drv_fpm_fpm_bb_decode_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_fpm_bb_dbg_cfg:
        ag_err = ag_drv_fpm_fpm_bb_dbg_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_fpm_bb_misc:
    {
        fpm_fpm_bb_misc fpm_bb_misc = { .old_task_num = parm[1].value.unumber, .alc_fre_arb_rr = parm[2].value.unumber, .alc_fst_ack = parm[3].value.unumber, .pool_0_size = parm[4].value.unumber, .pool_1_size = parm[5].value.unumber, .poolx_en = parm[6].value.unumber};
        ag_err = ag_drv_fpm_fpm_bb_misc_set(&fpm_bb_misc);
        break;
    }
    case cli_fpm_clk_gate_cntrl:
    {
        fpm_clk_gate_cntrl clk_gate_cntrl = { .bypass_clk_gate = parm[1].value.unumber, .timer_val = parm[2].value.unumber, .keep_alive_en = parm[3].value.unumber, .keep_alive_intrvl = parm[4].value.unumber, .keep_alive_cyc = parm[5].value.unumber};
        ag_err = ag_drv_fpm_clk_gate_cntrl_set(&clk_gate_cntrl);
        break;
    }
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
    case cli_fpm_pool1_en:
    {
        bdmf_boolean pool1_enable;
        ag_err = ag_drv_fpm_pool1_en_get(&pool1_enable);
        bdmf_session_print(session, "pool1_enable = %u = 0x%x\n", pool1_enable, pool1_enable);
        break;
    }
    case cli_fpm_bb_reset:
    {
        bdmf_boolean fpm_bb_soft_reset;
        ag_err = ag_drv_fpm_bb_reset_get(&fpm_bb_soft_reset);
        bdmf_session_print(session, "fpm_bb_soft_reset = %u = 0x%x\n", fpm_bb_soft_reset, fpm_bb_soft_reset);
        break;
    }
    case cli_fpm_ddr0_weight:
    {
        uint8_t ddr0_alloc_weight;
        uint8_t ddr0_free_weight;
        ag_err = ag_drv_fpm_ddr0_weight_get(&ddr0_alloc_weight, &ddr0_free_weight);
        bdmf_session_print(session, "ddr0_alloc_weight = %u = 0x%x\n", ddr0_alloc_weight, ddr0_alloc_weight);
        bdmf_session_print(session, "ddr0_free_weight = %u = 0x%x\n", ddr0_free_weight, ddr0_free_weight);
        break;
    }
    case cli_fpm_ddr1_weight:
    {
        uint8_t ddr1_alloc_weight;
        uint8_t ddr1_free_weight;
        ag_err = ag_drv_fpm_ddr1_weight_get(&ddr1_alloc_weight, &ddr1_free_weight);
        bdmf_session_print(session, "ddr1_alloc_weight = %u = 0x%x\n", ddr1_alloc_weight, ddr1_alloc_weight);
        bdmf_session_print(session, "ddr1_free_weight = %u = 0x%x\n", ddr1_free_weight, ddr1_free_weight);
        break;
    }
    case cli_fpm_pool_cfg:
    {
        fpm_pool_cfg pool_cfg;
        ag_err = ag_drv_fpm_pool_cfg_get(&pool_cfg);
        bdmf_session_print(session, "fp_buf_size = %u = 0x%x\n", pool_cfg.fp_buf_size, pool_cfg.fp_buf_size);
        bdmf_session_print(session, "pool_base_address = %u = 0x%x\n", pool_cfg.pool_base_address, pool_cfg.pool_base_address);
        bdmf_session_print(session, "pool_base_address_pool2 = %u = 0x%x\n", pool_cfg.pool_base_address_pool2, pool_cfg.pool_base_address_pool2);
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
    case cli_fpm_pool4_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool3_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool2_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool1_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        break;
    }
    case cli_fpm_back_door_mem:
    {
        uint32_t mem_data1;
        uint32_t mem_data2;
        ag_err = ag_drv_fpm_back_door_mem_get(&mem_data1, &mem_data2);
        bdmf_session_print(session, "mem_data1 = %u = 0x%x\n", mem_data1, mem_data1);
        bdmf_session_print(session, "mem_data2 = %u = 0x%x\n", mem_data2, mem_data2);
        break;
    }
    case cli_fpm_pool1_count:
    {
        uint32_t expired_count;
        uint32_t recovered_count;
        ag_err = ag_drv_fpm_pool1_count_get(&expired_count, &recovered_count);
        bdmf_session_print(session, "expired_count = %u = 0x%x\n", expired_count, expired_count);
        bdmf_session_print(session, "recovered_count = %u = 0x%x\n", recovered_count, recovered_count);
        break;
    }
    case cli_fpm_pool2_count:
    {
        uint32_t expired_count;
        uint32_t recovered_count;
        ag_err = ag_drv_fpm_pool2_count_get(&expired_count, &recovered_count);
        bdmf_session_print(session, "expired_count = %u = 0x%x\n", expired_count, expired_count);
        bdmf_session_print(session, "recovered_count = %u = 0x%x\n", recovered_count, recovered_count);
        break;
    }
    case cli_fpm_timer:
    {
        fpm_timer timer;
        ag_err = ag_drv_fpm_timer_get(&timer);
        bdmf_session_print(session, "long_aging_timer = %u = 0x%x\n", timer.long_aging_timer, timer.long_aging_timer);
        bdmf_session_print(session, "short_aging_timer = %u = 0x%x\n", timer.short_aging_timer, timer.short_aging_timer);
        bdmf_session_print(session, "recycle_timer = %u = 0x%x\n", timer.recycle_timer, timer.recycle_timer);
        break;
    }
    case cli_fpm_fpm_cfg1:
    {
        bdmf_boolean pool1_search_mode;
        bdmf_boolean enable_jumbo_support;
        ag_err = ag_drv_fpm_fpm_cfg1_get(&pool1_search_mode, &enable_jumbo_support);
        bdmf_session_print(session, "pool1_search_mode = %u = 0x%x\n", pool1_search_mode, pool1_search_mode);
        bdmf_session_print(session, "enable_jumbo_support = %u = 0x%x\n", enable_jumbo_support, enable_jumbo_support);
        break;
    }
    case cli_fpm_fpm_bb_cfg:
    {
        uint8_t bb_ddr_sel;
        ag_err = ag_drv_fpm_fpm_bb_cfg_get(&bb_ddr_sel);
        bdmf_session_print(session, "bb_ddr_sel = %u = 0x%x\n", bb_ddr_sel, bb_ddr_sel);
        break;
    }
    case cli_fpm_pool1_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk;
        ag_err = ag_drv_fpm_pool1_intr_msk_get(&pool2_intr_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u = 0x%x\n", pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u = 0x%x\n", pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "pool_full_msk = %u = 0x%x\n", pool2_intr_msk.pool_full_msk, pool2_intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u = 0x%x\n", pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u = 0x%x\n", pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u = 0x%x\n", pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u = 0x%x\n", pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u = 0x%x\n", pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u = 0x%x\n", pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "xoff_msk = %u = 0x%x\n", pool2_intr_msk.xoff_msk, pool2_intr_msk.xoff_msk);
        bdmf_session_print(session, "xon_msk = %u = 0x%x\n", pool2_intr_msk.xon_msk, pool2_intr_msk.xon_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u = 0x%x\n", pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u = 0x%x\n", pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u = 0x%x\n", pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u = 0x%x\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_recov_msk);
        break;
    }
    case cli_fpm_pool1_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts;
        ag_err = ag_drv_fpm_pool1_intr_sts_get(&pool2_intr_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u = 0x%x\n", pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u = 0x%x\n", pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "pool_full_sts = %u = 0x%x\n", pool2_intr_sts.pool_full_sts, pool2_intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u = 0x%x\n", pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u = 0x%x\n", pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u = 0x%x\n", pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u = 0x%x\n", pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u = 0x%x\n", pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u = 0x%x\n", pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "xoff_state_sts = %u = 0x%x\n", pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xoff_state_sts);
        bdmf_session_print(session, "xon_state_sts = %u = 0x%x\n", pool2_intr_sts.xon_state_sts, pool2_intr_sts.xon_state_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u = 0x%x\n", pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u = 0x%x\n", pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u = 0x%x\n", pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u = 0x%x\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_recov_sts);
        break;
    }
    case cli_fpm_pool1_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk;
        ag_err = ag_drv_fpm_pool1_stall_msk_get(&pool2_stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u = 0x%x\n", pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u = 0x%x\n", pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u = 0x%x\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.memory_corrupt_stall_msk);
        break;
    }
    case cli_fpm_pool2_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk;
        ag_err = ag_drv_fpm_pool2_intr_msk_get(&pool2_intr_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u = 0x%x\n", pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u = 0x%x\n", pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "pool_full_msk = %u = 0x%x\n", pool2_intr_msk.pool_full_msk, pool2_intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u = 0x%x\n", pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u = 0x%x\n", pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u = 0x%x\n", pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u = 0x%x\n", pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u = 0x%x\n", pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u = 0x%x\n", pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "xoff_msk = %u = 0x%x\n", pool2_intr_msk.xoff_msk, pool2_intr_msk.xoff_msk);
        bdmf_session_print(session, "xon_msk = %u = 0x%x\n", pool2_intr_msk.xon_msk, pool2_intr_msk.xon_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u = 0x%x\n", pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u = 0x%x\n", pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u = 0x%x\n", pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u = 0x%x\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_recov_msk);
        break;
    }
    case cli_fpm_pool2_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts;
        ag_err = ag_drv_fpm_pool2_intr_sts_get(&pool2_intr_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u = 0x%x\n", pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u = 0x%x\n", pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "pool_full_sts = %u = 0x%x\n", pool2_intr_sts.pool_full_sts, pool2_intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u = 0x%x\n", pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u = 0x%x\n", pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u = 0x%x\n", pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u = 0x%x\n", pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u = 0x%x\n", pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u = 0x%x\n", pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "xoff_state_sts = %u = 0x%x\n", pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xoff_state_sts);
        bdmf_session_print(session, "xon_state_sts = %u = 0x%x\n", pool2_intr_sts.xon_state_sts, pool2_intr_sts.xon_state_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u = 0x%x\n", pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u = 0x%x\n", pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u = 0x%x\n", pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u = 0x%x\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_recov_sts);
        break;
    }
    case cli_fpm_pool2_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk;
        ag_err = ag_drv_fpm_pool2_stall_msk_get(&pool2_stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u = 0x%x\n", pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u = 0x%x\n", pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u = 0x%x\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.memory_corrupt_stall_msk);
        break;
    }
    case cli_fpm_pool3_intr_msk:
    {
        fpm_pool3_intr_msk pool3_intr_msk;
        ag_err = ag_drv_fpm_pool3_intr_msk_get(&pool3_intr_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u = 0x%x\n", pool3_intr_msk.alloc_fifo_full_msk, pool3_intr_msk.alloc_fifo_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u = 0x%x\n", pool3_intr_msk.free_fifo_full_msk, pool3_intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "pool_full_msk = %u = 0x%x\n", pool3_intr_msk.pool_full_msk, pool3_intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u = 0x%x\n", pool3_intr_msk.free_token_no_valid_msk, pool3_intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u = 0x%x\n", pool3_intr_msk.free_token_index_out_of_range_msk, pool3_intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u = 0x%x\n", pool3_intr_msk.multi_token_no_valid_msk, pool3_intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u = 0x%x\n", pool3_intr_msk.multi_token_index_out_of_range_msk, pool3_intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u = 0x%x\n", pool3_intr_msk.pool_dis_free_multi_msk, pool3_intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u = 0x%x\n", pool3_intr_msk.memory_corrupt_msk, pool3_intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "xoff_msk = %u = 0x%x\n", pool3_intr_msk.xoff_msk, pool3_intr_msk.xoff_msk);
        bdmf_session_print(session, "xon_msk = %u = 0x%x\n", pool3_intr_msk.xon_msk, pool3_intr_msk.xon_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u = 0x%x\n", pool3_intr_msk.illegal_address_access_msk, pool3_intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u = 0x%x\n", pool3_intr_msk.illegal_alloc_request_msk, pool3_intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u = 0x%x\n", pool3_intr_msk.expired_token_det_msk, pool3_intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u = 0x%x\n", pool3_intr_msk.expired_token_recov_msk, pool3_intr_msk.expired_token_recov_msk);
        break;
    }
    case cli_fpm_pool3_intr_sts:
    {
        fpm_pool3_intr_sts pool3_intr_sts;
        ag_err = ag_drv_fpm_pool3_intr_sts_get(&pool3_intr_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u = 0x%x\n", pool3_intr_sts.alloc_fifo_full_sts, pool3_intr_sts.alloc_fifo_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u = 0x%x\n", pool3_intr_sts.free_fifo_full_sts, pool3_intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "pool_full_sts = %u = 0x%x\n", pool3_intr_sts.pool_full_sts, pool3_intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u = 0x%x\n", pool3_intr_sts.free_token_no_valid_sts, pool3_intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u = 0x%x\n", pool3_intr_sts.free_token_index_out_of_range_sts, pool3_intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u = 0x%x\n", pool3_intr_sts.multi_token_no_valid_sts, pool3_intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u = 0x%x\n", pool3_intr_sts.multi_token_index_out_of_range_sts, pool3_intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u = 0x%x\n", pool3_intr_sts.pool_dis_free_multi_sts, pool3_intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u = 0x%x\n", pool3_intr_sts.memory_corrupt_sts, pool3_intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "xoff_state_sts = %u = 0x%x\n", pool3_intr_sts.xoff_state_sts, pool3_intr_sts.xoff_state_sts);
        bdmf_session_print(session, "xon_state_sts = %u = 0x%x\n", pool3_intr_sts.xon_state_sts, pool3_intr_sts.xon_state_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u = 0x%x\n", pool3_intr_sts.illegal_address_access_sts, pool3_intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u = 0x%x\n", pool3_intr_sts.illegal_alloc_request_sts, pool3_intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u = 0x%x\n", pool3_intr_sts.expired_token_det_sts, pool3_intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u = 0x%x\n", pool3_intr_sts.expired_token_recov_sts, pool3_intr_sts.expired_token_recov_sts);
        break;
    }
    case cli_fpm_pool3_stall_msk:
    {
        fpm_pool3_stall_msk pool3_stall_msk;
        ag_err = ag_drv_fpm_pool3_stall_msk_get(&pool3_stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u = 0x%x\n", pool3_stall_msk.free_token_no_valid_stall_msk, pool3_stall_msk.free_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool3_stall_msk.free_token_index_out_of_range_stall_msk, pool3_stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u = 0x%x\n", pool3_stall_msk.multi_token_no_valid_stall_msk, pool3_stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u = 0x%x\n", pool3_stall_msk.multi_token_index_out_of_range_stall_msk, pool3_stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u = 0x%x\n", pool3_stall_msk.memory_corrupt_stall_msk, pool3_stall_msk.memory_corrupt_stall_msk);
        break;
    }
    case cli_fpm_pool1_cfg4:
    {
        uint32_t pool_base_address_pool3;
        ag_err = ag_drv_fpm_pool1_cfg4_get(&pool_base_address_pool3);
        bdmf_session_print(session, "pool_base_address_pool3 = %u = 0x%x\n", pool_base_address_pool3, pool_base_address_pool3);
        break;
    }
    case cli_fpm_pool1_stat5:
    {
        uint32_t mem_corrupt_sts_related_alloc_token;
        ag_err = ag_drv_fpm_pool1_stat5_get(&mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u = 0x%x\n", mem_corrupt_sts_related_alloc_token, mem_corrupt_sts_related_alloc_token);
        break;
    }
    case cli_fpm_pool2_stat1:
    {
        uint16_t undrfl;
        uint16_t ovrfl;
        ag_err = ag_drv_fpm_pool2_stat1_get(&undrfl, &ovrfl);
        bdmf_session_print(session, "undrfl = %u = 0x%x\n", undrfl, undrfl);
        bdmf_session_print(session, "ovrfl = %u = 0x%x\n", ovrfl, ovrfl);
        break;
    }
    case cli_fpm_pool2_stat2:
    {
        fpm_pool2_stat2 pool2_stat2;
        ag_err = ag_drv_fpm_pool2_stat2_get(&pool2_stat2);
        bdmf_session_print(session, "num_of_tokens_available = %u = 0x%x\n", pool2_stat2.num_of_tokens_available, pool2_stat2.num_of_tokens_available);
        bdmf_session_print(session, "alloc_fifo_empty = %u = 0x%x\n", pool2_stat2.alloc_fifo_empty, pool2_stat2.alloc_fifo_empty);
        bdmf_session_print(session, "alloc_fifo_full = %u = 0x%x\n", pool2_stat2.alloc_fifo_full, pool2_stat2.alloc_fifo_full);
        bdmf_session_print(session, "free_fifo_empty = %u = 0x%x\n", pool2_stat2.free_fifo_empty, pool2_stat2.free_fifo_empty);
        bdmf_session_print(session, "free_fifo_full = %u = 0x%x\n", pool2_stat2.free_fifo_full, pool2_stat2.free_fifo_full);
        bdmf_session_print(session, "pool_full = %u = 0x%x\n", pool2_stat2.pool_full, pool2_stat2.pool_full);
        break;
    }
    case cli_fpm_pool2_stat3:
    {
        uint32_t num_of_not_valid_token_frees;
        ag_err = ag_drv_fpm_pool2_stat3_get(&num_of_not_valid_token_frees);
        bdmf_session_print(session, "num_of_not_valid_token_frees = %u = 0x%x\n", num_of_not_valid_token_frees, num_of_not_valid_token_frees);
        break;
    }
    case cli_fpm_pool2_stat4:
    {
        uint32_t num_of_not_valid_token_multi;
        ag_err = ag_drv_fpm_pool2_stat4_get(&num_of_not_valid_token_multi);
        bdmf_session_print(session, "num_of_not_valid_token_multi = %u = 0x%x\n", num_of_not_valid_token_multi, num_of_not_valid_token_multi);
        break;
    }
    case cli_fpm_pool2_stat5:
    {
        uint32_t mem_corrupt_sts_related_alloc_token;
        ag_err = ag_drv_fpm_pool2_stat5_get(&mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u = 0x%x\n", mem_corrupt_sts_related_alloc_token, mem_corrupt_sts_related_alloc_token);
        break;
    }
    case cli_fpm_pool2_stat6:
    {
        uint32_t invalid_free_token;
        ag_err = ag_drv_fpm_pool2_stat6_get(&invalid_free_token);
        bdmf_session_print(session, "invalid_free_token = %u = 0x%x\n", invalid_free_token, invalid_free_token);
        break;
    }
    case cli_fpm_pool2_stat7:
    {
        uint32_t invalid_mcast_token;
        ag_err = ag_drv_fpm_pool2_stat7_get(&invalid_mcast_token);
        bdmf_session_print(session, "invalid_mcast_token = %u = 0x%x\n", invalid_mcast_token, invalid_mcast_token);
        break;
    }
    case cli_fpm_pool2_stat8:
    {
        uint32_t tokens_available_low_wtmk;
        ag_err = ag_drv_fpm_pool2_stat8_get(&tokens_available_low_wtmk);
        bdmf_session_print(session, "tokens_available_low_wtmk = %u = 0x%x\n", tokens_available_low_wtmk, tokens_available_low_wtmk);
        break;
    }
    case cli_fpm_pool3_stat1:
    {
        uint16_t undrfl;
        uint16_t ovrfl;
        ag_err = ag_drv_fpm_pool3_stat1_get(&undrfl, &ovrfl);
        bdmf_session_print(session, "undrfl = %u = 0x%x\n", undrfl, undrfl);
        bdmf_session_print(session, "ovrfl = %u = 0x%x\n", ovrfl, ovrfl);
        break;
    }
    case cli_fpm_pool3_stat2:
    {
        fpm_pool3_stat2 pool3_stat2;
        ag_err = ag_drv_fpm_pool3_stat2_get(&pool3_stat2);
        bdmf_session_print(session, "num_of_tokens_available = %u = 0x%x\n", pool3_stat2.num_of_tokens_available, pool3_stat2.num_of_tokens_available);
        bdmf_session_print(session, "alloc_fifo_empty = %u = 0x%x\n", pool3_stat2.alloc_fifo_empty, pool3_stat2.alloc_fifo_empty);
        bdmf_session_print(session, "alloc_fifo_full = %u = 0x%x\n", pool3_stat2.alloc_fifo_full, pool3_stat2.alloc_fifo_full);
        bdmf_session_print(session, "free_fifo_empty = %u = 0x%x\n", pool3_stat2.free_fifo_empty, pool3_stat2.free_fifo_empty);
        bdmf_session_print(session, "free_fifo_full = %u = 0x%x\n", pool3_stat2.free_fifo_full, pool3_stat2.free_fifo_full);
        bdmf_session_print(session, "pool_full = %u = 0x%x\n", pool3_stat2.pool_full, pool3_stat2.pool_full);
        break;
    }
    case cli_fpm_pool3_stat3:
    {
        uint32_t num_of_not_valid_token_frees;
        ag_err = ag_drv_fpm_pool3_stat3_get(&num_of_not_valid_token_frees);
        bdmf_session_print(session, "num_of_not_valid_token_frees = %u = 0x%x\n", num_of_not_valid_token_frees, num_of_not_valid_token_frees);
        break;
    }
    case cli_fpm_pool3_stat5:
    {
        uint32_t mem_corrupt_sts_related_alloc_token;
        ag_err = ag_drv_fpm_pool3_stat5_get(&mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u = 0x%x\n", mem_corrupt_sts_related_alloc_token, mem_corrupt_sts_related_alloc_token);
        break;
    }
    case cli_fpm_pool3_stat6:
    {
        uint32_t invalid_free_token;
        ag_err = ag_drv_fpm_pool3_stat6_get(&invalid_free_token);
        bdmf_session_print(session, "invalid_free_token = %u = 0x%x\n", invalid_free_token, invalid_free_token);
        break;
    }
    case cli_fpm_pool3_stat8:
    {
        uint32_t tokens_available_low_wtmk;
        ag_err = ag_drv_fpm_pool3_stat8_get(&tokens_available_low_wtmk);
        bdmf_session_print(session, "tokens_available_low_wtmk = %u = 0x%x\n", tokens_available_low_wtmk, tokens_available_low_wtmk);
        break;
    }
    case cli_fpm_pool1_xon_xoff_cfg:
    {
        uint16_t xoff_threshold;
        uint16_t xon_threshold;
        ag_err = ag_drv_fpm_pool1_xon_xoff_cfg_get(&xoff_threshold, &xon_threshold);
        bdmf_session_print(session, "xoff_threshold = %u = 0x%x\n", xoff_threshold, xoff_threshold);
        bdmf_session_print(session, "xon_threshold = %u = 0x%x\n", xon_threshold, xon_threshold);
        break;
    }
    case cli_fpm_fpm_not_empty_cfg:
    {
        uint8_t not_empty_threshold;
        ag_err = ag_drv_fpm_fpm_not_empty_cfg_get(&not_empty_threshold);
        bdmf_session_print(session, "not_empty_threshold = %u = 0x%x\n", not_empty_threshold, not_empty_threshold);
        break;
    }
    case cli_fpm_mem_ctl:
    {
        uint16_t mem_addr;
        uint8_t mem_sel;
        bdmf_boolean mem_rd;
        bdmf_boolean mem_wr;
        ag_err = ag_drv_fpm_mem_ctl_get(&mem_addr, &mem_sel, &mem_rd, &mem_wr);
        bdmf_session_print(session, "mem_addr = %u = 0x%x\n", mem_addr, mem_addr);
        bdmf_session_print(session, "mem_sel = %u = 0x%x\n", mem_sel, mem_sel);
        bdmf_session_print(session, "mem_rd = %u = 0x%x\n", mem_rd, mem_rd);
        bdmf_session_print(session, "mem_wr = %u = 0x%x\n", mem_wr, mem_wr);
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
    case cli_fpm_token_recover_start_end_pool1:
    {
        uint16_t end_index;
        uint16_t start_index;
        ag_err = ag_drv_fpm_token_recover_start_end_pool1_get(&end_index, &start_index);
        bdmf_session_print(session, "end_index = %u = 0x%x\n", end_index, end_index);
        bdmf_session_print(session, "start_index = %u = 0x%x\n", start_index, start_index);
        break;
    }
    case cli_fpm_token_recover_start_end_pool2:
    {
        uint16_t end_index;
        uint16_t start_index;
        ag_err = ag_drv_fpm_token_recover_start_end_pool2_get(&end_index, &start_index);
        bdmf_session_print(session, "end_index = %u = 0x%x\n", end_index, end_index);
        bdmf_session_print(session, "start_index = %u = 0x%x\n", start_index, start_index);
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
    case cli_fpm_pool1_alloc_dealloc_1:
    {
        uint16_t token_size;
        uint32_t token_index;
        bdmf_boolean ddr;
        bdmf_boolean token_valid;
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        break;
    }
    case cli_fpm_pool2_alloc_dealloc_1:
    {
        uint16_t token_size;
        uint32_t token_index;
        bdmf_boolean ddr;
        bdmf_boolean token_valid;
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        break;
    }
    case cli_fpm_pool3_alloc_dealloc_1:
    {
        uint16_t token_size;
        uint32_t token_index;
        bdmf_boolean ddr;
        bdmf_boolean token_valid;
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        break;
    }
    case cli_fpm_pool4_alloc_dealloc_1:
    {
        uint16_t token_size;
        uint32_t token_index;
        bdmf_boolean ddr;
        bdmf_boolean token_valid;
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);
        bdmf_session_print(session, "token_size = %u = 0x%x\n", token_size, token_size);
        bdmf_session_print(session, "token_index = %u = 0x%x\n", token_index, token_index);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "token_valid = %u = 0x%x\n", token_valid, token_valid);
        break;
    }
    case cli_fpm_search_data_1:
    {
        fpm_search_data_1 search_data_1;
        ag_err = ag_drv_fpm_search_data_1_get(parm[1].value.unumber, &search_data_1);
        bdmf_session_print(session, "searchdata10_m = %u = 0x%x\n", search_data_1.searchdata10_m, search_data_1.searchdata10_m);
        bdmf_session_print(session, "searchdata11 = %u = 0x%x\n", search_data_1.searchdata11, search_data_1.searchdata11);
        bdmf_session_print(session, "searchdata12 = %u = 0x%x\n", search_data_1.searchdata12, search_data_1.searchdata12);
        bdmf_session_print(session, "searchdata13 = %u = 0x%x\n", search_data_1.searchdata13, search_data_1.searchdata13);
        bdmf_session_print(session, "searchdata14 = %u = 0x%x\n", search_data_1.searchdata14, search_data_1.searchdata14);
        bdmf_session_print(session, "searchdata15 = %u = 0x%x\n", search_data_1.searchdata15, search_data_1.searchdata15);
        break;
    }
    case cli_fpm_search_data_2:
    {
        fpm_search_data_2 search_data_2;
        ag_err = ag_drv_fpm_search_data_2_get(parm[1].value.unumber, &search_data_2);
        bdmf_session_print(session, "searchdata0 = %u = 0x%x\n", search_data_2.searchdata0, search_data_2.searchdata0);
        bdmf_session_print(session, "searchdata1 = %u = 0x%x\n", search_data_2.searchdata1, search_data_2.searchdata1);
        bdmf_session_print(session, "searchdata2 = %u = 0x%x\n", search_data_2.searchdata2, search_data_2.searchdata2);
        bdmf_session_print(session, "searchdata3 = %u = 0x%x\n", search_data_2.searchdata3, search_data_2.searchdata3);
        bdmf_session_print(session, "searchdata4 = %u = 0x%x\n", search_data_2.searchdata4, search_data_2.searchdata4);
        bdmf_session_print(session, "searchdata5 = %u = 0x%x\n", search_data_2.searchdata5, search_data_2.searchdata5);
        bdmf_session_print(session, "searchdata6 = %u = 0x%x\n", search_data_2.searchdata6, search_data_2.searchdata6);
        bdmf_session_print(session, "searchdata7 = %u = 0x%x\n", search_data_2.searchdata7, search_data_2.searchdata7);
        bdmf_session_print(session, "searchdata8 = %u = 0x%x\n", search_data_2.searchdata8, search_data_2.searchdata8);
        bdmf_session_print(session, "searchdata9 = %u = 0x%x\n", search_data_2.searchdata9, search_data_2.searchdata9);
        bdmf_session_print(session, "searchdata10_l = %u = 0x%x\n", search_data_2.searchdata10_l, search_data_2.searchdata10_l);
        break;
    }
    case cli_fpm_search_data_3:
    {
        fpm_search_data_3 search_data_3;
        ag_err = ag_drv_fpm_search_data_3_get(parm[1].value.unumber, &search_data_3);
        bdmf_session_print(session, "searchdata10_m = %u = 0x%x\n", search_data_3.searchdata10_m, search_data_3.searchdata10_m);
        bdmf_session_print(session, "searchdata11 = %u = 0x%x\n", search_data_3.searchdata11, search_data_3.searchdata11);
        bdmf_session_print(session, "searchdata12 = %u = 0x%x\n", search_data_3.searchdata12, search_data_3.searchdata12);
        bdmf_session_print(session, "searchdata13 = %u = 0x%x\n", search_data_3.searchdata13, search_data_3.searchdata13);
        bdmf_session_print(session, "searchdata14 = %u = 0x%x\n", search_data_3.searchdata14, search_data_3.searchdata14);
        bdmf_session_print(session, "searchdata15 = %u = 0x%x\n", search_data_3.searchdata15, search_data_3.searchdata15);
        break;
    }
    case cli_fpm_multicast_data:
    {
        uint32_t multicast;
        ag_err = ag_drv_fpm_multicast_data_get(parm[1].value.unumber, &multicast);
        bdmf_session_print(session, "multicast = %u = 0x%x\n", multicast, multicast);
        break;
    }
    case cli_fpm_compute_pool_data:
    {
        uint8_t poolid;
        ag_err = ag_drv_fpm_compute_pool_data_get(parm[1].value.unumber, &poolid);
        bdmf_session_print(session, "poolid = %u = 0x%x\n", poolid, poolid);
        break;
    }
    case cli_fpm_fpm_bb_force:
    {
        bdmf_boolean force;
        ag_err = ag_drv_fpm_fpm_bb_force_get(&force);
        bdmf_session_print(session, "force = %u = 0x%x\n", force, force);
        break;
    }
    case cli_fpm_fpm_bb_forced_ctrl:
    {
        uint16_t ctrl;
        ag_err = ag_drv_fpm_fpm_bb_forced_ctrl_get(&ctrl);
        bdmf_session_print(session, "ctrl = %u = 0x%x\n", ctrl, ctrl);
        break;
    }
    case cli_fpm_fpm_bb_forced_addr:
    {
        uint16_t ta_addr;
        uint8_t dest_addr;
        ag_err = ag_drv_fpm_fpm_bb_forced_addr_get(&ta_addr, &dest_addr);
        bdmf_session_print(session, "ta_addr = %u = 0x%x\n", ta_addr, ta_addr);
        bdmf_session_print(session, "dest_addr = %u = 0x%x\n", dest_addr, dest_addr);
        break;
    }
    case cli_fpm_fpm_bb_forced_data:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_forced_data_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_decode_cfg:
    {
        uint8_t dest_id;
        bdmf_boolean override_en;
        uint16_t route_addr;
        ag_err = ag_drv_fpm_fpm_bb_decode_cfg_get(&dest_id, &override_en, &route_addr);
        bdmf_session_print(session, "dest_id = %u = 0x%x\n", dest_id, dest_id);
        bdmf_session_print(session, "override_en = %u = 0x%x\n", override_en, override_en);
        bdmf_session_print(session, "route_addr = %u = 0x%x\n", route_addr, route_addr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_cfg:
    {
        uint8_t rxfifo_sw_addr;
        uint8_t txfifo_sw_addr;
        bdmf_boolean rxfifo_sw_rst;
        bdmf_boolean txfifo_sw_rst;
        ag_err = ag_drv_fpm_fpm_bb_dbg_cfg_get(&rxfifo_sw_addr, &txfifo_sw_addr, &rxfifo_sw_rst, &txfifo_sw_rst);
        bdmf_session_print(session, "rxfifo_sw_addr = %u = 0x%x\n", rxfifo_sw_addr, rxfifo_sw_addr);
        bdmf_session_print(session, "txfifo_sw_addr = %u = 0x%x\n", txfifo_sw_addr, txfifo_sw_addr);
        bdmf_session_print(session, "rxfifo_sw_rst = %u = 0x%x\n", rxfifo_sw_rst, rxfifo_sw_rst);
        bdmf_session_print(session, "txfifo_sw_rst = %u = 0x%x\n", txfifo_sw_rst, txfifo_sw_rst);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_sts:
    {
        fpm_fpm_bb_dbg_rxfifo_sts fpm_bb_dbg_rxfifo_sts;
        ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(&fpm_bb_dbg_rxfifo_sts);
        bdmf_session_print(session, "fifo_empty = %u = 0x%x\n", fpm_bb_dbg_rxfifo_sts.fifo_empty, fpm_bb_dbg_rxfifo_sts.fifo_empty);
        bdmf_session_print(session, "fifo_full = %u = 0x%x\n", fpm_bb_dbg_rxfifo_sts.fifo_full, fpm_bb_dbg_rxfifo_sts.fifo_full);
        bdmf_session_print(session, "fifo_used_words = %u = 0x%x\n", fpm_bb_dbg_rxfifo_sts.fifo_used_words, fpm_bb_dbg_rxfifo_sts.fifo_used_words);
        bdmf_session_print(session, "fifo_rd_cntr = %u = 0x%x\n", fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr, fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr);
        bdmf_session_print(session, "fifo_wr_cntr = %u = 0x%x\n", fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr, fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_sts:
    {
        fpm_fpm_bb_dbg_txfifo_sts fpm_bb_dbg_txfifo_sts;
        ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(&fpm_bb_dbg_txfifo_sts);
        bdmf_session_print(session, "fifo_empty = %u = 0x%x\n", fpm_bb_dbg_txfifo_sts.fifo_empty, fpm_bb_dbg_txfifo_sts.fifo_empty);
        bdmf_session_print(session, "fifo_full = %u = 0x%x\n", fpm_bb_dbg_txfifo_sts.fifo_full, fpm_bb_dbg_txfifo_sts.fifo_full);
        bdmf_session_print(session, "fifo_used_words = %u = 0x%x\n", fpm_bb_dbg_txfifo_sts.fifo_used_words, fpm_bb_dbg_txfifo_sts.fifo_used_words);
        bdmf_session_print(session, "fifo_rd_cntr = %u = 0x%x\n", fpm_bb_dbg_txfifo_sts.fifo_rd_cntr, fpm_bb_dbg_txfifo_sts.fifo_rd_cntr);
        bdmf_session_print(session, "fifo_wr_cntr = %u = 0x%x\n", fpm_bb_dbg_txfifo_sts.fifo_wr_cntr, fpm_bb_dbg_txfifo_sts.fifo_wr_cntr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_data1:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_data2:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data1:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data2:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data3:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_misc:
    {
        fpm_fpm_bb_misc fpm_bb_misc;
        ag_err = ag_drv_fpm_fpm_bb_misc_get(&fpm_bb_misc);
        bdmf_session_print(session, "old_task_num = %u = 0x%x\n", fpm_bb_misc.old_task_num, fpm_bb_misc.old_task_num);
        bdmf_session_print(session, "alc_fre_arb_rr = %u = 0x%x\n", fpm_bb_misc.alc_fre_arb_rr, fpm_bb_misc.alc_fre_arb_rr);
        bdmf_session_print(session, "alc_fst_ack = %u = 0x%x\n", fpm_bb_misc.alc_fst_ack, fpm_bb_misc.alc_fst_ack);
        bdmf_session_print(session, "pool_0_size = %u = 0x%x\n", fpm_bb_misc.pool_0_size, fpm_bb_misc.pool_0_size);
        bdmf_session_print(session, "pool_1_size = %u = 0x%x\n", fpm_bb_misc.pool_1_size, fpm_bb_misc.pool_1_size);
        bdmf_session_print(session, "poolx_en = %u = 0x%x\n", fpm_bb_misc.poolx_en, fpm_bb_misc.poolx_en);
        break;
    }
    case cli_fpm_clk_gate_cntrl:
    {
        fpm_clk_gate_cntrl clk_gate_cntrl;
        ag_err = ag_drv_fpm_clk_gate_cntrl_get(&clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", clk_gate_cntrl.timer_val, clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", clk_gate_cntrl.keep_alive_intrvl, clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", clk_gate_cntrl.keep_alive_cyc, clk_gate_cntrl.keep_alive_cyc);
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
        bdmf_boolean pool1_enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool1_en_set( %u)\n",
            pool1_enable);
        ag_err = ag_drv_fpm_pool1_en_set(pool1_enable);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_en_get(&pool1_enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_en_get( %u)\n",
                pool1_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool1_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean fpm_bb_soft_reset = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_bb_reset_set( %u)\n",
            fpm_bb_soft_reset);
        ag_err = ag_drv_fpm_bb_reset_set(fpm_bb_soft_reset);
        if (!ag_err)
            ag_err = ag_drv_fpm_bb_reset_get(&fpm_bb_soft_reset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_bb_reset_get( %u)\n",
                fpm_bb_soft_reset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_bb_soft_reset != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr0_alloc_weight = gtmv(m, 8);
        uint8_t ddr0_free_weight = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_fpm_ddr0_weight_set( %u %u)\n",
            ddr0_alloc_weight, ddr0_free_weight);
        ag_err = ag_drv_fpm_ddr0_weight_set(ddr0_alloc_weight, ddr0_free_weight);
        if (!ag_err)
            ag_err = ag_drv_fpm_ddr0_weight_get(&ddr0_alloc_weight, &ddr0_free_weight);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_ddr0_weight_get( %u %u)\n",
                ddr0_alloc_weight, ddr0_free_weight);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr0_alloc_weight != gtmv(m, 8) || ddr0_free_weight != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr1_alloc_weight = gtmv(m, 8);
        uint8_t ddr1_free_weight = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_fpm_ddr1_weight_set( %u %u)\n",
            ddr1_alloc_weight, ddr1_free_weight);
        ag_err = ag_drv_fpm_ddr1_weight_set(ddr1_alloc_weight, ddr1_free_weight);
        if (!ag_err)
            ag_err = ag_drv_fpm_ddr1_weight_get(&ddr1_alloc_weight, &ddr1_free_weight);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_ddr1_weight_get( %u %u)\n",
                ddr1_alloc_weight, ddr1_free_weight);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr1_alloc_weight != gtmv(m, 8) || ddr1_free_weight != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool_cfg pool_cfg = {.fp_buf_size = gtmv(m, 3), .pool_base_address = gtmv(m, 30), .pool_base_address_pool2 = gtmv(m, 30)};
        bdmf_session_print(session, "ag_drv_fpm_pool_cfg_set( %u %u %u)\n",
            pool_cfg.fp_buf_size, pool_cfg.pool_base_address, pool_cfg.pool_base_address_pool2);
        ag_err = ag_drv_fpm_pool_cfg_set(&pool_cfg);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool_cfg_get(&pool_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool_cfg_get( %u %u %u)\n",
                pool_cfg.fp_buf_size, pool_cfg.pool_base_address, pool_cfg.pool_base_address_pool2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_cfg.fp_buf_size != gtmv(m, 3) || pool_cfg.pool_base_address != gtmv(m, 30) || pool_cfg.pool_base_address_pool2 != gtmv(m, 30))
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
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_boolean ddr = gtmv(m, 1);
        uint32_t token_index = gtmv(m, 18);
        uint16_t token_size = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_set( %u %u %u %u)\n",
            token_valid, ddr, token_index, token_size);
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_get( %u %u %u %u)\n",
                token_valid, ddr, token_index, token_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_valid != gtmv(m, 1) || ddr != gtmv(m, 1) || token_index != gtmv(m, 18) || token_size != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_boolean ddr = gtmv(m, 1);
        uint32_t token_index = gtmv(m, 18);
        uint16_t token_size = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_set( %u %u %u %u)\n",
            token_valid, ddr, token_index, token_size);
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_get( %u %u %u %u)\n",
                token_valid, ddr, token_index, token_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_valid != gtmv(m, 1) || ddr != gtmv(m, 1) || token_index != gtmv(m, 18) || token_size != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_boolean ddr = gtmv(m, 1);
        uint32_t token_index = gtmv(m, 18);
        uint16_t token_size = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_set( %u %u %u %u)\n",
            token_valid, ddr, token_index, token_size);
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_get( %u %u %u %u)\n",
                token_valid, ddr, token_index, token_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_valid != gtmv(m, 1) || ddr != gtmv(m, 1) || token_index != gtmv(m, 18) || token_size != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_boolean ddr = gtmv(m, 1);
        uint32_t token_index = gtmv(m, 18);
        uint16_t token_size = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_set( %u %u %u %u)\n",
            token_valid, ddr, token_index, token_size);
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_get( %u %u %u %u)\n",
                token_valid, ddr, token_index, token_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_valid != gtmv(m, 1) || ddr != gtmv(m, 1) || token_index != gtmv(m, 18) || token_size != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t mem_data1 = gtmv(m, 32);
        uint32_t mem_data2 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_back_door_mem_set( %u %u)\n",
            mem_data1, mem_data2);
        ag_err = ag_drv_fpm_back_door_mem_set(mem_data1, mem_data2);
        if (!ag_err)
            ag_err = ag_drv_fpm_back_door_mem_get(&mem_data1, &mem_data2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_back_door_mem_get( %u %u)\n",
                mem_data1, mem_data2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mem_data1 != gtmv(m, 32) || mem_data2 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t expired_count = gtmv(m, 32);
        uint32_t recovered_count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_count_get(&expired_count, &recovered_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_count_get( %u %u)\n",
                expired_count, recovered_count);
        }
    }

    {
        uint32_t expired_count = gtmv(m, 32);
        uint32_t recovered_count = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_count_get(&expired_count, &recovered_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_count_get( %u %u)\n",
                expired_count, recovered_count);
        }
    }

    {
        fpm_timer timer = {.long_aging_timer = gtmv(m, 32), .short_aging_timer = gtmv(m, 32), .recycle_timer = gtmv(m, 16)};
        bdmf_session_print(session, "ag_drv_fpm_timer_set( %u %u %u)\n",
            timer.long_aging_timer, timer.short_aging_timer, timer.recycle_timer);
        ag_err = ag_drv_fpm_timer_set(&timer);
        if (!ag_err)
            ag_err = ag_drv_fpm_timer_get(&timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_timer_get( %u %u %u)\n",
                timer.long_aging_timer, timer.short_aging_timer, timer.recycle_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (timer.long_aging_timer != gtmv(m, 32) || timer.short_aging_timer != gtmv(m, 32) || timer.recycle_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pool1_search_mode = gtmv(m, 1);
        bdmf_boolean enable_jumbo_support = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpm_cfg1_set( %u %u)\n",
            pool1_search_mode, enable_jumbo_support);
        ag_err = ag_drv_fpm_fpm_cfg1_set(pool1_search_mode, enable_jumbo_support);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_cfg1_get(&pool1_search_mode, &enable_jumbo_support);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_cfg1_get( %u %u)\n",
                pool1_search_mode, enable_jumbo_support);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool1_search_mode != gtmv(m, 1) || enable_jumbo_support != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bb_ddr_sel = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_cfg_set( %u)\n",
            bb_ddr_sel);
        ag_err = ag_drv_fpm_fpm_bb_cfg_set(bb_ddr_sel);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_cfg_get(&bb_ddr_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_cfg_get( %u)\n",
                bb_ddr_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bb_ddr_sel != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_intr_msk pool2_intr_msk = {.alloc_fifo_full_msk = gtmv(m, 1), .free_fifo_full_msk = gtmv(m, 1), .pool_full_msk = gtmv(m, 1), .free_token_no_valid_msk = gtmv(m, 1), .free_token_index_out_of_range_msk = gtmv(m, 1), .multi_token_no_valid_msk = gtmv(m, 1), .multi_token_index_out_of_range_msk = gtmv(m, 1), .pool_dis_free_multi_msk = gtmv(m, 1), .memory_corrupt_msk = gtmv(m, 1), .xoff_msk = gtmv(m, 1), .xon_msk = gtmv(m, 1), .illegal_address_access_msk = gtmv(m, 1), .illegal_alloc_request_msk = gtmv(m, 1), .expired_token_det_msk = gtmv(m, 1), .expired_token_recov_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool1_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_token_no_valid_msk, 
            pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.pool_dis_free_multi_msk, 
            pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.illegal_address_access_msk, 
            pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_recov_msk);
        ag_err = ag_drv_fpm_pool1_intr_msk_set(&pool2_intr_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_intr_msk_get(&pool2_intr_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_token_no_valid_msk, 
                pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.pool_dis_free_multi_msk, 
                pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.illegal_address_access_msk, 
                pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_recov_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_intr_msk.alloc_fifo_full_msk != gtmv(m, 1) || pool2_intr_msk.free_fifo_full_msk != gtmv(m, 1) || pool2_intr_msk.pool_full_msk != gtmv(m, 1) || pool2_intr_msk.free_token_no_valid_msk != gtmv(m, 1) || pool2_intr_msk.free_token_index_out_of_range_msk != gtmv(m, 1) || pool2_intr_msk.multi_token_no_valid_msk != gtmv(m, 1) || pool2_intr_msk.multi_token_index_out_of_range_msk != gtmv(m, 1) || pool2_intr_msk.pool_dis_free_multi_msk != gtmv(m, 1) || pool2_intr_msk.memory_corrupt_msk != gtmv(m, 1) || pool2_intr_msk.xoff_msk != gtmv(m, 1) || pool2_intr_msk.xon_msk != gtmv(m, 1) || pool2_intr_msk.illegal_address_access_msk != gtmv(m, 1) || pool2_intr_msk.illegal_alloc_request_msk != gtmv(m, 1) || pool2_intr_msk.expired_token_det_msk != gtmv(m, 1) || pool2_intr_msk.expired_token_recov_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_intr_sts pool2_intr_sts = {.alloc_fifo_full_sts = gtmv(m, 1), .free_fifo_full_sts = gtmv(m, 1), .pool_full_sts = gtmv(m, 1), .free_token_no_valid_sts = gtmv(m, 1), .free_token_index_out_of_range_sts = gtmv(m, 1), .multi_token_no_valid_sts = gtmv(m, 1), .multi_token_index_out_of_range_sts = gtmv(m, 1), .pool_dis_free_multi_sts = gtmv(m, 1), .memory_corrupt_sts = gtmv(m, 1), .xoff_state_sts = gtmv(m, 1), .xon_state_sts = gtmv(m, 1), .illegal_address_access_sts = gtmv(m, 1), .illegal_alloc_request_sts = gtmv(m, 1), .expired_token_det_sts = gtmv(m, 1), .expired_token_recov_sts = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool1_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_token_no_valid_sts, 
            pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.pool_dis_free_multi_sts, 
            pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.illegal_address_access_sts, 
            pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_recov_sts);
        ag_err = ag_drv_fpm_pool1_intr_sts_set(&pool2_intr_sts);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_intr_sts_get(&pool2_intr_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_token_no_valid_sts, 
                pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.pool_dis_free_multi_sts, 
                pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.illegal_address_access_sts, 
                pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_recov_sts);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_intr_sts.alloc_fifo_full_sts != gtmv(m, 1) || pool2_intr_sts.free_fifo_full_sts != gtmv(m, 1) || pool2_intr_sts.pool_full_sts != gtmv(m, 1) || pool2_intr_sts.free_token_no_valid_sts != gtmv(m, 1) || pool2_intr_sts.free_token_index_out_of_range_sts != gtmv(m, 1) || pool2_intr_sts.multi_token_no_valid_sts != gtmv(m, 1) || pool2_intr_sts.multi_token_index_out_of_range_sts != gtmv(m, 1) || pool2_intr_sts.pool_dis_free_multi_sts != gtmv(m, 1) || pool2_intr_sts.memory_corrupt_sts != gtmv(m, 1) || pool2_intr_sts.xoff_state_sts != gtmv(m, 1) || pool2_intr_sts.xon_state_sts != gtmv(m, 1) || pool2_intr_sts.illegal_address_access_sts != gtmv(m, 1) || pool2_intr_sts.illegal_alloc_request_sts != gtmv(m, 1) || pool2_intr_sts.expired_token_det_sts != gtmv(m, 1) || pool2_intr_sts.expired_token_recov_sts != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_stall_msk pool2_stall_msk = {.free_token_no_valid_stall_msk = gtmv(m, 1), .free_token_index_out_of_range_stall_msk = gtmv(m, 1), .multi_token_no_valid_stall_msk = gtmv(m, 1), .multi_token_index_out_of_range_stall_msk = gtmv(m, 1), .memory_corrupt_stall_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool1_stall_msk_set( %u %u %u %u %u)\n",
            pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, 
            pool2_stall_msk.memory_corrupt_stall_msk);
        ag_err = ag_drv_fpm_pool1_stall_msk_set(&pool2_stall_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_stall_msk_get(&pool2_stall_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_stall_msk_get( %u %u %u %u %u)\n",
                pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, 
                pool2_stall_msk.memory_corrupt_stall_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_stall_msk.free_token_no_valid_stall_msk != gtmv(m, 1) || pool2_stall_msk.free_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool2_stall_msk.multi_token_no_valid_stall_msk != gtmv(m, 1) || pool2_stall_msk.multi_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool2_stall_msk.memory_corrupt_stall_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_intr_msk pool2_intr_msk = {.alloc_fifo_full_msk = gtmv(m, 1), .free_fifo_full_msk = gtmv(m, 1), .pool_full_msk = gtmv(m, 1), .free_token_no_valid_msk = gtmv(m, 1), .free_token_index_out_of_range_msk = gtmv(m, 1), .multi_token_no_valid_msk = gtmv(m, 1), .multi_token_index_out_of_range_msk = gtmv(m, 1), .pool_dis_free_multi_msk = gtmv(m, 1), .memory_corrupt_msk = gtmv(m, 1), .xoff_msk = gtmv(m, 1), .xon_msk = gtmv(m, 1), .illegal_address_access_msk = gtmv(m, 1), .illegal_alloc_request_msk = gtmv(m, 1), .expired_token_det_msk = gtmv(m, 1), .expired_token_recov_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool2_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_token_no_valid_msk, 
            pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.pool_dis_free_multi_msk, 
            pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.illegal_address_access_msk, 
            pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_recov_msk);
        ag_err = ag_drv_fpm_pool2_intr_msk_set(&pool2_intr_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_intr_msk_get(&pool2_intr_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_token_no_valid_msk, 
                pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.pool_dis_free_multi_msk, 
                pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.illegal_address_access_msk, 
                pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_recov_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_intr_msk.alloc_fifo_full_msk != gtmv(m, 1) || pool2_intr_msk.free_fifo_full_msk != gtmv(m, 1) || pool2_intr_msk.pool_full_msk != gtmv(m, 1) || pool2_intr_msk.free_token_no_valid_msk != gtmv(m, 1) || pool2_intr_msk.free_token_index_out_of_range_msk != gtmv(m, 1) || pool2_intr_msk.multi_token_no_valid_msk != gtmv(m, 1) || pool2_intr_msk.multi_token_index_out_of_range_msk != gtmv(m, 1) || pool2_intr_msk.pool_dis_free_multi_msk != gtmv(m, 1) || pool2_intr_msk.memory_corrupt_msk != gtmv(m, 1) || pool2_intr_msk.xoff_msk != gtmv(m, 1) || pool2_intr_msk.xon_msk != gtmv(m, 1) || pool2_intr_msk.illegal_address_access_msk != gtmv(m, 1) || pool2_intr_msk.illegal_alloc_request_msk != gtmv(m, 1) || pool2_intr_msk.expired_token_det_msk != gtmv(m, 1) || pool2_intr_msk.expired_token_recov_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_intr_sts pool2_intr_sts = {.alloc_fifo_full_sts = gtmv(m, 1), .free_fifo_full_sts = gtmv(m, 1), .pool_full_sts = gtmv(m, 1), .free_token_no_valid_sts = gtmv(m, 1), .free_token_index_out_of_range_sts = gtmv(m, 1), .multi_token_no_valid_sts = gtmv(m, 1), .multi_token_index_out_of_range_sts = gtmv(m, 1), .pool_dis_free_multi_sts = gtmv(m, 1), .memory_corrupt_sts = gtmv(m, 1), .xoff_state_sts = gtmv(m, 1), .xon_state_sts = gtmv(m, 1), .illegal_address_access_sts = gtmv(m, 1), .illegal_alloc_request_sts = gtmv(m, 1), .expired_token_det_sts = gtmv(m, 1), .expired_token_recov_sts = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool2_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_token_no_valid_sts, 
            pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.pool_dis_free_multi_sts, 
            pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.illegal_address_access_sts, 
            pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_recov_sts);
        ag_err = ag_drv_fpm_pool2_intr_sts_set(&pool2_intr_sts);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_intr_sts_get(&pool2_intr_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_token_no_valid_sts, 
                pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.pool_dis_free_multi_sts, 
                pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.illegal_address_access_sts, 
                pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_recov_sts);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_intr_sts.alloc_fifo_full_sts != gtmv(m, 1) || pool2_intr_sts.free_fifo_full_sts != gtmv(m, 1) || pool2_intr_sts.pool_full_sts != gtmv(m, 1) || pool2_intr_sts.free_token_no_valid_sts != gtmv(m, 1) || pool2_intr_sts.free_token_index_out_of_range_sts != gtmv(m, 1) || pool2_intr_sts.multi_token_no_valid_sts != gtmv(m, 1) || pool2_intr_sts.multi_token_index_out_of_range_sts != gtmv(m, 1) || pool2_intr_sts.pool_dis_free_multi_sts != gtmv(m, 1) || pool2_intr_sts.memory_corrupt_sts != gtmv(m, 1) || pool2_intr_sts.xoff_state_sts != gtmv(m, 1) || pool2_intr_sts.xon_state_sts != gtmv(m, 1) || pool2_intr_sts.illegal_address_access_sts != gtmv(m, 1) || pool2_intr_sts.illegal_alloc_request_sts != gtmv(m, 1) || pool2_intr_sts.expired_token_det_sts != gtmv(m, 1) || pool2_intr_sts.expired_token_recov_sts != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_stall_msk pool2_stall_msk = {.free_token_no_valid_stall_msk = gtmv(m, 1), .free_token_index_out_of_range_stall_msk = gtmv(m, 1), .multi_token_no_valid_stall_msk = gtmv(m, 1), .multi_token_index_out_of_range_stall_msk = gtmv(m, 1), .memory_corrupt_stall_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool2_stall_msk_set( %u %u %u %u %u)\n",
            pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, 
            pool2_stall_msk.memory_corrupt_stall_msk);
        ag_err = ag_drv_fpm_pool2_stall_msk_set(&pool2_stall_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stall_msk_get(&pool2_stall_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stall_msk_get( %u %u %u %u %u)\n",
                pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, 
                pool2_stall_msk.memory_corrupt_stall_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool2_stall_msk.free_token_no_valid_stall_msk != gtmv(m, 1) || pool2_stall_msk.free_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool2_stall_msk.multi_token_no_valid_stall_msk != gtmv(m, 1) || pool2_stall_msk.multi_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool2_stall_msk.memory_corrupt_stall_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool3_intr_msk pool3_intr_msk = {.alloc_fifo_full_msk = gtmv(m, 1), .free_fifo_full_msk = gtmv(m, 1), .pool_full_msk = gtmv(m, 1), .free_token_no_valid_msk = gtmv(m, 1), .free_token_index_out_of_range_msk = gtmv(m, 1), .multi_token_no_valid_msk = gtmv(m, 1), .multi_token_index_out_of_range_msk = gtmv(m, 1), .pool_dis_free_multi_msk = gtmv(m, 1), .memory_corrupt_msk = gtmv(m, 1), .xoff_msk = gtmv(m, 1), .xon_msk = gtmv(m, 1), .illegal_address_access_msk = gtmv(m, 1), .illegal_alloc_request_msk = gtmv(m, 1), .expired_token_det_msk = gtmv(m, 1), .expired_token_recov_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool3_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool3_intr_msk.alloc_fifo_full_msk, pool3_intr_msk.free_fifo_full_msk, pool3_intr_msk.pool_full_msk, pool3_intr_msk.free_token_no_valid_msk, 
            pool3_intr_msk.free_token_index_out_of_range_msk, pool3_intr_msk.multi_token_no_valid_msk, pool3_intr_msk.multi_token_index_out_of_range_msk, pool3_intr_msk.pool_dis_free_multi_msk, 
            pool3_intr_msk.memory_corrupt_msk, pool3_intr_msk.xoff_msk, pool3_intr_msk.xon_msk, pool3_intr_msk.illegal_address_access_msk, 
            pool3_intr_msk.illegal_alloc_request_msk, pool3_intr_msk.expired_token_det_msk, pool3_intr_msk.expired_token_recov_msk);
        ag_err = ag_drv_fpm_pool3_intr_msk_set(&pool3_intr_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_intr_msk_get(&pool3_intr_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool3_intr_msk.alloc_fifo_full_msk, pool3_intr_msk.free_fifo_full_msk, pool3_intr_msk.pool_full_msk, pool3_intr_msk.free_token_no_valid_msk, 
                pool3_intr_msk.free_token_index_out_of_range_msk, pool3_intr_msk.multi_token_no_valid_msk, pool3_intr_msk.multi_token_index_out_of_range_msk, pool3_intr_msk.pool_dis_free_multi_msk, 
                pool3_intr_msk.memory_corrupt_msk, pool3_intr_msk.xoff_msk, pool3_intr_msk.xon_msk, pool3_intr_msk.illegal_address_access_msk, 
                pool3_intr_msk.illegal_alloc_request_msk, pool3_intr_msk.expired_token_det_msk, pool3_intr_msk.expired_token_recov_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool3_intr_msk.alloc_fifo_full_msk != gtmv(m, 1) || pool3_intr_msk.free_fifo_full_msk != gtmv(m, 1) || pool3_intr_msk.pool_full_msk != gtmv(m, 1) || pool3_intr_msk.free_token_no_valid_msk != gtmv(m, 1) || pool3_intr_msk.free_token_index_out_of_range_msk != gtmv(m, 1) || pool3_intr_msk.multi_token_no_valid_msk != gtmv(m, 1) || pool3_intr_msk.multi_token_index_out_of_range_msk != gtmv(m, 1) || pool3_intr_msk.pool_dis_free_multi_msk != gtmv(m, 1) || pool3_intr_msk.memory_corrupt_msk != gtmv(m, 1) || pool3_intr_msk.xoff_msk != gtmv(m, 1) || pool3_intr_msk.xon_msk != gtmv(m, 1) || pool3_intr_msk.illegal_address_access_msk != gtmv(m, 1) || pool3_intr_msk.illegal_alloc_request_msk != gtmv(m, 1) || pool3_intr_msk.expired_token_det_msk != gtmv(m, 1) || pool3_intr_msk.expired_token_recov_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool3_intr_sts pool3_intr_sts = {.alloc_fifo_full_sts = gtmv(m, 1), .free_fifo_full_sts = gtmv(m, 1), .pool_full_sts = gtmv(m, 1), .free_token_no_valid_sts = gtmv(m, 1), .free_token_index_out_of_range_sts = gtmv(m, 1), .multi_token_no_valid_sts = gtmv(m, 1), .multi_token_index_out_of_range_sts = gtmv(m, 1), .pool_dis_free_multi_sts = gtmv(m, 1), .memory_corrupt_sts = gtmv(m, 1), .xoff_state_sts = gtmv(m, 1), .xon_state_sts = gtmv(m, 1), .illegal_address_access_sts = gtmv(m, 1), .illegal_alloc_request_sts = gtmv(m, 1), .expired_token_det_sts = gtmv(m, 1), .expired_token_recov_sts = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool3_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            pool3_intr_sts.alloc_fifo_full_sts, pool3_intr_sts.free_fifo_full_sts, pool3_intr_sts.pool_full_sts, pool3_intr_sts.free_token_no_valid_sts, 
            pool3_intr_sts.free_token_index_out_of_range_sts, pool3_intr_sts.multi_token_no_valid_sts, pool3_intr_sts.multi_token_index_out_of_range_sts, pool3_intr_sts.pool_dis_free_multi_sts, 
            pool3_intr_sts.memory_corrupt_sts, pool3_intr_sts.xoff_state_sts, pool3_intr_sts.xon_state_sts, pool3_intr_sts.illegal_address_access_sts, 
            pool3_intr_sts.illegal_alloc_request_sts, pool3_intr_sts.expired_token_det_sts, pool3_intr_sts.expired_token_recov_sts);
        ag_err = ag_drv_fpm_pool3_intr_sts_set(&pool3_intr_sts);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_intr_sts_get(&pool3_intr_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                pool3_intr_sts.alloc_fifo_full_sts, pool3_intr_sts.free_fifo_full_sts, pool3_intr_sts.pool_full_sts, pool3_intr_sts.free_token_no_valid_sts, 
                pool3_intr_sts.free_token_index_out_of_range_sts, pool3_intr_sts.multi_token_no_valid_sts, pool3_intr_sts.multi_token_index_out_of_range_sts, pool3_intr_sts.pool_dis_free_multi_sts, 
                pool3_intr_sts.memory_corrupt_sts, pool3_intr_sts.xoff_state_sts, pool3_intr_sts.xon_state_sts, pool3_intr_sts.illegal_address_access_sts, 
                pool3_intr_sts.illegal_alloc_request_sts, pool3_intr_sts.expired_token_det_sts, pool3_intr_sts.expired_token_recov_sts);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool3_intr_sts.alloc_fifo_full_sts != gtmv(m, 1) || pool3_intr_sts.free_fifo_full_sts != gtmv(m, 1) || pool3_intr_sts.pool_full_sts != gtmv(m, 1) || pool3_intr_sts.free_token_no_valid_sts != gtmv(m, 1) || pool3_intr_sts.free_token_index_out_of_range_sts != gtmv(m, 1) || pool3_intr_sts.multi_token_no_valid_sts != gtmv(m, 1) || pool3_intr_sts.multi_token_index_out_of_range_sts != gtmv(m, 1) || pool3_intr_sts.pool_dis_free_multi_sts != gtmv(m, 1) || pool3_intr_sts.memory_corrupt_sts != gtmv(m, 1) || pool3_intr_sts.xoff_state_sts != gtmv(m, 1) || pool3_intr_sts.xon_state_sts != gtmv(m, 1) || pool3_intr_sts.illegal_address_access_sts != gtmv(m, 1) || pool3_intr_sts.illegal_alloc_request_sts != gtmv(m, 1) || pool3_intr_sts.expired_token_det_sts != gtmv(m, 1) || pool3_intr_sts.expired_token_recov_sts != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool3_stall_msk pool3_stall_msk = {.free_token_no_valid_stall_msk = gtmv(m, 1), .free_token_index_out_of_range_stall_msk = gtmv(m, 1), .multi_token_no_valid_stall_msk = gtmv(m, 1), .multi_token_index_out_of_range_stall_msk = gtmv(m, 1), .memory_corrupt_stall_msk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool3_stall_msk_set( %u %u %u %u %u)\n",
            pool3_stall_msk.free_token_no_valid_stall_msk, pool3_stall_msk.free_token_index_out_of_range_stall_msk, pool3_stall_msk.multi_token_no_valid_stall_msk, pool3_stall_msk.multi_token_index_out_of_range_stall_msk, 
            pool3_stall_msk.memory_corrupt_stall_msk);
        ag_err = ag_drv_fpm_pool3_stall_msk_set(&pool3_stall_msk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stall_msk_get(&pool3_stall_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stall_msk_get( %u %u %u %u %u)\n",
                pool3_stall_msk.free_token_no_valid_stall_msk, pool3_stall_msk.free_token_index_out_of_range_stall_msk, pool3_stall_msk.multi_token_no_valid_stall_msk, pool3_stall_msk.multi_token_index_out_of_range_stall_msk, 
                pool3_stall_msk.memory_corrupt_stall_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool3_stall_msk.free_token_no_valid_stall_msk != gtmv(m, 1) || pool3_stall_msk.free_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool3_stall_msk.multi_token_no_valid_stall_msk != gtmv(m, 1) || pool3_stall_msk.multi_token_index_out_of_range_stall_msk != gtmv(m, 1) || pool3_stall_msk.memory_corrupt_stall_msk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t pool_base_address_pool3 = gtmv(m, 30);
        bdmf_session_print(session, "ag_drv_fpm_pool1_cfg4_set( %u)\n",
            pool_base_address_pool3);
        ag_err = ag_drv_fpm_pool1_cfg4_set(pool_base_address_pool3);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_cfg4_get(&pool_base_address_pool3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_cfg4_get( %u)\n",
                pool_base_address_pool3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_base_address_pool3 != gtmv(m, 30))
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
        bdmf_session_print(session, "ag_drv_fpm_pool1_stat5_set( %u)\n",
            mem_corrupt_sts_related_alloc_token);
        ag_err = ag_drv_fpm_pool1_stat5_set(mem_corrupt_sts_related_alloc_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_stat5_get(&mem_corrupt_sts_related_alloc_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_stat5_get( %u)\n",
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
        uint16_t undrfl = gtmv(m, 16);
        uint16_t ovrfl = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat1_set( %u %u)\n",
            undrfl, ovrfl);
        ag_err = ag_drv_fpm_pool2_stat1_set(undrfl, ovrfl);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat1_get(&undrfl, &ovrfl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat1_get( %u %u)\n",
                undrfl, ovrfl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (undrfl != gtmv(m, 16) || ovrfl != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool2_stat2 pool2_stat2 = {.num_of_tokens_available = gtmv(m, 22), .alloc_fifo_empty = gtmv(m, 1), .alloc_fifo_full = gtmv(m, 1), .free_fifo_empty = gtmv(m, 1), .free_fifo_full = gtmv(m, 1), .pool_full = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat2_get(&pool2_stat2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat2_get( %u %u %u %u %u %u)\n",
                pool2_stat2.num_of_tokens_available, pool2_stat2.alloc_fifo_empty, pool2_stat2.alloc_fifo_full, pool2_stat2.free_fifo_empty, 
                pool2_stat2.free_fifo_full, pool2_stat2.pool_full);
        }
    }

    {
        uint32_t num_of_not_valid_token_frees = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat3_set( %u)\n",
            num_of_not_valid_token_frees);
        ag_err = ag_drv_fpm_pool2_stat3_set(num_of_not_valid_token_frees);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat3_get(&num_of_not_valid_token_frees);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat3_get( %u)\n",
                num_of_not_valid_token_frees);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (num_of_not_valid_token_frees != gtmv(m, 22))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t num_of_not_valid_token_multi = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat4_set( %u)\n",
            num_of_not_valid_token_multi);
        ag_err = ag_drv_fpm_pool2_stat4_set(num_of_not_valid_token_multi);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat4_get(&num_of_not_valid_token_multi);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat4_get( %u)\n",
                num_of_not_valid_token_multi);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (num_of_not_valid_token_multi != gtmv(m, 22))
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
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat5_set( %u)\n",
            mem_corrupt_sts_related_alloc_token);
        ag_err = ag_drv_fpm_pool2_stat5_set(mem_corrupt_sts_related_alloc_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat5_get(&mem_corrupt_sts_related_alloc_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat5_get( %u)\n",
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
        uint32_t invalid_free_token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat6_set( %u)\n",
            invalid_free_token);
        ag_err = ag_drv_fpm_pool2_stat6_set(invalid_free_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat6_get(&invalid_free_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat6_get( %u)\n",
                invalid_free_token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (invalid_free_token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t invalid_mcast_token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat7_set( %u)\n",
            invalid_mcast_token);
        ag_err = ag_drv_fpm_pool2_stat7_set(invalid_mcast_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat7_get(&invalid_mcast_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat7_get( %u)\n",
                invalid_mcast_token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (invalid_mcast_token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t tokens_available_low_wtmk = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_fpm_pool2_stat8_set( %u)\n",
            tokens_available_low_wtmk);
        ag_err = ag_drv_fpm_pool2_stat8_set(tokens_available_low_wtmk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_stat8_get(&tokens_available_low_wtmk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_stat8_get( %u)\n",
                tokens_available_low_wtmk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tokens_available_low_wtmk != gtmv(m, 22))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t undrfl = gtmv(m, 16);
        uint16_t ovrfl = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_pool3_stat1_set( %u %u)\n",
            undrfl, ovrfl);
        ag_err = ag_drv_fpm_pool3_stat1_set(undrfl, ovrfl);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat1_get(&undrfl, &ovrfl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat1_get( %u %u)\n",
                undrfl, ovrfl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (undrfl != gtmv(m, 16) || ovrfl != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool3_stat2 pool3_stat2 = {.num_of_tokens_available = gtmv(m, 22), .alloc_fifo_empty = gtmv(m, 1), .alloc_fifo_full = gtmv(m, 1), .free_fifo_empty = gtmv(m, 1), .free_fifo_full = gtmv(m, 1), .pool_full = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat2_get(&pool3_stat2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat2_get( %u %u %u %u %u %u)\n",
                pool3_stat2.num_of_tokens_available, pool3_stat2.alloc_fifo_empty, pool3_stat2.alloc_fifo_full, pool3_stat2.free_fifo_empty, 
                pool3_stat2.free_fifo_full, pool3_stat2.pool_full);
        }
    }

    {
        uint32_t num_of_not_valid_token_frees = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_fpm_pool3_stat3_set( %u)\n",
            num_of_not_valid_token_frees);
        ag_err = ag_drv_fpm_pool3_stat3_set(num_of_not_valid_token_frees);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat3_get(&num_of_not_valid_token_frees);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat3_get( %u)\n",
                num_of_not_valid_token_frees);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (num_of_not_valid_token_frees != gtmv(m, 22))
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
        bdmf_session_print(session, "ag_drv_fpm_pool3_stat5_set( %u)\n",
            mem_corrupt_sts_related_alloc_token);
        ag_err = ag_drv_fpm_pool3_stat5_set(mem_corrupt_sts_related_alloc_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat5_get(&mem_corrupt_sts_related_alloc_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat5_get( %u)\n",
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
        uint32_t invalid_free_token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_pool3_stat6_set( %u)\n",
            invalid_free_token);
        ag_err = ag_drv_fpm_pool3_stat6_set(invalid_free_token);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat6_get(&invalid_free_token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat6_get( %u)\n",
                invalid_free_token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (invalid_free_token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t tokens_available_low_wtmk = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_fpm_pool3_stat8_set( %u)\n",
            tokens_available_low_wtmk);
        ag_err = ag_drv_fpm_pool3_stat8_set(tokens_available_low_wtmk);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_stat8_get(&tokens_available_low_wtmk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_stat8_get( %u)\n",
                tokens_available_low_wtmk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tokens_available_low_wtmk != gtmv(m, 22))
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
        bdmf_session_print(session, "ag_drv_fpm_pool1_xon_xoff_cfg_set( %u %u)\n",
            xoff_threshold, xon_threshold);
        ag_err = ag_drv_fpm_pool1_xon_xoff_cfg_set(xoff_threshold, xon_threshold);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_xon_xoff_cfg_get(&xoff_threshold, &xon_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_xon_xoff_cfg_get( %u %u)\n",
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
        bdmf_session_print(session, "ag_drv_fpm_fpm_not_empty_cfg_set( %u)\n",
            not_empty_threshold);
        ag_err = ag_drv_fpm_fpm_not_empty_cfg_set(not_empty_threshold);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_not_empty_cfg_get(&not_empty_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_not_empty_cfg_get( %u)\n",
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
        uint16_t mem_addr = gtmv(m, 16);
        uint8_t mem_sel = gtmv(m, 2);
        bdmf_boolean mem_rd = gtmv(m, 1);
        bdmf_boolean mem_wr = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_mem_ctl_set( %u %u %u %u)\n",
            mem_addr, mem_sel, mem_rd, mem_wr);
        ag_err = ag_drv_fpm_mem_ctl_set(mem_addr, mem_sel, mem_rd, mem_wr);
        if (!ag_err)
            ag_err = ag_drv_fpm_mem_ctl_get(&mem_addr, &mem_sel, &mem_rd, &mem_wr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_mem_ctl_get( %u %u %u %u)\n",
                mem_addr, mem_sel, mem_rd, mem_wr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mem_addr != gtmv(m, 16) || mem_sel != gtmv(m, 2) || mem_rd != gtmv(m, 1) || mem_wr != gtmv(m, 1))
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
        uint16_t end_index = gtmv(m, 16);
        uint16_t start_index = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool1_set( %u %u)\n",
            end_index, start_index);
        ag_err = ag_drv_fpm_token_recover_start_end_pool1_set(end_index, start_index);
        if (!ag_err)
            ag_err = ag_drv_fpm_token_recover_start_end_pool1_get(&end_index, &start_index);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool1_get( %u %u)\n",
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
        uint16_t end_index = gtmv(m, 16);
        uint16_t start_index = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool2_set( %u %u)\n",
            end_index, start_index);
        ag_err = ag_drv_fpm_token_recover_start_end_pool2_set(end_index, start_index);
        if (!ag_err)
            ag_err = ag_drv_fpm_token_recover_start_end_pool2_get(&end_index, &start_index);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool2_get( %u %u)\n",
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
        fpm_pool_multi pool_multi = {.token_multi = gtmv(m, 7), .update_type = gtmv(m, 1), .token_index = gtmv(m, 18), .ddr = gtmv(m, 1), .token_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool_multi_set( %u %u %u %u %u)\n",
            pool_multi.token_multi, pool_multi.update_type, pool_multi.token_index, pool_multi.ddr, 
            pool_multi.token_valid);
        ag_err = ag_drv_fpm_pool_multi_set(&pool_multi);
    }

    {
        uint16_t token_size = gtmv(m, 12);
        uint32_t token_index = gtmv(m, 18);
        bdmf_boolean ddr = gtmv(m, 1);
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_1_set( %u %u %u %u)\n",
            token_size, token_index, ddr, token_valid);
        ag_err = ag_drv_fpm_pool1_alloc_dealloc_1_set(token_size, token_index, ddr, token_valid);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool1_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_1_get( %u %u %u %u)\n",
                token_size, token_index, ddr, token_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 12) || token_index != gtmv(m, 18) || ddr != gtmv(m, 1) || token_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 12);
        uint32_t token_index = gtmv(m, 18);
        bdmf_boolean ddr = gtmv(m, 1);
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_1_set( %u %u %u %u)\n",
            token_size, token_index, ddr, token_valid);
        ag_err = ag_drv_fpm_pool2_alloc_dealloc_1_set(token_size, token_index, ddr, token_valid);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool2_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_1_get( %u %u %u %u)\n",
                token_size, token_index, ddr, token_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 12) || token_index != gtmv(m, 18) || ddr != gtmv(m, 1) || token_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 12);
        uint32_t token_index = gtmv(m, 18);
        bdmf_boolean ddr = gtmv(m, 1);
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_1_set( %u %u %u %u)\n",
            token_size, token_index, ddr, token_valid);
        ag_err = ag_drv_fpm_pool3_alloc_dealloc_1_set(token_size, token_index, ddr, token_valid);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool3_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_1_get( %u %u %u %u)\n",
                token_size, token_index, ddr, token_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 12) || token_index != gtmv(m, 18) || ddr != gtmv(m, 1) || token_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t token_size = gtmv(m, 12);
        uint32_t token_index = gtmv(m, 18);
        bdmf_boolean ddr = gtmv(m, 1);
        bdmf_boolean token_valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_1_set( %u %u %u %u)\n",
            token_size, token_index, ddr, token_valid);
        ag_err = ag_drv_fpm_pool4_alloc_dealloc_1_set(token_size, token_index, ddr, token_valid);
        if (!ag_err)
            ag_err = ag_drv_fpm_pool4_alloc_dealloc_1_get(&token_size, &token_index, &ddr, &token_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_1_get( %u %u %u %u)\n",
                token_size, token_index, ddr, token_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token_size != gtmv(m, 12) || token_index != gtmv(m, 18) || ddr != gtmv(m, 1) || token_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_pool_multi_1 pool_multi_1 = {.token_multi = gtmv(m, 7), .update_type = gtmv(m, 1), .token_index = gtmv(m, 18), .ddr = gtmv(m, 1), .token_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_pool_multi_1_set( %u %u %u %u %u)\n",
            pool_multi_1.token_multi, pool_multi_1.update_type, pool_multi_1.token_index, pool_multi_1.ddr, 
            pool_multi_1.token_valid);
        ag_err = ag_drv_fpm_pool_multi_1_set(&pool_multi_1);
    }

    {
        uint32_t search1 = gtmv(m, 8);
        fpm_search_data_1 search_data_1 = {.searchdata10_m = gtmv(m, 1), .searchdata11 = gtmv(m, 3), .searchdata12 = gtmv(m, 3), .searchdata13 = gtmv(m, 3), .searchdata14 = gtmv(m, 3), .searchdata15 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u %u %u %u %u %u)\n", search1,
            search_data_1.searchdata10_m, search_data_1.searchdata11, search_data_1.searchdata12, search_data_1.searchdata13, 
            search_data_1.searchdata14, search_data_1.searchdata15);
        ag_err = ag_drv_fpm_search_data_1_set(search1, &search_data_1);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_1_get(search1, &search_data_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_get( %u %u %u %u %u %u %u)\n", search1,
                search_data_1.searchdata10_m, search_data_1.searchdata11, search_data_1.searchdata12, search_data_1.searchdata13, 
                search_data_1.searchdata14, search_data_1.searchdata15);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (search_data_1.searchdata10_m != gtmv(m, 1) || search_data_1.searchdata11 != gtmv(m, 3) || search_data_1.searchdata12 != gtmv(m, 3) || search_data_1.searchdata13 != gtmv(m, 3) || search_data_1.searchdata14 != gtmv(m, 3) || search_data_1.searchdata15 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t search1 = gtmv(m, 8);
        fpm_search_data_2 search_data_2 = {.searchdata0 = gtmv(m, 3), .searchdata1 = gtmv(m, 3), .searchdata2 = gtmv(m, 3), .searchdata3 = gtmv(m, 3), .searchdata4 = gtmv(m, 3), .searchdata5 = gtmv(m, 3), .searchdata6 = gtmv(m, 3), .searchdata7 = gtmv(m, 3), .searchdata8 = gtmv(m, 3), .searchdata9 = gtmv(m, 3), .searchdata10_l = gtmv(m, 2)};
        bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u %u %u %u %u %u %u %u %u %u %u)\n", search1,
            search_data_2.searchdata0, search_data_2.searchdata1, search_data_2.searchdata2, search_data_2.searchdata3, 
            search_data_2.searchdata4, search_data_2.searchdata5, search_data_2.searchdata6, search_data_2.searchdata7, 
            search_data_2.searchdata8, search_data_2.searchdata9, search_data_2.searchdata10_l);
        ag_err = ag_drv_fpm_search_data_2_set(search1, &search_data_2);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_2_get(search1, &search_data_2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_get( %u %u %u %u %u %u %u %u %u %u %u %u)\n", search1,
                search_data_2.searchdata0, search_data_2.searchdata1, search_data_2.searchdata2, search_data_2.searchdata3, 
                search_data_2.searchdata4, search_data_2.searchdata5, search_data_2.searchdata6, search_data_2.searchdata7, 
                search_data_2.searchdata8, search_data_2.searchdata9, search_data_2.searchdata10_l);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (search_data_2.searchdata0 != gtmv(m, 3) || search_data_2.searchdata1 != gtmv(m, 3) || search_data_2.searchdata2 != gtmv(m, 3) || search_data_2.searchdata3 != gtmv(m, 3) || search_data_2.searchdata4 != gtmv(m, 3) || search_data_2.searchdata5 != gtmv(m, 3) || search_data_2.searchdata6 != gtmv(m, 3) || search_data_2.searchdata7 != gtmv(m, 3) || search_data_2.searchdata8 != gtmv(m, 3) || search_data_2.searchdata9 != gtmv(m, 3) || search_data_2.searchdata10_l != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t search3 = gtmv(m, 8);
        fpm_search_data_3 search_data_3 = {.searchdata10_m = gtmv(m, 1), .searchdata11 = gtmv(m, 3), .searchdata12 = gtmv(m, 3), .searchdata13 = gtmv(m, 3), .searchdata14 = gtmv(m, 3), .searchdata15 = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u %u %u %u %u %u)\n", search3,
            search_data_3.searchdata10_m, search_data_3.searchdata11, search_data_3.searchdata12, search_data_3.searchdata13, 
            search_data_3.searchdata14, search_data_3.searchdata15);
        ag_err = ag_drv_fpm_search_data_3_set(search3, &search_data_3);
        if (!ag_err)
            ag_err = ag_drv_fpm_search_data_3_get(search3, &search_data_3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_get( %u %u %u %u %u %u %u)\n", search3,
                search_data_3.searchdata10_m, search_data_3.searchdata11, search_data_3.searchdata12, search_data_3.searchdata13, 
                search_data_3.searchdata14, search_data_3.searchdata15);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (search_data_3.searchdata10_m != gtmv(m, 1) || search_data_3.searchdata11 != gtmv(m, 3) || search_data_3.searchdata12 != gtmv(m, 3) || search_data_3.searchdata13 != gtmv(m, 3) || search_data_3.searchdata14 != gtmv(m, 3) || search_data_3.searchdata15 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t multicast_data = gtmv(m, 13);
        uint32_t multicast = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
            multicast);
        ag_err = ag_drv_fpm_multicast_data_set(multicast_data, multicast);
        if (!ag_err)
            ag_err = ag_drv_fpm_multicast_data_get(multicast_data, &multicast);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_get( %u %u)\n", multicast_data,
                multicast);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (multicast != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t compute_pool_data = gtmv(m, 14);
        uint8_t poolid = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_fpm_compute_pool_data_get(compute_pool_data, &poolid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_compute_pool_data_get( %u %u)\n", compute_pool_data,
                poolid);
        }
    }

    {
        bdmf_boolean force = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_force_set( %u)\n",
            force);
        ag_err = ag_drv_fpm_fpm_bb_force_set(force);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_force_get(&force);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_force_get( %u)\n",
                force);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (force != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ctrl = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_ctrl_set( %u)\n",
            ctrl);
        ag_err = ag_drv_fpm_fpm_bb_forced_ctrl_set(ctrl);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_forced_ctrl_get(&ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_ctrl_get( %u)\n",
                ctrl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ctrl != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ta_addr = gtmv(m, 16);
        uint8_t dest_addr = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_addr_set( %u %u)\n",
            ta_addr, dest_addr);
        ag_err = ag_drv_fpm_fpm_bb_forced_addr_set(ta_addr, dest_addr);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_forced_addr_get(&ta_addr, &dest_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_addr_get( %u %u)\n",
                ta_addr, dest_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ta_addr != gtmv(m, 16) || dest_addr != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_data_set( %u)\n",
            data);
        ag_err = ag_drv_fpm_fpm_bb_forced_data_set(data);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_forced_data_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_data_get( %u)\n",
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
        uint8_t dest_id = gtmv(m, 6);
        bdmf_boolean override_en = gtmv(m, 1);
        uint16_t route_addr = gtmv(m, 10);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_decode_cfg_set( %u %u %u)\n",
            dest_id, override_en, route_addr);
        ag_err = ag_drv_fpm_fpm_bb_decode_cfg_set(dest_id, override_en, route_addr);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_decode_cfg_get(&dest_id, &override_en, &route_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_decode_cfg_get( %u %u %u)\n",
                dest_id, override_en, route_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dest_id != gtmv(m, 6) || override_en != gtmv(m, 1) || route_addr != gtmv(m, 10))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t rxfifo_sw_addr = gtmv(m, 4);
        uint8_t txfifo_sw_addr = gtmv(m, 4);
        bdmf_boolean rxfifo_sw_rst = gtmv(m, 1);
        bdmf_boolean txfifo_sw_rst = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_cfg_set( %u %u %u %u)\n",
            rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        ag_err = ag_drv_fpm_fpm_bb_dbg_cfg_set(rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_cfg_get(&rxfifo_sw_addr, &txfifo_sw_addr, &rxfifo_sw_rst, &txfifo_sw_rst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_cfg_get( %u %u %u %u)\n",
                rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rxfifo_sw_addr != gtmv(m, 4) || txfifo_sw_addr != gtmv(m, 4) || rxfifo_sw_rst != gtmv(m, 1) || txfifo_sw_rst != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_fpm_bb_dbg_rxfifo_sts fpm_bb_dbg_rxfifo_sts = {.fifo_empty = gtmv(m, 1), .fifo_full = gtmv(m, 1), .fifo_used_words = gtmv(m, 5), .fifo_rd_cntr = gtmv(m, 5), .fifo_wr_cntr = gtmv(m, 5)};
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(&fpm_bb_dbg_rxfifo_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get( %u %u %u %u %u)\n",
                fpm_bb_dbg_rxfifo_sts.fifo_empty, fpm_bb_dbg_rxfifo_sts.fifo_full, fpm_bb_dbg_rxfifo_sts.fifo_used_words, fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr, 
                fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr);
        }
    }

    {
        fpm_fpm_bb_dbg_txfifo_sts fpm_bb_dbg_txfifo_sts = {.fifo_empty = gtmv(m, 1), .fifo_full = gtmv(m, 1), .fifo_used_words = gtmv(m, 5), .fifo_rd_cntr = gtmv(m, 5), .fifo_wr_cntr = gtmv(m, 5)};
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(&fpm_bb_dbg_txfifo_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get( %u %u %u %u %u)\n",
                fpm_bb_dbg_txfifo_sts.fifo_empty, fpm_bb_dbg_txfifo_sts.fifo_full, fpm_bb_dbg_txfifo_sts.fifo_used_words, fpm_bb_dbg_txfifo_sts.fifo_rd_cntr, 
                fpm_bb_dbg_txfifo_sts.fifo_wr_cntr);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get( %u)\n",
                data);
        }
    }

    {
        fpm_fpm_bb_misc fpm_bb_misc = {.old_task_num = gtmv(m, 1), .alc_fre_arb_rr = gtmv(m, 1), .alc_fst_ack = gtmv(m, 1), .pool_0_size = gtmv(m, 2), .pool_1_size = gtmv(m, 2), .poolx_en = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_fpm_fpm_bb_misc_set( %u %u %u %u %u %u)\n",
            fpm_bb_misc.old_task_num, fpm_bb_misc.alc_fre_arb_rr, fpm_bb_misc.alc_fst_ack, fpm_bb_misc.pool_0_size, 
            fpm_bb_misc.pool_1_size, fpm_bb_misc.poolx_en);
        ag_err = ag_drv_fpm_fpm_bb_misc_set(&fpm_bb_misc);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpm_bb_misc_get(&fpm_bb_misc);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpm_bb_misc_get( %u %u %u %u %u %u)\n",
                fpm_bb_misc.old_task_num, fpm_bb_misc.alc_fre_arb_rr, fpm_bb_misc.alc_fst_ack, fpm_bb_misc.pool_0_size, 
                fpm_bb_misc.pool_1_size, fpm_bb_misc.poolx_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_bb_misc.old_task_num != gtmv(m, 1) || fpm_bb_misc.alc_fre_arb_rr != gtmv(m, 1) || fpm_bb_misc.alc_fst_ack != gtmv(m, 1) || fpm_bb_misc.pool_0_size != gtmv(m, 2) || fpm_bb_misc.pool_1_size != gtmv(m, 2) || fpm_bb_misc.poolx_en != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        fpm_clk_gate_cntrl clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_fpm_clk_gate_cntrl_set( %u %u %u %u %u)\n",
            clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.timer_val, clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_intrvl, 
            clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_fpm_clk_gate_cntrl_set(&clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_fpm_clk_gate_cntrl_get(&clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_clk_gate_cntrl_get( %u %u %u %u %u)\n",
                clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.timer_val, clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_intrvl, 
                clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || clk_gate_cntrl.timer_val != gtmv(m, 8) || clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
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
        uint32_t max_search1 = 273;
        uint32_t search1 = gtmv(m, 8);
        fpm_search_data_1 search_data_1 = {
            .searchdata10_m = gtmv(m, 1), 
            .searchdata11 = gtmv(m, 3), 
            .searchdata12 = gtmv(m, 3), 
            .searchdata13 = gtmv(m, 3), 
            .searchdata14 = gtmv(m, 3), 
            .searchdata15 = gtmv(m, 3) };

        if ((start_idx >= max_search1) || (stop_idx >= max_search1))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search1);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search1 = 0; search1 < max_search1; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u %u %u %u %u %u)\n", search1,
                search_data_1.searchdata10_m, search_data_1.searchdata11, search_data_1.searchdata12, search_data_1.searchdata13, 
                search_data_1.searchdata14, search_data_1.searchdata15);
            ag_err = ag_drv_fpm_search_data_1_set(search1, &search_data_1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        search_data_1.searchdata10_m = gtmv(m, 1);
        search_data_1.searchdata11 = gtmv(m, 3);
        search_data_1.searchdata12 = gtmv(m, 3);
        search_data_1.searchdata13 = gtmv(m, 3);
        search_data_1.searchdata14 = gtmv(m, 3);
        search_data_1.searchdata15 = gtmv(m, 3);

        for (search1 = start_idx; search1 <= stop_idx; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_1_set( %u %u %u %u %u %u %u)\n", search1,
                search_data_1.searchdata10_m, search_data_1.searchdata11, search_data_1.searchdata12, search_data_1.searchdata13, 
                search_data_1.searchdata14, search_data_1.searchdata15);
            ag_err = ag_drv_fpm_search_data_1_set(search1, &search_data_1);
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

            ag_err = ag_drv_fpm_search_data_1_get(search1, &search_data_1);

            bdmf_session_print(session, "ag_drv_fpm_search_data_1_get( %u %u %u %u %u %u %u)\n", search1,
                search_data_1.searchdata10_m, search_data_1.searchdata11, search_data_1.searchdata12, search_data_1.searchdata13, 
                search_data_1.searchdata14, search_data_1.searchdata15);

            if (search_data_1.searchdata10_m != gtmv(m, 1) || 
                search_data_1.searchdata11 != gtmv(m, 3) || 
                search_data_1.searchdata12 != gtmv(m, 3) || 
                search_data_1.searchdata13 != gtmv(m, 3) || 
                search_data_1.searchdata14 != gtmv(m, 3) || 
                search_data_1.searchdata15 != gtmv(m, 3) || 
                0)
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
        uint32_t max_search1 = 273;
        uint32_t search1 = gtmv(m, 8);
        fpm_search_data_2 search_data_2 = {
            .searchdata0 = gtmv(m, 3), 
            .searchdata1 = gtmv(m, 3), 
            .searchdata2 = gtmv(m, 3), 
            .searchdata3 = gtmv(m, 3), 
            .searchdata4 = gtmv(m, 3), 
            .searchdata5 = gtmv(m, 3), 
            .searchdata6 = gtmv(m, 3), 
            .searchdata7 = gtmv(m, 3), 
            .searchdata8 = gtmv(m, 3), 
            .searchdata9 = gtmv(m, 3), 
            .searchdata10_l = gtmv(m, 2) };

        if ((start_idx >= max_search1) || (stop_idx >= max_search1))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search1);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search1 = 0; search1 < max_search1; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u %u %u %u %u %u %u %u %u %u %u)\n", search1,
                search_data_2.searchdata0, search_data_2.searchdata1, search_data_2.searchdata2, search_data_2.searchdata3, 
                search_data_2.searchdata4, search_data_2.searchdata5, search_data_2.searchdata6, search_data_2.searchdata7, 
                search_data_2.searchdata8, search_data_2.searchdata9, search_data_2.searchdata10_l);
            ag_err = ag_drv_fpm_search_data_2_set(search1, &search_data_2);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search1);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        search_data_2.searchdata0 = gtmv(m, 3);
        search_data_2.searchdata1 = gtmv(m, 3);
        search_data_2.searchdata2 = gtmv(m, 3);
        search_data_2.searchdata3 = gtmv(m, 3);
        search_data_2.searchdata4 = gtmv(m, 3);
        search_data_2.searchdata5 = gtmv(m, 3);
        search_data_2.searchdata6 = gtmv(m, 3);
        search_data_2.searchdata7 = gtmv(m, 3);
        search_data_2.searchdata8 = gtmv(m, 3);
        search_data_2.searchdata9 = gtmv(m, 3);
        search_data_2.searchdata10_l = gtmv(m, 2);

        for (search1 = start_idx; search1 <= stop_idx; search1++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_2_set( %u %u %u %u %u %u %u %u %u %u %u %u)\n", search1,
                search_data_2.searchdata0, search_data_2.searchdata1, search_data_2.searchdata2, search_data_2.searchdata3, 
                search_data_2.searchdata4, search_data_2.searchdata5, search_data_2.searchdata6, search_data_2.searchdata7, 
                search_data_2.searchdata8, search_data_2.searchdata9, search_data_2.searchdata10_l);
            ag_err = ag_drv_fpm_search_data_2_set(search1, &search_data_2);
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

            ag_err = ag_drv_fpm_search_data_2_get(search1, &search_data_2);

            bdmf_session_print(session, "ag_drv_fpm_search_data_2_get( %u %u %u %u %u %u %u %u %u %u %u %u)\n", search1,
                search_data_2.searchdata0, search_data_2.searchdata1, search_data_2.searchdata2, search_data_2.searchdata3, 
                search_data_2.searchdata4, search_data_2.searchdata5, search_data_2.searchdata6, search_data_2.searchdata7, 
                search_data_2.searchdata8, search_data_2.searchdata9, search_data_2.searchdata10_l);

            if (search_data_2.searchdata0 != gtmv(m, 3) || 
                search_data_2.searchdata1 != gtmv(m, 3) || 
                search_data_2.searchdata2 != gtmv(m, 3) || 
                search_data_2.searchdata3 != gtmv(m, 3) || 
                search_data_2.searchdata4 != gtmv(m, 3) || 
                search_data_2.searchdata5 != gtmv(m, 3) || 
                search_data_2.searchdata6 != gtmv(m, 3) || 
                search_data_2.searchdata7 != gtmv(m, 3) || 
                search_data_2.searchdata8 != gtmv(m, 3) || 
                search_data_2.searchdata9 != gtmv(m, 3) || 
                search_data_2.searchdata10_l != gtmv(m, 2) || 
                0)
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
        uint32_t max_search3 = 273;
        uint32_t search3 = gtmv(m, 8);
        fpm_search_data_3 search_data_3 = {
            .searchdata10_m = gtmv(m, 1), 
            .searchdata11 = gtmv(m, 3), 
            .searchdata12 = gtmv(m, 3), 
            .searchdata13 = gtmv(m, 3), 
            .searchdata14 = gtmv(m, 3), 
            .searchdata15 = gtmv(m, 3) };

        if ((start_idx >= max_search3) || (stop_idx >= max_search3))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_search3);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (search3 = 0; search3 < max_search3; search3++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u %u %u %u %u %u)\n", search3,
                search_data_3.searchdata10_m, search_data_3.searchdata11, search_data_3.searchdata12, search_data_3.searchdata13, 
                search_data_3.searchdata14, search_data_3.searchdata15);
            ag_err = ag_drv_fpm_search_data_3_set(search3, &search_data_3);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", search3);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        search_data_3.searchdata10_m = gtmv(m, 1);
        search_data_3.searchdata11 = gtmv(m, 3);
        search_data_3.searchdata12 = gtmv(m, 3);
        search_data_3.searchdata13 = gtmv(m, 3);
        search_data_3.searchdata14 = gtmv(m, 3);
        search_data_3.searchdata15 = gtmv(m, 3);

        for (search3 = start_idx; search3 <= stop_idx; search3++)
        {
            bdmf_session_print(session, "ag_drv_fpm_search_data_3_set( %u %u %u %u %u %u %u)\n", search3,
                search_data_3.searchdata10_m, search_data_3.searchdata11, search_data_3.searchdata12, search_data_3.searchdata13, 
                search_data_3.searchdata14, search_data_3.searchdata15);
            ag_err = ag_drv_fpm_search_data_3_set(search3, &search_data_3);
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

            ag_err = ag_drv_fpm_search_data_3_get(search3, &search_data_3);

            bdmf_session_print(session, "ag_drv_fpm_search_data_3_get( %u %u %u %u %u %u %u)\n", search3,
                search_data_3.searchdata10_m, search_data_3.searchdata11, search_data_3.searchdata12, search_data_3.searchdata13, 
                search_data_3.searchdata14, search_data_3.searchdata15);

            if (search_data_3.searchdata10_m != gtmv(m, 1) || 
                search_data_3.searchdata11 != gtmv(m, 3) || 
                search_data_3.searchdata12 != gtmv(m, 3) || 
                search_data_3.searchdata13 != gtmv(m, 3) || 
                search_data_3.searchdata14 != gtmv(m, 3) || 
                search_data_3.searchdata15 != gtmv(m, 3) || 
                0)
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
        uint32_t max_multicast_data = 8192;
        uint32_t multicast_data = gtmv(m, 13);
        uint32_t multicast = gtmv(m, 32);

        if ((start_idx >= max_multicast_data) || (stop_idx >= max_multicast_data))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_multicast_data);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (multicast_data = 0; multicast_data < max_multicast_data; multicast_data++)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
                multicast);
            ag_err = ag_drv_fpm_multicast_data_set(multicast_data, multicast);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", multicast_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        multicast = gtmv(m, 32);

        for (multicast_data = start_idx; multicast_data <= stop_idx; multicast_data++)
        {
            bdmf_session_print(session, "ag_drv_fpm_multicast_data_set( %u %u)\n", multicast_data,
                multicast);
            ag_err = ag_drv_fpm_multicast_data_set(multicast_data, multicast);
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

            ag_err = ag_drv_fpm_multicast_data_get(multicast_data, &multicast);

            bdmf_session_print(session, "ag_drv_fpm_multicast_data_get( %u %u)\n", multicast_data,
                multicast);

            if (multicast != gtmv(m, 32) || 0)
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
    case bdmf_address_fpm_ctl: reg = &RU_REG(FPM, FPM_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_cfg1: reg = &RU_REG(FPM, FPM_CFG1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_weight: reg = &RU_REG(FPM, FPM_WEIGHT); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_cfg: reg = &RU_REG(FPM, FPM_BB_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_intr_msk: reg = &RU_REG(FPM, POOL1_INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_intr_sts: reg = &RU_REG(FPM, POOL1_INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stall_msk: reg = &RU_REG(FPM, POOL1_STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_intr_msk: reg = &RU_REG(FPM, POOL2_INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_intr_sts: reg = &RU_REG(FPM, POOL2_INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stall_msk: reg = &RU_REG(FPM, POOL2_STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_intr_msk: reg = &RU_REG(FPM, POOL3_INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_intr_sts: reg = &RU_REG(FPM, POOL3_INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stall_msk: reg = &RU_REG(FPM, POOL3_STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg1: reg = &RU_REG(FPM, POOL1_CFG1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg2: reg = &RU_REG(FPM, POOL1_CFG2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg3: reg = &RU_REG(FPM, POOL1_CFG3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg4: reg = &RU_REG(FPM, POOL1_CFG4); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat1: reg = &RU_REG(FPM, POOL1_STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat2: reg = &RU_REG(FPM, POOL1_STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat3: reg = &RU_REG(FPM, POOL1_STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat4: reg = &RU_REG(FPM, POOL1_STAT4); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat5: reg = &RU_REG(FPM, POOL1_STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat6: reg = &RU_REG(FPM, POOL1_STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat7: reg = &RU_REG(FPM, POOL1_STAT7); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat8: reg = &RU_REG(FPM, POOL1_STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat1: reg = &RU_REG(FPM, POOL2_STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat2: reg = &RU_REG(FPM, POOL2_STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat3: reg = &RU_REG(FPM, POOL2_STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat4: reg = &RU_REG(FPM, POOL2_STAT4); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat5: reg = &RU_REG(FPM, POOL2_STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat6: reg = &RU_REG(FPM, POOL2_STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat7: reg = &RU_REG(FPM, POOL2_STAT7); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat8: reg = &RU_REG(FPM, POOL2_STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat1: reg = &RU_REG(FPM, POOL3_STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat2: reg = &RU_REG(FPM, POOL3_STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat3: reg = &RU_REG(FPM, POOL3_STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat5: reg = &RU_REG(FPM, POOL3_STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat6: reg = &RU_REG(FPM, POOL3_STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_stat8: reg = &RU_REG(FPM, POOL3_STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_xon_xoff_cfg: reg = &RU_REG(FPM, POOL1_XON_XOFF_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_not_empty_cfg: reg = &RU_REG(FPM, FPM_NOT_EMPTY_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_ctl: reg = &RU_REG(FPM, MEM_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_data1: reg = &RU_REG(FPM, MEM_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_data2: reg = &RU_REG(FPM, MEM_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_spare: reg = &RU_REG(FPM, SPARE); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_ctl: reg = &RU_REG(FPM, TOKEN_RECOVER_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_short_aging_timer: reg = &RU_REG(FPM, SHORT_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_long_aging_timer: reg = &RU_REG(FPM, LONG_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_cache_recycle_timer: reg = &RU_REG(FPM, CACHE_RECYCLE_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_expired_token_count_pool1: reg = &RU_REG(FPM, EXPIRED_TOKEN_COUNT_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_recovered_token_count_pool1: reg = &RU_REG(FPM, RECOVERED_TOKEN_COUNT_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_expired_token_count_pool2: reg = &RU_REG(FPM, EXPIRED_TOKEN_COUNT_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_recovered_token_count_pool2: reg = &RU_REG(FPM, RECOVERED_TOKEN_COUNT_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_start_end_pool1: reg = &RU_REG(FPM, TOKEN_RECOVER_START_END_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_start_end_pool2: reg = &RU_REG(FPM, TOKEN_RECOVER_START_END_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_prbs_invalid_gen: reg = &RU_REG(FPM, PRBS_INVALID_GEN); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_alloc_dealloc: reg = &RU_REG(FPM, POOL1_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_alloc_dealloc: reg = &RU_REG(FPM, POOL2_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_alloc_dealloc: reg = &RU_REG(FPM, POOL3_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool4_alloc_dealloc: reg = &RU_REG(FPM, POOL4_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool_multi: reg = &RU_REG(FPM, POOL_MULTI); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_alloc_dealloc_1: reg = &RU_REG(FPM, POOL1_ALLOC_DEALLOC_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_alloc_dealloc_1: reg = &RU_REG(FPM, POOL2_ALLOC_DEALLOC_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_alloc_dealloc_1: reg = &RU_REG(FPM, POOL3_ALLOC_DEALLOC_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool4_alloc_dealloc_1: reg = &RU_REG(FPM, POOL4_ALLOC_DEALLOC_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool_multi_1: reg = &RU_REG(FPM, POOL_MULTI_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_0: reg = &RU_REG(FPM, SEARCH_DATA_0); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_1: reg = &RU_REG(FPM, SEARCH_DATA_1); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_2: reg = &RU_REG(FPM, SEARCH_DATA_2); blk = &RU_BLK(FPM); break;
    case bdmf_address_search_data_3: reg = &RU_REG(FPM, SEARCH_DATA_3); blk = &RU_BLK(FPM); break;
    case bdmf_address_multicast_data: reg = &RU_REG(FPM, MULTICAST_DATA); blk = &RU_BLK(FPM); break;
    case bdmf_address_compute_pool_data: reg = &RU_REG(FPM, COMPUTE_POOL_DATA); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_force: reg = &RU_REG(FPM, FPM_BB_FORCE); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_ctrl: reg = &RU_REG(FPM, FPM_BB_FORCED_CTRL); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_addr: reg = &RU_REG(FPM, FPM_BB_FORCED_ADDR); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_data: reg = &RU_REG(FPM, FPM_BB_FORCED_DATA); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_decode_cfg: reg = &RU_REG(FPM, FPM_BB_DECODE_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_cfg: reg = &RU_REG(FPM, FPM_BB_DBG_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_sts: reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_sts: reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_data1: reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_data2: reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data1: reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data2: reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data3: reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA3); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_misc: reg = &RU_REG(FPM, FPM_BB_MISC); blk = &RU_BLK(FPM); break;
    case bdmf_address_clk_gate_cntrl: reg = &RU_REG(FPM, CLK_GATE_CNTRL); blk = &RU_BLK(FPM); break;
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
        static bdmfmon_cmd_parm_t set_pool1_en[] = {
            BDMFMON_MAKE_PARM("pool1_enable", "pool1_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_reset[] = {
            BDMFMON_MAKE_PARM("fpm_bb_soft_reset", "fpm_bb_soft_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr0_weight[] = {
            BDMFMON_MAKE_PARM("ddr0_alloc_weight", "ddr0_alloc_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr0_free_weight", "ddr0_free_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr1_weight[] = {
            BDMFMON_MAKE_PARM("ddr1_alloc_weight", "ddr1_alloc_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr1_free_weight", "ddr1_free_weight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_cfg[] = {
            BDMFMON_MAKE_PARM("fp_buf_size", "fp_buf_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_base_address", "pool_base_address", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_base_address_pool2", "pool_base_address_pool2", BDMFMON_PARM_UNUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_pool4_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_alloc_dealloc[] = {
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_back_door_mem[] = {
            BDMFMON_MAKE_PARM("mem_data1", "mem_data1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_data2", "mem_data2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_timer[] = {
            BDMFMON_MAKE_PARM("long_aging_timer", "long_aging_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("short_aging_timer", "short_aging_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("recycle_timer", "recycle_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_cfg1[] = {
            BDMFMON_MAKE_PARM("pool1_search_mode", "pool1_search_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_jumbo_support", "enable_jumbo_support", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_cfg[] = {
            BDMFMON_MAKE_PARM("bb_ddr_sel", "bb_ddr_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_intr_msk[] = {
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
        static bdmfmon_cmd_parm_t set_pool2_intr_sts[] = {
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
        static bdmfmon_cmd_parm_t set_pool2_stall_msk[] = {
            BDMFMON_MAKE_PARM("free_token_no_valid_stall_msk", "free_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_stall_msk", "free_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_stall_msk", "multi_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_stall_msk", "multi_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_stall_msk", "memory_corrupt_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_intr_msk[] = {
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
        static bdmfmon_cmd_parm_t set_pool3_intr_sts[] = {
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
        static bdmfmon_cmd_parm_t set_pool3_stall_msk[] = {
            BDMFMON_MAKE_PARM("free_token_no_valid_stall_msk", "free_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_stall_msk", "free_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_stall_msk", "multi_token_no_valid_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_stall_msk", "multi_token_index_out_of_range_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_stall_msk", "memory_corrupt_stall_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_cfg4[] = {
            BDMFMON_MAKE_PARM("pool_base_address_pool3", "pool_base_address_pool3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_stat5[] = {
            BDMFMON_MAKE_PARM("mem_corrupt_sts_related_alloc_token", "mem_corrupt_sts_related_alloc_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat1[] = {
            BDMFMON_MAKE_PARM("undrfl", "undrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovrfl", "ovrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat3[] = {
            BDMFMON_MAKE_PARM("num_of_not_valid_token_frees", "num_of_not_valid_token_frees", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat4[] = {
            BDMFMON_MAKE_PARM("num_of_not_valid_token_multi", "num_of_not_valid_token_multi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat5[] = {
            BDMFMON_MAKE_PARM("mem_corrupt_sts_related_alloc_token", "mem_corrupt_sts_related_alloc_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat6[] = {
            BDMFMON_MAKE_PARM("invalid_free_token", "invalid_free_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat7[] = {
            BDMFMON_MAKE_PARM("invalid_mcast_token", "invalid_mcast_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stat8[] = {
            BDMFMON_MAKE_PARM("tokens_available_low_wtmk", "tokens_available_low_wtmk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_stat1[] = {
            BDMFMON_MAKE_PARM("undrfl", "undrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovrfl", "ovrfl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_stat3[] = {
            BDMFMON_MAKE_PARM("num_of_not_valid_token_frees", "num_of_not_valid_token_frees", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_stat5[] = {
            BDMFMON_MAKE_PARM("mem_corrupt_sts_related_alloc_token", "mem_corrupt_sts_related_alloc_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_stat6[] = {
            BDMFMON_MAKE_PARM("invalid_free_token", "invalid_free_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_stat8[] = {
            BDMFMON_MAKE_PARM("tokens_available_low_wtmk", "tokens_available_low_wtmk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_xon_xoff_cfg[] = {
            BDMFMON_MAKE_PARM("xoff_threshold", "xoff_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xon_threshold", "xon_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_not_empty_cfg[] = {
            BDMFMON_MAKE_PARM("not_empty_threshold", "not_empty_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mem_ctl[] = {
            BDMFMON_MAKE_PARM("mem_addr", "mem_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_sel", "mem_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_rd", "mem_rd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_wr", "mem_wr", BDMFMON_PARM_UNUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_token_recover_start_end_pool1[] = {
            BDMFMON_MAKE_PARM("end_index", "end_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_index", "start_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_start_end_pool2[] = {
            BDMFMON_MAKE_PARM("end_index", "end_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_index", "start_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_prbs_invalid_gen[] = {
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_multi[] = {
            BDMFMON_MAKE_PARM("token_multi", "token_multi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("update_type", "update_type", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_alloc_dealloc_1[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_alloc_dealloc_1[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_alloc_dealloc_1[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool4_alloc_dealloc_1[] = {
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_multi_1[] = {
            BDMFMON_MAKE_PARM("token_multi", "token_multi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("update_type", "update_type", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_1[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata10_m", "searchdata10_m", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata11", "searchdata11", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata12", "searchdata12", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata13", "searchdata13", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata14", "searchdata14", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata15", "searchdata15", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_2[] = {
            BDMFMON_MAKE_PARM("search1", "search1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata0", "searchdata0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata1", "searchdata1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata2", "searchdata2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata3", "searchdata3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata4", "searchdata4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata5", "searchdata5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata6", "searchdata6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata7", "searchdata7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata8", "searchdata8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata9", "searchdata9", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata10_l", "searchdata10_l", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_search_data_3[] = {
            BDMFMON_MAKE_PARM("search3", "search3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata10_m", "searchdata10_m", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata11", "searchdata11", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata12", "searchdata12", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata13", "searchdata13", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata14", "searchdata14", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("searchdata15", "searchdata15", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_multicast_data[] = {
            BDMFMON_MAKE_PARM("multicast_data", "multicast_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multicast", "multicast", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_force[] = {
            BDMFMON_MAKE_PARM("force", "force", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_ctrl[] = {
            BDMFMON_MAKE_PARM("ctrl", "ctrl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_addr[] = {
            BDMFMON_MAKE_PARM("ta_addr", "ta_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest_addr", "dest_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_data[] = {
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_decode_cfg[] = {
            BDMFMON_MAKE_PARM("dest_id", "dest_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("override_en", "override_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr", "route_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_dbg_cfg[] = {
            BDMFMON_MAKE_PARM("rxfifo_sw_addr", "rxfifo_sw_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("txfifo_sw_addr", "txfifo_sw_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rxfifo_sw_rst", "rxfifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("txfifo_sw_rst", "txfifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_misc[] = {
            BDMFMON_MAKE_PARM("old_task_num", "old_task_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alc_fre_arb_rr", "alc_fre_arb_rr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("alc_fst_ack", "alc_fst_ack", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_0_size", "pool_0_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_1_size", "pool_1_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("poolx_en", "poolx_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "init_mem", .val = cli_fpm_init_mem, .parms = set_init_mem },
            { .name = "pool1_en", .val = cli_fpm_pool1_en, .parms = set_pool1_en },
            { .name = "bb_reset", .val = cli_fpm_bb_reset, .parms = set_bb_reset },
            { .name = "ddr0_weight", .val = cli_fpm_ddr0_weight, .parms = set_ddr0_weight },
            { .name = "ddr1_weight", .val = cli_fpm_ddr1_weight, .parms = set_ddr1_weight },
            { .name = "pool_cfg", .val = cli_fpm_pool_cfg, .parms = set_pool_cfg },
            { .name = "pool_stat", .val = cli_fpm_pool_stat, .parms = set_pool_stat },
            { .name = "pool4_alloc_dealloc", .val = cli_fpm_pool4_alloc_dealloc, .parms = set_pool4_alloc_dealloc },
            { .name = "pool3_alloc_dealloc", .val = cli_fpm_pool3_alloc_dealloc, .parms = set_pool3_alloc_dealloc },
            { .name = "pool2_alloc_dealloc", .val = cli_fpm_pool2_alloc_dealloc, .parms = set_pool2_alloc_dealloc },
            { .name = "pool1_alloc_dealloc", .val = cli_fpm_pool1_alloc_dealloc, .parms = set_pool1_alloc_dealloc },
            { .name = "back_door_mem", .val = cli_fpm_back_door_mem, .parms = set_back_door_mem },
            { .name = "timer", .val = cli_fpm_timer, .parms = set_timer },
            { .name = "fpm_cfg1", .val = cli_fpm_fpm_cfg1, .parms = set_fpm_cfg1 },
            { .name = "fpm_bb_cfg", .val = cli_fpm_fpm_bb_cfg, .parms = set_fpm_bb_cfg },
            { .name = "pool1_intr_msk", .val = cli_fpm_pool1_intr_msk, .parms = set_pool2_intr_msk },
            { .name = "pool1_intr_sts", .val = cli_fpm_pool1_intr_sts, .parms = set_pool2_intr_sts },
            { .name = "pool1_stall_msk", .val = cli_fpm_pool1_stall_msk, .parms = set_pool2_stall_msk },
            { .name = "pool2_intr_msk", .val = cli_fpm_pool2_intr_msk, .parms = set_pool2_intr_msk },
            { .name = "pool2_intr_sts", .val = cli_fpm_pool2_intr_sts, .parms = set_pool2_intr_sts },
            { .name = "pool2_stall_msk", .val = cli_fpm_pool2_stall_msk, .parms = set_pool2_stall_msk },
            { .name = "pool3_intr_msk", .val = cli_fpm_pool3_intr_msk, .parms = set_pool3_intr_msk },
            { .name = "pool3_intr_sts", .val = cli_fpm_pool3_intr_sts, .parms = set_pool3_intr_sts },
            { .name = "pool3_stall_msk", .val = cli_fpm_pool3_stall_msk, .parms = set_pool3_stall_msk },
            { .name = "pool1_cfg4", .val = cli_fpm_pool1_cfg4, .parms = set_pool1_cfg4 },
            { .name = "pool1_stat5", .val = cli_fpm_pool1_stat5, .parms = set_pool1_stat5 },
            { .name = "pool2_stat1", .val = cli_fpm_pool2_stat1, .parms = set_pool2_stat1 },
            { .name = "pool2_stat3", .val = cli_fpm_pool2_stat3, .parms = set_pool2_stat3 },
            { .name = "pool2_stat4", .val = cli_fpm_pool2_stat4, .parms = set_pool2_stat4 },
            { .name = "pool2_stat5", .val = cli_fpm_pool2_stat5, .parms = set_pool2_stat5 },
            { .name = "pool2_stat6", .val = cli_fpm_pool2_stat6, .parms = set_pool2_stat6 },
            { .name = "pool2_stat7", .val = cli_fpm_pool2_stat7, .parms = set_pool2_stat7 },
            { .name = "pool2_stat8", .val = cli_fpm_pool2_stat8, .parms = set_pool2_stat8 },
            { .name = "pool3_stat1", .val = cli_fpm_pool3_stat1, .parms = set_pool3_stat1 },
            { .name = "pool3_stat3", .val = cli_fpm_pool3_stat3, .parms = set_pool3_stat3 },
            { .name = "pool3_stat5", .val = cli_fpm_pool3_stat5, .parms = set_pool3_stat5 },
            { .name = "pool3_stat6", .val = cli_fpm_pool3_stat6, .parms = set_pool3_stat6 },
            { .name = "pool3_stat8", .val = cli_fpm_pool3_stat8, .parms = set_pool3_stat8 },
            { .name = "pool1_xon_xoff_cfg", .val = cli_fpm_pool1_xon_xoff_cfg, .parms = set_pool1_xon_xoff_cfg },
            { .name = "fpm_not_empty_cfg", .val = cli_fpm_fpm_not_empty_cfg, .parms = set_fpm_not_empty_cfg },
            { .name = "mem_ctl", .val = cli_fpm_mem_ctl, .parms = set_mem_ctl },
            { .name = "token_recover_ctl", .val = cli_fpm_token_recover_ctl, .parms = set_token_recover_ctl },
            { .name = "token_recover_start_end_pool1", .val = cli_fpm_token_recover_start_end_pool1, .parms = set_token_recover_start_end_pool1 },
            { .name = "token_recover_start_end_pool2", .val = cli_fpm_token_recover_start_end_pool2, .parms = set_token_recover_start_end_pool2 },
            { .name = "prbs_invalid_gen", .val = cli_fpm_prbs_invalid_gen, .parms = set_prbs_invalid_gen },
            { .name = "pool_multi", .val = cli_fpm_pool_multi, .parms = set_pool_multi },
            { .name = "pool1_alloc_dealloc_1", .val = cli_fpm_pool1_alloc_dealloc_1, .parms = set_pool1_alloc_dealloc_1 },
            { .name = "pool2_alloc_dealloc_1", .val = cli_fpm_pool2_alloc_dealloc_1, .parms = set_pool2_alloc_dealloc_1 },
            { .name = "pool3_alloc_dealloc_1", .val = cli_fpm_pool3_alloc_dealloc_1, .parms = set_pool3_alloc_dealloc_1 },
            { .name = "pool4_alloc_dealloc_1", .val = cli_fpm_pool4_alloc_dealloc_1, .parms = set_pool4_alloc_dealloc_1 },
            { .name = "pool_multi_1", .val = cli_fpm_pool_multi_1, .parms = set_pool_multi_1 },
            { .name = "search_data_1", .val = cli_fpm_search_data_1, .parms = set_search_data_1 },
            { .name = "search_data_2", .val = cli_fpm_search_data_2, .parms = set_search_data_2 },
            { .name = "search_data_3", .val = cli_fpm_search_data_3, .parms = set_search_data_3 },
            { .name = "multicast_data", .val = cli_fpm_multicast_data, .parms = set_multicast_data },
            { .name = "fpm_bb_force", .val = cli_fpm_fpm_bb_force, .parms = set_fpm_bb_force },
            { .name = "fpm_bb_forced_ctrl", .val = cli_fpm_fpm_bb_forced_ctrl, .parms = set_fpm_bb_forced_ctrl },
            { .name = "fpm_bb_forced_addr", .val = cli_fpm_fpm_bb_forced_addr, .parms = set_fpm_bb_forced_addr },
            { .name = "fpm_bb_forced_data", .val = cli_fpm_fpm_bb_forced_data, .parms = set_fpm_bb_forced_data },
            { .name = "fpm_bb_decode_cfg", .val = cli_fpm_fpm_bb_decode_cfg, .parms = set_fpm_bb_decode_cfg },
            { .name = "fpm_bb_dbg_cfg", .val = cli_fpm_fpm_bb_dbg_cfg, .parms = set_fpm_bb_dbg_cfg },
            { .name = "fpm_bb_misc", .val = cli_fpm_fpm_bb_misc, .parms = set_fpm_bb_misc },
            { .name = "clk_gate_cntrl", .val = cli_fpm_clk_gate_cntrl, .parms = set_clk_gate_cntrl },
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
        static bdmfmon_cmd_parm_t get_compute_pool_data[] = {
            BDMFMON_MAKE_PARM("compute_pool_data", "compute_pool_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "init_mem", .val = cli_fpm_init_mem, .parms = get_default },
            { .name = "pool1_en", .val = cli_fpm_pool1_en, .parms = get_default },
            { .name = "bb_reset", .val = cli_fpm_bb_reset, .parms = get_default },
            { .name = "ddr0_weight", .val = cli_fpm_ddr0_weight, .parms = get_default },
            { .name = "ddr1_weight", .val = cli_fpm_ddr1_weight, .parms = get_default },
            { .name = "pool_cfg", .val = cli_fpm_pool_cfg, .parms = get_default },
            { .name = "pool_stat", .val = cli_fpm_pool_stat, .parms = get_default },
            { .name = "pool4_alloc_dealloc", .val = cli_fpm_pool4_alloc_dealloc, .parms = get_default },
            { .name = "pool3_alloc_dealloc", .val = cli_fpm_pool3_alloc_dealloc, .parms = get_default },
            { .name = "pool2_alloc_dealloc", .val = cli_fpm_pool2_alloc_dealloc, .parms = get_default },
            { .name = "pool1_alloc_dealloc", .val = cli_fpm_pool1_alloc_dealloc, .parms = get_default },
            { .name = "back_door_mem", .val = cli_fpm_back_door_mem, .parms = get_default },
            { .name = "pool1_count", .val = cli_fpm_pool1_count, .parms = get_default },
            { .name = "pool2_count", .val = cli_fpm_pool2_count, .parms = get_default },
            { .name = "timer", .val = cli_fpm_timer, .parms = get_default },
            { .name = "fpm_cfg1", .val = cli_fpm_fpm_cfg1, .parms = get_default },
            { .name = "fpm_bb_cfg", .val = cli_fpm_fpm_bb_cfg, .parms = get_default },
            { .name = "pool1_intr_msk", .val = cli_fpm_pool1_intr_msk, .parms = get_default },
            { .name = "pool1_intr_sts", .val = cli_fpm_pool1_intr_sts, .parms = get_default },
            { .name = "pool1_stall_msk", .val = cli_fpm_pool1_stall_msk, .parms = get_default },
            { .name = "pool2_intr_msk", .val = cli_fpm_pool2_intr_msk, .parms = get_default },
            { .name = "pool2_intr_sts", .val = cli_fpm_pool2_intr_sts, .parms = get_default },
            { .name = "pool2_stall_msk", .val = cli_fpm_pool2_stall_msk, .parms = get_default },
            { .name = "pool3_intr_msk", .val = cli_fpm_pool3_intr_msk, .parms = get_default },
            { .name = "pool3_intr_sts", .val = cli_fpm_pool3_intr_sts, .parms = get_default },
            { .name = "pool3_stall_msk", .val = cli_fpm_pool3_stall_msk, .parms = get_default },
            { .name = "pool1_cfg4", .val = cli_fpm_pool1_cfg4, .parms = get_default },
            { .name = "pool1_stat5", .val = cli_fpm_pool1_stat5, .parms = get_default },
            { .name = "pool2_stat1", .val = cli_fpm_pool2_stat1, .parms = get_default },
            { .name = "pool2_stat2", .val = cli_fpm_pool2_stat2, .parms = get_default },
            { .name = "pool2_stat3", .val = cli_fpm_pool2_stat3, .parms = get_default },
            { .name = "pool2_stat4", .val = cli_fpm_pool2_stat4, .parms = get_default },
            { .name = "pool2_stat5", .val = cli_fpm_pool2_stat5, .parms = get_default },
            { .name = "pool2_stat6", .val = cli_fpm_pool2_stat6, .parms = get_default },
            { .name = "pool2_stat7", .val = cli_fpm_pool2_stat7, .parms = get_default },
            { .name = "pool2_stat8", .val = cli_fpm_pool2_stat8, .parms = get_default },
            { .name = "pool3_stat1", .val = cli_fpm_pool3_stat1, .parms = get_default },
            { .name = "pool3_stat2", .val = cli_fpm_pool3_stat2, .parms = get_default },
            { .name = "pool3_stat3", .val = cli_fpm_pool3_stat3, .parms = get_default },
            { .name = "pool3_stat5", .val = cli_fpm_pool3_stat5, .parms = get_default },
            { .name = "pool3_stat6", .val = cli_fpm_pool3_stat6, .parms = get_default },
            { .name = "pool3_stat8", .val = cli_fpm_pool3_stat8, .parms = get_default },
            { .name = "pool1_xon_xoff_cfg", .val = cli_fpm_pool1_xon_xoff_cfg, .parms = get_default },
            { .name = "fpm_not_empty_cfg", .val = cli_fpm_fpm_not_empty_cfg, .parms = get_default },
            { .name = "mem_ctl", .val = cli_fpm_mem_ctl, .parms = get_default },
            { .name = "token_recover_ctl", .val = cli_fpm_token_recover_ctl, .parms = get_default },
            { .name = "token_recover_start_end_pool1", .val = cli_fpm_token_recover_start_end_pool1, .parms = get_default },
            { .name = "token_recover_start_end_pool2", .val = cli_fpm_token_recover_start_end_pool2, .parms = get_default },
            { .name = "prbs_invalid_gen", .val = cli_fpm_prbs_invalid_gen, .parms = get_default },
            { .name = "pool1_alloc_dealloc_1", .val = cli_fpm_pool1_alloc_dealloc_1, .parms = get_default },
            { .name = "pool2_alloc_dealloc_1", .val = cli_fpm_pool2_alloc_dealloc_1, .parms = get_default },
            { .name = "pool3_alloc_dealloc_1", .val = cli_fpm_pool3_alloc_dealloc_1, .parms = get_default },
            { .name = "pool4_alloc_dealloc_1", .val = cli_fpm_pool4_alloc_dealloc_1, .parms = get_default },
            { .name = "search_data_1", .val = cli_fpm_search_data_1, .parms = get_search_data_1 },
            { .name = "search_data_2", .val = cli_fpm_search_data_2, .parms = get_search_data_2 },
            { .name = "search_data_3", .val = cli_fpm_search_data_3, .parms = get_search_data_3 },
            { .name = "multicast_data", .val = cli_fpm_multicast_data, .parms = get_multicast_data },
            { .name = "compute_pool_data", .val = cli_fpm_compute_pool_data, .parms = get_compute_pool_data },
            { .name = "fpm_bb_force", .val = cli_fpm_fpm_bb_force, .parms = get_default },
            { .name = "fpm_bb_forced_ctrl", .val = cli_fpm_fpm_bb_forced_ctrl, .parms = get_default },
            { .name = "fpm_bb_forced_addr", .val = cli_fpm_fpm_bb_forced_addr, .parms = get_default },
            { .name = "fpm_bb_forced_data", .val = cli_fpm_fpm_bb_forced_data, .parms = get_default },
            { .name = "fpm_bb_decode_cfg", .val = cli_fpm_fpm_bb_decode_cfg, .parms = get_default },
            { .name = "fpm_bb_dbg_cfg", .val = cli_fpm_fpm_bb_dbg_cfg, .parms = get_default },
            { .name = "fpm_bb_dbg_rxfifo_sts", .val = cli_fpm_fpm_bb_dbg_rxfifo_sts, .parms = get_default },
            { .name = "fpm_bb_dbg_txfifo_sts", .val = cli_fpm_fpm_bb_dbg_txfifo_sts, .parms = get_default },
            { .name = "fpm_bb_dbg_rxfifo_data1", .val = cli_fpm_fpm_bb_dbg_rxfifo_data1, .parms = get_default },
            { .name = "fpm_bb_dbg_rxfifo_data2", .val = cli_fpm_fpm_bb_dbg_rxfifo_data2, .parms = get_default },
            { .name = "fpm_bb_dbg_txfifo_data1", .val = cli_fpm_fpm_bb_dbg_txfifo_data1, .parms = get_default },
            { .name = "fpm_bb_dbg_txfifo_data2", .val = cli_fpm_fpm_bb_dbg_txfifo_data2, .parms = get_default },
            { .name = "fpm_bb_dbg_txfifo_data3", .val = cli_fpm_fpm_bb_dbg_txfifo_data3, .parms = get_default },
            { .name = "fpm_bb_misc", .val = cli_fpm_fpm_bb_misc, .parms = get_default },
            { .name = "clk_gate_cntrl", .val = cli_fpm_clk_gate_cntrl, .parms = get_default },
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
            { .name = "FPM_CTL", .val = bdmf_address_fpm_ctl },
            { .name = "FPM_CFG1", .val = bdmf_address_fpm_cfg1 },
            { .name = "FPM_WEIGHT", .val = bdmf_address_fpm_weight },
            { .name = "FPM_BB_CFG", .val = bdmf_address_fpm_bb_cfg },
            { .name = "POOL1_INTR_MSK", .val = bdmf_address_pool1_intr_msk },
            { .name = "POOL1_INTR_STS", .val = bdmf_address_pool1_intr_sts },
            { .name = "POOL1_STALL_MSK", .val = bdmf_address_pool1_stall_msk },
            { .name = "POOL2_INTR_MSK", .val = bdmf_address_pool2_intr_msk },
            { .name = "POOL2_INTR_STS", .val = bdmf_address_pool2_intr_sts },
            { .name = "POOL2_STALL_MSK", .val = bdmf_address_pool2_stall_msk },
            { .name = "POOL3_INTR_MSK", .val = bdmf_address_pool3_intr_msk },
            { .name = "POOL3_INTR_STS", .val = bdmf_address_pool3_intr_sts },
            { .name = "POOL3_STALL_MSK", .val = bdmf_address_pool3_stall_msk },
            { .name = "POOL1_CFG1", .val = bdmf_address_pool1_cfg1 },
            { .name = "POOL1_CFG2", .val = bdmf_address_pool1_cfg2 },
            { .name = "POOL1_CFG3", .val = bdmf_address_pool1_cfg3 },
            { .name = "POOL1_CFG4", .val = bdmf_address_pool1_cfg4 },
            { .name = "POOL1_STAT1", .val = bdmf_address_pool1_stat1 },
            { .name = "POOL1_STAT2", .val = bdmf_address_pool1_stat2 },
            { .name = "POOL1_STAT3", .val = bdmf_address_pool1_stat3 },
            { .name = "POOL1_STAT4", .val = bdmf_address_pool1_stat4 },
            { .name = "POOL1_STAT5", .val = bdmf_address_pool1_stat5 },
            { .name = "POOL1_STAT6", .val = bdmf_address_pool1_stat6 },
            { .name = "POOL1_STAT7", .val = bdmf_address_pool1_stat7 },
            { .name = "POOL1_STAT8", .val = bdmf_address_pool1_stat8 },
            { .name = "POOL2_STAT1", .val = bdmf_address_pool2_stat1 },
            { .name = "POOL2_STAT2", .val = bdmf_address_pool2_stat2 },
            { .name = "POOL2_STAT3", .val = bdmf_address_pool2_stat3 },
            { .name = "POOL2_STAT4", .val = bdmf_address_pool2_stat4 },
            { .name = "POOL2_STAT5", .val = bdmf_address_pool2_stat5 },
            { .name = "POOL2_STAT6", .val = bdmf_address_pool2_stat6 },
            { .name = "POOL2_STAT7", .val = bdmf_address_pool2_stat7 },
            { .name = "POOL2_STAT8", .val = bdmf_address_pool2_stat8 },
            { .name = "POOL3_STAT1", .val = bdmf_address_pool3_stat1 },
            { .name = "POOL3_STAT2", .val = bdmf_address_pool3_stat2 },
            { .name = "POOL3_STAT3", .val = bdmf_address_pool3_stat3 },
            { .name = "POOL3_STAT5", .val = bdmf_address_pool3_stat5 },
            { .name = "POOL3_STAT6", .val = bdmf_address_pool3_stat6 },
            { .name = "POOL3_STAT8", .val = bdmf_address_pool3_stat8 },
            { .name = "POOL1_XON_XOFF_CFG", .val = bdmf_address_pool1_xon_xoff_cfg },
            { .name = "FPM_NOT_EMPTY_CFG", .val = bdmf_address_fpm_not_empty_cfg },
            { .name = "MEM_CTL", .val = bdmf_address_mem_ctl },
            { .name = "MEM_DATA1", .val = bdmf_address_mem_data1 },
            { .name = "MEM_DATA2", .val = bdmf_address_mem_data2 },
            { .name = "SPARE", .val = bdmf_address_spare },
            { .name = "TOKEN_RECOVER_CTL", .val = bdmf_address_token_recover_ctl },
            { .name = "SHORT_AGING_TIMER", .val = bdmf_address_short_aging_timer },
            { .name = "LONG_AGING_TIMER", .val = bdmf_address_long_aging_timer },
            { .name = "CACHE_RECYCLE_TIMER", .val = bdmf_address_cache_recycle_timer },
            { .name = "EXPIRED_TOKEN_COUNT_POOL1", .val = bdmf_address_expired_token_count_pool1 },
            { .name = "RECOVERED_TOKEN_COUNT_POOL1", .val = bdmf_address_recovered_token_count_pool1 },
            { .name = "EXPIRED_TOKEN_COUNT_POOL2", .val = bdmf_address_expired_token_count_pool2 },
            { .name = "RECOVERED_TOKEN_COUNT_POOL2", .val = bdmf_address_recovered_token_count_pool2 },
            { .name = "TOKEN_RECOVER_START_END_POOL1", .val = bdmf_address_token_recover_start_end_pool1 },
            { .name = "TOKEN_RECOVER_START_END_POOL2", .val = bdmf_address_token_recover_start_end_pool2 },
            { .name = "PRBS_INVALID_GEN", .val = bdmf_address_prbs_invalid_gen },
            { .name = "POOL1_ALLOC_DEALLOC", .val = bdmf_address_pool1_alloc_dealloc },
            { .name = "POOL2_ALLOC_DEALLOC", .val = bdmf_address_pool2_alloc_dealloc },
            { .name = "POOL3_ALLOC_DEALLOC", .val = bdmf_address_pool3_alloc_dealloc },
            { .name = "POOL4_ALLOC_DEALLOC", .val = bdmf_address_pool4_alloc_dealloc },
            { .name = "POOL_MULTI", .val = bdmf_address_pool_multi },
            { .name = "POOL1_ALLOC_DEALLOC_1", .val = bdmf_address_pool1_alloc_dealloc_1 },
            { .name = "POOL2_ALLOC_DEALLOC_1", .val = bdmf_address_pool2_alloc_dealloc_1 },
            { .name = "POOL3_ALLOC_DEALLOC_1", .val = bdmf_address_pool3_alloc_dealloc_1 },
            { .name = "POOL4_ALLOC_DEALLOC_1", .val = bdmf_address_pool4_alloc_dealloc_1 },
            { .name = "POOL_MULTI_1", .val = bdmf_address_pool_multi_1 },
            { .name = "SEARCH_DATA_0", .val = bdmf_address_search_data_0 },
            { .name = "SEARCH_DATA_1", .val = bdmf_address_search_data_1 },
            { .name = "SEARCH_DATA_2", .val = bdmf_address_search_data_2 },
            { .name = "SEARCH_DATA_3", .val = bdmf_address_search_data_3 },
            { .name = "MULTICAST_DATA", .val = bdmf_address_multicast_data },
            { .name = "COMPUTE_POOL_DATA", .val = bdmf_address_compute_pool_data },
            { .name = "FPM_BB_FORCE", .val = bdmf_address_fpm_bb_force },
            { .name = "FPM_BB_FORCED_CTRL", .val = bdmf_address_fpm_bb_forced_ctrl },
            { .name = "FPM_BB_FORCED_ADDR", .val = bdmf_address_fpm_bb_forced_addr },
            { .name = "FPM_BB_FORCED_DATA", .val = bdmf_address_fpm_bb_forced_data },
            { .name = "FPM_BB_DECODE_CFG", .val = bdmf_address_fpm_bb_decode_cfg },
            { .name = "FPM_BB_DBG_CFG", .val = bdmf_address_fpm_bb_dbg_cfg },
            { .name = "FPM_BB_DBG_RXFIFO_STS", .val = bdmf_address_fpm_bb_dbg_rxfifo_sts },
            { .name = "FPM_BB_DBG_TXFIFO_STS", .val = bdmf_address_fpm_bb_dbg_txfifo_sts },
            { .name = "FPM_BB_DBG_RXFIFO_DATA1", .val = bdmf_address_fpm_bb_dbg_rxfifo_data1 },
            { .name = "FPM_BB_DBG_RXFIFO_DATA2", .val = bdmf_address_fpm_bb_dbg_rxfifo_data2 },
            { .name = "FPM_BB_DBG_TXFIFO_DATA1", .val = bdmf_address_fpm_bb_dbg_txfifo_data1 },
            { .name = "FPM_BB_DBG_TXFIFO_DATA2", .val = bdmf_address_fpm_bb_dbg_txfifo_data2 },
            { .name = "FPM_BB_DBG_TXFIFO_DATA3", .val = bdmf_address_fpm_bb_dbg_txfifo_data3 },
            { .name = "FPM_BB_MISC", .val = bdmf_address_fpm_bb_misc },
            { .name = "CLK_GATE_CNTRL", .val = bdmf_address_clk_gate_cntrl },
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
