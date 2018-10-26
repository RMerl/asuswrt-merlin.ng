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
#include "xrdp_drv_cnpl_ag.h"

bdmf_error_t ag_drv_cnpl_counter_cfg_set(uint32_t cnt_loc_profile, const cnpl_counter_cfg *counter_cfg)
{
    uint32_t reg_counters_configurations_cn_loc_prof=0;

#ifdef VALIDATE_PARMS
    if(!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((cnt_loc_profile >= 16) ||
       (counter_cfg->cn_double >= _1BITS_MAX_VAL_) ||
       (counter_cfg->cn0_byts >= _2BITS_MAX_VAL_) ||
       (counter_cfg->ba >= _11BITS_MAX_VAL_) ||
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
    if(!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((cnt_loc_profile >= 16))
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
    uint32_t reg_policers_configurations_pl_loc_prof0=0;
    uint32_t reg_policers_configurations_pl_loc_prof1=0;

#ifdef VALIDATE_PARMS
    if(!policer_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((conf_idx >= 2) ||
       (policer_cfg->bk_ba >= _11BITS_MAX_VAL_) ||
       (policer_cfg->pa_ba >= _11BITS_MAX_VAL_) ||
       (policer_cfg->pl_double >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, BK_BA, reg_policers_configurations_pl_loc_prof0, policer_cfg->bk_ba);
    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, PA_BA, reg_policers_configurations_pl_loc_prof0, policer_cfg->pa_ba);
    reg_policers_configurations_pl_loc_prof0 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, DOUBLLE, reg_policers_configurations_pl_loc_prof0, policer_cfg->pl_double);
    reg_policers_configurations_pl_loc_prof1 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_ST, reg_policers_configurations_pl_loc_prof1, policer_cfg->pl_st);
    reg_policers_configurations_pl_loc_prof1 = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_END, reg_policers_configurations_pl_loc_prof1, policer_cfg->pl_end);

    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0, reg_policers_configurations_pl_loc_prof0);
    RU_REG_RAM_WRITE(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, reg_policers_configurations_pl_loc_prof1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policer_cfg_get(uint32_t conf_idx, cnpl_policer_cfg *policer_cfg)
{
    uint32_t reg_policers_configurations_pl_loc_prof0;
    uint32_t reg_policers_configurations_pl_loc_prof1;

#ifdef VALIDATE_PARMS
    if(!policer_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((conf_idx >= 2))
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
    policer_cfg->pl_st = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_ST, reg_policers_configurations_pl_loc_prof1);
    policer_cfg->pl_end = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1, PL_END, reg_policers_configurations_pl_loc_prof1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_stat_get(cnpl_sw_stat *sw_stat)
{
    uint32_t reg_sw_if_sw_stat;

#ifdef VALIDATE_PARMS
    if(!sw_stat)
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
    uint32_t reg_memory_data=0;

#ifdef VALIDATE_PARMS
    if((mem_idx >= 3584))
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((mem_idx >= 3584))
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
    uint32_t reg_policers_configurations_pl_calc_type=0;

#ifdef VALIDATE_PARMS
    if((conf_idx >= 3))
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
    if(!vec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((conf_idx >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, reg_policers_configurations_pl_calc_type);

    *vec = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, VEC, reg_policers_configurations_pl_calc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_set(uint8_t n, bdmf_boolean en)
{
    uint32_t reg_policers_configurations_per_up=0;

#ifdef VALIDATE_PARMS
    if((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_per_up = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, N, reg_policers_configurations_per_up, n);
    reg_policers_configurations_per_up = RU_FIELD_SET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up, en);

    RU_REG_WRITE(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_get(uint8_t *n, bdmf_boolean *en)
{
    uint32_t reg_policers_configurations_per_up;

#ifdef VALIDATE_PARMS
    if(!n || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    *n = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, N, reg_policers_configurations_per_up);
    *en = RU_FIELD_GET(0, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_set(uint8_t sw_prio)
{
    uint32_t reg_misc_arb_prm=0;

#ifdef VALIDATE_PARMS
    if((sw_prio >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_arb_prm = RU_FIELD_SET(0, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm, sw_prio);

    RU_REG_WRITE(0, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_get(uint8_t *sw_prio)
{
    uint32_t reg_misc_arb_prm;

#ifdef VALIDATE_PARMS
    if(!sw_prio)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    *sw_prio = RU_FIELD_GET(0, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_set(uint32_t val)
{
    uint32_t reg_sw_if_sw_cmd=0;

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
    if(!val)
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
    if(!col)
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
    if(!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bucket >= 2))
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
    if(!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rd_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rd_idx, CNPL, SW_IF_SW_CNT_RD, reg_sw_if_sw_cnt_rd);

    *rd = RU_FIELD_GET(0, CNPL, SW_IF_SW_CNT_RD, RD, reg_sw_if_sw_cnt_rd);

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
    bdmf_address_policers_configurations_per_up,
    bdmf_address_misc_arb_prm,
    bdmf_address_sw_if_sw_cmd,
    bdmf_address_sw_if_sw_stat,
    bdmf_address_sw_if_sw_pl_rslt,
    bdmf_address_sw_if_sw_pl_rd,
    bdmf_address_sw_if_sw_cnt_rd,
}
bdmf_address;

static int bcm_cnpl_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_cnpl_counter_cfg:
    {
        cnpl_counter_cfg counter_cfg = { .cn_double=parm[2].value.unumber, .cn0_byts=parm[3].value.unumber, .ba=parm[4].value.unumber, .wrap=parm[5].value.unumber, .clr=parm[6].value.unumber};
        err = ag_drv_cnpl_counter_cfg_set(parm[1].value.unumber, &counter_cfg);
        break;
    }
    case cli_cnpl_policer_cfg:
    {
        cnpl_policer_cfg policer_cfg = { .bk_ba=parm[2].value.unumber, .pa_ba=parm[3].value.unumber, .pl_double=parm[4].value.unumber, .pl_st=parm[5].value.unumber, .pl_end=parm[6].value.unumber};
        err = ag_drv_cnpl_policer_cfg_set(parm[1].value.unumber, &policer_cfg);
        break;
    }
    case cli_cnpl_memory_data:
        err = ag_drv_cnpl_memory_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_calc_type:
        err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_policers_configurations_per_up:
        err = ag_drv_cnpl_policers_configurations_per_up_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_misc_arb_prm:
        err = ag_drv_cnpl_misc_arb_prm_set(parm[1].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_cmd:
        err = ag_drv_cnpl_sw_if_sw_cmd_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_cnpl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_cnpl_counter_cfg:
    {
        cnpl_counter_cfg counter_cfg;
        err = ag_drv_cnpl_counter_cfg_get(parm[1].value.unumber, &counter_cfg);
        bdmf_session_print(session, "cn_double = %u (0x%x)\n", counter_cfg.cn_double, counter_cfg.cn_double);
        bdmf_session_print(session, "cn0_byts = %u (0x%x)\n", counter_cfg.cn0_byts, counter_cfg.cn0_byts);
        bdmf_session_print(session, "ba = %u (0x%x)\n", counter_cfg.ba, counter_cfg.ba);
        bdmf_session_print(session, "wrap = %u (0x%x)\n", counter_cfg.wrap, counter_cfg.wrap);
        bdmf_session_print(session, "clr = %u (0x%x)\n", counter_cfg.clr, counter_cfg.clr);
        break;
    }
    case cli_cnpl_policer_cfg:
    {
        cnpl_policer_cfg policer_cfg;
        err = ag_drv_cnpl_policer_cfg_get(parm[1].value.unumber, &policer_cfg);
        bdmf_session_print(session, "bk_ba = %u (0x%x)\n", policer_cfg.bk_ba, policer_cfg.bk_ba);
        bdmf_session_print(session, "pa_ba = %u (0x%x)\n", policer_cfg.pa_ba, policer_cfg.pa_ba);
        bdmf_session_print(session, "pl_double = %u (0x%x)\n", policer_cfg.pl_double, policer_cfg.pl_double);
        bdmf_session_print(session, "pl_st = %u (0x%x)\n", policer_cfg.pl_st, policer_cfg.pl_st);
        bdmf_session_print(session, "pl_end = %u (0x%x)\n", policer_cfg.pl_end, policer_cfg.pl_end);
        break;
    }
    case cli_cnpl_sw_stat:
    {
        cnpl_sw_stat sw_stat;
        err = ag_drv_cnpl_sw_stat_get(&sw_stat);
        bdmf_session_print(session, "cn_rd_st = %u (0x%x)\n", sw_stat.cn_rd_st, sw_stat.cn_rd_st);
        bdmf_session_print(session, "pl_plc_st = %u (0x%x)\n", sw_stat.pl_plc_st, sw_stat.pl_plc_st);
        bdmf_session_print(session, "pl_rd_st = %u (0x%x)\n", sw_stat.pl_rd_st, sw_stat.pl_rd_st);
        break;
    }
    case cli_cnpl_memory_data:
    {
        uint32_t data;
        err = ag_drv_cnpl_memory_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_cnpl_policers_configurations_pl_calc_type:
    {
        uint32_t vec;
        err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(parm[1].value.unumber, &vec);
        bdmf_session_print(session, "vec = %u (0x%x)\n", vec, vec);
        break;
    }
    case cli_cnpl_policers_configurations_per_up:
    {
        uint8_t n;
        bdmf_boolean en;
        err = ag_drv_cnpl_policers_configurations_per_up_get(&n, &en);
        bdmf_session_print(session, "n = %u (0x%x)\n", n, n);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_cnpl_misc_arb_prm:
    {
        uint8_t sw_prio;
        err = ag_drv_cnpl_misc_arb_prm_get(&sw_prio);
        bdmf_session_print(session, "sw_prio = %u (0x%x)\n", sw_prio, sw_prio);
        break;
    }
    case cli_cnpl_sw_if_sw_cmd:
    {
        uint32_t val;
        err = ag_drv_cnpl_sw_if_sw_cmd_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rslt:
    {
        uint8_t col;
        err = ag_drv_cnpl_sw_if_sw_pl_rslt_get(&col);
        bdmf_session_print(session, "col = %u (0x%x)\n", col, col);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rd:
    {
        uint32_t rd;
        err = ag_drv_cnpl_sw_if_sw_pl_rd_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u (0x%x)\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_cnt_rd:
    {
        uint32_t rd;
        err = ag_drv_cnpl_sw_if_sw_cnt_rd_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u (0x%x)\n", rd, rd);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_cnpl_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t cnt_loc_profile=gtmv(m, 4);
        cnpl_counter_cfg counter_cfg = {.cn_double=gtmv(m, 1), .cn0_byts=gtmv(m, 2), .ba=gtmv(m, 11), .wrap=gtmv(m, 1), .clr=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set( %u %u %u %u %u %u)\n", cnt_loc_profile, counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, counter_cfg.clr);
        if(!err) ag_drv_cnpl_counter_cfg_set(cnt_loc_profile, &counter_cfg);
        if(!err) ag_drv_cnpl_counter_cfg_get( cnt_loc_profile, &counter_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_get( %u %u %u %u %u %u)\n", cnt_loc_profile, counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, counter_cfg.clr);
        if(err || counter_cfg.cn_double!=gtmv(m, 1) || counter_cfg.cn0_byts!=gtmv(m, 2) || counter_cfg.ba!=gtmv(m, 11) || counter_cfg.wrap!=gtmv(m, 1) || counter_cfg.clr!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t conf_idx=gtmv(m, 1);
        cnpl_policer_cfg policer_cfg = {.bk_ba=gtmv(m, 11), .pa_ba=gtmv(m, 11), .pl_double=gtmv(m, 1), .pl_st=gtmv(m, 8), .pl_end=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_set( %u %u %u %u %u %u)\n", conf_idx, policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.pl_st, policer_cfg.pl_end);
        if(!err) ag_drv_cnpl_policer_cfg_set(conf_idx, &policer_cfg);
        if(!err) ag_drv_cnpl_policer_cfg_get( conf_idx, &policer_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policer_cfg_get( %u %u %u %u %u %u)\n", conf_idx, policer_cfg.bk_ba, policer_cfg.pa_ba, policer_cfg.pl_double, policer_cfg.pl_st, policer_cfg.pl_end);
        if(err || policer_cfg.bk_ba!=gtmv(m, 11) || policer_cfg.pa_ba!=gtmv(m, 11) || policer_cfg.pl_double!=gtmv(m, 1) || policer_cfg.pl_st!=gtmv(m, 8) || policer_cfg.pl_end!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        cnpl_sw_stat sw_stat = {.cn_rd_st=gtmv(m, 1), .pl_plc_st=gtmv(m, 1), .pl_rd_st=gtmv(m, 1)};
        if(!err) ag_drv_cnpl_sw_stat_get( &sw_stat);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_stat_get( %u %u %u)\n", sw_stat.cn_rd_st, sw_stat.pl_plc_st, sw_stat.pl_rd_st);
    }
    {
        uint32_t mem_idx=gtmv(m, 9);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_memory_data_set( %u %u)\n", mem_idx, data);
        if(!err) ag_drv_cnpl_memory_data_set(mem_idx, data);
        if(!err) ag_drv_cnpl_memory_data_get( mem_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_memory_data_get( %u %u)\n", mem_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t conf_idx=gtmv(m, 0);
        uint32_t vec=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set( %u %u)\n", conf_idx, vec);
        if(!err) ag_drv_cnpl_policers_configurations_pl_calc_type_set(conf_idx, vec);
        if(!err) ag_drv_cnpl_policers_configurations_pl_calc_type_get( conf_idx, &vec);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_get( %u %u)\n", conf_idx, vec);
        if(err || vec!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t n=gtmv(m, 8);
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_set( %u %u)\n", n, en);
        if(!err) ag_drv_cnpl_policers_configurations_per_up_set(n, en);
        if(!err) ag_drv_cnpl_policers_configurations_per_up_get( &n, &en);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_get( %u %u)\n", n, en);
        if(err || n!=gtmv(m, 8) || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t sw_prio=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_set( %u)\n", sw_prio);
        if(!err) ag_drv_cnpl_misc_arb_prm_set(sw_prio);
        if(!err) ag_drv_cnpl_misc_arb_prm_get( &sw_prio);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_get( %u)\n", sw_prio);
        if(err || sw_prio!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_set( %u)\n", val);
        if(!err) ag_drv_cnpl_sw_if_sw_cmd_set(val);
        if(!err) ag_drv_cnpl_sw_if_sw_cmd_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_get( %u)\n", val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t col=gtmv(m, 2);
        if(!err) ag_drv_cnpl_sw_if_sw_pl_rslt_get( &col);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rslt_get( %u)\n", col);
    }
    {
        uint32_t bucket=gtmv(m, 1);
        uint32_t rd=gtmv(m, 32);
        if(!err) ag_drv_cnpl_sw_if_sw_pl_rd_get( bucket, &rd);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rd_get( %u %u)\n", bucket, rd);
    }
    {
        uint32_t rd_idx=gtmv(m, 3);
        uint32_t rd=gtmv(m, 32);
        if(!err) ag_drv_cnpl_sw_if_sw_cnt_rd_get( rd_idx, &rd);
        if(!err) bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cnt_rd_get( %u %u)\n", rd_idx, rd);
    }
    return err;
}

static int bcm_cnpl_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_memory_data : reg = &RU_REG(CNPL, MEMORY_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_counters_configurations_cn_loc_prof : reg = &RU_REG(CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof0 : reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF0); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof1 : reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF1); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_calc_type : reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_per_up : reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PER_UP); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_arb_prm : reg = &RU_REG(CNPL, MISC_ARB_PRM); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cmd : reg = &RU_REG(CNPL, SW_IF_SW_CMD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_stat : reg = &RU_REG(CNPL, SW_IF_SW_STAT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rslt : reg = &RU_REG(CNPL, SW_IF_SW_PL_RSLT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rd : reg = &RU_REG(CNPL, SW_IF_SW_PL_RD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cnt_rd : reg = &RU_REG(CNPL, SW_IF_SW_CNT_RD); blk = &RU_BLK(CNPL); break;
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

bdmfmon_handle_t ag_drv_cnpl_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "cnpl"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "cnpl", "cnpl", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_counter_cfg[]={
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cn_double", "cn_double", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cn0_byts", "cn0_byts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ba", "ba", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clr", "clr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policer_cfg[]={
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bk_ba", "bk_ba", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pa_ba", "pa_ba", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pl_double", "pl_double", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pl_st", "pl_st", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pl_end", "pl_end", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_memory_data[]={
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_calc_type[]={
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vec", "vec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_per_up[]={
            BDMFMON_MAKE_PARM("n", "n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_arb_prm[]={
            BDMFMON_MAKE_PARM("sw_prio", "sw_prio", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_cmd[]={
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="counter_cfg", .val=cli_cnpl_counter_cfg, .parms=set_counter_cfg },
            { .name="policer_cfg", .val=cli_cnpl_policer_cfg, .parms=set_policer_cfg },
            { .name="memory_data", .val=cli_cnpl_memory_data, .parms=set_memory_data },
            { .name="policers_configurations_pl_calc_type", .val=cli_cnpl_policers_configurations_pl_calc_type, .parms=set_policers_configurations_pl_calc_type },
            { .name="policers_configurations_per_up", .val=cli_cnpl_policers_configurations_per_up, .parms=set_policers_configurations_per_up },
            { .name="misc_arb_prm", .val=cli_cnpl_misc_arb_prm, .parms=set_misc_arb_prm },
            { .name="sw_if_sw_cmd", .val=cli_cnpl_sw_if_sw_cmd, .parms=set_sw_if_sw_cmd },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_cnpl_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counter_cfg[]={
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policer_cfg[]={
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_memory_data[]={
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_calc_type[]={
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_pl_rd[]={
            BDMFMON_MAKE_PARM("bucket", "bucket", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_cnt_rd[]={
            BDMFMON_MAKE_PARM("rd_idx", "rd_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="counter_cfg", .val=cli_cnpl_counter_cfg, .parms=set_counter_cfg },
            { .name="policer_cfg", .val=cli_cnpl_policer_cfg, .parms=set_policer_cfg },
            { .name="sw_stat", .val=cli_cnpl_sw_stat, .parms=set_default },
            { .name="memory_data", .val=cli_cnpl_memory_data, .parms=set_memory_data },
            { .name="policers_configurations_pl_calc_type", .val=cli_cnpl_policers_configurations_pl_calc_type, .parms=set_policers_configurations_pl_calc_type },
            { .name="policers_configurations_per_up", .val=cli_cnpl_policers_configurations_per_up, .parms=set_default },
            { .name="misc_arb_prm", .val=cli_cnpl_misc_arb_prm, .parms=set_default },
            { .name="sw_if_sw_cmd", .val=cli_cnpl_sw_if_sw_cmd, .parms=set_default },
            { .name="sw_if_sw_pl_rslt", .val=cli_cnpl_sw_if_sw_pl_rslt, .parms=set_default },
            { .name="sw_if_sw_pl_rd", .val=cli_cnpl_sw_if_sw_pl_rd, .parms=set_sw_if_sw_pl_rd },
            { .name="sw_if_sw_cnt_rd", .val=cli_cnpl_sw_if_sw_cnt_rd, .parms=set_sw_if_sw_cnt_rd },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_cnpl_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_cnpl_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="MEMORY_DATA" , .val=bdmf_address_memory_data },
            { .name="COUNTERS_CONFIGURATIONS_CN_LOC_PROF" , .val=bdmf_address_counters_configurations_cn_loc_prof },
            { .name="POLICERS_CONFIGURATIONS_PL_LOC_PROF0" , .val=bdmf_address_policers_configurations_pl_loc_prof0 },
            { .name="POLICERS_CONFIGURATIONS_PL_LOC_PROF1" , .val=bdmf_address_policers_configurations_pl_loc_prof1 },
            { .name="POLICERS_CONFIGURATIONS_PL_CALC_TYPE" , .val=bdmf_address_policers_configurations_pl_calc_type },
            { .name="POLICERS_CONFIGURATIONS_PER_UP" , .val=bdmf_address_policers_configurations_per_up },
            { .name="MISC_ARB_PRM" , .val=bdmf_address_misc_arb_prm },
            { .name="SW_IF_SW_CMD" , .val=bdmf_address_sw_if_sw_cmd },
            { .name="SW_IF_SW_STAT" , .val=bdmf_address_sw_if_sw_stat },
            { .name="SW_IF_SW_PL_RSLT" , .val=bdmf_address_sw_if_sw_pl_rslt },
            { .name="SW_IF_SW_PL_RD" , .val=bdmf_address_sw_if_sw_pl_rd },
            { .name="SW_IF_SW_CNT_RD" , .val=bdmf_address_sw_if_sw_cnt_rd },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_cnpl_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

