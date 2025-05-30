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
#include "xrdp_drv_cnpl_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_cnpl_counter_cfg_set(uint32_t cnt_loc_profile, const cnpl_counter_cfg *counter_cfg)
{
    uint32_t reg_counters_configurations_cn_loc_prof = 0;

#ifdef VALIDATE_PARMS
    if(!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnt_loc_profile >= 16) ||
       (counter_cfg->cn_double >= _1BITS_MAX_VAL_) ||
       (counter_cfg->cn0_byts >= _2BITS_MAX_VAL_) ||
       (counter_cfg->ba >= _12BITS_MAX_VAL_) ||
       (counter_cfg->wrap >= _1BITS_MAX_VAL_) ||
       (counter_cfg->clr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, DOUBLLE, reg_counters_configurations_cn_loc_prof, counter_cfg->cn_double);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CN0_BYTS, reg_counters_configurations_cn_loc_prof, counter_cfg->cn0_byts);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, BA, reg_counters_configurations_cn_loc_prof, counter_cfg->ba);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, WRAP, reg_counters_configurations_cn_loc_prof, counter_cfg->wrap);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CLR, reg_counters_configurations_cn_loc_prof, counter_cfg->clr);

    RU_REG_RAM_WRITE(0, cnt_loc_profile, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, reg_counters_configurations_cn_loc_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_counter_cfg_get(uint32_t cnt_loc_profile, cnpl_counter_cfg *counter_cfg)
{
    uint32_t reg_counters_configurations_cn_loc_prof;

#ifdef VALIDATE_PARMS
    if (!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnt_loc_profile >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, cnt_loc_profile, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, reg_counters_configurations_cn_loc_prof);

    counter_cfg->cn_double = RU_FIELD_GET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, DOUBLLE, reg_counters_configurations_cn_loc_prof);
    counter_cfg->cn0_byts = RU_FIELD_GET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CN0_BYTS, reg_counters_configurations_cn_loc_prof);
    counter_cfg->ba = RU_FIELD_GET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, BA, reg_counters_configurations_cn_loc_prof);
    counter_cfg->wrap = RU_FIELD_GET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, WRAP, reg_counters_configurations_cn_loc_prof);
    counter_cfg->clr = RU_FIELD_GET(0, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CLR, reg_counters_configurations_cn_loc_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policer_cfg_set(uint32_t conf_idx, const cnpl_policer_cfg *policer_cfg)
{
    uint32_t reg_policers_configurations_pl_loc_prof0 = 0;
    uint32_t reg_policers_configurations_pl_loc_prof1 = 0;

#ifdef VALIDATE_PARMS
    if(!policer_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((conf_idx >= 4) ||
       (policer_cfg->bk_ba >= _12BITS_MAX_VAL_) ||
       (policer_cfg->pa_ba >= _12BITS_MAX_VAL_) ||
       (policer_cfg->pl_double >= _1BITS_MAX_VAL_) ||
       (policer_cfg->fc >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, BK_BA, reg_policers_configurations_pl_loc_prof0, policer_cfg->bk_ba);
    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, PA_BA, reg_policers_configurations_pl_loc_prof0, policer_cfg->pa_ba);
    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, DOUBLLE, reg_policers_configurations_pl_loc_prof0, policer_cfg->pl_double);
    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, FC, reg_policers_configurations_pl_loc_prof0, policer_cfg->fc);
    reg_policers_configurations_pl_loc_prof1 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_ST, reg_policers_configurations_pl_loc_prof1, policer_cfg->pl_st);
    reg_policers_configurations_pl_loc_prof1 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_END, reg_policers_configurations_pl_loc_prof1, policer_cfg->pl_end);
    reg_policers_configurations_pl_loc_prof1 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, N, reg_policers_configurations_pl_loc_prof1, policer_cfg->n);

    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, reg_policers_configurations_pl_loc_prof0);
    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, reg_policers_configurations_pl_loc_prof1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policer_cfg_get(uint32_t conf_idx, cnpl_policer_cfg *policer_cfg)
{
    uint32_t reg_policers_configurations_pl_loc_prof0;
    uint32_t reg_policers_configurations_pl_loc_prof1;

#ifdef VALIDATE_PARMS
    if (!policer_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((conf_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, reg_policers_configurations_pl_loc_prof0);
    RU_REG_RAM_READ(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, reg_policers_configurations_pl_loc_prof1);

    policer_cfg->bk_ba = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, BK_BA, reg_policers_configurations_pl_loc_prof0);
    policer_cfg->pa_ba = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, PA_BA, reg_policers_configurations_pl_loc_prof0);
    policer_cfg->pl_double = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, DOUBLLE, reg_policers_configurations_pl_loc_prof0);
    policer_cfg->fc = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, FC, reg_policers_configurations_pl_loc_prof0);
    policer_cfg->pl_st = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_ST, reg_policers_configurations_pl_loc_prof1);
    policer_cfg->pl_end = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_END, reg_policers_configurations_pl_loc_prof1);
    policer_cfg->n = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, N, reg_policers_configurations_pl_loc_prof1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_stat_get(cnpl_sw_stat *sw_stat)
{
    uint32_t reg_sw_if_sw_stat;

#ifdef VALIDATE_PARMS
    if (!sw_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, SW_IF_SW_STAT, reg_sw_if_sw_stat);

    sw_stat->cn_rd_st = RU_FIELD_GET(0, CNPL, SW_IF_SW_STAT, CN_RD_ST, reg_sw_if_sw_stat);
    sw_stat->pl_plc_st = RU_FIELD_GET(0, CNPL, SW_IF_SW_STAT, PL_PLC_ST, reg_sw_if_sw_stat);
    sw_stat->pl_rd_st = RU_FIELD_GET(0, CNPL, SW_IF_SW_STAT, PL_RD_ST, reg_sw_if_sw_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_memory_data_set(uint32_t mem_idx, uint32_t data)
{
    uint32_t reg_memory_data = 0;

#ifdef VALIDATE_PARMS
    if ((mem_idx >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_memory_data = RU_FIELD_SET(0, CNPL, MEMORY_DATA, DATA, reg_memory_data, data);

    RU_REG_RAM_WRITE(0, mem_idx, CNPL, MEMORY_DATA, reg_memory_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_memory_data_get(uint32_t mem_idx, uint32_t *data)
{
    uint32_t reg_memory_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((mem_idx >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, mem_idx, CNPL, MEMORY_DATA, reg_memory_data);

    *data = RU_FIELD_GET(0, CNPL, MEMORY_DATA, DATA, reg_memory_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_set(uint32_t conf_idx, uint32_t vec)
{
    uint32_t reg_policers_configurations_pl_calc_type = 0;

#ifdef VALIDATE_PARMS
    if ((conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_calc_type = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, VEC, reg_policers_configurations_pl_calc_type, vec);

    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, reg_policers_configurations_pl_calc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_get(uint32_t conf_idx, uint32_t *vec)
{
    uint32_t reg_policers_configurations_pl_calc_type;

#ifdef VALIDATE_PARMS
    if (!vec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, reg_policers_configurations_pl_calc_type);

    *vec = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, VEC, reg_policers_configurations_pl_calc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_set(uint32_t conf_idx, uint16_t prf0, uint16_t prf1)
{
    uint32_t reg_policers_configurations_pl_size_prof = 0;

#ifdef VALIDATE_PARMS
    if ((conf_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_size_prof = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF0, reg_policers_configurations_pl_size_prof, prf0);
    reg_policers_configurations_pl_size_prof = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF1, reg_policers_configurations_pl_size_prof, prf1);

    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, reg_policers_configurations_pl_size_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_get(uint32_t conf_idx, uint16_t *prf0, uint16_t *prf1)
{
    uint32_t reg_policers_configurations_pl_size_prof;

#ifdef VALIDATE_PARMS
    if (!prf0 || !prf1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((conf_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, reg_policers_configurations_pl_size_prof);

    *prf0 = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF0, reg_policers_configurations_pl_size_prof);
    *prf1 = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF1, reg_policers_configurations_pl_size_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_set(bdmf_boolean en, uint16_t mtu)
{
    uint32_t reg_policers_configurations_per_up = 0;

#ifdef VALIDATE_PARMS
    if ((en >= _1BITS_MAX_VAL_) ||
       (mtu >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_per_up = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up, en);
    reg_policers_configurations_per_up = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, MTU, reg_policers_configurations_per_up, mtu);

    RU_REG_WRITE(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_get(bdmf_boolean *en, uint16_t *mtu)
{
    uint32_t reg_policers_configurations_per_up;

#ifdef VALIDATE_PARMS
    if (!en || !mtu)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    *en = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up);
    *mtu = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, MTU, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_set(uint32_t val)
{
    uint32_t reg_sw_if_sw_cmd = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_sw_if_sw_cmd = RU_FIELD_SET(0, CNPL, SW_IF_SW_CMD, VAL, reg_sw_if_sw_cmd, val);

    RU_REG_WRITE(0, CNPL, SW_IF_SW_CMD, reg_sw_if_sw_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_get(uint32_t *val)
{
    uint32_t reg_sw_if_sw_cmd;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, SW_IF_SW_CMD, reg_sw_if_sw_cmd);

    *val = RU_FIELD_GET(0, CNPL, SW_IF_SW_CMD, VAL, reg_sw_if_sw_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rslt_get(uint8_t *col)
{
    uint32_t reg_sw_if_sw_pl_rslt;

#ifdef VALIDATE_PARMS
    if (!col)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, SW_IF_SW_PL_RSLT, reg_sw_if_sw_pl_rslt);

    *col = RU_FIELD_GET(0, CNPL, SW_IF_SW_PL_RSLT, COL, reg_sw_if_sw_pl_rslt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rd_get(uint32_t bucket, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_pl_rd;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bucket >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, bucket, CNPL, SW_IF_SW_PL_RD, reg_sw_if_sw_pl_rd);

    *rd = RU_FIELD_GET(0, CNPL, SW_IF_SW_PL_RD, RD, reg_sw_if_sw_pl_rd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cnt_rd_get(uint32_t rd_idx, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_cnt_rd;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rd_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rd_idx, CNPL, SW_IF_SW_CNT_RD, reg_sw_if_sw_cnt_rd);

    *rd = RU_FIELD_GET(0, CNPL, SW_IF_SW_CNT_RD, RD, reg_sw_if_sw_cnt_rd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_set(uint8_t sw_prio, uint8_t mem_addr_bit_sel)
{
    uint32_t reg_misc_arb_prm = 0;

#ifdef VALIDATE_PARMS
    if ((sw_prio >= _2BITS_MAX_VAL_) ||
       (mem_addr_bit_sel >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_arb_prm = RU_FIELD_SET(0, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm, sw_prio);
    reg_misc_arb_prm = RU_FIELD_SET(0, CNPL, MISC_ARB_PRM, MEM_ADDR_BIT_SEL, reg_misc_arb_prm, mem_addr_bit_sel);

    RU_REG_WRITE(0, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_get(uint8_t *sw_prio, uint8_t *mem_addr_bit_sel)
{
    uint32_t reg_misc_arb_prm;

#ifdef VALIDATE_PARMS
    if (!sw_prio || !mem_addr_bit_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    *sw_prio = RU_FIELD_GET(0, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm);
    *mem_addr_bit_sel = RU_FIELD_GET(0, CNPL, MISC_ARB_PRM, MEM_ADDR_BIT_SEL, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_col_awr_en_set(bdmf_boolean en)
{
    uint32_t reg_misc_col_awr_en = 0;

#ifdef VALIDATE_PARMS
    if ((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_col_awr_en = RU_FIELD_SET(0, CNPL, MISC_COL_AWR_EN, EN, reg_misc_col_awr_en, en);

    RU_REG_WRITE(0, CNPL, MISC_COL_AWR_EN, reg_misc_col_awr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_col_awr_en_get(bdmf_boolean *en)
{
    uint32_t reg_misc_col_awr_en;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, MISC_COL_AWR_EN, reg_misc_col_awr_en);

    *en = RU_FIELD_GET(0, CNPL, MISC_COL_AWR_EN, EN, reg_misc_col_awr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_mem_init_set(bdmf_boolean init0, bdmf_boolean init1)
{
    uint32_t reg_misc_mem_init = 0;

#ifdef VALIDATE_PARMS
    if ((init0 >= _1BITS_MAX_VAL_) ||
       (init1 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_mem_init = RU_FIELD_SET(0, CNPL, MISC_MEM_INIT, INIT0, reg_misc_mem_init, init0);
    reg_misc_mem_init = RU_FIELD_SET(0, CNPL, MISC_MEM_INIT, INIT1, reg_misc_mem_init, init1);

    RU_REG_WRITE(0, CNPL, MISC_MEM_INIT, reg_misc_mem_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_mem_init_get(bdmf_boolean *init0, bdmf_boolean *init1)
{
    uint32_t reg_misc_mem_init;

#ifdef VALIDATE_PARMS
    if (!init0 || !init1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, MISC_MEM_INIT, reg_misc_mem_init);

    *init0 = RU_FIELD_GET(0, CNPL, MISC_MEM_INIT, INIT0, reg_misc_mem_init);
    *init1 = RU_FIELD_GET(0, CNPL, MISC_MEM_INIT, INIT1, reg_misc_mem_init);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_memory_data,
    bdmf_address_counters_configurations_cn_loc_prof,
    bdmf_address_policers_configurations_pl_loc_prof0,
    bdmf_address_policers_configurations_pl_loc_prof1,
    bdmf_address_policers_configurations_pl_calc_type,
    bdmf_address_policers_configurations_pl_size_prof,
    bdmf_address_policers_configurations_per_up,
    bdmf_address_sw_if_sw_cmd,
    bdmf_address_sw_if_sw_stat,
    bdmf_address_sw_if_sw_pl_rslt,
    bdmf_address_sw_if_sw_pl_rd,
    bdmf_address_sw_if_sw_cnt_rd,
    bdmf_address_misc_arb_prm,
    bdmf_address_misc_col_awr_en,
    bdmf_address_misc_mem_init,
    bdmf_address_pm_counters_eng_cmds,
    bdmf_address_pm_counters_cmd_wait,
    bdmf_address_pm_counters_tot_cyc,
    bdmf_address_pm_counters_gnt_cyc,
    bdmf_address_pm_counters_arb_cyc,
    bdmf_address_pm_counters_pl_up_err,
    bdmf_address_pm_counters_gen_cfg,
    bdmf_address_debug_dbgsel,
    bdmf_address_debug_dbgbus,
    bdmf_address_debug_req_vec,
    bdmf_address_debug_pol_up_st,
}
bdmf_address;

static int ag_drv_cnpl_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_cnpl_counter_cfg:
    {
        cnpl_counter_cfg counter_cfg = { .cn_double = parm[2].value.unumber, .cn0_byts = parm[3].value.unumber, .ba = parm[4].value.unumber, .wrap = parm[5].value.unumber, .clr = parm[6].value.unumber};
        ag_err = ag_drv_cnpl_counter_cfg_set(parm[1].value.unumber, &counter_cfg);
        break;
    }
    case cli_cnpl_policer_cfg:
    {
        cnpl_policer_cfg policer_cfg = { .bk_ba = parm[2].value.unumber, .pa_ba = parm[3].value.unumber, .pl_double = parm[4].value.unumber, .fc = parm[5].value.unumber, .pl_st = parm[6].value.unumber, .pl_end = parm[7].value.unumber, .n = parm[8].value.unumber};
        ag_err = ag_drv_cnpl_policer_cfg_set(parm[1].value.unumber, &policer_cfg);
        break;
    }
    case cli_cnpl_memory_data:
        ag_err = ag_drv_cnpl_memory_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_calc_type:
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_size_prof:
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_policers_configurations_per_up:
        ag_err = ag_drv_cnpl_policers_configurations_per_up_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_cmd:
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_set(parm[1].value.unumber);
        break;
    case cli_cnpl_misc_arb_prm:
        ag_err = ag_drv_cnpl_misc_arb_prm_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_misc_col_awr_en:
        ag_err = ag_drv_cnpl_misc_col_awr_en_set(parm[1].value.unumber);
        break;
    case cli_cnpl_misc_mem_init:
        ag_err = ag_drv_cnpl_misc_mem_init_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_cnpl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_cnpl_counter_cfg:
    {
        cnpl_counter_cfg counter_cfg;
        ag_err = ag_drv_cnpl_counter_cfg_get(parm[1].value.unumber, &counter_cfg);
        bdmf_session_print(session, "cn_double = %u = 0x%x\n", counter_cfg.cn_double, counter_cfg.cn_double);
        bdmf_session_print(session, "cn0_byts = %u = 0x%x\n", counter_cfg.cn0_byts, counter_cfg.cn0_byts);
        bdmf_session_print(session, "ba = %u = 0x%x\n", counter_cfg.ba, counter_cfg.ba);
        bdmf_session_print(session, "wrap = %u = 0x%x\n", counter_cfg.wrap, counter_cfg.wrap);
        bdmf_session_print(session, "clr = %u = 0x%x\n", counter_cfg.clr, counter_cfg.clr);
        break;
    }
    case cli_cnpl_policer_cfg:
    {
        cnpl_policer_cfg policer_cfg;
        ag_err = ag_drv_cnpl_policer_cfg_get(parm[1].value.unumber, &policer_cfg);
        bdmf_session_print(session, "bk_ba = %u = 0x%x\n", policer_cfg.bk_ba, policer_cfg.bk_ba);
        bdmf_session_print(session, "pa_ba = %u = 0x%x\n", policer_cfg.pa_ba, policer_cfg.pa_ba);
        bdmf_session_print(session, "pl_double = %u = 0x%x\n", policer_cfg.pl_double, policer_cfg.pl_double);
        bdmf_session_print(session, "fc = %u = 0x%x\n", policer_cfg.fc, policer_cfg.fc);
        bdmf_session_print(session, "pl_st = %u = 0x%x\n", policer_cfg.pl_st, policer_cfg.pl_st);
        bdmf_session_print(session, "pl_end = %u = 0x%x\n", policer_cfg.pl_end, policer_cfg.pl_end);
        bdmf_session_print(session, "n = %u = 0x%x\n", policer_cfg.n, policer_cfg.n);
        break;
    }
    case cli_cnpl_sw_stat:
    {
        cnpl_sw_stat sw_stat;
        ag_err = ag_drv_cnpl_sw_stat_get(&sw_stat);
        bdmf_session_print(session, "cn_rd_st = %u = 0x%x\n", sw_stat.cn_rd_st, sw_stat.cn_rd_st);
        bdmf_session_print(session, "pl_plc_st = %u = 0x%x\n", sw_stat.pl_plc_st, sw_stat.pl_plc_st);
        bdmf_session_print(session, "pl_rd_st = %u = 0x%x\n", sw_stat.pl_rd_st, sw_stat.pl_rd_st);
        break;
    }
    case cli_cnpl_memory_data:
    {
        uint32_t data;
        ag_err = ag_drv_cnpl_memory_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_cnpl_policers_configurations_pl_calc_type:
    {
        uint32_t vec;
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(parm[1].value.unumber, &vec);
        bdmf_session_print(session, "vec = %u = 0x%x\n", vec, vec);
        break;
    }
    case cli_cnpl_policers_configurations_pl_size_prof:
    {
        uint16_t prf0;
        uint16_t prf1;
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(parm[1].value.unumber, &prf0, &prf1);
        bdmf_session_print(session, "prf0 = %u = 0x%x\n", prf0, prf0);
        bdmf_session_print(session, "prf1 = %u = 0x%x\n", prf1, prf1);
        break;
    }
    case cli_cnpl_policers_configurations_per_up:
    {
        bdmf_boolean en;
        uint16_t mtu;
        ag_err = ag_drv_cnpl_policers_configurations_per_up_get(&en, &mtu);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        bdmf_session_print(session, "mtu = %u = 0x%x\n", mtu, mtu);
        break;
    }
    case cli_cnpl_sw_if_sw_cmd:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rslt:
    {
        uint8_t col;
        ag_err = ag_drv_cnpl_sw_if_sw_pl_rslt_get(&col);
        bdmf_session_print(session, "col = %u = 0x%x\n", col, col);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rd:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_pl_rd_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_cnt_rd:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_cnt_rd_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_misc_arb_prm:
    {
        uint8_t sw_prio;
        uint8_t mem_addr_bit_sel;
        ag_err = ag_drv_cnpl_misc_arb_prm_get(&sw_prio, &mem_addr_bit_sel);
        bdmf_session_print(session, "sw_prio = %u = 0x%x\n", sw_prio, sw_prio);
        bdmf_session_print(session, "mem_addr_bit_sel = %u = 0x%x\n", mem_addr_bit_sel, mem_addr_bit_sel);
        break;
    }
    case cli_cnpl_misc_col_awr_en:
    {
        bdmf_boolean en;
        ag_err = ag_drv_cnpl_misc_col_awr_en_get(&en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_cnpl_misc_mem_init:
    {
        bdmf_boolean init0;
        bdmf_boolean init1;
        ag_err = ag_drv_cnpl_misc_mem_init_get(&init0, &init1);
        bdmf_session_print(session, "init0 = %u = 0x%x\n", init0, init0);
        bdmf_session_print(session, "init1 = %u = 0x%x\n", init1, init1);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_cnpl_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t cnt_loc_profile = gtmv(m, 4);
        cnpl_counter_cfg counter_cfg = {.cn_double = gtmv(m, 1), .cn0_byts = gtmv(m, 2), .ba = gtmv(m, 12), .wrap = gtmv(m, 1), .clr = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set( %u %u %u %u %u %u)\n", cnt_loc_profile,
            counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
            counter_cfg.clr);
        ag_err = ag_drv_cnpl_counter_cfg_set(cnt_loc_profile, &counter_cfg);
        if (!ag_err)
            ag_err = ag_drv_cnpl_counter_cfg_get(cnt_loc_profile, &counter_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_get( %u %u %u %u %u %u)\n", cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (counter_cfg.cn_double != gtmv(m, 1) || counter_cfg.cn0_byts != gtmv(m, 2) || counter_cfg.ba != gtmv(m, 12) || counter_cfg.wrap != gtmv(m, 1) || counter_cfg.clr != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t conf_idx = gtmv(m, 2);
        cnpl_policer_cfg policer_cfg = {.bk_ba = gtmv(m, 12), .pa_ba = gtmv(m, 12), .pl_double = gtmv(m, 1), .fc = gtmv(m, 1), .pl_st = gtmv(m, 8), .pl_end = gtmv(m, 8), .n = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_set( %u %u %u %u %u %u %u %u)\n", conf_idx,
            policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.fc, 
            policer_cfg.pl_st, policer_cfg.pl_end, policer_cfg.n);
        ag_err = ag_drv_cnpl_policer_cfg_set(conf_idx, &policer_cfg);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policer_cfg_get(conf_idx, &policer_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_get( %u %u %u %u %u %u %u %u)\n", conf_idx,
                policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.fc, 
                policer_cfg.pl_st, policer_cfg.pl_end, policer_cfg.n);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (policer_cfg.bk_ba != gtmv(m, 12) || policer_cfg.pa_ba != gtmv(m, 12) || policer_cfg.pl_double != gtmv(m, 1) || policer_cfg.fc != gtmv(m, 1) || policer_cfg.pl_st != gtmv(m, 8) || policer_cfg.pl_end != gtmv(m, 8) || policer_cfg.n != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        cnpl_sw_stat sw_stat = {.cn_rd_st = gtmv(m, 1), .pl_plc_st = gtmv(m, 1), .pl_rd_st = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_stat_get(&sw_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_stat_get( %u %u %u)\n",
                sw_stat.cn_rd_st, sw_stat.pl_plc_st, sw_stat.pl_rd_st);
        }
    }

    {
        uint32_t mem_idx = gtmv(m, 12);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_memory_data_set( %u %u)\n", mem_idx,
            data);
        ag_err = ag_drv_cnpl_memory_data_set(mem_idx, data);
        if (!ag_err)
            ag_err = ag_drv_cnpl_memory_data_get(mem_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_memory_data_get( %u %u)\n", mem_idx,
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
        uint32_t conf_idx = gtmv(m, 3);
        uint32_t vec = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set( %u %u)\n", conf_idx,
            vec);
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(conf_idx, vec);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(conf_idx, &vec);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_get( %u %u)\n", conf_idx,
                vec);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vec != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t conf_idx = gtmv(m, 2);
        uint16_t prf0 = gtmv(m, 16);
        uint16_t prf1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set( %u %u %u)\n", conf_idx,
            prf0, prf1);
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(conf_idx, prf0, prf1);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(conf_idx, &prf0, &prf1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_get( %u %u %u)\n", conf_idx,
                prf0, prf1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (prf0 != gtmv(m, 16) || prf1 != gtmv(m, 16))
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
        uint16_t mtu = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_set( %u %u)\n",
            en, mtu);
        ag_err = ag_drv_cnpl_policers_configurations_per_up_set(en, mtu);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_per_up_get(&en, &mtu);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_get( %u %u)\n",
                en, mtu);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en != gtmv(m, 1) || mtu != gtmv(m, 14))
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
        bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_set( %u)\n",
            val);
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_set(val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_cmd_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_get( %u)\n",
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
        uint8_t col = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_pl_rslt_get(&col);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rslt_get( %u)\n",
                col);
        }
    }

    {
        uint32_t bucket = gtmv(m, 1);
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_pl_rd_get(bucket, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rd_get( %u %u)\n", bucket,
                rd);
        }
    }

    {
        uint32_t rd_idx = gtmv(m, 3);
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_cnt_rd_get(rd_idx, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cnt_rd_get( %u %u)\n", rd_idx,
                rd);
        }
    }

    {
        uint8_t sw_prio = gtmv(m, 2);
        uint8_t mem_addr_bit_sel = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_set( %u %u)\n",
            sw_prio, mem_addr_bit_sel);
        ag_err = ag_drv_cnpl_misc_arb_prm_set(sw_prio, mem_addr_bit_sel);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_arb_prm_get(&sw_prio, &mem_addr_bit_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_get( %u %u)\n",
                sw_prio, mem_addr_bit_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sw_prio != gtmv(m, 2) || mem_addr_bit_sel != gtmv(m, 4))
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
        bdmf_session_print(session, "ag_drv_cnpl_misc_col_awr_en_set( %u)\n",
            en);
        ag_err = ag_drv_cnpl_misc_col_awr_en_set(en);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_col_awr_en_get(&en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_col_awr_en_get( %u)\n",
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
        bdmf_boolean init0 = gtmv(m, 1);
        bdmf_boolean init1 = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_cnpl_misc_mem_init_set( %u %u)\n",
            init0, init1);
        ag_err = ag_drv_cnpl_misc_mem_init_set(init0, init1);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_mem_init_get(&init0, &init1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_mem_init_get( %u %u)\n",
                init0, init1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init0 != gtmv(m, 1) || init1 != gtmv(m, 1))
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
static int ag_drv_cnpl_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case cli_cnpl_counter_cfg:
    {
        uint32_t max_cnt_loc_profile = 16;
        uint32_t cnt_loc_profile = gtmv(m, 4);
        cnpl_counter_cfg counter_cfg = {
            .cn_double = gtmv(m, 1), 
            .cn0_byts = gtmv(m, 2), 
            .ba = gtmv(m, 12), 
            .wrap = gtmv(m, 1), 
            .clr = gtmv(m, 1) };

        if ((start_idx >= max_cnt_loc_profile) || (stop_idx >= max_cnt_loc_profile))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_cnt_loc_profile);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (cnt_loc_profile = 0; cnt_loc_profile < max_cnt_loc_profile; cnt_loc_profile++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set( %u %u %u %u %u %u)\n", cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
            ag_err = ag_drv_cnpl_counter_cfg_set(cnt_loc_profile, &counter_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", cnt_loc_profile);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        counter_cfg.cn_double = gtmv(m, 1);
        counter_cfg.cn0_byts = gtmv(m, 2);
        counter_cfg.ba = gtmv(m, 12);
        counter_cfg.wrap = gtmv(m, 1);
        counter_cfg.clr = gtmv(m, 1);

        for (cnt_loc_profile = start_idx; cnt_loc_profile <= stop_idx; cnt_loc_profile++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set( %u %u %u %u %u %u)\n", cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
            ag_err = ag_drv_cnpl_counter_cfg_set(cnt_loc_profile, &counter_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", cnt_loc_profile);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (cnt_loc_profile = 0; cnt_loc_profile < max_cnt_loc_profile; cnt_loc_profile++)
        {
            if (cnt_loc_profile < start_idx || cnt_loc_profile > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_cnpl_counter_cfg_get(cnt_loc_profile, &counter_cfg);

            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_get( %u %u %u %u %u %u)\n", cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);

            if (counter_cfg.cn_double != gtmv(m, 1) || 
                counter_cfg.cn0_byts != gtmv(m, 2) || 
                counter_cfg.ba != gtmv(m, 12) || 
                counter_cfg.wrap != gtmv(m, 1) || 
                counter_cfg.clr != gtmv(m, 1) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", cnt_loc_profile);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of counter_cfg completed. Number of tested entries %u.\n", max_cnt_loc_profile);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policer_cfg:
    {
        uint32_t max_conf_idx = 4;
        uint32_t conf_idx = gtmv(m, 2);
        cnpl_policer_cfg policer_cfg = {
            .bk_ba = gtmv(m, 12), 
            .pa_ba = gtmv(m, 12), 
            .pl_double = gtmv(m, 1), 
            .fc = gtmv(m, 1), 
            .pl_st = gtmv(m, 8), 
            .pl_end = gtmv(m, 8), 
            .n = gtmv(m, 8) };

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_set( %u %u %u %u %u %u %u %u)\n", conf_idx,
                policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.fc, 
                policer_cfg.pl_st, policer_cfg.pl_end, policer_cfg.n);
            ag_err = ag_drv_cnpl_policer_cfg_set(conf_idx, &policer_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        policer_cfg.bk_ba = gtmv(m, 12);
        policer_cfg.pa_ba = gtmv(m, 12);
        policer_cfg.pl_double = gtmv(m, 1);
        policer_cfg.fc = gtmv(m, 1);
        policer_cfg.pl_st = gtmv(m, 8);
        policer_cfg.pl_end = gtmv(m, 8);
        policer_cfg.n = gtmv(m, 8);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_set( %u %u %u %u %u %u %u %u)\n", conf_idx,
                policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.fc, 
                policer_cfg.pl_st, policer_cfg.pl_end, policer_cfg.n);
            ag_err = ag_drv_cnpl_policer_cfg_set(conf_idx, &policer_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            if (conf_idx < start_idx || conf_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_cnpl_policer_cfg_get(conf_idx, &policer_cfg);

            bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_get( %u %u %u %u %u %u %u %u)\n", conf_idx,
                policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.fc, 
                policer_cfg.pl_st, policer_cfg.pl_end, policer_cfg.n);

            if (policer_cfg.bk_ba != gtmv(m, 12) || 
                policer_cfg.pa_ba != gtmv(m, 12) || 
                policer_cfg.pl_double != gtmv(m, 1) || 
                policer_cfg.fc != gtmv(m, 1) || 
                policer_cfg.pl_st != gtmv(m, 8) || 
                policer_cfg.pl_end != gtmv(m, 8) || 
                policer_cfg.n != gtmv(m, 8) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", conf_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of policer_cfg completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_memory_data:
    {
        uint32_t max_mem_idx = 5120;
        uint32_t mem_idx = gtmv(m, 12);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_mem_idx) || (stop_idx >= max_mem_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_mem_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (mem_idx = 0; mem_idx < max_mem_idx; mem_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_memory_data_set( %u %u)\n", mem_idx,
                data);
            ag_err = ag_drv_cnpl_memory_data_set(mem_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", mem_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (mem_idx = start_idx; mem_idx <= stop_idx; mem_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_memory_data_set( %u %u)\n", mem_idx,
                data);
            ag_err = ag_drv_cnpl_memory_data_set(mem_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", mem_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (mem_idx = 0; mem_idx < max_mem_idx; mem_idx++)
        {
            if (mem_idx < start_idx || mem_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_cnpl_memory_data_get(mem_idx, &data);

            bdmf_session_print(session, "ag_drv_cnpl_memory_data_get( %u %u)\n", mem_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", mem_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of memory_data completed. Number of tested entries %u.\n", max_mem_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_configurations_pl_calc_type:
    {
        uint32_t max_conf_idx = 8;
        uint32_t conf_idx = gtmv(m, 3);
        uint32_t vec = gtmv(m, 32);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set( %u %u)\n", conf_idx,
                vec);
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(conf_idx, vec);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        vec = gtmv(m, 32);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set( %u %u)\n", conf_idx,
                vec);
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(conf_idx, vec);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            if (conf_idx < start_idx || conf_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(conf_idx, &vec);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_get( %u %u)\n", conf_idx,
                vec);

            if (vec != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", conf_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of policers_configurations_pl_calc_type completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_configurations_pl_size_prof:
    {
        uint32_t max_conf_idx = 4;
        uint32_t conf_idx = gtmv(m, 2);
        uint16_t prf0 = gtmv(m, 16);
        uint16_t prf1 = gtmv(m, 16);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set( %u %u %u)\n", conf_idx,
                prf0, prf1);
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(conf_idx, prf0, prf1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        prf0 = gtmv(m, 16);
        prf1 = gtmv(m, 16);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set( %u %u %u)\n", conf_idx,
                prf0, prf1);
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(conf_idx, prf0, prf1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            if (conf_idx < start_idx || conf_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(conf_idx, &prf0, &prf1);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_get( %u %u %u)\n", conf_idx,
                prf0, prf1);

            if (prf0 != gtmv(m, 16) || prf1 != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", conf_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of policers_configurations_pl_size_prof completed. Number of tested entries %u.\n", max_conf_idx);
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
static int ag_drv_cnpl_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_memory_data: reg = &RU_REG(CNPL, MEMORY_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_counters_configurations_cn_loc_prof: reg = &RU_REG(CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof0: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof1: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_calc_type: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_size_prof: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_per_up: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PER_UP); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cmd: reg = &RU_REG(CNPL, SW_IF_SW_CMD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_stat: reg = &RU_REG(CNPL, SW_IF_SW_STAT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rslt: reg = &RU_REG(CNPL, SW_IF_SW_PL_RSLT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rd: reg = &RU_REG(CNPL, SW_IF_SW_PL_RD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cnt_rd: reg = &RU_REG(CNPL, SW_IF_SW_CNT_RD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_arb_prm: reg = &RU_REG(CNPL, MISC_ARB_PRM); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_col_awr_en: reg = &RU_REG(CNPL, MISC_COL_AWR_EN); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_mem_init: reg = &RU_REG(CNPL, MISC_MEM_INIT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_eng_cmds: reg = &RU_REG(CNPL, PM_COUNTERS_ENG_CMDS); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_cmd_wait: reg = &RU_REG(CNPL, PM_COUNTERS_CMD_WAIT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_tot_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_TOT_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_gnt_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_GNT_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_arb_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_ARB_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_pl_up_err: reg = &RU_REG(CNPL, PM_COUNTERS_PL_UP_ERR); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_gen_cfg: reg = &RU_REG(CNPL, PM_COUNTERS_GEN_CFG); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_dbgsel: reg = &RU_REG(CNPL, DEBUG_DBGSEL); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_dbgbus: reg = &RU_REG(CNPL, DEBUG_DBGBUS); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_req_vec: reg = &RU_REG(CNPL, DEBUG_REQ_VEC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_pol_up_st: reg = &RU_REG(CNPL, DEBUG_POL_UP_ST); blk = &RU_BLK(CNPL); break;
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

bdmfmon_handle_t ag_drv_cnpl_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "cnpl", "cnpl", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_counter_cfg[] = {
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cn_double", "cn_double", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cn0_byts", "cn0_byts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ba", "ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("clr", "clr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policer_cfg[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bk_ba", "bk_ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pa_ba", "pa_ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pl_double", "pl_double", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fc", "fc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pl_st", "pl_st", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pl_end", "pl_end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("n", "n", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_memory_data[] = {
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_calc_type[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vec", "vec", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_size_prof[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prf0", "prf0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prf1", "prf1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_per_up[] = {
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mtu", "mtu", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_cmd[] = {
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_arb_prm[] = {
            BDMFMON_MAKE_PARM("sw_prio", "sw_prio", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_addr_bit_sel", "mem_addr_bit_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_col_awr_en[] = {
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_mem_init[] = {
            BDMFMON_MAKE_PARM("init0", "init0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init1", "init1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = set_counter_cfg },
            { .name = "policer_cfg", .val = cli_cnpl_policer_cfg, .parms = set_policer_cfg },
            { .name = "memory_data", .val = cli_cnpl_memory_data, .parms = set_memory_data },
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = set_policers_configurations_pl_calc_type },
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = set_policers_configurations_pl_size_prof },
            { .name = "policers_configurations_per_up", .val = cli_cnpl_policers_configurations_per_up, .parms = set_policers_configurations_per_up },
            { .name = "sw_if_sw_cmd", .val = cli_cnpl_sw_if_sw_cmd, .parms = set_sw_if_sw_cmd },
            { .name = "misc_arb_prm", .val = cli_cnpl_misc_arb_prm, .parms = set_misc_arb_prm },
            { .name = "misc_col_awr_en", .val = cli_cnpl_misc_col_awr_en, .parms = set_misc_col_awr_en },
            { .name = "misc_mem_init", .val = cli_cnpl_misc_mem_init, .parms = set_misc_mem_init },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_cnpl_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counter_cfg[] = {
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policer_cfg[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_memory_data[] = {
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_calc_type[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_size_prof[] = {
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_sw_if_sw_pl_rd[] = {
            BDMFMON_MAKE_PARM("bucket", "bucket", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_sw_if_sw_cnt_rd[] = {
            BDMFMON_MAKE_PARM("rd_idx", "rd_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = get_counter_cfg },
            { .name = "policer_cfg", .val = cli_cnpl_policer_cfg, .parms = get_policer_cfg },
            { .name = "sw_stat", .val = cli_cnpl_sw_stat, .parms = get_default },
            { .name = "memory_data", .val = cli_cnpl_memory_data, .parms = get_memory_data },
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = get_policers_configurations_pl_calc_type },
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = get_policers_configurations_pl_size_prof },
            { .name = "policers_configurations_per_up", .val = cli_cnpl_policers_configurations_per_up, .parms = get_default },
            { .name = "sw_if_sw_cmd", .val = cli_cnpl_sw_if_sw_cmd, .parms = get_default },
            { .name = "sw_if_sw_pl_rslt", .val = cli_cnpl_sw_if_sw_pl_rslt, .parms = get_default },
            { .name = "sw_if_sw_pl_rd", .val = cli_cnpl_sw_if_sw_pl_rd, .parms = get_sw_if_sw_pl_rd },
            { .name = "sw_if_sw_cnt_rd", .val = cli_cnpl_sw_if_sw_cnt_rd, .parms = get_sw_if_sw_cnt_rd },
            { .name = "misc_arb_prm", .val = cli_cnpl_misc_arb_prm, .parms = get_default },
            { .name = "misc_col_awr_en", .val = cli_cnpl_misc_col_awr_en, .parms = get_default },
            { .name = "misc_mem_init", .val = cli_cnpl_misc_mem_init, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_cnpl_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_cnpl_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = ext_test_default},
            { .name = "policer_cfg", .val = cli_cnpl_policer_cfg, .parms = ext_test_default},
            { .name = "memory_data", .val = cli_cnpl_memory_data, .parms = ext_test_default},
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = ext_test_default},
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_cnpl_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "MEMORY_DATA", .val = bdmf_address_memory_data },
            { .name = "COUNTERS_CONFIGURATIONS_CN_LOC_PROF", .val = bdmf_address_counters_configurations_cn_loc_prof },
            { .name = "POLICERS_CONFIGURATIONS_PL_LOC_PROF0", .val = bdmf_address_policers_configurations_pl_loc_prof0 },
            { .name = "POLICERS_CONFIGURATIONS_PL_LOC_PROF1", .val = bdmf_address_policers_configurations_pl_loc_prof1 },
            { .name = "POLICERS_CONFIGURATIONS_PL_CALC_TYPE", .val = bdmf_address_policers_configurations_pl_calc_type },
            { .name = "POLICERS_CONFIGURATIONS_PL_SIZE_PROF", .val = bdmf_address_policers_configurations_pl_size_prof },
            { .name = "POLICERS_CONFIGURATIONS_PER_UP", .val = bdmf_address_policers_configurations_per_up },
            { .name = "SW_IF_SW_CMD", .val = bdmf_address_sw_if_sw_cmd },
            { .name = "SW_IF_SW_STAT", .val = bdmf_address_sw_if_sw_stat },
            { .name = "SW_IF_SW_PL_RSLT", .val = bdmf_address_sw_if_sw_pl_rslt },
            { .name = "SW_IF_SW_PL_RD", .val = bdmf_address_sw_if_sw_pl_rd },
            { .name = "SW_IF_SW_CNT_RD", .val = bdmf_address_sw_if_sw_cnt_rd },
            { .name = "MISC_ARB_PRM", .val = bdmf_address_misc_arb_prm },
            { .name = "MISC_COL_AWR_EN", .val = bdmf_address_misc_col_awr_en },
            { .name = "MISC_MEM_INIT", .val = bdmf_address_misc_mem_init },
            { .name = "PM_COUNTERS_ENG_CMDS", .val = bdmf_address_pm_counters_eng_cmds },
            { .name = "PM_COUNTERS_CMD_WAIT", .val = bdmf_address_pm_counters_cmd_wait },
            { .name = "PM_COUNTERS_TOT_CYC", .val = bdmf_address_pm_counters_tot_cyc },
            { .name = "PM_COUNTERS_GNT_CYC", .val = bdmf_address_pm_counters_gnt_cyc },
            { .name = "PM_COUNTERS_ARB_CYC", .val = bdmf_address_pm_counters_arb_cyc },
            { .name = "PM_COUNTERS_PL_UP_ERR", .val = bdmf_address_pm_counters_pl_up_err },
            { .name = "PM_COUNTERS_GEN_CFG", .val = bdmf_address_pm_counters_gen_cfg },
            { .name = "DEBUG_DBGSEL", .val = bdmf_address_debug_dbgsel },
            { .name = "DEBUG_DBGBUS", .val = bdmf_address_debug_dbgbus },
            { .name = "DEBUG_REQ_VEC", .val = bdmf_address_debug_req_vec },
            { .name = "DEBUG_POL_UP_ST", .val = bdmf_address_debug_pol_up_st },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_cnpl_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
