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

bdmf_error_t ag_drv_ubus_resp_vpb_start_set(uint32_t start)
{
    uint32_t reg_vpb_start = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_vpb_start = RU_FIELD_SET(0, UBUS_RESP, VPB_START, START, reg_vpb_start, start);

    RU_REG_WRITE(0, UBUS_RESP, VPB_START, reg_vpb_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_vpb_start_get(uint32_t *start)
{
    uint32_t reg_vpb_start;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, VPB_START, reg_vpb_start);

    *start = RU_FIELD_GET(0, UBUS_RESP, VPB_START, START, reg_vpb_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_vpb_end_set(uint32_t end)
{
    uint32_t reg_vpb_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_vpb_end = RU_FIELD_SET(0, UBUS_RESP, VPB_END, END, reg_vpb_end, end);

    RU_REG_WRITE(0, UBUS_RESP, VPB_END, reg_vpb_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_vpb_end_get(uint32_t *end)
{
    uint32_t reg_vpb_end;

#ifdef VALIDATE_PARMS
    if (!end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, VPB_END, reg_vpb_end);

    *end = RU_FIELD_GET(0, UBUS_RESP, VPB_END, END, reg_vpb_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_apb_start_set(uint32_t start)
{
    uint32_t reg_apb_start = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_apb_start = RU_FIELD_SET(0, UBUS_RESP, APB_START, START, reg_apb_start, start);

    RU_REG_WRITE(0, UBUS_RESP, APB_START, reg_apb_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_apb_start_get(uint32_t *start)
{
    uint32_t reg_apb_start;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, APB_START, reg_apb_start);

    *start = RU_FIELD_GET(0, UBUS_RESP, APB_START, START, reg_apb_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_apb_end_set(uint32_t end)
{
    uint32_t reg_apb_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_apb_end = RU_FIELD_SET(0, UBUS_RESP, APB_END, END, reg_apb_end, end);

    RU_REG_WRITE(0, UBUS_RESP, APB_END, reg_apb_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_apb_end_get(uint32_t *end)
{
    uint32_t reg_apb_end;

#ifdef VALIDATE_PARMS
    if (!end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, APB_END, reg_apb_end);

    *end = RU_FIELD_GET(0, UBUS_RESP, APB_END, END, reg_apb_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_0_start_set(uint32_t start)
{
    uint32_t reg_device_0_start = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_0_start = RU_FIELD_SET(0, UBUS_RESP, DEVICE_0_START, START, reg_device_0_start, start);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_0_START, reg_device_0_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_0_start_get(uint32_t *start)
{
    uint32_t reg_device_0_start;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_0_START, reg_device_0_start);

    *start = RU_FIELD_GET(0, UBUS_RESP, DEVICE_0_START, START, reg_device_0_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_0_end_set(uint32_t end)
{
    uint32_t reg_device_0_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_0_end = RU_FIELD_SET(0, UBUS_RESP, DEVICE_0_END, END, reg_device_0_end, end);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_0_END, reg_device_0_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_0_end_get(uint32_t *end)
{
    uint32_t reg_device_0_end;

#ifdef VALIDATE_PARMS
    if (!end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_0_END, reg_device_0_end);

    *end = RU_FIELD_GET(0, UBUS_RESP, DEVICE_0_END, END, reg_device_0_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_1_start_set(uint32_t start)
{
    uint32_t reg_device_1_start = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_1_start = RU_FIELD_SET(0, UBUS_RESP, DEVICE_1_START, START, reg_device_1_start, start);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_1_START, reg_device_1_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_1_start_get(uint32_t *start)
{
    uint32_t reg_device_1_start;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_1_START, reg_device_1_start);

    *start = RU_FIELD_GET(0, UBUS_RESP, DEVICE_1_START, START, reg_device_1_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_1_end_set(uint32_t end)
{
    uint32_t reg_device_1_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_1_end = RU_FIELD_SET(0, UBUS_RESP, DEVICE_1_END, END, reg_device_1_end, end);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_1_END, reg_device_1_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_1_end_get(uint32_t *end)
{
    uint32_t reg_device_1_end;

#ifdef VALIDATE_PARMS
    if (!end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_1_END, reg_device_1_end);

    *end = RU_FIELD_GET(0, UBUS_RESP, DEVICE_1_END, END, reg_device_1_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_2_start_set(uint32_t start)
{
    uint32_t reg_device_2_start = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_2_start = RU_FIELD_SET(0, UBUS_RESP, DEVICE_2_START, START, reg_device_2_start, start);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_2_START, reg_device_2_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_2_start_get(uint32_t *start)
{
    uint32_t reg_device_2_start;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_2_START, reg_device_2_start);

    *start = RU_FIELD_GET(0, UBUS_RESP, DEVICE_2_START, START, reg_device_2_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_2_end_set(uint32_t end)
{
    uint32_t reg_device_2_end = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_device_2_end = RU_FIELD_SET(0, UBUS_RESP, DEVICE_2_END, END, reg_device_2_end, end);

    RU_REG_WRITE(0, UBUS_RESP, DEVICE_2_END, reg_device_2_end);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_resp_device_2_end_get(uint32_t *end)
{
    uint32_t reg_device_2_end;

#ifdef VALIDATE_PARMS
    if (!end)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_RESP, DEVICE_2_END, reg_device_2_end);

    *end = RU_FIELD_GET(0, UBUS_RESP, DEVICE_2_END, END, reg_device_2_end);

    return BDMF_ERR_OK;
}

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

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_vpb_start,
    bdmf_address_vpb_end,
    bdmf_address_apb_start,
    bdmf_address_apb_end,
    bdmf_address_device_0_start,
    bdmf_address_device_0_end,
    bdmf_address_device_1_start,
    bdmf_address_device_1_end,
    bdmf_address_device_2_start,
    bdmf_address_device_2_end,
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
}
bdmf_address;

static int ag_drv_ubus_resp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_ubus_resp_vpb_start:
        ag_err = ag_drv_ubus_resp_vpb_start_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_vpb_end:
        ag_err = ag_drv_ubus_resp_vpb_end_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_apb_start:
        ag_err = ag_drv_ubus_resp_apb_start_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_apb_end:
        ag_err = ag_drv_ubus_resp_apb_end_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_0_start:
        ag_err = ag_drv_ubus_resp_device_0_start_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_0_end:
        ag_err = ag_drv_ubus_resp_device_0_end_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_1_start:
        ag_err = ag_drv_ubus_resp_device_1_start_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_1_end:
        ag_err = ag_drv_ubus_resp_device_1_end_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_2_start:
        ag_err = ag_drv_ubus_resp_device_2_start_set(parm[1].value.unumber);
        break;
    case cli_ubus_resp_device_2_end:
        ag_err = ag_drv_ubus_resp_device_2_end_set(parm[1].value.unumber);
        break;
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
    case cli_ubus_resp_vpb_start:
    {
        uint32_t start;
        ag_err = ag_drv_ubus_resp_vpb_start_get(&start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_ubus_resp_vpb_end:
    {
        uint32_t end;
        ag_err = ag_drv_ubus_resp_vpb_end_get(&end);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
    case cli_ubus_resp_apb_start:
    {
        uint32_t start;
        ag_err = ag_drv_ubus_resp_apb_start_get(&start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_ubus_resp_apb_end:
    {
        uint32_t end;
        ag_err = ag_drv_ubus_resp_apb_end_get(&end);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
    case cli_ubus_resp_device_0_start:
    {
        uint32_t start;
        ag_err = ag_drv_ubus_resp_device_0_start_get(&start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_ubus_resp_device_0_end:
    {
        uint32_t end;
        ag_err = ag_drv_ubus_resp_device_0_end_get(&end);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
    case cli_ubus_resp_device_1_start:
    {
        uint32_t start;
        ag_err = ag_drv_ubus_resp_device_1_start_get(&start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_ubus_resp_device_1_end:
    {
        uint32_t end;
        ag_err = ag_drv_ubus_resp_device_1_end_get(&end);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
    case cli_ubus_resp_device_2_start:
    {
        uint32_t start;
        ag_err = ag_drv_ubus_resp_device_2_start_get(&start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_ubus_resp_device_2_end:
    {
        uint32_t end;
        ag_err = ag_drv_ubus_resp_device_2_end_get(&end);
        bdmf_session_print(session, "end = %u = 0x%x\n", end, end);
        break;
    }
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
        uint32_t start = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_vpb_start_set( %u)\n",
            start);
        ag_err = ag_drv_ubus_resp_vpb_start_set(start);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_vpb_start_get(&start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_vpb_start_get( %u)\n",
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t end = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_vpb_end_set( %u)\n",
            end);
        ag_err = ag_drv_ubus_resp_vpb_end_set(end);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_vpb_end_get(&end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_vpb_end_get( %u)\n",
                end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t start = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_apb_start_set( %u)\n",
            start);
        ag_err = ag_drv_ubus_resp_apb_start_set(start);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_apb_start_get(&start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_apb_start_get( %u)\n",
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t end = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_apb_end_set( %u)\n",
            end);
        ag_err = ag_drv_ubus_resp_apb_end_set(end);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_apb_end_get(&end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_apb_end_get( %u)\n",
                end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t start = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_0_start_set( %u)\n",
            start);
        ag_err = ag_drv_ubus_resp_device_0_start_set(start);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_0_start_get(&start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_0_start_get( %u)\n",
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t end = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_0_end_set( %u)\n",
            end);
        ag_err = ag_drv_ubus_resp_device_0_end_set(end);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_0_end_get(&end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_0_end_get( %u)\n",
                end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t start = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_1_start_set( %u)\n",
            start);
        ag_err = ag_drv_ubus_resp_device_1_start_set(start);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_1_start_get(&start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_1_start_get( %u)\n",
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t end = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_1_end_set( %u)\n",
            end);
        ag_err = ag_drv_ubus_resp_device_1_end_set(end);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_1_end_get(&end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_1_end_get( %u)\n",
                end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t start = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_2_start_set( %u)\n",
            start);
        ag_err = ag_drv_ubus_resp_device_2_start_set(start);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_2_start_get(&start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_2_start_get( %u)\n",
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t end = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_ubus_resp_device_2_end_set( %u)\n",
            end);
        ag_err = ag_drv_ubus_resp_device_2_end_set(end);
        if (!ag_err)
            ag_err = ag_drv_ubus_resp_device_2_end_get(&end);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_ubus_resp_device_2_end_get( %u)\n",
                end);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (end != gtmv(m, 32))
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

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

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
    case bdmf_address_vpb_start: reg = &RU_REG(UBUS_RESP, VPB_START); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_vpb_end: reg = &RU_REG(UBUS_RESP, VPB_END); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_apb_start: reg = &RU_REG(UBUS_RESP, APB_START); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_apb_end: reg = &RU_REG(UBUS_RESP, APB_END); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_0_start: reg = &RU_REG(UBUS_RESP, DEVICE_0_START); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_0_end: reg = &RU_REG(UBUS_RESP, DEVICE_0_END); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_1_start: reg = &RU_REG(UBUS_RESP, DEVICE_1_START); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_1_end: reg = &RU_REG(UBUS_RESP, DEVICE_1_END); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_2_start: reg = &RU_REG(UBUS_RESP, DEVICE_2_START); blk = &RU_BLK(UBUS_RESP); break;
    case bdmf_address_device_2_end: reg = &RU_REG(UBUS_RESP, DEVICE_2_END); blk = &RU_BLK(UBUS_RESP); break;
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
        static bdmfmon_cmd_parm_t set_vpb_start[] = {
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_vpb_end[] = {
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_apb_start[] = {
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_apb_end[] = {
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_0_start[] = {
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_0_end[] = {
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_1_start[] = {
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_1_end[] = {
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_2_start[] = {
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_device_2_end[] = {
            BDMFMON_MAKE_PARM("end", "end", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
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
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "vpb_start", .val = cli_ubus_resp_vpb_start, .parms = set_vpb_start },
            { .name = "vpb_end", .val = cli_ubus_resp_vpb_end, .parms = set_vpb_end },
            { .name = "apb_start", .val = cli_ubus_resp_apb_start, .parms = set_apb_start },
            { .name = "apb_end", .val = cli_ubus_resp_apb_end, .parms = set_apb_end },
            { .name = "device_0_start", .val = cli_ubus_resp_device_0_start, .parms = set_device_0_start },
            { .name = "device_0_end", .val = cli_ubus_resp_device_0_end, .parms = set_device_0_end },
            { .name = "device_1_start", .val = cli_ubus_resp_device_1_start, .parms = set_device_1_start },
            { .name = "device_1_end", .val = cli_ubus_resp_device_1_end, .parms = set_device_1_end },
            { .name = "device_2_start", .val = cli_ubus_resp_device_2_start, .parms = set_device_2_start },
            { .name = "device_2_end", .val = cli_ubus_resp_device_2_end, .parms = set_device_2_end },
            { .name = "rnr_intr_ctrl_isr", .val = cli_ubus_resp_rnr_intr_ctrl_isr, .parms = set_rnr_intr_ctrl_isr },
            { .name = "rnr_intr_ctrl_ier", .val = cli_ubus_resp_rnr_intr_ctrl_ier, .parms = set_rnr_intr_ctrl_ier },
            { .name = "rnr_intr_ctrl_itr", .val = cli_ubus_resp_rnr_intr_ctrl_itr, .parms = set_rnr_intr_ctrl_itr },
            { .name = "profiling_cfg", .val = cli_ubus_resp_profiling_cfg, .parms = set_profiling_cfg },
            { .name = "profiling_cycle_num", .val = cli_ubus_resp_profiling_cycle_num, .parms = set_profiling_cycle_num },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_ubus_resp_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "vpb_start", .val = cli_ubus_resp_vpb_start, .parms = get_default },
            { .name = "vpb_end", .val = cli_ubus_resp_vpb_end, .parms = get_default },
            { .name = "apb_start", .val = cli_ubus_resp_apb_start, .parms = get_default },
            { .name = "apb_end", .val = cli_ubus_resp_apb_end, .parms = get_default },
            { .name = "device_0_start", .val = cli_ubus_resp_device_0_start, .parms = get_default },
            { .name = "device_0_end", .val = cli_ubus_resp_device_0_end, .parms = get_default },
            { .name = "device_1_start", .val = cli_ubus_resp_device_1_start, .parms = get_default },
            { .name = "device_1_end", .val = cli_ubus_resp_device_1_end, .parms = get_default },
            { .name = "device_2_start", .val = cli_ubus_resp_device_2_start, .parms = get_default },
            { .name = "device_2_end", .val = cli_ubus_resp_device_2_end, .parms = get_default },
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
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "VPB_START", .val = bdmf_address_vpb_start },
            { .name = "VPB_END", .val = bdmf_address_vpb_end },
            { .name = "APB_START", .val = bdmf_address_apb_start },
            { .name = "APB_END", .val = bdmf_address_apb_end },
            { .name = "DEVICE_0_START", .val = bdmf_address_device_0_start },
            { .name = "DEVICE_0_END", .val = bdmf_address_device_0_end },
            { .name = "DEVICE_1_START", .val = bdmf_address_device_1_start },
            { .name = "DEVICE_1_END", .val = bdmf_address_device_1_end },
            { .name = "DEVICE_2_START", .val = bdmf_address_device_2_start },
            { .name = "DEVICE_2_END", .val = bdmf_address_device_2_end },
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
