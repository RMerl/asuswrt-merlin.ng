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
#include "xrdp_drv_psram_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_psram_pm_counters_arb_get(psram_pm_counters_arb *pm_counters_arb)
{
    uint32_t reg_pm_counters_arb_comb_banks;
    uint32_t reg_pm_counters_arb_comb_4;
    uint32_t reg_pm_counters_arb_comb;
    uint32_t reg_pm_counters_arb_arb;
    uint32_t reg_pm_counters_arb_req;

#ifdef VALIDATE_PARMS
    if (!pm_counters_arb)
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
    if (!last_acc_cnt_rd || !last_acc_cnt_wr || !acc_cnt_rd || !acc_cnt_wr)
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
    uint32_t reg_pm_counters_bwen = 0;
    uint32_t reg_pm_counters_bwcl = 0;
    uint32_t reg_pm_counters_muen = 0;

#ifdef VALIDATE_PARMS
    if(!pm_counters_muen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pm_counters_muen->bwcen >= _1BITS_MAX_VAL_) ||
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
    if (!pm_counters_muen)
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
    uint32_t reg_configurations_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((cfg_ctrl->perm_en >= _1BITS_MAX_VAL_) ||
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
    if (!cfg_ctrl)
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

bdmf_error_t ag_drv_psram_configurations_scrm_seed_set(uint32_t val)
{
    uint32_t reg_configurations_scrm_seed = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_configurations_scrm_seed = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_SCRM_SEED, VAL, reg_configurations_scrm_seed, val);

    RU_REG_WRITE(0, PSRAM, CONFIGURATIONS_SCRM_SEED, reg_configurations_scrm_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_scrm_seed_get(uint32_t *val)
{
    uint32_t reg_configurations_scrm_seed;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, CONFIGURATIONS_SCRM_SEED, reg_configurations_scrm_seed);

    *val = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_SCRM_SEED, VAL, reg_configurations_scrm_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_scrm_addr_set(uint32_t val)
{
    uint32_t reg_configurations_scrm_addr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_configurations_scrm_addr = RU_FIELD_SET(0, PSRAM, CONFIGURATIONS_SCRM_ADDR, VAL, reg_configurations_scrm_addr, val);

    RU_REG_WRITE(0, PSRAM, CONFIGURATIONS_SCRM_ADDR, reg_configurations_scrm_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_scrm_addr_get(uint32_t *val)
{
    uint32_t reg_configurations_scrm_addr;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PSRAM, CONFIGURATIONS_SCRM_ADDR, reg_configurations_scrm_addr);

    *val = RU_FIELD_GET(0, PSRAM, CONFIGURATIONS_SCRM_ADDR, VAL, reg_configurations_scrm_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_set(const psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl)
{
    uint32_t reg_configurations_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
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
    if (!configurations_clk_gate_cntrl)
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
    if (!pm_counters_max_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_MAX_TIME, pm_counters_max_time->pm_counters_max_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_acc_time_get(uint32_t zero, psram_pm_counters_acc_time *pm_counters_acc_time)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_acc_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_ACC_TIME, pm_counters_acc_time->pm_counters_acc_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_acc_req_get(uint32_t zero, psram_pm_counters_acc_req *pm_counters_acc_req)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_acc_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_ACC_REQ, pm_counters_acc_req->pm_counters_acc_req[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_last_acc_time_get(uint32_t zero, psram_pm_counters_last_acc_time *pm_counters_last_acc_time)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_last_acc_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_LAST_ACC_TIME, pm_counters_last_acc_time->pm_counters_last_acc_time[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_last_acc_req_get(uint32_t zero, psram_pm_counters_last_acc_req *pm_counters_last_acc_req)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_last_acc_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_LAST_ACC_REQ, pm_counters_last_acc_req->pm_counters_last_acc_req[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_get(uint32_t zero, psram_pm_counters_bw_wr_cnt *pm_counters_bw_wr_cnt)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_bw_wr_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_BW_WR_CNT, pm_counters_bw_wr_cnt->pm_counters_bw_wr_cnt[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_get(uint32_t zero, psram_pm_counters_bw_rd_cnt *pm_counters_bw_rd_cnt)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_bw_rd_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_BW_RD_CNT, pm_counters_bw_rd_cnt->pm_counters_bw_rd_cnt[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_last_get(uint32_t zero, psram_pm_counters_bw_wr_cnt_last *pm_counters_bw_wr_cnt_last)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_bw_wr_cnt_last)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_BW_WR_CNT_LAST, pm_counters_bw_wr_cnt_last->pm_counters_bw_wr_cnt_last[6]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_last_get(uint32_t zero, psram_pm_counters_bw_rd_cnt_last *pm_counters_bw_rd_cnt_last)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_bw_rd_cnt_last)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero * 7 + 0, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[0]);
    RU_REG_RAM_READ(0, zero * 7 + 1, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[1]);
    RU_REG_RAM_READ(0, zero * 7 + 2, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[2]);
    RU_REG_RAM_READ(0, zero * 7 + 3, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[3]);
    RU_REG_RAM_READ(0, zero * 7 + 4, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[4]);
    RU_REG_RAM_READ(0, zero * 7 + 5, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[5]);
    RU_REG_RAM_READ(0, zero * 7 + 6, PSRAM, PM_COUNTERS_BW_RD_CNT_LAST, pm_counters_bw_rd_cnt_last->pm_counters_bw_rd_cnt_last[6]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_configurations_ctrl,
    bdmf_address_configurations_scrm_seed,
    bdmf_address_configurations_scrm_addr,
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
    bdmf_address_debug_dbgsel,
    bdmf_address_debug_dbgbus,
    bdmf_address_debug_req_vec,
    bdmf_address_debug_dbg_cap_cfg1,
    bdmf_address_debug_dbg_cap_cfg2,
    bdmf_address_debug_dbg_cap_st,
    bdmf_address_debug_dbg_cap_w0,
    bdmf_address_debug_dbg_cap_w1,
    bdmf_address_debug_dbg_cap_w2,
    bdmf_address_debug_dbg_cap_w3,
    bdmf_address_debug_dbg_cap_wmsk,
    bdmf_address_debug_dbg_cap_r0,
    bdmf_address_debug_dbg_cap_r1,
    bdmf_address_debug_dbg_cap_r2,
    bdmf_address_debug_dbg_cap_r3,
}
bdmf_address;

static int ag_drv_psram_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_psram_pm_counters_muen:
    {
        psram_pm_counters_muen pm_counters_muen = { .bwcen = parm[1].value.unumber, .cbwcen = parm[2].value.unumber, .tw = parm[3].value.unumber, .cl0men = parm[4].value.unumber, .cl1men = parm[5].value.unumber, .cl2men = parm[6].value.unumber, .cl3men = parm[7].value.unumber, .cl4men = parm[8].value.unumber, .cl5men = parm[9].value.unumber, .cl6men = parm[10].value.unumber};
        ag_err = ag_drv_psram_pm_counters_muen_set(&pm_counters_muen);
        break;
    }
    case cli_psram_cfg_ctrl:
    {
        psram_cfg_ctrl cfg_ctrl = { .perm_en = parm[1].value.unumber, .comb_en = parm[2].value.unumber, .comb_full = parm[3].value.unumber};
        ag_err = ag_drv_psram_cfg_ctrl_set(&cfg_ctrl);
        break;
    }
    case cli_psram_configurations_scrm_seed:
        ag_err = ag_drv_psram_configurations_scrm_seed_set(parm[1].value.unumber);
        break;
    case cli_psram_configurations_scrm_addr:
        ag_err = ag_drv_psram_configurations_scrm_addr_set(parm[1].value.unumber);
        break;
    case cli_psram_configurations_clk_gate_cntrl:
    {
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl = { .bypass_clk_gate = parm[1].value.unumber, .timer_val = parm[2].value.unumber, .keep_alive_en = parm[3].value.unumber, .keep_alive_intrvl = parm[4].value.unumber, .keep_alive_cyc = parm[5].value.unumber};
        ag_err = ag_drv_psram_configurations_clk_gate_cntrl_set(&configurations_clk_gate_cntrl);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_psram_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_psram_pm_counters_arb:
    {
        psram_pm_counters_arb pm_counters_arb;
        ag_err = ag_drv_psram_pm_counters_arb_get(&pm_counters_arb);
        bdmf_session_print(session, "arb_comb_banks_val = %u = 0x%x\n", pm_counters_arb.arb_comb_banks_val, pm_counters_arb.arb_comb_banks_val);
        bdmf_session_print(session, "arb_comb4_val = %u = 0x%x\n", pm_counters_arb.arb_comb4_val, pm_counters_arb.arb_comb4_val);
        bdmf_session_print(session, "arb_comb_val = %u = 0x%x\n", pm_counters_arb.arb_comb_val, pm_counters_arb.arb_comb_val);
        bdmf_session_print(session, "arb_arb_val = %u = 0x%x\n", pm_counters_arb.arb_arb_val, pm_counters_arb.arb_arb_val);
        bdmf_session_print(session, "arb_req_val = %u = 0x%x\n", pm_counters_arb.arb_req_val, pm_counters_arb.arb_req_val);
        break;
    }
    case cli_psram_pm_counters_cnt_acc:
    {
        uint32_t last_acc_cnt_rd;
        uint32_t last_acc_cnt_wr;
        uint32_t acc_cnt_rd;
        uint32_t acc_cnt_wr;
        ag_err = ag_drv_psram_pm_counters_cnt_acc_get(&last_acc_cnt_rd, &last_acc_cnt_wr, &acc_cnt_rd, &acc_cnt_wr);
        bdmf_session_print(session, "last_acc_cnt_rd = %u = 0x%x\n", last_acc_cnt_rd, last_acc_cnt_rd);
        bdmf_session_print(session, "last_acc_cnt_wr = %u = 0x%x\n", last_acc_cnt_wr, last_acc_cnt_wr);
        bdmf_session_print(session, "acc_cnt_rd = %u = 0x%x\n", acc_cnt_rd, acc_cnt_rd);
        bdmf_session_print(session, "acc_cnt_wr = %u = 0x%x\n", acc_cnt_wr, acc_cnt_wr);
        break;
    }
    case cli_psram_pm_counters_muen:
    {
        psram_pm_counters_muen pm_counters_muen;
        ag_err = ag_drv_psram_pm_counters_muen_get(&pm_counters_muen);
        bdmf_session_print(session, "bwcen = %u = 0x%x\n", pm_counters_muen.bwcen, pm_counters_muen.bwcen);
        bdmf_session_print(session, "cbwcen = %u = 0x%x\n", pm_counters_muen.cbwcen, pm_counters_muen.cbwcen);
        bdmf_session_print(session, "tw = %u = 0x%x\n", pm_counters_muen.tw, pm_counters_muen.tw);
        bdmf_session_print(session, "cl0men = %u = 0x%x\n", pm_counters_muen.cl0men, pm_counters_muen.cl0men);
        bdmf_session_print(session, "cl1men = %u = 0x%x\n", pm_counters_muen.cl1men, pm_counters_muen.cl1men);
        bdmf_session_print(session, "cl2men = %u = 0x%x\n", pm_counters_muen.cl2men, pm_counters_muen.cl2men);
        bdmf_session_print(session, "cl3men = %u = 0x%x\n", pm_counters_muen.cl3men, pm_counters_muen.cl3men);
        bdmf_session_print(session, "cl4men = %u = 0x%x\n", pm_counters_muen.cl4men, pm_counters_muen.cl4men);
        bdmf_session_print(session, "cl5men = %u = 0x%x\n", pm_counters_muen.cl5men, pm_counters_muen.cl5men);
        bdmf_session_print(session, "cl6men = %u = 0x%x\n", pm_counters_muen.cl6men, pm_counters_muen.cl6men);
        break;
    }
    case cli_psram_cfg_ctrl:
    {
        psram_cfg_ctrl cfg_ctrl;
        ag_err = ag_drv_psram_cfg_ctrl_get(&cfg_ctrl);
        bdmf_session_print(session, "perm_en = %u = 0x%x\n", cfg_ctrl.perm_en, cfg_ctrl.perm_en);
        bdmf_session_print(session, "comb_en = %u = 0x%x\n", cfg_ctrl.comb_en, cfg_ctrl.comb_en);
        bdmf_session_print(session, "comb_full = %u = 0x%x\n", cfg_ctrl.comb_full, cfg_ctrl.comb_full);
        break;
    }
    case cli_psram_configurations_scrm_seed:
    {
        uint32_t val;
        ag_err = ag_drv_psram_configurations_scrm_seed_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_psram_configurations_scrm_addr:
    {
        uint32_t val;
        ag_err = ag_drv_psram_configurations_scrm_addr_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_psram_configurations_clk_gate_cntrl:
    {
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl;
        ag_err = ag_drv_psram_configurations_clk_gate_cntrl_get(&configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", configurations_clk_gate_cntrl.keep_alive_intrvl, configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", configurations_clk_gate_cntrl.keep_alive_cyc, configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_psram_pm_counters_max_time:
    {
        psram_pm_counters_max_time pm_counters_max_time;
        ag_err = ag_drv_psram_pm_counters_max_time_get(parm[1].value.unumber, &pm_counters_max_time);
        bdmf_session_print(session, "pm_counters_max_time[0] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[0], pm_counters_max_time.pm_counters_max_time[0]);
        bdmf_session_print(session, "pm_counters_max_time[1] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[1], pm_counters_max_time.pm_counters_max_time[1]);
        bdmf_session_print(session, "pm_counters_max_time[2] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[2], pm_counters_max_time.pm_counters_max_time[2]);
        bdmf_session_print(session, "pm_counters_max_time[3] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[3], pm_counters_max_time.pm_counters_max_time[3]);
        bdmf_session_print(session, "pm_counters_max_time[4] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[4], pm_counters_max_time.pm_counters_max_time[4]);
        bdmf_session_print(session, "pm_counters_max_time[5] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[5], pm_counters_max_time.pm_counters_max_time[5]);
        bdmf_session_print(session, "pm_counters_max_time[6] = %u = 0x%x\n", pm_counters_max_time.pm_counters_max_time[6], pm_counters_max_time.pm_counters_max_time[6]);
        break;
    }
    case cli_psram_pm_counters_acc_time:
    {
        psram_pm_counters_acc_time pm_counters_acc_time;
        ag_err = ag_drv_psram_pm_counters_acc_time_get(parm[1].value.unumber, &pm_counters_acc_time);
        bdmf_session_print(session, "pm_counters_acc_time[0] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[0], pm_counters_acc_time.pm_counters_acc_time[0]);
        bdmf_session_print(session, "pm_counters_acc_time[1] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[1], pm_counters_acc_time.pm_counters_acc_time[1]);
        bdmf_session_print(session, "pm_counters_acc_time[2] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[2], pm_counters_acc_time.pm_counters_acc_time[2]);
        bdmf_session_print(session, "pm_counters_acc_time[3] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[3], pm_counters_acc_time.pm_counters_acc_time[3]);
        bdmf_session_print(session, "pm_counters_acc_time[4] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[4], pm_counters_acc_time.pm_counters_acc_time[4]);
        bdmf_session_print(session, "pm_counters_acc_time[5] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[5], pm_counters_acc_time.pm_counters_acc_time[5]);
        bdmf_session_print(session, "pm_counters_acc_time[6] = %u = 0x%x\n", pm_counters_acc_time.pm_counters_acc_time[6], pm_counters_acc_time.pm_counters_acc_time[6]);
        break;
    }
    case cli_psram_pm_counters_acc_req:
    {
        psram_pm_counters_acc_req pm_counters_acc_req;
        ag_err = ag_drv_psram_pm_counters_acc_req_get(parm[1].value.unumber, &pm_counters_acc_req);
        bdmf_session_print(session, "pm_counters_acc_req[0] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[0], pm_counters_acc_req.pm_counters_acc_req[0]);
        bdmf_session_print(session, "pm_counters_acc_req[1] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[1], pm_counters_acc_req.pm_counters_acc_req[1]);
        bdmf_session_print(session, "pm_counters_acc_req[2] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[2], pm_counters_acc_req.pm_counters_acc_req[2]);
        bdmf_session_print(session, "pm_counters_acc_req[3] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[3], pm_counters_acc_req.pm_counters_acc_req[3]);
        bdmf_session_print(session, "pm_counters_acc_req[4] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[4], pm_counters_acc_req.pm_counters_acc_req[4]);
        bdmf_session_print(session, "pm_counters_acc_req[5] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[5], pm_counters_acc_req.pm_counters_acc_req[5]);
        bdmf_session_print(session, "pm_counters_acc_req[6] = %u = 0x%x\n", pm_counters_acc_req.pm_counters_acc_req[6], pm_counters_acc_req.pm_counters_acc_req[6]);
        break;
    }
    case cli_psram_pm_counters_last_acc_time:
    {
        psram_pm_counters_last_acc_time pm_counters_last_acc_time;
        ag_err = ag_drv_psram_pm_counters_last_acc_time_get(parm[1].value.unumber, &pm_counters_last_acc_time);
        bdmf_session_print(session, "pm_counters_last_acc_time[0] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[0], pm_counters_last_acc_time.pm_counters_last_acc_time[0]);
        bdmf_session_print(session, "pm_counters_last_acc_time[1] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[1], pm_counters_last_acc_time.pm_counters_last_acc_time[1]);
        bdmf_session_print(session, "pm_counters_last_acc_time[2] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[2], pm_counters_last_acc_time.pm_counters_last_acc_time[2]);
        bdmf_session_print(session, "pm_counters_last_acc_time[3] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[3], pm_counters_last_acc_time.pm_counters_last_acc_time[3]);
        bdmf_session_print(session, "pm_counters_last_acc_time[4] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[4], pm_counters_last_acc_time.pm_counters_last_acc_time[4]);
        bdmf_session_print(session, "pm_counters_last_acc_time[5] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[5], pm_counters_last_acc_time.pm_counters_last_acc_time[5]);
        bdmf_session_print(session, "pm_counters_last_acc_time[6] = %u = 0x%x\n", pm_counters_last_acc_time.pm_counters_last_acc_time[6], pm_counters_last_acc_time.pm_counters_last_acc_time[6]);
        break;
    }
    case cli_psram_pm_counters_last_acc_req:
    {
        psram_pm_counters_last_acc_req pm_counters_last_acc_req;
        ag_err = ag_drv_psram_pm_counters_last_acc_req_get(parm[1].value.unumber, &pm_counters_last_acc_req);
        bdmf_session_print(session, "pm_counters_last_acc_req[0] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[0], pm_counters_last_acc_req.pm_counters_last_acc_req[0]);
        bdmf_session_print(session, "pm_counters_last_acc_req[1] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[1], pm_counters_last_acc_req.pm_counters_last_acc_req[1]);
        bdmf_session_print(session, "pm_counters_last_acc_req[2] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[2], pm_counters_last_acc_req.pm_counters_last_acc_req[2]);
        bdmf_session_print(session, "pm_counters_last_acc_req[3] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[3], pm_counters_last_acc_req.pm_counters_last_acc_req[3]);
        bdmf_session_print(session, "pm_counters_last_acc_req[4] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[4], pm_counters_last_acc_req.pm_counters_last_acc_req[4]);
        bdmf_session_print(session, "pm_counters_last_acc_req[5] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[5], pm_counters_last_acc_req.pm_counters_last_acc_req[5]);
        bdmf_session_print(session, "pm_counters_last_acc_req[6] = %u = 0x%x\n", pm_counters_last_acc_req.pm_counters_last_acc_req[6], pm_counters_last_acc_req.pm_counters_last_acc_req[6]);
        break;
    }
    case cli_psram_pm_counters_bw_wr_cnt:
    {
        psram_pm_counters_bw_wr_cnt pm_counters_bw_wr_cnt;
        ag_err = ag_drv_psram_pm_counters_bw_wr_cnt_get(parm[1].value.unumber, &pm_counters_bw_wr_cnt);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[0] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[1] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[2] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[3] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[4] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[5] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt[6] = %u = 0x%x\n", pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6]);
        break;
    }
    case cli_psram_pm_counters_bw_rd_cnt:
    {
        psram_pm_counters_bw_rd_cnt pm_counters_bw_rd_cnt;
        ag_err = ag_drv_psram_pm_counters_bw_rd_cnt_get(parm[1].value.unumber, &pm_counters_bw_rd_cnt);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[0] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[1] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[2] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[3] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[4] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[5] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt[6] = %u = 0x%x\n", pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6]);
        break;
    }
    case cli_psram_pm_counters_bw_wr_cnt_last:
    {
        psram_pm_counters_bw_wr_cnt_last pm_counters_bw_wr_cnt_last;
        ag_err = ag_drv_psram_pm_counters_bw_wr_cnt_last_get(parm[1].value.unumber, &pm_counters_bw_wr_cnt_last);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[0] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[1] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[2] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[3] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[4] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[5] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5]);
        bdmf_session_print(session, "pm_counters_bw_wr_cnt_last[6] = %u = 0x%x\n", pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6]);
        break;
    }
    case cli_psram_pm_counters_bw_rd_cnt_last:
    {
        psram_pm_counters_bw_rd_cnt_last pm_counters_bw_rd_cnt_last;
        ag_err = ag_drv_psram_pm_counters_bw_rd_cnt_last_get(parm[1].value.unumber, &pm_counters_bw_rd_cnt_last);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[0] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[1] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[2] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[3] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[4] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[5] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5]);
        bdmf_session_print(session, "pm_counters_bw_rd_cnt_last[6] = %u = 0x%x\n", pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6]);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_psram_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        psram_pm_counters_arb pm_counters_arb = {.arb_comb_banks_val = gtmv(m, 32), .arb_comb4_val = gtmv(m, 32), .arb_comb_val = gtmv(m, 32), .arb_arb_val = gtmv(m, 32), .arb_req_val = gtmv(m, 32)};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_arb_get(&pm_counters_arb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_arb_get( %u %u %u %u %u)\n",
                pm_counters_arb.arb_comb_banks_val, pm_counters_arb.arb_comb4_val, pm_counters_arb.arb_comb_val, pm_counters_arb.arb_arb_val, 
                pm_counters_arb.arb_req_val);
        }
    }

    {
        uint32_t last_acc_cnt_rd = gtmv(m, 32);
        uint32_t last_acc_cnt_wr = gtmv(m, 32);
        uint32_t acc_cnt_rd = gtmv(m, 32);
        uint32_t acc_cnt_wr = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_cnt_acc_get(&last_acc_cnt_rd, &last_acc_cnt_wr, &acc_cnt_rd, &acc_cnt_wr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_cnt_acc_get( %u %u %u %u)\n",
                last_acc_cnt_rd, last_acc_cnt_wr, acc_cnt_rd, acc_cnt_wr);
        }
    }

    {
        psram_pm_counters_muen pm_counters_muen = {.bwcen = gtmv(m, 1), .cbwcen = gtmv(m, 1), .tw = gtmv(m, 32), .cl0men = gtmv(m, 1), .cl1men = gtmv(m, 1), .cl2men = gtmv(m, 1), .cl3men = gtmv(m, 1), .cl4men = gtmv(m, 1), .cl5men = gtmv(m, 1), .cl6men = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_psram_pm_counters_muen_set( %u %u %u %u %u %u %u %u %u %u)\n",
            pm_counters_muen.bwcen, pm_counters_muen.cbwcen, pm_counters_muen.tw, pm_counters_muen.cl0men, 
            pm_counters_muen.cl1men, pm_counters_muen.cl2men, pm_counters_muen.cl3men, pm_counters_muen.cl4men, 
            pm_counters_muen.cl5men, pm_counters_muen.cl6men);
        ag_err = ag_drv_psram_pm_counters_muen_set(&pm_counters_muen);
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_muen_get(&pm_counters_muen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_muen_get( %u %u %u %u %u %u %u %u %u %u)\n",
                pm_counters_muen.bwcen, pm_counters_muen.cbwcen, pm_counters_muen.tw, pm_counters_muen.cl0men, 
                pm_counters_muen.cl1men, pm_counters_muen.cl2men, pm_counters_muen.cl3men, pm_counters_muen.cl4men, 
                pm_counters_muen.cl5men, pm_counters_muen.cl6men);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pm_counters_muen.bwcen != gtmv(m, 1) || pm_counters_muen.cbwcen != gtmv(m, 1) || pm_counters_muen.tw != gtmv(m, 32) || pm_counters_muen.cl0men != gtmv(m, 1) || pm_counters_muen.cl1men != gtmv(m, 1) || pm_counters_muen.cl2men != gtmv(m, 1) || pm_counters_muen.cl3men != gtmv(m, 1) || pm_counters_muen.cl4men != gtmv(m, 1) || pm_counters_muen.cl5men != gtmv(m, 1) || pm_counters_muen.cl6men != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        psram_cfg_ctrl cfg_ctrl = {.perm_en = gtmv(m, 1), .comb_en = gtmv(m, 1), .comb_full = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_psram_cfg_ctrl_set( %u %u %u)\n",
            cfg_ctrl.perm_en, cfg_ctrl.comb_en, cfg_ctrl.comb_full);
        ag_err = ag_drv_psram_cfg_ctrl_set(&cfg_ctrl);
        if (!ag_err)
            ag_err = ag_drv_psram_cfg_ctrl_get(&cfg_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_cfg_ctrl_get( %u %u %u)\n",
                cfg_ctrl.perm_en, cfg_ctrl.comb_en, cfg_ctrl.comb_full);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_ctrl.perm_en != gtmv(m, 1) || cfg_ctrl.comb_en != gtmv(m, 1) || cfg_ctrl.comb_full != gtmv(m, 1))
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
        bdmf_session_print(session, "ag_drv_psram_configurations_scrm_seed_set( %u)\n",
            val);
        ag_err = ag_drv_psram_configurations_scrm_seed_set(val);
        if (!ag_err)
            ag_err = ag_drv_psram_configurations_scrm_seed_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_configurations_scrm_seed_get( %u)\n",
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
        bdmf_session_print(session, "ag_drv_psram_configurations_scrm_addr_set( %u)\n",
            val);
        ag_err = ag_drv_psram_configurations_scrm_addr_set(val);
        if (!ag_err)
            ag_err = ag_drv_psram_configurations_scrm_addr_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_configurations_scrm_addr_get( %u)\n",
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
        psram_configurations_clk_gate_cntrl configurations_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_psram_configurations_clk_gate_cntrl_set( %u %u %u %u %u)\n",
            configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_intrvl, 
            configurations_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_psram_configurations_clk_gate_cntrl_set(&configurations_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_psram_configurations_clk_gate_cntrl_get(&configurations_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_configurations_clk_gate_cntrl_get( %u %u %u %u %u)\n",
                configurations_clk_gate_cntrl.bypass_clk_gate, configurations_clk_gate_cntrl.timer_val, configurations_clk_gate_cntrl.keep_alive_en, configurations_clk_gate_cntrl.keep_alive_intrvl, 
                configurations_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (configurations_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || configurations_clk_gate_cntrl.timer_val != gtmv(m, 8) || configurations_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || configurations_clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || configurations_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_max_time pm_counters_max_time = {.pm_counters_max_time = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_max_time_get(zero, &pm_counters_max_time);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_max_time_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_max_time.pm_counters_max_time[0], pm_counters_max_time.pm_counters_max_time[1], pm_counters_max_time.pm_counters_max_time[2], pm_counters_max_time.pm_counters_max_time[3], 
                pm_counters_max_time.pm_counters_max_time[4], pm_counters_max_time.pm_counters_max_time[5], pm_counters_max_time.pm_counters_max_time[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_acc_time pm_counters_acc_time = {.pm_counters_acc_time = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_acc_time_get(zero, &pm_counters_acc_time);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_acc_time_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_acc_time.pm_counters_acc_time[0], pm_counters_acc_time.pm_counters_acc_time[1], pm_counters_acc_time.pm_counters_acc_time[2], pm_counters_acc_time.pm_counters_acc_time[3], 
                pm_counters_acc_time.pm_counters_acc_time[4], pm_counters_acc_time.pm_counters_acc_time[5], pm_counters_acc_time.pm_counters_acc_time[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_acc_req pm_counters_acc_req = {.pm_counters_acc_req = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_acc_req_get(zero, &pm_counters_acc_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_acc_req_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_acc_req.pm_counters_acc_req[0], pm_counters_acc_req.pm_counters_acc_req[1], pm_counters_acc_req.pm_counters_acc_req[2], pm_counters_acc_req.pm_counters_acc_req[3], 
                pm_counters_acc_req.pm_counters_acc_req[4], pm_counters_acc_req.pm_counters_acc_req[5], pm_counters_acc_req.pm_counters_acc_req[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_last_acc_time pm_counters_last_acc_time = {.pm_counters_last_acc_time = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_last_acc_time_get(zero, &pm_counters_last_acc_time);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_last_acc_time_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_last_acc_time.pm_counters_last_acc_time[0], pm_counters_last_acc_time.pm_counters_last_acc_time[1], pm_counters_last_acc_time.pm_counters_last_acc_time[2], pm_counters_last_acc_time.pm_counters_last_acc_time[3], 
                pm_counters_last_acc_time.pm_counters_last_acc_time[4], pm_counters_last_acc_time.pm_counters_last_acc_time[5], pm_counters_last_acc_time.pm_counters_last_acc_time[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_last_acc_req pm_counters_last_acc_req = {.pm_counters_last_acc_req = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_last_acc_req_get(zero, &pm_counters_last_acc_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_last_acc_req_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_last_acc_req.pm_counters_last_acc_req[0], pm_counters_last_acc_req.pm_counters_last_acc_req[1], pm_counters_last_acc_req.pm_counters_last_acc_req[2], pm_counters_last_acc_req.pm_counters_last_acc_req[3], 
                pm_counters_last_acc_req.pm_counters_last_acc_req[4], pm_counters_last_acc_req.pm_counters_last_acc_req[5], pm_counters_last_acc_req.pm_counters_last_acc_req[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_bw_wr_cnt pm_counters_bw_wr_cnt = {.pm_counters_bw_wr_cnt = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_bw_wr_cnt_get(zero, &pm_counters_bw_wr_cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_wr_cnt_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[0], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[1], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[2], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[3], 
                pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[4], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[5], pm_counters_bw_wr_cnt.pm_counters_bw_wr_cnt[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_bw_rd_cnt pm_counters_bw_rd_cnt = {.pm_counters_bw_rd_cnt = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_bw_rd_cnt_get(zero, &pm_counters_bw_rd_cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_rd_cnt_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[0], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[1], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[2], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[3], 
                pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[4], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[5], pm_counters_bw_rd_cnt.pm_counters_bw_rd_cnt[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_bw_wr_cnt_last pm_counters_bw_wr_cnt_last = {.pm_counters_bw_wr_cnt_last = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_bw_wr_cnt_last_get(zero, &pm_counters_bw_wr_cnt_last);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_wr_cnt_last_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[0], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[1], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[2], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[3], 
                pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[4], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[5], pm_counters_bw_wr_cnt_last.pm_counters_bw_wr_cnt_last[6]);
        }
    }

    {
        uint32_t zero = gtmv(m, 0);
        psram_pm_counters_bw_rd_cnt_last pm_counters_bw_rd_cnt_last = {.pm_counters_bw_rd_cnt_last = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_psram_pm_counters_bw_rd_cnt_last_get(zero, &pm_counters_bw_rd_cnt_last);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_pm_counters_bw_rd_cnt_last_get( %u %u %u %u %u %u %u %u)\n", zero,
                pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[0], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[1], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[2], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[3], 
                pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[4], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[5], pm_counters_bw_rd_cnt_last.pm_counters_bw_rd_cnt_last[6]);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_psram_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_configurations_ctrl: reg = &RU_REG(PSRAM, CONFIGURATIONS_CTRL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_configurations_scrm_seed: reg = &RU_REG(PSRAM, CONFIGURATIONS_SCRM_SEED); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_configurations_scrm_addr: reg = &RU_REG(PSRAM, CONFIGURATIONS_SCRM_ADDR); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_configurations_clk_gate_cntrl: reg = &RU_REG(PSRAM, CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_muen: reg = &RU_REG(PSRAM, PM_COUNTERS_MUEN); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bwcl: reg = &RU_REG(PSRAM, PM_COUNTERS_BWCL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bwen: reg = &RU_REG(PSRAM, PM_COUNTERS_BWEN); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_max_time: reg = &RU_REG(PSRAM, PM_COUNTERS_MAX_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_acc_time: reg = &RU_REG(PSRAM, PM_COUNTERS_ACC_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_acc_req: reg = &RU_REG(PSRAM, PM_COUNTERS_ACC_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_last_acc_time: reg = &RU_REG(PSRAM, PM_COUNTERS_LAST_ACC_TIME); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_last_acc_req: reg = &RU_REG(PSRAM, PM_COUNTERS_LAST_ACC_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_acc: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_acc: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_last_acc: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_LAST_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_last_acc: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_LAST_ACC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_wr_cnt_last: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_WR_CNT_LAST); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_bw_rd_cnt_last: reg = &RU_REG(PSRAM, PM_COUNTERS_BW_RD_CNT_LAST); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_req: reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_REQ); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_arb: reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_ARB); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb: reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb_4: reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB_4); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_pm_counters_arb_comb_banks: reg = &RU_REG(PSRAM, PM_COUNTERS_ARB_COMB_BANKS); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbgsel: reg = &RU_REG(PSRAM, DEBUG_DBGSEL); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbgbus: reg = &RU_REG(PSRAM, DEBUG_DBGBUS); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_req_vec: reg = &RU_REG(PSRAM, DEBUG_REQ_VEC); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_cfg1: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_CFG1); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_cfg2: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_CFG2); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_st: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_ST); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_w0: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_W0); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_w1: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_W1); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_w2: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_W2); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_w3: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_W3); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_wmsk: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_WMSK); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_r0: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_R0); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_r1: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_R1); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_r2: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_R2); blk = &RU_BLK(PSRAM); break;
    case bdmf_address_debug_dbg_cap_r3: reg = &RU_REG(PSRAM, DEBUG_DBG_CAP_R3); blk = &RU_BLK(PSRAM); break;
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

bdmfmon_handle_t ag_drv_psram_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "psram", "psram", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_pm_counters_muen[] = {
            BDMFMON_MAKE_PARM("bwcen", "bwcen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cbwcen", "cbwcen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tw", "tw", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl0men", "cl0men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl1men", "cl1men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl2men", "cl2men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl3men", "cl3men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl4men", "cl4men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl5men", "cl5men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cl6men", "cl6men", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ctrl[] = {
            BDMFMON_MAKE_PARM("perm_en", "perm_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("comb_en", "comb_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("comb_full", "comb_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_configurations_scrm_seed[] = {
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_configurations_scrm_addr[] = {
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_configurations_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "pm_counters_muen", .val = cli_psram_pm_counters_muen, .parms = set_pm_counters_muen },
            { .name = "cfg_ctrl", .val = cli_psram_cfg_ctrl, .parms = set_cfg_ctrl },
            { .name = "configurations_scrm_seed", .val = cli_psram_configurations_scrm_seed, .parms = set_configurations_scrm_seed },
            { .name = "configurations_scrm_addr", .val = cli_psram_configurations_scrm_addr, .parms = set_configurations_scrm_addr },
            { .name = "configurations_clk_gate_cntrl", .val = cli_psram_configurations_clk_gate_cntrl, .parms = set_configurations_clk_gate_cntrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_psram_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_max_time[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_acc_time[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_acc_req[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_last_acc_time[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_last_acc_req[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_bw_wr_cnt[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_bw_rd_cnt[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_bw_wr_cnt_last[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_bw_rd_cnt_last[] = {
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "pm_counters_arb", .val = cli_psram_pm_counters_arb, .parms = get_default },
            { .name = "pm_counters_cnt_acc", .val = cli_psram_pm_counters_cnt_acc, .parms = get_default },
            { .name = "pm_counters_muen", .val = cli_psram_pm_counters_muen, .parms = get_default },
            { .name = "cfg_ctrl", .val = cli_psram_cfg_ctrl, .parms = get_default },
            { .name = "configurations_scrm_seed", .val = cli_psram_configurations_scrm_seed, .parms = get_default },
            { .name = "configurations_scrm_addr", .val = cli_psram_configurations_scrm_addr, .parms = get_default },
            { .name = "configurations_clk_gate_cntrl", .val = cli_psram_configurations_clk_gate_cntrl, .parms = get_default },
            { .name = "pm_counters_max_time", .val = cli_psram_pm_counters_max_time, .parms = get_pm_counters_max_time },
            { .name = "pm_counters_acc_time", .val = cli_psram_pm_counters_acc_time, .parms = get_pm_counters_acc_time },
            { .name = "pm_counters_acc_req", .val = cli_psram_pm_counters_acc_req, .parms = get_pm_counters_acc_req },
            { .name = "pm_counters_last_acc_time", .val = cli_psram_pm_counters_last_acc_time, .parms = get_pm_counters_last_acc_time },
            { .name = "pm_counters_last_acc_req", .val = cli_psram_pm_counters_last_acc_req, .parms = get_pm_counters_last_acc_req },
            { .name = "pm_counters_bw_wr_cnt", .val = cli_psram_pm_counters_bw_wr_cnt, .parms = get_pm_counters_bw_wr_cnt },
            { .name = "pm_counters_bw_rd_cnt", .val = cli_psram_pm_counters_bw_rd_cnt, .parms = get_pm_counters_bw_rd_cnt },
            { .name = "pm_counters_bw_wr_cnt_last", .val = cli_psram_pm_counters_bw_wr_cnt_last, .parms = get_pm_counters_bw_wr_cnt_last },
            { .name = "pm_counters_bw_rd_cnt_last", .val = cli_psram_pm_counters_bw_rd_cnt_last, .parms = get_pm_counters_bw_rd_cnt_last },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_psram_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_psram_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CONFIGURATIONS_CTRL", .val = bdmf_address_configurations_ctrl },
            { .name = "CONFIGURATIONS_SCRM_SEED", .val = bdmf_address_configurations_scrm_seed },
            { .name = "CONFIGURATIONS_SCRM_ADDR", .val = bdmf_address_configurations_scrm_addr },
            { .name = "CONFIGURATIONS_CLK_GATE_CNTRL", .val = bdmf_address_configurations_clk_gate_cntrl },
            { .name = "PM_COUNTERS_MUEN", .val = bdmf_address_pm_counters_muen },
            { .name = "PM_COUNTERS_BWCL", .val = bdmf_address_pm_counters_bwcl },
            { .name = "PM_COUNTERS_BWEN", .val = bdmf_address_pm_counters_bwen },
            { .name = "PM_COUNTERS_MAX_TIME", .val = bdmf_address_pm_counters_max_time },
            { .name = "PM_COUNTERS_ACC_TIME", .val = bdmf_address_pm_counters_acc_time },
            { .name = "PM_COUNTERS_ACC_REQ", .val = bdmf_address_pm_counters_acc_req },
            { .name = "PM_COUNTERS_LAST_ACC_TIME", .val = bdmf_address_pm_counters_last_acc_time },
            { .name = "PM_COUNTERS_LAST_ACC_REQ", .val = bdmf_address_pm_counters_last_acc_req },
            { .name = "PM_COUNTERS_BW_WR_CNT_ACC", .val = bdmf_address_pm_counters_bw_wr_cnt_acc },
            { .name = "PM_COUNTERS_BW_RD_CNT_ACC", .val = bdmf_address_pm_counters_bw_rd_cnt_acc },
            { .name = "PM_COUNTERS_BW_WR_CNT", .val = bdmf_address_pm_counters_bw_wr_cnt },
            { .name = "PM_COUNTERS_BW_RD_CNT", .val = bdmf_address_pm_counters_bw_rd_cnt },
            { .name = "PM_COUNTERS_BW_WR_CNT_LAST_ACC", .val = bdmf_address_pm_counters_bw_wr_cnt_last_acc },
            { .name = "PM_COUNTERS_BW_RD_CNT_LAST_ACC", .val = bdmf_address_pm_counters_bw_rd_cnt_last_acc },
            { .name = "PM_COUNTERS_BW_WR_CNT_LAST", .val = bdmf_address_pm_counters_bw_wr_cnt_last },
            { .name = "PM_COUNTERS_BW_RD_CNT_LAST", .val = bdmf_address_pm_counters_bw_rd_cnt_last },
            { .name = "PM_COUNTERS_ARB_REQ", .val = bdmf_address_pm_counters_arb_req },
            { .name = "PM_COUNTERS_ARB_ARB", .val = bdmf_address_pm_counters_arb_arb },
            { .name = "PM_COUNTERS_ARB_COMB", .val = bdmf_address_pm_counters_arb_comb },
            { .name = "PM_COUNTERS_ARB_COMB_4", .val = bdmf_address_pm_counters_arb_comb_4 },
            { .name = "PM_COUNTERS_ARB_COMB_BANKS", .val = bdmf_address_pm_counters_arb_comb_banks },
            { .name = "DEBUG_DBGSEL", .val = bdmf_address_debug_dbgsel },
            { .name = "DEBUG_DBGBUS", .val = bdmf_address_debug_dbgbus },
            { .name = "DEBUG_REQ_VEC", .val = bdmf_address_debug_req_vec },
            { .name = "DEBUG_DBG_CAP_CFG1", .val = bdmf_address_debug_dbg_cap_cfg1 },
            { .name = "DEBUG_DBG_CAP_CFG2", .val = bdmf_address_debug_dbg_cap_cfg2 },
            { .name = "DEBUG_DBG_CAP_ST", .val = bdmf_address_debug_dbg_cap_st },
            { .name = "DEBUG_DBG_CAP_W0", .val = bdmf_address_debug_dbg_cap_w0 },
            { .name = "DEBUG_DBG_CAP_W1", .val = bdmf_address_debug_dbg_cap_w1 },
            { .name = "DEBUG_DBG_CAP_W2", .val = bdmf_address_debug_dbg_cap_w2 },
            { .name = "DEBUG_DBG_CAP_W3", .val = bdmf_address_debug_dbg_cap_w3 },
            { .name = "DEBUG_DBG_CAP_WMSK", .val = bdmf_address_debug_dbg_cap_wmsk },
            { .name = "DEBUG_DBG_CAP_R0", .val = bdmf_address_debug_dbg_cap_r0 },
            { .name = "DEBUG_DBG_CAP_R1", .val = bdmf_address_debug_dbg_cap_r1 },
            { .name = "DEBUG_DBG_CAP_R2", .val = bdmf_address_debug_dbg_cap_r2 },
            { .name = "DEBUG_DBG_CAP_R3", .val = bdmf_address_debug_dbg_cap_r3 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_psram_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
