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
#include "xrdp_drv_psram_ag.h"

bdmf_error_t ag_drv_psram_pm_counters_arb_get(psram_pm_counters_arb *pm_counters_arb)
{
    uint32_t reg_pm_counters_arb_comb_banks;
    uint32_t reg_pm_counters_arb_comb_4;
    uint32_t reg_pm_counters_arb_comb;
    uint32_t reg_pm_counters_arb_arb;
    uint32_t reg_pm_counters_arb_req;

#ifdef VALIDATE_PARMS
    if(!pm_counters_arb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, PM_COUNTERS_ARB_COMB_BANKS, reg_pm_counters_arb_comb_banks);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_ARB_COMB_4, reg_pm_counters_arb_comb_4);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_ARB_COMB, reg_pm_counters_arb_comb);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_ARB_ARB, reg_pm_counters_arb_arb);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_ARB_REQ, reg_pm_counters_arb_req);

    pm_counters_arb->arb_comb_banks_val = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_ARB_COMB_BANKS, VAL, reg_pm_counters_arb_comb_banks);
    pm_counters_arb->arb_comb4_val = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_ARB_COMB_4, VAL, reg_pm_counters_arb_comb_4);
    pm_counters_arb->arb_comb_val = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_ARB_COMB, VAL, reg_pm_counters_arb_comb);
    pm_counters_arb->arb_arb_val = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_ARB_ARB, VAL, reg_pm_counters_arb_arb);
    pm_counters_arb->arb_req_val = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_ARB_REQ, VAL, reg_pm_counters_arb_req);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_cnt_acc_get(uint32_t *last_acc_cnt_rd, uint32_t *last_acc_cnt_wr, uint32_t *acc_cnt_rd, uint32_t *acc_cnt_wr)
{
    uint32_t reg_pm_counters_bw_rd_cnt_last_acc;
    uint32_t reg_pm_counters_bw_wr_cnt_last_acc;
    uint32_t reg_pm_counters_bw_rd_cnt_acc;
    uint32_t reg_pm_counters_bw_wr_cnt_acc;

#ifdef VALIDATE_PARMS
    if(!last_acc_cnt_rd || !last_acc_cnt_wr || !acc_cnt_rd || !acc_cnt_wr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST_ACC, reg_pm_counters_bw_rd_cnt_last_acc);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST_ACC, reg_pm_counters_bw_wr_cnt_last_acc);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_BW_RD_CNT_ACC, reg_pm_counters_bw_rd_cnt_acc);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_BW_WR_CNT_ACC, reg_pm_counters_bw_wr_cnt_acc);

    *last_acc_cnt_rd = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST_ACC, CNT, reg_pm_counters_bw_rd_cnt_last_acc);
    *last_acc_cnt_wr = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST_ACC, CNT, reg_pm_counters_bw_wr_cnt_last_acc);
    *acc_cnt_rd = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BW_RD_CNT_ACC, CNT, reg_pm_counters_bw_rd_cnt_acc);
    *acc_cnt_wr = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BW_WR_CNT_ACC, CNT, reg_pm_counters_bw_wr_cnt_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_muen_set(const psram_pm_counters_muen *pm_counters_muen)
{
    uint32_t reg_pm_counters_bwen=0;
    uint32_t reg_pm_counters_bwcl=0;
    uint32_t reg_pm_counters_muen=0;

#ifdef VALIDATE_PARMS
    if(!pm_counters_muen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pm_counters_muen->bwcen >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cbwcen >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl0men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl1men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl2men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl3men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl4men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl5men >= _1BITS_MAX_VAL_) ||
       (pm_counters_muen->cl6men >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pm_counters_bwen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_BWEN, BWCEN, reg_pm_counters_bwen, pm_counters_muen->bwcen);
    reg_pm_counters_bwen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_BWEN, CBWCEN, reg_pm_counters_bwen, pm_counters_muen->cbwcen);
    reg_pm_counters_bwcl = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_BWCL, TW, reg_pm_counters_bwcl, pm_counters_muen->tw);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL0MEN, reg_pm_counters_muen, pm_counters_muen->cl0men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL1MEN, reg_pm_counters_muen, pm_counters_muen->cl1men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL2MEN, reg_pm_counters_muen, pm_counters_muen->cl2men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL3MEN, reg_pm_counters_muen, pm_counters_muen->cl3men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL4MEN, reg_pm_counters_muen, pm_counters_muen->cl4men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL5MEN, reg_pm_counters_muen, pm_counters_muen->cl5men);
    reg_pm_counters_muen = RU_FIELD_SET(0, PSRAM, PM_COUNTERS_MUEN, CL6MEN, reg_pm_counters_muen, pm_counters_muen->cl6men);

    RU_REG_WRITE(0, PSRAM, PM_COUNTERS_BWEN, reg_pm_counters_bwen);
    RU_REG_WRITE(0, PSRAM, PM_COUNTERS_BWCL, reg_pm_counters_bwcl);
    RU_REG_WRITE(0, PSRAM, PM_COUNTERS_MUEN, reg_pm_counters_muen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_muen_get(psram_pm_counters_muen *pm_counters_muen)
{
    uint32_t reg_pm_counters_bwen;
    uint32_t reg_pm_counters_bwcl;
    uint32_t reg_pm_counters_muen;

#ifdef VALIDATE_PARMS
    if(!pm_counters_muen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, PM_COUNTERS_BWEN, reg_pm_counters_bwen);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_BWCL, reg_pm_counters_bwcl);
    RU_REG_READ(0, PSRAM, PM_COUNTERS_MUEN, reg_pm_counters_muen);

    pm_counters_muen->bwcen = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BWEN, BWCEN, reg_pm_counters_bwen);
    pm_counters_muen->cbwcen = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BWEN, CBWCEN, reg_pm_counters_bwen);
    pm_counters_muen->tw = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_BWCL, TW, reg_pm_counters_bwcl);
    pm_counters_muen->cl0men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL0MEN, reg_pm_counters_muen);
    pm_counters_muen->cl1men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL1MEN, reg_pm_counters_muen);
    pm_counters_muen->cl2men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL2MEN, reg_pm_counters_muen);
    pm_counters_muen->cl3men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL3MEN, reg_pm_counters_muen);
    pm_counters_muen->cl4men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL4MEN, reg_pm_counters_muen);
    pm_counters_muen->cl5men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL5MEN, reg_pm_counters_muen);
    pm_counters_muen->cl6men = RU_FIELD_GET(0, PSRAM, PM_COUNTERS_MUEN, CL6MEN, reg_pm_counters_muen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_cfg_ctrl_set(const psram_cfg_ctrl *cfg_ctrl)
{
    uint32_t reg_configurations_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!cfg_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((cfg_ctrl->perm_en >= _1BITS_MAX_VAL_) ||
       (cfg_ctrl->comb_en >= _1BITS_MAX_VAL_) ||
       (cfg_ctrl->comb_full >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, PSRAM, CONFIGURATIONS_CTRL, reg_configurations_ctrl);

    reg_configurations_ctrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CTRL, PERM_EN, reg_configurations_ctrl, cfg_ctrl->perm_en);
    reg_configurations_ctrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CTRL, COMB_EN, reg_configurations_ctrl, cfg_ctrl->comb_en);
    reg_configurations_ctrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CTRL, COMB_FULL, reg_configurations_ctrl, cfg_ctrl->comb_full);

    RU_REG_WRITE(0, PSRAM, CONFIGURATIONS_CTRL, reg_configurations_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_cfg_ctrl_get(psram_cfg_ctrl *cfg_ctrl)
{
    uint32_t reg_configurations_ctrl;

#ifdef VALIDATE_PARMS
    if(!cfg_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, CONFIGURATIONS_CTRL, reg_configurations_ctrl);

    cfg_ctrl->perm_en = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CTRL, PERM_EN, reg_configurations_ctrl);
    cfg_ctrl->comb_en = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CTRL, COMB_EN, reg_configurations_ctrl);
    cfg_ctrl->comb_full = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CTRL, COMB_FULL, reg_configurations_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_memory_data_set(uint32_t psram_enrty, const psram_memory_data *memory_data)
{
#ifdef VALIDATE_PARMS
    if(!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((psram_enrty >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, psram_enrty *32 + 0, PSRAM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 1, PSRAM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 2, PSRAM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 3, PSRAM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 4, PSRAM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 5, PSRAM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 6, PSRAM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 7, PSRAM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 8, PSRAM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 9, PSRAM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 10, PSRAM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 11, PSRAM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 12, PSRAM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 13, PSRAM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 14, PSRAM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 15, PSRAM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 16, PSRAM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 17, PSRAM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 18, PSRAM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 19, PSRAM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 20, PSRAM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 21, PSRAM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 22, PSRAM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 23, PSRAM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 24, PSRAM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 25, PSRAM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 26, PSRAM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 27, PSRAM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 28, PSRAM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 29, PSRAM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 30, PSRAM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 31, PSRAM, MEMORY_DATA, memory_data->memory_data[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_memory_data_get(uint32_t psram_enrty, psram_memory_data *memory_data)
{
#ifdef VALIDATE_PARMS
    if(!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((psram_enrty >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, psram_enrty *32 + 0, PSRAM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 1, PSRAM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 2, PSRAM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 3, PSRAM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 4, PSRAM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 5, PSRAM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 6, PSRAM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 7, PSRAM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 8, PSRAM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 9, PSRAM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 10, PSRAM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 11, PSRAM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 12, PSRAM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 13, PSRAM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 14, PSRAM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 15, PSRAM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 16, PSRAM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 17, PSRAM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 18, PSRAM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 19, PSRAM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 20, PSRAM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 21, PSRAM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 22, PSRAM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 23, PSRAM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 24, PSRAM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 25, PSRAM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 26, PSRAM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 27, PSRAM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 28, PSRAM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 29, PSRAM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 30, PSRAM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 31, PSRAM, MEMORY_DATA, memory_data->memory_data[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_set(const psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl)
{
    uint32_t reg_configurations_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (configurations_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (configurations_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_configurations_clk_gate_cntrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_configurations_clk_gate_cntrl, configurations_clk_gate_cntrl->bypass_clk_gate);
    reg_configurations_clk_gate_cntrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_configurations_clk_gate_cntrl, configurations_clk_gate_cntrl->timer_val);
    reg_configurations_clk_gate_cntrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_configurations_clk_gate_cntrl, configurations_clk_gate_cntrl->keep_alive_en);
    reg_configurations_clk_gate_cntrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_configurations_clk_gate_cntrl, configurations_clk_gate_cntrl->keep_alive_intrvl);
    reg_configurations_clk_gate_cntrl = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_configurations_clk_gate_cntrl, configurations_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, reg_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_get(psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl)
{
    uint32_t reg_configurations_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, reg_configurations_clk_gate_cntrl);

    configurations_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_configurations_clk_gate_cntrl);
    configurations_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_configurations_clk_gate_cntrl);
    configurations_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_configurations_clk_gate_cntrl);
    configurations_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_configurations_clk_gate_cntrl);
    configurations_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_max_time_get(uint32_t zero, psram_pm_counters_max_time *pm_counters_max_time)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_max_time)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_acc_time_get(uint32_t zero, psram_pm_counters_acc_time *pm_counters_acc_time)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_acc_time)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_acc_req_get(uint32_t zero, psram_pm_counters_acc_req *pm_counters_acc_req)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_acc_req)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_last_acc_time_get(uint32_t zero, psram_pm_counters_last_acc_time *pm_counters_last_acc_time)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_last_acc_time)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_last_acc_req_get(uint32_t zero, psram_pm_counters_last_acc_req *pm_counters_last_acc_req)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_last_acc_req)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_get(uint32_t zero, psram_pm_counters_bw_wr_cnt *pm_counters_bw_wr_cnt)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_bw_wr_cnt)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_get(uint32_t zero, psram_pm_counters_bw_rd_cnt *pm_counters_bw_rd_cnt)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_bw_rd_cnt)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_last_get(uint32_t zero, psram_pm_counters_bw_wr_cnt_last *pm_counters_bw_wr_cnt_last)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_bw_wr_cnt_last)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_last_get(uint32_t zero, psram_pm_counters_bw_rd_cnt_last *pm_counters_bw_rd_cnt_last)
{
#ifdef VALIDATE_PARMS
    if(!pm_counters_bw_rd_cnt_last)
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

    RU_REG_RAM_READ(0, zero *7 + 0, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[0]);
    RU_REG_RAM_READ(0, zero *7 + 1, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[1]);
    RU_REG_RAM_READ(0, zero *7 + 2, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[2]);
    RU_REG_RAM_READ(0, zero *7 + 3, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[3]);
    RU_REG_RAM_READ(0, zero *7 + 4, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[4]);
    RU_REG_RAM_READ(0, zero *7 + 5, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[5]);
    RU_REG_RAM_READ(0, zero *7 + 6, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[6]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_memory_data,
    bdmf_address_configurations_ctrl,
    bdmf_address_configurations_clk_gate_cntrl,
    bdmf_address_pm_counters_muen,
    bdmf_address_pm_counters_bwcl,
    bdmf_address_pm_counters_bwen,
    bdmf_address_pm_counters_max_time,
    bdmf_address_pm_counters_acc_time,
    bdmf_address_pm_counters_acc_req,
    bdmf_address_pm_counters_last_acc_time,
    bdmf_address_pm_counters_last_acc_req,
    bdmf_address_pm_counters_bw_wr_cnt_acc,
    bdmf_address_pm_counters_bw_rd_cnt_acc,
    bdmf_address_pm_counters_bw_wr_cnt,
    bdmf_address_pm_counters_bw_rd_cnt,
    bdmf_address_pm_counters_bw_wr_cnt_last_acc,
    bdmf_address_pm_counters_bw_rd_cnt_last_acc,
    bdmf_address_pm_counters_bw_wr_cnt_last,
    bdmf_address_pm_counters_bw_rd_cnt_last,
    bdmf_address_pm_counters_arb_req,
    bdmf_address_pm_counters_arb_arb,
    bdmf_address_pm_counters_arb_comb,
    bdmf_address_pm_counters_arb_comb_4,
    bdmf_address_pm_counters_arb_comb_banks,
}
bdmf_address;

static int bcm_psram_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_psram_pm_counters_muen:
    {
        psram_pm_counters_muen pm_counters_muen = { .bwcen=parm[1].value.unumber, .cbwcen=parm[2].value.unumber, .tw=parm[3].value.unumber, .cl0men=parm[4].value.unumber, .cl1men=parm[5].value.unumber, .cl2men=parm[6].value.unumber, .cl3men=parm[7].value.unumber, .cl4men=parm[8].value.unumber, .cl5men=parm[9].value.unumber, .cl6men=parm[10].value.unumber};
        err = ag_drv_psram_pm_counters_muen_set(&pm_counters_muen);
        break;
    }
    case cli_psram_cfg_ctrl:
    {
        psram_cfg_ctrl cfg_ctrl = { .perm_en=parm[1].value.unumber, .comb_en=parm[2].value.unumber, .comb_full=parm[3].value.unumber};
        err = ag_drv_psram_cfg_ctrl_set(&cfg_ctrl);
        break;
    }
    case cli_psram_memory_data:
    {
        psram_memory_data memory_data = { .memory_data = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber}};
        err = ag_drv_psram_memory_data_set(parm[1].value.unumber, &memory_data);
        break;
    }
    case cli_psram_configurations_clk_gate_cntrl:
    {
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl = { .bypass_clk_gate=parm[1].value.unumber, .timer_val=parm[2].value.unumber, .keep_alive_en=parm[3].value.unumber, .keep_alive_intrvl=parm[4].value.unumber, .keep_alive_cyc=parm[5].value.unumber};
        err = ag_drv_psram_configurations_clk_gate_cntrl_set(&configurations_clk_gate_cntrl);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_psram_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_psram_pm_counters_arb:
    {
        psram_pm_counters_arb pm_counters_arb;
        err = ag_drv_psram_pm_counters_arb_get(&pm_counters_arb);
        bdmf_session_print(session, "arb_comb_banks_val = %u (0x%x)\n", pm_counters_arb.arb_comb_banks_val, pm_counters_arb.arb_comb_banks_val);
        bdmf_session_print(session, "arb_comb4_val = %u (0x%x)\n", pm_counters_arb.arb_comb4_val, pm_counters_arb.arb_comb4_val);
        bdmf_session_print(session, "arb_comb_val = %u (0x%x)\n", pm_counters_arb.arb_comb_val, pm_counters_arb.arb_comb_val);
        bdmf_session_print(session, "arb_arb_val = %u (0x%x)\n", pm_counters_arb.arb_arb_val, pm_counters_arb.arb_arb_val);
        bdmf_session_print(session, "arb_req_val = %u (0x%x)\n", pm_counters_arb.arb_req_val, pm_counters_arb.arb_req_val);
        break;
    }
    case cli_psram_pm_counters_cnt_acc:
    {
        uint32_t last_acc_cnt_rd;
        uint32_t last_acc_cnt_wr;
        uint32_t acc_cnt_rd;
        uint32_t acc_cnt_wr;
        err = ag_drv_psram_pm_counters_cnt_acc_get(&last_acc_cnt_rd, &last_acc_cnt_wr, &acc_cnt_rd, &acc_cnt_wr);
        bdmf_session_print(session, "last_acc_cnt_rd = %u (0x%x)\n", last_acc_cnt_rd, last_acc_cnt_rd);
        bdmf_session_print(session, "last_acc_cnt_wr = %u (0x%x)\n", last_acc_cnt_wr, last_acc_cnt_wr);
        bdmf_session_print(session, "acc_cnt_rd = %u (0x%x)\n", acc_cnt_rd, acc_cnt_rd);
        bdmf_session_print(session, "acc_cnt_wr = %u (0x%x)\n", acc_cnt_wr, acc_cnt_wr);
        break;
    }
    case cli_psram_pm_counters_muen:
    {
        psram_pm_counters_muen pm_counters_muen;
        err = ag_drv_psram_pm_counters_muen_get(&pm_counters_muen);
        bdmf_session_print(session, "bwcen = %u (0x%x)\n", pm_counters_muen.bwcen, pm_counters_muen.bwcen);
        bdmf_session_print(session, "cbwcen = %u (0x%x)\n", pm_counters_muen.cbwcen, pm_counters_muen.cbwcen);
        bdmf_session_print(session, "tw = %u (0x%x)\n", pm_counters_muen.tw, pm_counters_muen.tw);
        bdmf_session_print(session, "cl0men = %u (0x%x)\n", pm_counters_muen.cl0men, pm_counters_muen.cl0men);
        bdmf_session_print(session, "cl1men = %u (0x%x)\n", pm_counters_muen.cl1men, pm_counters_muen.cl1men);
        bdmf_session_print(session, "cl2men = %u (0x%x)\n", pm_counters_muen.cl2men, pm_counters_muen.cl2men);
        bdmf_session_print(session, "cl3men = %u (0x%x)\n", pm_counters_muen.cl3men, pm_counters_muen.cl3men);
        bdmf_session_print(session, "cl4men = %u (0x%x)\n", pm_counters_muen.cl4men, pm_counters_muen.cl4men);
        bdmf_session_print(session, "cl5men = %u (0x%x)\n", pm_counters_muen.cl5men, pm_counters_muen.cl5men);
        bdmf_session_print(session, "cl6men = %u (0x%x)\n", pm_counters_muen.cl6men, pm_counters_muen.cl6men);
        break;
    }
    case cli_psram_cfg_ctrl:
    {
        psram_cfg_ctrl cfg_ctrl;
        err = ag_drv_psram_cfg_ctrl_get(&cfg_ctrl);
        bdmf_session_print(session, "perm_en = %u (0x%x)\n", cfg_ctrl.perm_en, cfg_ctrl.perm_en);
        bdmf_session_print(session, "comb_en = %u (0x%x)\n", cfg_ctrl.comb_en, cfg_ctrl.comb_en);
        bdmf_session_print(session, "comb_full = %u (0x%x)\n", cfg_ctrl.comb_full, cfg_ctrl.comb_full);
        break;
    }
    case cli_psram_memory_data:
    {
        psram_memory_data memory_data;
        err = ag_drv_psram_memory_data_get(parm[1].value.unumber, &memory_data);
        bdmf_session_print(session, "memory_data[0] = %u (0x%x)\n", memory_data.memory_data[0], memory_data.memory_data[0]);
        bdmf_session_print(session, "memory_data[1] = %u (0x%x)\n", memory_data.memory_data[1], memory_data.memory_data[1]);
        bdmf_session_print(session, "memory_data[2] = %u (0x%x)\n", memory_data.memory_data[2], memory_data.memory_data[2]);
        bdmf_session_print(session, "memory_data[3] = %u (0x%x)\n", memory_data.memory_data[3], memory_data.memory_data[3]);
        bdmf_session_print(session, "memory_data[4] = %u (0x%x)\n", memory_data.memory_data[4], memory_data.memory_data[4]);
        bdmf_session_print(session, "memory_data[5] = %u (0x%x)\n", memory_data.memory_data[5], memory_data.memory_data[5]);
        bdmf_session_print(session, "memory_data[6] = %u (0x%x)\n", memory_data.memory_data[6], memory_data.memory_data[6]);
        bdmf_session_print(session, "memory_data[7] = %u (0x%x)\n", memory_data.memory_data[7], memory_data.memory_data[7]);
        bdmf_session_print(session, "memory_data[8] = %u (0x%x)\n", memory_data.memory_data[8], memory_data.memory_data[8]);
        bdmf_session_print(session, "memory_data[9] = %u (0x%x)\n", memory_data.memory_data[9], memory_data.memory_data[9]);
        bdmf_session_print(session, "memory_data[10] = %u (0x%x)\n", memory_data.memory_data[10], memory_data.memory_data[10]);
        bdmf_session_print(session, "memory_data[11] = %u (0x%x)\n", memory_data.memory_data[11], memory_data.memory_data[11]);
        bdmf_session_print(session, "memory_data[12] = %u (0x%x)\n", memory_data.memory_data[12], memory_data.memory_data[12]);
        bdmf_session_print(session, "memory_data[13] = %u (0x%x)\n", memory_data.memory_data[13], memory_data.memory_data[13]);
        bdmf_session_print(session, "memory_data[14] = %u (0x%x)\n", memory_data.memory_data[14], memory_data.memory_data[14]);
        bdmf_session_print(session, "memory_data[15] = %u (0x%x)\n", memory_data.memory_data[15], memory_data.memory_data[15]);
        bdmf_session_print(session, "memory_data[16] = %u (0x%x)\n", memory_data.memory_data[16], memory_data.memory_data[16]);
        bdmf_session_print(session, "memory_data[17] = %u (0x%x)\n", memory_data.memory_data[17], memory_data.memory_data[17]);
        bdmf_session_print(session, "memory_data[18] = %u (0x%x)\n", memory_data.memory_data[18], memory_data.memory_data[18]);
        bdmf_session_print(session, "memory_data[19] = %u (0x%x)\n", memory_data.memory_data[19], memory_data.memory_data[19]);
        bdmf_session_print(session, "memory_data[20] = %u (0x%x)\n", memory_data.memory_data[20], memory_data.memory_data[20]);
        bdmf_session_print(session, "memory_data[21] = %u (0x%x)\n", memory_data.memory_data[21], memory_data.memory_data[21]);
        bdmf_session_print(session, "memory_data[22] = %u (0x%x)\n", memory_data.memory_data[22], memory_data.memory_data[22]);
        bdmf_session_print(session, "memory_data[23] = %u (0x%x)\n", memory_data.memory_data[23], memory_data.memory_data[23]);
        bdmf_session_print(session, "memory_data[24] = %u (0x%x)\n", memory_data.memory_data[24], memory_data.memory_data[24]);
        bdmf_session_print(session, "memory_data[25] = %u (0x%x)\n", memory_data.memory_data[25], memory_data.memory_data[25]);
        bdmf_session_print(session, "memory_data[26] = %u (0x%x)\n", memory_data.memory_data[26], memory_data.memory_data[26]);
        bdmf_session_print(session, "memory_data[27] = %u (0x%x)\n", memory_data.memory_data[27], memory_data.memory_data[27]);
        bdmf_session_print(session, "memory_data[28] = %u (0x%x)\n", memory_data.memory_data[28], memory_data.memory_data[28]);
        bdmf_session_print(session, "memory_data[29] = %u (0x%x)\n", memory_data.memory_data[29], memory_data.memory_data[29]);
        bdmf_session_print(session, "memory_data[30] = %u (0x%x)\n", memory_data.memory_data[30], memory_data.memory_data[30]);
        bdmf_session_print(session, "memory_data[31] = %u (0x%x)\n", memory_data.memory_data[31], memory_data.memory_data[31]);
        break;
    }
    case cli_psram_configurations_clk_gate_cntrl:
    {
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl;
        err = ag_drv_psram_configurations_clk_gate_cntrl_get(&configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", configurations_clk_gate_cntrl.keep_alive_intrvl, configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", configurations_clk_gate_cntrl.keep_alive_cyc, configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_psram_pm_counters_max_time:
    {
        psram_pm_counters_max_time pm_counters_max_time;
        err = ag_drv_psram_pm_counters_max_time_get(parm[1].value.unumber, &pm_counters_max_time);
        bdmf_session_print(session, "pm_counters_max_time[0] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[0], pm_counters_max_time.pm_counters_max_time[0]);
        bdmf_session_print(session, "pm_counters_max_time[1] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[1], pm_counters_max_time.pm_counters_max_time[1]);
        bdmf_session_print(session, "pm_counters_max_time[2] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[2], pm_counters_max_time.pm_counters_max_time[2]);
        bdmf_session_print(session, "pm_counters_max_time[3] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[3], pm_counters_max_time.pm_counters_max_time[3]);
        bdmf_session_print(session, "pm_counters_max_time[4] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[4], pm_counters_max_time.pm_counters_max_time[4]);
        bdmf_session_print(session, "pm_counters_max_time[5] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[5], pm_counters_max_time.pm_counters_max_time[5]);
        bdmf_session_print(session, "pm_counters_max_time[6] = %u (0x%x)\n", pm_counters_max_time.pm_counters_max_time[6], pm_counters_max_time.pm_counters_max_time[6]);
        break;
    }
    case cli_psram_pm_counters_acc_time:
    {
        psram_pm_counters_acc_time pm_counters_acc_time;
        err = ag_drv_psram_pm_counters_acc_time_get(parm[1].value.unumber, &pm_counters_acc_time);
        bdmf_session_print(session, "pm_counters_acc_time[0] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[0], pm_counters_acc_time.pm_counters_acc_time[0]);
        bdmf_session_print(session, "pm_counters_acc_time[1] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[1], pm_counters_acc_time.pm_counters_acc_time[1]);
        bdmf_session_print(session, "pm_counters_acc_time[2] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[2], pm_counters_acc_time.pm_counters_acc_time[2]);
        bdmf_session_print(session, "pm_counters_acc_time[3] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[3], pm_counters_acc_time.pm_counters_acc_time[3]);
        bdmf_session_print(session, "pm_counters_acc_time[4] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[4], pm_counters_acc_time.pm_counters_acc_time[4]);
        bdmf_session_print(session, "pm_counters_acc_time[5] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[5], pm_counters_acc_time.pm_counters_acc_time[5]);
        bdmf_session_print(session, "pm_counters_acc_time[6] = %u (0x%x)\n", pm_counters_acc_time.pm_counters_acc_time[6], pm_counters_acc_time.pm_counters_acc_time[6]);
        break;
    }
    case cli_psram_pm_counters_acc_req:
    {
        psram_pm_counters_acc_req pm_counters_acc_req;
        err = ag_drv_psram_pm_counters_acc_req_get(parm[1].value.unumber, &pm_counters_acc_req);
        bdmf_session_print(session, "pm_counters_acc_req[0] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[0], pm_counters_acc_req.pm_counters_acc_req[0]);
        bdmf_session_print(session, "pm_counters_acc_req[1] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[1], pm_counters_acc_req.pm_counters_acc_req[1]);
        bdmf_session_print(session, "pm_counters_acc_req[2] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[2], pm_counters_acc_req.pm_counters_acc_req[2]);
        bdmf_session_print(session, "pm_counters_acc_req[3] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[3], pm_counters_acc_req.pm_counters_acc_req[3]);
        bdmf_session_print(session, "pm_counters_acc_req[4] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[4], pm_counters_acc_req.pm_counters_acc_req[4]);
        bdmf_session_print(session, "pm_counters_acc_req[5] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[5], pm_counters_acc_req.pm_counters_acc_req[5]);
        bdmf_session_print(session, "pm_counters_acc_req[6] = %u (0x%x)\n", pm_counters_acc_req.pm_counters_acc_req[6], pm_counters_acc_req.pm_counters_acc_req[6]);
        break;
    }
    case cli_psram_pm_counters_last_acc_time:
    {
        psram_pm_counters_last_acc_time pm_counters_last_acc_time;
        err = ag_drv_psram_pm_counters_last_acc_time_get(parm[1].value.unumber, &pm_counters_last_acc_time);
        bdmf_session_print(session, "pm_counters_last_acc_time[0] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[0], pm_counters_last_acc_time.pm_counters_last_acc_time[0]);
        bdmf_session_print(session, "pm_counters_last_acc_time[1] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[1], pm_counters_last_acc_time.pm_counters_last_acc_time[1]);
        bdmf_session_print(session, "pm_counters_last_acc_time[2] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[2], pm_counters_last_acc_time.pm_counters_last_acc_time[2]);
        bdmf_session_print(session, "pm_counters_last_acc_time[3] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[3], pm_counters_last_acc_time.pm_counters_last_acc_time[3]);
        bdmf_session_print(session, "pm_counters_last_acc_time[4] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[4], pm_counters_last_acc_time.pm_counters_last_acc_time[4]);
        bdmf_session_print(session, "pm_counters_last_acc_time[5] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[5], pm_counters_last_acc_time.pm_counters_last_acc_time[5]);
        bdmf_session_print(session, "pm_counters_last_acc_time[6] = %u (0x%x)\n", pm_counters_last_acc_time.pm_counters_last_acc_time[6], pm_counters_last_acc_time.pm_counters_last_acc_time[6]);
        break;
    }
    case cli_psram_pm_counters_last_acc_req:
    {
        psram_pm_counters_last_acc_req pm_counters_last_acc_req;
        err = ag_drv_psram_pm_counters_last_acc_req_get(parm[1].value.unumber, &pm_counters_last_acc_req);
        bdmf_session_print(session, "pm_counters_last_acc_req[0] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[0], pm_counters_last_acc_req.pm_counters_last_acc_req[0]);
        bdmf_session_print(session, "pm_counters_last_acc_req[1] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[1], pm_counters_last_acc_req.pm_counters_last_acc_req[1]);
        bdmf_session_print(session, "pm_counters_last_acc_req[2] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[2], pm_counters_last_acc_req.pm_counters_last_acc_req[2]);
        bdmf_session_print(session, "pm_counters_last_acc_req[3] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[3], pm_counters_last_acc_req.pm_counters_last_acc_req[3]);
        bdmf_session_print(session, "pm_counters_last_acc_req[4] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[4], pm_counters_last_acc_req.pm_counters_last_acc_req[4]);
        bdmf_session_print(session, "pm_counters_last_acc_req[5] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[5], pm_counters_last_acc_req.pm_counters_last_acc_req[5]);
        bdmf_session_print(session, "pm_counters_last_acc_req[6] = %u (0x%x)\n", pm_counters_last_acc_req.pm_counters_last_acc_req[6], pm_counters_last_acc_req.pm_counters_last_acc_req[6]);
        break;
    }
    case cli_psram_pm_counters_bw_wr_cnt:
    {
        psram_pm_counters_bw_wr_cnt pm_counters_bw_wr_cnt;
        err = ag_drv_psram_pm_counters_bw_wr_cnt_get(parm[1].value.unumber, &pm_counters_bw_wr_cnt);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[0] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[1] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[2] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[3] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[4] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[5] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[6] = %u (0x%x)\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6]);
        break;
    }
    case cli_psram_pm_counters_bw_rd_cnt:
    {
        psram_pm_counters_bw_rd_cnt pm_counters_bw_rd_cnt;
        err = ag_drv_psram_pm_counters_bw_rd_cnt_get(parm[1].value.unumber, &pm_counters_bw_rd_cnt);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[0] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[1] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[2] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[3] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[4] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[5] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[6] = %u (0x%x)\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6]);
        break;
    }
    case cli_psram_pm_counters_bw_wr_cnt_last:
    {
        psram_pm_counters_bw_wr_cnt_last pm_counters_bw_wr_cnt_last;
        err = ag_drv_psram_pm_counters_bw_wr_cnt_last_get(parm[1].value.unumber, &pm_counters_bw_wr_cnt_last);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[0] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[1] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[2] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[3] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[4] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[5] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[6] = %u (0x%x)\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6]);
        break;
    }
    case cli_psram_pm_counters_bw_rd_cnt_last:
    {
        psram_pm_counters_bw_rd_cnt_last pm_counters_bw_rd_cnt_last;
        err = ag_drv_psram_pm_counters_bw_rd_cnt_last_get(parm[1].value.unumber, &pm_counters_bw_rd_cnt_last);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[0] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[1] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[2] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[3] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[4] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[5] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[6] = %u (0x%x)\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6]);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_psram_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        psram_pm_counters_arb pm_counters_arb = {.arb_comb_banks_val=gtmv(m, 32), .arb_comb4_val=gtmv(m, 32), .arb_comb_val=gtmv(m, 32), .arb_arb_val=gtmv(m, 32), .arb_req_val=gtmv(m, 32)};
        if(!err) ag_drv_psram_pm_counters_arb_get( &pm_counters_arb);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_arb_get( %u %u %u %u %u)\n", pm_counters_arb.arb_comb_banks_val, pm_counters_arb.arb_comb4_val, pm_counters_arb.arb_comb_val, pm_counters_arb.arb_arb_val, pm_counters_arb.arb_req_val);
    }
    {
        uint32_t last_acc_cnt_rd=gtmv(m, 32);
        uint32_t last_acc_cnt_wr=gtmv(m, 32);
        uint32_t acc_cnt_rd=gtmv(m, 32);
        uint32_t acc_cnt_wr=gtmv(m, 32);
        if(!err) ag_drv_psram_pm_counters_cnt_acc_get( &last_acc_cnt_rd, &last_acc_cnt_wr, &acc_cnt_rd, &acc_cnt_wr);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_cnt_acc_get( %u %u %u %u)\n", last_acc_cnt_rd, last_acc_cnt_wr, acc_cnt_rd, acc_cnt_wr);
    }
    {
        psram_pm_counters_muen pm_counters_muen = {.bwcen=gtmv(m, 1), .cbwcen=gtmv(m, 1), .tw=gtmv(m, 32), .cl0men=gtmv(m, 1), .cl1men=gtmv(m, 1), .cl2men=gtmv(m, 1), .cl3men=gtmv(m, 1), .cl4men=gtmv(m, 1), .cl5men=gtmv(m, 1), .cl6men=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_muen_set( %u %u %u %u %u %u %u %u %u %u)\n", pm_counters_muen.bwcen, pm_counters_muen.cbwcen, pm_counters_muen.tw, pm_counters_muen.cl0men, pm_counters_muen.cl1men, pm_counters_muen.cl2men, pm_counters_muen.cl3men, pm_counters_muen.cl4men, pm_counters_muen.cl5men, pm_counters_muen.cl6men);
        if(!err) ag_drv_psram_pm_counters_muen_set(&pm_counters_muen);
        if(!err) ag_drv_psram_pm_counters_muen_get( &pm_counters_muen);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_muen_get( %u %u %u %u %u %u %u %u %u %u)\n", pm_counters_muen.bwcen, pm_counters_muen.cbwcen, pm_counters_muen.tw, pm_counters_muen.cl0men, pm_counters_muen.cl1men, pm_counters_muen.cl2men, pm_counters_muen.cl3men, pm_counters_muen.cl4men, pm_counters_muen.cl5men, pm_counters_muen.cl6men);
        if(err || pm_counters_muen.bwcen!=gtmv(m, 1) || pm_counters_muen.cbwcen!=gtmv(m, 1) || pm_counters_muen.tw!=gtmv(m, 32) || pm_counters_muen.cl0men!=gtmv(m, 1) || pm_counters_muen.cl1men!=gtmv(m, 1) || pm_counters_muen.cl2men!=gtmv(m, 1) || pm_counters_muen.cl3men!=gtmv(m, 1) || pm_counters_muen.cl4men!=gtmv(m, 1) || pm_counters_muen.cl5men!=gtmv(m, 1) || pm_counters_muen.cl6men!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        psram_cfg_ctrl cfg_ctrl = {.perm_en=gtmv(m, 1), .comb_en=gtmv(m, 1), .comb_full=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_psram_cfg_ctrl_set( %u %u %u)\n", cfg_ctrl.perm_en, cfg_ctrl.comb_en, cfg_ctrl.comb_full);
        if(!err) ag_drv_psram_cfg_ctrl_set(&cfg_ctrl);
        if(!err) ag_drv_psram_cfg_ctrl_get( &cfg_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_psram_cfg_ctrl_get( %u %u %u)\n", cfg_ctrl.perm_en, cfg_ctrl.comb_en, cfg_ctrl.comb_full);
        if(err || cfg_ctrl.perm_en!=gtmv(m, 1) || cfg_ctrl.comb_en!=gtmv(m, 1) || cfg_ctrl.comb_full!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t psram_enrty=gtmv(m, 11);
        psram_memory_data memory_data = {.memory_data={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_psram_memory_data_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_enrty, memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
        if(!err) ag_drv_psram_memory_data_set(psram_enrty, &memory_data);
        if(!err) ag_drv_psram_memory_data_get( psram_enrty, &memory_data);
        if(!err) bdmf_session_print(session, "ag_drv_psram_memory_data_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_enrty, memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
        if(err || memory_data.memory_data[0]!=gtmv(m, 32) || memory_data.memory_data[1]!=gtmv(m, 32) || memory_data.memory_data[2]!=gtmv(m, 32) || memory_data.memory_data[3]!=gtmv(m, 32) || memory_data.memory_data[4]!=gtmv(m, 32) || memory_data.memory_data[5]!=gtmv(m, 32) || memory_data.memory_data[6]!=gtmv(m, 32) || memory_data.memory_data[7]!=gtmv(m, 32) || memory_data.memory_data[8]!=gtmv(m, 32) || memory_data.memory_data[9]!=gtmv(m, 32) || memory_data.memory_data[10]!=gtmv(m, 32) || memory_data.memory_data[11]!=gtmv(m, 32) || memory_data.memory_data[12]!=gtmv(m, 32) || memory_data.memory_data[13]!=gtmv(m, 32) || memory_data.memory_data[14]!=gtmv(m, 32) || memory_data.memory_data[15]!=gtmv(m, 32) || memory_data.memory_data[16]!=gtmv(m, 32) || memory_data.memory_data[17]!=gtmv(m, 32) || memory_data.memory_data[18]!=gtmv(m, 32) || memory_data.memory_data[19]!=gtmv(m, 32) || memory_data.memory_data[20]!=gtmv(m, 32) || memory_data.memory_data[21]!=gtmv(m, 32) || memory_data.memory_data[22]!=gtmv(m, 32) || memory_data.memory_data[23]!=gtmv(m, 32) || memory_data.memory_data[24]!=gtmv(m, 32) || memory_data.memory_data[25]!=gtmv(m, 32) || memory_data.memory_data[26]!=gtmv(m, 32) || memory_data.memory_data[27]!=gtmv(m, 32) || memory_data.memory_data[28]!=gtmv(m, 32) || memory_data.memory_data[29]!=gtmv(m, 32) || memory_data.memory_data[30]!=gtmv(m, 32) || memory_data.memory_data[31]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_psram_configurations_clk_gate_cntrl_set( %u %u %u %u %u)\n", configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_intrvl, configurations_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_psram_configurations_clk_gate_cntrl_set(&configurations_clk_gate_cntrl);
        if(!err) ag_drv_psram_configurations_clk_gate_cntrl_get( &configurations_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_psram_configurations_clk_gate_cntrl_get( %u %u %u %u %u)\n", configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_intrvl, configurations_clk_gate_cntrl.keep_alive_cyc);
        if(err || configurations_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || configurations_clk_gate_cntrl.timer_val!=gtmv(m, 8) || configurations_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || configurations_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || configurations_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_max_time pm_counters_max_time = {.pm_counters_max_time={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_max_time_get( zero, &pm_counters_max_time);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_max_time_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_max_time.pm_counters_max_time[0], pm_counters_max_time.pm_counters_max_time[1], pm_counters_max_time.pm_counters_max_time[2], pm_counters_max_time.pm_counters_max_time[3], pm_counters_max_time.pm_counters_max_time[4], pm_counters_max_time.pm_counters_max_time[5], pm_counters_max_time.pm_counters_max_time[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_acc_time pm_counters_acc_time = {.pm_counters_acc_time={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_acc_time_get( zero, &pm_counters_acc_time);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_acc_time_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_acc_time.pm_counters_acc_time[0], pm_counters_acc_time.pm_counters_acc_time[1], pm_counters_acc_time.pm_counters_acc_time[2], pm_counters_acc_time.pm_counters_acc_time[3], pm_counters_acc_time.pm_counters_acc_time[4], pm_counters_acc_time.pm_counters_acc_time[5], pm_counters_acc_time.pm_counters_acc_time[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_acc_req pm_counters_acc_req = {.pm_counters_acc_req={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_acc_req_get( zero, &pm_counters_acc_req);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_acc_req_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_acc_req.pm_counters_acc_req[0], pm_counters_acc_req.pm_counters_acc_req[1], pm_counters_acc_req.pm_counters_acc_req[2], pm_counters_acc_req.pm_counters_acc_req[3], pm_counters_acc_req.pm_counters_acc_req[4], pm_counters_acc_req.pm_counters_acc_req[5], pm_counters_acc_req.pm_counters_acc_req[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_last_acc_time pm_counters_last_acc_time = {.pm_counters_last_acc_time={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_last_acc_time_get( zero, &pm_counters_last_acc_time);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_last_acc_time_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_last_acc_time.pm_counters_last_acc_time[0], pm_counters_last_acc_time.pm_counters_last_acc_time[1], pm_counters_last_acc_time.pm_counters_last_acc_time[2], pm_counters_last_acc_time.pm_counters_last_acc_time[3], pm_counters_last_acc_time.pm_counters_last_acc_time[4], pm_counters_last_acc_time.pm_counters_last_acc_time[5], pm_counters_last_acc_time.pm_counters_last_acc_time[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_last_acc_req pm_counters_last_acc_req = {.pm_counters_last_acc_req={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_last_acc_req_get( zero, &pm_counters_last_acc_req);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_last_acc_req_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_last_acc_req.pm_counters_last_acc_req[0], pm_counters_last_acc_req.pm_counters_last_acc_req[1], pm_counters_last_acc_req.pm_counters_last_acc_req[2], pm_counters_last_acc_req.pm_counters_last_acc_req[3], pm_counters_last_acc_req.pm_counters_last_acc_req[4], pm_counters_last_acc_req.pm_counters_last_acc_req[5], pm_counters_last_acc_req.pm_counters_last_acc_req[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_bw_wr_cnt pm_counters_bw_wr_cnt = {.pm_counters_bw_wr_cnt={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_bw_wr_cnt_get( zero, &pm_counters_bw_wr_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_wr_cnt_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_bw_rd_cnt pm_counters_bw_rd_cnt = {.pm_counters_bw_rd_cnt={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_bw_rd_cnt_get( zero, &pm_counters_bw_rd_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_rd_cnt_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_bw_wr_cnt_last pm_counters_bw_wr_cnt_last = {.pm_counters_bw_wr_cnt_last={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_bw_wr_cnt_last_get( zero, &pm_counters_bw_wr_cnt_last);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_wr_cnt_last_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        psram_pm_counters_bw_rd_cnt_last pm_counters_bw_rd_cnt_last = {.pm_counters_bw_rd_cnt_last={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_psram_pm_counters_bw_rd_cnt_last_get( zero, &pm_counters_bw_rd_cnt_last);
        if(!err) bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_rd_cnt_last_get( %u %u %u %u %u %u %u %u)\n", zero, pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6]);
    }
    return err;
}

static int bcm_psram_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_memory_data : reg = &RU_REG(PSRAM, MEMORY_DATA); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_configurations_ctrl : reg = &RU_REG(PSRAM, CONFIGURATIONS_CTRL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_configurations_clk_gate_cntrl : reg = &RU_REG(PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_muen : reg = &RU_REG(PSRAM, PM_COUNTERS_MUEN); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bwcl : reg = &RU_REG(PSRAM, PM_COUNTERS_BWCL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bwen : reg = &RU_REG(PSRAM, PM_COUNTERS_BWEN); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_max_time : reg = &RU_REG(PSRAM, PM_COUNTERS_MAX_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_acc_time : reg = &RU_REG(PSRAM, PM_COUNTERS_ACC_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_acc_req : reg = &RU_REG(PSRAM, PM_COUNTERS_ACC_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_last_acc_time : reg = &RU_REG(PSRAM, PM_COUNTERS_LAST_ACC_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_last_acc_req : reg = &RU_REG(PSRAM, PM_COUNTERS_LAST_ACC_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_acc : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_acc : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_last_acc : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_LAST_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_last_acc : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_LAST_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_last : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_LAST); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_last : reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_LAST); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_req : reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_arb : reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_ARB); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb : reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb_4 : reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB_4); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb_banks : reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB_BANKS); blk = &RU_BLK(PSRAM); break;
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

bdmfmon_handle_t ag_drv_psram_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "psram"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "psram", "psram", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_pm_counters_muen[]={
            BDMFMON_MAKE_PARM("bwcen", "bwcen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cbwcen", "cbwcen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tw", "tw", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl0men", "cl0men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl1men", "cl1men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl2men", "cl2men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl3men", "cl3men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl4men", "cl4men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl5men", "cl5men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cl6men", "cl6men", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ctrl[]={
            BDMFMON_MAKE_PARM("perm_en", "perm_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("comb_en", "comb_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("comb_full", "comb_full", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_memory_data[]={
            BDMFMON_MAKE_PARM("psram_enrty", "psram_enrty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("memory_data0", "memory_data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data1", "memory_data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data2", "memory_data2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data3", "memory_data3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data4", "memory_data4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data5", "memory_data5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data6", "memory_data6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data7", "memory_data7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data8", "memory_data8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data9", "memory_data9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data10", "memory_data10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data11", "memory_data11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data12", "memory_data12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data13", "memory_data13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data14", "memory_data14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data15", "memory_data15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data16", "memory_data16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data17", "memory_data17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data18", "memory_data18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data19", "memory_data19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data20", "memory_data20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data21", "memory_data21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data22", "memory_data22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data23", "memory_data23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data24", "memory_data24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data25", "memory_data25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data26", "memory_data26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data27", "memory_data27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data28", "memory_data28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data29", "memory_data29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data30", "memory_data30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data31", "memory_data31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_configurations_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pm_counters_muen", .val=cli_psram_pm_counters_muen, .parms=set_pm_counters_muen },
            { .name="cfg_ctrl", .val=cli_psram_cfg_ctrl, .parms=set_cfg_ctrl },
            { .name="memory_data", .val=cli_psram_memory_data, .parms=set_memory_data },
            { .name="configurations_clk_gate_cntrl", .val=cli_psram_configurations_clk_gate_cntrl, .parms=set_configurations_clk_gate_cntrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_psram_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_memory_data[]={
            BDMFMON_MAKE_PARM("psram_enrty", "psram_enrty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_max_time[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_acc_time[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_acc_req[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_last_acc_time[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_last_acc_req[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_bw_wr_cnt[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_bw_rd_cnt[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_bw_wr_cnt_last[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pm_counters_bw_rd_cnt_last[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pm_counters_arb", .val=cli_psram_pm_counters_arb, .parms=set_default },
            { .name="pm_counters_cnt_acc", .val=cli_psram_pm_counters_cnt_acc, .parms=set_default },
            { .name="pm_counters_muen", .val=cli_psram_pm_counters_muen, .parms=set_default },
            { .name="cfg_ctrl", .val=cli_psram_cfg_ctrl, .parms=set_default },
            { .name="memory_data", .val=cli_psram_memory_data, .parms=set_memory_data },
            { .name="configurations_clk_gate_cntrl", .val=cli_psram_configurations_clk_gate_cntrl, .parms=set_default },
            { .name="pm_counters_max_time", .val=cli_psram_pm_counters_max_time, .parms=set_pm_counters_max_time },
            { .name="pm_counters_acc_time", .val=cli_psram_pm_counters_acc_time, .parms=set_pm_counters_acc_time },
            { .name="pm_counters_acc_req", .val=cli_psram_pm_counters_acc_req, .parms=set_pm_counters_acc_req },
            { .name="pm_counters_last_acc_time", .val=cli_psram_pm_counters_last_acc_time, .parms=set_pm_counters_last_acc_time },
            { .name="pm_counters_last_acc_req", .val=cli_psram_pm_counters_last_acc_req, .parms=set_pm_counters_last_acc_req },
            { .name="pm_counters_bw_wr_cnt", .val=cli_psram_pm_counters_bw_wr_cnt, .parms=set_pm_counters_bw_wr_cnt },
            { .name="pm_counters_bw_rd_cnt", .val=cli_psram_pm_counters_bw_rd_cnt, .parms=set_pm_counters_bw_rd_cnt },
            { .name="pm_counters_bw_wr_cnt_last", .val=cli_psram_pm_counters_bw_wr_cnt_last, .parms=set_pm_counters_bw_wr_cnt_last },
            { .name="pm_counters_bw_rd_cnt_last", .val=cli_psram_pm_counters_bw_rd_cnt_last, .parms=set_pm_counters_bw_rd_cnt_last },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_psram_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_psram_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="MEMORY_DATA" , .val=bdmf_address_memory_data },
            { .name="CONFIGURATIONS_CTRL" , .val=bdmf_address_configurations_ctrl },
            { .name="CONFIGURATIONS_CLK_GATE_CNTRL" , .val=bdmf_address_configurations_clk_gate_cntrl },
            { .name="PM_COUNTERS_MUEN" , .val=bdmf_address_pm_counters_muen },
            { .name="PM_COUNTERS_BWCL" , .val=bdmf_address_pm_counters_bwcl },
            { .name="PM_COUNTERS_BWEN" , .val=bdmf_address_pm_counters_bwen },
            { .name="PM_COUNTERS_MAX_TIME" , .val=bdmf_address_pm_counters_max_time },
            { .name="PM_COUNTERS_ACC_TIME" , .val=bdmf_address_pm_counters_acc_time },
            { .name="PM_COUNTERS_ACC_REQ" , .val=bdmf_address_pm_counters_acc_req },
            { .name="PM_COUNTERS_LAST_ACC_TIME" , .val=bdmf_address_pm_counters_last_acc_time },
            { .name="PM_COUNTERS_LAST_ACC_REQ" , .val=bdmf_address_pm_counters_last_acc_req },
            { .name="PM_COUNTERS_BW_WR_CNT_ACC" , .val=bdmf_address_pm_counters_bw_wr_cnt_acc },
            { .name="PM_COUNTERS_BW_RD_CNT_ACC" , .val=bdmf_address_pm_counters_bw_rd_cnt_acc },
            { .name="PM_COUNTERS_BW_WR_CNT" , .val=bdmf_address_pm_counters_bw_wr_cnt },
            { .name="PM_COUNTERS_BW_RD_CNT" , .val=bdmf_address_pm_counters_bw_rd_cnt },
            { .name="PM_COUNTERS_BW_WR_CNT_LAST_ACC" , .val=bdmf_address_pm_counters_bw_wr_cnt_last_acc },
            { .name="PM_COUNTERS_BW_RD_CNT_LAST_ACC" , .val=bdmf_address_pm_counters_bw_rd_cnt_last_acc },
            { .name="PM_COUNTERS_BW_WR_CNT_LAST" , .val=bdmf_address_pm_counters_bw_wr_cnt_last },
            { .name="PM_COUNTERS_BW_RD_CNT_LAST" , .val=bdmf_address_pm_counters_bw_rd_cnt_last },
            { .name="PM_COUNTERS_ARB_REQ" , .val=bdmf_address_pm_counters_arb_req },
            { .name="PM_COUNTERS_ARB_ARB" , .val=bdmf_address_pm_counters_arb_arb },
            { .name="PM_COUNTERS_ARB_COMB" , .val=bdmf_address_pm_counters_arb_comb },
            { .name="PM_COUNTERS_ARB_COMB_4" , .val=bdmf_address_pm_counters_arb_comb_4 },
            { .name="PM_COUNTERS_ARB_COMB_BANKS" , .val=bdmf_address_pm_counters_arb_comb_banks },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_psram_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

