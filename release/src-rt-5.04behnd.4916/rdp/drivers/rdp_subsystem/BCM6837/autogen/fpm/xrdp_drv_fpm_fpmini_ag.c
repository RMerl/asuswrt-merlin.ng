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
#include "xrdp_drv_fpm_fpmini_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_set(uint32_t data)
{
    uint32_t reg_fpmini_lvl_0_reg_l0 = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_fpmini_lvl_0_reg_l0 = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_LVL_0_REG_L0, DATA, reg_fpmini_lvl_0_reg_l0, data);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_LVL_0_REG_L0, reg_fpmini_lvl_0_reg_l0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_get(uint32_t *data)
{
    uint32_t reg_fpmini_lvl_0_reg_l0;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_LVL_0_REG_L0, reg_fpmini_lvl_0_reg_l0);

    *data = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_LVL_0_REG_L0, DATA, reg_fpmini_lvl_0_reg_l0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_set(bdmf_boolean init)
{
    uint32_t reg_fpmini_cfg0_l2_init = 0;

#ifdef VALIDATE_PARMS
    if ((init >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmini_cfg0_l2_init = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_CFG0_L2_INIT, INIT, reg_fpmini_cfg0_l2_init, init);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_CFG0_L2_INIT, reg_fpmini_cfg0_l2_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_get(bdmf_boolean *init)
{
    uint32_t reg_fpmini_cfg0_l2_init;

#ifdef VALIDATE_PARMS
    if (!init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_CFG0_L2_INIT, reg_fpmini_cfg0_l2_init);

    *init = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_CFG0_L2_INIT, INIT, reg_fpmini_cfg0_l2_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_set(bdmf_boolean en)
{
    uint32_t reg_fpmini_cfg0_allc_fast_ack = 0;

#ifdef VALIDATE_PARMS
    if ((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmini_cfg0_allc_fast_ack = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_CFG0_ALLC_FAST_ACK, EN, reg_fpmini_cfg0_allc_fast_ack, en);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_CFG0_ALLC_FAST_ACK, reg_fpmini_cfg0_allc_fast_ack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_get(bdmf_boolean *en)
{
    uint32_t reg_fpmini_cfg0_allc_fast_ack;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_CFG0_ALLC_FAST_ACK, reg_fpmini_cfg0_allc_fast_ack);

    *en = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_CFG0_ALLC_FAST_ACK, EN, reg_fpmini_cfg0_allc_fast_ack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_get(uint32_t *val)
{
    uint32_t reg_fpmini_prfm_cntrs_num_avail_tkns;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS, reg_fpmini_prfm_cntrs_num_avail_tkns);

    *val = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS, VAL, reg_fpmini_prfm_cntrs_num_avail_tkns);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_set(uint32_t val)
{
    uint32_t reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM, VAL, reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm, val);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM, reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_get(uint32_t *val)
{
    uint32_t reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM, reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm);

    *val = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM, VAL, reg_fpmini_prfm_cntrs_num_avail_tkns_low_wm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_free_err_get(uint32_t *val)
{
    uint32_t reg_fpmini_prfm_cntrs_free_err;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_FREE_ERR, reg_fpmini_prfm_cntrs_free_err);

    *val = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_FREE_ERR, VAL, reg_fpmini_prfm_cntrs_free_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_set(const fpm_fpmini_fpmini_prfm_cntrs_gen_cfg *fpmini_prfm_cntrs_gen_cfg)
{
    uint32_t reg_fpmini_prfm_cntrs_gen_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!fpmini_prfm_cntrs_gen_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((fpmini_prfm_cntrs_gen_cfg->rd_clr >= _1BITS_MAX_VAL_) ||
       (fpmini_prfm_cntrs_gen_cfg->wrap >= _1BITS_MAX_VAL_) ||
       (fpmini_prfm_cntrs_gen_cfg->msk_free_err >= _1BITS_MAX_VAL_) ||
       (fpmini_prfm_cntrs_gen_cfg->msk_mc_inc_err >= _1BITS_MAX_VAL_) ||
       (fpmini_prfm_cntrs_gen_cfg->msk_mc_dec_err >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmini_prfm_cntrs_gen_cfg = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, RD_CLR, reg_fpmini_prfm_cntrs_gen_cfg, fpmini_prfm_cntrs_gen_cfg->rd_clr);
    reg_fpmini_prfm_cntrs_gen_cfg = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, WRAP, reg_fpmini_prfm_cntrs_gen_cfg, fpmini_prfm_cntrs_gen_cfg->wrap);
    reg_fpmini_prfm_cntrs_gen_cfg = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_FREE_ERR, reg_fpmini_prfm_cntrs_gen_cfg, fpmini_prfm_cntrs_gen_cfg->msk_free_err);
    reg_fpmini_prfm_cntrs_gen_cfg = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_MC_INC_ERR, reg_fpmini_prfm_cntrs_gen_cfg, fpmini_prfm_cntrs_gen_cfg->msk_mc_inc_err);
    reg_fpmini_prfm_cntrs_gen_cfg = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_MC_DEC_ERR, reg_fpmini_prfm_cntrs_gen_cfg, fpmini_prfm_cntrs_gen_cfg->msk_mc_dec_err);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, reg_fpmini_prfm_cntrs_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_get(fpm_fpmini_fpmini_prfm_cntrs_gen_cfg *fpmini_prfm_cntrs_gen_cfg)
{
    uint32_t reg_fpmini_prfm_cntrs_gen_cfg;

#ifdef VALIDATE_PARMS
    if (!fpmini_prfm_cntrs_gen_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, reg_fpmini_prfm_cntrs_gen_cfg);

    fpmini_prfm_cntrs_gen_cfg->rd_clr = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, RD_CLR, reg_fpmini_prfm_cntrs_gen_cfg);
    fpmini_prfm_cntrs_gen_cfg->wrap = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, WRAP, reg_fpmini_prfm_cntrs_gen_cfg);
    fpmini_prfm_cntrs_gen_cfg->msk_free_err = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_FREE_ERR, reg_fpmini_prfm_cntrs_gen_cfg);
    fpmini_prfm_cntrs_gen_cfg->msk_mc_inc_err = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_MC_INC_ERR, reg_fpmini_prfm_cntrs_gen_cfg);
    fpmini_prfm_cntrs_gen_cfg->msk_mc_dec_err = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG, MSK_MC_DEC_ERR, reg_fpmini_prfm_cntrs_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_debug_dbgsel_set(uint8_t vs)
{
    uint32_t reg_fpmini_debug_dbgsel = 0;

#ifdef VALIDATE_PARMS
    if ((vs >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmini_debug_dbgsel = RU_FIELD_SET(0, FPM_FPMINI, FPMINI_DEBUG_DBGSEL, VS, reg_fpmini_debug_dbgsel, vs);

    RU_REG_WRITE(0, FPM_FPMINI, FPMINI_DEBUG_DBGSEL, reg_fpmini_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_debug_dbgsel_get(uint8_t *vs)
{
    uint32_t reg_fpmini_debug_dbgsel;

#ifdef VALIDATE_PARMS
    if (!vs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_DEBUG_DBGSEL, reg_fpmini_debug_dbgsel);

    *vs = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_DEBUG_DBGSEL, VS, reg_fpmini_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmini_debug_dbgbus_get(uint32_t *vb)
{
    uint32_t reg_fpmini_debug_dbgbus;

#ifdef VALIDATE_PARMS
    if (!vb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMINI_DEBUG_DBGBUS, reg_fpmini_debug_dbgbus);

    *vb = RU_FIELD_GET(0, FPM_FPMINI, FPMINI_DEBUG_DBGBUS, VB, reg_fpmini_debug_dbgbus);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_set(bdmf_boolean init)
{
    uint32_t reg_fpmcast_cfg0_mc_init = 0;

#ifdef VALIDATE_PARMS
    if ((init >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmcast_cfg0_mc_init = RU_FIELD_SET(0, FPM_FPMINI, FPMCAST_CFG0_MC_INIT, INIT, reg_fpmcast_cfg0_mc_init, init);

    RU_REG_WRITE(0, FPM_FPMINI, FPMCAST_CFG0_MC_INIT, reg_fpmcast_cfg0_mc_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_get(bdmf_boolean *init)
{
    uint32_t reg_fpmcast_cfg0_mc_init;

#ifdef VALIDATE_PARMS
    if (!init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_CFG0_MC_INIT, reg_fpmcast_cfg0_mc_init);

    *init = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_CFG0_MC_INIT, INIT, reg_fpmcast_cfg0_mc_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_set(bdmf_boolean en)
{
    uint32_t reg_fpmcast_cfg0_free_bp_mc = 0;

#ifdef VALIDATE_PARMS
    if ((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmcast_cfg0_free_bp_mc = RU_FIELD_SET(0, FPM_FPMINI, FPMCAST_CFG0_FREE_BP_MC, EN, reg_fpmcast_cfg0_free_bp_mc, en);

    RU_REG_WRITE(0, FPM_FPMINI, FPMCAST_CFG0_FREE_BP_MC, reg_fpmcast_cfg0_free_bp_mc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_get(bdmf_boolean *en)
{
    uint32_t reg_fpmcast_cfg0_free_bp_mc;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_CFG0_FREE_BP_MC, reg_fpmcast_cfg0_free_bp_mc);

    *en = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_CFG0_FREE_BP_MC, EN, reg_fpmcast_cfg0_free_bp_mc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err_get(uint32_t *val)
{
    uint32_t reg_fpmcast_prfm_cntrs_mc_inc_err;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_INC_ERR, reg_fpmcast_prfm_cntrs_mc_inc_err);

    *val = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_INC_ERR, VAL, reg_fpmcast_prfm_cntrs_mc_inc_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err_get(uint32_t *val)
{
    uint32_t reg_fpmcast_prfm_cntrs_mc_dec_err;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_DEC_ERR, reg_fpmcast_prfm_cntrs_mc_dec_err);

    *val = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_DEC_ERR, VAL, reg_fpmcast_prfm_cntrs_mc_dec_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_set(uint8_t vs)
{
    uint32_t reg_fpmcast_debug_dbgsel = 0;

#ifdef VALIDATE_PARMS
    if ((vs >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpmcast_debug_dbgsel = RU_FIELD_SET(0, FPM_FPMINI, FPMCAST_DEBUG_DBGSEL, VS, reg_fpmcast_debug_dbgsel, vs);

    RU_REG_WRITE(0, FPM_FPMINI, FPMCAST_DEBUG_DBGSEL, reg_fpmcast_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_get(uint8_t *vs)
{
    uint32_t reg_fpmcast_debug_dbgsel;

#ifdef VALIDATE_PARMS
    if (!vs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_DEBUG_DBGSEL, reg_fpmcast_debug_dbgsel);

    *vs = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_DEBUG_DBGSEL, VS, reg_fpmcast_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_fpm_fpmini_fpmcast_debug_dbgbus_get(uint32_t *vb)
{
    uint32_t reg_fpmcast_debug_dbgbus;

#ifdef VALIDATE_PARMS
    if (!vb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FPM_FPMINI, FPMCAST_DEBUG_DBGBUS, reg_fpmcast_debug_dbgbus);

    *vb = RU_FIELD_GET(0, FPM_FPMINI, FPMCAST_DEBUG_DBGBUS, VB, reg_fpmcast_debug_dbgbus);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_fpmini_lvl_2_mem_l2,
    bdmf_address_fpmini_lvl_1_regs_l1,
    bdmf_address_fpmini_lvl_0_reg_l0,
    bdmf_address_fpmini_cfg0_l2_init,
    bdmf_address_fpmini_cfg0_allc_fast_ack,
    bdmf_address_fpmini_prfm_cntrs_num_avail_tkns,
    bdmf_address_fpmini_prfm_cntrs_num_avail_tkns_low_wm,
    bdmf_address_fpmini_prfm_cntrs_free_err,
    bdmf_address_fpmini_prfm_cntrs_gen_cfg,
    bdmf_address_fpmini_debug_dbgsel,
    bdmf_address_fpmini_debug_dbgbus,
    bdmf_address_fpmcast_mc_mem_mc,
    bdmf_address_fpmcast_cfg0_mc_init,
    bdmf_address_fpmcast_cfg0_free_bp_mc,
    bdmf_address_fpmcast_prfm_cntrs_mc_inc_err,
    bdmf_address_fpmcast_prfm_cntrs_mc_dec_err,
    bdmf_address_fpmcast_debug_dbgsel,
    bdmf_address_fpmcast_debug_dbgbus,
}
bdmf_address;

static int ag_drv_fpm_fpmini_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_fpm_fpmini_fpmini_lvl_0_reg_l0:
        ag_err = ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmini_cfg0_l2_init:
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmini_cfg0_allc_fast_ack:
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm:
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg:
    {
        fpm_fpmini_fpmini_prfm_cntrs_gen_cfg fpmini_prfm_cntrs_gen_cfg = { .rd_clr = parm[1].value.unumber, .wrap = parm[2].value.unumber, .msk_free_err = parm[3].value.unumber, .msk_mc_inc_err = parm[4].value.unumber, .msk_mc_dec_err = parm[5].value.unumber};
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_set(&fpmini_prfm_cntrs_gen_cfg);
        break;
    }
    case cli_fpm_fpmini_fpmini_debug_dbgsel:
        ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgsel_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmcast_cfg0_mc_init:
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmcast_cfg0_free_bp_mc:
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_set(parm[1].value.unumber);
        break;
    case cli_fpm_fpmini_fpmcast_debug_dbgsel:
        ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_set(parm[1].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_fpm_fpmini_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_fpm_fpmini_fpmini_lvl_0_reg_l0:
    {
        uint32_t data;
        ag_err = ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_fpm_fpmini_fpmini_cfg0_l2_init:
    {
        bdmf_boolean init;
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_get(&init);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        break;
    }
    case cli_fpm_fpmini_fpmini_cfg0_allc_fast_ack:
    {
        bdmf_boolean en;
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_get(&en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns:
    {
        uint32_t val;
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm:
    {
        uint32_t val;
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_fpm_fpmini_fpmini_prfm_cntrs_free_err:
    {
        uint32_t val;
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_free_err_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg:
    {
        fpm_fpmini_fpmini_prfm_cntrs_gen_cfg fpmini_prfm_cntrs_gen_cfg;
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_get(&fpmini_prfm_cntrs_gen_cfg);
        bdmf_session_print(session, "rd_clr = %u = 0x%x\n", fpmini_prfm_cntrs_gen_cfg.rd_clr, fpmini_prfm_cntrs_gen_cfg.rd_clr);
        bdmf_session_print(session, "wrap = %u = 0x%x\n", fpmini_prfm_cntrs_gen_cfg.wrap, fpmini_prfm_cntrs_gen_cfg.wrap);
        bdmf_session_print(session, "msk_free_err = %u = 0x%x\n", fpmini_prfm_cntrs_gen_cfg.msk_free_err, fpmini_prfm_cntrs_gen_cfg.msk_free_err);
        bdmf_session_print(session, "msk_mc_inc_err = %u = 0x%x\n", fpmini_prfm_cntrs_gen_cfg.msk_mc_inc_err, fpmini_prfm_cntrs_gen_cfg.msk_mc_inc_err);
        bdmf_session_print(session, "msk_mc_dec_err = %u = 0x%x\n", fpmini_prfm_cntrs_gen_cfg.msk_mc_dec_err, fpmini_prfm_cntrs_gen_cfg.msk_mc_dec_err);
        break;
    }
    case cli_fpm_fpmini_fpmini_debug_dbgsel:
    {
        uint8_t vs;
        ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgsel_get(&vs);
        bdmf_session_print(session, "vs = %u = 0x%x\n", vs, vs);
        break;
    }
    case cli_fpm_fpmini_fpmini_debug_dbgbus:
    {
        uint32_t vb;
        ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgbus_get(&vb);
        bdmf_session_print(session, "vb = %u = 0x%x\n", vb, vb);
        break;
    }
    case cli_fpm_fpmini_fpmcast_cfg0_mc_init:
    {
        bdmf_boolean init;
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_get(&init);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        break;
    }
    case cli_fpm_fpmini_fpmcast_cfg0_free_bp_mc:
    {
        bdmf_boolean en;
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_get(&en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err:
    {
        uint32_t val;
        ag_err = ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err:
    {
        uint32_t val;
        ag_err = ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_fpm_fpmini_fpmcast_debug_dbgsel:
    {
        uint8_t vs;
        ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_get(&vs);
        bdmf_session_print(session, "vs = %u = 0x%x\n", vs, vs);
        break;
    }
    case cli_fpm_fpmini_fpmcast_debug_dbgbus:
    {
        uint32_t vb;
        ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgbus_get(&vb);
        bdmf_session_print(session, "vb = %u = 0x%x\n", vb, vb);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_fpm_fpmini_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_set( %u)\n",
            data);
        ag_err = ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_set(data);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_get( %u)\n",
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
        bdmf_boolean init = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_set( %u)\n",
            init);
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_set(init);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_get(&init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_get( %u)\n",
                init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_set( %u)\n",
            en);
        ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_set(en);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_get(&en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_cfg0_allc_fast_ack_get( %u)\n",
                en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en != gtmv(m, 1))
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
            ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_get( %u)\n",
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_set( %u)\n",
            val);
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_set(val);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm_get( %u)\n",
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
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
            ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_free_err_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_free_err_get( %u)\n",
                val);
        }
    }

    {
        fpm_fpmini_fpmini_prfm_cntrs_gen_cfg fpmini_prfm_cntrs_gen_cfg = {.rd_clr = gtmv(m, 1), .wrap = gtmv(m, 1), .msk_free_err = gtmv(m, 1), .msk_mc_inc_err = gtmv(m, 1), .msk_mc_dec_err = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_set( %u %u %u %u %u)\n",
            fpmini_prfm_cntrs_gen_cfg.rd_clr, fpmini_prfm_cntrs_gen_cfg.wrap, fpmini_prfm_cntrs_gen_cfg.msk_free_err, fpmini_prfm_cntrs_gen_cfg.msk_mc_inc_err, 
            fpmini_prfm_cntrs_gen_cfg.msk_mc_dec_err);
        ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_set(&fpmini_prfm_cntrs_gen_cfg);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_get(&fpmini_prfm_cntrs_gen_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg_get( %u %u %u %u %u)\n",
                fpmini_prfm_cntrs_gen_cfg.rd_clr, fpmini_prfm_cntrs_gen_cfg.wrap, fpmini_prfm_cntrs_gen_cfg.msk_free_err, fpmini_prfm_cntrs_gen_cfg.msk_mc_inc_err, 
                fpmini_prfm_cntrs_gen_cfg.msk_mc_dec_err);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpmini_prfm_cntrs_gen_cfg.rd_clr != gtmv(m, 1) || fpmini_prfm_cntrs_gen_cfg.wrap != gtmv(m, 1) || fpmini_prfm_cntrs_gen_cfg.msk_free_err != gtmv(m, 1) || fpmini_prfm_cntrs_gen_cfg.msk_mc_inc_err != gtmv(m, 1) || fpmini_prfm_cntrs_gen_cfg.msk_mc_dec_err != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t vs = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_debug_dbgsel_set( %u)\n",
            vs);
        ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgsel_set(vs);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgsel_get(&vs);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_debug_dbgsel_get( %u)\n",
                vs);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vs != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t vb = gtmv(m, 21);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmini_debug_dbgbus_get(&vb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmini_debug_dbgbus_get( %u)\n",
                vb);
        }
    }

    {
        bdmf_boolean init = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_set( %u)\n",
            init);
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_set(init);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_get(&init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_get( %u)\n",
                init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_set( %u)\n",
            en);
        ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_set(en);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_get(&en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_cfg0_free_bp_mc_get( %u)\n",
                en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en != gtmv(m, 1))
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
            ag_err = ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err_get( %u)\n",
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err_get( %u)\n",
                val);
        }
    }

    {
        uint8_t vs = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_set( %u)\n",
            vs);
        ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_set(vs);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_get(&vs);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_debug_dbgsel_get( %u)\n",
                vs);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vs != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t vb = gtmv(m, 21);
        if (!ag_err)
            ag_err = ag_drv_fpm_fpmini_fpmcast_debug_dbgbus_get(&vb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_fpm_fpmini_fpmcast_debug_dbgbus_get( %u)\n",
                vb);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_fpm_fpmini_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_fpmini_lvl_2_mem_l2: reg = &RU_REG(FPM_FPMINI, FPMINI_LVL_2_MEM_L2); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_lvl_1_regs_l1: reg = &RU_REG(FPM_FPMINI, FPMINI_LVL_1_REGS_L1); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_lvl_0_reg_l0: reg = &RU_REG(FPM_FPMINI, FPMINI_LVL_0_REG_L0); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_cfg0_l2_init: reg = &RU_REG(FPM_FPMINI, FPMINI_CFG0_L2_INIT); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_cfg0_allc_fast_ack: reg = &RU_REG(FPM_FPMINI, FPMINI_CFG0_ALLC_FAST_ACK); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_prfm_cntrs_num_avail_tkns: reg = &RU_REG(FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_prfm_cntrs_num_avail_tkns_low_wm: reg = &RU_REG(FPM_FPMINI, FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_prfm_cntrs_free_err: reg = &RU_REG(FPM_FPMINI, FPMINI_PRFM_CNTRS_FREE_ERR); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_prfm_cntrs_gen_cfg: reg = &RU_REG(FPM_FPMINI, FPMINI_PRFM_CNTRS_GEN_CFG); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_debug_dbgsel: reg = &RU_REG(FPM_FPMINI, FPMINI_DEBUG_DBGSEL); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmini_debug_dbgbus: reg = &RU_REG(FPM_FPMINI, FPMINI_DEBUG_DBGBUS); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_mc_mem_mc: reg = &RU_REG(FPM_FPMINI, FPMCAST_MC_MEM_MC); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_cfg0_mc_init: reg = &RU_REG(FPM_FPMINI, FPMCAST_CFG0_MC_INIT); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_cfg0_free_bp_mc: reg = &RU_REG(FPM_FPMINI, FPMCAST_CFG0_FREE_BP_MC); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_prfm_cntrs_mc_inc_err: reg = &RU_REG(FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_INC_ERR); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_prfm_cntrs_mc_dec_err: reg = &RU_REG(FPM_FPMINI, FPMCAST_PRFM_CNTRS_MC_DEC_ERR); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_debug_dbgsel: reg = &RU_REG(FPM_FPMINI, FPMCAST_DEBUG_DBGSEL); blk = &RU_BLK(FPM_FPMINI); break;
    case bdmf_address_fpmcast_debug_dbgbus: reg = &RU_REG(FPM_FPMINI, FPMCAST_DEBUG_DBGBUS); blk = &RU_BLK(FPM_FPMINI); break;
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

bdmfmon_handle_t ag_drv_fpm_fpmini_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "fpm_fpmini", "fpm_fpmini", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_fpmini_lvl_0_reg_l0[] = {
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmini_cfg0_l2_init[] = {
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmini_cfg0_allc_fast_ack[] = {
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmini_prfm_cntrs_num_avail_tkns_low_wm[] = {
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmini_prfm_cntrs_gen_cfg[] = {
            BDMFMON_MAKE_PARM("rd_clr", "rd_clr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("msk_free_err", "msk_free_err", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("msk_mc_inc_err", "msk_mc_inc_err", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("msk_mc_dec_err", "msk_mc_dec_err", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmini_debug_dbgsel[] = {
            BDMFMON_MAKE_PARM("vs", "vs", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmcast_cfg0_mc_init[] = {
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmcast_cfg0_free_bp_mc[] = {
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpmcast_debug_dbgsel[] = {
            BDMFMON_MAKE_PARM("vs", "vs", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "fpmini_lvl_0_reg_l0", .val = cli_fpm_fpmini_fpmini_lvl_0_reg_l0, .parms = set_fpmini_lvl_0_reg_l0 },
            { .name = "fpmini_cfg0_l2_init", .val = cli_fpm_fpmini_fpmini_cfg0_l2_init, .parms = set_fpmini_cfg0_l2_init },
            { .name = "fpmini_cfg0_allc_fast_ack", .val = cli_fpm_fpmini_fpmini_cfg0_allc_fast_ack, .parms = set_fpmini_cfg0_allc_fast_ack },
            { .name = "fpmini_prfm_cntrs_num_avail_tkns_low_wm", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm, .parms = set_fpmini_prfm_cntrs_num_avail_tkns_low_wm },
            { .name = "fpmini_prfm_cntrs_gen_cfg", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg, .parms = set_fpmini_prfm_cntrs_gen_cfg },
            { .name = "fpmini_debug_dbgsel", .val = cli_fpm_fpmini_fpmini_debug_dbgsel, .parms = set_fpmini_debug_dbgsel },
            { .name = "fpmcast_cfg0_mc_init", .val = cli_fpm_fpmini_fpmcast_cfg0_mc_init, .parms = set_fpmcast_cfg0_mc_init },
            { .name = "fpmcast_cfg0_free_bp_mc", .val = cli_fpm_fpmini_fpmcast_cfg0_free_bp_mc, .parms = set_fpmcast_cfg0_free_bp_mc },
            { .name = "fpmcast_debug_dbgsel", .val = cli_fpm_fpmini_fpmcast_debug_dbgsel, .parms = set_fpmcast_debug_dbgsel },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_fpm_fpmini_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "fpmini_lvl_0_reg_l0", .val = cli_fpm_fpmini_fpmini_lvl_0_reg_l0, .parms = get_default },
            { .name = "fpmini_cfg0_l2_init", .val = cli_fpm_fpmini_fpmini_cfg0_l2_init, .parms = get_default },
            { .name = "fpmini_cfg0_allc_fast_ack", .val = cli_fpm_fpmini_fpmini_cfg0_allc_fast_ack, .parms = get_default },
            { .name = "fpmini_prfm_cntrs_num_avail_tkns", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns, .parms = get_default },
            { .name = "fpmini_prfm_cntrs_num_avail_tkns_low_wm", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_num_avail_tkns_low_wm, .parms = get_default },
            { .name = "fpmini_prfm_cntrs_free_err", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_free_err, .parms = get_default },
            { .name = "fpmini_prfm_cntrs_gen_cfg", .val = cli_fpm_fpmini_fpmini_prfm_cntrs_gen_cfg, .parms = get_default },
            { .name = "fpmini_debug_dbgsel", .val = cli_fpm_fpmini_fpmini_debug_dbgsel, .parms = get_default },
            { .name = "fpmini_debug_dbgbus", .val = cli_fpm_fpmini_fpmini_debug_dbgbus, .parms = get_default },
            { .name = "fpmcast_cfg0_mc_init", .val = cli_fpm_fpmini_fpmcast_cfg0_mc_init, .parms = get_default },
            { .name = "fpmcast_cfg0_free_bp_mc", .val = cli_fpm_fpmini_fpmcast_cfg0_free_bp_mc, .parms = get_default },
            { .name = "fpmcast_prfm_cntrs_mc_inc_err", .val = cli_fpm_fpmini_fpmcast_prfm_cntrs_mc_inc_err, .parms = get_default },
            { .name = "fpmcast_prfm_cntrs_mc_dec_err", .val = cli_fpm_fpmini_fpmcast_prfm_cntrs_mc_dec_err, .parms = get_default },
            { .name = "fpmcast_debug_dbgsel", .val = cli_fpm_fpmini_fpmcast_debug_dbgsel, .parms = get_default },
            { .name = "fpmcast_debug_dbgbus", .val = cli_fpm_fpmini_fpmcast_debug_dbgbus, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_fpm_fpmini_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_fpm_fpmini_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "FPMINI_LVL_2_MEM_L2", .val = bdmf_address_fpmini_lvl_2_mem_l2 },
            { .name = "FPMINI_LVL_1_REGS_L1", .val = bdmf_address_fpmini_lvl_1_regs_l1 },
            { .name = "FPMINI_LVL_0_REG_L0", .val = bdmf_address_fpmini_lvl_0_reg_l0 },
            { .name = "FPMINI_CFG0_L2_INIT", .val = bdmf_address_fpmini_cfg0_l2_init },
            { .name = "FPMINI_CFG0_ALLC_FAST_ACK", .val = bdmf_address_fpmini_cfg0_allc_fast_ack },
            { .name = "FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS", .val = bdmf_address_fpmini_prfm_cntrs_num_avail_tkns },
            { .name = "FPMINI_PRFM_CNTRS_NUM_AVAIL_TKNS_LOW_WM", .val = bdmf_address_fpmini_prfm_cntrs_num_avail_tkns_low_wm },
            { .name = "FPMINI_PRFM_CNTRS_FREE_ERR", .val = bdmf_address_fpmini_prfm_cntrs_free_err },
            { .name = "FPMINI_PRFM_CNTRS_GEN_CFG", .val = bdmf_address_fpmini_prfm_cntrs_gen_cfg },
            { .name = "FPMINI_DEBUG_DBGSEL", .val = bdmf_address_fpmini_debug_dbgsel },
            { .name = "FPMINI_DEBUG_DBGBUS", .val = bdmf_address_fpmini_debug_dbgbus },
            { .name = "FPMCAST_MC_MEM_MC", .val = bdmf_address_fpmcast_mc_mem_mc },
            { .name = "FPMCAST_CFG0_MC_INIT", .val = bdmf_address_fpmcast_cfg0_mc_init },
            { .name = "FPMCAST_CFG0_FREE_BP_MC", .val = bdmf_address_fpmcast_cfg0_free_bp_mc },
            { .name = "FPMCAST_PRFM_CNTRS_MC_INC_ERR", .val = bdmf_address_fpmcast_prfm_cntrs_mc_inc_err },
            { .name = "FPMCAST_PRFM_CNTRS_MC_DEC_ERR", .val = bdmf_address_fpmcast_prfm_cntrs_mc_dec_err },
            { .name = "FPMCAST_DEBUG_DBGSEL", .val = bdmf_address_fpmcast_debug_dbgsel },
            { .name = "FPMCAST_DEBUG_DBGBUS", .val = bdmf_address_fpmcast_debug_dbgbus },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_fpm_fpmini_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
