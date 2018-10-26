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
#include "xrdp_drv_hash_ag.h"

bdmf_error_t ag_drv_hash_key_padding_set(uint32_t key_pad_h, uint32_t key_pad_l)
{
    uint32_t reg_general_configuration_pad_high=0;
    uint32_t reg_general_configuration_pad_low=0;

#ifdef VALIDATE_PARMS
    if((key_pad_h >= _28BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_pad_high = RU_FIELD_SET(0, HASH, GENERAL_CONFIGURATION_PAD_HIGH, VAL, reg_general_configuration_pad_high, key_pad_h);
    reg_general_configuration_pad_low = RU_FIELD_SET(0, HASH, GENERAL_CONFIGURATION_PAD_LOW, VAL, reg_general_configuration_pad_low, key_pad_l);

    RU_REG_WRITE(0, HASH, GENERAL_CONFIGURATION_PAD_HIGH, reg_general_configuration_pad_high);
    RU_REG_WRITE(0, HASH, GENERAL_CONFIGURATION_PAD_LOW, reg_general_configuration_pad_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_key_padding_get(uint32_t *key_pad_h, uint32_t *key_pad_l)
{
    uint32_t reg_general_configuration_pad_high;
    uint32_t reg_general_configuration_pad_low;

#ifdef VALIDATE_PARMS
    if(!key_pad_h || !key_pad_l)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, GENERAL_CONFIGURATION_PAD_HIGH, reg_general_configuration_pad_high);
    RU_REG_READ(0, HASH, GENERAL_CONFIGURATION_PAD_LOW, reg_general_configuration_pad_low);

    *key_pad_h = RU_FIELD_GET(0, HASH, GENERAL_CONFIGURATION_PAD_HIGH, VAL, reg_general_configuration_pad_high);
    *key_pad_l = RU_FIELD_GET(0, HASH, GENERAL_CONFIGURATION_PAD_LOW, VAL, reg_general_configuration_pad_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_mask_set(uint8_t tbl_idx, const hash_mask *mask)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_low=0;
    uint32_t reg_lkup_tbl_cfg_key_mask_high=0;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7) ||
       (mask->maskh >= _28BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_key_mask_low = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, MASKL, reg_lkup_tbl_cfg_key_mask_low, mask->maskl);
    reg_lkup_tbl_cfg_key_mask_high = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, MASKH, reg_lkup_tbl_cfg_key_mask_high, mask->maskh);

    RU_REG_RAM_WRITE(0, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, reg_lkup_tbl_cfg_key_mask_low);
    RU_REG_RAM_WRITE(0, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, reg_lkup_tbl_cfg_key_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_mask_get(uint8_t tbl_idx, hash_mask *mask)
{
    uint32_t reg_lkup_tbl_cfg_key_mask_low;
    uint32_t reg_lkup_tbl_cfg_key_mask_high;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, reg_lkup_tbl_cfg_key_mask_low);
    RU_REG_RAM_READ(0, tbl_idx, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, reg_lkup_tbl_cfg_key_mask_high);

    mask->maskl = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_KEY_MASK_LOW, MASKL, reg_lkup_tbl_cfg_key_mask_low);
    mask->maskh = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_KEY_MASK_HIGH, MASKH, reg_lkup_tbl_cfg_key_mask_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_pwr_sav_en_set(bdmf_boolean value)
{
    uint32_t reg_general_configuration_pwr_sav_en=0;

#ifdef VALIDATE_PARMS
    if((value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_pwr_sav_en = RU_FIELD_SET(0, HASH, GENERAL_CONFIGURATION_PWR_SAV_EN, VALUE, reg_general_configuration_pwr_sav_en, value);

    RU_REG_WRITE(0, HASH, GENERAL_CONFIGURATION_PWR_SAV_EN, reg_general_configuration_pwr_sav_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_pwr_sav_en_get(bdmf_boolean *value)
{
    uint32_t reg_general_configuration_pwr_sav_en;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, GENERAL_CONFIGURATION_PWR_SAV_EN, reg_general_configuration_pwr_sav_en);

    *value = RU_FIELD_GET(0, HASH, GENERAL_CONFIGURATION_PWR_SAV_EN, VALUE, reg_general_configuration_pwr_sav_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_mult_hit_err_get(uint8_t *val)
{
    uint32_t reg_general_configuration_mult_hit_err;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR, reg_general_configuration_mult_hit_err);

    *val = RU_FIELD_GET(0, HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR, VAL, reg_general_configuration_mult_hit_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_undo_fix_set(bdmf_boolean frst_mul_hit)
{
    uint32_t reg_general_configuration_undo_fix=0;

#ifdef VALIDATE_PARMS
    if((frst_mul_hit >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_undo_fix = RU_FIELD_SET(0, HASH, GENERAL_CONFIGURATION_UNDO_FIX, FRST_MUL_HIT, reg_general_configuration_undo_fix, frst_mul_hit);

    RU_REG_WRITE(0, HASH, GENERAL_CONFIGURATION_UNDO_FIX, reg_general_configuration_undo_fix);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_general_configuration_undo_fix_get(bdmf_boolean *frst_mul_hit)
{
    uint32_t reg_general_configuration_undo_fix;

#ifdef VALIDATE_PARMS
    if(!frst_mul_hit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, GENERAL_CONFIGURATION_UNDO_FIX, reg_general_configuration_undo_fix);

    *frst_mul_hit = RU_FIELD_GET(0, HASH, GENERAL_CONFIGURATION_UNDO_FIX, FRST_MUL_HIT, reg_general_configuration_undo_fix);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hits_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_hits;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_HITS, reg_pm_counters_hits);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_HITS, CNT, reg_pm_counters_hits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_srchs_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_srchs;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_SRCHS, reg_pm_counters_srchs);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_SRCHS, CNT, reg_pm_counters_srchs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_miss_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_miss;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_MISS, reg_pm_counters_miss);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_MISS, CNT, reg_pm_counters_miss);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hit_1st_acs_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_hit_1st_acs;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_HIT_1ST_ACS, reg_pm_counters_hit_1st_acs);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_HIT_1ST_ACS, CNT, reg_pm_counters_hit_1st_acs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hit_2nd_acs_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_hit_2nd_acs;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_HIT_2ND_ACS, reg_pm_counters_hit_2nd_acs);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_HIT_2ND_ACS, CNT, reg_pm_counters_hit_2nd_acs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hit_3rd_acs_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_hit_3rd_acs;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_HIT_3RD_ACS, reg_pm_counters_hit_3rd_acs);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_HIT_3RD_ACS, CNT, reg_pm_counters_hit_3rd_acs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_hit_4th_acs_get(uint32_t *cnt)
{
    uint32_t reg_pm_counters_hit_4th_acs;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_HIT_4TH_ACS, reg_pm_counters_hit_4th_acs);

    *cnt = RU_FIELD_GET(0, HASH, PM_COUNTERS_HIT_4TH_ACS, CNT, reg_pm_counters_hit_4th_acs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_set(bdmf_boolean val)
{
    uint32_t reg_pm_counters_frz_cnt=0;

#ifdef VALIDATE_PARMS
    if((val >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pm_counters_frz_cnt = RU_FIELD_SET(0, HASH, PM_COUNTERS_FRZ_CNT, VAL, reg_pm_counters_frz_cnt, val);

    RU_REG_WRITE(0, HASH, PM_COUNTERS_FRZ_CNT, reg_pm_counters_frz_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_pm_counters_frz_cnt_get(bdmf_boolean *val)
{
    uint32_t reg_pm_counters_frz_cnt;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, PM_COUNTERS_FRZ_CNT, reg_pm_counters_frz_cnt);

    *val = RU_FIELD_GET(0, HASH, PM_COUNTERS_FRZ_CNT, VAL, reg_pm_counters_frz_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(uint8_t tbl_idx, const hash_lkup_tbl_cfg_tbl_cfg *lkup_tbl_cfg_tbl_cfg)
{
    uint32_t reg_lkup_tbl_cfg_tbl_cfg=0;

#ifdef VALIDATE_PARMS
    if(!lkup_tbl_cfg_tbl_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7) ||
       (lkup_tbl_cfg_tbl_cfg->hash_base_addr >= _11BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->tbl_size >= _3BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->max_hop >= _4BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->cam_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->direct_lkup_en >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->hash_type >= _1BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_tbl_cfg->int_ctx_size >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, HASH_BASE_ADDR, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->hash_base_addr);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, TBL_SIZE, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->tbl_size);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, MAX_HOP, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->max_hop);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, CAM_EN, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->cam_en);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, DIRECT_LKUP_EN, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->direct_lkup_en);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, HASH_TYPE, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->hash_type);
    reg_lkup_tbl_cfg_tbl_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_TBL_CFG, INT_CNTX_SIZE, reg_lkup_tbl_cfg_tbl_cfg, lkup_tbl_cfg_tbl_cfg->int_ctx_size);

    RU_REG_RAM_WRITE(0, tbl_idx, HASH, LKUP_TBL_CFG_TBL_CFG, reg_lkup_tbl_cfg_tbl_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get(uint8_t tbl_idx, hash_lkup_tbl_cfg_tbl_cfg *lkup_tbl_cfg_tbl_cfg)
{
    uint32_t reg_lkup_tbl_cfg_tbl_cfg;

#ifdef VALIDATE_PARMS
    if(!lkup_tbl_cfg_tbl_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, tbl_idx, HASH, LKUP_TBL_CFG_TBL_CFG, reg_lkup_tbl_cfg_tbl_cfg);

    lkup_tbl_cfg_tbl_cfg->hash_base_addr = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, HASH_BASE_ADDR, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->tbl_size = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, TBL_SIZE, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->max_hop = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, MAX_HOP, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->cam_en = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, CAM_EN, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->direct_lkup_en = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, DIRECT_LKUP_EN, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->hash_type = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, HASH_TYPE, reg_lkup_tbl_cfg_tbl_cfg);
    lkup_tbl_cfg_tbl_cfg->int_ctx_size = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_TBL_CFG, INT_CNTX_SIZE, reg_lkup_tbl_cfg_tbl_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(uint8_t tbl_idx, const hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg)
{
    uint32_t reg_lkup_tbl_cfg_cntxt_cfg=0;

#ifdef VALIDATE_PARMS
    if(!lkup_tbl_cfg_cntxt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7) ||
       (lkup_tbl_cfg_cntxt_cfg->base_address >= _12BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->first_hash_idx >= _13BITS_MAX_VAL_) ||
       (lkup_tbl_cfg_cntxt_cfg->ext_ctx_size >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, BASE_ADDRESS, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->base_address);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, FIRST_HASH_IDX, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->first_hash_idx);
    reg_lkup_tbl_cfg_cntxt_cfg = RU_FIELD_SET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, CNXT_SIZE, reg_lkup_tbl_cfg_cntxt_cfg, lkup_tbl_cfg_cntxt_cfg->ext_ctx_size);

    RU_REG_RAM_WRITE(0, tbl_idx, HASH, LKUP_TBL_CFG_CNTXT_CFG, reg_lkup_tbl_cfg_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(uint8_t tbl_idx, hash_lkup_tbl_cfg_cntxt_cfg *lkup_tbl_cfg_cntxt_cfg)
{
    uint32_t reg_lkup_tbl_cfg_cntxt_cfg;

#ifdef VALIDATE_PARMS
    if(!lkup_tbl_cfg_cntxt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tbl_idx >= 7))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, tbl_idx, HASH, LKUP_TBL_CFG_CNTXT_CFG, reg_lkup_tbl_cfg_cntxt_cfg);

    lkup_tbl_cfg_cntxt_cfg->base_address = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, BASE_ADDRESS, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->first_hash_idx = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, FIRST_HASH_IDX, reg_lkup_tbl_cfg_cntxt_cfg);
    lkup_tbl_cfg_cntxt_cfg->ext_ctx_size = RU_FIELD_GET(0, HASH, LKUP_TBL_CFG_CNTXT_CFG, CNXT_SIZE, reg_lkup_tbl_cfg_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_base_addr_set(uint16_t base_address)
{
    uint32_t reg_cam_configuration_cntxt_cfg=0;

#ifdef VALIDATE_PARMS
    if((base_address >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_configuration_cntxt_cfg = RU_FIELD_SET(0, HASH, CAM_CONFIGURATION_CNTXT_CFG, BASE_ADDRESS, reg_cam_configuration_cntxt_cfg, base_address);

    RU_REG_WRITE(0, HASH, CAM_CONFIGURATION_CNTXT_CFG, reg_cam_configuration_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_base_addr_get(uint16_t *base_address)
{
    uint32_t reg_cam_configuration_cntxt_cfg;

#ifdef VALIDATE_PARMS
    if(!base_address)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_CONFIGURATION_CNTXT_CFG, reg_cam_configuration_cntxt_cfg);

    *base_address = RU_FIELD_GET(0, HASH, CAM_CONFIGURATION_CNTXT_CFG, BASE_ADDRESS, reg_cam_configuration_cntxt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_set(uint8_t cmd)
{
    uint32_t reg_cam_indirect_op=0;

#ifdef VALIDATE_PARMS
    if((cmd >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_op = RU_FIELD_SET(0, HASH, CAM_INDIRECT_OP, CMD, reg_cam_indirect_op, cmd);

    RU_REG_WRITE(0, HASH, CAM_INDIRECT_OP, reg_cam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_get(uint8_t *cmd)
{
    uint32_t reg_cam_indirect_op;

#ifdef VALIDATE_PARMS
    if(!cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_OP, reg_cam_indirect_op);

    *cmd = RU_FIELD_GET(0, HASH, CAM_INDIRECT_OP, CMD, reg_cam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_op_done_get(bdmf_boolean *val)
{
    uint32_t reg_cam_indirect_op_done;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_OP_DONE, reg_cam_indirect_op_done);

    *val = RU_FIELD_GET(0, HASH, CAM_INDIRECT_OP_DONE, VAL, reg_cam_indirect_op_done);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_addr_set(bdmf_boolean key1_ind, uint8_t entry_addr)
{
    uint32_t reg_cam_indirect_addr=0;

#ifdef VALIDATE_PARMS
    if((key1_ind >= _1BITS_MAX_VAL_) ||
       (entry_addr >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_addr = RU_FIELD_SET(0, HASH, CAM_INDIRECT_ADDR, KEY1_IND, reg_cam_indirect_addr, key1_ind);
    reg_cam_indirect_addr = RU_FIELD_SET(0, HASH, CAM_INDIRECT_ADDR, ENTRY_ADDR, reg_cam_indirect_addr, entry_addr);

    RU_REG_WRITE(0, HASH, CAM_INDIRECT_ADDR, reg_cam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_addr_get(bdmf_boolean *key1_ind, uint8_t *entry_addr)
{
    uint32_t reg_cam_indirect_addr;

#ifdef VALIDATE_PARMS
    if(!key1_ind || !entry_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_ADDR, reg_cam_indirect_addr);

    *key1_ind = RU_FIELD_GET(0, HASH, CAM_INDIRECT_ADDR, KEY1_IND, reg_cam_indirect_addr);
    *entry_addr = RU_FIELD_GET(0, HASH, CAM_INDIRECT_ADDR, ENTRY_ADDR, reg_cam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_set(bdmf_boolean valid)
{
    uint32_t reg_cam_indirect_vlid_in=0;

#ifdef VALIDATE_PARMS
    if((valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_vlid_in = RU_FIELD_SET(0, HASH, CAM_INDIRECT_VLID_IN, VALID, reg_cam_indirect_vlid_in, valid);

    RU_REG_WRITE(0, HASH, CAM_INDIRECT_VLID_IN, reg_cam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_in_get(bdmf_boolean *valid)
{
    uint32_t reg_cam_indirect_vlid_in;

#ifdef VALIDATE_PARMS
    if(!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_VLID_IN, reg_cam_indirect_vlid_in);

    *valid = RU_FIELD_GET(0, HASH, CAM_INDIRECT_VLID_IN, VALID, reg_cam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_set(bdmf_boolean valid)
{
    uint32_t reg_cam_indirect_vlid_out=0;

#ifdef VALIDATE_PARMS
    if((valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_indirect_vlid_out = RU_FIELD_SET(0, HASH, CAM_INDIRECT_VLID_OUT, VALID, reg_cam_indirect_vlid_out, valid);

    RU_REG_WRITE(0, HASH, CAM_INDIRECT_VLID_OUT, reg_cam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_vlid_out_get(bdmf_boolean *valid)
{
    uint32_t reg_cam_indirect_vlid_out;

#ifdef VALIDATE_PARMS
    if(!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_VLID_OUT, reg_cam_indirect_vlid_out);

    *valid = RU_FIELD_GET(0, HASH, CAM_INDIRECT_VLID_OUT, VALID, reg_cam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_rslt_get(bdmf_boolean *match, uint8_t *index)
{
    uint32_t reg_cam_indirect_rslt;

#ifdef VALIDATE_PARMS
    if(!match || !index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_INDIRECT_RSLT, reg_cam_indirect_rslt);

    *match = RU_FIELD_GET(0, HASH, CAM_INDIRECT_RSLT, MATCH, reg_cam_indirect_rslt);
    *index = RU_FIELD_GET(0, HASH, CAM_INDIRECT_RSLT, INDEX, reg_cam_indirect_rslt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_in_set(uint8_t zero, const hash_cam_indirect_key_in *cam_indirect_key_in)
{
#ifdef VALIDATE_PARMS
    if(!cam_indirect_key_in)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, zero *2 + 0, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[0]);
    RU_REG_RAM_WRITE(0, zero *2 + 1, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_in_get(uint8_t zero, hash_cam_indirect_key_in *cam_indirect_key_in)
{
#ifdef VALIDATE_PARMS
    if(!cam_indirect_key_in)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *2 + 0, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[0]);
    RU_REG_RAM_READ(0, zero *2 + 1, HASH, CAM_INDIRECT_KEY_IN, cam_indirect_key_in->key_in[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_indirect_key_out_get(uint8_t zero, hash_cam_indirect_key_out *cam_indirect_key_out)
{
#ifdef VALIDATE_PARMS
    if(!cam_indirect_key_out)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *2 + 0, HASH, CAM_INDIRECT_KEY_OUT, cam_indirect_key_out->key_out[0]);
    RU_REG_RAM_READ(0, zero *2 + 1, HASH, CAM_INDIRECT_KEY_OUT, cam_indirect_key_out->key_out[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_status_get(uint32_t *value)
{
    uint32_t reg_cam_bist_bist_status;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_STATUS, reg_cam_bist_bist_status);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_STATUS, VALUE, reg_cam_bist_bist_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_compare_en_set(bdmf_boolean value)
{
    uint32_t reg_cam_bist_bist_dbg_compare_en=0;

#ifdef VALIDATE_PARMS
    if((value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_dbg_compare_en = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_DBG_COMPARE_EN, VALUE, reg_cam_bist_bist_dbg_compare_en, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_DBG_COMPARE_EN, reg_cam_bist_bist_dbg_compare_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_compare_en_get(bdmf_boolean *value)
{
    uint32_t reg_cam_bist_bist_dbg_compare_en;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_DBG_COMPARE_EN, reg_cam_bist_bist_dbg_compare_en);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_DBG_COMPARE_EN, VALUE, reg_cam_bist_bist_dbg_compare_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_set(uint32_t value)
{
    uint32_t reg_cam_bist_bist_dbg_data=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cam_bist_bist_dbg_data = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_DBG_DATA, VALUE, reg_cam_bist_bist_dbg_data, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_DBG_DATA, reg_cam_bist_bist_dbg_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_get(uint32_t *value)
{
    uint32_t reg_cam_bist_bist_dbg_data;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_DBG_DATA, reg_cam_bist_bist_dbg_data);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_DBG_DATA, VALUE, reg_cam_bist_bist_dbg_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_dbg_data_slice_or_status_sel=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cam_bist_bist_dbg_data_slice_or_status_sel = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL, VALUE, reg_cam_bist_bist_dbg_data_slice_or_status_sel, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL, reg_cam_bist_bist_dbg_data_slice_or_status_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_dbg_data_slice_or_status_sel;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL, reg_cam_bist_bist_dbg_data_slice_or_status_sel);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL, VALUE, reg_cam_bist_bist_dbg_data_slice_or_status_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_valid_set(bdmf_boolean value)
{
    uint32_t reg_cam_bist_bist_dbg_data_valid=0;

#ifdef VALIDATE_PARMS
    if((value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_dbg_data_valid = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_DBG_DATA_VALID, VALUE, reg_cam_bist_bist_dbg_data_valid, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_DBG_DATA_VALID, reg_cam_bist_bist_dbg_data_valid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_dbg_data_valid_get(bdmf_boolean *value)
{
    uint32_t reg_cam_bist_bist_dbg_data_valid;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_DBG_DATA_VALID, reg_cam_bist_bist_dbg_data_valid);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_DBG_DATA_VALID, VALUE, reg_cam_bist_bist_dbg_data_valid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_en_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cam_bist_bist_en = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_EN, VALUE, reg_cam_bist_bist_en, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_EN, reg_cam_bist_bist_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_en_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_en;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_EN, reg_cam_bist_bist_en);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_EN, VALUE, reg_cam_bist_bist_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_mode_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_mode=0;

#ifdef VALIDATE_PARMS
    if((value >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_mode = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_MODE, VALUE, reg_cam_bist_bist_mode, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_MODE, reg_cam_bist_bist_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_mode_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_mode;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_MODE, reg_cam_bist_bist_mode);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_MODE, VALUE, reg_cam_bist_bist_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_rst_l_set(bdmf_boolean value)
{
    uint32_t reg_cam_bist_bist_rst_l=0;

#ifdef VALIDATE_PARMS
    if((value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_rst_l = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_RST_L, VALUE, reg_cam_bist_bist_rst_l, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_RST_L, reg_cam_bist_bist_rst_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_rst_l_get(bdmf_boolean *value)
{
    uint32_t reg_cam_bist_bist_rst_l;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_RST_L, reg_cam_bist_bist_rst_l);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_RST_L, VALUE, reg_cam_bist_bist_rst_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_skip_error_cnt_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_skip_error_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cam_bist_bist_skip_error_cnt = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_SKIP_ERROR_CNT, VALUE, reg_cam_bist_bist_skip_error_cnt, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_SKIP_ERROR_CNT, reg_cam_bist_bist_skip_error_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_skip_error_cnt_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_skip_error_cnt;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_SKIP_ERROR_CNT, reg_cam_bist_bist_skip_error_cnt);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_SKIP_ERROR_CNT, VALUE, reg_cam_bist_bist_skip_error_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_dbg_en_set(uint8_t value)
{
    uint32_t reg_cam_bist_dbg_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cam_bist_dbg_en = RU_FIELD_SET(0, HASH, CAM_BIST_DBG_EN, VALUE, reg_cam_bist_dbg_en, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_DBG_EN, reg_cam_bist_dbg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_dbg_en_get(uint8_t *value)
{
    uint32_t reg_cam_bist_dbg_en;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_DBG_EN, reg_cam_bist_dbg_en);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_DBG_EN, VALUE, reg_cam_bist_dbg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_cascade_select_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_cascade_select=0;

#ifdef VALIDATE_PARMS
    if((value >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_cascade_select = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_CASCADE_SELECT, VALUE, reg_cam_bist_bist_cascade_select, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_CASCADE_SELECT, reg_cam_bist_bist_cascade_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_cascade_select_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_cascade_select;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_CASCADE_SELECT, reg_cam_bist_bist_cascade_select);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_CASCADE_SELECT, VALUE, reg_cam_bist_bist_cascade_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_block_select_set(uint8_t value)
{
    uint32_t reg_cam_bist_bist_block_select=0;

#ifdef VALIDATE_PARMS
    if((value >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_block_select = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_BLOCK_SELECT, VALUE, reg_cam_bist_bist_block_select, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_BLOCK_SELECT, reg_cam_bist_bist_block_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_block_select_get(uint8_t *value)
{
    uint32_t reg_cam_bist_bist_block_select;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_BLOCK_SELECT, reg_cam_bist_bist_block_select);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_BLOCK_SELECT, VALUE, reg_cam_bist_bist_block_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_repair_enable_set(bdmf_boolean value)
{
    uint32_t reg_cam_bist_bist_repair_enable=0;

#ifdef VALIDATE_PARMS
    if((value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cam_bist_bist_repair_enable = RU_FIELD_SET(0, HASH, CAM_BIST_BIST_REPAIR_ENABLE, VALUE, reg_cam_bist_bist_repair_enable, value);

    RU_REG_WRITE(0, HASH, CAM_BIST_BIST_REPAIR_ENABLE, reg_cam_bist_bist_repair_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_cam_bist_bist_repair_enable_get(bdmf_boolean *value)
{
    uint32_t reg_cam_bist_bist_repair_enable;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, CAM_BIST_BIST_REPAIR_ENABLE, reg_cam_bist_bist_repair_enable);

    *value = RU_FIELD_GET(0, HASH, CAM_BIST_BIST_REPAIR_ENABLE, VALUE, reg_cam_bist_bist_repair_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_isr_set(const hash_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr=0;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((intr_ctrl_isr->invld_cmd >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->mult_match >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->hash_0_idx_ovflv >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->hash_1_idx_ovflv >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->hash_2_idx_ovflv >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->hash_3_idx_ovflv >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->cntxt_idx_ovflv >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, INVLD_CMD, reg_intr_ctrl_isr, intr_ctrl_isr->invld_cmd);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr, intr_ctrl_isr->mult_match);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, HASH_0_IDX_OVFLV, reg_intr_ctrl_isr, intr_ctrl_isr->hash_0_idx_ovflv);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, HASH_1_IDX_OVFLV, reg_intr_ctrl_isr, intr_ctrl_isr->hash_1_idx_ovflv);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, HASH_2_IDX_OVFLV, reg_intr_ctrl_isr, intr_ctrl_isr->hash_2_idx_ovflv);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, HASH_3_IDX_OVFLV, reg_intr_ctrl_isr, intr_ctrl_isr->hash_3_idx_ovflv);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, HASH, INTR_CTRL_ISR, CNTXT_IDX_OVFLV, reg_intr_ctrl_isr, intr_ctrl_isr->cntxt_idx_ovflv);

    RU_REG_WRITE(0, HASH, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_isr_get(hash_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    intr_ctrl_isr->invld_cmd = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, INVLD_CMD, reg_intr_ctrl_isr);
    intr_ctrl_isr->mult_match = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr);
    intr_ctrl_isr->hash_0_idx_ovflv = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, HASH_0_IDX_OVFLV, reg_intr_ctrl_isr);
    intr_ctrl_isr->hash_1_idx_ovflv = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, HASH_1_IDX_OVFLV, reg_intr_ctrl_isr);
    intr_ctrl_isr->hash_2_idx_ovflv = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, HASH_2_IDX_OVFLV, reg_intr_ctrl_isr);
    intr_ctrl_isr->hash_3_idx_ovflv = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, HASH_3_IDX_OVFLV, reg_intr_ctrl_isr);
    intr_ctrl_isr->cntxt_idx_ovflv = RU_FIELD_GET(0, HASH, INTR_CTRL_ISR, CNTXT_IDX_OVFLV, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if(!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(0, HASH, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ier_set(uint32_t iem)
{
    uint32_t reg_intr_ctrl_ier=0;

#ifdef VALIDATE_PARMS
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(0, HASH, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(0, HASH, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_ier_get(uint32_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if(!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(0, HASH, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_itr_set(uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(0, HASH, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, HASH, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_intr_ctrl_itr_get(uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if(!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(0, HASH, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg0_get(uint32_t *val)
{
    uint32_t reg_debug_dbg0;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG0, reg_debug_dbg0);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG0, VAL, reg_debug_dbg0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg1_get(uint32_t *val)
{
    uint32_t reg_debug_dbg1;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG1, reg_debug_dbg1);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG1, VAL, reg_debug_dbg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg2_get(uint32_t *val)
{
    uint32_t reg_debug_dbg2;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG2, reg_debug_dbg2);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG2, VAL, reg_debug_dbg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg3_get(uint32_t *val)
{
    uint32_t reg_debug_dbg3;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG3, reg_debug_dbg3);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG3, VAL, reg_debug_dbg3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg4_get(uint32_t *val)
{
    uint32_t reg_debug_dbg4;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG4, reg_debug_dbg4);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG4, VAL, reg_debug_dbg4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg5_get(uint32_t *val)
{
    uint32_t reg_debug_dbg5;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG5, reg_debug_dbg5);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG5, VAL, reg_debug_dbg5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg6_get(uint32_t *val)
{
    uint32_t reg_debug_dbg6;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG6, reg_debug_dbg6);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG6, VAL, reg_debug_dbg6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg7_get(uint32_t *val)
{
    uint32_t reg_debug_dbg7;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG7, reg_debug_dbg7);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG7, VAL, reg_debug_dbg7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg8_get(uint32_t *val)
{
    uint32_t reg_debug_dbg8;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG8, reg_debug_dbg8);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG8, VAL, reg_debug_dbg8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg9_get(uint32_t *val)
{
    uint32_t reg_debug_dbg9;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG9, reg_debug_dbg9);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG9, VAL, reg_debug_dbg9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg10_get(uint32_t *val)
{
    uint32_t reg_debug_dbg10;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG10, reg_debug_dbg10);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG10, VAL, reg_debug_dbg10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg11_get(uint32_t *val)
{
    uint32_t reg_debug_dbg11;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG11, reg_debug_dbg11);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG11, VAL, reg_debug_dbg11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg12_get(uint32_t *val)
{
    uint32_t reg_debug_dbg12;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG12, reg_debug_dbg12);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG12, VAL, reg_debug_dbg12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg13_get(uint32_t *val)
{
    uint32_t reg_debug_dbg13;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG13, reg_debug_dbg13);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG13, VAL, reg_debug_dbg13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg14_get(uint32_t *val)
{
    uint32_t reg_debug_dbg14;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG14, reg_debug_dbg14);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG14, VAL, reg_debug_dbg14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg15_get(uint32_t *val)
{
    uint32_t reg_debug_dbg15;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG15, reg_debug_dbg15);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG15, VAL, reg_debug_dbg15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg16_get(uint32_t *val)
{
    uint32_t reg_debug_dbg16;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG16, reg_debug_dbg16);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG16, VAL, reg_debug_dbg16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg17_get(uint32_t *val)
{
    uint32_t reg_debug_dbg17;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG17, reg_debug_dbg17);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG17, VAL, reg_debug_dbg17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg18_get(uint32_t *val)
{
    uint32_t reg_debug_dbg18;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG18, reg_debug_dbg18);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG18, VAL, reg_debug_dbg18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg19_get(uint32_t *val)
{
    uint32_t reg_debug_dbg19;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG19, reg_debug_dbg19);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG19, VAL, reg_debug_dbg19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg20_get(uint32_t *val)
{
    uint32_t reg_debug_dbg20;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG20, reg_debug_dbg20);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG20, VAL, reg_debug_dbg20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg_sel_set(uint8_t val)
{
    uint32_t reg_debug_dbg_sel=0;

#ifdef VALIDATE_PARMS
    if((val >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_dbg_sel = RU_FIELD_SET(0, HASH, DEBUG_DBG_SEL, VAL, reg_debug_dbg_sel, val);

    RU_REG_WRITE(0, HASH, DEBUG_DBG_SEL, reg_debug_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_debug_dbg_sel_get(uint8_t *val)
{
    uint32_t reg_debug_dbg_sel;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, HASH, DEBUG_DBG_SEL, reg_debug_dbg_sel);

    *val = RU_FIELD_GET(0, HASH, DEBUG_DBG_SEL, VAL, reg_debug_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_aging_ram_aging_set(uint16_t entry_idx, bdmf_boolean age)
{
    uint32_t reg_aging_ram_aging=0;

#ifdef VALIDATE_PARMS
    if((entry_idx >= 2112))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    FIELD_SET(reg_aging_ram_aging, (entry_idx % 32) *1, 0x1, age);

    RU_REG_RAM_WRITE(0, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_aging_ram_aging_get(uint16_t entry_idx, bdmf_boolean *age)
{
    uint32_t reg_aging_ram_aging;

#ifdef VALIDATE_PARMS
    if(!age)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((entry_idx >= 2112))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, entry_idx / 32, HASH, AGING_RAM_AGING, reg_aging_ram_aging);

    *age = FIELD_GET(reg_aging_ram_aging, (entry_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_47_24_set(uint16_t ctx_idx, uint32_t bits)
{
    uint32_t reg_context_ram_context_47_24=0;

#ifdef VALIDATE_PARMS
    if((ctx_idx >= 1216) ||
       (bits >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_context_ram_context_47_24 = RU_FIELD_SET(0, HASH, CONTEXT_RAM_CONTEXT_47_24, DATA, reg_context_ram_context_47_24, bits);

    RU_REG_RAM_WRITE(0, ctx_idx, HASH, CONTEXT_RAM_CONTEXT_47_24, reg_context_ram_context_47_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_47_24_get(uint16_t ctx_idx, uint32_t *bits)
{
    uint32_t reg_context_ram_context_47_24;

#ifdef VALIDATE_PARMS
    if(!bits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctx_idx >= 1216))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctx_idx, HASH, CONTEXT_RAM_CONTEXT_47_24, reg_context_ram_context_47_24);

    *bits = RU_FIELD_GET(0, HASH, CONTEXT_RAM_CONTEXT_47_24, DATA, reg_context_ram_context_47_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_23_0_set(uint16_t ctx_idx, uint32_t bits)
{
    uint32_t reg_context_ram_context_23_0=0;

#ifdef VALIDATE_PARMS
    if((ctx_idx >= 1216) ||
       (bits >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_context_ram_context_23_0 = RU_FIELD_SET(0, HASH, CONTEXT_RAM_CONTEXT_23_0, DATA, reg_context_ram_context_23_0, bits);

    RU_REG_RAM_WRITE(0, ctx_idx, HASH, CONTEXT_RAM_CONTEXT_23_0, reg_context_ram_context_23_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_context_ram_context_23_0_get(uint16_t ctx_idx, uint32_t *bits)
{
    uint32_t reg_context_ram_context_23_0;

#ifdef VALIDATE_PARMS
    if(!bits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctx_idx >= 1216))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctx_idx, HASH, CONTEXT_RAM_CONTEXT_23_0, reg_context_ram_context_23_0);

    *bits = RU_FIELD_GET(0, HASH, CONTEXT_RAM_CONTEXT_23_0, DATA, reg_context_ram_context_23_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_eng_high_set(uint16_t idx, uint32_t key_59_28_or_dat)
{
    uint32_t reg_ram_eng_high=0;

#ifdef VALIDATE_PARMS
    if((idx >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ram_eng_high = RU_FIELD_SET(0, HASH, RAM_ENG_HIGH, KEY_59_28_OR_DAT, reg_ram_eng_high, key_59_28_or_dat);

    RU_REG_RAM_WRITE(0, idx, HASH, RAM_ENG_HIGH, reg_ram_eng_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_eng_high_get(uint16_t idx, uint32_t *key_59_28_or_dat)
{
    uint32_t reg_ram_eng_high;

#ifdef VALIDATE_PARMS
    if(!key_59_28_or_dat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, HASH, RAM_ENG_HIGH, reg_ram_eng_high);

    *key_59_28_or_dat = RU_FIELD_GET(0, HASH, RAM_ENG_HIGH, KEY_59_28_OR_DAT, reg_ram_eng_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_eng_low_set(uint16_t idx, bdmf_boolean skp, uint8_t cfg, uint16_t key_11_0, uint16_t key_27_12_or_dat)
{
    uint32_t reg_ram_eng_low=0;

#ifdef VALIDATE_PARMS
    if((idx >= 2048) ||
       (skp >= _1BITS_MAX_VAL_) ||
       (cfg >= _3BITS_MAX_VAL_) ||
       (key_11_0 >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ram_eng_low = RU_FIELD_SET(0, HASH, RAM_ENG_LOW, SKP, reg_ram_eng_low, skp);
    reg_ram_eng_low = RU_FIELD_SET(0, HASH, RAM_ENG_LOW, CFG, reg_ram_eng_low, cfg);
    reg_ram_eng_low = RU_FIELD_SET(0, HASH, RAM_ENG_LOW, KEY_11_0, reg_ram_eng_low, key_11_0);
    reg_ram_eng_low = RU_FIELD_SET(0, HASH, RAM_ENG_LOW, KEY_27_12_OR_DAT, reg_ram_eng_low, key_27_12_or_dat);

    RU_REG_RAM_WRITE(0, idx, HASH, RAM_ENG_LOW, reg_ram_eng_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_hash_ram_eng_low_get(uint16_t idx, bdmf_boolean *skp, uint8_t *cfg, uint16_t *key_11_0, uint16_t *key_27_12_or_dat)
{
    uint32_t reg_ram_eng_low;

#ifdef VALIDATE_PARMS
    if(!skp || !cfg || !key_11_0 || !key_27_12_or_dat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, HASH, RAM_ENG_LOW, reg_ram_eng_low);

    *skp = RU_FIELD_GET(0, HASH, RAM_ENG_LOW, SKP, reg_ram_eng_low);
    *cfg = RU_FIELD_GET(0, HASH, RAM_ENG_LOW, CFG, reg_ram_eng_low);
    *key_11_0 = RU_FIELD_GET(0, HASH, RAM_ENG_LOW, KEY_11_0, reg_ram_eng_low);
    *key_27_12_or_dat = RU_FIELD_GET(0, HASH, RAM_ENG_LOW, KEY_27_12_OR_DAT, reg_ram_eng_low);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_general_configuration_pwr_sav_en,
    bdmf_address_general_configuration_pad_high,
    bdmf_address_general_configuration_pad_low,
    bdmf_address_general_configuration_mult_hit_err,
    bdmf_address_general_configuration_undo_fix,
    bdmf_address_pm_counters_hits,
    bdmf_address_pm_counters_srchs,
    bdmf_address_pm_counters_miss,
    bdmf_address_pm_counters_hit_1st_acs,
    bdmf_address_pm_counters_hit_2nd_acs,
    bdmf_address_pm_counters_hit_3rd_acs,
    bdmf_address_pm_counters_hit_4th_acs,
    bdmf_address_pm_counters_frz_cnt,
    bdmf_address_lkup_tbl_cfg_tbl_cfg,
    bdmf_address_lkup_tbl_cfg_key_mask_high,
    bdmf_address_lkup_tbl_cfg_key_mask_low,
    bdmf_address_lkup_tbl_cfg_cntxt_cfg,
    bdmf_address_cam_configuration_cntxt_cfg,
    bdmf_address_cam_indirect_op,
    bdmf_address_cam_indirect_op_done,
    bdmf_address_cam_indirect_addr,
    bdmf_address_cam_indirect_vlid_in,
    bdmf_address_cam_indirect_vlid_out,
    bdmf_address_cam_indirect_rslt,
    bdmf_address_cam_indirect_key_in,
    bdmf_address_cam_indirect_key_out,
    bdmf_address_cam_bist_bist_status,
    bdmf_address_cam_bist_bist_dbg_compare_en,
    bdmf_address_cam_bist_bist_dbg_data,
    bdmf_address_cam_bist_bist_dbg_data_slice_or_status_sel,
    bdmf_address_cam_bist_bist_dbg_data_valid,
    bdmf_address_cam_bist_bist_en,
    bdmf_address_cam_bist_bist_mode,
    bdmf_address_cam_bist_bist_rst_l,
    bdmf_address_cam_bist_bist_skip_error_cnt,
    bdmf_address_cam_bist_dbg_en,
    bdmf_address_cam_bist_bist_cascade_select,
    bdmf_address_cam_bist_bist_block_select,
    bdmf_address_cam_bist_bist_repair_enable,
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
    bdmf_address_context_ram_context_47_24,
    bdmf_address_context_ram_context_23_0,
    bdmf_address_ram_eng_high,
    bdmf_address_ram_eng_low,
}
bdmf_address;

static int bcm_hash_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_hash_key_padding:
        err = ag_drv_hash_key_padding_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_mask:
    {
        hash_mask mask = { .maskl=parm[2].value.unumber, .maskh=parm[3].value.unumber};
        err = ag_drv_hash_mask_set(parm[1].value.unumber, &mask);
        break;
    }
    case cli_hash_general_configuration_pwr_sav_en:
        err = ag_drv_hash_general_configuration_pwr_sav_en_set(parm[1].value.unumber);
        break;
    case cli_hash_general_configuration_undo_fix:
        err = ag_drv_hash_general_configuration_undo_fix_set(parm[1].value.unumber);
        break;
    case cli_hash_pm_counters_frz_cnt:
        err = ag_drv_hash_pm_counters_frz_cnt_set(parm[1].value.unumber);
        break;
    case cli_hash_lkup_tbl_cfg_tbl_cfg:
    {
        hash_lkup_tbl_cfg_tbl_cfg lkup_tbl_cfg_tbl_cfg = { .hash_base_addr=parm[2].value.unumber, .tbl_size=parm[3].value.unumber, .max_hop=parm[4].value.unumber, .cam_en=parm[5].value.unumber, .direct_lkup_en=parm[6].value.unumber, .hash_type=parm[7].value.unumber, .int_ctx_size=parm[8].value.unumber};
        err = ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(parm[1].value.unumber, &lkup_tbl_cfg_tbl_cfg);
        break;
    }
    case cli_hash_lkup_tbl_cfg_cntxt_cfg:
    {
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg = { .base_address=parm[2].value.unumber, .first_hash_idx=parm[3].value.unumber, .ext_ctx_size=parm[4].value.unumber};
        err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(parm[1].value.unumber, &lkup_tbl_cfg_cntxt_cfg);
        break;
    }
    case cli_hash_cam_base_addr:
        err = ag_drv_hash_cam_base_addr_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_indirect_op:
        err = ag_drv_hash_cam_indirect_op_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_indirect_addr:
        err = ag_drv_hash_cam_indirect_addr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_cam_indirect_vlid_in:
        err = ag_drv_hash_cam_indirect_vlid_in_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_indirect_vlid_out:
        err = ag_drv_hash_cam_indirect_vlid_out_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_indirect_key_in:
    {
        hash_cam_indirect_key_in cam_indirect_key_in = { .key_in = { parm[2].value.unumber, parm[3].value.unumber}};
        err = ag_drv_hash_cam_indirect_key_in_set(parm[1].value.unumber, &cam_indirect_key_in);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_compare_en:
        err = ag_drv_hash_cam_bist_bist_dbg_compare_en_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_dbg_data:
        err = ag_drv_hash_cam_bist_bist_dbg_data_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_dbg_data_slice_or_status_sel:
        err = ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_dbg_data_valid:
        err = ag_drv_hash_cam_bist_bist_dbg_data_valid_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_en:
        err = ag_drv_hash_cam_bist_bist_en_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_mode:
        err = ag_drv_hash_cam_bist_bist_mode_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_rst_l:
        err = ag_drv_hash_cam_bist_bist_rst_l_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_skip_error_cnt:
        err = ag_drv_hash_cam_bist_bist_skip_error_cnt_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_dbg_en:
        err = ag_drv_hash_cam_bist_dbg_en_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_cascade_select:
        err = ag_drv_hash_cam_bist_bist_cascade_select_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_block_select:
        err = ag_drv_hash_cam_bist_bist_block_select_set(parm[1].value.unumber);
        break;
    case cli_hash_cam_bist_bist_repair_enable:
        err = ag_drv_hash_cam_bist_bist_repair_enable_set(parm[1].value.unumber);
        break;
    case cli_hash_intr_ctrl_isr:
    {
        hash_intr_ctrl_isr intr_ctrl_isr = { .invld_cmd=parm[1].value.unumber, .mult_match=parm[2].value.unumber, .hash_0_idx_ovflv=parm[3].value.unumber, .hash_1_idx_ovflv=parm[4].value.unumber, .hash_2_idx_ovflv=parm[5].value.unumber, .hash_3_idx_ovflv=parm[6].value.unumber, .cntxt_idx_ovflv=parm[7].value.unumber};
        err = ag_drv_hash_intr_ctrl_isr_set(&intr_ctrl_isr);
        break;
    }
    case cli_hash_intr_ctrl_ier:
        err = ag_drv_hash_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_hash_intr_ctrl_itr:
        err = ag_drv_hash_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    case cli_hash_debug_dbg_sel:
        err = ag_drv_hash_debug_dbg_sel_set(parm[1].value.unumber);
        break;
    case cli_hash_aging_ram_aging:
        err = ag_drv_hash_aging_ram_aging_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_context_ram_context_47_24:
        err = ag_drv_hash_context_ram_context_47_24_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_context_ram_context_23_0:
        err = ag_drv_hash_context_ram_context_23_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_ram_eng_high:
        err = ag_drv_hash_ram_eng_high_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_hash_ram_eng_low:
        err = ag_drv_hash_ram_eng_low_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_hash_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_hash_key_padding:
    {
        uint32_t key_pad_h;
        uint32_t key_pad_l;
        err = ag_drv_hash_key_padding_get(&key_pad_h, &key_pad_l);
        bdmf_session_print(session, "key_pad_h = %u (0x%x)\n", key_pad_h, key_pad_h);
        bdmf_session_print(session, "key_pad_l = %u (0x%x)\n", key_pad_l, key_pad_l);
        break;
    }
    case cli_hash_mask:
    {
        hash_mask mask;
        err = ag_drv_hash_mask_get(parm[1].value.unumber, &mask);
        bdmf_session_print(session, "maskl = %u (0x%x)\n", mask.maskl, mask.maskl);
        bdmf_session_print(session, "maskh = %u (0x%x)\n", mask.maskh, mask.maskh);
        break;
    }
    case cli_hash_general_configuration_pwr_sav_en:
    {
        bdmf_boolean value;
        err = ag_drv_hash_general_configuration_pwr_sav_en_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_general_configuration_mult_hit_err:
    {
        uint8_t val;
        err = ag_drv_hash_general_configuration_mult_hit_err_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_general_configuration_undo_fix:
    {
        bdmf_boolean frst_mul_hit;
        err = ag_drv_hash_general_configuration_undo_fix_get(&frst_mul_hit);
        bdmf_session_print(session, "frst_mul_hit = %u (0x%x)\n", frst_mul_hit, frst_mul_hit);
        break;
    }
    case cli_hash_pm_counters_hits:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_hits_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_srchs:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_srchs_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_miss:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_miss_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_hit_1st_acs:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_hit_1st_acs_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_hit_2nd_acs:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_hit_2nd_acs_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_hit_3rd_acs:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_hit_3rd_acs_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_hit_4th_acs:
    {
        uint32_t cnt;
        err = ag_drv_hash_pm_counters_hit_4th_acs_get(&cnt);
        bdmf_session_print(session, "cnt = %u (0x%x)\n", cnt, cnt);
        break;
    }
    case cli_hash_pm_counters_frz_cnt:
    {
        bdmf_boolean val;
        err = ag_drv_hash_pm_counters_frz_cnt_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_lkup_tbl_cfg_tbl_cfg:
    {
        hash_lkup_tbl_cfg_tbl_cfg lkup_tbl_cfg_tbl_cfg;
        err = ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get(parm[1].value.unumber, &lkup_tbl_cfg_tbl_cfg);
        bdmf_session_print(session, "hash_base_addr = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.hash_base_addr, lkup_tbl_cfg_tbl_cfg.hash_base_addr);
        bdmf_session_print(session, "tbl_size = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.tbl_size, lkup_tbl_cfg_tbl_cfg.tbl_size);
        bdmf_session_print(session, "max_hop = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.max_hop, lkup_tbl_cfg_tbl_cfg.max_hop);
        bdmf_session_print(session, "cam_en = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.cam_en, lkup_tbl_cfg_tbl_cfg.cam_en);
        bdmf_session_print(session, "direct_lkup_en = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.direct_lkup_en, lkup_tbl_cfg_tbl_cfg.direct_lkup_en);
        bdmf_session_print(session, "hash_type = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.hash_type, lkup_tbl_cfg_tbl_cfg.hash_type);
        bdmf_session_print(session, "int_ctx_size = %u (0x%x)\n", lkup_tbl_cfg_tbl_cfg.int_ctx_size, lkup_tbl_cfg_tbl_cfg.int_ctx_size);
        break;
    }
    case cli_hash_lkup_tbl_cfg_cntxt_cfg:
    {
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg;
        err = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(parm[1].value.unumber, &lkup_tbl_cfg_cntxt_cfg);
        bdmf_session_print(session, "base_address = %u (0x%x)\n", lkup_tbl_cfg_cntxt_cfg.base_address, lkup_tbl_cfg_cntxt_cfg.base_address);
        bdmf_session_print(session, "first_hash_idx = %u (0x%x)\n", lkup_tbl_cfg_cntxt_cfg.first_hash_idx, lkup_tbl_cfg_cntxt_cfg.first_hash_idx);
        bdmf_session_print(session, "ext_ctx_size = %u (0x%x)\n", lkup_tbl_cfg_cntxt_cfg.ext_ctx_size, lkup_tbl_cfg_cntxt_cfg.ext_ctx_size);
        break;
    }
    case cli_hash_cam_base_addr:
    {
        uint16_t base_address;
        err = ag_drv_hash_cam_base_addr_get(&base_address);
        bdmf_session_print(session, "base_address = %u (0x%x)\n", base_address, base_address);
        break;
    }
    case cli_hash_cam_indirect_op:
    {
        uint8_t cmd;
        err = ag_drv_hash_cam_indirect_op_get(&cmd);
        bdmf_session_print(session, "cmd = %u (0x%x)\n", cmd, cmd);
        break;
    }
    case cli_hash_cam_indirect_op_done:
    {
        bdmf_boolean val;
        err = ag_drv_hash_cam_indirect_op_done_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_cam_indirect_addr:
    {
        bdmf_boolean key1_ind;
        uint8_t entry_addr;
        err = ag_drv_hash_cam_indirect_addr_get(&key1_ind, &entry_addr);
        bdmf_session_print(session, "key1_ind = %u (0x%x)\n", key1_ind, key1_ind);
        bdmf_session_print(session, "entry_addr = %u (0x%x)\n", entry_addr, entry_addr);
        break;
    }
    case cli_hash_cam_indirect_vlid_in:
    {
        bdmf_boolean valid;
        err = ag_drv_hash_cam_indirect_vlid_in_get(&valid);
        bdmf_session_print(session, "valid = %u (0x%x)\n", valid, valid);
        break;
    }
    case cli_hash_cam_indirect_vlid_out:
    {
        bdmf_boolean valid;
        err = ag_drv_hash_cam_indirect_vlid_out_get(&valid);
        bdmf_session_print(session, "valid = %u (0x%x)\n", valid, valid);
        break;
    }
    case cli_hash_cam_indirect_rslt:
    {
        bdmf_boolean match;
        uint8_t index;
        err = ag_drv_hash_cam_indirect_rslt_get(&match, &index);
        bdmf_session_print(session, "match = %u (0x%x)\n", match, match);
        bdmf_session_print(session, "index = %u (0x%x)\n", index, index);
        break;
    }
    case cli_hash_cam_indirect_key_in:
    {
        hash_cam_indirect_key_in cam_indirect_key_in;
        err = ag_drv_hash_cam_indirect_key_in_get(parm[1].value.unumber, &cam_indirect_key_in);
        bdmf_session_print(session, "key_in[0] = %u (0x%x)\n", cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[0]);
        bdmf_session_print(session, "key_in[1] = %u (0x%x)\n", cam_indirect_key_in.key_in[1], cam_indirect_key_in.key_in[1]);
        break;
    }
    case cli_hash_cam_indirect_key_out:
    {
        hash_cam_indirect_key_out cam_indirect_key_out;
        err = ag_drv_hash_cam_indirect_key_out_get(parm[1].value.unumber, &cam_indirect_key_out);
        bdmf_session_print(session, "key_out[0] = %u (0x%x)\n", cam_indirect_key_out.key_out[0], cam_indirect_key_out.key_out[0]);
        bdmf_session_print(session, "key_out[1] = %u (0x%x)\n", cam_indirect_key_out.key_out[1], cam_indirect_key_out.key_out[1]);
        break;
    }
    case cli_hash_cam_bist_bist_status:
    {
        uint32_t value;
        err = ag_drv_hash_cam_bist_bist_status_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_compare_en:
    {
        bdmf_boolean value;
        err = ag_drv_hash_cam_bist_bist_dbg_compare_en_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_data:
    {
        uint32_t value;
        err = ag_drv_hash_cam_bist_bist_dbg_data_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_data_slice_or_status_sel:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_dbg_data_valid:
    {
        bdmf_boolean value;
        err = ag_drv_hash_cam_bist_bist_dbg_data_valid_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_en:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_en_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_mode:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_mode_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_rst_l:
    {
        bdmf_boolean value;
        err = ag_drv_hash_cam_bist_bist_rst_l_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_skip_error_cnt:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_skip_error_cnt_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_dbg_en:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_dbg_en_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_cascade_select:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_cascade_select_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_block_select:
    {
        uint8_t value;
        err = ag_drv_hash_cam_bist_bist_block_select_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_cam_bist_bist_repair_enable:
    {
        bdmf_boolean value;
        err = ag_drv_hash_cam_bist_bist_repair_enable_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_hash_intr_ctrl_isr:
    {
        hash_intr_ctrl_isr intr_ctrl_isr;
        err = ag_drv_hash_intr_ctrl_isr_get(&intr_ctrl_isr);
        bdmf_session_print(session, "invld_cmd = %u (0x%x)\n", intr_ctrl_isr.invld_cmd, intr_ctrl_isr.invld_cmd);
        bdmf_session_print(session, "mult_match = %u (0x%x)\n", intr_ctrl_isr.mult_match, intr_ctrl_isr.mult_match);
        bdmf_session_print(session, "hash_0_idx_ovflv = %u (0x%x)\n", intr_ctrl_isr.hash_0_idx_ovflv, intr_ctrl_isr.hash_0_idx_ovflv);
        bdmf_session_print(session, "hash_1_idx_ovflv = %u (0x%x)\n", intr_ctrl_isr.hash_1_idx_ovflv, intr_ctrl_isr.hash_1_idx_ovflv);
        bdmf_session_print(session, "hash_2_idx_ovflv = %u (0x%x)\n", intr_ctrl_isr.hash_2_idx_ovflv, intr_ctrl_isr.hash_2_idx_ovflv);
        bdmf_session_print(session, "hash_3_idx_ovflv = %u (0x%x)\n", intr_ctrl_isr.hash_3_idx_ovflv, intr_ctrl_isr.hash_3_idx_ovflv);
        bdmf_session_print(session, "cntxt_idx_ovflv = %u (0x%x)\n", intr_ctrl_isr.cntxt_idx_ovflv, intr_ctrl_isr.cntxt_idx_ovflv);
        break;
    }
    case cli_hash_intr_ctrl_ism:
    {
        uint32_t ism;
        err = ag_drv_hash_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u (0x%x)\n", ism, ism);
        break;
    }
    case cli_hash_intr_ctrl_ier:
    {
        uint32_t iem;
        err = ag_drv_hash_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u (0x%x)\n", iem, iem);
        break;
    }
    case cli_hash_intr_ctrl_itr:
    {
        uint32_t ist;
        err = ag_drv_hash_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
        break;
    }
    case cli_hash_debug_dbg0:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg0_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg1:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg1_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg2:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg2_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg3:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg3_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg4:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg4_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg5:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg5_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg6:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg6_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg7:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg7_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg8:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg8_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg9:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg9_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg10:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg10_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg11:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg11_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg12:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg12_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg13:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg13_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg14:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg14_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg15:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg15_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg16:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg16_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg17:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg17_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg18:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg18_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg19:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg19_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg20:
    {
        uint32_t val;
        err = ag_drv_hash_debug_dbg20_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_debug_dbg_sel:
    {
        uint8_t val;
        err = ag_drv_hash_debug_dbg_sel_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_hash_aging_ram_aging:
    {
        bdmf_boolean age;
        err = ag_drv_hash_aging_ram_aging_get(parm[1].value.unumber, &age);
        bdmf_session_print(session, "age = %u (0x%x)\n", age, age);
        break;
    }
    case cli_hash_context_ram_context_47_24:
    {
        uint32_t bits;
        err = ag_drv_hash_context_ram_context_47_24_get(parm[1].value.unumber, &bits);
        bdmf_session_print(session, "bits = %u (0x%x)\n", bits, bits);
        break;
    }
    case cli_hash_context_ram_context_23_0:
    {
        uint32_t bits;
        err = ag_drv_hash_context_ram_context_23_0_get(parm[1].value.unumber, &bits);
        bdmf_session_print(session, "bits = %u (0x%x)\n", bits, bits);
        break;
    }
    case cli_hash_ram_eng_high:
    {
        uint32_t key_59_28_or_dat;
        err = ag_drv_hash_ram_eng_high_get(parm[1].value.unumber, &key_59_28_or_dat);
        bdmf_session_print(session, "key_59_28_or_dat = %u (0x%x)\n", key_59_28_or_dat, key_59_28_or_dat);
        break;
    }
    case cli_hash_ram_eng_low:
    {
        bdmf_boolean skp;
        uint8_t cfg;
        uint16_t key_11_0;
        uint16_t key_27_12_or_dat;
        err = ag_drv_hash_ram_eng_low_get(parm[1].value.unumber, &skp, &cfg, &key_11_0, &key_27_12_or_dat);
        bdmf_session_print(session, "skp = %u (0x%x)\n", skp, skp);
        bdmf_session_print(session, "cfg = %u (0x%x)\n", cfg, cfg);
        bdmf_session_print(session, "key_11_0 = %u (0x%x)\n", key_11_0, key_11_0);
        bdmf_session_print(session, "key_27_12_or_dat = %u (0x%x)\n", key_27_12_or_dat, key_27_12_or_dat);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_hash_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t key_pad_h=gtmv(m, 28);
        uint32_t key_pad_l=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_hash_key_padding_set( %u %u)\n", key_pad_h, key_pad_l);
        if(!err) ag_drv_hash_key_padding_set(key_pad_h, key_pad_l);
        if(!err) ag_drv_hash_key_padding_get( &key_pad_h, &key_pad_l);
        if(!err) bdmf_session_print(session, "ag_drv_hash_key_padding_get( %u %u)\n", key_pad_h, key_pad_l);
        if(err || key_pad_h!=gtmv(m, 28) || key_pad_l!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tbl_idx=gtmv(m, 0);
        hash_mask mask = {.maskl=gtmv(m, 32), .maskh=gtmv(m, 28)};
        if(!err) bdmf_session_print(session, "ag_drv_hash_mask_set( %u %u %u)\n", tbl_idx, mask.maskl, mask.maskh);
        if(!err) ag_drv_hash_mask_set(tbl_idx, &mask);
        if(!err) ag_drv_hash_mask_get( tbl_idx, &mask);
        if(!err) bdmf_session_print(session, "ag_drv_hash_mask_get( %u %u %u)\n", tbl_idx, mask.maskl, mask.maskh);
        if(err || mask.maskl!=gtmv(m, 32) || mask.maskh!=gtmv(m, 28))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean value=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_general_configuration_pwr_sav_en_set( %u)\n", value);
        if(!err) ag_drv_hash_general_configuration_pwr_sav_en_set(value);
        if(!err) ag_drv_hash_general_configuration_pwr_sav_en_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_general_configuration_pwr_sav_en_get( %u)\n", value);
        if(err || value!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t val=gtmv(m, 4);
        if(!err) ag_drv_hash_general_configuration_mult_hit_err_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_general_configuration_mult_hit_err_get( %u)\n", val);
    }
    {
        bdmf_boolean frst_mul_hit=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_general_configuration_undo_fix_set( %u)\n", frst_mul_hit);
        if(!err) ag_drv_hash_general_configuration_undo_fix_set(frst_mul_hit);
        if(!err) ag_drv_hash_general_configuration_undo_fix_get( &frst_mul_hit);
        if(!err) bdmf_session_print(session, "ag_drv_hash_general_configuration_undo_fix_get( %u)\n", frst_mul_hit);
        if(err || frst_mul_hit!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_hits_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_hits_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_srchs_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_srchs_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_miss_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_miss_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_hit_1st_acs_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_hit_1st_acs_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_hit_2nd_acs_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_hit_2nd_acs_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_hit_3rd_acs_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_hit_3rd_acs_get( %u)\n", cnt);
    }
    {
        uint32_t cnt=gtmv(m, 32);
        if(!err) ag_drv_hash_pm_counters_hit_4th_acs_get( &cnt);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_hit_4th_acs_get( %u)\n", cnt);
    }
    {
        bdmf_boolean val=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_frz_cnt_set( %u)\n", val);
        if(!err) ag_drv_hash_pm_counters_frz_cnt_set(val);
        if(!err) ag_drv_hash_pm_counters_frz_cnt_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_pm_counters_frz_cnt_get( %u)\n", val);
        if(err || val!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tbl_idx=gtmv(m, 0);
        hash_lkup_tbl_cfg_tbl_cfg lkup_tbl_cfg_tbl_cfg = {.hash_base_addr=gtmv(m, 11), .tbl_size=gtmv(m, 3), .max_hop=gtmv(m, 4), .cam_en=gtmv(m, 1), .direct_lkup_en=gtmv(m, 1), .hash_type=gtmv(m, 1), .int_ctx_size=gtmv(m, 2)};
        if(!err) bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set( %u %u %u %u %u %u %u %u)\n", tbl_idx, lkup_tbl_cfg_tbl_cfg.hash_base_addr, lkup_tbl_cfg_tbl_cfg.tbl_size, lkup_tbl_cfg_tbl_cfg.max_hop, lkup_tbl_cfg_tbl_cfg.cam_en, lkup_tbl_cfg_tbl_cfg.direct_lkup_en, lkup_tbl_cfg_tbl_cfg.hash_type, lkup_tbl_cfg_tbl_cfg.int_ctx_size);
        if(!err) ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(tbl_idx, &lkup_tbl_cfg_tbl_cfg);
        if(!err) ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get( tbl_idx, &lkup_tbl_cfg_tbl_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get( %u %u %u %u %u %u %u %u)\n", tbl_idx, lkup_tbl_cfg_tbl_cfg.hash_base_addr, lkup_tbl_cfg_tbl_cfg.tbl_size, lkup_tbl_cfg_tbl_cfg.max_hop, lkup_tbl_cfg_tbl_cfg.cam_en, lkup_tbl_cfg_tbl_cfg.direct_lkup_en, lkup_tbl_cfg_tbl_cfg.hash_type, lkup_tbl_cfg_tbl_cfg.int_ctx_size);
        if(err || lkup_tbl_cfg_tbl_cfg.hash_base_addr!=gtmv(m, 11) || lkup_tbl_cfg_tbl_cfg.tbl_size!=gtmv(m, 3) || lkup_tbl_cfg_tbl_cfg.max_hop!=gtmv(m, 4) || lkup_tbl_cfg_tbl_cfg.cam_en!=gtmv(m, 1) || lkup_tbl_cfg_tbl_cfg.direct_lkup_en!=gtmv(m, 1) || lkup_tbl_cfg_tbl_cfg.hash_type!=gtmv(m, 1) || lkup_tbl_cfg_tbl_cfg.int_ctx_size!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tbl_idx=gtmv(m, 0);
        hash_lkup_tbl_cfg_cntxt_cfg lkup_tbl_cfg_cntxt_cfg = {.base_address=gtmv(m, 12), .first_hash_idx=gtmv(m, 13), .ext_ctx_size=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set( %u %u %u %u)\n", tbl_idx, lkup_tbl_cfg_cntxt_cfg.base_address, lkup_tbl_cfg_cntxt_cfg.first_hash_idx, lkup_tbl_cfg_cntxt_cfg.ext_ctx_size);
        if(!err) ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(tbl_idx, &lkup_tbl_cfg_cntxt_cfg);
        if(!err) ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get( tbl_idx, &lkup_tbl_cfg_cntxt_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get( %u %u %u %u)\n", tbl_idx, lkup_tbl_cfg_cntxt_cfg.base_address, lkup_tbl_cfg_cntxt_cfg.first_hash_idx, lkup_tbl_cfg_cntxt_cfg.ext_ctx_size);
        if(err || lkup_tbl_cfg_cntxt_cfg.base_address!=gtmv(m, 12) || lkup_tbl_cfg_cntxt_cfg.first_hash_idx!=gtmv(m, 13) || lkup_tbl_cfg_cntxt_cfg.ext_ctx_size!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t base_address=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_base_addr_set( %u)\n", base_address);
        if(!err) ag_drv_hash_cam_base_addr_set(base_address);
        if(!err) ag_drv_hash_cam_base_addr_get( &base_address);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_base_addr_get( %u)\n", base_address);
        if(err || base_address!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cmd=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_set( %u)\n", cmd);
        if(!err) ag_drv_hash_cam_indirect_op_set(cmd);
        if(!err) ag_drv_hash_cam_indirect_op_get( &cmd);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_get( %u)\n", cmd);
        if(err || cmd!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_hash_cam_indirect_op_done_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_op_done_get( %u)\n", val);
    }
    {
        bdmf_boolean key1_ind=gtmv(m, 1);
        uint8_t entry_addr=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_addr_set( %u %u)\n", key1_ind, entry_addr);
        if(!err) ag_drv_hash_cam_indirect_addr_set(key1_ind, entry_addr);
        if(!err) ag_drv_hash_cam_indirect_addr_get( &key1_ind, &entry_addr);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_addr_get( %u %u)\n", key1_ind, entry_addr);
        if(err || key1_ind!=gtmv(m, 1) || entry_addr!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean valid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_in_set( %u)\n", valid);
        if(!err) ag_drv_hash_cam_indirect_vlid_in_set(valid);
        if(!err) ag_drv_hash_cam_indirect_vlid_in_get( &valid);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_in_get( %u)\n", valid);
        if(err || valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean valid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_out_set( %u)\n", valid);
        if(!err) ag_drv_hash_cam_indirect_vlid_out_set(valid);
        if(!err) ag_drv_hash_cam_indirect_vlid_out_get( &valid);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_vlid_out_get( %u)\n", valid);
        if(err || valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean match=gtmv(m, 1);
        uint8_t index=gtmv(m, 6);
        if(!err) ag_drv_hash_cam_indirect_rslt_get( &match, &index);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_rslt_get( %u %u)\n", match, index);
    }
    {
        uint8_t zero=gtmv(m, 0);
        hash_cam_indirect_key_in cam_indirect_key_in = {.key_in={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_set( %u %u %u)\n", zero, cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
        if(!err) ag_drv_hash_cam_indirect_key_in_set(zero, &cam_indirect_key_in);
        if(!err) ag_drv_hash_cam_indirect_key_in_get( zero, &cam_indirect_key_in);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_in_get( %u %u %u)\n", zero, cam_indirect_key_in.key_in[0], cam_indirect_key_in.key_in[1]);
        if(err || cam_indirect_key_in.key_in[0]!=gtmv(m, 32) || cam_indirect_key_in.key_in[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        hash_cam_indirect_key_out cam_indirect_key_out = {.key_out={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_hash_cam_indirect_key_out_get( zero, &cam_indirect_key_out);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_indirect_key_out_get( %u %u %u)\n", zero, cam_indirect_key_out.key_out[0], cam_indirect_key_out.key_out[1]);
    }
    {
        uint32_t value=gtmv(m, 32);
        if(!err) ag_drv_hash_cam_bist_bist_status_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_status_get( %u)\n", value);
    }
    {
        bdmf_boolean value=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_compare_en_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_compare_en_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_compare_en_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_compare_en_get( %u)\n", value);
        if(err || value!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t value=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_get( %u)\n", value);
        if(err || value!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_slice_or_status_sel_get( %u)\n", value);
        if(err || value!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean value=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_valid_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_valid_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_dbg_data_valid_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_dbg_data_valid_get( %u)\n", value);
        if(err || value!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_en_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_en_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_en_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_en_get( %u)\n", value);
        if(err || value!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_mode_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_mode_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_mode_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_mode_get( %u)\n", value);
        if(err || value!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean value=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_rst_l_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_rst_l_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_rst_l_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_rst_l_get( %u)\n", value);
        if(err || value!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_skip_error_cnt_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_skip_error_cnt_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_skip_error_cnt_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_skip_error_cnt_get( %u)\n", value);
        if(err || value!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_dbg_en_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_dbg_en_set(value);
        if(!err) ag_drv_hash_cam_bist_dbg_en_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_dbg_en_get( %u)\n", value);
        if(err || value!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_cascade_select_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_cascade_select_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_cascade_select_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_cascade_select_get( %u)\n", value);
        if(err || value!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_block_select_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_block_select_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_block_select_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_block_select_get( %u)\n", value);
        if(err || value!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean value=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_repair_enable_set( %u)\n", value);
        if(!err) ag_drv_hash_cam_bist_bist_repair_enable_set(value);
        if(!err) ag_drv_hash_cam_bist_bist_repair_enable_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_hash_cam_bist_bist_repair_enable_get( %u)\n", value);
        if(err || value!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        hash_intr_ctrl_isr intr_ctrl_isr = {.invld_cmd=gtmv(m, 1), .mult_match=gtmv(m, 1), .hash_0_idx_ovflv=gtmv(m, 1), .hash_1_idx_ovflv=gtmv(m, 1), .hash_2_idx_ovflv=gtmv(m, 1), .hash_3_idx_ovflv=gtmv(m, 1), .cntxt_idx_ovflv=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_isr_set( %u %u %u %u %u %u %u)\n", intr_ctrl_isr.invld_cmd, intr_ctrl_isr.mult_match, intr_ctrl_isr.hash_0_idx_ovflv, intr_ctrl_isr.hash_1_idx_ovflv, intr_ctrl_isr.hash_2_idx_ovflv, intr_ctrl_isr.hash_3_idx_ovflv, intr_ctrl_isr.cntxt_idx_ovflv);
        if(!err) ag_drv_hash_intr_ctrl_isr_set(&intr_ctrl_isr);
        if(!err) ag_drv_hash_intr_ctrl_isr_get( &intr_ctrl_isr);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_isr_get( %u %u %u %u %u %u %u)\n", intr_ctrl_isr.invld_cmd, intr_ctrl_isr.mult_match, intr_ctrl_isr.hash_0_idx_ovflv, intr_ctrl_isr.hash_1_idx_ovflv, intr_ctrl_isr.hash_2_idx_ovflv, intr_ctrl_isr.hash_3_idx_ovflv, intr_ctrl_isr.cntxt_idx_ovflv);
        if(err || intr_ctrl_isr.invld_cmd!=gtmv(m, 1) || intr_ctrl_isr.mult_match!=gtmv(m, 1) || intr_ctrl_isr.hash_0_idx_ovflv!=gtmv(m, 1) || intr_ctrl_isr.hash_1_idx_ovflv!=gtmv(m, 1) || intr_ctrl_isr.hash_2_idx_ovflv!=gtmv(m, 1) || intr_ctrl_isr.hash_3_idx_ovflv!=gtmv(m, 1) || intr_ctrl_isr.cntxt_idx_ovflv!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 32);
        if(!err) ag_drv_hash_intr_ctrl_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ier_set( %u)\n", iem);
        if(!err) ag_drv_hash_intr_ctrl_ier_set(iem);
        if(!err) ag_drv_hash_intr_ctrl_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_itr_set( %u)\n", ist);
        if(!err) ag_drv_hash_intr_ctrl_itr_set(ist);
        if(!err) ag_drv_hash_intr_ctrl_itr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_hash_intr_ctrl_itr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg0_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg0_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg1_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg1_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg2_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg2_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg3_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg3_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg4_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg4_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg5_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg5_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg6_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg6_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg7_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg7_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg8_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg8_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg9_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg9_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg10_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg10_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg11_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg11_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg12_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg12_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg13_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg13_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg14_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg14_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg15_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg15_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg16_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg16_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg17_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg17_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg18_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg18_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg19_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg19_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_hash_debug_dbg20_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg20_get( %u)\n", val);
    }
    {
        uint8_t val=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg_sel_set( %u)\n", val);
        if(!err) ag_drv_hash_debug_dbg_sel_set(val);
        if(!err) ag_drv_hash_debug_dbg_sel_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_hash_debug_dbg_sel_get( %u)\n", val);
        if(err || val!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t entry_idx=gtmv(m, 6);
        bdmf_boolean age=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_set( %u %u)\n", entry_idx, age);
        if(!err) ag_drv_hash_aging_ram_aging_set(entry_idx, age);
        if(!err) ag_drv_hash_aging_ram_aging_get( entry_idx, &age);
        if(!err) bdmf_session_print(session, "ag_drv_hash_aging_ram_aging_get( %u %u)\n", entry_idx, age);
        if(err || age!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ctx_idx=gtmv(m, 6);
        uint32_t bits=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_hash_context_ram_context_47_24_set( %u %u)\n", ctx_idx, bits);
        if(!err) ag_drv_hash_context_ram_context_47_24_set(ctx_idx, bits);
        if(!err) ag_drv_hash_context_ram_context_47_24_get( ctx_idx, &bits);
        if(!err) bdmf_session_print(session, "ag_drv_hash_context_ram_context_47_24_get( %u %u)\n", ctx_idx, bits);
        if(err || bits!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ctx_idx=gtmv(m, 6);
        uint32_t bits=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_hash_context_ram_context_23_0_set( %u %u)\n", ctx_idx, bits);
        if(!err) ag_drv_hash_context_ram_context_23_0_set(ctx_idx, bits);
        if(!err) ag_drv_hash_context_ram_context_23_0_get( ctx_idx, &bits);
        if(!err) bdmf_session_print(session, "ag_drv_hash_context_ram_context_23_0_get( %u %u)\n", ctx_idx, bits);
        if(err || bits!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t idx=gtmv(m, 11);
        uint32_t key_59_28_or_dat=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_hash_ram_eng_high_set( %u %u)\n", idx, key_59_28_or_dat);
        if(!err) ag_drv_hash_ram_eng_high_set(idx, key_59_28_or_dat);
        if(!err) ag_drv_hash_ram_eng_high_get( idx, &key_59_28_or_dat);
        if(!err) bdmf_session_print(session, "ag_drv_hash_ram_eng_high_get( %u %u)\n", idx, key_59_28_or_dat);
        if(err || key_59_28_or_dat!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t idx=gtmv(m, 11);
        bdmf_boolean skp=gtmv(m, 1);
        uint8_t cfg=gtmv(m, 3);
        uint16_t key_11_0=gtmv(m, 12);
        uint16_t key_27_12_or_dat=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_hash_ram_eng_low_set( %u %u %u %u %u)\n", idx, skp, cfg, key_11_0, key_27_12_or_dat);
        if(!err) ag_drv_hash_ram_eng_low_set(idx, skp, cfg, key_11_0, key_27_12_or_dat);
        if(!err) ag_drv_hash_ram_eng_low_get( idx, &skp, &cfg, &key_11_0, &key_27_12_or_dat);
        if(!err) bdmf_session_print(session, "ag_drv_hash_ram_eng_low_get( %u %u %u %u %u)\n", idx, skp, cfg, key_11_0, key_27_12_or_dat);
        if(err || skp!=gtmv(m, 1) || cfg!=gtmv(m, 3) || key_11_0!=gtmv(m, 12) || key_27_12_or_dat!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_hash_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_general_configuration_pwr_sav_en : reg = &RU_REG(HASH, GENERAL_CONFIGURATION_PWR_SAV_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_pad_high : reg = &RU_REG(HASH, GENERAL_CONFIGURATION_PAD_HIGH); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_pad_low : reg = &RU_REG(HASH, GENERAL_CONFIGURATION_PAD_LOW); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_mult_hit_err : reg = &RU_REG(HASH, GENERAL_CONFIGURATION_MULT_HIT_ERR); blk = &RU_BLK(HASH); break;
    case bdmf_address_general_configuration_undo_fix : reg = &RU_REG(HASH, GENERAL_CONFIGURATION_UNDO_FIX); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hits : reg = &RU_REG(HASH, PM_COUNTERS_HITS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_srchs : reg = &RU_REG(HASH, PM_COUNTERS_SRCHS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_miss : reg = &RU_REG(HASH, PM_COUNTERS_MISS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hit_1st_acs : reg = &RU_REG(HASH, PM_COUNTERS_HIT_1ST_ACS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hit_2nd_acs : reg = &RU_REG(HASH, PM_COUNTERS_HIT_2ND_ACS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hit_3rd_acs : reg = &RU_REG(HASH, PM_COUNTERS_HIT_3RD_ACS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_hit_4th_acs : reg = &RU_REG(HASH, PM_COUNTERS_HIT_4TH_ACS); blk = &RU_BLK(HASH); break;
    case bdmf_address_pm_counters_frz_cnt : reg = &RU_REG(HASH, PM_COUNTERS_FRZ_CNT); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_tbl_cfg : reg = &RU_REG(HASH, LKUP_TBL_CFG_TBL_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_high : reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_HIGH); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_key_mask_low : reg = &RU_REG(HASH, LKUP_TBL_CFG_KEY_MASK_LOW); blk = &RU_BLK(HASH); break;
    case bdmf_address_lkup_tbl_cfg_cntxt_cfg : reg = &RU_REG(HASH, LKUP_TBL_CFG_CNTXT_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_configuration_cntxt_cfg : reg = &RU_REG(HASH, CAM_CONFIGURATION_CNTXT_CFG); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_op : reg = &RU_REG(HASH, CAM_INDIRECT_OP); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_op_done : reg = &RU_REG(HASH, CAM_INDIRECT_OP_DONE); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_addr : reg = &RU_REG(HASH, CAM_INDIRECT_ADDR); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_vlid_in : reg = &RU_REG(HASH, CAM_INDIRECT_VLID_IN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_vlid_out : reg = &RU_REG(HASH, CAM_INDIRECT_VLID_OUT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_rslt : reg = &RU_REG(HASH, CAM_INDIRECT_RSLT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_key_in : reg = &RU_REG(HASH, CAM_INDIRECT_KEY_IN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_indirect_key_out : reg = &RU_REG(HASH, CAM_INDIRECT_KEY_OUT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_status : reg = &RU_REG(HASH, CAM_BIST_BIST_STATUS); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_compare_en : reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_COMPARE_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_data : reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_DATA); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_data_slice_or_status_sel : reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_dbg_data_valid : reg = &RU_REG(HASH, CAM_BIST_BIST_DBG_DATA_VALID); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_en : reg = &RU_REG(HASH, CAM_BIST_BIST_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_mode : reg = &RU_REG(HASH, CAM_BIST_BIST_MODE); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_rst_l : reg = &RU_REG(HASH, CAM_BIST_BIST_RST_L); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_skip_error_cnt : reg = &RU_REG(HASH, CAM_BIST_BIST_SKIP_ERROR_CNT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_dbg_en : reg = &RU_REG(HASH, CAM_BIST_DBG_EN); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_cascade_select : reg = &RU_REG(HASH, CAM_BIST_BIST_CASCADE_SELECT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_block_select : reg = &RU_REG(HASH, CAM_BIST_BIST_BLOCK_SELECT); blk = &RU_BLK(HASH); break;
    case bdmf_address_cam_bist_bist_repair_enable : reg = &RU_REG(HASH, CAM_BIST_BIST_REPAIR_ENABLE); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_isr : reg = &RU_REG(HASH, INTR_CTRL_ISR); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_ism : reg = &RU_REG(HASH, INTR_CTRL_ISM); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_ier : reg = &RU_REG(HASH, INTR_CTRL_IER); blk = &RU_BLK(HASH); break;
    case bdmf_address_intr_ctrl_itr : reg = &RU_REG(HASH, INTR_CTRL_ITR); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg0 : reg = &RU_REG(HASH, DEBUG_DBG0); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg1 : reg = &RU_REG(HASH, DEBUG_DBG1); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg2 : reg = &RU_REG(HASH, DEBUG_DBG2); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg3 : reg = &RU_REG(HASH, DEBUG_DBG3); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg4 : reg = &RU_REG(HASH, DEBUG_DBG4); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg5 : reg = &RU_REG(HASH, DEBUG_DBG5); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg6 : reg = &RU_REG(HASH, DEBUG_DBG6); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg7 : reg = &RU_REG(HASH, DEBUG_DBG7); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg8 : reg = &RU_REG(HASH, DEBUG_DBG8); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg9 : reg = &RU_REG(HASH, DEBUG_DBG9); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg10 : reg = &RU_REG(HASH, DEBUG_DBG10); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg11 : reg = &RU_REG(HASH, DEBUG_DBG11); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg12 : reg = &RU_REG(HASH, DEBUG_DBG12); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg13 : reg = &RU_REG(HASH, DEBUG_DBG13); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg14 : reg = &RU_REG(HASH, DEBUG_DBG14); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg15 : reg = &RU_REG(HASH, DEBUG_DBG15); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg16 : reg = &RU_REG(HASH, DEBUG_DBG16); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg17 : reg = &RU_REG(HASH, DEBUG_DBG17); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg18 : reg = &RU_REG(HASH, DEBUG_DBG18); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg19 : reg = &RU_REG(HASH, DEBUG_DBG19); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg20 : reg = &RU_REG(HASH, DEBUG_DBG20); blk = &RU_BLK(HASH); break;
    case bdmf_address_debug_dbg_sel : reg = &RU_REG(HASH, DEBUG_DBG_SEL); blk = &RU_BLK(HASH); break;
    case bdmf_address_aging_ram_aging : reg = &RU_REG(HASH, AGING_RAM_AGING); blk = &RU_BLK(HASH); break;
    case bdmf_address_context_ram_context_47_24 : reg = &RU_REG(HASH, CONTEXT_RAM_CONTEXT_47_24); blk = &RU_BLK(HASH); break;
    case bdmf_address_context_ram_context_23_0 : reg = &RU_REG(HASH, CONTEXT_RAM_CONTEXT_23_0); blk = &RU_BLK(HASH); break;
    case bdmf_address_ram_eng_high : reg = &RU_REG(HASH, RAM_ENG_HIGH); blk = &RU_BLK(HASH); break;
    case bdmf_address_ram_eng_low : reg = &RU_REG(HASH, RAM_ENG_LOW); blk = &RU_BLK(HASH); break;
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

bdmfmon_handle_t ag_drv_hash_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "hash"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "hash", "hash", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_key_padding[]={
            BDMFMON_MAKE_PARM("key_pad_h", "key_pad_h", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("key_pad_l", "key_pad_l", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskl", "maskl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskh", "maskh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_pwr_sav_en[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_undo_fix[]={
            BDMFMON_MAKE_PARM("frst_mul_hit", "frst_mul_hit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_frz_cnt[]={
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_tbl_cfg[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_base_addr", "hash_base_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tbl_size", "tbl_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("max_hop", "max_hop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cam_en", "cam_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("direct_lkup_en", "direct_lkup_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_type", "hash_type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_ctx_size", "int_ctx_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_cntxt_cfg[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("base_address", "base_address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("first_hash_idx", "first_hash_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ext_ctx_size", "ext_ctx_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_base_addr[]={
            BDMFMON_MAKE_PARM("base_address", "base_address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_op[]={
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_addr[]={
            BDMFMON_MAKE_PARM("key1_ind", "key1_ind", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("entry_addr", "entry_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_vlid_in[]={
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_vlid_out[]={
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_key_in[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("key_in0", "key_in0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("key_in1", "key_in1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_compare_en[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_data[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_data_slice_or_status_sel[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_dbg_data_valid[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_en[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_mode[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_rst_l[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_skip_error_cnt[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_dbg_en[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_cascade_select[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_block_select[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_bist_bist_repair_enable[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[]={
            BDMFMON_MAKE_PARM("invld_cmd", "invld_cmd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mult_match", "mult_match", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_0_idx_ovflv", "hash_0_idx_ovflv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_1_idx_ovflv", "hash_1_idx_ovflv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_2_idx_ovflv", "hash_2_idx_ovflv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hash_3_idx_ovflv", "hash_3_idx_ovflv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cntxt_idx_ovflv", "cntxt_idx_ovflv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[]={
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_dbg_sel[]={
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aging_ram_aging[]={
            BDMFMON_MAKE_PARM("entry_idx", "entry_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("age", "age", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_ram_context_47_24[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bits", "bits", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_ram_context_23_0[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bits", "bits", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ram_eng_high[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("key_59_28_or_dat", "key_59_28_or_dat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ram_eng_low[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("skp", "skp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg", "cfg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("key_11_0", "key_11_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("key_27_12_or_dat", "key_27_12_or_dat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="key_padding", .val=cli_hash_key_padding, .parms=set_key_padding },
            { .name="mask", .val=cli_hash_mask, .parms=set_mask },
            { .name="general_configuration_pwr_sav_en", .val=cli_hash_general_configuration_pwr_sav_en, .parms=set_general_configuration_pwr_sav_en },
            { .name="general_configuration_undo_fix", .val=cli_hash_general_configuration_undo_fix, .parms=set_general_configuration_undo_fix },
            { .name="pm_counters_frz_cnt", .val=cli_hash_pm_counters_frz_cnt, .parms=set_pm_counters_frz_cnt },
            { .name="lkup_tbl_cfg_tbl_cfg", .val=cli_hash_lkup_tbl_cfg_tbl_cfg, .parms=set_lkup_tbl_cfg_tbl_cfg },
            { .name="lkup_tbl_cfg_cntxt_cfg", .val=cli_hash_lkup_tbl_cfg_cntxt_cfg, .parms=set_lkup_tbl_cfg_cntxt_cfg },
            { .name="cam_base_addr", .val=cli_hash_cam_base_addr, .parms=set_cam_base_addr },
            { .name="cam_indirect_op", .val=cli_hash_cam_indirect_op, .parms=set_cam_indirect_op },
            { .name="cam_indirect_addr", .val=cli_hash_cam_indirect_addr, .parms=set_cam_indirect_addr },
            { .name="cam_indirect_vlid_in", .val=cli_hash_cam_indirect_vlid_in, .parms=set_cam_indirect_vlid_in },
            { .name="cam_indirect_vlid_out", .val=cli_hash_cam_indirect_vlid_out, .parms=set_cam_indirect_vlid_out },
            { .name="cam_indirect_key_in", .val=cli_hash_cam_indirect_key_in, .parms=set_cam_indirect_key_in },
            { .name="cam_bist_bist_dbg_compare_en", .val=cli_hash_cam_bist_bist_dbg_compare_en, .parms=set_cam_bist_bist_dbg_compare_en },
            { .name="cam_bist_bist_dbg_data", .val=cli_hash_cam_bist_bist_dbg_data, .parms=set_cam_bist_bist_dbg_data },
            { .name="cam_bist_bist_dbg_data_slice_or_status_sel", .val=cli_hash_cam_bist_bist_dbg_data_slice_or_status_sel, .parms=set_cam_bist_bist_dbg_data_slice_or_status_sel },
            { .name="cam_bist_bist_dbg_data_valid", .val=cli_hash_cam_bist_bist_dbg_data_valid, .parms=set_cam_bist_bist_dbg_data_valid },
            { .name="cam_bist_bist_en", .val=cli_hash_cam_bist_bist_en, .parms=set_cam_bist_bist_en },
            { .name="cam_bist_bist_mode", .val=cli_hash_cam_bist_bist_mode, .parms=set_cam_bist_bist_mode },
            { .name="cam_bist_bist_rst_l", .val=cli_hash_cam_bist_bist_rst_l, .parms=set_cam_bist_bist_rst_l },
            { .name="cam_bist_bist_skip_error_cnt", .val=cli_hash_cam_bist_bist_skip_error_cnt, .parms=set_cam_bist_bist_skip_error_cnt },
            { .name="cam_bist_dbg_en", .val=cli_hash_cam_bist_dbg_en, .parms=set_cam_bist_dbg_en },
            { .name="cam_bist_bist_cascade_select", .val=cli_hash_cam_bist_bist_cascade_select, .parms=set_cam_bist_bist_cascade_select },
            { .name="cam_bist_bist_block_select", .val=cli_hash_cam_bist_bist_block_select, .parms=set_cam_bist_bist_block_select },
            { .name="cam_bist_bist_repair_enable", .val=cli_hash_cam_bist_bist_repair_enable, .parms=set_cam_bist_bist_repair_enable },
            { .name="intr_ctrl_isr", .val=cli_hash_intr_ctrl_isr, .parms=set_intr_ctrl_isr },
            { .name="intr_ctrl_ier", .val=cli_hash_intr_ctrl_ier, .parms=set_intr_ctrl_ier },
            { .name="intr_ctrl_itr", .val=cli_hash_intr_ctrl_itr, .parms=set_intr_ctrl_itr },
            { .name="debug_dbg_sel", .val=cli_hash_debug_dbg_sel, .parms=set_debug_dbg_sel },
            { .name="aging_ram_aging", .val=cli_hash_aging_ram_aging, .parms=set_aging_ram_aging },
            { .name="context_ram_context_47_24", .val=cli_hash_context_ram_context_47_24, .parms=set_context_ram_context_47_24 },
            { .name="context_ram_context_23_0", .val=cli_hash_context_ram_context_23_0, .parms=set_context_ram_context_23_0 },
            { .name="ram_eng_high", .val=cli_hash_ram_eng_high, .parms=set_ram_eng_high },
            { .name="ram_eng_low", .val=cli_hash_ram_eng_low, .parms=set_ram_eng_low },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_hash_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_tbl_cfg[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lkup_tbl_cfg_cntxt_cfg[]={
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_key_in[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_indirect_key_out[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aging_ram_aging[]={
            BDMFMON_MAKE_PARM("entry_idx", "entry_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_ram_context_47_24[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_ram_context_23_0[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ram_eng_high[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ram_eng_low[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="key_padding", .val=cli_hash_key_padding, .parms=set_default },
            { .name="mask", .val=cli_hash_mask, .parms=set_mask },
            { .name="general_configuration_pwr_sav_en", .val=cli_hash_general_configuration_pwr_sav_en, .parms=set_default },
            { .name="general_configuration_mult_hit_err", .val=cli_hash_general_configuration_mult_hit_err, .parms=set_default },
            { .name="general_configuration_undo_fix", .val=cli_hash_general_configuration_undo_fix, .parms=set_default },
            { .name="pm_counters_hits", .val=cli_hash_pm_counters_hits, .parms=set_default },
            { .name="pm_counters_srchs", .val=cli_hash_pm_counters_srchs, .parms=set_default },
            { .name="pm_counters_miss", .val=cli_hash_pm_counters_miss, .parms=set_default },
            { .name="pm_counters_hit_1st_acs", .val=cli_hash_pm_counters_hit_1st_acs, .parms=set_default },
            { .name="pm_counters_hit_2nd_acs", .val=cli_hash_pm_counters_hit_2nd_acs, .parms=set_default },
            { .name="pm_counters_hit_3rd_acs", .val=cli_hash_pm_counters_hit_3rd_acs, .parms=set_default },
            { .name="pm_counters_hit_4th_acs", .val=cli_hash_pm_counters_hit_4th_acs, .parms=set_default },
            { .name="pm_counters_frz_cnt", .val=cli_hash_pm_counters_frz_cnt, .parms=set_default },
            { .name="lkup_tbl_cfg_tbl_cfg", .val=cli_hash_lkup_tbl_cfg_tbl_cfg, .parms=set_lkup_tbl_cfg_tbl_cfg },
            { .name="lkup_tbl_cfg_cntxt_cfg", .val=cli_hash_lkup_tbl_cfg_cntxt_cfg, .parms=set_lkup_tbl_cfg_cntxt_cfg },
            { .name="cam_base_addr", .val=cli_hash_cam_base_addr, .parms=set_default },
            { .name="cam_indirect_op", .val=cli_hash_cam_indirect_op, .parms=set_default },
            { .name="cam_indirect_op_done", .val=cli_hash_cam_indirect_op_done, .parms=set_default },
            { .name="cam_indirect_addr", .val=cli_hash_cam_indirect_addr, .parms=set_default },
            { .name="cam_indirect_vlid_in", .val=cli_hash_cam_indirect_vlid_in, .parms=set_default },
            { .name="cam_indirect_vlid_out", .val=cli_hash_cam_indirect_vlid_out, .parms=set_default },
            { .name="cam_indirect_rslt", .val=cli_hash_cam_indirect_rslt, .parms=set_default },
            { .name="cam_indirect_key_in", .val=cli_hash_cam_indirect_key_in, .parms=set_cam_indirect_key_in },
            { .name="cam_indirect_key_out", .val=cli_hash_cam_indirect_key_out, .parms=set_cam_indirect_key_out },
            { .name="cam_bist_bist_status", .val=cli_hash_cam_bist_bist_status, .parms=set_default },
            { .name="cam_bist_bist_dbg_compare_en", .val=cli_hash_cam_bist_bist_dbg_compare_en, .parms=set_default },
            { .name="cam_bist_bist_dbg_data", .val=cli_hash_cam_bist_bist_dbg_data, .parms=set_default },
            { .name="cam_bist_bist_dbg_data_slice_or_status_sel", .val=cli_hash_cam_bist_bist_dbg_data_slice_or_status_sel, .parms=set_default },
            { .name="cam_bist_bist_dbg_data_valid", .val=cli_hash_cam_bist_bist_dbg_data_valid, .parms=set_default },
            { .name="cam_bist_bist_en", .val=cli_hash_cam_bist_bist_en, .parms=set_default },
            { .name="cam_bist_bist_mode", .val=cli_hash_cam_bist_bist_mode, .parms=set_default },
            { .name="cam_bist_bist_rst_l", .val=cli_hash_cam_bist_bist_rst_l, .parms=set_default },
            { .name="cam_bist_bist_skip_error_cnt", .val=cli_hash_cam_bist_bist_skip_error_cnt, .parms=set_default },
            { .name="cam_bist_dbg_en", .val=cli_hash_cam_bist_dbg_en, .parms=set_default },
            { .name="cam_bist_bist_cascade_select", .val=cli_hash_cam_bist_bist_cascade_select, .parms=set_default },
            { .name="cam_bist_bist_block_select", .val=cli_hash_cam_bist_bist_block_select, .parms=set_default },
            { .name="cam_bist_bist_repair_enable", .val=cli_hash_cam_bist_bist_repair_enable, .parms=set_default },
            { .name="intr_ctrl_isr", .val=cli_hash_intr_ctrl_isr, .parms=set_default },
            { .name="intr_ctrl_ism", .val=cli_hash_intr_ctrl_ism, .parms=set_default },
            { .name="intr_ctrl_ier", .val=cli_hash_intr_ctrl_ier, .parms=set_default },
            { .name="intr_ctrl_itr", .val=cli_hash_intr_ctrl_itr, .parms=set_default },
            { .name="debug_dbg0", .val=cli_hash_debug_dbg0, .parms=set_default },
            { .name="debug_dbg1", .val=cli_hash_debug_dbg1, .parms=set_default },
            { .name="debug_dbg2", .val=cli_hash_debug_dbg2, .parms=set_default },
            { .name="debug_dbg3", .val=cli_hash_debug_dbg3, .parms=set_default },
            { .name="debug_dbg4", .val=cli_hash_debug_dbg4, .parms=set_default },
            { .name="debug_dbg5", .val=cli_hash_debug_dbg5, .parms=set_default },
            { .name="debug_dbg6", .val=cli_hash_debug_dbg6, .parms=set_default },
            { .name="debug_dbg7", .val=cli_hash_debug_dbg7, .parms=set_default },
            { .name="debug_dbg8", .val=cli_hash_debug_dbg8, .parms=set_default },
            { .name="debug_dbg9", .val=cli_hash_debug_dbg9, .parms=set_default },
            { .name="debug_dbg10", .val=cli_hash_debug_dbg10, .parms=set_default },
            { .name="debug_dbg11", .val=cli_hash_debug_dbg11, .parms=set_default },
            { .name="debug_dbg12", .val=cli_hash_debug_dbg12, .parms=set_default },
            { .name="debug_dbg13", .val=cli_hash_debug_dbg13, .parms=set_default },
            { .name="debug_dbg14", .val=cli_hash_debug_dbg14, .parms=set_default },
            { .name="debug_dbg15", .val=cli_hash_debug_dbg15, .parms=set_default },
            { .name="debug_dbg16", .val=cli_hash_debug_dbg16, .parms=set_default },
            { .name="debug_dbg17", .val=cli_hash_debug_dbg17, .parms=set_default },
            { .name="debug_dbg18", .val=cli_hash_debug_dbg18, .parms=set_default },
            { .name="debug_dbg19", .val=cli_hash_debug_dbg19, .parms=set_default },
            { .name="debug_dbg20", .val=cli_hash_debug_dbg20, .parms=set_default },
            { .name="debug_dbg_sel", .val=cli_hash_debug_dbg_sel, .parms=set_default },
            { .name="aging_ram_aging", .val=cli_hash_aging_ram_aging, .parms=set_aging_ram_aging },
            { .name="context_ram_context_47_24", .val=cli_hash_context_ram_context_47_24, .parms=set_context_ram_context_47_24 },
            { .name="context_ram_context_23_0", .val=cli_hash_context_ram_context_23_0, .parms=set_context_ram_context_23_0 },
            { .name="ram_eng_high", .val=cli_hash_ram_eng_high, .parms=set_ram_eng_high },
            { .name="ram_eng_low", .val=cli_hash_ram_eng_low, .parms=set_ram_eng_low },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_hash_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_hash_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="GENERAL_CONFIGURATION_PWR_SAV_EN" , .val=bdmf_address_general_configuration_pwr_sav_en },
            { .name="GENERAL_CONFIGURATION_PAD_HIGH" , .val=bdmf_address_general_configuration_pad_high },
            { .name="GENERAL_CONFIGURATION_PAD_LOW" , .val=bdmf_address_general_configuration_pad_low },
            { .name="GENERAL_CONFIGURATION_MULT_HIT_ERR" , .val=bdmf_address_general_configuration_mult_hit_err },
            { .name="GENERAL_CONFIGURATION_UNDO_FIX" , .val=bdmf_address_general_configuration_undo_fix },
            { .name="PM_COUNTERS_HITS" , .val=bdmf_address_pm_counters_hits },
            { .name="PM_COUNTERS_SRCHS" , .val=bdmf_address_pm_counters_srchs },
            { .name="PM_COUNTERS_MISS" , .val=bdmf_address_pm_counters_miss },
            { .name="PM_COUNTERS_HIT_1ST_ACS" , .val=bdmf_address_pm_counters_hit_1st_acs },
            { .name="PM_COUNTERS_HIT_2ND_ACS" , .val=bdmf_address_pm_counters_hit_2nd_acs },
            { .name="PM_COUNTERS_HIT_3RD_ACS" , .val=bdmf_address_pm_counters_hit_3rd_acs },
            { .name="PM_COUNTERS_HIT_4TH_ACS" , .val=bdmf_address_pm_counters_hit_4th_acs },
            { .name="PM_COUNTERS_FRZ_CNT" , .val=bdmf_address_pm_counters_frz_cnt },
            { .name="LKUP_TBL_CFG_TBL_CFG" , .val=bdmf_address_lkup_tbl_cfg_tbl_cfg },
            { .name="LKUP_TBL_CFG_KEY_MASK_HIGH" , .val=bdmf_address_lkup_tbl_cfg_key_mask_high },
            { .name="LKUP_TBL_CFG_KEY_MASK_LOW" , .val=bdmf_address_lkup_tbl_cfg_key_mask_low },
            { .name="LKUP_TBL_CFG_CNTXT_CFG" , .val=bdmf_address_lkup_tbl_cfg_cntxt_cfg },
            { .name="CAM_CONFIGURATION_CNTXT_CFG" , .val=bdmf_address_cam_configuration_cntxt_cfg },
            { .name="CAM_INDIRECT_OP" , .val=bdmf_address_cam_indirect_op },
            { .name="CAM_INDIRECT_OP_DONE" , .val=bdmf_address_cam_indirect_op_done },
            { .name="CAM_INDIRECT_ADDR" , .val=bdmf_address_cam_indirect_addr },
            { .name="CAM_INDIRECT_VLID_IN" , .val=bdmf_address_cam_indirect_vlid_in },
            { .name="CAM_INDIRECT_VLID_OUT" , .val=bdmf_address_cam_indirect_vlid_out },
            { .name="CAM_INDIRECT_RSLT" , .val=bdmf_address_cam_indirect_rslt },
            { .name="CAM_INDIRECT_KEY_IN" , .val=bdmf_address_cam_indirect_key_in },
            { .name="CAM_INDIRECT_KEY_OUT" , .val=bdmf_address_cam_indirect_key_out },
            { .name="CAM_BIST_BIST_STATUS" , .val=bdmf_address_cam_bist_bist_status },
            { .name="CAM_BIST_BIST_DBG_COMPARE_EN" , .val=bdmf_address_cam_bist_bist_dbg_compare_en },
            { .name="CAM_BIST_BIST_DBG_DATA" , .val=bdmf_address_cam_bist_bist_dbg_data },
            { .name="CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL" , .val=bdmf_address_cam_bist_bist_dbg_data_slice_or_status_sel },
            { .name="CAM_BIST_BIST_DBG_DATA_VALID" , .val=bdmf_address_cam_bist_bist_dbg_data_valid },
            { .name="CAM_BIST_BIST_EN" , .val=bdmf_address_cam_bist_bist_en },
            { .name="CAM_BIST_BIST_MODE" , .val=bdmf_address_cam_bist_bist_mode },
            { .name="CAM_BIST_BIST_RST_L" , .val=bdmf_address_cam_bist_bist_rst_l },
            { .name="CAM_BIST_BIST_SKIP_ERROR_CNT" , .val=bdmf_address_cam_bist_bist_skip_error_cnt },
            { .name="CAM_BIST_DBG_EN" , .val=bdmf_address_cam_bist_dbg_en },
            { .name="CAM_BIST_BIST_CASCADE_SELECT" , .val=bdmf_address_cam_bist_bist_cascade_select },
            { .name="CAM_BIST_BIST_BLOCK_SELECT" , .val=bdmf_address_cam_bist_bist_block_select },
            { .name="CAM_BIST_BIST_REPAIR_ENABLE" , .val=bdmf_address_cam_bist_bist_repair_enable },
            { .name="INTR_CTRL_ISR" , .val=bdmf_address_intr_ctrl_isr },
            { .name="INTR_CTRL_ISM" , .val=bdmf_address_intr_ctrl_ism },
            { .name="INTR_CTRL_IER" , .val=bdmf_address_intr_ctrl_ier },
            { .name="INTR_CTRL_ITR" , .val=bdmf_address_intr_ctrl_itr },
            { .name="DEBUG_DBG0" , .val=bdmf_address_debug_dbg0 },
            { .name="DEBUG_DBG1" , .val=bdmf_address_debug_dbg1 },
            { .name="DEBUG_DBG2" , .val=bdmf_address_debug_dbg2 },
            { .name="DEBUG_DBG3" , .val=bdmf_address_debug_dbg3 },
            { .name="DEBUG_DBG4" , .val=bdmf_address_debug_dbg4 },
            { .name="DEBUG_DBG5" , .val=bdmf_address_debug_dbg5 },
            { .name="DEBUG_DBG6" , .val=bdmf_address_debug_dbg6 },
            { .name="DEBUG_DBG7" , .val=bdmf_address_debug_dbg7 },
            { .name="DEBUG_DBG8" , .val=bdmf_address_debug_dbg8 },
            { .name="DEBUG_DBG9" , .val=bdmf_address_debug_dbg9 },
            { .name="DEBUG_DBG10" , .val=bdmf_address_debug_dbg10 },
            { .name="DEBUG_DBG11" , .val=bdmf_address_debug_dbg11 },
            { .name="DEBUG_DBG12" , .val=bdmf_address_debug_dbg12 },
            { .name="DEBUG_DBG13" , .val=bdmf_address_debug_dbg13 },
            { .name="DEBUG_DBG14" , .val=bdmf_address_debug_dbg14 },
            { .name="DEBUG_DBG15" , .val=bdmf_address_debug_dbg15 },
            { .name="DEBUG_DBG16" , .val=bdmf_address_debug_dbg16 },
            { .name="DEBUG_DBG17" , .val=bdmf_address_debug_dbg17 },
            { .name="DEBUG_DBG18" , .val=bdmf_address_debug_dbg18 },
            { .name="DEBUG_DBG19" , .val=bdmf_address_debug_dbg19 },
            { .name="DEBUG_DBG20" , .val=bdmf_address_debug_dbg20 },
            { .name="DEBUG_DBG_SEL" , .val=bdmf_address_debug_dbg_sel },
            { .name="AGING_RAM_AGING" , .val=bdmf_address_aging_ram_aging },
            { .name="CONTEXT_RAM_CONTEXT_47_24" , .val=bdmf_address_context_ram_context_47_24 },
            { .name="CONTEXT_RAM_CONTEXT_23_0" , .val=bdmf_address_context_ram_context_23_0 },
            { .name="RAM_ENG_HIGH" , .val=bdmf_address_ram_eng_high },
            { .name="RAM_ENG_LOW" , .val=bdmf_address_ram_eng_low },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_hash_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

