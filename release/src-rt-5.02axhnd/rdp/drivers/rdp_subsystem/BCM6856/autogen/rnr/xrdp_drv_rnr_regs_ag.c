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
#include "xrdp_drv_rnr_regs_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_rnr_regs_rnr_enable_set(uint8_t rnr_id, bdmf_boolean en)
{
    uint32_t reg_cfg_global_ctrl=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *en = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, EN, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_dma_illegal_set(uint8_t rnr_id, bdmf_boolean dma_illegal_status)
{
    uint32_t reg_cfg_global_ctrl=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_illegal_status >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    reg_cfg_global_ctrl = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, DMA_ILLEGAL_STATUS, reg_cfg_global_ctrl, dma_illegal_status);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_dma_illegal_get(uint8_t rnr_id, bdmf_boolean *dma_illegal_status)
{
    uint32_t reg_cfg_global_ctrl;

#ifdef VALIDATE_PARMS
    if(!dma_illegal_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, reg_cfg_global_ctrl);

    *dma_illegal_status = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GLOBAL_CTRL, DMA_ILLEGAL_STATUS, reg_cfg_global_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_rnr_freq_set(uint8_t rnr_id, uint16_t micro_sec_val)
{
    uint32_t reg_cfg_global_ctrl=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    if(!micro_sec_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_cam_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    if(!stop_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    if(!profiling_sts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_is_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_fifo_overrun)
{
    uint32_t reg_cfg_profiling_sts;

#ifdef VALIDATE_PARMS
    if(!trace_fifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_profiling_cfg_1=0;

#ifdef VALIDATE_PARMS
    if(!trace_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (trace_config->trace_wraparound >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_mode >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_disable_idle_in >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_disable_wakeup_log >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_task >= _4BITS_MAX_VAL_) ||
       (trace_config->idle_counter_source_sel >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_reset_event_fifo >= _1BITS_MAX_VAL_) ||
       (trace_config->trace_clear_fifo_overrun >= _1BITS_MAX_VAL_))
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
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1, trace_config->trace_reset_event_fifo);
    reg_cfg_profiling_cfg_1 = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1, trace_config->trace_clear_fifo_overrun);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_trace_config_get(uint8_t rnr_id, rnr_regs_trace_config *trace_config)
{
    uint32_t reg_cfg_profiling_cfg_1;

#ifdef VALIDATE_PARMS
    if(!trace_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, reg_cfg_profiling_cfg_1);

    trace_config->trace_wraparound = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_WRAPAROUND, reg_cfg_profiling_cfg_1);
    trace_config->trace_mode = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_MODE, reg_cfg_profiling_cfg_1);
    trace_config->trace_disable_idle_in = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_IDLE_IN, reg_cfg_profiling_cfg_1);
    trace_config->trace_disable_wakeup_log = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_DISABLE_WAKEUP_LOG, reg_cfg_profiling_cfg_1);
    trace_config->trace_task = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_TASK, reg_cfg_profiling_cfg_1);
    trace_config->idle_counter_source_sel = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, IDLE_COUNTER_SOURCE_SEL, reg_cfg_profiling_cfg_1);
    trace_config->trace_reset_event_fifo = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_RESET_EVENT_FIFO, reg_cfg_profiling_cfg_1);
    trace_config->trace_clear_fifo_overrun = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PROFILING_CFG_1, TRACE_CLEAR_FIFO_OVERRUN, reg_cfg_profiling_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_set(uint8_t rnr_id, bdmf_boolean trace_reset_event_fifo)
{
    uint32_t reg_cfg_profiling_cfg_1=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!trace_reset_event_fifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_profiling_cfg_1=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!trace_clear_fifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_idle_cnt1;
    uint32_t reg_cfg_jmp_cnt;

#ifdef VALIDATE_PARMS
    if(!rnr_core_cntrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT1, reg_cfg_stall_cnt1);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_STALL_CNT2, reg_cfg_stall_cnt2);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_IDLE_CNT1, reg_cfg_idle_cnt1);
    RU_REG_READ(rnr_id, RNR_REGS, CFG_JMP_CNT, reg_cfg_jmp_cnt);

    rnr_core_cntrs->ld_stall_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT1, LD_STALL_CNT, reg_cfg_stall_cnt1);
    rnr_core_cntrs->acc_stall_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT1, ACC_STALL_CNT, reg_cfg_stall_cnt1);
    rnr_core_cntrs->ldio_stall_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT2, LDIO_STALL_CNT, reg_cfg_stall_cnt2);
    rnr_core_cntrs->store_stall_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_STALL_CNT2, STORE_STALL_CNT, reg_cfg_stall_cnt2);
    rnr_core_cntrs->idle_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_IDLE_CNT1, IDLE_CNT, reg_cfg_idle_cnt1);
    rnr_core_cntrs->jmp_taken_predicted_untaken_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_JMP_CNT, UNTAKEN_JMP_CNT, reg_cfg_jmp_cnt);
    rnr_core_cntrs->jmp_untaken_predicted_taken_cnt = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_JMP_CNT, TAKEN_JMP_CNT, reg_cfg_jmp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num)
{
    uint32_t reg_cfg_cpu_wakeup=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!thread_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_int_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!cfg_int_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!cfg_int_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!cfg_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!cfg_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_set(uint8_t rnr_id, bdmf_boolean disable_dma_old_flow_control, bdmf_boolean test_fit_fail)
{
    uint32_t reg_cfg_gen_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (disable_dma_old_flow_control >= _1BITS_MAX_VAL_) ||
       (test_fit_fail >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, DISABLE_DMA_OLD_FLOW_CONTROL, reg_cfg_gen_cfg, disable_dma_old_flow_control);
    reg_cfg_gen_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_GEN_CFG, TEST_FIT_FAIL, reg_cfg_gen_cfg, test_fit_fail);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_GEN_CFG, reg_cfg_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_get(uint8_t rnr_id, bdmf_boolean *disable_dma_old_flow_control, bdmf_boolean *test_fit_fail)
{
    uint32_t reg_cfg_gen_cfg;

#ifdef VALIDATE_PARMS
    if(!disable_dma_old_flow_control || !test_fit_fail)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_GEN_CFG, reg_cfg_gen_cfg);

    *disable_dma_old_flow_control = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, DISABLE_DMA_OLD_FLOW_CONTROL, reg_cfg_gen_cfg);
    *test_fit_fail = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_GEN_CFG, TEST_FIT_FAIL, reg_cfg_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, uint8_t dma_static_offset)
{
    uint32_t reg_cfg_ddr_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_base >= _20BITS_MAX_VAL_) ||
       (dma_buf_size >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BASE, reg_cfg_ddr_cfg, dma_base);
    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE, reg_cfg_ddr_cfg, dma_buf_size);
    reg_cfg_ddr_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_STATIC_OFFSET, reg_cfg_ddr_cfg, dma_static_offset);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_DDR_CFG, reg_cfg_ddr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, uint8_t *dma_static_offset)
{
    uint32_t reg_cfg_ddr_cfg;

#ifdef VALIDATE_PARMS
    if(!dma_base || !dma_buf_size || !dma_static_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_DDR_CFG, reg_cfg_ddr_cfg);

    *dma_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BASE, reg_cfg_ddr_cfg);
    *dma_buf_size = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_BUF_SIZE, reg_cfg_ddr_cfg);
    *dma_static_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_DDR_CFG, DMA_STATIC_OFFSET, reg_cfg_ddr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, uint8_t dma_static_offset)
{
    uint32_t reg_cfg_psram_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (dma_base >= _20BITS_MAX_VAL_) ||
       (dma_buf_size >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BASE, reg_cfg_psram_cfg, dma_base);
    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE, reg_cfg_psram_cfg, dma_buf_size);
    reg_cfg_psram_cfg = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_STATIC_OFFSET, reg_cfg_psram_cfg, dma_static_offset);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_PSRAM_CFG, reg_cfg_psram_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, uint8_t *dma_static_offset)
{
    uint32_t reg_cfg_psram_cfg;

#ifdef VALIDATE_PARMS
    if(!dma_base || !dma_buf_size || !dma_static_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_PSRAM_CFG, reg_cfg_psram_cfg);

    *dma_base = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BASE, reg_cfg_psram_cfg);
    *dma_buf_size = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_BUF_SIZE, reg_cfg_psram_cfg);
    *dma_static_offset = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_PSRAM_CFG, DMA_STATIC_OFFSET, reg_cfg_psram_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(uint8_t rnr_id, uint16_t mask0, uint16_t mask1)
{
    uint32_t reg_cfg_ramrd_range_mask_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    if(!mask0 || !mask1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_sch_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!scheduler_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_cfg_bkpt_cfg=0;

#ifdef VALIDATE_PARMS
    if(!cfg_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
       (cfg_bkpt_cfg->new_flags_val >= _4BITS_MAX_VAL_))
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

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_BKPT_CFG, reg_cfg_bkpt_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg)
{
    uint32_t reg_cfg_bkpt_cfg;

#ifdef VALIDATE_PARMS
    if(!cfg_bkpt_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_set(uint8_t rnr_id, bdmf_boolean enable)
{
    uint32_t reg_cfg_bkpt_imm=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_BKPT_IMM, reg_cfg_bkpt_imm);

    *enable = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_IMM, ENABLE, reg_cfg_bkpt_imm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_sts_get(uint8_t rnr_id, uint16_t *bkpt_addr, bdmf_boolean *active)
{
    uint32_t reg_cfg_bkpt_sts;

#ifdef VALIDATE_PARMS
    if(!bkpt_addr || !active)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(rnr_id, RNR_REGS, CFG_BKPT_STS, reg_cfg_bkpt_sts);

    *bkpt_addr = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, BKPT_ADDR, reg_cfg_bkpt_sts);
    *active = RU_FIELD_GET(rnr_id, RNR_REGS, CFG_BKPT_STS, ACTIVE, reg_cfg_bkpt_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_regs_cfg_pc_sts_get(uint8_t rnr_id, uint16_t *current_pc_addr, uint16_t *pc_ret)
{
    uint32_t reg_cfg_pc_sts;

#ifdef VALIDATE_PARMS
    if(!current_pc_addr || !pc_ret)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_set(uint8_t rnr_id, uint16_t trace_base_addr, uint16_t trace_max_addr)
{
    uint32_t reg_cfg_profiling_cfg_0=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
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
    if(!trace_base_addr || !trace_max_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_id >= BLOCK_ADDR_COUNT))
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
    bdmf_address_cfg_ddr_cfg,
    bdmf_address_cfg_psram_cfg,
    bdmf_address_cfg_ramrd_range_mask_cfg,
    bdmf_address_cfg_sch_cfg,
    bdmf_address_cfg_bkpt_cfg,
    bdmf_address_cfg_bkpt_imm,
    bdmf_address_cfg_bkpt_sts,
    bdmf_address_cfg_pc_sts,
    bdmf_address_cfg_profiling_sts,
    bdmf_address_cfg_profiling_cfg_0,
    bdmf_address_cfg_profiling_cfg_1,
    bdmf_address_cfg_profiling_counter,
    bdmf_address_cfg_stall_cnt1,
    bdmf_address_cfg_stall_cnt2,
    bdmf_address_cfg_idle_cnt1,
    bdmf_address_cfg_jmp_cnt,
}
bdmf_address;

static int bcm_rnr_regs_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_rnr_regs_rnr_enable:
        err = ag_drv_rnr_regs_rnr_enable_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_dma_illegal:
        err = ag_drv_rnr_regs_dma_illegal_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_rnr_freq:
        err = ag_drv_rnr_regs_rnr_freq_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cam_stop_val:
        err = ag_drv_rnr_regs_cam_stop_val_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_trace_config:
    {
        rnr_regs_trace_config trace_config = { .trace_wraparound=parm[2].value.unumber, .trace_mode=parm[3].value.unumber, .trace_disable_idle_in=parm[4].value.unumber, .trace_disable_wakeup_log=parm[5].value.unumber, .trace_task=parm[6].value.unumber, .idle_counter_source_sel=parm[7].value.unumber, .trace_reset_event_fifo=parm[8].value.unumber, .trace_clear_fifo_overrun=parm[9].value.unumber};
        err = ag_drv_rnr_regs_trace_config_set(parm[1].value.unumber, &trace_config);
        break;
    }
    case cli_rnr_regs_reset_trace_fifo:
        err = ag_drv_rnr_regs_reset_trace_fifo_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_clear_trace_fifo_overrun:
        err = ag_drv_rnr_regs_clear_trace_fifo_overrun_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_cpu_wakeup:
        err = ag_drv_rnr_regs_cfg_cpu_wakeup_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_int_ctrl:
    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl = { .int0_sts=parm[2].value.unumber, .int1_sts=parm[3].value.unumber, .int2_sts=parm[4].value.unumber, .int3_sts=parm[5].value.unumber, .int4_sts=parm[6].value.unumber, .int5_sts=parm[7].value.unumber, .int6_sts=parm[8].value.unumber, .int7_sts=parm[9].value.unumber, .int8_sts=parm[10].value.unumber, .int9_sts=parm[11].value.unumber, .fit_fail_sts=parm[12].value.unumber};
        err = ag_drv_rnr_regs_cfg_int_ctrl_set(parm[1].value.unumber, &cfg_int_ctrl);
        break;
    }
    case cli_rnr_regs_cfg_int_mask:
    {
        rnr_regs_cfg_int_mask cfg_int_mask = { .int0_mask=parm[2].value.unumber, .int1_mask=parm[3].value.unumber, .int2_mask=parm[4].value.unumber, .int3_mask=parm[5].value.unumber, .int4_mask=parm[6].value.unumber, .int5_mask=parm[7].value.unumber, .int6_mask=parm[8].value.unumber, .int7_mask=parm[9].value.unumber, .int8_mask=parm[10].value.unumber, .int9_mask=parm[11].value.unumber};
        err = ag_drv_rnr_regs_cfg_int_mask_set(parm[1].value.unumber, &cfg_int_mask);
        break;
    }
    case cli_rnr_regs_cfg_gen_cfg:
        err = ag_drv_rnr_regs_cfg_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_regs_cfg_ddr_cfg:
        err = ag_drv_rnr_regs_cfg_ddr_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_rnr_regs_cfg_psram_cfg:
        err = ag_drv_rnr_regs_cfg_psram_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_rnr_regs_cfg_ramrd_range_mask_cfg:
        err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_regs_cfg_sch_cfg:
        err = ag_drv_rnr_regs_cfg_sch_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_bkpt_cfg:
    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg = { .bkpt_0_en=parm[2].value.unumber, .bkpt_0_use_thread=parm[3].value.unumber, .bkpt_1_en=parm[4].value.unumber, .bkpt_1_use_thread=parm[5].value.unumber, .bkpt_2_en=parm[6].value.unumber, .bkpt_2_use_thread=parm[7].value.unumber, .bkpt_3_en=parm[8].value.unumber, .bkpt_3_use_thread=parm[9].value.unumber, .bkpt_4_en=parm[10].value.unumber, .bkpt_4_use_thread=parm[11].value.unumber, .bkpt_5_en=parm[12].value.unumber, .bkpt_5_use_thread=parm[13].value.unumber, .bkpt_6_en=parm[14].value.unumber, .bkpt_6_use_thread=parm[15].value.unumber, .bkpt_7_en=parm[16].value.unumber, .bkpt_7_use_thread=parm[17].value.unumber, .step_mode=parm[18].value.unumber, .new_flags_val=parm[19].value.unumber};
        err = ag_drv_rnr_regs_cfg_bkpt_cfg_set(parm[1].value.unumber, &cfg_bkpt_cfg);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_imm:
        err = ag_drv_rnr_regs_cfg_bkpt_imm_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_regs_cfg_profiling_cfg_0:
        err = ag_drv_rnr_regs_cfg_profiling_cfg_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_rnr_regs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_rnr_regs_rnr_enable:
    {
        bdmf_boolean en;
        err = ag_drv_rnr_regs_rnr_enable_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_rnr_regs_dma_illegal:
    {
        bdmf_boolean dma_illegal_status;
        err = ag_drv_rnr_regs_dma_illegal_get(parm[1].value.unumber, &dma_illegal_status);
        bdmf_session_print(session, "dma_illegal_status = %u (0x%x)\n", dma_illegal_status, dma_illegal_status);
        break;
    }
    case cli_rnr_regs_rnr_freq:
    {
        uint16_t micro_sec_val;
        err = ag_drv_rnr_regs_rnr_freq_get(parm[1].value.unumber, &micro_sec_val);
        bdmf_session_print(session, "micro_sec_val = %u (0x%x)\n", micro_sec_val, micro_sec_val);
        break;
    }
    case cli_rnr_regs_cam_stop_val:
    {
        uint16_t stop_value;
        err = ag_drv_rnr_regs_cam_stop_val_get(parm[1].value.unumber, &stop_value);
        bdmf_session_print(session, "stop_value = %u (0x%x)\n", stop_value, stop_value);
        break;
    }
    case cli_rnr_regs_profiling_sts:
    {
        rnr_regs_profiling_sts profiling_sts;
        err = ag_drv_rnr_regs_profiling_sts_get(parm[1].value.unumber, &profiling_sts);
        bdmf_session_print(session, "trace_write_pnt = %u (0x%x)\n", profiling_sts.trace_write_pnt, profiling_sts.trace_write_pnt);
        bdmf_session_print(session, "idle_no_active_task = %u (0x%x)\n", profiling_sts.idle_no_active_task, profiling_sts.idle_no_active_task);
        bdmf_session_print(session, "curr_thread_num = %u (0x%x)\n", profiling_sts.curr_thread_num, profiling_sts.curr_thread_num);
        bdmf_session_print(session, "profiling_active = %u (0x%x)\n", profiling_sts.profiling_active, profiling_sts.profiling_active);
        bdmf_session_print(session, "trace_fifo_overrun = %u (0x%x)\n", profiling_sts.trace_fifo_overrun, profiling_sts.trace_fifo_overrun);
        break;
    }
    case cli_rnr_regs_is_trace_fifo_overrun:
    {
        bdmf_boolean trace_fifo_overrun;
        err = ag_drv_rnr_regs_is_trace_fifo_overrun_get(parm[1].value.unumber, &trace_fifo_overrun);
        bdmf_session_print(session, "trace_fifo_overrun = %u (0x%x)\n", trace_fifo_overrun, trace_fifo_overrun);
        break;
    }
    case cli_rnr_regs_trace_config:
    {
        rnr_regs_trace_config trace_config;
        err = ag_drv_rnr_regs_trace_config_get(parm[1].value.unumber, &trace_config);
        bdmf_session_print(session, "trace_wraparound = %u (0x%x)\n", trace_config.trace_wraparound, trace_config.trace_wraparound);
        bdmf_session_print(session, "trace_mode = %u (0x%x)\n", trace_config.trace_mode, trace_config.trace_mode);
        bdmf_session_print(session, "trace_disable_idle_in = %u (0x%x)\n", trace_config.trace_disable_idle_in, trace_config.trace_disable_idle_in);
        bdmf_session_print(session, "trace_disable_wakeup_log = %u (0x%x)\n", trace_config.trace_disable_wakeup_log, trace_config.trace_disable_wakeup_log);
        bdmf_session_print(session, "trace_task = %u (0x%x)\n", trace_config.trace_task, trace_config.trace_task);
        bdmf_session_print(session, "idle_counter_source_sel = %u (0x%x)\n", trace_config.idle_counter_source_sel, trace_config.idle_counter_source_sel);
        bdmf_session_print(session, "trace_reset_event_fifo = %u (0x%x)\n", trace_config.trace_reset_event_fifo, trace_config.trace_reset_event_fifo);
        bdmf_session_print(session, "trace_clear_fifo_overrun = %u (0x%x)\n", trace_config.trace_clear_fifo_overrun, trace_config.trace_clear_fifo_overrun);
        break;
    }
    case cli_rnr_regs_reset_trace_fifo:
    {
        bdmf_boolean trace_reset_event_fifo;
        err = ag_drv_rnr_regs_reset_trace_fifo_get(parm[1].value.unumber, &trace_reset_event_fifo);
        bdmf_session_print(session, "trace_reset_event_fifo = %u (0x%x)\n", trace_reset_event_fifo, trace_reset_event_fifo);
        break;
    }
    case cli_rnr_regs_clear_trace_fifo_overrun:
    {
        bdmf_boolean trace_clear_fifo_overrun;
        err = ag_drv_rnr_regs_clear_trace_fifo_overrun_get(parm[1].value.unumber, &trace_clear_fifo_overrun);
        bdmf_session_print(session, "trace_clear_fifo_overrun = %u (0x%x)\n", trace_clear_fifo_overrun, trace_clear_fifo_overrun);
        break;
    }
    case cli_rnr_regs_rnr_core_cntrs:
    {
        rnr_regs_rnr_core_cntrs rnr_core_cntrs;
        err = ag_drv_rnr_regs_rnr_core_cntrs_get(parm[1].value.unumber, &rnr_core_cntrs);
        bdmf_session_print(session, "ld_stall_cnt = %u (0x%x)\n", rnr_core_cntrs.ld_stall_cnt, rnr_core_cntrs.ld_stall_cnt);
        bdmf_session_print(session, "acc_stall_cnt = %u (0x%x)\n", rnr_core_cntrs.acc_stall_cnt, rnr_core_cntrs.acc_stall_cnt);
        bdmf_session_print(session, "ldio_stall_cnt = %u (0x%x)\n", rnr_core_cntrs.ldio_stall_cnt, rnr_core_cntrs.ldio_stall_cnt);
        bdmf_session_print(session, "store_stall_cnt = %u (0x%x)\n", rnr_core_cntrs.store_stall_cnt, rnr_core_cntrs.store_stall_cnt);
        bdmf_session_print(session, "idle_cnt = %u (0x%x)\n", rnr_core_cntrs.idle_cnt, rnr_core_cntrs.idle_cnt);
        bdmf_session_print(session, "jmp_taken_predicted_untaken_cnt = %u (0x%x)\n", rnr_core_cntrs.jmp_taken_predicted_untaken_cnt, rnr_core_cntrs.jmp_taken_predicted_untaken_cnt);
        bdmf_session_print(session, "jmp_untaken_predicted_taken_cnt = %u (0x%x)\n", rnr_core_cntrs.jmp_untaken_predicted_taken_cnt, rnr_core_cntrs.jmp_untaken_predicted_taken_cnt);
        break;
    }
    case cli_rnr_regs_cfg_cpu_wakeup:
    {
        uint8_t thread_num;
        err = ag_drv_rnr_regs_cfg_cpu_wakeup_get(parm[1].value.unumber, &thread_num);
        bdmf_session_print(session, "thread_num = %u (0x%x)\n", thread_num, thread_num);
        break;
    }
    case cli_rnr_regs_cfg_int_ctrl:
    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl;
        err = ag_drv_rnr_regs_cfg_int_ctrl_get(parm[1].value.unumber, &cfg_int_ctrl);
        bdmf_session_print(session, "int0_sts = %u (0x%x)\n", cfg_int_ctrl.int0_sts, cfg_int_ctrl.int0_sts);
        bdmf_session_print(session, "int1_sts = %u (0x%x)\n", cfg_int_ctrl.int1_sts, cfg_int_ctrl.int1_sts);
        bdmf_session_print(session, "int2_sts = %u (0x%x)\n", cfg_int_ctrl.int2_sts, cfg_int_ctrl.int2_sts);
        bdmf_session_print(session, "int3_sts = %u (0x%x)\n", cfg_int_ctrl.int3_sts, cfg_int_ctrl.int3_sts);
        bdmf_session_print(session, "int4_sts = %u (0x%x)\n", cfg_int_ctrl.int4_sts, cfg_int_ctrl.int4_sts);
        bdmf_session_print(session, "int5_sts = %u (0x%x)\n", cfg_int_ctrl.int5_sts, cfg_int_ctrl.int5_sts);
        bdmf_session_print(session, "int6_sts = %u (0x%x)\n", cfg_int_ctrl.int6_sts, cfg_int_ctrl.int6_sts);
        bdmf_session_print(session, "int7_sts = %u (0x%x)\n", cfg_int_ctrl.int7_sts, cfg_int_ctrl.int7_sts);
        bdmf_session_print(session, "int8_sts = %u (0x%x)\n", cfg_int_ctrl.int8_sts, cfg_int_ctrl.int8_sts);
        bdmf_session_print(session, "int9_sts = %u (0x%x)\n", cfg_int_ctrl.int9_sts, cfg_int_ctrl.int9_sts);
        bdmf_session_print(session, "fit_fail_sts = %u (0x%x)\n", cfg_int_ctrl.fit_fail_sts, cfg_int_ctrl.fit_fail_sts);
        break;
    }
    case cli_rnr_regs_cfg_int_mask:
    {
        rnr_regs_cfg_int_mask cfg_int_mask;
        err = ag_drv_rnr_regs_cfg_int_mask_get(parm[1].value.unumber, &cfg_int_mask);
        bdmf_session_print(session, "int0_mask = %u (0x%x)\n", cfg_int_mask.int0_mask, cfg_int_mask.int0_mask);
        bdmf_session_print(session, "int1_mask = %u (0x%x)\n", cfg_int_mask.int1_mask, cfg_int_mask.int1_mask);
        bdmf_session_print(session, "int2_mask = %u (0x%x)\n", cfg_int_mask.int2_mask, cfg_int_mask.int2_mask);
        bdmf_session_print(session, "int3_mask = %u (0x%x)\n", cfg_int_mask.int3_mask, cfg_int_mask.int3_mask);
        bdmf_session_print(session, "int4_mask = %u (0x%x)\n", cfg_int_mask.int4_mask, cfg_int_mask.int4_mask);
        bdmf_session_print(session, "int5_mask = %u (0x%x)\n", cfg_int_mask.int5_mask, cfg_int_mask.int5_mask);
        bdmf_session_print(session, "int6_mask = %u (0x%x)\n", cfg_int_mask.int6_mask, cfg_int_mask.int6_mask);
        bdmf_session_print(session, "int7_mask = %u (0x%x)\n", cfg_int_mask.int7_mask, cfg_int_mask.int7_mask);
        bdmf_session_print(session, "int8_mask = %u (0x%x)\n", cfg_int_mask.int8_mask, cfg_int_mask.int8_mask);
        bdmf_session_print(session, "int9_mask = %u (0x%x)\n", cfg_int_mask.int9_mask, cfg_int_mask.int9_mask);
        break;
    }
    case cli_rnr_regs_cfg_gen_cfg:
    {
        bdmf_boolean disable_dma_old_flow_control;
        bdmf_boolean test_fit_fail;
        err = ag_drv_rnr_regs_cfg_gen_cfg_get(parm[1].value.unumber, &disable_dma_old_flow_control, &test_fit_fail);
        bdmf_session_print(session, "disable_dma_old_flow_control = %u (0x%x)\n", disable_dma_old_flow_control, disable_dma_old_flow_control);
        bdmf_session_print(session, "test_fit_fail = %u (0x%x)\n", test_fit_fail, test_fit_fail);
        break;
    }
    case cli_rnr_regs_cfg_ddr_cfg:
    {
        uint32_t dma_base;
        uint8_t dma_buf_size;
        uint8_t dma_static_offset;
        err = ag_drv_rnr_regs_cfg_ddr_cfg_get(parm[1].value.unumber, &dma_base, &dma_buf_size, &dma_static_offset);
        bdmf_session_print(session, "dma_base = %u (0x%x)\n", dma_base, dma_base);
        bdmf_session_print(session, "dma_buf_size = %u (0x%x)\n", dma_buf_size, dma_buf_size);
        bdmf_session_print(session, "dma_static_offset = %u (0x%x)\n", dma_static_offset, dma_static_offset);
        break;
    }
    case cli_rnr_regs_cfg_psram_cfg:
    {
        uint32_t dma_base;
        uint8_t dma_buf_size;
        uint8_t dma_static_offset;
        err = ag_drv_rnr_regs_cfg_psram_cfg_get(parm[1].value.unumber, &dma_base, &dma_buf_size, &dma_static_offset);
        bdmf_session_print(session, "dma_base = %u (0x%x)\n", dma_base, dma_base);
        bdmf_session_print(session, "dma_buf_size = %u (0x%x)\n", dma_buf_size, dma_buf_size);
        bdmf_session_print(session, "dma_static_offset = %u (0x%x)\n", dma_static_offset, dma_static_offset);
        break;
    }
    case cli_rnr_regs_cfg_ramrd_range_mask_cfg:
    {
        uint16_t mask0;
        uint16_t mask1;
        err = ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(parm[1].value.unumber, &mask0, &mask1);
        bdmf_session_print(session, "mask0 = %u (0x%x)\n", mask0, mask0);
        bdmf_session_print(session, "mask1 = %u (0x%x)\n", mask1, mask1);
        break;
    }
    case cli_rnr_regs_cfg_sch_cfg:
    {
        uint8_t scheduler_mode;
        err = ag_drv_rnr_regs_cfg_sch_cfg_get(parm[1].value.unumber, &scheduler_mode);
        bdmf_session_print(session, "scheduler_mode = %u (0x%x)\n", scheduler_mode, scheduler_mode);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_cfg:
    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg;
        err = ag_drv_rnr_regs_cfg_bkpt_cfg_get(parm[1].value.unumber, &cfg_bkpt_cfg);
        bdmf_session_print(session, "bkpt_0_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_en);
        bdmf_session_print(session, "bkpt_0_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_0_use_thread);
        bdmf_session_print(session, "bkpt_1_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_en);
        bdmf_session_print(session, "bkpt_1_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_1_use_thread, cfg_bkpt_cfg.bkpt_1_use_thread);
        bdmf_session_print(session, "bkpt_2_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_en);
        bdmf_session_print(session, "bkpt_2_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_2_use_thread);
        bdmf_session_print(session, "bkpt_3_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_en);
        bdmf_session_print(session, "bkpt_3_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_3_use_thread, cfg_bkpt_cfg.bkpt_3_use_thread);
        bdmf_session_print(session, "bkpt_4_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_en);
        bdmf_session_print(session, "bkpt_4_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_4_use_thread);
        bdmf_session_print(session, "bkpt_5_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_en);
        bdmf_session_print(session, "bkpt_5_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_5_use_thread, cfg_bkpt_cfg.bkpt_5_use_thread);
        bdmf_session_print(session, "bkpt_6_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_en);
        bdmf_session_print(session, "bkpt_6_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_6_use_thread);
        bdmf_session_print(session, "bkpt_7_en = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_en);
        bdmf_session_print(session, "bkpt_7_use_thread = %u (0x%x)\n", cfg_bkpt_cfg.bkpt_7_use_thread, cfg_bkpt_cfg.bkpt_7_use_thread);
        bdmf_session_print(session, "step_mode = %u (0x%x)\n", cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.step_mode);
        bdmf_session_print(session, "new_flags_val = %u (0x%x)\n", cfg_bkpt_cfg.new_flags_val, cfg_bkpt_cfg.new_flags_val);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_imm:
    {
        bdmf_boolean enable;
        err = ag_drv_rnr_regs_cfg_bkpt_imm_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        break;
    }
    case cli_rnr_regs_cfg_bkpt_sts:
    {
        uint16_t bkpt_addr;
        bdmf_boolean active;
        err = ag_drv_rnr_regs_cfg_bkpt_sts_get(parm[1].value.unumber, &bkpt_addr, &active);
        bdmf_session_print(session, "bkpt_addr = %u (0x%x)\n", bkpt_addr, bkpt_addr);
        bdmf_session_print(session, "active = %u (0x%x)\n", active, active);
        break;
    }
    case cli_rnr_regs_cfg_pc_sts:
    {
        uint16_t current_pc_addr;
        uint16_t pc_ret;
        err = ag_drv_rnr_regs_cfg_pc_sts_get(parm[1].value.unumber, &current_pc_addr, &pc_ret);
        bdmf_session_print(session, "current_pc_addr = %u (0x%x)\n", current_pc_addr, current_pc_addr);
        bdmf_session_print(session, "pc_ret = %u (0x%x)\n", pc_ret, pc_ret);
        break;
    }
    case cli_rnr_regs_cfg_profiling_cfg_0:
    {
        uint16_t trace_base_addr;
        uint16_t trace_max_addr;
        err = ag_drv_rnr_regs_cfg_profiling_cfg_0_get(parm[1].value.unumber, &trace_base_addr, &trace_max_addr);
        bdmf_session_print(session, "trace_base_addr = %u (0x%x)\n", trace_base_addr, trace_base_addr);
        bdmf_session_print(session, "trace_max_addr = %u (0x%x)\n", trace_max_addr, trace_max_addr);
        break;
    }
    case cli_rnr_regs_cfg_profiling_counter:
    {
        uint32_t val;
        err = ag_drv_rnr_regs_cfg_profiling_counter_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_rnr_regs_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t rnr_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_rnr_enable_set(%u %u)\n", rnr_id, en);
        if(!err) ag_drv_rnr_regs_rnr_enable_set(rnr_id, en);
        if(!err) ag_drv_rnr_regs_rnr_enable_get( rnr_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_rnr_enable_get(%u %u)\n", rnr_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean dma_illegal_status=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_dma_illegal_set(%u %u)\n", rnr_id, dma_illegal_status);
        if(!err) ag_drv_rnr_regs_dma_illegal_set(rnr_id, dma_illegal_status);
        if(!err) ag_drv_rnr_regs_dma_illegal_get( rnr_id, &dma_illegal_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_dma_illegal_get(%u %u)\n", rnr_id, dma_illegal_status);
        if(err || dma_illegal_status!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t micro_sec_val=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_rnr_freq_set(%u %u)\n", rnr_id, micro_sec_val);
        if(!err) ag_drv_rnr_regs_rnr_freq_set(rnr_id, micro_sec_val);
        if(!err) ag_drv_rnr_regs_rnr_freq_get( rnr_id, &micro_sec_val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_rnr_freq_get(%u %u)\n", rnr_id, micro_sec_val);
        if(err || micro_sec_val!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t stop_value=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cam_stop_val_set(%u %u)\n", rnr_id, stop_value);
        if(!err) ag_drv_rnr_regs_cam_stop_val_set(rnr_id, stop_value);
        if(!err) ag_drv_rnr_regs_cam_stop_val_get( rnr_id, &stop_value);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cam_stop_val_get(%u %u)\n", rnr_id, stop_value);
        if(err || stop_value!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_regs_profiling_sts profiling_sts = {.trace_write_pnt=gtmv(m, 13), .idle_no_active_task=gtmv(m, 1), .curr_thread_num=gtmv(m, 4), .profiling_active=gtmv(m, 1), .trace_fifo_overrun=gtmv(m, 1)};
        if(!err) ag_drv_rnr_regs_profiling_sts_get( rnr_id, &profiling_sts);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_profiling_sts_get(%u %u %u %u %u %u)\n", rnr_id, profiling_sts.trace_write_pnt, profiling_sts.idle_no_active_task, profiling_sts.curr_thread_num, profiling_sts.profiling_active, profiling_sts.trace_fifo_overrun);
    }
    {
        bdmf_boolean trace_fifo_overrun=gtmv(m, 1);
        if(!err) ag_drv_rnr_regs_is_trace_fifo_overrun_get( rnr_id, &trace_fifo_overrun);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_is_trace_fifo_overrun_get(%u %u)\n", rnr_id, trace_fifo_overrun);
    }
    {
        rnr_regs_trace_config trace_config = {.trace_wraparound=gtmv(m, 1), .trace_mode=gtmv(m, 1), .trace_disable_idle_in=gtmv(m, 1), .trace_disable_wakeup_log=gtmv(m, 1), .trace_task=gtmv(m, 4), .idle_counter_source_sel=gtmv(m, 1), .trace_reset_event_fifo=gtmv(m, 1), .trace_clear_fifo_overrun=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_trace_config_set(%u %u %u %u %u %u %u %u %u)\n", rnr_id, trace_config.trace_wraparound, trace_config.trace_mode, trace_config.trace_disable_idle_in, trace_config.trace_disable_wakeup_log, trace_config.trace_task, trace_config.idle_counter_source_sel, trace_config.trace_reset_event_fifo, trace_config.trace_clear_fifo_overrun);
        if(!err) ag_drv_rnr_regs_trace_config_set(rnr_id, &trace_config);
        if(!err) ag_drv_rnr_regs_trace_config_get( rnr_id, &trace_config);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_trace_config_get(%u %u %u %u %u %u %u %u %u)\n", rnr_id, trace_config.trace_wraparound, trace_config.trace_mode, trace_config.trace_disable_idle_in, trace_config.trace_disable_wakeup_log, trace_config.trace_task, trace_config.idle_counter_source_sel, trace_config.trace_reset_event_fifo, trace_config.trace_clear_fifo_overrun);
        if(err || trace_config.trace_wraparound!=gtmv(m, 1) || trace_config.trace_mode!=gtmv(m, 1) || trace_config.trace_disable_idle_in!=gtmv(m, 1) || trace_config.trace_disable_wakeup_log!=gtmv(m, 1) || trace_config.trace_task!=gtmv(m, 4) || trace_config.idle_counter_source_sel!=gtmv(m, 1) || trace_config.trace_reset_event_fifo!=gtmv(m, 1) || trace_config.trace_clear_fifo_overrun!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean trace_reset_event_fifo=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_reset_trace_fifo_set(%u %u)\n", rnr_id, trace_reset_event_fifo);
        if(!err) ag_drv_rnr_regs_reset_trace_fifo_set(rnr_id, trace_reset_event_fifo);
        if(!err) ag_drv_rnr_regs_reset_trace_fifo_get( rnr_id, &trace_reset_event_fifo);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_reset_trace_fifo_get(%u %u)\n", rnr_id, trace_reset_event_fifo);
        if(err || trace_reset_event_fifo!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean trace_clear_fifo_overrun=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_clear_trace_fifo_overrun_set(%u %u)\n", rnr_id, trace_clear_fifo_overrun);
        if(!err) ag_drv_rnr_regs_clear_trace_fifo_overrun_set(rnr_id, trace_clear_fifo_overrun);
        if(!err) ag_drv_rnr_regs_clear_trace_fifo_overrun_get( rnr_id, &trace_clear_fifo_overrun);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_clear_trace_fifo_overrun_get(%u %u)\n", rnr_id, trace_clear_fifo_overrun);
        if(err || trace_clear_fifo_overrun!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_regs_rnr_core_cntrs rnr_core_cntrs = {.ld_stall_cnt=gtmv(m, 16), .acc_stall_cnt=gtmv(m, 16), .ldio_stall_cnt=gtmv(m, 16), .store_stall_cnt=gtmv(m, 16), .idle_cnt=gtmv(m, 32), .jmp_taken_predicted_untaken_cnt=gtmv(m, 16), .jmp_untaken_predicted_taken_cnt=gtmv(m, 16)};
        if(!err) ag_drv_rnr_regs_rnr_core_cntrs_get( rnr_id, &rnr_core_cntrs);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_rnr_core_cntrs_get(%u %u %u %u %u %u %u %u)\n", rnr_id, rnr_core_cntrs.ld_stall_cnt, rnr_core_cntrs.acc_stall_cnt, rnr_core_cntrs.ldio_stall_cnt, rnr_core_cntrs.store_stall_cnt, rnr_core_cntrs.idle_cnt, rnr_core_cntrs.jmp_taken_predicted_untaken_cnt, rnr_core_cntrs.jmp_untaken_predicted_taken_cnt);
    }
    {
        uint8_t thread_num=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_cpu_wakeup_set(%u %u)\n", rnr_id, thread_num);
        if(!err) ag_drv_rnr_regs_cfg_cpu_wakeup_set(rnr_id, thread_num);
        if(!err) ag_drv_rnr_regs_cfg_cpu_wakeup_get( rnr_id, &thread_num);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_cpu_wakeup_get(%u %u)\n", rnr_id, thread_num);
        if(err || thread_num!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_regs_cfg_int_ctrl cfg_int_ctrl = {.int0_sts=gtmv(m, 8), .int1_sts=gtmv(m, 8), .int2_sts=gtmv(m, 1), .int3_sts=gtmv(m, 1), .int4_sts=gtmv(m, 1), .int5_sts=gtmv(m, 1), .int6_sts=gtmv(m, 1), .int7_sts=gtmv(m, 1), .int8_sts=gtmv(m, 1), .int9_sts=gtmv(m, 1), .fit_fail_sts=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_ctrl_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_int_ctrl.int0_sts, cfg_int_ctrl.int1_sts, cfg_int_ctrl.int2_sts, cfg_int_ctrl.int3_sts, cfg_int_ctrl.int4_sts, cfg_int_ctrl.int5_sts, cfg_int_ctrl.int6_sts, cfg_int_ctrl.int7_sts, cfg_int_ctrl.int8_sts, cfg_int_ctrl.int9_sts, cfg_int_ctrl.fit_fail_sts);
        if(!err) ag_drv_rnr_regs_cfg_int_ctrl_set(rnr_id, &cfg_int_ctrl);
        if(!err) ag_drv_rnr_regs_cfg_int_ctrl_get( rnr_id, &cfg_int_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_ctrl_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_int_ctrl.int0_sts, cfg_int_ctrl.int1_sts, cfg_int_ctrl.int2_sts, cfg_int_ctrl.int3_sts, cfg_int_ctrl.int4_sts, cfg_int_ctrl.int5_sts, cfg_int_ctrl.int6_sts, cfg_int_ctrl.int7_sts, cfg_int_ctrl.int8_sts, cfg_int_ctrl.int9_sts, cfg_int_ctrl.fit_fail_sts);
        if(err || cfg_int_ctrl.int0_sts!=gtmv(m, 8) || cfg_int_ctrl.int1_sts!=gtmv(m, 8) || cfg_int_ctrl.int2_sts!=gtmv(m, 1) || cfg_int_ctrl.int3_sts!=gtmv(m, 1) || cfg_int_ctrl.int4_sts!=gtmv(m, 1) || cfg_int_ctrl.int5_sts!=gtmv(m, 1) || cfg_int_ctrl.int6_sts!=gtmv(m, 1) || cfg_int_ctrl.int7_sts!=gtmv(m, 1) || cfg_int_ctrl.int8_sts!=gtmv(m, 1) || cfg_int_ctrl.int9_sts!=gtmv(m, 1) || cfg_int_ctrl.fit_fail_sts!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_regs_cfg_int_mask cfg_int_mask = {.int0_mask=gtmv(m, 8), .int1_mask=gtmv(m, 8), .int2_mask=gtmv(m, 1), .int3_mask=gtmv(m, 1), .int4_mask=gtmv(m, 1), .int5_mask=gtmv(m, 1), .int6_mask=gtmv(m, 1), .int7_mask=gtmv(m, 1), .int8_mask=gtmv(m, 1), .int9_mask=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_mask_set(%u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_int_mask.int0_mask, cfg_int_mask.int1_mask, cfg_int_mask.int2_mask, cfg_int_mask.int3_mask, cfg_int_mask.int4_mask, cfg_int_mask.int5_mask, cfg_int_mask.int6_mask, cfg_int_mask.int7_mask, cfg_int_mask.int8_mask, cfg_int_mask.int9_mask);
        if(!err) ag_drv_rnr_regs_cfg_int_mask_set(rnr_id, &cfg_int_mask);
        if(!err) ag_drv_rnr_regs_cfg_int_mask_get( rnr_id, &cfg_int_mask);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_int_mask_get(%u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_int_mask.int0_mask, cfg_int_mask.int1_mask, cfg_int_mask.int2_mask, cfg_int_mask.int3_mask, cfg_int_mask.int4_mask, cfg_int_mask.int5_mask, cfg_int_mask.int6_mask, cfg_int_mask.int7_mask, cfg_int_mask.int8_mask, cfg_int_mask.int9_mask);
        if(err || cfg_int_mask.int0_mask!=gtmv(m, 8) || cfg_int_mask.int1_mask!=gtmv(m, 8) || cfg_int_mask.int2_mask!=gtmv(m, 1) || cfg_int_mask.int3_mask!=gtmv(m, 1) || cfg_int_mask.int4_mask!=gtmv(m, 1) || cfg_int_mask.int5_mask!=gtmv(m, 1) || cfg_int_mask.int6_mask!=gtmv(m, 1) || cfg_int_mask.int7_mask!=gtmv(m, 1) || cfg_int_mask.int8_mask!=gtmv(m, 1) || cfg_int_mask.int9_mask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean disable_dma_old_flow_control=gtmv(m, 1);
        bdmf_boolean test_fit_fail=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_gen_cfg_set(%u %u %u)\n", rnr_id, disable_dma_old_flow_control, test_fit_fail);
        if(!err) ag_drv_rnr_regs_cfg_gen_cfg_set(rnr_id, disable_dma_old_flow_control, test_fit_fail);
        if(!err) ag_drv_rnr_regs_cfg_gen_cfg_get( rnr_id, &disable_dma_old_flow_control, &test_fit_fail);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_gen_cfg_get(%u %u %u)\n", rnr_id, disable_dma_old_flow_control, test_fit_fail);
        if(err || disable_dma_old_flow_control!=gtmv(m, 1) || test_fit_fail!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t dma_base=gtmv(m, 20);
        uint8_t dma_buf_size=gtmv(m, 3);
        uint8_t dma_static_offset=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ddr_cfg_set(%u %u %u %u)\n", rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(!err) ag_drv_rnr_regs_cfg_ddr_cfg_set(rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(!err) ag_drv_rnr_regs_cfg_ddr_cfg_get( rnr_id, &dma_base, &dma_buf_size, &dma_static_offset);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ddr_cfg_get(%u %u %u %u)\n", rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(err || dma_base!=gtmv(m, 20) || dma_buf_size!=gtmv(m, 3) || dma_static_offset!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t dma_base=gtmv(m, 20);
        uint8_t dma_buf_size=gtmv(m, 3);
        uint8_t dma_static_offset=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_psram_cfg_set(%u %u %u %u)\n", rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(!err) ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(!err) ag_drv_rnr_regs_cfg_psram_cfg_get( rnr_id, &dma_base, &dma_buf_size, &dma_static_offset);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_psram_cfg_get(%u %u %u %u)\n", rnr_id, dma_base, dma_buf_size, dma_static_offset);
        if(err || dma_base!=gtmv(m, 20) || dma_buf_size!=gtmv(m, 3) || dma_static_offset!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t mask0=gtmv(m, 16);
        uint16_t mask1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(%u %u %u)\n", rnr_id, mask0, mask1);
        if(!err) ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(rnr_id, mask0, mask1);
        if(!err) ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get( rnr_id, &mask0, &mask1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(%u %u %u)\n", rnr_id, mask0, mask1);
        if(err || mask0!=gtmv(m, 16) || mask1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t scheduler_mode=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_sch_cfg_set(%u %u)\n", rnr_id, scheduler_mode);
        if(!err) ag_drv_rnr_regs_cfg_sch_cfg_set(rnr_id, scheduler_mode);
        if(!err) ag_drv_rnr_regs_cfg_sch_cfg_get( rnr_id, &scheduler_mode);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_sch_cfg_get(%u %u)\n", rnr_id, scheduler_mode);
        if(err || scheduler_mode!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_regs_cfg_bkpt_cfg cfg_bkpt_cfg = {.bkpt_0_en=gtmv(m, 1), .bkpt_0_use_thread=gtmv(m, 1), .bkpt_1_en=gtmv(m, 1), .bkpt_1_use_thread=gtmv(m, 1), .bkpt_2_en=gtmv(m, 1), .bkpt_2_use_thread=gtmv(m, 1), .bkpt_3_en=gtmv(m, 1), .bkpt_3_use_thread=gtmv(m, 1), .bkpt_4_en=gtmv(m, 1), .bkpt_4_use_thread=gtmv(m, 1), .bkpt_5_en=gtmv(m, 1), .bkpt_5_use_thread=gtmv(m, 1), .bkpt_6_en=gtmv(m, 1), .bkpt_6_use_thread=gtmv(m, 1), .bkpt_7_en=gtmv(m, 1), .bkpt_7_use_thread=gtmv(m, 1), .step_mode=gtmv(m, 1), .new_flags_val=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_cfg_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_use_thread, cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_use_thread, cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_use_thread, cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_use_thread, cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.new_flags_val);
        if(!err) ag_drv_rnr_regs_cfg_bkpt_cfg_set(rnr_id, &cfg_bkpt_cfg);
        if(!err) ag_drv_rnr_regs_cfg_bkpt_cfg_get( rnr_id, &cfg_bkpt_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_cfg_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rnr_id, cfg_bkpt_cfg.bkpt_0_en, cfg_bkpt_cfg.bkpt_0_use_thread, cfg_bkpt_cfg.bkpt_1_en, cfg_bkpt_cfg.bkpt_1_use_thread, cfg_bkpt_cfg.bkpt_2_en, cfg_bkpt_cfg.bkpt_2_use_thread, cfg_bkpt_cfg.bkpt_3_en, cfg_bkpt_cfg.bkpt_3_use_thread, cfg_bkpt_cfg.bkpt_4_en, cfg_bkpt_cfg.bkpt_4_use_thread, cfg_bkpt_cfg.bkpt_5_en, cfg_bkpt_cfg.bkpt_5_use_thread, cfg_bkpt_cfg.bkpt_6_en, cfg_bkpt_cfg.bkpt_6_use_thread, cfg_bkpt_cfg.bkpt_7_en, cfg_bkpt_cfg.bkpt_7_use_thread, cfg_bkpt_cfg.step_mode, cfg_bkpt_cfg.new_flags_val);
        if(err || cfg_bkpt_cfg.bkpt_0_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_0_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_1_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_1_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_2_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_2_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_3_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_3_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_4_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_4_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_5_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_5_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_6_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_6_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_7_en!=gtmv(m, 1) || cfg_bkpt_cfg.bkpt_7_use_thread!=gtmv(m, 1) || cfg_bkpt_cfg.step_mode!=gtmv(m, 1) || cfg_bkpt_cfg.new_flags_val!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_imm_set(%u %u)\n", rnr_id, enable);
        if(!err) ag_drv_rnr_regs_cfg_bkpt_imm_set(rnr_id, enable);
        if(!err) ag_drv_rnr_regs_cfg_bkpt_imm_get( rnr_id, &enable);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_imm_get(%u %u)\n", rnr_id, enable);
        if(err || enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t bkpt_addr=gtmv(m, 13);
        bdmf_boolean active=gtmv(m, 1);
        if(!err) ag_drv_rnr_regs_cfg_bkpt_sts_get( rnr_id, &bkpt_addr, &active);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_bkpt_sts_get(%u %u %u)\n", rnr_id, bkpt_addr, active);
    }
    {
        uint16_t current_pc_addr=gtmv(m, 13);
        uint16_t pc_ret=gtmv(m, 13);
        if(!err) ag_drv_rnr_regs_cfg_pc_sts_get( rnr_id, &current_pc_addr, &pc_ret);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_pc_sts_get(%u %u %u)\n", rnr_id, current_pc_addr, pc_ret);
    }
    {
        uint16_t trace_base_addr=gtmv(m, 13);
        uint16_t trace_max_addr=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_cfg_0_set(%u %u %u)\n", rnr_id, trace_base_addr, trace_max_addr);
        if(!err) ag_drv_rnr_regs_cfg_profiling_cfg_0_set(rnr_id, trace_base_addr, trace_max_addr);
        if(!err) ag_drv_rnr_regs_cfg_profiling_cfg_0_get( rnr_id, &trace_base_addr, &trace_max_addr);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_cfg_0_get(%u %u %u)\n", rnr_id, trace_base_addr, trace_max_addr);
        if(err || trace_base_addr!=gtmv(m, 13) || trace_max_addr!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_rnr_regs_cfg_profiling_counter_get( rnr_id, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_regs_cfg_profiling_counter_get(%u %u)\n", rnr_id, val);
    }
    return err;
}

static int bcm_rnr_regs_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cfg_global_ctrl : reg = &RU_REG(RNR_REGS, CFG_GLOBAL_CTRL); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_cpu_wakeup : reg = &RU_REG(RNR_REGS, CFG_CPU_WAKEUP); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_int_ctrl : reg = &RU_REG(RNR_REGS, CFG_INT_CTRL); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_int_mask : reg = &RU_REG(RNR_REGS, CFG_INT_MASK); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_gen_cfg : reg = &RU_REG(RNR_REGS, CFG_GEN_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_cam_cfg : reg = &RU_REG(RNR_REGS, CFG_CAM_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_ddr_cfg : reg = &RU_REG(RNR_REGS, CFG_DDR_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_psram_cfg : reg = &RU_REG(RNR_REGS, CFG_PSRAM_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_ramrd_range_mask_cfg : reg = &RU_REG(RNR_REGS, CFG_RAMRD_RANGE_MASK_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_sch_cfg : reg = &RU_REG(RNR_REGS, CFG_SCH_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_cfg : reg = &RU_REG(RNR_REGS, CFG_BKPT_CFG); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_imm : reg = &RU_REG(RNR_REGS, CFG_BKPT_IMM); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_bkpt_sts : reg = &RU_REG(RNR_REGS, CFG_BKPT_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_pc_sts : reg = &RU_REG(RNR_REGS, CFG_PC_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_sts : reg = &RU_REG(RNR_REGS, CFG_PROFILING_STS); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_cfg_0 : reg = &RU_REG(RNR_REGS, CFG_PROFILING_CFG_0); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_cfg_1 : reg = &RU_REG(RNR_REGS, CFG_PROFILING_CFG_1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_profiling_counter : reg = &RU_REG(RNR_REGS, CFG_PROFILING_COUNTER); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt1 : reg = &RU_REG(RNR_REGS, CFG_STALL_CNT1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_stall_cnt2 : reg = &RU_REG(RNR_REGS, CFG_STALL_CNT2); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_idle_cnt1 : reg = &RU_REG(RNR_REGS, CFG_IDLE_CNT1); blk = &RU_BLK(RNR_REGS); break;
    case bdmf_address_cfg_jmp_cnt : reg = &RU_REG(RNR_REGS, CFG_JMP_CNT); blk = &RU_BLK(RNR_REGS); break;
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

bdmfmon_handle_t ag_drv_rnr_regs_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "rnr_regs"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "rnr_regs", "rnr_regs", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_rnr_enable[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_illegal[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dma_illegal_status", "dma_illegal_status", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_freq[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("micro_sec_val", "micro_sec_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cam_stop_val[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("stop_value", "stop_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_trace_config[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("trace_wraparound", "trace_wraparound", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_mode", "trace_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_disable_idle_in", "trace_disable_idle_in", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_disable_wakeup_log", "trace_disable_wakeup_log", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_task", "trace_task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("idle_counter_source_sel", "idle_counter_source_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_reset_event_fifo", "trace_reset_event_fifo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_clear_fifo_overrun", "trace_clear_fifo_overrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_trace_fifo[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("trace_reset_event_fifo", "trace_reset_event_fifo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clear_trace_fifo_overrun[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("trace_clear_fifo_overrun", "trace_clear_fifo_overrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_cpu_wakeup[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("thread_num", "thread_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_int_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("int0_sts", "int0_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int1_sts", "int1_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int2_sts", "int2_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int3_sts", "int3_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int4_sts", "int4_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int5_sts", "int5_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int6_sts", "int6_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int7_sts", "int7_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int8_sts", "int8_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int9_sts", "int9_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fit_fail_sts", "fit_fail_sts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_int_mask[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("int0_mask", "int0_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int1_mask", "int1_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int2_mask", "int2_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int3_mask", "int3_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int4_mask", "int4_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int5_mask", "int5_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int6_mask", "int6_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int7_mask", "int7_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int8_mask", "int8_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int9_mask", "int9_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_gen_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("disable_dma_old_flow_control", "disable_dma_old_flow_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("test_fit_fail", "test_fit_fail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ddr_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dma_base", "dma_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size", "dma_buf_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dma_static_offset", "dma_static_offset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_psram_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dma_base", "dma_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dma_buf_size", "dma_buf_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dma_static_offset", "dma_static_offset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ramrd_range_mask_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mask0", "mask0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mask1", "mask1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_sch_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("scheduler_mode", "scheduler_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_bkpt_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bkpt_0_en", "bkpt_0_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_0_use_thread", "bkpt_0_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_en", "bkpt_1_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_1_use_thread", "bkpt_1_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_en", "bkpt_2_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_2_use_thread", "bkpt_2_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_en", "bkpt_3_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_3_use_thread", "bkpt_3_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_4_en", "bkpt_4_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_4_use_thread", "bkpt_4_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_5_en", "bkpt_5_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_5_use_thread", "bkpt_5_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_6_en", "bkpt_6_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_6_use_thread", "bkpt_6_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_7_en", "bkpt_7_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bkpt_7_use_thread", "bkpt_7_use_thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("step_mode", "step_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("new_flags_val", "new_flags_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_bkpt_imm[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_profiling_cfg_0[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("trace_base_addr", "trace_base_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("trace_max_addr", "trace_max_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rnr_enable", .val=cli_rnr_regs_rnr_enable, .parms=set_rnr_enable },
            { .name="dma_illegal", .val=cli_rnr_regs_dma_illegal, .parms=set_dma_illegal },
            { .name="rnr_freq", .val=cli_rnr_regs_rnr_freq, .parms=set_rnr_freq },
            { .name="cam_stop_val", .val=cli_rnr_regs_cam_stop_val, .parms=set_cam_stop_val },
            { .name="trace_config", .val=cli_rnr_regs_trace_config, .parms=set_trace_config },
            { .name="reset_trace_fifo", .val=cli_rnr_regs_reset_trace_fifo, .parms=set_reset_trace_fifo },
            { .name="clear_trace_fifo_overrun", .val=cli_rnr_regs_clear_trace_fifo_overrun, .parms=set_clear_trace_fifo_overrun },
            { .name="cfg_cpu_wakeup", .val=cli_rnr_regs_cfg_cpu_wakeup, .parms=set_cfg_cpu_wakeup },
            { .name="cfg_int_ctrl", .val=cli_rnr_regs_cfg_int_ctrl, .parms=set_cfg_int_ctrl },
            { .name="cfg_int_mask", .val=cli_rnr_regs_cfg_int_mask, .parms=set_cfg_int_mask },
            { .name="cfg_gen_cfg", .val=cli_rnr_regs_cfg_gen_cfg, .parms=set_cfg_gen_cfg },
            { .name="cfg_ddr_cfg", .val=cli_rnr_regs_cfg_ddr_cfg, .parms=set_cfg_ddr_cfg },
            { .name="cfg_psram_cfg", .val=cli_rnr_regs_cfg_psram_cfg, .parms=set_cfg_psram_cfg },
            { .name="cfg_ramrd_range_mask_cfg", .val=cli_rnr_regs_cfg_ramrd_range_mask_cfg, .parms=set_cfg_ramrd_range_mask_cfg },
            { .name="cfg_sch_cfg", .val=cli_rnr_regs_cfg_sch_cfg, .parms=set_cfg_sch_cfg },
            { .name="cfg_bkpt_cfg", .val=cli_rnr_regs_cfg_bkpt_cfg, .parms=set_cfg_bkpt_cfg },
            { .name="cfg_bkpt_imm", .val=cli_rnr_regs_cfg_bkpt_imm, .parms=set_cfg_bkpt_imm },
            { .name="cfg_profiling_cfg_0", .val=cli_rnr_regs_cfg_profiling_cfg_0, .parms=set_cfg_profiling_cfg_0 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_rnr_regs_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rnr_enable", .val=cli_rnr_regs_rnr_enable, .parms=set_default },
            { .name="dma_illegal", .val=cli_rnr_regs_dma_illegal, .parms=set_default },
            { .name="rnr_freq", .val=cli_rnr_regs_rnr_freq, .parms=set_default },
            { .name="cam_stop_val", .val=cli_rnr_regs_cam_stop_val, .parms=set_default },
            { .name="profiling_sts", .val=cli_rnr_regs_profiling_sts, .parms=set_default },
            { .name="is_trace_fifo_overrun", .val=cli_rnr_regs_is_trace_fifo_overrun, .parms=set_default },
            { .name="trace_config", .val=cli_rnr_regs_trace_config, .parms=set_default },
            { .name="reset_trace_fifo", .val=cli_rnr_regs_reset_trace_fifo, .parms=set_default },
            { .name="clear_trace_fifo_overrun", .val=cli_rnr_regs_clear_trace_fifo_overrun, .parms=set_default },
            { .name="rnr_core_cntrs", .val=cli_rnr_regs_rnr_core_cntrs, .parms=set_default },
            { .name="cfg_cpu_wakeup", .val=cli_rnr_regs_cfg_cpu_wakeup, .parms=set_default },
            { .name="cfg_int_ctrl", .val=cli_rnr_regs_cfg_int_ctrl, .parms=set_default },
            { .name="cfg_int_mask", .val=cli_rnr_regs_cfg_int_mask, .parms=set_default },
            { .name="cfg_gen_cfg", .val=cli_rnr_regs_cfg_gen_cfg, .parms=set_default },
            { .name="cfg_ddr_cfg", .val=cli_rnr_regs_cfg_ddr_cfg, .parms=set_default },
            { .name="cfg_psram_cfg", .val=cli_rnr_regs_cfg_psram_cfg, .parms=set_default },
            { .name="cfg_ramrd_range_mask_cfg", .val=cli_rnr_regs_cfg_ramrd_range_mask_cfg, .parms=set_default },
            { .name="cfg_sch_cfg", .val=cli_rnr_regs_cfg_sch_cfg, .parms=set_default },
            { .name="cfg_bkpt_cfg", .val=cli_rnr_regs_cfg_bkpt_cfg, .parms=set_default },
            { .name="cfg_bkpt_imm", .val=cli_rnr_regs_cfg_bkpt_imm, .parms=set_default },
            { .name="cfg_bkpt_sts", .val=cli_rnr_regs_cfg_bkpt_sts, .parms=set_default },
            { .name="cfg_pc_sts", .val=cli_rnr_regs_cfg_pc_sts, .parms=set_default },
            { .name="cfg_profiling_cfg_0", .val=cli_rnr_regs_cfg_profiling_cfg_0, .parms=set_default },
            { .name="cfg_profiling_counter", .val=cli_rnr_regs_cfg_profiling_counter, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_rnr_regs_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_rnr_regs_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CFG_GLOBAL_CTRL" , .val=bdmf_address_cfg_global_ctrl },
            { .name="CFG_CPU_WAKEUP" , .val=bdmf_address_cfg_cpu_wakeup },
            { .name="CFG_INT_CTRL" , .val=bdmf_address_cfg_int_ctrl },
            { .name="CFG_INT_MASK" , .val=bdmf_address_cfg_int_mask },
            { .name="CFG_GEN_CFG" , .val=bdmf_address_cfg_gen_cfg },
            { .name="CFG_CAM_CFG" , .val=bdmf_address_cfg_cam_cfg },
            { .name="CFG_DDR_CFG" , .val=bdmf_address_cfg_ddr_cfg },
            { .name="CFG_PSRAM_CFG" , .val=bdmf_address_cfg_psram_cfg },
            { .name="CFG_RAMRD_RANGE_MASK_CFG" , .val=bdmf_address_cfg_ramrd_range_mask_cfg },
            { .name="CFG_SCH_CFG" , .val=bdmf_address_cfg_sch_cfg },
            { .name="CFG_BKPT_CFG" , .val=bdmf_address_cfg_bkpt_cfg },
            { .name="CFG_BKPT_IMM" , .val=bdmf_address_cfg_bkpt_imm },
            { .name="CFG_BKPT_STS" , .val=bdmf_address_cfg_bkpt_sts },
            { .name="CFG_PC_STS" , .val=bdmf_address_cfg_pc_sts },
            { .name="CFG_PROFILING_STS" , .val=bdmf_address_cfg_profiling_sts },
            { .name="CFG_PROFILING_CFG_0" , .val=bdmf_address_cfg_profiling_cfg_0 },
            { .name="CFG_PROFILING_CFG_1" , .val=bdmf_address_cfg_profiling_cfg_1 },
            { .name="CFG_PROFILING_COUNTER" , .val=bdmf_address_cfg_profiling_counter },
            { .name="CFG_STALL_CNT1" , .val=bdmf_address_cfg_stall_cnt1 },
            { .name="CFG_STALL_CNT2" , .val=bdmf_address_cfg_stall_cnt2 },
            { .name="CFG_IDLE_CNT1" , .val=bdmf_address_cfg_idle_cnt1 },
            { .name="CFG_JMP_CNT" , .val=bdmf_address_cfg_jmp_cnt },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_rnr_regs_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "rnr_id", rnr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

