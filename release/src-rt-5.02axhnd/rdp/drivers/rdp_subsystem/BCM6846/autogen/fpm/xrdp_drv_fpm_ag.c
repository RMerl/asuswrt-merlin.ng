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
#include "xrdp_drv_fpm_ag.h"

bdmf_error_t ag_drv_fpm_init_mem_set(bdmf_boolean init_mem)
{
    uint32_t reg_fpm_ctl=0;

#ifdef VALIDATE_PARMS
    if((init_mem >= _1BITS_MAX_VAL_))
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
    if(!init_mem)
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
    uint32_t reg_fpm_ctl=0;

#ifdef VALIDATE_PARMS
    if((pool1_enable >= _1BITS_MAX_VAL_))
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
    if(!pool1_enable)
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
    uint32_t reg_fpm_ctl=0;

#ifdef VALIDATE_PARMS
    if((fpm_bb_soft_reset >= _1BITS_MAX_VAL_))
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
    if(!fpm_bb_soft_reset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CTL, reg_fpm_ctl);

    *fpm_bb_soft_reset = RU_FIELD_GET(0, FPM, FPM_CTL, FPM_BB_SOFT_RESET, reg_fpm_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_search_set(bdmf_boolean pool1_search_mode, bdmf_boolean pool1_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1=0;

#ifdef VALIDATE_PARMS
    if((pool1_search_mode >= _1BITS_MAX_VAL_) ||
       (pool1_cache_bypass_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL1_SEARCH_MODE, reg_fpm_cfg1, pool1_search_mode);
    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL1_CACHE_BYPASS_EN, reg_fpm_cfg1, pool1_cache_bypass_en);

    RU_REG_WRITE(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_search_get(bdmf_boolean *pool1_search_mode, bdmf_boolean *pool1_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1;

#ifdef VALIDATE_PARMS
    if(!pool1_search_mode || !pool1_cache_bypass_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    *pool1_search_mode = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL1_SEARCH_MODE, reg_fpm_cfg1);
    *pool1_cache_bypass_en = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL1_CACHE_BYPASS_EN, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_search2_set(bdmf_boolean pool2_search_mode, bdmf_boolean pool2_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1=0;

#ifdef VALIDATE_PARMS
    if((pool2_search_mode >= _1BITS_MAX_VAL_) ||
       (pool2_cache_bypass_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL2_SEARCH_MODE, reg_fpm_cfg1, pool2_search_mode);
    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL2_CACHE_BYPASS_EN, reg_fpm_cfg1, pool2_cache_bypass_en);

    RU_REG_WRITE(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_search2_get(bdmf_boolean *pool2_search_mode, bdmf_boolean *pool2_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1;

#ifdef VALIDATE_PARMS
    if(!pool2_search_mode || !pool2_cache_bypass_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    *pool2_search_mode = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL2_SEARCH_MODE, reg_fpm_cfg1);
    *pool2_cache_bypass_en = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL2_CACHE_BYPASS_EN, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_search3_set(bdmf_boolean pool3_search_mode, bdmf_boolean pool3_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1=0;

#ifdef VALIDATE_PARMS
    if((pool3_search_mode >= _1BITS_MAX_VAL_) ||
       (pool3_cache_bypass_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL3_SEARCH_MODE, reg_fpm_cfg1, pool3_search_mode);
    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL3_CACHE_BYPASS_EN, reg_fpm_cfg1, pool3_cache_bypass_en);

    RU_REG_WRITE(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool3_search3_get(bdmf_boolean *pool3_search_mode, bdmf_boolean *pool3_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1;

#ifdef VALIDATE_PARMS
    if(!pool3_search_mode || !pool3_cache_bypass_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    *pool3_search_mode = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL3_SEARCH_MODE, reg_fpm_cfg1);
    *pool3_cache_bypass_en = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL3_CACHE_BYPASS_EN, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_search4_set(bdmf_boolean pool4_search_mode, bdmf_boolean pool4_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1=0;

#ifdef VALIDATE_PARMS
    if((pool4_search_mode >= _1BITS_MAX_VAL_) ||
       (pool4_cache_bypass_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL4_SEARCH_MODE, reg_fpm_cfg1, pool4_search_mode);
    reg_fpm_cfg1 = RU_FIELD_SET(0, FPM, FPM_CFG1, POOL4_CACHE_BYPASS_EN, reg_fpm_cfg1, pool4_cache_bypass_en);

    RU_REG_WRITE(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool4_search4_get(bdmf_boolean *pool4_search_mode, bdmf_boolean *pool4_cache_bypass_en)
{
    uint32_t reg_fpm_cfg1;

#ifdef VALIDATE_PARMS
    if(!pool4_search_mode || !pool4_cache_bypass_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_CFG1, reg_fpm_cfg1);

    *pool4_search_mode = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL4_SEARCH_MODE, reg_fpm_cfg1);
    *pool4_cache_bypass_en = RU_FIELD_GET(0, FPM, FPM_CFG1, POOL4_CACHE_BYPASS_EN, reg_fpm_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_ddr0_weight_set(uint8_t ddr0_alloc_weight, uint8_t ddr0_free_weight)
{
    uint32_t reg_fpm_weight=0;

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
    if(!ddr0_alloc_weight || !ddr0_free_weight)
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
    uint32_t reg_fpm_weight=0;

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
    if(!ddr1_alloc_weight || !ddr1_free_weight)
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
    uint32_t reg_pool1_cfg1=0;
    uint32_t reg_pool1_cfg2=0;
    uint32_t reg_pool1_cfg3=0;

#ifdef VALIDATE_PARMS
    if(!pool_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool_cfg->fp_buf_size >= _3BITS_MAX_VAL_) ||
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
    if(!pool_cfg)
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

bdmf_error_t ag_drv_fpm_pool_stat_get(fpm_pool_stat *pool_stat)
{
    uint32_t reg_pool1_stat1;
    uint32_t reg_pool1_stat2;
    uint32_t reg_pool1_stat3;
    uint32_t reg_pool1_stat4;
    uint32_t reg_pool1_stat5;
    uint32_t reg_pool1_stat6;
    uint32_t reg_pool1_stat7;
    uint32_t reg_pool1_stat8;

#ifdef VALIDATE_PARMS
    if(!pool_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_STAT1, reg_pool1_stat1);
    RU_REG_READ(0, FPM, POOL1_STAT2, reg_pool1_stat2);
    RU_REG_READ(0, FPM, POOL1_STAT3, reg_pool1_stat3);
    RU_REG_READ(0, FPM, POOL1_STAT4, reg_pool1_stat4);
    RU_REG_READ(0, FPM, POOL1_STAT5, reg_pool1_stat5);
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
    pool_stat->mem_corrupt_sts_related_alloc_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID, reg_pool1_stat5);
    pool_stat->mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool1_stat5);
    pool_stat->invalid_free_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN_VALID, reg_pool1_stat6);
    pool_stat->invalid_free_token = RU_FIELD_GET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN, reg_pool1_stat6);
    pool_stat->invalid_mcast_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN_VALID, reg_pool1_stat7);
    pool_stat->invalid_mcast_token = RU_FIELD_GET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN, reg_pool1_stat7);
    pool_stat->tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, POOL1_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool1_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stat_get(fpm_pool_stat *pool_stat)
{
    /* Identical to pool_stat */
    uint32_t reg_pool1_stat1;
    uint32_t reg_pool1_stat2;
    uint32_t reg_pool1_stat3;
    uint32_t reg_pool1_stat4;
    uint32_t reg_pool1_stat5;
    uint32_t reg_pool1_stat6;
    uint32_t reg_pool1_stat7;
    uint32_t reg_pool1_stat8;

#ifdef VALIDATE_PARMS
    if(!pool_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STAT1, reg_pool1_stat1);
    RU_REG_READ(0, FPM, POOL2_STAT2, reg_pool1_stat2);
    RU_REG_READ(0, FPM, POOL2_STAT3, reg_pool1_stat3);
    RU_REG_READ(0, FPM, POOL2_STAT4, reg_pool1_stat4);
    RU_REG_READ(0, FPM, POOL2_STAT5, reg_pool1_stat5);
    RU_REG_READ(0, FPM, POOL2_STAT6, reg_pool1_stat6);
    RU_REG_READ(0, FPM, POOL2_STAT7, reg_pool1_stat7);
    RU_REG_READ(0, FPM, POOL2_STAT8, reg_pool1_stat8);

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
    pool_stat->mem_corrupt_sts_related_alloc_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID, reg_pool1_stat5);
    pool_stat->mem_corrupt_sts_related_alloc_token = RU_FIELD_GET(0, FPM, POOL1_STAT5, MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN, reg_pool1_stat5);
    pool_stat->invalid_free_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN_VALID, reg_pool1_stat6);
    pool_stat->invalid_free_token = RU_FIELD_GET(0, FPM, POOL1_STAT6, INVALID_FREE_TOKEN, reg_pool1_stat6);
    pool_stat->invalid_mcast_token_valid = RU_FIELD_GET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN_VALID, reg_pool1_stat7);
    pool_stat->invalid_mcast_token = RU_FIELD_GET(0, FPM, POOL1_STAT7, INVALID_MCAST_TOKEN, reg_pool1_stat7);
    pool_stat->tokens_available_low_wtmk = RU_FIELD_GET(0, FPM, POOL1_STAT8, TOKENS_AVAILABLE_LOW_WTMK, reg_pool1_stat8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_back_door_mem_set(uint32_t mem_data1, uint32_t mem_data2)
{
    uint32_t reg_mem_data1=0;
    uint32_t reg_mem_data2=0;

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
    if(!mem_data1 || !mem_data2)
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
    if(!expired_count || !recovered_count)
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
    if(!expired_count || !recovered_count)
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
    uint32_t reg_long_aging_timer=0;
    uint32_t reg_short_aging_timer=0;
    uint32_t reg_cache_recycle_timer=0;

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
    if(!timer)
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

bdmf_error_t ag_drv_fpm_fpm_bb_cfg_set(uint8_t bb_ddr_sel)
{
    uint32_t reg_fpm_bb_cfg=0;

#ifdef VALIDATE_PARMS
    if((bb_ddr_sel >= _2BITS_MAX_VAL_))
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
    if(!bb_ddr_sel)
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
    uint32_t reg_pool2_intr_msk=0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_recov_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_det_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_alloc_request_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_address_access_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk, pool2_intr_msk->xon_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk, pool2_intr_msk->xoff_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk, pool2_intr_msk->memory_corrupt_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_dis_free_multi_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->alloc_fifo_full_msk);

    RU_REG_WRITE(0, FPM, POOL1_INTR_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk)
{
    /* Identical to pool2_intr_msk */
    uint32_t reg_pool2_intr_msk;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_INTR_MSK, reg_pool2_intr_msk);

    pool2_intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xon_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts)
{
    /* Identical to pool2_intr_sts */
    uint32_t reg_pool2_intr_sts=0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_recov_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_det_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_alloc_request_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_address_access_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xon_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xoff_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts, pool2_intr_sts->memory_corrupt_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_dis_free_multi_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->free_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->alloc_fifo_full_sts);

    RU_REG_WRITE(0, FPM, POOL1_INTR_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts)
{
    /* Identical to pool2_intr_sts */
    uint32_t reg_pool2_intr_sts;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_INTR_STS, reg_pool2_intr_sts);

    pool2_intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk)
{
    /* Identical to pool2_stall_msk */
    uint32_t reg_pool2_stall_msk=0;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->memory_corrupt_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_no_valid_stall_msk);

    RU_REG_WRITE(0, FPM, POOL1_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk)
{
    /* Identical to pool2_stall_msk */
    uint32_t reg_pool2_stall_msk;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_STALL_MSK, reg_pool2_stall_msk);

    pool2_stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_msk_set(const fpm_pool2_intr_msk *pool2_intr_msk)
{
    uint32_t reg_pool2_intr_msk=0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_intr_msk->expired_token_recov_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->expired_token_det_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_alloc_request_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->illegal_address_access_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xon_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->xoff_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->memory_corrupt_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_dis_free_multi_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->multi_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_index_out_of_range_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_token_no_valid_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->pool_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->free_fifo_full_msk >= _1BITS_MAX_VAL_) ||
       (pool2_intr_msk->alloc_fifo_full_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_recov_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk, pool2_intr_msk->expired_token_det_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_alloc_request_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk, pool2_intr_msk->illegal_address_access_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk, pool2_intr_msk->xon_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk, pool2_intr_msk->xoff_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk, pool2_intr_msk->memory_corrupt_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_dis_free_multi_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->multi_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_index_out_of_range_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_token_no_valid_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->pool_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->free_fifo_full_msk);
    reg_pool2_intr_msk = RU_FIELD_SET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk, pool2_intr_msk->alloc_fifo_full_msk);

    RU_REG_WRITE(0, FPM, POOL2_INTR_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk)
{
    uint32_t reg_pool2_intr_msk;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_INTR_MSK, reg_pool2_intr_msk);

    pool2_intr_msk->expired_token_recov_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_RECOV_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->expired_token_det_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, EXPIRED_TOKEN_DET_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_alloc_request_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ALLOC_REQUEST_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->illegal_address_access_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ILLEGAL_ADDRESS_ACCESS_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xon_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XON_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->xoff_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, XOFF_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->memory_corrupt_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MEMORY_CORRUPT_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_dis_free_multi_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_DIS_FREE_MULTI_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->multi_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, MULTI_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_index_out_of_range_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_token_no_valid_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_TOKEN_NO_VALID_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->pool_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, POOL_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->free_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, FREE_FIFO_FULL_MSK, reg_pool2_intr_msk);
    pool2_intr_msk->alloc_fifo_full_msk = RU_FIELD_GET(0, FPM, POOL2_INTR_MSK, ALLOC_FIFO_FULL_MSK, reg_pool2_intr_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts)
{
    uint32_t reg_pool2_intr_sts=0;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_intr_sts->expired_token_recov_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->expired_token_det_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_alloc_request_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->illegal_address_access_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xon_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->xoff_state_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->memory_corrupt_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_dis_free_multi_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->multi_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_index_out_of_range_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_token_no_valid_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->pool_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->free_fifo_full_sts >= _1BITS_MAX_VAL_) ||
       (pool2_intr_sts->alloc_fifo_full_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_recov_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts, pool2_intr_sts->expired_token_det_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_alloc_request_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts, pool2_intr_sts->illegal_address_access_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xon_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts, pool2_intr_sts->xoff_state_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts, pool2_intr_sts->memory_corrupt_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_dis_free_multi_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->multi_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_index_out_of_range_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts, pool2_intr_sts->free_token_no_valid_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->pool_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->free_fifo_full_sts);
    reg_pool2_intr_sts = RU_FIELD_SET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts, pool2_intr_sts->alloc_fifo_full_sts);

    RU_REG_WRITE(0, FPM, POOL2_INTR_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts)
{
    uint32_t reg_pool2_intr_sts;

#ifdef VALIDATE_PARMS
    if(!pool2_intr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_INTR_STS, reg_pool2_intr_sts);

    pool2_intr_sts->expired_token_recov_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_RECOV_STS, reg_pool2_intr_sts);
    pool2_intr_sts->expired_token_det_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, EXPIRED_TOKEN_DET_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_alloc_request_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ALLOC_REQUEST_STS, reg_pool2_intr_sts);
    pool2_intr_sts->illegal_address_access_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ILLEGAL_ADDRESS_ACCESS_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xon_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XON_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->xoff_state_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, XOFF_STATE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->memory_corrupt_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MEMORY_CORRUPT_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_dis_free_multi_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_DIS_FREE_MULTI_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->multi_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, MULTI_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_index_out_of_range_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_INDEX_OUT_OF_RANGE_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_token_no_valid_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_TOKEN_NO_VALID_STS, reg_pool2_intr_sts);
    pool2_intr_sts->pool_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, POOL_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->free_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, FREE_FIFO_FULL_STS, reg_pool2_intr_sts);
    pool2_intr_sts->alloc_fifo_full_sts = RU_FIELD_GET(0, FPM, POOL2_INTR_STS, ALLOC_FIFO_FULL_STS, reg_pool2_intr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk)
{
    uint32_t reg_pool2_stall_msk=0;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool2_stall_msk->memory_corrupt_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->multi_token_no_valid_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_index_out_of_range_stall_msk >= _1BITS_MAX_VAL_) ||
       (pool2_stall_msk->free_token_no_valid_stall_msk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->memory_corrupt_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->multi_token_no_valid_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_index_out_of_range_stall_msk);
    reg_pool2_stall_msk = RU_FIELD_SET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk, pool2_stall_msk->free_token_no_valid_stall_msk);

    RU_REG_WRITE(0, FPM, POOL2_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool2_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk)
{
    uint32_t reg_pool2_stall_msk;

#ifdef VALIDATE_PARMS
    if(!pool2_stall_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL2_STALL_MSK, reg_pool2_stall_msk);

    pool2_stall_msk->memory_corrupt_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MEMORY_CORRUPT_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->multi_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, MULTI_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_index_out_of_range_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK, reg_pool2_stall_msk);
    pool2_stall_msk->free_token_no_valid_stall_msk = RU_FIELD_GET(0, FPM, POOL2_STALL_MSK, FREE_TOKEN_NO_VALID_STALL_MSK, reg_pool2_stall_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_set(uint16_t xon_threshold, uint16_t xoff_threshold)
{
    uint32_t reg_pool1_xon_xoff_cfg=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pool1_xon_xoff_cfg = RU_FIELD_SET(0, FPM, POOL1_XON_XOFF_CFG, XON_THRESHOLD, reg_pool1_xon_xoff_cfg, xon_threshold);
    reg_pool1_xon_xoff_cfg = RU_FIELD_SET(0, FPM, POOL1_XON_XOFF_CFG, XOFF_THRESHOLD, reg_pool1_xon_xoff_cfg, xoff_threshold);

    RU_REG_WRITE(0, FPM, POOL1_XON_XOFF_CFG, reg_pool1_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_get(uint16_t *xon_threshold, uint16_t *xoff_threshold)
{
    uint32_t reg_pool1_xon_xoff_cfg;

#ifdef VALIDATE_PARMS
    if(!xon_threshold || !xoff_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL1_XON_XOFF_CFG, reg_pool1_xon_xoff_cfg);

    *xon_threshold = RU_FIELD_GET(0, FPM, POOL1_XON_XOFF_CFG, XON_THRESHOLD, reg_pool1_xon_xoff_cfg);
    *xoff_threshold = RU_FIELD_GET(0, FPM, POOL1_XON_XOFF_CFG, XOFF_THRESHOLD, reg_pool1_xon_xoff_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_set(uint8_t not_empty_threshold)
{
    uint32_t reg_fpm_not_empty_cfg=0;

#ifdef VALIDATE_PARMS
    if((not_empty_threshold >= _6BITS_MAX_VAL_))
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
    if(!not_empty_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_NOT_EMPTY_CFG, reg_fpm_not_empty_cfg);

    *not_empty_threshold = RU_FIELD_GET(0, FPM, FPM_NOT_EMPTY_CFG, NOT_EMPTY_THRESHOLD, reg_fpm_not_empty_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_mem_ctl_set(bdmf_boolean mem_wr, bdmf_boolean mem_rd, uint8_t mem_sel, uint16_t mem_addr)
{
    uint32_t reg_mem_ctl=0;

#ifdef VALIDATE_PARMS
    if((mem_wr >= _1BITS_MAX_VAL_) ||
       (mem_rd >= _1BITS_MAX_VAL_) ||
       (mem_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_WR, reg_mem_ctl, mem_wr);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_RD, reg_mem_ctl, mem_rd);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_SEL, reg_mem_ctl, mem_sel);
    reg_mem_ctl = RU_FIELD_SET(0, FPM, MEM_CTL, MEM_ADDR, reg_mem_ctl, mem_addr);

    RU_REG_WRITE(0, FPM, MEM_CTL, reg_mem_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_mem_ctl_get(bdmf_boolean *mem_wr, bdmf_boolean *mem_rd, uint8_t *mem_sel, uint16_t *mem_addr)
{
    uint32_t reg_mem_ctl;

#ifdef VALIDATE_PARMS
    if(!mem_wr || !mem_rd || !mem_sel || !mem_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, MEM_CTL, reg_mem_ctl);

    *mem_wr = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_WR, reg_mem_ctl);
    *mem_rd = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_RD, reg_mem_ctl);
    *mem_sel = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_SEL, reg_mem_ctl);
    *mem_addr = RU_FIELD_GET(0, FPM, MEM_CTL, MEM_ADDR, reg_mem_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_ctl_set(const fpm_token_recover_ctl *token_recover_ctl)
{
    uint32_t reg_token_recover_ctl=0;

#ifdef VALIDATE_PARMS
    if(!token_recover_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((token_recover_ctl->clr_recovered_token_count >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->clr_expired_token_count >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->force_token_reclaim >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->token_reclaim_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->token_remark_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->single_pass_ena >= _1BITS_MAX_VAL_) ||
       (token_recover_ctl->token_recover_ena >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, CLR_RECOVERED_TOKEN_COUNT, reg_token_recover_ctl, token_recover_ctl->clr_recovered_token_count);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, CLR_EXPIRED_TOKEN_COUNT, reg_token_recover_ctl, token_recover_ctl->clr_expired_token_count);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, FORCE_TOKEN_RECLAIM, reg_token_recover_ctl, token_recover_ctl->force_token_reclaim);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECLAIM_ENA, reg_token_recover_ctl, token_recover_ctl->token_reclaim_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_REMARK_ENA, reg_token_recover_ctl, token_recover_ctl->token_remark_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, SINGLE_PASS_ENA, reg_token_recover_ctl, token_recover_ctl->single_pass_ena);
    reg_token_recover_ctl = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECOVER_ENA, reg_token_recover_ctl, token_recover_ctl->token_recover_ena);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_CTL, reg_token_recover_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_ctl_get(fpm_token_recover_ctl *token_recover_ctl)
{
    uint32_t reg_token_recover_ctl;

#ifdef VALIDATE_PARMS
    if(!token_recover_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_CTL, reg_token_recover_ctl);

    token_recover_ctl->clr_recovered_token_count = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, CLR_RECOVERED_TOKEN_COUNT, reg_token_recover_ctl);
    token_recover_ctl->clr_expired_token_count = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, CLR_EXPIRED_TOKEN_COUNT, reg_token_recover_ctl);
    token_recover_ctl->force_token_reclaim = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, FORCE_TOKEN_RECLAIM, reg_token_recover_ctl);
    token_recover_ctl->token_reclaim_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECLAIM_ENA, reg_token_recover_ctl);
    token_recover_ctl->token_remark_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_REMARK_ENA, reg_token_recover_ctl);
    token_recover_ctl->single_pass_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, SINGLE_PASS_ENA, reg_token_recover_ctl);
    token_recover_ctl->token_recover_ena = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_CTL, TOKEN_RECOVER_ENA, reg_token_recover_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_set(uint16_t start_index, uint16_t end_index)
{
    uint32_t reg_token_recover_start_end_pool1=0;

#ifdef VALIDATE_PARMS
    if((start_index >= _12BITS_MAX_VAL_) ||
       (end_index >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_token_recover_start_end_pool1 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL1, START_INDEX, reg_token_recover_start_end_pool1, start_index);
    reg_token_recover_start_end_pool1 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL1, END_INDEX, reg_token_recover_start_end_pool1, end_index);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_START_END_POOL1, reg_token_recover_start_end_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_get(uint16_t *start_index, uint16_t *end_index)
{
    uint32_t reg_token_recover_start_end_pool1;

#ifdef VALIDATE_PARMS
    if(!start_index || !end_index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_START_END_POOL1, reg_token_recover_start_end_pool1);

    *start_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL1, START_INDEX, reg_token_recover_start_end_pool1);
    *end_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL1, END_INDEX, reg_token_recover_start_end_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_set(uint16_t start_index, uint16_t end_index)
{
    uint32_t reg_token_recover_start_end_pool2=0;

#ifdef VALIDATE_PARMS
    if((start_index >= _12BITS_MAX_VAL_) ||
       (end_index >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_token_recover_start_end_pool2 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL2, START_INDEX, reg_token_recover_start_end_pool2, start_index);
    reg_token_recover_start_end_pool2 = RU_FIELD_SET(0, FPM, TOKEN_RECOVER_START_END_POOL2, END_INDEX, reg_token_recover_start_end_pool2, end_index);

    RU_REG_WRITE(0, FPM, TOKEN_RECOVER_START_END_POOL2, reg_token_recover_start_end_pool2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_get(uint16_t *start_index, uint16_t *end_index)
{
    uint32_t reg_token_recover_start_end_pool2;

#ifdef VALIDATE_PARMS
    if(!start_index || !end_index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, TOKEN_RECOVER_START_END_POOL2, reg_token_recover_start_end_pool2);

    *start_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL2, START_INDEX, reg_token_recover_start_end_pool2);
    *end_index = RU_FIELD_GET(0, FPM, TOKEN_RECOVER_START_END_POOL2, END_INDEX, reg_token_recover_start_end_pool2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool1_alloc_dealloc=0;

#ifdef VALIDATE_PARMS
    if((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _17BITS_MAX_VAL_) ||
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
    if(!token_valid || !ddr || !token_index || !token_size)
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

bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool2_alloc_dealloc=0;

#ifdef VALIDATE_PARMS
    if((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _17BITS_MAX_VAL_) ||
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
    if(!token_valid || !ddr || !token_index || !token_size)
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

bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool3_alloc_dealloc=0;

#ifdef VALIDATE_PARMS
    if((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _17BITS_MAX_VAL_) ||
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
    if(!token_valid || !ddr || !token_index || !token_size)
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

bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size)
{
    uint32_t reg_pool4_alloc_dealloc=0;

#ifdef VALIDATE_PARMS
    if((token_valid >= _1BITS_MAX_VAL_) ||
       (ddr >= _1BITS_MAX_VAL_) ||
       (token_index >= _17BITS_MAX_VAL_) ||
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
    if(!token_valid || !ddr || !token_index || !token_size)
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

bdmf_error_t ag_drv_fpm_pool_multi_set(const fpm_pool_multi *pool_multi)
{
    uint32_t reg_pool_multi=0;

#ifdef VALIDATE_PARMS
    if(!pool_multi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool_multi->token_valid >= _1BITS_MAX_VAL_) ||
       (pool_multi->ddr >= _1BITS_MAX_VAL_) ||
       (pool_multi->token_index >= _17BITS_MAX_VAL_) ||
       (pool_multi->update_type >= _1BITS_MAX_VAL_) ||
       (pool_multi->token_multi >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_VALID, reg_pool_multi, pool_multi->token_valid);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, DDR, reg_pool_multi, pool_multi->ddr);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_INDEX, reg_pool_multi, pool_multi->token_index);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, UPDATE_TYPE, reg_pool_multi, pool_multi->update_type);
    reg_pool_multi = RU_FIELD_SET(0, FPM, POOL_MULTI, TOKEN_MULTI, reg_pool_multi, pool_multi->token_multi);

    RU_REG_WRITE(0, FPM, POOL_MULTI, reg_pool_multi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_pool_multi_get(fpm_pool_multi *pool_multi)
{
    uint32_t reg_pool_multi;

#ifdef VALIDATE_PARMS
    if(!pool_multi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, POOL_MULTI, reg_pool_multi);

    pool_multi->token_valid = RU_FIELD_GET(0, FPM, POOL_MULTI, TOKEN_VALID, reg_pool_multi);
    pool_multi->ddr = RU_FIELD_GET(0, FPM, POOL_MULTI, DDR, reg_pool_multi);
    pool_multi->token_index = RU_FIELD_GET(0, FPM, POOL_MULTI, TOKEN_INDEX, reg_pool_multi);
    pool_multi->update_type = RU_FIELD_GET(0, FPM, POOL_MULTI, UPDATE_TYPE, reg_pool_multi);
    pool_multi->token_multi = RU_FIELD_GET(0, FPM, POOL_MULTI, TOKEN_MULTI, reg_pool_multi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpm_bb_force_set(bdmf_boolean force)
{
    uint32_t reg_fpm_bb_force=0;

#ifdef VALIDATE_PARMS
    if((force >= _1BITS_MAX_VAL_))
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
    if(!force)
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
    uint32_t reg_fpm_bb_forced_ctrl=0;

#ifdef VALIDATE_PARMS
    if((ctrl >= _12BITS_MAX_VAL_))
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
    if(!ctrl)
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
    uint32_t reg_fpm_bb_forced_addr=0;

#ifdef VALIDATE_PARMS
    if((dest_addr >= _6BITS_MAX_VAL_))
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
    if(!ta_addr || !dest_addr)
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
    uint32_t reg_fpm_bb_forced_data=0;

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
    if(!data)
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
    uint32_t reg_fpm_bb_decode_cfg=0;

#ifdef VALIDATE_PARMS
    if((dest_id >= _6BITS_MAX_VAL_) ||
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
    if(!dest_id || !override_en || !route_addr)
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
    uint32_t reg_fpm_bb_dbg_cfg=0;

#ifdef VALIDATE_PARMS
    if((rxfifo_sw_addr >= _4BITS_MAX_VAL_) ||
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
    if(!rxfifo_sw_addr || !txfifo_sw_addr || !rxfifo_sw_rst || !txfifo_sw_rst)
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
    if(!fpm_bb_dbg_rxfifo_sts)
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
    if(!fpm_bb_dbg_txfifo_sts)
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
    if(!data)
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
    if(!data)
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
    if(!data)
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
    if(!data)
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM, FPM_BB_DBG_TXFIFO_DATA3, reg_fpm_bb_dbg_txfifo_data3);

    *data = RU_FIELD_GET(0, FPM, FPM_BB_DBG_TXFIFO_DATA3, DATA, reg_fpm_bb_dbg_txfifo_data3);

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
    bdmf_address_pool1_cfg1,
    bdmf_address_pool1_cfg2,
    bdmf_address_pool1_cfg3,
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
    bdmf_address_pool1_xon_xoff_cfg,
    bdmf_address_fpm_not_empty_cfg,
    bdmf_address_mem_ctl,
    bdmf_address_mem_data1,
    bdmf_address_mem_data2,
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
    bdmf_address_pool1_alloc_dealloc,
    bdmf_address_pool2_alloc_dealloc,
    bdmf_address_pool3_alloc_dealloc,
    bdmf_address_pool4_alloc_dealloc,
    bdmf_address_pool_multi,
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
}
bdmf_address;

static int bcm_fpm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_fpm_init_mem:
        err = ag_drv_fpm_init_mem_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool1_en:
        err = ag_drv_fpm_pool1_en_set(parm[1].value.unumber);
        break;
    case cli_fpm_bb_reset:
        err = ag_drv_fpm_bb_reset_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool_search:
        err = ag_drv_fpm_pool_search_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool2_search2:
        err = ag_drv_fpm_pool2_search2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool3_search3:
        err = ag_drv_fpm_pool3_search3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool4_search4:
        err = ag_drv_fpm_pool4_search4_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_ddr0_weight:
        err = ag_drv_fpm_ddr0_weight_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_ddr1_weight:
        err = ag_drv_fpm_ddr1_weight_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool_cfg:
    {
        fpm_pool_cfg pool_cfg = { .fp_buf_size=parm[1].value.unumber, .pool_base_address=parm[2].value.unumber, .pool_base_address_pool2=parm[3].value.unumber};
        err = ag_drv_fpm_pool_cfg_set(&pool_cfg);
        break;
    }
    case cli_fpm_back_door_mem:
        err = ag_drv_fpm_back_door_mem_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_timer:
    {
        fpm_timer timer = { .long_aging_timer=parm[1].value.unumber, .short_aging_timer=parm[2].value.unumber, .recycle_timer=parm[3].value.unumber};
        err = ag_drv_fpm_timer_set(&timer);
        break;
    }
    case cli_fpm_fpm_bb_cfg:
        err = ag_drv_fpm_fpm_bb_cfg_set(parm[1].value.unumber);
        break;
    case cli_fpm_pool1_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk = { .expired_token_recov_msk=parm[1].value.unumber, .expired_token_det_msk=parm[2].value.unumber, .illegal_alloc_request_msk=parm[3].value.unumber, .illegal_address_access_msk=parm[4].value.unumber, .xon_msk=parm[5].value.unumber, .xoff_msk=parm[6].value.unumber, .memory_corrupt_msk=parm[7].value.unumber, .pool_dis_free_multi_msk=parm[8].value.unumber, .multi_token_index_out_of_range_msk=parm[9].value.unumber, .multi_token_no_valid_msk=parm[10].value.unumber, .free_token_index_out_of_range_msk=parm[11].value.unumber, .free_token_no_valid_msk=parm[12].value.unumber, .pool_full_msk=parm[13].value.unumber, .free_fifo_full_msk=parm[14].value.unumber, .alloc_fifo_full_msk=parm[15].value.unumber};
        err = ag_drv_fpm_pool1_intr_msk_set(&pool2_intr_msk);
        break;
    }
    case cli_fpm_pool1_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts = { .expired_token_recov_sts=parm[1].value.unumber, .expired_token_det_sts=parm[2].value.unumber, .illegal_alloc_request_sts=parm[3].value.unumber, .illegal_address_access_sts=parm[4].value.unumber, .xon_state_sts=parm[5].value.unumber, .xoff_state_sts=parm[6].value.unumber, .memory_corrupt_sts=parm[7].value.unumber, .pool_dis_free_multi_sts=parm[8].value.unumber, .multi_token_index_out_of_range_sts=parm[9].value.unumber, .multi_token_no_valid_sts=parm[10].value.unumber, .free_token_index_out_of_range_sts=parm[11].value.unumber, .free_token_no_valid_sts=parm[12].value.unumber, .pool_full_sts=parm[13].value.unumber, .free_fifo_full_sts=parm[14].value.unumber, .alloc_fifo_full_sts=parm[15].value.unumber};
        err = ag_drv_fpm_pool1_intr_sts_set(&pool2_intr_sts);
        break;
    }
    case cli_fpm_pool1_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk = { .memory_corrupt_stall_msk=parm[1].value.unumber, .multi_token_index_out_of_range_stall_msk=parm[2].value.unumber, .multi_token_no_valid_stall_msk=parm[3].value.unumber, .free_token_index_out_of_range_stall_msk=parm[4].value.unumber, .free_token_no_valid_stall_msk=parm[5].value.unumber};
        err = ag_drv_fpm_pool1_stall_msk_set(&pool2_stall_msk);
        break;
    }
    case cli_fpm_pool2_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk = { .expired_token_recov_msk=parm[1].value.unumber, .expired_token_det_msk=parm[2].value.unumber, .illegal_alloc_request_msk=parm[3].value.unumber, .illegal_address_access_msk=parm[4].value.unumber, .xon_msk=parm[5].value.unumber, .xoff_msk=parm[6].value.unumber, .memory_corrupt_msk=parm[7].value.unumber, .pool_dis_free_multi_msk=parm[8].value.unumber, .multi_token_index_out_of_range_msk=parm[9].value.unumber, .multi_token_no_valid_msk=parm[10].value.unumber, .free_token_index_out_of_range_msk=parm[11].value.unumber, .free_token_no_valid_msk=parm[12].value.unumber, .pool_full_msk=parm[13].value.unumber, .free_fifo_full_msk=parm[14].value.unumber, .alloc_fifo_full_msk=parm[15].value.unumber};
        err = ag_drv_fpm_pool2_intr_msk_set(&pool2_intr_msk);
        break;
    }
    case cli_fpm_pool2_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts = { .expired_token_recov_sts=parm[1].value.unumber, .expired_token_det_sts=parm[2].value.unumber, .illegal_alloc_request_sts=parm[3].value.unumber, .illegal_address_access_sts=parm[4].value.unumber, .xon_state_sts=parm[5].value.unumber, .xoff_state_sts=parm[6].value.unumber, .memory_corrupt_sts=parm[7].value.unumber, .pool_dis_free_multi_sts=parm[8].value.unumber, .multi_token_index_out_of_range_sts=parm[9].value.unumber, .multi_token_no_valid_sts=parm[10].value.unumber, .free_token_index_out_of_range_sts=parm[11].value.unumber, .free_token_no_valid_sts=parm[12].value.unumber, .pool_full_sts=parm[13].value.unumber, .free_fifo_full_sts=parm[14].value.unumber, .alloc_fifo_full_sts=parm[15].value.unumber};
        err = ag_drv_fpm_pool2_intr_sts_set(&pool2_intr_sts);
        break;
    }
    case cli_fpm_pool2_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk = { .memory_corrupt_stall_msk=parm[1].value.unumber, .multi_token_index_out_of_range_stall_msk=parm[2].value.unumber, .multi_token_no_valid_stall_msk=parm[3].value.unumber, .free_token_index_out_of_range_stall_msk=parm[4].value.unumber, .free_token_no_valid_stall_msk=parm[5].value.unumber};
        err = ag_drv_fpm_pool2_stall_msk_set(&pool2_stall_msk);
        break;
    }
    case cli_fpm_pool1_xon_xoff_cfg:
        err = ag_drv_fpm_pool1_xon_xoff_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_not_empty_cfg:
        err = ag_drv_fpm_fpm_not_empty_cfg_set(parm[1].value.unumber);
        break;
    case cli_fpm_mem_ctl:
        err = ag_drv_fpm_mem_ctl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_token_recover_ctl:
    {
        fpm_token_recover_ctl token_recover_ctl = { .clr_recovered_token_count=parm[1].value.unumber, .clr_expired_token_count=parm[2].value.unumber, .force_token_reclaim=parm[3].value.unumber, .token_reclaim_ena=parm[4].value.unumber, .token_remark_ena=parm[5].value.unumber, .single_pass_ena=parm[6].value.unumber, .token_recover_ena=parm[7].value.unumber};
        err = ag_drv_fpm_token_recover_ctl_set(&token_recover_ctl);
        break;
    }
    case cli_fpm_token_recover_start_end_pool1:
        err = ag_drv_fpm_token_recover_start_end_pool1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_token_recover_start_end_pool2:
        err = ag_drv_fpm_token_recover_start_end_pool2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_pool1_alloc_dealloc:
        err = ag_drv_fpm_pool1_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool2_alloc_dealloc:
        err = ag_drv_fpm_pool2_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool3_alloc_dealloc:
        err = ag_drv_fpm_pool3_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool4_alloc_dealloc:
        err = ag_drv_fpm_pool4_alloc_dealloc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_fpm_pool_multi:
    {
        fpm_pool_multi pool_multi = { .token_valid=parm[1].value.unumber, .ddr=parm[2].value.unumber, .token_index=parm[3].value.unumber, .update_type=parm[4].value.unumber, .token_multi=parm[5].value.unumber};
        err = ag_drv_fpm_pool_multi_set(&pool_multi);
        break;
    }
    case cli_fpm_fpm_bb_force:
        err = ag_drv_fpm_fpm_bb_force_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_ctrl:
        err = ag_drv_fpm_fpm_bb_forced_ctrl_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_addr:
        err = ag_drv_fpm_fpm_bb_forced_addr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_fpm_fpm_bb_forced_data:
        err = ag_drv_fpm_fpm_bb_forced_data_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpm_bb_decode_cfg:
        err = ag_drv_fpm_fpm_bb_decode_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_fpm_fpm_bb_dbg_cfg:
        err = ag_drv_fpm_fpm_bb_dbg_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_fpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_fpm_init_mem:
    {
        bdmf_boolean init_mem;
        err = ag_drv_fpm_init_mem_get(&init_mem);
        bdmf_session_print(session, "init_mem = %u (0x%x)\n", init_mem, init_mem);
        break;
    }
    case cli_fpm_pool1_en:
    {
        bdmf_boolean pool1_enable;
        err = ag_drv_fpm_pool1_en_get(&pool1_enable);
        bdmf_session_print(session, "pool1_enable = %u (0x%x)\n", pool1_enable, pool1_enable);
        break;
    }
    case cli_fpm_bb_reset:
    {
        bdmf_boolean fpm_bb_soft_reset;
        err = ag_drv_fpm_bb_reset_get(&fpm_bb_soft_reset);
        bdmf_session_print(session, "fpm_bb_soft_reset = %u (0x%x)\n", fpm_bb_soft_reset, fpm_bb_soft_reset);
        break;
    }
    case cli_fpm_pool_search:
    {
        bdmf_boolean pool1_search_mode;
        bdmf_boolean pool1_cache_bypass_en;
        err = ag_drv_fpm_pool_search_get(&pool1_search_mode, &pool1_cache_bypass_en);
        bdmf_session_print(session, "pool1_search_mode = %u (0x%x)\n", pool1_search_mode, pool1_search_mode);
        bdmf_session_print(session, "pool1_cache_bypass_en = %u (0x%x)\n", pool1_cache_bypass_en, pool1_cache_bypass_en);
        break;
    }
    case cli_fpm_pool2_search2:
    {
        bdmf_boolean pool2_search_mode;
        bdmf_boolean pool2_cache_bypass_en;
        err = ag_drv_fpm_pool2_search2_get(&pool2_search_mode, &pool2_cache_bypass_en);
        bdmf_session_print(session, "pool2_search_mode = %u (0x%x)\n", pool2_search_mode, pool2_search_mode);
        bdmf_session_print(session, "pool2_cache_bypass_en = %u (0x%x)\n", pool2_cache_bypass_en, pool2_cache_bypass_en);
        break;
    }
    case cli_fpm_pool3_search3:
    {
        bdmf_boolean pool3_search_mode;
        bdmf_boolean pool3_cache_bypass_en;
        err = ag_drv_fpm_pool3_search3_get(&pool3_search_mode, &pool3_cache_bypass_en);
        bdmf_session_print(session, "pool3_search_mode = %u (0x%x)\n", pool3_search_mode, pool3_search_mode);
        bdmf_session_print(session, "pool3_cache_bypass_en = %u (0x%x)\n", pool3_cache_bypass_en, pool3_cache_bypass_en);
        break;
    }
    case cli_fpm_pool4_search4:
    {
        bdmf_boolean pool4_search_mode;
        bdmf_boolean pool4_cache_bypass_en;
        err = ag_drv_fpm_pool4_search4_get(&pool4_search_mode, &pool4_cache_bypass_en);
        bdmf_session_print(session, "pool4_search_mode = %u (0x%x)\n", pool4_search_mode, pool4_search_mode);
        bdmf_session_print(session, "pool4_cache_bypass_en = %u (0x%x)\n", pool4_cache_bypass_en, pool4_cache_bypass_en);
        break;
    }
    case cli_fpm_ddr0_weight:
    {
        uint8_t ddr0_alloc_weight;
        uint8_t ddr0_free_weight;
        err = ag_drv_fpm_ddr0_weight_get(&ddr0_alloc_weight, &ddr0_free_weight);
        bdmf_session_print(session, "ddr0_alloc_weight = %u (0x%x)\n", ddr0_alloc_weight, ddr0_alloc_weight);
        bdmf_session_print(session, "ddr0_free_weight = %u (0x%x)\n", ddr0_free_weight, ddr0_free_weight);
        break;
    }
    case cli_fpm_ddr1_weight:
    {
        uint8_t ddr1_alloc_weight;
        uint8_t ddr1_free_weight;
        err = ag_drv_fpm_ddr1_weight_get(&ddr1_alloc_weight, &ddr1_free_weight);
        bdmf_session_print(session, "ddr1_alloc_weight = %u (0x%x)\n", ddr1_alloc_weight, ddr1_alloc_weight);
        bdmf_session_print(session, "ddr1_free_weight = %u (0x%x)\n", ddr1_free_weight, ddr1_free_weight);
        break;
    }
    case cli_fpm_pool_cfg:
    {
        fpm_pool_cfg pool_cfg;
        err = ag_drv_fpm_pool_cfg_get(&pool_cfg);
        bdmf_session_print(session, "fp_buf_size = %u (0x%x)\n", pool_cfg.fp_buf_size, pool_cfg.fp_buf_size);
        bdmf_session_print(session, "pool_base_address = %u (0x%x)\n", pool_cfg.pool_base_address, pool_cfg.pool_base_address);
        bdmf_session_print(session, "pool_base_address_pool2 = %u (0x%x)\n", pool_cfg.pool_base_address_pool2, pool_cfg.pool_base_address_pool2);
        break;
    }
    case cli_fpm_pool_stat:
    {
        fpm_pool_stat pool_stat;
        err = ag_drv_fpm_pool_stat_get(&pool_stat);
        bdmf_session_print(session, "ovrfl = %u (0x%x)\n", pool_stat.ovrfl, pool_stat.ovrfl);
        bdmf_session_print(session, "undrfl = %u (0x%x)\n", pool_stat.undrfl, pool_stat.undrfl);
        bdmf_session_print(session, "pool_full = %u (0x%x)\n", pool_stat.pool_full, pool_stat.pool_full);
        bdmf_session_print(session, "free_fifo_full = %u (0x%x)\n", pool_stat.free_fifo_full, pool_stat.free_fifo_full);
        bdmf_session_print(session, "free_fifo_empty = %u (0x%x)\n", pool_stat.free_fifo_empty, pool_stat.free_fifo_empty);
        bdmf_session_print(session, "alloc_fifo_full = %u (0x%x)\n", pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_full);
        bdmf_session_print(session, "alloc_fifo_empty = %u (0x%x)\n", pool_stat.alloc_fifo_empty, pool_stat.alloc_fifo_empty);
        bdmf_session_print(session, "num_of_tokens_available = %u (0x%x)\n", pool_stat.num_of_tokens_available, pool_stat.num_of_tokens_available);
        bdmf_session_print(session, "num_of_not_valid_token_frees = %u (0x%x)\n", pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_frees);
        bdmf_session_print(session, "num_of_not_valid_token_multi = %u (0x%x)\n", pool_stat.num_of_not_valid_token_multi, pool_stat.num_of_not_valid_token_multi);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token_valid = %u (0x%x)\n", pool_stat.mem_corrupt_sts_related_alloc_token_valid, pool_stat.mem_corrupt_sts_related_alloc_token_valid);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u (0x%x)\n", pool_stat.mem_corrupt_sts_related_alloc_token, pool_stat.mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "invalid_free_token_valid = %u (0x%x)\n", pool_stat.invalid_free_token_valid, pool_stat.invalid_free_token_valid);
        bdmf_session_print(session, "invalid_free_token = %u (0x%x)\n", pool_stat.invalid_free_token, pool_stat.invalid_free_token);
        bdmf_session_print(session, "invalid_mcast_token_valid = %u (0x%x)\n", pool_stat.invalid_mcast_token_valid, pool_stat.invalid_mcast_token_valid);
        bdmf_session_print(session, "invalid_mcast_token = %u (0x%x)\n", pool_stat.invalid_mcast_token, pool_stat.invalid_mcast_token);
        bdmf_session_print(session, "tokens_available_low_wtmk = %u (0x%x)\n", pool_stat.tokens_available_low_wtmk, pool_stat.tokens_available_low_wtmk);
        break;
    }
    case cli_fpm_pool2_stat:
    {
        fpm_pool_stat pool_stat;
        err = ag_drv_fpm_pool2_stat_get(&pool_stat);
        bdmf_session_print(session, "ovrfl = %u (0x%x)\n", pool_stat.ovrfl, pool_stat.ovrfl);
        bdmf_session_print(session, "undrfl = %u (0x%x)\n", pool_stat.undrfl, pool_stat.undrfl);
        bdmf_session_print(session, "pool_full = %u (0x%x)\n", pool_stat.pool_full, pool_stat.pool_full);
        bdmf_session_print(session, "free_fifo_full = %u (0x%x)\n", pool_stat.free_fifo_full, pool_stat.free_fifo_full);
        bdmf_session_print(session, "free_fifo_empty = %u (0x%x)\n", pool_stat.free_fifo_empty, pool_stat.free_fifo_empty);
        bdmf_session_print(session, "alloc_fifo_full = %u (0x%x)\n", pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_full);
        bdmf_session_print(session, "alloc_fifo_empty = %u (0x%x)\n", pool_stat.alloc_fifo_empty, pool_stat.alloc_fifo_empty);
        bdmf_session_print(session, "num_of_tokens_available = %u (0x%x)\n", pool_stat.num_of_tokens_available, pool_stat.num_of_tokens_available);
        bdmf_session_print(session, "num_of_not_valid_token_frees = %u (0x%x)\n", pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_frees);
        bdmf_session_print(session, "num_of_not_valid_token_multi = %u (0x%x)\n", pool_stat.num_of_not_valid_token_multi, pool_stat.num_of_not_valid_token_multi);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token_valid = %u (0x%x)\n", pool_stat.mem_corrupt_sts_related_alloc_token_valid, pool_stat.mem_corrupt_sts_related_alloc_token_valid);
        bdmf_session_print(session, "mem_corrupt_sts_related_alloc_token = %u (0x%x)\n", pool_stat.mem_corrupt_sts_related_alloc_token, pool_stat.mem_corrupt_sts_related_alloc_token);
        bdmf_session_print(session, "invalid_free_token_valid = %u (0x%x)\n", pool_stat.invalid_free_token_valid, pool_stat.invalid_free_token_valid);
        bdmf_session_print(session, "invalid_free_token = %u (0x%x)\n", pool_stat.invalid_free_token, pool_stat.invalid_free_token);
        bdmf_session_print(session, "invalid_mcast_token_valid = %u (0x%x)\n", pool_stat.invalid_mcast_token_valid, pool_stat.invalid_mcast_token_valid);
        bdmf_session_print(session, "invalid_mcast_token = %u (0x%x)\n", pool_stat.invalid_mcast_token, pool_stat.invalid_mcast_token);
        bdmf_session_print(session, "tokens_available_low_wtmk = %u (0x%x)\n", pool_stat.tokens_available_low_wtmk, pool_stat.tokens_available_low_wtmk);
        break;
    }
    case cli_fpm_back_door_mem:
    {
        uint32_t mem_data1;
        uint32_t mem_data2;
        err = ag_drv_fpm_back_door_mem_get(&mem_data1, &mem_data2);
        bdmf_session_print(session, "mem_data1 = %u (0x%x)\n", mem_data1, mem_data1);
        bdmf_session_print(session, "mem_data2 = %u (0x%x)\n", mem_data2, mem_data2);
        break;
    }
    case cli_fpm_pool1_count:
    {
        uint32_t expired_count;
        uint32_t recovered_count;
        err = ag_drv_fpm_pool1_count_get(&expired_count, &recovered_count);
        bdmf_session_print(session, "expired_count = %u (0x%x)\n", expired_count, expired_count);
        bdmf_session_print(session, "recovered_count = %u (0x%x)\n", recovered_count, recovered_count);
        break;
    }
    case cli_fpm_pool2_count:
    {
        uint32_t expired_count;
        uint32_t recovered_count;
        err = ag_drv_fpm_pool2_count_get(&expired_count, &recovered_count);
        bdmf_session_print(session, "expired_count = %u (0x%x)\n", expired_count, expired_count);
        bdmf_session_print(session, "recovered_count = %u (0x%x)\n", recovered_count, recovered_count);
        break;
    }
    case cli_fpm_timer:
    {
        fpm_timer timer;
        err = ag_drv_fpm_timer_get(&timer);
        bdmf_session_print(session, "long_aging_timer = %u (0x%x)\n", timer.long_aging_timer, timer.long_aging_timer);
        bdmf_session_print(session, "short_aging_timer = %u (0x%x)\n", timer.short_aging_timer, timer.short_aging_timer);
        bdmf_session_print(session, "recycle_timer = %u (0x%x)\n", timer.recycle_timer, timer.recycle_timer);
        break;
    }
    case cli_fpm_fpm_bb_cfg:
    {
        uint8_t bb_ddr_sel;
        err = ag_drv_fpm_fpm_bb_cfg_get(&bb_ddr_sel);
        bdmf_session_print(session, "bb_ddr_sel = %u (0x%x)\n", bb_ddr_sel, bb_ddr_sel);
        break;
    }
    case cli_fpm_pool1_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk;
        err = ag_drv_fpm_pool1_intr_msk_get(&pool2_intr_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u (0x%x)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_recov_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u (0x%x)\n", pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u (0x%x)\n", pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u (0x%x)\n", pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "xon_msk = %u (0x%x)\n", pool2_intr_msk.xon_msk, pool2_intr_msk.xon_msk);
        bdmf_session_print(session, "xoff_msk = %u (0x%x)\n", pool2_intr_msk.xoff_msk, pool2_intr_msk.xoff_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u (0x%x)\n", pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u (0x%x)\n", pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u (0x%x)\n", pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u (0x%x)\n", pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u (0x%x)\n", pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u (0x%x)\n", pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "pool_full_msk = %u (0x%x)\n", pool2_intr_msk.pool_full_msk, pool2_intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u (0x%x)\n", pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u (0x%x)\n", pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        break;
    }
    case cli_fpm_pool1_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts;
        err = ag_drv_fpm_pool1_intr_sts_get(&pool2_intr_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u (0x%x)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_recov_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u (0x%x)\n", pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u (0x%x)\n", pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u (0x%x)\n", pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "xon_state_sts = %u (0x%x)\n", pool2_intr_sts.xon_state_sts, pool2_intr_sts.xon_state_sts);
        bdmf_session_print(session, "xoff_state_sts = %u (0x%x)\n", pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xoff_state_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u (0x%x)\n", pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u (0x%x)\n", pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u (0x%x)\n", pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u (0x%x)\n", pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u (0x%x)\n", pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u (0x%x)\n", pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "pool_full_sts = %u (0x%x)\n", pool2_intr_sts.pool_full_sts, pool2_intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u (0x%x)\n", pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u (0x%x)\n", pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        break;
    }
    case cli_fpm_pool1_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk;
        err = ag_drv_fpm_pool1_stall_msk_get(&pool2_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u (0x%x)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.memory_corrupt_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u (0x%x)\n", pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u (0x%x)\n", pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u (0x%x)\n", pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u (0x%x)\n", pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        break;
    }
    case cli_fpm_pool2_intr_msk:
    {
        fpm_pool2_intr_msk pool2_intr_msk;
        err = ag_drv_fpm_pool2_intr_msk_get(&pool2_intr_msk);
        bdmf_session_print(session, "expired_token_recov_msk = %u (0x%x)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_recov_msk);
        bdmf_session_print(session, "expired_token_det_msk = %u (0x%x)\n", pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.expired_token_det_msk);
        bdmf_session_print(session, "illegal_alloc_request_msk = %u (0x%x)\n", pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_alloc_request_msk);
        bdmf_session_print(session, "illegal_address_access_msk = %u (0x%x)\n", pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.illegal_address_access_msk);
        bdmf_session_print(session, "xon_msk = %u (0x%x)\n", pool2_intr_msk.xon_msk, pool2_intr_msk.xon_msk);
        bdmf_session_print(session, "xoff_msk = %u (0x%x)\n", pool2_intr_msk.xoff_msk, pool2_intr_msk.xoff_msk);
        bdmf_session_print(session, "memory_corrupt_msk = %u (0x%x)\n", pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.memory_corrupt_msk);
        bdmf_session_print(session, "pool_dis_free_multi_msk = %u (0x%x)\n", pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.pool_dis_free_multi_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_msk = %u (0x%x)\n", pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_index_out_of_range_msk);
        bdmf_session_print(session, "multi_token_no_valid_msk = %u (0x%x)\n", pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.multi_token_no_valid_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_msk = %u (0x%x)\n", pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_index_out_of_range_msk);
        bdmf_session_print(session, "free_token_no_valid_msk = %u (0x%x)\n", pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.free_token_no_valid_msk);
        bdmf_session_print(session, "pool_full_msk = %u (0x%x)\n", pool2_intr_msk.pool_full_msk, pool2_intr_msk.pool_full_msk);
        bdmf_session_print(session, "free_fifo_full_msk = %u (0x%x)\n", pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.free_fifo_full_msk);
        bdmf_session_print(session, "alloc_fifo_full_msk = %u (0x%x)\n", pool2_intr_msk.alloc_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        break;
    }
    case cli_fpm_pool2_intr_sts:
    {
        fpm_pool2_intr_sts pool2_intr_sts;
        err = ag_drv_fpm_pool2_intr_sts_get(&pool2_intr_sts);
        bdmf_session_print(session, "expired_token_recov_sts = %u (0x%x)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_recov_sts);
        bdmf_session_print(session, "expired_token_det_sts = %u (0x%x)\n", pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.expired_token_det_sts);
        bdmf_session_print(session, "illegal_alloc_request_sts = %u (0x%x)\n", pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_alloc_request_sts);
        bdmf_session_print(session, "illegal_address_access_sts = %u (0x%x)\n", pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.illegal_address_access_sts);
        bdmf_session_print(session, "xon_state_sts = %u (0x%x)\n", pool2_intr_sts.xon_state_sts, pool2_intr_sts.xon_state_sts);
        bdmf_session_print(session, "xoff_state_sts = %u (0x%x)\n", pool2_intr_sts.xoff_state_sts, pool2_intr_sts.xoff_state_sts);
        bdmf_session_print(session, "memory_corrupt_sts = %u (0x%x)\n", pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.memory_corrupt_sts);
        bdmf_session_print(session, "pool_dis_free_multi_sts = %u (0x%x)\n", pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.pool_dis_free_multi_sts);
        bdmf_session_print(session, "multi_token_index_out_of_range_sts = %u (0x%x)\n", pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_index_out_of_range_sts);
        bdmf_session_print(session, "multi_token_no_valid_sts = %u (0x%x)\n", pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.multi_token_no_valid_sts);
        bdmf_session_print(session, "free_token_index_out_of_range_sts = %u (0x%x)\n", pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_index_out_of_range_sts);
        bdmf_session_print(session, "free_token_no_valid_sts = %u (0x%x)\n", pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.free_token_no_valid_sts);
        bdmf_session_print(session, "pool_full_sts = %u (0x%x)\n", pool2_intr_sts.pool_full_sts, pool2_intr_sts.pool_full_sts);
        bdmf_session_print(session, "free_fifo_full_sts = %u (0x%x)\n", pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.free_fifo_full_sts);
        bdmf_session_print(session, "alloc_fifo_full_sts = %u (0x%x)\n", pool2_intr_sts.alloc_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        break;
    }
    case cli_fpm_pool2_stall_msk:
    {
        fpm_pool2_stall_msk pool2_stall_msk;
        err = ag_drv_fpm_pool2_stall_msk_get(&pool2_stall_msk);
        bdmf_session_print(session, "memory_corrupt_stall_msk = %u (0x%x)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.memory_corrupt_stall_msk);
        bdmf_session_print(session, "multi_token_index_out_of_range_stall_msk = %u (0x%x)\n", pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "multi_token_no_valid_stall_msk = %u (0x%x)\n", pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk);
        bdmf_session_print(session, "free_token_index_out_of_range_stall_msk = %u (0x%x)\n", pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk);
        bdmf_session_print(session, "free_token_no_valid_stall_msk = %u (0x%x)\n", pool2_stall_msk.free_token_no_valid_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        break;
    }
    case cli_fpm_pool1_xon_xoff_cfg:
    {
        uint16_t xon_threshold;
        uint16_t xoff_threshold;
        err = ag_drv_fpm_pool1_xon_xoff_cfg_get(&xon_threshold, &xoff_threshold);
        bdmf_session_print(session, "xon_threshold = %u (0x%x)\n", xon_threshold, xon_threshold);
        bdmf_session_print(session, "xoff_threshold = %u (0x%x)\n", xoff_threshold, xoff_threshold);
        break;
    }
    case cli_fpm_fpm_not_empty_cfg:
    {
        uint8_t not_empty_threshold;
        err = ag_drv_fpm_fpm_not_empty_cfg_get(&not_empty_threshold);
        bdmf_session_print(session, "not_empty_threshold = %u (0x%x)\n", not_empty_threshold, not_empty_threshold);
        break;
    }
    case cli_fpm_mem_ctl:
    {
        bdmf_boolean mem_wr;
        bdmf_boolean mem_rd;
        uint8_t mem_sel;
        uint16_t mem_addr;
        err = ag_drv_fpm_mem_ctl_get(&mem_wr, &mem_rd, &mem_sel, &mem_addr);
        bdmf_session_print(session, "mem_wr = %u (0x%x)\n", mem_wr, mem_wr);
        bdmf_session_print(session, "mem_rd = %u (0x%x)\n", mem_rd, mem_rd);
        bdmf_session_print(session, "mem_sel = %u (0x%x)\n", mem_sel, mem_sel);
        bdmf_session_print(session, "mem_addr = %u (0x%x)\n", mem_addr, mem_addr);
        break;
    }
    case cli_fpm_token_recover_ctl:
    {
        fpm_token_recover_ctl token_recover_ctl;
        err = ag_drv_fpm_token_recover_ctl_get(&token_recover_ctl);
        bdmf_session_print(session, "clr_recovered_token_count = %u (0x%x)\n", token_recover_ctl.clr_recovered_token_count, token_recover_ctl.clr_recovered_token_count);
        bdmf_session_print(session, "clr_expired_token_count = %u (0x%x)\n", token_recover_ctl.clr_expired_token_count, token_recover_ctl.clr_expired_token_count);
        bdmf_session_print(session, "force_token_reclaim = %u (0x%x)\n", token_recover_ctl.force_token_reclaim, token_recover_ctl.force_token_reclaim);
        bdmf_session_print(session, "token_reclaim_ena = %u (0x%x)\n", token_recover_ctl.token_reclaim_ena, token_recover_ctl.token_reclaim_ena);
        bdmf_session_print(session, "token_remark_ena = %u (0x%x)\n", token_recover_ctl.token_remark_ena, token_recover_ctl.token_remark_ena);
        bdmf_session_print(session, "single_pass_ena = %u (0x%x)\n", token_recover_ctl.single_pass_ena, token_recover_ctl.single_pass_ena);
        bdmf_session_print(session, "token_recover_ena = %u (0x%x)\n", token_recover_ctl.token_recover_ena, token_recover_ctl.token_recover_ena);
        break;
    }
    case cli_fpm_token_recover_start_end_pool1:
    {
        uint16_t start_index;
        uint16_t end_index;
        err = ag_drv_fpm_token_recover_start_end_pool1_get(&start_index, &end_index);
        bdmf_session_print(session, "start_index = %u (0x%x)\n", start_index, start_index);
        bdmf_session_print(session, "end_index = %u (0x%x)\n", end_index, end_index);
        break;
    }
    case cli_fpm_token_recover_start_end_pool2:
    {
        uint16_t start_index;
        uint16_t end_index;
        err = ag_drv_fpm_token_recover_start_end_pool2_get(&start_index, &end_index);
        bdmf_session_print(session, "start_index = %u (0x%x)\n", start_index, start_index);
        bdmf_session_print(session, "end_index = %u (0x%x)\n", end_index, end_index);
        break;
    }
    case cli_fpm_pool1_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        err = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u (0x%x)\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u (0x%x)\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u (0x%x)\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u (0x%x)\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool2_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        err = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u (0x%x)\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u (0x%x)\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u (0x%x)\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u (0x%x)\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool3_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        err = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u (0x%x)\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u (0x%x)\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u (0x%x)\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u (0x%x)\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool4_alloc_dealloc:
    {
        bdmf_boolean token_valid;
        bdmf_boolean ddr;
        uint32_t token_index;
        uint16_t token_size;
        err = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr, &token_index, &token_size);
        bdmf_session_print(session, "token_valid = %u (0x%x)\n", token_valid, token_valid);
        bdmf_session_print(session, "ddr = %u (0x%x)\n", ddr, ddr);
        bdmf_session_print(session, "token_index = %u (0x%x)\n", token_index, token_index);
        bdmf_session_print(session, "token_size = %u (0x%x)\n", token_size, token_size);
        break;
    }
    case cli_fpm_pool_multi:
    {
        fpm_pool_multi pool_multi;
        err = ag_drv_fpm_pool_multi_get(&pool_multi);
        bdmf_session_print(session, "token_valid = %u (0x%x)\n", pool_multi.token_valid, pool_multi.token_valid);
        bdmf_session_print(session, "ddr = %u (0x%x)\n", pool_multi.ddr, pool_multi.ddr);
        bdmf_session_print(session, "token_index = %u (0x%x)\n", pool_multi.token_index, pool_multi.token_index);
        bdmf_session_print(session, "update_type = %u (0x%x)\n", pool_multi.update_type, pool_multi.update_type);
        bdmf_session_print(session, "token_multi = %u (0x%x)\n", pool_multi.token_multi, pool_multi.token_multi);
        break;
    }
    case cli_fpm_fpm_bb_force:
    {
        bdmf_boolean force;
        err = ag_drv_fpm_fpm_bb_force_get(&force);
        bdmf_session_print(session, "force = %u (0x%x)\n", force, force);
        break;
    }
    case cli_fpm_fpm_bb_forced_ctrl:
    {
        uint16_t ctrl;
        err = ag_drv_fpm_fpm_bb_forced_ctrl_get(&ctrl);
        bdmf_session_print(session, "ctrl = %u (0x%x)\n", ctrl, ctrl);
        break;
    }
    case cli_fpm_fpm_bb_forced_addr:
    {
        uint16_t ta_addr;
        uint8_t dest_addr;
        err = ag_drv_fpm_fpm_bb_forced_addr_get(&ta_addr, &dest_addr);
        bdmf_session_print(session, "ta_addr = %u (0x%x)\n", ta_addr, ta_addr);
        bdmf_session_print(session, "dest_addr = %u (0x%x)\n", dest_addr, dest_addr);
        break;
    }
    case cli_fpm_fpm_bb_forced_data:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_forced_data_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_decode_cfg:
    {
        uint8_t dest_id;
        bdmf_boolean override_en;
        uint16_t route_addr;
        err = ag_drv_fpm_fpm_bb_decode_cfg_get(&dest_id, &override_en, &route_addr);
        bdmf_session_print(session, "dest_id = %u (0x%x)\n", dest_id, dest_id);
        bdmf_session_print(session, "override_en = %u (0x%x)\n", override_en, override_en);
        bdmf_session_print(session, "route_addr = %u (0x%x)\n", route_addr, route_addr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_cfg:
    {
        uint8_t rxfifo_sw_addr;
        uint8_t txfifo_sw_addr;
        bdmf_boolean rxfifo_sw_rst;
        bdmf_boolean txfifo_sw_rst;
        err = ag_drv_fpm_fpm_bb_dbg_cfg_get(&rxfifo_sw_addr, &txfifo_sw_addr, &rxfifo_sw_rst, &txfifo_sw_rst);
        bdmf_session_print(session, "rxfifo_sw_addr = %u (0x%x)\n", rxfifo_sw_addr, rxfifo_sw_addr);
        bdmf_session_print(session, "txfifo_sw_addr = %u (0x%x)\n", txfifo_sw_addr, txfifo_sw_addr);
        bdmf_session_print(session, "rxfifo_sw_rst = %u (0x%x)\n", rxfifo_sw_rst, rxfifo_sw_rst);
        bdmf_session_print(session, "txfifo_sw_rst = %u (0x%x)\n", txfifo_sw_rst, txfifo_sw_rst);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_sts:
    {
        fpm_fpm_bb_dbg_rxfifo_sts fpm_bb_dbg_rxfifo_sts;
        err = ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(&fpm_bb_dbg_rxfifo_sts);
        bdmf_session_print(session, "fifo_empty = %u (0x%x)\n", fpm_bb_dbg_rxfifo_sts.fifo_empty, fpm_bb_dbg_rxfifo_sts.fifo_empty);
        bdmf_session_print(session, "fifo_full = %u (0x%x)\n", fpm_bb_dbg_rxfifo_sts.fifo_full, fpm_bb_dbg_rxfifo_sts.fifo_full);
        bdmf_session_print(session, "fifo_used_words = %u (0x%x)\n", fpm_bb_dbg_rxfifo_sts.fifo_used_words, fpm_bb_dbg_rxfifo_sts.fifo_used_words);
        bdmf_session_print(session, "fifo_rd_cntr = %u (0x%x)\n", fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr, fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr);
        bdmf_session_print(session, "fifo_wr_cntr = %u (0x%x)\n", fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr, fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_sts:
    {
        fpm_fpm_bb_dbg_txfifo_sts fpm_bb_dbg_txfifo_sts;
        err = ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(&fpm_bb_dbg_txfifo_sts);
        bdmf_session_print(session, "fifo_empty = %u (0x%x)\n", fpm_bb_dbg_txfifo_sts.fifo_empty, fpm_bb_dbg_txfifo_sts.fifo_empty);
        bdmf_session_print(session, "fifo_full = %u (0x%x)\n", fpm_bb_dbg_txfifo_sts.fifo_full, fpm_bb_dbg_txfifo_sts.fifo_full);
        bdmf_session_print(session, "fifo_used_words = %u (0x%x)\n", fpm_bb_dbg_txfifo_sts.fifo_used_words, fpm_bb_dbg_txfifo_sts.fifo_used_words);
        bdmf_session_print(session, "fifo_rd_cntr = %u (0x%x)\n", fpm_bb_dbg_txfifo_sts.fifo_rd_cntr, fpm_bb_dbg_txfifo_sts.fifo_rd_cntr);
        bdmf_session_print(session, "fifo_wr_cntr = %u (0x%x)\n", fpm_bb_dbg_txfifo_sts.fifo_wr_cntr, fpm_bb_dbg_txfifo_sts.fifo_wr_cntr);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_data1:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_rxfifo_data2:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data1:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data2:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_fpm_fpm_bb_dbg_txfifo_data3:
    {
        uint32_t data;
        err = ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_fpm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean init_mem=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_init_mem_set( %u)\n", init_mem);
        if(!err) ag_drv_fpm_init_mem_set(init_mem);
        if(!err) ag_drv_fpm_init_mem_get( &init_mem);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_init_mem_get( %u)\n", init_mem);
        if(err || init_mem!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pool1_enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_en_set( %u)\n", pool1_enable);
        if(!err) ag_drv_fpm_pool1_en_set(pool1_enable);
        if(!err) ag_drv_fpm_pool1_en_get( &pool1_enable);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_en_get( %u)\n", pool1_enable);
        if(err || pool1_enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean fpm_bb_soft_reset=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_bb_reset_set( %u)\n", fpm_bb_soft_reset);
        if(!err) ag_drv_fpm_bb_reset_set(fpm_bb_soft_reset);
        if(!err) ag_drv_fpm_bb_reset_get( &fpm_bb_soft_reset);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_bb_reset_get( %u)\n", fpm_bb_soft_reset);
        if(err || fpm_bb_soft_reset!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pool1_search_mode=gtmv(m, 1);
        bdmf_boolean pool1_cache_bypass_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_search_set( %u %u)\n", pool1_search_mode, pool1_cache_bypass_en);
        if(!err) ag_drv_fpm_pool_search_set(pool1_search_mode, pool1_cache_bypass_en);
        if(!err) ag_drv_fpm_pool_search_get( &pool1_search_mode, &pool1_cache_bypass_en);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_search_get( %u %u)\n", pool1_search_mode, pool1_cache_bypass_en);
        if(err || pool1_search_mode!=gtmv(m, 1) || pool1_cache_bypass_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pool2_search_mode=gtmv(m, 1);
        bdmf_boolean pool2_cache_bypass_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_search2_set( %u %u)\n", pool2_search_mode, pool2_cache_bypass_en);
        if(!err) ag_drv_fpm_pool2_search2_set(pool2_search_mode, pool2_cache_bypass_en);
        if(!err) ag_drv_fpm_pool2_search2_get( &pool2_search_mode, &pool2_cache_bypass_en);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_search2_get( %u %u)\n", pool2_search_mode, pool2_cache_bypass_en);
        if(err || pool2_search_mode!=gtmv(m, 1) || pool2_cache_bypass_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pool3_search_mode=gtmv(m, 1);
        bdmf_boolean pool3_cache_bypass_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool3_search3_set( %u %u)\n", pool3_search_mode, pool3_cache_bypass_en);
        if(!err) ag_drv_fpm_pool3_search3_set(pool3_search_mode, pool3_cache_bypass_en);
        if(!err) ag_drv_fpm_pool3_search3_get( &pool3_search_mode, &pool3_cache_bypass_en);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool3_search3_get( %u %u)\n", pool3_search_mode, pool3_cache_bypass_en);
        if(err || pool3_search_mode!=gtmv(m, 1) || pool3_cache_bypass_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pool4_search_mode=gtmv(m, 1);
        bdmf_boolean pool4_cache_bypass_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool4_search4_set( %u %u)\n", pool4_search_mode, pool4_cache_bypass_en);
        if(!err) ag_drv_fpm_pool4_search4_set(pool4_search_mode, pool4_cache_bypass_en);
        if(!err) ag_drv_fpm_pool4_search4_get( &pool4_search_mode, &pool4_cache_bypass_en);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool4_search4_get( %u %u)\n", pool4_search_mode, pool4_cache_bypass_en);
        if(err || pool4_search_mode!=gtmv(m, 1) || pool4_cache_bypass_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t ddr0_alloc_weight=gtmv(m, 8);
        uint8_t ddr0_free_weight=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_ddr0_weight_set( %u %u)\n", ddr0_alloc_weight, ddr0_free_weight);
        if(!err) ag_drv_fpm_ddr0_weight_set(ddr0_alloc_weight, ddr0_free_weight);
        if(!err) ag_drv_fpm_ddr0_weight_get( &ddr0_alloc_weight, &ddr0_free_weight);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_ddr0_weight_get( %u %u)\n", ddr0_alloc_weight, ddr0_free_weight);
        if(err || ddr0_alloc_weight!=gtmv(m, 8) || ddr0_free_weight!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t ddr1_alloc_weight=gtmv(m, 8);
        uint8_t ddr1_free_weight=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_ddr1_weight_set( %u %u)\n", ddr1_alloc_weight, ddr1_free_weight);
        if(!err) ag_drv_fpm_ddr1_weight_set(ddr1_alloc_weight, ddr1_free_weight);
        if(!err) ag_drv_fpm_ddr1_weight_get( &ddr1_alloc_weight, &ddr1_free_weight);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_ddr1_weight_get( %u %u)\n", ddr1_alloc_weight, ddr1_free_weight);
        if(err || ddr1_alloc_weight!=gtmv(m, 8) || ddr1_free_weight!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool_cfg pool_cfg = {.fp_buf_size=gtmv(m, 3), .pool_base_address=gtmv(m, 30), .pool_base_address_pool2=gtmv(m, 30)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_cfg_set( %u %u %u)\n", pool_cfg.fp_buf_size, pool_cfg.pool_base_address, pool_cfg.pool_base_address_pool2);
        if(!err) ag_drv_fpm_pool_cfg_set(&pool_cfg);
        if(!err) ag_drv_fpm_pool_cfg_get( &pool_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_cfg_get( %u %u %u)\n", pool_cfg.fp_buf_size, pool_cfg.pool_base_address, pool_cfg.pool_base_address_pool2);
        if(err || pool_cfg.fp_buf_size!=gtmv(m, 3) || pool_cfg.pool_base_address!=gtmv(m, 30) || pool_cfg.pool_base_address_pool2!=gtmv(m, 30))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool_stat pool_stat = {.ovrfl=gtmv(m, 16), .undrfl=gtmv(m, 16), .pool_full=gtmv(m, 1), .free_fifo_full=gtmv(m, 1), .free_fifo_empty=gtmv(m, 1), .alloc_fifo_full=gtmv(m, 1), .alloc_fifo_empty=gtmv(m, 1), .num_of_tokens_available=gtmv(m, 18), .num_of_not_valid_token_frees=gtmv(m, 18), .num_of_not_valid_token_multi=gtmv(m, 18), .mem_corrupt_sts_related_alloc_token_valid=gtmv(m, 1), .mem_corrupt_sts_related_alloc_token=gtmv(m, 31), .invalid_free_token_valid=gtmv(m, 1), .invalid_free_token=gtmv(m, 31), .invalid_mcast_token_valid=gtmv(m, 1), .invalid_mcast_token=gtmv(m, 31), .tokens_available_low_wtmk=gtmv(m, 18)};
        if(!err) ag_drv_fpm_pool_stat_get( &pool_stat);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_stat_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool_stat.ovrfl, pool_stat.undrfl, pool_stat.pool_full, pool_stat.free_fifo_full, pool_stat.free_fifo_empty, pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_empty, pool_stat.num_of_tokens_available, pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_multi, pool_stat.mem_corrupt_sts_related_alloc_token_valid, pool_stat.mem_corrupt_sts_related_alloc_token, pool_stat.invalid_free_token_valid, pool_stat.invalid_free_token, pool_stat.invalid_mcast_token_valid, pool_stat.invalid_mcast_token, pool_stat.tokens_available_low_wtmk);
    }
    {
        fpm_pool_stat pool_stat = {.ovrfl=gtmv(m, 16), .undrfl=gtmv(m, 16), .pool_full=gtmv(m, 1), .free_fifo_full=gtmv(m, 1), .free_fifo_empty=gtmv(m, 1), .alloc_fifo_full=gtmv(m, 1), .alloc_fifo_empty=gtmv(m, 1), .num_of_tokens_available=gtmv(m, 18), .num_of_not_valid_token_frees=gtmv(m, 18), .num_of_not_valid_token_multi=gtmv(m, 18), .mem_corrupt_sts_related_alloc_token_valid=gtmv(m, 1), .mem_corrupt_sts_related_alloc_token=gtmv(m, 31), .invalid_free_token_valid=gtmv(m, 1), .invalid_free_token=gtmv(m, 31), .invalid_mcast_token_valid=gtmv(m, 1), .invalid_mcast_token=gtmv(m, 31), .tokens_available_low_wtmk=gtmv(m, 18)};
        if(!err) ag_drv_fpm_pool2_stat_get( &pool_stat);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_stat_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool_stat.ovrfl, pool_stat.undrfl, pool_stat.pool_full, pool_stat.free_fifo_full, pool_stat.free_fifo_empty, pool_stat.alloc_fifo_full, pool_stat.alloc_fifo_empty, pool_stat.num_of_tokens_available, pool_stat.num_of_not_valid_token_frees, pool_stat.num_of_not_valid_token_multi, pool_stat.mem_corrupt_sts_related_alloc_token_valid, pool_stat.mem_corrupt_sts_related_alloc_token, pool_stat.invalid_free_token_valid, pool_stat.invalid_free_token, pool_stat.invalid_mcast_token_valid, pool_stat.invalid_mcast_token, pool_stat.tokens_available_low_wtmk);
    }
    {
        uint32_t mem_data1=gtmv(m, 32);
        uint32_t mem_data2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_back_door_mem_set( %u %u)\n", mem_data1, mem_data2);
        if(!err) ag_drv_fpm_back_door_mem_set(mem_data1, mem_data2);
        if(!err) ag_drv_fpm_back_door_mem_get( &mem_data1, &mem_data2);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_back_door_mem_get( %u %u)\n", mem_data1, mem_data2);
        if(err || mem_data1!=gtmv(m, 32) || mem_data2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t expired_count=gtmv(m, 32);
        uint32_t recovered_count=gtmv(m, 32);
        if(!err) ag_drv_fpm_pool1_count_get( &expired_count, &recovered_count);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_count_get( %u %u)\n", expired_count, recovered_count);
    }
    {
        uint32_t expired_count=gtmv(m, 32);
        uint32_t recovered_count=gtmv(m, 32);
        if(!err) ag_drv_fpm_pool2_count_get( &expired_count, &recovered_count);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_count_get( %u %u)\n", expired_count, recovered_count);
    }
    {
        fpm_timer timer = {.long_aging_timer=gtmv(m, 32), .short_aging_timer=gtmv(m, 32), .recycle_timer=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_timer_set( %u %u %u)\n", timer.long_aging_timer, timer.short_aging_timer, timer.recycle_timer);
        if(!err) ag_drv_fpm_timer_set(&timer);
        if(!err) ag_drv_fpm_timer_get( &timer);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_timer_get( %u %u %u)\n", timer.long_aging_timer, timer.short_aging_timer, timer.recycle_timer);
        if(err || timer.long_aging_timer!=gtmv(m, 32) || timer.short_aging_timer!=gtmv(m, 32) || timer.recycle_timer!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t bb_ddr_sel=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_cfg_set( %u)\n", bb_ddr_sel);
        if(!err) ag_drv_fpm_fpm_bb_cfg_set(bb_ddr_sel);
        if(!err) ag_drv_fpm_fpm_bb_cfg_get( &bb_ddr_sel);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_cfg_get( %u)\n", bb_ddr_sel);
        if(err || bb_ddr_sel!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_intr_msk pool2_intr_msk = {.expired_token_recov_msk=gtmv(m, 1), .expired_token_det_msk=gtmv(m, 1), .illegal_alloc_request_msk=gtmv(m, 1), .illegal_address_access_msk=gtmv(m, 1), .xon_msk=gtmv(m, 1), .xoff_msk=gtmv(m, 1), .memory_corrupt_msk=gtmv(m, 1), .pool_dis_free_multi_msk=gtmv(m, 1), .multi_token_index_out_of_range_msk=gtmv(m, 1), .multi_token_no_valid_msk=gtmv(m, 1), .free_token_index_out_of_range_msk=gtmv(m, 1), .free_token_no_valid_msk=gtmv(m, 1), .pool_full_msk=gtmv(m, 1), .free_fifo_full_msk=gtmv(m, 1), .alloc_fifo_full_msk=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        if(!err) ag_drv_fpm_pool1_intr_msk_set(&pool2_intr_msk);
        if(!err) ag_drv_fpm_pool1_intr_msk_get( &pool2_intr_msk);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        if(err || pool2_intr_msk.expired_token_recov_msk!=gtmv(m, 1) || pool2_intr_msk.expired_token_det_msk!=gtmv(m, 1) || pool2_intr_msk.illegal_alloc_request_msk!=gtmv(m, 1) || pool2_intr_msk.illegal_address_access_msk!=gtmv(m, 1) || pool2_intr_msk.xon_msk!=gtmv(m, 1) || pool2_intr_msk.xoff_msk!=gtmv(m, 1) || pool2_intr_msk.memory_corrupt_msk!=gtmv(m, 1) || pool2_intr_msk.pool_dis_free_multi_msk!=gtmv(m, 1) || pool2_intr_msk.multi_token_index_out_of_range_msk!=gtmv(m, 1) || pool2_intr_msk.multi_token_no_valid_msk!=gtmv(m, 1) || pool2_intr_msk.free_token_index_out_of_range_msk!=gtmv(m, 1) || pool2_intr_msk.free_token_no_valid_msk!=gtmv(m, 1) || pool2_intr_msk.pool_full_msk!=gtmv(m, 1) || pool2_intr_msk.free_fifo_full_msk!=gtmv(m, 1) || pool2_intr_msk.alloc_fifo_full_msk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_intr_sts pool2_intr_sts = {.expired_token_recov_sts=gtmv(m, 1), .expired_token_det_sts=gtmv(m, 1), .illegal_alloc_request_sts=gtmv(m, 1), .illegal_address_access_sts=gtmv(m, 1), .xon_state_sts=gtmv(m, 1), .xoff_state_sts=gtmv(m, 1), .memory_corrupt_sts=gtmv(m, 1), .pool_dis_free_multi_sts=gtmv(m, 1), .multi_token_index_out_of_range_sts=gtmv(m, 1), .multi_token_no_valid_sts=gtmv(m, 1), .free_token_index_out_of_range_sts=gtmv(m, 1), .free_token_no_valid_sts=gtmv(m, 1), .pool_full_sts=gtmv(m, 1), .free_fifo_full_sts=gtmv(m, 1), .alloc_fifo_full_sts=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        if(!err) ag_drv_fpm_pool1_intr_sts_set(&pool2_intr_sts);
        if(!err) ag_drv_fpm_pool1_intr_sts_get( &pool2_intr_sts);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        if(err || pool2_intr_sts.expired_token_recov_sts!=gtmv(m, 1) || pool2_intr_sts.expired_token_det_sts!=gtmv(m, 1) || pool2_intr_sts.illegal_alloc_request_sts!=gtmv(m, 1) || pool2_intr_sts.illegal_address_access_sts!=gtmv(m, 1) || pool2_intr_sts.xon_state_sts!=gtmv(m, 1) || pool2_intr_sts.xoff_state_sts!=gtmv(m, 1) || pool2_intr_sts.memory_corrupt_sts!=gtmv(m, 1) || pool2_intr_sts.pool_dis_free_multi_sts!=gtmv(m, 1) || pool2_intr_sts.multi_token_index_out_of_range_sts!=gtmv(m, 1) || pool2_intr_sts.multi_token_no_valid_sts!=gtmv(m, 1) || pool2_intr_sts.free_token_index_out_of_range_sts!=gtmv(m, 1) || pool2_intr_sts.free_token_no_valid_sts!=gtmv(m, 1) || pool2_intr_sts.pool_full_sts!=gtmv(m, 1) || pool2_intr_sts.free_fifo_full_sts!=gtmv(m, 1) || pool2_intr_sts.alloc_fifo_full_sts!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_stall_msk pool2_stall_msk = {.memory_corrupt_stall_msk=gtmv(m, 1), .multi_token_index_out_of_range_stall_msk=gtmv(m, 1), .multi_token_no_valid_stall_msk=gtmv(m, 1), .free_token_index_out_of_range_stall_msk=gtmv(m, 1), .free_token_no_valid_stall_msk=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_stall_msk_set( %u %u %u %u %u)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        if(!err) ag_drv_fpm_pool1_stall_msk_set(&pool2_stall_msk);
        if(!err) ag_drv_fpm_pool1_stall_msk_get( &pool2_stall_msk);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_stall_msk_get( %u %u %u %u %u)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        if(err || pool2_stall_msk.memory_corrupt_stall_msk!=gtmv(m, 1) || pool2_stall_msk.multi_token_index_out_of_range_stall_msk!=gtmv(m, 1) || pool2_stall_msk.multi_token_no_valid_stall_msk!=gtmv(m, 1) || pool2_stall_msk.free_token_index_out_of_range_stall_msk!=gtmv(m, 1) || pool2_stall_msk.free_token_no_valid_stall_msk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_intr_msk pool2_intr_msk = {.expired_token_recov_msk=gtmv(m, 1), .expired_token_det_msk=gtmv(m, 1), .illegal_alloc_request_msk=gtmv(m, 1), .illegal_address_access_msk=gtmv(m, 1), .xon_msk=gtmv(m, 1), .xoff_msk=gtmv(m, 1), .memory_corrupt_msk=gtmv(m, 1), .pool_dis_free_multi_msk=gtmv(m, 1), .multi_token_index_out_of_range_msk=gtmv(m, 1), .multi_token_no_valid_msk=gtmv(m, 1), .free_token_index_out_of_range_msk=gtmv(m, 1), .free_token_no_valid_msk=gtmv(m, 1), .pool_full_msk=gtmv(m, 1), .free_fifo_full_msk=gtmv(m, 1), .alloc_fifo_full_msk=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_intr_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        if(!err) ag_drv_fpm_pool2_intr_msk_set(&pool2_intr_msk);
        if(!err) ag_drv_fpm_pool2_intr_msk_get( &pool2_intr_msk);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_intr_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_msk.expired_token_recov_msk, pool2_intr_msk.expired_token_det_msk, pool2_intr_msk.illegal_alloc_request_msk, pool2_intr_msk.illegal_address_access_msk, pool2_intr_msk.xon_msk, pool2_intr_msk.xoff_msk, pool2_intr_msk.memory_corrupt_msk, pool2_intr_msk.pool_dis_free_multi_msk, pool2_intr_msk.multi_token_index_out_of_range_msk, pool2_intr_msk.multi_token_no_valid_msk, pool2_intr_msk.free_token_index_out_of_range_msk, pool2_intr_msk.free_token_no_valid_msk, pool2_intr_msk.pool_full_msk, pool2_intr_msk.free_fifo_full_msk, pool2_intr_msk.alloc_fifo_full_msk);
        if(err || pool2_intr_msk.expired_token_recov_msk!=gtmv(m, 1) || pool2_intr_msk.expired_token_det_msk!=gtmv(m, 1) || pool2_intr_msk.illegal_alloc_request_msk!=gtmv(m, 1) || pool2_intr_msk.illegal_address_access_msk!=gtmv(m, 1) || pool2_intr_msk.xon_msk!=gtmv(m, 1) || pool2_intr_msk.xoff_msk!=gtmv(m, 1) || pool2_intr_msk.memory_corrupt_msk!=gtmv(m, 1) || pool2_intr_msk.pool_dis_free_multi_msk!=gtmv(m, 1) || pool2_intr_msk.multi_token_index_out_of_range_msk!=gtmv(m, 1) || pool2_intr_msk.multi_token_no_valid_msk!=gtmv(m, 1) || pool2_intr_msk.free_token_index_out_of_range_msk!=gtmv(m, 1) || pool2_intr_msk.free_token_no_valid_msk!=gtmv(m, 1) || pool2_intr_msk.pool_full_msk!=gtmv(m, 1) || pool2_intr_msk.free_fifo_full_msk!=gtmv(m, 1) || pool2_intr_msk.alloc_fifo_full_msk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_intr_sts pool2_intr_sts = {.expired_token_recov_sts=gtmv(m, 1), .expired_token_det_sts=gtmv(m, 1), .illegal_alloc_request_sts=gtmv(m, 1), .illegal_address_access_sts=gtmv(m, 1), .xon_state_sts=gtmv(m, 1), .xoff_state_sts=gtmv(m, 1), .memory_corrupt_sts=gtmv(m, 1), .pool_dis_free_multi_sts=gtmv(m, 1), .multi_token_index_out_of_range_sts=gtmv(m, 1), .multi_token_no_valid_sts=gtmv(m, 1), .free_token_index_out_of_range_sts=gtmv(m, 1), .free_token_no_valid_sts=gtmv(m, 1), .pool_full_sts=gtmv(m, 1), .free_fifo_full_sts=gtmv(m, 1), .alloc_fifo_full_sts=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_intr_sts_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        if(!err) ag_drv_fpm_pool2_intr_sts_set(&pool2_intr_sts);
        if(!err) ag_drv_fpm_pool2_intr_sts_get( &pool2_intr_sts);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_intr_sts_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pool2_intr_sts.expired_token_recov_sts, pool2_intr_sts.expired_token_det_sts, pool2_intr_sts.illegal_alloc_request_sts, pool2_intr_sts.illegal_address_access_sts, pool2_intr_sts.xon_state_sts, pool2_intr_sts.xoff_state_sts, pool2_intr_sts.memory_corrupt_sts, pool2_intr_sts.pool_dis_free_multi_sts, pool2_intr_sts.multi_token_index_out_of_range_sts, pool2_intr_sts.multi_token_no_valid_sts, pool2_intr_sts.free_token_index_out_of_range_sts, pool2_intr_sts.free_token_no_valid_sts, pool2_intr_sts.pool_full_sts, pool2_intr_sts.free_fifo_full_sts, pool2_intr_sts.alloc_fifo_full_sts);
        if(err || pool2_intr_sts.expired_token_recov_sts!=gtmv(m, 1) || pool2_intr_sts.expired_token_det_sts!=gtmv(m, 1) || pool2_intr_sts.illegal_alloc_request_sts!=gtmv(m, 1) || pool2_intr_sts.illegal_address_access_sts!=gtmv(m, 1) || pool2_intr_sts.xon_state_sts!=gtmv(m, 1) || pool2_intr_sts.xoff_state_sts!=gtmv(m, 1) || pool2_intr_sts.memory_corrupt_sts!=gtmv(m, 1) || pool2_intr_sts.pool_dis_free_multi_sts!=gtmv(m, 1) || pool2_intr_sts.multi_token_index_out_of_range_sts!=gtmv(m, 1) || pool2_intr_sts.multi_token_no_valid_sts!=gtmv(m, 1) || pool2_intr_sts.free_token_index_out_of_range_sts!=gtmv(m, 1) || pool2_intr_sts.free_token_no_valid_sts!=gtmv(m, 1) || pool2_intr_sts.pool_full_sts!=gtmv(m, 1) || pool2_intr_sts.free_fifo_full_sts!=gtmv(m, 1) || pool2_intr_sts.alloc_fifo_full_sts!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool2_stall_msk pool2_stall_msk = {.memory_corrupt_stall_msk=gtmv(m, 1), .multi_token_index_out_of_range_stall_msk=gtmv(m, 1), .multi_token_no_valid_stall_msk=gtmv(m, 1), .free_token_index_out_of_range_stall_msk=gtmv(m, 1), .free_token_no_valid_stall_msk=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_stall_msk_set( %u %u %u %u %u)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        if(!err) ag_drv_fpm_pool2_stall_msk_set(&pool2_stall_msk);
        if(!err) ag_drv_fpm_pool2_stall_msk_get( &pool2_stall_msk);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_stall_msk_get( %u %u %u %u %u)\n", pool2_stall_msk.memory_corrupt_stall_msk, pool2_stall_msk.multi_token_index_out_of_range_stall_msk, pool2_stall_msk.multi_token_no_valid_stall_msk, pool2_stall_msk.free_token_index_out_of_range_stall_msk, pool2_stall_msk.free_token_no_valid_stall_msk);
        if(err || pool2_stall_msk.memory_corrupt_stall_msk!=gtmv(m, 1) || pool2_stall_msk.multi_token_index_out_of_range_stall_msk!=gtmv(m, 1) || pool2_stall_msk.multi_token_no_valid_stall_msk!=gtmv(m, 1) || pool2_stall_msk.free_token_index_out_of_range_stall_msk!=gtmv(m, 1) || pool2_stall_msk.free_token_no_valid_stall_msk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t xon_threshold=gtmv(m, 16);
        uint16_t xoff_threshold=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_xon_xoff_cfg_set( %u %u)\n", xon_threshold, xoff_threshold);
        if(!err) ag_drv_fpm_pool1_xon_xoff_cfg_set(xon_threshold, xoff_threshold);
        if(!err) ag_drv_fpm_pool1_xon_xoff_cfg_get( &xon_threshold, &xoff_threshold);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_xon_xoff_cfg_get( %u %u)\n", xon_threshold, xoff_threshold);
        if(err || xon_threshold!=gtmv(m, 16) || xoff_threshold!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t not_empty_threshold=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_not_empty_cfg_set( %u)\n", not_empty_threshold);
        if(!err) ag_drv_fpm_fpm_not_empty_cfg_set(not_empty_threshold);
        if(!err) ag_drv_fpm_fpm_not_empty_cfg_get( &not_empty_threshold);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_not_empty_cfg_get( %u)\n", not_empty_threshold);
        if(err || not_empty_threshold!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean mem_wr=gtmv(m, 1);
        bdmf_boolean mem_rd=gtmv(m, 1);
        uint8_t mem_sel=gtmv(m, 2);
        uint16_t mem_addr=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_mem_ctl_set( %u %u %u %u)\n", mem_wr, mem_rd, mem_sel, mem_addr);
        if(!err) ag_drv_fpm_mem_ctl_set(mem_wr, mem_rd, mem_sel, mem_addr);
        if(!err) ag_drv_fpm_mem_ctl_get( &mem_wr, &mem_rd, &mem_sel, &mem_addr);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_mem_ctl_get( %u %u %u %u)\n", mem_wr, mem_rd, mem_sel, mem_addr);
        if(err || mem_wr!=gtmv(m, 1) || mem_rd!=gtmv(m, 1) || mem_sel!=gtmv(m, 2) || mem_addr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_token_recover_ctl token_recover_ctl = {.clr_recovered_token_count=gtmv(m, 1), .clr_expired_token_count=gtmv(m, 1), .force_token_reclaim=gtmv(m, 1), .token_reclaim_ena=gtmv(m, 1), .token_remark_ena=gtmv(m, 1), .single_pass_ena=gtmv(m, 1), .token_recover_ena=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_ctl_set( %u %u %u %u %u %u %u)\n", token_recover_ctl.clr_recovered_token_count, token_recover_ctl.clr_expired_token_count, token_recover_ctl.force_token_reclaim, token_recover_ctl.token_reclaim_ena, token_recover_ctl.token_remark_ena, token_recover_ctl.single_pass_ena, token_recover_ctl.token_recover_ena);
        if(!err) ag_drv_fpm_token_recover_ctl_set(&token_recover_ctl);
        if(!err) ag_drv_fpm_token_recover_ctl_get( &token_recover_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_ctl_get( %u %u %u %u %u %u %u)\n", token_recover_ctl.clr_recovered_token_count, token_recover_ctl.clr_expired_token_count, token_recover_ctl.force_token_reclaim, token_recover_ctl.token_reclaim_ena, token_recover_ctl.token_remark_ena, token_recover_ctl.single_pass_ena, token_recover_ctl.token_recover_ena);
        if(err || token_recover_ctl.clr_recovered_token_count!=gtmv(m, 1) || token_recover_ctl.clr_expired_token_count!=gtmv(m, 1) || token_recover_ctl.force_token_reclaim!=gtmv(m, 1) || token_recover_ctl.token_reclaim_ena!=gtmv(m, 1) || token_recover_ctl.token_remark_ena!=gtmv(m, 1) || token_recover_ctl.single_pass_ena!=gtmv(m, 1) || token_recover_ctl.token_recover_ena!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t start_index=gtmv(m, 12);
        uint16_t end_index=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool1_set( %u %u)\n", start_index, end_index);
        if(!err) ag_drv_fpm_token_recover_start_end_pool1_set(start_index, end_index);
        if(!err) ag_drv_fpm_token_recover_start_end_pool1_get( &start_index, &end_index);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool1_get( %u %u)\n", start_index, end_index);
        if(err || start_index!=gtmv(m, 12) || end_index!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t start_index=gtmv(m, 12);
        uint16_t end_index=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool2_set( %u %u)\n", start_index, end_index);
        if(!err) ag_drv_fpm_token_recover_start_end_pool2_set(start_index, end_index);
        if(!err) ag_drv_fpm_token_recover_start_end_pool2_get( &start_index, &end_index);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_token_recover_start_end_pool2_get( %u %u)\n", start_index, end_index);
        if(err || start_index!=gtmv(m, 12) || end_index!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean token_valid=gtmv(m, 1);
        bdmf_boolean ddr=gtmv(m, 1);
        uint32_t token_index=gtmv(m, 17);
        uint16_t token_size=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_set( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool1_alloc_dealloc_get( &token_valid, &ddr, &token_index, &token_size);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool1_alloc_dealloc_get( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(err || token_valid!=gtmv(m, 1) || ddr!=gtmv(m, 1) || token_index!=gtmv(m, 17) || token_size!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean token_valid=gtmv(m, 1);
        bdmf_boolean ddr=gtmv(m, 1);
        uint32_t token_index=gtmv(m, 17);
        uint16_t token_size=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_set( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool2_alloc_dealloc_get( &token_valid, &ddr, &token_index, &token_size);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool2_alloc_dealloc_get( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(err || token_valid!=gtmv(m, 1) || ddr!=gtmv(m, 1) || token_index!=gtmv(m, 17) || token_size!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean token_valid=gtmv(m, 1);
        bdmf_boolean ddr=gtmv(m, 1);
        uint32_t token_index=gtmv(m, 17);
        uint16_t token_size=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_set( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool3_alloc_dealloc_get( &token_valid, &ddr, &token_index, &token_size);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool3_alloc_dealloc_get( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(err || token_valid!=gtmv(m, 1) || ddr!=gtmv(m, 1) || token_index!=gtmv(m, 17) || token_size!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean token_valid=gtmv(m, 1);
        bdmf_boolean ddr=gtmv(m, 1);
        uint32_t token_index=gtmv(m, 17);
        uint16_t token_size=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_set( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr, token_index, token_size);
        if(!err) ag_drv_fpm_pool4_alloc_dealloc_get( &token_valid, &ddr, &token_index, &token_size);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool4_alloc_dealloc_get( %u %u %u %u)\n", token_valid, ddr, token_index, token_size);
        if(err || token_valid!=gtmv(m, 1) || ddr!=gtmv(m, 1) || token_index!=gtmv(m, 17) || token_size!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_pool_multi pool_multi = {.token_valid=gtmv(m, 1), .ddr=gtmv(m, 1), .token_index=gtmv(m, 17), .update_type=gtmv(m, 1), .token_multi=gtmv(m, 7)};
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_multi_set( %u %u %u %u %u)\n", pool_multi.token_valid, pool_multi.ddr, pool_multi.token_index, pool_multi.update_type, pool_multi.token_multi);
        if(!err) ag_drv_fpm_pool_multi_set(&pool_multi);
        if(!err) ag_drv_fpm_pool_multi_get( &pool_multi);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_pool_multi_get( %u %u %u %u %u)\n", pool_multi.token_valid, pool_multi.ddr, pool_multi.token_index, pool_multi.update_type, pool_multi.token_multi);
        if(err || pool_multi.token_valid!=gtmv(m, 1) || pool_multi.ddr!=gtmv(m, 1) || pool_multi.token_index!=gtmv(m, 17) || pool_multi.update_type!=gtmv(m, 1) || pool_multi.token_multi!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean force=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_force_set( %u)\n", force);
        if(!err) ag_drv_fpm_fpm_bb_force_set(force);
        if(!err) ag_drv_fpm_fpm_bb_force_get( &force);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_force_get( %u)\n", force);
        if(err || force!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ctrl=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_ctrl_set( %u)\n", ctrl);
        if(!err) ag_drv_fpm_fpm_bb_forced_ctrl_set(ctrl);
        if(!err) ag_drv_fpm_fpm_bb_forced_ctrl_get( &ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_ctrl_get( %u)\n", ctrl);
        if(err || ctrl!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ta_addr=gtmv(m, 16);
        uint8_t dest_addr=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_addr_set( %u %u)\n", ta_addr, dest_addr);
        if(!err) ag_drv_fpm_fpm_bb_forced_addr_set(ta_addr, dest_addr);
        if(!err) ag_drv_fpm_fpm_bb_forced_addr_get( &ta_addr, &dest_addr);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_addr_get( %u %u)\n", ta_addr, dest_addr);
        if(err || ta_addr!=gtmv(m, 16) || dest_addr!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_data_set( %u)\n", data);
        if(!err) ag_drv_fpm_fpm_bb_forced_data_set(data);
        if(!err) ag_drv_fpm_fpm_bb_forced_data_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_forced_data_get( %u)\n", data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dest_id=gtmv(m, 6);
        bdmf_boolean override_en=gtmv(m, 1);
        uint16_t route_addr=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_decode_cfg_set( %u %u %u)\n", dest_id, override_en, route_addr);
        if(!err) ag_drv_fpm_fpm_bb_decode_cfg_set(dest_id, override_en, route_addr);
        if(!err) ag_drv_fpm_fpm_bb_decode_cfg_get( &dest_id, &override_en, &route_addr);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_decode_cfg_get( %u %u %u)\n", dest_id, override_en, route_addr);
        if(err || dest_id!=gtmv(m, 6) || override_en!=gtmv(m, 1) || route_addr!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfifo_sw_addr=gtmv(m, 4);
        uint8_t txfifo_sw_addr=gtmv(m, 4);
        bdmf_boolean rxfifo_sw_rst=gtmv(m, 1);
        bdmf_boolean txfifo_sw_rst=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_cfg_set( %u %u %u %u)\n", rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        if(!err) ag_drv_fpm_fpm_bb_dbg_cfg_set(rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        if(!err) ag_drv_fpm_fpm_bb_dbg_cfg_get( &rxfifo_sw_addr, &txfifo_sw_addr, &rxfifo_sw_rst, &txfifo_sw_rst);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_cfg_get( %u %u %u %u)\n", rxfifo_sw_addr, txfifo_sw_addr, rxfifo_sw_rst, txfifo_sw_rst);
        if(err || rxfifo_sw_addr!=gtmv(m, 4) || txfifo_sw_addr!=gtmv(m, 4) || rxfifo_sw_rst!=gtmv(m, 1) || txfifo_sw_rst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        fpm_fpm_bb_dbg_rxfifo_sts fpm_bb_dbg_rxfifo_sts = {.fifo_empty=gtmv(m, 1), .fifo_full=gtmv(m, 1), .fifo_used_words=gtmv(m, 5), .fifo_rd_cntr=gtmv(m, 5), .fifo_wr_cntr=gtmv(m, 5)};
        if(!err) ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get( &fpm_bb_dbg_rxfifo_sts);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get( %u %u %u %u %u)\n", fpm_bb_dbg_rxfifo_sts.fifo_empty, fpm_bb_dbg_rxfifo_sts.fifo_full, fpm_bb_dbg_rxfifo_sts.fifo_used_words, fpm_bb_dbg_rxfifo_sts.fifo_rd_cntr, fpm_bb_dbg_rxfifo_sts.fifo_wr_cntr);
    }
    {
        fpm_fpm_bb_dbg_txfifo_sts fpm_bb_dbg_txfifo_sts = {.fifo_empty=gtmv(m, 1), .fifo_full=gtmv(m, 1), .fifo_used_words=gtmv(m, 5), .fifo_rd_cntr=gtmv(m, 5), .fifo_wr_cntr=gtmv(m, 5)};
        if(!err) ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get( &fpm_bb_dbg_txfifo_sts);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get( %u %u %u %u %u)\n", fpm_bb_dbg_txfifo_sts.fifo_empty, fpm_bb_dbg_txfifo_sts.fifo_full, fpm_bb_dbg_txfifo_sts.fifo_used_words, fpm_bb_dbg_txfifo_sts.fifo_rd_cntr, fpm_bb_dbg_txfifo_sts.fifo_wr_cntr);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get( %u)\n", data);
    }
    return err;
}

static int bcm_fpm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_fpm_ctl : reg = &RU_REG(FPM, FPM_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_cfg1 : reg = &RU_REG(FPM, FPM_CFG1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_weight : reg = &RU_REG(FPM, FPM_WEIGHT); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_cfg : reg = &RU_REG(FPM, FPM_BB_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_intr_msk : reg = &RU_REG(FPM, POOL1_INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_intr_sts : reg = &RU_REG(FPM, POOL1_INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stall_msk : reg = &RU_REG(FPM, POOL1_STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_intr_msk : reg = &RU_REG(FPM, POOL2_INTR_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_intr_sts : reg = &RU_REG(FPM, POOL2_INTR_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stall_msk : reg = &RU_REG(FPM, POOL2_STALL_MSK); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg1 : reg = &RU_REG(FPM, POOL1_CFG1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg2 : reg = &RU_REG(FPM, POOL1_CFG2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_cfg3 : reg = &RU_REG(FPM, POOL1_CFG3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat1 : reg = &RU_REG(FPM, POOL1_STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat2 : reg = &RU_REG(FPM, POOL1_STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat3 : reg = &RU_REG(FPM, POOL1_STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat4 : reg = &RU_REG(FPM, POOL1_STAT4); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat5 : reg = &RU_REG(FPM, POOL1_STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat6 : reg = &RU_REG(FPM, POOL1_STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat7 : reg = &RU_REG(FPM, POOL1_STAT7); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_stat8 : reg = &RU_REG(FPM, POOL1_STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat1 : reg = &RU_REG(FPM, POOL2_STAT1); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat2 : reg = &RU_REG(FPM, POOL2_STAT2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat3 : reg = &RU_REG(FPM, POOL2_STAT3); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat4 : reg = &RU_REG(FPM, POOL2_STAT4); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat5 : reg = &RU_REG(FPM, POOL2_STAT5); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat6 : reg = &RU_REG(FPM, POOL2_STAT6); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat7 : reg = &RU_REG(FPM, POOL2_STAT7); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_stat8 : reg = &RU_REG(FPM, POOL2_STAT8); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_xon_xoff_cfg : reg = &RU_REG(FPM, POOL1_XON_XOFF_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_not_empty_cfg : reg = &RU_REG(FPM, FPM_NOT_EMPTY_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_ctl : reg = &RU_REG(FPM, MEM_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_data1 : reg = &RU_REG(FPM, MEM_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_mem_data2 : reg = &RU_REG(FPM, MEM_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_ctl : reg = &RU_REG(FPM, TOKEN_RECOVER_CTL); blk = &RU_BLK(FPM); break;
    case bdmf_address_short_aging_timer : reg = &RU_REG(FPM, SHORT_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_long_aging_timer : reg = &RU_REG(FPM, LONG_AGING_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_cache_recycle_timer : reg = &RU_REG(FPM, CACHE_RECYCLE_TIMER); blk = &RU_BLK(FPM); break;
    case bdmf_address_expired_token_count_pool1 : reg = &RU_REG(FPM, EXPIRED_TOKEN_COUNT_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_recovered_token_count_pool1 : reg = &RU_REG(FPM, RECOVERED_TOKEN_COUNT_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_expired_token_count_pool2 : reg = &RU_REG(FPM, EXPIRED_TOKEN_COUNT_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_recovered_token_count_pool2 : reg = &RU_REG(FPM, RECOVERED_TOKEN_COUNT_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_start_end_pool1 : reg = &RU_REG(FPM, TOKEN_RECOVER_START_END_POOL1); blk = &RU_BLK(FPM); break;
    case bdmf_address_token_recover_start_end_pool2 : reg = &RU_REG(FPM, TOKEN_RECOVER_START_END_POOL2); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool1_alloc_dealloc : reg = &RU_REG(FPM, POOL1_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool2_alloc_dealloc : reg = &RU_REG(FPM, POOL2_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool3_alloc_dealloc : reg = &RU_REG(FPM, POOL3_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool4_alloc_dealloc : reg = &RU_REG(FPM, POOL4_ALLOC_DEALLOC); blk = &RU_BLK(FPM); break;
    case bdmf_address_pool_multi : reg = &RU_REG(FPM, POOL_MULTI); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_force : reg = &RU_REG(FPM, FPM_BB_FORCE); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_ctrl : reg = &RU_REG(FPM, FPM_BB_FORCED_CTRL); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_addr : reg = &RU_REG(FPM, FPM_BB_FORCED_ADDR); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_forced_data : reg = &RU_REG(FPM, FPM_BB_FORCED_DATA); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_decode_cfg : reg = &RU_REG(FPM, FPM_BB_DECODE_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_cfg : reg = &RU_REG(FPM, FPM_BB_DBG_CFG); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_sts : reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_sts : reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_STS); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_data1 : reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_rxfifo_data2 : reg = &RU_REG(FPM, FPM_BB_DBG_RXFIFO_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data1 : reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA1); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data2 : reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA2); blk = &RU_BLK(FPM); break;
    case bdmf_address_fpm_bb_dbg_txfifo_data3 : reg = &RU_REG(FPM, FPM_BB_DBG_TXFIFO_DATA3); blk = &RU_BLK(FPM); break;
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

bdmfmon_handle_t ag_drv_fpm_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "fpm"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "fpm", "fpm", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_init_mem[]={
            BDMFMON_MAKE_PARM("init_mem", "init_mem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_en[]={
            BDMFMON_MAKE_PARM("pool1_enable", "pool1_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_reset[]={
            BDMFMON_MAKE_PARM("fpm_bb_soft_reset", "fpm_bb_soft_reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_search[]={
            BDMFMON_MAKE_PARM("pool1_search_mode", "pool1_search_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool1_cache_bypass_en", "pool1_cache_bypass_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_search2[]={
            BDMFMON_MAKE_PARM("pool2_search_mode", "pool2_search_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool2_cache_bypass_en", "pool2_cache_bypass_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_search3[]={
            BDMFMON_MAKE_PARM("pool3_search_mode", "pool3_search_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool3_cache_bypass_en", "pool3_cache_bypass_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool4_search4[]={
            BDMFMON_MAKE_PARM("pool4_search_mode", "pool4_search_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool4_cache_bypass_en", "pool4_cache_bypass_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr0_weight[]={
            BDMFMON_MAKE_PARM("ddr0_alloc_weight", "ddr0_alloc_weight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr0_free_weight", "ddr0_free_weight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr1_weight[]={
            BDMFMON_MAKE_PARM("ddr1_alloc_weight", "ddr1_alloc_weight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr1_free_weight", "ddr1_free_weight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_cfg[]={
            BDMFMON_MAKE_PARM("fp_buf_size", "fp_buf_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_base_address", "pool_base_address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_base_address_pool2", "pool_base_address_pool2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_back_door_mem[]={
            BDMFMON_MAKE_PARM("mem_data1", "mem_data1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mem_data2", "mem_data2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_timer[]={
            BDMFMON_MAKE_PARM("long_aging_timer", "long_aging_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("short_aging_timer", "short_aging_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("recycle_timer", "recycle_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_cfg[]={
            BDMFMON_MAKE_PARM("bb_ddr_sel", "bb_ddr_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_intr_msk[]={
            BDMFMON_MAKE_PARM("expired_token_recov_msk", "expired_token_recov_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_det_msk", "expired_token_det_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_alloc_request_msk", "illegal_alloc_request_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_address_access_msk", "illegal_address_access_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xon_msk", "xon_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xoff_msk", "xoff_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_msk", "memory_corrupt_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_dis_free_multi_msk", "pool_dis_free_multi_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_msk", "multi_token_index_out_of_range_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_msk", "multi_token_no_valid_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_msk", "free_token_index_out_of_range_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_no_valid_msk", "free_token_no_valid_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_full_msk", "pool_full_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_full_msk", "free_fifo_full_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_fifo_full_msk", "alloc_fifo_full_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_intr_sts[]={
            BDMFMON_MAKE_PARM("expired_token_recov_sts", "expired_token_recov_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("expired_token_det_sts", "expired_token_det_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_alloc_request_sts", "illegal_alloc_request_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("illegal_address_access_sts", "illegal_address_access_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xon_state_sts", "xon_state_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xoff_state_sts", "xoff_state_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("memory_corrupt_sts", "memory_corrupt_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_dis_free_multi_sts", "pool_dis_free_multi_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_sts", "multi_token_index_out_of_range_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_sts", "multi_token_no_valid_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_sts", "free_token_index_out_of_range_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_no_valid_sts", "free_token_no_valid_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pool_full_sts", "pool_full_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_fifo_full_sts", "free_fifo_full_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("alloc_fifo_full_sts", "alloc_fifo_full_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_stall_msk[]={
            BDMFMON_MAKE_PARM("memory_corrupt_stall_msk", "memory_corrupt_stall_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_index_out_of_range_stall_msk", "multi_token_index_out_of_range_stall_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multi_token_no_valid_stall_msk", "multi_token_no_valid_stall_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_index_out_of_range_stall_msk", "free_token_index_out_of_range_stall_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_token_no_valid_stall_msk", "free_token_no_valid_stall_msk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_xon_xoff_cfg[]={
            BDMFMON_MAKE_PARM("xon_threshold", "xon_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xoff_threshold", "xoff_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_not_empty_cfg[]={
            BDMFMON_MAKE_PARM("not_empty_threshold", "not_empty_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mem_ctl[]={
            BDMFMON_MAKE_PARM("mem_wr", "mem_wr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mem_rd", "mem_rd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mem_sel", "mem_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mem_addr", "mem_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_ctl[]={
            BDMFMON_MAKE_PARM("clr_recovered_token_count", "clr_recovered_token_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clr_expired_token_count", "clr_expired_token_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("force_token_reclaim", "force_token_reclaim", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_reclaim_ena", "token_reclaim_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_remark_ena", "token_remark_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("single_pass_ena", "single_pass_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_recover_ena", "token_recover_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_start_end_pool1[]={
            BDMFMON_MAKE_PARM("start_index", "start_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("end_index", "end_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_recover_start_end_pool2[]={
            BDMFMON_MAKE_PARM("start_index", "start_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("end_index", "end_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool1_alloc_dealloc[]={
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool2_alloc_dealloc[]={
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool3_alloc_dealloc[]={
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool4_alloc_dealloc[]={
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_size", "token_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pool_multi[]={
            BDMFMON_MAKE_PARM("token_valid", "token_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_index", "token_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("update_type", "update_type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_multi", "token_multi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_force[]={
            BDMFMON_MAKE_PARM("force", "force", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_ctrl[]={
            BDMFMON_MAKE_PARM("ctrl", "ctrl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_addr[]={
            BDMFMON_MAKE_PARM("ta_addr", "ta_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dest_addr", "dest_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_forced_data[]={
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_decode_cfg[]={
            BDMFMON_MAKE_PARM("dest_id", "dest_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("override_en", "override_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr", "route_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_bb_dbg_cfg[]={
            BDMFMON_MAKE_PARM("rxfifo_sw_addr", "rxfifo_sw_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txfifo_sw_addr", "txfifo_sw_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxfifo_sw_rst", "rxfifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txfifo_sw_rst", "txfifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="init_mem", .val=cli_fpm_init_mem, .parms=set_init_mem },
            { .name="pool1_en", .val=cli_fpm_pool1_en, .parms=set_pool1_en },
            { .name="bb_reset", .val=cli_fpm_bb_reset, .parms=set_bb_reset },
            { .name="pool_search", .val=cli_fpm_pool_search, .parms=set_pool_search },
            { .name="pool2_search2", .val=cli_fpm_pool2_search2, .parms=set_pool2_search2 },
            { .name="pool3_search3", .val=cli_fpm_pool3_search3, .parms=set_pool3_search3 },
            { .name="pool4_search4", .val=cli_fpm_pool4_search4, .parms=set_pool4_search4 },
            { .name="ddr0_weight", .val=cli_fpm_ddr0_weight, .parms=set_ddr0_weight },
            { .name="ddr1_weight", .val=cli_fpm_ddr1_weight, .parms=set_ddr1_weight },
            { .name="pool_cfg", .val=cli_fpm_pool_cfg, .parms=set_pool_cfg },
            { .name="back_door_mem", .val=cli_fpm_back_door_mem, .parms=set_back_door_mem },
            { .name="timer", .val=cli_fpm_timer, .parms=set_timer },
            { .name="fpm_bb_cfg", .val=cli_fpm_fpm_bb_cfg, .parms=set_fpm_bb_cfg },
            { .name="pool1_intr_msk", .val=cli_fpm_pool1_intr_msk, .parms=set_pool2_intr_msk },
            { .name="pool1_intr_sts", .val=cli_fpm_pool1_intr_sts, .parms=set_pool2_intr_sts },
            { .name="pool1_stall_msk", .val=cli_fpm_pool1_stall_msk, .parms=set_pool2_stall_msk },
            { .name="pool2_intr_msk", .val=cli_fpm_pool2_intr_msk, .parms=set_pool2_intr_msk },
            { .name="pool2_intr_sts", .val=cli_fpm_pool2_intr_sts, .parms=set_pool2_intr_sts },
            { .name="pool2_stall_msk", .val=cli_fpm_pool2_stall_msk, .parms=set_pool2_stall_msk },
            { .name="pool1_xon_xoff_cfg", .val=cli_fpm_pool1_xon_xoff_cfg, .parms=set_pool1_xon_xoff_cfg },
            { .name="fpm_not_empty_cfg", .val=cli_fpm_fpm_not_empty_cfg, .parms=set_fpm_not_empty_cfg },
            { .name="mem_ctl", .val=cli_fpm_mem_ctl, .parms=set_mem_ctl },
            { .name="token_recover_ctl", .val=cli_fpm_token_recover_ctl, .parms=set_token_recover_ctl },
            { .name="token_recover_start_end_pool1", .val=cli_fpm_token_recover_start_end_pool1, .parms=set_token_recover_start_end_pool1 },
            { .name="token_recover_start_end_pool2", .val=cli_fpm_token_recover_start_end_pool2, .parms=set_token_recover_start_end_pool2 },
            { .name="pool1_alloc_dealloc", .val=cli_fpm_pool1_alloc_dealloc, .parms=set_pool1_alloc_dealloc },
            { .name="pool2_alloc_dealloc", .val=cli_fpm_pool2_alloc_dealloc, .parms=set_pool2_alloc_dealloc },
            { .name="pool3_alloc_dealloc", .val=cli_fpm_pool3_alloc_dealloc, .parms=set_pool3_alloc_dealloc },
            { .name="pool4_alloc_dealloc", .val=cli_fpm_pool4_alloc_dealloc, .parms=set_pool4_alloc_dealloc },
            { .name="pool_multi", .val=cli_fpm_pool_multi, .parms=set_pool_multi },
            { .name="fpm_bb_force", .val=cli_fpm_fpm_bb_force, .parms=set_fpm_bb_force },
            { .name="fpm_bb_forced_ctrl", .val=cli_fpm_fpm_bb_forced_ctrl, .parms=set_fpm_bb_forced_ctrl },
            { .name="fpm_bb_forced_addr", .val=cli_fpm_fpm_bb_forced_addr, .parms=set_fpm_bb_forced_addr },
            { .name="fpm_bb_forced_data", .val=cli_fpm_fpm_bb_forced_data, .parms=set_fpm_bb_forced_data },
            { .name="fpm_bb_decode_cfg", .val=cli_fpm_fpm_bb_decode_cfg, .parms=set_fpm_bb_decode_cfg },
            { .name="fpm_bb_dbg_cfg", .val=cli_fpm_fpm_bb_dbg_cfg, .parms=set_fpm_bb_dbg_cfg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_fpm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="init_mem", .val=cli_fpm_init_mem, .parms=set_default },
            { .name="pool1_en", .val=cli_fpm_pool1_en, .parms=set_default },
            { .name="bb_reset", .val=cli_fpm_bb_reset, .parms=set_default },
            { .name="pool_search", .val=cli_fpm_pool_search, .parms=set_default },
            { .name="pool2_search2", .val=cli_fpm_pool2_search2, .parms=set_default },
            { .name="pool3_search3", .val=cli_fpm_pool3_search3, .parms=set_default },
            { .name="pool4_search4", .val=cli_fpm_pool4_search4, .parms=set_default },
            { .name="ddr0_weight", .val=cli_fpm_ddr0_weight, .parms=set_default },
            { .name="ddr1_weight", .val=cli_fpm_ddr1_weight, .parms=set_default },
            { .name="pool_cfg", .val=cli_fpm_pool_cfg, .parms=set_default },
            { .name="pool_stat", .val=cli_fpm_pool_stat, .parms=set_default },
            { .name="pool2_stat", .val=cli_fpm_pool2_stat, .parms=set_default },
            { .name="back_door_mem", .val=cli_fpm_back_door_mem, .parms=set_default },
            { .name="pool1_count", .val=cli_fpm_pool1_count, .parms=set_default },
            { .name="pool2_count", .val=cli_fpm_pool2_count, .parms=set_default },
            { .name="timer", .val=cli_fpm_timer, .parms=set_default },
            { .name="fpm_bb_cfg", .val=cli_fpm_fpm_bb_cfg, .parms=set_default },
            { .name="pool1_intr_msk", .val=cli_fpm_pool1_intr_msk, .parms=set_default },
            { .name="pool1_intr_sts", .val=cli_fpm_pool1_intr_sts, .parms=set_default },
            { .name="pool1_stall_msk", .val=cli_fpm_pool1_stall_msk, .parms=set_default },
            { .name="pool2_intr_msk", .val=cli_fpm_pool2_intr_msk, .parms=set_default },
            { .name="pool2_intr_sts", .val=cli_fpm_pool2_intr_sts, .parms=set_default },
            { .name="pool2_stall_msk", .val=cli_fpm_pool2_stall_msk, .parms=set_default },
            { .name="pool1_xon_xoff_cfg", .val=cli_fpm_pool1_xon_xoff_cfg, .parms=set_default },
            { .name="fpm_not_empty_cfg", .val=cli_fpm_fpm_not_empty_cfg, .parms=set_default },
            { .name="mem_ctl", .val=cli_fpm_mem_ctl, .parms=set_default },
            { .name="token_recover_ctl", .val=cli_fpm_token_recover_ctl, .parms=set_default },
            { .name="token_recover_start_end_pool1", .val=cli_fpm_token_recover_start_end_pool1, .parms=set_default },
            { .name="token_recover_start_end_pool2", .val=cli_fpm_token_recover_start_end_pool2, .parms=set_default },
            { .name="pool1_alloc_dealloc", .val=cli_fpm_pool1_alloc_dealloc, .parms=set_default },
            { .name="pool2_alloc_dealloc", .val=cli_fpm_pool2_alloc_dealloc, .parms=set_default },
            { .name="pool3_alloc_dealloc", .val=cli_fpm_pool3_alloc_dealloc, .parms=set_default },
            { .name="pool4_alloc_dealloc", .val=cli_fpm_pool4_alloc_dealloc, .parms=set_default },
            { .name="pool_multi", .val=cli_fpm_pool_multi, .parms=set_default },
            { .name="fpm_bb_force", .val=cli_fpm_fpm_bb_force, .parms=set_default },
            { .name="fpm_bb_forced_ctrl", .val=cli_fpm_fpm_bb_forced_ctrl, .parms=set_default },
            { .name="fpm_bb_forced_addr", .val=cli_fpm_fpm_bb_forced_addr, .parms=set_default },
            { .name="fpm_bb_forced_data", .val=cli_fpm_fpm_bb_forced_data, .parms=set_default },
            { .name="fpm_bb_decode_cfg", .val=cli_fpm_fpm_bb_decode_cfg, .parms=set_default },
            { .name="fpm_bb_dbg_cfg", .val=cli_fpm_fpm_bb_dbg_cfg, .parms=set_default },
            { .name="fpm_bb_dbg_rxfifo_sts", .val=cli_fpm_fpm_bb_dbg_rxfifo_sts, .parms=set_default },
            { .name="fpm_bb_dbg_txfifo_sts", .val=cli_fpm_fpm_bb_dbg_txfifo_sts, .parms=set_default },
            { .name="fpm_bb_dbg_rxfifo_data1", .val=cli_fpm_fpm_bb_dbg_rxfifo_data1, .parms=set_default },
            { .name="fpm_bb_dbg_rxfifo_data2", .val=cli_fpm_fpm_bb_dbg_rxfifo_data2, .parms=set_default },
            { .name="fpm_bb_dbg_txfifo_data1", .val=cli_fpm_fpm_bb_dbg_txfifo_data1, .parms=set_default },
            { .name="fpm_bb_dbg_txfifo_data2", .val=cli_fpm_fpm_bb_dbg_txfifo_data2, .parms=set_default },
            { .name="fpm_bb_dbg_txfifo_data3", .val=cli_fpm_fpm_bb_dbg_txfifo_data3, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_fpm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_fpm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="FPM_CTL" , .val=bdmf_address_fpm_ctl },
            { .name="FPM_CFG1" , .val=bdmf_address_fpm_cfg1 },
            { .name="FPM_WEIGHT" , .val=bdmf_address_fpm_weight },
            { .name="FPM_BB_CFG" , .val=bdmf_address_fpm_bb_cfg },
            { .name="POOL1_INTR_MSK" , .val=bdmf_address_pool1_intr_msk },
            { .name="POOL1_INTR_STS" , .val=bdmf_address_pool1_intr_sts },
            { .name="POOL1_STALL_MSK" , .val=bdmf_address_pool1_stall_msk },
            { .name="POOL2_INTR_MSK" , .val=bdmf_address_pool2_intr_msk },
            { .name="POOL2_INTR_STS" , .val=bdmf_address_pool2_intr_sts },
            { .name="POOL2_STALL_MSK" , .val=bdmf_address_pool2_stall_msk },
            { .name="POOL1_CFG1" , .val=bdmf_address_pool1_cfg1 },
            { .name="POOL1_CFG2" , .val=bdmf_address_pool1_cfg2 },
            { .name="POOL1_CFG3" , .val=bdmf_address_pool1_cfg3 },
            { .name="POOL1_STAT1" , .val=bdmf_address_pool1_stat1 },
            { .name="POOL1_STAT2" , .val=bdmf_address_pool1_stat2 },
            { .name="POOL1_STAT3" , .val=bdmf_address_pool1_stat3 },
            { .name="POOL1_STAT4" , .val=bdmf_address_pool1_stat4 },
            { .name="POOL1_STAT5" , .val=bdmf_address_pool1_stat5 },
            { .name="POOL1_STAT6" , .val=bdmf_address_pool1_stat6 },
            { .name="POOL1_STAT7" , .val=bdmf_address_pool1_stat7 },
            { .name="POOL1_STAT8" , .val=bdmf_address_pool1_stat8 },
            { .name="POOL2_STAT1" , .val=bdmf_address_pool2_stat1 },
            { .name="POOL2_STAT2" , .val=bdmf_address_pool2_stat2 },
            { .name="POOL2_STAT3" , .val=bdmf_address_pool2_stat3 },
            { .name="POOL2_STAT4" , .val=bdmf_address_pool2_stat4 },
            { .name="POOL2_STAT5" , .val=bdmf_address_pool2_stat5 },
            { .name="POOL2_STAT6" , .val=bdmf_address_pool2_stat6 },
            { .name="POOL2_STAT7" , .val=bdmf_address_pool2_stat7 },
            { .name="POOL2_STAT8" , .val=bdmf_address_pool2_stat8 },
            { .name="POOL1_XON_XOFF_CFG" , .val=bdmf_address_pool1_xon_xoff_cfg },
            { .name="FPM_NOT_EMPTY_CFG" , .val=bdmf_address_fpm_not_empty_cfg },
            { .name="MEM_CTL" , .val=bdmf_address_mem_ctl },
            { .name="MEM_DATA1" , .val=bdmf_address_mem_data1 },
            { .name="MEM_DATA2" , .val=bdmf_address_mem_data2 },
            { .name="TOKEN_RECOVER_CTL" , .val=bdmf_address_token_recover_ctl },
            { .name="SHORT_AGING_TIMER" , .val=bdmf_address_short_aging_timer },
            { .name="LONG_AGING_TIMER" , .val=bdmf_address_long_aging_timer },
            { .name="CACHE_RECYCLE_TIMER" , .val=bdmf_address_cache_recycle_timer },
            { .name="EXPIRED_TOKEN_COUNT_POOL1" , .val=bdmf_address_expired_token_count_pool1 },
            { .name="RECOVERED_TOKEN_COUNT_POOL1" , .val=bdmf_address_recovered_token_count_pool1 },
            { .name="EXPIRED_TOKEN_COUNT_POOL2" , .val=bdmf_address_expired_token_count_pool2 },
            { .name="RECOVERED_TOKEN_COUNT_POOL2" , .val=bdmf_address_recovered_token_count_pool2 },
            { .name="TOKEN_RECOVER_START_END_POOL1" , .val=bdmf_address_token_recover_start_end_pool1 },
            { .name="TOKEN_RECOVER_START_END_POOL2" , .val=bdmf_address_token_recover_start_end_pool2 },
            { .name="POOL1_ALLOC_DEALLOC" , .val=bdmf_address_pool1_alloc_dealloc },
            { .name="POOL2_ALLOC_DEALLOC" , .val=bdmf_address_pool2_alloc_dealloc },
            { .name="POOL3_ALLOC_DEALLOC" , .val=bdmf_address_pool3_alloc_dealloc },
            { .name="POOL4_ALLOC_DEALLOC" , .val=bdmf_address_pool4_alloc_dealloc },
            { .name="POOL_MULTI" , .val=bdmf_address_pool_multi },
            { .name="FPM_BB_FORCE" , .val=bdmf_address_fpm_bb_force },
            { .name="FPM_BB_FORCED_CTRL" , .val=bdmf_address_fpm_bb_forced_ctrl },
            { .name="FPM_BB_FORCED_ADDR" , .val=bdmf_address_fpm_bb_forced_addr },
            { .name="FPM_BB_FORCED_DATA" , .val=bdmf_address_fpm_bb_forced_data },
            { .name="FPM_BB_DECODE_CFG" , .val=bdmf_address_fpm_bb_decode_cfg },
            { .name="FPM_BB_DBG_CFG" , .val=bdmf_address_fpm_bb_dbg_cfg },
            { .name="FPM_BB_DBG_RXFIFO_STS" , .val=bdmf_address_fpm_bb_dbg_rxfifo_sts },
            { .name="FPM_BB_DBG_TXFIFO_STS" , .val=bdmf_address_fpm_bb_dbg_txfifo_sts },
            { .name="FPM_BB_DBG_RXFIFO_DATA1" , .val=bdmf_address_fpm_bb_dbg_rxfifo_data1 },
            { .name="FPM_BB_DBG_RXFIFO_DATA2" , .val=bdmf_address_fpm_bb_dbg_rxfifo_data2 },
            { .name="FPM_BB_DBG_TXFIFO_DATA1" , .val=bdmf_address_fpm_bb_dbg_txfifo_data1 },
            { .name="FPM_BB_DBG_TXFIFO_DATA2" , .val=bdmf_address_fpm_bb_dbg_txfifo_data2 },
            { .name="FPM_BB_DBG_TXFIFO_DATA3" , .val=bdmf_address_fpm_bb_dbg_txfifo_data3 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_fpm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

