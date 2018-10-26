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
#include "xrdp_drv_ubus_slv_ag.h"

bdmf_error_t ag_drv_ubus_slv_vpb_base_set(uint32_t base)
{
    uint32_t reg_vpb_base=0;

#ifdef VALIDATE_PARMS
#endif

    reg_vpb_base = RU_FIELD_SET(0, UBUS_SLV, VPB_BASE, BASE, reg_vpb_base, base);

    RU_REG_WRITE(0, UBUS_SLV, VPB_BASE, reg_vpb_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_vpb_base_get(uint32_t *base)
{
    uint32_t reg_vpb_base;

#ifdef VALIDATE_PARMS
    if(!base)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, VPB_BASE, reg_vpb_base);

    *base = RU_FIELD_GET(0, UBUS_SLV, VPB_BASE, BASE, reg_vpb_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_vpb_mask_set(uint32_t mask)
{
    uint32_t reg_vpb_mask=0;

#ifdef VALIDATE_PARMS
#endif

    reg_vpb_mask = RU_FIELD_SET(0, UBUS_SLV, VPB_MASK, MASK, reg_vpb_mask, mask);

    RU_REG_WRITE(0, UBUS_SLV, VPB_MASK, reg_vpb_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_vpb_mask_get(uint32_t *mask)
{
    uint32_t reg_vpb_mask;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, VPB_MASK, reg_vpb_mask);

    *mask = RU_FIELD_GET(0, UBUS_SLV, VPB_MASK, MASK, reg_vpb_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_apb_base_set(uint32_t base)
{
    uint32_t reg_apb_base=0;

#ifdef VALIDATE_PARMS
#endif

    reg_apb_base = RU_FIELD_SET(0, UBUS_SLV, APB_BASE, BASE, reg_apb_base, base);

    RU_REG_WRITE(0, UBUS_SLV, APB_BASE, reg_apb_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_apb_base_get(uint32_t *base)
{
    uint32_t reg_apb_base;

#ifdef VALIDATE_PARMS
    if(!base)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, APB_BASE, reg_apb_base);

    *base = RU_FIELD_GET(0, UBUS_SLV, APB_BASE, BASE, reg_apb_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_apb_mask_set(uint32_t mask)
{
    uint32_t reg_apb_mask=0;

#ifdef VALIDATE_PARMS
#endif

    reg_apb_mask = RU_FIELD_SET(0, UBUS_SLV, APB_MASK, MASK, reg_apb_mask, mask);

    RU_REG_WRITE(0, UBUS_SLV, APB_MASK, reg_apb_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_apb_mask_get(uint32_t *mask)
{
    uint32_t reg_apb_mask;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, APB_MASK, reg_apb_mask);

    *mask = RU_FIELD_GET(0, UBUS_SLV, APB_MASK, MASK, reg_apb_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_dqm_base_set(uint32_t base)
{
    uint32_t reg_dqm_base=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dqm_base = RU_FIELD_SET(0, UBUS_SLV, DQM_BASE, BASE, reg_dqm_base, base);

    RU_REG_WRITE(0, UBUS_SLV, DQM_BASE, reg_dqm_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_dqm_base_get(uint32_t *base)
{
    uint32_t reg_dqm_base;

#ifdef VALIDATE_PARMS
    if(!base)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, DQM_BASE, reg_dqm_base);

    *base = RU_FIELD_GET(0, UBUS_SLV, DQM_BASE, BASE, reg_dqm_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_dqm_mask_set(uint32_t mask)
{
    uint32_t reg_dqm_mask=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dqm_mask = RU_FIELD_SET(0, UBUS_SLV, DQM_MASK, MASK, reg_dqm_mask, mask);

    RU_REG_WRITE(0, UBUS_SLV, DQM_MASK, reg_dqm_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_dqm_mask_get(uint32_t *mask)
{
    uint32_t reg_dqm_mask;

#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, DQM_MASK, reg_dqm_mask);

    *mask = RU_FIELD_GET(0, UBUS_SLV, DQM_MASK, MASK, reg_dqm_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(uint32_t ist)
{
    uint32_t reg_rnr_intr_ctrl_isr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_isr = RU_FIELD_SET(0, UBUS_SLV, RNR_INTR_CTRL_ISR, IST, reg_rnr_intr_ctrl_isr, ist);

    RU_REG_WRITE(0, UBUS_SLV, RNR_INTR_CTRL_ISR, reg_rnr_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_get(uint32_t *ist)
{
    uint32_t reg_rnr_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if(!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, RNR_INTR_CTRL_ISR, reg_rnr_intr_ctrl_isr);

    *ist = RU_FIELD_GET(0, UBUS_SLV, RNR_INTR_CTRL_ISR, IST, reg_rnr_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_rnr_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if(!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, RNR_INTR_CTRL_ISM, reg_rnr_intr_ctrl_ism);

    *ism = RU_FIELD_GET(0, UBUS_SLV, RNR_INTR_CTRL_ISM, ISM, reg_rnr_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(uint32_t iem)
{
    uint32_t reg_rnr_intr_ctrl_ier=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_ier = RU_FIELD_SET(0, UBUS_SLV, RNR_INTR_CTRL_IER, IEM, reg_rnr_intr_ctrl_ier, iem);

    RU_REG_WRITE(0, UBUS_SLV, RNR_INTR_CTRL_IER, reg_rnr_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(uint32_t *iem)
{
    uint32_t reg_rnr_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if(!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, RNR_INTR_CTRL_IER, reg_rnr_intr_ctrl_ier);

    *iem = RU_FIELD_GET(0, UBUS_SLV, RNR_INTR_CTRL_IER, IEM, reg_rnr_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_set(uint32_t ist)
{
    uint32_t reg_rnr_intr_ctrl_itr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rnr_intr_ctrl_itr = RU_FIELD_SET(0, UBUS_SLV, RNR_INTR_CTRL_ITR, IST, reg_rnr_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, UBUS_SLV, RNR_INTR_CTRL_ITR, reg_rnr_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_get(uint32_t *ist)
{
    uint32_t reg_rnr_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if(!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, RNR_INTR_CTRL_ITR, reg_rnr_intr_ctrl_itr);

    *ist = RU_FIELD_GET(0, UBUS_SLV, RNR_INTR_CTRL_ITR, IST, reg_rnr_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_cfg_set(bdmf_boolean counter_enable, bdmf_boolean profiling_start, bdmf_boolean manual_stop_mode, bdmf_boolean do_manual_stop)
{
    uint32_t reg_profiling_cfg=0;

#ifdef VALIDATE_PARMS
    if((counter_enable >= _1BITS_MAX_VAL_) ||
       (profiling_start >= _1BITS_MAX_VAL_) ||
       (manual_stop_mode >= _1BITS_MAX_VAL_) ||
       (do_manual_stop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_SLV, PROFILING_CFG, COUNTER_ENABLE, reg_profiling_cfg, counter_enable);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_SLV, PROFILING_CFG, PROFILING_START, reg_profiling_cfg, profiling_start);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_SLV, PROFILING_CFG, MANUAL_STOP_MODE, reg_profiling_cfg, manual_stop_mode);
    reg_profiling_cfg = RU_FIELD_SET(0, UBUS_SLV, PROFILING_CFG, DO_MANUAL_STOP, reg_profiling_cfg, do_manual_stop);

    RU_REG_WRITE(0, UBUS_SLV, PROFILING_CFG, reg_profiling_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_cfg_get(bdmf_boolean *counter_enable, bdmf_boolean *profiling_start, bdmf_boolean *manual_stop_mode, bdmf_boolean *do_manual_stop)
{
    uint32_t reg_profiling_cfg;

#ifdef VALIDATE_PARMS
    if(!counter_enable || !profiling_start || !manual_stop_mode || !do_manual_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_CFG, reg_profiling_cfg);

    *counter_enable = RU_FIELD_GET(0, UBUS_SLV, PROFILING_CFG, COUNTER_ENABLE, reg_profiling_cfg);
    *profiling_start = RU_FIELD_GET(0, UBUS_SLV, PROFILING_CFG, PROFILING_START, reg_profiling_cfg);
    *manual_stop_mode = RU_FIELD_GET(0, UBUS_SLV, PROFILING_CFG, MANUAL_STOP_MODE, reg_profiling_cfg);
    *do_manual_stop = RU_FIELD_GET(0, UBUS_SLV, PROFILING_CFG, DO_MANUAL_STOP, reg_profiling_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_status_get(bdmf_boolean *profiling_on, uint32_t *cycles_counter)
{
    uint32_t reg_profiling_status;

#ifdef VALIDATE_PARMS
    if(!profiling_on || !cycles_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_STATUS, reg_profiling_status);

    *profiling_on = RU_FIELD_GET(0, UBUS_SLV, PROFILING_STATUS, PROFILING_ON, reg_profiling_status);
    *cycles_counter = RU_FIELD_GET(0, UBUS_SLV, PROFILING_STATUS, CYCLES_COUNTER, reg_profiling_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_counter_get(uint32_t *val)
{
    uint32_t reg_profiling_counter;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_COUNTER, reg_profiling_counter);

    *val = RU_FIELD_GET(0, UBUS_SLV, PROFILING_COUNTER, VAL, reg_profiling_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_start_value_get(uint32_t *val)
{
    uint32_t reg_profiling_start_value;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_START_VALUE, reg_profiling_start_value);

    *val = RU_FIELD_GET(0, UBUS_SLV, PROFILING_START_VALUE, VAL, reg_profiling_start_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_stop_value_get(uint32_t *val)
{
    uint32_t reg_profiling_stop_value;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_STOP_VALUE, reg_profiling_stop_value);

    *val = RU_FIELD_GET(0, UBUS_SLV, PROFILING_STOP_VALUE, VAL, reg_profiling_stop_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_set(uint32_t profiling_cycles_num)
{
    uint32_t reg_profiling_cycle_num=0;

#ifdef VALIDATE_PARMS
#endif

    reg_profiling_cycle_num = RU_FIELD_SET(0, UBUS_SLV, PROFILING_CYCLE_NUM, PROFILING_CYCLES_NUM, reg_profiling_cycle_num, profiling_cycles_num);

    RU_REG_WRITE(0, UBUS_SLV, PROFILING_CYCLE_NUM, reg_profiling_cycle_num);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_get(uint32_t *profiling_cycles_num)
{
    uint32_t reg_profiling_cycle_num;

#ifdef VALIDATE_PARMS
    if(!profiling_cycles_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, PROFILING_CYCLE_NUM, reg_profiling_cycle_num);

    *profiling_cycles_num = RU_FIELD_GET(0, UBUS_SLV, PROFILING_CYCLE_NUM, PROFILING_CYCLES_NUM, reg_profiling_cycle_num);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_vpb_base,
    bdmf_address_vpb_mask,
    bdmf_address_apb_base,
    bdmf_address_apb_mask,
    bdmf_address_dqm_base,
    bdmf_address_dqm_mask,
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

static int bcm_ubus_slv_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_slv_vpb_base:
        err = ag_drv_ubus_slv_vpb_base_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_vpb_mask:
        err = ag_drv_ubus_slv_vpb_mask_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_apb_base:
        err = ag_drv_ubus_slv_apb_base_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_apb_mask:
        err = ag_drv_ubus_slv_apb_mask_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_dqm_base:
        err = ag_drv_ubus_slv_dqm_base_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_dqm_mask:
        err = ag_drv_ubus_slv_dqm_mask_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_rnr_intr_ctrl_isr:
        err = ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_rnr_intr_ctrl_ier:
        err = ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_rnr_intr_ctrl_itr:
        err = ag_drv_ubus_slv_rnr_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    case cli_ubus_slv_profiling_cfg:
        err = ag_drv_ubus_slv_profiling_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_ubus_slv_profiling_cycle_num:
        err = ag_drv_ubus_slv_profiling_cycle_num_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_ubus_slv_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_slv_vpb_base:
    {
        uint32_t base;
        err = ag_drv_ubus_slv_vpb_base_get(&base);
        bdmf_session_print(session, "base = %u (0x%x)\n", base, base);
        break;
    }
    case cli_ubus_slv_vpb_mask:
    {
        uint32_t mask;
        err = ag_drv_ubus_slv_vpb_mask_get(&mask);
        bdmf_session_print(session, "mask = %u (0x%x)\n", mask, mask);
        break;
    }
    case cli_ubus_slv_apb_base:
    {
        uint32_t base;
        err = ag_drv_ubus_slv_apb_base_get(&base);
        bdmf_session_print(session, "base = %u (0x%x)\n", base, base);
        break;
    }
    case cli_ubus_slv_apb_mask:
    {
        uint32_t mask;
        err = ag_drv_ubus_slv_apb_mask_get(&mask);
        bdmf_session_print(session, "mask = %u (0x%x)\n", mask, mask);
        break;
    }
    case cli_ubus_slv_dqm_base:
    {
        uint32_t base;
        err = ag_drv_ubus_slv_dqm_base_get(&base);
        bdmf_session_print(session, "base = %u (0x%x)\n", base, base);
        break;
    }
    case cli_ubus_slv_dqm_mask:
    {
        uint32_t mask;
        err = ag_drv_ubus_slv_dqm_mask_get(&mask);
        bdmf_session_print(session, "mask = %u (0x%x)\n", mask, mask);
        break;
    }
    case cli_ubus_slv_rnr_intr_ctrl_isr:
    {
        uint32_t ist;
        err = ag_drv_ubus_slv_rnr_intr_ctrl_isr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
        break;
    }
    case cli_ubus_slv_rnr_intr_ctrl_ism:
    {
        uint32_t ism;
        err = ag_drv_ubus_slv_rnr_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u (0x%x)\n", ism, ism);
        break;
    }
    case cli_ubus_slv_rnr_intr_ctrl_ier:
    {
        uint32_t iem;
        err = ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u (0x%x)\n", iem, iem);
        break;
    }
    case cli_ubus_slv_rnr_intr_ctrl_itr:
    {
        uint32_t ist;
        err = ag_drv_ubus_slv_rnr_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
        break;
    }
    case cli_ubus_slv_profiling_cfg:
    {
        bdmf_boolean counter_enable;
        bdmf_boolean profiling_start;
        bdmf_boolean manual_stop_mode;
        bdmf_boolean do_manual_stop;
        err = ag_drv_ubus_slv_profiling_cfg_get(&counter_enable, &profiling_start, &manual_stop_mode, &do_manual_stop);
        bdmf_session_print(session, "counter_enable = %u (0x%x)\n", counter_enable, counter_enable);
        bdmf_session_print(session, "profiling_start = %u (0x%x)\n", profiling_start, profiling_start);
        bdmf_session_print(session, "manual_stop_mode = %u (0x%x)\n", manual_stop_mode, manual_stop_mode);
        bdmf_session_print(session, "do_manual_stop = %u (0x%x)\n", do_manual_stop, do_manual_stop);
        break;
    }
    case cli_ubus_slv_profiling_status:
    {
        bdmf_boolean profiling_on;
        uint32_t cycles_counter;
        err = ag_drv_ubus_slv_profiling_status_get(&profiling_on, &cycles_counter);
        bdmf_session_print(session, "profiling_on = %u (0x%x)\n", profiling_on, profiling_on);
        bdmf_session_print(session, "cycles_counter = %u (0x%x)\n", cycles_counter, cycles_counter);
        break;
    }
    case cli_ubus_slv_profiling_counter:
    {
        uint32_t val;
        err = ag_drv_ubus_slv_profiling_counter_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_ubus_slv_profiling_start_value:
    {
        uint32_t val;
        err = ag_drv_ubus_slv_profiling_start_value_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_ubus_slv_profiling_stop_value:
    {
        uint32_t val;
        err = ag_drv_ubus_slv_profiling_stop_value_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_ubus_slv_profiling_cycle_num:
    {
        uint32_t profiling_cycles_num;
        err = ag_drv_ubus_slv_profiling_cycle_num_get(&profiling_cycles_num);
        bdmf_session_print(session, "profiling_cycles_num = %u (0x%x)\n", profiling_cycles_num, profiling_cycles_num);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_ubus_slv_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t base=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_vpb_base_set( %u)\n", base);
        if(!err) ag_drv_ubus_slv_vpb_base_set(base);
        if(!err) ag_drv_ubus_slv_vpb_base_get( &base);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_vpb_base_get( %u)\n", base);
        if(err || base!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mask=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_vpb_mask_set( %u)\n", mask);
        if(!err) ag_drv_ubus_slv_vpb_mask_set(mask);
        if(!err) ag_drv_ubus_slv_vpb_mask_get( &mask);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_vpb_mask_get( %u)\n", mask);
        if(err || mask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t base=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_apb_base_set( %u)\n", base);
        if(!err) ag_drv_ubus_slv_apb_base_set(base);
        if(!err) ag_drv_ubus_slv_apb_base_get( &base);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_apb_base_get( %u)\n", base);
        if(err || base!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mask=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_apb_mask_set( %u)\n", mask);
        if(!err) ag_drv_ubus_slv_apb_mask_set(mask);
        if(!err) ag_drv_ubus_slv_apb_mask_get( &mask);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_apb_mask_get( %u)\n", mask);
        if(err || mask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t base=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_dqm_base_set( %u)\n", base);
        if(!err) ag_drv_ubus_slv_dqm_base_set(base);
        if(!err) ag_drv_ubus_slv_dqm_base_get( &base);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_dqm_base_get( %u)\n", base);
        if(err || base!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mask=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_dqm_mask_set( %u)\n", mask);
        if(!err) ag_drv_ubus_slv_dqm_mask_set(mask);
        if(!err) ag_drv_ubus_slv_dqm_mask_get( &mask);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_dqm_mask_get( %u)\n", mask);
        if(err || mask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_isr_set( %u)\n", ist);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(ist);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_isr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_isr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 32);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_ier_set( %u)\n", iem);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(iem);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_itr_set( %u)\n", ist);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_itr_set(ist);
        if(!err) ag_drv_ubus_slv_rnr_intr_ctrl_itr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_rnr_intr_ctrl_itr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean counter_enable=gtmv(m, 1);
        bdmf_boolean profiling_start=gtmv(m, 1);
        bdmf_boolean manual_stop_mode=gtmv(m, 1);
        bdmf_boolean do_manual_stop=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_cfg_set( %u %u %u %u)\n", counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        if(!err) ag_drv_ubus_slv_profiling_cfg_set(counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        if(!err) ag_drv_ubus_slv_profiling_cfg_get( &counter_enable, &profiling_start, &manual_stop_mode, &do_manual_stop);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_cfg_get( %u %u %u %u)\n", counter_enable, profiling_start, manual_stop_mode, do_manual_stop);
        if(err || counter_enable!=gtmv(m, 1) || profiling_start!=gtmv(m, 1) || manual_stop_mode!=gtmv(m, 1) || do_manual_stop!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean profiling_on=gtmv(m, 1);
        uint32_t cycles_counter=gtmv(m, 31);
        if(!err) ag_drv_ubus_slv_profiling_status_get( &profiling_on, &cycles_counter);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_status_get( %u %u)\n", profiling_on, cycles_counter);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_ubus_slv_profiling_counter_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_counter_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_ubus_slv_profiling_start_value_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_start_value_get( %u)\n", val);
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_ubus_slv_profiling_stop_value_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_stop_value_get( %u)\n", val);
    }
    {
        uint32_t profiling_cycles_num=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_cycle_num_set( %u)\n", profiling_cycles_num);
        if(!err) ag_drv_ubus_slv_profiling_cycle_num_set(profiling_cycles_num);
        if(!err) ag_drv_ubus_slv_profiling_cycle_num_get( &profiling_cycles_num);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_profiling_cycle_num_get( %u)\n", profiling_cycles_num);
        if(err || profiling_cycles_num!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_ubus_slv_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_vpb_base : reg = &RU_REG(UBUS_SLV, VPB_BASE); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_vpb_mask : reg = &RU_REG(UBUS_SLV, VPB_MASK); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_apb_base : reg = &RU_REG(UBUS_SLV, APB_BASE); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_apb_mask : reg = &RU_REG(UBUS_SLV, APB_MASK); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_dqm_base : reg = &RU_REG(UBUS_SLV, DQM_BASE); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_dqm_mask : reg = &RU_REG(UBUS_SLV, DQM_MASK); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_rnr_intr_ctrl_isr : reg = &RU_REG(UBUS_SLV, RNR_INTR_CTRL_ISR); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_rnr_intr_ctrl_ism : reg = &RU_REG(UBUS_SLV, RNR_INTR_CTRL_ISM); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_rnr_intr_ctrl_ier : reg = &RU_REG(UBUS_SLV, RNR_INTR_CTRL_IER); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_rnr_intr_ctrl_itr : reg = &RU_REG(UBUS_SLV, RNR_INTR_CTRL_ITR); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_cfg : reg = &RU_REG(UBUS_SLV, PROFILING_CFG); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_status : reg = &RU_REG(UBUS_SLV, PROFILING_STATUS); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_counter : reg = &RU_REG(UBUS_SLV, PROFILING_COUNTER); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_start_value : reg = &RU_REG(UBUS_SLV, PROFILING_START_VALUE); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_stop_value : reg = &RU_REG(UBUS_SLV, PROFILING_STOP_VALUE); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_profiling_cycle_num : reg = &RU_REG(UBUS_SLV, PROFILING_CYCLE_NUM); blk = &RU_BLK(UBUS_SLV); break;
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

bdmfmon_handle_t ag_drv_ubus_slv_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "ubus_slv"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "ubus_slv", "ubus_slv", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_vpb_base[]={
            BDMFMON_MAKE_PARM("base", "base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_vpb_mask[]={
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_apb_base[]={
            BDMFMON_MAKE_PARM("base", "base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_apb_mask[]={
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqm_base[]={
            BDMFMON_MAKE_PARM("base", "base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqm_mask[]={
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_isr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_ier[]={
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_intr_ctrl_itr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiling_cfg[]={
            BDMFMON_MAKE_PARM("counter_enable", "counter_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("profiling_start", "profiling_start", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("manual_stop_mode", "manual_stop_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("do_manual_stop", "do_manual_stop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiling_cycle_num[]={
            BDMFMON_MAKE_PARM("profiling_cycles_num", "profiling_cycles_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="vpb_base", .val=cli_ubus_slv_vpb_base, .parms=set_vpb_base },
            { .name="vpb_mask", .val=cli_ubus_slv_vpb_mask, .parms=set_vpb_mask },
            { .name="apb_base", .val=cli_ubus_slv_apb_base, .parms=set_apb_base },
            { .name="apb_mask", .val=cli_ubus_slv_apb_mask, .parms=set_apb_mask },
            { .name="dqm_base", .val=cli_ubus_slv_dqm_base, .parms=set_dqm_base },
            { .name="dqm_mask", .val=cli_ubus_slv_dqm_mask, .parms=set_dqm_mask },
            { .name="rnr_intr_ctrl_isr", .val=cli_ubus_slv_rnr_intr_ctrl_isr, .parms=set_rnr_intr_ctrl_isr },
            { .name="rnr_intr_ctrl_ier", .val=cli_ubus_slv_rnr_intr_ctrl_ier, .parms=set_rnr_intr_ctrl_ier },
            { .name="rnr_intr_ctrl_itr", .val=cli_ubus_slv_rnr_intr_ctrl_itr, .parms=set_rnr_intr_ctrl_itr },
            { .name="profiling_cfg", .val=cli_ubus_slv_profiling_cfg, .parms=set_profiling_cfg },
            { .name="profiling_cycle_num", .val=cli_ubus_slv_profiling_cycle_num, .parms=set_profiling_cycle_num },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_ubus_slv_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="vpb_base", .val=cli_ubus_slv_vpb_base, .parms=set_default },
            { .name="vpb_mask", .val=cli_ubus_slv_vpb_mask, .parms=set_default },
            { .name="apb_base", .val=cli_ubus_slv_apb_base, .parms=set_default },
            { .name="apb_mask", .val=cli_ubus_slv_apb_mask, .parms=set_default },
            { .name="dqm_base", .val=cli_ubus_slv_dqm_base, .parms=set_default },
            { .name="dqm_mask", .val=cli_ubus_slv_dqm_mask, .parms=set_default },
            { .name="rnr_intr_ctrl_isr", .val=cli_ubus_slv_rnr_intr_ctrl_isr, .parms=set_default },
            { .name="rnr_intr_ctrl_ism", .val=cli_ubus_slv_rnr_intr_ctrl_ism, .parms=set_default },
            { .name="rnr_intr_ctrl_ier", .val=cli_ubus_slv_rnr_intr_ctrl_ier, .parms=set_default },
            { .name="rnr_intr_ctrl_itr", .val=cli_ubus_slv_rnr_intr_ctrl_itr, .parms=set_default },
            { .name="profiling_cfg", .val=cli_ubus_slv_profiling_cfg, .parms=set_default },
            { .name="profiling_status", .val=cli_ubus_slv_profiling_status, .parms=set_default },
            { .name="profiling_counter", .val=cli_ubus_slv_profiling_counter, .parms=set_default },
            { .name="profiling_start_value", .val=cli_ubus_slv_profiling_start_value, .parms=set_default },
            { .name="profiling_stop_value", .val=cli_ubus_slv_profiling_stop_value, .parms=set_default },
            { .name="profiling_cycle_num", .val=cli_ubus_slv_profiling_cycle_num, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_ubus_slv_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_ubus_slv_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="VPB_BASE" , .val=bdmf_address_vpb_base },
            { .name="VPB_MASK" , .val=bdmf_address_vpb_mask },
            { .name="APB_BASE" , .val=bdmf_address_apb_base },
            { .name="APB_MASK" , .val=bdmf_address_apb_mask },
            { .name="DQM_BASE" , .val=bdmf_address_dqm_base },
            { .name="DQM_MASK" , .val=bdmf_address_dqm_mask },
            { .name="RNR_INTR_CTRL_ISR" , .val=bdmf_address_rnr_intr_ctrl_isr },
            { .name="RNR_INTR_CTRL_ISM" , .val=bdmf_address_rnr_intr_ctrl_ism },
            { .name="RNR_INTR_CTRL_IER" , .val=bdmf_address_rnr_intr_ctrl_ier },
            { .name="RNR_INTR_CTRL_ITR" , .val=bdmf_address_rnr_intr_ctrl_itr },
            { .name="PROFILING_CFG" , .val=bdmf_address_profiling_cfg },
            { .name="PROFILING_STATUS" , .val=bdmf_address_profiling_status },
            { .name="PROFILING_COUNTER" , .val=bdmf_address_profiling_counter },
            { .name="PROFILING_START_VALUE" , .val=bdmf_address_profiling_start_value },
            { .name="PROFILING_STOP_VALUE" , .val=bdmf_address_profiling_stop_value },
            { .name="PROFILING_CYCLE_NUM" , .val=bdmf_address_profiling_cycle_num },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_ubus_slv_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

