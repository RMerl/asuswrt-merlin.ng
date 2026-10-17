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
#include "xrdp_drv_hash_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_hash_mask_set(uint8_t hash_id, uint8_t tbl_idx, const hash_mask *mask)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_low = 0;
    uint32_t reg_lkup_tbl_cfg_key_mask_high = 0;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_key_mask_low = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, MASKL, reg_lkup_tbl_cfg_key_mask_low, mask->maskl);
    reg_lkup_tbl_cfg_key_mask_high = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, MASKH, reg_lkup_tbl_cfg_key_mask_high, mask->maskh);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, reg_lkup_tbl_cfg_key_mask_low);
    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, reg_lkup_tbl_cfg_key_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_mask_get(uint8_t hash_id, uint8_t tbl_idx, hash_mask *mask)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_low;
    uint32_t reg_lkup_tbl_cfg_key_mask_high;

#ifdef VALIDATE_PARMS
    if (!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, reg_lkup_tbl_cfg_key_mask_low);
    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, reg_lkup_tbl_cfg_key_mask_high);

    mask->maskl = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, MASKL, reg_lkup_tbl_cfg_key_mask_low);
    mask->maskh = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, MASKH, reg_lkup_tbl_cfg_key_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_gen_cfg_set(uint8_t hash_id, bdmf_boolean block_enable, uint32_t lkup_eng_search_disable, bdmf_boolean storm_move_dis)
{
    uint32_t reg_general_configuration_gen_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (block_enable >= _1BITS_MAX_VAL_) ||
       (lkup_eng_search_disable >= _18BITS_MAX_VAL_) ||
       (storm_move_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_gen_cfg = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, BLOCK_ENABLE, reg_general_configuration_gen_cfg, block_enable);
    reg_general_configuration_gen_cfg = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, LKUP_ENG_SEARCH_DISABLE, reg_general_configuration_gen_cfg, lkup_eng_search_disable);
    reg_general_configuration_gen_cfg = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, STORM_MOVE_DIS, reg_general_configuration_gen_cfg, storm_move_dis);

    RU_REG_WRITE(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, reg_general_configuration_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_gen_cfg_get(uint8_t hash_id, bdmf_boolean *block_enable, uint32_t *lkup_eng_search_disable, bdmf_boolean *storm_move_dis)
{
    uint32_t reg_general_configuration_gen_cfg;

#ifdef VALIDATE_PARMS
    if (!block_enable || !lkup_eng_search_disable || !storm_move_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, reg_general_configuration_gen_cfg);

    *block_enable = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, BLOCK_ENABLE, reg_general_configuration_gen_cfg);
    *lkup_eng_search_disable = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, LKUP_ENG_SEARCH_DISABLE, reg_general_configuration_gen_cfg);
    *storm_move_dis = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_GEN_CFG, STORM_MOVE_DIS, reg_general_configuration_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_fifo_threshold_set(uint8_t hash_id, const hash_general_configuration_fifo_threshold *general_configuration_fifo_threshold)
{
    uint32_t reg_general_configuration_fifo_threshold = 0;

#ifdef VALIDATE_PARMS
    if(!general_configuration_fifo_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (general_configuration_fifo_threshold->hash_table_threshold >= _4BITS_MAX_VAL_) ||
       (general_configuration_fifo_threshold->cam_threshold >= _4BITS_MAX_VAL_) ||
       (general_configuration_fifo_threshold->cntxt_in_threshold >= _4BITS_MAX_VAL_) ||
       (general_configuration_fifo_threshold->storm_threshold >= _6BITS_MAX_VAL_) ||
       (general_configuration_fifo_threshold->storm_threshold_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_fifo_threshold = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, HASH_TABLE_THRESHOLD, reg_general_configuration_fifo_threshold, general_configuration_fifo_threshold->hash_table_threshold);
    reg_general_configuration_fifo_threshold = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, CAM_THRESHOLD, reg_general_configuration_fifo_threshold, general_configuration_fifo_threshold->cam_threshold);
    reg_general_configuration_fifo_threshold = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, CNTXT_IN_THRESHOLD, reg_general_configuration_fifo_threshold, general_configuration_fifo_threshold->cntxt_in_threshold);
    reg_general_configuration_fifo_threshold = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, STORM_THRESHOLD, reg_general_configuration_fifo_threshold, general_configuration_fifo_threshold->storm_threshold);
    reg_general_configuration_fifo_threshold = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, STORM_THRESHOLD_ENABLE, reg_general_configuration_fifo_threshold, general_configuration_fifo_threshold->storm_threshold_enable);

    RU_REG_WRITE(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, reg_general_configuration_fifo_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_fifo_threshold_get(uint8_t hash_id, hash_general_configuration_fifo_threshold *general_configuration_fifo_threshold)
{
    uint32_t reg_general_configuration_fifo_threshold;

#ifdef VALIDATE_PARMS
    if (!general_configuration_fifo_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, reg_general_configuration_fifo_threshold);

    general_configuration_fifo_threshold->hash_table_threshold = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, HASH_TABLE_THRESHOLD, reg_general_configuration_fifo_threshold);
    general_configuration_fifo_threshold->cam_threshold = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, CAM_THRESHOLD, reg_general_configuration_fifo_threshold);
    general_configuration_fifo_threshold->cntxt_in_threshold = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, CNTXT_IN_THRESHOLD, reg_general_configuration_fifo_threshold);
    general_configuration_fifo_threshold->storm_threshold = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, STORM_THRESHOLD, reg_general_configuration_fifo_threshold);
    general_configuration_fifo_threshold->storm_threshold_enable = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD, STORM_THRESHOLD_ENABLE, reg_general_configuration_fifo_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_mult_hit_err_get(uint8_t hash_id, uint32_t *engine_val, uint16_t *key_val)
{
    uint32_t reg_general_configuration_mult_hit_err;

#ifdef VALIDATE_PARMS
    if (!engine_val || !key_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR, reg_general_configuration_mult_hit_err);

    *engine_val = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR, ENGINE_VAL, reg_general_configuration_mult_hit_err);
    *key_val = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR, KEY_VAL, reg_general_configuration_mult_hit_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_auto_init_en_set(uint8_t hash_id, bdmf_boolean auto_init_en)
{
    uint32_t reg_general_configuration_auto_init_en = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (auto_init_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_auto_init_en = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_AUTO_INIT_EN, AUTO_INIT_EN, reg_general_configuration_auto_init_en, auto_init_en);

    RU_REG_WRITE(hash_id, HASH, GENERAL_CONFIGURATION_AUTO_INIT_EN, reg_general_configuration_auto_init_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_auto_init_en_get(uint8_t hash_id, bdmf_boolean *auto_init_en)
{
    uint32_t reg_general_configuration_auto_init_en;

#ifdef VALIDATE_PARMS
    if (!auto_init_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, GENERAL_CONFIGURATION_AUTO_INIT_EN, reg_general_configuration_auto_init_en);

    *auto_init_en = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_AUTO_INIT_EN, AUTO_INIT_EN, reg_general_configuration_auto_init_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_storm_auto_init_en_set(uint8_t hash_id, bdmf_boolean auto_init_en)
{
    uint32_t reg_general_configuration_storm_auto_init_en = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (auto_init_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_storm_auto_init_en = RU_FIELD_SET(hash_id, HASH, GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN, AUTO_INIT_EN, reg_general_configuration_storm_auto_init_en, auto_init_en);

    RU_REG_WRITE(hash_id, HASH, GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN, reg_general_configuration_storm_auto_init_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_storm_auto_init_en_get(uint8_t hash_id, bdmf_boolean *auto_init_en)
{
    uint32_t reg_general_configuration_storm_auto_init_en;

#ifdef VALIDATE_PARMS
    if (!auto_init_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN, reg_general_configuration_storm_auto_init_en);

    *auto_init_en = RU_FIELD_GET(hash_id, HASH, GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN, AUTO_INIT_EN, reg_general_configuration_storm_auto_init_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hits_get(uint8_t hash_id, uint32_t *cnt)
{
    uint32_t reg_pm_counters_hits;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_HITS, reg_pm_counters_hits);

    *cnt = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_HITS, CNT, reg_pm_counters_hits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_srchs_get(uint8_t hash_id, uint32_t *cnt)
{
    uint32_t reg_pm_counters_srchs;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_SRCHS, reg_pm_counters_srchs);

    *cnt = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_SRCHS, CNT, reg_pm_counters_srchs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_miss_get(uint8_t hash_id, uint32_t *cnt)
{
    uint32_t reg_pm_counters_miss;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_MISS, reg_pm_counters_miss);

    *cnt = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_MISS, CNT, reg_pm_counters_miss);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_storm_window_size_set(uint8_t hash_id, uint32_t cnt)
{
    uint32_t reg_pm_counters_storm_window_size = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pm_counters_storm_window_size = RU_FIELD_SET(hash_id, HASH, PM_COUNTERS_STORM_WINDOW_SIZE, CNT, reg_pm_counters_storm_window_size, cnt);

    RU_REG_WRITE(hash_id, HASH, PM_COUNTERS_STORM_WINDOW_SIZE, reg_pm_counters_storm_window_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_storm_window_size_get(uint8_t hash_id, uint32_t *cnt)
{
    uint32_t reg_pm_counters_storm_window_size;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_STORM_WINDOW_SIZE, reg_pm_counters_storm_window_size);

    *cnt = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_STORM_WINDOW_SIZE, CNT, reg_pm_counters_storm_window_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_storm_cfg_set(uint8_t hash_id, uint16_t storm_th, bdmf_boolean storm_window_enable)
{
    uint32_t reg_pm_counters_storm_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (storm_window_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pm_counters_storm_cfg = RU_FIELD_SET(hash_id, HASH, PM_COUNTERS_STORM_CFG, STORM_TH, reg_pm_counters_storm_cfg, storm_th);
    reg_pm_counters_storm_cfg = RU_FIELD_SET(hash_id, HASH, PM_COUNTERS_STORM_CFG, STORM_WINDOW_ENABLE, reg_pm_counters_storm_cfg, storm_window_enable);

    RU_REG_WRITE(hash_id, HASH, PM_COUNTERS_STORM_CFG, reg_pm_counters_storm_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_storm_cfg_get(uint8_t hash_id, uint16_t *storm_th, bdmf_boolean *storm_window_enable)
{
    uint32_t reg_pm_counters_storm_cfg;

#ifdef VALIDATE_PARMS
    if (!storm_th || !storm_window_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_STORM_CFG, reg_pm_counters_storm_cfg);

    *storm_th = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_STORM_CFG, STORM_TH, reg_pm_counters_storm_cfg);
    *storm_window_enable = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_STORM_CFG, STORM_WINDOW_ENABLE, reg_pm_counters_storm_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_set(uint8_t hash_id, bdmf_boolean val)
{
    uint32_t reg_pm_counters_frz_cnt = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (val >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pm_counters_frz_cnt = RU_FIELD_SET(hash_id, HASH, PM_COUNTERS_FRZ_CNT, VAL, reg_pm_counters_frz_cnt, val);

    RU_REG_WRITE(hash_id, HASH, PM_COUNTERS_FRZ_CNT, reg_pm_counters_frz_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_get(uint8_t hash_id, bdmf_boolean *val)
{
    uint32_t reg_pm_counters_frz_cnt;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, PM_COUNTERS_FRZ_CNT, reg_pm_counters_frz_cnt);

    *val = RU_FIELD_GET(hash_id, HASH, PM_COUNTERS_FRZ_CNT, VAL, reg_pm_counters_frz_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(uint8_t hash_id, uint8_t tbl_idx, const hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg)
{
    uint32_t reg_lkup_tbl_cfg_cntxt_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!lkup_tbl_cfg_cntxt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8) ||
       (lkup_tbl_cfg_cntxt_cfg->direct_read_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->cam_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->move_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->storm_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->aging_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->rslt128_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->move_mask >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, DIRECT_READ_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->direct_read_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, CAM_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->cam_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, MOVE_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->move_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, STORM_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->storm_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, AGING_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->aging_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, RSLT128_EN, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->rslt128_en);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, MOVE_MASK, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->move_mask);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_CNTXT_CFG, reg_lkup_tbl_cfg_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(uint8_t hash_id, uint8_t tbl_idx, hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg)
{
    uint32_t reg_lkup_tbl_cfg_cntxt_cfg;

#ifdef VALIDATE_PARMS
    if (!lkup_tbl_cfg_cntxt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_CNTXT_CFG, reg_lkup_tbl_cfg_cntxt_cfg);

    lkup_tbl_cfg_cntxt_cfg->direct_read_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, DIRECT_READ_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->cam_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, CAM_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->move_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, MOVE_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->storm_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, STORM_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->aging_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, AGING_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->rslt128_en = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, RSLT128_EN, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->move_mask = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_CNTXT_CFG, MOVE_MASK, reg_lkup_tbl_cfg_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t maskh)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_highest = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_key_mask_highest = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_HIGHEST, MASKH, reg_lkup_tbl_cfg_key_mask_highest, maskh);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGHEST, reg_lkup_tbl_cfg_key_mask_highest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *maskh)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_highest;

#ifdef VALIDATE_PARMS
    if (!maskh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGHEST, reg_lkup_tbl_cfg_key_mask_highest);

    *maskh = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_HIGHEST, MASKH, reg_lkup_tbl_cfg_key_mask_highest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t maskm)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_mid = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_key_mask_mid = RU_FIELD_SET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_MID, MASKM, reg_lkup_tbl_cfg_key_mask_mid, maskm);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_MID, reg_lkup_tbl_cfg_key_mask_mid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *maskm)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_mid;

#ifdef VALIDATE_PARMS
    if (!maskm)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_MID, reg_lkup_tbl_cfg_key_mask_mid);

    *maskm = RU_FIELD_GET(hash_id, HASH, LKUP_TBL_CFG_KEY_MASK_MID, MASKM, reg_lkup_tbl_cfg_key_mask_mid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_configuration_tm_cfg_set(uint8_t hash_id, uint16_t val)
{
    uint32_t reg_cam_configuration_tm_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (val >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_configuration_tm_cfg = RU_FIELD_SET(hash_id, HASH, CAM_CONFIGURATION_TM_CFG, VAL, reg_cam_configuration_tm_cfg, val);

    RU_REG_WRITE(hash_id, HASH, CAM_CONFIGURATION_TM_CFG, reg_cam_configuration_tm_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_configuration_tm_cfg_get(uint8_t hash_id, uint16_t *val)
{
    uint32_t reg_cam_configuration_tm_cfg;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_CONFIGURATION_TM_CFG, reg_cam_configuration_tm_cfg);

    *val = RU_FIELD_GET(hash_id, HASH, CAM_CONFIGURATION_TM_CFG, VAL, reg_cam_configuration_tm_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_set(uint8_t hash_id, uint8_t cmd)
{
    uint32_t reg_cam_indirect_op = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (cmd >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_op = RU_FIELD_SET(hash_id, HASH, CAM_INDIRECT_OP, CMD, reg_cam_indirect_op, cmd);

    RU_REG_WRITE(hash_id, HASH, CAM_INDIRECT_OP, reg_cam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_get(uint8_t hash_id, uint8_t *cmd)
{
    uint32_t reg_cam_indirect_op;

#ifdef VALIDATE_PARMS
    if (!cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_OP, reg_cam_indirect_op);

    *cmd = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_OP, CMD, reg_cam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_done_get(uint8_t hash_id, bdmf_boolean *val)
{
    uint32_t reg_cam_indirect_op_done;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_OP_DONE, reg_cam_indirect_op_done);

    *val = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_OP_DONE, VAL, reg_cam_indirect_op_done);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_addr_set(uint8_t hash_id, bdmf_boolean key_ind, uint8_t entry_addr)
{
    uint32_t reg_cam_indirect_addr = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (key_ind >= _1BITS_MAX_VAL_) ||
       (entry_addr >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_addr = RU_FIELD_SET(hash_id, HASH, CAM_INDIRECT_ADDR, KEY_IND, reg_cam_indirect_addr, key_ind);
    reg_cam_indirect_addr = RU_FIELD_SET(hash_id, HASH, CAM_INDIRECT_ADDR, ENTRY_ADDR, reg_cam_indirect_addr, entry_addr);

    RU_REG_WRITE(hash_id, HASH, CAM_INDIRECT_ADDR, reg_cam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_addr_get(uint8_t hash_id, bdmf_boolean *key_ind, uint8_t *entry_addr)
{
    uint32_t reg_cam_indirect_addr;

#ifdef VALIDATE_PARMS
    if (!key_ind || !entry_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_ADDR, reg_cam_indirect_addr);

    *key_ind = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_ADDR, KEY_IND, reg_cam_indirect_addr);
    *entry_addr = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_ADDR, ENTRY_ADDR, reg_cam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_set(uint8_t hash_id, bdmf_boolean valid)
{
    uint32_t reg_cam_indirect_vlid_in = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_vlid_in = RU_FIELD_SET(hash_id, HASH, CAM_INDIRECT_VLID_IN, VALID, reg_cam_indirect_vlid_in, valid);

    RU_REG_WRITE(hash_id, HASH, CAM_INDIRECT_VLID_IN, reg_cam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_get(uint8_t hash_id, bdmf_boolean *valid)
{
    uint32_t reg_cam_indirect_vlid_in;

#ifdef VALIDATE_PARMS
    if (!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_VLID_IN, reg_cam_indirect_vlid_in);

    *valid = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_VLID_IN, VALID, reg_cam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_set(uint8_t hash_id, uint8_t valid)
{
    uint32_t reg_cam_indirect_vlid_out = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (valid >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_vlid_out = RU_FIELD_SET(hash_id, HASH, CAM_INDIRECT_VLID_OUT, VALID, reg_cam_indirect_vlid_out, valid);

    RU_REG_WRITE(hash_id, HASH, CAM_INDIRECT_VLID_OUT, reg_cam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_get(uint8_t hash_id, uint8_t *valid)
{
    uint32_t reg_cam_indirect_vlid_out;

#ifdef VALIDATE_PARMS
    if (!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_VLID_OUT, reg_cam_indirect_vlid_out);

    *valid = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_VLID_OUT, VALID, reg_cam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_rslt_get(uint8_t hash_id, bdmf_boolean *match, uint8_t *index)
{
    uint32_t reg_cam_indirect_rslt;

#ifdef VALIDATE_PARMS
    if (!match || !index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_INDIRECT_RSLT, reg_cam_indirect_rslt);

    *match = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_RSLT, MATCH, reg_cam_indirect_rslt);
    *index = RU_FIELD_GET(hash_id, HASH, CAM_INDIRECT_RSLT, INDEX, reg_cam_indirect_rslt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_in_set(uint8_t hash_id, uint8_t zero, const hash_cam_indirect_key_in *cam_indirect_key_in)
{
#ifdef VALIDATE_PARMS
    if(!cam_indirect_key_in)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(hash_id, zero * 2 + 0, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[0]);
    RU_REG_RAM_WRITE(hash_id, zero * 2 + 1, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_in_get(uint8_t hash_id, uint8_t zero, hash_cam_indirect_key_in *cam_indirect_key_in)
{
#ifdef VALIDATE_PARMS
    if (!cam_indirect_key_in)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, zero * 2 + 0, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[0]);
    RU_REG_RAM_READ(hash_id, zero * 2 + 1, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_out_get(uint8_t hash_id, uint8_t zero, hash_cam_indirect_key_out *cam_indirect_key_out)
{
#ifdef VALIDATE_PARMS
    if (!cam_indirect_key_out)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, zero * 2 + 0, HASH, CAM_INDIRECT_KEY_OUT, cam_indirect_key_out->key_out[0]);
    RU_REG_RAM_READ(hash_id, zero * 2 + 1, HASH, CAM_INDIRECT_KEY_OUT, cam_indirect_key_out->key_out[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_status_get(uint8_t hash_id, uint32_t *value)
{
    uint32_t reg_cam_bist_bist_status;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_BIST_BIST_STATUS, reg_cam_bist_bist_status);

    *value = RU_FIELD_GET(hash_id, HASH, CAM_BIST_BIST_STATUS, VALUE, reg_cam_bist_bist_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_control_set(uint8_t hash_id, uint32_t value)
{
    uint32_t reg_cam_bist_bist_control = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_control = RU_FIELD_SET(hash_id, HASH, CAM_BIST_BIST_CONTROL, VALUE, reg_cam_bist_bist_control, value);

    RU_REG_WRITE(hash_id, HASH, CAM_BIST_BIST_CONTROL, reg_cam_bist_bist_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_control_get(uint8_t hash_id, uint32_t *value)
{
    uint32_t reg_cam_bist_bist_control;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_BIST_BIST_CONTROL, reg_cam_bist_bist_control);

    *value = RU_FIELD_GET(hash_id, HASH, CAM_BIST_BIST_CONTROL, VALUE, reg_cam_bist_bist_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_cache_repair_set(uint8_t hash_id, uint32_t value)
{
    uint32_t reg_cam_bist_bist_cache_repair = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_cache_repair = RU_FIELD_SET(hash_id, HASH, CAM_BIST_BIST_CACHE_REPAIR, VALUE, reg_cam_bist_bist_cache_repair, value);

    RU_REG_WRITE(hash_id, HASH, CAM_BIST_BIST_CACHE_REPAIR, reg_cam_bist_bist_cache_repair);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_cache_repair_get(uint8_t hash_id, uint32_t *value)
{
    uint32_t reg_cam_bist_bist_cache_repair;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_BIST_BIST_CACHE_REPAIR, reg_cam_bist_bist_cache_repair);

    *value = RU_FIELD_GET(hash_id, HASH, CAM_BIST_BIST_CACHE_REPAIR, VALUE, reg_cam_bist_bist_cache_repair);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_control_set(uint8_t hash_id, uint32_t value)
{
    uint32_t reg_cam_bist_bist_dbg_control = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_dbg_control = RU_FIELD_SET(hash_id, HASH, CAM_BIST_BIST_DBG_CONTROL, VALUE, reg_cam_bist_bist_dbg_control, value);

    RU_REG_WRITE(hash_id, HASH, CAM_BIST_BIST_DBG_CONTROL, reg_cam_bist_bist_dbg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_control_get(uint8_t hash_id, uint32_t *value)
{
    uint32_t reg_cam_bist_bist_dbg_control;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_BIST_BIST_DBG_CONTROL, reg_cam_bist_bist_dbg_control);

    *value = RU_FIELD_GET(hash_id, HASH, CAM_BIST_BIST_DBG_CONTROL, VALUE, reg_cam_bist_bist_dbg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_set(uint8_t hash_id, uint32_t value)
{
    uint32_t reg_cam_bist_bist_dbg_data = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_dbg_data = RU_FIELD_SET(hash_id, HASH, CAM_BIST_BIST_DBG_DATA, VALUE, reg_cam_bist_bist_dbg_data, value);

    RU_REG_WRITE(hash_id, HASH, CAM_BIST_BIST_DBG_DATA, reg_cam_bist_bist_dbg_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_get(uint8_t hash_id, uint32_t *value)
{
    uint32_t reg_cam_bist_bist_dbg_data;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, CAM_BIST_BIST_DBG_DATA, reg_cam_bist_bist_dbg_data);

    *value = RU_FIELD_GET(hash_id, HASH, CAM_BIST_BIST_DBG_DATA, VALUE, reg_cam_bist_bist_dbg_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_isr_set(uint8_t hash_id, bdmf_boolean mult_match)
{
    uint32_t reg_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (mult_match >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(hash_id, HASH, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr, mult_match);

    RU_REG_WRITE(hash_id, HASH, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_isr_get(uint8_t hash_id, bdmf_boolean *mult_match)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!mult_match)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    *mult_match = RU_FIELD_GET(hash_id, HASH, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ism_get(uint8_t hash_id, uint8_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(hash_id, HASH, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ier_set(uint8_t hash_id, uint8_t iem)
{
    uint32_t reg_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (iem >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(hash_id, HASH, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(hash_id, HASH, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ier_get(uint8_t hash_id, uint8_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(hash_id, HASH, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_itr_set(uint8_t hash_id, uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(hash_id, HASH, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(hash_id, HASH, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_itr_get(uint8_t hash_id, uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(hash_id, HASH, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg0_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg0;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG0, reg_debug_dbg0);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG0, VAL, reg_debug_dbg0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg1_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg1;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG1, reg_debug_dbg1);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG1, VAL, reg_debug_dbg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg2_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg2;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG2, reg_debug_dbg2);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG2, VAL, reg_debug_dbg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg3_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg3;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG3, reg_debug_dbg3);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG3, VAL, reg_debug_dbg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg4_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg4;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG4, reg_debug_dbg4);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG4, VAL, reg_debug_dbg4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg5_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg5;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG5, reg_debug_dbg5);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG5, VAL, reg_debug_dbg5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg6_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg6;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG6, reg_debug_dbg6);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG6, VAL, reg_debug_dbg6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg7_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg7;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG7, reg_debug_dbg7);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG7, VAL, reg_debug_dbg7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg8_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg8;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG8, reg_debug_dbg8);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG8, VAL, reg_debug_dbg8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg9_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg9;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG9, reg_debug_dbg9);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG9, VAL, reg_debug_dbg9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg10_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg10;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG10, reg_debug_dbg10);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG10, VAL, reg_debug_dbg10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg11_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg11;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG11, reg_debug_dbg11);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG11, VAL, reg_debug_dbg11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg12_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg12;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG12, reg_debug_dbg12);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG12, VAL, reg_debug_dbg12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg13_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg13;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG13, reg_debug_dbg13);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG13, VAL, reg_debug_dbg13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg14_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg14;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG14, reg_debug_dbg14);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG14, VAL, reg_debug_dbg14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg15_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg15;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG15, reg_debug_dbg15);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG15, VAL, reg_debug_dbg15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg16_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg16;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG16, reg_debug_dbg16);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG16, VAL, reg_debug_dbg16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg17_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg17;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG17, reg_debug_dbg17);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG17, VAL, reg_debug_dbg17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg18_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg18;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG18, reg_debug_dbg18);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG18, VAL, reg_debug_dbg18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg19_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg19;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG19, reg_debug_dbg19);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG19, VAL, reg_debug_dbg19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg20_get(uint8_t hash_id, uint32_t *val)
{
    uint32_t reg_debug_dbg20;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG20, reg_debug_dbg20);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG20, VAL, reg_debug_dbg20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg_sel_set(uint8_t hash_id, uint8_t val)
{
    uint32_t reg_debug_dbg_sel = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (val >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_dbg_sel = RU_FIELD_SET(hash_id, HASH, DEBUG_DBG_SEL, VAL, reg_debug_dbg_sel, val);

    RU_REG_WRITE(hash_id, HASH, DEBUG_DBG_SEL, reg_debug_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg_sel_get(uint8_t hash_id, uint8_t *val)
{
    uint32_t reg_debug_dbg_sel;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(hash_id, HASH, DEBUG_DBG_SEL, reg_debug_dbg_sel);

    *val = RU_FIELD_GET(hash_id, HASH, DEBUG_DBG_SEL, VAL, reg_debug_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_aging_ram_aging_set(uint8_t hash_id, uint16_t entry_idx, bdmf_boolean age)
{
    uint32_t reg_aging_ram_aging = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (entry_idx >= 147584))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    reg_aging_ram_aging = RU_FLD_VAL_SET(reg_aging_ram_aging, 0x1 << (entry_idx % 32), entry_idx % 32, age);

    RU_REG_RAM_WRITE(hash_id, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_aging_ram_aging_get(uint8_t hash_id, uint16_t entry_idx, bdmf_boolean *age)
{
    uint32_t reg_aging_ram_aging;

#ifdef VALIDATE_PARMS
    if (!age)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (entry_idx >= 147584))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    *age = RU_FLD_VAL_GET(reg_aging_ram_aging, 0x1 << (entry_idx % 32), entry_idx % 32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_storm_ram_storm_set(uint8_t hash_id, uint32_t idx, uint32_t data)
{
    uint32_t reg_storm_ram_storm = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 4096))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_storm_ram_storm = RU_FIELD_SET(hash_id, HASH, STORM_RAM_STORM, DATA, reg_storm_ram_storm, data);

    RU_REG_RAM_WRITE(hash_id, idx, HASH, STORM_RAM_STORM, reg_storm_ram_storm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_storm_ram_storm_get(uint8_t hash_id, uint32_t idx, uint32_t *data)
{
    uint32_t reg_storm_ram_storm;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 4096))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, idx, HASH, STORM_RAM_STORM, reg_storm_ram_storm);

    *data = RU_FIELD_GET(hash_id, HASH, STORM_RAM_STORM, DATA, reg_storm_ram_storm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_context_ram_context_cam_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data)
{
    uint32_t reg_cam_context_ram_context_cam = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 512))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_context_ram_context_cam = RU_FIELD_SET(hash_id, HASH, CAM_CONTEXT_RAM_CONTEXT_CAM, DATA, reg_cam_context_ram_context_cam, data);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, CAM_CONTEXT_RAM_CONTEXT_CAM, reg_cam_context_ram_context_cam);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_context_ram_context_cam_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data)
{
    uint32_t reg_cam_context_ram_context_cam;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 512))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, CAM_CONTEXT_RAM_CONTEXT_CAM, reg_cam_context_ram_context_cam);

    *data = RU_FIELD_GET(hash_id, HASH, CAM_CONTEXT_RAM_CONTEXT_CAM, DATA, reg_cam_context_ram_context_cam);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data)
{
    uint32_t reg_context_ram_context = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 245760))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_context_ram_context = RU_FIELD_SET(hash_id, HASH, CONTEXT_RAM_CONTEXT, DATA, reg_context_ram_context, data);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, CONTEXT_RAM_CONTEXT, reg_context_ram_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data)
{
    uint32_t reg_context_ram_context;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 245760))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, CONTEXT_RAM_CONTEXT, reg_context_ram_context);

    *data = RU_FIELD_GET(hash_id, HASH, CONTEXT_RAM_CONTEXT, DATA, reg_context_ram_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_hash_ram_set(uint8_t hash_id, uint8_t tbl_idx, uint32_t data)
{
    uint32_t reg_ram_hash_ram = 0;

#ifdef VALIDATE_PARMS
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 589824))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ram_hash_ram = RU_FIELD_SET(hash_id, HASH, RAM_HASH_RAM, DATA, reg_ram_hash_ram, data);

    RU_REG_RAM_WRITE(hash_id, tbl_idx, HASH, RAM_HASH_RAM, reg_ram_hash_ram);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_hash_ram_get(uint8_t hash_id, uint8_t tbl_idx, uint32_t *data)
{
    uint32_t reg_ram_hash_ram;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hash_id >= BLOCK_ADDR_COUNT) ||
       (tbl_idx >= 589824))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(hash_id, tbl_idx, HASH, RAM_HASH_RAM, reg_ram_hash_ram);

    *data = RU_FIELD_GET(hash_id, HASH, RAM_HASH_RAM, DATA, reg_ram_hash_ram);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_general_configuration_gen_cfg,
    bdmf_address_general_configuration_fifo_threshold,
    bdmf_address_general_configuration_mult_hit_err,
    bdmf_address_general_configuration_auto_init_en,
    bdmf_address_general_configuration_storm_auto_init_en,
    bdmf_address_pm_counters_hits,
    bdmf_address_pm_counters_srchs,
    bdmf_address_pm_counters_miss,
    bdmf_address_pm_counters_storm_window_size,
    bdmf_address_pm_counters_storm_cfg,
    bdmf_address_pm_counters_frz_cnt,
    bdmf_address_lkup_tbl_cfg_cntxt_cfg,
    bdmf_address_lkup_tbl_cfg_key_mask_highest,
    bdmf_address_lkup_tbl_cfg_key_mask_high,
    bdmf_address_lkup_tbl_cfg_key_mask_mid,
    bdmf_address_lkup_tbl_cfg_key_mask_low,
    bdmf_address_cam_configuration_tm_cfg,
    bdmf_address_cam_indirect_op,
    bdmf_address_cam_indirect_op_done,
    bdmf_address_cam_indirect_addr,
    bdmf_address_cam_indirect_vlid_in,
    bdmf_address_cam_indirect_vlid_out,
    bdmf_address_cam_indirect_rslt,
    bdmf_address_cam_indirect_key_in,
    bdmf_address_cam_indirect_key_out,
    bdmf_address_cam_bist_bist_status,
    bdmf_address_cam_bist_bist_control,
    bdmf_address_cam_bist_bist_cache_repair,
    bdmf_address_cam_bist_bist_dbg_control,
    bdmf_address_cam_bist_bist_dbg_data,
    bdmf_address_intr_ctrl_isr,
    bdmf_address_intr_ctrl_ism,
    bdmf_address_intr_ctrl_ier,
    bdmf_address_intr_ctrl_itr,
    bdmf_address_debug_dbg0,
    bdmf_address_debug_dbg1,
    bdmf_address_debug_dbg2,
    bdmf_address_debug_dbg3,
    bdmf_address_debug_dbg4,
    bdmf_address_debug_dbg5,
    bdmf_address_debug_dbg6,
    bdmf_address_debug_dbg7,
    bdmf_address_debug_dbg8,
    bdmf_address_debug_dbg9,
    bdmf_address_debug_dbg10,
    bdmf_address_debug_dbg11,
    bdmf_address_debug_dbg12,
    bdmf_address_debug_dbg13,
    bdmf_address_debug_dbg14,
    bdmf_address_debug_dbg15,
    bdmf_address_debug_dbg16,
    bdmf_address_debug_dbg17,
    bdmf_address_debug_dbg18,
    bdmf_address_debug_dbg19,
    bdmf_address_debug_dbg20,
    bdmf_address_debug_dbg_sel,
    bdmf_address_aging_ram_aging,
    bdmf_address_storm_ram_storm,
    bdmf_address_cam_context_ram_context_cam,
    bdmf_address_context_ram_context,
    bdmf_address_ram_hash_ram,
}
bdmf_address;

static int ag_drv_hash_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_hash_mask:
    {
        hash_mask mask = { .maskl = parm[3].value.unumber, .maskh = parm[4].value.unumber};
        ag_err = ag_drv_hash_mask_set(parm[1].value.unumber, parm[2].value.unumber, &mask);
        break;
    }
    case cli_hash_general_configuration_gen_cfg:
        ag_err = ag_drv_hash_general_configuration_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_hash_general_configuration_fifo_threshold:
    {
        hash_general_configuration_fifo_threshold general_configuration_fifo_threshold = { .hash_table_threshold = parm[2].value.unumber, .cam_threshold = parm[3].value.unumber, .cntxt_in_threshold = parm[4].value.unumber, .storm_threshold = parm[5].value.unumber, .storm_threshold_enable = parm[6].value.unumber};
        ag_err = ag_drv_hash_general_configuration_fifo_threshold_set(parm[1].value.unumber, &general_configuration_fifo_threshold);
        break;
    }
    case cli_hash_general_configuration_auto_init_en:
        ag_err = ag_drv_hash_general_configuration_auto_init_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_general_configuration_storm_auto_init_en:
        ag_err = ag_drv_hash_general_configuration_storm_auto_init_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_pm_counters_storm_window_size:
        ag_err = ag_drv_hash_pm_counters_storm_window_size_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_pm_counters_storm_cfg:
        ag_err = ag_drv_hash_pm_counters_storm_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_pm_counters_frz_cnt:
        ag_err = ag_drv_hash_pm_counters_frz_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_lkup_tbl_cfg_cntxt_cfg:
    {
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg = { .direct_read_en = parm[3].value.unumber, .cam_en = parm[4].value.unumber, .move_en = parm[5].value.unumber, .storm_en = parm[6].value.unumber, .aging_en = parm[7].value.unumber, .rslt128_en = parm[8].value.unumber, .move_mask = parm[9].value.unumber};
        ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(parm[1].value.unumber, parm[2].value.unumber, &lkup_tbl_cfg_cntxt_cfg);
        break;
    }
    case cli_hash_lkup_tbl_cfg_key_mask_highest:
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_lkup_tbl_cfg_key_mask_mid:
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_cam_configuration_tm_cfg:
        ag_err = ag_drv_hash_cam_configuration_tm_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_indirect_op:
        ag_err = ag_drv_hash_cam_indirect_op_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_indirect_addr:
        ag_err = ag_drv_hash_cam_indirect_addr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_cam_indirect_vlid_in:
        ag_err = ag_drv_hash_cam_indirect_vlid_in_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_indirect_vlid_out:
        ag_err = ag_drv_hash_cam_indirect_vlid_out_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_indirect_key_in:
    {
        hash_cam_indirect_key_in cam_indirect_key_in = { .key_in = { parm[3].value.unumber, parm[4].value.unumber}};
        ag_err = ag_drv_hash_cam_indirect_key_in_set(parm[1].value.unumber, parm[2].value.unumber, &cam_indirect_key_in);
        break;
    }
    case cli_hash_cam_bist_bist_control:
        ag_err = ag_drv_hash_cam_bist_bist_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_bist_bist_cache_repair:
        ag_err = ag_drv_hash_cam_bist_bist_cache_repair_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_bist_bist_dbg_control:
        ag_err = ag_drv_hash_cam_bist_bist_dbg_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_bist_bist_dbg_data:
        ag_err = ag_drv_hash_cam_bist_bist_dbg_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_intr_ctrl_isr:
        ag_err = ag_drv_hash_intr_ctrl_isr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_intr_ctrl_ier:
        ag_err = ag_drv_hash_intr_ctrl_ier_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_intr_ctrl_itr:
        ag_err = ag_drv_hash_intr_ctrl_itr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_debug_dbg_sel:
        ag_err = ag_drv_hash_debug_dbg_sel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_aging_ram_aging:
        ag_err = ag_drv_hash_aging_ram_aging_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_storm_ram_storm:
        ag_err = ag_drv_hash_storm_ram_storm_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_cam_context_ram_context_cam:
        ag_err = ag_drv_hash_cam_context_ram_context_cam_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_context_ram_context:
        ag_err = ag_drv_hash_context_ram_context_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_hash_ram_hash_ram:
        ag_err = ag_drv_hash_ram_hash_ram_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_hash_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_hash_mask:
    {
        hash_mask mask;
        ag_err = ag_drv_hash_mask_get(parm[1].value.unumber, parm[2].value.unumber, &mask);
        bdmf_session_print(session, "maskl = %u = 0x%x\n", mask.maskl, mask.maskl);
        bdmf_session_print(session, "maskh = %u = 0x%x\n", mask.maskh, mask.maskh);
        break;
    }
    case cli_hash_general_configuration_gen_cfg:
    {
        bdmf_boolean block_enable;
        uint32_t lkup_eng_search_disable;
        bdmf_boolean storm_move_dis;
        ag_err = ag_drv_hash_general_configuration_gen_cfg_get(parm[1].value.unumber, &block_enable, &lkup_eng_search_disable, &storm_move_dis);
        bdmf_session_print(session, "block_enable = %u = 0x%x\n", block_enable, block_enable);
        bdmf_session_print(session, "lkup_eng_search_disable = %u = 0x%x\n", lkup_eng_search_disable, lkup_eng_search_disable);
        bdmf_session_print(session, "storm_move_dis = %u = 0x%x\n", storm_move_dis, storm_move_dis);
        break;
    }
    case cli_hash_general_configuration_fifo_threshold:
    {
        hash_general_configuration_fifo_threshold general_configuration_fifo_threshold;
        ag_err = ag_drv_hash_general_configuration_fifo_threshold_get(parm[1].value.unumber, &general_configuration_fifo_threshold);
        bdmf_session_print(session, "hash_table_threshold = %u = 0x%x\n", general_configuration_fifo_threshold.hash_table_threshold, general_configuration_fifo_threshold.hash_table_threshold);
        bdmf_session_print(session, "cam_threshold = %u = 0x%x\n", general_configuration_fifo_threshold.cam_threshold, general_configuration_fifo_threshold.cam_threshold);
        bdmf_session_print(session, "cntxt_in_threshold = %u = 0x%x\n", general_configuration_fifo_threshold.cntxt_in_threshold, general_configuration_fifo_threshold.cntxt_in_threshold);
        bdmf_session_print(session, "storm_threshold = %u = 0x%x\n", general_configuration_fifo_threshold.storm_threshold, general_configuration_fifo_threshold.storm_threshold);
        bdmf_session_print(session, "storm_threshold_enable = %u = 0x%x\n", general_configuration_fifo_threshold.storm_threshold_enable, general_configuration_fifo_threshold.storm_threshold_enable);
        break;
    }
    case cli_hash_general_configuration_mult_hit_err:
    {
        uint32_t engine_val;
        uint16_t key_val;
        ag_err = ag_drv_hash_general_configuration_mult_hit_err_get(parm[1].value.unumber, &engine_val, &key_val);
        bdmf_session_print(session, "engine_val = %u = 0x%x\n", engine_val, engine_val);
        bdmf_session_print(session, "key_val = %u = 0x%x\n", key_val, key_val);
        break;
    }
    case cli_hash_general_configuration_auto_init_en:
    {
        bdmf_boolean auto_init_en;
        ag_err = ag_drv_hash_general_configuration_auto_init_en_get(parm[1].value.unumber, &auto_init_en);
        bdmf_session_print(session, "auto_init_en = %u = 0x%x\n", auto_init_en, auto_init_en);
        break;
    }
    case cli_hash_general_configuration_storm_auto_init_en:
    {
        bdmf_boolean auto_init_en;
        ag_err = ag_drv_hash_general_configuration_storm_auto_init_en_get(parm[1].value.unumber, &auto_init_en);
        bdmf_session_print(session, "auto_init_en = %u = 0x%x\n", auto_init_en, auto_init_en);
        break;
    }
    case cli_hash_pm_counters_hits:
    {
        uint32_t cnt;
        ag_err = ag_drv_hash_pm_counters_hits_get(parm[1].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_srchs:
    {
        uint32_t cnt;
        ag_err = ag_drv_hash_pm_counters_srchs_get(parm[1].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_miss:
    {
        uint32_t cnt;
        ag_err = ag_drv_hash_pm_counters_miss_get(parm[1].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_storm_window_size:
    {
        uint32_t cnt;
        ag_err = ag_drv_hash_pm_counters_storm_window_size_get(parm[1].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_storm_cfg:
    {
        uint16_t storm_th;
        bdmf_boolean storm_window_enable;
        ag_err = ag_drv_hash_pm_counters_storm_cfg_get(parm[1].value.unumber, &storm_th, &storm_window_enable);
        bdmf_session_print(session, "storm_th = %u = 0x%x\n", storm_th, storm_th);
        bdmf_session_print(session, "storm_window_enable = %u = 0x%x\n", storm_window_enable, storm_window_enable);
        break;
    }
    case cli_hash_pm_counters_frz_cnt:
    {
        bdmf_boolean val;
        ag_err = ag_drv_hash_pm_counters_frz_cnt_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_lkup_tbl_cfg_cntxt_cfg:
    {
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg;
        ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(parm[1].value.unumber, parm[2].value.unumber, &lkup_tbl_cfg_cntxt_cfg);
        bdmf_session_print(session, "direct_read_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.direct_read_en);
        bdmf_session_print(session, "cam_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.cam_en);
        bdmf_session_print(session, "move_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.move_en);
        bdmf_session_print(session, "storm_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.storm_en, lkup_tbl_cfg_cntxt_cfg.storm_en);
        bdmf_session_print(session, "aging_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.aging_en);
        bdmf_session_print(session, "rslt128_en = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en);
        bdmf_session_print(session, "move_mask = %u = 0x%x\n", lkup_tbl_cfg_cntxt_cfg.move_mask, lkup_tbl_cfg_cntxt_cfg.move_mask);
        break;
    }
    case cli_hash_lkup_tbl_cfg_key_mask_highest:
    {
        uint32_t maskh;
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(parm[1].value.unumber, parm[2].value.unumber, &maskh);
        bdmf_session_print(session, "maskh = %u = 0x%x\n", maskh, maskh);
        break;
    }
    case cli_hash_lkup_tbl_cfg_key_mask_mid:
    {
        uint32_t maskm;
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(parm[1].value.unumber, parm[2].value.unumber, &maskm);
        bdmf_session_print(session, "maskm = %u = 0x%x\n", maskm, maskm);
        break;
    }
    case cli_hash_cam_configuration_tm_cfg:
    {
        uint16_t val;
        ag_err = ag_drv_hash_cam_configuration_tm_cfg_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_cam_indirect_op:
    {
        uint8_t cmd;
        ag_err = ag_drv_hash_cam_indirect_op_get(parm[1].value.unumber, &cmd);
        bdmf_session_print(session, "cmd = %u = 0x%x\n", cmd, cmd);
        break;
    }
    case cli_hash_cam_indirect_op_done:
    {
        bdmf_boolean val;
        ag_err = ag_drv_hash_cam_indirect_op_done_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_cam_indirect_addr:
    {
        bdmf_boolean key_ind;
        uint8_t entry_addr;
        ag_err = ag_drv_hash_cam_indirect_addr_get(parm[1].value.unumber, &key_ind, &entry_addr);
        bdmf_session_print(session, "key_ind = %u = 0x%x\n", key_ind, key_ind);
        bdmf_session_print(session, "entry_addr = %u = 0x%x\n", entry_addr, entry_addr);
        break;
    }
    case cli_hash_cam_indirect_vlid_in:
    {
        bdmf_boolean valid;
        ag_err = ag_drv_hash_cam_indirect_vlid_in_get(parm[1].value.unumber, &valid);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_hash_cam_indirect_vlid_out:
    {
        uint8_t valid;
        ag_err = ag_drv_hash_cam_indirect_vlid_out_get(parm[1].value.unumber, &valid);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_hash_cam_indirect_rslt:
    {
        bdmf_boolean match;
        uint8_t index;
        ag_err = ag_drv_hash_cam_indirect_rslt_get(parm[1].value.unumber, &match, &index);
        bdmf_session_print(session, "match = %u = 0x%x\n", match, match);
        bdmf_session_print(session, "index = %u = 0x%x\n", index, index);
        break;
    }
    case cli_hash_cam_indirect_key_in:
    {
        hash_cam_indirect_key_in cam_indirect_key_in;
        ag_err = ag_drv_hash_cam_indirect_key_in_get(parm[1].value.unumber, parm[2].value.unumber, &cam_indirect_key_in);
        bdmf_session_print(session, "key_in[0] = %u = 0x%x\n", cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[0]);
        bdmf_session_print(session, "key_in[1] = %u = 0x%x\n", cam_indirect_key_in.key_in[1], cam_indirect_key_in.key_in[1]);
        break;
    }
    case cli_hash_cam_indirect_key_out:
    {
        hash_cam_indirect_key_out cam_indirect_key_out;
        ag_err = ag_drv_hash_cam_indirect_key_out_get(parm[1].value.unumber, parm[2].value.unumber, &cam_indirect_key_out);
        bdmf_session_print(session, "key_out[0] = %u = 0x%x\n", cam_indirect_key_out.key_out[0], cam_indirect_key_out.key_out[0]);
        bdmf_session_print(session, "key_out[1] = %u = 0x%x\n", cam_indirect_key_out.key_out[1], cam_indirect_key_out.key_out[1]);
        break;
    }
    case cli_hash_cam_bist_bist_status:
    {
        uint32_t value;
        ag_err = ag_drv_hash_cam_bist_bist_status_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_control:
    {
        uint32_t value;
        ag_err = ag_drv_hash_cam_bist_bist_control_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_cache_repair:
    {
        uint32_t value;
        ag_err = ag_drv_hash_cam_bist_bist_cache_repair_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_control:
    {
        uint32_t value;
        ag_err = ag_drv_hash_cam_bist_bist_dbg_control_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_data:
    {
        uint32_t value;
        ag_err = ag_drv_hash_cam_bist_bist_dbg_data_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_hash_intr_ctrl_isr:
    {
        bdmf_boolean mult_match;
        ag_err = ag_drv_hash_intr_ctrl_isr_get(parm[1].value.unumber, &mult_match);
        bdmf_session_print(session, "mult_match = %u = 0x%x\n", mult_match, mult_match);
        break;
    }
    case cli_hash_intr_ctrl_ism:
    {
        uint8_t ism;
        ag_err = ag_drv_hash_intr_ctrl_ism_get(parm[1].value.unumber, &ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_hash_intr_ctrl_ier:
    {
        uint8_t iem;
        ag_err = ag_drv_hash_intr_ctrl_ier_get(parm[1].value.unumber, &iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_hash_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_hash_intr_ctrl_itr_get(parm[1].value.unumber, &ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
        break;
    }
    case cli_hash_debug_dbg0:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg0_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg1:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg1_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg2:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg2_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg3:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg3_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg4:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg4_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg5:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg5_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg6:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg6_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg7:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg7_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg8:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg8_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg9:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg9_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg10:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg10_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg11:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg11_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg12:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg12_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg13:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg13_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg14:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg14_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg15:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg15_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg16:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg16_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg17:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg17_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg18:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg18_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg19:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg19_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg20:
    {
        uint32_t val;
        ag_err = ag_drv_hash_debug_dbg20_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_debug_dbg_sel:
    {
        uint8_t val;
        ag_err = ag_drv_hash_debug_dbg_sel_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_hash_aging_ram_aging:
    {
        bdmf_boolean age;
        ag_err = ag_drv_hash_aging_ram_aging_get(parm[1].value.unumber, parm[2].value.unumber, &age);
        bdmf_session_print(session, "age = %u = 0x%x\n", age, age);
        break;
    }
    case cli_hash_storm_ram_storm:
    {
        uint32_t data;
        ag_err = ag_drv_hash_storm_ram_storm_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_hash_cam_context_ram_context_cam:
    {
        uint32_t data;
        ag_err = ag_drv_hash_cam_context_ram_context_cam_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_hash_context_ram_context:
    {
        uint32_t data;
        ag_err = ag_drv_hash_context_ram_context_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_hash_ram_hash_ram:
    {
        uint32_t data;
        ag_err = ag_drv_hash_ram_hash_ram_get(parm[1].value.unumber, parm[2].value.unumber, &data);
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
static int ag_drv_hash_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t hash_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint8_t tbl_idx = gtmv(m, 3);
        hash_mask mask = {.maskl = gtmv(m, 32), .maskh = gtmv(m, 32)};
        bdmf_session_print(session, "ag_drv_hash_mask_set(%u %u %u %u)\n", hash_id, tbl_idx,
            mask.maskl, mask.maskh);
        ag_err = ag_drv_hash_mask_set(hash_id, tbl_idx, &mask);
        if (!ag_err)
            ag_err = ag_drv_hash_mask_get(hash_id, tbl_idx, &mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_mask_get(%u %u %u %u)\n", hash_id, tbl_idx,
                mask.maskl, mask.maskh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask.maskl != gtmv(m, 32) || mask.maskh != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean block_enable = gtmv(m, 1);
        uint32_t lkup_eng_search_disable = gtmv(m, 18);
        bdmf_boolean storm_move_dis = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_general_configuration_gen_cfg_set(%u %u %u %u)\n", hash_id,
            block_enable, lkup_eng_search_disable, storm_move_dis);
        ag_err = ag_drv_hash_general_configuration_gen_cfg_set(hash_id, block_enable, lkup_eng_search_disable, storm_move_dis);
        if (!ag_err)
            ag_err = ag_drv_hash_general_configuration_gen_cfg_get(hash_id, &block_enable, &lkup_eng_search_disable, &storm_move_dis);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_general_configuration_gen_cfg_get(%u %u %u %u)\n", hash_id,
                block_enable, lkup_eng_search_disable, storm_move_dis);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (block_enable != gtmv(m, 1) || lkup_eng_search_disable != gtmv(m, 18) || storm_move_dis != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        hash_general_configuration_fifo_threshold general_configuration_fifo_threshold = {.hash_table_threshold = gtmv(m, 4), .cam_threshold = gtmv(m, 4), .cntxt_in_threshold = gtmv(m, 4), .storm_threshold = gtmv(m, 6), .storm_threshold_enable = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_hash_general_configuration_fifo_threshold_set(%u %u %u %u %u %u)\n", hash_id,
            general_configuration_fifo_threshold.hash_table_threshold, general_configuration_fifo_threshold.cam_threshold, general_configuration_fifo_threshold.cntxt_in_threshold, general_configuration_fifo_threshold.storm_threshold, 
            general_configuration_fifo_threshold.storm_threshold_enable);
        ag_err = ag_drv_hash_general_configuration_fifo_threshold_set(hash_id, &general_configuration_fifo_threshold);
        if (!ag_err)
            ag_err = ag_drv_hash_general_configuration_fifo_threshold_get(hash_id, &general_configuration_fifo_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_general_configuration_fifo_threshold_get(%u %u %u %u %u %u)\n", hash_id,
                general_configuration_fifo_threshold.hash_table_threshold, general_configuration_fifo_threshold.cam_threshold, general_configuration_fifo_threshold.cntxt_in_threshold, general_configuration_fifo_threshold.storm_threshold, 
                general_configuration_fifo_threshold.storm_threshold_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_configuration_fifo_threshold.hash_table_threshold != gtmv(m, 4) || general_configuration_fifo_threshold.cam_threshold != gtmv(m, 4) || general_configuration_fifo_threshold.cntxt_in_threshold != gtmv(m, 4) || general_configuration_fifo_threshold.storm_threshold != gtmv(m, 6) || general_configuration_fifo_threshold.storm_threshold_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t engine_val = gtmv(m, 18);
        uint16_t key_val = gtmv(m, 14);
        if (!ag_err)
            ag_err = ag_drv_hash_general_configuration_mult_hit_err_get(hash_id, &engine_val, &key_val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_general_configuration_mult_hit_err_get(%u %u %u)\n", hash_id,
                engine_val, key_val);
        }
    }

    {
        bdmf_boolean auto_init_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_general_configuration_auto_init_en_set(%u %u)\n", hash_id,
            auto_init_en);
        ag_err = ag_drv_hash_general_configuration_auto_init_en_set(hash_id, auto_init_en);
        if (!ag_err)
            ag_err = ag_drv_hash_general_configuration_auto_init_en_get(hash_id, &auto_init_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_general_configuration_auto_init_en_get(%u %u)\n", hash_id,
                auto_init_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (auto_init_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean auto_init_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_general_configuration_storm_auto_init_en_set(%u %u)\n", hash_id,
            auto_init_en);
        ag_err = ag_drv_hash_general_configuration_storm_auto_init_en_set(hash_id, auto_init_en);
        if (!ag_err)
            ag_err = ag_drv_hash_general_configuration_storm_auto_init_en_get(hash_id, &auto_init_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_general_configuration_storm_auto_init_en_get(%u %u)\n", hash_id,
                auto_init_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (auto_init_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_hits_get(hash_id, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_hits_get(%u %u)\n", hash_id,
                cnt);
        }
    }

    {
        uint32_t cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_srchs_get(hash_id, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_srchs_get(%u %u)\n", hash_id,
                cnt);
        }
    }

    {
        uint32_t cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_miss_get(hash_id, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_miss_get(%u %u)\n", hash_id,
                cnt);
        }
    }

    {
        uint32_t cnt = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_pm_counters_storm_window_size_set(%u %u)\n", hash_id,
            cnt);
        ag_err = ag_drv_hash_pm_counters_storm_window_size_set(hash_id, cnt);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_storm_window_size_get(hash_id, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_storm_window_size_get(%u %u)\n", hash_id,
                cnt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cnt != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t storm_th = gtmv(m, 16);
        bdmf_boolean storm_window_enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_pm_counters_storm_cfg_set(%u %u %u)\n", hash_id,
            storm_th, storm_window_enable);
        ag_err = ag_drv_hash_pm_counters_storm_cfg_set(hash_id, storm_th, storm_window_enable);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_storm_cfg_get(hash_id, &storm_th, &storm_window_enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_storm_cfg_get(%u %u %u)\n", hash_id,
                storm_th, storm_window_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (storm_th != gtmv(m, 16) || storm_window_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean val = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_pm_counters_frz_cnt_set(%u %u)\n", hash_id,
            val);
        ag_err = ag_drv_hash_pm_counters_frz_cnt_set(hash_id, val);
        if (!ag_err)
            ag_err = ag_drv_hash_pm_counters_frz_cnt_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_pm_counters_frz_cnt_get(%u %u)\n", hash_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tbl_idx = gtmv(m, 3);
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg = {.direct_read_en = gtmv(m, 1), .cam_en = gtmv(m, 1), .move_en = gtmv(m, 1), .storm_en = gtmv(m, 1), .aging_en = gtmv(m, 1), .rslt128_en = gtmv(m, 1), .move_mask = gtmv(m, 20)};
        bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(%u %u %u %u %u %u %u %u %u)\n", hash_id, tbl_idx,
            lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.storm_en, 
            lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.move_mask);
        ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(hash_id, tbl_idx, &lkup_tbl_cfg_cntxt_cfg);
        if (!ag_err)
            ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(hash_id, tbl_idx, &lkup_tbl_cfg_cntxt_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(%u %u %u %u %u %u %u %u %u)\n", hash_id, tbl_idx,
                lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.storm_en, 
                lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.move_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (lkup_tbl_cfg_cntxt_cfg.direct_read_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.cam_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.move_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.storm_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.aging_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.rslt128_en != gtmv(m, 1) || lkup_tbl_cfg_cntxt_cfg.move_mask != gtmv(m, 20))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tbl_idx = gtmv(m, 3);
        uint32_t maskh = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(%u %u %u)\n", hash_id, tbl_idx,
            maskh);
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(hash_id, tbl_idx, maskh);
        if (!ag_err)
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(hash_id, tbl_idx, &maskh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(%u %u %u)\n", hash_id, tbl_idx,
                maskh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (maskh != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tbl_idx = gtmv(m, 3);
        uint32_t maskm = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(%u %u %u)\n", hash_id, tbl_idx,
            maskm);
        ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(hash_id, tbl_idx, maskm);
        if (!ag_err)
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(hash_id, tbl_idx, &maskm);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(%u %u %u)\n", hash_id, tbl_idx,
                maskm);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (maskm != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t val = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_hash_cam_configuration_tm_cfg_set(%u %u)\n", hash_id,
            val);
        ag_err = ag_drv_hash_cam_configuration_tm_cfg_set(hash_id, val);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_configuration_tm_cfg_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_configuration_tm_cfg_get(%u %u)\n", hash_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t cmd = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_set(%u %u)\n", hash_id,
            cmd);
        ag_err = ag_drv_hash_cam_indirect_op_set(hash_id, cmd);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_op_get(hash_id, &cmd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_get(%u %u)\n", hash_id,
                cmd);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cmd != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean val = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_op_done_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_done_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        bdmf_boolean key_ind = gtmv(m, 1);
        uint8_t entry_addr = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_hash_cam_indirect_addr_set(%u %u %u)\n", hash_id,
            key_ind, entry_addr);
        ag_err = ag_drv_hash_cam_indirect_addr_set(hash_id, key_ind, entry_addr);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_addr_get(hash_id, &key_ind, &entry_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_addr_get(%u %u %u)\n", hash_id,
                key_ind, entry_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_ind != gtmv(m, 1) || entry_addr != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_in_set(%u %u)\n", hash_id,
            valid);
        ag_err = ag_drv_hash_cam_indirect_vlid_in_set(hash_id, valid);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_vlid_in_get(hash_id, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_in_get(%u %u)\n", hash_id,
                valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t valid = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_out_set(%u %u)\n", hash_id,
            valid);
        ag_err = ag_drv_hash_cam_indirect_vlid_out_set(hash_id, valid);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_vlid_out_get(hash_id, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_out_get(%u %u)\n", hash_id,
                valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (valid != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean match = gtmv(m, 1);
        uint8_t index = gtmv(m, 6);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_rslt_get(hash_id, &match, &index);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_rslt_get(%u %u %u)\n", hash_id,
                match, index);
        }
    }

    {
        uint8_t zero = gtmv(m, 1);
        hash_cam_indirect_key_in cam_indirect_key_in = {.key_in = {gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_set(%u %u %u %u)\n", hash_id, zero,
            cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
        ag_err = ag_drv_hash_cam_indirect_key_in_set(hash_id, zero, &cam_indirect_key_in);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_key_in_get(hash_id, zero, &cam_indirect_key_in);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_get(%u %u %u %u)\n", hash_id, zero,
                cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cam_indirect_key_in.key_in[0] != gtmv(m, 32) || cam_indirect_key_in.key_in[1] != gtmv(m, 32))
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
        hash_cam_indirect_key_out cam_indirect_key_out = {.key_out = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_hash_cam_indirect_key_out_get(hash_id, zero, &cam_indirect_key_out);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_out_get(%u %u %u %u)\n", hash_id, zero,
                cam_indirect_key_out.key_out[0], cam_indirect_key_out.key_out[1]);
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_bist_bist_status_get(hash_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_status_get(%u %u)\n", hash_id,
                value);
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_control_set(%u %u)\n", hash_id,
            value);
        ag_err = ag_drv_hash_cam_bist_bist_control_set(hash_id, value);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_bist_bist_control_get(hash_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_control_get(%u %u)\n", hash_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_cache_repair_set(%u %u)\n", hash_id,
            value);
        ag_err = ag_drv_hash_cam_bist_bist_cache_repair_set(hash_id, value);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_bist_bist_cache_repair_get(hash_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_cache_repair_get(%u %u)\n", hash_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_control_set(%u %u)\n", hash_id,
            value);
        ag_err = ag_drv_hash_cam_bist_bist_dbg_control_set(hash_id, value);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_bist_bist_dbg_control_get(hash_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_control_get(%u %u)\n", hash_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_set(%u %u)\n", hash_id,
            value);
        ag_err = ag_drv_hash_cam_bist_bist_dbg_data_set(hash_id, value);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_bist_bist_dbg_data_get(hash_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_get(%u %u)\n", hash_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mult_match = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_intr_ctrl_isr_set(%u %u)\n", hash_id,
            mult_match);
        ag_err = ag_drv_hash_intr_ctrl_isr_set(hash_id, mult_match);
        if (!ag_err)
            ag_err = ag_drv_hash_intr_ctrl_isr_get(hash_id, &mult_match);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_intr_ctrl_isr_get(%u %u)\n", hash_id,
                mult_match);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mult_match != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ism = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_hash_intr_ctrl_ism_get(hash_id, &ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ism_get(%u %u)\n", hash_id,
                ism);
        }
    }

    {
        uint8_t iem = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ier_set(%u %u)\n", hash_id,
            iem);
        ag_err = ag_drv_hash_intr_ctrl_ier_set(hash_id, iem);
        if (!ag_err)
            ag_err = ag_drv_hash_intr_ctrl_ier_get(hash_id, &iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ier_get(%u %u)\n", hash_id,
                iem);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (iem != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ist = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_intr_ctrl_itr_set(%u %u)\n", hash_id,
            ist);
        ag_err = ag_drv_hash_intr_ctrl_itr_set(hash_id, ist);
        if (!ag_err)
            ag_err = ag_drv_hash_intr_ctrl_itr_get(hash_id, &ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_intr_ctrl_itr_get(%u %u)\n", hash_id,
                ist);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ist != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg0_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg0_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg1_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg1_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg2_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg2_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg3_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg3_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg4_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg4_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg5_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg5_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg6_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg6_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg7_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg7_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg8_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg8_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg9_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg9_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg10_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg10_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg11_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg11_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg12_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg12_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg13_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg13_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg14_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg14_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg15_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg15_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg16_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg16_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg17_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg17_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg18_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg18_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg19_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg19_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg20_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg20_get(%u %u)\n", hash_id,
                val);
        }
    }

    {
        uint8_t val = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_hash_debug_dbg_sel_set(%u %u)\n", hash_id,
            val);
        ag_err = ag_drv_hash_debug_dbg_sel_set(hash_id, val);
        if (!ag_err)
            ag_err = ag_drv_hash_debug_dbg_sel_get(hash_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_debug_dbg_sel_get(%u %u)\n", hash_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t entry_idx = gtmv(m, 17);
        bdmf_boolean age = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_set(%u %u %u)\n", hash_id, entry_idx,
            age);
        ag_err = ag_drv_hash_aging_ram_aging_set(hash_id, entry_idx, age);
        if (!ag_err)
            ag_err = ag_drv_hash_aging_ram_aging_get(hash_id, entry_idx, &age);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_get(%u %u %u)\n", hash_id, entry_idx,
                age);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (age != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t idx = gtmv(m, 12);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_storm_ram_storm_set(%u %u %u)\n", hash_id, idx,
            data);
        ag_err = ag_drv_hash_storm_ram_storm_set(hash_id, idx, data);
        if (!ag_err)
            ag_err = ag_drv_hash_storm_ram_storm_get(hash_id, idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_storm_ram_storm_get(%u %u %u)\n", hash_id, idx,
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
        uint8_t tbl_idx = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_cam_context_ram_context_cam_set(%u %u %u)\n", hash_id, tbl_idx,
            data);
        ag_err = ag_drv_hash_cam_context_ram_context_cam_set(hash_id, tbl_idx, data);
        if (!ag_err)
            ag_err = ag_drv_hash_cam_context_ram_context_cam_get(hash_id, tbl_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_context_ram_context_cam_get(%u %u %u)\n", hash_id, tbl_idx,
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
        uint8_t tbl_idx = gtmv(m, 17);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_context_ram_context_set(%u %u %u)\n", hash_id, tbl_idx,
            data);
        ag_err = ag_drv_hash_context_ram_context_set(hash_id, tbl_idx, data);
        if (!ag_err)
            ag_err = ag_drv_hash_context_ram_context_get(hash_id, tbl_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_context_ram_context_get(%u %u %u)\n", hash_id, tbl_idx,
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
        uint8_t tbl_idx = gtmv(m, 19);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_hash_ram_hash_ram_set(%u %u %u)\n", hash_id, tbl_idx,
            data);
        ag_err = ag_drv_hash_ram_hash_ram_set(hash_id, tbl_idx, data);
        if (!ag_err)
            ag_err = ag_drv_hash_ram_hash_ram_get(hash_id, tbl_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_hash_ram_hash_ram_get(%u %u %u)\n", hash_id, tbl_idx,
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
static int ag_drv_hash_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t hash_id = parm[2].value.unumber;
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
    case cli_hash_mask:
    {
        uint8_t max_tbl_idx = 8;
        uint8_t tbl_idx = gtmv(m, 3);
        hash_mask mask = {
            .maskl = gtmv(m, 32), 
            .maskh = gtmv(m, 32) };

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_mask_set(%u %u %u %u)\n", hash_id, tbl_idx,
                mask.maskl, mask.maskh);
            ag_err = ag_drv_hash_mask_set(hash_id, tbl_idx, &mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask.maskl = gtmv(m, 32);
        mask.maskh = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_mask_set(%u %u %u %u)\n", hash_id, tbl_idx,
                mask.maskl, mask.maskh);
            ag_err = ag_drv_hash_mask_set(hash_id, tbl_idx, &mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_mask_get(hash_id, tbl_idx, &mask);

            bdmf_session_print(session, "ag_drv_hash_mask_get(%u %u %u %u)\n", hash_id, tbl_idx,
                mask.maskl, mask.maskh);

            if (mask.maskl != gtmv(m, 32) || 
                mask.maskh != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of mask completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_lkup_tbl_cfg_cntxt_cfg:
    {
        uint8_t max_tbl_idx = 8;
        uint8_t tbl_idx = gtmv(m, 3);
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg = {
            .direct_read_en = gtmv(m, 1), 
            .cam_en = gtmv(m, 1), 
            .move_en = gtmv(m, 1), 
            .storm_en = gtmv(m, 1), 
            .aging_en = gtmv(m, 1), 
            .rslt128_en = gtmv(m, 1), 
            .move_mask = gtmv(m, 20) };

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(%u %u %u %u %u %u %u %u %u)\n", hash_id, tbl_idx,
                lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.storm_en, 
                lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.move_mask);
            ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(hash_id, tbl_idx, &lkup_tbl_cfg_cntxt_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        lkup_tbl_cfg_cntxt_cfg.direct_read_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.cam_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.move_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.storm_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.aging_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.rslt128_en = gtmv(m, 1);
        lkup_tbl_cfg_cntxt_cfg.move_mask = gtmv(m, 20);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(%u %u %u %u %u %u %u %u %u)\n", hash_id, tbl_idx,
                lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.storm_en, 
                lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.move_mask);
            ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(hash_id, tbl_idx, &lkup_tbl_cfg_cntxt_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(hash_id, tbl_idx, &lkup_tbl_cfg_cntxt_cfg);

            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(%u %u %u %u %u %u %u %u %u)\n", hash_id, tbl_idx,
                lkup_tbl_cfg_cntxt_cfg.direct_read_en, lkup_tbl_cfg_cntxt_cfg.cam_en, lkup_tbl_cfg_cntxt_cfg.move_en, lkup_tbl_cfg_cntxt_cfg.storm_en, 
                lkup_tbl_cfg_cntxt_cfg.aging_en, lkup_tbl_cfg_cntxt_cfg.rslt128_en, lkup_tbl_cfg_cntxt_cfg.move_mask);

            if (lkup_tbl_cfg_cntxt_cfg.direct_read_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.cam_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.move_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.storm_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.aging_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.rslt128_en != gtmv(m, 1) || 
                lkup_tbl_cfg_cntxt_cfg.move_mask != gtmv(m, 20) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of lkup_tbl_cfg_cntxt_cfg completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_lkup_tbl_cfg_key_mask_highest:
    {
        uint8_t max_tbl_idx = 8;
        uint8_t tbl_idx = gtmv(m, 3);
        uint32_t maskh = gtmv(m, 32);

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(%u %u %u)\n", hash_id, tbl_idx,
                maskh);
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(hash_id, tbl_idx, maskh);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        maskh = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(%u %u %u)\n", hash_id, tbl_idx,
                maskh);
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_set(hash_id, tbl_idx, maskh);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(hash_id, tbl_idx, &maskh);

            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_highest_get(%u %u %u)\n", hash_id, tbl_idx,
                maskh);

            if (maskh != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of lkup_tbl_cfg_key_mask_highest completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_lkup_tbl_cfg_key_mask_mid:
    {
        uint8_t max_tbl_idx = 8;
        uint8_t tbl_idx = gtmv(m, 3);
        uint32_t maskm = gtmv(m, 32);

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(%u %u %u)\n", hash_id, tbl_idx,
                maskm);
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(hash_id, tbl_idx, maskm);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        maskm = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(%u %u %u)\n", hash_id, tbl_idx,
                maskm);
            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_set(hash_id, tbl_idx, maskm);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(hash_id, tbl_idx, &maskm);

            bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_key_mask_mid_get(%u %u %u)\n", hash_id, tbl_idx,
                maskm);

            if (maskm != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of lkup_tbl_cfg_key_mask_mid completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_cam_indirect_key_in:
    {
        uint8_t max_zero = 2;
        uint8_t zero = gtmv(m, 1);
        hash_cam_indirect_key_in cam_indirect_key_in = {.key_in = {gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_set(%u %u %u %u)\n", hash_id, zero,
                cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
            ag_err = ag_drv_hash_cam_indirect_key_in_set(hash_id, zero, &cam_indirect_key_in);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        cam_indirect_key_in.key_in[0] = gtmv(m, 32);
        cam_indirect_key_in.key_in[1] = gtmv(m, 32);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_set(%u %u %u %u)\n", hash_id, zero,
                cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
            ag_err = ag_drv_hash_cam_indirect_key_in_set(hash_id, zero, &cam_indirect_key_in);
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

            ag_err = ag_drv_hash_cam_indirect_key_in_get(hash_id, zero, &cam_indirect_key_in);

            bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_get(%u %u %u %u)\n", hash_id, zero,
                cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);

            if (cam_indirect_key_in.key_in[0] != gtmv(m, 32) || 
                cam_indirect_key_in.key_in[1] != gtmv(m, 32) || 
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
        bdmf_session_print(session, "Test of cam_indirect_key_in completed. Number of tested entries %u.\n", max_zero);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_aging_ram_aging:
    {
        uint16_t max_entry_idx = 147584;
        uint16_t entry_idx = gtmv(m, 17);
        bdmf_boolean age = gtmv(m, 1);

        if ((start_idx >= max_entry_idx) || (stop_idx >= max_entry_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_entry_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (entry_idx = 0; entry_idx < max_entry_idx; entry_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_set(%u %u %u)\n", hash_id, entry_idx,
                age);
            ag_err = ag_drv_hash_aging_ram_aging_set(hash_id, entry_idx, age);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", entry_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        age = gtmv(m, 1);

        for (entry_idx = start_idx; entry_idx <= stop_idx; entry_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_set(%u %u %u)\n", hash_id, entry_idx,
                age);
            ag_err = ag_drv_hash_aging_ram_aging_set(hash_id, entry_idx, age);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", entry_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (entry_idx = 0; entry_idx < max_entry_idx; entry_idx++)
        {
            if (entry_idx < start_idx || entry_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_aging_ram_aging_get(hash_id, entry_idx, &age);

            bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_get(%u %u %u)\n", hash_id, entry_idx,
                age);

            if (age != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", entry_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of aging_ram_aging completed. Number of tested entries %u.\n", max_entry_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_storm_ram_storm:
    {
        uint32_t max_idx = 4096;
        uint32_t idx = gtmv(m, 12);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_storm_ram_storm_set(%u %u %u)\n", hash_id, idx,
                data);
            ag_err = ag_drv_hash_storm_ram_storm_set(hash_id, idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_storm_ram_storm_set(%u %u %u)\n", hash_id, idx,
                data);
            ag_err = ag_drv_hash_storm_ram_storm_set(hash_id, idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (idx = 0; idx < max_idx; idx++)
        {
            if (idx < start_idx || idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_storm_ram_storm_get(hash_id, idx, &data);

            bdmf_session_print(session, "ag_drv_hash_storm_ram_storm_get(%u %u %u)\n", hash_id, idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of storm_ram_storm completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_cam_context_ram_context_cam:
    {
        uint8_t max_tbl_idx = 512;
        uint8_t tbl_idx = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_context_ram_context_cam_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_cam_context_ram_context_cam_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_cam_context_ram_context_cam_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_cam_context_ram_context_cam_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_cam_context_ram_context_cam_get(hash_id, tbl_idx, &data);

            bdmf_session_print(session, "ag_drv_hash_cam_context_ram_context_cam_get(%u %u %u)\n", hash_id, tbl_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cam_context_ram_context_cam completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_context_ram_context:
    {
        uint8_t max_tbl_idx = 245760;
        uint8_t tbl_idx = gtmv(m, 17);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_context_ram_context_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_context_ram_context_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_context_ram_context_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_context_ram_context_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_context_ram_context_get(hash_id, tbl_idx, &data);

            bdmf_session_print(session, "ag_drv_hash_context_ram_context_get(%u %u %u)\n", hash_id, tbl_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of context_ram_context completed. Number of tested entries %u.\n", max_tbl_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_hash_ram_hash_ram:
    {
        uint8_t max_tbl_idx = 589824;
        uint8_t tbl_idx = gtmv(m, 19);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_tbl_idx) || (stop_idx >= max_tbl_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tbl_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_ram_hash_ram_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_ram_hash_ram_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (tbl_idx = start_idx; tbl_idx <= stop_idx; tbl_idx++)
        {
            bdmf_session_print(session, "ag_drv_hash_ram_hash_ram_set(%u %u %u)\n", hash_id, tbl_idx,
                data);
            ag_err = ag_drv_hash_ram_hash_ram_set(hash_id, tbl_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tbl_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tbl_idx = 0; tbl_idx < max_tbl_idx; tbl_idx++)
        {
            if (tbl_idx < start_idx || tbl_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_hash_ram_hash_ram_get(hash_id, tbl_idx, &data);

            bdmf_session_print(session, "ag_drv_hash_ram_hash_ram_get(%u %u %u)\n", hash_id, tbl_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tbl_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ram_hash_ram completed. Number of tested entries %u.\n", max_tbl_idx);
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
static int ag_drv_hash_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_general_configuration_gen_cfg: reg = &RU_REG(HASH, GENERAL_CONFIGURATION_GEN_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_fifo_threshold: reg = &RU_REG(HASH, GENERAL_CONFIGURATION_FIFO_THRESHOLD); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_mult_hit_err: reg = &RU_REG(HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_auto_init_en: reg = &RU_REG(HASH, GENERAL_CONFIGURATION_AUTO_INIT_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_storm_auto_init_en: reg = &RU_REG(HASH, GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hits: reg = &RU_REG(HASH, PM_COUNTERS_HITS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_srchs: reg = &RU_REG(HASH, PM_COUNTERS_SRCHS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_miss: reg = &RU_REG(HASH, PM_COUNTERS_MISS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_storm_window_size: reg = &RU_REG(HASH, PM_COUNTERS_STORM_WINDOW_SIZE); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_storm_cfg: reg = &RU_REG(HASH, PM_COUNTERS_STORM_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_frz_cnt: reg = &RU_REG(HASH, PM_COUNTERS_FRZ_CNT); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_cntxt_cfg: reg = &RU_REG(HASH, LKUP_TBL_CFG_CNTXT_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_highest: reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_HIGHEST); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_high: reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_HIGH); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_mid: reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_MID); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_low: reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_LOW); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_configuration_tm_cfg: reg = &RU_REG(HASH, CAM_CONFIGURATION_TM_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_op: reg = &RU_REG(HASH, CAM_INDIRECT_OP); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_op_done: reg = &RU_REG(HASH, CAM_INDIRECT_OP_DONE); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_addr: reg = &RU_REG(HASH, CAM_INDIRECT_ADDR); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_vlid_in: reg = &RU_REG(HASH, CAM_INDIRECT_VLID_IN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_vlid_out: reg = &RU_REG(HASH, CAM_INDIRECT_VLID_OUT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_rslt: reg = &RU_REG(HASH, CAM_INDIRECT_RSLT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_key_in: reg = &RU_REG(HASH, CAM_INDIRECT_KEY_IN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_key_out: reg = &RU_REG(HASH, CAM_INDIRECT_KEY_OUT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_status: reg = &RU_REG(HASH, CAM_BIST_BIST_STATUS); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_control: reg = &RU_REG(HASH, CAM_BIST_BIST_CONTROL); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_cache_repair: reg = &RU_REG(HASH, CAM_BIST_BIST_CACHE_REPAIR); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_control: reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_CONTROL); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_data: reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_DATA); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_isr: reg = &RU_REG(HASH, INTR_CTRL_ISR); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_ism: reg = &RU_REG(HASH, INTR_CTRL_ISM); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_ier: reg = &RU_REG(HASH, INTR_CTRL_IER); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_itr: reg = &RU_REG(HASH, INTR_CTRL_ITR); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg0: reg = &RU_REG(HASH, DEBUG_DBG0); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg1: reg = &RU_REG(HASH, DEBUG_DBG1); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg2: reg = &RU_REG(HASH, DEBUG_DBG2); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg3: reg = &RU_REG(HASH, DEBUG_DBG3); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg4: reg = &RU_REG(HASH, DEBUG_DBG4); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg5: reg = &RU_REG(HASH, DEBUG_DBG5); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg6: reg = &RU_REG(HASH, DEBUG_DBG6); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg7: reg = &RU_REG(HASH, DEBUG_DBG7); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg8: reg = &RU_REG(HASH, DEBUG_DBG8); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg9: reg = &RU_REG(HASH, DEBUG_DBG9); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg10: reg = &RU_REG(HASH, DEBUG_DBG10); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg11: reg = &RU_REG(HASH, DEBUG_DBG11); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg12: reg = &RU_REG(HASH, DEBUG_DBG12); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg13: reg = &RU_REG(HASH, DEBUG_DBG13); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg14: reg = &RU_REG(HASH, DEBUG_DBG14); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg15: reg = &RU_REG(HASH, DEBUG_DBG15); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg16: reg = &RU_REG(HASH, DEBUG_DBG16); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg17: reg = &RU_REG(HASH, DEBUG_DBG17); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg18: reg = &RU_REG(HASH, DEBUG_DBG18); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg19: reg = &RU_REG(HASH, DEBUG_DBG19); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg20: reg = &RU_REG(HASH, DEBUG_DBG20); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg_sel: reg = &RU_REG(HASH, DEBUG_DBG_SEL); blk = &RU_BLK(HASH); break;
    case bdmf_address_aging_ram_aging: reg = &RU_REG(HASH, AGING_RAM_AGING); blk = &RU_BLK(HASH); break;
    case bdmf_address_storm_ram_storm: reg = &RU_REG(HASH, STORM_RAM_STORM); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_context_ram_context_cam: reg = &RU_REG(HASH, CAM_CONTEXT_RAM_CONTEXT_CAM); blk = &RU_BLK(HASH); break;
    case bdmf_address_context_ram_context: reg = &RU_REG(HASH, CONTEXT_RAM_CONTEXT); blk = &RU_BLK(HASH); break;
    case bdmf_address_ram_hash_ram: reg = &RU_REG(HASH, RAM_HASH_RAM); blk = &RU_BLK(HASH); break;
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

bdmfmon_handle_t ag_drv_hash_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "hash", "hash", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_mask[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maskl", "maskl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maskh", "maskh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_gen_cfg[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("block_enable", "block_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lkup_eng_search_disable", "lkup_eng_search_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_move_dis", "storm_move_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_fifo_threshold[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hash_table_threshold", "hash_table_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cam_threshold", "cam_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cntxt_in_threshold", "cntxt_in_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_threshold", "storm_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_threshold_enable", "storm_threshold_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_auto_init_en[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("auto_init_en", "auto_init_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_storm_auto_init_en[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("auto_init_en", "auto_init_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_storm_window_size[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cnt", "cnt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_storm_cfg[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_th", "storm_th", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_window_enable", "storm_window_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_frz_cnt[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_cntxt_cfg[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("direct_read_en", "direct_read_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cam_en", "cam_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("move_en", "move_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("storm_en", "storm_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aging_en", "aging_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rslt128_en", "rslt128_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("move_mask", "move_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_key_mask_highest[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maskh", "maskh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_key_mask_mid[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maskm", "maskm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_configuration_tm_cfg[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_op[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_addr[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_ind", "key_ind", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_addr", "entry_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_vlid_in[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_vlid_out[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_key_in[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_in0", "key_in0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("key_in1", "key_in1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_control[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_cache_repair[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_control[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_data[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mult_match", "mult_match", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_dbg_sel[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aging_ram_aging[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_idx", "entry_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("age", "age", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_storm_ram_storm[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_context_ram_context_cam[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_ram_context[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ram_hash_ram[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_hash_mask, .parms = set_mask },
            { .name = "general_configuration_gen_cfg", .val = cli_hash_general_configuration_gen_cfg, .parms = set_general_configuration_gen_cfg },
            { .name = "general_configuration_fifo_threshold", .val = cli_hash_general_configuration_fifo_threshold, .parms = set_general_configuration_fifo_threshold },
            { .name = "general_configuration_auto_init_en", .val = cli_hash_general_configuration_auto_init_en, .parms = set_general_configuration_auto_init_en },
            { .name = "general_configuration_storm_auto_init_en", .val = cli_hash_general_configuration_storm_auto_init_en, .parms = set_general_configuration_storm_auto_init_en },
            { .name = "pm_counters_storm_window_size", .val = cli_hash_pm_counters_storm_window_size, .parms = set_pm_counters_storm_window_size },
            { .name = "pm_counters_storm_cfg", .val = cli_hash_pm_counters_storm_cfg, .parms = set_pm_counters_storm_cfg },
            { .name = "pm_counters_frz_cnt", .val = cli_hash_pm_counters_frz_cnt, .parms = set_pm_counters_frz_cnt },
            { .name = "lkup_tbl_cfg_cntxt_cfg", .val = cli_hash_lkup_tbl_cfg_cntxt_cfg, .parms = set_lkup_tbl_cfg_cntxt_cfg },
            { .name = "lkup_tbl_cfg_key_mask_highest", .val = cli_hash_lkup_tbl_cfg_key_mask_highest, .parms = set_lkup_tbl_cfg_key_mask_highest },
            { .name = "lkup_tbl_cfg_key_mask_mid", .val = cli_hash_lkup_tbl_cfg_key_mask_mid, .parms = set_lkup_tbl_cfg_key_mask_mid },
            { .name = "cam_configuration_tm_cfg", .val = cli_hash_cam_configuration_tm_cfg, .parms = set_cam_configuration_tm_cfg },
            { .name = "cam_indirect_op", .val = cli_hash_cam_indirect_op, .parms = set_cam_indirect_op },
            { .name = "cam_indirect_addr", .val = cli_hash_cam_indirect_addr, .parms = set_cam_indirect_addr },
            { .name = "cam_indirect_vlid_in", .val = cli_hash_cam_indirect_vlid_in, .parms = set_cam_indirect_vlid_in },
            { .name = "cam_indirect_vlid_out", .val = cli_hash_cam_indirect_vlid_out, .parms = set_cam_indirect_vlid_out },
            { .name = "cam_indirect_key_in", .val = cli_hash_cam_indirect_key_in, .parms = set_cam_indirect_key_in },
            { .name = "cam_bist_bist_control", .val = cli_hash_cam_bist_bist_control, .parms = set_cam_bist_bist_control },
            { .name = "cam_bist_bist_cache_repair", .val = cli_hash_cam_bist_bist_cache_repair, .parms = set_cam_bist_bist_cache_repair },
            { .name = "cam_bist_bist_dbg_control", .val = cli_hash_cam_bist_bist_dbg_control, .parms = set_cam_bist_bist_dbg_control },
            { .name = "cam_bist_bist_dbg_data", .val = cli_hash_cam_bist_bist_dbg_data, .parms = set_cam_bist_bist_dbg_data },
            { .name = "intr_ctrl_isr", .val = cli_hash_intr_ctrl_isr, .parms = set_intr_ctrl_isr },
            { .name = "intr_ctrl_ier", .val = cli_hash_intr_ctrl_ier, .parms = set_intr_ctrl_ier },
            { .name = "intr_ctrl_itr", .val = cli_hash_intr_ctrl_itr, .parms = set_intr_ctrl_itr },
            { .name = "debug_dbg_sel", .val = cli_hash_debug_dbg_sel, .parms = set_debug_dbg_sel },
            { .name = "aging_ram_aging", .val = cli_hash_aging_ram_aging, .parms = set_aging_ram_aging },
            { .name = "storm_ram_storm", .val = cli_hash_storm_ram_storm, .parms = set_storm_ram_storm },
            { .name = "cam_context_ram_context_cam", .val = cli_hash_cam_context_ram_context_cam, .parms = set_cam_context_ram_context_cam },
            { .name = "context_ram_context", .val = cli_hash_context_ram_context, .parms = set_context_ram_context },
            { .name = "ram_hash_ram", .val = cli_hash_ram_hash_ram, .parms = set_ram_hash_ram },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_hash_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_mask[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_lkup_tbl_cfg_cntxt_cfg[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_lkup_tbl_cfg_key_mask_highest[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_lkup_tbl_cfg_key_mask_mid[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cam_indirect_key_in[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cam_indirect_key_out[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_aging_ram_aging[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_idx", "entry_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_storm_ram_storm[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cam_context_ram_context_cam[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_context_ram_context[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ram_hash_ram[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_hash_mask, .parms = get_mask },
            { .name = "general_configuration_gen_cfg", .val = cli_hash_general_configuration_gen_cfg, .parms = get_default },
            { .name = "general_configuration_fifo_threshold", .val = cli_hash_general_configuration_fifo_threshold, .parms = get_default },
            { .name = "general_configuration_mult_hit_err", .val = cli_hash_general_configuration_mult_hit_err, .parms = get_default },
            { .name = "general_configuration_auto_init_en", .val = cli_hash_general_configuration_auto_init_en, .parms = get_default },
            { .name = "general_configuration_storm_auto_init_en", .val = cli_hash_general_configuration_storm_auto_init_en, .parms = get_default },
            { .name = "pm_counters_hits", .val = cli_hash_pm_counters_hits, .parms = get_default },
            { .name = "pm_counters_srchs", .val = cli_hash_pm_counters_srchs, .parms = get_default },
            { .name = "pm_counters_miss", .val = cli_hash_pm_counters_miss, .parms = get_default },
            { .name = "pm_counters_storm_window_size", .val = cli_hash_pm_counters_storm_window_size, .parms = get_default },
            { .name = "pm_counters_storm_cfg", .val = cli_hash_pm_counters_storm_cfg, .parms = get_default },
            { .name = "pm_counters_frz_cnt", .val = cli_hash_pm_counters_frz_cnt, .parms = get_default },
            { .name = "lkup_tbl_cfg_cntxt_cfg", .val = cli_hash_lkup_tbl_cfg_cntxt_cfg, .parms = get_lkup_tbl_cfg_cntxt_cfg },
            { .name = "lkup_tbl_cfg_key_mask_highest", .val = cli_hash_lkup_tbl_cfg_key_mask_highest, .parms = get_lkup_tbl_cfg_key_mask_highest },
            { .name = "lkup_tbl_cfg_key_mask_mid", .val = cli_hash_lkup_tbl_cfg_key_mask_mid, .parms = get_lkup_tbl_cfg_key_mask_mid },
            { .name = "cam_configuration_tm_cfg", .val = cli_hash_cam_configuration_tm_cfg, .parms = get_default },
            { .name = "cam_indirect_op", .val = cli_hash_cam_indirect_op, .parms = get_default },
            { .name = "cam_indirect_op_done", .val = cli_hash_cam_indirect_op_done, .parms = get_default },
            { .name = "cam_indirect_addr", .val = cli_hash_cam_indirect_addr, .parms = get_default },
            { .name = "cam_indirect_vlid_in", .val = cli_hash_cam_indirect_vlid_in, .parms = get_default },
            { .name = "cam_indirect_vlid_out", .val = cli_hash_cam_indirect_vlid_out, .parms = get_default },
            { .name = "cam_indirect_rslt", .val = cli_hash_cam_indirect_rslt, .parms = get_default },
            { .name = "cam_indirect_key_in", .val = cli_hash_cam_indirect_key_in, .parms = get_cam_indirect_key_in },
            { .name = "cam_indirect_key_out", .val = cli_hash_cam_indirect_key_out, .parms = get_cam_indirect_key_out },
            { .name = "cam_bist_bist_status", .val = cli_hash_cam_bist_bist_status, .parms = get_default },
            { .name = "cam_bist_bist_control", .val = cli_hash_cam_bist_bist_control, .parms = get_default },
            { .name = "cam_bist_bist_cache_repair", .val = cli_hash_cam_bist_bist_cache_repair, .parms = get_default },
            { .name = "cam_bist_bist_dbg_control", .val = cli_hash_cam_bist_bist_dbg_control, .parms = get_default },
            { .name = "cam_bist_bist_dbg_data", .val = cli_hash_cam_bist_bist_dbg_data, .parms = get_default },
            { .name = "intr_ctrl_isr", .val = cli_hash_intr_ctrl_isr, .parms = get_default },
            { .name = "intr_ctrl_ism", .val = cli_hash_intr_ctrl_ism, .parms = get_default },
            { .name = "intr_ctrl_ier", .val = cli_hash_intr_ctrl_ier, .parms = get_default },
            { .name = "intr_ctrl_itr", .val = cli_hash_intr_ctrl_itr, .parms = get_default },
            { .name = "debug_dbg0", .val = cli_hash_debug_dbg0, .parms = get_default },
            { .name = "debug_dbg1", .val = cli_hash_debug_dbg1, .parms = get_default },
            { .name = "debug_dbg2", .val = cli_hash_debug_dbg2, .parms = get_default },
            { .name = "debug_dbg3", .val = cli_hash_debug_dbg3, .parms = get_default },
            { .name = "debug_dbg4", .val = cli_hash_debug_dbg4, .parms = get_default },
            { .name = "debug_dbg5", .val = cli_hash_debug_dbg5, .parms = get_default },
            { .name = "debug_dbg6", .val = cli_hash_debug_dbg6, .parms = get_default },
            { .name = "debug_dbg7", .val = cli_hash_debug_dbg7, .parms = get_default },
            { .name = "debug_dbg8", .val = cli_hash_debug_dbg8, .parms = get_default },
            { .name = "debug_dbg9", .val = cli_hash_debug_dbg9, .parms = get_default },
            { .name = "debug_dbg10", .val = cli_hash_debug_dbg10, .parms = get_default },
            { .name = "debug_dbg11", .val = cli_hash_debug_dbg11, .parms = get_default },
            { .name = "debug_dbg12", .val = cli_hash_debug_dbg12, .parms = get_default },
            { .name = "debug_dbg13", .val = cli_hash_debug_dbg13, .parms = get_default },
            { .name = "debug_dbg14", .val = cli_hash_debug_dbg14, .parms = get_default },
            { .name = "debug_dbg15", .val = cli_hash_debug_dbg15, .parms = get_default },
            { .name = "debug_dbg16", .val = cli_hash_debug_dbg16, .parms = get_default },
            { .name = "debug_dbg17", .val = cli_hash_debug_dbg17, .parms = get_default },
            { .name = "debug_dbg18", .val = cli_hash_debug_dbg18, .parms = get_default },
            { .name = "debug_dbg19", .val = cli_hash_debug_dbg19, .parms = get_default },
            { .name = "debug_dbg20", .val = cli_hash_debug_dbg20, .parms = get_default },
            { .name = "debug_dbg_sel", .val = cli_hash_debug_dbg_sel, .parms = get_default },
            { .name = "aging_ram_aging", .val = cli_hash_aging_ram_aging, .parms = get_aging_ram_aging },
            { .name = "storm_ram_storm", .val = cli_hash_storm_ram_storm, .parms = get_storm_ram_storm },
            { .name = "cam_context_ram_context_cam", .val = cli_hash_cam_context_ram_context_cam, .parms = get_cam_context_ram_context_cam },
            { .name = "context_ram_context", .val = cli_hash_context_ram_context, .parms = get_context_ram_context },
            { .name = "ram_hash_ram", .val = cli_hash_ram_hash_ram, .parms = get_ram_hash_ram },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_hash_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_hash_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("hash_id", "hash_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_hash_mask, .parms = ext_test_default},
            { .name = "lkup_tbl_cfg_cntxt_cfg", .val = cli_hash_lkup_tbl_cfg_cntxt_cfg, .parms = ext_test_default},
            { .name = "lkup_tbl_cfg_key_mask_highest", .val = cli_hash_lkup_tbl_cfg_key_mask_highest, .parms = ext_test_default},
            { .name = "lkup_tbl_cfg_key_mask_mid", .val = cli_hash_lkup_tbl_cfg_key_mask_mid, .parms = ext_test_default},
            { .name = "cam_indirect_key_in", .val = cli_hash_cam_indirect_key_in, .parms = ext_test_default},
            { .name = "aging_ram_aging", .val = cli_hash_aging_ram_aging, .parms = ext_test_default},
            { .name = "storm_ram_storm", .val = cli_hash_storm_ram_storm, .parms = ext_test_default},
            { .name = "cam_context_ram_context_cam", .val = cli_hash_cam_context_ram_context_cam, .parms = ext_test_default},
            { .name = "context_ram_context", .val = cli_hash_context_ram_context, .parms = ext_test_default},
            { .name = "ram_hash_ram", .val = cli_hash_ram_hash_ram, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_hash_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "GENERAL_CONFIGURATION_GEN_CFG", .val = bdmf_address_general_configuration_gen_cfg },
            { .name = "GENERAL_CONFIGURATION_FIFO_THRESHOLD", .val = bdmf_address_general_configuration_fifo_threshold },
            { .name = "GENERAL_CONFIGURATION_MULT_HIT_ERR", .val = bdmf_address_general_configuration_mult_hit_err },
            { .name = "GENERAL_CONFIGURATION_AUTO_INIT_EN", .val = bdmf_address_general_configuration_auto_init_en },
            { .name = "GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN", .val = bdmf_address_general_configuration_storm_auto_init_en },
            { .name = "PM_COUNTERS_HITS", .val = bdmf_address_pm_counters_hits },
            { .name = "PM_COUNTERS_SRCHS", .val = bdmf_address_pm_counters_srchs },
            { .name = "PM_COUNTERS_MISS", .val = bdmf_address_pm_counters_miss },
            { .name = "PM_COUNTERS_STORM_WINDOW_SIZE", .val = bdmf_address_pm_counters_storm_window_size },
            { .name = "PM_COUNTERS_STORM_CFG", .val = bdmf_address_pm_counters_storm_cfg },
            { .name = "PM_COUNTERS_FRZ_CNT", .val = bdmf_address_pm_counters_frz_cnt },
            { .name = "LKUP_TBL_CFG_CNTXT_CFG", .val = bdmf_address_lkup_tbl_cfg_cntxt_cfg },
            { .name = "LKUP_TBL_CFG_KEY_MASK_HIGHEST", .val = bdmf_address_lkup_tbl_cfg_key_mask_highest },
            { .name = "LKUP_TBL_CFG_KEY_MASK_HIGH", .val = bdmf_address_lkup_tbl_cfg_key_mask_high },
            { .name = "LKUP_TBL_CFG_KEY_MASK_MID", .val = bdmf_address_lkup_tbl_cfg_key_mask_mid },
            { .name = "LKUP_TBL_CFG_KEY_MASK_LOW", .val = bdmf_address_lkup_tbl_cfg_key_mask_low },
            { .name = "CAM_CONFIGURATION_TM_CFG", .val = bdmf_address_cam_configuration_tm_cfg },
            { .name = "CAM_INDIRECT_OP", .val = bdmf_address_cam_indirect_op },
            { .name = "CAM_INDIRECT_OP_DONE", .val = bdmf_address_cam_indirect_op_done },
            { .name = "CAM_INDIRECT_ADDR", .val = bdmf_address_cam_indirect_addr },
            { .name = "CAM_INDIRECT_VLID_IN", .val = bdmf_address_cam_indirect_vlid_in },
            { .name = "CAM_INDIRECT_VLID_OUT", .val = bdmf_address_cam_indirect_vlid_out },
            { .name = "CAM_INDIRECT_RSLT", .val = bdmf_address_cam_indirect_rslt },
            { .name = "CAM_INDIRECT_KEY_IN", .val = bdmf_address_cam_indirect_key_in },
            { .name = "CAM_INDIRECT_KEY_OUT", .val = bdmf_address_cam_indirect_key_out },
            { .name = "CAM_BIST_BIST_STATUS", .val = bdmf_address_cam_bist_bist_status },
            { .name = "CAM_BIST_BIST_CONTROL", .val = bdmf_address_cam_bist_bist_control },
            { .name = "CAM_BIST_BIST_CACHE_REPAIR", .val = bdmf_address_cam_bist_bist_cache_repair },
            { .name = "CAM_BIST_BIST_DBG_CONTROL", .val = bdmf_address_cam_bist_bist_dbg_control },
            { .name = "CAM_BIST_BIST_DBG_DATA", .val = bdmf_address_cam_bist_bist_dbg_data },
            { .name = "INTR_CTRL_ISR", .val = bdmf_address_intr_ctrl_isr },
            { .name = "INTR_CTRL_ISM", .val = bdmf_address_intr_ctrl_ism },
            { .name = "INTR_CTRL_IER", .val = bdmf_address_intr_ctrl_ier },
            { .name = "INTR_CTRL_ITR", .val = bdmf_address_intr_ctrl_itr },
            { .name = "DEBUG_DBG0", .val = bdmf_address_debug_dbg0 },
            { .name = "DEBUG_DBG1", .val = bdmf_address_debug_dbg1 },
            { .name = "DEBUG_DBG2", .val = bdmf_address_debug_dbg2 },
            { .name = "DEBUG_DBG3", .val = bdmf_address_debug_dbg3 },
            { .name = "DEBUG_DBG4", .val = bdmf_address_debug_dbg4 },
            { .name = "DEBUG_DBG5", .val = bdmf_address_debug_dbg5 },
            { .name = "DEBUG_DBG6", .val = bdmf_address_debug_dbg6 },
            { .name = "DEBUG_DBG7", .val = bdmf_address_debug_dbg7 },
            { .name = "DEBUG_DBG8", .val = bdmf_address_debug_dbg8 },
            { .name = "DEBUG_DBG9", .val = bdmf_address_debug_dbg9 },
            { .name = "DEBUG_DBG10", .val = bdmf_address_debug_dbg10 },
            { .name = "DEBUG_DBG11", .val = bdmf_address_debug_dbg11 },
            { .name = "DEBUG_DBG12", .val = bdmf_address_debug_dbg12 },
            { .name = "DEBUG_DBG13", .val = bdmf_address_debug_dbg13 },
            { .name = "DEBUG_DBG14", .val = bdmf_address_debug_dbg14 },
            { .name = "DEBUG_DBG15", .val = bdmf_address_debug_dbg15 },
            { .name = "DEBUG_DBG16", .val = bdmf_address_debug_dbg16 },
            { .name = "DEBUG_DBG17", .val = bdmf_address_debug_dbg17 },
            { .name = "DEBUG_DBG18", .val = bdmf_address_debug_dbg18 },
            { .name = "DEBUG_DBG19", .val = bdmf_address_debug_dbg19 },
            { .name = "DEBUG_DBG20", .val = bdmf_address_debug_dbg20 },
            { .name = "DEBUG_DBG_SEL", .val = bdmf_address_debug_dbg_sel },
            { .name = "AGING_RAM_AGING", .val = bdmf_address_aging_ram_aging },
            { .name = "STORM_RAM_STORM", .val = bdmf_address_storm_ram_storm },
            { .name = "CAM_CONTEXT_RAM_CONTEXT_CAM", .val = bdmf_address_cam_context_ram_context_cam },
            { .name = "CONTEXT_RAM_CONTEXT", .val = bdmf_address_context_ram_context },
            { .name = "RAM_HASH_RAM", .val = bdmf_address_ram_hash_ram },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_hash_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "hash_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
