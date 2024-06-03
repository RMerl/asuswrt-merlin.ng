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
#include "xrdp_drv_bufmng_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ordr_set(uint16_t ctr_idx, uint8_t nxtlvl)
{
    uint32_t reg_counters_cfg_stat_ordr = 0;

#ifdef VALIDATE_PARMS
    if ((ctr_idx >= 32) ||
       (nxtlvl >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_ordr = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_ORDR, NXTLVL, reg_counters_cfg_stat_ordr, nxtlvl);

    RU_REG_RAM_WRITE(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_ORDR, reg_counters_cfg_stat_ordr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ordr_get(uint16_t ctr_idx, uint8_t *nxtlvl)
{
    uint32_t reg_counters_cfg_stat_ordr;

#ifdef VALIDATE_PARMS
    if (!nxtlvl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctr_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_ORDR, reg_counters_cfg_stat_ordr);

    *nxtlvl = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_ORDR, NXTLVL, reg_counters_cfg_stat_ordr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(uint16_t ctr_idx, uint32_t thr)
{
    uint32_t reg_counters_cfg_stat_rsrv_thr = 0;

#ifdef VALIDATE_PARMS
    if ((ctr_idx >= 32) ||
       (thr >= _19BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_rsrv_thr = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_RSRV_THR, THR, reg_counters_cfg_stat_rsrv_thr, thr);

    RU_REG_RAM_WRITE(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_RSRV_THR, reg_counters_cfg_stat_rsrv_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get(uint16_t ctr_idx, uint32_t *thr)
{
    uint32_t reg_counters_cfg_stat_rsrv_thr;

#ifdef VALIDATE_PARMS
    if (!thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctr_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_RSRV_THR, reg_counters_cfg_stat_rsrv_thr);

    *thr = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_RSRV_THR, THR, reg_counters_cfg_stat_rsrv_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(uint16_t ctr_idx, uint32_t thr)
{
    uint32_t reg_counters_cfg_stat_hipri_thr = 0;

#ifdef VALIDATE_PARMS
    if ((ctr_idx >= 32) ||
       (thr >= _19BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_hipri_thr = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_HIPRI_THR, THR, reg_counters_cfg_stat_hipri_thr, thr);

    RU_REG_RAM_WRITE(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_HIPRI_THR, reg_counters_cfg_stat_hipri_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hipri_thr_get(uint16_t ctr_idx, uint32_t *thr)
{
    uint32_t reg_counters_cfg_stat_hipri_thr;

#ifdef VALIDATE_PARMS
    if (!thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctr_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_HIPRI_THR, reg_counters_cfg_stat_hipri_thr);

    *thr = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_HIPRI_THR, THR, reg_counters_cfg_stat_hipri_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_max_thr_set(uint16_t ctr_idx, uint32_t thr)
{
    uint32_t reg_counters_cfg_stat_max_thr = 0;

#ifdef VALIDATE_PARMS
    if ((ctr_idx >= 32) ||
       (thr >= _19BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_max_thr = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_MAX_THR, THR, reg_counters_cfg_stat_max_thr, thr);

    RU_REG_RAM_WRITE(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_MAX_THR, reg_counters_cfg_stat_max_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_max_thr_get(uint16_t ctr_idx, uint32_t *thr)
{
    uint32_t reg_counters_cfg_stat_max_thr;

#ifdef VALIDATE_PARMS
    if (!thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctr_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_MAX_THR, reg_counters_cfg_stat_max_thr);

    *thr = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_MAX_THR, THR, reg_counters_cfg_stat_max_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set(uint8_t ctr0, uint8_t ctr1, uint8_t ctr2)
{
    uint32_t reg_counters_cfg_stat_hi_wmrk_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((ctr0 >= _5BITS_MAX_VAL_) ||
       (ctr1 >= _5BITS_MAX_VAL_) ||
       (ctr2 >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_hi_wmrk_cfg = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR0, reg_counters_cfg_stat_hi_wmrk_cfg, ctr0);
    reg_counters_cfg_stat_hi_wmrk_cfg = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR1, reg_counters_cfg_stat_hi_wmrk_cfg, ctr1);
    reg_counters_cfg_stat_hi_wmrk_cfg = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR2, reg_counters_cfg_stat_hi_wmrk_cfg, ctr2);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, reg_counters_cfg_stat_hi_wmrk_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_get(uint8_t *ctr0, uint8_t *ctr1, uint8_t *ctr2)
{
    uint32_t reg_counters_cfg_stat_hi_wmrk_cfg;

#ifdef VALIDATE_PARMS
    if (!ctr0 || !ctr1 || !ctr2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, reg_counters_cfg_stat_hi_wmrk_cfg);

    *ctr0 = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR0, reg_counters_cfg_stat_hi_wmrk_cfg);
    *ctr1 = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR1, reg_counters_cfg_stat_hi_wmrk_cfg);
    *ctr2 = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG, CTR2, reg_counters_cfg_stat_hi_wmrk_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_val_get(uint16_t hi_wmrk_idx, uint32_t *val)
{
    uint32_t reg_counters_cfg_stat_hi_wmrk_val;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((hi_wmrk_idx >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, hi_wmrk_idx, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_VAL, reg_counters_cfg_stat_hi_wmrk_val);

    *val = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_VAL, VAL, reg_counters_cfg_stat_hi_wmrk_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_init_set(uint8_t idx, uint32_t val)
{
    uint32_t reg_counters_cfg_stat_cntr_init = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= _5BITS_MAX_VAL_) ||
       (val >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_cntr_init = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_INIT, IDX, reg_counters_cfg_stat_cntr_init, idx);
    reg_counters_cfg_stat_cntr_init = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_INIT, VAL, reg_counters_cfg_stat_cntr_init, val);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_INIT, reg_counters_cfg_stat_cntr_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get(uint32_t *val)
{
    uint32_t reg_counters_cfg_stat_cntr_neg_st;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST, reg_counters_cfg_stat_cntr_neg_st);

    *val = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST, VAL, reg_counters_cfg_stat_cntr_neg_st);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_clr_set(uint32_t val)
{
    uint32_t reg_counters_cfg_stat_cntr_neg_st_clr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_counters_cfg_stat_cntr_neg_st_clr = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR, VAL, reg_counters_cfg_stat_cntr_neg_st_clr, val);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR, reg_counters_cfg_stat_cntr_neg_st_clr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_capt_cfg_set(bdmf_boolean mod)
{
    uint32_t reg_counters_cfg_stat_capt_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((mod >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_capt_cfg = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_CAPT_CFG, MOD, reg_counters_cfg_stat_capt_cfg, mod);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_CAPT_CFG, reg_counters_cfg_stat_capt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_capt_cfg_get(bdmf_boolean *mod)
{
    uint32_t reg_counters_cfg_stat_capt_cfg;

#ifdef VALIDATE_PARMS
    if (!mod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_CAPT_CFG, reg_counters_cfg_stat_capt_cfg);

    *mod = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CAPT_CFG, MOD, reg_counters_cfg_stat_capt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get(uint16_t neg_cap_cnt, uint8_t *idx, uint32_t *val)
{
    uint32_t reg_counters_cfg_stat_cnt_neg_cap_cnt;

#ifdef VALIDATE_PARMS
    if (!idx || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((neg_cap_cnt >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, neg_cap_cnt, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT, reg_counters_cfg_stat_cnt_neg_cap_cnt);

    *idx = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT, IDX, reg_counters_cfg_stat_cnt_neg_cap_cnt);
    *val = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT, VAL, reg_counters_cfg_stat_cnt_neg_cap_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get(uint8_t *idx, uint32_t *cmd)
{
    uint32_t reg_counters_cfg_stat_cnt_neg_cap_cmd;

#ifdef VALIDATE_PARMS
    if (!idx || !cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD, reg_counters_cfg_stat_cnt_neg_cap_cmd);

    *idx = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD, IDX, reg_counters_cfg_stat_cnt_neg_cap_cmd);
    *cmd = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD, CMD, reg_counters_cfg_stat_cnt_neg_cap_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_pools_size_set(uint8_t pool0_size, uint8_t pool1_size, uint8_t pool2_size, uint8_t pool3_size)
{
    uint32_t reg_counters_cfg_stat_pools_size = 0;

#ifdef VALIDATE_PARMS
    if ((pool0_size >= _6BITS_MAX_VAL_) ||
       (pool1_size >= _6BITS_MAX_VAL_) ||
       (pool2_size >= _6BITS_MAX_VAL_) ||
       (pool3_size >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_pools_size = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL0_SIZE, reg_counters_cfg_stat_pools_size, pool0_size);
    reg_counters_cfg_stat_pools_size = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL1_SIZE, reg_counters_cfg_stat_pools_size, pool1_size);
    reg_counters_cfg_stat_pools_size = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL2_SIZE, reg_counters_cfg_stat_pools_size, pool2_size);
    reg_counters_cfg_stat_pools_size = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL3_SIZE, reg_counters_cfg_stat_pools_size, pool3_size);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, reg_counters_cfg_stat_pools_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_pools_size_get(uint8_t *pool0_size, uint8_t *pool1_size, uint8_t *pool2_size, uint8_t *pool3_size)
{
    uint32_t reg_counters_cfg_stat_pools_size;

#ifdef VALIDATE_PARMS
    if (!pool0_size || !pool1_size || !pool2_size || !pool3_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, reg_counters_cfg_stat_pools_size);

    *pool0_size = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL0_SIZE, reg_counters_cfg_stat_pools_size);
    *pool1_size = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL1_SIZE, reg_counters_cfg_stat_pools_size);
    *pool2_size = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL2_SIZE, reg_counters_cfg_stat_pools_size);
    *pool3_size = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE, POOL3_SIZE, reg_counters_cfg_stat_pools_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_misc_set(bdmf_boolean neg_en, uint8_t fc_hyst)
{
    uint32_t reg_counters_cfg_stat_misc = 0;

#ifdef VALIDATE_PARMS
    if ((neg_en >= _1BITS_MAX_VAL_) ||
       (fc_hyst >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_cfg_stat_misc = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_MISC, NEG_EN, reg_counters_cfg_stat_misc, neg_en);
    reg_counters_cfg_stat_misc = RU_FIELD_SET(0, BUFMNG, COUNTERS_CFG_STAT_MISC, FC_HYST, reg_counters_cfg_stat_misc, fc_hyst);

    RU_REG_WRITE(0, BUFMNG, COUNTERS_CFG_STAT_MISC, reg_counters_cfg_stat_misc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_misc_get(bdmf_boolean *neg_en, uint8_t *fc_hyst)
{
    uint32_t reg_counters_cfg_stat_misc;

#ifdef VALIDATE_PARMS
    if (!neg_en || !fc_hyst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_MISC, reg_counters_cfg_stat_misc);

    *neg_en = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_MISC, NEG_EN, reg_counters_cfg_stat_misc);
    *fc_hyst = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_MISC, FC_HYST, reg_counters_cfg_stat_misc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_fc_st_vec_get(uint32_t *val)
{
    uint32_t reg_counters_cfg_stat_fc_st_vec;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, BUFMNG, COUNTERS_CFG_STAT_FC_ST_VEC, reg_counters_cfg_stat_fc_st_vec);

    *val = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_FC_ST_VEC, VAL, reg_counters_cfg_stat_fc_st_vec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ctrs_val_get(uint16_t ctr_idx, uint32_t *val)
{
    uint32_t reg_counters_cfg_stat_ctrs_val;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ctr_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctr_idx, BUFMNG, COUNTERS_CFG_STAT_CTRS_VAL, reg_counters_cfg_stat_ctrs_val);

    *val = RU_FIELD_GET(0, BUFMNG, COUNTERS_CFG_STAT_CTRS_VAL, VAL, reg_counters_cfg_stat_ctrs_val);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_counters_cfg_stat_ordr,
    bdmf_address_counters_cfg_stat_rsrv_thr,
    bdmf_address_counters_cfg_stat_hipri_thr,
    bdmf_address_counters_cfg_stat_max_thr,
    bdmf_address_counters_cfg_stat_hi_wmrk_cfg,
    bdmf_address_counters_cfg_stat_hi_wmrk_val,
    bdmf_address_counters_cfg_stat_cntr_init,
    bdmf_address_counters_cfg_stat_cntr_neg_st,
    bdmf_address_counters_cfg_stat_cntr_neg_st_clr,
    bdmf_address_counters_cfg_stat_capt_cfg,
    bdmf_address_counters_cfg_stat_cnt_neg_cap_cnt,
    bdmf_address_counters_cfg_stat_cnt_neg_cap_cmd,
    bdmf_address_counters_cfg_stat_pools_size,
    bdmf_address_counters_cfg_stat_misc,
    bdmf_address_counters_cfg_stat_fc_st_vec,
    bdmf_address_counters_cfg_stat_ctrs_val,
}
bdmf_address;

static int ag_drv_bufmng_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_bufmng_counters_cfg_stat_ordr:
        ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_rsrv_thr:
        ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_hipri_thr:
        ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_max_thr:
        ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_hi_wmrk_cfg:
        ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_cntr_init:
        ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_init_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_cntr_neg_st_clr:
        ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_clr_set(parm[1].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_capt_cfg:
        ag_err = ag_drv_bufmng_counters_cfg_stat_capt_cfg_set(parm[1].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_pools_size:
        ag_err = ag_drv_bufmng_counters_cfg_stat_pools_size_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bufmng_counters_cfg_stat_misc:
        ag_err = ag_drv_bufmng_counters_cfg_stat_misc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_bufmng_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_bufmng_counters_cfg_stat_ordr:
    {
        uint8_t nxtlvl;
        ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_get(parm[1].value.unumber, &nxtlvl);
        bdmf_session_print(session, "nxtlvl = %u = 0x%x\n", nxtlvl, nxtlvl);
        break;
    }
    case cli_bufmng_counters_cfg_stat_rsrv_thr:
    {
        uint32_t thr;
        ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get(parm[1].value.unumber, &thr);
        bdmf_session_print(session, "thr = %u = 0x%x\n", thr, thr);
        break;
    }
    case cli_bufmng_counters_cfg_stat_hipri_thr:
    {
        uint32_t thr;
        ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_get(parm[1].value.unumber, &thr);
        bdmf_session_print(session, "thr = %u = 0x%x\n", thr, thr);
        break;
    }
    case cli_bufmng_counters_cfg_stat_max_thr:
    {
        uint32_t thr;
        ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_get(parm[1].value.unumber, &thr);
        bdmf_session_print(session, "thr = %u = 0x%x\n", thr, thr);
        break;
    }
    case cli_bufmng_counters_cfg_stat_hi_wmrk_cfg:
    {
        uint8_t ctr0;
        uint8_t ctr1;
        uint8_t ctr2;
        ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_get(&ctr0, &ctr1, &ctr2);
        bdmf_session_print(session, "ctr0 = %u = 0x%x\n", ctr0, ctr0);
        bdmf_session_print(session, "ctr1 = %u = 0x%x\n", ctr1, ctr1);
        bdmf_session_print(session, "ctr2 = %u = 0x%x\n", ctr2, ctr2);
        break;
    }
    case cli_bufmng_counters_cfg_stat_hi_wmrk_val:
    {
        uint32_t val;
        ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_val_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_bufmng_counters_cfg_stat_cntr_neg_st:
    {
        uint32_t val;
        ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_bufmng_counters_cfg_stat_capt_cfg:
    {
        bdmf_boolean mod;
        ag_err = ag_drv_bufmng_counters_cfg_stat_capt_cfg_get(&mod);
        bdmf_session_print(session, "mod = %u = 0x%x\n", mod, mod);
        break;
    }
    case cli_bufmng_counters_cfg_stat_cnt_neg_cap_cnt:
    {
        uint8_t idx;
        uint32_t val;
        ag_err = ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get(parm[1].value.unumber, &idx, &val);
        bdmf_session_print(session, "idx = %u = 0x%x\n", idx, idx);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_bufmng_counters_cfg_stat_cnt_neg_cap_cmd:
    {
        uint8_t idx;
        uint32_t cmd;
        ag_err = ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get(&idx, &cmd);
        bdmf_session_print(session, "idx = %u = 0x%x\n", idx, idx);
        bdmf_session_print(session, "cmd = %u = 0x%x\n", cmd, cmd);
        break;
    }
    case cli_bufmng_counters_cfg_stat_pools_size:
    {
        uint8_t pool0_size;
        uint8_t pool1_size;
        uint8_t pool2_size;
        uint8_t pool3_size;
        ag_err = ag_drv_bufmng_counters_cfg_stat_pools_size_get(&pool0_size, &pool1_size, &pool2_size, &pool3_size);
        bdmf_session_print(session, "pool0_size = %u = 0x%x\n", pool0_size, pool0_size);
        bdmf_session_print(session, "pool1_size = %u = 0x%x\n", pool1_size, pool1_size);
        bdmf_session_print(session, "pool2_size = %u = 0x%x\n", pool2_size, pool2_size);
        bdmf_session_print(session, "pool3_size = %u = 0x%x\n", pool3_size, pool3_size);
        break;
    }
    case cli_bufmng_counters_cfg_stat_misc:
    {
        bdmf_boolean neg_en;
        uint8_t fc_hyst;
        ag_err = ag_drv_bufmng_counters_cfg_stat_misc_get(&neg_en, &fc_hyst);
        bdmf_session_print(session, "neg_en = %u = 0x%x\n", neg_en, neg_en);
        bdmf_session_print(session, "fc_hyst = %u = 0x%x\n", fc_hyst, fc_hyst);
        break;
    }
    case cli_bufmng_counters_cfg_stat_fc_st_vec:
    {
        uint32_t val;
        ag_err = ag_drv_bufmng_counters_cfg_stat_fc_st_vec_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_bufmng_counters_cfg_stat_ctrs_val:
    {
        uint32_t val;
        ag_err = ag_drv_bufmng_counters_cfg_stat_ctrs_val_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_bufmng_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint16_t ctr_idx = gtmv(m, 5);
        uint8_t nxtlvl = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ordr_set( %u %u)\n", ctr_idx,
            nxtlvl);
        ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_set(ctr_idx, nxtlvl);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_get(ctr_idx, &nxtlvl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ordr_get( %u %u)\n", ctr_idx,
                nxtlvl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (nxtlvl != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set( %u %u)\n", ctr_idx,
            thr);
        ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(ctr_idx, thr);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get(ctr_idx, &thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get( %u %u)\n", ctr_idx,
                thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thr != gtmv(m, 19))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hipri_thr_set( %u %u)\n", ctr_idx,
            thr);
        ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(ctr_idx, thr);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_get(ctr_idx, &thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hipri_thr_get( %u %u)\n", ctr_idx,
                thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thr != gtmv(m, 19))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_max_thr_set( %u %u)\n", ctr_idx,
            thr);
        ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_set(ctr_idx, thr);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_get(ctr_idx, &thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_max_thr_get( %u %u)\n", ctr_idx,
                thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thr != gtmv(m, 19))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ctr0 = gtmv(m, 5);
        uint8_t ctr1 = gtmv(m, 5);
        uint8_t ctr2 = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set( %u %u %u)\n",
            ctr0, ctr1, ctr2);
        ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set(ctr0, ctr1, ctr2);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_get(&ctr0, &ctr1, &ctr2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_get( %u %u %u)\n",
                ctr0, ctr1, ctr2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ctr0 != gtmv(m, 5) || ctr1 != gtmv(m, 5) || ctr2 != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t hi_wmrk_idx = gtmv(m, 1);
        uint32_t val = gtmv(m, 20);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_hi_wmrk_val_get(hi_wmrk_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hi_wmrk_val_get( %u %u)\n", hi_wmrk_idx,
                val);
        }
    }

    {
        uint8_t idx = gtmv(m, 5);
        uint32_t val = gtmv(m, 20);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_cntr_init_set( %u %u)\n",
            idx, val);
        ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_init_set(idx, val);
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get( %u)\n",
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_clr_set( %u)\n",
            val);
        ag_err = ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_clr_set(val);
    }

    {
        bdmf_boolean mod = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_capt_cfg_set( %u)\n",
            mod);
        ag_err = ag_drv_bufmng_counters_cfg_stat_capt_cfg_set(mod);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_capt_cfg_get(&mod);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_capt_cfg_get( %u)\n",
                mod);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mod != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t neg_cap_cnt = gtmv(m, 1);
        uint8_t idx = gtmv(m, 5);
        uint32_t val = gtmv(m, 20);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get(neg_cap_cnt, &idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get( %u %u %u)\n", neg_cap_cnt,
                idx, val);
        }
    }

    {
        uint8_t idx = gtmv(m, 8);
        uint32_t cmd = gtmv(m, 24);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get(&idx, &cmd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get( %u %u)\n",
                idx, cmd);
        }
    }

    {
        uint8_t pool0_size = gtmv(m, 6);
        uint8_t pool1_size = gtmv(m, 6);
        uint8_t pool2_size = gtmv(m, 6);
        uint8_t pool3_size = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_pools_size_set( %u %u %u %u)\n",
            pool0_size, pool1_size, pool2_size, pool3_size);
        ag_err = ag_drv_bufmng_counters_cfg_stat_pools_size_set(pool0_size, pool1_size, pool2_size, pool3_size);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_pools_size_get(&pool0_size, &pool1_size, &pool2_size, &pool3_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_pools_size_get( %u %u %u %u)\n",
                pool0_size, pool1_size, pool2_size, pool3_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool0_size != gtmv(m, 6) || pool1_size != gtmv(m, 6) || pool2_size != gtmv(m, 6) || pool3_size != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean neg_en = gtmv(m, 1);
        uint8_t fc_hyst = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_misc_set( %u %u)\n",
            neg_en, fc_hyst);
        ag_err = ag_drv_bufmng_counters_cfg_stat_misc_set(neg_en, fc_hyst);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_misc_get(&neg_en, &fc_hyst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_misc_get( %u %u)\n",
                neg_en, fc_hyst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (neg_en != gtmv(m, 1) || fc_hyst != gtmv(m, 4))
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
            ag_err = ag_drv_bufmng_counters_cfg_stat_fc_st_vec_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_fc_st_vec_get( %u)\n",
                val);
        }
    }

    {
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t val = gtmv(m, 20);
        if (!ag_err)
            ag_err = ag_drv_bufmng_counters_cfg_stat_ctrs_val_get(ctr_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ctrs_val_get( %u %u)\n", ctr_idx,
                val);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_bufmng_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case cli_bufmng_counters_cfg_stat_ordr:
    {
        uint16_t max_ctr_idx = 32;
        uint16_t ctr_idx = gtmv(m, 5);
        uint8_t nxtlvl = gtmv(m, 6);

        if ((start_idx >= max_ctr_idx) || (stop_idx >= max_ctr_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ctr_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ordr_set( %u %u)\n", ctr_idx,
                nxtlvl);
            ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_set(ctr_idx, nxtlvl);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        nxtlvl = gtmv(m, 6);

        for (ctr_idx = start_idx; ctr_idx <= stop_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ordr_set( %u %u)\n", ctr_idx,
                nxtlvl);
            ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_set(ctr_idx, nxtlvl);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            if (ctr_idx < start_idx || ctr_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bufmng_counters_cfg_stat_ordr_get(ctr_idx, &nxtlvl);

            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_ordr_get( %u %u)\n", ctr_idx,
                nxtlvl);

            if (nxtlvl != gtmv(m, 6) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ctr_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of counters_cfg_stat_ordr completed. Number of tested entries %u.\n", max_ctr_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bufmng_counters_cfg_stat_rsrv_thr:
    {
        uint16_t max_ctr_idx = 32;
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);

        if ((start_idx >= max_ctr_idx) || (stop_idx >= max_ctr_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ctr_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        thr = gtmv(m, 19);

        for (ctr_idx = start_idx; ctr_idx <= stop_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            if (ctr_idx < start_idx || ctr_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get(ctr_idx, &thr);

            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get( %u %u)\n", ctr_idx,
                thr);

            if (thr != gtmv(m, 19) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ctr_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of counters_cfg_stat_rsrv_thr completed. Number of tested entries %u.\n", max_ctr_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bufmng_counters_cfg_stat_hipri_thr:
    {
        uint16_t max_ctr_idx = 32;
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);

        if ((start_idx >= max_ctr_idx) || (stop_idx >= max_ctr_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ctr_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hipri_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        thr = gtmv(m, 19);

        for (ctr_idx = start_idx; ctr_idx <= stop_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hipri_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            if (ctr_idx < start_idx || ctr_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bufmng_counters_cfg_stat_hipri_thr_get(ctr_idx, &thr);

            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_hipri_thr_get( %u %u)\n", ctr_idx,
                thr);

            if (thr != gtmv(m, 19) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ctr_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of counters_cfg_stat_hipri_thr completed. Number of tested entries %u.\n", max_ctr_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bufmng_counters_cfg_stat_max_thr:
    {
        uint16_t max_ctr_idx = 32;
        uint16_t ctr_idx = gtmv(m, 5);
        uint32_t thr = gtmv(m, 19);

        if ((start_idx >= max_ctr_idx) || (stop_idx >= max_ctr_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ctr_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_max_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        thr = gtmv(m, 19);

        for (ctr_idx = start_idx; ctr_idx <= stop_idx; ctr_idx++)
        {
            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_max_thr_set( %u %u)\n", ctr_idx,
                thr);
            ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_set(ctr_idx, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ctr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ctr_idx = 0; ctr_idx < max_ctr_idx; ctr_idx++)
        {
            if (ctr_idx < start_idx || ctr_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bufmng_counters_cfg_stat_max_thr_get(ctr_idx, &thr);

            bdmf_session_print(session, "ag_drv_bufmng_counters_cfg_stat_max_thr_get( %u %u)\n", ctr_idx,
                thr);

            if (thr != gtmv(m, 19) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ctr_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of counters_cfg_stat_max_thr completed. Number of tested entries %u.\n", max_ctr_idx);
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
static int ag_drv_bufmng_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_counters_cfg_stat_ordr: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_ORDR); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_rsrv_thr: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_RSRV_THR); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_hipri_thr: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_HIPRI_THR); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_max_thr: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_MAX_THR); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_hi_wmrk_cfg: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_CFG); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_hi_wmrk_val: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_HI_WMRK_VAL); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_cntr_init: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CNTR_INIT); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_cntr_neg_st: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_cntr_neg_st_clr: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_capt_cfg: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CAPT_CFG); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_cnt_neg_cap_cnt: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_cnt_neg_cap_cmd: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_pools_size: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_POOLS_SIZE); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_misc: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_MISC); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_fc_st_vec: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_FC_ST_VEC); blk = &RU_BLK(BUFMNG); break;
    case bdmf_address_counters_cfg_stat_ctrs_val: reg = &RU_REG(BUFMNG, COUNTERS_CFG_STAT_CTRS_VAL); blk = &RU_BLK(BUFMNG); break;
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

bdmfmon_handle_t ag_drv_bufmng_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "bufmng", "bufmng", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_ordr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("nxtlvl", "nxtlvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_rsrv_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thr", "thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_hipri_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thr", "thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_max_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thr", "thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_hi_wmrk_cfg[] = {
            BDMFMON_MAKE_PARM("ctr0", "ctr0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ctr1", "ctr1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ctr2", "ctr2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_cntr_init[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_cntr_neg_st_clr[] = {
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_capt_cfg[] = {
            BDMFMON_MAKE_PARM("mod", "mod", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_pools_size[] = {
            BDMFMON_MAKE_PARM("pool0_size", "pool0_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool1_size", "pool1_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool2_size", "pool2_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool3_size", "pool3_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_cfg_stat_misc[] = {
            BDMFMON_MAKE_PARM("neg_en", "neg_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fc_hyst", "fc_hyst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counters_cfg_stat_ordr", .val = cli_bufmng_counters_cfg_stat_ordr, .parms = set_counters_cfg_stat_ordr },
            { .name = "counters_cfg_stat_rsrv_thr", .val = cli_bufmng_counters_cfg_stat_rsrv_thr, .parms = set_counters_cfg_stat_rsrv_thr },
            { .name = "counters_cfg_stat_hipri_thr", .val = cli_bufmng_counters_cfg_stat_hipri_thr, .parms = set_counters_cfg_stat_hipri_thr },
            { .name = "counters_cfg_stat_max_thr", .val = cli_bufmng_counters_cfg_stat_max_thr, .parms = set_counters_cfg_stat_max_thr },
            { .name = "counters_cfg_stat_hi_wmrk_cfg", .val = cli_bufmng_counters_cfg_stat_hi_wmrk_cfg, .parms = set_counters_cfg_stat_hi_wmrk_cfg },
            { .name = "counters_cfg_stat_cntr_init", .val = cli_bufmng_counters_cfg_stat_cntr_init, .parms = set_counters_cfg_stat_cntr_init },
            { .name = "counters_cfg_stat_cntr_neg_st_clr", .val = cli_bufmng_counters_cfg_stat_cntr_neg_st_clr, .parms = set_counters_cfg_stat_cntr_neg_st_clr },
            { .name = "counters_cfg_stat_capt_cfg", .val = cli_bufmng_counters_cfg_stat_capt_cfg, .parms = set_counters_cfg_stat_capt_cfg },
            { .name = "counters_cfg_stat_pools_size", .val = cli_bufmng_counters_cfg_stat_pools_size, .parms = set_counters_cfg_stat_pools_size },
            { .name = "counters_cfg_stat_misc", .val = cli_bufmng_counters_cfg_stat_misc, .parms = set_counters_cfg_stat_misc },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_bufmng_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_ordr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_rsrv_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_hipri_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_max_thr[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_hi_wmrk_val[] = {
            BDMFMON_MAKE_PARM("hi_wmrk_idx", "hi_wmrk_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_cnt_neg_cap_cnt[] = {
            BDMFMON_MAKE_PARM("neg_cap_cnt", "neg_cap_cnt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_cfg_stat_ctrs_val[] = {
            BDMFMON_MAKE_PARM("ctr_idx", "ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counters_cfg_stat_ordr", .val = cli_bufmng_counters_cfg_stat_ordr, .parms = get_counters_cfg_stat_ordr },
            { .name = "counters_cfg_stat_rsrv_thr", .val = cli_bufmng_counters_cfg_stat_rsrv_thr, .parms = get_counters_cfg_stat_rsrv_thr },
            { .name = "counters_cfg_stat_hipri_thr", .val = cli_bufmng_counters_cfg_stat_hipri_thr, .parms = get_counters_cfg_stat_hipri_thr },
            { .name = "counters_cfg_stat_max_thr", .val = cli_bufmng_counters_cfg_stat_max_thr, .parms = get_counters_cfg_stat_max_thr },
            { .name = "counters_cfg_stat_hi_wmrk_cfg", .val = cli_bufmng_counters_cfg_stat_hi_wmrk_cfg, .parms = get_default },
            { .name = "counters_cfg_stat_hi_wmrk_val", .val = cli_bufmng_counters_cfg_stat_hi_wmrk_val, .parms = get_counters_cfg_stat_hi_wmrk_val },
            { .name = "counters_cfg_stat_cntr_neg_st", .val = cli_bufmng_counters_cfg_stat_cntr_neg_st, .parms = get_default },
            { .name = "counters_cfg_stat_capt_cfg", .val = cli_bufmng_counters_cfg_stat_capt_cfg, .parms = get_default },
            { .name = "counters_cfg_stat_cnt_neg_cap_cnt", .val = cli_bufmng_counters_cfg_stat_cnt_neg_cap_cnt, .parms = get_counters_cfg_stat_cnt_neg_cap_cnt },
            { .name = "counters_cfg_stat_cnt_neg_cap_cmd", .val = cli_bufmng_counters_cfg_stat_cnt_neg_cap_cmd, .parms = get_default },
            { .name = "counters_cfg_stat_pools_size", .val = cli_bufmng_counters_cfg_stat_pools_size, .parms = get_default },
            { .name = "counters_cfg_stat_misc", .val = cli_bufmng_counters_cfg_stat_misc, .parms = get_default },
            { .name = "counters_cfg_stat_fc_st_vec", .val = cli_bufmng_counters_cfg_stat_fc_st_vec, .parms = get_default },
            { .name = "counters_cfg_stat_ctrs_val", .val = cli_bufmng_counters_cfg_stat_ctrs_val, .parms = get_counters_cfg_stat_ctrs_val },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_bufmng_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_bufmng_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counters_cfg_stat_ordr", .val = cli_bufmng_counters_cfg_stat_ordr, .parms = ext_test_default},
            { .name = "counters_cfg_stat_rsrv_thr", .val = cli_bufmng_counters_cfg_stat_rsrv_thr, .parms = ext_test_default},
            { .name = "counters_cfg_stat_hipri_thr", .val = cli_bufmng_counters_cfg_stat_hipri_thr, .parms = ext_test_default},
            { .name = "counters_cfg_stat_max_thr", .val = cli_bufmng_counters_cfg_stat_max_thr, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_bufmng_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "COUNTERS_CFG_STAT_ORDR", .val = bdmf_address_counters_cfg_stat_ordr },
            { .name = "COUNTERS_CFG_STAT_RSRV_THR", .val = bdmf_address_counters_cfg_stat_rsrv_thr },
            { .name = "COUNTERS_CFG_STAT_HIPRI_THR", .val = bdmf_address_counters_cfg_stat_hipri_thr },
            { .name = "COUNTERS_CFG_STAT_MAX_THR", .val = bdmf_address_counters_cfg_stat_max_thr },
            { .name = "COUNTERS_CFG_STAT_HI_WMRK_CFG", .val = bdmf_address_counters_cfg_stat_hi_wmrk_cfg },
            { .name = "COUNTERS_CFG_STAT_HI_WMRK_VAL", .val = bdmf_address_counters_cfg_stat_hi_wmrk_val },
            { .name = "COUNTERS_CFG_STAT_CNTR_INIT", .val = bdmf_address_counters_cfg_stat_cntr_init },
            { .name = "COUNTERS_CFG_STAT_CNTR_NEG_ST", .val = bdmf_address_counters_cfg_stat_cntr_neg_st },
            { .name = "COUNTERS_CFG_STAT_CNTR_NEG_ST_CLR", .val = bdmf_address_counters_cfg_stat_cntr_neg_st_clr },
            { .name = "COUNTERS_CFG_STAT_CAPT_CFG", .val = bdmf_address_counters_cfg_stat_capt_cfg },
            { .name = "COUNTERS_CFG_STAT_CNT_NEG_CAP_CNT", .val = bdmf_address_counters_cfg_stat_cnt_neg_cap_cnt },
            { .name = "COUNTERS_CFG_STAT_CNT_NEG_CAP_CMD", .val = bdmf_address_counters_cfg_stat_cnt_neg_cap_cmd },
            { .name = "COUNTERS_CFG_STAT_POOLS_SIZE", .val = bdmf_address_counters_cfg_stat_pools_size },
            { .name = "COUNTERS_CFG_STAT_MISC", .val = bdmf_address_counters_cfg_stat_misc },
            { .name = "COUNTERS_CFG_STAT_FC_ST_VEC", .val = bdmf_address_counters_cfg_stat_fc_st_vec },
            { .name = "COUNTERS_CFG_STAT_CTRS_VAL", .val = bdmf_address_counters_cfg_stat_ctrs_val },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_bufmng_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
