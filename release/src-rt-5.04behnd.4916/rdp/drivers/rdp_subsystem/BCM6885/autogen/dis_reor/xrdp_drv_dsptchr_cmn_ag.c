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
#include "xrdp_drv_dsptchr_cmn_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t base_add, uint16_t offset_add)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, BASE_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add, base_add);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, OFFSET_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add, offset_add);

    RU_REG_RAM_WRITE(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t *base_add, uint16_t *offset_add)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add;

#ifdef VALIDATE_PARMS
    if (!base_add || !offset_add)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add);

    *base_add = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, BASE_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add);
    *offset_add = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD, OFFSET_ADD, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(uint8_t dsptchr_cmn_id, uint32_t idx, const dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof *task_arb_reord_task_arb_queue_mapping_pd_rnr_prof)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = 0;

#ifdef VALIDATE_PARMS
    if(!task_arb_reord_task_arb_queue_mapping_pd_rnr_prof)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 16) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_1 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_2 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_1 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_2 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_1 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_2 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_1 >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_2 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_0_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_1);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_0_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_2);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_1_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_1);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_1_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_2);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_2_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_1);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_2_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_2);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_3_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_1);
    reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_3_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_2);

    RU_REG_RAM_WRITE(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(uint8_t dsptchr_cmn_id, uint32_t idx, dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof *task_arb_reord_task_arb_queue_mapping_pd_rnr_prof)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof;

#ifdef VALIDATE_PARMS
    if (!task_arb_reord_task_arb_queue_mapping_pd_rnr_prof)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_0_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_0_2 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_0_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_1_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_1_2 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_1_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_2_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_2_2 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_2_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_3_1, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
    task_arb_reord_task_arb_queue_mapping_pd_rnr_prof->rnr_prof_3_2 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF, RNR_PROF_3_2, reg_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel *task_arb_reord_task_arb_queue_mapping_rg_prof_sel)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = 0;

#ifdef VALIDATE_PARMS
    if(!task_arb_reord_task_arb_queue_mapping_rg_prof_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel0 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel1 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel2 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel3 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel4 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel5 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel6 >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel7 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL0, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel0);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL1, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel1);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL2, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel2);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL3, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel3);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL4, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel4);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL5, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel5);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL6, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel6);
    reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL7, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel7);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel *task_arb_reord_task_arb_queue_mapping_rg_prof_sel)
{
    uint32_t reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel;

#ifdef VALIDATE_PARMS
    if (!task_arb_reord_task_arb_queue_mapping_rg_prof_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL0, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL1, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel2 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL2, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel3 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL3, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel4 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL4, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel5 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL5, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel6 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL6, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel->rnr_g_sel7 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL, RNR_G_SEL7, reg_task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (src_id >= _9BITS_MAX_VAL_) ||
       (dst_id_ovride >= _9BITS_MAX_VAL_) ||
       (ovride_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, SRC_ID, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg, src_id);
    reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, DST_ID_OVRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg, dst_id_ovride);
    reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, OVRIDE_EN, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg, ovride_en);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg;

#ifdef VALIDATE_PARMS
    if (!src_id || !dst_id_ovride || !ovride_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg);

    *src_id = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, SRC_ID, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg);
    *dst_id_ovride = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, DST_ID_OVRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg);
    *ovride_en = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG, OVRIDE_EN, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (route_override >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt, route_override);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt;

#ifdef VALIDATE_PARMS
    if (!route_override)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt);

    *route_override = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (src_id >= _9BITS_MAX_VAL_) ||
       (dst_id_ovride >= _9BITS_MAX_VAL_) ||
       (ovride_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, SRC_ID, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg, src_id);
    reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, DST_ID_OVRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg, dst_id_ovride);
    reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, OVRIDE_EN, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg, ovride_en);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg;

#ifdef VALIDATE_PARMS
    if (!src_id || !dst_id_ovride || !ovride_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg);

    *src_id = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, SRC_ID, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg);
    *dst_id_ovride = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, DST_ID_OVRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg);
    *ovride_en = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG, OVRIDE_EN, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (route_override >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt, route_override);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override)
{
    uint32_t reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt;

#ifdef VALIDATE_PARMS
    if (!route_override)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt);

    *route_override = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(uint8_t dsptchr_cmn_id, bdmf_boolean lb_mode, uint8_t sp_thrshld)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_lb_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (lb_mode >= _1BITS_MAX_VAL_) ||
       (sp_thrshld >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_load_balancing_lb_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, LB_MODE, reg_task_arb_reord_task_arb_load_balancing_lb_cfg, lb_mode);
    reg_task_arb_reord_task_arb_load_balancing_lb_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, SP_THRSHLD, reg_task_arb_reord_task_arb_load_balancing_lb_cfg, sp_thrshld);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, reg_task_arb_reord_task_arb_load_balancing_lb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_get(uint8_t dsptchr_cmn_id, bdmf_boolean *lb_mode, uint8_t *sp_thrshld)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_lb_cfg;

#ifdef VALIDATE_PARMS
    if (!lb_mode || !sp_thrshld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, reg_task_arb_reord_task_arb_load_balancing_lb_cfg);

    *lb_mode = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, LB_MODE, reg_task_arb_reord_task_arb_load_balancing_lb_cfg);
    *sp_thrshld = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG, SP_THRSHLD, reg_task_arb_reord_task_arb_load_balancing_lb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1;

#ifdef VALIDATE_PARMS
    if (!tsk_cnt_rg_0 || !tsk_cnt_rg_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_0_1, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1);

    *tsk_cnt_rg_0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_0_1, TSK_CNT_RG_0, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1);
    *tsk_cnt_rg_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_0_1, TSK_CNT_RG_1, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3;

#ifdef VALIDATE_PARMS
    if (!tsk_cnt_rg_0 || !tsk_cnt_rg_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_2_3, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3);

    *tsk_cnt_rg_0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_2_3, TSK_CNT_RG_0, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3);
    *tsk_cnt_rg_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_2_3, TSK_CNT_RG_1, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5;

#ifdef VALIDATE_PARMS
    if (!tsk_cnt_rg_0 || !tsk_cnt_rg_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_4_5, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5);

    *tsk_cnt_rg_0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_4_5, TSK_CNT_RG_0, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5);
    *tsk_cnt_rg_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_4_5, TSK_CNT_RG_1, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7;

#ifdef VALIDATE_PARMS
    if (!tsk_cnt_rg_0 || !tsk_cnt_rg_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_6_7, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7);

    *tsk_cnt_rg_0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_6_7, TSK_CNT_RG_0, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7);
    *tsk_cnt_rg_1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_6_7, TSK_CNT_RG_1, reg_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t rnr0, uint16_t rnr1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_free_task = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_load_balancing_free_task = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, RNR0, reg_task_arb_reord_task_arb_load_balancing_free_task, rnr0);
    reg_task_arb_reord_task_arb_load_balancing_free_task = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, RNR1, reg_task_arb_reord_task_arb_load_balancing_free_task, rnr1);

    RU_REG_RAM_WRITE(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, reg_task_arb_reord_task_arb_load_balancing_free_task);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t *rnr0, uint16_t *rnr1)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_free_task;

#ifdef VALIDATE_PARMS
    if (!rnr0 || !rnr1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, reg_task_arb_reord_task_arb_load_balancing_free_task);

    *rnr0 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, RNR0, reg_task_arb_reord_task_arb_load_balancing_free_task);
    *rnr1 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK, RNR1, reg_task_arb_reord_task_arb_load_balancing_free_task);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis *task_arb_reord_task_arb_load_balancing_task_en_dis)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_task_en_dis = 0;

#ifdef VALIDATE_PARMS
    if(!task_arb_reord_task_arb_load_balancing_task_en_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->tsk >= _4BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->rnr >= _6BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->en_dis >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->rnr_all >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->vld >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_task_arb_load_balancing_task_en_dis->done >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, TSK, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->tsk);
    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, RNR, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->rnr);
    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, EN_DIS, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->en_dis);
    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, RNR_ALL, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->rnr_all);
    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, VLD, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->vld);
    reg_task_arb_reord_task_arb_load_balancing_task_en_dis = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, DONE, reg_task_arb_reord_task_arb_load_balancing_task_en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis->done);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis *task_arb_reord_task_arb_load_balancing_task_en_dis)
{
    uint32_t reg_task_arb_reord_task_arb_load_balancing_task_en_dis;

#ifdef VALIDATE_PARMS
    if (!task_arb_reord_task_arb_load_balancing_task_en_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);

    task_arb_reord_task_arb_load_balancing_task_en_dis->tsk = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, TSK, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);
    task_arb_reord_task_arb_load_balancing_task_en_dis->rnr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, RNR, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);
    task_arb_reord_task_arb_load_balancing_task_en_dis->en_dis = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, EN_DIS, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);
    task_arb_reord_task_arb_load_balancing_task_en_dis->rnr_all = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, RNR_ALL, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);
    task_arb_reord_task_arb_load_balancing_task_en_dis->vld = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, VLD, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);
    task_arb_reord_task_arb_load_balancing_task_en_dis->done = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS, DONE, reg_task_arb_reord_task_arb_load_balancing_task_en_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint32_t mask)
{
    uint32_t reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0 = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 256))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0 = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0, MASK, reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0, mask);

    RU_REG_RAM_WRITE(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0, reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint32_t *mask)
{
    uint32_t reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0;

#ifdef VALIDATE_PARMS
    if (!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 256))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dsptchr_cmn_id, idx, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0, reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0);

    *mask = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0, MASK, reg_task_arb_reord_task_arb_mask_msk_tsk_1023_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode *task_arb_reord_reord_chn_cfg_chn_mode)
{
    uint32_t reg_task_arb_reord_reord_chn_cfg_chn_mode = 0;

#ifdef VALIDATE_PARMS
    if(!task_arb_reord_reord_chn_cfg_chn_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn0_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn1_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn2_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn3_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn4_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn5_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn6_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn7_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn8_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn9_cfg >= _2BITS_MAX_VAL_) ||
       (task_arb_reord_reord_chn_cfg_chn_mode->chn_crdt_init >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN0_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn0_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN1_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn1_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN2_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn2_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN3_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn3_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN4_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn4_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN5_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn5_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN6_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn6_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN7_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn7_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN8_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn8_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN9_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn9_cfg);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN_CRDT_INIT, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn_crdt_init);
    reg_task_arb_reord_reord_chn_cfg_chn_mode = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN_CRDT_INIT_VAL, reg_task_arb_reord_reord_chn_cfg_chn_mode, task_arb_reord_reord_chn_cfg_chn_mode->chn_crdt_init_val);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, reg_task_arb_reord_reord_chn_cfg_chn_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode *task_arb_reord_reord_chn_cfg_chn_mode)
{
    uint32_t reg_task_arb_reord_reord_chn_cfg_chn_mode;

#ifdef VALIDATE_PARMS
    if (!task_arb_reord_reord_chn_cfg_chn_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, reg_task_arb_reord_reord_chn_cfg_chn_mode);

    task_arb_reord_reord_chn_cfg_chn_mode->chn0_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN0_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn1_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN1_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn2_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN2_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn3_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN3_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn4_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN4_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn5_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN5_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn6_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN6_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn7_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN7_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn8_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN8_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn9_cfg = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN9_CFG, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn_crdt_init = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN_CRDT_INIT, reg_task_arb_reord_reord_chn_cfg_chn_mode);
    task_arb_reord_reord_chn_cfg_chn_mode->chn_crdt_init_val = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE, CHN_CRDT_INIT_VAL, reg_task_arb_reord_reord_chn_cfg_chn_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(uint8_t dsptchr_cmn_id, bdmf_boolean dsp_half)
{
    uint32_t reg_task_arb_reord_reord_cfg_reord_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (dsp_half >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_cfg_reord_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CFG_REORD_CFG, DSP_HALF, reg_task_arb_reord_reord_cfg_reord_cfg, dsp_half);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CFG_REORD_CFG, reg_task_arb_reord_reord_cfg_reord_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_get(uint8_t dsptchr_cmn_id, bdmf_boolean *dsp_half)
{
    uint32_t reg_task_arb_reord_reord_cfg_reord_cfg;

#ifdef VALIDATE_PARMS
    if (!dsp_half)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CFG_REORD_CFG, reg_task_arb_reord_reord_cfg_reord_cfg);

    *dsp_half = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_CFG_REORD_CFG, DSP_HALF, reg_task_arb_reord_reord_cfg_reord_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_1_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (src_id >= _9BITS_MAX_VAL_) ||
       (dst_id_ovride >= _9BITS_MAX_VAL_) ||
       (ovride_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, SRC_ID, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg, src_id);
    reg_task_arb_reord_reord_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, DST_ID_OVRIDE, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg, dst_id_ovride);
    reg_task_arb_reord_reord_bb_cfg_bb_1_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, OVRIDE_EN, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg, ovride_en);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_1_cfg;

