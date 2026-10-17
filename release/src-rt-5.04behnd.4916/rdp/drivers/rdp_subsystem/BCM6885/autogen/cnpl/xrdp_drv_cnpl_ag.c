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

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_cnpl_counter_cfg_set(uint8_t cnpl_id, uint32_t cnt_loc_profile, const cnpl_counter_cfg *counter_cfg)
{
    uint32_t reg_counters_configurations_cn_loc_prof = 0;

#ifdef VALIDATE_PARMS
    if(!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (cnt_loc_profile >= 16) ||
       (counter_cfg->cn_double >= _1BITS_MAX_VAL_) ||
       (counter_cfg->cn0_byts >= _2BITS_MAX_VAL_) ||
       (counter_cfg->ba >= _18BITS_MAX_VAL_) ||
       (counter_cfg->wrap >= _1BITS_MAX_VAL_) ||
       (counter_cfg->clr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, cnt_loc_profile, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, reg_counters_configurations_cn_loc_prof);

    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, DOUBLLE, reg_counters_configurations_cn_loc_prof, counter_cfg->cn_double);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CN0_BYTS, reg_counters_configurations_cn_loc_prof, counter_cfg->cn0_byts);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, BA, reg_counters_configurations_cn_loc_prof, counter_cfg->ba);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, WRAP, reg_counters_configurations_cn_loc_prof, counter_cfg->wrap);
    reg_counters_configurations_cn_loc_prof = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CLR, reg_counters_configurations_cn_loc_prof, counter_cfg->clr);

    RU_REG_RAM_WRITE(cnpl_id, cnt_loc_profile, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, reg_counters_configurations_cn_loc_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_counter_cfg_get(uint8_t cnpl_id, uint32_t cnt_loc_profile, cnpl_counter_cfg *counter_cfg)
{
    uint32_t reg_counters_configurations_cn_loc_prof;

#ifdef VALIDATE_PARMS
    if (!counter_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (cnt_loc_profile >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, cnt_loc_profile, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, reg_counters_configurations_cn_loc_prof);

    counter_cfg->cn_double = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, DOUBLLE, reg_counters_configurations_cn_loc_prof);
    counter_cfg->cn0_byts = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CN0_BYTS, reg_counters_configurations_cn_loc_prof);
    counter_cfg->ba = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, BA, reg_counters_configurations_cn_loc_prof);
    counter_cfg->wrap = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, WRAP, reg_counters_configurations_cn_loc_prof);
    counter_cfg->clr = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF, CLR, reg_counters_configurations_cn_loc_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_stat_get(uint8_t cnpl_id, cnpl_sw_stat *sw_stat)
{
    uint32_t reg_sw_if_sw_stat;

#ifdef VALIDATE_PARMS
    if (!sw_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_STAT, reg_sw_if_sw_stat);

    sw_stat->cn_rd_st = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_STAT, CN_RD_ST, reg_sw_if_sw_stat);
    sw_stat->pl_plc_st = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_STAT, PL_PLC_ST, reg_sw_if_sw_stat);
    sw_stat->pl_rd_st = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_STAT, PL_RD_ST, reg_sw_if_sw_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_main_memory_main_data_set(uint8_t cnpl_id, uint32_t mem_idx, uint32_t data)
{
    uint32_t reg_main_memory_main_data = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mem_idx >= 196608))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_main_memory_main_data = RU_FIELD_SET(cnpl_id, CNPL, MAIN_MEMORY_MAIN_DATA, DATA, reg_main_memory_main_data, data);

    RU_REG_RAM_WRITE(cnpl_id, mem_idx, CNPL, MAIN_MEMORY_MAIN_DATA, reg_main_memory_main_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_main_memory_main_data_get(uint8_t cnpl_id, uint32_t mem_idx, uint32_t *data)
{
    uint32_t reg_main_memory_main_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mem_idx >= 196608))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, mem_idx, CNPL, MAIN_MEMORY_MAIN_DATA, reg_main_memory_main_data);

    *data = RU_FIELD_GET(cnpl_id, CNPL, MAIN_MEMORY_MAIN_DATA, DATA, reg_main_memory_main_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_num64_memory_b64_data_set(uint8_t cnpl_id, uint32_t mem_idx, uint32_t data)
{
    uint32_t reg_num64_memory_b64_data = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mem_idx >= 384))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_num64_memory_b64_data = RU_FIELD_SET(cnpl_id, CNPL, NUM64_MEMORY_B64_DATA, DATA, reg_num64_memory_b64_data, data);

    RU_REG_RAM_WRITE(cnpl_id, mem_idx, CNPL, NUM64_MEMORY_B64_DATA, reg_num64_memory_b64_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_num64_memory_b64_data_get(uint8_t cnpl_id, uint32_t mem_idx, uint32_t *data)
{
    uint32_t reg_num64_memory_b64_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mem_idx >= 384))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, mem_idx, CNPL, NUM64_MEMORY_B64_DATA, reg_num64_memory_b64_data);

    *data = RU_FIELD_GET(cnpl_id, CNPL, NUM64_MEMORY_B64_DATA, DATA, reg_num64_memory_b64_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t data)
{
    uint32_t reg_plc_prof_sel_mem_plc_prof_sel_data = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8192))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_plc_prof_sel_mem_plc_prof_sel_data = RU_FIELD_SET(cnpl_id, CNPL, PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA, DATA, reg_plc_prof_sel_mem_plc_prof_sel_data, data);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA, reg_plc_prof_sel_mem_plc_prof_sel_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *data)
{
    uint32_t reg_plc_prof_sel_mem_plc_prof_sel_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8192))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA, reg_plc_prof_sel_mem_plc_prof_sel_data);

    *data = RU_FIELD_GET(cnpl_id, CNPL, PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA, DATA, reg_plc_prof_sel_mem_plc_prof_sel_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t data)
{
    uint32_t reg_plc_profiles_mem_plc_profiles_data = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 1024))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_plc_profiles_mem_plc_profiles_data = RU_FIELD_SET(cnpl_id, CNPL, PLC_PROFILES_MEM_PLC_PROFILES_DATA, DATA, reg_plc_profiles_mem_plc_profiles_data, data);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, PLC_PROFILES_MEM_PLC_PROFILES_DATA, reg_plc_profiles_mem_plc_profiles_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *data)
{
    uint32_t reg_plc_profiles_mem_plc_profiles_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 1024))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, PLC_PROFILES_MEM_PLC_PROFILES_DATA, reg_plc_profiles_mem_plc_profiles_data);

    *data = RU_FIELD_GET(cnpl_id, CNPL, PLC_PROFILES_MEM_PLC_PROFILES_DATA, DATA, reg_plc_profiles_mem_plc_profiles_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_counters_configurations_cn_glb_ctrl_set(uint8_t cnpl_id, uint8_t mode)
{
    uint32_t reg_counters_configurations_cn_glb_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mode >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_counters_configurations_cn_glb_ctrl = RU_FIELD_SET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_GLB_CTRL, MODE, reg_counters_configurations_cn_glb_ctrl, mode);

    RU_REG_WRITE(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_GLB_CTRL, reg_counters_configurations_cn_glb_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_counters_configurations_cn_glb_ctrl_get(uint8_t cnpl_id, uint8_t *mode)
{
    uint32_t reg_counters_configurations_cn_glb_ctrl;

#ifdef VALIDATE_PARMS
    if (!mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_GLB_CTRL, reg_counters_configurations_cn_glb_ctrl);

    *mode = RU_FIELD_GET(cnpl_id, CNPL, COUNTERS_CONFIGURATIONS_CN_GLB_CTRL, MODE, reg_counters_configurations_cn_glb_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t bk_ba, uint16_t pa_ba, bdmf_boolean doublle, bdmf_boolean fc)
{
    uint32_t reg_policers_configurations_pl_loc_prof_a = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8) ||
       (pa_ba >= _12BITS_MAX_VAL_) ||
       (doublle >= _1BITS_MAX_VAL_) ||
       (fc >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_loc_prof_a = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, BK_BA, reg_policers_configurations_pl_loc_prof_a, bk_ba);
    reg_policers_configurations_pl_loc_prof_a = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, PA_BA, reg_policers_configurations_pl_loc_prof_a, pa_ba);
    reg_policers_configurations_pl_loc_prof_a = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, DOUBLLE, reg_policers_configurations_pl_loc_prof_a, doublle);
    reg_policers_configurations_pl_loc_prof_a = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, FC, reg_policers_configurations_pl_loc_prof_a, fc);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, reg_policers_configurations_pl_loc_prof_a);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *bk_ba, uint16_t *pa_ba, bdmf_boolean *doublle, bdmf_boolean *fc)
{
    uint32_t reg_policers_configurations_pl_loc_prof_a;

#ifdef VALIDATE_PARMS
    if (!bk_ba || !pa_ba || !doublle || !fc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, reg_policers_configurations_pl_loc_prof_a);

    *bk_ba = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, BK_BA, reg_policers_configurations_pl_loc_prof_a);
    *pa_ba = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, PA_BA, reg_policers_configurations_pl_loc_prof_a);
    *doublle = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, DOUBLLE, reg_policers_configurations_pl_loc_prof_a);
    *fc = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, FC, reg_policers_configurations_pl_loc_prof_a);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t pl_st, uint16_t pl_end)
{
    uint32_t reg_policers_configurations_pl_loc_prof_b = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_loc_prof_b = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, PL_ST, reg_policers_configurations_pl_loc_prof_b, pl_st);
    reg_policers_configurations_pl_loc_prof_b = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, PL_END, reg_policers_configurations_pl_loc_prof_b, pl_end);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, reg_policers_configurations_pl_loc_prof_b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *pl_st, uint16_t *pl_end)
{
    uint32_t reg_policers_configurations_pl_loc_prof_b;

#ifdef VALIDATE_PARMS
    if (!pl_st || !pl_end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, reg_policers_configurations_pl_loc_prof_b);

    *pl_st = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, PL_ST, reg_policers_configurations_pl_loc_prof_b);
    *pl_end = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, PL_END, reg_policers_configurations_pl_loc_prof_b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(uint8_t cnpl_id, uint32_t conf_idx, uint8_t n)
{
    uint32_t reg_policers_configurations_pl_loc_prof_c = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_loc_prof_c = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_C, N, reg_policers_configurations_pl_loc_prof_c, n);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_C, reg_policers_configurations_pl_loc_prof_c);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(uint8_t cnpl_id, uint32_t conf_idx, uint8_t *n)
{
    uint32_t reg_policers_configurations_pl_loc_prof_c;

#ifdef VALIDATE_PARMS
    if (!n)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_C, reg_policers_configurations_pl_loc_prof_c);

    *n = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_C, N, reg_policers_configurations_pl_loc_prof_c);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t vec)
{
    uint32_t reg_policers_configurations_pl_calc_type = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_calc_type = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, VEC, reg_policers_configurations_pl_calc_type, vec);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, reg_policers_configurations_pl_calc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *vec)
{
    uint32_t reg_policers_configurations_pl_calc_type;

#ifdef VALIDATE_PARMS
    if (!vec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, reg_policers_configurations_pl_calc_type);

    *vec = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE, VEC, reg_policers_configurations_pl_calc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_set(uint8_t cnpl_id, uint32_t conf_idx, uint16_t prf0, uint16_t prf1)
{
    uint32_t reg_policers_configurations_pl_size_prof = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_pl_size_prof = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF0, reg_policers_configurations_pl_size_prof, prf0);
    reg_policers_configurations_pl_size_prof = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF1, reg_policers_configurations_pl_size_prof, prf1);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, reg_policers_configurations_pl_size_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_get(uint8_t cnpl_id, uint32_t conf_idx, uint16_t *prf0, uint16_t *prf1)
{
    uint32_t reg_policers_configurations_pl_size_prof;

#ifdef VALIDATE_PARMS
    if (!prf0 || !prf1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, reg_policers_configurations_pl_size_prof);

    *prf0 = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF0, reg_policers_configurations_pl_size_prof);
    *prf1 = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF, PRF1, reg_policers_configurations_pl_size_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_set(uint8_t cnpl_id, bdmf_boolean en, uint16_t mtu)
{
    uint32_t reg_policers_configurations_per_up = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_) ||
       (mtu >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_configurations_per_up = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up, en);
    reg_policers_configurations_per_up = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, MTU, reg_policers_configurations_per_up, mtu);

    RU_REG_WRITE(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_get(uint8_t cnpl_id, bdmf_boolean *en, uint16_t *mtu)
{
    uint32_t reg_policers_configurations_per_up;

#ifdef VALIDATE_PARMS
    if (!en || !mtu)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, reg_policers_configurations_per_up);

    *en = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, EN, reg_policers_configurations_per_up);
    *mtu = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_CONFIGURATIONS_PER_UP, MTU, reg_policers_configurations_per_up);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(uint8_t cnpl_id, uint32_t conf_idx, uint32_t val)
{
    uint32_t reg_policers_fc_thr_fc_thr_l = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_fc_thr_fc_thr_l = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_FC_THR_FC_THR_L, VAL, reg_policers_fc_thr_fc_thr_l, val);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_FC_THR_FC_THR_L, reg_policers_fc_thr_fc_thr_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(uint8_t cnpl_id, uint32_t conf_idx, uint32_t *val)
{
    uint32_t reg_policers_fc_thr_fc_thr_l;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_FC_THR_FC_THR_L, reg_policers_fc_thr_fc_thr_l);

    *val = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_FC_THR_FC_THR_L, VAL, reg_policers_fc_thr_fc_thr_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(uint8_t cnpl_id, uint32_t conf_idx, uint8_t val)
{
    uint32_t reg_policers_fc_thr_fc_thr_m = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 64) ||
       (val >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_policers_fc_thr_fc_thr_m = RU_FIELD_SET(cnpl_id, CNPL, POLICERS_FC_THR_FC_THR_M, VAL, reg_policers_fc_thr_fc_thr_m, val);

    RU_REG_RAM_WRITE(cnpl_id, conf_idx, CNPL, POLICERS_FC_THR_FC_THR_M, reg_policers_fc_thr_fc_thr_m);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(uint8_t cnpl_id, uint32_t conf_idx, uint8_t *val)
{
    uint32_t reg_policers_fc_thr_fc_thr_m;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (conf_idx >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, conf_idx, CNPL, POLICERS_FC_THR_FC_THR_M, reg_policers_fc_thr_fc_thr_m);

    *val = RU_FIELD_GET(cnpl_id, CNPL, POLICERS_FC_THR_FC_THR_M, VAL, reg_policers_fc_thr_fc_thr_m);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_h_set(uint8_t cnpl_id, uint32_t val)
{
    uint32_t reg_sw_if_sw_cmd_h = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sw_if_sw_cmd_h = RU_FIELD_SET(cnpl_id, CNPL, SW_IF_SW_CMD_H, VAL, reg_sw_if_sw_cmd_h, val);

    RU_REG_WRITE(cnpl_id, CNPL, SW_IF_SW_CMD_H, reg_sw_if_sw_cmd_h);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_h_get(uint8_t cnpl_id, uint32_t *val)
{
    uint32_t reg_sw_if_sw_cmd_h;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_CMD_H, reg_sw_if_sw_cmd_h);

    *val = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_CMD_H, VAL, reg_sw_if_sw_cmd_h);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_l_set(uint8_t cnpl_id, uint32_t val)
{
    uint32_t reg_sw_if_sw_cmd_l = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sw_if_sw_cmd_l = RU_FIELD_SET(cnpl_id, CNPL, SW_IF_SW_CMD_L, VAL, reg_sw_if_sw_cmd_l, val);

    RU_REG_WRITE(cnpl_id, CNPL, SW_IF_SW_CMD_L, reg_sw_if_sw_cmd_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_l_get(uint8_t cnpl_id, uint32_t *val)
{
    uint32_t reg_sw_if_sw_cmd_l;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_CMD_L, reg_sw_if_sw_cmd_l);

    *val = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_CMD_L, VAL, reg_sw_if_sw_cmd_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rslt_get(uint8_t cnpl_id, uint8_t *col)
{
    uint32_t reg_sw_if_sw_pl_rslt;

#ifdef VALIDATE_PARMS
    if (!col)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_PL_RSLT, reg_sw_if_sw_pl_rslt);

    *col = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_PL_RSLT, COL, reg_sw_if_sw_pl_rslt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rd_get(uint8_t cnpl_id, uint32_t bucket, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_pl_rd;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (bucket >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, bucket, CNPL, SW_IF_SW_PL_RD, reg_sw_if_sw_pl_rd);

    *rd = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_PL_RD, RD, reg_sw_if_sw_pl_rd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_cnt_rd_get(uint8_t cnpl_id, uint32_t rd_idx, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_cnt_rd;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (rd_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(cnpl_id, rd_idx, CNPL, SW_IF_SW_CNT_RD, reg_sw_if_sw_cnt_rd);

    *rd = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_CNT_RD, RD, reg_sw_if_sw_cnt_rd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_rd_used_get(uint8_t cnpl_id, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_ovflw_f_rd_used;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_RD_USED, reg_sw_if_sw_ovflw_f_rd_used);

    *rd = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_RD_USED, RD, reg_sw_if_sw_ovflw_f_rd_used);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_pop_get(uint8_t cnpl_id, uint32_t *rd)
{
    uint32_t reg_sw_if_sw_ovflw_f_pop;

#ifdef VALIDATE_PARMS
    if (!rd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_POP, reg_sw_if_sw_ovflw_f_pop);

    *rd = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_POP, RD, reg_sw_if_sw_ovflw_f_pop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_thr_set(uint8_t cnpl_id, uint8_t val)
{
    uint32_t reg_sw_if_sw_ovflw_f_thr = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sw_if_sw_ovflw_f_thr = RU_FIELD_SET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_THR, VAL, reg_sw_if_sw_ovflw_f_thr, val);

    RU_REG_WRITE(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_THR, reg_sw_if_sw_ovflw_f_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_thr_get(uint8_t cnpl_id, uint8_t *val)
{
    uint32_t reg_sw_if_sw_ovflw_f_thr;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_THR, reg_sw_if_sw_ovflw_f_thr);

    *val = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_THR, VAL, reg_sw_if_sw_ovflw_f_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_set(uint8_t cnpl_id, bdmf_boolean val)
{
    uint32_t reg_sw_if_sw_ovflw_f_full_mod = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (val >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sw_if_sw_ovflw_f_full_mod = RU_FIELD_SET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_FULL_MOD, VAL, reg_sw_if_sw_ovflw_f_full_mod, val);

    RU_REG_WRITE(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_FULL_MOD, reg_sw_if_sw_ovflw_f_full_mod);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_get(uint8_t cnpl_id, bdmf_boolean *val)
{
    uint32_t reg_sw_if_sw_ovflw_f_full_mod;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_FULL_MOD, reg_sw_if_sw_ovflw_f_full_mod);

    *val = RU_FIELD_GET(cnpl_id, CNPL, SW_IF_SW_OVFLW_F_FULL_MOD, VAL, reg_sw_if_sw_ovflw_f_full_mod);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_set(uint8_t cnpl_id, uint8_t sw_prio, uint8_t mem_addr_bit_sel)
{
    uint32_t reg_misc_arb_prm = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (sw_prio >= _2BITS_MAX_VAL_) ||
       (mem_addr_bit_sel >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_arb_prm = RU_FIELD_SET(cnpl_id, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm, sw_prio);
    reg_misc_arb_prm = RU_FIELD_SET(cnpl_id, CNPL, MISC_ARB_PRM, MEM_ADDR_BIT_SEL, reg_misc_arb_prm, mem_addr_bit_sel);

    RU_REG_WRITE(cnpl_id, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_arb_prm_get(uint8_t cnpl_id, uint8_t *sw_prio, uint8_t *mem_addr_bit_sel)
{
    uint32_t reg_misc_arb_prm;

#ifdef VALIDATE_PARMS
    if (!sw_prio || !mem_addr_bit_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, MISC_ARB_PRM, reg_misc_arb_prm);

    *sw_prio = RU_FIELD_GET(cnpl_id, CNPL, MISC_ARB_PRM, SW_PRIO, reg_misc_arb_prm);
    *mem_addr_bit_sel = RU_FIELD_GET(cnpl_id, CNPL, MISC_ARB_PRM, MEM_ADDR_BIT_SEL, reg_misc_arb_prm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_col_awr_en_set(uint8_t cnpl_id, bdmf_boolean en)
{
    uint32_t reg_misc_col_awr_en = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_col_awr_en = RU_FIELD_SET(cnpl_id, CNPL, MISC_COL_AWR_EN, EN, reg_misc_col_awr_en, en);

    RU_REG_WRITE(cnpl_id, CNPL, MISC_COL_AWR_EN, reg_misc_col_awr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_col_awr_en_get(uint8_t cnpl_id, bdmf_boolean *en)
{
    uint32_t reg_misc_col_awr_en;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, MISC_COL_AWR_EN, reg_misc_col_awr_en);

    *en = RU_FIELD_GET(cnpl_id, CNPL, MISC_COL_AWR_EN, EN, reg_misc_col_awr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_mem_init_set(uint8_t cnpl_id, bdmf_boolean main_init, bdmf_boolean plc_prof_sel_init, bdmf_boolean plc_profiles_init)
{
    uint32_t reg_misc_mem_init = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (main_init >= _1BITS_MAX_VAL_) ||
       (plc_prof_sel_init >= _1BITS_MAX_VAL_) ||
       (plc_profiles_init >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_mem_init = RU_FIELD_SET(cnpl_id, CNPL, MISC_MEM_INIT, MAIN_INIT, reg_misc_mem_init, main_init);
    reg_misc_mem_init = RU_FIELD_SET(cnpl_id, CNPL, MISC_MEM_INIT, PLC_PROF_SEL_INIT, reg_misc_mem_init, plc_prof_sel_init);
    reg_misc_mem_init = RU_FIELD_SET(cnpl_id, CNPL, MISC_MEM_INIT, PLC_PROFILES_INIT, reg_misc_mem_init, plc_profiles_init);

    RU_REG_WRITE(cnpl_id, CNPL, MISC_MEM_INIT, reg_misc_mem_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_misc_mem_init_get(uint8_t cnpl_id, bdmf_boolean *main_init, bdmf_boolean *plc_prof_sel_init, bdmf_boolean *plc_profiles_init)
{
    uint32_t reg_misc_mem_init;

#ifdef VALIDATE_PARMS
    if (!main_init || !plc_prof_sel_init || !plc_profiles_init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, MISC_MEM_INIT, reg_misc_mem_init);

    *main_init = RU_FIELD_GET(cnpl_id, CNPL, MISC_MEM_INIT, MAIN_INIT, reg_misc_mem_init);
    *plc_prof_sel_init = RU_FIELD_GET(cnpl_id, CNPL, MISC_MEM_INIT, PLC_PROF_SEL_INIT, reg_misc_mem_init);
    *plc_profiles_init = RU_FIELD_GET(cnpl_id, CNPL, MISC_MEM_INIT, PLC_PROFILES_INIT, reg_misc_mem_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_pm_counters_err_bad_entry_get(uint8_t cnpl_id, uint32_t *val)
{
    uint32_t reg_pm_counters_err_bad_entry;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, PM_COUNTERS_ERR_BAD_ENTRY, reg_pm_counters_err_bad_entry);

    *val = RU_FIELD_GET(cnpl_id, CNPL, PM_COUNTERS_ERR_BAD_ENTRY, VAL, reg_pm_counters_err_bad_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_pm_counters_ovflw_f_push_err_get(uint8_t cnpl_id, uint32_t *val)
{
    uint32_t reg_pm_counters_ovflw_f_push_err;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, PM_COUNTERS_OVFLW_F_PUSH_ERR, reg_pm_counters_ovflw_f_push_err);

    *val = RU_FIELD_GET(cnpl_id, CNPL, PM_COUNTERS_OVFLW_F_PUSH_ERR, VAL, reg_pm_counters_ovflw_f_push_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_debug_err_bad_entry_data_get(uint8_t cnpl_id, uint8_t *req)
{
    uint32_t reg_debug_err_bad_entry_data;

#ifdef VALIDATE_PARMS
    if (!req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, DEBUG_ERR_BAD_ENTRY_DATA, reg_debug_err_bad_entry_data);

    *req = RU_FIELD_GET(cnpl_id, CNPL, DEBUG_ERR_BAD_ENTRY_DATA, REQ, reg_debug_err_bad_entry_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_isr_set(uint8_t cnpl_id, bdmf_boolean mult_match)
{
    uint32_t reg_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (mult_match >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(cnpl_id, CNPL, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr, mult_match);

    RU_REG_WRITE(cnpl_id, CNPL, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_isr_get(uint8_t cnpl_id, bdmf_boolean *mult_match)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!mult_match)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    *mult_match = RU_FIELD_GET(cnpl_id, CNPL, INTR_CTRL_ISR, MULT_MATCH, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_ism_get(uint8_t cnpl_id, uint8_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(cnpl_id, CNPL, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_ier_set(uint8_t cnpl_id, uint8_t iem)
{
    uint32_t reg_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT) ||
       (iem >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(cnpl_id, CNPL, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(cnpl_id, CNPL, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_ier_get(uint8_t cnpl_id, uint8_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(cnpl_id, CNPL, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_itr_set(uint8_t cnpl_id, uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(cnpl_id, CNPL, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(cnpl_id, CNPL, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_cnpl_intr_ctrl_itr_get(uint8_t cnpl_id, uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cnpl_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(cnpl_id, CNPL, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(cnpl_id, CNPL, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_main_memory_main_data,
    bdmf_address_num64_memory_b64_data,
    bdmf_address_plc_prof_sel_mem_plc_prof_sel_data,
    bdmf_address_plc_profiles_mem_plc_profiles_data,
    bdmf_address_counters_configurations_cn_loc_prof,
    bdmf_address_counters_configurations_cn_glb_ctrl,
    bdmf_address_policers_configurations_pl_loc_prof_a,
    bdmf_address_policers_configurations_pl_loc_prof_b,
    bdmf_address_policers_configurations_pl_loc_prof_c,
    bdmf_address_policers_configurations_pl_calc_type,
    bdmf_address_policers_configurations_pl_size_prof,
    bdmf_address_policers_configurations_per_up,
    bdmf_address_policers_fc_thr_fc_thr_l,
    bdmf_address_policers_fc_thr_fc_thr_m,
    bdmf_address_sw_if_sw_cmd_h,
    bdmf_address_sw_if_sw_cmd_l,
    bdmf_address_sw_if_sw_stat,
    bdmf_address_sw_if_sw_pl_rslt,
    bdmf_address_sw_if_sw_pl_rd,
    bdmf_address_sw_if_sw_cnt_rd,
    bdmf_address_sw_if_sw_ovflw_f_rd_used,
    bdmf_address_sw_if_sw_ovflw_f_pop,
    bdmf_address_sw_if_sw_ovflw_f_thr,
    bdmf_address_sw_if_sw_ovflw_f_full_mod,
    bdmf_address_misc_arb_prm,
    bdmf_address_misc_col_awr_en,
    bdmf_address_misc_mem_init,
    bdmf_address_pm_counters_eng_cmds,
    bdmf_address_pm_counters_cmd_wait,
    bdmf_address_pm_counters_pl_up_err,
    bdmf_address_pm_counters_tot_cyc,
    bdmf_address_pm_counters_gnt_cyc,
    bdmf_address_pm_counters_arb_cyc,
    bdmf_address_pm_counters_err_bad_entry,
    bdmf_address_pm_counters_ovflw_f_push_err,
    bdmf_address_pm_counters_gen_cfg,
    bdmf_address_debug_dbgsel,
    bdmf_address_debug_dbgbus,
    bdmf_address_debug_req_vec,
    bdmf_address_debug_err_bad_entry_data,
    bdmf_address_debug_pol_up_st,
    bdmf_address_intr_ctrl_isr,
    bdmf_address_intr_ctrl_ism,
    bdmf_address_intr_ctrl_ier,
    bdmf_address_intr_ctrl_itr,
}
bdmf_address;

static int ag_drv_cnpl_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_cnpl_counter_cfg:
    {
        cnpl_counter_cfg counter_cfg = { .cn_double = parm[3].value.unumber, .cn0_byts = parm[4].value.unumber, .ba = parm[5].value.unumber, .wrap = parm[6].value.unumber, .clr = parm[7].value.unumber};
        ag_err = ag_drv_cnpl_counter_cfg_set(parm[1].value.unumber, parm[2].value.unumber, &counter_cfg);
        break;
    }
    case cli_cnpl_main_memory_main_data:
        ag_err = ag_drv_cnpl_main_memory_main_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_num64_memory_b64_data:
        ag_err = ag_drv_cnpl_num64_memory_b64_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data:
        ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_plc_profiles_mem_plc_profiles_data:
        ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_counters_configurations_cn_glb_ctrl:
        ag_err = ag_drv_cnpl_counters_configurations_cn_glb_ctrl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_loc_prof_a:
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_loc_prof_b:
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_loc_prof_c:
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_calc_type:
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_policers_configurations_pl_size_prof:
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_cnpl_policers_configurations_per_up:
        ag_err = ag_drv_cnpl_policers_configurations_per_up_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_policers_fc_thr_fc_thr_l:
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_policers_fc_thr_fc_thr_m:
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_cmd_h:
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_h_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_cmd_l:
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_l_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_ovflw_f_thr:
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_thr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_sw_if_sw_ovflw_f_full_mod:
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_misc_arb_prm:
        ag_err = ag_drv_cnpl_misc_arb_prm_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_cnpl_misc_col_awr_en:
        ag_err = ag_drv_cnpl_misc_col_awr_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_misc_mem_init:
        ag_err = ag_drv_cnpl_misc_mem_init_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_cnpl_intr_ctrl_isr:
        ag_err = ag_drv_cnpl_intr_ctrl_isr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_intr_ctrl_ier:
        ag_err = ag_drv_cnpl_intr_ctrl_ier_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_cnpl_intr_ctrl_itr:
        ag_err = ag_drv_cnpl_intr_ctrl_itr_set(parm[1].value.unumber, parm[2].value.unumber);
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
        ag_err = ag_drv_cnpl_counter_cfg_get(parm[1].value.unumber, parm[2].value.unumber, &counter_cfg);
        bdmf_session_print(session, "cn_double = %u = 0x%x\n", counter_cfg.cn_double, counter_cfg.cn_double);
        bdmf_session_print(session, "cn0_byts = %u = 0x%x\n", counter_cfg.cn0_byts, counter_cfg.cn0_byts);
        bdmf_session_print(session, "ba = %u = 0x%x\n", counter_cfg.ba, counter_cfg.ba);
        bdmf_session_print(session, "wrap = %u = 0x%x\n", counter_cfg.wrap, counter_cfg.wrap);
        bdmf_session_print(session, "clr = %u = 0x%x\n", counter_cfg.clr, counter_cfg.clr);
        break;
    }
    case cli_cnpl_sw_stat:
    {
        cnpl_sw_stat sw_stat;
        ag_err = ag_drv_cnpl_sw_stat_get(parm[1].value.unumber, &sw_stat);
        bdmf_session_print(session, "cn_rd_st = %u = 0x%x\n", sw_stat.cn_rd_st, sw_stat.cn_rd_st);
        bdmf_session_print(session, "pl_plc_st = %u = 0x%x\n", sw_stat.pl_plc_st, sw_stat.pl_plc_st);
        bdmf_session_print(session, "pl_rd_st = %u = 0x%x\n", sw_stat.pl_rd_st, sw_stat.pl_rd_st);
        break;
    }
    case cli_cnpl_main_memory_main_data:
    {
        uint32_t data;
        ag_err = ag_drv_cnpl_main_memory_main_data_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_cnpl_num64_memory_b64_data:
    {
        uint32_t data;
        ag_err = ag_drv_cnpl_num64_memory_b64_data_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data:
    {
        uint32_t data;
        ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_cnpl_plc_profiles_mem_plc_profiles_data:
    {
        uint32_t data;
        ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_cnpl_counters_configurations_cn_glb_ctrl:
    {
        uint8_t mode;
        ag_err = ag_drv_cnpl_counters_configurations_cn_glb_ctrl_get(parm[1].value.unumber, &mode);
        bdmf_session_print(session, "mode = %u = 0x%x\n", mode, mode);
        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_a:
    {
        uint16_t bk_ba;
        uint16_t pa_ba;
        bdmf_boolean doublle;
        bdmf_boolean fc;
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(parm[1].value.unumber, parm[2].value.unumber, &bk_ba, &pa_ba, &doublle, &fc);
        bdmf_session_print(session, "bk_ba = %u = 0x%x\n", bk_ba, bk_ba);
        bdmf_session_print(session, "pa_ba = %u = 0x%x\n", pa_ba, pa_ba);
        bdmf_session_print(session, "doublle = %u = 0x%x\n", doublle, doublle);
        bdmf_session_print(session, "fc = %u = 0x%x\n", fc, fc);
        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_b:
    {
        uint16_t pl_st;
        uint16_t pl_end;
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(parm[1].value.unumber, parm[2].value.unumber, &pl_st, &pl_end);
        bdmf_session_print(session, "pl_st = %u = 0x%x\n", pl_st, pl_st);
        bdmf_session_print(session, "pl_end = %u = 0x%x\n", pl_end, pl_end);
        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_c:
    {
        uint8_t n;
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(parm[1].value.unumber, parm[2].value.unumber, &n);
        bdmf_session_print(session, "n = %u = 0x%x\n", n, n);
        break;
    }
    case cli_cnpl_policers_configurations_pl_calc_type:
    {
        uint32_t vec;
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(parm[1].value.unumber, parm[2].value.unumber, &vec);
        bdmf_session_print(session, "vec = %u = 0x%x\n", vec, vec);
        break;
    }
    case cli_cnpl_policers_configurations_pl_size_prof:
    {
        uint16_t prf0;
        uint16_t prf1;
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(parm[1].value.unumber, parm[2].value.unumber, &prf0, &prf1);
        bdmf_session_print(session, "prf0 = %u = 0x%x\n", prf0, prf0);
        bdmf_session_print(session, "prf1 = %u = 0x%x\n", prf1, prf1);
        break;
    }
    case cli_cnpl_policers_configurations_per_up:
    {
        bdmf_boolean en;
        uint16_t mtu;
        ag_err = ag_drv_cnpl_policers_configurations_per_up_get(parm[1].value.unumber, &en, &mtu);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        bdmf_session_print(session, "mtu = %u = 0x%x\n", mtu, mtu);
        break;
    }
    case cli_cnpl_policers_fc_thr_fc_thr_l:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_policers_fc_thr_fc_thr_m:
    {
        uint8_t val;
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_cmd_h:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_h_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_cmd_l:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_l_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rslt:
    {
        uint8_t col;
        ag_err = ag_drv_cnpl_sw_if_sw_pl_rslt_get(parm[1].value.unumber, &col);
        bdmf_session_print(session, "col = %u = 0x%x\n", col, col);
        break;
    }
    case cli_cnpl_sw_if_sw_pl_rd:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_pl_rd_get(parm[1].value.unumber, parm[2].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_cnt_rd:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_cnt_rd_get(parm[1].value.unumber, parm[2].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_ovflw_f_rd_used:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_rd_used_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_ovflw_f_pop:
    {
        uint32_t rd;
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_pop_get(parm[1].value.unumber, &rd);
        bdmf_session_print(session, "rd = %u = 0x%x\n", rd, rd);
        break;
    }
    case cli_cnpl_sw_if_sw_ovflw_f_thr:
    {
        uint8_t val;
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_thr_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_sw_if_sw_ovflw_f_full_mod:
    {
        bdmf_boolean val;
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_misc_arb_prm:
    {
        uint8_t sw_prio;
        uint8_t mem_addr_bit_sel;
        ag_err = ag_drv_cnpl_misc_arb_prm_get(parm[1].value.unumber, &sw_prio, &mem_addr_bit_sel);
        bdmf_session_print(session, "sw_prio = %u = 0x%x\n", sw_prio, sw_prio);
        bdmf_session_print(session, "mem_addr_bit_sel = %u = 0x%x\n", mem_addr_bit_sel, mem_addr_bit_sel);
        break;
    }
    case cli_cnpl_misc_col_awr_en:
    {
        bdmf_boolean en;
        ag_err = ag_drv_cnpl_misc_col_awr_en_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_cnpl_misc_mem_init:
    {
        bdmf_boolean main_init;
        bdmf_boolean plc_prof_sel_init;
        bdmf_boolean plc_profiles_init;
        ag_err = ag_drv_cnpl_misc_mem_init_get(parm[1].value.unumber, &main_init, &plc_prof_sel_init, &plc_profiles_init);
        bdmf_session_print(session, "main_init = %u = 0x%x\n", main_init, main_init);
        bdmf_session_print(session, "plc_prof_sel_init = %u = 0x%x\n", plc_prof_sel_init, plc_prof_sel_init);
        bdmf_session_print(session, "plc_profiles_init = %u = 0x%x\n", plc_profiles_init, plc_profiles_init);
        break;
    }
    case cli_cnpl_pm_counters_err_bad_entry:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_pm_counters_err_bad_entry_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_pm_counters_ovflw_f_push_err:
    {
        uint32_t val;
        ag_err = ag_drv_cnpl_pm_counters_ovflw_f_push_err_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_cnpl_debug_err_bad_entry_data:
    {
        uint8_t req;
        ag_err = ag_drv_cnpl_debug_err_bad_entry_data_get(parm[1].value.unumber, &req);
        bdmf_session_print(session, "req = %u = 0x%x\n", req, req);
        break;
    }
    case cli_cnpl_intr_ctrl_isr:
    {
        bdmf_boolean mult_match;
        ag_err = ag_drv_cnpl_intr_ctrl_isr_get(parm[1].value.unumber, &mult_match);
        bdmf_session_print(session, "mult_match = %u = 0x%x\n", mult_match, mult_match);
        break;
    }
    case cli_cnpl_intr_ctrl_ism:
    {
        uint8_t ism;
        ag_err = ag_drv_cnpl_intr_ctrl_ism_get(parm[1].value.unumber, &ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_cnpl_intr_ctrl_ier:
    {
        uint8_t iem;
        ag_err = ag_drv_cnpl_intr_ctrl_ier_get(parm[1].value.unumber, &iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_cnpl_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_cnpl_intr_ctrl_itr_get(parm[1].value.unumber, &ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
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
    uint8_t cnpl_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t cnt_loc_profile = gtmv(m, 4);
        cnpl_counter_cfg counter_cfg = {.cn_double = gtmv(m, 1), .cn0_byts = gtmv(m, 2), .ba = gtmv(m, 18), .wrap = gtmv(m, 1), .clr = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set(%u %u %u %u %u %u %u)\n", cnpl_id, cnt_loc_profile,
            counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
            counter_cfg.clr);
        ag_err = ag_drv_cnpl_counter_cfg_set(cnpl_id, cnt_loc_profile, &counter_cfg);
        if (!ag_err)
            ag_err = ag_drv_cnpl_counter_cfg_get(cnpl_id, cnt_loc_profile, &counter_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_get(%u %u %u %u %u %u %u)\n", cnpl_id, cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (counter_cfg.cn_double != gtmv(m, 1) || counter_cfg.cn0_byts != gtmv(m, 2) || counter_cfg.ba != gtmv(m, 18) || counter_cfg.wrap != gtmv(m, 1) || counter_cfg.clr != gtmv(m, 1))
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
            ag_err = ag_drv_cnpl_sw_stat_get(cnpl_id, &sw_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_stat_get(%u %u %u %u)\n", cnpl_id,
                sw_stat.cn_rd_st, sw_stat.pl_plc_st, sw_stat.pl_rd_st);
        }
    }

    {
        uint32_t mem_idx = gtmv(m, 17);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_main_memory_main_data_set(%u %u %u)\n", cnpl_id, mem_idx,
            data);
        ag_err = ag_drv_cnpl_main_memory_main_data_set(cnpl_id, mem_idx, data);
        if (!ag_err)
            ag_err = ag_drv_cnpl_main_memory_main_data_get(cnpl_id, mem_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_main_memory_main_data_get(%u %u %u)\n", cnpl_id, mem_idx,
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
        uint32_t mem_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_num64_memory_b64_data_set(%u %u %u)\n", cnpl_id, mem_idx,
            data);
        ag_err = ag_drv_cnpl_num64_memory_b64_data_set(cnpl_id, mem_idx, data);
        if (!ag_err)
            ag_err = ag_drv_cnpl_num64_memory_b64_data_get(cnpl_id, mem_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_num64_memory_b64_data_get(%u %u %u)\n", cnpl_id, mem_idx,
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
        uint32_t conf_idx = gtmv(m, 13);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(%u %u %u)\n", cnpl_id, conf_idx,
            data);
        ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(cnpl_id, conf_idx, data);
        if (!ag_err)
            ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(cnpl_id, conf_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(%u %u %u)\n", cnpl_id, conf_idx,
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
        uint32_t conf_idx = gtmv(m, 10);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(%u %u %u)\n", cnpl_id, conf_idx,
            data);
        ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(cnpl_id, conf_idx, data);
        if (!ag_err)
            ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(cnpl_id, conf_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(%u %u %u)\n", cnpl_id, conf_idx,
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
        uint8_t mode = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_cnpl_counters_configurations_cn_glb_ctrl_set(%u %u)\n", cnpl_id,
            mode);
        ag_err = ag_drv_cnpl_counters_configurations_cn_glb_ctrl_set(cnpl_id, mode);
        if (!ag_err)
            ag_err = ag_drv_cnpl_counters_configurations_cn_glb_ctrl_get(cnpl_id, &mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counters_configurations_cn_glb_ctrl_get(%u %u)\n", cnpl_id,
                mode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mode != gtmv(m, 2))
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
        uint16_t bk_ba = gtmv(m, 16);
        uint16_t pa_ba = gtmv(m, 12);
        bdmf_boolean doublle = gtmv(m, 1);
        bdmf_boolean fc = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(%u %u %u %u %u %u)\n", cnpl_id, conf_idx,
            bk_ba, pa_ba, doublle, fc);
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(cnpl_id, conf_idx, bk_ba, pa_ba, doublle, fc);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(cnpl_id, conf_idx, &bk_ba, &pa_ba, &doublle, &fc);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(%u %u %u %u %u %u)\n", cnpl_id, conf_idx,
                bk_ba, pa_ba, doublle, fc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bk_ba != gtmv(m, 16) || pa_ba != gtmv(m, 12) || doublle != gtmv(m, 1) || fc != gtmv(m, 1))
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
        uint16_t pl_st = gtmv(m, 16);
        uint16_t pl_end = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(%u %u %u %u)\n", cnpl_id, conf_idx,
            pl_st, pl_end);
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(cnpl_id, conf_idx, pl_st, pl_end);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(cnpl_id, conf_idx, &pl_st, &pl_end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(%u %u %u %u)\n", cnpl_id, conf_idx,
                pl_st, pl_end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pl_st != gtmv(m, 16) || pl_end != gtmv(m, 16))
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
        uint8_t n = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(%u %u %u)\n", cnpl_id, conf_idx,
            n);
        ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(cnpl_id, conf_idx, n);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(cnpl_id, conf_idx, &n);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(%u %u %u)\n", cnpl_id, conf_idx,
                n);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (n != gtmv(m, 8))
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
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set(%u %u %u)\n", cnpl_id, conf_idx,
            vec);
        ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(cnpl_id, conf_idx, vec);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(cnpl_id, conf_idx, &vec);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_get(%u %u %u)\n", cnpl_id, conf_idx,
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
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set(%u %u %u %u)\n", cnpl_id, conf_idx,
            prf0, prf1);
        ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(cnpl_id, conf_idx, prf0, prf1);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(cnpl_id, conf_idx, &prf0, &prf1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_get(%u %u %u %u)\n", cnpl_id, conf_idx,
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
        bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_set(%u %u %u)\n", cnpl_id,
            en, mtu);
        ag_err = ag_drv_cnpl_policers_configurations_per_up_set(cnpl_id, en, mtu);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_configurations_per_up_get(cnpl_id, &en, &mtu);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_per_up_get(%u %u %u)\n", cnpl_id,
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
        uint32_t conf_idx = gtmv(m, 6);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(%u %u %u)\n", cnpl_id, conf_idx,
            val);
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(cnpl_id, conf_idx, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(cnpl_id, conf_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(%u %u %u)\n", cnpl_id, conf_idx,
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
        uint32_t conf_idx = gtmv(m, 6);
        uint8_t val = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(%u %u %u)\n", cnpl_id, conf_idx,
            val);
        ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(cnpl_id, conf_idx, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(cnpl_id, conf_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(%u %u %u)\n", cnpl_id, conf_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 4))
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
        bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_h_set(%u %u)\n", cnpl_id,
            val);
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_h_set(cnpl_id, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_cmd_h_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_h_get(%u %u)\n", cnpl_id,
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
        bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_l_set(%u %u)\n", cnpl_id,
            val);
        ag_err = ag_drv_cnpl_sw_if_sw_cmd_l_set(cnpl_id, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_cmd_l_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cmd_l_get(%u %u)\n", cnpl_id,
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
            ag_err = ag_drv_cnpl_sw_if_sw_pl_rslt_get(cnpl_id, &col);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rslt_get(%u %u)\n", cnpl_id,
                col);
        }
    }

    {
        uint32_t bucket = gtmv(m, 1);
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_pl_rd_get(cnpl_id, bucket, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_pl_rd_get(%u %u %u)\n", cnpl_id, bucket,
                rd);
        }
    }

    {
        uint32_t rd_idx = gtmv(m, 3);
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_cnt_rd_get(cnpl_id, rd_idx, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_cnt_rd_get(%u %u %u)\n", cnpl_id, rd_idx,
                rd);
        }
    }

    {
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_rd_used_get(cnpl_id, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_rd_used_get(%u %u)\n", cnpl_id,
                rd);
        }
    }

    {
        uint32_t rd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_pop_get(cnpl_id, &rd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_pop_get(%u %u)\n", cnpl_id,
                rd);
        }
    }

    {
        uint8_t val = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_thr_set(%u %u)\n", cnpl_id,
            val);
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_thr_set(cnpl_id, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_thr_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_thr_get(%u %u)\n", cnpl_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 8))
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
        bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_set(%u %u)\n", cnpl_id,
            val);
        ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_set(cnpl_id, val);
        if (!ag_err)
            ag_err = ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_sw_if_sw_ovflw_f_full_mod_get(%u %u)\n", cnpl_id,
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
        uint8_t sw_prio = gtmv(m, 2);
        uint8_t mem_addr_bit_sel = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_set(%u %u %u)\n", cnpl_id,
            sw_prio, mem_addr_bit_sel);
        ag_err = ag_drv_cnpl_misc_arb_prm_set(cnpl_id, sw_prio, mem_addr_bit_sel);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_arb_prm_get(cnpl_id, &sw_prio, &mem_addr_bit_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_arb_prm_get(%u %u %u)\n", cnpl_id,
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
        bdmf_session_print(session, "ag_drv_cnpl_misc_col_awr_en_set(%u %u)\n", cnpl_id,
            en);
        ag_err = ag_drv_cnpl_misc_col_awr_en_set(cnpl_id, en);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_col_awr_en_get(cnpl_id, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_col_awr_en_get(%u %u)\n", cnpl_id,
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
        bdmf_boolean main_init = gtmv(m, 1);
        bdmf_boolean plc_prof_sel_init = gtmv(m, 1);
        bdmf_boolean plc_profiles_init = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_cnpl_misc_mem_init_set(%u %u %u %u)\n", cnpl_id,
            main_init, plc_prof_sel_init, plc_profiles_init);
        ag_err = ag_drv_cnpl_misc_mem_init_set(cnpl_id, main_init, plc_prof_sel_init, plc_profiles_init);
        if (!ag_err)
            ag_err = ag_drv_cnpl_misc_mem_init_get(cnpl_id, &main_init, &plc_prof_sel_init, &plc_profiles_init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_misc_mem_init_get(%u %u %u %u)\n", cnpl_id,
                main_init, plc_prof_sel_init, plc_profiles_init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (main_init != gtmv(m, 1) || plc_prof_sel_init != gtmv(m, 1) || plc_profiles_init != gtmv(m, 1))
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
            ag_err = ag_drv_cnpl_pm_counters_err_bad_entry_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_pm_counters_err_bad_entry_get(%u %u)\n", cnpl_id,
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_cnpl_pm_counters_ovflw_f_push_err_get(cnpl_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_pm_counters_ovflw_f_push_err_get(%u %u)\n", cnpl_id,
                val);
        }
    }

    {
        uint8_t req = gtmv(m, 7);
        if (!ag_err)
            ag_err = ag_drv_cnpl_debug_err_bad_entry_data_get(cnpl_id, &req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_debug_err_bad_entry_data_get(%u %u)\n", cnpl_id,
                req);
        }
    }

    {
        bdmf_boolean mult_match = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_isr_set(%u %u)\n", cnpl_id,
            mult_match);
        ag_err = ag_drv_cnpl_intr_ctrl_isr_set(cnpl_id, mult_match);
        if (!ag_err)
            ag_err = ag_drv_cnpl_intr_ctrl_isr_get(cnpl_id, &mult_match);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_isr_get(%u %u)\n", cnpl_id,
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
            ag_err = ag_drv_cnpl_intr_ctrl_ism_get(cnpl_id, &ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_ism_get(%u %u)\n", cnpl_id,
                ism);
        }
    }

    {
        uint8_t iem = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_ier_set(%u %u)\n", cnpl_id,
            iem);
        ag_err = ag_drv_cnpl_intr_ctrl_ier_set(cnpl_id, iem);
        if (!ag_err)
            ag_err = ag_drv_cnpl_intr_ctrl_ier_get(cnpl_id, &iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_ier_get(%u %u)\n", cnpl_id,
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
        bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_itr_set(%u %u)\n", cnpl_id,
            ist);
        ag_err = ag_drv_cnpl_intr_ctrl_itr_set(cnpl_id, ist);
        if (!ag_err)
            ag_err = ag_drv_cnpl_intr_ctrl_itr_get(cnpl_id, &ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_cnpl_intr_ctrl_itr_get(%u %u)\n", cnpl_id,
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

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_cnpl_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t cnpl_id = parm[2].value.unumber;
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
            .ba = gtmv(m, 18), 
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
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set(%u %u %u %u %u %u %u)\n", cnpl_id, cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
            ag_err = ag_drv_cnpl_counter_cfg_set(cnpl_id, cnt_loc_profile, &counter_cfg);
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
        counter_cfg.ba = gtmv(m, 18);
        counter_cfg.wrap = gtmv(m, 1);
        counter_cfg.clr = gtmv(m, 1);

        for (cnt_loc_profile = start_idx; cnt_loc_profile <= stop_idx; cnt_loc_profile++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_set(%u %u %u %u %u %u %u)\n", cnpl_id, cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);
            ag_err = ag_drv_cnpl_counter_cfg_set(cnpl_id, cnt_loc_profile, &counter_cfg);
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

            ag_err = ag_drv_cnpl_counter_cfg_get(cnpl_id, cnt_loc_profile, &counter_cfg);

            bdmf_session_print(session, "ag_drv_cnpl_counter_cfg_get(%u %u %u %u %u %u %u)\n", cnpl_id, cnt_loc_profile,
                counter_cfg.cn_double, counter_cfg.cn0_byts, counter_cfg.ba, counter_cfg.wrap, 
                counter_cfg.clr);

            if (counter_cfg.cn_double != gtmv(m, 1) || 
                counter_cfg.cn0_byts != gtmv(m, 2) || 
                counter_cfg.ba != gtmv(m, 18) || 
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
    case cli_cnpl_main_memory_main_data:
    {
        uint32_t max_mem_idx = 196608;
        uint32_t mem_idx = gtmv(m, 17);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_mem_idx) || (stop_idx >= max_mem_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_mem_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (mem_idx = 0; mem_idx < max_mem_idx; mem_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_main_memory_main_data_set(%u %u %u)\n", cnpl_id, mem_idx,
                data);
            ag_err = ag_drv_cnpl_main_memory_main_data_set(cnpl_id, mem_idx, data);
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
            bdmf_session_print(session, "ag_drv_cnpl_main_memory_main_data_set(%u %u %u)\n", cnpl_id, mem_idx,
                data);
            ag_err = ag_drv_cnpl_main_memory_main_data_set(cnpl_id, mem_idx, data);
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

            ag_err = ag_drv_cnpl_main_memory_main_data_get(cnpl_id, mem_idx, &data);

            bdmf_session_print(session, "ag_drv_cnpl_main_memory_main_data_get(%u %u %u)\n", cnpl_id, mem_idx,
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
        bdmf_session_print(session, "Test of main_memory_main_data completed. Number of tested entries %u.\n", max_mem_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_num64_memory_b64_data:
    {
        uint32_t max_mem_idx = 384;
        uint32_t mem_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_mem_idx) || (stop_idx >= max_mem_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_mem_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (mem_idx = 0; mem_idx < max_mem_idx; mem_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_num64_memory_b64_data_set(%u %u %u)\n", cnpl_id, mem_idx,
                data);
            ag_err = ag_drv_cnpl_num64_memory_b64_data_set(cnpl_id, mem_idx, data);
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
            bdmf_session_print(session, "ag_drv_cnpl_num64_memory_b64_data_set(%u %u %u)\n", cnpl_id, mem_idx,
                data);
            ag_err = ag_drv_cnpl_num64_memory_b64_data_set(cnpl_id, mem_idx, data);
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

            ag_err = ag_drv_cnpl_num64_memory_b64_data_get(cnpl_id, mem_idx, &data);

            bdmf_session_print(session, "ag_drv_cnpl_num64_memory_b64_data_get(%u %u %u)\n", cnpl_id, mem_idx,
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
        bdmf_session_print(session, "Test of num64_memory_b64_data completed. Number of tested entries %u.\n", max_mem_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data:
    {
        uint32_t max_conf_idx = 8192;
        uint32_t conf_idx = gtmv(m, 13);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(%u %u %u)\n", cnpl_id, conf_idx,
                data);
            ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(cnpl_id, conf_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(%u %u %u)\n", cnpl_id, conf_idx,
                data);
            ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_set(cnpl_id, conf_idx, data);
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

            ag_err = ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(cnpl_id, conf_idx, &data);

            bdmf_session_print(session, "ag_drv_cnpl_plc_prof_sel_mem_plc_prof_sel_data_get(%u %u %u)\n", cnpl_id, conf_idx,
                data);

            if (data != gtmv(m, 32) || 0)
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
        bdmf_session_print(session, "Test of plc_prof_sel_mem_plc_prof_sel_data completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_plc_profiles_mem_plc_profiles_data:
    {
        uint32_t max_conf_idx = 1024;
        uint32_t conf_idx = gtmv(m, 10);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(%u %u %u)\n", cnpl_id, conf_idx,
                data);
            ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(cnpl_id, conf_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(%u %u %u)\n", cnpl_id, conf_idx,
                data);
            ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_set(cnpl_id, conf_idx, data);
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

            ag_err = ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(cnpl_id, conf_idx, &data);

            bdmf_session_print(session, "ag_drv_cnpl_plc_profiles_mem_plc_profiles_data_get(%u %u %u)\n", cnpl_id, conf_idx,
                data);

            if (data != gtmv(m, 32) || 0)
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
        bdmf_session_print(session, "Test of plc_profiles_mem_plc_profiles_data completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_a:
    {
        uint32_t max_conf_idx = 8;
        uint32_t conf_idx = gtmv(m, 3);
        uint16_t bk_ba = gtmv(m, 16);
        uint16_t pa_ba = gtmv(m, 12);
        bdmf_boolean doublle = gtmv(m, 1);
        bdmf_boolean fc = gtmv(m, 1);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(%u %u %u %u %u %u)\n", cnpl_id, conf_idx,
                bk_ba, pa_ba, doublle, fc);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(cnpl_id, conf_idx, bk_ba, pa_ba, doublle, fc);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        bk_ba = gtmv(m, 16);
        pa_ba = gtmv(m, 12);
        doublle = gtmv(m, 1);
        fc = gtmv(m, 1);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(%u %u %u %u %u %u)\n", cnpl_id, conf_idx,
                bk_ba, pa_ba, doublle, fc);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_set(cnpl_id, conf_idx, bk_ba, pa_ba, doublle, fc);
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

            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(cnpl_id, conf_idx, &bk_ba, &pa_ba, &doublle, &fc);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_a_get(%u %u %u %u %u %u)\n", cnpl_id, conf_idx,
                bk_ba, pa_ba, doublle, fc);

            if (bk_ba != gtmv(m, 16) || pa_ba != gtmv(m, 12) || doublle != gtmv(m, 1) || fc != gtmv(m, 1) || 0)
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
        bdmf_session_print(session, "Test of policers_configurations_pl_loc_prof_a completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_b:
    {
        uint32_t max_conf_idx = 8;
        uint32_t conf_idx = gtmv(m, 3);
        uint16_t pl_st = gtmv(m, 16);
        uint16_t pl_end = gtmv(m, 16);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(%u %u %u %u)\n", cnpl_id, conf_idx,
                pl_st, pl_end);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(cnpl_id, conf_idx, pl_st, pl_end);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        pl_st = gtmv(m, 16);
        pl_end = gtmv(m, 16);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(%u %u %u %u)\n", cnpl_id, conf_idx,
                pl_st, pl_end);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_set(cnpl_id, conf_idx, pl_st, pl_end);
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

            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(cnpl_id, conf_idx, &pl_st, &pl_end);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_b_get(%u %u %u %u)\n", cnpl_id, conf_idx,
                pl_st, pl_end);

            if (pl_st != gtmv(m, 16) || pl_end != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of policers_configurations_pl_loc_prof_b completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_configurations_pl_loc_prof_c:
    {
        uint32_t max_conf_idx = 8;
        uint32_t conf_idx = gtmv(m, 3);
        uint8_t n = gtmv(m, 8);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(%u %u %u)\n", cnpl_id, conf_idx,
                n);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(cnpl_id, conf_idx, n);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        n = gtmv(m, 8);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(%u %u %u)\n", cnpl_id, conf_idx,
                n);
            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_set(cnpl_id, conf_idx, n);
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

            ag_err = ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(cnpl_id, conf_idx, &n);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_loc_prof_c_get(%u %u %u)\n", cnpl_id, conf_idx,
                n);

            if (n != gtmv(m, 8) || 0)
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
        bdmf_session_print(session, "Test of policers_configurations_pl_loc_prof_c completed. Number of tested entries %u.\n", max_conf_idx);
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
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set(%u %u %u)\n", cnpl_id, conf_idx,
                vec);
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(cnpl_id, conf_idx, vec);
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
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_set(%u %u %u)\n", cnpl_id, conf_idx,
                vec);
            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_set(cnpl_id, conf_idx, vec);
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

            ag_err = ag_drv_cnpl_policers_configurations_pl_calc_type_get(cnpl_id, conf_idx, &vec);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_calc_type_get(%u %u %u)\n", cnpl_id, conf_idx,
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
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set(%u %u %u %u)\n", cnpl_id, conf_idx,
                prf0, prf1);
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(cnpl_id, conf_idx, prf0, prf1);
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
            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_set(%u %u %u %u)\n", cnpl_id, conf_idx,
                prf0, prf1);
            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_set(cnpl_id, conf_idx, prf0, prf1);
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

            ag_err = ag_drv_cnpl_policers_configurations_pl_size_prof_get(cnpl_id, conf_idx, &prf0, &prf1);

            bdmf_session_print(session, "ag_drv_cnpl_policers_configurations_pl_size_prof_get(%u %u %u %u)\n", cnpl_id, conf_idx,
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
    case cli_cnpl_policers_fc_thr_fc_thr_l:
    {
        uint32_t max_conf_idx = 64;
        uint32_t conf_idx = gtmv(m, 6);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(%u %u %u)\n", cnpl_id, conf_idx,
                val);
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(cnpl_id, conf_idx, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(%u %u %u)\n", cnpl_id, conf_idx,
                val);
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_set(cnpl_id, conf_idx, val);
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

            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(cnpl_id, conf_idx, &val);

            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_l_get(%u %u %u)\n", cnpl_id, conf_idx,
                val);

            if (val != gtmv(m, 32) || 0)
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
        bdmf_session_print(session, "Test of policers_fc_thr_fc_thr_l completed. Number of tested entries %u.\n", max_conf_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_cnpl_policers_fc_thr_fc_thr_m:
    {
        uint32_t max_conf_idx = 64;
        uint32_t conf_idx = gtmv(m, 6);
        uint8_t val = gtmv(m, 4);

        if ((start_idx >= max_conf_idx) || (stop_idx >= max_conf_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_conf_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (conf_idx = 0; conf_idx < max_conf_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(%u %u %u)\n", cnpl_id, conf_idx,
                val);
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(cnpl_id, conf_idx, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", conf_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 4);

        for (conf_idx = start_idx; conf_idx <= stop_idx; conf_idx++)
        {
            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(%u %u %u)\n", cnpl_id, conf_idx,
                val);
            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_set(cnpl_id, conf_idx, val);
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

            ag_err = ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(cnpl_id, conf_idx, &val);

            bdmf_session_print(session, "ag_drv_cnpl_policers_fc_thr_fc_thr_m_get(%u %u %u)\n", cnpl_id, conf_idx,
                val);

            if (val != gtmv(m, 4) || 0)
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
        bdmf_session_print(session, "Test of policers_fc_thr_fc_thr_m completed. Number of tested entries %u.\n", max_conf_idx);
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
    case bdmf_address_main_memory_main_data: reg = &RU_REG(CNPL, MAIN_MEMORY_MAIN_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_num64_memory_b64_data: reg = &RU_REG(CNPL, NUM64_MEMORY_B64_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_plc_prof_sel_mem_plc_prof_sel_data: reg = &RU_REG(CNPL, PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_plc_profiles_mem_plc_profiles_data: reg = &RU_REG(CNPL, PLC_PROFILES_MEM_PLC_PROFILES_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_counters_configurations_cn_loc_prof: reg = &RU_REG(CNPL, COUNTERS_CONFIGURATIONS_CN_LOC_PROF); blk = &RU_BLK(CNPL); break;
    case bdmf_address_counters_configurations_cn_glb_ctrl: reg = &RU_REG(CNPL, COUNTERS_CONFIGURATIONS_CN_GLB_CTRL); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof_a: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_A); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof_b: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_B); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_loc_prof_c: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_LOC_PROF_C); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_calc_type: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_CALC_TYPE); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_pl_size_prof: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PL_SIZE_PROF); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_configurations_per_up: reg = &RU_REG(CNPL, POLICERS_CONFIGURATIONS_PER_UP); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_fc_thr_fc_thr_l: reg = &RU_REG(CNPL, POLICERS_FC_THR_FC_THR_L); blk = &RU_BLK(CNPL); break;
    case bdmf_address_policers_fc_thr_fc_thr_m: reg = &RU_REG(CNPL, POLICERS_FC_THR_FC_THR_M); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cmd_h: reg = &RU_REG(CNPL, SW_IF_SW_CMD_H); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cmd_l: reg = &RU_REG(CNPL, SW_IF_SW_CMD_L); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_stat: reg = &RU_REG(CNPL, SW_IF_SW_STAT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rslt: reg = &RU_REG(CNPL, SW_IF_SW_PL_RSLT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_pl_rd: reg = &RU_REG(CNPL, SW_IF_SW_PL_RD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_cnt_rd: reg = &RU_REG(CNPL, SW_IF_SW_CNT_RD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_ovflw_f_rd_used: reg = &RU_REG(CNPL, SW_IF_SW_OVFLW_F_RD_USED); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_ovflw_f_pop: reg = &RU_REG(CNPL, SW_IF_SW_OVFLW_F_POP); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_ovflw_f_thr: reg = &RU_REG(CNPL, SW_IF_SW_OVFLW_F_THR); blk = &RU_BLK(CNPL); break;
    case bdmf_address_sw_if_sw_ovflw_f_full_mod: reg = &RU_REG(CNPL, SW_IF_SW_OVFLW_F_FULL_MOD); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_arb_prm: reg = &RU_REG(CNPL, MISC_ARB_PRM); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_col_awr_en: reg = &RU_REG(CNPL, MISC_COL_AWR_EN); blk = &RU_BLK(CNPL); break;
    case bdmf_address_misc_mem_init: reg = &RU_REG(CNPL, MISC_MEM_INIT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_eng_cmds: reg = &RU_REG(CNPL, PM_COUNTERS_ENG_CMDS); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_cmd_wait: reg = &RU_REG(CNPL, PM_COUNTERS_CMD_WAIT); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_pl_up_err: reg = &RU_REG(CNPL, PM_COUNTERS_PL_UP_ERR); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_tot_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_TOT_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_gnt_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_GNT_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_arb_cyc: reg = &RU_REG(CNPL, PM_COUNTERS_ARB_CYC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_err_bad_entry: reg = &RU_REG(CNPL, PM_COUNTERS_ERR_BAD_ENTRY); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_ovflw_f_push_err: reg = &RU_REG(CNPL, PM_COUNTERS_OVFLW_F_PUSH_ERR); blk = &RU_BLK(CNPL); break;
    case bdmf_address_pm_counters_gen_cfg: reg = &RU_REG(CNPL, PM_COUNTERS_GEN_CFG); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_dbgsel: reg = &RU_REG(CNPL, DEBUG_DBGSEL); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_dbgbus: reg = &RU_REG(CNPL, DEBUG_DBGBUS); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_req_vec: reg = &RU_REG(CNPL, DEBUG_REQ_VEC); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_err_bad_entry_data: reg = &RU_REG(CNPL, DEBUG_ERR_BAD_ENTRY_DATA); blk = &RU_BLK(CNPL); break;
    case bdmf_address_debug_pol_up_st: reg = &RU_REG(CNPL, DEBUG_POL_UP_ST); blk = &RU_BLK(CNPL); break;
    case bdmf_address_intr_ctrl_isr: reg = &RU_REG(CNPL, INTR_CTRL_ISR); blk = &RU_BLK(CNPL); break;
    case bdmf_address_intr_ctrl_ism: reg = &RU_REG(CNPL, INTR_CTRL_ISM); blk = &RU_BLK(CNPL); break;
    case bdmf_address_intr_ctrl_ier: reg = &RU_REG(CNPL, INTR_CTRL_IER); blk = &RU_BLK(CNPL); break;
    case bdmf_address_intr_ctrl_itr: reg = &RU_REG(CNPL, INTR_CTRL_ITR); blk = &RU_BLK(CNPL); break;
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
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cn_double", "cn_double", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cn0_byts", "cn0_byts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ba", "ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("clr", "clr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_main_memory_main_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num64_memory_b64_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_plc_prof_sel_mem_plc_prof_sel_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_plc_profiles_mem_plc_profiles_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_counters_configurations_cn_glb_ctrl[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mode", "mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_loc_prof_a[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bk_ba", "bk_ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pa_ba", "pa_ba", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("doublle", "doublle", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fc", "fc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_loc_prof_b[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pl_st", "pl_st", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pl_end", "pl_end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_loc_prof_c[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("n", "n", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_calc_type[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vec", "vec", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_pl_size_prof[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prf0", "prf0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prf1", "prf1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_configurations_per_up[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mtu", "mtu", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_fc_thr_fc_thr_l[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_policers_fc_thr_fc_thr_m[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_cmd_h[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_cmd_l[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_ovflw_f_thr[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sw_if_sw_ovflw_f_full_mod[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_arb_prm[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sw_prio", "sw_prio", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_addr_bit_sel", "mem_addr_bit_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_col_awr_en[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_mem_init[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("main_init", "main_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("plc_prof_sel_init", "plc_prof_sel_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("plc_profiles_init", "plc_profiles_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mult_match", "mult_match", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = set_counter_cfg },
            { .name = "main_memory_main_data", .val = cli_cnpl_main_memory_main_data, .parms = set_main_memory_main_data },
            { .name = "num64_memory_b64_data", .val = cli_cnpl_num64_memory_b64_data, .parms = set_num64_memory_b64_data },
            { .name = "plc_prof_sel_mem_plc_prof_sel_data", .val = cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data, .parms = set_plc_prof_sel_mem_plc_prof_sel_data },
            { .name = "plc_profiles_mem_plc_profiles_data", .val = cli_cnpl_plc_profiles_mem_plc_profiles_data, .parms = set_plc_profiles_mem_plc_profiles_data },
            { .name = "counters_configurations_cn_glb_ctrl", .val = cli_cnpl_counters_configurations_cn_glb_ctrl, .parms = set_counters_configurations_cn_glb_ctrl },
            { .name = "policers_configurations_pl_loc_prof_a", .val = cli_cnpl_policers_configurations_pl_loc_prof_a, .parms = set_policers_configurations_pl_loc_prof_a },
            { .name = "policers_configurations_pl_loc_prof_b", .val = cli_cnpl_policers_configurations_pl_loc_prof_b, .parms = set_policers_configurations_pl_loc_prof_b },
            { .name = "policers_configurations_pl_loc_prof_c", .val = cli_cnpl_policers_configurations_pl_loc_prof_c, .parms = set_policers_configurations_pl_loc_prof_c },
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = set_policers_configurations_pl_calc_type },
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = set_policers_configurations_pl_size_prof },
            { .name = "policers_configurations_per_up", .val = cli_cnpl_policers_configurations_per_up, .parms = set_policers_configurations_per_up },
            { .name = "policers_fc_thr_fc_thr_l", .val = cli_cnpl_policers_fc_thr_fc_thr_l, .parms = set_policers_fc_thr_fc_thr_l },
            { .name = "policers_fc_thr_fc_thr_m", .val = cli_cnpl_policers_fc_thr_fc_thr_m, .parms = set_policers_fc_thr_fc_thr_m },
            { .name = "sw_if_sw_cmd_h", .val = cli_cnpl_sw_if_sw_cmd_h, .parms = set_sw_if_sw_cmd_h },
            { .name = "sw_if_sw_cmd_l", .val = cli_cnpl_sw_if_sw_cmd_l, .parms = set_sw_if_sw_cmd_l },
            { .name = "sw_if_sw_ovflw_f_thr", .val = cli_cnpl_sw_if_sw_ovflw_f_thr, .parms = set_sw_if_sw_ovflw_f_thr },
            { .name = "sw_if_sw_ovflw_f_full_mod", .val = cli_cnpl_sw_if_sw_ovflw_f_full_mod, .parms = set_sw_if_sw_ovflw_f_full_mod },
            { .name = "misc_arb_prm", .val = cli_cnpl_misc_arb_prm, .parms = set_misc_arb_prm },
            { .name = "misc_col_awr_en", .val = cli_cnpl_misc_col_awr_en, .parms = set_misc_col_awr_en },
            { .name = "misc_mem_init", .val = cli_cnpl_misc_mem_init, .parms = set_misc_mem_init },
            { .name = "intr_ctrl_isr", .val = cli_cnpl_intr_ctrl_isr, .parms = set_intr_ctrl_isr },
            { .name = "intr_ctrl_ier", .val = cli_cnpl_intr_ctrl_ier, .parms = set_intr_ctrl_ier },
            { .name = "intr_ctrl_itr", .val = cli_cnpl_intr_ctrl_itr, .parms = set_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_cnpl_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counter_cfg[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cnt_loc_profile", "cnt_loc_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_main_memory_main_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_num64_memory_b64_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_idx", "mem_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_plc_prof_sel_mem_plc_prof_sel_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_plc_profiles_mem_plc_profiles_data[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_loc_prof_a[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_loc_prof_b[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_loc_prof_c[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_calc_type[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_configurations_pl_size_prof[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_fc_thr_fc_thr_l[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_policers_fc_thr_fc_thr_m[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("conf_idx", "conf_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_sw_if_sw_pl_rd[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bucket", "bucket", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_sw_if_sw_cnt_rd[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rd_idx", "rd_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = get_counter_cfg },
            { .name = "sw_stat", .val = cli_cnpl_sw_stat, .parms = get_default },
            { .name = "main_memory_main_data", .val = cli_cnpl_main_memory_main_data, .parms = get_main_memory_main_data },
            { .name = "num64_memory_b64_data", .val = cli_cnpl_num64_memory_b64_data, .parms = get_num64_memory_b64_data },
            { .name = "plc_prof_sel_mem_plc_prof_sel_data", .val = cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data, .parms = get_plc_prof_sel_mem_plc_prof_sel_data },
            { .name = "plc_profiles_mem_plc_profiles_data", .val = cli_cnpl_plc_profiles_mem_plc_profiles_data, .parms = get_plc_profiles_mem_plc_profiles_data },
            { .name = "counters_configurations_cn_glb_ctrl", .val = cli_cnpl_counters_configurations_cn_glb_ctrl, .parms = get_default },
            { .name = "policers_configurations_pl_loc_prof_a", .val = cli_cnpl_policers_configurations_pl_loc_prof_a, .parms = get_policers_configurations_pl_loc_prof_a },
            { .name = "policers_configurations_pl_loc_prof_b", .val = cli_cnpl_policers_configurations_pl_loc_prof_b, .parms = get_policers_configurations_pl_loc_prof_b },
            { .name = "policers_configurations_pl_loc_prof_c", .val = cli_cnpl_policers_configurations_pl_loc_prof_c, .parms = get_policers_configurations_pl_loc_prof_c },
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = get_policers_configurations_pl_calc_type },
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = get_policers_configurations_pl_size_prof },
            { .name = "policers_configurations_per_up", .val = cli_cnpl_policers_configurations_per_up, .parms = get_default },
            { .name = "policers_fc_thr_fc_thr_l", .val = cli_cnpl_policers_fc_thr_fc_thr_l, .parms = get_policers_fc_thr_fc_thr_l },
            { .name = "policers_fc_thr_fc_thr_m", .val = cli_cnpl_policers_fc_thr_fc_thr_m, .parms = get_policers_fc_thr_fc_thr_m },
            { .name = "sw_if_sw_cmd_h", .val = cli_cnpl_sw_if_sw_cmd_h, .parms = get_default },
            { .name = "sw_if_sw_cmd_l", .val = cli_cnpl_sw_if_sw_cmd_l, .parms = get_default },
            { .name = "sw_if_sw_pl_rslt", .val = cli_cnpl_sw_if_sw_pl_rslt, .parms = get_default },
            { .name = "sw_if_sw_pl_rd", .val = cli_cnpl_sw_if_sw_pl_rd, .parms = get_sw_if_sw_pl_rd },
            { .name = "sw_if_sw_cnt_rd", .val = cli_cnpl_sw_if_sw_cnt_rd, .parms = get_sw_if_sw_cnt_rd },
            { .name = "sw_if_sw_ovflw_f_rd_used", .val = cli_cnpl_sw_if_sw_ovflw_f_rd_used, .parms = get_default },
            { .name = "sw_if_sw_ovflw_f_pop", .val = cli_cnpl_sw_if_sw_ovflw_f_pop, .parms = get_default },
            { .name = "sw_if_sw_ovflw_f_thr", .val = cli_cnpl_sw_if_sw_ovflw_f_thr, .parms = get_default },
            { .name = "sw_if_sw_ovflw_f_full_mod", .val = cli_cnpl_sw_if_sw_ovflw_f_full_mod, .parms = get_default },
            { .name = "misc_arb_prm", .val = cli_cnpl_misc_arb_prm, .parms = get_default },
            { .name = "misc_col_awr_en", .val = cli_cnpl_misc_col_awr_en, .parms = get_default },
            { .name = "misc_mem_init", .val = cli_cnpl_misc_mem_init, .parms = get_default },
            { .name = "pm_counters_err_bad_entry", .val = cli_cnpl_pm_counters_err_bad_entry, .parms = get_default },
            { .name = "pm_counters_ovflw_f_push_err", .val = cli_cnpl_pm_counters_ovflw_f_push_err, .parms = get_default },
            { .name = "debug_err_bad_entry_data", .val = cli_cnpl_debug_err_bad_entry_data, .parms = get_default },
            { .name = "intr_ctrl_isr", .val = cli_cnpl_intr_ctrl_isr, .parms = get_default },
            { .name = "intr_ctrl_ism", .val = cli_cnpl_intr_ctrl_ism, .parms = get_default },
            { .name = "intr_ctrl_ier", .val = cli_cnpl_intr_ctrl_ier, .parms = get_default },
            { .name = "intr_ctrl_itr", .val = cli_cnpl_intr_ctrl_itr, .parms = get_default },
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
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("cnpl_id", "cnpl_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "counter_cfg", .val = cli_cnpl_counter_cfg, .parms = ext_test_default},
            { .name = "main_memory_main_data", .val = cli_cnpl_main_memory_main_data, .parms = ext_test_default},
            { .name = "num64_memory_b64_data", .val = cli_cnpl_num64_memory_b64_data, .parms = ext_test_default},
            { .name = "plc_prof_sel_mem_plc_prof_sel_data", .val = cli_cnpl_plc_prof_sel_mem_plc_prof_sel_data, .parms = ext_test_default},
            { .name = "plc_profiles_mem_plc_profiles_data", .val = cli_cnpl_plc_profiles_mem_plc_profiles_data, .parms = ext_test_default},
            { .name = "policers_configurations_pl_loc_prof_a", .val = cli_cnpl_policers_configurations_pl_loc_prof_a, .parms = ext_test_default},
            { .name = "policers_configurations_pl_loc_prof_b", .val = cli_cnpl_policers_configurations_pl_loc_prof_b, .parms = ext_test_default},
            { .name = "policers_configurations_pl_loc_prof_c", .val = cli_cnpl_policers_configurations_pl_loc_prof_c, .parms = ext_test_default},
            { .name = "policers_configurations_pl_calc_type", .val = cli_cnpl_policers_configurations_pl_calc_type, .parms = ext_test_default},
            { .name = "policers_configurations_pl_size_prof", .val = cli_cnpl_policers_configurations_pl_size_prof, .parms = ext_test_default},
            { .name = "policers_fc_thr_fc_thr_l", .val = cli_cnpl_policers_fc_thr_fc_thr_l, .parms = ext_test_default},
            { .name = "policers_fc_thr_fc_thr_m", .val = cli_cnpl_policers_fc_thr_fc_thr_m, .parms = ext_test_default},
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
            { .name = "MAIN_MEMORY_MAIN_DATA", .val = bdmf_address_main_memory_main_data },
            { .name = "NUM64_MEMORY_B64_DATA", .val = bdmf_address_num64_memory_b64_data },
            { .name = "PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA", .val = bdmf_address_plc_prof_sel_mem_plc_prof_sel_data },
            { .name = "PLC_PROFILES_MEM_PLC_PROFILES_DATA", .val = bdmf_address_plc_profiles_mem_plc_profiles_data },
            { .name = "COUNTERS_CONFIGURATIONS_CN_LOC_PROF", .val = bdmf_address_counters_configurations_cn_loc_prof },
            { .name = "COUNTERS_CONFIGURATIONS_CN_GLB_CTRL", .val = bdmf_address_counters_configurations_cn_glb_ctrl },
            { .name = "POLICERS_CONFIGURATIONS_PL_LOC_PROF_A", .val = bdmf_address_policers_configurations_pl_loc_prof_a },
            { .name = "POLICERS_CONFIGURATIONS_PL_LOC_PROF_B", .val = bdmf_address_policers_configurations_pl_loc_prof_b },
            { .name = "POLICERS_CONFIGURATIONS_PL_LOC_PROF_C", .val = bdmf_address_policers_configurations_pl_loc_prof_c },
            { .name = "POLICERS_CONFIGURATIONS_PL_CALC_TYPE", .val = bdmf_address_policers_configurations_pl_calc_type },
            { .name = "POLICERS_CONFIGURATIONS_PL_SIZE_PROF", .val = bdmf_address_policers_configurations_pl_size_prof },
            { .name = "POLICERS_CONFIGURATIONS_PER_UP", .val = bdmf_address_policers_configurations_per_up },
            { .name = "POLICERS_FC_THR_FC_THR_L", .val = bdmf_address_policers_fc_thr_fc_thr_l },
            { .name = "POLICERS_FC_THR_FC_THR_M", .val = bdmf_address_policers_fc_thr_fc_thr_m },
            { .name = "SW_IF_SW_CMD_H", .val = bdmf_address_sw_if_sw_cmd_h },
            { .name = "SW_IF_SW_CMD_L", .val = bdmf_address_sw_if_sw_cmd_l },
            { .name = "SW_IF_SW_STAT", .val = bdmf_address_sw_if_sw_stat },
            { .name = "SW_IF_SW_PL_RSLT", .val = bdmf_address_sw_if_sw_pl_rslt },
            { .name = "SW_IF_SW_PL_RD", .val = bdmf_address_sw_if_sw_pl_rd },
            { .name = "SW_IF_SW_CNT_RD", .val = bdmf_address_sw_if_sw_cnt_rd },
            { .name = "SW_IF_SW_OVFLW_F_RD_USED", .val = bdmf_address_sw_if_sw_ovflw_f_rd_used },
            { .name = "SW_IF_SW_OVFLW_F_POP", .val = bdmf_address_sw_if_sw_ovflw_f_pop },
            { .name = "SW_IF_SW_OVFLW_F_THR", .val = bdmf_address_sw_if_sw_ovflw_f_thr },
            { .name = "SW_IF_SW_OVFLW_F_FULL_MOD", .val = bdmf_address_sw_if_sw_ovflw_f_full_mod },
            { .name = "MISC_ARB_PRM", .val = bdmf_address_misc_arb_prm },
            { .name = "MISC_COL_AWR_EN", .val = bdmf_address_misc_col_awr_en },
            { .name = "MISC_MEM_INIT", .val = bdmf_address_misc_mem_init },
            { .name = "PM_COUNTERS_ENG_CMDS", .val = bdmf_address_pm_counters_eng_cmds },
            { .name = "PM_COUNTERS_CMD_WAIT", .val = bdmf_address_pm_counters_cmd_wait },
            { .name = "PM_COUNTERS_PL_UP_ERR", .val = bdmf_address_pm_counters_pl_up_err },
            { .name = "PM_COUNTERS_TOT_CYC", .val = bdmf_address_pm_counters_tot_cyc },
            { .name = "PM_COUNTERS_GNT_CYC", .val = bdmf_address_pm_counters_gnt_cyc },
            { .name = "PM_COUNTERS_ARB_CYC", .val = bdmf_address_pm_counters_arb_cyc },
            { .name = "PM_COUNTERS_ERR_BAD_ENTRY", .val = bdmf_address_pm_counters_err_bad_entry },
            { .name = "PM_COUNTERS_OVFLW_F_PUSH_ERR", .val = bdmf_address_pm_counters_ovflw_f_push_err },
            { .name = "PM_COUNTERS_GEN_CFG", .val = bdmf_address_pm_counters_gen_cfg },
            { .name = "DEBUG_DBGSEL", .val = bdmf_address_debug_dbgsel },
            { .name = "DEBUG_DBGBUS", .val = bdmf_address_debug_dbgbus },
            { .name = "DEBUG_REQ_VEC", .val = bdmf_address_debug_req_vec },
            { .name = "DEBUG_ERR_BAD_ENTRY_DATA", .val = bdmf_address_debug_err_bad_entry_data },
            { .name = "DEBUG_POL_UP_ST", .val = bdmf_address_debug_pol_up_st },
            { .name = "INTR_CTRL_ISR", .val = bdmf_address_intr_ctrl_isr },
            { .name = "INTR_CTRL_ISM", .val = bdmf_address_intr_ctrl_ism },
            { .name = "INTR_CTRL_IER", .val = bdmf_address_intr_ctrl_ier },
            { .name = "INTR_CTRL_ITR", .val = bdmf_address_intr_ctrl_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_cnpl_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "cnpl_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
