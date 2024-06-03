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
#include "xrdp_drv_ubus_resp_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_isr_set(uint32_t ist)
{
    uint32_t reg_rnr_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_isr = RU_FIELD_SET(0, UBUS_RESP, RNR_INTR_CTRL_ISR, IST, reg_rnr_intr_ctrl_isr, ist);

    RU_REG_WRITE(0, UBUS_RESP, RNR_INTR_CTRL_ISR, reg_rnr_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_isr_get(uint32_t *ist)
{
    uint32_t reg_rnr_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RNR_INTR_CTRL_ISR, reg_rnr_intr_ctrl_isr);

    *ist = RU_FIELD_GET(0, UBUS_RESP, RNR_INTR_CTRL_ISR, IST, reg_rnr_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_rnr_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RNR_INTR_CTRL_ISM, reg_rnr_intr_ctrl_ism);

    *ism = RU_FIELD_GET(0, UBUS_RESP, RNR_INTR_CTRL_ISM, ISM, reg_rnr_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ier_set(uint32_t iem)
{
    uint32_t reg_rnr_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_ier = RU_FIELD_SET(0, UBUS_RESP, RNR_INTR_CTRL_IER, IEM, reg_rnr_intr_ctrl_ier, iem);

    RU_REG_WRITE(0, UBUS_RESP, RNR_INTR_CTRL_IER, reg_rnr_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ier_get(uint32_t *iem)
{
    uint32_t reg_rnr_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RNR_INTR_CTRL_IER, reg_rnr_intr_ctrl_ier);

    *iem = RU_FIELD_GET(0, UBUS_RESP, RNR_INTR_CTRL_IER, IEM, reg_rnr_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_itr_set(uint32_t ist)
{
    uint32_t reg_rnr_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_itr = RU_FIELD_SET(0, UBUS_RESP, RNR_INTR_CTRL_ITR, IST, reg_rnr_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, UBUS_RESP, RNR_INTR_CTRL_ITR, reg_rnr_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_itr_get(uint32_t *ist)
{
    uint32_t reg_rnr_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RNR_INTR_CTRL_ITR, reg_rnr_intr_ctrl_itr);

    *ist = RU_FIELD_GET(0, UBUS_RESP, RNR_INTR_CTRL_ITR, IST, reg_rnr_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_cfg_set(bdmf_boolean counter_enable, bdmf_boolean profiling_start, bdmf_boolean manual_stop_mode, bdmf_boolean do_manual_stop)
{
    uint32_t reg_profiling_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((counter_enable >= _1BITS_MAX_VAL_) ||
       (profiling_start >= _1BITS_MAX_VAL_) ||
       (manual_stop_mode >= _1BITS_MAX_VAL_) ||
       (do_manual_stop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_RESP, PROFILING_CFG, COUNTER_ENABLE, reg_profiling_cfg, counter_enable);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_RESP, PROFILING_CFG, PROFILING_START, reg_profiling_cfg, profiling_start);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_RESP, PROFILING_CFG, MANUAL_STOP_MODE, reg_profiling_cfg, manual_stop_mode);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_RESP, PROFILING_CFG, DO_MANUAL_STOP, reg_profiling_cfg, do_manual_stop);

    RU_REG_WRITE(0, UBUS_RESP, PROFILING_CFG, reg_profiling_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_cfg_get(bdmf_boolean *counter_enable, bdmf_boolean *profiling_start, bdmf_boolean *manual_stop_mode, bdmf_boolean *do_manual_stop)
{
    uint32_t reg_profiling_cfg;

#ifdef VALIDATE_PARMS
    if (!counter_enable || !profiling_start || !manual_stop_mode || !do_manual_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_CFG, reg_profiling_cfg);

    *counter_enable = RU_FIELD_GET(0, UBUS_RESP, PROFILING_CFG, COUNTER_ENABLE, reg_profiling_cfg);
    *profiling_start = RU_FIELD_GET(0, UBUS_RESP, PROFILING_CFG, PROFILING_START, reg_profiling_cfg);
    *manual_stop_mode = RU_FIELD_GET(0, UBUS_RESP, PROFILING_CFG, MANUAL_STOP_MODE, reg_profiling_cfg);
    *do_manual_stop = RU_FIELD_GET(0, UBUS_RESP, PROFILING_CFG, DO_MANUAL_STOP, reg_profiling_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_status_get(bdmf_boolean *profiling_on, uint32_t *cycles_counter)
{
    uint32_t reg_profiling_status;

#ifdef VALIDATE_PARMS
    if (!profiling_on || !cycles_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_STATUS, reg_profiling_status);

    *profiling_on = RU_FIELD_GET(0, UBUS_RESP, PROFILING_STATUS, PROFILING_ON, reg_profiling_status);
    *cycles_counter = RU_FIELD_GET(0, UBUS_RESP, PROFILING_STATUS, CYCLES_COUNTER, reg_profiling_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_counter_get(uint32_t *val)
{
    uint32_t reg_profiling_counter;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_COUNTER, reg_profiling_counter);

    *val = RU_FIELD_GET(0, UBUS_RESP, PROFILING_COUNTER, VAL, reg_profiling_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_start_value_get(uint32_t *val)
{
    uint32_t reg_profiling_start_value;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_START_VALUE, reg_profiling_start_value);

    *val = RU_FIELD_GET(0, UBUS_RESP, PROFILING_START_VALUE, VAL, reg_profiling_start_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_stop_value_get(uint32_t *val)
{
    uint32_t reg_profiling_stop_value;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_STOP_VALUE, reg_profiling_stop_value);

    *val = RU_FIELD_GET(0, UBUS_RESP, PROFILING_STOP_VALUE, VAL, reg_profiling_stop_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_cycle_num_set(uint32_t profiling_cycles_num)
{
    uint32_t reg_profiling_cycle_num = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_profiling_cycle_num = RU_FIELD_SET(0, UBUS_RESP, PROFILING_CYCLE_NUM, PROFILING_CYCLES_NUM, reg_profiling_cycle_num, profiling_cycles_num);

    RU_REG_WRITE(0, UBUS_RESP, PROFILING_CYCLE_NUM, reg_profiling_cycle_num);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_profiling_cycle_num_get(uint32_t *profiling_cycles_num)
{
    uint32_t reg_profiling_cycle_num;

#ifdef VALIDATE_PARMS
    if (!profiling_cycles_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, PROFILING_CYCLE_NUM, reg_profiling_cycle_num);

    *profiling_cycles_num = RU_FIELD_GET(0, UBUS_RESP, PROFILING_CYCLE_NUM, PROFILING_CYCLES_NUM, reg_profiling_cycle_num);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_lock_set(bdmf_boolean rchk_lock)
{
    uint32_t reg_rchk_lock = 0;

#ifdef VALIDATE_PARMS
    if ((rchk_lock >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_lock = RU_FIELD_SET(0, UBUS_RESP, RCHK_LOCK, RCHK_LOCK, reg_rchk_lock, rchk_lock);

    RU_REG_WRITE(0, UBUS_RESP, RCHK_LOCK, reg_rchk_lock);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_lock_get(bdmf_boolean *rchk_lock)
{
    uint32_t reg_rchk_lock;

#ifdef VALIDATE_PARMS
    if (!rchk_lock)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RCHK_LOCK, reg_rchk_lock);

    *rchk_lock = RU_FIELD_GET(0, UBUS_RESP, RCHK_LOCK, RCHK_LOCK, reg_rchk_lock);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_ctrl_set(uint8_t idx, bdmf_boolean wr_abort, bdmf_boolean rd_abort, uint8_t prot, uint8_t prot_msk)
{
    uint32_t reg_rchk_eng_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8) ||
       (wr_abort >= _1BITS_MAX_VAL_) ||
       (rd_abort >= _1BITS_MAX_VAL_) ||
       (prot >= _3BITS_MAX_VAL_) ||
       (prot_msk >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_eng_ctrl = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_CTRL, WR_ABORT, reg_rchk_eng_ctrl, wr_abort);
    reg_rchk_eng_ctrl = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_CTRL, RD_ABORT, reg_rchk_eng_ctrl, rd_abort);
    reg_rchk_eng_ctrl = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_CTRL, PROT, reg_rchk_eng_ctrl, prot);
    reg_rchk_eng_ctrl = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_CTRL, PROT_MSK, reg_rchk_eng_ctrl, prot_msk);

    RU_REG_RAM_WRITE(0, idx, UBUS_RESP, RCHK_ENG_CTRL, reg_rchk_eng_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_ctrl_get(uint8_t idx, bdmf_boolean *wr_abort, bdmf_boolean *rd_abort, uint8_t *prot, uint8_t *prot_msk)
{
    uint32_t reg_rchk_eng_ctrl;

#ifdef VALIDATE_PARMS
    if (!wr_abort || !rd_abort || !prot || !prot_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, UBUS_RESP, RCHK_ENG_CTRL, reg_rchk_eng_ctrl);

    *wr_abort = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_CTRL, WR_ABORT, reg_rchk_eng_ctrl);
    *rd_abort = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_CTRL, RD_ABORT, reg_rchk_eng_ctrl);
    *prot = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_CTRL, PROT, reg_rchk_eng_ctrl);
    *prot_msk = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_CTRL, PROT_MSK, reg_rchk_eng_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_start_add_set(uint8_t idx, uint32_t start_add)
{
    uint32_t reg_rchk_eng_start_add = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8) ||
       (start_add >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_eng_start_add = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_START_ADD, START_ADD, reg_rchk_eng_start_add, start_add);

    RU_REG_RAM_WRITE(0, idx, UBUS_RESP, RCHK_ENG_START_ADD, reg_rchk_eng_start_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_start_add_get(uint8_t idx, uint32_t *start_add)
{
    uint32_t reg_rchk_eng_start_add;

#ifdef VALIDATE_PARMS
    if (!start_add)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, UBUS_RESP, RCHK_ENG_START_ADD, reg_rchk_eng_start_add);

    *start_add = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_START_ADD, START_ADD, reg_rchk_eng_start_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_end_add_set(uint8_t idx, uint32_t end_add)
{
    uint32_t reg_rchk_eng_end_add = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8) ||
       (end_add >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_eng_end_add = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_END_ADD, END_ADD, reg_rchk_eng_end_add, end_add);

    RU_REG_RAM_WRITE(0, idx, UBUS_RESP, RCHK_ENG_END_ADD, reg_rchk_eng_end_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_end_add_get(uint8_t idx, uint32_t *end_add)
{
    uint32_t reg_rchk_eng_end_add;

#ifdef VALIDATE_PARMS
    if (!end_add)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, UBUS_RESP, RCHK_ENG_END_ADD, reg_rchk_eng_end_add);

    *end_add = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_END_ADD, END_ADD, reg_rchk_eng_end_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_seclev_en_set(uint8_t idx, uint32_t seclev_en)
{
    uint32_t reg_rchk_eng_seclev_en = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_eng_seclev_en = RU_FIELD_SET(0, UBUS_RESP, RCHK_ENG_SECLEV_EN, SECLEV_EN, reg_rchk_eng_seclev_en, seclev_en);

    RU_REG_RAM_WRITE(0, idx, UBUS_RESP, RCHK_ENG_SECLEV_EN, reg_rchk_eng_seclev_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_eng_seclev_en_get(uint8_t idx, uint32_t *seclev_en)
{
    uint32_t reg_rchk_eng_seclev_en;

#ifdef VALIDATE_PARMS
    if (!seclev_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, UBUS_RESP, RCHK_ENG_SECLEV_EN, reg_rchk_eng_seclev_en);

    *seclev_en = RU_FIELD_GET(0, UBUS_RESP, RCHK_ENG_SECLEV_EN, SECLEV_EN, reg_rchk_eng_seclev_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_rnr_en_set(uint8_t idx, uint8_t rchk_rnr_en)
{
    uint32_t reg_rchk_rnr_en = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8) ||
       (rchk_rnr_en >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rchk_rnr_en = RU_FIELD_SET(0, UBUS_RESP, RCHK_RNR_EN, RCHK_RNR_EN, reg_rchk_rnr_en, rchk_rnr_en);

    RU_REG_RAM_WRITE(0, idx, UBUS_RESP, RCHK_RNR_EN, reg_rchk_rnr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_rnr_en_get(uint8_t idx, uint8_t *rchk_rnr_en)
{
    uint32_t reg_rchk_rnr_en;

#ifdef VALIDATE_PARMS
    if (!rchk_rnr_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, UBUS_RESP, RCHK_RNR_EN, reg_rchk_rnr_en);

    *rchk_rnr_en = RU_FIELD_GET(0, UBUS_RESP, RCHK_RNR_EN, RCHK_RNR_EN, reg_rchk_rnr_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt0_get(uint32_t *rchk_abort_capt0)
{
    uint32_t reg_rchk_abort_capt0;

#ifdef VALIDATE_PARMS
    if (!rchk_abort_capt0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RCHK_ABORT_CAPT0, reg_rchk_abort_capt0);

    *rchk_abort_capt0 = RU_FIELD_GET(0, UBUS_RESP, RCHK_ABORT_CAPT0, RCHK_ABORT_CAPT0, reg_rchk_abort_capt0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt1_get(uint32_t *rchk_abort_capt1)
{
    uint32_t reg_rchk_abort_capt1;

#ifdef VALIDATE_PARMS
    if (!rchk_abort_capt1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RCHK_ABORT_CAPT1, reg_rchk_abort_capt1);

    *rchk_abort_capt1 = RU_FIELD_GET(0, UBUS_RESP, RCHK_ABORT_CAPT1, RCHK_ABORT_CAPT1, reg_rchk_abort_capt1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt2_get(uint32_t *rchk_abort_capt2)
{
    uint32_t reg_rchk_abort_capt2;

#ifdef VALIDATE_PARMS
    if (!rchk_abort_capt2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, RCHK_ABORT_CAPT2, reg_rchk_abort_capt2);

    *rchk_abort_capt2 = RU_FIELD_GET(0, UBUS_RESP, RCHK_ABORT_CAPT2, RCHK_ABORT_CAPT2, reg_rchk_abort_capt2);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_rnr_intr_ctrl_isr,
    bdmf_address_rnr_intr_ctrl_ism,
    bdmf_address_rnr_intr_ctrl_ier,
    bdmf_address_rnr_intr_ctrl_itr,
    bdmf_address_profiling_cfg,
    bdmf_address_profiling_status,
    bdmf_address_profiling_counter,
    bdmf_address_profiling_start_value,
    bdmf_address_profiling_stop_value,
    bdmf_address_profiling_cycle_num,
    bdmf_address_rchk_lock,
    bdmf_address_rchk_eng_ctrl,
    bdmf_address_rchk_eng_start_add,
    bdmf_address_rchk_eng_end_add,
    bdmf_address_rchk_eng_seclev_en,
    bdmf_address_rchk_rnr_en,
    bdmf_address_rchk_abort_capt0,
    bdmf_address_rchk_abort_capt1,
    bdmf_address_rchk_abort_capt2,
}
bdmf_address;

static int ag_drv_ubus_resp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_ubus_resp_rnr_intr_ctrl_isr:
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_isr_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_rnr_intr_ctrl_ier:
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_rnr_intr_ctrl_itr:
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_profiling_cfg:
        ag_err = ag_drv_ubus_resp_profiling_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_ubus_resp_profiling_cycle_num:
        ag_err = ag_drv_ubus_resp_profiling_cycle_num_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_rchk_lock:
        ag_err = ag_drv_ubus_resp_rchk_lock_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_rchk_eng_ctrl:
        ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_ubus_resp_rchk_eng_start_add:
        ag_err = ag_drv_ubus_resp_rchk_eng_start_add_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_resp_rchk_eng_end_add:
        ag_err = ag_drv_ubus_resp_rchk_eng_end_add_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_resp_rchk_eng_seclev_en:
        ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_resp_rchk_rnr_en:
        ag_err = ag_drv_ubus_resp_rchk_rnr_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_ubus_resp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_ubus_resp_rnr_intr_ctrl_isr:
    {
        uint32_t ist;
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_isr_get(&ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
        break;
    }
    case cli_ubus_resp_rnr_intr_ctrl_ism:
    {
        uint32_t ism;
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_ubus_resp_rnr_intr_ctrl_ier:
    {
        uint32_t iem;
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_ubus_resp_rnr_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
        break;
    }
    case cli_ubus_resp_profiling_cfg:
    {
        bdmf_boolean counter_enable;
        bdmf_boolean profiling_start;
        bdmf_boolean manual_stop_mode;
        bdmf_boolean do_manual_stop;
        ag_err = ag_drv_ubus_resp_profiling_cfg_get(&counter_enable, &profiling_start, &manual_stop_mode, &do_manual_stop);
        bdmf_session_print(session, "counter_enable = %u = 0x%x\n", counter_enable, counter_enable);
        bdmf_session_print(session, "profiling_start = %u = 0x%x\n", profiling_start, profiling_start);
        bdmf_session_print(session, "manual_stop_mode = %u = 0x%x\n", manual_stop_mode, manual_stop_mode);
        bdmf_session_print(session, "do_manual_stop = %u = 0x%x\n", do_manual_stop, do_manual_stop);
        break;
    }
    case cli_ubus_resp_profiling_status:
    {
        bdmf_boolean profiling_on;
        uint32_t cycles_counter;
        ag_err = ag_drv_ubus_resp_profiling_status_get(&profiling_on, &cycles_counter);
        bdmf_session_print(session, "profiling_on = %u = 0x%x\n", profiling_on, profiling_on);
        bdmf_session_print(session, "cycles_counter = %u = 0x%x\n", cycles_counter, cycles_counter);
        break;
    }
    case cli_ubus_resp_profiling_counter:
    {
        uint32_t val;
        ag_err = ag_drv_ubus_resp_profiling_counter_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_ubus_resp_profiling_start_value:
    {
        uint32_t val;
        ag_err = ag_drv_ubus_resp_profiling_start_value_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_ubus_resp_profiling_stop_value:
    {
        uint32_t val;
        ag_err = ag_drv_ubus_resp_profiling_stop_value_get(&val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_ubus_resp_profiling_cycle_num:
    {
        uint32_t profiling_cycles_num;
        ag_err = ag_drv_ubus_resp_profiling_cycle_num_get(&profiling_cycles_num);
        bdmf_session_print(session, "profiling_cycles_num = %u = 0x%x\n", profiling_cycles_num, profiling_cycles_num);
        break;
    }
    case cli_ubus_resp_rchk_lock:
    {
        bdmf_boolean rchk_lock;
        ag_err = ag_drv_ubus_resp_rchk_lock_get(&rchk_lock);
        bdmf_session_print(session, "rchk_lock = %u = 0x%x\n", rchk_lock, rchk_lock);
        break;
    }
    case cli_ubus_resp_rchk_eng_ctrl:
    {
        bdmf_boolean wr_abort;
        bdmf_boolean rd_abort;
        uint8_t prot;
        uint8_t prot_msk;
        ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_get(parm[1].value.unumber, &wr_abort, &rd_abort, &prot, &prot_msk);
        bdmf_session_print(session, "wr_abort = %u = 0x%x\n", wr_abort, wr_abort);
        bdmf_session_print(session, "rd_abort = %u = 0x%x\n", rd_abort, rd_abort);
        bdmf_session_print(session, "prot = %u = 0x%x\n", prot, prot);
        bdmf_session_print(session, "prot_msk = %u = 0x%x\n", prot_msk, prot_msk);
        break;
    }
    case cli_ubus_resp_rchk_eng_start_add:
    {
        uint32_t start_add;
        ag_err = ag_drv_ubus_resp_rchk_eng_start_add_get(parm[1].value.unumber, &start_add);
        bdmf_session_print(session, "start_add = %u = 0x%x\n", start_add, start_add);
        break;
    }
    case cli_ubus_resp_rchk_eng_end_add:
    {
        uint32_t end_add;
        ag_err = ag_drv_ubus_resp_rchk_eng_end_add_get(parm[1].value.unumber, &end_add);
        bdmf_session_print(session, "end_add = %u = 0x%x\n", end_add, end_add);
        break;
    }
    case cli_ubus_resp_rchk_eng_seclev_en:
    {
        uint32_t seclev_en;
        ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_get(parm[1].value.unumber, &seclev_en);
        bdmf_session_print(session, "seclev_en = %u = 0x%x\n", seclev_en, seclev_en);
        break;
    }
    case cli_ubus_resp_rchk_rnr_en:
    {
        uint8_t rchk_rnr_en;
        ag_err = ag_drv_ubus_resp_rchk_rnr_en_get(parm[1].value.unumber, &rchk_rnr_en);
        bdmf_session_print(session, "rchk_rnr_en = %u = 0x%x\n", rchk_rnr_en, rchk_rnr_en);
        break;
    }
    case cli_ubus_resp_rchk_abort_cpt0:
    {
        uint32_t rchk_abort_capt0;
        ag_err = ag_drv_ubus_resp_rchk_abort_cpt0_get(&rchk_abort_capt0);
        bdmf_session_print(session, "rchk_abort_capt0 = %u = 0x%x\n", rchk_abort_capt0, rchk_abort_capt0);
        break;
    }
    case cli_ubus_resp_rchk_abort_cpt1:
    {
        uint32_t rchk_abort_capt1;
        ag_err = ag_drv_ubus_resp_rchk_abort_cpt1_get(&rchk_abort_capt1);
        bdmf_session_print(session, "rchk_abort_capt1 = %u = 0x%x\n", rchk_abort_capt1, rchk_abort_capt1);
        break;
    }
    case cli_ubus_resp_rchk_abort_cpt2:
    {
        uint32_t rchk_abort_capt2;
        ag_err = ag_drv_ubus_resp_rchk_abort_cpt2_get(&rchk_abort_capt2);
        bdmf_session_print(session, "rchk_abort_capt2 = %u = 0x%x\n", rchk_abort_capt2, rchk_abort_capt2);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_ubus_resp_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t ist = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_isr_set( %u)\n",
            ist);
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_isr_set(ist);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_isr_get(&ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_isr_get( %u)\n",
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

    {
        uint32_t ism = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ism_get(&ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_ism_get( %u)\n",
                ism);
        }
    }

    {
        uint32_t iem = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_ier_set( %u)\n",
            iem);
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ier_set(iem);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_ier_get(&iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_ier_get( %u)\n",
                iem);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (iem != gtmv(m, 32))
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
        bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_itr_set( %u)\n",
            ist);
        ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_itr_set(ist);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rnr_intr_ctrl_itr_get(&ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rnr_intr_ctrl_itr_get( %u)\n",
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

    {
        bdmf_boolean counter_enable = gtmv(m, 1);
        bdmf_boolean profiling_start = gtmv(m, 1);
        bdmf_boolean manual_stop_mode = gtmv(m, 1);
        bdmf_boolean do_manual_stop = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_ubus_resp_profiling_cfg_set( %u %u %u %u)\n",
            counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        ag_err = ag_drv_ubus_resp_profiling_cfg_set(counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_cfg_get(&counter_enable, &profiling_start, &manual_stop_mode, &do_manual_stop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_cfg_get( %u %u %u %u)\n",
                counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (counter_enable != gtmv(m, 1) || profiling_start != gtmv(m, 1) || manual_stop_mode != gtmv(m, 1) || do_manual_stop != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean profiling_on = gtmv(m, 1);
        uint32_t cycles_counter = gtmv(m, 31);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_status_get(&profiling_on, &cycles_counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_status_get( %u %u)\n",
                profiling_on, cycles_counter);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_counter_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_counter_get( %u)\n",
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_start_value_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_start_value_get( %u)\n",
                val);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_stop_value_get(&val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_stop_value_get( %u)\n",
                val);
        }
    }

    {
        uint32_t profiling_cycles_num = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_profiling_cycle_num_set( %u)\n",
            profiling_cycles_num);
        ag_err = ag_drv_ubus_resp_profiling_cycle_num_set(profiling_cycles_num);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_profiling_cycle_num_get(&profiling_cycles_num);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_profiling_cycle_num_get( %u)\n",
                profiling_cycles_num);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (profiling_cycles_num != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean rchk_lock = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_lock_set( %u)\n",
            rchk_lock);
        ag_err = ag_drv_ubus_resp_rchk_lock_set(rchk_lock);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_lock_get(&rchk_lock);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_lock_get( %u)\n",
                rchk_lock);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rchk_lock != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        bdmf_boolean wr_abort = gtmv(m, 1);
        bdmf_boolean rd_abort = gtmv(m, 1);
        uint8_t prot = gtmv(m, 3);
        uint8_t prot_msk = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_ctrl_set( %u %u %u %u %u)\n", idx,
            wr_abort, rd_abort, prot, prot_msk);
        ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_set(idx, wr_abort, rd_abort, prot, prot_msk);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_get(idx, &wr_abort, &rd_abort, &prot, &prot_msk);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_ctrl_get( %u %u %u %u %u)\n", idx,
                wr_abort, rd_abort, prot, prot_msk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wr_abort != gtmv(m, 1) || rd_abort != gtmv(m, 1) || prot != gtmv(m, 3) || prot_msk != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t start_add = gtmv(m, 17);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_start_add_set( %u %u)\n", idx,
            start_add);
        ag_err = ag_drv_ubus_resp_rchk_eng_start_add_set(idx, start_add);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_eng_start_add_get(idx, &start_add);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_start_add_get( %u %u)\n", idx,
                start_add);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start_add != gtmv(m, 17))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t end_add = gtmv(m, 17);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_end_add_set( %u %u)\n", idx,
            end_add);
        ag_err = ag_drv_ubus_resp_rchk_eng_end_add_set(idx, end_add);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_eng_end_add_get(idx, &end_add);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_end_add_get( %u %u)\n", idx,
                end_add);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end_add != gtmv(m, 17))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t seclev_en = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_seclev_en_set( %u %u)\n", idx,
            seclev_en);
        ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_set(idx, seclev_en);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_get(idx, &seclev_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_seclev_en_get( %u %u)\n", idx,
                seclev_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (seclev_en != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint8_t rchk_rnr_en = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_ubus_resp_rchk_rnr_en_set( %u %u)\n", idx,
            rchk_rnr_en);
        ag_err = ag_drv_ubus_resp_rchk_rnr_en_set(idx, rchk_rnr_en);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_rnr_en_get(idx, &rchk_rnr_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_rnr_en_get( %u %u)\n", idx,
                rchk_rnr_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rchk_rnr_en != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t rchk_abort_capt0 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_abort_cpt0_get(&rchk_abort_capt0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_abort_cpt0_get( %u)\n",
                rchk_abort_capt0);
        }
    }

    {
        uint32_t rchk_abort_capt1 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_abort_cpt1_get(&rchk_abort_capt1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_abort_cpt1_get( %u)\n",
                rchk_abort_capt1);
        }
    }

    {
        uint32_t rchk_abort_capt2 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_rchk_abort_cpt2_get(&rchk_abort_capt2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_abort_cpt2_get( %u)\n",
                rchk_abort_capt2);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_ubus_resp_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case cli_ubus_resp_rchk_eng_ctrl:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        bdmf_boolean wr_abort = gtmv(m, 1);
        bdmf_boolean rd_abort = gtmv(m, 1);
        uint8_t prot = gtmv(m, 3);
        uint8_t prot_msk = gtmv(m, 3);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_ctrl_set( %u %u %u %u %u)\n", idx,
                wr_abort, rd_abort, prot, prot_msk);
            ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_set(idx, wr_abort, rd_abort, prot, prot_msk);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wr_abort = gtmv(m, 1);
        rd_abort = gtmv(m, 1);
        prot = gtmv(m, 3);
        prot_msk = gtmv(m, 3);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_ctrl_set( %u %u %u %u %u)\n", idx,
                wr_abort, rd_abort, prot, prot_msk);
            ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_set(idx, wr_abort, rd_abort, prot, prot_msk);
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

            ag_err = ag_drv_ubus_resp_rchk_eng_ctrl_get(idx, &wr_abort, &rd_abort, &prot, &prot_msk);

            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_ctrl_get( %u %u %u %u %u)\n", idx,
                wr_abort, rd_abort, prot, prot_msk);

            if (wr_abort != gtmv(m, 1) || rd_abort != gtmv(m, 1) || prot != gtmv(m, 3) || prot_msk != gtmv(m, 3) || 0)
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
        bdmf_session_print(session, "Test of rchk_eng_ctrl completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_ubus_resp_rchk_eng_start_add:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint32_t start_add = gtmv(m, 17);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_start_add_set( %u %u)\n", idx,
                start_add);
            ag_err = ag_drv_ubus_resp_rchk_eng_start_add_set(idx, start_add);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        start_add = gtmv(m, 17);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_start_add_set( %u %u)\n", idx,
                start_add);
            ag_err = ag_drv_ubus_resp_rchk_eng_start_add_set(idx, start_add);
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

            ag_err = ag_drv_ubus_resp_rchk_eng_start_add_get(idx, &start_add);

            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_start_add_get( %u %u)\n", idx,
                start_add);

            if (start_add != gtmv(m, 17) || 0)
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
        bdmf_session_print(session, "Test of rchk_eng_start_add completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_ubus_resp_rchk_eng_end_add:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint32_t end_add = gtmv(m, 17);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_end_add_set( %u %u)\n", idx,
                end_add);
            ag_err = ag_drv_ubus_resp_rchk_eng_end_add_set(idx, end_add);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        end_add = gtmv(m, 17);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_end_add_set( %u %u)\n", idx,
                end_add);
            ag_err = ag_drv_ubus_resp_rchk_eng_end_add_set(idx, end_add);
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

            ag_err = ag_drv_ubus_resp_rchk_eng_end_add_get(idx, &end_add);

            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_end_add_get( %u %u)\n", idx,
                end_add);

            if (end_add != gtmv(m, 17) || 0)
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
        bdmf_session_print(session, "Test of rchk_eng_end_add completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_ubus_resp_rchk_eng_seclev_en:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint32_t seclev_en = gtmv(m, 32);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_seclev_en_set( %u %u)\n", idx,
                seclev_en);
            ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_set(idx, seclev_en);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        seclev_en = gtmv(m, 32);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_seclev_en_set( %u %u)\n", idx,
                seclev_en);
            ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_set(idx, seclev_en);
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

            ag_err = ag_drv_ubus_resp_rchk_eng_seclev_en_get(idx, &seclev_en);

            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_eng_seclev_en_get( %u %u)\n", idx,
                seclev_en);

            if (seclev_en != gtmv(m, 32) || 0)
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
        bdmf_session_print(session, "Test of rchk_eng_seclev_en completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_ubus_resp_rchk_rnr_en:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint8_t rchk_rnr_en = gtmv(m, 7);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_rnr_en_set( %u %u)\n", idx,
                rchk_rnr_en);
            ag_err = ag_drv_ubus_resp_rchk_rnr_en_set(idx, rchk_rnr_en);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        rchk_rnr_en = gtmv(m, 7);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_rnr_en_set( %u %u)\n", idx,
                rchk_rnr_en);
            ag_err = ag_drv_ubus_resp_rchk_rnr_en_set(idx, rchk_rnr_en);
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

            ag_err = ag_drv_ubus_resp_rchk_rnr_en_get(idx, &rchk_rnr_en);

            bdmf_session_print(session, "ag_drv_ubus_resp_rchk_rnr_en_get( %u %u)\n", idx,
                rchk_rnr_en);

            if (rchk_rnr_en != gtmv(m, 7) || 0)
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
        bdmf_session_print(session, "Test of rchk_rnr_en completed. Number of tested entries %u.\n", max_idx);
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
static int ag_drv_ubus_resp_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_rnr_intr_ctrl_isr: reg = &RU_REG(UBUS_RESP, RNR_INTR_CTRL_ISR); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rnr_intr_ctrl_ism: reg = &RU_REG(UBUS_RESP, RNR_INTR_CTRL_ISM); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rnr_intr_ctrl_ier: reg = &RU_REG(UBUS_RESP, RNR_INTR_CTRL_IER); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rnr_intr_ctrl_itr: reg = &RU_REG(UBUS_RESP, RNR_INTR_CTRL_ITR); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_cfg: reg = &RU_REG(UBUS_RESP, PROFILING_CFG); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_status: reg = &RU_REG(UBUS_RESP, PROFILING_STATUS); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_counter: reg = &RU_REG(UBUS_RESP, PROFILING_COUNTER); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_start_value: reg = &RU_REG(UBUS_RESP, PROFILING_START_VALUE); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_stop_value: reg = &RU_REG(UBUS_RESP, PROFILING_STOP_VALUE); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_profiling_cycle_num: reg = &RU_REG(UBUS_RESP, PROFILING_CYCLE_NUM); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_lock: reg = &RU_REG(UBUS_RESP, RCHK_LOCK); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_eng_ctrl: reg = &RU_REG(UBUS_RESP, RCHK_ENG_CTRL); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_eng_start_add: reg = &RU_REG(UBUS_RESP, RCHK_ENG_START_ADD); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_eng_end_add: reg = &RU_REG(UBUS_RESP, RCHK_ENG_END_ADD); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_eng_seclev_en: reg = &RU_REG(UBUS_RESP, RCHK_ENG_SECLEV_EN); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_rnr_en: reg = &RU_REG(UBUS_RESP, RCHK_RNR_EN); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_abort_capt0: reg = &RU_REG(UBUS_RESP, RCHK_ABORT_CAPT0); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_abort_capt1: reg = &RU_REG(UBUS_RESP, RCHK_ABORT_CAPT1); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_rchk_abort_capt2: reg = &RU_REG(UBUS_RESP, RCHK_ABORT_CAPT2); blk = &RU_BLK(UBUS_RESP); break;
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

bdmfmon_handle_t ag_drv_ubus_resp_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "ubus_resp", "ubus_resp", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiling_cfg[] = {
            BDMFMON_MAKE_PARM("counter_enable", "counter_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profiling_start", "profiling_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("manual_stop_mode", "manual_stop_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("do_manual_stop", "do_manual_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiling_cycle_num[] = {
            BDMFMON_MAKE_PARM("profiling_cycles_num", "profiling_cycles_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_lock[] = {
            BDMFMON_MAKE_PARM("rchk_lock", "rchk_lock", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_eng_ctrl[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wr_abort", "wr_abort", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rd_abort", "rd_abort", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prot", "prot", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prot_msk", "prot_msk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_eng_start_add[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_add", "start_add", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_eng_end_add[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("end_add", "end_add", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_eng_seclev_en[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("seclev_en", "seclev_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rchk_rnr_en[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rchk_rnr_en", "rchk_rnr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "rnr_intr_ctrl_isr", .val = cli_ubus_resp_rnr_intr_ctrl_isr, .parms = set_rnr_intr_ctrl_isr },
            { .name = "rnr_intr_ctrl_ier", .val = cli_ubus_resp_rnr_intr_ctrl_ier, .parms = set_rnr_intr_ctrl_ier },
            { .name = "rnr_intr_ctrl_itr", .val = cli_ubus_resp_rnr_intr_ctrl_itr, .parms = set_rnr_intr_ctrl_itr },
            { .name = "profiling_cfg", .val = cli_ubus_resp_profiling_cfg, .parms = set_profiling_cfg },
            { .name = "profiling_cycle_num", .val = cli_ubus_resp_profiling_cycle_num, .parms = set_profiling_cycle_num },
            { .name = "rchk_lock", .val = cli_ubus_resp_rchk_lock, .parms = set_rchk_lock },
            { .name = "rchk_eng_ctrl", .val = cli_ubus_resp_rchk_eng_ctrl, .parms = set_rchk_eng_ctrl },
            { .name = "rchk_eng_start_add", .val = cli_ubus_resp_rchk_eng_start_add, .parms = set_rchk_eng_start_add },
            { .name = "rchk_eng_end_add", .val = cli_ubus_resp_rchk_eng_end_add, .parms = set_rchk_eng_end_add },
            { .name = "rchk_eng_seclev_en", .val = cli_ubus_resp_rchk_eng_seclev_en, .parms = set_rchk_eng_seclev_en },
            { .name = "rchk_rnr_en", .val = cli_ubus_resp_rchk_rnr_en, .parms = set_rchk_rnr_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_ubus_resp_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rchk_eng_ctrl[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rchk_eng_start_add[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rchk_eng_end_add[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rchk_eng_seclev_en[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rchk_rnr_en[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "rnr_intr_ctrl_isr", .val = cli_ubus_resp_rnr_intr_ctrl_isr, .parms = get_default },
            { .name = "rnr_intr_ctrl_ism", .val = cli_ubus_resp_rnr_intr_ctrl_ism, .parms = get_default },
            { .name = "rnr_intr_ctrl_ier", .val = cli_ubus_resp_rnr_intr_ctrl_ier, .parms = get_default },
            { .name = "rnr_intr_ctrl_itr", .val = cli_ubus_resp_rnr_intr_ctrl_itr, .parms = get_default },
            { .name = "profiling_cfg", .val = cli_ubus_resp_profiling_cfg, .parms = get_default },
            { .name = "profiling_status", .val = cli_ubus_resp_profiling_status, .parms = get_default },
            { .name = "profiling_counter", .val = cli_ubus_resp_profiling_counter, .parms = get_default },
            { .name = "profiling_start_value", .val = cli_ubus_resp_profiling_start_value, .parms = get_default },
            { .name = "profiling_stop_value", .val = cli_ubus_resp_profiling_stop_value, .parms = get_default },
            { .name = "profiling_cycle_num", .val = cli_ubus_resp_profiling_cycle_num, .parms = get_default },
            { .name = "rchk_lock", .val = cli_ubus_resp_rchk_lock, .parms = get_default },
            { .name = "rchk_eng_ctrl", .val = cli_ubus_resp_rchk_eng_ctrl, .parms = get_rchk_eng_ctrl },
            { .name = "rchk_eng_start_add", .val = cli_ubus_resp_rchk_eng_start_add, .parms = get_rchk_eng_start_add },
            { .name = "rchk_eng_end_add", .val = cli_ubus_resp_rchk_eng_end_add, .parms = get_rchk_eng_end_add },
            { .name = "rchk_eng_seclev_en", .val = cli_ubus_resp_rchk_eng_seclev_en, .parms = get_rchk_eng_seclev_en },
            { .name = "rchk_rnr_en", .val = cli_ubus_resp_rchk_rnr_en, .parms = get_rchk_rnr_en },
            { .name = "rchk_abort_cpt0", .val = cli_ubus_resp_rchk_abort_cpt0, .parms = get_default },
            { .name = "rchk_abort_cpt1", .val = cli_ubus_resp_rchk_abort_cpt1, .parms = get_default },
            { .name = "rchk_abort_cpt2", .val = cli_ubus_resp_rchk_abort_cpt2, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_ubus_resp_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_ubus_resp_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "rchk_eng_ctrl", .val = cli_ubus_resp_rchk_eng_ctrl, .parms = ext_test_default},
            { .name = "rchk_eng_start_add", .val = cli_ubus_resp_rchk_eng_start_add, .parms = ext_test_default},
            { .name = "rchk_eng_end_add", .val = cli_ubus_resp_rchk_eng_end_add, .parms = ext_test_default},
            { .name = "rchk_eng_seclev_en", .val = cli_ubus_resp_rchk_eng_seclev_en, .parms = ext_test_default},
            { .name = "rchk_rnr_en", .val = cli_ubus_resp_rchk_rnr_en, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_ubus_resp_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "RNR_INTR_CTRL_ISR", .val = bdmf_address_rnr_intr_ctrl_isr },
            { .name = "RNR_INTR_CTRL_ISM", .val = bdmf_address_rnr_intr_ctrl_ism },
            { .name = "RNR_INTR_CTRL_IER", .val = bdmf_address_rnr_intr_ctrl_ier },
            { .name = "RNR_INTR_CTRL_ITR", .val = bdmf_address_rnr_intr_ctrl_itr },
            { .name = "PROFILING_CFG", .val = bdmf_address_profiling_cfg },
            { .name = "PROFILING_STATUS", .val = bdmf_address_profiling_status },
            { .name = "PROFILING_COUNTER", .val = bdmf_address_profiling_counter },
            { .name = "PROFILING_START_VALUE", .val = bdmf_address_profiling_start_value },
            { .name = "PROFILING_STOP_VALUE", .val = bdmf_address_profiling_stop_value },
            { .name = "PROFILING_CYCLE_NUM", .val = bdmf_address_profiling_cycle_num },
            { .name = "RCHK_LOCK", .val = bdmf_address_rchk_lock },
            { .name = "RCHK_ENG_CTRL", .val = bdmf_address_rchk_eng_ctrl },
            { .name = "RCHK_ENG_START_ADD", .val = bdmf_address_rchk_eng_start_add },
            { .name = "RCHK_ENG_END_ADD", .val = bdmf_address_rchk_eng_end_add },
            { .name = "RCHK_ENG_SECLEV_EN", .val = bdmf_address_rchk_eng_seclev_en },
            { .name = "RCHK_RNR_EN", .val = bdmf_address_rchk_rnr_en },
            { .name = "RCHK_ABORT_CAPT0", .val = bdmf_address_rchk_abort_capt0 },
            { .name = "RCHK_ABORT_CAPT1", .val = bdmf_address_rchk_abort_capt1 },
            { .name = "RCHK_ABORT_CAPT2", .val = bdmf_address_rchk_abort_capt2 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_ubus_resp_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