#ifdef VALIDATE_PARMS
    if (!src_id || !dst_id_ovride || !ovride_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg);

    *src_id = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, SRC_ID, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg);
    *dst_id_ovride = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, DST_ID_OVRIDE, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg);
    *ovride_en = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG, OVRIDE_EN, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (route_override >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt, route_override);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt;

#ifdef VALIDATE_PARMS
    if (!route_override)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt);

    *route_override = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_2_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (src_id >= _9BITS_MAX_VAL_) ||
       (dst_id_ovride >= _9BITS_MAX_VAL_) ||
       (ovride_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, SRC_ID, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg, src_id);
    reg_task_arb_reord_reord_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, DST_ID_OVRIDE, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg, dst_id_ovride);
    reg_task_arb_reord_reord_bb_cfg_bb_2_cfg = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, OVRIDE_EN, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg, ovride_en);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_2_cfg;

#ifdef VALIDATE_PARMS
    if (!src_id || !dst_id_ovride || !ovride_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg);

    *src_id = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, SRC_ID, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg);
    *dst_id_ovride = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, DST_ID_OVRIDE, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg);
    *ovride_en = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG, OVRIDE_EN, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (route_override >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt, route_override);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override)
{
    uint32_t reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt;

#ifdef VALIDATE_PARMS
    if (!route_override)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt);

    *route_override = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT, ROUTE_OVERRIDE, reg_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght, chn0_byte_wght);
    reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght, chn1_byte_wght);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght)
{
    uint32_t reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght;

#ifdef VALIDATE_PARMS
    if (!chn0_byte_wght || !chn1_byte_wght)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght);

    *chn0_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, CHN0_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght);
    *chn1_byte_wght = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT, CHN1_BYTE_WGHT, reg_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn0_inc_cntr, uint8_t chn1_inc_cntr, uint8_t chn2_inc_cntr, uint8_t chn3_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN0_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr, chn0_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN1_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr, chn1_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN2_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr, chn2_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN3_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr, chn3_inc_cntr);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn0_inc_cntr, uint8_t *chn1_inc_cntr, uint8_t *chn2_inc_cntr, uint8_t *chn3_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr;

#ifdef VALIDATE_PARMS
    if (!chn0_inc_cntr || !chn1_inc_cntr || !chn2_inc_cntr || !chn3_inc_cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);

    *chn0_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN0_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);
    *chn1_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN1_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);
    *chn2_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN2_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);
    *chn3_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR, CHN3_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn03_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn4_inc_cntr, uint8_t chn5_inc_cntr, uint8_t chn6_inc_cntr, uint8_t chn7_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN4_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr, chn4_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN5_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr, chn5_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN6_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr, chn6_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN7_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr, chn7_inc_cntr);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn4_inc_cntr, uint8_t *chn5_inc_cntr, uint8_t *chn6_inc_cntr, uint8_t *chn7_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr;

#ifdef VALIDATE_PARMS
    if (!chn4_inc_cntr || !chn5_inc_cntr || !chn6_inc_cntr || !chn7_inc_cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);

    *chn4_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN4_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);
    *chn5_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN5_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);
    *chn6_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN6_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);
    *chn7_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR, CHN7_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn47_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn8_inc_cntr, uint8_t chn9_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, CHN8_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr, chn8_inc_cntr);
    reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, CHN9_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr, chn9_inc_cntr);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn8_inc_cntr, uint8_t *chn9_inc_cntr)
{
    uint32_t reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr;

#ifdef VALIDATE_PARMS
    if (!chn8_inc_cntr || !chn9_inc_cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr);

    *chn8_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, CHN8_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr);
    *chn9_inc_cntr = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR, CHN9_INC_CNTR, reg_task_arb_reord_reord_xon_inc_chn89_inc_cntr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts, sn_cntr_01);
    reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts, sn_cntr_23);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts;

#ifdef VALIDATE_PARMS
    if (!sn_cntr_01 || !sn_cntr_23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts);

    *sn_cntr_01 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts);
    *sn_cntr_23 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts, sn_cntr_01);
    reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts, sn_cntr_23);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts;

#ifdef VALIDATE_PARMS
    if (!sn_cntr_01 || !sn_cntr_23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts);

    *sn_cntr_01 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts);
    *sn_cntr_23 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts, sn_cntr_01);
    reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts, sn_cntr_23);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts;

#ifdef VALIDATE_PARMS
    if (!sn_cntr_01 || !sn_cntr_23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts);

    *sn_cntr_01 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts);
    *sn_cntr_23 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts, sn_cntr_01);
    reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts, sn_cntr_23);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts;

#ifdef VALIDATE_PARMS
    if (!sn_cntr_01 || !sn_cntr_23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts);

    *sn_cntr_01 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, SN_CNTR_01, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts);
    *sn_cntr_23 = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS, SN_CNTR_23, reg_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_set(uint8_t dsptchr_cmn_id, uint8_t cntr_rst)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_rst = 0;

#ifdef VALIDATE_PARMS
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_reord_status_agg_sn_rst = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST, CNTR_RST, reg_task_arb_reord_reord_status_agg_sn_rst, cntr_rst);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST, reg_task_arb_reord_reord_status_agg_sn_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_get(uint8_t dsptchr_cmn_id, uint8_t *cntr_rst)
{
    uint32_t reg_task_arb_reord_reord_status_agg_sn_rst;

#ifdef VALIDATE_PARMS
    if (!cntr_rst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST, reg_task_arb_reord_reord_status_agg_sn_rst);

    *cntr_rst = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST, CNTR_RST, reg_task_arb_reord_reord_status_agg_sn_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts_get(uint8_t dsptchr_cmn_id, uint8_t *qm_crdt_cntr_sts)
{
    uint32_t reg_task_arb_reord_reord_status_qm_crdt_cntr_sts;

#ifdef VALIDATE_PARMS
    if (!qm_crdt_cntr_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_QM_CRDT_CNTR_STS, reg_task_arb_reord_reord_status_qm_crdt_cntr_sts);

    *qm_crdt_cntr_sts = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_QM_CRDT_CNTR_STS, QM_CRDT_CNTR_STS, reg_task_arb_reord_reord_status_qm_crdt_cntr_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts_get(uint8_t dsptchr_cmn_id, uint32_t *reord_full)
{
    uint32_t reg_task_arb_reord_reord_status_fifo_full_sts;

#ifdef VALIDATE_PARMS
    if (!reord_full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_FULL_STS, reg_task_arb_reord_reord_status_fifo_full_sts);

    *reord_full = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_FULL_STS, REORD_FULL, reg_task_arb_reord_reord_status_fifo_full_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts_get(uint8_t dsptchr_cmn_id, uint32_t *reord_empty)
{
    uint32_t reg_task_arb_reord_reord_status_fifo_empty_sts;

#ifdef VALIDATE_PARMS
    if (!reord_empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_EMPTY_STS, reg_task_arb_reord_reord_status_fifo_empty_sts);

    *reord_empty = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_EMPTY_STS, REORD_EMPTY, reg_task_arb_reord_reord_status_fifo_empty_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01_get(uint8_t dsptchr_cmn_id, uint8_t *disp0_chn0_uw, uint8_t *disp0_chn1_uw, uint8_t *disp1_chn0_uw, uint8_t *disp1_chn1_uw)
{
    uint32_t reg_task_arb_reord_reord_status_pd_fifo_used_words_01;

#ifdef VALIDATE_PARMS
    if (!disp0_chn0_uw || !disp0_chn1_uw || !disp1_chn0_uw || !disp1_chn1_uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01, reg_task_arb_reord_reord_status_pd_fifo_used_words_01);

    *disp0_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01, DISP0_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_01);
    *disp0_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01, DISP0_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_01);
    *disp1_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01, DISP1_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_01);
    *disp1_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01, DISP1_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_01);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23_get(uint8_t dsptchr_cmn_id, uint8_t *disp2_chn0_uw, uint8_t *disp2_chn1_uw, uint8_t *disp3_chn0_uw, uint8_t *disp3_chn1_uw)
{
    uint32_t reg_task_arb_reord_reord_status_pd_fifo_used_words_23;

#ifdef VALIDATE_PARMS
    if (!disp2_chn0_uw || !disp2_chn1_uw || !disp3_chn0_uw || !disp3_chn1_uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23, reg_task_arb_reord_reord_status_pd_fifo_used_words_23);

    *disp2_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23, DISP2_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_23);
    *disp2_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23, DISP2_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_23);
    *disp3_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23, DISP3_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_23);
    *disp3_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23, DISP3_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56_get(uint8_t dsptchr_cmn_id, uint8_t *disp5_chn0_uw, uint8_t *disp5_chn1_uw, uint8_t *disp6_chn0_uw, uint8_t *disp6_chn1_uw)
{
    uint32_t reg_task_arb_reord_reord_status_pd_fifo_used_words_56;

#ifdef VALIDATE_PARMS
    if (!disp5_chn0_uw || !disp5_chn1_uw || !disp6_chn0_uw || !disp6_chn1_uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56, reg_task_arb_reord_reord_status_pd_fifo_used_words_56);

    *disp5_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56, DISP5_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_56);
    *disp5_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56, DISP5_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_56);
    *disp6_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56, DISP6_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_56);
    *disp6_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56, DISP6_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_56);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78_get(uint8_t dsptchr_cmn_id, uint8_t *disp7_chn0_uw, uint8_t *disp7_chn1_uw, uint8_t *disp8_chn0_uw, uint8_t *disp8_chn1_uw)
{
    uint32_t reg_task_arb_reord_reord_status_pd_fifo_used_words_78;

#ifdef VALIDATE_PARMS
    if (!disp7_chn0_uw || !disp7_chn1_uw || !disp8_chn0_uw || !disp8_chn1_uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78, reg_task_arb_reord_reord_status_pd_fifo_used_words_78);

    *disp7_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78, DISP7_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_78);
    *disp7_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78, DISP7_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_78);
    *disp8_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78, DISP8_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_78);
    *disp8_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78, DISP8_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_78);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49_get(uint8_t dsptchr_cmn_id, uint8_t *disp4_chn0_uw, uint8_t *disp4_chn1_uw, uint8_t *disp9_chn0_uw, uint8_t *disp9_chn1_uw)
{
    uint32_t reg_task_arb_reord_reord_status_pd_fifo_used_words_49;

#ifdef VALIDATE_PARMS
    if (!disp4_chn0_uw || !disp4_chn1_uw || !disp9_chn0_uw || !disp9_chn1_uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49, reg_task_arb_reord_reord_status_pd_fifo_used_words_49);

    *disp4_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49, DISP4_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_49);
    *disp4_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49, DISP4_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_49);
    *disp9_chn0_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49, DISP9_CHN0_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_49);
    *disp9_chn1_uw = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49, DISP9_CHN1_UW, reg_task_arb_reord_reord_status_pd_fifo_used_words_49);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl *task_arb_reord_clock_gate_control_clk_gate_cntrl)
{
    uint32_t reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!task_arb_reord_clock_gate_control_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT) ||
       (task_arb_reord_clock_gate_control_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl, task_arb_reord_clock_gate_control_clk_gate_cntrl->bypass_clk_gate);
    reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, TIMER_VAL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl, task_arb_reord_clock_gate_control_clk_gate_cntrl->timer_val);
    reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl, task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_en);
    reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl, task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_intrvl);
    reg_task_arb_reord_clock_gate_control_clk_gate_cntrl = RU_FIELD_SET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl, task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl *task_arb_reord_clock_gate_control_clk_gate_cntrl)
{
    uint32_t reg_task_arb_reord_clock_gate_control_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!task_arb_reord_clock_gate_control_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((dsptchr_cmn_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);

    task_arb_reord_clock_gate_control_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);
    task_arb_reord_clock_gate_control_clk_gate_cntrl->timer_val = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, TIMER_VAL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);
    task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);
    task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);
    task_arb_reord_clock_gate_control_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(dsptchr_cmn_id, DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_task_arb_reord_clock_gate_control_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add,
    bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof,
    bdmf_address_task_arb_reord_task_arb_queue_mapping_rg_prof_sel,
    bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg,
    bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt,
    bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg,
    bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt,
    bdmf_address_task_arb_reord_task_arb_load_balancing_lb_cfg,
    bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1,
    bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3,
    bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5,
    bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7,
    bdmf_address_task_arb_reord_task_arb_load_balancing_free_task,
    bdmf_address_task_arb_reord_task_arb_load_balancing_task_en_dis,
    bdmf_address_task_arb_reord_task_arb_mask_msk_tsk_1023_0,
    bdmf_address_task_arb_reord_reord_chn_cfg_chn_mode,
    bdmf_address_task_arb_reord_reord_cfg_reord_cfg,
    bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg,
    bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt,
    bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg,
    bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght,
    bdmf_address_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght,
    bdmf_address_task_arb_reord_reord_xon_inc_chn03_inc_cntr,
    bdmf_address_task_arb_reord_reord_xon_inc_chn47_inc_cntr,
    bdmf_address_task_arb_reord_reord_xon_inc_chn89_inc_cntr,
    bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts,
    bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts,
    bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts,
    bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts,
    bdmf_address_task_arb_reord_reord_status_agg_sn_rst,
    bdmf_address_task_arb_reord_reord_status_qm_crdt_cntr_sts,
    bdmf_address_task_arb_reord_reord_status_fifo_full_sts,
    bdmf_address_task_arb_reord_reord_status_fifo_empty_sts,
    bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_01,
    bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_23,
    bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_56,
    bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_78,
    bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_49,
    bdmf_address_task_arb_reord_clock_gate_control_clk_gate_cntrl,
}
bdmf_address;

