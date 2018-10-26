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
#include "xrdp_drv_dsptchr_ag.h"

bdmf_error_t ag_drv_dsptchr_cngs_params_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params)
{
    uint32_t reg_congestion_ingrs_congstn=0;

#ifdef VALIDATE_PARMS
    if(!cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((viq_idx >= 32) ||
       (cngs_params->frst_lvl >= _12BITS_MAX_VAL_) ||
       (cngs_params->scnd_lvl >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, FRST_LVL, reg_congestion_ingrs_congstn, cngs_params->frst_lvl);
    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, SCND_LVL, reg_congestion_ingrs_congstn, cngs_params->scnd_lvl);
    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, HYST_THRS, reg_congestion_ingrs_congstn, cngs_params->hyst_thrs);

    RU_REG_RAM_WRITE(0, viq_idx, DSPTCHR, CONGESTION_INGRS_CONGSTN, reg_congestion_ingrs_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cngs_params_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params)
{
    uint32_t reg_congestion_ingrs_congstn;

#ifdef VALIDATE_PARMS
    if(!cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((viq_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, viq_idx, DSPTCHR, CONGESTION_INGRS_CONGSTN, reg_congestion_ingrs_congstn);

    cngs_params->frst_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, FRST_LVL, reg_congestion_ingrs_congstn);
    cngs_params->scnd_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, SCND_LVL, reg_congestion_ingrs_congstn);
    cngs_params->hyst_thrs = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, HYST_THRS, reg_congestion_ingrs_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_set(const dsptchr_glbl_cngs_params *glbl_cngs_params)
{
    uint32_t reg_congestion_glbl_congstn=0;

#ifdef VALIDATE_PARMS
    if(!glbl_cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((glbl_cngs_params->frst_lvl >= _12BITS_MAX_VAL_) ||
       (glbl_cngs_params->scnd_lvl >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, FRST_LVL, reg_congestion_glbl_congstn, glbl_cngs_params->frst_lvl);
    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, SCND_LVL, reg_congestion_glbl_congstn, glbl_cngs_params->scnd_lvl);
    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, HYST_THRS, reg_congestion_glbl_congstn, glbl_cngs_params->hyst_thrs);

    RU_REG_WRITE(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, reg_congestion_glbl_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_get(dsptchr_glbl_cngs_params *glbl_cngs_params)
{
    uint32_t reg_congestion_glbl_congstn;

#ifdef VALIDATE_PARMS
    if(!glbl_cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, reg_congestion_glbl_congstn);

    glbl_cngs_params->frst_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, FRST_LVL, reg_congestion_glbl_congstn);
    glbl_cngs_params->scnd_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, SCND_LVL, reg_congestion_glbl_congstn);
    glbl_cngs_params->hyst_thrs = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, HYST_THRS, reg_congestion_glbl_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_q_size_params_set(uint8_t q_idx, uint16_t cmn_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_size=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32) ||
       (cmn_cnt >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ingrs_queues_q_ingrs_size = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_SIZE, CMN_CNT, reg_ingrs_queues_q_ingrs_size, cmn_cnt);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_SIZE, reg_ingrs_queues_q_ingrs_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_q_size_params_get(uint8_t q_idx, uint16_t *cmn_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_size;

#ifdef VALIDATE_PARMS
    if(!cmn_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_SIZE, reg_ingrs_queues_q_ingrs_size);

    *cmn_cnt = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_SIZE, CMN_CNT, reg_ingrs_queues_q_ingrs_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_credit_cnt_set(uint8_t q_idx, uint16_t credit_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_limits=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32) ||
       (credit_cnt >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    reg_ingrs_queues_q_ingrs_limits = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, CREDIT_CNT, reg_ingrs_queues_q_ingrs_limits, credit_cnt);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_credit_cnt_get(uint8_t q_idx, uint16_t *credit_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_limits;

#ifdef VALIDATE_PARMS
    if(!credit_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    *credit_cnt = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, CREDIT_CNT, reg_ingrs_queues_q_ingrs_limits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_q_limits_params_set(uint8_t q_idx, uint16_t cmn_max, uint16_t gurntd_max)
{
    uint32_t reg_ingrs_queues_q_ingrs_limits=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32) ||
       (cmn_max >= _10BITS_MAX_VAL_) ||
       (gurntd_max >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    reg_ingrs_queues_q_ingrs_limits = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, CMN_MAX, reg_ingrs_queues_q_ingrs_limits, cmn_max);
    reg_ingrs_queues_q_ingrs_limits = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, GURNTD_MAX, reg_ingrs_queues_q_ingrs_limits, gurntd_max);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_q_limits_params_get(uint8_t q_idx, uint16_t *cmn_max, uint16_t *gurntd_max)
{
    uint32_t reg_ingrs_queues_q_ingrs_limits;

#ifdef VALIDATE_PARMS
    if(!cmn_max || !gurntd_max)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, reg_ingrs_queues_q_ingrs_limits);

    *cmn_max = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, CMN_MAX, reg_ingrs_queues_q_ingrs_limits);
    *gurntd_max = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS, GURNTD_MAX, reg_ingrs_queues_q_ingrs_limits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_set(uint8_t q_idx, bdmf_boolean chrncy_en, uint16_t chrncy_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_cohrency=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32) ||
       (chrncy_en >= _1BITS_MAX_VAL_) ||
       (chrncy_cnt >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, reg_ingrs_queues_q_ingrs_cohrency);

    reg_ingrs_queues_q_ingrs_cohrency = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, CHRNCY_EN, reg_ingrs_queues_q_ingrs_cohrency, chrncy_en);
    reg_ingrs_queues_q_ingrs_cohrency = RU_FIELD_SET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, CHRNCY_CNT, reg_ingrs_queues_q_ingrs_cohrency, chrncy_cnt);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, reg_ingrs_queues_q_ingrs_cohrency);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_get(uint8_t q_idx, bdmf_boolean *chrncy_en, uint16_t *chrncy_cnt)
{
    uint32_t reg_ingrs_queues_q_ingrs_cohrency;

#ifdef VALIDATE_PARMS
    if(!chrncy_en || !chrncy_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, reg_ingrs_queues_q_ingrs_cohrency);

    *chrncy_en = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, CHRNCY_EN, reg_ingrs_queues_q_ingrs_cohrency);
    *chrncy_cnt = RU_FIELD_GET(0, DSPTCHR, INGRS_QUEUES_Q_INGRS_COHRENCY, CHRNCY_CNT, reg_ingrs_queues_q_ingrs_cohrency);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_pools_limits_set(const dsptchr_pools_limits *pools_limits)
{
    uint32_t reg_pool_sizes_cmn_pool_lmt=0;
    uint32_t reg_pool_sizes_grnted_pool_lmt=0;
    uint32_t reg_pool_sizes_multi_cst_pool_lmt=0;
    uint32_t reg_pool_sizes_rnr_pool_lmt=0;
    uint32_t reg_pool_sizes_cmn_pool_size=0;
    uint32_t reg_pool_sizes_grnted_pool_size=0;
    uint32_t reg_pool_sizes_multi_cst_pool_size=0;
    uint32_t reg_pool_sizes_rnr_pool_size=0;
    uint32_t reg_pool_sizes_prcssing_pool_size=0;

#ifdef VALIDATE_PARMS
    if(!pools_limits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pools_limits->cmn_pool_lmt >= _10BITS_MAX_VAL_) ||
       (pools_limits->grnted_pool_lmt >= _10BITS_MAX_VAL_) ||
       (pools_limits->mcast_pool_lmt >= _10BITS_MAX_VAL_) ||
       (pools_limits->rnr_pool_lmt >= _10BITS_MAX_VAL_) ||
       (pools_limits->cmn_pool_size >= _10BITS_MAX_VAL_) ||
       (pools_limits->grnted_pool_size >= _10BITS_MAX_VAL_) ||
       (pools_limits->mcast_pool_size >= _10BITS_MAX_VAL_) ||
       (pools_limits->rnr_pool_size >= _10BITS_MAX_VAL_) ||
       (pools_limits->processing_pool_size >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pool_sizes_cmn_pool_lmt = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_CMN_POOL_LMT, POOL_LMT, reg_pool_sizes_cmn_pool_lmt, pools_limits->cmn_pool_lmt);
    reg_pool_sizes_grnted_pool_lmt = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_LMT, POOL_LMT, reg_pool_sizes_grnted_pool_lmt, pools_limits->grnted_pool_lmt);
    reg_pool_sizes_multi_cst_pool_lmt = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_LMT, POOL_LMT, reg_pool_sizes_multi_cst_pool_lmt, pools_limits->mcast_pool_lmt);
    reg_pool_sizes_rnr_pool_lmt = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_RNR_POOL_LMT, POOL_LMT, reg_pool_sizes_rnr_pool_lmt, pools_limits->rnr_pool_lmt);
    reg_pool_sizes_cmn_pool_size = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_CMN_POOL_SIZE, POOL_SIZE, reg_pool_sizes_cmn_pool_size, pools_limits->cmn_pool_size);
    reg_pool_sizes_grnted_pool_size = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_SIZE, POOL_SIZE, reg_pool_sizes_grnted_pool_size, pools_limits->grnted_pool_size);
    reg_pool_sizes_multi_cst_pool_size = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_SIZE, POOL_SIZE, reg_pool_sizes_multi_cst_pool_size, pools_limits->mcast_pool_size);
    reg_pool_sizes_rnr_pool_size = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_RNR_POOL_SIZE, POOL_SIZE, reg_pool_sizes_rnr_pool_size, pools_limits->rnr_pool_size);
    reg_pool_sizes_prcssing_pool_size = RU_FIELD_SET(0, DSPTCHR, POOL_SIZES_PRCSSING_POOL_SIZE, POOL_SIZE, reg_pool_sizes_prcssing_pool_size, pools_limits->processing_pool_size);

    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_CMN_POOL_LMT, reg_pool_sizes_cmn_pool_lmt);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_LMT, reg_pool_sizes_grnted_pool_lmt);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_LMT, reg_pool_sizes_multi_cst_pool_lmt);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_RNR_POOL_LMT, reg_pool_sizes_rnr_pool_lmt);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_CMN_POOL_SIZE, reg_pool_sizes_cmn_pool_size);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_SIZE, reg_pool_sizes_grnted_pool_size);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_SIZE, reg_pool_sizes_multi_cst_pool_size);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_RNR_POOL_SIZE, reg_pool_sizes_rnr_pool_size);
    RU_REG_WRITE(0, DSPTCHR, POOL_SIZES_PRCSSING_POOL_SIZE, reg_pool_sizes_prcssing_pool_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_pools_limits_get(dsptchr_pools_limits *pools_limits)
{
    uint32_t reg_pool_sizes_cmn_pool_lmt;
    uint32_t reg_pool_sizes_grnted_pool_lmt;
    uint32_t reg_pool_sizes_multi_cst_pool_lmt;
    uint32_t reg_pool_sizes_rnr_pool_lmt;
    uint32_t reg_pool_sizes_cmn_pool_size;
    uint32_t reg_pool_sizes_grnted_pool_size;
    uint32_t reg_pool_sizes_multi_cst_pool_size;
    uint32_t reg_pool_sizes_rnr_pool_size;
    uint32_t reg_pool_sizes_prcssing_pool_size;

#ifdef VALIDATE_PARMS
    if(!pools_limits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, POOL_SIZES_CMN_POOL_LMT, reg_pool_sizes_cmn_pool_lmt);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_LMT, reg_pool_sizes_grnted_pool_lmt);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_LMT, reg_pool_sizes_multi_cst_pool_lmt);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_RNR_POOL_LMT, reg_pool_sizes_rnr_pool_lmt);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_CMN_POOL_SIZE, reg_pool_sizes_cmn_pool_size);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_SIZE, reg_pool_sizes_grnted_pool_size);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_SIZE, reg_pool_sizes_multi_cst_pool_size);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_RNR_POOL_SIZE, reg_pool_sizes_rnr_pool_size);
    RU_REG_READ(0, DSPTCHR, POOL_SIZES_PRCSSING_POOL_SIZE, reg_pool_sizes_prcssing_pool_size);

    pools_limits->cmn_pool_lmt = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_CMN_POOL_LMT, POOL_LMT, reg_pool_sizes_cmn_pool_lmt);
    pools_limits->grnted_pool_lmt = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_LMT, POOL_LMT, reg_pool_sizes_grnted_pool_lmt);
    pools_limits->mcast_pool_lmt = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_LMT, POOL_LMT, reg_pool_sizes_multi_cst_pool_lmt);
    pools_limits->rnr_pool_lmt = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_RNR_POOL_LMT, POOL_LMT, reg_pool_sizes_rnr_pool_lmt);
    pools_limits->cmn_pool_size = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_CMN_POOL_SIZE, POOL_SIZE, reg_pool_sizes_cmn_pool_size);
    pools_limits->grnted_pool_size = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_GRNTED_POOL_SIZE, POOL_SIZE, reg_pool_sizes_grnted_pool_size);
    pools_limits->mcast_pool_size = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_MULTI_CST_POOL_SIZE, POOL_SIZE, reg_pool_sizes_multi_cst_pool_size);
    pools_limits->rnr_pool_size = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_RNR_POOL_SIZE, POOL_SIZE, reg_pool_sizes_rnr_pool_size);
    pools_limits->processing_pool_size = RU_FIELD_GET(0, DSPTCHR, POOL_SIZES_PRCSSING_POOL_SIZE, POOL_SIZE, reg_pool_sizes_prcssing_pool_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_fll_entry_set(const dsptchr_fll_entry *fll_entry)
{
    uint32_t reg_flldes_head=0;
    uint32_t reg_flldes_tail=0;
    uint32_t reg_flldes_ltint=0;
    uint32_t reg_flldes_bfin=0;
    uint32_t reg_flldes_bfout=0;

#ifdef VALIDATE_PARMS
    if(!fll_entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    reg_flldes_head = RU_FIELD_SET(0, DSPTCHR, FLLDES_HEAD, HEAD, reg_flldes_head, fll_entry->head);
    reg_flldes_tail = RU_FIELD_SET(0, DSPTCHR, FLLDES_TAIL, TAIL, reg_flldes_tail, fll_entry->tail);
    reg_flldes_ltint = RU_FIELD_SET(0, DSPTCHR, FLLDES_LTINT, MINBUF, reg_flldes_ltint, fll_entry->minbuf);
    reg_flldes_bfin = RU_FIELD_SET(0, DSPTCHR, FLLDES_BFIN, BFIN, reg_flldes_bfin, fll_entry->bfin);
    reg_flldes_bfout = RU_FIELD_SET(0, DSPTCHR, FLLDES_BFOUT, COUNT, reg_flldes_bfout, fll_entry->count);

    RU_REG_WRITE(0, DSPTCHR, FLLDES_HEAD, reg_flldes_head);
    RU_REG_WRITE(0, DSPTCHR, FLLDES_TAIL, reg_flldes_tail);
    RU_REG_WRITE(0, DSPTCHR, FLLDES_LTINT, reg_flldes_ltint);
    RU_REG_WRITE(0, DSPTCHR, FLLDES_BFIN, reg_flldes_bfin);
    RU_REG_WRITE(0, DSPTCHR, FLLDES_BFOUT, reg_flldes_bfout);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_fll_entry_get(dsptchr_fll_entry *fll_entry)
{
    uint32_t reg_flldes_head;
    uint32_t reg_flldes_tail;
    uint32_t reg_flldes_ltint;
    uint32_t reg_flldes_bfin;
    uint32_t reg_flldes_bfout;

#ifdef VALIDATE_PARMS
    if(!fll_entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, FLLDES_HEAD, reg_flldes_head);
    RU_REG_READ(0, DSPTCHR, FLLDES_TAIL, reg_flldes_tail);
    RU_REG_READ(0, DSPTCHR, FLLDES_LTINT, reg_flldes_ltint);
    RU_REG_READ(0, DSPTCHR, FLLDES_BFIN, reg_flldes_bfin);
    RU_REG_READ(0, DSPTCHR, FLLDES_BFOUT, reg_flldes_bfout);

    fll_entry->head = RU_FIELD_GET(0, DSPTCHR, FLLDES_HEAD, HEAD, reg_flldes_head);
    fll_entry->tail = RU_FIELD_GET(0, DSPTCHR, FLLDES_TAIL, TAIL, reg_flldes_tail);
    fll_entry->minbuf = RU_FIELD_GET(0, DSPTCHR, FLLDES_LTINT, MINBUF, reg_flldes_ltint);
    fll_entry->bfin = RU_FIELD_GET(0, DSPTCHR, FLLDES_BFIN, BFIN, reg_flldes_bfin);
    fll_entry->count = RU_FIELD_GET(0, DSPTCHR, FLLDES_BFOUT, COUNT, reg_flldes_bfout);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_set(uint8_t rnr_idx, const dsptchr_rnr_dsptch_addr *rnr_dsptch_addr)
{
    uint32_t reg_queue_mapping_pd_dsptch_add=0;

#ifdef VALIDATE_PARMS
    if(!rnr_dsptch_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_mapping_pd_dsptch_add = RU_FIELD_SET(0, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, BASE_ADD, reg_queue_mapping_pd_dsptch_add, rnr_dsptch_addr->base_add);
    reg_queue_mapping_pd_dsptch_add = RU_FIELD_SET(0, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, OFFSET_ADD, reg_queue_mapping_pd_dsptch_add, rnr_dsptch_addr->offset_add);

    RU_REG_RAM_WRITE(0, rnr_idx, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, reg_queue_mapping_pd_dsptch_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_get(uint8_t rnr_idx, dsptchr_rnr_dsptch_addr *rnr_dsptch_addr)
{
    uint32_t reg_queue_mapping_pd_dsptch_add;

#ifdef VALIDATE_PARMS
    if(!rnr_dsptch_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rnr_idx, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, reg_queue_mapping_pd_dsptch_add);

    rnr_dsptch_addr->base_add = RU_FIELD_GET(0, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, BASE_ADD, reg_queue_mapping_pd_dsptch_add);
    rnr_dsptch_addr->offset_add = RU_FIELD_GET(0, DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD, OFFSET_ADD, reg_queue_mapping_pd_dsptch_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(const dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg)
{
    uint32_t reg_reorder_cfg_dsptchr_reordr_cfg=0;

#ifdef VALIDATE_PARMS
    if(!reorder_cfg_dsptchr_reordr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((reorder_cfg_dsptchr_reordr_cfg->disp_enable >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->rdy >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->reordr_par_mod >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->per_q_egrs_congst_en >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->dsptch_sm_enh_mod >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_en >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_cnt >= _3BITS_MAX_VAL_) ||
       (reorder_cfg_dsptchr_reordr_cfg->egrs_drop_only >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, EN, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->disp_enable);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, RDY, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->rdy);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, REORDR_PAR_MOD, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->reordr_par_mod);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, PER_Q_EGRS_CONGST_EN, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->per_q_egrs_congst_en);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, DSPTCH_SM_ENH_MOD, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->dsptch_sm_enh_mod);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, INGRS_PIPE_DLY_EN, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_en);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, INGRS_PIPE_DLY_CNT, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_cnt);
    reg_reorder_cfg_dsptchr_reordr_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, EGRS_DROP_ONLY, reg_reorder_cfg_dsptchr_reordr_cfg, reorder_cfg_dsptchr_reordr_cfg->egrs_drop_only);

    RU_REG_WRITE(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, reg_reorder_cfg_dsptchr_reordr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get(dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg)
{
    uint32_t reg_reorder_cfg_dsptchr_reordr_cfg;

#ifdef VALIDATE_PARMS
    if(!reorder_cfg_dsptchr_reordr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, reg_reorder_cfg_dsptchr_reordr_cfg);

    reorder_cfg_dsptchr_reordr_cfg->disp_enable = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, EN, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->rdy = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, RDY, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->reordr_par_mod = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, REORDR_PAR_MOD, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->per_q_egrs_congst_en = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, PER_Q_EGRS_CONGST_EN, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->dsptch_sm_enh_mod = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, DSPTCH_SM_ENH_MOD, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_en = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, INGRS_PIPE_DLY_EN, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->ingrs_pipe_dly_cnt = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, INGRS_PIPE_DLY_CNT, reg_reorder_cfg_dsptchr_reordr_cfg);
    reorder_cfg_dsptchr_reordr_cfg->egrs_drop_only = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG, EGRS_DROP_ONLY, reg_reorder_cfg_dsptchr_reordr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_set(uint32_t en)
{
    uint32_t reg_reorder_cfg_vq_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_reorder_cfg_vq_en = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_VQ_EN, EN, reg_reorder_cfg_vq_en, en);

    RU_REG_WRITE(0, DSPTCHR, REORDER_CFG_VQ_EN, reg_reorder_cfg_vq_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_get(uint32_t *en)
{
    uint32_t reg_reorder_cfg_vq_en;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, REORDER_CFG_VQ_EN, reg_reorder_cfg_vq_en);

    *en = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_VQ_EN, EN, reg_reorder_cfg_vq_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_set(uint8_t src_id, uint8_t dst_id_ovride, uint16_t route_ovride, bdmf_boolean ovride_en)
{
    uint32_t reg_reorder_cfg_bb_cfg=0;

#ifdef VALIDATE_PARMS
    if((src_id >= _6BITS_MAX_VAL_) ||
       (dst_id_ovride >= _6BITS_MAX_VAL_) ||
       (route_ovride >= _10BITS_MAX_VAL_) ||
       (ovride_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reorder_cfg_bb_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_BB_CFG, SRC_ID, reg_reorder_cfg_bb_cfg, src_id);
    reg_reorder_cfg_bb_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_BB_CFG, DST_ID_OVRIDE, reg_reorder_cfg_bb_cfg, dst_id_ovride);
    reg_reorder_cfg_bb_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_BB_CFG, ROUTE_OVRIDE, reg_reorder_cfg_bb_cfg, route_ovride);
    reg_reorder_cfg_bb_cfg = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_BB_CFG, OVRIDE_EN, reg_reorder_cfg_bb_cfg, ovride_en);

    RU_REG_WRITE(0, DSPTCHR, REORDER_CFG_BB_CFG, reg_reorder_cfg_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_get(uint8_t *src_id, uint8_t *dst_id_ovride, uint16_t *route_ovride, bdmf_boolean *ovride_en)
{
    uint32_t reg_reorder_cfg_bb_cfg;

#ifdef VALIDATE_PARMS
    if(!src_id || !dst_id_ovride || !route_ovride || !ovride_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, REORDER_CFG_BB_CFG, reg_reorder_cfg_bb_cfg);

    *src_id = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_BB_CFG, SRC_ID, reg_reorder_cfg_bb_cfg);
    *dst_id_ovride = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_BB_CFG, DST_ID_OVRIDE, reg_reorder_cfg_bb_cfg);
    *route_ovride = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_BB_CFG, ROUTE_OVRIDE, reg_reorder_cfg_bb_cfg);
    *ovride_en = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_BB_CFG, OVRIDE_EN, reg_reorder_cfg_bb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(const dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl)
{
    uint32_t reg_reorder_cfg_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!reorder_cfg_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((reorder_cfg_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (reorder_cfg_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reorder_cfg_clk_gate_cntrl = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_reorder_cfg_clk_gate_cntrl, reorder_cfg_clk_gate_cntrl->bypass_clk_gate);
    reg_reorder_cfg_clk_gate_cntrl = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, TIMER_VAL, reg_reorder_cfg_clk_gate_cntrl, reorder_cfg_clk_gate_cntrl->timer_val);
    reg_reorder_cfg_clk_gate_cntrl = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_reorder_cfg_clk_gate_cntrl, reorder_cfg_clk_gate_cntrl->keep_alive_en);
    reg_reorder_cfg_clk_gate_cntrl = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_reorder_cfg_clk_gate_cntrl, reorder_cfg_clk_gate_cntrl->keep_alive_intrvl);
    reg_reorder_cfg_clk_gate_cntrl = RU_FIELD_SET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_reorder_cfg_clk_gate_cntrl, reorder_cfg_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, reg_reorder_cfg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl)
{
    uint32_t reg_reorder_cfg_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!reorder_cfg_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, reg_reorder_cfg_clk_gate_cntrl);

    reorder_cfg_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_reorder_cfg_clk_gate_cntrl);
    reorder_cfg_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, TIMER_VAL, reg_reorder_cfg_clk_gate_cntrl);
    reorder_cfg_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_reorder_cfg_clk_gate_cntrl);
    reorder_cfg_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_reorder_cfg_clk_gate_cntrl);
    reorder_cfg_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_reorder_cfg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params)
{
    /* Identical to cngs_params */
    uint32_t reg_congestion_ingrs_congstn=0;

#ifdef VALIDATE_PARMS
    if(!cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((viq_idx >= 32) ||
       (cngs_params->frst_lvl >= _12BITS_MAX_VAL_) ||
       (cngs_params->scnd_lvl >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, FRST_LVL, reg_congestion_ingrs_congstn, cngs_params->frst_lvl);
    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, SCND_LVL, reg_congestion_ingrs_congstn, cngs_params->scnd_lvl);
    reg_congestion_ingrs_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, HYST_THRS, reg_congestion_ingrs_congstn, cngs_params->hyst_thrs);

    RU_REG_RAM_WRITE(0, viq_idx, DSPTCHR, CONGESTION_EGRS_CONGSTN, reg_congestion_ingrs_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params)
{
    /* Identical to cngs_params */
    uint32_t reg_congestion_ingrs_congstn;

#ifdef VALIDATE_PARMS
    if(!cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((viq_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, viq_idx, DSPTCHR, CONGESTION_EGRS_CONGSTN, reg_congestion_ingrs_congstn);

    cngs_params->frst_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, FRST_LVL, reg_congestion_ingrs_congstn);
    cngs_params->scnd_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, SCND_LVL, reg_congestion_ingrs_congstn);
    cngs_params->hyst_thrs = RU_FIELD_GET(0, DSPTCHR, CONGESTION_INGRS_CONGSTN, HYST_THRS, reg_congestion_ingrs_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_set(const dsptchr_glbl_cngs_params *glbl_cngs_params)
{
    /* Identical to glbl_cngs_params */
    uint32_t reg_congestion_glbl_congstn=0;

#ifdef VALIDATE_PARMS
    if(!glbl_cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((glbl_cngs_params->frst_lvl >= _12BITS_MAX_VAL_) ||
       (glbl_cngs_params->scnd_lvl >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, FRST_LVL, reg_congestion_glbl_congstn, glbl_cngs_params->frst_lvl);
    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, SCND_LVL, reg_congestion_glbl_congstn, glbl_cngs_params->scnd_lvl);
    reg_congestion_glbl_congstn = RU_FIELD_SET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, HYST_THRS, reg_congestion_glbl_congstn, glbl_cngs_params->hyst_thrs);

    RU_REG_WRITE(0, DSPTCHR, CONGESTION_TOTAL_EGRS_CONGSTN, reg_congestion_glbl_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_get(dsptchr_glbl_cngs_params *glbl_cngs_params)
{
    /* Identical to glbl_cngs_params */
    uint32_t reg_congestion_glbl_congstn;

#ifdef VALIDATE_PARMS
    if(!glbl_cngs_params)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_TOTAL_EGRS_CONGSTN, reg_congestion_glbl_congstn);

    glbl_cngs_params->frst_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, FRST_LVL, reg_congestion_glbl_congstn);
    glbl_cngs_params->scnd_lvl = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, SCND_LVL, reg_congestion_glbl_congstn);
    glbl_cngs_params->hyst_thrs = RU_FIELD_GET(0, DSPTCHR, CONGESTION_GLBL_CONGSTN, HYST_THRS, reg_congestion_glbl_congstn);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_congstn_status_get(dsptchr_congestion_congstn_status *congestion_congstn_status)
{
    uint32_t reg_congestion_congstn_status;

#ifdef VALIDATE_PARMS
    if(!congestion_congstn_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, reg_congestion_congstn_status);

    congestion_congstn_status->glbl_congstn = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, GLBL_CONGSTN, reg_congestion_congstn_status);
    congestion_congstn_status->glbl_egrs_congstn = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, GLBL_EGRS_CONGSTN, reg_congestion_congstn_status);
    congestion_congstn_status->sbpm_congstn = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, SBPM_CONGSTN, reg_congestion_congstn_status);
    congestion_congstn_status->glbl_congstn_stcky = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, GLBL_CONGSTN_STCKY, reg_congestion_congstn_status);
    congestion_congstn_status->glbl_egrs_congstn_stcky = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, GLBL_EGRS_CONGSTN_STCKY, reg_congestion_congstn_status);
    congestion_congstn_status->sbpm_congstn_stcky = RU_FIELD_GET(0, DSPTCHR, CONGESTION_CONGSTN_STATUS, SBPM_CONGSTN_STCKY, reg_congestion_congstn_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get(uint32_t *congstn_state)
{
    uint32_t reg_congestion_per_q_ingrs_congstn_low;

#ifdef VALIDATE_PARMS
    if(!congstn_state)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_LOW, reg_congestion_per_q_ingrs_congstn_low);

    *congstn_state = RU_FIELD_GET(0, DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_LOW, CONGSTN_STATE, reg_congestion_per_q_ingrs_congstn_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get(uint32_t *congstn_state)
{
    uint32_t reg_congestion_per_q_ingrs_congstn_high;

#ifdef VALIDATE_PARMS
    if(!congstn_state)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_HIGH, reg_congestion_per_q_ingrs_congstn_high);

    *congstn_state = RU_FIELD_GET(0, DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_HIGH, CONGSTN_STATE, reg_congestion_per_q_ingrs_congstn_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get(uint32_t *congstn_state)
{
    uint32_t reg_congestion_per_q_egrs_congstn_low;

#ifdef VALIDATE_PARMS
    if(!congstn_state)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_LOW, reg_congestion_per_q_egrs_congstn_low);

    *congstn_state = RU_FIELD_GET(0, DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_LOW, CONGSTN_STATE, reg_congestion_per_q_egrs_congstn_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get(uint32_t *congstn_state)
{
    uint32_t reg_congestion_per_q_egrs_congstn_high;

#ifdef VALIDATE_PARMS
    if(!congstn_state)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_HIGH, reg_congestion_per_q_egrs_congstn_high);

    *congstn_state = RU_FIELD_GET(0, DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_HIGH, CONGSTN_STATE, reg_congestion_per_q_egrs_congstn_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_set(uint8_t q_idx, uint8_t bb_id, uint16_t trgt_add)
{
    uint32_t reg_queue_mapping_crdt_cfg=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_mapping_crdt_cfg = RU_FIELD_SET(0, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, BB_ID, reg_queue_mapping_crdt_cfg, bb_id);
    reg_queue_mapping_crdt_cfg = RU_FIELD_SET(0, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, TRGT_ADD, reg_queue_mapping_crdt_cfg, trgt_add);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, reg_queue_mapping_crdt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_get(uint8_t q_idx, uint8_t *bb_id, uint16_t *trgt_add)
{
    uint32_t reg_queue_mapping_crdt_cfg;

#ifdef VALIDATE_PARMS
    if(!bb_id || !trgt_add)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, reg_queue_mapping_crdt_cfg);

    *bb_id = RU_FIELD_GET(0, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, BB_ID, reg_queue_mapping_crdt_cfg);
    *trgt_add = RU_FIELD_GET(0, DSPTCHR, QUEUE_MAPPING_CRDT_CFG, TRGT_ADD, reg_queue_mapping_crdt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_set(uint8_t q_idx, bdmf_boolean is_dest_disp)
{
    uint32_t reg_queue_mapping_q_dest=0;

#ifdef VALIDATE_PARMS
    if((is_dest_disp >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QUEUE_MAPPING_Q_DEST, reg_queue_mapping_q_dest);

    FIELD_SET(reg_queue_mapping_q_dest, (q_idx % 32) *1, 0x1, is_dest_disp);

    RU_REG_WRITE(0, DSPTCHR, QUEUE_MAPPING_Q_DEST, reg_queue_mapping_q_dest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_get(uint8_t q_idx, bdmf_boolean *is_dest_disp)
{
    uint32_t reg_queue_mapping_q_dest;

#ifdef VALIDATE_PARMS
    if(!is_dest_disp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QUEUE_MAPPING_Q_DEST, reg_queue_mapping_q_dest);

    *is_dest_disp = FIELD_GET(reg_queue_mapping_q_dest, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_set(uint8_t group_idx, const dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0)
{
#ifdef VALIDATE_PARMS
    if(!mask_msk_tsk_255_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((group_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, group_idx *8 + 0, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[0]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 1, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[1]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 2, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[2]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 3, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[3]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 4, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[4]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 5, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[5]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 6, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[6]);
    RU_REG_RAM_WRITE(0, group_idx *8 + 7, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_get(uint8_t group_idx, dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0)
{
#ifdef VALIDATE_PARMS
    if(!mask_msk_tsk_255_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((group_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, group_idx *8 + 0, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[0]);
    RU_REG_RAM_READ(0, group_idx *8 + 1, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[1]);
    RU_REG_RAM_READ(0, group_idx *8 + 2, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[2]);
    RU_REG_RAM_READ(0, group_idx *8 + 3, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[3]);
    RU_REG_RAM_READ(0, group_idx *8 + 4, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[4]);
    RU_REG_RAM_READ(0, group_idx *8 + 5, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[5]);
    RU_REG_RAM_READ(0, group_idx *8 + 6, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[6]);
    RU_REG_RAM_READ(0, group_idx *8 + 7, DSPTCHR, MASK_MSK_TSK_255_0, mask_msk_tsk_255_0->task_mask[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_msk_q_set(uint8_t group_idx, uint32_t mask)
{
    uint32_t reg_mask_msk_q=0;

#ifdef VALIDATE_PARMS
    if((group_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mask_msk_q = RU_FIELD_SET(0, DSPTCHR, MASK_MSK_Q, MASK, reg_mask_msk_q, mask);

    RU_REG_RAM_WRITE(0, group_idx, DSPTCHR, MASK_MSK_Q, reg_mask_msk_q);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_msk_q_get(uint8_t group_idx, uint32_t *mask)
{
    uint32_t reg_mask_msk_q;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((group_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, group_idx, DSPTCHR, MASK_MSK_Q, reg_mask_msk_q);

    *mask = RU_FIELD_GET(0, DSPTCHR, MASK_MSK_Q, MASK, reg_mask_msk_q);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_dly_q_set(uint8_t q_idx, bdmf_boolean set_delay)
{
    uint32_t reg_mask_dly_q=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, DSPTCHR, MASK_DLY_Q, reg_mask_dly_q);

    FIELD_SET(reg_mask_dly_q, (q_idx % 32) *1, 0x1, set_delay);

    RU_REG_WRITE(0, DSPTCHR, MASK_DLY_Q, reg_mask_dly_q);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_dly_q_get(uint8_t q_idx, bdmf_boolean *set_delay)
{
    uint32_t reg_mask_dly_q;

#ifdef VALIDATE_PARMS
    if(!set_delay)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, MASK_DLY_Q, reg_mask_dly_q);

    *set_delay = FIELD_GET(reg_mask_dly_q, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_set(uint8_t q_idx, bdmf_boolean set_non_delay)
{
    uint32_t reg_mask_non_dly_q=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, DSPTCHR, MASK_NON_DLY_Q, reg_mask_non_dly_q);

    FIELD_SET(reg_mask_non_dly_q, (q_idx % 32) *1, 0x1, set_non_delay);

    RU_REG_WRITE(0, DSPTCHR, MASK_NON_DLY_Q, reg_mask_non_dly_q);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_get(uint8_t q_idx, bdmf_boolean *set_non_delay)
{
    uint32_t reg_mask_non_dly_q;

#ifdef VALIDATE_PARMS
    if(!set_non_delay)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, MASK_NON_DLY_Q, reg_mask_non_dly_q);

    *set_non_delay = FIELD_GET(reg_mask_non_dly_q, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set(uint8_t dly_crdt)
{
    uint32_t reg_egrs_queues_egrs_dly_qm_crdt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_egrs_queues_egrs_dly_qm_crdt = RU_FIELD_SET(0, DSPTCHR, EGRS_QUEUES_EGRS_DLY_QM_CRDT, DLY_CRDT, reg_egrs_queues_egrs_dly_qm_crdt, dly_crdt);

    RU_REG_WRITE(0, DSPTCHR, EGRS_QUEUES_EGRS_DLY_QM_CRDT, reg_egrs_queues_egrs_dly_qm_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get(uint8_t *dly_crdt)
{
    uint32_t reg_egrs_queues_egrs_dly_qm_crdt;

#ifdef VALIDATE_PARMS
    if(!dly_crdt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, EGRS_QUEUES_EGRS_DLY_QM_CRDT, reg_egrs_queues_egrs_dly_qm_crdt);

    *dly_crdt = RU_FIELD_GET(0, DSPTCHR, EGRS_QUEUES_EGRS_DLY_QM_CRDT, DLY_CRDT, reg_egrs_queues_egrs_dly_qm_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set(uint8_t non_dly_crdt)
{
    uint32_t reg_egrs_queues_egrs_non_dly_qm_crdt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_egrs_queues_egrs_non_dly_qm_crdt = RU_FIELD_SET(0, DSPTCHR, EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT, NON_DLY_CRDT, reg_egrs_queues_egrs_non_dly_qm_crdt, non_dly_crdt);

    RU_REG_WRITE(0, DSPTCHR, EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT, reg_egrs_queues_egrs_non_dly_qm_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get(uint8_t *non_dly_crdt)
{
    uint32_t reg_egrs_queues_egrs_non_dly_qm_crdt;

#ifdef VALIDATE_PARMS
    if(!non_dly_crdt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT, reg_egrs_queues_egrs_non_dly_qm_crdt);

    *non_dly_crdt = RU_FIELD_GET(0, DSPTCHR, EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT, NON_DLY_CRDT, reg_egrs_queues_egrs_non_dly_qm_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set(uint16_t total_egrs_size)
{
    uint32_t reg_egrs_queues_total_q_egrs_size=0;

#ifdef VALIDATE_PARMS
    if((total_egrs_size >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_egrs_queues_total_q_egrs_size = RU_FIELD_SET(0, DSPTCHR, EGRS_QUEUES_TOTAL_Q_EGRS_SIZE, TOTAL_EGRS_SIZE, reg_egrs_queues_total_q_egrs_size, total_egrs_size);

    RU_REG_WRITE(0, DSPTCHR, EGRS_QUEUES_TOTAL_Q_EGRS_SIZE, reg_egrs_queues_total_q_egrs_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get(uint16_t *total_egrs_size)
{
    uint32_t reg_egrs_queues_total_q_egrs_size;

#ifdef VALIDATE_PARMS
    if(!total_egrs_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, EGRS_QUEUES_TOTAL_Q_EGRS_SIZE, reg_egrs_queues_total_q_egrs_size);

    *total_egrs_size = RU_FIELD_GET(0, DSPTCHR, EGRS_QUEUES_TOTAL_Q_EGRS_SIZE, TOTAL_EGRS_SIZE, reg_egrs_queues_total_q_egrs_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get(uint16_t q_idx, uint16_t *q_egrs_size)
{
    uint32_t reg_egrs_queues_per_q_egrs_size;

#ifdef VALIDATE_PARMS
    if(!q_egrs_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, EGRS_QUEUES_PER_Q_EGRS_SIZE, reg_egrs_queues_per_q_egrs_size);

    *q_egrs_size = RU_FIELD_GET(0, DSPTCHR, EGRS_QUEUES_PER_Q_EGRS_SIZE, Q_EGRS_SIZE, reg_egrs_queues_per_q_egrs_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_set(const dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req)
{
    uint32_t reg_wakeup_control_wkup_req=0;

#ifdef VALIDATE_PARMS
    if(!wakeup_control_wkup_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((wakeup_control_wkup_req->q0 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q1 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q2 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q3 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q4 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q5 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q6 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q7 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q8 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q9 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q10 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q11 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q12 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q13 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q14 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q15 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q16 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q17 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q18 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q19 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q20 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q21 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q22 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q23 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q24 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q25 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q26 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q27 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q28 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q29 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q30 >= _1BITS_MAX_VAL_) ||
       (wakeup_control_wkup_req->q31 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q0, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q0);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q1, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q1);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q2, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q2);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q3, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q3);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q4, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q4);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q5, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q5);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q6, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q6);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q7, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q7);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q8, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q8);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q9, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q9);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q10, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q10);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q11, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q11);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q12, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q12);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q13, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q13);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q14, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q14);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q15, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q15);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q16, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q16);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q17, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q17);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q18, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q18);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q19, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q19);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q20, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q20);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q21, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q21);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q22, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q22);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q23, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q23);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q24, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q24);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q25, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q25);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q26, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q26);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q27, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q27);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q28, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q28);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q29, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q29);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q30, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q30);
    reg_wakeup_control_wkup_req = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q31, reg_wakeup_control_wkup_req, wakeup_control_wkup_req->q31);

    RU_REG_WRITE(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, reg_wakeup_control_wkup_req);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_get(dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req)
{
    uint32_t reg_wakeup_control_wkup_req;

#ifdef VALIDATE_PARMS
    if(!wakeup_control_wkup_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, reg_wakeup_control_wkup_req);

    wakeup_control_wkup_req->q0 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q0, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q1 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q1, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q2 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q2, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q3 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q3, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q4 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q4, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q5 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q5, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q6 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q6, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q7 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q7, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q8 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q8, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q9 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q9, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q10 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q10, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q11 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q11, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q12 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q12, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q13 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q13, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q14 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q14, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q15 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q15, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q16 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q16, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q17 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q17, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q18 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q18, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q19 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q19, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q20 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q20, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q21 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q21, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q22 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q22, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q23 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q23, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q24 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q24, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q25 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q25, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q26 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q26, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q27 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q27, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q28 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q28, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q29 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q29, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q30 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q30, reg_wakeup_control_wkup_req);
    wakeup_control_wkup_req->q31 = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_REQ, Q31, reg_wakeup_control_wkup_req);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(uint16_t wkup_thrshld)
{
    uint32_t reg_wakeup_control_wkup_thrshld=0;

#ifdef VALIDATE_PARMS
    if((wkup_thrshld >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wakeup_control_wkup_thrshld = RU_FIELD_SET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_THRSHLD, WKUP_THRSHLD, reg_wakeup_control_wkup_thrshld, wkup_thrshld);

    RU_REG_WRITE(0, DSPTCHR, WAKEUP_CONTROL_WKUP_THRSHLD, reg_wakeup_control_wkup_thrshld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_get(uint16_t *wkup_thrshld)
{
    uint32_t reg_wakeup_control_wkup_thrshld;

#ifdef VALIDATE_PARMS
    if(!wkup_thrshld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, WAKEUP_CONTROL_WKUP_THRSHLD, reg_wakeup_control_wkup_thrshld);

    *wkup_thrshld = RU_FIELD_GET(0, DSPTCHR, WAKEUP_CONTROL_WKUP_THRSHLD, WKUP_THRSHLD, reg_wakeup_control_wkup_thrshld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_set(uint8_t dwrr_q_idx, uint32_t q_crdt, bdmf_boolean ngtv, uint16_t quntum)
{
    uint32_t reg_disptch_scheduling_dwrr_info=0;

#ifdef VALIDATE_PARMS
    if((dwrr_q_idx >= 32) ||
       (q_crdt >= _20BITS_MAX_VAL_) ||
       (ngtv >= _1BITS_MAX_VAL_) ||
       (quntum >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_disptch_scheduling_dwrr_info = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, Q_CRDT, reg_disptch_scheduling_dwrr_info, q_crdt);
    reg_disptch_scheduling_dwrr_info = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, NGTV, reg_disptch_scheduling_dwrr_info, ngtv);
    reg_disptch_scheduling_dwrr_info = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, QUNTUM, reg_disptch_scheduling_dwrr_info, quntum);

    RU_REG_RAM_WRITE(0, dwrr_q_idx, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, reg_disptch_scheduling_dwrr_info);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_get(uint8_t dwrr_q_idx, uint32_t *q_crdt, bdmf_boolean *ngtv, uint16_t *quntum)
{
    uint32_t reg_disptch_scheduling_dwrr_info;

#ifdef VALIDATE_PARMS
    if(!q_crdt || !ngtv || !quntum)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dwrr_q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, dwrr_q_idx, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, reg_disptch_scheduling_dwrr_info);

    *q_crdt = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, Q_CRDT, reg_disptch_scheduling_dwrr_info);
    *ngtv = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, NGTV, reg_disptch_scheduling_dwrr_info);
    *quntum = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO, QUNTUM, reg_disptch_scheduling_dwrr_info);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_set(const dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt)
{
    uint32_t reg_disptch_scheduling_vld_crdt=0;

#ifdef VALIDATE_PARMS
    if(!disptch_scheduling_vld_crdt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((disptch_scheduling_vld_crdt->q0 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q1 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q2 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q3 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q4 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q5 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q6 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q7 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q8 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q9 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q10 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q11 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q12 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q13 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q14 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q15 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q16 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q17 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q18 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q19 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q20 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q21 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q22 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q23 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q24 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q25 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q26 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q27 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q28 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q29 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q30 >= _1BITS_MAX_VAL_) ||
       (disptch_scheduling_vld_crdt->q31 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q0, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q0);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q1, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q1);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q2, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q2);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q3, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q3);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q4, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q4);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q5, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q5);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q6, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q6);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q7, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q7);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q8, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q8);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q9, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q9);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q10, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q10);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q11, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q11);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q12, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q12);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q13, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q13);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q14, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q14);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q15, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q15);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q16, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q16);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q17, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q17);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q18, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q18);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q19, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q19);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q20, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q20);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q21, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q21);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q22, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q22);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q23, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q23);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q24, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q24);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q25, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q25);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q26, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q26);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q27, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q27);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q28, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q28);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q29, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q29);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q30, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q30);
    reg_disptch_scheduling_vld_crdt = RU_FIELD_SET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q31, reg_disptch_scheduling_vld_crdt, disptch_scheduling_vld_crdt->q31);

    RU_REG_WRITE(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, reg_disptch_scheduling_vld_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_get(dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt)
{
    uint32_t reg_disptch_scheduling_vld_crdt;

#ifdef VALIDATE_PARMS
    if(!disptch_scheduling_vld_crdt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, reg_disptch_scheduling_vld_crdt);

    disptch_scheduling_vld_crdt->q0 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q0, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q1 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q1, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q2 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q2, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q3 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q3, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q4 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q4, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q5 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q5, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q6 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q6, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q7 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q7, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q8 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q8, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q9 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q9, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q10 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q10, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q11 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q11, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q12 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q12, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q13 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q13, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q14 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q14, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q15 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q15, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q16 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q16, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q17 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q17, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q18 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q18, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q19 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q19, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q20 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q20, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q21 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q21, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q22 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q22, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q23 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q23, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q24 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q24, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q25 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q25, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q26 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q26, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q27 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q27, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q28 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q28, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q29 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q29, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q30 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q30, reg_disptch_scheduling_vld_crdt);
    disptch_scheduling_vld_crdt->q31 = RU_FIELD_GET(0, DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT, Q31, reg_disptch_scheduling_vld_crdt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_set(bdmf_boolean lb_mode, uint8_t sp_thrshld)
{
    uint32_t reg_load_balancing_lb_cfg=0;

#ifdef VALIDATE_PARMS
    if((lb_mode >= _1BITS_MAX_VAL_) ||
       (sp_thrshld >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_load_balancing_lb_cfg = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_LB_CFG, LB_MODE, reg_load_balancing_lb_cfg, lb_mode);
    reg_load_balancing_lb_cfg = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_LB_CFG, SP_THRSHLD, reg_load_balancing_lb_cfg, sp_thrshld);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_LB_CFG, reg_load_balancing_lb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_get(bdmf_boolean *lb_mode, uint8_t *sp_thrshld)
{
    uint32_t reg_load_balancing_lb_cfg;

#ifdef VALIDATE_PARMS
    if(!lb_mode || !sp_thrshld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_LB_CFG, reg_load_balancing_lb_cfg);

    *lb_mode = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_LB_CFG, LB_MODE, reg_load_balancing_lb_cfg);
    *sp_thrshld = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_LB_CFG, SP_THRSHLD, reg_load_balancing_lb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_set(uint16_t rnr0, uint16_t rnr1)
{
    uint32_t reg_load_balancing_free_task_0_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_0_1 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, RNR0, reg_load_balancing_free_task_0_1, rnr0);
    reg_load_balancing_free_task_0_1 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, RNR1, reg_load_balancing_free_task_0_1, rnr1);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, reg_load_balancing_free_task_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_get(uint16_t *rnr0, uint16_t *rnr1)
{
    uint32_t reg_load_balancing_free_task_0_1;

#ifdef VALIDATE_PARMS
    if(!rnr0 || !rnr1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, reg_load_balancing_free_task_0_1);

    *rnr0 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, RNR0, reg_load_balancing_free_task_0_1);
    *rnr1 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1, RNR1, reg_load_balancing_free_task_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_set(uint16_t rnr2, uint16_t rnr3)
{
    uint32_t reg_load_balancing_free_task_2_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_2_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, RNR2, reg_load_balancing_free_task_2_3, rnr2);
    reg_load_balancing_free_task_2_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, RNR3, reg_load_balancing_free_task_2_3, rnr3);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, reg_load_balancing_free_task_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_get(uint16_t *rnr2, uint16_t *rnr3)
{
    uint32_t reg_load_balancing_free_task_2_3;

#ifdef VALIDATE_PARMS
    if(!rnr2 || !rnr3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, reg_load_balancing_free_task_2_3);

    *rnr2 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, RNR2, reg_load_balancing_free_task_2_3);
    *rnr3 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3, RNR3, reg_load_balancing_free_task_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_set(uint16_t rnr4, uint16_t rnr5)
{
    uint32_t reg_load_balancing_free_task_4_5=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_4_5 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, RNR4, reg_load_balancing_free_task_4_5, rnr4);
    reg_load_balancing_free_task_4_5 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, RNR5, reg_load_balancing_free_task_4_5, rnr5);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, reg_load_balancing_free_task_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_get(uint16_t *rnr4, uint16_t *rnr5)
{
    uint32_t reg_load_balancing_free_task_4_5;

#ifdef VALIDATE_PARMS
    if(!rnr4 || !rnr5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, reg_load_balancing_free_task_4_5);

    *rnr4 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, RNR4, reg_load_balancing_free_task_4_5);
    *rnr5 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5, RNR5, reg_load_balancing_free_task_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_set(uint16_t rnr6, uint16_t rnr7)
{
    uint32_t reg_load_balancing_free_task_6_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_6_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, RNR6, reg_load_balancing_free_task_6_7, rnr6);
    reg_load_balancing_free_task_6_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, RNR7, reg_load_balancing_free_task_6_7, rnr7);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, reg_load_balancing_free_task_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_get(uint16_t *rnr6, uint16_t *rnr7)
{
    uint32_t reg_load_balancing_free_task_6_7;

#ifdef VALIDATE_PARMS
    if(!rnr6 || !rnr7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, reg_load_balancing_free_task_6_7);

    *rnr6 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, RNR6, reg_load_balancing_free_task_6_7);
    *rnr7 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7, RNR7, reg_load_balancing_free_task_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_set(uint16_t rnr8, uint16_t rnr9)
{
    uint32_t reg_load_balancing_free_task_8_9=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_8_9 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, RNR8, reg_load_balancing_free_task_8_9, rnr8);
    reg_load_balancing_free_task_8_9 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, RNR9, reg_load_balancing_free_task_8_9, rnr9);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, reg_load_balancing_free_task_8_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_get(uint16_t *rnr8, uint16_t *rnr9)
{
    uint32_t reg_load_balancing_free_task_8_9;

#ifdef VALIDATE_PARMS
    if(!rnr8 || !rnr9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, reg_load_balancing_free_task_8_9);

    *rnr8 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, RNR8, reg_load_balancing_free_task_8_9);
    *rnr9 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9, RNR9, reg_load_balancing_free_task_8_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_set(uint16_t rnr10, uint16_t rnr11)
{
    uint32_t reg_load_balancing_free_task_10_11=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_10_11 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, RNR10, reg_load_balancing_free_task_10_11, rnr10);
    reg_load_balancing_free_task_10_11 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, RNR11, reg_load_balancing_free_task_10_11, rnr11);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, reg_load_balancing_free_task_10_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_get(uint16_t *rnr10, uint16_t *rnr11)
{
    uint32_t reg_load_balancing_free_task_10_11;

#ifdef VALIDATE_PARMS
    if(!rnr10 || !rnr11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, reg_load_balancing_free_task_10_11);

    *rnr10 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, RNR10, reg_load_balancing_free_task_10_11);
    *rnr11 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11, RNR11, reg_load_balancing_free_task_10_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_set(uint16_t rnr12, uint16_t rnr13)
{
    uint32_t reg_load_balancing_free_task_12_13=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_12_13 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, RNR12, reg_load_balancing_free_task_12_13, rnr12);
    reg_load_balancing_free_task_12_13 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, RNR13, reg_load_balancing_free_task_12_13, rnr13);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, reg_load_balancing_free_task_12_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_get(uint16_t *rnr12, uint16_t *rnr13)
{
    uint32_t reg_load_balancing_free_task_12_13;

#ifdef VALIDATE_PARMS
    if(!rnr12 || !rnr13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, reg_load_balancing_free_task_12_13);

    *rnr12 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, RNR12, reg_load_balancing_free_task_12_13);
    *rnr13 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13, RNR13, reg_load_balancing_free_task_12_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_set(uint16_t rnr14, uint16_t rnr15)
{
    uint32_t reg_load_balancing_free_task_14_15=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_free_task_14_15 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, RNR14, reg_load_balancing_free_task_14_15, rnr14);
    reg_load_balancing_free_task_14_15 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, RNR15, reg_load_balancing_free_task_14_15, rnr15);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, reg_load_balancing_free_task_14_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_get(uint16_t *rnr14, uint16_t *rnr15)
{
    uint32_t reg_load_balancing_free_task_14_15;

#ifdef VALIDATE_PARMS
    if(!rnr14 || !rnr15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, reg_load_balancing_free_task_14_15);

    *rnr14 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, RNR14, reg_load_balancing_free_task_14_15);
    *rnr15 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15, RNR15, reg_load_balancing_free_task_14_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(uint8_t task_to_rg_mapping, const dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping)
{
    uint32_t reg_load_balancing_tsk_to_rg_mapping=0;

#ifdef VALIDATE_PARMS
    if(!load_balancing_tsk_to_rg_mapping)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((task_to_rg_mapping >= 32) ||
       (load_balancing_tsk_to_rg_mapping->tsk0 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk1 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk2 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk3 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk4 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk5 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk6 >= _3BITS_MAX_VAL_) ||
       (load_balancing_tsk_to_rg_mapping->tsk7 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK0, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk0);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK1, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk1);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK2, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk2);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK3, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk3);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK4, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk4);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK5, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk5);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK6, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk6);
    reg_load_balancing_tsk_to_rg_mapping = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK7, reg_load_balancing_tsk_to_rg_mapping, load_balancing_tsk_to_rg_mapping->tsk7);

    RU_REG_RAM_WRITE(0, task_to_rg_mapping, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, reg_load_balancing_tsk_to_rg_mapping);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get(uint8_t task_to_rg_mapping, dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping)
{
    uint32_t reg_load_balancing_tsk_to_rg_mapping;

#ifdef VALIDATE_PARMS
    if(!load_balancing_tsk_to_rg_mapping)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((task_to_rg_mapping >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, task_to_rg_mapping, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, reg_load_balancing_tsk_to_rg_mapping);

    load_balancing_tsk_to_rg_mapping->tsk0 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK0, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk1 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK1, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk2 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK2, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk3 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK3, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk4 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK4, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk5 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK5, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk6 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK6, reg_load_balancing_tsk_to_rg_mapping);
    load_balancing_tsk_to_rg_mapping->tsk7 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING, TSK7, reg_load_balancing_tsk_to_rg_mapping);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(uint8_t tsk_cnt_rg_0, uint8_t tsk_cnt_rg_1, uint8_t tsk_cnt_rg_2, uint8_t tsk_cnt_rg_3)
{
    uint32_t reg_load_balancing_rg_avlabl_tsk_0_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_rg_avlabl_tsk_0_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_0, reg_load_balancing_rg_avlabl_tsk_0_3, tsk_cnt_rg_0);
    reg_load_balancing_rg_avlabl_tsk_0_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_1, reg_load_balancing_rg_avlabl_tsk_0_3, tsk_cnt_rg_1);
    reg_load_balancing_rg_avlabl_tsk_0_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_2, reg_load_balancing_rg_avlabl_tsk_0_3, tsk_cnt_rg_2);
    reg_load_balancing_rg_avlabl_tsk_0_3 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_3, reg_load_balancing_rg_avlabl_tsk_0_3, tsk_cnt_rg_3);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, reg_load_balancing_rg_avlabl_tsk_0_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get(uint8_t *tsk_cnt_rg_0, uint8_t *tsk_cnt_rg_1, uint8_t *tsk_cnt_rg_2, uint8_t *tsk_cnt_rg_3)
{
    uint32_t reg_load_balancing_rg_avlabl_tsk_0_3;

#ifdef VALIDATE_PARMS
    if(!tsk_cnt_rg_0 || !tsk_cnt_rg_1 || !tsk_cnt_rg_2 || !tsk_cnt_rg_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, reg_load_balancing_rg_avlabl_tsk_0_3);

    *tsk_cnt_rg_0 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_0, reg_load_balancing_rg_avlabl_tsk_0_3);
    *tsk_cnt_rg_1 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_1, reg_load_balancing_rg_avlabl_tsk_0_3);
    *tsk_cnt_rg_2 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_2, reg_load_balancing_rg_avlabl_tsk_0_3);
    *tsk_cnt_rg_3 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3, TSK_CNT_RG_3, reg_load_balancing_rg_avlabl_tsk_0_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(uint8_t tsk_cnt_rg_4, uint8_t tsk_cnt_rg_5, uint8_t tsk_cnt_rg_6, uint8_t tsk_cnt_rg_7)
{
    uint32_t reg_load_balancing_rg_avlabl_tsk_4_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_load_balancing_rg_avlabl_tsk_4_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_4, reg_load_balancing_rg_avlabl_tsk_4_7, tsk_cnt_rg_4);
    reg_load_balancing_rg_avlabl_tsk_4_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_5, reg_load_balancing_rg_avlabl_tsk_4_7, tsk_cnt_rg_5);
    reg_load_balancing_rg_avlabl_tsk_4_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_6, reg_load_balancing_rg_avlabl_tsk_4_7, tsk_cnt_rg_6);
    reg_load_balancing_rg_avlabl_tsk_4_7 = RU_FIELD_SET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_7, reg_load_balancing_rg_avlabl_tsk_4_7, tsk_cnt_rg_7);

    RU_REG_WRITE(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, reg_load_balancing_rg_avlabl_tsk_4_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get(uint8_t *tsk_cnt_rg_4, uint8_t *tsk_cnt_rg_5, uint8_t *tsk_cnt_rg_6, uint8_t *tsk_cnt_rg_7)
{
    uint32_t reg_load_balancing_rg_avlabl_tsk_4_7;

#ifdef VALIDATE_PARMS
    if(!tsk_cnt_rg_4 || !tsk_cnt_rg_5 || !tsk_cnt_rg_6 || !tsk_cnt_rg_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, reg_load_balancing_rg_avlabl_tsk_4_7);

    *tsk_cnt_rg_4 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_4, reg_load_balancing_rg_avlabl_tsk_4_7);
    *tsk_cnt_rg_5 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_5, reg_load_balancing_rg_avlabl_tsk_4_7);
    *tsk_cnt_rg_6 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_6, reg_load_balancing_rg_avlabl_tsk_4_7);
    *tsk_cnt_rg_7 = RU_FIELD_GET(0, DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7, TSK_CNT_RG_7, reg_load_balancing_rg_avlabl_tsk_4_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_isr=0;

#ifdef VALIDATE_PARMS
    if(!dsptcher_reordr_top_intr_ctrl_0_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dsptcher_reordr_top_intr_ctrl_0_isr->fll_return_buf >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_0_isr->fll_cnt_drp >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_0_isr->unknwn_msg >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_0_isr->fll_overflow >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_0_isr->fll_neg >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dsptcher_reordr_top_intr_ctrl_0_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_RETURN_BUF, reg_dsptcher_reordr_top_intr_ctrl_0_isr, dsptcher_reordr_top_intr_ctrl_0_isr->fll_return_buf);
    reg_dsptcher_reordr_top_intr_ctrl_0_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_CNT_DRP, reg_dsptcher_reordr_top_intr_ctrl_0_isr, dsptcher_reordr_top_intr_ctrl_0_isr->fll_cnt_drp);
    reg_dsptcher_reordr_top_intr_ctrl_0_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, UNKNWN_MSG, reg_dsptcher_reordr_top_intr_ctrl_0_isr, dsptcher_reordr_top_intr_ctrl_0_isr->unknwn_msg);
    reg_dsptcher_reordr_top_intr_ctrl_0_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_OVERFLOW, reg_dsptcher_reordr_top_intr_ctrl_0_isr, dsptcher_reordr_top_intr_ctrl_0_isr->fll_overflow);
    reg_dsptcher_reordr_top_intr_ctrl_0_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_NEG, reg_dsptcher_reordr_top_intr_ctrl_0_isr, dsptcher_reordr_top_intr_ctrl_0_isr->fll_neg);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, reg_dsptcher_reordr_top_intr_ctrl_0_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_isr;

#ifdef VALIDATE_PARMS
    if(!dsptcher_reordr_top_intr_ctrl_0_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, reg_dsptcher_reordr_top_intr_ctrl_0_isr);

    dsptcher_reordr_top_intr_ctrl_0_isr->fll_return_buf = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_RETURN_BUF, reg_dsptcher_reordr_top_intr_ctrl_0_isr);
    dsptcher_reordr_top_intr_ctrl_0_isr->fll_cnt_drp = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_CNT_DRP, reg_dsptcher_reordr_top_intr_ctrl_0_isr);
    dsptcher_reordr_top_intr_ctrl_0_isr->unknwn_msg = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, UNKNWN_MSG, reg_dsptcher_reordr_top_intr_ctrl_0_isr);
    dsptcher_reordr_top_intr_ctrl_0_isr->fll_overflow = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_OVERFLOW, reg_dsptcher_reordr_top_intr_ctrl_0_isr);
    dsptcher_reordr_top_intr_ctrl_0_isr->fll_neg = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR, FLL_NEG, reg_dsptcher_reordr_top_intr_ctrl_0_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get(uint32_t *ism)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_ism;

#ifdef VALIDATE_PARMS
    if(!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM, reg_dsptcher_reordr_top_intr_ctrl_0_ism);

    *ism = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM, ISM, reg_dsptcher_reordr_top_intr_ctrl_0_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set(uint32_t iem)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_ier=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dsptcher_reordr_top_intr_ctrl_0_ier = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER, IEM, reg_dsptcher_reordr_top_intr_ctrl_0_ier, iem);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER, reg_dsptcher_reordr_top_intr_ctrl_0_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get(uint32_t *iem)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_ier;

#ifdef VALIDATE_PARMS
    if(!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER, reg_dsptcher_reordr_top_intr_ctrl_0_ier);

    *iem = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER, IEM, reg_dsptcher_reordr_top_intr_ctrl_0_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set(uint32_t ist)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_itr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dsptcher_reordr_top_intr_ctrl_0_itr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR, IST, reg_dsptcher_reordr_top_intr_ctrl_0_itr, ist);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR, reg_dsptcher_reordr_top_intr_ctrl_0_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get(uint32_t *ist)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_0_itr;

#ifdef VALIDATE_PARMS
    if(!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR, reg_dsptcher_reordr_top_intr_ctrl_0_itr);

    *ist = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR, IST, reg_dsptcher_reordr_top_intr_ctrl_0_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_isr=0;

#ifdef VALIDATE_PARMS
    if(!dsptcher_reordr_top_intr_ctrl_1_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dsptcher_reordr_top_intr_ctrl_1_isr->qdest0_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest1_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest2_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest3_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest4_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest5_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest6_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest7_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest8_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest9_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest10_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest11_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest12_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest13_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest14_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest15_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest16_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest17_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest18_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest19_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest20_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest21_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest22_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest23_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest24_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest25_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest26_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest27_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest28_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest29_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest30_int >= _1BITS_MAX_VAL_) ||
       (dsptcher_reordr_top_intr_ctrl_1_isr->qdest31_int >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST0_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest0_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST1_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest1_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST2_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest2_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST3_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest3_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST4_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest4_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST5_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest5_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST6_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest6_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST7_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest7_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST8_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest8_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST9_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest9_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST10_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest10_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST11_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest11_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST12_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest12_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST13_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest13_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST14_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest14_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST15_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest15_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST16_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest16_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST17_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest17_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST18_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest18_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST19_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest19_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST20_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest20_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST21_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest21_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST22_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest22_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST23_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest23_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST24_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest24_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST25_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest25_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST26_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest26_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST27_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest27_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST28_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest28_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST29_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest29_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST30_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest30_int);
    reg_dsptcher_reordr_top_intr_ctrl_1_isr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST31_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr, dsptcher_reordr_top_intr_ctrl_1_isr->qdest31_int);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, reg_dsptcher_reordr_top_intr_ctrl_1_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_isr;

#ifdef VALIDATE_PARMS
    if(!dsptcher_reordr_top_intr_ctrl_1_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, reg_dsptcher_reordr_top_intr_ctrl_1_isr);

    dsptcher_reordr_top_intr_ctrl_1_isr->qdest0_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST0_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest1_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST1_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest2_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST2_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest3_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST3_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest4_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST4_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest5_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST5_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest6_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST6_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest7_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST7_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest8_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST8_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest9_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST9_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest10_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST10_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest11_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST11_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest12_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST12_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest13_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST13_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest14_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST14_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest15_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST15_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest16_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST16_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest17_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST17_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest18_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST18_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest19_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST19_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest20_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST20_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest21_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST21_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest22_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST22_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest23_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST23_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest24_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST24_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest25_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST25_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest26_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST26_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest27_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST27_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest28_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST28_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest29_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST29_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest30_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST30_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);
    dsptcher_reordr_top_intr_ctrl_1_isr->qdest31_int = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR, QDEST31_INT, reg_dsptcher_reordr_top_intr_ctrl_1_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get(uint32_t *ism)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_ism;

#ifdef VALIDATE_PARMS
    if(!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM, reg_dsptcher_reordr_top_intr_ctrl_1_ism);

    *ism = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM, ISM, reg_dsptcher_reordr_top_intr_ctrl_1_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set(uint32_t iem)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_ier=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dsptcher_reordr_top_intr_ctrl_1_ier = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER, IEM, reg_dsptcher_reordr_top_intr_ctrl_1_ier, iem);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER, reg_dsptcher_reordr_top_intr_ctrl_1_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get(uint32_t *iem)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_ier;

#ifdef VALIDATE_PARMS
    if(!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER, reg_dsptcher_reordr_top_intr_ctrl_1_ier);

    *iem = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER, IEM, reg_dsptcher_reordr_top_intr_ctrl_1_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set(uint32_t ist)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_itr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dsptcher_reordr_top_intr_ctrl_1_itr = RU_FIELD_SET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR, IST, reg_dsptcher_reordr_top_intr_ctrl_1_itr, ist);

    RU_REG_WRITE(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR, reg_dsptcher_reordr_top_intr_ctrl_1_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get(uint32_t *ist)
{
    uint32_t reg_dsptcher_reordr_top_intr_ctrl_1_itr;

#ifdef VALIDATE_PARMS
    if(!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR, reg_dsptcher_reordr_top_intr_ctrl_1_itr);

    *ist = RU_FIELD_GET(0, DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR, IST, reg_dsptcher_reordr_top_intr_ctrl_1_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_set(bdmf_boolean en_byp, uint8_t bbid_non_dly, uint8_t bbid_dly)
{
    uint32_t reg_debug_dbg_bypss_cntrl=0;

#ifdef VALIDATE_PARMS
    if((en_byp >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_dbg_bypss_cntrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, EN_BYP, reg_debug_dbg_bypss_cntrl, en_byp);
    reg_debug_dbg_bypss_cntrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, BBID_NON_DLY, reg_debug_dbg_bypss_cntrl, bbid_non_dly);
    reg_debug_dbg_bypss_cntrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, BBID_DLY, reg_debug_dbg_bypss_cntrl, bbid_dly);

    RU_REG_WRITE(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, reg_debug_dbg_bypss_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_get(bdmf_boolean *en_byp, uint8_t *bbid_non_dly, uint8_t *bbid_dly)
{
    uint32_t reg_debug_dbg_bypss_cntrl;

#ifdef VALIDATE_PARMS
    if(!en_byp || !bbid_non_dly || !bbid_dly)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, reg_debug_dbg_bypss_cntrl);

    *en_byp = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, EN_BYP, reg_debug_dbg_bypss_cntrl);
    *bbid_non_dly = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, BBID_NON_DLY, reg_debug_dbg_bypss_cntrl);
    *bbid_dly = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_BYPSS_CNTRL, BBID_DLY, reg_debug_dbg_bypss_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set(const dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7)
{
    uint32_t reg_debug_glbl_tsk_cnt_0_7=0;

#ifdef VALIDATE_PARMS
    if(!debug_glbl_tsk_cnt_0_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_0 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_1 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_2 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_3 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_4 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_5 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_6 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_7 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_0, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_0);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_1, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_1);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_2, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_2);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_3, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_3);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_4, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_4);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_5, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_5);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_6, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_6);
    reg_debug_glbl_tsk_cnt_0_7 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_7, reg_debug_glbl_tsk_cnt_0_7, debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_7);

    RU_REG_WRITE(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, reg_debug_glbl_tsk_cnt_0_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get(dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7)
{
    uint32_t reg_debug_glbl_tsk_cnt_0_7;

#ifdef VALIDATE_PARMS
    if(!debug_glbl_tsk_cnt_0_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, reg_debug_glbl_tsk_cnt_0_7);

    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_0 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_0, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_1 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_1, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_2 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_2, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_3 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_3, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_4 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_4, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_5 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_5, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_6 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_6, reg_debug_glbl_tsk_cnt_0_7);
    debug_glbl_tsk_cnt_0_7->tsk_cnt_rnr_7 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7, TSK_CNT_RNR_7, reg_debug_glbl_tsk_cnt_0_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set(const dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15)
{
    uint32_t reg_debug_glbl_tsk_cnt_8_15=0;

#ifdef VALIDATE_PARMS
    if(!debug_glbl_tsk_cnt_8_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_8 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_9 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_10 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_11 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_12 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_13 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_14 >= _4BITS_MAX_VAL_) ||
       (debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_15 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_8, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_8);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_9, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_9);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_10, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_10);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_11, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_11);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_12, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_12);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_13, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_13);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_14, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_14);
    reg_debug_glbl_tsk_cnt_8_15 = RU_FIELD_SET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_15, reg_debug_glbl_tsk_cnt_8_15, debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_15);

    RU_REG_WRITE(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, reg_debug_glbl_tsk_cnt_8_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get(dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15)
{
    uint32_t reg_debug_glbl_tsk_cnt_8_15;

#ifdef VALIDATE_PARMS
    if(!debug_glbl_tsk_cnt_8_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, reg_debug_glbl_tsk_cnt_8_15);

    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_8 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_8, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_9 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_9, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_10 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_10, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_11 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_11, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_12 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_12, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_13 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_13, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_14 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_14, reg_debug_glbl_tsk_cnt_8_15);
    debug_glbl_tsk_cnt_8_15->tsk_cnt_rnr_15 = RU_FIELD_GET(0, DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15, TSK_CNT_RNR_15, reg_debug_glbl_tsk_cnt_8_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_set(uint8_t dbg_sel)
{
    uint32_t reg_debug_dbg_bus_cntrl=0;

#ifdef VALIDATE_PARMS
    if((dbg_sel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_dbg_bus_cntrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_DBG_BUS_CNTRL, DBG_SEL, reg_debug_dbg_bus_cntrl, dbg_sel);

    RU_REG_WRITE(0, DSPTCHR, DEBUG_DBG_BUS_CNTRL, reg_debug_dbg_bus_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_get(uint8_t *dbg_sel)
{
    uint32_t reg_debug_dbg_bus_cntrl;

#ifdef VALIDATE_PARMS
    if(!dbg_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_BUS_CNTRL, reg_debug_dbg_bus_cntrl);

    *dbg_sel = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_BUS_CNTRL, DBG_SEL, reg_debug_dbg_bus_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_0_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_0;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_0, reg_debug_dbg_vec_0);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_0, DBG_VEC_VAL, reg_debug_dbg_vec_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_1_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_1;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_1, reg_debug_dbg_vec_1);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_1, DBG_VEC_VAL, reg_debug_dbg_vec_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_2_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_2;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_2, reg_debug_dbg_vec_2);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_2, DBG_VEC_VAL, reg_debug_dbg_vec_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_3_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_3;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_3, reg_debug_dbg_vec_3);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_3, DBG_VEC_VAL, reg_debug_dbg_vec_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_4_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_4;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_4, reg_debug_dbg_vec_4);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_4, DBG_VEC_VAL, reg_debug_dbg_vec_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_5_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_5;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_5, reg_debug_dbg_vec_5);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_5, DBG_VEC_VAL, reg_debug_dbg_vec_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_6_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_6;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_6, reg_debug_dbg_vec_6);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_6, DBG_VEC_VAL, reg_debug_dbg_vec_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_7_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_7;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_7, reg_debug_dbg_vec_7);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_7, DBG_VEC_VAL, reg_debug_dbg_vec_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_8_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_8;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_8, reg_debug_dbg_vec_8);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_8, DBG_VEC_VAL, reg_debug_dbg_vec_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_9_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_9;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_9, reg_debug_dbg_vec_9);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_9, DBG_VEC_VAL, reg_debug_dbg_vec_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_10_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_10;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_10, reg_debug_dbg_vec_10);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_10, DBG_VEC_VAL, reg_debug_dbg_vec_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_11_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_11;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_11, reg_debug_dbg_vec_11);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_11, DBG_VEC_VAL, reg_debug_dbg_vec_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_12_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_12;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_12, reg_debug_dbg_vec_12);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_12, DBG_VEC_VAL, reg_debug_dbg_vec_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_13_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_13;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_13, reg_debug_dbg_vec_13);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_13, DBG_VEC_VAL, reg_debug_dbg_vec_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_14_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_14;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_14, reg_debug_dbg_vec_14);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_14, DBG_VEC_VAL, reg_debug_dbg_vec_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_15_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_15;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_15, reg_debug_dbg_vec_15);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_15, DBG_VEC_VAL, reg_debug_dbg_vec_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_16_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_16;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_16, reg_debug_dbg_vec_16);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_16, DBG_VEC_VAL, reg_debug_dbg_vec_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_17_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_17;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_17, reg_debug_dbg_vec_17);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_17, DBG_VEC_VAL, reg_debug_dbg_vec_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_18_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_18;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_18, reg_debug_dbg_vec_18);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_18, DBG_VEC_VAL, reg_debug_dbg_vec_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_19_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_19;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_19, reg_debug_dbg_vec_19);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_19, DBG_VEC_VAL, reg_debug_dbg_vec_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_20_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_20;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_20, reg_debug_dbg_vec_20);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_20, DBG_VEC_VAL, reg_debug_dbg_vec_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_21_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_21;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_21, reg_debug_dbg_vec_21);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_21, DBG_VEC_VAL, reg_debug_dbg_vec_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_22_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_22;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_22, reg_debug_dbg_vec_22);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_22, DBG_VEC_VAL, reg_debug_dbg_vec_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_23_get(uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_dbg_vec_23;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_DBG_VEC_23, reg_debug_dbg_vec_23);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_DBG_VEC_23, DBG_VEC_VAL, reg_debug_dbg_vec_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set(uint8_t dbg_mode, bdmf_boolean en_cntrs, bdmf_boolean clr_cntrs, uint8_t dbg_rnr_sel)
{
    uint32_t reg_debug_statistics_dbg_sttstcs_ctrl=0;

#ifdef VALIDATE_PARMS
    if((dbg_mode >= _2BITS_MAX_VAL_) ||
       (en_cntrs >= _1BITS_MAX_VAL_) ||
       (clr_cntrs >= _1BITS_MAX_VAL_) ||
       (dbg_rnr_sel >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_statistics_dbg_sttstcs_ctrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, DBG_MODE, reg_debug_statistics_dbg_sttstcs_ctrl, dbg_mode);
    reg_debug_statistics_dbg_sttstcs_ctrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, EN_CNTRS, reg_debug_statistics_dbg_sttstcs_ctrl, en_cntrs);
    reg_debug_statistics_dbg_sttstcs_ctrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, CLR_CNTRS, reg_debug_statistics_dbg_sttstcs_ctrl, clr_cntrs);
    reg_debug_statistics_dbg_sttstcs_ctrl = RU_FIELD_SET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, DBG_RNR_SEL, reg_debug_statistics_dbg_sttstcs_ctrl, dbg_rnr_sel);

    RU_REG_WRITE(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, reg_debug_statistics_dbg_sttstcs_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get(uint8_t *dbg_mode, bdmf_boolean *en_cntrs, bdmf_boolean *clr_cntrs, uint8_t *dbg_rnr_sel)
{
    uint32_t reg_debug_statistics_dbg_sttstcs_ctrl;

#ifdef VALIDATE_PARMS
    if(!dbg_mode || !en_cntrs || !clr_cntrs || !dbg_rnr_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, reg_debug_statistics_dbg_sttstcs_ctrl);

    *dbg_mode = RU_FIELD_GET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, DBG_MODE, reg_debug_statistics_dbg_sttstcs_ctrl);
    *en_cntrs = RU_FIELD_GET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, EN_CNTRS, reg_debug_statistics_dbg_sttstcs_ctrl);
    *clr_cntrs = RU_FIELD_GET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, CLR_CNTRS, reg_debug_statistics_dbg_sttstcs_ctrl);
    *dbg_rnr_sel = RU_FIELD_GET(0, DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL, DBG_RNR_SEL, reg_debug_statistics_dbg_sttstcs_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_cnt_get(uint8_t index, uint32_t *dbg_vec_val)
{
    uint32_t reg_debug_statistics_dbg_cnt;

#ifdef VALIDATE_PARMS
    if(!dbg_vec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((index >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, DSPTCHR, DEBUG_STATISTICS_DBG_CNT, reg_debug_statistics_dbg_cnt);

    *dbg_vec_val = RU_FIELD_GET(0, DSPTCHR, DEBUG_STATISTICS_DBG_CNT, DBG_VEC_VAL, reg_debug_statistics_dbg_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_head_set(uint8_t q_idx, uint32_t head)
{
    uint32_t reg_qdes_head=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qdes_head = RU_FIELD_SET(0, DSPTCHR, QDES_HEAD, HEAD, reg_qdes_head, head);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, QDES_HEAD, reg_qdes_head);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_head_get(uint8_t q_idx, uint32_t *head)
{
    uint32_t reg_qdes_head;

#ifdef VALIDATE_PARMS
    if(!head)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, QDES_HEAD, reg_qdes_head);

    *head = RU_FIELD_GET(0, DSPTCHR, QDES_HEAD, HEAD, reg_qdes_head);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_bfout_set(uint8_t zero, const dsptchr_qdes_bfout *qdes_bfout)
{
#ifdef VALIDATE_PARMS
    if(!qdes_bfout)
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

    RU_REG_RAM_WRITE(0, zero *32 + 0, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[0]);
    RU_REG_RAM_WRITE(0, zero *32 + 1, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[1]);
    RU_REG_RAM_WRITE(0, zero *32 + 2, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[2]);
    RU_REG_RAM_WRITE(0, zero *32 + 3, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[3]);
    RU_REG_RAM_WRITE(0, zero *32 + 4, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[4]);
    RU_REG_RAM_WRITE(0, zero *32 + 5, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[5]);
    RU_REG_RAM_WRITE(0, zero *32 + 6, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[6]);
    RU_REG_RAM_WRITE(0, zero *32 + 7, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[7]);
    RU_REG_RAM_WRITE(0, zero *32 + 8, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[8]);
    RU_REG_RAM_WRITE(0, zero *32 + 9, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[9]);
    RU_REG_RAM_WRITE(0, zero *32 + 10, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[10]);
    RU_REG_RAM_WRITE(0, zero *32 + 11, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[11]);
    RU_REG_RAM_WRITE(0, zero *32 + 12, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[12]);
    RU_REG_RAM_WRITE(0, zero *32 + 13, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[13]);
    RU_REG_RAM_WRITE(0, zero *32 + 14, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[14]);
    RU_REG_RAM_WRITE(0, zero *32 + 15, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[15]);
    RU_REG_RAM_WRITE(0, zero *32 + 16, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[16]);
    RU_REG_RAM_WRITE(0, zero *32 + 17, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[17]);
    RU_REG_RAM_WRITE(0, zero *32 + 18, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[18]);
    RU_REG_RAM_WRITE(0, zero *32 + 19, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[19]);
    RU_REG_RAM_WRITE(0, zero *32 + 20, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[20]);
    RU_REG_RAM_WRITE(0, zero *32 + 21, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[21]);
    RU_REG_RAM_WRITE(0, zero *32 + 22, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[22]);
    RU_REG_RAM_WRITE(0, zero *32 + 23, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[23]);
    RU_REG_RAM_WRITE(0, zero *32 + 24, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[24]);
    RU_REG_RAM_WRITE(0, zero *32 + 25, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[25]);
    RU_REG_RAM_WRITE(0, zero *32 + 26, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[26]);
    RU_REG_RAM_WRITE(0, zero *32 + 27, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[27]);
    RU_REG_RAM_WRITE(0, zero *32 + 28, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[28]);
    RU_REG_RAM_WRITE(0, zero *32 + 29, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[29]);
    RU_REG_RAM_WRITE(0, zero *32 + 30, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[30]);
    RU_REG_RAM_WRITE(0, zero *32 + 31, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_bfout_get(uint8_t zero, dsptchr_qdes_bfout *qdes_bfout)
{
#ifdef VALIDATE_PARMS
    if(!qdes_bfout)
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

    RU_REG_RAM_READ(0, zero *32 + 0, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[0]);
    RU_REG_RAM_READ(0, zero *32 + 1, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[1]);
    RU_REG_RAM_READ(0, zero *32 + 2, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[2]);
    RU_REG_RAM_READ(0, zero *32 + 3, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[3]);
    RU_REG_RAM_READ(0, zero *32 + 4, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[4]);
    RU_REG_RAM_READ(0, zero *32 + 5, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[5]);
    RU_REG_RAM_READ(0, zero *32 + 6, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[6]);
    RU_REG_RAM_READ(0, zero *32 + 7, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[7]);
    RU_REG_RAM_READ(0, zero *32 + 8, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[8]);
    RU_REG_RAM_READ(0, zero *32 + 9, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[9]);
    RU_REG_RAM_READ(0, zero *32 + 10, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[10]);
    RU_REG_RAM_READ(0, zero *32 + 11, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[11]);
    RU_REG_RAM_READ(0, zero *32 + 12, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[12]);
    RU_REG_RAM_READ(0, zero *32 + 13, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[13]);
    RU_REG_RAM_READ(0, zero *32 + 14, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[14]);
    RU_REG_RAM_READ(0, zero *32 + 15, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[15]);
    RU_REG_RAM_READ(0, zero *32 + 16, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[16]);
    RU_REG_RAM_READ(0, zero *32 + 17, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[17]);
    RU_REG_RAM_READ(0, zero *32 + 18, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[18]);
    RU_REG_RAM_READ(0, zero *32 + 19, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[19]);
    RU_REG_RAM_READ(0, zero *32 + 20, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[20]);
    RU_REG_RAM_READ(0, zero *32 + 21, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[21]);
    RU_REG_RAM_READ(0, zero *32 + 22, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[22]);
    RU_REG_RAM_READ(0, zero *32 + 23, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[23]);
    RU_REG_RAM_READ(0, zero *32 + 24, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[24]);
    RU_REG_RAM_READ(0, zero *32 + 25, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[25]);
    RU_REG_RAM_READ(0, zero *32 + 26, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[26]);
    RU_REG_RAM_READ(0, zero *32 + 27, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[27]);
    RU_REG_RAM_READ(0, zero *32 + 28, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[28]);
    RU_REG_RAM_READ(0, zero *32 + 29, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[29]);
    RU_REG_RAM_READ(0, zero *32 + 30, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[30]);
    RU_REG_RAM_READ(0, zero *32 + 31, DSPTCHR, QDES_BFOUT, qdes_bfout->qdes_bfout[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_bufin_set(uint8_t zero, const dsptchr_qdes_bufin *qdes_bufin)
{
#ifdef VALIDATE_PARMS
    if(!qdes_bufin)
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

    RU_REG_RAM_WRITE(0, zero *32 + 0, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[0]);
    RU_REG_RAM_WRITE(0, zero *32 + 1, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[1]);
    RU_REG_RAM_WRITE(0, zero *32 + 2, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[2]);
    RU_REG_RAM_WRITE(0, zero *32 + 3, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[3]);
    RU_REG_RAM_WRITE(0, zero *32 + 4, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[4]);
    RU_REG_RAM_WRITE(0, zero *32 + 5, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[5]);
    RU_REG_RAM_WRITE(0, zero *32 + 6, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[6]);
    RU_REG_RAM_WRITE(0, zero *32 + 7, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[7]);
    RU_REG_RAM_WRITE(0, zero *32 + 8, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[8]);
    RU_REG_RAM_WRITE(0, zero *32 + 9, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[9]);
    RU_REG_RAM_WRITE(0, zero *32 + 10, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[10]);
    RU_REG_RAM_WRITE(0, zero *32 + 11, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[11]);
    RU_REG_RAM_WRITE(0, zero *32 + 12, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[12]);
    RU_REG_RAM_WRITE(0, zero *32 + 13, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[13]);
    RU_REG_RAM_WRITE(0, zero *32 + 14, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[14]);
    RU_REG_RAM_WRITE(0, zero *32 + 15, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[15]);
    RU_REG_RAM_WRITE(0, zero *32 + 16, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[16]);
    RU_REG_RAM_WRITE(0, zero *32 + 17, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[17]);
    RU_REG_RAM_WRITE(0, zero *32 + 18, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[18]);
    RU_REG_RAM_WRITE(0, zero *32 + 19, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[19]);
    RU_REG_RAM_WRITE(0, zero *32 + 20, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[20]);
    RU_REG_RAM_WRITE(0, zero *32 + 21, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[21]);
    RU_REG_RAM_WRITE(0, zero *32 + 22, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[22]);
    RU_REG_RAM_WRITE(0, zero *32 + 23, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[23]);
    RU_REG_RAM_WRITE(0, zero *32 + 24, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[24]);
    RU_REG_RAM_WRITE(0, zero *32 + 25, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[25]);
    RU_REG_RAM_WRITE(0, zero *32 + 26, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[26]);
    RU_REG_RAM_WRITE(0, zero *32 + 27, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[27]);
    RU_REG_RAM_WRITE(0, zero *32 + 28, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[28]);
    RU_REG_RAM_WRITE(0, zero *32 + 29, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[29]);
    RU_REG_RAM_WRITE(0, zero *32 + 30, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[30]);
    RU_REG_RAM_WRITE(0, zero *32 + 31, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_bufin_get(uint8_t zero, dsptchr_qdes_bufin *qdes_bufin)
{
#ifdef VALIDATE_PARMS
    if(!qdes_bufin)
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

    RU_REG_RAM_READ(0, zero *32 + 0, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[0]);
    RU_REG_RAM_READ(0, zero *32 + 1, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[1]);
    RU_REG_RAM_READ(0, zero *32 + 2, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[2]);
    RU_REG_RAM_READ(0, zero *32 + 3, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[3]);
    RU_REG_RAM_READ(0, zero *32 + 4, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[4]);
    RU_REG_RAM_READ(0, zero *32 + 5, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[5]);
    RU_REG_RAM_READ(0, zero *32 + 6, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[6]);
    RU_REG_RAM_READ(0, zero *32 + 7, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[7]);
    RU_REG_RAM_READ(0, zero *32 + 8, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[8]);
    RU_REG_RAM_READ(0, zero *32 + 9, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[9]);
    RU_REG_RAM_READ(0, zero *32 + 10, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[10]);
    RU_REG_RAM_READ(0, zero *32 + 11, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[11]);
    RU_REG_RAM_READ(0, zero *32 + 12, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[12]);
    RU_REG_RAM_READ(0, zero *32 + 13, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[13]);
    RU_REG_RAM_READ(0, zero *32 + 14, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[14]);
    RU_REG_RAM_READ(0, zero *32 + 15, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[15]);
    RU_REG_RAM_READ(0, zero *32 + 16, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[16]);
    RU_REG_RAM_READ(0, zero *32 + 17, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[17]);
    RU_REG_RAM_READ(0, zero *32 + 18, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[18]);
    RU_REG_RAM_READ(0, zero *32 + 19, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[19]);
    RU_REG_RAM_READ(0, zero *32 + 20, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[20]);
    RU_REG_RAM_READ(0, zero *32 + 21, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[21]);
    RU_REG_RAM_READ(0, zero *32 + 22, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[22]);
    RU_REG_RAM_READ(0, zero *32 + 23, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[23]);
    RU_REG_RAM_READ(0, zero *32 + 24, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[24]);
    RU_REG_RAM_READ(0, zero *32 + 25, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[25]);
    RU_REG_RAM_READ(0, zero *32 + 26, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[26]);
    RU_REG_RAM_READ(0, zero *32 + 27, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[27]);
    RU_REG_RAM_READ(0, zero *32 + 28, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[28]);
    RU_REG_RAM_READ(0, zero *32 + 29, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[29]);
    RU_REG_RAM_READ(0, zero *32 + 30, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[30]);
    RU_REG_RAM_READ(0, zero *32 + 31, DSPTCHR, QDES_BUFIN, qdes_bufin->qdes_bfin[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_tail_set(uint8_t q_idx, uint32_t tail)
{
    uint32_t reg_qdes_tail=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qdes_tail = RU_FIELD_SET(0, DSPTCHR, QDES_TAIL, TAIL, reg_qdes_tail, tail);

    RU_REG_RAM_WRITE(0, q_idx, DSPTCHR, QDES_TAIL, reg_qdes_tail);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_tail_get(uint8_t q_idx, uint32_t *tail)
{
    uint32_t reg_qdes_tail;

#ifdef VALIDATE_PARMS
    if(!tail)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, DSPTCHR, QDES_TAIL, reg_qdes_tail);

    *tail = RU_FIELD_GET(0, DSPTCHR, QDES_TAIL, TAIL, reg_qdes_tail);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_set(uint8_t zero, const dsptchr_qdes_fbdnull *qdes_fbdnull)
{
#ifdef VALIDATE_PARMS
    if(!qdes_fbdnull)
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

    RU_REG_RAM_WRITE(0, zero *32 + 0, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[0]);
    RU_REG_RAM_WRITE(0, zero *32 + 1, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[1]);
    RU_REG_RAM_WRITE(0, zero *32 + 2, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[2]);
    RU_REG_RAM_WRITE(0, zero *32 + 3, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[3]);
    RU_REG_RAM_WRITE(0, zero *32 + 4, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[4]);
    RU_REG_RAM_WRITE(0, zero *32 + 5, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[5]);
    RU_REG_RAM_WRITE(0, zero *32 + 6, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[6]);
    RU_REG_RAM_WRITE(0, zero *32 + 7, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[7]);
    RU_REG_RAM_WRITE(0, zero *32 + 8, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[8]);
    RU_REG_RAM_WRITE(0, zero *32 + 9, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[9]);
    RU_REG_RAM_WRITE(0, zero *32 + 10, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[10]);
    RU_REG_RAM_WRITE(0, zero *32 + 11, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[11]);
    RU_REG_RAM_WRITE(0, zero *32 + 12, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[12]);
    RU_REG_RAM_WRITE(0, zero *32 + 13, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[13]);
    RU_REG_RAM_WRITE(0, zero *32 + 14, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[14]);
    RU_REG_RAM_WRITE(0, zero *32 + 15, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[15]);
    RU_REG_RAM_WRITE(0, zero *32 + 16, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[16]);
    RU_REG_RAM_WRITE(0, zero *32 + 17, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[17]);
    RU_REG_RAM_WRITE(0, zero *32 + 18, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[18]);
    RU_REG_RAM_WRITE(0, zero *32 + 19, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[19]);
    RU_REG_RAM_WRITE(0, zero *32 + 20, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[20]);
    RU_REG_RAM_WRITE(0, zero *32 + 21, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[21]);
    RU_REG_RAM_WRITE(0, zero *32 + 22, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[22]);
    RU_REG_RAM_WRITE(0, zero *32 + 23, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[23]);
    RU_REG_RAM_WRITE(0, zero *32 + 24, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[24]);
    RU_REG_RAM_WRITE(0, zero *32 + 25, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[25]);
    RU_REG_RAM_WRITE(0, zero *32 + 26, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[26]);
    RU_REG_RAM_WRITE(0, zero *32 + 27, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[27]);
    RU_REG_RAM_WRITE(0, zero *32 + 28, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[28]);
    RU_REG_RAM_WRITE(0, zero *32 + 29, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[29]);
    RU_REG_RAM_WRITE(0, zero *32 + 30, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[30]);
    RU_REG_RAM_WRITE(0, zero *32 + 31, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_get(uint8_t zero, dsptchr_qdes_fbdnull *qdes_fbdnull)
{
#ifdef VALIDATE_PARMS
    if(!qdes_fbdnull)
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

    RU_REG_RAM_READ(0, zero *32 + 0, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[0]);
    RU_REG_RAM_READ(0, zero *32 + 1, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[1]);
    RU_REG_RAM_READ(0, zero *32 + 2, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[2]);
    RU_REG_RAM_READ(0, zero *32 + 3, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[3]);
    RU_REG_RAM_READ(0, zero *32 + 4, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[4]);
    RU_REG_RAM_READ(0, zero *32 + 5, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[5]);
    RU_REG_RAM_READ(0, zero *32 + 6, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[6]);
    RU_REG_RAM_READ(0, zero *32 + 7, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[7]);
    RU_REG_RAM_READ(0, zero *32 + 8, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[8]);
    RU_REG_RAM_READ(0, zero *32 + 9, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[9]);
    RU_REG_RAM_READ(0, zero *32 + 10, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[10]);
    RU_REG_RAM_READ(0, zero *32 + 11, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[11]);
    RU_REG_RAM_READ(0, zero *32 + 12, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[12]);
    RU_REG_RAM_READ(0, zero *32 + 13, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[13]);
    RU_REG_RAM_READ(0, zero *32 + 14, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[14]);
    RU_REG_RAM_READ(0, zero *32 + 15, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[15]);
    RU_REG_RAM_READ(0, zero *32 + 16, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[16]);
    RU_REG_RAM_READ(0, zero *32 + 17, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[17]);
    RU_REG_RAM_READ(0, zero *32 + 18, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[18]);
    RU_REG_RAM_READ(0, zero *32 + 19, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[19]);
    RU_REG_RAM_READ(0, zero *32 + 20, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[20]);
    RU_REG_RAM_READ(0, zero *32 + 21, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[21]);
    RU_REG_RAM_READ(0, zero *32 + 22, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[22]);
    RU_REG_RAM_READ(0, zero *32 + 23, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[23]);
    RU_REG_RAM_READ(0, zero *32 + 24, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[24]);
    RU_REG_RAM_READ(0, zero *32 + 25, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[25]);
    RU_REG_RAM_READ(0, zero *32 + 26, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[26]);
    RU_REG_RAM_READ(0, zero *32 + 27, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[27]);
    RU_REG_RAM_READ(0, zero *32 + 28, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[28]);
    RU_REG_RAM_READ(0, zero *32 + 29, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[29]);
    RU_REG_RAM_READ(0, zero *32 + 30, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[30]);
    RU_REG_RAM_READ(0, zero *32 + 31, DSPTCHR, QDES_FBDNULL, qdes_fbdnull->qdes_fbdnull[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_nullbd_set(uint8_t zero, const dsptchr_qdes_nullbd *qdes_nullbd)
{
#ifdef VALIDATE_PARMS
    if(!qdes_nullbd)
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

    RU_REG_RAM_WRITE(0, zero *32 + 0, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[0]);
    RU_REG_RAM_WRITE(0, zero *32 + 1, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[1]);
    RU_REG_RAM_WRITE(0, zero *32 + 2, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[2]);
    RU_REG_RAM_WRITE(0, zero *32 + 3, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[3]);
    RU_REG_RAM_WRITE(0, zero *32 + 4, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[4]);
    RU_REG_RAM_WRITE(0, zero *32 + 5, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[5]);
    RU_REG_RAM_WRITE(0, zero *32 + 6, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[6]);
    RU_REG_RAM_WRITE(0, zero *32 + 7, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[7]);
    RU_REG_RAM_WRITE(0, zero *32 + 8, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[8]);
    RU_REG_RAM_WRITE(0, zero *32 + 9, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[9]);
    RU_REG_RAM_WRITE(0, zero *32 + 10, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[10]);
    RU_REG_RAM_WRITE(0, zero *32 + 11, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[11]);
    RU_REG_RAM_WRITE(0, zero *32 + 12, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[12]);
    RU_REG_RAM_WRITE(0, zero *32 + 13, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[13]);
    RU_REG_RAM_WRITE(0, zero *32 + 14, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[14]);
    RU_REG_RAM_WRITE(0, zero *32 + 15, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[15]);
    RU_REG_RAM_WRITE(0, zero *32 + 16, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[16]);
    RU_REG_RAM_WRITE(0, zero *32 + 17, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[17]);
    RU_REG_RAM_WRITE(0, zero *32 + 18, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[18]);
    RU_REG_RAM_WRITE(0, zero *32 + 19, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[19]);
    RU_REG_RAM_WRITE(0, zero *32 + 20, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[20]);
    RU_REG_RAM_WRITE(0, zero *32 + 21, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[21]);
    RU_REG_RAM_WRITE(0, zero *32 + 22, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[22]);
    RU_REG_RAM_WRITE(0, zero *32 + 23, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[23]);
    RU_REG_RAM_WRITE(0, zero *32 + 24, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[24]);
    RU_REG_RAM_WRITE(0, zero *32 + 25, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[25]);
    RU_REG_RAM_WRITE(0, zero *32 + 26, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[26]);
    RU_REG_RAM_WRITE(0, zero *32 + 27, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[27]);
    RU_REG_RAM_WRITE(0, zero *32 + 28, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[28]);
    RU_REG_RAM_WRITE(0, zero *32 + 29, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[29]);
    RU_REG_RAM_WRITE(0, zero *32 + 30, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[30]);
    RU_REG_RAM_WRITE(0, zero *32 + 31, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_nullbd_get(uint8_t zero, dsptchr_qdes_nullbd *qdes_nullbd)
{
#ifdef VALIDATE_PARMS
    if(!qdes_nullbd)
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

    RU_REG_RAM_READ(0, zero *32 + 0, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[0]);
    RU_REG_RAM_READ(0, zero *32 + 1, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[1]);
    RU_REG_RAM_READ(0, zero *32 + 2, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[2]);
    RU_REG_RAM_READ(0, zero *32 + 3, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[3]);
    RU_REG_RAM_READ(0, zero *32 + 4, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[4]);
    RU_REG_RAM_READ(0, zero *32 + 5, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[5]);
    RU_REG_RAM_READ(0, zero *32 + 6, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[6]);
    RU_REG_RAM_READ(0, zero *32 + 7, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[7]);
    RU_REG_RAM_READ(0, zero *32 + 8, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[8]);
    RU_REG_RAM_READ(0, zero *32 + 9, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[9]);
    RU_REG_RAM_READ(0, zero *32 + 10, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[10]);
    RU_REG_RAM_READ(0, zero *32 + 11, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[11]);
    RU_REG_RAM_READ(0, zero *32 + 12, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[12]);
    RU_REG_RAM_READ(0, zero *32 + 13, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[13]);
    RU_REG_RAM_READ(0, zero *32 + 14, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[14]);
    RU_REG_RAM_READ(0, zero *32 + 15, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[15]);
    RU_REG_RAM_READ(0, zero *32 + 16, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[16]);
    RU_REG_RAM_READ(0, zero *32 + 17, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[17]);
    RU_REG_RAM_READ(0, zero *32 + 18, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[18]);
    RU_REG_RAM_READ(0, zero *32 + 19, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[19]);
    RU_REG_RAM_READ(0, zero *32 + 20, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[20]);
    RU_REG_RAM_READ(0, zero *32 + 21, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[21]);
    RU_REG_RAM_READ(0, zero *32 + 22, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[22]);
    RU_REG_RAM_READ(0, zero *32 + 23, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[23]);
    RU_REG_RAM_READ(0, zero *32 + 24, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[24]);
    RU_REG_RAM_READ(0, zero *32 + 25, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[25]);
    RU_REG_RAM_READ(0, zero *32 + 26, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[26]);
    RU_REG_RAM_READ(0, zero *32 + 27, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[27]);
    RU_REG_RAM_READ(0, zero *32 + 28, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[28]);
    RU_REG_RAM_READ(0, zero *32 + 29, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[29]);
    RU_REG_RAM_READ(0, zero *32 + 30, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[30]);
    RU_REG_RAM_READ(0, zero *32 + 31, DSPTCHR, QDES_NULLBD, qdes_nullbd->qdes_nullbd[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_bufavail_get(uint8_t zero, dsptchr_qdes_bufavail *qdes_bufavail)
{
#ifdef VALIDATE_PARMS
    if(!qdes_bufavail)
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

    RU_REG_RAM_READ(0, zero *32 + 0, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[0]);
    RU_REG_RAM_READ(0, zero *32 + 1, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[1]);
    RU_REG_RAM_READ(0, zero *32 + 2, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[2]);
    RU_REG_RAM_READ(0, zero *32 + 3, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[3]);
    RU_REG_RAM_READ(0, zero *32 + 4, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[4]);
    RU_REG_RAM_READ(0, zero *32 + 5, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[5]);
    RU_REG_RAM_READ(0, zero *32 + 6, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[6]);
    RU_REG_RAM_READ(0, zero *32 + 7, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[7]);
    RU_REG_RAM_READ(0, zero *32 + 8, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[8]);
    RU_REG_RAM_READ(0, zero *32 + 9, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[9]);
    RU_REG_RAM_READ(0, zero *32 + 10, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[10]);
    RU_REG_RAM_READ(0, zero *32 + 11, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[11]);
    RU_REG_RAM_READ(0, zero *32 + 12, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[12]);
    RU_REG_RAM_READ(0, zero *32 + 13, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[13]);
    RU_REG_RAM_READ(0, zero *32 + 14, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[14]);
    RU_REG_RAM_READ(0, zero *32 + 15, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[15]);
    RU_REG_RAM_READ(0, zero *32 + 16, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[16]);
    RU_REG_RAM_READ(0, zero *32 + 17, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[17]);
    RU_REG_RAM_READ(0, zero *32 + 18, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[18]);
    RU_REG_RAM_READ(0, zero *32 + 19, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[19]);
    RU_REG_RAM_READ(0, zero *32 + 20, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[20]);
    RU_REG_RAM_READ(0, zero *32 + 21, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[21]);
    RU_REG_RAM_READ(0, zero *32 + 22, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[22]);
    RU_REG_RAM_READ(0, zero *32 + 23, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[23]);
    RU_REG_RAM_READ(0, zero *32 + 24, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[24]);
    RU_REG_RAM_READ(0, zero *32 + 25, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[25]);
    RU_REG_RAM_READ(0, zero *32 + 26, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[26]);
    RU_REG_RAM_READ(0, zero *32 + 27, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[27]);
    RU_REG_RAM_READ(0, zero *32 + 28, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[28]);
    RU_REG_RAM_READ(0, zero *32 + 29, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[29]);
    RU_REG_RAM_READ(0, zero *32 + 30, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[30]);
    RU_REG_RAM_READ(0, zero *32 + 31, DSPTCHR, QDES_BUFAVAIL, qdes_bufavail->qdes_bufavail[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_set(uint8_t q_head_idx, uint16_t head)
{
    uint32_t reg_qdes_reg_q_head=0;

#ifdef VALIDATE_PARMS
    if((q_head_idx >= 32) ||
       (head >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qdes_reg_q_head = RU_FIELD_SET(0, DSPTCHR, QDES_REG_Q_HEAD, HEAD, reg_qdes_reg_q_head, head);

    RU_REG_RAM_WRITE(0, q_head_idx, DSPTCHR, QDES_REG_Q_HEAD, reg_qdes_reg_q_head);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_get(uint8_t q_head_idx, uint16_t *head)
{
    uint32_t reg_qdes_reg_q_head;

#ifdef VALIDATE_PARMS
    if(!head)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_head_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_head_idx, DSPTCHR, QDES_REG_Q_HEAD, reg_qdes_reg_q_head);

    *head = RU_FIELD_GET(0, DSPTCHR, QDES_REG_Q_HEAD, HEAD, reg_qdes_reg_q_head);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_set(uint32_t viq_head_vld)
{
    uint32_t reg_qdes_reg_viq_head_vld=0;

#ifdef VALIDATE_PARMS
#endif

    reg_qdes_reg_viq_head_vld = RU_FIELD_SET(0, DSPTCHR, QDES_REG_VIQ_HEAD_VLD, VIQ_HEAD_VLD, reg_qdes_reg_viq_head_vld, viq_head_vld);

    RU_REG_WRITE(0, DSPTCHR, QDES_REG_VIQ_HEAD_VLD, reg_qdes_reg_viq_head_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_get(uint32_t *viq_head_vld)
{
    uint32_t reg_qdes_reg_viq_head_vld;

#ifdef VALIDATE_PARMS
    if(!viq_head_vld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QDES_REG_VIQ_HEAD_VLD, reg_qdes_reg_viq_head_vld);

    *viq_head_vld = RU_FIELD_GET(0, DSPTCHR, QDES_REG_VIQ_HEAD_VLD, VIQ_HEAD_VLD, reg_qdes_reg_viq_head_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set(uint32_t chrncy_vld)
{
    uint32_t reg_qdes_reg_viq_chrncy_vld=0;

#ifdef VALIDATE_PARMS
#endif

    reg_qdes_reg_viq_chrncy_vld = RU_FIELD_SET(0, DSPTCHR, QDES_REG_VIQ_CHRNCY_VLD, CHRNCY_VLD, reg_qdes_reg_viq_chrncy_vld, chrncy_vld);

    RU_REG_WRITE(0, DSPTCHR, QDES_REG_VIQ_CHRNCY_VLD, reg_qdes_reg_viq_chrncy_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get(uint32_t *chrncy_vld)
{
    uint32_t reg_qdes_reg_viq_chrncy_vld;

#ifdef VALIDATE_PARMS
    if(!chrncy_vld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QDES_REG_VIQ_CHRNCY_VLD, reg_qdes_reg_viq_chrncy_vld);

    *chrncy_vld = RU_FIELD_GET(0, DSPTCHR, QDES_REG_VIQ_CHRNCY_VLD, CHRNCY_VLD, reg_qdes_reg_viq_chrncy_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_set(uint32_t viq_head_vld)
{
    uint32_t reg_qdes_reg_veq_head_vld=0;

#ifdef VALIDATE_PARMS
#endif

    reg_qdes_reg_veq_head_vld = RU_FIELD_SET(0, DSPTCHR, QDES_REG_VEQ_HEAD_VLD, VIQ_HEAD_VLD, reg_qdes_reg_veq_head_vld, viq_head_vld);

    RU_REG_WRITE(0, DSPTCHR, QDES_REG_VEQ_HEAD_VLD, reg_qdes_reg_veq_head_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_get(uint32_t *viq_head_vld)
{
    uint32_t reg_qdes_reg_veq_head_vld;

#ifdef VALIDATE_PARMS
    if(!viq_head_vld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QDES_REG_VEQ_HEAD_VLD, reg_qdes_reg_veq_head_vld);

    *viq_head_vld = RU_FIELD_GET(0, DSPTCHR, QDES_REG_VEQ_HEAD_VLD, VIQ_HEAD_VLD, reg_qdes_reg_veq_head_vld);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set(bdmf_boolean use_buf_avl, bdmf_boolean dec_bufout_when_mltcst)
{
    uint32_t reg_qdes_reg_qdes_buf_avl_cntrl=0;

#ifdef VALIDATE_PARMS
    if((use_buf_avl >= _1BITS_MAX_VAL_) ||
       (dec_bufout_when_mltcst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qdes_reg_qdes_buf_avl_cntrl = RU_FIELD_SET(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, USE_BUF_AVL, reg_qdes_reg_qdes_buf_avl_cntrl, use_buf_avl);
    reg_qdes_reg_qdes_buf_avl_cntrl = RU_FIELD_SET(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, DEC_BUFOUT_WHEN_MLTCST, reg_qdes_reg_qdes_buf_avl_cntrl, dec_bufout_when_mltcst);

    RU_REG_WRITE(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, reg_qdes_reg_qdes_buf_avl_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get(bdmf_boolean *use_buf_avl, bdmf_boolean *dec_bufout_when_mltcst)
{
    uint32_t reg_qdes_reg_qdes_buf_avl_cntrl;

#ifdef VALIDATE_PARMS
    if(!use_buf_avl || !dec_bufout_when_mltcst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, reg_qdes_reg_qdes_buf_avl_cntrl);

    *use_buf_avl = RU_FIELD_GET(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, USE_BUF_AVL, reg_qdes_reg_qdes_buf_avl_cntrl);
    *dec_bufout_when_mltcst = RU_FIELD_GET(0, DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL, DEC_BUFOUT_WHEN_MLTCST, reg_qdes_reg_qdes_buf_avl_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_flldes_flldrop_set(uint32_t drpcnt)
{
    uint32_t reg_flldes_flldrop=0;

#ifdef VALIDATE_PARMS
#endif

    reg_flldes_flldrop = RU_FIELD_SET(0, DSPTCHR, FLLDES_FLLDROP, DRPCNT, reg_flldes_flldrop, drpcnt);

    RU_REG_WRITE(0, DSPTCHR, FLLDES_FLLDROP, reg_flldes_flldrop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_flldes_flldrop_get(uint32_t *drpcnt)
{
    uint32_t reg_flldes_flldrop;

#ifdef VALIDATE_PARMS
    if(!drpcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, FLLDES_FLLDROP, reg_flldes_flldrop);

    *drpcnt = RU_FIELD_GET(0, DSPTCHR, FLLDES_FLLDROP, DRPCNT, reg_flldes_flldrop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_flldes_bufavail_get(uint32_t *bufavail)
{
    uint32_t reg_flldes_bufavail;

#ifdef VALIDATE_PARMS
    if(!bufavail)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, FLLDES_BUFAVAIL, reg_flldes_bufavail);

    *bufavail = RU_FIELD_GET(0, DSPTCHR, FLLDES_BUFAVAIL, BUFAVAIL, reg_flldes_bufavail);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_flldes_freemin_get(uint32_t *freemin)
{
    uint32_t reg_flldes_freemin;

#ifdef VALIDATE_PARMS
    if(!freemin)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DSPTCHR, FLLDES_FREEMIN, reg_flldes_freemin);

    *freemin = RU_FIELD_GET(0, DSPTCHR, FLLDES_FREEMIN, FREEMIN, reg_flldes_freemin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_bdram_next_data_set(uint16_t temp_index, uint16_t data)
{
    uint32_t reg_bdram_next_data=0;

#ifdef VALIDATE_PARMS
    if((temp_index >= 1024) ||
       (data >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bdram_next_data = RU_FIELD_SET(0, DSPTCHR, BDRAM_NEXT_DATA, DATA, reg_bdram_next_data, data);

    RU_REG_RAM_WRITE(0, temp_index, DSPTCHR, BDRAM_NEXT_DATA, reg_bdram_next_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_bdram_next_data_get(uint16_t temp_index, uint16_t *data)
{
    uint32_t reg_bdram_next_data;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((temp_index >= 1024))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, temp_index, DSPTCHR, BDRAM_NEXT_DATA, reg_bdram_next_data);

    *data = RU_FIELD_GET(0, DSPTCHR, BDRAM_NEXT_DATA, DATA, reg_bdram_next_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_bdram_prev_data_set(uint16_t temp_index, uint16_t data)
{
    uint32_t reg_bdram_prev_data=0;

#ifdef VALIDATE_PARMS
    if((temp_index >= 1024) ||
       (data >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bdram_prev_data = RU_FIELD_SET(0, DSPTCHR, BDRAM_PREV_DATA, DATA, reg_bdram_prev_data, data);

    RU_REG_RAM_WRITE(0, temp_index, DSPTCHR, BDRAM_PREV_DATA, reg_bdram_prev_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_bdram_prev_data_get(uint16_t temp_index, uint16_t *data)
{
    uint32_t reg_bdram_prev_data;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((temp_index >= 1024))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, temp_index, DSPTCHR, BDRAM_PREV_DATA, reg_bdram_prev_data);

    *data = RU_FIELD_GET(0, DSPTCHR, BDRAM_PREV_DATA, DATA, reg_bdram_prev_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_pdram_data_set(uint16_t temp_index, const dsptchr_pdram_data *pdram_data)
{
#ifdef VALIDATE_PARMS
    if(!pdram_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((temp_index >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, temp_index *2 + 0, DSPTCHR, PDRAM_DATA, pdram_data->reorder_ram_data[0]);
    RU_REG_RAM_WRITE(0, temp_index *2 + 1, DSPTCHR, PDRAM_DATA, pdram_data->reorder_ram_data[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_pdram_data_get(uint16_t temp_index, dsptchr_pdram_data *pdram_data)
{
#ifdef VALIDATE_PARMS
    if(!pdram_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((temp_index >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, temp_index *2 + 0, DSPTCHR, PDRAM_DATA, pdram_data->reorder_ram_data[0]);
    RU_REG_RAM_READ(0, temp_index *2 + 1, DSPTCHR, PDRAM_DATA, pdram_data->reorder_ram_data[1]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_reorder_cfg_dsptchr_reordr_cfg,
    bdmf_address_reorder_cfg_vq_en,
    bdmf_address_reorder_cfg_bb_cfg,
    bdmf_address_reorder_cfg_clk_gate_cntrl,
    bdmf_address_congestion_ingrs_congstn,
    bdmf_address_congestion_egrs_congstn,
    bdmf_address_congestion_total_egrs_congstn,
    bdmf_address_congestion_glbl_congstn,
    bdmf_address_congestion_congstn_status,
    bdmf_address_congestion_per_q_ingrs_congstn_low,
    bdmf_address_congestion_per_q_ingrs_congstn_high,
    bdmf_address_congestion_per_q_egrs_congstn_low,
    bdmf_address_congestion_per_q_egrs_congstn_high,
    bdmf_address_ingrs_queues_q_ingrs_size,
    bdmf_address_ingrs_queues_q_ingrs_limits,
    bdmf_address_queue_mapping_crdt_cfg,
    bdmf_address_queue_mapping_pd_dsptch_add,
    bdmf_address_queue_mapping_q_dest,
    bdmf_address_pool_sizes_cmn_pool_lmt,
    bdmf_address_pool_sizes_cmn_pool_size,
    bdmf_address_pool_sizes_grnted_pool_lmt,
    bdmf_address_pool_sizes_grnted_pool_size,
    bdmf_address_pool_sizes_multi_cst_pool_lmt,
    bdmf_address_pool_sizes_multi_cst_pool_size,
    bdmf_address_pool_sizes_rnr_pool_lmt,
    bdmf_address_pool_sizes_rnr_pool_size,
    bdmf_address_pool_sizes_prcssing_pool_size,
    bdmf_address_mask_msk_tsk_255_0,
    bdmf_address_mask_msk_q,
    bdmf_address_mask_dly_q,
    bdmf_address_mask_non_dly_q,
    bdmf_address_egrs_queues_egrs_dly_qm_crdt,
    bdmf_address_egrs_queues_egrs_non_dly_qm_crdt,
    bdmf_address_egrs_queues_total_q_egrs_size,
    bdmf_address_egrs_queues_per_q_egrs_size,
    bdmf_address_wakeup_control_wkup_req,
    bdmf_address_wakeup_control_wkup_thrshld,
    bdmf_address_disptch_scheduling_dwrr_info,
    bdmf_address_disptch_scheduling_vld_crdt,
    bdmf_address_load_balancing_lb_cfg,
    bdmf_address_load_balancing_free_task_0_1,
    bdmf_address_load_balancing_free_task_2_3,
    bdmf_address_load_balancing_free_task_4_5,
    bdmf_address_load_balancing_free_task_6_7,
    bdmf_address_load_balancing_free_task_8_9,
    bdmf_address_load_balancing_free_task_10_11,
    bdmf_address_load_balancing_free_task_12_13,
    bdmf_address_load_balancing_free_task_14_15,
    bdmf_address_load_balancing_tsk_to_rg_mapping,
    bdmf_address_load_balancing_rg_avlabl_tsk_0_3,
    bdmf_address_load_balancing_rg_avlabl_tsk_4_7,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_0_isr,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ism,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ier,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_0_itr,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_1_isr,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ism,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ier,
    bdmf_address_dsptcher_reordr_top_intr_ctrl_1_itr,
    bdmf_address_debug_dbg_bypss_cntrl,
    bdmf_address_debug_glbl_tsk_cnt_0_7,
    bdmf_address_debug_glbl_tsk_cnt_8_15,
    bdmf_address_debug_dbg_bus_cntrl,
    bdmf_address_debug_dbg_vec_0,
    bdmf_address_debug_dbg_vec_1,
    bdmf_address_debug_dbg_vec_2,
    bdmf_address_debug_dbg_vec_3,
    bdmf_address_debug_dbg_vec_4,
    bdmf_address_debug_dbg_vec_5,
    bdmf_address_debug_dbg_vec_6,
    bdmf_address_debug_dbg_vec_7,
    bdmf_address_debug_dbg_vec_8,
    bdmf_address_debug_dbg_vec_9,
    bdmf_address_debug_dbg_vec_10,
    bdmf_address_debug_dbg_vec_11,
    bdmf_address_debug_dbg_vec_12,
    bdmf_address_debug_dbg_vec_13,
    bdmf_address_debug_dbg_vec_14,
    bdmf_address_debug_dbg_vec_15,
    bdmf_address_debug_dbg_vec_16,
    bdmf_address_debug_dbg_vec_17,
    bdmf_address_debug_dbg_vec_18,
    bdmf_address_debug_dbg_vec_19,
    bdmf_address_debug_dbg_vec_20,
    bdmf_address_debug_dbg_vec_21,
    bdmf_address_debug_dbg_vec_22,
    bdmf_address_debug_dbg_vec_23,
    bdmf_address_debug_statistics_dbg_sttstcs_ctrl,
    bdmf_address_debug_statistics_dbg_cnt,
    bdmf_address_qdes_head,
    bdmf_address_qdes_bfout,
    bdmf_address_qdes_bufin,
    bdmf_address_qdes_tail,
    bdmf_address_qdes_fbdnull,
    bdmf_address_qdes_nullbd,
    bdmf_address_qdes_bufavail,
    bdmf_address_qdes_reg_q_head,
    bdmf_address_qdes_reg_viq_head_vld,
    bdmf_address_qdes_reg_viq_chrncy_vld,
    bdmf_address_qdes_reg_veq_head_vld,
    bdmf_address_qdes_reg_qdes_buf_avl_cntrl,
    bdmf_address_flldes_head,
    bdmf_address_flldes_bfout,
    bdmf_address_flldes_bfin,
    bdmf_address_flldes_tail,
    bdmf_address_flldes_flldrop,
    bdmf_address_flldes_ltint,
    bdmf_address_flldes_bufavail,
    bdmf_address_flldes_freemin,
    bdmf_address_bdram_next_data,
    bdmf_address_bdram_prev_data,
    bdmf_address_pdram_data,
}
bdmf_address;

static int bcm_dsptchr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dsptchr_cngs_params:
    {
        dsptchr_cngs_params cngs_params = { .frst_lvl=parm[2].value.unumber, .scnd_lvl=parm[3].value.unumber, .hyst_thrs=parm[4].value.unumber};
        err = ag_drv_dsptchr_cngs_params_set(parm[1].value.unumber, &cngs_params);
        break;
    }
    case cli_dsptchr_glbl_cngs_params:
    {
        dsptchr_glbl_cngs_params glbl_cngs_params = { .frst_lvl=parm[1].value.unumber, .scnd_lvl=parm[2].value.unumber, .hyst_thrs=parm[3].value.unumber};
        err = ag_drv_dsptchr_glbl_cngs_params_set(&glbl_cngs_params);
        break;
    }
    case cli_dsptchr_q_size_params:
        err = ag_drv_dsptchr_q_size_params_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_credit_cnt:
        err = ag_drv_dsptchr_credit_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_q_limits_params:
        err = ag_drv_dsptchr_q_limits_params_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_ingress_coherency_params:
        err = ag_drv_dsptchr_ingress_coherency_params_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_pools_limits:
    {
        dsptchr_pools_limits pools_limits = { .cmn_pool_lmt=parm[1].value.unumber, .grnted_pool_lmt=parm[2].value.unumber, .mcast_pool_lmt=parm[3].value.unumber, .rnr_pool_lmt=parm[4].value.unumber, .cmn_pool_size=parm[5].value.unumber, .grnted_pool_size=parm[6].value.unumber, .mcast_pool_size=parm[7].value.unumber, .rnr_pool_size=parm[8].value.unumber, .processing_pool_size=parm[9].value.unumber};
        err = ag_drv_dsptchr_pools_limits_set(&pools_limits);
        break;
    }
    case cli_dsptchr_fll_entry:
    {
        dsptchr_fll_entry fll_entry = { .head=parm[1].value.unumber, .tail=parm[2].value.unumber, .minbuf=parm[3].value.unumber, .bfin=parm[4].value.unumber, .count=parm[5].value.unumber};
        err = ag_drv_dsptchr_fll_entry_set(&fll_entry);
        break;
    }
    case cli_dsptchr_rnr_dsptch_addr:
    {
        dsptchr_rnr_dsptch_addr rnr_dsptch_addr = { .base_add=parm[2].value.unumber, .offset_add=parm[3].value.unumber};
        err = ag_drv_dsptchr_rnr_dsptch_addr_set(parm[1].value.unumber, &rnr_dsptch_addr);
        break;
    }
    case cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg:
    {
        dsptchr_reorder_cfg_dsptchr_reordr_cfg reorder_cfg_dsptchr_reordr_cfg = { .disp_enable=parm[1].value.unumber, .rdy=parm[2].value.unumber, .reordr_par_mod=parm[3].value.unumber, .per_q_egrs_congst_en=parm[4].value.unumber, .dsptch_sm_enh_mod=parm[5].value.unumber, .ingrs_pipe_dly_en=parm[6].value.unumber, .ingrs_pipe_dly_cnt=parm[7].value.unumber, .egrs_drop_only=parm[8].value.unumber};
        err = ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(&reorder_cfg_dsptchr_reordr_cfg);
        break;
    }
    case cli_dsptchr_reorder_cfg_vq_en:
        err = ag_drv_dsptchr_reorder_cfg_vq_en_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_reorder_cfg_bb_cfg:
        err = ag_drv_dsptchr_reorder_cfg_bb_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_reorder_cfg_clk_gate_cntrl:
    {
        dsptchr_reorder_cfg_clk_gate_cntrl reorder_cfg_clk_gate_cntrl = { .bypass_clk_gate=parm[1].value.unumber, .timer_val=parm[2].value.unumber, .keep_alive_en=parm[3].value.unumber, .keep_alive_intrvl=parm[4].value.unumber, .keep_alive_cyc=parm[5].value.unumber};
        err = ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(&reorder_cfg_clk_gate_cntrl);
        break;
    }
    case cli_dsptchr_congestion_egrs_congstn:
    {
        dsptchr_cngs_params cngs_params = { .frst_lvl=parm[2].value.unumber, .scnd_lvl=parm[3].value.unumber, .hyst_thrs=parm[4].value.unumber};
        err = ag_drv_dsptchr_congestion_egrs_congstn_set(parm[1].value.unumber, &cngs_params);
        break;
    }
    case cli_dsptchr_congestion_total_egrs_congstn:
    {
        dsptchr_glbl_cngs_params glbl_cngs_params = { .frst_lvl=parm[1].value.unumber, .scnd_lvl=parm[2].value.unumber, .hyst_thrs=parm[3].value.unumber};
        err = ag_drv_dsptchr_congestion_total_egrs_congstn_set(&glbl_cngs_params);
        break;
    }
    case cli_dsptchr_queue_mapping_crdt_cfg:
        err = ag_drv_dsptchr_queue_mapping_crdt_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_queue_mapping_q_dest:
        err = ag_drv_dsptchr_queue_mapping_q_dest_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_mask_msk_tsk_255_0:
    {
        dsptchr_mask_msk_tsk_255_0 mask_msk_tsk_255_0 = { .task_mask = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber}};
        err = ag_drv_dsptchr_mask_msk_tsk_255_0_set(parm[1].value.unumber, &mask_msk_tsk_255_0);
        break;
    }
    case cli_dsptchr_mask_msk_q:
        err = ag_drv_dsptchr_mask_msk_q_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_mask_dly_q:
        err = ag_drv_dsptchr_mask_dly_q_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_mask_non_dly_q:
        err = ag_drv_dsptchr_mask_non_dly_q_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_egrs_queues_egrs_dly_qm_crdt:
        err = ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt:
        err = ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_egrs_queues_total_q_egrs_size:
        err = ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_wakeup_control_wkup_req:
    {
        dsptchr_wakeup_control_wkup_req wakeup_control_wkup_req = { .q0=parm[1].value.unumber, .q1=parm[2].value.unumber, .q2=parm[3].value.unumber, .q3=parm[4].value.unumber, .q4=parm[5].value.unumber, .q5=parm[6].value.unumber, .q6=parm[7].value.unumber, .q7=parm[8].value.unumber, .q8=parm[9].value.unumber, .q9=parm[10].value.unumber, .q10=parm[11].value.unumber, .q11=parm[12].value.unumber, .q12=parm[13].value.unumber, .q13=parm[14].value.unumber, .q14=parm[15].value.unumber, .q15=parm[16].value.unumber, .q16=parm[17].value.unumber, .q17=parm[18].value.unumber, .q18=parm[19].value.unumber, .q19=parm[20].value.unumber, .q20=parm[21].value.unumber, .q21=parm[22].value.unumber, .q22=parm[23].value.unumber, .q23=parm[24].value.unumber, .q24=parm[25].value.unumber, .q25=parm[26].value.unumber, .q26=parm[27].value.unumber, .q27=parm[28].value.unumber, .q28=parm[29].value.unumber, .q29=parm[30].value.unumber, .q30=parm[31].value.unumber, .q31=parm[32].value.unumber};
        err = ag_drv_dsptchr_wakeup_control_wkup_req_set(&wakeup_control_wkup_req);
        break;
    }
    case cli_dsptchr_wakeup_control_wkup_thrshld:
        err = ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_disptch_scheduling_dwrr_info:
        err = ag_drv_dsptchr_disptch_scheduling_dwrr_info_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_disptch_scheduling_vld_crdt:
    {
        dsptchr_disptch_scheduling_vld_crdt disptch_scheduling_vld_crdt = { .q0=parm[1].value.unumber, .q1=parm[2].value.unumber, .q2=parm[3].value.unumber, .q3=parm[4].value.unumber, .q4=parm[5].value.unumber, .q5=parm[6].value.unumber, .q6=parm[7].value.unumber, .q7=parm[8].value.unumber, .q8=parm[9].value.unumber, .q9=parm[10].value.unumber, .q10=parm[11].value.unumber, .q11=parm[12].value.unumber, .q12=parm[13].value.unumber, .q13=parm[14].value.unumber, .q14=parm[15].value.unumber, .q15=parm[16].value.unumber, .q16=parm[17].value.unumber, .q17=parm[18].value.unumber, .q18=parm[19].value.unumber, .q19=parm[20].value.unumber, .q20=parm[21].value.unumber, .q21=parm[22].value.unumber, .q22=parm[23].value.unumber, .q23=parm[24].value.unumber, .q24=parm[25].value.unumber, .q25=parm[26].value.unumber, .q26=parm[27].value.unumber, .q27=parm[28].value.unumber, .q28=parm[29].value.unumber, .q29=parm[30].value.unumber, .q30=parm[31].value.unumber, .q31=parm[32].value.unumber};
        err = ag_drv_dsptchr_disptch_scheduling_vld_crdt_set(&disptch_scheduling_vld_crdt);
        break;
    }
    case cli_dsptchr_load_balancing_lb_cfg:
        err = ag_drv_dsptchr_load_balancing_lb_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_0_1:
        err = ag_drv_dsptchr_load_balancing_free_task_0_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_2_3:
        err = ag_drv_dsptchr_load_balancing_free_task_2_3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_4_5:
        err = ag_drv_dsptchr_load_balancing_free_task_4_5_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_6_7:
        err = ag_drv_dsptchr_load_balancing_free_task_6_7_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_8_9:
        err = ag_drv_dsptchr_load_balancing_free_task_8_9_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_10_11:
        err = ag_drv_dsptchr_load_balancing_free_task_10_11_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_12_13:
        err = ag_drv_dsptchr_load_balancing_free_task_12_13_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_free_task_14_15:
        err = ag_drv_dsptchr_load_balancing_free_task_14_15_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_load_balancing_tsk_to_rg_mapping:
    {
        dsptchr_load_balancing_tsk_to_rg_mapping load_balancing_tsk_to_rg_mapping = { .tsk0=parm[2].value.unumber, .tsk1=parm[3].value.unumber, .tsk2=parm[4].value.unumber, .tsk3=parm[5].value.unumber, .tsk4=parm[6].value.unumber, .tsk5=parm[7].value.unumber, .tsk6=parm[8].value.unumber, .tsk7=parm[9].value.unumber};
        err = ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(parm[1].value.unumber, &load_balancing_tsk_to_rg_mapping);
        break;
    }
    case cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3:
        err = ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7:
        err = ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr:
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr dsptcher_reordr_top_intr_ctrl_0_isr = { .fll_return_buf=parm[1].value.unumber, .fll_cnt_drp=parm[2].value.unumber, .unknwn_msg=parm[3].value.unumber, .fll_overflow=parm[4].value.unumber, .fll_neg=parm[5].value.unumber};
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(&dsptcher_reordr_top_intr_ctrl_0_isr);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier:
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr:
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr:
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr dsptcher_reordr_top_intr_ctrl_1_isr = { .qdest0_int=parm[1].value.unumber, .qdest1_int=parm[2].value.unumber, .qdest2_int=parm[3].value.unumber, .qdest3_int=parm[4].value.unumber, .qdest4_int=parm[5].value.unumber, .qdest5_int=parm[6].value.unumber, .qdest6_int=parm[7].value.unumber, .qdest7_int=parm[8].value.unumber, .qdest8_int=parm[9].value.unumber, .qdest9_int=parm[10].value.unumber, .qdest10_int=parm[11].value.unumber, .qdest11_int=parm[12].value.unumber, .qdest12_int=parm[13].value.unumber, .qdest13_int=parm[14].value.unumber, .qdest14_int=parm[15].value.unumber, .qdest15_int=parm[16].value.unumber, .qdest16_int=parm[17].value.unumber, .qdest17_int=parm[18].value.unumber, .qdest18_int=parm[19].value.unumber, .qdest19_int=parm[20].value.unumber, .qdest20_int=parm[21].value.unumber, .qdest21_int=parm[22].value.unumber, .qdest22_int=parm[23].value.unumber, .qdest23_int=parm[24].value.unumber, .qdest24_int=parm[25].value.unumber, .qdest25_int=parm[26].value.unumber, .qdest26_int=parm[27].value.unumber, .qdest27_int=parm[28].value.unumber, .qdest28_int=parm[29].value.unumber, .qdest29_int=parm[30].value.unumber, .qdest30_int=parm[31].value.unumber, .qdest31_int=parm[32].value.unumber};
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set(&dsptcher_reordr_top_intr_ctrl_1_isr);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier:
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr:
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_debug_dbg_bypss_cntrl:
        err = ag_drv_dsptchr_debug_dbg_bypss_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_debug_glbl_tsk_cnt_0_7:
    {
        dsptchr_debug_glbl_tsk_cnt_0_7 debug_glbl_tsk_cnt_0_7 = { .tsk_cnt_rnr_0=parm[1].value.unumber, .tsk_cnt_rnr_1=parm[2].value.unumber, .tsk_cnt_rnr_2=parm[3].value.unumber, .tsk_cnt_rnr_3=parm[4].value.unumber, .tsk_cnt_rnr_4=parm[5].value.unumber, .tsk_cnt_rnr_5=parm[6].value.unumber, .tsk_cnt_rnr_6=parm[7].value.unumber, .tsk_cnt_rnr_7=parm[8].value.unumber};
        err = ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set(&debug_glbl_tsk_cnt_0_7);
        break;
    }
    case cli_dsptchr_debug_glbl_tsk_cnt_8_15:
    {
        dsptchr_debug_glbl_tsk_cnt_8_15 debug_glbl_tsk_cnt_8_15 = { .tsk_cnt_rnr_8=parm[1].value.unumber, .tsk_cnt_rnr_9=parm[2].value.unumber, .tsk_cnt_rnr_10=parm[3].value.unumber, .tsk_cnt_rnr_11=parm[4].value.unumber, .tsk_cnt_rnr_12=parm[5].value.unumber, .tsk_cnt_rnr_13=parm[6].value.unumber, .tsk_cnt_rnr_14=parm[7].value.unumber, .tsk_cnt_rnr_15=parm[8].value.unumber};
        err = ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set(&debug_glbl_tsk_cnt_8_15);
        break;
    }
    case cli_dsptchr_debug_dbg_bus_cntrl:
        err = ag_drv_dsptchr_debug_dbg_bus_cntrl_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl:
        err = ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_qdes_head:
        err = ag_drv_dsptchr_qdes_head_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_qdes_bfout:
    {
        dsptchr_qdes_bfout qdes_bfout = { .qdes_bfout = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber}};
        err = ag_drv_dsptchr_qdes_bfout_set(parm[1].value.unumber, &qdes_bfout);
        break;
    }
    case cli_dsptchr_qdes_bufin:
    {
        dsptchr_qdes_bufin qdes_bufin = { .qdes_bfin = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber}};
        err = ag_drv_dsptchr_qdes_bufin_set(parm[1].value.unumber, &qdes_bufin);
        break;
    }
    case cli_dsptchr_qdes_tail:
        err = ag_drv_dsptchr_qdes_tail_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_qdes_fbdnull:
    {
        dsptchr_qdes_fbdnull qdes_fbdnull = { .qdes_fbdnull = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber}};
        err = ag_drv_dsptchr_qdes_fbdnull_set(parm[1].value.unumber, &qdes_fbdnull);
        break;
    }
    case cli_dsptchr_qdes_nullbd:
    {
        dsptchr_qdes_nullbd qdes_nullbd = { .qdes_nullbd = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber}};
        err = ag_drv_dsptchr_qdes_nullbd_set(parm[1].value.unumber, &qdes_nullbd);
        break;
    }
    case cli_dsptchr_qdes_reg_q_head:
        err = ag_drv_dsptchr_qdes_reg_q_head_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_qdes_reg_viq_head_vld:
        err = ag_drv_dsptchr_qdes_reg_viq_head_vld_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_qdes_reg_viq_chrncy_vld:
        err = ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_qdes_reg_veq_head_vld:
        err = ag_drv_dsptchr_qdes_reg_veq_head_vld_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl:
        err = ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_flldes_flldrop:
        err = ag_drv_dsptchr_flldes_flldrop_set(parm[1].value.unumber);
        break;
    case cli_dsptchr_bdram_next_data:
        err = ag_drv_dsptchr_bdram_next_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_bdram_prev_data:
        err = ag_drv_dsptchr_bdram_prev_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_pdram_data:
    {
        dsptchr_pdram_data pdram_data = { .reorder_ram_data = { parm[2].value.unumber, parm[3].value.unumber}};
        err = ag_drv_dsptchr_pdram_data_set(parm[1].value.unumber, &pdram_data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_dsptchr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dsptchr_cngs_params:
    {
        dsptchr_cngs_params cngs_params;
        err = ag_drv_dsptchr_cngs_params_get(parm[1].value.unumber, &cngs_params);
        bdmf_session_print(session, "frst_lvl = %u (0x%x)\n", cngs_params.frst_lvl, cngs_params.frst_lvl);
        bdmf_session_print(session, "scnd_lvl = %u (0x%x)\n", cngs_params.scnd_lvl, cngs_params.scnd_lvl);
        bdmf_session_print(session, "hyst_thrs = %u (0x%x)\n", cngs_params.hyst_thrs, cngs_params.hyst_thrs);
        break;
    }
    case cli_dsptchr_glbl_cngs_params:
    {
        dsptchr_glbl_cngs_params glbl_cngs_params;
        err = ag_drv_dsptchr_glbl_cngs_params_get(&glbl_cngs_params);
        bdmf_session_print(session, "frst_lvl = %u (0x%x)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.frst_lvl);
        bdmf_session_print(session, "scnd_lvl = %u (0x%x)\n", glbl_cngs_params.scnd_lvl, glbl_cngs_params.scnd_lvl);
        bdmf_session_print(session, "hyst_thrs = %u (0x%x)\n", glbl_cngs_params.hyst_thrs, glbl_cngs_params.hyst_thrs);
        break;
    }
    case cli_dsptchr_q_size_params:
    {
        uint16_t cmn_cnt;
        err = ag_drv_dsptchr_q_size_params_get(parm[1].value.unumber, &cmn_cnt);
        bdmf_session_print(session, "cmn_cnt = %u (0x%x)\n", cmn_cnt, cmn_cnt);
        break;
    }
    case cli_dsptchr_credit_cnt:
    {
        uint16_t credit_cnt;
        err = ag_drv_dsptchr_credit_cnt_get(parm[1].value.unumber, &credit_cnt);
        bdmf_session_print(session, "credit_cnt = %u (0x%x)\n", credit_cnt, credit_cnt);
        break;
    }
    case cli_dsptchr_q_limits_params:
    {
        uint16_t cmn_max;
        uint16_t gurntd_max;
        err = ag_drv_dsptchr_q_limits_params_get(parm[1].value.unumber, &cmn_max, &gurntd_max);
        bdmf_session_print(session, "cmn_max = %u (0x%x)\n", cmn_max, cmn_max);
        bdmf_session_print(session, "gurntd_max = %u (0x%x)\n", gurntd_max, gurntd_max);
        break;
    }
    case cli_dsptchr_ingress_coherency_params:
    {
        bdmf_boolean chrncy_en;
        uint16_t chrncy_cnt;
        err = ag_drv_dsptchr_ingress_coherency_params_get(parm[1].value.unumber, &chrncy_en, &chrncy_cnt);
        bdmf_session_print(session, "chrncy_en = %u (0x%x)\n", chrncy_en, chrncy_en);
        bdmf_session_print(session, "chrncy_cnt = %u (0x%x)\n", chrncy_cnt, chrncy_cnt);
        break;
    }
    case cli_dsptchr_pools_limits:
    {
        dsptchr_pools_limits pools_limits;
        err = ag_drv_dsptchr_pools_limits_get(&pools_limits);
        bdmf_session_print(session, "cmn_pool_lmt = %u (0x%x)\n", pools_limits.cmn_pool_lmt, pools_limits.cmn_pool_lmt);
        bdmf_session_print(session, "grnted_pool_lmt = %u (0x%x)\n", pools_limits.grnted_pool_lmt, pools_limits.grnted_pool_lmt);
        bdmf_session_print(session, "mcast_pool_lmt = %u (0x%x)\n", pools_limits.mcast_pool_lmt, pools_limits.mcast_pool_lmt);
        bdmf_session_print(session, "rnr_pool_lmt = %u (0x%x)\n", pools_limits.rnr_pool_lmt, pools_limits.rnr_pool_lmt);
        bdmf_session_print(session, "cmn_pool_size = %u (0x%x)\n", pools_limits.cmn_pool_size, pools_limits.cmn_pool_size);
        bdmf_session_print(session, "grnted_pool_size = %u (0x%x)\n", pools_limits.grnted_pool_size, pools_limits.grnted_pool_size);
        bdmf_session_print(session, "mcast_pool_size = %u (0x%x)\n", pools_limits.mcast_pool_size, pools_limits.mcast_pool_size);
        bdmf_session_print(session, "rnr_pool_size = %u (0x%x)\n", pools_limits.rnr_pool_size, pools_limits.rnr_pool_size);
        bdmf_session_print(session, "processing_pool_size = %u (0x%x)\n", pools_limits.processing_pool_size, pools_limits.processing_pool_size);
        break;
    }
    case cli_dsptchr_fll_entry:
    {
        dsptchr_fll_entry fll_entry;
        err = ag_drv_dsptchr_fll_entry_get(&fll_entry);
        bdmf_session_print(session, "head = %u (0x%x)\n", fll_entry.head, fll_entry.head);
        bdmf_session_print(session, "tail = %u (0x%x)\n", fll_entry.tail, fll_entry.tail);
        bdmf_session_print(session, "minbuf = %u (0x%x)\n", fll_entry.minbuf, fll_entry.minbuf);
        bdmf_session_print(session, "bfin = %u (0x%x)\n", fll_entry.bfin, fll_entry.bfin);
        bdmf_session_print(session, "count = %u (0x%x)\n", fll_entry.count, fll_entry.count);
        break;
    }
    case cli_dsptchr_rnr_dsptch_addr:
    {
        dsptchr_rnr_dsptch_addr rnr_dsptch_addr;
        err = ag_drv_dsptchr_rnr_dsptch_addr_get(parm[1].value.unumber, &rnr_dsptch_addr);
        bdmf_session_print(session, "base_add = %u (0x%x)\n", rnr_dsptch_addr.base_add, rnr_dsptch_addr.base_add);
        bdmf_session_print(session, "offset_add = %u (0x%x)\n", rnr_dsptch_addr.offset_add, rnr_dsptch_addr.offset_add);
        break;
    }
    case cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg:
    {
        dsptchr_reorder_cfg_dsptchr_reordr_cfg reorder_cfg_dsptchr_reordr_cfg;
        err = ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get(&reorder_cfg_dsptchr_reordr_cfg);
        bdmf_session_print(session, "disp_enable = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.disp_enable, reorder_cfg_dsptchr_reordr_cfg.disp_enable);
        bdmf_session_print(session, "rdy = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.rdy, reorder_cfg_dsptchr_reordr_cfg.rdy);
        bdmf_session_print(session, "reordr_par_mod = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.reordr_par_mod, reorder_cfg_dsptchr_reordr_cfg.reordr_par_mod);
        bdmf_session_print(session, "per_q_egrs_congst_en = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.per_q_egrs_congst_en, reorder_cfg_dsptchr_reordr_cfg.per_q_egrs_congst_en);
        bdmf_session_print(session, "dsptch_sm_enh_mod = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.dsptch_sm_enh_mod, reorder_cfg_dsptchr_reordr_cfg.dsptch_sm_enh_mod);
        bdmf_session_print(session, "ingrs_pipe_dly_en = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_en, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_en);
        bdmf_session_print(session, "ingrs_pipe_dly_cnt = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_cnt, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_cnt);
        bdmf_session_print(session, "egrs_drop_only = %u (0x%x)\n", reorder_cfg_dsptchr_reordr_cfg.egrs_drop_only, reorder_cfg_dsptchr_reordr_cfg.egrs_drop_only);
        break;
    }
    case cli_dsptchr_reorder_cfg_vq_en:
    {
        uint32_t en;
        err = ag_drv_dsptchr_reorder_cfg_vq_en_get(&en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_dsptchr_reorder_cfg_bb_cfg:
    {
        uint8_t src_id;
        uint8_t dst_id_ovride;
        uint16_t route_ovride;
        bdmf_boolean ovride_en;
        err = ag_drv_dsptchr_reorder_cfg_bb_cfg_get(&src_id, &dst_id_ovride, &route_ovride, &ovride_en);
        bdmf_session_print(session, "src_id = %u (0x%x)\n", src_id, src_id);
        bdmf_session_print(session, "dst_id_ovride = %u (0x%x)\n", dst_id_ovride, dst_id_ovride);
        bdmf_session_print(session, "route_ovride = %u (0x%x)\n", route_ovride, route_ovride);
        bdmf_session_print(session, "ovride_en = %u (0x%x)\n", ovride_en, ovride_en);
        break;
    }
    case cli_dsptchr_reorder_cfg_clk_gate_cntrl:
    {
        dsptchr_reorder_cfg_clk_gate_cntrl reorder_cfg_clk_gate_cntrl;
        err = ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(&reorder_cfg_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", reorder_cfg_clk_gate_cntrl.bypass_clk_gate, reorder_cfg_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", reorder_cfg_clk_gate_cntrl.timer_val, reorder_cfg_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", reorder_cfg_clk_gate_cntrl.keep_alive_en, reorder_cfg_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", reorder_cfg_clk_gate_cntrl.keep_alive_intrvl, reorder_cfg_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", reorder_cfg_clk_gate_cntrl.keep_alive_cyc, reorder_cfg_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_dsptchr_congestion_egrs_congstn:
    {
        dsptchr_cngs_params cngs_params;
        err = ag_drv_dsptchr_congestion_egrs_congstn_get(parm[1].value.unumber, &cngs_params);
        bdmf_session_print(session, "frst_lvl = %u (0x%x)\n", cngs_params.frst_lvl, cngs_params.frst_lvl);
        bdmf_session_print(session, "scnd_lvl = %u (0x%x)\n", cngs_params.scnd_lvl, cngs_params.scnd_lvl);
        bdmf_session_print(session, "hyst_thrs = %u (0x%x)\n", cngs_params.hyst_thrs, cngs_params.hyst_thrs);
        break;
    }
    case cli_dsptchr_congestion_total_egrs_congstn:
    {
        dsptchr_glbl_cngs_params glbl_cngs_params;
        err = ag_drv_dsptchr_congestion_total_egrs_congstn_get(&glbl_cngs_params);
        bdmf_session_print(session, "frst_lvl = %u (0x%x)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.frst_lvl);
        bdmf_session_print(session, "scnd_lvl = %u (0x%x)\n", glbl_cngs_params.scnd_lvl, glbl_cngs_params.scnd_lvl);
        bdmf_session_print(session, "hyst_thrs = %u (0x%x)\n", glbl_cngs_params.hyst_thrs, glbl_cngs_params.hyst_thrs);
        break;
    }
    case cli_dsptchr_congestion_congstn_status:
    {
        dsptchr_congestion_congstn_status congestion_congstn_status;
        err = ag_drv_dsptchr_congestion_congstn_status_get(&congestion_congstn_status);
        bdmf_session_print(session, "glbl_congstn = %u (0x%x)\n", congestion_congstn_status.glbl_congstn, congestion_congstn_status.glbl_congstn);
        bdmf_session_print(session, "glbl_egrs_congstn = %u (0x%x)\n", congestion_congstn_status.glbl_egrs_congstn, congestion_congstn_status.glbl_egrs_congstn);
        bdmf_session_print(session, "sbpm_congstn = %u (0x%x)\n", congestion_congstn_status.sbpm_congstn, congestion_congstn_status.sbpm_congstn);
        bdmf_session_print(session, "glbl_congstn_stcky = %u (0x%x)\n", congestion_congstn_status.glbl_congstn_stcky, congestion_congstn_status.glbl_congstn_stcky);
        bdmf_session_print(session, "glbl_egrs_congstn_stcky = %u (0x%x)\n", congestion_congstn_status.glbl_egrs_congstn_stcky, congestion_congstn_status.glbl_egrs_congstn_stcky);
        bdmf_session_print(session, "sbpm_congstn_stcky = %u (0x%x)\n", congestion_congstn_status.sbpm_congstn_stcky, congestion_congstn_status.sbpm_congstn_stcky);
        break;
    }
    case cli_dsptchr_congestion_per_q_ingrs_congstn_low:
    {
        uint32_t congstn_state;
        err = ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get(&congstn_state);
        bdmf_session_print(session, "congstn_state = %u (0x%x)\n", congstn_state, congstn_state);
        break;
    }
    case cli_dsptchr_congestion_per_q_ingrs_congstn_high:
    {
        uint32_t congstn_state;
        err = ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get(&congstn_state);
        bdmf_session_print(session, "congstn_state = %u (0x%x)\n", congstn_state, congstn_state);
        break;
    }
    case cli_dsptchr_congestion_per_q_egrs_congstn_low:
    {
        uint32_t congstn_state;
        err = ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get(&congstn_state);
        bdmf_session_print(session, "congstn_state = %u (0x%x)\n", congstn_state, congstn_state);
        break;
    }
    case cli_dsptchr_congestion_per_q_egrs_congstn_high:
    {
        uint32_t congstn_state;
        err = ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get(&congstn_state);
        bdmf_session_print(session, "congstn_state = %u (0x%x)\n", congstn_state, congstn_state);
        break;
    }
    case cli_dsptchr_queue_mapping_crdt_cfg:
    {
        uint8_t bb_id;
        uint16_t trgt_add;
        err = ag_drv_dsptchr_queue_mapping_crdt_cfg_get(parm[1].value.unumber, &bb_id, &trgt_add);
        bdmf_session_print(session, "bb_id = %u (0x%x)\n", bb_id, bb_id);
        bdmf_session_print(session, "trgt_add = %u (0x%x)\n", trgt_add, trgt_add);
        break;
    }
    case cli_dsptchr_queue_mapping_q_dest:
    {
        bdmf_boolean is_dest_disp;
        err = ag_drv_dsptchr_queue_mapping_q_dest_get(parm[1].value.unumber, &is_dest_disp);
        bdmf_session_print(session, "is_dest_disp = %u (0x%x)\n", is_dest_disp, is_dest_disp);
        break;
    }
    case cli_dsptchr_mask_msk_tsk_255_0:
    {
        dsptchr_mask_msk_tsk_255_0 mask_msk_tsk_255_0;
        err = ag_drv_dsptchr_mask_msk_tsk_255_0_get(parm[1].value.unumber, &mask_msk_tsk_255_0);
        bdmf_session_print(session, "task_mask[0] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[0], mask_msk_tsk_255_0.task_mask[0]);
        bdmf_session_print(session, "task_mask[1] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[1], mask_msk_tsk_255_0.task_mask[1]);
        bdmf_session_print(session, "task_mask[2] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[2], mask_msk_tsk_255_0.task_mask[2]);
        bdmf_session_print(session, "task_mask[3] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[3], mask_msk_tsk_255_0.task_mask[3]);
        bdmf_session_print(session, "task_mask[4] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[4], mask_msk_tsk_255_0.task_mask[4]);
        bdmf_session_print(session, "task_mask[5] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[5], mask_msk_tsk_255_0.task_mask[5]);
        bdmf_session_print(session, "task_mask[6] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[6], mask_msk_tsk_255_0.task_mask[6]);
        bdmf_session_print(session, "task_mask[7] = %u (0x%x)\n", mask_msk_tsk_255_0.task_mask[7], mask_msk_tsk_255_0.task_mask[7]);
        break;
    }
    case cli_dsptchr_mask_msk_q:
    {
        uint32_t mask;
        err = ag_drv_dsptchr_mask_msk_q_get(parm[1].value.unumber, &mask);
        bdmf_session_print(session, "mask = %u (0x%x)\n", mask, mask);
        break;
    }
    case cli_dsptchr_mask_dly_q:
    {
        bdmf_boolean set_delay;
        err = ag_drv_dsptchr_mask_dly_q_get(parm[1].value.unumber, &set_delay);
        bdmf_session_print(session, "set_delay = %u (0x%x)\n", set_delay, set_delay);
        break;
    }
    case cli_dsptchr_mask_non_dly_q:
    {
        bdmf_boolean set_non_delay;
        err = ag_drv_dsptchr_mask_non_dly_q_get(parm[1].value.unumber, &set_non_delay);
        bdmf_session_print(session, "set_non_delay = %u (0x%x)\n", set_non_delay, set_non_delay);
        break;
    }
    case cli_dsptchr_egrs_queues_egrs_dly_qm_crdt:
    {
        uint8_t dly_crdt;
        err = ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get(&dly_crdt);
        bdmf_session_print(session, "dly_crdt = %u (0x%x)\n", dly_crdt, dly_crdt);
        break;
    }
    case cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt:
    {
        uint8_t non_dly_crdt;
        err = ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get(&non_dly_crdt);
        bdmf_session_print(session, "non_dly_crdt = %u (0x%x)\n", non_dly_crdt, non_dly_crdt);
        break;
    }
    case cli_dsptchr_egrs_queues_total_q_egrs_size:
    {
        uint16_t total_egrs_size;
        err = ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get(&total_egrs_size);
        bdmf_session_print(session, "total_egrs_size = %u (0x%x)\n", total_egrs_size, total_egrs_size);
        break;
    }
    case cli_dsptchr_egrs_queues_per_q_egrs_size:
    {
        uint16_t q_egrs_size;
        err = ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get(parm[1].value.unumber, &q_egrs_size);
        bdmf_session_print(session, "q_egrs_size = %u (0x%x)\n", q_egrs_size, q_egrs_size);
        break;
    }
    case cli_dsptchr_wakeup_control_wkup_req:
    {
        dsptchr_wakeup_control_wkup_req wakeup_control_wkup_req;
        err = ag_drv_dsptchr_wakeup_control_wkup_req_get(&wakeup_control_wkup_req);
        bdmf_session_print(session, "q0 = %u (0x%x)\n", wakeup_control_wkup_req.q0, wakeup_control_wkup_req.q0);
        bdmf_session_print(session, "q1 = %u (0x%x)\n", wakeup_control_wkup_req.q1, wakeup_control_wkup_req.q1);
        bdmf_session_print(session, "q2 = %u (0x%x)\n", wakeup_control_wkup_req.q2, wakeup_control_wkup_req.q2);
        bdmf_session_print(session, "q3 = %u (0x%x)\n", wakeup_control_wkup_req.q3, wakeup_control_wkup_req.q3);
        bdmf_session_print(session, "q4 = %u (0x%x)\n", wakeup_control_wkup_req.q4, wakeup_control_wkup_req.q4);
        bdmf_session_print(session, "q5 = %u (0x%x)\n", wakeup_control_wkup_req.q5, wakeup_control_wkup_req.q5);
        bdmf_session_print(session, "q6 = %u (0x%x)\n", wakeup_control_wkup_req.q6, wakeup_control_wkup_req.q6);
        bdmf_session_print(session, "q7 = %u (0x%x)\n", wakeup_control_wkup_req.q7, wakeup_control_wkup_req.q7);
        bdmf_session_print(session, "q8 = %u (0x%x)\n", wakeup_control_wkup_req.q8, wakeup_control_wkup_req.q8);
        bdmf_session_print(session, "q9 = %u (0x%x)\n", wakeup_control_wkup_req.q9, wakeup_control_wkup_req.q9);
        bdmf_session_print(session, "q10 = %u (0x%x)\n", wakeup_control_wkup_req.q10, wakeup_control_wkup_req.q10);
        bdmf_session_print(session, "q11 = %u (0x%x)\n", wakeup_control_wkup_req.q11, wakeup_control_wkup_req.q11);
        bdmf_session_print(session, "q12 = %u (0x%x)\n", wakeup_control_wkup_req.q12, wakeup_control_wkup_req.q12);
        bdmf_session_print(session, "q13 = %u (0x%x)\n", wakeup_control_wkup_req.q13, wakeup_control_wkup_req.q13);
        bdmf_session_print(session, "q14 = %u (0x%x)\n", wakeup_control_wkup_req.q14, wakeup_control_wkup_req.q14);
        bdmf_session_print(session, "q15 = %u (0x%x)\n", wakeup_control_wkup_req.q15, wakeup_control_wkup_req.q15);
        bdmf_session_print(session, "q16 = %u (0x%x)\n", wakeup_control_wkup_req.q16, wakeup_control_wkup_req.q16);
        bdmf_session_print(session, "q17 = %u (0x%x)\n", wakeup_control_wkup_req.q17, wakeup_control_wkup_req.q17);
        bdmf_session_print(session, "q18 = %u (0x%x)\n", wakeup_control_wkup_req.q18, wakeup_control_wkup_req.q18);
        bdmf_session_print(session, "q19 = %u (0x%x)\n", wakeup_control_wkup_req.q19, wakeup_control_wkup_req.q19);
        bdmf_session_print(session, "q20 = %u (0x%x)\n", wakeup_control_wkup_req.q20, wakeup_control_wkup_req.q20);
        bdmf_session_print(session, "q21 = %u (0x%x)\n", wakeup_control_wkup_req.q21, wakeup_control_wkup_req.q21);
        bdmf_session_print(session, "q22 = %u (0x%x)\n", wakeup_control_wkup_req.q22, wakeup_control_wkup_req.q22);
        bdmf_session_print(session, "q23 = %u (0x%x)\n", wakeup_control_wkup_req.q23, wakeup_control_wkup_req.q23);
        bdmf_session_print(session, "q24 = %u (0x%x)\n", wakeup_control_wkup_req.q24, wakeup_control_wkup_req.q24);
        bdmf_session_print(session, "q25 = %u (0x%x)\n", wakeup_control_wkup_req.q25, wakeup_control_wkup_req.q25);
        bdmf_session_print(session, "q26 = %u (0x%x)\n", wakeup_control_wkup_req.q26, wakeup_control_wkup_req.q26);
        bdmf_session_print(session, "q27 = %u (0x%x)\n", wakeup_control_wkup_req.q27, wakeup_control_wkup_req.q27);
        bdmf_session_print(session, "q28 = %u (0x%x)\n", wakeup_control_wkup_req.q28, wakeup_control_wkup_req.q28);
        bdmf_session_print(session, "q29 = %u (0x%x)\n", wakeup_control_wkup_req.q29, wakeup_control_wkup_req.q29);
        bdmf_session_print(session, "q30 = %u (0x%x)\n", wakeup_control_wkup_req.q30, wakeup_control_wkup_req.q30);
        bdmf_session_print(session, "q31 = %u (0x%x)\n", wakeup_control_wkup_req.q31, wakeup_control_wkup_req.q31);
        break;
    }
    case cli_dsptchr_wakeup_control_wkup_thrshld:
    {
        uint16_t wkup_thrshld;
        err = ag_drv_dsptchr_wakeup_control_wkup_thrshld_get(&wkup_thrshld);
        bdmf_session_print(session, "wkup_thrshld = %u (0x%x)\n", wkup_thrshld, wkup_thrshld);
        break;
    }
    case cli_dsptchr_disptch_scheduling_dwrr_info:
    {
        uint32_t q_crdt;
        bdmf_boolean ngtv;
        uint16_t quntum;
        err = ag_drv_dsptchr_disptch_scheduling_dwrr_info_get(parm[1].value.unumber, &q_crdt, &ngtv, &quntum);
        bdmf_session_print(session, "q_crdt = %u (0x%x)\n", q_crdt, q_crdt);
        bdmf_session_print(session, "ngtv = %u (0x%x)\n", ngtv, ngtv);
        bdmf_session_print(session, "quntum = %u (0x%x)\n", quntum, quntum);
        break;
    }
    case cli_dsptchr_disptch_scheduling_vld_crdt:
    {
        dsptchr_disptch_scheduling_vld_crdt disptch_scheduling_vld_crdt;
        err = ag_drv_dsptchr_disptch_scheduling_vld_crdt_get(&disptch_scheduling_vld_crdt);
        bdmf_session_print(session, "q0 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q0, disptch_scheduling_vld_crdt.q0);
        bdmf_session_print(session, "q1 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q1, disptch_scheduling_vld_crdt.q1);
        bdmf_session_print(session, "q2 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q2, disptch_scheduling_vld_crdt.q2);
        bdmf_session_print(session, "q3 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q3, disptch_scheduling_vld_crdt.q3);
        bdmf_session_print(session, "q4 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q4, disptch_scheduling_vld_crdt.q4);
        bdmf_session_print(session, "q5 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q5, disptch_scheduling_vld_crdt.q5);
        bdmf_session_print(session, "q6 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q6, disptch_scheduling_vld_crdt.q6);
        bdmf_session_print(session, "q7 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q7, disptch_scheduling_vld_crdt.q7);
        bdmf_session_print(session, "q8 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q8, disptch_scheduling_vld_crdt.q8);
        bdmf_session_print(session, "q9 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q9, disptch_scheduling_vld_crdt.q9);
        bdmf_session_print(session, "q10 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q10, disptch_scheduling_vld_crdt.q10);
        bdmf_session_print(session, "q11 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q11, disptch_scheduling_vld_crdt.q11);
        bdmf_session_print(session, "q12 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q12, disptch_scheduling_vld_crdt.q12);
        bdmf_session_print(session, "q13 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q13, disptch_scheduling_vld_crdt.q13);
        bdmf_session_print(session, "q14 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q14, disptch_scheduling_vld_crdt.q14);
        bdmf_session_print(session, "q15 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q15, disptch_scheduling_vld_crdt.q15);
        bdmf_session_print(session, "q16 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q16, disptch_scheduling_vld_crdt.q16);
        bdmf_session_print(session, "q17 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q17, disptch_scheduling_vld_crdt.q17);
        bdmf_session_print(session, "q18 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q18, disptch_scheduling_vld_crdt.q18);
        bdmf_session_print(session, "q19 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q19, disptch_scheduling_vld_crdt.q19);
        bdmf_session_print(session, "q20 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q20, disptch_scheduling_vld_crdt.q20);
        bdmf_session_print(session, "q21 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q21, disptch_scheduling_vld_crdt.q21);
        bdmf_session_print(session, "q22 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q22, disptch_scheduling_vld_crdt.q22);
        bdmf_session_print(session, "q23 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q23, disptch_scheduling_vld_crdt.q23);
        bdmf_session_print(session, "q24 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q24, disptch_scheduling_vld_crdt.q24);
        bdmf_session_print(session, "q25 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q25, disptch_scheduling_vld_crdt.q25);
        bdmf_session_print(session, "q26 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q26, disptch_scheduling_vld_crdt.q26);
        bdmf_session_print(session, "q27 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q27, disptch_scheduling_vld_crdt.q27);
        bdmf_session_print(session, "q28 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q28, disptch_scheduling_vld_crdt.q28);
        bdmf_session_print(session, "q29 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q29, disptch_scheduling_vld_crdt.q29);
        bdmf_session_print(session, "q30 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q30, disptch_scheduling_vld_crdt.q30);
        bdmf_session_print(session, "q31 = %u (0x%x)\n", disptch_scheduling_vld_crdt.q31, disptch_scheduling_vld_crdt.q31);
        break;
    }
    case cli_dsptchr_load_balancing_lb_cfg:
    {
        bdmf_boolean lb_mode;
        uint8_t sp_thrshld;
        err = ag_drv_dsptchr_load_balancing_lb_cfg_get(&lb_mode, &sp_thrshld);
        bdmf_session_print(session, "lb_mode = %u (0x%x)\n", lb_mode, lb_mode);
        bdmf_session_print(session, "sp_thrshld = %u (0x%x)\n", sp_thrshld, sp_thrshld);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_0_1:
    {
        uint16_t rnr0;
        uint16_t rnr1;
        err = ag_drv_dsptchr_load_balancing_free_task_0_1_get(&rnr0, &rnr1);
        bdmf_session_print(session, "rnr0 = %u (0x%x)\n", rnr0, rnr0);
        bdmf_session_print(session, "rnr1 = %u (0x%x)\n", rnr1, rnr1);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_2_3:
    {
        uint16_t rnr2;
        uint16_t rnr3;
        err = ag_drv_dsptchr_load_balancing_free_task_2_3_get(&rnr2, &rnr3);
        bdmf_session_print(session, "rnr2 = %u (0x%x)\n", rnr2, rnr2);
        bdmf_session_print(session, "rnr3 = %u (0x%x)\n", rnr3, rnr3);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_4_5:
    {
        uint16_t rnr4;
        uint16_t rnr5;
        err = ag_drv_dsptchr_load_balancing_free_task_4_5_get(&rnr4, &rnr5);
        bdmf_session_print(session, "rnr4 = %u (0x%x)\n", rnr4, rnr4);
        bdmf_session_print(session, "rnr5 = %u (0x%x)\n", rnr5, rnr5);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_6_7:
    {
        uint16_t rnr6;
        uint16_t rnr7;
        err = ag_drv_dsptchr_load_balancing_free_task_6_7_get(&rnr6, &rnr7);
        bdmf_session_print(session, "rnr6 = %u (0x%x)\n", rnr6, rnr6);
        bdmf_session_print(session, "rnr7 = %u (0x%x)\n", rnr7, rnr7);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_8_9:
    {
        uint16_t rnr8;
        uint16_t rnr9;
        err = ag_drv_dsptchr_load_balancing_free_task_8_9_get(&rnr8, &rnr9);
        bdmf_session_print(session, "rnr8 = %u (0x%x)\n", rnr8, rnr8);
        bdmf_session_print(session, "rnr9 = %u (0x%x)\n", rnr9, rnr9);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_10_11:
    {
        uint16_t rnr10;
        uint16_t rnr11;
        err = ag_drv_dsptchr_load_balancing_free_task_10_11_get(&rnr10, &rnr11);
        bdmf_session_print(session, "rnr10 = %u (0x%x)\n", rnr10, rnr10);
        bdmf_session_print(session, "rnr11 = %u (0x%x)\n", rnr11, rnr11);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_12_13:
    {
        uint16_t rnr12;
        uint16_t rnr13;
        err = ag_drv_dsptchr_load_balancing_free_task_12_13_get(&rnr12, &rnr13);
        bdmf_session_print(session, "rnr12 = %u (0x%x)\n", rnr12, rnr12);
        bdmf_session_print(session, "rnr13 = %u (0x%x)\n", rnr13, rnr13);
        break;
    }
    case cli_dsptchr_load_balancing_free_task_14_15:
    {
        uint16_t rnr14;
        uint16_t rnr15;
        err = ag_drv_dsptchr_load_balancing_free_task_14_15_get(&rnr14, &rnr15);
        bdmf_session_print(session, "rnr14 = %u (0x%x)\n", rnr14, rnr14);
        bdmf_session_print(session, "rnr15 = %u (0x%x)\n", rnr15, rnr15);
        break;
    }
    case cli_dsptchr_load_balancing_tsk_to_rg_mapping:
    {
        dsptchr_load_balancing_tsk_to_rg_mapping load_balancing_tsk_to_rg_mapping;
        err = ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get(parm[1].value.unumber, &load_balancing_tsk_to_rg_mapping);
        bdmf_session_print(session, "tsk0 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk0, load_balancing_tsk_to_rg_mapping.tsk0);
        bdmf_session_print(session, "tsk1 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk1, load_balancing_tsk_to_rg_mapping.tsk1);
        bdmf_session_print(session, "tsk2 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk2, load_balancing_tsk_to_rg_mapping.tsk2);
        bdmf_session_print(session, "tsk3 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk3, load_balancing_tsk_to_rg_mapping.tsk3);
        bdmf_session_print(session, "tsk4 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk4, load_balancing_tsk_to_rg_mapping.tsk4);
        bdmf_session_print(session, "tsk5 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk5, load_balancing_tsk_to_rg_mapping.tsk5);
        bdmf_session_print(session, "tsk6 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk6, load_balancing_tsk_to_rg_mapping.tsk6);
        bdmf_session_print(session, "tsk7 = %u (0x%x)\n", load_balancing_tsk_to_rg_mapping.tsk7, load_balancing_tsk_to_rg_mapping.tsk7);
        break;
    }
    case cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3:
    {
        uint8_t tsk_cnt_rg_0;
        uint8_t tsk_cnt_rg_1;
        uint8_t tsk_cnt_rg_2;
        uint8_t tsk_cnt_rg_3;
        err = ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get(&tsk_cnt_rg_0, &tsk_cnt_rg_1, &tsk_cnt_rg_2, &tsk_cnt_rg_3);
        bdmf_session_print(session, "tsk_cnt_rg_0 = %u (0x%x)\n", tsk_cnt_rg_0, tsk_cnt_rg_0);
        bdmf_session_print(session, "tsk_cnt_rg_1 = %u (0x%x)\n", tsk_cnt_rg_1, tsk_cnt_rg_1);
        bdmf_session_print(session, "tsk_cnt_rg_2 = %u (0x%x)\n", tsk_cnt_rg_2, tsk_cnt_rg_2);
        bdmf_session_print(session, "tsk_cnt_rg_3 = %u (0x%x)\n", tsk_cnt_rg_3, tsk_cnt_rg_3);
        break;
    }
    case cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7:
    {
        uint8_t tsk_cnt_rg_4;
        uint8_t tsk_cnt_rg_5;
        uint8_t tsk_cnt_rg_6;
        uint8_t tsk_cnt_rg_7;
        err = ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get(&tsk_cnt_rg_4, &tsk_cnt_rg_5, &tsk_cnt_rg_6, &tsk_cnt_rg_7);
        bdmf_session_print(session, "tsk_cnt_rg_4 = %u (0x%x)\n", tsk_cnt_rg_4, tsk_cnt_rg_4);
        bdmf_session_print(session, "tsk_cnt_rg_5 = %u (0x%x)\n", tsk_cnt_rg_5, tsk_cnt_rg_5);
        bdmf_session_print(session, "tsk_cnt_rg_6 = %u (0x%x)\n", tsk_cnt_rg_6, tsk_cnt_rg_6);
        bdmf_session_print(session, "tsk_cnt_rg_7 = %u (0x%x)\n", tsk_cnt_rg_7, tsk_cnt_rg_7);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr:
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr dsptcher_reordr_top_intr_ctrl_0_isr;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get(&dsptcher_reordr_top_intr_ctrl_0_isr);
        bdmf_session_print(session, "fll_return_buf = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf, dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf);
        bdmf_session_print(session, "fll_cnt_drp = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp, dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp);
        bdmf_session_print(session, "unknwn_msg = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg, dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg);
        bdmf_session_print(session, "fll_overflow = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow, dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow);
        bdmf_session_print(session, "fll_neg = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg, dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism:
    {
        uint32_t ism;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get(&ism);
        bdmf_session_print(session, "ism = %u (0x%x)\n", ism, ism);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier:
    {
        uint32_t iem;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get(&iem);
        bdmf_session_print(session, "iem = %u (0x%x)\n", iem, iem);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr:
    {
        uint32_t ist;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr:
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr dsptcher_reordr_top_intr_ctrl_1_isr;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get(&dsptcher_reordr_top_intr_ctrl_1_isr);
        bdmf_session_print(session, "qdest0_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest0_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest0_int);
        bdmf_session_print(session, "qdest1_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest1_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest1_int);
        bdmf_session_print(session, "qdest2_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest2_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest2_int);
        bdmf_session_print(session, "qdest3_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest3_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest3_int);
        bdmf_session_print(session, "qdest4_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest4_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest4_int);
        bdmf_session_print(session, "qdest5_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest5_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest5_int);
        bdmf_session_print(session, "qdest6_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest6_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest6_int);
        bdmf_session_print(session, "qdest7_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest7_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest7_int);
        bdmf_session_print(session, "qdest8_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest8_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest8_int);
        bdmf_session_print(session, "qdest9_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest9_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest9_int);
        bdmf_session_print(session, "qdest10_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest10_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest10_int);
        bdmf_session_print(session, "qdest11_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest11_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest11_int);
        bdmf_session_print(session, "qdest12_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest12_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest12_int);
        bdmf_session_print(session, "qdest13_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest13_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest13_int);
        bdmf_session_print(session, "qdest14_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest14_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest14_int);
        bdmf_session_print(session, "qdest15_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest15_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest15_int);
        bdmf_session_print(session, "qdest16_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest16_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest16_int);
        bdmf_session_print(session, "qdest17_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest17_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest17_int);
        bdmf_session_print(session, "qdest18_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest18_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest18_int);
        bdmf_session_print(session, "qdest19_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest19_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest19_int);
        bdmf_session_print(session, "qdest20_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest20_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest20_int);
        bdmf_session_print(session, "qdest21_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest21_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest21_int);
        bdmf_session_print(session, "qdest22_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest22_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest22_int);
        bdmf_session_print(session, "qdest23_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest23_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest23_int);
        bdmf_session_print(session, "qdest24_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest24_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest24_int);
        bdmf_session_print(session, "qdest25_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest25_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest25_int);
        bdmf_session_print(session, "qdest26_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest26_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest26_int);
        bdmf_session_print(session, "qdest27_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest27_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest27_int);
        bdmf_session_print(session, "qdest28_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest28_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest28_int);
        bdmf_session_print(session, "qdest29_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest29_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest29_int);
        bdmf_session_print(session, "qdest30_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest30_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest30_int);
        bdmf_session_print(session, "qdest31_int = %u (0x%x)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest31_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest31_int);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism:
    {
        uint32_t ism;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get(&ism);
        bdmf_session_print(session, "ism = %u (0x%x)\n", ism, ism);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier:
    {
        uint32_t iem;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get(&iem);
        bdmf_session_print(session, "iem = %u (0x%x)\n", iem, iem);
        break;
    }
    case cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr:
    {
        uint32_t ist;
        err = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
        break;
    }
    case cli_dsptchr_debug_dbg_bypss_cntrl:
    {
        bdmf_boolean en_byp;
        uint8_t bbid_non_dly;
        uint8_t bbid_dly;
        err = ag_drv_dsptchr_debug_dbg_bypss_cntrl_get(&en_byp, &bbid_non_dly, &bbid_dly);
        bdmf_session_print(session, "en_byp = %u (0x%x)\n", en_byp, en_byp);
        bdmf_session_print(session, "bbid_non_dly = %u (0x%x)\n", bbid_non_dly, bbid_non_dly);
        bdmf_session_print(session, "bbid_dly = %u (0x%x)\n", bbid_dly, bbid_dly);
        break;
    }
    case cli_dsptchr_debug_glbl_tsk_cnt_0_7:
    {
        dsptchr_debug_glbl_tsk_cnt_0_7 debug_glbl_tsk_cnt_0_7;
        err = ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get(&debug_glbl_tsk_cnt_0_7);
        bdmf_session_print(session, "tsk_cnt_rnr_0 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_0, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_0);
        bdmf_session_print(session, "tsk_cnt_rnr_1 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_1, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_1);
        bdmf_session_print(session, "tsk_cnt_rnr_2 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_2, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_2);
        bdmf_session_print(session, "tsk_cnt_rnr_3 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_3, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_3);
        bdmf_session_print(session, "tsk_cnt_rnr_4 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_4, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_4);
        bdmf_session_print(session, "tsk_cnt_rnr_5 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_5, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_5);
        bdmf_session_print(session, "tsk_cnt_rnr_6 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_6, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_6);
        bdmf_session_print(session, "tsk_cnt_rnr_7 = %u (0x%x)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_7, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_7);
        break;
    }
    case cli_dsptchr_debug_glbl_tsk_cnt_8_15:
    {
        dsptchr_debug_glbl_tsk_cnt_8_15 debug_glbl_tsk_cnt_8_15;
        err = ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get(&debug_glbl_tsk_cnt_8_15);
        bdmf_session_print(session, "tsk_cnt_rnr_8 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_8, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_8);
        bdmf_session_print(session, "tsk_cnt_rnr_9 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_9, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_9);
        bdmf_session_print(session, "tsk_cnt_rnr_10 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_10, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_10);
        bdmf_session_print(session, "tsk_cnt_rnr_11 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_11, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_11);
        bdmf_session_print(session, "tsk_cnt_rnr_12 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_12, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_12);
        bdmf_session_print(session, "tsk_cnt_rnr_13 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_13, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_13);
        bdmf_session_print(session, "tsk_cnt_rnr_14 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_14, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_14);
        bdmf_session_print(session, "tsk_cnt_rnr_15 = %u (0x%x)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_15, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_15);
        break;
    }
    case cli_dsptchr_debug_dbg_bus_cntrl:
    {
        uint8_t dbg_sel;
        err = ag_drv_dsptchr_debug_dbg_bus_cntrl_get(&dbg_sel);
        bdmf_session_print(session, "dbg_sel = %u (0x%x)\n", dbg_sel, dbg_sel);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_0:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_0_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_1:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_1_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_2:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_2_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_3:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_3_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_4:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_4_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_5:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_5_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_6:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_6_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_7:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_7_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_8:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_8_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_9:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_9_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_10:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_10_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_11:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_11_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_12:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_12_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_13:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_13_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_14:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_14_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_15:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_15_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_16:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_16_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_17:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_17_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_18:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_18_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_19:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_19_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_20:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_20_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_21:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_21_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_22:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_22_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_dbg_vec_23:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_dbg_vec_23_get(&dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl:
    {
        uint8_t dbg_mode;
        bdmf_boolean en_cntrs;
        bdmf_boolean clr_cntrs;
        uint8_t dbg_rnr_sel;
        err = ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get(&dbg_mode, &en_cntrs, &clr_cntrs, &dbg_rnr_sel);
        bdmf_session_print(session, "dbg_mode = %u (0x%x)\n", dbg_mode, dbg_mode);
        bdmf_session_print(session, "en_cntrs = %u (0x%x)\n", en_cntrs, en_cntrs);
        bdmf_session_print(session, "clr_cntrs = %u (0x%x)\n", clr_cntrs, clr_cntrs);
        bdmf_session_print(session, "dbg_rnr_sel = %u (0x%x)\n", dbg_rnr_sel, dbg_rnr_sel);
        break;
    }
    case cli_dsptchr_debug_statistics_dbg_cnt:
    {
        uint32_t dbg_vec_val;
        err = ag_drv_dsptchr_debug_statistics_dbg_cnt_get(parm[1].value.unumber, &dbg_vec_val);
        bdmf_session_print(session, "dbg_vec_val = %u (0x%x)\n", dbg_vec_val, dbg_vec_val);
        break;
    }
    case cli_dsptchr_qdes_head:
    {
        uint32_t head;
        err = ag_drv_dsptchr_qdes_head_get(parm[1].value.unumber, &head);
        bdmf_session_print(session, "head = %u (0x%x)\n", head, head);
        break;
    }
    case cli_dsptchr_qdes_bfout:
    {
        dsptchr_qdes_bfout qdes_bfout;
        err = ag_drv_dsptchr_qdes_bfout_get(parm[1].value.unumber, &qdes_bfout);
        bdmf_session_print(session, "qdes_bfout[0] = %u (0x%x)\n", qdes_bfout.qdes_bfout[0], qdes_bfout.qdes_bfout[0]);
        bdmf_session_print(session, "qdes_bfout[1] = %u (0x%x)\n", qdes_bfout.qdes_bfout[1], qdes_bfout.qdes_bfout[1]);
        bdmf_session_print(session, "qdes_bfout[2] = %u (0x%x)\n", qdes_bfout.qdes_bfout[2], qdes_bfout.qdes_bfout[2]);
        bdmf_session_print(session, "qdes_bfout[3] = %u (0x%x)\n", qdes_bfout.qdes_bfout[3], qdes_bfout.qdes_bfout[3]);
        bdmf_session_print(session, "qdes_bfout[4] = %u (0x%x)\n", qdes_bfout.qdes_bfout[4], qdes_bfout.qdes_bfout[4]);
        bdmf_session_print(session, "qdes_bfout[5] = %u (0x%x)\n", qdes_bfout.qdes_bfout[5], qdes_bfout.qdes_bfout[5]);
        bdmf_session_print(session, "qdes_bfout[6] = %u (0x%x)\n", qdes_bfout.qdes_bfout[6], qdes_bfout.qdes_bfout[6]);
        bdmf_session_print(session, "qdes_bfout[7] = %u (0x%x)\n", qdes_bfout.qdes_bfout[7], qdes_bfout.qdes_bfout[7]);
        bdmf_session_print(session, "qdes_bfout[8] = %u (0x%x)\n", qdes_bfout.qdes_bfout[8], qdes_bfout.qdes_bfout[8]);
        bdmf_session_print(session, "qdes_bfout[9] = %u (0x%x)\n", qdes_bfout.qdes_bfout[9], qdes_bfout.qdes_bfout[9]);
        bdmf_session_print(session, "qdes_bfout[10] = %u (0x%x)\n", qdes_bfout.qdes_bfout[10], qdes_bfout.qdes_bfout[10]);
        bdmf_session_print(session, "qdes_bfout[11] = %u (0x%x)\n", qdes_bfout.qdes_bfout[11], qdes_bfout.qdes_bfout[11]);
        bdmf_session_print(session, "qdes_bfout[12] = %u (0x%x)\n", qdes_bfout.qdes_bfout[12], qdes_bfout.qdes_bfout[12]);
        bdmf_session_print(session, "qdes_bfout[13] = %u (0x%x)\n", qdes_bfout.qdes_bfout[13], qdes_bfout.qdes_bfout[13]);
        bdmf_session_print(session, "qdes_bfout[14] = %u (0x%x)\n", qdes_bfout.qdes_bfout[14], qdes_bfout.qdes_bfout[14]);
        bdmf_session_print(session, "qdes_bfout[15] = %u (0x%x)\n", qdes_bfout.qdes_bfout[15], qdes_bfout.qdes_bfout[15]);
        bdmf_session_print(session, "qdes_bfout[16] = %u (0x%x)\n", qdes_bfout.qdes_bfout[16], qdes_bfout.qdes_bfout[16]);
        bdmf_session_print(session, "qdes_bfout[17] = %u (0x%x)\n", qdes_bfout.qdes_bfout[17], qdes_bfout.qdes_bfout[17]);
        bdmf_session_print(session, "qdes_bfout[18] = %u (0x%x)\n", qdes_bfout.qdes_bfout[18], qdes_bfout.qdes_bfout[18]);
        bdmf_session_print(session, "qdes_bfout[19] = %u (0x%x)\n", qdes_bfout.qdes_bfout[19], qdes_bfout.qdes_bfout[19]);
        bdmf_session_print(session, "qdes_bfout[20] = %u (0x%x)\n", qdes_bfout.qdes_bfout[20], qdes_bfout.qdes_bfout[20]);
        bdmf_session_print(session, "qdes_bfout[21] = %u (0x%x)\n", qdes_bfout.qdes_bfout[21], qdes_bfout.qdes_bfout[21]);
        bdmf_session_print(session, "qdes_bfout[22] = %u (0x%x)\n", qdes_bfout.qdes_bfout[22], qdes_bfout.qdes_bfout[22]);
        bdmf_session_print(session, "qdes_bfout[23] = %u (0x%x)\n", qdes_bfout.qdes_bfout[23], qdes_bfout.qdes_bfout[23]);
        bdmf_session_print(session, "qdes_bfout[24] = %u (0x%x)\n", qdes_bfout.qdes_bfout[24], qdes_bfout.qdes_bfout[24]);
        bdmf_session_print(session, "qdes_bfout[25] = %u (0x%x)\n", qdes_bfout.qdes_bfout[25], qdes_bfout.qdes_bfout[25]);
        bdmf_session_print(session, "qdes_bfout[26] = %u (0x%x)\n", qdes_bfout.qdes_bfout[26], qdes_bfout.qdes_bfout[26]);
        bdmf_session_print(session, "qdes_bfout[27] = %u (0x%x)\n", qdes_bfout.qdes_bfout[27], qdes_bfout.qdes_bfout[27]);
        bdmf_session_print(session, "qdes_bfout[28] = %u (0x%x)\n", qdes_bfout.qdes_bfout[28], qdes_bfout.qdes_bfout[28]);
        bdmf_session_print(session, "qdes_bfout[29] = %u (0x%x)\n", qdes_bfout.qdes_bfout[29], qdes_bfout.qdes_bfout[29]);
        bdmf_session_print(session, "qdes_bfout[30] = %u (0x%x)\n", qdes_bfout.qdes_bfout[30], qdes_bfout.qdes_bfout[30]);
        bdmf_session_print(session, "qdes_bfout[31] = %u (0x%x)\n", qdes_bfout.qdes_bfout[31], qdes_bfout.qdes_bfout[31]);
        break;
    }
    case cli_dsptchr_qdes_bufin:
    {
        dsptchr_qdes_bufin qdes_bufin;
        err = ag_drv_dsptchr_qdes_bufin_get(parm[1].value.unumber, &qdes_bufin);
        bdmf_session_print(session, "qdes_bfin[0] = %u (0x%x)\n", qdes_bufin.qdes_bfin[0], qdes_bufin.qdes_bfin[0]);
        bdmf_session_print(session, "qdes_bfin[1] = %u (0x%x)\n", qdes_bufin.qdes_bfin[1], qdes_bufin.qdes_bfin[1]);
        bdmf_session_print(session, "qdes_bfin[2] = %u (0x%x)\n", qdes_bufin.qdes_bfin[2], qdes_bufin.qdes_bfin[2]);
        bdmf_session_print(session, "qdes_bfin[3] = %u (0x%x)\n", qdes_bufin.qdes_bfin[3], qdes_bufin.qdes_bfin[3]);
        bdmf_session_print(session, "qdes_bfin[4] = %u (0x%x)\n", qdes_bufin.qdes_bfin[4], qdes_bufin.qdes_bfin[4]);
        bdmf_session_print(session, "qdes_bfin[5] = %u (0x%x)\n", qdes_bufin.qdes_bfin[5], qdes_bufin.qdes_bfin[5]);
        bdmf_session_print(session, "qdes_bfin[6] = %u (0x%x)\n", qdes_bufin.qdes_bfin[6], qdes_bufin.qdes_bfin[6]);
        bdmf_session_print(session, "qdes_bfin[7] = %u (0x%x)\n", qdes_bufin.qdes_bfin[7], qdes_bufin.qdes_bfin[7]);
        bdmf_session_print(session, "qdes_bfin[8] = %u (0x%x)\n", qdes_bufin.qdes_bfin[8], qdes_bufin.qdes_bfin[8]);
        bdmf_session_print(session, "qdes_bfin[9] = %u (0x%x)\n", qdes_bufin.qdes_bfin[9], qdes_bufin.qdes_bfin[9]);
        bdmf_session_print(session, "qdes_bfin[10] = %u (0x%x)\n", qdes_bufin.qdes_bfin[10], qdes_bufin.qdes_bfin[10]);
        bdmf_session_print(session, "qdes_bfin[11] = %u (0x%x)\n", qdes_bufin.qdes_bfin[11], qdes_bufin.qdes_bfin[11]);
        bdmf_session_print(session, "qdes_bfin[12] = %u (0x%x)\n", qdes_bufin.qdes_bfin[12], qdes_bufin.qdes_bfin[12]);
        bdmf_session_print(session, "qdes_bfin[13] = %u (0x%x)\n", qdes_bufin.qdes_bfin[13], qdes_bufin.qdes_bfin[13]);
        bdmf_session_print(session, "qdes_bfin[14] = %u (0x%x)\n", qdes_bufin.qdes_bfin[14], qdes_bufin.qdes_bfin[14]);
        bdmf_session_print(session, "qdes_bfin[15] = %u (0x%x)\n", qdes_bufin.qdes_bfin[15], qdes_bufin.qdes_bfin[15]);
        bdmf_session_print(session, "qdes_bfin[16] = %u (0x%x)\n", qdes_bufin.qdes_bfin[16], qdes_bufin.qdes_bfin[16]);
        bdmf_session_print(session, "qdes_bfin[17] = %u (0x%x)\n", qdes_bufin.qdes_bfin[17], qdes_bufin.qdes_bfin[17]);
        bdmf_session_print(session, "qdes_bfin[18] = %u (0x%x)\n", qdes_bufin.qdes_bfin[18], qdes_bufin.qdes_bfin[18]);
        bdmf_session_print(session, "qdes_bfin[19] = %u (0x%x)\n", qdes_bufin.qdes_bfin[19], qdes_bufin.qdes_bfin[19]);
        bdmf_session_print(session, "qdes_bfin[20] = %u (0x%x)\n", qdes_bufin.qdes_bfin[20], qdes_bufin.qdes_bfin[20]);
        bdmf_session_print(session, "qdes_bfin[21] = %u (0x%x)\n", qdes_bufin.qdes_bfin[21], qdes_bufin.qdes_bfin[21]);
        bdmf_session_print(session, "qdes_bfin[22] = %u (0x%x)\n", qdes_bufin.qdes_bfin[22], qdes_bufin.qdes_bfin[22]);
        bdmf_session_print(session, "qdes_bfin[23] = %u (0x%x)\n", qdes_bufin.qdes_bfin[23], qdes_bufin.qdes_bfin[23]);
        bdmf_session_print(session, "qdes_bfin[24] = %u (0x%x)\n", qdes_bufin.qdes_bfin[24], qdes_bufin.qdes_bfin[24]);
        bdmf_session_print(session, "qdes_bfin[25] = %u (0x%x)\n", qdes_bufin.qdes_bfin[25], qdes_bufin.qdes_bfin[25]);
        bdmf_session_print(session, "qdes_bfin[26] = %u (0x%x)\n", qdes_bufin.qdes_bfin[26], qdes_bufin.qdes_bfin[26]);
        bdmf_session_print(session, "qdes_bfin[27] = %u (0x%x)\n", qdes_bufin.qdes_bfin[27], qdes_bufin.qdes_bfin[27]);
        bdmf_session_print(session, "qdes_bfin[28] = %u (0x%x)\n", qdes_bufin.qdes_bfin[28], qdes_bufin.qdes_bfin[28]);
        bdmf_session_print(session, "qdes_bfin[29] = %u (0x%x)\n", qdes_bufin.qdes_bfin[29], qdes_bufin.qdes_bfin[29]);
        bdmf_session_print(session, "qdes_bfin[30] = %u (0x%x)\n", qdes_bufin.qdes_bfin[30], qdes_bufin.qdes_bfin[30]);
        bdmf_session_print(session, "qdes_bfin[31] = %u (0x%x)\n", qdes_bufin.qdes_bfin[31], qdes_bufin.qdes_bfin[31]);
        break;
    }
    case cli_dsptchr_qdes_tail:
    {
        uint32_t tail;
        err = ag_drv_dsptchr_qdes_tail_get(parm[1].value.unumber, &tail);
        bdmf_session_print(session, "tail = %u (0x%x)\n", tail, tail);
        break;
    }
    case cli_dsptchr_qdes_fbdnull:
    {
        dsptchr_qdes_fbdnull qdes_fbdnull;
        err = ag_drv_dsptchr_qdes_fbdnull_get(parm[1].value.unumber, &qdes_fbdnull);
        bdmf_session_print(session, "qdes_fbdnull[0] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[0], qdes_fbdnull.qdes_fbdnull[0]);
        bdmf_session_print(session, "qdes_fbdnull[1] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[1], qdes_fbdnull.qdes_fbdnull[1]);
        bdmf_session_print(session, "qdes_fbdnull[2] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[2], qdes_fbdnull.qdes_fbdnull[2]);
        bdmf_session_print(session, "qdes_fbdnull[3] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[3], qdes_fbdnull.qdes_fbdnull[3]);
        bdmf_session_print(session, "qdes_fbdnull[4] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[4], qdes_fbdnull.qdes_fbdnull[4]);
        bdmf_session_print(session, "qdes_fbdnull[5] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[5], qdes_fbdnull.qdes_fbdnull[5]);
        bdmf_session_print(session, "qdes_fbdnull[6] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[6], qdes_fbdnull.qdes_fbdnull[6]);
        bdmf_session_print(session, "qdes_fbdnull[7] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[7], qdes_fbdnull.qdes_fbdnull[7]);
        bdmf_session_print(session, "qdes_fbdnull[8] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[8], qdes_fbdnull.qdes_fbdnull[8]);
        bdmf_session_print(session, "qdes_fbdnull[9] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[9], qdes_fbdnull.qdes_fbdnull[9]);
        bdmf_session_print(session, "qdes_fbdnull[10] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[10], qdes_fbdnull.qdes_fbdnull[10]);
        bdmf_session_print(session, "qdes_fbdnull[11] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[11], qdes_fbdnull.qdes_fbdnull[11]);
        bdmf_session_print(session, "qdes_fbdnull[12] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[12], qdes_fbdnull.qdes_fbdnull[12]);
        bdmf_session_print(session, "qdes_fbdnull[13] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[13], qdes_fbdnull.qdes_fbdnull[13]);
        bdmf_session_print(session, "qdes_fbdnull[14] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[14], qdes_fbdnull.qdes_fbdnull[14]);
        bdmf_session_print(session, "qdes_fbdnull[15] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[15], qdes_fbdnull.qdes_fbdnull[15]);
        bdmf_session_print(session, "qdes_fbdnull[16] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[16], qdes_fbdnull.qdes_fbdnull[16]);
        bdmf_session_print(session, "qdes_fbdnull[17] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[17], qdes_fbdnull.qdes_fbdnull[17]);
        bdmf_session_print(session, "qdes_fbdnull[18] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[18], qdes_fbdnull.qdes_fbdnull[18]);
        bdmf_session_print(session, "qdes_fbdnull[19] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[19], qdes_fbdnull.qdes_fbdnull[19]);
        bdmf_session_print(session, "qdes_fbdnull[20] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[20], qdes_fbdnull.qdes_fbdnull[20]);
        bdmf_session_print(session, "qdes_fbdnull[21] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[21], qdes_fbdnull.qdes_fbdnull[21]);
        bdmf_session_print(session, "qdes_fbdnull[22] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[22], qdes_fbdnull.qdes_fbdnull[22]);
        bdmf_session_print(session, "qdes_fbdnull[23] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[23], qdes_fbdnull.qdes_fbdnull[23]);
        bdmf_session_print(session, "qdes_fbdnull[24] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[24], qdes_fbdnull.qdes_fbdnull[24]);
        bdmf_session_print(session, "qdes_fbdnull[25] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[25], qdes_fbdnull.qdes_fbdnull[25]);
        bdmf_session_print(session, "qdes_fbdnull[26] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[26], qdes_fbdnull.qdes_fbdnull[26]);
        bdmf_session_print(session, "qdes_fbdnull[27] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[27], qdes_fbdnull.qdes_fbdnull[27]);
        bdmf_session_print(session, "qdes_fbdnull[28] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[28], qdes_fbdnull.qdes_fbdnull[28]);
        bdmf_session_print(session, "qdes_fbdnull[29] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[29], qdes_fbdnull.qdes_fbdnull[29]);
        bdmf_session_print(session, "qdes_fbdnull[30] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[30], qdes_fbdnull.qdes_fbdnull[30]);
        bdmf_session_print(session, "qdes_fbdnull[31] = %u (0x%x)\n", qdes_fbdnull.qdes_fbdnull[31], qdes_fbdnull.qdes_fbdnull[31]);
        break;
    }
    case cli_dsptchr_qdes_nullbd:
    {
        dsptchr_qdes_nullbd qdes_nullbd;
        err = ag_drv_dsptchr_qdes_nullbd_get(parm[1].value.unumber, &qdes_nullbd);
        bdmf_session_print(session, "qdes_nullbd[0] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[0], qdes_nullbd.qdes_nullbd[0]);
        bdmf_session_print(session, "qdes_nullbd[1] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[1], qdes_nullbd.qdes_nullbd[1]);
        bdmf_session_print(session, "qdes_nullbd[2] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[2], qdes_nullbd.qdes_nullbd[2]);
        bdmf_session_print(session, "qdes_nullbd[3] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[3], qdes_nullbd.qdes_nullbd[3]);
        bdmf_session_print(session, "qdes_nullbd[4] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[4], qdes_nullbd.qdes_nullbd[4]);
        bdmf_session_print(session, "qdes_nullbd[5] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[5], qdes_nullbd.qdes_nullbd[5]);
        bdmf_session_print(session, "qdes_nullbd[6] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[6], qdes_nullbd.qdes_nullbd[6]);
        bdmf_session_print(session, "qdes_nullbd[7] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[7], qdes_nullbd.qdes_nullbd[7]);
        bdmf_session_print(session, "qdes_nullbd[8] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[8], qdes_nullbd.qdes_nullbd[8]);
        bdmf_session_print(session, "qdes_nullbd[9] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[9], qdes_nullbd.qdes_nullbd[9]);
        bdmf_session_print(session, "qdes_nullbd[10] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[10], qdes_nullbd.qdes_nullbd[10]);
        bdmf_session_print(session, "qdes_nullbd[11] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[11], qdes_nullbd.qdes_nullbd[11]);
        bdmf_session_print(session, "qdes_nullbd[12] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[12], qdes_nullbd.qdes_nullbd[12]);
        bdmf_session_print(session, "qdes_nullbd[13] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[13], qdes_nullbd.qdes_nullbd[13]);
        bdmf_session_print(session, "qdes_nullbd[14] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[14], qdes_nullbd.qdes_nullbd[14]);
        bdmf_session_print(session, "qdes_nullbd[15] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[15], qdes_nullbd.qdes_nullbd[15]);
        bdmf_session_print(session, "qdes_nullbd[16] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[16], qdes_nullbd.qdes_nullbd[16]);
        bdmf_session_print(session, "qdes_nullbd[17] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[17], qdes_nullbd.qdes_nullbd[17]);
        bdmf_session_print(session, "qdes_nullbd[18] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[18], qdes_nullbd.qdes_nullbd[18]);
        bdmf_session_print(session, "qdes_nullbd[19] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[19], qdes_nullbd.qdes_nullbd[19]);
        bdmf_session_print(session, "qdes_nullbd[20] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[20], qdes_nullbd.qdes_nullbd[20]);
        bdmf_session_print(session, "qdes_nullbd[21] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[21], qdes_nullbd.qdes_nullbd[21]);
        bdmf_session_print(session, "qdes_nullbd[22] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[22], qdes_nullbd.qdes_nullbd[22]);
        bdmf_session_print(session, "qdes_nullbd[23] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[23], qdes_nullbd.qdes_nullbd[23]);
        bdmf_session_print(session, "qdes_nullbd[24] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[24], qdes_nullbd.qdes_nullbd[24]);
        bdmf_session_print(session, "qdes_nullbd[25] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[25], qdes_nullbd.qdes_nullbd[25]);
        bdmf_session_print(session, "qdes_nullbd[26] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[26], qdes_nullbd.qdes_nullbd[26]);
        bdmf_session_print(session, "qdes_nullbd[27] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[27], qdes_nullbd.qdes_nullbd[27]);
        bdmf_session_print(session, "qdes_nullbd[28] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[28], qdes_nullbd.qdes_nullbd[28]);
        bdmf_session_print(session, "qdes_nullbd[29] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[29], qdes_nullbd.qdes_nullbd[29]);
        bdmf_session_print(session, "qdes_nullbd[30] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[30], qdes_nullbd.qdes_nullbd[30]);
        bdmf_session_print(session, "qdes_nullbd[31] = %u (0x%x)\n", qdes_nullbd.qdes_nullbd[31], qdes_nullbd.qdes_nullbd[31]);
        break;
    }
    case cli_dsptchr_qdes_bufavail:
    {
        dsptchr_qdes_bufavail qdes_bufavail;
        err = ag_drv_dsptchr_qdes_bufavail_get(parm[1].value.unumber, &qdes_bufavail);
        bdmf_session_print(session, "qdes_bufavail[0] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[0], qdes_bufavail.qdes_bufavail[0]);
        bdmf_session_print(session, "qdes_bufavail[1] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[1], qdes_bufavail.qdes_bufavail[1]);
        bdmf_session_print(session, "qdes_bufavail[2] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[2], qdes_bufavail.qdes_bufavail[2]);
        bdmf_session_print(session, "qdes_bufavail[3] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[3], qdes_bufavail.qdes_bufavail[3]);
        bdmf_session_print(session, "qdes_bufavail[4] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[4], qdes_bufavail.qdes_bufavail[4]);
        bdmf_session_print(session, "qdes_bufavail[5] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[5], qdes_bufavail.qdes_bufavail[5]);
        bdmf_session_print(session, "qdes_bufavail[6] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[6], qdes_bufavail.qdes_bufavail[6]);
        bdmf_session_print(session, "qdes_bufavail[7] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[7], qdes_bufavail.qdes_bufavail[7]);
        bdmf_session_print(session, "qdes_bufavail[8] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[8], qdes_bufavail.qdes_bufavail[8]);
        bdmf_session_print(session, "qdes_bufavail[9] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[9], qdes_bufavail.qdes_bufavail[9]);
        bdmf_session_print(session, "qdes_bufavail[10] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[10], qdes_bufavail.qdes_bufavail[10]);
        bdmf_session_print(session, "qdes_bufavail[11] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[11], qdes_bufavail.qdes_bufavail[11]);
        bdmf_session_print(session, "qdes_bufavail[12] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[12], qdes_bufavail.qdes_bufavail[12]);
        bdmf_session_print(session, "qdes_bufavail[13] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[13], qdes_bufavail.qdes_bufavail[13]);
        bdmf_session_print(session, "qdes_bufavail[14] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[14], qdes_bufavail.qdes_bufavail[14]);
        bdmf_session_print(session, "qdes_bufavail[15] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[15], qdes_bufavail.qdes_bufavail[15]);
        bdmf_session_print(session, "qdes_bufavail[16] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[16], qdes_bufavail.qdes_bufavail[16]);
        bdmf_session_print(session, "qdes_bufavail[17] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[17], qdes_bufavail.qdes_bufavail[17]);
        bdmf_session_print(session, "qdes_bufavail[18] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[18], qdes_bufavail.qdes_bufavail[18]);
        bdmf_session_print(session, "qdes_bufavail[19] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[19], qdes_bufavail.qdes_bufavail[19]);
        bdmf_session_print(session, "qdes_bufavail[20] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[20], qdes_bufavail.qdes_bufavail[20]);
        bdmf_session_print(session, "qdes_bufavail[21] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[21], qdes_bufavail.qdes_bufavail[21]);
        bdmf_session_print(session, "qdes_bufavail[22] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[22], qdes_bufavail.qdes_bufavail[22]);
        bdmf_session_print(session, "qdes_bufavail[23] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[23], qdes_bufavail.qdes_bufavail[23]);
        bdmf_session_print(session, "qdes_bufavail[24] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[24], qdes_bufavail.qdes_bufavail[24]);
        bdmf_session_print(session, "qdes_bufavail[25] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[25], qdes_bufavail.qdes_bufavail[25]);
        bdmf_session_print(session, "qdes_bufavail[26] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[26], qdes_bufavail.qdes_bufavail[26]);
        bdmf_session_print(session, "qdes_bufavail[27] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[27], qdes_bufavail.qdes_bufavail[27]);
        bdmf_session_print(session, "qdes_bufavail[28] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[28], qdes_bufavail.qdes_bufavail[28]);
        bdmf_session_print(session, "qdes_bufavail[29] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[29], qdes_bufavail.qdes_bufavail[29]);
        bdmf_session_print(session, "qdes_bufavail[30] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[30], qdes_bufavail.qdes_bufavail[30]);
        bdmf_session_print(session, "qdes_bufavail[31] = %u (0x%x)\n", qdes_bufavail.qdes_bufavail[31], qdes_bufavail.qdes_bufavail[31]);
        break;
    }
    case cli_dsptchr_qdes_reg_q_head:
    {
        uint16_t head;
        err = ag_drv_dsptchr_qdes_reg_q_head_get(parm[1].value.unumber, &head);
        bdmf_session_print(session, "head = %u (0x%x)\n", head, head);
        break;
    }
    case cli_dsptchr_qdes_reg_viq_head_vld:
    {
        uint32_t viq_head_vld;
        err = ag_drv_dsptchr_qdes_reg_viq_head_vld_get(&viq_head_vld);
        bdmf_session_print(session, "viq_head_vld = %u (0x%x)\n", viq_head_vld, viq_head_vld);
        break;
    }
    case cli_dsptchr_qdes_reg_viq_chrncy_vld:
    {
        uint32_t chrncy_vld;
        err = ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get(&chrncy_vld);
        bdmf_session_print(session, "chrncy_vld = %u (0x%x)\n", chrncy_vld, chrncy_vld);
        break;
    }
    case cli_dsptchr_qdes_reg_veq_head_vld:
    {
        uint32_t viq_head_vld;
        err = ag_drv_dsptchr_qdes_reg_veq_head_vld_get(&viq_head_vld);
        bdmf_session_print(session, "viq_head_vld = %u (0x%x)\n", viq_head_vld, viq_head_vld);
        break;
    }
    case cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl:
    {
        bdmf_boolean use_buf_avl;
        bdmf_boolean dec_bufout_when_mltcst;
        err = ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get(&use_buf_avl, &dec_bufout_when_mltcst);
        bdmf_session_print(session, "use_buf_avl = %u (0x%x)\n", use_buf_avl, use_buf_avl);
        bdmf_session_print(session, "dec_bufout_when_mltcst = %u (0x%x)\n", dec_bufout_when_mltcst, dec_bufout_when_mltcst);
        break;
    }
    case cli_dsptchr_flldes_flldrop:
    {
        uint32_t drpcnt;
        err = ag_drv_dsptchr_flldes_flldrop_get(&drpcnt);
        bdmf_session_print(session, "drpcnt = %u (0x%x)\n", drpcnt, drpcnt);
        break;
    }
    case cli_dsptchr_flldes_bufavail:
    {
        uint32_t bufavail;
        err = ag_drv_dsptchr_flldes_bufavail_get(&bufavail);
        bdmf_session_print(session, "bufavail = %u (0x%x)\n", bufavail, bufavail);
        break;
    }
    case cli_dsptchr_flldes_freemin:
    {
        uint32_t freemin;
        err = ag_drv_dsptchr_flldes_freemin_get(&freemin);
        bdmf_session_print(session, "freemin = %u (0x%x)\n", freemin, freemin);
        break;
    }
    case cli_dsptchr_bdram_next_data:
    {
        uint16_t data;
        err = ag_drv_dsptchr_bdram_next_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_dsptchr_bdram_prev_data:
    {
        uint16_t data;
        err = ag_drv_dsptchr_bdram_prev_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_dsptchr_pdram_data:
    {
        dsptchr_pdram_data pdram_data;
        err = ag_drv_dsptchr_pdram_data_get(parm[1].value.unumber, &pdram_data);
        bdmf_session_print(session, "reorder_ram_data[0] = %u (0x%x)\n", pdram_data.reorder_ram_data[0], pdram_data.reorder_ram_data[0]);
        bdmf_session_print(session, "reorder_ram_data[1] = %u (0x%x)\n", pdram_data.reorder_ram_data[1], pdram_data.reorder_ram_data[1]);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_dsptchr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t viq_idx=gtmv(m, 5);
        dsptchr_cngs_params cngs_params = {.frst_lvl=gtmv(m, 12), .scnd_lvl=gtmv(m, 12), .hyst_thrs=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_cngs_params_set( %u %u %u %u)\n", viq_idx, cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
        if(!err) ag_drv_dsptchr_cngs_params_set(viq_idx, &cngs_params);
        if(!err) ag_drv_dsptchr_cngs_params_get( viq_idx, &cngs_params);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_cngs_params_get( %u %u %u %u)\n", viq_idx, cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
        if(err || cngs_params.frst_lvl!=gtmv(m, 12) || cngs_params.scnd_lvl!=gtmv(m, 12) || cngs_params.hyst_thrs!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_glbl_cngs_params glbl_cngs_params = {.frst_lvl=gtmv(m, 12), .scnd_lvl=gtmv(m, 12), .hyst_thrs=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_glbl_cngs_params_set( %u %u %u)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.scnd_lvl, glbl_cngs_params.hyst_thrs);
        if(!err) ag_drv_dsptchr_glbl_cngs_params_set(&glbl_cngs_params);
        if(!err) ag_drv_dsptchr_glbl_cngs_params_get( &glbl_cngs_params);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_glbl_cngs_params_get( %u %u %u)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.scnd_lvl, glbl_cngs_params.hyst_thrs);
        if(err || glbl_cngs_params.frst_lvl!=gtmv(m, 12) || glbl_cngs_params.scnd_lvl!=gtmv(m, 12) || glbl_cngs_params.hyst_thrs!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint16_t cmn_cnt=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_q_size_params_set( %u %u)\n", q_idx, cmn_cnt);
        if(!err) ag_drv_dsptchr_q_size_params_set(q_idx, cmn_cnt);
        if(!err) ag_drv_dsptchr_q_size_params_get( q_idx, &cmn_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_q_size_params_get( %u %u)\n", q_idx, cmn_cnt);
        if(err || cmn_cnt!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint16_t credit_cnt=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_credit_cnt_set( %u %u)\n", q_idx, credit_cnt);
        if(!err) ag_drv_dsptchr_credit_cnt_set(q_idx, credit_cnt);
        if(!err) ag_drv_dsptchr_credit_cnt_get( q_idx, &credit_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_credit_cnt_get( %u %u)\n", q_idx, credit_cnt);
        if(err || credit_cnt!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint16_t cmn_max=gtmv(m, 10);
        uint16_t gurntd_max=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_q_limits_params_set( %u %u %u)\n", q_idx, cmn_max, gurntd_max);
        if(!err) ag_drv_dsptchr_q_limits_params_set(q_idx, cmn_max, gurntd_max);
        if(!err) ag_drv_dsptchr_q_limits_params_get( q_idx, &cmn_max, &gurntd_max);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_q_limits_params_get( %u %u %u)\n", q_idx, cmn_max, gurntd_max);
        if(err || cmn_max!=gtmv(m, 10) || gurntd_max!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        bdmf_boolean chrncy_en=gtmv(m, 1);
        uint16_t chrncy_cnt=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_ingress_coherency_params_set( %u %u %u)\n", q_idx, chrncy_en, chrncy_cnt);
        if(!err) ag_drv_dsptchr_ingress_coherency_params_set(q_idx, chrncy_en, chrncy_cnt);
        if(!err) ag_drv_dsptchr_ingress_coherency_params_get( q_idx, &chrncy_en, &chrncy_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_ingress_coherency_params_get( %u %u %u)\n", q_idx, chrncy_en, chrncy_cnt);
        if(err || chrncy_en!=gtmv(m, 1) || chrncy_cnt!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_pools_limits pools_limits = {.cmn_pool_lmt=gtmv(m, 10), .grnted_pool_lmt=gtmv(m, 10), .mcast_pool_lmt=gtmv(m, 10), .rnr_pool_lmt=gtmv(m, 10), .cmn_pool_size=gtmv(m, 10), .grnted_pool_size=gtmv(m, 10), .mcast_pool_size=gtmv(m, 10), .rnr_pool_size=gtmv(m, 10), .processing_pool_size=gtmv(m, 10)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_pools_limits_set( %u %u %u %u %u %u %u %u %u)\n", pools_limits.cmn_pool_lmt, pools_limits.grnted_pool_lmt, pools_limits.mcast_pool_lmt, pools_limits.rnr_pool_lmt, pools_limits.cmn_pool_size, pools_limits.grnted_pool_size, pools_limits.mcast_pool_size, pools_limits.rnr_pool_size, pools_limits.processing_pool_size);
        if(!err) ag_drv_dsptchr_pools_limits_set(&pools_limits);
        if(!err) ag_drv_dsptchr_pools_limits_get( &pools_limits);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_pools_limits_get( %u %u %u %u %u %u %u %u %u)\n", pools_limits.cmn_pool_lmt, pools_limits.grnted_pool_lmt, pools_limits.mcast_pool_lmt, pools_limits.rnr_pool_lmt, pools_limits.cmn_pool_size, pools_limits.grnted_pool_size, pools_limits.mcast_pool_size, pools_limits.rnr_pool_size, pools_limits.processing_pool_size);
        if(err || pools_limits.cmn_pool_lmt!=gtmv(m, 10) || pools_limits.grnted_pool_lmt!=gtmv(m, 10) || pools_limits.mcast_pool_lmt!=gtmv(m, 10) || pools_limits.rnr_pool_lmt!=gtmv(m, 10) || pools_limits.cmn_pool_size!=gtmv(m, 10) || pools_limits.grnted_pool_size!=gtmv(m, 10) || pools_limits.mcast_pool_size!=gtmv(m, 10) || pools_limits.rnr_pool_size!=gtmv(m, 10) || pools_limits.processing_pool_size!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_fll_entry fll_entry = {.head=gtmv(m, 32), .tail=gtmv(m, 32), .minbuf=gtmv(m, 32), .bfin=gtmv(m, 32), .count=gtmv(m, 32)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_fll_entry_set( %u %u %u %u %u)\n", fll_entry.head, fll_entry.tail, fll_entry.minbuf, fll_entry.bfin, fll_entry.count);
        if(!err) ag_drv_dsptchr_fll_entry_set(&fll_entry);
        if(!err) ag_drv_dsptchr_fll_entry_get( &fll_entry);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_fll_entry_get( %u %u %u %u %u)\n", fll_entry.head, fll_entry.tail, fll_entry.minbuf, fll_entry.bfin, fll_entry.count);
        if(err || fll_entry.head!=gtmv(m, 32) || fll_entry.tail!=gtmv(m, 32) || fll_entry.minbuf!=gtmv(m, 32) || fll_entry.bfin!=gtmv(m, 32) || fll_entry.count!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rnr_idx=gtmv(m, 4);
        dsptchr_rnr_dsptch_addr rnr_dsptch_addr = {.base_add=gtmv(m, 16), .offset_add=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_rnr_dsptch_addr_set( %u %u %u)\n", rnr_idx, rnr_dsptch_addr.base_add, rnr_dsptch_addr.offset_add);
        if(!err) ag_drv_dsptchr_rnr_dsptch_addr_set(rnr_idx, &rnr_dsptch_addr);
        if(!err) ag_drv_dsptchr_rnr_dsptch_addr_get( rnr_idx, &rnr_dsptch_addr);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_rnr_dsptch_addr_get( %u %u %u)\n", rnr_idx, rnr_dsptch_addr.base_add, rnr_dsptch_addr.offset_add);
        if(err || rnr_dsptch_addr.base_add!=gtmv(m, 16) || rnr_dsptch_addr.offset_add!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_reorder_cfg_dsptchr_reordr_cfg reorder_cfg_dsptchr_reordr_cfg = {.disp_enable=gtmv(m, 1), .rdy=gtmv(m, 1), .reordr_par_mod=gtmv(m, 1), .per_q_egrs_congst_en=gtmv(m, 1), .dsptch_sm_enh_mod=gtmv(m, 1), .ingrs_pipe_dly_en=gtmv(m, 1), .ingrs_pipe_dly_cnt=gtmv(m, 3), .egrs_drop_only=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set( %u %u %u %u %u %u %u %u)\n", reorder_cfg_dsptchr_reordr_cfg.disp_enable, reorder_cfg_dsptchr_reordr_cfg.rdy, reorder_cfg_dsptchr_reordr_cfg.reordr_par_mod, reorder_cfg_dsptchr_reordr_cfg.per_q_egrs_congst_en, reorder_cfg_dsptchr_reordr_cfg.dsptch_sm_enh_mod, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_en, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_cnt, reorder_cfg_dsptchr_reordr_cfg.egrs_drop_only);
        if(!err) ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(&reorder_cfg_dsptchr_reordr_cfg);
        if(!err) ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get( &reorder_cfg_dsptchr_reordr_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get( %u %u %u %u %u %u %u %u)\n", reorder_cfg_dsptchr_reordr_cfg.disp_enable, reorder_cfg_dsptchr_reordr_cfg.rdy, reorder_cfg_dsptchr_reordr_cfg.reordr_par_mod, reorder_cfg_dsptchr_reordr_cfg.per_q_egrs_congst_en, reorder_cfg_dsptchr_reordr_cfg.dsptch_sm_enh_mod, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_en, reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_cnt, reorder_cfg_dsptchr_reordr_cfg.egrs_drop_only);
        if(err || reorder_cfg_dsptchr_reordr_cfg.disp_enable!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.rdy!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.reordr_par_mod!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.per_q_egrs_congst_en!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.dsptch_sm_enh_mod!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_en!=gtmv(m, 1) || reorder_cfg_dsptchr_reordr_cfg.ingrs_pipe_dly_cnt!=gtmv(m, 3) || reorder_cfg_dsptchr_reordr_cfg.egrs_drop_only!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t en=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_vq_en_set( %u)\n", en);
        if(!err) ag_drv_dsptchr_reorder_cfg_vq_en_set(en);
        if(!err) ag_drv_dsptchr_reorder_cfg_vq_en_get( &en);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_vq_en_get( %u)\n", en);
        if(err || en!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t src_id=gtmv(m, 6);
        uint8_t dst_id_ovride=gtmv(m, 6);
        uint16_t route_ovride=gtmv(m, 10);
        bdmf_boolean ovride_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_bb_cfg_set( %u %u %u %u)\n", src_id, dst_id_ovride, route_ovride, ovride_en);
        if(!err) ag_drv_dsptchr_reorder_cfg_bb_cfg_set(src_id, dst_id_ovride, route_ovride, ovride_en);
        if(!err) ag_drv_dsptchr_reorder_cfg_bb_cfg_get( &src_id, &dst_id_ovride, &route_ovride, &ovride_en);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_bb_cfg_get( %u %u %u %u)\n", src_id, dst_id_ovride, route_ovride, ovride_en);
        if(err || src_id!=gtmv(m, 6) || dst_id_ovride!=gtmv(m, 6) || route_ovride!=gtmv(m, 10) || ovride_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_reorder_cfg_clk_gate_cntrl reorder_cfg_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set( %u %u %u %u %u)\n", reorder_cfg_clk_gate_cntrl.bypass_clk_gate, reorder_cfg_clk_gate_cntrl.timer_val, reorder_cfg_clk_gate_cntrl.keep_alive_en, reorder_cfg_clk_gate_cntrl.keep_alive_intrvl, reorder_cfg_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(&reorder_cfg_clk_gate_cntrl);
        if(!err) ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get( &reorder_cfg_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get( %u %u %u %u %u)\n", reorder_cfg_clk_gate_cntrl.bypass_clk_gate, reorder_cfg_clk_gate_cntrl.timer_val, reorder_cfg_clk_gate_cntrl.keep_alive_en, reorder_cfg_clk_gate_cntrl.keep_alive_intrvl, reorder_cfg_clk_gate_cntrl.keep_alive_cyc);
        if(err || reorder_cfg_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || reorder_cfg_clk_gate_cntrl.timer_val!=gtmv(m, 8) || reorder_cfg_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || reorder_cfg_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || reorder_cfg_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t viq_idx=gtmv(m, 5);
        dsptchr_cngs_params cngs_params = {.frst_lvl=gtmv(m, 12), .scnd_lvl=gtmv(m, 12), .hyst_thrs=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_egrs_congstn_set( %u %u %u %u)\n", viq_idx, cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
        if(!err) ag_drv_dsptchr_congestion_egrs_congstn_set(viq_idx, &cngs_params);
        if(!err) ag_drv_dsptchr_congestion_egrs_congstn_get( viq_idx, &cngs_params);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_egrs_congstn_get( %u %u %u %u)\n", viq_idx, cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
        if(err || cngs_params.frst_lvl!=gtmv(m, 12) || cngs_params.scnd_lvl!=gtmv(m, 12) || cngs_params.hyst_thrs!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_glbl_cngs_params glbl_cngs_params = {.frst_lvl=gtmv(m, 12), .scnd_lvl=gtmv(m, 12), .hyst_thrs=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_total_egrs_congstn_set( %u %u %u)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.scnd_lvl, glbl_cngs_params.hyst_thrs);
        if(!err) ag_drv_dsptchr_congestion_total_egrs_congstn_set(&glbl_cngs_params);
        if(!err) ag_drv_dsptchr_congestion_total_egrs_congstn_get( &glbl_cngs_params);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_total_egrs_congstn_get( %u %u %u)\n", glbl_cngs_params.frst_lvl, glbl_cngs_params.scnd_lvl, glbl_cngs_params.hyst_thrs);
        if(err || glbl_cngs_params.frst_lvl!=gtmv(m, 12) || glbl_cngs_params.scnd_lvl!=gtmv(m, 12) || glbl_cngs_params.hyst_thrs!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_congestion_congstn_status congestion_congstn_status = {.glbl_congstn=gtmv(m, 2), .glbl_egrs_congstn=gtmv(m, 2), .sbpm_congstn=gtmv(m, 2), .glbl_congstn_stcky=gtmv(m, 2), .glbl_egrs_congstn_stcky=gtmv(m, 2), .sbpm_congstn_stcky=gtmv(m, 2)};
        if(!err) ag_drv_dsptchr_congestion_congstn_status_get( &congestion_congstn_status);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_congstn_status_get( %u %u %u %u %u %u)\n", congestion_congstn_status.glbl_congstn, congestion_congstn_status.glbl_egrs_congstn, congestion_congstn_status.sbpm_congstn, congestion_congstn_status.glbl_congstn_stcky, congestion_congstn_status.glbl_egrs_congstn_stcky, congestion_congstn_status.sbpm_congstn_stcky);
    }
    {
        uint32_t congstn_state=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get( &congstn_state);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get( %u)\n", congstn_state);
    }
    {
        uint32_t congstn_state=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get( &congstn_state);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get( %u)\n", congstn_state);
    }
    {
        uint32_t congstn_state=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get( &congstn_state);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get( %u)\n", congstn_state);
    }
    {
        uint32_t congstn_state=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get( &congstn_state);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get( %u)\n", congstn_state);
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint8_t bb_id=gtmv(m, 8);
        uint16_t trgt_add=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_queue_mapping_crdt_cfg_set( %u %u %u)\n", q_idx, bb_id, trgt_add);
        if(!err) ag_drv_dsptchr_queue_mapping_crdt_cfg_set(q_idx, bb_id, trgt_add);
        if(!err) ag_drv_dsptchr_queue_mapping_crdt_cfg_get( q_idx, &bb_id, &trgt_add);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_queue_mapping_crdt_cfg_get( %u %u %u)\n", q_idx, bb_id, trgt_add);
        if(err || bb_id!=gtmv(m, 8) || trgt_add!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        bdmf_boolean is_dest_disp=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_queue_mapping_q_dest_set( %u %u)\n", q_idx, is_dest_disp);
        if(!err) ag_drv_dsptchr_queue_mapping_q_dest_set(q_idx, is_dest_disp);
        if(!err) ag_drv_dsptchr_queue_mapping_q_dest_get( q_idx, &is_dest_disp);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_queue_mapping_q_dest_get( %u %u)\n", q_idx, is_dest_disp);
        if(err || is_dest_disp!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t group_idx=gtmv(m, 3);
        dsptchr_mask_msk_tsk_255_0 mask_msk_tsk_255_0 = {.task_mask={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_msk_tsk_255_0_set( %u %u %u %u %u %u %u %u %u)\n", group_idx, mask_msk_tsk_255_0.task_mask[0], mask_msk_tsk_255_0.task_mask[1], mask_msk_tsk_255_0.task_mask[2], mask_msk_tsk_255_0.task_mask[3], mask_msk_tsk_255_0.task_mask[4], mask_msk_tsk_255_0.task_mask[5], mask_msk_tsk_255_0.task_mask[6], mask_msk_tsk_255_0.task_mask[7]);
        if(!err) ag_drv_dsptchr_mask_msk_tsk_255_0_set(group_idx, &mask_msk_tsk_255_0);
        if(!err) ag_drv_dsptchr_mask_msk_tsk_255_0_get( group_idx, &mask_msk_tsk_255_0);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_msk_tsk_255_0_get( %u %u %u %u %u %u %u %u %u)\n", group_idx, mask_msk_tsk_255_0.task_mask[0], mask_msk_tsk_255_0.task_mask[1], mask_msk_tsk_255_0.task_mask[2], mask_msk_tsk_255_0.task_mask[3], mask_msk_tsk_255_0.task_mask[4], mask_msk_tsk_255_0.task_mask[5], mask_msk_tsk_255_0.task_mask[6], mask_msk_tsk_255_0.task_mask[7]);
        if(err || mask_msk_tsk_255_0.task_mask[0]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[1]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[2]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[3]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[4]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[5]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[6]!=gtmv(m, 32) || mask_msk_tsk_255_0.task_mask[7]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t group_idx=gtmv(m, 3);
        uint32_t mask=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_msk_q_set( %u %u)\n", group_idx, mask);
        if(!err) ag_drv_dsptchr_mask_msk_q_set(group_idx, mask);
        if(!err) ag_drv_dsptchr_mask_msk_q_get( group_idx, &mask);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_msk_q_get( %u %u)\n", group_idx, mask);
        if(err || mask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        bdmf_boolean set_delay=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_dly_q_set( %u %u)\n", q_idx, set_delay);
        if(!err) ag_drv_dsptchr_mask_dly_q_set(q_idx, set_delay);
        if(!err) ag_drv_dsptchr_mask_dly_q_get( q_idx, &set_delay);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_dly_q_get( %u %u)\n", q_idx, set_delay);
        if(err || set_delay!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        bdmf_boolean set_non_delay=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_non_dly_q_set( %u %u)\n", q_idx, set_non_delay);
        if(!err) ag_drv_dsptchr_mask_non_dly_q_set(q_idx, set_non_delay);
        if(!err) ag_drv_dsptchr_mask_non_dly_q_get( q_idx, &set_non_delay);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_mask_non_dly_q_get( %u %u)\n", q_idx, set_non_delay);
        if(err || set_non_delay!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dly_crdt=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set( %u)\n", dly_crdt);
        if(!err) ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set(dly_crdt);
        if(!err) ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get( &dly_crdt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get( %u)\n", dly_crdt);
        if(err || dly_crdt!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t non_dly_crdt=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set( %u)\n", non_dly_crdt);
        if(!err) ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set(non_dly_crdt);
        if(!err) ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get( &non_dly_crdt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get( %u)\n", non_dly_crdt);
        if(err || non_dly_crdt!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t total_egrs_size=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set( %u)\n", total_egrs_size);
        if(!err) ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set(total_egrs_size);
        if(!err) ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get( &total_egrs_size);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get( %u)\n", total_egrs_size);
        if(err || total_egrs_size!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        uint16_t q_egrs_size=gtmv(m, 10);
        if(!err) ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get( q_idx, &q_egrs_size);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get( %u %u)\n", q_idx, q_egrs_size);
    }
    {
        dsptchr_wakeup_control_wkup_req wakeup_control_wkup_req = {.q0=gtmv(m, 1), .q1=gtmv(m, 1), .q2=gtmv(m, 1), .q3=gtmv(m, 1), .q4=gtmv(m, 1), .q5=gtmv(m, 1), .q6=gtmv(m, 1), .q7=gtmv(m, 1), .q8=gtmv(m, 1), .q9=gtmv(m, 1), .q10=gtmv(m, 1), .q11=gtmv(m, 1), .q12=gtmv(m, 1), .q13=gtmv(m, 1), .q14=gtmv(m, 1), .q15=gtmv(m, 1), .q16=gtmv(m, 1), .q17=gtmv(m, 1), .q18=gtmv(m, 1), .q19=gtmv(m, 1), .q20=gtmv(m, 1), .q21=gtmv(m, 1), .q22=gtmv(m, 1), .q23=gtmv(m, 1), .q24=gtmv(m, 1), .q25=gtmv(m, 1), .q26=gtmv(m, 1), .q27=gtmv(m, 1), .q28=gtmv(m, 1), .q29=gtmv(m, 1), .q30=gtmv(m, 1), .q31=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_wakeup_control_wkup_req_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", wakeup_control_wkup_req.q0, wakeup_control_wkup_req.q1, wakeup_control_wkup_req.q2, wakeup_control_wkup_req.q3, wakeup_control_wkup_req.q4, wakeup_control_wkup_req.q5, wakeup_control_wkup_req.q6, wakeup_control_wkup_req.q7, wakeup_control_wkup_req.q8, wakeup_control_wkup_req.q9, wakeup_control_wkup_req.q10, wakeup_control_wkup_req.q11, wakeup_control_wkup_req.q12, wakeup_control_wkup_req.q13, wakeup_control_wkup_req.q14, wakeup_control_wkup_req.q15, wakeup_control_wkup_req.q16, wakeup_control_wkup_req.q17, wakeup_control_wkup_req.q18, wakeup_control_wkup_req.q19, wakeup_control_wkup_req.q20, wakeup_control_wkup_req.q21, wakeup_control_wkup_req.q22, wakeup_control_wkup_req.q23, wakeup_control_wkup_req.q24, wakeup_control_wkup_req.q25, wakeup_control_wkup_req.q26, wakeup_control_wkup_req.q27, wakeup_control_wkup_req.q28, wakeup_control_wkup_req.q29, wakeup_control_wkup_req.q30, wakeup_control_wkup_req.q31);
        if(!err) ag_drv_dsptchr_wakeup_control_wkup_req_set(&wakeup_control_wkup_req);
        if(!err) ag_drv_dsptchr_wakeup_control_wkup_req_get( &wakeup_control_wkup_req);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_wakeup_control_wkup_req_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", wakeup_control_wkup_req.q0, wakeup_control_wkup_req.q1, wakeup_control_wkup_req.q2, wakeup_control_wkup_req.q3, wakeup_control_wkup_req.q4, wakeup_control_wkup_req.q5, wakeup_control_wkup_req.q6, wakeup_control_wkup_req.q7, wakeup_control_wkup_req.q8, wakeup_control_wkup_req.q9, wakeup_control_wkup_req.q10, wakeup_control_wkup_req.q11, wakeup_control_wkup_req.q12, wakeup_control_wkup_req.q13, wakeup_control_wkup_req.q14, wakeup_control_wkup_req.q15, wakeup_control_wkup_req.q16, wakeup_control_wkup_req.q17, wakeup_control_wkup_req.q18, wakeup_control_wkup_req.q19, wakeup_control_wkup_req.q20, wakeup_control_wkup_req.q21, wakeup_control_wkup_req.q22, wakeup_control_wkup_req.q23, wakeup_control_wkup_req.q24, wakeup_control_wkup_req.q25, wakeup_control_wkup_req.q26, wakeup_control_wkup_req.q27, wakeup_control_wkup_req.q28, wakeup_control_wkup_req.q29, wakeup_control_wkup_req.q30, wakeup_control_wkup_req.q31);
        if(err || wakeup_control_wkup_req.q0!=gtmv(m, 1) || wakeup_control_wkup_req.q1!=gtmv(m, 1) || wakeup_control_wkup_req.q2!=gtmv(m, 1) || wakeup_control_wkup_req.q3!=gtmv(m, 1) || wakeup_control_wkup_req.q4!=gtmv(m, 1) || wakeup_control_wkup_req.q5!=gtmv(m, 1) || wakeup_control_wkup_req.q6!=gtmv(m, 1) || wakeup_control_wkup_req.q7!=gtmv(m, 1) || wakeup_control_wkup_req.q8!=gtmv(m, 1) || wakeup_control_wkup_req.q9!=gtmv(m, 1) || wakeup_control_wkup_req.q10!=gtmv(m, 1) || wakeup_control_wkup_req.q11!=gtmv(m, 1) || wakeup_control_wkup_req.q12!=gtmv(m, 1) || wakeup_control_wkup_req.q13!=gtmv(m, 1) || wakeup_control_wkup_req.q14!=gtmv(m, 1) || wakeup_control_wkup_req.q15!=gtmv(m, 1) || wakeup_control_wkup_req.q16!=gtmv(m, 1) || wakeup_control_wkup_req.q17!=gtmv(m, 1) || wakeup_control_wkup_req.q18!=gtmv(m, 1) || wakeup_control_wkup_req.q19!=gtmv(m, 1) || wakeup_control_wkup_req.q20!=gtmv(m, 1) || wakeup_control_wkup_req.q21!=gtmv(m, 1) || wakeup_control_wkup_req.q22!=gtmv(m, 1) || wakeup_control_wkup_req.q23!=gtmv(m, 1) || wakeup_control_wkup_req.q24!=gtmv(m, 1) || wakeup_control_wkup_req.q25!=gtmv(m, 1) || wakeup_control_wkup_req.q26!=gtmv(m, 1) || wakeup_control_wkup_req.q27!=gtmv(m, 1) || wakeup_control_wkup_req.q28!=gtmv(m, 1) || wakeup_control_wkup_req.q29!=gtmv(m, 1) || wakeup_control_wkup_req.q30!=gtmv(m, 1) || wakeup_control_wkup_req.q31!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t wkup_thrshld=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_wakeup_control_wkup_thrshld_set( %u)\n", wkup_thrshld);
        if(!err) ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(wkup_thrshld);
        if(!err) ag_drv_dsptchr_wakeup_control_wkup_thrshld_get( &wkup_thrshld);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_wakeup_control_wkup_thrshld_get( %u)\n", wkup_thrshld);
        if(err || wkup_thrshld!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dwrr_q_idx=gtmv(m, 5);
        uint32_t q_crdt=gtmv(m, 20);
        bdmf_boolean ngtv=gtmv(m, 1);
        uint16_t quntum=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_disptch_scheduling_dwrr_info_set( %u %u %u %u)\n", dwrr_q_idx, q_crdt, ngtv, quntum);
        if(!err) ag_drv_dsptchr_disptch_scheduling_dwrr_info_set(dwrr_q_idx, q_crdt, ngtv, quntum);
        if(!err) ag_drv_dsptchr_disptch_scheduling_dwrr_info_get( dwrr_q_idx, &q_crdt, &ngtv, &quntum);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_disptch_scheduling_dwrr_info_get( %u %u %u %u)\n", dwrr_q_idx, q_crdt, ngtv, quntum);
        if(err || q_crdt!=gtmv(m, 20) || ngtv!=gtmv(m, 1) || quntum!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_disptch_scheduling_vld_crdt disptch_scheduling_vld_crdt = {.q0=gtmv(m, 1), .q1=gtmv(m, 1), .q2=gtmv(m, 1), .q3=gtmv(m, 1), .q4=gtmv(m, 1), .q5=gtmv(m, 1), .q6=gtmv(m, 1), .q7=gtmv(m, 1), .q8=gtmv(m, 1), .q9=gtmv(m, 1), .q10=gtmv(m, 1), .q11=gtmv(m, 1), .q12=gtmv(m, 1), .q13=gtmv(m, 1), .q14=gtmv(m, 1), .q15=gtmv(m, 1), .q16=gtmv(m, 1), .q17=gtmv(m, 1), .q18=gtmv(m, 1), .q19=gtmv(m, 1), .q20=gtmv(m, 1), .q21=gtmv(m, 1), .q22=gtmv(m, 1), .q23=gtmv(m, 1), .q24=gtmv(m, 1), .q25=gtmv(m, 1), .q26=gtmv(m, 1), .q27=gtmv(m, 1), .q28=gtmv(m, 1), .q29=gtmv(m, 1), .q30=gtmv(m, 1), .q31=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_disptch_scheduling_vld_crdt_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", disptch_scheduling_vld_crdt.q0, disptch_scheduling_vld_crdt.q1, disptch_scheduling_vld_crdt.q2, disptch_scheduling_vld_crdt.q3, disptch_scheduling_vld_crdt.q4, disptch_scheduling_vld_crdt.q5, disptch_scheduling_vld_crdt.q6, disptch_scheduling_vld_crdt.q7, disptch_scheduling_vld_crdt.q8, disptch_scheduling_vld_crdt.q9, disptch_scheduling_vld_crdt.q10, disptch_scheduling_vld_crdt.q11, disptch_scheduling_vld_crdt.q12, disptch_scheduling_vld_crdt.q13, disptch_scheduling_vld_crdt.q14, disptch_scheduling_vld_crdt.q15, disptch_scheduling_vld_crdt.q16, disptch_scheduling_vld_crdt.q17, disptch_scheduling_vld_crdt.q18, disptch_scheduling_vld_crdt.q19, disptch_scheduling_vld_crdt.q20, disptch_scheduling_vld_crdt.q21, disptch_scheduling_vld_crdt.q22, disptch_scheduling_vld_crdt.q23, disptch_scheduling_vld_crdt.q24, disptch_scheduling_vld_crdt.q25, disptch_scheduling_vld_crdt.q26, disptch_scheduling_vld_crdt.q27, disptch_scheduling_vld_crdt.q28, disptch_scheduling_vld_crdt.q29, disptch_scheduling_vld_crdt.q30, disptch_scheduling_vld_crdt.q31);
        if(!err) ag_drv_dsptchr_disptch_scheduling_vld_crdt_set(&disptch_scheduling_vld_crdt);
        if(!err) ag_drv_dsptchr_disptch_scheduling_vld_crdt_get( &disptch_scheduling_vld_crdt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_disptch_scheduling_vld_crdt_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", disptch_scheduling_vld_crdt.q0, disptch_scheduling_vld_crdt.q1, disptch_scheduling_vld_crdt.q2, disptch_scheduling_vld_crdt.q3, disptch_scheduling_vld_crdt.q4, disptch_scheduling_vld_crdt.q5, disptch_scheduling_vld_crdt.q6, disptch_scheduling_vld_crdt.q7, disptch_scheduling_vld_crdt.q8, disptch_scheduling_vld_crdt.q9, disptch_scheduling_vld_crdt.q10, disptch_scheduling_vld_crdt.q11, disptch_scheduling_vld_crdt.q12, disptch_scheduling_vld_crdt.q13, disptch_scheduling_vld_crdt.q14, disptch_scheduling_vld_crdt.q15, disptch_scheduling_vld_crdt.q16, disptch_scheduling_vld_crdt.q17, disptch_scheduling_vld_crdt.q18, disptch_scheduling_vld_crdt.q19, disptch_scheduling_vld_crdt.q20, disptch_scheduling_vld_crdt.q21, disptch_scheduling_vld_crdt.q22, disptch_scheduling_vld_crdt.q23, disptch_scheduling_vld_crdt.q24, disptch_scheduling_vld_crdt.q25, disptch_scheduling_vld_crdt.q26, disptch_scheduling_vld_crdt.q27, disptch_scheduling_vld_crdt.q28, disptch_scheduling_vld_crdt.q29, disptch_scheduling_vld_crdt.q30, disptch_scheduling_vld_crdt.q31);
        if(err || disptch_scheduling_vld_crdt.q0!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q1!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q2!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q3!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q4!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q5!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q6!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q7!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q8!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q9!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q10!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q11!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q12!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q13!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q14!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q15!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q16!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q17!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q18!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q19!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q20!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q21!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q22!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q23!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q24!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q25!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q26!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q27!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q28!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q29!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q30!=gtmv(m, 1) || disptch_scheduling_vld_crdt.q31!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean lb_mode=gtmv(m, 1);
        uint8_t sp_thrshld=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_lb_cfg_set( %u %u)\n", lb_mode, sp_thrshld);
        if(!err) ag_drv_dsptchr_load_balancing_lb_cfg_set(lb_mode, sp_thrshld);
        if(!err) ag_drv_dsptchr_load_balancing_lb_cfg_get( &lb_mode, &sp_thrshld);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_lb_cfg_get( %u %u)\n", lb_mode, sp_thrshld);
        if(err || lb_mode!=gtmv(m, 1) || sp_thrshld!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr0=gtmv(m, 16);
        uint16_t rnr1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_0_1_set( %u %u)\n", rnr0, rnr1);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_0_1_set(rnr0, rnr1);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_0_1_get( &rnr0, &rnr1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_0_1_get( %u %u)\n", rnr0, rnr1);
        if(err || rnr0!=gtmv(m, 16) || rnr1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr2=gtmv(m, 16);
        uint16_t rnr3=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_2_3_set( %u %u)\n", rnr2, rnr3);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_2_3_set(rnr2, rnr3);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_2_3_get( &rnr2, &rnr3);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_2_3_get( %u %u)\n", rnr2, rnr3);
        if(err || rnr2!=gtmv(m, 16) || rnr3!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr4=gtmv(m, 16);
        uint16_t rnr5=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_4_5_set( %u %u)\n", rnr4, rnr5);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_4_5_set(rnr4, rnr5);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_4_5_get( &rnr4, &rnr5);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_4_5_get( %u %u)\n", rnr4, rnr5);
        if(err || rnr4!=gtmv(m, 16) || rnr5!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr6=gtmv(m, 16);
        uint16_t rnr7=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_6_7_set( %u %u)\n", rnr6, rnr7);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_6_7_set(rnr6, rnr7);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_6_7_get( &rnr6, &rnr7);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_6_7_get( %u %u)\n", rnr6, rnr7);
        if(err || rnr6!=gtmv(m, 16) || rnr7!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr8=gtmv(m, 16);
        uint16_t rnr9=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_8_9_set( %u %u)\n", rnr8, rnr9);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_8_9_set(rnr8, rnr9);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_8_9_get( &rnr8, &rnr9);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_8_9_get( %u %u)\n", rnr8, rnr9);
        if(err || rnr8!=gtmv(m, 16) || rnr9!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr10=gtmv(m, 16);
        uint16_t rnr11=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_10_11_set( %u %u)\n", rnr10, rnr11);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_10_11_set(rnr10, rnr11);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_10_11_get( &rnr10, &rnr11);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_10_11_get( %u %u)\n", rnr10, rnr11);
        if(err || rnr10!=gtmv(m, 16) || rnr11!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr12=gtmv(m, 16);
        uint16_t rnr13=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_12_13_set( %u %u)\n", rnr12, rnr13);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_12_13_set(rnr12, rnr13);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_12_13_get( &rnr12, &rnr13);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_12_13_get( %u %u)\n", rnr12, rnr13);
        if(err || rnr12!=gtmv(m, 16) || rnr13!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr14=gtmv(m, 16);
        uint16_t rnr15=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_14_15_set( %u %u)\n", rnr14, rnr15);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_14_15_set(rnr14, rnr15);
        if(!err) ag_drv_dsptchr_load_balancing_free_task_14_15_get( &rnr14, &rnr15);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_free_task_14_15_get( %u %u)\n", rnr14, rnr15);
        if(err || rnr14!=gtmv(m, 16) || rnr15!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t task_to_rg_mapping=gtmv(m, 5);
        dsptchr_load_balancing_tsk_to_rg_mapping load_balancing_tsk_to_rg_mapping = {.tsk0=gtmv(m, 3), .tsk1=gtmv(m, 3), .tsk2=gtmv(m, 3), .tsk3=gtmv(m, 3), .tsk4=gtmv(m, 3), .tsk5=gtmv(m, 3), .tsk6=gtmv(m, 3), .tsk7=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set( %u %u %u %u %u %u %u %u %u)\n", task_to_rg_mapping, load_balancing_tsk_to_rg_mapping.tsk0, load_balancing_tsk_to_rg_mapping.tsk1, load_balancing_tsk_to_rg_mapping.tsk2, load_balancing_tsk_to_rg_mapping.tsk3, load_balancing_tsk_to_rg_mapping.tsk4, load_balancing_tsk_to_rg_mapping.tsk5, load_balancing_tsk_to_rg_mapping.tsk6, load_balancing_tsk_to_rg_mapping.tsk7);
        if(!err) ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(task_to_rg_mapping, &load_balancing_tsk_to_rg_mapping);
        if(!err) ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get( task_to_rg_mapping, &load_balancing_tsk_to_rg_mapping);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get( %u %u %u %u %u %u %u %u %u)\n", task_to_rg_mapping, load_balancing_tsk_to_rg_mapping.tsk0, load_balancing_tsk_to_rg_mapping.tsk1, load_balancing_tsk_to_rg_mapping.tsk2, load_balancing_tsk_to_rg_mapping.tsk3, load_balancing_tsk_to_rg_mapping.tsk4, load_balancing_tsk_to_rg_mapping.tsk5, load_balancing_tsk_to_rg_mapping.tsk6, load_balancing_tsk_to_rg_mapping.tsk7);
        if(err || load_balancing_tsk_to_rg_mapping.tsk0!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk1!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk2!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk3!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk4!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk5!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk6!=gtmv(m, 3) || load_balancing_tsk_to_rg_mapping.tsk7!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tsk_cnt_rg_0=gtmv(m, 8);
        uint8_t tsk_cnt_rg_1=gtmv(m, 8);
        uint8_t tsk_cnt_rg_2=gtmv(m, 8);
        uint8_t tsk_cnt_rg_3=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set( %u %u %u %u)\n", tsk_cnt_rg_0, tsk_cnt_rg_1, tsk_cnt_rg_2, tsk_cnt_rg_3);
        if(!err) ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(tsk_cnt_rg_0, tsk_cnt_rg_1, tsk_cnt_rg_2, tsk_cnt_rg_3);
        if(!err) ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get( &tsk_cnt_rg_0, &tsk_cnt_rg_1, &tsk_cnt_rg_2, &tsk_cnt_rg_3);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get( %u %u %u %u)\n", tsk_cnt_rg_0, tsk_cnt_rg_1, tsk_cnt_rg_2, tsk_cnt_rg_3);
        if(err || tsk_cnt_rg_0!=gtmv(m, 8) || tsk_cnt_rg_1!=gtmv(m, 8) || tsk_cnt_rg_2!=gtmv(m, 8) || tsk_cnt_rg_3!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tsk_cnt_rg_4=gtmv(m, 8);
        uint8_t tsk_cnt_rg_5=gtmv(m, 8);
        uint8_t tsk_cnt_rg_6=gtmv(m, 8);
        uint8_t tsk_cnt_rg_7=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set( %u %u %u %u)\n", tsk_cnt_rg_4, tsk_cnt_rg_5, tsk_cnt_rg_6, tsk_cnt_rg_7);
        if(!err) ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(tsk_cnt_rg_4, tsk_cnt_rg_5, tsk_cnt_rg_6, tsk_cnt_rg_7);
        if(!err) ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get( &tsk_cnt_rg_4, &tsk_cnt_rg_5, &tsk_cnt_rg_6, &tsk_cnt_rg_7);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get( %u %u %u %u)\n", tsk_cnt_rg_4, tsk_cnt_rg_5, tsk_cnt_rg_6, tsk_cnt_rg_7);
        if(err || tsk_cnt_rg_4!=gtmv(m, 8) || tsk_cnt_rg_5!=gtmv(m, 8) || tsk_cnt_rg_6!=gtmv(m, 8) || tsk_cnt_rg_7!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr dsptcher_reordr_top_intr_ctrl_0_isr = {.fll_return_buf=gtmv(m, 1), .fll_cnt_drp=gtmv(m, 1), .unknwn_msg=gtmv(m, 1), .fll_overflow=gtmv(m, 1), .fll_neg=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set( %u %u %u %u %u)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf, dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp, dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg, dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow, dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(&dsptcher_reordr_top_intr_ctrl_0_isr);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get( &dsptcher_reordr_top_intr_ctrl_0_isr);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get( %u %u %u %u %u)\n", dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf, dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp, dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg, dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow, dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg);
        if(err || dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set( %u)\n", iem);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set(iem);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set( %u)\n", ist);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set(ist);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr dsptcher_reordr_top_intr_ctrl_1_isr = {.qdest0_int=gtmv(m, 1), .qdest1_int=gtmv(m, 1), .qdest2_int=gtmv(m, 1), .qdest3_int=gtmv(m, 1), .qdest4_int=gtmv(m, 1), .qdest5_int=gtmv(m, 1), .qdest6_int=gtmv(m, 1), .qdest7_int=gtmv(m, 1), .qdest8_int=gtmv(m, 1), .qdest9_int=gtmv(m, 1), .qdest10_int=gtmv(m, 1), .qdest11_int=gtmv(m, 1), .qdest12_int=gtmv(m, 1), .qdest13_int=gtmv(m, 1), .qdest14_int=gtmv(m, 1), .qdest15_int=gtmv(m, 1), .qdest16_int=gtmv(m, 1), .qdest17_int=gtmv(m, 1), .qdest18_int=gtmv(m, 1), .qdest19_int=gtmv(m, 1), .qdest20_int=gtmv(m, 1), .qdest21_int=gtmv(m, 1), .qdest22_int=gtmv(m, 1), .qdest23_int=gtmv(m, 1), .qdest24_int=gtmv(m, 1), .qdest25_int=gtmv(m, 1), .qdest26_int=gtmv(m, 1), .qdest27_int=gtmv(m, 1), .qdest28_int=gtmv(m, 1), .qdest29_int=gtmv(m, 1), .qdest30_int=gtmv(m, 1), .qdest31_int=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest0_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest1_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest2_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest3_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest4_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest5_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest6_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest7_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest8_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest9_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest10_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest11_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest12_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest13_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest14_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest15_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest16_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest17_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest18_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest19_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest20_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest21_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest22_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest23_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest24_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest25_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest26_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest27_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest28_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest29_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest30_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest31_int);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set(&dsptcher_reordr_top_intr_ctrl_1_isr);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get( &dsptcher_reordr_top_intr_ctrl_1_isr);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dsptcher_reordr_top_intr_ctrl_1_isr.qdest0_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest1_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest2_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest3_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest4_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest5_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest6_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest7_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest8_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest9_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest10_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest11_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest12_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest13_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest14_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest15_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest16_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest17_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest18_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest19_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest20_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest21_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest22_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest23_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest24_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest25_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest26_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest27_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest28_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest29_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest30_int, dsptcher_reordr_top_intr_ctrl_1_isr.qdest31_int);
        if(err || dsptcher_reordr_top_intr_ctrl_1_isr.qdest0_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest1_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest2_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest3_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest4_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest5_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest6_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest7_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest8_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest9_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest10_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest11_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest12_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest13_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest14_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest15_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest16_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest17_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest18_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest19_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest20_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest21_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest22_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest23_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest24_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest25_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest26_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest27_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest28_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest29_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest30_int!=gtmv(m, 1) || dsptcher_reordr_top_intr_ctrl_1_isr.qdest31_int!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set( %u)\n", iem);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set(iem);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set( %u)\n", ist);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set(ist);
        if(!err) ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en_byp=gtmv(m, 1);
        uint8_t bbid_non_dly=gtmv(m, 8);
        uint8_t bbid_dly=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_bypss_cntrl_set( %u %u %u)\n", en_byp, bbid_non_dly, bbid_dly);
        if(!err) ag_drv_dsptchr_debug_dbg_bypss_cntrl_set(en_byp, bbid_non_dly, bbid_dly);
        if(!err) ag_drv_dsptchr_debug_dbg_bypss_cntrl_get( &en_byp, &bbid_non_dly, &bbid_dly);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_bypss_cntrl_get( %u %u %u)\n", en_byp, bbid_non_dly, bbid_dly);
        if(err || en_byp!=gtmv(m, 1) || bbid_non_dly!=gtmv(m, 8) || bbid_dly!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_debug_glbl_tsk_cnt_0_7 debug_glbl_tsk_cnt_0_7 = {.tsk_cnt_rnr_0=gtmv(m, 4), .tsk_cnt_rnr_1=gtmv(m, 4), .tsk_cnt_rnr_2=gtmv(m, 4), .tsk_cnt_rnr_3=gtmv(m, 4), .tsk_cnt_rnr_4=gtmv(m, 4), .tsk_cnt_rnr_5=gtmv(m, 4), .tsk_cnt_rnr_6=gtmv(m, 4), .tsk_cnt_rnr_7=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set( %u %u %u %u %u %u %u %u)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_0, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_1, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_2, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_3, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_4, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_5, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_6, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_7);
        if(!err) ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set(&debug_glbl_tsk_cnt_0_7);
        if(!err) ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get( &debug_glbl_tsk_cnt_0_7);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get( %u %u %u %u %u %u %u %u)\n", debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_0, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_1, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_2, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_3, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_4, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_5, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_6, debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_7);
        if(err || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_0!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_1!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_2!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_3!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_4!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_5!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_6!=gtmv(m, 4) || debug_glbl_tsk_cnt_0_7.tsk_cnt_rnr_7!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dsptchr_debug_glbl_tsk_cnt_8_15 debug_glbl_tsk_cnt_8_15 = {.tsk_cnt_rnr_8=gtmv(m, 4), .tsk_cnt_rnr_9=gtmv(m, 4), .tsk_cnt_rnr_10=gtmv(m, 4), .tsk_cnt_rnr_11=gtmv(m, 4), .tsk_cnt_rnr_12=gtmv(m, 4), .tsk_cnt_rnr_13=gtmv(m, 4), .tsk_cnt_rnr_14=gtmv(m, 4), .tsk_cnt_rnr_15=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set( %u %u %u %u %u %u %u %u)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_8, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_9, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_10, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_11, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_12, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_13, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_14, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_15);
        if(!err) ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set(&debug_glbl_tsk_cnt_8_15);
        if(!err) ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get( &debug_glbl_tsk_cnt_8_15);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get( %u %u %u %u %u %u %u %u)\n", debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_8, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_9, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_10, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_11, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_12, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_13, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_14, debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_15);
        if(err || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_8!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_9!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_10!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_11!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_12!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_13!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_14!=gtmv(m, 4) || debug_glbl_tsk_cnt_8_15.tsk_cnt_rnr_15!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dbg_sel=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_bus_cntrl_set( %u)\n", dbg_sel);
        if(!err) ag_drv_dsptchr_debug_dbg_bus_cntrl_set(dbg_sel);
        if(!err) ag_drv_dsptchr_debug_dbg_bus_cntrl_get( &dbg_sel);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_bus_cntrl_get( %u)\n", dbg_sel);
        if(err || dbg_sel!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_0_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_0_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_1_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_1_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_2_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_2_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_3_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_3_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_4_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_4_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_5_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_5_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_6_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_6_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_7_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_7_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_8_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_8_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_9_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_9_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_10_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_10_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_11_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_11_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_12_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_12_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_13_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_13_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_14_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_14_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_15_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_15_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_16_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_16_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_17_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_17_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_18_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_18_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_19_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_19_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_20_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_20_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_21_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_21_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_22_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_22_get( %u)\n", dbg_vec_val);
    }
    {
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_dbg_vec_23_get( &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_dbg_vec_23_get( %u)\n", dbg_vec_val);
    }
    {
        uint8_t dbg_mode=gtmv(m, 2);
        bdmf_boolean en_cntrs=gtmv(m, 1);
        bdmf_boolean clr_cntrs=gtmv(m, 1);
        uint8_t dbg_rnr_sel=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set( %u %u %u %u)\n", dbg_mode, en_cntrs, clr_cntrs, dbg_rnr_sel);
        if(!err) ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set(dbg_mode, en_cntrs, clr_cntrs, dbg_rnr_sel);
        if(!err) ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get( &dbg_mode, &en_cntrs, &clr_cntrs, &dbg_rnr_sel);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get( %u %u %u %u)\n", dbg_mode, en_cntrs, clr_cntrs, dbg_rnr_sel);
        if(err || dbg_mode!=gtmv(m, 2) || en_cntrs!=gtmv(m, 1) || clr_cntrs!=gtmv(m, 1) || dbg_rnr_sel!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t index=gtmv(m, 5);
        uint32_t dbg_vec_val=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_debug_statistics_dbg_cnt_get( index, &dbg_vec_val);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_debug_statistics_dbg_cnt_get( %u %u)\n", index, dbg_vec_val);
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint32_t head=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_head_set( %u %u)\n", q_idx, head);
        if(!err) ag_drv_dsptchr_qdes_head_set(q_idx, head);
        if(!err) ag_drv_dsptchr_qdes_head_get( q_idx, &head);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_head_get( %u %u)\n", q_idx, head);
        if(err || head!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        dsptchr_qdes_bfout qdes_bfout = {.qdes_bfout={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_bfout_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_bfout.qdes_bfout[0], qdes_bfout.qdes_bfout[1], qdes_bfout.qdes_bfout[2], qdes_bfout.qdes_bfout[3], qdes_bfout.qdes_bfout[4], qdes_bfout.qdes_bfout[5], qdes_bfout.qdes_bfout[6], qdes_bfout.qdes_bfout[7], qdes_bfout.qdes_bfout[8], qdes_bfout.qdes_bfout[9], qdes_bfout.qdes_bfout[10], qdes_bfout.qdes_bfout[11], qdes_bfout.qdes_bfout[12], qdes_bfout.qdes_bfout[13], qdes_bfout.qdes_bfout[14], qdes_bfout.qdes_bfout[15], qdes_bfout.qdes_bfout[16], qdes_bfout.qdes_bfout[17], qdes_bfout.qdes_bfout[18], qdes_bfout.qdes_bfout[19], qdes_bfout.qdes_bfout[20], qdes_bfout.qdes_bfout[21], qdes_bfout.qdes_bfout[22], qdes_bfout.qdes_bfout[23], qdes_bfout.qdes_bfout[24], qdes_bfout.qdes_bfout[25], qdes_bfout.qdes_bfout[26], qdes_bfout.qdes_bfout[27], qdes_bfout.qdes_bfout[28], qdes_bfout.qdes_bfout[29], qdes_bfout.qdes_bfout[30], qdes_bfout.qdes_bfout[31]);
        if(!err) ag_drv_dsptchr_qdes_bfout_set(zero, &qdes_bfout);
        if(!err) ag_drv_dsptchr_qdes_bfout_get( zero, &qdes_bfout);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_bfout_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_bfout.qdes_bfout[0], qdes_bfout.qdes_bfout[1], qdes_bfout.qdes_bfout[2], qdes_bfout.qdes_bfout[3], qdes_bfout.qdes_bfout[4], qdes_bfout.qdes_bfout[5], qdes_bfout.qdes_bfout[6], qdes_bfout.qdes_bfout[7], qdes_bfout.qdes_bfout[8], qdes_bfout.qdes_bfout[9], qdes_bfout.qdes_bfout[10], qdes_bfout.qdes_bfout[11], qdes_bfout.qdes_bfout[12], qdes_bfout.qdes_bfout[13], qdes_bfout.qdes_bfout[14], qdes_bfout.qdes_bfout[15], qdes_bfout.qdes_bfout[16], qdes_bfout.qdes_bfout[17], qdes_bfout.qdes_bfout[18], qdes_bfout.qdes_bfout[19], qdes_bfout.qdes_bfout[20], qdes_bfout.qdes_bfout[21], qdes_bfout.qdes_bfout[22], qdes_bfout.qdes_bfout[23], qdes_bfout.qdes_bfout[24], qdes_bfout.qdes_bfout[25], qdes_bfout.qdes_bfout[26], qdes_bfout.qdes_bfout[27], qdes_bfout.qdes_bfout[28], qdes_bfout.qdes_bfout[29], qdes_bfout.qdes_bfout[30], qdes_bfout.qdes_bfout[31]);
        if(err || qdes_bfout.qdes_bfout[0]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[1]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[2]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[3]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[4]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[5]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[6]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[7]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[8]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[9]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[10]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[11]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[12]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[13]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[14]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[15]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[16]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[17]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[18]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[19]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[20]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[21]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[22]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[23]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[24]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[25]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[26]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[27]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[28]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[29]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[30]!=gtmv(m, 32) || qdes_bfout.qdes_bfout[31]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        dsptchr_qdes_bufin qdes_bufin = {.qdes_bfin={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_bufin_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_bufin.qdes_bfin[0], qdes_bufin.qdes_bfin[1], qdes_bufin.qdes_bfin[2], qdes_bufin.qdes_bfin[3], qdes_bufin.qdes_bfin[4], qdes_bufin.qdes_bfin[5], qdes_bufin.qdes_bfin[6], qdes_bufin.qdes_bfin[7], qdes_bufin.qdes_bfin[8], qdes_bufin.qdes_bfin[9], qdes_bufin.qdes_bfin[10], qdes_bufin.qdes_bfin[11], qdes_bufin.qdes_bfin[12], qdes_bufin.qdes_bfin[13], qdes_bufin.qdes_bfin[14], qdes_bufin.qdes_bfin[15], qdes_bufin.qdes_bfin[16], qdes_bufin.qdes_bfin[17], qdes_bufin.qdes_bfin[18], qdes_bufin.qdes_bfin[19], qdes_bufin.qdes_bfin[20], qdes_bufin.qdes_bfin[21], qdes_bufin.qdes_bfin[22], qdes_bufin.qdes_bfin[23], qdes_bufin.qdes_bfin[24], qdes_bufin.qdes_bfin[25], qdes_bufin.qdes_bfin[26], qdes_bufin.qdes_bfin[27], qdes_bufin.qdes_bfin[28], qdes_bufin.qdes_bfin[29], qdes_bufin.qdes_bfin[30], qdes_bufin.qdes_bfin[31]);
        if(!err) ag_drv_dsptchr_qdes_bufin_set(zero, &qdes_bufin);
        if(!err) ag_drv_dsptchr_qdes_bufin_get( zero, &qdes_bufin);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_bufin_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_bufin.qdes_bfin[0], qdes_bufin.qdes_bfin[1], qdes_bufin.qdes_bfin[2], qdes_bufin.qdes_bfin[3], qdes_bufin.qdes_bfin[4], qdes_bufin.qdes_bfin[5], qdes_bufin.qdes_bfin[6], qdes_bufin.qdes_bfin[7], qdes_bufin.qdes_bfin[8], qdes_bufin.qdes_bfin[9], qdes_bufin.qdes_bfin[10], qdes_bufin.qdes_bfin[11], qdes_bufin.qdes_bfin[12], qdes_bufin.qdes_bfin[13], qdes_bufin.qdes_bfin[14], qdes_bufin.qdes_bfin[15], qdes_bufin.qdes_bfin[16], qdes_bufin.qdes_bfin[17], qdes_bufin.qdes_bfin[18], qdes_bufin.qdes_bfin[19], qdes_bufin.qdes_bfin[20], qdes_bufin.qdes_bfin[21], qdes_bufin.qdes_bfin[22], qdes_bufin.qdes_bfin[23], qdes_bufin.qdes_bfin[24], qdes_bufin.qdes_bfin[25], qdes_bufin.qdes_bfin[26], qdes_bufin.qdes_bfin[27], qdes_bufin.qdes_bfin[28], qdes_bufin.qdes_bfin[29], qdes_bufin.qdes_bfin[30], qdes_bufin.qdes_bfin[31]);
        if(err || qdes_bufin.qdes_bfin[0]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[1]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[2]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[3]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[4]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[5]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[6]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[7]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[8]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[9]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[10]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[11]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[12]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[13]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[14]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[15]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[16]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[17]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[18]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[19]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[20]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[21]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[22]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[23]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[24]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[25]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[26]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[27]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[28]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[29]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[30]!=gtmv(m, 32) || qdes_bufin.qdes_bfin[31]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_idx=gtmv(m, 5);
        uint32_t tail=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_tail_set( %u %u)\n", q_idx, tail);
        if(!err) ag_drv_dsptchr_qdes_tail_set(q_idx, tail);
        if(!err) ag_drv_dsptchr_qdes_tail_get( q_idx, &tail);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_tail_get( %u %u)\n", q_idx, tail);
        if(err || tail!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        dsptchr_qdes_fbdnull qdes_fbdnull = {.qdes_fbdnull={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_fbdnull_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_fbdnull.qdes_fbdnull[0], qdes_fbdnull.qdes_fbdnull[1], qdes_fbdnull.qdes_fbdnull[2], qdes_fbdnull.qdes_fbdnull[3], qdes_fbdnull.qdes_fbdnull[4], qdes_fbdnull.qdes_fbdnull[5], qdes_fbdnull.qdes_fbdnull[6], qdes_fbdnull.qdes_fbdnull[7], qdes_fbdnull.qdes_fbdnull[8], qdes_fbdnull.qdes_fbdnull[9], qdes_fbdnull.qdes_fbdnull[10], qdes_fbdnull.qdes_fbdnull[11], qdes_fbdnull.qdes_fbdnull[12], qdes_fbdnull.qdes_fbdnull[13], qdes_fbdnull.qdes_fbdnull[14], qdes_fbdnull.qdes_fbdnull[15], qdes_fbdnull.qdes_fbdnull[16], qdes_fbdnull.qdes_fbdnull[17], qdes_fbdnull.qdes_fbdnull[18], qdes_fbdnull.qdes_fbdnull[19], qdes_fbdnull.qdes_fbdnull[20], qdes_fbdnull.qdes_fbdnull[21], qdes_fbdnull.qdes_fbdnull[22], qdes_fbdnull.qdes_fbdnull[23], qdes_fbdnull.qdes_fbdnull[24], qdes_fbdnull.qdes_fbdnull[25], qdes_fbdnull.qdes_fbdnull[26], qdes_fbdnull.qdes_fbdnull[27], qdes_fbdnull.qdes_fbdnull[28], qdes_fbdnull.qdes_fbdnull[29], qdes_fbdnull.qdes_fbdnull[30], qdes_fbdnull.qdes_fbdnull[31]);
        if(!err) ag_drv_dsptchr_qdes_fbdnull_set(zero, &qdes_fbdnull);
        if(!err) ag_drv_dsptchr_qdes_fbdnull_get( zero, &qdes_fbdnull);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_fbdnull_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_fbdnull.qdes_fbdnull[0], qdes_fbdnull.qdes_fbdnull[1], qdes_fbdnull.qdes_fbdnull[2], qdes_fbdnull.qdes_fbdnull[3], qdes_fbdnull.qdes_fbdnull[4], qdes_fbdnull.qdes_fbdnull[5], qdes_fbdnull.qdes_fbdnull[6], qdes_fbdnull.qdes_fbdnull[7], qdes_fbdnull.qdes_fbdnull[8], qdes_fbdnull.qdes_fbdnull[9], qdes_fbdnull.qdes_fbdnull[10], qdes_fbdnull.qdes_fbdnull[11], qdes_fbdnull.qdes_fbdnull[12], qdes_fbdnull.qdes_fbdnull[13], qdes_fbdnull.qdes_fbdnull[14], qdes_fbdnull.qdes_fbdnull[15], qdes_fbdnull.qdes_fbdnull[16], qdes_fbdnull.qdes_fbdnull[17], qdes_fbdnull.qdes_fbdnull[18], qdes_fbdnull.qdes_fbdnull[19], qdes_fbdnull.qdes_fbdnull[20], qdes_fbdnull.qdes_fbdnull[21], qdes_fbdnull.qdes_fbdnull[22], qdes_fbdnull.qdes_fbdnull[23], qdes_fbdnull.qdes_fbdnull[24], qdes_fbdnull.qdes_fbdnull[25], qdes_fbdnull.qdes_fbdnull[26], qdes_fbdnull.qdes_fbdnull[27], qdes_fbdnull.qdes_fbdnull[28], qdes_fbdnull.qdes_fbdnull[29], qdes_fbdnull.qdes_fbdnull[30], qdes_fbdnull.qdes_fbdnull[31]);
        if(err || qdes_fbdnull.qdes_fbdnull[0]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[1]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[2]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[3]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[4]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[5]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[6]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[7]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[8]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[9]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[10]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[11]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[12]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[13]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[14]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[15]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[16]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[17]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[18]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[19]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[20]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[21]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[22]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[23]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[24]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[25]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[26]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[27]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[28]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[29]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[30]!=gtmv(m, 32) || qdes_fbdnull.qdes_fbdnull[31]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        dsptchr_qdes_nullbd qdes_nullbd = {.qdes_nullbd={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_nullbd_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_nullbd.qdes_nullbd[0], qdes_nullbd.qdes_nullbd[1], qdes_nullbd.qdes_nullbd[2], qdes_nullbd.qdes_nullbd[3], qdes_nullbd.qdes_nullbd[4], qdes_nullbd.qdes_nullbd[5], qdes_nullbd.qdes_nullbd[6], qdes_nullbd.qdes_nullbd[7], qdes_nullbd.qdes_nullbd[8], qdes_nullbd.qdes_nullbd[9], qdes_nullbd.qdes_nullbd[10], qdes_nullbd.qdes_nullbd[11], qdes_nullbd.qdes_nullbd[12], qdes_nullbd.qdes_nullbd[13], qdes_nullbd.qdes_nullbd[14], qdes_nullbd.qdes_nullbd[15], qdes_nullbd.qdes_nullbd[16], qdes_nullbd.qdes_nullbd[17], qdes_nullbd.qdes_nullbd[18], qdes_nullbd.qdes_nullbd[19], qdes_nullbd.qdes_nullbd[20], qdes_nullbd.qdes_nullbd[21], qdes_nullbd.qdes_nullbd[22], qdes_nullbd.qdes_nullbd[23], qdes_nullbd.qdes_nullbd[24], qdes_nullbd.qdes_nullbd[25], qdes_nullbd.qdes_nullbd[26], qdes_nullbd.qdes_nullbd[27], qdes_nullbd.qdes_nullbd[28], qdes_nullbd.qdes_nullbd[29], qdes_nullbd.qdes_nullbd[30], qdes_nullbd.qdes_nullbd[31]);
        if(!err) ag_drv_dsptchr_qdes_nullbd_set(zero, &qdes_nullbd);
        if(!err) ag_drv_dsptchr_qdes_nullbd_get( zero, &qdes_nullbd);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_nullbd_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_nullbd.qdes_nullbd[0], qdes_nullbd.qdes_nullbd[1], qdes_nullbd.qdes_nullbd[2], qdes_nullbd.qdes_nullbd[3], qdes_nullbd.qdes_nullbd[4], qdes_nullbd.qdes_nullbd[5], qdes_nullbd.qdes_nullbd[6], qdes_nullbd.qdes_nullbd[7], qdes_nullbd.qdes_nullbd[8], qdes_nullbd.qdes_nullbd[9], qdes_nullbd.qdes_nullbd[10], qdes_nullbd.qdes_nullbd[11], qdes_nullbd.qdes_nullbd[12], qdes_nullbd.qdes_nullbd[13], qdes_nullbd.qdes_nullbd[14], qdes_nullbd.qdes_nullbd[15], qdes_nullbd.qdes_nullbd[16], qdes_nullbd.qdes_nullbd[17], qdes_nullbd.qdes_nullbd[18], qdes_nullbd.qdes_nullbd[19], qdes_nullbd.qdes_nullbd[20], qdes_nullbd.qdes_nullbd[21], qdes_nullbd.qdes_nullbd[22], qdes_nullbd.qdes_nullbd[23], qdes_nullbd.qdes_nullbd[24], qdes_nullbd.qdes_nullbd[25], qdes_nullbd.qdes_nullbd[26], qdes_nullbd.qdes_nullbd[27], qdes_nullbd.qdes_nullbd[28], qdes_nullbd.qdes_nullbd[29], qdes_nullbd.qdes_nullbd[30], qdes_nullbd.qdes_nullbd[31]);
        if(err || qdes_nullbd.qdes_nullbd[0]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[1]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[2]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[3]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[4]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[5]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[6]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[7]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[8]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[9]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[10]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[11]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[12]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[13]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[14]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[15]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[16]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[17]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[18]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[19]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[20]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[21]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[22]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[23]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[24]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[25]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[26]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[27]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[28]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[29]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[30]!=gtmv(m, 32) || qdes_nullbd.qdes_nullbd[31]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        dsptchr_qdes_bufavail qdes_bufavail = {.qdes_bufavail={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_dsptchr_qdes_bufavail_get( zero, &qdes_bufavail);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_bufavail_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, qdes_bufavail.qdes_bufavail[0], qdes_bufavail.qdes_bufavail[1], qdes_bufavail.qdes_bufavail[2], qdes_bufavail.qdes_bufavail[3], qdes_bufavail.qdes_bufavail[4], qdes_bufavail.qdes_bufavail[5], qdes_bufavail.qdes_bufavail[6], qdes_bufavail.qdes_bufavail[7], qdes_bufavail.qdes_bufavail[8], qdes_bufavail.qdes_bufavail[9], qdes_bufavail.qdes_bufavail[10], qdes_bufavail.qdes_bufavail[11], qdes_bufavail.qdes_bufavail[12], qdes_bufavail.qdes_bufavail[13], qdes_bufavail.qdes_bufavail[14], qdes_bufavail.qdes_bufavail[15], qdes_bufavail.qdes_bufavail[16], qdes_bufavail.qdes_bufavail[17], qdes_bufavail.qdes_bufavail[18], qdes_bufavail.qdes_bufavail[19], qdes_bufavail.qdes_bufavail[20], qdes_bufavail.qdes_bufavail[21], qdes_bufavail.qdes_bufavail[22], qdes_bufavail.qdes_bufavail[23], qdes_bufavail.qdes_bufavail[24], qdes_bufavail.qdes_bufavail[25], qdes_bufavail.qdes_bufavail[26], qdes_bufavail.qdes_bufavail[27], qdes_bufavail.qdes_bufavail[28], qdes_bufavail.qdes_bufavail[29], qdes_bufavail.qdes_bufavail[30], qdes_bufavail.qdes_bufavail[31]);
    }
    {
        uint8_t q_head_idx=gtmv(m, 5);
        uint16_t head=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_q_head_set( %u %u)\n", q_head_idx, head);
        if(!err) ag_drv_dsptchr_qdes_reg_q_head_set(q_head_idx, head);
        if(!err) ag_drv_dsptchr_qdes_reg_q_head_get( q_head_idx, &head);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_q_head_get( %u %u)\n", q_head_idx, head);
        if(err || head!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t viq_head_vld=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_viq_head_vld_set( %u)\n", viq_head_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_viq_head_vld_set(viq_head_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_viq_head_vld_get( &viq_head_vld);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_viq_head_vld_get( %u)\n", viq_head_vld);
        if(err || viq_head_vld!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t chrncy_vld=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set( %u)\n", chrncy_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set(chrncy_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get( &chrncy_vld);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get( %u)\n", chrncy_vld);
        if(err || chrncy_vld!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t viq_head_vld=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_veq_head_vld_set( %u)\n", viq_head_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_veq_head_vld_set(viq_head_vld);
        if(!err) ag_drv_dsptchr_qdes_reg_veq_head_vld_get( &viq_head_vld);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_veq_head_vld_get( %u)\n", viq_head_vld);
        if(err || viq_head_vld!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean use_buf_avl=gtmv(m, 1);
        bdmf_boolean dec_bufout_when_mltcst=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set( %u %u)\n", use_buf_avl, dec_bufout_when_mltcst);
        if(!err) ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set(use_buf_avl, dec_bufout_when_mltcst);
        if(!err) ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get( &use_buf_avl, &dec_bufout_when_mltcst);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get( %u %u)\n", use_buf_avl, dec_bufout_when_mltcst);
        if(err || use_buf_avl!=gtmv(m, 1) || dec_bufout_when_mltcst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t drpcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_flldes_flldrop_set( %u)\n", drpcnt);
        if(!err) ag_drv_dsptchr_flldes_flldrop_set(drpcnt);
        if(!err) ag_drv_dsptchr_flldes_flldrop_get( &drpcnt);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_flldes_flldrop_get( %u)\n", drpcnt);
        if(err || drpcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t bufavail=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_flldes_bufavail_get( &bufavail);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_flldes_bufavail_get( %u)\n", bufavail);
    }
    {
        uint32_t freemin=gtmv(m, 32);
        if(!err) ag_drv_dsptchr_flldes_freemin_get( &freemin);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_flldes_freemin_get( %u)\n", freemin);
    }
    {
        uint16_t temp_index=gtmv(m, 10);
        uint16_t data=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_bdram_next_data_set( %u %u)\n", temp_index, data);
        if(!err) ag_drv_dsptchr_bdram_next_data_set(temp_index, data);
        if(!err) ag_drv_dsptchr_bdram_next_data_get( temp_index, &data);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_bdram_next_data_get( %u %u)\n", temp_index, data);
        if(err || data!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t temp_index=gtmv(m, 10);
        uint16_t data=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_bdram_prev_data_set( %u %u)\n", temp_index, data);
        if(!err) ag_drv_dsptchr_bdram_prev_data_set(temp_index, data);
        if(!err) ag_drv_dsptchr_bdram_prev_data_get( temp_index, &data);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_bdram_prev_data_get( %u %u)\n", temp_index, data);
        if(err || data!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t temp_index=gtmv(m, 11);
        dsptchr_pdram_data pdram_data = {.reorder_ram_data={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_pdram_data_set( %u %u %u)\n", temp_index, pdram_data.reorder_ram_data[0], pdram_data.reorder_ram_data[1]);
        if(!err) ag_drv_dsptchr_pdram_data_set(temp_index, &pdram_data);
        if(!err) ag_drv_dsptchr_pdram_data_get( temp_index, &pdram_data);
        if(!err) bdmf_session_print(session, "ag_drv_dsptchr_pdram_data_get( %u %u %u)\n", temp_index, pdram_data.reorder_ram_data[0], pdram_data.reorder_ram_data[1]);
        if(err || pdram_data.reorder_ram_data[0]!=gtmv(m, 32) || pdram_data.reorder_ram_data[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_dsptchr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_reorder_cfg_dsptchr_reordr_cfg : reg = &RU_REG(DSPTCHR, REORDER_CFG_DSPTCHR_REORDR_CFG); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_reorder_cfg_vq_en : reg = &RU_REG(DSPTCHR, REORDER_CFG_VQ_EN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_reorder_cfg_bb_cfg : reg = &RU_REG(DSPTCHR, REORDER_CFG_BB_CFG); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_reorder_cfg_clk_gate_cntrl : reg = &RU_REG(DSPTCHR, REORDER_CFG_CLK_GATE_CNTRL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_ingrs_congstn : reg = &RU_REG(DSPTCHR, CONGESTION_INGRS_CONGSTN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_egrs_congstn : reg = &RU_REG(DSPTCHR, CONGESTION_EGRS_CONGSTN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_total_egrs_congstn : reg = &RU_REG(DSPTCHR, CONGESTION_TOTAL_EGRS_CONGSTN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_glbl_congstn : reg = &RU_REG(DSPTCHR, CONGESTION_GLBL_CONGSTN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_congstn_status : reg = &RU_REG(DSPTCHR, CONGESTION_CONGSTN_STATUS); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_per_q_ingrs_congstn_low : reg = &RU_REG(DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_LOW); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_per_q_ingrs_congstn_high : reg = &RU_REG(DSPTCHR, CONGESTION_PER_Q_INGRS_CONGSTN_HIGH); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_per_q_egrs_congstn_low : reg = &RU_REG(DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_LOW); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_congestion_per_q_egrs_congstn_high : reg = &RU_REG(DSPTCHR, CONGESTION_PER_Q_EGRS_CONGSTN_HIGH); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_ingrs_queues_q_ingrs_size : reg = &RU_REG(DSPTCHR, INGRS_QUEUES_Q_INGRS_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_ingrs_queues_q_ingrs_limits : reg = &RU_REG(DSPTCHR, INGRS_QUEUES_Q_INGRS_LIMITS); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_queue_mapping_crdt_cfg : reg = &RU_REG(DSPTCHR, QUEUE_MAPPING_CRDT_CFG); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_queue_mapping_pd_dsptch_add : reg = &RU_REG(DSPTCHR, QUEUE_MAPPING_PD_DSPTCH_ADD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_queue_mapping_q_dest : reg = &RU_REG(DSPTCHR, QUEUE_MAPPING_Q_DEST); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_cmn_pool_lmt : reg = &RU_REG(DSPTCHR, POOL_SIZES_CMN_POOL_LMT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_cmn_pool_size : reg = &RU_REG(DSPTCHR, POOL_SIZES_CMN_POOL_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_grnted_pool_lmt : reg = &RU_REG(DSPTCHR, POOL_SIZES_GRNTED_POOL_LMT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_grnted_pool_size : reg = &RU_REG(DSPTCHR, POOL_SIZES_GRNTED_POOL_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_multi_cst_pool_lmt : reg = &RU_REG(DSPTCHR, POOL_SIZES_MULTI_CST_POOL_LMT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_multi_cst_pool_size : reg = &RU_REG(DSPTCHR, POOL_SIZES_MULTI_CST_POOL_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_rnr_pool_lmt : reg = &RU_REG(DSPTCHR, POOL_SIZES_RNR_POOL_LMT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_rnr_pool_size : reg = &RU_REG(DSPTCHR, POOL_SIZES_RNR_POOL_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pool_sizes_prcssing_pool_size : reg = &RU_REG(DSPTCHR, POOL_SIZES_PRCSSING_POOL_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_mask_msk_tsk_255_0 : reg = &RU_REG(DSPTCHR, MASK_MSK_TSK_255_0); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_mask_msk_q : reg = &RU_REG(DSPTCHR, MASK_MSK_Q); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_mask_dly_q : reg = &RU_REG(DSPTCHR, MASK_DLY_Q); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_mask_non_dly_q : reg = &RU_REG(DSPTCHR, MASK_NON_DLY_Q); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_egrs_queues_egrs_dly_qm_crdt : reg = &RU_REG(DSPTCHR, EGRS_QUEUES_EGRS_DLY_QM_CRDT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_egrs_queues_egrs_non_dly_qm_crdt : reg = &RU_REG(DSPTCHR, EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_egrs_queues_total_q_egrs_size : reg = &RU_REG(DSPTCHR, EGRS_QUEUES_TOTAL_Q_EGRS_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_egrs_queues_per_q_egrs_size : reg = &RU_REG(DSPTCHR, EGRS_QUEUES_PER_Q_EGRS_SIZE); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_wakeup_control_wkup_req : reg = &RU_REG(DSPTCHR, WAKEUP_CONTROL_WKUP_REQ); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_wakeup_control_wkup_thrshld : reg = &RU_REG(DSPTCHR, WAKEUP_CONTROL_WKUP_THRSHLD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_disptch_scheduling_dwrr_info : reg = &RU_REG(DSPTCHR, DISPTCH_SCHEDULING_DWRR_INFO); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_disptch_scheduling_vld_crdt : reg = &RU_REG(DSPTCHR, DISPTCH_SCHEDULING_VLD_CRDT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_lb_cfg : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_LB_CFG); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_0_1 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_2_3 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_2_3); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_4_5 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_4_5); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_6_7 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_6_7); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_8_9 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_8_9); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_10_11 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_10_11); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_12_13 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_12_13); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_free_task_14_15 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_FREE_TASK_14_15); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_tsk_to_rg_mapping : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_TSK_TO_RG_MAPPING); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_rg_avlabl_tsk_0_3 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_0_3); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_load_balancing_rg_avlabl_tsk_4_7 : reg = &RU_REG(DSPTCHR, LOAD_BALANCING_RG_AVLABL_TSK_4_7); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_0_isr : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ism : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ier : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_0_itr : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_1_isr : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ism : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ier : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_dsptcher_reordr_top_intr_ctrl_1_itr : reg = &RU_REG(DSPTCHR, DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_bypss_cntrl : reg = &RU_REG(DSPTCHR, DEBUG_DBG_BYPSS_CNTRL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_glbl_tsk_cnt_0_7 : reg = &RU_REG(DSPTCHR, DEBUG_GLBL_TSK_CNT_0_7); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_glbl_tsk_cnt_8_15 : reg = &RU_REG(DSPTCHR, DEBUG_GLBL_TSK_CNT_8_15); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_bus_cntrl : reg = &RU_REG(DSPTCHR, DEBUG_DBG_BUS_CNTRL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_0 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_0); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_1 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_1); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_2 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_2); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_3 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_3); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_4 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_4); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_5 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_5); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_6 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_6); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_7 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_7); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_8 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_8); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_9 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_9); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_10 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_10); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_11 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_11); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_12 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_12); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_13 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_13); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_14 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_14); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_15 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_15); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_16 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_16); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_17 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_17); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_18 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_18); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_19 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_19); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_20 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_20); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_21 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_21); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_22 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_22); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_dbg_vec_23 : reg = &RU_REG(DSPTCHR, DEBUG_DBG_VEC_23); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_statistics_dbg_sttstcs_ctrl : reg = &RU_REG(DSPTCHR, DEBUG_STATISTICS_DBG_STTSTCS_CTRL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_debug_statistics_dbg_cnt : reg = &RU_REG(DSPTCHR, DEBUG_STATISTICS_DBG_CNT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_head : reg = &RU_REG(DSPTCHR, QDES_HEAD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_bfout : reg = &RU_REG(DSPTCHR, QDES_BFOUT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_bufin : reg = &RU_REG(DSPTCHR, QDES_BUFIN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_tail : reg = &RU_REG(DSPTCHR, QDES_TAIL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_fbdnull : reg = &RU_REG(DSPTCHR, QDES_FBDNULL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_nullbd : reg = &RU_REG(DSPTCHR, QDES_NULLBD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_bufavail : reg = &RU_REG(DSPTCHR, QDES_BUFAVAIL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_reg_q_head : reg = &RU_REG(DSPTCHR, QDES_REG_Q_HEAD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_reg_viq_head_vld : reg = &RU_REG(DSPTCHR, QDES_REG_VIQ_HEAD_VLD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_reg_viq_chrncy_vld : reg = &RU_REG(DSPTCHR, QDES_REG_VIQ_CHRNCY_VLD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_reg_veq_head_vld : reg = &RU_REG(DSPTCHR, QDES_REG_VEQ_HEAD_VLD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_qdes_reg_qdes_buf_avl_cntrl : reg = &RU_REG(DSPTCHR, QDES_REG_QDES_BUF_AVL_CNTRL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_head : reg = &RU_REG(DSPTCHR, FLLDES_HEAD); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_bfout : reg = &RU_REG(DSPTCHR, FLLDES_BFOUT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_bfin : reg = &RU_REG(DSPTCHR, FLLDES_BFIN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_tail : reg = &RU_REG(DSPTCHR, FLLDES_TAIL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_flldrop : reg = &RU_REG(DSPTCHR, FLLDES_FLLDROP); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_ltint : reg = &RU_REG(DSPTCHR, FLLDES_LTINT); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_bufavail : reg = &RU_REG(DSPTCHR, FLLDES_BUFAVAIL); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_flldes_freemin : reg = &RU_REG(DSPTCHR, FLLDES_FREEMIN); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_bdram_next_data : reg = &RU_REG(DSPTCHR, BDRAM_NEXT_DATA); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_bdram_prev_data : reg = &RU_REG(DSPTCHR, BDRAM_PREV_DATA); blk = &RU_BLK(DSPTCHR); break;
    case bdmf_address_pdram_data : reg = &RU_REG(DSPTCHR, PDRAM_DATA); blk = &RU_BLK(DSPTCHR); break;
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

bdmfmon_handle_t ag_drv_dsptchr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "dsptchr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "dsptchr", "dsptchr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_cngs_params[]={
            BDMFMON_MAKE_PARM("viq_idx", "viq_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("frst_lvl", "frst_lvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("scnd_lvl", "scnd_lvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hyst_thrs", "hyst_thrs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_glbl_cngs_params[]={
            BDMFMON_MAKE_PARM("frst_lvl", "frst_lvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("scnd_lvl", "scnd_lvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hyst_thrs", "hyst_thrs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_size_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmn_cnt", "cmn_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_credit_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("credit_cnt", "credit_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_limits_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmn_max", "cmn_max", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gurntd_max", "gurntd_max", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ingress_coherency_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("chrncy_en", "chrncy_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("chrncy_cnt", "chrncy_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pools_limits[]={
            BDMFMON_MAKE_PARM("cmn_pool_lmt", "cmn_pool_lmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("grnted_pool_lmt", "grnted_pool_lmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mcast_pool_lmt", "mcast_pool_lmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_pool_lmt", "rnr_pool_lmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmn_pool_size", "cmn_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("grnted_pool_size", "grnted_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mcast_pool_size", "mcast_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_pool_size", "rnr_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("processing_pool_size", "processing_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fll_entry[]={
            BDMFMON_MAKE_PARM("head", "head", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tail", "tail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("minbuf", "minbuf", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bfin", "bfin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_dsptch_addr[]={
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("base_add", "base_add", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("offset_add", "offset_add", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reorder_cfg_dsptchr_reordr_cfg[]={
            BDMFMON_MAKE_PARM("disp_enable", "disp_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rdy", "rdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reordr_par_mod", "reordr_par_mod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("per_q_egrs_congst_en", "per_q_egrs_congst_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dsptch_sm_enh_mod", "dsptch_sm_enh_mod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ingrs_pipe_dly_en", "ingrs_pipe_dly_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ingrs_pipe_dly_cnt", "ingrs_pipe_dly_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("egrs_drop_only", "egrs_drop_only", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reorder_cfg_vq_en[]={
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reorder_cfg_bb_cfg[]={
            BDMFMON_MAKE_PARM("src_id", "src_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dst_id_ovride", "dst_id_ovride", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("route_ovride", "route_ovride", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ovride_en", "ovride_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reorder_cfg_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_mapping_crdt_cfg[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bb_id", "bb_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trgt_add", "trgt_add", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_mapping_q_dest[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("is_dest_disp", "is_dest_disp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_msk_tsk_255_0[]={
            BDMFMON_MAKE_PARM("group_idx", "group_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task_mask0", "task_mask0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask1", "task_mask1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask2", "task_mask2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask3", "task_mask3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask4", "task_mask4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask5", "task_mask5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask6", "task_mask6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("task_mask7", "task_mask7", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_msk_q[]={
            BDMFMON_MAKE_PARM("group_idx", "group_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_dly_q[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("set_delay", "set_delay", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_non_dly_q[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("set_non_delay", "set_non_delay", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_egrs_queues_egrs_dly_qm_crdt[]={
            BDMFMON_MAKE_PARM("dly_crdt", "dly_crdt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_egrs_queues_egrs_non_dly_qm_crdt[]={
            BDMFMON_MAKE_PARM("non_dly_crdt", "non_dly_crdt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_egrs_queues_total_q_egrs_size[]={
            BDMFMON_MAKE_PARM("total_egrs_size", "total_egrs_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wakeup_control_wkup_req[]={
            BDMFMON_MAKE_PARM("q0", "q0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q1", "q1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q2", "q2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q3", "q3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q4", "q4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q5", "q5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q6", "q6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q7", "q7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q8", "q8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q9", "q9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q10", "q10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q11", "q11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q12", "q12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q13", "q13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q14", "q14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q15", "q15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q16", "q16", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q17", "q17", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q18", "q18", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q19", "q19", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q20", "q20", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q21", "q21", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q22", "q22", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q23", "q23", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q24", "q24", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q25", "q25", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q26", "q26", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q27", "q27", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q28", "q28", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q29", "q29", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q30", "q30", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q31", "q31", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wakeup_control_wkup_thrshld[]={
            BDMFMON_MAKE_PARM("wkup_thrshld", "wkup_thrshld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disptch_scheduling_dwrr_info[]={
            BDMFMON_MAKE_PARM("dwrr_q_idx", "dwrr_q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q_crdt", "q_crdt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngtv", "ngtv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("quntum", "quntum", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disptch_scheduling_vld_crdt[]={
            BDMFMON_MAKE_PARM("q0", "q0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q1", "q1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q2", "q2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q3", "q3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q4", "q4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q5", "q5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q6", "q6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q7", "q7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q8", "q8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q9", "q9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q10", "q10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q11", "q11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q12", "q12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q13", "q13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q14", "q14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q15", "q15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q16", "q16", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q17", "q17", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q18", "q18", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q19", "q19", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q20", "q20", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q21", "q21", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q22", "q22", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q23", "q23", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q24", "q24", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q25", "q25", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q26", "q26", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q27", "q27", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q28", "q28", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q29", "q29", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q30", "q30", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q31", "q31", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_lb_cfg[]={
            BDMFMON_MAKE_PARM("lb_mode", "lb_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sp_thrshld", "sp_thrshld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_0_1[]={
            BDMFMON_MAKE_PARM("rnr0", "rnr0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr1", "rnr1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_2_3[]={
            BDMFMON_MAKE_PARM("rnr2", "rnr2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr3", "rnr3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_4_5[]={
            BDMFMON_MAKE_PARM("rnr4", "rnr4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr5", "rnr5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_6_7[]={
            BDMFMON_MAKE_PARM("rnr6", "rnr6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr7", "rnr7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_8_9[]={
            BDMFMON_MAKE_PARM("rnr8", "rnr8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr9", "rnr9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_10_11[]={
            BDMFMON_MAKE_PARM("rnr10", "rnr10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr11", "rnr11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_12_13[]={
            BDMFMON_MAKE_PARM("rnr12", "rnr12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr13", "rnr13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_free_task_14_15[]={
            BDMFMON_MAKE_PARM("rnr14", "rnr14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr15", "rnr15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_tsk_to_rg_mapping[]={
            BDMFMON_MAKE_PARM("task_to_rg_mapping", "task_to_rg_mapping", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk0", "tsk0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk1", "tsk1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk2", "tsk2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk3", "tsk3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk4", "tsk4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk5", "tsk5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk6", "tsk6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk7", "tsk7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_rg_avlabl_tsk_0_3[]={
            BDMFMON_MAKE_PARM("tsk_cnt_rg_0", "tsk_cnt_rg_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_1", "tsk_cnt_rg_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_2", "tsk_cnt_rg_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_3", "tsk_cnt_rg_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_rg_avlabl_tsk_4_7[]={
            BDMFMON_MAKE_PARM("tsk_cnt_rg_4", "tsk_cnt_rg_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_5", "tsk_cnt_rg_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_6", "tsk_cnt_rg_6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rg_7", "tsk_cnt_rg_7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_0_isr[]={
            BDMFMON_MAKE_PARM("fll_return_buf", "fll_return_buf", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fll_cnt_drp", "fll_cnt_drp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unknwn_msg", "unknwn_msg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fll_overflow", "fll_overflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fll_neg", "fll_neg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_0_ier[]={
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_0_itr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_1_isr[]={
            BDMFMON_MAKE_PARM("qdest0_int", "qdest0_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest1_int", "qdest1_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest2_int", "qdest2_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest3_int", "qdest3_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest4_int", "qdest4_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest5_int", "qdest5_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest6_int", "qdest6_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest7_int", "qdest7_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest8_int", "qdest8_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest9_int", "qdest9_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest10_int", "qdest10_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest11_int", "qdest11_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest12_int", "qdest12_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest13_int", "qdest13_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest14_int", "qdest14_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest15_int", "qdest15_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest16_int", "qdest16_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest17_int", "qdest17_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest18_int", "qdest18_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest19_int", "qdest19_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest20_int", "qdest20_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest21_int", "qdest21_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest22_int", "qdest22_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest23_int", "qdest23_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest24_int", "qdest24_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest25_int", "qdest25_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest26_int", "qdest26_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest27_int", "qdest27_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest28_int", "qdest28_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest29_int", "qdest29_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest30_int", "qdest30_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdest31_int", "qdest31_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_1_ier[]={
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dsptcher_reordr_top_intr_ctrl_1_itr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_dbg_bypss_cntrl[]={
            BDMFMON_MAKE_PARM("en_byp", "en_byp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bbid_non_dly", "bbid_non_dly", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bbid_dly", "bbid_dly", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_glbl_tsk_cnt_0_7[]={
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_0", "tsk_cnt_rnr_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_1", "tsk_cnt_rnr_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_2", "tsk_cnt_rnr_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_3", "tsk_cnt_rnr_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_4", "tsk_cnt_rnr_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_5", "tsk_cnt_rnr_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_6", "tsk_cnt_rnr_6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_7", "tsk_cnt_rnr_7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_glbl_tsk_cnt_8_15[]={
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_8", "tsk_cnt_rnr_8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_9", "tsk_cnt_rnr_9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_10", "tsk_cnt_rnr_10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_11", "tsk_cnt_rnr_11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_12", "tsk_cnt_rnr_12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_13", "tsk_cnt_rnr_13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_14", "tsk_cnt_rnr_14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsk_cnt_rnr_15", "tsk_cnt_rnr_15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_dbg_bus_cntrl[]={
            BDMFMON_MAKE_PARM("dbg_sel", "dbg_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_statistics_dbg_sttstcs_ctrl[]={
            BDMFMON_MAKE_PARM("dbg_mode", "dbg_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_cntrs", "en_cntrs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clr_cntrs", "clr_cntrs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dbg_rnr_sel", "dbg_rnr_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_head[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("head", "head", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_bfout[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdes_bfout0", "qdes_bfout0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout1", "qdes_bfout1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout2", "qdes_bfout2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout3", "qdes_bfout3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout4", "qdes_bfout4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout5", "qdes_bfout5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout6", "qdes_bfout6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout7", "qdes_bfout7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout8", "qdes_bfout8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout9", "qdes_bfout9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout10", "qdes_bfout10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout11", "qdes_bfout11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout12", "qdes_bfout12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout13", "qdes_bfout13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout14", "qdes_bfout14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout15", "qdes_bfout15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout16", "qdes_bfout16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout17", "qdes_bfout17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout18", "qdes_bfout18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout19", "qdes_bfout19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout20", "qdes_bfout20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout21", "qdes_bfout21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout22", "qdes_bfout22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout23", "qdes_bfout23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout24", "qdes_bfout24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout25", "qdes_bfout25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout26", "qdes_bfout26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout27", "qdes_bfout27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout28", "qdes_bfout28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout29", "qdes_bfout29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout30", "qdes_bfout30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfout31", "qdes_bfout31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_bufin[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdes_bfin0", "qdes_bfin0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin1", "qdes_bfin1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin2", "qdes_bfin2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin3", "qdes_bfin3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin4", "qdes_bfin4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin5", "qdes_bfin5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin6", "qdes_bfin6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin7", "qdes_bfin7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin8", "qdes_bfin8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin9", "qdes_bfin9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin10", "qdes_bfin10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin11", "qdes_bfin11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin12", "qdes_bfin12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin13", "qdes_bfin13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin14", "qdes_bfin14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin15", "qdes_bfin15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin16", "qdes_bfin16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin17", "qdes_bfin17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin18", "qdes_bfin18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin19", "qdes_bfin19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin20", "qdes_bfin20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin21", "qdes_bfin21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin22", "qdes_bfin22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin23", "qdes_bfin23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin24", "qdes_bfin24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin25", "qdes_bfin25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin26", "qdes_bfin26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin27", "qdes_bfin27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin28", "qdes_bfin28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin29", "qdes_bfin29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin30", "qdes_bfin30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_bfin31", "qdes_bfin31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_tail[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tail", "tail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_fbdnull[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull0", "qdes_fbdnull0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull1", "qdes_fbdnull1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull2", "qdes_fbdnull2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull3", "qdes_fbdnull3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull4", "qdes_fbdnull4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull5", "qdes_fbdnull5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull6", "qdes_fbdnull6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull7", "qdes_fbdnull7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull8", "qdes_fbdnull8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull9", "qdes_fbdnull9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull10", "qdes_fbdnull10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull11", "qdes_fbdnull11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull12", "qdes_fbdnull12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull13", "qdes_fbdnull13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull14", "qdes_fbdnull14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull15", "qdes_fbdnull15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull16", "qdes_fbdnull16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull17", "qdes_fbdnull17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull18", "qdes_fbdnull18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull19", "qdes_fbdnull19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull20", "qdes_fbdnull20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull21", "qdes_fbdnull21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull22", "qdes_fbdnull22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull23", "qdes_fbdnull23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull24", "qdes_fbdnull24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull25", "qdes_fbdnull25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull26", "qdes_fbdnull26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull27", "qdes_fbdnull27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull28", "qdes_fbdnull28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull29", "qdes_fbdnull29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull30", "qdes_fbdnull30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_fbdnull31", "qdes_fbdnull31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_nullbd[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd0", "qdes_nullbd0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd1", "qdes_nullbd1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd2", "qdes_nullbd2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd3", "qdes_nullbd3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd4", "qdes_nullbd4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd5", "qdes_nullbd5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd6", "qdes_nullbd6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd7", "qdes_nullbd7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd8", "qdes_nullbd8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd9", "qdes_nullbd9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd10", "qdes_nullbd10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd11", "qdes_nullbd11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd12", "qdes_nullbd12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd13", "qdes_nullbd13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd14", "qdes_nullbd14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd15", "qdes_nullbd15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd16", "qdes_nullbd16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd17", "qdes_nullbd17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd18", "qdes_nullbd18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd19", "qdes_nullbd19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd20", "qdes_nullbd20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd21", "qdes_nullbd21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd22", "qdes_nullbd22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd23", "qdes_nullbd23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd24", "qdes_nullbd24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd25", "qdes_nullbd25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd26", "qdes_nullbd26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd27", "qdes_nullbd27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd28", "qdes_nullbd28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd29", "qdes_nullbd29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd30", "qdes_nullbd30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("qdes_nullbd31", "qdes_nullbd31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_q_head[]={
            BDMFMON_MAKE_PARM("q_head_idx", "q_head_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("head", "head", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_viq_head_vld[]={
            BDMFMON_MAKE_PARM("viq_head_vld", "viq_head_vld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_viq_chrncy_vld[]={
            BDMFMON_MAKE_PARM("chrncy_vld", "chrncy_vld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_veq_head_vld[]={
            BDMFMON_MAKE_PARM("viq_head_vld", "viq_head_vld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_qdes_buf_avl_cntrl[]={
            BDMFMON_MAKE_PARM("use_buf_avl", "use_buf_avl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dec_bufout_when_mltcst", "dec_bufout_when_mltcst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flldes_flldrop[]={
            BDMFMON_MAKE_PARM("drpcnt", "drpcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bdram_next_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bdram_prev_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pdram_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reorder_ram_data0", "reorder_ram_data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("reorder_ram_data1", "reorder_ram_data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="cngs_params", .val=cli_dsptchr_cngs_params, .parms=set_cngs_params },
            { .name="glbl_cngs_params", .val=cli_dsptchr_glbl_cngs_params, .parms=set_glbl_cngs_params },
            { .name="q_size_params", .val=cli_dsptchr_q_size_params, .parms=set_q_size_params },
            { .name="credit_cnt", .val=cli_dsptchr_credit_cnt, .parms=set_credit_cnt },
            { .name="q_limits_params", .val=cli_dsptchr_q_limits_params, .parms=set_q_limits_params },
            { .name="ingress_coherency_params", .val=cli_dsptchr_ingress_coherency_params, .parms=set_ingress_coherency_params },
            { .name="pools_limits", .val=cli_dsptchr_pools_limits, .parms=set_pools_limits },
            { .name="fll_entry", .val=cli_dsptchr_fll_entry, .parms=set_fll_entry },
            { .name="rnr_dsptch_addr", .val=cli_dsptchr_rnr_dsptch_addr, .parms=set_rnr_dsptch_addr },
            { .name="reorder_cfg_dsptchr_reordr_cfg", .val=cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg, .parms=set_reorder_cfg_dsptchr_reordr_cfg },
            { .name="reorder_cfg_vq_en", .val=cli_dsptchr_reorder_cfg_vq_en, .parms=set_reorder_cfg_vq_en },
            { .name="reorder_cfg_bb_cfg", .val=cli_dsptchr_reorder_cfg_bb_cfg, .parms=set_reorder_cfg_bb_cfg },
            { .name="reorder_cfg_clk_gate_cntrl", .val=cli_dsptchr_reorder_cfg_clk_gate_cntrl, .parms=set_reorder_cfg_clk_gate_cntrl },
            { .name="congestion_egrs_congstn", .val=cli_dsptchr_congestion_egrs_congstn, .parms=set_cngs_params },
            { .name="congestion_total_egrs_congstn", .val=cli_dsptchr_congestion_total_egrs_congstn, .parms=set_glbl_cngs_params },
            { .name="queue_mapping_crdt_cfg", .val=cli_dsptchr_queue_mapping_crdt_cfg, .parms=set_queue_mapping_crdt_cfg },
            { .name="queue_mapping_q_dest", .val=cli_dsptchr_queue_mapping_q_dest, .parms=set_queue_mapping_q_dest },
            { .name="mask_msk_tsk_255_0", .val=cli_dsptchr_mask_msk_tsk_255_0, .parms=set_mask_msk_tsk_255_0 },
            { .name="mask_msk_q", .val=cli_dsptchr_mask_msk_q, .parms=set_mask_msk_q },
            { .name="mask_dly_q", .val=cli_dsptchr_mask_dly_q, .parms=set_mask_dly_q },
            { .name="mask_non_dly_q", .val=cli_dsptchr_mask_non_dly_q, .parms=set_mask_non_dly_q },
            { .name="egrs_queues_egrs_dly_qm_crdt", .val=cli_dsptchr_egrs_queues_egrs_dly_qm_crdt, .parms=set_egrs_queues_egrs_dly_qm_crdt },
            { .name="egrs_queues_egrs_non_dly_qm_crdt", .val=cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt, .parms=set_egrs_queues_egrs_non_dly_qm_crdt },
            { .name="egrs_queues_total_q_egrs_size", .val=cli_dsptchr_egrs_queues_total_q_egrs_size, .parms=set_egrs_queues_total_q_egrs_size },
            { .name="wakeup_control_wkup_req", .val=cli_dsptchr_wakeup_control_wkup_req, .parms=set_wakeup_control_wkup_req },
            { .name="wakeup_control_wkup_thrshld", .val=cli_dsptchr_wakeup_control_wkup_thrshld, .parms=set_wakeup_control_wkup_thrshld },
            { .name="disptch_scheduling_dwrr_info", .val=cli_dsptchr_disptch_scheduling_dwrr_info, .parms=set_disptch_scheduling_dwrr_info },
            { .name="disptch_scheduling_vld_crdt", .val=cli_dsptchr_disptch_scheduling_vld_crdt, .parms=set_disptch_scheduling_vld_crdt },
            { .name="load_balancing_lb_cfg", .val=cli_dsptchr_load_balancing_lb_cfg, .parms=set_load_balancing_lb_cfg },
            { .name="load_balancing_free_task_0_1", .val=cli_dsptchr_load_balancing_free_task_0_1, .parms=set_load_balancing_free_task_0_1 },
            { .name="load_balancing_free_task_2_3", .val=cli_dsptchr_load_balancing_free_task_2_3, .parms=set_load_balancing_free_task_2_3 },
            { .name="load_balancing_free_task_4_5", .val=cli_dsptchr_load_balancing_free_task_4_5, .parms=set_load_balancing_free_task_4_5 },
            { .name="load_balancing_free_task_6_7", .val=cli_dsptchr_load_balancing_free_task_6_7, .parms=set_load_balancing_free_task_6_7 },
            { .name="load_balancing_free_task_8_9", .val=cli_dsptchr_load_balancing_free_task_8_9, .parms=set_load_balancing_free_task_8_9 },
            { .name="load_balancing_free_task_10_11", .val=cli_dsptchr_load_balancing_free_task_10_11, .parms=set_load_balancing_free_task_10_11 },
            { .name="load_balancing_free_task_12_13", .val=cli_dsptchr_load_balancing_free_task_12_13, .parms=set_load_balancing_free_task_12_13 },
            { .name="load_balancing_free_task_14_15", .val=cli_dsptchr_load_balancing_free_task_14_15, .parms=set_load_balancing_free_task_14_15 },
            { .name="load_balancing_tsk_to_rg_mapping", .val=cli_dsptchr_load_balancing_tsk_to_rg_mapping, .parms=set_load_balancing_tsk_to_rg_mapping },
            { .name="load_balancing_rg_avlabl_tsk_0_3", .val=cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3, .parms=set_load_balancing_rg_avlabl_tsk_0_3 },
            { .name="load_balancing_rg_avlabl_tsk_4_7", .val=cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7, .parms=set_load_balancing_rg_avlabl_tsk_4_7 },
            { .name="dsptcher_reordr_top_intr_ctrl_0_isr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr, .parms=set_dsptcher_reordr_top_intr_ctrl_0_isr },
            { .name="dsptcher_reordr_top_intr_ctrl_0_ier", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier, .parms=set_dsptcher_reordr_top_intr_ctrl_0_ier },
            { .name="dsptcher_reordr_top_intr_ctrl_0_itr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr, .parms=set_dsptcher_reordr_top_intr_ctrl_0_itr },
            { .name="dsptcher_reordr_top_intr_ctrl_1_isr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr, .parms=set_dsptcher_reordr_top_intr_ctrl_1_isr },
            { .name="dsptcher_reordr_top_intr_ctrl_1_ier", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier, .parms=set_dsptcher_reordr_top_intr_ctrl_1_ier },
            { .name="dsptcher_reordr_top_intr_ctrl_1_itr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr, .parms=set_dsptcher_reordr_top_intr_ctrl_1_itr },
            { .name="debug_dbg_bypss_cntrl", .val=cli_dsptchr_debug_dbg_bypss_cntrl, .parms=set_debug_dbg_bypss_cntrl },
            { .name="debug_glbl_tsk_cnt_0_7", .val=cli_dsptchr_debug_glbl_tsk_cnt_0_7, .parms=set_debug_glbl_tsk_cnt_0_7 },
            { .name="debug_glbl_tsk_cnt_8_15", .val=cli_dsptchr_debug_glbl_tsk_cnt_8_15, .parms=set_debug_glbl_tsk_cnt_8_15 },
            { .name="debug_dbg_bus_cntrl", .val=cli_dsptchr_debug_dbg_bus_cntrl, .parms=set_debug_dbg_bus_cntrl },
            { .name="debug_statistics_dbg_sttstcs_ctrl", .val=cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl, .parms=set_debug_statistics_dbg_sttstcs_ctrl },
            { .name="qdes_head", .val=cli_dsptchr_qdes_head, .parms=set_qdes_head },
            { .name="qdes_bfout", .val=cli_dsptchr_qdes_bfout, .parms=set_qdes_bfout },
            { .name="qdes_bufin", .val=cli_dsptchr_qdes_bufin, .parms=set_qdes_bufin },
            { .name="qdes_tail", .val=cli_dsptchr_qdes_tail, .parms=set_qdes_tail },
            { .name="qdes_fbdnull", .val=cli_dsptchr_qdes_fbdnull, .parms=set_qdes_fbdnull },
            { .name="qdes_nullbd", .val=cli_dsptchr_qdes_nullbd, .parms=set_qdes_nullbd },
            { .name="qdes_reg_q_head", .val=cli_dsptchr_qdes_reg_q_head, .parms=set_qdes_reg_q_head },
            { .name="qdes_reg_viq_head_vld", .val=cli_dsptchr_qdes_reg_viq_head_vld, .parms=set_qdes_reg_viq_head_vld },
            { .name="qdes_reg_viq_chrncy_vld", .val=cli_dsptchr_qdes_reg_viq_chrncy_vld, .parms=set_qdes_reg_viq_chrncy_vld },
            { .name="qdes_reg_veq_head_vld", .val=cli_dsptchr_qdes_reg_veq_head_vld, .parms=set_qdes_reg_veq_head_vld },
            { .name="qdes_reg_qdes_buf_avl_cntrl", .val=cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl, .parms=set_qdes_reg_qdes_buf_avl_cntrl },
            { .name="flldes_flldrop", .val=cli_dsptchr_flldes_flldrop, .parms=set_flldes_flldrop },
            { .name="bdram_next_data", .val=cli_dsptchr_bdram_next_data, .parms=set_bdram_next_data },
            { .name="bdram_prev_data", .val=cli_dsptchr_bdram_prev_data, .parms=set_bdram_prev_data },
            { .name="pdram_data", .val=cli_dsptchr_pdram_data, .parms=set_pdram_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_dsptchr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cngs_params[]={
            BDMFMON_MAKE_PARM("viq_idx", "viq_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_size_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_credit_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_limits_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ingress_coherency_params[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_dsptch_addr[]={
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_mapping_crdt_cfg[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_msk_tsk_255_0[]={
            BDMFMON_MAKE_PARM("group_idx", "group_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mask_msk_q[]={
            BDMFMON_MAKE_PARM("group_idx", "group_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_egrs_queues_per_q_egrs_size[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disptch_scheduling_dwrr_info[]={
            BDMFMON_MAKE_PARM("dwrr_q_idx", "dwrr_q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_load_balancing_tsk_to_rg_mapping[]={
            BDMFMON_MAKE_PARM("task_to_rg_mapping", "task_to_rg_mapping", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_statistics_dbg_cnt[]={
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_head[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_bfout[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_bufin[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_tail[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_fbdnull[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_nullbd[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_bufavail[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qdes_reg_q_head[]={
            BDMFMON_MAKE_PARM("q_head_idx", "q_head_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bdram_next_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bdram_prev_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pdram_data[]={
            BDMFMON_MAKE_PARM("temp_index", "temp_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="cngs_params", .val=cli_dsptchr_cngs_params, .parms=set_cngs_params },
            { .name="glbl_cngs_params", .val=cli_dsptchr_glbl_cngs_params, .parms=set_default },
            { .name="q_size_params", .val=cli_dsptchr_q_size_params, .parms=set_q_size_params },
            { .name="credit_cnt", .val=cli_dsptchr_credit_cnt, .parms=set_credit_cnt },
            { .name="q_limits_params", .val=cli_dsptchr_q_limits_params, .parms=set_q_limits_params },
            { .name="ingress_coherency_params", .val=cli_dsptchr_ingress_coherency_params, .parms=set_ingress_coherency_params },
            { .name="pools_limits", .val=cli_dsptchr_pools_limits, .parms=set_default },
            { .name="fll_entry", .val=cli_dsptchr_fll_entry, .parms=set_default },
            { .name="rnr_dsptch_addr", .val=cli_dsptchr_rnr_dsptch_addr, .parms=set_rnr_dsptch_addr },
            { .name="reorder_cfg_dsptchr_reordr_cfg", .val=cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg, .parms=set_default },
            { .name="reorder_cfg_vq_en", .val=cli_dsptchr_reorder_cfg_vq_en, .parms=set_default },
            { .name="reorder_cfg_bb_cfg", .val=cli_dsptchr_reorder_cfg_bb_cfg, .parms=set_default },
            { .name="reorder_cfg_clk_gate_cntrl", .val=cli_dsptchr_reorder_cfg_clk_gate_cntrl, .parms=set_default },
            { .name="congestion_egrs_congstn", .val=cli_dsptchr_congestion_egrs_congstn, .parms=set_cngs_params },
            { .name="congestion_total_egrs_congstn", .val=cli_dsptchr_congestion_total_egrs_congstn, .parms=set_default },
            { .name="congestion_congstn_status", .val=cli_dsptchr_congestion_congstn_status, .parms=set_default },
            { .name="congestion_per_q_ingrs_congstn_low", .val=cli_dsptchr_congestion_per_q_ingrs_congstn_low, .parms=set_default },
            { .name="congestion_per_q_ingrs_congstn_high", .val=cli_dsptchr_congestion_per_q_ingrs_congstn_high, .parms=set_default },
            { .name="congestion_per_q_egrs_congstn_low", .val=cli_dsptchr_congestion_per_q_egrs_congstn_low, .parms=set_default },
            { .name="congestion_per_q_egrs_congstn_high", .val=cli_dsptchr_congestion_per_q_egrs_congstn_high, .parms=set_default },
            { .name="queue_mapping_crdt_cfg", .val=cli_dsptchr_queue_mapping_crdt_cfg, .parms=set_queue_mapping_crdt_cfg },
            { .name="queue_mapping_q_dest", .val=cli_dsptchr_queue_mapping_q_dest, .parms=set_default },
            { .name="mask_msk_tsk_255_0", .val=cli_dsptchr_mask_msk_tsk_255_0, .parms=set_mask_msk_tsk_255_0 },
            { .name="mask_msk_q", .val=cli_dsptchr_mask_msk_q, .parms=set_mask_msk_q },
            { .name="mask_dly_q", .val=cli_dsptchr_mask_dly_q, .parms=set_default },
            { .name="mask_non_dly_q", .val=cli_dsptchr_mask_non_dly_q, .parms=set_default },
            { .name="egrs_queues_egrs_dly_qm_crdt", .val=cli_dsptchr_egrs_queues_egrs_dly_qm_crdt, .parms=set_default },
            { .name="egrs_queues_egrs_non_dly_qm_crdt", .val=cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt, .parms=set_default },
            { .name="egrs_queues_total_q_egrs_size", .val=cli_dsptchr_egrs_queues_total_q_egrs_size, .parms=set_default },
            { .name="egrs_queues_per_q_egrs_size", .val=cli_dsptchr_egrs_queues_per_q_egrs_size, .parms=set_egrs_queues_per_q_egrs_size },
            { .name="wakeup_control_wkup_req", .val=cli_dsptchr_wakeup_control_wkup_req, .parms=set_default },
            { .name="wakeup_control_wkup_thrshld", .val=cli_dsptchr_wakeup_control_wkup_thrshld, .parms=set_default },
            { .name="disptch_scheduling_dwrr_info", .val=cli_dsptchr_disptch_scheduling_dwrr_info, .parms=set_disptch_scheduling_dwrr_info },
            { .name="disptch_scheduling_vld_crdt", .val=cli_dsptchr_disptch_scheduling_vld_crdt, .parms=set_default },
            { .name="load_balancing_lb_cfg", .val=cli_dsptchr_load_balancing_lb_cfg, .parms=set_default },
            { .name="load_balancing_free_task_0_1", .val=cli_dsptchr_load_balancing_free_task_0_1, .parms=set_default },
            { .name="load_balancing_free_task_2_3", .val=cli_dsptchr_load_balancing_free_task_2_3, .parms=set_default },
            { .name="load_balancing_free_task_4_5", .val=cli_dsptchr_load_balancing_free_task_4_5, .parms=set_default },
            { .name="load_balancing_free_task_6_7", .val=cli_dsptchr_load_balancing_free_task_6_7, .parms=set_default },
            { .name="load_balancing_free_task_8_9", .val=cli_dsptchr_load_balancing_free_task_8_9, .parms=set_default },
            { .name="load_balancing_free_task_10_11", .val=cli_dsptchr_load_balancing_free_task_10_11, .parms=set_default },
            { .name="load_balancing_free_task_12_13", .val=cli_dsptchr_load_balancing_free_task_12_13, .parms=set_default },
            { .name="load_balancing_free_task_14_15", .val=cli_dsptchr_load_balancing_free_task_14_15, .parms=set_default },
            { .name="load_balancing_tsk_to_rg_mapping", .val=cli_dsptchr_load_balancing_tsk_to_rg_mapping, .parms=set_load_balancing_tsk_to_rg_mapping },
            { .name="load_balancing_rg_avlabl_tsk_0_3", .val=cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3, .parms=set_default },
            { .name="load_balancing_rg_avlabl_tsk_4_7", .val=cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_0_isr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_0_ism", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_0_ier", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_0_itr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_1_isr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_1_ism", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_1_ier", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier, .parms=set_default },
            { .name="dsptcher_reordr_top_intr_ctrl_1_itr", .val=cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr, .parms=set_default },
            { .name="debug_dbg_bypss_cntrl", .val=cli_dsptchr_debug_dbg_bypss_cntrl, .parms=set_default },
            { .name="debug_glbl_tsk_cnt_0_7", .val=cli_dsptchr_debug_glbl_tsk_cnt_0_7, .parms=set_default },
            { .name="debug_glbl_tsk_cnt_8_15", .val=cli_dsptchr_debug_glbl_tsk_cnt_8_15, .parms=set_default },
            { .name="debug_dbg_bus_cntrl", .val=cli_dsptchr_debug_dbg_bus_cntrl, .parms=set_default },
            { .name="debug_dbg_vec_0", .val=cli_dsptchr_debug_dbg_vec_0, .parms=set_default },
            { .name="debug_dbg_vec_1", .val=cli_dsptchr_debug_dbg_vec_1, .parms=set_default },
            { .name="debug_dbg_vec_2", .val=cli_dsptchr_debug_dbg_vec_2, .parms=set_default },
            { .name="debug_dbg_vec_3", .val=cli_dsptchr_debug_dbg_vec_3, .parms=set_default },
            { .name="debug_dbg_vec_4", .val=cli_dsptchr_debug_dbg_vec_4, .parms=set_default },
            { .name="debug_dbg_vec_5", .val=cli_dsptchr_debug_dbg_vec_5, .parms=set_default },
            { .name="debug_dbg_vec_6", .val=cli_dsptchr_debug_dbg_vec_6, .parms=set_default },
            { .name="debug_dbg_vec_7", .val=cli_dsptchr_debug_dbg_vec_7, .parms=set_default },
            { .name="debug_dbg_vec_8", .val=cli_dsptchr_debug_dbg_vec_8, .parms=set_default },
            { .name="debug_dbg_vec_9", .val=cli_dsptchr_debug_dbg_vec_9, .parms=set_default },
            { .name="debug_dbg_vec_10", .val=cli_dsptchr_debug_dbg_vec_10, .parms=set_default },
            { .name="debug_dbg_vec_11", .val=cli_dsptchr_debug_dbg_vec_11, .parms=set_default },
            { .name="debug_dbg_vec_12", .val=cli_dsptchr_debug_dbg_vec_12, .parms=set_default },
            { .name="debug_dbg_vec_13", .val=cli_dsptchr_debug_dbg_vec_13, .parms=set_default },
            { .name="debug_dbg_vec_14", .val=cli_dsptchr_debug_dbg_vec_14, .parms=set_default },
            { .name="debug_dbg_vec_15", .val=cli_dsptchr_debug_dbg_vec_15, .parms=set_default },
            { .name="debug_dbg_vec_16", .val=cli_dsptchr_debug_dbg_vec_16, .parms=set_default },
            { .name="debug_dbg_vec_17", .val=cli_dsptchr_debug_dbg_vec_17, .parms=set_default },
            { .name="debug_dbg_vec_18", .val=cli_dsptchr_debug_dbg_vec_18, .parms=set_default },
            { .name="debug_dbg_vec_19", .val=cli_dsptchr_debug_dbg_vec_19, .parms=set_default },
            { .name="debug_dbg_vec_20", .val=cli_dsptchr_debug_dbg_vec_20, .parms=set_default },
            { .name="debug_dbg_vec_21", .val=cli_dsptchr_debug_dbg_vec_21, .parms=set_default },
            { .name="debug_dbg_vec_22", .val=cli_dsptchr_debug_dbg_vec_22, .parms=set_default },
            { .name="debug_dbg_vec_23", .val=cli_dsptchr_debug_dbg_vec_23, .parms=set_default },
            { .name="debug_statistics_dbg_sttstcs_ctrl", .val=cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl, .parms=set_default },
            { .name="debug_statistics_dbg_cnt", .val=cli_dsptchr_debug_statistics_dbg_cnt, .parms=set_debug_statistics_dbg_cnt },
            { .name="qdes_head", .val=cli_dsptchr_qdes_head, .parms=set_qdes_head },
            { .name="qdes_bfout", .val=cli_dsptchr_qdes_bfout, .parms=set_qdes_bfout },
            { .name="qdes_bufin", .val=cli_dsptchr_qdes_bufin, .parms=set_qdes_bufin },
            { .name="qdes_tail", .val=cli_dsptchr_qdes_tail, .parms=set_qdes_tail },
            { .name="qdes_fbdnull", .val=cli_dsptchr_qdes_fbdnull, .parms=set_qdes_fbdnull },
            { .name="qdes_nullbd", .val=cli_dsptchr_qdes_nullbd, .parms=set_qdes_nullbd },
            { .name="qdes_bufavail", .val=cli_dsptchr_qdes_bufavail, .parms=set_qdes_bufavail },
            { .name="qdes_reg_q_head", .val=cli_dsptchr_qdes_reg_q_head, .parms=set_qdes_reg_q_head },
            { .name="qdes_reg_viq_head_vld", .val=cli_dsptchr_qdes_reg_viq_head_vld, .parms=set_default },
            { .name="qdes_reg_viq_chrncy_vld", .val=cli_dsptchr_qdes_reg_viq_chrncy_vld, .parms=set_default },
            { .name="qdes_reg_veq_head_vld", .val=cli_dsptchr_qdes_reg_veq_head_vld, .parms=set_default },
            { .name="qdes_reg_qdes_buf_avl_cntrl", .val=cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl, .parms=set_default },
            { .name="flldes_flldrop", .val=cli_dsptchr_flldes_flldrop, .parms=set_default },
            { .name="flldes_bufavail", .val=cli_dsptchr_flldes_bufavail, .parms=set_default },
            { .name="flldes_freemin", .val=cli_dsptchr_flldes_freemin, .parms=set_default },
            { .name="bdram_next_data", .val=cli_dsptchr_bdram_next_data, .parms=set_bdram_next_data },
            { .name="bdram_prev_data", .val=cli_dsptchr_bdram_prev_data, .parms=set_bdram_prev_data },
            { .name="pdram_data", .val=cli_dsptchr_pdram_data, .parms=set_pdram_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_dsptchr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_dsptchr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="REORDER_CFG_DSPTCHR_REORDR_CFG" , .val=bdmf_address_reorder_cfg_dsptchr_reordr_cfg },
            { .name="REORDER_CFG_VQ_EN" , .val=bdmf_address_reorder_cfg_vq_en },
            { .name="REORDER_CFG_BB_CFG" , .val=bdmf_address_reorder_cfg_bb_cfg },
            { .name="REORDER_CFG_CLK_GATE_CNTRL" , .val=bdmf_address_reorder_cfg_clk_gate_cntrl },
            { .name="CONGESTION_INGRS_CONGSTN" , .val=bdmf_address_congestion_ingrs_congstn },
            { .name="CONGESTION_EGRS_CONGSTN" , .val=bdmf_address_congestion_egrs_congstn },
            { .name="CONGESTION_TOTAL_EGRS_CONGSTN" , .val=bdmf_address_congestion_total_egrs_congstn },
            { .name="CONGESTION_GLBL_CONGSTN" , .val=bdmf_address_congestion_glbl_congstn },
            { .name="CONGESTION_CONGSTN_STATUS" , .val=bdmf_address_congestion_congstn_status },
            { .name="CONGESTION_PER_Q_INGRS_CONGSTN_LOW" , .val=bdmf_address_congestion_per_q_ingrs_congstn_low },
            { .name="CONGESTION_PER_Q_INGRS_CONGSTN_HIGH" , .val=bdmf_address_congestion_per_q_ingrs_congstn_high },
            { .name="CONGESTION_PER_Q_EGRS_CONGSTN_LOW" , .val=bdmf_address_congestion_per_q_egrs_congstn_low },
            { .name="CONGESTION_PER_Q_EGRS_CONGSTN_HIGH" , .val=bdmf_address_congestion_per_q_egrs_congstn_high },
            { .name="INGRS_QUEUES_Q_INGRS_SIZE" , .val=bdmf_address_ingrs_queues_q_ingrs_size },
            { .name="INGRS_QUEUES_Q_INGRS_LIMITS" , .val=bdmf_address_ingrs_queues_q_ingrs_limits },
            { .name="QUEUE_MAPPING_CRDT_CFG" , .val=bdmf_address_queue_mapping_crdt_cfg },
            { .name="QUEUE_MAPPING_PD_DSPTCH_ADD" , .val=bdmf_address_queue_mapping_pd_dsptch_add },
            { .name="QUEUE_MAPPING_Q_DEST" , .val=bdmf_address_queue_mapping_q_dest },
            { .name="POOL_SIZES_CMN_POOL_LMT" , .val=bdmf_address_pool_sizes_cmn_pool_lmt },
            { .name="POOL_SIZES_CMN_POOL_SIZE" , .val=bdmf_address_pool_sizes_cmn_pool_size },
            { .name="POOL_SIZES_GRNTED_POOL_LMT" , .val=bdmf_address_pool_sizes_grnted_pool_lmt },
            { .name="POOL_SIZES_GRNTED_POOL_SIZE" , .val=bdmf_address_pool_sizes_grnted_pool_size },
            { .name="POOL_SIZES_MULTI_CST_POOL_LMT" , .val=bdmf_address_pool_sizes_multi_cst_pool_lmt },
            { .name="POOL_SIZES_MULTI_CST_POOL_SIZE" , .val=bdmf_address_pool_sizes_multi_cst_pool_size },
            { .name="POOL_SIZES_RNR_POOL_LMT" , .val=bdmf_address_pool_sizes_rnr_pool_lmt },
            { .name="POOL_SIZES_RNR_POOL_SIZE" , .val=bdmf_address_pool_sizes_rnr_pool_size },
            { .name="POOL_SIZES_PRCSSING_POOL_SIZE" , .val=bdmf_address_pool_sizes_prcssing_pool_size },
            { .name="MASK_MSK_TSK_255_0" , .val=bdmf_address_mask_msk_tsk_255_0 },
            { .name="MASK_MSK_Q" , .val=bdmf_address_mask_msk_q },
            { .name="MASK_DLY_Q" , .val=bdmf_address_mask_dly_q },
            { .name="MASK_NON_DLY_Q" , .val=bdmf_address_mask_non_dly_q },
            { .name="EGRS_QUEUES_EGRS_DLY_QM_CRDT" , .val=bdmf_address_egrs_queues_egrs_dly_qm_crdt },
            { .name="EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT" , .val=bdmf_address_egrs_queues_egrs_non_dly_qm_crdt },
            { .name="EGRS_QUEUES_TOTAL_Q_EGRS_SIZE" , .val=bdmf_address_egrs_queues_total_q_egrs_size },
            { .name="EGRS_QUEUES_PER_Q_EGRS_SIZE" , .val=bdmf_address_egrs_queues_per_q_egrs_size },
            { .name="WAKEUP_CONTROL_WKUP_REQ" , .val=bdmf_address_wakeup_control_wkup_req },
            { .name="WAKEUP_CONTROL_WKUP_THRSHLD" , .val=bdmf_address_wakeup_control_wkup_thrshld },
            { .name="DISPTCH_SCHEDULING_DWRR_INFO" , .val=bdmf_address_disptch_scheduling_dwrr_info },
            { .name="DISPTCH_SCHEDULING_VLD_CRDT" , .val=bdmf_address_disptch_scheduling_vld_crdt },
            { .name="LOAD_BALANCING_LB_CFG" , .val=bdmf_address_load_balancing_lb_cfg },
            { .name="LOAD_BALANCING_FREE_TASK_0_1" , .val=bdmf_address_load_balancing_free_task_0_1 },
            { .name="LOAD_BALANCING_FREE_TASK_2_3" , .val=bdmf_address_load_balancing_free_task_2_3 },
            { .name="LOAD_BALANCING_FREE_TASK_4_5" , .val=bdmf_address_load_balancing_free_task_4_5 },
            { .name="LOAD_BALANCING_FREE_TASK_6_7" , .val=bdmf_address_load_balancing_free_task_6_7 },
            { .name="LOAD_BALANCING_FREE_TASK_8_9" , .val=bdmf_address_load_balancing_free_task_8_9 },
            { .name="LOAD_BALANCING_FREE_TASK_10_11" , .val=bdmf_address_load_balancing_free_task_10_11 },
            { .name="LOAD_BALANCING_FREE_TASK_12_13" , .val=bdmf_address_load_balancing_free_task_12_13 },
            { .name="LOAD_BALANCING_FREE_TASK_14_15" , .val=bdmf_address_load_balancing_free_task_14_15 },
            { .name="LOAD_BALANCING_TSK_TO_RG_MAPPING" , .val=bdmf_address_load_balancing_tsk_to_rg_mapping },
            { .name="LOAD_BALANCING_RG_AVLABL_TSK_0_3" , .val=bdmf_address_load_balancing_rg_avlabl_tsk_0_3 },
            { .name="LOAD_BALANCING_RG_AVLABL_TSK_4_7" , .val=bdmf_address_load_balancing_rg_avlabl_tsk_4_7 },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_0_isr },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ism },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_0_ier },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_0_itr },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_1_isr },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ism },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_1_ier },
            { .name="DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR" , .val=bdmf_address_dsptcher_reordr_top_intr_ctrl_1_itr },
            { .name="DEBUG_DBG_BYPSS_CNTRL" , .val=bdmf_address_debug_dbg_bypss_cntrl },
            { .name="DEBUG_GLBL_TSK_CNT_0_7" , .val=bdmf_address_debug_glbl_tsk_cnt_0_7 },
            { .name="DEBUG_GLBL_TSK_CNT_8_15" , .val=bdmf_address_debug_glbl_tsk_cnt_8_15 },
            { .name="DEBUG_DBG_BUS_CNTRL" , .val=bdmf_address_debug_dbg_bus_cntrl },
            { .name="DEBUG_DBG_VEC_0" , .val=bdmf_address_debug_dbg_vec_0 },
            { .name="DEBUG_DBG_VEC_1" , .val=bdmf_address_debug_dbg_vec_1 },
            { .name="DEBUG_DBG_VEC_2" , .val=bdmf_address_debug_dbg_vec_2 },
            { .name="DEBUG_DBG_VEC_3" , .val=bdmf_address_debug_dbg_vec_3 },
            { .name="DEBUG_DBG_VEC_4" , .val=bdmf_address_debug_dbg_vec_4 },
            { .name="DEBUG_DBG_VEC_5" , .val=bdmf_address_debug_dbg_vec_5 },
            { .name="DEBUG_DBG_VEC_6" , .val=bdmf_address_debug_dbg_vec_6 },
            { .name="DEBUG_DBG_VEC_7" , .val=bdmf_address_debug_dbg_vec_7 },
            { .name="DEBUG_DBG_VEC_8" , .val=bdmf_address_debug_dbg_vec_8 },
            { .name="DEBUG_DBG_VEC_9" , .val=bdmf_address_debug_dbg_vec_9 },
            { .name="DEBUG_DBG_VEC_10" , .val=bdmf_address_debug_dbg_vec_10 },
            { .name="DEBUG_DBG_VEC_11" , .val=bdmf_address_debug_dbg_vec_11 },
            { .name="DEBUG_DBG_VEC_12" , .val=bdmf_address_debug_dbg_vec_12 },
            { .name="DEBUG_DBG_VEC_13" , .val=bdmf_address_debug_dbg_vec_13 },
            { .name="DEBUG_DBG_VEC_14" , .val=bdmf_address_debug_dbg_vec_14 },
            { .name="DEBUG_DBG_VEC_15" , .val=bdmf_address_debug_dbg_vec_15 },
            { .name="DEBUG_DBG_VEC_16" , .val=bdmf_address_debug_dbg_vec_16 },
            { .name="DEBUG_DBG_VEC_17" , .val=bdmf_address_debug_dbg_vec_17 },
            { .name="DEBUG_DBG_VEC_18" , .val=bdmf_address_debug_dbg_vec_18 },
            { .name="DEBUG_DBG_VEC_19" , .val=bdmf_address_debug_dbg_vec_19 },
            { .name="DEBUG_DBG_VEC_20" , .val=bdmf_address_debug_dbg_vec_20 },
            { .name="DEBUG_DBG_VEC_21" , .val=bdmf_address_debug_dbg_vec_21 },
            { .name="DEBUG_DBG_VEC_22" , .val=bdmf_address_debug_dbg_vec_22 },
            { .name="DEBUG_DBG_VEC_23" , .val=bdmf_address_debug_dbg_vec_23 },
            { .name="DEBUG_STATISTICS_DBG_STTSTCS_CTRL" , .val=bdmf_address_debug_statistics_dbg_sttstcs_ctrl },
            { .name="DEBUG_STATISTICS_DBG_CNT" , .val=bdmf_address_debug_statistics_dbg_cnt },
            { .name="QDES_HEAD" , .val=bdmf_address_qdes_head },
            { .name="QDES_BFOUT" , .val=bdmf_address_qdes_bfout },
            { .name="QDES_BUFIN" , .val=bdmf_address_qdes_bufin },
            { .name="QDES_TAIL" , .val=bdmf_address_qdes_tail },
            { .name="QDES_FBDNULL" , .val=bdmf_address_qdes_fbdnull },
            { .name="QDES_NULLBD" , .val=bdmf_address_qdes_nullbd },
            { .name="QDES_BUFAVAIL" , .val=bdmf_address_qdes_bufavail },
            { .name="QDES_REG_Q_HEAD" , .val=bdmf_address_qdes_reg_q_head },
            { .name="QDES_REG_VIQ_HEAD_VLD" , .val=bdmf_address_qdes_reg_viq_head_vld },
            { .name="QDES_REG_VIQ_CHRNCY_VLD" , .val=bdmf_address_qdes_reg_viq_chrncy_vld },
            { .name="QDES_REG_VEQ_HEAD_VLD" , .val=bdmf_address_qdes_reg_veq_head_vld },
            { .name="QDES_REG_QDES_BUF_AVL_CNTRL" , .val=bdmf_address_qdes_reg_qdes_buf_avl_cntrl },
            { .name="FLLDES_HEAD" , .val=bdmf_address_flldes_head },
            { .name="FLLDES_BFOUT" , .val=bdmf_address_flldes_bfout },
            { .name="FLLDES_BFIN" , .val=bdmf_address_flldes_bfin },
            { .name="FLLDES_TAIL" , .val=bdmf_address_flldes_tail },
            { .name="FLLDES_FLLDROP" , .val=bdmf_address_flldes_flldrop },
            { .name="FLLDES_LTINT" , .val=bdmf_address_flldes_ltint },
            { .name="FLLDES_BUFAVAIL" , .val=bdmf_address_flldes_bufavail },
            { .name="FLLDES_FREEMIN" , .val=bdmf_address_flldes_freemin },
            { .name="BDRAM_NEXT_DATA" , .val=bdmf_address_bdram_next_data },
            { .name="BDRAM_PREV_DATA" , .val=bdmf_address_bdram_prev_data },
            { .name="PDRAM_DATA" , .val=bdmf_address_pdram_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_dsptchr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

