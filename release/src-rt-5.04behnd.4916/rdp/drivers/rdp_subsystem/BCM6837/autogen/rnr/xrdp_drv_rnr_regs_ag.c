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
#include "xrdp_drv_rnr_regs_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_rnr_regs_rnr_enable_set(uint8_t rnr_id, bdmf_boolean en)
{
    uint32_t reg_cfg_global_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    reg_cfg_global_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, EN, reg_cfg_global_ctrl, en);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_rnr_enable_get(uint8_t rnr_id, bdmf_boolean *en)
{
    uint32_t reg_cfg_global_ctrl;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, EN, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_dma_illegal_get(uint8_t rnr_id, bdmf_boolean *dma_illegal_status)
{
    uint32_t reg_cfg_global_ctrl;

#ifdef VALIDATE_PARMS
    if (!dma_illegal_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *dma_illegal_status = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, DMA_ILLEGAL_STATUS, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_prediction_overrun_get(uint8_t rnr_id, bdmf_boolean *prediction_overrun_status)
{
    uint32_t reg_cfg_global_ctrl;

#ifdef VALIDATE_PARMS
    if (!prediction_overrun_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *prediction_overrun_status = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, PREDICTION_OVERRUN_STATUS, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_rnr_freq_set(uint8_t rnr_id, uint16_t micro_sec_val)
{
    uint32_t reg_cfg_global_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    reg_cfg_global_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, MICRO_SEC_VAL, reg_cfg_global_ctrl, micro_sec_val);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_rnr_freq_get(uint8_t rnr_id, uint16_t *micro_sec_val)
{
    uint32_t reg_cfg_global_ctrl;

#ifdef VALIDATE_PARMS
    if (!micro_sec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *micro_sec_val = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, MICRO_SEC_VAL, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cam_stop_val_set(uint8_t rnr_id, uint16_t stop_value)
{
    uint32_t reg_cfg_cam_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_cam_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_CAM_CFG, STOP_VALUE, reg_cfg_cam_cfg, stop_value);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_CAM_CFG, reg_cfg_cam_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cam_stop_val_get(uint8_t rnr_id, uint16_t *stop_value)
{
    uint32_t reg_cfg_cam_cfg;

#ifdef VALIDATE_PARMS
    if (!stop_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_CAM_CFG, reg_cfg_cam_cfg);

    *stop_value = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_CAM_CFG, STOP_VALUE, reg_cfg_cam_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_profiling_sts_get(uint8_t rnr_id, rnr_regs_profiling_sts *profiling_sts)
{
    uint32_t reg_cfg_profiling_sts;

#ifdef VALIDATE_PARMS
    if (!profiling_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_STS, reg_cfg_profiling_sts);

    profiling_sts->trace_write_pnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, TRACE_WRITE_PNT, reg_cfg_profiling_sts);
    profiling_sts->idle_no_active_task = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, IDLE_NO_ACTIVE_TASK, reg_cfg_profiling_sts);
    profiling_sts->curr_thread_num = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, CURR_THREAD_NUM, reg_cfg_profiling_sts);
    profiling_sts->profiling_active = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, PROFILING_ACTIVE, reg_cfg_profiling_sts);
    profiling_sts->trace_fifo_overrun = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, TRACE_FIFO_OVERRUN, reg_cfg_profiling_sts);
    profiling_sts->single_mode_profiling_status = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, SINGLE_MODE_PROFILING_STATUS, reg_cfg_profiling_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_is_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_fifo_overrun)
{
    uint32_t reg_cfg_profiling_sts;

#ifdef VALIDATE_PARMS
    if (!trace_fifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_STS, reg_cfg_profiling_sts);

    *trace_fifo_overrun = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_STS, TRACE_FIFO_OVERRUN, reg_cfg_profiling_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_trace_config_set(uint8_t rnr_id, const rnr_regs_trace_config *trace_config)
{
    uint32_t reg_cfg_profiling_cfg_1 = 0;
    uint32_t reg_cfg_profiling_cfg_2 = 0;

#ifdef VALIDATE_PARMS
    if(!trace_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (trace_config->trace_wraparound >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_mode >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_disable_idle_in >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_disable_wakeup_log >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_task >= _4BITS_MAX_VAL_) ||
       (trace_config->idle_counter_source_sel >= _1BITS_MAX_VAL_) ||
       (trace_config->counters_selected_task_mode >= _1BITS_MAX_VAL_) ||
       (trace_config->counters_task >= _4BITS_MAX_VAL_) ||
       (trace_config->profiling_window_mode >= _1BITS_MAX_VAL_) ||
       (trace_config->single_mode_start_option >= _3BITS_MAX_VAL_) ||
       (trace_config->single_mode_stop_option >= _3BITS_MAX_VAL_) ||
       (trace_config->window_manual_start >= _1BITS_MAX_VAL_) ||
       (trace_config->window_manual_stop >= _1BITS_MAX_VAL_) ||
       (trace_config->tracer_enable >= _1BITS_MAX_VAL_) ||
       (trace_config->profiling_window_reset >= _1BITS_MAX_VAL_) ||
       (trace_config->profiling_window_enable >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_reset_event_fifo >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_clear_fifo_overrun >= _1BITS_MAX_VAL_) ||
       (trace_config->trigger_on_second >= _1BITS_MAX_VAL_) ||
       (trace_config->pc_start >= _13BITS_MAX_VAL_) ||
       (trace_config->pc_stop_or_cycle_count >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_WRAPAROUND, reg_cfg_profiling_cfg_1, trace_config->trace_wraparound);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_MODE, reg_cfg_profiling_cfg_1, trace_config->trace_mode);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_IDLE_IN, reg_cfg_profiling_cfg_1, trace_config->trace_disable_idle_in);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_WAKEUP_LOG, reg_cfg_profiling_cfg_1, trace_config->trace_disable_wakeup_log);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_TASK, reg_cfg_profiling_cfg_1, trace_config->trace_task);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, IDLE_COUNTER_SOURCE_SEL, reg_cfg_profiling_cfg_1, trace_config->idle_counter_source_sel);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, COUNTERS_SELECTED_TASK_MODE, reg_cfg_profiling_cfg_1, trace_config->counters_selected_task_mode);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, COUNTERS_TASK, reg_cfg_profiling_cfg_1, trace_config->counters_task);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_MODE, reg_cfg_profiling_cfg_1, trace_config->profiling_window_mode);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, SINGLE_MODE_START_OPTION, reg_cfg_profiling_cfg_1, trace_config->single_mode_start_option);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, SINGLE_MODE_STOP_OPTION, reg_cfg_profiling_cfg_1, trace_config->single_mode_stop_option);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, WINDOW_MANUAL_START, reg_cfg_profiling_cfg_1, trace_config->window_manual_start);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, WINDOW_MANUAL_STOP, reg_cfg_profiling_cfg_1, trace_config->window_manual_stop);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACER_ENABLE, reg_cfg_profiling_cfg_1, trace_config->tracer_enable);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_RESET, reg_cfg_profiling_cfg_1, trace_config->profiling_window_reset);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_ENABLE, reg_cfg_profiling_cfg_1, trace_config->profiling_window_enable);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1, trace_config->trace_reset_event_fifo);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1, trace_config->trace_clear_fifo_overrun);
    reg_cfg_profiling_cfg_2 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, TRIGGER_ON_SECOND, reg_cfg_profiling_cfg_2, trace_config->trigger_on_second);
    reg_cfg_profiling_cfg_2 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, PC_START, reg_cfg_profiling_cfg_2, trace_config->pc_start);
    reg_cfg_profiling_cfg_2 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, PC_STOP_OR_CYCLE_COUNT, reg_cfg_profiling_cfg_2, trace_config->pc_stop_or_cycle_count);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);
    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, reg_cfg_profiling_cfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_trace_config_get(uint8_t rnr_id, rnr_regs_trace_config *trace_config)
{
    uint32_t reg_cfg_profiling_cfg_1;
    uint32_t reg_cfg_profiling_cfg_2;

#ifdef VALIDATE_PARMS
    if (!trace_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, reg_cfg_profiling_cfg_2);

    trace_config->trace_wraparound = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_WRAPAROUND, reg_cfg_profiling_cfg_1);
    trace_config->trace_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_MODE, reg_cfg_profiling_cfg_1);
    trace_config->trace_disable_idle_in = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_IDLE_IN, reg_cfg_profiling_cfg_1);
    trace_config->trace_disable_wakeup_log = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_WAKEUP_LOG, reg_cfg_profiling_cfg_1);
    trace_config->trace_task = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_TASK, reg_cfg_profiling_cfg_1);
    trace_config->idle_counter_source_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, IDLE_COUNTER_SOURCE_SEL, reg_cfg_profiling_cfg_1);
    trace_config->counters_selected_task_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, COUNTERS_SELECTED_TASK_MODE, reg_cfg_profiling_cfg_1);
    trace_config->counters_task = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, COUNTERS_TASK, reg_cfg_profiling_cfg_1);
    trace_config->profiling_window_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_MODE, reg_cfg_profiling_cfg_1);
    trace_config->single_mode_start_option = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, SINGLE_MODE_START_OPTION, reg_cfg_profiling_cfg_1);
    trace_config->single_mode_stop_option = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, SINGLE_MODE_STOP_OPTION, reg_cfg_profiling_cfg_1);
    trace_config->window_manual_start = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, WINDOW_MANUAL_START, reg_cfg_profiling_cfg_1);
    trace_config->window_manual_stop = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, WINDOW_MANUAL_STOP, reg_cfg_profiling_cfg_1);
    trace_config->tracer_enable = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACER_ENABLE, reg_cfg_profiling_cfg_1);
    trace_config->profiling_window_reset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_RESET, reg_cfg_profiling_cfg_1);
    trace_config->profiling_window_enable = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, PROFILING_WINDOW_ENABLE, reg_cfg_profiling_cfg_1);
    trace_config->trace_reset_event_fifo = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1);
    trace_config->trace_clear_fifo_overrun = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1);
    trace_config->trigger_on_second = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, TRIGGER_ON_SECOND, reg_cfg_profiling_cfg_2);
    trace_config->pc_start = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, PC_START, reg_cfg_profiling_cfg_2);
    trace_config->pc_stop_or_cycle_count = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_2, PC_STOP_OR_CYCLE_COUNT, reg_cfg_profiling_cfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_set(uint8_t rnr_id, bdmf_boolean trace_reset_event_fifo)
{
    uint32_t reg_cfg_profiling_cfg_1 = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (trace_reset_event_fifo >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1, trace_reset_event_fifo);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_get(uint8_t rnr_id, bdmf_boolean *trace_reset_event_fifo)
{
    uint32_t reg_cfg_profiling_cfg_1;

#ifdef VALIDATE_PARMS
    if (!trace_reset_event_fifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    *trace_reset_event_fifo = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_set(uint8_t rnr_id, bdmf_boolean trace_clear_fifo_overrun)
{
    uint32_t reg_cfg_profiling_cfg_1 = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (trace_clear_fifo_overrun >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1, trace_clear_fifo_overrun);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_clear_fifo_overrun)
{
    uint32_t reg_cfg_profiling_cfg_1;

#ifdef VALIDATE_PARMS
    if (!trace_clear_fifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    *trace_clear_fifo_overrun = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_rnr_core_cntrs_get(uint8_t rnr_id, rnr_regs_rnr_core_cntrs *rnr_core_cntrs)
{
    uint32_t reg_cfg_stall_cnt1;
    uint32_t reg_cfg_stall_cnt2;
    uint32_t reg_cfg_stall_cnt3;
    uint32_t reg_cfg_stall_cnt4;
    uint32_t reg_cfg_stall_cnt5;
    uint32_t reg_cfg_stall_cnt6;
    uint32_t reg_cfg_stall_cnt7;
    uint32_t reg_cfg_exec_cmds_cnt;
    uint32_t reg_cfg_idle_cnt1;
    uint32_t reg_cfg_jmp_cnt;

#ifdef VALIDATE_PARMS
    if (!rnr_core_cntrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT1, reg_cfg_stall_cnt1);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT2, reg_cfg_stall_cnt2);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT3, reg_cfg_stall_cnt3);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT4, reg_cfg_stall_cnt4);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT5, reg_cfg_stall_cnt5);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT6, reg_cfg_stall_cnt6);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT7, reg_cfg_stall_cnt7);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_EXEC_CMDS_CNT, reg_cfg_exec_cmds_cnt);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_IDLE_CNT1, reg_cfg_idle_cnt1);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_JMP_CNT, reg_cfg_jmp_cnt);

    rnr_core_cntrs->total_stall_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT1, TOTAL_STALL_CNT, reg_cfg_stall_cnt1);
    rnr_core_cntrs->stall_on_alu_b_full_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT2, STALL_ON_ALU_B_FULL_CNT, reg_cfg_stall_cnt2);
    rnr_core_cntrs->stall_on_alu_a_full_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT2, STALL_ON_ALU_A_FULL_CNT, reg_cfg_stall_cnt2);
    rnr_core_cntrs->stall_on_jmpreg = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT3, STALL_ON_JMPREG, reg_cfg_stall_cnt3);
    rnr_core_cntrs->stall_on_memio_full_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT3, STALL_ON_MEMIO_FULL_CNT, reg_cfg_stall_cnt3);
    rnr_core_cntrs->stall_on_waw_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT4, STALL_ON_WAW_CNT, reg_cfg_stall_cnt4);
    rnr_core_cntrs->stall_on_super_cmd = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT4, STALL_ON_SUPER_CMD, reg_cfg_stall_cnt4);
    rnr_core_cntrs->stall_on_super_cmd_when_full = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT5, STALL_ON_SUPER_CMD_WHEN_FULL, reg_cfg_stall_cnt5);
    rnr_core_cntrs->stall_on_cs_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT5, STALL_ON_CS_CNT, reg_cfg_stall_cnt5);
    rnr_core_cntrs->active_cycles_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT6, ACTIVE_CYCLES_CNT, reg_cfg_stall_cnt6);
    rnr_core_cntrs->stall_on_jmp_full_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT7, STALL_ON_JMP_FULL_CNT, reg_cfg_stall_cnt7);
    rnr_core_cntrs->stall_on_skip_jmp_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT7, STALL_ON_SKIP_JMP_CNT, reg_cfg_stall_cnt7);
    rnr_core_cntrs->exec_counter = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_EXEC_CMDS_CNT, EXEC_COUNTER, reg_cfg_exec_cmds_cnt);
    rnr_core_cntrs->idle_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_IDLE_CNT1, IDLE_CNT, reg_cfg_idle_cnt1);
    rnr_core_cntrs->jmp_taken_predicted_untaken_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_JMP_CNT, UNTAKEN_JMP_CNT, reg_cfg_jmp_cnt);
    rnr_core_cntrs->jmp_untaken_predicted_taken_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_JMP_CNT, TAKEN_JMP_CNT, reg_cfg_jmp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num)
{
    uint32_t reg_cfg_cpu_wakeup = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (thread_num >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_cpu_wakeup = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, THREAD_NUM, reg_cfg_cpu_wakeup, thread_num);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, reg_cfg_cpu_wakeup);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_get(uint8_t rnr_id, uint8_t *thread_num)
{
    uint32_t reg_cfg_cpu_wakeup;

#ifdef VALIDATE_PARMS
    if (!thread_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, reg_cfg_cpu_wakeup);

    *thread_num = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, THREAD_NUM, reg_cfg_cpu_wakeup);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_set(uint8_t rnr_id, const rnr_regs_cfg_int_ctrl *cfg_int_ctrl)
{
    uint32_t reg_cfg_int_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_int_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (cfg_int_ctrl->int2_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int3_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int4_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int5_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int6_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int7_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int8_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->int9_sts >= _1BITS_MAX_VAL_) ||
       (cfg_int_ctrl->fit_fail_sts >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT0_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int0_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT1_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int1_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT2_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int2_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT3_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int3_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT4_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int4_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT5_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int5_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT6_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int6_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT7_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int7_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT8_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int8_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT9_STS, reg_cfg_int_ctrl, cfg_int_ctrl->int9_sts);
    reg_cfg_int_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_CTRL, FIT_FAIL_STS, reg_cfg_int_ctrl, cfg_int_ctrl->fit_fail_sts);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_INT_CTRL, reg_cfg_int_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_get(uint8_t rnr_id, rnr_regs_cfg_int_ctrl *cfg_int_ctrl)
{
    uint32_t reg_cfg_int_ctrl;

#ifdef VALIDATE_PARMS
    if (!cfg_int_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_INT_CTRL, reg_cfg_int_ctrl);

    cfg_int_ctrl->int0_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT0_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int1_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT1_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int2_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT2_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int3_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT3_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int4_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT4_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int5_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT5_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int6_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT6_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int7_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT7_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int8_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT8_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->int9_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, INT9_STS, reg_cfg_int_ctrl);
    cfg_int_ctrl->fit_fail_sts = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_CTRL, FIT_FAIL_STS, reg_cfg_int_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_set(uint8_t rnr_id, const rnr_regs_cfg_int_mask *cfg_int_mask)
{
    uint32_t reg_cfg_int_mask = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (cfg_int_mask->int2_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int3_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int4_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int5_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int6_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int7_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int8_mask >= _1BITS_MAX_VAL_) ||
       (cfg_int_mask->int9_mask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT0_MASK, reg_cfg_int_mask, cfg_int_mask->int0_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT1_MASK, reg_cfg_int_mask, cfg_int_mask->int1_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT2_MASK, reg_cfg_int_mask, cfg_int_mask->int2_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT3_MASK, reg_cfg_int_mask, cfg_int_mask->int3_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT4_MASK, reg_cfg_int_mask, cfg_int_mask->int4_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT5_MASK, reg_cfg_int_mask, cfg_int_mask->int5_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT6_MASK, reg_cfg_int_mask, cfg_int_mask->int6_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT7_MASK, reg_cfg_int_mask, cfg_int_mask->int7_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT8_MASK, reg_cfg_int_mask, cfg_int_mask->int8_mask);
    reg_cfg_int_mask = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_INT_MASK, INT9_MASK, reg_cfg_int_mask, cfg_int_mask->int9_mask);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_INT_MASK, reg_cfg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_get(uint8_t rnr_id, rnr_regs_cfg_int_mask *cfg_int_mask)
{
    uint32_t reg_cfg_int_mask;

#ifdef VALIDATE_PARMS
    if (!cfg_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_INT_MASK, reg_cfg_int_mask);

    cfg_int_mask->int0_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT0_MASK, reg_cfg_int_mask);
    cfg_int_mask->int1_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT1_MASK, reg_cfg_int_mask);
    cfg_int_mask->int2_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT2_MASK, reg_cfg_int_mask);
    cfg_int_mask->int3_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT3_MASK, reg_cfg_int_mask);
    cfg_int_mask->int4_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT4_MASK, reg_cfg_int_mask);
    cfg_int_mask->int5_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT5_MASK, reg_cfg_int_mask);
    cfg_int_mask->int6_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT6_MASK, reg_cfg_int_mask);
    cfg_int_mask->int7_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT7_MASK, reg_cfg_int_mask);
    cfg_int_mask->int8_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT8_MASK, reg_cfg_int_mask);
    cfg_int_mask->int9_mask = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_INT_MASK, INT9_MASK, reg_cfg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_gen_cfg *cfg_gen_cfg)
{
    uint32_t reg_cfg_gen_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_gen_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (cfg_gen_cfg->disable_dma_old_flow_control >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->test_fit_fail >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->zero_data_mem >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->zero_context_mem >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->zero_data_mem_done >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->zero_context_mem_done >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->chicken_disable_skip_jmp >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->chicken_disable_alu_load_balancing >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->bbtx_tcam_dest_sel >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->bbtx_hash_dest_sel >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->bbtx_natc_dest_sel >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->bbtx_cnpl_dest_sel >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->gdma_gdesc_buffer_size >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->chicken_enable_old_unique_id_mode >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->chicken_enable_dma_old_mode >= _1BITS_MAX_VAL_) ||
       (cfg_gen_cfg->prevent_cs_till_stores_done >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, DISABLE_DMA_OLD_FLOW_CONTROL, reg_cfg_gen_cfg, cfg_gen_cfg->disable_dma_old_flow_control);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, TEST_FIT_FAIL, reg_cfg_gen_cfg, cfg_gen_cfg->test_fit_fail);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_DATA_MEM, reg_cfg_gen_cfg, cfg_gen_cfg->zero_data_mem);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_CONTEXT_MEM, reg_cfg_gen_cfg, cfg_gen_cfg->zero_context_mem);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_DATA_MEM_DONE, reg_cfg_gen_cfg, cfg_gen_cfg->zero_data_mem_done);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_CONTEXT_MEM_DONE, reg_cfg_gen_cfg, cfg_gen_cfg->zero_context_mem_done);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_DISABLE_SKIP_JMP, reg_cfg_gen_cfg, cfg_gen_cfg->chicken_disable_skip_jmp);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_DISABLE_ALU_LOAD_BALANCING, reg_cfg_gen_cfg, cfg_gen_cfg->chicken_disable_alu_load_balancing);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, GDMA_DESC_OFFSET, reg_cfg_gen_cfg, cfg_gen_cfg->gdma_desc_offset);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_TCAM_DEST_SEL, reg_cfg_gen_cfg, cfg_gen_cfg->bbtx_tcam_dest_sel);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_HASH_DEST_SEL, reg_cfg_gen_cfg, cfg_gen_cfg->bbtx_hash_dest_sel);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_NATC_DEST_SEL, reg_cfg_gen_cfg, cfg_gen_cfg->bbtx_natc_dest_sel);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_CNPL_DEST_SEL, reg_cfg_gen_cfg, cfg_gen_cfg->bbtx_cnpl_dest_sel);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, GDMA_GDESC_BUFFER_SIZE, reg_cfg_gen_cfg, cfg_gen_cfg->gdma_gdesc_buffer_size);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE, reg_cfg_gen_cfg, cfg_gen_cfg->chicken_enable_old_unique_id_mode);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_ENABLE_DMA_OLD_MODE, reg_cfg_gen_cfg, cfg_gen_cfg->chicken_enable_dma_old_mode);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, PREVENT_CS_TILL_STORES_DONE, reg_cfg_gen_cfg, cfg_gen_cfg->prevent_cs_till_stores_done);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_GEN_CFG, reg_cfg_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_get(uint8_t rnr_id, rnr_regs_cfg_gen_cfg *cfg_gen_cfg)
{
    uint32_t reg_cfg_gen_cfg;

#ifdef VALIDATE_PARMS
    if (!cfg_gen_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GEN_CFG, reg_cfg_gen_cfg);

    cfg_gen_cfg->disable_dma_old_flow_control = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, DISABLE_DMA_OLD_FLOW_CONTROL, reg_cfg_gen_cfg);
    cfg_gen_cfg->test_fit_fail = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, TEST_FIT_FAIL, reg_cfg_gen_cfg);
    cfg_gen_cfg->zero_data_mem = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_DATA_MEM, reg_cfg_gen_cfg);
    cfg_gen_cfg->zero_context_mem = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_CONTEXT_MEM, reg_cfg_gen_cfg);
    cfg_gen_cfg->zero_data_mem_done = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_DATA_MEM_DONE, reg_cfg_gen_cfg);
    cfg_gen_cfg->zero_context_mem_done = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, ZERO_CONTEXT_MEM_DONE, reg_cfg_gen_cfg);
    cfg_gen_cfg->chicken_disable_skip_jmp = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_DISABLE_SKIP_JMP, reg_cfg_gen_cfg);
    cfg_gen_cfg->chicken_disable_alu_load_balancing = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_DISABLE_ALU_LOAD_BALANCING, reg_cfg_gen_cfg);
    cfg_gen_cfg->gdma_desc_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, GDMA_DESC_OFFSET, reg_cfg_gen_cfg);
    cfg_gen_cfg->bbtx_tcam_dest_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_TCAM_DEST_SEL, reg_cfg_gen_cfg);
    cfg_gen_cfg->bbtx_hash_dest_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_HASH_DEST_SEL, reg_cfg_gen_cfg);
    cfg_gen_cfg->bbtx_natc_dest_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_NATC_DEST_SEL, reg_cfg_gen_cfg);
    cfg_gen_cfg->bbtx_cnpl_dest_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, BBTX_CNPL_DEST_SEL, reg_cfg_gen_cfg);
    cfg_gen_cfg->gdma_gdesc_buffer_size = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, GDMA_GDESC_BUFFER_SIZE, reg_cfg_gen_cfg);
    cfg_gen_cfg->chicken_enable_old_unique_id_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_ENABLE_OLD_UNIQUE_ID_MODE, reg_cfg_gen_cfg);
    cfg_gen_cfg->chicken_enable_dma_old_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, CHICKEN_ENABLE_DMA_OLD_MODE, reg_cfg_gen_cfg);
    cfg_gen_cfg->prevent_cs_till_stores_done = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, PREVENT_CS_TILL_STORES_DONE, reg_cfg_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_fpm_mini_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_static_offset)
{
    uint32_t reg_cfg_fpm_mini_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_base >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_fpm_mini_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, DMA_BASE, reg_cfg_fpm_mini_cfg, dma_base);
    reg_cfg_fpm_mini_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, DMA_STATIC_OFFSET, reg_cfg_fpm_mini_cfg, dma_static_offset);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, reg_cfg_fpm_mini_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_fpm_mini_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_static_offset)
{
    uint32_t reg_cfg_fpm_mini_cfg;

#ifdef VALIDATE_PARMS
    if (!dma_base || !dma_static_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, reg_cfg_fpm_mini_cfg);

    *dma_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, DMA_BASE, reg_cfg_fpm_mini_cfg);
    *dma_static_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_FPM_MINI_CFG, DMA_STATIC_OFFSET, reg_cfg_fpm_mini_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, bdmf_boolean dma_buf_size_mode, uint8_t dma_static_offset)
{
    uint32_t reg_cfg_ddr_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_base >= _20BITS_MAX_VAL_) ||
       (dma_buf_size >= _3BITS_MAX_VAL_) ||
       (dma_buf_size_mode >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BASE, reg_cfg_ddr_cfg, dma_base);
    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE, reg_cfg_ddr_cfg, dma_buf_size);
    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE_MODE, reg_cfg_ddr_cfg, dma_buf_size_mode);
    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_STATIC_OFFSET, reg_cfg_ddr_cfg, dma_static_offset);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_DDR_CFG, reg_cfg_ddr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, bdmf_boolean *dma_buf_size_mode, uint8_t *dma_static_offset)
{
    uint32_t reg_cfg_ddr_cfg;

#ifdef VALIDATE_PARMS
    if (!dma_base || !dma_buf_size || !dma_buf_size_mode || !dma_static_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_DDR_CFG, reg_cfg_ddr_cfg);

    *dma_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BASE, reg_cfg_ddr_cfg);
    *dma_buf_size = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE, reg_cfg_ddr_cfg);
    *dma_buf_size_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE_MODE, reg_cfg_ddr_cfg);
    *dma_static_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_STATIC_OFFSET, reg_cfg_ddr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, bdmf_boolean dma_buf_size_mode, uint8_t dma_static_offset)
{
    uint32_t reg_cfg_psram_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_base >= _20BITS_MAX_VAL_) ||
       (dma_buf_size >= _3BITS_MAX_VAL_) ||
       (dma_buf_size_mode >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BASE, reg_cfg_psram_cfg, dma_base);
    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE, reg_cfg_psram_cfg, dma_buf_size);
    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE_MODE, reg_cfg_psram_cfg, dma_buf_size_mode);
    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_STATIC_OFFSET, reg_cfg_psram_cfg, dma_static_offset);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PSRAM_CFG, reg_cfg_psram_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, bdmf_boolean *dma_buf_size_mode, uint8_t *dma_static_offset)
{
    uint32_t reg_cfg_psram_cfg;

#ifdef VALIDATE_PARMS
    if (!dma_base || !dma_buf_size || !dma_buf_size_mode || !dma_static_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PSRAM_CFG, reg_cfg_psram_cfg);

    *dma_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BASE, reg_cfg_psram_cfg);
    *dma_buf_size = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE, reg_cfg_psram_cfg);
    *dma_buf_size_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE_MODE, reg_cfg_psram_cfg);
    *dma_static_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_STATIC_OFFSET, reg_cfg_psram_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(uint8_t rnr_id, uint16_t mask0, uint16_t mask1)
{
    uint32_t reg_cfg_ramrd_range_mask_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ramrd_range_mask_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, MASK0, reg_cfg_ramrd_range_mask_cfg, mask0);
    reg_cfg_ramrd_range_mask_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, MASK1, reg_cfg_ramrd_range_mask_cfg, mask1);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, reg_cfg_ramrd_range_mask_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(uint8_t rnr_id, uint16_t *mask0, uint16_t *mask1)
{
    uint32_t reg_cfg_ramrd_range_mask_cfg;

#ifdef VALIDATE_PARMS
    if (!mask0 || !mask1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, reg_cfg_ramrd_range_mask_cfg);

    *mask0 = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, MASK0, reg_cfg_ramrd_range_mask_cfg);
    *mask1 = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG, MASK1, reg_cfg_ramrd_range_mask_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_set(uint8_t rnr_id, uint8_t scheduler_mode)
{
    uint32_t reg_cfg_sch_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (scheduler_mode >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_sch_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_SCH_CFG, SCHEDULER_MODE, reg_cfg_sch_cfg, scheduler_mode);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_SCH_CFG, reg_cfg_sch_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_get(uint8_t rnr_id, uint8_t *scheduler_mode)
{
    uint32_t reg_cfg_sch_cfg;

#ifdef VALIDATE_PARMS
    if (!scheduler_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_SCH_CFG, reg_cfg_sch_cfg);

    *scheduler_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_SCH_CFG, SCHEDULER_MODE, reg_cfg_sch_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg)
{
    uint32_t reg_cfg_bkpt_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (cfg_bkpt_cfg->bkpt_0_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_0_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_1_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_1_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_2_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_2_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_3_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_3_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_4_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_4_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_5_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_5_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_6_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_6_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_7_en >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->bkpt_7_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->step_mode >= _1BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->new_flags_val >= _4BITS_MAX_VAL_) ||
       (cfg_bkpt_cfg->enable_breakpoint_on_fit_fail >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_0_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_0_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_0_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_0_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_1_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_1_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_1_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_1_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_2_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_2_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_2_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_2_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_3_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_3_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_3_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_3_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_4_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_4_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_4_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_4_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_5_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_5_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_5_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_5_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_6_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_6_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_6_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_6_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_7_EN, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_7_en);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_7_USE_THREAD, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->bkpt_7_use_thread);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, STEP_MODE, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->step_mode);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, NEW_FLAGS_VAL, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->new_flags_val);
    reg_cfg_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_CFG, ENABLE_BREAKPOINT_ON_FIT_FAIL, reg_cfg_bkpt_cfg, cfg_bkpt_cfg->enable_breakpoint_on_fit_fail);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_BKPT_CFG, reg_cfg_bkpt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg)
{
    uint32_t reg_cfg_bkpt_cfg;

#ifdef VALIDATE_PARMS
    if (!cfg_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_BKPT_CFG, reg_cfg_bkpt_cfg);

    cfg_bkpt_cfg->bkpt_0_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_0_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_0_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_0_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_1_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_1_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_1_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_1_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_2_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_2_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_2_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_2_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_3_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_3_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_3_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_3_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_4_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_4_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_4_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_4_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_5_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_5_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_5_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_5_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_6_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_6_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_6_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_6_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_7_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_7_EN, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->bkpt_7_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, BKPT_7_USE_THREAD, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->step_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, STEP_MODE, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->new_flags_val = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, NEW_FLAGS_VAL, reg_cfg_bkpt_cfg);
    cfg_bkpt_cfg->enable_breakpoint_on_fit_fail = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_CFG, ENABLE_BREAKPOINT_ON_FIT_FAIL, reg_cfg_bkpt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_set(uint8_t rnr_id, bdmf_boolean enable)
{
    uint32_t reg_cfg_bkpt_imm = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_bkpt_imm = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_BKPT_IMM, ENABLE, reg_cfg_bkpt_imm, enable);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_BKPT_IMM, reg_cfg_bkpt_imm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_get(uint8_t rnr_id, bdmf_boolean *enable)
{
    uint32_t reg_cfg_bkpt_imm;

#ifdef VALIDATE_PARMS
    if (!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_BKPT_IMM, reg_cfg_bkpt_imm);

    *enable = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_IMM, ENABLE, reg_cfg_bkpt_imm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_sts_get(uint8_t rnr_id, uint16_t *bkpt_addr, bdmf_boolean *active, uint16_t *data_bkpt_addr, uint8_t *bkpt_reason)
{
    uint32_t reg_cfg_bkpt_sts;

#ifdef VALIDATE_PARMS
    if (!bkpt_addr || !active || !data_bkpt_addr || !bkpt_reason)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_BKPT_STS, reg_cfg_bkpt_sts);

    *bkpt_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, BKPT_ADDR, reg_cfg_bkpt_sts);
    *active = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, ACTIVE, reg_cfg_bkpt_sts);
    *data_bkpt_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, DATA_BKPT_ADDR, reg_cfg_bkpt_sts);
    *bkpt_reason = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, BKPT_REASON, reg_cfg_bkpt_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_pc_sts_get(uint8_t rnr_id, uint16_t *current_pc_addr, uint16_t *pc_ret)
{
    uint32_t reg_cfg_pc_sts;

#ifdef VALIDATE_PARMS
    if (!current_pc_addr || !pc_ret)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PC_STS, reg_cfg_pc_sts);

    *current_pc_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PC_STS, CURRENT_PC_ADDR, reg_cfg_pc_sts);
    *pc_ret = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PC_STS, PC_RET, reg_cfg_pc_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_set(uint8_t rnr_id, uint16_t addr_base, uint8_t addr_step_0, uint8_t addr_step_1, uint8_t start_thread)
{
    uint32_t reg_cfg_ext_acc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (addr_base >= _13BITS_MAX_VAL_) ||
       (addr_step_0 >= _4BITS_MAX_VAL_) ||
       (addr_step_1 >= _4BITS_MAX_VAL_) ||
       (start_thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ext_acc_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_BASE, reg_cfg_ext_acc_cfg, addr_base);
    reg_cfg_ext_acc_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_STEP_0, reg_cfg_ext_acc_cfg, addr_step_0);
    reg_cfg_ext_acc_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_STEP_1, reg_cfg_ext_acc_cfg, addr_step_1);
    reg_cfg_ext_acc_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, START_THREAD, reg_cfg_ext_acc_cfg, start_thread);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, reg_cfg_ext_acc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_get(uint8_t rnr_id, uint16_t *addr_base, uint8_t *addr_step_0, uint8_t *addr_step_1, uint8_t *start_thread)
{
    uint32_t reg_cfg_ext_acc_cfg;

#ifdef VALIDATE_PARMS
    if (!addr_base || !addr_step_0 || !addr_step_1 || !start_thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, reg_cfg_ext_acc_cfg);

    *addr_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_BASE, reg_cfg_ext_acc_cfg);
    *addr_step_0 = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_STEP_0, reg_cfg_ext_acc_cfg);
    *addr_step_1 = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, ADDR_STEP_1, reg_cfg_ext_acc_cfg);
    *start_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_EXT_ACC_CFG, START_THREAD, reg_cfg_ext_acc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_fit_fail_cfg_set(uint8_t rnr_id, uint16_t start_addr, uint16_t stop_addr)
{
    uint32_t reg_cfg_fit_fail_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (start_addr >= _13BITS_MAX_VAL_) ||
       (stop_addr >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_fit_fail_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, START_ADDR, reg_cfg_fit_fail_cfg, start_addr);
    reg_cfg_fit_fail_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, STOP_ADDR, reg_cfg_fit_fail_cfg, stop_addr);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, reg_cfg_fit_fail_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_fit_fail_cfg_get(uint8_t rnr_id, uint16_t *start_addr, uint16_t *stop_addr)
{
    uint32_t reg_cfg_fit_fail_cfg;

#ifdef VALIDATE_PARMS
    if (!start_addr || !stop_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, reg_cfg_fit_fail_cfg);

    *start_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, START_ADDR, reg_cfg_fit_fail_cfg);
    *stop_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_FIT_FAIL_CFG, STOP_ADDR, reg_cfg_fit_fail_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_data_bkpt_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_data_bkpt_cfg *cfg_data_bkpt_cfg)
{
    uint32_t reg_cfg_data_bkpt_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_data_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (cfg_data_bkpt_cfg->bkpt_0_en >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_0_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_1_en >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_1_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_2_en >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_2_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_3_en >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->bkpt_3_use_thread >= _1BITS_MAX_VAL_) ||
       (cfg_data_bkpt_cfg->reset_data_bkpt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_0_EN, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_0_en);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_0_USE_THREAD, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_0_use_thread);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_1_EN, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_1_en);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_1_USE_THREAD, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_1_use_thread);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_2_EN, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_2_en);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_2_USE_THREAD, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_2_use_thread);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_3_EN, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_3_en);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_3_USE_THREAD, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->bkpt_3_use_thread);
    reg_cfg_data_bkpt_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, RESET_DATA_BKPT, reg_cfg_data_bkpt_cfg, cfg_data_bkpt_cfg->reset_data_bkpt);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, reg_cfg_data_bkpt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_data_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_data_bkpt_cfg *cfg_data_bkpt_cfg)
{
    uint32_t reg_cfg_data_bkpt_cfg;

#ifdef VALIDATE_PARMS
    if (!cfg_data_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, reg_cfg_data_bkpt_cfg);

    cfg_data_bkpt_cfg->bkpt_0_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_0_EN, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_0_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_0_USE_THREAD, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_1_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_1_EN, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_1_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_1_USE_THREAD, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_2_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_2_EN, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_2_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_2_USE_THREAD, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_3_en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_3_EN, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->bkpt_3_use_thread = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, BKPT_3_USE_THREAD, reg_cfg_data_bkpt_cfg);
    cfg_data_bkpt_cfg->reset_data_bkpt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DATA_BKPT_CFG, RESET_DATA_BKPT, reg_cfg_data_bkpt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_aqm_counter_val_get(uint8_t rnr_id, uint32_t *aqm_counter_value)
{
    uint32_t reg_cfg_aqm_counter_val;

#ifdef VALIDATE_PARMS
    if (!aqm_counter_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_AQM_COUNTER_VAL, reg_cfg_aqm_counter_val);

    *aqm_counter_value = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_AQM_COUNTER_VAL, AQM_COUNTER_VALUE, reg_cfg_aqm_counter_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_set(uint8_t rnr_id, uint16_t trace_base_addr, uint16_t trace_max_addr)
{
    uint32_t reg_cfg_profiling_cfg_0 = 0;

#ifdef VALIDATE_PARMS
    if ((rnr_id >= BLOCK_ADDR_COUNT) ||
       (trace_base_addr >= _13BITS_MAX_VAL_) ||
       (trace_max_addr >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_profiling_cfg_0 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, TRACE_BASE_ADDR, reg_cfg_profiling_cfg_0, trace_base_addr);
    reg_cfg_profiling_cfg_0 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, TRACE_MAX_ADDR, reg_cfg_profiling_cfg_0, trace_max_addr);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, reg_cfg_profiling_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_get(uint8_t rnr_id, uint16_t *trace_base_addr, uint16_t *trace_max_addr)
{
    uint32_t reg_cfg_profiling_cfg_0;

#ifdef VALIDATE_PARMS
    if (!trace_base_addr || !trace_max_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, reg_cfg_profiling_cfg_0);

    *trace_base_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, TRACE_BASE_ADDR, reg_cfg_profiling_cfg_0);
    *trace_max_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_0, TRACE_MAX_ADDR, reg_cfg_profiling_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_profiling_counter_get(uint8_t rnr_id, uint32_t *val)
{
    uint32_t reg_cfg_profiling_counter;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_COUNTER, reg_cfg_profiling_counter);

    *val = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_COUNTER, VAL, reg_cfg_profiling_counter);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_cfg_global_ctrl,
    bdmf_address_cfg_cpu_wakeup,
    bdmf_address_cfg_int_ctrl,
    bdmf_address_cfg_int_mask,
    bdmf_address_cfg_gen_cfg,
    bdmf_address_cfg_cam_cfg,
    bdmf_address_cfg_fpm_mini_cfg,
    bdmf_address_cfg_ddr_cfg,
    bdmf_address_cfg_psram_cfg,
    bdmf_address_cfg_ramrd_range_mask_cfg,
    bdmf_address_cfg_sch_cfg,
    bdmf_address_cfg_bkpt_cfg,
    bdmf_address_cfg_bkpt_imm,
    bdmf_address_cfg_bkpt_sts,
    bdmf_address_cfg_pc_sts,
    bdmf_address_cfg_ext_acc_cfg,
    bdmf_address_cfg_fit_fail_cfg,
    bdmf_address_cfg_data_bkpt_cfg,
    bdmf_address_cfg_aqm_counter_val,
    bdmf_address_cfg_stall_cnt1,
    bdmf_address_cfg_stall_cnt2,
    bdmf_address_cfg_stall_cnt3,
    bdmf_address_cfg_stall_cnt4,
    bdmf_address_cfg_stall_cnt5,
    bdmf_address_cfg_stall_cnt6,
    bdmf_address_cfg_stall_cnt7,
    bdmf_address_cfg_profiling_sts,
    bdmf_address_cfg_profiling_cfg_0,
    bdmf_address_cfg_profiling_cfg_1,
    bdmf_address_cfg_profiling_counter,
    bdmf_address_cfg_profiling_cfg_2,
    bdmf_address_cfg_exec_cmds_cnt,
    bdmf_address_cfg_idle_cnt1,
    bdmf_address_cfg_jmp_cnt,
    bdmf_address_cfg_metal_fix_reg,
}
bdmf_address;