static int ag_drv_dsptchr_cmn_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof:
    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = { .rnr_prof_0_1 = parm[3].value.unumber, .rnr_prof_0_2 = parm[4].value.unumber, .rnr_prof_1_1 = parm[5].value.unumber, .rnr_prof_1_2 = parm[6].value.unumber, .rnr_prof_2_1 = parm[7].value.unumber, .rnr_prof_2_2 = parm[8].value.unumber, .rnr_prof_3_1 = parm[9].value.unumber, .rnr_prof_3_2 = parm[10].value.unumber};
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(parm[1].value.unumber, parm[2].value.unumber, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel:
    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel task_arb_reord_task_arb_queue_mapping_rg_prof_sel = { .rnr_g_sel0 = parm[2].value.unumber, .rnr_g_sel1 = parm[3].value.unumber, .rnr_g_sel2 = parm[4].value.unumber, .rnr_g_sel3 = parm[5].value.unumber, .rnr_g_sel4 = parm[6].value.unumber, .rnr_g_sel5 = parm[7].value.unumber, .rnr_g_sel6 = parm[8].value.unumber, .rnr_g_sel7 = parm[9].value.unumber};
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(parm[1].value.unumber, &task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis:
    {
        dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis task_arb_reord_task_arb_load_balancing_task_en_dis = { .tsk = parm[2].value.unumber, .rnr = parm[3].value.unumber, .en_dis = parm[4].value.unumber, .rnr_all = parm[5].value.unumber, .vld = parm[6].value.unumber, .done = parm[7].value.unumber};
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_set(parm[1].value.unumber, &task_arb_reord_task_arb_load_balancing_task_en_dis);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode:
    {
        dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode task_arb_reord_reord_chn_cfg_chn_mode = { .chn0_cfg = parm[2].value.unumber, .chn1_cfg = parm[3].value.unumber, .chn2_cfg = parm[4].value.unumber, .chn3_cfg = parm[5].value.unumber, .chn4_cfg = parm[6].value.unumber, .chn5_cfg = parm[7].value.unumber, .chn6_cfg = parm[8].value.unumber, .chn7_cfg = parm[9].value.unumber, .chn8_cfg = parm[10].value.unumber, .chn9_cfg = parm[11].value.unumber, .chn_crdt_init = parm[12].value.unumber, .chn_crdt_init_val = parm[13].value.unumber};
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(parm[1].value.unumber, &task_arb_reord_reord_chn_cfg_chn_mode);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst:
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl:
    {
        dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl task_arb_reord_clock_gate_control_clk_gate_cntrl = { .bypass_clk_gate = parm[2].value.unumber, .timer_val = parm[3].value.unumber, .keep_alive_en = parm[4].value.unumber, .keep_alive_intrvl = parm[5].value.unumber, .keep_alive_cyc = parm[6].value.unumber};
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_set(parm[1].value.unumber, &task_arb_reord_clock_gate_control_clk_gate_cntrl);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_dsptchr_cmn_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add:
    {
        uint16_t base_add;
        uint16_t offset_add;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(parm[1].value.unumber, parm[2].value.unumber, &base_add, &offset_add);
        bdmf_session_print(session, "base_add = %u = 0x%x\n", base_add, base_add);
        bdmf_session_print(session, "offset_add = %u = 0x%x\n", offset_add, offset_add);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof:
    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof task_arb_reord_task_arb_queue_mapping_pd_rnr_prof;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(parm[1].value.unumber, parm[2].value.unumber, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
        bdmf_session_print(session, "rnr_prof_0_1 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1);
        bdmf_session_print(session, "rnr_prof_0_2 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2);
        bdmf_session_print(session, "rnr_prof_1_1 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1);
        bdmf_session_print(session, "rnr_prof_1_2 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2);
        bdmf_session_print(session, "rnr_prof_2_1 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1);
        bdmf_session_print(session, "rnr_prof_2_2 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2);
        bdmf_session_print(session, "rnr_prof_3_1 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1);
        bdmf_session_print(session, "rnr_prof_3_2 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel:
    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel task_arb_reord_task_arb_queue_mapping_rg_prof_sel;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_get(parm[1].value.unumber, &task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
        bdmf_session_print(session, "rnr_g_sel0 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0);
        bdmf_session_print(session, "rnr_g_sel1 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1);
        bdmf_session_print(session, "rnr_g_sel2 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2);
        bdmf_session_print(session, "rnr_g_sel3 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3);
        bdmf_session_print(session, "rnr_g_sel4 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4);
        bdmf_session_print(session, "rnr_g_sel5 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5);
        bdmf_session_print(session, "rnr_g_sel6 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6);
        bdmf_session_print(session, "rnr_g_sel7 = %u = 0x%x\n", task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg:
    {
        uint16_t src_id;
        uint16_t dst_id_ovride;
        bdmf_boolean ovride_en;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_get(parm[1].value.unumber, &src_id, &dst_id_ovride, &ovride_en);
        bdmf_session_print(session, "src_id = %u = 0x%x\n", src_id, src_id);
        bdmf_session_print(session, "dst_id_ovride = %u = 0x%x\n", dst_id_ovride, dst_id_ovride);
        bdmf_session_print(session, "ovride_en = %u = 0x%x\n", ovride_en, ovride_en);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt:
    {
        uint32_t route_override;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_get(parm[1].value.unumber, &route_override);
        bdmf_session_print(session, "route_override = %u = 0x%x\n", route_override, route_override);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg:
    {
        uint16_t src_id;
        uint16_t dst_id_ovride;
        bdmf_boolean ovride_en;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_get(parm[1].value.unumber, &src_id, &dst_id_ovride, &ovride_en);
        bdmf_session_print(session, "src_id = %u = 0x%x\n", src_id, src_id);
        bdmf_session_print(session, "dst_id_ovride = %u = 0x%x\n", dst_id_ovride, dst_id_ovride);
        bdmf_session_print(session, "ovride_en = %u = 0x%x\n", ovride_en, ovride_en);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt:
    {
        uint32_t route_override;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_get(parm[1].value.unumber, &route_override);
        bdmf_session_print(session, "route_override = %u = 0x%x\n", route_override, route_override);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg:
    {
        bdmf_boolean lb_mode;
        uint8_t sp_thrshld;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_get(parm[1].value.unumber, &lb_mode, &sp_thrshld);
        bdmf_session_print(session, "lb_mode = %u = 0x%x\n", lb_mode, lb_mode);
        bdmf_session_print(session, "sp_thrshld = %u = 0x%x\n", sp_thrshld, sp_thrshld);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1:
    {
        uint16_t tsk_cnt_rg_0;
        uint16_t tsk_cnt_rg_1;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(parm[1].value.unumber, &tsk_cnt_rg_0, &tsk_cnt_rg_1);
        bdmf_session_print(session, "tsk_cnt_rg_0 = %u = 0x%x\n", tsk_cnt_rg_0, tsk_cnt_rg_0);
        bdmf_session_print(session, "tsk_cnt_rg_1 = %u = 0x%x\n", tsk_cnt_rg_1, tsk_cnt_rg_1);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3:
    {
        uint16_t tsk_cnt_rg_0;
        uint16_t tsk_cnt_rg_1;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(parm[1].value.unumber, &tsk_cnt_rg_0, &tsk_cnt_rg_1);
        bdmf_session_print(session, "tsk_cnt_rg_0 = %u = 0x%x\n", tsk_cnt_rg_0, tsk_cnt_rg_0);
        bdmf_session_print(session, "tsk_cnt_rg_1 = %u = 0x%x\n", tsk_cnt_rg_1, tsk_cnt_rg_1);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5:
    {
        uint16_t tsk_cnt_rg_0;
        uint16_t tsk_cnt_rg_1;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(parm[1].value.unumber, &tsk_cnt_rg_0, &tsk_cnt_rg_1);
        bdmf_session_print(session, "tsk_cnt_rg_0 = %u = 0x%x\n", tsk_cnt_rg_0, tsk_cnt_rg_0);
        bdmf_session_print(session, "tsk_cnt_rg_1 = %u = 0x%x\n", tsk_cnt_rg_1, tsk_cnt_rg_1);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7:
    {
        uint16_t tsk_cnt_rg_0;
        uint16_t tsk_cnt_rg_1;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(parm[1].value.unumber, &tsk_cnt_rg_0, &tsk_cnt_rg_1);
        bdmf_session_print(session, "tsk_cnt_rg_0 = %u = 0x%x\n", tsk_cnt_rg_0, tsk_cnt_rg_0);
        bdmf_session_print(session, "tsk_cnt_rg_1 = %u = 0x%x\n", tsk_cnt_rg_1, tsk_cnt_rg_1);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task:
    {
        uint16_t rnr0;
        uint16_t rnr1;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(parm[1].value.unumber, parm[2].value.unumber, &rnr0, &rnr1);
        bdmf_session_print(session, "rnr0 = %u = 0x%x\n", rnr0, rnr0);
        bdmf_session_print(session, "rnr1 = %u = 0x%x\n", rnr1, rnr1);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis:
    {
        dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis task_arb_reord_task_arb_load_balancing_task_en_dis;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_get(parm[1].value.unumber, &task_arb_reord_task_arb_load_balancing_task_en_dis);
        bdmf_session_print(session, "tsk = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.tsk, task_arb_reord_task_arb_load_balancing_task_en_dis.tsk);
        bdmf_session_print(session, "rnr = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.rnr, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr);
        bdmf_session_print(session, "en_dis = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis.en_dis);
        bdmf_session_print(session, "rnr_all = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.rnr_all, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr_all);
        bdmf_session_print(session, "vld = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.vld, task_arb_reord_task_arb_load_balancing_task_en_dis.vld);
        bdmf_session_print(session, "done = %u = 0x%x\n", task_arb_reord_task_arb_load_balancing_task_en_dis.done, task_arb_reord_task_arb_load_balancing_task_en_dis.done);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0:
    {
        uint32_t mask;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(parm[1].value.unumber, parm[2].value.unumber, &mask);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode:
    {
        dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode task_arb_reord_reord_chn_cfg_chn_mode;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(parm[1].value.unumber, &task_arb_reord_reord_chn_cfg_chn_mode);
        bdmf_session_print(session, "chn0_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn0_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn0_cfg);
        bdmf_session_print(session, "chn1_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn1_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn1_cfg);
        bdmf_session_print(session, "chn2_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn2_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn2_cfg);
        bdmf_session_print(session, "chn3_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn3_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn3_cfg);
        bdmf_session_print(session, "chn4_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn4_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn4_cfg);
        bdmf_session_print(session, "chn5_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn5_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn5_cfg);
        bdmf_session_print(session, "chn6_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn6_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn6_cfg);
        bdmf_session_print(session, "chn7_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn7_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn7_cfg);
        bdmf_session_print(session, "chn8_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn8_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn8_cfg);
        bdmf_session_print(session, "chn9_cfg = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn9_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn9_cfg);
        bdmf_session_print(session, "chn_crdt_init = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init);
        bdmf_session_print(session, "chn_crdt_init_val = %u = 0x%x\n", task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init_val, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init_val);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg:
    {
        bdmf_boolean dsp_half;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_get(parm[1].value.unumber, &dsp_half);
        bdmf_session_print(session, "dsp_half = %u = 0x%x\n", dsp_half, dsp_half);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg:
    {
        uint16_t src_id;
        uint16_t dst_id_ovride;
        bdmf_boolean ovride_en;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_get(parm[1].value.unumber, &src_id, &dst_id_ovride, &ovride_en);
        bdmf_session_print(session, "src_id = %u = 0x%x\n", src_id, src_id);
        bdmf_session_print(session, "dst_id_ovride = %u = 0x%x\n", dst_id_ovride, dst_id_ovride);
        bdmf_session_print(session, "ovride_en = %u = 0x%x\n", ovride_en, ovride_en);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt:
    {
        uint32_t route_override;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_get(parm[1].value.unumber, &route_override);
        bdmf_session_print(session, "route_override = %u = 0x%x\n", route_override, route_override);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg:
    {
        uint16_t src_id;
        uint16_t dst_id_ovride;
        bdmf_boolean ovride_en;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_get(parm[1].value.unumber, &src_id, &dst_id_ovride, &ovride_en);
        bdmf_session_print(session, "src_id = %u = 0x%x\n", src_id, src_id);
        bdmf_session_print(session, "dst_id_ovride = %u = 0x%x\n", dst_id_ovride, dst_id_ovride);
        bdmf_session_print(session, "ovride_en = %u = 0x%x\n", ovride_en, ovride_en);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt:
    {
        uint32_t route_override;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_get(parm[1].value.unumber, &route_override);
        bdmf_session_print(session, "route_override = %u = 0x%x\n", route_override, route_override);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght:
    {
        uint16_t chn0_byte_wght;
        uint16_t chn1_byte_wght;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_get(parm[1].value.unumber, &chn0_byte_wght, &chn1_byte_wght);
        bdmf_session_print(session, "chn0_byte_wght = %u = 0x%x\n", chn0_byte_wght, chn0_byte_wght);
        bdmf_session_print(session, "chn1_byte_wght = %u = 0x%x\n", chn1_byte_wght, chn1_byte_wght);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr:
    {
        uint8_t chn0_inc_cntr;
        uint8_t chn1_inc_cntr;
        uint8_t chn2_inc_cntr;
        uint8_t chn3_inc_cntr;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_get(parm[1].value.unumber, &chn0_inc_cntr, &chn1_inc_cntr, &chn2_inc_cntr, &chn3_inc_cntr);
        bdmf_session_print(session, "chn0_inc_cntr = %u = 0x%x\n", chn0_inc_cntr, chn0_inc_cntr);
        bdmf_session_print(session, "chn1_inc_cntr = %u = 0x%x\n", chn1_inc_cntr, chn1_inc_cntr);
        bdmf_session_print(session, "chn2_inc_cntr = %u = 0x%x\n", chn2_inc_cntr, chn2_inc_cntr);
        bdmf_session_print(session, "chn3_inc_cntr = %u = 0x%x\n", chn3_inc_cntr, chn3_inc_cntr);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr:
    {
        uint8_t chn4_inc_cntr;
        uint8_t chn5_inc_cntr;
        uint8_t chn6_inc_cntr;
        uint8_t chn7_inc_cntr;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_get(parm[1].value.unumber, &chn4_inc_cntr, &chn5_inc_cntr, &chn6_inc_cntr, &chn7_inc_cntr);
        bdmf_session_print(session, "chn4_inc_cntr = %u = 0x%x\n", chn4_inc_cntr, chn4_inc_cntr);
        bdmf_session_print(session, "chn5_inc_cntr = %u = 0x%x\n", chn5_inc_cntr, chn5_inc_cntr);
        bdmf_session_print(session, "chn6_inc_cntr = %u = 0x%x\n", chn6_inc_cntr, chn6_inc_cntr);
        bdmf_session_print(session, "chn7_inc_cntr = %u = 0x%x\n", chn7_inc_cntr, chn7_inc_cntr);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr:
    {
        uint8_t chn8_inc_cntr;
        uint8_t chn9_inc_cntr;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_get(parm[1].value.unumber, &chn8_inc_cntr, &chn9_inc_cntr);
        bdmf_session_print(session, "chn8_inc_cntr = %u = 0x%x\n", chn8_inc_cntr, chn8_inc_cntr);
        bdmf_session_print(session, "chn9_inc_cntr = %u = 0x%x\n", chn9_inc_cntr, chn9_inc_cntr);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts:
    {
        uint16_t sn_cntr_01;
        uint16_t sn_cntr_23;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_get(parm[1].value.unumber, &sn_cntr_01, &sn_cntr_23);
        bdmf_session_print(session, "sn_cntr_01 = %u = 0x%x\n", sn_cntr_01, sn_cntr_01);
        bdmf_session_print(session, "sn_cntr_23 = %u = 0x%x\n", sn_cntr_23, sn_cntr_23);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts:
    {
        uint16_t sn_cntr_01;
        uint16_t sn_cntr_23;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_get(parm[1].value.unumber, &sn_cntr_01, &sn_cntr_23);
        bdmf_session_print(session, "sn_cntr_01 = %u = 0x%x\n", sn_cntr_01, sn_cntr_01);
        bdmf_session_print(session, "sn_cntr_23 = %u = 0x%x\n", sn_cntr_23, sn_cntr_23);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts:
    {
        uint16_t sn_cntr_01;
        uint16_t sn_cntr_23;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_get(parm[1].value.unumber, &sn_cntr_01, &sn_cntr_23);
        bdmf_session_print(session, "sn_cntr_01 = %u = 0x%x\n", sn_cntr_01, sn_cntr_01);
        bdmf_session_print(session, "sn_cntr_23 = %u = 0x%x\n", sn_cntr_23, sn_cntr_23);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts:
    {
        uint16_t sn_cntr_01;
        uint16_t sn_cntr_23;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_get(parm[1].value.unumber, &sn_cntr_01, &sn_cntr_23);
        bdmf_session_print(session, "sn_cntr_01 = %u = 0x%x\n", sn_cntr_01, sn_cntr_01);
        bdmf_session_print(session, "sn_cntr_23 = %u = 0x%x\n", sn_cntr_23, sn_cntr_23);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst:
    {
        uint8_t cntr_rst;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_get(parm[1].value.unumber, &cntr_rst);
        bdmf_session_print(session, "cntr_rst = %u = 0x%x\n", cntr_rst, cntr_rst);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts:
    {
        uint8_t qm_crdt_cntr_sts;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts_get(parm[1].value.unumber, &qm_crdt_cntr_sts);
        bdmf_session_print(session, "qm_crdt_cntr_sts = %u = 0x%x\n", qm_crdt_cntr_sts, qm_crdt_cntr_sts);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts:
    {
        uint32_t reord_full;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts_get(parm[1].value.unumber, &reord_full);
        bdmf_session_print(session, "reord_full = %u = 0x%x\n", reord_full, reord_full);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts:
    {
        uint32_t reord_empty;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts_get(parm[1].value.unumber, &reord_empty);
        bdmf_session_print(session, "reord_empty = %u = 0x%x\n", reord_empty, reord_empty);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01:
    {
        uint8_t disp0_chn0_uw;
        uint8_t disp0_chn1_uw;
        uint8_t disp1_chn0_uw;
        uint8_t disp1_chn1_uw;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01_get(parm[1].value.unumber, &disp0_chn0_uw, &disp0_chn1_uw, &disp1_chn0_uw, &disp1_chn1_uw);
        bdmf_session_print(session, "disp0_chn0_uw = %u = 0x%x\n", disp0_chn0_uw, disp0_chn0_uw);
        bdmf_session_print(session, "disp0_chn1_uw = %u = 0x%x\n", disp0_chn1_uw, disp0_chn1_uw);
        bdmf_session_print(session, "disp1_chn0_uw = %u = 0x%x\n", disp1_chn0_uw, disp1_chn0_uw);
        bdmf_session_print(session, "disp1_chn1_uw = %u = 0x%x\n", disp1_chn1_uw, disp1_chn1_uw);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23:
    {
        uint8_t disp2_chn0_uw;
        uint8_t disp2_chn1_uw;
        uint8_t disp3_chn0_uw;
        uint8_t disp3_chn1_uw;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23_get(parm[1].value.unumber, &disp2_chn0_uw, &disp2_chn1_uw, &disp3_chn0_uw, &disp3_chn1_uw);
        bdmf_session_print(session, "disp2_chn0_uw = %u = 0x%x\n", disp2_chn0_uw, disp2_chn0_uw);
        bdmf_session_print(session, "disp2_chn1_uw = %u = 0x%x\n", disp2_chn1_uw, disp2_chn1_uw);
        bdmf_session_print(session, "disp3_chn0_uw = %u = 0x%x\n", disp3_chn0_uw, disp3_chn0_uw);
        bdmf_session_print(session, "disp3_chn1_uw = %u = 0x%x\n", disp3_chn1_uw, disp3_chn1_uw);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56:
    {
        uint8_t disp5_chn0_uw;
        uint8_t disp5_chn1_uw;
        uint8_t disp6_chn0_uw;
        uint8_t disp6_chn1_uw;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56_get(parm[1].value.unumber, &disp5_chn0_uw, &disp5_chn1_uw, &disp6_chn0_uw, &disp6_chn1_uw);
        bdmf_session_print(session, "disp5_chn0_uw = %u = 0x%x\n", disp5_chn0_uw, disp5_chn0_uw);
        bdmf_session_print(session, "disp5_chn1_uw = %u = 0x%x\n", disp5_chn1_uw, disp5_chn1_uw);
        bdmf_session_print(session, "disp6_chn0_uw = %u = 0x%x\n", disp6_chn0_uw, disp6_chn0_uw);
        bdmf_session_print(session, "disp6_chn1_uw = %u = 0x%x\n", disp6_chn1_uw, disp6_chn1_uw);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78:
    {
        uint8_t disp7_chn0_uw;
        uint8_t disp7_chn1_uw;
        uint8_t disp8_chn0_uw;
        uint8_t disp8_chn1_uw;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78_get(parm[1].value.unumber, &disp7_chn0_uw, &disp7_chn1_uw, &disp8_chn0_uw, &disp8_chn1_uw);
        bdmf_session_print(session, "disp7_chn0_uw = %u = 0x%x\n", disp7_chn0_uw, disp7_chn0_uw);
        bdmf_session_print(session, "disp7_chn1_uw = %u = 0x%x\n", disp7_chn1_uw, disp7_chn1_uw);
        bdmf_session_print(session, "disp8_chn0_uw = %u = 0x%x\n", disp8_chn0_uw, disp8_chn0_uw);
        bdmf_session_print(session, "disp8_chn1_uw = %u = 0x%x\n", disp8_chn1_uw, disp8_chn1_uw);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49:
    {
        uint8_t disp4_chn0_uw;
        uint8_t disp4_chn1_uw;
        uint8_t disp9_chn0_uw;
        uint8_t disp9_chn1_uw;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49_get(parm[1].value.unumber, &disp4_chn0_uw, &disp4_chn1_uw, &disp9_chn0_uw, &disp9_chn1_uw);
        bdmf_session_print(session, "disp4_chn0_uw = %u = 0x%x\n", disp4_chn0_uw, disp4_chn0_uw);
        bdmf_session_print(session, "disp4_chn1_uw = %u = 0x%x\n", disp4_chn1_uw, disp4_chn1_uw);
        bdmf_session_print(session, "disp9_chn0_uw = %u = 0x%x\n", disp9_chn0_uw, disp9_chn0_uw);
        bdmf_session_print(session, "disp9_chn1_uw = %u = 0x%x\n", disp9_chn1_uw, disp9_chn1_uw);
        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl:
    {
        dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl task_arb_reord_clock_gate_control_clk_gate_cntrl;
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_get(parm[1].value.unumber, &task_arb_reord_clock_gate_control_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", task_arb_reord_clock_gate_control_clk_gate_cntrl.bypass_clk_gate, task_arb_reord_clock_gate_control_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", task_arb_reord_clock_gate_control_clk_gate_cntrl.timer_val, task_arb_reord_clock_gate_control_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_en, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_intrvl, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_cyc, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_dsptchr_cmn_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t dsptchr_cmn_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t idx = gtmv(m, 4);
        uint16_t base_add = gtmv(m, 16);
        uint16_t offset_add = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
            base_add, offset_add);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(dsptchr_cmn_id, idx, base_add, offset_add);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(dsptchr_cmn_id, idx, &base_add, &offset_add);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                base_add, offset_add);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (base_add != gtmv(m, 16) || offset_add != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t idx = gtmv(m, 4);
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = {.rnr_prof_0_1 = gtmv(m, 4), .rnr_prof_0_2 = gtmv(m, 4), .rnr_prof_1_1 = gtmv(m, 4), .rnr_prof_1_2 = gtmv(m, 4), .rnr_prof_2_1 = gtmv(m, 4), .rnr_prof_2_2 = gtmv(m, 4), .rnr_prof_3_1 = gtmv(m, 4), .rnr_prof_3_2 = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(%u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id, idx,
            task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, 
            task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(dsptchr_cmn_id, idx, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(dsptchr_cmn_id, idx, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(%u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id, idx,
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1 != gtmv(m, 4) || task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2 != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel task_arb_reord_task_arb_queue_mapping_rg_prof_sel = {.rnr_g_sel0 = gtmv(m, 1), .rnr_g_sel1 = gtmv(m, 1), .rnr_g_sel2 = gtmv(m, 1), .rnr_g_sel3 = gtmv(m, 1), .rnr_g_sel4 = gtmv(m, 1), .rnr_g_sel5 = gtmv(m, 1), .rnr_g_sel6 = gtmv(m, 1), .rnr_g_sel7 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(%u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
            task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3, 
            task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(dsptchr_cmn_id, &task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_get(dsptchr_cmn_id, &task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_get(%u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
                task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3, 
                task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6, task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6 != gtmv(m, 1) || task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t src_id = gtmv(m, 9);
        uint16_t dst_id_ovride = gtmv(m, 9);
        bdmf_boolean ovride_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_set(%u %u %u %u)\n", dsptchr_cmn_id,
            src_id, dst_id_ovride, ovride_en);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_set(dsptchr_cmn_id, src_id, dst_id_ovride, ovride_en);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_get(dsptchr_cmn_id, &src_id, &dst_id_ovride, &ovride_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_get(%u %u %u %u)\n", dsptchr_cmn_id,
                src_id, dst_id_ovride, ovride_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (src_id != gtmv(m, 9) || dst_id_ovride != gtmv(m, 9) || ovride_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route_override = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_set(%u %u)\n", dsptchr_cmn_id,
            route_override);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_set(dsptchr_cmn_id, route_override);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_get(dsptchr_cmn_id, &route_override);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_get(%u %u)\n", dsptchr_cmn_id,
                route_override);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route_override != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t src_id = gtmv(m, 9);
        uint16_t dst_id_ovride = gtmv(m, 9);
        bdmf_boolean ovride_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_set(%u %u %u %u)\n", dsptchr_cmn_id,
            src_id, dst_id_ovride, ovride_en);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_set(dsptchr_cmn_id, src_id, dst_id_ovride, ovride_en);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_get(dsptchr_cmn_id, &src_id, &dst_id_ovride, &ovride_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_get(%u %u %u %u)\n", dsptchr_cmn_id,
                src_id, dst_id_ovride, ovride_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (src_id != gtmv(m, 9) || dst_id_ovride != gtmv(m, 9) || ovride_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route_override = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_set(%u %u)\n", dsptchr_cmn_id,
            route_override);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_set(dsptchr_cmn_id, route_override);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_get(dsptchr_cmn_id, &route_override);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_get(%u %u)\n", dsptchr_cmn_id,
                route_override);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route_override != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean lb_mode = gtmv(m, 1);
        uint8_t sp_thrshld = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(%u %u %u)\n", dsptchr_cmn_id,
            lb_mode, sp_thrshld);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(dsptchr_cmn_id, lb_mode, sp_thrshld);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_get(dsptchr_cmn_id, &lb_mode, &sp_thrshld);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_get(%u %u %u)\n", dsptchr_cmn_id,
                lb_mode, sp_thrshld);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (lb_mode != gtmv(m, 1) || sp_thrshld != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t tsk_cnt_rg_0 = gtmv(m, 12);
        uint16_t tsk_cnt_rg_1 = gtmv(m, 12);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(dsptchr_cmn_id, &tsk_cnt_rg_0, &tsk_cnt_rg_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(%u %u %u)\n", dsptchr_cmn_id,
                tsk_cnt_rg_0, tsk_cnt_rg_1);
        }
    }

    {
        uint16_t tsk_cnt_rg_0 = gtmv(m, 12);
        uint16_t tsk_cnt_rg_1 = gtmv(m, 12);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(dsptchr_cmn_id, &tsk_cnt_rg_0, &tsk_cnt_rg_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(%u %u %u)\n", dsptchr_cmn_id,
                tsk_cnt_rg_0, tsk_cnt_rg_1);
        }
    }

    {
        uint16_t tsk_cnt_rg_0 = gtmv(m, 12);
        uint16_t tsk_cnt_rg_1 = gtmv(m, 12);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(dsptchr_cmn_id, &tsk_cnt_rg_0, &tsk_cnt_rg_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(%u %u %u)\n", dsptchr_cmn_id,
                tsk_cnt_rg_0, tsk_cnt_rg_1);
        }
    }

    {
        uint16_t tsk_cnt_rg_0 = gtmv(m, 12);
        uint16_t tsk_cnt_rg_1 = gtmv(m, 12);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(dsptchr_cmn_id, &tsk_cnt_rg_0, &tsk_cnt_rg_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(%u %u %u)\n", dsptchr_cmn_id,
                tsk_cnt_rg_0, tsk_cnt_rg_1);
        }
    }

    {
        uint32_t idx = gtmv(m, 5);
        uint16_t rnr0 = gtmv(m, 16);
        uint16_t rnr1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
            rnr0, rnr1);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(dsptchr_cmn_id, idx, rnr0, rnr1);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(dsptchr_cmn_id, idx, &rnr0, &rnr1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                rnr0, rnr1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rnr0 != gtmv(m, 16) || rnr1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis task_arb_reord_task_arb_load_balancing_task_en_dis = {.tsk = gtmv(m, 4), .rnr = gtmv(m, 6), .en_dis = gtmv(m, 1), .rnr_all = gtmv(m, 1), .vld = gtmv(m, 1), .done = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_set(%u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
            task_arb_reord_task_arb_load_balancing_task_en_dis.tsk, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr, task_arb_reord_task_arb_load_balancing_task_en_dis.en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr_all, 
            task_arb_reord_task_arb_load_balancing_task_en_dis.vld, task_arb_reord_task_arb_load_balancing_task_en_dis.done);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_set(dsptchr_cmn_id, &task_arb_reord_task_arb_load_balancing_task_en_dis);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_get(dsptchr_cmn_id, &task_arb_reord_task_arb_load_balancing_task_en_dis);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_get(%u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
                task_arb_reord_task_arb_load_balancing_task_en_dis.tsk, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr, task_arb_reord_task_arb_load_balancing_task_en_dis.en_dis, task_arb_reord_task_arb_load_balancing_task_en_dis.rnr_all, 
                task_arb_reord_task_arb_load_balancing_task_en_dis.vld, task_arb_reord_task_arb_load_balancing_task_en_dis.done);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (task_arb_reord_task_arb_load_balancing_task_en_dis.tsk != gtmv(m, 4) || task_arb_reord_task_arb_load_balancing_task_en_dis.rnr != gtmv(m, 6) || task_arb_reord_task_arb_load_balancing_task_en_dis.en_dis != gtmv(m, 1) || task_arb_reord_task_arb_load_balancing_task_en_dis.rnr_all != gtmv(m, 1) || task_arb_reord_task_arb_load_balancing_task_en_dis.vld != gtmv(m, 1) || task_arb_reord_task_arb_load_balancing_task_en_dis.done != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t idx = gtmv(m, 8);
        uint32_t mask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(%u %u %u)\n", dsptchr_cmn_id, idx,
            mask);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(dsptchr_cmn_id, idx, mask);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(dsptchr_cmn_id, idx, &mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(%u %u %u)\n", dsptchr_cmn_id, idx,
                mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode task_arb_reord_reord_chn_cfg_chn_mode = {.chn0_cfg = gtmv(m, 2), .chn1_cfg = gtmv(m, 2), .chn2_cfg = gtmv(m, 2), .chn3_cfg = gtmv(m, 2), .chn4_cfg = gtmv(m, 2), .chn5_cfg = gtmv(m, 2), .chn6_cfg = gtmv(m, 2), .chn7_cfg = gtmv(m, 2), .chn8_cfg = gtmv(m, 2), .chn9_cfg = gtmv(m, 2), .chn_crdt_init = gtmv(m, 1), .chn_crdt_init_val = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(%u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
            task_arb_reord_reord_chn_cfg_chn_mode.chn0_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn1_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn2_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn3_cfg, 
            task_arb_reord_reord_chn_cfg_chn_mode.chn4_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn5_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn6_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn7_cfg, 
            task_arb_reord_reord_chn_cfg_chn_mode.chn8_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn9_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init_val);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(dsptchr_cmn_id, &task_arb_reord_reord_chn_cfg_chn_mode);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(dsptchr_cmn_id, &task_arb_reord_reord_chn_cfg_chn_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(%u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id,
                task_arb_reord_reord_chn_cfg_chn_mode.chn0_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn1_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn2_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn3_cfg, 
                task_arb_reord_reord_chn_cfg_chn_mode.chn4_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn5_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn6_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn7_cfg, 
                task_arb_reord_reord_chn_cfg_chn_mode.chn8_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn9_cfg, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init, task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (task_arb_reord_reord_chn_cfg_chn_mode.chn0_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn1_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn2_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn3_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn4_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn5_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn6_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn7_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn8_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn9_cfg != gtmv(m, 2) || task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init != gtmv(m, 1) || task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init_val != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean dsp_half = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(%u %u)\n", dsptchr_cmn_id,
            dsp_half);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(dsptchr_cmn_id, dsp_half);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_get(dsptchr_cmn_id, &dsp_half);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_get(%u %u)\n", dsptchr_cmn_id,
                dsp_half);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dsp_half != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t src_id = gtmv(m, 9);
        uint16_t dst_id_ovride = gtmv(m, 9);
        bdmf_boolean ovride_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_set(%u %u %u %u)\n", dsptchr_cmn_id,
            src_id, dst_id_ovride, ovride_en);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_set(dsptchr_cmn_id, src_id, dst_id_ovride, ovride_en);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_get(dsptchr_cmn_id, &src_id, &dst_id_ovride, &ovride_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_get(%u %u %u %u)\n", dsptchr_cmn_id,
                src_id, dst_id_ovride, ovride_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (src_id != gtmv(m, 9) || dst_id_ovride != gtmv(m, 9) || ovride_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route_override = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_set(%u %u)\n", dsptchr_cmn_id,
            route_override);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_set(dsptchr_cmn_id, route_override);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_get(dsptchr_cmn_id, &route_override);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_get(%u %u)\n", dsptchr_cmn_id,
                route_override);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route_override != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t src_id = gtmv(m, 9);
        uint16_t dst_id_ovride = gtmv(m, 9);
        bdmf_boolean ovride_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_set(%u %u %u %u)\n", dsptchr_cmn_id,
            src_id, dst_id_ovride, ovride_en);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_set(dsptchr_cmn_id, src_id, dst_id_ovride, ovride_en);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_get(dsptchr_cmn_id, &src_id, &dst_id_ovride, &ovride_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_get(%u %u %u %u)\n", dsptchr_cmn_id,
                src_id, dst_id_ovride, ovride_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (src_id != gtmv(m, 9) || dst_id_ovride != gtmv(m, 9) || ovride_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route_override = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_set(%u %u)\n", dsptchr_cmn_id,
            route_override);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_set(dsptchr_cmn_id, route_override);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_get(dsptchr_cmn_id, &route_override);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_get(%u %u)\n", dsptchr_cmn_id,
                route_override);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route_override != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t chn0_byte_wght = gtmv(m, 16);
        uint16_t chn1_byte_wght = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_set(%u %u %u)\n", dsptchr_cmn_id,
            chn0_byte_wght, chn1_byte_wght);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_set(dsptchr_cmn_id, chn0_byte_wght, chn1_byte_wght);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_get(dsptchr_cmn_id, &chn0_byte_wght, &chn1_byte_wght);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_get(%u %u %u)\n", dsptchr_cmn_id,
                chn0_byte_wght, chn1_byte_wght);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_byte_wght != gtmv(m, 16) || chn1_byte_wght != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t chn0_inc_cntr = gtmv(m, 8);
        uint8_t chn1_inc_cntr = gtmv(m, 8);
        uint8_t chn2_inc_cntr = gtmv(m, 8);
        uint8_t chn3_inc_cntr = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_set(%u %u %u %u %u)\n", dsptchr_cmn_id,
            chn0_inc_cntr, chn1_inc_cntr, chn2_inc_cntr, chn3_inc_cntr);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_set(dsptchr_cmn_id, chn0_inc_cntr, chn1_inc_cntr, chn2_inc_cntr, chn3_inc_cntr);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_get(dsptchr_cmn_id, &chn0_inc_cntr, &chn1_inc_cntr, &chn2_inc_cntr, &chn3_inc_cntr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                chn0_inc_cntr, chn1_inc_cntr, chn2_inc_cntr, chn3_inc_cntr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn0_inc_cntr != gtmv(m, 8) || chn1_inc_cntr != gtmv(m, 8) || chn2_inc_cntr != gtmv(m, 8) || chn3_inc_cntr != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t chn4_inc_cntr = gtmv(m, 8);
        uint8_t chn5_inc_cntr = gtmv(m, 8);
        uint8_t chn6_inc_cntr = gtmv(m, 8);
        uint8_t chn7_inc_cntr = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_set(%u %u %u %u %u)\n", dsptchr_cmn_id,
            chn4_inc_cntr, chn5_inc_cntr, chn6_inc_cntr, chn7_inc_cntr);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_set(dsptchr_cmn_id, chn4_inc_cntr, chn5_inc_cntr, chn6_inc_cntr, chn7_inc_cntr);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_get(dsptchr_cmn_id, &chn4_inc_cntr, &chn5_inc_cntr, &chn6_inc_cntr, &chn7_inc_cntr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                chn4_inc_cntr, chn5_inc_cntr, chn6_inc_cntr, chn7_inc_cntr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn4_inc_cntr != gtmv(m, 8) || chn5_inc_cntr != gtmv(m, 8) || chn6_inc_cntr != gtmv(m, 8) || chn7_inc_cntr != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t chn8_inc_cntr = gtmv(m, 8);
        uint8_t chn9_inc_cntr = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_set(%u %u %u)\n", dsptchr_cmn_id,
            chn8_inc_cntr, chn9_inc_cntr);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_set(dsptchr_cmn_id, chn8_inc_cntr, chn9_inc_cntr);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_get(dsptchr_cmn_id, &chn8_inc_cntr, &chn9_inc_cntr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_get(%u %u %u)\n", dsptchr_cmn_id,
                chn8_inc_cntr, chn9_inc_cntr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (chn8_inc_cntr != gtmv(m, 8) || chn9_inc_cntr != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sn_cntr_01 = gtmv(m, 16);
        uint16_t sn_cntr_23 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_set(%u %u %u)\n", dsptchr_cmn_id,
            sn_cntr_01, sn_cntr_23);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_set(dsptchr_cmn_id, sn_cntr_01, sn_cntr_23);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_get(dsptchr_cmn_id, &sn_cntr_01, &sn_cntr_23);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_get(%u %u %u)\n", dsptchr_cmn_id,
                sn_cntr_01, sn_cntr_23);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sn_cntr_01 != gtmv(m, 16) || sn_cntr_23 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sn_cntr_01 = gtmv(m, 16);
        uint16_t sn_cntr_23 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_set(%u %u %u)\n", dsptchr_cmn_id,
            sn_cntr_01, sn_cntr_23);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_set(dsptchr_cmn_id, sn_cntr_01, sn_cntr_23);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_get(dsptchr_cmn_id, &sn_cntr_01, &sn_cntr_23);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_get(%u %u %u)\n", dsptchr_cmn_id,
                sn_cntr_01, sn_cntr_23);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sn_cntr_01 != gtmv(m, 16) || sn_cntr_23 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sn_cntr_01 = gtmv(m, 16);
        uint16_t sn_cntr_23 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_set(%u %u %u)\n", dsptchr_cmn_id,
            sn_cntr_01, sn_cntr_23);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_set(dsptchr_cmn_id, sn_cntr_01, sn_cntr_23);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_get(dsptchr_cmn_id, &sn_cntr_01, &sn_cntr_23);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_get(%u %u %u)\n", dsptchr_cmn_id,
                sn_cntr_01, sn_cntr_23);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sn_cntr_01 != gtmv(m, 16) || sn_cntr_23 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sn_cntr_01 = gtmv(m, 16);
        uint16_t sn_cntr_23 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_set(%u %u %u)\n", dsptchr_cmn_id,
            sn_cntr_01, sn_cntr_23);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_set(dsptchr_cmn_id, sn_cntr_01, sn_cntr_23);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_get(dsptchr_cmn_id, &sn_cntr_01, &sn_cntr_23);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_get(%u %u %u)\n", dsptchr_cmn_id,
                sn_cntr_01, sn_cntr_23);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sn_cntr_01 != gtmv(m, 16) || sn_cntr_23 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t cntr_rst = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_set(%u %u)\n", dsptchr_cmn_id,
            cntr_rst);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_set(dsptchr_cmn_id, cntr_rst);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_get(dsptchr_cmn_id, &cntr_rst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_get(%u %u)\n", dsptchr_cmn_id,
                cntr_rst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cntr_rst != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t qm_crdt_cntr_sts = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts_get(dsptchr_cmn_id, &qm_crdt_cntr_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts_get(%u %u)\n", dsptchr_cmn_id,
                qm_crdt_cntr_sts);
        }
    }

    {
        uint32_t reord_full = gtmv(m, 20);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts_get(dsptchr_cmn_id, &reord_full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts_get(%u %u)\n", dsptchr_cmn_id,
                reord_full);
        }
    }

    {
        uint32_t reord_empty = gtmv(m, 20);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts_get(dsptchr_cmn_id, &reord_empty);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts_get(%u %u)\n", dsptchr_cmn_id,
                reord_empty);
        }
    }

    {
        uint8_t disp0_chn0_uw = gtmv(m, 8);
        uint8_t disp0_chn1_uw = gtmv(m, 8);
        uint8_t disp1_chn0_uw = gtmv(m, 8);
        uint8_t disp1_chn1_uw = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01_get(dsptchr_cmn_id, &disp0_chn0_uw, &disp0_chn1_uw, &disp1_chn0_uw, &disp1_chn1_uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                disp0_chn0_uw, disp0_chn1_uw, disp1_chn0_uw, disp1_chn1_uw);
        }
    }

    {
        uint8_t disp2_chn0_uw = gtmv(m, 8);
        uint8_t disp2_chn1_uw = gtmv(m, 8);
        uint8_t disp3_chn0_uw = gtmv(m, 8);
        uint8_t disp3_chn1_uw = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23_get(dsptchr_cmn_id, &disp2_chn0_uw, &disp2_chn1_uw, &disp3_chn0_uw, &disp3_chn1_uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                disp2_chn0_uw, disp2_chn1_uw, disp3_chn0_uw, disp3_chn1_uw);
        }
    }

    {
        uint8_t disp5_chn0_uw = gtmv(m, 8);
        uint8_t disp5_chn1_uw = gtmv(m, 8);
        uint8_t disp6_chn0_uw = gtmv(m, 8);
        uint8_t disp6_chn1_uw = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56_get(dsptchr_cmn_id, &disp5_chn0_uw, &disp5_chn1_uw, &disp6_chn0_uw, &disp6_chn1_uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                disp5_chn0_uw, disp5_chn1_uw, disp6_chn0_uw, disp6_chn1_uw);
        }
    }

    {
        uint8_t disp7_chn0_uw = gtmv(m, 8);
        uint8_t disp7_chn1_uw = gtmv(m, 8);
        uint8_t disp8_chn0_uw = gtmv(m, 8);
        uint8_t disp8_chn1_uw = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78_get(dsptchr_cmn_id, &disp7_chn0_uw, &disp7_chn1_uw, &disp8_chn0_uw, &disp8_chn1_uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                disp7_chn0_uw, disp7_chn1_uw, disp8_chn0_uw, disp8_chn1_uw);
        }
    }

    {
        uint8_t disp4_chn0_uw = gtmv(m, 8);
        uint8_t disp4_chn1_uw = gtmv(m, 8);
        uint8_t disp9_chn0_uw = gtmv(m, 8);
        uint8_t disp9_chn1_uw = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49_get(dsptchr_cmn_id, &disp4_chn0_uw, &disp4_chn1_uw, &disp9_chn0_uw, &disp9_chn1_uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49_get(%u %u %u %u %u)\n", dsptchr_cmn_id,
                disp4_chn0_uw, disp4_chn1_uw, disp9_chn0_uw, disp9_chn1_uw);
        }
    }

    {
        dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl task_arb_reord_clock_gate_control_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", dsptchr_cmn_id,
            task_arb_reord_clock_gate_control_clk_gate_cntrl.bypass_clk_gate, task_arb_reord_clock_gate_control_clk_gate_cntrl.timer_val, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_en, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_intrvl, 
            task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_set(dsptchr_cmn_id, &task_arb_reord_clock_gate_control_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_get(dsptchr_cmn_id, &task_arb_reord_clock_gate_control_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", dsptchr_cmn_id,
                task_arb_reord_clock_gate_control_clk_gate_cntrl.bypass_clk_gate, task_arb_reord_clock_gate_control_clk_gate_cntrl.timer_val, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_en, task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_intrvl, 
                task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (task_arb_reord_clock_gate_control_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || task_arb_reord_clock_gate_control_clk_gate_cntrl.timer_val != gtmv(m, 8) || task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || task_arb_reord_clock_gate_control_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
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
static int ag_drv_dsptchr_cmn_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t dsptchr_cmn_id = parm[2].value.unumber;
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
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add:
    {
        uint32_t max_idx = 16;
        uint32_t idx = gtmv(m, 4);
        uint16_t base_add = gtmv(m, 16);
        uint16_t offset_add = gtmv(m, 16);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                base_add, offset_add);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(dsptchr_cmn_id, idx, base_add, offset_add);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        base_add = gtmv(m, 16);
        offset_add = gtmv(m, 16);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                base_add, offset_add);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(dsptchr_cmn_id, idx, base_add, offset_add);
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

            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(dsptchr_cmn_id, idx, &base_add, &offset_add);

            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                base_add, offset_add);

            if (base_add != gtmv(m, 16) || offset_add != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof:
    {
        uint32_t max_idx = 16;
        uint32_t idx = gtmv(m, 4);
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof task_arb_reord_task_arb_queue_mapping_pd_rnr_prof = {
            .rnr_prof_0_1 = gtmv(m, 4), 
            .rnr_prof_0_2 = gtmv(m, 4), 
            .rnr_prof_1_1 = gtmv(m, 4), 
            .rnr_prof_1_2 = gtmv(m, 4), 
            .rnr_prof_2_1 = gtmv(m, 4), 
            .rnr_prof_2_2 = gtmv(m, 4), 
            .rnr_prof_3_1 = gtmv(m, 4), 
            .rnr_prof_3_2 = gtmv(m, 4) };

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(%u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id, idx,
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(dsptchr_cmn_id, idx, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1 = gtmv(m, 4);
        task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2 = gtmv(m, 4);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(%u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id, idx,
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(dsptchr_cmn_id, idx, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
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

            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(dsptchr_cmn_id, idx, &task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(%u %u %u %u %u %u %u %u %u %u)\n", dsptchr_cmn_id, idx,
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2, 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1, task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2);

            if (task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_1 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_0_2 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_1 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_1_2 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_1 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_2_2 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_1 != gtmv(m, 4) || 
                task_arb_reord_task_arb_queue_mapping_pd_rnr_prof.rnr_prof_3_2 != gtmv(m, 4) || 
                0)
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
        bdmf_session_print(session, "Test of task_arb_reord_task_arb_queue_mapping_pd_rnr_prof completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task:
    {
        uint32_t max_idx = 32;
        uint32_t idx = gtmv(m, 5);
        uint16_t rnr0 = gtmv(m, 16);
        uint16_t rnr1 = gtmv(m, 16);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                rnr0, rnr1);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(dsptchr_cmn_id, idx, rnr0, rnr1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        rnr0 = gtmv(m, 16);
        rnr1 = gtmv(m, 16);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                rnr0, rnr1);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(dsptchr_cmn_id, idx, rnr0, rnr1);
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

            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(dsptchr_cmn_id, idx, &rnr0, &rnr1);

            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(%u %u %u %u)\n", dsptchr_cmn_id, idx,
                rnr0, rnr1);

            if (rnr0 != gtmv(m, 16) || rnr1 != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of task_arb_reord_task_arb_load_balancing_free_task completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0:
    {
        uint32_t max_idx = 256;
        uint32_t idx = gtmv(m, 8);
        uint32_t mask = gtmv(m, 32);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(%u %u %u)\n", dsptchr_cmn_id, idx,
                mask);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(dsptchr_cmn_id, idx, mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask = gtmv(m, 32);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(%u %u %u)\n", dsptchr_cmn_id, idx,
                mask);
            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(dsptchr_cmn_id, idx, mask);
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

            ag_err = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(dsptchr_cmn_id, idx, &mask);

            bdmf_session_print(session, "ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(%u %u %u)\n", dsptchr_cmn_id, idx,
                mask);

            if (mask != gtmv(m, 32) || 0)
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
        bdmf_session_print(session, "Test of task_arb_reord_task_arb_mask_msk_tsk_1023_0 completed. Number of tested entries %u.\n", max_idx);
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
static int ag_drv_dsptchr_cmn_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_queue_mapping_rg_prof_sel: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_lb_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_0_1); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_2_3); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_4_5); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_6_7); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_free_task: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_load_balancing_task_en_dis: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_task_arb_mask_msk_tsk_1023_0: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_chn_cfg_chn_mode: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_cfg_reord_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_CFG_REORD_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_xon_inc_chn03_inc_cntr: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_xon_inc_chn47_inc_cntr: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_xon_inc_chn89_inc_cntr: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_agg_sn_rst: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_qm_crdt_cntr_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_QM_CRDT_CNTR_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_fifo_full_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_FULL_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_fifo_empty_sts: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_FIFO_EMPTY_STS); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_01: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_23: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_56: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_78: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_49: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49); blk = &RU_BLK(DSPTCHR_CMN); break;
    case bdmf_address_task_arb_reord_clock_gate_control_clk_gate_cntrl: reg = &RU_REG(DSPTCHR_CMN, TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL); blk = &RU_BLK(DSPTCHR_CMN); break;
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

bdmfmon_handle_t ag_drv_dsptchr_cmn_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "dsptchr_cmn", "dsptchr_cmn", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("base_add", "base_add", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offset_add", "offset_add", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_0_1", "rnr_prof_0_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_0_2", "rnr_prof_0_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_1_1", "rnr_prof_1_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_1_2", "rnr_prof_1_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_2_1", "rnr_prof_2_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_2_2", "rnr_prof_2_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_3_1", "rnr_prof_3_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_prof_3_2", "rnr_prof_3_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_queue_mapping_rg_prof_sel[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel0", "rnr_g_sel0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel1", "rnr_g_sel1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel2", "rnr_g_sel2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel3", "rnr_g_sel3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel4", "rnr_g_sel4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel5", "rnr_g_sel5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel6", "rnr_g_sel6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_g_sel7", "rnr_g_sel7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_bb_cfg_bb_1_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_id", "src_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dst_id_ovride", "dst_id_ovride", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovride_en", "ovride_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_override", "route_override", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_bb_cfg_bb_2_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_id", "src_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dst_id_ovride", "dst_id_ovride", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovride_en", "ovride_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_override", "route_override", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_load_balancing_lb_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lb_mode", "lb_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sp_thrshld", "sp_thrshld", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_load_balancing_free_task[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr0", "rnr0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr1", "rnr1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_load_balancing_task_en_dis[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tsk", "tsk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr", "rnr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_dis", "en_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_all", "rnr_all", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vld", "vld", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("done", "done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_task_arb_mask_msk_tsk_1023_0[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_chn_cfg_chn_mode[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_cfg", "chn0_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_cfg", "chn1_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn2_cfg", "chn2_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn3_cfg", "chn3_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn4_cfg", "chn4_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn5_cfg", "chn5_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn6_cfg", "chn6_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn7_cfg", "chn7_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn8_cfg", "chn8_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn9_cfg", "chn9_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn_crdt_init", "chn_crdt_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn_crdt_init_val", "chn_crdt_init_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_cfg_reord_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dsp_half", "dsp_half", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_bb_cfg_bb_1_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_id", "src_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dst_id_ovride", "dst_id_ovride", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovride_en", "ovride_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_override", "route_override", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_bb_cfg_bb_2_cfg[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_id", "src_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dst_id_ovride", "dst_id_ovride", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovride_en", "ovride_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_override", "route_override", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_byte_wght", "chn0_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_byte_wght", "chn1_byte_wght", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_xon_inc_chn03_inc_cntr[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn0_inc_cntr", "chn0_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn1_inc_cntr", "chn1_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn2_inc_cntr", "chn2_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn3_inc_cntr", "chn3_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_xon_inc_chn47_inc_cntr[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn4_inc_cntr", "chn4_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn5_inc_cntr", "chn5_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn6_inc_cntr", "chn6_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn7_inc_cntr", "chn7_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_xon_inc_chn89_inc_cntr[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn8_inc_cntr", "chn8_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chn9_inc_cntr", "chn9_inc_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_01", "sn_cntr_01", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_23", "sn_cntr_23", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_01", "sn_cntr_01", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_23", "sn_cntr_23", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_01", "sn_cntr_01", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_23", "sn_cntr_23", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_01", "sn_cntr_01", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sn_cntr_23", "sn_cntr_23", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_reord_status_agg_sn_rst[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cntr_rst", "cntr_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_task_arb_reord_clock_gate_control_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add, .parms = set_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add },
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, .parms = set_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof },
            { .name = "task_arb_reord_task_arb_queue_mapping_rg_prof_sel", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, .parms = set_task_arb_reord_task_arb_queue_mapping_rg_prof_sel },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_1_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg, .parms = set_task_arb_reord_task_arb_bb_cfg_bb_1_cfg },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt, .parms = set_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_2_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg, .parms = set_task_arb_reord_task_arb_bb_cfg_bb_2_cfg },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt, .parms = set_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt },
            { .name = "task_arb_reord_task_arb_load_balancing_lb_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg, .parms = set_task_arb_reord_task_arb_load_balancing_lb_cfg },
            { .name = "task_arb_reord_task_arb_load_balancing_free_task", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task, .parms = set_task_arb_reord_task_arb_load_balancing_free_task },
            { .name = "task_arb_reord_task_arb_load_balancing_task_en_dis", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis, .parms = set_task_arb_reord_task_arb_load_balancing_task_en_dis },
            { .name = "task_arb_reord_task_arb_mask_msk_tsk_1023_0", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0, .parms = set_task_arb_reord_task_arb_mask_msk_tsk_1023_0 },
            { .name = "task_arb_reord_reord_chn_cfg_chn_mode", .val = cli_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode, .parms = set_task_arb_reord_reord_chn_cfg_chn_mode },
            { .name = "task_arb_reord_reord_cfg_reord_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg, .parms = set_task_arb_reord_reord_cfg_reord_cfg },
            { .name = "task_arb_reord_reord_bb_cfg_bb_1_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg, .parms = set_task_arb_reord_reord_bb_cfg_bb_1_cfg },
            { .name = "task_arb_reord_reord_bb_cfg_bb_1_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt, .parms = set_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt },
            { .name = "task_arb_reord_reord_bb_cfg_bb_2_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg, .parms = set_task_arb_reord_reord_bb_cfg_bb_2_cfg },
            { .name = "task_arb_reord_reord_bb_cfg_bb_2_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt, .parms = set_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp0_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp1_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp2_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp3_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp4_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp5_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp6_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp7_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp8_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp9_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght, .parms = set_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght },
            { .name = "task_arb_reord_reord_xon_inc_chn03_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr, .parms = set_task_arb_reord_reord_xon_inc_chn03_inc_cntr },
            { .name = "task_arb_reord_reord_xon_inc_chn47_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr, .parms = set_task_arb_reord_reord_xon_inc_chn47_inc_cntr },
            { .name = "task_arb_reord_reord_xon_inc_chn89_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr, .parms = set_task_arb_reord_reord_xon_inc_chn89_inc_cntr },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_03_0_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts, .parms = set_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_58_0_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts, .parms = set_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_03_1_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts, .parms = set_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_58_1_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts, .parms = set_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts },
            { .name = "task_arb_reord_reord_status_agg_sn_rst", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst, .parms = set_task_arb_reord_reord_status_agg_sn_rst },
            { .name = "task_arb_reord_clock_gate_control_clk_gate_cntrl", .val = cli_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl, .parms = set_task_arb_reord_clock_gate_control_clk_gate_cntrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_dsptchr_cmn_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_task_arb_reord_task_arb_load_balancing_free_task[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_task_arb_reord_task_arb_mask_msk_tsk_1023_0[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add, .parms = get_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add },
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, .parms = get_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof },
            { .name = "task_arb_reord_task_arb_queue_mapping_rg_prof_sel", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel, .parms = get_default },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_1_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg, .parms = get_default },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt, .parms = get_default },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_2_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg, .parms = get_default },
            { .name = "task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_lb_cfg", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7, .parms = get_default },
            { .name = "task_arb_reord_task_arb_load_balancing_free_task", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task, .parms = get_task_arb_reord_task_arb_load_balancing_free_task },
            { .name = "task_arb_reord_task_arb_load_balancing_task_en_dis", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis, .parms = get_default },
            { .name = "task_arb_reord_task_arb_mask_msk_tsk_1023_0", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0, .parms = get_task_arb_reord_task_arb_mask_msk_tsk_1023_0 },
            { .name = "task_arb_reord_reord_chn_cfg_chn_mode", .val = cli_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode, .parms = get_default },
            { .name = "task_arb_reord_reord_cfg_reord_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg, .parms = get_default },
            { .name = "task_arb_reord_reord_bb_cfg_bb_1_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg, .parms = get_default },
            { .name = "task_arb_reord_reord_bb_cfg_bb_1_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt, .parms = get_default },
            { .name = "task_arb_reord_reord_bb_cfg_bb_2_cfg", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg, .parms = get_default },
            { .name = "task_arb_reord_reord_bb_cfg_bb_2_cfg_rt", .val = cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp0_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp1_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp2_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp3_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp4_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp5_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp6_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp7_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp8_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_dwrr_cfg_disp9_byte_wght", .val = cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght, .parms = get_default },
            { .name = "task_arb_reord_reord_xon_inc_chn03_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr, .parms = get_default },
            { .name = "task_arb_reord_reord_xon_inc_chn47_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr, .parms = get_default },
            { .name = "task_arb_reord_reord_xon_inc_chn89_inc_cntr", .val = cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr, .parms = get_default },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_03_0_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_58_0_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_03_1_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_agg_sn_cntr_58_1_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_agg_sn_rst", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst, .parms = get_default },
            { .name = "task_arb_reord_reord_status_qm_crdt_cntr_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_fifo_full_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_fifo_empty_sts", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts, .parms = get_default },
            { .name = "task_arb_reord_reord_status_pd_fifo_used_words_01", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01, .parms = get_default },
            { .name = "task_arb_reord_reord_status_pd_fifo_used_words_23", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23, .parms = get_default },
            { .name = "task_arb_reord_reord_status_pd_fifo_used_words_56", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56, .parms = get_default },
            { .name = "task_arb_reord_reord_status_pd_fifo_used_words_78", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78, .parms = get_default },
            { .name = "task_arb_reord_reord_status_pd_fifo_used_words_49", .val = cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49, .parms = get_default },
            { .name = "task_arb_reord_clock_gate_control_clk_gate_cntrl", .val = cli_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_dsptchr_cmn_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_dsptchr_cmn_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("dsptchr_cmn_id", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add, .parms = ext_test_default},
            { .name = "task_arb_reord_task_arb_queue_mapping_pd_rnr_prof", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof, .parms = ext_test_default},
            { .name = "task_arb_reord_task_arb_load_balancing_free_task", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task, .parms = ext_test_default},
            { .name = "task_arb_reord_task_arb_mask_msk_tsk_1023_0", .val = cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_dsptchr_cmn_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF_ADD", .val = bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add },
            { .name = "TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_PD_RNR_PROF", .val = bdmf_address_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof },
            { .name = "TASK_ARB_REORD_TASK_ARB_QUEUE_MAPPING_RG_PROF_SEL", .val = bdmf_address_task_arb_reord_task_arb_queue_mapping_rg_prof_sel },
            { .name = "TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG", .val = bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg },
            { .name = "TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_1_CFG_RT", .val = bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt },
            { .name = "TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG", .val = bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg },
            { .name = "TASK_ARB_REORD_TASK_ARB_BB_CFG_BB_2_CFG_RT", .val = bdmf_address_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_LB_CFG", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_lb_cfg },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_0_1", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1 },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_2_3", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3 },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_4_5", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5 },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_RG_AVLABL_TSK_6_7", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7 },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_free_task },
            { .name = "TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_TASK_EN_DIS", .val = bdmf_address_task_arb_reord_task_arb_load_balancing_task_en_dis },
            { .name = "TASK_ARB_REORD_TASK_ARB_MASK_MSK_TSK_1023_0", .val = bdmf_address_task_arb_reord_task_arb_mask_msk_tsk_1023_0 },
            { .name = "TASK_ARB_REORD_REORD_CHN_CFG_CHN_MODE", .val = bdmf_address_task_arb_reord_reord_chn_cfg_chn_mode },
            { .name = "TASK_ARB_REORD_REORD_CFG_REORD_CFG", .val = bdmf_address_task_arb_reord_reord_cfg_reord_cfg },
            { .name = "TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG", .val = bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg },
            { .name = "TASK_ARB_REORD_REORD_BB_CFG_BB_1_CFG_RT", .val = bdmf_address_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt },
            { .name = "TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG", .val = bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg },
            { .name = "TASK_ARB_REORD_REORD_BB_CFG_BB_2_CFG_RT", .val = bdmf_address_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP0_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP1_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP2_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP3_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP4_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP5_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP6_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP7_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP8_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_DWRR_CFG_DISP9_BYTE_WGHT", .val = bdmf_address_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght },
            { .name = "TASK_ARB_REORD_REORD_XON_INC_CHN03_INC_CNTR", .val = bdmf_address_task_arb_reord_reord_xon_inc_chn03_inc_cntr },
            { .name = "TASK_ARB_REORD_REORD_XON_INC_CHN47_INC_CNTR", .val = bdmf_address_task_arb_reord_reord_xon_inc_chn47_inc_cntr },
            { .name = "TASK_ARB_REORD_REORD_XON_INC_CHN89_INC_CNTR", .val = bdmf_address_task_arb_reord_reord_xon_inc_chn89_inc_cntr },
            { .name = "TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_0_STS", .val = bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_0_STS", .val = bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_03_1_STS", .val = bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_AGG_SN_CNTR_58_1_STS", .val = bdmf_address_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_AGG_SN_RST", .val = bdmf_address_task_arb_reord_reord_status_agg_sn_rst },
            { .name = "TASK_ARB_REORD_REORD_STATUS_QM_CRDT_CNTR_STS", .val = bdmf_address_task_arb_reord_reord_status_qm_crdt_cntr_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_FIFO_FULL_STS", .val = bdmf_address_task_arb_reord_reord_status_fifo_full_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_FIFO_EMPTY_STS", .val = bdmf_address_task_arb_reord_reord_status_fifo_empty_sts },
            { .name = "TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_01", .val = bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_01 },
            { .name = "TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_23", .val = bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_23 },
            { .name = "TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_56", .val = bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_56 },
            { .name = "TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_78", .val = bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_78 },
            { .name = "TASK_ARB_REORD_REORD_STATUS_PD_FIFO_USED_WORDS_49", .val = bdmf_address_task_arb_reord_reord_status_pd_fifo_used_words_49 },
            { .name = "TASK_ARB_REORD_CLOCK_GATE_CONTROL_CLK_GATE_CNTRL", .val = bdmf_address_task_arb_reord_clock_gate_control_clk_gate_cntrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_dsptchr_cmn_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "dsptchr_cmn_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
