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

bdmf_error_t ag_drv_ubus_slv_led_cntrl_set(const ubus_slv_led_cntrl *led_cntrl)
{
    uint32_t reg_led_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((led_cntrl->rx_act_en >= _1BITS_MAX_VAL_) ||
       (led_cntrl->tx_act_en >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led0_act_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led1_act_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led2_act_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->act_led_act_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led0_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led1_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spdlnk_led2_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->act_led_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_cntrl->led_spd_ovrd >= _3BITS_MAX_VAL_) ||
       (led_cntrl->lnk_status_ovrd >= _1BITS_MAX_VAL_) ||
       (led_cntrl->spd_ovrd_en >= _1BITS_MAX_VAL_) ||
       (led_cntrl->lnk_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, RX_ACT_EN, reg_led_cntrl, led_cntrl->rx_act_en);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, TX_ACT_EN, reg_led_cntrl, led_cntrl->tx_act_en);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_cntrl, led_cntrl->spdlnk_led0_act_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_cntrl, led_cntrl->spdlnk_led1_act_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_cntrl, led_cntrl->spdlnk_led2_act_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, ACT_LED_ACT_SEL, reg_led_cntrl, led_cntrl->act_led_act_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_cntrl, led_cntrl->spdlnk_led0_act_pol_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_cntrl, led_cntrl->spdlnk_led1_act_pol_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_cntrl, led_cntrl->spdlnk_led2_act_pol_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, ACT_LED_POL_SEL, reg_led_cntrl, led_cntrl->act_led_pol_sel);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, LED_SPD_OVRD, reg_led_cntrl, led_cntrl->led_spd_ovrd);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, LNK_STATUS_OVRD, reg_led_cntrl, led_cntrl->lnk_status_ovrd);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, SPD_OVRD_EN, reg_led_cntrl, led_cntrl->spd_ovrd_en);
    reg_led_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_CNTRL, LNK_OVRD_EN, reg_led_cntrl, led_cntrl->lnk_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, LED_CNTRL, reg_led_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_cntrl_get(ubus_slv_led_cntrl *led_cntrl)
{
    uint32_t reg_led_cntrl;

#ifdef VALIDATE_PARMS
    if(!led_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, LED_CNTRL, reg_led_cntrl);

    led_cntrl->rx_act_en = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, RX_ACT_EN, reg_led_cntrl);
    led_cntrl->tx_act_en = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, TX_ACT_EN, reg_led_cntrl);
    led_cntrl->spdlnk_led0_act_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_cntrl);
    led_cntrl->spdlnk_led1_act_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_cntrl);
    led_cntrl->spdlnk_led2_act_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_cntrl);
    led_cntrl->act_led_act_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, ACT_LED_ACT_SEL, reg_led_cntrl);
    led_cntrl->spdlnk_led0_act_pol_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_cntrl);
    led_cntrl->spdlnk_led1_act_pol_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_cntrl);
    led_cntrl->spdlnk_led2_act_pol_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_cntrl);
    led_cntrl->act_led_pol_sel = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, ACT_LED_POL_SEL, reg_led_cntrl);
    led_cntrl->led_spd_ovrd = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, LED_SPD_OVRD, reg_led_cntrl);
    led_cntrl->lnk_status_ovrd = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, LNK_STATUS_OVRD, reg_led_cntrl);
    led_cntrl->spd_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, SPD_OVRD_EN, reg_led_cntrl);
    led_cntrl->lnk_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, LED_CNTRL, LNK_OVRD_EN, reg_led_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_link_and_speed_encoding_sel_set(const ubus_slv_led_link_and_speed_encoding_sel *led_link_and_speed_encoding_sel)
{
    uint32_t reg_led_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!led_link_and_speed_encoding_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((led_link_and_speed_encoding_sel->sel_no_link_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding_sel->sel_10m_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding_sel->sel_100m_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding_sel->sel_1000m_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding_sel->sel_2500m_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding_sel->sel_10g_encode >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_no_link_encode);
    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_10m_encode);
    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_100m_encode);
    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_1000m_encode);
    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_2500m_encode);
    reg_led_link_and_speed_encoding_sel = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_link_and_speed_encoding_sel, led_link_and_speed_encoding_sel->sel_10g_encode);

    RU_REG_WRITE(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, reg_led_link_and_speed_encoding_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_link_and_speed_encoding_sel_get(ubus_slv_led_link_and_speed_encoding_sel *led_link_and_speed_encoding_sel)
{
    uint32_t reg_led_link_and_speed_encoding_sel;

#ifdef VALIDATE_PARMS
    if(!led_link_and_speed_encoding_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, reg_led_link_and_speed_encoding_sel);

    led_link_and_speed_encoding_sel->sel_no_link_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_link_and_speed_encoding_sel);
    led_link_and_speed_encoding_sel->sel_10m_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_link_and_speed_encoding_sel);
    led_link_and_speed_encoding_sel->sel_100m_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_link_and_speed_encoding_sel);
    led_link_and_speed_encoding_sel->sel_1000m_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_link_and_speed_encoding_sel);
    led_link_and_speed_encoding_sel->sel_2500m_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_link_and_speed_encoding_sel);
    led_link_and_speed_encoding_sel->sel_10g_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_link_and_speed_encoding_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_link_and_speed_encoding_set(const ubus_slv_led_link_and_speed_encoding *led_link_and_speed_encoding)
{
    uint32_t reg_led_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!led_link_and_speed_encoding)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((led_link_and_speed_encoding->no_link_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding->m10_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding->m100_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding->m1000_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding->m2500_encode >= _3BITS_MAX_VAL_) ||
       (led_link_and_speed_encoding->m10g_encode >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->no_link_encode);
    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->m10_encode);
    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->m100_encode);
    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->m1000_encode);
    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->m2500_encode);
    reg_led_link_and_speed_encoding = RU_FIELD_SET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_link_and_speed_encoding, led_link_and_speed_encoding->m10g_encode);

    RU_REG_WRITE(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, reg_led_link_and_speed_encoding);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_link_and_speed_encoding_get(ubus_slv_led_link_and_speed_encoding *led_link_and_speed_encoding)
{
    uint32_t reg_led_link_and_speed_encoding;

#ifdef VALIDATE_PARMS
    if(!led_link_and_speed_encoding)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, reg_led_link_and_speed_encoding);

    led_link_and_speed_encoding->no_link_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_link_and_speed_encoding);
    led_link_and_speed_encoding->m10_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_link_and_speed_encoding);
    led_link_and_speed_encoding->m100_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_link_and_speed_encoding);
    led_link_and_speed_encoding->m1000_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_link_and_speed_encoding);
    led_link_and_speed_encoding->m2500_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_link_and_speed_encoding);
    led_link_and_speed_encoding->m10g_encode = RU_FIELD_GET(0, UBUS_SLV, LED_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_link_and_speed_encoding);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_blink_rate_cntrl_set(uint16_t led_off_time, uint16_t led_on_time)
{
    uint32_t reg_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl, led_off_time);
    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl, led_on_time);

    RU_REG_WRITE(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv_led_blink_rate_cntrl_get(uint16_t *led_off_time, uint16_t *led_on_time)
{
    uint32_t reg_led_blink_rate_cntrl;

#ifdef VALIDATE_PARMS
    if(!led_off_time || !led_on_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    *led_off_time = RU_FIELD_GET(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl);
    *led_on_time = RU_FIELD_GET(0, UBUS_SLV, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__cntrl_set(const ubus_slv__cntrl *_cntrl)
{
    uint32_t reg__cntrl=0;

#ifdef VALIDATE_PARMS
    if(!_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_cntrl->rgmii_mode_en >= _1BITS_MAX_VAL_) ||
       (_cntrl->id_mode_dis >= _1BITS_MAX_VAL_) ||
       (_cntrl->port_mode >= _3BITS_MAX_VAL_) ||
       (_cntrl->rvmii_ref_sel >= _1BITS_MAX_VAL_) ||
       (_cntrl->rx_pause_en >= _1BITS_MAX_VAL_) ||
       (_cntrl->tx_pause_en >= _1BITS_MAX_VAL_) ||
       (_cntrl->tx_clk_stop_en >= _1BITS_MAX_VAL_) ||
       (_cntrl->lpi_count >= _5BITS_MAX_VAL_) ||
       (_cntrl->rx_err_mask >= _1BITS_MAX_VAL_) ||
       (_cntrl->col_crs_mask >= _1BITS_MAX_VAL_) ||
       (_cntrl->pseudo_hd_mode_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, RGMII_MODE_EN, reg__cntrl, _cntrl->rgmii_mode_en);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, ID_MODE_DIS, reg__cntrl, _cntrl->id_mode_dis);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, PORT_MODE, reg__cntrl, _cntrl->port_mode);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, RVMII_REF_SEL, reg__cntrl, _cntrl->rvmii_ref_sel);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, RX_PAUSE_EN, reg__cntrl, _cntrl->rx_pause_en);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, TX_PAUSE_EN, reg__cntrl, _cntrl->tx_pause_en);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, TX_CLK_STOP_EN, reg__cntrl, _cntrl->tx_clk_stop_en);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, LPI_COUNT, reg__cntrl, _cntrl->lpi_count);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, RX_ERR_MASK, reg__cntrl, _cntrl->rx_err_mask);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, COL_CRS_MASK, reg__cntrl, _cntrl->col_crs_mask);
    reg__cntrl = RU_FIELD_SET(0, UBUS_SLV, _CNTRL, PSEUDO_HD_MODE_EN, reg__cntrl, _cntrl->pseudo_hd_mode_en);

    RU_REG_WRITE(0, UBUS_SLV, _CNTRL, reg__cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__cntrl_get(ubus_slv__cntrl *_cntrl)
{
    uint32_t reg__cntrl;

#ifdef VALIDATE_PARMS
    if(!_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _CNTRL, reg__cntrl);

    _cntrl->rgmii_mode_en = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, RGMII_MODE_EN, reg__cntrl);
    _cntrl->id_mode_dis = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, ID_MODE_DIS, reg__cntrl);
    _cntrl->port_mode = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, PORT_MODE, reg__cntrl);
    _cntrl->rvmii_ref_sel = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, RVMII_REF_SEL, reg__cntrl);
    _cntrl->rx_pause_en = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, RX_PAUSE_EN, reg__cntrl);
    _cntrl->tx_pause_en = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, TX_PAUSE_EN, reg__cntrl);
    _cntrl->tx_clk_stop_en = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, TX_CLK_STOP_EN, reg__cntrl);
    _cntrl->lpi_count = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, LPI_COUNT, reg__cntrl);
    _cntrl->rx_err_mask = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, RX_ERR_MASK, reg__cntrl);
    _cntrl->col_crs_mask = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, COL_CRS_MASK, reg__cntrl);
    _cntrl->pseudo_hd_mode_en = RU_FIELD_GET(0, UBUS_SLV, _CNTRL, PSEUDO_HD_MODE_EN, reg__cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ib_status_set(uint8_t speed_decode, bdmf_boolean duplex_decode, bdmf_boolean link_decode, bdmf_boolean ib_status_ovrd)
{
    uint32_t reg__ib_status=0;

#ifdef VALIDATE_PARMS
    if((speed_decode >= _2BITS_MAX_VAL_) ||
       (duplex_decode >= _1BITS_MAX_VAL_) ||
       (link_decode >= _1BITS_MAX_VAL_) ||
       (ib_status_ovrd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ib_status = RU_FIELD_SET(0, UBUS_SLV, _IB_STATUS, SPEED_DECODE, reg__ib_status, speed_decode);
    reg__ib_status = RU_FIELD_SET(0, UBUS_SLV, _IB_STATUS, DUPLEX_DECODE, reg__ib_status, duplex_decode);
    reg__ib_status = RU_FIELD_SET(0, UBUS_SLV, _IB_STATUS, LINK_DECODE, reg__ib_status, link_decode);
    reg__ib_status = RU_FIELD_SET(0, UBUS_SLV, _IB_STATUS, IB_STATUS_OVRD, reg__ib_status, ib_status_ovrd);

    RU_REG_WRITE(0, UBUS_SLV, _IB_STATUS, reg__ib_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ib_status_get(uint8_t *speed_decode, bdmf_boolean *duplex_decode, bdmf_boolean *link_decode, bdmf_boolean *ib_status_ovrd)
{
    uint32_t reg__ib_status;

#ifdef VALIDATE_PARMS
    if(!speed_decode || !duplex_decode || !link_decode || !ib_status_ovrd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _IB_STATUS, reg__ib_status);

    *speed_decode = RU_FIELD_GET(0, UBUS_SLV, _IB_STATUS, SPEED_DECODE, reg__ib_status);
    *duplex_decode = RU_FIELD_GET(0, UBUS_SLV, _IB_STATUS, DUPLEX_DECODE, reg__ib_status);
    *link_decode = RU_FIELD_GET(0, UBUS_SLV, _IB_STATUS, LINK_DECODE, reg__ib_status);
    *ib_status_ovrd = RU_FIELD_GET(0, UBUS_SLV, _IB_STATUS, IB_STATUS_OVRD, reg__ib_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_clock_delay_cntrl_set(const ubus_slv__rx_clock_delay_cntrl *_rx_clock_delay_cntrl)
{
    uint32_t reg__rx_clock_delay_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!_rx_clock_delay_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_rx_clock_delay_cntrl->ctri >= _2BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->drng >= _2BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->iddq >= _1BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->bypass >= _1BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->dly_sel >= _1BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->dly_override >= _1BITS_MAX_VAL_) ||
       (_rx_clock_delay_cntrl->reset >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, CTRI, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->ctri);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DRNG, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->drng);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, IDDQ, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->iddq);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, BYPASS, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->bypass);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DLY_SEL, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->dly_sel);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DLY_OVERRIDE, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->dly_override);
    reg__rx_clock_delay_cntrl = RU_FIELD_SET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, RESET, reg__rx_clock_delay_cntrl, _rx_clock_delay_cntrl->reset);

    RU_REG_WRITE(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, reg__rx_clock_delay_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_clock_delay_cntrl_get(ubus_slv__rx_clock_delay_cntrl *_rx_clock_delay_cntrl)
{
    uint32_t reg__rx_clock_delay_cntrl;

#ifdef VALIDATE_PARMS
    if(!_rx_clock_delay_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, reg__rx_clock_delay_cntrl);

    _rx_clock_delay_cntrl->ctri = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, CTRI, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->drng = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DRNG, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->iddq = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, IDDQ, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->bypass = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, BYPASS, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->dly_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DLY_SEL, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->dly_override = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, DLY_OVERRIDE, reg__rx_clock_delay_cntrl);
    _rx_clock_delay_cntrl->reset = RU_FIELD_GET(0, UBUS_SLV, _RX_CLOCK_DELAY_CNTRL, RESET, reg__rx_clock_delay_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_cntrl_exp_data_set(const ubus_slv__ate_rx_cntrl_exp_data *_ate_rx_cntrl_exp_data)
{
    uint32_t reg__ate_rx_cntrl_exp_data=0;

#ifdef VALIDATE_PARMS
    if(!_ate_rx_cntrl_exp_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_ate_rx_cntrl_exp_data->expected_data_0 >= _9BITS_MAX_VAL_) ||
       (_ate_rx_cntrl_exp_data->expected_data_1 >= _9BITS_MAX_VAL_) ||
       (_ate_rx_cntrl_exp_data->pkt_count_rst >= _1BITS_MAX_VAL_) ||
       (_ate_rx_cntrl_exp_data->ate_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ate_rx_cntrl_exp_data = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_0, reg__ate_rx_cntrl_exp_data, _ate_rx_cntrl_exp_data->expected_data_0);
    reg__ate_rx_cntrl_exp_data = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_1, reg__ate_rx_cntrl_exp_data, _ate_rx_cntrl_exp_data->expected_data_1);
    reg__ate_rx_cntrl_exp_data = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, GOOD_COUNT, reg__ate_rx_cntrl_exp_data, _ate_rx_cntrl_exp_data->good_count);
    reg__ate_rx_cntrl_exp_data = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, PKT_COUNT_RST, reg__ate_rx_cntrl_exp_data, _ate_rx_cntrl_exp_data->pkt_count_rst);
    reg__ate_rx_cntrl_exp_data = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, ATE_EN, reg__ate_rx_cntrl_exp_data, _ate_rx_cntrl_exp_data->ate_en);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, reg__ate_rx_cntrl_exp_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_cntrl_exp_data_get(ubus_slv__ate_rx_cntrl_exp_data *_ate_rx_cntrl_exp_data)
{
    uint32_t reg__ate_rx_cntrl_exp_data;

#ifdef VALIDATE_PARMS
    if(!_ate_rx_cntrl_exp_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, reg__ate_rx_cntrl_exp_data);

    _ate_rx_cntrl_exp_data->expected_data_0 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_0, reg__ate_rx_cntrl_exp_data);
    _ate_rx_cntrl_exp_data->expected_data_1 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_1, reg__ate_rx_cntrl_exp_data);
    _ate_rx_cntrl_exp_data->good_count = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, GOOD_COUNT, reg__ate_rx_cntrl_exp_data);
    _ate_rx_cntrl_exp_data->pkt_count_rst = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, PKT_COUNT_RST, reg__ate_rx_cntrl_exp_data);
    _ate_rx_cntrl_exp_data->ate_en = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA, ATE_EN, reg__ate_rx_cntrl_exp_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_exp_data_1_set(uint16_t expected_data_2, uint16_t expected_data_3)
{
    uint32_t reg__ate_rx_exp_data_1=0;

#ifdef VALIDATE_PARMS
    if((expected_data_2 >= _9BITS_MAX_VAL_) ||
       (expected_data_3 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ate_rx_exp_data_1 = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, EXPECTED_DATA_2, reg__ate_rx_exp_data_1, expected_data_2);
    reg__ate_rx_exp_data_1 = RU_FIELD_SET(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, EXPECTED_DATA_3, reg__ate_rx_exp_data_1, expected_data_3);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, reg__ate_rx_exp_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_exp_data_1_get(uint16_t *expected_data_2, uint16_t *expected_data_3)
{
    uint32_t reg__ate_rx_exp_data_1;

#ifdef VALIDATE_PARMS
    if(!expected_data_2 || !expected_data_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, reg__ate_rx_exp_data_1);

    *expected_data_2 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, EXPECTED_DATA_2, reg__ate_rx_exp_data_1);
    *expected_data_3 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_EXP_DATA_1, EXPECTED_DATA_3, reg__ate_rx_exp_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_status_0_get(uint16_t *received_data_0, uint16_t *received_data_1, bdmf_boolean *rx_ok)
{
    uint32_t reg__ate_rx_status_0;

#ifdef VALIDATE_PARMS
    if(!received_data_0 || !received_data_1 || !rx_ok)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_RX_STATUS_0, reg__ate_rx_status_0);

    *received_data_0 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_STATUS_0, RECEIVED_DATA_0, reg__ate_rx_status_0);
    *received_data_1 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_STATUS_0, RECEIVED_DATA_1, reg__ate_rx_status_0);
    *rx_ok = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_STATUS_0, RX_OK, reg__ate_rx_status_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_rx_status_1_get(uint16_t *received_data_2, uint16_t *received_data_3)
{
    uint32_t reg__ate_rx_status_1;

#ifdef VALIDATE_PARMS
    if(!received_data_2 || !received_data_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_RX_STATUS_1, reg__ate_rx_status_1);

    *received_data_2 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_STATUS_1, RECEIVED_DATA_2, reg__ate_rx_status_1);
    *received_data_3 = RU_FIELD_GET(0, UBUS_SLV, _ATE_RX_STATUS_1, RECEIVED_DATA_3, reg__ate_rx_status_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_cntrl_set(const ubus_slv__ate_tx_cntrl *_ate_tx_cntrl)
{
    uint32_t reg__ate_tx_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!_ate_tx_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_ate_tx_cntrl->start_stop_ovrd >= _1BITS_MAX_VAL_) ||
       (_ate_tx_cntrl->start_stop >= _1BITS_MAX_VAL_) ||
       (_ate_tx_cntrl->pkt_gen_en >= _1BITS_MAX_VAL_) ||
       (_ate_tx_cntrl->payload_length >= _11BITS_MAX_VAL_) ||
       (_ate_tx_cntrl->pkt_ipg >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, START_STOP_OVRD, reg__ate_tx_cntrl, _ate_tx_cntrl->start_stop_ovrd);
    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, START_STOP, reg__ate_tx_cntrl, _ate_tx_cntrl->start_stop);
    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_GEN_EN, reg__ate_tx_cntrl, _ate_tx_cntrl->pkt_gen_en);
    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_CNT, reg__ate_tx_cntrl, _ate_tx_cntrl->pkt_cnt);
    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, PAYLOAD_LENGTH, reg__ate_tx_cntrl, _ate_tx_cntrl->payload_length);
    reg__ate_tx_cntrl = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_IPG, reg__ate_tx_cntrl, _ate_tx_cntrl->pkt_ipg);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_TX_CNTRL, reg__ate_tx_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_cntrl_get(ubus_slv__ate_tx_cntrl *_ate_tx_cntrl)
{
    uint32_t reg__ate_tx_cntrl;

#ifdef VALIDATE_PARMS
    if(!_ate_tx_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_TX_CNTRL, reg__ate_tx_cntrl);

    _ate_tx_cntrl->start_stop_ovrd = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, START_STOP_OVRD, reg__ate_tx_cntrl);
    _ate_tx_cntrl->start_stop = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, START_STOP, reg__ate_tx_cntrl);
    _ate_tx_cntrl->pkt_gen_en = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_GEN_EN, reg__ate_tx_cntrl);
    _ate_tx_cntrl->pkt_cnt = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_CNT, reg__ate_tx_cntrl);
    _ate_tx_cntrl->payload_length = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, PAYLOAD_LENGTH, reg__ate_tx_cntrl);
    _ate_tx_cntrl->pkt_ipg = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_CNTRL, PKT_IPG, reg__ate_tx_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_0_set(uint16_t tx_data_0, uint16_t tx_data_1)
{
    uint32_t reg__ate_tx_data_0=0;

#ifdef VALIDATE_PARMS
    if((tx_data_0 >= _9BITS_MAX_VAL_) ||
       (tx_data_1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ate_tx_data_0 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_0, TX_DATA_0, reg__ate_tx_data_0, tx_data_0);
    reg__ate_tx_data_0 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_0, TX_DATA_1, reg__ate_tx_data_0, tx_data_1);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_TX_DATA_0, reg__ate_tx_data_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_0_get(uint16_t *tx_data_0, uint16_t *tx_data_1)
{
    uint32_t reg__ate_tx_data_0;

#ifdef VALIDATE_PARMS
    if(!tx_data_0 || !tx_data_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_TX_DATA_0, reg__ate_tx_data_0);

    *tx_data_0 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_0, TX_DATA_0, reg__ate_tx_data_0);
    *tx_data_1 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_0, TX_DATA_1, reg__ate_tx_data_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_1_set(uint16_t tx_data_2, uint16_t tx_data_3)
{
    uint32_t reg__ate_tx_data_1=0;

#ifdef VALIDATE_PARMS
    if((tx_data_2 >= _9BITS_MAX_VAL_) ||
       (tx_data_3 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__ate_tx_data_1 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_1, TX_DATA_2, reg__ate_tx_data_1, tx_data_2);
    reg__ate_tx_data_1 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_1, TX_DATA_3, reg__ate_tx_data_1, tx_data_3);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_TX_DATA_1, reg__ate_tx_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_1_get(uint16_t *tx_data_2, uint16_t *tx_data_3)
{
    uint32_t reg__ate_tx_data_1;

#ifdef VALIDATE_PARMS
    if(!tx_data_2 || !tx_data_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_TX_DATA_1, reg__ate_tx_data_1);

    *tx_data_2 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_1, TX_DATA_2, reg__ate_tx_data_1);
    *tx_data_3 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_1, TX_DATA_3, reg__ate_tx_data_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_2_set(uint8_t tx_data_4, uint8_t tx_data_5, uint16_t ether_type)
{
    uint32_t reg__ate_tx_data_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg__ate_tx_data_2 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_2, TX_DATA_4, reg__ate_tx_data_2, tx_data_4);
    reg__ate_tx_data_2 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_2, TX_DATA_5, reg__ate_tx_data_2, tx_data_5);
    reg__ate_tx_data_2 = RU_FIELD_SET(0, UBUS_SLV, _ATE_TX_DATA_2, ETHER_TYPE, reg__ate_tx_data_2, ether_type);

    RU_REG_WRITE(0, UBUS_SLV, _ATE_TX_DATA_2, reg__ate_tx_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__ate_tx_data_2_get(uint8_t *tx_data_4, uint8_t *tx_data_5, uint16_t *ether_type)
{
    uint32_t reg__ate_tx_data_2;

#ifdef VALIDATE_PARMS
    if(!tx_data_4 || !tx_data_5 || !ether_type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _ATE_TX_DATA_2, reg__ate_tx_data_2);

    *tx_data_4 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_2, TX_DATA_4, reg__ate_tx_data_2);
    *tx_data_5 = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_2, TX_DATA_5, reg__ate_tx_data_2);
    *ether_type = RU_FIELD_GET(0, UBUS_SLV, _ATE_TX_DATA_2, ETHER_TYPE, reg__ate_tx_data_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_0_set(const ubus_slv__tx_delay_cntrl_0 *_tx_delay_cntrl_0)
{
    uint32_t reg__tx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!_tx_delay_cntrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_tx_delay_cntrl_0->txd0_del_sel >= _6BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd0_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd1_del_sel >= _6BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd1_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd2_del_sel >= _6BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd2_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd3_del_sel >= _6BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_0->txd3_del_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD0_DEL_SEL, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd0_del_sel);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD0_DEL_OVRD_EN, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd0_del_ovrd_en);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD1_DEL_SEL, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd1_del_sel);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD1_DEL_OVRD_EN, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd1_del_ovrd_en);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD2_DEL_SEL, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd2_del_sel);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD2_DEL_OVRD_EN, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd2_del_ovrd_en);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD3_DEL_SEL, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd3_del_sel);
    reg__tx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD3_DEL_OVRD_EN, reg__tx_delay_cntrl_0, _tx_delay_cntrl_0->txd3_del_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, _TX_DELAY_CNTRL_0, reg__tx_delay_cntrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_0_get(ubus_slv__tx_delay_cntrl_0 *_tx_delay_cntrl_0)
{
    uint32_t reg__tx_delay_cntrl_0;

#ifdef VALIDATE_PARMS
    if(!_tx_delay_cntrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _TX_DELAY_CNTRL_0, reg__tx_delay_cntrl_0);

    _tx_delay_cntrl_0->txd0_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD0_DEL_SEL, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd0_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD0_DEL_OVRD_EN, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd1_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD1_DEL_SEL, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd1_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD1_DEL_OVRD_EN, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd2_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD2_DEL_SEL, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd2_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD2_DEL_OVRD_EN, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd3_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD3_DEL_SEL, reg__tx_delay_cntrl_0);
    _tx_delay_cntrl_0->txd3_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_0, TXD3_DEL_OVRD_EN, reg__tx_delay_cntrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_1_set(const ubus_slv__tx_delay_cntrl_1 *_tx_delay_cntrl_1)
{
    uint32_t reg__tx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!_tx_delay_cntrl_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_tx_delay_cntrl_1->txctl_del_sel >= _6BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_1->txctl_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_1->txclk_del_sel >= _4BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_1->txclk_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_1->txclk_id_del_sel >= _4BITS_MAX_VAL_) ||
       (_tx_delay_cntrl_1->txclk_id_del_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCTL_DEL_SEL, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txctl_del_sel);
    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCTL_DEL_OVRD_EN, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txctl_del_ovrd_en);
    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_DEL_SEL, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txclk_del_sel);
    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_DEL_OVRD_EN, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txclk_del_ovrd_en);
    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_ID_DEL_SEL, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txclk_id_del_sel);
    reg__tx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_ID_DEL_OVRD_EN, reg__tx_delay_cntrl_1, _tx_delay_cntrl_1->txclk_id_del_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, _TX_DELAY_CNTRL_1, reg__tx_delay_cntrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_1_get(ubus_slv__tx_delay_cntrl_1 *_tx_delay_cntrl_1)
{
    uint32_t reg__tx_delay_cntrl_1;

#ifdef VALIDATE_PARMS
    if(!_tx_delay_cntrl_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _TX_DELAY_CNTRL_1, reg__tx_delay_cntrl_1);

    _tx_delay_cntrl_1->txctl_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCTL_DEL_SEL, reg__tx_delay_cntrl_1);
    _tx_delay_cntrl_1->txctl_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCTL_DEL_OVRD_EN, reg__tx_delay_cntrl_1);
    _tx_delay_cntrl_1->txclk_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_DEL_SEL, reg__tx_delay_cntrl_1);
    _tx_delay_cntrl_1->txclk_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_DEL_OVRD_EN, reg__tx_delay_cntrl_1);
    _tx_delay_cntrl_1->txclk_id_del_sel = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_ID_DEL_SEL, reg__tx_delay_cntrl_1);
    _tx_delay_cntrl_1->txclk_id_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _TX_DELAY_CNTRL_1, TXCLK_ID_DEL_OVRD_EN, reg__tx_delay_cntrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_0_set(const ubus_slv__rx_delay_cntrl_0 *_rx_delay_cntrl_0)
{
    uint32_t reg__rx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_rx_delay_cntrl_0->rxd0_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd0_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd1_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd1_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd2_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd2_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd3_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_0->rxd3_del_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD0_DEL_SEL, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd0_del_sel);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD0_DEL_OVRD_EN, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd0_del_ovrd_en);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD1_DEL_SEL, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd1_del_sel);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD1_DEL_OVRD_EN, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd1_del_ovrd_en);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD2_DEL_SEL, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd2_del_sel);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD2_DEL_OVRD_EN, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd2_del_ovrd_en);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD3_DEL_SEL, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd3_del_sel);
    reg__rx_delay_cntrl_0 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD3_DEL_OVRD_EN, reg__rx_delay_cntrl_0, _rx_delay_cntrl_0->rxd3_del_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, _RX_DELAY_CNTRL_0, reg__rx_delay_cntrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_0_get(ubus_slv__rx_delay_cntrl_0 *_rx_delay_cntrl_0)
{
    uint32_t reg__rx_delay_cntrl_0;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _RX_DELAY_CNTRL_0, reg__rx_delay_cntrl_0);

    _rx_delay_cntrl_0->rxd0_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD0_DEL_SEL, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd0_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD0_DEL_OVRD_EN, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd1_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD1_DEL_SEL, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd1_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD1_DEL_OVRD_EN, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd2_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD2_DEL_SEL, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd2_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD2_DEL_OVRD_EN, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd3_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD3_DEL_SEL, reg__rx_delay_cntrl_0);
    _rx_delay_cntrl_0->rxd3_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_0, RXD3_DEL_OVRD_EN, reg__rx_delay_cntrl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_1_set(const ubus_slv__rx_delay_cntrl_1 *_rx_delay_cntrl_1)
{
    uint32_t reg__rx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_rx_delay_cntrl_1->rxd4_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd4_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd5_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd5_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd6_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd6_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd7_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_1->rxd7_del_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD4_DEL_SEL, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd4_del_sel);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD4_DEL_OVRD_EN, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd4_del_ovrd_en);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD5_DEL_SEL, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd5_del_sel);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD5_DEL_OVRD_EN, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd5_del_ovrd_en);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD6_DEL_SEL, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd6_del_sel);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD6_DEL_OVRD_EN, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd6_del_ovrd_en);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD7_DEL_SEL, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd7_del_sel);
    reg__rx_delay_cntrl_1 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD7_DEL_OVRD_EN, reg__rx_delay_cntrl_1, _rx_delay_cntrl_1->rxd7_del_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, _RX_DELAY_CNTRL_1, reg__rx_delay_cntrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_1_get(ubus_slv__rx_delay_cntrl_1 *_rx_delay_cntrl_1)
{
    uint32_t reg__rx_delay_cntrl_1;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _RX_DELAY_CNTRL_1, reg__rx_delay_cntrl_1);

    _rx_delay_cntrl_1->rxd4_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD4_DEL_SEL, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd4_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD4_DEL_OVRD_EN, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd5_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD5_DEL_SEL, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd5_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD5_DEL_OVRD_EN, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd6_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD6_DEL_SEL, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd6_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD6_DEL_OVRD_EN, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd7_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD7_DEL_SEL, reg__rx_delay_cntrl_1);
    _rx_delay_cntrl_1->rxd7_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_1, RXD7_DEL_OVRD_EN, reg__rx_delay_cntrl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_2_set(const ubus_slv__rx_delay_cntrl_2 *_rx_delay_cntrl_2)
{
    uint32_t reg__rx_delay_cntrl_2=0;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((_rx_delay_cntrl_2->rxctl_pos_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_2->rxctl_pos_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_2->rxctl_neg_del_sel >= _6BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_2->rxctl_neg_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_2->rxclk_del_sel >= _4BITS_MAX_VAL_) ||
       (_rx_delay_cntrl_2->rxclk_del_ovrd_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_POS_DEL_SEL, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxctl_pos_del_sel);
    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_POS_DEL_OVRD_EN, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxctl_pos_del_ovrd_en);
    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_SEL, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxctl_neg_del_sel);
    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_OVRD_EN, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxctl_neg_del_ovrd_en);
    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCLK_DEL_SEL, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxclk_del_sel);
    reg__rx_delay_cntrl_2 = RU_FIELD_SET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCLK_DEL_OVRD_EN, reg__rx_delay_cntrl_2, _rx_delay_cntrl_2->rxclk_del_ovrd_en);

    RU_REG_WRITE(0, UBUS_SLV, _RX_DELAY_CNTRL_2, reg__rx_delay_cntrl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_2_get(ubus_slv__rx_delay_cntrl_2 *_rx_delay_cntrl_2)
{
    uint32_t reg__rx_delay_cntrl_2;

#ifdef VALIDATE_PARMS
    if(!_rx_delay_cntrl_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _RX_DELAY_CNTRL_2, reg__rx_delay_cntrl_2);

    _rx_delay_cntrl_2->rxctl_pos_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_POS_DEL_SEL, reg__rx_delay_cntrl_2);
    _rx_delay_cntrl_2->rxctl_pos_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_POS_DEL_OVRD_EN, reg__rx_delay_cntrl_2);
    _rx_delay_cntrl_2->rxctl_neg_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_SEL, reg__rx_delay_cntrl_2);
    _rx_delay_cntrl_2->rxctl_neg_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_OVRD_EN, reg__rx_delay_cntrl_2);
    _rx_delay_cntrl_2->rxclk_del_sel = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCLK_DEL_SEL, reg__rx_delay_cntrl_2);
    _rx_delay_cntrl_2->rxclk_del_ovrd_en = RU_FIELD_GET(0, UBUS_SLV, _RX_DELAY_CNTRL_2, RXCLK_DEL_OVRD_EN, reg__rx_delay_cntrl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__clk_rst_ctrl_set(bdmf_boolean swinit, bdmf_boolean clk250en)
{
    uint32_t reg__clk_rst_ctrl=0;

#ifdef VALIDATE_PARMS
    if((swinit >= _1BITS_MAX_VAL_) ||
       (clk250en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg__clk_rst_ctrl = RU_FIELD_SET(0, UBUS_SLV, _CLK_RST_CTRL, SWINIT, reg__clk_rst_ctrl, swinit);
    reg__clk_rst_ctrl = RU_FIELD_SET(0, UBUS_SLV, _CLK_RST_CTRL, CLK250EN, reg__clk_rst_ctrl, clk250en);

    RU_REG_WRITE(0, UBUS_SLV, _CLK_RST_CTRL, reg__clk_rst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_slv__clk_rst_ctrl_get(bdmf_boolean *swinit, bdmf_boolean *clk250en)
{
    uint32_t reg__clk_rst_ctrl;

#ifdef VALIDATE_PARMS
    if(!swinit || !clk250en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, UBUS_SLV, _CLK_RST_CTRL, reg__clk_rst_ctrl);

    *swinit = RU_FIELD_GET(0, UBUS_SLV, _CLK_RST_CTRL, SWINIT, reg__clk_rst_ctrl);
    *clk250en = RU_FIELD_GET(0, UBUS_SLV, _CLK_RST_CTRL, CLK250EN, reg__clk_rst_ctrl);

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
    bdmf_address_led_cntrl,
    bdmf_address_led_link_and_speed_encoding_sel,
    bdmf_address_led_link_and_speed_encoding,
    bdmf_address_led_blink_rate_cntrl,
    bdmf_address__cntrl,
    bdmf_address__ib_status,
    bdmf_address__rx_clock_delay_cntrl,
    bdmf_address__ate_rx_cntrl_exp_data,
    bdmf_address__ate_rx_exp_data_1,
    bdmf_address__ate_rx_status_0,
    bdmf_address__ate_rx_status_1,
    bdmf_address__ate_tx_cntrl,
    bdmf_address__ate_tx_data_0,
    bdmf_address__ate_tx_data_1,
    bdmf_address__ate_tx_data_2,
    bdmf_address__tx_delay_cntrl_0,
    bdmf_address__tx_delay_cntrl_1,
    bdmf_address__rx_delay_cntrl_0,
    bdmf_address__rx_delay_cntrl_1,
    bdmf_address__rx_delay_cntrl_2,
    bdmf_address__clk_rst_ctrl,
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
    case cli_ubus_slv_led_cntrl:
    {
        ubus_slv_led_cntrl led_cntrl = { .rx_act_en=parm[1].value.unumber, .tx_act_en=parm[2].value.unumber, .spdlnk_led0_act_sel=parm[3].value.unumber, .spdlnk_led1_act_sel=parm[4].value.unumber, .spdlnk_led2_act_sel=parm[5].value.unumber, .act_led_act_sel=parm[6].value.unumber, .spdlnk_led0_act_pol_sel=parm[7].value.unumber, .spdlnk_led1_act_pol_sel=parm[8].value.unumber, .spdlnk_led2_act_pol_sel=parm[9].value.unumber, .act_led_pol_sel=parm[10].value.unumber, .led_spd_ovrd=parm[11].value.unumber, .lnk_status_ovrd=parm[12].value.unumber, .spd_ovrd_en=parm[13].value.unumber, .lnk_ovrd_en=parm[14].value.unumber};
        err = ag_drv_ubus_slv_led_cntrl_set(&led_cntrl);
        break;
    }
    case cli_ubus_slv_led_link_and_speed_encoding_sel:
    {
        ubus_slv_led_link_and_speed_encoding_sel led_link_and_speed_encoding_sel = { .sel_no_link_encode=parm[1].value.unumber, .sel_10m_encode=parm[2].value.unumber, .sel_100m_encode=parm[3].value.unumber, .sel_1000m_encode=parm[4].value.unumber, .sel_2500m_encode=parm[5].value.unumber, .sel_10g_encode=parm[6].value.unumber};
        err = ag_drv_ubus_slv_led_link_and_speed_encoding_sel_set(&led_link_and_speed_encoding_sel);
        break;
    }
    case cli_ubus_slv_led_link_and_speed_encoding:
    {
        ubus_slv_led_link_and_speed_encoding led_link_and_speed_encoding = { .no_link_encode=parm[1].value.unumber, .m10_encode=parm[2].value.unumber, .m100_encode=parm[3].value.unumber, .m1000_encode=parm[4].value.unumber, .m2500_encode=parm[5].value.unumber, .m10g_encode=parm[6].value.unumber};
        err = ag_drv_ubus_slv_led_link_and_speed_encoding_set(&led_link_and_speed_encoding);
        break;
    }
    case cli_ubus_slv_led_blink_rate_cntrl:
        err = ag_drv_ubus_slv_led_blink_rate_cntrl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_slv__cntrl:
    {
        ubus_slv__cntrl _cntrl = { .rgmii_mode_en=parm[1].value.unumber, .id_mode_dis=parm[2].value.unumber, .port_mode=parm[3].value.unumber, .rvmii_ref_sel=parm[4].value.unumber, .rx_pause_en=parm[5].value.unumber, .tx_pause_en=parm[6].value.unumber, .tx_clk_stop_en=parm[7].value.unumber, .lpi_count=parm[8].value.unumber, .rx_err_mask=parm[9].value.unumber, .col_crs_mask=parm[10].value.unumber, .pseudo_hd_mode_en=parm[11].value.unumber};
        err = ag_drv_ubus_slv__cntrl_set(&_cntrl);
        break;
    }
    case cli_ubus_slv__ib_status:
        err = ag_drv_ubus_slv__ib_status_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_ubus_slv__rx_clock_delay_cntrl:
    {
        ubus_slv__rx_clock_delay_cntrl _rx_clock_delay_cntrl = { .ctri=parm[1].value.unumber, .drng=parm[2].value.unumber, .iddq=parm[3].value.unumber, .bypass=parm[4].value.unumber, .dly_sel=parm[5].value.unumber, .dly_override=parm[6].value.unumber, .reset=parm[7].value.unumber};
        err = ag_drv_ubus_slv__rx_clock_delay_cntrl_set(&_rx_clock_delay_cntrl);
        break;
    }
    case cli_ubus_slv__ate_rx_cntrl_exp_data:
    {
        ubus_slv__ate_rx_cntrl_exp_data _ate_rx_cntrl_exp_data = { .expected_data_0=parm[1].value.unumber, .expected_data_1=parm[2].value.unumber, .good_count=parm[3].value.unumber, .pkt_count_rst=parm[4].value.unumber, .ate_en=parm[5].value.unumber};
        err = ag_drv_ubus_slv__ate_rx_cntrl_exp_data_set(&_ate_rx_cntrl_exp_data);
        break;
    }
    case cli_ubus_slv__ate_rx_exp_data_1:
        err = ag_drv_ubus_slv__ate_rx_exp_data_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_slv__ate_tx_cntrl:
    {
        ubus_slv__ate_tx_cntrl _ate_tx_cntrl = { .start_stop_ovrd=parm[1].value.unumber, .start_stop=parm[2].value.unumber, .pkt_gen_en=parm[3].value.unumber, .pkt_cnt=parm[4].value.unumber, .payload_length=parm[5].value.unumber, .pkt_ipg=parm[6].value.unumber};
        err = ag_drv_ubus_slv__ate_tx_cntrl_set(&_ate_tx_cntrl);
        break;
    }
    case cli_ubus_slv__ate_tx_data_0:
        err = ag_drv_ubus_slv__ate_tx_data_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_slv__ate_tx_data_1:
        err = ag_drv_ubus_slv__ate_tx_data_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_slv__ate_tx_data_2:
        err = ag_drv_ubus_slv__ate_tx_data_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_ubus_slv__tx_delay_cntrl_0:
    {
        ubus_slv__tx_delay_cntrl_0 _tx_delay_cntrl_0 = { .txd0_del_sel=parm[1].value.unumber, .txd0_del_ovrd_en=parm[2].value.unumber, .txd1_del_sel=parm[3].value.unumber, .txd1_del_ovrd_en=parm[4].value.unumber, .txd2_del_sel=parm[5].value.unumber, .txd2_del_ovrd_en=parm[6].value.unumber, .txd3_del_sel=parm[7].value.unumber, .txd3_del_ovrd_en=parm[8].value.unumber};
        err = ag_drv_ubus_slv__tx_delay_cntrl_0_set(&_tx_delay_cntrl_0);
        break;
    }
    case cli_ubus_slv__tx_delay_cntrl_1:
    {
        ubus_slv__tx_delay_cntrl_1 _tx_delay_cntrl_1 = { .txctl_del_sel=parm[1].value.unumber, .txctl_del_ovrd_en=parm[2].value.unumber, .txclk_del_sel=parm[3].value.unumber, .txclk_del_ovrd_en=parm[4].value.unumber, .txclk_id_del_sel=parm[5].value.unumber, .txclk_id_del_ovrd_en=parm[6].value.unumber};
        err = ag_drv_ubus_slv__tx_delay_cntrl_1_set(&_tx_delay_cntrl_1);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_0:
    {
        ubus_slv__rx_delay_cntrl_0 _rx_delay_cntrl_0 = { .rxd0_del_sel=parm[1].value.unumber, .rxd0_del_ovrd_en=parm[2].value.unumber, .rxd1_del_sel=parm[3].value.unumber, .rxd1_del_ovrd_en=parm[4].value.unumber, .rxd2_del_sel=parm[5].value.unumber, .rxd2_del_ovrd_en=parm[6].value.unumber, .rxd3_del_sel=parm[7].value.unumber, .rxd3_del_ovrd_en=parm[8].value.unumber};
        err = ag_drv_ubus_slv__rx_delay_cntrl_0_set(&_rx_delay_cntrl_0);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_1:
    {
        ubus_slv__rx_delay_cntrl_1 _rx_delay_cntrl_1 = { .rxd4_del_sel=parm[1].value.unumber, .rxd4_del_ovrd_en=parm[2].value.unumber, .rxd5_del_sel=parm[3].value.unumber, .rxd5_del_ovrd_en=parm[4].value.unumber, .rxd6_del_sel=parm[5].value.unumber, .rxd6_del_ovrd_en=parm[6].value.unumber, .rxd7_del_sel=parm[7].value.unumber, .rxd7_del_ovrd_en=parm[8].value.unumber};
        err = ag_drv_ubus_slv__rx_delay_cntrl_1_set(&_rx_delay_cntrl_1);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_2:
    {
        ubus_slv__rx_delay_cntrl_2 _rx_delay_cntrl_2 = { .rxctl_pos_del_sel=parm[1].value.unumber, .rxctl_pos_del_ovrd_en=parm[2].value.unumber, .rxctl_neg_del_sel=parm[3].value.unumber, .rxctl_neg_del_ovrd_en=parm[4].value.unumber, .rxclk_del_sel=parm[5].value.unumber, .rxclk_del_ovrd_en=parm[6].value.unumber};
        err = ag_drv_ubus_slv__rx_delay_cntrl_2_set(&_rx_delay_cntrl_2);
        break;
    }
    case cli_ubus_slv__clk_rst_ctrl:
        err = ag_drv_ubus_slv__clk_rst_ctrl_set(parm[1].value.unumber, parm[2].value.unumber);
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
    case cli_ubus_slv_led_cntrl:
    {
        ubus_slv_led_cntrl led_cntrl;
        err = ag_drv_ubus_slv_led_cntrl_get(&led_cntrl);
        bdmf_session_print(session, "rx_act_en = %u (0x%x)\n", led_cntrl.rx_act_en, led_cntrl.rx_act_en);
        bdmf_session_print(session, "tx_act_en = %u (0x%x)\n", led_cntrl.tx_act_en, led_cntrl.tx_act_en);
        bdmf_session_print(session, "spdlnk_led0_act_sel = %u (0x%x)\n", led_cntrl.spdlnk_led0_act_sel, led_cntrl.spdlnk_led0_act_sel);
        bdmf_session_print(session, "spdlnk_led1_act_sel = %u (0x%x)\n", led_cntrl.spdlnk_led1_act_sel, led_cntrl.spdlnk_led1_act_sel);
        bdmf_session_print(session, "spdlnk_led2_act_sel = %u (0x%x)\n", led_cntrl.spdlnk_led2_act_sel, led_cntrl.spdlnk_led2_act_sel);
        bdmf_session_print(session, "act_led_act_sel = %u (0x%x)\n", led_cntrl.act_led_act_sel, led_cntrl.act_led_act_sel);
        bdmf_session_print(session, "spdlnk_led0_act_pol_sel = %u (0x%x)\n", led_cntrl.spdlnk_led0_act_pol_sel, led_cntrl.spdlnk_led0_act_pol_sel);
        bdmf_session_print(session, "spdlnk_led1_act_pol_sel = %u (0x%x)\n", led_cntrl.spdlnk_led1_act_pol_sel, led_cntrl.spdlnk_led1_act_pol_sel);
        bdmf_session_print(session, "spdlnk_led2_act_pol_sel = %u (0x%x)\n", led_cntrl.spdlnk_led2_act_pol_sel, led_cntrl.spdlnk_led2_act_pol_sel);
        bdmf_session_print(session, "act_led_pol_sel = %u (0x%x)\n", led_cntrl.act_led_pol_sel, led_cntrl.act_led_pol_sel);
        bdmf_session_print(session, "led_spd_ovrd = %u (0x%x)\n", led_cntrl.led_spd_ovrd, led_cntrl.led_spd_ovrd);
        bdmf_session_print(session, "lnk_status_ovrd = %u (0x%x)\n", led_cntrl.lnk_status_ovrd, led_cntrl.lnk_status_ovrd);
        bdmf_session_print(session, "spd_ovrd_en = %u (0x%x)\n", led_cntrl.spd_ovrd_en, led_cntrl.spd_ovrd_en);
        bdmf_session_print(session, "lnk_ovrd_en = %u (0x%x)\n", led_cntrl.lnk_ovrd_en, led_cntrl.lnk_ovrd_en);
        break;
    }
    case cli_ubus_slv_led_link_and_speed_encoding_sel:
    {
        ubus_slv_led_link_and_speed_encoding_sel led_link_and_speed_encoding_sel;
        err = ag_drv_ubus_slv_led_link_and_speed_encoding_sel_get(&led_link_and_speed_encoding_sel);
        bdmf_session_print(session, "sel_no_link_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_no_link_encode, led_link_and_speed_encoding_sel.sel_no_link_encode);
        bdmf_session_print(session, "sel_10m_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_10m_encode, led_link_and_speed_encoding_sel.sel_10m_encode);
        bdmf_session_print(session, "sel_100m_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_100m_encode, led_link_and_speed_encoding_sel.sel_100m_encode);
        bdmf_session_print(session, "sel_1000m_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_1000m_encode, led_link_and_speed_encoding_sel.sel_1000m_encode);
        bdmf_session_print(session, "sel_2500m_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_2500m_encode, led_link_and_speed_encoding_sel.sel_2500m_encode);
        bdmf_session_print(session, "sel_10g_encode = %u (0x%x)\n", led_link_and_speed_encoding_sel.sel_10g_encode, led_link_and_speed_encoding_sel.sel_10g_encode);
        break;
    }
    case cli_ubus_slv_led_link_and_speed_encoding:
    {
        ubus_slv_led_link_and_speed_encoding led_link_and_speed_encoding;
        err = ag_drv_ubus_slv_led_link_and_speed_encoding_get(&led_link_and_speed_encoding);
        bdmf_session_print(session, "no_link_encode = %u (0x%x)\n", led_link_and_speed_encoding.no_link_encode, led_link_and_speed_encoding.no_link_encode);
        bdmf_session_print(session, "m10_encode = %u (0x%x)\n", led_link_and_speed_encoding.m10_encode, led_link_and_speed_encoding.m10_encode);
        bdmf_session_print(session, "m100_encode = %u (0x%x)\n", led_link_and_speed_encoding.m100_encode, led_link_and_speed_encoding.m100_encode);
        bdmf_session_print(session, "m1000_encode = %u (0x%x)\n", led_link_and_speed_encoding.m1000_encode, led_link_and_speed_encoding.m1000_encode);
        bdmf_session_print(session, "m2500_encode = %u (0x%x)\n", led_link_and_speed_encoding.m2500_encode, led_link_and_speed_encoding.m2500_encode);
        bdmf_session_print(session, "m10g_encode = %u (0x%x)\n", led_link_and_speed_encoding.m10g_encode, led_link_and_speed_encoding.m10g_encode);
        break;
    }
    case cli_ubus_slv_led_blink_rate_cntrl:
    {
        uint16_t led_off_time;
        uint16_t led_on_time;
        err = ag_drv_ubus_slv_led_blink_rate_cntrl_get(&led_off_time, &led_on_time);
        bdmf_session_print(session, "led_off_time = %u (0x%x)\n", led_off_time, led_off_time);
        bdmf_session_print(session, "led_on_time = %u (0x%x)\n", led_on_time, led_on_time);
        break;
    }
    case cli_ubus_slv__cntrl:
    {
        ubus_slv__cntrl _cntrl;
        err = ag_drv_ubus_slv__cntrl_get(&_cntrl);
        bdmf_session_print(session, "rgmii_mode_en = %u (0x%x)\n", _cntrl.rgmii_mode_en, _cntrl.rgmii_mode_en);
        bdmf_session_print(session, "id_mode_dis = %u (0x%x)\n", _cntrl.id_mode_dis, _cntrl.id_mode_dis);
        bdmf_session_print(session, "port_mode = %u (0x%x)\n", _cntrl.port_mode, _cntrl.port_mode);
        bdmf_session_print(session, "rvmii_ref_sel = %u (0x%x)\n", _cntrl.rvmii_ref_sel, _cntrl.rvmii_ref_sel);
        bdmf_session_print(session, "rx_pause_en = %u (0x%x)\n", _cntrl.rx_pause_en, _cntrl.rx_pause_en);
        bdmf_session_print(session, "tx_pause_en = %u (0x%x)\n", _cntrl.tx_pause_en, _cntrl.tx_pause_en);
        bdmf_session_print(session, "tx_clk_stop_en = %u (0x%x)\n", _cntrl.tx_clk_stop_en, _cntrl.tx_clk_stop_en);
        bdmf_session_print(session, "lpi_count = %u (0x%x)\n", _cntrl.lpi_count, _cntrl.lpi_count);
        bdmf_session_print(session, "rx_err_mask = %u (0x%x)\n", _cntrl.rx_err_mask, _cntrl.rx_err_mask);
        bdmf_session_print(session, "col_crs_mask = %u (0x%x)\n", _cntrl.col_crs_mask, _cntrl.col_crs_mask);
        bdmf_session_print(session, "pseudo_hd_mode_en = %u (0x%x)\n", _cntrl.pseudo_hd_mode_en, _cntrl.pseudo_hd_mode_en);
        break;
    }
    case cli_ubus_slv__ib_status:
    {
        uint8_t speed_decode;
        bdmf_boolean duplex_decode;
        bdmf_boolean link_decode;
        bdmf_boolean ib_status_ovrd;
        err = ag_drv_ubus_slv__ib_status_get(&speed_decode, &duplex_decode, &link_decode, &ib_status_ovrd);
        bdmf_session_print(session, "speed_decode = %u (0x%x)\n", speed_decode, speed_decode);
        bdmf_session_print(session, "duplex_decode = %u (0x%x)\n", duplex_decode, duplex_decode);
        bdmf_session_print(session, "link_decode = %u (0x%x)\n", link_decode, link_decode);
        bdmf_session_print(session, "ib_status_ovrd = %u (0x%x)\n", ib_status_ovrd, ib_status_ovrd);
        break;
    }
    case cli_ubus_slv__rx_clock_delay_cntrl:
    {
        ubus_slv__rx_clock_delay_cntrl _rx_clock_delay_cntrl;
        err = ag_drv_ubus_slv__rx_clock_delay_cntrl_get(&_rx_clock_delay_cntrl);
        bdmf_session_print(session, "ctri = %u (0x%x)\n", _rx_clock_delay_cntrl.ctri, _rx_clock_delay_cntrl.ctri);
        bdmf_session_print(session, "drng = %u (0x%x)\n", _rx_clock_delay_cntrl.drng, _rx_clock_delay_cntrl.drng);
        bdmf_session_print(session, "iddq = %u (0x%x)\n", _rx_clock_delay_cntrl.iddq, _rx_clock_delay_cntrl.iddq);
        bdmf_session_print(session, "bypass = %u (0x%x)\n", _rx_clock_delay_cntrl.bypass, _rx_clock_delay_cntrl.bypass);
        bdmf_session_print(session, "dly_sel = %u (0x%x)\n", _rx_clock_delay_cntrl.dly_sel, _rx_clock_delay_cntrl.dly_sel);
        bdmf_session_print(session, "dly_override = %u (0x%x)\n", _rx_clock_delay_cntrl.dly_override, _rx_clock_delay_cntrl.dly_override);
        bdmf_session_print(session, "reset = %u (0x%x)\n", _rx_clock_delay_cntrl.reset, _rx_clock_delay_cntrl.reset);
        break;
    }
    case cli_ubus_slv__ate_rx_cntrl_exp_data:
    {
        ubus_slv__ate_rx_cntrl_exp_data _ate_rx_cntrl_exp_data;
        err = ag_drv_ubus_slv__ate_rx_cntrl_exp_data_get(&_ate_rx_cntrl_exp_data);
        bdmf_session_print(session, "expected_data_0 = %u (0x%x)\n", _ate_rx_cntrl_exp_data.expected_data_0, _ate_rx_cntrl_exp_data.expected_data_0);
        bdmf_session_print(session, "expected_data_1 = %u (0x%x)\n", _ate_rx_cntrl_exp_data.expected_data_1, _ate_rx_cntrl_exp_data.expected_data_1);
        bdmf_session_print(session, "good_count = %u (0x%x)\n", _ate_rx_cntrl_exp_data.good_count, _ate_rx_cntrl_exp_data.good_count);
        bdmf_session_print(session, "pkt_count_rst = %u (0x%x)\n", _ate_rx_cntrl_exp_data.pkt_count_rst, _ate_rx_cntrl_exp_data.pkt_count_rst);
        bdmf_session_print(session, "ate_en = %u (0x%x)\n", _ate_rx_cntrl_exp_data.ate_en, _ate_rx_cntrl_exp_data.ate_en);
        break;
    }
    case cli_ubus_slv__ate_rx_exp_data_1:
    {
        uint16_t expected_data_2;
        uint16_t expected_data_3;
        err = ag_drv_ubus_slv__ate_rx_exp_data_1_get(&expected_data_2, &expected_data_3);
        bdmf_session_print(session, "expected_data_2 = %u (0x%x)\n", expected_data_2, expected_data_2);
        bdmf_session_print(session, "expected_data_3 = %u (0x%x)\n", expected_data_3, expected_data_3);
        break;
    }
    case cli_ubus_slv__ate_rx_status_0:
    {
        uint16_t received_data_0;
        uint16_t received_data_1;
        bdmf_boolean rx_ok;
        err = ag_drv_ubus_slv__ate_rx_status_0_get(&received_data_0, &received_data_1, &rx_ok);
        bdmf_session_print(session, "received_data_0 = %u (0x%x)\n", received_data_0, received_data_0);
        bdmf_session_print(session, "received_data_1 = %u (0x%x)\n", received_data_1, received_data_1);
        bdmf_session_print(session, "rx_ok = %u (0x%x)\n", rx_ok, rx_ok);
        break;
    }
    case cli_ubus_slv__ate_rx_status_1:
    {
        uint16_t received_data_2;
        uint16_t received_data_3;
        err = ag_drv_ubus_slv__ate_rx_status_1_get(&received_data_2, &received_data_3);
        bdmf_session_print(session, "received_data_2 = %u (0x%x)\n", received_data_2, received_data_2);
        bdmf_session_print(session, "received_data_3 = %u (0x%x)\n", received_data_3, received_data_3);
        break;
    }
    case cli_ubus_slv__ate_tx_cntrl:
    {
        ubus_slv__ate_tx_cntrl _ate_tx_cntrl;
        err = ag_drv_ubus_slv__ate_tx_cntrl_get(&_ate_tx_cntrl);
        bdmf_session_print(session, "start_stop_ovrd = %u (0x%x)\n", _ate_tx_cntrl.start_stop_ovrd, _ate_tx_cntrl.start_stop_ovrd);
        bdmf_session_print(session, "start_stop = %u (0x%x)\n", _ate_tx_cntrl.start_stop, _ate_tx_cntrl.start_stop);
        bdmf_session_print(session, "pkt_gen_en = %u (0x%x)\n", _ate_tx_cntrl.pkt_gen_en, _ate_tx_cntrl.pkt_gen_en);
        bdmf_session_print(session, "pkt_cnt = %u (0x%x)\n", _ate_tx_cntrl.pkt_cnt, _ate_tx_cntrl.pkt_cnt);
        bdmf_session_print(session, "payload_length = %u (0x%x)\n", _ate_tx_cntrl.payload_length, _ate_tx_cntrl.payload_length);
        bdmf_session_print(session, "pkt_ipg = %u (0x%x)\n", _ate_tx_cntrl.pkt_ipg, _ate_tx_cntrl.pkt_ipg);
        break;
    }
    case cli_ubus_slv__ate_tx_data_0:
    {
        uint16_t tx_data_0;
        uint16_t tx_data_1;
        err = ag_drv_ubus_slv__ate_tx_data_0_get(&tx_data_0, &tx_data_1);
        bdmf_session_print(session, "tx_data_0 = %u (0x%x)\n", tx_data_0, tx_data_0);
        bdmf_session_print(session, "tx_data_1 = %u (0x%x)\n", tx_data_1, tx_data_1);
        break;
    }
    case cli_ubus_slv__ate_tx_data_1:
    {
        uint16_t tx_data_2;
        uint16_t tx_data_3;
        err = ag_drv_ubus_slv__ate_tx_data_1_get(&tx_data_2, &tx_data_3);
        bdmf_session_print(session, "tx_data_2 = %u (0x%x)\n", tx_data_2, tx_data_2);
        bdmf_session_print(session, "tx_data_3 = %u (0x%x)\n", tx_data_3, tx_data_3);
        break;
    }
    case cli_ubus_slv__ate_tx_data_2:
    {
        uint8_t tx_data_4;
        uint8_t tx_data_5;
        uint16_t ether_type;
        err = ag_drv_ubus_slv__ate_tx_data_2_get(&tx_data_4, &tx_data_5, &ether_type);
        bdmf_session_print(session, "tx_data_4 = %u (0x%x)\n", tx_data_4, tx_data_4);
        bdmf_session_print(session, "tx_data_5 = %u (0x%x)\n", tx_data_5, tx_data_5);
        bdmf_session_print(session, "ether_type = %u (0x%x)\n", ether_type, ether_type);
        break;
    }
    case cli_ubus_slv__tx_delay_cntrl_0:
    {
        ubus_slv__tx_delay_cntrl_0 _tx_delay_cntrl_0;
        err = ag_drv_ubus_slv__tx_delay_cntrl_0_get(&_tx_delay_cntrl_0);
        bdmf_session_print(session, "txd0_del_sel = %u (0x%x)\n", _tx_delay_cntrl_0.txd0_del_sel, _tx_delay_cntrl_0.txd0_del_sel);
        bdmf_session_print(session, "txd0_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_0.txd0_del_ovrd_en, _tx_delay_cntrl_0.txd0_del_ovrd_en);
        bdmf_session_print(session, "txd1_del_sel = %u (0x%x)\n", _tx_delay_cntrl_0.txd1_del_sel, _tx_delay_cntrl_0.txd1_del_sel);
        bdmf_session_print(session, "txd1_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_0.txd1_del_ovrd_en, _tx_delay_cntrl_0.txd1_del_ovrd_en);
        bdmf_session_print(session, "txd2_del_sel = %u (0x%x)\n", _tx_delay_cntrl_0.txd2_del_sel, _tx_delay_cntrl_0.txd2_del_sel);
        bdmf_session_print(session, "txd2_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_0.txd2_del_ovrd_en, _tx_delay_cntrl_0.txd2_del_ovrd_en);
        bdmf_session_print(session, "txd3_del_sel = %u (0x%x)\n", _tx_delay_cntrl_0.txd3_del_sel, _tx_delay_cntrl_0.txd3_del_sel);
        bdmf_session_print(session, "txd3_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_0.txd3_del_ovrd_en, _tx_delay_cntrl_0.txd3_del_ovrd_en);
        break;
    }
    case cli_ubus_slv__tx_delay_cntrl_1:
    {
        ubus_slv__tx_delay_cntrl_1 _tx_delay_cntrl_1;
        err = ag_drv_ubus_slv__tx_delay_cntrl_1_get(&_tx_delay_cntrl_1);
        bdmf_session_print(session, "txctl_del_sel = %u (0x%x)\n", _tx_delay_cntrl_1.txctl_del_sel, _tx_delay_cntrl_1.txctl_del_sel);
        bdmf_session_print(session, "txctl_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_1.txctl_del_ovrd_en, _tx_delay_cntrl_1.txctl_del_ovrd_en);
        bdmf_session_print(session, "txclk_del_sel = %u (0x%x)\n", _tx_delay_cntrl_1.txclk_del_sel, _tx_delay_cntrl_1.txclk_del_sel);
        bdmf_session_print(session, "txclk_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_1.txclk_del_ovrd_en, _tx_delay_cntrl_1.txclk_del_ovrd_en);
        bdmf_session_print(session, "txclk_id_del_sel = %u (0x%x)\n", _tx_delay_cntrl_1.txclk_id_del_sel, _tx_delay_cntrl_1.txclk_id_del_sel);
        bdmf_session_print(session, "txclk_id_del_ovrd_en = %u (0x%x)\n", _tx_delay_cntrl_1.txclk_id_del_ovrd_en, _tx_delay_cntrl_1.txclk_id_del_ovrd_en);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_0:
    {
        ubus_slv__rx_delay_cntrl_0 _rx_delay_cntrl_0;
        err = ag_drv_ubus_slv__rx_delay_cntrl_0_get(&_rx_delay_cntrl_0);
        bdmf_session_print(session, "rxd0_del_sel = %u (0x%x)\n", _rx_delay_cntrl_0.rxd0_del_sel, _rx_delay_cntrl_0.rxd0_del_sel);
        bdmf_session_print(session, "rxd0_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_0.rxd0_del_ovrd_en, _rx_delay_cntrl_0.rxd0_del_ovrd_en);
        bdmf_session_print(session, "rxd1_del_sel = %u (0x%x)\n", _rx_delay_cntrl_0.rxd1_del_sel, _rx_delay_cntrl_0.rxd1_del_sel);
        bdmf_session_print(session, "rxd1_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_0.rxd1_del_ovrd_en, _rx_delay_cntrl_0.rxd1_del_ovrd_en);
        bdmf_session_print(session, "rxd2_del_sel = %u (0x%x)\n", _rx_delay_cntrl_0.rxd2_del_sel, _rx_delay_cntrl_0.rxd2_del_sel);
        bdmf_session_print(session, "rxd2_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_0.rxd2_del_ovrd_en, _rx_delay_cntrl_0.rxd2_del_ovrd_en);
        bdmf_session_print(session, "rxd3_del_sel = %u (0x%x)\n", _rx_delay_cntrl_0.rxd3_del_sel, _rx_delay_cntrl_0.rxd3_del_sel);
        bdmf_session_print(session, "rxd3_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_0.rxd3_del_ovrd_en, _rx_delay_cntrl_0.rxd3_del_ovrd_en);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_1:
    {
        ubus_slv__rx_delay_cntrl_1 _rx_delay_cntrl_1;
        err = ag_drv_ubus_slv__rx_delay_cntrl_1_get(&_rx_delay_cntrl_1);
        bdmf_session_print(session, "rxd4_del_sel = %u (0x%x)\n", _rx_delay_cntrl_1.rxd4_del_sel, _rx_delay_cntrl_1.rxd4_del_sel);
        bdmf_session_print(session, "rxd4_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_1.rxd4_del_ovrd_en, _rx_delay_cntrl_1.rxd4_del_ovrd_en);
        bdmf_session_print(session, "rxd5_del_sel = %u (0x%x)\n", _rx_delay_cntrl_1.rxd5_del_sel, _rx_delay_cntrl_1.rxd5_del_sel);
        bdmf_session_print(session, "rxd5_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_1.rxd5_del_ovrd_en, _rx_delay_cntrl_1.rxd5_del_ovrd_en);
        bdmf_session_print(session, "rxd6_del_sel = %u (0x%x)\n", _rx_delay_cntrl_1.rxd6_del_sel, _rx_delay_cntrl_1.rxd6_del_sel);
        bdmf_session_print(session, "rxd6_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_1.rxd6_del_ovrd_en, _rx_delay_cntrl_1.rxd6_del_ovrd_en);
        bdmf_session_print(session, "rxd7_del_sel = %u (0x%x)\n", _rx_delay_cntrl_1.rxd7_del_sel, _rx_delay_cntrl_1.rxd7_del_sel);
        bdmf_session_print(session, "rxd7_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_1.rxd7_del_ovrd_en, _rx_delay_cntrl_1.rxd7_del_ovrd_en);
        break;
    }
    case cli_ubus_slv__rx_delay_cntrl_2:
    {
        ubus_slv__rx_delay_cntrl_2 _rx_delay_cntrl_2;
        err = ag_drv_ubus_slv__rx_delay_cntrl_2_get(&_rx_delay_cntrl_2);
        bdmf_session_print(session, "rxctl_pos_del_sel = %u (0x%x)\n", _rx_delay_cntrl_2.rxctl_pos_del_sel, _rx_delay_cntrl_2.rxctl_pos_del_sel);
        bdmf_session_print(session, "rxctl_pos_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_2.rxctl_pos_del_ovrd_en, _rx_delay_cntrl_2.rxctl_pos_del_ovrd_en);
        bdmf_session_print(session, "rxctl_neg_del_sel = %u (0x%x)\n", _rx_delay_cntrl_2.rxctl_neg_del_sel, _rx_delay_cntrl_2.rxctl_neg_del_sel);
        bdmf_session_print(session, "rxctl_neg_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_2.rxctl_neg_del_ovrd_en, _rx_delay_cntrl_2.rxctl_neg_del_ovrd_en);
        bdmf_session_print(session, "rxclk_del_sel = %u (0x%x)\n", _rx_delay_cntrl_2.rxclk_del_sel, _rx_delay_cntrl_2.rxclk_del_sel);
        bdmf_session_print(session, "rxclk_del_ovrd_en = %u (0x%x)\n", _rx_delay_cntrl_2.rxclk_del_ovrd_en, _rx_delay_cntrl_2.rxclk_del_ovrd_en);
        break;
    }
    case cli_ubus_slv__clk_rst_ctrl:
    {
        bdmf_boolean swinit;
        bdmf_boolean clk250en;
        err = ag_drv_ubus_slv__clk_rst_ctrl_get(&swinit, &clk250en);
        bdmf_session_print(session, "swinit = %u (0x%x)\n", swinit, swinit);
        bdmf_session_print(session, "clk250en = %u (0x%x)\n", clk250en, clk250en);
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
    {
        ubus_slv_led_cntrl led_cntrl = {.rx_act_en=gtmv(m, 1), .tx_act_en=gtmv(m, 1), .spdlnk_led0_act_sel=gtmv(m, 1), .spdlnk_led1_act_sel=gtmv(m, 1), .spdlnk_led2_act_sel=gtmv(m, 1), .act_led_act_sel=gtmv(m, 1), .spdlnk_led0_act_pol_sel=gtmv(m, 1), .spdlnk_led1_act_pol_sel=gtmv(m, 1), .spdlnk_led2_act_pol_sel=gtmv(m, 1), .act_led_pol_sel=gtmv(m, 1), .led_spd_ovrd=gtmv(m, 3), .lnk_status_ovrd=gtmv(m, 1), .spd_ovrd_en=gtmv(m, 1), .lnk_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_cntrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", led_cntrl.rx_act_en, led_cntrl.tx_act_en, led_cntrl.spdlnk_led0_act_sel, led_cntrl.spdlnk_led1_act_sel, led_cntrl.spdlnk_led2_act_sel, led_cntrl.act_led_act_sel, led_cntrl.spdlnk_led0_act_pol_sel, led_cntrl.spdlnk_led1_act_pol_sel, led_cntrl.spdlnk_led2_act_pol_sel, led_cntrl.act_led_pol_sel, led_cntrl.led_spd_ovrd, led_cntrl.lnk_status_ovrd, led_cntrl.spd_ovrd_en, led_cntrl.lnk_ovrd_en);
        if(!err) ag_drv_ubus_slv_led_cntrl_set(&led_cntrl);
        if(!err) ag_drv_ubus_slv_led_cntrl_get( &led_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_cntrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", led_cntrl.rx_act_en, led_cntrl.tx_act_en, led_cntrl.spdlnk_led0_act_sel, led_cntrl.spdlnk_led1_act_sel, led_cntrl.spdlnk_led2_act_sel, led_cntrl.act_led_act_sel, led_cntrl.spdlnk_led0_act_pol_sel, led_cntrl.spdlnk_led1_act_pol_sel, led_cntrl.spdlnk_led2_act_pol_sel, led_cntrl.act_led_pol_sel, led_cntrl.led_spd_ovrd, led_cntrl.lnk_status_ovrd, led_cntrl.spd_ovrd_en, led_cntrl.lnk_ovrd_en);
        if(err || led_cntrl.rx_act_en!=gtmv(m, 1) || led_cntrl.tx_act_en!=gtmv(m, 1) || led_cntrl.spdlnk_led0_act_sel!=gtmv(m, 1) || led_cntrl.spdlnk_led1_act_sel!=gtmv(m, 1) || led_cntrl.spdlnk_led2_act_sel!=gtmv(m, 1) || led_cntrl.act_led_act_sel!=gtmv(m, 1) || led_cntrl.spdlnk_led0_act_pol_sel!=gtmv(m, 1) || led_cntrl.spdlnk_led1_act_pol_sel!=gtmv(m, 1) || led_cntrl.spdlnk_led2_act_pol_sel!=gtmv(m, 1) || led_cntrl.act_led_pol_sel!=gtmv(m, 1) || led_cntrl.led_spd_ovrd!=gtmv(m, 3) || led_cntrl.lnk_status_ovrd!=gtmv(m, 1) || led_cntrl.spd_ovrd_en!=gtmv(m, 1) || led_cntrl.lnk_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv_led_link_and_speed_encoding_sel led_link_and_speed_encoding_sel = {.sel_no_link_encode=gtmv(m, 3), .sel_10m_encode=gtmv(m, 3), .sel_100m_encode=gtmv(m, 3), .sel_1000m_encode=gtmv(m, 3), .sel_2500m_encode=gtmv(m, 3), .sel_10g_encode=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_link_and_speed_encoding_sel_set( %u %u %u %u %u %u)\n", led_link_and_speed_encoding_sel.sel_no_link_encode, led_link_and_speed_encoding_sel.sel_10m_encode, led_link_and_speed_encoding_sel.sel_100m_encode, led_link_and_speed_encoding_sel.sel_1000m_encode, led_link_and_speed_encoding_sel.sel_2500m_encode, led_link_and_speed_encoding_sel.sel_10g_encode);
        if(!err) ag_drv_ubus_slv_led_link_and_speed_encoding_sel_set(&led_link_and_speed_encoding_sel);
        if(!err) ag_drv_ubus_slv_led_link_and_speed_encoding_sel_get( &led_link_and_speed_encoding_sel);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_link_and_speed_encoding_sel_get( %u %u %u %u %u %u)\n", led_link_and_speed_encoding_sel.sel_no_link_encode, led_link_and_speed_encoding_sel.sel_10m_encode, led_link_and_speed_encoding_sel.sel_100m_encode, led_link_and_speed_encoding_sel.sel_1000m_encode, led_link_and_speed_encoding_sel.sel_2500m_encode, led_link_and_speed_encoding_sel.sel_10g_encode);
        if(err || led_link_and_speed_encoding_sel.sel_no_link_encode!=gtmv(m, 3) || led_link_and_speed_encoding_sel.sel_10m_encode!=gtmv(m, 3) || led_link_and_speed_encoding_sel.sel_100m_encode!=gtmv(m, 3) || led_link_and_speed_encoding_sel.sel_1000m_encode!=gtmv(m, 3) || led_link_and_speed_encoding_sel.sel_2500m_encode!=gtmv(m, 3) || led_link_and_speed_encoding_sel.sel_10g_encode!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv_led_link_and_speed_encoding led_link_and_speed_encoding = {.no_link_encode=gtmv(m, 3), .m10_encode=gtmv(m, 3), .m100_encode=gtmv(m, 3), .m1000_encode=gtmv(m, 3), .m2500_encode=gtmv(m, 3), .m10g_encode=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_link_and_speed_encoding_set( %u %u %u %u %u %u)\n", led_link_and_speed_encoding.no_link_encode, led_link_and_speed_encoding.m10_encode, led_link_and_speed_encoding.m100_encode, led_link_and_speed_encoding.m1000_encode, led_link_and_speed_encoding.m2500_encode, led_link_and_speed_encoding.m10g_encode);
        if(!err) ag_drv_ubus_slv_led_link_and_speed_encoding_set(&led_link_and_speed_encoding);
        if(!err) ag_drv_ubus_slv_led_link_and_speed_encoding_get( &led_link_and_speed_encoding);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_link_and_speed_encoding_get( %u %u %u %u %u %u)\n", led_link_and_speed_encoding.no_link_encode, led_link_and_speed_encoding.m10_encode, led_link_and_speed_encoding.m100_encode, led_link_and_speed_encoding.m1000_encode, led_link_and_speed_encoding.m2500_encode, led_link_and_speed_encoding.m10g_encode);
        if(err || led_link_and_speed_encoding.no_link_encode!=gtmv(m, 3) || led_link_and_speed_encoding.m10_encode!=gtmv(m, 3) || led_link_and_speed_encoding.m100_encode!=gtmv(m, 3) || led_link_and_speed_encoding.m1000_encode!=gtmv(m, 3) || led_link_and_speed_encoding.m2500_encode!=gtmv(m, 3) || led_link_and_speed_encoding.m10g_encode!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t led_off_time=gtmv(m, 16);
        uint16_t led_on_time=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_blink_rate_cntrl_set( %u %u)\n", led_off_time, led_on_time);
        if(!err) ag_drv_ubus_slv_led_blink_rate_cntrl_set(led_off_time, led_on_time);
        if(!err) ag_drv_ubus_slv_led_blink_rate_cntrl_get( &led_off_time, &led_on_time);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv_led_blink_rate_cntrl_get( %u %u)\n", led_off_time, led_on_time);
        if(err || led_off_time!=gtmv(m, 16) || led_on_time!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__cntrl _cntrl = {.rgmii_mode_en=gtmv(m, 1), .id_mode_dis=gtmv(m, 1), .port_mode=gtmv(m, 3), .rvmii_ref_sel=gtmv(m, 1), .rx_pause_en=gtmv(m, 1), .tx_pause_en=gtmv(m, 1), .tx_clk_stop_en=gtmv(m, 1), .lpi_count=gtmv(m, 5), .rx_err_mask=gtmv(m, 1), .col_crs_mask=gtmv(m, 1), .pseudo_hd_mode_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__cntrl_set( %u %u %u %u %u %u %u %u %u %u %u)\n", _cntrl.rgmii_mode_en, _cntrl.id_mode_dis, _cntrl.port_mode, _cntrl.rvmii_ref_sel, _cntrl.rx_pause_en, _cntrl.tx_pause_en, _cntrl.tx_clk_stop_en, _cntrl.lpi_count, _cntrl.rx_err_mask, _cntrl.col_crs_mask, _cntrl.pseudo_hd_mode_en);
        if(!err) ag_drv_ubus_slv__cntrl_set(&_cntrl);
        if(!err) ag_drv_ubus_slv__cntrl_get( &_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__cntrl_get( %u %u %u %u %u %u %u %u %u %u %u)\n", _cntrl.rgmii_mode_en, _cntrl.id_mode_dis, _cntrl.port_mode, _cntrl.rvmii_ref_sel, _cntrl.rx_pause_en, _cntrl.tx_pause_en, _cntrl.tx_clk_stop_en, _cntrl.lpi_count, _cntrl.rx_err_mask, _cntrl.col_crs_mask, _cntrl.pseudo_hd_mode_en);
        if(err || _cntrl.rgmii_mode_en!=gtmv(m, 1) || _cntrl.id_mode_dis!=gtmv(m, 1) || _cntrl.port_mode!=gtmv(m, 3) || _cntrl.rvmii_ref_sel!=gtmv(m, 1) || _cntrl.rx_pause_en!=gtmv(m, 1) || _cntrl.tx_pause_en!=gtmv(m, 1) || _cntrl.tx_clk_stop_en!=gtmv(m, 1) || _cntrl.lpi_count!=gtmv(m, 5) || _cntrl.rx_err_mask!=gtmv(m, 1) || _cntrl.col_crs_mask!=gtmv(m, 1) || _cntrl.pseudo_hd_mode_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t speed_decode=gtmv(m, 2);
        bdmf_boolean duplex_decode=gtmv(m, 1);
        bdmf_boolean link_decode=gtmv(m, 1);
        bdmf_boolean ib_status_ovrd=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ib_status_set( %u %u %u %u)\n", speed_decode, duplex_decode, link_decode, ib_status_ovrd);
        if(!err) ag_drv_ubus_slv__ib_status_set(speed_decode, duplex_decode, link_decode, ib_status_ovrd);
        if(!err) ag_drv_ubus_slv__ib_status_get( &speed_decode, &duplex_decode, &link_decode, &ib_status_ovrd);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ib_status_get( %u %u %u %u)\n", speed_decode, duplex_decode, link_decode, ib_status_ovrd);
        if(err || speed_decode!=gtmv(m, 2) || duplex_decode!=gtmv(m, 1) || link_decode!=gtmv(m, 1) || ib_status_ovrd!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__rx_clock_delay_cntrl _rx_clock_delay_cntrl = {.ctri=gtmv(m, 2), .drng=gtmv(m, 2), .iddq=gtmv(m, 1), .bypass=gtmv(m, 1), .dly_sel=gtmv(m, 1), .dly_override=gtmv(m, 1), .reset=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_clock_delay_cntrl_set( %u %u %u %u %u %u %u)\n", _rx_clock_delay_cntrl.ctri, _rx_clock_delay_cntrl.drng, _rx_clock_delay_cntrl.iddq, _rx_clock_delay_cntrl.bypass, _rx_clock_delay_cntrl.dly_sel, _rx_clock_delay_cntrl.dly_override, _rx_clock_delay_cntrl.reset);
        if(!err) ag_drv_ubus_slv__rx_clock_delay_cntrl_set(&_rx_clock_delay_cntrl);
        if(!err) ag_drv_ubus_slv__rx_clock_delay_cntrl_get( &_rx_clock_delay_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_clock_delay_cntrl_get( %u %u %u %u %u %u %u)\n", _rx_clock_delay_cntrl.ctri, _rx_clock_delay_cntrl.drng, _rx_clock_delay_cntrl.iddq, _rx_clock_delay_cntrl.bypass, _rx_clock_delay_cntrl.dly_sel, _rx_clock_delay_cntrl.dly_override, _rx_clock_delay_cntrl.reset);
        if(err || _rx_clock_delay_cntrl.ctri!=gtmv(m, 2) || _rx_clock_delay_cntrl.drng!=gtmv(m, 2) || _rx_clock_delay_cntrl.iddq!=gtmv(m, 1) || _rx_clock_delay_cntrl.bypass!=gtmv(m, 1) || _rx_clock_delay_cntrl.dly_sel!=gtmv(m, 1) || _rx_clock_delay_cntrl.dly_override!=gtmv(m, 1) || _rx_clock_delay_cntrl.reset!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__ate_rx_cntrl_exp_data _ate_rx_cntrl_exp_data = {.expected_data_0=gtmv(m, 9), .expected_data_1=gtmv(m, 9), .good_count=gtmv(m, 8), .pkt_count_rst=gtmv(m, 1), .ate_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_cntrl_exp_data_set( %u %u %u %u %u)\n", _ate_rx_cntrl_exp_data.expected_data_0, _ate_rx_cntrl_exp_data.expected_data_1, _ate_rx_cntrl_exp_data.good_count, _ate_rx_cntrl_exp_data.pkt_count_rst, _ate_rx_cntrl_exp_data.ate_en);
        if(!err) ag_drv_ubus_slv__ate_rx_cntrl_exp_data_set(&_ate_rx_cntrl_exp_data);
        if(!err) ag_drv_ubus_slv__ate_rx_cntrl_exp_data_get( &_ate_rx_cntrl_exp_data);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_cntrl_exp_data_get( %u %u %u %u %u)\n", _ate_rx_cntrl_exp_data.expected_data_0, _ate_rx_cntrl_exp_data.expected_data_1, _ate_rx_cntrl_exp_data.good_count, _ate_rx_cntrl_exp_data.pkt_count_rst, _ate_rx_cntrl_exp_data.ate_en);
        if(err || _ate_rx_cntrl_exp_data.expected_data_0!=gtmv(m, 9) || _ate_rx_cntrl_exp_data.expected_data_1!=gtmv(m, 9) || _ate_rx_cntrl_exp_data.good_count!=gtmv(m, 8) || _ate_rx_cntrl_exp_data.pkt_count_rst!=gtmv(m, 1) || _ate_rx_cntrl_exp_data.ate_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t expected_data_2=gtmv(m, 9);
        uint16_t expected_data_3=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_exp_data_1_set( %u %u)\n", expected_data_2, expected_data_3);
        if(!err) ag_drv_ubus_slv__ate_rx_exp_data_1_set(expected_data_2, expected_data_3);
        if(!err) ag_drv_ubus_slv__ate_rx_exp_data_1_get( &expected_data_2, &expected_data_3);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_exp_data_1_get( %u %u)\n", expected_data_2, expected_data_3);
        if(err || expected_data_2!=gtmv(m, 9) || expected_data_3!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t received_data_0=gtmv(m, 9);
        uint16_t received_data_1=gtmv(m, 9);
        bdmf_boolean rx_ok=gtmv(m, 1);
        if(!err) ag_drv_ubus_slv__ate_rx_status_0_get( &received_data_0, &received_data_1, &rx_ok);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_status_0_get( %u %u %u)\n", received_data_0, received_data_1, rx_ok);
    }
    {
        uint16_t received_data_2=gtmv(m, 9);
        uint16_t received_data_3=gtmv(m, 9);
        if(!err) ag_drv_ubus_slv__ate_rx_status_1_get( &received_data_2, &received_data_3);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_rx_status_1_get( %u %u)\n", received_data_2, received_data_3);
    }
    {
        ubus_slv__ate_tx_cntrl _ate_tx_cntrl = {.start_stop_ovrd=gtmv(m, 1), .start_stop=gtmv(m, 1), .pkt_gen_en=gtmv(m, 1), .pkt_cnt=gtmv(m, 8), .payload_length=gtmv(m, 11), .pkt_ipg=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_cntrl_set( %u %u %u %u %u %u)\n", _ate_tx_cntrl.start_stop_ovrd, _ate_tx_cntrl.start_stop, _ate_tx_cntrl.pkt_gen_en, _ate_tx_cntrl.pkt_cnt, _ate_tx_cntrl.payload_length, _ate_tx_cntrl.pkt_ipg);
        if(!err) ag_drv_ubus_slv__ate_tx_cntrl_set(&_ate_tx_cntrl);
        if(!err) ag_drv_ubus_slv__ate_tx_cntrl_get( &_ate_tx_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_cntrl_get( %u %u %u %u %u %u)\n", _ate_tx_cntrl.start_stop_ovrd, _ate_tx_cntrl.start_stop, _ate_tx_cntrl.pkt_gen_en, _ate_tx_cntrl.pkt_cnt, _ate_tx_cntrl.payload_length, _ate_tx_cntrl.pkt_ipg);
        if(err || _ate_tx_cntrl.start_stop_ovrd!=gtmv(m, 1) || _ate_tx_cntrl.start_stop!=gtmv(m, 1) || _ate_tx_cntrl.pkt_gen_en!=gtmv(m, 1) || _ate_tx_cntrl.pkt_cnt!=gtmv(m, 8) || _ate_tx_cntrl.payload_length!=gtmv(m, 11) || _ate_tx_cntrl.pkt_ipg!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t tx_data_0=gtmv(m, 9);
        uint16_t tx_data_1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_0_set( %u %u)\n", tx_data_0, tx_data_1);
        if(!err) ag_drv_ubus_slv__ate_tx_data_0_set(tx_data_0, tx_data_1);
        if(!err) ag_drv_ubus_slv__ate_tx_data_0_get( &tx_data_0, &tx_data_1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_0_get( %u %u)\n", tx_data_0, tx_data_1);
        if(err || tx_data_0!=gtmv(m, 9) || tx_data_1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t tx_data_2=gtmv(m, 9);
        uint16_t tx_data_3=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_1_set( %u %u)\n", tx_data_2, tx_data_3);
        if(!err) ag_drv_ubus_slv__ate_tx_data_1_set(tx_data_2, tx_data_3);
        if(!err) ag_drv_ubus_slv__ate_tx_data_1_get( &tx_data_2, &tx_data_3);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_1_get( %u %u)\n", tx_data_2, tx_data_3);
        if(err || tx_data_2!=gtmv(m, 9) || tx_data_3!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tx_data_4=gtmv(m, 8);
        uint8_t tx_data_5=gtmv(m, 8);
        uint16_t ether_type=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_2_set( %u %u %u)\n", tx_data_4, tx_data_5, ether_type);
        if(!err) ag_drv_ubus_slv__ate_tx_data_2_set(tx_data_4, tx_data_5, ether_type);
        if(!err) ag_drv_ubus_slv__ate_tx_data_2_get( &tx_data_4, &tx_data_5, &ether_type);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__ate_tx_data_2_get( %u %u %u)\n", tx_data_4, tx_data_5, ether_type);
        if(err || tx_data_4!=gtmv(m, 8) || tx_data_5!=gtmv(m, 8) || ether_type!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__tx_delay_cntrl_0 _tx_delay_cntrl_0 = {.txd0_del_sel=gtmv(m, 6), .txd0_del_ovrd_en=gtmv(m, 1), .txd1_del_sel=gtmv(m, 6), .txd1_del_ovrd_en=gtmv(m, 1), .txd2_del_sel=gtmv(m, 6), .txd2_del_ovrd_en=gtmv(m, 1), .txd3_del_sel=gtmv(m, 6), .txd3_del_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__tx_delay_cntrl_0_set( %u %u %u %u %u %u %u %u)\n", _tx_delay_cntrl_0.txd0_del_sel, _tx_delay_cntrl_0.txd0_del_ovrd_en, _tx_delay_cntrl_0.txd1_del_sel, _tx_delay_cntrl_0.txd1_del_ovrd_en, _tx_delay_cntrl_0.txd2_del_sel, _tx_delay_cntrl_0.txd2_del_ovrd_en, _tx_delay_cntrl_0.txd3_del_sel, _tx_delay_cntrl_0.txd3_del_ovrd_en);
        if(!err) ag_drv_ubus_slv__tx_delay_cntrl_0_set(&_tx_delay_cntrl_0);
        if(!err) ag_drv_ubus_slv__tx_delay_cntrl_0_get( &_tx_delay_cntrl_0);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__tx_delay_cntrl_0_get( %u %u %u %u %u %u %u %u)\n", _tx_delay_cntrl_0.txd0_del_sel, _tx_delay_cntrl_0.txd0_del_ovrd_en, _tx_delay_cntrl_0.txd1_del_sel, _tx_delay_cntrl_0.txd1_del_ovrd_en, _tx_delay_cntrl_0.txd2_del_sel, _tx_delay_cntrl_0.txd2_del_ovrd_en, _tx_delay_cntrl_0.txd3_del_sel, _tx_delay_cntrl_0.txd3_del_ovrd_en);
        if(err || _tx_delay_cntrl_0.txd0_del_sel!=gtmv(m, 6) || _tx_delay_cntrl_0.txd0_del_ovrd_en!=gtmv(m, 1) || _tx_delay_cntrl_0.txd1_del_sel!=gtmv(m, 6) || _tx_delay_cntrl_0.txd1_del_ovrd_en!=gtmv(m, 1) || _tx_delay_cntrl_0.txd2_del_sel!=gtmv(m, 6) || _tx_delay_cntrl_0.txd2_del_ovrd_en!=gtmv(m, 1) || _tx_delay_cntrl_0.txd3_del_sel!=gtmv(m, 6) || _tx_delay_cntrl_0.txd3_del_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__tx_delay_cntrl_1 _tx_delay_cntrl_1 = {.txctl_del_sel=gtmv(m, 6), .txctl_del_ovrd_en=gtmv(m, 1), .txclk_del_sel=gtmv(m, 4), .txclk_del_ovrd_en=gtmv(m, 1), .txclk_id_del_sel=gtmv(m, 4), .txclk_id_del_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__tx_delay_cntrl_1_set( %u %u %u %u %u %u)\n", _tx_delay_cntrl_1.txctl_del_sel, _tx_delay_cntrl_1.txctl_del_ovrd_en, _tx_delay_cntrl_1.txclk_del_sel, _tx_delay_cntrl_1.txclk_del_ovrd_en, _tx_delay_cntrl_1.txclk_id_del_sel, _tx_delay_cntrl_1.txclk_id_del_ovrd_en);
        if(!err) ag_drv_ubus_slv__tx_delay_cntrl_1_set(&_tx_delay_cntrl_1);
        if(!err) ag_drv_ubus_slv__tx_delay_cntrl_1_get( &_tx_delay_cntrl_1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__tx_delay_cntrl_1_get( %u %u %u %u %u %u)\n", _tx_delay_cntrl_1.txctl_del_sel, _tx_delay_cntrl_1.txctl_del_ovrd_en, _tx_delay_cntrl_1.txclk_del_sel, _tx_delay_cntrl_1.txclk_del_ovrd_en, _tx_delay_cntrl_1.txclk_id_del_sel, _tx_delay_cntrl_1.txclk_id_del_ovrd_en);
        if(err || _tx_delay_cntrl_1.txctl_del_sel!=gtmv(m, 6) || _tx_delay_cntrl_1.txctl_del_ovrd_en!=gtmv(m, 1) || _tx_delay_cntrl_1.txclk_del_sel!=gtmv(m, 4) || _tx_delay_cntrl_1.txclk_del_ovrd_en!=gtmv(m, 1) || _tx_delay_cntrl_1.txclk_id_del_sel!=gtmv(m, 4) || _tx_delay_cntrl_1.txclk_id_del_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__rx_delay_cntrl_0 _rx_delay_cntrl_0 = {.rxd0_del_sel=gtmv(m, 6), .rxd0_del_ovrd_en=gtmv(m, 1), .rxd1_del_sel=gtmv(m, 6), .rxd1_del_ovrd_en=gtmv(m, 1), .rxd2_del_sel=gtmv(m, 6), .rxd2_del_ovrd_en=gtmv(m, 1), .rxd3_del_sel=gtmv(m, 6), .rxd3_del_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_0_set( %u %u %u %u %u %u %u %u)\n", _rx_delay_cntrl_0.rxd0_del_sel, _rx_delay_cntrl_0.rxd0_del_ovrd_en, _rx_delay_cntrl_0.rxd1_del_sel, _rx_delay_cntrl_0.rxd1_del_ovrd_en, _rx_delay_cntrl_0.rxd2_del_sel, _rx_delay_cntrl_0.rxd2_del_ovrd_en, _rx_delay_cntrl_0.rxd3_del_sel, _rx_delay_cntrl_0.rxd3_del_ovrd_en);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_0_set(&_rx_delay_cntrl_0);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_0_get( &_rx_delay_cntrl_0);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_0_get( %u %u %u %u %u %u %u %u)\n", _rx_delay_cntrl_0.rxd0_del_sel, _rx_delay_cntrl_0.rxd0_del_ovrd_en, _rx_delay_cntrl_0.rxd1_del_sel, _rx_delay_cntrl_0.rxd1_del_ovrd_en, _rx_delay_cntrl_0.rxd2_del_sel, _rx_delay_cntrl_0.rxd2_del_ovrd_en, _rx_delay_cntrl_0.rxd3_del_sel, _rx_delay_cntrl_0.rxd3_del_ovrd_en);
        if(err || _rx_delay_cntrl_0.rxd0_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_0.rxd0_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_0.rxd1_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_0.rxd1_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_0.rxd2_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_0.rxd2_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_0.rxd3_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_0.rxd3_del_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__rx_delay_cntrl_1 _rx_delay_cntrl_1 = {.rxd4_del_sel=gtmv(m, 6), .rxd4_del_ovrd_en=gtmv(m, 1), .rxd5_del_sel=gtmv(m, 6), .rxd5_del_ovrd_en=gtmv(m, 1), .rxd6_del_sel=gtmv(m, 6), .rxd6_del_ovrd_en=gtmv(m, 1), .rxd7_del_sel=gtmv(m, 6), .rxd7_del_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_1_set( %u %u %u %u %u %u %u %u)\n", _rx_delay_cntrl_1.rxd4_del_sel, _rx_delay_cntrl_1.rxd4_del_ovrd_en, _rx_delay_cntrl_1.rxd5_del_sel, _rx_delay_cntrl_1.rxd5_del_ovrd_en, _rx_delay_cntrl_1.rxd6_del_sel, _rx_delay_cntrl_1.rxd6_del_ovrd_en, _rx_delay_cntrl_1.rxd7_del_sel, _rx_delay_cntrl_1.rxd7_del_ovrd_en);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_1_set(&_rx_delay_cntrl_1);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_1_get( &_rx_delay_cntrl_1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_1_get( %u %u %u %u %u %u %u %u)\n", _rx_delay_cntrl_1.rxd4_del_sel, _rx_delay_cntrl_1.rxd4_del_ovrd_en, _rx_delay_cntrl_1.rxd5_del_sel, _rx_delay_cntrl_1.rxd5_del_ovrd_en, _rx_delay_cntrl_1.rxd6_del_sel, _rx_delay_cntrl_1.rxd6_del_ovrd_en, _rx_delay_cntrl_1.rxd7_del_sel, _rx_delay_cntrl_1.rxd7_del_ovrd_en);
        if(err || _rx_delay_cntrl_1.rxd4_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_1.rxd4_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_1.rxd5_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_1.rxd5_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_1.rxd6_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_1.rxd6_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_1.rxd7_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_1.rxd7_del_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_slv__rx_delay_cntrl_2 _rx_delay_cntrl_2 = {.rxctl_pos_del_sel=gtmv(m, 6), .rxctl_pos_del_ovrd_en=gtmv(m, 1), .rxctl_neg_del_sel=gtmv(m, 6), .rxctl_neg_del_ovrd_en=gtmv(m, 1), .rxclk_del_sel=gtmv(m, 4), .rxclk_del_ovrd_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_2_set( %u %u %u %u %u %u)\n", _rx_delay_cntrl_2.rxctl_pos_del_sel, _rx_delay_cntrl_2.rxctl_pos_del_ovrd_en, _rx_delay_cntrl_2.rxctl_neg_del_sel, _rx_delay_cntrl_2.rxctl_neg_del_ovrd_en, _rx_delay_cntrl_2.rxclk_del_sel, _rx_delay_cntrl_2.rxclk_del_ovrd_en);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_2_set(&_rx_delay_cntrl_2);
        if(!err) ag_drv_ubus_slv__rx_delay_cntrl_2_get( &_rx_delay_cntrl_2);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__rx_delay_cntrl_2_get( %u %u %u %u %u %u)\n", _rx_delay_cntrl_2.rxctl_pos_del_sel, _rx_delay_cntrl_2.rxctl_pos_del_ovrd_en, _rx_delay_cntrl_2.rxctl_neg_del_sel, _rx_delay_cntrl_2.rxctl_neg_del_ovrd_en, _rx_delay_cntrl_2.rxclk_del_sel, _rx_delay_cntrl_2.rxclk_del_ovrd_en);
        if(err || _rx_delay_cntrl_2.rxctl_pos_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_2.rxctl_pos_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_2.rxctl_neg_del_sel!=gtmv(m, 6) || _rx_delay_cntrl_2.rxctl_neg_del_ovrd_en!=gtmv(m, 1) || _rx_delay_cntrl_2.rxclk_del_sel!=gtmv(m, 4) || _rx_delay_cntrl_2.rxclk_del_ovrd_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean swinit=gtmv(m, 1);
        bdmf_boolean clk250en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__clk_rst_ctrl_set( %u %u)\n", swinit, clk250en);
        if(!err) ag_drv_ubus_slv__clk_rst_ctrl_set(swinit, clk250en);
        if(!err) ag_drv_ubus_slv__clk_rst_ctrl_get( &swinit, &clk250en);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_slv__clk_rst_ctrl_get( %u %u)\n", swinit, clk250en);
        if(err || swinit!=gtmv(m, 1) || clk250en!=gtmv(m, 1))
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
    case bdmf_address_led_cntrl : reg = &RU_REG(UBUS_SLV, LED_CNTRL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_led_link_and_speed_encoding_sel : reg = &RU_REG(UBUS_SLV, LED_LINK_AND_SPEED_ENCODING_SEL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_led_link_and_speed_encoding : reg = &RU_REG(UBUS_SLV, LED_LINK_AND_SPEED_ENCODING); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address_led_blink_rate_cntrl : reg = &RU_REG(UBUS_SLV, LED_BLINK_RATE_CNTRL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__cntrl : reg = &RU_REG(UBUS_SLV, _CNTRL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ib_status : reg = &RU_REG(UBUS_SLV, _IB_STATUS); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__rx_clock_delay_cntrl : reg = &RU_REG(UBUS_SLV, _RX_CLOCK_DELAY_CNTRL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_rx_cntrl_exp_data : reg = &RU_REG(UBUS_SLV, _ATE_RX_CNTRL_EXP_DATA); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_rx_exp_data_1 : reg = &RU_REG(UBUS_SLV, _ATE_RX_EXP_DATA_1); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_rx_status_0 : reg = &RU_REG(UBUS_SLV, _ATE_RX_STATUS_0); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_rx_status_1 : reg = &RU_REG(UBUS_SLV, _ATE_RX_STATUS_1); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_tx_cntrl : reg = &RU_REG(UBUS_SLV, _ATE_TX_CNTRL); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_tx_data_0 : reg = &RU_REG(UBUS_SLV, _ATE_TX_DATA_0); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_tx_data_1 : reg = &RU_REG(UBUS_SLV, _ATE_TX_DATA_1); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__ate_tx_data_2 : reg = &RU_REG(UBUS_SLV, _ATE_TX_DATA_2); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__tx_delay_cntrl_0 : reg = &RU_REG(UBUS_SLV, _TX_DELAY_CNTRL_0); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__tx_delay_cntrl_1 : reg = &RU_REG(UBUS_SLV, _TX_DELAY_CNTRL_1); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__rx_delay_cntrl_0 : reg = &RU_REG(UBUS_SLV, _RX_DELAY_CNTRL_0); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__rx_delay_cntrl_1 : reg = &RU_REG(UBUS_SLV, _RX_DELAY_CNTRL_1); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__rx_delay_cntrl_2 : reg = &RU_REG(UBUS_SLV, _RX_DELAY_CNTRL_2); blk = &RU_BLK(UBUS_SLV); break;
    case bdmf_address__clk_rst_ctrl : reg = &RU_REG(UBUS_SLV, _CLK_RST_CTRL); blk = &RU_BLK(UBUS_SLV); break;
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
        static bdmfmon_cmd_parm_t set_led_cntrl[]={
            BDMFMON_MAKE_PARM("rx_act_en", "rx_act_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_act_en", "tx_act_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led0_act_sel", "spdlnk_led0_act_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led1_act_sel", "spdlnk_led1_act_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led2_act_sel", "spdlnk_led2_act_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("act_led_act_sel", "act_led_act_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led0_act_pol_sel", "spdlnk_led0_act_pol_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led1_act_pol_sel", "spdlnk_led1_act_pol_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spdlnk_led2_act_pol_sel", "spdlnk_led2_act_pol_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("act_led_pol_sel", "act_led_pol_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("led_spd_ovrd", "led_spd_ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lnk_status_ovrd", "lnk_status_ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("spd_ovrd_en", "spd_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lnk_ovrd_en", "lnk_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_led_link_and_speed_encoding_sel[]={
            BDMFMON_MAKE_PARM("sel_no_link_encode", "sel_no_link_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_10m_encode", "sel_10m_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_100m_encode", "sel_100m_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_1000m_encode", "sel_1000m_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_2500m_encode", "sel_2500m_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_10g_encode", "sel_10g_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_led_link_and_speed_encoding[]={
            BDMFMON_MAKE_PARM("no_link_encode", "no_link_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("m10_encode", "m10_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("m100_encode", "m100_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("m1000_encode", "m1000_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("m2500_encode", "m2500_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("m10g_encode", "m10g_encode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_led_blink_rate_cntrl[]={
            BDMFMON_MAKE_PARM("led_off_time", "led_off_time", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("led_on_time", "led_on_time", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__cntrl[]={
            BDMFMON_MAKE_PARM("rgmii_mode_en", "rgmii_mode_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("id_mode_dis", "id_mode_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("port_mode", "port_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rvmii_ref_sel", "rvmii_ref_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_pause_en", "rx_pause_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_pause_en", "tx_pause_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_clk_stop_en", "tx_clk_stop_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lpi_count", "lpi_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_err_mask", "rx_err_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("col_crs_mask", "col_crs_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pseudo_hd_mode_en", "pseudo_hd_mode_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ib_status[]={
            BDMFMON_MAKE_PARM("speed_decode", "speed_decode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("duplex_decode", "duplex_decode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("link_decode", "link_decode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ib_status_ovrd", "ib_status_ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__rx_clock_delay_cntrl[]={
            BDMFMON_MAKE_PARM("ctri", "ctri", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("drng", "drng", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("iddq", "iddq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bypass", "bypass", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dly_sel", "dly_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dly_override", "dly_override", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reset", "reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_rx_cntrl_exp_data[]={
            BDMFMON_MAKE_PARM("expected_data_0", "expected_data_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("expected_data_1", "expected_data_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("good_count", "good_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_count_rst", "pkt_count_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ate_en", "ate_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_rx_exp_data_1[]={
            BDMFMON_MAKE_PARM("expected_data_2", "expected_data_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("expected_data_3", "expected_data_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_tx_cntrl[]={
            BDMFMON_MAKE_PARM("start_stop_ovrd", "start_stop_ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("start_stop", "start_stop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_gen_en", "pkt_gen_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_cnt", "pkt_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("payload_length", "payload_length", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_ipg", "pkt_ipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_tx_data_0[]={
            BDMFMON_MAKE_PARM("tx_data_0", "tx_data_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_data_1", "tx_data_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_tx_data_1[]={
            BDMFMON_MAKE_PARM("tx_data_2", "tx_data_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_data_3", "tx_data_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__ate_tx_data_2[]={
            BDMFMON_MAKE_PARM("tx_data_4", "tx_data_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_data_5", "tx_data_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ether_type", "ether_type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__tx_delay_cntrl_0[]={
            BDMFMON_MAKE_PARM("txd0_del_sel", "txd0_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd0_del_ovrd_en", "txd0_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd1_del_sel", "txd1_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd1_del_ovrd_en", "txd1_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd2_del_sel", "txd2_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd2_del_ovrd_en", "txd2_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd3_del_sel", "txd3_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txd3_del_ovrd_en", "txd3_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__tx_delay_cntrl_1[]={
            BDMFMON_MAKE_PARM("txctl_del_sel", "txctl_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txctl_del_ovrd_en", "txctl_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txclk_del_sel", "txclk_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txclk_del_ovrd_en", "txclk_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txclk_id_del_sel", "txclk_id_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txclk_id_del_ovrd_en", "txclk_id_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__rx_delay_cntrl_0[]={
            BDMFMON_MAKE_PARM("rxd0_del_sel", "rxd0_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd0_del_ovrd_en", "rxd0_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd1_del_sel", "rxd1_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd1_del_ovrd_en", "rxd1_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd2_del_sel", "rxd2_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd2_del_ovrd_en", "rxd2_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd3_del_sel", "rxd3_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd3_del_ovrd_en", "rxd3_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__rx_delay_cntrl_1[]={
            BDMFMON_MAKE_PARM("rxd4_del_sel", "rxd4_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd4_del_ovrd_en", "rxd4_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd5_del_sel", "rxd5_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd5_del_ovrd_en", "rxd5_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd6_del_sel", "rxd6_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd6_del_ovrd_en", "rxd6_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd7_del_sel", "rxd7_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxd7_del_ovrd_en", "rxd7_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__rx_delay_cntrl_2[]={
            BDMFMON_MAKE_PARM("rxctl_pos_del_sel", "rxctl_pos_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxctl_pos_del_ovrd_en", "rxctl_pos_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxctl_neg_del_sel", "rxctl_neg_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxctl_neg_del_ovrd_en", "rxctl_neg_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxclk_del_sel", "rxclk_del_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxclk_del_ovrd_en", "rxclk_del_ovrd_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set__clk_rst_ctrl[]={
            BDMFMON_MAKE_PARM("swinit", "swinit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clk250en", "clk250en", BDMFMON_PARM_NUMBER, 0),
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
            { .name="led_cntrl", .val=cli_ubus_slv_led_cntrl, .parms=set_led_cntrl },
            { .name="led_link_and_speed_encoding_sel", .val=cli_ubus_slv_led_link_and_speed_encoding_sel, .parms=set_led_link_and_speed_encoding_sel },
            { .name="led_link_and_speed_encoding", .val=cli_ubus_slv_led_link_and_speed_encoding, .parms=set_led_link_and_speed_encoding },
            { .name="led_blink_rate_cntrl", .val=cli_ubus_slv_led_blink_rate_cntrl, .parms=set_led_blink_rate_cntrl },
            { .name="_cntrl", .val=cli_ubus_slv__cntrl, .parms=set__cntrl },
            { .name="_ib_status", .val=cli_ubus_slv__ib_status, .parms=set__ib_status },
            { .name="_rx_clock_delay_cntrl", .val=cli_ubus_slv__rx_clock_delay_cntrl, .parms=set__rx_clock_delay_cntrl },
            { .name="_ate_rx_cntrl_exp_data", .val=cli_ubus_slv__ate_rx_cntrl_exp_data, .parms=set__ate_rx_cntrl_exp_data },
            { .name="_ate_rx_exp_data_1", .val=cli_ubus_slv__ate_rx_exp_data_1, .parms=set__ate_rx_exp_data_1 },
            { .name="_ate_tx_cntrl", .val=cli_ubus_slv__ate_tx_cntrl, .parms=set__ate_tx_cntrl },
            { .name="_ate_tx_data_0", .val=cli_ubus_slv__ate_tx_data_0, .parms=set__ate_tx_data_0 },
            { .name="_ate_tx_data_1", .val=cli_ubus_slv__ate_tx_data_1, .parms=set__ate_tx_data_1 },
            { .name="_ate_tx_data_2", .val=cli_ubus_slv__ate_tx_data_2, .parms=set__ate_tx_data_2 },
            { .name="_tx_delay_cntrl_0", .val=cli_ubus_slv__tx_delay_cntrl_0, .parms=set__tx_delay_cntrl_0 },
            { .name="_tx_delay_cntrl_1", .val=cli_ubus_slv__tx_delay_cntrl_1, .parms=set__tx_delay_cntrl_1 },
            { .name="_rx_delay_cntrl_0", .val=cli_ubus_slv__rx_delay_cntrl_0, .parms=set__rx_delay_cntrl_0 },
            { .name="_rx_delay_cntrl_1", .val=cli_ubus_slv__rx_delay_cntrl_1, .parms=set__rx_delay_cntrl_1 },
            { .name="_rx_delay_cntrl_2", .val=cli_ubus_slv__rx_delay_cntrl_2, .parms=set__rx_delay_cntrl_2 },
            { .name="_clk_rst_ctrl", .val=cli_ubus_slv__clk_rst_ctrl, .parms=set__clk_rst_ctrl },
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
            { .name="led_cntrl", .val=cli_ubus_slv_led_cntrl, .parms=set_default },
            { .name="led_link_and_speed_encoding_sel", .val=cli_ubus_slv_led_link_and_speed_encoding_sel, .parms=set_default },
            { .name="led_link_and_speed_encoding", .val=cli_ubus_slv_led_link_and_speed_encoding, .parms=set_default },
            { .name="led_blink_rate_cntrl", .val=cli_ubus_slv_led_blink_rate_cntrl, .parms=set_default },
            { .name="_cntrl", .val=cli_ubus_slv__cntrl, .parms=set_default },
            { .name="_ib_status", .val=cli_ubus_slv__ib_status, .parms=set_default },
            { .name="_rx_clock_delay_cntrl", .val=cli_ubus_slv__rx_clock_delay_cntrl, .parms=set_default },
            { .name="_ate_rx_cntrl_exp_data", .val=cli_ubus_slv__ate_rx_cntrl_exp_data, .parms=set_default },
            { .name="_ate_rx_exp_data_1", .val=cli_ubus_slv__ate_rx_exp_data_1, .parms=set_default },
            { .name="_ate_rx_status_0", .val=cli_ubus_slv__ate_rx_status_0, .parms=set_default },
            { .name="_ate_rx_status_1", .val=cli_ubus_slv__ate_rx_status_1, .parms=set_default },
            { .name="_ate_tx_cntrl", .val=cli_ubus_slv__ate_tx_cntrl, .parms=set_default },
            { .name="_ate_tx_data_0", .val=cli_ubus_slv__ate_tx_data_0, .parms=set_default },
            { .name="_ate_tx_data_1", .val=cli_ubus_slv__ate_tx_data_1, .parms=set_default },
            { .name="_ate_tx_data_2", .val=cli_ubus_slv__ate_tx_data_2, .parms=set_default },
            { .name="_tx_delay_cntrl_0", .val=cli_ubus_slv__tx_delay_cntrl_0, .parms=set_default },
            { .name="_tx_delay_cntrl_1", .val=cli_ubus_slv__tx_delay_cntrl_1, .parms=set_default },
            { .name="_rx_delay_cntrl_0", .val=cli_ubus_slv__rx_delay_cntrl_0, .parms=set_default },
            { .name="_rx_delay_cntrl_1", .val=cli_ubus_slv__rx_delay_cntrl_1, .parms=set_default },
            { .name="_rx_delay_cntrl_2", .val=cli_ubus_slv__rx_delay_cntrl_2, .parms=set_default },
            { .name="_clk_rst_ctrl", .val=cli_ubus_slv__clk_rst_ctrl, .parms=set_default },
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
            { .name="LED_CNTRL" , .val=bdmf_address_led_cntrl },
            { .name="LED_LINK_AND_SPEED_ENCODING_SEL" , .val=bdmf_address_led_link_and_speed_encoding_sel },
            { .name="LED_LINK_AND_SPEED_ENCODING" , .val=bdmf_address_led_link_and_speed_encoding },
            { .name="LED_BLINK_RATE_CNTRL" , .val=bdmf_address_led_blink_rate_cntrl },
            { .name="_CNTRL" , .val=bdmf_address__cntrl },
            { .name="_IB_STATUS" , .val=bdmf_address__ib_status },
            { .name="_RX_CLOCK_DELAY_CNTRL" , .val=bdmf_address__rx_clock_delay_cntrl },
            { .name="_ATE_RX_CNTRL_EXP_DATA" , .val=bdmf_address__ate_rx_cntrl_exp_data },
            { .name="_ATE_RX_EXP_DATA_1" , .val=bdmf_address__ate_rx_exp_data_1 },
            { .name="_ATE_RX_STATUS_0" , .val=bdmf_address__ate_rx_status_0 },
            { .name="_ATE_RX_STATUS_1" , .val=bdmf_address__ate_rx_status_1 },
            { .name="_ATE_TX_CNTRL" , .val=bdmf_address__ate_tx_cntrl },
            { .name="_ATE_TX_DATA_0" , .val=bdmf_address__ate_tx_data_0 },
            { .name="_ATE_TX_DATA_1" , .val=bdmf_address__ate_tx_data_1 },
            { .name="_ATE_TX_DATA_2" , .val=bdmf_address__ate_tx_data_2 },
            { .name="_TX_DELAY_CNTRL_0" , .val=bdmf_address__tx_delay_cntrl_0 },
            { .name="_TX_DELAY_CNTRL_1" , .val=bdmf_address__tx_delay_cntrl_1 },
            { .name="_RX_DELAY_CNTRL_0" , .val=bdmf_address__rx_delay_cntrl_0 },
            { .name="_RX_DELAY_CNTRL_1" , .val=bdmf_address__rx_delay_cntrl_1 },
            { .name="_RX_DELAY_CNTRL_2" , .val=bdmf_address__rx_delay_cntrl_2 },
            { .name="_CLK_RST_CTRL" , .val=bdmf_address__clk_rst_ctrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_ubus_slv_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