static int ag_drv_rnr_regs_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_rnr_regs_rnr_enable:
        ag_err = ag_drv_rnr_regs_rnr_enable_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_rnr_freq:
        ag_err = ag_drv_rnr_regs_rnr_freq_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cam_stop_val:
        ag_err = ag_drv_rnr_regs_cam_stop_val_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_trace_config:
    {
        rnr_regs_trace_config trace_config = { .trace_wraparound = parm[2].value.unumber, .trace_mode = parm[3].value.unumber, .trace_disable_idle_in = parm[4].value.unumber, .trace_disable_wakeup_log = parm[5].value.unumber, .trace_task = parm[6].value.unumber, .idle_counter_source_sel = parm[7].value.unumber, .counters_selected_task_mode = parm[8].value.unumber, .counters_task = parm[9].value.unumber, .profiling_window_mode = parm[10].value.unumber, .single_mode_start_option = parm[11].value.unumber, .single_mode_stop_option = parm[12].value.unumber, .window_manual_start = parm[13].value.unumber, .window_manual_stop = parm[14].value.unumber, .tracer_enable = parm[15].value.unumber, .profiling_window_reset = parm[16].value.unumber, .profiling_window_enable = parm[17].value.unumber, .trace_reset_event_fifo = parm[18].value.unumber, .trace_clear_fifo_overrun = parm[19].value.unumber, .trigger_on_second = parm[20].value.unumber, .pc_start = parm[21].value.unumber, .pc_stop_or_cycle_count = parm[22].value.unumber};
        ag_err = ag_drv_rnr_regs_trace_config_set(parm[1].value.unumber, &trace_config);
        break;
    }
    case cli_rnr_regs_reset_trace_fifo:
        ag_err = ag_drv_rnr_regs_reset_trace_fifo_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_clear_trace_fifo_overrun:
        ag_err = ag_drv_rnr_regs_clear_trace_fifo_overrun_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_cpu_wakeup:
        ag_err = ag_drv_rnr_regs_cfg_cpu_wakeup_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_int_ctrl:
    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl = { .int0_sts = parm[2].value.unumber, .int1_sts = parm[3].value.unumber, .int2_sts = parm[4].value.unumber, .int3_sts = parm[5].value.unumber, .int4_sts = parm[6].value.unumber, .int5_sts = parm[7].value.unumber, .int6_sts = parm[8].value.unumber, .int7_sts = parm[9].value.unumber, .int8_sts = parm[10].value.unumber, .int9_sts = parm[11].value.unumber, .fit_fail_sts = parm[12].value.unumber};
        ag_err = ag_drv_rnr_regs_cfg_int_ctrl_set(parm[1].value.unumber, &cfg_int_ctrl);
        break;
    }
    case cli_rnr_regs_cfg_int_mask:
    {
        rnr_regs_cfg_int_mask cfg_int_mask = { .int0_mask = parm[2].value.unumber, .int1_mask = parm[3].value.unumber, .int2_mask = parm[4].value.unumber, .int3_mask = parm[5].value.unumber, .int4_mask = parm[6].value.unumber, .int5_mask = parm[7].value.unumber, .int6_mask = parm[8].value.unumber, .int7_mask = parm[9].value.unumber, .int8_mask = parm[10].value.unumber, .int9_mask = parm[11].value.unumber};
        ag_err = ag_drv_rnr_regs_cfg_int_mask_set(parm[1].value.unumber, &cfg_int_mask);
        break;
    }
    case cli_rnr_regs_cfg_gen_cfg:
    {
        rnr_regs_cfg_gen_cfg cfg_gen_cfg = { .disable_dma_old_flow_control = parm[2].value.unumber, .test_fit_fail = parm[3].value.unumber, .zero_data_mem = parm[4].value.unumber, .zero_context_mem = parm[5].value.unumber, .zero_data_mem_done = parm[6].value.unumber, .zero_context_mem_done = parm[7].value.unumber, .chicken_disable_skip_jmp = parm[8].value.unumber, .chicken_disable_alu_load_balancing = parm[9].value.unumber, .gdma_desc_offset = parm[10].value.unumber, .bbtx_tcam_dest_sel = parm[11].value.unumber, .bbtx_hash_dest_sel = parm[12].value.unumber, .bbtx_natc_dest_sel = parm[13].value.unumber, .bbtx_cnpl_dest_sel = parm[14].value.unumber, .gdma_gdesc_buffer_size = parm[15].value.unumber, .chicken_enable_old_unique_id_mode = parm[16].value.unumber, .chicken_enable_dma_old_mode = parm[17].value.unumber, .prevent_cs_till_stores_done = parm[18].value.unumber};
        ag_err = ag_drv_rnr_regs_cfg_gen_cfg_set(parm[1].value.unumber, &cfg_gen_cfg);
        break;
    }
    case cli_rnr_regs_cfg_fpm_mini_cfg:
        ag_err = ag_drv_rnr_regs_cfg_fpm_mini_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_regs_cfg_ddr_cfg:
        ag_err = ag_drv_rnr_regs_cfg_ddr_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_regs_cfg_psram_cfg:
        ag_err = ag_drv_rnr_regs_cfg_psram_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_regs_cfg_ramrd_range_mask_cfg:
        ag_err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_regs_cfg_sch_cfg:
        ag_err = ag_drv_rnr_regs_cfg_sch_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_bkpt_cfg:
    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg = { .bkpt_0_en = parm[2].value.unumber, .bkpt_0_use_thread = parm[3].value.unumber, .bkpt_1_en = parm[4].value.unumber, .bkpt_1_use_thread = parm[5].value.unumber, .bkpt_2_en = parm[6].value.unumber, .bkpt_2_use_thread = parm[7].value.unumber, .bkpt_3_en = parm[8].value.unumber, .bkpt_3_use_thread = parm[9].value.unumber, .bkpt_4_en = parm[10].value.unumber, .bkpt_4_use_thread = parm[11].value.unumber, .bkpt_5_en = parm[12].value.unumber, .bkpt_5_use_thread = parm[13].value.unumber, .bkpt_6_en = parm[14].value.unumber, .bkpt_6_use_thread = parm[15].value.unumber, .bkpt_7_en = parm[16].value.unumber, .bkpt_7_use_thread = parm[17].value.unumber, .step_mode = parm[18].value.unumber, .new_flags_val = parm[19].value.unumber, .enable_breakpoint_on_fit_fail = parm[20].value.unumber};
        ag_err = ag_drv_rnr_regs_cfg_bkpt_cfg_set(parm[1].value.unumber, &cfg_bkpt_cfg);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_imm:
        ag_err = ag_drv_rnr_regs_cfg_bkpt_imm_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_ext_acc_cfg:
        ag_err = ag_drv_rnr_regs_cfg_ext_acc_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_regs_cfg_fit_fail_cfg:
        ag_err = ag_drv_rnr_regs_cfg_fit_fail_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_regs_cfg_data_bkpt_cfg:
    {
        rnr_regs_cfg_data_bkpt_cfg cfg_data_bkpt_cfg = { .bkpt_0_en = parm[2].value.unumber, .bkpt_0_use_thread = parm[3].value.unumber, .bkpt_1_en = parm[4].value.unumber, .bkpt_1_use_thread = parm[5].value.unumber, .bkpt_2_en = parm[6].value.unumber, .bkpt_2_use_thread = parm[7].value.unumber, .bkpt_3_en = parm[8].value.unumber, .bkpt_3_use_thread = parm[9].value.unumber, .reset_data_bkpt = parm[10].value.unumber};
        ag_err = ag_drv_rnr_regs_cfg_data_bkpt_cfg_set(parm[1].value.unumber, &cfg_data_bkpt_cfg);
        break;
    }
    case cli_rnr_regs_cfg_profiling_cfg_0:
        ag_err = ag_drv_rnr_regs_cfg_profiling_cfg_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_rnr_regs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_rnr_regs_rnr_enable:
    {
        bdmf_boolean en;
        ag_err = ag_drv_rnr_regs_rnr_enable_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_rnr_regs_dma_illegal:
    {
        bdmf_boolean dma_illegal_status;
        ag_err = ag_drv_rnr_regs_dma_illegal_get(parm[1].value.unumber, &dma_illegal_status);
        bdmf_session_print(session, "dma_illegal_status = %u = 0x%x\n", dma_illegal_status, dma_illegal_status);
        break;
    }
    case cli_rnr_regs_prediction_overrun:
    {
        bdmf_boolean prediction_overrun_status;
        ag_err = ag_drv_rnr_regs_prediction_overrun_get(parm[1].value.unumber, &prediction_overrun_status);
        bdmf_session_print(session, "prediction_overrun_status = %u = 0x%x\n", prediction_overrun_status, prediction_overrun_status);
        break;
    }
    case cli_rnr_regs_rnr_freq:
    {
        uint16_t micro_sec_val;
        ag_err = ag_drv_rnr_regs_rnr_freq_get(parm[1].value.unumber, &micro_sec_val);
        bdmf_session_print(session, "micro_sec_val = %u = 0x%x\n", micro_sec_val, micro_sec_val);
        break;
    }
    case cli_rnr_regs_cam_stop_val:
    {
        uint16_t stop_value;
        ag_err = ag_drv_rnr_regs_cam_stop_val_get(parm[1].value.unumber, &stop_value);
        bdmf_session_print(session, "stop_value = %u = 0x%x\n", stop_value, stop_value);
        break;
    }
    case cli_rnr_regs_profiling_sts:
    {
        rnr_regs_profiling_sts profiling_sts;
        ag_err = ag_drv_rnr_regs_profiling_sts_get(parm[1].value.unumber, &profiling_sts);
        bdmf_session_print(session, "trace_write_pnt = %u = 0x%x\n", profiling_sts.trace_write_pnt, profiling_sts.trace_write_pnt);
        bdmf_session_print(session, "idle_no_active_task = %u = 0x%x\n", profiling_sts.idle_no_active_task, profiling_sts.idle_no_active_task);
        bdmf_session_print(session, "curr_thread_num = %u = 0x%x\n", profiling_sts.curr_thread_num, profiling_sts.curr_thread_num);
        bdmf_session_print(session, "profiling_active = %u = 0x%x\n", profiling_sts.profiling_active, profiling_sts.profiling_active);
        bdmf_session_print(session, "trace_fifo_overrun = %u = 0x%x\n", profiling_sts.trace_fifo_overrun, profiling_sts.trace_fifo_overrun);
        bdmf_session_print(session, "single_mode_profiling_status = %u = 0x%x\n", profiling_sts.single_mode_profiling_status, profiling_sts.single_mode_profiling_status);
        break;
    }
    case cli_rnr_regs_is_trace_fifo_overrun:
    {
        bdmf_boolean trace_fifo_overrun;
        ag_err = ag_drv_rnr_regs_is_trace_fifo_overrun_get(parm[1].value.unumber, &trace_fifo_overrun);
        bdmf_session_print(session, "trace_fifo_overrun = %u = 0x%x\n", trace_fifo_overrun, trace_fifo_overrun);
        break;
    }
    case cli_rnr_regs_trace_config:
    {
        rnr_regs_trace_config trace_config;
        ag_err = ag_drv_rnr_regs_trace_config_get(parm[1].value.unumber, &trace_config);
        bdmf_session_print(session, "trace_wraparound = %u = 0x%x\n", trace_config.trace_wraparound, trace_config.trace_wraparound);
        bdmf_session_print(session, "trace_mode = %u = 0x%x\n", trace_config.trace_mode, trace_config.trace_mode);
        bdmf_session_print(session, "trace_disable_idle_in = %u = 0x%x\n", trace_config.trace_disable_idle_in, trace_config.trace_disable_idle_in);
        bdmf_session_print(session, "trace_disable_wakeup_log = %u = 0x%x\n", trace_config.trace_disable_wakeup_log, trace_config.trace_disable_wakeup_log);
        bdmf_session_print(session, "trace_task = %u = 0x%x\n", trace_config.trace_task, trace_config.trace_task);
        bdmf_session_print(session, "idle_counter_source_sel = %u = 0x%x\n", trace_config.idle_counter_source_sel, trace_config.idle_counter_source_sel);
        bdmf_session_print(session, "counters_selected_task_mode = %u = 0x%x\n", trace_config.counters_selected_task_mode, trace_config.counters_selected_task_mode);
        bdmf_session_print(session, "counters_task = %u = 0x%x\n", trace_config.counters_task, trace_config.counters_task);
        bdmf_session_print(session, "profiling_window_mode = %u = 0x%x\n", trace_config.profiling_window_mode, trace_config.profiling_window_mode);
        bdmf_session_print(session, "single_mode_start_option = %u = 0x%x\n", trace_config.single_mode_start_option, trace_config.single_mode_start_option);
        bdmf_session_print(session, "single_mode_stop_option = %u = 0x%x\n", trace_config.single_mode_stop_option, trace_config.single_mode_stop_option);
        bdmf_session_print(session, "window_manual_start = %u = 0x%x\n", trace_config.window_manual_start, trace_config.window_manual_start);
        bdmf_session_print(session, "window_manual_stop = %u = 0x%x\n", trace_config.window_manual_stop, trace_config.window_manual_stop);
        bdmf_session_print(session, "tracer_enable = %u = 0x%x\n", trace_config.tracer_enable, trace_config.tracer_enable);
        bdmf_session_print(session, "profiling_window_reset = %u = 0x%x\n", trace_config.profiling_window_reset, trace_config.profiling_window_reset);
        bdmf_session_print(session, "profiling_window_enable = %u = 0x%x\n", trace_config.profiling_window_enable, trace_config.profiling_window_enable);
        bdmf_session_print(session, "trace_reset_event_fifo = %u = 0x%x\n", trace_config.trace_reset_event_fifo, trace_config.trace_reset_event_fifo);
        bdmf_session_print(session, "trace_clear_fifo_overrun = %u = 0x%x\n", trace_config.trace_clear_fifo_overrun, trace_config.trace_clear_fifo_overrun);
        bdmf_session_print(session, "trigger_on_second = %u = 0x%x\n", trace_config.trigger_on_second, trace_config.trigger_on_second);
        bdmf_session_print(session, "pc_start = %u = 0x%x\n", trace_config.pc_start, trace_config.pc_start);
        bdmf_session_print(session, "pc_stop_or_cycle_count = %u = 0x%x\n", trace_config.pc_stop_or_cycle_count, trace_config.pc_stop_or_cycle_count);
        break;
    }
    case cli_rnr_regs_reset_trace_fifo:
    {
        bdmf_boolean trace_reset_event_fifo;
        ag_err = ag_drv_rnr_regs_reset_trace_fifo_get(parm[1].value.unumber, &trace_reset_event_fifo);
        bdmf_session_print(session, "trace_reset_event_fifo = %u = 0x%x\n", trace_reset_event_fifo, trace_reset_event_fifo);
        break;
    }
    case cli_rnr_regs_clear_trace_fifo_overrun:
    {
        bdmf_boolean trace_clear_fifo_overrun;
        ag_err = ag_drv_rnr_regs_clear_trace_fifo_overrun_get(parm[1].value.unumber, &trace_clear_fifo_overrun);
        bdmf_session_print(session, "trace_clear_fifo_overrun = %u = 0x%x\n", trace_clear_fifo_overrun, trace_clear_fifo_overrun);
        break;
    }
    case cli_rnr_regs_rnr_core_cntrs:
    {
        rnr_regs_rnr_core_cntrs rnr_core_cntrs;
        ag_err = ag_drv_rnr_regs_rnr_core_cntrs_get(parm[1].value.unumber, &rnr_core_cntrs);
        bdmf_session_print(session, "total_stall_cnt = %u = 0x%x\n", rnr_core_cntrs.total_stall_cnt, rnr_core_cntrs.total_stall_cnt);
        bdmf_session_print(session, "stall_on_alu_b_full_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_alu_b_full_cnt, rnr_core_cntrs.stall_on_alu_b_full_cnt);
        bdmf_session_print(session, "stall_on_alu_a_full_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_alu_a_full_cnt, rnr_core_cntrs.stall_on_alu_a_full_cnt);
        bdmf_session_print(session, "stall_on_jmpreg = %u = 0x%x\n", rnr_core_cntrs.stall_on_jmpreg, rnr_core_cntrs.stall_on_jmpreg);
        bdmf_session_print(session, "stall_on_memio_full_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_memio_full_cnt, rnr_core_cntrs.stall_on_memio_full_cnt);
        bdmf_session_print(session, "stall_on_waw_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_waw_cnt, rnr_core_cntrs.stall_on_waw_cnt);
        bdmf_session_print(session, "stall_on_super_cmd = %u = 0x%x\n", rnr_core_cntrs.stall_on_super_cmd, rnr_core_cntrs.stall_on_super_cmd);
        bdmf_session_print(session, "stall_on_super_cmd_when_full = %u = 0x%x\n", rnr_core_cntrs.stall_on_super_cmd_when_full, rnr_core_cntrs.stall_on_super_cmd_when_full);
        bdmf_session_print(session, "stall_on_cs_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_cs_cnt, rnr_core_cntrs.stall_on_cs_cnt);
        bdmf_session_print(session, "active_cycles_cnt = %u = 0x%x\n", rnr_core_cntrs.active_cycles_cnt, rnr_core_cntrs.active_cycles_cnt);
        bdmf_session_print(session, "stall_on_jmp_full_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_jmp_full_cnt, rnr_core_cntrs.stall_on_jmp_full_cnt);
        bdmf_session_print(session, "stall_on_skip_jmp_cnt = %u = 0x%x\n", rnr_core_cntrs.stall_on_skip_jmp_cnt, rnr_core_cntrs.stall_on_skip_jmp_cnt);
        bdmf_session_print(session, "exec_counter = %u = 0x%x\n", rnr_core_cntrs.exec_counter, rnr_core_cntrs.exec_counter);
        bdmf_session_print(session, "idle_cnt = %u = 0x%x\n", rnr_core_cntrs.idle_cnt, rnr_core_cntrs.idle_cnt);
        bdmf_session_print(session, "jmp_taken_predicted_untaken_cnt = %u = 0x%x\n", rnr_core_cntrs.jmp_taken_predicted_untaken_cnt, rnr_core_cntrs.jmp_taken_predicted_untaken_cnt);
        bdmf_session_print(session, "jmp_untaken_predicted_taken_cnt = %u = 0x%x\n", rnr_core_cntrs.jmp_untaken_predicted_taken_cnt, rnr_core_cntrs.jmp_untaken_predicted_taken_cnt);
        break;
    }
    case cli_rnr_regs_cfg_cpu_wakeup:
    {
        uint8_t thread_num;
        ag_err = ag_drv_rnr_regs_cfg_cpu_wakeup_get(parm[1].value.unumber, &thread_num);
        bdmf_session_print(session, "thread_num = %u = 0x%x\n", thread_num, thread_num);
        break;
    }
    case cli_rnr_regs_cfg_int_ctrl:
    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl;
        ag_err = ag_drv_rnr_regs_cfg_int_ctrl_get(parm[1].value.unumber, &cfg_int_ctrl);
        bdmf_session_print(session, "int0_sts = %u = 0x%x\n", cfg_int_ctrl.int0_sts, cfg_int_ctrl.int0_sts);
        bdmf_session_print(session, "int1_sts = %u = 0x%x\n", cfg_int_ctrl.int1_sts, cfg_int_ctrl.int1_sts);
        bdmf_session_print(session, "int2_sts = %u = 0x%x\n", cfg_int_ctrl.int2_sts, cfg_int_ctrl.int2_sts);
        bdmf_session_print(session, "int3_sts = %u = 0x%x\n", cfg_int_ctrl.int3_sts, cfg_int_ctrl.int3_sts);
        bdmf_session_print(session, "int4_sts = %u = 0x%x\n", cfg_int_ctrl.int4_sts, cfg_int_ctrl.int4_sts);
        bdmf_session_print(session, "int5_sts = %u = 0x%x\n", cfg_int_ctrl.int5_sts, cfg_int_ctrl.int5_sts);
        bdmf_session_print(session, "int6_sts = %u = 0x%x\n", cfg_int_ctrl.int6_sts, cfg_int_ctrl.int6_sts);
        bdmf_session_print(session, "int7_sts = %u = 0x%x\n", cfg_int_ctrl.int7_sts, cfg_int_ctrl.int7_sts);
        bdmf_session_print(session, "int8_sts = %u = 0x%x\n", cfg_int_ctrl.int8_sts, cfg_int_ctrl.int8_sts);
        bdmf_session_print(session, "int9_sts = %u = 0x%x\n", cfg_int_ctrl.int9_sts, cfg_int_ctrl.int9_sts);
        bdmf_session_print(session, "fit_fail_sts = %u = 0x%x\n", cfg_int_ctrl.fit_fail_sts, cfg_int_ctrl.fit_fail_sts);
        break;
    }
    case cli_rnr_regs_cfg_int_mask:
    {
        rnr_regs_cfg_int_mask cfg_int_mask;
        ag_err = ag_drv_rnr_regs_cfg_int_mask_get(parm[1].value.unumber, &cfg_int_mask);
        bdmf_session_print(session, "int0_mask = %u = 0x%x\n", cfg_int_mask.int0_mask, cfg_int_mask.int0_mask);
        bdmf_session_print(session, "int1_mask = %u = 0x%x\n", cfg_int_mask.int1_mask, cfg_int_mask.int1_mask);
        bdmf_session_print(session, "int2_mask = %u = 0x%x\n", cfg_int_mask.int2_mask, cfg_int_mask.int2_mask);
        bdmf_session_print(session, "int3_mask = %u = 0x%x\n", cfg_int_mask.int3_mask, cfg_int_mask.int3_mask);
        bdmf_session_print(session, "int4_mask = %u = 0x%x\n", cfg_int_mask.int4_mask, cfg_int_mask.int4_mask);
        bdmf_session_print(session, "int5_mask = %u = 0x%x\n", cfg_int_mask.int5_mask, cfg_int_mask.int5_mask);
        bdmf_session_print(session, "int6_mask = %u = 0x%x\n", cfg_int_mask.int6_mask, cfg_int_mask.int6_mask);
        bdmf_session_print(session, "int7_mask = %u = 0x%x\n", cfg_int_mask.int7_mask, cfg_int_mask.int7_mask);
        bdmf_session_print(session, "int8_mask = %u = 0x%x\n", cfg_int_mask.int8_mask, cfg_int_mask.int8_mask);
        bdmf_session_print(session, "int9_mask = %u = 0x%x\n", cfg_int_mask.int9_mask, cfg_int_mask.int9_mask);
        break;
    }
    case cli_rnr_regs_cfg_gen_cfg:
    {
        rnr_regs_cfg_gen_cfg cfg_gen_cfg;
        ag_err = ag_drv_rnr_regs_cfg_gen_cfg_get(parm[1].value.unumber, &cfg_gen_cfg);
        bdmf_session_print(session, "disable_dma_old_flow_control = %u = 0x%x\n", cfg_gen_cfg.disable_dma_old_flow_control, cfg_gen_cfg.disable_dma_old_flow_control);
        bdmf_session_print(session, "test_fit_fail = %u = 0x%x\n", cfg_gen_cfg.test_fit_fail, cfg_gen_cfg.test_fit_fail);
        bdmf_session_print(session, "zero_data_mem = %u = 0x%x\n", cfg_gen_cfg.zero_data_mem, cfg_gen_cfg.zero_data_mem);
        bdmf_session_print(session, "zero_context_mem = %u = 0x%x\n", cfg_gen_cfg.zero_context_mem, cfg_gen_cfg.zero_context_mem);
        bdmf_session_print(session, "zero_data_mem_done = %u = 0x%x\n", cfg_gen_cfg.zero_data_mem_done, cfg_gen_cfg.zero_data_mem_done);
        bdmf_session_print(session, "zero_context_mem_done = %u = 0x%x\n", cfg_gen_cfg.zero_context_mem_done, cfg_gen_cfg.zero_context_mem_done);
        bdmf_session_print(session, "chicken_disable_skip_jmp = %u = 0x%x\n", cfg_gen_cfg.chicken_disable_skip_jmp, cfg_gen_cfg.chicken_disable_skip_jmp);
        bdmf_session_print(session, "chicken_disable_alu_load_balancing = %u = 0x%x\n", cfg_gen_cfg.chicken_disable_alu_load_balancing, cfg_gen_cfg.chicken_disable_alu_load_balancing);
        bdmf_session_print(session, "gdma_desc_offset = %u = 0x%x\n", cfg_gen_cfg.gdma_desc_offset, cfg_gen_cfg.gdma_desc_offset);
        bdmf_session_print(session, "bbtx_tcam_dest_sel = %u = 0x%x\n", cfg_gen_cfg.bbtx_tcam_dest_sel, cfg_gen_cfg.bbtx_tcam_dest_sel);
        bdmf_session_print(session, "bbtx_hash_dest_sel = %u = 0x%x\n", cfg_gen_cfg.bbtx_hash_dest_sel, cfg_gen_cfg.bbtx_hash_dest_sel);
        bdmf_session_print(session, "bbtx_natc_dest_sel = %u = 0x%x\n", cfg_gen_cfg.bbtx_natc_dest_sel, cfg_gen_cfg.bbtx_natc_dest_sel);
        bdmf_session_print(session, "bbtx_cnpl_dest_sel = %u = 0x%x\n", cfg_gen_cfg.bbtx_cnpl_dest_sel, cfg_gen_cfg.bbtx_cnpl_dest_sel);
        bdmf_session_print(session, "gdma_gdesc_buffer_size = %u = 0x%x\n", cfg_gen_cfg.gdma_gdesc_buffer_size, cfg_gen_cfg.gdma_gdesc_buffer_size);
        bdmf_session_print(session, "chicken_enable_old_unique_id_mode = %u = 0x%x\n", cfg_gen_cfg.chicken_enable_old_unique_id_mode, cfg_gen_cfg.chicken_enable_old_unique_id_mode);
        bdmf_session_print(session, "chicken_enable_dma_old_mode = %u = 0x%x\n", cfg_gen_cfg.chicken_enable_dma_old_mode, cfg_gen_cfg.chicken_enable_dma_old_mode);
        bdmf_session_print(session, "prevent_cs_till_stores_done = %u = 0x%x\n", cfg_gen_cfg.prevent_cs_till_stores_done, cfg_gen_cfg.prevent_cs_till_stores_done);
        break;
    }
    case cli_rnr_regs_cfg_fpm_mini_cfg:
    {
        uint32_t dma_base;
        uint8_t dma_static_offset;
        ag_err = ag_drv_rnr_regs_cfg_fpm_mini_cfg_get(parm[1].value.unumber, &dma_base, &dma_static_offset);
        bdmf_session_print(session, "dma_base = %u = 0x%x\n", dma_base, dma_base);
        bdmf_session_print(session, "dma_static_offset = %u = 0x%x\n", dma_static_offset, dma_static_offset);
        break;
    }
    case cli_rnr_regs_cfg_ddr_cfg:
    {
        uint32_t dma_base;
        uint8_t dma_buf_size;
        bdmf_boolean dma_buf_size_mode;
        uint8_t dma_static_offset;
        ag_err = ag_drv_rnr_regs_cfg_ddr_cfg_get(parm[1].value.unumber, &dma_base, &dma_buf_size, &dma_buf_size_mode, &dma_static_offset);
        bdmf_session_print(session, "dma_base = %u = 0x%x\n", dma_base, dma_base);
        bdmf_session_print(session, "dma_buf_size = %u = 0x%x\n", dma_buf_size, dma_buf_size);
        bdmf_session_print(session, "dma_buf_size_mode = %u = 0x%x\n", dma_buf_size_mode, dma_buf_size_mode);
        bdmf_session_print(session, "dma_static_offset = %u = 0x%x\n", dma_static_offset, dma_static_offset);
        break;
    }
    case cli_rnr_regs_cfg_psram_cfg:
    {
        uint32_t dma_base;
        uint8_t dma_buf_size;
        bdmf_boolean dma_buf_size_mode;
        uint8_t dma_static_offset;
        ag_err = ag_drv_rnr_regs_cfg_psram_cfg_get(parm[1].value.unumber, &dma_base, &dma_buf_size, &dma_buf_size_mode, &dma_static_offset);
        bdmf_session_print(session, "dma_base = %u = 0x%x\n", dma_base, dma_base);
        bdmf_session_print(session, "dma_buf_size = %u = 0x%x\n", dma_buf_size, dma_buf_size);
        bdmf_session_print(session, "dma_buf_size_mode = %u = 0x%x\n", dma_buf_size_mode, dma_buf_size_mode);
        bdmf_session_print(session, "dma_static_offset = %u = 0x%x\n", dma_static_offset, dma_static_offset);
        break;
    }
    case cli_rnr_regs_cfg_ramrd_range_mask_cfg:
    {
        uint16_t mask0;
        uint16_t mask1;
        ag_err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(parm[1].value.unumber, &mask0, &mask1);
        bdmf_session_print(session, "mask0 = %u = 0x%x\n", mask0, mask0);
        bdmf_session_print(session, "mask1 = %u = 0x%x\n", mask1, mask1);
        break;
    }
    case cli_rnr_regs_cfg_sch_cfg:
    {
        uint8_t scheduler_mode;
        ag_err = ag_drv_rnr_regs_cfg_sch_cfg_get(parm[1].value.unumber, &scheduler_mode);
        bdmf_session_print(session, "scheduler_mode = %u = 0x%x\n", scheduler_mode, scheduler_mode);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_cfg:
    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg;
        ag_err = ag_drv_rnr_regs_cfg_bkpt_cfg_get(parm[1].value.unumber, &cfg_bkpt_cfg);
        bdmf_session_print(session, "bkpt_0_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_en);
        bdmf_session_print(session, "bkpt_0_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_0_use_thread);
        bdmf_session_print(session, "bkpt_1_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_en);
        bdmf_session_print(session, "bkpt_1_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_1_use_thread, cfg_bkpt_cfg.bkpt_1_use_thread);
        bdmf_session_print(session, "bkpt_2_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_en);
        bdmf_session_print(session, "bkpt_2_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_2_use_thread);
        bdmf_session_print(session, "bkpt_3_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_en);
        bdmf_session_print(session, "bkpt_3_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_3_use_thread, cfg_bkpt_cfg.bkpt_3_use_thread);
        bdmf_session_print(session, "bkpt_4_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_en);
        bdmf_session_print(session, "bkpt_4_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_4_use_thread);
        bdmf_session_print(session, "bkpt_5_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_en);
        bdmf_session_print(session, "bkpt_5_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_5_use_thread, cfg_bkpt_cfg.bkpt_5_use_thread);
        bdmf_session_print(session, "bkpt_6_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_en);
        bdmf_session_print(session, "bkpt_6_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_6_use_thread);
        bdmf_session_print(session, "bkpt_7_en = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_en);
        bdmf_session_print(session, "bkpt_7_use_thread = %u = 0x%x\n", cfg_bkpt_cfg.bkpt_7_use_thread, cfg_bkpt_cfg.bkpt_7_use_thread);
        bdmf_session_print(session, "step_mode = %u = 0x%x\n", cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.step_mode);
        bdmf_session_print(session, "new_flags_val = %u = 0x%x\n", cfg_bkpt_cfg.new_flags_val, cfg_bkpt_cfg.new_flags_val);
        bdmf_session_print(session, "enable_breakpoint_on_fit_fail = %u = 0x%x\n", cfg_bkpt_cfg.enable_breakpoint_on_fit_fail, cfg_bkpt_cfg.enable_breakpoint_on_fit_fail);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_imm:
    {
        bdmf_boolean enable;
        ag_err = ag_drv_rnr_regs_cfg_bkpt_imm_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_sts:
    {
        uint16_t bkpt_addr;
        bdmf_boolean active;
        uint16_t data_bkpt_addr;
        uint8_t bkpt_reason;
        ag_err = ag_drv_rnr_regs_cfg_bkpt_sts_get(parm[1].value.unumber, &bkpt_addr, &active, &data_bkpt_addr, &bkpt_reason);
        bdmf_session_print(session, "bkpt_addr = %u = 0x%x\n", bkpt_addr, bkpt_addr);
        bdmf_session_print(session, "active = %u = 0x%x\n", active, active);
        bdmf_session_print(session, "data_bkpt_addr = %u = 0x%x\n", data_bkpt_addr, data_bkpt_addr);
        bdmf_session_print(session, "bkpt_reason = %u = 0x%x\n", bkpt_reason, bkpt_reason);
        break;
    }
    case cli_rnr_regs_cfg_pc_sts:
    {
        uint16_t current_pc_addr;
        uint16_t pc_ret;
        ag_err = ag_drv_rnr_regs_cfg_pc_sts_get(parm[1].value.unumber, &current_pc_addr, &pc_ret);
        bdmf_session_print(session, "current_pc_addr = %u = 0x%x\n", current_pc_addr, current_pc_addr);
        bdmf_session_print(session, "pc_ret = %u = 0x%x\n", pc_ret, pc_ret);
        break;
    }
    case cli_rnr_regs_cfg_ext_acc_cfg:
    {
        uint16_t addr_base;
        uint8_t addr_step_0;
        uint8_t addr_step_1;
        uint8_t start_thread;
        ag_err = ag_drv_rnr_regs_cfg_ext_acc_cfg_get(parm[1].value.unumber, &addr_base, &addr_step_0, &addr_step_1, &start_thread);
        bdmf_session_print(session, "addr_base = %u = 0x%x\n", addr_base, addr_base);
        bdmf_session_print(session, "addr_step_0 = %u = 0x%x\n", addr_step_0, addr_step_0);
        bdmf_session_print(session, "addr_step_1 = %u = 0x%x\n", addr_step_1, addr_step_1);
        bdmf_session_print(session, "start_thread = %u = 0x%x\n", start_thread, start_thread);
        break;
    }
    case cli_rnr_regs_cfg_fit_fail_cfg:
    {
        uint16_t start_addr;
        uint16_t stop_addr;
        ag_err = ag_drv_rnr_regs_cfg_fit_fail_cfg_get(parm[1].value.unumber, &start_addr, &stop_addr);
        bdmf_session_print(session, "start_addr = %u = 0x%x\n", start_addr, start_addr);
        bdmf_session_print(session, "stop_addr = %u = 0x%x\n", stop_addr, stop_addr);
        break;
    }
    case cli_rnr_regs_cfg_data_bkpt_cfg:
    {
        rnr_regs_cfg_data_bkpt_cfg cfg_data_bkpt_cfg;
        ag_err = ag_drv_rnr_regs_cfg_data_bkpt_cfg_get(parm[1].value.unumber, &cfg_data_bkpt_cfg);
        bdmf_session_print(session, "bkpt_0_en = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_0_en, cfg_data_bkpt_cfg.bkpt_0_en);
        bdmf_session_print(session, "bkpt_0_use_thread = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_0_use_thread, cfg_data_bkpt_cfg.bkpt_0_use_thread);
        bdmf_session_print(session, "bkpt_1_en = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_1_en, cfg_data_bkpt_cfg.bkpt_1_en);
        bdmf_session_print(session, "bkpt_1_use_thread = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_1_use_thread, cfg_data_bkpt_cfg.bkpt_1_use_thread);
        bdmf_session_print(session, "bkpt_2_en = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_2_en, cfg_data_bkpt_cfg.bkpt_2_en);
        bdmf_session_print(session, "bkpt_2_use_thread = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_2_use_thread, cfg_data_bkpt_cfg.bkpt_2_use_thread);
        bdmf_session_print(session, "bkpt_3_en = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_3_en, cfg_data_bkpt_cfg.bkpt_3_en);
        bdmf_session_print(session, "bkpt_3_use_thread = %u = 0x%x\n", cfg_data_bkpt_cfg.bkpt_3_use_thread, cfg_data_bkpt_cfg.bkpt_3_use_thread);
        bdmf_session_print(session, "reset_data_bkpt = %u = 0x%x\n", cfg_data_bkpt_cfg.reset_data_bkpt, cfg_data_bkpt_cfg.reset_data_bkpt);
        break;
    }
    case cli_rnr_regs_cfg_aqm_counter_val:
    {
        uint32_t aqm_counter_value;
        ag_err = ag_drv_rnr_regs_cfg_aqm_counter_val_get(parm[1].value.unumber, &aqm_counter_value);
        bdmf_session_print(session, "aqm_counter_value = %u = 0x%x\n", aqm_counter_value, aqm_counter_value);
        break;
    }
    case cli_rnr_regs_cfg_profiling_cfg_0:
    {
        uint16_t trace_base_addr;
        uint16_t trace_max_addr;
        ag_err = ag_drv_rnr_regs_cfg_profiling_cfg_0_get(parm[1].value.unumber, &trace_base_addr, &trace_max_addr);
        bdmf_session_print(session, "trace_base_addr = %u = 0x%x\n", trace_base_addr, trace_base_addr);
        bdmf_session_print(session, "trace_max_addr = %u = 0x%x\n", trace_max_addr, trace_max_addr);
        break;
    }
    case cli_rnr_regs_cfg_profiling_counter:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_regs_cfg_profiling_counter_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_rnr_regs_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t rnr_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_regs_rnr_enable_set(%u %u)\n", rnr_id,
            en);
        ag_err = ag_drv_rnr_regs_rnr_enable_set(rnr_id, en);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_rnr_enable_get(rnr_id, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_rnr_enable_get(%u %u)\n", rnr_id,
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
        bdmf_boolean dma_illegal_status = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_dma_illegal_get(rnr_id, &dma_illegal_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_dma_illegal_get(%u %u)\n", rnr_id,
                dma_illegal_status);
        }
    }

    {
        bdmf_boolean prediction_overrun_status = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_prediction_overrun_get(rnr_id, &prediction_overrun_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_prediction_overrun_get(%u %u)\n", rnr_id,
                prediction_overrun_status);
        }
    }

    {
        uint16_t micro_sec_val = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_regs_rnr_freq_set(%u %u)\n", rnr_id,
            micro_sec_val);
        ag_err = ag_drv_rnr_regs_rnr_freq_set(rnr_id, micro_sec_val);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_rnr_freq_get(rnr_id, &micro_sec_val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_rnr_freq_get(%u %u)\n", rnr_id,
                micro_sec_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (micro_sec_val != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t stop_value = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_regs_cam_stop_val_set(%u %u)\n", rnr_id,
            stop_value);
        ag_err = ag_drv_rnr_regs_cam_stop_val_set(rnr_id, stop_value);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cam_stop_val_get(rnr_id, &stop_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cam_stop_val_get(%u %u)\n", rnr_id,
                stop_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (stop_value != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_profiling_sts profiling_sts = {.trace_write_pnt = gtmv(m, 13), .idle_no_active_task = gtmv(m, 1), .curr_thread_num = gtmv(m, 4), .profiling_active = gtmv(m, 1), .trace_fifo_overrun = gtmv(m, 1), .single_mode_profiling_status = gtmv(m, 2)};
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_profiling_sts_get(rnr_id, &profiling_sts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_profiling_sts_get(%u %u %u %u %u %u %u)\n", rnr_id,
                profiling_sts.trace_write_pnt, profiling_sts.idle_no_active_task, profiling_sts.curr_thread_num, profiling_sts.profiling_active, 
                profiling_sts.trace_fifo_overrun, profiling_sts.single_mode_profiling_status);
        }
    }

    {
        bdmf_boolean trace_fifo_overrun = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_is_trace_fifo_overrun_get(rnr_id, &trace_fifo_overrun);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_is_trace_fifo_overrun_get(%u %u)\n", rnr_id,
                trace_fifo_overrun);
        }
    }

    {
        rnr_regs_trace_config trace_config = {.trace_wraparound = gtmv(m, 1), .trace_mode = gtmv(m, 1), .trace_disable_idle_in = gtmv(m, 1), .trace_disable_wakeup_log = gtmv(m, 1), .trace_task = gtmv(m, 4), .idle_counter_source_sel = gtmv(m, 1), .counters_selected_task_mode = gtmv(m, 1), .counters_task = gtmv(m, 4), .profiling_window_mode = gtmv(m, 1), .single_mode_start_option = gtmv(m, 3), .single_mode_stop_option = gtmv(m, 3), .window_manual_start = gtmv(m, 1), .window_manual_stop = gtmv(m, 1), .tracer_enable = gtmv(m, 1), .profiling_window_reset = gtmv(m, 1), .profiling_window_enable = gtmv(m, 1), .trace_reset_event_fifo = gtmv(m, 1), .trace_clear_fifo_overrun = gtmv(m, 1), .trigger_on_second = gtmv(m, 1), .pc_start = gtmv(m, 13), .pc_stop_or_cycle_count = gtmv(m, 18)};
        bdmf_session_print(session, "ag_drv_rnr_regs_trace_config_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            trace_config.trace_wraparound, trace_config.trace_mode, trace_config.trace_disable_idle_in, trace_config.trace_disable_wakeup_log, 
            trace_config.trace_task, trace_config.idle_counter_source_sel, trace_config.counters_selected_task_mode, trace_config.counters_task, 
            trace_config.profiling_window_mode, trace_config.single_mode_start_option, trace_config.single_mode_stop_option, trace_config.window_manual_start, 
            trace_config.window_manual_stop, trace_config.tracer_enable, trace_config.profiling_window_reset, trace_config.profiling_window_enable, 
            trace_config.trace_reset_event_fifo, trace_config.trace_clear_fifo_overrun, trace_config.trigger_on_second, trace_config.pc_start, 
            trace_config.pc_stop_or_cycle_count);
        ag_err = ag_drv_rnr_regs_trace_config_set(rnr_id, &trace_config);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_trace_config_get(rnr_id, &trace_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_trace_config_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                trace_config.trace_wraparound, trace_config.trace_mode, trace_config.trace_disable_idle_in, trace_config.trace_disable_wakeup_log, 
                trace_config.trace_task, trace_config.idle_counter_source_sel, trace_config.counters_selected_task_mode, trace_config.counters_task, 
                trace_config.profiling_window_mode, trace_config.single_mode_start_option, trace_config.single_mode_stop_option, trace_config.window_manual_start, 
                trace_config.window_manual_stop, trace_config.tracer_enable, trace_config.profiling_window_reset, trace_config.profiling_window_enable, 
                trace_config.trace_reset_event_fifo, trace_config.trace_clear_fifo_overrun, trace_config.trigger_on_second, trace_config.pc_start, 
                trace_config.pc_stop_or_cycle_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (trace_config.trace_wraparound != gtmv(m, 1) || trace_config.trace_mode != gtmv(m, 1) || trace_config.trace_disable_idle_in != gtmv(m, 1) || trace_config.trace_disable_wakeup_log != gtmv(m, 1) || trace_config.trace_task != gtmv(m, 4) || trace_config.idle_counter_source_sel != gtmv(m, 1) || trace_config.counters_selected_task_mode != gtmv(m, 1) || trace_config.counters_task != gtmv(m, 4) || trace_config.profiling_window_mode != gtmv(m, 1) || trace_config.single_mode_start_option != gtmv(m, 3) || trace_config.single_mode_stop_option != gtmv(m, 3) || trace_config.window_manual_start != gtmv(m, 1) || trace_config.window_manual_stop != gtmv(m, 1) || trace_config.tracer_enable != gtmv(m, 1) || trace_config.profiling_window_reset != gtmv(m, 1) || trace_config.profiling_window_enable != gtmv(m, 1) || trace_config.trace_reset_event_fifo != gtmv(m, 1) || trace_config.trace_clear_fifo_overrun != gtmv(m, 1) || trace_config.trigger_on_second != gtmv(m, 1) || trace_config.pc_start != gtmv(m, 13) || trace_config.pc_stop_or_cycle_count != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean trace_reset_event_fifo = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_regs_reset_trace_fifo_set(%u %u)\n", rnr_id,
            trace_reset_event_fifo);
        ag_err = ag_drv_rnr_regs_reset_trace_fifo_set(rnr_id, trace_reset_event_fifo);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_reset_trace_fifo_get(rnr_id, &trace_reset_event_fifo);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_reset_trace_fifo_get(%u %u)\n", rnr_id,
                trace_reset_event_fifo);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (trace_reset_event_fifo != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean trace_clear_fifo_overrun = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_regs_clear_trace_fifo_overrun_set(%u %u)\n", rnr_id,
            trace_clear_fifo_overrun);
        ag_err = ag_drv_rnr_regs_clear_trace_fifo_overrun_set(rnr_id, trace_clear_fifo_overrun);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_clear_trace_fifo_overrun_get(rnr_id, &trace_clear_fifo_overrun);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_clear_trace_fifo_overrun_get(%u %u)\n", rnr_id,
                trace_clear_fifo_overrun);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (trace_clear_fifo_overrun != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_rnr_core_cntrs rnr_core_cntrs = {.total_stall_cnt = gtmv(m, 24), .stall_on_alu_b_full_cnt = gtmv(m, 16), .stall_on_alu_a_full_cnt = gtmv(m, 16), .stall_on_jmpreg = gtmv(m, 16), .stall_on_memio_full_cnt = gtmv(m, 16), .stall_on_waw_cnt = gtmv(m, 16), .stall_on_super_cmd = gtmv(m, 16), .stall_on_super_cmd_when_full = gtmv(m, 16), .stall_on_cs_cnt = gtmv(m, 16), .active_cycles_cnt = gtmv(m, 32), .stall_on_jmp_full_cnt = gtmv(m, 16), .stall_on_skip_jmp_cnt = gtmv(m, 16), .exec_counter = gtmv(m, 32), .idle_cnt = gtmv(m, 32), .jmp_taken_predicted_untaken_cnt = gtmv(m, 16), .jmp_untaken_predicted_taken_cnt = gtmv(m, 16)};
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_rnr_core_cntrs_get(rnr_id, &rnr_core_cntrs);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_rnr_core_cntrs_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                rnr_core_cntrs.total_stall_cnt, rnr_core_cntrs.stall_on_alu_b_full_cnt, rnr_core_cntrs.stall_on_alu_a_full_cnt, rnr_core_cntrs.stall_on_jmpreg, 
                rnr_core_cntrs.stall_on_memio_full_cnt, rnr_core_cntrs.stall_on_waw_cnt, rnr_core_cntrs.stall_on_super_cmd, rnr_core_cntrs.stall_on_super_cmd_when_full, 
                rnr_core_cntrs.stall_on_cs_cnt, rnr_core_cntrs.active_cycles_cnt, rnr_core_cntrs.stall_on_jmp_full_cnt, rnr_core_cntrs.stall_on_skip_jmp_cnt, 
                rnr_core_cntrs.exec_counter, rnr_core_cntrs.idle_cnt, rnr_core_cntrs.jmp_taken_predicted_untaken_cnt, rnr_core_cntrs.jmp_untaken_predicted_taken_cnt);
        }
    }

    {
        uint8_t thread_num = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_cpu_wakeup_set(%u %u)\n", rnr_id,
            thread_num);
        ag_err = ag_drv_rnr_regs_cfg_cpu_wakeup_set(rnr_id, thread_num);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_cpu_wakeup_get(rnr_id, &thread_num);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_cpu_wakeup_get(%u %u)\n", rnr_id,
                thread_num);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thread_num != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl = {.int0_sts = gtmv(m, 8), .int1_sts = gtmv(m, 8), .int2_sts = gtmv(m, 1), .int3_sts = gtmv(m, 1), .int4_sts = gtmv(m, 1), .int5_sts = gtmv(m, 1), .int6_sts = gtmv(m, 1), .int7_sts = gtmv(m, 1), .int8_sts = gtmv(m, 1), .int9_sts = gtmv(m, 1), .fit_fail_sts = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_ctrl_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            cfg_int_ctrl.int0_sts, cfg_int_ctrl.int1_sts, cfg_int_ctrl.int2_sts, cfg_int_ctrl.int3_sts, 
            cfg_int_ctrl.int4_sts, cfg_int_ctrl.int5_sts, cfg_int_ctrl.int6_sts, cfg_int_ctrl.int7_sts, 
            cfg_int_ctrl.int8_sts, cfg_int_ctrl.int9_sts, cfg_int_ctrl.fit_fail_sts);
        ag_err = ag_drv_rnr_regs_cfg_int_ctrl_set(rnr_id, &cfg_int_ctrl);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_int_ctrl_get(rnr_id, &cfg_int_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_ctrl_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                cfg_int_ctrl.int0_sts, cfg_int_ctrl.int1_sts, cfg_int_ctrl.int2_sts, cfg_int_ctrl.int3_sts, 
                cfg_int_ctrl.int4_sts, cfg_int_ctrl.int5_sts, cfg_int_ctrl.int6_sts, cfg_int_ctrl.int7_sts, 
                cfg_int_ctrl.int8_sts, cfg_int_ctrl.int9_sts, cfg_int_ctrl.fit_fail_sts);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_int_ctrl.int0_sts != gtmv(m, 8) || cfg_int_ctrl.int1_sts != gtmv(m, 8) || cfg_int_ctrl.int2_sts != gtmv(m, 1) || cfg_int_ctrl.int3_sts != gtmv(m, 1) || cfg_int_ctrl.int4_sts != gtmv(m, 1) || cfg_int_ctrl.int5_sts != gtmv(m, 1) || cfg_int_ctrl.int6_sts != gtmv(m, 1) || cfg_int_ctrl.int7_sts != gtmv(m, 1) || cfg_int_ctrl.int8_sts != gtmv(m, 1) || cfg_int_ctrl.int9_sts != gtmv(m, 1) || cfg_int_ctrl.fit_fail_sts != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_cfg_int_mask cfg_int_mask = {.int0_mask = gtmv(m, 8), .int1_mask = gtmv(m, 8), .int2_mask = gtmv(m, 1), .int3_mask = gtmv(m, 1), .int4_mask = gtmv(m, 1), .int5_mask = gtmv(m, 1), .int6_mask = gtmv(m, 1), .int7_mask = gtmv(m, 1), .int8_mask = gtmv(m, 1), .int9_mask = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_mask_set(%u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            cfg_int_mask.int0_mask, cfg_int_mask.int1_mask, cfg_int_mask.int2_mask, cfg_int_mask.int3_mask, 
            cfg_int_mask.int4_mask, cfg_int_mask.int5_mask, cfg_int_mask.int6_mask, cfg_int_mask.int7_mask, 
            cfg_int_mask.int8_mask, cfg_int_mask.int9_mask);
        ag_err = ag_drv_rnr_regs_cfg_int_mask_set(rnr_id, &cfg_int_mask);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_int_mask_get(rnr_id, &cfg_int_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_mask_get(%u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                cfg_int_mask.int0_mask, cfg_int_mask.int1_mask, cfg_int_mask.int2_mask, cfg_int_mask.int3_mask, 
                cfg_int_mask.int4_mask, cfg_int_mask.int5_mask, cfg_int_mask.int6_mask, cfg_int_mask.int7_mask, 
                cfg_int_mask.int8_mask, cfg_int_mask.int9_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_int_mask.int0_mask != gtmv(m, 8) || cfg_int_mask.int1_mask != gtmv(m, 8) || cfg_int_mask.int2_mask != gtmv(m, 1) || cfg_int_mask.int3_mask != gtmv(m, 1) || cfg_int_mask.int4_mask != gtmv(m, 1) || cfg_int_mask.int5_mask != gtmv(m, 1) || cfg_int_mask.int6_mask != gtmv(m, 1) || cfg_int_mask.int7_mask != gtmv(m, 1) || cfg_int_mask.int8_mask != gtmv(m, 1) || cfg_int_mask.int9_mask != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_cfg_gen_cfg cfg_gen_cfg = {.disable_dma_old_flow_control = gtmv(m, 1), .test_fit_fail = gtmv(m, 1), .zero_data_mem = gtmv(m, 1), .zero_context_mem = gtmv(m, 1), .zero_data_mem_done = gtmv(m, 1), .zero_context_mem_done = gtmv(m, 1), .chicken_disable_skip_jmp = gtmv(m, 1), .chicken_disable_alu_load_balancing = gtmv(m, 1), .gdma_desc_offset = gtmv(m, 8), .bbtx_tcam_dest_sel = gtmv(m, 1), .bbtx_hash_dest_sel = gtmv(m, 1), .bbtx_natc_dest_sel = gtmv(m, 1), .bbtx_cnpl_dest_sel = gtmv(m, 1), .gdma_gdesc_buffer_size = gtmv(m, 1), .chicken_enable_old_unique_id_mode = gtmv(m, 1), .chicken_enable_dma_old_mode = gtmv(m, 1), .prevent_cs_till_stores_done = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_gen_cfg_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            cfg_gen_cfg.disable_dma_old_flow_control, cfg_gen_cfg.test_fit_fail, cfg_gen_cfg.zero_data_mem, cfg_gen_cfg.zero_context_mem, 
            cfg_gen_cfg.zero_data_mem_done, cfg_gen_cfg.zero_context_mem_done, cfg_gen_cfg.chicken_disable_skip_jmp, cfg_gen_cfg.chicken_disable_alu_load_balancing, 
            cfg_gen_cfg.gdma_desc_offset, cfg_gen_cfg.bbtx_tcam_dest_sel, cfg_gen_cfg.bbtx_hash_dest_sel, cfg_gen_cfg.bbtx_natc_dest_sel, 
            cfg_gen_cfg.bbtx_cnpl_dest_sel, cfg_gen_cfg.gdma_gdesc_buffer_size, cfg_gen_cfg.chicken_enable_old_unique_id_mode, cfg_gen_cfg.chicken_enable_dma_old_mode, 
            cfg_gen_cfg.prevent_cs_till_stores_done);
        ag_err = ag_drv_rnr_regs_cfg_gen_cfg_set(rnr_id, &cfg_gen_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_gen_cfg_get(rnr_id, &cfg_gen_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_gen_cfg_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                cfg_gen_cfg.disable_dma_old_flow_control, cfg_gen_cfg.test_fit_fail, cfg_gen_cfg.zero_data_mem, cfg_gen_cfg.zero_context_mem, 
                cfg_gen_cfg.zero_data_mem_done, cfg_gen_cfg.zero_context_mem_done, cfg_gen_cfg.chicken_disable_skip_jmp, cfg_gen_cfg.chicken_disable_alu_load_balancing, 
                cfg_gen_cfg.gdma_desc_offset, cfg_gen_cfg.bbtx_tcam_dest_sel, cfg_gen_cfg.bbtx_hash_dest_sel, cfg_gen_cfg.bbtx_natc_dest_sel, 
                cfg_gen_cfg.bbtx_cnpl_dest_sel, cfg_gen_cfg.gdma_gdesc_buffer_size, cfg_gen_cfg.chicken_enable_old_unique_id_mode, cfg_gen_cfg.chicken_enable_dma_old_mode, 
                cfg_gen_cfg.prevent_cs_till_stores_done);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_gen_cfg.disable_dma_old_flow_control != gtmv(m, 1) || cfg_gen_cfg.test_fit_fail != gtmv(m, 1) || cfg_gen_cfg.zero_data_mem != gtmv(m, 1) || cfg_gen_cfg.zero_context_mem != gtmv(m, 1) || cfg_gen_cfg.zero_data_mem_done != gtmv(m, 1) || cfg_gen_cfg.zero_context_mem_done != gtmv(m, 1) || cfg_gen_cfg.chicken_disable_skip_jmp != gtmv(m, 1) || cfg_gen_cfg.chicken_disable_alu_load_balancing != gtmv(m, 1) || cfg_gen_cfg.gdma_desc_offset != gtmv(m, 8) || cfg_gen_cfg.bbtx_tcam_dest_sel != gtmv(m, 1) || cfg_gen_cfg.bbtx_hash_dest_sel != gtmv(m, 1) || cfg_gen_cfg.bbtx_natc_dest_sel != gtmv(m, 1) || cfg_gen_cfg.bbtx_cnpl_dest_sel != gtmv(m, 1) || cfg_gen_cfg.gdma_gdesc_buffer_size != gtmv(m, 1) || cfg_gen_cfg.chicken_enable_old_unique_id_mode != gtmv(m, 1) || cfg_gen_cfg.chicken_enable_dma_old_mode != gtmv(m, 1) || cfg_gen_cfg.prevent_cs_till_stores_done != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t dma_base = gtmv(m, 20);
        uint8_t dma_static_offset = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_fpm_mini_cfg_set(%u %u %u)\n", rnr_id,
            dma_base, dma_static_offset);
        ag_err = ag_drv_rnr_regs_cfg_fpm_mini_cfg_set(rnr_id, dma_base, dma_static_offset);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_fpm_mini_cfg_get(rnr_id, &dma_base, &dma_static_offset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_fpm_mini_cfg_get(%u %u %u)\n", rnr_id,
                dma_base, dma_static_offset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dma_base != gtmv(m, 20) || dma_static_offset != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t dma_base = gtmv(m, 20);
        uint8_t dma_buf_size = gtmv(m, 3);
        bdmf_boolean dma_buf_size_mode = gtmv(m, 1);
        uint8_t dma_static_offset = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ddr_cfg_set(%u %u %u %u %u)\n", rnr_id,
            dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        ag_err = ag_drv_rnr_regs_cfg_ddr_cfg_set(rnr_id, dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_ddr_cfg_get(rnr_id, &dma_base, &dma_buf_size, &dma_buf_size_mode, &dma_static_offset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ddr_cfg_get(%u %u %u %u %u)\n", rnr_id,
                dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dma_base != gtmv(m, 20) || dma_buf_size != gtmv(m, 3) || dma_buf_size_mode != gtmv(m, 1) || dma_static_offset != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t dma_base = gtmv(m, 20);
        uint8_t dma_buf_size = gtmv(m, 3);
        bdmf_boolean dma_buf_size_mode = gtmv(m, 1);
        uint8_t dma_static_offset = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_psram_cfg_set(%u %u %u %u %u)\n", rnr_id,
            dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        ag_err = ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_id, dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_psram_cfg_get(rnr_id, &dma_base, &dma_buf_size, &dma_buf_size_mode, &dma_static_offset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_psram_cfg_get(%u %u %u %u %u)\n", rnr_id,
                dma_base, dma_buf_size, dma_buf_size_mode, dma_static_offset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dma_base != gtmv(m, 20) || dma_buf_size != gtmv(m, 3) || dma_buf_size_mode != gtmv(m, 1) || dma_static_offset != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t mask0 = gtmv(m, 16);
        uint16_t mask1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(%u %u %u)\n", rnr_id,
            mask0, mask1);
        ag_err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(rnr_id, mask0, mask1);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(rnr_id, &mask0, &mask1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(%u %u %u)\n", rnr_id,
                mask0, mask1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask0 != gtmv(m, 16) || mask1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t scheduler_mode = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_sch_cfg_set(%u %u)\n", rnr_id,
            scheduler_mode);
        ag_err = ag_drv_rnr_regs_cfg_sch_cfg_set(rnr_id, scheduler_mode);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_sch_cfg_get(rnr_id, &scheduler_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_sch_cfg_get(%u %u)\n", rnr_id,
                scheduler_mode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (scheduler_mode != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg = {.bkpt_0_en = gtmv(m, 1), .bkpt_0_use_thread = gtmv(m, 1), .bkpt_1_en = gtmv(m, 1), .bkpt_1_use_thread = gtmv(m, 1), .bkpt_2_en = gtmv(m, 1), .bkpt_2_use_thread = gtmv(m, 1), .bkpt_3_en = gtmv(m, 1), .bkpt_3_use_thread = gtmv(m, 1), .bkpt_4_en = gtmv(m, 1), .bkpt_4_use_thread = gtmv(m, 1), .bkpt_5_en = gtmv(m, 1), .bkpt_5_use_thread = gtmv(m, 1), .bkpt_6_en = gtmv(m, 1), .bkpt_6_use_thread = gtmv(m, 1), .bkpt_7_en = gtmv(m, 1), .bkpt_7_use_thread = gtmv(m, 1), .step_mode = gtmv(m, 1), .new_flags_val = gtmv(m, 4), .enable_breakpoint_on_fit_fail = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_cfg_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_use_thread, 
            cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_use_thread, 
            cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_use_thread, 
            cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_use_thread, 
            cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.new_flags_val, cfg_bkpt_cfg.enable_breakpoint_on_fit_fail);
        ag_err = ag_drv_rnr_regs_cfg_bkpt_cfg_set(rnr_id, &cfg_bkpt_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_bkpt_cfg_get(rnr_id, &cfg_bkpt_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_cfg_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_use_thread, 
                cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_use_thread, 
                cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_use_thread, 
                cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_use_thread, 
                cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.new_flags_val, cfg_bkpt_cfg.enable_breakpoint_on_fit_fail);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_bkpt_cfg.bkpt_0_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_0_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_1_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_1_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_2_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_2_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_3_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_3_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_4_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_4_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_5_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_5_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_6_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_6_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_7_en != gtmv(m, 1) || cfg_bkpt_cfg.bkpt_7_use_thread != gtmv(m, 1) || cfg_bkpt_cfg.step_mode != gtmv(m, 1) || cfg_bkpt_cfg.new_flags_val != gtmv(m, 4) || cfg_bkpt_cfg.enable_breakpoint_on_fit_fail != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_imm_set(%u %u)\n", rnr_id,
            enable);
        ag_err = ag_drv_rnr_regs_cfg_bkpt_imm_set(rnr_id, enable);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_bkpt_imm_get(rnr_id, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_imm_get(%u %u)\n", rnr_id,
                enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t bkpt_addr = gtmv(m, 13);
        bdmf_boolean active = gtmv(m, 1);
        uint16_t data_bkpt_addr = gtmv(m, 16);
        uint8_t bkpt_reason = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_bkpt_sts_get(rnr_id, &bkpt_addr, &active, &data_bkpt_addr, &bkpt_reason);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_sts_get(%u %u %u %u %u)\n", rnr_id,
                bkpt_addr, active, data_bkpt_addr, bkpt_reason);
        }
    }

    {
        uint16_t current_pc_addr = gtmv(m, 13);
        uint16_t pc_ret = gtmv(m, 13);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_pc_sts_get(rnr_id, &current_pc_addr, &pc_ret);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_pc_sts_get(%u %u %u)\n", rnr_id,
                current_pc_addr, pc_ret);
        }
    }

    {
        uint16_t addr_base = gtmv(m, 13);
        uint8_t addr_step_0 = gtmv(m, 4);
        uint8_t addr_step_1 = gtmv(m, 4);
        uint8_t start_thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ext_acc_cfg_set(%u %u %u %u %u)\n", rnr_id,
            addr_base, addr_step_0, addr_step_1, start_thread);
        ag_err = ag_drv_rnr_regs_cfg_ext_acc_cfg_set(rnr_id, addr_base, addr_step_0, addr_step_1, start_thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_ext_acc_cfg_get(rnr_id, &addr_base, &addr_step_0, &addr_step_1, &start_thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ext_acc_cfg_get(%u %u %u %u %u)\n", rnr_id,
                addr_base, addr_step_0, addr_step_1, start_thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr_base != gtmv(m, 13) || addr_step_0 != gtmv(m, 4) || addr_step_1 != gtmv(m, 4) || start_thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t start_addr = gtmv(m, 13);
        uint16_t stop_addr = gtmv(m, 13);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_fit_fail_cfg_set(%u %u %u)\n", rnr_id,
            start_addr, stop_addr);
        ag_err = ag_drv_rnr_regs_cfg_fit_fail_cfg_set(rnr_id, start_addr, stop_addr);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_fit_fail_cfg_get(rnr_id, &start_addr, &stop_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_fit_fail_cfg_get(%u %u %u)\n", rnr_id,
                start_addr, stop_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start_addr != gtmv(m, 13) || stop_addr != gtmv(m, 13))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_regs_cfg_data_bkpt_cfg cfg_data_bkpt_cfg = {.bkpt_0_en = gtmv(m, 1), .bkpt_0_use_thread = gtmv(m, 1), .bkpt_1_en = gtmv(m, 1), .bkpt_1_use_thread = gtmv(m, 1), .bkpt_2_en = gtmv(m, 1), .bkpt_2_use_thread = gtmv(m, 1), .bkpt_3_en = gtmv(m, 1), .bkpt_3_use_thread = gtmv(m, 1), .reset_data_bkpt = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_data_bkpt_cfg_set(%u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
            cfg_data_bkpt_cfg.bkpt_0_en, cfg_data_bkpt_cfg.bkpt_0_use_thread, cfg_data_bkpt_cfg.bkpt_1_en, cfg_data_bkpt_cfg.bkpt_1_use_thread, 
            cfg_data_bkpt_cfg.bkpt_2_en, cfg_data_bkpt_cfg.bkpt_2_use_thread, cfg_data_bkpt_cfg.bkpt_3_en, cfg_data_bkpt_cfg.bkpt_3_use_thread, 
            cfg_data_bkpt_cfg.reset_data_bkpt);
        ag_err = ag_drv_rnr_regs_cfg_data_bkpt_cfg_set(rnr_id, &cfg_data_bkpt_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_data_bkpt_cfg_get(rnr_id, &cfg_data_bkpt_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_data_bkpt_cfg_get(%u %u %u %u %u %u %u %u %u %u)\n", rnr_id,
                cfg_data_bkpt_cfg.bkpt_0_en, cfg_data_bkpt_cfg.bkpt_0_use_thread, cfg_data_bkpt_cfg.bkpt_1_en, cfg_data_bkpt_cfg.bkpt_1_use_thread, 
                cfg_data_bkpt_cfg.bkpt_2_en, cfg_data_bkpt_cfg.bkpt_2_use_thread, cfg_data_bkpt_cfg.bkpt_3_en, cfg_data_bkpt_cfg.bkpt_3_use_thread, 
                cfg_data_bkpt_cfg.reset_data_bkpt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_data_bkpt_cfg.bkpt_0_en != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_0_use_thread != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_1_en != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_1_use_thread != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_2_en != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_2_use_thread != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_3_en != gtmv(m, 1) || cfg_data_bkpt_cfg.bkpt_3_use_thread != gtmv(m, 1) || cfg_data_bkpt_cfg.reset_data_bkpt != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t aqm_counter_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_aqm_counter_val_get(rnr_id, &aqm_counter_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_aqm_counter_val_get(%u %u)\n", rnr_id,
                aqm_counter_value);
        }
    }

    {
        uint16_t trace_base_addr = gtmv(m, 13);
        uint16_t trace_max_addr = gtmv(m, 13);
        bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_cfg_0_set(%u %u %u)\n", rnr_id,
            trace_base_addr, trace_max_addr);
        ag_err = ag_drv_rnr_regs_cfg_profiling_cfg_0_set(rnr_id, trace_base_addr, trace_max_addr);
        if (!ag_err)
            ag_err = ag_drv_rnr_regs_cfg_profiling_cfg_0_get(rnr_id, &trace_base_addr, &trace_max_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_cfg_0_get(%u %u %u)\n", rnr_id,
                trace_base_addr, trace_max_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (trace_base_addr != gtmv(m, 13) || trace_max_addr != gtmv(m, 13))
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
            ag_err = ag_drv_rnr_regs_cfg_profiling_counter_get(rnr_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_counter_get(%u %u)\n", rnr_id,
                val);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_rnr_regs_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cfg_global_ctrl: reg = &RU_REG(RNR_REGS, CFG_GLOBAL_CTRL); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_cpu_wakeup: reg = &RU_REG(RNR_REGS, CFG_CPU_WAKEUP); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_int_ctrl: reg = &RU_REG(RNR_REGS, CFG_INT_CTRL); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_int_mask: reg = &RU_REG(RNR_REGS, CFG_INT_MASK); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_gen_cfg: reg = &RU_REG(RNR_REGS, CFG_GEN_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_cam_cfg: reg = &RU_REG(RNR_REGS, CFG_CAM_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_fpm_mini_cfg: reg = &RU_REG(RNR_REGS, CFG_FPM_MINI_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_ddr_cfg: reg = &RU_REG(RNR_REGS, CFG_DDR_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_psram_cfg: reg = &RU_REG(RNR_REGS, CFG_PSRAM_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_ramrd_range_mask_cfg: reg = &RU_REG(RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_sch_cfg: reg = &RU_REG(RNR_REGS, CFG_SCH_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_cfg: reg = &RU_REG(RNR_REGS, CFG_BKPT_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_imm: reg = &RU_REG(RNR_REGS, CFG_BKPT_IMM); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_sts: reg = &RU_REG(RNR_REGS, CFG_BKPT_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_pc_sts: reg = &RU_REG(RNR_REGS, CFG_PC_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_ext_acc_cfg: reg = &RU_REG(RNR_REGS, CFG_EXT_ACC_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_fit_fail_cfg: reg = &RU_REG(RNR_REGS, CFG_FIT_FAIL_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_data_bkpt_cfg: reg = &RU_REG(RNR_REGS, CFG_DATA_BKPT_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_aqm_counter_val: reg = &RU_REG(RNR_REGS, CFG_AQM_COUNTER_VAL); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt1: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt2: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT2); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt3: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT3); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt4: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT4); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt5: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT5); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt6: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT6); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt7: reg = &RU_REG(RNR_REGS, CFG_STALL_CNT7); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_sts: reg = &RU_REG(RNR_REGS, CFG_PROFILING_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_cfg_0: reg = &RU_REG(RNR_REGS, CFG_PROFILING_CFG_0); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_cfg_1: reg = &RU_REG(RNR_REGS, CFG_PROFILING_CFG_1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_counter: reg = &RU_REG(RNR_REGS, CFG_PROFILING_COUNTER); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_cfg_2: reg = &RU_REG(RNR_REGS, CFG_PROFILING_CFG_2); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_exec_cmds_cnt: reg = &RU_REG(RNR_REGS, CFG_EXEC_CMDS_CNT); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_idle_cnt1: reg = &RU_REG(RNR_REGS, CFG_IDLE_CNT1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_jmp_cnt: reg = &RU_REG(RNR_REGS, CFG_JMP_CNT); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_metal_fix_reg: reg = &RU_REG(RNR_REGS, CFG_METAL_FIX_REG); blk = &RU_BLK(RNR_REGS); break;
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

bdmfmon_handle_t ag_drv_rnr_regs_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "rnr_regs", "rnr_regs", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_rnr_enable[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_freq[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("micro_sec_val", "micro_sec_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_stop_val[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("stop_value", "stop_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_trace_config[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_wraparound", "trace_wraparound", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_mode", "trace_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_disable_idle_in", "trace_disable_idle_in", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_disable_wakeup_log", "trace_disable_wakeup_log", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_task", "trace_task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idle_counter_source_sel", "idle_counter_source_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counters_selected_task_mode", "counters_selected_task_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counters_task", "counters_task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profiling_window_mode", "profiling_window_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("single_mode_start_option", "single_mode_start_option", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("single_mode_stop_option", "single_mode_stop_option", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("window_manual_start", "window_manual_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("window_manual_stop", "window_manual_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tracer_enable", "tracer_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profiling_window_reset", "profiling_window_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profiling_window_enable", "profiling_window_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_reset_event_fifo", "trace_reset_event_fifo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_clear_fifo_overrun", "trace_clear_fifo_overrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trigger_on_second", "trigger_on_second", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pc_start", "pc_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pc_stop_or_cycle_count", "pc_stop_or_cycle_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_trace_fifo[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_reset_event_fifo", "trace_reset_event_fifo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clear_trace_fifo_overrun[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_clear_fifo_overrun", "trace_clear_fifo_overrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_cpu_wakeup[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread_num", "thread_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_int_ctrl[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int0_sts", "int0_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int1_sts", "int1_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int2_sts", "int2_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int3_sts", "int3_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int4_sts", "int4_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int5_sts", "int5_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int6_sts", "int6_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int7_sts", "int7_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int8_sts", "int8_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int9_sts", "int9_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fit_fail_sts", "fit_fail_sts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_int_mask[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int0_mask", "int0_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int1_mask", "int1_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int2_mask", "int2_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int3_mask", "int3_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int4_mask", "int4_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int5_mask", "int5_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int6_mask", "int6_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int7_mask", "int7_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int8_mask", "int8_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("int9_mask", "int9_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_gen_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_dma_old_flow_control", "disable_dma_old_flow_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("test_fit_fail", "test_fit_fail", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero_data_mem", "zero_data_mem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero_context_mem", "zero_context_mem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero_data_mem_done", "zero_data_mem_done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero_context_mem_done", "zero_context_mem_done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_disable_skip_jmp", "chicken_disable_skip_jmp", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_disable_alu_load_balancing", "chicken_disable_alu_load_balancing", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gdma_desc_offset", "gdma_desc_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbtx_tcam_dest_sel", "bbtx_tcam_dest_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbtx_hash_dest_sel", "bbtx_hash_dest_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbtx_natc_dest_sel", "bbtx_natc_dest_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbtx_cnpl_dest_sel", "bbtx_cnpl_dest_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gdma_gdesc_buffer_size", "gdma_gdesc_buffer_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_enable_old_unique_id_mode", "chicken_enable_old_unique_id_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_enable_dma_old_mode", "chicken_enable_dma_old_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("prevent_cs_till_stores_done", "prevent_cs_till_stores_done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_fpm_mini_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_base", "dma_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_static_offset", "dma_static_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ddr_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_base", "dma_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size", "dma_buf_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size_mode", "dma_buf_size_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_static_offset", "dma_static_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_psram_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_base", "dma_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size", "dma_buf_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size_mode", "dma_buf_size_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dma_static_offset", "dma_static_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ramrd_range_mask_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask0", "mask0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask1", "mask1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_sch_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("scheduler_mode", "scheduler_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_bkpt_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_0_en", "bkpt_0_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_0_use_thread", "bkpt_0_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_en", "bkpt_1_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_use_thread", "bkpt_1_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_en", "bkpt_2_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_use_thread", "bkpt_2_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_en", "bkpt_3_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_use_thread", "bkpt_3_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_4_en", "bkpt_4_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_4_use_thread", "bkpt_4_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_5_en", "bkpt_5_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_5_use_thread", "bkpt_5_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_6_en", "bkpt_6_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_6_use_thread", "bkpt_6_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_7_en", "bkpt_7_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_7_use_thread", "bkpt_7_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("step_mode", "step_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("new_flags_val", "new_flags_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_breakpoint_on_fit_fail", "enable_breakpoint_on_fit_fail", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_bkpt_imm[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ext_acc_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr_base", "addr_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr_step_0", "addr_step_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr_step_1", "addr_step_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_thread", "start_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_fit_fail_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_addr", "start_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("stop_addr", "stop_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_data_bkpt_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_0_en", "bkpt_0_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_0_use_thread", "bkpt_0_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_en", "bkpt_1_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_use_thread", "bkpt_1_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_en", "bkpt_2_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_use_thread", "bkpt_2_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_en", "bkpt_3_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_use_thread", "bkpt_3_use_thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reset_data_bkpt", "reset_data_bkpt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_profiling_cfg_0[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_base_addr", "trace_base_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("trace_max_addr", "trace_max_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "rnr_enable", .val = cli_rnr_regs_rnr_enable, .parms = set_rnr_enable },
            { .name = "rnr_freq", .val = cli_rnr_regs_rnr_freq, .parms = set_rnr_freq },
            { .name = "cam_stop_val", .val = cli_rnr_regs_cam_stop_val, .parms = set_cam_stop_val },
            { .name = "trace_config", .val = cli_rnr_regs_trace_config, .parms = set_trace_config },
            { .name = "reset_trace_fifo", .val = cli_rnr_regs_reset_trace_fifo, .parms = set_reset_trace_fifo },
            { .name = "clear_trace_fifo_overrun", .val = cli_rnr_regs_clear_trace_fifo_overrun, .parms = set_clear_trace_fifo_overrun },
            { .name = "cfg_cpu_wakeup", .val = cli_rnr_regs_cfg_cpu_wakeup, .parms = set_cfg_cpu_wakeup },
            { .name = "cfg_int_ctrl", .val = cli_rnr_regs_cfg_int_ctrl, .parms = set_cfg_int_ctrl },
            { .name = "cfg_int_mask", .val = cli_rnr_regs_cfg_int_mask, .parms = set_cfg_int_mask },
            { .name = "cfg_gen_cfg", .val = cli_rnr_regs_cfg_gen_cfg, .parms = set_cfg_gen_cfg },
            { .name = "cfg_fpm_mini_cfg", .val = cli_rnr_regs_cfg_fpm_mini_cfg, .parms = set_cfg_fpm_mini_cfg },
            { .name = "cfg_ddr_cfg", .val = cli_rnr_regs_cfg_ddr_cfg, .parms = set_cfg_ddr_cfg },
            { .name = "cfg_psram_cfg", .val = cli_rnr_regs_cfg_psram_cfg, .parms = set_cfg_psram_cfg },
            { .name = "cfg_ramrd_range_mask_cfg", .val = cli_rnr_regs_cfg_ramrd_range_mask_cfg, .parms = set_cfg_ramrd_range_mask_cfg },
            { .name = "cfg_sch_cfg", .val = cli_rnr_regs_cfg_sch_cfg, .parms = set_cfg_sch_cfg },
            { .name = "cfg_bkpt_cfg", .val = cli_rnr_regs_cfg_bkpt_cfg, .parms = set_cfg_bkpt_cfg },
            { .name = "cfg_bkpt_imm", .val = cli_rnr_regs_cfg_bkpt_imm, .parms = set_cfg_bkpt_imm },
            { .name = "cfg_ext_acc_cfg", .val = cli_rnr_regs_cfg_ext_acc_cfg, .parms = set_cfg_ext_acc_cfg },
            { .name = "cfg_fit_fail_cfg", .val = cli_rnr_regs_cfg_fit_fail_cfg, .parms = set_cfg_fit_fail_cfg },
            { .name = "cfg_data_bkpt_cfg", .val = cli_rnr_regs_cfg_data_bkpt_cfg, .parms = set_cfg_data_bkpt_cfg },
            { .name = "cfg_profiling_cfg_0", .val = cli_rnr_regs_cfg_profiling_cfg_0, .parms = set_cfg_profiling_cfg_0 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_rnr_regs_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "rnr_enable", .val = cli_rnr_regs_rnr_enable, .parms = get_default },
            { .name = "dma_illegal", .val = cli_rnr_regs_dma_illegal, .parms = get_default },
            { .name = "prediction_overrun", .val = cli_rnr_regs_prediction_overrun, .parms = get_default },
            { .name = "rnr_freq", .val = cli_rnr_regs_rnr_freq, .parms = get_default },
            { .name = "cam_stop_val", .val = cli_rnr_regs_cam_stop_val, .parms = get_default },
            { .name = "profiling_sts", .val = cli_rnr_regs_profiling_sts, .parms = get_default },
            { .name = "is_trace_fifo_overrun", .val = cli_rnr_regs_is_trace_fifo_overrun, .parms = get_default },
            { .name = "trace_config", .val = cli_rnr_regs_trace_config, .parms = get_default },
            { .name = "reset_trace_fifo", .val = cli_rnr_regs_reset_trace_fifo, .parms = get_default },
            { .name = "clear_trace_fifo_overrun", .val = cli_rnr_regs_clear_trace_fifo_overrun, .parms = get_default },
            { .name = "rnr_core_cntrs", .val = cli_rnr_regs_rnr_core_cntrs, .parms = get_default },
            { .name = "cfg_cpu_wakeup", .val = cli_rnr_regs_cfg_cpu_wakeup, .parms = get_default },
            { .name = "cfg_int_ctrl", .val = cli_rnr_regs_cfg_int_ctrl, .parms = get_default },
            { .name = "cfg_int_mask", .val = cli_rnr_regs_cfg_int_mask, .parms = get_default },
            { .name = "cfg_gen_cfg", .val = cli_rnr_regs_cfg_gen_cfg, .parms = get_default },
            { .name = "cfg_fpm_mini_cfg", .val = cli_rnr_regs_cfg_fpm_mini_cfg, .parms = get_default },
            { .name = "cfg_ddr_cfg", .val = cli_rnr_regs_cfg_ddr_cfg, .parms = get_default },
            { .name = "cfg_psram_cfg", .val = cli_rnr_regs_cfg_psram_cfg, .parms = get_default },
            { .name = "cfg_ramrd_range_mask_cfg", .val = cli_rnr_regs_cfg_ramrd_range_mask_cfg, .parms = get_default },
            { .name = "cfg_sch_cfg", .val = cli_rnr_regs_cfg_sch_cfg, .parms = get_default },
            { .name = "cfg_bkpt_cfg", .val = cli_rnr_regs_cfg_bkpt_cfg, .parms = get_default },
            { .name = "cfg_bkpt_imm", .val = cli_rnr_regs_cfg_bkpt_imm, .parms = get_default },
            { .name = "cfg_bkpt_sts", .val = cli_rnr_regs_cfg_bkpt_sts, .parms = get_default },
            { .name = "cfg_pc_sts", .val = cli_rnr_regs_cfg_pc_sts, .parms = get_default },
            { .name = "cfg_ext_acc_cfg", .val = cli_rnr_regs_cfg_ext_acc_cfg, .parms = get_default },
            { .name = "cfg_fit_fail_cfg", .val = cli_rnr_regs_cfg_fit_fail_cfg, .parms = get_default },
            { .name = "cfg_data_bkpt_cfg", .val = cli_rnr_regs_cfg_data_bkpt_cfg, .parms = get_default },
            { .name = "cfg_aqm_counter_val", .val = cli_rnr_regs_cfg_aqm_counter_val, .parms = get_default },
            { .name = "cfg_profiling_cfg_0", .val = cli_rnr_regs_cfg_profiling_cfg_0, .parms = get_default },
            { .name = "cfg_profiling_counter", .val = cli_rnr_regs_cfg_profiling_counter, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_rnr_regs_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_rnr_regs_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("rnr_id", "rnr_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CFG_GLOBAL_CTRL", .val = bdmf_address_cfg_global_ctrl },
            { .name = "CFG_CPU_WAKEUP", .val = bdmf_address_cfg_cpu_wakeup },
            { .name = "CFG_INT_CTRL", .val = bdmf_address_cfg_int_ctrl },
            { .name = "CFG_INT_MASK", .val = bdmf_address_cfg_int_mask },
            { .name = "CFG_GEN_CFG", .val = bdmf_address_cfg_gen_cfg },
            { .name = "CFG_CAM_CFG", .val = bdmf_address_cfg_cam_cfg },
            { .name = "CFG_FPM_MINI_CFG", .val = bdmf_address_cfg_fpm_mini_cfg },
            { .name = "CFG_DDR_CFG", .val = bdmf_address_cfg_ddr_cfg },
            { .name = "CFG_PSRAM_CFG", .val = bdmf_address_cfg_psram_cfg },
            { .name = "CFG_RAMRD_RANGE_MASK_CFG", .val = bdmf_address_cfg_ramrd_range_mask_cfg },
            { .name = "CFG_SCH_CFG", .val = bdmf_address_cfg_sch_cfg },
            { .name = "CFG_BKPT_CFG", .val = bdmf_address_cfg_bkpt_cfg },
            { .name = "CFG_BKPT_IMM", .val = bdmf_address_cfg_bkpt_imm },
            { .name = "CFG_BKPT_STS", .val = bdmf_address_cfg_bkpt_sts },
            { .name = "CFG_PC_STS", .val = bdmf_address_cfg_pc_sts },
            { .name = "CFG_EXT_ACC_CFG", .val = bdmf_address_cfg_ext_acc_cfg },
            { .name = "CFG_FIT_FAIL_CFG", .val = bdmf_address_cfg_fit_fail_cfg },
            { .name = "CFG_DATA_BKPT_CFG", .val = bdmf_address_cfg_data_bkpt_cfg },
            { .name = "CFG_AQM_COUNTER_VAL", .val = bdmf_address_cfg_aqm_counter_val },
            { .name = "CFG_STALL_CNT1", .val = bdmf_address_cfg_stall_cnt1 },
            { .name = "CFG_STALL_CNT2", .val = bdmf_address_cfg_stall_cnt2 },
            { .name = "CFG_STALL_CNT3", .val = bdmf_address_cfg_stall_cnt3 },
            { .name = "CFG_STALL_CNT4", .val = bdmf_address_cfg_stall_cnt4 },
            { .name = "CFG_STALL_CNT5", .val = bdmf_address_cfg_stall_cnt5 },
            { .name = "CFG_STALL_CNT6", .val = bdmf_address_cfg_stall_cnt6 },
            { .name = "CFG_STALL_CNT7", .val = bdmf_address_cfg_stall_cnt7 },
            { .name = "CFG_PROFILING_STS", .val = bdmf_address_cfg_profiling_sts },
            { .name = "CFG_PROFILING_CFG_0", .val = bdmf_address_cfg_profiling_cfg_0 },
            { .name = "CFG_PROFILING_CFG_1", .val = bdmf_address_cfg_profiling_cfg_1 },
            { .name = "CFG_PROFILING_COUNTER", .val = bdmf_address_cfg_profiling_counter },
            { .name = "CFG_PROFILING_CFG_2", .val = bdmf_address_cfg_profiling_cfg_2 },
            { .name = "CFG_EXEC_CMDS_CNT", .val = bdmf_address_cfg_exec_cmds_cnt },
            { .name = "CFG_IDLE_CNT1", .val = bdmf_address_cfg_idle_cnt1 },
            { .name = "CFG_JMP_CNT", .val = bdmf_address_cfg_jmp_cnt },
            { .name = "CFG_METAL_FIX_REG", .val = bdmf_address_cfg_metal_fix_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_rnr_regs_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "rnr_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
