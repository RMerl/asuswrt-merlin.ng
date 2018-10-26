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
#include "xrdp_drv_bac_if_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(uint8_t bacif_id, uint8_t thr)
{
    uint32_t reg_bacif_block_bacif_configurations_rslt_f_full_thr=0;

#ifdef VALIDATE_PARMS
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (thr >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bacif_block_bacif_configurations_rslt_f_full_thr = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR, THR, reg_bacif_block_bacif_configurations_rslt_f_full_thr, thr);

    RU_REG_WRITE(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR, reg_bacif_block_bacif_configurations_rslt_f_full_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get(uint8_t bacif_id, uint8_t *thr)
{
    uint32_t reg_bacif_block_bacif_configurations_rslt_f_full_thr;

#ifdef VALIDATE_PARMS
    if(!thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR, reg_bacif_block_bacif_configurations_rslt_f_full_thr);

    *thr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR, THR, reg_bacif_block_bacif_configurations_rslt_f_full_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(uint8_t bacif_id, bdmf_boolean en, uint8_t id, uint16_t addr)
{
    uint32_t reg_bacif_block_bacif_configurations_dec_rout_ovride=0;

#ifdef VALIDATE_PARMS
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_) ||
       (id >= _6BITS_MAX_VAL_) ||
       (addr >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bacif_block_bacif_configurations_dec_rout_ovride = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, EN, reg_bacif_block_bacif_configurations_dec_rout_ovride, en);
    reg_bacif_block_bacif_configurations_dec_rout_ovride = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, ID, reg_bacif_block_bacif_configurations_dec_rout_ovride, id);
    reg_bacif_block_bacif_configurations_dec_rout_ovride = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, ADDR, reg_bacif_block_bacif_configurations_dec_rout_ovride, addr);

    RU_REG_WRITE(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, reg_bacif_block_bacif_configurations_dec_rout_ovride);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get(uint8_t bacif_id, bdmf_boolean *en, uint8_t *id, uint16_t *addr)
{
    uint32_t reg_bacif_block_bacif_configurations_dec_rout_ovride;

#ifdef VALIDATE_PARMS
    if(!en || !id || !addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, reg_bacif_block_bacif_configurations_dec_rout_ovride);

    *en = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, EN, reg_bacif_block_bacif_configurations_dec_rout_ovride);
    *id = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, ID, reg_bacif_block_bacif_configurations_dec_rout_ovride);
    *addr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE, ADDR, reg_bacif_block_bacif_configurations_dec_rout_ovride);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(uint8_t bacif_id, const bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl)
{
    uint32_t reg_bacif_block_bacif_configurations_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!bacif_block_bacif_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (bacif_block_bacif_configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bacif_block_bacif_configurations_clk_gate_cntrl = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_bacif_block_bacif_configurations_clk_gate_cntrl, bacif_block_bacif_configurations_clk_gate_cntrl->bypass_clk_gate);
    reg_bacif_block_bacif_configurations_clk_gate_cntrl = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_bacif_block_bacif_configurations_clk_gate_cntrl, bacif_block_bacif_configurations_clk_gate_cntrl->timer_val);
    reg_bacif_block_bacif_configurations_clk_gate_cntrl = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_bacif_block_bacif_configurations_clk_gate_cntrl, bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_en);
    reg_bacif_block_bacif_configurations_clk_gate_cntrl = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_bacif_block_bacif_configurations_clk_gate_cntrl, bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_intrvl);
    reg_bacif_block_bacif_configurations_clk_gate_cntrl = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_bacif_block_bacif_configurations_clk_gate_cntrl, bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, reg_bacif_block_bacif_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(uint8_t bacif_id, bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl)
{
    uint32_t reg_bacif_block_bacif_configurations_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!bacif_block_bacif_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, reg_bacif_block_bacif_configurations_clk_gate_cntrl);

    bacif_block_bacif_configurations_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_bacif_block_bacif_configurations_clk_gate_cntrl);
    bacif_block_bacif_configurations_clk_gate_cntrl->timer_val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_bacif_block_bacif_configurations_clk_gate_cntrl);
    bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_bacif_block_bacif_configurations_clk_gate_cntrl);
    bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_bacif_block_bacif_configurations_clk_gate_cntrl);
    bacif_block_bacif_configurations_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_bacif_block_bacif_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val)
{
    uint32_t reg_bacif_block_bacif_fifos_ingfifo;

#ifdef VALIDATE_PARMS
    if(!entry || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (fifo_idx >= 128))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bacif_id, fifo_idx, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_INGFIFO, reg_bacif_block_bacif_fifos_ingfifo);

    *entry = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_INGFIFO, ENTRY, reg_bacif_block_bacif_fifos_ingfifo);
    *val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_INGFIFO, VAL, reg_bacif_block_bacif_fifos_ingfifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val)
{
    uint32_t reg_bacif_block_bacif_fifos_cmdfifo;

#ifdef VALIDATE_PARMS
    if(!entry || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (fifo_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bacif_id, fifo_idx, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_CMDFIFO, reg_bacif_block_bacif_fifos_cmdfifo);

    *entry = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_CMDFIFO, ENTRY, reg_bacif_block_bacif_fifos_cmdfifo);
    *val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_CMDFIFO, VAL, reg_bacif_block_bacif_fifos_cmdfifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val)
{
    uint32_t reg_bacif_block_bacif_fifos_rsltfifo;

#ifdef VALIDATE_PARMS
    if(!entry || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (fifo_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bacif_id, fifo_idx, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO, reg_bacif_block_bacif_fifos_rsltfifo);

    *entry = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO, ENTRY, reg_bacif_block_bacif_fifos_rsltfifo);
    *val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO, VAL, reg_bacif_block_bacif_fifos_rsltfifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val)
{
    uint32_t reg_bacif_block_bacif_fifos_egfifo;

#ifdef VALIDATE_PARMS
    if(!entry || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (fifo_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bacif_id, fifo_idx, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_EGFIFO, reg_bacif_block_bacif_fifos_egfifo);

    *entry = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_EGFIFO, ENTRY, reg_bacif_block_bacif_fifos_egfifo);
    *val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_EGFIFO, VAL, reg_bacif_block_bacif_fifos_egfifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val)
{
    uint32_t reg_bacif_block_bacif_fifos_rpprmarr;

#ifdef VALIDATE_PARMS
    if(!entry || !val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (fifo_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bacif_id, fifo_idx, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RPPRMARR, reg_bacif_block_bacif_fifos_rpprmarr);

    *entry = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RPPRMARR, ENTRY, reg_bacif_block_bacif_fifos_rpprmarr);
    *val = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RPPRMARR, VAL, reg_bacif_block_bacif_fifos_rpprmarr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_ing_f_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT, reg_bacif_block_bacif_pm_counters_ing_f_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT, CNTR, reg_bacif_block_bacif_pm_counters_ing_f_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_cmd_f_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT, reg_bacif_block_bacif_pm_counters_cmd_f_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT, CNTR, reg_bacif_block_bacif_pm_counters_cmd_f_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_eng_cmd_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT, reg_bacif_block_bacif_pm_counters_eng_cmd_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT, CNTR, reg_bacif_block_bacif_pm_counters_eng_cmd_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_eng_rslt_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT, reg_bacif_block_bacif_pm_counters_eng_rslt_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT, CNTR, reg_bacif_block_bacif_pm_counters_eng_rslt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_rslt_f_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT, reg_bacif_block_bacif_pm_counters_rslt_f_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT, CNTR, reg_bacif_block_bacif_pm_counters_rslt_f_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_egr_f_cnt;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT, reg_bacif_block_bacif_pm_counters_egr_f_cnt);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT, CNTR, reg_bacif_block_bacif_pm_counters_egr_f_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_err_cmdlng_c;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C, reg_bacif_block_bacif_pm_counters_err_cmdlng_c);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C, CNTR, reg_bacif_block_bacif_pm_counters_err_cmdlng_c);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_err_params_of_c;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C, reg_bacif_block_bacif_pm_counters_err_params_of_c);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C, CNTR, reg_bacif_block_bacif_pm_counters_err_params_of_c);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get(uint8_t bacif_id, uint32_t *cntr)
{
    uint32_t reg_bacif_block_bacif_pm_counters_err_params_uf_c;

#ifdef VALIDATE_PARMS
    if(!cntr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C, reg_bacif_block_bacif_pm_counters_err_params_uf_c);

    *cntr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C, CNTR, reg_bacif_block_bacif_pm_counters_err_params_uf_c);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(uint8_t bacif_id, bdmf_boolean rd_clr, bdmf_boolean wrap)
{
    uint32_t reg_bacif_block_bacif_pm_counters_gen_cfg=0;

#ifdef VALIDATE_PARMS
    if((bacif_id >= BLOCK_ADDR_COUNT) ||
       (rd_clr >= _1BITS_MAX_VAL_) ||
       (wrap >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bacif_block_bacif_pm_counters_gen_cfg = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, RD_CLR, reg_bacif_block_bacif_pm_counters_gen_cfg, rd_clr);
    reg_bacif_block_bacif_pm_counters_gen_cfg = RU_FIELD_SET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, WRAP, reg_bacif_block_bacif_pm_counters_gen_cfg, wrap);

    RU_REG_WRITE(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, reg_bacif_block_bacif_pm_counters_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get(uint8_t bacif_id, bdmf_boolean *rd_clr, bdmf_boolean *wrap)
{
    uint32_t reg_bacif_block_bacif_pm_counters_gen_cfg;

#ifdef VALIDATE_PARMS
    if(!rd_clr || !wrap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bacif_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, reg_bacif_block_bacif_pm_counters_gen_cfg);

    *rd_clr = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, RD_CLR, reg_bacif_block_bacif_pm_counters_gen_cfg);
    *wrap = RU_FIELD_GET(bacif_id, BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG, WRAP, reg_bacif_block_bacif_pm_counters_gen_cfg);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_bacif_block_bacif_configurations_rslt_f_full_thr,
    bdmf_address_bacif_block_bacif_configurations_dec_rout_ovride,
    bdmf_address_bacif_block_bacif_configurations_clk_gate_cntrl,
    bdmf_address_bacif_block_bacif_fifos_ingfifo,
    bdmf_address_bacif_block_bacif_fifos_cmdfifo,
    bdmf_address_bacif_block_bacif_fifos_rsltfifo,
    bdmf_address_bacif_block_bacif_fifos_egfifo,
    bdmf_address_bacif_block_bacif_fifos_rpprmarr,
    bdmf_address_bacif_block_bacif_pm_counters_ing_f_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_cmd_f_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_eng_cmd_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_eng_rslt_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_rslt_f_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_egr_f_cnt,
    bdmf_address_bacif_block_bacif_pm_counters_err_cmdlng_c,
    bdmf_address_bacif_block_bacif_pm_counters_err_params_of_c,
    bdmf_address_bacif_block_bacif_pm_counters_err_params_uf_c,
    bdmf_address_bacif_block_bacif_pm_counters_gen_cfg,
}
bdmf_address;

static int bcm_bac_if_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr:
        err = ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride:
        err = ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl:
    {
        bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_block_bacif_configurations_clk_gate_cntrl = { .bypass_clk_gate=parm[2].value.unumber, .timer_val=parm[3].value.unumber, .keep_alive_en=parm[4].value.unumber, .keep_alive_intrvl=parm[5].value.unumber, .keep_alive_cyc=parm[6].value.unumber};
        err = ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(parm[1].value.unumber, &bacif_block_bacif_configurations_clk_gate_cntrl);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_gen_cfg:
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_bac_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr:
    {
        uint8_t thr;
        err = ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get(parm[1].value.unumber, &thr);
        bdmf_session_print(session, "thr = %u (0x%x)\n", thr, thr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride:
    {
        bdmf_boolean en;
        uint8_t id;
        uint16_t addr;
        err = ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get(parm[1].value.unumber, &en, &id, &addr);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        bdmf_session_print(session, "id = %u (0x%x)\n", id, id);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl:
    {
        bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_block_bacif_configurations_clk_gate_cntrl;
        err = ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(parm[1].value.unumber, &bacif_block_bacif_configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", bacif_block_bacif_configurations_clk_gate_cntrl.bypass_clk_gate, bacif_block_bacif_configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", bacif_block_bacif_configurations_clk_gate_cntrl.timer_val, bacif_block_bacif_configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_en, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_intrvl, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_cyc, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_bac_if_bacif_block_bacif_fifos_ingfifo:
    {
        uint32_t entry;
        bdmf_boolean val;
        err = ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get(parm[1].value.unumber, parm[2].value.unumber, &entry, &val);
        bdmf_session_print(session, "entry = %u (0x%x)\n", entry, entry);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_bac_if_bacif_block_bacif_fifos_cmdfifo:
    {
        uint32_t entry;
        bdmf_boolean val;
        err = ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get(parm[1].value.unumber, parm[2].value.unumber, &entry, &val);
        bdmf_session_print(session, "entry = %u (0x%x)\n", entry, entry);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_bac_if_bacif_block_bacif_fifos_rsltfifo:
    {
        uint32_t entry;
        bdmf_boolean val;
        err = ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get(parm[1].value.unumber, parm[2].value.unumber, &entry, &val);
        bdmf_session_print(session, "entry = %u (0x%x)\n", entry, entry);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_bac_if_bacif_block_bacif_fifos_egfifo:
    {
        uint32_t entry;
        bdmf_boolean val;
        err = ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get(parm[1].value.unumber, parm[2].value.unumber, &entry, &val);
        bdmf_session_print(session, "entry = %u (0x%x)\n", entry, entry);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_bac_if_bacif_block_bacif_fifos_rpprmarr:
    {
        uint32_t entry;
        bdmf_boolean val;
        err = ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get(parm[1].value.unumber, parm[2].value.unumber, &entry, &val);
        bdmf_session_print(session, "entry = %u (0x%x)\n", entry, entry);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_err_params_of_c:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c:
    {
        uint32_t cntr;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get(parm[1].value.unumber, &cntr);
        bdmf_session_print(session, "cntr = %u (0x%x)\n", cntr, cntr);
        break;
    }
    case cli_bac_if_bacif_block_bacif_pm_counters_gen_cfg:
    {
        bdmf_boolean rd_clr;
        bdmf_boolean wrap;
        err = ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get(parm[1].value.unumber, &rd_clr, &wrap);
        bdmf_session_print(session, "rd_clr = %u (0x%x)\n", rd_clr, rd_clr);
        bdmf_session_print(session, "wrap = %u (0x%x)\n", wrap, wrap);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_bac_if_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t bacif_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t thr=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(%u %u)\n", bacif_id, thr);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(bacif_id, thr);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get( bacif_id, &thr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get(%u %u)\n", bacif_id, thr);
        if(err || thr!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        uint8_t id=gtmv(m, 6);
        uint16_t addr=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(%u %u %u %u)\n", bacif_id, en, id, addr);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(bacif_id, en, id, addr);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get( bacif_id, &en, &id, &addr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get(%u %u %u %u)\n", bacif_id, en, id, addr);
        if(err || en!=gtmv(m, 1) || id!=gtmv(m, 6) || addr!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_block_bacif_configurations_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", bacif_id, bacif_block_bacif_configurations_clk_gate_cntrl.bypass_clk_gate, bacif_block_bacif_configurations_clk_gate_cntrl.timer_val, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_en, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_intrvl, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(bacif_id, &bacif_block_bacif_configurations_clk_gate_cntrl);
        if(!err) ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get( bacif_id, &bacif_block_bacif_configurations_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", bacif_id, bacif_block_bacif_configurations_clk_gate_cntrl.bypass_clk_gate, bacif_block_bacif_configurations_clk_gate_cntrl.timer_val, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_en, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_intrvl, bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(err || bacif_block_bacif_configurations_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || bacif_block_bacif_configurations_clk_gate_cntrl.timer_val!=gtmv(m, 8) || bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || bacif_block_bacif_configurations_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t fifo_idx=gtmv(m, 7);
        uint32_t entry=gtmv(m, 31);
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get( bacif_id, fifo_idx, &entry, &val);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get(%u %u %u %u)\n", bacif_id, fifo_idx, entry, val);
    }
    {
        uint8_t fifo_idx=gtmv(m, 5);
        uint32_t entry=gtmv(m, 31);
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get( bacif_id, fifo_idx, &entry, &val);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get(%u %u %u %u)\n", bacif_id, fifo_idx, entry, val);
    }
    {
        uint8_t fifo_idx=gtmv(m, 5);
        uint32_t entry=gtmv(m, 31);
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get( bacif_id, fifo_idx, &entry, &val);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get(%u %u %u %u)\n", bacif_id, fifo_idx, entry, val);
    }
    {
        uint8_t fifo_idx=gtmv(m, 3);
        uint32_t entry=gtmv(m, 31);
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get( bacif_id, fifo_idx, &entry, &val);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get(%u %u %u %u)\n", bacif_id, fifo_idx, entry, val);
    }
    {
        uint8_t fifo_idx=gtmv(m, 3);
        uint32_t entry=gtmv(m, 31);
        bdmf_boolean val=gtmv(m, 1);
        if(!err) ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get( bacif_id, fifo_idx, &entry, &val);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get(%u %u %u %u)\n", bacif_id, fifo_idx, entry, val);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get(%u %u)\n", bacif_id, cntr);
    }
    {
        uint32_t cntr=gtmv(m, 32);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get( bacif_id, &cntr);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get(%u %u)\n", bacif_id, cntr);
    }
    {
        bdmf_boolean rd_clr=gtmv(m, 1);
        bdmf_boolean wrap=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(%u %u %u)\n", bacif_id, rd_clr, wrap);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(bacif_id, rd_clr, wrap);
        if(!err) ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get( bacif_id, &rd_clr, &wrap);
        if(!err) bdmf_session_print(session, "ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get(%u %u %u)\n", bacif_id, rd_clr, wrap);
        if(err || rd_clr!=gtmv(m, 1) || wrap!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_bac_if_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_bacif_block_bacif_configurations_rslt_f_full_thr : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_configurations_dec_rout_ovride : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_configurations_clk_gate_cntrl : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_fifos_ingfifo : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_FIFOS_INGFIFO); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_fifos_cmdfifo : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_FIFOS_CMDFIFO); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_fifos_rsltfifo : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_fifos_egfifo : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_FIFOS_EGFIFO); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_fifos_rpprmarr : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_FIFOS_RPPRMARR); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_ing_f_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_cmd_f_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_eng_cmd_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_eng_rslt_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_rslt_f_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_egr_f_cnt : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_err_cmdlng_c : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_err_params_of_c : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_err_params_uf_c : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C); blk = &RU_BLK(BAC_IF); break;
    case bdmf_address_bacif_block_bacif_pm_counters_gen_cfg : reg = &RU_REG(BAC_IF, BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG); blk = &RU_BLK(BAC_IF); break;
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

bdmfmon_handle_t ag_drv_bac_if_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "bac_if"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "bac_if", "bac_if", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_configurations_rslt_f_full_thr[]={
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_MAKE_PARM("thr", "thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_configurations_dec_rout_ovride[]={
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("id", "id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_configurations_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_pm_counters_gen_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rd_clr", "rd_clr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="bacif_block_bacif_configurations_rslt_f_full_thr", .val=cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr, .parms=set_bacif_block_bacif_configurations_rslt_f_full_thr },
            { .name="bacif_block_bacif_configurations_dec_rout_ovride", .val=cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride, .parms=set_bacif_block_bacif_configurations_dec_rout_ovride },
            { .name="bacif_block_bacif_configurations_clk_gate_cntrl", .val=cli_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl, .parms=set_bacif_block_bacif_configurations_clk_gate_cntrl },
            { .name="bacif_block_bacif_pm_counters_gen_cfg", .val=cli_bac_if_bacif_block_bacif_pm_counters_gen_cfg, .parms=set_bacif_block_bacif_pm_counters_gen_cfg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_bac_if_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_fifos_ingfifo[]={
            BDMFMON_MAKE_PARM("bacif_id", "bacif_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_fifos_cmdfifo[]={
            BDMFMON_MAKE_PARM("bacif_id", "bacif_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_fifos_rsltfifo[]={
            BDMFMON_MAKE_PARM("bacif_id", "bacif_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_fifos_egfifo[]={
            BDMFMON_MAKE_PARM("bacif_id", "bacif_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bacif_block_bacif_fifos_rpprmarr[]={
            BDMFMON_MAKE_PARM("bacif_id", "bacif_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="bacif_block_bacif_configurations_rslt_f_full_thr", .val=cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr, .parms=set_default },
            { .name="bacif_block_bacif_configurations_dec_rout_ovride", .val=cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride, .parms=set_default },
            { .name="bacif_block_bacif_configurations_clk_gate_cntrl", .val=cli_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl, .parms=set_default },
            { .name="bacif_block_bacif_fifos_ingfifo", .val=cli_bac_if_bacif_block_bacif_fifos_ingfifo, .parms=set_bacif_block_bacif_fifos_ingfifo },
            { .name="bacif_block_bacif_fifos_cmdfifo", .val=cli_bac_if_bacif_block_bacif_fifos_cmdfifo, .parms=set_bacif_block_bacif_fifos_cmdfifo },
            { .name="bacif_block_bacif_fifos_rsltfifo", .val=cli_bac_if_bacif_block_bacif_fifos_rsltfifo, .parms=set_bacif_block_bacif_fifos_rsltfifo },
            { .name="bacif_block_bacif_fifos_egfifo", .val=cli_bac_if_bacif_block_bacif_fifos_egfifo, .parms=set_bacif_block_bacif_fifos_egfifo },
            { .name="bacif_block_bacif_fifos_rpprmarr", .val=cli_bac_if_bacif_block_bacif_fifos_rpprmarr, .parms=set_bacif_block_bacif_fifos_rpprmarr },
            { .name="bacif_block_bacif_pm_counters_ing_f_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_cmd_f_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_eng_cmd_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_eng_rslt_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_rslt_f_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_egr_f_cnt", .val=cli_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_err_cmdlng_c", .val=cli_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_err_params_of_c", .val=cli_bac_if_bacif_block_bacif_pm_counters_err_params_of_c, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_err_params_uf_c", .val=cli_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c, .parms=set_default },
            { .name="bacif_block_bacif_pm_counters_gen_cfg", .val=cli_bac_if_bacif_block_bacif_pm_counters_gen_cfg, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_bac_if_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_bac_if_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("bacif_id", "bacif_id", bacif_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="BACIF_BLOCK_BACIF_CONFIGURATIONS_RSLT_F_FULL_THR" , .val=bdmf_address_bacif_block_bacif_configurations_rslt_f_full_thr },
            { .name="BACIF_BLOCK_BACIF_CONFIGURATIONS_DEC_ROUT_OVRIDE" , .val=bdmf_address_bacif_block_bacif_configurations_dec_rout_ovride },
            { .name="BACIF_BLOCK_BACIF_CONFIGURATIONS_CLK_GATE_CNTRL" , .val=bdmf_address_bacif_block_bacif_configurations_clk_gate_cntrl },
            { .name="BACIF_BLOCK_BACIF_FIFOS_INGFIFO" , .val=bdmf_address_bacif_block_bacif_fifos_ingfifo },
            { .name="BACIF_BLOCK_BACIF_FIFOS_CMDFIFO" , .val=bdmf_address_bacif_block_bacif_fifos_cmdfifo },
            { .name="BACIF_BLOCK_BACIF_FIFOS_RSLTFIFO" , .val=bdmf_address_bacif_block_bacif_fifos_rsltfifo },
            { .name="BACIF_BLOCK_BACIF_FIFOS_EGFIFO" , .val=bdmf_address_bacif_block_bacif_fifos_egfifo },
            { .name="BACIF_BLOCK_BACIF_FIFOS_RPPRMARR" , .val=bdmf_address_bacif_block_bacif_fifos_rpprmarr },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ING_F_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_ing_f_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_CMD_F_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_cmd_f_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_CMD_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_eng_cmd_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ENG_RSLT_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_eng_rslt_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_RSLT_F_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_rslt_f_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_EGR_F_CNT" , .val=bdmf_address_bacif_block_bacif_pm_counters_egr_f_cnt },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_CMDLNG_C" , .val=bdmf_address_bacif_block_bacif_pm_counters_err_cmdlng_c },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_OF_C" , .val=bdmf_address_bacif_block_bacif_pm_counters_err_params_of_c },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_ERR_PARAMS_UF_C" , .val=bdmf_address_bacif_block_bacif_pm_counters_err_params_uf_c },
            { .name="BACIF_BLOCK_BACIF_PM_COUNTERS_GEN_CFG" , .val=bdmf_address_bacif_block_bacif_pm_counters_gen_cfg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_bac_if_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "bacif_id", bacif_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

